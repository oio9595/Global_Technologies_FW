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

#define XDIC_PWM_DUTY_MAX           (100.0f)
#define XDIC_CURRENT_MAX            (128.0f)
#define XDIC_CURRENT_DEFAULT        (10.0f)
#define PATTERN_CHANGE_FRAME_CNT    (120U)

static bool gb_led_vsync_flag;

static uint16_t gn_led_ld_buffer[TOTAL_BLOCK_SIZE];
static uint8_t gn_pattern_idx;
static uint32_t gn_vsync_tick;

void Vsync_Timer_Start(void)
{
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);
    gf_xd_max_current = 64.0f;
}

void Vsync_Timer_Stop(void)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0U);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM8);
    gb_led_vsync_flag = false;
    gf_xd_max_current = 0.0f;
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

        LED_Generate_Pattern(gn_pattern_idx, gn_vsync_tick);

        XC24_IF_Write_LD(gn_led_ld_buffer);
        XDIC_Update_Max_Current_Vref(gf_xd_max_current);

        gb_led_vsync_flag = false;
        ++gn_vsync_tick;
    }
}

void LED_BLK_Set_LD(uint8_t in_blk_num, uint16_t in_ld_value)
{
    if (in_ld_value > XDIC_LD_MAX)
    {
        print(LOG_PC, "\r\n    Input LD(%u) is limited to %u", in_ld_value, XDIC_LD_MAX);
        in_ld_value = XDIC_LD_MAX;
    }

    if (in_blk_num > TOTAL_BLOCK_SIZE)
    {
        return;
    }

    if (in_blk_num == 0U)
    {
        for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
        {
            gn_led_ld_buffer[i] = in_ld_value;
        }
        print(LOG_PC, "\r\n    Set All Block LD to %u (0x%03X)", in_ld_value, in_ld_value);
    }
    else
    {
        gn_led_ld_buffer[in_blk_num - 1U] = in_ld_value;
        print(LOG_PC, "\r\n    Set Block %u LD to %u (0x%03X)", in_blk_num, in_ld_value, in_ld_value);
    }
}

void LED_Current_Select(float in_current)
{
    float temp_current = in_current;
    if (temp_current < 0.0f)
    {
        print(LOG_PC, "\r\n    Input Current (%.1f) is limited to %.1f", in_current, 0.0f);
        temp_current = 0.0f;
    }
    else if (temp_current > XDIC_CURRENT_MAX)
    {
        print(LOG_PC, "\r\n    Input Current (%.1f) is limited to %.1f", in_current, XDIC_CURRENT_MAX);
        temp_current = XDIC_CURRENT_MAX;
    }
    gf_xd_max_current = temp_current;
    print(LOG_PC, "\r\n    Set Max Current to %.1f", gf_xd_max_current);
}

void LED_Generate_Pattern(uint8_t in_pattern, uint32_t in_vsync_tick)
{
    static bool gb_pattern_change_local = false;
    switch (in_pattern)
    {
        case PATTERN_01:
        {
            if (true == gb_pattern_change_local)
            {
                for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
                {
                    if (i % 2U == 0U)
                    {
                        gn_led_ld_buffer[i] = 0x040U;
                    }
                    else
                    {
                        gn_led_ld_buffer[i] = 0xFFFU;
                    }
                }
            }
            else
            {
                for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
                {
                    if (i % 2U == 0U)
                    {
                        gn_led_ld_buffer[i] = 0xFFFU;
                    }
                    else
                    {
                        gn_led_ld_buffer[i] = 0x040U;
                    }
                }
            }
            break;
        }
        case PATTERN_02:
        {
            if (true == gb_pattern_change_local)
            {
                for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
                {
                    uint8_t row = i / BLK_COL_SIZE;
                    uint8_t col = i % BLK_COL_SIZE;

                    if (row % 2U == 0U) // even row
                    {
                        if (col % 2U == 0U)
                        {
                            gn_led_ld_buffer[i] = 0x040U;
                        }
                        else
                        {
                            gn_led_ld_buffer[i] = 0xFFFU;
                        }
                    }
                    else // odd row
                    {
                        if (col % 2U == 0U)
                        {
                            gn_led_ld_buffer[i] = 0xFFFU;
                        }
                        else
                        {
                            gn_led_ld_buffer[i] = 0x040U;
                        }
                    }
                }
            }
            else
            {
                for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
                {
                    uint8_t row = i / BLK_COL_SIZE;
                    uint8_t col = i % BLK_COL_SIZE;

                    if (row % 2U == 0U) // even row
                    {
                        if (col % 2U == 0U)
                        {
                            gn_led_ld_buffer[i] = 0xFFFU;
                        }
                        else
                        {
                            gn_led_ld_buffer[i] = 0x040U;
                        }
                    }
                    else // odd row
                    {
                        if (col % 2U == 0U)
                        {
                            gn_led_ld_buffer[i] = 0x040U;
                        }
                        else
                        {
                            gn_led_ld_buffer[i] = 0xFFFU;
                        }
                    }
                }
            }
            break;
        }
        case PATTERN_03:
        {
            if (true == gb_pattern_change_local)
            {
                for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
                {
                    uint8_t col = i % BLK_COL_SIZE;
                    if (1U == ((col >> 1U) & 0x01U))
                    {
                        gn_led_ld_buffer[i] = 0xFFFU;
                    }
                    else
                    {
                        gn_led_ld_buffer[i] = 0x040U;
                    }
                }
            }
            else
            {
                for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
                {
                    uint8_t col = i % BLK_COL_SIZE;
                    if (1U == ((col >> 1U) & 0x01U))
                    {
                        gn_led_ld_buffer[i] = 0x040U;
                    }
                    else
                    {
                        gn_led_ld_buffer[i] = 0xFFFU;
                    }
                }
            }
            break;
        }
        default:
        {
            for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
            {
                gn_led_ld_buffer[i] = 0xFFFU;
            }
            gb_pattern_change_local = false;
            break;
        }
    }

    if (in_vsync_tick % PATTERN_CHANGE_FRAME_CNT == 0U)
    {
        if (true == gb_pattern_change_local)
        {
            gb_pattern_change_local = false;
        }
        else
        {
            gb_pattern_change_local = true;
        }
    }
}

void LED_Change_Pattern(uint8_t in_pattern)
{
    if (in_pattern > PATTERN_NONE)
    {
        print(LOG_PC, "\r\n    Input Pattern(%u) is limited to %u", in_pattern, PATTERN_NONE);
        in_pattern = PATTERN_NONE;
    }
    gn_pattern_idx = in_pattern;

    if (gn_pattern_idx == PATTERN_NONE)
    {
        print(LOG_PC, "\r\n    Set Pattern to none (%u)", PATTERN_NONE);
    }
    else
    {
        print(LOG_PC, "\r\n    Set Pattern to %u", gn_pattern_idx);
    }
}

void LED_Change_Pattern_to_P1(void)
{
    gn_pattern_idx = PATTERN_01;
    print(LOG_PC, "\r\n    Set Pattern to %u", gn_pattern_idx);
}

void LED_Change_Pattern_to_P2(void)
{
    gn_pattern_idx = PATTERN_02;
    print(LOG_PC, "\r\n    Set Pattern to %u", gn_pattern_idx);
}

void LED_Change_Pattern_to_P3(void)
{
    gn_pattern_idx = PATTERN_03;
    print(LOG_PC, "\r\n    Set Pattern to %u", gn_pattern_idx);
}

void LED_Change_Pattern_to_None(void)
{
    gn_pattern_idx = PATTERN_NONE;
    print(LOG_PC, "\r\n    Set Pattern to %u", gn_pattern_idx);
}

void LED_System_Init(void)
{
    for (uint8_t i = 0U ; i < TOTAL_BLOCK_SIZE ; ++i)
    {
        gn_led_ld_buffer[i] = 0x800U;
    }
    gn_pattern_idx = PATTERN_MAX;

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
/*** end of file ***/