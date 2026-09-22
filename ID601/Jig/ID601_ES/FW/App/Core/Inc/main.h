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

#include "stm32f4xx_ll_adc.h"
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
#define LTC_CURRENT_HIGH_Pin LL_GPIO_PIN_0
#define LTC_CURRENT_HIGH_GPIO_Port GPIOC
#define ADC_MOSI_Pin LL_GPIO_PIN_1
#define ADC_MOSI_GPIO_Port GPIOC
#define ADC_MISO_Pin LL_GPIO_PIN_2
#define ADC_MISO_GPIO_Port GPIOC
#define BUFFER_OE_Pin LL_GPIO_PIN_3
#define BUFFER_OE_GPIO_Port GPIOC
#define CNT_FREQ_IN_Pin LL_GPIO_PIN_0
#define CNT_FREQ_IN_GPIO_Port GPIOA
#define LTC_CURRENT_LOW_Pin LL_GPIO_PIN_1
#define LTC_CURRENT_LOW_GPIO_Port GPIOA
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LTC_CURRENT_MID_Pin LL_GPIO_PIN_4
#define LTC_CURRENT_MID_GPIO_Port GPIOA
#define IC603_CLK_Pin LL_GPIO_PIN_5
#define IC603_CLK_GPIO_Port GPIOA
#define ID603_MISO_Pin LL_GPIO_PIN_6
#define ID603_MISO_GPIO_Port GPIOA
#define IC603_MOSI_Pin LL_GPIO_PIN_7
#define IC603_MOSI_GPIO_Port GPIOA
#define ADC_DRDY1_Pin LL_GPIO_PIN_4
#define ADC_DRDY1_GPIO_Port GPIOC
#define ADC_DRDY1_EXTI_IRQn EXTI4_IRQn
#define IC603_VCC_EN_Pin LL_GPIO_PIN_5
#define IC603_VCC_EN_GPIO_Port GPIOC
#define ID601_TEST_Pin LL_GPIO_PIN_0
#define ID601_TEST_GPIO_Port GPIOB
#define ID601_CH_MUX4_Pin LL_GPIO_PIN_1
#define ID601_CH_MUX4_GPIO_Port GPIOB
#define VLED_DCDC_EN_Pin LL_GPIO_PIN_2
#define VLED_DCDC_EN_GPIO_Port GPIOB
#define ADC_CLK_Pin LL_GPIO_PIN_10
#define ADC_CLK_GPIO_Port GPIOB
#define ID601_CH_MUX3_Pin LL_GPIO_PIN_12
#define ID601_CH_MUX3_GPIO_Port GPIOB
#define IC603_nINT_LD_Pin LL_GPIO_PIN_13
#define IC603_nINT_LD_GPIO_Port GPIOB
#define IC603_nINT_LD_EXTI_IRQn EXTI15_10_IRQn
#define ID601_5V5_EN_Pin LL_GPIO_PIN_14
#define ID601_5V5_EN_GPIO_Port GPIOB
#define CNT_RST_Pin LL_GPIO_PIN_15
#define CNT_RST_GPIO_Port GPIOB
#define ID601_SVSYNC_Pin LL_GPIO_PIN_6
#define ID601_SVSYNC_GPIO_Port GPIOC
#define VSYNC_Pin LL_GPIO_PIN_7
#define VSYNC_GPIO_Port GPIOC
#define ID601_CH_MUX1_Pin LL_GPIO_PIN_8
#define ID601_CH_MUX1_GPIO_Port GPIOC
#define IC603_MCLK_Pin LL_GPIO_PIN_9
#define IC603_MCLK_GPIO_Port GPIOC
#define ID601_SERIAL_OUT_Pin LL_GPIO_PIN_8
#define ID601_SERIAL_OUT_GPIO_Port GPIOA
#define ADC_CS1_Pin LL_GPIO_PIN_9
#define ADC_CS1_GPIO_Port GPIOA
#define IC603_nINT_FAULT_Pin LL_GPIO_PIN_10
#define IC603_nINT_FAULT_GPIO_Port GPIOA
#define IC603_nINT_FAULT_EXTI_IRQn EXTI15_10_IRQn
#define ID601_CH_MUX2_Pin LL_GPIO_PIN_12
#define ID601_CH_MUX2_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define ADC_CS2_Pin LL_GPIO_PIN_15
#define ADC_CS2_GPIO_Port GPIOA
#define IC603_5V5_EN_Pin LL_GPIO_PIN_10
#define IC603_5V5_EN_GPIO_Port GPIOC
#define IC603_FB3_Pin LL_GPIO_PIN_11
#define IC603_FB3_GPIO_Port GPIOC
#define IC603_FB2_Pin LL_GPIO_PIN_12
#define IC603_FB2_GPIO_Port GPIOC
#define IC603_FB1_Pin LL_GPIO_PIN_2
#define IC603_FB1_GPIO_Port GPIOD
#define ID601_SERIAL_IN_Pin LL_GPIO_PIN_3
#define ID601_SERIAL_IN_GPIO_Port GPIOB
#define ID601_VLED_EN_Pin LL_GPIO_PIN_4
#define ID601_VLED_EN_GPIO_Port GPIOB
#define ID601_VCC_EN_Pin LL_GPIO_PIN_5
#define ID601_VCC_EN_GPIO_Port GPIOB
#define IC603_FLLSYNC_Pin LL_GPIO_PIN_6
#define IC603_FLLSYNC_GPIO_Port GPIOB
#define ID601_TEST_RELAY_Pin LL_GPIO_PIN_7
#define ID601_TEST_RELAY_GPIO_Port GPIOB
#define ADC_DRDY2_Pin LL_GPIO_PIN_8
#define ADC_DRDY2_GPIO_Port GPIOB
#define ADC_DRDY2_EXTI_IRQn EXTI9_5_IRQn
#define IC603_CS_Pin LL_GPIO_PIN_9
#define IC603_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
