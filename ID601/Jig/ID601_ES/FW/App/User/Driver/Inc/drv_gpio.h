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
typedef enum tag_ID804_VCC_STATE
{
    ID804_VCC_OFF = 0U,
    ID804_VCC_5V0,
    ID804_VCC_5V5,
    ID804_VCC_MAX,
} id804_vcc_state_t;

typedef enum tag_ID804_VLED_STATE
{
    ID804_VLED_OFF = 0U,
    ID804_VLED_ON,
    ID804_VLED_MAX,
} id804_vled_state_t;

typedef enum tag_ID804_IO_STATE
{
    ID804_IO_DIS = 0U,
    ID804_IO_CON,
    ID804_IO_MAX,
} id804_io_state_t;

typedef enum tag_ID804_COMM_DIRECTION
{
    ID804_COMM_FORWARD = 0U,
    ID804_COMM_BACKWARD,
    ID804_COMM_MAX,
} id804_comm_direction_t;
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
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_GPIO_H__ */
