/* USER CODE BEGIN Header */
/**
     ******************************************************************************
    * @file           : drv_i2c.c
    * @brief          : I2C driver implementation
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
#include "drv_i2c.h"
/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ID804_I2C_ADDRESS   (0x58U << 1U)
#define I2C_TX              (0x00U)
#define I2C_RX              (0x01U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern I2C_HandleTypeDef hi2c1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drv_i2c_init(void)
{

}

/**
  * @brief  Write data to the I2C bus.
  * @param  p_data: Pointer to the data buffer to be transmitted.
  * @param  size: Number of bytes to transmit.
  * @retval true if the transmission was successful, false otherwise.
  */
bool drv_i2c_write(uint8_t* p_data, uint16_t size)
{
    bool ret = false;
    if (p_data != NULL && size > 0)
    {
        if (HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, (ID804_I2C_ADDRESS | I2C_TX), p_data, size, HAL_MAX_DELAY)) // order: DEV_ID + Register Address + Data MSB + Data LSB
        {
            drv_uart_printf("\r\nI2C write successful");
            ret = true;
        }
        else
        {
            drv_uart_printf("\r\nI2C write failed");
            ret = false;
        }
    }
    return ret;
}

/**
  * @brief  Read data from the I2C bus.
  * @param  p_data: Pointer to the data buffer to store the received data.
  * @param  size: Number of bytes to read.
  * @retval true if the reception was successful, false otherwise.
  */
bool drv_i2c_read(uint8_t* p_data, uint16_t size)
{
    bool ret = false;
    if (p_data != NULL && size > 0)
    {
        if (HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, (ID804_I2C_ADDRESS | I2C_TX), p_data, 2, HAL_MAX_DELAY)) // order: DEV_ID + Register Address
        {
            if (HAL_OK == HAL_I2C_Master_Receive(&hi2c1, (ID804_I2C_ADDRESS | I2C_RX), (p_data + 1U), size, HAL_MAX_DELAY)) // order: DEV_ID + Data MSB + Data LSB
            {
                drv_uart_printf("\r\nI2C read successful");
                ret = true;
            }
            else
            {
                drv_uart_printf("\r\nI2C read failed");
                ret = false;
            }
        }
        else
        {
            drv_uart_printf("\r\nI2C write failed");
            ret = false;
        }
    }
    return ret;
}
/* USER CODE END 0 */
