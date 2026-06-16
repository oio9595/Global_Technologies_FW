/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "drv_gpio.h"
#include "drv_timer.h"
#include "drv_xdr12.h"
#include "drv_xcr24.h"

#include "comm_debugging.h"
#include "framework.h"
#include "ads124s08.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

    fw_systick_handler();

    if(gn_ads114s08_read_timeout)
    {
        --gn_ads114s08_read_timeout;
    }

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line 4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_IRQn 0 */

  /* USER CODE END EXTI4_IRQn 0 */
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_4) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_4);
    /* USER CODE BEGIN LL_EXTI_LINE_4 */
        ADC_DRDY_INT_Handler();

    /* USER CODE END LL_EXTI_LINE_4 */
  }
  /* USER CODE BEGIN EXTI4_IRQn 1 */

  /* USER CODE END EXTI4_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream2 global interrupt.
  */
void DMA1_Stream2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream2_IRQn 0 */
    if (LL_DMA_IsActiveFlag_TC2(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TC2(DMA1);
        LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_2);

        mcu_peripheral_tim_input_capture_stop();
    }
    else if(LL_DMA_IsActiveFlag_TE2(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TE2(DMA1);
    }

  /* USER CODE END DMA1_Stream2_IRQn 0 */
  /* USER CODE BEGIN DMA1_Stream2_IRQn 1 */

  /* USER CODE END DMA1_Stream2_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream6 global interrupt.
  */
void DMA1_Stream6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream6_IRQn 0 */
    if(LL_DMA_IsActiveFlag_TC6((DMA1)) == 1)
    {
        LL_DMA_ClearFlag_TC6((DMA1));
        LL_DMA_DisableStream((DMA1), LL_DMA_STREAM_6);

        //gb_xd_pwm_in_flag = false;
    }
    else if(LL_DMA_IsActiveFlag_TE6((DMA1)) == 1)
    {
        LL_DMA_ClearFlag_TE6(DMA1);
    }

  /* USER CODE END DMA1_Stream6_IRQn 0 */
  /* USER CODE BEGIN DMA1_Stream6_IRQn 1 */

  /* USER CODE END DMA1_Stream6_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */
    if(LL_TIM_IsActiveFlag_UPDATE(TIM3))
    {
        LL_TIM_ClearFlag_UPDATE(TIM3);
        tim_svsync_out_handler();
    }

  /* USER CODE END TIM3_IRQn 0 */
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
    if(LL_USART_IsActiveFlag_RXNE(USART2))
    {
        /* Read one byte from the receive data register */
        uint8_t rx_data = LL_USART_ReceiveData8(USART2);
        comm_rx_handler(rx_data);
    }
    else if(LL_USART_IsActiveFlag_TXE(USART2))
    {
        comm_tx_handler();
    }

  /* USER CODE END USART2_IRQn 0 */
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles TIM8 break interrupt and TIM12 global interrupt.
  */
void TIM8_BRK_TIM12_IRQHandler(void)
{
  /* USER CODE BEGIN TIM8_BRK_TIM12_IRQn 0 */

    if(LL_TIM_IsActiveFlag_UPDATE(TIM12))
    {
        LL_TIM_ClearFlag_UPDATE(TIM12);

        gb_xd_pwm_in_timeout = true;
    }

  /* USER CODE END TIM8_BRK_TIM12_IRQn 0 */
  /* USER CODE BEGIN TIM8_BRK_TIM12_IRQn 1 */

  /* USER CODE END TIM8_BRK_TIM12_IRQn 1 */
}

/**
  * @brief This function handles TIM8 update interrupt and TIM13 global interrupt.
  */
void TIM8_UP_TIM13_IRQHandler(void)
{
  /* USER CODE BEGIN TIM8_UP_TIM13_IRQn 0 */
    if(LL_TIM_IsActiveFlag_UPDATE(TIM8))
    {
        LL_TIM_ClearFlag_UPDATE(TIM8);
        tim_vsync_out_handler();
    }

  /* USER CODE END TIM8_UP_TIM13_IRQn 0 */
  /* USER CODE BEGIN TIM8_UP_TIM13_IRQn 1 */

  /* USER CODE END TIM8_UP_TIM13_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream1 global interrupt.
  */
void DMA2_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream1_IRQn 0 */
    if(LL_DMA_IsActiveFlag_TC1(DMA2) == 1)
    {
        LL_DMA_ClearFlag_TC1(DMA2);
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);

        gb_xd_pwm_out_flag = false;
        LL_TIM_DisableCounter(TIM1);
    }
    else if(LL_DMA_IsActiveFlag_TE1(DMA2) == 1)
    {
        LL_DMA_ClearFlag_TE1(DMA2);
    }

  /* USER CODE END DMA2_Stream1_IRQn 0 */
  /* USER CODE BEGIN DMA2_Stream1_IRQn 1 */

  /* USER CODE END DMA2_Stream1_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream3 global interrupt.
  */
void DMA2_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream3_IRQn 0 */
  if(LL_DMA_IsActiveFlag_TC3(DMA2) == 1)
  {
      LL_DMA_ClearFlag_TC3(DMA2);
      LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_3);

      while(LL_SPI_IsActiveFlag_BSY(SPI1) == SET) { };
      user_delay(2U);
      XCR_NSS_HI();

      gb_xcr_ld_transfer_spi_dma_flag = false;
  }
  else if(LL_DMA_IsActiveFlag_TE3(DMA2) == 1)
  {
      LL_DMA_ClearFlag_TE3(DMA2);
  }

  /* USER CODE END DMA2_Stream3_IRQn 0 */
  /* USER CODE BEGIN DMA2_Stream3_IRQn 1 */

  /* USER CODE END DMA2_Stream3_IRQn 1 */
}

/* USER CODE BEGIN 1 */

void DMA1_Stream5_IRQHandler(void)
{
    if(LL_DMA_IsActiveFlag_TC5((DMA1)) == 1)
    {
        LL_DMA_ClearFlag_TC5((DMA1));
        LL_DMA_DisableStream((DMA1), LL_DMA_STREAM_5);

        gb_xd_pwm_in_flag = false;
    }
    else if(LL_DMA_IsActiveFlag_TE5((DMA1)) == 1)
    {
        LL_DMA_ClearFlag_TE5(DMA1);
    }
}

/* USER CODE END 1 */
