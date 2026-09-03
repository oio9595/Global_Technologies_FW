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
#define SPI_TIMEOUT_MS  (100U)
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
void drv_spi_init(void)
{
    /* Implement the SPI initialization functionality here */
}

bool drv_spi_transmit_direct(const uint16_t *p_data, uint16_t length)
{
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
        LL_SPI_TransmitData16(SPI1, p_data[i]);
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

bool drv_spi_receive_direct(uint16_t *p_data, uint16_t length)
{
    /* Configure SPI1 as slave, simplex RX, CPOL high */
    LL_SPI_Disable(SPI1);
    LL_SPI_SetMode(SPI1, LL_SPI_MODE_SLAVE);
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_RX);
    LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);
    LL_SPI_Enable(SPI1);

    if (LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
        (void)LL_SPI_ReceiveData16(SPI1);
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
        p_data[i] = LL_SPI_ReceiveData16(SPI1);
    }

    LL_SPI_Disable(SPI1);
    return true;
}
/* USER CODE END 0 */
