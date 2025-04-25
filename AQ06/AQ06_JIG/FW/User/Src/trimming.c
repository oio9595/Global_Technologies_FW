/** @file trimming.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __APIC_TRIMMING_C__
#include "include.h"
#include <stdlib.h>

#define _Y__  TRUE
#define I2C_SUB_START   0xB0
#define I2C_SUB_END     0xFF

#define SCREEN_STEP 0.1

const uint8_t REGISTER_READONLY_MAP_AQ06[]={0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xd0};
const uint8_t REGISTER_USED_MAP_AQ06[]={
/*          0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F */
/*0xB0*/    _Y__, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, _Y__, _Y__, _Y__, _Y__, 0x00, 0x00,
/*0xC0*/    _Y__, _Y__, _Y__, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*0xD0*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*0xE0*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*0xF0*/    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
uint8_t *REGISTER_VALUE_MAP_AQIC[I2C_SUB_END - I2C_SUB_START +1]={0,};

static volatile uint8_t gb_GUI_ACTIVATED;
static volatile uint8_t gb_GUI_REG_APPLY_FOR_TRIM = 0;
static uint8_t gn_e2p_read[5]= {0x0,};
static uint8_t gb_gui_czmax1 = 0;
static uint8_t gb_gui_czmax2 = 1;
static uint8_t gb_gui_czn = 1;
static uint8_t gb_gui_Screenczn = 1;

double b[6] = {0.0, };

/* B1h OTP1 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t clk_sel      : 1;
        uint8_t oscen        : 1;
        uint8_t man_e2p_load : 1;
        uint8_t              : 1;
        uint8_t seq_period   : 4;
    }u;
}aqic_otp1_t;

/* B2h OTP2 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t protect_key : 8;
    }u;
}aqic_otp2_t;

/* BAh TEST1 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t sel_l_ch    : 6;
        uint8_t test_mode   : 1;
        uint8_t config_mode : 1;
    }u;
}aqic_test1_t;

/* BBh TEST2 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t cz_ch1    : 4;
        uint8_t cz_ch2    : 4;
    }u;
}aqic_test2_t;

/* BCh TEST2 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t cz_ch3    : 4;
        uint8_t cz_ch4    : 4;
    }u;
}aqic_test3_t;

/* BDh TEST2 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t cz_ch5    : 4;
        uint8_t cz_ch6    : 4;
    }u;
}aqic_test4_t;

/* C0h OTP1 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t pgm_time;
    }u;
}aqic_otpctl1_t;

/* C1h OTP2 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t pgm_wsel   : 4;
        uint8_t read_wsel  : 4;
    }u;
}aqic_otpctl2_t;

/* C2h OTP3 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t read_time   : 4;
        uint8_t rd          : 2;
        uint8_t             : 2;
    }u;
}aqic_otpctl3_t;

/* C3h OTP4 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t pgm_protect;
    }u;
}aqic_otpctl4_t;

/* C4h OTP5 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t read_start    : 1;
        uint8_t read_seq_step : 3;
        uint8_t pgm_start     : 1;
        uint8_t pgm_seq_step  : 3;
    }u;
}aqic_otpctl5_t;

/* D0h TRIM1 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t chkbit    : 2;
        uint8_t           : 6;
    }u;
}aqic_trim_d0_t;

/* D1h TRIM2 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_temp   : 4;
        uint8_t ictl_osc   : 4;
    }u;
}aqic_trim_d1_t;

/* D2h TRIM3 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch1 : 8;
    }u;
}aqic_trim_d2_t;

/* D3h TRIM4 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch2 : 8;
    }u;
}aqic_trim_d3_t;


/* D4h TRIM5 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch3 : 8;
	}u;
}aqic_trim_d4_t;

/* D5h TRIM6 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch4 : 8;
    }u;
}aqic_trim_d5_t;

/* D6h TRIM7 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch5 : 8;
    }u;
}aqic_trim_d6_t;

/* D7h TRIM8 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch6 : 8;
    }u;
}aqic_trim_d7_t;


/* D8h TRIM9 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch1 : 3;
        uint8_t hdr_mode    : 1;
        uint8_t ofs_lin_ch2 : 3;
        uint8_t             : 1;
    }u;
}aqic_trim_d8_t;

/* E0h TRIM10 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch3 : 3;
        uint8_t czmax1      : 1;
        uint8_t ofs_lin_ch4 : 3;
        uint8_t             : 1;
    }u;
}aqic_trim_e0_t;

/* E1h TRIM11 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofs_lin_ch5 : 3;
        uint8_t czmax2      : 1;
        uint8_t ofs_lin_ch6 : 3;
        uint8_t             : 1;
    }u;
}aqic_trim_e1_t;

/* E2h TRIM12 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ictl_ch1    : 7;
        uint8_t             : 1;
    }u;
}aqic_trim_e2_t;

/* E3h TRIM13 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ictl_ch2    : 7;
        uint8_t             : 1;
    }u;
}aqic_trim_e3_t;

/* E4h TRIM14 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ictl_ch3    : 7;
        uint8_t             : 1;
    }u;
}aqic_trim_e4_t;

/* E5h TRIM15 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ictl_ch4    : 7;
        uint8_t             : 1;
    }u;
}aqic_trim_e5_t;

/* E6h TRIM16 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ictl_ch5    : 7;
        uint8_t             : 1;
    }u;
}aqic_trim_e6_t;

/* E7h TRIM17 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ictl_ch6    : 7;
        uint8_t             : 1;
    }u;
}aqic_trim_e7_t;


/* E8h TRIM18 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t config_mode_rf    : 1;
        uint8_t ictl_ch1    : 1;
        uint8_t ictl_ch2    : 1;
        uint8_t ictl_ch3    : 1;
        uint8_t ictl_ch4    : 1;
        uint8_t ictl_ch5    : 1;
        uint8_t ictl_ch6    : 1;
        uint8_t             : 1;
    }u;
}aqic_trim_e8_t;

static uint8_t gn_trim_current_gain = GAIN_HIGH;

/* i2c address */
#define I2C_APIC_ID     0x90

/* i2c sub-address */
#define I2C_SUB_DEVICEID    0xB0    /* readonly reg  */

#define I2C_SUB_OTP1	0xB1
#define I2C_SUB_OTP2	0xB2

#define I2C_SUB_TEST1   0xBA
#define I2C_SUB_TEST2   0xBB
#define I2C_SUB_TEST3   0xBC
#define I2C_SUB_TEST4   0xBD

#define I2C_SUB_OTPCTL1    0xC0
#define I2C_SUB_OTPCTL2    0xC1
#define I2C_SUB_OTPCTL3    0xC2
#define I2C_SUB_OTPCTL4    0xC3
#define I2C_SUB_OTPCTL5    0xC4

#define I2C_SUB_TRIM1   0xD0
#define I2C_SUB_TRIM2   0xD1
#define I2C_SUB_TRIM3   0xD2
#define I2C_SUB_TRIM4   0xD3
#define I2C_SUB_TRIM5   0xD4
#define I2C_SUB_TRIM6   0xD5
#define I2C_SUB_TRIM7   0xD6
#define I2C_SUB_TRIM8   0xD7
#define I2C_SUB_TRIM9   0xD8

#define I2C_SUB_TRIM10  0xE0
#define I2C_SUB_TRIM11  0xE1
#define I2C_SUB_TRIM12  0xE2
#define I2C_SUB_TRIM13  0xE3
#define I2C_SUB_TRIM14  0xE4
#define I2C_SUB_TRIM15  0xE5
#define I2C_SUB_TRIM16  0xE6
#define I2C_SUB_TRIM17  0xE7
#define I2C_SUB_TRIM18  0xE8

#define PWM_OFS_RANGE 0.07
#define PWM_OFS_MIN   (1.0 - PWM_OFS_RANGE)
#define PWM_OFS_MAX   (1.0 + PWM_OFS_RANGE)

#define PWM_OFS_SECOND_PERSENT 10
#define PWM_OFS_SECOND_MIN   (100 - PWM_OFS_GT_PERSENT) / 100
#define PWM_OFS_SECOND_MAX   (100 + PWM_OFS_GT_PERSENT) / 100

// #define TRIM_ENABLE  0
static volatile uint8_t gub_TRIM_ENABLE;
static volatile uint8_t gub_DIN_MODE_ENABLE;

#define ADJ_NONE    0
#define ADJ_PLUS    1
#define ADJ_MINUS   2
#define ADJ_DEFAULT 3

typedef enum
{
    TRIM_OFS_LIN_CHS = 0,
    TRIM_ICTL_CHS,
    TRIM_SCREENING,
    TRIM_OUTPUT_TEST,
    TRIM_APIC_OUTPUT_TEST,
    TRIM_MAX
}trim_mode_t;

extern I2C_HandleTypeDef hi2c1;

static trimming_step_t gt_jig_trimming_step;

static uint8_t gt_aqic_reg_deviceid;

static aqic_otp1_t gt_aqic_reg_otp_b1;
static aqic_otp2_t gt_aqic_reg_otp_b2;

static aqic_trim_d0_t gt_aqic_reg_trim_d0;
static aqic_trim_d1_t gt_aqic_reg_trim_d1;
static aqic_trim_d2_t gt_aqic_reg_trim_d2;
static aqic_trim_d3_t gt_aqic_reg_trim_d3;
static aqic_trim_d4_t gt_aqic_reg_trim_d4;
static aqic_trim_d5_t gt_aqic_reg_trim_d5;
static aqic_trim_d6_t gt_aqic_reg_trim_d6;
static aqic_trim_d7_t gt_aqic_reg_trim_d7;
static aqic_trim_d8_t gt_aqic_reg_trim_d8;

static aqic_trim_e0_t gt_aqic_reg_trim_e0;
static aqic_trim_e1_t gt_aqic_reg_trim_e1;
static aqic_trim_e2_t gt_aqic_reg_trim_e2;
static aqic_trim_e3_t gt_aqic_reg_trim_e3;
static aqic_trim_e4_t gt_aqic_reg_trim_e4;
static aqic_trim_e5_t gt_aqic_reg_trim_e5;
static aqic_trim_e6_t gt_aqic_reg_trim_e6;
static aqic_trim_e7_t gt_aqic_reg_trim_e7;
static aqic_trim_e8_t gt_aqic_reg_trim_e8;

static aqic_test1_t gt_aqic_reg_test1;
static aqic_test2_t gt_aqic_reg_test2;
static aqic_test3_t gt_aqic_reg_test3;
static aqic_test4_t gt_aqic_reg_test4;

static uint16_t gt_aqic_reg_trim_ofs_lin_ch1_11bit;
static uint16_t gt_aqic_reg_trim_ofs_lin_ch2_11bit;
static uint16_t gt_aqic_reg_trim_ofs_lin_ch3_11bit;
static uint16_t gt_aqic_reg_trim_ofs_lin_ch4_11bit;
static uint16_t gt_aqic_reg_trim_ofs_lin_ch5_11bit;
static uint16_t gt_aqic_reg_trim_ofs_lin_ch6_11bit;

static uint16_t gt_aqic_reg_trim_ictl_ch1_8bit;
static uint16_t gt_aqic_reg_trim_ictl_ch2_8bit;
static uint16_t gt_aqic_reg_trim_ictl_ch3_8bit;
static uint16_t gt_aqic_reg_trim_ictl_ch4_8bit;
static uint16_t gt_aqic_reg_trim_ictl_ch5_8bit;
static uint16_t gt_aqic_reg_trim_ictl_ch6_8bit;

static trim_mode_t gt_trim_search_mode = TRIM_ICTL_CHS;
static uint8_t gn_trim_out_of_spec_stop_trimming;

static uint8_t gn_e2p_program[5]= { 0x20, 0x03, 0x00, 0xA5, 0x10 }; /* E2P program byte sequence for OTP write */

static uint8_t gn_read_adc_vout_channel = 5;
static uint16_t gn_trim_adc_result[TRIM_MAX][AQIC_O_MAX];
static uint16_t gn_trim_adjust_flag[TRIM_MAX][AQIC_O_MAX];

#define TRIM_REGISTER_SAVED_CNT  10
#define TRIM_OUT_RANGE_CNT  20
// #define RANGE_MARGIN_ICTL   0.05 // 5%
// #define RANGE_MARGIN_OFF   0.05 // 5%
typedef struct
{
    uint8_t u8_saved_regi;
    uint16_t u16_saved_adc;
}sTrimSaved_ictl;

typedef struct
{
    uint16_t u16_saved_regi;
    uint16_t u16_saved_adc;
} sTrimSaved_off;

static sTrimSaved_ictl gn_trim_ictl_regi_saved[TRIM_REGISTER_SAVED_CNT] = {0, };
static sTrimSaved_off gn_trim_ofs_regi_saved[TRIM_REGISTER_SAVED_CNT] = {0, };

static uint8_t gn_aqic_trim_ofs_match_cnt = 0;
static uint8_t gn_aqic_trim_ictl_match_cnt = 0;
static uint8_t gn_trim_gain_level_ch_a = 0;

static uint8_t gn_trim_ofs_step[AQIC_O_MAX];
static uint8_t gn_trim_ictl_step[AQIC_O_MAX];

static uint16_t gn_step_delay;

static uint16_t gn_apic_output_test_index;
static uint16_t gn_apic_output_test_max;
static uint16_t gn_apic_output_mode;    /* 0 : current/pwm fix, 1 : current fix/pwm*/

static uint8_t gn_aqic_slope_cnt;
static uint16_t gn_aqic_slope_adc[2][AQIC_O_MAX];
static double gn_aqic_slope[AQIC_O_MAX];
static uint16_t gn_aqic_slope_adc_temp[AQIC_O_MAX];
static uint16_t gn_aqic_slope_adc_ofs_pre[AQIC_O_MAX]={6,} ;
static uint16_t gn_aqic_slope_adc_ictl_pre[AQIC_O_MAX]={6,} ;

static uint8_t gn_aqic_trim_ofs_cnt = 0;
static uint8_t gn_aqic_trim_ictl_cnt = 0;

static uint16_t gn_aqic_screen_adc[AQIC_O_MAX];

static float gf_apic_output_test_table[] = {
0.0
};

/* Di, Dpwm voltage */
static volatile float gf_trim_voltage_table[TRIM_MAX][2] =
{   /* current, pwm */

    { 0.2727f, 0.3154f },   /*  OFS_LIN_CH */ // 100uA
    { 0.4500f, 1.2000f },   /*  ICTL_LSB   */ // 16mA @ cz_4
    { 0.2000f, 0.0000f },   /*  SCREENING  */ // 16mA @ cz_4
};

static volatile min_max_t gt_adjust_trim_range[TRIM_MAX] =
{
    { 3822, 3834, 3846}, // ofs_lin_ch @ 100uA
    { 6667, 6734, 6801}, // ictl_lsb @ 10mA slope
    // { 3309, 3376}, // ictl_lsb @ 4mA
};

static volatile float gf_din_mode_end = 0;

//static float gf_voltage_for_slope_table[3] = { 0.500, 0.200, 0.200 };    /* X2, X1, X1 */
static volatile float gf_voltage_for_slope_table[3] = { 0.250, 0.150, 0.150 };    /* X2, X1, X1 */

static double screen_avg = 0;
static double screen_dev[AQIC_O_MAX] = {0, };
static double screen_cur[AQIC_O_MAX] = {0, };

uint16_t gn_screen_cnt;

uint16_t gn_adc_compensate[3][6] =
{
    {199, 122, 187, 148, 142, 193},
    { 20,  12,  19,  15,  14,  19},
    {  2,   1,   2,   1,   1,   2},
};

static uint8_t apic_get_test_regs(void);
static void dump_apic_regs(uint8_t flag);
static void apic_set_e2p_program(void);

extern inline double convert_adc_to_voltage(uint8_t channel, uint16_t adc)
{
    return (adc * ADC_VOLT_PER_STEP); /* mV */
}

inline double convert_adc_to_current(uint8_t channel, uint16_t adc, uint8_t gain)
{
	double ret = 0;
	switch (gain)
	{
    case GAIN_HIGH :
        ret = ((adc * ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_HIGH * CURRENT_SENSE_R));	/* mA */
        break;
    case GAIN_MID :
        ret = ((adc * ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_MID * CURRENT_SENSE_R));	/* mA */
        break;
    case GAIN_LOW :
        ret = ((adc * ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_LOW * CURRENT_SENSE_R));	/* mA */
        break;
	}
	return ret; //mA
}

void apic_get_regs_for_trimming(void)
{
    if(0 == apic_get_test_regs())
    {
        dump_apic_regs(0);
    }
    else
    {
        print("======== APIC I2C READ ERROR ========\r\n");
    }
}

void apic_program_e2p(void)
{
    dump_apic_regs(1);
    apic_set_e2p_program();
}

uint8_t aqic_set_reg_otp1_clk_sel(uint8_t clk_sel)
{
    uint8_t ret = 0;

    if(clk_sel > 1)
    {
        ret = 1;    /* invalid parameter */
    }
    else
    {
        if(gt_aqic_reg_otp_b1.u.clk_sel != clk_sel)
        {
            gt_aqic_reg_otp_b1.u.clk_sel = clk_sel;
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_OTP1, 1, &(gt_aqic_reg_otp_b1.value), 1, 10);
        }
    }
    return ret;
}

uint8_t aqic_set_reg_otp1_oscen(uint8_t oscen)
{
    uint8_t ret = 0;

    if(oscen > 1)
    {
        ret = 1;    /* invalid parameter */
    }
    else
    {
        if(gt_aqic_reg_otp_b1.u.oscen != oscen)
        {
            gt_aqic_reg_otp_b1.u.oscen = oscen;
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_OTP1, 1, &(gt_aqic_reg_otp_b1.value), 1, 10);
        }
    }
    return ret;
}

uint8_t aqic_set_reg_otp2_writeen(uint16_t writeen)
{
    uint8_t ret = 0;

    if(writeen > 0xFF)
    {
        ret = 1;    /* invalid parameter */
    }
    else
    {
        if(gt_aqic_reg_otp_b2.value != writeen)
        {
            gt_aqic_reg_otp_b2.value = writeen;
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_OTP2, 1, &(gt_aqic_reg_otp_b2.value), 1, 10);
        }
    }
    return ret;
}

static void dump_apic_regs(uint8_t flag)
{
	print("======== APIC TEST REGS VALUE ========\r\n");

	if(flag == 0)
	{
		print("\tTEST1 ~ TEST4 ( BA ~ BD )\r\n");
		snprintf(msg_buffer, sizeof msg_buffer, "\t\tSEL_L_CH : %u, TEST_MODE : %u, CONFIG_MODE : %u\r\n", gt_aqic_reg_test1.u.sel_l_ch, gt_aqic_reg_test1.u.test_mode, gt_aqic_reg_test1.u.config_mode);
		print(msg_buffer);
		snprintf(msg_buffer, sizeof msg_buffer, "\t\tCZ_CH1 : %u, CZ_CH2 : %u\r\n", gt_aqic_reg_test2.u.cz_ch1, gt_aqic_reg_test2.u.cz_ch2);
		print(msg_buffer);
		snprintf(msg_buffer, sizeof msg_buffer, "\t\tCZ_CH3 : %u, CZ_CH4 : %u\r\n", gt_aqic_reg_test3.u.cz_ch3, gt_aqic_reg_test3.u.cz_ch4);
		print(msg_buffer);
		snprintf(msg_buffer, sizeof msg_buffer, "\t\tCZ_CH5 : %u, CZ_CH6 : %u\r\n", gt_aqic_reg_test4.u.cz_ch5, gt_aqic_reg_test4.u.cz_ch6);
		print(msg_buffer);
		print("\r\n");
	}

	print("\tTRIM1 ~ TRIM8 ( D0 ~ D7 )\r\n");
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tCHKBIT : %u\r\n", gt_aqic_reg_trim_d0.u.chkbit);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_TEMP : %u, ICTL_OSC : %u\r\n", gt_aqic_reg_trim_d1.u.ofs_temp, gt_aqic_reg_trim_d1.u.ictl_osc);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH1 : %u\r\n", gt_aqic_reg_trim_d2.u.ofs_lin_ch1);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH2 : %u\r\n", gt_aqic_reg_trim_d3.u.ofs_lin_ch2);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH3 : %u\r\n", gt_aqic_reg_trim_d4.u.ofs_lin_ch3);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH4 : %u\r\n", gt_aqic_reg_trim_d5.u.ofs_lin_ch4);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH5 : %u\r\n", gt_aqic_reg_trim_d6.u.ofs_lin_ch5);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH6 : %u\r\n", gt_aqic_reg_trim_d7.u.ofs_lin_ch6);
	print(msg_buffer);
	print("\r\n");

	print("\tTRIM9 ~ TRIM17 ( D8 ~ E7 )\r\n");
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH1 : %u, HDR_MODE : %u, OFS_LIN_CH2 : %u\r\n", gt_aqic_reg_trim_d8.u.ofs_lin_ch1, gt_aqic_reg_trim_d8.u.hdr_mode, gt_aqic_reg_trim_d8.u.ofs_lin_ch2);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH3 : %u, CZMAX1 : %u, OFS_LIN_CH4 : %u\r\n", gt_aqic_reg_trim_e0.u.ofs_lin_ch3, gt_aqic_reg_trim_e0.u.czmax1, gt_aqic_reg_trim_e0.u.ofs_lin_ch4);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH5 : %u, CZMAX2 : %u, OFS_LIN_CH6 : %u\r\n", gt_aqic_reg_trim_e1.u.ofs_lin_ch5, gt_aqic_reg_trim_e1.u.czmax2, gt_aqic_reg_trim_e1.u.ofs_lin_ch6);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH1 : %u, %u, %u\r\n", gt_aqic_reg_trim_e2.u.ictl_ch1, gt_aqic_reg_trim_e8.u.ictl_ch1, ((gt_aqic_reg_trim_e2.u.ictl_ch1 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch1));
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH2 : %u, %u, %u\r\n", gt_aqic_reg_trim_e3.u.ictl_ch2, gt_aqic_reg_trim_e8.u.ictl_ch2, ((gt_aqic_reg_trim_e3.u.ictl_ch2 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch2));
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH3 : %u, %u, %u\r\n", gt_aqic_reg_trim_e4.u.ictl_ch3, gt_aqic_reg_trim_e8.u.ictl_ch3, ((gt_aqic_reg_trim_e4.u.ictl_ch3 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch3));
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH4 : %u, %u, %u\r\n", gt_aqic_reg_trim_e5.u.ictl_ch4, gt_aqic_reg_trim_e8.u.ictl_ch4, ((gt_aqic_reg_trim_e5.u.ictl_ch4 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch4));
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH5 : %u, %u, %u\r\n", gt_aqic_reg_trim_e6.u.ictl_ch5, gt_aqic_reg_trim_e8.u.ictl_ch5, ((gt_aqic_reg_trim_e6.u.ictl_ch5 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch5));
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH6 : %u, %u, %u\r\n", gt_aqic_reg_trim_e7.u.ictl_ch6, gt_aqic_reg_trim_e8.u.ictl_ch6, ((gt_aqic_reg_trim_e7.u.ictl_ch6 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch6));
	print(msg_buffer);
	print("\r\n");


	print("\tOFS_LIN_CH 1 ~ 6 (11 BITS)\r\n");
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH1 : %u\r\n", (gt_aqic_reg_trim_d8.u.ofs_lin_ch1 << 8) | gt_aqic_reg_trim_d2.u.ofs_lin_ch1);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH2 : %u\r\n", (gt_aqic_reg_trim_d8.u.ofs_lin_ch2 << 8) | gt_aqic_reg_trim_d3.u.ofs_lin_ch2);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH3 : %u\r\n", (gt_aqic_reg_trim_e0.u.ofs_lin_ch3 << 8) | gt_aqic_reg_trim_d4.u.ofs_lin_ch3);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH4 : %u\r\n", (gt_aqic_reg_trim_e0.u.ofs_lin_ch4 << 8) | gt_aqic_reg_trim_d5.u.ofs_lin_ch4);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH5 : %u\r\n", (gt_aqic_reg_trim_e1.u.ofs_lin_ch5 << 8) | gt_aqic_reg_trim_d6.u.ofs_lin_ch5);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tOFS_LIN_CH6 : %u\r\n", (gt_aqic_reg_trim_e1.u.ofs_lin_ch6 << 8) | gt_aqic_reg_trim_d7.u.ofs_lin_ch6);
	print(msg_buffer);

	print("\tICTL_CH 1 ~ 6 (8 BITS)\r\n");
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH1 : %u\r\n", (gt_aqic_reg_trim_e2.u.ictl_ch1 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch1);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH2 : %u\r\n", (gt_aqic_reg_trim_e3.u.ictl_ch2 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch2);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH3 : %u\r\n", (gt_aqic_reg_trim_e4.u.ictl_ch3 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch3);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH4 : %u\r\n", (gt_aqic_reg_trim_e5.u.ictl_ch4 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch4);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH5 : %u\r\n", (gt_aqic_reg_trim_e6.u.ictl_ch5 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch5);
	print(msg_buffer);
	snprintf(msg_buffer, sizeof msg_buffer, "\t\tICTL_CH6 : %u\r\n", (gt_aqic_reg_trim_e7.u.ictl_ch6 << 1) | gt_aqic_reg_trim_e8.u.ictl_ch6);
	print(msg_buffer);
	print("======================================\r\n");
}

static uint8_t apic_get_test_regs(void)
{
    uint8_t dummy[8] = {0, };
    uint8_t sub_addr = 0;
    uint8_t ret = 0;

    sub_addr = I2C_SUB_DEVICEID;

    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_deviceid = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\t DEVICEID :  %3u\r\n", gt_aqic_reg_deviceid);
            print(msg_buffer);
        }
        else
        {
            print("=======DEVICEID READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======DEVICEID TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_OTP1;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_otp_b1.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\t OTP_B1 :  %3u\r\n", gt_aqic_reg_otp_b1.value);
            print(msg_buffer);
        }
        else
        {
            print("=======OTP1 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======OTP1 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_OTP2;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_otp_b2.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\t OTP_B2 :  %3u\r\n", gt_aqic_reg_otp_b2.value);
            print(msg_buffer);
        }
        else
        {
            print("=======OTP2 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======OTP2 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM1;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d0.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D0 :  %3u\r\n", gt_aqic_reg_trim_d0.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM1 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM1 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM2;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d1.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D1 :  %3u\r\n", gt_aqic_reg_trim_d1.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM2 READ ERROR #2======\r\n");
        }

    }
    else
    {
        ret = 1;
        print("=======TRIM2 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM3;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d2.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D2 :  %3u\r\n", gt_aqic_reg_trim_d2.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM3 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM3 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM4;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d3.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D3 :  %3u\r\n", gt_aqic_reg_trim_d3.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM4 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM4 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM5;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d4.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D4 :  %3u\r\n", gt_aqic_reg_trim_d4.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM5 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM5 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM6;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d5.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D5 :  %3u\r\n", gt_aqic_reg_trim_d5.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM6 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM6 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM7;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d6.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D6 :  %3u\r\n", gt_aqic_reg_trim_d6.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM7 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM7 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM8;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d7.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D7 :  %3u\r\n", gt_aqic_reg_trim_d7.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM8 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM8 TRANS ERROR #1======\r\n");
    }

	sub_addr = I2C_SUB_TRIM9;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_d8.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_D8 :  %3u\r\n", gt_aqic_reg_trim_d8.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM9 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM9 TRANS ERROR #1======\r\n");
    }


    sub_addr = I2C_SUB_TEST1;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_test1.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTEST_BA :  %3u\r\n", gt_aqic_reg_test1.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TEST1 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TEST1 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TEST2;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_test2.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTEST_BB :  %3u\r\n", gt_aqic_reg_test2.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TEST2 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TEST2 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TEST3;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_test3.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTEST_BC :  %3u\r\n", gt_aqic_reg_test3.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TEST3 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TEST3 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TEST4;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_test4.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTEST_BD :  %3u\r\n", gt_aqic_reg_test4.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TEST4 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TEST4 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM10;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e0.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E0 :  %3u\r\n", gt_aqic_reg_trim_e0.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM10 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM10 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM11;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e1.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E1 :  %3u\r\n", gt_aqic_reg_trim_e1.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM11 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM11 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM12;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e2.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E2 :  %3u\r\n", gt_aqic_reg_trim_e2.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM12 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM12 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM13;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e3.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E3 :  %3u\r\n", gt_aqic_reg_trim_e3.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM13 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM13 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM14;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e4.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E4 :  %3u\r\n", gt_aqic_reg_trim_e4.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM14 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM14 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM15;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e5.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E5 :  %3u\r\n", gt_aqic_reg_trim_e5.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM15 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM15 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM16;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e6.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E6 :  %3u\r\n", gt_aqic_reg_trim_e6.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM16 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM16 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM17;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e7.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E7 :  %3u\r\n", gt_aqic_reg_trim_e7.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM17 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM17 TRANS ERROR #1======\r\n");
    }

    sub_addr = I2C_SUB_TRIM18;
    if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, I2C_APIC_ID, &sub_addr, 1, 1))
    {
        if(HAL_OK == HAL_I2C_Master_Receive(&hi2c1, I2C_APIC_ID, dummy, 1, 1))
        {
            gt_aqic_reg_trim_e8.value = dummy[0];
            snprintf(msg_buffer, sizeof msg_buffer, "\tTRIM_E8 :  %3u\r\n", gt_aqic_reg_trim_e8.value);
            print(msg_buffer);
        }
        else
        {
            print("=======TRIM18 READ ERROR #2======\r\n");
        }
    }
    else
    {
        ret = 1;
        print("=======TRIM18 TRANS ERROR #1======\r\n");
    }
    return ret;
}

static void apic_set_e2p_program(void)
{
    print("======== E2P PROGRAM ========\r\n");
    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_OTPCTL1, 1, gn_e2p_program + 0, 1, 10);
    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_OTPCTL2, 1, gn_e2p_program + 1, 1, 10);
    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_OTPCTL3, 1, gn_e2p_program + 2, 1, 10);
    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_OTPCTL4, 1, gn_e2p_program + 3, 1, 10);
    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_OTPCTL5, 1, gn_e2p_program + 4, 1, 10);
}

#define USE_Z_SLOPE 0
//#define USE_Z_SLOPE 1

static volatile uint8_t gn_channel_offset_flag;

volatile float my_din = 0.0f;

void trimming_procedure_start(void)
{
    gt_jig_trimming_step = TRIMMING_STEP_INIT;
}

void trimming_procedure_run(void)
{
    if(gt_jig_trimming_step != TRIMMING_STEP_NONE)
    {
    uint8_t temp = 0;
    uint8_t channel = 0;
    uint8_t ub_adc_per_register = 0;

    uint16_t temp_adjust_trim_range_min = 0;
    uint16_t temp_adjust_trim_range_max = 0;
    uint16_t temp_adjust_trim_range_half = 0;
    uint8_t ictl_regi_value = 0;
    uint16_t ofs_regi_value = 0;

    switch(gt_jig_trimming_step)
    {
    case TRIMMING_STEP_INIT:
        print("\r\n======== TRIM START ========\r\n");

        gn_trim_out_of_spec_stop_trimming = 0;
        gn_read_adc_vout_channel = 0;

        gn_apic_output_test_index = 0;
        gn_apic_output_mode = 0;
        gn_apic_output_test_max = sizeof(gf_apic_output_test_table)/sizeof(float);

        set_ok_led(OFF);
        set_ng_led(OFF);
        current_discharge(DISCHARGE);

        for(uint8_t item = 0 ; item < TRIM_MAX ; ++item)
        {
            for(uint8_t ch = 0 ; ch < AQIC_O_MAX ; ++ch)
            {
                gn_trim_adc_result[item][ch] = 0;
            }
        }

        AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
        AQIC_Apply_Voltage(0.0f, AQIC_MODE);

        gn_trim_current_gain = GAIN_HIGH;
        change_current_gain(gn_trim_current_gain);
        AQIC_Mode_Duty(DUTY_ZERO);

        gt_jig_trimming_step = TRIMMING_STEP_APIC_PWR;
        break;
    case TRIMMING_STEP_APIC_PWR:
        AQIC_VCC_EN(PWR_ON);
        HAL_Delay(50);

        AQIC_Mode_Setting(MODE_DAC);
        HAL_Delay(10);
        AQIC_Set_Operating_Mode(AQIC_MODE_4);

        HAL_Delay(10);

        AQIC_Mode_Setting(MODE_PWM);
        AQIC_Mode_Duty(DUTY_HALF); /* pwm duty ~50%, freq 1MHz */
        change_i2c_setting(I2C_USED_I2C);  /* change gpio to i2c */

        gn_step_delay = 10;
        gt_jig_trimming_step = TRIMMING_STEP_OSCEN;
        // LSJ
        // gt_jig_trimming_step = TRIMMING_STEP_EXT_CLK_OFF;
        break;
    case TRIMMING_STEP_OSCEN:
        if (gn_step_delay)
        {
            --gn_step_delay;
        }
        else
        {
            aqic_set_reg_otp2_writeen(OTP2_WRITE_EN);
            // aqic_set_reg_otp1_oscen(1);
            gt_jig_trimming_step = TRIMMING_STEP_CLK_SEL;
        }
        break;
    case TRIMMING_STEP_CLK_SEL:
        aqic_set_reg_otp2_writeen(OTP2_WRITE_EN);
        // aqic_set_reg_otp1_clk_sel(1);
        gt_jig_trimming_step = TRIMMING_STEP_EXT_CLK_OFF;
        break;
    case TRIMMING_STEP_EXT_CLK_OFF:
        if(gb_GUI_ACTIVATED)
        {
            apic_get_test_regs();
            GUI_print_reg_all();
            gt_jig_trimming_step = TRIMMING_STEP_NONE;
            break;
        }
        else
        {
            gt_jig_trimming_step = TRIMMING_STEP_READ_REGISTER;
            break;
        }

    case TRIMMING_STEP_READ_REGISTER:
        AQIC_VLED_EN(ON);
        /* VLED_CTR_7V : ON */
        if(gn_step_delay)
        {
            --gn_step_delay;
        }
        else
        {
            if(0 == apic_get_test_regs())
            {
                dump_apic_regs(0);
                if(gb_GUI_ACTIVATED)
                {
                    gt_aqic_reg_trim_e0.u.czmax1 = gb_gui_czmax1;
                    gt_aqic_reg_trim_e1.u.czmax2 = gb_gui_czmax2;

                    gt_aqic_reg_test2.u.cz_ch1 = gb_gui_czn;
                    gt_aqic_reg_test2.u.cz_ch2 = gb_gui_czn;
                    gt_aqic_reg_test3.u.cz_ch3 = gb_gui_czn;
                    gt_aqic_reg_test3.u.cz_ch4 = gb_gui_czn;
                    gt_aqic_reg_test4.u.cz_ch5 = gb_gui_czn;
                    gt_aqic_reg_test4.u.cz_ch6 = gb_gui_czn;
                }
                else
                {
                    gt_aqic_reg_trim_e0.u.czmax1 = 0;
                    gt_aqic_reg_trim_e1.u.czmax2 = 0;

                    gt_aqic_reg_test2.u.cz_ch1 = 1;
                    gt_aqic_reg_test2.u.cz_ch2 = 1;
                    gt_aqic_reg_test3.u.cz_ch3 = 1;
                    gt_aqic_reg_test3.u.cz_ch4 = 1;
                    gt_aqic_reg_test4.u.cz_ch5 = 1;
                    gt_aqic_reg_test4.u.cz_ch6 = 1;
                }
                if(gb_GUI_REG_APPLY_FOR_TRIM)
                {
                    print("\r\nTRIM starts with Register Values changed by GUI!\r\n");
                }
                else
                {
                    // gt_aqic_reg_trim_d1.u.ofs_temp = 7;
                    // gt_aqic_reg_trim_d1.u.ofs_temp = 11; //+20 degree
                    gt_aqic_reg_trim_d1.u.ofs_temp = 13; //+30 degree

                    gt_aqic_reg_trim_d1.u.ictl_osc = 7;
                    gt_aqic_reg_trim_d8.u.hdr_mode = 0;
                    // gt_aqic_reg_trim_e0.u.czmax1 = 0;
                    // gt_aqic_reg_trim_e1.u.czmax2 = 1;
                    gt_aqic_reg_trim_e8.u.config_mode_rf = 0;
#if 1
                    gt_aqic_reg_trim_ofs_lin_ch1_11bit = 1023;
                    gt_aqic_reg_trim_ofs_lin_ch2_11bit = 1023;
                    gt_aqic_reg_trim_ofs_lin_ch3_11bit = 1023;
                    gt_aqic_reg_trim_ofs_lin_ch4_11bit = 1023;
                    gt_aqic_reg_trim_ofs_lin_ch5_11bit = 1023;
                    gt_aqic_reg_trim_ofs_lin_ch6_11bit = 1023;

                    gt_aqic_reg_trim_d2.value = (gt_aqic_reg_trim_ofs_lin_ch1_11bit & 0xFF);
                    gt_aqic_reg_trim_d8.u.ofs_lin_ch1 = ((gt_aqic_reg_trim_ofs_lin_ch1_11bit >> 8) & 0x7);
                    gt_aqic_reg_trim_d3.value = (gt_aqic_reg_trim_ofs_lin_ch2_11bit & 0xFF);
                    gt_aqic_reg_trim_d8.u.ofs_lin_ch2 = ((gt_aqic_reg_trim_ofs_lin_ch2_11bit >> 8) & 0x7);
                    gt_aqic_reg_trim_d4.value = (gt_aqic_reg_trim_ofs_lin_ch3_11bit & 0xFF);
                    gt_aqic_reg_trim_e0.u.ofs_lin_ch3 = ((gt_aqic_reg_trim_ofs_lin_ch3_11bit >> 8) & 0x7);
                    gt_aqic_reg_trim_d5.value = (gt_aqic_reg_trim_ofs_lin_ch4_11bit & 0xFF);
                    gt_aqic_reg_trim_e0.u.ofs_lin_ch4 = ((gt_aqic_reg_trim_ofs_lin_ch4_11bit >> 8) & 0x7);
                    gt_aqic_reg_trim_d6.value = (gt_aqic_reg_trim_ofs_lin_ch5_11bit & 0xFF);
                    gt_aqic_reg_trim_e1.u.ofs_lin_ch5 = ((gt_aqic_reg_trim_ofs_lin_ch5_11bit >> 8) & 0x7);
                    gt_aqic_reg_trim_d7.value = (gt_aqic_reg_trim_ofs_lin_ch6_11bit & 0xFF);
                    gt_aqic_reg_trim_e1.u.ofs_lin_ch6 = ((gt_aqic_reg_trim_ofs_lin_ch6_11bit >> 8) & 0x7);

                    gt_aqic_reg_trim_ictl_ch1_8bit = 127;
                    gt_aqic_reg_trim_ictl_ch2_8bit = 127;
                    gt_aqic_reg_trim_ictl_ch3_8bit = 127;
                    gt_aqic_reg_trim_ictl_ch4_8bit = 127;
                    gt_aqic_reg_trim_ictl_ch5_8bit = 127;
                    gt_aqic_reg_trim_ictl_ch6_8bit = 127;

                    gt_aqic_reg_trim_e2.u.ictl_ch1 = (gt_aqic_reg_trim_ictl_ch1_8bit & 0xFE) >> 1;
                    gt_aqic_reg_trim_e3.u.ictl_ch2 = (gt_aqic_reg_trim_ictl_ch2_8bit & 0xFE) >> 1;
                    gt_aqic_reg_trim_e4.u.ictl_ch3 = (gt_aqic_reg_trim_ictl_ch3_8bit & 0xFE) >> 1;
                    gt_aqic_reg_trim_e5.u.ictl_ch4 = (gt_aqic_reg_trim_ictl_ch4_8bit & 0xFE) >> 1;
                    gt_aqic_reg_trim_e6.u.ictl_ch5 = (gt_aqic_reg_trim_ictl_ch5_8bit & 0xFE) >> 1;
                    gt_aqic_reg_trim_e7.u.ictl_ch6 = (gt_aqic_reg_trim_ictl_ch6_8bit & 0xFE) >> 1;

                    gt_aqic_reg_trim_e8.u.ictl_ch1 = (gt_aqic_reg_trim_ictl_ch1_8bit & 0x01);
                    gt_aqic_reg_trim_e8.u.ictl_ch2 = (gt_aqic_reg_trim_ictl_ch2_8bit & 0x01);
                    gt_aqic_reg_trim_e8.u.ictl_ch3 = (gt_aqic_reg_trim_ictl_ch3_8bit & 0x01);
                    gt_aqic_reg_trim_e8.u.ictl_ch4 = (gt_aqic_reg_trim_ictl_ch4_8bit & 0x01);
                    gt_aqic_reg_trim_e8.u.ictl_ch5 = (gt_aqic_reg_trim_ictl_ch5_8bit & 0x01);
                    gt_aqic_reg_trim_e8.u.ictl_ch6 = (gt_aqic_reg_trim_ictl_ch6_8bit & 0x01);
#endif
                }

                // write default test2 ~ test4 reg
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST2, 1, &(gt_aqic_reg_test2.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST3, 1, &(gt_aqic_reg_test3.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST4, 1, &(gt_aqic_reg_test4.value), 1, 10);

                // write default trim1 ~ trim9 reg
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM1, 1, &(gt_aqic_reg_trim_d0.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM2, 1, &(gt_aqic_reg_trim_d1.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM3, 1, &(gt_aqic_reg_trim_d2.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM4, 1, &(gt_aqic_reg_trim_d3.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM5, 1, &(gt_aqic_reg_trim_d4.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM6, 1, &(gt_aqic_reg_trim_d5.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM7, 1, &(gt_aqic_reg_trim_d6.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM8, 1, &(gt_aqic_reg_trim_d7.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM9, 1, &(gt_aqic_reg_trim_d8.value), 1, 10);

                // write default trim10 ~ trim18 reg
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM10, 1, &(gt_aqic_reg_trim_e0.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM11, 1, &(gt_aqic_reg_trim_e1.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM12, 1, &(gt_aqic_reg_trim_e2.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM13, 1, &(gt_aqic_reg_trim_e3.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM14, 1, &(gt_aqic_reg_trim_e4.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM15, 1, &(gt_aqic_reg_trim_e5.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM16, 1, &(gt_aqic_reg_trim_e6.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM17, 1, &(gt_aqic_reg_trim_e7.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM18, 1, &(gt_aqic_reg_trim_e8.value), 1, 10);

                if(0 == apic_get_test_regs())
                {
                    dump_apic_regs(0);
                }
                gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
            }
            else
            {
                gn_trim_out_of_spec_stop_trimming = 2;  /* APIC I2C error */
                gt_jig_trimming_step = TRIMMING_STEP_STOP;
            }

            for (uint8_t ch = 0; ch < AQIC_O_MAX ; ch++)
            {
                gn_trim_adjust_flag[gt_trim_search_mode][ch] = ADJ_DEFAULT;
            }

            uint8_t gn_trim_cz = (gt_aqic_reg_trim_e1.u.czmax2 << 1 | gt_aqic_reg_trim_e0.u.czmax1);
            switch (gn_trim_cz)
            {
                case 0 :
                    snprintf(msg_buffer, sizeof msg_buffer, " CZ_MAX_0 [16mA Max]\r\n");
                    print(msg_buffer);
                    break;
                case 1 :
                    snprintf(msg_buffer, sizeof msg_buffer, " CZ_MAX_1 [24mA Max]\r\n");
                    print(msg_buffer);
                    break;
                case 2 :
                    snprintf(msg_buffer, sizeof msg_buffer, " CZ_MAX_2 [32mA Max]\r\n");
                    print(msg_buffer);
                    break;
                case 3 :
                    snprintf(msg_buffer, sizeof msg_buffer, " CZ_MAX_3 [40mA Max]\r\n");
                    print(msg_buffer);
                    break;
            }
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n\t  %s\r\n", gub_TRIM_ENABLE == 1 ? "***** TRIM ENABLE *****\r\n" : "***** TRIM DISABLE *****\r\n");
            print(msg_buffer);
            GUI_SEND_reg_all();
        }
        break;
    case TRIMMING_STEP_CHANGE_OUTPUT_INIT:
        if(gt_trim_search_mode == TRIM_SCREENING && gn_screen_cnt == 0)
        {
            if(gb_GUI_ACTIVATED)
            {
                gt_aqic_reg_test2.u.cz_ch1 = gb_gui_Screenczn;
                gt_aqic_reg_test2.u.cz_ch2 = gb_gui_Screenczn;
                gt_aqic_reg_test3.u.cz_ch3 = gb_gui_Screenczn;
                gt_aqic_reg_test3.u.cz_ch4 = gb_gui_Screenczn;
                gt_aqic_reg_test4.u.cz_ch5 = gb_gui_Screenczn;
                gt_aqic_reg_test4.u.cz_ch6 = gb_gui_Screenczn;
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST2, 1, &(gt_aqic_reg_test2.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST3, 1, &(gt_aqic_reg_test3.value), 1, 10);
                HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST4, 1, &(gt_aqic_reg_test4.value), 1, 10);
            }

            print("jig:scrn:");
            snprintf(msg_buffer, sizeof msg_buffer, "%d:%d:%d:%d:%d:%d:%d:%d\r\n",gt_aqic_reg_trim_e0.u.czmax1, gt_aqic_reg_trim_e1.u.czmax2,
                        gt_aqic_reg_test2.u.cz_ch1, gt_aqic_reg_test2.u.cz_ch2, gt_aqic_reg_test3.u.cz_ch3, gt_aqic_reg_test3.u.cz_ch4,
                        gt_aqic_reg_test4.u.cz_ch5, gt_aqic_reg_test4.u.cz_ch6);
            print(msg_buffer);

            print("jig:offv:");
            snprintf(msg_buffer, sizeof msg_buffer, "%d:%d:%d:%d:%d:%d\r\n",
                gt_aqic_reg_trim_d2.u.ofs_lin_ch1 + (gt_aqic_reg_trim_d8.u.ofs_lin_ch1 << 8),
                gt_aqic_reg_trim_d3.u.ofs_lin_ch2 + (gt_aqic_reg_trim_d8.u.ofs_lin_ch2 << 8),
                gt_aqic_reg_trim_d4.u.ofs_lin_ch3 + (gt_aqic_reg_trim_e0.u.ofs_lin_ch3 << 8),
                gt_aqic_reg_trim_d5.u.ofs_lin_ch4 + (gt_aqic_reg_trim_e0.u.ofs_lin_ch4 << 8),
                gt_aqic_reg_trim_d6.u.ofs_lin_ch5 + (gt_aqic_reg_trim_e1.u.ofs_lin_ch5 << 8),
                gt_aqic_reg_trim_d7.u.ofs_lin_ch6 + (gt_aqic_reg_trim_e1.u.ofs_lin_ch6 << 8)
            );
            print(msg_buffer);

            print("jig:ictv:");
            snprintf(msg_buffer, sizeof msg_buffer, "%d:%d:%d:%d:%d:%d\r\n",
                (gt_aqic_reg_trim_e2.u.ictl_ch1 << 1) | (gt_aqic_reg_trim_e8.u.ictl_ch1),
                (gt_aqic_reg_trim_e3.u.ictl_ch2 << 1) | (gt_aqic_reg_trim_e8.u.ictl_ch2),
                (gt_aqic_reg_trim_e4.u.ictl_ch3 << 1) | (gt_aqic_reg_trim_e8.u.ictl_ch3),
                (gt_aqic_reg_trim_e5.u.ictl_ch4 << 1) | (gt_aqic_reg_trim_e8.u.ictl_ch4),
                (gt_aqic_reg_trim_e6.u.ictl_ch5 << 1) | (gt_aqic_reg_trim_e8.u.ictl_ch5),
                (gt_aqic_reg_trim_e7.u.ictl_ch6 << 1) | (gt_aqic_reg_trim_e8.u.ictl_ch6)
            );
            print(msg_buffer);
        }

        AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
        us_delay(10);    /* TBD */
        gt_aqic_reg_test1.u.sel_l_ch = 0x3F;
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);

        us_delay(10);    /* TBD */
        gt_aqic_reg_test1.u.sel_l_ch = 0x00;
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
        us_delay(10);    /* TBD */
        AQIC_Select_Output_Ch(gn_read_adc_vout_channel);
        /*
        GAIN_LOW :  1mA
        GAIN_MID :  5mA
        GAIN_HIGH : 50mA
        */
        switch(gt_trim_search_mode)
        {
        case TRIM_OFS_LIN_CHS:
            gn_trim_current_gain = GAIN_LOW;
            change_current_gain(gn_trim_current_gain);
            current_discharge(CHARGE);
            break;
        case TRIM_ICTL_CHS:
            gn_trim_current_gain = gn_trim_gain_level_ch_a;
            change_current_gain(gn_trim_current_gain);
            current_discharge(CHARGE);
            break;
        case TRIM_SCREENING:
            gn_trim_current_gain = GAIN_HIGH;
            change_current_gain(gn_trim_current_gain);
            current_discharge(CHARGE);
            break;
        }
        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT;
        break;

    case TRIMMING_STEP_CHANGE_OUTPUT:
        switch(gt_trim_search_mode)
        {
        case TRIM_OFS_LIN_CHS:
            AQIC_Apply_Voltage(gf_trim_voltage_table[gt_trim_search_mode][gn_aqic_slope_cnt], AQIC_D_ALL);
            us_delay(10);
            gt_aqic_reg_test1.u.sel_l_ch = (0x01U << gn_read_adc_vout_channel);
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(10);
            gt_aqic_reg_test1.u.sel_l_ch = 0x00;
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(10);
            AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
            us_delay(10);
            break;
        case TRIM_ICTL_CHS:
            AQIC_Apply_Voltage(gf_trim_voltage_table[gt_trim_search_mode][gn_aqic_slope_cnt], AQIC_D_ALL);
            us_delay(10);
            gt_aqic_reg_test1.u.sel_l_ch = (0x01U << gn_read_adc_vout_channel);
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(10);
            gt_aqic_reg_test1.u.sel_l_ch = 0x00;
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(10);
            AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
            us_delay(10);
            break;
        case TRIM_OUTPUT_TEST:
            snprintf(msg_buffer, sizeof msg_buffer, "TRIM_OUTPUT_TEST [%u] : Di = %4.3f, Dpwm = %4.3f\r\n", gn_read_adc_vout_channel, gf_trim_voltage_table[gt_trim_search_mode][0], gf_trim_voltage_table[gt_trim_search_mode][1]);
            print(msg_buffer);
            AQIC_Apply_Voltage(gf_trim_voltage_table[gt_trim_search_mode][0], AQIC_D_ALL);  /* current */
            us_delay(3);    /* TBD, Time for dac voltage output to stabilize */
            gt_aqic_reg_test1.u.sel_l_ch = (0x01U << gn_read_adc_vout_channel);
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(20);    /* TBD, Time for output latch to stabilize */

            gt_aqic_reg_test1.u.sel_l_ch = 0x00;
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(20);    /* TBD, Time for output latch to stabilize */

            AQIC_Apply_Voltage(gf_trim_voltage_table[gt_trim_search_mode][1], AQIC_D_ALL);  /* pwm */
            us_delay(3);    /* TBD, Time for dac voltage output to stabilize */
            break;
        case TRIM_APIC_OUTPUT_TEST:
            //snprintf(msg_buffer, sizeof msg_buffer, "TRIM_APIC_OUTPUT_TEST [%u] : Di = %6.4f\r\n", gn_read_adc_vout_channel, gf_apic_output_test_table[gn_apic_output_test_index]);
            //print(msg_buffer);
            if(gn_apic_output_mode == 0)
            {
                AQIC_Apply_Voltage(gf_apic_output_test_table[gn_apic_output_test_index], AQIC_D_ALL);
            }
            else
            {
                AQIC_Apply_Voltage(1.000f, AQIC_D_ALL);
            }
            us_delay(3);    /* TBD, Time for dac voltage output to stabilize */
            gt_aqic_reg_test1.u.sel_l_ch = (0x01U << gn_read_adc_vout_channel);
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(20);    /* TBD, Time for output latch to stabilize */

            gt_aqic_reg_test1.u.sel_l_ch = 0x00;
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(20);    /* TBD, Time for output latch to stabilize */

            if(gn_apic_output_mode == 0)
            {
                AQIC_Apply_Voltage(5.000f, AQIC_D_ALL);
            }
            else
            {
                AQIC_Apply_Voltage(gf_apic_output_test_table[gn_apic_output_test_index], AQIC_D_ALL);
            }
            us_delay(3);    /* TBD, Time for dac voltage output to stabilize */
            break;

        case TRIM_SCREENING:
            AQIC_Select_Output_Ch(gn_read_adc_vout_channel);
            if(gub_TRIM_ENABLE == 1)
            {
                AQIC_Apply_Voltage(0.5, AQIC_D_ALL);
            }
            else
            {
                AQIC_Apply_Voltage(gf_trim_voltage_table[gt_trim_search_mode][0] + gn_screen_cnt * SCREEN_STEP, AQIC_D_ALL);
            }
            us_delay(100);    /* TBD, Time for dac voltage output to stabilize */

            gt_aqic_reg_test1.u.sel_l_ch = (0x01U << gn_read_adc_vout_channel);
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(20);    /* TBD, Time for output latch to stabilize */

            gt_aqic_reg_test1.u.sel_l_ch = 0x00;
            HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TEST1, 1, &(gt_aqic_reg_test1.value), 1, 10);
            us_delay(20);    /* TBD, Time for output latch to stabilize */

            AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
            us_delay(3);    /* TBD, Time for dac voltage output to stabilize */
            break;
        default:
            break;
        }
        gt_jig_trimming_step = TRIMMING_STEP_SET_ADC_CH;
        break;

        case TRIMMING_STEP_CHANGE_REGISTER:
            switch(gt_trim_search_mode)
            {
                case TRIM_OFS_LIN_CHS:
                    if(gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch1_11bit < 2047)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch1_11bit += gn_trim_ofs_step[0];
                            gt_aqic_reg_trim_d2.value = (gt_aqic_reg_trim_ofs_lin_ch1_11bit & 0xFF);
                            gt_aqic_reg_trim_d8.u.ofs_lin_ch1 = ((gt_aqic_reg_trim_ofs_lin_ch1_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #1 H - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch1_11bit > 0)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch1_11bit -= gn_trim_ofs_step[0];
                            gt_aqic_reg_trim_d2.value = (gt_aqic_reg_trim_ofs_lin_ch1_11bit & 0xFF);
                            gt_aqic_reg_trim_d8.u.ofs_lin_ch1 = ((gt_aqic_reg_trim_ofs_lin_ch1_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #1 L - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch2_11bit < 2047)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch2_11bit += gn_trim_ofs_step[1];
                            gt_aqic_reg_trim_d3.value = (gt_aqic_reg_trim_ofs_lin_ch2_11bit & 0xFF);
                            gt_aqic_reg_trim_d8.u.ofs_lin_ch2 = ((gt_aqic_reg_trim_ofs_lin_ch2_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #2 H - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch2_11bit > 0)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch2_11bit -= gn_trim_ofs_step[1];
                            gt_aqic_reg_trim_d3.value = (gt_aqic_reg_trim_ofs_lin_ch2_11bit & 0xFF);
                            gt_aqic_reg_trim_d8.u.ofs_lin_ch2 = ((gt_aqic_reg_trim_ofs_lin_ch2_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #2 L - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch3_11bit < 2047)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch3_11bit += gn_trim_ofs_step[2];
                            gt_aqic_reg_trim_d4.value = (gt_aqic_reg_trim_ofs_lin_ch3_11bit & 0xFF);
                            gt_aqic_reg_trim_e0.u.ofs_lin_ch3 = ((gt_aqic_reg_trim_ofs_lin_ch3_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #3 H - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch3_11bit > 0)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch3_11bit -= gn_trim_ofs_step[2];
                            gt_aqic_reg_trim_d4.value = (gt_aqic_reg_trim_ofs_lin_ch3_11bit & 0xFF);
                            gt_aqic_reg_trim_e0.u.ofs_lin_ch3 = ((gt_aqic_reg_trim_ofs_lin_ch3_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #3 L - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch4_11bit < 2047)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch4_11bit += gn_trim_ofs_step[3];
                            gt_aqic_reg_trim_d5.value = (gt_aqic_reg_trim_ofs_lin_ch4_11bit & 0xFF);
                            gt_aqic_reg_trim_e0.u.ofs_lin_ch4 = ((gt_aqic_reg_trim_ofs_lin_ch4_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #4 H - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch4_11bit > 0)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch4_11bit -= gn_trim_ofs_step[3];
                            gt_aqic_reg_trim_d5.value = (gt_aqic_reg_trim_ofs_lin_ch4_11bit & 0xFF);
                            gt_aqic_reg_trim_e0.u.ofs_lin_ch4 = ((gt_aqic_reg_trim_ofs_lin_ch4_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #4 L - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch5_11bit < 2047)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch5_11bit += gn_trim_ofs_step[4];
                            gt_aqic_reg_trim_d6.value = (gt_aqic_reg_trim_ofs_lin_ch5_11bit & 0xFF);
                            gt_aqic_reg_trim_e1.u.ofs_lin_ch5 = ((gt_aqic_reg_trim_ofs_lin_ch5_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #5 H - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch5_11bit > 0)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch5_11bit -= gn_trim_ofs_step[4];
                            gt_aqic_reg_trim_d6.value = (gt_aqic_reg_trim_ofs_lin_ch5_11bit & 0xFF);
                            gt_aqic_reg_trim_e1.u.ofs_lin_ch5 = ((gt_aqic_reg_trim_ofs_lin_ch5_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #5 L - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch6_11bit < 2047)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch6_11bit += gn_trim_ofs_step[5];
                            gt_aqic_reg_trim_d7.value = (gt_aqic_reg_trim_ofs_lin_ch6_11bit & 0xFF);
                            gt_aqic_reg_trim_e1.u.ofs_lin_ch6 = ((gt_aqic_reg_trim_ofs_lin_ch6_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #6 H - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ofs_lin_ch6_11bit > 0)
                        {
                            gt_aqic_reg_trim_ofs_lin_ch6_11bit -= gn_trim_ofs_step[5];
                            gt_aqic_reg_trim_d7.value = (gt_aqic_reg_trim_ofs_lin_ch6_11bit & 0xFF);
                            gt_aqic_reg_trim_e1.u.ofs_lin_ch6 = ((gt_aqic_reg_trim_ofs_lin_ch6_11bit >> 8) & 0x07);
                        }
                        else
                        {
                            print("======== LIN_CHS #6 L - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    if(gn_trim_out_of_spec_stop_trimming == 0)
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                    }
                    else
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_STOP;
                    }
                    // print("\t  REG WRITE \r\n\r\n");

                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM3, 1, &(gt_aqic_reg_trim_d2.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM4, 1, &(gt_aqic_reg_trim_d3.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM5, 1, &(gt_aqic_reg_trim_d4.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM6, 1, &(gt_aqic_reg_trim_d5.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM7, 1, &(gt_aqic_reg_trim_d6.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM8, 1, &(gt_aqic_reg_trim_d7.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM9, 1, &(gt_aqic_reg_trim_d8.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM10, 1, &(gt_aqic_reg_trim_e0.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM11, 1, &(gt_aqic_reg_trim_e1.value), 1, 10);
                    // apic_get_test_regs();
                    break;

                case TRIM_ICTL_CHS:
                    gn_trim_out_of_spec_stop_trimming = 0;
                    if(gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch1_8bit < 255)
                        {
                            gt_aqic_reg_trim_ictl_ch1_8bit += gn_trim_ictl_step[0];
                            gt_aqic_reg_trim_e2.u.ictl_ch1 = (gt_aqic_reg_trim_ictl_ch1_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch1 = gt_aqic_reg_trim_ictl_ch1_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB1 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch1_8bit > 0)
                        {
                            gt_aqic_reg_trim_ictl_ch1_8bit -= gn_trim_ictl_step[0];
                            gt_aqic_reg_trim_e2.u.ictl_ch1 = (gt_aqic_reg_trim_ictl_ch1_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch1 = gt_aqic_reg_trim_ictl_ch1_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB1 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }

                    else if (gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_NONE)
                    {

                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch2_8bit < 255)
                        {
                            gt_aqic_reg_trim_ictl_ch2_8bit += gn_trim_ictl_step[1];
                            gt_aqic_reg_trim_e3.u.ictl_ch2 = (gt_aqic_reg_trim_ictl_ch2_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch2 = gt_aqic_reg_trim_ictl_ch2_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB2 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch2_8bit > 0)
                        {
                            gt_aqic_reg_trim_ictl_ch2_8bit -= gn_trim_ictl_step[1];
                            gt_aqic_reg_trim_e3.u.ictl_ch2 = (gt_aqic_reg_trim_ictl_ch2_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch2 = gt_aqic_reg_trim_ictl_ch2_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB2 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }


                    else if (gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_NONE)
                    {

                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch3_8bit < 255)
                        {
                            gt_aqic_reg_trim_ictl_ch3_8bit += gn_trim_ictl_step[2];
                            gt_aqic_reg_trim_e4.u.ictl_ch3 = (gt_aqic_reg_trim_ictl_ch3_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch3 = gt_aqic_reg_trim_ictl_ch3_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB 3 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch3_8bit > 0)
                        {
                            gt_aqic_reg_trim_ictl_ch3_8bit -= gn_trim_ictl_step[2];
                            gt_aqic_reg_trim_e4.u.ictl_ch3 = (gt_aqic_reg_trim_ictl_ch3_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch3 = gt_aqic_reg_trim_ictl_ch3_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB 3 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }


                    else if (gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_NONE)
                    {
                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch4_8bit < 255)
                        {
                            gt_aqic_reg_trim_ictl_ch4_8bit += gn_trim_ictl_step[3];
                            gt_aqic_reg_trim_e5.u.ictl_ch4 = (gt_aqic_reg_trim_ictl_ch4_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch4 = gt_aqic_reg_trim_ictl_ch4_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB 4 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch4_8bit > 0)
                        {
                            gt_aqic_reg_trim_ictl_ch4_8bit -= gn_trim_ictl_step[3];
                            gt_aqic_reg_trim_e5.u.ictl_ch4 = (gt_aqic_reg_trim_ictl_ch4_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch4 = gt_aqic_reg_trim_ictl_ch4_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB 4 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }


                    else if (gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_NONE)
                    {
                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch5_8bit < 255)
                        {
                            gt_aqic_reg_trim_ictl_ch5_8bit += gn_trim_ictl_step[4];
                            gt_aqic_reg_trim_e6.u.ictl_ch5 = (gt_aqic_reg_trim_ictl_ch5_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch5 = gt_aqic_reg_trim_ictl_ch5_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB 5 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch5_8bit > 0)
                        {
                            gt_aqic_reg_trim_ictl_ch5_8bit -= gn_trim_ictl_step[4];
                            gt_aqic_reg_trim_e6.u.ictl_ch5 = (gt_aqic_reg_trim_ictl_ch5_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch5 = gt_aqic_reg_trim_ictl_ch5_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB 5 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }


                    else if (gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_NONE)
                    {
                    }

                    if(gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_PLUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch6_8bit < 255)
                        {
                            gt_aqic_reg_trim_ictl_ch6_8bit += gn_trim_ictl_step[5];
                            gt_aqic_reg_trim_e7.u.ictl_ch6 = (gt_aqic_reg_trim_ictl_ch6_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch6 = gt_aqic_reg_trim_ictl_ch6_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB 6 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if(gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_MINUS)
                    {
                        if(gt_aqic_reg_trim_ictl_ch6_8bit > 0)
                        {
                            gt_aqic_reg_trim_ictl_ch6_8bit -= gn_trim_ictl_step[5];
                            gt_aqic_reg_trim_e7.u.ictl_ch6 = (gt_aqic_reg_trim_ictl_ch6_8bit & 0xFE) >> 1;
                            gt_aqic_reg_trim_e8.u.ictl_ch6 = gt_aqic_reg_trim_ictl_ch6_8bit & 0x01;
                        }
                        else
                        {
                            print("======== ICTL_LSB 6 - CAN NOT trim ========\r\n");
                            /* NG - stop APIC trimming */
                            gn_step_delay = 0;
                            gn_trim_out_of_spec_stop_trimming = 1;
                        }
                    }
                    else if (gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_NONE)
                    {
                    }
                    if(gn_trim_out_of_spec_stop_trimming == 0)
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                    }
                    else
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_STOP;
                    }
                    // print("\t  REG WRITE \r\n\r\n");

                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM12, 1, &(gt_aqic_reg_trim_e2.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM13, 1, &(gt_aqic_reg_trim_e3.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM14, 1, &(gt_aqic_reg_trim_e4.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM15, 1, &(gt_aqic_reg_trim_e5.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM16, 1, &(gt_aqic_reg_trim_e6.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM17, 1, &(gt_aqic_reg_trim_e7.value), 1, 10);
                    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM18, 1, &(gt_aqic_reg_trim_e8.value), 1, 10);
                    // apic_get_test_regs();
                    break;
            }
            break;

        case TRIMMING_STEP_SET_ADC_CH:
            gn_ads114s08_adc_temp = 0;
            ads114s08_select_single_ended_input(0);
            gt_jig_trimming_step = TRIMMING_STEP_START_ADC_CONVERSION;
            break;

        case TRIMMING_STEP_START_ADC_CONVERSION:
            gb_ads114s08_drdy_done = 0;
            gn_ads114s08_adc_temp = 0;

            gn_adc_read_count = ADS114S08_READ_COUNT;
            gt_jig_trimming_step = TRIMMING_STEP_GET_ADC_CH;

            ads114s08_set_start(1);
            break;

        case TRIMMING_STEP_GET_ADC_CH:
            if(gb_ads114s08_drdy_done == 1)
            {
            	if (gt_trim_search_mode == TRIM_ICTL_CHS || gt_trim_search_mode == TRIM_OFS_LIN_CHS)
                {
                    gn_aqic_slope_adc[gn_aqic_slope_cnt][gn_read_adc_vout_channel] = (uint16_t)(gn_ads114s08_adc_temp/ADS114S08_READ_COUNT);
                    ++gn_aqic_slope_cnt;
                    if (gn_aqic_slope_cnt >= 2)
                    {
                        gn_aqic_slope_cnt = 0;
                        gt_jig_trimming_step = TRIMMING_STEP_CHECK;

                    }
                    else
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                    }
                }
                else if(gt_trim_search_mode == TRIM_SCREENING)
                {
                    gn_aqic_screen_adc[gn_read_adc_vout_channel] = (uint16_t)(gn_ads114s08_adc_temp/ADS114S08_READ_COUNT);
                    ++gn_read_adc_vout_channel;
                    if(gn_read_adc_vout_channel >= AQIC_O_MAX)
                    {
                        gn_read_adc_vout_channel = 0;
                        gt_jig_trimming_step = TRIMMING_STEP_CHECK;
                    }
                    else
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                    }
                }

                else
                {
                    gn_trim_adc_result[gt_trim_search_mode][gn_read_adc_vout_channel] = (uint16_t)(gn_ads114s08_adc_temp/ADS114S08_READ_COUNT);
                    ++gn_read_adc_vout_channel;
                    if(gn_read_adc_vout_channel >= AQIC_O_MAX)
                    {
                        gn_read_adc_vout_channel = 0;
                        gt_jig_trimming_step = TRIMMING_STEP_CHECK;
                    }
                    else
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                    }
                }
                current_discharge(DISCHARGE);
                HAL_Delay(1);
                gb_ads114s08_drdy_done = 0;
            }
            break;
        case TRIMMING_STEP_CHECK:
            switch(gt_trim_search_mode)
            {
                case TRIM_OFS_LIN_CHS:
                    ++gn_aqic_trim_ofs_cnt;
                    temp = 0;
                    temp_adjust_trim_range_min = 0;
                    temp_adjust_trim_range_max = 0;
                    temp_adjust_trim_range_half = 0;
                    ofs_regi_value = 0;
                    temp_adjust_trim_range_min = gt_adjust_trim_range[gt_trim_search_mode].min;
                    temp_adjust_trim_range_max = gt_adjust_trim_range[gt_trim_search_mode].max;
                    temp_adjust_trim_range_half = (uint16_t)((temp_adjust_trim_range_max - temp_adjust_trim_range_min)/2);

                    if(gn_read_adc_vout_channel < AQIC_O_MAX)
                    {
                        channel = gn_read_adc_vout_channel;
                        ofs_regi_value = Get_ofs_regi_value(channel);
                        switch (channel)
                        {
                        case AQIC_O1 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O1];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O1];
                            break;
                        case AQIC_O2 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O2];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O2];
                            break;
                        case AQIC_O3 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O3];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O3];
                            break;
                        case AQIC_O4 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O4];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O4];
                            break;
                        case AQIC_O5 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O5];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O5];
                            break;
                        case AQIC_O6 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O6];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O6];
                            break;
                        }

                        gn_aqic_slope_adc_temp[channel] = (gn_aqic_slope_adc[1][channel] + gn_aqic_slope_adc[0][channel])/2;

                        // TRIM_OFS_LIN_CHS is uA
                        gn_aqic_slope[channel] = 1000 * convert_adc_to_current(channel, gn_aqic_slope_adc_temp[channel], gn_trim_current_gain);

                        if(gn_aqic_trim_ofs_cnt == 1) // Initial adc per register
                        {
                            ub_adc_per_register = INIT_OFS_LIN_CHS_ADC_PER_REGI;
                        }
                        else
                        {
                            ub_adc_per_register = abs(gn_aqic_slope_adc_ofs_pre[channel] - gn_aqic_slope_adc_temp[channel]) / gn_trim_ofs_step[channel];
                        }

                        #ifdef DBG_TRIM_ALGORITHM
                        snprintf(msg_buffer, sizeof msg_buffer, "gn_aqic_slope_adc_ofs_pre[channel]:%d / gn_aqic_slope_adc_temp[channel]:%d gn_trim_ofs_step[channel]:%d\r\n ",gn_aqic_slope_adc_ofs_pre[channel], gn_aqic_slope_adc_temp[channel], gn_trim_ofs_step[channel]);
                        print(msg_buffer);
                        #endif //#ifdef DBG_TRIM_ALGORITHM
                        gn_aqic_slope_adc_ofs_pre[channel] = gn_aqic_slope_adc_temp[channel];

                        if(gn_aqic_slope_adc_temp[channel] < temp_adjust_trim_range_min)
                        {
                            temp |= (1<<channel);
                            gn_trim_adjust_flag[gt_trim_search_mode][channel] = ADJ_PLUS;
                            if(ub_adc_per_register == 0)
                            {
                                gn_trim_ofs_step[channel] = 1;
                            }
                            else
                            {
                                if (temp_adjust_trim_range_min - gn_aqic_slope_adc_temp[channel] >= ub_adc_per_register*2)
                                {
                                    if(check_valid_ofs_step((uint8_t)((temp_adjust_trim_range_min - gn_aqic_slope_adc_temp[channel])/(ub_adc_per_register)), channel, ADJ_PLUS))
                                    {
                                        gn_trim_ofs_step[channel] = (uint8_t)((temp_adjust_trim_range_min - gn_aqic_slope_adc_temp[channel])/(ub_adc_per_register));
                                        if(gn_trim_ofs_step[channel] == 0)
                                        {
                                            gn_trim_ofs_step[channel] = 1;
                                        }
                                    }
                                    else
                                    {
                                        gn_trim_ofs_step[channel] = 1;
                                    }
                                }
                                else
                                {
                                    gn_trim_ofs_step[channel] = 1;
                                }
                            }

                            // Check Additional Margin is matched.
                            if(gn_aqic_slope_adc_temp[channel] > (temp_adjust_trim_range_min - temp_adjust_trim_range_half))
                            {
                                gn_trim_ofs_regi_saved[(gn_aqic_trim_ofs_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_regi = ofs_regi_value;
                                gn_trim_ofs_regi_saved[(gn_aqic_trim_ofs_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_adc = gn_aqic_slope_adc_temp[channel];
                                ++gn_aqic_trim_ofs_match_cnt;
                                #ifdef DBG_TRIM_ALGORITHM
                                snprintf(msg_buffer, sizeof msg_buffer, "RANGE_MARGIN_OFS-ADJ_PLUS[%d]]\r\n",gn_aqic_trim_ofs_match_cnt);
                                print(msg_buffer);
                                #endif //#ifdef DBG_TRIM_ALGORITHM
                            }

                        }
                        else if(gn_aqic_slope_adc_temp[channel] > temp_adjust_trim_range_max)
                        {
                            temp |= (1<<channel);
                            gn_trim_adjust_flag[gt_trim_search_mode][channel] = ADJ_MINUS;
                            if(ub_adc_per_register == 0)
                            {
                                gn_trim_ofs_step[channel] = 1;
                            }
                            else
                            {
                                if (gn_aqic_slope_adc_temp[channel] - temp_adjust_trim_range_max >= ub_adc_per_register*2)
                                {
                                    if(check_valid_ofs_step((uint8_t)((gn_aqic_slope_adc_temp[channel] - temp_adjust_trim_range_max)/(ub_adc_per_register)), channel, ADJ_MINUS))
                                    {
                                        gn_trim_ofs_step[channel] = (uint8_t)((gn_aqic_slope_adc_temp[channel] - temp_adjust_trim_range_max)/(ub_adc_per_register));
                                        if(gn_trim_ofs_step[channel] == 0)
                                        {
                                            gn_trim_ofs_step[channel] = 1;
                                        }
                                    }
                                    else
                                    {
                                        gn_trim_ofs_step[channel] = 1;
                                    }
                                }
                                else
                                {
                                    gn_trim_ofs_step[channel] = 1;
                                }
                            }

                            // Check Additional Margin is matched.
                            if(gn_aqic_slope_adc_temp[channel] < (temp_adjust_trim_range_max + temp_adjust_trim_range_half))
                            {
                                gn_trim_ofs_regi_saved[(gn_aqic_trim_ofs_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_regi = ofs_regi_value;
                                gn_trim_ofs_regi_saved[(gn_aqic_trim_ofs_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_adc = gn_aqic_slope_adc_temp[channel];
                                ++gn_aqic_trim_ofs_match_cnt;
                                #ifdef DBG_TRIM_ALGORITHM
                                snprintf(msg_buffer, sizeof msg_buffer, "RANGE_MARGIN_OFS-ADJ_MINUS[%d]\r\n",gn_aqic_trim_ofs_match_cnt);
                                print(msg_buffer);
                                #endif //#ifdef DBG_TRIM_ALGORITHM
                            }
                        }
                        else
                        {
                            gn_trim_adjust_flag[gt_trim_search_mode][channel] = ADJ_NONE;
                            gn_trim_ofs_step[channel] = 0;

                            for(int i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
                            {
                                if(gn_trim_ofs_regi_saved[i].u16_saved_regi == ofs_regi_value) //If there is an 2 times matched
                                {
                                    ++gn_read_adc_vout_channel;
                                    gn_aqic_trim_ofs_match_cnt = 0;
                                    gn_aqic_trim_ofs_cnt = 0;
                                    for(int k = 0; k < TRIM_REGISTER_SAVED_CNT; k++)
                                    {
                                        gn_trim_ofs_regi_saved[k].u16_saved_regi = 0;
                                        gn_trim_ofs_regi_saved[k].u16_saved_adc = 0;
                                    }
                                    break;
                                }
                            }

                            // If there is not an 2 times matched,
                            if(gn_read_adc_vout_channel == channel)
                            {
                                gn_trim_ofs_regi_saved[(gn_aqic_trim_ofs_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_regi = ofs_regi_value;
                                gn_trim_ofs_regi_saved[(gn_aqic_trim_ofs_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_adc = gn_aqic_slope_adc_temp[channel];
                                ++gn_aqic_trim_ofs_match_cnt;
                            }
                        }
                        #ifdef DBG_TRIM_ALGORITHM
                        snprintf(msg_buffer, sizeof msg_buffer, "ub_adc_per_register:%d / gn_trim_ofs_step[%d]:%d\r\n ",ub_adc_per_register, channel, gn_trim_ofs_step[channel]);
                        print(msg_buffer);
                        #endif //#ifdef DBG_TRIM_ALGORITHM
                    }

                    // Check Vibration
                    if(gn_aqic_trim_ofs_match_cnt >= TRIM_REGISTER_SAVED_CNT)
                    {
                        uint16_t ui_target_adc = (temp_adjust_trim_range_max + temp_adjust_trim_range_max)/2;
                        uint16_t ui_adc_gap_closest = 0xffff;
                        uint16_t ui_adc_gap_temp = 0;
                        uint8_t ub_closest_adc_index = 0;
                        for(uint8_t i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
                        {
                            ui_adc_gap_temp = abs(gn_trim_ofs_regi_saved[i].u16_saved_adc - ui_target_adc);
                            if( ui_adc_gap_temp < ui_adc_gap_closest )
                            {
                                ui_adc_gap_closest = ui_adc_gap_temp;
                                ub_closest_adc_index = i;
                            }
                        }
                        // Write Register
                        snprintf(msg_buffer, sizeof msg_buffer, "********ADJUST_OVER_RANGE(%d,%d)********\r\n",channel+1, gn_trim_ofs_regi_saved[ub_closest_adc_index].u16_saved_regi);
                        print(msg_buffer);
                        Set_ofs_regi_value(channel, gn_trim_ofs_regi_saved[ub_closest_adc_index].u16_saved_regi);
                        ++gn_read_adc_vout_channel;
                        gn_aqic_trim_ofs_match_cnt = 0;
                        gn_aqic_trim_ofs_cnt = 0;
                        gn_trim_ofs_step[channel] = 0;
                    }

                    // Check Limit count of trying
                    if(gn_aqic_trim_ofs_cnt > TRIM_REGISTER_SAVED_CNT + TRIM_OUT_RANGE_CNT)
                    {
                        snprintf(msg_buffer, sizeof msg_buffer, "ERROR!! TRIM_OFS_LIN_CH[%d] : RETRY COUNT is OVER %d\r\n",channel+1, TRIM_REGISTER_SAVED_CNT + TRIM_OUT_RANGE_CNT);
                        print(msg_buffer);
                        gn_trim_out_of_spec_stop_trimming = 1;
                        gt_jig_trimming_step = TRIMMING_STEP_NONE;
                        break;
                    }

                    if(gn_aqic_trim_ofs_cnt == 1)
                    {
                        snprintf(msg_buffer, sizeof msg_buffer, "TRIM_OFS_LIN_CH[%d]\r\n",channel+1);
                        print(msg_buffer);
                        #ifdef DBG_TRIM_ALGORITHM
                        snprintf(msg_buffer, sizeof msg_buffer, "RANGE_MARGIN_OFS-ADJ_PLUS:%d\r\n",(temp_adjust_trim_range_min - temp_adjust_trim_range_half));
                        print(msg_buffer);
                        snprintf(msg_buffer, sizeof msg_buffer, "RANGE_MARGIN_OFS-ADJ_MINUS:%d\r\n", (temp_adjust_trim_range_max + temp_adjust_trim_range_half));
                        print(msg_buffer);
                        #endif //#ifdef DBG_TRIM_ALGORITHM
                        print("[Cnt]    [RANGE]         [ADC]    [uA]       [uA]       [uA]      [REG]  [Check]\r\n");
                    }
                    snprintf(msg_buffer, sizeof msg_buffer, "  %d    %7u/%7u %7u   %7.3f   %7.3f   %7.3f   %7u  %7s\r\n",
                        gn_aqic_trim_ofs_cnt, temp_adjust_trim_range_min, temp_adjust_trim_range_max,
                        gn_aqic_slope_adc_temp[channel], gn_aqic_slope[channel], convert_adc_to_current(channel, gn_aqic_slope_adc[0][channel], gn_trim_current_gain), convert_adc_to_current(channel, gn_aqic_slope_adc[1][channel], gn_trim_current_gain), ofs_regi_value,
                        (gn_trim_adjust_flag[gt_trim_search_mode][channel] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][channel] == ADJ_MINUS ? "MINUS" : "NONE")));
                    print(msg_buffer);

                    if(gn_read_adc_vout_channel >= AQIC_O_MAX)
                    {
                        snprintf(msg_buffer, sizeof msg_buffer, "[RANGE]   %7u   %7u\r\n",
                            temp_adjust_trim_range_min, temp_adjust_trim_range_max);
                        print(msg_buffer);
                        snprintf(msg_buffer, sizeof msg_buffer, "[ADC]     %7u   %7u   %7u   %7u   %7u   %7u\r\n",
                            gn_aqic_slope_adc_temp[0], gn_aqic_slope_adc_temp[1], gn_aqic_slope_adc_temp[2], gn_aqic_slope_adc_temp[3],
                            gn_aqic_slope_adc_temp[4], gn_aqic_slope_adc_temp[5]);
                        print(msg_buffer);
                        snprintf(msg_buffer, sizeof msg_buffer, "[uA]      %7.3f   %7.3f   %7.3f   %7.3f   %7.3f   %7.3f\r\n",
                            gn_aqic_slope[0], gn_aqic_slope[1], gn_aqic_slope[2], gn_aqic_slope[3],
                            gn_aqic_slope[4], gn_aqic_slope[5]);
                        print(msg_buffer);
                        snprintf(msg_buffer, sizeof msg_buffer, "[REG]     %7u   %7u   %7u   %7u   %7u   %7u\r\n",
                            gt_aqic_reg_trim_ofs_lin_ch1_11bit, gt_aqic_reg_trim_ofs_lin_ch2_11bit, gt_aqic_reg_trim_ofs_lin_ch3_11bit, gt_aqic_reg_trim_ofs_lin_ch4_11bit,
                            gt_aqic_reg_trim_ofs_lin_ch5_11bit, gt_aqic_reg_trim_ofs_lin_ch6_11bit);
                        print(msg_buffer);
                        snprintf(msg_buffer, sizeof msg_buffer, "[Check]   %7s   %7s   %7s   %7s   %7s   %7s\r\n",
                        (gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_MINUS ? "MINUS" : "NONE")));
                        print(msg_buffer);

                        gn_aqic_trim_ofs_match_cnt = 0;
                        gn_read_adc_vout_channel = 0;
                        gn_aqic_trim_ofs_cnt = 0;

                        print("\t  AQ06_CH - DONE:EXIT\r\n\r\n");

                        apic_get_test_regs();
                        dump_apic_regs(0);

                        GUI_SEND_reg_all();
                        #ifdef NO_SCREEN_AFTER_TRIMMING
                            gt_trim_search_mode = TRIM_SCREENING;
                            gt_jig_trimming_step = TRIMMING_STEP_NONE;
                        #else
                            gt_trim_search_mode = TRIM_SCREENING;
                            gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                        #endif
                    }
                    else
                    {
						// print("\t  REPEAT \r\n");
                        gt_trim_search_mode = TRIM_OFS_LIN_CHS;
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_REGISTER;
                    }
                    break;

                case TRIM_ICTL_CHS:
                    ++gn_aqic_trim_ictl_cnt;
                    temp = 0;
                    temp_adjust_trim_range_min = 0;
                    temp_adjust_trim_range_max = 0;
                    temp_adjust_trim_range_half = 0;
                    ictl_regi_value = 0;

                    temp_adjust_trim_range_min = gt_adjust_trim_range[gt_trim_search_mode].min;
                    temp_adjust_trim_range_max = gt_adjust_trim_range[gt_trim_search_mode].max;

                    temp_adjust_trim_range_half = (uint16_t)((temp_adjust_trim_range_max - temp_adjust_trim_range_min)/2);

                    if(gn_read_adc_vout_channel < AQIC_O_MAX)
                    {
                        channel = gn_read_adc_vout_channel;
                        ictl_regi_value = Get_ictl_regi_value(channel);
                        switch (channel)
                        {
                        case AQIC_O1 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O1];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O1];
                            break;
                        case AQIC_O2 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O2];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O2];
                            break;
                        case AQIC_O3 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O3];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O3];
                            break;
                        case AQIC_O4 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O4];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O4];
                            break;
                        case AQIC_O5 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O5];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O5];
                            break;
                        case AQIC_O6 :
                            gn_aqic_slope_adc[1][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O6];
                            gn_aqic_slope_adc[0][channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O6];
                            break;
                        }

                        gn_aqic_slope_adc_temp[channel] = (gn_aqic_slope_adc[1][channel] - gn_aqic_slope_adc[0][channel]);

                        if(gn_aqic_trim_ictl_cnt == 1) // Initial adc per register
                        {
                            ub_adc_per_register = INIT_ICTL_CHS_ADC_PER_REGI;
                        }
                        else
                        {
                            ub_adc_per_register = abs(gn_aqic_slope_adc_ictl_pre[channel] - gn_aqic_slope_adc_temp[channel]) / gn_trim_ictl_step[channel];
                        }

                        gn_aqic_slope_adc_ictl_pre[channel] = gn_aqic_slope_adc_temp[channel];
                        gn_aqic_slope[channel] = convert_adc_to_current(channel, gn_aqic_slope_adc_temp[channel], gn_trim_current_gain);
                        if (gn_aqic_slope_adc_temp[channel] < temp_adjust_trim_range_min)
                        {
                            temp |= (1<<channel);
                            gn_trim_adjust_flag[gt_trim_search_mode][channel] = ADJ_PLUS;
                            if(ub_adc_per_register == 0)
                            {
                                gn_trim_ictl_step[channel] = 1;
                            }
                            else
                            {
                                if (temp_adjust_trim_range_min - gn_aqic_slope_adc_temp[channel] >= ub_adc_per_register*2)
                                {
                                    if(check_valid_ictl_step((uint8_t)((temp_adjust_trim_range_min - gn_aqic_slope_adc_temp[channel])/(ub_adc_per_register)), channel, ADJ_PLUS) )
                                    {
                                        gn_trim_ictl_step[channel] = (uint8_t)((temp_adjust_trim_range_min - gn_aqic_slope_adc_temp[channel])/(ub_adc_per_register));
                                        if(gn_trim_ictl_step[channel] == 0)
                                        {
                                            gn_trim_ictl_step[channel] = 1;
                                        }
                                    }
                                    else
                                    {
                                        gn_trim_ictl_step[channel] = 1;
                                    }
                                }
                                else
                                {
                                    gn_trim_ictl_step[channel] = 1;
                                }
                            }

                            // Check Additional Margin is matched.
                            if(gn_aqic_slope_adc_temp[channel] > (temp_adjust_trim_range_min - temp_adjust_trim_range_half))
                            {
                                gn_trim_ictl_regi_saved[(gn_aqic_trim_ictl_match_cnt % TRIM_REGISTER_SAVED_CNT)].u8_saved_regi = ictl_regi_value;
                                gn_trim_ictl_regi_saved[(gn_aqic_trim_ictl_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_adc = gn_aqic_slope_adc_temp[channel];
                                ++gn_aqic_trim_ictl_match_cnt;
                                #ifdef DBG_TRIM_ALGORITHM
                                snprintf(msg_buffer, sizeof msg_buffer, "RANGE_MARGIN_ICTL-ADJ_PLUS[%d]]\r\n",gn_aqic_trim_ictl_match_cnt);
                                print(msg_buffer);
                                #endif
                            }
                        }
                        else if(gn_aqic_slope_adc_temp[channel] > temp_adjust_trim_range_max)
                        {
                            temp |= (1<<channel);
                            gn_trim_adjust_flag[gt_trim_search_mode][channel] = ADJ_MINUS;
                            if(ub_adc_per_register == 0)
                            {
                                gn_trim_ictl_step[channel] = 1;
                            }
                            else
                            {
                                if (gn_aqic_slope_adc_temp[channel] - temp_adjust_trim_range_max >= ub_adc_per_register*2)
                                {
                                    if(check_valid_ictl_step((uint8_t)((gn_aqic_slope_adc_temp[channel] - temp_adjust_trim_range_max)/(ub_adc_per_register)), channel, ADJ_MINUS) )
                                    {
                                        gn_trim_ictl_step[channel] = (uint8_t)((gn_aqic_slope_adc_temp[channel] - temp_adjust_trim_range_max)/(ub_adc_per_register));
                                        if(gn_trim_ictl_step[channel] == 0)
                                        {
                                            gn_trim_ictl_step[channel] = 1;
                                        }
                                    }
                                    else
                                    {
                                        gn_trim_ictl_step[channel] = 1;
                                    }
                                }
                                else
                                {
                                    gn_trim_ictl_step[channel] = 1;
                                }
                            }

                            // Check Additional Margin is matched.
                            if(gn_aqic_slope_adc_temp[channel] < (temp_adjust_trim_range_max + temp_adjust_trim_range_half))
                            {
                                gn_trim_ictl_regi_saved[(gn_aqic_trim_ictl_match_cnt % TRIM_REGISTER_SAVED_CNT)].u8_saved_regi = ictl_regi_value;
                                gn_trim_ictl_regi_saved[(gn_aqic_trim_ictl_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_adc = gn_aqic_slope_adc_temp[channel];
                                ++gn_aqic_trim_ictl_match_cnt;
                                #ifdef DBG_TRIM_ALGORITHM
                                snprintf(msg_buffer, sizeof msg_buffer, "gn_aqic_trim_ictl_match_cnt[%d]\r\n",gn_aqic_trim_ictl_match_cnt);
                                print(msg_buffer);
                                #endif
                            }
                        }
                        else
                        {
                            gn_trim_adjust_flag[gt_trim_search_mode][channel] = ADJ_NONE;
                            gn_trim_ictl_step[channel] = 0;

                            for(int i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
                            {
                                if(gn_trim_ictl_regi_saved[i].u8_saved_regi == ictl_regi_value) //If there is an 2 times matched
                                {
                                    ++gn_read_adc_vout_channel;
                                    gn_aqic_trim_ictl_match_cnt = 0;
                                    gn_aqic_trim_ictl_cnt = 0;
                                    for(int k = 0; k < TRIM_REGISTER_SAVED_CNT; k++)
                                    {
                                        gn_trim_ictl_regi_saved[k].u8_saved_regi = 0;
                                        gn_trim_ictl_regi_saved[k].u16_saved_adc = 0;
                                    }
                                    break;
                                }
                            }

                            // If there is not an 2 times matched,
                            if(gn_read_adc_vout_channel == channel)
                            {
                                gn_trim_ictl_regi_saved[(gn_aqic_trim_ictl_match_cnt % TRIM_REGISTER_SAVED_CNT)].u8_saved_regi = ictl_regi_value;
                                gn_trim_ictl_regi_saved[(gn_aqic_trim_ictl_match_cnt % TRIM_REGISTER_SAVED_CNT)].u16_saved_adc = gn_aqic_slope_adc_temp[channel];
                                ++gn_aqic_trim_ictl_match_cnt;
                            }
                        }
                        #ifdef DBG_TRIM_ALGORITHM
                        snprintf(msg_buffer, sizeof msg_buffer, "ub_adc_per_register:%d / gn_trim_ofs_step[%d]:%d\r\n",ub_adc_per_register, channel, gn_trim_ictl_step[channel]);
                        print(msg_buffer);
                        #endif //#ifdef DBG_TRIM_ALGORITHM
                    }

                    // Check Vibration
                    if(gn_aqic_trim_ictl_match_cnt >= TRIM_REGISTER_SAVED_CNT)
                    {
                        uint16_t ui_target_adc = (temp_adjust_trim_range_max + temp_adjust_trim_range_max)/2;
                        uint16_t ui_adc_gap_closest = 0xffff;
                        uint16_t ui_adc_gap_temp = 0;
                        uint8_t ub_closest_adc_index = 0;
                        for(uint8_t i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
                        {
                            ui_adc_gap_temp = abs(gn_trim_ictl_regi_saved[i].u16_saved_adc - ui_target_adc);
                            if( ui_adc_gap_temp < ui_adc_gap_closest )
                            {
                                ui_adc_gap_closest = ui_adc_gap_temp;
                                ub_closest_adc_index = i;
                            }
                        }
                        // Write Register
                        snprintf(msg_buffer, sizeof msg_buffer, "********ADJUST_OVER_RANGE(%d,%d)********\r\n",channel+1, gn_trim_ictl_regi_saved[ub_closest_adc_index].u8_saved_regi);
                        print(msg_buffer);
                        Set_ictl_regi_value(channel, gn_trim_ictl_regi_saved[ub_closest_adc_index].u8_saved_regi);
                        ++gn_read_adc_vout_channel;
                        gn_aqic_trim_ictl_match_cnt = 0;
                        gn_aqic_trim_ictl_cnt = 0;
                        gn_trim_ictl_step[channel] = 0;
                    }

                    // Check Limit count of trying
                    if(gn_aqic_trim_ictl_cnt > TRIM_REGISTER_SAVED_CNT + TRIM_OUT_RANGE_CNT)
                    {
                        snprintf(msg_buffer, sizeof msg_buffer, "ERROR!! TRIM_ICTL_CH[%d] : RETRY COUNT is OVER %d\r\n",channel+1, TRIM_REGISTER_SAVED_CNT + TRIM_OUT_RANGE_CNT);
                        print(msg_buffer);
                        gn_trim_out_of_spec_stop_trimming = 1;
                        gt_jig_trimming_step = TRIMMING_STEP_NONE;
                        break;
                    }

                    if(gn_aqic_trim_ictl_cnt == 1)
                    {
                        snprintf(msg_buffer, sizeof msg_buffer, "TRIM_ICTL_CH[%d]\r\n",channel+1);
                        print(msg_buffer);
                        #ifdef DBG_TRIM_ALGORITHM
                        snprintf(msg_buffer, sizeof msg_buffer, "RANGE_MARGIN_ICTL-ADJ_PLUS:%d\r\n",(temp_adjust_trim_range_min - temp_adjust_trim_range_half));
                        print(msg_buffer);
                        snprintf(msg_buffer, sizeof msg_buffer, "RANGE_MARGIN_ICTL-ADJ_MINUS:%d\r\n", (temp_adjust_trim_range_max + temp_adjust_trim_range_half));
                        print(msg_buffer);
                        #endif //#ifdef DBG_TRIM_ALGORITHM
                        print("[Cnt]    [RANGE]       [ADC]    [mA]      [REG]  [Check]\r\n");
                    }
                    snprintf(msg_buffer, sizeof msg_buffer, "  %d    %7u/%7u %7u %7.4f   %7u  %7s\r\n",
                        gn_aqic_trim_ictl_cnt, temp_adjust_trim_range_min, temp_adjust_trim_range_max,
                        gn_aqic_slope_adc_temp[channel], gn_aqic_slope[channel], ictl_regi_value,
                        (gn_trim_adjust_flag[gt_trim_search_mode][channel] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][channel] == ADJ_MINUS ? "MINUS" : "NONE")));
                    print(msg_buffer);

                    if(gn_read_adc_vout_channel >= AQIC_O_MAX)
                    {
                        snprintf(msg_buffer, sizeof msg_buffer, "[RANGE]   %7u   %7u\r\n",
                            temp_adjust_trim_range_min, temp_adjust_trim_range_max);
                        print(msg_buffer);
                        snprintf(msg_buffer, sizeof msg_buffer, "[ADC]     %7u   %7u   %7u   %7u   %7u   %7u\r\n",
                            gn_aqic_slope_adc_temp[0], gn_aqic_slope_adc_temp[1], gn_aqic_slope_adc_temp[2], gn_aqic_slope_adc_temp[3],
                            gn_aqic_slope_adc_temp[4], gn_aqic_slope_adc_temp[5]);
                        print(msg_buffer);
                        snprintf(msg_buffer, sizeof msg_buffer, "[mA]      %7.3f   %7.3f   %7.3f   %7.3f   %7.3f   %7.3f\r\n",
                            gn_aqic_slope[0], gn_aqic_slope[1], gn_aqic_slope[2], gn_aqic_slope[3],
                            gn_aqic_slope[4], gn_aqic_slope[5]);
                        print(msg_buffer);

                        snprintf(msg_buffer, sizeof msg_buffer, "[REG]     %7u   %7u   %7u   %7u   %7u   %7u\r\n",
                            ((gt_aqic_reg_trim_e2.u.ictl_ch1 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch1 & 0x01),
                            ((gt_aqic_reg_trim_e3.u.ictl_ch2 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch2 & 0x01),
                            ((gt_aqic_reg_trim_e4.u.ictl_ch3 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch3 & 0x01),
                            ((gt_aqic_reg_trim_e5.u.ictl_ch4 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch4 & 0x01),
                            ((gt_aqic_reg_trim_e6.u.ictl_ch5 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch5 & 0x01),
                            ((gt_aqic_reg_trim_e7.u.ictl_ch6 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch6 & 0x01));
                        print(msg_buffer);

                        snprintf(msg_buffer, sizeof msg_buffer, "[Check]   %7s   %7s   %7s   %7s   %7s   %7s\r\n",
                        (gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][0] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][1] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][2] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][3] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][4] == ADJ_MINUS ? "MINUS" : "NONE")),
                        (gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_PLUS ? "PLUS" : (gn_trim_adjust_flag[gt_trim_search_mode][5] == ADJ_MINUS ? "MINUS" : "NONE")));
                        print(msg_buffer);
                        print("\t  DONE\r\n\r\n");

                        gt_trim_search_mode = TRIM_OFS_LIN_CHS;
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                        for (uint8_t ch = 0; ch < AQIC_O_MAX ; ch++)
                        {
                            gn_trim_adjust_flag[gt_trim_search_mode][ch] = ADJ_DEFAULT;
                        }
                        gn_aqic_trim_ictl_match_cnt = 0;
                        gn_read_adc_vout_channel = 0;
                        gn_aqic_trim_ictl_cnt = 0;
                    }
                    else
                    {
						// print("\t  REPEAT \r\n");
                        gt_trim_search_mode = TRIM_ICTL_CHS;
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_REGISTER;
                    }
                    break;

                case TRIM_SCREENING:
                    temp = 0;
                    for (uint8_t channel = 0 ; channel < AQIC_O_MAX ; channel++)
                    {
                        switch (channel)
                        {
                        case AQIC_O1 :
                            gn_aqic_screen_adc[channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O1];
                            break;
                        case AQIC_O2 :
                            gn_aqic_screen_adc[channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O2];
                            break;
                        case AQIC_O3 :
                            gn_aqic_screen_adc[channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O3];
                            break;
                        case AQIC_O4 :
                            gn_aqic_screen_adc[channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O4];
                            break;
                        case AQIC_O5 :
                            gn_aqic_screen_adc[channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O5];
                            break;
                        case AQIC_O6 :
                            gn_aqic_screen_adc[channel] -= gn_adc_compensate[gn_trim_current_gain][AQIC_O6];
                            break;
                        }
                    }
                    for (uint8_t ch = 0 ; ch < AQIC_O_MAX ; ch++)
                    {
                        screen_cur[ch] = convert_adc_to_current(0, gn_aqic_screen_adc[ch], gn_trim_current_gain);
                        screen_avg += (screen_cur[ch]/AQIC_O_MAX);
                    }

                    for (uint8_t ch = 0 ; ch < AQIC_O_MAX ; ch++)
                    {
                        screen_dev[ch] = 100 * (screen_cur[ch] - screen_avg) / screen_avg;
                    }

                    print("jig:csvu:");
                    snprintf(msg_buffer, sizeof msg_buffer, "%5.3f:", gf_trim_voltage_table[gt_trim_search_mode][0] + gn_screen_cnt * SCREEN_STEP);
                    print(msg_buffer);
                    snprintf(msg_buffer, sizeof msg_buffer, "%7.3f:%7.3f:%7.3f:%7.3f:%7.3f:%7.3f:", screen_cur[0], screen_cur[1], screen_cur[2], screen_cur[3], screen_cur[4], screen_cur[5]);
                    print(msg_buffer);
                    snprintf(msg_buffer, sizeof msg_buffer, "%7.3f:%7.3f:%7.3f:%7.3f:%7.3f:%7.3f\r\n", screen_dev[0], screen_dev[1], screen_dev[2], screen_dev[3], screen_dev[4], screen_dev[5]);
                    print(msg_buffer);

                    if (gub_TRIM_ENABLE == 1)
                    {
                        if(temp == 0)
                        {
                            gt_trim_search_mode = TRIM_ICTL_CHS;
                            gt_jig_trimming_step = TRIMMING_STEP_E2P_PROGRAM;
                            // gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                        }
                        else
                        {
                            gt_trim_search_mode = TRIM_ICTL_CHS;
                            gt_jig_trimming_step = TRIMMING_STEP_E2P_PROGRAM;
                        }
                    }
                    else
                    {
                        if(gub_DIN_MODE_ENABLE == TRUE)
                        {
                            if((gf_trim_voltage_table[gt_trim_search_mode][0] + gn_screen_cnt * SCREEN_STEP) >= gf_din_mode_end)
                            {
                                gt_jig_trimming_step = TRIMMING_STEP_STOP;
                                gn_screen_cnt = 0;
                                print("\r\nEND - SCREENING\r\n");
                            }
                            else
                            {
                                ++gn_screen_cnt;
                                gt_trim_search_mode = TRIM_SCREENING;
                                gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                            }
                        }
                        else
                        {
                            if((gf_trim_voltage_table[gt_trim_search_mode][0] + gn_screen_cnt * SCREEN_STEP) > 4.5)
                            {
                                gt_jig_trimming_step = TRIMMING_STEP_STOP;
                                gn_screen_cnt = 0;
                                print("\r\nEND - SCREENING\r\n");
                            }
                            else
                            {
                                ++gn_screen_cnt;
                                gt_trim_search_mode = TRIM_SCREENING;
                                gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                            }
                        }
                    }
                    screen_avg = 0;
                    break;

                case TRIM_OUTPUT_TEST:
                    snprintf(msg_buffer, sizeof msg_buffer, "\t TRIM_OUTPUT_TEST : %5u, %5u, %5u, %5u, %5u, %5u\r\n", gn_trim_adc_result[gt_trim_search_mode][0],gn_trim_adc_result[gt_trim_search_mode][1],gn_trim_adc_result[gt_trim_search_mode][2],gn_trim_adc_result[gt_trim_search_mode][3],gn_trim_adc_result[gt_trim_search_mode][4],gn_trim_adc_result[gt_trim_search_mode][5]);
                    print(msg_buffer);
                    snprintf(msg_buffer, sizeof msg_buffer, "\t              %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f [mV]\r\n", convert_adc_to_voltage(0, gn_trim_adc_result[gt_trim_search_mode][0]),convert_adc_to_voltage(1, gn_trim_adc_result[gt_trim_search_mode][1]),convert_adc_to_voltage(2, gn_trim_adc_result[gt_trim_search_mode][2]),convert_adc_to_voltage(3, gn_trim_adc_result[gt_trim_search_mode][3]),convert_adc_to_voltage(4, gn_trim_adc_result[gt_trim_search_mode][4]),convert_adc_to_voltage(5, gn_trim_adc_result[gt_trim_search_mode][5]));
                    print(msg_buffer);
                    snprintf(msg_buffer, sizeof msg_buffer, "\t              %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f [mA]\r\n", convert_adc_to_current(0, gn_trim_adc_result[gt_trim_search_mode][0], gn_trim_current_gain),convert_adc_to_current(1, gn_trim_adc_result[gt_trim_search_mode][1], gn_trim_current_gain),convert_adc_to_current(2, gn_trim_adc_result[gt_trim_search_mode][2], gn_trim_current_gain),convert_adc_to_current(3, gn_trim_adc_result[gt_trim_search_mode][3], gn_trim_current_gain),convert_adc_to_current(4, gn_trim_adc_result[gt_trim_search_mode][4], gn_trim_current_gain),convert_adc_to_current(5, gn_trim_adc_result[gt_trim_search_mode][5], gn_trim_current_gain));
                    print(msg_buffer);

                    gn_step_delay = 0;
                    gt_jig_trimming_step = TRIMMING_STEP_STOP;
                    break;

                case TRIM_APIC_OUTPUT_TEST:
                    snprintf(msg_buffer, sizeof msg_buffer, "%9.6f,%9.6f,%9.6f,%9.6f,%9.6f,%9.6f\r\n", convert_adc_to_current(0, gn_trim_adc_result[gt_trim_search_mode][0], gn_trim_current_gain),convert_adc_to_current(1, gn_trim_adc_result[gt_trim_search_mode][1], gn_trim_current_gain),convert_adc_to_current(2, gn_trim_adc_result[gt_trim_search_mode][2], gn_trim_current_gain),convert_adc_to_current(3, gn_trim_adc_result[gt_trim_search_mode][3], gn_trim_current_gain),convert_adc_to_current(4, gn_trim_adc_result[gt_trim_search_mode][4], gn_trim_current_gain),convert_adc_to_current(5, gn_trim_adc_result[gt_trim_search_mode][5], gn_trim_current_gain));
                    print(msg_buffer);

                    ++gn_apic_output_test_index;
                    if(gn_apic_output_test_index < gn_apic_output_test_max)
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                    }
                    else
                    {
                        print("\tPWM\r\n");
                        ++gn_apic_output_mode;
                        if(gn_apic_output_mode == 2)
                        {
                            gn_step_delay = 0;
                            gt_jig_trimming_step = TRIMMING_STEP_STOP;
                        }
                        else
                        {
                            gn_apic_output_test_index = 0;
                            gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                        }
                    }
                    break;
            }
            break;

        case TRIMMING_STEP_E2P_PROGRAM:   /* E2P program */
            if (gub_TRIM_ENABLE)
            {
                apic_program_e2p();
                apic_get_test_regs();
                dump_apic_regs(1);
                aqic_set_reg_otp2_writeen(OTP2_WRITE_DIS);
            }
            else
            {
                dump_apic_regs(1);
            }
            gn_step_delay = 5; /* 5ms */
            gt_jig_trimming_step = TRIMMING_STEP_STOP;
            break;

        case TRIMMING_STEP_STOP:
            if(gn_step_delay)
            {
                --gn_step_delay;
            }
            else
            {
                AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);

                /* VLED_CTR_7V : OFF */
                AQIC_VLED_EN(PWR_OFF);

                AQIC_Mode_Setting(MODE_DAC);
                AQIC_Apply_Voltage(0.0f, AQIC_MODE);

                AQIC_Mode_Duty(DUTY_ZERO);     /* clock off */
                change_i2c_setting(I2C_USED_GPIO);  /* change i2c to gpio */
                AQIC_Select_Output_Ch(AQIC_O_MAX);

                AQIC_VCC_EN(PWR_OFF); /* APIC power off */
                gt_jig_trimming_step = TRIMMING_STEP_RESULT;

                GUI_SEND_otp_written();
                HAL_Delay(1000);
                NVIC_SystemReset();
            }
            break;
        case TRIMMING_STEP_RESULT:
            if(gn_trim_out_of_spec_stop_trimming == 1)
            {
                print("======== TRIM ERROR ========\r\n");
                AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
                dump_apic_regs(1);

                set_ok_led(OFF);
                set_ng_led(ON);
            }
            else if(gn_trim_out_of_spec_stop_trimming == 2)
            {
                print("======== APIC I2C read error ========\r\n");

                set_ok_led(OFF);
                set_ng_led(ON);
            }
            else if(gn_trim_out_of_spec_stop_trimming == 3)
            {
                print("======== AQIC Already trimmed ========\r\n");

                set_ok_led(OFF);
                set_ng_led(ON);
            }
            else
            {
                print("======== TRIM END ========\r\n");
                set_ok_led(ON);
                set_ng_led(OFF);
            }
            gt_jig_trimming_step = TRIMMING_STEP_NONE;
            break;
        default:
            break;
    }
    }
}

uint8_t GUI_link_reg_with_gval(uint8_t in_AQIC_TYPE, uint8_t in_reg_addr, uint8_t *in_reg_val)
{
    if(in_AQIC_TYPE == I2C_AQIC_06)
    {
        if(REGISTER_USED_MAP_AQ06[in_reg_addr-I2C_SUB_START])
        {
            REGISTER_VALUE_MAP_AQIC[in_reg_addr-I2C_SUB_START] = in_reg_val;
        }
        else
        {
            print("=======NOT SUPPORTED REGISTER!======\r\n");
            return FALSE;
        }
    }
    return TRUE;
}

uint8_t GUI_SET_init_reg_tbl(uint8_t in_AQIC_TYPE)
{
    if(in_AQIC_TYPE == I2C_AQIC_06)
    {
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_DEVICEID, &gt_aqic_reg_deviceid);

        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_OTP1, &gt_aqic_reg_otp_b1.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_OTP2, &gt_aqic_reg_otp_b2.value);

        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TEST1, &gt_aqic_reg_test1.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TEST2, &gt_aqic_reg_test2.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TEST3, &gt_aqic_reg_test3.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TEST4, &gt_aqic_reg_test4.value);

        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_OTPCTL1, &gn_e2p_read[0]);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_OTPCTL2, &gn_e2p_read[1]);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_OTPCTL3, &gn_e2p_read[2]);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_OTPCTL4, &gn_e2p_read[3]);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_OTPCTL5, &gn_e2p_read[4]);

        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM1, &gt_aqic_reg_trim_d0.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM2, &gt_aqic_reg_trim_d1.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM3, &gt_aqic_reg_trim_d2.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM4, &gt_aqic_reg_trim_d3.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM5, &gt_aqic_reg_trim_d4.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM6, &gt_aqic_reg_trim_d5.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM7, &gt_aqic_reg_trim_d6.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM8, &gt_aqic_reg_trim_d7.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM9, &gt_aqic_reg_trim_d8.value);

        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM10, &gt_aqic_reg_trim_e0.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM11, &gt_aqic_reg_trim_e1.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM12, &gt_aqic_reg_trim_e2.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM13, &gt_aqic_reg_trim_e3.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM14, &gt_aqic_reg_trim_e4.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM15, &gt_aqic_reg_trim_e5.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM16, &gt_aqic_reg_trim_e6.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM17, &gt_aqic_reg_trim_e7.value);
        GUI_link_reg_with_gval(I2C_AQIC_06, I2C_SUB_TRIM18, &gt_aqic_reg_trim_e8.value);
    }
    return 0;
}

void GUI_SET_Activate()
{
    print("GUI ACTIVATED");
    gb_GUI_ACTIVATED = TRUE;
}

void GUI_SET_TrimStart(uint32_t register_apply_flg)
{
    snprintf(msg_buffer, sizeof msg_buffer, "GUI TRIM START:%d\r\n",register_apply_flg);
    print(msg_buffer);
    gb_GUI_REG_APPLY_FOR_TRIM = register_apply_flg;
    gt_jig_trimming_step = TRIMMING_STEP_READ_REGISTER;

    if(gub_DIN_MODE_ENABLE == TRUE)
    {
        gt_trim_search_mode = TRIM_SCREENING;
    }
}

void GUI_SET_RegisterReadAll()
{
    print("\r\nGUI_SET_RegisterReadAll\r\n");
    apic_get_test_regs();
}

void GUI_print_reg_all()
{
    for(int i = 0 ; i < I2C_SUB_END-I2C_SUB_START + 1 ; ++i)
    {
        if(REGISTER_USED_MAP_AQ06[i])
        {
            snprintf(msg_buffer, sizeof msg_buffer, "0x%2x : %3u\r\n", i + I2C_SUB_START, *REGISTER_VALUE_MAP_AQIC[i]);
            print(msg_buffer);
        }
    }
}

void GUI_SET_otp_regs(uint8_t in_AQIC_TYPE)
{
    if(in_AQIC_TYPE == I2C_AQIC_06)
    {
        print(CMD_JIG_OTP_LIST);

        for(int i = 0 ; i < sizeof(REGISTER_READONLY_MAP_AQ06) ; ++i)
        {
            snprintf(msg_buffer, sizeof msg_buffer, ":%2x",REGISTER_READONLY_MAP_AQ06[i]);
            print(msg_buffer);
        }
        print("\r\n");
    }
}

void GUI_SEND_reg_all()
{
    print(CMD_JIG_READ_REGISTER_ALL);
    for(int i = 0 ; i < (I2C_SUB_END - I2C_SUB_START + 1) ; ++i)
    {
        if(REGISTER_USED_MAP_AQ06[i])
        {
            snprintf(msg_buffer, sizeof msg_buffer, ":%2x:%02x",i+I2C_SUB_START, *REGISTER_VALUE_MAP_AQIC[i]);
            print(msg_buffer);
        }
        else
        {
            snprintf(msg_buffer, sizeof msg_buffer, ":%2x:NA",i+I2C_SUB_START);
            print(msg_buffer);
        }
    }
    print("\r\n");
}

void GUI_write_reg(uint8_t in_addr, uint8_t in_val)
{
    *REGISTER_VALUE_MAP_AQIC[in_addr - I2C_SUB_START] = in_val;
    snprintf(msg_buffer, sizeof msg_buffer, "addr:%x val:%x\r\n",in_addr,*REGISTER_VALUE_MAP_AQIC[in_addr-I2C_SUB_START]);
    print(msg_buffer);
    HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, in_addr, 1, REGISTER_VALUE_MAP_AQIC[in_addr-I2C_SUB_START], 1, 10);
}

void GUI_trim_enable(uint8_t in_val)
{
    gub_TRIM_ENABLE = in_val;
    gub_TRIM_ENABLE ? (print("\r\nTRIM_ENABLE is ON!!\r\n")) : (print("\r\nTRIM_ENABLE is off!!\r\n"));
}

void GUI_din_mode_enable(uint8_t in_val)
{
    gub_DIN_MODE_ENABLE = in_val;
    gub_DIN_MODE_ENABLE ? (print("\r\nDIN_MODE_ENABLE is ON!!\r\n")) : (print("\r\nDIN_MODE_ENABLE is off!!\r\n"));
}

void GUI_trim_VoltageTbl(float in_trim_volt[])
{
    gf_trim_voltage_table[0][0] = in_trim_volt[0]; // OFS_LIN_CH current
    gf_trim_voltage_table[0][1] = in_trim_volt[1]; // OFS_LIN_CH pwm

    gf_trim_voltage_table[1][0] = in_trim_volt[2]; // ICTL_LSB current
    gf_trim_voltage_table[1][1] = in_trim_volt[3]; // ICTL_LSB pwm

    gf_trim_voltage_table[2][0] = in_trim_volt[4]; // SCREENING current
    if(gub_DIN_MODE_ENABLE == TRUE)
    {
        gf_din_mode_end = in_trim_volt[5];
        gf_trim_voltage_table[2][1] = 0; // SCREENING pwm
    }
    else
    {
        gf_trim_voltage_table[2][1] = in_trim_volt[5]; // SCREENING pwm
    }

    snprintf(msg_buffer, sizeof msg_buffer, "gf_trim_voltage_table[0][0] :  %f\r\n", gf_trim_voltage_table[0][0]);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "gf_trim_voltage_table[0][1] :  %f\r\n", gf_trim_voltage_table[0][1]);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "gf_trim_voltage_table[1][0] :  %f\r\n", gf_trim_voltage_table[1][0]);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "gf_trim_voltage_table[1][1] :  %f\r\n", gf_trim_voltage_table[1][1]);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "gf_trim_voltage_table[2][0] :  %f\r\n", gf_trim_voltage_table[2][0]);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "gf_trim_voltage_table[2][1] :  %f\r\n", gf_trim_voltage_table[2][1]);
    print(msg_buffer);
}



void GUI_trim_RangeTbl(uint32_t in_trim_rang[])
{
    gt_adjust_trim_range[0].min = in_trim_rang[0]; // OFS_LIN_CH min
    gt_adjust_trim_range[0].max = in_trim_rang[1]; // OFS_LIN_CH max

    gt_adjust_trim_range[1].min = in_trim_rang[2]; // ICTL_LSB min
    gt_adjust_trim_range[1].max = in_trim_rang[3]; // ICTL_LSB max

    snprintf(msg_buffer, sizeof msg_buffer, "gt_adjust_trim_range[0].min :  %d\r\n", gt_adjust_trim_range[0].min);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "gt_adjust_trim_range[0].max :  %d\r\n", gt_adjust_trim_range[0].max);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "gt_adjust_trim_range[1].min :  %d\r\n", gt_adjust_trim_range[1].min);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "gt_adjust_trim_range[1].max :  %d\r\n", gt_adjust_trim_range[1].max);
    print(msg_buffer);
}

void GUI_CZmax_CZn_Set(uint16_t Czmax, uint16_t Czn, uint16_t ScreenCzn)
{
    snprintf(msg_buffer, sizeof msg_buffer, "\r\nGUI_CZmax_CZn_Set(%d,%d,%d)\r\n", Czmax, Czn, ScreenCzn);
    print(msg_buffer);

    switch(Czmax)
    {
    case 1:
        gb_gui_czmax1 = 0;
        gb_gui_czmax2 = 0;
        break;
    case 2:
        gb_gui_czmax1 = 1;
        gb_gui_czmax2 = 0;
        break;
    case 3:
        gb_gui_czmax1 = 0;
        gb_gui_czmax2 = 1;
        break;
    case 4:
        gb_gui_czmax1 = 1;
        gb_gui_czmax2 = 1;
        break;
    default:
        snprintf(msg_buffer, sizeof msg_buffer, "\r\nERROR: GUI_CZmax_CZn_Set(%d,%d,%d) WRONG INPUT-CZmax\r\n", Czmax, Czn,ScreenCzn);
        print(msg_buffer);
        break;
    }

    switch(Czn)
    {
    case 1:
        gb_gui_czn = 1;
        break;
    case 2:
        gb_gui_czn = 2;
        break;
        break;
    case 3:
        gb_gui_czn = 4;
        break;
        break;
    case 4:
        gb_gui_czn = 8;
        break;
        break;
    default:
        snprintf(msg_buffer, sizeof msg_buffer, "\r\nERROR: GUI_CZmax_CZn_Set(%d,%d,%d) WRONG INPUT-CZmax\r\n", Czmax, Czn,ScreenCzn);
        print(msg_buffer);
        break;
    }

    switch(ScreenCzn)
    {
    case 1:
        gb_gui_Screenczn = 1;
        break;
    case 2:
        gb_gui_Screenczn = 2;
        break;
    case 3:
        gb_gui_Screenczn = 4;
        break;
    case 4:
        gb_gui_Screenczn = 8;
        break;
    default:
        snprintf(msg_buffer, sizeof msg_buffer, "\r\nERROR: GUI_CZmax_CZn_Set(%d,%d,%d) WRONG INPUT-CZmax\r\n", Czmax, Czn,ScreenCzn);
        print(msg_buffer);
        break;
    }
}

void GUI_GAIN_LEVEL(uint16_t ch_a_level)
{
    snprintf(msg_buffer, sizeof msg_buffer, "\r\nGUI_GAIN_LEVEL(%d)\r\n", ch_a_level);
    print(msg_buffer);

    gn_trim_gain_level_ch_a = ch_a_level;
}

#define I2C_SUB_OTP_START 0xd1
#define I2C_SUB_OTP_END 0xe8

void GUI_SEND_otp_written()
{
    print(CMD_JIG_OTP_WRITTEN);
    for(int i = 0 ; i < (I2C_SUB_OTP_END - I2C_SUB_OTP_START + 1) ; ++i)
    {
        if(REGISTER_USED_MAP_AQ06[i + (I2C_SUB_OTP_START - I2C_SUB_START)])
        {
            snprintf(msg_buffer, sizeof msg_buffer, ":%2x:%02x",i+I2C_SUB_OTP_START, *REGISTER_VALUE_MAP_AQIC[i+(I2C_SUB_OTP_START-I2C_SUB_START)]);
            print(msg_buffer);
        }
        else{
            snprintf(msg_buffer, sizeof msg_buffer, ":%2x:NA",i+I2C_SUB_OTP_START);
            print(msg_buffer);
        }
    }
    print("\r\n");
}

uint8_t check_valid_ictl_step(uint8_t in_ictl_step, uint8_t in_channel_num, uint8_t in_adj_type)
{
	uint8_t ret = TRUE;
    uint16_t *temp_aqic_reg_trim_ictl_ch1_8bit;

	switch (in_channel_num)
	{
    case AQIC_O1 :
        temp_aqic_reg_trim_ictl_ch1_8bit = &gt_aqic_reg_trim_ictl_ch1_8bit;
        break;
    case AQIC_O2 :
        temp_aqic_reg_trim_ictl_ch1_8bit = &gt_aqic_reg_trim_ictl_ch2_8bit;
        break;
    case AQIC_O3 :
        temp_aqic_reg_trim_ictl_ch1_8bit = &gt_aqic_reg_trim_ictl_ch3_8bit;
        break;
    case AQIC_O4 :
        temp_aqic_reg_trim_ictl_ch1_8bit = &gt_aqic_reg_trim_ictl_ch4_8bit;
        break;
    case AQIC_O5 :
        temp_aqic_reg_trim_ictl_ch1_8bit = &gt_aqic_reg_trim_ictl_ch5_8bit;
        break;
    case AQIC_O6 :
        temp_aqic_reg_trim_ictl_ch1_8bit = &gt_aqic_reg_trim_ictl_ch6_8bit;
        break;
	}

    if(in_adj_type == ADJ_PLUS)
    {
        if( (*temp_aqic_reg_trim_ictl_ch1_8bit + in_ictl_step) > 255 )
        {
            ret = FALSE;
        }
    }

    if(in_adj_type == ADJ_MINUS)
    {
        if( (*temp_aqic_reg_trim_ictl_ch1_8bit - in_ictl_step) < 1 )
        {
            ret = FALSE;
        }
    }

	return ret;
}

uint8_t check_valid_ofs_step(uint8_t in_ofs_step, uint8_t in_channel_num, uint8_t in_adj_type)
{
	uint8_t ret = TRUE;
    uint16_t *temp_aqic_reg_trim_ofs_lin_11bit;

	switch (in_channel_num)
	{
    case AQIC_O1 :
        temp_aqic_reg_trim_ofs_lin_11bit = &gt_aqic_reg_trim_ofs_lin_ch1_11bit;
        break;
    case AQIC_O2 :
        temp_aqic_reg_trim_ofs_lin_11bit = &gt_aqic_reg_trim_ofs_lin_ch2_11bit;
        break;
    case AQIC_O3 :
        temp_aqic_reg_trim_ofs_lin_11bit = &gt_aqic_reg_trim_ofs_lin_ch3_11bit;
        break;
    case AQIC_O4 :
        temp_aqic_reg_trim_ofs_lin_11bit = &gt_aqic_reg_trim_ofs_lin_ch4_11bit;
        break;
    case AQIC_O5 :
        temp_aqic_reg_trim_ofs_lin_11bit = &gt_aqic_reg_trim_ofs_lin_ch5_11bit;
        break;
    case AQIC_O6 :
        temp_aqic_reg_trim_ofs_lin_11bit = &gt_aqic_reg_trim_ofs_lin_ch6_11bit;
        break;
	}

    if(in_adj_type == ADJ_PLUS)
    {
        if( (*temp_aqic_reg_trim_ofs_lin_11bit + in_ofs_step) > 2046 )
        {
            ret = FALSE;
        }
    }

    if(in_adj_type == ADJ_MINUS)
    {
        if( (*temp_aqic_reg_trim_ofs_lin_11bit - in_ofs_step) < 1 )
        {
            ret = FALSE;
        }
    }

	return ret;
}

uint8_t Get_ictl_regi_value(uint8_t channel)
{
    uint8_t rtn_val = 0;

    switch (channel)
    {
    case AQIC_O1:
        rtn_val = ((gt_aqic_reg_trim_e2.u.ictl_ch1 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch1 & 0x01);
        break;
    case AQIC_O2:
        rtn_val = ((gt_aqic_reg_trim_e3.u.ictl_ch2 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch2 & 0x01);
        break;
    case AQIC_O3:
        rtn_val = ((gt_aqic_reg_trim_e4.u.ictl_ch3 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch3 & 0x01);
        break;
    case AQIC_O4:
        rtn_val = ((gt_aqic_reg_trim_e5.u.ictl_ch4 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch4 & 0x01);
        break;
    case AQIC_O5:
        rtn_val = ((gt_aqic_reg_trim_e6.u.ictl_ch5 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch5 & 0x01);
        break;
    case AQIC_O6:
        rtn_val = ((gt_aqic_reg_trim_e7.u.ictl_ch6 & 0xFE) << 1) + (gt_aqic_reg_trim_e8.u.ictl_ch6 & 0x01);
        break;
    }
    // snprintf(msg_buffer, sizeof msg_buffer, "Get_ictl_regi_value(%d)-%d\r\n",channel, rtn_val);
    // print(msg_buffer);

    return rtn_val;
}

uint16_t Get_ofs_regi_value(uint8_t channel)
{
    uint16_t rtn_val = 0;
    switch(channel)
    {
    case AQIC_O1:
        rtn_val = gt_aqic_reg_trim_d2.u.ofs_lin_ch1 + (gt_aqic_reg_trim_d8.u.ofs_lin_ch1 << 8);
        break;
    case AQIC_O2:
        rtn_val = gt_aqic_reg_trim_d3.u.ofs_lin_ch2 + (gt_aqic_reg_trim_d8.u.ofs_lin_ch2 << 8);
        break;
    case AQIC_O3:
        rtn_val = gt_aqic_reg_trim_d4.u.ofs_lin_ch3 + (gt_aqic_reg_trim_e0.u.ofs_lin_ch3 << 8);
        break;
    case AQIC_O4:
        rtn_val = gt_aqic_reg_trim_d5.u.ofs_lin_ch4 + (gt_aqic_reg_trim_e0.u.ofs_lin_ch4 << 8);
        break;
    case AQIC_O5:
        rtn_val = gt_aqic_reg_trim_d6.u.ofs_lin_ch5 + (gt_aqic_reg_trim_e1.u.ofs_lin_ch5 << 8);
        break;
    case AQIC_O6:
        rtn_val = gt_aqic_reg_trim_d7.u.ofs_lin_ch6 + (gt_aqic_reg_trim_e1.u.ofs_lin_ch6 << 8);
        break;
    }

    return rtn_val;
}

void Set_ictl_regi_value(uint8_t channel, uint8_t input_regi_val)
{
    switch (channel)
    {
    case AQIC_O1:
        gt_aqic_reg_trim_e2.u.ictl_ch1 = (input_regi_val & 0xFE) >> 1;
        gt_aqic_reg_trim_e8.u.ictl_ch1 = (input_regi_val & 0x01);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM12, 1, &(gt_aqic_reg_trim_e2.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM18, 1, &(gt_aqic_reg_trim_e8.value), 1, 10);
        break;
    case AQIC_O2:
        gt_aqic_reg_trim_e3.u.ictl_ch2 = (input_regi_val & 0xFE) >> 1;
        gt_aqic_reg_trim_e8.u.ictl_ch2 = (input_regi_val & 0x01);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM13, 1, &(gt_aqic_reg_trim_e3.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM18, 1, &(gt_aqic_reg_trim_e8.value), 1, 10);
        break;
    case AQIC_O3:
        gt_aqic_reg_trim_e4.u.ictl_ch3 = (input_regi_val & 0xFE) >> 1;
        gt_aqic_reg_trim_e8.u.ictl_ch3 = (input_regi_val & 0x01);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM14, 1, &(gt_aqic_reg_trim_e4.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM18, 1, &(gt_aqic_reg_trim_e8.value), 1, 10);
        break;
    case AQIC_O4:
        gt_aqic_reg_trim_e5.u.ictl_ch4 = (input_regi_val & 0xFE) >> 1;
        gt_aqic_reg_trim_e8.u.ictl_ch4 = (input_regi_val & 0x01);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM15, 1, &(gt_aqic_reg_trim_e5.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM18, 1, &(gt_aqic_reg_trim_e8.value), 1, 10);
        break;
    case AQIC_O5:
        gt_aqic_reg_trim_e6.u.ictl_ch5 = (input_regi_val & 0xFE) >> 1;
        gt_aqic_reg_trim_e8.u.ictl_ch5 = (input_regi_val & 0x01);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM16, 1, &(gt_aqic_reg_trim_e6.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM18, 1, &(gt_aqic_reg_trim_e8.value), 1, 10);
        break;
    case AQIC_O6:
        gt_aqic_reg_trim_e7.u.ictl_ch6 = (input_regi_val & 0xFE) >> 1;
        gt_aqic_reg_trim_e8.u.ictl_ch6 = (input_regi_val & 0x01);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM17, 1, &(gt_aqic_reg_trim_e7.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM18, 1, &(gt_aqic_reg_trim_e8.value), 1, 10);
        break;
    }
}

void Set_ofs_regi_value(uint8_t channel, uint16_t input_regi_val)
{
    switch(channel)
    {
    case AQIC_O1:
        gt_aqic_reg_trim_ofs_lin_ch1_11bit = input_regi_val;
        gt_aqic_reg_trim_d2.value = (gt_aqic_reg_trim_ofs_lin_ch1_11bit & 0xFF);
        gt_aqic_reg_trim_d8.u.ofs_lin_ch1 = ((gt_aqic_reg_trim_ofs_lin_ch1_11bit >> 8) & 0x7);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM3, 1, &(gt_aqic_reg_trim_d2.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM9, 1, &(gt_aqic_reg_trim_d8.value), 1, 10);
        break;
    case AQIC_O2:
        gt_aqic_reg_trim_ofs_lin_ch2_11bit = input_regi_val;
        gt_aqic_reg_trim_d3.value = (gt_aqic_reg_trim_ofs_lin_ch2_11bit & 0xFF);
        gt_aqic_reg_trim_d8.u.ofs_lin_ch2 = ((gt_aqic_reg_trim_ofs_lin_ch2_11bit >> 8) & 0x7);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM4, 1, &(gt_aqic_reg_trim_d3.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM9, 1, &(gt_aqic_reg_trim_d8.value), 1, 10);
        break;
    case AQIC_O3:
        gt_aqic_reg_trim_ofs_lin_ch3_11bit = input_regi_val;
        gt_aqic_reg_trim_d4.value = (gt_aqic_reg_trim_ofs_lin_ch3_11bit & 0xFF);
        gt_aqic_reg_trim_e0.u.ofs_lin_ch3 = ((gt_aqic_reg_trim_ofs_lin_ch3_11bit >> 8) & 0x7);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM5, 1, &(gt_aqic_reg_trim_d4.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM10, 1, &(gt_aqic_reg_trim_e0.value), 1, 10);
        break;
    case AQIC_O4:
        gt_aqic_reg_trim_ofs_lin_ch4_11bit = input_regi_val;
        gt_aqic_reg_trim_d5.value = (gt_aqic_reg_trim_ofs_lin_ch4_11bit & 0xFF);
        gt_aqic_reg_trim_e0.u.ofs_lin_ch4 = ((gt_aqic_reg_trim_ofs_lin_ch4_11bit >> 8) & 0x7);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM6, 1, &(gt_aqic_reg_trim_d5.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM10, 1, &(gt_aqic_reg_trim_e0.value), 1, 10);
        break;
    case AQIC_O5:
        gt_aqic_reg_trim_ofs_lin_ch5_11bit = input_regi_val;
        gt_aqic_reg_trim_d6.value = (gt_aqic_reg_trim_ofs_lin_ch5_11bit & 0xFF);
        gt_aqic_reg_trim_e1.u.ofs_lin_ch5 = ((gt_aqic_reg_trim_ofs_lin_ch5_11bit >> 8) & 0x7);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM7, 1, &(gt_aqic_reg_trim_d6.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM11, 1, &(gt_aqic_reg_trim_e1.value), 1, 10);
        break;
    case AQIC_O6:
        gt_aqic_reg_trim_ofs_lin_ch6_11bit = input_regi_val;
        gt_aqic_reg_trim_d7.value = (gt_aqic_reg_trim_ofs_lin_ch6_11bit & 0xFF);
        gt_aqic_reg_trim_e1.u.ofs_lin_ch6 = ((gt_aqic_reg_trim_ofs_lin_ch6_11bit >> 8) & 0x7);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM8, 1, &(gt_aqic_reg_trim_d7.value), 1, 10);
        HAL_I2C_Mem_Write(&hi2c1, I2C_APIC_ID, I2C_SUB_TRIM11, 1, &(gt_aqic_reg_trim_e1.value), 1, 10);
        break;
    }
    return;
}

/*** end of file ***/


