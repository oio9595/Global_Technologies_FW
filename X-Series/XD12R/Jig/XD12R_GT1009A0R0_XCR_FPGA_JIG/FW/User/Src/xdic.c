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

#define XDIC_REG_GENERAL            (0U)
#define XDIC_REG_MIRROR             (1U)

#define XDIC_OTP_PROTECT_DISABLE    (0xA5AU)
#define XDIC_OTP_PROTECT_ENABLE     (0x5A5U)

#define XDIC_LD_MODE_NORMAL         (0U)
#define XDIC_LD_MODE_X8             (1U)

#define XDIC_LD_DIR_HEAD_SHIFT      (0U)
#define XDIC_LD_DIR_TAIL_SHIFT      (1U)

#define XDIC_LD_TYPE_NOP            (0U) // NOP
#define XDIC_LD_TYPE_1              (1U) // RGB, 12CH
#define XDIC_LD_TYPE_2              (2U) // R/GB, 6CH
#define XDIC_LD_TYPE_3              (3U) // MONO, 12CH

#define XDIC_PWM_RES_12BIT          (0U)
#define XDIC_PWM_RES_14BIT          (1U)

#define XDIC_SYNC_MODE_CMD          (0U)
#define XDIC_SYNC_MODE_SVI          (1U)

#define XDIC_VREF_MODE_VREF2_3      (0U)
#define XDIC_VREF_MODE_VREF2        (1U)

#define XDIC_PARITY_CHECK_DIS       (0U)
#define XDIC_PARITY_CHECK_EN        (1U)

#define XDIC_MCLK_FLL_ENABLE        (0U)
#define XDIC_MCLK_FLL_DISABLE       (1U)

#define XDIC_MCLK_LSB_MASK          (0xFFFU) // LSB 12-bit
#define XDIC_MCLK_MSB_MASK          (0x1FFU) // MSB  9-bit

#define XDIC_WR_PWM_DIV_VREF        (0xAAAU)
#define XDIC_WR_OTHERS              (0x555U)

#define XDIC_PWM_DIV_RED            (6U)
#define XDIC_PWM_DIV_GREEN          (6U)
#define XDIC_PWM_DIV_BLUE           (6U)

#define XDIC_V_MASK_SIZE            (410U)
#define XDIC_SV_MASK_SIZE           (570U)

#define XDIC_MCLK_LOCK_CNT          (450000U)
#define XDIC_VREF                   (4095U)

#define XDIC_CHANNEL_ENABLE_MAX     (uint16_t)((1U << XDIC_CH_SIZE) - 1U)

#define XDIC_R01_DAC_A_OFS_SHIFT    (0U)
#define XDIC_R01_DAC_A_OFS_MASK     ((uint16_t)(0x7FU << XDIC_R01_DAC_A_OFS_SHIFT))

#define XDIC_R01_DAC_B_OFS_SHIFT    (7U)
#define XDIC_R01_DAC_B_OFS_MASK     ((uint16_t)(0x1FU << XDIC_R01_DAC_B_OFS_SHIFT))

#define XDIC_R02_BGR_TC_SHIFT       (0U)
#define XDIC_R02_BGR_TC_MASK        ((uint16_t)(0x1FU << XDIC_R02_BGR_TC_SHIFT))

#define XDIC_R02_IREF_CTL_SHIFT     (5U)
#define XDIC_R02_IREF_CTL_MASK      ((uint16_t)(0x1FU << XDIC_R02_IREF_CTL_SHIFT))

#define XDIC_R02_DAC_B_OFS_SHIFT    (10U)
#define XDIC_R02_DAC_B_OFS_MASK     ((uint16_t)(0x03U << XDIC_R02_DAC_B_OFS_SHIFT))

#define XDIC_R03_OSC_RCTL_SHIFT     (0U)
#define XDIC_R03_OSC_RCTL_MASK      ((uint16_t)(0x0FU << XDIC_R03_OSC_RCTL_SHIFT))

#define XDIC_R03_LDO_DAC_CTL_SHIFT  (11U)
#define XDIC_R03_LDO_DAC_CTL_MASK   ((uint16_t)(0x01U << XDIC_R03_LDO_DAC_CTL_SHIFT))

#define XDIC_R04_LDO_CTL_SHIFT      (0U)
#define XDIC_R04_LDO_CTL_MASK       ((uint16_t)(0x0FU << XDIC_R04_LDO_CTL_SHIFT))

#define XDIC_R04_LDO_OSC_CTL_SHIFT  (4U)
#define XDIC_R04_LDO_OSC_CTL_MASK   ((uint16_t)(0x0FU << XDIC_R04_LDO_OSC_CTL_SHIFT))

#define XDIC_R04_LDO_DAC_CTL_SHIFT  (8U)
#define XDIC_R04_LDO_DAC_CTL_MASK   ((uint16_t)(0x0FU << XDIC_R04_LDO_DAC_CTL_SHIFT))

#define XDIC_R09_VDD_OV_SHIFT       (9U)
#define XDIC_R09_VDD_OV_MASK        (0x01U)

#define XDIC_R09_VDD_UV_SHIFT       (8U)
#define XDIC_R09_VDD_UV_MASK        (0x01U)

#define XDIC_R09_LDO_OV_SHIFT       (7U)
#define XDIC_R09_LDO_OV_MASK        (0x01U)

#define XDIC_R09_LDO_UV_SHIFT       (6U)
#define XDIC_R09_LDO_UV_MASK        (0x01U)

typedef enum tag_XDIC_SUBSTITUTE_VALUE_ORDER_T
{
    XDIC_SUBSTITUTE_VALUE_ORDER_IBN_2uA,
    XDIC_SUBSTITUTE_VALUE_ORDER_DAC_LDO_1V5,
    XDIC_SUBSTITUTE_VALUE_ORDER_DIG_LDO_1V5,
    XDIC_SUBSTITUTE_VALUE_ORDER_DAC_A_OFS,
    XDIC_SUBSTITUTE_VALUE_ORDER_DAC_B_OFS,
    XDIC_SUBSTITUTE_VALUE_ORDER_FLL_LDO_1V5,
    XDIC_SUBSTITUTE_VALUE_ORDER_OSC,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH01,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH02,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH03,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH04,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH05,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH06,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH07,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH08,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH09,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH10,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH11,
    XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH12,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH01,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH02,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH03,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH04,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH05,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH06,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH07,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH08,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH09,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH10,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH11,
    XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH12,
    XDIC_SUBSTITUTE_VALUE_ORDER_MAX,
} xdic_substitute_value_order_t;

volatile bool gb_xdic_initial_failed;

static _xdic_general_regs_t gt_xdic_general_regs;
static _xdic_mirror_regs_t gt_xdic_mirror_regs;

static _reg_map_t gt_xdic_general_maps[] =
{
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_RESET_ID         , _r00 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_LD_CONTROL       , _r01 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SVSYNC_NUM       , _r02 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FPWM_DIVIDER_1_2 , _r03 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FPWM_DIVIDER_2_3 , _r04 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_CHANNEL_ENABLE   , _r05 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FAULT_CONTROL    , _r06 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FB_LEVEL         , _r07 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SHORT_LEVEL      , _r08 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FAULT_STATUS     , _r09 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MAX_CURRENT_LEVEL, _r0A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MAX_CURRENT_VREF1, _r0B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MAX_CURRENT_VREF2, _r0C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_MAX_CURRENT_VREF3, _r0D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_01_02   , _r0E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_03_04   , _r0F ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_05_06   , _r10 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_07_08   , _r11 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_09_10   , _r12 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DELAY_CH_11_12   , _r13 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SERIAL_BAUDRATE  , _r14 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SERIAL_LATENCY   , _r15 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_V_MASK           , _r16 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SV_MASK          , _r17 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_RSTCNT           , _r18 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_TIMEOUT          , _r19 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FLL_CONTROL_1    , _r1A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_FLL_CONTROL_2    , _r1B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_WR_PROTECT       , _r1C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_NF_CONTROL       , _r1D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1E         , _r1E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_1F         , _r1F ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_CHOP_EN          , _r20 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_TEMP             , _r21 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OSC_FLL_MANUAL_1 , _r22 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_OSC_FLL_MANUAL_2 , _r23 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_SPREAD           , _r24 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_25         , _r25 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_26         , _r26 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_27         , _r27 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_28         , _r28 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_29         , _r29 ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_2A         , _r2A ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_2B         , _r2B ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_2C         , _r2C ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_2D         , _r2D ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_2E         , _r2E ),
    XDIC_GENERAL_REG_ENTRY( XDIC_ADDR_DUMMY_2F         , _r2F ),
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

static _reg_map_t gt_xdic_mirror_maps[] =
{
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OTP_CRC             , _r00 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_DAC_OFS             , _r01 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_IREF_CTL_BGR_TC     , _r02 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_LDO_DAC_CTL_OSC_RCTL, _r03 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_LDO_CTL             , _r04 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_01           , _r05 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_02           , _r06 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_03           , _r07 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_04           , _r08 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_05           , _r09 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_06           , _r0A ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_07           , _r0B ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_08           , _r0C ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_09           , _r0D ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_10           , _r0E ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_11           , _r0F ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_OFS_CH_12           , _r10 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_01          , _r11 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_02          , _r12 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_03          , _r13 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_04          , _r14 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_05          , _r15 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_06          , _r16 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_07          , _r17 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_08          , _r18 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_09          , _r19 ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_10          , _r1A ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_11          , _r1B ),
    XDIC_MIRROR_REG_ENTRY( XDIC_MIRROR_ADDR_GAIN_CH_12          , _r1C ),
};
_Static_assert(XDIC_MIRROR_ADDR_MAX == (sizeof(gt_xdic_mirror_maps) / sizeof(_reg_map_t)), "XDIC Mirror Address map mismatch!");

static uint16_t gn_xdic_saved_trim_reg[XDIC_MIRROR_ADDR_MAX] =
{
    // 00     01     02     03     04     05     06     07     08     09     0A     0B     0C     0D     0E     0F
    0x000, 0x000, 0x210, 0x020, 0x020, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080,

    // 10     11     12     13     14     15     16     17     18     19     1A     1B     1C
    0x080, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020, 0x020
};

static uint16_t gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_MAX] =
{
    XDIC_IBN_2uA_DEFAULT_VALUE,
    XDIC_DAC_LDO_1V5_DEFAULT_VALUE,
    XDIC_DIG_LDO_1V5_DEFAULT_VALUE,
    XDIC_DAC_A_OFS_DEFAULT_VALUE,
    XDIC_DAC_B_OFS_DEFAULT_VALUE,
    XDIC_FLL_LDO_1V5_DEFAULT_VALUE,
    XDIC_OSC_DEFAULT_VALUE,
    XDIC_OFS_CH_DEFAULT_VALUE, XDIC_OFS_CH_DEFAULT_VALUE, XDIC_OFS_CH_DEFAULT_VALUE,
    XDIC_OFS_CH_DEFAULT_VALUE, XDIC_OFS_CH_DEFAULT_VALUE, XDIC_OFS_CH_DEFAULT_VALUE,
    XDIC_OFS_CH_DEFAULT_VALUE, XDIC_OFS_CH_DEFAULT_VALUE, XDIC_OFS_CH_DEFAULT_VALUE,
    XDIC_OFS_CH_DEFAULT_VALUE, XDIC_OFS_CH_DEFAULT_VALUE, XDIC_OFS_CH_DEFAULT_VALUE,
    XDIC_GAIN_CH_DEFAULT_VALUE,XDIC_GAIN_CH_DEFAULT_VALUE,XDIC_GAIN_CH_DEFAULT_VALUE,
    XDIC_GAIN_CH_DEFAULT_VALUE,XDIC_GAIN_CH_DEFAULT_VALUE,XDIC_GAIN_CH_DEFAULT_VALUE,
    XDIC_GAIN_CH_DEFAULT_VALUE,XDIC_GAIN_CH_DEFAULT_VALUE,XDIC_GAIN_CH_DEFAULT_VALUE,
    XDIC_GAIN_CH_DEFAULT_VALUE,XDIC_GAIN_CH_DEFAULT_VALUE,XDIC_GAIN_CH_DEFAULT_VALUE,
};

/* Variable for XD Registers */
static float gf_xdic_mclk;
static float gf_vsync_out;

static uint32_t gn_xdic_mclk_lock_cnt;

static dev_max_curr_level_t gt_xdic_dev_max_curr_level[3] = {0};
static short_level_t gt_xdic_short_level[3] = {0};
static fb_level_t gt_xdic_fb_level[3] = {0};

static void XDIC_Set_Delay_CH(void);

static void XDIC_Set_Writeable_Type(bool reg_type)
{
    if (reg_type)
    {
        gt_xdic_general_regs._r1C.wr_protect = XDIC_WR_PWM_DIV_VREF;
    }
    else
    {
        gt_xdic_general_regs._r1C.wr_protect = XDIC_WR_OTHERS;
    }
    JigBD_IF_Write_Command(XDIC_ADDR_WR_PROTECT, gt_xdic_general_regs._r1C.val);
}

static void XDIC_Set_Register_Type(uint8_t reg_type)
{
    if ((reg_type != XDIC_REG_GENERAL) && (reg_type != XDIC_REG_MIRROR))
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

static const _reg_map_t* XDIC_Get_General_Map_Pointer(xdic_addr_t addr)
{
    for (uint8_t i = 0U ; i < sizeof(gt_xdic_general_maps) / sizeof(gt_xdic_general_maps[0]); ++i)
    {
        if (gt_xdic_general_maps[i].address == addr)
        {
            return &gt_xdic_general_maps[i];
        }
    }
    return NULL;
}

void XDIC_Write_General_Reg(xdic_addr_t addr, uint16_t data)
{
    const _reg_map_t* map = XDIC_Get_General_Map_Pointer(addr);
    if (map)
    {
        //print(LOG_INFO, "Write General Addr 0x%02X, Data 0x%04X\r\n", addr, data);
        XDIC_Set_Register_Type(XDIC_REG_GENERAL);
        if (/*(addr == 0x02U) || */(addr == 0x03U) || (addr == 0x04U) || (addr == 0x0BU) || (addr == 0x0CU) || (addr == 0x0DU))
        {
            XDIC_Set_Writeable_Type(true);
            *((uint16_t*)(map->reg_ptr)) = data;
            JigBD_IF_Write_Command(addr, data);
            XDIC_Set_Writeable_Type(false);
        }
        else
        {
            *((uint16_t*)(map->reg_ptr)) = data;
            JigBD_IF_Write_Command(addr, data);
        }
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }
}

uint16_t XDIC_Read_General_Reg(xdic_addr_t addr)
{
    uint16_t xdic_reg_val = 0xFFFFU;
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

uint16_t XDIC_Get_General_Reg(xdic_addr_t addr)
{
    uint16_t xdic_reg_val = 0xFFFFU;
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

static const _reg_map_t* XDIC_Get_Mirror_Map_Pointer(xdic_mirror_addr_t addr)
{
    for (uint8_t i = 0U ; i < sizeof(gt_xdic_mirror_maps) / sizeof(gt_xdic_mirror_maps[0]); ++i)
    {
        if (gt_xdic_mirror_maps[i].address == addr)
        {
            return &gt_xdic_mirror_maps[i];
        }
    }
    return NULL;
}

void XDIC_Write_Mirror_Reg(xdic_mirror_addr_t addr, uint16_t data)
{
    const _reg_map_t* map = XDIC_Get_Mirror_Map_Pointer(addr);
    if (map)
    {
        //print(LOG_INFO, "Write Mirror Addr 0x%02X, Data 0x%04X             ", addr, data);
        XDIC_Set_Register_Type(XDIC_REG_MIRROR);
        *((uint16_t*)(map->reg_ptr)) = data;
        JigBD_IF_Write_Command(addr, data);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }
}

uint16_t XDIC_Read_Mirror_Reg(xdic_mirror_addr_t addr)
{
    uint16_t xdic_reg_val = 0xFFFFU;
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

uint16_t XDIC_Get_Mirror_Reg(xdic_mirror_addr_t addr)
{
    uint16_t xdic_reg_val = 0xFFFFU;
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

void XDIC_Write_Substitute_Value_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode, uint16_t in_sub_val)
{
    uint16_t reg_val = 0U;
    uint16_t reg_form_value = 0U;
    switch(in_trim_mode)
    {
        case XD_TRIM_IBN_2uA:
            if (in_sub_val > XDIC_REG_LIMIT_IBN_2uA)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_IBN_2uA- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_IBN_2uA] = in_sub_val;
                reg_form_value = in_sub_val;
                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_IREF_CTL_BGR_TC);
                reg_val &= (uint16_t)(~XDIC_R02_IREF_CTL_MASK);
                reg_val |= (reg_form_value << XDIC_R02_IREF_CTL_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_IREF_CTL_BGR_TC, reg_val);
            }
            break;
        case XD_TRIM_DAC_LDO_1V5:
            if (in_sub_val > XDIC_REG_LIMIT_DAC_LDO_1V5)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_DAC_LDO_1V5- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_DAC_LDO_1V5] = in_sub_val;
                reg_form_value = in_sub_val;

                uint16_t ldo_dac_ctl_lsb = ((reg_form_value >> 0U) & 0x0FU);
                uint16_t ldo_dac_ctl_msb = ((reg_form_value >> 4U) & 0x01U);

                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_DAC_CTL_OSC_RCTL);
                reg_val &= (uint16_t)(~XDIC_R03_LDO_DAC_CTL_MASK);
                reg_val |= (ldo_dac_ctl_msb << XDIC_R03_LDO_DAC_CTL_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_DAC_CTL_OSC_RCTL, reg_val);

                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_CTL);
                reg_val &= (uint16_t)(~XDIC_R04_LDO_DAC_CTL_MASK);
                reg_val |= (ldo_dac_ctl_lsb << XDIC_R04_LDO_DAC_CTL_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_CTL, reg_val);
            }
            break;
        case XD_TRIM_DIG_LDO_1V5:
            if (in_sub_val > XDIC_REG_LIMIT_DIG_LDO_1V5)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_DIG_LDO_1V5- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_DIG_LDO_1V5] = in_sub_val;
#if 0
                if (in_sub_val < 8)
                {
                    reg_form_value = 15 - in_sub_val;
                }
                else
                {
                    reg_form_value = in_sub_val - 8;
                }
#endif
                reg_form_value = in_sub_val ^ 8U;
                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_CTL);
                reg_val &= (uint16_t)(~XDIC_R04_LDO_CTL_MASK);
                reg_val |= (reg_form_value << XDIC_R04_LDO_CTL_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_CTL, reg_val);
            }
            break;
        case XD_TRIM_DAC_A_OFS:
            if (in_sub_val > XDIC_REG_LIMIT_DAC_A_OFS)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_DAC_A_OFS- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_DAC_A_OFS] = in_sub_val;

                if (in_sub_val < 64U)
                {
                    reg_form_value = 127U - in_sub_val;
                }
                else
                {
                    reg_form_value = in_sub_val - 64U;
                }
                //reg_form_value = in_sub_val ^ 64U;
                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_DAC_OFS);
                reg_val &= (uint16_t)(~XDIC_R01_DAC_A_OFS_MASK);
                reg_val |= (reg_form_value << XDIC_R01_DAC_A_OFS_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_DAC_OFS, reg_val);
            }
            break;
        case XD_TRIM_DAC_B_OFS:
            if (in_sub_val > XDIC_REG_LIMIT_DAC_B_OFS)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_DAC_B_OFS- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_DAC_B_OFS] = in_sub_val;
                if (in_sub_val < 64U)
                {
                    reg_form_value = 127U - in_sub_val;
                }
                else
                {
                    reg_form_value = in_sub_val - 64U;
                }
                //reg_form_value = in_sub_val ^ 64U;
                uint16_t dac_b_ofs_lsb = ((reg_form_value >> 0U) & 0x1FU);
                uint16_t dac_b_ofs_msb = ((reg_form_value >> 5U) & 0x03U);

                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_DAC_OFS);
                reg_val &= (uint16_t)(~XDIC_R01_DAC_B_OFS_MASK);
                reg_val |= (dac_b_ofs_lsb << XDIC_R01_DAC_B_OFS_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_DAC_OFS, reg_val);

                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_IREF_CTL_BGR_TC);
                reg_val &= (uint16_t)(~XDIC_R02_DAC_B_OFS_MASK);
                reg_val |= (dac_b_ofs_msb << XDIC_R02_DAC_B_OFS_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_IREF_CTL_BGR_TC, reg_val);
            }
            break;
        case XD_TRIM_FLL_LDO_1V5:
            if (in_sub_val > XDIC_REG_LIMIT_FLL_LDO_1V5)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_FLL_LDO_1V5- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_FLL_LDO_1V5] = in_sub_val;
                reg_form_value = in_sub_val;
                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_CTL);
                reg_val &= (uint16_t)(~XDIC_R04_LDO_OSC_CTL_MASK);
                reg_val |= (reg_form_value << XDIC_R04_LDO_OSC_CTL_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_CTL, reg_val);
            }
            break;
        case XD_TRIM_OSC:
            if (in_sub_val > XDIC_REG_LIMIT_OSC)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_OSC- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_OSC] = in_sub_val;
                reg_form_value = in_sub_val;
                reg_val = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_DAC_CTL_OSC_RCTL);
                reg_val &= (uint16_t)(~XDIC_R03_OSC_RCTL_MASK);
                reg_val |= (reg_form_value << XDIC_R03_OSC_RCTL_SHIFT);
                XDIC_Write_Mirror_Reg(XDIC_MIRROR_ADDR_LDO_DAC_CTL_OSC_RCTL, reg_val);
            }
            break;
        case XD_TRIM_CH_GAIN_ODD:
            if (in_sub_val > XDIC_REG_LIMIT_CH_GAIN)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_CH_GAIN_ODD- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH01 + (ch_num * 2U)] = in_sub_val;
                XDIC_Write_Mirror_Reg((XDIC_MIRROR_ADDR_GAIN_CH_01 + (xdic_mirror_addr_t)(ch_num * 2U)), in_sub_val);
            }
            break;
        case XD_TRIM_CH_GAIN_EVEN:
            if (in_sub_val > XDIC_REG_LIMIT_CH_GAIN)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_CH_GAIN_EVEN- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH02 + (ch_num * 2U)] = in_sub_val;
                XDIC_Write_Mirror_Reg((XDIC_MIRROR_ADDR_GAIN_CH_02 + (xdic_mirror_addr_t)(ch_num * 2U)), in_sub_val);
            }
            break;
        case XD_TRIM_CH_OFS_ODD:
            if (in_sub_val > XDIC_REG_LIMIT_CH_OFS)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_CH_OFS_ODD- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH01 + (ch_num * 2U)] = in_sub_val;
                XDIC_Write_Mirror_Reg((XDIC_MIRROR_ADDR_OFS_CH_01 + (xdic_mirror_addr_t)(ch_num * 2U)), in_sub_val);
            }
            break;
        case XD_TRIM_CH_OFS_EVEN:
            if (in_sub_val > XDIC_REG_LIMIT_CH_OFS)
            {
                print(LOG_ERROR, "ERROR: %s XD_TRIM_CH_OFS_EVEN- in_sub_val(%d) Over !!\r\n", __func__, in_sub_val);
            }
            else
            {
                gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH02 + (ch_num * 2U)] = in_sub_val;
                XDIC_Write_Mirror_Reg((XDIC_MIRROR_ADDR_OFS_CH_02 + (xdic_mirror_addr_t)(ch_num * 2U)), in_sub_val);
            }
            break;
        default:
            print(LOG_ERROR, "ERROR: %s - Invalid Trim Mode !!\r\n", __func__);
            break;
    }
}

uint16_t XDIC_Get_Substitute_Value_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0xFFFFU;
    switch(in_trim_mode)
    {
        case XD_TRIM_IBN_2uA:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_IBN_2uA];
            break;
        case XD_TRIM_DAC_LDO_1V5:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_DAC_LDO_1V5];
            break;
        case XD_TRIM_DIG_LDO_1V5:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_DIG_LDO_1V5];
            break;
        case XD_TRIM_DAC_A_OFS:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_DAC_A_OFS];
            break;
        case XD_TRIM_DAC_B_OFS:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_DAC_B_OFS];
            break;
        case XD_TRIM_FLL_LDO_1V5:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_FLL_LDO_1V5];
            break;
        case XD_TRIM_OSC:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_OSC];
            break;
        case XD_TRIM_CH_GAIN_ODD:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH01 + (ch_num * 2U)];
            break;
        case XD_TRIM_CH_GAIN_EVEN:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_GAIN_CH02 + (ch_num * 2U)];
            break;
        case XD_TRIM_CH_OFS_ODD:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH01 + (ch_num * 2U)];
            break;
        case XD_TRIM_CH_OFS_EVEN:
            rtn_val = gn_xdic_trim_substitute_value[XDIC_SUBSTITUTE_VALUE_ORDER_OFS_CH02 + (ch_num * 2U)];
            break;
        default:
            print(LOG_ERROR, "ERROR: %s - Invalid Trim Mode !!\r\n", __func__);
            break;
    }

    return rtn_val;
}

uint16_t XDIC_Get_Substitute_Value_Limit_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0xFFFFU;
    switch(in_trim_mode)
    {
    case XD_TRIM_IBN_2uA:
        rtn_val = XDIC_REG_LIMIT_IBN_2uA;
        break;
    case XD_TRIM_DAC_LDO_1V5:
        rtn_val = XDIC_REG_LIMIT_DAC_LDO_1V5;
        break;
    case XD_TRIM_DIG_LDO_1V5:
        rtn_val = XDIC_REG_LIMIT_DIG_LDO_1V5;
        break;
    case XD_TRIM_DAC_A_OFS:
        rtn_val = XDIC_REG_LIMIT_DAC_A_OFS;
        break;
    case XD_TRIM_DAC_B_OFS:
        rtn_val = XDIC_REG_LIMIT_DAC_B_OFS;
        break;
    case XD_TRIM_FLL_LDO_1V5:
        rtn_val = XDIC_REG_LIMIT_FLL_LDO_1V5;
        break;
    case XD_TRIM_OSC:
        rtn_val = XDIC_REG_LIMIT_OSC;
        break;
    case XD_TRIM_CH_GAIN_ODD:
    case XD_TRIM_CH_GAIN_EVEN:
        rtn_val = XDIC_REG_LIMIT_CH_GAIN;
        break;
    case XD_TRIM_CH_OFS_ODD:
    case XD_TRIM_CH_OFS_EVEN:
        rtn_val = XDIC_REG_LIMIT_CH_OFS;
        break;
    default:
        print(LOG_ERROR, "ERROR: %s - Invalid Trim Mode !!\r\n", __func__);
        break;
    }
    return rtn_val;
}

static void XDIC_Dump_All_Registers(void)
{
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
    for (xdic_addr_t xd_general_addr = XDIC_ADDR_RESET_ID ; xd_general_addr < XDIC_ADDR_MAX ; ++xd_general_addr)
    {
        const _reg_map_t* map = XDIC_Get_General_Map_Pointer(xd_general_addr);
        if (map)
        {
            print(LOG_INFO, "[ %-40s 0x%02X | 0x%04X | %-6u ]\r\n", map->name, map->address, *((uint16_t*)(map->reg_ptr)), *((uint16_t*)(map->reg_ptr)));
        }
    }
#if 1
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
    for (xdic_mirror_addr_t xd_mirror_addr = XDIC_MIRROR_ADDR_OTP_CRC ; xd_mirror_addr < XDIC_MIRROR_ADDR_MAX ; ++xd_mirror_addr)
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
    for (xdic_mirror_addr_t xd_mirror_addr = XDIC_MIRROR_ADDR_OTP_CRC ; xd_mirror_addr < XDIC_MIRROR_ADDR_MAX ; ++xd_mirror_addr)
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
    for (xdic_addr_t xd_general_addr = XDIC_ADDR_RESET_ID ; xd_general_addr < XDIC_ADDR_MAX ; ++xd_general_addr)
    {
        XDIC_Read_General_Reg(xd_general_addr);
    }

    for (xdic_mirror_addr_t xd_mirror_addr = XDIC_MIRROR_ADDR_OTP_CRC ; xd_mirror_addr < XDIC_MIRROR_ADDR_MAX ; ++xd_mirror_addr)
    {
        XDIC_Read_Mirror_Reg(xd_mirror_addr);
    }

    XDIC_Dump_All_Registers();
}

static void XDIC_Param_Init(void)
{
    gf_xdic_mclk = XDIC_MCLK;
    gf_vsync_out = VSYNC;

    //gn_xdic_mclk_lock_cnt = (uint32_t)(gf_xdic_mclk / gf_vsync_out + 0.5f);
    gn_xdic_mclk_lock_cnt = XDIC_MCLK_LOCK_CNT;

    for (uint8_t i = 0U ; i < 3U ; ++i)
    {
        gt_xdic_fb_level[i] = FB_LEVEL_0V45;
        gt_xdic_short_level[i] = SHORT_LEVEL_32V;
        gt_xdic_dev_max_curr_level[i] = DEV_MAX_CURR_LEVEL_4mA;
    }
}

void XDIC_Init(void)
{
    JigBD_IF_Detect_XC24();

    JigBD_IF_Select_Output_Ch(XD_CH_MAX);
    JigBD_IF_Change_Current_Gain(GAIN_HIGH);
    JigBD_IF_XD_VCC_Level(PWR_ON_5V0);
    JigBD_IF_XD_VCC_EN(PWR_ON);

    LL_mDelay(100U);

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
                gt_xdic_general_regs._r01.ld_mode = XDIC_LD_MODE_NORMAL;
                gt_xdic_general_regs._r01.ld_dir = XDIC_LD_DIR_HEAD_SHIFT;
                gt_xdic_general_regs._r01.ld_type = XDIC_LD_TYPE_2;
                gt_xdic_general_regs._r01.pwm_res = XDIC_PWM_RES_14BIT;
                gt_xdic_general_regs._r01.syncmode = XDIC_SYNC_MODE_SVI;
                gt_xdic_general_regs._r01.vrefmode = XDIC_VREF_MODE_VREF2_3;
                gt_xdic_general_regs._r01.pc_en = XDIC_PARITY_CHECK_DIS;
                gt_xdic_general_regs._r01.ld_size = XDIC_CH_SIZE;
                break;
            case XDIC_ADDR_SVSYNC_NUM :
                gt_xdic_general_regs._r02.fpwm_div1 = (XDIC_PWM_DIV_RED >> 8U);
                gt_xdic_general_regs._r02.sv_no = SVSYNC_SIZE;
                break;
            case XDIC_ADDR_FPWM_DIVIDER_1_2 :
                gt_xdic_general_regs._r03.fpwm_div1 = (XDIC_PWM_DIV_RED & 0xFFU);
                gt_xdic_general_regs._r03.fpwm_div2 = ((XDIC_PWM_DIV_GREEN >> 0U) & 0x0FU);
                break;
            case XDIC_ADDR_FPWM_DIVIDER_2_3 :
                gt_xdic_general_regs._r04.fpwm_div2 = ((XDIC_PWM_DIV_GREEN >> 4U) & 0x0FU);
                gt_xdic_general_regs._r04.fpwm_div3 = XDIC_PWM_DIV_BLUE;
                break;
            case XDIC_ADDR_CHANNEL_ENABLE :
                gt_xdic_general_regs._r05.val = XDIC_CHANNEL_ENABLE_MAX;
                break;
            case XDIC_ADDR_FAULT_CONTROL :
                gt_xdic_general_regs._r06.o_off_e = 0U;
                gt_xdic_general_regs._r06.s_off_e = 0U;
                gt_xdic_general_regs._r06.t_off_e = 0U;
                gt_xdic_general_regs._r06.o_det_e = 1U;
                gt_xdic_general_regs._r06.s_det_e = 1U;
                gt_xdic_general_regs._r06.o_slew = 1U;
                break;
            case XDIC_ADDR_FB_LEVEL :
                gt_xdic_general_regs._r07.fb1_level = gt_xdic_fb_level[0];
                gt_xdic_general_regs._r07.fb2_level = gt_xdic_fb_level[1];
                gt_xdic_general_regs._r07.fb3_level = gt_xdic_fb_level[2];
                break;
            case XDIC_ADDR_SHORT_LEVEL :
                gt_xdic_general_regs._r08.short1_level = gt_xdic_short_level[0];
                gt_xdic_general_regs._r08.short2_level = gt_xdic_short_level[1];
                gt_xdic_general_regs._r08.short3_level = gt_xdic_short_level[2];
                break;
            case XDIC_ADDR_MAX_CURRENT_LEVEL :
                gt_xdic_general_regs._r0A.max_curr_level1 = gt_xdic_dev_max_curr_level[0];
                gt_xdic_general_regs._r0A.max_curr_level2 = gt_xdic_dev_max_curr_level[1];
                gt_xdic_general_regs._r0A.max_curr_level3 = gt_xdic_dev_max_curr_level[2];
                break;
            case XDIC_ADDR_MAX_CURRENT_VREF1 :
                gt_xdic_general_regs._r0B.max_curr_vref = XDIC_VREF;
                break;
            case XDIC_ADDR_MAX_CURRENT_VREF2 :
                gt_xdic_general_regs._r0C.max_curr_vref = XDIC_VREF;
                break;
            case XDIC_ADDR_MAX_CURRENT_VREF3 :
                gt_xdic_general_regs._r0D.max_curr_vref = XDIC_VREF;
                break;
            case XDIC_ADDR_SERIAL_BAUDRATE :
                gt_xdic_general_regs._r14.serial_clk_high = XDIC_SERIAL_CLK_CNT_HIGH;
                gt_xdic_general_regs._r14.serial_clk_low = XDIC_SERIAL_CLK_CNT_LOW;
                break;
            case XDIC_ADDR_V_MASK :
                gt_xdic_general_regs._r16.v_mask = XDIC_V_MASK_SIZE;
                break;
            case XDIC_ADDR_SV_MASK :
                gt_xdic_general_regs._r17.sv_mask = XDIC_SV_MASK_SIZE;
                gt_xdic_general_regs._r17.sv_mask_en = 1U;
                break;
            case XDIC_ADDR_FLL_CONTROL_1 :
                gt_xdic_general_regs._r1A.fllcnt = ((gn_xdic_mclk_lock_cnt >>  0U) & XDIC_MCLK_LSB_MASK);
                break;
            case XDIC_ADDR_FLL_CONTROL_2 :
                gt_xdic_general_regs._r1B.fllcnt = ((gn_xdic_mclk_lock_cnt >> 12U) & XDIC_MCLK_MSB_MASK);
                gt_xdic_general_regs._r1B.fll_range = 3U;
                gt_xdic_general_regs._r1B.fll_en = 1U;
                break;
            case XDIC_ADDR_CHOP_EN :
                gt_xdic_general_regs._r20.chop_bgr_en = 1U;
                gt_xdic_general_regs._r20.chop_dac_en = 1U;
                gt_xdic_general_regs._r20.chop_iref_en = 1U;
                gt_xdic_general_regs._r20.chop_osc_en = 1U;
                gt_xdic_general_regs._r20.chop_oscldo_en = 1U;
                gt_xdic_general_regs._r20.chop_drv_en = 1U;
                break;
            case XDIC_ADDR_TEMP :
                gt_xdic_general_regs._r21.ofs_temp = 8U;
                gt_xdic_general_regs._r21.vdd_sel = 0U;
                gt_xdic_general_regs._r21.dac_rng = 1U;
                gt_xdic_general_regs._r21.flt_ctl = 1U;
                gt_xdic_general_regs._r21.flt_gain = 2U;
                break;
            case XDIC_ADDR_OSC_FLL_MANUAL_2 :
                gt_xdic_general_regs._r23.osc_fll_man = 0x08U;
                gt_xdic_general_regs._r23.osc_fll_man_e = 0U;
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

    XDIC_Set_Delay_CH();
    XDIC_Read_All_Registers();

    print(LOG_INFO, "==================== XDIC Initialization Result ====================\r\n");

    if (gb_xdic_initial_failed == true)
    {
        print(LOG_ERROR, "XDIC Initial Failed !!\r\n");
    }
    else
    {
        print(LOG_INFO, "XDIC Initial Success !!\r\n");
    }
}

static void XDIC_Trim_Param_Init(void)
{
    for (uint8_t i = 0U ; i < 3U ; ++i)
    {
        gt_xdic_fb_level[i] = FB_LEVEL_0V45;
        gt_xdic_short_level[i] = SHORT_LEVEL_32V;
        gt_xdic_dev_max_curr_level[i] = DEV_MAX_CURR_LEVEL_4mA;
    }
}

void XDIC_Trim_Init(void)
{
    JigBD_IF_Detect_XC24();

    JigBD_IF_Select_Output_Ch(XD_CH_MAX);
    JigBD_IF_Change_Current_Gain(GAIN_HIGH);
    JigBD_IF_XD_VCC_Level(PWR_ON_5V0);
    JigBD_IF_XD_VCC_EN(PWR_ON);

    LL_mDelay(100U);

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
            case XDIC_ADDR_LD_CONTROL :
                gt_xdic_general_regs._r01.ld_type = XDIC_LD_TYPE_2;
                break;
            case XDIC_ADDR_CHANNEL_ENABLE :
                gt_xdic_general_regs._r05.val = XDIC_CHANNEL_ENABLE_MAX;
                break;
            case XDIC_ADDR_FB_LEVEL :
                gt_xdic_general_regs._r07.fb1_level = gt_xdic_fb_level[0];
                gt_xdic_general_regs._r07.fb2_level = gt_xdic_fb_level[1];
                gt_xdic_general_regs._r07.fb3_level = gt_xdic_fb_level[2];
                break;
            case XDIC_ADDR_SHORT_LEVEL :
                gt_xdic_general_regs._r08.short1_level = gt_xdic_short_level[0];
                gt_xdic_general_regs._r08.short2_level = gt_xdic_short_level[1];
                gt_xdic_general_regs._r08.short3_level = gt_xdic_short_level[2];
                break;
            case XDIC_ADDR_MAX_CURRENT_LEVEL :
                gt_xdic_general_regs._r0A.max_curr_level1 = gt_xdic_dev_max_curr_level[0];
                gt_xdic_general_regs._r0A.max_curr_level2 = gt_xdic_dev_max_curr_level[1];
                gt_xdic_general_regs._r0A.max_curr_level3 = gt_xdic_dev_max_curr_level[2];
                break;
            case XDIC_ADDR_MAX_CURRENT_VREF1 :
                gt_xdic_general_regs._r0B.max_curr_vref = 0U;
                break;
            case XDIC_ADDR_MAX_CURRENT_VREF2 :
                gt_xdic_general_regs._r0C.max_curr_vref = 0U;
                break;
            case XDIC_ADDR_MAX_CURRENT_VREF3 :
                gt_xdic_general_regs._r0D.max_curr_vref = 0U;
                break;
            case XDIC_ADDR_SERIAL_BAUDRATE :
                gt_xdic_general_regs._r14.serial_clk_high = XDIC_SERIAL_CLK_CNT_HIGH;
                gt_xdic_general_regs._r14.serial_clk_low = XDIC_SERIAL_CLK_CNT_LOW;
                break;
            case XDIC_ADDR_CHOP_EN :
                gt_xdic_general_regs._r20.chop_bgr_en = 1U;
                gt_xdic_general_regs._r20.chop_dac_en = 1U;
                gt_xdic_general_regs._r20.chop_iref_en = 1U;
                gt_xdic_general_regs._r20.chop_osc_en = 1U;
                gt_xdic_general_regs._r20.chop_oscldo_en = 1U;
                gt_xdic_general_regs._r20.chop_drv_en = 1U;
                break;
            case XDIC_ADDR_TEMP :
                gt_xdic_general_regs._r21.ofs_temp = 8U;
                gt_xdic_general_regs._r21.vdd_sel = 0U;
                gt_xdic_general_regs._r21.dac_rng = 1U;
                gt_xdic_general_regs._r21.flt_ctl = 1U;
                gt_xdic_general_regs._r21.flt_gain = 2U;
                break;
            case XDIC_ADDR_OTP_OP_MODE :
                gt_xdic_general_regs._r3F.test_en = 1U;
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
    XDIC_Read_All_Registers();

    print(LOG_INFO, "==================== XDIC Initialization Result ====================\r\n");

    if (gb_xdic_initial_failed == true)
    {
        print(LOG_ERROR, "XDIC Initial Failed !!\r\n");
    }
    else
    {
        print(LOG_INFO, "XDIC Initial Success !!\r\n");
    }
}

/* ================================================================================================================================================= */
/* General Function */
/* ================================================================================================================================================= */
static void XDIC_Set_Delay_CH(void)
{
    for (uint8_t ch = 0U ; ch < (XDIC_CH_SIZE / 2U) ; ++ch)
    {
        uint16_t channel_odd = ((((uint16_t)ch * 4U) + 0U) << 0U);
        uint16_t channel_even = ((((uint16_t)ch * 4U) + 2U) << 5U);
        uint16_t temp_delay = (uint16_t)(channel_even | channel_odd);
        XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_01_02 + (xdic_addr_t)ch, temp_delay);
    }
}

void XDIC_Set_Max_Current_Level(dev_max_curr_level_t in_dev_max_curr)
{
    gt_xdic_general_regs._r0A.max_curr_level1 = in_dev_max_curr;
    gt_xdic_general_regs._r0A.max_curr_level2 = in_dev_max_curr;
    gt_xdic_general_regs._r0A.max_curr_level3 = in_dev_max_curr;

    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_LEVEL, gt_xdic_general_regs._r0A.val);
}

float XDIC_Get_Max_Current_Level(void)
{
    float f_rtn = 0.0f;

    uint16_t max_curr_level = XDIC_Read_General_Reg(XDIC_ADDR_MAX_CURRENT_LEVEL);
    dev_max_curr_level_t dev_max_curr_lvl = (dev_max_curr_level_t)(max_curr_level & (uint16_t)0x00FU);

    switch (dev_max_curr_lvl)
    {
    case DEV_MAX_CURR_LEVEL_4mA :
        f_rtn = 4.0f;
        break;
    case DEV_MAX_CURR_LEVEL_8mA :
        f_rtn = 8.0f;
        break;
    case DEV_MAX_CURR_LEVEL_12mA :
        f_rtn = 12.0f;
        break;
    case DEV_MAX_CURR_LEVEL_16mA :
        f_rtn = 16.0f;
        break;
    case DEV_MAX_CURR_LEVEL_20mA :
        f_rtn = 20.0f;
        break;
    case DEV_MAX_CURR_LEVEL_24mA :
        f_rtn = 24.0f;
        break;
    case DEV_MAX_CURR_LEVEL_28mA :
        f_rtn = 28.0f;
        break;
    case DEV_MAX_CURR_LEVEL_32mA :
        f_rtn = 32.0f;
        break;
    case DEV_MAX_CURR_LEVEL_36mA :
        f_rtn = 36.0f;
        break;
    case DEV_MAX_CURR_LEVEL_40mA :
        f_rtn = 40.0f;
        break;
    default :
        print(LOG_ERROR, "ERROR: %s - dev_max_curr_lvl(%d) Over !!\r\n", __func__, dev_max_curr_lvl);
        f_rtn = 0.0f;
        break;
    }
    print(LOG_INFO, "Max Current Level : %.3f\r\n", f_rtn);
    return f_rtn;
}

void XDIC_Set_FB_Level(fb_level_t in_fb_level)
{
    gt_xdic_general_regs._r07.fb1_level = in_fb_level;
    gt_xdic_general_regs._r07.fb2_level = in_fb_level;
    gt_xdic_general_regs._r07.fb3_level = in_fb_level;

    XDIC_Write_General_Reg(XDIC_ADDR_FB_LEVEL, gt_xdic_general_regs._r07.val);
}

float XDIC_Get_FB_Level(void)
{
    float f_rtn = 0.0f;

    uint16_t fb_level = XDIC_Read_General_Reg(XDIC_ADDR_FB_LEVEL);
    fb_level_t fb_level_ch = (fb_level_t)(fb_level & (uint16_t)0x007U);
    print(LOG_INFO, "XDIC FB Level Reg Value : 0x%03X, Level : %d\r\n", fb_level, fb_level_ch);

    switch (fb_level_ch)
    {
    case FB_LEVEL_0V45 :
        f_rtn = 0.45f;
        break;
    case FB_LEVEL_0V55 :
        f_rtn = 0.55f;
        break;
    case FB_LEVEL_0V65 :
        f_rtn = 0.65f;
        break;
    case FB_LEVEL_0V75 :
        f_rtn = 0.75f;
        break;
    case FB_LEVEL_0V90 :
        f_rtn = 0.90f;
        break;
    case FB_LEVEL_1V05 :
        f_rtn = 1.05f;
        break;
    case FB_LEVEL_1V20 :
        f_rtn = 1.20f;
        break;
    case FB_LEVEL_1V35 :
        f_rtn = 1.35f;
        break;
    default :
        print(LOG_ERROR, "ERROR: %s - fb_level_ch(%d) Over !!\r\n", __func__, fb_level_ch);
        f_rtn = 0.0f;
        break;
    }
    print(LOG_INFO, "FB Level : %.3f\r\n", f_rtn);
    return f_rtn;
}

void XDIC_Set_Short_Level(short_level_t in_short_level)
{
    gt_xdic_general_regs._r08.short1_level = in_short_level;
    gt_xdic_general_regs._r08.short2_level = in_short_level;
    gt_xdic_general_regs._r08.short3_level = in_short_level;

    XDIC_Write_General_Reg(XDIC_ADDR_SHORT_LEVEL, gt_xdic_general_regs._r08.val);
}

float XDIC_Get_Short_Level(void)
{
    float f_rtn = 0.0f;

    uint16_t short_level = XDIC_Read_General_Reg(XDIC_ADDR_SHORT_LEVEL);
    short_level_t short_level_ch = (short_level_t)(short_level & (uint16_t)0x007U);
    print(LOG_INFO, "XDIC Short Level Reg Value : 0x%03X, Level : %d\r\n", short_level, short_level_ch);

    switch (short_level_ch)
    {
    case SHORT_LEVEL_3V5 :
        f_rtn = 3.5f;
        break;
    case SHORT_LEVEL_4V5 :
        f_rtn = 4.5f;
        break;
    case SHORT_LEVEL_6V5 :
        f_rtn = 6.5f;
        break;
    case SHORT_LEVEL_9V0 :
        f_rtn = 9.0f;
        break;
    case SHORT_LEVEL_13V5 :
        f_rtn = 13.5f;
        break;
    case SHORT_LEVEL_17V5 :
        f_rtn = 17.5f;
        break;
    case SHORT_LEVEL_26V :
        f_rtn = 26.0f;
        break;
    case SHORT_LEVEL_32V :
        f_rtn = 32.0f;
        break;
    default :
        print(LOG_ERROR, "ERROR: %s - short_level_ch(%d) Over !!\r\n", __func__, short_level_ch);
        f_rtn = 0.0f;
        break;
    }
    print(LOG_INFO, "Short Level : %.3f\r\n", f_rtn);
    return f_rtn;
}

void XDIC_Set_Max_Curr_Vref(uint16_t in_max_curr_vref)
{
    gt_xdic_general_regs._r0B.max_curr_vref = in_max_curr_vref;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF1, gt_xdic_general_regs._r0B.val);

    gt_xdic_general_regs._r0C.max_curr_vref = in_max_curr_vref;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF2, gt_xdic_general_regs._r0C.val);

    gt_xdic_general_regs._r0D.max_curr_vref = in_max_curr_vref;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF3, gt_xdic_general_regs._r0D.val);
}

static void XDIC_Set_OSC_Manual_Enable(bool en)
{
    if (en == true)
    {
        gt_xdic_general_regs._r23.osc_fll_man_e = 1U;
    }
    else
    {
        gt_xdic_general_regs._r23.osc_fll_man_e = 0U;
    }
    XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_2, gt_xdic_general_regs._r23.val);
}

static void XDIC_Set_OSC_Manual(uint16_t osc_manual)
{
    uint16_t osc_manual_lsb = ((osc_manual >>  0U) & 0xFFFU);
    uint16_t osc_manual_msb = ((osc_manual >> 12U) & 0x00FU);

    gt_xdic_general_regs._r22.osc_fll_man = osc_manual_lsb;
    gt_xdic_general_regs._r23.osc_fll_man = osc_manual_msb;

    XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_1, gt_xdic_general_regs._r22.val);
    XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_2, gt_xdic_general_regs._r23.val);
}
/* ================================================================================================================================================= */
/* Trim Function */
/* ================================================================================================================================================= */
void XDIC_Overwrite_Mirror_Regs(void)
{
    uint16_t u16_otp_crc = XDIC_Get_Mirror_Reg(XDIC_MIRROR_ADDR_OTP_CRC);
    if (u16_otp_crc != 0U)
    {
        print(LOG_INFO, "XDIC Already Trimmed!!!!\r\n");
    }
    else
    {
        for (xdic_mirror_addr_t addr = XDIC_MIRROR_ADDR_OTP_CRC ; addr < XDIC_MIRROR_ADDR_MAX ; ++addr)
        {
            XDIC_Write_Mirror_Reg(addr, gn_xdic_saved_trim_reg[addr]);
        }
    }
}

void XDIC_Display_Mirror_Regs(void)
{
    print(LOG_INFO, "DAC_OFS,0x%03X\r\n", gt_xdic_mirror_regs._r01.val);
    print(LOG_INFO, "IREF_CTL_BGR_TC,0x%03X\r\n", gt_xdic_mirror_regs._r02.val);
    print(LOG_INFO, "LDO_DAC_CTL_OSC_RCTL,0x%03X\r\n", gt_xdic_mirror_regs._r03.val);
    print(LOG_INFO, "LDO_CTL,0x%03X\r\n", gt_xdic_mirror_regs._r04.val);

    print(LOG_INFO, "OFS_CH,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n",\
        gt_xdic_mirror_regs._r05.val, gt_xdic_mirror_regs._r06.val, gt_xdic_mirror_regs._r07.val, gt_xdic_mirror_regs._r08.val,\
        gt_xdic_mirror_regs._r09.val, gt_xdic_mirror_regs._r0A.val, gt_xdic_mirror_regs._r0B.val, gt_xdic_mirror_regs._r0C.val,\
        gt_xdic_mirror_regs._r0D.val, gt_xdic_mirror_regs._r0E.val, gt_xdic_mirror_regs._r0F.val, gt_xdic_mirror_regs._r10.val);
    print(LOG_INFO, "GAIN_CH,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n",\
        gt_xdic_mirror_regs._r11.val, gt_xdic_mirror_regs._r12.val, gt_xdic_mirror_regs._r13.val, gt_xdic_mirror_regs._r14.val,\
        gt_xdic_mirror_regs._r15.val, gt_xdic_mirror_regs._r16.val, gt_xdic_mirror_regs._r17.val, gt_xdic_mirror_regs._r18.val,\
        gt_xdic_mirror_regs._r19.val, gt_xdic_mirror_regs._r1A.val, gt_xdic_mirror_regs._r1B.val, gt_xdic_mirror_regs._r1C.val);
}

void XDIC_Save_Mirror_Regs(void)
{
    for (xdic_mirror_addr_t addr = XDIC_MIRROR_ADDR_OTP_CRC ; addr < XDIC_MIRROR_ADDR_MAX ; ++addr)
    {
        gn_xdic_saved_trim_reg[addr] = XDIC_Read_Mirror_Reg(addr);
    }
}

uint64_t XDIC_Compare_Mirror_Regs(void)
{
    uint64_t ret = 0U;
    uint16_t u16_reg_val = 0U;

    for (xdic_mirror_addr_t mirror_addr = XDIC_MIRROR_ADDR_DAC_OFS ; mirror_addr < XDIC_MIRROR_ADDR_MAX ; ++mirror_addr)
    {
        u16_reg_val = XDIC_Read_Mirror_Reg(mirror_addr);

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

void XDIC_Trim_Init_IBN_2uA(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 0U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
}

void XDIC_Trim_Init_DAC_LDO_1V5(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 1U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
}

void XDIC_Trim_Init_DIG_LDO_1V5(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 2U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
}

void XDIC_Trim_Init_DAC_A_OFS(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 3U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    gt_xdic_general_regs._r0B.max_curr_vref = 300U;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF1, gt_xdic_general_regs._r0B.val);

    gt_xdic_general_regs._r0C.max_curr_vref = 300U;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF2, gt_xdic_general_regs._r0C.val);

    gt_xdic_general_regs._r0D.max_curr_vref = 300U;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF3, gt_xdic_general_regs._r0D.val);
}

void XDIC_Trim_Init_DAC_B_OFS(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 4U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    gt_xdic_general_regs._r0B.max_curr_vref = 300U;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF1, gt_xdic_general_regs._r0B.val);

    gt_xdic_general_regs._r0C.max_curr_vref = 300U;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF2, gt_xdic_general_regs._r0C.val);

    gt_xdic_general_regs._r0D.max_curr_vref = 300U;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURRENT_VREF3, gt_xdic_general_regs._r0D.val);
}

void XDIC_Trim_Init_FLL_LDO_1V5(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 5U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
}

void XDIC_Trim_Init_OSC(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 0U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 1U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_OSC_Manual_Enable(true);
    XDIC_Set_OSC_Manual(0x8000U);
}

void XDIC_Trim_Init_CH_GAIN_ODD(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 0U;
    gt_xdic_general_regs._r3F.pwm_full_o = 1U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_Max_Current_Level(XDIC_GAIN_ODD_MAX_CURRENT_LVL);
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
}

void XDIC_Trim_Init_CH_GAIN_EVEN(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 0U;
    gt_xdic_general_regs._r3F.pwm_full_o = 1U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_Max_Current_Level(XDIC_GAIN_EVEN_MAX_CURRENT_LVL);
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
}

void XDIC_Trim_Init_CH_OFS_ODD(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 0U;
    gt_xdic_general_regs._r3F.pwm_full_o = 1U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_Max_Current_Level(XDIC_OFS_ODD_MAX_CURRENT_LVL);
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
}

void XDIC_Trim_Init_CH_OFS_EVEN(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 0U;
    gt_xdic_general_regs._r3F.pwm_full_o = 1U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_Max_Current_Level(XDIC_OFS_EVEN_MAX_CURRENT_LVL);
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
}

void XDIC_Trim_Init_UVOV_1P5(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 6U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
}

void XDIC_Trim_Init_UVOV_VDD(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 7U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 0U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
}

void XDIC_Trim_Partial_IBN_2uA(void)
{
    XDIC_Trim_Init_IBN_2uA();
    JigBD_IF_Start_MCU_ADC();
    uint16_t mcu_adc =  JigBD_IF_Get_MCU_ADC();
    print(LOG_INFO, "\r\n IBN_2uA : %.3f\r\n", JigBD_IF_Convert_MCU_ADC_To_Volt(mcu_adc));
}

void XDIC_Trim_Partial_DAC_LDO_1V5(void)
{
    XDIC_Trim_Init_DAC_LDO_1V5();
    JigBD_IF_Start_MCU_ADC();
    uint16_t mcu_adc =  JigBD_IF_Get_MCU_ADC();
    print(LOG_INFO, "\r\n DAC_LDO_1V5 : %.3f\r\n", JigBD_IF_Convert_MCU_ADC_To_Volt(mcu_adc));
}

void XDIC_Trim_Partial_DIG_LDO_1V5(void)
{
    XDIC_Trim_Init_DIG_LDO_1V5();
    JigBD_IF_Start_MCU_ADC();
    uint16_t mcu_adc =  JigBD_IF_Get_MCU_ADC();
    print(LOG_INFO, "\r\n DIG_LDO_1V5 : %.3f\r\n", JigBD_IF_Convert_MCU_ADC_To_Volt(mcu_adc));
}

void XDIC_Trim_Partial_DAC_A_OFS(void)
{
    XDIC_Trim_Init_DAC_A_OFS();
    JigBD_IF_Start_MCU_ADC();
    uint16_t mcu_adc =  JigBD_IF_Get_MCU_ADC();
    print(LOG_INFO, "\r\n DAC_A_OFS : %.3f\r\n", JigBD_IF_Convert_MCU_ADC_To_Volt(mcu_adc));
}

void XDIC_Trim_Partial_DAC_B_OFS(void)
{
    XDIC_Trim_Init_DAC_B_OFS();
    JigBD_IF_Start_MCU_ADC();
    uint16_t mcu_adc =  JigBD_IF_Get_MCU_ADC();
    print(LOG_INFO, "\r\n DAC_B_OFS : %.3f\r\n", JigBD_IF_Convert_MCU_ADC_To_Volt(mcu_adc));
}

void XDIC_Trim_Partial_FLL_LDO_1V5(void)
{
    XDIC_Trim_Init_FLL_LDO_1V5();
    JigBD_IF_Start_MCU_ADC();
    uint16_t mcu_adc =  JigBD_IF_Get_MCU_ADC();
    print(LOG_INFO, "\r\n FLL_LDO_1V5 : %.3f\r\n", JigBD_IF_Convert_MCU_ADC_To_Volt(mcu_adc));
}

void XDIC_Trim_Partial_OSC(void)
{
    XDIC_Trim_Init_OSC();
    JigBD_IF_Start_Input_Capture();
    JigBD_IF_Wait_Input_Capture_Done();
    JigBD_IF_Stop_Input_Capture();
    float osc_freq = JigBD_IF_Get_Input_Capture_Freq() * XDIC_CONST_FREQ_DIVIDE / CONST_MHz_TO_Hz;
    print(LOG_INFO, "\r\n OSC Freq : %.3f [MHz]\r\n", osc_freq);
}

void XDIC_Trim_Partial_CH_GAIN(void)
{
    current_gain_t current_gain = GAIN_HIGH;

    XDIC_Trim_Init_CH_GAIN_ODD();
    JigBD_IF_VLED_9V_EN(PWR_ON);
    JigBD_IF_Change_Current_Gain(current_gain);

    for (uint8_t i = 0U ; i < XDIC_CH_SIZE ; ++i)
    {
        uint16_t iout_adc[XDIC_CH_SIZE][2] = { { 0 } };
        float iout_float[XDIC_CH_SIZE][2] = { { 0.0f } };

        JigBD_IF_Select_Output_Ch(i);

        XDIC_Set_Max_Curr_Vref(1500U); // XDIC_GAIN_P1
        ADS114S08_Set_Start(1U);
        ADS114S08_Wait_Done();
        iout_adc[i][0] = ADS114S08_Get_ADC_Value();

        XDIC_Set_Max_Curr_Vref(300U); // XDIC_GAIN_P2
        ADS114S08_Set_Start(1U);
        ADS114S08_Wait_Done();
        iout_adc[i][1] = ADS114S08_Get_ADC_Value();

        uint16_t adc_average = iout_adc[i][0] - iout_adc[i][1]; //delta
        float iout_avg = JigBD_IF_Convert_Adc_To_Current(adc_average, current_gain);
        iout_float[i][0] = JigBD_IF_Convert_Adc_To_Current(iout_adc[i][0], current_gain);
        iout_float[i][1] = JigBD_IF_Convert_Adc_To_Current(iout_adc[i][1], current_gain);
        print(LOG_INFO, "GAIN CH[%d] : Delta %.3f, P1 %.3f, P2 %.3f\r\n", (i + 1U), iout_avg, iout_float[i][0], iout_float[i][1]);
    }
}

void XDIC_Trim_Partial_CH_OFS(void)
{
    current_gain_t current_gain = GAIN_MID;

    XDIC_Trim_Init_CH_OFS_ODD();
    JigBD_IF_VLED_9V_EN(PWR_ON);
    JigBD_IF_Change_Current_Gain(current_gain);

    for (uint8_t i = 0U ; i < XDIC_CH_SIZE ; ++i)
    {
        uint16_t iout_adc[XDIC_CH_SIZE][2] = { { 0 } };
        float iout_float[XDIC_CH_SIZE][2] = { { 0.0f } };

        JigBD_IF_Select_Output_Ch(i);

        XDIC_Set_Max_Curr_Vref(200U); // XDIC_OFS_P1
        ADS114S08_Set_Start(1U);
        ADS114S08_Wait_Done();
        iout_adc[i][0] = ADS114S08_Get_ADC_Value();

        XDIC_Set_Max_Curr_Vref(400U); // XDIC_OFS_P2
        ADS114S08_Set_Start(1U);
        ADS114S08_Wait_Done();
        iout_adc[i][1] = ADS114S08_Get_ADC_Value();

        uint16_t adc_average = (iout_adc[i][0] + iout_adc[i][1]) / 2U; //avg
        float iout_avg = JigBD_IF_Convert_Adc_To_Current(adc_average, current_gain);
        iout_float[i][0] = JigBD_IF_Convert_Adc_To_Current(iout_adc[i][0], current_gain);
        iout_float[i][1] = JigBD_IF_Convert_Adc_To_Current(iout_adc[i][1], current_gain);
        print(LOG_INFO, "OFS CH[%d], AVG, %.3f, P1, %.3f, P2, %.3f\r\n", (i + 1U), iout_avg, iout_float[i][0], iout_float[i][1]);
    }
}

void XDIC_Trim_Partial_UVOV_1P5(void)
{
    uint16_t ldo_ov = 0xFFFFU;
    uint16_t ldo_uv = 0xFFFFU;

    XDIC_Trim_Init_UVOV_1P5();
    uint16_t fault_status = XDIC_Read_General_Reg(XDIC_ADDR_FAULT_STATUS);

    ldo_ov = ((fault_status >> XDIC_R09_LDO_OV_SHIFT) & XDIC_R09_LDO_OV_MASK);
    ldo_uv = ((fault_status >> XDIC_R09_LDO_UV_SHIFT) & XDIC_R09_LDO_UV_MASK);
    print(LOG_INFO, "\r\n UVOV_1P5 Fault Status : 0x%03X, LDO_OV : %d, LDO_UV : %d\r\n", fault_status, ldo_ov, ldo_uv);
}

void XDIC_Trim_Partial_UVOV_VDD(void)
{
    uint16_t vdd_ov = 0xFFFFU;
    uint16_t vdd_uv = 0xFFFFU;

    XDIC_Trim_Init_UVOV_VDD();
    uint16_t fault_status = XDIC_Read_General_Reg(XDIC_ADDR_FAULT_STATUS);

    vdd_ov = ((fault_status >> XDIC_R09_VDD_OV_SHIFT) & XDIC_R09_VDD_OV_MASK);
    vdd_uv = ((fault_status >> XDIC_R09_VDD_UV_SHIFT) & XDIC_R09_VDD_UV_MASK);
    print(LOG_INFO, "\r\n UVOV_VDD Fault Status : 0x%03X, VDD_OV : %d, VDD_UV : %d\r\n", fault_status, vdd_ov, vdd_uv);
}

void XDIC_Trim_Show_OSC(void)
{
    gt_xdic_general_regs._r3F.test_en = 1U;
    gt_xdic_general_regs._r3F.test_ana_en = 0U;
    gt_xdic_general_regs._r3F.pwm_full_o = 0U;
    gt_xdic_general_regs._r3F.mclk64_o = 1U;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
    LL_GPIO_ResetOutputPin(XD_FREQ_RESET_GPIO_Port, XD_FREQ_RESET_Pin);
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
        gt_xdic_general_regs._r3D.otp_pg_start = 1U;
    }
    else
    {
        gt_xdic_general_regs._r3D.otp_pg_start = 0U;
    }
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_RD_PROG, gt_xdic_general_regs._r3D.val);
}