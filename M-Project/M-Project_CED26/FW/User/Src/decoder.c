#include "config.h"

void decode_init()
{
    decode_reset();
}

void decode_reset()
{
    DEC_EN_GPIO_HI();
    DEC_A_LO();
    DEC_B_LO();
    DEC_C_LO();
    DEC_D_LO();
    DEC_E_LO();
}

static void decode_g_1_8_set_ch_state(DECODE_CH_T ch)
{
    decode_g_1_8_set_en_state(DECODE_STATE_SET);
    decode_ch_select(ch);
    decode_g_1_8_set_en_state(DECODE_STATE_RESET);
}

static void decode_g_9_16_set_ch_state(DECODE_CH_T ch)
{
    decode_g_9_16_set_en_state(DECODE_STATE_SET);
    decode_ch_select(ch);
    decode_g_9_16_set_en_state(DECODE_STATE_RESET);
}

static void decode_g_17_24_set_ch_state(DECODE_CH_T ch)
{
    decode_g_17_24_set_en_state(DECODE_STATE_SET);
    decode_ch_select(ch);
    decode_g_17_24_set_en_state(DECODE_STATE_RESET);
}

static void decode_g_25_26_set_ch_state(DECODE_CH_T ch)
{
    decode_g_25_26_set_en_state(DECODE_STATE_SET);
    decode_ch_select(ch);
    decode_g_25_26_set_en_state(DECODE_STATE_RESET);
}

void decode_set_scan_line(uint8_t ch_num)
{
    uint8_t remain = (ch_num & 0x07);
    
    switch (ch_num)
    {
        case  0 : case  1 : case  2 : case  3 : case  4 : case  5: case  6 : case  7 : 
            decode_g_1_8_set_ch_state((DECODE_CH_T)remain);
        break;
        
        case  8 : case  9 : case 10 : case 11 : case 12 : case 13: case 14 : case 15 : 
            decode_g_9_16_set_ch_state((DECODE_CH_T)remain);
        break;
        
        case 16 : case 17 : case 18 : case 19 : case 20 : case 21: case 22 : case 23 : 
            decode_g_17_24_set_ch_state((DECODE_CH_T)remain);
        break;
        
        case 24 : case 25 : 
            decode_g_25_26_set_ch_state((DECODE_CH_T)remain);
        break;
        case DECODE_CH_MAX : 
            decode_reset();
        break;
    }
}

