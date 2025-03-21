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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef ____MAIN_C__ 
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
/* USER CODE END Header */


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
#define USE_SPI_USER_FUNCTION

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define SPI2_MOSI_Pin GPIO_PIN_1
#define SPI2_MOSI_GPIO_Port GPIOC
#define SPI2_MISO_Pin GPIO_PIN_2
#define SPI2_MISO_GPIO_Port GPIOC
#define MODE1_1_Pin GPIO_PIN_3
#define MODE1_1_GPIO_Port GPIOC
#define ADC1_START_Pin GPIO_PIN_0
#define ADC1_START_GPIO_Port GPIOA
#define AQIC_PWR_CTL_Pin GPIO_PIN_1
#define AQIC_PWR_CTL_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define MODE2_1_Pin GPIO_PIN_4
#define MODE2_1_GPIO_Port GPIOA
#define SPI1_SCK_Pin GPIO_PIN_5
#define SPI1_SCK_GPIO_Port GPIOA
#define SPI1_MISO_Pin GPIO_PIN_6
#define SPI1_MISO_GPIO_Port GPIOA
#define SPI1_MOSI_Pin GPIO_PIN_7
#define SPI1_MOSI_GPIO_Port GPIOA
#define ADC1_DRDY_Pin GPIO_PIN_4
#define ADC1_DRDY_GPIO_Port GPIOC
#define ADC1_DRDY_EXTI_IRQn EXTI4_IRQn
#define LED_TRIM_Pin GPIO_PIN_5
#define LED_TRIM_GPIO_Port GPIOC
#define CQ24_DAC_Pin GPIO_PIN_0
#define CQ24_DAC_GPIO_Port GPIOB
#define ADC2_DRDY_Pin GPIO_PIN_1
#define ADC2_DRDY_GPIO_Port GPIOB
#define ADC2_DRDY_EXTI_IRQn EXTI1_IRQn
#define VSYNC_IN_Pin GPIO_PIN_2
#define VSYNC_IN_GPIO_Port GPIOB
#define ADC1_CS_Pin GPIO_PIN_10
#define ADC1_CS_GPIO_Port GPIOB
#define KEY_MODE_Pin GPIO_PIN_12
#define KEY_MODE_GPIO_Port GPIOB
#define SPI2_SCK_Pin GPIO_PIN_13
#define SPI2_SCK_GPIO_Port GPIOB
#define TEST_2_Pin GPIO_PIN_14
#define TEST_2_GPIO_Port GPIOB
#define BUZZER_PWM_Pin GPIO_PIN_15
#define BUZZER_PWM_GPIO_Port GPIOB
#define TEST_1_Pin GPIO_PIN_6
#define TEST_1_GPIO_Port GPIOC
#define NSCS1_Pin GPIO_PIN_7
#define NSCS1_GPIO_Port GPIOC
#define MCLK_Pin GPIO_PIN_9
#define MCLK_GPIO_Port GPIOC
#define KEY_RUN_Pin GPIO_PIN_8
#define KEY_RUN_GPIO_Port GPIOA
#define NSCS2_Pin GPIO_PIN_9
#define NSCS2_GPIO_Port GPIOA
#define CQ24_5_0V_Pin GPIO_PIN_11
#define CQ24_5_0V_GPIO_Port GPIOA
#define ADC_RESET_Pin GPIO_PIN_12
#define ADC_RESET_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define CQ24_5_7V_Pin GPIO_PIN_12
#define CQ24_5_7V_GPIO_Port GPIOC
#define ADC2_CS_Pin GPIO_PIN_2
#define ADC2_CS_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define CQ24_VCC_EN_Pin GPIO_PIN_5
#define CQ24_VCC_EN_GPIO_Port GPIOB
#define ADC2_START_Pin GPIO_PIN_8
#define ADC2_START_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define	CQIC1_NSCS_LO()         (NSCS1_GPIO_Port->BSRR = (uint32_t)NSCS1_Pin << 16U)
#define	CQIC1_NSCS_HI()         (NSCS1_GPIO_Port->BSRR = NSCS1_Pin)

#define	CQIC2_NSCS_LO()         (NSCS2_GPIO_Port->BSRR = (uint32_t)NSCS2_Pin << 16U)
#define	CQIC2_NSCS_HI()         (NSCS2_GPIO_Port->BSRR = NSCS2_Pin)

#define set_cqic_power_off()		(CQ24_VCC_EN_GPIO_Port->BSRR = (uint32_t)CQ24_VCC_EN_Pin)
#define set_cqic_power_on()			(CQ24_VCC_EN_GPIO_Port->BSRR = (uint32_t)CQ24_VCC_EN_Pin << 16U)

#define set_cqic_5_0V_off()	    (CQ24_5_0V_GPIO_Port->BSRR = (uint32_t)CQ24_5_0V_Pin << 16U)
#define set_cqic_5_0V_on()		  (CQ24_5_0V_GPIO_Port->BSRR = (uint32_t)CQ24_5_0V_Pin)

#define set_cqic_5_7V_off()	    (CQ24_5_7V_GPIO_Port->BSRR = (uint32_t)CQ24_5_7V_Pin << 16U)
#define set_cqic_5_7V_on()		  (CQ24_5_7V_GPIO_Port->BSRR = (uint32_t)CQ24_5_7V_Pin)

#define ADC_RESET_LO			      (ADC_RESET_GPIO_Port->BSRR = (uint32_t)ADC_RESET_Pin << 16U)	
#define ADC_RESET_HI			      (ADC_RESET_GPIO_Port->BSRR = ADC_RESET_Pin)	

#define ADC1_START_LO			      (ADC1_START_GPIO_Port->BSRR = (uint32_t)ADC1_START_Pin << 16U)	
#define ADC1_START_HI			      (ADC1_START_GPIO_Port->BSRR = ADC1_START_Pin)	

#define ADC2_START_LO			      (ADC2_START_GPIO_Port->BSRR = (uint32_t)ADC2_START_Pin << 16U)	
#define ADC2_START_HI			      (ADC2_START_GPIO_Port->BSRR = ADC2_START_Pin)	

#define ADC_CS1_LO				      (ADC1_CS_GPIO_Port->BSRR = (uint32_t)ADC1_CS_Pin << 16U)	
#define ADC_CS1_HI				      (ADC1_CS_GPIO_Port->BSRR = ADC1_CS_Pin)

#define ADC_CS2_LO				      (ADC2_CS_GPIO_Port->BSRR = (uint32_t)ADC2_CS_Pin << 16U)	
#define ADC_CS2_HI				      (ADC2_CS_GPIO_Port->BSRR = ADC2_CS_Pin)

#define VSYNC_OUT_LO()			    (VSYNC_IN_GPIO_Port->BSRR = (uint32_t)VSYNC_IN_Pin << 16U)	
#define VSYNC_OUT_HI()		  	  (VSYNC_IN_GPIO_Port->BSRR = VSYNC_IN_Pin)

EXTERN void GUI_Reset_test_state();

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
