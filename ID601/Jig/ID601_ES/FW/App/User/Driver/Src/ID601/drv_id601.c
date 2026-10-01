/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_id601.c
    * @brief          : ID601 driver implementation
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
#include "drv_id601.h"
/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_timer.h"
#include "drv_uart.h"
#include "drv_gpio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ID601_DAISY_LENGTH_MAX              (5U)
#define ID601_DAISY_LENGTH_INITIAL          (3U)
#define ID601_BROADCAST                     (0U)
#define ID601_CHANNEL_SIZE                  (12U)

#define ID601_SERIAL_TIMEOUT_MS             (100U)

#define ID601_SERIAL_RATIO_BIT0             (1U)
#define ID601_SERIAL_RATIO_BIT1             (2U)
#define ID601_SERIAL_RATIO_SUM              (ID601_SERIAL_RATIO_BIT0 + ID601_SERIAL_RATIO_BIT1)

/********************** ID601 CRC **********************/
#define ID601_CRC8_BIT_COUNT                (8U)

#define ID601_CRC8_POLY                     (0x1DU)
#define ID601_CRC8_INIT                     (0xFFU)
#define ID601_CRC8_XOROUT                   (0xFFU)

/********************** ID601 Write Frame **********************/
/* [Command field:4][Address:6][Data:12] */
#define ID601_WRITE_PAYLOAD_BIT_COUNT       (22U)
#define ID601_WRITE_SERIAL_BIT_COUNT        (ID601_WRITE_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_WRITE_COMMAND                 (0x0DU)
#define ID601_WRITE_COMMAND_Pos             (18U)
#define ID601_WRITE_COMMAND_Msk             (0x000FU << ID601_WRITE_COMMAND_Pos)

#define ID601_WRITE_ADDRESS_Pos             (12U)
#define ID601_WRITE_ADDRESS_Msk             (0x001FU << ID601_WRITE_ADDRESS_Pos)

#define ID601_WRITE_DATA_Pos                (0U)
#define ID601_WRITE_DATA_Msk                (0x0FFFU << ID601_WRITE_DATA_Pos)

/********************** ID601 Read Frame **********************/
/* [Command field:4][Address:6] */
#define ID601_READ_PAYLOAD_BIT_COUNT        (10U)
#define ID601_READ_SERIAL_BIT_COUNT         (ID601_READ_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_READ_COMMAND                  (0x0EU)
#define ID601_READ_COMMAND_Pos              (6U)
#define ID601_READ_COMMAND_Msk              (0x000FU << ID601_READ_COMMAND_Pos)

#define ID601_READ_ADDRESS_Pos              (0U)
#define ID601_READ_ADDRESS_Msk              (0x003FU << ID601_READ_ADDRESS_Pos)

/********************** ID601 Read Receive Frame **********************/
/* [Command field:4][Device ID:5][Data:12] */
#define ID601_READ_RECV_PAYLOAD_BIT_COUNT   (21U)
#define ID601_READ_RECV_SERIAL_BIT_COUNT    (ID601_READ_RECV_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_READ_RECV_COMMAND             (0x0EU)
#define ID601_READ_RECV_COMMAND_Pos         (17U)
#define ID601_READ_RECV_COMMAND_Msk         (0x000FU << ID601_READ_RECV_COMMAND_Pos)

#define ID601_READ_RECV_DEVICE_ID_Pos       (12U)
#define ID601_READ_RECV_DEVICE_ID_Msk       (0x001FU << ID601_READ_RECV_DEVICE_ID_Pos)

#define ID601_READ_RECV_DATA_Pos            (0U)
#define ID601_READ_RECV_DATA_Msk            (0x0FFFU << ID601_READ_RECV_DATA_Pos)

/********************** ID601 LD Trans 12bit Frame **********************/
/* [Command field:4][LD:12 * CHANNEL] */
#define ID601_LD_12B_PAYLOAD_BIT_COUNT      (4U + 12U * ID601_CHANNEL_SIZE)
#define ID601_LD_12B_SERIAL_BIT_COUNT       (ID601_LD_12B_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_LD_12B_COMMAND                (0x0FU)
#define ID601_LD_12B_COMMAND_Pos            (12U * ID601_CHANNEL_SIZE)
#define ID601_LD_12B_COMMAND_Msk            (0x000FU << ID601_LD_12B_COMMAND_Pos)

/********************** ID601 LD Trans 14bit Frame **********************/
/* [Command field:4][LD:14 * CHANNEL] */
#define ID601_LD_14B_PAYLOAD_BIT_COUNT      (4U + 14U * ID601_CHANNEL_SIZE)
#define ID601_LD_14B_SERIAL_BIT_COUNT       (ID601_LD_14B_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_LD_14B_COMMAND                (0x0FU)
#define ID601_LD_14B_COMMAND_Pos            (14U * ID601_CHANNEL_SIZE)
#define ID601_LD_14B_COMMAND_Msk            (0x000FU << ID601_LD_14B_COMMAND_Pos)

/********************** ID601 Fault Read Frame **********************/
/* [Command field:4] */
#define ID601_FAULT_READ_PAYLOAD_BIT_COUNT  (4U)
#define ID601_FAULT_READ_SERIAL_BIT_COUNT   (ID601_FAULT_READ_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_FAULT_READ_COMMAND            (0x0AU)
#define ID601_FAULT_READ_COMMAND_Pos        (0U)
#define ID601_FAULT_READ_COMMAND_Msk        (0x000FU << ID601_FAULT_READ_COMMAND_Pos)

/********************** ID601 Fault Recv Receive Frame **********************/
/* [Command field:4][Fault status:4] */
#define ID601_FAULT_RECV_PAYLOAD_BIT_COUNT  (8U)
#define ID601_FAULT_RECV_SERIAL_BIT_COUNT   (ID601_FAULT_RECV_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_FAULT_RECV_COMMAND            (0x0AU)
#define ID601_FAULT_RECV_COMMAND_Pos        (4U)
#define ID601_FAULT_RECV_COMMAND_Msk        (0x000FU << ID601_FAULT_RECV_COMMAND_Pos)

#define ID601_FAULT_RECV_STATUS_Pos         (0U)
#define ID601_FAULT_RECV_STATUS_Msk         (0x000FU << ID601_FAULT_RECV_STATUS_Pos)

/********************** ID601 Syncgen Frame **********************/
/* [Command field:4] */
#define ID601_SYNCGEN_PAYLOAD_BIT_COUNT     (4U)
#define ID601_SYNCGEN_SERIAL_BIT_COUNT      (ID601_SYNCGEN_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_SYNCGEN_COMMAND               (0x09U)
#define ID601_SYNCGEN_COMMAND_Pos           (0U)
#define ID601_SYNCGEN_COMMAND_Msk           (0x000FU << ID601_SYNCGEN_COMMAND_Pos)

/********************** ID601 IDGEN Frame **********************/
/* [Command field:4] */
#define ID601_IDGEN_PAYLOAD_BIT_COUNT       (4U)
#define ID601_IDGEN_SERIAL_BIT_COUNT        (ID601_IDGEN_PAYLOAD_BIT_COUNT + ID601_CRC8_BIT_COUNT)

#define ID601_IDGEN_COMMAND                 (0x08U)
#define ID601_IDGEN_COMMAND_Pos             (0U)
#define ID601_IDGEN_COMMAND_Msk             (0x000FU << ID601_IDGEN_COMMAND_Pos)

#define ID601_TX_CCR_BUFFER_CAPACITY        (ID601_DAISY_LENGTH_MAX * ID601_LD_14B_SERIAL_BIT_COUNT)
#define ID601_RX_CCR_BUFFER_CAPACITY        (ID601_DAISY_LENGTH_MAX * ID601_READ_RECV_SERIAL_BIT_COUNT)

#define ID601_DELAY_RESET_MS                (1U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint16_t gn_id601_tx_ccr_buffer[ID601_TX_CCR_BUFFER_CAPACITY] = { 0U };
static uint16_t gn_id601_rx_ccr_rise_buffer[ID601_RX_CCR_BUFFER_CAPACITY] = { 0U };
static uint16_t gn_id601_rx_ccr_fall_buffer[ID601_RX_CCR_BUFFER_CAPACITY] = { 0U };

static uint16_t gn_id601_serial_bit0_ccr;
static uint16_t gn_id601_serial_bit1_ccr;

static uint16_t gn_id601_daisy_length;

static id601_crc_func_t gp_id601_crc_func = NULL;

static uint16_t gn_id601_general_regs[ID601_ADDR_GENERAL_COUNT][ID601_DAISY_LENGTH_MAX];
static uint16_t gn_id601_mirror_regs[ID601_ADDR_MIRROR_COUNT][ID601_DAISY_LENGTH_MAX];
static uint16_t gn_id601_fault;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drv_id601_init(void)
{
    uint16_t const tim1_period = LL_TIM_GetAutoReload(TIM1);
    gn_id601_serial_bit0_ccr = ((tim1_period * ID601_SERIAL_RATIO_BIT0) / ID601_SERIAL_RATIO_SUM);
    gn_id601_serial_bit1_ccr = ((tim1_period * ID601_SERIAL_RATIO_BIT1) / ID601_SERIAL_RATIO_SUM);

    gn_id601_daisy_length = ID601_DAISY_LENGTH_INITIAL;
}

static uint8_t id601_calculate_crc(uint32_t data, uint16_t size)
{
    if (size == 0U)
    {
        return 0U;
    }

    uint8_t crc = ID601_CRC8_INIT;
    uint32_t mask = 0U;
    for (uint32_t i = 0U; i < size; i++)
    {
        mask |= 1U << i;
    }
    data &= mask;
    int8_t repeat = (size - 1U);

    for (int i = repeat; i >= 0; i--)
    {
        uint8_t data_bit = (data >> i) & 0x01;
        uint8_t feedback = ((crc >> 7) & 0x01) ^ data_bit;

        crc <<= 1;

        if (feedback)
            crc ^= ID601_CRC8_POLY;
    }

    return crc ^ ID601_CRC8_XOROUT;
}

static uint16_t id601_encode_ccr_buffer_from_serial(uint32_t serial, uint16_t* p_cnt, uint16_t length)
{
    if ((NULL == p_cnt) || (0U == length))
    {
        drv_uart_printf("\r\n    (%s)Invalid CCR buffer or length.", __func__);
        return 0U;
    }
    uint16_t count = 0U;
    for (uint16_t i = 0; i < length; i++)
    {
        uint16_t shift_amount = (length - i) - 1U;
        if ((serial >> shift_amount) & 0x01U)
        {
            p_cnt[count++] = gn_id601_serial_bit1_ccr;
        }
        else
        {
            p_cnt[count++] = gn_id601_serial_bit0_ccr;
        }
    }
    return count;
}

static bool id601_decode_serial_from_ccr_buffer(uint16_t* p_rise, uint16_t* p_fall, uint32_t* p_serial, uint16_t length)
{
    if ((NULL == p_rise) || (NULL == p_fall) || (NULL == p_serial) || (0U == length))
    {
        drv_uart_printf("\r\n    (%s)Invalid CCR buffer or length.", __func__);
        return false;
    }
    uint16_t rise_cnt_sum = 0U;
    float freq_avg = 0.0f;
    for (uint16_t i = 1U; i < length; i++) // exclude the first element
    {
        rise_cnt_sum += p_rise[i];
    }
    freq_avg = (float)(rise_cnt_sum + 1U) / (float)(length - 1U); // exclude the first element

    const uint16_t cnt_half_duty = (uint16_t)(freq_avg / 2.0f);

    for (uint16_t i = 0U; i < length; i++)
    {
        if (p_rise[i] > cnt_half_duty)
        {
            *p_serial |= (1U) << (length - i - 1U);
        }
        else
        {
            *p_serial &= ~((1U) << (length - i - 1U));
        }
    }
    return true;
}

static inline uint16_t id601_set_field(uint16_t reg_value, uint16_t field_msk, uint16_t field_pos, uint16_t field_value)
{
    reg_value &= (uint16_t)~field_msk;
    reg_value |= (uint16_t)((field_value << field_pos) & field_msk);

    return reg_value;
}

static inline uint16_t id601_get_field(uint16_t reg_value, uint16_t field_msk, uint16_t field_pos)
{
    return (uint16_t)((reg_value & field_msk) >> field_pos);
}

static inline uint32_t id601_build_write_payload(uint16_t addr, uint16_t data)
{
    uint32_t payload = 0U;
    payload = id601_set_field(payload, ID601_WRITE_COMMAND_Msk, ID601_WRITE_COMMAND_Pos, ID601_WRITE_COMMAND);
    payload = id601_set_field(payload, ID601_WRITE_ADDRESS_Msk, ID601_WRITE_ADDRESS_Pos, addr);
    payload = id601_set_field(payload, ID601_WRITE_DATA_Msk, ID601_WRITE_DATA_Pos, data);
    return payload;
}

static bool id601_write_command(uint16_t addr, const uint16_t* p_data, id601_crc_func_t p_crc_func)
{
    if (p_data == NULL)
    {
        drv_uart_printf("\r\n    (%s)Invalid data pointer.", __func__);
        return false;
    }

    uint16_t tx_ccr_count = 0U;
    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        uint32_t payload = id601_build_write_payload(addr, p_data[device_index]);
        uint16_t frame_length = ID601_WRITE_PAYLOAD_BIT_COUNT;

        if (NULL != p_crc_func)
        {
            const uint8_t crc = p_crc_func(payload, frame_length);
            payload = ((payload << ID601_CRC8_BIT_COUNT) | (uint32_t)crc);
            frame_length += ID601_CRC8_BIT_COUNT;
        }
        tx_ccr_count += id601_encode_ccr_buffer_from_serial(payload, (gn_id601_tx_ccr_buffer + tx_ccr_count), frame_length);
    }

    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    return drv_tim_generate_serial((uint16_t*)gn_id601_tx_ccr_buffer, tx_ccr_count, ID601_SERIAL_TIMEOUT_MS);
}

static inline uint32_t id601_build_read_payload(uint16_t addr)
{
    uint32_t payload = 0U;
    payload = id601_set_field(payload, ID601_READ_COMMAND_Msk, ID601_READ_COMMAND_Pos, ID601_READ_COMMAND);
    payload = id601_set_field(payload, ID601_READ_ADDRESS_Msk, ID601_READ_ADDRESS_Pos, addr);
    return payload;
}

static bool id601_read_command(uint16_t addr, id601_crc_func_t p_crc_func)
{
    uint16_t tx_ccr_count = 0U;
    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        uint32_t payload = id601_build_read_payload(addr);
        uint16_t frame_length = ID601_READ_PAYLOAD_BIT_COUNT;

        if (NULL != p_crc_func)
        {
            const uint8_t crc = p_crc_func(payload, frame_length);
            payload = ((payload << ID601_CRC8_BIT_COUNT) | (uint32_t)crc);
            frame_length += ID601_CRC8_BIT_COUNT;
        }
        tx_ccr_count += id601_encode_ccr_buffer_from_serial(payload, (gn_id601_tx_ccr_buffer + tx_ccr_count), frame_length);
    }

    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    return drv_tim_generate_serial((uint16_t*)gn_id601_tx_ccr_buffer, tx_ccr_count, ID601_SERIAL_TIMEOUT_MS);
}

static bool id601_read_receive_command(uint16_t* p_data, id601_crc_func_t p_crc_func)
{
    if (NULL == p_data)
    {
        drv_uart_printf("\r\n    (%s)Invalid data pointer.", __func__);
        return false;
    }

    uint16_t frame_length = ID601_WRITE_PAYLOAD_BIT_COUNT;
    if (NULL != p_crc_func)
    {
        frame_length += ID601_CRC8_BIT_COUNT;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        uint16_t* p_rise = &gn_id601_rx_ccr_rise_buffer[device_index * frame_length];
        uint16_t* p_fall = &gn_id601_rx_ccr_fall_buffer[device_index * frame_length];
        if (false == drv_tim_capture_serial(p_rise, p_fall, frame_length, ID601_SERIAL_TIMEOUT_MS))
        {
            return false;
        }
        uint32_t serial_data = 0U;
        if (false == id601_decode_serial_from_ccr_buffer(p_rise, p_fall, &serial_data, ID601_READ_RECV_PAYLOAD_BIT_COUNT))
        {
            return false;
        }
        uint16_t command = id601_get_field(serial_data, ID601_READ_RECV_COMMAND_Msk, ID601_READ_RECV_COMMAND_Pos);
        uint16_t ID = id601_get_field(serial_data, ID601_READ_RECV_DEVICE_ID_Msk, ID601_READ_RECV_DEVICE_ID_Pos);
        uint16_t data = id601_get_field(serial_data, ID601_READ_RECV_DATA_Msk, ID601_READ_RECV_DATA_Pos);
        drv_uart_printf("\r\n    [ID601 Read Recv] [device_index: %u, command: 0x%02X, ID: 0x%02X, data: 0x%03X]", device_index, command, ID, data);
        p_data[device_index] = data;
    }

    return true;
}

static inline uint32_t id601_build_ld_12B_transfer_payload(uint16_t* p_data)
{
    return 0U;
}

static bool id601_ld_12B_transfer_command(uint16_t* p_data, id601_crc_func_t p_crc_func)
{
    id601_build_ld_12B_transfer_payload(/*p_data*/NULL);
    return false;
}

static inline uint32_t id601_build_ld_14B_transfer_payload(uint16_t* p_data)
{
    return 0U;
}

static bool id601_ld_14B_transfer_command(uint16_t* p_data, id601_crc_func_t p_crc_func)
{
    id601_build_ld_14B_transfer_payload(/*p_data*/NULL);
    return false;
}

static inline uint32_t id601_build_fault_read_payload(void)
{
    uint32_t payload = 0U;
    payload = id601_set_field(payload, ID601_FAULT_READ_COMMAND_Msk, ID601_FAULT_READ_COMMAND_Pos, ID601_FAULT_READ_COMMAND);
    return payload;
}

static bool id601_fault_read_command(id601_crc_func_t p_crc_func)
{
    uint16_t tx_ccr_count = 0U;
    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        uint32_t payload = id601_build_fault_read_payload();
        uint16_t frame_length = ID601_FAULT_READ_PAYLOAD_BIT_COUNT;

        if (NULL != p_crc_func)
        {
            const uint8_t crc = p_crc_func(payload, frame_length);
            payload = ((payload << ID601_CRC8_BIT_COUNT) | (uint32_t)crc);
            frame_length += ID601_CRC8_BIT_COUNT;
        }
        tx_ccr_count += id601_encode_ccr_buffer_from_serial(payload, (gn_id601_tx_ccr_buffer + tx_ccr_count), frame_length);
    }

    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    return drv_tim_generate_serial((uint16_t*)gn_id601_tx_ccr_buffer, tx_ccr_count, ID601_SERIAL_TIMEOUT_MS);
}

static bool id601_fault_receive_command(uint16_t* p_status, id601_crc_func_t p_crc_func)
{
    if (NULL == p_status)
    {
        drv_uart_printf("\r\n    (%s)Invalid status pointer.", __func__);
        return false;
    }

    uint16_t frame_length = ID601_FAULT_RECV_PAYLOAD_BIT_COUNT;
    if (NULL != p_crc_func)
    {
        frame_length += ID601_CRC8_BIT_COUNT;
    }

    uint16_t* p_rise = gn_id601_rx_ccr_rise_buffer;
    uint16_t* p_fall = gn_id601_rx_ccr_fall_buffer;
    if (false == drv_tim_capture_serial(p_rise, p_fall, frame_length, ID601_SERIAL_TIMEOUT_MS))
    {
        return false;
    }
    uint32_t serial_data = 0U;
    if (false == id601_decode_serial_from_ccr_buffer(p_rise, p_fall, &serial_data, ID601_FAULT_RECV_PAYLOAD_BIT_COUNT))
    {
        return false;
    }
    uint16_t command = id601_get_field(serial_data, ID601_FAULT_RECV_COMMAND_Msk, ID601_FAULT_RECV_COMMAND_Pos);
    uint16_t fault = id601_get_field(serial_data, ID601_FAULT_RECV_STATUS_Msk, ID601_FAULT_RECV_STATUS_Pos);
    drv_uart_printf("\r\n    [ID601 Fault Recv] [command: 0x%02X, Fault: 0x%01X]", command, fault);
    *p_status = fault;

    return true;
}

static inline uint32_t id601_build_syncgen_payload(void)
{
    uint32_t payload = 0U;
    payload = id601_set_field(payload, ID601_SYNCGEN_COMMAND_Msk, ID601_SYNCGEN_COMMAND_Pos, ID601_SYNCGEN_COMMAND);
    return payload;
}

static bool id601_syncgen_command(id601_crc_func_t p_crc_func)
{
    uint16_t tx_ccr_count = 0U;
    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        uint32_t payload = id601_build_syncgen_payload();
        uint16_t frame_length = ID601_SYNCGEN_PAYLOAD_BIT_COUNT;

        if (NULL != p_crc_func)
        {
            const uint8_t crc = p_crc_func(payload, frame_length);
            payload = ((payload << ID601_CRC8_BIT_COUNT) | (uint32_t)crc);
            frame_length += ID601_CRC8_BIT_COUNT;
        }
        tx_ccr_count += id601_encode_ccr_buffer_from_serial(payload, (gn_id601_tx_ccr_buffer + tx_ccr_count), frame_length);
    }

    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    return drv_tim_generate_serial((uint16_t*)gn_id601_tx_ccr_buffer, tx_ccr_count, ID601_SERIAL_TIMEOUT_MS);
}

static inline uint32_t id601_build_idgen_payload(void)
{
    uint32_t payload = 0U;
    payload = id601_set_field(payload, ID601_IDGEN_COMMAND_Msk, ID601_IDGEN_COMMAND_Pos, ID601_IDGEN_COMMAND);
    return payload;
}

static bool id601_idgen_command(id601_crc_func_t p_crc_func)
{
    uint16_t tx_ccr_count = 0U;
    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        uint32_t payload = id601_build_idgen_payload();
        uint16_t frame_length = ID601_IDGEN_PAYLOAD_BIT_COUNT;

        if (NULL != p_crc_func)
        {
            const uint8_t crc = p_crc_func(payload, frame_length);
            payload = ((payload << ID601_CRC8_BIT_COUNT) | (uint32_t)crc);
            frame_length += ID601_CRC8_BIT_COUNT;
        }
        tx_ccr_count += id601_encode_ccr_buffer_from_serial(payload, (gn_id601_tx_ccr_buffer + tx_ccr_count), frame_length);
    }

    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    return drv_tim_generate_serial((uint16_t*)gn_id601_tx_ccr_buffer, tx_ccr_count, ID601_SERIAL_TIMEOUT_MS);
}

static bool id601_select_register_bank(id601_register_bank_t bank, id601_crc_func_t p_crc_func)
{
    static id601_register_bank_t id601_register_bank = ID601_REGISTER_BANK_GENERAL;
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_op_mode = gn_id601_general_regs[ID601_ADDR_GENERAL_OP_MODE];

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_op_mode[device_index];
    }

    if (bank != id601_register_bank)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            uint16_t addr_ext = 0U;
            if (ID601_REGISTER_BANK_GENERAL == bank)
            {
                addr_ext = 0U;
            }
            else if (ID601_REGISTER_BANK_MIRROR == bank)
            {
                addr_ext = 1U;
            }
            else
            {
                return false;
            }
            tx_data[device_index] = id601_set_field(p_op_mode[device_index], ID601_OP_MODE_ADDR_EXT_Msk, ID601_OP_MODE_ADDR_EXT_Pos, addr_ext);
        }
        if (false == id601_write_command(ID601_ADDR_GENERAL_OP_MODE, tx_data, p_crc_func))
        {
            return false;
        }

        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            p_op_mode[device_index] = tx_data[device_index];
        }
        id601_register_bank = bank;
    }
    return true;
}

bool id601_write_register(id601_register_bank_t bank, uint16_t addr, const uint16_t* p_value, id601_crc_func_t p_crc_func)
{
    if (false == id601_select_register_bank(bank, p_crc_func))
    {
        return false;
    }
    if (false == id601_write_command(addr, p_value, p_crc_func))
    {
        return false;
    }
    drv_uart_printf("\r\n    (%s)Write register success.", __func__);
    return true;
}

bool id601_read_register(id601_register_bank_t bank, uint16_t addr, uint16_t* p_value, id601_crc_func_t p_crc_func)
{
    if (false == id601_select_register_bank(bank, p_crc_func))
    {
        return false;
    }
    if (false == id601_read_command(addr, p_crc_func))
    {
        return false;
    }
    if (false == id601_read_receive_command(p_value, p_crc_func))
    {
        return false;
    }
    drv_uart_printf("\r\n    (%s)Read register success.", __func__);
    return true;
}

bool id601_reset(void)
{
    uint16_t reset[ID601_DAISY_LENGTH_MAX] = { 0U };
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        reset[device_index] = id601_set_field(reset[device_index], ID601_RESET_ID_RST_Msk, ID601_RESET_ID_RST_Pos, 1U);
    }

    if (false == id601_write_command(ID601_ADDR_GENERAL_OP_MODE, reset, NULL))
    {
        return false;
    }
    drv_tim_delay_ms(ID601_DELAY_RESET_MS);
    return true;
}

static inline uint16_t id601_build_reset_id(uint16_t rst, uint16_t vs_rst, uint16_t e_rst, uint16_t lkg_e, uint16_t id)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_RESET_ID_RST_Msk, ID601_RESET_ID_RST_Pos, (uint16_t)rst);
    register_value = id601_set_field(register_value, ID601_RESET_ID_VS_RST_Msk, ID601_RESET_ID_VS_RST_Pos, (uint16_t)vs_rst);
    register_value = id601_set_field(register_value, ID601_RESET_ID_E_RST_Msk, ID601_RESET_ID_E_RST_Pos, (uint16_t)e_rst);
    register_value = id601_set_field(register_value, ID601_RESET_ID_LKG_E_Msk, ID601_RESET_ID_LKG_E_Pos, (uint16_t)lkg_e);
    register_value = id601_set_field(register_value, ID601_RESET_ID_ID_Msk, ID601_RESET_ID_ID_Pos, (uint16_t)id);
    return register_value;
}

bool id601_write_reset_id(uint16_t device_select, uint16_t rst, uint16_t vs_rst, uint16_t e_rst, uint16_t lkg_e, uint16_t id)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_reset_id = gn_id601_general_regs[ID601_ADDR_GENERAL_RESET_ID];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_reset_id[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_reset_id(rst, vs_rst, e_rst, lkg_e, id);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_reset_id(rst, vs_rst, e_rst, lkg_e, id);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_RESET_ID, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_reset_id[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_ld_control(uint16_t sv_no, uint16_t ld_type, uint16_t delay_ch_en, uint16_t syncmode, uint16_t pwm_res, uint16_t ld_dir, uint16_t ld_mode)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_LD_CONTROL_SV_NO_Msk, ID601_LD_CONTROL_SV_NO_Pos, (uint16_t)sv_no);
    register_value = id601_set_field(register_value, ID601_LD_CONTROL_LD_TYPE_Msk, ID601_LD_CONTROL_LD_TYPE_Pos, (uint16_t)ld_type);
    register_value = id601_set_field(register_value, ID601_LD_CONTROL_DELAY_CH_EN_Msk, ID601_LD_CONTROL_DELAY_CH_EN_Pos, (uint16_t)delay_ch_en);
    register_value = id601_set_field(register_value, ID601_LD_CONTROL_SYNCMODE_Msk, ID601_LD_CONTROL_SYNCMODE_Pos, (uint16_t)syncmode);
    register_value = id601_set_field(register_value, ID601_LD_CONTROL_PWM_RES_Msk, ID601_LD_CONTROL_PWM_RES_Pos, (uint16_t)pwm_res);
    register_value = id601_set_field(register_value, ID601_LD_CONTROL_LD_DIR_Msk, ID601_LD_CONTROL_LD_DIR_Pos, (uint16_t)ld_dir);
    register_value = id601_set_field(register_value, ID601_LD_CONTROL_LD_MODE_Msk, ID601_LD_CONTROL_LD_MODE_Pos, (uint16_t)ld_mode);
    return register_value;
}

bool id601_write_ld_control(uint16_t device_select, uint16_t sv_no, uint16_t ld_type, uint16_t delay_ch_en, uint16_t syncmode, uint16_t pwm_res, uint16_t ld_dir, uint16_t ld_mode)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_ld_control = gn_id601_general_regs[ID601_ADDR_GENERAL_LD_CONTROL];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_ld_control[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_ld_control(sv_no, ld_type, delay_ch_en, syncmode, pwm_res, ld_dir, ld_mode);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_ld_control(sv_no, ld_type, delay_ch_en, syncmode, pwm_res, ld_dir, ld_mode);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_LD_CONTROL, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_ld_control[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_ld_size(uint16_t ld_size)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_LD_SIZE_LD_SIZE_Msk, ID601_LD_SIZE_LD_SIZE_Pos, (uint16_t)ld_size);
    return register_value;
}

bool id601_write_ld_size(uint16_t device_select, uint16_t ld_size)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_ld_size = gn_id601_general_regs[ID601_ADDR_GENERAL_LD_SIZE];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_ld_size[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_ld_size(ld_size);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_ld_size(ld_size);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_LD_SIZE, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_ld_size[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_pwmclk_div_1_2(uint16_t fpwm_div_2, uint16_t fpwm_div_1)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_PWMCLK_DIV1_2_FPWM_DIV2_Msk, ID601_PWMCLK_DIV1_2_FPWM_DIV2_Pos, (uint16_t)fpwm_div_2);
    register_value = id601_set_field(register_value, ID601_PWMCLK_DIV1_2_FPWM_DIV1_Msk, ID601_PWMCLK_DIV1_2_FPWM_DIV1_Pos, (uint16_t)fpwm_div_1);
    return register_value;
}

bool id601_write_pwmclk_div_1_2(uint16_t device_select, uint16_t fpwm_div_2, uint16_t fpwm_div_1)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_pwmclk_div_1_2 = gn_id601_general_regs[ID601_ADDR_GENERAL_PWMCLK_DIV1_2];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_pwmclk_div_1_2[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_pwmclk_div_1_2(fpwm_div_2, fpwm_div_1);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_pwmclk_div_1_2(fpwm_div_2, fpwm_div_1);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_PWMCLK_DIV1_2, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_pwmclk_div_1_2[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_pwmclk_div_2_3(uint16_t fpwm_div_3, uint16_t fpwm_div_2)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_PWMCLK_DIV2_3_FPWM_DIV3_Msk, ID601_PWMCLK_DIV2_3_FPWM_DIV3_Pos, (uint16_t)fpwm_div_3);
    register_value = id601_set_field(register_value, ID601_PWMCLK_DIV2_3_FPWM_DIV2_Msk, ID601_PWMCLK_DIV2_3_FPWM_DIV2_Pos, (uint16_t)fpwm_div_2);
    return register_value;
}

bool id601_write_pwmclk_div_2_3(uint16_t device_select, uint16_t fpwm_div_3, uint16_t fpwm_div_2)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_pwmclk_div_2_3 = gn_id601_general_regs[ID601_ADDR_GENERAL_PWMCLK_DIV2_3];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_pwmclk_div_2_3[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_pwmclk_div_2_3(fpwm_div_3, fpwm_div_2);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_pwmclk_div_2_3(fpwm_div_3, fpwm_div_2);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_PWMCLK_DIV2_3, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_pwmclk_div_2_3[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_channel_enable(uint16_t ch12_en, uint16_t ch11_en, uint16_t ch10_en, uint16_t ch9_en, uint16_t ch8_en, uint16_t ch7_en, uint16_t ch6_en, uint16_t ch5_en, uint16_t ch4_en, uint16_t ch3_en, uint16_t ch2_en, uint16_t ch1_en)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH12_EN_Msk, ID601_CHANNEL_ENABLE_CH12_EN_Pos, (uint16_t)ch12_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH11_EN_Msk, ID601_CHANNEL_ENABLE_CH11_EN_Pos, (uint16_t)ch11_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH10_EN_Msk, ID601_CHANNEL_ENABLE_CH10_EN_Pos, (uint16_t)ch10_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH9_EN_Msk, ID601_CHANNEL_ENABLE_CH9_EN_Pos, (uint16_t)ch9_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH8_EN_Msk, ID601_CHANNEL_ENABLE_CH8_EN_Pos, (uint16_t)ch8_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH7_EN_Msk, ID601_CHANNEL_ENABLE_CH7_EN_Pos, (uint16_t)ch7_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH6_EN_Msk, ID601_CHANNEL_ENABLE_CH6_EN_Pos, (uint16_t)ch6_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH5_EN_Msk, ID601_CHANNEL_ENABLE_CH5_EN_Pos, (uint16_t)ch5_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH4_EN_Msk, ID601_CHANNEL_ENABLE_CH4_EN_Pos, (uint16_t)ch4_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH3_EN_Msk, ID601_CHANNEL_ENABLE_CH3_EN_Pos, (uint16_t)ch3_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH2_EN_Msk, ID601_CHANNEL_ENABLE_CH2_EN_Pos, (uint16_t)ch2_en);
    register_value = id601_set_field(register_value, ID601_CHANNEL_ENABLE_CH1_EN_Msk, ID601_CHANNEL_ENABLE_CH1_EN_Pos, (uint16_t)ch1_en);
    return register_value;
}

bool id601_write_channel_enable(uint16_t device_select, uint16_t ch12_en, uint16_t ch11_en, uint16_t ch10_en, uint16_t ch9_en, uint16_t ch8_en, uint16_t ch7_en, uint16_t ch6_en, uint16_t ch5_en, uint16_t ch4_en, uint16_t ch3_en, uint16_t ch2_en, uint16_t ch1_en)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_channel_enable = gn_id601_general_regs[ID601_ADDR_GENERAL_CHANNEL_ENABLE];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_channel_enable[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_channel_enable(ch12_en, ch11_en, ch10_en, ch9_en, ch8_en, ch7_en, ch6_en, ch5_en, ch4_en, ch3_en, ch2_en, ch1_en);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_channel_enable(ch12_en, ch11_en, ch10_en, ch9_en, ch8_en, ch7_en, ch6_en, ch5_en, ch4_en, ch3_en, ch2_en, ch1_en);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_CHANNEL_ENABLE, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_channel_enable[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_fault_control(uint16_t ft_mode, uint16_t fb_mode, uint16_t o_fb_e, uint16_t t_det_e, uint16_t s_det_e, uint16_t o_det_e, uint16_t t_off_e, uint16_t s_off_e, uint16_t o_off_e)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_FT_MODE_Msk, ID601_FAULT_CONTROL_FT_MODE_Pos, (uint16_t)ft_mode);
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_FB_MODE_Msk, ID601_FAULT_CONTROL_FB_MODE_Pos, (uint16_t)fb_mode);
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_O_FB_E_Msk, ID601_FAULT_CONTROL_O_FB_E_Pos, (uint16_t)o_fb_e);
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_T_DET_E_Msk, ID601_FAULT_CONTROL_T_DET_E_Pos, (uint16_t)t_det_e);
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_S_DET_E_Msk, ID601_FAULT_CONTROL_S_DET_E_Pos, (uint16_t)s_det_e);
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_O_DET_E_Msk, ID601_FAULT_CONTROL_O_DET_E_Pos, (uint16_t)o_det_e);
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_T_OFF_E_Msk, ID601_FAULT_CONTROL_T_OFF_E_Pos, (uint16_t)t_off_e);
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_S_OFF_E_Msk, ID601_FAULT_CONTROL_S_OFF_E_Pos, (uint16_t)s_off_e);
    register_value = id601_set_field(register_value, ID601_FAULT_CONTROL_O_OFF_E_Msk, ID601_FAULT_CONTROL_O_OFF_E_Pos, (uint16_t)o_off_e);
    return register_value;
}

bool id601_write_fault_control(uint16_t device_select, uint16_t ft_mode, uint16_t fb_mode, uint16_t o_fb_e, uint16_t t_det_e, uint16_t s_det_e, uint16_t o_det_e, uint16_t t_off_e, uint16_t s_off_e, uint16_t o_off_e)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_fault_control = gn_id601_general_regs[ID601_ADDR_GENERAL_FAULT_CONTROL];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_fault_control[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_fault_control(ft_mode, fb_mode, o_fb_e, t_det_e, s_det_e, o_det_e, t_off_e, s_off_e, o_off_e);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_fault_control(ft_mode, fb_mode, o_fb_e, t_det_e, s_det_e, o_det_e, t_off_e, s_off_e, o_off_e);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_FAULT_CONTROL, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_fault_control[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_fb_level(uint16_t fb3_level, uint16_t fb2_level, uint16_t fb1_level)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_FB_LEVEL_FB3_LEVEL_Msk, ID601_FB_LEVEL_FB3_LEVEL_Pos, (uint16_t)fb3_level);
    register_value = id601_set_field(register_value, ID601_FB_LEVEL_FB2_LEVEL_Msk, ID601_FB_LEVEL_FB2_LEVEL_Pos, (uint16_t)fb2_level);
    register_value = id601_set_field(register_value, ID601_FB_LEVEL_FB1_LEVEL_Msk, ID601_FB_LEVEL_FB1_LEVEL_Pos, (uint16_t)fb1_level);
    return register_value;
}

bool id601_write_fb_level(uint16_t device_select, uint16_t fb3_level, uint16_t fb2_level, uint16_t fb1_level)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_fb_level = gn_id601_general_regs[ID601_ADDR_GENERAL_FB_LEVEL];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_fb_level[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_fb_level(fb3_level, fb2_level, fb1_level);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_fb_level(fb3_level, fb2_level, fb1_level);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_FB_LEVEL, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_fb_level[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_short_level(uint16_t short3_level, uint16_t short2_level, uint16_t short1_level)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_SHORT_LEVEL_SHORT3_LEVEL_Msk, ID601_SHORT_LEVEL_SHORT3_LEVEL_Pos, (uint16_t)short3_level);
    register_value = id601_set_field(register_value, ID601_SHORT_LEVEL_SHORT2_LEVEL_Msk, ID601_SHORT_LEVEL_SHORT2_LEVEL_Pos, (uint16_t)short2_level);
    register_value = id601_set_field(register_value, ID601_SHORT_LEVEL_SHORT1_LEVEL_Msk, ID601_SHORT_LEVEL_SHORT1_LEVEL_Pos, (uint16_t)short1_level);
    return register_value;
}

bool id601_write_short_level(uint16_t device_select, uint16_t short3_level, uint16_t short2_level, uint16_t short1_level)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_short_level = gn_id601_general_regs[ID601_ADDR_GENERAL_SHORT_LEVEL];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_short_level[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_short_level(short3_level, short2_level, short1_level);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_short_level(short3_level, short2_level, short1_level);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_SHORT_LEVEL, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_short_level[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_fault_status0(void)
{
    return 0U; // read only
}

bool id601_write_fault_status0(void)
{
    id601_build_fault_status0();
    return false; // read only
}

static inline uint16_t id601_build_fault_status1(void)
{
    return 0U; // read only
}

bool id601_write_fault_status1(void)
{
    id601_build_fault_status1();
    return false; // read only
}

static inline uint16_t id601_build_fault_status2(void)
{
    return 0U; // read only
}

bool id601_write_fault_status2(void)
{
    id601_build_fault_status2();
    return false; // read only
}

static inline uint16_t id601_build_fault_status3(void)
{
    return 0U; // read only
}

bool id601_write_fault_status3(void)
{
    id601_build_fault_status3();
    return false; // read only
}

static inline uint16_t id601_build_max_current_level(uint16_t max_curr_level3, uint16_t max_curr_level2, uint16_t max_curr_level1)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL3_Msk, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL3_Pos, (uint16_t)max_curr_level3);
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL2_Msk, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL2_Pos, (uint16_t)max_curr_level2);
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL1_Msk, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL1_Pos, (uint16_t)max_curr_level1);
    return register_value;
}

bool id601_write_max_current_level(uint16_t device_select, uint16_t max_curr_level3, uint16_t max_curr_level2, uint16_t max_curr_level1)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_max_current_level = gn_id601_general_regs[ID601_ADDR_GENERAL_MAX_CURR_LEVEL];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_max_current_level[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_max_current_level(max_curr_level3, max_curr_level2, max_curr_level1);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_max_current_level(max_curr_level3, max_curr_level2, max_curr_level1);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_MAX_CURR_LEVEL, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_max_current_level[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_max_curr_vref1(uint16_t max_curr_vref1)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_VREF1_MAX_CURR_VREF1_Msk, ID601_MAX_CURRENT_VREF1_MAX_CURR_VREF1_Pos, (uint16_t)max_curr_vref1);
    return register_value;
}

bool id601_write_max_curr_vref1(uint16_t device_select, uint16_t max_curr_vref1)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_max_curr_vref1 = gn_id601_general_regs[ID601_ADDR_GENERAL_MAX_CURR_VREF1];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_max_curr_vref1[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_max_curr_vref1(max_curr_vref1);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_max_curr_vref1(max_curr_vref1);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_MAX_CURR_VREF1, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_max_curr_vref1[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_max_curr_vref2(uint16_t max_curr_vref2)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_VREF2_MAX_CURR_VREF2_Msk, ID601_MAX_CURRENT_VREF2_MAX_CURR_VREF2_Pos, (uint16_t)max_curr_vref2);
    return register_value;
}

bool id601_write_max_curr_vref2(uint16_t device_select, uint16_t max_curr_vref2)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_max_curr_vref2 = gn_id601_general_regs[ID601_ADDR_GENERAL_MAX_CURR_VREF2];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_max_curr_vref2[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_max_curr_vref2(max_curr_vref2);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_max_curr_vref2(max_curr_vref2);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_MAX_CURR_VREF2, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_max_curr_vref2[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_max_curr_vref3(uint16_t max_curr_vref3)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_VREF3_MAX_CURR_VREF3_Msk, ID601_MAX_CURRENT_VREF3_MAX_CURR_VREF3_Pos, (uint16_t)max_curr_vref3);
    return register_value;
}

bool id601_write_max_curr_vref3(uint16_t device_select, uint16_t max_curr_vref3)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_max_curr_vref3 = gn_id601_general_regs[ID601_ADDR_GENERAL_MAX_CURR_VREF3];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_max_curr_vref3[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_max_curr_vref3(max_curr_vref3);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_max_curr_vref3(max_curr_vref3);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_MAX_CURR_VREF3, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_max_curr_vref3[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_ch_ld_type(uint16_t ch12_ld_type, uint16_t ch11_ld_type, uint16_t ch10_ld_type, uint16_t ch9_ld_type, uint16_t ch8_ld_type, uint16_t ch7_ld_type,
                                                uint16_t ch6_ld_type, uint16_t ch5_ld_type, uint16_t ch4_ld_type, uint16_t ch3_ld_type, uint16_t ch2_ld_type, uint16_t ch1_ld_type)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH12_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH12_LD_TYPE_Pos, (uint16_t)ch12_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH11_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH11_LD_TYPE_Pos, (uint16_t)ch11_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH10_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH10_LD_TYPE_Pos, (uint16_t)ch10_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH9_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH9_LD_TYPE_Pos, (uint16_t)ch9_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH8_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH8_LD_TYPE_Pos, (uint16_t)ch8_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH7_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH7_LD_TYPE_Pos, (uint16_t)ch7_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH6_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH6_LD_TYPE_Pos, (uint16_t)ch6_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH5_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH5_LD_TYPE_Pos, (uint16_t)ch5_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH4_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH4_LD_TYPE_Pos, (uint16_t)ch4_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH3_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH3_LD_TYPE_Pos, (uint16_t)ch3_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH2_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH2_LD_TYPE_Pos, (uint16_t)ch2_ld_type);
    register_value = id601_set_field(register_value, ID601_CH_LD_TYPE_CH1_LD_TYPE_Msk, ID601_CH_LD_TYPE_CH1_LD_TYPE_Pos, (uint16_t)ch1_ld_type);
    return register_value;
}

bool id601_write_ch_ld_type(uint16_t device_select, uint16_t ch12_ld_type, uint16_t ch11_ld_type, uint16_t ch10_ld_type, uint16_t ch9_ld_type, uint16_t ch8_ld_type, uint16_t ch7_ld_type,
                                                        uint16_t ch6_ld_type, uint16_t ch5_ld_type, uint16_t ch4_ld_type, uint16_t ch3_ld_type, uint16_t ch2_ld_type, uint16_t ch1_ld_type)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_ch_ld_type = gn_id601_general_regs[ID601_ADDR_GENERAL_CH_LD_TYPE];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_ch_ld_type[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_ch_ld_type(ch12_ld_type, ch11_ld_type, ch10_ld_type, ch9_ld_type, ch8_ld_type, ch7_ld_type,
                                                            ch6_ld_type, ch5_ld_type, ch4_ld_type, ch3_ld_type, ch2_ld_type, ch1_ld_type);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_ch_ld_type(ch12_ld_type, ch11_ld_type, ch10_ld_type, ch9_ld_type, ch8_ld_type, ch7_ld_type,
                                                        ch6_ld_type, ch5_ld_type, ch4_ld_type, ch3_ld_type, ch2_ld_type, ch1_ld_type);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_CH_LD_TYPE, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_ch_ld_type[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_delay_ch1_2(uint16_t delay_ch2, uint16_t delay_ch1)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH1_2_DELAY_CH2_Msk, ID601_DELAY_CH1_2_DELAY_CH2_Pos, (uint16_t)delay_ch2);
    register_value = id601_set_field(register_value, ID601_DELAY_CH1_2_DELAY_CH1_Msk, ID601_DELAY_CH1_2_DELAY_CH1_Pos, (uint16_t)delay_ch1);
    return register_value;
}

bool id601_write_delay_ch1_2(uint16_t device_select, uint16_t delay_ch2, uint16_t delay_ch1)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_delay_ch1_2 = gn_id601_general_regs[ID601_ADDR_GENERAL_DELAY_CH1_2];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_delay_ch1_2[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_delay_ch1_2(delay_ch2, delay_ch1);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_delay_ch1_2(delay_ch2, delay_ch1);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_DELAY_CH1_2, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_delay_ch1_2[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_delay_ch3_4(uint16_t delay_ch4, uint16_t delay_ch3)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH3_4_DELAY_CH4_Msk, ID601_DELAY_CH3_4_DELAY_CH4_Pos, (uint16_t)delay_ch4);
    register_value = id601_set_field(register_value, ID601_DELAY_CH3_4_DELAY_CH3_Msk, ID601_DELAY_CH3_4_DELAY_CH3_Pos, (uint16_t)delay_ch3);
    return register_value;
}

bool id601_write_delay_ch3_4(uint16_t device_select, uint16_t delay_ch4, uint16_t delay_ch3)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_delay_ch3_4 = gn_id601_general_regs[ID601_ADDR_GENERAL_DELAY_CH3_4];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_delay_ch3_4[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_delay_ch3_4(delay_ch4, delay_ch3);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_delay_ch3_4(delay_ch4, delay_ch3);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_DELAY_CH3_4, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_delay_ch3_4[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_delay_ch5_6(uint16_t delay_ch6, uint16_t delay_ch5)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH5_6_DELAY_CH6_Msk, ID601_DELAY_CH5_6_DELAY_CH6_Pos, (uint16_t)delay_ch6);
    register_value = id601_set_field(register_value, ID601_DELAY_CH5_6_DELAY_CH5_Msk, ID601_DELAY_CH5_6_DELAY_CH5_Pos, (uint16_t)delay_ch5);
    return register_value;
}

bool id601_write_delay_ch5_6(uint16_t device_select, uint16_t delay_ch6, uint16_t delay_ch5)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_delay_ch5_6 = gn_id601_general_regs[ID601_ADDR_GENERAL_DELAY_CH5_6];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_delay_ch5_6[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_delay_ch5_6(delay_ch6, delay_ch5);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_delay_ch5_6(delay_ch6, delay_ch5);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_DELAY_CH5_6, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_delay_ch5_6[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_delay_ch7_8(uint16_t delay_ch8, uint16_t delay_ch7)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH7_8_DELAY_CH8_Msk, ID601_DELAY_CH7_8_DELAY_CH8_Pos, (uint16_t)delay_ch8);
    register_value = id601_set_field(register_value, ID601_DELAY_CH7_8_DELAY_CH7_Msk, ID601_DELAY_CH7_8_DELAY_CH7_Pos, (uint16_t)delay_ch7);
    return register_value;
}

bool id601_write_delay_ch7_8(uint16_t device_select, uint16_t delay_ch8, uint16_t delay_ch7)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_delay_ch7_8 = gn_id601_general_regs[ID601_ADDR_GENERAL_DELAY_CH7_8];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_delay_ch7_8[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_delay_ch7_8(delay_ch8, delay_ch7);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_delay_ch7_8(delay_ch8, delay_ch7);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_DELAY_CH7_8, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_delay_ch7_8[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_delay_ch9_10(uint16_t delay_ch10, uint16_t delay_ch9)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH9_10_DELAY_CH10_Msk, ID601_DELAY_CH9_10_DELAY_CH10_Pos, (uint16_t)delay_ch10);
    register_value = id601_set_field(register_value, ID601_DELAY_CH9_10_DELAY_CH9_Msk, ID601_DELAY_CH9_10_DELAY_CH9_Pos, (uint16_t)delay_ch9);
    return register_value;
}

bool id601_write_delay_ch9_10(uint16_t device_select, uint16_t delay_ch10, uint16_t delay_ch9)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_delay_ch9_10 = gn_id601_general_regs[ID601_ADDR_GENERAL_DELAY_CH9_10];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_delay_ch9_10[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_delay_ch9_10(delay_ch10, delay_ch9);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_delay_ch9_10(delay_ch10, delay_ch9);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_DELAY_CH9_10, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_delay_ch9_10[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_delay_ch11_12(uint16_t delay_ch12, uint16_t delay_ch11)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH11_12_DELAY_CH12_Msk, ID601_DELAY_CH11_12_DELAY_CH12_Pos, (uint16_t)delay_ch12);
    register_value = id601_set_field(register_value, ID601_DELAY_CH11_12_DELAY_CH11_Msk, ID601_DELAY_CH11_12_DELAY_CH11_Pos, (uint16_t)delay_ch11);
    return register_value;
}

bool id601_write_delay_ch11_12(uint16_t device_select, uint16_t delay_ch12, uint16_t delay_ch11)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_delay_ch11_12 = gn_id601_general_regs[ID601_ADDR_GENERAL_DELAY_CH11_12];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_delay_ch11_12[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_delay_ch11_12(delay_ch12, delay_ch11);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_delay_ch11_12(delay_ch12, delay_ch11);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_DELAY_CH11_12, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_delay_ch11_12[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_fault_configuration(uint16_t crc_en, uint16_t wd_dimm_en, uint16_t wd_timecfg, uint16_t wd_en, uint16_t vs_miss_dimm_en, uint16_t vs_miss_en, uint16_t ch_ctrl_mismatch_en, \
        uint16_t vref_ovuv_off_en, uint16_t vref_ovuv_en, uint16_t osc_ldo_ovuv_en, uint16_t ldo_ovuv_en)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_CRC_EN_Msk, ID601_FAULT_CONFIGURATION_CRC_EN_Pos, (uint16_t)crc_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_WD_DIMM_EN_Msk, ID601_FAULT_CONFIGURATION_WD_DIMM_EN_Pos, (uint16_t)wd_dimm_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_WD_TIMECFG_Msk, ID601_FAULT_CONFIGURATION_WD_TIMECFG_Pos, (uint16_t)wd_timecfg);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_WD_EN_Msk, ID601_FAULT_CONFIGURATION_WD_EN_Pos, (uint16_t)wd_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_VS_MISS_DIMM_EN_Msk, ID601_FAULT_CONFIGURATION_VS_MISS_DIMM_EN_Pos, (uint16_t)vs_miss_dimm_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_VS_MISS_EN_Msk, ID601_FAULT_CONFIGURATION_VS_MISS_EN_Pos, (uint16_t)vs_miss_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_CH_CTRL_MISMATCH_EN_Msk, ID601_FAULT_CONFIGURATION_CH_CTRL_MISMATCH_EN_Pos, (uint16_t)ch_ctrl_mismatch_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_VREF_OVUV_OFF_EN_Msk, ID601_FAULT_CONFIGURATION_VREF_OVUV_OFF_EN_Pos, (uint16_t)vref_ovuv_off_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_VREF_OVUV_EN_Msk, ID601_FAULT_CONFIGURATION_VREF_OVUV_EN_Pos, (uint16_t)vref_ovuv_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_OSC_LDO_OVUV_EN_Msk, ID601_FAULT_CONFIGURATION_OSC_LDO_OVUV_EN_Pos, (uint16_t)osc_ldo_ovuv_en);
    register_value = id601_set_field(register_value, ID601_FAULT_CONFIGURATION_LDO_OVUV_EN_Msk, ID601_FAULT_CONFIGURATION_LDO_OVUV_EN_Pos, (uint16_t)ldo_ovuv_en);
    return register_value;
}

bool id601_write_fault_configuration(uint16_t device_select, uint16_t crc_en, uint16_t wd_dimm_en, uint16_t wd_timecfg, uint16_t wd_en, uint16_t vs_miss_dimm_en, uint16_t vs_miss_en, uint16_t ch_ctrl_mismatch_en, \
        uint16_t vref_ovuv_off_en, uint16_t vref_ovuv_en, uint16_t osc_ldo_ovuv_en, uint16_t ldo_ovuv_en)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_fault_configuration = gn_id601_general_regs[ID601_ADDR_GENERAL_FAULT_CONFIGURATION];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_fault_configuration[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_fault_configuration(crc_en, wd_dimm_en, wd_timecfg, wd_en, vs_miss_dimm_en, vs_miss_en, ch_ctrl_mismatch_en, vref_ovuv_off_en, vref_ovuv_en, osc_ldo_ovuv_en, ldo_ovuv_en);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_fault_configuration(crc_en, wd_dimm_en, wd_timecfg, wd_en, vs_miss_dimm_en, vs_miss_en, ch_ctrl_mismatch_en, vref_ovuv_off_en, vref_ovuv_en, osc_ldo_ovuv_en, ldo_ovuv_en);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_FAULT_CONFIGURATION, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_fault_configuration[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_bist(uint16_t bist_start)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_BIST_BIST_START_Msk, ID601_BIST_BIST_START_Pos, (uint16_t)bist_start);
    return register_value;
}

bool id601_write_bist(uint16_t device_select, uint16_t bist_start)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_bist = gn_id601_general_regs[ID601_ADDR_GENERAL_BIST];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_bist[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_bist(bist_start);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_bist(bist_start);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_BIST, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_bist[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_test_mode0(uint16_t dmux_sel)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TEST_MODE0_DMUX_SEL_Msk, ID601_TEST_MODE0_DMUX_SEL_Pos, (uint16_t)dmux_sel);
    return register_value;
}

bool id601_write_test_mode0(uint16_t device_select, uint16_t dmux_sel)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_test_mode0 = gn_id601_general_regs[ID601_ADDR_GENERAL_TEST_MODE0];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_test_mode0[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_test_mode0(dmux_sel);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_test_mode0(dmux_sel);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_TEST_MODE0, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_test_mode0[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_test_mode1(uint16_t testmode, uint16_t tm_ch_ctrl_input_flip)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TEST_MODE1_TESTMODE_Msk, ID601_TEST_MODE1_TESTMODE_Pos, (uint16_t)testmode);
    register_value = id601_set_field(register_value, ID601_TEST_MODE1_TM_CH_CTRL_INPUT_FLIP_Msk, ID601_TEST_MODE1_TM_CH_CTRL_INPUT_FLIP_Pos, (uint16_t)tm_ch_ctrl_input_flip);
    return register_value;
}

bool id601_write_test_mode1(uint16_t device_select, uint16_t testmode, uint16_t tm_ch_ctrl_input_flip)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_test_mode1 = gn_id601_general_regs[ID601_ADDR_GENERAL_TEST_MODE1];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_test_mode1[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_test_mode1(testmode, tm_ch_ctrl_input_flip);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_test_mode1(testmode, tm_ch_ctrl_input_flip);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_TEST_MODE1, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_test_mode1[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_serial_clk_gen(uint16_t serial_clk_low, uint16_t serial_clk_high)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_SERIAL_CLK_GEN_SERIAL_CLK_LOW_Msk, ID601_SERIAL_CLK_GEN_SERIAL_CLK_LOW_Pos, (uint16_t)serial_clk_low);
    register_value = id601_set_field(register_value, ID601_SERIAL_CLK_GEN_SERIAL_CLK_HIGH_Msk, ID601_SERIAL_CLK_GEN_SERIAL_CLK_HIGH_Pos, (uint16_t)serial_clk_high);
    return register_value;
}

bool id601_write_serial_clk_gen(uint16_t device_select, uint16_t serial_clk_low, uint16_t serial_clk_high)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_serial_clk_gen = gn_id601_general_regs[ID601_ADDR_GENERAL_SERIAL_CLK_GEN];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_serial_clk_gen[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_serial_clk_gen(serial_clk_low, serial_clk_high);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_serial_clk_gen(serial_clk_low, serial_clk_high);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_SERIAL_CLK_GEN, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_serial_clk_gen[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_serial_latency(uint16_t serial_latency)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_SERIAL_LATENCY_SERIAL_LATENCY_Msk, ID601_SERIAL_LATENCY_SERIAL_LATENCY_Pos, (uint16_t)serial_latency);
    return register_value;
}

bool id601_write_serial_latency(uint16_t device_select, uint16_t serial_latency)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_serial_latency = gn_id601_general_regs[ID601_ADDR_GENERAL_SERIAL_LATENCY];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_serial_latency[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_serial_latency(serial_latency);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_serial_latency(serial_latency);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_SERIAL_LATENCY, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_serial_latency[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_v_mask(uint16_t v_mask)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_V_MASK_V_MASK_Msk, ID601_V_MASK_V_MASK_Pos, (uint16_t)v_mask);
    return register_value;
}

bool id601_write_v_mask(uint16_t device_select, uint16_t v_mask)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_v_mask = gn_id601_general_regs[ID601_ADDR_GENERAL_V_MASK];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_v_mask[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_v_mask(v_mask);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_v_mask(v_mask);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_V_MASK, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_v_mask[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_sv_mask(uint16_t sv_mask_en, uint16_t sv_mask)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_SV_MASK_SV_MASK_EN_Msk, ID601_SV_MASK_SV_MASK_EN_Pos, (uint16_t)sv_mask_en);
    register_value = id601_set_field(register_value, ID601_SV_MASK_SV_MASK_Msk, ID601_SV_MASK_SV_MASK_Pos, (uint16_t)sv_mask);
    return register_value;
}

bool id601_write_sv_mask(uint16_t device_select, uint16_t sv_mask_en, uint16_t sv_mask)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_sv_mask = gn_id601_general_regs[ID601_ADDR_GENERAL_SV_MASK];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_sv_mask[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_sv_mask(sv_mask_en, sv_mask);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_sv_mask(sv_mask_en, sv_mask);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_SV_MASK, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_sv_mask[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_rstcnt(uint16_t rstcnt)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_RSTCNT_RSTCNT_Msk, ID601_RSTCNT_RSTCNT_Pos, (uint16_t)rstcnt);
    return register_value;
}

bool id601_write_rstcnt(uint16_t device_select, uint16_t rstcnt)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_rstcnt = gn_id601_general_regs[ID601_ADDR_GENERAL_RSTCNT];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_rstcnt[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_rstcnt(rstcnt);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_rstcnt(rstcnt);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_RSTCNT, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_rstcnt[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_timeout(uint16_t timeout)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TIMEOUT_TIMEOUT_Msk, ID601_TIMEOUT_TIMEOUT_Pos, (uint16_t)timeout);
    return register_value;
}

bool id601_write_timeout(uint16_t device_select, uint16_t timeout)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_timeout = gn_id601_general_regs[ID601_ADDR_GENERAL_TIMEOUT];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_timeout[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_timeout(timeout);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_timeout(timeout);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_TIMEOUT, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_timeout[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_fllcnt1(uint16_t fllcnt)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_FLLCNT1_FLLCNT_Msk, ID601_FLLCNT1_FLLCNT_Pos, (uint16_t)fllcnt);
    return register_value;
}

bool id601_write_fllcnt1(uint16_t device_select, uint16_t fllcnt)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_fllcnt1 = gn_id601_general_regs[ID601_ADDR_GENERAL_FLLCNT1];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_fllcnt1[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_fllcnt1(fllcnt);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_fllcnt1(fllcnt);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_FLLCNT1, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_fllcnt1[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_fllcnt2(uint16_t fll_en, uint16_t fll_range, uint16_t fllcnt)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_FLLCNT2_FLL_EN_Msk, ID601_FLLCNT2_FLL_EN_Pos, (uint16_t)fll_en);
    register_value = id601_set_field(register_value, ID601_FLLCNT2_FLL_RANGE_Msk, ID601_FLLCNT2_FLL_RANGE_Pos, (uint16_t)fll_range);
    register_value = id601_set_field(register_value, ID601_FLLCNT2_FLLCNT_Msk, ID601_FLLCNT2_FLLCNT_Pos, (uint16_t)fllcnt);
    return register_value;
}

bool id601_write_fllcnt2(uint16_t device_select, uint16_t fll_en, uint16_t fll_range, uint16_t fllcnt)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_fllcnt2 = gn_id601_general_regs[ID601_ADDR_GENERAL_FLLCNT2];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_fllcnt2[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_fllcnt2(fll_en, fll_range, fllcnt);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_fllcnt2(fll_en, fll_range, fllcnt);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_FLLCNT2, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_fllcnt2[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_wr_protect(uint16_t wr_protect)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_WR_PROTECT_WR_PROTECT_Msk, ID601_WR_PROTECT_WR_PROTECT_Pos, (uint16_t)wr_protect);
    return register_value;
}

bool id601_write_wr_protect(uint16_t device_select, uint16_t wr_protect)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_wr_protect = gn_id601_general_regs[ID601_ADDR_GENERAL_WR_PROTECT];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_wr_protect[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_wr_protect(wr_protect);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_wr_protect(wr_protect);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_WR_PROTECT, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_wr_protect[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_nf_control(uint16_t bbkn_th, uint16_t o_emi_rej_en, uint16_t sgrjt_en2, uint16_t sgrjt_en1, uint16_t bbkn_en, uint16_t dgrjt_en2, uint16_t dgrjt_en1)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_NF_CONTROL_BBKN_TH_Msk, ID601_NF_CONTROL_BBKN_TH_Pos, (uint16_t)bbkn_th);
    register_value = id601_set_field(register_value, ID601_NF_CONTROL_O_EMI_REJ_EN_Msk, ID601_NF_CONTROL_O_EMI_REJ_EN_Pos, (uint16_t)o_emi_rej_en);
    register_value = id601_set_field(register_value, ID601_NF_CONTROL_SGRJT_EN2_Msk, ID601_NF_CONTROL_SGRJT_EN2_Pos, (uint16_t)sgrjt_en2);
    register_value = id601_set_field(register_value, ID601_NF_CONTROL_SGRJT_EN1_Msk, ID601_NF_CONTROL_SGRJT_EN1_Pos, (uint16_t)sgrjt_en1);
    register_value = id601_set_field(register_value, ID601_NF_CONTROL_BBKN_EN_Msk, ID601_NF_CONTROL_BBKN_EN_Pos, (uint16_t)bbkn_en);
    register_value = id601_set_field(register_value, ID601_NF_CONTROL_DGRJT_EN2_Msk, ID601_NF_CONTROL_DGRJT_EN2_Pos, (uint16_t)dgrjt_en2);
    register_value = id601_set_field(register_value, ID601_NF_CONTROL_DGRJT_EN1_Msk, ID601_NF_CONTROL_DGRJT_EN1_Pos, (uint16_t)dgrjt_en1);
    return register_value;
}

bool id601_write_nf_control(uint16_t device_select, uint16_t o_emi_rej_en, uint16_t sgrjt_en2, uint16_t sgrjt_en1, uint16_t bbkn_en, uint16_t dgrjt_en2, uint16_t dgrjt_en1)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_nf_control = gn_id601_general_regs[ID601_ADDR_GENERAL_NF_CONTROL];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_nf_control[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_nf_control(0U, o_emi_rej_en, sgrjt_en2, sgrjt_en1, bbkn_en, dgrjt_en2, dgrjt_en1);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_nf_control(0U, o_emi_rej_en, sgrjt_en2, sgrjt_en1, bbkn_en, dgrjt_en2, dgrjt_en1);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_NF_CONTROL, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_nf_control[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_chop_en(uint16_t open_mask_opt, uint16_t chop_en, uint16_t chop_drv_en, uint16_t chop_osc_ldo_en, uint16_t chop_osc_en, uint16_t chop_dac_en, uint16_t chop_bgr_en)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_CHOP_EN_OPEN_MASK_OPT_Msk, ID601_CHOP_EN_OPEN_MASK_OPT_Pos, (uint16_t)open_mask_opt);
    register_value = id601_set_field(register_value, ID601_CHOP_EN_CHOP_EN_Msk, ID601_CHOP_EN_CHOP_EN_Pos, (uint16_t)chop_en);
    register_value = id601_set_field(register_value, ID601_CHOP_EN_CHOP_DRV_EN_Msk, ID601_CHOP_EN_CHOP_DRV_EN_Pos, (uint16_t)chop_drv_en);
    register_value = id601_set_field(register_value, ID601_CHOP_EN_CHOP_OSC_LDO_EN_Msk, ID601_CHOP_EN_CHOP_OSC_LDO_EN_Pos, (uint16_t)chop_osc_ldo_en);
    register_value = id601_set_field(register_value, ID601_CHOP_EN_CHOP_OSC_EN_Msk, ID601_CHOP_EN_CHOP_OSC_EN_Pos, (uint16_t)chop_osc_en);
    register_value = id601_set_field(register_value, ID601_CHOP_EN_CHOP_DAC_EN_Msk, ID601_CHOP_EN_CHOP_DAC_EN_Pos, (uint16_t)chop_dac_en);
    register_value = id601_set_field(register_value, ID601_CHOP_EN_CHOP_BGR_EN_Msk, ID601_CHOP_EN_CHOP_BGR_EN_Pos, (uint16_t)chop_bgr_en);
    return register_value;
}

bool id601_write_chop_en(uint16_t device_select, uint16_t open_mask_opt, uint16_t chop_en, uint16_t chop_drv_en, uint16_t chop_osc_ldo_en, uint16_t chop_osc_en, uint16_t chop_dac_en, uint16_t chop_bgr_en)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_chop_en = gn_id601_general_regs[ID601_ADDR_GENERAL_CHOP_EN];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_chop_en[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_chop_en(open_mask_opt, chop_en, chop_drv_en, chop_osc_ldo_en, chop_osc_en, chop_dac_en, chop_bgr_en);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_chop_en(open_mask_opt, chop_en, chop_drv_en, chop_osc_ldo_en, chop_osc_en, chop_dac_en, chop_bgr_en);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_CHOP_EN, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_chop_en[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_temp(uint16_t ov_swap_en, uint16_t dac_rng, uint16_t flt_ctl, uint16_t o_slew, uint16_t flt_gain)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TEMP_OV_SWAP_EN_Msk, ID601_TEMP_OV_SWAP_EN_Pos, (uint16_t)ov_swap_en);
    register_value = id601_set_field(register_value, ID601_TEMP_DAC_RNG_Msk, ID601_TEMP_DAC_RNG_Pos, (uint16_t)dac_rng);
    register_value = id601_set_field(register_value, ID601_TEMP_FLT_CTL_Msk, ID601_TEMP_FLT_CTL_Pos, (uint16_t)flt_ctl);
    register_value = id601_set_field(register_value, ID601_TEMP_O_SLEW_Msk, ID601_TEMP_O_SLEW_Pos, (uint16_t)o_slew);
    register_value = id601_set_field(register_value, ID601_TEMP_FLT_GAIN_Msk, ID601_TEMP_FLT_GAIN_Pos, (uint16_t)flt_gain);
    return register_value;
}

bool id601_write_temp(uint16_t device_select, uint16_t ov_swap_en, uint16_t dac_rng, uint16_t flt_ctl, uint16_t o_slew, uint16_t flt_gain)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_temp = gn_id601_general_regs[ID601_ADDR_GENERAL_TEMP];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_temp[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_temp(ov_swap_en, dac_rng, flt_ctl, o_slew, flt_gain);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_temp(ov_swap_en, dac_rng, flt_ctl, o_slew, flt_gain);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_TEMP, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_temp[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_osc_fll_man1(uint16_t osc_fll_man)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OSC_FLL_MAN1_OSC_FLL_MAN_Msk, ID601_OSC_FLL_MAN1_OSC_FLL_MAN_Pos, (uint16_t)osc_fll_man);
    return register_value;
}

bool id601_write_osc_fll_man1(uint16_t device_select, uint16_t osc_fll_man)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_osc_fll_man1 = gn_id601_general_regs[ID601_ADDR_GENERAL_OSC_FLL_MAN1];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_osc_fll_man1[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_osc_fll_man1(osc_fll_man);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_osc_fll_man1(osc_fll_man);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OSC_FLL_MAN1, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_osc_fll_man1[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_osc_fll_man2(uint16_t osc_man_en, uint16_t osc_fll_err_range, uint16_t osc_fll_man)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OSC_FLL_MAN2_OSC_MAN_EN_Msk, ID601_OSC_FLL_MAN2_OSC_MAN_EN_Pos, (uint16_t)osc_man_en);
    register_value = id601_set_field(register_value, ID601_OSC_FLL_MAN2_OSC_FLL_ERR_RANGE_Msk, ID601_OSC_FLL_MAN2_OSC_FLL_ERR_RANGE_Pos, (uint16_t)osc_fll_err_range);
    register_value = id601_set_field(register_value, ID601_OSC_FLL_MAN2_OSC_FLL_MAN_Msk, ID601_OSC_FLL_MAN2_OSC_FLL_MAN_Pos, (uint16_t)osc_fll_man);
    return register_value;
}

bool id601_write_osc_fll_man2(uint16_t device_select, uint16_t osc_man_en, uint16_t osc_fll_err_range, uint16_t osc_fll_man)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_osc_fll_man2 = gn_id601_general_regs[ID601_ADDR_GENERAL_OSC_FLL_MAN2];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_osc_fll_man2[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_osc_fll_man2(osc_man_en, osc_fll_err_range, osc_fll_man);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_osc_fll_man2(osc_man_en, osc_fll_err_range, osc_fll_man);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OSC_FLL_MAN2, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_osc_fll_man2[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_osc_spread(uint16_t sprd_en, uint16_t sprd_spd, uint16_t sprd_gain)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OSC_SPREAD_SPRD_EN_Msk, ID601_OSC_SPREAD_SPRD_EN_Pos, (uint16_t)sprd_en);
    register_value = id601_set_field(register_value, ID601_OSC_SPREAD_SPRD_SPD_Msk, ID601_OSC_SPREAD_SPRD_SPD_Pos, (uint16_t)sprd_spd);
    register_value = id601_set_field(register_value, ID601_OSC_SPREAD_SPRD_GAIN_Msk, ID601_OSC_SPREAD_SPRD_GAIN_Pos, (uint16_t)sprd_gain);
    return register_value;
}

bool id601_write_osc_spread(uint16_t device_select, uint16_t sprd_en, uint16_t sprd_spd, uint16_t sprd_gain)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_osc_spread = gn_id601_general_regs[ID601_ADDR_GENERAL_OSC_SPREAD];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_osc_spread[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_osc_spread(sprd_en, sprd_spd, sprd_gain);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_osc_spread(sprd_en, sprd_spd, sprd_gain);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OSC_SPREAD, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_osc_spread[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_clock_gate_en(uint16_t otp_mclk_en, uint16_t fr2_mclk_en, uint16_t fr1_mclk_en, uint16_t dc_mclk_en)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_CLOCK_GATE_EN_OTP_MCLK_EN_Msk, ID601_CLOCK_GATE_EN_OTP_MCLK_EN_Pos, (uint16_t)otp_mclk_en);
    register_value = id601_set_field(register_value, ID601_CLOCK_GATE_EN_FR2_MCLK_EN_Msk, ID601_CLOCK_GATE_EN_FR2_MCLK_EN_Pos, (uint16_t)fr2_mclk_en);
    register_value = id601_set_field(register_value, ID601_CLOCK_GATE_EN_FR1_MCLK_EN_Msk, ID601_CLOCK_GATE_EN_FR1_MCLK_EN_Pos, (uint16_t)fr1_mclk_en);
    register_value = id601_set_field(register_value, ID601_CLOCK_GATE_EN_DC_MCLK_EN_Msk, ID601_CLOCK_GATE_EN_DC_MCLK_EN_Pos, (uint16_t)dc_mclk_en);
    return register_value;
}

bool id601_write_clock_gate_en(uint16_t device_select, uint16_t otp_mclk_en, uint16_t fr2_mclk_en, uint16_t fr1_mclk_en, uint16_t dc_mclk_en)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_clock_gate_en = gn_id601_general_regs[ID601_ADDR_GENERAL_CLOCK_GATE_EN];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_clock_gate_en[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_clock_gate_en(otp_mclk_en, fr2_mclk_en, fr1_mclk_en, dc_mclk_en);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_clock_gate_en(otp_mclk_en, fr2_mclk_en, fr1_mclk_en, dc_mclk_en);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_CLOCK_GATE_EN, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_clock_gate_en[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_vref_fix1(uint16_t vref_fix1)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_VREF_FIX1_VREF_FIX1_Msk, ID601_VREF_FIX1_VREF_FIX1_Pos, (uint16_t)vref_fix1);
    return register_value;
}

bool id601_write_vref_fix1(uint16_t device_select, uint16_t vref_fix1)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_vref_fix1 = gn_id601_general_regs[ID601_ADDR_GENERAL_VREF_FIX1];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_vref_fix1[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_vref_fix1(vref_fix1);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_vref_fix1(vref_fix1);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_VREF_FIX1, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_vref_fix1[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_vref_fix2(uint16_t vref_fix2)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_VREF_FIX2_VREF_FIX2_Msk, ID601_VREF_FIX2_VREF_FIX2_Pos, (uint16_t)vref_fix2);
    return register_value;
}

bool id601_write_vref_fix2(uint16_t device_select, uint16_t vref_fix2)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_vref_fix2 = gn_id601_general_regs[ID601_ADDR_GENERAL_VREF_FIX2];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_vref_fix2[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_vref_fix2(vref_fix2);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_vref_fix2(vref_fix2);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_VREF_FIX2, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_vref_fix2[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_vref_fix3(uint16_t vref_fix3)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_VREF_FIX3_VREF_FIX3_Msk, ID601_VREF_FIX3_VREF_FIX3_Pos, (uint16_t)vref_fix3);
    return register_value;
}

bool id601_write_vref_fix3(uint16_t device_select, uint16_t vref_fix3)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_vref_fix3 = gn_id601_general_regs[ID601_ADDR_GENERAL_VREF_FIX3];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_vref_fix3[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_vref_fix3(vref_fix3);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_vref_fix3(vref_fix3);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_VREF_FIX3, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_vref_fix3[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_test_ana_en(uint16_t test_ana_en)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TEST_ANA_EN_TEST_ANA_EN_Msk, ID601_TEST_ANA_EN_TEST_ANA_EN_Pos, (uint16_t)test_ana_en);
    return register_value;
}

bool id601_write_test_ana_en(uint16_t device_select, uint16_t test_ana_en)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_test_ana_en = gn_id601_general_regs[ID601_ADDR_GENERAL_TEST_ANA_EN];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_test_ana_en[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_test_ana_en(test_ana_en);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_test_ana_en(test_ana_en);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_TEST_ANA_EN, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_test_ana_en[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_otp_access1(uint16_t otp_pg_acc_cycle)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_ACCESS1_OTP_PG_ACC_CYCLE_Msk, ID601_OTP_ACCESS1_OTP_PG_ACC_CYCLE_Pos, (uint16_t)otp_pg_acc_cycle);
    return register_value;
}

bool id601_write_otp_access1(uint16_t device_select, uint16_t otp_pg_acc_cycle)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_otp_access1 = gn_id601_general_regs[ID601_ADDR_GENERAL_OTP_ACCESS1];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_otp_access1[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_otp_access1(otp_pg_acc_cycle);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_otp_access1(otp_pg_acc_cycle);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OTP_ACCESS1, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_otp_access1[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_otp_access2(uint16_t otp_pg_acc_cycle)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_ACCESS2_OTP_PG_ACC_CYCLE_Msk, ID601_OTP_ACCESS2_OTP_PG_ACC_CYCLE_Pos, (uint16_t)otp_pg_acc_cycle);
    return register_value;
}

bool id601_write_otp_access2(uint16_t device_select, uint16_t otp_pg_acc_cycle)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_otp_access2 = gn_id601_general_regs[ID601_ADDR_GENERAL_OTP_ACCESS2];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_otp_access2[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_otp_access2(otp_pg_acc_cycle);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_otp_access2(otp_pg_acc_cycle);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OTP_ACCESS2, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_otp_access2[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_otp_write(uint16_t otp_rd, uint16_t otp_wsel)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_WRITE_OTP_RD_Msk, ID601_OTP_WRITE_OTP_RD_Pos, (uint16_t)otp_rd);
    register_value = id601_set_field(register_value, ID601_OTP_WRITE_OTP_WSEL_Msk, ID601_OTP_WRITE_OTP_WSEL_Pos, (uint16_t)otp_wsel);
    return register_value;
}

bool id601_write_otp_write(uint16_t device_select, uint16_t otp_rd, uint16_t otp_wsel)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_otp_write = gn_id601_general_regs[ID601_ADDR_GENERAL_OTP_WRITE];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_otp_write[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_otp_write(otp_rd, otp_wsel);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_otp_write(otp_rd, otp_wsel);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OTP_WRITE, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_otp_write[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_otp_rd_prog(uint16_t otp_pg_done, uint16_t otp_rd_s, uint16_t otp_pg_s)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_RD_PROG_OTP_PG_DONE_Msk, ID601_OTP_RD_PROG_OTP_PG_DONE_Pos, (uint16_t)otp_pg_done);
    register_value = id601_set_field(register_value, ID601_OTP_RD_PROG_OTP_RD_S_Msk, ID601_OTP_RD_PROG_OTP_RD_S_Pos, (uint16_t)otp_rd_s);
    register_value = id601_set_field(register_value, ID601_OTP_RD_PROG_OTP_PG_S_Msk, ID601_OTP_RD_PROG_OTP_PG_S_Pos, (uint16_t)otp_pg_s);
    return register_value;
}

bool id601_write_otp_rd_prog(uint16_t device_select, uint16_t otp_pg_done, uint16_t otp_rd_s, uint16_t otp_pg_s)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_otp_rd_prog = gn_id601_general_regs[ID601_ADDR_GENERAL_OTP_RD_PROG];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_otp_rd_prog[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_otp_rd_prog(otp_pg_done, otp_rd_s, otp_pg_s);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_otp_rd_prog(otp_pg_done, otp_rd_s, otp_pg_s);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OTP_RD_PROG, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_otp_rd_prog[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_otp_protect(uint16_t protect_en)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_PROTECT_PROTECT_EN_Msk, ID601_OTP_PROTECT_PROTECT_EN_Pos, (uint16_t)protect_en);
    return register_value;
}

bool id601_write_otp_protect(uint16_t device_select, uint16_t protect_en)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_otp_protect = gn_id601_general_regs[ID601_ADDR_GENERAL_OTP_PROTECT];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_otp_protect[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_otp_protect(protect_en);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_otp_protect(protect_en);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OTP_PROTECT, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_otp_protect[device_index] = tx_data[device_index];
    }
    return true;
}

static inline uint16_t id601_build_op_mode(uint16_t test_en, uint16_t sw_sel, uint16_t pwmout_full, uint16_t mclk64_o, uint16_t dtest_mux_en, uint16_t rd_en, uint16_t ext_clkin, uint16_t addr_ext)
{
    uint16_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OP_MODE_TEST_EN_Msk, ID601_OP_MODE_TEST_EN_Pos, (uint16_t)test_en);
    register_value = id601_set_field(register_value, ID601_OP_MODE_SW_SEL_Msk, ID601_OP_MODE_SW_SEL_Pos, (uint16_t)sw_sel);
    register_value = id601_set_field(register_value, ID601_OP_MODE_PWMOUT_FULL_Msk, ID601_OP_MODE_PWMOUT_FULL_Pos, (uint16_t)pwmout_full);
    register_value = id601_set_field(register_value, ID601_OP_MODE_MCLK64_O_Msk, ID601_OP_MODE_MCLK64_O_Pos, (uint16_t)mclk64_o);
    register_value = id601_set_field(register_value, ID601_OP_MODE_DTEST_MUX_EN_Msk, ID601_OP_MODE_DTEST_MUX_EN_Pos, (uint16_t)dtest_mux_en);
    register_value = id601_set_field(register_value, ID601_OP_MODE_RD_EN_Msk, ID601_OP_MODE_RD_EN_Pos, (uint16_t)rd_en);
    register_value = id601_set_field(register_value, ID601_OP_MODE_EXT_CLKIN_Msk, ID601_OP_MODE_EXT_CLKIN_Pos, (uint16_t)ext_clkin);
    register_value = id601_set_field(register_value, ID601_OP_MODE_ADDR_EXT_Msk, ID601_OP_MODE_ADDR_EXT_Pos, (uint16_t)addr_ext);
    return register_value;
}

bool id601_write_op_mode(uint16_t device_select, uint16_t test_en, uint16_t sw_sel, uint16_t pwmout_full, uint16_t mclk64_o, uint16_t dtest_mux_en, uint16_t rd_en, uint16_t ext_clkin, uint16_t addr_ext)
{
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_op_mode = gn_id601_general_regs[ID601_ADDR_GENERAL_OP_MODE];
    if (device_select > gn_id601_daisy_length)
    {
        return false;
    }
    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_op_mode[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = id601_build_op_mode(test_en, sw_sel, pwmout_full, mclk64_o, dtest_mux_en, rd_en, ext_clkin, addr_ext);
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = id601_build_op_mode(test_en, sw_sel, pwmout_full, mclk64_o, dtest_mux_en, rd_en, ext_clkin, addr_ext);
    }

    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, ID601_ADDR_GENERAL_OP_MODE, tx_data, NULL))
    {
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_op_mode[device_index] = tx_data[device_index];
    }
    return true;
}

static bool id601_read_all(void)
{
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_RESET_ID ; id601_addr < ID601_ADDR_GENERAL_COUNT ; ++id601_addr)
    {
        uint16_t rx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
        if (false == id601_read_register(ID601_REGISTER_BANK_GENERAL, id601_addr, rx_data, NULL))
        {
            drv_uart_printf("\r\n    ID601 Failed to read general register 0x%04X\r\n    Read Stoped!!", id601_addr);
            return false;
        }
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            gn_id601_general_regs[id601_addr][device_index] = rx_data[device_index];
        }
    }

    for (uint16_t id601_addr = ID601_ADDR_MIRROR1 ; id601_addr < ID601_ADDR_MIRROR_COUNT ; ++id601_addr)
    {
        uint16_t rx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
        if (false == id601_read_register(ID601_REGISTER_BANK_MIRROR, id601_addr, rx_data, NULL))
        {
            drv_uart_printf("\r\n    ID601 Failed to read mirror register 0x%04X\r\n    Read Stoped!!", id601_addr);
            return false;
        }
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            gn_id601_mirror_regs[id601_addr][device_index] = rx_data[device_index];
        }
    }
    return true;
}

static bool id601_dump_all(void)
{
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_RESET_ID ; id601_addr < ID601_ADDR_GENERAL_COUNT ; ++id601_addr)
    {
        drv_uart_printf("\r\n    ID601 General Register 0x%04X Dump:", id601_addr);
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            drv_uart_printf("\r\n        Device %u: 0x%04X", device_index, gn_id601_general_regs[id601_addr][device_index]);
        }
    }

    for (uint16_t id601_addr = ID601_ADDR_MIRROR1 ; id601_addr < ID601_ADDR_MIRROR_COUNT ; ++id601_addr)
    {
        drv_uart_printf("\r\n    ID601 Mirror Register 0x%04X Dump:", id601_addr);
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            drv_uart_printf("\r\n        Device %u: 0x%04X", device_index, gn_id601_mirror_regs[id601_addr][device_index]);
        }
    }
    return true;
}

bool id601_test_init_analog(void)
{
    drv_gpio_id601_vcc(ID601_VCC_3V3);
    id601_reset();
    id601_idgen_command(gp_id601_crc_func);
    id601_read_all();
    id601_dump_all();
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_RESET_ID ; id601_addr < ID601_ADDR_GENERAL_COUNT ; ++id601_addr)
    {
        switch (id601_addr)
        {
            case ID601_ADDR_GENERAL_FAULT_CONTROL:
            {
                if (false == id601_write_fault_control(ID601_BROADCAST, 0U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_FLLCNT1:
            {
                if (false == id601_write_fllcnt1(ID601_BROADCAST, 0xDCDU))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_CHOP_EN:
            {
                if (false == id601_write_chop_en(ID601_BROADCAST, 0U, 1U, 1U, 1U, 1U, 1U, 1U))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_OP_MODE:
            {
                if (false == id601_write_op_mode(ID601_BROADCAST, 1U, 1U, 1U, 0U, 0U, 1U, 0U, 0U))
                {
                    return false;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return true;
}
/* USER CODE END 0 */
