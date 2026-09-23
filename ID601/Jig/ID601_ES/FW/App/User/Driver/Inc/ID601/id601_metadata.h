/* USER CODE BEGIN Header */
/*
    * File:   id601_metadata.h
    * Author: GT
    *
    * Created on 2026. 09. 22.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ID601_METADATA_H__
#define __ID601_METADATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define ID601_INTERNAL_MCLK (50000000UL)    /* 50.0MHz */
#define ID601_MODEL_NAME       "ID601 ES0"
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum tag_ID601_ADDR_GENERAL
{
    ID601_RESET_ID = 0x00U,         /* 0x00 */
    ID601_LD_CONTROL,               /* 0x01 */
    ID601_LD_SIZE,                  /* 0x02 */
    ID601_PWMCLK_DIV1_2,            /* 0x03 */
    ID601_PWMCLK_DIV2_3,            /* 0x04 */
    ID601_CHANNEL_ENABLE,           /* 0x05 */
    ID601_FAULT_CONTROL,            /* 0x06 */
    ID601_FB_LEVEL,                 /* 0x07 */
    ID601_SHORT_LEVEL,              /* 0x08 */
    ID601_FAULT_STATUS0,            /* 0x09 */
    ID601_FAULT_STATUS1,            /* 0x0A */
    ID601_FAULT_STATUS2,            /* 0x0B */
    ID601_FAULT_STATUS3,            /* 0x0C */
    ID601_MAX_CURR_LEVEL,           /* 0x0D */
    ID601_MAX_CURR_VREF1,           /* 0x0E */
    ID601_MAX_CURR_VREF2,           /* 0x0F */

    ID601_MAX_CURR_VREF3 = 0x10U,   /* 0x10 */
    ID601_CH_LD_TYPE,               /* 0x11 */
    ID601_DELAY_CH1_2,              /* 0x12 */
    ID601_DELAY_CH3_4,              /* 0x13 */
    ID601_DELAY_CH5_6,              /* 0x14 */
    ID601_DELAY_CH7_8,              /* 0x15 */
    ID601_DELAY_CH9_10,             /* 0x16 */
    ID601_DELAY_CH11_12,            /* 0x17 */
    ID601_FAULT_CONFIGURATION,      /* 0x18 */
    ID601_BIST,                     /* 0x19 */
    ID601_TEST_MODE0,               /* 0x1A */
    ID601_TEST_MODE1,               /* 0x1B */
    ID601_SERIAL_CLK_GEN,           /* 0x1C */
    ID601_SERIAL_LATENCY,           /* 0x1D */
    ID601_V_MASK,                   /* 0x1E */
    ID601_SV_MASK,                  /* 0x1F */

    ID601_RSTCNT = 0x20U,           /* 0x20 */
    ID601_TIMEOUT,                  /* 0x21 */
    ID601_FLLCNT1,                  /* 0x22 */
    ID601_FLLCNT2,                  /* 0x23 */
    ID601_WR_PROTECT,               /* 0x24 */
    ID601_NF_CONTROL,               /* 0x25 */
    ID601_CHOP_EN,                  /* 0x26 */
    ID601_TEMP,                     /* 0x27 */
    ID601_OSC_FLL_MAN1,             /* 0x28 */
    ID601_OSC_FLL_MAN2,             /* 0x29 */
    ID601_OSC_SPREAD,               /* 0x2A */
    ID601_CLOCK_GATE_EN,            /* 0x2B */
    ID601_VREF_FIX1,                /* 0x2C */
    ID601_VREF_FIX2,                /* 0x2D */
    ID601_VREF_FIX3,                /* 0x2E */

    ID601_TEST_ANA_EN = 0x39U,      /* 0x39 */
    ID601_OTP_ACCESS1,              /* 0x3A */
    ID601_OTP_ACCESS2,              /* 0x3B */
    ID601_OTP_WRITE,                /* 0x3C */
    ID601_OTP_RD_PROG,              /* 0x3D */
    ID601_OTP_PROTECT,              /* 0x3E */
    ID601_OP_MODE,                  /* 0x3F */

    ID601_MAX,
} id601_addr_general_t;

typedef enum tag_ID601_ADDR_MIRROR
{
    ID601_MIRROR1 = 0x00U,          /* 0x00 */
    ID601_MIRROR2,                  /* 0x01 */
    ID601_MIRROR3,                  /* 0x02 */
    ID601_MIRROR4,                  /* 0x03 */
    ID601_MIRROR5,                  /* 0x04 */
    ID601_MIRROR6,                  /* 0x05 */
    ID601_MIRROR_OFS_CH01,          /* 0x06 */
    ID601_MIRROR_OFS_CH02,          /* 0x07 */
    ID601_MIRROR_OFS_CH03,          /* 0x08 */
    ID601_MIRROR_OFS_CH04,          /* 0x09 */
    ID601_MIRROR_OFS_CH05,          /* 0x0A */
    ID601_MIRROR_OFS_CH06,          /* 0x0B */
    ID601_MIRROR_OFS_CH07,          /* 0x0C */
    ID601_MIRROR_OFS_CH08,          /* 0x0D */
    ID601_MIRROR_OFS_CH09,          /* 0x0E */
    ID601_MIRROR_OFS_CH10,          /* 0x0F */
    ID601_MIRROR_OFS_CH11,          /* 0x10 */
    ID601_MIRROR_OFS_CH12,          /* 0x11 */

    ID601_MIRROR_GAIN_CH01 = 0x12U, /* 0x12 */
    ID601_MIRROR_GAIN_CH02,         /* 0x13 */
    ID601_MIRROR_GAIN_CH03,         /* 0x14 */
    ID601_MIRROR_GAIN_CH04,         /* 0x15 */
    ID601_MIRROR_GAIN_CH05,         /* 0x16 */
    ID601_MIRROR_GAIN_CH06,         /* 0x17 */
    ID601_MIRROR_GAIN_CH07,         /* 0x18 */
    ID601_MIRROR_GAIN_CH08,         /* 0x19 */
    ID601_MIRROR_GAIN_CH09,         /* 0x1A */
    ID601_MIRROR_GAIN_CH10,         /* 0x1B */
    ID601_MIRROR_GAIN_CH11,         /* 0x1C */
    ID601_MIRROR_GAIN_CH12,         /* 0x1D */
    ID601_MIRROR_VERSION,           /* 0x1E */

    ID601_MIRROR_MAX,
} id601_addr_mirror_t;

typedef union tag_ID601_DUMMY
{
    struct
    {
        uint16_t dummy :12;
        uint16_t       : 4;
    } bit;
} _id601_dummy_t;

typedef union tag_ID601_RESET_ID
{
    struct
    {
        uint16_t id     : 5;
        uint16_t        : 3;
        uint16_t lkg_e  : 1;
        uint16_t e_rst  : 1;
        uint16_t vs_rst : 1;
        uint16_t rst    : 1;
        uint16_t        : 4;
    } bit;
} _id601_reset_id_t;

typedef union tag_ID601_LD_CONTROL
{
    struct
    {
        uint16_t ld_mode     : 1;
        uint16_t ld_dir      : 1;
        uint16_t pwm_res     : 1;
        uint16_t syncmode    : 1;
        uint16_t delay_ch_en : 1;
        uint16_t ld_type     : 1;
        uint16_t sv_no       : 6;
        uint16_t             : 4;
    } bit;
} _id601_ld_control_t;

typedef union tag_ID601_LD_SIZE
{
    struct
    {
        uint16_t ld_size : 6;
        uint16_t         :10;
    } bit;
} _id601_ld_size_t;

typedef union tag_ID601_PWMCLK_DIV1_2
{
    struct
    {
        uint16_t fpwm_div1 : 8;
        uint16_t fpwm_div2 : 4;
        uint16_t           : 4;
    } bit;
} _id601_pwmclk_div1_2_t;

typedef union tag_ID601_PWMCLK_DIV2_3
{
    struct
    {
        uint16_t fpwm_div2 : 4;
        uint16_t fpwm_div3 : 8;
        uint16_t           : 4;
    } bit;
} _id601_pwmclk_div2_3_t;

typedef union tag_ID601_CHANNEL_ENABLE
{
    struct
    {
        uint16_t ch1_en  : 1;
        uint16_t ch2_en  : 1;
        uint16_t ch3_en  : 1;
        uint16_t ch4_en  : 1;
        uint16_t ch5_en  : 1;
        uint16_t ch6_en  : 1;
        uint16_t ch7_en  : 1;
        uint16_t ch8_en  : 1;
        uint16_t ch9_en  : 1;
        uint16_t ch10_en : 1;
        uint16_t ch11_en : 1;
        uint16_t ch12_en : 1;
        uint16_t         : 4;
    } bit;
} _id601_channel_enable_t;

typedef union tag_ID601_FAULT_CONTROL0
{
    struct
    {
        uint16_t o_off_e  : 1;
        uint16_t s_off_e  : 1;
        uint16_t t_off_e  : 1;
        uint16_t o_det_e  : 1;
        uint16_t s_det_e  : 1;
        uint16_t t_det_e  : 1;
        uint16_t o_fb_e   : 1;
        uint16_t fb_mode  : 2;
        uint16_t ft_mode  : 2;
        uint16_t          : 5;
    } bit;
} _id601_fault_control0_t;

typedef union tag_ID601_FB_LEVEL
{
    struct
    {
        uint16_t fb1_level : 3;
        uint16_t fb2_level : 3;
        uint16_t fb3_level : 3;
        uint16_t           : 7;
    } bit;
} _id601_fb_level_t;

typedef union tag_ID601_SHORT_LEVEL
{
    struct
    {
        uint16_t short1_level : 3;
        uint16_t short2_level : 3;
        uint16_t short3_level : 3;
        uint16_t              : 7;
    } bit;
} _id601_short_level_t;

typedef union tag_ID601_FAULT_STATUS0
{
    struct
    {
        uint16_t bit_fb1   : 1;
        uint16_t bit_fb2   : 1;
        uint16_t bit_fb3   : 1;
        uint16_t bit_open  : 1;
        uint16_t bit_short : 1;
        uint16_t           :11;
    } bit;
} _id601_fault_status0_t;

typedef union tag_ID601_FAULT_STATUS1
{
    struct
    {
        uint16_t thermal          : 1;
        uint16_t ldo_uv           : 1;
        uint16_t ldo_ov           : 1;
        uint16_t osc_ldo_uv       : 1;
        uint16_t osc_ldo_ov       : 1;
        uint16_t vref_uv          : 1;
        uint16_t vref_ov          : 1;
        uint16_t ch_ctrl_mismatch : 1;
        uint16_t vs_miss          : 1;
        uint16_t wd_timeout       : 1;
        uint16_t cmd_err          : 1;
        uint16_t resp_err         : 1;
        uint16_t                  : 4;
    } bit;
} _id601_fault_status1_t;

typedef union tag_ID601_FAULT_STATUS2
{
    struct
    {
        uint16_t open_ch1  : 1;
        uint16_t open_ch2  : 1;
        uint16_t open_ch3  : 1;
        uint16_t open_ch4  : 1;
        uint16_t open_ch5  : 1;
        uint16_t open_ch6  : 1;
        uint16_t open_ch7  : 1;
        uint16_t open_ch8  : 1;
        uint16_t open_ch9  : 1;
        uint16_t open_ch10 : 1;
        uint16_t open_ch11 : 1;
        uint16_t open_ch12 : 1;
        uint16_t           : 4;
    } bit;
} _id601_fault_status2_t;

typedef union tag_ID601_FAULT_STATUS3
{
    struct
    {
        uint16_t short_ch1  : 1;
        uint16_t short_ch2  : 1;
        uint16_t short_ch3  : 1;
        uint16_t short_ch4  : 1;
        uint16_t short_ch5  : 1;
        uint16_t short_ch6  : 1;
        uint16_t short_ch7  : 1;
        uint16_t short_ch8  : 1;
        uint16_t short_ch9  : 1;
        uint16_t short_ch10 : 1;
        uint16_t short_ch11 : 1;
        uint16_t short_ch12 : 1;
        uint16_t            : 4;
    } bit;
} _id601_fault_status3_t;

typedef union tag_ID601_MAX_CURRENT_LEVEL
{
    struct
    {
        uint16_t max_curr_level1 : 4;
        uint16_t max_curr_level2 : 4;
        uint16_t max_curr_level3 : 4;
        uint16_t                 : 4;
    } bit;
} _id601_max_current_level_t;

typedef union tag_ID601_MAX_CURR_VREF
{
    struct
    {
        uint16_t max_curr_vref :12;
        uint16_t               : 4;
    } bit;
} _id601_max_curr_vref_t;

typedef union tag_ID601_CHX_LD_TYPE
{
    struct
    {
        uint16_t ch1_ld_type  : 1;
        uint16_t ch2_ld_type  : 1;
        uint16_t ch3_ld_type  : 1;
        uint16_t ch4_ld_type  : 1;
        uint16_t ch5_ld_type  : 1;
        uint16_t ch6_ld_type  : 1;
        uint16_t ch7_ld_type  : 1;
        uint16_t ch8_ld_type  : 1;
        uint16_t ch9_ld_type  : 1;
        uint16_t ch10_ld_type : 1;
        uint16_t ch11_ld_type : 1;
        uint16_t ch12_ld_type : 1;
        uint16_t              : 4;
    } bit;
} _id601_chx_ld_type_t;

typedef union tag_ID601_DELAY_CH
{
    struct
    {
        uint16_t delay_ch_x1 : 5;
        uint16_t delay_ch_x2 : 5;
        uint16_t             : 6;
    } bit;
} _id601_delay_ch_t;

typedef union tag_ID601_FAULT_CONFIGURATION
{
    struct
    {
        uint16_t ldo_ovuv_en          : 1;
        uint16_t osc_ldo_ovuv_en      : 1;
        uint16_t vref_ovuv_en         : 1;
        uint16_t vref_ovuv_off_en     : 1;
        uint16_t ch_ctrl_mismatch_en  : 1;
        uint16_t vs_miss_en           : 1;
        uint16_t vs_miss_dimm_en      : 1;
        uint16_t wd_en                : 1;
        uint16_t wd_timecfg           : 2;
        uint16_t wd_dimm_en           : 1;
        uint16_t crc_en               : 1;
        uint16_t                      : 4;
    } bit;
} _id601_fault_configuration_t;

typedef union tag_ID601_BIST
{
    struct
    {
        uint16_t bist_err   : 1;
        uint16_t bist_done  : 1;
        uint16_t bist_busy  : 1;
        uint16_t            : 8;
        uint16_t bist_start : 1;
        uint16_t            : 4;
    } bit;
} _id601_bist_t;

typedef union tag_ID601_TEST_MODE0
{
    struct
    {
        uint16_t dmux_sel : 5;
        uint16_t          :11;
    } bit;
} _id601_test_mode0_t;

typedef union tag_ID601_TEST_MODE1
{
    struct
    {
        uint16_t tm_ch_ctrl_input_flip : 1;
        uint16_t                       :10;
        uint16_t testmode              : 1;
        uint16_t                       : 4;
    } bit;
} _id601_test_mode1_t;

typedef union tag_ID601_SERIAL_CLK_GEN
{
    struct
    {
        uint16_t serial_clk_high : 6;
        uint16_t serial_clk_low  : 6;
        uint16_t                 : 4;
    } bit;
} _id601_serial_clk_gen_t;

typedef union tag_ID601_SERIAL_LATENCY
{
    struct
    {
        uint16_t serial_latency : 9;
        uint16_t                : 7;
    } bit;
} _id601_serial_latency_t;

typedef union tag_ID601_V_MASK
{
    struct
    {
        uint16_t v_mask :10;
        uint16_t        : 6;
    } bit;
} _id601_v_mask_t;

typedef union tag_ID601_SV_MASK
{
    struct
    {
        uint16_t sv_mask    :10;
        uint16_t            : 1;
        uint16_t sv_mask_en : 1;
        uint16_t            : 4;
    } bit;
} _id601_sv_mask_t;

typedef union tag_ID601_RSTCNT
{
    struct
    {
        uint16_t rstcnt :10;
        uint16_t        : 6;
    } bit;
} _id601_rstcnt_t;

typedef union tag_ID601_TIMEOUT
{
    struct
    {
        uint16_t timeout :11;
        uint16_t         : 5;
    } bit;
} _id601_timeout_t;

typedef union tag_ID601_FLLCNT1
{
    struct
    {
        uint16_t fllcnt :12;
        uint16_t        : 4;
    } bit;
} _id601_fllcnt1_t;

typedef union tag_ID601_FLLCNT2
{
    struct
    {
        uint16_t fllcnt_high : 8;
        uint16_t             : 1;
        uint16_t fll_range   : 2;
        uint16_t fll_en      : 1;
        uint16_t             : 4;
    } bit;
} _id601_fllcnt2_t;

typedef union tag_ID601_WR_PROTECT
{
    struct
    {
        uint16_t wr_protect :12;
        uint16_t            : 4;
    } bit;
} _id601_wr_protect_t;

typedef union tag_ID601_NF_CONTROL
{
    struct
    {
        uint16_t dgrjt_en1    : 1;
        uint16_t dgrjt_en2    : 1;
        uint16_t bbkn_en      : 1;
        uint16_t sgrjt_en1    : 1;
        uint16_t sgrjt_en2    : 1;
        uint16_t o_emi_rej_en : 1;
        uint16_t bbkn_th      : 6;
        uint16_t              : 4;
    } bit;
} _id601_nf_control_t;

typedef union tag_ID601_CHOP_EN
{
    struct
    {
        uint16_t chop_bgr_en     : 1;
        uint16_t chop_dac_en     : 1;
        uint16_t chop_osc_en     : 1;
        uint16_t chop_osc_ldo_en : 1;
        uint16_t chop_drv_en     : 1;
        uint16_t chop_en         : 1;
        uint16_t                 : 5;
        uint16_t open_mask_opt   : 1;
        uint16_t                 : 1;
    } bit;
} _id601_chop_en_t;

typedef union tag_ID601_TEMP
{
    struct
    {
        uint16_t flt_gain   : 2;
        uint16_t o_slew     : 2;
        uint16_t flt_ctl    : 2;
        uint16_t dac_rng    : 1;
        uint16_t ov_swap_en : 1;
        uint16_t            : 8;
    } bit;
} _id601_temp_t;

typedef union tag_ID601_OSC_FLL_MAN1
{
    struct
    {
        uint16_t osc_fll_man :12;
        uint16_t             : 4;
    } bit;
} _id601_osc_fll_man1_t;

typedef union tag_ID601_OSC_FLL_MAN2
{
    struct
    {
        uint16_t osc_fll_man       : 4;
        uint16_t osc_fll_err_range : 2;
        uint16_t                   : 5;
        uint16_t osc_man_en        : 1;
        uint16_t                   : 4;
    } bit;
} _id601_osc_fll_man2_t;

typedef union tag_ID601_OSC_SPREAD
{
    struct
    {
        uint16_t sprd_gain : 3;
        uint16_t           : 1;
        uint16_t sprd_spd  : 3;
        uint16_t           : 4;
        uint16_t sprd_en   : 1;
        uint16_t           : 4;
    } bit;
} _id601_osc_spread_t;

typedef union tag_ID601_CLOCK_GATE_EN
{
    struct
    {
        uint16_t dc_mclk_en  : 1;
        uint16_t fr1_mclk_en : 1;
        uint16_t fr2_mclk_en : 1;
        uint16_t             : 1;
        uint16_t otp_mclk_en : 1;
        uint16_t             :11;
    } bit;
} _id601_clock_gate_en_t;

typedef union tag_ID601_VREF_FIX
{
    struct
    {
        uint16_t vref_fix :12;
        uint16_t          : 4;
    } bit;
} _id601_vref_fix_t;

typedef union tag_ID601_TEST_ANA_EN
{
    struct
    {
        uint16_t test_ana_en : 5;
        uint16_t             :11;
    } bit;
} _id601_test_ana_en_t;

typedef union tag_ID601_OTP_ACCESS1
{
    struct
    {
        uint16_t otp_pg_acc_cycle : 4;
        uint16_t                  :12;
    } bit;
} _id601_otp_access1_t;

typedef union tag_ID601_OTP_ACCESS2
{
    struct
    {
        uint16_t otp_pg_acc_cycle :12;
        uint16_t                  : 4;
    } bit;
} _id601_otp_access2_t;

typedef union tag_ID601_OTP_WRITE
{
    struct
    {
        uint16_t otp_wsel : 4;
        uint16_t otp_rd   : 2;
        uint16_t          :10;
    } bit;
} _id601_otp_write_t;

typedef union tag_ID601_OTP_RD_PROG
{
    struct
    {
        uint16_t otp_pg_s    : 1;
        uint16_t otp_rd_s    : 1;
        uint16_t             : 9;
        uint16_t otp_pg_done : 1;
        uint16_t             : 4;
    } bit;
} _id601_otp_rd_prog_t;

typedef union tag_ID601_OTP_PROTECT
{
    struct
    {
        uint16_t protect_en :12;
        uint16_t            : 4;
    } bit;
} _id601_otp_protect_t;

typedef union tag_ID601_OP_MODE
{
    struct
    {
        uint16_t addr_ext     : 1;
        uint16_t ext_clkin    : 1;
        uint16_t rd_en        : 1;
        uint16_t dtest_mux_en : 1;
        uint16_t mclk64_o     : 1;
        uint16_t pwmout_full  : 1;
        uint16_t sw_sel       : 2;
        uint16_t              : 3;
        uint16_t test_en      : 1;
        uint16_t              : 4;
    } bit;
} _id601_op_mode_t;

typedef union tag_ID601_OTP_MIRROR1
{
    struct
    {
        uint16_t otp_crc_checksum : 8;
        uint16_t                  : 8;
    } bit;
} _id601_otp_mirror1_t;

typedef union tag_ID601_OTP_MIRROR2
{
    struct
    {
        uint16_t bgr_tc   : 5;
        uint16_t iref_ctl : 5;
        uint16_t          : 6;
    } bit;
} _id601_otp_mirror2_t;

typedef union tag_ID601_OTP_MIRROR3
{
    struct
    {
        uint16_t bgr2_tc   : 5;
        uint16_t iref2_ctl : 5;
        uint16_t           : 6;
    } bit;
} _id601_otp_mirror3_t;

typedef union tag_ID601_OTP_MIRROR4
{
    struct
    {
        uint16_t ldo_bgr2_ctl : 5;
        uint16_t              : 3;
        uint16_t ofs_temp     : 4;
        uint16_t              : 4;
    } bit;
} _id601_otp_mirror4_t;

typedef union tag_ID601_OTP_MIRROR5
{
    struct
    {
        uint16_t osc_rctl    : 5;
        uint16_t             : 2;
        uint16_t ldo_dac_ctl : 5;
        uint16_t             : 4;
    } bit;
} _id601_otp_mirror5_t;

typedef union tag_ID601_OTP_MIRROR6
{
    struct
    {
        uint16_t ldo_ctl     : 4;
        uint16_t ldo_osc_ctl : 4;
        uint16_t             : 8;
    } bit;
} _id601_otp_mirror6_t;

typedef union tag_ID601_OTP_MIRROR_OFS
{
    struct
    {
        uint16_t ofs_ch : 9;
        uint16_t        : 7;
    } bit;
} _id601_otp_mirror_ofs_t;

typedef union tag_ID601_OTP_MIRROR_GAIN
{
    struct
    {
        uint16_t gain_ch : 7;
        uint16_t         : 9;
    } bit;
} _id601_otp_mirror_gain_t;

typedef union tag_ID601_OTP_MIRROR_VERSION
{
    struct
    {
        uint16_t version0 : 8;
        uint16_t          : 8;
    } bit;
} _id601_otp_mirror_version_t;

typedef struct tag_ID601_GENERAL_REGS
{
    _id601_reset_id_t               _r00;
    _id601_ld_control_t             _r01;
    _id601_ld_size_t                _r02;
    _id601_pwmclk_div1_2_t          _r03;
    _id601_pwmclk_div2_3_t          _r04;
    _id601_channel_enable_t         _r05;
    _id601_fault_control0_t         _r06;
    _id601_fb_level_t               _r07;
    _id601_short_level_t            _r08;
    _id601_fault_status0_t          _r09;
    _id601_fault_status1_t          _r0A;
    _id601_fault_status2_t          _r0B;
    _id601_fault_status3_t          _r0C;
    _id601_max_current_level_t      _r0D;
    _id601_max_curr_vref_t          _r0E;
    _id601_max_curr_vref_t          _r0F;

    _id601_max_curr_vref_t          _r10;
    _id601_chx_ld_type_t            _r11;
    _id601_delay_ch_t               _r12;
    _id601_delay_ch_t               _r13;
    _id601_delay_ch_t               _r14;
    _id601_delay_ch_t               _r15;
    _id601_delay_ch_t               _r16;
    _id601_delay_ch_t               _r17;
    _id601_fault_configuration_t    _r18;
    _id601_bist_t                   _r19;
    _id601_test_mode0_t             _r1A;
    _id601_test_mode1_t             _r1B;
    _id601_serial_clk_gen_t         _r1C;
    _id601_serial_latency_t         _r1D;
    _id601_v_mask_t                 _r1E;
    _id601_sv_mask_t                _r1F;
    _id601_rstcnt_t                 _r20;
    _id601_timeout_t                _r21;
    _id601_fllcnt1_t                _r22;
    _id601_fllcnt2_t                _r23;
    _id601_wr_protect_t             _r24;
    _id601_nf_control_t             _r25;
    _id601_chop_en_t                _r26;
    _id601_temp_t                   _r27;
    _id601_osc_fll_man1_t           _r28;
    _id601_osc_fll_man2_t           _r29;
    _id601_osc_spread_t             _r2A;
    _id601_clock_gate_en_t          _r2B;
    _id601_vref_fix_t               _r2C;
    _id601_vref_fix_t               _r2D;
    _id601_vref_fix_t               _r2E;
    _id601_dummy_t                  _r2F;

    _id601_dummy_t                  _r30;
    _id601_dummy_t                  _r31;
    _id601_dummy_t                  _r32;
    _id601_dummy_t                  _r33;
    _id601_dummy_t                  _r34;
    _id601_dummy_t                  _r35;
    _id601_dummy_t                  _r36;
    _id601_dummy_t                  _r37;
    _id601_dummy_t                  _r38;
    _id601_test_ana_en_t            _r39;
    _id601_otp_access1_t            _r3A;
    _id601_otp_access2_t            _r3B;
    _id601_otp_write_t              _r3C;
    _id601_otp_rd_prog_t            _r3D;
    _id601_otp_protect_t            _r3E;
    _id601_op_mode_t                _r3F;
} _id601_general_regs_t;

typedef struct tag_ID601_MIRROR_REGS
{
    _id601_otp_mirror1_t        _r00;
    _id601_otp_mirror2_t        _r01;
    _id601_otp_mirror3_t        _r02;
    _id601_otp_mirror4_t        _r03;
    _id601_otp_mirror5_t        _r04;
    _id601_otp_mirror6_t        _r05;
    _id601_otp_mirror_ofs_t     _r06;
    _id601_otp_mirror_ofs_t     _r07;
    _id601_otp_mirror_ofs_t     _r08;
    _id601_otp_mirror_ofs_t     _r09;
    _id601_otp_mirror_ofs_t     _r0A;
    _id601_otp_mirror_ofs_t     _r0B;
    _id601_otp_mirror_ofs_t     _r0C;
    _id601_otp_mirror_ofs_t     _r0D;
    _id601_otp_mirror_ofs_t     _r0E;
    _id601_otp_mirror_ofs_t     _r0F;

    _id601_otp_mirror_ofs_t     _r10;
    _id601_otp_mirror_ofs_t     _r11;
    _id601_otp_mirror_gain_t    _r12;
    _id601_otp_mirror_gain_t    _r13;
    _id601_otp_mirror_gain_t    _r14;
    _id601_otp_mirror_gain_t    _r15;
    _id601_otp_mirror_gain_t    _r16;
    _id601_otp_mirror_gain_t    _r17;
    _id601_otp_mirror_gain_t    _r18;
    _id601_otp_mirror_gain_t    _r19;
    _id601_otp_mirror_gain_t    _r1A;
    _id601_otp_mirror_gain_t    _r1B;
    _id601_otp_mirror_gain_t    _r1C;
    _id601_otp_mirror_gain_t    _r1D;
    _id601_otp_mirror_version_t _r1E;
} _id601_mirror_regs_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __ID601_METADATA_H__ */
