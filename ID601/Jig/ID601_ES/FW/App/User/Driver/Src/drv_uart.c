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
#include <string.h>
/* 3. Project internal / System-related headers */
#include "main.h"
#include "cmsis_os.h"
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

#define CLI_KEY_UP              (0x41)
#define CLI_KEY_DOWN            (0x42)
#define CLI_KEY_RIGHT           (0x43)
#define CLI_KEY_LEFT            (0x44)

#define LOG_COLOR_RESET         "\x1B[0m"
#define LOG_COLOR_ERROR         "\x1B[31m"
#define LOG_COLOR_WARN          "\x1B[33m"
#define LOG_COLOR_INFO          "\x1B[32m"
#define LOG_COLOR_DEBUG         "\x1B[36m"
#define LOG_COLOR_TRACE         "\x1B[90m"

#define LOG_LV_MSG_OFF          "LOG_LV_OFF"
#define LOG_LV_MSG_ERROR        "LOG_LV_ERROR"
#define LOG_LV_MSG_WARN         "LOG_LV_WARN"
#define LOG_LV_MSG_INFO         "LOG_LV_INFO"
#define LOG_LV_MSG_DEBUG        "LOG_LV_DEBUG"
#define LOG_LV_MSG_TRACE        "LOG_LV_TRACE"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static osMutexId_t uart_tx_mutex;

static tx_ring_buffer_t gt_uart_tx;
static rx_ring_buffer_t gt_uart_rx;

bool gb_uart_tx_dma_progress = false;

static log_level_t gt_log_level;

static const char* const gp_log_colors[LOG_LV_MAX] =
{
    [LOG_LV_OFF]   = LOG_COLOR_RESET,
    [LOG_LV_ERROR] = LOG_COLOR_ERROR,
    [LOG_LV_WARN]  = LOG_COLOR_WARN,
    [LOG_LV_INFO]  = LOG_COLOR_INFO,
    [LOG_LV_DEBUG] = LOG_COLOR_DEBUG,
    [LOG_LV_TRACE] = LOG_COLOR_TRACE,
};

static const char* const gp_log_msgs[LOG_LV_MAX] =
{
    [LOG_LV_OFF]   = LOG_LV_MSG_OFF,
    [LOG_LV_ERROR] = LOG_LV_MSG_ERROR,
    [LOG_LV_WARN]  = LOG_LV_MSG_WARN,
    [LOG_LV_INFO]  = LOG_LV_MSG_INFO,
    [LOG_LV_DEBUG] = LOG_LV_MSG_DEBUG,
    [LOG_LV_TRACE] = LOG_LV_MSG_TRACE,
};
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

    //LL_USART_EnableDMAReq_TX(USART2);
    LL_USART_EnableIT_RXNE(USART2);

    //LL_DMA_EnableIT_TC(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM);
    //LL_DMA_EnableIT_TE(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM);
    //LL_DMA_SetPeriphAddress(UART2_TX_DMA_BASE, UART2_TX_DMA_STREAM, (uint32_t)&(USART2->DR));

    gt_log_level = LOG_LV_DEBUG;
    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [MCU] UART initialized successfully.");
}
const char* drv_uart_get_log_level(void)
{
    return gp_log_msgs[gt_log_level];
}
void drv_uart_make_mutex(void)
{
    uart_tx_mutex = osMutexNew(NULL);
    if (NULL == uart_tx_mutex)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [UART] %s: Failed to create UART TX mutex.", __func__);
    }
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

    while ('\0' != *str)
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
    gt_uart_tx.tail = ((gt_uart_tx.tail + 1U) % UART_TX_RING_BUFFER_SIZE);
    return p_msg;
}

/**
 * @brief  Start the UART TX DMA transfer for the given message.
 * @param  p_msg  Pointer to the message buffer structure containing the data to be transmitted.
 * @note   This function will block until the UART TX is not busy.
 */
void drv_uart_tx_dma_start(msg_buffer_t* p_msg)
{
    if (NULL == p_msg)
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

static void uart_tx_polling(const uint8_t* p_data, uint16_t size)
{
    for (uint16_t i = 0U; i < size; ++i)
    {
        while (false == LL_USART_IsActiveFlag_TXE(USART2))
        {
        }

        LL_USART_TransmitData8(USART2, p_data[i]);
    }
}

/**
 * @brief  Directly transmit a block of data over UART without using the TX ring buffer.
 * @param  p_data  Pointer to the data to be transmitted.
 * @param  size    Size of the data block to be transmitted.
 * @note   This function bypasses the TX ring buffer and sends data directly over UART.
 */
void drv_uart_printf_direct(const uint8_t* p_data, uint16_t size)
{
    if ((NULL == p_data) || (0U == size))
    {
        return;
    }

    if (osOK != osMutexAcquire(uart_tx_mutex, osWaitForever))
    {
        return;
    }

    uart_tx_polling(p_data, size);

    osMutexRelease(uart_tx_mutex);
}

/**
 * @brief  Formatted print over UART using the TX ring buffer.
 * @param  format  Format string (similar to printf).
 * @note   This function formats the string and pushes it into the UART TX ring buffer.
 */
void drv_uart_printf(log_level_t level, const char* format, ...)
{
    if ((level > gt_log_level) || (NULL == format))
    {
        return;
    }

    char buffer[MSG_BUFFER_SIZE];

    const char *color = gp_log_colors[level];
    const size_t color_len = strlen(color);
    const size_t reset_len = strlen(LOG_COLOR_RESET);

    if (sizeof(buffer) < (color_len + reset_len + 2U))
    {
        return;
    }

    memcpy(buffer, color, color_len);

    const size_t message_capacity = (sizeof(buffer) - color_len - reset_len);

    va_list args;
    va_start(args, format);
    const int len = vsnprintf(buffer + color_len, message_capacity, format, args);
    va_end(args);

    if (len <= 0)
    {
        return;
    }

    const size_t message_len = ((size_t)len < message_capacity) ? (size_t)len : (message_capacity - 1U);
    memcpy(buffer + color_len + message_len, LOG_COLOR_RESET, reset_len);
    buffer[color_len + message_len + reset_len] = '\0';

    drv_uart_tx_ring_buffer_push(buffer);
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
    static uint8_t vt100_state = 0U;
    msg_buffer_t* current_msg = &gt_uart_rx.buffer[gt_uart_rx.head];

    if (1U == vt100_state)
    {
        if ('[' == received_data)
        {
            vt100_state = 2U;
            return;
        }
        vt100_state = 0U;
    }
    else if (2U == vt100_state)
    {
        vt100_state = 0U;
        switch (received_data)
        {
            case CLI_KEY_UP:
            {
                uint8_t prev_head = (gt_uart_rx.head + UART_RX_RING_BUFFER_SIZE - 1U) % UART_RX_RING_BUFFER_SIZE;
                msg_buffer_t* prev_msg = &gt_uart_rx.buffer[prev_head];

                if (prev_msg->size > 0U)
                {
                    memcpy(current_msg->msg, prev_msg->msg, sizeof(prev_msg->msg));
                    current_msg->size = prev_msg->size;

                    for (uint8_t i = 0U; i < current_msg->size; i++)
                    {
                        uart_tx_polling((uint8_t*)&current_msg->msg[i], 1U);
                    }
                }
                break;
            }
            case CLI_KEY_DOWN:
            {
                /* code for down key */
                break;
            }
            case CLI_KEY_RIGHT:
            {
                /* code for right key */
                break;
            }
            case CLI_KEY_LEFT:
            {
                /* code for left key */
                break;
            }
            default:
            {
                break;
            }
        }
        return;
    }

    switch (received_data)
    {
        case '\r':
        case '\n':
        {
            uart_tx_polling((uint8_t*)"\r\n", 2U);
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
                uart_tx_polling((uint8_t*)"\b \b", 3U);
            }
            break;
        }
        case '\x1B': // ESC 키 수신 시 시퀀스 파싱 시작
        {
            vt100_state = 1U;
            break;
        }
        default:
        {
            if (current_msg->size < (MSG_BUFFER_SIZE - 1U))
            {
                uart_tx_polling(&received_data, 1U);
                current_msg->msg[current_msg->size++] = received_data;
                current_msg->msg[current_msg->size] = '\0';
            }
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
    if (true == LL_USART_IsActiveFlag_RXNE(USART2))
    {
        uint8_t received_data = (uint8_t)LL_USART_ReceiveData8(USART2);
        drv_uart_rx_ring_buffer_push(received_data);
    }
}
/* USER CODE END 0 */
