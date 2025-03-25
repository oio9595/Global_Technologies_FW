/** @file drv_sal_trim.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef _DRV_SAL_TRIM_H_
#define _DRV_SAL_TRIM_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "config.h"
#include "sal_struct.h"
#include "switch.h"

#define OTP_WRITE_TRUE          (1)
#define OTP_WRITE_FALSE         (0)
#define OTP_WRITE_EN            OTP_WRITE_TRUE

#define SAL_TRIM_SAVED_CNT_MAX  (10)
#define SAL_TRIM_TRIAL_CNT_MAX  (50)
#define SAL_CH_MAX              (3)

/////////////////////////////////
//       OTP REGISTERS         //
/////////////////////////////////

/* OTP_CONTROL */
typedef union
{
    uint16_t val;
    struct
    {
        uint16_t pwm_max_b_e : 1;
        uint16_t pwm_max_g_e : 1;
        uint16_t pwm_max_r_e : 1;
        uint16_t t_ana_sel   : 3;
        uint16_t ana_mod_sel : 3;
        uint16_t adc_clk_pol : 1;
        uint16_t cko_e       : 1;
        uint16_t test_e      : 1;
        uint16_t             : 4;
    };
}_sal_trim_control_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t tmux_sel   : 4;
        uint16_t otp_wsel   : 4;
        uint16_t otp_rd     : 1;
        uint16_t t_ana_e    : 1;
        uint16_t otp_pg1    : 1;
        uint16_t otp_pg2    : 1;
        uint16_t            : 4;
    };
}_sal_otp_control1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_pg_acc_cycle : 12;
        uint16_t                  :  4;
    };
}_sal_otp_control2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t protect : 12;
        uint16_t         :  4;
    };
}_sal_otp_control3_t;

/* OTP_WAFER */
typedef union
{
    uint16_t val;
    struct
    {
        uint16_t v_trim_bgr      : 6;
        uint16_t sio2_tr_tx_bias : 3;
        uint16_t sio1_tr_tx_bias : 3;
        uint16_t                 : 4;
    };
}_sal_otp1_mirror1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t i_trim_bgr  : 5;
        uint16_t vldo_ctrl   : 3;
        uint16_t sio2_tr_res : 2;
        uint16_t sio1_tr_res : 2;
        uint16_t             : 4;
    };
}_sal_otp1_mirror2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t bgr_r1_trim   : 4;
        uint16_t temp_trim_bgr : 6;
        uint16_t               : 6;
    };
}_sal_otp1_mirror3_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t bgr_r2_trim : 4;
        uint16_t osc_ctl     : 7;
        uint16_t osc_ctl_sign: 1;
        uint16_t             : 4;
    };
}_sal_otp1_mirror4_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t rsvd : 16;
    };
}_sal_otp1_mirror5_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t rsvd : 16;
    };
}_sal_otp1_mirror6_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t adc_offset : 9;
        uint16_t            : 7;
    };
}_sal_otp1_mirror7_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t adc_gain :  5;
        uint16_t          : 11;
    };
}_sal_otp1_mirror8_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t itrim_r : 8;
        uint16_t         : 8;
    };
}_sal_otp1_mirror9_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t itrim_g : 8;
        uint16_t         : 8;
    };
}_sal_otp1_mirror10_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t itrim_b : 8;
        uint16_t         : 8;
    };
}_sal_otp1_mirror11_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t tc_base : 10;
        uint16_t         :  6;
    };
}_sal_otp1_mirror12_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t tc_offset : 10;
        uint16_t           :  6;
    };
}_sal_otp1_mirror13_t;

/* OTP_PACKAGE */
typedef union
{
    uint16_t val;
    struct
    {
        uint16_t pwm_max_r : 12;
        uint16_t           :  4;
    };
}_sal_otp2_mirror14_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t pwm_max_g : 12;
        uint16_t           :  4;
    };
}_sal_otp2_mirror15_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t pwm_max_b : 12;
        uint16_t           :  4;
    };
}_sal_otp2_mirror16_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t otp_crc : 8;
        uint16_t         : 8;
    };
}_sal_otp2_mirror17_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t rsvd : 16;
    };
}_sal_otp2_mirror18_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t rsvd : 16;
    };
}_sal_otp2_mirror19_t;


typedef enum
{
    /* OTP_CONTROL */
    SAL_TRIM_ADDR_TRIM_CONTROL = 0xE0,
    SAL_TRIM_ADDR_OTP_CONTROL1 = 0xE1,
    SAL_TRIM_ADDR_OTP_CONTROL2 = 0xE2,
    SAL_TRIM_ADDR_OTP_CONTROL3 = 0xE3,

    /* OTP_WAFER */
    SAL_TRIM_ADDR_OTP1_MIRROR1 = 0xE4,
    SAL_TRIM_ADDR_OTP1_MIRROR2 = 0xE5,
    SAL_TRIM_ADDR_OTP1_MIRROR3 = 0xE6,
    SAL_TRIM_ADDR_OTP1_MIRROR4 = 0xE7,
    SAL_TRIM_ADDR_OTP1_MIRROR5 = 0xE8,
    SAL_TRIM_ADDR_OTP1_MIRROR6 = 0xE9,
    SAL_TRIM_ADDR_OTP1_MIRROR7 = 0xEA,
    SAL_TRIM_ADDR_OTP1_MIRROR8 = 0xEB,
    SAL_TRIM_ADDR_OTP1_MIRROR9 = 0xEC,
    SAL_TRIM_ADDR_OTP1_MIRROR10 = 0xED,
    SAL_TRIM_ADDR_OTP1_MIRROR11 = 0xEE,
    SAL_TRIM_ADDR_OTP1_MIRROR12 = 0xEF,
    SAL_TRIM_ADDR_OTP1_MIRROR13 = 0xF0,

    /* OTP_PACKAGE */
    SAL_TRIM_ADDR_OTP2_MIRROR14 = 0xF1,
    SAL_TRIM_ADDR_OTP2_MIRROR15 = 0xF2,
    SAL_TRIM_ADDR_OTP2_MIRROR16 = 0xF3,
    SAL_TRIM_ADDR_OTP2_MIRROR17 = 0xF4,
    SAL_TRIM_ADDR_OTP2_MIRROR18 = 0xF5,
    SAL_TRIM_ADDR_OTP2_MIRROR19 = 0xF6,
    SAL_TRIM_ADDR_MAX = 0xF7,

    SAL_TRIM_ADDR_OFFSET = SAL_TRIM_ADDR_TRIM_CONTROL,
    SAL_TRIM_ADDR_SIZE = SAL_TRIM_ADDR_MAX - SAL_TRIM_ADDR_TRIM_CONTROL,
}sal_trim_addr_t;

typedef union _sal_trim_regs
{
    uint16_t ALL[SAL_TRIM_ADDR_SIZE];
    struct
    {
        _sal_trim_control_t     _rE0;
        _sal_otp_control1_t     _rE1;
        _sal_otp_control2_t     _rE2;
        _sal_otp_control3_t     _rE3;
        _sal_otp1_mirror1_t     _rE4;
        _sal_otp1_mirror2_t     _rE5;
        _sal_otp1_mirror3_t     _rE6;
        _sal_otp1_mirror4_t     _rE7;
        _sal_otp1_mirror5_t     _rE8;
        _sal_otp1_mirror6_t     _rE9;
        _sal_otp1_mirror7_t     _rEA;
        _sal_otp1_mirror8_t     _rEB;
        _sal_otp1_mirror9_t     _rEC;
        _sal_otp1_mirror10_t    _rED;
        _sal_otp1_mirror11_t    _rEE;
        _sal_otp1_mirror12_t    _rEF;
        _sal_otp1_mirror13_t    _rF0;
        _sal_otp2_mirror14_t    _rF1;
        _sal_otp2_mirror15_t    _rF2;
        _sal_otp2_mirror16_t    _rF3;
        _sal_otp2_mirror17_t    _rF4;
        _sal_otp2_mirror18_t    _rF5;
        _sal_otp2_mirror19_t    _rF6;
    };
}_sal_trim_regs_t;

typedef struct
{
    uint8_t reg_addr;
    uint16_t data;
}_sal_i2c_info_t;

typedef enum
{
    OTP_PROTECT_ON = 0,
    OTP_PROTECT_OFF,
    OTP_PROTECT_MAX,
}otp_protect_t;

typedef enum tag_trim_step
{
    TRIM_STEP_NONE = 0,
    TRIM_STEP_PWR_ON,
    TRIM_STEP_SAL_INIT,

    TRIM_STEP_MODE_INIT,
    TRIM_STEP_MODE_SET_CONDITION,

    TRIM_STEP_ADC_SET_CH,
    TRIM_STEP_ADC_START,
    TRIM_STEP_ADC_GET_CH,

    TRIM_STEP_CHECK,
    TRIM_STEP_REG_WRITE,
    TRIM_STEP_OTP_WRITE_INIT,
    TRIM_STEP_OTP_START,
    TRIM_STEP_OTP_DONE,
    TRIM_STEP_OTP_VERIFY,

    TRIM_STEP_DONE,
    TRIM_STEP_ERROR,
    TRIM_STEP_PWR_OFF,
    TRIM_STEP_MAX,
}trim_step_t;

typedef enum tag_trim_mode
{
    TRIM_MODE_V_TRIM_BGR,
    TRIM_MODE_V_TRIM_R1_R2,
    TRIM_MODE_I_TRIM_BGR,

    TRIM_MODE_LDO_CTL,
    TRIM_MODE_OSC_CTL,

    TRIM_MODE_LVDS_TX,
    TRIM_MODE_LVDS_RX,

    TRIM_MODE_ADC_GAIN,
    TRIM_MODE_ADC_OFFSET,

    TRIM_MODE_TEMP_TRIM_BGR,

    TRIM_MODE_CURRENT_ITRIM,
    TRIM_MODE_MAX,
}trim_mode_t;

typedef enum tag_trim_param
{
    TRIM_PARAM_TARGET_MIN = 0,
    TRIM_PARAM_TARGET_MAX,
    TRIM_PARAM_TARGET,
    TRIM_PARAM_OUT_R = 0,
    TRIM_PARAM_OUT_G,
    TRIM_PARAM_OUT_B,
    TRIM_PARAM_MAX,
}trim_param_t;

typedef enum tag_trim_error
{
    TRIM_ERROR_NONE = 0,
    TRIM_ERROR_COMMUNICATION,
    TRIM_ERROR_OVERRUN,
    TRIM_ERROR_OTP_VERIFY,
    TRIM_ERROR_REG_OVER_RANGE,
    TRIM_ERROR_MAX,
}trim_error_t;

typedef enum tag_adc_ch
{
    ADC_CH_SAL_IOUT = 0,
    ADC_CH_TEST_V,
    ADC_CH_LVDS_TX,
    ADC_CH_LVDS_RX,
    ADC_CH_NONE,
    ADC_CH_MAX = 4,
}adc_ch_t;

typedef enum tag_trim_adj_flag
{
    TRIM_ADJ_FLAG_NONE = 0,
    TRIM_ADJ_FLAG_UP,
    TRIM_ADJ_FLAG_DW,
    TRIM_ADJ_FLAG_DONE,
    TRIM_ADJ_FLAG_ERR,
    TRIM_ADJ_FLAG_MAX,
}trim_adj_flag_t;

typedef struct tag_sal_trim_param
{
    uint8_t loop_cnt;
    uint8_t saved_cnt;
    uint8_t init_reg_step;
    uint16_t value_now[SAL_CH_MAX];
    uint16_t value_prev[SAL_CH_MAX];
    uint16_t value_saved[SAL_CH_MAX][SAL_TRIM_SAVED_CNT_MAX];
    uint16_t reg_now[SAL_CH_MAX];
    uint16_t reg_prev[SAL_CH_MAX];
    uint16_t reg_saved[SAL_CH_MAX][SAL_TRIM_SAVED_CNT_MAX];
    uint16_t reg_step[SAL_CH_MAX];
    trim_adj_flag_t adjust_flag[SAL_CH_MAX];
}_sal_trim_param_t;

typedef enum
{
    OTP_VERIFY_PWR_OFF = 0,
    OTP_VERIFY_PWR_ON,
    OTP_VERIFY_REG_READ,
}otp_verify_step_t;

#define INIT_STEP_V_TRIM_BGR        (1)
#define INIT_STEP_V_R1_R2           (1)
#define INIT_STEP_I_TRIM_BGR        (1)
#define INIT_STEP_TEMP_TRIM_BGR     (5)

#define INIT_STEP_LDO_CTL           (1)
#define INIT_STEP_OSC_CTL           (10)

#define INIT_STEP_LVDS_TX           (1)
#define INIT_STEP_LVDS_RX           (1)

#define INIT_STEP_ADC_GAIN          (3)
#define INIT_STEP_ADC_OFFSET        (30)

#define INIT_STEP_CURRENT_ITRIM     (5)
#define INIT_STEP_CURRENT_GAIN      (5)

#define SAL_RTN_VAL_I2C_RX_ERROR    (0xFFFF)

EXTERN void sal_write_reg_i2c(_sal_i2c_info_t *p_info);
EXTERN uint16_t sal_read_reg_i2c(_sal_i2c_info_t *p_info);

EXTERN _sal_regs_t* sal_get_regs(void);
EXTERN _sal_trim_regs_t* sal_get_trim_regs(void);

EXTERN const char* sal_get_regs_str(uint8_t reg_addr);
EXTERN const char* sal_get_trim_regs_str(uint8_t reg_addr);

EXTERN void sal_calculate_trim_range(void);
EXTERN void sal_trim_start(void);
EXTERN void sal_trimming_procedure(void);

EXTERN void tim_input_capture_start(void);
EXTERN void tim_input_capture_stop(void);
EXTERN uint32_t tim_input_capture_get_average_cnt(void);
EXTERN float convert_count_to_freq(uint32_t cnt);

EXTERN void sal_trim_t_ana_sel(trim_mode_t n_trim_mode);
EXTERN void sal_trim_test_enable(void);
EXTERN void sal_trim_otp_protection(otp_protect_t n_protect);
EXTERN void sal_trim_read_all_registers(void);
EXTERN void sal_trim_set_max_curr(void);

EXTERN void sal_adc_test_init(void);
EXTERN void sal_adc_test_read_adc(void);

EXTERN void sal_current_test_init(void);

EXTERN void sal_osc_test_init(void);
EXTERN void sal_osc_test_start(uint8_t kkk);

EXTERN void sal_otp_burn_test(void);
EXTERN void sal_mode_boot_test(uint8_t sio1_mode);
EXTERN void sal_tmux_sel_test(uint8_t tmux_sel);

EXTERN volatile bool gb_input_capture_started;

/* BEGIN - INTERFACE FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ~_DRV_SAL_TRIM_H_ */

/*** end of file ***/
