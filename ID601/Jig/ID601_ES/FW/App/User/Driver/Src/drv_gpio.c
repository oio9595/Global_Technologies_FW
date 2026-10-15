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
#define POWER_STABLE_MS         (100)  // Delay in milliseconds for VDD ramp-up
#define RELAY_STABLE_MS         (10)  // Delay in milliseconds for relay stabilization
#define SPI_CS_SETUP_US         (5U)

#define ID601_VCC_EN_MCP_PORT   (MCP_GPA0)
#define ID601_VCC_5V5_MCP_PORT  (MCP_GPA1)
#define ID601_VLED_CTL_9V       (MCP_GPA2)
#define ID601_TEST_RELAY        (MCP_GPA3)
#define ID601_O1_RELAY          (MCP_GPA4)
#define ID601_O2_RELAY          (MCP_GPA5)
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
    mcp23017_init();

    drv_gpio_id601_vcc(ID601_VCC_OFF);
    drv_gpio_id601_vled(ID601_VLED_OFF);
    drv_gpio_id601_test_relay(ID601_RELAY_DIS);
    drv_gpio_id601_o1_relay(ID601_RELAY_DIS);
    drv_gpio_id601_o2_relay(ID601_RELAY_DIS);
    drv_gpio_id601_ch_demux(ID601_CH_MAX);
    drv_gpio_id601_iout_gain(ID601_IOUT_GAIN_MAX);

    drv_gpio_ic603_cs(false);
    drv_gpio_ads114s08_dev_all_cs(false);
}

bool drv_gpio_id601_vcc(uint16_t state)
{
    switch (state)
    {
        case ID601_VCC_OFF:
        {
            mcp23017_set_gpio_state(ID601_VCC_EN_MCP_PORT, true);
            mcp23017_set_gpio_state(ID601_VCC_5V5_MCP_PORT, false);
            break;
        }
        case ID601_VCC_3V3:
        {
            mcp23017_set_gpio_state(ID601_VCC_EN_MCP_PORT, false);
            mcp23017_set_gpio_state(ID601_VCC_5V5_MCP_PORT, false);
            break;
        }
        case ID601_VCC_5V5:
        {
            mcp23017_set_gpio_state(ID601_VCC_EN_MCP_PORT, false);
            mcp23017_set_gpio_state(ID601_VCC_5V5_MCP_PORT, true);
            break;
        }
        default:
        {
            mcp23017_set_gpio_state(ID601_VCC_EN_MCP_PORT, true);
            mcp23017_set_gpio_state(ID601_VCC_5V5_MCP_PORT, false);
            return false;
        }
    }
    drv_tim_delay_ms(POWER_STABLE_MS);
    return true;
}

bool drv_gpio_id601_vled(uint16_t state)
{
    switch (state)
    {
        case ID601_VLED_OFF:
        {
            mcp23017_set_gpio_state(ID601_VLED_CTL_9V, true);
            break;
        }
        case ID601_VLED_ON:
        {
            mcp23017_set_gpio_state(ID601_VLED_CTL_9V, false);
            break;
        }
        default:
        {
            mcp23017_set_gpio_state(ID601_VLED_CTL_9V, false);
            return false;
        }
    }
    drv_tim_delay_ms(POWER_STABLE_MS);
    return true;
}

bool drv_gpio_id601_test_relay(uint16_t state)
{
    switch (state)
    {
        case ID601_RELAY_DIS:
        {
            mcp23017_set_gpio_state(ID601_TEST_RELAY, false);
            break;
        }
        case ID601_RELAY_CON:
        {
            mcp23017_set_gpio_state(ID601_TEST_RELAY, true);
            break;
        }
        default:
        {
            mcp23017_set_gpio_state(ID601_TEST_RELAY, false);
            return false;
        }
    }
    drv_tim_delay_ms(RELAY_STABLE_MS);
    return true;
}

bool drv_gpio_id601_o1_relay(uint16_t state)
{
    switch (state)
    {
        case ID601_RELAY_DIS:
        {
            mcp23017_set_gpio_state(ID601_O1_RELAY, false);
            break;
        }
        case ID601_RELAY_CON:
        {
            mcp23017_set_gpio_state(ID601_O1_RELAY, true);
            break;
        }
        default:
        {
            mcp23017_set_gpio_state(ID601_O1_RELAY, false);
            return false;
        }
    }
    drv_tim_delay_ms(RELAY_STABLE_MS);
    return true;
}

bool drv_gpio_id601_o2_relay(uint16_t state)
{
    switch (state)
    {
        case ID601_RELAY_DIS:
        {
            mcp23017_set_gpio_state(ID601_O2_RELAY, false);
            break;
        }
        case ID601_RELAY_CON:
        {
            mcp23017_set_gpio_state(ID601_O2_RELAY, true);
            break;
        }
        default:
        {
            mcp23017_set_gpio_state(ID601_O2_RELAY, false);
            return false;
        }
    }
    drv_tim_delay_ms(RELAY_STABLE_MS);
    return true;
}

bool drv_gpio_id601_ch_demux(uint16_t ch)
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

bool drv_gpio_id601_iout_gain(uint16_t gain)
{
    static uint32_t last_gain = 0xFFFFFFFF;
    const uint32_t gain_val = (uint32_t)gain;
    if (last_gain == gain_val)
    {
        return true;
    }

    last_gain = gain_val;
    if (ID601_IOUT_GAIN_MAX <= gain_val)
    {
        LL_GPIO_ResetOutputPin(LTC_CURRENT_LOW_GPIO_Port, LTC_CURRENT_LOW_Pin);
        LL_GPIO_ResetOutputPin(LTC_CURRENT_MID_GPIO_Port, LTC_CURRENT_MID_Pin);
        LL_GPIO_ResetOutputPin(LTC_CURRENT_HIGH_GPIO_Port, LTC_CURRENT_HIGH_Pin);
        return false;
    }
    switch (gain_val)
    {
        case ID601_IOUT_GAIN_LOW:
        {
            LL_GPIO_SetOutputPin(LTC_CURRENT_LOW_GPIO_Port, LTC_CURRENT_LOW_Pin);
            LL_GPIO_ResetOutputPin(LTC_CURRENT_MID_GPIO_Port, LTC_CURRENT_MID_Pin);
            LL_GPIO_ResetOutputPin(LTC_CURRENT_HIGH_GPIO_Port, LTC_CURRENT_HIGH_Pin);
            break;
        }
        case ID601_IOUT_GAIN_MID:
        {
            LL_GPIO_ResetOutputPin(LTC_CURRENT_LOW_GPIO_Port, LTC_CURRENT_LOW_Pin);
            LL_GPIO_SetOutputPin(LTC_CURRENT_MID_GPIO_Port, LTC_CURRENT_MID_Pin);
            LL_GPIO_ResetOutputPin(LTC_CURRENT_HIGH_GPIO_Port, LTC_CURRENT_HIGH_Pin);
            break;
        }
        case ID601_IOUT_GAIN_HIGH:
        {
            LL_GPIO_ResetOutputPin(LTC_CURRENT_LOW_GPIO_Port, LTC_CURRENT_LOW_Pin);
            LL_GPIO_ResetOutputPin(LTC_CURRENT_MID_GPIO_Port, LTC_CURRENT_MID_Pin);
            LL_GPIO_SetOutputPin(LTC_CURRENT_HIGH_GPIO_Port, LTC_CURRENT_HIGH_Pin);
            break;
        }
        default:
        {
            LL_GPIO_ResetOutputPin(LTC_CURRENT_LOW_GPIO_Port, LTC_CURRENT_LOW_Pin);
            LL_GPIO_ResetOutputPin(LTC_CURRENT_MID_GPIO_Port, LTC_CURRENT_MID_Pin);
            LL_GPIO_ResetOutputPin(LTC_CURRENT_HIGH_GPIO_Port, LTC_CURRENT_HIGH_Pin);
            return false;
        }
    }
    drv_tim_delay_ms(POWER_STABLE_MS);
    return true;
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

bool drv_gpio_counter_reset(bool state)
{
    if (false == state)
    {
        LL_GPIO_ResetOutputPin(CNT_RST_GPIO_Port, CNT_RST_Pin);
    }
    else
    {
        LL_GPIO_SetOutputPin(CNT_RST_GPIO_Port, CNT_RST_Pin);
    }
    return true;
}

void drv_gpio_ic603_nINT_LD_irq_handler(void)
{

}

void drv_gpio_ic603_nINT_FAULT_irq_handler(void)
{

}
/* USER CODE END 0 */
