/*
 * File:   drv_gpio.c
 * Author: GT
 *
 * Created on 2026. 05. 06.
 */

#include "drv_gpio.h"

void gpio_set_xc_vdd_5v(vcc_state_t state)
{
    switch(state)
    {
        case VCC_OFF: // on -> off : 40ms
        {
            XC_VCC_EN_HI();
            XC_5V5_EN_LO();
            break;
        }
        case VCC_ON_3V3: // off -> 3.3 : 10ms
        {
            XC_VCC_EN_LO();
            XC_5V5_EN_LO();
            break;
        }
        case VCC_ON_5V5: // 3.3 -> 5.5 : 10ms
        {
            XC_VCC_EN_LO();
            XC_5V5_EN_HI();
            break;
        }
        default:
        {
            XC_VCC_EN_HI();
            XC_5V5_EN_LO();
            break;
        }
    }
}

