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

#define LD_WIDTH_MAX            (0x3FFF)

#define FAULT_MASK_FB           (1 << 0)
#define FAULT_MASK_OPEN         (1 << 1)
#define FAULT_MASK_SHORT        (1 << 2)
#define FAULT_MASK_THERMAL      (1 << 3)

bool gb_jig_vsync_active;
static bool gb_xdic_vsync_flag;

static bool gb_xdic_write_flag;
static uint8_t gn_xdic_write_addr;
static uint16_t gn_xdic_write_data;

static bool gb_xdic_read_flag;
static uint8_t gn_xdic_read_addr;

static uint16_t gn_xdic_LD_out;

static uint16_t gn_svsync_count;

static bool gb_svsync_phase_vsync;

static bool gb_vsync_for_trim;

#define TIM3_APB_FREQ_HZ        (APB1_TIM_FREQ * 1000000U)
#define TIM3_PRESCALER          (0U)
#define TIM3_FREQUENCY_HZ       (TIM3_APB_FREQ_HZ / (TIM3_PRESCALER + 1))
#define TIM3_CALC_PERIOD(freq)  ((uint32_t)(TIM3_FREQUENCY_HZ / (freq) - 1U))

void Svsync_Timer_Start(void)
{
    gn_svsync_count = 0;
    // For Vsync, change frequency
    uint32_t period = 3599; // 10us on / 30us off
    LL_TIM_SetAutoReload(TIM3, period);
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM3);
    gb_svsync_phase_vsync = true;
}

void Svsync_Update_Handler(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM3);

    if (gb_svsync_phase_vsync)
    {
        gb_svsync_phase_vsync = false;
    }
    else
    {
        ++gn_svsync_count;
        uint8_t phase = (gn_svsync_count % SVSYNC_CYCLE);
        if (phase == SVSYNC_PHASE_GREEN)
        {
            // For Green, change frequency
            uint32_t period = TIM3_CALC_PERIOD(7680); // 50% of 1-sub frame
            LL_TIM_SetAutoReload(TIM3, period);
        }
        else if (phase == SVSYNC_PHASE_BLUE)
        {
            // For Blue, change frequency
            uint32_t period = TIM3_CALC_PERIOD(7680); // 50% of 1-sub frame
            LL_TIM_SetAutoReload(TIM3, period);
            if (gn_svsync_count == SVSYNC_TOTAL_CYCLE)
            {
                us_delay(SVSYNC_GATING_TIME_US + 1);
                LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1);
                LL_TIM_DisableCounter(TIM3);
            }
        }
    }
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
    LL_TIM_ClearFlag_UPDATE(TIM8);
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
    LL_TIM_ClearFlag_UPDATE(TIM8);
#if 0
    if (!IS_XC24_Support())
    {
        JigBD_IF_SyncGen_Command();
    }
#endif
    Svsync_Timer_Start();
    if (!gb_vsync_for_trim)
    {
        gb_xdic_vsync_flag = true;
    }
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

void XDIC_Set_LD_Data(uint32_t in_ld_out)
{
    if (in_ld_out <= LD_WIDTH_MAX)
    {
        gn_xdic_LD_out = in_ld_out;
    }
    else
    {
        print(LOG_ERROR, "\r\n Out of LD_out [%u] [0 - %u]\r\n", in_ld_out, LD_WIDTH_MAX);
    }
}

uint16_t XDIC_Get_LD_Data(void)
{
    return gn_xdic_LD_out;
}

void XDIC_Get_Fault_Status(void)
{
    uint8_t now_fault_status = (JigBD_IF_Fault_Read_Command() & 0x0F);
    static uint8_t prev_fault_status = 0xFF;
    static uint16_t vsync_tick = 0;

    if (now_fault_status != prev_fault_status)
    {
        if (!now_fault_status)
        {
            print(LOG_INFO, "\r\n [%u] XD FAULT None\r\n", vsync_tick);
        }
        else
        {
            char msg[50] = {0, };
            snprintf(msg, sizeof(msg), "\r\n [%u] XD FAULT Detected [ ", vsync_tick);
            if (now_fault_status & FAULT_MASK_FB)
            {
                strncat(msg, "FB ", sizeof(msg) - strlen(msg) - 1);
            }
            if (now_fault_status & FAULT_MASK_OPEN)
            {
                strncat(msg, "OPEN ", sizeof(msg) - strlen(msg) - 1);
            }
            if (now_fault_status & FAULT_MASK_SHORT)
            {
                strncat(msg, "SHORT ", sizeof(msg) - strlen(msg) - 1);
            }
            if (now_fault_status & FAULT_MASK_THERMAL)
            {
                strncat(msg, "THERMAL", sizeof(msg) - strlen(msg) - 1);
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
        if (IS_XC24_Support())
        {
            XC24_Turn_Off_Sync_Auto();
            us_delay(100);
            XC24_Turn_On_Sync_Auto();
        }

        us_delay(1500);
        JigBD_IF_Write_LD_Command(gn_xdic_LD_out);
        //JigBD_IF_Fault_Read_Command();

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