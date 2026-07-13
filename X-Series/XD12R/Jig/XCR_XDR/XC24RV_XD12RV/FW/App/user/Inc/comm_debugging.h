
/**
 * @file comm_debugging.h
 * @author GT
 * @version v0.0.1
 *
 * @copyright Copyright (c) 2022, Global Technologies Inc. All rights reserved.
 */
#ifndef __COMM_DEBUGGING_H__
#define __COMM_DEBUGGING_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum __LOG_LEVEL__
{
    LOG_LV_DEBUG = 0,
    LOG_LV_INFO,
    LOG_LV_WARN,
    LOG_LV_ERROR,
    LOG_LV_FATAL,
    LOG_LV_MAX,
}LOG_LV_t;

#define ANSI_FONT_NONE              "\033[0m"
#define ANSI_FONT_RED               "\033[31m"
#define ANSI_FONT_GREEN             "\033[32m"
#define ANSI_FONT_YELLOW            "\033[33m"
#define ANSI_FONT_BLUE              "\033[34m"
#define ANSI_FONT_MAGENTA           "\033[35m"
#define ANSI_FONT_CYAN              "\033[36m"

#define FATAL_INVALID_INPUT(val)    comm_UART_Printf(LOG_LV_FATAL, "\r\nFunction[%s] invalid input (%u)", __func__, (val))

extern bool gb_usart_tx_start_flag;

void comm_init(void);
void comm_debugging_process(void);
void comm_UART_Printf(LOG_LV_t lv, const char *fmt, ...);
void comm_rx_handler(uint8_t rx_data);
void comm_tx_handler(void);

#endif /* __COMM_DEBUGGING_H__ */

