#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#include "drv_xdr12.h"
#include "comm_debugging.h"
#include "ldim_conversion.h"

#define SVSYNC_VSYNC_FREQ       (50000U)    /* 20us */

#define SVSYNC_PHASE_GREEN      (1U)
#define SVSYNC_PHASE_BLUE       (0U)

#define SVSYNC_GATING_TIME_US   (10U)
#define SVSYNC_CYCLE            (2U)
#define SVSYNC_SIZE             (XDR_SV_NO)
#define SVSYNC_TOTAL_CYCLE      (SVSYNC_CYCLE * SVSYNC_SIZE)

#define SVSYNC_RED_FREQ         (11520U)
#define SVSYNC_GREEN_FREQ       (7680U)
#define SVSYNC_BLUE_FREQ        (7680U)
#define SVSYNC_DIMMING_ON_us    (20U) // 20us

static bool gb_vsync_out_flag;
static uint8_t gn_svsync_count;

static float gf_vsync_sub_freq;
static float gf_vsync_sub_freq_20us;
static float gf_svsync_sub_freq;
static uint32_t gn_svsync_sub_period;
static uint32_t gn_svsync_sub_duty;

static inline void tim_update_vsync_out_freq(void)
{
    uint32_t AutoReload = LL_TIM_GetAutoReload(TIM8);
    uint32_t Prescaler = LL_TIM_GetPrescaler(TIM8);
    gf_vsync_sub_freq = (((float)APB2_TIM_CLK / (Prescaler + 1U)) / (AutoReload + 1U));
    gf_vsync_sub_freq_20us = (float)(1000000 / (((float)1000000/gf_vsync_sub_freq) - 20.0f));
}

static inline void tim_update_svsync_out_freq(void)
{
    gf_svsync_sub_freq = (float)((gf_vsync_sub_freq_20us * XDR_SV_NO) * SVSYNC_CYCLE);   /* 2 for R/GB */
    gn_svsync_sub_period = TIM3_PERIOD_HZ(gf_svsync_sub_freq);
    gn_svsync_sub_duty = SVSYNC_OUT_PULSE(20U);
}

static void tim_svsync_timer_start(void)
{
    LL_TIM_SetCounter(TIM3, 0);
    LL_TIM_ClearFlag_UPDATE(TIM3);
    LL_TIM_SetAutoReload(TIM3, TIM3_PERIOD_HZ(SVSYNC_VSYNC_FREQ));  /* 50% of 1-sub frame */
    LL_TIM_OC_SetCompareCH1(TIM3, SVSYNC_OUT_PULSE(10U));
    LL_TIM_EnableCounter(TIM3);

    tim_update_svsync_out_freq();

    gn_svsync_count = 0U;
}

static void tim_svsync_timer_stop(void)
{
    LL_TIM_OC_SetCompareCH1(TIM3, 0U);
    LL_TIM_DisableCounter(TIM3);

    gn_svsync_count = 0U;
}

void tim_vsync_out_start(void)
{
    tim_update_vsync_out_freq();

    LL_TIM_OC_SetCompareCH2(TIM8, VSYNC_OUT_PULSE);
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_EnableCounter(TIM8);
}

void tim_vsync_out_stop(void)
{
    LL_TIM_OC_SetCompareCH2(TIM8, 0U);
    LL_TIM_DisableCounter(TIM8);
}

void tim_fllsync_start(void)
{
    FLLSYNC_ENABLE();
    LL_TIM_SetCounter(TIM4, 0U);
    LL_TIM_OC_SetCompareCH2(TIM4, FLLSYNC_OUT_PULSE);
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM4);
}

void tim_fllsync_stop(void)
{
    FLLSYNC_DISABLE();
    LL_TIM_DisableCounter(TIM4);
    LL_TIM_CC_DisableChannel(TIM4, LL_TIM_CHANNEL_CH1);
    LL_TIM_SetCounter(TIM4, 0U);
}

void tim_svsync_out_handler(void)
{
    uint32_t period = 0U;
    uint32_t CompareValue = 0U;

    ++gn_svsync_count;

    uint32_t phase = (gn_svsync_count % SVSYNC_CYCLE);
    //uint32_t phase = (gn_svsync_count & (SVSYNC_CYCLE - 1U));

    switch(phase)
    {
        case SVSYNC_PHASE_GREEN:
        {
            period = gn_svsync_sub_period;
            CompareValue = gn_svsync_sub_duty;
            break;
        }
        case SVSYNC_PHASE_BLUE:
        {
            period = gn_svsync_sub_period;
            CompareValue = gn_svsync_sub_duty;
            break;
        }
        default:
        {
            FATAL_INVALID_INPUT(phase);
            break;
        }
    }

    LL_TIM_SetAutoReload(TIM3, period);
    LL_TIM_OC_SetCompareCH1(TIM3, CompareValue);

    if((SVSYNC_TOTAL_CYCLE + 0U) == gn_svsync_count)
    {
        us_delay(21U);
        tim_svsync_timer_stop();
    }
}

void tim_vsync_out_handler(void)
{
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
        xdr12_syncgen();
    #endif
    tim_svsync_timer_start();
    gb_vsync_out_flag = true;
}

void tim_set_vsync_out_freq(float f)
{
    uint32_t AutoReload = TIM8_PERIOD_HZ(f);

    LL_TIM_SetAutoReload(TIM8, AutoReload);

    tim_update_vsync_out_freq();
}

static bool gb_xcr_ldim_block_conversion_flag;
static uint16_t gn_xcr_ldim_block_conversion_index;

void tim_vsync_out_process(void)
{
    if(true == gb_vsync_out_flag)
    {
        gn_xcr_ldim_block_conversion_index = 0U;
        gb_xcr_ldim_block_conversion_flag = true;
        gb_vsync_out_flag = false;
    }

    if(true == gb_xcr_ldim_block_conversion_flag)
    {
        uint16_t* p_led_color_table = ldim_get_led_color_buffer();
        ldim_set_ldim_rgb(gn_xcr_ldim_block_conversion_index, \
            p_led_color_table[COLOR_RED], p_led_color_table[COLOR_GREEN], p_led_color_table[COLOR_BLUE], p_led_color_table[COLOR_CYAN]);
        ++gn_xcr_ldim_block_conversion_index;
        if(LDIM_BLK_SIZE == gn_xcr_ldim_block_conversion_index)
        {
            xdr12_ld_transfer();
            gb_xcr_ldim_block_conversion_flag = false;
            gn_xcr_ldim_block_conversion_index = 0U;
        }
    }
}
