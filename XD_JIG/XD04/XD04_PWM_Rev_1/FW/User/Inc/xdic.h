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

#include "xd_trim.h"

#define REG_LIMIT_OSC                   ( 63) /* 6-bit */
#define REG_LIMIT_VREF                  ( 63) /* 6-bit */
#define REG_LIMIT_ICTL_L                (127) /* 7-bit */
#define REG_LIMIT_ICTL_H                (127) /* 7-bit */

typedef enum tag_XDIC_REG_TYPE_T
{
    XDIC_REG_TYPE_NON_TRIM = 0,
    XDIC_REG_TYPE_TRIM,
    XDIC_REG_TYPE_MAX,
} XDIC_REG_TYPE_t;

typedef enum tag_XDIC_CHANNEL_T
{
    XD_CH_01 = 0,
    XD_CH_02,
    XD_CH_03,
    XD_CH_04,
    XD_CH_MAX
} XD_CH_t;

typedef enum tag_XDIC_DEV_MAX_CURRENT_LEVEL_T
{
    DEV_MAX_CURR_LEVEL_8mA = 0, // 3b'000
    DEV_MAX_CURR_LEVEL_16mA,    // 3b'001
    DEV_MAX_CURR_LEVEL_24mA,    // 3b'010
    DEV_MAX_CURR_LEVEL_32mA,    // 3b'011
    DEV_MAX_CURR_LEVEL_48mA,    // 3b'100
    DEV_MAX_CURR_LEVEL_64mA,    // 3b'101
    DEV_MAX_CURR_LEVEL_92mA,    // 3b'110
    DEV_MAX_CURR_LEVEL_128mA,   // 3b'111
    DEV_MAX_CURR_LEVEL_MAX,
} dev_max_curr_level_t;

typedef enum tag_XDIC_SHORT_LEVEL_T
{
    SHORT_LEVEL_6V = 0, // 3b'000
    SHORT_LEVEL_8V,     // 3b'001
    SHORT_LEVEL_12V,    // 3b'010
    SHORT_LEVEL_16V,    // 3b'011
    SHORT_LEVEL_24V,    // 3b'100
    SHORT_LEVEL_32V,    // 3b'101
    SHORT_LEVEL_48V,    // 3b'110
    SHORT_LEVEL_70V,    // 3b'111
    SHORT_LEVEL_MAX,
} short_level_t;

typedef enum tag_XDIC_FB_LEVEL_T
{
    FB_LEVEL_0V4 = 0,   // 3b'000
    FB_LEVEL_0V6,       // 3b'001
    FB_LEVEL_0V8,       // 3b'010
    FB_LEVEL_1V0,       // 3b'011
    FB_LEVEL_1V2,       // 3b'100
    FB_LEVEL_1V4,       // 3b'101
    FB_LEVEL_1V6,       // 3b'110
    FB_LEVEL_1V8,       // 3b'111
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
        uint16_t id     : 5;
        uint16_t        : 4;
        uint16_t e_rst  : 1;
        uint16_t vs_rst : 1;
        uint16_t rst    : 1;
        uint16_t        : 4;
    };
} _xdic_reset_id_t;

typedef union tag_XDIC_GENERAL_0x01_T
{
    uint16_t val;
    struct
    {
        uint16_t ld_dir     : 1;
        uint16_t pwm_res    : 1;
        uint16_t over_to_e  : 1;
        uint16_t scan_no    : 3;
        uint16_t io_mode    : 2;
        uint16_t ld_size    : 4;
        uint16_t            : 4;
    };
} _xdic_ld_control_t;

typedef union tag_XDIC_GENERAL_0x02_T
{
    uint16_t val;
    struct
    {
        uint16_t fpwm_div   : 8;
        uint16_t            : 8;
    };
} _xdic_fpwm_divider_t;

typedef union tag_XDIC_GENERAL_0x03_T
{
    uint16_t val;
    struct
    {
        uint16_t ch1_en :  1;
        uint16_t ch2_en :  1;
        uint16_t ch3_en :  1;
        uint16_t ch4_en :  1;
        uint16_t        : 12;
    };
} _xdic_channel_enable_t;

typedef union tag_XDIC_GENERAL_0x05_T
{
    uint16_t val;
    struct
    {
        uint16_t bit_fb      : 1;
        uint16_t bit_open    : 1;
        uint16_t bit_short   : 1;
        uint16_t bit_thermal : 1;
        uint16_t             : 4;
        uint16_t bit_uv15    : 1;
        uint16_t bit_ov15    : 1;
        uint16_t             : 1;
        uint16_t bit_miss_vs : 1;
        uint16_t             : 4;
    };
} _xdic_fault_status_t;

typedef union tag_XDIC_GENERAL_0x06_T
{
    uint16_t val;
    struct
    {
        uint16_t fb_level           : 3;
        uint16_t                    : 1;
        uint16_t short_level        : 3;
        uint16_t                    : 1;
        uint16_t dev_max_curr_level : 3;
        uint16_t                    : 5;
    };
} _xdic_fault_level_t;

typedef union tag_XDIC_GENERAL_0x07_T
{
    uint16_t val;
    struct
    {
        uint16_t o_off_e        : 1;
        uint16_t s_off_e        : 1;
        uint16_t t_off_e        : 1;
        uint16_t s_det_e        : 1;
        uint16_t o_det_e        : 1;
        uint16_t o_fb_e         : 1;
        uint16_t ms_vs_det_e    : 1;
        uint16_t ms_vs_dimm     : 1;
        uint16_t ms_vs_lock     : 1;
        uint16_t                : 1;
        uint16_t ovu15_en       : 1;
        uint16_t timeout_en     : 1;
        uint16_t                : 4;
    };
} _xdic_fault_control_t;

typedef union tag_XDIC_GENERAL_0x08_T
{
    uint16_t val;
    struct
    {
        uint16_t max_curr_vref : 12;
        uint16_t               :  4;
    };
} _xdic_max_current_vref_t;

typedef union tag_XDIC_GENERAL_0x09_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch01 : 2;
        uint16_t delay_ch02 : 2;
        uint16_t delay_ch03 : 2;
        uint16_t delay_ch04 : 2;
        uint16_t            : 8;
    };
} _xdic_delay_ch_extend_t;

typedef union tag_XDIC_GENERAL_0x0B_0x0E_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_chx : 12;
        uint16_t           :  4;
    };
} _xdic_delay_ch_x_t;

typedef union tag_XDIC_GENERAL_0x25_T
{
    uint16_t val;
    struct
    {
        uint16_t serial_clk_high : 6;
        uint16_t serial_clk_low  : 6;
        uint16_t                 : 4;
    };
} _xdic_serial_baudrate_t;

typedef union tag_XDIC_GENERAL_0x26_T
{
    uint16_t val;
    struct
    {
        uint16_t serial_latency : 8;
        uint16_t                : 8;
    };
} _xdic_serial_latency_t;

typedef union tag_XDIC_GENERAL_0x27_T
{
    uint16_t val;
    struct
    {
        uint16_t mclk_lock_cnt : 12;
        uint16_t               :  4;
    };
} _xdic_mclk_lock_1_t;

typedef union tag_XDIC_GENERAL_0x28_T
{
    uint16_t val;
    struct
    {
        uint16_t mclk_lock_cnt   : 8;
        uint16_t fll_range       : 2;
        uint16_t                 : 1;
        uint16_t mclk_lock_cnt_e : 1;
        uint16_t                 : 4;
    };
} _xdic_mclk_lock_2_t;

typedef union tag_XDIC_GENERAL_0x29_T
{
    uint16_t val;
    struct
    {
        uint16_t flt_gain   : 2;
        uint16_t            : 2;
        uint16_t flt_ctl    : 2;
        uint16_t dac_rng    : 1;
        uint16_t            : 1;
        uint16_t ofs_temp   : 4;
        uint16_t            : 4;
    };
} _xdic_temp_t;

typedef union tag_XDIC_GENERAL_0x2A_T
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_man    : 12;
        uint16_t                :  4;
    };
} _xdic_osc_fll_manual_1_t;

typedef union tag_XDIC_GENERAL_0x2B_T
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

typedef union tag_XDIC_GENERAL_0x2C_T
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_flt    : 7;
        uint16_t                : 9;
    };
} _xdic_osc_fll_monitor_t;

typedef union tag_XDIC_GENERAL_0x2D_T
{
    uint16_t val;
    struct
    {
        uint16_t reg_wr_protect : 10;
        uint16_t                :  1;
        uint16_t pck_mode       :  1;
        uint16_t                :  4;
    };
} _xdic_wr_protect_t;

typedef union tag_XDIC_GENERAL_0x2F_T
{
    uint16_t val;
    struct
    {
        uint16_t vref_fix : 12;
        uint16_t          :  4;
    };
} _xdic_vref_fix_t;

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
        uint16_t otp_wsel :  4;
        uint16_t          : 12;
    };
} _xdic_otp_write_t;

typedef union tag_XDIC_GENERAL_0x3D_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_start :  1;
        uint16_t otp_rd_start :  1;
        uint16_t              : 14;
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
        uint16_t                : 3;
        uint16_t vref_o         : 1;
        uint16_t mclk32_o       : 1;
        uint16_t pwm_full_o     : 1;
        uint16_t                : 1;
        uint16_t test_ana_en    : 2;
        uint16_t ddio_dis       : 1;
        uint16_t test_en        : 1;
        uint16_t                : 4;
    };
} _xdic_op_mode_t;

/////////////////////////////////
//       TRIM REGISTERS        //
/////////////////////////////////
typedef union tag_XDIC_MIRROR_0x00_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_crc_checksum : 8;
        uint16_t                  : 8;
    };
} _xdic_otp_crc_t;

typedef union tag_XDIC_MIRROR_0x01_T
{
    uint16_t val;
    struct
    {
        uint16_t osc :  6;
        uint16_t     : 11;
    };
} _xdic_osc_t;

typedef union tag_XDIC_MIRROR_0x02_T
{
    uint16_t val;
    struct
    {
        uint16_t vref_ctl :  6;
        uint16_t          : 10;
    };
} _xdic_vref_ctl_t;

typedef union tag_XDIC_MIRROR_0x03_0x06_T
{
    uint16_t val;
    struct
    {
        uint16_t ictl_l_chx : 7;
        uint16_t            : 9;
    };
} _xdic_ictl_l_ch_t;

typedef union tag_XDIC_MIRROR_0x1B_0x1E_T
{
    uint16_t val;
    struct
    {
        uint16_t ictl_h_chx : 7;
        uint16_t            : 9;
    };
} _xdic_ictl_h_ch_t;

typedef enum tag_XDIC_GENERAL_ADDR_T
{
    XDIC_ADDR_RESET_ID              = 0X00, // 0x00
    XDIC_ADDR_LD_CONTROL            = 0X01, // 0x01
    XDIC_ADDR_FPWM_DIVIDER          = 0X02, // 0x02
    XDIC_ADDR_CHANNEL_ENABLE        = 0X03, // 0x03
    XDIC_ADDR_DUMMY_04              = 0X04, // 0x04
    XDIC_ADDR_FAULT_STATUS          = 0X05, // 0x05
    XDIC_ADDR_FAULT_LEVEL           = 0X06, // 0x06
    XDIC_ADDR_FAULT_CONTROL         = 0X07, // 0x07
    XDIC_ADDR_MAX_CURRENT_VREF      = 0X08, // 0x08
    XDIC_ADDR_DELAY_CH_EXTEND       = 0X09, // 0x09
    XDIC_ADDR_DUMMY_0A              = 0X0A, // 0x0A
    XDIC_ADDR_DELAY_CH_01           = 0X0B, // 0x0B
    XDIC_ADDR_DELAY_CH_02           = 0X0C, // 0x0C
    XDIC_ADDR_DELAY_CH_03           = 0X0D, // 0x0D
    XDIC_ADDR_DELAY_CH_04           = 0X0E, // 0x0E
    XDIC_ADDR_DUMMY_0F              = 0X0F, // 0x0F
    XDIC_ADDR_DUMMY_10              = 0X10, // 0x10
    XDIC_ADDR_DUMMY_11              = 0X11, // 0x11
    XDIC_ADDR_DUMMY_12              = 0X12, // 0x12
    XDIC_ADDR_DUMMY_13              = 0X13, // 0x13
    XDIC_ADDR_DUMMY_14              = 0X14, // 0x14
    XDIC_ADDR_DUMMY_15              = 0X15, // 0x15
    XDIC_ADDR_DUMMY_16              = 0X16, // 0x16
    XDIC_ADDR_DUMMY_17              = 0X17, // 0x17
    XDIC_ADDR_DUMMY_18              = 0X18, // 0x18
    XDIC_ADDR_DUMMY_19              = 0X19, // 0x19
    XDIC_ADDR_DUMMY_1A              = 0X1A, // 0x1A
    XDIC_ADDR_DUMMY_1B              = 0X1B, // 0x1B
    XDIC_ADDR_DUMMY_1C              = 0X1C, // 0x1C
    XDIC_ADDR_DUMMY_1D              = 0X1D, // 0x1D
    XDIC_ADDR_DUMMY_1E              = 0X1E, // 0x1E
    XDIC_ADDR_DUMMY_1F              = 0X1F, // 0x1F
    XDIC_ADDR_DUMMY_20              = 0X20, // 0x20
    XDIC_ADDR_DUMMY_21              = 0X21, // 0x21
    XDIC_ADDR_DUMMY_22              = 0X22, // 0x22
    XDIC_ADDR_DUMMY_23              = 0X23, // 0x23
    XDIC_ADDR_DUMMY_24              = 0X24, // 0x24
    XDIC_ADDR_SERIAL_BAUDRATE       = 0X25, // 0x25
    XDIC_ADDR_SERIAL_LATENCY        = 0X26, // 0x26
    XDIC_ADDR_MCLK_LOCK_1           = 0X27, // 0x27
    XDIC_ADDR_MCLK_LOCK_2           = 0X28, // 0x28
    XDIC_ADDR_TEMP                  = 0X29, // 0x29
    XDIC_ADDR_OSC_FLL_MANUAL_1      = 0X2A, // 0x2A
    XDIC_ADDR_OSC_FLL_MANUAL_2      = 0X2B, // 0x2B
    XDIC_ADDR_OSC_FLL_MONITOR       = 0X2C, // 0x2C
    XDIC_ADDR_WR_PROTECT            = 0X2D, // 0x2D
    XDIC_ADDR_DUMMY_2E              = 0X2E, // 0x2E
    XDIC_ADDR_VREF_FIX              = 0X2F, // 0x2F
    XDIC_ADDR_DUMMY_30              = 0X30, // 0x30
    XDIC_ADDR_DUMMY_31              = 0X31, // 0x31
    XDIC_ADDR_DUMMY_32              = 0X32, // 0x32
    XDIC_ADDR_DUMMY_33              = 0X33, // 0x33
    XDIC_ADDR_DUMMY_34              = 0X34, // 0x34
    XDIC_ADDR_DUMMY_35              = 0X35, // 0x35
    XDIC_ADDR_DUMMY_36              = 0X36, // 0x36
    XDIC_ADDR_DUMMY_37              = 0X37, // 0x37
    XDIC_ADDR_DUMMY_38              = 0X38, // 0x38
    XDIC_ADDR_DUMMY_39              = 0X39, // 0x39
    XDIC_ADDR_OTP_ACCESS_1          = 0X3A, // 0x3A
    XDIC_ADDR_OTP_ACCESS_2          = 0X3B, // 0x3B
    XDIC_ADDR_OTP_WRITE             = 0X3C, // 0x3C
    XDIC_ADDR_OTP_RD_PROG           = 0X3D, // 0x3D
    XDIC_ADDR_OTP_PROTECT           = 0X3E, // 0x3E
    XDIC_ADDR_OTP_OP_MODE           = 0X3F, // 0x3F
    XDIC_ADDR_MAX                   = 0x40, // 0x40
} xdic_addr_t;

typedef enum tag_XDIC_MIRROR_ADDR_T
{
    XDIC_MIRROR_ADDR_OTP_CRC        = 0x00,  // 0x00
    XDIC_MIRROR_ADDR_OSC            = 0x01,  // 0x01
    XDIC_MIRROR_ADDR_VREF_CTL       = 0x02,  // 0x02
    XDIC_MIRROR_ADDR_ICTL_L_CH_01   = 0x03,  // 0x03
    XDIC_MIRROR_ADDR_ICTL_L_CH_02   = 0x04,  // 0x04
    XDIC_MIRROR_ADDR_ICTL_L_CH_03   = 0x05,  // 0x05
    XDIC_MIRROR_ADDR_ICTL_L_CH_04   = 0x06,  // 0x06
    XDIC_MIRROR_ADDR_DUMMY_07       = 0x07,  // 0x07
    XDIC_MIRROR_ADDR_DUMMY_08       = 0x08,  // 0x08
    XDIC_MIRROR_ADDR_DUMMY_09       = 0x09,  // 0x09
    XDIC_MIRROR_ADDR_DUMMY_0A       = 0x0A,  // 0x0A
    XDIC_MIRROR_ADDR_DUMMY_0B       = 0x0B,  // 0x0B
    XDIC_MIRROR_ADDR_DUMMY_0C       = 0x0C,  // 0x0C
    XDIC_MIRROR_ADDR_DUMMY_0D       = 0x0D,  // 0x0D
    XDIC_MIRROR_ADDR_DUMMY_0E       = 0x0E,  // 0x0E
    XDIC_MIRROR_ADDR_DUMMY_0F       = 0x0F,  // 0x0F
    XDIC_MIRROR_ADDR_DUMMY_10       = 0x10,  // 0x10
    XDIC_MIRROR_ADDR_DUMMY_11       = 0x11,  // 0x11
    XDIC_MIRROR_ADDR_DUMMY_12       = 0x12,  // 0x12
    XDIC_MIRROR_ADDR_DUMMY_13       = 0x13,  // 0x13
    XDIC_MIRROR_ADDR_DUMMY_14       = 0x14,  // 0x14
    XDIC_MIRROR_ADDR_DUMMY_15       = 0x15,  // 0x15
    XDIC_MIRROR_ADDR_DUMMY_16       = 0x16,  // 0x16
    XDIC_MIRROR_ADDR_DUMMY_17       = 0x17,  // 0x17
    XDIC_MIRROR_ADDR_DUMMY_18       = 0x18,  // 0x18
    XDIC_MIRROR_ADDR_DUMMY_19       = 0x19,  // 0x19
    XDIC_MIRROR_ADDR_DUMMY_1A       = 0x1A,  // 0x1A
    XDIC_MIRROR_ADDR_ICTL_H_CH_01   = 0x1B,  // 0x1B
    XDIC_MIRROR_ADDR_ICTL_H_CH_02   = 0x1C,  // 0x1C
    XDIC_MIRROR_ADDR_ICTL_H_CH_03   = 0x1D,  // 0x1D
    XDIC_MIRROR_ADDR_ICTL_H_CH_04   = 0x1E,  // 0x1E
    XDIC_MIRROR_ADDR_DUMMY_1F       = 0x1F,  // 0x1F
    XDIC_MIRROR_ADDR_DUMMY_20       = 0x20,  // 0x20
    XDIC_MIRROR_ADDR_DUMMY_21       = 0x21,  // 0x21
    XDIC_MIRROR_ADDR_DUMMY_22       = 0x22,  // 0x22
    XDIC_MIRROR_ADDR_DUMMY_23       = 0x23,  // 0x23
    XDIC_MIRROR_ADDR_DUMMY_24       = 0x24,  // 0x24
    XDIC_MIRROR_ADDR_DUMMY_25       = 0x25,  // 0x25
    XDIC_MIRROR_ADDR_DUMMY_26       = 0x26,  // 0x26
    XDIC_MIRROR_ADDR_MAX            = 0x27   // 0x27
} xdic_mirror_addr_t;

typedef union tag_XDIC_GENERAL_REG_T
{
    uint16_t ALL[XDIC_ADDR_MAX];
    struct
    {
        _xdic_reset_id_t            _r00;
        _xdic_ld_control_t          _r01;
        _xdic_fpwm_divider_t        _r02;
        _xdic_channel_enable_t      _r03;
        _xdic_dummy_t               _r04;
        _xdic_fault_status_t        _r05;
        _xdic_fault_level_t         _r06;
        _xdic_fault_control_t       _r07;
        _xdic_max_current_vref_t    _r08;
        _xdic_delay_ch_extend_t     _r09;
        _xdic_dummy_t               _r0A;
        _xdic_delay_ch_x_t          _r0B;
        _xdic_delay_ch_x_t          _r0C;
        _xdic_delay_ch_x_t          _r0D;
        _xdic_delay_ch_x_t          _r0E;
        _xdic_dummy_t               _r0F;
        _xdic_dummy_t               _r10;
        _xdic_dummy_t               _r11;
        _xdic_dummy_t               _r12;
        _xdic_dummy_t               _r13;
        _xdic_dummy_t               _r14;
        _xdic_dummy_t               _r15;
        _xdic_dummy_t               _r16;
        _xdic_dummy_t               _r17;
        _xdic_dummy_t               _r18;
        _xdic_dummy_t               _r19;
        _xdic_dummy_t               _r1A;
        _xdic_dummy_t               _r1B;
        _xdic_dummy_t               _r1C;
        _xdic_dummy_t               _r1D;
        _xdic_dummy_t               _r1E;
        _xdic_dummy_t               _r1F;
        _xdic_dummy_t               _r20;
        _xdic_dummy_t               _r21;
        _xdic_dummy_t               _r22;
        _xdic_dummy_t               _r23;
        _xdic_dummy_t               _r24;
        _xdic_serial_baudrate_t     _r25;
        _xdic_serial_latency_t      _r26;
        _xdic_mclk_lock_1_t         _r27;
        _xdic_mclk_lock_2_t         _r28;
        _xdic_temp_t                _r29;
        _xdic_osc_fll_manual_1_t    _r2A;
        _xdic_osc_fll_manual_2_t    _r2B;
        _xdic_osc_fll_monitor_t     _r2C;
        _xdic_wr_protect_t          _r2D;
        _xdic_dummy_t               _r2E;
        _xdic_vref_fix_t            _r2F;
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

typedef union tag_XDIC_MIRROR_REG_T
{
    uint16_t ALL[XDIC_MIRROR_ADDR_MAX];
    struct
    {
        _xdic_otp_crc_t     _r00;
        _xdic_osc_t         _r01;
        _xdic_vref_ctl_t    _r02;
        _xdic_ictl_l_ch_t   _r03;
        _xdic_ictl_l_ch_t   _r04;
        _xdic_ictl_l_ch_t   _r05;
        _xdic_ictl_l_ch_t   _r06;
        _xdic_dummy_t       _r07;
        _xdic_dummy_t       _r08;
        _xdic_dummy_t       _r09;
        _xdic_dummy_t       _r0A;
        _xdic_dummy_t       _r0B;
        _xdic_dummy_t       _r0C;
        _xdic_dummy_t       _r0D;
        _xdic_dummy_t       _r0E;
        _xdic_dummy_t       _r0F;
        _xdic_dummy_t       _r10;
        _xdic_dummy_t       _r11;
        _xdic_dummy_t       _r12;
        _xdic_dummy_t       _r13;
        _xdic_dummy_t       _r14;
        _xdic_dummy_t       _r15;
        _xdic_dummy_t       _r16;
        _xdic_dummy_t       _r17;
        _xdic_dummy_t       _r18;
        _xdic_dummy_t       _r19;
        _xdic_dummy_t       _r1A;
        _xdic_ictl_h_ch_t   _r1B;
        _xdic_ictl_h_ch_t   _r1C;
        _xdic_ictl_h_ch_t   _r1D;
        _xdic_ictl_h_ch_t   _r1E;
        _xdic_dummy_t       _r1F;
        _xdic_dummy_t       _r20;
        _xdic_dummy_t       _r21;
        _xdic_dummy_t       _r22;
        _xdic_dummy_t       _r23;
        _xdic_dummy_t       _r24;
        _xdic_dummy_t       _r25;
        _xdic_dummy_t       _r26;
    };
} _xdic_mirror_regs_t;

extern void XDIC_Write_General_Reg(uint8_t addr, uint16_t data);
extern uint16_t XDIC_Read_General_Reg(uint8_t addr);
extern uint16_t XDIC_Get_General_Reg(uint8_t addr);

extern void XDIC_Write_Mirror_Reg(uint8_t addr, uint16_t data);
extern uint16_t XDIC_Read_Mirror_Reg(uint8_t addr);
extern uint16_t XDIC_Get_Mirror_Reg(uint8_t addr);

extern void XDIC_Write_Mirror_Register_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode, uint16_t in_reg_val);
extern uint16_t XDIC_Get_Mirror_Register_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode);
extern uint16_t XDIC_Get_Mirror_Register_Limit_By_Trim_Mode(uint8_t ch_num, xd_trim_mode_t in_trim_mode);

extern void XDIC_Read_All_Registers(void);

extern void XDIC_Param_Init(void);
extern void XDIC_Init(void);

extern void XDIC_Trim_Param_Init(void);
extern void XDIC_Trim_Init(void);

extern void XDIC_Set_LD_Fix(uint16_t in_ld_fix);
extern void XDIC_Set_Max_Current_Level(dev_max_curr_level_t in_dev_max_curr);
extern float XDIC_Get_Max_Current_level(void);
extern void XDIC_Set_Max_Curr_Vref(uint16_t in_max_curr_vref);

extern bool XDIC_Is_Vsync_Mode_External(void);
extern void XDIC_Update_Vsync_Frequency(float n_freq);

extern void XDIC_Save_Trim_Regs(void);
extern uint64_t XDIC_Compare_Trim_Regs(void);

extern void XDIC_Trim_Init_VREF_CTL(void);
extern void XDIC_Trim_Init_OSC(void);
extern void XDIC_Trim_Init_ICTL_L_CH(void);
extern void XDIC_Trim_Init_ICTL_H_CH(void);

extern void XDIC_Set_OTP_Protect(bool en);
extern void XDIC_Set_OTP_PG_Start(bool en);

#ifdef __cplusplus
}
#endif

#endif /* ~__XDIC_H__ */

/*** end of file ***/
