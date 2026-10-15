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
#define ADS_INPUT_N_GND (0x0CU)
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum tag_ADS_DEVICE
{
    ADS_DEV_1 = 0U,
    ADS_DEV_2,
    ADS_DEV_MAX,
} ads_device_t;

typedef enum tag_ADS_DEV1_INPUT_MUX
{
    ADS_DEV1_AIN00_ID601_IOUT   = 0x00U, // 0x00U
    ADS_DEV1_AIN01_IC603_LDOOUT = 0x01U, // 0x01U
    ADS_DEV1_AIN02_ID601_ICC_P  = 0x02U, // 0x02U
    ADS_DEV1_AIN03_ID601_ICC_N  = 0x03U, // 0x03U
    ADS_DEV1_AIN04_IC603_ICC_P  = 0x04U, // 0x04U
    ADS_DEV1_AIN05_IC603_ICC_N  = 0x05U, // 0x05U
    ADS_DEV1_AIN06_IC603_DAC1   = 0x06U, // 0x06U
    ADS_DEV1_AIN07_IC603_DAC2   = 0x07U, // 0x07U
    ADS_DEV1_AIN08_IC603_DAC3   = 0x08U, // 0x08U
    ADS_DEV1_AIN09_IC603_2UA    = 0x09U, // 0x09U
    ADS_DEV1_AIN10_IC603_1V5    = 0x0AU, // 0x0AU
    ADS_DEV1_AIN11_NONE         = 0x0BU, // 0x0BU
} ads_dev1_input_mux_t;

typedef enum tag_ADS_DEV2_INPUT_MUX
{
    ADS_DEV2_AIN00_IC603_S1     = 0x00U, // 0x00U
    ADS_DEV2_AIN01_IC603_G1     = 0x01U, // 0x01U
    ADS_DEV2_AIN02_IC603_S2     = 0x02U, // 0x02U
    ADS_DEV2_AIN03_IC603_G2     = 0x03U, // 0x03U
    ADS_DEV2_AIN04_IC603_S3     = 0x04U, // 0x04U
    ADS_DEV2_AIN05_IC603_G3     = 0x05U, // 0x05U
    ADS_DEV2_AIN06_ID601_O1     = 0x06U, // 0x06U
    ADS_DEV2_AIN07_ID601_O2     = 0x07U, // 0x07U
    ADS_DEV2_AIN08_NONE         = 0x08U, // 0x08U
    ADS_DEV2_AIN09_NONE         = 0x09U, // 0x09U
    ADS_DEV2_AIN10_NONE         = 0x0AU, // 0x0AU
    ADS_DEV2_AIN11_NONE         = 0x0BU, // 0x0BU
} ads_dev2_input_mux_t;
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
extern int32_t ads114s08_get_average_conversion(ads_device_t dev);
extern float ads114s08_conversion_adc_to_mvoltage(uint32_t adc_value);

extern void ads114s08_init(void);

extern void ads114s08_set_input_mux(ads_device_t dev, uint8_t input_p, uint8_t input_n);

extern void ads114s08_drdy1_irq_handler(void);
extern void ads114s08_drdy2_irq_handler(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ADS114S08_H__ */
