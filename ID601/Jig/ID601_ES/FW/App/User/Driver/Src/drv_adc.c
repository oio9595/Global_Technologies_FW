/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_adc.c
    * @brief          : ADC driver implementation
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
#include "drv_adc.h"
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
#define MCU_ADC_READ_COUNT    (16U)
#define MCU_ADC_TIMEOUT       (20U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static volatile bool gb_mcu_adc_conversion_done;
static uint32_t gn_mcu_adc_sum;
static uint16_t gn_mcu_conversion_count;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void mcu_adc_set_conversion_enable(bool b_start)
{
    if(true == b_start)
    {
        gn_mcu_adc_sum = 0U;
        gb_mcu_adc_conversion_done = false;
        gn_mcu_conversion_count = MCU_ADC_READ_COUNT;
        LL_GPIO_SetOutputPin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        LL_ADC_EnableIT_EOCS(ADC1);
        LL_ADC_Enable(ADC1);
        LL_ADC_REG_StartConversionSWStart(ADC1);
    }
    else
    {
        LL_ADC_Disable(ADC1);
        LL_ADC_DisableIT_EOCS(ADC1);
        LL_GPIO_ResetOutputPin(DEBUG1_GPIO_Port, DEBUG1_Pin);
    }
}

bool mcu_adc_wait_conversion_complete(void)
{
    uint32_t start_tick = HAL_GetTick();
    while (false == gb_mcu_adc_conversion_done)
    {
        if ((HAL_GetTick() - start_tick) > MCU_ADC_TIMEOUT)
        {
            drv_uart_printf("\r\nMCU ADC timeout!");
            return false;
        }
    }

    return true;
}

static uint16_t mcu_adc_read_conversion(void)
{
    return LL_ADC_REG_ReadConversionData12(ADC1);;
}

uint32_t mcu_adc_get_average_conversion(void)
{
    return ((uint32_t)((float)gn_mcu_adc_sum / MCU_ADC_READ_COUNT));
}

void mcu_adc_eoc_irq_handler(void)
{
    if (true == LL_ADC_IsActiveFlag_EOCS(ADC1))
    {
        LL_ADC_ClearFlag_EOCS(ADC1);
        LL_GPIO_TogglePin(DEBUG2_GPIO_Port, DEBUG2_Pin);
        gn_mcu_adc_sum += mcu_adc_read_conversion();
        --gn_mcu_conversion_count;

        if (0U == gn_mcu_conversion_count)
        {
            gb_mcu_adc_conversion_done = true;
            mcu_adc_set_conversion_enable(false);
        }
    }
}
/* USER CODE END 0 */
