/** @file jigbd_hw.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */

#include "include.h"

extern TIM_HandleTypeDef htim3;

uint16_t gui_latch_1_data = 0;
uint16_t gui_latch_2_data = 0;
uint16_t gui_latch_3_data = 0;

const static uint16_t LATCH_PORT_DATA_MAP[] =
{
    LS_D0_Pin, LS_D1_Pin, LS_D2_Pin, LS_D3_Pin, LS_D4_Pin, LS_D5_Pin, LS_D6_Pin, LS_D7_Pin
};

static GPIO_TypeDef * LATCH_PORT_CTL_MAP[] =
{
    GPIOC, GPIOC, GPIOA, GPIOB, GPIOB, GPIOA, GPIOA, GPIOA
};

void LATCH_LOAD_DATA(uint8_t latch_data)
{
    uint16_t temp_PIN_DATA = 0;
    for(uint8_t i = 0 ; i < 8 ; ++i)
    {
        temp_PIN_DATA = (latch_data >> i) & 0x01;
        HAL_GPIO_WritePin(LATCH_PORT_CTL_MAP[i], LATCH_PORT_DATA_MAP[i], (GPIO_PinState)temp_PIN_DATA);
    }
}

void LATCH__WritePin(uint8_t latch_num, uint16_t latch_bit, uint8_t ub_val)
{
    if(ub_val == LATCH_PIN_SET)
    {
        switch(latch_num)
        {
        case 1:
            gui_latch_1_data |= (1 <<latch_bit);
            LATCH_LOAD_DATA(gui_latch_1_data);
            HAL_GPIO_WritePin(GPIOC, LS_CP1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LS_OE1_GPIO_Port, LS_OE1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LS_CP1_Pin, GPIO_PIN_SET);
            break;
        case 2:
            gui_latch_2_data |= (1 <<latch_bit);
            LATCH_LOAD_DATA(gui_latch_2_data);
            HAL_GPIO_WritePin(GPIOA, LS_CP2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, LS_OE2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, LS_CP2_Pin, GPIO_PIN_SET);
            break;
        case 3:
            gui_latch_3_data |= (1 <<latch_bit);
            LATCH_LOAD_DATA(gui_latch_3_data);
            HAL_GPIO_WritePin(GPIOA, LS_CP3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LS_OE3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, LS_CP3_Pin, GPIO_PIN_SET);
            break;
        }
    }
    else //LATCH_PIN_RESET
    {
        switch(latch_num)
        {
        case 1:
            gui_latch_1_data &= ~(1 <<latch_bit);
            LATCH_LOAD_DATA(gui_latch_1_data);
            HAL_GPIO_WritePin(GPIOC, LS_CP1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LS_OE1_GPIO_Port, LS_OE1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LS_CP1_Pin, GPIO_PIN_SET);
            break;
        case 2:
            gui_latch_2_data &= ~(1 <<latch_bit);
            LATCH_LOAD_DATA(gui_latch_2_data);
            HAL_GPIO_WritePin(GPIOA, LS_CP2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, LS_OE2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, LS_CP2_Pin, GPIO_PIN_SET);
            break;
        case 3:
            gui_latch_3_data &= ~(1 <<latch_bit);
            LATCH_LOAD_DATA(gui_latch_3_data);
            HAL_GPIO_WritePin(GPIOA, LS_CP3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, LS_OE3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, LS_CP3_Pin, GPIO_PIN_SET);
            break;
        }
    }
}

void AQIC_VCC_EN(uint8_t on)
{
    if(on == PWR_OFF) /* off */
    {
        /* AQIC_PWR_CTL : OFF, LOW */
        HAL_GPIO_WritePin(AQIC_PWR_CTL_GPIO_Port, AQIC_PWR_CTL_Pin, GPIO_PIN_RESET);
    }
    else
    {
        /* AQIC_PWR_CTL : ON, HIGH */
        HAL_GPIO_WritePin(AQIC_PWR_CTL_GPIO_Port, AQIC_PWR_CTL_Pin, GPIO_PIN_SET);
    }
}

void AQIC_VLED_EN(uint8_t on)
{
    if(on == PWR_ON)
    {
        HAL_GPIO_WritePin(VLED_CTR_9V_GPIO_Port, VLED_CTR_9V_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(VLED_CTR_9V_GPIO_Port, VLED_CTR_9V_Pin, GPIO_PIN_SET);
    }
}

void AQIC_Mode_Duty(uint8_t duty)
{
    uint32_t tim_period = __HAL_TIM_GET_AUTORELOAD(&htim3);
    uint32_t compare_value = 0;

    if(duty == DUTY_FULL)
    {
        compare_value = (tim_period + 1);    /* pwm duty 100% */
    }
    else if(duty == DUTY_HALF)
    {
        compare_value = (uint32_t)(((float)(tim_period + 1) / 2.0f) - 1);    /* pwm duty ~50%, freq 1MHz */
    }
    else
    {
        compare_value = 0;    /* pwm duty 0% */
    }

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, compare_value);    /* pwm duty 100% */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void AQIC_Mode_Setting(uint8_t op_mode)
{
	if (op_mode == MODE_PWM)
	{
        LATCH__WritePin(MODE_SEL_LS_CP, MODE_SEL_LS_PORT, LATCH_PIN_RESET);
		AQIC_Apply_Voltage(5.0f, AQIC_MODE);
	}
	else if (op_mode == MODE_DAC)
	{
        LATCH__WritePin(MODE_SEL_LS_CP, MODE_SEL_LS_PORT, LATCH_PIN_SET);
		AQIC_Mode_Duty(DUTY_FULL);
	}
}

void AQIC_Select_Output_Ch(uint8_t ch)
{
    static uint8_t ch_mem = 0xFF;
    if (ch_mem != ch)
    {
        ch_mem = ch;
        switch (ch_mem)
        {
        case AQIC_O1 :
            LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_RESET);
            LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_RESET);
            LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_RESET);
            break;
        case AQIC_O2 :
            LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_SET);
            LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_RESET);
            LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_RESET);
            break;
        case AQIC_O3 :
            LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_RESET);
            LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_SET);
            LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_RESET);
            break;
        case AQIC_O4 :
            LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_SET);
            LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_SET);
            LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_RESET);
            break;
        case AQIC_O5 :
            LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_RESET);
            LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_RESET);
            LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_SET);
            break;
        case AQIC_O6 :
            LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_SET);
            LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_RESET);
            LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_SET);
            break;
        default :
            LATCH__WritePin(ENABLE_SELECT1_LS_CP, ENABLE_SELECT1_LS_PORT, LATCH_PIN_SET);
            LATCH__WritePin(ENABLE_SELECT2_LS_CP, ENABLE_SELECT2_LS_PORT, LATCH_PIN_SET);
            LATCH__WritePin(ENABLE_SELECT3_LS_CP, ENABLE_SELECT3_LS_PORT, LATCH_PIN_SET);
            break;
        }
    }
}

/*
CTL_UPPER_GAIN
 - HIGH : Sensing range 40mA
 - MID  : Sensing range 10mA
 - LOW  : Sensing range  1mA
*/

void change_current_gain(uint8_t gain)
{
    if(gain == GAIN_HIGH)
    {
        LATCH__WritePin(LTC_HIGH_CURRENT_LS_CP, LTC_HIGH_CURRENT_LS_PORT, LATCH_PIN_RESET);
        LATCH__WritePin(LTC_MID_CURRENT_LS_CP, LTC_MID_CURRENT_LS_PORT, LATCH_PIN_SET);
        LATCH__WritePin(LTC_LOW_CURRENT_LS_CP, LTC_LOW_CURRENT_LS_PORT, LATCH_PIN_SET);
    }
    else if (gain == GAIN_MID)
    {
        LATCH__WritePin(LTC_HIGH_CURRENT_LS_CP, LTC_HIGH_CURRENT_LS_PORT, LATCH_PIN_SET);
        LATCH__WritePin(LTC_MID_CURRENT_LS_CP, LTC_MID_CURRENT_LS_PORT, LATCH_PIN_RESET);
        LATCH__WritePin(LTC_LOW_CURRENT_LS_CP, LTC_LOW_CURRENT_LS_PORT, LATCH_PIN_SET);
    }
    else if (gain == GAIN_LOW)
    {
        LATCH__WritePin(LTC_HIGH_CURRENT_LS_CP, LTC_HIGH_CURRENT_LS_PORT, LATCH_PIN_SET);
        LATCH__WritePin(LTC_MID_CURRENT_LS_CP, LTC_MID_CURRENT_LS_PORT, LATCH_PIN_SET);
        LATCH__WritePin(LTC_LOW_CURRENT_LS_CP, LTC_LOW_CURRENT_LS_PORT, LATCH_PIN_RESET);
    }
}

void current_discharge(uint8_t mode)
{
    static uint8_t setting = 0;

    if(setting != mode)
    {
        setting = mode;
        if(setting == CHARGE)
        {
            HAL_GPIO_WritePin(LTC_DISCHARGE_GPIO_Port, LTC_DISCHARGE_Pin, GPIO_PIN_RESET);
        }
        else // if (setting == DISCHARGE)
        {
            HAL_GPIO_WritePin(LTC_DISCHARGE_GPIO_Port, LTC_DISCHARGE_Pin, GPIO_PIN_SET);
        }
    }
}

void set_ok_led(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_RESET);
    }
}

void set_ng_led(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_NG_GPIO_Port, LED_NG_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_NG_GPIO_Port, LED_NG_Pin, GPIO_PIN_RESET);
    }
}

void set_trimming_led(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_TRIM_GPIO_Port, LED_TRIM_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_TRIM_GPIO_Port, LED_TRIM_Pin, GPIO_PIN_RESET);
    }
}


/*** end of file ***/

