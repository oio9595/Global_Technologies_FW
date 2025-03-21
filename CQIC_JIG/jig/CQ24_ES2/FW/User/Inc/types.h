/** @file types.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) GT. All rights reserved.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#define TRUE    1
#define FALSE   0

typedef unsigned char       BOOL;
typedef unsigned char       U8;
typedef char                S8;
typedef unsigned short      U16;
typedef short               S16;
typedef unsigned long       U32;
typedef long                S32;
typedef unsigned long long  U64;
typedef long long           S64;
typedef void*               HANDLE;
typedef float               FLOAT;
typedef double              DOUBLE;

typedef struct _bits
{
    unsigned b0	: 1;
    unsigned b1	: 1;
    unsigned b2	: 1;
    unsigned b3	: 1;
    unsigned b4	: 1;
    unsigned b5	: 1;
    unsigned b6	: 1;
    unsigned b7	: 1;
}BITS;

typedef union
{
    BITS bits_t;
    U8 value;
}BYTE;

#ifndef NULL
#define NULL	((void*)0)
#endif

typedef enum
{
#if 0
    KEY_NONE = 0,
    KEY_POWER_UP,
    KEY_AUTO_MANUAL_UP,
    KEY_MANUAL_STEP_UP,
    KEY_TRIMMING_UP,
    KEY_B1_UP,
    KEY_MAX,
#else
    KEY_NONE = 0,
    KEY_FUNCTION_AUTO_UP,
    KEY_FUNCTION_MANUAL_UP,
    KEY_TRIMMING_AUTO_UP,
    KEY_TRIMMING_MANUAL_UP,
    KEY_MODE_UP,
    KEY_RUN_UP,
    KEY_B1_UP,
    KEY_MAX,
    KEY_AUTO_START_UP,
    KEY_MANUAL_STEP_UP    
#endif
}key_event_t;

typedef enum
{
    PWR_OFF = 0,
    PWR_ON,
}power_state_t;

typedef enum
{
    OFF = 0,
    ON,
    GEN_PWM,
}onoff_state_t;

typedef enum
{
    TEST_AUTO = 0,
    TEST_MANUAL,
    TEST_MAX,
}auto_manual_t;

typedef enum
{
    AMIC_O1 = 0,
    AMIC_O2,
    AMIC_O3,
    AMIC_O4,
    AMIC_O5,
    AMIC_O6,
    AMIC_O_MAX,
}amic_out_t;

typedef enum
{
    MODE_BF = 0,
    MODE_DAC,
}aqic_mode_t;

typedef enum
{
    AQIC_D1 = 0,
    AQIC_D2,
    AQIC_D3,
    AQIC_D_ALL,
    AQIC_MODE,
}dac_sel_t;

#if 0
typedef enum
{
    CQIC_DAC_1_5V = 0,
    CQIC_DAC_4_5V = 0,
    CQIC_MODE_2,
    CQIC_MODE_1,
    CQIC_AOUT_MAX
}amic_etc_out_t;
#else
typedef enum
{
    CQIC_MODE_2  = 0,
    CQIC_DAC_1_5V = 1,
    CQIC_DAC_4_5V = 1,
    CQIC_MODE_1 = 2,
    CQIC_AOUT_MAX
}amic_etc_out_t;
#endif

typedef enum
{
    SPI_FOR_ADC = 0,
    SPI_FOR_DAC,
}spi_setting_t;

typedef enum
{
    I2C_USED_I2C = 0,
    I2C_USED_GPIO,
}i2c_setting_t;

typedef enum
{
	GAIN_LOW = 0, 
	GAIN_MID, 
	GAIN_HIGH, 
}current_gain_t;

typedef enum
{
	DISCHARGE = 0, 
	CHARGE,  
}current_charge_t;


typedef enum
{
	one_by_six = 0, 
	two_by_three, 
	three_by_two, 
	test_mode,  
}aqic_set_mode_t;

typedef enum
{
    TEST_MODE_NONE = 0,
		
    TEST_MODE_1_BY_6_CZ_1,
    TEST_MODE_1_BY_6_CZ_2,
    TEST_MODE_1_BY_6_CZ_3,
    TEST_MODE_1_BY_6_CZ_4,
    
    TEST_MODE_2_BY_3_CZ_1,
    TEST_MODE_2_BY_3_CZ_2,
    TEST_MODE_2_BY_3_CZ_3,
    TEST_MODE_2_BY_3_CZ_4,
    
    TEST_MODE_3_BY_2_CZ_1,
    TEST_MODE_3_BY_2_CZ_2,
    TEST_MODE_3_BY_2_CZ_3,
    TEST_MODE_3_BY_2_CZ_4,
    
    TEST_MODE_RESULT,
    TEST_MODE_MAX,
}test_mode_t;

typedef enum _TEST_STEP_
{
    TEST_STEP_INIT = 0,
    TEST_STEP_INIT_DELAY,

    TEST_STEP_ADS114S08_INPUT_SELECT,
    TEST_STEP_ADS114S08_CONV_START,
    TEST_STEP_ADS114S08_ADC_READ_CHK,

    TEST_STEP_READ_ADS114S08,
    TEST_STEP_READ_ADC,

    TEST_STEP_VIN_CURRENT_CHANGE,
    TEST_STEP_CURRENT_HIGH,
    TEST_STEP_CURRENT_LOW,
    TEST_STEP_VIN_PWM_CHANGE,
    TEST_STEP_PWM_HIGH,
    TEST_STEP_PWM_LOW,
    TEST_STEP_CURRENT_DISCHARGE, 
    
    TEST_STEP_END,
    TEST_STEP_BEEP_DELAY,
    TEST_STEP_NEXT,
}test_step_t;


typedef enum
{
	TEST_STEP_NONE = 0,
	TRIMMING_STEP00_00,
	TRIMMING_STEP00_01,
	TRIMMING_STEP00_02,
	TRIMMING_STEP00_03,
	TRIMMING_STEP01_00,
	TRIMMING_STEP01_01,
	TRIMMING_STEP01_02,
	TRIMMING_STEP01_03,

	TRIMMING_STEP01_10,
	TRIMMING_STEP01_20,
	TRIMMING_STEP01_30,
	TRIMMING_STEP01_40,
	TRIMMING_STEP01_50,

	TRIMMING_STEP02_00,
	TRIMMING_STEP02_01,
	TRIMMING_STEP02_10,
	TRIMMING_STEP02_11,
	TRIMMING_STEP02_02,
	TRIMMING_STEP02_03,

	TRIMMING_STEP12_00,
	TRIMMING_STEP12_01,
	TRIMMING_STEP12_10,
	TRIMMING_STEP12_11,
	TRIMMING_STEP12_02,
	TRIMMING_STEP12_03,
	
	TRIMMING_STEP03_00,
	TRIMMING_STEP03_01,
	TRIMMING_STEP03_11,
	TRIMMING_STEP03_02,
	TRIMMING_STEP03_03,
	TRIMMING_STEP03_04,

	TRIMMING_STEP04_00,
	TRIMMING_STEP04_01,
	TRIMMING_STEP04_11,
	TRIMMING_STEP04_02,
	TRIMMING_STEP04_03,
	TRIMMING_STEP04_04,

	TRIMMING_STEP05_00,
	TRIMMING_STEP05_01,
	TRIMMING_STEP05_02,
	TRIMMING_STEP05_03,
	TRIMMING_STEP05_04,
	
	TRIMMING_STEP_OTP_WRITE,
	TRIMMING_STEP_STOP,
	TRIMMING_STEP_ERROR,
	TRIMMING_STEP_OK,
	TRIMMING_STEP_RESULT,	
	TRIMMING_STEP_MAX,

	MODE_TEST_START,
 	MODE_TEST_STEP00_00,
	MODE_TEST_STEP00_01,
	MODE_TEST_STEP00_02,
	MODE_TEST_STEP00_03,
	MODE_TEST_STEP01_00,
	MODE_TEST_STEP01_01,
	MODE_TEST_STEP01_02,
	MODE_TEST_STEP01_03,
	MODE_TEST_STEP02_00,
	MODE_TEST_STEP02_01,
	MODE_TEST_STEP02_02,
	MODE_TEST_STEP02_03,
	MODE_TEST_STEP03_00,
	MODE_TEST_STEP03_01,
	MODE_TEST_STEP03_02,
	MODE_TEST_STEP03_03,
	MODE_TEST_RESULT,
	MODE_TEST_OK,
	MODE_TEST_ERROR
 }cqic_test_step_t;


typedef enum
{
    BEEP_START = 0,
    BEEP_PLAYING,
    BEEP_STOP,
    BEEP_NG_START,
    BEEP_NG_PLAYING,
    BEEP_NG_STOP,
}beep_play_t;

typedef enum
{
    BEEP_DO_C4 = 0,
    BEEP_RE_D4,
    BEEP_MI_E4,
    BEEP_FA_F4,
    BEEP_SO_G4,
    BEEP_LA_A4,
    BEEP_SI_B4,
    BEEP_DO_C5,
    BEEP_MAX,
}beep_freq_t;

#define FUNCTION_TEST_REPEAT_CNT    (10)
#define ADC_READ_COUNT          	(30) /* 100 - 700ms */
#define CONFIRM_COUNT				(10)
#define RETRY_COUNT					(100)

enum notes_freq {
  C4 = 2616256U, // ?��(261.6256Hz)
  D4 = 2936648U, // ?��(293.6648Hz)
  E4 = 3296276U, // �?(329.6276Hz)
  F4 = 3492282U, // ?��(349.2282Hz)
  G4 = 3919954U, // ?��(391.9954Hz)
  A4 = 4400000U, // ?��(440.0000Hz)
  B4 = 4938833U, // ?��(493.8833Hz)
  C5 = 5232511U  // ?��(523.2511Hz)
};

 /* for BUZZER PWM - TIM12 */
#define APB1_TIMER_CLOCK    (90000000UL)
#define TIM12_PRESCALER     (90-1)
enum tim12_period {
    tim12_c4_arr =(uint32_t)((double)APB1_TIMER_CLOCK/((C4/10000.0)*TIM12_PRESCALER)+0.5),   //3866,
    tim12_d4_arr =(uint32_t)((double)APB1_TIMER_CLOCK/((D4/10000.0)*TIM12_PRESCALER)+0.5),   //3443,
    tim12_e4_arr =(uint32_t)((double)APB1_TIMER_CLOCK/((E4/10000.0)*TIM12_PRESCALER)+0.5),   //3067,
    tim12_f4_arr =(uint32_t)((double)APB1_TIMER_CLOCK/((F4/10000.0)*TIM12_PRESCALER)+0.5),   //2895,
    tim12_g4_arr =(uint32_t)((double)APB1_TIMER_CLOCK/((G4/10000.0)*TIM12_PRESCALER)+0.5),   //2579,
    tim12_a4_arr =(uint32_t)((double)APB1_TIMER_CLOCK/((A4/10000.0)*TIM12_PRESCALER)+0.5),   //2298,
    tim12_b4_arr =(uint32_t)((double)APB1_TIMER_CLOCK/((B4/10000.0)*TIM12_PRESCALER)+0.5),   //2049,
    tim12_c5_arr =(uint32_t)((double)APB1_TIMER_CLOCK/((C5/10000.0)*TIM12_PRESCALER)+0.5),   //1932,
};


#ifdef USE_TESTKEY_B1
typedef enum {
    TEST_FUNCTION_MODE0 = 0,
    TEST_FUNCTION_MODE1,
    TEST_FUNCTION_MODE2,
    TEST_FUNCTION_MODE3,
    TEST_FUNCTION_MODE4,
    TEST_FUNCTION_MODE5,
    TEST_FUNCTION_MODE6,
    TEST_FUNCTION_MODE7,
    TEST_FUNCTION_MODE8,
    TEST_FUNCTION_MODE9,
    TEST_FUNCTION_MODE10,
    TEST_FUNCTION_MODE11,
    TEST_FUNCTION_MODE12,
    TEST_FUNCTION_MODE13,
    TEST_FUNCTION_MODE14,
    TEST_FUNCTION_MAX,
}test_function_t;

#endif
#define	DAC_I2C_ADDRESS    0xC0

#define KEY_BEEP_TIME           75
#define RESULT_BEEP_TIME        498
#define NG_BEEP_TIME            (RESULT_BEEP_TIME/3)

#define KEY_PUSH_RELEASE_BEEP   BEEP_DO_C4
#define KEY_TEST_NG_BEEP        BEEP_MI_E4
#define KEY_TEST_OK_BEEP        BEEP_DO_C5


#define CURRENT_SENSE_R       (50) /* ohm */
#define CURRENT_SENSE_RIN     (2000) /* ohm */
#define CURRENT_SENSE_C_HIGH  (150) /* nF */
#define CURRENT_SENSE_C_MID   (27) /* nF */
#define CURRENT_SENSE_C_LOW   (3.3) /* nF */
#define SAMPLE_TIME           (0.5) /* ms */

#define OTP2_WRITE_EN 0xA5
#define OTP2_WRITE_DIS 0x5A

typedef enum
{
    DEBUG_MODE_NONE = 0,
    DEBUG_OFFSET,
}debug_mode_t;

typedef enum
{
    ADC_TOLER_5 = 0,
    ADC_TOLER_7,
    ADC_TOLER_MAX
}adc_tolerance_t;

typedef enum
{
    MSG_NONE = 0,
    MSG_DETAIL,
    MSG_CVS,
    MSG_MAX
}output_msg_type_t;

typedef struct _min_max_
{
    uint16_t min;
    uint16_t max;
}min_max_t;






#define BUILD_YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
              + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))
 
#define BUILD_MONTH (__DATE__ [2] == 'n' ? 0 \
               : __DATE__ [2] == 'b' ? 1 \
               : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
               : __DATE__ [2] == 'y' ? 4 \
               : __DATE__ [2] == 'n' ? 5 \
               : __DATE__ [2] == 'l' ? 6 \
               : __DATE__ [2] == 'g' ? 7 \
               : __DATE__ [2] == 'p' ? 8 \
               : __DATE__ [2] == 't' ? 9 \
               : __DATE__ [2] == 'v' ? 10 : 11)
 
#define BUILD_DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))

#define VERSION_MAJOR       0x0000
#define VERSION_MINOR       0x0001
#define VERSION_REVISION    0x0001


#define CMD_JIG_READ_REGISTER_ALL       "jig:rreg:"
#define CMD_GUI_WRITE_REGISTER_CMD1     "ui:1wreg"
#define CMD_GUI_WRITE_REGISTER_CMD2     "ui:2wreg"
#define CMD_GUI_KEY_POWER_UP            "ui:kpup"
#define CMD_GUI_KEY_TRIMMING_UP         "ui:ktup"
#define CMD_GUI_KEY_MANUAL_STEP_UP      "ui:kmup"
#define CMD_GUI_ACTIVATE                "ui:gact"
#define CMD_GUI_TRIMSTART               "ui:gtrim"
#define CMD_GUI_READ_REGISTER_ALL       "ui:grreg"
#define CMD_GUI_WRITE_REGISTER          "ui:gwreg"
#define CMD_GUI_TRIM_ENDABLE            "ui:gtren"
#define CMD_GUI_TRIMSVOLT               "ui:gvolt"
#define CMD_GUI_TRIMSRANGE              "ui:grang"
#define CMD_JIG_OTP_LIST                "jig:otp"
#define CMD_GUI_CZMCZN                  "ui:czmn"
#define CMD_JIG_OTP_WRITTEN             "jig:wotp"
#define CMD_CQIC_OTP_WRITTEN            "cqic:wotp"
#define CMD_GUI_RESET                   "ui:reset"
#define CMD_GUI_NO_TRIM_MODE            "ui:din"
#define CMD_GUI_SLOPERANGE_CQIC         "ui:cslop"

#define CMD_CQIC_CMD1_READ_REGISTER_ALL "cqic:rreg1"
#define CMD_CQIC_CMD2_READ_REGISTER_ALL "cqic:rreg2"
#define CMD_GUI_TRIMSRANGE_CQIC1        "ui:ctrim1"
#define CMD_GUI_TRIMSRANGE_CQIC2        "ui:ctrim2"
#define CMD_GUI_TRIMSRANGE_CQIC3        "ui:ctrim3"



#endif /* ~__TYPES_H__ */

/*** end of file ***/
