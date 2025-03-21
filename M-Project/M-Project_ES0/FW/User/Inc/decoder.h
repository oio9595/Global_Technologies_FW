#ifndef __DECODER_H__
#define __DECODER_H__
#include "config.h"

#define DECODE_CH_MAX   255

typedef enum
{
    DECODE_STATE_SET = 0, 
    DECODE_STATE_RESET = 1, 
}DECODE_STATE_T;

typedef enum
{
    DECODE_Y0 = 0, 
    DECODE_Y1, 
    DECODE_Y2, 
    DECODE_Y3, 
    DECODE_Y4, 
    DECODE_Y5, 
    DECODE_Y6, 
    DECODE_Y7, 
}DECODE_CH_T;
    
void decode_init();
void decode_reset();

static void decode_g_1_8_set_ch_state(DECODE_CH_T ch);
static void decode_g_9_16_set_ch_state(DECODE_CH_T ch);
static void decode_g_17_24_set_ch_state(DECODE_CH_T ch);
static void decode_g_25_26_set_ch_state(DECODE_CH_T ch);

void decode_set_scan_line(uint8_t ch_num);

__STATIC_INLINE void decode_ch_select(DECODE_CH_T ch)
{
    switch (ch)
    {
        case DECODE_Y0 : 
            DEC_A_LO();
            DEC_B_LO();
            DEC_C_LO();
            break;
        case DECODE_Y1 : 
            DEC_A_HI();
            DEC_B_LO();
            DEC_C_LO();
            break;
        case DECODE_Y2 : 
            DEC_A_LO();
            DEC_B_HI();
            DEC_C_LO();
            break;
        case DECODE_Y3 : 
            DEC_A_HI();
            DEC_B_HI();
            DEC_C_LO();
            break;
        case DECODE_Y4 : 
            DEC_A_LO();
            DEC_B_LO();
            DEC_C_HI();
            break;
        case DECODE_Y5 : 
            DEC_A_HI();
            DEC_B_LO();
            DEC_C_HI();
            break;
        case DECODE_Y6 : 
            DEC_A_LO();
            DEC_B_HI();
            DEC_C_HI();
            break;
        case DECODE_Y7 : 
            DEC_A_HI();
            DEC_B_HI();
            DEC_C_HI();
            break;
    }
}

__STATIC_INLINE void decode_g_1_8_set_en_state(DECODE_STATE_T state)
{
    if (state == DECODE_STATE_SET)
    {
        DEC_EN_GPIO_HI();
    }
    else
    {
        DEC_D_LO();
        DEC_E_LO();
        DEC_EN_GPIO_LO();
   }
}

__STATIC_INLINE void decode_g_9_16_set_en_state(DECODE_STATE_T state)
{
    if (state == DECODE_STATE_SET)
    {
        DEC_EN_GPIO_HI();
    }
    else
    {
        DEC_D_HI();
        DEC_E_LO();
        DEC_EN_GPIO_LO();
   }
}

__STATIC_INLINE void decode_g_17_24_set_en_state(DECODE_STATE_T state)
{
    if (state == DECODE_STATE_SET)
    {
        DEC_EN_GPIO_HI();
    }
    else
    {
        DEC_D_LO();
        DEC_E_HI();
        DEC_EN_GPIO_LO();
   }
}

__STATIC_INLINE void decode_g_25_26_set_en_state(DECODE_STATE_T state)
{
    if (state == DECODE_STATE_SET)
    {
        DEC_EN_GPIO_HI();
    }
    else
    {
        DEC_D_HI();
        DEC_E_HI();
        DEC_EN_GPIO_LO();
   }
}
#endif /* end of __DECODER_H__ */
