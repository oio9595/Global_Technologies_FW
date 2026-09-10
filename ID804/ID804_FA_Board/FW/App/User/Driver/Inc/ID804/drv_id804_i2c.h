/* USER CODE BEGIN Header */
/*
    * File:   drv_id804_i2c.h
    * Author: GT
    *
    * Created on 2026. 08. 31.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_ID804_I2C_H__
#define __DRV_ID804_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdbool.h>
#include <stdint.h>
/* 2. Project internal / System-related headers */
#include "id804_metadata.h"
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
extern id804_comm_result_t id804_write_via_i2c(uint16_t dev_addr, uint8_t reg_addr, uint32_t data);
extern id804_comm_result_t id804_read_via_i2c(uint16_t dev_addr, uint8_t reg_addr, uint32_t* p_data);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ID804_I2C_H__ */
