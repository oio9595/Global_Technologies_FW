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

typedef enum tag_CURRENT_GAIN_T
{
    GAIN_LOW = 0U,  /* Max 0.5mA */
    GAIN_MID,       /* Max  10mA */
    GAIN_HIGH,      /* Max  40mA */
    GAIN_MAX,
} current_gain_t;

#define XCR_NSS_LO()            (XCR_NSS_GPIO_Port->BSRR = (XCR_NSS_Pin << 16U))
#define XCR_NSS_HI()            (XCR_NSS_GPIO_Port->BSRR = (XCR_NSS_Pin <<  0U))

#define ADS_CS_LO()             (ADC_NSS_GPIO_Port->BSRR = (ADC_NSS_Pin << 16U))
#define ADS_CS_HI()             (ADC_NSS_GPIO_Port->BSRR = (ADC_NSS_Pin <<  0U))

#define LTC_CURRENT_LOW_LO()    (LTC_CURRENT_LOW_GPIO_Port->BSRR = (LTC_CURRENT_LOW_Pin << 16U))
#define LTC_CURRENT_LOW_HI()    (LTC_CURRENT_LOW_GPIO_Port->BSRR = (LTC_CURRENT_LOW_Pin <<  0U))
#define LTC_CURRENT_MID_LO()    (LTC_CURRENT_MID_GPIO_Port->BSRR = (LTC_CURRENT_MID_Pin << 16U))
#define LTC_CURRENT_MID_HI()    (LTC_CURRENT_MID_GPIO_Port->BSRR = (LTC_CURRENT_MID_Pin <<  0U))
#define LTC_CURRENT_HIGH_LO()   (LTC_CURRENT_HIGH_GPIO_Port->BSRR = (LTC_CURRENT_HIGH_Pin << 16U))
#define LTC_CURRENT_HIGH_HI()   (LTC_CURRENT_HIGH_GPIO_Port->BSRR = (LTC_CURRENT_HIGH_Pin <<  0U))

#define DEMUX_CHSEL_1_LO()      (DEMUX_CHSEL_1_GPIO_Port->BSRR = (DEMUX_CHSEL_1_Pin << 16U))
#define DEMUX_CHSEL_1_HI()      (DEMUX_CHSEL_1_GPIO_Port->BSRR = (DEMUX_CHSEL_1_Pin <<  0U))
#define DEMUX_CHSEL_2_LO()      (DEMUX_CHSEL_2_GPIO_Port->BSRR = (DEMUX_CHSEL_2_Pin << 16U))
#define DEMUX_CHSEL_2_HI()      (DEMUX_CHSEL_2_GPIO_Port->BSRR = (DEMUX_CHSEL_2_Pin <<  0U))
#define DEMUX_CHSEL_3_LO()      (DEMUX_CHSEL_3_GPIO_Port->BSRR = (DEMUX_CHSEL_3_Pin << 16U))
#define DEMUX_CHSEL_3_HI()      (DEMUX_CHSEL_3_GPIO_Port->BSRR = (DEMUX_CHSEL_3_Pin <<  0U))
#define DEMUX_CHSEL_4_LO()      (DEMUX_CHSEL_4_GPIO_Port->BSRR = (DEMUX_CHSEL_4_Pin << 16U))
#define DEMUX_CHSEL_4_HI()      (DEMUX_CHSEL_4_GPIO_Port->BSRR = (DEMUX_CHSEL_4_Pin <<  0U))

#define VLED_9V_EN_LO()         (VLED_9V_EN_GPIO_Port->BSRR = (VLED_9V_EN_Pin << 16U))
#define VLED_9V_EN_HI()         (VLED_9V_EN_GPIO_Port->BSRR = (VLED_9V_EN_Pin <<  0U))

#define VLED_DCDC_EN_LO()       (VLED_DCDC_EN_GPIO_Port->BSRR = (VLED_DCDC_EN_Pin << 16U))
#define VLED_DCDC_EN_HI()       (VLED_DCDC_EN_GPIO_Port->BSRR = (VLED_DCDC_EN_Pin <<  0U))

#define XD_VCC_EN_LO()          (XD_VCC_EN_GPIO_Port->BSRR = (XD_VCC_EN_Pin << 16U))
#define XD_VCC_EN_HI()          (XD_VCC_EN_GPIO_Port->BSRR = (XD_VCC_EN_Pin <<  0U))

#define XD_5V5_EN_LO()          (XD_5V5_EN_GPIO_Port->BSRR = (XD_5V5_EN_Pin << 16U))
#define XD_5V5_EN_HI()          (XD_5V5_EN_GPIO_Port->BSRR = (XD_5V5_EN_Pin <<  0U))

#define XC_VCC_EN_LO()          (XC_VCC_EN_GPIO_Port->BSRR = (XC_VCC_EN_Pin << 16U))
#define XC_VCC_EN_HI()          (XC_VCC_EN_GPIO_Port->BSRR = (XC_VCC_EN_Pin <<  0U))

#define XC_5V5_EN_LO()          (XC_5V5_EN_GPIO_Port->BSRR = (XC_5V5_EN_Pin << 16U))
#define XC_5V5_EN_HI()          (XC_5V5_EN_GPIO_Port->BSRR = (XC_5V5_EN_Pin <<  0U))

#define FREQ_MEASURE_RESET_LO() (FREQ_MEASURE_RESET_GPIO_Port->BSRR = (FREQ_MEASURE_RESET_Pin << 16U))
#define FREQ_MEASURE_RESET_HI() (FREQ_MEASURE_RESET_GPIO_Port->BSRR = (FREQ_MEASURE_RESET_Pin <<  0U))

#define BUFFER_OE_LO()          (BUFFER_OE_GPIO_Port->BSRR = (BUFFER_OE_Pin << 16U))
#define BUFFER_OE_HI()          (BUFFER_OE_GPIO_Port->BSRR = (BUFFER_OE_Pin <<  0U))

#define MCO2_ENABLE()           do { LL_GPIO_SetPinMode(XCR_MCLK_GPIO_Port, XCR_MCLK_Pin, LL_GPIO_MODE_ALTERNATE);  LL_GPIO_SetPinPull(XCR_MCLK_GPIO_Port, XCR_MCLK_Pin, LL_GPIO_PULL_NO);      } while(0U)
#define MCO2_DISABLE()          do { LL_GPIO_SetPinMode(XCR_MCLK_GPIO_Port, XCR_MCLK_Pin, LL_GPIO_MODE_INPUT);      LL_GPIO_SetPinPull(XCR_MCLK_GPIO_Port, XCR_MCLK_Pin, LL_GPIO_PULL_DOWN);    } while(0U)

#define FLLSYNC_ENABLE()        do { LL_GPIO_SetPinMode(FLL_SYNC_GPIO_Port, FLL_SYNC_Pin, LL_GPIO_MODE_ALTERNATE);  LL_GPIO_SetPinPull(FLL_SYNC_GPIO_Port, FLL_SYNC_Pin, LL_GPIO_PULL_NO);      } while(0U)
#define FLLSYNC_DISABLE()       do { LL_GPIO_SetPinMode(FLL_SYNC_GPIO_Port, FLL_SYNC_Pin, LL_GPIO_MODE_INPUT);      LL_GPIO_SetPinPull(FLL_SYNC_GPIO_Port, FLL_SYNC_Pin, LL_GPIO_PULL_NO);      } while(0U)

void gpio_set_vled_9v(vled_state_t state);
void gpio_set_vled_dcdc(vled_state_t state);
void gpio_set_xd_vdd_5v(vcc_state_t state);
void gpio_set_xc_vdd_5v(vcc_state_t state);
void gpio_set_current_gain(current_gain_t gain);
void gpio_set_demux_channel_selection(XD_CH_t output_ch);

#ifdef __cplusplus
}
#endif

#endif  /* ~DRV_GPIO_H */

