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
#include <stdio.h>
#include <string.h>
#include <math.h>

/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_uart.h"
#include "drv_gpio.h"
#include "drv_id601.h"
#include "drv_timer.h"
#include "drv_adc.h"
#include "drv_ads114s08.h"
#include "cmsis_os.h"
#include "common_def.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef void (*trim_setup_func_t)(void);
typedef uint16_t (*trim_apply_code_func_t)(uint16_t, uint16_t);

typedef enum tag_TRIM_STEP_T
{
    TRIM_STEP_NONE = 0U,
    TRIM_STEP_INITIAL,
    TRIM_STEP_SETUP_BY_LIST,
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

typedef struct
{
    bool initialized;
    bool searched;
    uint16_t attempts;
    int32_t low;
    int32_t high;
    uint16_t best_code;
    uint16_t best_reg;
    float best_error;
} trim_search_t;

typedef struct tag_TRIM_INFO
{
    float tmp_value[12][2];
    float value[12];
    uint16_t trim_code[12];
    uint16_t reg_val[12];
    uint16_t ch;
    bool repeat[12];
    trim_search_t search[12];
} trim_info_t;

typedef enum tag_TRIM_RANGE_T
{
    TRIM_RANGE_TARGET = 0U,
    TRIM_RANGE_LOWER,
    TRIM_RANGE_UPPER,
    TRIM_RANGE_MAX,
} trim_range_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define REPEAT_FIRST                (false)
#define REPEAT_SECOND               (true)

//#define RTOS_TIME_DEBUG
#define T_DEFAULT_MS                (1U)
#define T_STABLE_MS                 (10U)
#define T_MEASURE_DONE_MCU_ADC_MS   (1U)
#define T_MEASURE_DONE_MCU_TIM_MS   (60U)
#define T_MEASURE_DONE_ADS_ADC_MS   (10U)
#define T_RAMP_UP_MS                (10U)
#define T_FUSING_DONE_MS            (10U)
#define T_RAMP_DOWN_MS              (10U)

#define TRIM_IREF1_TARGET_UA        (10.0f)   // 10uA
#define TRIM_IREF2_TARGET_UA        (10.0f)   // 10uA
#define TRIM_LDO_DET_TARGET_V       (1.5f)    // 1.5V
#define TRIM_LDO_DIG_TARGET_V       (1.5f)    // 1.5V
#define TRIM_LDO_DAC_TARGET_V       (1.5f)    // 1.5V
#define TRIM_LDO_FLL_TARGET_V       (1.5f)    // 1.5V
#define TRIM_OFS_TEMP_TARGET_V      (0.2094f) // 209.4mV
#define TRIM_OSC_RCTL_TARGET_MHZ    (51.0f)   // 51MHz
#if (ID601_TRIM_GAIN_MAX_CURR_LVL == ID601_MAX_CURRENT_LEVEL_08mA)
    #define TRIM_GAIN_TARGET_MA     ((8.0f * (ID601_TRIM_GAIN_VREF2 - ID601_TRIM_GAIN_VREF1)) / 4095.0f)
#else
    #error "Unsupported ID601_TRIM_GAIN_MAX_CURR_LVL value"
#endif
#if (ID601_TRIM_OFFSET_MAX_CURR_LVL == ID601_MAX_CURRENT_LEVEL_08mA)
    #define TRIM_OFFSET_TARGET_MA   ((8.0f * (ID601_TRIM_OFFSET_VREF2 + ID601_TRIM_OFFSET_VREF1) / 2.0f) / 4095.0f)
#else
    #error "Unsupported ID601_TRIM_OFFSET_MAX_CURR_LVL value"
#endif

#define TRIM_IREF1_TOL_RATIO        (3.0f / 100.0f)
#define TRIM_IREF2_TOL_RATIO        (3.0f / 100.0f)
#define TRIM_LDO_DET_TOL_RATIO      (3.0f / 100.0f)
#define TRIM_LDO_DIG_TOL_RATIO      (3.0f / 100.0f)
#define TRIM_LDO_DAC_TOL_RATIO      (3.0f / 100.0f)
#define TRIM_LDO_FLL_TOL_RATIO      (3.0f / 100.0f)
#define TRIM_OFS_TEMP_TOL_RATIO     (3.0f / 100.0f)
#define TRIM_OSC_RCTL_TOL_RATIO     (3.0f / 100.0f)
#define TRIM_GAIN_TOL_RATIO         (3.0f / 100.0f)
#define TRIM_OFFSET_TOL_RATIO       (3.0f / 100.0f)

// remove below if not used
#define TRIM_IREF1_INIT_MOVE        (4U)
#define TRIM_IREF2_INIT_MOVE        (4U)
#define TRIM_LDO_DET_INIT_MOVE      (4U)
#define TRIM_LDO_DIG_INIT_MOVE      (4U)
#define TRIM_LDO_DAC_INIT_MOVE      (4U)
#define TRIM_LDO_FLL_INIT_MOVE      (4U)
#define TRIM_OFS_TEMP_INIT_MOVE     (4U)
#define TRIM_OSC_RCTL_INIT_MOVE     (4U)
#define TRIM_GAIN_INIT_MOVE         (4U)
#define TRIM_OFFSET_INIT_MOVE       (4U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static bool trim_step_none(thread_data_t *td);
static bool trim_step_initial(thread_data_t *td);
static bool trim_step_setup_by_list(thread_data_t *td);
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
static const trim_step_handler_t gp_trim_step_handlers[TRIM_STEP_MAX] =
{
    [TRIM_STEP_NONE]                        = trim_step_none,
    [TRIM_STEP_INITIAL]                     = trim_step_initial,
    [TRIM_STEP_SETUP_BY_LIST]               = trim_step_setup_by_list,
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

static const trim_setup_func_t gp_trim_setup_by_list[TRIM_LIST_MAX] =
{
    [TRIM_LIST_IREF1] = id601_trim_setup_iref1,
    [TRIM_LIST_IREF2] = id601_trim_setup_iref2,
    [TRIM_LIST_LDO_DET] = id601_trim_setup_ldo_det,
    [TRIM_LIST_LDO_DIG] = id601_trim_setup_ldo_dig,
    [TRIM_LIST_LDO_DAC] = id601_trim_setup_ldo_dac,
    [TRIM_LIST_LDO_FLL] = id601_trim_setup_ldo_fll,
    [TRIM_LIST_OFS_TEMP] = id601_trim_setup_ofs_temp,
    [TRIM_LIST_OSC_RCTL] = id601_trim_setup_osc_rctl,
    [TRIM_LIST_GAIN] = id601_trim_setup_gain,
    [TRIM_LIST_OFFSET] = id601_trim_setup_offset,
};

static const trim_apply_code_func_t gp_trim_apply_code_by_list[TRIM_LIST_MAX] =
{
    [TRIM_LIST_IREF1] = id601_trim_apply_iref1,
    [TRIM_LIST_IREF2] = id601_trim_apply_iref2,
    [TRIM_LIST_LDO_DET] = id601_trim_apply_ldo_det,
    [TRIM_LIST_LDO_DIG] = id601_trim_apply_ldo_dig,
    [TRIM_LIST_LDO_DAC] = id601_trim_apply_ldo_dac,
    [TRIM_LIST_LDO_FLL] = id601_trim_apply_ldo_fll,
    [TRIM_LIST_OFS_TEMP] = id601_trim_apply_ofs_temp,
    [TRIM_LIST_OSC_RCTL] = id601_trim_apply_osc_rctl,
    [TRIM_LIST_GAIN] = id601_trim_apply_gain,
    [TRIM_LIST_OFFSET] = id601_trim_apply_offset,
};

static const float gf_trim_tolerance[TRIM_LIST_MAX][TRIM_RANGE_MAX] =
{
    [TRIM_LIST_IREF1]    = {     TRIM_IREF1_TARGET_UA,     TRIM_IREF1_TARGET_UA * (1 - TRIM_IREF1_TOL_RATIO),        TRIM_IREF1_TARGET_UA    * (1 + TRIM_IREF1_TOL_RATIO) },
    [TRIM_LIST_IREF2]    = {     TRIM_IREF2_TARGET_UA,     TRIM_IREF2_TARGET_UA * (1 - TRIM_IREF2_TOL_RATIO),        TRIM_IREF2_TARGET_UA    * (1 + TRIM_IREF2_TOL_RATIO) },
    [TRIM_LIST_LDO_DET]  = {    TRIM_LDO_DET_TARGET_V,    TRIM_LDO_DET_TARGET_V * (1 - TRIM_LDO_DET_TOL_RATIO),     TRIM_LDO_DET_TARGET_V  * (1 + TRIM_LDO_DET_TOL_RATIO) },
    [TRIM_LIST_LDO_DIG]  = {    TRIM_LDO_DIG_TARGET_V,    TRIM_LDO_DIG_TARGET_V * (1 - TRIM_LDO_DIG_TOL_RATIO),     TRIM_LDO_DIG_TARGET_V  * (1 + TRIM_LDO_DIG_TOL_RATIO) },
    [TRIM_LIST_LDO_DAC]  = {    TRIM_LDO_DAC_TARGET_V,    TRIM_LDO_DAC_TARGET_V * (1 - TRIM_LDO_DAC_TOL_RATIO),     TRIM_LDO_DAC_TARGET_V  * (1 + TRIM_LDO_DAC_TOL_RATIO) },
    [TRIM_LIST_LDO_FLL]  = {    TRIM_LDO_FLL_TARGET_V,    TRIM_LDO_FLL_TARGET_V * (1 - TRIM_LDO_FLL_TOL_RATIO),     TRIM_LDO_FLL_TARGET_V  * (1 + TRIM_LDO_FLL_TOL_RATIO) },
    [TRIM_LIST_OFS_TEMP] = {   TRIM_OFS_TEMP_TARGET_V,   TRIM_OFS_TEMP_TARGET_V * (1 - TRIM_OFS_TEMP_TOL_RATIO),   TRIM_OFS_TEMP_TARGET_V * (1 + TRIM_OFS_TEMP_TOL_RATIO) },
    [TRIM_LIST_OSC_RCTL] = { TRIM_OSC_RCTL_TARGET_MHZ, TRIM_OSC_RCTL_TARGET_MHZ * (1 - TRIM_OSC_RCTL_TOL_RATIO), TRIM_OSC_RCTL_TARGET_MHZ * (1 + TRIM_OSC_RCTL_TOL_RATIO) },
    [TRIM_LIST_GAIN]     = {      TRIM_GAIN_TARGET_MA,      TRIM_GAIN_TARGET_MA * (1 - TRIM_GAIN_TOL_RATIO),          TRIM_GAIN_TARGET_MA     * (1 + TRIM_GAIN_TOL_RATIO) },
    [TRIM_LIST_OFFSET]   = {    TRIM_OFFSET_TARGET_MA,    TRIM_OFFSET_TARGET_MA * (1 - TRIM_OFFSET_TOL_RATIO),      TRIM_OFFSET_TARGET_MA   * (1 + TRIM_OFFSET_TOL_RATIO) },
};

static const uint16_t gn_trim_code_max[TRIM_LIST_MAX] =
{
    [TRIM_LIST_IREF1]    = ID601_TRIM_CODE_IREF1_MAX,
    [TRIM_LIST_IREF2]    = ID601_TRIM_CODE_IREF2_MAX,
    [TRIM_LIST_LDO_DET]  = ID601_TRIM_CODE_LDO_BGR2_CTL_MAX,
    [TRIM_LIST_LDO_DIG]  = ID601_TRIM_CODE_LDO_CTL_MAX,
    [TRIM_LIST_LDO_DAC]  = ID601_TRIM_CODE_LDO_DAC_CTL_MAX,
    [TRIM_LIST_LDO_FLL]  = ID601_TRIM_CODE_LDO_OSC_CTL_MAX,
    [TRIM_LIST_OFS_TEMP] = ID601_TRIM_CODE_OFS_TEMP_MAX,
    [TRIM_LIST_OSC_RCTL] = ID601_TRIM_CODE_OSC_RCTL_MAX,
    [TRIM_LIST_GAIN]     = ID601_TRIM_CODE_GAIN_MAX,
    [TRIM_LIST_OFFSET]   = ID601_TRIM_CODE_OFFSET_MAX,
};

static const char * const gp_trim_list_name[TRIM_LIST_MAX] =
{
    [TRIM_LIST_NONE]     = "NONE    ",
    [TRIM_LIST_IREF1]    = "IREF1   ",
    [TRIM_LIST_IREF2]    = "IREF2   ",
    [TRIM_LIST_LDO_DET]  = "LDO_DET ",
    [TRIM_LIST_LDO_DIG]  = "LDO_DIG ",
    [TRIM_LIST_LDO_DAC]  = "LDO_DAC ",
    [TRIM_LIST_LDO_FLL]  = "LDO_FLL ",
    [TRIM_LIST_OFS_TEMP] = "OFS_TEMP",
    [TRIM_LIST_OSC_RCTL] = "OSC_RCTL",
    [TRIM_LIST_GAIN]     = "GAIN    ",
    [TRIM_LIST_OFFSET]   = "OFFSET  ",
};

static trim_list_t gt_trim_list;
static trim_info_t gt_trim_info[TRIM_LIST_MAX];
static uint8_t gn_id601_trim_iout_gain;
static bool gb_id601_efuse_burn;
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static const char *trim_get_list_name(trim_list_t list)
{
    if ((unsigned)TRIM_LIST_MAX <= (unsigned)list)
    {
        return "INVALID";
    }

    return gp_trim_list_name[list];
}
static const char *trim_get_unit(trim_list_t list)
{
    switch (list)
    {
        case TRIM_LIST_IREF1:
        case TRIM_LIST_IREF2:
        {
            return "uA";
        }
        case TRIM_LIST_LDO_DET:
        case TRIM_LIST_LDO_DIG:
        case TRIM_LIST_LDO_DAC:
        case TRIM_LIST_LDO_FLL:
        case TRIM_LIST_OFS_TEMP:
        {
            return "V";
        }
        case TRIM_LIST_OSC_RCTL:
        {
            return "MHz";
        }
        case TRIM_LIST_GAIN:
        case TRIM_LIST_OFFSET:
        {
            return "mA";
        }
        default:
        {
            return "";
        }
    }
}
static void trim_make_prefix(char *buffer, size_t size, trim_list_t list, uint16_t ch)
{
    if ((TRIM_LIST_GAIN == list) || (TRIM_LIST_OFFSET == list))
    {
        snprintf(buffer, size, "    [TRIM] [%s] [CH%02u]", trim_get_list_name(list), (unsigned)(ch + 1U));
    }
    else
    {
        snprintf(buffer, size, "    [TRIM] [%s]", trim_get_list_name(list));
    }
}
static inline bool trim_error_handler(thread_data_t *td)
{
    td->step = TRIM_STEP_PWR_OFF;
    td->tout = T_DEFAULT_MS;
    return true;
}

void id601_set_efuse_burn_enable(bool enable)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    [TRIM] %s: EFUSE burn %s", __func__, enable ? "enabled" : "disabled");
    gb_id601_efuse_burn = enable;
}

bool id601_get_efuse_burn_enable(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    [TRIM] %s: EFUSE burn %s", __func__, gb_id601_efuse_burn ? "enabled" : "disabled");
    return gb_id601_efuse_burn;
}

static bool trim_step_none(thread_data_t *td)
{
    (void)td;
    return false;
}

static bool trim_step_initial(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    if (false == id601_init_testmode_entry())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] %s: ID601 testmode entry failed", __func__);
        return trim_error_handler(td);
    }

    if (false == id601_compare_initial_code())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Initial code comparison failed.", __func__);
        return false;
    }

    if (false == id601_init_analog_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] %s: ID601 initialization failed", __func__);
        return trim_error_handler(td);
    }

    for (uint16_t i = TRIM_LIST_IREF1; i < TRIM_LIST_MAX; i++)
    {
        const uint16_t j_loop = ((TRIM_LIST_GAIN == i) || (TRIM_LIST_OFFSET == i)) ? (12U) : (1U);
        for (uint16_t j = 0U; j < j_loop; ++j)
        {
            if (NULL != gp_trim_apply_code_by_list[i])
            {
                const uint16_t reg_val = gp_trim_apply_code_by_list[i](j, gt_trim_info[i].trim_code[j]);
                if (0xFFFF != reg_val)
                {
                    gt_trim_info[i].reg_val[j] = reg_val;
                }
                else
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] [%s] %s: Initial trim write failed, ch=%u code=%u", trim_get_list_name((trim_list_t)i), __func__, (unsigned)j,(unsigned)gt_trim_info[i].trim_code[j]);
                    return trim_error_handler(td);
                }
            }
        }
    }

    gn_id601_trim_iout_gain = ID601_IOUT_GAIN_MID;
    drv_gpio_id601_iout_gain(gn_id601_trim_iout_gain);

    td->step = TRIM_STEP_SETUP_BY_LIST;
    td->tout = T_STABLE_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_setup_by_list(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);
    USER_ASSERT(TRIM_LIST_NONE < gt_trim_list);
    USER_ASSERT(TRIM_LIST_MAX > gt_trim_list);
    USER_ASSERT(NULL != gp_trim_setup_by_list[gt_trim_list]);

    const uint16_t ch = gt_trim_info[gt_trim_list].ch;
    USER_ASSERT(ch < (sizeof(gt_trim_info[gt_trim_list].repeat) / sizeof(gt_trim_info[gt_trim_list].repeat[0])));
    const bool repeat = gt_trim_info[gt_trim_list].repeat[ch];

    gp_trim_setup_by_list[gt_trim_list]();

    switch (gt_trim_list)
    {
        case TRIM_LIST_IREF1:
        case TRIM_LIST_IREF2:
        case TRIM_LIST_LDO_DET:
        case TRIM_LIST_LDO_DIG:
        case TRIM_LIST_LDO_DAC:
        case TRIM_LIST_LDO_FLL:
        {
            break;
        }
        case TRIM_LIST_OFS_TEMP:
        {
            if (false == repeat)
            {
                id601_trim_setup_ofs_temp_o1();
            }
            else
            {
                id601_trim_setup_ofs_temp_o2();
            }
            break;
        }
        case TRIM_LIST_OSC_RCTL:
        {
            break;
        }
        case TRIM_LIST_GAIN:
        case TRIM_LIST_OFFSET:
        {
            drv_gpio_id601_vled(ID601_VLED_ON);
            drv_gpio_id601_ch_demux(ch);
            id601_trim_setup_current_chx(ch);
            ads114s08_set_input_mux(ADS_DEV_1, ADS_DEV1_AIN00_ID601_IOUT, ADS_INPUT_N_GND);

            void_func_ptr_t fp_init = NULL;
            if (TRIM_LIST_GAIN == gt_trim_list)
            {
                fp_init = (false == repeat) ? (id601_trim_setup_gain_vref1) : (id601_trim_setup_gain_vref2);
            }
            else
            {
                fp_init = (false == repeat) ? (id601_trim_setup_offset_vref1) : (id601_trim_setup_offset_vref2);
            }

            if (NULL == fp_init)
            {
                USER_ASSERT(false);
            }
            fp_init();
            break;
        }
        default:
        {
            USER_ASSERT(false);
        }
    }
    td->step = TRIM_STEP_START_MEASURE;
    td->tout = T_STABLE_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_vsync_stop(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_start_measure(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);
    USER_ASSERT(TRIM_LIST_NONE < gt_trim_list);
    USER_ASSERT(TRIM_LIST_MAX > gt_trim_list);

    switch (gt_trim_list)
    {
        case TRIM_LIST_IREF1:
        case TRIM_LIST_IREF2:
        case TRIM_LIST_LDO_DET:
        case TRIM_LIST_LDO_DIG:
        case TRIM_LIST_LDO_DAC:
        case TRIM_LIST_LDO_FLL:
        case TRIM_LIST_OFS_TEMP:
        {
            mcu_adc_set_conversion_enable(true);
            td->tout = T_MEASURE_DONE_MCU_ADC_MS;
            break;
        }
        case TRIM_LIST_OSC_RCTL:
        {
            mcu_tim_set_conversion_enable(true);
            td->tout = T_MEASURE_DONE_MCU_TIM_MS;
            break;
        }
        case TRIM_LIST_GAIN:
        case TRIM_LIST_OFFSET:
        {
            ads114s08_set_conversion_enable(ADS_DEV_1, true);
            td->tout = T_MEASURE_DONE_ADS_ADC_MS;
            break;
        }
        default:
        {
            USER_ASSERT(false);
        }
    }
    td->step = TRIM_STEP_GET_MEASURED_VALUE;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_get_measured_value(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);
    USER_ASSERT(TRIM_LIST_NONE < gt_trim_list);
    USER_ASSERT(TRIM_LIST_MAX > gt_trim_list);

    trim_info_t* trim_info = &gt_trim_info[gt_trim_list];

    const uint16_t ch = trim_info->ch;
    USER_ASSERT(ch < (sizeof(trim_info->repeat) / sizeof(trim_info->repeat[0])));
    const bool repeat = trim_info->repeat[ch];

    bool conversion_success = false;

    switch (gt_trim_list)
    {
        case TRIM_LIST_IREF1:
        case TRIM_LIST_IREF2:
        case TRIM_LIST_LDO_DET:
        case TRIM_LIST_LDO_DIG:
        case TRIM_LIST_LDO_DAC:
        case TRIM_LIST_LDO_FLL:
        case TRIM_LIST_OFS_TEMP:
        {
            if (true == mcu_adc_wait_conversion_complete())
            {
                const uint32_t adc_value = mcu_adc_get_average_conversion();
                const float voltage_mv = mcu_adc_conversion_adc_to_mvoltage(adc_value);
                drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] MEASURE: MCU ADC=%lu voltage=%.2f mV", trim_get_list_name(gt_trim_list), (unsigned long)adc_value, (double)voltage_mv);

                const bool is_ref = ((TRIM_LIST_IREF1 == gt_trim_list) || (TRIM_LIST_IREF2 == gt_trim_list)) ? (true) : (false);
                trim_info->tmp_value[ch][repeat] = (true == is_ref) ? (CONST_MA_to_UA(voltage_mv / HW_ID601_IREF_SENSE_OHM)) : (CONST_MV_to_V(voltage_mv));
                conversion_success = true;
            }
            break;
        }
        case TRIM_LIST_OSC_RCTL:
        {
            if (true == mcu_tim_wait_conversion_complete())
            {
                const uint32_t cnt_value = mcu_tim_get_average_conversion();
                const float frequency_hz = mcu_tim_conversion_cnt_to_frequency_Hz(cnt_value);
                drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] MEASURE: Timer CNT=%lu frequency=%.2f Hz", trim_get_list_name(gt_trim_list), (unsigned long)cnt_value, (double)frequency_hz);
                trim_info->tmp_value[ch][repeat] = CONST_Hz_to_MHz(frequency_hz * ID601_FREQ_DIV_INTERNAL_RATIO * ID601_FREQ_DIV_HW_RATIO);
                conversion_success = true;
            }
            break;
        }
        case TRIM_LIST_GAIN:
        case TRIM_LIST_OFFSET:
        {
            if (true == ads114s08_wait_conversion_complete())
            {
                int32_t adc_value = ads114s08_get_average_conversion(ADS_DEV_1);
                const float voltage_mv = ads114s08_conversion_adc_to_mvoltage(adc_value);
                drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] MEASURE: ADS ADC=%ld voltage=%.2f mV", trim_get_list_name(gt_trim_list), (long)adc_value, (double)voltage_mv);
                switch (gn_id601_trim_iout_gain)
                {
                    case ID601_IOUT_GAIN_LOW:
                    {
                        trim_info->tmp_value[ch][repeat] = HW_ID601_IOUT_MV_TO_MA_LOW(voltage_mv);
                        break;
                    }
                    case ID601_IOUT_GAIN_MID:
                    {
                        trim_info->tmp_value[ch][repeat] = HW_ID601_IOUT_MV_TO_MA_MID(voltage_mv);
                        break;
                    }
                    case ID601_IOUT_GAIN_HIGH:
                    {
                        trim_info->tmp_value[ch][repeat] = HW_ID601_IOUT_MV_TO_MA_HIGH(voltage_mv);
                        break;
                    }
                    default:
                    {
                        USER_ASSERT(false);
                    }
                }
                conversion_success = true;
            }
            break;
        }
        default:
        {
            USER_ASSERT(false);
        }
    }

    if (false == conversion_success)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] [%s] %s: Conversion failed, Trim stop", trim_get_list_name(gt_trim_list), __func__);
        return trim_error_handler(td);
    }
    else
    {
        if (REPEAT_FIRST == repeat)
        {
            td->step = TRIM_STEP_SETUP_BY_LIST;
            trim_info->repeat[ch] = REPEAT_SECOND;
        }
        else
        {
            td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
            trim_info->repeat[ch] = REPEAT_FIRST;

            const float first_val = trim_info->tmp_value[ch][REPEAT_FIRST];
            const float second_val = trim_info->tmp_value[ch][REPEAT_SECOND];

            if ((TRIM_LIST_OFS_TEMP == gt_trim_list) || (TRIM_LIST_GAIN == gt_trim_list))
            {
                trim_info->value[ch] = GET_DELTA(first_val, second_val);
            }
            else
            {
                trim_info->value[ch] = GET_AVERAGE(first_val, second_val);
            }
        }
        td->tout = T_DEFAULT_MS;
    }

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_judge_range_update_register(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);
    USER_ASSERT(TRIM_LIST_NONE < gt_trim_list);
    USER_ASSERT(TRIM_LIST_MAX > gt_trim_list);

    trim_info_t *trim_info = &gt_trim_info[gt_trim_list];

    const uint16_t ch = trim_info->ch;
    const uint16_t channel_count = ((TRIM_LIST_GAIN == gt_trim_list) || (TRIM_LIST_OFFSET == gt_trim_list)) ? (12U) : (1U);
    if (ch >= channel_count)
    {
        return trim_error_handler(td);
    }
    trim_search_t *search = &trim_info->search[ch];
    const uint16_t now_code = trim_info->trim_code[ch];

    const float measured = trim_info->value[ch];
    const float target = gf_trim_tolerance[gt_trim_list][TRIM_RANGE_TARGET];
    const float lower_limit = gf_trim_tolerance[gt_trim_list][TRIM_RANGE_LOWER];
    const float upper_limit = gf_trim_tolerance[gt_trim_list][TRIM_RANGE_UPPER];
    const float error = measured - target;
    const float abs_error = (error >= 0.0f) ? error : -error;

    char prefix[48];
    trim_make_prefix(prefix, sizeof(prefix), gt_trim_list, ch);
    const char *unit = trim_get_unit(gt_trim_list);

    if ((now_code > gn_trim_code_max[gt_trim_list]) || (lower_limit > upper_limit) || !isfinite(measured) || !isfinite(target) || !isfinite(lower_limit) || !isfinite(upper_limit) || !isfinite(error) || !isfinite(abs_error))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] [%s] %s: Invalid trim data, ch=%u code=%u", trim_get_list_name(gt_trim_list), __func__, (unsigned)ch, (unsigned)now_code);
        return trim_error_handler(td);
    }

    const trim_apply_code_func_t apply_code = gp_trim_apply_code_by_list[gt_trim_list];

    if (NULL == apply_code)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] [%s] %s: Trim apply function missing", trim_get_list_name(gt_trim_list), __func__);
        return trim_error_handler(td);
    }

    if (true == search->searched)
    {
        if ((now_code != search->best_code) || (trim_info->reg_val[ch] != search->best_reg) || (measured < lower_limit) || (measured > upper_limit))
        {
            if ((now_code != search->best_code) || (trim_info->reg_val[ch] != search->best_reg))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n%s FAIL:  code=%u expected_code=%u reg=%3u expected_reg=%3u", prefix, (unsigned)now_code, (unsigned)search->best_code, (unsigned)trim_info->reg_val[ch], (unsigned)search->best_reg);
            }
            else
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n%s FAIL:  measured=%.3f %s range=[%.3f, %.3f] %s", prefix, (double)measured, unit, (double)lower_limit, (double)upper_limit, unit);
            }
            return trim_error_handler(td);
        }
        drv_uart_printf(LOG_LV_INFO, "\r\n%s PASS:  trials=%u code=%u reg=%3u measured=%.3f %s", prefix, (unsigned)search->attempts, (unsigned)now_code, (unsigned)trim_info->reg_val[ch], (double)measured, unit);
        if ((ch + 1U) < channel_count)
        {
            ++trim_info->ch;
            td->step = TRIM_STEP_SETUP_BY_LIST;
        }
        else if ((gt_trim_list + 1U) < TRIM_LIST_MAX)
        {
            gt_trim_list = (trim_list_t)(gt_trim_list + 1U);
            td->step = TRIM_STEP_SETUP_BY_LIST;
        }
        else
        {
            td->step = TRIM_STEP_PREPARE_EFUSE;
        }

        td->tout = T_STABLE_MS;
        return true;
    }

    if (false == search->initialized)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n\r\n%s START: target=%.3f %s range=[%.3f, %.3f] %s", prefix, (double)target, unit, (double)lower_limit, (double)upper_limit, unit);

        search->low = 0U; // smallest possible code
        search->high = gn_trim_code_max[gt_trim_list]; // biggest possible code
        search->best_code = now_code; // initial code
        search->best_error = abs_error;
        search->best_reg = trim_info->reg_val[ch];
        search->attempts = 0U;
        search->searched = false;
        search->initialized = true;
    }

    if (abs_error < search->best_error)
    {
        search->best_error = abs_error;
        search->best_code = now_code;
        search->best_reg = trim_info->reg_val[ch];
    }

    ++search->attempts;

    drv_uart_printf(LOG_LV_INFO, "\r\n%s TRIAL: n=%u code=%u measured=%.3f %s error=%+.3f %s", prefix, (unsigned)search->attempts, (unsigned)now_code, (double)measured, unit, (double)error, unit);

    if (measured < target)
    {
        search->low = (int32_t)now_code + 1; // increase the lower bound since measured is less than target
    }
    else if (measured > target)
    {
        search->high = (int32_t)now_code - 1; // decrease the upper bound since measured is greater than target
    }
    else
    {
        search->low = search->high + 1;
    }

    uint16_t next_code = 0xFFFFU;
    bool next_verifying = false;

    if (search->low <= search->high)
    {
        next_code = (uint16_t)(search->low + (search->high - search->low) / 2);
    }
    else
    {
        next_code = search->best_code;
        next_verifying = true;
    }

    const uint16_t reg_val = apply_code(ch, next_code);

    if (UINT16_MAX == reg_val)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] [%s] %s: Trim register write failed, ch=%u code=%u", trim_get_list_name(gt_trim_list), __func__, (unsigned)ch, (unsigned)next_code);
        return trim_error_handler(td);
    }

    trim_info->trim_code[ch] = next_code;
    trim_info->reg_val[ch] = reg_val;
    search->searched = next_verifying;

    td->step = TRIM_STEP_SETUP_BY_LIST;
    td->tout = T_STABLE_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_prepare_efuse(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    LL_GPIO_SetOutputPin(DEBUG2_GPIO_Port, DEBUG2_Pin);

    if (false == id601_trim_save_search_registers())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] %s: Failed to save mirror registers", __func__);
        return trim_error_handler(td);
    }

    if (true == gb_id601_efuse_burn)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [TRIM] [OTP]: BURNING");
        drv_gpio_id601_vcc(ID601_VCC_5V5); // should check time for VDD ramp-up (3.3V -> 5.5V)
        td->step = TRIM_STEP_START_EFUSE;
        td->tout = T_RAMP_UP_MS;
    }
    else
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [TRIM] [OTP]: SKIPPED");
        td->step = TRIM_STEP_LOG_SUMMARY;
        td->tout = T_DEFAULT_MS;
    }
    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_start_efuse(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    id601_trim_setup_efuse();
    td->step = TRIM_STEP_END_EFUSE;
    td->tout = T_FUSING_DONE_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_end_efuse(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    drv_gpio_id601_vcc(ID601_VCC_OFF); // should check time for VDD ramp-down (5.5V -> 0V)
    td->step = TRIM_STEP_PWR_REBOOT;
    td->tout = T_RAMP_DOWN_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_pwr_reboot(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    if (false == id601_init_after_efuse_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] %s: Failed to initialize after efuse", __func__);
        return trim_error_handler(td);
    }

    td->step = TRIM_STEP_VERIFY_OTP_DUMP;
    td->tout = T_RAMP_DOWN_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_verify_otp_dump(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    if (false == id601_trim_compare_search_registers())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] [OTP VERIFY]: FAIL");
        return trim_error_handler(td);
    }
    drv_uart_printf(LOG_LV_INFO, "\r\n    [TRIM] [OTP VERIFY]: PASS");

    td->step = TRIM_STEP_LOG_SUMMARY;
    td->tout = T_RAMP_DOWN_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_log_summary(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    drv_uart_printf(LOG_LV_INFO, "\r\n    [TRIM] SUMMARY");
    drv_uart_printf(LOG_LV_INFO, "\r\n        %-10s %4s %5s %5s %10s %4s", "ITEM", "CH", "CODE", "REG", "VALUE", "UNIT");

    for (uint16_t list = TRIM_LIST_IREF1; list < TRIM_LIST_MAX; ++list)
    {
        const trim_info_t *info = &gt_trim_info[list];
        const char *list_name = trim_get_list_name((trim_list_t)list);
        const char *unit = trim_get_unit((trim_list_t)list);

        const bool has_channel = ((TRIM_LIST_GAIN == list) || (TRIM_LIST_OFFSET == list)) ? (true) : (false);
        const uint8_t max_ch = (has_channel) ? (12U) : (1U);

        for (uint8_t ch = 0U; ch < max_ch; ++ch)
        {
            char channel[4] = "-";
            if (has_channel)
            {
                snprintf(channel, sizeof(channel), "%2u", (unsigned)(ch + 1U));
            }
            drv_uart_printf(LOG_LV_INFO, "\r\n        %-10s %4s %5u %5u %10.3f %4s", list_name, channel, (unsigned)info->trim_code[ch], (unsigned)info->reg_val[ch], (double)info->value[ch], unit);
        }
    }

    drv_uart_printf(LOG_LV_INFO, "\r\n    [TRIM] COMPLETE: PASS, eFUSE: %s", (true == gb_id601_efuse_burn) ? ("PG DONE") : ("PG SKIP"));

    td->step = TRIM_STEP_PWR_OFF;
    td->tout = T_DEFAULT_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return true;
}

static bool trim_step_pwr_off(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    LL_GPIO_ResetOutputPin(DEBUG2_GPIO_Port, DEBUG2_Pin);

    drv_gpio_id601_vcc(ID601_VCC_OFF);
    drv_gpio_id601_vled(ID601_VLED_OFF);
    drv_gpio_id601_test_relay(ID601_RELAY_DIS);
    drv_gpio_id601_ch_demux(ID601_CH_MAX);
    drv_gpio_id601_iout_gain(ID601_IOUT_GAIN_MAX);

    drv_uart_printf(LOG_LV_INFO, "\r\n    [TRIM] Power Off");

    td->step = TRIM_STEP_NONE;
    td->tout = T_DEFAULT_MS;

    #ifdef RTOS_TIME_DEBUG
        LL_GPIO_TogglePin(DEBUG1_GPIO_Port, DEBUG1_Pin);
        drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TRIM] [%s] %s: Next execution in %u ms", trim_get_list_name(gt_trim_list), __func__, (unsigned)td->tout);
    #endif
    return false; // Power off step always returns false to indicate termination of the trim process
}

void id601_trim_clear_parameters(thread_data_t *td)
{
    USER_ASSERT(NULL != td);

    drv_gpio_id601_vcc(ID601_VCC_OFF);
    drv_gpio_id601_vled(ID601_VLED_OFF);
    drv_gpio_id601_test_relay(ID601_RELAY_DIS);
    drv_gpio_id601_ch_demux(ID601_CH_MAX);
    drv_gpio_id601_iout_gain(ID601_IOUT_GAIN_MAX);

    gt_trim_list = TRIM_LIST_NONE;

    memset(gt_trim_info, 0, sizeof(gt_trim_info)); //flush all trim information

    for (uint16_t list = TRIM_LIST_IREF1; list < TRIM_LIST_MAX; ++list)
    {
        const uint16_t channel_max = ((TRIM_LIST_GAIN == list) || (TRIM_LIST_OFFSET == list)) ? (12U) : (1U);
        const uint16_t initial_code = ((gn_trim_code_max[list] + 1U) >> 1U);
        for (uint16_t ch = 0U; ch < channel_max; ++ch)
        {
            gt_trim_info[list].trim_code[ch] = initial_code; // Set initial trim code to the midpoint of the allowable range
        }
    }

    td->step = TRIM_STEP_NONE;
    td->func = trim_step_none;
    td->tout = 10U;
}

void id601_trim_start(thread_data_t *td)
{
    USER_ASSERT(NULL != td);

    id601_trim_clear_parameters(td);

    gt_trim_list = TRIM_LIST_IREF1;

    td->step = TRIM_STEP_INITIAL;
    td->tout = 10U;
    td->func = trim_step_initial;
}

bool id601_trim_process(thread_data_t *td)
{
    USER_ASSERT(NULL != td);

    const uint32_t step = td->step;
    if ((TRIM_STEP_MAX <= step) || (NULL == gp_trim_step_handlers[step]))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TRIM] [%s] %s: Invalid trim step=%u", trim_get_list_name(gt_trim_list), __func__, (unsigned)step);
        return false;
    }

    return gp_trim_step_handlers[step](td);
}
/* USER CODE END 0 */
