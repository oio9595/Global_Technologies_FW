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
#define I2C_TIMEOUT_MS      (10U)

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
 * @brief  Print I2C error message based on the error code.
 * @param  err_code: I2C error code.
 * @retval None.
 */
static void drv_i2c_print_error(uint32_t err_code)
{
    switch (err_code)
    {
        case HAL_I2C_ERROR_BERR:
            drv_uart_printf(" (I2C Bus Error)");
            break;
        case HAL_I2C_ERROR_ARLO:
            drv_uart_printf(" (I2C Arbitration Lost)");
            break;
        case HAL_I2C_ERROR_AF:
            drv_uart_printf(" (I2C Acknowledge Failure)");
            break;
        case HAL_I2C_ERROR_OVR:
            drv_uart_printf(" (I2C Overrun/Underrun)");
            break;
        case HAL_I2C_ERROR_TIMEOUT:
            drv_uart_printf(" (I2C Timeout)");
            break;
        default:
            drv_uart_printf(" (I2C Unknown Error 0x%X)", err_code);
            break;
    }
}

/**
  * @brief  Write data to the I2C bus.
  * @param  p_data: Pointer to the data buffer to be transmitted.
  * @param  size: Number of bytes to transmit.
  * @retval true if the transmission was successful, false otherwise.
  */
bool drv_i2c_write(uint8_t* p_data, uint16_t size)
{
if (p_data == NULL || size == 0U)
    {
        return false;
    }

    if (HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, (ID804_I2C_ADDRESS | I2C_TX), p_data, size, I2C_TIMEOUT_MS))
    {
        drv_uart_printf("\r\n    I2C write successful");
        return true;
    }

    drv_uart_printf("\r\n    I2C write failed");
    drv_i2c_print_error(hi2c1.ErrorCode);
    return false;
}

/**
  * @brief  Read data from the I2C bus.
  * @param  p_data: Pointer to the data buffer to store the received data.
  * @param  size: Number of bytes to read.
  * @retval true if the reception was successful, false otherwise.
  */
bool drv_i2c_read(uint8_t* p_data, uint16_t size)
{
    if (p_data == NULL || size == 0)
    {
        return false;
    }

    // 1. 레지스터 주소 전송 (Write)
    if (HAL_OK != HAL_I2C_Master_Transmit(&hi2c1, (ID804_I2C_ADDRESS | I2C_TX), p_data, 1U, I2C_TIMEOUT_MS))
    {
        drv_uart_printf("\r\n    I2C write failed");
        drv_i2c_print_error(hi2c1.ErrorCode);
        return false;
    }

    // 2. 데이터 수신 (Read)
    if (HAL_OK != HAL_I2C_Master_Receive(&hi2c1, (ID804_I2C_ADDRESS | I2C_RX), p_data + 1U, size, I2C_TIMEOUT_MS))
    {
        drv_uart_printf("\r\n    I2C read failed");
        drv_i2c_print_error(hi2c1.ErrorCode);
        return false;
    }

    drv_uart_printf("\r\n    I2C read successful");
    return true;
}
/* USER CODE END 0 */
