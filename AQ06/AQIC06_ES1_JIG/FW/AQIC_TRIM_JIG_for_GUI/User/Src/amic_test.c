/** @file amic_test.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __AMIC_TEST_C__
#include "main.h"
#include "log.h"
#include "amic_test.h"
#include "user_flash.h"
#include "fnd.h"
#include "ADS124S08.h"
#include "trimming.h"
#include "types.h"

extern I2C_HandleTypeDef hi2c1;
extern ADC_HandleTypeDef hadc1;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim12;
//extern TIM_HandleTypeDef htim6;


uint16_t gn_auto_test_count;
extern uint16_t gn_auto_test_ok_count;
extern uint16_t gn_auto_test_ng_count;

test_mode_t gt_jig_test_mode;
test_mode_t gt_jig_manual_test_mode;
extern auto_manual_t gt_auto_manual_state;
test_step_t gt_test_mode_step;

extern trimming_step_t gt_jig_trimming_step;
extern uint8_t gn_apic_test_type;      /* 0 : normal test, 1 : 1 : trimming */

extern aqic_config_t aqic_config;

static uint8_t gb_adc_start_dma;
static uint16_t gn_adc_dma_buffer[4];

static uint16_t gn_test_step_delay;
uint16_t gn_adc_read_count;
uint32_t gn_test_tickcount;

static uint16_t gn_mode_adc_reault[TEST_MODE_MAX][AQ06_O_MAX];
static uint8_t gn_test_mode_result[TEST_MODE_MAX];

static uint8_t gn_manual_function_test_repeat_cnt;
static min_max_t gt_manual_function_test_min_max[FUNCTION_TEST_REPEAT_CNT][AQ06_O_MAX];
static uint16_t gn_mode_iout_delta_reault[TEST_IOUT_DELTA_COUNT][AQ06_O_MAX];

static uint16_t gn_measure_adc[6];
static float gf_measure_current[6];
static float gf_measure_current_avg;
static float gf_measure_current_summary[24];

float gf_measure_cz_volt = 0.0f;

float gf_manual_volt = 0.0f;
static float gf_measure_volt;
float gf_measure_end_point = 4.5f;

uint8_t gn_manual_gain = GAIN_HIGH;

#define SCREEN_TABLE_INDEX_MAX  4
static float gf_screen_dac_table[SCREEN_TABLE_INDEX_MAX] = {0.286f, 2.5f, 4.5f, 1.0f};
static float gf_screen_cz_table[SCREEN_TABLE_INDEX_MAX] = {CZ_2, CZ_1, CZ_1, CZ_2};
static uint8_t gn_screen_gain_table[SCREEN_TABLE_INDEX_MAX] = {GAIN_LOW, GAIN_HIGH, GAIN_HIGH, GAIN_HIGH};
static uint8_t gn_screen_table_index;

uint8_t gb_ads114s08_drdy_done;
uint64_t gn_ads114s08_adc_temp;
uint32_t gn_ads114s08_adc_accu_count;

static uint8_t gn_read_vout_id;
static uint8_t gn_aqic_strobe_cnt;

#if 0
TEST_MODE_NONE = 0,
TEST_MODE_LEAKAGE,
TEST_MODE_PWM_DUTY_VARIATION,
TEST_MODE_IOUT_GATE_HOLD,
TEST_MODE_IOUT_D0R10V,
TEST_MODE_IOUT_D0R17V,
TEST_MODE_IOUT_D1R00V,
TEST_MODE_IOUT_D2R00V,
TEST_MODE_IOUT_D3R50V,
TEST_MODE_IOUT_FUNCTION,
TEST_MODE_IOUT_FUNC_PWM,
TEST_MODE_IOUT_FUNC_DELTA,
TEST_MODE_RESULT = 10,
#endif
static uint16_t gn_test_mode_delay_table[TEST_MODE_MAX][4] =
{
  {0},
};

static min_max_t gt_adc_tolerance[ADC_TOLER_MAX][TEST_MODE_MAX] =
{
  {0},
};


static uint8_t gn_test_measure_index;
static float gf_test_measure_table[] = { 0.12, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5 };

static float gf_test_mode_current_table[TEST_MODE_MAX][2] =/* current, pwm */
{
  {0},
};


extern void PlayBeep(beep_freq_t t_type, uint16_t n_time);
extern void change_i2c_setting(uint8_t i2c_type);
extern void dac_set_voltage(float f_voltage, uint8_t ch);
static double adc_to_current(uint16_t adc_value, uint8_t gain)
{
    double ret = 0;
    switch (gain)
    {
        case GAIN_HIGH :
            ret = ((adc_value * ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_HIGH * CURRENT_SENSE_R));	/* mA */
            break;

        case GAIN_MID :
            ret = ((adc_value * ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_MID * CURRENT_SENSE_R));	/* mA */
            break;

        case GAIN_LOW :
            ret = ((adc_value * ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_LOW * CURRENT_SENSE_R));	/* mA */
            break;
    }
    return ret;
}

void set_amic_power(uint8_t on)
{
    if(on == PWR_OFF) /* off */
    {
        /* AMIC_PWR_CTL : OFF, LOW */
        HAL_GPIO_WritePin(AQIC_PWR_CTL_GPIO_Port, AQIC_PWR_CTL_Pin, GPIO_PIN_RESET);
    }
    else
    {
        /* AMIC_PWR_CTL : ON, HIGH */
        HAL_GPIO_WritePin(AQIC_PWR_CTL_GPIO_Port, AQIC_PWR_CTL_Pin, GPIO_PIN_SET);
    }
}

void set_vled_9v(uint8_t on)
{
    if(on == PWR_ON)
    {
        HAL_GPIO_WritePin(VLED_CTR_9V_GPIO_Port, VLED_CTR_9V_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(VLED_CTR_9V_GPIO_Port, VLED_CTR_9V_Pin, GPIO_PIN_SET);
    }
}



static void set_amic_chip_select(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(MCU_I2C_SCL_CS_GPIO_Port, MCU_I2C_SCL_CS_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(MCU_I2C_SCL_CS_GPIO_Port, MCU_I2C_SCL_CS_Pin, GPIO_PIN_RESET);
    }
}

static void set_amic_gate_select(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(MCU_I2C_SDA_G_GPIO_Port, MCU_I2C_SDA_G_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(MCU_I2C_SDA_G_GPIO_Port, MCU_I2C_SDA_G_Pin, GPIO_PIN_RESET);
    }
}

void set_amic_mode(uint8_t en)
{
    if(en == ON)
    {
        //HAL_GPIO_WritePin(APIC_MODE_GPIO_Port, APIC_MODE_Pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (htim3.Init.Period+1));    /* pwm duty 100% */
    }
    else if(GEN_PWM == en)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (htim3.Init.Period / 2)+1);    /* pwm duty ~50%, freq 1MHz */
    }
    else
    {
        //HAL_GPIO_WritePin(APIC_MODE_GPIO_Port, APIC_MODE_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);    /* pwm duty 0% */
    }
}

void aqic_set_mode(uint8_t mode)
{
    AQIC_CS_LO;
    AQIC_G_LO;
    switch (mode)
    {
        case one_by_six :
            AQIC_G_HI;
            HAL_Delay(1);
            dac_set_voltage(CZ_2, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_3, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_4, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_1, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            AQIC_G_LO;
            break;

        case two_by_three :
            AQIC_G_HI;
            HAL_Delay(1);
            dac_set_voltage(CZ_3, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_4, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_1, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_2, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            AQIC_G_LO;
            break;

        case three_by_two :
            AQIC_G_HI;
            HAL_Delay(1);
            dac_set_voltage(CZ_4, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_1, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_2, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_3, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            AQIC_G_LO;
            break;

        case test_mode :
            AQIC_G_HI;
            HAL_Delay(1);
            dac_set_voltage(CZ_1, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_2, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_3, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            dac_set_voltage(CZ_4, AQIC_MODE);
            HAL_Delay(1);
            AQIC_CS_HI;
            HAL_Delay(1);
            AQIC_CS_LO;
            HAL_Delay(1);
            AQIC_G_LO;
            break;
    }
}

void aqic_output_select(uint8_t ch)
{
    static uint8_t ch_mem = 0xFF;
    if (ch_mem != ch)
    {
        ch_mem = ch;
    	switch (ch_mem)
    	{
            case 0 :
                // HAL_GPIO_WritePin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin, GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin, GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin, GPIO_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_RESET);
                break;

            case 1 :
                // HAL_GPIO_WritePin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin, GPIO_PIN_SET);
                // HAL_GPIO_WritePin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin, GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin, GPIO_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_RESET);
                break;

            case 2 :
                // HAL_GPIO_WritePin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin, GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin, GPIO_PIN_SET);
                // HAL_GPIO_WritePin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin, GPIO_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_RESET);
                break;

            case 3 :
                // HAL_GPIO_WritePin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin, GPIO_PIN_SET);
                // HAL_GPIO_WritePin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin, GPIO_PIN_SET);
                // HAL_GPIO_WritePin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin, GPIO_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_RESET);
                break;

            case 4 :
                // HAL_GPIO_WritePin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin, GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin, GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin, GPIO_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_SET);
                break;

            case 5 :
                // HAL_GPIO_WritePin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin, GPIO_PIN_SET);
                // HAL_GPIO_WritePin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin, GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin, GPIO_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_SET);
                break;

            case 6 :
                // HAL_GPIO_WritePin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin, GPIO_PIN_SET);
                // HAL_GPIO_WritePin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin, GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin, GPIO_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_RESET);
                LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_SET);
                break;

            default :
                // HAL_GPIO_WritePin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin, GPIO_PIN_SET);
                // HAL_GPIO_WritePin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin, GPIO_PIN_SET);
                // HAL_GPIO_WritePin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin, GPIO_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_SET);
                LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_SET);
                break;
        }
    }
}

void set_ok_led(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_RESET);
    }
}

void set_ng_led(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_NG_GPIO_Port, LED_NG_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_NG_GPIO_Port, LED_NG_Pin, GPIO_PIN_RESET);
    }
}

void set_trimming_led(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_TRIM_GPIO_Port, LED_TRIM_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_TRIM_GPIO_Port, LED_TRIM_Pin, GPIO_PIN_RESET);
    }
}

/*
CTL_UPPER_GAIN
 - HIGH : Sensing range 40mA
 - MID  : Sensing range 10mA
 - LOW  : Sensing range  1mA
*/
uint8_t gn_current_gain;

void change_current_gain(uint8_t gain)
{
    if(gain == GAIN_HIGH)
    {
        // HAL_GPIO_WritePin(LTC_HIGH_CURRENT_GPIO_Port, LTC_HIGH_CURRENT_Pin, GPIO_PIN_RESET);   // HIGH : Sensing range 40mA
        // HAL_GPIO_WritePin(LTC_MID_CURRENT_GPIO_Port, LTC_MID_CURRENT_Pin, GPIO_PIN_SET);	   // MID : Sensing range 10mA
        // HAL_GPIO_WritePin(LTC_LOW_CURRENT_GPIO_Port, LTC_LOW_CURRENT_Pin, GPIO_PIN_SET);	   // LOW : Sensing range 1mA
        LATCH__WritePin(LTC_HIGH_CURRENT_LS_CP, LTC_HIGH_CURRENT_LS_PORT, LATCH_PIN_RESET);   // HIGH : Sensing range 40mA
        LATCH__WritePin(LTC_MID_CURRENT_LS_CP, LTC_MID_CURRENT_LS_PORT, LATCH_PIN_SET);   // HIGH : Sensing range 40mA
        LATCH__WritePin(LTC_LOW_CURRENT_LS_CP, LTC_LOW_CURRENT_LS_PORT, LATCH_PIN_SET);   // HIGH : Sensing range 40mA
    }
    else if (gain == GAIN_MID)
    {
        // HAL_GPIO_WritePin(LTC_HIGH_CURRENT_GPIO_Port, LTC_HIGH_CURRENT_Pin, GPIO_PIN_SET);   // HIGH : Sensing range 40mA
        // HAL_GPIO_WritePin(LTC_MID_CURRENT_GPIO_Port, LTC_MID_CURRENT_Pin, GPIO_PIN_RESET);	   // MID : Sensing range 10mA
        // HAL_GPIO_WritePin(LTC_LOW_CURRENT_GPIO_Port, LTC_LOW_CURRENT_Pin, GPIO_PIN_SET);	   // LOW : Sensing range 1mA
        LATCH__WritePin(LTC_HIGH_CURRENT_LS_CP, LTC_HIGH_CURRENT_LS_PORT, LATCH_PIN_SET);   // HIGH : Sensing range 40mA
        LATCH__WritePin(LTC_MID_CURRENT_LS_CP, LTC_MID_CURRENT_LS_PORT, LATCH_PIN_RESET);   // HIGH : Sensing range 40mA
        LATCH__WritePin(LTC_LOW_CURRENT_LS_CP, LTC_LOW_CURRENT_LS_PORT, LATCH_PIN_SET);   // HIGH : Sensing range 40mA
    }
    else if (gain == GAIN_LOW)
    {
        // HAL_GPIO_WritePin(LTC_HIGH_CURRENT_GPIO_Port, LTC_HIGH_CURRENT_Pin, GPIO_PIN_SET);   // HIGH : Sensing range 40mA
        // HAL_GPIO_WritePin(LTC_MID_CURRENT_GPIO_Port, LTC_MID_CURRENT_Pin, GPIO_PIN_SET);	   // MID : Sensing range 10mA
        // HAL_GPIO_WritePin(LTC_LOW_CURRENT_GPIO_Port, LTC_LOW_CURRENT_Pin, GPIO_PIN_RESET);	   // LOW : Sensing range 1mA
        LATCH__WritePin(LTC_HIGH_CURRENT_LS_CP, LTC_HIGH_CURRENT_LS_PORT, LATCH_PIN_SET);   // HIGH : Sensing range 40mA
        LATCH__WritePin(LTC_MID_CURRENT_LS_CP, LTC_MID_CURRENT_LS_PORT, LATCH_PIN_SET);   // HIGH : Sensing range 40mA
        LATCH__WritePin(LTC_LOW_CURRENT_LS_CP, LTC_LOW_CURRENT_LS_PORT, LATCH_PIN_RESET);   // HIGH : Sensing range 40mA
    }
}

void current_discharge(uint8_t mode)
{
    static uint8_t setting = 0;

    if(setting != mode)
    {
        setting = mode;
        if(setting == CHARGE)
        {
            HAL_GPIO_WritePin(LTC_DISCHARGE_GPIO_Port, LTC_DISCHARGE_Pin, GPIO_PIN_RESET);
        }
        else // if (setting == DISCHARGE)
        {
            HAL_GPIO_WritePin(LTC_DISCHARGE_GPIO_Port, LTC_DISCHARGE_Pin, GPIO_PIN_SET);
        }
    }
}

static void test_mode_port_init(void)
{
    /* VLED_CTR_7V : OFF */
    set_amic_power(PWR_OFF);
    set_vled_9v(PWR_OFF);

    /* SPI DAC : 0V */
    dac_set_voltage(0.0f, AQIC_D_ALL);
    dac_set_voltage(0.0f, AQIC_MODE);

    /* Ctrl for APIC : LOW */
    set_amic_mode(OFF);
}

void test_procedure_run(void)
{
    switch(gt_test_mode_step)
    {
        case TEST_STEP_INIT:
            gn_read_vout_id = 0;
            gn_aqic_strobe_cnt = 0;

            gf_measure_volt = 0.2f;
            gf_measure_cz_volt = CZ_2;

            dac_set_voltage(0.0f, AQIC_D_ALL);
            dac_set_voltage(0.0f, AQIC_MODE);

            set_amic_mode(OFF);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

            set_amic_power(PWR_ON);
            HAL_Delay(50);

            aqic_mode_select(MODE_DAC);
            HAL_Delay(10);

            aqic_set_mode(one_by_six);
            HAL_Delay(10);

            //////////////////////////////////////////
            //set_vled_9v(ON);
            HAL_Delay(10);

            aqic_output_select(8);

            current_discharge(CHARGE);
#ifndef MANUAL_TEST
            gn_current_gain = GAIN_HIGH;
#else
            gn_current_gain = gn_manual_gain;
#endif
            change_current_gain(gn_current_gain);

            gt_test_mode_step = TEST_STEP_INIT_DELAY;
            break;

        case TEST_STEP_INIT_DELAY:
            if(gn_test_step_delay == 0)
            {
                gt_test_mode_step = TEST_STEP_VIN_CURRENT_CHANGE;
            }
            else
            {
                --gn_test_step_delay;
            }
            break;

        case TEST_STEP_VIN_CURRENT_CHANGE:
            AQIC_G_LO;
            AQIC_CS_HI;

            //////////////////////////////////////////
            aqic_output_select(gn_read_vout_id);
            gf_measure_cz_volt = gf_screen_cz_table[gn_screen_table_index];
            gf_measure_volt = gf_screen_dac_table[gn_screen_table_index];
            gn_current_gain = gn_screen_gain_table[gn_screen_table_index];
            change_current_gain(gn_current_gain);
            gt_test_mode_step = TEST_STEP_VIN_CURRENT_SET;
            break;

        case TEST_STEP_VIN_CURRENT_SET:
            if (gn_read_vout_id == gn_aqic_strobe_cnt)
            {
                dac_set_voltage(gf_measure_cz_volt, AQIC_MODE);
#ifndef MANUAL_TEST
                dac_set_voltage(gf_measure_volt, AQIC_D_ALL);
#else
                dac_set_voltage(gf_manual_volt, AQIC_D_ALL);
#endif
            }
            else
            {
                //dac_set_voltage(0.0f, AQIC_MODE);
                dac_set_voltage(0.0f, AQIC_D_ALL);
            }
            gt_test_mode_step = TEST_STEP_STROBE;
            break;

        case TEST_STEP_STROBE:
            AQIC_G_HI;
            us_delay(100);
            gn_aqic_strobe_cnt++;
            AQIC_G_LO;

            if (gn_aqic_strobe_cnt >= AQ06_O_MAX)
            {
                gt_test_mode_step = TEST_STEP_ADS114S08_CONV_START;
            }
            else
            {
                gt_test_mode_step = TEST_STEP_VIN_CURRENT_SET;
            }
            break;

        case TEST_STEP_ADS114S08_CONV_START:
            AQIC_CS_LO;
            // dac_set_voltage(0.0f, AQIC_MODE);
#ifndef MANUAL_TEST
            dac_set_voltage(0.0f, AQIC_D_ALL);
#endif
            ads114s08_select_single_ended_input(0);

            gb_ads114s08_drdy_done = 0;
            gn_ads114s08_adc_temp = 0;
            gn_ads114s08_adc_accu_count = 0;

            gn_adc_read_count = ADS114S08_READ_COUNT;
            gt_test_mode_step = TEST_STEP_ADS114S08_ADC_READ_CHK;

            ads114s08_set_start(1);
            break;

        case TEST_STEP_ADS114S08_ADC_READ_CHK:
            if(gb_ads114s08_drdy_done == 1)
            {
                gn_measure_adc[gn_read_vout_id] = (uint16_t)(gn_ads114s08_adc_temp/gn_ads114s08_adc_accu_count);
                gb_ads114s08_drdy_done = 0;
                gt_test_mode_step = TEST_STEP_CURRENT_DISCHARGE;
            }
            break;

        case TEST_STEP_CURRENT_DISCHARGE:
            gn_aqic_strobe_cnt = 0;
            gn_read_vout_id++;
            if (gn_read_vout_id >= AQ06_O_MAX)
            {
                gn_read_vout_id = 0;
                gt_test_mode_step = TEST_STEP_LOG;
            }
            else
            {
                gn_test_step_delay = 0;
                gt_test_mode_step = TEST_STEP_VIN_CURRENT_CHANGE;
            }
            break;

        case TEST_STEP_LOG:
            for (uint8_t ch = 0 ; ch < 6 ; ++ch)
            {
                gf_measure_current[ch] = 1000 * adc_to_current(gn_measure_adc[ch], gn_current_gain);
                gf_measure_current_avg += (gf_measure_current[ch] / 6);
                gf_measure_current_summary[6 * gn_screen_table_index + ch] = gf_measure_current[ch];
            }
#ifndef MANUAL_TEST
            snprintf(msg_buffer, sizeof msg_buffer, "%3.2f   ", gf_measure_volt);
#else
            snprintf(msg_buffer, sizeof msg_buffer, "%3.2f   ", gf_manual_volt);
#endif
            print(msg_buffer);
            snprintf(msg_buffer, sizeof msg_buffer, "%9.3f   %9.3f   %9.3f   %9.3f   %9.3f   %9.3f   [uA]   ", gf_measure_current[0], gf_measure_current[1], gf_measure_current[2], gf_measure_current[3], gf_measure_current[4], gf_measure_current[5]);
            print(msg_buffer);
            snprintf(msg_buffer, sizeof msg_buffer, "%9.3f   ", gf_measure_current_avg);
            print(msg_buffer);
            snprintf(msg_buffer, sizeof msg_buffer, "%8.3f   %8.3f   %8.3f   %8.3f   %8.3f   %8.3f   [%]\r\n",
            100 * (gf_measure_current[0] - gf_measure_current_avg)/gf_measure_current_avg, 100 * (gf_measure_current[1] - gf_measure_current_avg)/gf_measure_current_avg, 100 * (gf_measure_current[2] - gf_measure_current_avg)/gf_measure_current_avg,
            100 * (gf_measure_current[3] - gf_measure_current_avg)/gf_measure_current_avg, 100 * (gf_measure_current[4] - gf_measure_current_avg)/gf_measure_current_avg, 100 * (gf_measure_current[5] - gf_measure_current_avg)/gf_measure_current_avg);
            print(msg_buffer);

            gf_measure_current_avg = 0;
            #if 0
            if (gf_measure_volt < 0.295)
            {
                gf_measure_volt += 0.01f;
            }
            else
            {
                gf_measure_volt += 0.1f;
            }

            if (gf_measure_volt > gf_measure_end_point+0.05)
            {
                gt_test_mode_step = TEST_STEP_END;
            }
            else
            {
                gt_test_mode_step = TEST_STEP_VIN_CURRENT_CHANGE;
            }
            #else

            ++gn_screen_table_index;
            if (gn_screen_table_index == 4)
            {
                gt_test_mode_step = TEST_STEP_END;
            }
            else
            {
                gt_test_mode_step = TEST_STEP_VIN_CURRENT_CHANGE;
            }

            #endif
            break;


        case TEST_STEP_END:
            if(gn_test_step_delay == 0)
            {
                dac_set_voltage(0.0f, AQIC_D_ALL);
                dac_set_voltage(0.0f, AQIC_MODE);

                /* VLED_CTR_7V : OFF */
                set_vled_9v(PWR_OFF);
                aqic_output_select(AQ06_O_MAX);
                set_amic_power(PWR_OFF); /* APIC power off */

                for (uint8_t ch = 0 ; ch < 24 ; ++ch)
                {
                    snprintf(msg_buffer, sizeof msg_buffer, "%9.3f, ", gf_measure_current_summary[ch]);
                    print(msg_buffer);
                }

                NVIC_SystemReset();
            }
            else
            {
                --gn_test_step_delay;
            }
            break;

        case TEST_STEP_BEEP_DELAY:
            if(gn_test_step_delay == 0)
            {
                snprintf(msg_buffer, sizeof msg_buffer, "ng_beep\r\n");
                print(msg_buffer);
                gt_test_mode_step = TEST_STEP_NEXT;
            }
            else
            {
                --gn_test_step_delay;
            }
            break;
    }
}
//static uint8_t gb_pattern_test_started;
void test_start(void)
{
    if(gn_apic_test_type == 0)
    {
        if(gt_jig_test_mode == TEST_MODE_NONE)
        {
            if(gt_auto_manual_state == TEST_AUTO)
            {
                /* init test result flag */
                gn_test_mode_result[TEST_MODE_MEASURE] = 0;

                gt_jig_test_mode = TEST_MODE_MEASURE;

                ++gn_auto_test_count;
                log_send_auto_test_start(gn_auto_test_count);
                gn_test_tickcount = HAL_GetTick();
            }
            else if(gt_auto_manual_state == TEST_MANUAL)
            {
#if 1
#if 0
                if(gb_pattern_test_started == 0)
                {
                    gb_pattern_test_started = 1;

                    /* Ctrl for APIC : LOW */
                    set_amic_chip_select(OFF);
                    set_amic_mode(OFF);
                    HAL_GPIO_WritePin(MCU_I2C_SDA_G_GPIO_Port, MCU_I2C_SDA_G_Pin, GPIO_PIN_RESET);

                    set_amic_power(PWR_ON);
                    /* VLED_CTR_7V : ON */
                    set_vled_9v(PWR_ON);

                    /* SPI DAC : 0.0V */
                    dac_set_voltage(0.0f, AQIC_D_ALL);

                    HAL_TIM_Base_Start_IT(&htim4);
                }
                else
                {
                    gb_pattern_test_started = 0;

                    HAL_TIM_Base_Stop_IT(&htim4);

                    /* Ctrl for APIC : LOW */
                    set_amic_chip_select(OFF);
                    set_amic_mode(OFF);
                    HAL_GPIO_WritePin(MCU_I2C_SDA_G_GPIO_Port, MCU_I2C_SDA_G_Pin, GPIO_PIN_RESET);

                    set_amic_power(PWR_OFF);
                    /* VLED_CTR_7V : OFF */
                    set_vled_9v(PWR_OFF);

                    /* SPI DAC : 0.0V */
                    dac_set_voltage(0.0f, AQIC_D_ALL);

                }
#else
                gn_manual_function_test_repeat_cnt = 0;

                for(uint16_t n=0 ; n<FUNCTION_TEST_REPEAT_CNT ; ++n)
                {
                    for(uint16_t o=0 ; o<AQ06_O_MAX ; ++o)
                    {
                        gt_manual_function_test_min_max[n][o].min = 32767;
                        gt_manual_function_test_min_max[n][o].max = 0;
                    }
                }


                gt_jig_manual_test_mode = TEST_MODE_MEASURE;
#endif
#else
                ++gt_jig_manual_test_mode;

                if(gt_jig_manual_test_mode == TEST_MODE_RESULT)
                {
                    gt_jig_manual_test_mode = TEST_MODE_LEAKAGE;
                }
#endif
                gt_jig_test_mode = gt_jig_manual_test_mode;

                log_send_manual_test_start();
            }
        }
    }
    else
    {
        if(gt_jig_trimming_step == TRIMMING_STEP_NONE)
        {
            gt_jig_trimming_step = TRIMMING_STEP_INIT;
        }
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        HAL_ADC_Stop_DMA(hadc);
        gb_adc_start_dma = 0;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(ADC_DRDY_Pin == GPIO_Pin)
    {
        int32_t temp = 0;
#if 1
        temp = ads114s08_get_data(NULL, NULL);
#else
        temp = ads114s08_get_rdata(NULL, NULL);
#endif
        if (temp < 0)
        {
            temp = 0;
        }
        if(temp > 32767)
        {
            temp = 32767;
        }
#if 0
        snprintf(msg_buffer, sizeof msg_buffer, "\t[%5d]\r\n", temp);
        print(msg_buffer);
#endif

        if(gn_adc_read_count)
        {
            gn_ads114s08_adc_temp += temp;
            ++gn_ads114s08_adc_accu_count;
            --gn_adc_read_count;
        }

        if(gn_adc_read_count == 0)
        {
            gb_ads114s08_drdy_done = 1;
            ads114s08_set_start(0);    /* stop continuous conversion */
        }
    }
}
/*** end of file ***/

