/** @file vsync_task.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
 #define __VSYNC_TASK_C__

#include "main.h"
#include "JigBd_IF.h"
#include "xdic.h"
#include "xc24.h"
#include "config.h"

#define LD_WIDTH_MAX        (0x3FFF)

#define FAULT_MASK_FB1      (1 << 0)
#define FAULT_MASK_FB2      (1 << 1)
#define FAULT_MASK_FB3      (1 << 2)
#define FAULT_MASK_FAULT    (1 << 3)

bool gb_jig_vsync_active;
static bool gb_xdic_vsync_flag;

static bool gb_xdic_write_flag;
static uint8_t gn_xdic_write_addr;
static uint16_t gn_xdic_write_data;

static bool gb_xdic_read_flag;
static uint8_t gn_xdic_read_addr;

static uint16_t gn_xdic_LD_out[3]; // R, G, B

static uint16_t gn_svsync_count;

static bool gb_vsync_for_trim;

#define TIM3_APB_FREQ_Hz        (APB1_TIM_FREQ * 1000000U)
#define TIM3_APB_FREQ_MHz       (APB1_TIM_FREQ)
#define TIM3_PRESCALER          (0U)
#define TIM3_FREQUENCY_Hz       (TIM3_APB_FREQ_Hz / (TIM3_PRESCALER + 1))
#define TIM3_CALC_PERIOD(freq)  ((uint32_t)(TIM3_FREQUENCY_Hz / freq - 1U))
#define TIM3_CALC_COMPARE(us)   ((uint32_t)(TIM3_APB_FREQ_MHz * us - 1U))

#define SVSYNC_VSYNC_FREQ       (50000U)
#define SVSYNC_VSYNC_ON_us      (10U) // 10us

#define SVSYNC_RED_FREQ         (11520U)
#define SVSYNC_GREEN_FREQ       (7680U)
#define SVSYNC_BLUE_FREQ        (7680U)
#define SVSYNC_DIMMING_ON_us    (20U) // 20us

void Svsync_Timer_Start(void)
{
    gn_svsync_count = 0U;
    LL_TIM_DisableCounter(TIM3);
    LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1);

    LL_TIM_SetCounter(TIM3, 0);
    LL_TIM_ClearFlag_UPDATE(TIM3);

    uint32_t period = TIM3_CALC_PERIOD(SVSYNC_VSYNC_FREQ); // 50% of 1-sub frame
    LL_TIM_SetAutoReload(TIM3, period);

    uint32_t compare = TIM3_CALC_COMPARE(SVSYNC_VSYNC_ON_us); // 10us
    LL_TIM_OC_SetCompareCH1(TIM3, compare);

    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM3);
}

void Svsync_Update_Handler(void)
{
    ++gn_svsync_count;
    uint8_t phase = (gn_svsync_count % SVSYNC_CYCLE);

    if (phase == SVSYNC_PHASE_GREEN) // For Green, change frequency
    {
        uint32_t period = TIM3_CALC_PERIOD(SVSYNC_GREEN_FREQ); // 50% of 1-sub frame
        LL_TIM_SetAutoReload(TIM3, period);
    }
    else if (phase == SVSYNC_PHASE_BLUE) // For Blue, change frequency
    {
        uint32_t period = TIM3_CALC_PERIOD(SVSYNC_BLUE_FREQ); // 50% of 1-sub frame
        LL_TIM_SetAutoReload(TIM3, period);
        if (gn_svsync_count == SVSYNC_TOTAL_CYCLE)
        {
            us_delay(SVSYNC_GATING_TIME_US + 1);
            LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1);
            LL_TIM_DisableCounter(TIM3);
            LL_TIM_SetCounter(TIM3, 0);
        }
    }

    uint32_t compare = TIM3_CALC_COMPARE(SVSYNC_DIMMING_ON_us); // 20us
    LL_TIM_OC_SetCompareCH1(TIM3, compare);
}

void Trim_Vsync_Timer_Start(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM8);
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);

    gb_vsync_for_trim = true;
}

void Vsync_Timer_Start(void)
{
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);

    gb_jig_vsync_active = true;
}

void Vsync_Timer_Stop(void)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM8);

    gb_jig_vsync_active = false;
}

void Vsync_Update_Handler(void)
{
    Svsync_Timer_Start();
    //JigBD_IF_SyncGen_Command();
    if (!gb_vsync_for_trim)
    {
        gb_xdic_vsync_flag = true;
    }
    else
    {
        JigBD_IF_SyncGen_Command();
    }
}

void Vsync_Change_Frequency(uint16_t in_freq_Hz)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0);
    uint32_t prescaler = LL_TIM_GetPrescaler(TIM8);
    uint32_t period = (uint32_t)((APB2_TIM_FREQ * 1000000U) / ((prescaler + 1U) * in_freq_Hz) - 1U);
    LL_TIM_SetAutoReload(TIM8, period);

    LL_TIM_EnableCounter(TIM8);
}

void XDIC_Set_Write_Target_Reg(uint8_t addr, uint16_t data)
{
    gn_xdic_write_addr = addr;
    gn_xdic_write_data = data;
    gb_xdic_write_flag = true;
}

void XDIC_Set_Read_Target_Reg(uint8_t addr)
{
    gn_xdic_read_addr = addr;
    gb_xdic_read_flag = true;
}

void XDIC_Set_LD_Data(uint32_t in_ld_R, uint32_t in_ld_G, uint32_t in_ld_B)
{
    if (in_ld_R <= LD_WIDTH_MAX && in_ld_G <= LD_WIDTH_MAX && in_ld_B <= LD_WIDTH_MAX)
    {
        gn_xdic_LD_out[0] = in_ld_R;
        gn_xdic_LD_out[1] = in_ld_G;
        gn_xdic_LD_out[2] = in_ld_B;
    }
    else
    {
        print(LOG_ERROR, "\r\n Out of LD_out [%u, %u, %u] [0 - %u]\r\n", in_ld_R, in_ld_G, in_ld_B, LD_WIDTH_MAX);
    }
}

uint16_t* XDIC_Get_LD_Data(void)
{
    return gn_xdic_LD_out;
}

void XDIC_Get_Fault_Status(void)
{
    uint8_t now_fault_status = (JigBD_IF_Fault_Read_Command() & 0x0FU);
    static uint8_t prev_fault_status = 0xFFU;
    static uint16_t vsync_tick = 0U;

    if (now_fault_status != prev_fault_status)
    {
        if (!now_fault_status)
        {
            print(LOG_INFO, "\r\n [%u] XD FAULT None\r\n", vsync_tick);
        }
        else
        {
            char msg[55] = { 0 };
            snprintf(msg, sizeof(msg), "\r\n [%u] XD FAULT Detected [ ", vsync_tick);
            if (now_fault_status & FAULT_MASK_FB1)
            {
                strncat(msg, "FB1 ", sizeof(msg) - strlen(msg) - 1);
            }
            if (now_fault_status & FAULT_MASK_FB2)
            {
                strncat(msg, "FB2 ", sizeof(msg) - strlen(msg) - 1);
            }
            if (now_fault_status & FAULT_MASK_FB3)
            {
                strncat(msg, "FB3 ", sizeof(msg) - strlen(msg) - 1);
            }
            if (now_fault_status & FAULT_MASK_FAULT)
            {
                strncat(msg, "FAULT", sizeof(msg) - strlen(msg) - 1);
            }
            strncat(msg, " ]\r\n", sizeof(msg) - strlen(msg) - 1);
            print(LOG_INFO, "%s", msg);
        }
        prev_fault_status = now_fault_status;
    }
    ++vsync_tick;
}

void XDIC_Vsync_Task(void)
{
    if (gb_xdic_vsync_flag)
    {
        if (IS_XC24R_Support())
        {
            XC24R_Turn_Off_Sync_Auto();
            us_delay(100);
            XC24R_Turn_On_Sync_Auto();
        }

        us_delay(1500);
        JigBD_IF_Write_LD_Command(gn_xdic_LD_out);
        XDIC_Get_Fault_Status();

        if (gb_xdic_write_flag)
        {
            XDIC_Write_General_Reg(gn_xdic_write_addr, gn_xdic_write_data);
            gb_xdic_write_flag = false;
        }
        if (gb_xdic_read_flag)
        {
            uint16_t ret = XDIC_Read_General_Reg(gn_xdic_read_addr);
            print(LOG_INFO, "XDIC Read --> [ 0x%02X - 0x%04X] \r\n", gn_xdic_read_addr, ret);
            gb_xdic_read_flag = false;
        }
        gb_xdic_vsync_flag = false;
    }
}

/*** end of file ***/