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

#define REG_LIMIT_OSC                   ( 63) /* 6-bit */
#define REG_LIMIT_VREF                  ( 63) /* 6-bit */
#define REG_LIMIT_ICTL_L                (127) /* 7-bit */
#define REG_LIMIT_ICTL_H                (127) /* 7-bit */
#define REG_LIMIT_DEV_MAX_CURR_LEVEL    (  7) /* 3-bit */
#define REG_LIMIT_SHORT_LEVEL           (  7) /* 3-bit */
#define REG_LIMIT_FB_LEVEL              (  7) /* 3-bit */

typedef enum
{
    XDIC_REG_TYPE_NON_TRIM = 0,
    XDIC_REG_TYPE_TRIM,
    XDIC_REG_TYPE_MAX,
}XDIC_REG_TYPE;

enum
{
    CH_01 = 0,
    CH_02,
    CH_03,
    CH_04,
    CH_05,
    CH_06,
    CH_07,
    CH_08,
    CH_09,
    CH_10,
    CH_11,
    CH_12,
    CH_MAX
};

typedef union
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy : 16;
    };
}_xdic_dummy_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t id  : 5;
        uint16_t     : 6;
        uint16_t rst : 1;
        uint16_t     : 4;
    };
}_xdic_reset_id_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t ld_dir     : 1;
        uint16_t pwm_res    : 1;
        uint16_t over_to_e  : 1;
        uint16_t scan_no    : 3;
        uint16_t io_mode    : 2;
        uint16_t ch_size    : 4;
        uint16_t            : 4;
    };
}_xdic_ld_control_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t fpwm_div : 7;
        uint16_t          : 9;
    };
}_xdic_fpwm_divider_t;

typedef union
{
    uint16_t val;
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
    };
}_xdic_channel_enable_t;

typedef union
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
}_xdic_fault_status_t;

typedef union
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
}_xdic_fault_level_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t o_off_e     : 1;
        uint16_t s_off_e     : 1;
        uint16_t t_off_e     : 1;
        uint16_t s_det_e     : 1;
        uint16_t o_det_e     : 1;
        uint16_t o_fb_e      : 1;
        uint16_t ms_vs_det_e : 1;
        uint16_t ms_vs_dimm  : 1;
        uint16_t ms_vs_lock  : 1;
        uint16_t             : 1;
        uint16_t ovu15_en    : 1;
        uint16_t timeout_e   : 1;
        uint16_t             : 4;
    };
}_xdic_fault_control_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t max_curr_vref : 12;
        uint16_t               :  4;
    };
}_xdic_max_current_vref_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch1 : 2;
        uint16_t delay_ch2 : 2;
        uint16_t delay_ch3 : 2;
        uint16_t delay_ch4 : 2;
        uint16_t delay_ch5 : 2;
        uint16_t delay_ch6 : 2;
        uint16_t           : 4;
    };
}_xdic_delay_ch_extend_1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch7  : 2;
        uint16_t delay_ch8  : 2;
        uint16_t delay_ch9  : 2;
        uint16_t delay_ch10 : 2;
        uint16_t delay_ch11 : 2;
        uint16_t delay_ch12 : 2;
        uint16_t            : 4;
    };
}_xdic_delay_ch_extend_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch1 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch2 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch3 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_3_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch4 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_4_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch5 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_5_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch6 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_6_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch7 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_7_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch8 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_8_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch9 : 12;
        uint16_t           :  4;
    };
}_xdic_delay_ch_9_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch10 : 12;
        uint16_t            :  4;
    };
}_xdic_delay_ch_10_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch11 : 12;
        uint16_t            :  4;
    };
}_xdic_delay_ch_11_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch12 : 12;
        uint16_t            :  4;
    };
}_xdic_delay_ch_12_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t serial_clk_high : 6;
        uint16_t serial_clk_low  : 6;
        uint16_t                 : 4;
    };
}_xdic_serial_clock_gen_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t serial_latency : 7;
        uint16_t                : 9;
    };
}_xdic_serial_latency_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t mclk_lock_cnt : 12;
        uint16_t               :  4;
    };
}_xdic_mclk_lock_1_t;

typedef union
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
}_xdic_mclk_lock_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t flt_gain : 2;
        uint16_t          : 2;
        uint16_t flt_ctl  : 2;
        uint16_t dac_rng  : 1;
        uint16_t          : 1;
        uint16_t ofs_temp : 4;
        uint16_t          : 4;
    };
}_xdic_temp_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_man : 12;
        uint16_t             :  4;
    };
}_xdic_osc_fll_manual_1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_man : 4;
        uint16_t             : 7;
        uint16_t osc_man_e   : 1;
        uint16_t             : 4;
    };
}_xdic_osc_fll_manual_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_flt : 7;
        uint16_t             : 9;
    };
}_xdic_osc_fll_monitor_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t vref_fix : 12;
        uint16_t          :  4;
    };
}_xdic_vref_fix_t;

/////////////////////////////////
//       OTP REGISTERS         //
/////////////////////////////////

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_access_cycle :  4;
        uint16_t                     : 12;
    };
}_xdic_otp_access_1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_access_cycle : 12;
        uint16_t                     :  4;
    };
}_xdic_otp_access_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_wsel :  4;
        uint16_t          : 12;
    };
}_xdic_otp_write_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_start :  1;
        uint16_t otp_rd_start :  1;
        uint16_t              : 14;
    };
}_xdic_otp_rd_prog_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t protect_en : 12;
        uint16_t            :  4;
    };
}_xdic_otp_protect_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t addr_ext    : 1;
        uint16_t             : 3;
        uint16_t vref_o      : 1;
        uint16_t mclk32_o    : 1;
        uint16_t pwm_full_o  : 1;
        uint16_t             : 1;
        uint16_t test_ana_en : 2;
        uint16_t ddio_dis    : 1;
        uint16_t test_en     : 1;
        uint16_t             : 4;
    };
}_xdic_op_mode_t;

/////////////////////////////////
//       TRIM REGISTERS        //
/////////////////////////////////
typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_crc_checksum : 8;
        uint16_t                  : 8;
    };
}_xdic_otp_crc_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t osc :  6;
        uint16_t     : 10;
    };
}_xdic_osc_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t vref_ctl :  6;
        uint16_t          : 10;
    };
}_xdic_vref_ctl_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t ictl_l_ch_x : 7;
        uint16_t             : 9;
    };
}_xdic_ictl_l_ch_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t ictl_h_ch_x : 7;
        uint16_t             : 9;
    };
}_xdic_ictl_h_ch_t;

typedef enum
{
    DEV_MAX_CURR_LEVEL_4mA = 0, // 3b'000
    DEV_MAX_CURR_LEVEL_8mA,     // 3b'001
    DEV_MAX_CURR_LEVEL_12mA,    // 3b'010
    DEV_MAX_CURR_LEVEL_16mA,    // 3b'011
    DEV_MAX_CURR_LEVEL_24mA,    // 3b'100
    DEV_MAX_CURR_LEVEL_32mA,    // 3b'101
    DEV_MAX_CURR_LEVEL_46mA,    // 3b'110
    DEV_MAX_CURR_LEVEL_64mA,    // 3b'111
    DEV_MAX_CURR_LEVEL_MAX,
}dev_max_curr_level_t;

typedef enum
{
    SHORT_LEVEL_3V = 0, // 3b'000
    SHORT_LEVEL_4V,     // 3b'001
    SHORT_LEVEL_6V,     // 3b'010
    SHORT_LEVEL_8V,     // 3b'011
    SHORT_LEVEL_12V,    // 3b'100
    SHORT_LEVEL_16V,    // 3b'101
    SHORT_LEVEL_24V,    // 3b'110
    SHORT_LEVEL_36V,    // 3b'111
    SHORT_LEVEL_MAX,
}short_level_t;

typedef enum
{
    FB_LEVEL_0V4 = 0,   // 3b'000
    FB_LEVEL_0V5,       // 3b'001
    FB_LEVEL_0V6,       // 3b'010
    FB_LEVEL_0V7,       // 3b'011
    FB_LEVEL_0V85,      // 3b'100
    FB_LEVEL_1V0,       // 3b'101
    FB_LEVEL_1V15,      // 3b'110
    FB_LEVEL_1V3,       // 3b'111
    FB_LEVEL_MAX,
}fb_level_t;

typedef enum
{
    XDIC_ADDR_RESET_ID = 0x00,
    XDIC_ADDR_LD_CONTROL,
    XDIC_ADDR_FPWM_DIVIDER,
    XDIC_ADDR_CHANNEL_ENABLE,
    /* BLANK - 0x04 */
    XDIC_ADDR_FAULT_STATUS = 0x05,
    XDIC_ADDR_FAULT_LEVEL,
    XDIC_ADDR_FAULT_CONTROL,
    XDIC_ADDR_MAX_CURR_VREF,
    XDIC_ADDR_DELAY_CH_EXTEND_1,
    XDIC_ADDR_DELAY_CH_EXTEND_2,
    XDIC_ADDR_DELAY_CH01,
    XDIC_ADDR_DELAY_CH02,
    XDIC_ADDR_DELAY_CH03,
    XDIC_ADDR_DELAY_CH04,
    XDIC_ADDR_DELAY_CH05,
    XDIC_ADDR_DELAY_CH06,
    XDIC_ADDR_DELAY_CH07,
    XDIC_ADDR_DELAY_CH08,
    XDIC_ADDR_DELAY_CH09,
    XDIC_ADDR_DELAY_CH10,
    XDIC_ADDR_DELAY_CH11,
    XDIC_ADDR_DELAY_CH12,
    /* BLANK - 0x17 ~ 0x24 */
    XDIC_ADDR_SERIAL_CLOCK_GEN = 0x25,
    XDIC_ADDR_SERIAL_LATENCY,
    XDIC_ADDR_MCLK_LOCK_1,
    XDIC_ADDR_MCLK_LOCK_2,
    XDIC_ADDR_TEMP,
    XDIC_ADDR_OSC_FLL_MANUAL_1,
    XDIC_ADDR_OSC_FLL_MANUAL_2,
    XDIC_ADDR_OSC_FLL_MONITOR,
    /* BLANK - 0x2D ~ 0x2E */
    XDIC_ADDR_VREF_FIX = 0x2F,
    /* BLANK - 0x30 ~ 0x39 */
    XDIC_ADDR_OTP_ACCESS_1 = 0x3A,
    XDIC_ADDR_OTP_ACCESS_2,
    XDIC_ADDR_OTP_WRITE,
    XDIC_ADDR_OTP_RD_PROG,
    XDIC_ADDR_OTP_PROTECT,
    XDIC_ADDR_OTP_OP_MODE,
    XDIC_ADDR_MAX,
}xdic_addr_t;

typedef enum
{
    XDIC_ADDR_TRIM_OTP_CRC = 0x00,
    XDIC_ADDR_TRIM_OSC,
    XDIC_ADDR_TRIM_VREF_CTL,
    XDIC_ADDR_TRIM_ICTL_L_CH_1,
    XDIC_ADDR_TRIM_ICTL_L_CH_2,
    XDIC_ADDR_TRIM_ICTL_L_CH_3,
    XDIC_ADDR_TRIM_ICTL_L_CH_4,
    XDIC_ADDR_TRIM_ICTL_L_CH_5,
    XDIC_ADDR_TRIM_ICTL_L_CH_6,
    XDIC_ADDR_TRIM_ICTL_L_CH_7,
    XDIC_ADDR_TRIM_ICTL_L_CH_8,
    XDIC_ADDR_TRIM_ICTL_L_CH_9,
    XDIC_ADDR_TRIM_ICTL_L_CH_10,
    XDIC_ADDR_TRIM_ICTL_L_CH_11,
    XDIC_ADDR_TRIM_ICTL_L_CH_12,
    /* BLANK - 0x0F ~ 0x1A */
    XDIC_ADDR_TRIM_ICTL_H_CH_1 = 0x1B,
    XDIC_ADDR_TRIM_ICTL_H_CH_2,
    XDIC_ADDR_TRIM_ICTL_H_CH_3,
    XDIC_ADDR_TRIM_ICTL_H_CH_4,
    XDIC_ADDR_TRIM_ICTL_H_CH_5,
    XDIC_ADDR_TRIM_ICTL_H_CH_6,
    XDIC_ADDR_TRIM_ICTL_H_CH_7,
    XDIC_ADDR_TRIM_ICTL_H_CH_8,
    XDIC_ADDR_TRIM_ICTL_H_CH_9,
    XDIC_ADDR_TRIM_ICTL_H_CH_10,
    XDIC_ADDR_TRIM_ICTL_H_CH_11,
    XDIC_ADDR_TRIM_ICTL_H_CH_12,
    XDIC_ADDR_TRIM_MAX,
}xdic_trim_addr_t;

typedef union _xdic_regs
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
        _xdic_delay_ch_extend_1_t   _r09;
        _xdic_delay_ch_extend_2_t   _r0A;
        _xdic_delay_ch_1_t          _r0B;
        _xdic_delay_ch_2_t          _r0C;
        _xdic_delay_ch_3_t          _r0D;
        _xdic_delay_ch_4_t          _r0E;
        _xdic_delay_ch_5_t          _r0F;
        _xdic_delay_ch_6_t          _r10;
        _xdic_delay_ch_7_t          _r11;
        _xdic_delay_ch_8_t          _r12;
        _xdic_delay_ch_9_t          _r13;
        _xdic_delay_ch_10_t         _r14;
        _xdic_delay_ch_11_t         _r15;
        _xdic_delay_ch_12_t         _r16;
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
        _xdic_serial_clock_gen_t    _r25;
        _xdic_serial_latency_t      _r26;
        _xdic_mclk_lock_1_t         _r27;
        _xdic_mclk_lock_2_t         _r28;
        _xdic_temp_t                _r29;
        _xdic_osc_fll_manual_1_t    _r2A;
        _xdic_osc_fll_manual_2_t    _r2B;
        _xdic_osc_fll_monitor_t     _r2C;
        _xdic_dummy_t               _r2D;
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
}_xdic_general_regs_t;

typedef union _xdic_trim_regs
{
    uint16_t ALL[XDIC_ADDR_TRIM_MAX];
    struct
    {
        _xdic_otp_crc_t     _r00;
        _xdic_osc_t         _r01;
        _xdic_vref_ctl_t    _r02;
        _xdic_ictl_l_ch_t   _r03;
        _xdic_ictl_l_ch_t   _r04;
        _xdic_ictl_l_ch_t   _r05;
        _xdic_ictl_l_ch_t   _r06;
        _xdic_ictl_l_ch_t   _r07;
        _xdic_ictl_l_ch_t   _r08;
        _xdic_ictl_l_ch_t   _r09;
        _xdic_ictl_l_ch_t   _r0A;
        _xdic_ictl_l_ch_t   _r0B;
        _xdic_ictl_l_ch_t   _r0C;
        _xdic_ictl_l_ch_t   _r0D;
        _xdic_ictl_l_ch_t   _r0E;
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
        _xdic_ictl_h_ch_t   _r1F;
        _xdic_ictl_h_ch_t   _r20;
        _xdic_ictl_h_ch_t   _r21;
        _xdic_ictl_h_ch_t   _r22;
        _xdic_ictl_h_ch_t   _r23;
        _xdic_ictl_h_ch_t   _r24;
        _xdic_ictl_h_ch_t   _r25;
        _xdic_ictl_h_ch_t   _r26;
    };
}_xdic_mirror_regs_t;

extern void XDIC_Write_General_Reg(uint8_t addr, uint16_t data);
extern uint16_t XDIC_Read_General_Reg(uint8_t addr);
extern uint16_t XDIC_Get_General_Reg(uint8_t addr);

extern void XDIC_Write_Mirror_Reg(uint8_t addr, uint16_t data);
extern uint16_t XDIC_Read_Mirror_Reg(uint8_t addr);
extern uint16_t XDIC_Get_Mirror_Reg(uint8_t addr);

extern void XDIC_Write_Mirror_Register_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode, uint16_t in_reg_val);
extern uint16_t XDIC_Get_Mirror_Register_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode);
extern uint16_t XDIC_Get_Mirror_Register_Limit_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode);

extern void XDIC_Read_All_Registers(void);
extern void XDIC_Dump_All_Registers(void);

extern void XDIC_Param_Init(void);
extern void XDIC_Init(void);

extern void XDIC_Trim_Param_Init(void);
extern void XDIC_Trim_Init(void);

extern void XDIC_Set_Max_Current_Level(dev_max_curr_level_t in_dev_max_curr);
extern float XDIC_Get_Max_Current_level(void);
extern void XDIC_Set_Max_Curr_Vref(uint16_t in_max_curr_vref);

extern bool XDIC_Is_Vsync_Mode_External(void);
extern void XDIC_Update_Vsync_Frequency(float n_freq);

extern void XDIC_Save_Trim_Regs(void);
extern uint64_t XDIC_Compare_Trim_Regs(void);

extern void XDIC_Trim_Init_VREF_CTL(void);
extern void XDIC_Trim_Init_OSC(void);
extern void XDIC_Trim_Init_ICTL(void);

extern void XDIC_Set_OTP_Protect(bool en);
extern void XDIC_Set_OTP_PG_Start(bool en);

extern void XDIC_Set_FLT_Gain(uint8_t flt_gain);
extern void XDIC_Set_FLT_CTL(uint8_t flt_ctl);

#ifdef __cplusplus
}
#endif

#endif /* ~__XDIC_H__ */

/*** end of file ***/
