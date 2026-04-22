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
#define XDIC_LD_SCAN_TEST_AT_0      (256)

#define LINE_DELAY_TABLE_SIZE       (13)

#define SEC_TO_VSYNC_CNT(sec)       ((uint16_t)(120.0f * ((float)(sec))))

static bool gb_led_vsync_flag;

bool gb_led_enable_table[TOTAL_BLOCK_SIZE] = {false};

static uint8_t gn_led_current_increase_cnt;

static bool gb_led_low_current_mode;
static bool gb_led_test_mode_scan_no;
static bool gb_led_test_mode_line_delay;
const uint16_t gn_line_delay_table[LINE_DELAY_TABLE_SIZE] =
{
    0U, 1U, 2U, 4U, 8U, 16U, 32U, 64U, 128U, 256U, 512U, 1024U, 2048U
};


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
        us_delay(10U);
        XC24_Turn_Off_Sync_Auto();

        us_delay(100U);
        XC24_Turn_On_Sync_Auto();

        us_delay(3500U);

        if (gb_led_test_mode_scan_no == true)
        {
            static uint16_t vsync_cnt = 0;
            static uint8_t scan_no_test_phase = 0U;
            static uint16_t ld_scan_test_val = 0U;
            static uint8_t check_channel = 0U;
            ++vsync_cnt;
            switch (scan_no_test_phase)
            {
                case 0U : // go to x8
                    if (vsync_cnt >= SEC_TO_VSYNC_CNT(0.1f))
                    {
                        XDIC_Test_Set_Scan_No(3U);
                        ld_scan_test_val = (XDIC_LD_SCAN_TEST_AT_0 >> 3U);
                        gf_xd_max_current = XDIC_CURRENT_DEFAULT;
                        scan_no_test_phase = 1U;
                        vsync_cnt = 0U;
                    }
                    break;
                case 1U : // off
                    if (vsync_cnt >= SEC_TO_VSYNC_CNT(1.0f))
                    {
                        ld_scan_test_val = 0U;
                        gf_xd_max_current = 0U;
                        scan_no_test_phase = 2U;
                        vsync_cnt = 0U;
                    }
                    break;
                case 2U : // go to x1
                    if (vsync_cnt >= SEC_TO_VSYNC_CNT(0.1f))
                    {
                        XDIC_Test_Set_Scan_No(0U);
                        ld_scan_test_val = (XDIC_LD_SCAN_TEST_AT_0 >> 0U);
                        gf_xd_max_current = XDIC_CURRENT_DEFAULT;
                        scan_no_test_phase = 3U;
                        vsync_cnt = 0U;
                    }
                    break;
                case 3U : // off
                    if (vsync_cnt >= SEC_TO_VSYNC_CNT(1.0f))
                    {
                        ld_scan_test_val = 0U;
                        gf_xd_max_current = 0U;
                        scan_no_test_phase = 0U;
                        vsync_cnt = 0U;
                        ++check_channel;
                        if (check_channel >= 8U)
                        {
                            vsync_cnt = 0;
                            scan_no_test_phase = 0U;
                            ld_scan_test_val = 0U;
                            check_channel = 0U;
                            LED_System_DeInit();
                        }
                    }
                    break;
            }
            XC24_IF_Write_LD(ld_scan_test_val);
            XDIC_Update_Max_Current_Vref(gf_xd_max_current, gb_led_low_current_mode);
        }
        else
        {
            if (gb_led_test_mode_line_delay)
            {
                static uint16_t vsync_cnt = 0;
                static uint16_t line_delay_table_idx = 0;
                ++vsync_cnt;
                if (vsync_cnt > SEC_TO_VSYNC_CNT(0.5f))
                {
                    XDIC_Test_Set_Line_Delay(gn_line_delay_table[line_delay_table_idx]);
                    ++line_delay_table_idx;
                    if (line_delay_table_idx >= LINE_DELAY_TABLE_SIZE)
                    {
                        line_delay_table_idx = 0U;
                        LED_System_DeInit();
                    }
                    vsync_cnt = 0U;
                }
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
        }
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

void LED_Enable_Scan_No_Test(bool en)
{
    gb_led_test_mode_scan_no = en;
    gf_xd_max_current = XDIC_CURRENT_DEFAULT;
}

void LED_Enable_Line_Delay_Test(bool en)
{
    gb_led_test_mode_line_delay = en;
    gf_xd_max_current = XDIC_CURRENT_DEFAULT;
    if (en == true)
    {
        gf_xd_duty = 25.0f;
    }
    else
    {
        gf_xd_duty = 100.0f;
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
    LED_Enable_Scan_No_Test(false);
    LED_Enable_Line_Delay_Test(false);
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
    gf_xd_max_current = XDIC_CURRENT_DEFAULT;
}

void LED_System_Manual_DeInit(void)
{
    Vsync_Timer_Stop();
    LED_Low_Current_Mode(false);
    LED_Enable_Scan_No_Test(false);
    LED_Enable_Line_Delay_Test(false);
    XDIC_DeInit();
    XC24_DeInit();
}
/*** end of file ***/