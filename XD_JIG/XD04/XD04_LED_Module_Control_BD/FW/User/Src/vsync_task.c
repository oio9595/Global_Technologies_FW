/** @file vsync_task.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
 #define __VSYNC_TASK_C__

#include "main.h"
#include "xdic.h"
#include "xc24.h"
#include "config.h"

static bool gb_xdic_vsync_flag;
volatile static bool gb_system_active;

bool gb_xd_led_enable_table[TOTAL_BLOCK_SIZE] = {false, };

void Vsync_Timer_Start(void)
{
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);
    gb_system_active = true;
}

void Vsync_Timer_Stop(void)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM8);

    for (uint8_t i = 0 ; i < TOTAL_BLOCK_SIZE ; ++i)
    {
        gb_xd_led_enable_table[i] = false;
    }
    gb_system_active = false;
}

void Vsync_Update_Handler(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM8);
    gb_xdic_vsync_flag = true;
}

void XDIC_Vsync_Task(void)
{
    if (gb_xdic_vsync_flag)
    {
        XC24_IF_Write_LD();

        XDIC_Update_Max_Current_Vref(gf_xd_max_current);

        gb_xdic_vsync_flag = false;
    }
}

void LED_BAR_On_Select(uint8_t in_bar_num)
{
    if (in_bar_num == 0)
    {
        for (uint8_t i = 0 ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gb_xd_led_enable_table[i] = true;
        }
    }
    else
    {
        uint8_t start_blk = in_bar_num - 1;
        for (uint8_t blk = 0 ; blk < 8 ; ++blk)
        {
            gb_xd_led_enable_table[start_blk + 20 * blk] = true;
        }
    }
}

void LED_BAR_Off_Select(uint8_t in_bar_num)
{
    if (in_bar_num == 0)
    {
        for (uint8_t i = 0 ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gb_xd_led_enable_table[i] = false;
        }
    }
    else
    {
        uint8_t start_blk = in_bar_num - 1;
        for (uint8_t blk = 0 ; blk < 8 ; ++blk)
        {
            gb_xd_led_enable_table[start_blk + 20 * blk] = false;
        }
    }
}

void LED_BLK_On_Select(uint8_t in_blk_num)
{
    if (in_blk_num == 0)
    {
        for (uint8_t i = 0 ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gb_xd_led_enable_table[i] = true;
        }
    }
    else if (in_blk_num <= TOTAL_BLOCK_SIZE)
    {
        gb_xd_led_enable_table[in_blk_num - 1] = true;
    }
}

void LED_BLK_Off_Select(uint8_t in_blk_num)
{
    if (in_blk_num == 0)
    {
        for (uint8_t i = 0 ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gb_xd_led_enable_table[i] = false;
        }
    }
    else if (in_blk_num <= TOTAL_BLOCK_SIZE)
    {
        gb_xd_led_enable_table[in_blk_num - 1] = false;
    }
}

void LED_Current_Select(float in_current)
{
    gf_xd_max_current = in_current;
}

/*** end of file ***/