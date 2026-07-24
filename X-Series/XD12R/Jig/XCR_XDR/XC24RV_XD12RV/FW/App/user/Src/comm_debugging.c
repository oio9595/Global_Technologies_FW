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
#include <math.h>

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

#if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    #define MSG_XDR_CTL "XDR Controlled By MCU"
#elif (XDR_CONTROL_TYPE == XDR_CONTROLLED_XCR)
    #define MSG_XDR_CTL "XDR Controlled By XCR"
#else
    #error "XDR_CONTROL_TYPE is not defined"
#endif

#if (XDR_EFUSE == XDR_EFUSE_SKIP)
    #define MSG_XDR_EFUSE "XDR EFUSE SKIP"
#elif (XDR_EFUSE == XDR_EFUSE_BURN)
    #define MSG_XDR_EFUSE "XDR EFUSE BURN"
#else
    #error "XDR_EFUSE is not defined"
#endif

#if (XCR_EFUSE == XCR_EFUSE_SKIP)
    #define MSG_XCR_EFUSE "XCR EFUSE SKIP"
#elif (XCR_EFUSE == XCR_EFUSE_BURN)
    #define MSG_XCR_EFUSE "XCR EFUSE BURN"
#else
    #error "XCR_EFUSE is not defined"
#endif

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
static rx_packet_t gt_last_uart_rx;
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
        if(gt_uart.TxOutCnt > (TX_BUFF_SIZE - 1U))
        {
            gt_uart.TxOutCnt = 0U;
        }

        ret = 1;
    }

    return ret;
}

__STATIC_INLINE void comm_print_help(void)
{
    comm_UART_Printf(LOG_LV_INFO, "\r\n------------------ Command Help -----------------------------");

    comm_UART_Printf(LOG_LV_INFO, "\r\n  help / ?                : Show command help");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  jig_vref                : Get MCU ADC value");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  jig_ic_start            : Start timer input capture");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  jig_ic_stop             : Stop input capture & print freq");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  mclk_enable / disable   : Enable/Disable MCO2 clock");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  fllsync_start / stop    : Start/Stop FLL sync timer");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  vsync_start / stop      : Start/Stop VSYNC output");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  vsync_freq [u]          : Set VSYNC output frequency");

    comm_UART_Printf(LOG_LV_INFO, "\r\n  xcr_trim_start          : Run XCR Trim Manager");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xdr_trim_start / 1      : Run XDR Trim Manager");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xcr_test_start          : Run XCR Test Manager");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xdr_test_start          : Run XDR Test Manager");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xcr_sweep_start         : Run XCR Sweep Test");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xdr_sweep_start         : Run XDR Sweep Test");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  launch                  : Launch system (VDD ON + XDR Init + LDIM + VSYNC)");

    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_icc                  : Read XCR Standby ICC via ADS114S08");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_debug                : Power ON & Init XCR24");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_trim_debug           : Power ON & Init XCR24 Trim");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_g1_w [hex_addr] [val]: Write XCR24 Group 1 Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_g1_r [hex_addr]      : Read XCR24 Group 1 Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_g2_w [hex_addr] [val]: Write XCR24 Group 2 Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_g2_r [hex_addr]      : Read XCR24 Group 2 Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_wt [hex_addr] [val]  : Write XCR24 OTP Control Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_rt [hex_addr]        : Read XCR24 OTP Control Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_test                 : Init XCR24 Test Mode & Start VSYNC");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_r_all                : Read all XCR24 registers");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_fll [idx] [cnt]      : Set XCR24 FLL count (idx < 3)");

    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_icc                  : Read XDR Standby ICC via ADS114S08");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_reset                : Reset XDR12");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_idgen                : Send IDGEN command to XDR12");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_syncgen              : Send SYNCGEN command to XDR12");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_debug                : Power ON & Init XDR12");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_trim_debug           : Power ON & Init XDR12 Trim");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_w [hex_addr] [val]   : Write XDR12 General Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_r [hex_addr]         : Read XDR12 General Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_wt [hex_addr] [val]  : Write XDR12 Mirror Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_rt [hex_addr]        : Read XDR12 Mirror Register");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_r_all                : Read all XDR12 registers");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xdr_ldim                : Run XDR12 Local Dimming Transfer");

    comm_UART_Printf(LOG_LV_INFO, "\r\n  step [idx] [r] [g] [b]  : Set specific/all LED color (idx 0=All)");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  step [idx] [val]        : Set specific/all LED grayscale");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  step                    : Print current buffer values for all LEDs");

    comm_UART_Printf(LOG_LV_INFO, "\r\n  vled_dcdc [u]           : Set VLED DCDC Power state");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  vled [u]                : Set VLED 9V Power state");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xd_vcc [u]              : Set XD VDD 5V Power state");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  xc_vcc [u]              : Set XC VDD 5V Power state");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  log_lv [d]              : Set system log level");
    comm_UART_Printf(LOG_LV_INFO, "\r\n  reset                   : Perform MCU System Reset");

    comm_UART_Printf(LOG_LV_INFO, "\r\n--------------------------------------------------------------\r\n");
}

static void comm_set_log_lv(LOG_LV_t log_lv)
{
    if (log_lv < LOG_LV_MAX)
    {
        gt_log_level = log_lv;
    }
    else
    {
        comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, invalid log level (%u)", __func__, log_lv);
    }
}

static void comm_print_startup(void)
{
    comm_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------");
    comm_UART_Printf(LOG_LV_INFO, "\n\r    [JIG for GT-XCR&XDR]");
    comm_UART_Printf(LOG_LV_INFO, "\n\r - Author  : xxx@glbltech.com");
    comm_UART_Printf(LOG_LV_INFO, "\n\r - Build   : %s", __DATE__);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - Version : %u.%u.%u", FW_MAJOR, FW_MINOR, FW_BUILD);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - GIT Rev : %s", FW_GIT_REV);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - Model   : %s, %s", XCR_MODEL_NAME, XDR_MODEL_NAME);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - %s", MSG_XDR_CTL);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - %s", MSG_XDR_EFUSE);
    comm_UART_Printf(LOG_LV_INFO, "\n\r - %s", MSG_XCR_EFUSE);

    //comm_UART_Printf(LOG_LV_INFO, "\r\n -%s %s %s", ANSI_FONT_YELLOW, (IS_XC24_Support() ? "XC24 ES2 REV ES2 IS SELECTED!" : "NOT SUPPORT XC24"), ANSI_FONT_NONE);
    //comm_UART_Printf(LOG_LV_INFO, "\r\n -%s %s %s", ANSI_FONT_YELLOW, (XD_Trim_IF_Get_OTP_Enable() ? "XDIC OTP WRITE ENABLE" : "XDIC OTP WRITE DISABLE"), ANSI_FONT_NONE);
    //comm_UART_Printf(LOG_LV_INFO, "\r\n -%s %s %s", ANSI_FONT_YELLOW, (XC_Trim_IF_Get_OTP_Enable() ? "XC24 OTP WRITE ENABLE" : "XC24 OTP WRITE DISABLE"), ANSI_FONT_NONE);
    comm_UART_Printf(LOG_LV_INFO, "\n\r--------------------------------------\r\n");
    comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
}

void comm_init(void)
{
    comm_print_startup();
    comm_set_log_lv(LOG_LV_INFO);
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
            comm_UART_Printf(LOG_LV_INFO, "Timer input capture freq: %.3f Hz\r\n", (double)(freq));
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
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
        else if(!(strcmp(str_in, "fllsync_start")))
        {
            tim_fllsync_start();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "fllsync_stop")))
        {
            tim_fllsync_stop();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
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

        /************* thread start **************/
        else if(!(strcmp(str_in, "xcr_trim_start")) || !(strcmp(str_in, "1")))
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
        else if(!(strcmp(str_in, "xdr_sweep_start")))
        {
            MGR_TEST()->cmd(TEST_CMD_XDR_SWEEP_START, NULL);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xcr_sweep_start")))
        {
            MGR_TEST()->cmd(TEST_CMD_XCR_SWEEP_START, NULL);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "launch")))
        {
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            LL_mDelay(99U);
            xdr12_init();
            LL_mDelay(9U);

            ldim_set_block_color_buffer(LDIM_BLK_INDEX_ALL, 100U, 100U, 100U);
            tim_vsync_out_start();
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
                float icc = JigBD_IF_Convert_Adc_To_ICC_XC(adc);
                comm_UART_Printf(LOG_LV_INFO, "\r\nxcr icc : %.3f", (double)(icc));
            }
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xc_debug")))
        {
            gpio_set_xc_vdd_5v(VCC_ON_3V3);
            LL_mDelay(99U);
            xcr24_init();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xc_trim_debug")))
        {
            gpio_set_xc_vdd_5v(VCC_ON_3V3);
            LL_mDelay(99U);
            xcr24_trim_init();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xc_g1_w", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            uint16_t addr = (uint16_t)u32_recv_param[0];
            uint16_t param = (uint16_t)u32_recv_param[1];
            if (true == tim_get_vsync_out_running_flag())
            {
                tim_set_xc_write_info(addr, param, XCR_RW_GRP1);
            }
            else
            {
                xcr24_write_grp1_reg(addr, &param, 1U);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_okay);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xc_g1_r", "%x", &u32_recv_param[0]))
        {
            if (true == tim_get_vsync_out_running_flag())
            {
                tim_set_xc_read_info((uint16_t)u32_recv_param[0], XCR_RW_GRP1);
            }
            else
            {
                uint16_t xcr = xcr24_read_grp1_reg((uint16_t)u32_recv_param[0], 1U);
                comm_UART_Printf(LOG_LV_INFO, "\r\nXCR GRP1 Read --> [ 0x%02X - 0x%04X ]\r\n", u32_recv_param[0], xcr);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xc_g2_w", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            uint16_t addr = (uint16_t)u32_recv_param[0];
            uint16_t param = (uint16_t)u32_recv_param[1];
            if (true == tim_get_vsync_out_running_flag())
            {
                tim_set_xc_write_info(addr, param, XCR_RW_GRP2);
            }
            else
            {
                xcr24_write_grp2_reg(addr, &param, 1U);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_okay);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xc_g2_r", "%x", &u32_recv_param[0]))
        {
            if (true == tim_get_vsync_out_running_flag())
            {
                tim_set_xc_read_info((uint16_t)u32_recv_param[0], XCR_RW_GRP2);
            }
            else
            {
                uint16_t xcr = xcr24_read_grp2_reg((uint16_t)u32_recv_param[0], 1U);
                comm_UART_Printf(LOG_LV_INFO, "\r\nXCR GRP2 Read --> [ 0x%02X - 0x%04X ]\r\n", u32_recv_param[0], xcr);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xc_wt", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            uint16_t addr = (uint16_t)u32_recv_param[0];
            uint16_t param = (uint16_t)u32_recv_param[1];
            if (addr >= XCR_OTP_BASE_ADDR)
            {
                if (true == tim_get_vsync_out_running_flag())
                {
                    tim_set_xc_write_info(addr, param, XCR_RW_GRP1);
                }
                else
                {
                    xcr24_write_otp_control(addr - XCR_OTP_BASE_ADDR, &param, 1U);
                    comm_UART_Printf(LOG_LV_INFO, gp_msg_okay);
                    comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
                }
            }
            else
            {
                comm_UART_Printf(LOG_LV_ERROR, "\r\nXCR Write --> [ 0x%02X ] is not OTP address\r\n", u32_recv_param[0]);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xc_rt", "%x", &u32_recv_param[0]))
        {
            uint16_t addr = (uint16_t)u32_recv_param[0];
            if (addr >= XCR_OTP_BASE_ADDR)
            {
                if (true == tim_get_vsync_out_running_flag())
                {
                    tim_set_xc_read_info((uint16_t)u32_recv_param[0], XCR_RW_GRP1);
                }
                else
                {
                    uint16_t xcr = xcr24_read_otp_control(addr - XCR_OTP_BASE_ADDR, 1U);
                    comm_UART_Printf(LOG_LV_INFO, "\r\nXCR OTP Read --> [ 0x%02X - 0x%04X ]\r\n", u32_recv_param[0], xcr);
                    comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
                }
            }
            else
            {
                comm_UART_Printf(LOG_LV_ERROR, "\r\nXCR OTP Read --> [ 0x%02X ] is not OTP address\r\n", u32_recv_param[0]);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(!(strcmp(str_in, "xc_test")))
        {
            gpio_set_xc_vdd_5v(VCC_ON_3V3);
            LL_mDelay(99U);
            xcr24_test_init();
            tim_vsync_out_start();
        }
        else if(!(strcmp(str_in, "xc_r_all")))
        {
            xcr24_read_all();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xc_fll", "%u %u", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (u32_recv_param[0] < 3)
            {
                xcr24_set_fll_cnt((uint8_t)u32_recv_param[0], (uint32_t)u32_recv_param[1]);
            }
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
                float icc = JigBD_IF_Convert_Adc_To_ICC_XD(adc);
                comm_UART_Printf(LOG_LV_INFO, "\r\nxdr icc : %.3f", (double)(icc));
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
        else if(!(strcmp(str_in, "xd_syncgen")))
        {
            xdr12_syncgen();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xd_debug")))
        {
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            LL_mDelay(99U);
            xdr12_init();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "xd_trim_debug")))
        {
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            LL_mDelay(99U);
            xdr12_trim_init();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xd_w", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (true == tim_get_vsync_out_running_flag())
            {
                tim_set_xd_write_info((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], XD12R_ADDR_TYPE_GENERAL);
            }
            else
            {
                xdr12_write_by_type((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], XD12R_ADDR_TYPE_GENERAL);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_okay);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xd_r", "%x", &u32_recv_param[0]))
        {
            if (true == tim_get_vsync_out_running_flag())
            {
                tim_set_xd_read_info((uint16_t)u32_recv_param[0], XD12R_ADDR_TYPE_GENERAL);
            }
            else
            {
                uint16_t xdr = xdr12_read_by_type((uint16_t)u32_recv_param[0], XD12R_ADDR_TYPE_GENERAL);
                comm_UART_Printf(LOG_LV_INFO, "\r\nXDIC Read --> [ 0x%02X - 0x%03X ]\r\n", u32_recv_param[0], xdr);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xd_wt", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (true == tim_get_vsync_out_running_flag())
            {
                tim_set_xd_write_info((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], XD12R_ADDR_TYPE_MIRROR);
            }
            else
            {
                xdr12_write_by_type((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], XD12R_ADDR_TYPE_MIRROR);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xd_rt", "%x", &u32_recv_param[0]))
        {
            if (true == tim_get_vsync_out_running_flag())
            {
                tim_set_xd_read_info((uint16_t)u32_recv_param[0], XD12R_ADDR_TYPE_MIRROR);
            }
            else
            {
                uint16_t xdr = xdr12_read_by_type((uint16_t)u32_recv_param[0], XD12R_ADDR_TYPE_MIRROR);
                comm_UART_Printf(LOG_LV_INFO, "\r\nXDIC Read --> [ 0x%02X - 0x%03X ]\r\n", u32_recv_param[0], xdr);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(!(strcmp(str_in, "xd_r_all")))
        {
            xdr12_read_all();
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(Command_Param_is_("xd_vref", "%u %u", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (u32_recv_param[0] >= XD12R_SETTING_GRP_MAX)
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\nInvalid XDIC channel index (%u). Must be 0 ~ %u", u32_recv_param[0], XD12R_SETTING_GRP_MAX - 1);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
            else
            {
                xdr12_set_max_curr_vref((xd12r_setting_grp_t)u32_recv_param[0], (uint16_t)u32_recv_param[1]);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xd_max_curr_lvl", "%u %u", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (u32_recv_param[0] >= XD12R_SETTING_GRP_MAX)
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\nInvalid XDIC channel index (%u). Must be 0 ~ %u", u32_recv_param[0], XD12R_SETTING_GRP_MAX - 1);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
            else
            {
                xdr12_set_max_curr_lvl((xd12r_setting_grp_t)u32_recv_param[0], (max_curr_level_t)u32_recv_param[1]);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xd_fb", "%u %u", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (u32_recv_param[0] >= XD12R_SETTING_GRP_MAX)
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\nInvalid XDIC channel index (%u). Must be 0 ~ %u", u32_recv_param[0], XD12R_SETTING_GRP_MAX - 1);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
            else
            {
                xdr12_set_fb_lvl((xd12r_setting_grp_t)u32_recv_param[0], (fb_level_t)u32_recv_param[1]);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("xd_short", "%u %u", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (u32_recv_param[0] >= XD12R_SETTING_GRP_MAX)
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\nInvalid XDIC channel index (%u). Must be 0 ~ %u", u32_recv_param[0], XD12R_SETTING_GRP_MAX - 1);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
            else
            {
                xdr12_set_short_lvl((xd12r_setting_grp_t)u32_recv_param[0], (short_level_t)u32_recv_param[1]);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        /************* common **************/
        else if(Command_Param_is_("step", "%u %u %u %u", &u32_recv_param[0], &u32_recv_param[1], &u32_recv_param[2], &u32_recv_param[3]))
        {
            if (u32_recv_param[0] > LDIM_BLK_SIZE)
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\nInvalid LED index (%u). Must be 0 ~ %u", u32_recv_param[0], LDIM_BLK_SIZE);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
            else
            {
                ldim_set_block_color_buffer((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], (uint16_t)u32_recv_param[2], (uint16_t)u32_recv_param[3]);
                if (u32_recv_param[0] == 0)
                {
                    comm_UART_Printf(LOG_LV_INFO, "\r\n\tSet all LED color to R:0x%04X, G:0x%04X, B:0x%04X", u32_recv_param[1], u32_recv_param[2], u32_recv_param[3]);
                }
                else
                {
                    comm_UART_Printf(LOG_LV_INFO, "\r\n\tSet LED[%2u] color to R:0x%04X, G:0x%04X, B:0x%04X", u32_recv_param[0], u32_recv_param[1], u32_recv_param[2], u32_recv_param[3]);
                }
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
            }
        }
        else if(Command_Param_is_("step", "%u %u", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (u32_recv_param[0] > LDIM_BLK_SIZE)
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\nInvalid LED index (%u). Must be 0 ~ %u", u32_recv_param[0], LDIM_BLK_SIZE);
                comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
                return;
            }
            ldim_set_block_color_buffer((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], (uint16_t)u32_recv_param[1], (uint16_t)u32_recv_param[1]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_prompt);
        }
        else if(!(strcmp(str_in, "step")))
        {
            block_color_t* p_block_color_buffer = ldim_get_block_color_buffer();
            for (uint16_t idx = 0U; idx < LDIM_BLK_SIZE; ++idx)
            {
                char log_buf[350] = {0};
                int log_buf_len = 0U;
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "\r\n[LED: %2u] R: %5u, G: %5u, B: %5u", \
                (idx + 1U), p_block_color_buffer[idx].r, p_block_color_buffer[idx].g, p_block_color_buffer[idx].b);
                comm_UART_Printf(LOG_LV_INFO, "%s", log_buf);
            }
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
        else if(Command_Param_is_("log_lv", "%u", &u32_recv_param[0]))
        {
            comm_set_log_lv((LOG_LV_t)u32_recv_param[0]);
            comm_UART_Printf(LOG_LV_INFO, gp_msg_okay);
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
            memcpy(gt_last_uart_rx.buffer, gt_uart.Rxbuff[gt_uart.RxInCnt].buffer, sizeof(gt_uart.Rxbuff[gt_uart.RxInCnt].buffer));
            gt_last_uart_rx.length = gt_uart.Rxbuff[gt_uart.RxInCnt].length;

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
            if((rx_data == CLI_KEY_UP) && (gt_uart.Rxbuff[gt_uart.RxInCnt].length >= 2U) && (gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[gt_uart.Rxbuff[gt_uart.RxInCnt].length - 2] == 0x1BU) && (gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[gt_uart.Rxbuff[gt_uart.RxInCnt].length - 1] == 0x5BU))
            {
                UART_PutChar('\n');
                //UART_PutChar('\r');
                memcpy(gt_uart.Rxbuff[gt_uart.RxInCnt].buffer, gt_last_uart_rx.buffer, sizeof(gt_last_uart_rx.buffer));
                gt_uart.Rxbuff[gt_uart.RxInCnt].length = gt_last_uart_rx.length;
                for (uint8_t i = 0; i < gt_uart.Rxbuff[gt_uart.RxInCnt].length; ++i)
                {
                    UART_PutChar(gt_uart.Rxbuff[gt_uart.RxInCnt].buffer[i]);
                }
            }
            else
            {
                if(gt_uart.Rxbuff[gt_uart.RxInCnt].length < (RX_PACKET_SIZE - 1U))
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
