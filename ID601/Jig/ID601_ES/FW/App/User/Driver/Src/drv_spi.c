/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_spi.c
    * @brief          : SPI driver implementation
    ******************************************************************************
    * @attention
    *
    * Copyright (c) 2026 Global Technologies.
    * All rights reserved.
    *
    ******************************************************************************
    */
/* USER CODE END Header */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. Direct pairing header (Corresponding header for this source file) */
#include "drv_spi.h"
/* 2. C standard library headers (Alphabetical order) */
/* 3. Project internal / System-related headers */
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SPI_TIMEOUT_MS      (100U)

#define SPI1_RX_FLAG_INDEX  (0U)
#define SPI1_RX_DMA_BASE    DMA2
#define SPI1_RX_DMA_STREAM  LL_DMA_STREAM_0

#define SPI1_TX_FLAG_INDEX  (1U)
#define SPI1_TX_DMA_BASE    DMA2
#define SPI1_TX_DMA_STREAM  LL_DMA_STREAM_3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
bool gn_spi_dma_flag[2];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drv_spi_init(void)
{
    /* Implement the SPI initialization functionality here */
    LL_DMA_EnableIT_TC(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM);
    LL_DMA_EnableIT_TE(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM);

    LL_DMA_EnableIT_TC(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM);
    LL_DMA_EnableIT_TE(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM);

    LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV16);

    /* DMA2_Stream0_IRQn interrupt configuration */
    NVIC_SetPriority(DMA2_Stream0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    /* DMA2_Stream3_IRQn interrupt configuration */
    NVIC_SetPriority(DMA2_Stream3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_0, LL_DMA_CHANNEL_3);
    LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_0, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_0);

    LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_3, LL_DMA_CHANNEL_3);
    LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_3, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA2, LL_DMA_STREAM_3, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_3, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_3, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_3, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_3, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_3);

    // Transmit a dummy byte to set high MOSI line initially
    uint8_t data = 0xFFU;
    drv_spi_transmit_direct(&data, 1U);

    gn_spi_dma_flag[SPI1_RX_FLAG_INDEX] = SPI_DMA_DONE;
    gn_spi_dma_flag[SPI1_TX_FLAG_INDEX] = SPI_DMA_DONE;
}

void drv_spi_tx_dma_irq_handler(void)
{
    if (true == LL_DMA_IsActiveFlag_TC3(SPI1_TX_DMA_BASE))
    {
        gn_spi_dma_flag[SPI1_TX_FLAG_INDEX] = SPI_DMA_DONE;
        LL_DMA_ClearFlag_TC3(SPI1_TX_DMA_BASE);
    }
    if (true == LL_DMA_IsActiveFlag_TE3(SPI1_TX_DMA_BASE))
    {
        LL_DMA_ClearFlag_TE3(SPI1_TX_DMA_BASE);
    }
}

void drv_spi_rx_dma_irq_handler(void)
{
    if (true == LL_DMA_IsActiveFlag_TC0(SPI1_RX_DMA_BASE))
    {
        gn_spi_dma_flag[SPI1_RX_FLAG_INDEX] = SPI_DMA_DONE;
        //LL_GPIO_SetOutputPin(DEBUG2_GPIO_Port, DEBUG2_Pin);
        LL_DMA_ClearFlag_TC0(SPI1_RX_DMA_BASE);
    }
    if (true == LL_DMA_IsActiveFlag_TE0(SPI1_RX_DMA_BASE))
    {
        LL_DMA_ClearFlag_TE0(SPI1_RX_DMA_BASE);
    }
}

bool drv_spi_transmit_direct(const uint8_t *p_data, uint16_t length)
{
    if ((p_data == NULL) || (length == 0U))
    {
        return false;
    }
    /* Configure SPI1 as master, full duplex, CPOL low */
    LL_SPI_Disable(SPI1);
    LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW);
    LL_SPI_Enable(SPI1);

    for (uint16_t i = 0U; i < length; ++i)
    {
        uint32_t start_time = HAL_GetTick();
        while (!LL_SPI_IsActiveFlag_TXE(SPI1))
        {
            if ((HAL_GetTick() - start_time) > SPI_TIMEOUT_MS)
            {
                LL_SPI_Disable(SPI1);
                return false;
            }
        }
        LL_SPI_TransmitData8(SPI1, p_data[i]);
    }

    uint32_t start_time = HAL_GetTick();
    while (!LL_SPI_IsActiveFlag_TXE(SPI1) || LL_SPI_IsActiveFlag_BSY(SPI1))
    {
        if ((HAL_GetTick() - start_time) > SPI_TIMEOUT_MS)
        {
            LL_SPI_Disable(SPI1);
            return false;
        }
    }
    LL_SPI_Disable(SPI1);
    return true;
}

bool drv_spi_receive_direct(uint8_t *p_data, uint16_t length)
{
    if ((p_data == NULL) || (length == 0U))
    {
        return false;
    }
    /* Configure SPI1 as slave, simplex RX, CPOL high */
    LL_SPI_Disable(SPI1);
    LL_SPI_SetMode(SPI1, LL_SPI_MODE_SLAVE);
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_RX);
    LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);
    LL_SPI_Enable(SPI1);

    if (LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
        (void)LL_SPI_ReceiveData8(SPI1);
    }

    for (uint16_t i = 0U; i < length; ++i)
    {
        uint32_t start_time = HAL_GetTick();
        while (!LL_SPI_IsActiveFlag_RXNE(SPI1))
        {
            if ((HAL_GetTick() - start_time) > SPI_TIMEOUT_MS)
            {
                return false;
            }
        }
        p_data[i] = LL_SPI_ReceiveData8(SPI1);
    }

    LL_SPI_Disable(SPI1);
    return true;
}

bool drv_spi_transmit_dma(const uint8_t *p_data, uint16_t length)
{
    if ((p_data == NULL) || (length == 0U))
    {
        return false;
    }

    /* Configure SPI1 as master, full duplex, CPOL low */
    LL_SPI_Disable(SPI1);
    LL_DMA_DisableStream(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM);

    LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW);
    LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);

    LL_DMA_ClearFlag_TC3(SPI1_TX_DMA_BASE);
    LL_DMA_ClearFlag_TE3(SPI1_TX_DMA_BASE);

    LL_DMA_SetDataTransferDirection(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetMemoryAddress(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM, (uint32_t)p_data);
    LL_DMA_SetPeriphAddress(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM, LL_SPI_DMA_GetRegAddr(SPI1));

    LL_DMA_SetDataLength(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM, length);

    LL_SPI_EnableDMAReq_TX(SPI1);
    LL_DMA_EnableStream(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM);

    //LL_GPIO_SetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);
    //LL_GPIO_ResetOutputPin(DEBUG2_GPIO_Port, DEBUG2_Pin);
    gn_spi_dma_flag[SPI1_TX_FLAG_INDEX] = SPI_DMA_BUSY;

    LL_SPI_Enable(SPI1);

    uint32_t start_time = HAL_GetTick();
    while (SPI_DMA_BUSY == gn_spi_dma_flag[SPI1_TX_FLAG_INDEX] || false == LL_SPI_IsActiveFlag_TXE(SPI1) || true == LL_SPI_IsActiveFlag_BSY(SPI1))
    {
        if ((HAL_GetTick() - start_time) > SPI_TIMEOUT_MS)
        {
            //LL_GPIO_ResetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);
            LL_DMA_DisableStream(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM);
            LL_SPI_Disable(SPI1);
            gn_spi_dma_flag[SPI1_TX_FLAG_INDEX] = SPI_DMA_DONE;
            return false;
        }
    }

    //LL_GPIO_ResetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);
    LL_DMA_DisableStream(SPI1_TX_DMA_BASE, SPI1_TX_DMA_STREAM);
    LL_SPI_Disable(SPI1);
    gn_spi_dma_flag[SPI1_TX_FLAG_INDEX] = SPI_DMA_DONE;
    return true;
}

bool drv_spi_receive_dma(uint8_t *p_data, uint16_t length)
{
    if ((p_data == NULL) || (length == 0U))
    {
        return false;
    }

    /* Configure SPI1 as slave, simplex RX, CPOL high */
    LL_SPI_Disable(SPI1);
    LL_DMA_DisableStream(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM);

    LL_SPI_SetMode(SPI1, LL_SPI_MODE_SLAVE);
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_RX);
    LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);
    LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);

    while (LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
        volatile uint32_t dummy = LL_SPI_ReceiveData8(SPI1);
    }

    LL_DMA_ClearFlag_TC0(SPI1_RX_DMA_BASE);
    LL_DMA_ClearFlag_TE0(SPI1_RX_DMA_BASE);

    LL_DMA_SetDataTransferDirection(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetMemoryAddress(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM, (uint32_t)p_data);
    LL_DMA_SetPeriphAddress(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM, LL_SPI_DMA_GetRegAddr(SPI1));

    LL_DMA_SetDataLength(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM, length);

    volatile const uint32_t wait = 45U;
    for (uint32_t i = 0U; i < wait; ++i)
    {
        __NOP();
    }

    LL_SPI_EnableDMAReq_RX(SPI1);
    LL_DMA_EnableStream(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM);

    //LL_GPIO_SetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);
    gn_spi_dma_flag[SPI1_RX_FLAG_INDEX] = SPI_DMA_BUSY;

    LL_SPI_Enable(SPI1);

    uint32_t start_time = HAL_GetTick();
    while (SPI_DMA_BUSY == gn_spi_dma_flag[SPI1_RX_FLAG_INDEX])
    {
        if ((HAL_GetTick() - start_time) > SPI_TIMEOUT_MS)
        {
            //LL_GPIO_ResetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);
            LL_DMA_DisableStream(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM);
            LL_SPI_Disable(SPI1);
            gn_spi_dma_flag[SPI1_RX_FLAG_INDEX] = SPI_DMA_DONE;
            return false;
        }
    }

    //LL_GPIO_ResetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);
    LL_DMA_DisableStream(SPI1_RX_DMA_BASE, SPI1_RX_DMA_STREAM);
    LL_SPI_Disable(SPI1);
    gn_spi_dma_flag[SPI1_RX_FLAG_INDEX] = SPI_DMA_DONE;
    return true;
}
/* USER CODE END 0 */
