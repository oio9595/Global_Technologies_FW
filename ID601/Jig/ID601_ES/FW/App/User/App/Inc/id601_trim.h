/* USER CODE BEGIN Header */
/*
    * File:   id601_trim.h
    * Author: GT
    *
    * Created on 2026. 09. 22.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ID601_TRIM_H__
#define __ID601_TRIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* 2. Project internal / System-related headers */
#include "common_def.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef thread_step_handler_t trim_step_handler_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern void id601_set_efuse_burn_enable(bool enable);
extern bool id601_get_efuse_burn_enable(void);

extern void id601_trim_clear_parameters(thread_data_t *td);
extern void id601_trim_start(thread_data_t *td);
extern bool id601_trim_process(thread_data_t *td);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __ID601_TRIM_H__ */
