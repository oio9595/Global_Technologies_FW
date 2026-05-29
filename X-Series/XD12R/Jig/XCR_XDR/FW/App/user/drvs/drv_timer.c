
#include "main.h"

#define SVSYNC_VSYNC_FREQ       (50000U)    /* 20us */

#define SVSYNC_PHASE_GREEN      (1U)
#define SVSYNC_PHASE_BLUE       (0U)

#define SVSYNC_GATING_TIME_US   (21U)
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

static void tim_svsync_timer_start(void)
{
    gf_svsync_sub_freq = (float)((gf_vsync_sub_freq_20us * XDR_SV_NO) * SVSYNC_CYCLE);   /* 2 for R/GB */
    gn_svsync_sub_period = TIM3_PERIOD_HZ(gf_svsync_sub_freq);
    gn_svsync_sub_duty = SVSYNC_OUT_PULSE(20U);

    LL_TIM_SetCounter(TIM3, 0);
    LL_TIM_ClearFlag_UPDATE(TIM3);
    LL_TIM_SetAutoReload(TIM3, TIM3_PERIOD_HZ(SVSYNC_VSYNC_FREQ));  /* 50% of 1-sub frame */
    LL_TIM_OC_SetCompareCH1(TIM3, SVSYNC_OUT_PULSE(10U));
    LL_TIM_EnableCounter(TIM3);

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
    uint32_t AutoReload = LL_TIM_GetAutoReload(TIM8);
    uint32_t Prescaler = LL_TIM_GetPrescaler(TIM8);
    gf_vsync_sub_freq = (((float)APB2_TIM_CLK / (Prescaler + 1U)) / (AutoReload + 1U));
    gf_vsync_sub_freq_20us = (float)(1000000 / (((float)1000000/gf_vsync_sub_freq) - 20.0f));

    LL_TIM_OC_SetCompareCH2(TIM8, VSYNC_OUT_PULSE);
    LL_TIM_EnableCounter(TIM8);
}

void tim_vsync_out_stop(void)
{
    LL_TIM_OC_SetCompareCH2(TIM8, 0U);
    LL_TIM_DisableCounter(TIM8);
}

void tim_svsync_out_handler(void)
{
    ++gn_svsync_count;

    if((SVSYNC_TOTAL_CYCLE + 1U) == gn_svsync_count)
    {
        tim_svsync_timer_stop();
    }
    else
    {
        uint32_t period = 0U;
        uint32_t CompareValue = 0U;

        uint32_t phase = (gn_svsync_count % SVSYNC_CYCLE);

        switch(phase)
        {
        case SVSYNC_PHASE_GREEN:
            period = gn_svsync_sub_period;
            CompareValue = gn_svsync_sub_duty;
            break;
        case SVSYNC_PHASE_BLUE:
            period = gn_svsync_sub_period;
            CompareValue = gn_svsync_sub_duty;
            break;
        default:
            break;
        }

        LL_TIM_SetAutoReload(TIM3, period);
        LL_TIM_OC_SetCompareCH1(TIM3, CompareValue);
    }
}

void tim_vsync_out_handler(void)
{
    gb_vsync_out_flag = true;
    tim_svsync_timer_start();
}

void tim_set_vsync_out_freq(float f)
{
    uint32_t AutoReload = TIM8_PERIOD_HZ(f);
    uint32_t Prescaler = LL_TIM_GetPrescaler(TIM8);

    LL_TIM_SetAutoReload(TIM8, AutoReload);

    gf_vsync_sub_freq = (((float)APB2_TIM_CLK / (Prescaler + 1U)) / (AutoReload + 1U));
    gf_vsync_sub_freq_20us = (float)(1000000 / (((float)1000000/gf_vsync_sub_freq) - 20.0f));
}

void tim_vsync_out_process(void)
{
    if(true == gb_vsync_out_flag)
    {
        gb_vsync_out_flag = false;
    }
}

