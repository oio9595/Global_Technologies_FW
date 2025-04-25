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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifdef __APIC_MAIN_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define SPI_DEFAULT_TIMEOUT 100U
#define SPI_BSY_FLAG_WORKAROUND_TIMEOUT 1000U /*!< Timeout 1000 µs             */

#define DAC_OP_MODE_WRM         0x8000 /* Write Register Mode */
#define DAC_OP_MODE_WTM         0x9000 /* Write Through Modes */
#define DAC_OP_BROADCAST        0xC000 /* written to all channels' control register and DAC output simultaneously */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern char msg_buffer[256];

EXTERN void us_delay(uint32_t us_delay);
EXTERN void print(const char* str);

extern void AQIC_Apply_Voltage(float f_voltage, uint8_t ch);
extern void change_i2c_setting(uint8_t i2c_type);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define SPI_MOSI_Pin GPIO_PIN_1
#define SPI_MOSI_GPIO_Port GPIOC
#define SPI_MISO_Pin GPIO_PIN_2
#define SPI_MISO_GPIO_Port GPIOC
#define LS_OE3_Pin GPIO_PIN_3
#define LS_OE3_GPIO_Port GPIOC
#define RXD_GPIO_Pin GPIO_PIN_0
#define RXD_GPIO_GPIO_Port GPIOA
#define LS_CP3_Pin GPIO_PIN_1
#define LS_CP3_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LS_OE2_Pin GPIO_PIN_4
#define LS_OE2_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define MCU_ADC_FB_Pin GPIO_PIN_6
#define MCU_ADC_FB_GPIO_Port GPIOA
#define LS_D5_Pin GPIO_PIN_7
#define LS_D5_GPIO_Port GPIOA
#define ADC_DRDY_Pin GPIO_PIN_4
#define ADC_DRDY_GPIO_Port GPIOC
#define ADC_DRDY_EXTI_IRQn EXTI4_IRQn
#define KEY_TRIMMING_Pin GPIO_PIN_5
#define KEY_TRIMMING_GPIO_Port GPIOC
#define LS_D4_Pin GPIO_PIN_2
#define LS_D4_GPIO_Port GPIOB
#define SPI_CLK_Pin GPIO_PIN_10
#define SPI_CLK_GPIO_Port GPIOB
#define LS_D3_Pin GPIO_PIN_12
#define LS_D3_GPIO_Port GPIOB
#define KEY_POWER_Pin GPIO_PIN_13
#define KEY_POWER_GPIO_Port GPIOB
#define LS_D1_Pin GPIO_PIN_6
#define LS_D1_GPIO_Port GPIOC
#define LS_CP1_Pin GPIO_PIN_7
#define LS_CP1_GPIO_Port GPIOC
#define LS_D0_Pin GPIO_PIN_8
#define LS_D0_GPIO_Port GPIOC
#define VLED_CTR_9V_Pin GPIO_PIN_9
#define VLED_CTR_9V_GPIO_Port GPIOC
#define TXS_OE_Pin GPIO_PIN_8
#define TXS_OE_GPIO_Port GPIOA
#define LS_D6_Pin GPIO_PIN_9
#define LS_D6_GPIO_Port GPIOA
#define LS_CP2_Pin GPIO_PIN_10
#define LS_CP2_GPIO_Port GPIOA
#define LED_TRIM_Pin GPIO_PIN_11
#define LED_TRIM_GPIO_Port GPIOA
#define LS_D2_Pin GPIO_PIN_12
#define LS_D2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define LS_D7_Pin GPIO_PIN_15
#define LS_D7_GPIO_Port GPIOA
#define AS1105_CLK_Pin GPIO_PIN_11
#define AS1105_CLK_GPIO_Port GPIOC
#define LTC_DISCHARGE_Pin GPIO_PIN_12
#define LTC_DISCHARGE_GPIO_Port GPIOC
#define LS_OE1_Pin GPIO_PIN_2
#define LS_OE1_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define MCU_MODE_Pin GPIO_PIN_4
#define MCU_MODE_GPIO_Port GPIOB
#define AQIC_PWR_CTL_Pin GPIO_PIN_5
#define AQIC_PWR_CTL_GPIO_Port GPIOB
#define MCU_I2C_SCL_CS_Pin GPIO_PIN_6
#define MCU_I2C_SCL_CS_GPIO_Port GPIOB
#define MCU_I2C_SDA_G_Pin GPIO_PIN_7
#define MCU_I2C_SDA_G_GPIO_Port GPIOB
#define LED_OK_Pin GPIO_PIN_8
#define LED_OK_GPIO_Port GPIOB
#define LED_NG_Pin GPIO_PIN_9
#define LED_NG_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define ADS114S08_DEV_NUM           (1U)

#define AQIC_G_LO()                 MCU_I2C_SDA_G_GPIO_Port->BSRR = (uint32_t)MCU_I2C_SDA_G_Pin << 16U
#define	AQIC_G_HI()                 MCU_I2C_SDA_G_GPIO_Port->BSRR = MCU_I2C_SDA_G_Pin

#define AQIC_CS_LO()                MCU_I2C_SCL_CS_GPIO_Port->BSRR = (uint32_t)MCU_I2C_SCL_CS_Pin << 16U
#define	AQIC_CS_HI()                MCU_I2C_SCL_CS_GPIO_Port->BSRR = MCU_I2C_SCL_CS_Pin

#define LATCH_PIN_RESET             0
#define LATCH_PIN_SET               1

#define LATCH_CP1                   1
#define LATCH_CP2                   2
#define LATCH_CP3                   3
#define LATCH_D0                    0
#define LATCH_D1                    1
#define LATCH_D2                    2
#define LATCH_D3                    3
#define LATCH_D4                    4
#define LATCH_D5                    5
#define LATCH_D6                    6
#define LATCH_D7                    7

#define MODE_SEL_LS_PORT            LATCH_D0
#define MODE_SEL_LS_CP              LATCH_CP1
#define ENABLE_SELECT1_LS_PORT      LATCH_D1
#define ENABLE_SELECT1_LS_CP        LATCH_CP1
#define ENABLE_SELECT2_LS_PORT      LATCH_D2
#define ENABLE_SELECT2_LS_CP        LATCH_CP1
#define ENABLE_SELECT3_LS_PORT      LATCH_D3
#define ENABLE_SELECT3_LS_CP        LATCH_CP1
#define LTC_LOW_CURRENT_LS_PORT     LATCH_D4
#define LTC_LOW_CURRENT_LS_CP       LATCH_CP1
#define LTC_MID_CURRENT_LS_PORT     LATCH_D5
#define LTC_MID_CURRENT_LS_CP       LATCH_CP1
#define LTC_HIGH_CURRENT_LS_PORT    LATCH_D6
#define LTC_HIGH_CURRENT_LS_CP      LATCH_CP1
#define ADC_START_LS_PORT           LATCH_D7
#define ADC_START_LS_CP             LATCH_CP1
#define DAC_CS_LS_PORT              LATCH_D0
#define DAC_CS_LS_CP                LATCH_CP2
#define ADC_RESET_LS_PORT           LATCH_D1
#define ADC_RESET_LS_CP             LATCH_CP2
#define ADC_CS_LS_PORT              LATCH_D2
#define ADC_CS_LS_CP                LATCH_CP2

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
