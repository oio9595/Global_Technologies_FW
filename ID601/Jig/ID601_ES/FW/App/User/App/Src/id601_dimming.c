/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : id601_dimming.c
    * @brief          : id601_dimming implementation
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
#include "id601_dimming.h"
/* 2. C standard library headers (Alphabetical order) */
/* 3. Project internal / System-related headers */
#include "common_def.h"

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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint16_t gn_id601_ld_buffer[ID601_DAISY_LENGTH_MAX][ID601_CHANNEL_SIZE][ID601_LED_PER_CH_MAX];
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t* id601_get_ld_buffer(void)
{
    return &gn_id601_ld_buffer[0][0][0];
}
void id601_set_ld_buffer(uint16_t daisy, uint16_t ch, uint16_t led, uint16_t value)
{
    gn_id601_ld_buffer[daisy][ch][led] = value;
}

/* USER CODE END 0 */
