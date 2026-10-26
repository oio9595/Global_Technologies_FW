/* USER CODE BEGIN Header */
/*
    * File:   common_def.h
    * Author: GT
    *
    * Created on 2026. 10. 13.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* 2. Project internal / System-related headers */
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define USER_ASSERT(cond) \
    do { \
        if (!(cond)) \
        { \
            char msg[256]; \
            int len = snprintf( \
                msg, sizeof(msg), \
                "\r\n    Assertion failed: %s" \
                "\r\n    Function: %s, Line: %d\r\n" \
                "\r\n    MCU will reset!\r\n", \
                #cond, __func__, __LINE__); \
            if (len > 0) \
            { \
                uint16_t size = (uint16_t)( \
                    ((size_t)len < sizeof(msg)) \
                    ? (size_t)len : (sizeof(msg) - 1U)); \
                drv_uart_printf_direct((const uint8_t*)msg, size); \
            } \
            osDelay(osKernelGetTickFreq() * 10U); \
            NVIC_SystemReset(); \
        } \
    } while (0)

#define TRIM_START_FLAG                 (1U << 0U)
#define TEST_START_FLAG                 (1U << 1U)
#define TEST_VSYNC_TEST_FLAG                 (1U << 2U)

#define CONST_MA_to_UA(x)               ((x) * 1000.0f) // Convert milliampere to microampere
#define CONST_MV_to_V(x)                ((x) / 1000.0f) // Convert millivolts to volts
#define CONST_Hz_to_MHz(x)              ((x) / 1000000.0f) // Convert hertz to megahertz

#define ID601_FREQ_DIV_INTERNAL_RATIO   (64U)
#define ID601_FREQ_DIV_HW_RATIO         (128U)

#define HW_ID601_IREF_SENSE_OHM         (10000U) // 10kΩ

#define HW_ID601_IDD_SENSE_OHM          (22U) // 22Ω

#define HW_ID601_IOUT_SENSE_LOW_OHM     (2000U) // 2.0kΩ, ~  1mA
#define HW_ID601_IOUT_SENSE_MID_OHM     (200U)  // 200Ω,  ~ 10mA
#define HW_ID601_IOUT_SENSE_HIGH_OHM    (25U)   // 25Ω,   ~ 80mA
#define HW_ID601_IOUT_INPUT_OHM         (2200U) // 2.2kΩ
#define HW_ID601_IOUT_OUTPUT_OHM        (3300U) // 3.3kΩ

#define HW_ID601_IOUT_MV_TO_MA_LOW(x)   (((float)(x) * (float)HW_ID601_IOUT_INPUT_OHM) / ((float)HW_ID601_IOUT_OUTPUT_OHM * (float)HW_ID601_IOUT_SENSE_LOW_OHM))
#define HW_ID601_IOUT_MV_TO_MA_MID(x)   (((float)(x) * (float)HW_ID601_IOUT_INPUT_OHM) / ((float)HW_ID601_IOUT_OUTPUT_OHM * (float)HW_ID601_IOUT_SENSE_MID_OHM))
#define HW_ID601_IOUT_MV_TO_MA_HIGH(x)  (((float)(x) * (float)HW_ID601_IOUT_INPUT_OHM) / ((float)HW_ID601_IOUT_OUTPUT_OHM * (float)HW_ID601_IOUT_SENSE_HIGH_OHM))
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct tag_THREAD_DATA thread_data_t;
typedef bool (*thread_step_handler_t)(thread_data_t *td);
struct tag_THREAD_DATA
{
    uint32_t step;
    uint32_t tout;
    thread_step_handler_t func;
};

typedef void (*void_func_ptr_t)(void);
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
static inline float GET_AVERAGE(float x, float y)
{
    return ((x + y) / 2.0f);
}

static inline float GET_DELTA(float x, float y)
{
    if (x >= y)
        return (x - y);
    else
        return (y - x);
}
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_DEF_H__ */
