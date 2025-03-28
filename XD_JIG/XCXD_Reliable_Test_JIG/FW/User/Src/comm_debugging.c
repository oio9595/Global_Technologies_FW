
/**
 * @file comm_debugging.c
 * @author GT
 * @version v0.0.1
 *
 * @copyright Copyright (c) 2022, Global Technologies Inc. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "main.h"
#include "xc24.h"
#include "xd12.h"
#include "drv_comm.h"
#include "comm_debugging.h"
#include "reliable_task.h"

#define CLI_KEY_BACK        0x08
#define CLI_KEY_DEL         0x7F
#define CLI_KEY_ENTER       0x0D
#define CLI_KEY_ESC         0x1B
#define CLI_KEY_LEFT        0x44
#define CLI_KEY_RIGHT       0x43
#define CLI_KEY_UP          0x41
#define CLI_KEY_DOWN        0x42
#define CLI_KEY_HOME        0x31
#define CLI_KEY_END         0x34

#define RX_BUFF_SIZE        8
#define TX_BUFF_SIZE        128

#define RX_PACKET_SIZE      32
#define TX_PACKET_SIZE      240

typedef struct
{
    uint8_t length;
    char buffer[RX_PACKET_SIZE];
}
rx_packet_t;

typedef struct
{
    uint8_t length;
    char buffer[TX_PACKET_SIZE];
}
tx_packet_t;

typedef struct
{
    uint8_t RxInCnt;
    uint8_t RxOutCnt;
    uint8_t TxInCnt;
    uint8_t TxOutCnt;
    rx_packet_t Rxbuff[RX_BUFF_SIZE];
    tx_packet_t Txbuff[TX_BUFF_SIZE];
}UART_t;
static LOG_LV_t gt_log_level;

static UART_t gt_uart;

static bool gb_usart_tx_flag;

static const char* const gp_msg_prompt = "\n\rLED> \0";
static const char* const gp_msg_okay = "\n\rOK";
static const char* const gp_msg_what = "\n\rWhat?";

#if 0
static char* str_curr_level[DEV_MAX_CURR_LEVEL_MAX] =
{
    "4mA", "8mA", "12mA", "16mA", "24mA", "32mA", "46mA", "64mA",
};

static char* str_short_level[SHORT_LEVEL_MAX] =
{
    "3V", "4V", "6V", "8V", "13V", "16V", "24V", "36V",
};

static char* str_fb_level[FB_LEVEL_MAX] =
{
    "0.40V", "0.50V", "0.60V", "0.70V", "0.85V", "1.00V", "1.15V", "1.30V",
};
#endif

static uint8_t debugging_get_rx_packet(rx_packet_t** pData);
static uint8_t debugging_get_tx_packet(tx_packet_t** pData);

__STATIC_INLINE void debugging_print_help(void)
{
    debugging_UART_Printf(LOG_LV_INFO, "\n\r- Command---------Description -----");
    debugging_UART_Printf(LOG_LV_INFO, "\n\r  help or ?");
    debugging_UART_Printf(LOG_LV_INFO, "\n\r  info");
    debugging_UART_Printf(LOG_LV_INFO, "\n\r  ldim\t\t : LD DATA");

    debugging_UART_Printf(LOG_LV_INFO, "\n\r  reset\t\t : reset MCU");

    debugging_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------\n\r");
    debugging_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
}

static void debugging_print_startup(void)
{
    debugging_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------");
    debugging_UART_Printf(LOG_LV_INFO, "\n\r    [LGD-XC24(ES1) mini-LED Driver]");
    debugging_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------");
    debugging_UART_Printf(LOG_LV_INFO, "\n\r-Project\t: LGD-XC24 DEMO");
    debugging_UART_Printf(LOG_LV_INFO, "\n\r-Author\t\t: xxx@glbltech.com");

    debugging_UART_Printf(LOG_LV_INFO, "\n\r-Build\t\t: %s %s", __DATE__, __TIME__);

    debugging_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------\n\r");
    debugging_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
}

void debugging_log_level(LOG_LV_t lv)
{
    gt_log_level = lv;
}

void debugging_tx_done(void)
{
    gb_usart_tx_flag = false;
}

void debugging_init(void)
{
    debugging_print_startup();
}

void debugging_process(void)
{
    rx_packet_t* p_data = NULL;

    if(debugging_get_rx_packet(&p_data))
    {
        char str_in[RX_PACKET_SIZE + 1] = {0, };
        //char str_model[256] = {0, };
        uint32_t param1 = 0;
        //uint32_t param2 = 0;
        //uint32_t param3 = 0;
        //int32_t i_param1 = 0;
        //float f_param1 = 0;

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0;

        if((!(strcmp(str_in, "help"))) || (!(strcmp(str_in, "?"))))
        {
            debugging_print_help();
        }
        else if(!(strcmp(str_in, "info")))
        {
            debugging_print_startup();
        }
		else if(!(strcmp(str_in, "reset")))
        {
            debugging_UART_Printf(LOG_LV_INFO, gp_msg_okay);
            NVIC_SystemReset();
        }
        else if(sscanf(str_in, "log_level %u", &param1) == 1)
        {
            if(param1 < LOG_LV_MAX)
            {
                debugging_log_level((LOG_LV_t)param1);
            }
            else
            {
                debugging_UART_Printf(LOG_LV_INFO, "\r\n Out of ana range [%u][0 - %u]", param1, (LOG_LV_MAX - 1));
            }
            debugging_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
		else if(!(strcmp(str_in, "xc_r_all")))
        {
            XC24_Read_Register_All();
            debugging_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xd_r_all")))
        {
            XD12_Read_All_Registers();
            debugging_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "ldim %u", &param1) == 1)
        {
            XD12_set_LD_out(param1);
            debugging_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "ldim")))
        {
            uint16_t u16_ld_data = 0;
            u16_ld_data = XD12_get_LD_out();
            debugging_UART_Printf(LOG_LV_INFO, "LDIM : %u", u16_ld_data);
            debugging_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else
        {
            debugging_UART_Printf(LOG_LV_INFO, gp_msg_what);
            debugging_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
    }
    else
    {
        if(gb_usart_tx_flag == false)
        {
            tx_packet_t* p_tx = NULL;

            if(debugging_get_tx_packet(&p_tx))
            {
                gb_usart_tx_flag = true;
                UART_Tx_DMA_Start(p_tx->buffer, p_tx->length);
            }
        }
    }
}

void debugging_UART_Printf(LOG_LV_t lv, const char *fmt, ...)
{
    if(lv >= gt_log_level)
    {
        int len = 0;
        va_list ap;

        va_start(ap, fmt);
        len = vsnprintf(gt_uart.Txbuff[gt_uart.TxInCnt].buffer, (TX_PACKET_SIZE - 1), fmt, ap);
        va_end(ap);

        //if(len > 0)
        {
            gt_uart.Txbuff[gt_uart.TxInCnt].length = len;

            ++gt_uart.TxInCnt;
            gt_uart.TxInCnt &= (uint8_t)(TX_BUFF_SIZE - 1);
        }
    }
}

void debugging_rx_handler(uint8_t rx_data)
{
    UART_PutChar(rx_data); /* echo back */

    if((rx_data == '\n') || (rx_data == '\r'))
    {
        if(gt_uart.Rxbuff[gt_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[gt_uart.Rxbuff[gt_uart.RxInCnt].length] = 0;
        }
        else
        {
            gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[(RX_PACKET_SIZE - 1)] = 0;
        }

        ++gt_uart.RxInCnt;
        gt_uart.RxInCnt &= (uint8_t)(RX_BUFF_SIZE - 1);
    }
    else if(rx_data == CLI_KEY_BACK)
    {
        if(gt_uart.Rxbuff[gt_uart.RxInCnt].length)
        {
            UART_PutChar(' ');
            UART_PutChar(CLI_KEY_BACK);

            --gt_uart.Rxbuff[gt_uart.RxInCnt].length;
        }
    }
    else
    {
        if(gt_uart.Rxbuff[gt_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[gt_uart.Rxbuff[gt_uart.RxInCnt].length] = rx_data;
            ++gt_uart.Rxbuff[gt_uart.RxInCnt].length;
        }
        else
        {
            //debugging_UART_Printf("buffer limit...\r\n");
        }
    }
}

static uint8_t debugging_get_rx_packet(rx_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_uart.RxInCnt != gt_uart.RxOutCnt)
    {
        *pData = gt_uart.Rxbuff + gt_uart.RxOutCnt;

        ++gt_uart.RxOutCnt;
        gt_uart.RxOutCnt &= (uint8_t)(RX_BUFF_SIZE -1);

        ret = 1;
    }

    return ret;
}

static uint8_t debugging_get_tx_packet(tx_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_uart.TxInCnt != gt_uart.TxOutCnt)
    {
        *pData = gt_uart.Txbuff + gt_uart.TxOutCnt;

        ++gt_uart.TxOutCnt;
        gt_uart.TxOutCnt &= (uint8_t)(TX_BUFF_SIZE - 1);

        ret = 1;
    }

    return ret;
}