/** @file JigBd_IF.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __JIG_BD_IF_H__
#define __JIG_BD_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define XD_TIMEOUT_MS       (2)
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern volatile bool gb_pwm_dma_tx_flag;
extern volatile bool gb_pwm_is_rx_flag;
extern volatile uint16_t gn_xd_rx_timeout;
extern volatile bool gb_xd_timeout_event;
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
extern void LED_Select_Color(uint8_t color);
extern void LED_Select_Brightness(uint8_t brightness);
extern void LED_Select_Brightness_Up(void);
extern void LED_Select_Brightness_Down(void);
extern void LED_Select_Pattern(uint8_t pattern);
extern void LED_Update_Buffer(void);

extern void us_delay(uint16_t n_delay);
extern void JigBD_IF_Link_DMA_With_Buffer(void);

static inline void Serialize_Tx_Start(uint32_t len)
{
    gb_pwm_dma_tx_flag = true;

    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, len);
    //LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_1);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);
    LL_TIM_EnableCounter(TIM1);
}

static inline void Serialize_Tx_Done(void)
{
    while (LL_TIM_IsActiveFlag_UPDATE(TIM1) == 0);  // ensure the update event occurred
    LL_TIM_ClearFlag_UPDATE(TIM1);

    LL_TIM_DisableCounter(TIM1);
    LL_DMA_ClearFlag_TC1(DMA2);
    //LL_DMA_DisableIT_TC(DMA2, LL_DMA_STREAM_1);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);
}

static inline void Serialize_Rx_Start(uint32_t len)
{
    gn_xd_rx_timeout = XD_TIMEOUT_MS;
    gb_xd_timeout_event = false;

    /* output enable set HIGH */
    PWM_SWITCH_HI();

    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, len);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, len);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);

    LL_TIM_EnableDMAReq_CC1(TIM2);
    LL_TIM_EnableDMAReq_CC2(TIM2);

    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    while(LL_DMA_IsActiveFlag_TC6(DMA1) == 0)
    {
        if (gn_xd_rx_timeout == 0)
        {
            gb_xd_timeout_event = true;
            break;
        }
    }
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, 0);
    LL_DMA_ClearFlag_TC6(DMA1);

    while(LL_DMA_IsActiveFlag_TC5(DMA1) == 0)
    {
        if (gn_xd_rx_timeout == 0)
        {
            gb_xd_timeout_event = true;
            break;
        }
    }
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, 0);
    LL_DMA_ClearFlag_TC5(DMA1);

    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_TIM_DisableDMAReq_CC1(TIM2);
    LL_TIM_DisableDMAReq_CC2(TIM2);

    /* output enable set LOW */
    PWM_SWITCH_LO();
    gb_pwm_is_rx_flag = false;
}

extern void MCU_IF_Write_XDIC(uint8_t in_addr, uint16_t in_data);
extern uint16_t MCU_IF_Read_XDIC(uint8_t in_addr);
extern void MCU_IF_Write_LD(void);
extern uint16_t MCU_IF_Fault_Read_Command(void);
extern void MCU_IF_IdGen_Command();
extern void MCU_IF_SyncGen_Command();

/* USER CODE END */

#ifdef __cplusplus
}
#endif

#endif /* ~__JIG_BD_IF_H__ */

/*** end of file ***/