/** @file uart.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_LOG_LV_T
{
    LOG_DEBUG = 0U,
    LOG_INFO,
    LOG_ERROR,
    LOG_MAX,
} LOG_LV_T;
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern bool gb_uart_tx_busy;
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
extern void Comm_Rx_Handler(uint8_t rx);
extern void Print(LOG_LV_T log_lv, const char *fmt, ...);
extern void Comm_Increase_Tx_OutCnt(void);
extern void Comm_Init(void);
extern void Comm_UART_Task(void);
/* USER CODE END */

#ifdef __cplusplus
}
#endif

#endif /* ~__UART_H__ */

/*** end of file ***/