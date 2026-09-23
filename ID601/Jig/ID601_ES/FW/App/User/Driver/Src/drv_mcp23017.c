/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_mcp23017.c
    * @brief          : MCP23017 driver implementation
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
#include "drv_mcp23017.h"
/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_TIMEOUT 100U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern I2C_HandleTypeDef hi2c1;
static uint8_t gt_mcp_gpio_state[2];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
HAL_StatusTypeDef mcp23017_write(uint8_t reg, uint8_t data)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    uint8_t i2c_tx_buff[2] = { 0U };

    i2c_tx_buff[0] = reg;
    i2c_tx_buff[1] = data;

    return HAL_I2C_Master_Transmit(&hi2c1, (MCP23017_I2C_ADDR | I2C_Tx), i2c_tx_buff, 2U, I2C_TIMEOUT);
}

HAL_StatusTypeDef mcp23017_read(uint8_t reg, uint8_t* p_buff)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    uint8_t i2c_tx_buff[2] = { 0U };

    i2c_tx_buff[0] = reg;

    ret = HAL_I2C_Master_Transmit(&hi2c1, (MCP23017_I2C_ADDR | I2C_Tx), i2c_tx_buff, 1U, I2C_TIMEOUT);
    if (HAL_OK != ret)
    {
        return ret;
    }

    ret = HAL_I2C_Master_Receive(&hi2c1, (MCP23017_I2C_ADDR | I2C_Rx), p_buff, 1U, I2C_TIMEOUT);
    if (HAL_OK != ret)
    {
        return ret;
    }

    return HAL_OK;
}

void mcp23017_init(void)
{
    mcp23017_write(MCP_IODIRA, 0x00);
    mcp23017_write(MCP_IODIRB, 0x00);
}

bool mcp23017_set_gpio_state(uint8_t pin, bool state)
{
    uint8_t i2c_tx_buff[2] = { 0U };
    uint8_t port = 0U;
    uint8_t bit = 0U;
    uint8_t new_state = 0U;
    bool ret = false;

    if (pin >= MCP_GPIO_MAX)
    {
        return false;
    }

    port = pin >> 3U;       /* 0: GPIOA, 1: GPIOB */
    bit  = pin & 0x07U;     /* 각 포트 내 핀 번호: 0~7 */

    if (state)
    {
        new_state = (uint8_t)(gt_mcp_gpio_state[port] | (uint8_t)(1U << bit));
    }
    else
    {
        new_state = (uint8_t)(gt_mcp_gpio_state[port] & (uint8_t)~(1U << bit));
    }

    i2c_tx_buff[0] = (port == 0U) ? MCP_GPIOA : MCP_GPIOB;
    i2c_tx_buff[1] = new_state;

    if (HAL_OK != mcp23017_write(i2c_tx_buff[0], i2c_tx_buff[1]))
    {
        return false;
    }
    else
    {
        gt_mcp_gpio_state[port] = new_state;
        ret = true;
    }
    return ret;
}

uint8_t mcp23017_get_gpio_state(uint8_t pin)
{
    uint8_t port = 0U;
    uint8_t bit = 0U;
    uint8_t reg = 0U;
    uint8_t value = 0U;
    HAL_StatusTypeDef ret = HAL_ERROR;

    if (pin >= MCP_GPIO_MAX)
    {
        return 0U;
    }

    port = pin >> 3U;       /* 0: GPIOA, 1: GPIOB */
    bit  = pin & 0x07U;     /* 각 포트 내 핀 번호: 0~7 */
    reg  = (port == 0U) ? MCP_GPIOA : MCP_GPIOB;

    if (HAL_OK != mcp23017_read(reg, &value))
    {
        return 0U;
    }

    gt_mcp_gpio_state[port] = value;

    return value;
}

/* USER CODE END 0 */
