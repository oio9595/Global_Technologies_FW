/* 
 * File:   drv_comm.h
 * Author: GT
 *
 * Created on 2023, 04, 06
 */

#ifndef DRV_COMM_H
#define	DRV_COMM_H

#include "main.h"

__STATIC_INLINE void UART_PutChar(uint8_t data)
{
    /* Loop until the end of transmission */
    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
    /* Echo received character on TX */
    LL_USART_TransmitData8(USART2, (uint8_t)data);
}

__STATIC_INLINE void UART_Tx_DMA_Start(char* buffer, uint16_t dataNumber)
{
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)buffer);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, dataNumber);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
}

#endif	/* DRV_COMM_H */

