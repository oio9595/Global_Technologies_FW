#ifndef _COMM_DEBUG_H_
#define _COMM_DEBUG_H_

#include "config.h"

typedef enum
{
    LOG_LV_DEBUG = 0,
    LOG_LV_INFO,
    LOG_LV_ERROR,
    LOG_LV_MAX,
}LOG_LV_T;

extern bool gb_uart_tx_started;

EXTERN void print(LOG_LV_T log_lv, const char *fmt, ...);
EXTERN void comm_debugging_process(void);
EXTERN void comm_print_startup(void);
EXTERN void UART_RxCpltCallback(void);

#endif /* end of _COMM_DEBUG_H_ */
