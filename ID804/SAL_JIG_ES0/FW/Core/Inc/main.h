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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "comm_debug.h"
#include "drv_sal.h"
#include "drv_sal_trim.h"
#include "switch.h"
#include "ADS124S08.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#define SAL_TYPE_A          (0)
#define SAL_TYPE_B          (1)
#define SAL_TYPE            SAL_TYPE_A

#define SPI_TX_BUFF_SIZE    150
#define SPI_RX_BUFF_SIZE    720

extern volatile bool gb_spi_tx_started[2];
extern volatile bool gb_spi_rx_started[2];

extern uint8_t gn_spi_tx_buff[2][SPI_TX_BUFF_SIZE];
extern uint8_t gn_spi_rx_buff[2][SPI_RX_BUFF_SIZE];

extern uint32_t gn_sw_trim_tickcount;
extern uint32_t gn_sw_btn_tickcount;
extern uint32_t gn_sw_demo_tickcount;

extern bool gb_trim_start_flag;
extern bool gb_demo_start_flag;

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
EXTERN void spi_write(uint8_t* p_data, uint8_t spi_tx_len);
EXTERN void spi_read(uint8_t spi_rx_len);

EXTERN void spi_write_dma(uint8_t strip, uint8_t* p_data, uint8_t spi_tx_len);
EXTERN void spi_read_dma(uint8_t strip, uint8_t spi_rx_len);


/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define SPI2_SIO1P_Pin LL_GPIO_PIN_0
#define SPI2_SIO1P_GPIO_Port GPIOC
#define SAL_MODE2_Pin LL_GPIO_PIN_2
#define SAL_MODE2_GPIO_Port GPIOC
#define SAL_VLED_EN_Pin LL_GPIO_PIN_3
#define SAL_VLED_EN_GPIO_Port GPIOC
#define PWM_INPUT_Pin LL_GPIO_PIN_0
#define PWM_INPUT_GPIO_Port GPIOA
#define SPI2_SIO2N_Pin LL_GPIO_PIN_1
#define SPI2_SIO2N_GPIO_Port GPIOA
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define SAL_MODE_Pin LL_GPIO_PIN_4
#define SAL_MODE_GPIO_Port GPIOA
#define I2C_SIO2P_PU_Pin LL_GPIO_PIN_5
#define I2C_SIO2P_PU_GPIO_Port GPIOA
#define ADC_RESET_Pin LL_GPIO_PIN_6
#define ADC_RESET_GPIO_Port GPIOA
#define ADC_START_Pin LL_GPIO_PIN_7
#define ADC_START_GPIO_Port GPIOA
#define ADC_DRDY_Pin LL_GPIO_PIN_4
#define ADC_DRDY_GPIO_Port GPIOC
#define ADC_DRDY_EXTI_IRQn EXTI4_IRQn
#define SPI1_SIO2P_Pin LL_GPIO_PIN_5
#define SPI1_SIO2P_GPIO_Port GPIOC
#define SPI2_SIO1N_Pin LL_GPIO_PIN_0
#define SPI2_SIO1N_GPIO_Port GPIOB
#define CNT_MR_Pin LL_GPIO_PIN_1
#define CNT_MR_GPIO_Port GPIOB
#define ADC_CS_Pin LL_GPIO_PIN_2
#define ADC_CS_GPIO_Port GPIOB
#define SPI1_SIO1N_Pin LL_GPIO_PIN_10
#define SPI1_SIO1N_GPIO_Port GPIOB
#define ENABLE_SELECT_3_Pin LL_GPIO_PIN_12
#define ENABLE_SELECT_3_GPIO_Port GPIOB
#define SAL_ADC_1V_Pin LL_GPIO_PIN_14
#define SAL_ADC_1V_GPIO_Port GPIOB
#define ENABLE_SELECT_4_Pin LL_GPIO_PIN_6
#define ENABLE_SELECT_4_GPIO_Port GPIOC
#define I2C_SIO2N_PU_Pin LL_GPIO_PIN_7
#define I2C_SIO2N_PU_GPIO_Port GPIOC
#define ENABLE_SELECT_1_Pin LL_GPIO_PIN_8
#define ENABLE_SELECT_1_GPIO_Port GPIOC
#define I2C_SIO1P_PU_Pin LL_GPIO_PIN_9
#define I2C_SIO1P_PU_GPIO_Port GPIOC
#define SPI1_SIO1P_Pin LL_GPIO_PIN_8
#define SPI1_SIO1P_GPIO_Port GPIOA
#define I2C_SIO2N_PD_Pin LL_GPIO_PIN_9
#define I2C_SIO2N_PD_GPIO_Port GPIOA
#define SAL_VCC_5V5_Pin LL_GPIO_PIN_10
#define SAL_VCC_5V5_GPIO_Port GPIOA
#define LED_Pin LL_GPIO_PIN_11
#define LED_GPIO_Port GPIOA
#define ENABLE_SELECT_2_Pin LL_GPIO_PIN_12
#define ENABLE_SELECT_2_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SPI2_SIO2P_Pin LL_GPIO_PIN_15
#define SPI2_SIO2P_GPIO_Port GPIOA
#define LTC_DISCHARGE_Pin LL_GPIO_PIN_12
#define LTC_DISCHARGE_GPIO_Port GPIOC
#define SPI1_SIO2N_Pin LL_GPIO_PIN_2
#define SPI1_SIO2N_GPIO_Port GPIOD
#define SAL_VCC_EN_Pin LL_GPIO_PIN_4
#define SAL_VCC_EN_GPIO_Port GPIOB
#define I2C_SIO1N_PU_Pin LL_GPIO_PIN_8
#define I2C_SIO1N_PU_GPIO_Port GPIOB
#define I2C_SIO1N_PD_Pin LL_GPIO_PIN_9
#define I2C_SIO1N_PD_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define CNT_MR_HI()         CNT_MR_GPIO_Port->BSRR = CNT_MR_Pin <<  0U
#define CNT_MR_LO()         CNT_MR_GPIO_Port->BSRR = CNT_MR_Pin << 16U

#define EN_SEL_1_HI()       ENABLE_SELECT_1_GPIO_Port->BSRR = ENABLE_SELECT_1_Pin <<  0U
#define EN_SEL_1_LO()       ENABLE_SELECT_1_GPIO_Port->BSRR = ENABLE_SELECT_1_Pin << 16U

#define EN_SEL_2_HI()       ENABLE_SELECT_2_GPIO_Port->BSRR = ENABLE_SELECT_2_Pin <<  0U
#define EN_SEL_2_LO()       ENABLE_SELECT_2_GPIO_Port->BSRR = ENABLE_SELECT_2_Pin << 16U

#define EN_SEL_3_HI()       ENABLE_SELECT_3_GPIO_Port->BSRR = ENABLE_SELECT_3_Pin <<  0U
#define EN_SEL_3_LO()       ENABLE_SELECT_3_GPIO_Port->BSRR = ENABLE_SELECT_3_Pin << 16U

#define EN_SEL_4_HI()       ENABLE_SELECT_4_GPIO_Port->BSRR = ENABLE_SELECT_4_Pin <<  0U
#define EN_SEL_4_LO()       ENABLE_SELECT_4_GPIO_Port->BSRR = ENABLE_SELECT_4_Pin << 16U

#define I2C_SIO1N_PD_HI()   I2C_SIO1N_PD_GPIO_Port->BSRR = I2C_SIO1N_PD_Pin <<  0U
#define I2C_SIO1N_PD_LO()   I2C_SIO1N_PD_GPIO_Port->BSRR = I2C_SIO1N_PD_Pin << 16U

#define I2C_SIO1N_PU_HI()   I2C_SIO1N_PU_GPIO_Port->BSRR = I2C_SIO1N_PU_Pin <<  0U
#define I2C_SIO1N_PU_LO()   I2C_SIO1N_PU_GPIO_Port->BSRR = I2C_SIO1N_PU_Pin << 16U

#define I2C_SIO1P_PU_HI()   I2C_SIO1P_PU_GPIO_Port->BSRR = I2C_SIO1P_PU_Pin <<  0U
#define I2C_SIO1P_PU_LO()   I2C_SIO1P_PU_GPIO_Port->BSRR = I2C_SIO1P_PU_Pin << 16U

#define I2C_SIO2N_PD_HI()   I2C_SIO2N_PD_GPIO_Port->BSRR = I2C_SIO2N_PD_Pin <<  0U
#define I2C_SIO2N_PD_LO()   I2C_SIO2N_PD_GPIO_Port->BSRR = I2C_SIO2N_PD_Pin << 16U

#define I2C_SIO2N_PU_HI()   I2C_SIO2N_PU_GPIO_Port->BSRR = I2C_SIO2N_PU_Pin <<  0U
#define I2C_SIO2N_PU_LO()   I2C_SIO2N_PU_GPIO_Port->BSRR = I2C_SIO2N_PU_Pin << 16U

#define I2C_SIO2P_PU_HI()   I2C_SIO2P_PU_GPIO_Port->BSRR = I2C_SIO2P_PU_Pin <<  0U
#define I2C_SIO2P_PU_LO()   I2C_SIO2P_PU_GPIO_Port->BSRR = I2C_SIO2P_PU_Pin << 16U

#define LED_HI()            LED_GPIO_Port->BSRR = LED_Pin <<  0U
#define LED_LO()            LED_GPIO_Port->BSRR = LED_Pin << 16U

#define LTC_DISCHARGE_HI()  LTC_DISCHARGE_GPIO_Port->BSRR = LTC_DISCHARGE_Pin <<  0U
#define LTC_DISCHARGE_LO()  LTC_DISCHARGE_GPIO_Port->BSRR = LTC_DISCHARGE_Pin << 16U

#define SAL_MODE_HI()       SAL_MODE_GPIO_Port->BSRR = SAL_MODE_Pin <<  0U
#define SAL_MODE_LO()       SAL_MODE_GPIO_Port->BSRR = SAL_MODE_Pin << 16U

#define SAL_MODE2_HI()      SAL_MODE2_GPIO_Port->BSRR = SAL_MODE2_Pin <<  0U
#define SAL_MODE2_LO()      SAL_MODE2_GPIO_Port->BSRR = SAL_MODE2_Pin << 16U

#define SAL_ADC_1V_HI()     SAL_ADC_1V_GPIO_Port->BSRR = SAL_ADC_1V_Pin <<  0U
#define SAL_ADC_1V_LO()     SAL_ADC_1V_GPIO_Port->BSRR = SAL_ADC_1V_Pin << 16U

#define SAL_VCC_5V5_HI()    SAL_VCC_5V5_GPIO_Port->BSRR = SAL_VCC_5V5_Pin <<  0U
#define SAL_VCC_5V5_LO()    SAL_VCC_5V5_GPIO_Port->BSRR = SAL_VCC_5V5_Pin << 16U

#define SAL_VCC_EN_HI()     SAL_VCC_EN_GPIO_Port->BSRR = SAL_VCC_EN_Pin <<  0U
#define SAL_VCC_EN_LO()     SAL_VCC_EN_GPIO_Port->BSRR = SAL_VCC_EN_Pin << 16U

#define SAL_VLED_EN_HI()    SAL_VLED_EN_GPIO_Port->BSRR = SAL_VLED_EN_Pin <<  0U
#define SAL_VLED_EN_LO()    SAL_VLED_EN_GPIO_Port->BSRR = SAL_VLED_EN_Pin << 16U

#define SPI1_SIO1N_HI()     SPI1_SIO1N_GPIO_Port->BSRR = SPI1_SIO1N_Pin <<  0U
#define SPI1_SIO1N_LO()     SPI1_SIO1N_GPIO_Port->BSRR = SPI1_SIO1N_Pin << 16U

#define SPI1_SIO1P_HI()     SPI1_SIO1P_GPIO_Port->BSRR = SPI1_SIO1P_Pin <<  0U
#define SPI1_SIO1P_LO()     SPI1_SIO1P_GPIO_Port->BSRR = SPI1_SIO1P_Pin << 16U

#define SPI1_SIO2N_HI()     SPI1_SIO2N_GPIO_Port->BSRR = SPI1_SIO2N_Pin <<  0U
#define SPI1_SIO2N_LO()     SPI1_SIO2N_GPIO_Port->BSRR = SPI1_SIO2N_Pin << 16U

#define SPI1_SIO2P_HI()     SPI1_SIO2P_GPIO_Port->BSRR = SPI1_SIO2P_Pin <<  0U
#define SPI1_SIO2P_LO()     SPI1_SIO2P_GPIO_Port->BSRR = SPI1_SIO2P_Pin << 16U

#define SPI2_SIO1N_HI()     SPI2_SIO1N_GPIO_Port->BSRR = SPI2_SIO1N_Pin <<  0U
#define SPI2_SIO1N_LO()     SPI2_SIO1N_GPIO_Port->BSRR = SPI2_SIO1N_Pin << 16U

#define SPI2_SIO1P_HI()     SPI2_SIO1P_GPIO_Port->BSRR = SPI2_SIO1P_Pin <<  0U
#define SPI2_SIO1P_LO()     SPI2_SIO1P_GPIO_Port->BSRR = SPI2_SIO1P_Pin << 16U

#define SPI2_SIO2N_HI()     SPI2_SIO2N_GPIO_Port->BSRR = SPI2_SIO2N_Pin <<  0U
#define SPI2_SIO2N_LO()     SPI2_SIO2N_GPIO_Port->BSRR = SPI2_SIO2N_Pin << 16U

#define SPI2_SIO2P_HI()     SPI2_SIO2P_GPIO_Port->BSRR = SPI2_SIO2P_Pin <<  0U
#define SPI2_SIO2P_LO()     SPI2_SIO2P_GPIO_Port->BSRR = SPI2_SIO2P_Pin << 16U

EXTERN void us_delay(uint16_t us_delay);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
