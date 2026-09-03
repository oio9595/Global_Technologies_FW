/* USER CODE BEGIN Header */
/*
    * File:   drv_i2c.h
    * Author: GT
    *
    * Created on 2026. 08. 28.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdbool.h>
#include <stdint.h>
/* 2. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern void drv_i2c_init(void);

extern bool drv_i2c_write(uint8_t* p_data, uint16_t size);
extern bool drv_i2c_read(uint8_t* p_data, uint16_t size);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_I2C_H__ */
