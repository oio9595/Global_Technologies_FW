/* 
 * File:   drv_gpio.c
 * Author: GT
 *
 * Created on 2026. 05. 06.
 */

#include "drv_gpio.h"

void gpio_set_power_9v(power_state_t state)
{
    switch(state)
    {
    case PWR_OFF:
        VLED_9V_EN_HI();
        break;
    case PWR_ON:
        VLED_9V_EN_LO();
        break;
    default:
        VLED_9V_EN_HI();
        break;
    }
}

void gpio_set_xd_vdd_5v(xd_vdd_volt_t state)
{
    switch(state)
    {
    case XD_PWR_OFF:
        XD_VCC_EN_HI();
        XD_5V5_EN_LO();
        break;
    case XD_PWR_ON_5V0:
        XD_VCC_EN_LO();
        XD_5V5_EN_LO();
        break;
    case XD_PWR_ON_5V5:
        XD_VCC_EN_LO();
        XD_5V5_EN_HI();
        break;
    default:
        XD_VCC_EN_HI();
        XD_5V5_EN_LO();
        break;
    }
}

void gpio_set_current_gain(current_gain_t gain)
{
    switch(gain)
    {
    case GAIN_LOW:
        LTC_CURRENT_LOW_HI();
        LTC_CURRENT_MID_LO();
        LTC_CURRENT_HIGH_LO();
        break;
    case GAIN_MID:
        LTC_CURRENT_LOW_LO();
        LTC_CURRENT_MID_HI();
        LTC_CURRENT_HIGH_LO();
        break;
    case GAIN_HIGH:
        LTC_CURRENT_LOW_LO();
        LTC_CURRENT_MID_LO();
        LTC_CURRENT_HIGH_HI();
        break;
    default:
        LTC_CURRENT_LOW_LO();
        LTC_CURRENT_MID_LO();
        LTC_CURRENT_HIGH_LO();
        break;
    }
}

void gpio_set_demux_channel_selection(XD_CH_t output_ch)
{
    switch(output_ch)
    {
    case XD_CH_01:
        DEMUX_CHSEL_1_LO();
        DEMUX_CHSEL_2_LO();
        DEMUX_CHSEL_3_LO();
        DEMUX_CHSEL_4_LO();
        break;
    case XD_CH_02:
        DEMUX_CHSEL_1_LO();
        DEMUX_CHSEL_2_HI();
        DEMUX_CHSEL_3_LO();
        DEMUX_CHSEL_4_LO();
        break;
    case XD_CH_03:
        DEMUX_CHSEL_1_HI();
        DEMUX_CHSEL_2_LO();
        DEMUX_CHSEL_3_LO();
        DEMUX_CHSEL_4_LO();
        break;
    case XD_CH_04:
        DEMUX_CHSEL_1_HI();
        DEMUX_CHSEL_2_HI();
        DEMUX_CHSEL_3_LO();
        DEMUX_CHSEL_4_LO();
        break;
    case XD_CH_05:
        DEMUX_CHSEL_1_LO();
        DEMUX_CHSEL_2_LO();
        DEMUX_CHSEL_3_HI();
        DEMUX_CHSEL_4_LO();
        break;
    case XD_CH_06:
        DEMUX_CHSEL_1_LO();
        DEMUX_CHSEL_2_HI();
        DEMUX_CHSEL_3_HI();
        DEMUX_CHSEL_4_LO();
        break;
    case XD_CH_07:
        DEMUX_CHSEL_1_HI();
        DEMUX_CHSEL_2_LO();
        DEMUX_CHSEL_3_HI();
        DEMUX_CHSEL_4_LO();
        break;
    case XD_CH_08:
        DEMUX_CHSEL_1_HI();
        DEMUX_CHSEL_2_HI();
        DEMUX_CHSEL_3_HI();
        DEMUX_CHSEL_4_LO();
        break;
    case XD_CH_09:
        DEMUX_CHSEL_1_LO();
        DEMUX_CHSEL_2_LO();
        DEMUX_CHSEL_3_LO();
        DEMUX_CHSEL_4_HI();
        break;
    case XD_CH_10:
        DEMUX_CHSEL_1_LO();
        DEMUX_CHSEL_2_HI();
        DEMUX_CHSEL_3_LO();
        DEMUX_CHSEL_4_HI();
        break;
    case XD_CH_11:
        DEMUX_CHSEL_1_HI();
        DEMUX_CHSEL_2_LO();
        DEMUX_CHSEL_3_LO();
        DEMUX_CHSEL_4_HI();
        break;
    case XD_CH_12:
        DEMUX_CHSEL_1_HI();
        DEMUX_CHSEL_2_HI();
        DEMUX_CHSEL_3_LO();
        DEMUX_CHSEL_4_HI();
        break;
    default :
        DEMUX_CHSEL_1_HI();
        DEMUX_CHSEL_2_HI();
        DEMUX_CHSEL_3_HI();
        DEMUX_CHSEL_4_HI();
        break;
    }
}



