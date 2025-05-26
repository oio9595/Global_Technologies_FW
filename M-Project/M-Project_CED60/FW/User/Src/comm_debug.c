#ifndef __COMM_DEBUG_C__
#define __COMM_DEBUG_C__

#include "comm_debug.h"
#include "tlc59581.h"

#if 0
const char* gs_pattern[PATTERN_MAX] =
{
    "PATTERN_PIXEL_WHITE",
    "PATTERN_PIXEL_RED",
    "PATTERN_PIXEL_BLUE",
    "PATTERN_PIXEL_GREEN",
    "PATTERN_QUADRANT_1",
    "PATTERN_QUADRANT_2",
    "PATTERN_QUADRANT_3",
    "PATTERN_QUADRANT_4",
};
#endif

static const char* gs_fpga_boot_up_msg = "waiting while FPGA Booting\r\n";

uint8_t gn_rx_temp;

static _uart_ring_buffer_t_ gt_uart_tx;
static _uart_ring_buffer_t_ gt_uart_rx;

void print(const char *fmt, ...)
{
    int len = 0;

    va_list args;
    va_start(args, fmt);
    len = vsprintf((gt_uart_tx.buff[gt_uart_tx.InCnt].buffer), fmt, args);
    gt_uart_tx.buff[gt_uart_tx.InCnt].length = len;

    ++gt_uart_tx.InCnt;
    gt_uart_tx.InCnt &= (uint16_t)(UART_BUFF_SIZE - 1);
    va_end(args);
}

static void uart_echo(char data)
{
    LL_USART_TransmitData8(USART3, data);

    while (RESET == LL_USART_IsActiveFlag_TXE(USART3));
}

static void comm_rx_handler(uint8_t rx_data)
{
    uart_echo(rx_data);

    if((rx_data == '\n') || (rx_data == '\r'))
    {
        uart_echo('\n');
        if(gt_uart_rx.buff[gt_uart_rx.InCnt].length < (UART_PACKET_SIZE - 1))
        {
            gt_uart_rx.buff[gt_uart_rx.InCnt].buffer[gt_uart_rx.buff[gt_uart_rx.InCnt].length] = 0;
        }
        else
        {
            gt_uart_rx.buff[gt_uart_rx.InCnt].buffer[(UART_PACKET_SIZE - 1)] = 0;
        }

        ++gt_uart_rx.InCnt;
        gt_uart_rx.InCnt &= (uint16_t)(UART_BUFF_SIZE - 1);
    }
    else if(rx_data == CLI_KEY_BACK)
    {
        if(gt_uart_rx.buff[gt_uart_rx.InCnt].length)
        {
            uart_echo(' ');
            uart_echo(CLI_KEY_BACK);

            --gt_uart_rx.buff[gt_uart_rx.InCnt].length;
            gt_uart_rx.buff[gt_uart_rx.InCnt].buffer[gt_uart_rx.buff[gt_uart_rx.InCnt].length] = 0;
        }
    }
    else
    {
        if(gt_uart_rx.buff[gt_uart_rx.InCnt].length < (UART_PACKET_SIZE - 1))
        {
            gt_uart_rx.buff[gt_uart_rx.InCnt].buffer[gt_uart_rx.buff[gt_uart_rx.InCnt].length] = rx_data;
            ++gt_uart_rx.buff[gt_uart_rx.InCnt].length;
        }
        else
        {
            //print("buffer limit...\r\n");
        }
    }
}

static uint8_t comm_get_rx_packet(_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_uart_rx.InCnt != gt_uart_rx.OutCnt)
    {
        *pData = gt_uart_rx.buff + gt_uart_rx.OutCnt;

        ++gt_uart_rx.OutCnt;
        gt_uart_rx.OutCnt &= (uint16_t)(UART_BUFF_SIZE - 1);

        ret = 1;
    }

    return ret;
}

static void comm_get_tx_packet(void)
{
    if (gt_uart_tx.InCnt != gt_uart_tx.OutCnt)
    {
        if (!LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_3))
        {
            LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_3);
            LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_3, (uint32_t)gt_uart_tx.buff[gt_uart_tx.OutCnt].buffer);
            LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_3, gt_uart_tx.buff[gt_uart_tx.OutCnt].length);
            LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_3);

            ++gt_uart_tx.OutCnt;
            gt_uart_tx.OutCnt &= (uint16_t)(UART_BUFF_SIZE - 1);
        }
    }
}

void comm_dma_rx_handler(void)
{
    comm_rx_handler(gn_rx_temp);
}

void comm_init(void)
{
    uint8_t index = 0;
    while(*(gs_fpga_boot_up_msg + index) != NULL)
    {
        LL_USART_TransmitData8(USART3, *(gs_fpga_boot_up_msg + index));
        while (RESET == LL_USART_IsActiveFlag_TXE(USART3));
        ++index;
    }

    print("========================\r\n");
    print(" Project: M-Project ES1\r\n");
    print(" Author : jy lee\r\n");
    print(" Build  : %s\r\n", __DATE__);
    print("          %s\r\n", __TIME__);
    print("========================\r\n");
}

static void comm_help_command(void)
{
    print("\r\n    ====         COMMAND        ====           FUNCTION          ====");
    print("\r\n    =================================================================");
    print("\r\n    ====               reset    ====                MCU RESET    ====");
    print("\r\n    ====             gs ____    ====           set gray scale    ====");
    print("\r\n    ====             gs         ====          show gray scale    ====");
    print("\r\n    ====              p ____    ====       set pattern number    ====");
    print("\r\n    ====              p         ====      show pattern number    ====");
    print("\r\n    =================================================================");
    print("\r\n");
}

void comm_debugging_process(void)
{
    _packet_t* p_rx_data = NULL;

    if(comm_get_rx_packet(&p_rx_data))
    {
        char str_in[UART_PACKET_SIZE + 1] = {0, };
        uint32_t u32_param[2] = {0, };
        // float f_param[2] = {0, };

        memcpy(str_in, p_rx_data->buffer, p_rx_data->length);
        p_rx_data->length = 0;

        if((!(strcmp(str_in, "help"))) || (!(strcmp(str_in, "?"))))
        {
            comm_help_command();
        }
        else if(!(strcmp(str_in, "reset")))
        {
            print("\r\n system reset \r\n");
            NVIC_SystemReset();
        }
        else if(sscanf(str_in, "p %u", &u32_param[0]) == 1)
        {
            tlc59581_set_pattern((_tlc59581_pattern_t_)u32_param[0]);
        }
        else if(!(strcmp(str_in, "p")))
        {
            print("\r\n pattern - [%s] \r\n", gs_tlc59581_pattern[tlc59581_get_pattern()]);
        }
        else if(sscanf(str_in, "gs %u", &u32_param[0]) == 1)
        {
            tlc59581_set_gray_scale((uint16_t)u32_param[0]);
        }
        else if(!(strcmp(str_in, "gs")))
        {
            uint16_t gray_scale_value = tlc59581_get_gray_scale();
            print("\r\n gray scale - %u [0x%04X] \r\n", gray_scale_value, gray_scale_value);
        }
        else
        {
            print("what? \r\n");
        }
    }

    comm_get_tx_packet();
}

#endif /* end of __COMM_DEBUG_C__ */