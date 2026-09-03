/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_id804_i2c.c
    * @brief          : ID804_I2C driver implementation
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
#include "drv_id804_i2c.h"
/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */
#include "drv_i2c.h"
#include "id804_metadata.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ID804_I2C_WRITE_SIZE  (3U)
#define ID804_I2C_READ_SIZE   (3U)
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
void id804_write_data_via_i2c(uint8_t reg_addr, uint16_t data)
{
    uint8_t i2c_buffer[ID804_I2C_WRITE_SIZE] = { reg_addr, (uint8_t)((data >> 8U) & 0xFFU), (uint8_t)(data & 0xFFU) };
    (void)drv_i2c_write(i2c_buffer, ID804_I2C_WRITE_SIZE);
}

uint16_t id804_read_data_via_i2c(uint8_t reg_addr)
{
    uint16_t regVal = 0U;
    uint8_t i2c_buffer[ID804_I2C_READ_SIZE] = { reg_addr, 0U, 0U };
    if (false == drv_i2c_read(i2c_buffer, ID804_I2C_READ_SIZE))
    {
        return 0U;
    }
    regVal = (uint16_t)((i2c_buffer[1] << 8U) | (i2c_buffer[2] << 0U));
    return regVal;
}

/* USER CODE END 0 */
