/* USER CODE BEGIN Header */
/*
    * File:   drv_ads114s08.h
    * Author: GT
    *
    * Created on 2026. 09. 22.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_ADS114S08_H__
#define __DRV_ADS114S08_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */
#include "main.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum tag_ADS_DEVICE
{
    ADS_DEV_1 = 0U,
    ADS_DEV_2,
    ADS_DEV_MAX,
} ads_device_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern void ads114s08_set_conversion_enable(ads_device_t dev, bool b_start);
extern bool ads114s08_wait_conversion_complete(void);
extern int32_t ads114s08_get_conversion(ads_device_t dev);

extern void ads114s08_init(void);

extern void ads114s08_set_input_mux(ads_device_t dev, uint8_t input_p, uint8_t input_n);
extern void ads114s08_drdy1_irq_handler(void);
extern void ads114s08_drdy2_irq_handler(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ADS114S08_H__ */
