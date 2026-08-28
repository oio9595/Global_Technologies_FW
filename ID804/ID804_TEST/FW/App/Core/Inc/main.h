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
void MX_I2C1_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define SIO2_LVDS_Pin LL_GPIO_PIN_0
#define SIO2_LVDS_GPIO_Port GPIOC
#define MCU_TM1_Pin LL_GPIO_PIN_2
#define MCU_TM1_GPIO_Port GPIOC
#define ID804_VLED_EN_Pin LL_GPIO_PIN_3
#define ID804_VLED_EN_GPIO_Port GPIOC
#define ID804_FREQ_IN_Pin LL_GPIO_PIN_0
#define ID804_FREQ_IN_GPIO_Port GPIOA
#define CAN_ID804_Pin LL_GPIO_PIN_1
#define CAN_ID804_GPIO_Port GPIOA
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define MCU_TM0_Pin LL_GPIO_PIN_4
#define MCU_TM0_GPIO_Port GPIOA
#define SIO2_MCU_Pin LL_GPIO_PIN_5
#define SIO2_MCU_GPIO_Port GPIOC
#define SIO1_LVDS_Pin LL_GPIO_PIN_0
#define SIO1_LVDS_GPIO_Port GPIOB
#define CNT_MR_Pin LL_GPIO_PIN_1
#define CNT_MR_GPIO_Port GPIOB
#define PN_CROSS_Pin LL_GPIO_PIN_10
#define PN_CROSS_GPIO_Port GPIOB
#define SIO1_CAN_Pin LL_GPIO_PIN_13
#define SIO1_CAN_GPIO_Port GPIOB
#define SIO2_CAN_Pin LL_GPIO_PIN_15
#define SIO2_CAN_GPIO_Port GPIOB
#define I2C_SIO1P_UP_Pin LL_GPIO_PIN_9
#define I2C_SIO1P_UP_GPIO_Port GPIOC
#define SIO1_MCU_Pin LL_GPIO_PIN_8
#define SIO1_MCU_GPIO_Port GPIOA
#define ID804_VCC_5V5_Pin LL_GPIO_PIN_10
#define ID804_VCC_5V5_GPIO_Port GPIOA
#define ID804_VCC_LED_Pin LL_GPIO_PIN_11
#define ID804_VCC_LED_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SPI2_SIO2P_Pin LL_GPIO_PIN_15
#define SPI2_SIO2P_GPIO_Port GPIOA
#define ID804_VCC_EN_Pin LL_GPIO_PIN_4
#define ID804_VCC_EN_GPIO_Port GPIOB
#define I2C_SIO1N_UP_Pin LL_GPIO_PIN_8
#define I2C_SIO1N_UP_GPIO_Port GPIOB
#define I2C_SIO1N_DOWN_Pin LL_GPIO_PIN_9
#define I2C_SIO1N_DOWN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
