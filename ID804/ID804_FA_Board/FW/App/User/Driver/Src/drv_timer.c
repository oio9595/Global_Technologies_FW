/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_timer.c
    * @brief          : Timer driver implementation
    ******************************************************************************
    * @attention
    *
    * Copyright (c) 2026 Global Technologies.
    * All rights reserved.
    *
    ******************************************************************************
    */
/* USER CODE END Header */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. Direct pairing header (Corresponding header for this source file) */
#include "drv_timer.h"
/* 2. C standard library headers (Alphabetical order) */
#include "main.h"
/* 3. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drv_timer_delay_us(uint32_t us)
{
    TIM13->CNT = 0U;
    LL_TIM_EnableCounter(TIM13);
    while (TIM13->CNT < us)
    {
    }
    LL_TIM_DisableCounter(TIM13);
}

void drv_timer_delay_ms(uint32_t ms)
{
    drv_timer_delay_us(ms * 1000U);
}

/* USER CODE END 0 */
