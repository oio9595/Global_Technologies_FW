/** @file xd12.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XD12_H__
#define __XD12_H__

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __XD12_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif
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

#define MCLK                            (39319200.0f)
#define VSYNC                           (120.0f)

typedef enum
{
    XD12_REG_TYPE_NON_TRIM = 0,
    XD12_REG_TYPE_TRIM,
    XD12_REG_TYPE_MAX,
}XD12_REG_TYPE;

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
}_xd12_dummy_t;

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
}_xd12_reset_id_t;

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
}_xd12_ld_control_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t fpwm_div : 7;
        uint16_t          : 9;
    };
}_xd12_fpwm_divider_t;

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
}_xd12_channel_enable_t;

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
}_xd12_fault_status_t;

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
}_xd12_fault_level_t;

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
}_xd12_fault_control_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t max_curr_vref : 12;
        uint16_t               :  4;
    };
}_xd12_max_current_vref_t;

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
}_xd12_delay_ch_extend_1_t;

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
}_xd12_delay_ch_extend_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch1 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch2 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch3 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_3_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch4 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_4_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch5 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_5_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch6 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_6_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch7 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_7_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch8 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_8_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch9 : 12;
        uint16_t           :  4;
    };
}_xd12_delay_ch_9_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch10 : 12;
        uint16_t            :  4;
    };
}_xd12_delay_ch_10_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch11 : 12;
        uint16_t            :  4;
    };
}_xd12_delay_ch_11_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t delay_ch12 : 12;
        uint16_t            :  4;
    };
}_xd12_delay_ch_12_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t serial_clk_high : 6;
        uint16_t serial_clk_low  : 6;
        uint16_t                 : 4;
    };
}_xd12_serial_clock_gen_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t serial_latency : 7;
        uint16_t                : 9;
    };
}_xd12_serial_latency_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t mclk_lock_cnt : 12;
        uint16_t               :  4;
    };
}_xd12_mclk_lock_1_t;

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
}_xd12_mclk_lock_2_t;

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
}_xd12_temp_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_man : 12;
        uint16_t             :  4;
    };
}_xd12_osc_fll_manual_1_t;

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
}_xd12_osc_fll_manual_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_flt : 7;
        uint16_t             : 9;
    };
}_xd12_osc_fll_monitor_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t vref_fix : 12;
        uint16_t          :  4;
    };
}_xd12_vref_fix_t;

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
}_xd12_otp_access_1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_access_cycle : 12;
        uint16_t                     :  4;
    };
}_xd12_otp_access_2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_wsel :  4;
        uint16_t          : 12;
    };
}_xd12_otp_write_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_start :  1;
        uint16_t otp_rd_start :  1;
        uint16_t              : 14;
    };
}_xd12_otp_rd_prog_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t protect_en : 12;
        uint16_t            :  4;
    };
}_xd12_otp_protect_t;

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
}_xd12_op_mode_t;

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
}_xd12_otp_crc_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t osc :  6;
        uint16_t     : 10;
    };
}_xd12_osc_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t vref_ctl :  6;
        uint16_t          : 10;
    };
}_xd12_vref_ctl_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t ictl_l_ch_x : 7;
        uint16_t             : 9;
    };
}_xd12_ictl_l_ch_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t ictl_h_ch_x : 7;
        uint16_t             : 9;
    };
}_xd12_ictl_h_ch_t;

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
    XD12_ADDR_RESET_ID = 0x00,
    XD12_ADDR_LD_CONTROL,
    XD12_ADDR_FPWM_DIVIDER,
    XD12_ADDR_CHANNEL_ENABLE,
    /* BLANK - 0x04 */
    XD12_ADDR_FAULT_STATUS = 0x05,
    XD12_ADDR_FAULT_LEVEL,
    XD12_ADDR_FAULT_CONTROL,
    XD12_ADDR_MAX_CURR_VREF,
    XD12_ADDR_DELAY_CH_EXTEND_1,
    XD12_ADDR_DELAY_CH_EXTEND_2,
    XD12_ADDR_DELAY_CH01,
    XD12_ADDR_DELAY_CH02,
    XD12_ADDR_DELAY_CH03,
    XD12_ADDR_DELAY_CH04,
    XD12_ADDR_DELAY_CH05,
    XD12_ADDR_DELAY_CH06,
    XD12_ADDR_DELAY_CH07,
    XD12_ADDR_DELAY_CH08,
    XD12_ADDR_DELAY_CH09,
    XD12_ADDR_DELAY_CH10,
    XD12_ADDR_DELAY_CH11,
    XD12_ADDR_DELAY_CH12,
    /* BLANK - 0x17 ~ 0x24 */
    XD12_ADDR_SERIAL_CLOCK_GEN = 0x25,
    XD12_ADDR_SERIAL_LATENCY,
    XD12_ADDR_MCLK_LOCK_1,
    XD12_ADDR_MCLK_LOCK_2,
    XD12_ADDR_TEMP,
    XD12_ADDR_OSC_FLL_MANUAL_1,
    XD12_ADDR_OSC_FLL_MANUAL_2,
    XD12_ADDR_OSC_FLL_MONITOR,
    /* BLANK - 0x2D ~ 0x2E */
    XD12_ADDR_VREF_FIX = 0x2F,
    /* BLANK - 0x30 ~ 0x39 */
    XD12_ADDR_OTP_ACCESS_1 = 0x3A,
    XD12_ADDR_OTP_ACCESS_2,
    XD12_ADDR_OTP_WRITE,
    XD12_ADDR_OTP_RD_PROG,
    XD12_ADDR_OTP_PROTECT,
    XD12_ADDR_OTP_OP_MODE,
    XD12_ADDR_MAX,
}xd12_addr_t;

typedef enum
{
    XD12_ADDR_TRIM_OTP_CRC = 0x00,
    XD12_ADDR_TRIM_OSC,
    XD12_ADDR_TRIM_VREF_CTL,
    XD12_ADDR_TRIM_ICTL_L_CH_1,
    XD12_ADDR_TRIM_ICTL_L_CH_2,
    XD12_ADDR_TRIM_ICTL_L_CH_3,
    XD12_ADDR_TRIM_ICTL_L_CH_4,
    XD12_ADDR_TRIM_ICTL_L_CH_5,
    XD12_ADDR_TRIM_ICTL_L_CH_6,
    XD12_ADDR_TRIM_ICTL_L_CH_7,
    XD12_ADDR_TRIM_ICTL_L_CH_8,
    XD12_ADDR_TRIM_ICTL_L_CH_9,
    XD12_ADDR_TRIM_ICTL_L_CH_10,
    XD12_ADDR_TRIM_ICTL_L_CH_11,
    XD12_ADDR_TRIM_ICTL_L_CH_12,
    /* BLANK - 0x0F ~ 0x1A */
    XD12_ADDR_TRIM_ICTL_H_CH_1 = 0x1B,
    XD12_ADDR_TRIM_ICTL_H_CH_2,
    XD12_ADDR_TRIM_ICTL_H_CH_3,
    XD12_ADDR_TRIM_ICTL_H_CH_4,
    XD12_ADDR_TRIM_ICTL_H_CH_5,
    XD12_ADDR_TRIM_ICTL_H_CH_6,
    XD12_ADDR_TRIM_ICTL_H_CH_7,
    XD12_ADDR_TRIM_ICTL_H_CH_8,
    XD12_ADDR_TRIM_ICTL_H_CH_9,
    XD12_ADDR_TRIM_ICTL_H_CH_10,
    XD12_ADDR_TRIM_ICTL_H_CH_11,
    XD12_ADDR_TRIM_ICTL_H_CH_12,
    XD12_ADDR_TRIM_MAX,
}xd12_trim_addr_t;

typedef union _xd12_regs
{
    uint16_t ALL[XD12_ADDR_MAX];
    struct
    {
        _xd12_reset_id_t            _r00;
        _xd12_ld_control_t          _r01;
        _xd12_fpwm_divider_t        _r02;
        _xd12_channel_enable_t      _r03;
        _xd12_dummy_t               _r04;
        _xd12_fault_status_t        _r05;
        _xd12_fault_level_t         _r06;
        _xd12_fault_control_t       _r07;
        _xd12_max_current_vref_t    _r08;
        _xd12_delay_ch_extend_1_t   _r09;
        _xd12_delay_ch_extend_2_t   _r0A;
        _xd12_delay_ch_1_t          _r0B;
        _xd12_delay_ch_2_t          _r0C;
        _xd12_delay_ch_3_t          _r0D;
        _xd12_delay_ch_4_t          _r0E;
        _xd12_delay_ch_5_t          _r0F;
        _xd12_delay_ch_6_t          _r10;
        _xd12_delay_ch_7_t          _r11;
        _xd12_delay_ch_8_t          _r12;
        _xd12_delay_ch_9_t          _r13;
        _xd12_delay_ch_10_t         _r14;
        _xd12_delay_ch_11_t         _r15;
        _xd12_delay_ch_12_t         _r16;
        _xd12_dummy_t               _r17;
        _xd12_dummy_t               _r18;
        _xd12_dummy_t               _r19;
        _xd12_dummy_t               _r1A;
        _xd12_dummy_t               _r1B;
        _xd12_dummy_t               _r1C;
        _xd12_dummy_t               _r1D;
        _xd12_dummy_t               _r1E;
        _xd12_dummy_t               _r1F;
        _xd12_dummy_t               _r20;
        _xd12_dummy_t               _r21;
        _xd12_dummy_t               _r22;
        _xd12_dummy_t               _r23;
        _xd12_dummy_t               _r24;
        _xd12_serial_clock_gen_t    _r25;
        _xd12_serial_latency_t      _r26;
        _xd12_mclk_lock_1_t         _r27;
        _xd12_mclk_lock_2_t         _r28;
        _xd12_temp_t                _r29;
        _xd12_osc_fll_manual_1_t    _r2A;
        _xd12_osc_fll_manual_2_t    _r2B;
        _xd12_osc_fll_monitor_t     _r2C;
        _xd12_dummy_t               _r2D;
        _xd12_dummy_t               _r2E;
        _xd12_vref_fix_t            _r2F;
        _xd12_dummy_t               _r30;
        _xd12_dummy_t               _r31;
        _xd12_dummy_t               _r32;
        _xd12_dummy_t               _r33;
        _xd12_dummy_t               _r34;
        _xd12_dummy_t               _r35;
        _xd12_dummy_t               _r36;
        _xd12_dummy_t               _r37;
        _xd12_dummy_t               _r38;
        _xd12_dummy_t               _r39;
        _xd12_otp_access_1_t        _r3A;
        _xd12_otp_access_2_t        _r3B;
        _xd12_otp_write_t           _r3C;
        _xd12_otp_rd_prog_t         _r3D;
        _xd12_otp_protect_t         _r3E;
        _xd12_op_mode_t             _r3F;
    };
}_xd12_general_regs_t;

typedef union _xd12_trim_regs
{
    uint16_t ALL[XD12_ADDR_TRIM_MAX];
    struct
    {
        _xd12_otp_crc_t     _r00;
        _xd12_osc_t         _r01;
        _xd12_vref_ctl_t    _r02;
        _xd12_ictl_l_ch_t   _r03;
        _xd12_ictl_l_ch_t   _r04;
        _xd12_ictl_l_ch_t   _r05;
        _xd12_ictl_l_ch_t   _r06;
        _xd12_ictl_l_ch_t   _r07;
        _xd12_ictl_l_ch_t   _r08;
        _xd12_ictl_l_ch_t   _r09;
        _xd12_ictl_l_ch_t   _r0A;
        _xd12_ictl_l_ch_t   _r0B;
        _xd12_ictl_l_ch_t   _r0C;
        _xd12_ictl_l_ch_t   _r0D;
        _xd12_ictl_l_ch_t   _r0E;
        _xd12_dummy_t       _r0F;
        _xd12_dummy_t       _r10;
        _xd12_dummy_t       _r11;
        _xd12_dummy_t       _r12;
        _xd12_dummy_t       _r13;
        _xd12_dummy_t       _r14;
        _xd12_dummy_t       _r15;
        _xd12_dummy_t       _r16;
        _xd12_dummy_t       _r17;
        _xd12_dummy_t       _r18;
        _xd12_dummy_t       _r19;
        _xd12_dummy_t       _r1A;
        _xd12_ictl_h_ch_t   _r1B;
        _xd12_ictl_h_ch_t   _r1C;
        _xd12_ictl_h_ch_t   _r1D;
        _xd12_ictl_h_ch_t   _r1E;
        _xd12_ictl_h_ch_t   _r1F;
        _xd12_ictl_h_ch_t   _r20;
        _xd12_ictl_h_ch_t   _r21;
        _xd12_ictl_h_ch_t   _r22;
        _xd12_ictl_h_ch_t   _r23;
        _xd12_ictl_h_ch_t   _r24;
        _xd12_ictl_h_ch_t   _r25;
        _xd12_ictl_h_ch_t   _r26;
    };
}_xd12_mirror_regs_t;

#ifdef __cplusplus
}
#endif


extern void XD12_Write_General_Reg(uint8_t addr, uint16_t data);
extern uint16_t XD12_Read_General_Reg(uint8_t addr);
extern uint16_t XD12_Get_General_Reg(uint8_t addr);

extern void XD12_Write_Mirror_Reg(uint8_t addr, uint16_t data);
extern uint16_t XD12_Read_Mirror_Reg(uint8_t addr);
extern uint16_t XD12_Get_Mirror_Reg(uint8_t addr);

extern void XD12_Write_Mirror_Register_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode, uint16_t in_reg_val);
extern uint16_t XD12_Get_Mirror_Register_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode);
extern uint16_t XD12_Get_Mirror_Register_Limit_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode);

extern void XD12_Read_All_Registers(void);
extern void XD12_Dump_All_Registers(void);

extern void XD12_Param_Init(void);
extern void XD12_Init(void);

extern void XD12_Trim_Param_Init(void);
extern void XD12_Trim_Init(void);

extern void XD12_Set_Max_Current_Level(dev_max_curr_level_t in_dev_max_curr);
extern float XD12_Get_Max_Current_level(void);
extern void XD12_Set_Max_Curr_Vref(uint16_t in_max_curr_vref);

extern bool XD12_Is_Vsync_Mode_External(void);
extern void XD12_Update_Vsync_Frequency(float n_freq);

extern void XD12_Save_Trim_Regs(void);
extern uint64_t XD12_Compare_Trim_Regs(void);
extern void XD12_Write_Trim_Find_Regs(void);

extern void XD12_Trim_Init_VREF_CTL(void);
extern void XD12_Trim_Init_OSC(void);
extern void XD12_Trim_Init_ICTL(void);

extern void XD12_Set_OTP_Protect(bool en);
extern void XD12_Set_OTP_PG_Start(bool en);

extern void XD12_Set_FLT_Gain(uint8_t flt_gain);
extern void XD12_Set_FLT_CTL(uint8_t flt_ctl);

#endif /* ~__XD12_H__ */

/*** end of file ***/
