/** @file log.h
 * 
 * @brief uart log message for test result
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include "types.h"

#define FOREGROUND_RED_COLOR        "\033[0;31m"
#define FOREGROUND_MAGENTA_COLOR    "\033[0;35m"
#define FOREGROUND_DEFAULT_COLOR    "\033[0;39m"

#define BACKGROUND_GREEN_COLOR      "\033[0;42m"
#define BACKGROUND_RED_COLOR        "\033[0;41m"
#define BACKGROUND_DEFAULT_COLOR    "\033[0;49m"

extern char msg_buffer[256];

void print(const char* str);

void log_send_auto_test_start(uint16_t n_test_count);
void log_send_manual_test_start(void);
void log_send_auto_test_result(uint8_t n_msg_type, uint8_t n_test_mode_result[TEST_MODE_MAX], uint16_t n_mode_adc_reault[TEST_MODE_MAX][AQ06_O_MAX]);
void log_send_manual_test_result(test_mode_t t_test_mode, uint8_t n_msg_type, uint8_t n_test_mode_result[TEST_MODE_MAX], uint16_t n_mode_adc_reault[TEST_MODE_MAX][AQ06_O_MAX]);
void log_send_function_test_min_max(min_max_t t_manual_function_test_min_max[FUNCTION_TEST_REPEAT_CNT][AQ06_O_MAX]);

extern inline double convert_adc_to_voltage(uint8_t channel, uint16_t adc);
#ifdef __cplusplus
}
#endif

#endif /* ~__LOG_H__ */

/*** end of file ***/

