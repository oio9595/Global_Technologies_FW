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
#define VA_GENERIC(_1, _2, _3, _4, _5, _6, x, ...) x
#define Command_Param_is_(a, b, ...) (sscanf(str_in, a b, ##__VA_ARGS__)==VA_GENERIC(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define Command_is_(x) (strncmp(str_in, x, strlen(x)) == 0)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
typedef enum tag_LOG_LV_T
{
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_ERROR,
    LOG_MAX,
} LOG_LV_T;

extern void print(LOG_LV_T log_lv, const char *fmt, ...);
extern void comm_rx_handler(uint8_t rx);
extern void sys_tick_handler(void);

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
#define SERIAL_SWITCH_Pin LL_GPIO_PIN_3
#define SERIAL_SWITCH_GPIO_Port GPIOC
#define XD_FREQ_INPUT_Pin LL_GPIO_PIN_0
#define XD_FREQ_INPUT_GPIO_Port GPIOA
#define LTC_LOW_CURRENT_Pin LL_GPIO_PIN_1
#define LTC_LOW_CURRENT_GPIO_Port GPIOA
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LTC_MID_CURRENT_Pin LL_GPIO_PIN_4
#define LTC_MID_CURRENT_GPIO_Port GPIOA
#define FPGA_SPI_CLK_Pin LL_GPIO_PIN_5
#define FPGA_SPI_CLK_GPIO_Port GPIOA
#define FPGA_SPI_MISO_Pin LL_GPIO_PIN_6
#define FPGA_SPI_MISO_GPIO_Port GPIOA
#define FPGA_SPI_MOSI_Pin LL_GPIO_PIN_7
#define FPGA_SPI_MOSI_GPIO_Port GPIOA
#define ADC_DRDY_Pin LL_GPIO_PIN_4
#define ADC_DRDY_GPIO_Port GPIOC
#define ADC_DRDY_EXTI_IRQn EXTI4_IRQn
#define FPGA_UART_Rx_Pin LL_GPIO_PIN_5
#define FPGA_UART_Rx_GPIO_Port GPIOC
#define XD_ADO_Pin LL_GPIO_PIN_0
#define XD_ADO_GPIO_Port GPIOB
#define XD_CH_SEL4_Pin LL_GPIO_PIN_1
#define XD_CH_SEL4_GPIO_Port GPIOB
#define ADC_SPI_CLK_Pin LL_GPIO_PIN_10
#define ADC_SPI_CLK_GPIO_Port GPIOB
#define XD_CH_SEL3_Pin LL_GPIO_PIN_12
#define XD_CH_SEL3_GPIO_Port GPIOB
#define nINT_LD_Pin LL_GPIO_PIN_13
#define nINT_LD_GPIO_Port GPIOB
#define XD_5V5_Pin LL_GPIO_PIN_14
#define XD_5V5_GPIO_Port GPIOB
#define XD_FREQ_RESET_Pin LL_GPIO_PIN_15
#define XD_FREQ_RESET_GPIO_Port GPIOB
#define XD_SVSYNC_Pin LL_GPIO_PIN_6
#define XD_SVSYNC_GPIO_Port GPIOC
#define FPGA_VSYNC_Pin LL_GPIO_PIN_7
#define FPGA_VSYNC_GPIO_Port GPIOC
#define XD_CH_SEL1_Pin LL_GPIO_PIN_8
#define XD_CH_SEL1_GPIO_Port GPIOC
#define FPGA_MCLK_Pin LL_GPIO_PIN_9
#define FPGA_MCLK_GPIO_Port GPIOC
#define XD_SERIAL_OUT_Pin LL_GPIO_PIN_8
#define XD_SERIAL_OUT_GPIO_Port GPIOA
#define ADC_CS_Pin LL_GPIO_PIN_9
#define ADC_CS_GPIO_Port GPIOA
#define nINT_FT_Pin LL_GPIO_PIN_10
#define nINT_FT_GPIO_Port GPIOA
#define XD_CH_SEL2_Pin LL_GPIO_PIN_12
#define XD_CH_SEL2_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define FPGA_UART_Tx_Pin LL_GPIO_PIN_10
#define FPGA_UART_Tx_GPIO_Port GPIOC
#define DEBUG_Pin LL_GPIO_PIN_2
#define DEBUG_GPIO_Port GPIOD
#define XD_SERIAL_IN_Pin LL_GPIO_PIN_3
#define XD_SERIAL_IN_GPIO_Port GPIOB
#define VLED_CTR_9V_Pin LL_GPIO_PIN_4
#define VLED_CTR_9V_GPIO_Port GPIOB
#define XD_VCC_EN_Pin LL_GPIO_PIN_5
#define XD_VCC_EN_GPIO_Port GPIOB
#define FPGA_FLLSYNC_Pin LL_GPIO_PIN_6
#define FPGA_FLLSYNC_GPIO_Port GPIOB
#define FPGA_NSCS_Pin LL_GPIO_PIN_9
#define FPGA_NSCS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define FREQ_IN_IC_LENGTH       (512U)

#define TIM5_FREQ               (90000000.0f)

#define UART_BACKSPACE          (0x08U)

#define XD_CH_SEL_1_HI()        XD_CH_SEL1_GPIO_Port->BSRR = XD_CH_SEL1_Pin <<  0U
#define XD_CH_SEL_1_LO()        XD_CH_SEL1_GPIO_Port->BSRR = XD_CH_SEL1_Pin << 16U

#define XD_CH_SEL_2_HI()        XD_CH_SEL2_GPIO_Port->BSRR = XD_CH_SEL2_Pin <<  0U
#define XD_CH_SEL_2_LO()        XD_CH_SEL2_GPIO_Port->BSRR = XD_CH_SEL2_Pin << 16U

#define XD_CH_SEL_3_HI()        XD_CH_SEL3_GPIO_Port->BSRR = XD_CH_SEL3_Pin <<  0U
#define XD_CH_SEL_3_LO()        XD_CH_SEL3_GPIO_Port->BSRR = XD_CH_SEL3_Pin << 16U

#define XD_CH_SEL_4_HI()        XD_CH_SEL4_GPIO_Port->BSRR = XD_CH_SEL4_Pin <<  0U
#define XD_CH_SEL_4_LO()        XD_CH_SEL4_GPIO_Port->BSRR = XD_CH_SEL4_Pin << 16U

#define XC_NSCS_HI()            FPGA_NSCS_GPIO_Port->BSRR = FPGA_NSCS_Pin <<  0U
#define XC_NSCS_LO()            FPGA_NSCS_GPIO_Port->BSRR = FPGA_NSCS_Pin << 16U

#define DEBUG_HI()              DEBUG_GPIO_Port->BSRR = DEBUG_Pin <<  0U
#define DEBUG_LO()              DEBUG_GPIO_Port->BSRR = DEBUG_Pin << 16U

#define SERIAL_SWITCH_HI()      SERIAL_SWITCH_GPIO_Port->BSRR = SERIAL_SWITCH_Pin <<  0U
#define SERIAL_SWITCH_LO()      SERIAL_SWITCH_GPIO_Port->BSRR = SERIAL_SWITCH_Pin << 16U

#define ADS_CS_HI()             ADC_CS_GPIO_Port->BSRR = ADC_CS_Pin <<  0U
#define ADS_CS_LO()             ADC_CS_GPIO_Port->BSRR = ADC_CS_Pin << 16U

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
