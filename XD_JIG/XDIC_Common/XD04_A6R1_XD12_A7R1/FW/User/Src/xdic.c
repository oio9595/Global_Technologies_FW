/** @file xdic.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XDIC_C__

#include "main.h"
#include "xd04.h"
#include "xd12.h"
#include "xdic.h"
#include "config.h"

volatile bool gb_xdic_type_is_xd04; //true: XD04, false: XD12.
uint8_t gn_xdic_channel_size;

void XDIC_Detect_Type(void)
{
    uint32_t xd_type = LL_GPIO_IsInputPinSet(XD_SELECT_GPIO_Port, XD_SELECT_Pin);
    if (xd_type == 0)
    {
        gb_xdic_type_is_xd04 = false; // XD12
        gn_xdic_channel_size = (XDIC_CH_12 + 1);
        print(LOG_INFO, "XDIC Type Detected : XD12\r\n");
    }
    else
    {
        gb_xdic_type_is_xd04 = true; // XD04
        gn_xdic_channel_size = (XDIC_CH_04 + 1);
        print(LOG_INFO, "XDIC Type Detected : XD04\r\n");
    }
}

void XDIC_Write_General_Reg(uint8_t addr, uint16_t data)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Write_General_Reg(addr, data);
    }
    else
    {
        XD12_Write_General_Reg(addr, data);
    }
}

uint16_t XDIC_Read_General_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    if (gb_xdic_type_is_xd04)
    {
        xdic_reg_val = XD04_Read_General_Reg(addr);
    }
    else
    {
        xdic_reg_val = XD12_Read_General_Reg(addr);
    }
    return xdic_reg_val;
}

uint16_t XDIC_Get_General_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    if (gb_xdic_type_is_xd04)
    {
        xdic_reg_val = XD04_Get_General_Reg(addr);
    }
    else
    {
        xdic_reg_val = XD12_Get_General_Reg(addr);
    }
    return xdic_reg_val;
}

static void XDIC_Dump_All_Registers(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Dump_All_Registers();
    }
    else
    {
        XD12_Dump_All_Registers();
    }
}

void XDIC_Read_All_Registers(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Read_All_Registers();
    }
    else
    {
        XD12_Read_All_Registers();
    }
}

void XDIC_Init(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Init();
    }
    else
    {
        XD12_Init();
    }
}

/* ================================================================================================================================================= */
/* General Function */
/* ================================================================================================================================================= */

void XDIC_Set_Max_Current_Level(uint8_t in_dev_max_curr)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Set_Max_Current_Level((xd04_dev_max_curr_level_t)in_dev_max_curr);
    }
    else
    {
        XD12_Set_Max_Current_Level((xd12_dev_max_curr_level_t)in_dev_max_curr);
    }
}

void XDIC_Set_Max_Curr_Vref(uint16_t in_max_curr_vref)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Set_Max_Curr_Vref(in_max_curr_vref);
    }
    else
    {
        XD12_Set_Max_Curr_Vref(in_max_curr_vref);
    }
}

void XDIC_Set_VREF_CTL(uint16_t in_vref_ctl)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Set_VREF_CTL(in_vref_ctl);
    }
    else
    {
        XD12_Set_VREF_CTL(in_vref_ctl);
    }
}
/* ================================================================================================================================================= */
/* Trim Function */
/* ================================================================================================================================================= */

void XDIC_Trim_Init_VREF_CTL(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Trim_Init_VREF_CTL();
    }
    else
    {
        XD12_Trim_Init_VREF_CTL();
    }
}

void XDIC_Trim_Init_OSC(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Trim_Init_OSC();
    }
    else
    {
        XD12_Trim_Init_OSC();
    }
}

void XDIC_Trim_Init_Current_Type_A(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Trim_Init_ICTL_L_CH();
    }
    else
    {
        XD12_Trim_Init_GAIN_CH();
    }
}

void XDIC_Trim_Init_Current_Type_B(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Trim_Init_ICTL_H_CH();
    }
    else
    {
        XD12_Trim_Init_OFS_CH();
    }
}

void XDIC_Trim_Init_ICC(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Trim_Init_ICC();
    }
    else
    {
        XD12_Trim_Init_ICC();
    }
}

void XDIC_Trim_Init_LDO_CTL(void)
{
    if (gb_xdic_type_is_xd04)
    {
        XD04_Trim_Init_LDO_CTL();
    }
    else
    {
        XD12_Trim_Init_LDO_CTL();
    }
}