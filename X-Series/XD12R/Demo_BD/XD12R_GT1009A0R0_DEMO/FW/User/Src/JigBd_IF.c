/** @file JigBd_IF.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __JIG_BD_IF_C__

#include "main.h"
#include "config.h"
#include "xdic.h"
#include "xc24.h"
#include "types.h"
#include "JigBd_IF.h"
#include "ads124s08.h"

#define MCU_ADC_MEASURE_COUNT       (20)
#define MCU_ADC_VREF                (3.3f)
#define MCU_ADC_RESOLUTION          (4095.0f)

#define TIM1_PERIOD                 (179)
#define _LOGIC_0_                   (uint16_t)(((TIM1_PERIOD + 1) * 1.0f / 3.0f) - 1 + 0.5f)
#define _LOGIC_1_                   (uint16_t)(((TIM1_PERIOD + 1) * 2.0f / 3.0f) - 1 + 0.5f)

#define BIT_1_RATIO                 (2.0f)
#define BIT_0_RATIO                 (1.0f)
#define BIT_RATIO_SUM               (3.0f)

#define D_ERR_RATE                  (0.3)
#define XD_LOGIC_0_DUTY             ((double)XDIC_SERIAL_CLK_CNT_LOW / (XDIC_SERIAL_CLK_CNT_HIGH + XDIC_SERIAL_CLK_CNT_LOW))
#define XD_LOGIC_1_DUTY             ((double)XDIC_SERIAL_CLK_CNT_HIGH / (XDIC_SERIAL_CLK_CNT_HIGH + XDIC_SERIAL_CLK_CNT_LOW))

#define LOGIC_0_COUNT_MIN(N)        ((uint16_t)((N) * (XD_LOGIC_0_DUTY * (1 - D_ERR_RATE))))
#define LOGIC_0_COUNT_MAX(N)        ((uint16_t)((N) * (XD_LOGIC_0_DUTY * (1 + D_ERR_RATE))))
#define LOGIC_1_COUNT_MIN(N)        ((uint16_t)((N) * (XD_LOGIC_1_DUTY * (1 - D_ERR_RATE))))
#define LOGIC_1_COUNT_MAX(N)        ((uint16_t)((N) * (XD_LOGIC_1_DUTY * (1 + D_ERR_RATE))))

#define SERIAL_DECODE_MASK_CODE     (0x0F)
#define SERIAL_DECODE_MASK_ID       (0x1F)
#define SERIAL_DECODE_MASK_DATA     (0xFFF)

static uint16_t gn_mcu_adc_value;

static uint16_t gn_serialize_tx_buffer[4000] = {0, };
static uint16_t gn_serialize_rx_risingBuffer[400] = {0, };
static uint16_t gn_serialize_rx_fallingBuffer[400] = {0, };

volatile uint16_t gn_xd_rx_timeout;
volatile bool gb_xd_timeout_event;

volatile bool gb_pwm_dma_tx_flag;
volatile bool gb_pwm_is_rx_flag;

static double gf_internal_freq_Hz;

bool gb_timer_input_capture_activated;
volatile bool gb_timer_input_capture_done;
static uint32_t gn_input_capture_cnt[FREQ_IN_IC_LENGTH];

static uint8_t gn_xdic_dimming_channel = 0;

void us_delay(uint16_t us_delay)
{
    uint16_t start = LL_TIM_GetCounter(TIM12);
    while ((uint16_t)(LL_TIM_GetCounter(TIM12) - start) < us_delay)
    {
    }
}

void JigBD_IF_Detect_XC24(void)
{
    if (IS_XC24_Support())
    {
        XC24_Init();
    }
}

void JigBD_IF_XC_VCC_EN(uint8_t on)
{
    if (on == PWR_ON)
    {
        LL_GPIO_ResetOutputPin(XC24_VCC_EN_GPIO_Port, XC24_VCC_EN_Pin);
    }
    else
    {
        if (IS_XC24_Support())
        {
            XC24_Write_Register(XC24_ADDR_GLOBAL_WRITE_DATA, 0x00);
        }
        XC_NSCS_LO();
        XC24_Start_MCLK_Oscillation(false);
        LL_GPIO_SetOutputPin(XC24_VCC_EN_GPIO_Port, XC24_VCC_EN_Pin);
    }
    LL_mDelay(10);
}

void JigBD_IF_XD_VCC_EN(uint8_t on)
{
    if (on == PWR_ON)
    {
        /* VCC_EN : ON, LOW */
        LL_GPIO_ResetOutputPin(XDIC_VCC_EN_GPIO_Port, XDIC_VCC_EN_Pin);
    }
    else
    {
        /* VCC_EN : OFF, HIGH */
        LL_GPIO_SetOutputPin(XDIC_VCC_EN_GPIO_Port, XDIC_VCC_EN_Pin);
    }
    LL_mDelay(10);
}

void JigBD_IF_XD_VCC_Level(power_volt_t pwr)
{
    uint32_t pin_mask = 0;
    if (pwr == PWR_ON_5V0) /*5.0V on, 5.7V off */
    {
        pin_mask |= (XDIC_5_7V_Pin << 16);   /* reset */
    }
    else if (pwr == PWR_ON_5V5) /*5.0V off, 5.7V On */
    {
        pin_mask |= (XDIC_5_7V_Pin <<  0);   /* set */
    }

    WRITE_REG(XDIC_5_7V_GPIO_Port->BSRR, pin_mask);
    LL_mDelay(10);
}

void JigBD_IF_XC_VCC_Level(power_volt_t pwr)
{
    uint32_t pin_mask = 0;
    if (pwr == PWR_ON_5V0) /*5.0V on, 5.7V off */
    {
        pin_mask |= (XC24_5V5_Pin << 16);   /* reset */
    }
    else if (pwr == PWR_ON_5V5) /*5.0V off, 5.7V On */
    {
        pin_mask |= (XC24_5V5_Pin <<  0);   /* set */
    }

    WRITE_REG(XC24_5V5_GPIO_Port->BSRR, pin_mask);
    LL_mDelay(10);
}

void JigBD_IF_VLED_9V_EN(uint8_t on)
{
    if (on == PWR_ON)
    {
        LL_GPIO_ResetOutputPin(VLED_CTR_9V_GPIO_Port, VLED_CTR_9V_Pin);
    }
    else
    {
        LL_GPIO_SetOutputPin(VLED_CTR_9V_GPIO_Port, VLED_CTR_9V_Pin);
    }
    LL_mDelay(10);
}

float JigBD_IF_XD_ICC(void)
{
    float icc = 0;
    uint16_t icc_adc[2] = {0, };

    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P);
    ADS114S08_Set_Start(1);
    ADS114S08_Wait_Done();
    icc_adc[0] = ADS114S08_Get_ADC_Value();

    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_N);
    ADS114S08_Set_Start(1);
    ADS114S08_Wait_Done();
    icc_adc[1] = ADS114S08_Get_ADC_Value();

    if (icc_adc[0] > icc_adc[1])
    {
        icc = ((icc_adc[0] - icc_adc[1]) * ADC_VOLT_PER_STEP) / CURRENT_SENSE_R_ICC;
    }
    else
    {
        icc = ((icc_adc[1] - icc_adc[0]) * ADC_VOLT_PER_STEP) / CURRENT_SENSE_R_ICC;
    }
    print(LOG_INFO, "\r\n XD ICC Current : %.3f [mA]\r\n", icc);

    // XD12 : 2.7mA / XD04 : 1.7mA
    return icc;
}

float JigBD_IF_XC_ICC(void)
{
    float icc = 0;
    uint16_t icc_adc[2] = {0, };

    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_ICC_P);
    ADS114S08_Set_Start(1);
    ADS114S08_Wait_Done();
    icc_adc[0] = ADS114S08_Get_ADC_Value();

    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_ICC_N);
    ADS114S08_Set_Start(1);
    ADS114S08_Wait_Done();
    icc_adc[1] = ADS114S08_Get_ADC_Value();

    if (icc_adc[0] > icc_adc[1])
    {
        icc = ((icc_adc[0] - icc_adc[1]) * ADC_VOLT_PER_STEP) / CURRENT_SENSE_R_ICC;
    }
    else
    {
        icc = ((icc_adc[1] - icc_adc[0]) * ADC_VOLT_PER_STEP) / CURRENT_SENSE_R_ICC;
    }
    print(LOG_INFO, "\r\n XC ICC Current : %.3f [mA]\r\n", icc);
    return icc;
}

/*
CTL_UPPER_GAIN
 - HIGH : Sensing range   40mA
 - MID  : Sensing range   10mA
 - LOW  : Sensing range  0.5mA
*/
void JigBD_IF_Change_Current_Gain(current_gain_t gain)
{
    static current_gain_t t_gain;
    if (gain == GAIN_HIGH)
    {
        LL_GPIO_SetOutputPin(LTC_HIGH_CURRENT_GPIO_Port, LTC_HIGH_CURRENT_Pin);
        LL_GPIO_ResetOutputPin(LTC_MID_CURRENT_GPIO_Port, LTC_MID_CURRENT_Pin);
        LL_GPIO_ResetOutputPin(LTC_LOW_CURRENT_GPIO_Port, LTC_LOW_CURRENT_Pin);
    }
    else if (gain == GAIN_MID)
    {
        LL_GPIO_ResetOutputPin(LTC_HIGH_CURRENT_GPIO_Port, LTC_HIGH_CURRENT_Pin);
        LL_GPIO_SetOutputPin(LTC_MID_CURRENT_GPIO_Port, LTC_MID_CURRENT_Pin);
        LL_GPIO_ResetOutputPin(LTC_LOW_CURRENT_GPIO_Port, LTC_LOW_CURRENT_Pin);
    }
    else if (gain == GAIN_LOW)
    {
        LL_GPIO_ResetOutputPin(LTC_HIGH_CURRENT_GPIO_Port, LTC_HIGH_CURRENT_Pin);
        LL_GPIO_ResetOutputPin(LTC_MID_CURRENT_GPIO_Port, LTC_MID_CURRENT_Pin);
        LL_GPIO_SetOutputPin(LTC_LOW_CURRENT_GPIO_Port, LTC_LOW_CURRENT_Pin);
    }

    if (t_gain != gain)
    {
        LL_mDelay(10);
        t_gain = gain;
    }
}

void JigBD_IF_Select_Output_Ch(uint8_t in_output_ch)
{
    switch (in_output_ch)
    {
    case XD_CH_01 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_LO();
        break;
    case XD_CH_02 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_LO();
        break;
    case XD_CH_03 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_LO();
        break;
    case XD_CH_04 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_LO();
        break;
    case XD_CH_05 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_LO();
        break;
    case XD_CH_06 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_LO();
        break;
    case XD_CH_07 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_LO();
        break;
    case XD_CH_08 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_LO();
        break;
    case XD_CH_09 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_HI();
        break;
    case XD_CH_10 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_HI();
        break;
    case XD_CH_11 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_HI();
        break;
    case XD_CH_12 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_HI();
        break;
    case XD_CH_MAX :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_HI();
        break;
    default :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_HI();
        print(LOG_ERROR, "\r\nERROR:JigBD_IF_Select_Output_Ch(%d)-INPUT WRONG!!\r\n", in_output_ch);
        break;
    }
}

/* BEGIN - Internal ADC ******************************************/
void JigBD_IF_Start_MCU_ADC(void)
{
    uint32_t u32_InternalADC = 0;
    for (uint8_t cnt = 0 ; cnt < MCU_ADC_MEASURE_COUNT ; ++cnt)
    {
        LL_ADC_REG_StartConversionSWStart(ADC1);
        while(!LL_ADC_IsActiveFlag_EOCS(ADC1)) {}
        LL_ADC_ClearFlag_EOCS(ADC1);
        u32_InternalADC += LL_ADC_REG_ReadConversionData12(ADC1);
    }
    gn_mcu_adc_value = (uint16_t)(((float)u32_InternalADC / MCU_ADC_MEASURE_COUNT) + 0.5f);
    print(LOG_DEBUG, "JigBD_IF_Get_MCU_ADC()-%d\r\n", gn_mcu_adc_value);
}

uint16_t JigBD_IF_Get_MCU_ADC(void)
{
    return gn_mcu_adc_value;
}

uint16_t JigBD_IF_Convert_Volt_To_MCU_ADC(double in_volt)
{
    uint16_t ret = (uint16_t)(((MCU_ADC_RESOLUTION * in_volt) / MCU_ADC_VREF) + 0.5f);
    return ret;
}

double JigBD_IF_Convert_MCU_ADC_To_Volt(uint16_t in_adc)
{
    double ret = (double)(((double)in_adc * MCU_ADC_VREF) / MCU_ADC_RESOLUTION);
    return ret;
}
/* END - Internal ADC ******************************************/

/* BEGIN - PWM Read Frequency ******************************************/
void JigBD_IF_Link_DMA_With_Buffer(void)
{
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)(&(TIM1->CCR1)));
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)gn_serialize_tx_buffer);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)(&(TIM2->CCR2)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)gn_serialize_rx_risingBuffer);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)(&(TIM2->CCR1)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)gn_serialize_rx_fallingBuffer);
}

void JigBD_IF_Start_Input_Capture(void)
{
    gf_internal_freq_Hz = 0.0f;

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_2, (uint32_t)(&(TIM5->CCR1)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_2, (uint32_t)gn_input_capture_cnt);

    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_2, FREQ_IN_IC_LENGTH);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_2);

    LL_GPIO_ResetOutputPin(CNT_MR_GPIO_Port, CNT_MR_Pin);

    gb_timer_input_capture_activated = 1;
    gb_timer_input_capture_done = 0;
}

void JigBD_IF_Wait_Input_Capture_Done(void)
{
    while(1)
    {
        if (gb_timer_input_capture_done)
        {
            break;
        }
    }
}

void JigBD_IF_Stop_Input_Capture(void)
{
    gb_timer_input_capture_activated = 0;
    gb_timer_input_capture_done = 0;

    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_2);
    LL_GPIO_SetOutputPin(CNT_MR_GPIO_Port, CNT_MR_Pin);
}

double JigBD_IF_Get_Input_Capture_Freq(void)
{
    return gf_internal_freq_Hz;
}

void JigBD_IF_Calculate_Input_Capture_Freq(void)
{
    if (gb_timer_input_capture_activated == 1)
    {
        double f_freq_avg = 0;
        uint32_t n_count = 0;

        for (uint32_t i = 1 ; i < (FREQ_IN_IC_LENGTH - 1) ; ++i)
        {
            uint32_t delta = 0;
            if (gn_input_capture_cnt[i + 1] > gn_input_capture_cnt[i + 0])
            {
                delta = (gn_input_capture_cnt[i + 1] - gn_input_capture_cnt[i + 0]);
            }
            else
            {
                delta = (0xFFFFFFFF - gn_input_capture_cnt[i + 0]) + gn_input_capture_cnt[i + 1] + 1;
            }

            f_freq_avg += (TIM5_FREQ / delta);
            ++n_count;
        }

        gf_internal_freq_Hz = (f_freq_avg / n_count);
    }
}

uint16_t JigBD_IF_Calculate_XDIC_Divided_Freq(double in_freq) //input must be 'MHz'
{
    uint32_t u32_rtn_val = (uint32_t)(((in_freq * CONST_MHz_TO_Hz) / XDIC_CONST_FREQ_DIVIDE) + 0.5f);

    if (u32_rtn_val > 65535) //2^16-1
    {
        print(LOG_ERROR, "\r\nERROR: %s (%lf) INPUT TOO BIG\r\n", __func__, in_freq);
        return 0;
    }

    return (uint16_t)u32_rtn_val;
}

double JigBD_IF_Reconvert_XDIC_Original_Freq(double count)
{
    double rtn_freq = 0.0;

    rtn_freq = (count * XDIC_CONST_FREQ_DIVIDE) / CONST_MHz_TO_Hz;
    return rtn_freq;
}
/* END - PWM Read Frequency ******************************************/

/* BEGIN - Make XDIC DATA SIGNAL through PWM DMA *******************************************/
static uint16_t Get_Nth_Bit(uint64_t x, int n)
{
    return (x >> (n - 1)) & 1ULL;
}

static float Decode_Input_Response(uint32_t* pdata, uint16_t len)
{
    uint32_t freq_count_sum = 0;
    uint32_t count = 0;

    float f_tim1_count_avg = 0;
    float f_input_frequency = 0;
    uint16_t logic_1_duty_cnt_min = 0;
    uint16_t logic_1_duty_cnt_max = 0;

    const uint16_t* p_rising = gn_serialize_rx_risingBuffer;
    const uint16_t* p_falling = gn_serialize_rx_fallingBuffer;

    for (uint16_t i = 1 ; i < len ; ++i)
    {
        freq_count_sum += p_rising[i];
        ++count;
    }

    if (count)
    {
        f_tim1_count_avg = ((float)freq_count_sum / count);

        logic_1_duty_cnt_min = (uint16_t)LOGIC_1_COUNT_MIN(f_tim1_count_avg);
        logic_1_duty_cnt_max = (uint16_t)LOGIC_1_COUNT_MAX(f_tim1_count_avg);
    }

    if (pdata)
    {
        uint32_t n_xdic_response = 0;

        for (volatile uint16_t j = 0 ; j < len ; ++j)
        {
            if ((p_falling[j] > logic_1_duty_cnt_min) && (p_falling[j] < logic_1_duty_cnt_max))
            {
                n_xdic_response |= (1U << ((len - 1) - j));
            }
        }

        *pdata = n_xdic_response;
    }

    if (f_tim1_count_avg > 0)
    {
        f_input_frequency = (APB1_TIM_FREQ / f_tim1_count_avg);
    }

    return f_input_frequency;
}

void MCU_IF_Set_XDIC_Channel(uint8_t in_channel)
{
    if (in_channel < (XDIC_CH_SIZE + 1))
    {
        gn_xdic_dimming_channel = in_channel;
    }
    else
    {
        gn_xdic_dimming_channel = 0;
    }
}

void MCU_IF_Write_XDIC(uint8_t in_addr, uint16_t in_data)
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;

        for (uint8_t j = 0 ; j < SERIAL_ADDR_SIZE ; ++j)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_addr, SERIAL_ADDR_SIZE - j)) ? bit_1 : bit_0);
        }

        for (uint8_t k = 0 ; k < SERIAL_DATA_SIZE ; ++k)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_data, SERIAL_DATA_SIZE - k)) ? bit_1 : bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    us_delay(XDIC_RESET_DELAY);
}

static uint16_t MCU_IF_Read_XDIC(uint8_t in_addr)
{
    uint32_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;

        for (uint8_t j = 0 ; j < SERIAL_ADDR_SIZE ; ++j)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_addr, SERIAL_ADDR_SIZE - j)) ? bit_1 : bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;
    gb_pwm_is_rx_flag = true;

    Serialize_Tx_Start(pwm_length);
    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    Serialize_Rx_Start(XDIC_READ_RECV_BITS);

    uint32_t n_response = 0;

    if (gb_xd_timeout_event)
    {
        print(LOG_ERROR, "Rx Timeout!!! [addr - 0x%02X]\r\n", in_addr);
        gb_xdic_initial_failed = true;
    }
    else
    {
        float f_frequency = Decode_Input_Response(&n_response, XDIC_READ_RECV_BITS);
        print(LOG_DEBUG, "Received DATA(0x%02X):[%1.3fMHz, CODE : 0x%01X, ID : 0x%02X, DATA - 0x%03X, FULL - 0x%04X]\r\n", in_addr, f_frequency,
            ((n_response >> 17) & SERIAL_DECODE_MASK_CODE), ((n_response >> 12) & SERIAL_DECODE_MASK_ID), ((n_response >> 0) & SERIAL_DECODE_MASK_DATA), n_response);
    }

    us_delay(XDIC_READ_DELAY);

    return (uint16_t)(n_response & SERIAL_DECODE_MASK_DATA);
}

void MCU_IF_Write_LD(uint16_t* p_in_LD_data)
{
    uint16_t pwm_length = 0;
    uint64_t LD_data_Even = p_in_LD_data[0];
    uint64_t LD_data_Odd = ((uint64_t)p_in_LD_data[1] << 14) | ((uint64_t)p_in_LD_data[2] << 0);

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;

        for (uint8_t j = 0 ; j < XDIC_CH_SIZE ; ++j)
        {
            bool write_data = (gn_xdic_dimming_channel == 0) || ((gn_xdic_dimming_channel - 1) == j);
            if (j % 2 == 0) // Even
            {
                for (uint8_t k = 0 ; k < SERIAL_EVEN_SIZE ; ++k)
                {
                    if (write_data)
                    {
                        gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint64_t)LD_data_Even, SERIAL_EVEN_SIZE - k)) ? bit_1 : bit_0);
                    }
                    else
                    {
                        gn_serialize_tx_buffer[pwm_length++] = bit_0;
                    }
                }
            }
            else // Odd
            {
                for (uint8_t k = 0 ; k < SERIAL_ODD_SIZE ; ++k)
                {
                    if (write_data)
                    {
                        gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint64_t)LD_data_Odd, SERIAL_ODD_SIZE - k)) ? bit_1 : bit_0);
                    }
                    else
                    {
                        gn_serialize_tx_buffer[pwm_length++] = bit_0;
                    }
                }
            }
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    us_delay(XDIC_LD_TRANS_DELAY);
}

static uint16_t MCU_IF_Fault_Read_Command(void)
{
    uint32_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();

    Serialize_Rx_Start(XDIC_FAULT_RECV_BITS);

    uint32_t n_response = 0;

    if (gb_xd_timeout_event)
    {
        print(LOG_ERROR, "Rx Timeout!!!\r\n");
    }
    else
    {
        float f_frequency = Decode_Input_Response(&n_response, XDIC_FAULT_RECV_BITS);
        print(LOG_DEBUG, "FAULT DATA(0x%02X):[%1.3fMHz, CODE : 0x%01X, DATA - 0x%01X]\r\n",\
        n_response, f_frequency, ((n_response >> 4) & 0x0F), ((n_response >> 0) & 0x0F));
    }

    us_delay(XDIC_FAULT_RECV_DELAY);

    return (uint16_t)(n_response & 0x0FFF);
}

static void MCU_IF_IdGen_Command()
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    us_delay(XDIC_IDGEN_DELAY);
}

static void MCU_IF_SyncGen_Command()
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    us_delay(XDIC_SYNCGEN_DELAY);
}

void JigBD_IF_Write_Command(uint8_t in_addr, uint16_t in_data)
{
    if (IS_XC24_Support())
    {
        XC24_IF_Write_XDIC(in_addr, in_data);
    }
    else
    {
        MCU_IF_Write_XDIC(in_addr, in_data);
    }
}

uint16_t JigBD_IF_Read_Command(uint8_t in_addr)
{
    uint16_t ret = 0;
    if (IS_XC24_Support())
    {
        ret = XC24_IF_Read_XDIC(in_addr);
    }
    else
    {
        ret = MCU_IF_Read_XDIC(in_addr);
    }
    return ret;
}

void JigBD_IF_Write_LD_Command(uint16_t* p_in_LD_data)
{
    if (IS_XC24_Support())
    {
        XC24_IF_Write_LD(p_in_LD_data);
    }
    else
    {
        MCU_IF_Write_LD(p_in_LD_data);
    }
}

uint16_t JigBD_IF_Fault_Read_Command(void)
{
    uint16_t ret = 0;
    if (IS_XC24_Support())
    {
        ret = XC24_IF_Fault_Read_Command();
    }
    else
    {
        ret = MCU_IF_Fault_Read_Command();
    }
    return ret;
}

void JigBD_IF_Reset_Command(void)
{
    uint16_t data = (1 << 11);
    if (IS_XC24_Support())
    {
        XC24_IF_Write_XDIC(XDIC_ADDR_RESET_ID, data);
    }
    else
    {
        MCU_IF_Write_XDIC(XDIC_ADDR_RESET_ID, data);
    }
}

void JigBD_IF_IdGen_Command(void)
{
    if (IS_XC24_Support())
    {
        XC24_IF_IdGen_Command();
    }
    else
    {
        MCU_IF_IdGen_Command();
    }
}

void JigBD_IF_SyncGen_Command(void)
{
    if (IS_XC24_Support())
    {
        XC24_IF_SyncGen_Command();
    }
    else
    {
        MCU_IF_SyncGen_Command();
    }
}

/*** end of file ***/