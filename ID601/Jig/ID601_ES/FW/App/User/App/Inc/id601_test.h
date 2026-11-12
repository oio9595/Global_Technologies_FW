/* USER CODE BEGIN Header */
/*
    * File:   id601_test.h
    * Author: GT
    *
    * Created on 2026. 09. 22.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ID601_TEST_H__
#define __ID601_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
/* 2. Project internal / System-related headers */
#include "common_def.h"

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef thread_step_handler_t test_step_handler_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

extern bool drv_id601_set_hvs_flag(bool hvs_flag);
extern bool drv_id601_get_hvs_flag(void);

extern bool id601_test_setup_idds1(void);
extern bool id601_test_setup_iddd1(void);
extern bool id601_test_setup_iref1(void);
extern bool id601_test_setup_iref2(void);
extern bool id601_test_setup_ldo_det(void);
extern bool id601_test_setup_ldo_dig(void);
extern bool id601_test_setup_ldo_dac(void);
extern bool id601_test_setup_ldo_fll(void);
extern bool id601_test_setup_bgr1(void);
extern bool id601_test_setup_bgr2(void);
extern bool id601_test_setup_osc(void);
extern bool id601_test_setup_fll1(void);
extern bool id601_test_setup_fll2(void);
extern bool id601_test_setup_fll3(void);
extern bool id601_test_setup_iout_3p(void);
extern bool id601_test_setup_iout_sweep(void);
extern bool id601_test_setup_iout_vref_sweep(void);

extern bool id601_test_setup_iout_3p_x(uint8_t ph);
extern bool id601_test_setup_iout_sweep_x(uint8_t ph);
extern bool id601_test_setup_iout_vref_sweep_x(uint16_t vref);

extern void test_set_vref_sweep_enable(bool enable);
extern void id601_test_clear_parameters(thread_data_t *td);
extern void id601_test_start(thread_data_t *td);
extern bool id601_test_process(thread_data_t *td);

extern void id601_test_iout_monitor_start(uint16_t channel, uint16_t max_curr_lvl, uint16_t vref);
extern void id601_test_iout_monitor_stop(void);
extern void id601_test_iout_monitor_process(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __ID601_TEST_H__ */
