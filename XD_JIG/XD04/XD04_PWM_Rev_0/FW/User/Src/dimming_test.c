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
        // fault read if need

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
    static uint16_t vsync_tick = 0;
#if 0
    uint16_t now_fault_status_reg_read = 0;
    static _xd04_fault_status_t prev_xd04_fault_status_ = {1, };
    now_fault_status_reg_read = TargetIC_IF_Read_Register(XD04_ADDR_FAULT_STATUS, XD04_REG_TYPE_NON_TRIM);

    if (now_fault_status_reg_read != prev_xd04_fault_status_.val)
    {
        prev_xd04_fault_status_.val = now_fault_status_reg_read;
        if (!(prev_xd04_fault_status_.val))
        {
            print(LOG_INFO, "\r\n [%u] XD FAULT Nothing [REG READ]\r\n", vsync_tick);
        }
        else
        {
            if (prev_xd04_fault_status_.bit_fb)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [FB] [REG READ]\r\n", vsync_tick);
            }
            if (prev_xd04_fault_status_.bit_open)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [OPEN] [REG READ]\r\n", vsync_tick);
            }
            if (prev_xd04_fault_status_.bit_short)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [SHORT] [REG READ]\r\n", vsync_tick);
            }
            if (prev_xd04_fault_status_.bit_thermal)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [THERMAL] [REG READ]\r\n", vsync_tick);
            }
            if (prev_xd04_fault_status_.bit_miss_vs)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [MISS_VS] [REG READ]\r\n", vsync_tick);
            }
        }
    }
#endif
    uint8_t now_fault_status_command_read = 0;
    static uint8_t prev_fault_status_command_read = 0xFF;
    now_fault_status_command_read = (JigBD_IF_Fault_Read_Command() & 0x0F);

    if (now_fault_status_command_read != prev_fault_status_command_read)
    {
        if (!now_fault_status_command_read)
        {
            print(LOG_INFO, "\r\n [%u] XD FAULT Nothing [CMD READ]\r\n", vsync_tick);
        }
        else
        {
            if (now_fault_status_command_read & 0x1)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [FB] [CMD READ]\r\n", vsync_tick);
            }
            if (now_fault_status_command_read & 0x2)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [OPEN] [CMD READ]\r\n", vsync_tick);
            }
            if (now_fault_status_command_read & 0x4)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [SHORT] [CMD READ]\r\n", vsync_tick);
            }
            if (now_fault_status_command_read & 0x8)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [THERMAL] [CMD READ]\r\n", vsync_tick);
            }
        }
        prev_fault_status_command_read = now_fault_status_command_read;
    }

    //XD04_Detect_FBO();
#if 0
    static uint16_t prev_fbo = 0xFFFF;
    uint16_t now_fbo = XD_FBO_READ();

    if (prev_fbo != now_fbo)
    {
        if (now_fbo)
        {
            print(LOG_INFO, "\r\n [%u] FBO --> HI\r\n", vsync_tick);
        }
        else
        {
            print(LOG_INFO, "\r\n [%u] FBO --> LOW\r\n", vsync_tick);
        }
        prev_fbo = now_fbo;
    }
#endif
    ++vsync_tick;
}

/*** end of file ***/