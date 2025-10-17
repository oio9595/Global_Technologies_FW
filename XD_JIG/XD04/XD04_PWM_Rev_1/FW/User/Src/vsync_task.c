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

#define LD_WIDTH_MAX            (0xFFFF)

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
    LL_TIM_ClearFlag_UPDATE(TIM8);
    if (!XDIC_Is_Vsync_Mode_External())
    {
        JigBD_IF_SyncGen_Command();
    }
    gb_xdic_vsync_flag = true;
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

    char msg[50] = {0, };

    if (now_fault_status != prev_fault_status)
    {
        if (!now_fault_status)
        {
            print(LOG_INFO, "\r\n [%u] XD FAULT None\r\n", vsync_tick);
        }
        else
        {
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
#if 0
    if (vsync_tick % 120 == 0)
    {
        print(LOG_INFO, "\r\n %u sec\r\n", vsync_tick / 120);
        if (vsync_tick / 120 == 10)
        {
            NVIC_SystemReset();
        }
    }
#endif
}

void XDIC_Vsync_Task(void)
{
    if (gb_xdic_vsync_flag)
    {
        // fault read if needed
        XDIC_Get_Fault_Status();

        JigBD_IF_Write_LD_Command(gn_xdic_LD_out);

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