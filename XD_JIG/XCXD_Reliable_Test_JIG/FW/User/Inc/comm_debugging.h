
/**
 * @file comm_debugging.h
 * @author GT
 * @version v0.0.1
 *
 * @copyright Copyright (c) 2022, Global Technologies Inc. All rights reserved.
 */
#ifndef __COMM_DEBUGGING_H__
#define __COMM_DEBUGGING_H__

typedef enum __LOG_LEVEL__
{
    LOG_LV_DEBUG = 0,
    LOG_LV_INFO,
    LOG_LV_WARN,
    LOG_LV_ERROR,
    LOG_LV_FATAL,
    LOG_LV_MAX,
}LOG_LV_t;

void debugging_log_level(LOG_LV_t lv);
void debugging_init(void);
void debugging_process(void);
void debugging_UART_Printf(LOG_LV_t lv, const char *fmt, ...);
void debugging_rx_handler(uint8_t rx_data);

void debugging_tx_done(void);

#endif /* __COMM_DEBUGGING_H__ */

