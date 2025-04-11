#include "config.h"

static uint16_t gn_sub_frame_cnt;
static uint16_t gn_scan_line_cnt;
uint8_t gn_gs_buffer_set_flag;
uint8_t gn_gs_buffer_parsing_flag;
uint8_t gn_change_pattern_flag;
uint16_t gn_dimming_value;

uint16_t gn_gclk_pwm_length;

static gdim_pattern_t gt_gdim_pattern;

#define MAX_CURRENT 15.858 //mA, in order to change max_current, access to CCX registers (0 ~ 511).

void set_dimming_value(uint16_t dim_data)
{
    gn_dimming_value = dim_data;
}

uint16_t get_dimming_value()
{
    return gn_dimming_value;
}

void set_pattern_value(gdim_pattern_t pattern_data)
{
    gt_gdim_pattern  = pattern_data;
}

gdim_pattern_t get_pattern_value()
{
    return gt_gdim_pattern;
}

void vsync_IT_user_handler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM6))
    {
        LL_TIM_ClearFlag_UPDATE(TIM6);

        /* init parameters */
        gn_scan_line_cnt = 0;
        gn_sub_frame_cnt = 0;
        gn_gs_buffer_set_flag = 1;

        HAL_GPIO_WritePin(DEBUG_PIN_GPIO_Port, DEBUG_PIN_Pin, GPIO_PIN_SET);
        LL_GPIO_SetOutputPin(DEBUG_2_GPIO_Port, DEBUG_2_Pin);

        /* init scan_line decoder*/
        decode_set_scan_line(DECODE_CH_MAX);

        if (ADDRESS_BANK_A == tlc_get_gs_bank_address())
        {
            tlc_set_gs_bank_address(ADDRESS_BANK_B);
        }
        else //if (tlc_get_gs_bank_address() == ADDRESS_BANK_B)
        {
            tlc_set_gs_bank_address(ADDRESS_BANK_A);
        }

        /* make vsync cmd */
        tlc_send_daisy_reg_write_command(CMD_VSYNC, 0, 0);

        /* start sub_frame timer */
        tim7_IT_start();

        /* start parsing_GS timer */
        tim14_IT_start();

        /* start F_scan Timer for 1st sub_frame */
        //tim9_IT_start();

        HAL_GPIO_WritePin(DEBUG_PIN_GPIO_Port, DEBUG_PIN_Pin, GPIO_PIN_RESET);
    }
}

void sub_frame_IT_user_handler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM7))
    {
        LL_TIM_ClearFlag_UPDATE(TIM7);
        /* start F_scan Timer */
        LL_GPIO_SetOutputPin(DEBUG_2_GPIO_Port, DEBUG_2_Pin);
        tim9_IT_start();

        decode_set_scan_line(DECODE_CH_MAX);

        ++gn_sub_frame_cnt;
        if (gn_sub_frame_cnt >= 256)
        {
            gn_sub_frame_cnt = 0;
            tim7_IT_stop();
        }
    }
}

void scan_line_IT_user_handler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM9))
    {
        LL_TIM_ClearFlag_UPDATE(TIM9);
        /* change scan line */
        decode_set_scan_line(gn_scan_line_cnt);
        ++gn_scan_line_cnt;

        /* make GCLK */
        tim1_PWM_DMA_start();

        if (gn_scan_line_cnt >= DIMMING_LINE_NUM)
        {
            LL_GPIO_ResetOutputPin(DEBUG_2_GPIO_Port, DEBUG_2_Pin);
            tim9_IT_stop();
            gn_scan_line_cnt = 0;
        }
    }
}

void parsing_GS_IT_user_handler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM14))
    {
        LL_TIM_ClearFlag_UPDATE(TIM14);
        gn_gs_buffer_parsing_flag = 1;
    }
}

void change_pattern_IT_user_handler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM13))
    {
        LL_TIM_ClearFlag_UPDATE(TIM13);
        gn_change_pattern_flag = 1;
    }
}

void M_Project_init()
{
    decode_init();

    gn_dimming_value = 65535;
    gn_gclk_pwm_length = GCLK_TABLE_LENGTH;
    tlc59581_init();

    HAL_Delay(10);

    /* vsync start */
    tim6_IT_start();

    /* pattern change timer start */
    tim13_IT_start();
}
