/* USER CODE BEGIN Header */
/**
     ******************************************************************************
    * @file           : drv_uart.c
    * @brief          : UART driver implementation
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
#include "drv_uart.h"
/* 2. C standard library headers (Alphabetical order) */
#include <stdio.h>
/* 3. Project internal / System-related headers */
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**
 * @brief  Structure representing the UART TX ring buffer.
 * @note   This structure is used internally by the UART driver to manage the buffer in a circular manner.
 */
typedef struct tag_RING_BUFFER
{
    msg_buffer_t buffer[UART_TX_RING_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} tx_ring_buffer_t;

typedef struct tag_RING_BUFFER_RX
{
    msg_buffer_t buffer[UART_RX_RING_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} rx_ring_buffer_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART2_TX_DMA_BASE       DMA1
#define UART2_TX_DMA_STREAM     LL_DMA_STREAM_6
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static tx_ring_buffer_t gt_uart_tx;
static rx_ring_buffer_t gt_uart_rx;

bool gb_uart_tx_dma_progress = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief  Initialize the UART driver, including TX and RX ring buffers, DMA, and interrupts.
 */
void drv_uart_init(void)
{
    gt_uart_tx.head = 0U;
    gt_uart_tx.tail = 0U;

    gt_uart_rx.head = 0U;
    gt_uart_rx.tail = 0U;

    LL_USART_EnableDMAReq_TX(USART2);
    LL_USART_EnableIT_RXNE(USART2);

    LL_DMA_EnableIT_TC(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM);
    LL_DMA_EnableIT_TE(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM);
    LL_DMA_SetPeriphAddress(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM, (uint32_t)&(USART2->DR));
}

/**
 * @brief  Set the UART TX busy status.
 * @param  is_busy  Boolean indicating whether the UART TX is busy.
 */
void drv_uart_set_tx_busy(bool is_busy)
{
    gb_uart_tx_dma_progress = is_busy;
}

/**
 * @brief  Check if the UART TX is busy.
 * @retval true if the UART TX is busy, false otherwise.
 */
bool drv_uart_is_tx_busy(void)
{
    return gb_uart_tx_dma_progress;
}

/**
 * @brief  Check if there is pending data in the UART TX ring buffer.
 * @retval true if there is pending data, false otherwise.
 */
bool drv_uart_tx_data_pending(void)
{
    if (gt_uart_tx.head != gt_uart_tx.tail)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief  Push a string into the UART TX ring buffer.
 * @param  str  Pointer to the null-terminated string to be pushed into the buffer.
 * @note   This function is used internally by the UART driver to manage outgoing data.
 */
static void drv_uart_tx_ring_buffer_push(const char* str)
{
    msg_buffer_t* current_msg = &gt_uart_tx.buffer[gt_uart_tx.head];
    current_msg->size = 0U;

    while (*str != '\0')
    {
        current_msg->msg[current_msg->size++] = (uint8_t)(*str);
        ++str;
    }
    gt_uart_tx.head = ((gt_uart_tx.head + 1U) % UART_TX_RING_BUFFER_SIZE);
}

/**
 * @brief  Pop a message from the UART TX ring buffer.
 * @retval Pointer to the message buffer structure containing the next message to be transmitted.
 */
msg_buffer_t* drv_uart_tx_ring_buffer_pop(void)
{
    msg_buffer_t* p_msg = NULL;
    p_msg = &gt_uart_tx.buffer[gt_uart_tx.tail];
    gt_uart_tx.tail = (gt_uart_tx.tail + 1U) % UART_TX_RING_BUFFER_SIZE;
    return p_msg;
}

/**
 * @brief  Start the UART TX DMA transfer for the given message.
 * @param  p_msg  Pointer to the message buffer structure containing the data to be transmitted.
 * @note   This function will block until the UART TX is not busy.
 */
void drv_uart_tx_dma_start(msg_buffer_t* p_msg)
{
    if (p_msg == NULL)
    {
        return;
    }

    while (true == drv_uart_is_tx_busy())
    {
    }

    LL_DMA_SetMemoryAddress(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM, (uint32_t)(p_msg->msg));
    LL_DMA_SetDataLength(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM, p_msg->size);
    drv_uart_set_tx_busy(true);

    LL_DMA_ClearFlag_TC6(UART2_TX_DMA_BASE);
    LL_DMA_ClearFlag_TE6(UART2_TX_DMA_BASE);
    LL_DMA_EnableStream(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM);
}

/**
 * @brief  Directly transmit a block of data over UART without using the TX ring buffer.
 * @param  p_data  Pointer to the data to be transmitted.
 * @param  size    Size of the data block to be transmitted.
 * @note   This function bypasses the TX ring buffer and sends data directly over UART.
 */
void drv_uart_printf_direct(const uint8_t* p_data, uint16_t size)
{
    if (p_data == NULL || size == 0U)
    {
        return;
    }

    for (uint16_t i = 0U; i < size; ++i)
    {
        while (false == LL_USART_IsActiveFlag_TXE(USART2));
        LL_USART_TransmitData8(USART2, p_data[i]);
    }
}

/**
 * @brief  Formatted print over UART using the TX ring buffer.
 * @param  format  Format string (similar to printf).
 * @note   This function formats the string and pushes it into the UART TX ring buffer.
 */
void drv_uart_printf(const char* format, ...)
{
    char buffer[MSG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0)
    {
        drv_uart_tx_ring_buffer_push(buffer);
    }
}

/**
 * @brief  Check if there is any pending data in the UART RX ring buffer.
 * @retval true if there is pending data, false otherwise.
 */
bool drv_uart_rx_data_pending(void)
{
    if (gt_uart_rx.head != gt_uart_rx.tail)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief  Push received data into the UART RX ring buffer.
 * @param  received_data  The byte of data received from UART.
 * @note   This function is used internally by the UART driver to manage incoming data.
 */
static void drv_uart_rx_ring_buffer_push(uint8_t received_data)
{
    msg_buffer_t* current_msg = &gt_uart_rx.buffer[gt_uart_rx.head];
    switch (received_data)
    {
        case '\r':
        case '\n':
        {
            drv_uart_printf_direct((uint8_t*)"\r\n", 2U);
            current_msg->msg[current_msg->size] = '\0';

            gt_uart_rx.head = (gt_uart_rx.head + 1U) % UART_RX_RING_BUFFER_SIZE;
            gt_uart_rx.buffer[gt_uart_rx.head].size = 0U;
            break;
        }
        case '\b':
        {
            if (current_msg->size > 0U)
            {
                current_msg->size--;
                current_msg->msg[current_msg->size] = '\0';
                drv_uart_printf_direct((uint8_t*)"\b \b", 3U);
            }
            break;
        }
        default:
        {
            drv_uart_printf_direct(&received_data, 1U);
            current_msg->msg[current_msg->size++] = received_data;
            current_msg->msg[current_msg->size] = '\0';
            break;
        }
    }
}

/**
 * @brief  Pop a message from the UART RX ring buffer.
 * @retval Pointer to the message buffer structure containing the next received message.
 */
msg_buffer_t* drv_uart_rx_ring_buffer_pop(void)
{
    msg_buffer_t* p_msg = &gt_uart_rx.buffer[gt_uart_rx.tail];
    gt_uart_rx.tail = (gt_uart_rx.tail + 1U) % UART_RX_RING_BUFFER_SIZE;
    return p_msg;
}

/**
 * @brief  UART RX interrupt handler.
 * @note   This function should be called from the actual UART RX interrupt service routine.
 */
void drv_uart_rx_irq_handler(void)
{
    if (LL_USART_IsActiveFlag_RXNE(USART2))
    {
        uint8_t received_data = (uint8_t)LL_USART_ReceiveData8(USART2);
        drv_uart_rx_ring_buffer_push(received_data);
    }
}
/* USER CODE END 0 */
