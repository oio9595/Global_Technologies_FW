/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>
#include <time.h>
#include "comm_debugging.h"
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
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define XC_SPI_SCLK_Pin LL_GPIO_PIN_5
#define XC_SPI_SCLK_GPIO_Port GPIOA
#define SPI_MISO_Pin LL_GPIO_PIN_6
#define SPI_MISO_GPIO_Port GPIOA
#define SPI_MOSI_Pin LL_GPIO_PIN_7
#define SPI_MOSI_GPIO_Port GPIOA
#define VSYNC_Pin LL_GPIO_PIN_7
#define VSYNC_GPIO_Port GPIOC
#define MCLK_Pin LL_GPIO_PIN_9
#define MCLK_GPIO_Port GPIOC
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define JIG_TYPE_Pin LL_GPIO_PIN_15
#define JIG_TYPE_GPIO_Port GPIOA
#define SWO_Pin LL_GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define XC_NSCS_Pin LL_GPIO_PIN_9
#define XC_NSCS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define CONST_MHz_TO_Hz             (1000000.0f)

#define APB1_TIM_CLK                (90000000UL)
#define TMR3_PRESCALER              (14)
#define VSYNC_CCR                   ((uint32_t)(((float)APB1_TIM_CLK / (TMR3_PRESCALER + 1)) / 120 * 0.003))   /* vsync : 120Hz, duty : 0.3% */

#define XD_DAISY_SIZE               (31)
#define XD_CH_SIZE                  (8)
#define XD_BLOCK_SIZE               (XD_DAISY_SIZE * XD_CH_SIZE)

#define XD_LOCAL_READ_HALF_SIZE     (16)

#define SPI_LD_BURST_SIZE           (16)
#define SPI_LD_REPEAT_NUM           ((XD_BLOCK_SIZE / SPI_LD_BURST_SIZE) + 1)

/* XC Serialize 1MHz */
// #define XC_SERIALIZE_FREQ           (1.0f) //MHz
// #define XC_SERIAL_CLK_CNT_LOW       (6)
// #define XC_SERIAL_CLK_CNT_HIGH      (10)

/* XC Serialize 1.6MHz */
// #define XC_SERIALIZE_FREQ           (1.6f) //MHz
// #define XC_SERIAL_CLK_CNT_LOW       (3)
// #define XC_SERIAL_CLK_CNT_HIGH      (7)

/* XC Serialize 2.0MHz */
#define XC_SERIALIZE_FREQ           (2.0f) //MHz
#define XC_SERIAL_CLK_CNT_LOW       (2)
#define XC_SERIAL_CLK_CNT_HIGH      (6)

#define XD_SERIAL_CLK_CNT_LOW       (13)
#define XD_SERIAL_CLK_CNT_HIGH      (26)

#define XD12_FREQ_ERR_RATE          (2.0)
#define XD12_FOSC_MIN               (39019200UL)
#define XD12_FOSC_TYP               (39319200UL)
#define XD12_FOSC_MAX               (39619200UL)

#define XD12_DELAY                  (((CONST_MHz_TO_Hz / XD12_FOSC_MIN) * (XD_SERIAL_CLK_CNT_HIGH + XD_SERIAL_CLK_CNT_LOW)))
#define XD12_WRITE_BITS             (22)
#define XD12_READ_BITS              (10)
#define XD12_READ_RECV_BITS         (21)
#define XD12_IDGEN_BITS             (4)

#define XD12_WRITE_DELAY            ((uint32_t)((XD12_DELAY * XD12_WRITE_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
#define XD12_IDGEN_DELAY            ((uint32_t)((XD12_DELAY * XD12_IDGEN_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
#define XD12_DELAY_DELAY            ((uint32_t)((XD12_DELAY * XD12_WRITE_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
#define XD12_READ_DELAY             ((uint32_t)((XD12_DELAY * XD12_READ_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
#define XD12_READ_RECV_DELAY        ((uint32_t)((XD12_DELAY * XD12_READ_RECV_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))

#define DELAY_READ_HALF             ((uint32_t)(((float)(10 * 31) / XC_SERIALIZE_FREQ) + (21 * 16)))
#define DELAY_READ_FULL             (21 * 15)

#define XC_NSCS_HI()                LL_GPIO_SetOutputPin(XC_NSCS_GPIO_Port, XC_NSCS_Pin)
#define XC_NSCS_LO()                LL_GPIO_ResetOutputPin(XC_NSCS_GPIO_Port, XC_NSCS_Pin)

__STATIC_INLINE void us_tdelay(uint16_t us_delay)
{
    LL_TIM_EnableCounter(TIM6);
    while(LL_TIM_GetCounter(TIM6) < us_delay)
    {
    }
    LL_TIM_DisableCounter(TIM6);
    LL_TIM_SetCounter(TIM6, 0);
}

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
