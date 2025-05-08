/** @file JigBd_IF.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __JIG_BD_IF_C__
#include "config.h"

#define MCU_ADC_MEASURE_COUNT       (20)
#define MCU_ADC_VREF                (3.3f)
#define MCU_ADC_RESOLUTION          (4095.0f)

#define TIM1_PERIOD                 (143)
#define _LOGIC_0_                   (uint16_t)(((TIM1_PERIOD + 1) * 1.0f / 3.0f) - 1 + 0.5f)
#define _LOGIC_1_                   (uint16_t)(((TIM1_PERIOD + 1) * 2.0f / 3.0f) - 1 + 0.5f)

#define BIT_1_RATIO                 (2.0f)
#define BIT_0_RATIO                 (1.0f)
#define BIT_RATIO_SUM               (3.0f)

#define D_ERR_RATE                  (0.3)
#define XD_LOGIC_0_DUTY             ((double)XD_SERIAL_CLK_CNT_LOW / (XD_SERIAL_CLK_CNT_HIGH + XD_SERIAL_CLK_CNT_LOW))
#define XD_LOGIC_1_DUTY             ((double)XD_SERIAL_CLK_CNT_HIGH / (XD_SERIAL_CLK_CNT_HIGH + XD_SERIAL_CLK_CNT_LOW))

#define LOGIC_0_COUNT_MIN(N)        ((uint16_t)((N) * (XD_LOGIC_0_DUTY * (1 - D_ERR_RATE))))
#define LOGIC_0_COUNT_MAX(N)        ((uint16_t)((N) * (XD_LOGIC_0_DUTY * (1 + D_ERR_RATE))))
#define LOGIC_1_COUNT_MIN(N)        ((uint16_t)((N) * (XD_LOGIC_1_DUTY * (1 - D_ERR_RATE))))
#define LOGIC_1_COUNT_MAX(N)        ((uint16_t)((N) * (XD_LOGIC_1_DUTY * (1 + D_ERR_RATE))))

#define SERIAL_DECODE_MASK_CODE     (0x0F)
#define SERIAL_DECODE_MASK_ID       (0x1F)
#define SERIAL_DECODE_MASK_DATA     (0xFFF)

#define XD_TIMEOUT_MS               (2)

static uint16_t gn_mcu_adc_value;

uint16_t gn_serialize_tx_buffer[400];
uint16_t gn_serialize_rx_risingBuffer[400];
uint16_t gn_serialize_rx_fallingBuffer[400];

volatile uint16_t gn_xd_rx_timeout;
static volatile bool gb_xd_timeout_event;

volatile bool gb_pwm_dma_tx_flag;

static double gf_xdic_internal_freq_Hz;
static double gf_freq_min;
static double gf_freq_max;

bool gb_timer_input_capture_activated;
bool gb_timer_input_capture_done;
static uint32_t gn_xdic_input_capture_cnt[FREQ_IN_IC_LENGTH];

static uint8_t gn_xdic_dimming_channel = 0;

void us_delay(uint16_t us_delay)
{
    LL_TIM_EnableCounter(TIM12);
    while(LL_TIM_GetCounter(TIM12) < us_delay)
    {
    }
    LL_TIM_DisableCounter(TIM12);
    LL_TIM_SetCounter(TIM12, 0);
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
        LL_GPIO_SetOutputPin(XC24_VCC_EN_GPIO_Port, XC24_VCC_EN_Pin);
    }
    else
    {
        LL_GPIO_ResetOutputPin(XC24_VCC_EN_GPIO_Port, XC24_VCC_EN_Pin);
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
    else if (pwr == PWR_ON_5V7) /*5.0V off, 5.7V On */
    {
        pin_mask |= (XDIC_5_7V_Pin <<  0);   /* set */
    }

    WRITE_REG(XDIC_5_7V_GPIO_Port->BSRR, pin_mask);
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

/*
CTL_UPPER_GAIN
 - HIGH : Sensing range  140mA
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
    case CH_01 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_LO();
        break;
    case CH_02 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_LO();
        break;
    case CH_03 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_LO();
        break;
    case CH_04 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_LO();
        break;
    case CH_05 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_LO();
        break;
    case CH_06 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_LO();
        break;
    case CH_07 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_LO();
        break;
    case CH_08 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_HI();
        ENABLE_SELECT4_LO();
        break;
    case CH_09 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_HI();
        break;
    case CH_10 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_LO();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_HI();
        break;
    case CH_11 :
        ENABLE_SELECT1_LO();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_HI();
        break;
    case CH_12 :
        ENABLE_SELECT1_HI();
        ENABLE_SELECT2_HI();
        ENABLE_SELECT3_LO();
        ENABLE_SELECT4_HI();
        break;
    case CH_MAX :
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
void JigBD_IF_VREF_ADC_StartStop(void)
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
    print(LOG_DEBUG, "JigBD_IF_VREF_ADC_Get()-%d\r\n", gn_mcu_adc_value);
}

uint16_t JigBD_IF_VREF_ADC_Get(void)
{
    return gn_mcu_adc_value;
}

uint16_t JigBD_IF_Convert_Volt_to_VREF_ADC(double in_volt)
{
    uint16_t ret = (uint16_t)(((MCU_ADC_RESOLUTION * in_volt) / MCU_ADC_VREF) + 0.5f);
    return ret;
}

double JigBD_IF_Convert_VREF_ADC_to_Volt(uint16_t in_adc)
{
    double ret = (double)(((double)in_adc * MCU_ADC_VREF) / MCU_ADC_RESOLUTION);
    return ret;
}
/* END - Internal ADC ******************************************/

/* BEGIN - PWM Read Frequency ******************************************/
void JigBD_IF_Input_Capture_Start(void)
{
    gf_xdic_internal_freq_Hz = 0;
    gf_freq_min = DBL_MAX;
    gf_freq_max = 0;

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_2, (uint32_t)(&(TIM5->CCR1)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_2, (uint32_t)gn_xdic_input_capture_cnt);

    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_2, FREQ_IN_IC_LENGTH);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_2);

    LL_GPIO_ResetOutputPin(CNT_MR_GPIO_Port, CNT_MR_Pin);

    gb_timer_input_capture_activated = 1;
    gb_timer_input_capture_done = 0;
}

void JigBD_IF_Input_Capture_Stop(void)
{
    gb_timer_input_capture_activated = 0;
    gb_timer_input_capture_done = 0;

    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_2);
    LL_GPIO_SetOutputPin(CNT_MR_GPIO_Port, CNT_MR_Pin);
}

uint16_t JigBD_IF_Input_Capture_Get_Freq(void)
{
    uint32_t input_freq = (uint32_t)(gf_xdic_internal_freq_Hz + 0.5f);
    if (input_freq > 65535) //2^16-1
    {
        print(LOG_ERROR, "\r\nERROR: JigBD_IF_Input_Capture_Get_Freq() Retrun[%d] is TOO BIG\r\n", input_freq);
        return 0;
    }
    //print(LOG_DEBUG, "\r\n JigBD_IF_Input_Capture_Get_Freq() : %d", input_freq);
    return (uint16_t)input_freq;
}

void JigBD_IF_Input_Capture_Calculate_Freq(void)
{
    if (gb_timer_input_capture_activated == 1)
    {
        double f_freq = 0;
        double f_freq_avg = 0;
        uint32_t delta = 0;
        uint32_t n_count = 0;

        for (uint32_t i = 1 ; i < (FREQ_IN_IC_LENGTH - 1) ; ++i)
        {
            if (gn_xdic_input_capture_cnt[i + 1] > gn_xdic_input_capture_cnt[i + 0])
            {
                delta = (gn_xdic_input_capture_cnt[i + 1] - gn_xdic_input_capture_cnt[i + 0]);
            }
            else
            {
                delta = (0xFFFFFFFF - gn_xdic_input_capture_cnt[i + 0]) + gn_xdic_input_capture_cnt[i + 1] + 1;
            }

            f_freq = (TIM5_FREQ / delta);
            f_freq_avg += f_freq;

            ++n_count;
        }

        gf_xdic_internal_freq_Hz = (f_freq_avg / n_count);
        if (gf_freq_min > gf_xdic_internal_freq_Hz)
        {
            gf_freq_min = gf_xdic_internal_freq_Hz;
        }

        if (gf_freq_max < gf_xdic_internal_freq_Hz)
        {
            gf_freq_max = gf_xdic_internal_freq_Hz;
        }
    }
}

uint16_t JigBD_IF_Freq_ConvertByPrescaler(double in_freq) //input must be 'MHz'
{
    // Convert to 'Hz' and Divide by TIM_CAPTURE_EXT_PRESCALER
    uint32_t u32_rtn_val = (uint32_t)(((in_freq * CONST_MHz_TO_Hz) / TIM_CAPTURE_EXT_PRESCALER) + 0.5f);

    if (u32_rtn_val > 65535) //2^16-1
    {
        print(LOG_ERROR, "\r\nERROR: JigBD_IF_Freq_ConvertByPrescaler(%lf) INPUT TOO BIG\r\n", in_freq);
        return 0;
    }

    if (u32_rtn_val < 1374) // 1374 = ( 90Mhz / 2^16 ) + 1;
    {
        print(LOG_ERROR, "\r\nERROR: JigBD_IF_Freq_ConvertByPrescaler(%lf) INPUT TOO SMALL\r\n", in_freq);
        return 0;
    }

    return (uint16_t)u32_rtn_val;
}

double JigBD_IF_Freq_Count_to_MHZ(uint16_t count)
{
    double rtn_freq = 0.0;
    if (count >= 65535) //2^16-1
    {
        print(LOG_ERROR, "\r\nERROR: JigBD_IF_Input_Capture_Get_Freq() Retrun[%d] is TOO BIG\r\n", count);
        return 0;
    }

    rtn_freq = ((double)count * TIM_CAPTURE_EXT_PRESCALER) / CONST_MHz_TO_Hz;
    return rtn_freq;
}
/* END - PWM Read Frequency ******************************************/

/* BEGIN - Make XDIC DATA SIGNAL through PWM DMA *******************************************/
__STATIC_INLINE uint16_t Get_Nth_Bit(uint16_t x, int n)
{
    return ((x & (1 << (n - 1))) >> (n - 1));
}

__STATIC_INLINE void Serialize_Tx_Start(uint32_t len)
{
    gb_pwm_dma_tx_flag = TRUE;

    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, len);
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_1);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);
    LL_TIM_EnableCounter(TIM1);
}

__STATIC_INLINE void Serialize_Tx_Done(void)
{
    LL_TIM_DisableCounter(TIM1);
    LL_DMA_ClearFlag_TC1(DMA2);
    LL_DMA_DisableIT_TC(DMA2, LL_DMA_STREAM_1);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);
}

__STATIC_INLINE void Serialize_Rx_Start(uint32_t len)
{
    gn_xd_rx_timeout = XD_TIMEOUT_MS;
    gb_xd_timeout_event = false;

    /* output enable set HIGH */
    PWM_SWITCH_HI();

    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, len);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, len);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);

    LL_TIM_EnableDMAReq_CC1(TIM2);
    LL_TIM_EnableDMAReq_CC2(TIM2);

    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    while(LL_DMA_IsActiveFlag_TC6(DMA1) == 0)
    {
        if (gn_xd_rx_timeout == 0)
        {
            gb_xd_timeout_event = true;
            break;
        }
    }
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, 0);
    LL_DMA_ClearFlag_TC6(DMA1);

    while(LL_DMA_IsActiveFlag_TC5(DMA1) == 0)
    {
        if (gn_xd_rx_timeout == 0)
        {
            gb_xd_timeout_event = true;
            break;
        }
    }
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, 0);
    LL_DMA_ClearFlag_TC5(DMA1);

    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_TIM_DisableDMAReq_CC1(TIM2);
    LL_TIM_DisableDMAReq_CC2(TIM2);

    /* output enable set LOW */
    PWM_SWITCH_LO();
}

__STATIC_INLINE float Decode_Input_Response(uint32_t* pdata, uint16_t len)
{
    uint32_t freq_count_sum = 0;
    uint32_t count = 0;

    float f_tim1_count_avg = 0;
    float f_input_frequency = 0;
    uint16_t logic_1_duty_cnt_min = 0;
    uint16_t logic_1_duty_cnt_max = 0;

    uint16_t* p_rising = gn_serialize_rx_risingBuffer;
    uint16_t* p_falling = gn_serialize_rx_fallingBuffer;

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
    if (in_channel < (XD_CH_SIZE + 1))
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

    for (uint8_t i = 0 ; i < XD_DAISY_SIZE ; ++i)
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

    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    us_delay(XDIC_RESET_DELAY);
}

static uint16_t MCU_IF_Read_XDIC(uint8_t in_addr)
{
    uint32_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;   /* dummy */

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XD_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;

        //bit5 ~ bit0 : addr[5:0]
        for (uint8_t j = 0 ; j < SERIAL_ADDR_SIZE ; ++j)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_addr, SERIAL_ADDR_SIZE - j)) ? bit_1 : bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0; //Make Signal End LOW.

    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();

    Serialize_Rx_Start(XDIC_READ_RECV_BITS);

    uint32_t n_response = 0;

    if (gb_xd_timeout_event)
    {
        print(LOG_ERROR, "%sRx Timeout!!!\r\n%s", ANSI_FONT_RED, ANSI_FONT_NONE);
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

void MCU_IF_Write_LD(uint16_t in_LD_data)
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;   /* dummy */

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XD_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;

        //LD[15:0]
        for (uint8_t j = 0 ; j < XD_CH_SIZE ; ++j)
        {
            bool write_data = (gn_xdic_dimming_channel == 0) || ((gn_xdic_dimming_channel - 1) == j);

            for (uint8_t i = 0; i < SERIAL_LD_SIZE; ++i)
            {
                if (write_data)
                {
                    gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_LD_data, SERIAL_LD_SIZE - i)) ? bit_1 : bit_0);
                }
                else
                {
                    gn_serialize_tx_buffer[pwm_length++] = bit_0;
                }
            }
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0; //Make Signal End LOW.

    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    us_delay(XDIC_LD_TRANS_DELAY);
}

static uint16_t MCU_IF_Fault_Read_Command(void)
{
    uint32_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;   /* dummy */

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    gn_serialize_tx_buffer[pwm_length++] = bit_1;
    gn_serialize_tx_buffer[pwm_length++] = bit_0;
    gn_serialize_tx_buffer[pwm_length++] = bit_1;
    gn_serialize_tx_buffer[pwm_length++] = bit_0;

    gn_serialize_tx_buffer[pwm_length++] = 0; //Make Signal End LOW.

    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();

    Serialize_Rx_Start(XDIC_FAULT_RECV_BITS);

    uint32_t n_response = 0;

    if (gb_xd_timeout_event)
    {
        print(LOG_ERROR, "%sRx Timeout!!!\r\n%s", ANSI_FONT_RED, ANSI_FONT_NONE);
    }
    else
    {
        float f_frequency = Decode_Input_Response(&n_response, XDIC_FAULT_RECV_BITS);
        print(LOG_DEBUG, "FAULT DATA(0x%02X):[%1.3fMHz, CODE : 0x%01X, DATA - 0x%01X]\r\n",\
        n_response, f_frequency, ((n_response >> 4) & 0x0F), ((n_response >> 0) & 0x0F));
    }

    us_delay(10);

    return (uint16_t)(n_response & 0x0FFF);
}


static void MCU_IF_IdGen_Command()
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;   /* dummy */

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XD_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0; //Make Signal End LOW.

    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    us_delay(XDIC_IDGEN_DELAY);
}

static void MCU_IF_SyncGen_Command()
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;   /* dummy */

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XD_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0; //Make Signal End LOW.

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

void JigBD_IF_Write_LD_Command(uint16_t in_LD_data)
{
    if (IS_XC24_Support())
    {
        XC24_IF_Write_LD(in_LD_data);
    }
    else
    {
        MCU_IF_Write_LD(in_LD_data);
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
