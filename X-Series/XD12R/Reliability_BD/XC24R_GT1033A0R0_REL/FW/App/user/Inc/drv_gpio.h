/*
 * File:   drv_gpio.h
 * Author: GT
 *
 * Created on 2026. 05. 06.
 */

#ifndef DRV_GPIO_H
#define	DRV_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum tag_VLED_STATE_T
{
    VLED_OFF = 0,
    VLED_ON,
    VLED_MAX,
} vled_state_t;

typedef enum tag_VCC_STATE
{
    VCC_OFF = 0U,
    VCC_ON_3V3,
    VCC_ON_5V5,
    VCC_MAX,
} vcc_state_t;

typedef enum tag_XDIC_CHANNEL_T
{
    XD_CH_01 = 0U,
    XD_CH_02,
    XD_CH_03,
    XD_CH_04,
    XD_CH_05,
    XD_CH_06,
    XD_CH_07,
    XD_CH_08,
    XD_CH_09,
    XD_CH_10,
    XD_CH_11,
    XD_CH_12,
    XD_CH_MAX,
} XD_CH_t;

typedef enum tag_XC_DAC_CHANNEL_T
{
    XC_DAC_CH_01 = 0U,
    XC_DAC_CH_02,
    XC_DAC_CH_03,
    XC_DAC_CH_MAX,
} XC_DAC_CH_t;

typedef enum tag_CURRENT_GAIN_T
{
    GAIN_LOW = 0U,  /* Max 0.5mA */
    GAIN_MID,       /* Max  10mA */
    GAIN_HIGH,      /* Max  40mA */
    GAIN_MAX,
} current_gain_t;

#define XCR_NSS_LO()            (XCR_NSS_GPIO_Port->BSRR = (XCR_NSS_Pin << 16U))
#define XCR_NSS_HI()            (XCR_NSS_GPIO_Port->BSRR = (XCR_NSS_Pin <<  0U))

#define XC_VCC_EN_LO()          (XC_VCC_EN_GPIO_Port->BSRR = (XC_VCC_EN_Pin << 16U))
#define XC_VCC_EN_HI()          (XC_VCC_EN_GPIO_Port->BSRR = (XC_VCC_EN_Pin <<  0U))

#define XC_5V5_EN_LO()          (XC_5V5_EN_GPIO_Port->BSRR = (XC_5V5_EN_Pin << 16U))
#define XC_5V5_EN_HI()          (XC_5V5_EN_GPIO_Port->BSRR = (XC_5V5_EN_Pin <<  0U))

#define BUFFER_OE_LO()          (BUFFER_OE_GPIO_Port->BSRR = (BUFFER_OE_Pin << 16U))
#define BUFFER_OE_HI()          (BUFFER_OE_GPIO_Port->BSRR = (BUFFER_OE_Pin <<  0U))

#define DEBUG_LO()              (DEBUG_GPIO_Port->BSRR = (DEBUG_Pin << 16U))
#define DEBUG_HI()              (DEBUG_GPIO_Port->BSRR = (DEBUG_Pin <<  0U))
#define DEBUG_TOGGLE()          (DEBUG_GPIO_Port->ODR ^= DEBUG_Pin)

void gpio_set_xc_vdd_5v(vcc_state_t state);

#ifdef __cplusplus
}
#endif

#endif  /* ~DRV_GPIO_H */

