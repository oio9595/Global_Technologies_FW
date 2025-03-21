/** @file xd12.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __JIGBD_LATCH_C__
#include "config.h"

#ifdef LATCH_SUPPORT

// Latch Contorl
const uint16_t LATCH_PORT_DATA_MAP[] ={
    LS_D0_Pin, LS_D1_Pin, LS_D2_Pin, LS_D3_Pin, LS_D4_Pin, LS_D5_Pin, LS_D6_Pin, LS_D7_Pin
};
GPIO_TypeDef * LATCH_PORT_CTL_MAP[] ={
    GPIOC, GPIOC, GPIOA, GPIOB, GPIOB, GPIOA, GPIOA, GPIOA
};
uint16_t gui_latch_1_data = 0;
uint16_t gui_latch_2_data = 0;
uint16_t gui_latch_3_data = 0;
void LATCH_LOAD_DATA(uint8_t latch_data)
{
    for(int i=0;i<8;i++)
    {
        if(((latch_data >> i) & 0x01) == 0x01)
        {
            LL_GPIO_SetOutputPin(LATCH_PORT_CTL_MAP[i], (uint16_t)LATCH_PORT_DATA_MAP[i]);
        }
        else
        {
            LL_GPIO_ResetOutputPin(LATCH_PORT_CTL_MAP[i], (uint16_t)LATCH_PORT_DATA_MAP[i]);
        }
    }
}

void LATCH__WritePin(uint8_t latch_num, uint16_t latch_bit, uint8_t ub_val)
{
    if(ub_val == LATCH_PIN_SET)
    {
        switch(latch_num)
        {
            case 1:
                gui_latch_1_data |= (1 <<latch_bit);
                LATCH_LOAD_DATA(gui_latch_1_data);
                // uprint( "\r\n gui_latch_1_data: %x\r\n", gui_latch_1_data);
                //
                LL_GPIO_ResetOutputPin(GPIOC, LS_CP1_Pin);
                LL_GPIO_ResetOutputPin(LS_OE1_GPIO_Port, LS_OE1_Pin);
                // HAL_Delay(LATCH_DELAY);
                LL_GPIO_SetOutputPin(GPIOC, LS_CP1_Pin);
                break;
            case 2:
                gui_latch_2_data |= (1 <<latch_bit);
                LATCH_LOAD_DATA(gui_latch_2_data);
                // uprint( "\r\n gui_latch_2_data: %x\r\n", gui_latch_2_data);
                //
                // LL_GPIO_ResetOutputPin(GPIOA, LS_CP2_Pin);
                LL_GPIO_ResetOutputPin(GPIOA, LS_OE2_Pin);
                // HAL_Delay(LATCH_DELAY);
                // LL_GPIO_SetOutputPin(GPIOA, LS_CP2_Pin);
                break;
            case 3:
                gui_latch_3_data |= (1 <<latch_bit);
                LATCH_LOAD_DATA(gui_latch_3_data);
                LL_GPIO_ResetOutputPin(GPIOA, LS_CP3_Pin);
                LL_GPIO_ResetOutputPin(GPIOC, LS_OE3_Pin);
                // HAL_Delay(LATCH_DELAY);
                LL_GPIO_SetOutputPin(GPIOA, LS_CP3_Pin);
                break;
        }
    }
    else //LATCH_PIN_RESET
    {
        switch(latch_num)
        {
            case 1:
                gui_latch_1_data &= ~(1 <<latch_bit);
                LATCH_LOAD_DATA(gui_latch_1_data);
                // uprint( "\r\n gui_latch_1_data: %x\r\n", gui_latch_1_data);
                //
                LL_GPIO_ResetOutputPin(GPIOC, LS_CP1_Pin);
                LL_GPIO_ResetOutputPin(LS_OE1_GPIO_Port, LS_OE1_Pin);
                // HAL_Delay(LATCH_DELAY);
                LL_GPIO_SetOutputPin(GPIOC, LS_CP1_Pin);
                break;
            case 2:
                gui_latch_2_data &= ~(1 <<latch_bit);
                LATCH_LOAD_DATA(gui_latch_2_data);
                // uprint( "\r\n gui_latch_2_data: %x\r\n", gui_latch_2_data);
                //
                // LL_GPIO_ResetOutputPin(GPIOA, LS_CP2_Pin);
                LL_GPIO_ResetOutputPin(GPIOA, LS_OE2_Pin);
                // HAL_Delay(LATCH_DELAY);
                // LL_GPIO_SetOutputPin(GPIOA, LS_CP2_Pin);
                break;
            case 3:
                gui_latch_3_data &= ~(1 <<latch_bit);
                LATCH_LOAD_DATA(gui_latch_3_data);
                LL_GPIO_ResetOutputPin(GPIOA, LS_CP3_Pin);
                LL_GPIO_ResetOutputPin(GPIOC, LS_OE3_Pin);
                // HAL_Delay(LATCH_DELAY);
                LL_GPIO_SetOutputPin(GPIOA, LS_CP3_Pin);
                break;
        }
    }
}



#ifdef DBG_TEST
#endif //DBG_TEST

#endif //LATCH_SUPPORT
/*** end of file ***/