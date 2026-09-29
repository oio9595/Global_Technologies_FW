/* USER CODE BEGIN Header */
/*
    * File:   drv_gpio.h
    * Author: GT
    *
    * Created on 2026. 09. 01.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdbool.h>

/* 2. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define DELAY_POWER_UP_MS   (10)  // Delay in milliseconds for VDD ramp-up
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum tag_ID601_VCC_STATE
{
    ID601_VCC_OFF = 0U,
    ID601_VCC_5V0,
    ID601_VCC_5V5,
    ID601_VCC_MAX,
} id601_vcc_state_t;

typedef enum tag_ID601_VLED_STATE
{
    ID601_VLED_OFF = 0U,
    ID601_VLED_ON,
    ID601_VLED_MAX,
} id601_vled_state_t;

typedef enum tag_ID601_CH
{
    ID601_CH_01 = 0U,
    ID601_CH_02,
    ID601_CH_03,
    ID601_CH_04,
    ID601_CH_05,
    ID601_CH_06,
    ID601_CH_07,
    ID601_CH_08,
    ID601_CH_09,
    ID601_CH_10,
    ID601_CH_11,
    ID601_CH_12,
    ID601_CH_MAX,
} id601_ch_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern void drv_gpio_init(void);

extern bool drv_gpio_id601_vcc(id601_vcc_state_t state);
extern bool drv_gpio_id601_vled(id601_vled_state_t state);
extern bool drv_gpio_id601_ch_demux(id601_ch_t ch);

extern bool drv_gpio_ic603_cs(bool state);
extern bool drv_gpio_ads114s08_dev1_cs(bool state);
extern bool drv_gpio_ads114s08_dev2_cs(bool state);
extern bool drv_gpio_ads114s08_dev_all_cs(bool state);

extern bool drv_gpio_serialize_out_enable(bool state);

extern void drv_gpio_ic603_nINT_LD_irq_handler(void);
extern void drv_gpio_ic603_nINT_FAULT_irq_handler(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_GPIO_H__ */
