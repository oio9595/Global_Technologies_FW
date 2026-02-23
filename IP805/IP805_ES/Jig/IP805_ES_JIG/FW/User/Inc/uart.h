/** @file uart.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum tag_LOG_LV_T
{
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_ERROR,
    LOG_MAX,
} LOG_LV_T;

extern bool gb_uart_tx_busy;

extern void Comm_Rx_Handler(uint8_t rx);
extern void Print(LOG_LV_T log_lv, const char *fmt, ...);
extern void Comm_Increase_Tx_OutCnt(void);
extern void Comm_Init(void);
extern void Comm_UART_Task(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__UART_H__ */

/*** end of file ***/