/** @file uart.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __UART_C__

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "uart.h"
#include "config.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PRINT_BUFF_SIZE     (128)
#define UART_PACKET_SIZE    (128)
#define UART_BUFF_SIZE      (128)
#define UART_BACKSPACE      (0x08)
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
    uint16_t length;
    char buffer[UART_BUFF_SIZE];
} uart_packet_t;

typedef struct
{
    uint16_t InCnt;
    uint16_t OutCnt;
    uart_packet_t buffer[UART_PACKET_SIZE];
} uart_ring_buffer_t;
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uart_ring_buffer_t gt_uart_rx;
static uart_ring_buffer_t gt_uart_tx;
static LOG_LV_T gt_log_lv = LOG_DEBUG;
bool gb_uart_tx_busy;
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
__STATIC_INLINE void UART_PutChar(uint8_t data)
{
    /* Echo received character on TX */
    if ((data == '\n') || (data == '\r'))
    {
        LL_USART_TransmitData8(USART2, '\r');
        LL_USART_TransmitData8(USART2, '\n');
    }
    else
    {
        LL_USART_TransmitData8(USART2, data);
    }

    /* Loop until the end of transmission */
    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
}

void Print(LOG_LV_T log_lv, const char *fmt, ...)
{
    if (log_lv >= gt_log_lv)
    {
        int len = 0;
        char msg_buffer[PRINT_BUFF_SIZE] = {0, };
        char temp_buffer[PRINT_BUFF_SIZE] = {0, };

        if (log_lv > LOG_INFO)
        {
            snprintf(temp_buffer, (PRINT_BUFF_SIZE - 1), "%s%s%s", ANSI_FONT_RED, fmt, ANSI_FONT_NONE);
            fmt = temp_buffer;
        }

        va_list ap;
        va_start(ap, fmt);
        len = vsnprintf(msg_buffer, (PRINT_BUFF_SIZE - 1), fmt, ap);
        va_end(ap);

        gt_uart_tx.buffer[gt_uart_tx.InCnt].length = len;
        memcpy(gt_uart_tx.buffer[gt_uart_tx.InCnt].buffer, msg_buffer, len);
        ++gt_uart_tx.InCnt;
        gt_uart_tx.InCnt &= (uint16_t)(UART_PACKET_SIZE - 1);
    }
}

void Comm_Rx_Handler(uint8_t rx)
{
    UART_PutChar(rx);

    if ((rx == '\n') || (rx == '\r'))
    {
        if (gt_uart_rx.buffer[gt_uart_rx.InCnt].length < (UART_BUFF_SIZE - 1))
        {
            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[gt_uart_rx.buffer[gt_uart_rx.InCnt].length] = 0;
        }
        else
        {
            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[(UART_BUFF_SIZE - 1)] = 0;
        }

        ++gt_uart_rx.InCnt;
        gt_uart_rx.InCnt &= (uint16_t)(UART_PACKET_SIZE - 1);
    }
    else if (rx == UART_BACKSPACE)
    {
        if (gt_uart_rx.buffer[gt_uart_rx.InCnt].length)
        {
            const uint8_t temp[2] = {' ', UART_BACKSPACE};
            UART_PutChar(temp[0]);
            UART_PutChar(temp[1]);
            --gt_uart_rx.buffer[gt_uart_rx.InCnt].length;
        }
    }
    else
    {
        if (gt_uart_rx.buffer[gt_uart_rx.InCnt].length < (UART_BUFF_SIZE - 1))
        {
            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[gt_uart_rx.buffer[gt_uart_rx.InCnt].length] = rx;
            ++gt_uart_rx.buffer[gt_uart_rx.InCnt].length;

            if ((gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[0] == 0x1B) && (gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[1] == 0x5B) && (gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[2] == 0x41))
            {
                Print(LOG_INFO, "\r\n\r\n");
                /* copy */
                for (uint8_t i = 0 ; i < 64 ; ++i)
                {
                    if (gt_uart_rx.InCnt)
                    {
                        if (gt_uart_rx.buffer[gt_uart_rx.InCnt - 1].buffer[i])
                        {
                            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[i] = gt_uart_rx.buffer[gt_uart_rx.InCnt - 1].buffer[i];
                            Print(LOG_INFO, "%c", gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[i]);
                        }
                        else
                        {
                            gt_uart_rx.buffer[gt_uart_rx.InCnt].length = i;
                            break;
                        }
                    }
                    else
                    {
                        if (gt_uart_rx.buffer[UART_PACKET_SIZE - 1].buffer[i])
                        {
                            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[i] = gt_uart_rx.buffer[UART_PACKET_SIZE - 1].buffer[i];
                            Print(LOG_INFO, "%c", gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[i]);
                        }
                        else
                        {
                            gt_uart_rx.buffer[gt_uart_rx.InCnt].length = i;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            //Print(LOG_ERROR, "buffer limit...\r\n");
        }
    }
}

static bool Comm_Get_Tx_Packet(uart_packet_t** pData)
{
    bool ret = false;

    if (gt_uart_tx.InCnt != gt_uart_tx.OutCnt)
    {
        *pData = gt_uart_tx.buffer + gt_uart_tx.OutCnt;
        ret = true;
    }

    return ret;
}

static bool Comm_Get_Rx_Packet(uart_packet_t** pData)
{
    bool ret = false;

    if (gt_uart_rx.InCnt != gt_uart_rx.OutCnt)
    {
        *pData = gt_uart_rx.buffer + gt_uart_rx.OutCnt;
        ret = true;
    }

    return ret;
}

void Comm_Increase_Tx_OutCnt(void)
{
    ++gt_uart_tx.OutCnt;
    gt_uart_tx.OutCnt &= (uint16_t)(UART_PACKET_SIZE - 1);
}

void Comm_Increase_Rx_OutCnt(void)
{
    ++gt_uart_rx.OutCnt;
    gt_uart_rx.OutCnt &= (uint16_t)(UART_PACKET_SIZE - 1);
}

static void Comm_Print_Help(void)
{
    Print(LOG_INFO, "\n\r------------------ Command Help -----------------------------");
    Print(LOG_INFO, "\n\r  help / ?\t\t : Show command help");
}

void Comm_Init(void)
{
    Print(LOG_INFO, "\n\r--------------------------------------");
    Print(LOG_INFO, "\n\r     [GT-IP805 (ES) JIG]");
    Print(LOG_INFO, "\n\r--------------------------------------");
    Print(LOG_INFO, "\n\r - Author: xxx@glbltech.com");
    Print(LOG_INFO, "\n\r - Date  : %s", __DATE__);
    Print(LOG_INFO, "\n\r - Ver   : %u.%u.%u", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    Print(LOG_INFO, "\n\r--------------------------------------\r\n");
}

void Comm_UART_Task(void)
{
    uart_packet_t* p_data = NULL;

    if (Comm_Get_Rx_Packet(&p_data))
    {
        char str_in[UART_BUFF_SIZE + 1] = {0, };
        uint32_t u32_recv_param[6] = {0, };
        double lf_recv_param[6] = {0, };

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0;

        if (Command_is_("help") || Command_is_("?"))
        {
            Comm_Print_Help();
        }
/* ----------------- command list - jig ----------------- */
/* ----------------- command list - IP805 ----------------- */
/* ----------------- command list - ui ----------------- */
        else if (Command_is_("reset"))
        {
            Print(LOG_INFO, "\r\n system reset \r\n");
            NVIC_SystemReset();
        }
        else
        {
            Print(LOG_INFO, "\r\n What?\n\r");
        }
        Comm_Increase_Rx_OutCnt();
    }

    if (gb_uart_tx_busy == 0)
    {
        if (Comm_Get_Tx_Packet(&p_data))
        {
            LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
            while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_6));

            LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)&USART2->DR);
            LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)p_data->buffer);
            LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, p_data->length);

            LL_DMA_ClearFlag_TC6(DMA1);
            LL_DMA_ClearFlag_TE6(DMA1);
            LL_USART_ClearFlag_TC(USART2);

            LL_USART_EnableDMAReq_TX(USART2);
            LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
            gb_uart_tx_busy = 1;
        }
    }
}
/* USER CODE END */

/*** end of file ***/