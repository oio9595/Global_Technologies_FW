/* USER CODE BEGIN Header */
/*
    * File:   drv_id601.h
    * Author: GT
    *
    * Created on 2026. 09. 22.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_ID601_H__
#define __DRV_ID601_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */
#include "id601_metadata.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef uint8_t (*id601_crc_func_t)(uint32_t, uint16_t);

typedef enum tag_ID601_REGISTER_BANK
{
    ID601_REGISTER_BANK_GENERAL = 0U,
    ID601_REGISTER_BANK_MIRROR,
    ID601_REGISTER_BANK_COUNT,
} id601_register_bank_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern void drv_id601_init(void);

extern bool id601_write_register(id601_register_bank_t bank, uint16_t addr, const uint16_t* p_value, id601_crc_func_t p_crc_func);
extern bool id601_read_register(id601_register_bank_t bank, uint16_t addr, uint16_t* p_value, id601_crc_func_t p_crc_func);
extern bool id601_reset(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ID601_H__ */
