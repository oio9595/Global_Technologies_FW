/** @file xd04.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XD04_H__
#define __XD04_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tag_XD04_DEV_MAX_CURRENT_LEVEL_T
{
    XD04_DEV_MAX_CURR_LEVEL_8mA = 0, // 3b'000
    XD04_DEV_MAX_CURR_LEVEL_16mA,    // 3b'001
    XD04_DEV_MAX_CURR_LEVEL_24mA,    // 3b'010
    XD04_DEV_MAX_CURR_LEVEL_32mA,    // 3b'011
    XD04_DEV_MAX_CURR_LEVEL_48mA,    // 3b'100
    XD04_DEV_MAX_CURR_LEVEL_64mA,    // 3b'101
    XD04_DEV_MAX_CURR_LEVEL_92mA,    // 3b'110
    XD04_DEV_MAX_CURR_LEVEL_128mA,   // 3b'111
    XD04_DEV_MAX_CURR_LEVEL_MAX,
} xd04_dev_max_curr_level_t;

typedef enum tag_XD04_SHORT_LEVEL_T
{
    XD04_SHORT_LEVEL_6V = 0, // 3b'000
    XD04_SHORT_LEVEL_8V,     // 3b'001
    XD04_SHORT_LEVEL_12V,    // 3b'010
    XD04_SHORT_LEVEL_16V,    // 3b'011
    XD04_SHORT_LEVEL_24V,    // 3b'100
    XD04_SHORT_LEVEL_32V,    // 3b'101
    XD04_SHORT_LEVEL_48V,    // 3b'110
    XD04_SHORT_LEVEL_70V,    // 3b'111
    XD04_SHORT_LEVEL_MAX,
} xd04_short_level_t;

typedef enum tag_XD04_FB_LEVEL_T
{
    XD04_FB_LEVEL_0V4 = 0,   // 3b'000
    XD04_FB_LEVEL_0V6,       // 3b'001
    XD04_FB_LEVEL_0V8,       // 3b'010
    XD04_FB_LEVEL_1V0,       // 3b'011
    XD04_FB_LEVEL_1V2,       // 3b'100
    XD04_FB_LEVEL_1V4,       // 3b'101
    XD04_FB_LEVEL_1V6,       // 3b'110
    XD04_FB_LEVEL_1V8,       // 3b'111
    XD04_FB_LEVEL_MAX,
} xd04_fb_level_t;

typedef union tag_XD04_DUMMY_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy : 16;
    };
} _xd04_dummy_t;

typedef union tag_XD04_GENERAL_0x00_T
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
} _xd04_reset_id_t;

typedef union tag_XD04_GENERAL_0x01_T
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
} _xd04_ld_control_t;

typedef union tag_XD04_GENERAL_0x02_T
{
    uint16_t val;
    struct
    {
        uint16_t fpwm_div   : 8;
        uint16_t            : 8;
    };
} _xd04_fpwm_divider_t;

typedef union tag_XD04_GENERAL_0x03_T
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
} _xd04_channel_enable_t;

typedef union tag_XD04_GENERAL_0x05_T
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
} _xd04_fault_status_t;

typedef union tag_XD04_GENERAL_0x06_T
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
} _xd04_fault_level_t;

typedef union tag_XD04_GENERAL_0x07_T
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
} _xd04_fault_control_t;

typedef union tag_XD04_GENERAL_0x08_T
{
    uint16_t val;
    struct
    {
        uint16_t max_curr_vref : 12;
        uint16_t               :  4;
    };
} _xd04_max_current_vref_t;

typedef union tag_XD04_GENERAL_0x09_T
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
} _xd04_delay_ch_extend_t;

typedef union tag_XD04_GENERAL_0x0B_0x0E_T
{
    uint16_t val;
    struct
    {
        uint16_t delay_chx : 12;
        uint16_t           :  4;
    };
} _xd04_delay_ch_x_t;

typedef union tag_XD04_GENERAL_0x25_T
{
    uint16_t val;
    struct
    {
        uint16_t serial_clk_high : 6;
        uint16_t serial_clk_low  : 6;
        uint16_t                 : 4;
    };
} _xd04_serial_baudrate_t;

typedef union tag_XD04_GENERAL_0x26_T
{
    uint16_t val;
    struct
    {
        uint16_t serial_latency : 8;
        uint16_t                : 8;
    };
} _xd04_serial_latency_t;

typedef union tag_XD04_GENERAL_0x27_T
{
    uint16_t val;
    struct
    {
        uint16_t mclk_lock_cnt : 12;
        uint16_t               :  4;
    };
} _xd04_mclk_lock_1_t;

typedef union tag_XD04_GENERAL_0x28_T
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
} _xd04_mclk_lock_2_t;

typedef union tag_XD04_GENERAL_0x29_T
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
} _xd04_temp_t;

typedef union tag_XD04_GENERAL_0x2A_T
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_man    : 12;
        uint16_t                :  4;
    };
} _xd04_osc_fll_manual_1_t;

typedef union tag_XD04_GENERAL_0x2B_T
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_man    : 4;
        uint16_t                : 7;
        uint16_t osc_fll_man_e  : 1;
        uint16_t                : 4;
    };
} _xd04_osc_fll_manual_2_t;

typedef union tag_XD04_GENERAL_0x2C_T
{
    uint16_t val;
    struct
    {
        uint16_t osc_fll_flt    : 7;
        uint16_t                : 9;
    };
} _xd04_osc_fll_monitor_t;

typedef union tag_XD04_GENERAL_0x2D_T
{
    uint16_t val;
    struct
    {
        uint16_t reg_wr_protect : 10;
        uint16_t pck_sync_e     :  1;
        uint16_t pck_mode       :  1;
        uint16_t                :  4;
    };
} _xd04_wr_protect_t;

typedef union tag_XD04_GENERAL_0x2F_T
{
    uint16_t val;
    struct
    {
        uint16_t vref_fix : 12;
        uint16_t          :  4;
    };
} _xd04_vref_fix_t;

typedef union tag_XD04_GENERAL_0x3A_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_access_cycle :  4;
        uint16_t                     : 12;
    };
} _xd04_otp_access_1_t;

typedef union tag_XD04_GENERAL_0x3B_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_access_cycle : 12;
        uint16_t                     :  4;
    };
} _xd04_otp_access_2_t;

typedef union tag_XD04_GENERAL_0x3C_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_wsel :  4;
        uint16_t          : 12;
    };
} _xd04_otp_write_t;

typedef union tag_XD04_GENERAL_0x3D_T
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_start :  1;
        uint16_t otp_rd_start :  1;
        uint16_t              : 14;
    };
} _xd04_otp_rd_prog_t;

typedef union tag_XD04_GENERAL_0x3E_T
{
    uint16_t val;
    struct
    {
        uint16_t protect_en : 12;
        uint16_t            :  4;
    };
} _xd04_otp_protect_t;

typedef union tag_XD04_GENERAL_0x3F_T
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
} _xd04_op_mode_t;

typedef enum tag_XD04_GENERAL_ADDR_T
{
    XD04_ADDR_RESET_ID              = 0X00, // 0x00
    XD04_ADDR_LD_CONTROL            = 0X01, // 0x01
    XD04_ADDR_FPWM_DIVIDER          = 0X02, // 0x02
    XD04_ADDR_CHANNEL_ENABLE        = 0X03, // 0x03
    XD04_ADDR_DUMMY_04              = 0X04, // 0x04
    XD04_ADDR_FAULT_STATUS          = 0X05, // 0x05
    XD04_ADDR_FAULT_LEVEL           = 0X06, // 0x06
    XD04_ADDR_FAULT_CONTROL         = 0X07, // 0x07
    XD04_ADDR_MAX_CURRENT_VREF      = 0X08, // 0x08
    XD04_ADDR_DELAY_CH_EXTEND       = 0X09, // 0x09
    XD04_ADDR_DUMMY_0A              = 0X0A, // 0x0A
    XD04_ADDR_DELAY_CH_01           = 0X0B, // 0x0B
    XD04_ADDR_DELAY_CH_02           = 0X0C, // 0x0C
    XD04_ADDR_DELAY_CH_03           = 0X0D, // 0x0D
    XD04_ADDR_DELAY_CH_04           = 0X0E, // 0x0E
    XD04_ADDR_DUMMY_0F              = 0X0F, // 0x0F
    XD04_ADDR_DUMMY_10              = 0X10, // 0x10
    XD04_ADDR_DUMMY_11              = 0X11, // 0x11
    XD04_ADDR_DUMMY_12              = 0X12, // 0x12
    XD04_ADDR_DUMMY_13              = 0X13, // 0x13
    XD04_ADDR_DUMMY_14              = 0X14, // 0x14
    XD04_ADDR_DUMMY_15              = 0X15, // 0x15
    XD04_ADDR_DUMMY_16              = 0X16, // 0x16
    XD04_ADDR_DUMMY_17              = 0X17, // 0x17
    XD04_ADDR_DUMMY_18              = 0X18, // 0x18
    XD04_ADDR_DUMMY_19              = 0X19, // 0x19
    XD04_ADDR_DUMMY_1A              = 0X1A, // 0x1A
    XD04_ADDR_DUMMY_1B              = 0X1B, // 0x1B
    XD04_ADDR_DUMMY_1C              = 0X1C, // 0x1C
    XD04_ADDR_DUMMY_1D              = 0X1D, // 0x1D
    XD04_ADDR_DUMMY_1E              = 0X1E, // 0x1E
    XD04_ADDR_DUMMY_1F              = 0X1F, // 0x1F
    XD04_ADDR_DUMMY_20              = 0X20, // 0x20
    XD04_ADDR_DUMMY_21              = 0X21, // 0x21
    XD04_ADDR_DUMMY_22              = 0X22, // 0x22
    XD04_ADDR_DUMMY_23              = 0X23, // 0x23
    XD04_ADDR_DUMMY_24              = 0X24, // 0x24
    XD04_ADDR_SERIAL_BAUDRATE       = 0X25, // 0x25
    XD04_ADDR_SERIAL_LATENCY        = 0X26, // 0x26
    XD04_ADDR_MCLK_LOCK_1           = 0X27, // 0x27
    XD04_ADDR_MCLK_LOCK_2           = 0X28, // 0x28
    XD04_ADDR_TEMP                  = 0X29, // 0x29
    XD04_ADDR_OSC_FLL_MANUAL_1      = 0X2A, // 0x2A
    XD04_ADDR_OSC_FLL_MANUAL_2      = 0X2B, // 0x2B
    XD04_ADDR_OSC_FLL_MONITOR       = 0X2C, // 0x2C
    XD04_ADDR_WR_PROTECT            = 0X2D, // 0x2D
    XD04_ADDR_DUMMY_2E              = 0X2E, // 0x2E
    XD04_ADDR_VREF_FIX              = 0X2F, // 0x2F
    XD04_ADDR_DUMMY_30              = 0X30, // 0x30
    XD04_ADDR_DUMMY_31              = 0X31, // 0x31
    XD04_ADDR_DUMMY_32              = 0X32, // 0x32
    XD04_ADDR_DUMMY_33              = 0X33, // 0x33
    XD04_ADDR_DUMMY_34              = 0X34, // 0x34
    XD04_ADDR_DUMMY_35              = 0X35, // 0x35
    XD04_ADDR_DUMMY_36              = 0X36, // 0x36
    XD04_ADDR_DUMMY_37              = 0X37, // 0x37
    XD04_ADDR_DUMMY_38              = 0X38, // 0x38
    XD04_ADDR_DUMMY_39              = 0X39, // 0x39
    XD04_ADDR_OTP_ACCESS_1          = 0X3A, // 0x3A
    XD04_ADDR_OTP_ACCESS_2          = 0X3B, // 0x3B
    XD04_ADDR_OTP_WRITE             = 0X3C, // 0x3C
    XD04_ADDR_OTP_RD_PROG           = 0X3D, // 0x3D
    XD04_ADDR_OTP_PROTECT           = 0X3E, // 0x3E
    XD04_ADDR_OTP_OP_MODE           = 0X3F, // 0x3F
    XD04_ADDR_MAX                   = 0x40, // 0x40
} xd04_addr_t;

typedef union tag_XD04_GENERAL_REG_T
{
    uint16_t ALL[XD04_ADDR_MAX];
    struct
    {
        _xd04_reset_id_t            _r00;
        _xd04_ld_control_t          _r01;
        _xd04_fpwm_divider_t        _r02;
        _xd04_channel_enable_t      _r03;
        _xd04_dummy_t               _r04;
        _xd04_fault_status_t        _r05;
        _xd04_fault_level_t         _r06;
        _xd04_fault_control_t       _r07;
        _xd04_max_current_vref_t    _r08;
        _xd04_delay_ch_extend_t     _r09;
        _xd04_dummy_t               _r0A;
        _xd04_delay_ch_x_t          _r0B;
        _xd04_delay_ch_x_t          _r0C;
        _xd04_delay_ch_x_t          _r0D;
        _xd04_delay_ch_x_t          _r0E;
        _xd04_dummy_t               _r0F;
        _xd04_dummy_t               _r10;
        _xd04_dummy_t               _r11;
        _xd04_dummy_t               _r12;
        _xd04_dummy_t               _r13;
        _xd04_dummy_t               _r14;
        _xd04_dummy_t               _r15;
        _xd04_dummy_t               _r16;
        _xd04_dummy_t               _r17;
        _xd04_dummy_t               _r18;
        _xd04_dummy_t               _r19;
        _xd04_dummy_t               _r1A;
        _xd04_dummy_t               _r1B;
        _xd04_dummy_t               _r1C;
        _xd04_dummy_t               _r1D;
        _xd04_dummy_t               _r1E;
        _xd04_dummy_t               _r1F;
        _xd04_dummy_t               _r20;
        _xd04_dummy_t               _r21;
        _xd04_dummy_t               _r22;
        _xd04_dummy_t               _r23;
        _xd04_dummy_t               _r24;
        _xd04_serial_baudrate_t     _r25;
        _xd04_serial_latency_t      _r26;
        _xd04_mclk_lock_1_t         _r27;
        _xd04_mclk_lock_2_t         _r28;
        _xd04_temp_t                _r29;
        _xd04_osc_fll_manual_1_t    _r2A;
        _xd04_osc_fll_manual_2_t    _r2B;
        _xd04_osc_fll_monitor_t     _r2C;
        _xd04_wr_protect_t          _r2D;
        _xd04_dummy_t               _r2E;
        _xd04_vref_fix_t            _r2F;
        _xd04_dummy_t               _r30;
        _xd04_dummy_t               _r31;
        _xd04_dummy_t               _r32;
        _xd04_dummy_t               _r33;
        _xd04_dummy_t               _r34;
        _xd04_dummy_t               _r35;
        _xd04_dummy_t               _r36;
        _xd04_dummy_t               _r37;
        _xd04_dummy_t               _r38;
        _xd04_dummy_t               _r39;
        _xd04_otp_access_1_t        _r3A;
        _xd04_otp_access_2_t        _r3B;
        _xd04_otp_write_t           _r3C;
        _xd04_otp_rd_prog_t         _r3D;
        _xd04_otp_protect_t         _r3E;
        _xd04_op_mode_t             _r3F;
    };
} _xd04_general_regs_t;

extern void XD04_Write_General_Reg(uint8_t addr, uint16_t data);
extern uint16_t XD04_Read_General_Reg(uint8_t addr);
extern uint16_t XD04_Get_General_Reg(uint8_t addr);

extern void XD04_Dump_All_Registers(void);
extern void XD04_Read_All_Registers(void);

extern void XD04_Param_Init(void);
extern void XD04_Init(void);

extern void XD04_Set_Max_Current_Level(xd04_dev_max_curr_level_t in_dev_max_curr);
extern void XD04_Set_Max_Curr_Vref(uint16_t in_max_curr_vref);

extern void XD04_Trim_Init_VREF_CTL(void);
extern void XD04_Trim_Init_LDO_CTL(void);
extern void XD04_Trim_Init_OSC(void);
extern void XD04_Trim_Init_ICTL_L_CH(void);
extern void XD04_Trim_Init_ICTL_H_CH(void);
extern void XD04_Trim_Init_ICC(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__XD04_H__ */

/*** end of file ***/
