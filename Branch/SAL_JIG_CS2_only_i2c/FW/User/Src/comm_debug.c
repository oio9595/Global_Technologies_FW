
#include "comm_debug.h"
#include "drv_sal.h"
#include "drv_sal_trim.h"
#include "switch.h"
#include "ADS124S08.h"
#include <main.h>

#define RX_BUFF_SIZE        8
#define TX_BUFF_SIZE        512

#define RX_PACKET_SIZE      32
#define TX_PACKET_SIZE      150

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

typedef struct
{
    uint8_t length;
    char buffer[RX_PACKET_SIZE];
}rx_packet_t;

typedef struct
{
    uint8_t length;
    char buffer[TX_PACKET_SIZE];
}tx_packet_t;


typedef struct
{
    uint8_t RxInCnt;
    uint8_t RxOutCnt;
    rx_packet_t Rxbuff[RX_BUFF_SIZE];
    uint8_t TxInCnt;
    uint8_t TxOutCnt;
    tx_packet_t Txbuff[TX_BUFF_SIZE];
}RX_UART_t;

static RX_UART_t gt_rx_uart;
static rx_packet_t gt_last_input;
static LOG_LV_T gt_log_lv = LOG_LV_INFO;

bool gb_uart_tx_started;

static const char* const gp_msg_prompt = "\n\rID804> \0";
static const char* const gp_msg_okay = "\n\rOK";
static const char* const gp_msg_what = "\n\rWhat?";

static const char* gs_uart_log_lv_str[LOG_LV_MAX] =
{
    "LOG_LV_DEBUG",
    "LOG_LV_INFO",
    "LOG_LV_ERROR",
};

void print(LOG_LV_T log_lv, const char *fmt, ...)
{
    if (log_lv >= gt_log_lv)
    {
        int len = 0;

        va_list args;
        va_start(args, fmt);
        len = vsnprintf(gt_rx_uart.Txbuff[gt_rx_uart.TxInCnt].buffer, (TX_PACKET_SIZE - 1), fmt, args);
        va_end(args);

        gt_rx_uart.Txbuff[gt_rx_uart.TxInCnt].length = len;

        ++gt_rx_uart.TxInCnt;
        gt_rx_uart.TxInCnt &= (uint8_t)(TX_BUFF_SIZE - 1);
    }
}

static void uart_echo(char data)
{
    /* Echo received character on TX */
    LL_USART_TransmitData8(USART2, (uint8_t)data);

    /* Loop until the end of transmission */
    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
}

static void comm_rx_handler(uint8_t rx_data)
{
    uart_echo(rx_data);

    switch(rx_data)
    {
    case '\n':
    case '\r':
        //uart_echo('\n');
        uart_echo('\r');
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length] = 0;
        }
        else
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[(RX_PACKET_SIZE - 1)] = 0;
        }

        ++gt_rx_uart.RxInCnt;
        gt_rx_uart.RxInCnt &= (uint8_t)(RX_BUFF_SIZE - 1);
        break;
    case CLI_KEY_BACK:
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length)
        {
            uart_echo(' ');
            uart_echo(CLI_KEY_BACK);

            --gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;
        }
        break;
    case CLI_KEY_UP:
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[0] == 0x1B && gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[1] == 0x5B)
        {
            uart_echo('\n');
            uart_echo('\r');

            if(gt_last_input.length != 0)
            {
                gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length = gt_last_input.length;
                memcpy(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer, gt_last_input.buffer, gt_last_input.length);

                for(uint8_t i=0 ; i<gt_last_input.length ; ++i)
                {
                    uart_echo(gt_last_input.buffer[i]);
                }
            }
            else
            {
                gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length = 0;
            }
        }
        break;
    default:
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length] = rx_data;
            ++gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;
        }
        break;
    }

}
static uint8_t comm_get_rx_packet(rx_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_rx_uart.RxInCnt != gt_rx_uart.RxOutCnt)
    {
        *pData = gt_rx_uart.Rxbuff + gt_rx_uart.RxOutCnt;

        ++gt_rx_uart.RxOutCnt;
        gt_rx_uart.RxOutCnt &= (uint8_t)(RX_BUFF_SIZE -1);

        ret = 1;
    }

    return ret;
}

static uint8_t comm_get_tx_packet(tx_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_rx_uart.TxInCnt != gt_rx_uart.TxOutCnt)
    {
        *pData = gt_rx_uart.Txbuff + gt_rx_uart.TxOutCnt;

        ++gt_rx_uart.TxOutCnt;
        gt_rx_uart.TxOutCnt &= (uint8_t)(TX_BUFF_SIZE -1);

        ret = 1;
    }

    return ret;
}

static void comm_help_command(void)
{
    print(LOG_LV_INFO, "\r\n    ====         COMMAND           ====         FUNCTION        ====");
    print(LOG_LV_INFO, "\r\n    ================================================================");
    print(LOG_LV_INFO, "\r\n    ====       804_i2c_init        ====   initial for I2C       ====");
    print(LOG_LV_INFO, "\r\n    ====       804_read_all        ====   read all registers    ====");
    print(LOG_LV_INFO, "\r\n    ====       804_w [hex] [hex]   ====   addr, value           ====");
    print(LOG_LV_INFO, "\r\n    ====       804_r [hex]         ====   addr                  ====");
    print(LOG_LV_INFO, "\r\n    ================================================================");
    print(LOG_LV_INFO, "\r\n");

    print(LOG_LV_INFO, gp_msg_prompt);
}

void comm_print_startup(void)
{
    print(LOG_LV_INFO, "\n\r--------------------------------------");
    print(LOG_LV_INFO, "\n\r    [GT-ID804(CS) TEST/TRIM JIG]");
    print(LOG_LV_INFO, "\n\r--------------------------------------");
    print(LOG_LV_INFO, "\n\r-Project\t: STM32 ID804 JIG");
    print(LOG_LV_INFO, "\n\r-Author\t\t: xxx@glbltech.com");
    print(LOG_LV_INFO, "\n\r-Build\t\t: %s", __DATE__);
    print(LOG_LV_INFO, "\n\r--------------------------------------");
    print(LOG_LV_INFO, gp_msg_prompt);
}

void comm_debugging_process(void)
{
    rx_packet_t* p_data = NULL;

    if(comm_get_rx_packet(&p_data))
    {
        char str_in[RX_PACKET_SIZE + 1] = {0, };
        uint32_t u32_param[8] = {0, };

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0;
//STM32
/*============================================================================================================*/
        if((!(strcmp(str_in, "help"))) || (!(strcmp(str_in, "?"))))
        {
            comm_help_command();
        }
        else if(!(strcmp(str_in, "reset")))
        {
            print(LOG_LV_INFO, "\r\n system reset \r\n");
            NVIC_SystemReset();
        }
/*============================================================================================================*/
        else if(!(strcmp(str_in, "804_read_all")))
        {
            sal_trim_read_all_registers();

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "804_i2c_init")))
        {
            sal_make_mcu_mode();
            sal_vcc_level_set(SAL_VCC_5V0);
            sal_vcc_en(PWR_ON);
            sal_vled_en(PWR_ON);
            LL_mDelay(10);
            sal_make_i2c_mode();
            LL_mDelay(10);
            sal_trim_test_enable();

            sal_trim_otp_protection(OTP_PROTECT_OFF);

            sal_trim_set_max_curr();

            decode_mode_set(DECODE_LTC_DRIVER_A);

            sal_trim_read_all_registers();
#if 1
            sal_trim_set_max_curr();
            sal_trim_t_ana_sel(TRIM_MODE_CURRENT_ITRIM);
            decode_mode_set(DECODE_LTC_DRIVER_A);

            _sal_i2c_info_t _info_ = {0, };

            _info_.reg_addr = SAL_ADDR_SETUP1;
            _info_.data = 0x7;
            sal_write_reg_i2c(&_info_);

            _info_.reg_addr = SAL_ADDR_RGB1;
            _info_.data = 0xfff;
            sal_write_reg_i2c(&_info_);

            _info_.reg_addr = SAL_ADDR_RGB2;
            _info_.data = 0xfff;
            sal_write_reg_i2c(&_info_);

            _info_.reg_addr = SAL_ADDR_RGB1;
            _info_.data = 0x000;
            sal_write_reg_i2c(&_info_);

            _info_.reg_addr = SAL_ADDR_RGB2;
            _info_.data = 0x000;
            sal_write_reg_i2c(&_info_);
#endif

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "804_w %x %x", &u32_param[0], &u32_param[1]) == 2)
        {
            _sal_i2c_info_t _info_ = {0, };
            _info_.reg_addr = u32_param[0];
            _info_.data = u32_param[1];
            if (true == sal_write_reg_i2c(&_info_))
            {
                print(LOG_LV_INFO, "\r\n\tID804 Write [0x%02X : 0x%03X]", u32_param[0], u32_param[1]);
            }

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "804_r %x", &u32_param[0]) == 1)
        {
            _sal_i2c_info_t _info_ = {0, };
            if (u32_param[0] > 0U)
            {
                _info_.reg_addr = u32_param[0];
                if (true == sal_read_reg_i2c(&_info_))
                {
                    print(LOG_LV_INFO, "\r\n\tID804 Read [0x%02X : 0x%03X]", u32_param[0], _info_.data);
                }
            }
            else
            {
                print(LOG_LV_INFO, "\r\n\tInvalid Input for Read (0x%02X)", u32_param[0]);
            }

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else
        {
            print(LOG_LV_INFO, gp_msg_what);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
    }
    else
    {
        if(gb_uart_tx_started == false)
        {
            tx_packet_t* p_data = NULL;

            if(comm_get_tx_packet(&p_data))
            {
                gb_uart_tx_started = true;

                LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)p_data->buffer);
                LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, (uint32_t)p_data->length);
                LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
            }
        }
    }
}

void UART_RxCpltCallback(void)
{
    if(LL_USART_IsActiveFlag_RXNE(USART2))
    {
        /* Read one byte from the receive data register */
        uint8_t rx_data = LL_USART_ReceiveData8(USART2);
        comm_rx_handler(rx_data);
    }
}