/* 
 * File:   drv_comm.h
 * Author: GT
 *
 * Created on 2023, 04, 06
 */

#ifndef DRV_COMM_H
#define	DRV_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

__STATIC_INLINE void UART_PutChar(uint8_t data)
{
    /* Loop until the end of transmission */
    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
    /* Echo received character on TX */
    LL_USART_TransmitData8(USART2, data);
}

__STATIC_INLINE void UART_Tx_DMA_Start(uint32_t mem_addr, uint16_t dataNumber)
{
    LL_USART_EnableIT_TXE(USART2);
}

#ifdef __cplusplus
}
#endif

#endif	/* DRV_COMM_H */

