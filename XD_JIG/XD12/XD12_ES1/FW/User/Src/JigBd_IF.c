/** @file JigBd_IF.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __JJIGBD_IF_C__
#include "config.h"

#define VREF_ADC_REF_VOLT       3.3f
#define VREF_ADC_RESOLUTION     4095.0f

#define XD12_PWM_READ_TIMEOUT   (5)    /* ms */

#define PWM_DMA_DONE        1
#define IC_DMA_DONE         3
#define IDGEN_CMD_LENTH     4
#define READ_CMD_LENTH      10
#define WRITE_CMD_LENTH     22
#define READ_RECV_LENTH     21
#define CMD_LENTH_MAX       26

#define DIN_PERIOD          117

#define _D_0_               37
#define _D_1_               80

#define D_ERR_RATE          (0.4)
#define XD_LOGIC_1_DUTY     ((double)SERIAL_CLK_CNT_HIGH/(SERIAL_CLK_CNT_HIGH + SERIAL_CLK_CNT_LOW))
#define XD_LOGIC_0_DUTY     ((double)SERIAL_CLK_CNT_LOW/(SERIAL_CLK_CNT_HIGH + SERIAL_CLK_CNT_LOW))

#define D_0_MIN(N)          ((uint16_t)((N) * (XD_LOGIC_0_DUTY * (1 - D_ERR_RATE))))
#define D_0_MAX(N)          ((uint16_t)((N) * (XD_LOGIC_0_DUTY * (1 + D_ERR_RATE))))
#define D_1_MIN(N)          ((uint16_t)((N) * (XD_LOGIC_1_DUTY * (1 - D_ERR_RATE))))
#define D_1_MAX(N)          ((uint16_t)((N) * (XD_LOGIC_1_DUTY * (1 + D_ERR_RATE))))

static uint16_t gu16_InternalADC;

uint16_t gu16_pwm_tx_risingBuffer[36*12];

uint16_t gu16_pwm_rx_risingBuffer[36*12];
uint16_t gu16_pwm_rx_fallingBuffer[36*12];

uint8_t gu8_freq_input_capture_activated;

volatile uint8_t gu8_pwm_dma_flg;

void us_tdelay(uint16_t us_delay)
{
    LL_TIM_EnableCounter(TIM12);
    while(LL_TIM_GetCounter(TIM12) < us_delay)
    {
    }
    LL_TIM_DisableCounter(TIM12);
    LL_TIM_SetCounter(TIM12, 0);
}

void JigBd_IF_Init(void)
{
    if(IS_XC24())
    {
        XC24_Initialize();
    }
    LL_GPIO_ResetOutputPin(CNT_MR_GPIO_Port, CNT_MR_Pin);
}

void JigBd_IF_XC_VCC_EN(uint8_t on)
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

void JigBd_IF_XD_VCC_EN(uint8_t on)
{
    if(on == PWR_ON)
    {
        /* VCC_EN : ON, LOW */
        LL_GPIO_ResetOutputPin(XD12_VCC_EN_GPIO_Port, XD12_VCC_EN_Pin);
    }
    else
    {
        /* VCC_EN : OFF, HIGH */
        LL_GPIO_SetOutputPin(XD12_VCC_EN_GPIO_Port, XD12_VCC_EN_Pin);
    }
    LL_mDelay(10);
}

void JigBd_IF_XD_VCC_Level(power_vlot_t pwr)
{
    uint32_t pin_mask = 0;
    if(pwr == PWR_ON_5V0) /*5.0V on, 5.7V off */
    {
        pin_mask |= (XD12_5_7V_Pin << 16);   /* reset */
    }
    else if(pwr == PWR_ON_5V7) /*5.0V off, 5.7V On */
    {
        pin_mask |= (XD12_5_7V_Pin <<  0);   /* set */
    }

    WRITE_REG(XD12_5_7V_GPIO_Port->BSRR, pin_mask);
    LL_mDelay(10);
}

void JigBd_IF_VLED_9V_EN(uint8_t on)
{
    if(on == PWR_ON)
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
 - HIGH : Sensing range   70mA
 - MID  : Sensing range   10mA 
 - LOW  : Sensing range  0.5mA
*/
void JigBd_IF_change_current_gain(current_gain_t gain)
{
    static current_gain_t t_gain; 
    if(gain == GAIN_HIGH)
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

void JigBd_IF_select_output_ch(uint8_t in_output_ch)
{
    switch (in_output_ch)
    {
        case 0 : 
            LL_GPIO_ResetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 1 : 
            LL_GPIO_SetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 2 : 
            LL_GPIO_ResetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 3 : 
            LL_GPIO_SetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 4 : 
            LL_GPIO_ResetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 5 : 
            LL_GPIO_SetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 6 : 
            LL_GPIO_ResetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 7 : 
            LL_GPIO_SetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 8 : 
            LL_GPIO_ResetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 9 : 
            LL_GPIO_SetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 10 : 
            LL_GPIO_ResetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;

        case 11 : 
            LL_GPIO_SetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_ResetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;
        
        case 12 : 
            LL_GPIO_SetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            break;
        
        default : 
            LL_GPIO_SetOutputPin(ENABLE_SELECT1_GPIO_Port, ENABLE_SELECT1_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT2_GPIO_Port, ENABLE_SELECT2_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT3_GPIO_Port, ENABLE_SELECT3_Pin);
            LL_GPIO_SetOutputPin(ENABLE_SELECT4_GPIO_Port, ENABLE_SELECT4_Pin);
            print(LOG_ERROR, "\r\nERROR:JigBd_IF_select_output_ch(%d)-INPUT WRONG!!\r\n", in_output_ch);
            break;
    }
}

void JigBd_IF_current_discharge(uint8_t mode)
{
    static uint8_t setting;

    if(setting != mode)
    {
        setting = mode;
        if(setting == CHARGE)
        {
            LL_GPIO_ResetOutputPin(LTC_DISCHARGE_GPIO_Port, LTC_DISCHARGE_Pin);
        }
        else // if (setting == DISCHARGE)
        {
            LL_GPIO_SetOutputPin(LTC_DISCHARGE_GPIO_Port, LTC_DISCHARGE_Pin);
        }
    }
}

/* BEGIN - Internal ADC ******************************************/
#define VREF_MEASURE_CNT_MAX 1
void JigBd_IF_VREF_ADC_StartStop(void)
{
    uint32_t u32_InternalADC = 0;
    for (uint8_t cnt = 0 ; cnt < VREF_MEASURE_CNT_MAX ; ++cnt)
    {
        LL_ADC_REG_StartConversionSWStart(ADC1);
        while(!LL_ADC_IsActiveFlag_EOCS(ADC1))
        {

        }
        LL_ADC_ClearFlag_EOCS(ADC1);
        u32_InternalADC += LL_ADC_REG_ReadConversionData12(ADC1);
    }    
    gu16_InternalADC = (uint16_t)(((float)u32_InternalADC / VREF_MEASURE_CNT_MAX) + 0.5f);
    print(LOG_DEBUG, "JigBd_IF_VREF_ADC_Get()-%d\r\n", gu16_InternalADC);
}

uint16_t JigBd_IF_VREF_ADC_Get(void)
{
    return gu16_InternalADC;
}

uint16_t JigBd_IF_Convert_Volt_to_VREF_ADC(double in_volt)
{
    uint16_t ret = (uint16_t)(((VREF_ADC_RESOLUTION * in_volt) / VREF_ADC_REF_VOLT) + 0.5f);
    return ret;
}

double JigBd_IF_Convert_VREF_ADC_to_Volt(uint16_t in_adc)
{
    double ret = (double)(((double)in_adc * VREF_ADC_REF_VOLT) / VREF_ADC_RESOLUTION);
    return ret;
}
/* END - Internal ADC ******************************************/

/* BEGIN - PWM Read Frequency ******************************************/
void JigBd_IF_TIM_Capture_Start(void)
{
    gu8_freq_input_capture_activated = 1;
    set_input_freq_init();
    
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_2, 512);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_2);
}

void JigBd_IF_TIM_Capture_Stop(void)
{
    gu8_freq_input_capture_activated = 0;
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_2);
}

uint16_t JigBd_IF_Freq_ConvertByPrecaler(double in_freq) //input must be 'MHz'
{
    // Convert to 'Hz' and Divide by TIM_CAPTURE_EXT_PRECALER
    uint32_t u32_rtn_val = (uint32_t)(((in_freq * 1000000) / TIM_CAPTURE_EXT_PRECALER) + 0.5f);
     
    if( u32_rtn_val > 65535) //2^16-1
    {
        print(LOG_ERROR, "\r\nERROR: JigBd_IF_Freq_ConvertByPrecaler(%lf) INPUT TOO BIG\r\n", in_freq);
        return 0;
    }

    if( u32_rtn_val < 1374) // 1374 = ( 90Mhz / 2^16 ) + 1;
    {
        print(LOG_ERROR, "\r\nERROR: JigBd_IF_Freq_ConvertByPrecaler(%lf) INPUT TOO SMALL\r\n", in_freq);
        return 0;
    }

    return (uint16_t)u32_rtn_val;
}

uint16_t JigBd_IF_Freq_Get(void)
{
    uint32_t input_freq = get_input_freq();
    if(input_freq > 65535) //2^16-1
    {
        print(LOG_ERROR, "\r\nERROR: JigBd_IF_Freq_Get() Retrun[%d] is TOO BIG\r\n", input_freq);
        return 0;
    }        
    // print(LOG_DEBUG, "\r\n JigBd_IF_Freq_Get() : %d",get_input_freq());
    return (uint16_t)input_freq;
}

double JigBd_IF_Freq_Count_to_MHZ(uint16_t count)
{
    double rtn_freq = 0.0;
    if(count >= 65535) //2^16-1
    {
        print(LOG_ERROR, "\r\nERROR: JigBd_IF_Freq_Get() Retrun[%d] is TOO BIG\r\n", count);
        return 0;
    }  

    rtn_freq = ((double)count * TIM_CAPTURE_EXT_PRECALER) / 1000000;
    
    // print(LOG_INFO, "\r\n JigBd_IF_Freq_Get() : %d",get_input_freq());
    return rtn_freq;
}
/* END - PWM Read Frequency ******************************************/

/* BEGIN - Make XDIC DATA SIGNAL through PWM DMA *******************************************/
__STATIC_INLINE uint16_t getAbit(uint16_t x, int n)
{
    return (x & (1 << n)) >> n;
}

__STATIC_INLINE void setDmaWriteStart(uint32_t len)
{
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, len);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);
}

__STATIC_INLINE void setDmaReadStart(uint32_t len)
{
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, len);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, len);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);
    
#if 1
    LL_TIM_EnableDMAReq_CC1(TIM2);
    LL_TIM_EnableDMAReq_CC2(TIM2);
    
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
#endif

    while(LL_DMA_IsActiveFlag_TC6(DMA1) == 0)
    {
        //__NOP();
    }
    LL_DMA_ClearFlag_TC6(DMA1);
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);

    while(LL_DMA_IsActiveFlag_TC5(DMA1) == 0)
    {
        //__NOP();
    }
    LL_DMA_ClearFlag_TC5(DMA1);
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
    
#if 1
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_TIM_DisableDMAReq_CC1(TIM2);
    LL_TIM_DisableDMAReq_CC2(TIM2);
#endif
}

__STATIC_INLINE float getParseReadCommand(uint32_t* pdata, uint16_t len)
{
    uint32_t diff = 0;
    uint32_t count = 0;

    float f_diff = 0;
    float f_input_frequency = 0;
    uint16_t logic_1_duty_min = 0;
    uint16_t logic_1_duty_max = 0;

    uint16_t* p_rising = gu16_pwm_rx_risingBuffer + 0;
    uint16_t* p_falling = gu16_pwm_rx_fallingBuffer + 0;

    // for(uint16_t i = 0 ; i < len ; ++i) skip 1st rising
    for(uint16_t i = 1 ; i < len ; ++i)
    {
        diff += p_rising[i];
        ++count;
    }

    if(count)
    {
        f_diff = ((float)diff / count);

        logic_1_duty_min = (uint16_t)D_1_MIN(f_diff);
        logic_1_duty_max = (uint16_t)D_1_MAX(f_diff);
    }

    if(pdata)
    {
        uint32_t n_xd12_response = 0;

        for(volatile uint16_t j = 0 ; j < len ; ++j)
        {
            if((p_falling[j] > logic_1_duty_min) && (p_falling[j] < logic_1_duty_max))
            {
                n_xd12_response |= (1U << ((len - 1) - j));
            }
        }

        *pdata = n_xd12_response;
    }

    if(f_diff > 0)
    {
        f_input_frequency = (APB1_TIM_FREQ / f_diff);
    }

    return f_input_frequency;
}

static void JigBd_IF_IdGen_Command_through_PWM_to_XD12()
{
    uint16_t pwm_length = 0;

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0;   /* dummy */

    //bit21 : start
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_; 

    //bit20 ~ bit18 : command_code[2:0]
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0; //Make Signal End LOW.

    //gu8_pwm_dma_flg = PWM_DMA_DONE;
    setDmaWriteStart(pwm_length);

    //while(gu8_pwm_dma_flg)
    while(LL_DMA_IsActiveFlag_TC1(DMA2) == 0)
    {
        __NOP();
    }

    //LL_TIM_ClearFlag_CC2(TIM1);
    //LL_TIM_ClearFlag_CC2OVR(TIM1);

    LL_DMA_ClearFlag_TC1(DMA2);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);
    us_tdelay(10);
}

static void JigBd_IF_SyncGen_Command_through_PWM_to_XD12()
{
    uint16_t pwm_length = 0;

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0;   /* dummy */

    //bit21 : start
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_; 

    //bit20 ~ bit18 : command_code[2:0]
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0; //Make Signal End LOW.

    //gu8_pwm_dma_flg = PWM_DMA_DONE;
    setDmaWriteStart(pwm_length);

    //while(gu8_pwm_dma_flg)
    while(LL_DMA_IsActiveFlag_TC1(DMA2) == 0)
    {
        __NOP();
    }

    //LL_TIM_ClearFlag_CC2(TIM1);
    //LL_TIM_ClearFlag_CC2OVR(TIM1);

    LL_DMA_ClearFlag_TC1(DMA2);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);
    us_tdelay(10);
}

//#pragma optimize=none
static uint16_t JigBd_IF_Read_Command_through_PWM_to_XD12(uint8_t in_addr)
{
    uint32_t pwm_length = 0;

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0;   /* dummy */

    //bit9 : start
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;

    //bit8 ~ bit6 : command_code[2:0]
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;

    //bit5 ~ bit0 : addr[5:0]
    for(uint8_t i = 0 ; i < 6 ; ++i)
    {
        if(getAbit((uint16_t)in_addr, 5-i) == 1)
        {
            gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
        }
        else
        {
            gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
        }        
    }

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0; //Make Signal End LOW.

#ifdef _DBG_ALL
    char temp[64] = {0, };
    for (uint8_t i = 0 ; i < pwm_length ; ++i)
    {
        if(gu16_pwm_tx_risingBuffer[i] == _D_1_)
        {
            temp[i] = '1';
        }
        else if(gu16_pwm_tx_risingBuffer[i] == _D_0_)
        {
            temp[i] = '0';
        }
        else
        {
            temp[i] = ' ';
        }
    }
    print(LOG_DEBUG, "\r\nReadCMD(0x%X):[%s](%2u)\r\n", in_addr, temp, (pwm_length - 2));
#endif //_DBG_ALL

    //gu8_pwm_dma_flg = PWM_DMA_DONE;
    setDmaWriteStart(pwm_length);

    //while(gu8_pwm_dma_flg)
    while(LL_DMA_IsActiveFlag_TC1(DMA2) == 0)
    {
        //__NOP();
    }

    //LL_TIM_ClearFlag_CC1(TIM1);
    //LL_TIM_ClearFlag_CC1OVR(TIM1);
    //LL_TIM_ClearFlag_CC2(TIM1);
    //LL_TIM_ClearFlag_CC2OVR(TIM1);

    LL_DMA_ClearFlag_TC1(DMA2);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);

    /* output enable set HIGH */
    LL_GPIO_SetOutputPin(PWM_SWITCH_GPIO_Port, PWM_SWITCH_Pin);
    
    setDmaReadStart((READ_RECV_LENTH + 0));
    
    /* output enable set LOW */
    LL_GPIO_ResetOutputPin(PWM_SWITCH_GPIO_Port, PWM_SWITCH_Pin);

    uint32_t n_response = 0;
    float f_frequency = getParseReadCommand(&n_response, READ_RECV_LENTH);

#ifdef _DBG_ALL
    print(LOG_INFO, "Received DATA(0x%2X):[%1.3fMHz, CODE : 0x%1X, ID : 0x%2X, DATA - 0x%3X]\r\n",in_addr, f_frequency, ((n_response >> 17) & 0x0F), ((n_response >> 12) & 0x1F), ((n_response >> 0) & 0xFFF));
#endif //_DBG_ALL

    us_tdelay(10);
    
    return (uint16_t)(n_response & 0x0FFF);
}

void JigBd_IF_Write_Command_through_PWM_to_XD12(uint8_t in_addr, uint16_t in_data)
{
    uint16_t pwm_length = 0;

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0;   /* dummy */

    //bit21 : start
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_; 

    //bit20 ~ bit18 : command_code[2:0]
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;

    //bit17 ~ bit12 : addr[5:0]
    for(uint8_t i = 0 ; i < 6 ; ++i)
    {
        if(getAbit((uint16_t)in_addr, 5-i) == 1)
        {
            gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
        }
        else
        {
            gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
        }        
    }

    //bit11 ~ bit00 : data[11:0]
    for(uint8_t i = 0 ; i < 12 ; ++i)
    {
        if(getAbit(in_data, 11-i) == 1)
        {
            gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
        }
        else
        {
            gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
        }
    }

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0; //Make Signal End LOW.

    //gu8_pwm_dma_flg = PWM_DMA_DONE;
    setDmaWriteStart(pwm_length);

    //while (gu8_pwm_dma_flg)
    while(LL_DMA_IsActiveFlag_TC1(DMA2) == 0)
    {
        __NOP();
    }

    //LL_TIM_ClearFlag_CC2(TIM1);
    //LL_TIM_ClearFlag_CC2OVR(TIM1);

    LL_DMA_ClearFlag_TC1(DMA2);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);
    us_tdelay(10);
}

void JigBd_IF_LD_Trans_Command_through_PWM_to_XD12(uint16_t in_LD_data)
{
    uint16_t pwm_length = 0;

    gu16_pwm_tx_risingBuffer[pwm_length++] = 0;   /* dummy */

    //bit25 : start
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_; 

    //bit24 ~ bit22 : command_code[2:0]
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
    gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;

    //LD[15:0]
    for(uint8_t j = 0 ; j < XD_CH_SIZE ; ++j)
    {
        for(uint8_t i = 0 ; i < 16 ; ++i)
        {
            if(getAbit((uint16_t)in_LD_data, 15-i) == 1)
            {
                gu16_pwm_tx_risingBuffer[pwm_length++] = _D_1_;
            }
            else
            {
                gu16_pwm_tx_risingBuffer[pwm_length++] = _D_0_;
            }        
        }
    }
    
    gu16_pwm_tx_risingBuffer[pwm_length++] = 0; //Make Signal End LOW.

    //gu8_pwm_dma_flg = PWM_DMA_DONE;
    setDmaWriteStart(pwm_length);

    //while(gu8_pwm_dma_flg)
    while(LL_DMA_IsActiveFlag_TC1(DMA2) == 0)
    {
        __NOP();
    }

    //LL_TIM_ClearFlag_CC2(TIM1);
    //LL_TIM_ClearFlag_CC2OVR(TIM1);

    LL_DMA_ClearFlag_TC1(DMA2);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);
    us_tdelay(10);
}


/* END - Make XDIC DATA SIGNAL through PWM DMA *******************************************/

/* BEGIN - Make XDIC DATA SIGNAL through SPI XC24 To XD12*********************************/

/* END - Make XDIC DATA SIGNAL through SPI XC24 To XD12 ***********************************/

void JigBd_IF_xd12_reset_Command(void)
{
    if(IS_XC24())
    {   
        XC24_IF_Write_XD12(XD12_ADDR_RESET, 0x01);
    }
    else
    {   
        JigBd_IF_Write_Command_through_PWM_to_XD12(XD12_ADDR_RESET, 0x01);
    }
}

void JigBd_IF_IdGen_Command(void)
{
    if(IS_XC24())
    {
        XC24_IF_IdGen();
    }
    else
    {   
        JigBd_IF_IdGen_Command_through_PWM_to_XD12();
    }
}

void JigBd_IF_SyncGen_Command(void)
{
    if(IS_XC24())
    {
        XC24_IF_SyncGen();
    }
    else
    {   
        JigBd_IF_SyncGen_Command_through_PWM_to_XD12();
    }
}

uint16_t JigBd_IF_Read_Command(uint8_t in_addr)
{
    uint16_t ret = 0;
    if(IS_XC24())
    {
        ret = XC24_IF_Read_XD12(in_addr);
    }
    else
    {        
        ret = JigBd_IF_Read_Command_through_PWM_to_XD12(in_addr);
    }
    return ret;
}

void JigBd_IF_Write_Command(uint8_t in_addr, uint16_t in_data)
{
    if(IS_XC24())
    {
        XC24_IF_Write_XD12(in_addr, in_data);
    }
    else
    {
        JigBd_IF_Write_Command_through_PWM_to_XD12(in_addr, in_data);
    }
    us_tdelay(100);
}

void JigBd_IF_Write_LD_Command(uint16_t in_LD_data)
{
    if(IS_XC24())
    {
        XC24_IF_Write_LD(in_LD_data);
    }
    else
    {
        JigBd_IF_LD_Trans_Command_through_PWM_to_XD12(in_LD_data);
    }
}

#ifdef DBG_TEST
#endif //DBG_TEST

/*** end of file ***/
