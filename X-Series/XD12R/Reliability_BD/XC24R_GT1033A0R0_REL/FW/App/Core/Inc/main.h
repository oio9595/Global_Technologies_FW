/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define BUFFER_OE_Pin LL_GPIO_PIN_3
#define BUFFER_OE_GPIO_Port GPIOC
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define XC_VCC_EN_Pin LL_GPIO_PIN_5
#define XC_VCC_EN_GPIO_Port GPIOC
#define XD_SVSYNC_Pin LL_GPIO_PIN_6
#define XD_SVSYNC_GPIO_Port GPIOC
#define VSYNC_OUT_Pin LL_GPIO_PIN_7
#define VSYNC_OUT_GPIO_Port GPIOC
#define XD_PWM_OUT_Pin LL_GPIO_PIN_8
#define XD_PWM_OUT_GPIO_Port GPIOA
#define DEBUG_Pin LL_GPIO_PIN_11
#define DEBUG_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define XC_5V5_EN_Pin LL_GPIO_PIN_10
#define XC_5V5_EN_GPIO_Port GPIOC
#define XCR_NSS_Pin LL_GPIO_PIN_9
#define XCR_NSS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define APB1_TIM_CLK            ( 90000000UL)   /* 2/3/4/5/12/13/14, SPI2/3 */
#define APB2_TIM_CLK            (180000000UL)   /* 1/8/9/10/11, SPI1/4 */

/* XD_PWM_OUT, TIMER1_CH1 */
#define XD_PWM_OUT_FREQ         (1000000UL)     /* 1us, 1MHz */
#define TIM1_PRESCALER          (0U)
#define TIM1_CLK                (APB2_TIM_CLK / (TIM1_PRESCALER + 1U))   /* 180MHz */
#define TIM1_PERIOD             ((TIM1_CLK / XD_PWM_OUT_FREQ) - 1U)             /* (180 - 1) */

/* SVSYNC_OUT, TIMER3_CH1 */
#define TIM3_PRESCALER          (0U)
#define TIM3_CLK                (APB1_TIM_CLK / (TIM3_PRESCALER + 1U))
#define TIM3_PERIOD             ((TIM3_CLK / 50000U) - 1U)
#define TIM3_PERIOD_HZ(f)       ((uint32_t)((TIM3_CLK / (f) + 0.5f) - 1U))
#define SVSYNC_OUT_PULSE(us)    ((uint32_t)(((us) * TIM3_CLK) / 1000000UL)) /* 10us pwm width */

/* VSYNC_OUT, TIMER8 */
#define TIM8_PRESCALER          (29U)
#define TIM8_CLK                ((float)APB2_TIM_CLK / (TIM8_PRESCALER + 1U))
#define TIM8_PERIOD             ((uint16_t)((TIM8_CLK / 120U) - 1U))
#define TIM8_PERIOD_HZ(f)       ((uint32_t)((TIM8_CLK / (f)) - 1U))
#define VSYNC_OUT_PULSE         ((uint16_t)((100.0f * TIM8_CLK) / 1000000UL)) /* 10us pwm width */

/* us_delay, TIMER12 */
#define TIM12_PRESCALER         ((uint16_t)((APB1_TIM_CLK/1000000UL) - 1U))    /* prescaler for 1us */
#define TIM12_CLK               ((float)APB1_TIM_CLK / (TIM12_PRESCALER + 1U))
#define TIM12_PERIOD            (65535UL)

/* FLLSYNC, TIMER4 */
#define TIM4_PRESCALER          (14U)
#define TIM4_PERIOD             (59999UL)
#define TIM4_CLK                ((float)APB1_TIM_CLK / ((TIM4_PRESCALER + 1U) * (TIM4_PERIOD + 1U)))
#define FLLSYNC_OUT_PULSE       ((uint16_t)((10.0f * TIM4_CLK) / 1000000UL)) /* 10us pwm width */

/* XD_PWM_IN, TIMER2 */


/* FREQ_INPUT, TIMER5 */
#define TIM5_PRESCALER      (0U)
#define TIM5_CLK            (APB1_TIM_CLK / (TIM5_PRESCALER + 1U))
#define TIM5_PERIOD         (0xFFFFFFFFUL)

#define CONST_HZ_TO_MHZ     (1000000.0f)
#define HW_OSC_PRESCALE     (128U)
#define XCR_PRESCALE        (64U)
#define XDR_PRESCALE        (64U)
//#define XCR_CONST_OSC       (HW_OSC_PRESCALE * XCR_PRESCALE / CONST_HZ_TO_MHZ)
#define XCR_CONST_OSC       (HW_OSC_PRESCALE / CONST_HZ_TO_MHZ)
#define XDR_CONST_OSC       (HW_OSC_PRESCALE * XDR_PRESCALE / CONST_HZ_TO_MHZ)

__STATIC_INLINE void us_delay(uint32_t n_delay)
{
    if(n_delay > 65535)
    {
        n_delay = 65535U;
    }

    LL_TIM_EnableCounter(TIM12);
    while(TIM12->CNT < n_delay)   {  }
    LL_TIM_DisableCounter(TIM12);
    TIM12->CNT = 0;
}

__STATIC_INLINE void user_delay(volatile uint32_t delay)
{
    while(delay--)
    {
        __NOP();
    }
}

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
