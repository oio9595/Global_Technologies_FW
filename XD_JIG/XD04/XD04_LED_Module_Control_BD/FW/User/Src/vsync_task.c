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

bool gb_xd_led_enable_table[TOTAL_BLOCK_SIZE] = {false, };

void Vsync_Timer_Start(void)
{
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);
}

void Vsync_Timer_Stop(void)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM8);
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
        XC24_IF_Write_LD(16383);

        XDIC_Update_Max_Current_Vref(gf_xd_max_current);

        gb_xdic_vsync_flag = false;
    }
}

/*** end of file ***/