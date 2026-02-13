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
#define Command_Param_is_(a, b, ...) (sscanf(str_in, a b, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_is_(x) (strncmp(str_in, x, strlen(x)) == 0)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern void System_Tick_Handler(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define IP805_SFM_0_Pin LL_GPIO_PIN_0
#define IP805_SFM_0_GPIO_Port GPIOC
#define IP805_SFM_1_Pin LL_GPIO_PIN_1
#define IP805_SFM_1_GPIO_Port GPIOC
#define IP805_SFM_2_Pin LL_GPIO_PIN_2
#define IP805_SFM_2_GPIO_Port GPIOC
#define IP805_SFM_4_Pin LL_GPIO_PIN_3
#define IP805_SFM_4_GPIO_Port GPIOC
#define IP805_FREQ_IN_Pin LL_GPIO_PIN_0
#define IP805_FREQ_IN_GPIO_Port GPIOA
#define PC_UART_Tx_Pin LL_GPIO_PIN_2
#define PC_UART_Tx_GPIO_Port GPIOA
#define PC_UART_Rx_Pin LL_GPIO_PIN_3
#define PC_UART_Rx_GPIO_Port GPIOA
#define ADC_CS_Pin LL_GPIO_PIN_4
#define ADC_CS_GPIO_Port GPIOA
#define ADC_CLK_Pin LL_GPIO_PIN_5
#define ADC_CLK_GPIO_Port GPIOA
#define ADC_MISO_Pin LL_GPIO_PIN_6
#define ADC_MISO_GPIO_Port GPIOA
#define ADC_MOSI_Pin LL_GPIO_PIN_7
#define ADC_MOSI_GPIO_Port GPIOA
#define ADC_DRDY_Pin LL_GPIO_PIN_4
#define ADC_DRDY_GPIO_Port GPIOC
#define ADC_DRDY_EXTI_IRQn EXTI4_IRQn
#define IP805_I2C_SCL_Pin LL_GPIO_PIN_10
#define IP805_I2C_SCL_GPIO_Port GPIOB
#define LS_D7_Pin LL_GPIO_PIN_13
#define LS_D7_GPIO_Port GPIOB
#define LS_D6_Pin LL_GPIO_PIN_14
#define LS_D6_GPIO_Port GPIOB
#define LS_D5_Pin LL_GPIO_PIN_15
#define LS_D5_GPIO_Port GPIOB
#define LS_D4_Pin LL_GPIO_PIN_6
#define LS_D4_GPIO_Port GPIOC
#define LS_D3_Pin LL_GPIO_PIN_7
#define LS_D3_GPIO_Port GPIOC
#define LS_D2_Pin LL_GPIO_PIN_8
#define LS_D2_GPIO_Port GPIOC
#define LS_D1_Pin LL_GPIO_PIN_9
#define LS_D1_GPIO_Port GPIOC
#define LS_D0_Pin LL_GPIO_PIN_8
#define LS_D0_GPIO_Port GPIOA
#define IP805_UART_Tx_Pin LL_GPIO_PIN_9
#define IP805_UART_Tx_GPIO_Port GPIOA
#define IP805_UART_Rx_Pin LL_GPIO_PIN_10
#define IP805_UART_Rx_GPIO_Port GPIOA
#define IP805_CAN_Rx_Pin LL_GPIO_PIN_11
#define IP805_CAN_Rx_GPIO_Port GPIOA
#define IP805_CAN_Tx_Pin LL_GPIO_PIN_12
#define IP805_CAN_Tx_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define LS_EN2_Pin LL_GPIO_PIN_15
#define LS_EN2_GPIO_Port GPIOA
#define LS_EN1_Pin LL_GPIO_PIN_10
#define LS_EN1_GPIO_Port GPIOC
#define LS_EN0_Pin LL_GPIO_PIN_11
#define LS_EN0_GPIO_Port GPIOC
#define IP805_I2C_SDA_Pin LL_GPIO_PIN_12
#define IP805_I2C_SDA_GPIO_Port GPIOC
#define IP805_ATB_2_Pin LL_GPIO_PIN_4
#define IP805_ATB_2_GPIO_Port GPIOB
#define IP805_ATB_1_Pin LL_GPIO_PIN_5
#define IP805_ATB_1_GPIO_Port GPIOB
#define IP805_TM_SEL_1_Pin LL_GPIO_PIN_6
#define IP805_TM_SEL_1_GPIO_Port GPIOB
#define IP805_TM_SEL_0_Pin LL_GPIO_PIN_7
#define IP805_TM_SEL_0_GPIO_Port GPIOB
#define IP805_TM_Pin LL_GPIO_PIN_8
#define IP805_TM_GPIO_Port GPIOB
#define IP805_EN_Pin LL_GPIO_PIN_9
#define IP805_EN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define FREQ_IN_IC_LENGTH       (512)

#define TIM5_FREQ               (72000000.0f)

#define ADS_CS_HI()             ADC_CS_GPIO_Port->BSRR = ADC_CS_Pin <<  0U
#define ADS_CS_LO()             ADC_CS_GPIO_Port->BSRR = ADC_CS_Pin << 16U

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
