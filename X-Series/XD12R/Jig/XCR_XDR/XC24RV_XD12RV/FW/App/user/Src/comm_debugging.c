/**
 * @file comm_debugging.c
 * @author GT
 * @version v0.0.1
 *
 * @copyright Copyright (c) 2022, Global Technologies Inc. All rights reserved.
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "drv_gpio.h"
#include "ads124s08.h"

#include "drv_comm.h"
#include "drv_timer.h"

#include "version.h"
#include "framework.h"
#include "comm_debugging.h"
#include "ldim_conversion.h"

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

#define RX_BUFF_SIZE        (4U)
#define TX_BUFF_SIZE        (255U)

#define RX_PACKET_SIZE      (32U)
#define TX_PACKET_SIZE      (400U)

#define VA_GENERIC(_1, _2, _3, _4, _5, _6,x, ...) x
// #define Command_Param_is_(x, ...) (sscanf(str_in, x, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_Param_is_(a, b, ...) (sscanf(str_in, a b, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_is_(x) (strncmp(str_in, x, strlen(x)) == 0)

typedef struct
{
    uint8_t length;
    char buffer[RX_PACKET_SIZE];
}
rx_packet_t;

typedef struct
{
    uint16_t length;
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

volatile static LOG_LV_t gt_log_level;
static UART_t gt_uart;
bool gb_usart_tx_start_flag;

static const char* const gp_msg_prompt = "\n\rJIG> \0";
static const char* const gp_msg_okay = "\n\rOK";
static const char* const gp_msg_what = "\n\rWhat?";

static tx_packet_t* p_tx = NULL;
static uint16_t gn_tx_irq_send_pos;

__STATIC_INLINE uint8_t comm_get_rx_packet(rx_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_uart.RxInCnt != gt_uart.RxOutCnt)
    {
        *pData = gt_uart.Rxbuff + gt_uart.RxOutCnt;

        ++gt_uart.RxOutCnt;
        if(gt_uart.RxOutCnt > (RX_BUFF_SIZE -1))
        {
            gt_uart.RxOutCnt = 0;
        }

        ret = 1;
    }

    return ret;
}

__STATIC_INLINE uint8_t comm_get_tx_packet(tx_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_uart.TxInCnt != gt_uart.TxOutCnt)
    {
        *pData = gt_uart.Txbuff + gt_uart.TxOutCnt;

        ++gt_uart.TxOutCnt;
        if(gt_uart.TxOutCnt > (TX_BUFF_SIZE -1))
        {
            gt_uart.TxOutCnt = 0;
        }

        ret = 1;
    }

    return ret;
}

__STATIC_INLINE void comm_print_help(void)
{
    comm_UART_Printf(LOG_LV_INFO, "\n\r------------------ Command Help -----------------------------");

    comm_UART_Printf(LOG_LV_INFO, "\n\r  help / ?\t\t : Show command help");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_ic_start\t\t : Start timer for frequency input capture");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_ic_stop\t\t : Stop input capture and print measured frequency");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_vref_start\t : Start MCU ADC");

    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_ch_sel [d]\t : Select output channel (0 ~ XD_CH_MAX)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_xd_vcc [u]\t : XD VCC On/Off (1=On, 0=Off)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_xd_vcc_level [u]\t : XD VCC level (0=5V, 1=5.7V)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_xc_vcc [u]\t : XC VCC On/Off (1=On, 0=Off)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_vled [u]\t\t : VLED 9V On/Off (1=On, 0=Off)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_gain [u]\t\t : Set current gain (0 ~ GAIN_MAX-1)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  jig_vsync [d]\t\t : Start/Stop Vsync (1=Start, 0=Stop)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  vsync [lf]\t\t : Set vsync frequency (Hz)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  pwm_freq [lf]\t\t : Set PWM frequency (kHz)");

    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_idgen\t\t : Send IDGEN command to XDIC");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_reset\t\t : Reset XDIC");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_syncgen\t\t : Send SYNCGEN command to XDIC");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_test_en\t\t : Enter trim mode of XDIC");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_r_all\t\t : Read all registers of XDIC");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_r [x]\t\t : Read general register [x] (HEX)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_rt [x]\t\t : Read trim register [x] (HEX)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_w [x] [y]\t\t : Write [y] to general register [x] (HEX)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_wt [x] [y]\t\t : Write [y] to trim register [x] (HEX)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_ldim [d]\t\t : Set LDIM value [0 ~ 65535]");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_ldim\t\t : Show current LDIM value");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_fbi [x]\t\t : Set FBI pin (1=HI, 0=LO)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_fbo\t\t : Read FBO pin status");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_ch [d]\t\t : Set XDIC channel (0 ~ XD_CH_MAX)");

    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_debug\t\t : Initialize XD debug environment");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_trim_debug\t : Initialize XD trim environment");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_trim_vref\t\t : Trim VREF of XDIC");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_trim_osc\t\t : Trim OSC of XDIC");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_trim_ofs [x]\t : Trim offset and fix LD to 1<<12");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_trim_gain [x]\t : Trim gain and fix LD to 6<<12");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_osc_debug\t\t : Auto scan OSC trim values");

    comm_UART_Printf(LOG_LV_INFO, "\n\r  xc_debug\t\t : Initialize XC24");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xc_r [x]\t\t : Read register [x] of XC24 (HEX)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xc_w [x] [y]\t\t : Write [y] to register [x] of XC24 (HEX)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xc_r_all\t\t : Read all registers of XC24");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xc_use [d]\t\t : Enable/Disable XC24 usage (0/1)");

    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_trim_start / 1\t : Start XDIC trim process");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_screen_start / 2\t : Start XDIC screen process");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xd_dimming_start / 3\t : Start XDIC dimming process");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  xc_trim_start / 4\t : Start XC24 trim process");

    comm_UART_Printf(LOG_LV_INFO, "\n\r  log_lv [d]\t\t : Set log level (0 ~ LOG_MAX-1)");
    comm_UART_Printf(LOG_LV_INFO, "\n\r  reset\t\t\t : Reset MCU");

    comm_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------------------------------\n\r");
}

static void comm_print_startup(void)
{
    comm_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------");
    comm_UART_Printf(LOG_LV_INFO, "\n\r    [JIG for GT-XCR&XDR]");
    comm_UART_Printf(LOG_LV_INFO, "\n\r - Author  : xxx@glbltech.com");
    comm_UART_Printf(LOG_LV_INFO, "\n\r - Build   : %s", __DATE__);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - Version : %u.%u.%u", FW_MAJOR, FW_MINOR, FW_BUILD);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - GIT Rev : %s", FW_GIT_REV);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - INFO - XCR : %s, XDR : %s", XCR_MODEL_NAME, XDR_MODEL_NAME);
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
        comm_UART_Printf(LOG_LV_INFO, "\n\r - XDR Controlled By MCU");
    #elif
        comm_UART_Printf(LOG_LV_INFO, "\n\r - XDR Controlled By XCR");
    #else
        #error "XDR_CONTROL_TYPE is not defined"
    #endif
    //comm_UART_Printf(LOG_LV_INFO, "\r\n -%s %s %s", ANSI_FONT_YELLOW, (IS_XC24_Support() ? "XC24 ES2 REV ES2 IS SELECTED!" : "NOT SUPPORT XC24"), ANSI_FONT_NONE);
    //comm_UART_Printf(LOG_LV_INFO, "\r\n -%s %s %s", ANSI_FONT_YELLOW, (XD_Trim_IF_Get_OTP_Enable() ? "XDIC OTP WRITE ENABLE" : "XDIC OTP WRITE DISABLE"), ANSI_FONT_NONE);
    //comm_UART_Printf(LOG_LV_INFO, "\r\n -%s %s %s", ANSI_FONT_YELLOW, (XC_Trim_IF_Get_OTP_Enable() ? "XC24 OTP WRITE ENABLE" : "XC24 OTP WRITE DISABLE"), ANSI_FONT_NONE);
    comm_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------\r\n");
}

void comm_init(void)
{
    comm_print_startup();
    gt_log_level = LOG_LV_DEBUG;
}

void comm_debugging_process(void)
{
    rx_packet_t* p_data = NULL;

    if(comm_get_rx_packet(&p_data))
    {
        char str_in[RX_PACKET_SIZE + 1U] = {0, };
        uint32_t u32_recv_param[6] = {0, };
        //double lf_recv_param[6] = {0, };

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0;

        if(Command_is_("help") || Command_is_("?"))
        {
            comm_print_help();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        else if (Command_is_("jig_vref"))
        {
            mcu_peripheral_adc_start();
            uint16_t mcu_adc_value = mcu_peripheral_adc_get();

            comm_UART_Printf(LOG_LV_INFO, "JigBD_IF_Get_MCU_ADC()-%d\r\n", mcu_adc_value);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        else if (Command_is_("jig_ic_start"))
        {
            mcu_peripheral_tim_input_capture_start();
            comm_UART_Printf(LOG_LV_INFO, "Timer input capture started\r\n");
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if (Command_is_("jig_ic_stop"))
        {
            float freq = mcu_peripheral_tim_conversion_freq();
            comm_UART_Printf(LOG_LV_INFO, "Timer input capture freq: %.3f MHz\r\n", freq / 1000000.0f);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        else if(Command_Param_is_("xcr_w", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            uint16_t addr = (uint16_t)u32_recv_param[0];
            uint16_t param = (uint16_t)u32_recv_param[1];

            xcr24_write_grp1_reg(addr, &param, 1U);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_okay);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xcr_r", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            xcr24_read_grp1_reg((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1]);

            //comm_UART_Printf(LOG_LV_INFO, "\r\n XDIC Read --> [ 0x%02X - 0x%03X ]\r\n", u32_recv_param[0], ret);

            comm_UART_Printf(LOG_LV_INFO, gp_msg_okay);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        else if(!(strcmp(str_in, "xcr_init")))
        {
            xcr24_init();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        /*********** MCO output enable/disable *************/
        else if(!(strcmp(str_in, "mclk_enable")))
        {
            MCO2_ENABLE();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "mclk_disable")))
        {
            MCO2_DISABLE();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        /************* thread start **************/
        else if(!(strcmp(str_in, "xcr_trim_start")))
        {
            MGR_TRIM()->cmd(TRIM_CMD_XCR_START, NULL);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xdr_trim_start")))
        {
            MGR_TRIM()->cmd(TRIM_CMD_XDR_START, NULL);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xcr_test_start")))
        {
            MGR_TEST()->cmd(TEST_CMD_XCR_START, NULL);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xdr_test_start")))
        {
            MGR_TEST()->cmd(TEST_CMD_XDR_START, NULL);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        else if(Command_Param_is_("step", "%u %u %u %u", &u32_recv_param[0], &u32_recv_param[1], &u32_recv_param[2], &u32_recv_param[3]))
        {
            ldim_set_led_color_buffer((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], (uint16_t)u32_recv_param[2], (uint16_t)u32_recv_param[3]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("step", "%u", &u32_recv_param[0]))
        {
            ldim_set_led_color_buffer((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[0]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "step")))
        {
            uint16_t* p_led_color_buffer = ldim_get_led_color_buffer();
            comm_UART_Printf(LOG_LV_INFO, "\r\nR: %u, C : %u, G : %u, B : %u", p_led_color_buffer[0], p_led_color_buffer[1], p_led_color_buffer[2], p_led_color_buffer[3]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xcr_ldim_start")))
        {
            extern bool gb_xcr_ldim_start;

            gb_xcr_ldim_start = true;

            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xcr_ldim_stop")))
        {
            extern bool gb_xcr_ldim_start;

            gb_xcr_ldim_start = false;

            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xcr_ldim_force")))
        {
            uint16_t* p = ldim_get_xcr_ld_transfer_buffer();
            uint16_t len = ldim_get_xcr_ld_transfer_size();

            xcr24_set_ld_transfer(p, len);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        /*************** GPIO ON/OFF *******************/
        else if(Command_Param_is_("vled_dcdc", "%u", &u32_recv_param[0]))
        {
            gpio_set_vled_dcdc((vled_state_t)u32_recv_param[0]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("vled", "%u", &u32_recv_param[0]))
        {
            gpio_set_vled_9v((vled_state_t)u32_recv_param[0]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xd_vcc", "%u", &u32_recv_param[0]))
        {
            gpio_set_xd_vdd_5v((vcc_state_t)u32_recv_param[0]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xc_vcc", "%u", &u32_recv_param[0]))
        {
            gpio_set_xc_vdd_5v((vcc_state_t)u32_recv_param[0]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        /*************** XDR12 *******************/
        else if(!(strcmp(str_in, "xd_icc")))
        {
            xdr12_test_init_icc_stby();
            ADS114S08_Set_Start(true);
            if (true == ADS114S08_Wait_Done())
            {
                uint16_t adc = ADS114S08_Get_ADC_Value();
                float icc = JigBD_IF_Convert_Adc_To_ICC(adc);
                comm_UART_Printf(LOG_LV_INFO, "\r\nxdr icc : %.3f", icc);
            }
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xd_reset")))
        {
            xdr12_reset();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xd_idgen")))
        {
            xdr12_idgen();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xd_debug")))
        {
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            LL_mDelay(99U);
            xdr12_init();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xd_w", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            xdr12_write_by_type((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], XD12R_ADDR_TYPE_GENERAL);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xd_r", "%x", &u32_recv_param[0]))
        {
            xdr12_read_by_type((uint16_t)u32_recv_param[0], XD12R_ADDR_TYPE_GENERAL);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xdr_syncgen")))
        {
            xdr12_syncgen();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        else if(!(strcmp(str_in, "xdr_ldim")))
        {
            uint16_t* p = ldim_get_xdr_ld_transfer_buffer();
            uint16_t len = ldim_get_xdr_ld_transfer_size();

            xdr12_ld_transfer(p, len);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        /*************** XCR24 *******************/
        else if(!(strcmp(str_in, "xc_icc")))
        {
            xcr24_test_init_icc_stby();
            ADS114S08_Set_Start(true);
            if (true == ADS114S08_Wait_Done())
            {
                uint16_t adc = ADS114S08_Get_ADC_Value();
                float icc = JigBD_IF_Convert_Adc_To_ICC(adc);
                comm_UART_Printf(LOG_LV_INFO, "\r\nxcr icc : %.3f", icc);
            }
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xc_debug")))
        {
            xcr24_init();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        /* TIMER */
        else if(!(strcmp(str_in, "vsync_start")))
        {
            tim_vsync_out_start();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "vsync_stop")))
        {
            tim_vsync_out_stop();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("vsync_freq", "%u", &u32_recv_param[0]))
        {
            tim_set_vsync_out_freq((float)u32_recv_param[0]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }

        else if(!(strcmp(str_in, "reset")))
        {
            NVIC_SystemReset();
        }
        else
        {
            comm_UART_Printf(LOG_LV_INFO, gp_msg_what);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
    }
    else
    {
        if(gb_usart_tx_start_flag == false)
        {
            if(comm_get_tx_packet(&p_tx))
            {
                UART_Tx_DMA_Start((uint32_t)p_tx->buffer, (uint16_t)p_tx->length);
                gn_tx_irq_send_pos = 0;
                gb_usart_tx_start_flag = true;
            }
        }
    }
}

void comm_UART_Printf(LOG_LV_t lv, const char *fmt, ...)
{
    if(lv >= gt_log_level)
    {
        int len = 0;
        va_list ap;
        char temp_buf[TX_PACKET_SIZE] = { 0U };

        va_start(ap, fmt);
        len = vsnprintf(temp_buf, (TX_PACKET_SIZE - 1), fmt, ap);
        va_end(ap);

        if(len > 0)
        {
            int final_len = 0;
            char* p_dest = gt_uart.Txbuff[gt_uart.TxInCnt].buffer;

            if (lv > LOG_LV_WARN)
            {
                final_len = snprintf(p_dest, (TX_PACKET_SIZE - 1U), "%s%s%s", ANSI_FONT_RED, temp_buf, ANSI_FONT_NONE);
            }
            else
            {
                final_len = snprintf(p_dest, (TX_PACKET_SIZE - 1U), "%s", temp_buf);
            }
            gt_uart.Txbuff[gt_uart.TxInCnt].length = final_len;

            ++gt_uart.TxInCnt;
            if(gt_uart.TxInCnt > (TX_BUFF_SIZE -1U))
            {
                gt_uart.TxInCnt = 0U;
            }
        }
    }
}

void comm_rx_handler(uint8_t rx_data)
{
    UART_PutChar(rx_data); /* echo back */

    switch(rx_data)
    {
        case '\n':
        case '\r':
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
            if(gt_uart.RxInCnt > (RX_BUFF_SIZE -1))
            {
                gt_uart.RxInCnt = 0;
            }
            break;
        }
        case CLI_KEY_BACK:
        case CLI_KEY_DEL:
        {
            if(gt_uart.Rxbuff[gt_uart.RxInCnt].length)
            {
                //UART_PutChar(CLI_KEY_BACK);
                UART_PutChar(' ');
                UART_PutChar(CLI_KEY_BACK);

                --gt_uart.Rxbuff[gt_uart.RxInCnt].length;
            }
            break;
        }
        default:
        {
            if((rx_data == CLI_KEY_UP) && (gt_uart.Rxbuff[gt_uart.RxInCnt].length > 2) && (gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[gt_uart.Rxbuff[gt_uart.RxInCnt].length-2] == 0x1B) && (gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[gt_uart.Rxbuff[gt_uart.RxInCnt].length -1] == 0x5B))
            {
                UART_PutChar('\n');
                UART_PutChar('\r');

                gt_uart.Rxbuff[gt_uart.RxInCnt].length = 0;
            }
            else
            {
                if(gt_uart.Rxbuff[gt_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
                {
                    gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[gt_uart.Rxbuff[gt_uart.RxInCnt].length] = rx_data;
                    ++gt_uart.Rxbuff[gt_uart.RxInCnt].length;
                }
            }
            break;
        }
    }
}

void comm_tx_handler(void)
{
    if(p_tx)
    {
        if(p_tx->length > gn_tx_irq_send_pos)
        {
            LL_USART_TransmitData8(USART2, p_tx->buffer[gn_tx_irq_send_pos]);
            ++gn_tx_irq_send_pos;
            if(p_tx->length == gn_tx_irq_send_pos)
            {
                LL_USART_DisableIT_TXE(USART2);
                gn_tx_irq_send_pos = 0;
                gb_usart_tx_start_flag = false;

                p_tx = NULL;
            }
        }
    }
    else
    {
        LL_USART_DisableIT_TXE(USART2);
        gb_usart_tx_start_flag = false;
    }
}
