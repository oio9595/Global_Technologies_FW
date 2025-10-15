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

static uint8_t gn_led_current_increase_cnt;

void Vsync_Timer_Start(void)
{
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);

    for (uint8_t i = 0 ; i < TOTAL_BLOCK_SIZE ; ++i)
    {
        gb_xd_led_enable_table[i] = false;
    }
    gf_xd_max_current = 0.0f;
    gn_led_current_increase_cnt = 0;

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
    gf_xd_max_current = 0.0f;
    gn_led_current_increase_cnt = 0;

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
        us_delay(10);
        XC24_IF_Turn_Off_Sync_Auto();

        us_delay(100);
        XC24_IF_Turn_On_Sync_Auto();

        us_delay(1200);
        XC24_IF_Write_LD();

        XDIC_Update_Max_Current_Vref(gf_xd_max_current);

        #if 0
            uint16_t fault_data[6] = {0, };
            fault_data[0] = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA1);
            fault_data[1] = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA2);
            fault_data[2] = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA3);
            fault_data[3] = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA4);
            fault_data[4] = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA5);
            fault_data[5] = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA6);

            print(LOG_PC, "FAULT: {[0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X]}\r\n",
                fault_data[0], fault_data[1], fault_data[2], fault_data[3], fault_data[4], fault_data[5]);
        #endif

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
    else if (in_bar_num <= 20)
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
    else if (in_bar_num <= 20)
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
    if (in_current < 0.0f)
    {
        in_current = 0.0f;
    }
    else if (in_current > 128.0f)
    {
        in_current = 128.0f;
    }
    gf_xd_max_current = in_current;
}

void LED_Current_Increase(void)
{
    if (gn_led_current_increase_cnt == 0)
    {
        gf_xd_max_current = 0.0f;
    }
    else
    {
        gf_xd_max_current = (16 * gn_led_current_increase_cnt) - 1;
    }

    ++gn_led_current_increase_cnt;
    if (gn_led_current_increase_cnt > 8)
    {
        gn_led_current_increase_cnt = 0;
    }
}

void LED_System_Init(void)
{
    XC24_Init();
    XDIC_Init();
    Vsync_Timer_Start();
}

void LED_System_DeInit(void)
{
    Vsync_Timer_Stop();
    XDIC_DeInit();
    XC24_DeInit();
}

void LED_System_Manual_Init(void)
{
    XC24_Init();
    XDIC_Init();
    Vsync_Timer_Start();

    for (uint8_t i = 0 ; i < TOTAL_BLOCK_SIZE ; ++i)
    {
        gb_xd_led_enable_table[i] = true;
    }
    gf_xd_max_current = 10.0f;
}

void LED_System_Manual_DeInit(void)
{
    Vsync_Timer_Stop();
    XDIC_DeInit();
    XC24_DeInit();
}
/*** end of file ***/