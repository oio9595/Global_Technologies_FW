/** @file dimming_test.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __DIMMING_TEST_C__
#include "config.h"

#define LD_WIDTH_MAX            (0xFFFF)

#define FAULT_MASK_FB           (1 << 0)
#define FAULT_MASK_OPEN         (1 << 1)
#define FAULT_MASK_SHORT        (1 << 2)
#define FAULT_MASK_THERMAL      (1 << 3)

bool gb_jig_vsync_running_flag;
static bool gb_xd04_vsync_flag;

static bool gb_xd_write_flag;
static uint8_t gn_xd_write_addr;
static uint16_t gn_xd_write_data;

static bool gb_xd_read_flag;
static uint8_t gn_xd_read_addr;

static uint16_t gn_xd04_LD_out;

void Vsync_Timer_Start(void)
{
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);

    gb_jig_vsync_running_flag = true;
}

void Vsync_Timer_Stop(void)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM8);

    gb_jig_vsync_running_flag = false;
}

void Vsync_Update_Handler(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM8);
    gb_xd04_vsync_flag = true;
}

void XD04_Vsync_Task(void)
{
    if (gb_xd04_vsync_flag)
    {
        if (!XD04_Is_Vsync_Mode_External())
        {
            JigBD_IF_SyncGen_Command();
        }

        JigBD_IF_Write_LD_Command(gn_xd04_LD_out);

        // fault read if needed
        XD04_get_fault_status();

        if (gb_xd_write_flag)
        {
            XD04_Write_General_Reg(gn_xd_write_addr, gn_xd_write_data);
            gb_xd_write_flag = false;
        }
        if (gb_xd_read_flag)
        {
            XD04_Read_General_Reg(gn_xd_read_addr);
            gb_xd_read_flag = false;
        }

        gb_xd04_vsync_flag = false;
    }
}

void XD04_Set_Write_Target_Reg(uint8_t addr, uint16_t data)
{
    gn_xd_write_addr = addr;
    gn_xd_write_data = data;
    gb_xd_write_flag = true;
}

void XD04_Set_Read_Target_Reg(uint8_t addr)
{
    gn_xd_read_addr = addr;
    gb_xd_read_flag = true;
}

void XD04_set_LD_out(uint32_t in_ld_out)
{
    if (in_ld_out <= LD_WIDTH_MAX)
    {
        gn_xd04_LD_out = in_ld_out;
    }
    else
    {
        print(LOG_ERROR, "\r\n Out of LD_out [%u] [0 - %u]\r\n", in_ld_out, LD_WIDTH_MAX);
    }
}

uint16_t XD04_get_LD_out(void)
{
    return gn_xd04_LD_out;
}

void XD04_get_fault_status(void)
{
    uint8_t now_fault_status = (JigBD_IF_Fault_Read_Command() & 0x0F);
    static uint8_t prev_fault_status = 0xFF;
    static uint16_t vsync_tick = 0;

    if (now_fault_status != prev_fault_status)
    {
        if (!now_fault_status)
        {
            DEBUG_LO();
            print(LOG_INFO, "\r\n [%u] XD FAULT None\r\n", vsync_tick);
        }
        else
        {
            DEBUG_HI();
            if (now_fault_status & FAULT_MASK_FB)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [FB]\r\n", vsync_tick);
            }
            if (now_fault_status & FAULT_MASK_OPEN)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [OPEN]\r\n", vsync_tick);
            }
            if (now_fault_status & FAULT_MASK_SHORT)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [SHORT]\r\n", vsync_tick);
            }
            if (now_fault_status & FAULT_MASK_THERMAL)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [THERMAL]\r\n", vsync_tick);
            }
        }
        prev_fault_status = now_fault_status;
    }
#if 0
    if (vsync_tick % 120 == 0)
    {
        print(LOG_INFO, "\r\n %u sec\r\n", vsync_tick / 120);
    }
#endif
    ++vsync_tick;
}

/*** end of file ***/