/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : id601_dimming.c
    * @brief          : id601_dimming implementation
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
#include "id601_dimming.h"
/* 2. C standard library headers (Alphabetical order) */
/* 3. Project internal / System-related headers */
#include "common_def.h"
#include "drv_id601.h"
#include "drv_uart.h"

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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint16_t gn_id601_ld_buffer[ID601_DAISY_LENGTH_MAX][ID601_CH_SIZE_MAX][ID601_LED_PER_CH_MAX];
static bool gb_id601_fault_read_enabled;
static bool gb_id601_ld_transfer_enabled;

static id601_ld_control_reg_t gt_id601_ld_control;
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void id601_dimming_init(void)
{
    gb_id601_fault_read_enabled = true;
    gb_id601_ld_transfer_enabled = true;

    gt_id601_ld_control.sv_no = ID601_SV_NO_DEFAULT;
    gt_id601_ld_control.ld_type = ID601_LD_TYPE_DEFAULT;
    gt_id601_ld_control.delay_ch_en = ID601_DELAY_CH_EN_DEFAULT;
    gt_id601_ld_control.syncmode = ID601_SYNCMODE_DEFAULT;
    gt_id601_ld_control.pwm_res = ID601_PWM_RES_DEFAULT;
    gt_id601_ld_control.ld_dir = ID601_LD_DIR_DEFAULT;
    gt_id601_ld_control.ld_mode = ID601_LD_MODE_DEFAULT;
}

void id601_config_ld_control(id601_ld_control_reg_t const* p_config)
{
    if (NULL == p_config)
    {
        return;
    }

    gt_id601_ld_control = *p_config;
}
id601_ld_control_reg_t const* id601_get_ld_control(void)
{
    return &gt_id601_ld_control;
}

void id601_config_fault_read(bool enable)
{
    gb_id601_fault_read_enabled = enable;
}
bool id601_get_fault_read_enabled(void)
{
    return gb_id601_fault_read_enabled;
}

void id601_config_ld_transfer(bool enable)
{
    gb_id601_ld_transfer_enabled = enable;
}
bool id601_get_ld_transfer_enabled(void)
{
    return gb_id601_ld_transfer_enabled;
}

uint16_t id601_get_ld_size_max(void)
{
    if (0U == gt_id601_ld_control.pwm_res)
    {
        return ID601_LD_SIZE_MAX_12BIT;
    }
    if (1U == gt_id601_ld_control.pwm_res)
    {
        return ID601_LD_SIZE_MAX_14BIT;
    }
    return 0U;
}

uint16_t* id601_get_ld_buffer(void)
{
    return &gn_id601_ld_buffer[0][0][0];
}
void id601_set_ld_buffer(uint16_t daisy, uint16_t ch, uint16_t led, uint16_t value)
{
    gn_id601_ld_buffer[daisy][ch][led] = value;
}

void id601_vsync_dimming_task(void)
{
    if (true == gb_id601_fault_read_enabled)
    {
        if (false == id601_fault_read(drv_id601_get_crc_func()))
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to read fault.", __func__);
            return;
        }
    }

    if (true == gb_id601_ld_transfer_enabled)
    {
        if (false == id601_ld_transfer(id601_get_ld_buffer()))
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to transfer LD.", __func__);
            return;
        }
    }
}

/* USER CODE END 0 */