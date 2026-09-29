/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_id601.c
    * @brief          : ID601 driver implementation
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
#include "drv_id601.h"
/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_timer.h"
#include "drv_uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ID601_SERIAL_TIMEOUT_MS         (100U)

#define ID601_SERIAL_RATIO_BIT0               (1U)
#define ID601_SERIAL_RATIO_BIT1               (2U)
#define ID601_SERIAL_RATIO_SUM                (ID601_SERIAL_RATIO_BIT0 + ID601_SERIAL_RATIO_BIT1)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static volatile uint16_t rise[21] = { 0U };
static volatile uint16_t fall[21] = { 0U };

static uint16_t gn_tim1_period;
static uint16_t gn_id601_serial_bit0_ccr;
static uint16_t gn_id601_serial_bit1_ccr;

static uint16_t const gn_id601_daisy_length = 1U;

static _id601_general_regs_t gt_id601_general_regs;
static _id601_mirror_regs_t gt_id601_mirror_regs;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drv_id601_init(void)
{
    gn_tim1_period = LL_TIM_GetAutoReload(TIM1);
    gn_id601_serial_bit0_ccr = ((gn_tim1_period * ID601_SERIAL_RATIO_BIT0) / ID601_SERIAL_RATIO_SUM);
    gn_id601_serial_bit1_ccr = ((gn_tim1_period * ID601_SERIAL_RATIO_BIT1) / ID601_SERIAL_RATIO_SUM);
}

uint16_t id601_build_serial_ccr_buffer(uint32_t serial, uint16_t* p_cnt, uint16_t length)
{
    if ((NULL == p_cnt) || (0U == length))
    {
        return 0U;
    }
    uint16_t serial_length = 0U;
    uint16_t cnt_length = 0U;
    p_cnt[cnt_length++] = 0U; // dummy start element
    for (uint16_t i = 0; i < length; i++)
    {
        uint16_t shift_amount = (length - i) - 1U;
        if ((serial >> shift_amount) & 0x01U)
        {
            p_cnt[cnt_length++] = gn_id601_serial_bit1_ccr;
        }
        else
        {
            p_cnt[cnt_length++] = gn_id601_serial_bit0_ccr;
        }
    }
    p_cnt[cnt_length++] = 0U; // dummy end element
    return cnt_length;
}

bool id601_write(uint16_t addr, uint16_t data)
{
    uint16_t temp[12] = { 0U, 119U, 119U, 119U, 119U, 119U, 119U, 119U, 119U, 119U, 119U, 0U };
    return drv_tim_generate_serial(temp, 12U, ID601_SERIAL_TIMEOUT_MS);
}

bool id601_read(uint16_t addr, uint16_t data)
{
    uint16_t temp[12] = { 0U, 119U, 119U, 119U, 119U, 119U, 119U, 119U, 119U, 119U, 119U, 0U };
    drv_tim_generate_serial(temp, 12U, ID601_SERIAL_TIMEOUT_MS);
    drv_tim_capture_serial((uint16_t*)rise, (uint16_t*)fall, 21U, ID601_SERIAL_TIMEOUT_MS);
}

bool id601_idgen(void)
{
    uint16_t idgen = 0x0008U;
    uint16_t cnt_buffer[100] = { 0U };
    uint16_t cnt_length = id601_build_serial_ccr_buffer(idgen, cnt_buffer, 4U);
    return drv_tim_generate_serial(cnt_buffer, cnt_length, ID601_SERIAL_TIMEOUT_MS);
}
/* USER CODE END 0 */
