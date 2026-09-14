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
#include <stddef.h>
/* 3. Project internal / System-related headers */
#include "drv_i2c.h"
#include "drv_uart.h"
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
static inline bool id804_is_valid_address(uint8_t reg_addr)
{
    // 1. 일반 레지스터 영역 검증 (0x01 ~ 0x1A, 0x20 ~ 0x21)
    bool is_valid_reg = (reg_addr >= ID804_ADDR_STATUS1 && reg_addr <= ID804_ADDR_TIMEOUT) ||
                        (reg_addr >= ID804_ADDR_RGB_1   && reg_addr <= ID804_ADDR_RGB_2);

    // 2. OTP 영역 검증 (0xE0 ~ 0xF6)
    bool is_valid_otp = (reg_addr >= ID804_ADDR_OTP_START && reg_addr <= ID804_ADDR_OTP2_MIRROR19);

    return (is_valid_reg || is_valid_otp);
}

id804_comm_result_t id804_write_via_i2c(uint16_t dev_addr, uint8_t reg_addr, uint32_t data)
{
    (void)dev_addr;
    if (false == id804_is_valid_address(reg_addr))
    {
        return ID804_COMM_ERR_ADDR;
    }
    uint8_t i2c_buffer[ID804_I2C_WRITE_SIZE] = { reg_addr, (uint8_t)((data >> 8U) & 0xFFU), (uint8_t)(data & 0xFFU) };
    if (false == drv_i2c_write(i2c_buffer, ID804_I2C_WRITE_SIZE))
    {
        return ID804_COMM_ERR_I2C;
    }
    drv_uart_printf("\r\n    [ID804] [I2C Write] [Addr:0x%03X | Data:0x%03X]", reg_addr, data);
    return ID804_COMM_WRITE_OK;
}

id804_comm_result_t id804_read_via_i2c(uint16_t dev_addr, uint8_t reg_addr, uint32_t* p_data)
{
    (void)dev_addr;

    if ((NULL == p_data) || (false == id804_is_valid_address(reg_addr)))
    {
        return ID804_COMM_ERR_ADDR;
    }

    uint8_t i2c_buffer[ID804_I2C_READ_SIZE] = { reg_addr, 0U, 0U };
    if (false == drv_i2c_read(i2c_buffer, ID804_I2C_READ_SIZE - 1U))
    {
        return ID804_COMM_ERR_I2C;
    }
    else
    {
        *p_data = (uint16_t)((i2c_buffer[1] << 8U) | (i2c_buffer[2] << 0U));
    }
    drv_uart_printf("\r\n    [ID804] [I2C Read ] [Addr:0x%03X | Data:0x%03X]", reg_addr, *p_data);
    return ID804_COMM_READ_OK;
}
/* USER CODE END 0 */
