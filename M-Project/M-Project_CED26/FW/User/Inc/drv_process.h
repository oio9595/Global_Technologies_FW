#ifndef __DRV_PROCESS_H__
#define __DRV_PROCESS_H__

extern uint8_t gn_gs_buffer_set_flag;
extern uint8_t gn_gs_buffer_parsing_flag;
extern uint8_t gn_change_pattern_flag;
extern uint16_t gn_dimming_value;

extern uint16_t gn_gclk_pwm_length;

typedef enum
{
    PATTERN_0 = 0,
    PATTERN_1,
    PATTERN_2,
    PATTERN_3,
    PATTERN_4,
    PATTERN_5,
    PATTERN_6,
    PATTERN_7,
    PATTERN_MAX,
}gdim_pattern_t;

void set_dimming_value(uint16_t dim_data);
uint16_t get_dimming_value();

void set_pattern_value(gdim_pattern_t pattern_data);
gdim_pattern_t get_pattern_value();

/* timer for Vsync */
__STATIC_INLINE void tim6_IT_start()
{
    LL_TIM_EnableIT_UPDATE(TIM6);
    LL_TIM_EnableCounter(TIM6);
}

__STATIC_INLINE void tim6_IT_stop()
{
    LL_TIM_DisableCounter(TIM6);
    LL_TIM_DisableIT_UPDATE(TIM6);
}

void vsync_IT_user_handler();

/* timer for sub-frame 256-ea */
__STATIC_INLINE void tim7_IT_start()
{
    LL_TIM_EnableIT_UPDATE(TIM7);
    LL_TIM_EnableCounter(TIM7);
#if 0
    if (!gn_sub_frame_cnt)
    {
        LL_TIM_GenerateEvent_UPDATE(TIM7);
    }
#endif
    LL_TIM_GenerateEvent_UPDATE(TIM7);
}

__STATIC_INLINE void tim7_IT_stop()
{
    LL_TIM_DisableCounter(TIM7);
    LL_TIM_DisableIT_UPDATE(TIM7);
}

void sub_frame_IT_user_handler();

/* timer for scan_line 26-ea */
__STATIC_INLINE void tim9_IT_start()
{
    LL_TIM_EnableIT_UPDATE(TIM9);
    LL_TIM_EnableCounter(TIM9);
#if 0
    if (!gn_scan_line_cnt)
    {
        LL_TIM_GenerateEvent_UPDATE(TIM9);
    }
#endif
    LL_TIM_GenerateEvent_UPDATE(TIM9);
}

__STATIC_INLINE void tim9_IT_stop()
{
    LL_TIM_DisableCounter(TIM9);
    LL_TIM_DisableIT_UPDATE(TIM9);
}

void scan_line_IT_user_handler();

/* timer for GCLK PWM DMA */
__STATIC_INLINE void tim1_init()
{
    LL_TIM_EnableDMAReq_CC2(TIM1);
    LL_TIM_EnableCounter(TIM1);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);
}

__STATIC_INLINE void tim1_PWM_DMA_start()
{
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)gn_gclk_pwm_table);
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)(&(TIM1->CCR2)));
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_2, gn_gclk_pwm_length);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_2);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_2);

    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);

    LL_TIM_EnableDMAReq_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_TIM_EnableCounter(TIM1);
}

__STATIC_INLINE void tim1_PWM_DMA_stop()
{
    if(LL_DMA_IsActiveFlag_TC2(DMA2) == 1)
    {
        LL_DMA_ClearFlag_TC2(DMA2);
        LL_TIM_DisableDMAReq_CC2(TIM2);
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_2);

        LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
        LL_TIM_DisableCounter(TIM1);
    }
    else if(LL_DMA_IsActiveFlag_TE2(DMA2) == 1)
    {
        LL_DMA_ClearFlag_TE2(DMA2);
    }
}

__STATIC_INLINE void tim14_IT_start()
{
    LL_TIM_EnableIT_UPDATE(TIM14);
    LL_TIM_EnableCounter(TIM14);
}

__STATIC_INLINE void tim14_IT_stop()
{
    LL_TIM_DisableCounter(TIM14);
    LL_TIM_DisableIT_UPDATE(TIM14);
}

void parsing_GS_IT_user_handler();


__STATIC_INLINE void tim13_IT_start()
{
    LL_TIM_EnableIT_UPDATE(TIM13);
    LL_TIM_EnableCounter(TIM13);
}

__STATIC_INLINE void tim13_IT_stop()
{
    LL_TIM_DisableCounter(TIM13);
    LL_TIM_DisableIT_UPDATE(TIM13);
}

void change_pattern_IT_user_handler();

void M_Project_init();
#endif /* end of __DRV_PROCESS_H__ */
