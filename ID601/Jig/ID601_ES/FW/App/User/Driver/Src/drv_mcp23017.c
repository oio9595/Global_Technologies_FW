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
#define MCP_I2C_ADDR   (0x21U << 1U)
#define I2C_Tx              (0U)
#define I2C_Rx              (1U)
#define I2C_TIMEOUT         (100U)

#define MCP_IODIRA          (0x00U)
#define MCP_IODIRB          (0x01U)
#define MCP_IPOLA           (0x02U)
#define MCP_IPOLB           (0x03U)
#define MCP_GPINTENA        (0x04U)
#define MCP_GPINTENB        (0x05U)
#define MCP_DEFVALA         (0x06U)
#define MCP_DEFVALB         (0x07U)
#define MCP_INTCONA         (0x08U)
#define MCP_INTCONB         (0x09U)
#define MCP_IOCONA          (0x0AU)
#define MCP_IOCONB          (0x0BU)
#define MCP_GPPUA           (0x0CU)
#define MCP_GPPUB           (0x0DU)
#define MCP_INTFA           (0x0EU)
#define MCP_INTFB           (0x0FU)
#define MCP_INTCAPA         (0x10U)
#define MCP_INTCAPB         (0x11U)
#define MCP_GPIOA           (0x12U)
#define MCP_GPIOB           (0x13U)
#define MCP_OLATA           (0x14U)
#define MCP_OLATB           (0x15U)
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

    ret = HAL_I2C_Master_Transmit(&hi2c1, (MCP_I2C_ADDR | I2C_Tx), i2c_tx_buff, 2U, I2C_TIMEOUT);
    return ret;
}

HAL_StatusTypeDef mcp23017_read(uint8_t reg, uint8_t* p_buff)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    uint8_t i2c_tx_buff[2] = { 0U };

    if (p_buff == NULL)
    {
        return HAL_ERROR;
    }

    i2c_tx_buff[0] = reg;

    ret = HAL_I2C_Master_Transmit(&hi2c1, (MCP_I2C_ADDR | I2C_Tx), i2c_tx_buff, 1U, I2C_TIMEOUT);
    if (HAL_OK != ret)
    {
        return ret;
    }

    ret = HAL_I2C_Master_Receive(&hi2c1, (MCP_I2C_ADDR | I2C_Rx), p_buff, 1U, I2C_TIMEOUT);
    if (HAL_OK != ret)
    {
        return ret;
    }

    return HAL_OK;
}

bool mcp23017_init(void)
{
    if (HAL_OK != mcp23017_write(MCP_IODIRA, 0x00))
    {
        return false;
    }
    if (HAL_OK != mcp23017_write(MCP_IODIRB, 0x00))
    {
        return false;
    }
    return true;
}

bool mcp23017_set_gpio_state(uint8_t pin, bool state)
{
    uint8_t reg = 0U;
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

    reg = (port == 0U) ? MCP_GPIOA : MCP_GPIOB;

    if (HAL_OK != mcp23017_write(reg, new_state))
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

bool mcp23017_get_gpio_state(uint8_t pin, bool* p_state)
{
    uint8_t port = 0U;
    uint8_t bit = 0U;
    uint8_t reg = 0U;
    uint8_t value = 0U;

    if ((pin >= MCP_GPIO_MAX) || (p_state == NULL))
    {
        return false;
    }

    port = pin >> 3U;       /* 0: GPIOA, 1: GPIOB */
    bit  = pin & 0x07U;     /* 각 포트 내 핀 번호: 0~7 */
    reg  = (port == 0U) ? MCP_GPIOA : MCP_GPIOB;

    if (HAL_OK != mcp23017_read(reg, &value))
    {
        return false;
    }

    gt_mcp_gpio_state[port] = value;
    *p_state = (value & (1U << bit)) ? true : false;

    return true;
}

/* USER CODE END 0 */
