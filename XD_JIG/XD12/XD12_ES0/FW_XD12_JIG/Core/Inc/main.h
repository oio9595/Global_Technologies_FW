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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#ifdef __XDIC_MAIN_C__ 
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define VA_GENERIC(_1, _2, _3, _4, _5, _6,x, ...) x
// #define Command_Param_is_(x, ...) (sscanf(str_in, x, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_Param_is_(a, b, ...) (sscanf(str_in, a b, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_is_(x) (strncmp(str_in, x, strlen(x)) == 0)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
EXTERN void print(const char *fmt, ...);

EXTERN void comm_rx_handler(uint8_t rx);

EXTERN void sys_tick_handler(void);

EXTERN uint8_t gu8_freq_input_dma_flg;

EXTERN void set_input_freq_init(void);
EXTERN uint32_t get_input_freq(void);
EXTERN void calc_input_freq(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define LTC_HIGH_CURRENT_Pin LL_GPIO_PIN_0
#define LTC_HIGH_CURRENT_GPIO_Port GPIOC
#define ADC_SPI_MOSI_Pin LL_GPIO_PIN_1
#define ADC_SPI_MOSI_GPIO_Port GPIOC
#define ADC_SPI_MISO_Pin LL_GPIO_PIN_2
#define ADC_SPI_MISO_GPIO_Port GPIOC
#define DOUT_Q4_Pin LL_GPIO_PIN_0
#define DOUT_Q4_GPIO_Port GPIOA
#define LTC_LOW_CURRENT_Pin LL_GPIO_PIN_1
#define LTC_LOW_CURRENT_GPIO_Port GPIOA
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LTC_MID_CURRENT_Pin LL_GPIO_PIN_4
#define LTC_MID_CURRENT_GPIO_Port GPIOA
#define ADC_DRDY_Pin LL_GPIO_PIN_4
#define ADC_DRDY_GPIO_Port GPIOC
#define ADC_DRDY_EXTI_IRQn EXTI4_IRQn
#define KEY_TRIMMING_Pin LL_GPIO_PIN_5
#define KEY_TRIMMING_GPIO_Port GPIOC
#define XD12_ADO_Pin LL_GPIO_PIN_0
#define XD12_ADO_GPIO_Port GPIOB
#define ENABLE_SELECT4_Pin LL_GPIO_PIN_1
#define ENABLE_SELECT4_GPIO_Port GPIOB
#define ENABLE_SELECT3_Pin LL_GPIO_PIN_2
#define ENABLE_SELECT3_GPIO_Port GPIOB
#define ADC_SPI_CLK_Pin LL_GPIO_PIN_10
#define ADC_SPI_CLK_GPIO_Port GPIOB
#define KEY_POWER_Pin LL_GPIO_PIN_13
#define KEY_POWER_GPIO_Port GPIOB
#define XD12_5_7V_Pin LL_GPIO_PIN_14
#define XD12_5_7V_GPIO_Port GPIOB
#define XD12_5_0V_Pin LL_GPIO_PIN_15
#define XD12_5_0V_GPIO_Port GPIOB
#define BUZZER_PWM_Pin LL_GPIO_PIN_6
#define BUZZER_PWM_GPIO_Port GPIOC
#define VSYNC_Pin LL_GPIO_PIN_7
#define VSYNC_GPIO_Port GPIOC
#define ENABLE_SELECT1_Pin LL_GPIO_PIN_8
#define ENABLE_SELECT1_GPIO_Port GPIOC
#define MCLK_Pin LL_GPIO_PIN_9
#define MCLK_GPIO_Port GPIOC
#define PWM_OUT_Pin LL_GPIO_PIN_8
#define PWM_OUT_GPIO_Port GPIOA
#define ADC_CS_Pin LL_GPIO_PIN_9
#define ADC_CS_GPIO_Port GPIOA
#define ADC_START_Pin LL_GPIO_PIN_10
#define ADC_START_GPIO_Port GPIOA
#define LED_TRIM_Pin LL_GPIO_PIN_11
#define LED_TRIM_GPIO_Port GPIOA
#define ENABLE_SELECT2_Pin LL_GPIO_PIN_12
#define ENABLE_SELECT2_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define CNT_MR_Pin LL_GPIO_PIN_10
#define CNT_MR_GPIO_Port GPIOC
#define XD12_TYPE_DETECT_Pin LL_GPIO_PIN_11
#define XD12_TYPE_DETECT_GPIO_Port GPIOC
#define LTC_DISCHARGE_Pin LL_GPIO_PIN_12
#define LTC_DISCHARGE_GPIO_Port GPIOC
#define SWO_Pin LL_GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define VLED_CTR_9V_Pin LL_GPIO_PIN_4
#define VLED_CTR_9V_GPIO_Port GPIOB
#define XD12_VCC_EN_Pin LL_GPIO_PIN_5
#define XD12_VCC_EN_GPIO_Port GPIOB
#define TP_Pin LL_GPIO_PIN_7
#define TP_GPIO_Port GPIOB
#define ADC_RESET_Pin LL_GPIO_PIN_8
#define ADC_RESET_GPIO_Port GPIOB
#define NSCS_Pin LL_GPIO_PIN_9
#define NSCS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define ADS114S08_DEV_NUM   (1U)

#define FREQ_IN_IC_LENGTH   (512)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
