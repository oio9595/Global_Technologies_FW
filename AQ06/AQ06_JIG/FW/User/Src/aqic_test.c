/** @file aqic_test.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __AQIC_TEST_C__

#include "include.h"

static test_step_t gt_test_mode_step;
const static float aqic_config_voltage_table[AQIC_MODE_MAX][4] =
{
    {CZ_2, CZ_3, CZ_4, CZ_1},
    {CZ_3, CZ_4, CZ_1, CZ_2},
    {CZ_4, CZ_1, CZ_2, CZ_3},
    {CZ_1, CZ_2, CZ_3, CZ_4},
};

static uint16_t gn_test_step_delay;

static uint16_t gn_measure_adc[6];
static float gf_measure_current[6];
static float gf_measure_current_avg;

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

static uint8_t gn_read_vout_id;
static uint8_t gn_aqic_strobe_cnt;

static uint8_t gn_current_gain;

void test_Procedure_Start(void)
{
    gt_test_mode_step = TEST_STEP_INIT;
}
#define CONFIG_DELAY 2000
void AQIC_Set_Operating_Mode(uint8_t mode)
{
    AQIC_CS_LO();
    AQIC_G_LO();

    AQIC_G_HI();
    us_delay(CONFIG_DELAY);
    for (uint8_t cnt = 0 ; cnt < 4 ; ++cnt)
    {
        AQIC_Apply_Voltage(aqic_config_voltage_table[mode][cnt], AQIC_MODE);
        us_delay(CONFIG_DELAY);
        AQIC_CS_HI();
        us_delay(CONFIG_DELAY);
        AQIC_CS_LO();
        us_delay(CONFIG_DELAY);
    }
    AQIC_G_LO();
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

            AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
            AQIC_Apply_Voltage(0.0f, AQIC_MODE);

            AQIC_Mode_Duty(DUTY_ZERO);

            AQIC_VCC_EN(PWR_ON);
            HAL_Delay(50);

            AQIC_Mode_Setting(MODE_DAC);
            HAL_Delay(10);

            AQIC_Set_Operating_Mode(AQIC_MODE_1);
            HAL_Delay(10);

            AQIC_Select_Output_Ch(AQIC_O_MAX);

            AQIC_VLED_EN(ON);
            HAL_Delay(10);

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
            AQIC_G_LO();
            AQIC_CS_HI();
            AQIC_Select_Output_Ch(gn_read_vout_id);
            gf_measure_cz_volt = gf_screen_cz_table[gn_screen_table_index];
            gf_measure_volt = gf_screen_dac_table[gn_screen_table_index];
            gn_current_gain = gn_screen_gain_table[gn_screen_table_index];
            change_current_gain(gn_current_gain);
            gt_test_mode_step = TEST_STEP_VIN_CURRENT_SET;
            break;

        case TEST_STEP_VIN_CURRENT_SET:
            if (gn_read_vout_id == gn_aqic_strobe_cnt)
            {
                AQIC_Apply_Voltage(gf_measure_cz_volt, AQIC_MODE);
#ifndef MANUAL_TEST
                AQIC_Apply_Voltage(gf_measure_volt, AQIC_D_ALL);
#else
                AQIC_Apply_Voltage(gf_manual_volt, AQIC_D_ALL);
#endif
            }
            else
            {
                //AQIC_Apply_Voltage(0.0f, AQIC_MODE);
                AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
            }
            gt_test_mode_step = TEST_STEP_STROBE;
            break;

        case TEST_STEP_STROBE:
            AQIC_G_HI();
            us_delay(100);
            ++gn_aqic_strobe_cnt;
            AQIC_G_LO();

            if (gn_aqic_strobe_cnt >= AQIC_O_MAX)
            {
                gt_test_mode_step = TEST_STEP_ADS114S08_CONV_START;
            }
            else
            {
                gt_test_mode_step = TEST_STEP_VIN_CURRENT_SET;
            }
            break;

        case TEST_STEP_ADS114S08_CONV_START:
            AQIC_CS_LO();
#ifndef MANUAL_TEST
            AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
#endif
            ads114s08_select_single_ended_input(0);

            gb_ads114s08_drdy_done = 0;
            gn_ads114s08_adc_temp = 0;

            gn_adc_read_count = ADS114S08_READ_COUNT;
            gt_test_mode_step = TEST_STEP_ADS114S08_ADC_READ_CHK;

            ads114s08_set_start(1);
            break;

        case TEST_STEP_ADS114S08_ADC_READ_CHK:
            if(gb_ads114s08_drdy_done == 1)
            {
                gn_measure_adc[gn_read_vout_id] = (uint16_t)(gn_ads114s08_adc_temp/ADS114S08_READ_COUNT);
                gb_ads114s08_drdy_done = 0;
                gt_test_mode_step = TEST_STEP_CURRENT_DISCHARGE;
            }
            break;

        case TEST_STEP_CURRENT_DISCHARGE:
            gn_aqic_strobe_cnt = 0;
            ++gn_read_vout_id;
            if (gn_read_vout_id >= AQIC_O_MAX)
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

            ++gn_screen_table_index;
            if (gn_screen_table_index == SCREEN_TABLE_INDEX_MAX)
            {
                gt_test_mode_step = TEST_STEP_END;
            }
            else
            {
                gt_test_mode_step = TEST_STEP_VIN_CURRENT_CHANGE;
            }
            break;

        case TEST_STEP_END:
            if(gn_test_step_delay == 0)
            {
                AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
                AQIC_Apply_Voltage(0.0f, AQIC_MODE);

                /* VLED_CTR_7V : OFF */
                AQIC_VLED_EN(PWR_OFF);
                AQIC_Select_Output_Ch(AQIC_O_MAX);
                AQIC_VCC_EN(PWR_OFF);
            }
            else
            {
                --gn_test_step_delay;
            }
            break;
    }
}
/*** end of file ***/

