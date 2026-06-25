/** @file xd12.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XD12_C__

#include "main.h"
#include "xd12.h"
#include "JigBd_IF.h"
#include "types.h"
#include "config.h"

#define XD12_GENERAL_REG_ENTRY(addr, reg)   { addr, #addr, &gt_xd12_general_regs.reg }

#define XD12_REG_GENERAL            (0)
#define XD12_REG_MIRROR             (1)

#define XD12_CHANNEL_ENABLE_MAX     ((1U << XD12_CH_SIZE) - 1)

#define XD12_VREF_MAX               (0xFFF)
#define XD12_TRIM_OSC_MANUAL        (0x8000)
#define XD12_OFS_MAX_CURRENT_LVL    (XD12_DEV_MAX_CURR_LEVEL_8mA)
#define XD12_GAIN_MAX_CURRENT_LVL   (XD12_DEV_MAX_CURR_LEVEL_8mA)

static _xd12_general_regs_t gt_xd12_general_regs;

static _reg_map_t gt_xd12_general_maps[] =
{
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_RESET_ID         , _r00 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_LD_CONTROL       , _r01 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_FPWM_DIVIDER     , _r02 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_CHANNEL_ENABLE   , _r03 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_04         , _r04 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_FAULT_STATUS     , _r05 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_FAULT_LEVEL      , _r06 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_FAULT_CONTROL    , _r07 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_MAX_CURRENT_VREF , _r08 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_EXTEND_1, _r09 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_EXTEND_2, _r0A ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_01      , _r0B ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_02      , _r0C ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_03      , _r0D ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_04      , _r0E ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_05      , _r0F ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_06      , _r10 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_07      , _r11 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_08      , _r12 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_09      , _r13 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_10      , _r14 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_11      , _r15 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DELAY_CH_12      , _r16 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_17         , _r17 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_18         , _r18 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_19         , _r19 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_1A         , _r1A ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_1B         , _r1B ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_1C         , _r1C ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_1D         , _r1D ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_1E         , _r1E ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_1F         , _r1F ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_20         , _r20 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_21         , _r21 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_22         , _r22 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_23         , _r23 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_24         , _r24 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_SERIAL_BAUDRATE  , _r25 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_SERIAL_LATENCY   , _r26 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_MCLK_LOCK_1      , _r27 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_MCLK_LOCK_2      , _r28 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_TEMP             , _r29 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OSC_FLL_MANUAL_1 , _r2A ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OSC_FLL_MANUAL_2 , _r2B ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OSC_FLL_MONITOR  , _r2C ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_WR_PROTECT       , _r2D ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_2E         , _r2E ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_VREF_FIX         , _r2F ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_30         , _r30 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_31         , _r31 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_32         , _r32 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_33         , _r33 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_34         , _r34 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_35         , _r35 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_36         , _r36 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_37         , _r37 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_38         , _r38 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_DUMMY_39         , _r39 ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OTP_ACCESS_1     , _r3A ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OTP_ACCESS_2     , _r3B ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OTP_WRITE        , _r3C ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OTP_RD_PROG      , _r3D ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OTP_PROTECT      , _r3E ),
    XD12_GENERAL_REG_ENTRY( XD12_ADDR_OTP_OP_MODE      , _r3F ),
};
static_assert(XD12_ADDR_MAX == (sizeof(gt_xd12_general_maps) / sizeof(_reg_map_t)), "XD12 General Address map mismatch!");

/* Variable for XD Registers */
static xd12_dev_max_curr_level_t gt_xd12_dev_max_curr_level;
static xd12_short_level_t gt_xd12_short_level;
static xd12_fb_level_t gt_xd12_fb_level;

static const _reg_map_t* XD12_Get_General_Map_Pointer(uint8_t addr)
{
    for (uint8_t i = 0; i < sizeof(gt_xd12_general_maps) / sizeof(gt_xd12_general_maps[0]); ++i)
    {
        if (gt_xd12_general_maps[i].address == addr)
        {
            return &gt_xd12_general_maps[i];
        }
    }
    return NULL;
}

void XD12_Write_General_Reg(uint8_t addr, uint16_t data)
{
    const _reg_map_t* map = XD12_Get_General_Map_Pointer(addr);
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

uint16_t XD12_Read_General_Reg(uint8_t addr)
{
    uint16_t xd12_reg_val = 0xFFFF;
    const _reg_map_t* map = XD12_Get_General_Map_Pointer(addr);
    if (map)
    {
        xd12_reg_val = JigBD_IF_Read_Command(addr);
        *((uint16_t*)(map->reg_ptr)) = xd12_reg_val;
        print(LOG_DEBUG, "XD12 General Read --> [ 0x%02X - 0x%04X] \r\n", addr, xd12_reg_val);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }

    return xd12_reg_val;
}

uint16_t XD12_Get_General_Reg(uint8_t addr)
{
    uint16_t xd12_reg_val = 0xFFFF;
    const _reg_map_t* map = XD12_Get_General_Map_Pointer(addr);
    if (map)
    {
        xd12_reg_val = *((uint16_t*)(map->reg_ptr));
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
        return xd12_reg_val;
    }

    return xd12_reg_val;
}

void XD12_Dump_All_Registers(void)
{
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XD12_ADDR_MAX ; ++xd_general_addr)
    {
        const _reg_map_t* map = XD12_Get_General_Map_Pointer(xd_general_addr);
        if (map)
        {
            print(LOG_INFO, "[ %-40s 0x%02X | 0x%04X | %-6u ]\r\n", map->name, map->address, *((uint16_t*)(map->reg_ptr)), *((uint16_t*)(map->reg_ptr)));
        }
    }
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
}

void XD12_Read_All_Registers(void)
{
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XD12_ADDR_MAX ; ++xd_general_addr)
    {
        XD12_Read_General_Reg(xd_general_addr);
    }
    XD12_Dump_All_Registers();
}

void XD12_Param_Init(void)
{
    gt_xd12_dev_max_curr_level = XD12_DEV_MAX_CURR_LEVEL_8mA;
    gt_xd12_short_level = XD12_SHORT_LEVEL_24V;
    gt_xd12_fb_level = XD12_FB_LEVEL_0V4;
}

void XD12_Init(void)
{
    JigBD_IF_Select_Output_Ch(XDIC_CH_MAX);
    JigBD_IF_Change_Current_Gain(GAIN_HIGH);
    JigBD_IF_XD_VCC_Level(PWR_ON_5V0);
    JigBD_IF_XD_VCC_EN(PWR_ON);

    LL_mDelay(100);

    XD12_Param_Init();

    JigBD_IF_Reset_Command();
    JigBD_IF_IdGen_Command();

    for (xd12_addr_t xd12_addr = XD12_ADDR_RESET_ID ; xd12_addr < XD12_ADDR_MAX ; ++xd12_addr)
    {
        const _reg_map_t* map = XD12_Get_General_Map_Pointer(xd12_addr);
        if (map)
        {
            switch (xd12_addr)
            {
            case XD12_ADDR_CHANNEL_ENABLE :
                gt_xd12_general_regs._r03.val = XD12_CHANNEL_ENABLE_MAX;
                break;
            case XD12_ADDR_FAULT_LEVEL :
                gt_xd12_general_regs._r06.fb_level = gt_xd12_fb_level;
                gt_xd12_general_regs._r06.short_level = gt_xd12_short_level;
                gt_xd12_general_regs._r06.dev_max_curr_level = gt_xd12_dev_max_curr_level;
                break;
            case XD12_ADDR_FAULT_CONTROL :
                gt_xd12_general_regs._r07.timeout_en = 1;
                break;
            case XD12_ADDR_MAX_CURRENT_VREF :
                gt_xd12_general_regs._r08.max_curr_vref = XD12_VREF_MAX;
                break;
            case XD12_ADDR_SERIAL_BAUDRATE :
                gt_xd12_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
                gt_xd12_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
                break;
            case XD12_ADDR_WR_PROTECT :
                gt_xd12_general_regs._r2D.val = 0x155;
                break;
            default :
                continue;
            }
            XD12_Write_General_Reg(xd12_addr, *((uint16_t*)(map->reg_ptr)));
        }
        else
        {
            print(LOG_ERROR, "ERROR: Register 0x%02X not initialized (map missing)\r\n", xd12_addr);
        }
    }

    XD12_Read_All_Registers();
    JigBD_IF_VLED_9V_EN(PWR_ON);
}

/* ================================================================================================================================================= */
/* General Function */
/* ================================================================================================================================================= */

void XD12_Set_Max_Current_Level(xd12_dev_max_curr_level_t in_dev_max_curr)
{
    gt_xd12_general_regs._r06.dev_max_curr_level = in_dev_max_curr;
    XD12_Write_General_Reg(XD12_ADDR_FAULT_LEVEL, gt_xd12_general_regs._r06.val);
}

void XD12_Set_Max_Curr_Vref(uint16_t in_max_curr_vref)
{
    gt_xd12_general_regs._r08.max_curr_vref = in_max_curr_vref;
    XD12_Write_General_Reg(XD12_ADDR_MAX_CURRENT_VREF, gt_xd12_general_regs._r08.val);
}

void XD12_Set_VREF_CTL(uint16_t in_vref_ctl)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.ddio_dis = 1;
    gt_xd12_general_regs._r3F.test_ana_en = 2;
    gt_xd12_general_regs._r3F.pwm_full_o = 0;
    gt_xd12_general_regs._r3F.mclk32_o = 0;
    gt_xd12_general_regs._r3F.vref_o = 1;
    gt_xd12_general_regs._r3F.addr_ext = 1;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);

    JigBD_IF_Write_Command(0x02, in_vref_ctl);
}

static void XD12_Set_OSC_Manual_Enable(bool en)
{
    if (en == true)
    {
        gt_xd12_general_regs._r2B.osc_fll_man_e = 1;
    }
    else
    {
        gt_xd12_general_regs._r2B.osc_fll_man_e = 0;
    }
    XD12_Write_General_Reg(XD12_ADDR_OSC_FLL_MANUAL_2, gt_xd12_general_regs._r2B.val);
}

static void XD12_Set_OSC_Manual(uint16_t osc_manual)
{
    uint16_t osc_manual_lsb = ((osc_manual & 0x0FFF) >>  0);
    uint16_t osc_manual_msb = ((osc_manual & 0xF000) >> 12);

    gt_xd12_general_regs._r2A.osc_fll_man = osc_manual_lsb;
    gt_xd12_general_regs._r2B.osc_fll_man = osc_manual_msb;

    XD12_Write_General_Reg(XD12_ADDR_OSC_FLL_MANUAL_1, gt_xd12_general_regs._r2A.val);
    XD12_Write_General_Reg(XD12_ADDR_OSC_FLL_MANUAL_2, gt_xd12_general_regs._r2B.val);
}
/* ================================================================================================================================================= */
/* Trim Function */
/* ================================================================================================================================================= */

void XD12_Trim_Init_VREF_CTL(void)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.ddio_dis = 1;
    gt_xd12_general_regs._r3F.test_ana_en = 3;
    gt_xd12_general_regs._r3F.pwm_full_o = 0;
    gt_xd12_general_regs._r3F.mclk32_o = 0;
    gt_xd12_general_regs._r3F.vref_o = 1;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);

    gt_xd12_general_regs._r08.max_curr_vref = XD12_VREF_MAX;
    XD12_Write_General_Reg(XD12_ADDR_MAX_CURRENT_VREF, gt_xd12_general_regs._r08.val);
}

void XD12_Trim_Init_OSC(void)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.ddio_dis = 1;
    gt_xd12_general_regs._r3F.test_ana_en = 0;
    gt_xd12_general_regs._r3F.pwm_full_o = 0;
    gt_xd12_general_regs._r3F.mclk32_o = 1;
    gt_xd12_general_regs._r3F.vref_o = 0;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);

    XD12_Set_OSC_Manual_Enable(true);
    XD12_Set_OSC_Manual(XD12_TRIM_OSC_MANUAL);
}

void XD12_Trim_Init_OFS_CH(void)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.ddio_dis = 1;
    gt_xd12_general_regs._r3F.test_ana_en = 0;
    gt_xd12_general_regs._r3F.pwm_full_o = 1;
    gt_xd12_general_regs._r3F.mclk32_o = 0;
    gt_xd12_general_regs._r3F.vref_o = 0;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);

    XD12_Set_Max_Current_Level((xd12_dev_max_curr_level_t)XD12_OFS_MAX_CURRENT_LVL);
}

void XD12_Trim_Init_GAIN_CH(void)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.ddio_dis = 1;
    gt_xd12_general_regs._r3F.test_ana_en = 0;
    gt_xd12_general_regs._r3F.pwm_full_o = 1;
    gt_xd12_general_regs._r3F.mclk32_o = 0;
    gt_xd12_general_regs._r3F.vref_o = 0;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);

    XD12_Set_Max_Current_Level((xd12_dev_max_curr_level_t)XD12_GAIN_MAX_CURRENT_LVL);
}

void XD12_Trim_Init_ICC(void)
{
    gt_xd12_general_regs._r3F.test_en = 0;
    gt_xd12_general_regs._r3F.ddio_dis = 0;
    gt_xd12_general_regs._r3F.test_ana_en = 0;
    gt_xd12_general_regs._r3F.pwm_full_o = 0;
    gt_xd12_general_regs._r3F.mclk32_o = 0;
    gt_xd12_general_regs._r3F.vref_o = 0;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);
}

void XD12_Trim_Init_LDO_CTL(void)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.ddio_dis = 1;
    gt_xd12_general_regs._r3F.test_ana_en = 2;
    gt_xd12_general_regs._r3F.pwm_full_o = 0;
    gt_xd12_general_regs._r3F.mclk32_o = 0;
    gt_xd12_general_regs._r3F.vref_o = 1;
    gt_xd12_general_regs._r3F.addr_ext = 1;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);

    XD12_Write_General_Reg(XD12_ADDR_OTP_PROTECT, 0xA5A);
}