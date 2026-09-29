/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_timer.c
    * @brief          : Timer driver implementation
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
#include "drv_timer.h"
/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_gpio.h"
#include "drv_uart.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM1_PWM_DMA_BASE               (DMA2)
#define TIM1_PWM_DMA_STREAM             (LL_DMA_STREAM_1)

#define TIM2_CAPTURE_RISE_DMA_BASE      (DMA1)
#define TIM2_CAPTURE_RISE_DMA_STREAM    (LL_DMA_STREAM_6)

#define TIM2_CAPTURE_FALL_DMA_BASE      (DMA1)
#define TIM2_CAPTURE_FALL_DMA_STREAM    (LL_DMA_STREAM_5)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static volatile bool gb_tim1_pwm_dma_active = false;
static volatile bool gb_tim2_capture_dma_active = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drv_tim_init(void)
{
    /* TIM1: Serialize PWM output */
    LL_DMA_ClearFlag_TC1(TIM1_PWM_DMA_BASE);
    LL_DMA_ClearFlag_TE1(TIM1_PWM_DMA_BASE);

    LL_TIM_EnableDMAReq_CC1(TIM1);
    LL_TIM_EnableAllOutputs(TIM1);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);

    LL_DMA_EnableIT_TC(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM);
    LL_DMA_EnableIT_TE(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM);
    LL_DMA_SetPeriphAddress(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM, (uint32_t)(&(TIM1->CCR1)));

    /* TIM2: Serialize PWM input */
    LL_DMA_ClearFlag_TC5(TIM2_CAPTURE_RISE_DMA_BASE);
    LL_DMA_ClearFlag_TE5(TIM2_CAPTURE_RISE_DMA_BASE);

    LL_DMA_ClearFlag_TC6(TIM2_CAPTURE_FALL_DMA_BASE);
    LL_DMA_ClearFlag_TE6(TIM2_CAPTURE_FALL_DMA_BASE);

    LL_DMA_EnableIT_TC(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
    LL_DMA_EnableIT_TE(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
    LL_DMA_SetPeriphAddress(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM, (uint32_t)(&(TIM2->CCR2)));

    LL_DMA_EnableIT_TC(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
    LL_DMA_EnableIT_TE(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
    LL_DMA_SetPeriphAddress(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, (uint32_t)(&(TIM2->CCR1)));

    LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_5, LL_DMA_CHANNEL_3);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_5, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_5, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_STREAM_5, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_5, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_5, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_5, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_5, LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_5);

    NVIC_SetPriority(DMA1_Stream5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

/**
 * @brief  Delay execution for a specified number of microseconds.
 * @param  us  The number of microseconds to delay.
 */
void drv_tim_delay_us(uint32_t us)
{
    TIM13->CNT = 0U;
    LL_TIM_EnableCounter(TIM13);
    while (TIM13->CNT < us)
    {
    }
    LL_TIM_DisableCounter(TIM13);
}

/**
 * @brief  Delay execution for a specified number of milliseconds.
 * @param  ms  The number of milliseconds to delay.
 */
void drv_tim_delay_ms(uint32_t ms)
{
    drv_tim_delay_us(ms * 1000U);
}

bool drv_tim_generate_serial(uint16_t* p_cnt, uint16_t length, uint16_t timeout)
{
    if ((NULL == p_cnt) || (0U == length) || (0U == timeout))
    {
        return false;
    }
    //drv_gpio_serialize_out_enable(true); // must be changed to true
    drv_gpio_serialize_out_enable(false); // only for testing

    LL_TIM_DisableCounter(TIM1);
    LL_DMA_DisableStream(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM);
    LL_TIM_SetCounter(TIM1, 0U);

    LL_DMA_SetMemoryAddress(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM, (uint32_t)(p_cnt));
    LL_DMA_SetDataLength(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM, length);

    LL_DMA_ClearFlag_TC1(TIM1_PWM_DMA_BASE);
    LL_DMA_ClearFlag_TE1(TIM1_PWM_DMA_BASE);

    gb_tim1_pwm_dma_active = true;

    uint32_t start_time = HAL_GetTick();

    LL_DMA_EnableStream(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM);
    LL_TIM_EnableCounter(TIM1);

    while (true == gb_tim1_pwm_dma_active)
    {
        if ((HAL_GetTick() - start_time) > timeout)
        {
            LL_DMA_DisableStream(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM);
            LL_TIM_DisableCounter(TIM1);
            gb_tim1_pwm_dma_active = false;
            drv_uart_printf("TIM1 Generate Serial timeout\r\n");
            return false;
        }
    }
    return true;
}

bool drv_tim_capture_serial(uint16_t* p_rise, uint16_t* p_fall, uint16_t length, uint16_t timeout)
{
    if ((NULL == p_rise) || (NULL == p_fall) || (0U == length) || (0U == timeout))
    {
        return false;
    }
    //drv_gpio_serialize_out_enable(false); // must be changed to true
    drv_gpio_serialize_out_enable(true); // only for testing

    LL_TIM_DisableCounter(TIM2);
    LL_TIM_SetCounter(TIM2, 0U);

    LL_TIM_DisableDMAReq_CC1(TIM2);
    LL_TIM_DisableDMAReq_CC2(TIM2);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_DMA_DisableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
    LL_DMA_SetMemoryAddress(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM, (uint32_t)(p_rise));
    LL_DMA_SetDataLength(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM, length);

    LL_DMA_ClearFlag_TC5(TIM2_CAPTURE_RISE_DMA_BASE);
    LL_DMA_ClearFlag_TE5(TIM2_CAPTURE_RISE_DMA_BASE);

    LL_DMA_DisableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
    LL_DMA_SetMemoryAddress(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, (uint32_t)(p_fall));
    LL_DMA_SetDataLength(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, length);

    LL_DMA_ClearFlag_TC6(TIM2_CAPTURE_FALL_DMA_BASE);
    LL_DMA_ClearFlag_TE6(TIM2_CAPTURE_FALL_DMA_BASE);

    gb_tim2_capture_dma_active = true;

    uint32_t start_time = HAL_GetTick();

    LL_TIM_EnableDMAReq_CC1(TIM2);
    LL_TIM_EnableDMAReq_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_DMA_EnableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
    LL_DMA_EnableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
    LL_TIM_EnableCounter(TIM2);

    while (true == gb_tim2_capture_dma_active)
    {
        if ((HAL_GetTick() - start_time) > timeout)
        {
            LL_DMA_DisableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
            LL_DMA_DisableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
            LL_TIM_DisableCounter(TIM2);
            gb_tim2_capture_dma_active = false;
            drv_uart_printf("TIM2 Capture Serial timeout\r\n");
            return false;
        }
    }
    return true;
}

bool drv_tim_decode_cnt_to_serial(uint16_t* p_rise, uint16_t* p_fall, uint16_t* p_serial, uint16_t length)
{
    if ((NULL == p_rise) || (NULL == p_fall) || (NULL == p_serial) || (0U == length))
    {
        return false;
    }
    uint16_t rise_cnt_sum = 0U;
    float freq_avg = 0.0f;
    for (uint16_t i = 1U; i < length; i++) // exclude the first element
    {
        rise_cnt_sum += p_rise[i];
    }
    freq_avg = (float)(rise_cnt_sum + 1U) / (float)(length - 1U); // exclude the first element

    const uint16_t cnt_half_duty = (uint16_t)(freq_avg / 2.0f);

    for (uint16_t i = 0U; i < length; i++)
    {
        if (p_rise[i] > cnt_half_duty)
        {
            p_serial[i] = 1U;
        }
        else
        {
            p_serial[i] = 0U;
        }
    }
    return true;
}

void drv_tim_generate_serial_irq_handler(void)
{
    if (true == LL_DMA_IsActiveFlag_TC1(TIM1_PWM_DMA_BASE))
    {
        LL_DMA_ClearFlag_TC1(TIM1_PWM_DMA_BASE);
        LL_DMA_DisableStream(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM);
        LL_TIM_DisableCounter(TIM1);
        gb_tim1_pwm_dma_active = false;
    }

    if (true == LL_DMA_IsActiveFlag_TE1(TIM1_PWM_DMA_BASE))
    {
        LL_DMA_ClearFlag_TE1(TIM1_PWM_DMA_BASE);
        LL_DMA_DisableStream(TIM1_PWM_DMA_BASE, TIM1_PWM_DMA_STREAM);
        LL_TIM_DisableCounter(TIM1);
        gb_tim1_pwm_dma_active = false;
    }
}

void drv_tim_capture_serial_rise_irq_handler(void)
{
    if (true == LL_DMA_IsActiveFlag_TC5(TIM2_CAPTURE_RISE_DMA_BASE))
    {
        LL_DMA_ClearFlag_TC5(TIM2_CAPTURE_RISE_DMA_BASE);
        LL_DMA_DisableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
        //LL_TIM_DisableCounter(TIM2);
        //gb_tim2_capture_dma_active = false;
    }

    if (true == LL_DMA_IsActiveFlag_TE5(TIM2_CAPTURE_RISE_DMA_BASE))
    {
        LL_DMA_ClearFlag_TE5(TIM2_CAPTURE_RISE_DMA_BASE);
        LL_DMA_DisableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
        //LL_TIM_DisableCounter(TIM2);
        //gb_tim2_capture_dma_active = false;
    }
}

void drv_tim_capture_serial_fall_irq_handler(void)
{
    if (true == LL_DMA_IsActiveFlag_TC6(TIM2_CAPTURE_FALL_DMA_BASE))
    {
        LL_DMA_ClearFlag_TC6(TIM2_CAPTURE_FALL_DMA_BASE);
        LL_DMA_DisableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
        LL_TIM_DisableCounter(TIM2);
        gb_tim2_capture_dma_active = false;
    }

    if (true == LL_DMA_IsActiveFlag_TE6(TIM2_CAPTURE_FALL_DMA_BASE))
    {
        LL_DMA_ClearFlag_TE6(TIM2_CAPTURE_FALL_DMA_BASE);
        LL_DMA_DisableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
        LL_TIM_DisableCounter(TIM2);
        gb_tim2_capture_dma_active = false;
    }
}
/* USER CODE END 0 */
