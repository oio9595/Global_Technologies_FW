/** @file xdic.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XDIC_C__

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "xdic.h"
#include "JigBd_IF.h"
#include "config.h"
#include "uart.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define XDIC_GENERAL_REG_ENTRY(addr, reg)   { addr, #addr, &gt_xdic_general_regs.reg }

#define XDIC_REG_GENERAL            (0)

#define XDIC_OTP_PROTECT_DISABLE    (0xA5A)
#define XDIC_OTP_PROTECT_ENABLE     (0x5A5)

#define XDIC_LD_MODE_NORMAL         (0)
#define XDIC_LD_MODE_X8             (1)

#define XDIC_LD_DIR_HEAD_SHIFT      (0)
#define XDIC_LD_DIR_TAIL_SHIFT      (1)

#define XDIC_LD_TYPE_NOP            (0) // NOP
#define XDIC_LD_TYPE_1              (1) // RGB, 12CH
#define XDIC_LD_TYPE_2              (2) // R/GB, 6CH
#define XDIC_LD_TYPE_3              (3) // MONO, 12CH

#define XDIC_PWM_RES_12BIT          (0)
#define XDIC_PWM_RES_14BIT          (1)

#define XDIC_SYNC_MODE_CMD          (0)
#define XDIC_SYNC_MODE_SVI          (1)

#define XDIC_VREF_MODE_VREF2_3      (0)
#define XDIC_VREF_MODE_VREF2        (1)

#define XDIC_PARITY_CHECK_DIS       (0)
#define XDIC_PARITY_CHECK_EN        (1)

#define XDIC_MCLK_FLL_ENABLE        (0)
#define XDIC_MCLK_FLL_DISABLE       (1)

#define XDIC_MCLK_LSB_MASK          (0x000FFFU) //LSB 12-bit
#define XDIC_MCLK_MSB_MASK          (0x1FF000U) //MSB  9-bit

#define XDIC_WR_PWM_DIV_VREF        (0xAAAU)
#define XDIC_WR_OTHERS              (0x555U)

#define XDIC_PWM_DIV_RED            (6U)
#define XDIC_PWM_DIV_GREEN          (6U)
#define XDIC_PWM_DIV_BLUE           (6U)

#define XDIC_V_MASK_SIZE            (410)
#define XDIC_SV_MASK_SIZE           (570)

#define XDIC_MCLK_LOCK_CNT          (450000U)
#define XDIC_VREF                   (4095U)

#define XDIC_CHANNEL_ENABLE_MAX     ((1U << XDIC_CH_SIZE) - 1)
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct tag_XDIC_REG_INFO_T
{
    uint8_t address;
    const char* name;
    void* reg_ptr;
} _reg_map_t;
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static _xdic_general_regs_t gt_xdic_general_regs;

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
static_assert(XDIC_ADDR_MAX == (sizeof(gt_xdic_general_maps) / sizeof(_reg_map_t)), "XDIC General Address map mismatch!");

/* Variable for XD Registers */
static float gf_xdic_mclk;
static float gf_vsync_out;

static uint32_t gn_xdic_mclk_lock_cnt;

static dev_max_curr_level_t gt_xdic_dev_max_curr_level[3] = { DEV_MAX_CURR_LEVEL_4mA, };
static short_level_t gt_xdic_short_level[3] = { SHORT_LEVEL_3V5, };
static fb_level_t gt_xdic_fb_level[3] = { FB_LEVEL_0V45, };
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
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
    MCU_IF_Write_XDIC(XDIC_ADDR_WR_PROTECT, gt_xdic_general_regs._r1C.val);
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
        if (/*addr == 0x02 || */addr == 0x03 || addr == 0x04 || addr == 0x0B || addr == 0x0C || addr == 0x0D)
        {
            XDIC_Set_Writeable_Type(true);
            *((uint16_t*)(map->reg_ptr)) = data;
            MCU_IF_Write_XDIC(addr, data);
            XDIC_Set_Writeable_Type(false);
        }
        else
        {
            *((uint16_t*)(map->reg_ptr)) = data;
            MCU_IF_Write_XDIC(addr, data);
        }
    }
    else
    {
        Print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
    }
}

uint16_t XDIC_Read_General_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    const _reg_map_t* map = XDIC_Get_General_Map_Pointer(addr);
    if (map)
    {
        xdic_reg_val = MCU_IF_Read_XDIC(addr);
        *((uint16_t*)(map->reg_ptr)) = xdic_reg_val;
        Print(LOG_DEBUG, "XDIC General Read --> [ 0x%02X - 0x%04X] \r\n", addr, xdic_reg_val);
    }
    else
    {
        Print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
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
        Print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, addr);
        return xdic_reg_val;
    }

    return xdic_reg_val;
}

static void XDIC_Dump_All_Registers(void)
{
    Print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XDIC_ADDR_MAX ; ++xd_general_addr)
    {
        const _reg_map_t* map = XDIC_Get_General_Map_Pointer(xd_general_addr);
        if (map)
        {
            Print(LOG_INFO, "[ %-40s 0x%02X | 0x%04X | %-6u ]\r\n", map->name, map->address, *((uint16_t*)(map->reg_ptr)), *((uint16_t*)(map->reg_ptr)));
        }
    }
    Print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
}

static void XDIC_Set_Delay_CH(void)
{
    for (uint8_t ch = 0 ; ch < (XDIC_CH_SIZE / 2) ; ++ch)
    {
        uint16_t temp_delay = (((ch * 2 + 1) << 5) | ((ch * 2 + 0) << 0));
        XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_01_02 + ch, temp_delay);
    }
}

static void XDIC_Param_Init(void)
{
    gf_xdic_mclk = XDIC_MCLK;
    gf_vsync_out = VSYNC;

    //gn_xdic_mclk_lock_cnt = (uint32_t)(gf_xdic_mclk / gf_vsync_out + 0.5f);
    gn_xdic_mclk_lock_cnt = XDIC_MCLK_LOCK_CNT;

    for (uint8_t i = 0 ; i < 3 ; ++i)
    {
        gt_xdic_fb_level[i] = FB_LEVEL_0V45;
        gt_xdic_short_level[i] = SHORT_LEVEL_32V;
        gt_xdic_dev_max_curr_level[i] = DEV_MAX_CURR_LEVEL_4mA;
    }
}

void XDIC_Init(void)
{
    XDIC_Param_Init();

    MCU_IF_Write_XDIC(XDIC_ADDR_RESET_ID, (1 << 11));
    MCU_IF_IdGen_Command();

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
                gt_xdic_general_regs._r02.fpwm_div1 = (XDIC_PWM_DIV_RED >> 8);
                gt_xdic_general_regs._r02.sv_no = SVSYNC_SIZE;
                break;
            case XDIC_ADDR_FPWM_DIVIDER_1_2 :
                gt_xdic_general_regs._r03.fpwm_div1 = (XDIC_PWM_DIV_RED & 0xFF);
                gt_xdic_general_regs._r03.fpwm_div2 = ((XDIC_PWM_DIV_GREEN & 0x0F) >> 0);
                break;
            case XDIC_ADDR_FPWM_DIVIDER_2_3 :
                gt_xdic_general_regs._r04.fpwm_div2 = ((XDIC_PWM_DIV_GREEN & 0xF0) >> 4);
                gt_xdic_general_regs._r04.fpwm_div3 = XDIC_PWM_DIV_BLUE;
                break;
            case XDIC_ADDR_CHANNEL_ENABLE :
                gt_xdic_general_regs._r05.val = XDIC_CHANNEL_ENABLE_MAX;
                break;
            case XDIC_ADDR_FAULT_CONTROL :
                gt_xdic_general_regs._r06.o_off_e = 0;
                gt_xdic_general_regs._r06.s_off_e = 0;
                gt_xdic_general_regs._r06.t_off_e = 0;
                gt_xdic_general_regs._r06.o_det_e = 1;
                gt_xdic_general_regs._r06.s_det_e = 1;
                gt_xdic_general_regs._r06.o_slew = 1;
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
                gt_xdic_general_regs._r17.sv_mask_en = 1;
                break;
            case XDIC_ADDR_FLL_CONTROL_1 :
                gt_xdic_general_regs._r1A.fllcnt = ((gn_xdic_mclk_lock_cnt & XDIC_MCLK_LSB_MASK) >>  0);
                break;
            case XDIC_ADDR_FLL_CONTROL_2 :
                gt_xdic_general_regs._r1B.fllcnt = ((gn_xdic_mclk_lock_cnt & XDIC_MCLK_MSB_MASK) >> 12);
                gt_xdic_general_regs._r1B.fll_range = 3;
                gt_xdic_general_regs._r1B.fll_en = 1;
                break;
            case XDIC_ADDR_CHOP_EN :
                gt_xdic_general_regs._r20.chop_bgr_en = 1;
                gt_xdic_general_regs._r20.chop_dac_en = 1;
                gt_xdic_general_regs._r20.chop_iref_en = 1;
                gt_xdic_general_regs._r20.chop_osc_en = 1;
                gt_xdic_general_regs._r20.chop_oscldo_en = 1;
                gt_xdic_general_regs._r20.chop_drv_en = 1;
                break;
            case XDIC_ADDR_TEMP :
                gt_xdic_general_regs._r21.ofs_temp = 8;
                gt_xdic_general_regs._r21.vdd_sel = 0;
                gt_xdic_general_regs._r21.dac_rng = 1;
                gt_xdic_general_regs._r21.flt_ctl = 1;
                gt_xdic_general_regs._r21.flt_gain = 2;
                break;
            case XDIC_ADDR_OSC_FLL_MANUAL_2 :
                gt_xdic_general_regs._r23.osc_fll_man = 0x08;
                gt_xdic_general_regs._r23.osc_fll_man_e = 0;
                break;
            default :
                continue;
            }
            XDIC_Write_General_Reg(xdic_addr, *((uint16_t*)(map->reg_ptr)));
        }
        else
        {
            Print(LOG_ERROR, "ERROR: Register 0x%02X not initialized (map missing)\r\n", xdic_addr);
        }
    }
    XDIC_Set_Delay_CH();
    XDIC_Dump_All_Registers();
}
/* USER CODE END */

/*** end of file ***/