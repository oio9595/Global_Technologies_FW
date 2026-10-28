/* USER CODE BEGIN Header */
/*
    * File:   id601_dimming.h
    * Author: GT
    *
    * Created on 2026. 10. 27.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ID601_DIMMING_H__
#define __ID601_DIMMING_H__

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
extern uint16_t* id601_get_ld_buffer(void);
extern void id601_set_ld_buffer(uint16_t daisy, uint16_t ch, uint16_t led, uint16_t value);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __ID601_DIMMING_H__ */
