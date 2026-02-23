/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <float.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define VA_GENERIC(_1, _2, _3, _4, _5, _6,x, ...) x
// #define Command_Param_is_(x, ...) (sscanf(str_in, x, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_Param_is_(a, b, ...) (sscanf(str_in, a b, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_is_(x) (strncmp(str_in, x, strlen(x)) == 0)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern void sys_tick_handler(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define PWM_SWITCH_Pin LL_GPIO_PIN_3
#define PWM_SWITCH_GPIO_Port GPIOC
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define SVSYNC_Pin LL_GPIO_PIN_6
#define SVSYNC_GPIO_Port GPIOC
#define VSYNC_Pin LL_GPIO_PIN_7
#define VSYNC_GPIO_Port GPIOC
#define DEBUG_Pin LL_GPIO_PIN_8
#define DEBUG_GPIO_Port GPIOC
#define SERIAL_OUT_Pin LL_GPIO_PIN_8
#define SERIAL_OUT_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define VLED_R_EN_Pin LL_GPIO_PIN_10
#define VLED_R_EN_GPIO_Port GPIOC
#define VLED_EN_Pin LL_GPIO_PIN_11
#define VLED_EN_GPIO_Port GPIOC
#define VLED_G_SW_Pin LL_GPIO_PIN_12
#define VLED_G_SW_GPIO_Port GPIOC
#define VLED_G_OFF_Pin LL_GPIO_PIN_2
#define VLED_G_OFF_GPIO_Port GPIOD
#define VLED_B_SW_Pin LL_GPIO_PIN_8
#define VLED_B_SW_GPIO_Port GPIOB
#define VLED_B_OFF_Pin LL_GPIO_PIN_9
#define VLED_B_OFF_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define DEBUG_HI()              DEBUG_GPIO_Port->BSRR = DEBUG_Pin <<  0U
#define DEBUG_LO()              DEBUG_GPIO_Port->BSRR = DEBUG_Pin << 16U

#define PWM_SWITCH_HI()         PWM_SWITCH_GPIO_Port->BSRR = PWM_SWITCH_Pin <<  0U
#define PWM_SWITCH_LO()         PWM_SWITCH_GPIO_Port->BSRR = PWM_SWITCH_Pin << 16U
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
