/** @file xdic.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XDIC_H__
#define __XDIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_XDIC_CHANNEL_T
{
    XD_CH_01 = 0,
    XD_CH_02,
    XD_CH_03,
    XD_CH_04,
    XD_CH_05,
    XD_CH_06,
    XD_CH_07,
    XD_CH_08,
    XD_CH_09,
    XD_CH_10,
    XD_CH_11,
    XD_CH_12,
    XD_CH_MAX,
} XD_CH_t;

typedef enum tag_XDIC_DEV_MAX_CURRENT_LEVEL_T
{
    DEV_MAX_CURR_LEVEL_4mA = 0,     // 4b'0000
    DEV_MAX_CURR_LEVEL_8mA,         // 4b'0001
    DEV_MAX_CURR_LEVEL_12mA,        // 4b'0010
    DEV_MAX_CURR_LEVEL_16mA,        // 4b'0011
    DEV_MAX_CURR_LEVEL_20mA,        // 4b'0100

    DEV_MAX_CURR_LEVEL_24mA = 11,   // 4b'1011
    DEV_MAX_CURR_LEVEL_28mA = 12,   // 4b'1100
    DEV_MAX_CURR_LEVEL_32mA = 13,   // 4b'1101
    DEV_MAX_CURR_LEVEL_36mA = 14,   // 4b'1110
    DEV_MAX_CURR_LEVEL_40mA = 15,   // 4b'1111
    DEV_MAX_CURR_LEVEL_MAX,
} dev_max_curr_level_t;

typedef enum tag_XDIC_SHORT_LEVEL_T
{
    SHORT_LEVEL_3V5 = 0, // 3b'000
    SHORT_LEVEL_4V5,     // 3b'001
    SHORT_LEVEL_6V5,     // 3b'010
    SHORT_LEVEL_9V0,     // 3b'011
    SHORT_LEVEL_13V5,    // 3b'100
    SHORT_LEVEL_17V5,    // 3b'101
    SHORT_LEVEL_26V,     // 3b'110
    SHORT_LEVEL_32V,     // 3b'111
    SHORT_LEVEL_MAX,
} short_level_t;

typedef enum tag_XDIC_FB_LEVEL_T
{
    FB_LEVEL_0V45 = 0,  // 3b'000
    FB_LEVEL_0V55,      // 3b'001
    FB_LEVEL_0V65,      // 3b'010
    FB_LEVEL_0V75,      // 3b'011
    FB_LEVEL_0V90,      // 3b'100
    FB_LEVEL_1V05,      // 3b'101
    FB_LEVEL_1V20,      // 3b'110
    FB_LEVEL_1V35,      // 3b'111
    FB_LEVEL_MAX,
} fb_level_t;

typedef union tag_XDIC_DUMMY_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy : 16;
    };
} _xdic_dummy_t;

typedef union tag_XDIC_GENERAL_0x00_T
{
    uint16_t val;
    struct
    {
        uint16_t id         : 5;
        uint16_t            : 4;
        uint16_t e_rst      : 1;
        uint16_t vs_rst     : 1;
        uint16_t rst        : 1;
        uint16_t            : 4;
    };
} _xdic_reset_id_t;

typedef union tag_XDIC_GENERAL_0x01_T
{
    uint16_t val;
    struct
    {
        uint16_t ld_mode    : 1;
        uint16_t ld_dir     : 1;
        uint16_t ld_type    : 2;
        uint16_t pwm_res    : 1;
        uint16_t syncmode   : 1;
        uint16_t vrefmode   : 1;
        uint16_t pc_en      : 1;
        uint16_t ld_size    : 4;
        uint16_t            : 4;
    };
} _xdic_ld_control_t;

typedef union tag_XDIC_GENERAL_0x02_T
{
    uint16_t val;
    struct
    {
        uint16_t sv_no      : 6;
        uint16_t            : 5;
        uint16_t fpwm_div1  : 1;
        uint16_t            : 4;
    };
} _xdic_svsync_num_t;

typedef union tag_XDIC_GENERAL_0x03_T
{
    uint16_t val;
    struct
    {
        uint16_t fpwm_div1  : 8;
        uint16_t fpwm_div2  : 4;
        uint16_t            : 4;
    };
} _xdic_fpwm_divider_1_2_t;

typedef union tag_XDIC_GENERAL_0x04_T
{
    uint16_t val;
    struct
    {
        uint16_t fpwm_div2  : 4;
        uint16_t fpwm_div3  : 8;
        uint16_t            : 4;
    };
} _xdic_fpwm_divider_2_3_t;

typedef union tag_XDIC_GENERAL_0x05_T
{
    uint16_t val;
    struct
    {
        uint16_t ch01_en : 1;
        uint16_t ch02_en : 1;
        uint16_t ch03_en : 1;
        uint16_t ch04_en : 1;
        uint16_t ch05_en : 1;
        uint16_t ch06_en : 1;
        uint16_t ch07_en : 1;
        uint16_t ch08_en : 1;
        uint16_t ch09_en : 1;
        uint16_t ch10_en : 1;
        uint16_t ch11_en : 1;
        uint16_t ch12_en : 1;
        uint16_t         : 4;
    };
} _xdic_channel_enable_t;

typedef union tag_XDIC_GENERAL_0x06_T
{
    uint16_t val;
    struct
    {
        uint16_t o_off_e        : 1;
        uint16_t s_off_e        : 1;
        uint16_t t_off_e        : 1;
        uint16_t o_det_e        : 1;
        uint16_t s_det_e        : 1;
        uint16_t fb_mode        : 1;
        uint16_t o_slew         : 2;
        uint16_t                : 8;
    };
} _xdic_fault_control_t;

typedef union tag_XDIC_GENERAL_0x07_T
{
    uint16_t val;
    struct
    {
        uint16_t fb1_level      : 3;
        uint16_t fb2_level      : 3;
        uint16_t fb3_level      : 3;
        uint16_t                : 7;
    };
} _xdic_fb_level_t;

typedef union tag_XDIC_GENERAL_0x08_T
{
    uint16_t val;
    struct
    {
        uint16_t short1_level   : 3;
        uint16_t short2_level   : 3;
        uint16_t short3_level   : 3;
        uint16_t                : 7;
    };
} _xdic_short_level_t;

typedef union tag_XDIC_GENERAL_0x09_T
{
    uint16_t val;
    struct
    {
        uint16_t bit_fb1            : 1;
        uint16_t bit_fb2            : 1;
        uint16_t bit_fb3            : 1;
        uint16_t bit_open           : 1;
        uint16_t bit_short          : 1;
        uint16_t bit_thermal        : 1;
        uint16_t bit_ldo_uv         : 1;
        uint16_t bit_ldo_ov         : 1;
        uint16_t bit_vdd_uv         : 1;
        uint16_t bit_vdd_ov         : 1;
        uint16_t                    : 5;
        uint16_t bit_timeout_err    : 1;
        uint16_t                    : 4;
    };
} _xdic_fault_status_t;

typedef union tag_XDIC_GENERAL_0x0A_T
{
    uint16_t val;
    struct
    {
        uint16_t max_curr_level1   : 4;
        uint16_t max_curr_level2   : 4;
        uint16_t max_curr_level3   : 4;
        uint16_t                   : 4;
    };
} _xdic_max_current_level_t;

typedef union tag_XDIC_GENERAL_0x0B_0x0D_T
{
    uint16_t val;
    struct
    {
        uint16_t max_curr_vref  : 12;
        uint16_t                :  4;
    };
} _xdic_max_current_vref_t;

typedef union tag_XDIC_GENERAL_0x0E_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch01 : 5;
        uint16_t delay_ch02 : 5;
        uint16_t            : 6;
    };
} _xdic_delay_ch_1_2_t;

typedef union tag_XDIC_GENERAL_0x0F_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch03 : 5;
        uint16_t delay_ch04 : 5;
        uint16_t            : 6;
    };
} _xdic_delay_ch_3_4_t;

typedef union tag_XDIC_GENERAL_0x10_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch05 : 5;
        uint16_t delay_ch06 : 5;
        uint16_t            : 6;
    };
} _xdic_delay_ch_5_6_t;

typedef union tag_XDIC_GENERAL_0x11_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch07 : 5;
        uint16_t delay_ch08 : 5;
        uint16_t            : 6;
    };
} _xdic_delay_ch_7_8_t;

typedef union tag_XDIC_GENERAL_0x12_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch09 : 5;
        uint16_t delay_ch10 : 5;
        uint16_t            : 6;
    };
} _xdic_delay_ch_9_10_t;

typedef union tag_XDIC_GENERAL_0x13_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch11 : 5;
        uint16_t delay_ch12 : 5;
        uint16_t            : 6;
    };
} _xdic_delay_ch_11_12_t;

typedef union tag_XDIC_GENERAL_0x14_T
{
    uint16_t val;
    struct
    {
        uint16_t serial_clk_high : 6;
        uint16_t serial_clk_low  : 6;
        uint16_t                 : 4;
    };
} _xdic_serial_baudrate_t;

typedef union tag_XDIC_GENERAL_0x15_T
{
    uint16_t val;
    struct
    {
        uint16_t serial_latency : 8;
        uint16_t                : 8;
    };
} _xdic_serial_latency_t;

typedef union tag_XDIC_GENERAL_0x16_T
{
    uint16_t val;
    struct
    {
        uint16_t v_mask : 10;
        uint16_t        :  6;
    };
} _xdic_v_mask_t;

typedef union tag_XDIC_GENERAL_0x17_T
{
    uint16_t val;
    struct
    {
        uint16_t sv_mask    : 10;
        uint16_t            :  1;
        uint16_t sv_mask_en :  1;
        uint16_t            :  4;
    };
} _xdic_sv_mask_t;

typedef union tag_XDIC_GENERAL_0x18_T
{
    uint16_t val;
    struct
    {
        uint16_t rstcnt : 11;
        uint16_t        :  5;
    };
} _xdic_rstcnt_t;

typedef union tag_XDIC_GENERAL_0x19_T
{
    uint16_t val;
    struct
    {
        uint16_t timeout : 10;
        uint16_t         :  6;
    };
} _xdic_timeout_t;

typedef union tag_XDIC_GENERAL_0x1A_T
{
    uint16_t val;
    struct
    {
        uint16_t fllcnt : 12;
        uint16_t        :  4;
    };
} _xdic_fll_control_1_t;

typedef union tag_XDIC_GENERAL_0x1B_T
{
    uint16_t val;
    struct
    {
        uint16_t fllcnt     : 9;
        uint16_t fll_range  : 2;
        uint16_t fll_en     : 1;
        uint16_t            : 4;
    };
} _xdic_fll_control_2_t;

typedef union tag_XDIC_GENERAL_0x1C_T
{
    uint16_t val;
    struct
    {
        uint16_t wr_protect : 12;
        uint16_t            :  4;
    };
} _xdic_wr_protect_t;

typedef union tag_XDIC_GENERAL_0x1D_T
{
    uint16_t val;
    struct
    {
        uint16_t dgrjt_en1  : 1;
        uint16_t dgrjt_en2  : 1;
        uint16_t bbkn_en    : 1;
        uint16_t sgrjt_en1  : 1;
        uint16_t sgrjt_en2  : 1;
        uint16_t emi_rej_en : 1;
        uint16_t bbkn_th    : 6;
        uint16_t            : 4;
    };
} _xdic_nf_control_t;

typedef union tag_XDIC_GENERAL_0x20_T
{
    uint16_t val;
    struct
    {
        uint16_t chop_bgr_en    :  1;
        uint16_t chop_dac_en    :  1;
        uint16_t chop_iref_en   :  1;
        uint16_t chop_osc_en    :  1;
        uint16_t chop_oscldo_en :  1;
        uint16_t chop_drv_en    :  1;
        uint16_t                : 10;
    };
} _xdic_chop_en_t;

typedef union tag_XDIC_GENERAL_0x21_T
{
    uint16_t val;
    struct
    {
        uint16_t flt_gain   : 2;
        uint16_t            : 2;
        uint16_t flt_ctl    : 2;
        uint16_t dac_rng    : 1;
        uint16_t vdd_sel    : 1;
        uint16_t ofs_temp   : 4;
        uint16_t            : 4;
    };
} _xdic_temp_t;

typedef union tag_XDIC_GENERAL_0x22_T
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_man    : 12;
        uint16_t                :  4;
    };
} _xdic_osc_fll_manual_1_t;

typedef union tag_XDIC_GENERAL_0x23_T
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_man    : 4;
        uint16_t                : 7;
        uint16_t osc_fll_man_e  : 1;
        uint16_t                : 4;
    };
} _xdic_osc_fll_manual_2_t;

typedef union tag_XDIC_GENERAL_0x24_T
{
    uint16_t val;
    struct
    {
        uint16_t sprd_gain  : 3;
        uint16_t            : 1;
        uint16_t sprd_speed : 3;
        uint16_t            : 4;
        uint16_t sprd_en    : 1;
        uint16_t            : 4;
    };
} _xdic_spread_t;

typedef union tag_XDIC_GENERAL_0x3A_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_access_cycle :  4;
        uint16_t                     : 12;
    };
} _xdic_otp_access_1_t;

typedef union tag_XDIC_GENERAL_0x3B_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_access_cycle : 12;
        uint16_t                     :  4;
    };
} _xdic_otp_access_2_t;

typedef union tag_XDIC_GENERAL_0x3C_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_wsel : 4;
        uint16_t otp_rd   : 2;
        uint16_t          : 10;
    };
} _xdic_otp_write_t;

typedef union tag_XDIC_GENERAL_0x3D_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_start : 1;
        uint16_t otp_rd_start : 1;
        uint16_t              : 9;
        uint16_t otp_pg_done  : 1;
        uint16_t              : 4;
    };
} _xdic_otp_rd_prog_t;

typedef union tag_XDIC_GENERAL_0x3E_T
{
    uint16_t val;
    struct
    {
        uint16_t protect_en : 12;
        uint16_t            :  4;
    };
} _xdic_otp_protect_t;

typedef union tag_XDIC_GENERAL_0x3F_T
{
    uint16_t val;
    struct
    {
        uint16_t addr_ext       : 1;
        uint16_t ext_clkin      : 1;
        uint16_t ofs_rng        : 1;
        uint16_t                : 1;
        uint16_t mclk64_o       : 1;
        uint16_t pwm_full_o     : 1;
        uint16_t                : 2;
        uint16_t test_ana_en    : 3;
        uint16_t test_en        : 1;
        uint16_t                : 4;
    };
} _xdic_op_mode_t;

typedef enum tag_XDIC_GENERAL_ADDR_T
{
    XDIC_ADDR_RESET_ID              = 0x00, // 0x00
    XDIC_ADDR_LD_CONTROL            = 0x01, // 0x01
    XDIC_ADDR_SVSYNC_NUM            = 0x02, // 0x02
    XDIC_ADDR_FPWM_DIVIDER_1_2      = 0x03, // 0x03
    XDIC_ADDR_FPWM_DIVIDER_2_3      = 0x04, // 0x04
    XDIC_ADDR_CHANNEL_ENABLE        = 0x05, // 0x05
    XDIC_ADDR_FAULT_CONTROL         = 0x06, // 0x06
    XDIC_ADDR_FB_LEVEL              = 0x07, // 0x07
    XDIC_ADDR_SHORT_LEVEL           = 0x08, // 0x08
    XDIC_ADDR_FAULT_STATUS          = 0x09, // 0x09
    XDIC_ADDR_MAX_CURRENT_LEVEL     = 0x0A, // 0x0A
    XDIC_ADDR_MAX_CURRENT_VREF1     = 0x0B, // 0x0B
    XDIC_ADDR_MAX_CURRENT_VREF2     = 0x0C, // 0x0C
    XDIC_ADDR_MAX_CURRENT_VREF3     = 0x0D, // 0x0D
    XDIC_ADDR_DELAY_CH_01_02        = 0x0E, // 0x0E
    XDIC_ADDR_DELAY_CH_03_04        = 0x0F, // 0x0F
    XDIC_ADDR_DELAY_CH_05_06        = 0x10, // 0x10
    XDIC_ADDR_DELAY_CH_07_08        = 0x11, // 0x11
    XDIC_ADDR_DELAY_CH_09_10        = 0x12, // 0x12
    XDIC_ADDR_DELAY_CH_11_12        = 0x13, // 0x13
    XDIC_ADDR_SERIAL_BAUDRATE       = 0x14, // 0x14
    XDIC_ADDR_SERIAL_LATENCY        = 0x15, // 0x15
    XDIC_ADDR_V_MASK                = 0x16, // 0x16
    XDIC_ADDR_SV_MASK               = 0x17, // 0x17
    XDIC_ADDR_RSTCNT                = 0x18, // 0x18
    XDIC_ADDR_TIMEOUT               = 0x19, // 0x19
    XDIC_ADDR_FLL_CONTROL_1         = 0x1A, // 0x1A
    XDIC_ADDR_FLL_CONTROL_2         = 0x1B, // 0x1B
    XDIC_ADDR_WR_PROTECT            = 0x1C, // 0x1C
    XDIC_ADDR_NF_CONTROL            = 0x1D, // 0x1D
    XDIC_ADDR_DUMMY_1E              = 0x1E, // 0x1E
    XDIC_ADDR_DUMMY_1F              = 0x1F, // 0x1F
    XDIC_ADDR_CHOP_EN               = 0x20, // 0x20
    XDIC_ADDR_TEMP                  = 0x21, // 0x21
    XDIC_ADDR_OSC_FLL_MANUAL_1      = 0x22, // 0x22
    XDIC_ADDR_OSC_FLL_MANUAL_2      = 0x23, // 0x23
    XDIC_ADDR_SPREAD                = 0x24, // 0x24
    XDIC_ADDR_DUMMY_25              = 0x25, // 0x25
    XDIC_ADDR_DUMMY_26              = 0x26, // 0x26
    XDIC_ADDR_DUMMY_27              = 0x27, // 0x27
    XDIC_ADDR_DUMMY_28              = 0x28, // 0x28
    XDIC_ADDR_DUMMY_29              = 0x29, // 0x29
    XDIC_ADDR_DUMMY_2A              = 0x2A, // 0x2A
    XDIC_ADDR_DUMMY_2B              = 0x2B, // 0x2B
    XDIC_ADDR_DUMMY_2C              = 0x2C, // 0x2C
    XDIC_ADDR_DUMMY_2D              = 0x2D, // 0x2D
    XDIC_ADDR_DUMMY_2E              = 0x2E, // 0x2E
    XDIC_ADDR_DUMMY_2F              = 0x2F, // 0x2F
    XDIC_ADDR_DUMMY_30              = 0x30, // 0x30
    XDIC_ADDR_DUMMY_31              = 0x31, // 0x31
    XDIC_ADDR_DUMMY_32              = 0x32, // 0x32
    XDIC_ADDR_DUMMY_33              = 0x33, // 0x33
    XDIC_ADDR_DUMMY_34              = 0x34, // 0x34
    XDIC_ADDR_DUMMY_35              = 0x35, // 0x35
    XDIC_ADDR_DUMMY_36              = 0x36, // 0x36
    XDIC_ADDR_DUMMY_37              = 0x37, // 0x37
    XDIC_ADDR_DUMMY_38              = 0x38, // 0x38
    XDIC_ADDR_DUMMY_39              = 0x39, // 0x39
    XDIC_ADDR_OTP_ACCESS_1          = 0x3A, // 0x3A
    XDIC_ADDR_OTP_ACCESS_2          = 0x3B, // 0x3B
    XDIC_ADDR_OTP_WRITE             = 0x3C, // 0x3C
    XDIC_ADDR_OTP_RD_PROG           = 0x3D, // 0x3D
    XDIC_ADDR_OTP_PROTECT           = 0x3E, // 0x3E
    XDIC_ADDR_OTP_OP_MODE           = 0x3F, // 0x3F
    XDIC_ADDR_MAX                   = 0x40, // 0x40
} xdic_addr_t;

typedef union tag_XDIC_GENERAL_REG_T
{
    uint16_t ALL[XDIC_ADDR_MAX];
    struct
    {
        _xdic_reset_id_t            _r00;
        _xdic_ld_control_t          _r01;
        _xdic_svsync_num_t          _r02;
        _xdic_fpwm_divider_1_2_t    _r03;
        _xdic_fpwm_divider_2_3_t    _r04;
        _xdic_channel_enable_t      _r05;
        _xdic_fault_control_t       _r06;
        _xdic_fb_level_t            _r07;
        _xdic_short_level_t         _r08;
        _xdic_fault_status_t        _r09;
        _xdic_max_current_level_t   _r0A;
        _xdic_max_current_vref_t    _r0B;
        _xdic_max_current_vref_t    _r0C;
        _xdic_max_current_vref_t    _r0D;
        _xdic_delay_ch_1_2_t        _r0E;
        _xdic_delay_ch_3_4_t        _r0F;
        _xdic_delay_ch_5_6_t        _r10;
        _xdic_delay_ch_7_8_t        _r11;
        _xdic_delay_ch_9_10_t       _r12;
        _xdic_delay_ch_11_12_t      _r13;
        _xdic_serial_baudrate_t     _r14;
        _xdic_serial_latency_t      _r15;
        _xdic_v_mask_t              _r16;
        _xdic_sv_mask_t             _r17;
        _xdic_rstcnt_t              _r18;
        _xdic_timeout_t             _r19;
        _xdic_fll_control_1_t       _r1A;
        _xdic_fll_control_2_t       _r1B;
        _xdic_wr_protect_t          _r1C;
        _xdic_nf_control_t          _r1D;
        _xdic_dummy_t               _r1E;
        _xdic_dummy_t               _r1F;
        _xdic_chop_en_t             _r20;
        _xdic_temp_t                _r21;
        _xdic_osc_fll_manual_1_t    _r22;
        _xdic_osc_fll_manual_2_t    _r23;
        _xdic_spread_t              _r24;
        _xdic_dummy_t               _r25;
        _xdic_dummy_t               _r26;
        _xdic_dummy_t               _r27;
        _xdic_dummy_t               _r28;
        _xdic_dummy_t               _r29;
        _xdic_dummy_t               _r2A;
        _xdic_dummy_t               _r2B;
        _xdic_dummy_t               _r2C;
        _xdic_dummy_t               _r2D;
        _xdic_dummy_t               _r2E;
        _xdic_dummy_t               _r2F;
        _xdic_dummy_t               _r30;
        _xdic_dummy_t               _r31;
        _xdic_dummy_t               _r32;
        _xdic_dummy_t               _r33;
        _xdic_dummy_t               _r34;
        _xdic_dummy_t               _r35;
        _xdic_dummy_t               _r36;
        _xdic_dummy_t               _r37;
        _xdic_dummy_t               _r38;
        _xdic_dummy_t               _r39;
        _xdic_otp_access_1_t        _r3A;
        _xdic_otp_access_2_t        _r3B;
        _xdic_otp_write_t           _r3C;
        _xdic_otp_rd_prog_t         _r3D;
        _xdic_otp_protect_t         _r3E;
        _xdic_op_mode_t             _r3F;
    };
} _xdic_general_regs_t;
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
extern void XDIC_Write_General_Reg(uint8_t addr, uint16_t data);
extern uint16_t XDIC_Read_General_Reg(uint8_t addr);
extern uint16_t XDIC_Get_General_Reg(uint8_t addr);
extern void XDIC_Init(void);
/* USER CODE END */

#ifdef __cplusplus
}
#endif

#endif /* ~__XDIC_H__ */

/*** end of file ***/