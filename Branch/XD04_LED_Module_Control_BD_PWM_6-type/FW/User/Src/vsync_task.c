/** @file vsync_task.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
 #define __VSYNC_TASK_C__

#include "main.h"
#include "vsync_task.h"
#include "xdic.h"
#include "xc24.h"
#include "config.h"

#define XDIC_LD_MAX                 (4095U)
#define XDIC_LD_LOW_CURR_MODE       (5U)

#define XDIC_PWM_DUTY_MAX           (100.0f)
#define XDIC_CURRENT_MAX            (128.0f)
#define XDIC_CURRENT_DEFAULT        (10.0f)

#define SEC_TO_VSYNC_CNT(sec)       ((uint16_t)(120.0f * ((float)(sec))))

static bool gb_led_vsync_flag;

bool gb_led_enable_table[TOTAL_BLOCK_SIZE] = {false};

static uint8_t gn_led_current_increase_cnt;

static bool gb_led_low_current_mode;

const static float pwm_test_duty_table[] =   { 100.0f,  87.0f,  75.0f,  37.0f,   6.0f,   0.0f, 100.0f, 100.0f };
const static float pwm_test_current_table[] = {  2.0f,   2.0f,   3.0f,   5.0f,  32.0f, 100.0f,  10.0f,  10.0f };

void Vsync_Timer_Start(void)
{
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);

    for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
    {
        gb_led_enable_table[i] = false;
    }
    gf_xd_max_current = 0.0f;
    gn_led_current_increase_cnt = 0U;
}

void Vsync_Timer_Stop(void)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0U);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM8);

    for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
    {
        gb_led_enable_table[i] = false;
    }
    gb_led_vsync_flag = false;
    gf_xd_max_current = 0.0f;
    gn_led_current_increase_cnt = 0U;
}

void Vsync_Update_Handler(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM8);
    gb_led_vsync_flag = true;
}

void Vsync_Task(void)
{
    if (gb_led_vsync_flag)
    {
        static uint16_t vsync_cnt = 0U;
        us_delay(10U);
        XC24_Turn_Off_Sync_Auto();

        us_delay(100U);
        XC24_Turn_On_Sync_Auto();

        us_delay(3500U);

        static float wait_time = 2.0f;

        if (++vsync_cnt >= SEC_TO_VSYNC_CNT(wait_time))
        {
            static uint8_t pwm_test_phase = 0U;
            if (pwm_test_phase % 2 == 0U)
            {
                uint8_t table_idx = pwm_test_phase / 2U;
                gf_xd_duty = pwm_test_duty_table[table_idx];
                gf_xd_max_current = pwm_test_current_table[table_idx];
            }
            else
            {
                gf_xd_duty = 0.0f;
                gf_xd_max_current = 0.0f;
            }

            ++pwm_test_phase;
            if (pwm_test_phase == 12U)
            {
                wait_time = 0.5f;
            }
            else if (pwm_test_phase >= 16U)
            {
                wait_time = 2.0f;
                pwm_test_phase = 0U;
                vsync_cnt = 0U;
                LED_System_DeInit();
                return;
            }
            vsync_cnt = 0U;
        }

        if (gb_led_low_current_mode)
        {
            XC24_IF_Write_LD(XDIC_LD_LOW_CURR_MODE);
        }
        else
        {
            uint16_t ld_val = (uint16_t)((gf_xd_duty / 100.0f * XDIC_LD_MAX) + 0.5f);
            XC24_IF_Write_LD(ld_val);
        }
        XDIC_Update_Max_Current_Vref(gf_xd_max_current, gb_led_low_current_mode);
        gb_led_vsync_flag = false;
    }
}

void LED_Low_Current_Mode(uint8_t on_off)
{
    if (on_off)
    {
        gb_led_low_current_mode = true;
    }
    else
    {
        gb_led_low_current_mode = false;
    }
}

void LED_BAR_On_Select(uint8_t in_bar_num)
{
    if (in_bar_num == 0U)
    {
        for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gb_led_enable_table[i] = true;
        }
    }
    else if (in_bar_num <= 20U)
    {
        uint8_t start_blk = (in_bar_num - 1U);
        for (uint8_t blk = 0U ; blk < 8U ; ++blk)
        {
            gb_led_enable_table[start_blk + (20 * blk)] = true;
        }
    }
}

void LED_BAR_Off_Select(uint8_t in_bar_num)
{
    if (in_bar_num == 0U)
    {
        for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gb_led_enable_table[i] = false;
        }
    }
    else if (in_bar_num <= 20U)
    {
        uint8_t start_blk = (in_bar_num - 1U);
        for (uint8_t blk = 0U ; blk < 8U ; ++blk)
        {
            gb_led_enable_table[start_blk + (20 * blk)] = false;
        }
    }
}

void LED_BLK_On_Select(uint8_t in_blk_num)
{
    if (in_blk_num == 0U)
    {
        for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gb_led_enable_table[i] = true;
        }
    }
    else if (in_blk_num <= TOTAL_BLOCK_SIZE)
    {
        gb_led_enable_table[in_blk_num - 1U] = true;
    }
}

void LED_BLK_Off_Select(uint8_t in_blk_num)
{
    if (in_blk_num == 0U)
    {
        for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gb_led_enable_table[i] = false;
        }
    }
    else if (in_blk_num <= TOTAL_BLOCK_SIZE)
    {
        gb_led_enable_table[in_blk_num - 1] = false;
    }
}

void LED_Current_Select(float in_current)
{
    float temp_current = in_current;
    if (temp_current < 0.0f)
    {
        temp_current = 0.0f;
    }
    else if (temp_current > XDIC_CURRENT_MAX)
    {
        temp_current = XDIC_CURRENT_MAX;
    }
    gf_xd_max_current = temp_current;
}

void LED_Current_Increase(void)
{
    if (gn_led_current_increase_cnt == 0U)
    {
        gf_xd_max_current = 0.0f;
    }
    else
    {
        gf_xd_max_current = ((16U * gn_led_current_increase_cnt) - 1U);
    }

    ++gn_led_current_increase_cnt;
    if (gn_led_current_increase_cnt > 8U)
    {
        gn_led_current_increase_cnt = 0U;
    }
}

void LED_Duty_Select(float in_duty)
{
    float temp_duty = in_duty;
    if (temp_duty < 0.0f)
    {
        temp_duty = 0.0f;
    }
    else if (temp_duty > XDIC_PWM_DUTY_MAX)
    {
        temp_duty = XDIC_PWM_DUTY_MAX;
    }
    gf_xd_duty = temp_duty;
}

void LED_System_Init(void)
{
    XC24_Init();
    XDIC_Init();
    XC24_Update_Channel_Enable_By_XDIC_ID_Check();
    Vsync_Timer_Start();
}

void LED_System_DeInit(void)
{
    Vsync_Timer_Stop();
    LED_Low_Current_Mode(false);
    XDIC_DeInit();
    XC24_DeInit();
}

void LED_System_Manual_Init(void)
{
    XC24_Init();
    XDIC_Init();
    XC24_Update_Channel_Enable_By_XDIC_ID_Check();
    Vsync_Timer_Start();

    for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
    {
        gb_led_enable_table[i] = true;
    }
    gf_xd_max_current = 0.0f;
    gf_xd_duty = 0.0f;
}

void LED_System_Manual_DeInit(void)
{
    Vsync_Timer_Stop();
    LED_Low_Current_Mode(false);
    XDIC_DeInit();
    XC24_DeInit();
}
/*** end of file ***/