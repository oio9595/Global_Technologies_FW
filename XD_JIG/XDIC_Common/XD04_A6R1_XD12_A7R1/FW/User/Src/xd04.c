/** @file xd04.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XD04_C__

#include "main.h"
#include "xd04.h"
#include "JigBd_IF.h"
#include "types.h"
#include "config.h"

#define XD04_GENERAL_REG_ENTRY(addr, reg)   { addr, #addr, &gt_xd04_general_regs.reg }

#define XD04_REG_GENERAL                    (0)
#define XD04_REG_MIRROR                     (1)

#define XD04_CHANNEL_ENABLE_MAX             ((1U << XD04_CH_SIZE) - 1)

#define XD04_VREF_MAX                       (0xFFF)
#define XD04_TRIM_OSC_MANUAL                (0x8000)
#define XD04_ICTL_L_MAX_CURRENT_LVL         (XD04_DEV_MAX_CURR_LEVEL_24mA)
#define XD04_ICTL_H_MAX_CURRENT_LVL         (XD04_DEV_MAX_CURR_LEVEL_128mA)

static _xd04_general_regs_t gt_xd04_general_regs;

static _reg_map_t gt_xd04_general_maps[] =
{
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_RESET_ID        , _r00 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_LD_CONTROL      , _r01 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_FPWM_DIVIDER    , _r02 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_CHANNEL_ENABLE  , _r03 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_04        , _r04 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_FAULT_STATUS    , _r05 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_FAULT_LEVEL     , _r06 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_FAULT_CONTROL   , _r07 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_MAX_CURRENT_VREF, _r08 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DELAY_CH_EXTEND , _r09 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_0A        , _r0A ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DELAY_CH_01     , _r0B ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DELAY_CH_02     , _r0C ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DELAY_CH_03     , _r0D ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DELAY_CH_04     , _r0E ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_0F        , _r0F ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_10        , _r10 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_11        , _r11 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_12        , _r12 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_13        , _r13 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_14        , _r14 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_15        , _r15 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_16        , _r16 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_17        , _r17 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_18        , _r18 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_19        , _r19 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_1A        , _r1A ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_1B        , _r1B ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_1C        , _r1C ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_1D        , _r1D ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_1E        , _r1E ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_1F        , _r1F ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_20        , _r20 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_21        , _r21 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_22        , _r22 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_23        , _r23 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_24        , _r24 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_SERIAL_BAUDRATE , _r25 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_SERIAL_LATENCY  , _r26 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_MCLK_LOCK_1     , _r27 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_MCLK_LOCK_2     , _r28 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_TEMP            , _r29 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OSC_FLL_MANUAL_1, _r2A ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OSC_FLL_MANUAL_2, _r2B ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OSC_FLL_MONITOR , _r2C ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_WR_PROTECT      , _r2D ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_2E        , _r2E ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_VREF_FIX        , _r2F ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_30        , _r30 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_31        , _r31 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_32        , _r32 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_33        , _r33 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_34        , _r34 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_35        , _r35 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_36        , _r36 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_37        , _r37 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_38        , _r38 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_DUMMY_39        , _r39 ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OTP_ACCESS_1    , _r3A ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OTP_ACCESS_2    , _r3B ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OTP_WRITE       , _r3C ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OTP_RD_PROG     , _r3D ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OTP_PROTECT     , _r3E ),
    XD04_GENERAL_REG_ENTRY( XD04_ADDR_OTP_OP_MODE     , _r3F ),
};
static_assert(XD04_ADDR_MAX == (sizeof(gt_xd04_general_maps) / sizeof(_reg_map_t)), "XD04 General Address map mismatch!");

/* Variable for XD Registers */
static xd04_dev_max_curr_level_t gt_xd04_dev_max_curr_level;
static xd04_short_level_t gt_xd04_short_level;
static xd04_fb_level_t gt_xd04_fb_level;

static const _reg_map_t* XD04_Get_General_Map_Pointer(uint8_t addr)
{
    for (uint8_t i = 0; i < sizeof(gt_xd04_general_maps) / sizeof(gt_xd04_general_maps[0]); ++i)
    {
        if (gt_xd04_general_maps[i].address == addr)
        {
            return &gt_xd04_general_maps[i];
        }
    }
    return NULL;
}

void XD04_Write_General_Reg(uint8_t addr, uint16_t data)
{
    const _reg_map_t* map = XD04_Get_General_Map_Pointer(addr);
    if (map)
    {
        *((uint16_t*)(map->reg_ptr)) = data;
        JigBD_IF_Write_Command(addr, data);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }
}

uint16_t XD04_Read_General_Reg(uint8_t addr)
{
    uint16_t xd04_reg_val = 0xFFFF;
    const _reg_map_t* map = XD04_Get_General_Map_Pointer(addr);
    if (map)
    {
        xd04_reg_val = JigBD_IF_Read_Command(addr);
        *((uint16_t*)(map->reg_ptr)) = xd04_reg_val;
        print(LOG_DEBUG, "XD04 General Read --> [ 0x%02X - 0x%04X] \r\n", addr, xd04_reg_val);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }

    return xd04_reg_val;
}

uint16_t XD04_Get_General_Reg(uint8_t addr)
{
    uint16_t xd04_reg_val = 0xFFFF;
    const _reg_map_t* map = XD04_Get_General_Map_Pointer(addr);
    if (map)
    {
        xd04_reg_val = *((uint16_t*)(map->reg_ptr));
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
        return xd04_reg_val;
    }

    return xd04_reg_val;
}

void XD04_Dump_All_Registers(void)
{
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XD04_ADDR_MAX ; ++xd_general_addr)
    {
        const _reg_map_t* map = XD04_Get_General_Map_Pointer(xd_general_addr);
        if (map)
        {
            print(LOG_INFO, "[ %-40s 0x%02X | 0x%04X | %-6u ]\r\n", map->name, map->address, *((uint16_t*)(map->reg_ptr)), *((uint16_t*)(map->reg_ptr)));
        }
    }
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
}

void XD04_Read_All_Registers(void)
{
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XD04_ADDR_MAX ; ++xd_general_addr)
    {
        XD04_Read_General_Reg(xd_general_addr);
    }
    XD04_Dump_All_Registers();
}

void XD04_Param_Init(void)
{
    gt_xd04_dev_max_curr_level = XD04_DEV_MAX_CURR_LEVEL_128mA;
    gt_xd04_short_level = XD04_SHORT_LEVEL_6V;
    gt_xd04_fb_level = XD04_FB_LEVEL_0V4;
}

void XD04_Init(void)
{
    JigBD_IF_Select_Output_Ch(XDIC_CH_MAX);
    JigBD_IF_Change_Current_Gain(GAIN_HIGH);
    JigBD_IF_XD_VCC_Level(PWR_ON_5V0);
    JigBD_IF_XD_VCC_EN(PWR_ON);

    LL_mDelay(100);

    XD04_Param_Init();

    //JigBD_IF_Reset_Command();
    JigBD_IF_IdGen_Command();

    for (xd04_addr_t xd04_addr = XD04_ADDR_RESET_ID ; xd04_addr < XD04_ADDR_MAX ; ++xd04_addr)
    {
        const _reg_map_t* map = XD04_Get_General_Map_Pointer(xd04_addr);
        if (map)
        {
            switch (xd04_addr)
            {
            case XD04_ADDR_CHANNEL_ENABLE :
                gt_xd04_general_regs._r03.val = XD04_CHANNEL_ENABLE_MAX;
                break;
            case XD04_ADDR_FAULT_LEVEL :
                gt_xd04_general_regs._r06.fb_level = gt_xd04_fb_level;
                gt_xd04_general_regs._r06.short_level = gt_xd04_short_level;
                gt_xd04_general_regs._r06.dev_max_curr_level = gt_xd04_dev_max_curr_level;
                break;
            case XD04_ADDR_FAULT_CONTROL :
                gt_xd04_general_regs._r07.timeout_en = 1;
                break;
            case XD04_ADDR_MAX_CURRENT_VREF :
                gt_xd04_general_regs._r08.max_curr_vref = XD04_VREF_MAX;
                break;
            case XD04_ADDR_SERIAL_BAUDRATE :
                gt_xd04_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
                gt_xd04_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
                break;
            case XD04_ADDR_SERIAL_LATENCY :
                gt_xd04_general_regs._r26.serial_latency = 60;
                break;
            default :
                continue;
            }
            XD04_Write_General_Reg(xd04_addr, *((uint16_t*)(map->reg_ptr)));
        }
        else
        {
            print(LOG_ERROR, "ERROR: Register 0x%02X not initialized (map missing)\r\n", xd04_addr);
        }
    }

    XD04_Read_All_Registers();
    JigBD_IF_VLED_9V_EN(PWR_ON);
}

/* ================================================================================================================================================= */
/* General Function */
/* ================================================================================================================================================= */

void XD04_Set_Max_Current_Level(xd04_dev_max_curr_level_t in_dev_max_curr)
{
    gt_xd04_general_regs._r06.dev_max_curr_level = in_dev_max_curr;
    XD04_Write_General_Reg(XD04_ADDR_FAULT_LEVEL, gt_xd04_general_regs._r06.val);
}

void XD04_Set_Max_Curr_Vref(uint16_t in_max_curr_vref)
{
    gt_xd04_general_regs._r08.max_curr_vref = in_max_curr_vref;
    XD04_Write_General_Reg(XD04_ADDR_MAX_CURRENT_VREF, gt_xd04_general_regs._r08.val);
}

static void XD04_Set_OSC_Manual_Enable(bool en)
{
    if (en == true)
    {
        gt_xd04_general_regs._r2B.osc_fll_man_e = 1;
    }
    else
    {
        gt_xd04_general_regs._r2B.osc_fll_man_e = 0;
    }
    XD04_Write_General_Reg(XD04_ADDR_OSC_FLL_MANUAL_2, gt_xd04_general_regs._r2B.val);
}

static void XD04_Set_OSC_Manual(uint16_t osc_manual)
{
    uint16_t osc_manual_lsb = ((osc_manual & 0x0FFF) >>  0);
    uint16_t osc_manual_msb = ((osc_manual & 0xF000) >> 12);

    gt_xd04_general_regs._r2A.osc_fll_man = osc_manual_lsb;
    gt_xd04_general_regs._r2B.osc_fll_man = osc_manual_msb;

    XD04_Write_General_Reg(XD04_ADDR_OSC_FLL_MANUAL_1, gt_xd04_general_regs._r2A.val);
    XD04_Write_General_Reg(XD04_ADDR_OSC_FLL_MANUAL_2, gt_xd04_general_regs._r2B.val);
}
/* ================================================================================================================================================= */
/* Trim Function */
/* ================================================================================================================================================= */

void XD04_Trim_Init_VREF_CTL(void)
{
    gt_xd04_general_regs._r3F.test_en = 1;
    gt_xd04_general_regs._r3F.ddio_dis = 1;
    gt_xd04_general_regs._r3F.test_ana_en = 3;
    gt_xd04_general_regs._r3F.pwm_full_o = 0;
    gt_xd04_general_regs._r3F.mclk32_o = 0;
    gt_xd04_general_regs._r3F.vref_o = 1;
    XD04_Write_General_Reg(XD04_ADDR_OTP_OP_MODE, gt_xd04_general_regs._r3F.val);

    gt_xd04_general_regs._r08.max_curr_vref = XD04_VREF_MAX;
    XD04_Write_General_Reg(XD04_ADDR_MAX_CURRENT_VREF, gt_xd04_general_regs._r08.val);
}

void XD04_Trim_Init_OSC(void)
{
    gt_xd04_general_regs._r3F.test_en = 1;
    gt_xd04_general_regs._r3F.ddio_dis = 1;
    gt_xd04_general_regs._r3F.test_ana_en = 0;
    gt_xd04_general_regs._r3F.pwm_full_o = 0;
    gt_xd04_general_regs._r3F.mclk32_o = 1;
    gt_xd04_general_regs._r3F.vref_o = 0;
    XD04_Write_General_Reg(XD04_ADDR_OTP_OP_MODE, gt_xd04_general_regs._r3F.val);

    XD04_Set_OSC_Manual_Enable(true);
    XD04_Set_OSC_Manual(XD04_TRIM_OSC_MANUAL);
}

void XD04_Trim_Init_ICTL_L_CH(void)
{
    gt_xd04_general_regs._r3F.test_en = 1;
    gt_xd04_general_regs._r3F.ddio_dis = 1;
    gt_xd04_general_regs._r3F.test_ana_en = 0;
    gt_xd04_general_regs._r3F.pwm_full_o = 1;
    gt_xd04_general_regs._r3F.mclk32_o = 0;
    gt_xd04_general_regs._r3F.vref_o = 0;
    XD04_Write_General_Reg(XD04_ADDR_OTP_OP_MODE, gt_xd04_general_regs._r3F.val);

    XD04_Set_Max_Current_Level((xd04_dev_max_curr_level_t)XD04_ICTL_L_MAX_CURRENT_LVL);
}

void XD04_Trim_Init_ICTL_H_CH(void)
{
    gt_xd04_general_regs._r3F.test_en = 1;
    gt_xd04_general_regs._r3F.ddio_dis = 1;
    gt_xd04_general_regs._r3F.test_ana_en = 0;
    gt_xd04_general_regs._r3F.pwm_full_o = 1;
    gt_xd04_general_regs._r3F.mclk32_o = 0;
    gt_xd04_general_regs._r3F.vref_o = 0;
    XD04_Write_General_Reg(XD04_ADDR_OTP_OP_MODE, gt_xd04_general_regs._r3F.val);

    XD04_Set_Max_Current_Level((xd04_dev_max_curr_level_t)XD04_ICTL_H_MAX_CURRENT_LVL);
}

void XD04_Trim_Init_ICC(void)
{
    gt_xd04_general_regs._r3F.test_en = 0;
    gt_xd04_general_regs._r3F.ddio_dis = 0;
    gt_xd04_general_regs._r3F.test_ana_en = 0;
    gt_xd04_general_regs._r3F.pwm_full_o = 0;
    gt_xd04_general_regs._r3F.mclk32_o = 0;
    gt_xd04_general_regs._r3F.vref_o = 0;
    XD04_Write_General_Reg(XD04_ADDR_OTP_OP_MODE, gt_xd04_general_regs._r3F.val);
}