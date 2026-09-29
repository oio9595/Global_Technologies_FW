/* USER CODE BEGIN Header */
/*
    * File:   drv_timer.h
    * Author: GT
    *
    * Created on 2026. 09. 07.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
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
extern void drv_tim_init(void);

extern void drv_tim_delay_us(uint32_t us);
extern void drv_tim_delay_ms(uint32_t ms);

extern bool drv_tim_generate_serial(uint16_t* p_duty, uint16_t length, uint16_t timeout);
extern void drv_tim_generate_serial_irq_handler(void);

extern bool drv_tim_capture_serial(uint16_t* p_rise, uint16_t* p_fall, uint16_t length, uint16_t timeout);
extern void drv_tim_capture_serial_rise_irq_handler(void);
extern void drv_tim_capture_serial_fall_irq_handler(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_TIMER_H__ */
