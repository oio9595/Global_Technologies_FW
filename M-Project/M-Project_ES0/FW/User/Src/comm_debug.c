#include "comm_debug.h"

const char* gs_pattern[PATTERN_MAX] = 
{   
    "PATTTERN_PIXEL_WHITE",
    "PATTTERN_PIXEL_RED",
    "PATTTERN_PIXEL_BLUE",
    "PATTTERN_PIXEL_GREEN",
    "PATTTERN_QUADRANT_1",
    "PATTTERN_QUADRANT_2",
    "PATTTERN_QUADRANT_3",
    "PATTTERN_QUADRANT_4",
};

extern UART_HandleTypeDef huart2;
uint8_t gn_rx_temp;
char gn_tx_buff[TX_BUFF_SIZE];

static RX_UART_t gt_rx_uart;

void print(const char *fmt, ...)
{
    int len = 0;
    char buffer[256] = {0, };
    
    va_list args;    
    va_start(args, fmt);
    len = vsprintf(buffer, fmt, args);
#if 0
    HAL_UART_Transmit(&huart2, (const uint8_t *)buffer, len, 100);
#else
    while (huart2.gState != HAL_UART_STATE_READY){};
    HAL_UART_Transmit_DMA(&huart2, (const uint8_t *)buffer, len);
#endif
    va_end(args);
}

static void uart_echo(char data)
{
#if 0
    HAL_UART_Transmit(&huart2, (const uint8_t *)&data, 1, 100);
#else
    while (huart2.gState != HAL_UART_STATE_READY){};    
    HAL_UART_Transmit_DMA(&huart2, (const uint8_t *)&data, 1);
#endif
}

static void comm_frame_command()
{
    _tlc_gs_memory_t_ (*p_tlc_gs_parsing_bank)[LED_BUFF_CH_NUM] = NULL;
    p_tlc_gs_parsing_bank = tlc_get_parsing_bank_address();
    
    for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
    {
        for (uint8_t rgb = 0 ; rgb < 3 ; ++rgb)
        {
            if (rgb == 0)
            {
                print("ch %2u: R ", ch);
                for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
                {
                    print("%5u ", p_tlc_gs_parsing_bank[line][ch].u.OUT_Rn);
                }
                print("\r\n");
            }
            else if (rgb == 1)
            {
                print("ch %2u: G ", ch);
                for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
                {
                    print("%5u ", p_tlc_gs_parsing_bank[line][ch].u.OUT_Gn);
                }
                print("\r\n");
            }
            else if (rgb == 2)
            {
                print("ch %2u: B ", ch);
                for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
                {
                    print("%5u ", p_tlc_gs_parsing_bank[line][ch].u.OUT_Bn);
                }
                print("\r\n");
            }
        }
    }
}

static void comm_rx_handler(uint8_t rx_data)
{
    uart_echo(rx_data);

    if((rx_data == '\n') || (rx_data == '\r'))
    {
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length] = 0;
        }
        else
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[(RX_PACKET_SIZE - 1)] = 0;
        }
    
        ++gt_rx_uart.RxInCnt;
        gt_rx_uart.RxInCnt &= (uint16_t)(RX_BUFF_SIZE - 1);
    }
    else if(rx_data == CLI_KEY_BACK)
    {
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length)
        {
            uart_echo(' ');
            uart_echo(CLI_KEY_BACK);

            --gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;
        }
    }
    else
    {
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length] = rx_data;
            ++gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;
        }
        else
        {
            //print("buffer limit...\r\n");
        }
    }
}
static uint8_t comm_get_rx_packet(rx_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_rx_uart.RxInCnt != gt_rx_uart.RxOutCnt)
    {
        *pData = gt_rx_uart.Rxbuff + gt_rx_uart.RxOutCnt;

        ++gt_rx_uart.RxOutCnt;
        gt_rx_uart.RxOutCnt &= (uint16_t)(RX_BUFF_SIZE -1);

        ret = 1;
    }

    return ret;
}

static void comm_help_command(void)
{
    print("\r\n    ====         COMMAND        ====           FUNCTION          ====");
    print("\r\n    =================================================================");
    print("\r\n    ====               reset    ====                MCU RESET    ====");
    print("\r\n    ====           step ____    ====         set dimming step    ====");
    print("\r\n    ====           step         ====        show dimming step    ====");
    print("\r\n    ====              p ____    ====       set pattern number    ====");
    print("\r\n    ====              p         ====      show pattern number    ====");
    print("\r\n    =================================================================");
    print("\r\n");
}

void comm_debugging_process(void)
{
    rx_packet_t* p_data = NULL;

    if(comm_get_rx_packet(&p_data))
    {
        char str_in[RX_PACKET_SIZE + 1] = {0, };
        uint32_t param1 = 0;
        uint32_t param2 = 0;
        float f_param1 = 0;

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0;

        if((!(strcmp(str_in, "help"))) || (!(strcmp(str_in, "?"))))
        {
            comm_help_command();
        }
        
        else if(!(strcmp(str_in, "reset")))
        {
            print("\r\n system reset \r\n");
            NVIC_SystemReset();
        }
        
        else if(!(strcmp(str_in, "frame")))
        {
            comm_frame_command();
        }
        
        else if(sscanf(str_in, "step %u", &param1) == 1)
        {
            if (param1 < 65536)
            {
                set_dimming_value((uint16_t)param1);
                print("\r\n step [%u]\r\n", get_dimming_value());
            }
            else
            {
                print("\r\n Invalid step [%u] [0 ~ 65535]\r\n", param1);
            }
        }        
        else if(!(strcmp(str_in, "step")))
        {
            print("\r\n step [%u] \r\n", get_dimming_value());
        }        
        
        else if(sscanf(str_in, "p %u", &param1) == 1)
        {
            if (param1 < PATTERN_MAX)
            {
                print("\r\n pattern [%s]\r\n", gs_pattern[param1]);
                set_pattern_value((gdim_pattern_t)param1);
            }
            else
            {
                print("\r\n invalid p [%u] [0 ~ %u]\r\n", param1, PATTERN_MAX - 1);
            }
        }
        else if(!(strcmp(str_in, "p")))
        {
            print("\r\n pattern [%s] \r\n", gs_pattern[get_pattern_value()]);
        }   
        
        else if(sscanf(str_in, "blk %u", &param1) == 1)
        {
            if (param1 < 677)
            {
                print("\r\n blk [%u]\r\n", param1);
                tlc_set_blk_num(param1);
            }
            else
            {
                print("\r\n invalid blk [%u] [0 ~ %u]\r\n", param1, 677 - 1);
            }
        }
        else if(!(strcmp(str_in, "blk")))
        {
            print("\r\n blk [%u] \r\n", tlc_get_blk_num());
        }
/* Begin Debug Command */
        else if(!(strcmp(str_in, "vsync_start")))
        {
            print("\r\n start vsync timer \r\n");
            tim6_IT_start();
        }
        else if(!(strcmp(str_in, "vsync_stop")))
        {
            print("\r\n stop vsync timer \r\n");
            tim6_IT_stop();
        }
        
        else if(!(strcmp(str_in, "sub_period_start")))
        {
            print("\r\n start sub_period timer \r\n");
            tim7_IT_start();
        }
        else if(!(strcmp(str_in, "sub_period_stop")))
        {
            print("\r\n stop sub_period timer \r\n");
            tim7_IT_stop();
        }
        
        else if(!(strcmp(str_in, "f_scan_start")))
        {
            print("\r\n start f_scan timer \r\n");
            tim9_IT_start();
        }
        else if(!(strcmp(str_in, "f_scan_stop")))
        {
            print("\r\n stop f_scan timer \r\n");
            tim9_IT_stop();
        }
        else if(sscanf(str_in, "scan_line %u", &param1) == 1)
        {
            if (param1 < DIMMING_LINE_NUM)
            {
                print("\r\n scan_line [%u]\r\n", param1);
                decode_set_scan_line((uint8_t)param1);
            }
            else
            {
                print("\r\n invalid scan_line [%u] [0 ~ %u]\r\n", param1, DIMMING_LINE_NUM - 1);
            }
        }
/* End Debug Command */
        else
        {
            print("\r\n what? \r\n");
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
    if (huart->Instance == USART2)
    {
        comm_rx_handler(gn_rx_temp);
        HAL_UART_Receive_DMA(&huart2, &gn_rx_temp, 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        /* Re-Enable the DMA transfer for transmit request by setting the DMAT bit
           in the UART CR3 register after uart transmit*/
        //ATOMIC_SET_BIT(huart->Instance->CR3, USART_CR3_DMAT);
    }    
}
