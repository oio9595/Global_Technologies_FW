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

bool gb_xd_line_delay_sweep;

bool gb_xd_ldim_sweep;

bool gb_xd_scan_no;
uint8_t gn_xd_scan_no;

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
    if (!XDIC_Is_Vsync_Mode_External() && !IS_XC24_Support())
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
    static uint8_t prev_fault_status = 0xFF;
    static uint16_t vsync_tick = 0;

    uint8_t now_fault_status = (JigBD_IF_Fault_Read_Command() & 0x0F);

    if (now_fault_status != prev_fault_status)
    {
        if (!now_fault_status)
        {
            print(LOG_INFO, "\r\n [%u] XD FAULT None\r\n", vsync_tick);
        }
        else
        {
            char msg[64] = {0};
            int limit = sizeof(msg);
            int len = 0;

            len += snprintf(msg + len, limit - len, "\r\n [%u] XD FAULT Detected [ ", vsync_tick);

            if(now_fault_status & FAULT_MASK_FB)      { len += snprintf(msg + len, limit - len, "FB ");         }
            if(now_fault_status & FAULT_MASK_OPEN)    { len += snprintf(msg + len, limit - len, "OPEN ");       }
            if(now_fault_status & FAULT_MASK_SHORT)   { len += snprintf(msg + len, limit - len, "SHORT ");      }
            if(now_fault_status & FAULT_MASK_THERMAL) { len += snprintf(msg + len, limit - len, "THERMAL ");    }

            snprintf(msg + len, limit - len, "]\r\n");
            print(LOG_INFO, "%s", msg);
        }
        prev_fault_status = now_fault_status;
    }
    ++vsync_tick;
}

void XDIC_Vsync_Task(void)
{
    static uint8_t vsync_count = 0;
    static bool b_is_x1 = true;
    if (gb_xdic_vsync_flag)
    {
        if (IS_XC24_Support())
        {
            XC24_Turn_Off_Sync_Auto();
            us_delay(100);
            XC24_Turn_On_Sync_Auto();
        }

        us_delay(1500);
#if 0
        if (++vsync_count > 120)
        {
            if (b_is_x1) // go to x8
            {
                uint16_t r01_value = XDIC_Get_General_Reg(XDIC_ADDR_LD_CONTROL);
                r01_value &= ~(7U << 3); // set x8
                r01_value |= (3U << 3);
                XDIC_Write_General_Reg(XDIC_ADDR_LD_CONTROL, r01_value);
                gn_xdic_LD_out >>= 3;
                b_is_x1 = false;
            }
            else // go to x1
            {
                uint16_t r01_value = XDIC_Get_General_Reg(XDIC_ADDR_LD_CONTROL);
                r01_value &= ~(7U << 3); // set x8
                r01_value |= (0U << 3);
                XDIC_Write_General_Reg(XDIC_ADDR_LD_CONTROL, r01_value);
                gn_xdic_LD_out <<= 3;
                b_is_x1 = true;
            }
            vsync_count = 0;
        }
#endif
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
        if (gb_xd_line_delay_sweep)
        {
            static uint16_t linedelay = 1U;
            if (++vsync_count > 120U)
            {
                XDIC_Set_Sweep_Line_Delay(linedelay);
                linedelay <<= 1U;
                if (linedelay > 0x3FFF)
                {
                    linedelay = 1U;
                }
                vsync_count = 0;
            }
        }
        if (gb_xd_ldim_sweep)
        {
            gn_xdic_LD_out += 15;
            if (gn_xdic_LD_out > 0x3FFF)
            {
                gn_xdic_LD_out = 0;
            }
        }
#if 1
        if (gb_xd_scan_no)
        {
            XDIC_Set_Scan_No(gn_xd_scan_no);

            if (gn_xd_scan_no == 3U)
            {
                gn_xdic_LD_out >>= 3;
            }
            else
            {
                gn_xdic_LD_out <<= 3;
            }
            gb_xd_scan_no = false;
        }
#endif
        gb_xdic_vsync_flag = false;
    }
}

/*** end of file ***/