/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : id601_test.c
    * @brief          : id601_test implementation
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
#include "id601_test.h"
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
#include "id601_dimming.h"

extern uint16_t id601_get_trim_search_mirror_reg(uint16_t addr);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef bool (*test_setup_func_t)(void);

typedef enum tag_TEST_STEP_T
{
    TEST_STEP_NONE = 0U,
    TEST_STEP_INITIAL,
    TEST_STEP_SETUP_BY_LIST,
    TEST_STEP_VSYNC_STOP,
    TEST_STEP_START_MEASURE,
    TEST_STEP_GET_MEASURED_VALUE,
    TEST_STEP_LOG_SUMMARY,
    TEST_STEP_PWR_OFF,
    TEST_STEP_MAX,
} test_step_t;

typedef enum tag_TEST_LIST_T
{
    TEST_LIST_NONE = 0U,
    TEST_LIST_IDDS1,
    TEST_LIST_IDDD1,
    TEST_LIST_IREF1,
    TEST_LIST_IREF2,
    TEST_LIST_LDO_DET,
    TEST_LIST_LDO_DIG,
    TEST_LIST_LDO_DAC,
    TEST_LIST_LDO_FLL,
    TEST_LIST_BGR1,
    TEST_LIST_BGR2,
    TEST_LIST_OSC,
    TEST_LIST_FLL1,
    TEST_LIST_FLL2,
    TEST_LIST_FLL3,
    TEST_LIST_IOUT_3P,
    TEST_LIST_IOUT_SWEEP,
    TEST_LIST_IOUT_VREF_SWEEP,
    TEST_LIST_MAX,
} test_list_t;

typedef struct tag_TEST_INFO
{
    float value[12][16]; //max channel, max curr level
    uint16_t ch;
    uint16_t test_phase;
} test_info_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define T_DEFAULT_MS                (1U)
#define T_STABLE_MS                 (10U)
#define T_VSYNC_STOP_MS             (200U)
#define T_MEASURE_DONE_MCU_ADC_MS   (1U)
#define T_MEASURE_DONE_MCU_TIM_MS   (60U)
#define T_MEASURE_DONE_ADS_ADC_MS   (10U)

#define VREF_INC                    (15U)
#if (VREF_INC < 1U) || (VREF_INC > 4095U)
    #error "VREF_INC must be between 1 and 4095"
#endif
#define VREF_MAX                    (4095U)
#if (VREF_MAX > 4095U)
    #error "VREF_MAX must be between 1 and 4095"
#endif

#define TEST_IOUT_GAIN              (ID601_IOUT_GAIN_HIGH)
#define TEST_IOUT_VREF_SWEEP_GAIN   (ID601_IOUT_GAIN_MID)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static bool test_step_none(thread_data_t *td);
static bool test_step_initial(thread_data_t *td);
static bool test_step_setup_by_list(thread_data_t *td);
static bool test_step_vsync_stop(thread_data_t *td);
static bool test_step_start_measure(thread_data_t *td);
static bool test_step_get_measured_value(thread_data_t *td);
static bool test_step_log_summary(thread_data_t *td);
static bool test_step_pwr_off(thread_data_t *td);
/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern osThreadId_t ID601TestTaskHandle;

static const test_step_handler_t gp_test_step_handlers[TEST_STEP_MAX] =
{
    [TEST_STEP_NONE]               = test_step_none,
    [TEST_STEP_INITIAL]            = test_step_initial,
    [TEST_STEP_SETUP_BY_LIST]      = test_step_setup_by_list,
    [TEST_STEP_VSYNC_STOP]         = test_step_vsync_stop,
    [TEST_STEP_START_MEASURE]      = test_step_start_measure,
    [TEST_STEP_GET_MEASURED_VALUE] = test_step_get_measured_value,
    [TEST_STEP_LOG_SUMMARY]        = test_step_log_summary,
    [TEST_STEP_PWR_OFF]            = test_step_pwr_off,
};

static const test_setup_func_t gp_test_setup_by_list[TEST_LIST_MAX] =
{
    [TEST_LIST_IDDS1] = id601_test_setup_idds1,
    [TEST_LIST_IDDD1] = id601_test_setup_iddd1,
    [TEST_LIST_IREF1] = id601_test_setup_iref1,
    [TEST_LIST_IREF2] = id601_test_setup_iref2,
    [TEST_LIST_LDO_DET] = id601_test_setup_ldo_det,
    [TEST_LIST_LDO_DIG] = id601_test_setup_ldo_dig,
    [TEST_LIST_LDO_DAC] = id601_test_setup_ldo_dac,
    [TEST_LIST_LDO_FLL] = id601_test_setup_ldo_fll,
    [TEST_LIST_BGR1] = id601_test_setup_bgr1,
    [TEST_LIST_BGR2] = id601_test_setup_bgr2,
    [TEST_LIST_OSC] = id601_test_setup_osc,
    [TEST_LIST_FLL1] = id601_test_setup_fll1,
    [TEST_LIST_FLL2] = id601_test_setup_fll2,
    [TEST_LIST_FLL3] = id601_test_setup_fll3,
    [TEST_LIST_IOUT_3P] = id601_test_setup_iout_3p,
    [TEST_LIST_IOUT_SWEEP] = id601_test_setup_iout_sweep,
    [TEST_LIST_IOUT_VREF_SWEEP] = id601_test_setup_iout_vref_sweep,
};

static const char* const gp_test_list_name[TEST_LIST_MAX] =
{
    [TEST_LIST_IDDS1]      = "IDDS1",
    [TEST_LIST_IDDD1]      = "IDDD1",
    [TEST_LIST_IREF1]      = "IREF1",
    [TEST_LIST_IREF2]      = "IREF2",
    [TEST_LIST_LDO_DET]    = "LDO_DET",
    [TEST_LIST_LDO_DIG]    = "LDO_DIG",
    [TEST_LIST_LDO_DAC]    = "LDO_DAC",
    [TEST_LIST_LDO_FLL]    = "LDO_FLL",
    [TEST_LIST_BGR1]       = "BGR1",
    [TEST_LIST_BGR2]       = "BGR2",
    [TEST_LIST_OSC]        = "OSC",
    [TEST_LIST_FLL1]       = "FLL1",
    [TEST_LIST_FLL2]       = "FLL2",
    [TEST_LIST_FLL3]       = "FLL3",
    [TEST_LIST_IOUT_3P]    = "IOUT_3P",
    [TEST_LIST_IOUT_SWEEP] = "IOUT_SWEEP",
    [TEST_LIST_IOUT_VREF_SWEEP] = "IOUT_VREF_SWEEP",
};

static bool gb_id601_hvs_flag;
static test_list_t gt_test_list;
static test_info_t gt_test_info[TEST_LIST_MAX];
static uint8_t gn_id601_test_iout_gain;

static bool gb_id601_test_vref_sweep;
static float gf_vref_sweep_row[12];

static bool gb_id601_iout_monitor_enable = false;
static uint16_t gn_id601_iout_monitor_channel = 0U;
static uint32_t gn_id601_iout_monitor_tick = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

bool drv_id601_set_hvs_flag(bool hvs_flag)
{
    gb_id601_hvs_flag = hvs_flag;
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Set HVS flag: %s.", hvs_flag ? "true" : "false");
    return true;
}
bool drv_id601_get_hvs_flag(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Get HVS flag: %s.", gb_id601_hvs_flag ? "true" : "false");
    return gb_id601_hvs_flag;
}
bool id601_test_setup_idds1(void)
{
    if (true == gb_id601_hvs_flag)
    {
        drv_gpio_id601_vcc(ID601_VCC_5V5);
        drv_tim_delay_ms(490U); // total 500ms (10ms @ drv_gpio_id601_vcc function)
        drv_gpio_id601_vcc(ID601_VCC_OFF);
        drv_tim_delay_ms(4990U); // total 500ms (10ms @ drv_gpio_id601_vcc function)
    }
    drv_gpio_id601_vcc(ID601_VCC_3V3);
    return true;
}
bool id601_test_setup_iddd1(void)
{
    if (false == id601_init_tm_entry_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 TM Entry Mode Failed", __func__);
        return false;
    }

    if (false == id601_set_analog_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_iref1(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());

    const bool already_trimmed = id601_check_is_trimmed();
    if (true == already_trimmed)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Already trimmed. Skip overwrite");
    }
    else
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Overwrite Mirror register");
        for (uint16_t addr = 0U; addr < ID601_ADDR_MIRROR_MAX_COUNT; ++addr)
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Overwriting... addr 0x%02X value=0x%03X", addr, id601_get_trim_search_mirror_reg(addr));
            if (false == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, addr, id601_get_trim_search_mirror_reg(addr), drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to overwrite mirror register: addr 0x%02X", addr);
                return false;
            }
        }
    }

    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }

    drv_gpio_id601_test_relay(ID601_RELAY_CON);
    return true;
}
bool id601_test_setup_iref2(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF2_TEST))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }

    drv_gpio_id601_test_relay(ID601_RELAY_CON);
    return true;
}
bool id601_test_setup_ldo_det(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_VREF1P5_BGR2_RES))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }

    drv_gpio_id601_test_relay(ID601_RELAY_DIS);
    return true;
}
bool id601_test_setup_ldo_dig(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_DIG_RES))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_ldo_dac(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_DAC_RES))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_ldo_fll(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_FLL_RES))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_bgr1(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_VREF_TEST))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_bgr2(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_VREF2_TEST))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_osc(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 1U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }

    const uint16_t osc_man_en = 1U;
    const uint16_t osc_fll_err_range = 0U;
    const uint16_t osc_fll_man = 8U;
    if (false == id601_write_osc_fll_man2(1U, osc_man_en, osc_fll_err_range, osc_fll_man))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OSC_FLL_MAN2.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_fll1(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_set_fll_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 FLL Test Mode Failed", __func__);
        return false;
    }

    const uint64_t tgt_hz = 40000000ULL;
    const uint64_t fllcnt = (tgt_hz / (120U * 2U));
    const uint16_t fll_lsb = (uint16_t)(fllcnt & 0x0FFFU);
    const uint16_t fll_msb = (uint16_t)((fllcnt >> 12U) & 0x0FFFU);

    const uint16_t fllcnt1 = fll_lsb;
    if (false == id601_write_fllcnt1(1U, fllcnt1))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write FLLCNT1.", __func__);
        return false;
    }

    const uint16_t fllcnt2_en = 0x1U;
    const uint16_t fllcnt2_rng = 0x2U;
    const uint16_t fllcnt2_cnt = fll_msb;
    if (false == id601_write_fllcnt2(1U, fllcnt2_en, fllcnt2_rng, fllcnt2_cnt))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write FLLCNT2.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_fll2(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_set_fll_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 FLL Test Mode Failed", __func__);
        return false;
    }

    const uint64_t tgt_hz = 60000000ULL;
    const uint64_t fllcnt = (tgt_hz / (120U * 2U));
    const uint16_t fll_lsb = (uint16_t)(fllcnt & 0x0FFFU);
    const uint16_t fll_msb = (uint16_t)((fllcnt >> 12U) & 0x0FFFU);

    const uint16_t fllcnt1 = fll_lsb;
    if (false == id601_write_fllcnt1(1U, fllcnt1))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write FLLCNT1.", __func__);
        return false;
    }

    const uint16_t fllcnt2_en = 0x1U;
    const uint16_t fllcnt2_rng = 0x2U;
    const uint16_t fllcnt2_cnt = fll_msb;
    if (false == id601_write_fllcnt2(1U, fllcnt2_en, fllcnt2_rng, fllcnt2_cnt))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write FLLCNT2.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_fll3(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_set_fll_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 FLL Test Mode Failed", __func__);
        return false;
    }

    const uint64_t tgt_hz = 50000000ULL;
    const uint64_t fllcnt = (tgt_hz / (120U * 2U));
    const uint16_t fll_lsb = (uint16_t)(fllcnt & 0x0FFFU);
    const uint16_t fll_msb = (uint16_t)((fllcnt >> 12U) & 0x0FFFU);

    const uint16_t fllcnt1 = fll_lsb;
    if (false == id601_write_fllcnt1(1U, fllcnt1))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write FLLCNT1.", __func__);
        return false;
    }

    const uint16_t fllcnt2_en = 0x1U;
    const uint16_t fllcnt2_rng = 0x2U;
    const uint16_t fllcnt2_cnt = fll_msb;
    if (false == id601_write_fllcnt2(1U, fllcnt2_en, fllcnt2_rng, fllcnt2_cnt))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write FLLCNT2.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_iout_3p(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());

    if (false == id601_set_analog_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
        return false;
    }

    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }

    const uint16_t gain_max_current_level3 = ID601_VFY_MAX_CURR_LVL;
    const uint16_t gain_max_current_level2 = ID601_VFY_MAX_CURR_LVL;
    const uint16_t gain_max_current_level1 = ID601_VFY_MAX_CURR_LVL;
    if (false == id601_write_max_current_level(1U, gain_max_current_level3, gain_max_current_level2, gain_max_current_level1))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURRENT_LEVEL.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_iout_3p_x(uint8_t ph)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    USER_ASSERT(ph < 3);

    const uint16_t vref_table[3] = {ID601_VFY_3P_VREF1, ID601_VFY_3P_VREF2, ID601_VFY_3P_VREF3};
    const uint8_t table_idx = ph;
    if (false == id601_write_max_curr_vref1(1U, vref_table[table_idx]))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF1.", __func__);
        return false;
    }
    if (false == id601_write_max_curr_vref2(1U, vref_table[table_idx]))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF2.", __func__);
        return false;
    }
    if (false == id601_write_max_curr_vref3(1U, vref_table[table_idx]))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF3.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_iout_sweep(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }

    if (false == id601_write_max_curr_vref1(1U, ID601_VFY_SWEEP_VREF))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF1.", __func__);
        return false;
    }
    if (false == id601_write_max_curr_vref2(1U, ID601_VFY_SWEEP_VREF))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF2.", __func__);
        return false;
    }
    if (false == id601_write_max_curr_vref3(1U, ID601_VFY_SWEEP_VREF))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF3.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_iout_sweep_x(uint8_t ph)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    USER_ASSERT(ph <= ID601_MAX_CURRENT_LEVEL_64mA);

    const uint16_t gain_max_current_level3 = (ID601_MAX_CURRENT_LEVEL_04mA + ph);
    const uint16_t gain_max_current_level2 = (ID601_MAX_CURRENT_LEVEL_04mA + ph);
    const uint16_t gain_max_current_level1 = (ID601_MAX_CURRENT_LEVEL_04mA + ph);
    if (false == id601_write_max_current_level(1U, gain_max_current_level3, gain_max_current_level2, gain_max_current_level1))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURRENT_LEVEL.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_iout_vref_sweep(void)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    if (false == id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write TEST_ANA_EN.", __func__);
        return false;
    }

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    if (false == id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write OP_MODE.", __func__);
        return false;
    }

    const uint16_t gain_max_current_level3 = ID601_VFY_MAX_CURR_LVL;
    const uint16_t gain_max_current_level2 = ID601_VFY_MAX_CURR_LVL;
    const uint16_t gain_max_current_level1 = ID601_VFY_MAX_CURR_LVL;
    if (false == id601_write_max_current_level(1U, gain_max_current_level3, gain_max_current_level2, gain_max_current_level1))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURRENT_LEVEL.", __func__);
        return false;
    }
    return true;
}
bool id601_test_setup_iout_vref_sweep_x(uint16_t vref)
{
    USER_ASSERT(1U == drv_id601_get_daisy_length());
    USER_ASSERT(vref <= 4095);

    if (false == id601_write_max_curr_vref1(1U, vref))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF1.", __func__);
        return false;
    }
    if (false == id601_write_max_curr_vref2(1U, vref))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF2.", __func__);
        return false;
    }
    if (false == id601_write_max_curr_vref3(1U, vref))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to Write MAX_CURR_VREF3.", __func__);
        return false;
    }
    return true;
}

bool id601_manual_test_setup(uint16_t test_list, uint16_t option1, uint16_t option2, uint16_t option3, uint16_t option4)
{
    drv_tim_vsync_stop();
    drv_gpio_clear_id601();
    drv_gpio_id601_vcc(ID601_VCC_3V3);
    if (false == id601_init_tm_entry_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 TM Entry Mode Failed", __func__);
        return false;
    }

    USER_ASSERT(1U == drv_id601_get_daisy_length());

    switch (test_list)
    {
        case MANUAL_TEST_DRIVING_VOLTAGE:
        {
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                return false;
            }
            id601_test_iout_monitor_start((uint16_t)(option1 - 1U), 1U, 4095U);
            break;
        }
        case MANUAL_TEST_UV_VDD:
        {
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                return false;
            }
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00A, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                return false;
            }
            else
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00A, 0x00A);
            }
            break;
        }
        case MANUAL_TEST_UVOV_LDO:
        {
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                return false;
            }
            switch (option1)
            {
                case 1U:
                {
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00D, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                        return false;
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00D, 0x00D);
                    }
                    break;
                }
                case 2U:
                {
                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_TEST_ANA_EN, ID601_ADDR_GENERAL_TEST_MODE0 };
                    const uint16_t setting_data[3] = { 0x86C, 0x00D, 0x00A };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                case 3U:
                {
                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_TEST_ANA_EN, ID601_ADDR_GENERAL_TEST_MODE0 };
                    const uint16_t setting_data[3] = { 0x86C, 0x00D, 0x009 };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                default:
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Invalid option1: %u", (unsigned)option1);
                    return false;
                    break;
                }
            }
            break;
        }
        case MANUAL_TEST_UVOV_DAC:
        {
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                return false;
            }
            switch (option1)
            {
                case 1U:
                {
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00E, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                        return false;
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00E, 0x00E);
                    }
                    break;
                }
                case 2U:
                {
                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_TEST_ANA_EN, ID601_ADDR_GENERAL_TEST_MODE0 };
                    const uint16_t setting_data[3] = { 0x86C, 0x00E, 0x00E };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                case 3U:
                {
                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_TEST_ANA_EN, ID601_ADDR_GENERAL_TEST_MODE0 };
                    const uint16_t setting_data[3] = { 0x86C, 0x00E, 0x00D };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                default:
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Invalid option1: %u", (unsigned)option1);
                    return false;
                    break;
                }
            }
            break;
        }
        case MANUAL_TEST_UVOV_OSC:
        {
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                return false;
            }
            switch (option1)
            {
                case 1U:
                {
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00F, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                        return false;
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00F, 0x00F);
                    }
                    break;
                }
                case 2U:
                {
                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_TEST_ANA_EN, ID601_ADDR_GENERAL_TEST_MODE0 };
                    const uint16_t setting_data[3] = { 0x86C, 0x00F, 0x00B };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                case 3U:
                {
                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_TEST_ANA_EN, ID601_ADDR_GENERAL_TEST_MODE0 };
                    const uint16_t setting_data[3] = { 0x86C, 0x00F, 0x00C };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                default:
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Invalid option1: %u", (unsigned)option1);
                    return false;
                    break;
                }
            }
            break;
        }
        case MANUAL_TEST_OPEN_DETECTOR:
        {
            switch (option1)
            {
                case 1U:
                {
                    if (false == id601_set_analog_test_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                        return false;
                    }

                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_TEST_ANA_EN, ID601_ADDR_GENERAL_FAULT_CONTROL };
                    const uint16_t setting_data[3] = { 0x844, 0x019, 0x03C };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                case 2U:
                case 3U:
                {
                    if (false == id601_set_dimming_type1_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Set Dimming Type1 Mode Failed", __func__);
                        return false;
                    }

                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_CHANNEL_ENABLE };
                    const uint16_t setting_data[3] = { 0x044, 0x03C, (1U << (option2 - 1U)) };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }

                    for (uint16_t ch = 0; ch < 12; ++ch)
                    {
                        id601_set_ld_buffer(0, ch, 0, 16383);
                        id601_set_ld_buffer(0, ch, 1, 16383);
                        id601_set_ld_buffer(0, ch, 2, 16383);
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] LD Buffer: All Channels Set To Max");

                    drv_tim_vsync_start();
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Vsync started");
                    break;
                }
                case 4U:
                {
                    if (false == id601_set_dimming_type1_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Set Dimming Type1 Mode Failed", __func__);
                        return false;
                    }

                    const uint16_t ch_enable_data = (1U << (option2 - 1U));
                    const uint16_t setting_addr[3] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_CHANNEL_ENABLE };
                    const uint16_t setting_data[3] = { 0x044, 0x03D, ch_enable_data };
                    for (uint16_t i = 0; i < 3U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }

                    for (uint16_t ch = 0; ch < 12; ++ch)
                    {
                        id601_set_ld_buffer(0, ch, 0, 16383);
                        id601_set_ld_buffer(0, ch, 1, 16383);
                        id601_set_ld_buffer(0, ch, 2, 16383);
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] LD Buffer: All Channels Set To Max");

                    drv_tim_vsync_start();
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Vsync started");
                    break;
                }
                default:
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Invalid option1: %u", (unsigned)option1);
                    return false;
                    break;
                }
            }
            break;
        }
        case MANUAL_TEST_FB_DETECTOR:
        {
            switch (option1)
            {
                case 1U:
                {
                    if (false == id601_set_analog_test_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                        return false;
                    }

                    const uint16_t fb_level_data = ((option4 << 6U) | (option4 << 3U) | (option4 << 0U));
                    const uint16_t ch_enable_data = (1U << (option2 - 1U));
                    const uint16_t setting_addr[5] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_CHANNEL_ENABLE, ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_FB_LEVEL, ID601_ADDR_GENERAL_TEST_ANA_EN };
                    const uint16_t setting_data[5] = { 0x844, ch_enable_data, 0x03C, fb_level_data, (0x015U + option3) };
                    for (uint16_t i = 0; i < 5U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                case 2U:
                {
                    if (false == id601_set_analog_test_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                        return false;
                    }

                    if (false == id601_set_dimming_type1_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Set Dimming Type1 Mode Failed", __func__);
                        return false;
                    }

                    const uint32_t op_mode_data = (0x804 | (option3 << 6U));
                    const uint16_t ch_enable_data = (1U << (option2 - 1U));
                    const uint16_t fb_level_data = ((option4 << 6U) | (option4 << 3U) | (option4 << 0U));
                    const uint16_t setting_addr[5] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_CHANNEL_ENABLE, ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_FB_LEVEL, ID601_ADDR_GENERAL_TEST_ANA_EN };
                    const uint16_t setting_data[5] = { op_mode_data, ch_enable_data, 0x03C, fb_level_data, (0x015U + option3) };
                    for (uint16_t i = 0; i < 5U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }

                    for (uint16_t ch = 0; ch < 12; ++ch)
                    {
                        id601_set_ld_buffer(0, ch, 0, 16383);
                        id601_set_ld_buffer(0, ch, 1, 16383);
                        id601_set_ld_buffer(0, ch, 2, 16383);
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] LD Buffer: All Channels Set To Max");

                    drv_tim_vsync_start();
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Vsync started");
                    break;
                }
                default:
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Invalid option1: %u", (unsigned)option1);
                    return false;
                }
            }
            break;
        }
        case MANUAL_TEST_SHORT_DETECTOR:
        {
            switch (option1)
            {
                case 1U:
                {
                    if (false == id601_set_analog_test_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                        return false;
                    }

                    const uint16_t short_level_data = ((option3 << 6U) | (option3 << 3U) | (option3 << 0U));
                    const uint16_t setting_addr[4] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_TEST_ANA_EN, ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_SHORT_LEVEL };
                    const uint16_t setting_data[4] = { 0x844U, 0x01AU, 0x03CU, short_level_data };
                    for (uint16_t i = 0; i < 4U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }
                    break;
                }
                case 2U:
                case 3U:
                {
                    if (false == id601_set_dimming_type1_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Dimming Type1 Mode Failed", __func__);
                        return false;
                    }

                    const uint16_t ch_enable_data = (1U << (option2 - 1U));
                    const uint16_t short_level_data = ((option3 << 6U) | (option3 << 3U) | (option3 << 0U));
                    const uint16_t setting_addr[4] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_CHANNEL_ENABLE, ID601_ADDR_GENERAL_SHORT_LEVEL };
                    const uint16_t setting_data[4] = { 0x044U, 0x03CU, ch_enable_data, short_level_data };
                    for (uint16_t i = 0; i < 4U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }

                    for (uint16_t ch = 0; ch < 12; ++ch)
                    {
                        id601_set_ld_buffer(0, ch, 0, 16383);
                        id601_set_ld_buffer(0, ch, 1, 16383);
                        id601_set_ld_buffer(0, ch, 2, 16383);
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] LD Buffer: All Channels Set To Max");

                    drv_tim_vsync_start();
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Vsync started");
                    break;
                }
                case 4U:
                {
                    if (false == id601_set_dimming_type1_mode())
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Dimming Type1 Mode Failed", __func__);
                        return false;
                    }

                    const uint16_t ch_enable_data = (1U << (option2 - 1U));
                    const uint16_t short_level_data = ((option3 << 6U) | (option3 << 3U) | (option3 << 0U));
                    const uint16_t setting_addr[4] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_CHANNEL_ENABLE, ID601_ADDR_GENERAL_SHORT_LEVEL };
                    const uint16_t setting_data[4] = { 0x044U, 0x03EU, ch_enable_data, short_level_data };
                    for (uint16_t i = 0; i < 4U; i++)
                    {
                        if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                        {
                            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                            return false;
                        }
                        else
                        {
                            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                        }
                    }

                    for (uint16_t ch = 0; ch < 12; ++ch)
                    {
                        id601_set_ld_buffer(0, ch, 0, 16383);
                        id601_set_ld_buffer(0, ch, 1, 16383);
                        id601_set_ld_buffer(0, ch, 2, 16383);
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] LD Buffer: All Channels Set To Max");

                    drv_tim_vsync_start();
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Vsync started");
                    break;
                }
                default:
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Invalid option1: %u", (unsigned)option1);
                    return false;
                }
            }
            break;
        }
        case MANUAL_TEST_FAULT_FLAG:
        {
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                return false;
            }

            const uint32_t op_mode_data = (0x824 | (option1 << 6U));
            const uint16_t setting_addr[4] = { ID601_ADDR_GENERAL_OP_MODE, ID601_ADDR_GENERAL_CHANNEL_ENABLE, ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_FB_LEVEL };
            const uint16_t setting_data[4] = { op_mode_data, 0xFFFU, 0x03CU, 0x16DU };
            for (uint16_t i = 0; i < 4U; i++)
            {
                if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                    return false;
                }
                else
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                }
            }

            for (uint16_t ch = 0; ch < 12; ++ch)
            {
                id601_set_ld_buffer(0, ch, 0, 16383);
                id601_set_ld_buffer(0, ch, 1, 16383);
                id601_set_ld_buffer(0, ch, 2, 16383);
            }
            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] LD Buffer: All Channels Set To Max");

            drv_tim_vsync_start();
            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Vsync started");

            break;
        }
        case MANUAL_TEST_BIST:
        {
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
                return false;
            }

            const uint16_t setting_addr[2] = { ID601_ADDR_GENERAL_FAULT_CONTROL, ID601_ADDR_GENERAL_BIST };
            const uint16_t setting_data[2] = { 0x000, 0x800 };
            for (uint16_t i = 0; i < 2U; i++)
            {
                if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, setting_addr[i], setting_data[i], drv_id601_get_crc_func()))
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to write to ID601 register.");
                    return false;
                }
                else
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", setting_addr[i], setting_data[i], setting_data[i]);
                }
            }

            drv_tim_delay_us(50U);

            uint16_t read_data = 0xFFF;
            if (false == id601_read_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_BIST, drv_id601_get_crc_func(), &read_data))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to read from ID601 register.");
            }
            else
            {
                if (0x02U == read_data)
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] [BIST] Addr:0x%02X, Data:%4u(0x%03X) (PASS)", ID601_ADDR_GENERAL_BIST, read_data, read_data);
                }
                else
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] [BIST] Addr:0x%02X, Data:%4u(0x%03X) (FAIL)", ID601_ADDR_GENERAL_BIST, read_data, read_data);
                }
            }
            break;
        }
        case MANUAL_TEST_HVS:
        {
            drv_id601_set_hvs_flag(true);
            test_set_vref_sweep_enable(false);
            osThreadFlagsSet(ID601TestTaskHandle, TEST_START_FLAG);
            break;
        }
        default:
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Invalid test list: %u", (unsigned)test_list);
            return false;
        }
    }
    return true;
}

static const char *test_get_list_name(test_list_t list)
{
    if ((unsigned)TEST_LIST_MAX <= (unsigned)list)
    {
        return "INVALID";
    }

    return gp_test_list_name[list];
}
static inline bool test_error_handler(thread_data_t *td)
{
    td->step = TEST_STEP_PWR_OFF;
    td->tout = T_DEFAULT_MS;
    return true;
}

static bool test_step_none(thread_data_t *td)
{
    (void)td;
    return false;
}

static bool test_step_initial(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    gn_id601_test_iout_gain = TEST_IOUT_GAIN;
    drv_gpio_id601_iout_gain(gn_id601_test_iout_gain);

    td->step = TEST_STEP_SETUP_BY_LIST;
    td->tout = T_DEFAULT_MS;
    return true;
}

static bool test_step_setup_by_list(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);
    USER_ASSERT(TEST_LIST_NONE < gt_test_list);
    USER_ASSERT(TEST_LIST_MAX > gt_test_list);
    USER_ASSERT(NULL != gp_test_setup_by_list[gt_test_list]);

    const uint16_t ch = gt_test_info[gt_test_list].ch;
    const uint16_t phase = gt_test_info[gt_test_list].test_phase;

    if (false == gp_test_setup_by_list[gt_test_list]())
    {
        return test_error_handler(td);
    }

    switch (gt_test_list)
    {
        case TEST_LIST_IDDS1:
        case TEST_LIST_IDDD1:
        {
            LL_GPIO_SetOutputPin(DEBUG1_GPIO_Port, DEBUG1_Pin);
            ads114s08_set_input_mux(ADS_DEV_1, ADS_DEV1_AIN02_ID601_ICC_P, ADS_DEV1_AIN03_ID601_ICC_N);
            td->step = TEST_STEP_START_MEASURE;
            td->tout = T_STABLE_MS;
            break;
        }
        case TEST_LIST_IREF1:
        case TEST_LIST_IREF2:
        case TEST_LIST_LDO_DET:
        case TEST_LIST_LDO_DIG:
        case TEST_LIST_LDO_DAC:
        case TEST_LIST_LDO_FLL:
        case TEST_LIST_BGR1:
        case TEST_LIST_BGR2:
        {
            td->step = TEST_STEP_START_MEASURE;
            td->tout = T_STABLE_MS;
            break;
        }
        case TEST_LIST_OSC:
        case TEST_LIST_FLL1:
        case TEST_LIST_FLL2:
        case TEST_LIST_FLL3:
        {
            drv_tim_test_vsync_start();
            td->step = TEST_STEP_VSYNC_STOP;
            td->tout = T_VSYNC_STOP_MS;
            break;
        }
        case TEST_LIST_IOUT_3P:
        {
            gn_id601_test_iout_gain = TEST_IOUT_VREF_SWEEP_GAIN;
            drv_gpio_id601_iout_gain(gn_id601_test_iout_gain);

            drv_gpio_id601_vled(ID601_VLED_ON);
            drv_gpio_id601_ch_demux(ch);
            if (false == id601_set_current_chx(ch))
            {
                return test_error_handler(td);
            }
            ads114s08_set_input_mux(ADS_DEV_1, ADS_DEV1_AIN00_ID601_IOUT, ADS_INPUT_N_GND);
            if (false == id601_test_setup_iout_3p_x(phase))
            {
                return test_error_handler(td);
            }
            td->step = TEST_STEP_START_MEASURE;
            td->tout = T_STABLE_MS;
            break;
        }
        case TEST_LIST_IOUT_SWEEP:
        {
            gn_id601_test_iout_gain = TEST_IOUT_VREF_SWEEP_GAIN;
            drv_gpio_id601_iout_gain(gn_id601_test_iout_gain);

            drv_gpio_id601_vled(ID601_VLED_ON);
            drv_gpio_id601_ch_demux(ch);
            if (false == id601_set_current_chx(ch))
            {
                return test_error_handler(td);
            }
            ads114s08_set_input_mux(ADS_DEV_1, ADS_DEV1_AIN00_ID601_IOUT, ADS_INPUT_N_GND);
            if (false == id601_test_setup_iout_sweep_x(phase))
            {
                return test_error_handler(td);
            }
            td->step = TEST_STEP_START_MEASURE;
            td->tout = T_STABLE_MS;
            break;
        }
        case TEST_LIST_IOUT_VREF_SWEEP:
        {
            drv_gpio_id601_vled(ID601_VLED_ON);
            drv_gpio_id601_ch_demux(ch);
            if (false == id601_set_current_chx(ch))
            {
                return test_error_handler(td);
            }
            ads114s08_set_input_mux(ADS_DEV_1, ADS_DEV1_AIN00_ID601_IOUT, ADS_INPUT_N_GND);
            if (false == id601_test_setup_iout_vref_sweep_x(phase))
            {
                return test_error_handler(td);
            }
            td->step = TEST_STEP_START_MEASURE;
            td->tout = T_STABLE_MS;
            break;
        }
        default:
        {
            USER_ASSERT(false);
            break;
        }
    }
    return true;
}

static bool test_step_vsync_stop(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    drv_tim_test_vsync_stop();

    td->step = TEST_STEP_START_MEASURE;
    td->tout = T_STABLE_MS;
    return true;
}

static bool test_step_start_measure(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);
    USER_ASSERT(TEST_LIST_NONE < gt_test_list);
    USER_ASSERT(TEST_LIST_MAX > gt_test_list);

    switch (gt_test_list)
    {
        case TEST_LIST_IDDS1:
        case TEST_LIST_IDDD1:
        {
            ads114s08_set_conversion_enable(ADS_DEV_1, true);
            td->tout = T_MEASURE_DONE_ADS_ADC_MS;
            break;
        }
        case TEST_LIST_IREF1:
        case TEST_LIST_IREF2:
        case TEST_LIST_LDO_DET:
        case TEST_LIST_LDO_DIG:
        case TEST_LIST_LDO_DAC:
        case TEST_LIST_LDO_FLL:
        case TEST_LIST_BGR1:
        case TEST_LIST_BGR2:
        {
            mcu_adc_set_conversion_enable(true);
            td->tout = T_MEASURE_DONE_MCU_ADC_MS;
            break;
        }
        case TEST_LIST_OSC:
        case TEST_LIST_FLL1:
        case TEST_LIST_FLL2:
        case TEST_LIST_FLL3:
        {
            mcu_tim_set_conversion_enable(true);
            td->tout = T_MEASURE_DONE_MCU_TIM_MS;
            break;
        }
        case TEST_LIST_IOUT_3P:
        case TEST_LIST_IOUT_SWEEP:
        {
            ads114s08_set_conversion_enable(ADS_DEV_1, true);
            td->tout = T_MEASURE_DONE_ADS_ADC_MS;
            break;
        }
        case TEST_LIST_IOUT_VREF_SWEEP:
        {
            ads114s08_set_conversion_enable(ADS_DEV_1, true);
            td->tout = T_MEASURE_DONE_ADS_ADC_MS;
            break;
        }
        default:
        {
            USER_ASSERT(false);
            break;
        }
    }
    td->step = TEST_STEP_GET_MEASURED_VALUE;
    return true;
}

static bool test_step_get_measured_value(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);
    USER_ASSERT(TEST_LIST_NONE < gt_test_list);
    USER_ASSERT(TEST_LIST_MAX > gt_test_list);

    test_info_t* test_info = &gt_test_info[gt_test_list];

    const uint16_t ch = test_info->ch;
    const uint16_t phase = test_info->test_phase;

    bool conversion_success = false;

    switch (gt_test_list)
    {
        case TEST_LIST_IDDS1:
        case TEST_LIST_IDDD1:
        {
            if (true == ads114s08_wait_conversion_complete())
            {
                int32_t adc_value = ads114s08_get_average_conversion(ADS_DEV_1);
                const float voltage_mv = ads114s08_conversion_adc_to_mvoltage(adc_value);
                drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TEST] [%s] MEASURE: ADS ADC=%ld voltage=%.2f mV", test_get_list_name(gt_test_list), (long)adc_value, (double)voltage_mv);
                test_info->value[ch][phase] = voltage_mv / HW_ID601_IDD_SENSE_OHM;
                conversion_success = true;
            }
            break;
        }
        case TEST_LIST_IREF1:
        case TEST_LIST_IREF2:
        case TEST_LIST_LDO_DET:
        case TEST_LIST_LDO_DIG:
        case TEST_LIST_LDO_DAC:
        case TEST_LIST_LDO_FLL:
        case TEST_LIST_BGR1:
        case TEST_LIST_BGR2:
        {
            if (true == mcu_adc_wait_conversion_complete())
            {
                const uint32_t adc_value = mcu_adc_get_average_conversion();
                const float voltage_mv = mcu_adc_conversion_adc_to_mvoltage(adc_value);
                drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TEST] [%s] MEASURE: MCU ADC=%lu voltage=%.2f mV", test_get_list_name(gt_test_list), (unsigned long)adc_value, (double)voltage_mv);

                const bool is_ref = ((TEST_LIST_IREF1 == gt_test_list) || (TEST_LIST_IREF2 == gt_test_list)) ? (true) : (false);
                test_info->value[ch][phase] = (true == is_ref) ? (CONST_MA_to_UA(voltage_mv / HW_ID601_IREF_SENSE_OHM)) : (CONST_MV_to_V(voltage_mv));
                conversion_success = true;
            }
            break;
        }
        case TEST_LIST_OSC:
        case TEST_LIST_FLL1:
        case TEST_LIST_FLL2:
        case TEST_LIST_FLL3:
        {
            if (true == mcu_tim_wait_conversion_complete())
            {
                const uint32_t cnt_value = mcu_tim_get_average_conversion();
                const float frequency_hz = mcu_tim_conversion_cnt_to_frequency_Hz(cnt_value);
                drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TEST] [%s] MEASURE: Timer CNT=%lu frequency=%.2f Hz", test_get_list_name(gt_test_list), (unsigned long)cnt_value, (double)frequency_hz);
                test_info->value[ch][phase] = CONST_Hz_to_MHz(frequency_hz * ID601_FREQ_DIV_INTERNAL_RATIO * ID601_FREQ_DIV_HW_RATIO);
                conversion_success = true;
            }
            break;
        }
        case TEST_LIST_IOUT_3P:
        case TEST_LIST_IOUT_SWEEP:
        {
            if (true == ads114s08_wait_conversion_complete())
            {
                int32_t adc_value = ads114s08_get_average_conversion(ADS_DEV_1);
                const float voltage_mv = ads114s08_conversion_adc_to_mvoltage(adc_value);
                drv_uart_printf(LOG_LV_DEBUG, "\r\n    [TEST] [%s] [CH%02u] [PH%u] MEASURE: ADS ADC=%ld voltage=%.2f mV", test_get_list_name(gt_test_list), (ch + 1U), phase, (long)adc_value, (double)voltage_mv);
                switch (gn_id601_test_iout_gain)
                {
                    case ID601_IOUT_GAIN_LOW:
                    {
                        test_info->value[ch][phase] = HW_ID601_IOUT_MV_TO_MA_LOW(voltage_mv);
                        break;
                    }
                    case ID601_IOUT_GAIN_MID:
                    {
                        test_info->value[ch][phase] = HW_ID601_IOUT_MV_TO_MA_MID(voltage_mv);
                        break;
                    }
                    case ID601_IOUT_GAIN_HIGH:
                    {
                        test_info->value[ch][phase] = HW_ID601_IOUT_MV_TO_MA_HIGH(voltage_mv);
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
        case TEST_LIST_IOUT_VREF_SWEEP:
        {
            USER_ASSERT(ch < 12U);
            USER_ASSERT(phase <= VREF_MAX);

            if (true == ads114s08_wait_conversion_complete())
            {
                const int32_t adc_value = ads114s08_get_average_conversion(ADS_DEV_1);
                const float voltage_mv = ads114s08_conversion_adc_to_mvoltage(adc_value);

                float iout_ma = 0.0f;

                switch (gn_id601_test_iout_gain)
                {
                    case ID601_IOUT_GAIN_LOW:
                        iout_ma = HW_ID601_IOUT_MV_TO_MA_LOW(voltage_mv);
                        break;

                    case ID601_IOUT_GAIN_MID:
                        iout_ma = HW_ID601_IOUT_MV_TO_MA_MID(voltage_mv);
                        break;

                    case ID601_IOUT_GAIN_HIGH:
                        iout_ma = HW_ID601_IOUT_MV_TO_MA_HIGH(voltage_mv);
                        break;

                    default:
                        USER_ASSERT(false);
                }

                gf_vref_sweep_row[ch] = iout_ma;
                conversion_success = true;
            }

            break;
        }
        default:
        {
            USER_ASSERT(false);
            break;
        }
    }

    if (false == conversion_success)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TEST] [%s] %s: Conversion failed. Testing stopped", test_get_list_name(gt_test_list), __func__);
        return test_error_handler(td);
    }
    else
    {
        if (gt_test_list < TEST_LIST_IOUT_3P)
        {
            ++gt_test_list;
            td->step = TEST_STEP_SETUP_BY_LIST;
        }
        else if (TEST_LIST_IOUT_3P == gt_test_list)
        {
            if (0U == phase) // p1
            {
                td->step = TEST_STEP_SETUP_BY_LIST;
                test_info->test_phase = 1U;
            }
            else if (1U == phase) // p2
            {
                td->step = TEST_STEP_SETUP_BY_LIST;
                test_info->test_phase = 2U;
            }
            else if (2U == phase) // p3
            {
                td->step = TEST_STEP_SETUP_BY_LIST;
                test_info->test_phase = 0U;
                ++test_info->ch;
                if (test_info->ch >= 12U)
                {
                    test_info->ch = 0U;
                    gt_test_list = TEST_LIST_IOUT_SWEEP;
                }
            }
            else
            {
                USER_ASSERT(false);
            }
        }
        else if (TEST_LIST_IOUT_SWEEP == gt_test_list)
        {
            if (phase < 0x0F)
            {
                ++test_info->test_phase;
                td->step = TEST_STEP_SETUP_BY_LIST;
            }
            else
            {
                test_info->test_phase = 0U;
                ++test_info->ch;
                if (test_info->ch >= 12U)
                {
                    test_info->ch = 0U;
                    td->step = TEST_STEP_LOG_SUMMARY;
                }
                else
                {
                    td->step = TEST_STEP_SETUP_BY_LIST;
                }
            }
        }
        else if (TEST_LIST_IOUT_VREF_SWEEP == gt_test_list)
        {
            ++test_info->ch;

            if (test_info->ch < 12U)
            {
                td->step = TEST_STEP_SETUP_BY_LIST;
            }
            else
            {
                char row[256];

                int n = snprintf(row, sizeof(row), "\r\n        %4u", (unsigned)phase);
                if ((n < 0) || ((size_t)n >= sizeof(row)))
                {
                    return test_error_handler(td);
                }
                size_t used = (size_t)n;
                for (uint16_t index = 0U; index < 12U; ++index)
                {
                    n = snprintf(row + used, sizeof(row) - used, " %10.4f", (double)gf_vref_sweep_row[index]);
                    if ((n < 0) || ((size_t)n >= (sizeof(row) - used)))
                    {
                        return test_error_handler(td);
                    }
                    used += (size_t)n;
                }
                n = snprintf(row + used, sizeof(row) - used, " %4s", "mA");
                if ((n < 0) || ((size_t)n >= (sizeof(row) - used)))
                {
                    return test_error_handler(td);
                }
                drv_uart_printf(LOG_LV_INFO, "%s", row);

                if (phase >= VREF_MAX)
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [TEST] VREF SWEEP COMPLETE");
                    td->step = TEST_STEP_PWR_OFF;
                }
                else
                {
                    const uint32_t next_vref = (uint32_t)phase + (uint32_t)VREF_INC;
                    test_info->test_phase = (uint16_t)((next_vref > VREF_MAX) ? VREF_MAX : next_vref);
                    td->step = TEST_STEP_SETUP_BY_LIST;
                }
                test_info->ch = 0U;
            }
        }
        td->tout = T_DEFAULT_MS;
    }
    return true;
}

static bool test_step_log_summary(thread_data_t *td)
{
    static uint16_t list;
    static uint16_t phase;
    static bool started = false;

    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    const uint32_t row_delay_ticks = (uint32_t)(((uint64_t)30U * osKernelGetTickFreq() + 999U) / 1000U);

    char line[256];
    int n;

    if (false == started)
    {
        list = TEST_LIST_IDDS1;
        phase = 0U;
        started = true;

        drv_uart_printf(LOG_LV_INFO, "\r\n    [TEST] [LOG]\r\n        %-12s %6s %12s", "ITEM", "UNIT", "VALUE");

        td->step = TEST_STEP_LOG_SUMMARY;
        td->tout = row_delay_ticks;
        return true;
    }

    const bool is_iout = ((TEST_LIST_IOUT_3P == list) || (TEST_LIST_IOUT_SWEEP == list));
    if (is_iout)
    {
        n = snprintf(line, sizeof(line), "\r\n        %-12s %6s", test_get_list_name((test_list_t)list), "mA");
        if ((n < 0) || ((size_t)n >= sizeof(line)))
        {
            started = false;
            drv_uart_printf(LOG_LV_ERROR, "\r\n    [TEST] Summary formatting failed");
            return test_error_handler(td);
        }

        size_t used = (size_t)n;

        for (uint8_t ch = 0U; ch < 12U; ++ch)
        {
            n = snprintf(line + used, sizeof(line) - used, " %12.4f", (double)gt_test_info[list].value[ch][phase]);
            if ((n < 0) || ((size_t)n >= (sizeof(line) - used)))
            {
                started = false;
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [TEST] Summary formatting failed");
                return test_error_handler(td);
            }
            used += (size_t)n;
        }

        drv_uart_printf(LOG_LV_INFO, "%s", line);

        const uint8_t max_phase = (TEST_LIST_IOUT_3P == list) ? 3U : 16U;
        ++phase;
        if (phase >= max_phase)
        {
            phase = 0U;
            ++list;
        }
    }
    else
    {
        const char *unit;
        switch (list)
        {
            case TEST_LIST_IDDS1:
            case TEST_LIST_IDDD1:
            {
                unit = "mA";
                break;
            }
            case TEST_LIST_IREF1:
            case TEST_LIST_IREF2:
            {
                unit = "uA";
                break;
            }
            case TEST_LIST_OSC:
            case TEST_LIST_FLL1:
            case TEST_LIST_FLL2:
            case TEST_LIST_FLL3:
            {
                unit = "MHz";
                break;
            }
            default:
            {
                unit = "V";
                break;
            }
        }

        drv_uart_printf(LOG_LV_INFO, "\r\n        %-12s %6s %12.4f", test_get_list_name((test_list_t)list), unit, (double)gt_test_info[list].value[0][0]);
        ++list;
    }

    if (list >= TEST_LIST_IOUT_VREF_SWEEP)
    {
        started = false;

        drv_uart_printf(LOG_LV_INFO, "\r\n    [TEST] MEASUREMENT COMPLETE");

        if (true == gb_id601_test_vref_sweep)
        {
            gt_test_list = TEST_LIST_IOUT_VREF_SWEEP;
            gt_test_info[gt_test_list].ch = 0U;
            gt_test_info[gt_test_list].test_phase = 0U;

            gn_id601_test_iout_gain = TEST_IOUT_VREF_SWEEP_GAIN;
            drv_gpio_id601_iout_gain(gn_id601_test_iout_gain);

            memset(gf_vref_sweep_row, 0, sizeof(gf_vref_sweep_row));

            drv_uart_printf(LOG_LV_INFO, "\r\n    [TEST] VREF SWEEP");
            drv_uart_printf(LOG_LV_INFO, "\r\n        %4s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %4s",
                "VREF", "CH01", "CH02", "CH03", "CH04", "CH05", "CH06", "CH07", "CH08", "CH09", "CH10", "CH11", "CH12", "UNIT");
            td->step = TEST_STEP_SETUP_BY_LIST;
        }
        else
        {
            td->step = TEST_STEP_PWR_OFF;
        }
        td->tout = T_DEFAULT_MS;
    }
    else
    {
        td->step = TEST_STEP_LOG_SUMMARY;
        td->tout = row_delay_ticks;
    }

    return true;
}

static bool test_step_pwr_off(thread_data_t *td)
{
    USER_ASSERT(NULL != td);
    USER_ASSERT(NULL != td->func);
    USER_ASSERT(0U != td->tout);

    drv_gpio_clear_id601();

    if (true == drv_id601_get_hvs_flag())
    {
        drv_id601_set_hvs_flag(false);
    }

    drv_uart_printf(LOG_LV_INFO, "\r\n    [TEST] Power Off");

    td->step = TEST_STEP_NONE;
    td->tout = T_DEFAULT_MS;
    return false; // Power off step always returns false to indicate termination of the test process
}

void test_set_vref_sweep_enable(bool enable)
{
    gb_id601_test_vref_sweep = enable;
}

void id601_test_clear_parameters(thread_data_t *td)
{
    USER_ASSERT(NULL != td);

    gb_id601_test_vref_sweep = false;

    drv_gpio_clear_id601();

    gt_test_list = TEST_LIST_NONE;

    memset(gt_test_info, 0, sizeof(gt_test_info));

    td->step = TEST_STEP_NONE;
    td->func = test_step_none;
    td->tout = 10U;
}

void id601_test_start(thread_data_t *td)
{
    USER_ASSERT(NULL != td);

    const bool enable_vref_sweep = gb_id601_test_vref_sweep;

    id601_test_clear_parameters(td);

    gb_id601_test_vref_sweep = enable_vref_sweep;

    gt_test_list = TEST_LIST_IDDS1;

    td->step = TEST_STEP_INITIAL;
    td->tout = 10U;
    td->func = test_step_initial;
}

bool id601_test_process(thread_data_t *td)
{
    USER_ASSERT(NULL != td);

    const uint32_t step = td->step;
    if ((TEST_STEP_MAX <= step) || (NULL == gp_test_step_handlers[step]))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n        [TEST] [%s] %s: Invalid test step=%u", test_get_list_name(gt_test_list), __func__, (unsigned)step);
        return false;
    }

    return gp_test_step_handlers[step](td);
}

void id601_test_iout_monitor_start(uint16_t channel, uint16_t max_curr_lvl, uint16_t vref)
{
    if (true == gn_id601_iout_monitor_channel)
    {
        id601_test_iout_monitor_stop();
    }

    drv_gpio_id601_ch_demux(channel);
    if (false == id601_set_current_chx(channel))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [TEST] %s: ID601 Trim Setup Current Channel Failed", __func__);
        id601_test_iout_monitor_stop();
        return;
    }
    ads114s08_set_input_mux(ADS_DEV_1, ADS_DEV1_AIN00_ID601_IOUT, ADS_INPUT_N_GND);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    const uint16_t gain_max_current_level3 = max_curr_lvl;
    const uint16_t gain_max_current_level2 = max_curr_lvl;
    const uint16_t gain_max_current_level1 = max_curr_lvl;
    id601_write_max_current_level(1U, gain_max_current_level3, gain_max_current_level2, gain_max_current_level1);

    const uint16_t max_curr_vref = vref;
    id601_write_max_curr_vref1(1U, max_curr_vref);
    id601_write_max_curr_vref2(1U, max_curr_vref);
    id601_write_max_curr_vref3(1U, max_curr_vref);

    gb_id601_iout_monitor_enable = true;
    gn_id601_iout_monitor_channel = channel;
    gn_id601_iout_monitor_tick = osKernelGetTickCount();

    gn_id601_test_iout_gain = ID601_IOUT_GAIN_HIGH;
    drv_gpio_id601_iout_gain(gn_id601_test_iout_gain);

    drv_gpio_id601_vled(ID601_VLED_ON);

    drv_uart_printf(LOG_LV_INFO, "\r\n    [TEST]: ID601 Iout One Channel Started. channel=%u, max_curr_lvl=%u, vref=%u", (unsigned)(channel + 1U), (unsigned)max_curr_lvl, (unsigned)vref);
}

void id601_test_iout_monitor_stop(void)
{
    drv_gpio_clear_id601();
    ads114s08_set_conversion_enable(ADS_DEV_1, false);

    gb_id601_iout_monitor_enable = false;
    gn_id601_iout_monitor_channel = 0U;
    gn_id601_iout_monitor_tick = 0U;

    drv_uart_printf(LOG_LV_INFO, "\r\n    [TEST]: ID601 Iout One Channel Stopped.");
}

void id601_test_iout_monitor_process(void)
{
    if (false == gb_id601_iout_monitor_enable)
    {
        return;
    }

    const uint32_t now = osKernelGetTickCount();
    const uint32_t period = osKernelGetTickFreq(); /* 1초 */

    if ((uint32_t)(now - gn_id601_iout_monitor_tick) < period)
    {
        return;
    }

    /* 측정 시작 간격이 약 1초가 되도록 설정 */
    gn_id601_iout_monitor_tick = now;

    ads114s08_set_conversion_enable(ADS_DEV_1, true);

    if (false == ads114s08_wait_conversion_complete())
    {
        /* 오류 시 기존 stop 함수로 출력까지 정리 */
        id601_test_iout_monitor_stop();
        return;
    }

    const int32_t adc_value = ads114s08_get_average_conversion(ADS_DEV_1);

    const float voltage_mv = ads114s08_conversion_adc_to_mvoltage(adc_value);

    /*
     * 현재 start 함수에서 ID601_IOUT_GAIN_HIGH를 선택하므로
     * HIGH 범위의 변환 매크로 사용
     */
    float iout_ma = 0.0f;

    switch (gn_id601_test_iout_gain)
    {
        case ID601_IOUT_GAIN_LOW:
        {
            iout_ma = HW_ID601_IOUT_MV_TO_MA_LOW(voltage_mv);
            break;
        }
        case ID601_IOUT_GAIN_MID:
        {
            iout_ma = HW_ID601_IOUT_MV_TO_MA_MID(voltage_mv);
            break;
        }
        case ID601_IOUT_GAIN_HIGH:
        {
            iout_ma = HW_ID601_IOUT_MV_TO_MA_HIGH(voltage_mv);
            break;
        }
        default:
        {
            USER_ASSERT(false);
        }
    }

    drv_uart_printf(LOG_LV_INFO, "\r\n    [IOUT] CH=%u, ADC=%ld, V=%.3f mV, Iout=%.6f mA", (unsigned)(gn_id601_iout_monitor_channel + 1U), (long)adc_value, (double)voltage_mv, (double)iout_ma);
}
/* USER CODE END 0 */
