/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_gpio.c
    * @brief          : GPIO driver implementation
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
#include "drv_gpio.h"

/* 2. C standard library headers (Alphabetical order) */
#include <stdbool.h>
/* 3. Project internal / System-related headers */
#include "main.h"
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
void drv_gpio_init(void)
{

}

bool drv_gpio_id804_vcc(id804_vcc_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_VCC_OFF:
        {
            LL_GPIO_SetOutputPin(ID804_VCC_EN_GPIO_Port, ID804_VCC_EN_Pin);
            LL_GPIO_ResetOutputPin(ID804_VCC_LED_GPIO_Port, ID804_VCC_LED_Pin);
            LL_GPIO_ResetOutputPin(ID804_VCC_5V5_GPIO_Port, ID804_VCC_5V5_Pin);
            result = true;
            break;
        }
        case ID804_VCC_5V0:
        {
            LL_GPIO_ResetOutputPin(ID804_VCC_EN_GPIO_Port, ID804_VCC_EN_Pin);
            LL_GPIO_SetOutputPin(ID804_VCC_LED_GPIO_Port, ID804_VCC_LED_Pin);
            LL_GPIO_ResetOutputPin(ID804_VCC_5V5_GPIO_Port, ID804_VCC_5V5_Pin);
            result = true;
            break;
        }
        case ID804_VCC_5V5:
        {
            LL_GPIO_ResetOutputPin(ID804_VCC_EN_GPIO_Port, ID804_VCC_EN_Pin);
            LL_GPIO_SetOutputPin(ID804_VCC_LED_GPIO_Port, ID804_VCC_LED_Pin);
            LL_GPIO_SetOutputPin(ID804_VCC_5V5_GPIO_Port, ID804_VCC_5V5_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_SetOutputPin(ID804_VCC_EN_GPIO_Port, ID804_VCC_EN_Pin);
            LL_GPIO_ResetOutputPin(ID804_VCC_LED_GPIO_Port, ID804_VCC_LED_Pin);
            LL_GPIO_ResetOutputPin(ID804_VCC_5V5_GPIO_Port, ID804_VCC_5V5_Pin);
            result = false;
            break;
        }
    }
    return result;
}

bool drv_gpio_id804_vled(id804_vled_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_VLED_OFF:
        {
            LL_GPIO_SetOutputPin(ID804_VLED_EN_GPIO_Port, ID804_VLED_EN_Pin);
            result = true;
            break;
        }
        case ID804_VLED_ON:
        {
            LL_GPIO_ResetOutputPin(ID804_VLED_EN_GPIO_Port, ID804_VLED_EN_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_SetOutputPin(ID804_VLED_EN_GPIO_Port, ID804_VLED_EN_Pin);
            result = false;
            break;
        }
    }
    return result;
}

bool drv_gpio_id804_io_clear(void)
{
    LL_GPIO_ResetOutputPin(MCU_TM0_GPIO_Port, MCU_TM0_Pin);
    LL_GPIO_ResetOutputPin(I2C_SIO1P_UP_GPIO_Port, I2C_SIO1P_UP_Pin);
    LL_GPIO_ResetOutputPin(I2C_SIO1N_UP_GPIO_Port, I2C_SIO1N_UP_Pin);
    LL_GPIO_ResetOutputPin(SIO1_MCU_GPIO_Port, SIO1_MCU_Pin);
    LL_GPIO_ResetOutputPin(SIO1_CAN_GPIO_Port, SIO1_CAN_Pin);
    LL_GPIO_ResetOutputPin(SIO1_LVDS_GPIO_Port, SIO1_LVDS_Pin);
    LL_GPIO_ResetOutputPin(SIO2_MCU_GPIO_Port, SIO2_MCU_Pin);
    LL_GPIO_ResetOutputPin(SIO2_CAN_GPIO_Port, SIO2_CAN_Pin);
    LL_GPIO_ResetOutputPin(SIO2_LVDS_GPIO_Port, SIO2_LVDS_Pin);
    return true;
}

bool drv_gpio_id804_tm0_to_mcu(void)
{
    LL_GPIO_ResetOutputPin(MCU_TM0_GPIO_Port, MCU_TM0_Pin);
    return true;
}

bool drv_gpio_id804_tm0_to_i2c(void)
{
    LL_GPIO_SetOutputPin(MCU_TM0_GPIO_Port, MCU_TM0_Pin);
    return true;
}

bool drv_gpio_id804_sio1_i2c(id804_io_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_IO_DIS:
        {
            LL_GPIO_ResetOutputPin(I2C_SIO1P_UP_GPIO_Port, I2C_SIO1P_UP_Pin);
            LL_GPIO_ResetOutputPin(I2C_SIO1N_UP_GPIO_Port, I2C_SIO1N_UP_Pin);
            result = true;
            break;
        }
        case ID804_IO_CON:
        {
            LL_GPIO_SetOutputPin(I2C_SIO1P_UP_GPIO_Port, I2C_SIO1P_UP_Pin);
            LL_GPIO_SetOutputPin(I2C_SIO1N_UP_GPIO_Port, I2C_SIO1N_UP_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_ResetOutputPin(I2C_SIO1P_UP_GPIO_Port, I2C_SIO1P_UP_Pin);
            LL_GPIO_ResetOutputPin(I2C_SIO1N_UP_GPIO_Port, I2C_SIO1N_UP_Pin);
            result = false;
            break;
        }
    }
    return result;
}

bool drv_gpio_id804_sio1_mcu(id804_io_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_IO_DIS:
        {
            LL_GPIO_ResetOutputPin(SIO1_MCU_GPIO_Port, SIO1_MCU_Pin);
            result = true;
            break;
        }
        case ID804_IO_CON:
        {
            LL_GPIO_SetOutputPin(SIO1_MCU_GPIO_Port, SIO1_MCU_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_ResetOutputPin(SIO1_MCU_GPIO_Port, SIO1_MCU_Pin);
            result = false;
            break;
        }
    }
    return result;
}

bool drv_gpio_id804_sio1_can(id804_io_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_IO_DIS:
        {
            LL_GPIO_ResetOutputPin(SIO1_CAN_GPIO_Port, SIO1_CAN_Pin);
            result = true;
            break;
        }
        case ID804_IO_CON:
        {
            LL_GPIO_SetOutputPin(SIO1_CAN_GPIO_Port, SIO1_CAN_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_ResetOutputPin(SIO1_CAN_GPIO_Port, SIO1_CAN_Pin);
            result = false;
            break;
        }
    }
    return result;
}

bool drv_gpio_id804_sio1_lvds(id804_io_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_IO_DIS:
        {
            LL_GPIO_ResetOutputPin(SIO1_LVDS_GPIO_Port, SIO1_LVDS_Pin);
            result = true;
            break;
        }
        case ID804_IO_CON:
        {
            LL_GPIO_SetOutputPin(SIO1_LVDS_GPIO_Port, SIO1_LVDS_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_ResetOutputPin(SIO1_LVDS_GPIO_Port, SIO1_LVDS_Pin);
            result = false;
            break;
        }
    }
    return result;
}


bool drv_gpio_id804_sio2_mcu(id804_io_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_IO_DIS:
        {
            LL_GPIO_ResetOutputPin(SIO2_MCU_GPIO_Port, SIO2_MCU_Pin);
            result = true;
            break;
        }
        case ID804_IO_CON:
        {
            LL_GPIO_SetOutputPin(SIO2_MCU_GPIO_Port, SIO2_MCU_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_ResetOutputPin(SIO2_MCU_GPIO_Port, SIO2_MCU_Pin);
            result = false;
            break;
        }
    }
    return result;
}

bool drv_gpio_id804_sio2_can(id804_io_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_IO_DIS:
        {
            LL_GPIO_ResetOutputPin(SIO2_CAN_GPIO_Port, SIO2_CAN_Pin);
            result = true;
            break;
        }
        case ID804_IO_CON:
        {
            LL_GPIO_SetOutputPin(SIO2_CAN_GPIO_Port, SIO2_CAN_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_ResetOutputPin(SIO2_CAN_GPIO_Port, SIO2_CAN_Pin);
            result = false;
            break;
        }
    }
    return result;
}

bool drv_gpio_id804_sio2_lvds(id804_io_state_t state)
{
    bool result = false;
    switch (state)
    {
        case ID804_IO_DIS:
        {
            LL_GPIO_ResetOutputPin(SIO2_LVDS_GPIO_Port, SIO2_LVDS_Pin);
            result = true;
            break;
        }
        case ID804_IO_CON:
        {
            LL_GPIO_SetOutputPin(SIO2_LVDS_GPIO_Port, SIO2_LVDS_Pin);
            result = true;
            break;
        }
        default:
        {
            LL_GPIO_ResetOutputPin(SIO2_LVDS_GPIO_Port, SIO2_LVDS_Pin);
            result = false;
            break;
        }
    }
    return result;
}
/* USER CODE END 0 */
