#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <main.h>

#include <stdint.h>
#include <string.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "decoder.h"
#include "tlc59581.h"
#include "drv_process.h"
#include "comm_debug.h"

#define EN_USE_GPIO 1

extern char gn_uart_tx[256];

#endif /* end of __CONFIG_H__ */
