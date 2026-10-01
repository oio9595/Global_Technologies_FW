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
#include <stdbool.h>
/* 3. Project internal / System-related headers */
#include "drv_mcp23017.h"
#include "drv_timer.h"
#include "drv_uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POWER_STABLE_MS     (100)  // Delay in milliseconds for VDD ramp-up
#define SPI_CS_SETUP_US     (5U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern volatile bool g_ic603_nint_ld_flag;
extern volatile bool g_ic603_nint_fault_flag;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drv_gpio_init(void)
{

}

bool drv_gpio_id601_vcc(id601_vcc_state_t state)
{
    switch (state)
    {
        case ID601_VCC_OFF:
        {
            // Implement the logic to turn off VCC

            break;
        }
        case ID601_VCC_3V3:
        {
            // Implement the logic to set VCC to 5.0V
            break;
        }
        case ID601_VCC_5V5:
        {
            // Implement the logic to set VCC to 5.5V
            break;
        }
        default:
        {
            return false;
        }
    }
    drv_tim_delay_ms(POWER_STABLE_MS);
    return true;
}

bool drv_gpio_id601_vled(id601_vled_state_t state)
{
    switch (state)
    {
        case ID601_VLED_OFF:
        {
            // Implement the logic to turn off VLED

            break;
        }
        case ID601_VLED_ON:
        {
            // Implement the logic to turn on VLED
            break;
        }
        default:
        {
            return false;
        }
    }
    drv_tim_delay_ms(POWER_STABLE_MS);
    return true;
}

bool drv_gpio_id601_ch_demux(id601_ch_t ch)
{
    const uint32_t ch_val = (uint32_t)ch;
    if (ch_val >= ID601_CH_MAX)
    {
        LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
        LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
        LL_GPIO_SetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
        LL_GPIO_SetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
        return false;
    }

    if (0U == (ch_val & 0x01U))
    {
        LL_GPIO_ResetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
    }
    else
    {
        LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
    }

    if (0U == (ch_val & 0x02U))
    {
        LL_GPIO_ResetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
    }
    else
    {
        LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
    }

    if (0U == (ch_val & 0x04U))
    {
        LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
    }
    else
    {
        LL_GPIO_SetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
    }

    if (0U == (ch_val & 0x08U))
    {
        LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
    }
    else
    {
        LL_GPIO_SetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
    }
    drv_tim_delay_ms(POWER_STABLE_MS);
    return true;
#if 0
    switch (ch)
    {
        case ID601_CH_01:
        {
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_02:
        {
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_03:
        {
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_04:
        {
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_05:
        {
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_06:
        {
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_07:
        {
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_08:
        {
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_09:
        {
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_10:
        {
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_11:
        {
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        case ID601_CH_12:
        {
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_ResetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            break;
        }
        default:
        {
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX1_GPIO_Port, ID601_CH_DEMUX1_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX2_GPIO_Port, ID601_CH_DEMUX2_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX3_GPIO_Port, ID601_CH_DEMUX3_Pin);
            LL_GPIO_SetOutputPin(ID601_CH_DEMUX4_GPIO_Port, ID601_CH_DEMUX4_Pin);
            return false;
        }
    }
    return true;
#endif
}

bool drv_gpio_ic603_cs(bool state)
{
    if (true == state)
    {
        LL_GPIO_SetOutputPin(IC603_CS_GPIO_Port, IC603_CS_Pin);
        drv_tim_delay_us(SPI_CS_SETUP_US); // transition and delay
    }
    else
    {
        drv_tim_delay_us(SPI_CS_SETUP_US); // delay and transition
        LL_GPIO_ResetOutputPin(IC603_CS_GPIO_Port, IC603_CS_Pin);
    }
    return true;
}

bool drv_gpio_ads114s08_dev1_cs(bool state)
{
    if (true == state)
    {
        LL_GPIO_SetOutputPin(ADC_CS1_GPIO_Port, ADC_CS1_Pin);
        drv_tim_delay_us(SPI_CS_SETUP_US); // transition and delay
    }
    else
    {
        drv_tim_delay_us(SPI_CS_SETUP_US); // delay and transition
        LL_GPIO_ResetOutputPin(ADC_CS1_GPIO_Port, ADC_CS1_Pin);
    }
    return true;
}

bool drv_gpio_ads114s08_dev2_cs(bool state)
{
    if (true == state)
    {
        LL_GPIO_SetOutputPin(ADC_CS2_GPIO_Port, ADC_CS2_Pin);
        drv_tim_delay_us(SPI_CS_SETUP_US); // transition and delay
    }
    else
    {
        drv_tim_delay_us(SPI_CS_SETUP_US); // delay and transition
        LL_GPIO_ResetOutputPin(ADC_CS2_GPIO_Port, ADC_CS2_Pin);
    }
    return true;
}

bool drv_gpio_ads114s08_dev_all_cs(bool state)
{
    if (true == state)
    {
        LL_GPIO_SetOutputPin(ADC_CS1_GPIO_Port, ADC_CS1_Pin);
        LL_GPIO_SetOutputPin(ADC_CS2_GPIO_Port, ADC_CS2_Pin);
        drv_tim_delay_us(SPI_CS_SETUP_US); // transition and delay
    }
    else
    {
        drv_tim_delay_us(SPI_CS_SETUP_US); // delay and transition
        LL_GPIO_ResetOutputPin(ADC_CS1_GPIO_Port, ADC_CS1_Pin);
        LL_GPIO_ResetOutputPin(ADC_CS2_GPIO_Port, ADC_CS2_Pin);
    }
    return true;
}

bool drv_gpio_serialize_out_enable(bool state)
{
    if (true == state)
    {
        LL_GPIO_ResetOutputPin(BUFFER_OE_GPIO_Port, BUFFER_OE_Pin);
    }
    else
    {
        LL_GPIO_SetOutputPin(BUFFER_OE_GPIO_Port, BUFFER_OE_Pin);
    }
    return true;
}

void drv_gpio_ic603_nINT_LD_irq_handler(void)
{
    g_ic603_nint_ld_flag = true;
}

void drv_gpio_ic603_nINT_FAULT_irq_handler(void)
{
    g_ic603_nint_fault_flag = true;
}
/* USER CODE END 0 */
