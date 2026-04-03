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
#include "xdic.h"
#include "config.h"
#include "JigBd_IF.h"
#include "vsync_task.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PRINT_BUFF_SIZE     (128U)
#define UART_PACKET_SIZE    (128U)
#define UART_BUFF_SIZE      (128U)
#define UART_BACKSPACE      (0x08U)

#define VA_GENERIC(_1, _2, _3, _4, _5, _6,x, ...) x
#define Command_Param_is_(a, b, ...) (sscanf(str_in, a b, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_is_(x) (strncmp(str_in, x, strlen(x)) == 0)
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
    if ((data == (uint8_t)'\n') || (data == (uint8_t)'\r'))
    {
        LL_USART_TransmitData8(USART2, (uint8_t)'\r');
        LL_USART_TransmitData8(USART2, (uint8_t)'\n');
    }
    else
    {
        LL_USART_TransmitData8(USART2, data);
    }

    /* Loop until the end of transmission */
    while (RESET == LL_USART_IsActiveFlag_TXE(USART2))
    {
        __NOP();
    }
}

void Print(LOG_LV_T log_lv, const char *fmt, ...)
{
    if (log_lv >= gt_log_lv)
    {
        int len = 0;
        char msg_buffer[PRINT_BUFF_SIZE] = {0};
        char temp_buffer[PRINT_BUFF_SIZE] = {0};

        if (log_lv > LOG_INFO)
        {
            snprintf(temp_buffer, (PRINT_BUFF_SIZE - 1U), "%s%s%s", ANSI_FONT_RED, fmt, ANSI_FONT_NONE);
            fmt = temp_buffer;
        }

        va_list ap;
        va_start(ap, fmt);
        len = vsnprintf(msg_buffer, (PRINT_BUFF_SIZE - 1U), fmt, ap);
        va_end(ap);

        gt_uart_tx.buffer[gt_uart_tx.InCnt].length = len;
        memcpy(gt_uart_tx.buffer[gt_uart_tx.InCnt].buffer, msg_buffer, len);
        ++gt_uart_tx.InCnt;
        gt_uart_tx.InCnt &= (uint16_t)(UART_PACKET_SIZE - 1U);
    }
}

void Comm_Rx_Handler(uint8_t rx)
{
    UART_PutChar(rx);

    if ((rx == (uint8_t)'\n') || (rx == (uint8_t)'\r'))
    {
        if (gt_uart_rx.buffer[gt_uart_rx.InCnt].length < (UART_BUFF_SIZE - 1U))
        {
            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[gt_uart_rx.buffer[gt_uart_rx.InCnt].length] = 0U;
        }
        else
        {
            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[(UART_BUFF_SIZE - 1U)] = 0U;
        }

        ++gt_uart_rx.InCnt;
        gt_uart_rx.InCnt &= (uint16_t)(UART_PACKET_SIZE - 1U);
    }
    else if (rx == UART_BACKSPACE)
    {
        if (gt_uart_rx.buffer[gt_uart_rx.InCnt].length != 0U)
        {
            const uint8_t temp[2] = {(uint8_t)' ', UART_BACKSPACE};
            UART_PutChar(temp[0]);
            UART_PutChar(temp[1]);
            --gt_uart_rx.buffer[gt_uart_rx.InCnt].length;
        }
    }
    else
    {
        if (gt_uart_rx.buffer[gt_uart_rx.InCnt].length < (UART_BUFF_SIZE - 1U))
        {
            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[gt_uart_rx.buffer[gt_uart_rx.InCnt].length] = rx;
            ++gt_uart_rx.buffer[gt_uart_rx.InCnt].length;

            if ((gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[0] == (char)0x1BU) && (gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[1] == (char)0x5BU) && (gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[2] == (char)0x41U))
            {
                Print(LOG_INFO, "\r\n\r\n");
                /* copy */
                for (uint8_t i = 0 ; i < 64U ; ++i)
                {
                    if (gt_uart_rx.InCnt != 0U)
                    {
                        if (gt_uart_rx.buffer[gt_uart_rx.InCnt - 1U].buffer[i] != (char)0U)
                        {
                            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[i] = gt_uart_rx.buffer[gt_uart_rx.InCnt - 1U].buffer[i];
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
                        if (gt_uart_rx.buffer[UART_PACKET_SIZE - 1U].buffer[i] != (char)0U)
                        {
                            gt_uart_rx.buffer[gt_uart_rx.InCnt].buffer[i] = gt_uart_rx.buffer[UART_PACKET_SIZE - 1U].buffer[i];
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
    gt_uart_tx.OutCnt &= (uint16_t)(UART_PACKET_SIZE - 1U);
}

static void Comm_Increase_Rx_OutCnt(void)
{
    ++gt_uart_rx.OutCnt;
    gt_uart_rx.OutCnt &= (uint16_t)(UART_PACKET_SIZE - 1U);
}

static void Comm_Print_Help(void)
{
    Print(LOG_INFO, "\n\r------------------ Command Help -----------------------------");
    Print(LOG_INFO, "\n\r  help / ?\t\t : Show command help");
}

void Comm_Init(void)
{
    Print(LOG_INFO, "\n\r--------------------------------------");
    Print(LOG_INFO, "\n\r     [GT-XD12R BLU (ES) DEMO]");
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
        char str_in[UART_BUFF_SIZE + 1U] = {0};
        uint32_t u32_recv_param[6] = {0};
        double lf_recv_param[6] = {0};

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0U;

        if (Command_is_("help") || Command_is_("?"))
        {
            Comm_Print_Help();
        }
/* ----------------- command list - LED ----------------- */
        else if (Command_Param_is_("led_color", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] < LED_COLOR_MAX)
            {
                LED_Select_Color(u32_recv_param[0]);
                Print(LOG_INFO, "\r\n OK \r\n");
            }
            else
            {
                Print(LOG_INFO, "\r\n Invalid Input. Use: led_color (0 ~ 3)]\r\n");
            }
        }
        else if (Command_is_("led_brightness_up"))
        {
            LED_Select_Brightness_Up();
            Print(LOG_INFO, "\r\n OK \r\n");
        }
        else if (Command_is_("led_brightness_down"))
        {
            LED_Select_Brightness_Down();
            Print(LOG_INFO, "\r\n OK \r\n");
        }
        else if (Command_Param_is_("led_brightness", "%u %u %u", &u32_recv_param[0], &u32_recv_param[1], &u32_recv_param[2]))
        {
            if (u32_recv_param[0] <= 100U)
            {
                LED_Select_Brightness(u32_recv_param[0], u32_recv_param[1], u32_recv_param[2]);
                Print(LOG_INFO, "\r\n OK \r\n");
            }
            else
            {
                Print(LOG_INFO, "\r\n Invalid Input. Use: led_brightness (0 ~ 100)\r\n");
            }
        }
        else if (Command_Param_is_("led_brightness", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= 100U)
            {
                LED_Select_Brightness(u32_recv_param[0], u32_recv_param[0], u32_recv_param[0]);
                Print(LOG_INFO, "\r\n OK \r\n");
            }
            else
            {
                Print(LOG_INFO, "\r\n Invalid Input. Use: led_brightness (0 ~ 100)\r\n");
            }
        }
        else if (Command_Param_is_("led_pattern", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] < LED_PATTERN_MAX)
            {
                LED_Select_Pattern(u32_recv_param[0]);
                Print(LOG_INFO, "\r\n OK \r\n");
            }
            else
            {
                Print(LOG_INFO, "\r\n Invalid Input. Use: led_pattern (0 ~ 4)\r\n");
            }
        }
        else if (Command_Param_is_("led_pixel", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] < LED_MAP_SIZE)
            {
                LED_Select_Pixel(u32_recv_param[0]);
                Print(LOG_INFO, "\r\n Pixel OK \r\n");
            }
            else
            {
                Print(LOG_INFO, "\r\n Invalid Input. Use: led_pixel (0 ~ 143)]\r\n");
            }
        }
        else if (Command_Param_is_("vref", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= 4095U)
            {
                XDIC_Set_Write_Target_Reg(XDIC_ADDR_MAX_CURRENT_VREF1, u32_recv_param[0]);
                Print(LOG_INFO, "\r\n OK R -> %u \r\n", u32_recv_param[0]);
            }
            else
            {
                Print(LOG_INFO, "\r\n Invalid Input. Use: vref_r (0 ~ 4095)]\r\n");
            }
        }
        else if (Command_Param_is_("max_curr", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= 15)
            {
                uint16_t max_curr = ((u32_recv_param[0] << 8) | (u32_recv_param[0] << 4) | (u32_recv_param[0] << 0));
                XDIC_Set_Write_Target_Reg(XDIC_ADDR_MAX_CURRENT_LEVEL, max_curr);
                Print(LOG_INFO, "\r\n OK max_curr -> %u \r\n", u32_recv_param[0]);
            }
            else
            {
                Print(LOG_INFO, "\r\n Invalid Input. Use: max_curr (0 ~ 15)]\r\n");
            }
        }
/* ----------------- command list - MCU Function ----------------- */
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

    if (gb_uart_tx_busy == false)
    {
        if (Comm_Get_Tx_Packet(&p_data))
        {
            LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
            while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_6))
            {
                __NOP();
            }

            LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)&USART2->DR);
            LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)p_data->buffer);
            LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, p_data->length);

            LL_DMA_ClearFlag_TC6(DMA1);
            LL_DMA_ClearFlag_TE6(DMA1);
            LL_USART_ClearFlag_TC(USART2);

            LL_USART_EnableDMAReq_TX(USART2);
            LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
            gb_uart_tx_busy = true;
        }
    }
}
/* USER CODE END */

/*** end of file ***/