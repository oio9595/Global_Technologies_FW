
#include "comm_debug.h"
#include "drv_sal.h"
#include "drv_sal_trim.h"
#include "switch.h"
#include "ADS124S08.h"

#include "sal_driver.h"
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
static LOG_LV_T gt_log_lv = LOG_LV_DEBUG;

bool gb_uart_tx_started;

static const char* const gp_msg_prompt = "\n\rSAL> \0";
static const char* const gp_msg_okay = "\n\rOK";
static const char* const gp_msg_what = "\n\rWhat?";

static const char* gs_uart_log_lv_str[LOG_LV_MAX] =
{
    "LOG_LV_DEBUG",
    "LOG_LV_INFO",
    "LOG_LV_ERROR",
};

static const char* gs_sal_type_str[2] =
{
    "SAL_TYPE_A",
    "SAL_TYPE_B",
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
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length == 0 && gt_last_input.length != 0)
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length = gt_last_input.length;
            memcpy(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer, gt_last_input.buffer, gt_last_input.length);
        }
        else
        {
            uart_echo('\n');
            //uart_echo('\r');

            gt_last_input.length = gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;
            memcpy(gt_last_input.buffer, gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer, gt_last_input.length);
        }
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
    print(LOG_LV_INFO, "\r\n    ====         COMMAND                  ====         FUNCTION        ====");
    print(LOG_LV_INFO, "\r\n    =======================================================================");
    print(LOG_LV_INFO, "\r\n    ====       cmd                        ====   display cmd list      ====");
    print(LOG_LV_INFO, "\r\n    ====       sal_w [dec] [hex] [hex]    ====   dev_id, cmd, value    ====");
    print(LOG_LV_INFO, "\r\n    ====       sal_r [dec] [hex]          ====   dev_id, cmd           ====");
    print(LOG_LV_INFO, "\r\n    ====       step [dec] [dec] [dec]     ====   red, green, blue      ====");

    print(LOG_LV_INFO, "\r\n    ====       sal_reset [dec]            ====   dev_id                ====");
    print(LOG_LV_INFO, "\r\n    ====       sal_initbidir [dec]        ====   dev_id                ====");
    print(LOG_LV_INFO, "\r\n    ====       sal_clrerror [dec]         ====   dev_id                ====");
    print(LOG_LV_INFO, "\r\n    ====       sal_gosleep [dec]          ====   dev_id                ====");
    print(LOG_LV_INFO, "\r\n    ====       sal_goactive [dec]         ====   dev_id                ====");
    print(LOG_LV_INFO, "\r\n    ====       sal_godeepsleep [dec]      ====   dev_id                ====");

    print(LOG_LV_INFO, "\r\n    ====       sal_crc [dec]              ====   0 : off / 1 : on      ====");
    print(LOG_LV_INFO, "\r\n    ====       sal_crc_manual [dec] [dec] ====   on/off, crc value     ====");

    print(LOG_LV_INFO, "\r\n    =======================================================================");
    print(LOG_LV_INFO, "\r\n");

    print(LOG_LV_INFO, gp_msg_prompt);
}

void comm_print_startup(void)
{
    print(LOG_LV_INFO, "\n\r--------------------------------------");
    print(LOG_LV_INFO, "\n\r    [GT-SAL(ES0) TEST/TRIM JIG]");
    print(LOG_LV_INFO, "\n\r--------------------------------------");
    print(LOG_LV_INFO, "\n\r-Project\t: STM32 SAL JIG");
    print(LOG_LV_INFO, "\n\r-Author\t\t: xxx@glbltech.com");
    print(LOG_LV_INFO, "\n\r-Build\t\t: %s", __DATE__);
    print(LOG_LV_INFO, "\n\r-Log_lv\t\t: %s", gs_uart_log_lv_str[gt_log_lv]);
    print(LOG_LV_INFO, "\n\r\033[0;33m-SAL Type\t: %s\033[0m", gs_sal_type_str[SAL_TYPE]);
    print(LOG_LV_INFO, "\n\r--------------------------------------");
    print(LOG_LV_INFO, gp_msg_prompt);
}

void comm_debugging_process(void)
{
    rx_packet_t* p_data = NULL;

    if(comm_get_rx_packet(&p_data))
    {
        char str_in[RX_PACKET_SIZE + 1] = {0, };

        //uint64_t u64_param[8] = {0, };
        uint32_t u32_param[8] = {0, };
        // float f_param1 = 0;

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
        else if(!(strcmp(str_in, "input_capture_start")))
        {
            tim_input_capture_start();
            while(gb_input_capture_started){}

            print(LOG_LV_INFO, "input freq is %.8f kHz\r\n", convert_count_to_freq(tim_input_capture_get_average_cnt()));
            print(LOG_LV_INFO, gp_msg_prompt);
        }
//JIG H/W
/*============================================================================================================*/
        else if(!(strcmp(str_in, "sal_mode_mcu")))
        {
            sal_make_mcu_mode();
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "sal_mode_i2c")))
        {
            sal_make_i2c_mode();
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_test %u", &u32_param[0]) == 1)
        {
            if (u32_param[0] == 0)
            {
                SAL_MODE_LO();
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else if (u32_param[0] == 1)
            {
                SAL_MODE_HI();
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input sal_test [%u] - [0 ~ 1]\r\n", u32_param[0]);
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_vcc_lv %u", &u32_param[0]) == 1)
        {
            if (u32_param[0] < 2)
            {
                sal_vcc_level_set((_sal_vcc_level_t)u32_param[0]);
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input sal_vcc_lv [%u] - [0 ~ 1]\r\n", u32_param[0]);
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_vcc %u", &u32_param[0]) == 1)
        {
            if (u32_param[0] < 2)
            {
                sal_vcc_en((power_state_t)u32_param[0]);
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input sal_vcc [%u] - [0 ~ 1]\r\n", u32_param[0]);
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_vled %u", &u32_param[0]) == 1)
        {
            if (u32_param[0] < 2)
            {
                sal_vled_en((power_state_t)u32_param[0]);
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input sal_vled [%u] - [0 ~ 1]\r\n", u32_param[0]);
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "adc_ch %x", &u32_param[0]) == 1)
        {
            if (u32_param[0] < ADC_CH_MAX)
            {
                uint16_t temp_adc_val = 0;
                gn_adc_read_count = ADS114S08_READ_COUNT;
                ads114s08_select_single_ended_input(u32_param[0]);
                ads114s08_set_start(1);

                while (!gb_ads114s08_drdy_done) {}

                gb_ads114s08_drdy_done = 0;
                temp_adc_val = get_adc_value();
                print(LOG_LV_DEBUG, "ADC CH [u32_param[0]] - [%u] / [%.3fmV]\r\n", temp_adc_val, convert_adc_to_mvoltage(temp_adc_val));
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input adc_ch [%u] - [0 ~ %u]\r\n", u32_param[0], (ADC_CH_MAX - 1));
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "decode_ch %u", &u32_param[0]) == 1)
        {
            decode_mode_set((decode_mode_t)u32_param[0]);
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
//SAL NORMAL
/*============================================================================================================*/
        else if(sscanf(str_in, "step %u %u %u", &u32_param[0], &u32_param[1], &u32_param[2]) == 3)
        {
            if ((u32_param[0] < 256) && (u32_param[1] < 256) && (u32_param[2] < 256))
            {
                uint32_t* p_buffer = sal_get_rgb_buffer();

                for (uint8_t idx = 0 ; idx < 100 ; ++idx)
                {
                    *(p_buffer + idx) = ((u32_param[0] << 16) | (u32_param[1] << 8) | (u32_param[2] << 0));
                }
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input step [%u, %u, %u] - [0 ~ 255]\r\n", u32_param[0], u32_param[1], u32_param[2]);
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_w %u %x %x", &u32_param[0], &u32_param[1], &u32_param[2]) == 3)
        {
#if 0
            _sal_single_ended_info_t _info_ = {0, };
            _info_.dev_id = u32_param[0];
            _info_.command = u32_param[1];
            _info_.data = u32_param[2];
            if (u32_param[1] == 0x88 || u32_param[1] == 0xA0)
            {
                _info_.data_size = 24;
            }
            else
            {
                _info_.data_size = 12;
            }
            sal_write_reg_single_ended(&_info_);
#endif
            sal_set_write_target(u32_param[0], u32_param[1], u32_param[2]);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_ws %u %x %x", &u32_param[0], &u32_param[1], &u32_param[2]) == 3)
        {
#if 0
            _sal_single_ended_info_t _info_ = {0, };
            _info_.dev_id = u32_param[0];
            _info_.command = u32_param[1];
            _info_.data = u32_param[2];
            if (u32_param[1] == 0xC8 || u32_param[1] == 0xE0)
            {
                _info_.data_size = 24;
            }
            else if (u32_param[1] == 0xF3 || u32_param[1] == 0xF4 || u32_param[1] == 0xF8 || u32_param[1] == 0xFC)
            {
                _info_.data_size = 0;
            }
            else
            {
                _info_.data_size = 12;
            }
            sal_write_SR_reg_single_ended(&_info_);
#endif
            sal_set_write_SR_target(u32_param[0], u32_param[1], u32_param[2]);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_r %u %x", &u32_param[0], &u32_param[1]) == 2)
        {
#if 0
            _sal_single_ended_info_t _info_ = {0, };
            _info_.dev_id = u32_param[0];
            _info_.command = u32_param[1];
            _info_.data = 0;
            if (u32_param[1] == 0x44 || u32_param[1] == 0x48 || u32_param[1] == 0x60)
            {
                _info_.data_size = 24;
            }
            else
            {
                _info_.data_size = 12;
            }
            sal_read_reg_single_ended(&_info_);
#endif
            sal_set_read_target(u32_param[0], u32_param[1]);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "sal_r_status2")))
        {
            sal_set_status2_read_enable();
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_reset %u", &u32_param[0]) == 1)
        {
            _sal_single_ended_info_t _info_ = {0, };

            _info_.dev_id = u32_param[0];
            _info_.command = CMD_SAL_RESET;
            _info_.data_size = 0;
            _info_.data = 0;

            print(LOG_LV_INFO, "send reset cmd to dev_id : %u!!\r\n", u32_param[0]);

            sal_write_reg_single_ended(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_initbidir %u", &u32_param[0]) == 1)
        {
            _sal_single_ended_info_t _info_ = {0, };

            _info_.dev_id = u32_param[0];
            _info_.command = CMD_SAL_INITBIDIR;
            if (u32_param[0] == 0 || u32_param[0] >= 0xFEF)
            {
                _info_.data_size = 24;
            }
            else
            {
                _info_.data_size = 0;
            }
            _info_.data = 0;

            print(LOG_LV_INFO, "send initbidir cmd to dev_id : %u!!\r\n", u32_param[0]);

            sal_read_reg_single_ended(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_clrerror %u", &u32_param[0]) == 1)
        {
            _sal_single_ended_info_t _info_ = {0, };

            _info_.dev_id = u32_param[0];
            _info_.command = CMD_SAL_CLRERROR;
            _info_.data_size = 0;
            _info_.data = 0;

            print(LOG_LV_INFO, "send clrerror cmd to dev_id : %u!!\r\n", u32_param[0]);

            sal_write_reg_single_ended(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_gosleep %u", &u32_param[0]) == 1)
        {
            _sal_single_ended_info_t _info_ = {0, };

            _info_.dev_id = u32_param[0];
            _info_.command = CMD_SAL_GOSLEEP;
            _info_.data_size = 0;
            _info_.data = 0;

            print(LOG_LV_INFO, "send gosleep cmd to dev_id : %u!!\r\n", u32_param[0]);

            sal_write_reg_single_ended(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_goactive %u", &u32_param[0]) == 1)
        {
            _sal_single_ended_info_t _info_ = {0, };

            _info_.dev_id = u32_param[0];
            _info_.command = CMD_SAL_GOACTIVE;
            _info_.data_size = 0;
            _info_.data = 0;

            print(LOG_LV_INFO, "send goactive cmd to dev_id : %u!!\r\n", u32_param[0]);

            sal_write_reg_single_ended(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_godeepsleep %u", &u32_param[0]) == 1)
        {
            _sal_single_ended_info_t _info_ = {0, };

            _info_.dev_id = u32_param[0];
            _info_.command = CMD_SAL_GODEEPSLEEP;
            _info_.data_size = 0;
            _info_.data = 0;

            print(LOG_LV_INFO, "send godeepsleep cmd to dev_id : %u!!\r\n", u32_param[0]);

            sal_write_reg_single_ended(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_uvlo %u", &u32_param[0]) == 1)
        {
            _sal_single_ended_info_t _info_ = {0, };

            _info_.dev_id = 1;
            _info_.command = CMD_SAL_SET_SETUP2;
            _info_.data_size = 12;
            _info_.data = ((u32_param[0] << 2) | 2);

            print(LOG_LV_INFO, "set uvlo to %u!!\r\n", u32_param[0]);

            sal_write_reg_single_ended(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_crc %u", &u32_param[0]) == 1)
        {
            sal_make_crc_enable(u32_param[0]);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_crc_manual %u %u", &u32_param[0], &u32_param[1]) == 2)
        {
            if ((u32_param[0] < 2) && (u32_param[1] < 256))
            {
                sal_set_crc_manual(u32_param[0], u32_param[1]);
                if (u32_param[0] == 0)
                {
                    print(LOG_LV_INFO, "Disable Downstream Manual CRC\r\n");
                }
                else
                {
                    print(LOG_LV_INFO, "Enable Downstream Manual CRC to %u\r\n", u32_param[1]);
                }
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input sal_crc_manual [%u / %u] - [0 ~ %u / 0 ~ %u]\r\n", u32_param[0], u32_param[1], 1, 255);
            }

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_preamble %u", &u32_param[0]) == 1)
        {
            if (u32_param[0] < 16)
            {
                sal_set_preamble(u32_param[0]);
                print(LOG_LV_INFO, "Set sal preamble to [%u]\r\n", u32_param[0]);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input sal_preamble [%u] - [0 ~ 15]\r\n", u32_param[0]);
            }

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "sal_preamble")))
        {
            print(LOG_LV_INFO, "sal_preamble [0x%02X]\r\n", sal_get_preamble());

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "sal_debug")))
        {
            sal_init();

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "p %u", &u32_param[0]) == 1)
        {
            if (u32_param[0] < DEMO_PAT_MAX)
            {
                sal_set_pattern((demo_pat_t)u32_param[0]);
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input p [%u] - [0 ~ %u]\r\n", u32_param[0], (DEMO_PAT_MAX - 1));
            }

            print(LOG_LV_INFO, gp_msg_prompt);
        }
//SAL TRIM
/*============================================================================================================*/
        else if(sscanf(str_in, "sal_t_ana_sel %u", &u32_param[0]) == 1)
        {
            if (u32_param[0] < TRIM_MODE_MAX)
            {
                sal_trim_t_ana_sel((trim_mode_t)u32_param[0]);
                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input sal_t_ana_sel [%u] - [0 ~ %u]\r\n", u32_param[0], (TRIM_MODE_MAX - 1));
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "trim_start")) || !(strcmp(str_in, "1")))
        {
            gb_trim_start_flag = true;
            sal_trim_start();
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "step_wt %u %u %u", &u32_param[0], &u32_param[1], &u32_param[2]) == 3)
        {
            if ((u32_param[0] <= 255) && (u32_param[1] <= 255) && (u32_param[2] <= 255))
            {
                uint8_t g_msb = ((u32_param[1] & 0xF0) >> 4);
                uint8_t g_lsb = ((u32_param[1] & 0x0F) >> 0);

                _sal_i2c_info_t _info_ = {0, };
                _info_.reg_addr = SAL_ADDR_RGB1;
                _info_.data = ((u32_param[0] << 4) | g_msb);
                sal_write_reg_i2c(&_info_);

                _info_.reg_addr = SAL_ADDR_RGB2;
                _info_.data = ((g_lsb << 8) | u32_param[2]);
                sal_write_reg_i2c(&_info_);

                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input step [%u, %u, %u] - [0 ~ 255]\r\n", u32_param[0], u32_param[1], u32_param[2]);
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "pwm_max %u %u %u", &u32_param[0], &u32_param[1], &u32_param[2]) == 3)
        {
            if ((u32_param[0] <= 4095) && (u32_param[1] <= 4095) && (u32_param[2] <= 4095))
            {
                _sal_i2c_info_t _info_ = {0, };

                _info_.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR14;
                _info_.data = u32_param[0];
                sal_write_reg_i2c(&_info_);

                _info_.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR15;
                _info_.data = u32_param[1];
                sal_write_reg_i2c(&_info_);

                _info_.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR16;
                _info_.data = u32_param[2];
                sal_write_reg_i2c(&_info_);

                print(LOG_LV_INFO, gp_msg_okay);
            }
            else
            {
                print(LOG_LV_INFO, "Invalid Input step [%u, %u, %u] - [0 ~ 255]\r\n", u32_param[0], u32_param[1], u32_param[2]);
            }
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_wt %x %x", &u32_param[0], &u32_param[1]) == 2)
        {
            _sal_i2c_info_t _info_ = {0, };
            _info_.reg_addr = u32_param[0];
            _info_.data = u32_param[1];
            sal_write_reg_i2c(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_rt %x", &u32_param[0]) == 1)
        {
            _sal_i2c_info_t _info_ = {0, };
            if (u32_param[0])
            {
                _info_.reg_addr = u32_param[0];
                sal_read_reg_i2c(&_info_);
            }
            else
            {
                sal_trim_read_all_registers();
            }

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "sal_trim_debug")))
        {
            sal_make_mcu_mode();
            sal_vcc_level_set(SAL_VCC_5V0);
            sal_vcc_en(PWR_ON);
            sal_vled_en(PWR_ON);
            LL_mDelay(10);
            MX_I2C1_Init();
            sal_make_i2c_mode();
            LL_mDelay(10);
            sal_trim_test_enable();

            sal_trim_otp_protection(OTP_PROTECT_OFF);

            sal_trim_set_max_curr();

            decode_mode_set(DECODE_LTC_DRIVER_A);

            sal_trim_read_all_registers();
#if 0
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
        else if(!(strcmp(str_in, "sal_trim_adc_init")))
        {
            sal_adc_test_init();
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "sal_trim_adc_start")))
        {
            LED_LO();
            sal_adc_test_read_adc();
            LED_HI();
        }
        else if(!(strcmp(str_in, "sal_trim_curr_init")))
        {
            sal_current_test_init();
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "sal_trim_curr_start")))
        {
            sal_current_test_read_adc();
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }

        else if(!(strcmp(str_in, "sal_trim_osc_init")))
        {
            sal_osc_test_init();
            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(sscanf(str_in, "sal_trim_osc_start %u", &u32_param[0]) == 1)
        {
            sal_osc_test_start(u32_param[0]);
        }
        else if(!(strcmp(str_in, "temp")))
        {
            _sal_i2c_info_t _info_ = {0, };
            _info_.reg_addr = 0x03;
            sal_read_reg_i2c(&_info_);

            _info_.reg_addr = 0x0C;
            sal_read_reg_i2c(&_info_);

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "sal_otp_test")))
        {
            sal_otp_burn_test();

            print(LOG_LV_INFO, gp_msg_okay);
            print(LOG_LV_INFO, gp_msg_prompt);
        }
/*============================================================================================================*/
        ///////////////////////////////////////
        /*************************************/
        else if(sscanf(str_in, "d_init %u", &u32_param[0]) == 1)
        {
            if(u32_param[0] < 2)
            {
                sal_InitType ChainInitPtr = {
                        .firstLedAdr = DEV_ADDR_UNICAST_MIN,
                        .crcEnable = true,
                        .tempCmpEnable = false,
                        .phaseShift = false,
                    };
                sal_ReadDataResultType ChainInitResultPtr = { 0, };
                uint8_t strip_num = u32_param[0];
                sal_ReturnType ret = sal_init_strip((const sal_InitType*)&ChainInitPtr, &ChainInitResultPtr, strip_num);

                print(LOG_LV_INFO, "\r\n sal_init_strip(%u) ret - %u, chainLength - %u \r\n", strip_num, ret, ChainInitResultPtr.chainLength);
            }
            else
            {
                print(LOG_LV_INFO, "\r\n [Error] Invalid index : %u\r\n", u32_param[0]);
            }
        }
        else if(sscanf(str_in, "d_power %u %u", &u32_param[0], &u32_param[1]) == 2)
        {
            if(u32_param[0] < 2 && u32_param[1] < SAL_PWR_MAX)
            {
                sal_ReturnType ret = sal_set_power_mode((sal_PowerMode_t)u32_param[1], DEV_ADDR_BROADCAST, (uint8_t)u32_param[0]);

                print(LOG_LV_INFO, "\r\n sal_set_power_mode(%u) ret - %u, power mode - %u \r\n", u32_param[0], ret, u32_param[1]);
            }
            else
            {
                print(LOG_LV_INFO, "\r\n [Error] Invalid index : %u, %u\r\n", u32_param[0], u32_param[1]);
            }
        }
        else if(sscanf(str_in, "d_rgb %u %u %u %u %u", &u32_param[0], &u32_param[1], &u32_param[2], &u32_param[3], &u32_param[4]) == 5)
        {
            if(u32_param[0] < 2 && u32_param[1] < SAL_PWR_MAX)
            {
                sal_ReturnType ret = sal_set_RGB((uint8_t)u32_param[2], (uint8_t)u32_param[3], (uint8_t)u32_param[4], (uint16_t)u32_param[1], (uint8_t)u32_param[0]);

                print(LOG_LV_INFO, "\r\n sal_set_RGB(%u, %u) ret - %u, rgb(%u, %u, %u) \r\n",u32_param[0], u32_param[1],  ret, u32_param[2], u32_param[3], u32_param[4]);
            }
            else
            {
                print(LOG_LV_INFO, "\r\n [Error] Invalid index : %u, %u\r\n", u32_param[0], u32_param[1]);
            }
        }
        /*************************************/
        ///////////////////////////////////////

        else if(!(strcmp(str_in, "cmd")))
        {
            display_command_list();
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