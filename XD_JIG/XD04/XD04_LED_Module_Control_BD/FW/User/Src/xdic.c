/** @file xdic.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XDIC_C__

#include "main.h"
#include "xdic.h"
#include "xc24.h"
#include "config.h"

#define XDIC_GENERAL_REG_ENTRY(addr, reg)   { addr, #addr, &gt_xdic_general_regs.reg }

#define XD_LD_DIR_HEAD_SHIFT        (0)
#define XD_LD_DIR_TAIL_SHIFT        (1)

#define XD_PWM_RES_12BIT            (0)
#define XD_PWM_RES_14BIT            (1)

#define XD_IO_MODE_NOP              (0)
#define XD_IO_MODE_EXT_VSYNC        (1)
#define XD_IO_MODE_FBO              (2)
#define XD_IO_MODE_EXT_VYI_FBO      (3)

#define XD_MCLK_FLL_ENABLE          (0)
#define XD_MCLK_FLL_DISABLE         (1)

#define MCLK_LSB_MASK               (0x00FFF) //LSB 12-bit
#define MCLK_MSB_MASK               (0xFF000) //MSB  8-bit

#define XDIC_RESET_VALUE            (1U << 11)
#define XDIC_CHANNEL_ENABLE_MAX     ((1U << XD_CH_SIZE) - 1)

static _xdic_general_regs_t gt_xdic_general_regs;

static _reg_map_t gt_xdic_general_maps[] =
{
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_RESET_ID        , _r00 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_LD_CONTROL      , _r01 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FPWM_DIVIDER    , _r02 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_CHANNEL_ENABLE  , _r03 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_04        , _r04 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FAULT_STATUS    , _r05 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FAULT_LEVEL     , _r06 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FAULT_CONTROL   , _r07 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MAX_CURRENT_VREF, _r08 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_EXTEND , _r09 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_0A        , _r0A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_01     , _r0B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_02     , _r0C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_03     , _r0D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_04     , _r0E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_0F        , _r0F ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_10        , _r10 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_11        , _r11 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_12        , _r12 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_13        , _r13 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_14        , _r14 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_15        , _r15 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_16        , _r16 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_17        , _r17 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_18        , _r18 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_19        , _r19 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1A        , _r1A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1B        , _r1B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1C        , _r1C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1D        , _r1D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1E        , _r1E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1F        , _r1F ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_20        , _r20 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_21        , _r21 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_22        , _r22 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_23        , _r23 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_24        , _r24 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SERIAL_BAUDRATE , _r25 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SERIAL_LATENCY  , _r26 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MCLK_LOCK_1     , _r27 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MCLK_LOCK_2     , _r28 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_TEMP            , _r29 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OSC_FLL_MANUAL_1, _r2A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OSC_FLL_MANUAL_2, _r2B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OSC_FLL_MONITOR , _r2C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_WR_PROTECT      , _r2D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_2E        , _r2E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_VREF_FIX        , _r2F ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_30        , _r30 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_31        , _r31 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_32        , _r32 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_33        , _r33 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_34        , _r34 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_35        , _r35 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_36        , _r36 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_37        , _r37 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_38        , _r38 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_39        , _r39 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_ACCESS_1    , _r3A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_ACCESS_2    , _r3B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_WRITE       , _r3C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_RD_PROG     , _r3D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_PROTECT     , _r3E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_OP_MODE     , _r3F ),
};
static_assert(XDIC_ADDR_MAX == (sizeof(gt_xdic_general_maps) / sizeof(_reg_map_t)), "XDIC General Address map mismatch!");

/* Variable for XD Registers */
static float gf_xd_mclk;
static float gf_vsync_out;

static uint16_t gn_xd_pwm_res;
static uint16_t gn_xd_scan_no;
static uint16_t gn_xd_fpwm_div;
static uint32_t gn_xd_mclk_lock_cnt;

static dev_max_curr_level_t gt_xd_dev_max_curr_level;
static short_level_t gt_xd_short_level;
static fb_level_t gt_xd_fb_level;

float gf_xd_max_current;

static const _reg_map_t* XDIC_Get_General_Map_Pointer(uint8_t addr)
{
    for (uint8_t i = 0 ; i < (sizeof(gt_xdic_general_maps) / sizeof(gt_xdic_general_maps[0])) ; ++i)
    {
        if (gt_xdic_general_maps[i].address == addr)
        {
            return &gt_xdic_general_maps[i];
        }
    }
    return NULL;
}

void XDIC_Write_General_Reg(uint8_t addr, uint16_t data)
{
    const _reg_map_t* map = XDIC_Get_General_Map_Pointer(addr);
    if (map)
    {
        *((uint16_t*)(map->reg_ptr)) = data;
        XC24_IF_Write_XDIC(addr, data);
    }
    else
    {
        print(LOG_PC, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }
}

uint16_t XDIC_Read_General_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    const _reg_map_t* map = XDIC_Get_General_Map_Pointer(addr);
    if (map)
    {
        xdic_reg_val = XC24_IF_Read_XDIC(addr);
        *((uint16_t*)(map->reg_ptr)) = xdic_reg_val;
        print(LOG_PC, "XDIC General Read --> [ 0x%02X - 0x%04X] \r\n", addr, xdic_reg_val);
    }
    else
    {
        print(LOG_PC, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }

    return xdic_reg_val;
}

static void XDIC_Dump_All_Registers(void)
{
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XDIC_ADDR_MAX ; ++xd_general_addr)
    {
        const _reg_map_t* map = XDIC_Get_General_Map_Pointer(xd_general_addr);
        if (map)
        {
            uint16_t value = *((uint16_t*)(map->reg_ptr));
            print(LOG_PC, "[%s (0x%02X)]\r\n\t VALUE : %s(0x%04X / %u)%s\r\n\r\n", map->name, map->address, ANSI_FONT_MAGENTA, value, value, ANSI_FONT_NONE);
        }
    }
}

void XDIC_Read_All_Registers(void)
{
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XDIC_ADDR_MAX ; ++xd_general_addr)
    {
        XDIC_Read_General_Reg(xd_general_addr);
    }

    XDIC_Dump_All_Registers();
}

void XDIC_Update_Max_Current_Vref(float in_current, bool low_current_mode)
{
    uint16_t max_curr_vref = 0;
    if (low_current_mode)
    {
        max_curr_vref = 1;
    }
    else
    {
        max_curr_vref = (uint16_t)(in_current * 4095.0f / 128.0f + 0.5f);
        if (max_curr_vref > 4095)
        {
            max_curr_vref = 4095;
        }
    }
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF, max_curr_vref);
}

void XDIC_Param_Init(void)
{
    gf_xd_mclk = XD_MCLK;
    gf_vsync_out = VSYNC;

    gn_xd_pwm_res = XD_PWM_RES_14BIT;
    gn_xd_scan_no = 0;

    if (gn_xd_pwm_res == XD_PWM_RES_12BIT)
    {
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 12)));
    }
    else //if (gn_xd_pwm_res == XD_PWM_RES_14BIT)
    {
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 14)));
    }

    gn_xd_mclk_lock_cnt = (uint32_t)((gf_xd_mclk / gf_vsync_out + 0.5f) * (1 + 0.1f / 100));

    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_128mA;
    gt_xd_short_level = SHORT_LEVEL_6V;
    gt_xd_fb_level = FB_LEVEL_0V4;

    gf_xd_max_current = 0.0f;
}

void XDIC_Init(void)
{
    XDIC_VCC_ON();
    LL_mDelay(100);

    XDIC_Param_Init();

    XDIC_Write_General_Reg(XDIC_ADDR_RESET_ID, XDIC_RESET_VALUE);
    XC24_IF_IdGen_Command();

    for (xdic_addr_t xdic_addr = XDIC_ADDR_RESET_ID ; xdic_addr < XDIC_ADDR_MAX ; ++xdic_addr)
    {
        const _reg_map_t* map = XDIC_Get_General_Map_Pointer(xdic_addr);
        if (map)
        {
            switch (xdic_addr)
            {
            case XDIC_ADDR_LD_CONTROL :
                gt_xdic_general_regs._r01.ld_dir = XD_LD_DIR_TAIL_SHIFT;
                gt_xdic_general_regs._r01.pwm_res = gn_xd_pwm_res;
                gt_xdic_general_regs._r01.over_to_e = 1;
                gt_xdic_general_regs._r01.scan_no = gn_xd_scan_no;
                gt_xdic_general_regs._r01.io_mode = XD_IO_MODE_NOP;
                gt_xdic_general_regs._r01.ld_size = XD_CH_SIZE;
                break;
            case XDIC_ADDR_FPWM_DIVIDER :
                gt_xdic_general_regs._r02.fpwm_div = gn_xd_fpwm_div;
                break;
            case XDIC_ADDR_CHANNEL_ENABLE :
                gt_xdic_general_regs._r03.val = XDIC_CHANNEL_ENABLE_MAX;
                break;
            case XDIC_ADDR_FAULT_LEVEL :
                gt_xdic_general_regs._r06.fb_level = gt_xd_fb_level;
                gt_xdic_general_regs._r06.short_level = gt_xd_short_level;
                gt_xdic_general_regs._r06.dev_max_curr_level = gt_xd_dev_max_curr_level;
                break;
            case XDIC_ADDR_FAULT_CONTROL :
                gt_xdic_general_regs._r07.timeout_en = 1;
                break;
            case XDIC_ADDR_MAX_CURRENT_VREF :
                gt_xdic_general_regs._r08.max_curr_vref = 0x00;
                break;
            case XDIC_ADDR_SERIAL_BAUDRATE :
                gt_xdic_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
                gt_xdic_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
                break;
            case XDIC_ADDR_SERIAL_LATENCY :
                gt_xdic_general_regs._r26.serial_latency = 60;
                break;
            case XDIC_ADDR_MCLK_LOCK_1 :
                gt_xdic_general_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & MCLK_LSB_MASK) >>  0);
                break;
            case XDIC_ADDR_MCLK_LOCK_2 :
                gt_xdic_general_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & MCLK_MSB_MASK) >> 12);
                gt_xdic_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_ENABLE;
                break;
            case XDIC_ADDR_OSC_FLL_MANUAL_2 :
                gt_xdic_general_regs._r2B.osc_fll_man_e = 0;
                break;
            case XDIC_ADDR_WR_PROTECT :
                gt_xdic_general_regs._r2D.val = 0x155;
                break;
            default :
                continue;
            }
            XDIC_Write_General_Reg(xdic_addr, *((uint16_t*)(map->reg_ptr)));
        }
        else
        {
            print(LOG_PC, "ERROR: Register 0x%02X not initialized (map missing)\r\n", xdic_addr);
        }
    }
}

void XDIC_DeInit(void)
{
    XDIC_Write_General_Reg(XDIC_ADDR_RESET_ID, (1 << 11)); // Reset
    XDIC_VCC_OFF();

    gf_xd_max_current = 0.0f;
}