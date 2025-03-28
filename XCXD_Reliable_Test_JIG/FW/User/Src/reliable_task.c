/** @file reliable_task.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __RELIABLE_TASK_C__

#include "reliable_task.h"
#include "xd12.h"
#include "xc24.h"

#define LD_WIDTH_MAX            (0xFFFF)

#define FAULT_MASK_FB           (1 << 0)
#define FAULT_MASK_OPEN         (1 << 1)
#define FAULT_MASK_SHORT        (1 << 2)
#define FAULT_MASK_THERMAL      (1 << 3)

static bool gb_xd12_vsync_flag;

static bool gb_xd_write_flag;
static uint8_t gn_xd_write_addr;
static uint16_t gn_xd_write_data;

static bool gb_xd_read_flag;
static uint8_t gn_xd_read_addr;

static uint16_t gn_xd12_LD_out;

void Vsync_Timer_Start(void)
{
    LL_TIM_OC_SetCompareCH2(TIM3, VSYNC_CCR);
    LL_TIM_EnableIT_UPDATE(TIM3);
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM3);

    gn_xd12_LD_out = 4 + 16 + 64;
}

void Vsync_Timer_Stop(void)
{
    LL_TIM_DisableCounter(TIM3);
    LL_TIM_SetCounter(TIM3, 0);
    LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM3);
    LL_TIM_OC_SetCompareCH2(TIM3, 0);
    LL_TIM_ClearFlag_UPDATE(TIM3);
}

void Vsync_Update_Handler(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM3);
    LL_TIM_OC_SetCompareCH2(TIM3, VSYNC_CCR);

    gb_xd12_vsync_flag = true;
}

void XD12_Vsync_Task(void)
{
    if (gb_xd12_vsync_flag)
    {
        if (!XD12_Is_Vsync_Mode_External())
        {
            XC24_IF_SyncGen_Command();
            us_tdelay(124);
        }

        XC24_IF_Write_LD(gn_xd12_LD_out);

        // fault read if needed
        // XD12_get_fault_status();

        if (gb_xd_write_flag)
        {
            XD12_Write_General_Reg(gn_xd_write_addr, gn_xd_write_data);
            gb_xd_write_flag = false;
        }
        if (gb_xd_read_flag)
        {
            XD12_Read_General_Reg(gn_xd_read_addr);
            gb_xd_read_flag = false;
        }

        gb_xd12_vsync_flag = false;
    }
}

void XD12_Set_Write_Target_Reg(uint8_t addr, uint16_t data)
{
    gn_xd_write_addr = addr;
    gn_xd_write_data = data;
    gb_xd_write_flag = true;
}

void XD12_Set_Read_Target_Reg(uint8_t addr)
{
    gn_xd_read_addr = addr;
    gb_xd_read_flag = true;
}

void XD12_set_LD_out(uint32_t in_ld_out)
{
    if (in_ld_out <= LD_WIDTH_MAX)
    {
        gn_xd12_LD_out = in_ld_out;
    }
    else
    {
        debugging_UART_Printf(LOG_LV_ERROR, "\r\n Out of LD_out [%u] [0 - %u]\r\n", in_ld_out, LD_WIDTH_MAX);
    }
}

uint16_t XD12_get_LD_out(void)
{
    return gn_xd12_LD_out;
}

void XD12_get_fault_status(void)
{
    static uint16_t vsync_tick = 0;
    uint8_t now_fault_status = 0;
    static uint8_t prev_fault_status = 0xFF;

    now_fault_status = (XC24_IF_Fault_Read_Command() & 0x0F);

    if (now_fault_status != prev_fault_status)
    {
        if (!now_fault_status)
        {
            debugging_UART_Printf(LOG_LV_INFO, "\r\n [%u] XD FAULT None\r\n", vsync_tick);
        }
        else
        {
            if (now_fault_status & FAULT_MASK_FB)
            {
                debugging_UART_Printf(LOG_LV_INFO, "\r\n [%u] XD FAULT Detected [FB]\r\n", vsync_tick);
            }
            if (now_fault_status & FAULT_MASK_OPEN)
            {
                debugging_UART_Printf(LOG_LV_INFO, "\r\n [%u] XD FAULT Detected [OPEN]\r\n", vsync_tick);
            }
            if (now_fault_status & FAULT_MASK_SHORT)
            {
                debugging_UART_Printf(LOG_LV_INFO, "\r\n [%u] XD FAULT Detected [SHORT]\r\n", vsync_tick);
            }
            if (now_fault_status & FAULT_MASK_THERMAL)
            {
                debugging_UART_Printf(LOG_LV_INFO, "\r\n [%u] XD FAULT Detected [THERMAL]\r\n", vsync_tick);
            }
        }
        prev_fault_status = now_fault_status;
    }
    ++vsync_tick;
}

/*** end of file ***/