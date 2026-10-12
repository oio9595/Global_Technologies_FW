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

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define TRIM_START_FLAG (1U << 0)
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct tag_THREAD_DATA thread_data_t;
typedef bool (*trim_step_handler_t)(thread_data_t *td);

typedef void (*trim_init_func_t)(void);
typedef void (*trim_write_func_t)(uint16_t val);

struct tag_THREAD_DATA
{
    uint32_t step;
    uint32_t tout;
    trim_step_handler_t func;
};

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

void id601_trim_init(thread_data_t *td);
void id601_trim_deinit(thread_data_t *td);

extern bool id601_trim_process(thread_data_t *td);

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __ID601_TRIM_H__ */
