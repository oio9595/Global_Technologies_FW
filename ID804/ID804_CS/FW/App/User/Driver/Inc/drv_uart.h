/* USER CODE BEGIN Header */
/*
    * File:   drv_uart.h
    * Author: GT
    *
    * Created on 2026. 08. 26.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define MSG_BUFFER_SIZE             (128U)
#define UART_TX_RING_BUFFER_SIZE    (32U) // must be power of 2
#define UART_RX_RING_BUFFER_SIZE    (8U) // must be power of 2
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct tag_MSG_BUFFER
{
    char msg[MSG_BUFFER_SIZE];
    uint16_t size;
} msg_buffer_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
extern void drv_uart_init(void);

void drv_uart_set_tx_busy(bool is_busy);
bool drv_uart_is_tx_busy(void);
bool drv_uart_tx_data_pending(void);
msg_buffer_t* drv_uart_tx_ring_buffer_pop(void);
void drv_uart_tx_dma_start(msg_buffer_t* p_msg);

extern void drv_uart_printf_direct(const uint8_t* p_data, uint16_t size);
extern void drv_uart_printf(const char* format, ...);

bool drv_uart_rx_data_pending(void);
msg_buffer_t* drv_uart_rx_ring_buffer_pop(void);
void drv_uart_rx_irq_handler(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_UART_H__ */
