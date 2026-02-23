/** @file drv_sal_trim.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __DRV_SAL_TRIM_C__

#include "drv_sal_trim.h"
#include "ADS124S08.h"
#include "main.h"
#include <math.h>
#include <string.h>

#ifdef __DRV_SAL_TRIM_C__

#define TRIM_WAFER              (0)
#define TRIM_FULL               (1)
#define TRIM_TYPE               TRIM_FULL

#define LED_EP                  (0)
#define LED_HC                  (1)
#define LED_TYPE                LED_EP

#define DEV_ID                  (0x58 << 1)
#define I2C_TX                  (0x00)
#define I2C_RX                  (0x01)

#define I2C_TX_BUFF_SIZE        (2)
#define I2C_RX_BUFF_SIZE        (2)

#define SAL_OTP_PROTECT_ON      (0x5A5)
#define SAL_OTP_PROTECT_OFF     (0xA5A)

#define CONST_V_to_mV           (1000.0f)
#define V_TRIM_BGR_MUL          (1.0f)
#define I_TRIM_BGR_MUL          (510.0f * 1.0)
#define LDO_CTL_MUL             (1.0f)

#define CONST_MHz_to_KHz        (1000.0f)
#define SAL_INTERNAL_DIVIDE     (16)
#define SAL_JIG_DIVIDE          (64)

#define TIM2_APB1_FREQ          (64) // MHz
#define INPUT_CAPTURE_SIZE      (512)

#define TRIM_LOG_ADC            (0)
#define TRIM_LOG_VALUE          (1)
#define TRIM_LOG_TYPE           TRIM_LOG_ADC

#define IOUT_ERROR_RATE         (0.01f) // 1%

#define LOG_NONE    "\033[0m"
#define LOG_GREEN   "\033[0;32m"
#define LOG_RED     "\033[0;31m"
#define COLOR_BROWN	"\033[0;33m"

extern I2C_HandleTypeDef hi2c1;

static _sal_regs_t gt_sal_regs;
static _sal_trim_regs_t gt_sal_trim_regs;

static trim_step_t gt_trim_step;
static trim_mode_t gt_trim_mode;
static trim_error_t gt_trim_error;

static uint8_t gn_trim_delay;

static float gf_sal_trim_condition[TRIM_MODE_MAX][TRIM_PARAM_MAX] =
{
                    // TARGET_MIN  TARGET_MAX       INPUT1
    /*V_TRIM_BGR*/    {     1.045,      1.155,      0.0}, // 1.2 V
    /*V_R1_R2*/       {      1.14,       1.26,      0.0}, // 1.2 V
    /*I_TRIM_BGR*/    {      0.95,       1.05,      0.0}, // 1 uA
    /*LDO_CTL*/       {     1.425,      1.575,      0.0}, // 1.5 V
    /*OSC_CTL*/       {        16,      16.16,      0.0}, // 16 MHz
    /*LVDS_TX*/       {       0.0,        0.0,      0.0}, // skip
    /*LVDS_RX*/       {       0.0,        0.0,      0.0}, // skip
    /*ADC_GAIN*/      {     263.0,      291.0,      0.0}, // 277 reg value
    /*ADC_OFFSET*/    {     479.0,      530.0,      0.0}, // 505 reg value
    /*TEMP_TRIM_BGR*/ {     495.0,      547.0,      0.0}, // 521 reg value

                      // TARGET_R    TARGET_G   TARGET_B
    /*CURRENT_ITRIM*/ {      22.3,       13.8,     6.24}, // mA
};

static uint16_t gn_sal_trim_reg_range[TRIM_MODE_MAX] =
{
    0x3F, /*V_TRIM_BGR*/
    0xF, /*V_R1_R2*/
    0x1F, /*I_TRIM_BGR*/
    0x7, /*LDO_CTL*/
    0xFF, /*OSC_CTL*/
    0x7, /*LVDS_TX*/
    0x3, /*LVDS_RX*/
    0x1F, /*ADC_GAIN*/
    0x1FF, /*ADC_OFFSET*/
    0x3F, /*TEMP_TRIM_BGR*/
    0xFF, /*CURRENT_ITRIM*/
};

static uint16_t gn_sal_trim_range[TRIM_MODE_MAX][SAL_CH_MAX][TRIM_PARAM_MAX];

static _sal_trim_param_t gt_sal_trim_param[TRIM_MODE_MAX];

static uint16_t gn_sal_trim_temp_adc_value[2];

static uint16_t gn_sal_trim_v_trim_r1_r2[16 * 16];
static uint16_t gn_sal_trim_v_trim_r1_r2_cnt;

static uint16_t gn_sal_trim_ldo[8];
static uint16_t gn_sal_trim_ldo_cnt;

static uint8_t gn_sal_trim_ch;

volatile static uint32_t gn_input_capture_count[INPUT_CAPTURE_SIZE];;
volatile bool gb_input_capture_started;

static otp_verify_step_t gt_otp_verify_step;

static uint8_t gn_v_trim_r1;
static uint8_t gn_v_trim_r2;

static uint8_t gn_sal_osc_trim_val = 128;
static uint16_t gn_sal_adc_offset_trim_val = 256;

static uint8_t gn_adc_ch_table[TRIM_MODE_MAX] =
{
    ADC_CH_TEST_V,      //TRIM_MODE_V_TRIM_BGR
    ADC_CH_TEST_V,      //TRIM_MODE_V_TRIM_R1_R2
    ADC_CH_TEST_V,      //TRIM_MODE_I_TRIM_BGR
    ADC_CH_TEST_V,      //TRIM_MODE_LDO_CTL
    ADC_CH_NONE,        //TRIM_MODE_OSC_CTL
    ADC_CH_LVDS_TX,     //TRIM_MODE_LVDS_TX
    ADC_CH_LVDS_RX,     //TRIM_MODE_LVDS_RX
    ADC_CH_NONE,        //TRIM_MODE_ADC_GAIN
    ADC_CH_NONE,        //TRIM_MODE_ADC_OFFSET
    ADC_CH_NONE,        //TRIM_MODE_TEMP_TRIM_BGR
    ADC_CH_SAL_IOUT,    //TRIM_MODE_CURRENT_ITRIM
};

static const char* gs_sal_trim_addr_str[SAL_TRIM_ADDR_MAX] =
{
    "TRIM_CONTROL",
    "OTP_CONTROL1",
    "OTP_CONTROL2",
    "OTP_CONTROL3",
    "OTP1_MIRROR_1",
    "OTP1_MIRROR_2",
    "OTP1_MIRROR_3",
    "OTP1_MIRROR_4",
    "OTP1_MIRROR_5",
    "OTP1_MIRROR_6",
    "OTP1_MIRROR_7",
    "OTP1_MIRROR_8",
    "OTP1_MIRROR_9",
    "OTP1_MIRROR_10",
    "OTP1_MIRROR_11",
    "OTP1_MIRROR_12",
    "OTP1_MIRROR_13",
    "OTP2_MIRROR_14",
    "OTP2_MIRROR_15",
    "OTP2_MIRROR_16",
    "OTP2_MIRROR_17",
    "OTP2_MIRROR_18",
    "OTP2_MIRROR_19",
};

static const char* gs_sal_addr_str[SAL_ADDR_MAX] =
{
    "DUMMY",
    "STATUS1",
    "STATUS2",
    "TEMP",
    "DUMMY",
    "VEXT_TM",
    "SETUP1",
    "SETUP2",
    "MCAST1",
    "MCAST2",
    "TEMPTH",
    "TEMPHYS",
    "CALC_PWM_RED",
    "CALC_PWM_GREEN",
    "CALC_PWM_BLUE",
    "CURR_MAX_LVL",
    "TEMP_LUT_TC1",
    "TEMP_LUT_TC2",
    "TEMP_LUT_TC3",
    "TEMP_LUT_TC4",
    "TEMP_LUT_TC5",
    "TEMP_LUT_TC6",
    "TEMP_LUT_TC7",
    "TEMP_LUT_TC8",
    "TEMP_LUT_TC9",
    "TEMP_LUT_TC10",
    "TIMEOUT",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "RGB1",
    "RGB2",
};

static const char* gs_sal_trim_mode_str[TRIM_MODE_MAX] =
{
    "TRIM_MODE_V_TRIM_BGR",
    "TRIM_MODE_V_TRIM_R1_R2",
    "TRIM_MODE_I_TRIM_BGR",
    "TRIM_MODE_LDO_CTL",
    "TRIM_MODE_OSC_CTL",
    "TRIM_MODE_LVDS_TX",
    "TRIM_MODE_LVDS_RX",
    "TRIM_MODE_ADC_GAIN",
    "TRIM_MODE_ADC_OFFSET",
    "TRIM_MODE_TEMP_TRIM_BGR",
    "TRIM_MODE_CURRENT_ITRIM",
};

static const char* gs_sal_trim_adj_flag_str[TRIM_ADJ_FLAG_MAX] =
{
    "TRIM_ADJ_FLAG_NONE",
    "TRIM_ADJ_FLAG_UP",
    "TRIM_ADJ_FLAG_DW",
    "TRIM_ADJ_FLAG_DONE",
    "TRIM_ADJ_FLAG_ERR",
};

static const char* gs_sal_trim_otp_error_str[TRIM_ERROR_MAX] =
{
    "TRIM_ERROR_NONE",
    "TRIM_ERROR_COMMUNICATION",
    "TRIM_ERROR_OVERRUN",
    "TRIM_ERROR_OTP_VERIFY",
    "TRIM_ERROR_REG_OVER_RANGE",
};

static const char* gs_sal_ic_status_str[5] =
{
    "IC_STATUS_POR",
    "IC_STATUS_UNINIT",
    "IC_STATUS_SLEEP",
    "IC_STATUS_ACTIVE",
    "IC_STATUS_DEEPSLEEP",
};

static const char* gs_sal_sio_status_str[4] =
{
    "SIO_STATUS_LVDS",
    "SIO_STATUS_EOL",
    "SIO_STATUS_MCU",
    "SIO_STATUS_CAN",
};

static const char* gs_sal_tmux_sel_str[16] =
{
    "TMUX_SEL_0",
    "TMUX_SEL_UV",
    "TMUX_SEL_OPEN_R",
    "TMUX_SEL_OPEN_G",
    "TMUX_SEL_OPEN_B",
    "TMUX_SEL_SHORT_R",
    "TMUX_SEL_SHORT_G",
    "TMUX_SEL_SHORT_B",
    "TMUX_SEL_PWM_R_OUT_EN",
    "TMUX_SEL_PWM_G_OUT_EN",
    "TMUX_SEL_PWM_B_OUT_EN",
    "TMUX_SEL_SIO1_LVDS_TH",
    "TMUX_SEL_SIO2_LVDS_TH",
    "TMUX_SEL_MCLK",
    "TMUX_SEL_OPEN_R_FLAG",
    "TMUX_SEL_SHORT_R_FLAG",
};

static void sal_trim_regs_init(void);
static void sal_trim_reg_write(uint8_t ch, trim_mode_t trim_mode, uint16_t n_value);

static uint32_t sal_trim_reg_verify(void);

static void clear_trim_parameters(void);
static void display_trim_info(trim_mode_t n_trim_mode);
static void display_r1_r2_info(void);

static uint16_t convert_frequency_to_count(float n_MHz);

static float convert_value_to_parameter(trim_mode_t trim_mode, uint16_t n_value);

static void change_trim_step(trim_step_t trim_step);

static void sal_trim_regs_init(void)
{
    _sal_i2c_info_t i2c_info = {0, };

    print(LOG_LV_INFO, "------------------------------------\n\r");
    print(LOG_LV_INFO, "----------- TRIM REG INIT ----------\r\n");

    for (sal_trim_addr_t addr = SAL_TRIM_ADDR_TRIM_CONTROL ; addr < SAL_TRIM_ADDR_MAX ; ++addr)
    {
        switch (addr)
        {
            case SAL_TRIM_ADDR_TRIM_CONTROL :
                #if 0
                    gt_sal_trim_regs._rE0.pwm_max_b_e = 0;
                    gt_sal_trim_regs._rE0.pwm_max_g_e = 0;
                    gt_sal_trim_regs._rE0.pwm_max_r_e = 0;
                    gt_sal_trim_regs._rE0.t_ana_sel = 0;
                    gt_sal_trim_regs._rE0.ana_mod_sel = 0;
                    gt_sal_trim_regs._rE0.adc_clk_pol = 0;
                    gt_sal_trim_regs._rE0.cko_e = 0;
                    gt_sal_trim_regs._rE0.test_e = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP_CONTROL1 :
                #if 0
                    gt_sal_trim_regs._rE1.otp_pg_acc_cycle = 0;
                    gt_sal_trim_regs._rE1.otp_wsel = 0;
                    gt_sal_trim_regs._rE1.otp_rd = 0;
                    gt_sal_trim_regs._rE1.t_ana_e = 0;
                    gt_sal_trim_regs._rE1.otp_pg1 = 0;
                    gt_sal_trim_regs._rE1.otp_pg2 = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP_CONTROL2 :
                #if 0
                    gt_sal_trim_regs._rE2.otp_pg_acc_cycle = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP_CONTROL3 :
                #if 0
                    gt_sal_trim_regs._rE3.protect = 0x5A5;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR1 :
                #if 0
                    gt_sal_trim_regs._rE4.v_trim_bgr = 0;
                    gt_sal_trim_regs._rE4.sio2_tr_tx_bias = 0;
                    gt_sal_trim_regs._rE4.sio1_tr_tx_bias = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR2 :
                #if 0
                    gt_sal_trim_regs._rE5.i_trim_bgr = 0;
                    gt_sal_trim_regs._rE5.vldo_ctrl = 0;
                    gt_sal_trim_regs._rE5.sio2_tr_res = 0;
                    gt_sal_trim_regs._rE5.sio1_tr_res = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR3 :
                #if 0
                    gt_sal_trim_regs._rE6.bgr_r1_trim = 0;
                    gt_sal_trim_regs._rE6.temp_trim_bgr = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR4 :
                #if 0
                    gt_sal_trim_regs._rE7.bgr_r2_trim = 0;
                    gt_sal_trim_regs._rE7.osc_ctl = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR5 :
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR6 :
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR7 :
                #if 0
                    gt_sal_trim_regs._rEA.adc_offset = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR8 :
                #if 0
                    gt_sal_trim_regs._rEB.adc_gain = 0x10;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR9 :
                #if 0
                    gt_sal_trim_regs._rEC.itrim_r = 0x80;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR10 :
                #if 0
                    gt_sal_trim_regs._rED.itrim_g = 0x80;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR11 :
                #if 0
                    gt_sal_trim_regs._rEE.itrim_b = 0x80;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR12 :
                #if 0
                    gt_sal_trim_regs._rEF.tc_base = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP1_MIRROR13 :
                #if 0
                    gt_sal_trim_regs._rF0.tc_offset = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP2_MIRROR14 :
                #if 0
                    gt_sal_trim_regs._rF1.pwm_max_r = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP2_MIRROR15 :
                #if 0
                    gt_sal_trim_regs._rF2.pwm_max_g = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP2_MIRROR16 :
                #if 0
                    gt_sal_trim_regs._rF3.pwm_max_b = 0;
                #endif
                break;
            case SAL_TRIM_ADDR_OTP2_MIRROR17 :
                break;
            case SAL_TRIM_ADDR_OTP2_MIRROR18 :
                break;
            case SAL_TRIM_ADDR_OTP2_MIRROR19 :
                break;
            default:
                continue;
        }

        i2c_info.reg_addr = addr;
        i2c_info.data = gt_sal_trim_regs.ALL[addr - SAL_TRIM_ADDR_OFFSET];

        print(LOG_LV_INFO, "[%s] [0x%02X] : 0x%03X\r\n", gs_sal_trim_addr_str[(uint8_t)(addr - SAL_TRIM_ADDR_OFFSET)], addr, gt_sal_trim_regs.ALL[addr - SAL_TRIM_ADDR_OFFSET]);
        sal_write_reg_i2c(&i2c_info);
    }
    print(LOG_LV_INFO, "------------------------------------\n\r");
}

void sal_write_reg_i2c(_sal_i2c_info_t *p_info)
{
    uint16_t data = p_info->data;
    uint8_t reg_addr = p_info->reg_addr;
    uint8_t i2c_tx_buff[I2C_TX_BUFF_SIZE] = {0, };

    i2c_tx_buff[0] = (data >> 8);
    i2c_tx_buff[1] = (data >> 0);

    if (HAL_OK != HAL_I2C_Mem_Write(&hi2c1, DEV_ID/*(DEV_ID << 1 | I2C_TX)*/, reg_addr, 1, i2c_tx_buff, I2C_TX_BUFF_SIZE, 100))
    {
        gt_trim_error = TRIM_ERROR_COMMUNICATION;
        gt_trim_step = TRIM_STEP_ERROR;
        print(LOG_LV_ERROR, "I2C TX ERROR [0x%02X, 0x%04X]\r\n", reg_addr, data);
    }
    else
    {
        if (reg_addr < SAL_ADDR_MAX)
        {
            gt_sal_regs.ALL[reg_addr] = data;
            //print(LOG_LV_DEBUG, "SAL WRITE REG [0x%02X] - [0x%04X]\r\n", reg_addr, gt_sal_regs.ALL[reg_addr]);
        }
        else
        {
            gt_sal_trim_regs.ALL[reg_addr - SAL_TRIM_ADDR_OFFSET] = data;
            //print(LOG_LV_DEBUG, "SAL WRITE TRIM REG [0x%02X] - [0x%04X]\r\n", reg_addr, gt_sal_trim_regs.ALL[reg_addr - SAL_TRIM_ADDR_OFFSET]);
        }
    }
}

uint16_t sal_read_reg_i2c(_sal_i2c_info_t *p_info)
{
    uint8_t i2c_rx_buff[I2C_RX_BUFF_SIZE] = {0, };
    uint16_t ret = SAL_RTN_VAL_I2C_RX_ERROR;

    uint8_t reg_addr = p_info->reg_addr;

    HAL_StatusTypeDef i2c_status = HAL_ERROR;

    i2c_status = HAL_I2C_Master_Transmit(&hi2c1, DEV_ID, &reg_addr, 1, 100);
    i2c_status = HAL_I2C_Master_Receive(&hi2c1, DEV_ID, i2c_rx_buff, 2, 100);

    if (i2c_status != HAL_OK)
    {
        gt_trim_error = TRIM_ERROR_COMMUNICATION;
        gt_trim_step = TRIM_STEP_ERROR;
        print(LOG_LV_ERROR, "I2C RX ERROR [0x%02X]\r\n", reg_addr);
    }
    else
    {
        ret = (i2c_rx_buff[0] << 8 | i2c_rx_buff[1]);
        if (reg_addr < SAL_ADDR_MAX)
        {
            _sal_regs_t* p_sal_regs = sal_get_regs();
            p_sal_regs->ALL[reg_addr] = ret;
            print(LOG_LV_DEBUG, "SAL READ REG [0x%02X] - [0x%04X (%5u)]\r\n", reg_addr, p_sal_regs->ALL[reg_addr], p_sal_regs->ALL[reg_addr]);
        }
        else
        {
            _sal_trim_regs_t* p_sal_trim_regs = sal_get_trim_regs();
            p_sal_trim_regs->ALL[reg_addr - SAL_TRIM_ADDR_OFFSET] = ret;
            print(LOG_LV_DEBUG, "SAL READ TRIM REG [0x%02X] - [0x%04X (%5u)]\r\n", reg_addr, p_sal_trim_regs->ALL[reg_addr - SAL_TRIM_ADDR_OFFSET], p_sal_trim_regs->ALL[reg_addr - SAL_TRIM_ADDR_OFFSET]);
        }
    }
    return ret;
}

_sal_regs_t* sal_get_regs(void)
{
    return &gt_sal_regs;
}

_sal_trim_regs_t* sal_get_trim_regs(void)
{
    return &gt_sal_trim_regs;
}

const char* sal_get_regs_str(uint8_t reg_addr)
{
    return gs_sal_addr_str[reg_addr];
}

const char* sal_get_trim_regs_str(uint8_t reg_addr)
{
    return gs_sal_trim_addr_str[reg_addr];
}

static void sal_trim_reg_write(uint8_t ch, trim_mode_t trim_mode, uint16_t n_value)
{
    _sal_i2c_info_t i2c_info = {0, };
    uint8_t reg_write_flag = 0;

    if (n_value <= gn_sal_trim_reg_range[trim_mode])
    {
        reg_write_flag = 1;
    }

    switch (trim_mode)
    {
    case TRIM_MODE_V_TRIM_BGR :
        gt_sal_trim_regs._rE4.v_trim_bgr = n_value;
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR1;
        i2c_info.data = gt_sal_trim_regs._rE4.val;
        break;
    case TRIM_MODE_V_TRIM_R1_R2 :
        if (ch == 0)
        {
            gt_sal_trim_regs._rE6.bgr_r1_trim = n_value;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR3;
            i2c_info.data = gt_sal_trim_regs._rE6.val;
        }
        else
        {
            gt_sal_trim_regs._rE7.bgr_r2_trim = n_value;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR4;
            i2c_info.data = gt_sal_trim_regs._rE7.val;
        }
        break;
    case TRIM_MODE_I_TRIM_BGR :
        gt_sal_trim_regs._rE5.i_trim_bgr = n_value;
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR2;
        i2c_info.data = gt_sal_trim_regs._rE5.val;
        break;
    case TRIM_MODE_LDO_CTL :
        gt_sal_trim_regs._rE5.vldo_ctrl = n_value;
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR2;
        i2c_info.data = gt_sal_trim_regs._rE5.val;
        break;
    case TRIM_MODE_OSC_CTL :
        if (n_value < 128)
        {
            // bit invert lsb 7-bits
            n_value ^= 127;
        }
        gt_sal_trim_regs._rE7.osc_ctl = (n_value & 0x7F);
        gt_sal_trim_regs._rE7.osc_ctl_sign = ((n_value & 0x80) >> 7);
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR4;
        i2c_info.data = gt_sal_trim_regs._rE7.val;
        break;
    case TRIM_MODE_LVDS_TX :
        if (ch == 0)
        {
            gt_sal_trim_regs._rE4.sio1_tr_tx_bias = n_value;
        }
        else
        {
            gt_sal_trim_regs._rE4.sio2_tr_tx_bias = n_value;
        }
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR1;
        i2c_info.data = gt_sal_trim_regs._rE4.val;
        break;
    case TRIM_MODE_LVDS_RX :
        if (ch == 0)
        {
            gt_sal_trim_regs._rE5.sio1_tr_res = n_value;
        }
        else
        {
            gt_sal_trim_regs._rE5.sio2_tr_res = n_value;
        }
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR2;
        i2c_info.data = gt_sal_trim_regs._rE5.val;
        break;
    case TRIM_MODE_ADC_GAIN :
        gt_sal_trim_regs._rEB.adc_gain = n_value;
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR8;
        i2c_info.data = gt_sal_trim_regs._rEB.val;
        break;
    case TRIM_MODE_ADC_OFFSET :
        if (n_value < 256)
        {
            // bit invert lsb 8-bits
            n_value ^= 255;
        }
        gt_sal_trim_regs._rEA.adc_offset = n_value;
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR7;
        i2c_info.data = gt_sal_trim_regs._rEA.val;
        break;
    case TRIM_MODE_TEMP_TRIM_BGR :
        gt_sal_trim_regs._rE6.temp_trim_bgr = n_value;
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR3;
        i2c_info.data = gt_sal_trim_regs._rE6.val;
        break;
    case TRIM_MODE_CURRENT_ITRIM :
        if (ch == 0)
        {
            gt_sal_trim_regs._rEC.itrim_r = n_value;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR9;
            i2c_info.data = gt_sal_trim_regs._rEC.val;
        }
        else if (ch == 1)
        {
            gt_sal_trim_regs._rED.itrim_g = n_value;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR10;
            i2c_info.data = gt_sal_trim_regs._rED.val;
        }
        else
        {
            gt_sal_trim_regs._rEE.itrim_b = n_value;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR11;
            i2c_info.data = gt_sal_trim_regs._rEE.val;
        }
        break;
    default :
        break;
    }

    if (reg_write_flag)
    {
        sal_write_reg_i2c(&i2c_info);
    }
    else
    {
        gt_trim_error = TRIM_ERROR_REG_OVER_RANGE;
    }
}

void sal_calculate_trim_range(void)
{
    print(LOG_LV_DEBUG, "\r\n");
    for (trim_mode_t trim_mode = TRIM_MODE_V_TRIM_BGR ; trim_mode < TRIM_MODE_MAX ; ++trim_mode)
    {
        for (uint8_t sal_ch = 0 ; sal_ch < SAL_CH_MAX ; ++sal_ch)
        {
            switch (trim_mode)
            {
            case TRIM_MODE_V_TRIM_BGR :
                gn_sal_trim_range[TRIM_MODE_V_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MIN] = convert_voltage_to_adc(gf_sal_trim_condition[TRIM_MODE_V_TRIM_BGR][TRIM_PARAM_TARGET_MIN] * CONST_V_to_mV * V_TRIM_BGR_MUL);
                gn_sal_trim_range[TRIM_MODE_V_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MAX] = convert_voltage_to_adc(gf_sal_trim_condition[TRIM_MODE_V_TRIM_BGR][TRIM_PARAM_TARGET_MAX] * CONST_V_to_mV * V_TRIM_BGR_MUL);
                gn_sal_trim_range[TRIM_MODE_V_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET] = (uint16_t)((gn_sal_trim_range[TRIM_MODE_V_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MIN] + gn_sal_trim_range[TRIM_MODE_V_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MAX]) / 2 + 0.5f);
                break;
            case TRIM_MODE_V_TRIM_R1_R2 :
                gn_sal_trim_range[TRIM_MODE_V_TRIM_R1_R2][sal_ch][TRIM_PARAM_TARGET_MIN] = convert_voltage_to_adc(gf_sal_trim_condition[TRIM_MODE_V_TRIM_R1_R2][TRIM_PARAM_TARGET_MIN] * CONST_V_to_mV * V_TRIM_BGR_MUL);
                gn_sal_trim_range[TRIM_MODE_V_TRIM_R1_R2][sal_ch][TRIM_PARAM_TARGET_MAX] = convert_voltage_to_adc(gf_sal_trim_condition[TRIM_MODE_V_TRIM_R1_R2][TRIM_PARAM_TARGET_MAX] * CONST_V_to_mV * V_TRIM_BGR_MUL);
                gn_sal_trim_range[TRIM_MODE_V_TRIM_R1_R2][sal_ch][TRIM_PARAM_TARGET] = (uint16_t)((gn_sal_trim_range[TRIM_MODE_V_TRIM_R1_R2][sal_ch][TRIM_PARAM_TARGET_MIN] + gn_sal_trim_range[TRIM_MODE_V_TRIM_R1_R2][sal_ch][TRIM_PARAM_TARGET_MAX]) / 2 + 0.5f);
                break;
            case TRIM_MODE_I_TRIM_BGR :
                gn_sal_trim_range[TRIM_MODE_I_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MIN] = convert_voltage_to_adc(gf_sal_trim_condition[TRIM_MODE_I_TRIM_BGR][TRIM_PARAM_TARGET_MIN] * I_TRIM_BGR_MUL);
                gn_sal_trim_range[TRIM_MODE_I_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MAX] = convert_voltage_to_adc(gf_sal_trim_condition[TRIM_MODE_I_TRIM_BGR][TRIM_PARAM_TARGET_MAX] * I_TRIM_BGR_MUL);
                gn_sal_trim_range[TRIM_MODE_I_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET] = (uint16_t)((gn_sal_trim_range[TRIM_MODE_I_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MIN] + gn_sal_trim_range[TRIM_MODE_I_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MAX]) / 2 + 0.5f);
                break;
            case TRIM_MODE_LDO_CTL :
                gn_sal_trim_range[TRIM_MODE_LDO_CTL][sal_ch][TRIM_PARAM_TARGET_MIN] = convert_voltage_to_adc(gf_sal_trim_condition[TRIM_MODE_LDO_CTL][TRIM_PARAM_TARGET_MIN] * CONST_V_to_mV * LDO_CTL_MUL);
                gn_sal_trim_range[TRIM_MODE_LDO_CTL][sal_ch][TRIM_PARAM_TARGET_MAX] = convert_voltage_to_adc(gf_sal_trim_condition[TRIM_MODE_LDO_CTL][TRIM_PARAM_TARGET_MAX] * CONST_V_to_mV * LDO_CTL_MUL);
                gn_sal_trim_range[TRIM_MODE_LDO_CTL][sal_ch][TRIM_PARAM_TARGET] = (uint16_t)((gn_sal_trim_range[TRIM_MODE_LDO_CTL][sal_ch][TRIM_PARAM_TARGET_MIN] + gn_sal_trim_range[TRIM_MODE_LDO_CTL][sal_ch][TRIM_PARAM_TARGET_MAX]) / 2 + 0.5f);
                break;
            case TRIM_MODE_OSC_CTL :
                gn_sal_trim_range[TRIM_MODE_OSC_CTL][sal_ch][TRIM_PARAM_TARGET_MIN] = convert_frequency_to_count(gf_sal_trim_condition[TRIM_MODE_OSC_CTL][TRIM_PARAM_TARGET_MAX]);
                gn_sal_trim_range[TRIM_MODE_OSC_CTL][sal_ch][TRIM_PARAM_TARGET_MAX] = convert_frequency_to_count(gf_sal_trim_condition[TRIM_MODE_OSC_CTL][TRIM_PARAM_TARGET_MIN]);
                gn_sal_trim_range[TRIM_MODE_OSC_CTL][sal_ch][TRIM_PARAM_TARGET] = gn_sal_trim_range[TRIM_MODE_OSC_CTL][sal_ch][TRIM_PARAM_TARGET_MAX];
                break;
            case TRIM_MODE_ADC_GAIN :
                gn_sal_trim_range[TRIM_MODE_ADC_GAIN][sal_ch][TRIM_PARAM_TARGET_MIN] = (uint16_t)(gf_sal_trim_condition[TRIM_MODE_ADC_GAIN][TRIM_PARAM_TARGET_MIN]);
                gn_sal_trim_range[TRIM_MODE_ADC_GAIN][sal_ch][TRIM_PARAM_TARGET_MAX] = (uint16_t)(gf_sal_trim_condition[TRIM_MODE_ADC_GAIN][TRIM_PARAM_TARGET_MAX]);
                gn_sal_trim_range[TRIM_MODE_ADC_GAIN][sal_ch][TRIM_PARAM_TARGET] = (uint16_t)((gn_sal_trim_range[TRIM_MODE_ADC_GAIN][sal_ch][TRIM_PARAM_TARGET_MIN] + gn_sal_trim_range[TRIM_MODE_ADC_GAIN][sal_ch][TRIM_PARAM_TARGET_MAX]) / 2 + 0.5f);
                break;
            case TRIM_MODE_ADC_OFFSET :
                gn_sal_trim_range[TRIM_MODE_ADC_OFFSET][sal_ch][TRIM_PARAM_TARGET_MIN] = (uint16_t)(gf_sal_trim_condition[TRIM_MODE_ADC_OFFSET][TRIM_PARAM_TARGET_MIN]);
                gn_sal_trim_range[TRIM_MODE_ADC_OFFSET][sal_ch][TRIM_PARAM_TARGET_MAX] = (uint16_t)(gf_sal_trim_condition[TRIM_MODE_ADC_OFFSET][TRIM_PARAM_TARGET_MAX]);
                gn_sal_trim_range[TRIM_MODE_ADC_OFFSET][sal_ch][TRIM_PARAM_TARGET] = (uint16_t)((gn_sal_trim_range[TRIM_MODE_ADC_OFFSET][sal_ch][TRIM_PARAM_TARGET_MIN] + gn_sal_trim_range[TRIM_MODE_ADC_OFFSET][sal_ch][TRIM_PARAM_TARGET_MAX]) / 2 + 0.5f);
                break;
            case TRIM_MODE_TEMP_TRIM_BGR :
                gn_sal_trim_range[TRIM_MODE_TEMP_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MIN] = (uint16_t)(gf_sal_trim_condition[TRIM_MODE_TEMP_TRIM_BGR][TRIM_PARAM_TARGET_MIN]);
                gn_sal_trim_range[TRIM_MODE_TEMP_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MAX] = (uint16_t)(gf_sal_trim_condition[TRIM_MODE_TEMP_TRIM_BGR][TRIM_PARAM_TARGET_MAX]);
                gn_sal_trim_range[TRIM_MODE_TEMP_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET] = (uint16_t)((gn_sal_trim_range[TRIM_MODE_TEMP_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MIN] + gn_sal_trim_range[TRIM_MODE_TEMP_TRIM_BGR][sal_ch][TRIM_PARAM_TARGET_MAX]) / 2 + 0.5f);
                break;
            case TRIM_MODE_CURRENT_ITRIM :
                if (sal_ch == 0)
                {
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET_MIN] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_R] * (1 - IOUT_ERROR_RATE), GAIN_HIGH);
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET_MAX] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_R] * (1 + IOUT_ERROR_RATE), GAIN_HIGH);
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_R], GAIN_HIGH);
                }
                else if (sal_ch == 1)
                {
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET_MIN] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_G] * (1 - IOUT_ERROR_RATE), GAIN_HIGH);
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET_MAX] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_G] * (1 + IOUT_ERROR_RATE), GAIN_HIGH);
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_G], GAIN_HIGH);
                }
                else
                {
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET_MIN] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_B] * (1 - IOUT_ERROR_RATE), GAIN_HIGH);
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET_MAX] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_B] * (1 + IOUT_ERROR_RATE), GAIN_HIGH);
                    gn_sal_trim_range[TRIM_MODE_CURRENT_ITRIM][sal_ch][TRIM_PARAM_TARGET] = convert_current_to_adc(gf_sal_trim_condition[TRIM_MODE_CURRENT_ITRIM][TRIM_PARAM_OUT_B], GAIN_HIGH);
                }
                break;
            default:
                break;
            }
        }
        if (trim_mode < TRIM_MODE_CURRENT_ITRIM)
        {
            print(LOG_LV_DEBUG, "[%s] - [min/max/target] - [%5u / %5u / %5u]\r\n\r\n", gs_sal_trim_mode_str[trim_mode], \
            gn_sal_trim_range[trim_mode][0][TRIM_PARAM_TARGET_MIN], gn_sal_trim_range[trim_mode][0][TRIM_PARAM_TARGET_MAX], gn_sal_trim_range[trim_mode][0][TRIM_PARAM_TARGET]);
        }
        else
        {
            print(LOG_LV_DEBUG, "[%s] - [R] [min/max/target] - [%5u / %5u / %5u]\r\n\r\n", gs_sal_trim_mode_str[trim_mode], \
            gn_sal_trim_range[trim_mode][0][TRIM_PARAM_TARGET_MIN], gn_sal_trim_range[trim_mode][0][TRIM_PARAM_TARGET_MAX], gn_sal_trim_range[trim_mode][0][TRIM_PARAM_TARGET]);
            print(LOG_LV_DEBUG, "[%s] - [G] [min/max/target] - [%5u / %5u / %5u]\r\n\r\n", gs_sal_trim_mode_str[trim_mode], \
            gn_sal_trim_range[trim_mode][1][TRIM_PARAM_TARGET_MIN], gn_sal_trim_range[trim_mode][1][TRIM_PARAM_TARGET_MAX], gn_sal_trim_range[trim_mode][1][TRIM_PARAM_TARGET]);
            print(LOG_LV_DEBUG, "[%s] - [B] [min/max/target] - [%5u / %5u / %5u]\r\n\r\n", gs_sal_trim_mode_str[trim_mode], \
            gn_sal_trim_range[trim_mode][2][TRIM_PARAM_TARGET_MIN], gn_sal_trim_range[trim_mode][2][TRIM_PARAM_TARGET_MAX], gn_sal_trim_range[trim_mode][2][TRIM_PARAM_TARGET]);
        }
    }
}

// static void sal_trim_otp_protection(otp_protect_t n_protect)
void sal_trim_otp_protection(otp_protect_t n_protect)
{
    _sal_i2c_info_t i2c_info = {0, };
    if (n_protect < OTP_PROTECT_MAX)
    {
        if (n_protect == OTP_PROTECT_ON)
        {
            gt_sal_trim_regs._rE3.protect = SAL_OTP_PROTECT_ON;
        }
        else if (n_protect == OTP_PROTECT_OFF)
        {
            gt_sal_trim_regs._rE3.protect = SAL_OTP_PROTECT_OFF;
        }
        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP_CONTROL3;
        i2c_info.data = gt_sal_trim_regs._rE3.val;
        sal_write_reg_i2c(&i2c_info);
    }
    else
    {
        print(LOG_LV_ERROR, "Invalid Input [%u] - [0 ~ 1]\r\n", n_protect);
    }
}

// static void sal_trim_read_all_registers(void)
void sal_trim_read_all_registers(void)
{
    _sal_i2c_info_t i2c_info = {0, };
    print(LOG_LV_INFO, "------------------------------------------\n\r");
    print(LOG_LV_INFO, "-------------- SAL REGISTER --------------\r\n");
    for (sal_regs_addr_t sal_reg_addr = SAL_ADDR_STATUS1 ; sal_reg_addr < SAL_ADDR_MAX ; ++sal_reg_addr)
    {
        i2c_info.reg_addr = sal_reg_addr;
        gt_sal_regs.ALL[sal_reg_addr] = sal_read_reg_i2c(&i2c_info);

        print(LOG_LV_INFO, "%014s [0x%02X] : 0x%03X\r\n", sal_get_regs_str((uint8_t)(sal_reg_addr)), sal_reg_addr, gt_sal_regs.ALL[sal_reg_addr]);
    }

    print(LOG_LV_INFO, "-------------------------------------------\n\r");
    print(LOG_LV_INFO, "------------ SAL TRIM REGISTER ------------\r\n");
    for (sal_trim_addr_t sal_trim_reg_addr = SAL_TRIM_ADDR_TRIM_CONTROL ; sal_trim_reg_addr < SAL_TRIM_ADDR_MAX ; ++sal_trim_reg_addr)
    {
        i2c_info.reg_addr = sal_trim_reg_addr;
        gt_sal_trim_regs.ALL[sal_trim_reg_addr - SAL_TRIM_ADDR_OFFSET] = sal_read_reg_i2c(&i2c_info);

        print(LOG_LV_INFO, "%014s [0x%02X] : 0x%03X\r\n", sal_get_trim_regs_str((uint8_t)(sal_trim_reg_addr - SAL_TRIM_ADDR_OFFSET)), sal_trim_reg_addr, gt_sal_trim_regs.ALL[sal_trim_reg_addr - SAL_TRIM_ADDR_OFFSET]);
    }
    print(LOG_LV_INFO, "--------------------------------------------\n\r");
}

static uint32_t sal_trim_reg_verify(void)
{
    uint32_t ret = 0;
    _sal_i2c_info_t i2c_info = {0, };
    print(LOG_LV_INFO, "-------------------------------------\n\r");
    print(LOG_LV_INFO, "----- SAL TRIM REGISTER VERIFY  -----\r\n");
    for (sal_trim_addr_t sal_trim_reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR1 ; sal_trim_reg_addr < SAL_TRIM_ADDR_OTP2_MIRROR17 ; ++sal_trim_reg_addr) //0xE4 ~ 0xF3
    {
        i2c_info.reg_addr = sal_trim_reg_addr;
        i2c_info.data = sal_read_reg_i2c(&i2c_info);

        if (gt_sal_trim_regs.ALL[i2c_info.reg_addr - SAL_TRIM_ADDR_OFFSET] != i2c_info.data)
        {
            ret |= (1 << (uint32_t)(sal_trim_reg_addr - SAL_TRIM_ADDR_OFFSET));
            print(LOG_LV_INFO, "%s %14s [0x%02X] - OTP FAIL [0x%3X / 0x%3X] %s\r\n", LOG_RED, gs_sal_trim_addr_str[i2c_info.reg_addr - SAL_TRIM_ADDR_OFFSET], sal_trim_reg_addr, gt_sal_trim_regs.ALL[i2c_info.reg_addr - SAL_TRIM_ADDR_OFFSET], i2c_info.data, LOG_NONE);
        }
        else
        {
            print(LOG_LV_INFO, "%s %14s [0x%02X] - OTP PASS [0x%3X / 0x%3X]%s \r\n", LOG_GREEN, gs_sal_trim_addr_str[i2c_info.reg_addr - SAL_TRIM_ADDR_OFFSET], sal_trim_reg_addr, gt_sal_trim_regs.ALL[i2c_info.reg_addr - SAL_TRIM_ADDR_OFFSET], i2c_info.data, LOG_NONE);
        }
        gt_sal_trim_regs.ALL[i2c_info.reg_addr - SAL_TRIM_ADDR_OFFSET] = i2c_info.data;
    }
    if (!ret)
    {
        print(LOG_LV_INFO, "%s ----- OTP Verify Result : OK (0x%04X) ----- %s \n\r", LOG_GREEN, ret, LOG_NONE);
    }
    else
    {
        print(LOG_LV_INFO, "%s ----- OTP Verify Result : NG (0x%04X) ----- %s \n\r", LOG_RED, ret, LOG_NONE);
    }
    print(LOG_LV_INFO, "-------------------------------------------\n\r");

    return ret;
}

//static void sal_trim_test_enable(void)
void sal_trim_test_enable(void)
{
    _sal_i2c_info_t i2c_info = {0, };
    gt_sal_trim_regs._rE0.test_e = 1;

    i2c_info.reg_addr = SAL_TRIM_ADDR_TRIM_CONTROL;
    i2c_info.data = gt_sal_trim_regs._rE0.val;

    sal_write_reg_i2c(&i2c_info);
}

// static void sal_trim_t_ana_sel(trim_mode_t n_trim_mode)
void sal_trim_t_ana_sel(trim_mode_t n_trim_mode)
{
    _sal_i2c_info_t i2c_info = {0, };
    switch (n_trim_mode)
    {
    case TRIM_MODE_V_TRIM_BGR :
        gt_sal_trim_regs._rE0.t_ana_sel = 3;
        break;
    case TRIM_MODE_V_TRIM_R1_R2 :
        gt_sal_trim_regs._rE0.t_ana_sel = 3;
        break;
    case TRIM_MODE_I_TRIM_BGR :
        gt_sal_trim_regs._rE0.t_ana_sel = 5;
        break;
    case TRIM_MODE_LDO_CTL :
        gt_sal_trim_regs._rE0.t_ana_sel = 4;
        break;
    case TRIM_MODE_OSC_CTL :
        gt_sal_trim_regs._rE0.t_ana_sel = 6;
        break;
    case TRIM_MODE_LVDS_TX :
        gt_sal_trim_regs._rE0.t_ana_sel = 6;
        break;
    case TRIM_MODE_LVDS_RX :
        gt_sal_trim_regs._rE0.t_ana_sel = 6;
        break;
    case TRIM_MODE_ADC_GAIN :
        gt_sal_trim_regs._rE0.t_ana_sel = 1;
        break;
    case TRIM_MODE_ADC_OFFSET :
        gt_sal_trim_regs._rE0.t_ana_sel = 1;
        break;
    case TRIM_MODE_TEMP_TRIM_BGR :
        gt_sal_trim_regs._rE0.t_ana_sel = 0;
        break;
    case TRIM_MODE_CURRENT_ITRIM :
        gt_sal_trim_regs._rE0.t_ana_sel = 6;
        break;
    }
    i2c_info.reg_addr = SAL_TRIM_ADDR_TRIM_CONTROL;
    i2c_info.data = gt_sal_trim_regs._rE0.val;
    sal_write_reg_i2c(&i2c_info);
}

void sal_trim_set_max_curr(void)
{
    _sal_i2c_info_t i2c_info = {0, };

    gt_sal_regs._r0F.r_curr_max_lvl = SAL_R_MAX_CURR;
    gt_sal_regs._r0F.g_curr_max_lvl = SAL_G_MAX_CURR;
    gt_sal_regs._r0F.b_curr_max_lvl = SAL_B_MAX_CURR;

    i2c_info.reg_addr = SAL_ADDR_CURR_MAX_LVL;
    i2c_info.data = gt_sal_regs._r0F.val;
    sal_write_reg_i2c(&i2c_info);
}

void sal_adc_test_init(void)
{
    _sal_i2c_info_t i2c_info = {0, };

    //decode_mode_set(DECODE_ADC_GAIN_TRIM);
    decode_mode_set(DECODE_NONE);

    gt_sal_regs._r07.vext_mon_e = 1;
    i2c_info.reg_addr = SAL_ADDR_SETUP2;
    i2c_info.data = gt_sal_regs._r07.val;
    sal_write_reg_i2c(&i2c_info);

    sal_trim_t_ana_sel(TRIM_MODE_ADC_GAIN);
}

void sal_adc_test_read_adc(void)
{
    _sal_i2c_info_t i2c_info = {0, };

    i2c_info.reg_addr = SAL_ADDR_VEXT_TM;
    sal_read_reg_i2c(&i2c_info);
}

void sal_current_test_init(void)
{
    _sal_i2c_info_t i2c_info = {0, };

    decode_mode_set(DECODE_LTC_DRIVER_A);

    gt_sal_trim_regs._rE0.pwm_max_r_e = 1;
    gt_sal_trim_regs._rE0.pwm_max_g_e = 1;
    gt_sal_trim_regs._rE0.pwm_max_b_e = 1;
    i2c_info.reg_addr = SAL_TRIM_ADDR_TRIM_CONTROL;
    i2c_info.data = gt_sal_trim_regs._rE0.val;
    sal_write_reg_i2c(&i2c_info);

    gt_sal_regs._r20.val = 0;
    i2c_info.reg_addr = SAL_ADDR_RGB1;
    i2c_info.data = gt_sal_regs._r20.val;
    sal_write_reg_i2c(&i2c_info);

    gt_sal_regs._r21.val = 0;
    i2c_info.reg_addr = SAL_ADDR_RGB2;
    i2c_info.data = gt_sal_regs._r21.val;
    sal_write_reg_i2c(&i2c_info);

    gt_sal_trim_regs._rF1.pwm_max_r = 0;
    i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR14;
    i2c_info.data = gt_sal_trim_regs._rF1.val;
    sal_write_reg_i2c(&i2c_info);

    gt_sal_trim_regs._rF2.pwm_max_g = 0;
    i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR15;
    i2c_info.data = gt_sal_trim_regs._rF2.val;
    sal_write_reg_i2c(&i2c_info);

    gt_sal_trim_regs._rF3.pwm_max_b = 0;
    i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR16;
    i2c_info.data = gt_sal_trim_regs._rF3.val;
    sal_write_reg_i2c(&i2c_info);

    sal_trim_set_max_curr();
    sal_trim_t_ana_sel(TRIM_MODE_CURRENT_ITRIM);
}

void sal_osc_test_init(void)
{
    _sal_i2c_info_t i2c_info = {0, };

    decode_mode_set(DECODE_NONE);
    I2C_SIO2P_PU_LO(); // disconnect SIO2P PU
    I2C_SIO2N_PU_LO(); // disconnect SIO2N PU
    I2C_SIO2N_PD_LO(); // disconnect SIO2N PD

    gt_sal_trim_regs._rE0.cko_e = 1;
    i2c_info.reg_addr = SAL_TRIM_ADDR_TRIM_CONTROL;
    i2c_info.data = gt_sal_trim_regs._rE0.val;
    sal_write_reg_i2c(&i2c_info);

    sal_trim_t_ana_sel(TRIM_MODE_OSC_CTL);
}

void sal_osc_test_start(uint8_t kkk)
{
    uint32_t cnt = 0;
    _sal_i2c_info_t i2c_info = {0, };

    i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR4;
    uint16_t temp_e7_val = sal_read_reg_i2c(&i2c_info);

    uint8_t start = kkk * 32;
    for (uint16_t reg = start ; reg < (start + 32) ; ++reg)
    {
        temp_e7_val &= 0x00F;

        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR4;
        i2c_info.data = temp_e7_val | (reg << 4);
        sal_write_reg_i2c(&i2c_info);

        tim_input_capture_start();
        while (gb_input_capture_started) {}
        cnt = tim_input_capture_get_average_cnt();

        print(LOG_LV_INFO, "reg,%u,cnt,%u\r\n", reg, cnt);
    }
}

void sal_otp_burn_test(void)
{
    _sal_i2c_info_t i2c_info = {0, };
    sal_vcc_level_set(SAL_VCC_5V5);

    LL_mDelay(9);

    gt_sal_trim_regs._rE1.otp_pg1 = 1;
    gt_sal_trim_regs._rE1.otp_pg2 = 1;
    i2c_info.reg_addr = SAL_TRIM_ADDR_OTP_CONTROL1;
    i2c_info.data = gt_sal_trim_regs._rE1.val;
    sal_write_reg_i2c(&i2c_info);
}

void sal_mode_boot_test(uint8_t sio1_mode)
{
    uint16_t status1 = 0;
    decode_mode_set((decode_mode_t)DECODE_NONE);

    sal_communication_mode_selection((_sal_com_mode_t)sio1_mode, SAL_COM_EOL);

    sal_vcc_level_set(SAL_VCC_5V0);
    sal_vcc_en(PWR_ON);
    sal_vled_en(PWR_ON);
    LL_mDelay(10-1);

    sal_make_mcu_mode();
    sal_make_i2c_mode();
    LL_mDelay(10);

    _sal_i2c_info_t _info_ = {0, };
    _info_.reg_addr = 0x01;
    status1 = sal_read_reg_i2c(&_info_);

    if (status1 != SAL_RTN_VAL_I2C_RX_ERROR)
    {
        uint8_t status = ((status1 & 0x70) >> 4);
        uint8_t sio2 = ((status1 & 0x0C) >> 2);
        uint8_t sio1 = ((status1 & 0x03) >> 0);

        print(LOG_LV_INFO, "status : %u, sio1_mode : %u, sio2_mode : %u\r\n", status, sio1, sio2);
        print(LOG_LV_INFO, "%s    %s    %s\r\n", gs_sal_ic_status_str[status], gs_sal_sio_status_str[sio1], gs_sal_sio_status_str[sio2]);
    }
}

void sal_tmux_sel_test(uint8_t tmux_sel)
{
    if (tmux_sel < 16)
    {
        gt_sal_trim_regs._rE1.tmux_sel = tmux_sel;

        _sal_i2c_info_t _info_ = {0, };
        _info_.reg_addr = SAL_TRIM_ADDR_OTP_CONTROL1;
        _info_.data = gt_sal_trim_regs._rE1.val;

        sal_write_reg_i2c(&_info_);

        print(LOG_LV_INFO, "%s\r\n", gs_sal_tmux_sel_str[tmux_sel]);
    }
    else
    {
        print(LOG_LV_INFO, "Invalid Input [%u] - [0 ~ 15]\r\n", tmux_sel);
    }
}

static void clear_trim_parameters(void)
{
    memset(&gt_sal_trim_param, 0, sizeof(gt_sal_trim_param));
    memset(&gt_sal_regs, 0, sizeof(gt_sal_regs));
    memset(&gt_sal_trim_regs, 0, sizeof(gt_sal_trim_regs));
    memset(&gn_sal_trim_v_trim_r1_r2, 0, sizeof(gn_sal_trim_v_trim_r1_r2));
    memset(&gn_sal_trim_ldo, 0, sizeof(gn_sal_trim_ldo));

    gt_trim_step = TRIM_STEP_NONE;
    gt_trim_mode = TRIM_MODE_V_TRIM_BGR;
    gt_trim_error = TRIM_ERROR_NONE;
    gt_otp_verify_step = OTP_VERIFY_PWR_OFF;

    gn_sal_trim_v_trim_r1_r2_cnt = 0;
    gn_sal_trim_ldo_cnt = 0;

    gn_sal_osc_trim_val = 128;
    gn_sal_adc_offset_trim_val = 256;
}

static void display_trim_info(trim_mode_t n_trim_mode)
{
#if (TRIM_LOG_TYPE == TRIM_LOG_ADC)
    print(LOG_LV_INFO, "[%s] [REG : 0x%04X] [VALUE : %5u] [TARGET : %5u/%5u] [ADJ : %s] [STEP : %3u] [CNT : %3u] [CH : %u]\r\n", \
        gs_sal_trim_mode_str[n_trim_mode],\
        gt_sal_trim_param[n_trim_mode].reg_now[gn_sal_trim_ch],\
        gt_sal_trim_param[n_trim_mode].value_now[gn_sal_trim_ch],\
        gn_sal_trim_range[n_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET_MIN],\
        gn_sal_trim_range[n_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET_MAX],\
        gs_sal_trim_adj_flag_str[gt_sal_trim_param[n_trim_mode].adjust_flag[gn_sal_trim_ch]],\
        gt_sal_trim_param[n_trim_mode].reg_step[gn_sal_trim_ch],\
        gt_sal_trim_param[n_trim_mode].saved_cnt, gn_sal_trim_ch);
#else
    print(LOG_LV_INFO, "[%s] [REG : 0x%04X] [VALUE : %.3f] [TARGET : %.3f/%.3f] [ADJ : %s] [STEP : %3u] [CNT : %3u] [CH : %u]\r\n", \
        gs_sal_trim_mode_str[n_trim_mode],\
        gt_sal_trim_param[n_trim_mode].reg_now[gn_sal_trim_ch],\
        convert_value_to_parameter(n_trim_mode, gt_sal_trim_param[n_trim_mode].value_now[gn_sal_trim_ch]),\
        convert_value_to_parameter(n_trim_mode, gn_sal_trim_range[n_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET_MIN]),\
        convert_value_to_parameter(n_trim_mode, gn_sal_trim_range[n_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET_MAX]),\
        gs_sal_trim_adj_flag_str[gt_sal_trim_param[n_trim_mode].adjust_flag[gn_sal_trim_ch]],\
        gt_sal_trim_param[n_trim_mode].reg_step[gn_sal_trim_ch],\
        gt_sal_trim_param[n_trim_mode].saved_cnt, gn_sal_trim_ch);
#endif
}
static void display_r1_r2_info(void)
{
    for (uint8_t r1 = 0 ; r1 < 16 ; ++r1)
    {
        char msg[(6+1)*16 + 5] = {0, };
        for (uint8_t r2 = 0 ; r2 < 16 ; ++r2)
        {
            snprintf(msg+r2*6, 6+1, "%6u", gn_sal_trim_v_trim_r1_r2[r1 * 16 + r2]);
        }
        print(LOG_LV_DEBUG, "%s\r\n", msg);
    }
}

void tim_input_capture_start(void)
{
    CNT_MR_LO();
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)gn_input_capture_count);

    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, (uint32_t)INPUT_CAPTURE_SIZE);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);

    gb_input_capture_started = true;
}

void tim_input_capture_stop(void)
{
    CNT_MR_HI();
    gb_input_capture_started = false;
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
}

uint32_t tim_input_capture_get_average_cnt(void)
{
    uint32_t ret = 0;
    uint64_t temp_sum = 0;
    for (uint16_t i = 0 ; i < (INPUT_CAPTURE_SIZE - 1) ; ++i)
    {
        temp_sum += gn_input_capture_count[i + 1]; // skip first count value
    }

    ret = (uint32_t)(((double)temp_sum / (INPUT_CAPTURE_SIZE - 1)) + 0.5f);
    return ret;
}

static uint16_t convert_frequency_to_count(float n_MHz)
{
    return (uint16_t)(((float)TIM2_APB1_FREQ / (n_MHz / (SAL_INTERNAL_DIVIDE * SAL_JIG_DIVIDE))) + 0.5f);
}

float convert_count_to_freq(uint32_t cnt)
{
    return ((TIM2_APB1_FREQ * CONST_MHz_to_KHz) / (float)cnt);
}

static float convert_value_to_parameter(trim_mode_t trim_mode, uint16_t n_value)
{
    float f_ret = 0.0f;
    float temp_value = 0.0f;
    switch (trim_mode)
    {
    case TRIM_MODE_V_TRIM_BGR :
        temp_value = convert_adc_to_mvoltage(n_value) / CONST_V_to_mV; // unit : V
        f_ret = temp_value / V_TRIM_BGR_MUL;
        break;
    case TRIM_MODE_V_TRIM_R1_R2 :
        temp_value = convert_adc_to_mvoltage(n_value) / CONST_V_to_mV; // unit : V
        f_ret = temp_value / V_TRIM_BGR_MUL;
        break;
    case TRIM_MODE_I_TRIM_BGR :
        temp_value = convert_adc_to_mvoltage(n_value); // unit : mV
        f_ret = temp_value / I_TRIM_BGR_MUL;
        break;
    case TRIM_MODE_LDO_CTL :
        temp_value = convert_adc_to_mvoltage(n_value) / CONST_V_to_mV; // unit : V
        f_ret = temp_value / LDO_CTL_MUL;
        break;
    case TRIM_MODE_OSC_CTL :
        f_ret = ((float)(TIM2_APB1_FREQ * SAL_INTERNAL_DIVIDE * SAL_JIG_DIVIDE) / n_value);
        break;
    case TRIM_MODE_LVDS_TX :
        break;
    case TRIM_MODE_LVDS_RX :
        break;
    case TRIM_MODE_ADC_GAIN :
        break;
    case TRIM_MODE_ADC_OFFSET :
        break;
    case TRIM_MODE_TEMP_TRIM_BGR :
        break;
    case TRIM_MODE_CURRENT_ITRIM :
        f_ret = convert_adc_to_current(n_value, GAIN_HIGH);
        break;
    }

    //print(LOG_LV_DEBUG, "%s - %.5f\r\n", gs_sal_trim_mode_str[trim_mode], f_ret);

    return f_ret;
}

void sal_trim_start(void)
{
    gt_trim_step = TRIM_STEP_PWR_ON;
}

static void change_trim_step(trim_step_t trim_step)
{
    if (gt_trim_error == TRIM_ERROR_NONE)
    {
        gt_trim_step = trim_step;
    }
    else
    {
        gt_trim_step = TRIM_STEP_ERROR;
    }
}

static void sal_set_condition_current_trim(uint8_t itrm_cnt)
{
    _sal_i2c_info_t i2c_info = {0, };

    gt_sal_trim_regs._rE0.pwm_max_r_e = 0;
    gt_sal_trim_regs._rE0.pwm_max_g_e = 0;
    gt_sal_trim_regs._rE0.pwm_max_b_e = 0;

    gt_sal_trim_regs._rF1.pwm_max_r = 0;
    gt_sal_trim_regs._rF2.pwm_max_g = 0;
    gt_sal_trim_regs._rF3.pwm_max_b = 0;

    if (gn_sal_trim_ch == 0) // ch-R
    {
        gt_sal_trim_regs._rE0.pwm_max_r_e = 1;
        gt_sal_trim_regs._rF1.pwm_max_r = 4095;
    }
    else if (gn_sal_trim_ch == 1) // ch-G
    {
        gt_sal_trim_regs._rE0.pwm_max_g_e = 1;
        gt_sal_trim_regs._rF2.pwm_max_g = 4095;
    }
    else // ch-B
    {
        gt_sal_trim_regs._rE0.pwm_max_b_e = 1;
        gt_sal_trim_regs._rF3.pwm_max_b = 4095;
    }

    i2c_info.reg_addr = SAL_TRIM_ADDR_TRIM_CONTROL;
    i2c_info.data = gt_sal_trim_regs._rE0.val;
    sal_write_reg_i2c(&i2c_info);

    i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR14;
    i2c_info.data = gt_sal_trim_regs._rF1.val;
    sal_write_reg_i2c(&i2c_info);

    i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR15;
    i2c_info.data = gt_sal_trim_regs._rF2.val;
    sal_write_reg_i2c(&i2c_info);

    i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR16;
    i2c_info.data = gt_sal_trim_regs._rF3.val;
    sal_write_reg_i2c(&i2c_info);

    gt_sal_regs._r20.val = 0;
    i2c_info.reg_addr = SAL_ADDR_RGB1;
    i2c_info.data = gt_sal_regs._r20.val;
    sal_write_reg_i2c(&i2c_info);

    gt_sal_regs._r21.val = 0;
    i2c_info.reg_addr = SAL_ADDR_RGB2;
    i2c_info.data = gt_sal_regs._r21.val;
    sal_write_reg_i2c(&i2c_info);
}

void sal_trimming_procedure(void)
{
    _sal_i2c_info_t i2c_info = {0, };
    switch(gt_trim_step)
    {
    case TRIM_STEP_NONE :
        break;
    case TRIM_STEP_PWR_ON :
        sal_make_mcu_mode();
        sal_vcc_level_set(SAL_VCC_5V0);
        sal_vcc_en(PWR_ON);
        sal_vled_en(PWR_ON);
        gn_trim_delay = 10;
        change_trim_step(TRIM_STEP_SAL_INIT);
        break;
    case TRIM_STEP_SAL_INIT :
        if (gn_trim_delay)
        {
            if (gn_trim_delay == 5)
            {
                sal_make_i2c_mode();
            }
            --gn_trim_delay;
        }
        else
        {
            sal_trim_test_enable();

            sal_trim_otp_protection(OTP_PROTECT_OFF);
            sal_trim_read_all_registers();

            // write something if needed
            sal_trim_regs_init();
            change_trim_step(TRIM_STEP_MODE_INIT);
            /************************************/
            //change_trim_step(TRIM_STEP_OTP_START);
            /************************************/
        }
        break;
    case TRIM_STEP_MODE_INIT :
        gn_sal_trim_ch = 0;
        switch (gt_trim_mode)
        {
        case TRIM_MODE_V_TRIM_BGR :
            decode_mode_set(DECODE_TEST_V_TRIM);
            gt_sal_trim_param[TRIM_MODE_V_TRIM_BGR].init_reg_step = INIT_STEP_V_TRIM_BGR;
            gt_sal_trim_param[TRIM_MODE_V_TRIM_BGR].reg_now[0] = gt_sal_trim_regs._rE4.v_trim_bgr;
            break;
        case TRIM_MODE_V_TRIM_R1_R2 :
            decode_mode_set(DECODE_TEST_V_TRIM);
            gt_sal_trim_param[TRIM_MODE_V_TRIM_R1_R2].init_reg_step = INIT_STEP_V_R1_R2;
            gt_sal_trim_param[TRIM_MODE_V_TRIM_R1_R2].reg_now[0] = gt_sal_trim_regs._rE6.bgr_r1_trim;
            gt_sal_trim_param[TRIM_MODE_V_TRIM_R1_R2].reg_now[1] = gt_sal_trim_regs._rE7.bgr_r2_trim;
            print(LOG_LV_INFO, "===============Get R1/R2 Start============\r\n");
            break;
        case TRIM_MODE_I_TRIM_BGR :
            decode_mode_set(DECODE_TEST_V_TRIM);
            gt_sal_trim_param[TRIM_MODE_I_TRIM_BGR].init_reg_step = INIT_STEP_I_TRIM_BGR;
            gt_sal_trim_param[TRIM_MODE_I_TRIM_BGR].reg_now[0] = gt_sal_trim_regs._rE5.i_trim_bgr;
            break;
        case TRIM_MODE_LDO_CTL :
            decode_mode_set(DECODE_TEST_V_TRIM);
            gt_sal_trim_param[TRIM_MODE_LDO_CTL].init_reg_step = INIT_STEP_LDO_CTL;
            gt_sal_trim_param[TRIM_MODE_LDO_CTL].reg_now[0] = gt_sal_trim_regs._rE5.vldo_ctrl;
            break;
        case TRIM_MODE_OSC_CTL :
            decode_mode_set(DECODE_NONE);
            I2C_SIO2P_PU_LO(); // disconnect SIO2P PU
            I2C_SIO2N_PU_LO(); // disconnect SIO2N PU
            I2C_SIO2N_PD_LO(); // disconnect SIO2N PD
            gt_sal_trim_regs._rE0.cko_e = 1;
            i2c_info.reg_addr = SAL_TRIM_ADDR_TRIM_CONTROL;
            i2c_info.data = gt_sal_trim_regs._rE0.val;
            sal_write_reg_i2c(&i2c_info);
            gt_sal_trim_param[TRIM_MODE_OSC_CTL].init_reg_step = INIT_STEP_OSC_CTL;
            gt_sal_trim_param[TRIM_MODE_OSC_CTL].reg_now[0] = gn_sal_osc_trim_val;//gt_sal_trim_regs._rE7.osc_ctl;
            break;
        case TRIM_MODE_LVDS_TX :
            //decode_mode_set(DECODE_SIO1_LVDS_TX);
            gt_sal_trim_param[TRIM_MODE_LVDS_TX].init_reg_step = INIT_STEP_LVDS_TX;
            gt_sal_trim_param[TRIM_MODE_LVDS_TX].reg_now[0] = gt_sal_trim_regs._rE4.sio1_tr_tx_bias;
            gt_sal_trim_param[TRIM_MODE_LVDS_TX].reg_now[1] = gt_sal_trim_regs._rE4.sio2_tr_tx_bias;
            break;
        case TRIM_MODE_LVDS_RX :
            //decode_mode_set(DECODE_SIO1_LVDS_RX);
            gt_sal_trim_param[TRIM_MODE_LVDS_RX].init_reg_step = INIT_STEP_LVDS_RX;
            gt_sal_trim_param[TRIM_MODE_LVDS_RX].reg_now[0] = gt_sal_trim_regs._rE5.sio1_tr_res;
            gt_sal_trim_param[TRIM_MODE_LVDS_RX].reg_now[1] = gt_sal_trim_regs._rE5.sio2_tr_res;
            break;
        case TRIM_MODE_ADC_GAIN :
            decode_mode_set(DECODE_ADC_GAIN_TRIM);
            gt_sal_regs._r07.vext_mon_e = 1;
            i2c_info.reg_addr = SAL_ADDR_SETUP2;
            i2c_info.data = gt_sal_regs._r07.val;
            sal_write_reg_i2c(&i2c_info);
            gt_sal_trim_param[TRIM_MODE_ADC_GAIN].init_reg_step = INIT_STEP_ADC_GAIN;
            gt_sal_trim_param[TRIM_MODE_ADC_GAIN].reg_now[0] = gt_sal_trim_regs._rEB.adc_gain;
            break;
        case TRIM_MODE_ADC_OFFSET :
            decode_mode_set(DECODE_ADC_GAIN_TRIM);
            gt_sal_regs._r07.vext_mon_e = 1;
            i2c_info.reg_addr = SAL_ADDR_SETUP2;
            i2c_info.data = gt_sal_regs._r07.val;
            sal_write_reg_i2c(&i2c_info);
            gt_sal_trim_param[TRIM_MODE_ADC_OFFSET].init_reg_step = INIT_STEP_ADC_OFFSET;
            gt_sal_trim_param[TRIM_MODE_ADC_OFFSET].reg_now[0] = gn_sal_adc_offset_trim_val;//gt_sal_trim_regs._rEA.adc_offset;
            break;
        case TRIM_MODE_TEMP_TRIM_BGR :
            decode_mode_set(DECODE_NONE); // set test_v pin to open
            gt_sal_regs._r07.vext_mon_e = 0;
            i2c_info.reg_addr = SAL_ADDR_SETUP2;
            i2c_info.data = gt_sal_regs._r07.val;
            sal_write_reg_i2c(&i2c_info);
            gt_sal_trim_param[TRIM_MODE_TEMP_TRIM_BGR].init_reg_step = INIT_STEP_TEMP_TRIM_BGR;
            gt_sal_trim_param[TRIM_MODE_TEMP_TRIM_BGR].reg_now[0] = gt_sal_trim_regs._rE6.temp_trim_bgr;
            break;
        case TRIM_MODE_CURRENT_ITRIM :
            decode_mode_set(DECODE_LTC_DRIVER_A);
            sal_trim_set_max_curr();
            gt_sal_trim_param[TRIM_MODE_CURRENT_ITRIM].init_reg_step = 5;//INIT_STEP_CURRENT_ITRIM;
            gt_sal_trim_param[TRIM_MODE_CURRENT_ITRIM].reg_now[0] = gt_sal_trim_regs._rEC.itrim_r;
            gt_sal_trim_param[TRIM_MODE_CURRENT_ITRIM].reg_now[1] = gt_sal_trim_regs._rED.itrim_g;
            gt_sal_trim_param[TRIM_MODE_CURRENT_ITRIM].reg_now[2] = gt_sal_trim_regs._rEE.itrim_b;
            break;
        default :
            break;
        }
        sal_trim_t_ana_sel(gt_trim_mode);
        change_trim_step(TRIM_STEP_MODE_SET_CONDITION);
        break;
    case TRIM_STEP_MODE_SET_CONDITION :
        switch (gt_trim_mode)
        {
        case TRIM_MODE_V_TRIM_BGR :
            // skip trim
            gt_sal_trim_regs._rE4.v_trim_bgr = 0x17;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR1;
            i2c_info.data = gt_sal_trim_regs._rE4.val;
            sal_write_reg_i2c(&i2c_info);

            print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
            ++gt_trim_mode;
            change_trim_step(TRIM_STEP_MODE_INIT);
            break;
        case TRIM_MODE_V_TRIM_R1_R2 :
#if 1
            gn_v_trim_r1 = (uint8_t)(gn_sal_trim_v_trim_r1_r2_cnt / 16);
            gn_v_trim_r2 = (uint8_t)(gn_sal_trim_v_trim_r1_r2_cnt % 16);

            gt_sal_trim_regs._rE6.bgr_r1_trim = gn_v_trim_r1;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR3;
            i2c_info.data = gt_sal_trim_regs._rE6.val;
            sal_write_reg_i2c(&i2c_info);

            gt_sal_trim_regs._rE7.bgr_r2_trim = gn_v_trim_r2;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR4;
            i2c_info.data = gt_sal_trim_regs._rE7.val;
            sal_write_reg_i2c(&i2c_info);

            change_trim_step(TRIM_STEP_ADC_SET_CH);
#else
            gt_sal_trim_regs._rE6.bgr_r1_trim = 8;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR3;
            i2c_info.data = gt_sal_trim_regs._rE6.val;
            sal_write_reg_i2c(&i2c_info);

            gt_sal_trim_regs._rE7.bgr_r2_trim = 8;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR4;
            i2c_info.data = gt_sal_trim_regs._rE7.val;
            sal_write_reg_i2c(&i2c_info);

            print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
            ++gt_trim_mode;
            change_trim_step(TRIM_STEP_MODE_INIT);
#endif
            break;
        case TRIM_MODE_I_TRIM_BGR :
            gt_sal_trim_regs._rE5.i_trim_bgr = 0x10;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR2;
            i2c_info.data = gt_sal_trim_regs._rE5.val;
            sal_write_reg_i2c(&i2c_info);

            print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
            ++gt_trim_mode;
            change_trim_step(TRIM_STEP_MODE_INIT);
            break;
        case TRIM_MODE_LDO_CTL :
            gt_sal_trim_regs._rE5.vldo_ctrl = gn_sal_trim_ldo_cnt;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR2;
            i2c_info.data = gt_sal_trim_regs._rE5.val;
            sal_write_reg_i2c(&i2c_info);
            change_trim_step(TRIM_STEP_ADC_SET_CH);
            break;
        case TRIM_MODE_OSC_CTL :
            change_trim_step(TRIM_STEP_ADC_SET_CH);
            break;
        case TRIM_MODE_LVDS_TX :
            // skip trim
            gt_sal_trim_regs._rE4.sio1_tr_tx_bias = 0;
            gt_sal_trim_regs._rE4.sio2_tr_tx_bias = 0;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR1;
            i2c_info.data = gt_sal_trim_regs._rE4.val;
            sal_write_reg_i2c(&i2c_info);

            print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
            ++gt_trim_mode;
            change_trim_step(TRIM_STEP_MODE_INIT);
            break;
        case TRIM_MODE_LVDS_RX :
            // skip trim
            gt_sal_trim_regs._rE5.sio1_tr_res = 0;
            gt_sal_trim_regs._rE5.sio2_tr_res = 0;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP1_MIRROR2;
            i2c_info.data = gt_sal_trim_regs._rE5.val;
            sal_write_reg_i2c(&i2c_info);

            print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
            ++gt_trim_mode;
            change_trim_step(TRIM_STEP_MODE_INIT);
            break;
        case TRIM_MODE_ADC_GAIN :
            if (gn_sal_trim_ch == 0)
            {
                SAL_ADC_1V_LO(); //supply 2.5V to TEST_V
            }
            else
            {
                SAL_ADC_1V_HI(); //supply 1V to TEST_V
            }
            gn_trim_delay = 10;
            change_trim_step(TRIM_STEP_ADC_SET_CH);
            break;
        case TRIM_MODE_ADC_OFFSET :
            SAL_ADC_1V_LO(); //supply 2.5V to TEST_V
            gn_trim_delay = 10;
            change_trim_step(TRIM_STEP_ADC_SET_CH);
            break;
        case TRIM_MODE_TEMP_TRIM_BGR :
            change_trim_step(TRIM_STEP_ADC_SET_CH);
            break;
        case TRIM_MODE_CURRENT_ITRIM :
            adc_clear_buff();
            sal_set_condition_current_trim(0);
            change_trim_step(TRIM_STEP_ADC_SET_CH);
            break;
        default :
            break;
        }
        break;
    case TRIM_STEP_ADC_SET_CH :
        gn_adc_read_count = ADS114S08_READ_COUNT;
        ads114s08_select_single_ended_input(gn_adc_ch_table[gt_trim_mode]);
        change_trim_step(TRIM_STEP_ADC_START);
        break;
    case TRIM_STEP_ADC_START :
        if (gt_trim_mode == TRIM_MODE_OSC_CTL)
        {
            tim_input_capture_start();
        }
        else
        {
            ads114s08_set_start(1);
        }
        gn_trim_delay = 10;
        change_trim_step(TRIM_STEP_ADC_GET_CH);
        break;
    case TRIM_STEP_ADC_GET_CH :
        switch (gt_trim_mode)
        {
        case TRIM_MODE_V_TRIM_BGR :
        case TRIM_MODE_I_TRIM_BGR :
        case TRIM_MODE_LVDS_TX :
        case TRIM_MODE_LVDS_RX :
            if (gn_trim_delay)
            {
                --gn_trim_delay;
            }
            else
            {
                if (gb_ads114s08_drdy_done)
                {
                    ads114s08_set_start(0);    /* stop continuous conversion */
                    gb_ads114s08_drdy_done = 0;
                    gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] = get_adc_value();
                    change_trim_step(TRIM_STEP_CHECK);
                }
            }
            break;
        case TRIM_MODE_CURRENT_ITRIM :
            if (gn_trim_delay)
            {
                --gn_trim_delay;
            }
            else
            {
                if (gb_ads114s08_drdy_done)
                {
                    ads114s08_set_start(0);    /* stop continuous conversion */
                    gb_ads114s08_drdy_done = 0;
                    gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] = get_adc_value();
                    change_trim_step(TRIM_STEP_CHECK);
                }
            }
            break;
        case TRIM_MODE_V_TRIM_R1_R2 :
            if (gb_ads114s08_drdy_done)
            {
                gb_ads114s08_drdy_done = 0;
                gn_sal_trim_v_trim_r1_r2[gn_sal_trim_v_trim_r1_r2_cnt] = get_adc_value();
                ++gn_sal_trim_v_trim_r1_r2_cnt;
                if (gn_sal_trim_v_trim_r1_r2_cnt < 256)
                {
                    change_trim_step(TRIM_STEP_MODE_SET_CONDITION);
                }
                else
                {
                    print(LOG_LV_INFO, "===============Get R1/R2 Done============\r\n");
                    // find closest value & index, go to next trim
                    uint16_t found_closeset_gap = 0xFFFF;
                    uint16_t found_closeset_index = 255;

                    for (uint16_t index = 0 ; index < 256 ; ++index)
                    {
                        uint16_t temp_gap = 0xFFFF;
                        temp_gap = abs(gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET] - gn_sal_trim_v_trim_r1_r2[index]);
                        if (temp_gap < found_closeset_gap)
                        {
                            found_closeset_gap = temp_gap;
                            found_closeset_index = index;
                        }
                    }
                    print(LOG_LV_DEBUG, "R1/R2 closest index [%u - %u(%.3f)]\r\n", found_closeset_index, gn_sal_trim_v_trim_r1_r2[found_closeset_index], ((5.0f * gn_sal_trim_v_trim_r1_r2[found_closeset_index]) / 32767));
                    gn_v_trim_r1 = (uint8_t)(found_closeset_index / 16);
                    gn_v_trim_r2 = (uint8_t)(found_closeset_index % 16);

                    gt_sal_trim_param[gt_trim_mode].reg_now[0] = gn_v_trim_r1;
                    gt_sal_trim_param[gt_trim_mode].reg_now[1] = gn_v_trim_r2;

                    sal_trim_reg_write(0, gt_trim_mode, gt_sal_trim_param[gt_trim_mode].reg_now[0]);
                    sal_trim_reg_write(1, gt_trim_mode, gt_sal_trim_param[gt_trim_mode].reg_now[1]);
                    print(LOG_LV_INFO, "R1/R2 -- DONE [%3X, %3X]\r\n", gt_sal_trim_param[gt_trim_mode].reg_now[0], gt_sal_trim_param[gt_trim_mode].reg_now[1]);

                    display_r1_r2_info();

                    print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
                    ++gt_trim_mode;
                    change_trim_step(TRIM_STEP_MODE_INIT);
                }
            }
            break;
        case TRIM_MODE_LDO_CTL :
            if (gb_ads114s08_drdy_done)
            {
                gb_ads114s08_drdy_done = 0;
                gn_sal_trim_ldo[gn_sal_trim_ldo_cnt] = get_adc_value();
                print(LOG_LV_DEBUG, "LDO - %u [%u (%.3f)]\r\n", gn_sal_trim_ldo_cnt, gn_sal_trim_ldo[gn_sal_trim_ldo_cnt], (5.0f * gn_sal_trim_ldo[gn_sal_trim_ldo_cnt] / 32767));
                ++gn_sal_trim_ldo_cnt;
                if (gn_sal_trim_ldo_cnt < 8)
                {
                    change_trim_step(TRIM_STEP_MODE_SET_CONDITION);
                }
                else
                {
                    // find closest value & index, go to next trim
                    uint16_t found_closeset_gap = 0xFFFF;
                    uint16_t found_closeset_index = 7;

                    for (uint16_t index = 0 ; index < 8 ; ++index)
                    {
                        uint16_t temp_gap = 0xFFFF;
                        temp_gap = abs(gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET] - gn_sal_trim_ldo[index]);
                        if (temp_gap < found_closeset_gap)
                        {
                            found_closeset_gap = temp_gap;
                            found_closeset_index = index;
                        }
                    }
                    print(LOG_LV_DEBUG, "LDO closest index [%u] - [%5u]\r\n", found_closeset_index, gn_sal_trim_ldo[found_closeset_index]);

                    gt_sal_trim_param[gt_trim_mode].reg_now[0] = found_closeset_index;

                    if (gn_sal_trim_ldo[found_closeset_index] < gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET_MAX] &&
                    gn_sal_trim_ldo[found_closeset_index] > gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET_MIN])
                    {
                        sal_trim_reg_write(0, gt_trim_mode, gt_sal_trim_param[gt_trim_mode].reg_now[0]);
                        print(LOG_LV_INFO, "LDO -- DONE [%3X - %u (%.3f)]\r\n", gt_sal_trim_param[gt_trim_mode].reg_now[0], gn_sal_trim_ldo[found_closeset_index], (5.0f * gn_sal_trim_ldo[found_closeset_index] / 32767));
                        print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
                        ++gt_trim_mode;
                        change_trim_step(TRIM_STEP_MODE_INIT);
                    }
                    else
                    {
                        print(LOG_LV_ERROR, "LDO -- FAIL [%3X - %u (%.3f)]\r\n", gt_sal_trim_param[gt_trim_mode].reg_now[0], gn_sal_trim_ldo[found_closeset_index], (5.0f * gn_sal_trim_ldo[found_closeset_index] / 32767));
                        gt_trim_error = TRIM_ERROR_REG_OVER_RANGE;
                        change_trim_step(TRIM_STEP_ERROR);
                    }
                }
            }
            break;
        case TRIM_MODE_OSC_CTL :
            if (gb_input_capture_started == false)
            {
                gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] = tim_input_capture_get_average_cnt();/* get timer count */
                change_trim_step(TRIM_STEP_CHECK);
            }
            break;
        case TRIM_MODE_ADC_GAIN :
            if (gn_trim_delay)
            {
                --gn_trim_delay;
            }
            else
            {
                i2c_info.reg_addr = SAL_ADDR_VEXT_TM;
                gt_sal_regs._r05.vext_tm = sal_read_reg_i2c(&i2c_info);
                gn_sal_trim_temp_adc_value[gn_sal_trim_ch] = gt_sal_regs._r05.vext_tm;
                ++gn_sal_trim_ch;
                if (gn_sal_trim_ch < 2)
                {
                    change_trim_step(TRIM_STEP_MODE_SET_CONDITION);
                }
                else
                {
                    gn_sal_trim_ch = 0;
                    // VEXT_TM @ TEST_V = 2.5V - VEXT_TM @ TEST_V = 1V
                    gt_sal_trim_param[gt_trim_mode].value_now[0] = (gn_sal_trim_temp_adc_value[0] - gn_sal_trim_temp_adc_value[1]);
                    print(LOG_LV_DEBUG, "2.5V : %5u    1V : %5u\r\n", gn_sal_trim_temp_adc_value[0], gn_sal_trim_temp_adc_value[1]);
                    change_trim_step(TRIM_STEP_CHECK);
                }
            }
            break;
        case TRIM_MODE_ADC_OFFSET :
            if (gn_trim_delay)
            {
                --gn_trim_delay;
            }
            else
            {
                i2c_info.reg_addr = SAL_ADDR_VEXT_TM;
                gt_sal_regs._r05.vext_tm = sal_read_reg_i2c(&i2c_info);
                gt_sal_trim_param[gt_trim_mode].value_now[0] = gt_sal_regs._r05.vext_tm;
                print(LOG_LV_DEBUG, "ADC OFFSET : %5u\r\n", gt_sal_regs._r05.vext_tm);
                change_trim_step(TRIM_STEP_CHECK);
            }
            break;
        case TRIM_MODE_TEMP_TRIM_BGR :
            i2c_info.reg_addr = SAL_ADDR_TEMP;
            gt_sal_regs._r03.temp = sal_read_reg_i2c(&i2c_info);
            gt_sal_trim_param[gt_trim_mode].value_now[0] = gt_sal_regs._r03.temp;
            print(LOG_LV_DEBUG, "TEMP BGR : %5u\r\n", gt_sal_regs._r03.temp);
            change_trim_step(TRIM_STEP_CHECK);
            break;
        default :
            break;
        }
        break;
    case TRIM_STEP_CHECK :
        ++gt_sal_trim_param[gt_trim_mode].loop_cnt;
        if (gt_sal_trim_param[gt_trim_mode].loop_cnt > SAL_TRIM_TRIAL_CNT_MAX)
        {
            gt_trim_error = TRIM_ERROR_OVERRUN;
            change_trim_step(TRIM_STEP_ERROR);
        }
        else
        {
            if (gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] < gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET])
            {
                if (gt_sal_trim_param[gt_trim_mode].loop_cnt == 1)
                {
                    gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch] = gt_sal_trim_param[gt_trim_mode].init_reg_step;
                }
                else
                {
                    uint16_t temp_val_gap = abs(gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] - gt_sal_trim_param[gt_trim_mode].value_prev[gn_sal_trim_ch]);
                    uint16_t temp_reg_gap = abs(gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] - gt_sal_trim_param[gt_trim_mode].reg_prev[gn_sal_trim_ch]);
                    float temp_val_per_reg = (float)temp_val_gap / temp_reg_gap;

                    gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch] = (uint16_t)(fabsf(gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET] - gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch]) / temp_val_per_reg + 0.5f);
                }

                if (gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] > gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET_MIN])
                {
                    gt_sal_trim_param[gt_trim_mode].reg_saved[gn_sal_trim_ch][gt_sal_trim_param[gt_trim_mode].saved_cnt] = gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch];
                    gt_sal_trim_param[gt_trim_mode].value_saved[gn_sal_trim_ch][gt_sal_trim_param[gt_trim_mode].saved_cnt] = gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch];
                    ++gt_sal_trim_param[gt_trim_mode].saved_cnt;
                }

                if (gt_sal_trim_param[gt_trim_mode].saved_cnt >= (SAL_TRIM_SAVED_CNT_MAX - 1)) // index is 0, 1, ..., (SAL_TRIM_SAVED_CNT_MAX - 1)
                {
                    gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] = TRIM_ADJ_FLAG_DONE;
                }
                else
                {
                    //if (gt_trim_mode == TRIM_MODE_OSC_CTL || gt_trim_mode == TRIM_MODE_I_TRIM_BGR || gt_trim_mode == TRIM_MODE_LDO_CTL)
                    if (gt_trim_mode == TRIM_MODE_OSC_CTL || gt_trim_mode == TRIM_MODE_I_TRIM_BGR || gt_trim_mode == TRIM_MODE_ADC_GAIN)
                    {
                        gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] = TRIM_ADJ_FLAG_DW;
                    }
                    else
                    {
                        gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] = TRIM_ADJ_FLAG_UP;
                    }
                }
            }
            else if (gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] > gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET])
            {
                if (gt_sal_trim_param[gt_trim_mode].loop_cnt == 1)
                {
                    gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch] = gt_sal_trim_param[gt_trim_mode].init_reg_step;
                }
                else
                {
                    uint16_t temp_val_gap = abs(gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] - gt_sal_trim_param[gt_trim_mode].value_prev[gn_sal_trim_ch]);
                    uint16_t temp_reg_gap = abs(gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] - gt_sal_trim_param[gt_trim_mode].reg_prev[gn_sal_trim_ch]);
                    float temp_val_per_reg = (float)temp_val_gap / temp_reg_gap;

                    gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch] = (uint16_t)(fabsf(gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET] - gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch]) / temp_val_per_reg + 0.5f);
                }

                if (gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch] < gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET_MAX])
                {
                    gt_sal_trim_param[gt_trim_mode].reg_saved[gn_sal_trim_ch][gt_sal_trim_param[gt_trim_mode].saved_cnt] = gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch];
                    gt_sal_trim_param[gt_trim_mode].value_saved[gn_sal_trim_ch][gt_sal_trim_param[gt_trim_mode].saved_cnt] = gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch];
                    ++gt_sal_trim_param[gt_trim_mode].saved_cnt;
                }

                if (gt_sal_trim_param[gt_trim_mode].saved_cnt >= (SAL_TRIM_SAVED_CNT_MAX - 1)) // index is 0, 1, ..., (SAL_TRIM_SAVED_CNT_MAX - 1)
                {
                    gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] = TRIM_ADJ_FLAG_DONE;
                }
                else
                {
                    //if (gt_trim_mode == TRIM_MODE_OSC_CTL || gt_trim_mode == TRIM_MODE_I_TRIM_BGR || gt_trim_mode == TRIM_MODE_LDO_CTL)
                    if (gt_trim_mode == TRIM_MODE_OSC_CTL || gt_trim_mode == TRIM_MODE_I_TRIM_BGR || gt_trim_mode == TRIM_MODE_ADC_GAIN)
                    {
                        gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] = TRIM_ADJ_FLAG_UP;
                    }
                    else
                    {
                        gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] = TRIM_ADJ_FLAG_DW;
                    }
                }
            }
            else
            {
				gt_sal_trim_param[gt_trim_mode].reg_saved[gn_sal_trim_ch][gt_sal_trim_param[gt_trim_mode].saved_cnt] = gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch];
				gt_sal_trim_param[gt_trim_mode].value_saved[gn_sal_trim_ch][gt_sal_trim_param[gt_trim_mode].saved_cnt] = gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch];
                ++gt_sal_trim_param[gt_trim_mode].saved_cnt;
                gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] = TRIM_ADJ_FLAG_DONE;
				gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch] = 0;
            }
            gt_sal_trim_param[gt_trim_mode].value_prev[gn_sal_trim_ch] = gt_sal_trim_param[gt_trim_mode].value_now[gn_sal_trim_ch];
            gt_sal_trim_param[gt_trim_mode].reg_prev[gn_sal_trim_ch] = gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch];
            display_trim_info(gt_trim_mode);
            change_trim_step(TRIM_STEP_REG_WRITE);
        }
        break;
    case TRIM_STEP_REG_WRITE :
        if (gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] == TRIM_ADJ_FLAG_DONE)
        {
            if (gt_sal_trim_param[gt_trim_mode].saved_cnt == 1)
            {
                sal_trim_reg_write(gn_sal_trim_ch, gt_trim_mode, gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch]);
            }
            else
            {
                uint16_t found_closeset_gap = 0xFFFF;
                uint16_t found_closeset_index = (SAL_TRIM_SAVED_CNT_MAX - 1); // closest index can be up to (SAL_TRIM_SAVED_CNT_MAX - 1)

                for (uint8_t index = 0 ; index < gt_sal_trim_param[gt_trim_mode].saved_cnt ; ++index)
                {
                    uint16_t temp_gap = 0xFFFF;
                    temp_gap = abs(gn_sal_trim_range[gt_trim_mode][gn_sal_trim_ch][TRIM_PARAM_TARGET] - gt_sal_trim_param[gt_trim_mode].value_saved[gn_sal_trim_ch][index]);
                    if (temp_gap < found_closeset_gap)
                    {
                        found_closeset_gap = temp_gap;
                        found_closeset_index = index;
                    }
                	print(LOG_LV_DEBUG, "index : %u, value : %u, reg : %u, gap : %u\r\n", index, gt_sal_trim_param[gt_trim_mode].value_saved[gn_sal_trim_ch][index], gt_sal_trim_param[gt_trim_mode].reg_saved[gn_sal_trim_ch][index], temp_gap);
                }
				print(LOG_LV_DEBUG, "closeset index : %u, gap : %u, reg : %u\r\n", found_closeset_index, found_closeset_gap, gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch]);
                gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] = gt_sal_trim_param[gt_trim_mode].reg_saved[gn_sal_trim_ch][found_closeset_index];
                sal_trim_reg_write(gn_sal_trim_ch, gt_trim_mode, gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch]);
            }
            if (gt_trim_mode < TRIM_MODE_CURRENT_ITRIM)
            {
                print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
                ++gt_trim_mode;
                change_trim_step(TRIM_STEP_MODE_INIT);
            }
            else
            {
                ++gn_sal_trim_ch;
                if (gn_sal_trim_ch < SAL_CH_MAX)
                {
                    gt_sal_trim_param[gt_trim_mode].loop_cnt = 0;
                    gt_sal_trim_param[gt_trim_mode].saved_cnt = 0;
                    change_trim_step(TRIM_STEP_MODE_SET_CONDITION);
                }
                else
                {
                    print(LOG_LV_INFO, "===============%s DONE===============\r\n", gs_sal_trim_mode_str[gt_trim_mode]);
                    ++gt_trim_mode;
                    change_trim_step(TRIM_STEP_OTP_WRITE_INIT);
                }
            }
        }
        else if (gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] == TRIM_ADJ_FLAG_UP)
        {
            if (gt_trim_mode == TRIM_MODE_OSC_CTL)
            {
                gn_sal_osc_trim_val += (gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch] + 1);
                gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] = gn_sal_osc_trim_val;
            }
            else if (gt_trim_mode == TRIM_MODE_ADC_OFFSET)
            {
                gn_sal_adc_offset_trim_val -= gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch];
                gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] = gn_sal_adc_offset_trim_val;
            }
            else
            {
                gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] += gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch];
            }
            sal_trim_reg_write(gn_sal_trim_ch, gt_trim_mode, gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch]);
            change_trim_step(TRIM_STEP_MODE_SET_CONDITION);
        }
        else if (gt_sal_trim_param[gt_trim_mode].adjust_flag[gn_sal_trim_ch] == TRIM_ADJ_FLAG_DW)
        {
            if (gt_trim_mode == TRIM_MODE_OSC_CTL)
            {
                gn_sal_osc_trim_val -= (gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch] + 1);
                gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] = gn_sal_osc_trim_val;
            }
            else if (gt_trim_mode == TRIM_MODE_ADC_OFFSET)
            {
                gn_sal_adc_offset_trim_val += gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch];
                gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] = gn_sal_adc_offset_trim_val;
            }
            else
            {
                gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch] -= gt_sal_trim_param[gt_trim_mode].reg_step[gn_sal_trim_ch];
            }
            sal_trim_reg_write(gn_sal_trim_ch, gt_trim_mode, gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch]);
            change_trim_step(TRIM_STEP_MODE_SET_CONDITION);
        }
        break;
    case TRIM_STEP_OTP_WRITE_INIT :
        print(LOG_LV_INFO, "%s ----- TRIM FOUND DONE ----- %s \n\r", LOG_GREEN, LOG_NONE);
        sal_vcc_level_set(SAL_VCC_5V5);
        gn_trim_delay = 100;

#if (TRIM_TYPE == TRIM_WAFER)
        gt_sal_trim_regs._rF1.pwm_max_r = 0;
        gt_sal_trim_regs._rF2.pwm_max_g = 0;
        gt_sal_trim_regs._rF3.pwm_max_b = 0;
#else

#if (LED_TYPE == LED_EP)
        gt_sal_trim_regs._rF1.pwm_max_r = 0xAA3;
        gt_sal_trim_regs._rF2.pwm_max_g = 0x933;
        gt_sal_trim_regs._rF3.pwm_max_b = 0x429;
#else
        gt_sal_trim_regs._rF1.pwm_max_r = 0xAA3;
        gt_sal_trim_regs._rF2.pwm_max_g = 0x933;
        gt_sal_trim_regs._rF3.pwm_max_b = 0x466;
#endif

#endif
        gt_sal_trim_regs._rF1.pwm_max_r = 2418;
        gt_sal_trim_regs._rF2.pwm_max_g = 3413;
        gt_sal_trim_regs._rF3.pwm_max_b = 3990;

        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR14;
        i2c_info.data = gt_sal_trim_regs._rF1.val;
        sal_write_reg_i2c(&i2c_info);

        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR15;
        i2c_info.data = gt_sal_trim_regs._rF2.val;
        sal_write_reg_i2c(&i2c_info);

        i2c_info.reg_addr = SAL_TRIM_ADDR_OTP2_MIRROR16;
        i2c_info.data = gt_sal_trim_regs._rF3.val;
        sal_write_reg_i2c(&i2c_info);

#if (OTP_WRITE_EN == OTP_WRITE_TRUE)
        change_trim_step(TRIM_STEP_OTP_START);
        print(LOG_LV_INFO, "OTP Burn Start!!! \n\r");
#else
        change_trim_step(TRIM_STEP_DONE);
        print(LOG_LV_INFO, "OTP Burn Skip!!! \n\r");
#endif
//-------------------------------------------------------------
        //change_trim_step(TRIM_STEP_PWR_OFF);
//-------------------------------------------------------------
        break;
    case TRIM_STEP_OTP_START :
        if (gn_trim_delay)
        {
            --gn_trim_delay;
        }
        else
        {
#if (TRIM_TYPE == TRIM_WAFER)
            gt_sal_trim_regs._rE1.otp_pg1 = 1;
            gt_sal_trim_regs._rE1.otp_pg2 = 0;
#else
            gt_sal_trim_regs._rE1.otp_pg1 = 1;
            gt_sal_trim_regs._rE1.otp_pg2 = 1;
#endif
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP_CONTROL1;
            i2c_info.data = gt_sal_trim_regs._rE1.val;
            sal_write_reg_i2c(&i2c_info);

            gn_trim_delay = 100;
            change_trim_step(TRIM_STEP_OTP_DONE);
        }
        break;
    case TRIM_STEP_OTP_DONE :
        if (gn_trim_delay)
        {
            --gn_trim_delay;
        }
        else
        {
            gt_sal_trim_regs._rE1.otp_rd = 1;
            i2c_info.reg_addr = SAL_TRIM_ADDR_OTP_CONTROL1;
            i2c_info.data = gt_sal_trim_regs._rE1.val;
            sal_write_reg_i2c(&i2c_info);
            change_trim_step(TRIM_STEP_OTP_VERIFY);
        }
        break;
    case TRIM_STEP_OTP_VERIFY :
        if (gt_otp_verify_step == OTP_VERIFY_PWR_OFF)
        {
            sal_vled_en(PWR_OFF);
            sal_vcc_level_set(SAL_VCC_5V0);
            sal_vcc_en(PWR_OFF);
            HAL_I2C_DeInit(&hi2c1);
            gt_otp_verify_step = OTP_VERIFY_PWR_ON;
            // proper delay
            gn_trim_delay = 100;
        }
        else if (gt_otp_verify_step == OTP_VERIFY_PWR_ON)
        {
            if (gn_trim_delay)
            {
                --gn_trim_delay;
            }
            else
            {
                sal_make_mcu_mode();
                sal_vcc_level_set(SAL_VCC_5V0);
                sal_vcc_en(PWR_ON);
                sal_vled_en(PWR_ON);
                gt_otp_verify_step = OTP_VERIFY_REG_READ;
                // proper delay
                gn_trim_delay = 100;
            }
        }
        else if (gt_otp_verify_step == OTP_VERIFY_REG_READ)
        {
            if (gn_trim_delay)
            {
                --gn_trim_delay;
                if (gn_trim_delay == 50)
                {
                    sal_make_i2c_mode();
                }
            }
            else
            {
                sal_trim_test_enable();
                sal_trim_otp_protection(OTP_PROTECT_OFF);
                if (!(sal_trim_reg_verify()))
                {
                    change_trim_step(TRIM_STEP_DONE);
                }
                else
                {
                    gt_trim_error = TRIM_ERROR_OTP_VERIFY;
                    change_trim_step(TRIM_STEP_ERROR);
                }
            }
        }
        break;
    case TRIM_STEP_DONE :
        change_trim_step(TRIM_STEP_PWR_OFF);
        break;
    case TRIM_STEP_ERROR :
        print(LOG_LV_ERROR, "%s===============TRIM ERROR===============\r\n [%s, %u, %u] - [%s]%s\r\n",\
        LOG_RED, gs_sal_trim_mode_str[gt_trim_mode], gn_sal_trim_ch, gt_sal_trim_param[gt_trim_mode].reg_now[gn_sal_trim_ch], gs_sal_trim_otp_error_str[gt_trim_error], LOG_NONE);
        gt_trim_error = TRIM_ERROR_NONE;
        change_trim_step(TRIM_STEP_PWR_OFF);
        break;
    case TRIM_STEP_PWR_OFF :
        sal_vled_en(PWR_OFF);
        sal_vcc_level_set(SAL_VCC_5V0);
        sal_vcc_en(PWR_OFF);
        decode_mode_set(DECODE_NONE);
        clear_trim_parameters();

        HAL_I2C_DeInit(&hi2c1);
        break;
    default :
        break;
    }
}
#endif
/* END - INTERFACE FUNCTIONS ************************************************************************/

/*** end of file ***/