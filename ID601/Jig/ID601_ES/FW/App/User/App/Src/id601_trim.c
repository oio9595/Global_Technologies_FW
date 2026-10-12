/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : id601_trim.c
    * @brief          : id601_trim implementation
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
#include "id601_trim.h"
/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_uart.h"
#include "drv_gpio.h"
#include "drv_id601.h"
#include "drv_timer.h"
#include "drv_adc.h"
#include "drv_ads114s08.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_TRIM_STEP_T
{
    TRIM_STEP_NONE = 0U,
    TRIM_STEP_INITIAL,
    TRIM_STEP_INITIAL_BY_LIST,
    TRIM_STEP_VSYNC_STOP,
    TRIM_STEP_START_MEASURE,
    TRIM_STEP_GET_MEASURED_VALUE,
    TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER,
    TRIM_STEP_PREPARE_EFUSE,
    TRIM_STEP_START_EFUSE,
    TRIM_STEP_END_EFUSE,
    TRIM_STEP_PWR_REBOOT,
    TRIM_STEP_VERIFY_OTP_DUMP,
    TRIM_STEP_LOG_SUMMARY,
    TRIM_STEP_PWR_OFF,
    TRIM_STEP_MAX,
} trim_step_t;

typedef enum tag_TRIM_LIST_T
{
    TRIM_LIST_NONE = 0U,
    TRIM_LIST_IREF1,
    TRIM_LIST_IREF2,
    TRIM_LIST_LDO_DET,
    TRIM_LIST_LDO_DIG,
    TRIM_LIST_LDO_DAC,
    TRIM_LIST_LDO_FLL,
    TRIM_LIST_OFS_TEMP,
    TRIM_LIST_OSC_RCTL,
    TRIM_LIST_GAIN,
    TRIM_LIST_OFFSET,
    TRIM_LIST_MAX,
} trim_list_t;

typedef struct tag_TRIM_INFO
{
    float value[12][2];
    bool repeat[12];
    uint16_t ch;
} trim_info_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define RTOS_TIME_DEBUG
#define T_DEFAULT_MS        (1U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static bool trim_step_none(thread_data_t *td);
static bool trim_step_initial(thread_data_t *td);
static bool trim_step_initial_by_list(thread_data_t *td);
static bool trim_step_vsync_stop(thread_data_t *td);
static bool trim_step_start_measure(thread_data_t *td);
static bool trim_step_get_measured_value(thread_data_t *td);
static bool trim_step_judge_range_update_register(thread_data_t *td);
static bool trim_step_prepare_efuse(thread_data_t *td);
static bool trim_step_start_efuse(thread_data_t *td);
static bool trim_step_end_efuse(thread_data_t *td);
static bool trim_step_pwr_reboot(thread_data_t *td);
static bool trim_step_verify_otp_dump(thread_data_t *td);
static bool trim_step_log_summary(thread_data_t *td);
static bool trim_step_pwr_off(thread_data_t *td);
/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static const trim_step_handler_t trim_step_handlers[TRIM_STEP_MAX] =
{
    [TRIM_STEP_NONE]                        = trim_step_none,
    [TRIM_STEP_INITIAL]                     = trim_step_initial,
    [TRIM_STEP_INITIAL_BY_LIST]             = trim_step_initial_by_list,
    [TRIM_STEP_VSYNC_STOP]                  = trim_step_vsync_stop,
    [TRIM_STEP_START_MEASURE]               = trim_step_start_measure,
    [TRIM_STEP_GET_MEASURED_VALUE]          = trim_step_get_measured_value,
    [TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER] = trim_step_judge_range_update_register,
    [TRIM_STEP_PREPARE_EFUSE]               = trim_step_prepare_efuse,
    [TRIM_STEP_START_EFUSE]                 = trim_step_start_efuse,
    [TRIM_STEP_END_EFUSE]                   = trim_step_end_efuse,
    [TRIM_STEP_PWR_REBOOT]                  = trim_step_pwr_reboot,
    [TRIM_STEP_VERIFY_OTP_DUMP]             = trim_step_verify_otp_dump,
    [TRIM_STEP_LOG_SUMMARY]                 = trim_step_log_summary,
    [TRIM_STEP_PWR_OFF]                     = trim_step_pwr_off,
};

static const trim_init_func_t trim_init_by_list_funcs[TRIM_LIST_MAX] =
{
    [TRIM_LIST_IREF1] = id601_trim_init_iref1,
    [TRIM_LIST_IREF2] = id601_trim_init_iref2,
    [TRIM_LIST_LDO_DET] = id601_trim_init_ldo_det,
    [TRIM_LIST_LDO_DIG] = id601_trim_init_ldo_dig,
    [TRIM_LIST_LDO_DAC] = id601_trim_init_ldo_dac,
    [TRIM_LIST_LDO_FLL] = id601_trim_init_ldo_fll,
    [TRIM_LIST_OFS_TEMP] = id601_trim_init_ofs_temp,
    [TRIM_LIST_OSC_RCTL] = id601_trim_init_osc_rctl,
    [TRIM_LIST_GAIN] = id601_trim_init_gain,
    [TRIM_LIST_OFFSET] = id601_trim_init_offset,
};

static const trim_write_func_t trim_write_funcs[TRIM_LIST_MAX] =
{
    [TRIM_LIST_IREF1] = id601_trim_write_iref1,
    [TRIM_LIST_IREF2] = id601_trim_write_iref2,
    [TRIM_LIST_LDO_DET] = id601_trim_write_ldo_det,
    [TRIM_LIST_LDO_DIG] = id601_trim_write_ldo_dig,
    [TRIM_LIST_LDO_DAC] = id601_trim_write_ldo_dac,
    [TRIM_LIST_LDO_FLL] = id601_trim_write_ldo_fll,
    [TRIM_LIST_OFS_TEMP] = id601_trim_write_ofs_temp,
    [TRIM_LIST_OSC_RCTL] = id601_trim_write_osc_rctl,
    [TRIM_LIST_GAIN] = id601_trim_write_gain,
    [TRIM_LIST_OFFSET] = id601_trim_write_offset,
};

static trim_list_t gt_trim_list;
static trim_info_t gt_trim_info[TRIM_LIST_MAX];
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void id601_trim_initiate_parameters(void)
{
    gt_trim_list = TRIM_LIST_NONE;
    for (trim_list_t i = 0; i < TRIM_LIST_MAX; i++)
    {
        gt_trim_info[i].ch = 0U;
        for (uint16_t j = 0U; j < 12U; j++)
        {
            gt_trim_info[i].repeat[j] = false;
            gt_trim_info[i].value[j][0] = 0.0f;
            gt_trim_info[i].value[j][1] = 0.0f;
        }
    }
}

static bool trim_step_none(thread_data_t *td)
{
    (void)td;
    return false;
}

static bool trim_step_initial(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }
    id601_init_analog_test_mode();
    td->step = TRIM_STEP_INITIAL_BY_LIST;
    td->tout = T_DEFAULT_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_initial_by_list(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    if (NULL != trim_init_by_list_funcs[gt_trim_list])
    {
        trim_init_by_list_funcs[gt_trim_list]();
    }

    if ((TRIM_LIST_IREF1 == gt_trim_list) || (TRIM_LIST_IREF2 == gt_trim_list) || (TRIM_LIST_LDO_DET == gt_trim_list) || (TRIM_LIST_LDO_DIG == gt_trim_list) || \
        (TRIM_LIST_LDO_DAC == gt_trim_list) || (TRIM_LIST_LDO_FLL == gt_trim_list))
    {
        td->step = TRIM_STEP_START_MEASURE;
    }
    else if (TRIM_LIST_OFS_TEMP == gt_trim_list)
    {
        uint16_t ch = gt_trim_info[TRIM_LIST_OFS_TEMP].ch;
        if (false == gt_trim_info[TRIM_LIST_OFS_TEMP].repeat[ch])
        {
            id601_trim_init_ofs_temp_o1();
            gt_trim_info[TRIM_LIST_OFS_TEMP].repeat[ch] = true;
        }
        else
        {
            id601_trim_init_ofs_temp_o2();
            gt_trim_info[TRIM_LIST_OFS_TEMP].repeat[ch] = false;
        }
        td->step = TRIM_STEP_START_MEASURE;
    }
    else if (TRIM_LIST_OSC_RCTL == gt_trim_list)
    {
        td->step = TRIM_STEP_START_MEASURE;
    }
    else if ((TRIM_LIST_GAIN == gt_trim_list) || (TRIM_LIST_OFFSET == gt_trim_list))
    {
        uint16_t ch = gt_trim_info[gt_trim_list].ch;
        id601_trim_init_current_chx(ch);
        drv_gpio_id601_ch_demux(ch);
        if (false == gt_trim_info[gt_trim_list].repeat[ch])
        {
            id601_trim_init_gain_vref1();
            gt_trim_info[gt_trim_list].repeat[ch] = true;
        }
        else
        {
            id601_trim_init_gain_vref2();
            gt_trim_info[gt_trim_list].repeat[ch] = false;
        }
        td->step = TRIM_STEP_START_MEASURE;
    }
    td->tout = T_DEFAULT_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_vsync_stop(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_start_measure(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    if ((TRIM_LIST_IREF1 == gt_trim_list) || (TRIM_LIST_IREF2 == gt_trim_list) || (TRIM_LIST_LDO_DET == gt_trim_list) || (TRIM_LIST_LDO_DIG == gt_trim_list) || (TRIM_LIST_LDO_DAC == gt_trim_list) || (TRIM_LIST_LDO_FLL == gt_trim_list))
    {
        mcu_adc_set_conversion_enable(true);
        td->step = TRIM_STEP_GET_MEASURED_VALUE;
    }
    else if (TRIM_LIST_OFS_TEMP == gt_trim_list)
    {
        ads114s08_set_input_mux(ADS_DEV_2, ADS_DEV2_AIN06_ID601_O1, ADS_INPUT_N_GND);
        ads114s08_set_conversion_enable(ADS_DEV_2, true);
        td->step = TRIM_STEP_GET_MEASURED_VALUE;
    }
    else if (TRIM_LIST_OSC_RCTL == gt_trim_list)
    {
        mcu_tim_set_conversion_enable(true);
        td->step = TRIM_STEP_GET_MEASURED_VALUE;
    }
    else if ((TRIM_LIST_GAIN == gt_trim_list) || (TRIM_LIST_OFFSET == gt_trim_list))
    {
        ads114s08_set_input_mux(ADS_DEV_1, ADS_DEV1_AIN00_ID601_IOUT, ADS_INPUT_N_GND);
        ads114s08_set_conversion_enable(ADS_DEV_1, true);
        td->step = TRIM_STEP_GET_MEASURED_VALUE;
    }
    td->tout = T_DEFAULT_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_get_measured_value(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    if ((TRIM_LIST_IREF1 == gt_trim_list) || (TRIM_LIST_IREF2 == gt_trim_list) || (TRIM_LIST_LDO_DET == gt_trim_list) || (TRIM_LIST_LDO_DIG == gt_trim_list) || (TRIM_LIST_LDO_DAC == gt_trim_list) || (TRIM_LIST_LDO_FLL == gt_trim_list))
    {
        if (true == mcu_adc_wait_conversion_complete())
        {
            uint32_t adc_value = mcu_adc_get_average_conversion();
            drv_uart_printf(LOG_LV_DEBUG, "\r\n    ADC conversion complete. %lu", adc_value);
        }
        td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
    }
    else if (TRIM_LIST_OFS_TEMP == gt_trim_list)
    {
        if (true == ads114s08_wait_conversion_complete())
        {
            int32_t adc_value = ads114s08_get_average_conversion(ADS_DEV_2);
            drv_uart_printf(LOG_LV_DEBUG, "\r\n    ADS114S08 conversion complete. %ld", adc_value);
        }
        td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
    }
    else if (TRIM_LIST_OSC_RCTL == gt_trim_list)
    {
        if (true == mcu_tim_wait_conversion_complete())
        {
            uint32_t tim_value = mcu_tim_get_average_conversion();
            drv_uart_printf(LOG_LV_DEBUG, "\r\n    Timer conversion complete. %lu", tim_value);
        }
        td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
    }
    else if ((TRIM_LIST_GAIN == gt_trim_list) || (TRIM_LIST_OFFSET == gt_trim_list))
    {
        if (true == ads114s08_wait_conversion_complete())
        {
            int32_t adc_value = ads114s08_get_average_conversion(ADS_DEV_1);
            drv_uart_printf(LOG_LV_DEBUG, "\r\n    ADS114S08 conversion complete. %ld", adc_value);
        }
        td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
    }
    td->tout = T_DEFAULT_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_judge_range_update_register(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_prepare_efuse(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_start_efuse(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_end_efuse(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_pwr_reboot(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_verify_otp_dump(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_log_summary(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

static bool trim_step_pwr_off(thread_data_t *td)
{
    if ((NULL == td) || (NULL == td->func) || (0U == td->tout))
    {
        return false;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    %s executed after: %u ms", __func__, td->tout);
    #endif
    return true;
}

void id601_trim_init(thread_data_t *td)
{
    if (NULL == td)
    {
        return;
    }

    id601_trim_initiate_parameters();

    td->step = TRIM_STEP_INITIAL;
    td->tout = 10U;
    td->func = trim_step_initial;
}

void id601_trim_deinit(thread_data_t *td)
{
    if (NULL == td)
    {
        return;
    }

    id601_trim_initiate_parameters();

    td->step = TRIM_STEP_NONE;
    td->tout = 10U;
    td->func = trim_step_none;
}

bool id601_trim_process(thread_data_t *td)
{
    if (NULL == td)
    {
        return false;
    }

    const uint32_t step = td->step;
    if ((TRIM_STEP_MAX <= step) || (NULL == trim_step_handlers[step]))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    Invalid trim step: %u", step);
        return false;
    }

    return trim_step_handlers[step](td);
}
/* USER CODE END 0 */
