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
    KEY_NONE = 0,
    KEY_POWER_UP,
    KEY_AUTO_MANUAL_UP,
    KEY_MANUAL_STEP_UP,
    KEY_TRIMMING_UP,
    KEY_B1_UP,
    KEY_MAX,
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
    AQIC_O1 = 0,
    AQIC_O2,
    AQIC_O3,
    AQIC_O4,
    AQIC_O5,
    AQIC_O6,
    AQIC_O_MAX,
}AQ06_out_t;

typedef enum
{
    MODE_PWM = 0,
    MODE_DAC,
}aqic_mode_t;

typedef enum
{
    DUTY_ZERO = 0,
    DUTY_HALF,
    DUTY_FULL,
}aqic_mode_duty_t;

typedef enum
{
    AQIC_D1 = 0,
    AQIC_D2,
    AQIC_D3,
    AQIC_MODE,
    AQIC_D_ALL,
}dac_sel_t;

typedef enum
{
    AMIC_ETC_FB = 0,
    AMIC_ETC_MON,
    AMIC_ETC_MAX,
}amic_etc_out_t;

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
	AQIC_MODE_1 = 0,    // 1 * 6
	AQIC_MODE_2,        // 2 * 3
	AQIC_MODE_3,        // 3 * 2
	AQIC_MODE_4,        // trimming mode
    AQIC_MODE_MAX,
}aqic_set_mode_t;

typedef enum
{
    TEST_MODE_NONE = 0,
    TEST_MODE_INIT,
    TEST_MODE_MEASURE,
    TEST_MODE_RESULT,
    TEST_MODE_MAX,
}test_mode_t;

typedef enum _TEST_STEP_
{
    TEST_STEP_NONE = 0,
    TEST_STEP_INIT,
    TEST_STEP_INIT_DELAY,

    TEST_STEP_ADS114S08_CONV_START,
    TEST_STEP_ADS114S08_ADC_READ_CHK,

    TEST_STEP_READ_ADS114S08,

    TEST_STEP_VIN_CURRENT_CHANGE,
    TEST_STEP_VIN_CURRENT_SET,
    TEST_STEP_STROBE,
    TEST_STEP_VIN_PWM_CHANGE,
    TEST_STEP_PWM_HIGH,
    TEST_STEP_PWM_LOW,
    TEST_STEP_CURRENT_DISCHARGE,
    TEST_STEP_LOG,

    TEST_STEP_END,
    TEST_STEP_NEXT,
}test_step_t;

typedef enum
{
    TRIMMING_STEP_NONE = 0,
    TRIMMING_STEP_INIT,     /* ready for reading registers via i2c  */
    TRIMMING_STEP_APIC_PWR,
    TRIMMING_STEP_READ_REGISTER,     /* read all register via i2c, from D0 to D7  */
    TRIMMING_STEP_OSCEN,
    TRIMMING_STEP_CLK_SEL,
    TRIMMING_STEP_EXT_CLK_OFF,
    TRIMMING_STEP_CHANGE_OUTPUT_INIT,
    TRIMMING_STEP_CHANGE_OUTPUT,
    TRIMMING_STEP_CHANGE_REGISTER,
    TRIMMING_STEP_SET_ADC_CH,
    TRIMMING_STEP_START_ADC_CONVERSION,
    TRIMMING_STEP_GET_ADC_CH,
    TRIMMING_STEP_CHECK,
    TRIMMING_STEP_SCREENING,
    TRIMMING_STEP_E2P_PROGRAM,     /* write all trim register to OTP, from D0 to D7  */
    TRIMMING_STEP_STOP,
    TRIMMING_STEP_RESULT,
    TRIMMING_STEP_MAX,
}trimming_step_t;

#define DAC_OUTPUT_0_0V  (0)
#define DAC_OUTPUT_0_9V  (1116)
#define DAC_OUTPUT_1_05V (1302)
#define DAC_OUTPUT_1_2V  (1489)
#define DAC_OUTPUT_2_0V  (2481)

#define ADC_READ_COUNT          (100) /* 100 - 700ms */
#define ADS114S08_READ_COUNT    (16) /* must be power of 2 */
#define TEST_IOUT_DELTA_COUNT   (20)

#define ADC_REF_VOLTAGE     (5000)
#define ADC_RESOLUTION      (32768-1)

#define LEAKAGE_CURRENT_RSENSE      (200.0f) /* ?�� */
#define LEAKAGE_CURRENT_ROUT        (2.5f) /* ?? */
#define LEAKAGE_CURRENT_RIN         (0.01f) /* ?�� */
#define LEAKAGE_CURRENT_GAIN        (LEAKAGE_CURRENT_ROUT/LEAKAGE_CURRENT_RIN)
#define LEAKAGE_RSENSE_VALUE        (LEAKAGE_CURRENT_RSENSE * LEAKAGE_CURRENT_GAIN)

#define IOUT_CURRENT_RSENSE         (50.0f) /* ?�� */
#define IOUT_CURRENT_ROUT           (4.0f) /* ?? */
#define IOUT_CURRENT_RIN            (0.8f) /* ?? */
#define IOUT_CURRENT_GAIN           (IOUT_CURRENT_ROUT/IOUT_CURRENT_RIN)
#define IOUT_RSENSE_VALUE           (IOUT_CURRENT_RSENSE * IOUT_CURRENT_GAIN)


#define CURRENT_SENSE_R         (24) /* ohm */
#define CURRENT_SENSE_RIN       (2000) /* ohm */
#define CURRENT_SENSE_RO_HIGH   (5100) /* ohm */
#define CURRENT_SENSE_RO_MID    (41000) /* ohm */
#define CURRENT_SENSE_RO_LOW    (410000) /* ohm */

#define CURRENT_SENSE_C_HIGH    (150) /* nF */
#define CURRENT_SENSE_C_MID     (27) /* nF */
#define CURRENT_SENSE_C_LOW     (3.3) /* nF */
#define SAMPLE_TIME             (0.5) /* ms */

#define OTP2_WRITE_EN 0xA5
#define OTP2_WRITE_DIS 0x5A

#if 1
#define CZ_1 0.0f
#define CZ_2 1.7f
#define CZ_3 2.5f
#define CZ_4 3.5f
#else
#define CZ_1 1.5f
#define CZ_2 2.2f
#define CZ_3 3.5f
#define CZ_4 4.5f
#endif

typedef struct _min_max_
{
    uint16_t min;
    uint16_t mid;
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
#define CMD_GUI_RESET                   "ui:reset"

#endif /* ~__TYPES_H__ */

/*** end of file ***/
