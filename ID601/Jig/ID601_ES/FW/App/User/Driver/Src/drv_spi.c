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
#include "drv_gpio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_SPI_STATUS
{
    SPI_STATUS_NONE = 0U,
    SPI_STATUS_BUSY,
    SPI_STATUS_DONE,
    SPI_STATUS_ERROR,
    SPI_STATUS_TIMEOUT
} spi_status_t;
typedef bool (*spi_cs_fn_t)(bool);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SPI_TIMEOUT_MS      (100U)

#define SPI1_TX_DMA_BASE    DMA2
#define SPI1_TX_DMA_STREAM  LL_DMA_STREAM_3

#define SPI1_RX_DMA_BASE    DMA2
#define SPI1_RX_DMA_STREAM  LL_DMA_STREAM_0

#define SPI2_TX_DMA_BASE    DMA1
#define SPI2_TX_DMA_STREAM  LL_DMA_STREAM_4

#define SPI2_RX_DMA_BASE    DMA1
#define SPI2_RX_DMA_STREAM  LL_DMA_STREAM_3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static SPI_TypeDef* const SPI_BASE[2] = { SPI1, SPI2 };

static DMA_TypeDef* const SPI_TX_DMA_BASE[2] = { SPI1_TX_DMA_BASE, SPI2_TX_DMA_BASE };
static const uint32_t SPI_TX_DMA_STREAM[2]   = { SPI1_TX_DMA_STREAM, SPI2_TX_DMA_STREAM };

static DMA_TypeDef* const SPI_RX_DMA_BASE[2] = { SPI1_RX_DMA_BASE, SPI2_RX_DMA_BASE };
static const uint32_t SPI_RX_DMA_STREAM[2]   = { SPI1_RX_DMA_STREAM, SPI2_RX_DMA_STREAM };

static spi_status_t gb_spi_tx_dma_flag[2] = { SPI_STATUS_NONE, SPI_STATUS_NONE };
static spi_status_t gb_spi_rx_dma_flag[2] = { SPI_STATUS_NONE, SPI_STATUS_NONE };

const static spi_cs_fn_t gp_spi_cs_transition[2] = { drv_gpio_ic603_cs, drv_gpio_ads114s08_dev_all_cs };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drv_spi_init(void)
{

}

static bool drv_dma_clear_tc_flag(DMA_TypeDef *DMAx, uint32_t stream)
{
    if (NULL == DMAx)
    {
        return false;
    }
    switch (stream)
    {
        case LL_DMA_STREAM_0:
        {
            LL_DMA_ClearFlag_TC0(DMAx);
            return true;
        }
        case LL_DMA_STREAM_1:
        {
            LL_DMA_ClearFlag_TC1(DMAx);
            return true;
        }
        case LL_DMA_STREAM_2:
        {
            LL_DMA_ClearFlag_TC2(DMAx);
            return true;
        }
        case LL_DMA_STREAM_3:
        {
            LL_DMA_ClearFlag_TC3(DMAx);
            return true;
        }
        case LL_DMA_STREAM_4:
        {
            LL_DMA_ClearFlag_TC4(DMAx);
            return true;
        }
        case LL_DMA_STREAM_5:
        {
            LL_DMA_ClearFlag_TC5(DMAx);
            return true;
        }
        case LL_DMA_STREAM_6:
        {
            LL_DMA_ClearFlag_TC6(DMAx);
            return true;
        }
        case LL_DMA_STREAM_7:
        {
            LL_DMA_ClearFlag_TC7(DMAx);
            return true;
        }
        default:
        {
            /* Invalid DMA stream: leave DMA flags unchanged. */
            return false;
        }
    }
}

static bool drv_dma_clear_te_flag(DMA_TypeDef *DMAx, uint32_t stream)
{
    if (NULL == DMAx)
    {
        return false;
    }
    switch (stream)
    {
        case LL_DMA_STREAM_0:
        {
            LL_DMA_ClearFlag_TE0(DMAx);
            return true;
        }
        case LL_DMA_STREAM_1:
        {
            LL_DMA_ClearFlag_TE1(DMAx);
            return true;
        }
        case LL_DMA_STREAM_2:
        {
            LL_DMA_ClearFlag_TE2(DMAx);
            return true;
        }
        case LL_DMA_STREAM_3:
        {
            LL_DMA_ClearFlag_TE3(DMAx);
            return true;
        }
        case LL_DMA_STREAM_4:
        {
            LL_DMA_ClearFlag_TE4(DMAx);
            return true;
        }
        case LL_DMA_STREAM_5:
        {
            LL_DMA_ClearFlag_TE5(DMAx);
            return true;
        }
        case LL_DMA_STREAM_6:
        {
            LL_DMA_ClearFlag_TE6(DMAx);
            return true;
        }
        case LL_DMA_STREAM_7:
        {
            LL_DMA_ClearFlag_TE7(DMAx);
            return true;
        }
        default:
        {
            /* Invalid DMA stream: leave DMA flags unchanged. */
            return false;
        }
    }
}

static bool drv_dma_is_active_tc_flag(DMA_TypeDef *DMAx, uint32_t stream)
{
    if (NULL == DMAx)
    {
        return false;
    }
    switch (stream)
    {
        case LL_DMA_STREAM_0:
        {
            return LL_DMA_IsActiveFlag_TC0(DMAx);
        }
        case LL_DMA_STREAM_1:
        {
            return LL_DMA_IsActiveFlag_TC1(DMAx);
        }
        case LL_DMA_STREAM_2:
        {
            return LL_DMA_IsActiveFlag_TC2(DMAx);
        }
        case LL_DMA_STREAM_3:
        {
            return LL_DMA_IsActiveFlag_TC3(DMAx);
        }
        case LL_DMA_STREAM_4:
        {
            return LL_DMA_IsActiveFlag_TC4(DMAx);
        }
        case LL_DMA_STREAM_5:
        {
            return LL_DMA_IsActiveFlag_TC5(DMAx);
        }
        case LL_DMA_STREAM_6:
        {
            return LL_DMA_IsActiveFlag_TC6(DMAx);
        }
        case LL_DMA_STREAM_7:
        {
            return LL_DMA_IsActiveFlag_TC7(DMAx);
        }
        default:
        {
            /* Invalid DMA stream: leave DMA flags unchanged. */
            return false;
        }
    }
}

static bool drv_dma_is_active_te_flag(DMA_TypeDef *DMAx, uint32_t stream)
{
    if (NULL == DMAx)
    {
        return false;
    }
    switch (stream)
    {
        case LL_DMA_STREAM_0:
        {
            return LL_DMA_IsActiveFlag_TE0(DMAx);
        }
        case LL_DMA_STREAM_1:
        {
            return LL_DMA_IsActiveFlag_TE1(DMAx);
        }
        case LL_DMA_STREAM_2:
        {
            return LL_DMA_IsActiveFlag_TE2(DMAx);
        }
        case LL_DMA_STREAM_3:
        {
            return LL_DMA_IsActiveFlag_TE3(DMAx);
        }
        case LL_DMA_STREAM_4:
        {
            return LL_DMA_IsActiveFlag_TE4(DMAx);
        }
        case LL_DMA_STREAM_5:
        {
            return LL_DMA_IsActiveFlag_TE5(DMAx);
        }
        case LL_DMA_STREAM_6:
        {
            return LL_DMA_IsActiveFlag_TE6(DMAx);
        }
        case LL_DMA_STREAM_7:
        {
            return LL_DMA_IsActiveFlag_TE7(DMAx);
        }
        default:
        {
            /* Invalid DMA stream: leave DMA flags unchanged. */
            return false;
        }
    }
}

bool drv_spi_transmit_dma_8bit(SPI_TypeDef* SPIx, const uint8_t *p_tx, uint16_t length, uint32_t timeout)
{
    if ((NULL == SPIx) || (NULL == p_tx) || (0U == length))
    {
        return false;
    }

    const uint8_t spi_index = (SPIx == SPI1) ? 0U : 1U;
    if (spi_index >= 2U)
    {
        return false;
    }

    LL_SPI_Disable(SPI_BASE[spi_index]);
    LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemorySize(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphSize(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_PDATAALIGN_BYTE);
    //LL_DMA_SetMemoryIncMode(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_MEMORY_INCREMENT);
    LL_SPI_SetDataWidth(SPI_BASE[spi_index], LL_SPI_DATAWIDTH_8BIT);

    drv_dma_clear_tc_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
    drv_dma_clear_te_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemoryAddress(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)p_tx);
    LL_DMA_SetPeriphAddress(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)&SPI_BASE[spi_index]->DR);
    LL_DMA_SetDataLength(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)length);

    LL_DMA_EnableIT_TC(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
    LL_DMA_EnableIT_TE(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_BUSY;

    LL_SPI_EnableDMAReq_TX(SPI_BASE[spi_index]);
    LL_DMA_EnableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_SPI_Enable(SPI_BASE[spi_index]);

    uint32_t start_time = HAL_GetTick();

    while (SPI_STATUS_BUSY == gb_spi_tx_dma_flag[spi_index])
    {
        if ((HAL_GetTick() - start_time) > timeout)
        {
            LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
            LL_SPI_Disable(SPI_BASE[spi_index]);
            gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_TIMEOUT;
            return false;
        }
    }

    return true;
}

bool drv_spi_receive_dma_8bit(SPI_TypeDef* SPIx, const uint8_t *p_tx, const uint8_t *p_rx, uint16_t length, uint32_t timeout)
{
    if ((NULL == SPIx) || (NULL == p_tx) || (NULL == p_rx) || (0U == length))
    {
        return false;
    }

    const uint8_t spi_index = (SPIx == SPI1) ? 0U : 1U;
    if (spi_index >= 2U)
    {
        return false;
    }

    if (LL_SPI_IsActiveFlag_RXNE(SPI_BASE[spi_index]))
    {
        (void)LL_SPI_ReceiveData8(SPI_BASE[spi_index]);
    }

    LL_SPI_Disable(SPI_BASE[spi_index]);
    LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemorySize(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphSize(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_PDATAALIGN_BYTE);
    //LL_DMA_SetMemoryIncMode(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_MEMORY_INCREMENT);
    LL_SPI_SetDataWidth(SPI_BASE[spi_index], LL_SPI_DATAWIDTH_8BIT);

    drv_dma_clear_tc_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
    drv_dma_clear_te_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemoryAddress(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)p_tx);
    LL_DMA_SetPeriphAddress(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)&SPI_BASE[spi_index]->DR);
    LL_DMA_SetDataLength(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)length);

    LL_DMA_EnableIT_TC(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
    LL_DMA_EnableIT_TE(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_BUSY;

    LL_SPI_Disable(SPI_BASE[spi_index]);
    LL_DMA_DisableStream(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemorySize(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphSize(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], LL_DMA_PDATAALIGN_BYTE);
    LL_SPI_SetDataWidth(SPI_BASE[spi_index], LL_SPI_DATAWIDTH_8BIT);

    drv_dma_clear_tc_flag(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
    drv_dma_clear_te_flag(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemoryAddress(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], (uint32_t)p_rx);
    LL_DMA_SetPeriphAddress(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], (uint32_t)&SPI_BASE[spi_index]->DR);
    LL_DMA_SetDataLength(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], (uint32_t)length);

    LL_DMA_EnableIT_TC(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
    LL_DMA_EnableIT_TE(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);

    gb_spi_rx_dma_flag[spi_index] = SPI_STATUS_BUSY;

    LL_SPI_EnableDMAReq_RX(SPI_BASE[spi_index]);
    LL_SPI_EnableDMAReq_TX(SPI_BASE[spi_index]);

    LL_DMA_EnableStream(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
    LL_DMA_EnableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_SPI_Enable(SPI_BASE[spi_index]);

    uint32_t start_time = HAL_GetTick();

    while ((SPI_STATUS_BUSY == gb_spi_tx_dma_flag[spi_index]) || (SPI_STATUS_BUSY == gb_spi_rx_dma_flag[spi_index]))
    {
        if ((HAL_GetTick() - start_time) > timeout)
        {
            LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
            LL_DMA_DisableStream(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
            LL_SPI_Disable(SPI_BASE[spi_index]);
            gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_TIMEOUT;
            return false;
        }
    }

    return true;
}

bool drv_spi_transmit_dma_16bit(SPI_TypeDef* SPIx, const uint16_t *p_tx, uint16_t length, uint32_t timeout)
{
    if ((NULL == SPIx) || (NULL == p_tx) || (0U == length))
    {
        return false;
    }

    const uint8_t spi_index = (SPIx == SPI1) ? 0U : 1U;
    if (spi_index >= 2U)
    {
        return false;
    }

    LL_SPI_Disable(SPI_BASE[spi_index]);
    LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemorySize(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_SetPeriphSize(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_PDATAALIGN_HALFWORD);
    //LL_DMA_SetMemoryIncMode(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_MEMORY_INCREMENT);
    LL_SPI_SetDataWidth(SPI_BASE[spi_index], LL_SPI_DATAWIDTH_16BIT);

    drv_dma_clear_tc_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
    drv_dma_clear_te_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemoryAddress(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)p_tx);
    LL_DMA_SetPeriphAddress(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)&SPI_BASE[spi_index]->DR);
    LL_DMA_SetDataLength(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)length);

    LL_DMA_EnableIT_TC(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
    LL_DMA_EnableIT_TE(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_BUSY;

    LL_SPI_EnableDMAReq_TX(SPI_BASE[spi_index]);
    LL_DMA_EnableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_SPI_Enable(SPI_BASE[spi_index]);

    uint32_t start_time = HAL_GetTick();

    while (SPI_STATUS_BUSY == gb_spi_tx_dma_flag[spi_index])
    {
        if ((HAL_GetTick() - start_time) > timeout)
        {
            LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
            LL_SPI_Disable(SPI_BASE[spi_index]);
            gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_TIMEOUT;
            return false;
        }
    }

    return true;
}

bool drv_spi_receive_dma_16bit(SPI_TypeDef* SPIx, const uint16_t *p_tx, const uint16_t *p_rx, uint16_t length, uint32_t timeout)
{
    if ((NULL == SPIx) || (NULL == p_tx) || (NULL == p_rx) || (0U == length))
    {
        return false;
    }

    const uint8_t spi_index = (SPIx == SPI1) ? 0U : 1U;
    if (spi_index >= 2U)
    {
        return false;
    }

    if (LL_SPI_IsActiveFlag_RXNE(SPI_BASE[spi_index]))
    {
        (void)LL_SPI_ReceiveData8(SPI_BASE[spi_index]);
    }

    LL_SPI_Disable(SPI_BASE[spi_index]);
    LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemorySize(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_SetPeriphSize(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_PDATAALIGN_HALFWORD);
    //LL_DMA_SetMemoryIncMode(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], LL_DMA_MEMORY_INCREMENT);
    LL_SPI_SetDataWidth(SPI_BASE[spi_index], LL_SPI_DATAWIDTH_16BIT);

    drv_dma_clear_tc_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
    drv_dma_clear_te_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemoryAddress(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)p_tx);
    LL_DMA_SetPeriphAddress(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)&SPI_BASE[spi_index]->DR);
    LL_DMA_SetDataLength(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index], (uint32_t)length);

    LL_DMA_EnableIT_TC(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
    LL_DMA_EnableIT_TE(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_BUSY;

    LL_SPI_Disable(SPI_BASE[spi_index]);
    LL_DMA_DisableStream(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemorySize(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_SetPeriphSize(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], LL_DMA_PDATAALIGN_HALFWORD);
    LL_SPI_SetDataWidth(SPI_BASE[spi_index], LL_SPI_DATAWIDTH_16BIT);

    drv_dma_clear_tc_flag(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
    drv_dma_clear_te_flag(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);

    LL_DMA_SetMemoryAddress(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], (uint32_t)p_rx);
    LL_DMA_SetPeriphAddress(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], (uint32_t)&SPI_BASE[spi_index]->DR);
    LL_DMA_SetDataLength(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index], (uint32_t)length);

    LL_DMA_EnableIT_TC(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
    LL_DMA_EnableIT_TE(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);

    gb_spi_rx_dma_flag[spi_index] = SPI_STATUS_BUSY;

    LL_SPI_EnableDMAReq_RX(SPI_BASE[spi_index]);
    LL_SPI_EnableDMAReq_TX(SPI_BASE[spi_index]);

    LL_DMA_EnableStream(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
    LL_DMA_EnableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);

    LL_SPI_Enable(SPI_BASE[spi_index]);

    uint32_t start_time = HAL_GetTick();

    while ((SPI_STATUS_BUSY == gb_spi_tx_dma_flag[spi_index]) || (SPI_STATUS_BUSY == gb_spi_rx_dma_flag[spi_index]))
    {
        if ((HAL_GetTick() - start_time) > timeout)
        {
            LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
            LL_DMA_DisableStream(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
            LL_SPI_Disable(SPI_BASE[spi_index]);
            gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_TIMEOUT;
            return false;
        }
    }

    return true;
}

bool drv_spi_tx_dma_irq_handler(SPI_TypeDef* SPIx)
{
    if (NULL == SPIx)
    {
        return false;
    }

    const uint8_t spi_index = (SPIx == SPI1) ? 0U : 1U;
    if (spi_index >= 2U)
    {
        return false;
    }

    if (true == drv_dma_is_active_tc_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]))
    {
        drv_dma_clear_tc_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
        while (true == LL_SPI_IsActiveFlag_BSY(SPI_BASE[spi_index]))
        {
            __NOP();
        }
        LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
        gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_DONE;
        gp_spi_cs_transition[spi_index](true);
        return true;
    }
    if (true == drv_dma_is_active_te_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]))
    {
        drv_dma_clear_te_flag(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
        LL_DMA_DisableStream(SPI_TX_DMA_BASE[spi_index], SPI_TX_DMA_STREAM[spi_index]);
        gb_spi_tx_dma_flag[spi_index] = SPI_STATUS_ERROR;
        gp_spi_cs_transition[spi_index](true);
        return false;
    }
    return false;
}

bool drv_spi_rx_dma_irq_handler(SPI_TypeDef* SPIx)
{
    if (NULL == SPIx)
    {
        return false;
    }

    const uint8_t spi_index = (SPIx == SPI1) ? 0U : 1U;
    if (spi_index >= 2U)
    {
        return false;
    }

    if (true == drv_dma_is_active_tc_flag(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]))
    {
        drv_dma_clear_tc_flag(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
        LL_DMA_DisableStream(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
        gb_spi_rx_dma_flag[spi_index] = SPI_STATUS_DONE;
        gp_spi_cs_transition[spi_index](true);
        return true;
    }
    if (true == drv_dma_is_active_te_flag(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]))
    {
        drv_dma_clear_te_flag(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
        LL_DMA_DisableStream(SPI_RX_DMA_BASE[spi_index], SPI_RX_DMA_STREAM[spi_index]);
        gb_spi_rx_dma_flag[spi_index] = SPI_STATUS_ERROR;
        gp_spi_cs_transition[spi_index](true);
        return false;
    }
    return false;
}
/* USER CODE END 0 */
