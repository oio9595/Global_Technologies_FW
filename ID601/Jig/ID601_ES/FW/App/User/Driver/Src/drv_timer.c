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
#include "drv_id601.h"
#include "cmsis_os.h"
#include "common_def.h"
#include "id601_dimming.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM1_PWM_DMA_BASE               (DMA2)
#define TIM1_PWM_DMA_STREAM             (LL_DMA_STREAM_1)

#define TIM2_CAPTURE_RISE               (0U)
#define TIM2_CAPTURE_RISE_DMA_BASE      (DMA1)
#define TIM2_CAPTURE_RISE_DMA_STREAM    (LL_DMA_STREAM_6)

#define TIM2_CAPTURE_FALL               (1U)
#define TIM2_CAPTURE_FALL_DMA_BASE      (DMA1)
#define TIM2_CAPTURE_FALL_DMA_STREAM    (LL_DMA_STREAM_5)

#define TIM5_CAPTURE_FREQ_DMA_BASE      (DMA1)
#define TIM5_CAPTURE_FREQ_DMA_STREAM    (LL_DMA_STREAM_2)

#define TIM5_CAPTURE_SIZE               (256U)
#define TIM5_CAPTURE_TIMEOUT            (1000U)

#define TIM3_FREQ                       (90000000UL) // 90MHz
#define TIM8_FREQ                       (180000000UL / 50U) // 3.6MHz

#define SVSYNC_PHASE_RED                (1U)
#define SVSYNC_PHASE_GREEN              (2U)
#define SVSYNC_PHASE_BLUE               (0U)

#define SVSYNC_VSYNC_MASK_H_US          (4U)
#define SVSYNC_VSYNC_MASK_L_US          (4U)
#define SVSYNC_GATING_TIME_US           (10U)
#define SVSYNC_CYCLE                    (3U)
#define SVSYNC_SIZE                     (ID601_SV_NO_DEFAULT)
#define SVSYNC_TOTAL_CYCLE              (SVSYNC_CYCLE * SVSYNC_SIZE)

#define SVSYNC_RED_FREQ                 (3840U)
#define SVSYNC_GREEN_FREQ               (6400U)
#define SVSYNC_BLUE_FREQ                (9600U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static volatile bool gb_tim1_pwm_dma_active = false;
static volatile bool gb_tim2_capture_dma_active[2] = {false, false};

static uint32_t gn_tim5_capture_ccr_buff[TIM5_CAPTURE_SIZE];
static volatile bool gb_tim5_capture_dma_active = false;

extern osThreadId_t VsyncTaskHandle;
extern osThreadId_t VsyncTestTaskHandle;

static uint16_t gn_svsync_count = 0U;
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
    LL_DMA_ClearFlag_TC6(TIM2_CAPTURE_RISE_DMA_BASE);
    LL_DMA_ClearFlag_TE6(TIM2_CAPTURE_RISE_DMA_BASE);

    LL_DMA_ClearFlag_TC5(TIM2_CAPTURE_FALL_DMA_BASE);
    LL_DMA_ClearFlag_TE5(TIM2_CAPTURE_FALL_DMA_BASE);

    LL_DMA_EnableIT_TC(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
    LL_DMA_EnableIT_TE(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
    LL_DMA_SetPeriphAddress(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM, (uint32_t)(&(TIM2->CCR2)));

    LL_DMA_EnableIT_TC(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
    LL_DMA_EnableIT_TE(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
    LL_DMA_SetPeriphAddress(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, (uint32_t)(&(TIM2->CCR1)));

    LL_DMA_SetChannelSelection(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, LL_DMA_CHANNEL_3);
    LL_DMA_SetDataTransferDirection(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_DisableFifoMode(TIM5_CAPTURE_FREQ_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);

    NVIC_SetPriority(DMA1_Stream5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(DMA1_Stream5_IRQn);

    LL_DMA_SetPeriphAddress(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM, (uint32_t)(&(TIM5->CCR1)));
    LL_DMA_SetMemoryAddress(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM, (uint32_t)gn_tim5_capture_ccr_buff);

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [MCU] Timer initialized successfully.");
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
    for (uint32_t i = 0U; i < ms; i++)
    {
        drv_tim_delay_us(1000U);
    }
}

bool drv_tim_generate_serial(uint16_t* p_cnt, uint16_t length, uint16_t timeout)
{
    if ((NULL == p_cnt) || (0U == length) || (0U == timeout))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [MCU] %s: Invalid count pointer or length or timeout.", __func__);
        return false;
    }
    drv_gpio_serialize_out_enable(true);

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
            drv_uart_printf(LOG_LV_ERROR, "\r\n    [MCU] TIM1 Generate Serialize timeout");
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
    drv_gpio_serialize_out_enable(false);

    LL_TIM_DisableCounter(TIM2);
    LL_TIM_SetCounter(TIM2, 0U);

    LL_TIM_DisableDMAReq_CC1(TIM2);
    LL_TIM_DisableDMAReq_CC2(TIM2);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_DMA_DisableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
    LL_DMA_SetMemoryAddress(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM, (uint32_t)(p_rise));
    LL_DMA_SetDataLength(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM, length);

    LL_DMA_ClearFlag_TC6(TIM2_CAPTURE_RISE_DMA_BASE);
    LL_DMA_ClearFlag_TE6(TIM2_CAPTURE_RISE_DMA_BASE);

    LL_DMA_DisableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
    LL_DMA_SetMemoryAddress(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, (uint32_t)(p_fall));
    LL_DMA_SetDataLength(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM, length);

    LL_DMA_ClearFlag_TC5(TIM2_CAPTURE_FALL_DMA_BASE);
    LL_DMA_ClearFlag_TE5(TIM2_CAPTURE_FALL_DMA_BASE);

    gb_tim2_capture_dma_active[TIM2_CAPTURE_RISE] = true;
    gb_tim2_capture_dma_active[TIM2_CAPTURE_FALL] = true;

    uint32_t start_time = HAL_GetTick();

    LL_TIM_EnableDMAReq_CC1(TIM2);
    LL_TIM_EnableDMAReq_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_DMA_EnableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
    LL_DMA_EnableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
    LL_TIM_EnableCounter(TIM2);

    while (true == gb_tim2_capture_dma_active[TIM2_CAPTURE_RISE] || true == gb_tim2_capture_dma_active[TIM2_CAPTURE_FALL])
    {
        if ((HAL_GetTick() - start_time) > timeout)
        {
            LL_DMA_DisableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
            LL_DMA_DisableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
            LL_TIM_DisableCounter(TIM2);
            gb_tim2_capture_dma_active[TIM2_CAPTURE_RISE] = false;
            gb_tim2_capture_dma_active[TIM2_CAPTURE_FALL] = false;
            drv_uart_printf(LOG_LV_ERROR, "\r\n    [MCU] TIM2 Capture Serialize timeout");
            return false;
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
    if (true == LL_DMA_IsActiveFlag_TC6(TIM2_CAPTURE_RISE_DMA_BASE))
    {
        LL_DMA_ClearFlag_TC6(TIM2_CAPTURE_RISE_DMA_BASE);
        LL_DMA_DisableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
        //LL_TIM_DisableCounter(TIM2);
        gb_tim2_capture_dma_active[TIM2_CAPTURE_RISE] = false;
    }

    if (true == LL_DMA_IsActiveFlag_TE6(TIM2_CAPTURE_RISE_DMA_BASE))
    {
        LL_DMA_ClearFlag_TE6(TIM2_CAPTURE_RISE_DMA_BASE);
        LL_DMA_DisableStream(TIM2_CAPTURE_RISE_DMA_BASE, TIM2_CAPTURE_RISE_DMA_STREAM);
        //LL_TIM_DisableCounter(TIM2);
        gb_tim2_capture_dma_active[TIM2_CAPTURE_RISE] = false;
    }
}

void drv_tim_capture_serial_fall_irq_handler(void)
{
    if (true == LL_DMA_IsActiveFlag_TC5(TIM2_CAPTURE_FALL_DMA_BASE))
    {
        LL_DMA_ClearFlag_TC5(TIM2_CAPTURE_FALL_DMA_BASE);
        LL_DMA_DisableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
        LL_TIM_DisableCounter(TIM2);
        gb_tim2_capture_dma_active[TIM2_CAPTURE_FALL] = false;
    }

    if (true == LL_DMA_IsActiveFlag_TE5(TIM2_CAPTURE_FALL_DMA_BASE))
    {
        LL_DMA_ClearFlag_TE5(TIM2_CAPTURE_FALL_DMA_BASE);
        LL_DMA_DisableStream(TIM2_CAPTURE_FALL_DMA_BASE, TIM2_CAPTURE_FALL_DMA_STREAM);
        LL_TIM_DisableCounter(TIM2);
        gb_tim2_capture_dma_active[TIM2_CAPTURE_FALL] = false;
    }
}

void mcu_tim_set_conversion_enable(bool b_start)
{
    if(true == b_start)
    {
        drv_gpio_counter_reset(false);
        LL_TIM_EnableDMAReq_CC1(TIM5);
        LL_TIM_CC_EnableChannel(TIM5, LL_TIM_CHANNEL_CH1);

        LL_DMA_EnableIT_TC(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM);
        LL_DMA_EnableIT_TE(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM);

        LL_DMA_SetDataLength(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM, TIM5_CAPTURE_SIZE);
        LL_DMA_EnableStream(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM);

        LL_TIM_EnableCounter(TIM5);
        gb_tim5_capture_dma_active = true;
    }
    else
    {
        drv_gpio_counter_reset(true);
        LL_TIM_DisableCounter(TIM5);

        LL_DMA_DisableStream(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM);

        LL_DMA_DisableIT_TC(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM);
        LL_DMA_DisableIT_TE(TIM5_CAPTURE_FREQ_DMA_BASE, TIM5_CAPTURE_FREQ_DMA_STREAM);

        LL_TIM_CC_DisableChannel(TIM5, LL_TIM_CHANNEL_CH1);
        LL_TIM_DisableDMAReq_CC1(TIM5);
        gb_tim5_capture_dma_active = false;
    }
}

bool mcu_tim_wait_conversion_complete(void)
{
    uint32_t start_tick = HAL_GetTick();
    while (true == gb_tim5_capture_dma_active)
    {
        if ((HAL_GetTick() - start_tick) > TIM5_CAPTURE_TIMEOUT)
        {
            drv_uart_printf(LOG_LV_ERROR, "\r\n    [MCU] TIM5 Capture Frequency timed out");
            mcu_tim_set_conversion_enable(false);
            return false;
        }
    }

    return true;
}

uint32_t mcu_tim_get_average_conversion(void)
{
    uint64_t tim_period_sum = 0U;
    const uint64_t counter_range = (uint64_t)LL_TIM_GetAutoReload(TIM5) + 1U;

    for (uint32_t i = 1U; i < TIM5_CAPTURE_SIZE; i++)
    {
        const uint32_t prev = gn_tim5_capture_ccr_buff[i - 1U];
        const uint32_t curr = gn_tim5_capture_ccr_buff[i];

        const uint64_t period = (curr >= prev) ? (uint64_t)(curr - prev) : counter_range - prev + curr;

        tim_period_sum += period;
    }

    return (uint32_t)(tim_period_sum / (TIM5_CAPTURE_SIZE - 1U));
}

float mcu_tim_conversion_cnt_to_frequency_Hz(uint32_t cnt_value)
{
    USER_ASSERT(0U != cnt_value);

    const uint32_t tim5_APB1_hz = 90000000U;
    const uint32_t tim5_prescaler_divisor = (LL_TIM_GetPrescaler(TIM5) + 1U);
    const float tim5_clock_hz = (float)tim5_APB1_hz / (float)tim5_prescaler_divisor;

    return tim5_clock_hz / (float)cnt_value;
}

void drv_tim_capture_freq_irq_handler(void)
{
    if (true == LL_DMA_IsActiveFlag_TC2(TIM5_CAPTURE_FREQ_DMA_BASE))
    {
        LL_DMA_ClearFlag_TC2(TIM5_CAPTURE_FREQ_DMA_BASE);
        mcu_tim_set_conversion_enable(false);
        gb_tim5_capture_dma_active = false;
    }

    if (true == LL_DMA_IsActiveFlag_TE2(TIM5_CAPTURE_FREQ_DMA_BASE))
    {
        LL_DMA_ClearFlag_TE2(TIM5_CAPTURE_FREQ_DMA_BASE);
        mcu_tim_set_conversion_enable(false);
        gb_tim5_capture_dma_active = false;
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [MCU] TIM5 Capture DMA Error!");
    }
}

void drv_tim_test_vsync_start(void)
{
    LL_TIM_SetCounter(TIM11, 0U);
    LL_TIM_EnableIT_UPDATE(TIM11);
    LL_TIM_EnableCounter(TIM11);
}
void drv_tim_test_vsync_stop(void)
{
    LL_TIM_DisableCounter(TIM11);
    LL_TIM_DisableIT_UPDATE(TIM11);
    LL_TIM_SetCounter(TIM11, 0U);
}
void drv_tim_test_vsync_irq_handler(void)
{
    if (true == LL_TIM_IsActiveFlag_UPDATE(TIM11))
    {
        LL_TIM_ClearFlag_UPDATE(TIM11);
        id601_sync_gen_wo_mutex(drv_id601_get_crc_func());
    }
}

static void drv_tim_svsync_start(void)
{
    LL_TIM_SetCounter(TIM3, 0);
    LL_TIM_ClearFlag_UPDATE(TIM3);
    LL_TIM_SetAutoReload(TIM3, (CONST_Hz_to_MHz(TIM3_FREQ) * (SVSYNC_VSYNC_MASK_H_US + SVSYNC_VSYNC_MASK_L_US) - 1U));
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableIT_UPDATE(TIM3);
    LL_TIM_OC_SetCompareCH1(TIM3, (CONST_Hz_to_MHz(TIM3_FREQ) * SVSYNC_VSYNC_MASK_H_US));
    LL_TIM_EnableCounter(TIM3);

    gn_svsync_count = 0U;
}
static void drv_tim_svsync_stop(void)
{
    LL_TIM_OC_SetCompareCH1(TIM3, 0U);
    LL_TIM_DisableCounter(TIM3);
    LL_TIM_DisableIT_UPDATE(TIM3);

    gn_svsync_count = 0U;
}
void drv_tim_svsync_irq_handler(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM3);
    uint32_t period = 0U;
    uint32_t CompareValue = 0U;

    ++gn_svsync_count;

    uint32_t phase = (gn_svsync_count % SVSYNC_CYCLE);

    switch(phase)
    {
        case SVSYNC_PHASE_RED:
        {
            period = ((TIM3_FREQ / SVSYNC_RED_FREQ) - 1U);
            CompareValue = (CONST_Hz_to_MHz(TIM3_FREQ) * SVSYNC_GATING_TIME_US);
            break;
        }
        case SVSYNC_PHASE_GREEN:
        {
            period = ((TIM3_FREQ / SVSYNC_GREEN_FREQ) - 1U);
            CompareValue = (CONST_Hz_to_MHz(TIM3_FREQ) * SVSYNC_GATING_TIME_US);
            break;
        }
        case SVSYNC_PHASE_BLUE:
        {
            period = ((TIM3_FREQ / SVSYNC_BLUE_FREQ) - 1U);
            CompareValue = (CONST_Hz_to_MHz(TIM3_FREQ) * SVSYNC_GATING_TIME_US);
            break;
        }
        default:
        {
            USER_ASSERT(false);
            break;
        }
    }

    LL_TIM_SetAutoReload(TIM3, period);
    LL_TIM_OC_SetCompareCH1(TIM3, CompareValue);

    if ((SVSYNC_TOTAL_CYCLE) == gn_svsync_count)
    {
        drv_tim_delay_us(SVSYNC_GATING_TIME_US + 2U); //SVSYNC_GATING_TIME_US + margin
        drv_tim_svsync_stop();
    }
}

void drv_tim_vsync_start(void)
{
    LL_TIM_SetCounter(TIM8, 0U);
    LL_TIM_OC_SetCompareCH2(TIM8, (CONST_Hz_to_MHz(TIM8_FREQ) * 10U)); // 10us
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_ClearFlag_UPDATE(TIM8);
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_EnableCounter(TIM8);
}
void drv_tim_vsync_stop(void)
{
    drv_tim_svsync_stop();
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_DisableIT_UPDATE(TIM8);
    LL_TIM_ClearFlag_UPDATE(TIM8);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_SetCounter(TIM8, 0U);
}
void drv_tim_vsync_irq_handler(void)
{
    if (true == LL_TIM_IsActiveFlag_UPDATE(TIM8))
    {
        LL_TIM_ClearFlag_UPDATE(TIM8);
        id601_sync_gen_wo_mutex(drv_id601_get_crc_func());
        drv_tim_svsync_start();
        osThreadFlagsSet(VsyncTaskHandle, VSYNC_DIMMING_FLAG);
    }
}
/* USER CODE END 0 */
