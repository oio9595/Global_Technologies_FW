/* USER CODE BEGIN Header */
/*
    * File:   drv_spi.h
    * Author: GT
    *
    * Created on 2026. 09. 01.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define SPI_DMA_DONE        (true)
#define SPI_DMA_BUSY        (false)
/* USER CODE END Private defines */

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

/* USER CODE BEGIN EFP */
extern void drv_spi_init(void);

extern void drv_spi_tx_dma_irq_handler(void);
extern void drv_spi_rx_dma_irq_handler(void);

extern bool drv_spi_transmit_direct(const uint8_t *p_data, uint16_t length);
extern bool drv_spi_receive_direct(uint8_t *p_data, uint16_t length);

extern bool drv_spi_transmit_dma(const uint8_t *p_data, uint16_t length);
extern bool drv_spi_receive_dma(uint8_t *p_data, uint16_t length);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_SPI_H__ */
