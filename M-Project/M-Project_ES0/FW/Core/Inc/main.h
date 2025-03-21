/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_rcc.h"
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
#define DMA_DEBUG_Pin LL_GPIO_PIN_3
#define DMA_DEBUG_GPIO_Port GPIOC
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin LL_GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define DEC_B_IN_Pin LL_GPIO_PIN_6
#define DEC_B_IN_GPIO_Port GPIOA
#define D1_IN_Pin LL_GPIO_PIN_7
#define D1_IN_GPIO_Port GPIOA
#define SCLK_IN_Pin LL_GPIO_PIN_15
#define SCLK_IN_GPIO_Port GPIOB
#define DEC_D_IN_Pin LL_GPIO_PIN_7
#define DEC_D_IN_GPIO_Port GPIOC
#define LAT_IN_Pin LL_GPIO_PIN_9
#define LAT_IN_GPIO_Port GPIOC
#define GCLK_IN_Pin LL_GPIO_PIN_9
#define GCLK_IN_GPIO_Port GPIOA
#define DEC_E_IN_Pin LL_GPIO_PIN_10
#define DEC_E_IN_GPIO_Port GPIOA
#define TRG_B_IN_Pin LL_GPIO_PIN_11
#define TRG_B_IN_GPIO_Port GPIOA
#define TRG_A_IN_Pin LL_GPIO_PIN_12
#define TRG_A_IN_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define DEBUG_PIN_Pin LL_GPIO_PIN_10
#define DEBUG_PIN_GPIO_Port GPIOC
#define DEBUG_2_Pin LL_GPIO_PIN_12
#define DEBUG_2_GPIO_Port GPIOC
#define SWO_Pin LL_GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define DEC_C_IN_Pin LL_GPIO_PIN_6
#define DEC_C_IN_GPIO_Port GPIOB
#define DEC_A_IN_Pin LL_GPIO_PIN_8
#define DEC_A_IN_GPIO_Port GPIOB
#define DEC_EN_GPIO_Pin LL_GPIO_PIN_9
#define DEC_EN_GPIO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define DEC_A_HI()          (DEC_A_IN_GPIO_Port->BSRR = (uint32_t) DEC_A_IN_Pin <<  0)
#define DEC_A_LO()          (DEC_A_IN_GPIO_Port->BSRR = (uint32_t) DEC_A_IN_Pin << 16)

#define DEC_B_HI()          (DEC_B_IN_GPIO_Port->BSRR = (uint32_t) DEC_B_IN_Pin <<  0)
#define DEC_B_LO()          (DEC_B_IN_GPIO_Port->BSRR = (uint32_t) DEC_B_IN_Pin << 16)

#define DEC_C_HI()          (DEC_C_IN_GPIO_Port->BSRR = (uint32_t) DEC_C_IN_Pin <<  0)
#define DEC_C_LO()          (DEC_C_IN_GPIO_Port->BSRR = (uint32_t) DEC_C_IN_Pin << 16)

#define DEC_D_HI()          (DEC_D_IN_GPIO_Port->BSRR = (uint32_t) DEC_D_IN_Pin <<  0)
#define DEC_D_LO()          (DEC_D_IN_GPIO_Port->BSRR = (uint32_t) DEC_D_IN_Pin << 16)

#define DEC_E_HI()          (DEC_E_IN_GPIO_Port->BSRR = (uint32_t) DEC_E_IN_Pin <<  0)
#define DEC_E_LO()          (DEC_E_IN_GPIO_Port->BSRR = (uint32_t) DEC_E_IN_Pin << 16)

#define DEC_EN_GPIO_HI()    (DEC_EN_GPIO_GPIO_Port->BSRR = (uint32_t) DEC_EN_GPIO_Pin <<  0)
#define DEC_EN_GPIO_LO()    (DEC_EN_GPIO_GPIO_Port->BSRR = (uint32_t) DEC_EN_GPIO_Pin << 16)

#define TLC_DATA_HI()       (D1_IN_GPIO_Port->BSRR = (uint32_t) D1_IN_Pin <<  0)
#define TLC_DATA_LO()       (D1_IN_GPIO_Port->BSRR = (uint32_t) D1_IN_Pin << 16)

#define TLC_LAT_HI()        (LAT_IN_GPIO_Port->BSRR = (uint32_t) LAT_IN_Pin <<  0)
#define TLC_LAT_LO()        (LAT_IN_GPIO_Port->BSRR = (uint32_t) LAT_IN_Pin << 16)

#define TLC_GCLK_HI()       (GCLK_IN_GPIO_Port->BSRR = (uint32_t) GCLK_IN_Pin <<  0)
#define TLC_GCLK_LO()       (GCLK_IN_GPIO_Port->BSRR = (uint32_t) GCLK_IN_Pin << 16)

#define TLC_SCLK_HI()       (SCLK_IN_GPIO_Port->BSRR = (uint32_t) SCLK_IN_Pin <<  0)
#define TLC_SCLK_LO()       (SCLK_IN_GPIO_Port->BSRR = (uint32_t) SCLK_IN_Pin << 16)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
