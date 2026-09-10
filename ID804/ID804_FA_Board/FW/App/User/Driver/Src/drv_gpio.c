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
#include <stdint.h>
/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_uart.h"
#include "drv_spi.h"
#include "drv_timer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DELAY_RELAY_OPERATE     (5U)
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
    drv_gpio_id804_io_clear();
}

void drv_gpio_id804_boot_i2c(void)
{
    drv_gpio_id804_tm0_to_GND();
    drv_gpio_id804_tm1_to_GND();

    drv_gpio_id804_sio1_mcu(ID804_IO_CON);
    drv_gpio_id804_sio2_eol(ID804_IO_CON);

    drv_gpio_id804_vcc(ID804_VCC_5V0);

    drv_timer_delay_ms(DELAY_POWER_UP_MS);

    drv_gpio_id804_tm0_to_VDD();
    drv_gpio_id804_sio1_mcu(ID804_IO_DIS);
    drv_gpio_id804_sio1_i2c(ID804_IO_CON);

    drv_gpio_id804_vled(ID804_VLED_ON);
}

void drv_gpio_id804_boot_mcu(void)
{
    drv_gpio_id804_tm0_to_GND();
    drv_gpio_id804_tm1_to_GND();

    drv_gpio_id804_sio1_mcu(ID804_IO_CON);
    drv_gpio_id804_sio2_eol(ID804_IO_CON);

    drv_gpio_id804_vcc(ID804_VCC_5V0);

    drv_timer_delay_ms(DELAY_POWER_UP_MS);

    drv_gpio_id804_vled(ID804_VLED_ON);
}

bool drv_gpio_id804_io_clear(void)
{
    drv_gpio_id804_vcc(ID804_VCC_OFF);
    drv_gpio_id804_vled(ID804_VLED_OFF);

    drv_gpio_id804_tm0_to_GND();

    drv_gpio_id804_sio1_i2c(ID804_IO_DIS);
    drv_gpio_id804_sio1_mcu(ID804_IO_DIS);
    drv_gpio_id804_sio1_can(ID804_IO_DIS);
    drv_gpio_id804_sio1_lvds(ID804_IO_DIS);

    drv_gpio_id804_sio2_mcu(ID804_IO_DIS);
    drv_gpio_id804_sio2_can(ID804_IO_DIS);
    drv_gpio_id804_sio2_lvds(ID804_IO_DIS);
    drv_gpio_id804_sio2_eol(ID804_IO_DIS);
    return true;
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

bool drv_gpio_id804_tm0_to_GND(void)
{
    LL_GPIO_ResetOutputPin(MCU_TM0_GPIO_Port, MCU_TM0_Pin);
    return true;
}

bool drv_gpio_id804_tm0_to_VDD(void)
{
    LL_GPIO_SetOutputPin(MCU_TM0_GPIO_Port, MCU_TM0_Pin);
    return true;
}

bool drv_gpio_id804_tm1_to_GND(void)
{
    LL_GPIO_ResetOutputPin(MCU_TM1_GPIO_Port, MCU_TM1_Pin);
    return true;
}

bool drv_gpio_id804_tm1_to_VDD(void)
{
    LL_GPIO_SetOutputPin(MCU_TM1_GPIO_Port, MCU_TM1_Pin);
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
    drv_timer_delay_ms(DELAY_RELAY_OPERATE);
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
    drv_timer_delay_ms(DELAY_RELAY_OPERATE);
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
    drv_timer_delay_ms(DELAY_RELAY_OPERATE);
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
    drv_timer_delay_ms(DELAY_RELAY_OPERATE);
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
    drv_timer_delay_ms(DELAY_RELAY_OPERATE);
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
    drv_timer_delay_ms(DELAY_RELAY_OPERATE);
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
    drv_timer_delay_ms(DELAY_RELAY_OPERATE);
    return result;
}

bool drv_gpio_id804_sio2_eol(id804_io_state_t state)
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
            drv_uart_printf("\r\n    You should check J26 (EOL/CAN)");
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
    drv_timer_delay_ms(DELAY_RELAY_OPERATE);
    return result;
}
/* USER CODE END 0 */
