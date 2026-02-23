/** @file switch.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef _SWITCH_H_
#define _SWITCH_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "config.h"

/* BEGIN - INTERFACE FUNCTIONS */

typedef enum
{
    DECODE_ADC_GAIN_TRIM = 0,
    DECODE_LTC_DRIVER_A,
    DECODE_TEST_V_TRIM,
    DECODE_SIO1_LVDS_TX,
    DECODE_SIO2_LVDS_TX,
    DECODE_SIO1_LVDS_RX,
    DECODE_SIO2_LVDS_RX,
    DECODE_LTC_DRIVER_B,
    DECODE_NONE,
}decode_mode_t;

typedef enum
{
    PWR_OFF = 0,
    PWR_ON,
}power_state_t;

typedef enum
{
    SAL_VCC_5V0 = 0,
    SAL_VCC_5V5,
}_sal_vcc_level_t;

EXTERN void decode_mode_set(decode_mode_t n_mode);

EXTERN void sal_make_mcu_mode(void);
EXTERN void sal_make_i2c_mode(void);

EXTERN void sal_vled_en(power_state_t pwr);
EXTERN void sal_vcc_en(power_state_t pwr);
EXTERN void sal_vcc_level_set(_sal_vcc_level_t level);

#ifdef __cplusplus
}
#endif

#endif /* ~_SWITCH_H_ */

/*** end of file ***/
