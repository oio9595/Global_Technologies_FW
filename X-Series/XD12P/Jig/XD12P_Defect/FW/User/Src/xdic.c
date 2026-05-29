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
#include "JigBd_IF.h"
#include "types.h"
#include "config.h"
#include "xd_trim.h"
#include "ads124s08.h"

#define XDIC_GENERAL_REG_ENTRY(addr, reg)   { addr, #addr, &gt_xdic_general_regs.reg }
#define XDIC_MIRROR_REG_ENTRY(addr, reg)    { addr, #addr, &gt_xdic_mirror_regs.reg }

#define XDIC_REG_GENERAL            (0)
#define XDIC_REG_MIRROR             (1)

#define XDIC_OTP_PROTECT_DISABLE    (0xA5A)
#define XDIC_OTP_PROTECT_ENABLE     (0x5A5)

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

#define XD_MCLK_LSB_MASK            (0x00FFF) //LSB 12-bit
#define XD_MCLK_MSB_MASK            (0xFF000) //MSB  8-bit

#define XDIC_CHANNEL_ENABLE_MAX     ((1U << XD_CH_SIZE) - 1)

#define XDIC_TRIM_OSC_MANUAL        (32768)

#define XDIC_BGR_TC                 (0x03)
#define XDIC_OFS_RNG                (0x03)
#define XDIC_OSC_RESERVED           ((XDIC_BGR_TC << 2) | (XDIC_OFS_RNG << 0))

#define XD_TGT_VREF                 (2.2f)
#define XD_TGT_LDO                  (1.5f)
#define XD_TGT_OSC                  (39.3192f)
#define XD_TGT_OFS                  (2.2f)
#define XD_TGT_GAIN                 (2.2f)
#define XD_TGT_ERR_RATE             (10.0f)

volatile bool gb_xdic_initial_failed;

static _xdic_general_regs_t gt_xdic_general_regs;
static _xdic_mirror_regs_t gt_xdic_mirror_regs;

const static _reg_map_t gt_xdic_general_maps[] =
{
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_RESET_ID         , _r00 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_LD_CONTROL       , _r01 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FPWM_DIVIDER     , _r02 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_CHANNEL_ENABLE   , _r03 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_04         , _r04 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FAULT_STATUS     , _r05 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FAULT_LEVEL      , _r06 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FAULT_CONTROL    , _r07 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MAX_CURRENT_VREF , _r08 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_EXTEND_1, _r09 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_EXTEND_2, _r0A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_01      , _r0B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_02      , _r0C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_03      , _r0D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_04      , _r0E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_05      , _r0F ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_06      , _r10 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_07      , _r11 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_08      , _r12 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_09      , _r13 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_10      , _r14 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_11      , _r15 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_12      , _r16 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_17         , _r17 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_18         , _r18 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_19         , _r19 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1A         , _r1A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1B         , _r1B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1C         , _r1C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1D         , _r1D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1E         , _r1E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1F         , _r1F ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_20         , _r20 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_21         , _r21 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_22         , _r22 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_23         , _r23 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_24         , _r24 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SERIAL_BAUDRATE  , _r25 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SERIAL_LATENCY   , _r26 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MCLK_LOCK_1      , _r27 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MCLK_LOCK_2      , _r28 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_TEMP             , _r29 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OSC_FLL_MANUAL_1 , _r2A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OSC_FLL_MANUAL_2 , _r2B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OSC_FLL_MONITOR  , _r2C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_WR_PROTECT       , _r2D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_2E         , _r2E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_VREF_FIX         , _r2F ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_30         , _r30 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_31         , _r31 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_32         , _r32 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_33         , _r33 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_34         , _r34 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_35         , _r35 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_36         , _r36 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_37         , _r37 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_38         , _r38 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_39         , _r39 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_ACCESS_1     , _r3A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_ACCESS_2     , _r3B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_WRITE        , _r3C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_RD_PROG      , _r3D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_PROTECT      , _r3E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OTP_OP_MODE      , _r3F ),
};
_Static_assert(XDIC_ADDR_MAX == (sizeof(gt_xdic_general_maps) / sizeof(_reg_map_t)), "XDIC General Address map mismatch!");

const static _reg_map_t gt_xdic_mirror_maps[] =
{
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OTP_CRC     , _r00 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OSC         , _r01 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_VREF_CTL    , _r02 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_01   , _r03 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_02   , _r04 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_03   , _r05 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_04   , _r06 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_05   , _r07 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_06   , _r08 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_07   , _r09 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_08   , _r0A ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_09   , _r0B ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_10   , _r0C ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_11   , _r0D ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_12   , _r0E ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_0F    , _r0F ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_10    , _r10 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_11    , _r11 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_12    , _r12 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_13    , _r13 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_14    , _r14 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_15    , _r15 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_16    , _r16 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_17    , _r17 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_18    , _r18 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_19    , _r19 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DUMMY_1A    , _r1A ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_01  , _r1B ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_02  , _r1C ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_03  , _r1D ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_04  , _r1E ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_05  , _r1F ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_06  , _r20 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_07  , _r21 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_08  , _r22 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_09  , _r23 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_10  , _r24 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_11  , _r25 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_12  , _r26 ),
};
_Static_assert(XDIC_MIRROR_ADDR_MAX == (sizeof(gt_xdic_mirror_maps) / sizeof(_reg_map_t)), "XDIC Mirror Address map mismatch!");

static uint16_t gn_xdic_saved_trim_reg[XDIC_MIRROR_ADDR_MAX] =
{
    // 00     01     02     03     04     05     06     07     08     09     0A     0B     0C     0D     0E     0F
    0x000, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x000,

    // 10     11     12     13     14     15     16     17     18     19     1A     1B     1C     1D     1E     1F
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x040, 0x040, 0x040, 0x040, 0x040,

    // 20     21     22     23     24     25     26
    0x040, 0x040, 0x040, 0x040, 0x040, 0x040, 0x040
};

const static float gt_dev_max_curr_level_table[DEV_MAX_CURR_LEVEL_MAX] = { 4, 8, 12, 16, 24, 32, 46, 64, };

/* Variable for XD Registers */
static float gf_xd_mclk;
static float gf_vsync_out;
static uint16_t gn_xd_pwm_res;
static uint16_t gn_xd_scan_no;
static uint16_t gn_xd_ch_size;

static uint16_t gn_xd_pwm_max_size;
static uint16_t gn_xd_fpwm_div;
static uint32_t gn_xd_mclk_lock_cnt;

static dev_max_curr_level_t gt_xd_dev_max_curr_level;
static short_level_t gt_xd_short_level;
static fb_level_t gt_xd_fb_level;

static uint16_t gn_xd_delay_ch[XD_CH_SIZE] = {0, };

uint32_t gn_xd_vref_sweep_delay = 1000U;

float gf_xd_iout_measure_tgt[18] =
{
    0.343333333,    1.05875,        4.228166667,    //  4mA
    0.667666667,    2.068666667,    8.217833333,    //  8mA
    0.99525,        3.069,          12.16233333,    // 12mA
    1.32025,        4.066333333,    16.09458333,    // 16mA
    1.974583333,    6.06525,        23.965,         // 24mA
    2.62725,        8.05025,        31.78758333,    // 32mA
};

static void XDIC_Detect_Is_OTP_Written(void);

static void XDIC_Set_Register_Type(uint8_t reg_type)
{
    if (reg_type != XDIC_REG_GENERAL && reg_type != XDIC_REG_MIRROR)
    {
        print(LOG_ERROR, "ERROR: %s - Invalid Register Type !!\r\n", __func__);
        return;
    }

    if (gt_xdic_general_regs._r3F.addr_ext != reg_type)
    {
        gt_xdic_general_regs._r3F.addr_ext = reg_type;
        JigBD_IF_Write_Command(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
    }
}

static const _reg_map_t* XDIC_Get_General_Map_Pointer(uint8_t addr)
{
    for (uint8_t i = 0; i < sizeof(gt_xdic_general_maps) / sizeof(gt_xdic_general_maps[0]); ++i)
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
        XDIC_Set_Register_Type(XDIC_REG_GENERAL);
        *((uint16_t*)(map->reg_ptr)) = data;
        JigBD_IF_Write_Command(addr, data);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }
}

uint16_t XDIC_Read_General_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    const _reg_map_t* map = XDIC_Get_General_Map_Pointer(addr);
    if (map)
    {
        XDIC_Set_Register_Type(XDIC_REG_GENERAL);
        xdic_reg_val = JigBD_IF_Read_Command(addr);
        *((uint16_t*)(map->reg_ptr)) = xdic_reg_val;
        print(LOG_DEBUG, "XDIC General Read --> [ 0x%02X - 0x%04X] \r\n", addr, xdic_reg_val);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }

    return xdic_reg_val;
}

uint16_t XDIC_Get_General_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    const _reg_map_t* map = XDIC_Get_General_Map_Pointer(addr);
    if (map)
    {
        xdic_reg_val = *((uint16_t*)(map->reg_ptr));
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
        return xdic_reg_val;
    }

    return xdic_reg_val;
}

static const _reg_map_t* XDIC_Get_Mirror_Map_Pointer(uint8_t addr)
{
    for (uint8_t i = 0; i < sizeof(gt_xdic_mirror_maps) / sizeof(gt_xdic_mirror_maps[0]); ++i)
    {
        if (gt_xdic_mirror_maps[i].address == addr)
        {
            return &gt_xdic_mirror_maps[i];
        }
    }
    return NULL;
}

void XDIC_Write_Mirror_Reg(uint8_t addr, uint16_t data)
{
    const _reg_map_t* map = XDIC_Get_Mirror_Map_Pointer(addr);
    if (map)
    {
        XDIC_Set_Register_Type(XDIC_REG_MIRROR);
        *((uint16_t*)(map->reg_ptr)) = data;
        JigBD_IF_Write_Command(addr, data);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }
}

uint16_t XDIC_Read_Mirror_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    const _reg_map_t* map = XDIC_Get_Mirror_Map_Pointer(addr);
    if (map)
    {
        XDIC_Set_Register_Type(XDIC_REG_MIRROR);
        xdic_reg_val = JigBD_IF_Read_Command(addr);
        *((uint16_t*)(map->reg_ptr)) = xdic_reg_val;
        print(LOG_DEBUG, "XDIC Mirror Read --> [ 0x%02X - 0x%04X] \r\n", addr, xdic_reg_val);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }

    return xdic_reg_val;
}

uint16_t XDIC_Get_Mirror_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    const _reg_map_t* map = XDIC_Get_Mirror_Map_Pointer(addr);
    if (map)
    {
        xdic_reg_val = *((uint16_t*)(map->reg_ptr));
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }

    return xdic_reg_val;
}

void XDIC_Write_Mirror_Register_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode, uint16_t in_reg_val)
{
    switch(in_trim_mode)
    {
    case XD_TRIM_VREF_CTL:
        if (in_reg_val > REG_LIMIT_VREF)
        {
            print(LOG_ERROR, "ERROR: %s XD_TRIM_VREF_CTL- in_reg_val(%d) Over !!\r\n", __func__, in_reg_val);
        }
        else
        {
            XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_VREF_CTL, in_reg_val);
        }
        break;
    case XD_TRIM_OSC_FREQUENCY:
        if (in_reg_val > REG_LIMIT_OSC)
        {
            print(LOG_ERROR, "ERROR: %s XD_TRIM_OSC_FREQUENCY- in_reg_val(%d) Over !!\r\n", __func__, in_reg_val);
        }
        else
        {
            in_reg_val = (in_reg_val | (XDIC_OSC_RESERVED << 6));
            XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_OSC, in_reg_val);
        }
        break;
    case XD_TRIM_GAIN_CHS:
        if (in_reg_val > REG_LIMIT_GAIN)
        {
            print(LOG_ERROR, "ERROR: %s XD_TRIM_GAIN_CHS- in_reg_val(%d) Over !!\r\n", __func__, in_reg_val);
        }
        else
        {
            XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_GAIN_CH_01 + ch_num, in_reg_val);
        }
        break;
    case XD_TRIM_OFS_CHS:
        if (in_reg_val > REG_LIMIT_OFS)
        {
            print(LOG_ERROR, "ERROR: %s XD_TRIM_OFS_CHS- in_reg_val(%d) Over !!\r\n", __func__, in_reg_val);
        }
        else
        {
            XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_OFS_CH_01 + ch_num, in_reg_val);
        }
        break;
    }
}

uint16_t XDIC_Get_Mirror_Register_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0xFFFF;
    uint8_t xd_mirror_addr = XDIC_MIRROR_ADDR_MAX;
    switch(in_trim_mode)
    {
    case XD_TRIM_VREF_CTL:
        xd_mirror_addr = XDIC_MIRROR_ADDR_VREF_CTL;
        break;
    case XD_TRIM_OSC_FREQUENCY:
        xd_mirror_addr = XDIC_MIRROR_ADDR_OSC;
        break;
    case XD_TRIM_GAIN_CHS:
        xd_mirror_addr = XDIC_MIRROR_ADDR_GAIN_CH_01 + ch_num;
        break;
    case XD_TRIM_OFS_CHS:
        xd_mirror_addr = XDIC_MIRROR_ADDR_OFS_CH_01 + ch_num;
        break;
    }

    const _reg_map_t* map = XDIC_Get_Mirror_Map_Pointer(xd_mirror_addr);
    if (map)
    {
        rtn_val = *((uint16_t*)(map->reg_ptr));
    }

    if (in_trim_mode == XD_TRIM_OSC_FREQUENCY)
    {
        rtn_val = (rtn_val & 0x003F);
    }

    return rtn_val;
}

uint16_t XDIC_Get_Mirror_Register_Limit_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0xFFFF;
    switch(in_trim_mode)
    {
    case XD_TRIM_VREF_CTL:
        rtn_val = REG_LIMIT_VREF;
        break;
    case XD_TRIM_OSC_FREQUENCY:
        rtn_val = REG_LIMIT_OSC;
        break;
    case XD_TRIM_GAIN_CHS:
        rtn_val = REG_LIMIT_GAIN;
        break;
    case XD_TRIM_OFS_CHS:
        rtn_val = REG_LIMIT_OFS;
        break;
    }

    return rtn_val;
}

static void XDIC_Dump_All_Registers(void)
{
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XDIC_ADDR_MAX ; ++xd_general_addr)
    {
        const _reg_map_t* map = XDIC_Get_General_Map_Pointer(xd_general_addr);
        if (map)
        {
            print(LOG_INFO, "[ %-40s 0x%02X | 0x%04X | %-6u ]\r\n", map->name, map->address, *((uint16_t*)(map->reg_ptr)), *((uint16_t*)(map->reg_ptr)));
        }
    }
#if 1
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
    for (uint8_t xd_mirror_addr = 0 ; xd_mirror_addr < XDIC_MIRROR_ADDR_MAX ; ++xd_mirror_addr)
    {
        const _reg_map_t* map = XDIC_Get_Mirror_Map_Pointer(xd_mirror_addr);
        if (map)
        {
            print(LOG_INFO, "[ %-40s 0x%02X | 0x%04X | %-6u ]\r\n", map->name, map->address, *((uint16_t*)(map->reg_ptr)), *((uint16_t*)(map->reg_ptr)));
        }
    }
#endif
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
}

void XDIC_Dump_Trim_Regs_OneLine(void)
{
    for (uint8_t xd_mirror_addr = 0 ; xd_mirror_addr < XDIC_MIRROR_ADDR_MAX ; ++xd_mirror_addr)
    {
        const _reg_map_t* map = XDIC_Get_Mirror_Map_Pointer(xd_mirror_addr);
        if (map)
        {
            print(LOG_INFO, "0x%02X, ", *((uint16_t*)(map->reg_ptr)));
        }
    }
}

void XDIC_Read_All_Registers(void)
{
    for (uint8_t xd_general_addr = XDIC_ADDR_RESET_ID ; xd_general_addr < XDIC_ADDR_MAX ; ++xd_general_addr)
    {
        XDIC_Read_General_Reg(xd_general_addr);
    }

    for (uint8_t xd_mirror_addr = XDIC_MIRROR_ADDR_OTP_CRC ; xd_mirror_addr < XDIC_MIRROR_ADDR_MAX ; ++xd_mirror_addr)
    {
        XDIC_Read_Mirror_Reg(xd_mirror_addr);
    }

    XDIC_Dump_All_Registers();
}

void XDIC_Param_Init(void)
{
    gf_xd_mclk = XD_MCLK;
    gf_vsync_out = VSYNC;

    gn_xd_pwm_res = XD_PWM_RES_14BIT;
    gn_xd_scan_no = 0;

    if (gn_xd_pwm_res == XD_PWM_RES_12BIT)
    {
        gn_xd_pwm_max_size = (1 << (12 - gn_xd_scan_no)) - 1;
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 12)));
    }
    else //if (gn_xd_pwm_res == XD_PWM_RES_14BIT)
    {
        gn_xd_pwm_max_size = (1 << (14 - gn_xd_scan_no)) - 1;
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 14)));
    }

    gn_xd_mclk_lock_cnt = (uint32_t)(gf_xd_mclk / gf_vsync_out + 0.5f);

    gn_xd_ch_size = XD_CH_SIZE;

    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_8mA;
    gt_xd_short_level = SHORT_LEVEL_24V;
    gt_xd_fb_level = FB_LEVEL_0V4;
}

void XDIC_Init(void)
{
    JigBD_IF_Select_Output_Ch(XD_CH_MAX);
    JigBD_IF_Change_Current_Gain(GAIN_HIGH);
    JigBD_IF_XD_VCC_Level(PWR_ON_5V0);
    JigBD_IF_XD_VCC_EN(PWR_ON);

    LL_mDelay(100);

    XDIC_Param_Init();

    JigBD_IF_Reset_Command();
    JigBD_IF_IdGen_Command();

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
                gt_xdic_general_regs._r01.over_to_e = 0;
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
                gt_xdic_general_regs._r07.s_det_e = 0;
                gt_xdic_general_regs._r07.o_det_e = 0;
                gt_xdic_general_regs._r07.s_off_e = 0;
                break;
            case XDIC_ADDR_MAX_CURRENT_VREF :
                gt_xdic_general_regs._r08.max_curr_vref = 0xFFF;
                break;
            case XDIC_ADDR_SERIAL_BAUDRATE :
                gt_xdic_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
                gt_xdic_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
                break;
            case XDIC_ADDR_SERIAL_LATENCY :
                gt_xdic_general_regs._r26.serial_latency = 200;
                break;
            case XDIC_ADDR_MCLK_LOCK_1 :
                gt_xdic_general_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & XD_MCLK_LSB_MASK) >>  0U);
                break;
            case XDIC_ADDR_MCLK_LOCK_2 :
                gt_xdic_general_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & XD_MCLK_MSB_MASK) >> 12U);
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
            print(LOG_ERROR, "ERROR: Register 0x%02X not initialized (map missing)\r\n", xdic_addr);
        }
    }

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = XDIC_FB_IN_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(XDIC_FB_IN_GPIO_Port, &GPIO_InitStruct);

    XDIC_Read_General_Reg(XDIC_ADDR_RESET_ID); // Dummy Read
    XDIC_Read_All_Registers();
}

void XDIC_Trim_Param_Init(void)
{
    gt_xd_fb_level = FB_LEVEL_0V4;
    gt_xd_short_level = SHORT_LEVEL_36V;
    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_8mA;
}

void XDIC_Trim_Init(void)
{
    JigBD_IF_Select_Output_Ch(XD_CH_MAX);
    JigBD_IF_Change_Current_Gain(GAIN_HIGH);
    JigBD_IF_XD_VCC_Level(PWR_ON_5V0);
    JigBD_IF_XD_VCC_EN(PWR_ON);

    LL_mDelay(100);

    XDIC_Trim_Param_Init();

    JigBD_IF_Reset_Command();
    JigBD_IF_IdGen_Command();

    for (xdic_addr_t xdic_addr = XDIC_ADDR_RESET_ID ; xdic_addr < XDIC_ADDR_MAX ; ++xdic_addr)
    {
        const _reg_map_t* map = XDIC_Get_General_Map_Pointer(xdic_addr);
        if (map)
        {
            switch (xdic_addr)
            {
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
                gt_xdic_general_regs._r08.max_curr_vref = 0xFFF;
                break;
            case XDIC_ADDR_SERIAL_BAUDRATE :
                gt_xdic_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
                gt_xdic_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
                break;
            case XDIC_ADDR_SERIAL_LATENCY :
                gt_xdic_general_regs._r26.serial_latency = 60;
                break;
            case XDIC_ADDR_OTP_OP_MODE :
                gt_xdic_general_regs._r3F.test_en = 1;
                gt_xdic_general_regs._r3F.ddio_dis = 1;
                break;
            default :
                continue;
            }
            XDIC_Write_General_Reg(xdic_addr, *((uint16_t*)(map->reg_ptr)));
        }
        else
        {
            print(LOG_ERROR, "ERROR: Register 0x%02X not initialized (map missing)\r\n", xdic_addr);
        }
    }
    XDIC_Set_OTP_Protect(false);
    XDIC_Read_General_Reg(XDIC_ADDR_RESET_ID); // Dummy Read
    XDIC_Read_All_Registers();
}

/* ================================================================================================================================================= */
/* General Function */
/* ================================================================================================================================================= */
void XDIC_Set_Line_Delay_Into_Equal(void)
{
    if(gn_xd_ch_size)
    {
        uint16_t delay_msb_accumulator[2] = {0, };
        uint16_t delay_per_ch = gn_xd_pwm_max_size / gn_xd_ch_size;

        for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
        {
            gn_xd_delay_ch[ch] = (delay_per_ch * ch);
            uint16_t delay_lsb = ((gn_xd_delay_ch[ch] & 0x0FFF) >>  0);
            uint16_t delay_msb = ((gn_xd_delay_ch[ch] & 0x3000) >> 12);

            delay_msb_accumulator[ch / 6] |= (delay_msb << (2 * (ch % 6)));

            print(LOG_DEBUG, "[%s] delay_ch[%u] = %u / msb = %u / lsb = %u\r\n", __func__, ch, gn_xd_delay_ch[ch], delay_msb, delay_lsb);

            XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_01 + ch, delay_lsb);
        }

        XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_EXTEND_1, delay_msb_accumulator[0]);
        XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_EXTEND_2, delay_msb_accumulator[1]);
    }
}

void XDIC_Set_Max_Current_Level(dev_max_curr_level_t in_dev_max_curr)
{
    gt_xdic_general_regs._r06.dev_max_curr_level = in_dev_max_curr;
    XDIC_Write_General_Reg(XDIC_ADDR_FAULT_LEVEL, gt_xdic_general_regs._r06.val);
}

float XDIC_Get_Max_Current_level(void)
{
    float f_rtn = 0.0f;

    uint16_t fault_level = XDIC_Read_General_Reg(XDIC_ADDR_FAULT_LEVEL);
    dev_max_curr_level_t dev_max_curr_lvl = (dev_max_curr_level_t)((fault_level & 0xF00) >> 8);

    if(dev_max_curr_lvl < DEV_MAX_CURR_LEVEL_MAX)
    {
        f_rtn = gt_dev_max_curr_level_table[dev_max_curr_lvl];
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - dev_max_curr_lvl(%d) Over !!\r\n", __func__, dev_max_curr_lvl);
        f_rtn = 0.0f;
    }
    return f_rtn;
}

bool XDIC_Is_Vsync_Mode_External(void)
{
    bool rtn = false;
    if (gt_xdic_general_regs._r01.io_mode == XD_IO_MODE_EXT_VSYNC || gt_xdic_general_regs._r01.io_mode == XD_IO_MODE_EXT_VYI_FBO)
    {
        rtn = true;
    }
    return rtn;
}

void XDIC_Set_Max_Curr_Vref(uint16_t in_max_curr_vref)
{
    gt_xdic_general_regs._r08.max_curr_vref = in_max_curr_vref;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF, gt_xdic_general_regs._r08.val);
}

static void XDIC_Set_OSC_Manual_Enable(bool en)
{
    if (en == true)
    {
        gt_xdic_general_regs._r2B.osc_fll_man_e = 1;
    }
    else
    {
        gt_xdic_general_regs._r2B.osc_fll_man_e = 0;
    }
    XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_2, gt_xdic_general_regs._r2B.val);
}

static void XDIC_Set_OSC_Manual(uint16_t osc_manual)
{
    uint16_t osc_manual_lsb = ((osc_manual & 0x0FFF) >>  0);
    uint16_t osc_manual_msb = ((osc_manual & 0xF000) >> 12);

    gt_xdic_general_regs._r2A.osc_fll_man = osc_manual_lsb;
    gt_xdic_general_regs._r2B.osc_fll_man = osc_manual_msb;

    XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_1, gt_xdic_general_regs._r2A.val);
    XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_2, gt_xdic_general_regs._r2B.val);
}

void XDIC_Update_Vsync_Frequency(float n_freq)
{
    uint32_t prescale = LL_TIM_GetPrescaler(TIM8);
    uint32_t period = (uint32_t)((APB2_TIM_FREQ * 1000000 / (prescale + 1)) / n_freq - 1 + 0.5f);

    LL_TIM_SetAutoReload(TIM8, period);

    gf_vsync_out = n_freq;

    // 1. disable mclk_lock_cnt_e
    gt_xdic_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_DISABLE;
    XDIC_Write_General_Reg(XDIC_ADDR_MCLK_LOCK_2, gt_xdic_general_regs._r28.val);

    // 3. change mclk_lock_cnt
    gn_xd_mclk_lock_cnt = (uint32_t)(gf_xd_mclk / gf_vsync_out + 0.5f);
    gt_xdic_general_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & XD_MCLK_LSB_MASK) >>  0);
    gt_xdic_general_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & XD_MCLK_MSB_MASK) >> 12);

    XDIC_Write_General_Reg(XDIC_ADDR_MCLK_LOCK_1, gt_xdic_general_regs._r27.val);
    XDIC_Write_General_Reg(XDIC_ADDR_MCLK_LOCK_2, gt_xdic_general_regs._r28.val);

    // 4. enable mclk_lock_cnt_e
    gt_xdic_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_ENABLE;
    XDIC_Write_General_Reg(XDIC_ADDR_MCLK_LOCK_2, gt_xdic_general_regs._r28.val);
}
/* ================================================================================================================================================= */
/* Trim Function */
/* ================================================================================================================================================= */

void XDIC_Overwrite_Mirror_Regs(void)
{
    uint16_t u16_otp_crc = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_OTP_CRC);
    if (u16_otp_crc != 0)
    {
        print(LOG_INFO, "XDIC Already Trimmed!!!!\r\n");
    }
    else
    {
        for (uint8_t addr = XDIC_MIRROR_ADDR_OTP_CRC ; addr < XDIC_MIRROR_ADDR_MAX ; ++addr)
        {
            XDIC_Write_Mirror_Reg(addr, gn_xdic_saved_trim_reg[addr]);
        }
    }
}

void XDIC_Display_Mirror_Regs(void)
{
    print(LOG_INFO, "osc,%3u\r\n", gt_xdic_mirror_regs._r01.val & 0x3F);
    print(LOG_INFO, "vref,%3u\r\n", gt_xdic_mirror_regs._r02.val);

    print(LOG_INFO, "ofs,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n",\
        gt_xdic_mirror_regs._r03.val, gt_xdic_mirror_regs._r04.val, gt_xdic_mirror_regs._r05.val, gt_xdic_mirror_regs._r06.val,\
        gt_xdic_mirror_regs._r07.val, gt_xdic_mirror_regs._r08.val, gt_xdic_mirror_regs._r09.val, gt_xdic_mirror_regs._r0A.val,\
        gt_xdic_mirror_regs._r0B.val, gt_xdic_mirror_regs._r0C.val, gt_xdic_mirror_regs._r0D.val, gt_xdic_mirror_regs._r0E.val);
    print(LOG_INFO, "gain,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n",\
        gt_xdic_mirror_regs._r1B.val, gt_xdic_mirror_regs._r1C.val, gt_xdic_mirror_regs._r1D.val, gt_xdic_mirror_regs._r1E.val,\
        gt_xdic_mirror_regs._r1F.val, gt_xdic_mirror_regs._r20.val, gt_xdic_mirror_regs._r21.val, gt_xdic_mirror_regs._r22.val,\
        gt_xdic_mirror_regs._r23.val, gt_xdic_mirror_regs._r24.val, gt_xdic_mirror_regs._r25.val, gt_xdic_mirror_regs._r26.val);
}

void XDIC_Save_Mirror_Regs(void)
{
    for (uint8_t addr = XDIC_MIRROR_ADDR_OTP_CRC ; addr < XDIC_MIRROR_ADDR_MAX ; ++addr)
    {
        gn_xdic_saved_trim_reg[addr] = XDIC_Get_Mirror_Reg(addr);
    }
#if 0
    gn_xdic_saved_trim_reg[XDIC_MIRROR_ADDR_OSC] |= 0x800;
    XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_OSC, gn_xdic_saved_trim_reg[XDIC_MIRROR_ADDR_OSC]);
#endif
}

uint64_t XDIC_Compare_Mirror_Regs(void)
{
    uint64_t ret = 0;
    uint16_t u16_reg_val = 0;

    for (xdic_mirror_addr_t mirror_addr = XDIC_MIRROR_ADDR_OSC ; mirror_addr < XDIC_MIRROR_ADDR_MAX ; ++mirror_addr)
    {
        u16_reg_val = XDIC_Get_Mirror_Reg(mirror_addr);

        if (gn_xdic_saved_trim_reg[mirror_addr] != u16_reg_val)
        {
            ret |= ((uint64_t)1UL << mirror_addr);
            print(LOG_ERROR, "%s %17s - NG", ANSI_FONT_RED, gt_xdic_mirror_maps[mirror_addr].name);
        }
        else
        {
            print(LOG_INFO, "%s %17s - OK", ANSI_FONT_GREEN, gt_xdic_mirror_maps[mirror_addr].name);
        }
        print(LOG_INFO, "   [0x%03X] - [0x%03X] %s\r\n", gn_xdic_saved_trim_reg[mirror_addr], u16_reg_val, ANSI_FONT_NONE);
    }
    return ret;
}

void XDIC_Trim_Init_VREF_CTL(void)
{
    gt_xdic_general_regs._r3F.test_en = 1;
    gt_xdic_general_regs._r3F.ddio_dis = 1;
    gt_xdic_general_regs._r3F.test_ana_en = 3;
    gt_xdic_general_regs._r3F.pwm_full_o = 0;
    gt_xdic_general_regs._r3F.mclk32_o = 0;
    gt_xdic_general_regs._r3F.vref_o = 1;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    gt_xdic_general_regs._r08.max_curr_vref = XDIC_VREF_MAX;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF, gt_xdic_general_regs._r08.val);
}

void XDIC_Trim_Init_LDO_CTL(void)
{
    gt_xdic_general_regs._r3F.test_en = 1;
    gt_xdic_general_regs._r3F.ddio_dis = 1;
    gt_xdic_general_regs._r3F.test_ana_en = 2;
    gt_xdic_general_regs._r3F.pwm_full_o = 0;
    gt_xdic_general_regs._r3F.mclk32_o = 0;
    gt_xdic_general_regs._r3F.vref_o = 1;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    gt_xdic_general_regs._r08.max_curr_vref = XDIC_VREF_MAX;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF, gt_xdic_general_regs._r08.val);
}

void XDIC_Trim_Init_OSC(void)
{
    gt_xdic_general_regs._r3F.test_en = 1;
    gt_xdic_general_regs._r3F.ddio_dis = 1;
    gt_xdic_general_regs._r3F.test_ana_en = 0;
    gt_xdic_general_regs._r3F.pwm_full_o = 0;
    gt_xdic_general_regs._r3F.mclk32_o = 1;
    gt_xdic_general_regs._r3F.vref_o = 0;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_OSC_Manual_Enable(true);
    XDIC_Set_OSC_Manual(XDIC_TRIM_OSC_MANUAL);
}

void XDIC_Trim_Init_OFS_CH(void)
{
    gt_xdic_general_regs._r3F.test_en = 1;
    gt_xdic_general_regs._r3F.ddio_dis = 1;
    gt_xdic_general_regs._r3F.test_ana_en = 0;
    gt_xdic_general_regs._r3F.pwm_full_o = 1;
    gt_xdic_general_regs._r3F.mclk32_o = 0;
    gt_xdic_general_regs._r3F.vref_o = 0;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_Max_Current_Level(XDIC_OFS_MAX_CURRENT_LVL);
}

void XDIC_Trim_Init_GAIN_CH(void)
{
    gt_xdic_general_regs._r3F.test_en = 1;
    gt_xdic_general_regs._r3F.ddio_dis = 1;
    gt_xdic_general_regs._r3F.test_ana_en = 0;
    gt_xdic_general_regs._r3F.pwm_full_o = 1;
    gt_xdic_general_regs._r3F.mclk32_o = 0;
    gt_xdic_general_regs._r3F.vref_o = 0;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_Max_Current_Level(XDIC_GAIN_MAX_CURRENT_LVL);
}

void XDIC_Set_OTP_Protect(bool en)
{
    if (en == true)
    {
        gt_xdic_general_regs._r3E.protect_en = XDIC_OTP_PROTECT_ENABLE;
    }
    else
    {
        gt_xdic_general_regs._r3E.protect_en = XDIC_OTP_PROTECT_DISABLE;
    }
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_PROTECT, gt_xdic_general_regs._r3E.val);
}

void XDIC_Set_OTP_PG_Start(bool en)
{
    if (en == true)
    {
        gt_xdic_general_regs._r3D.otp_pg_start = 1;
    }
    else
    {
        gt_xdic_general_regs._r3D.otp_pg_start = 0;
    }
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_RD_PROG, gt_xdic_general_regs._r3D.val);
}

void XDIC_Sweep_Vref(void)
{
    uint16_t vref_adc = 0;
    float vref_volt = 0.0f;

    XDIC_Trim_Init_LDO_CTL();

    for (uint16_t vref = 0 ; vref < 0x40 ; ++vref)
    {
        XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_VREF_CTL, vref);
        us_delay(gn_xd_vref_sweep_delay);
        JigBD_IF_Start_MCU_ADC();
        vref_adc =  JigBD_IF_Get_MCU_ADC();
        vref_volt = JigBD_IF_Convert_MCU_ADC_To_Volt(vref_adc);
        print(LOG_INFO, "%d, %.3f\r\n", vref, vref_volt);

        if (vref_volt > 1.65f)
        {
            break;
        }
    }
}

void XDIC_Set_Sweep_Delay(uint16_t delay_ms)
{
    gn_xd_vref_sweep_delay = delay_ms * 1000;
}

void XDIC_Set_Line_Delay(uint16_t line_delay)
{
    print(LOG_DEBUG, "sweep delay : %u\r\n", line_delay);
    uint16_t delay_msb_accumulator[2] = {0U};
    for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
    {
        uint16_t delay_lsb = ((line_delay & 0x0FFFU) >>  0U);
        uint16_t delay_msb = ((line_delay & 0x3000U) >> 12U);

        delay_msb_accumulator[ch / 6] |= (delay_msb << (2 * (ch % 6)));

        XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_01 + ch, delay_lsb);
    }

    XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_EXTEND_1, delay_msb_accumulator[0]);
    XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_EXTEND_2, delay_msb_accumulator[1]);
}

void XDIC_Set_Scan_No(uint8_t scan_no)
{
    uint16_t r01 = XDIC_Get_General_Reg(XDIC_ADDR_LD_CONTROL);
    r01 &= ~(0x07 << 3);
    r01 |= (scan_no << 3);
    XDIC_Write_General_Reg(XDIC_ADDR_LD_CONTROL, r01);
}

static void XDIC_Detect_Is_RW_Failed(void)
{
    if (gb_xdic_initial_failed == true)
    {
        print(LOG_ERROR, "NG. Read & Write Operation Failed\r\n");
    }
    else
    {
        print(LOG_INFO, "OK. Read & Write Operation Success\r\n");
    }
}

static void XDIC_Detect_Is_OTP_Written(void)
{
    uint16_t otp_crc = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_OTP_CRC);
    if (otp_crc != 0U)
    {
        print(LOG_INFO, "OK. OTP is Written\r\n");
    }
    else
    {
        print(LOG_ERROR, "NG. OTP is Empty\r\n");
    }
}

static void XDIC_Detect_Is_ID_Failed(void)
{
    uint16_t r00 = XDIC_Get_General_Reg(XDIC_ADDR_RESET_ID);
    uint16_t id = (r00 & 0x1FU);
    if (id == 1U)
    {
        print(LOG_INFO, "OK. ID is 1\r\n");
    }
    else
    {
        print(LOG_ERROR, "NG. ID is not 1 ()\r\n");
    }
}

static void XDIC_DeInit(void)
{
    JigBD_IF_Reset_Command();
    JigBD_IF_XD_VCC_Level(PWR_ON_5V0);
    JigBD_IF_XD_VCC_EN(PWR_OFF);
}

void XDIC_Test_1(void)
{
    XDIC_Trim_Init();
    print(LOG_INFO, "\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
    print(LOG_INFO, "====================================================\r\n");
    print(LOG_INFO, "    < TEST1 Basic Electrical Characteristics >    \r\n");
    XDIC_Detect_Is_RW_Failed();
    XDIC_Detect_Is_OTP_Written();
    XDIC_Detect_Is_ID_Failed();

    // 1. VREF
    XDIC_Trim_Init_VREF_CTL();
    JigBD_IF_Start_MCU_ADC();
    uint16_t vref_adc =  JigBD_IF_Get_MCU_ADC();
    float v_vref = JigBD_IF_Convert_MCU_ADC_To_Volt(vref_adc);
    if (v_vref <= (XD_TGT_VREF * (1U + XD_TGT_ERR_RATE / 100.0f)) && v_vref >= (XD_TGT_VREF * (1U - XD_TGT_ERR_RATE / 100.0f)))
    {
        print(LOG_INFO, "VREF[V] : %.3f (OK)\r\n", v_vref);
    }
    else
    {
        print(LOG_ERROR, "VREF[V] : %.3f (NG)\r\n", v_vref);
    }

    // 2. LDO
    XDIC_Trim_Init_LDO_CTL();
    JigBD_IF_Start_MCU_ADC();
    uint16_t ldo_adc =  JigBD_IF_Get_MCU_ADC();
    float v_ldo = JigBD_IF_Convert_MCU_ADC_To_Volt(ldo_adc);
    if (v_ldo <= (XD_TGT_LDO * (1U + XD_TGT_ERR_RATE / 100.0f)) && v_ldo >= (XD_TGT_LDO * (1U - XD_TGT_ERR_RATE / 100.0f)))
    {
        print(LOG_INFO, "LDO[V] : %.3f (OK)\r\n", v_ldo);
    }
    else
    {
        print(LOG_ERROR, "LDO[V] : %.3f (NG)\r\n", v_ldo);
    }

    // 3. OSC
    XDIC_Trim_Init_OSC();
    JigBD_IF_Start_Input_Capture();
    JigBD_IF_Wait_Input_Capture_Done();
    JigBD_IF_Stop_Input_Capture();
    uint16_t osc_cnt = (uint16_t)(JigBD_IF_Get_Input_Capture_Freq());
    float osc_freq = JigBD_IF_Get_Input_Capture_Freq() * XDIC_CONST_FREQ_DIVIDE / CONST_MHz_TO_Hz;
    if (osc_freq <= (XD_TGT_OSC * (1U + XD_TGT_ERR_RATE / 100.0f)) && osc_freq >= (XD_TGT_OSC * (1U - XD_TGT_ERR_RATE / 100.0f)))
    {
        print(LOG_INFO, "OSC[MHz] : %.3f (OK)\r\n", osc_freq);
    }
    else
    {
        print(LOG_ERROR, "OSC[MHz] : %.3f (NG)\r\n", osc_freq);
    }

    // 4. Current Sweep
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
    JigBD_IF_VLED_9V_EN(PWR_ON);
    current_gain_t current_gain = GAIN_HIGH;
    JigBD_IF_Change_Current_Gain(current_gain);
    XDIC_Trim_Init_OFS_CH();
    for (dev_max_curr_level_t max_curr = DEV_MAX_CURR_LEVEL_4mA ; max_curr < DEV_MAX_CURR_LEVEL_46mA ; ++max_curr)
    {
        XDIC_Set_Max_Current_Level(max_curr);
        print(LOG_INFO, "Max Current : %.1f[mA]\r\n", XDIC_Get_Max_Current_level());
        for (uint8_t i = 0 ; i < 3 ; ++i)
        {
            uint16_t vref_table[3] = {300U, 1000U, 4095U};
            XDIC_Set_Max_Curr_Vref(vref_table[i]);
            for (XD_CH_t xd_ch = XD_CH_01 ; xd_ch < XD_CH_MAX ; ++xd_ch)
            {
                JigBD_IF_Select_Output_Ch(xd_ch);
                uint16_t iout_adc = 0;
                ADS114S08_Set_Start(1);
                ADS114S08_Wait_Done();
                iout_adc = ADS114S08_Get_ADC_Value();
                float mA_iout = JigBD_IF_Convert_Adc_To_Current(iout_adc, current_gain);
                float tgt_mA = gf_xd_iout_measure_tgt[max_curr * 3 + i];
                if (mA_iout <= (tgt_mA * (1U + XD_TGT_ERR_RATE / 100.0f)) && mA_iout >= (tgt_mA * (1U - XD_TGT_ERR_RATE / 100.0f)))
                {
                    print(LOG_INFO, "%.3f(O)\t", mA_iout);
                }
                else
                {
                    print(LOG_ERROR, "%.3f(X)\t", mA_iout);
                }
            }
            print(LOG_INFO, "\r\n");
        }
    }
    print(LOG_INFO, "==================== TEST1 Done ====================\r\n");
    XDIC_DeInit();
}

void XDIC_Test_2(void)
{
    XDIC_Trim_Init();
    print(LOG_INFO, "====================================================\r\n");
    print(LOG_INFO, "    < TEST2 Channel Dependency >    \r\n");

    XDIC_Trim_Init_OFS_CH();
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
    JigBD_IF_VLED_9V_EN(PWR_ON);
    uint16_t iout_adc_all[XD_CH_SIZE][2] = {0, };
    uint16_t iout_adc_sel[XD_CH_SIZE][2] = {0, };
    current_gain_t current_gain = GAIN_HIGH;
    JigBD_IF_Change_Current_Gain(current_gain);
    XDIC_Set_Max_Curr_Vref(1000);

    // 1) All Channel Enable
    for (uint8_t i = 0 ; i < XD_CH_SIZE ; ++i)
    {
        XDIC_Write_General_Reg(XDIC_ADDR_CHANNEL_ENABLE, 0xFFF);
        JigBD_IF_Select_Output_Ch(i);
        ADS114S08_Set_Start(1);
        ADS114S08_Wait_Done();
        iout_adc_all[i][0] = ADS114S08_Get_ADC_Value();
    }
    // 2) Select Channel Enable
    for (uint8_t i = 0 ; i < XD_CH_SIZE ; ++i)
    {
        XDIC_Write_General_Reg(XDIC_ADDR_CHANNEL_ENABLE, (1 << i));
        JigBD_IF_Select_Output_Ch(i);
        ADS114S08_Set_Start(1);
        ADS114S08_Wait_Done();
        iout_adc_sel[i][0] = ADS114S08_Get_ADC_Value();
    }
    // 3) Display Result
    print(LOG_INFO, "\r\n[All CH Enable] vs [Select CH Enable]\r\n");
    for (uint8_t i = 0 ; i < XD_CH_SIZE ; ++i)
    {
        float iout_avg_all = JigBD_IF_Convert_Adc_To_Current(iout_adc_all[i][0], current_gain);
        float iout_avg_sel = JigBD_IF_Convert_Adc_To_Current(iout_adc_sel[i][0], current_gain);
        bool judge = false;
        if (iout_avg_all > iout_avg_sel)
        {
            if (iout_avg_all - iout_avg_sel < 0.05)
            {
                judge = true;
            }
        }
        else if (iout_avg_all < iout_avg_sel)
        {
            if (iout_avg_sel - iout_avg_all < 0.05)
            {
                judge = true;
            }
        }
        else
        {
            judge = true;
        }
        if (judge == true)
        {
            print(LOG_INFO, "OK. CH[%02d] : %.3f vs %.3f (O)\r\n", (i + 1), iout_avg_all, iout_avg_sel);
        }
        else
        {
            print(LOG_ERROR, "NG. CH[%02d] : %.3f vs %.3f (X)\r\n", (i + 1), iout_avg_all, iout_avg_sel);
        }
    }
    print(LOG_INFO, "==================== TEST2 Done ====================\r\n");
    XDIC_DeInit();
}