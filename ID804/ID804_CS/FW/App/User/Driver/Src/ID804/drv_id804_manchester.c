/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_id804_manchester.c
    * @brief          : ID804_MANCHESTER driver implementation
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
#include "drv_id804_manchester.h"
/* 2. C standard library headers (Alphabetical order) */
#include <stdio.h>
#include <stdbool.h>
/* 3. Project internal / System-related headers */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct tag_ID804_CMD_INFO
{
    const char          *name;
    id804_cmd_list_t    command;
    bool                broadcast;
    bool                multicast;
    uint8_t             databits;
} id804_cmd_info_t;

typedef uint8_t (*id804_crc_func_t)(const uint8_t *p_data, uint16_t len);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ID805_BC_DEV_ADDR       (0U)
#define ID804_BC_ABL            (true)
#define ID804_BC_DIS            (false)

#define ID804_MC_ABL            (true)
#define ID804_MC_DIS            (false)

#define ID804_DATABIT_00        ( 0U)
#define ID804_DATABIT_12        (12U)
#define ID804_DATABIT_24        (24U)

#define ID804_PREAMBLE_VAL      (0x0AU)

#define ID804_FRAME_LEN_24BIT   (7U)
#define ID804_FRAME_LEN_12BIT   (6U)
#define ID804_FRAME_LEN_0BIT    (4U)

/* 1. Protocol Specification Limits (실제 프로토콜 최대 규격 바이트) */
#define ID804_FRAME_MAX_RAW_BYTES           (7U)   /* 24-bit Data Frame: 7 Bytes */
#define ID804_FRAME_MAX_MANCHESTER_BYTES    (ID804_FRAME_MAX_RAW_BYTES * 2U) /* 14 Bytes */

/* 2. Buffer Allocation Sizes (2의 거듭제곱 정렬 & 오버플로우 방지 샌드박스 버퍼 크기) */
#define ID804_RAW_BUF_SIZE      (8U)   /* 8-Byte Aligned Buffer (Margin: +1 Byte) */
#define ID804_SPI_TX_BUF_SIZE   (16U)  /* 16-Byte Aligned Buffer (Margin: +2 Bytes) */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
const static id804_cmd_info_t gt_id804_command[] =
{
    /* ========================================================================================================= */
    /*  Name                     Command                         Broadcast       Multicast       Data Bits       */
    /* ========================================================================================================= */
    /* System Commands */
    { "RESET",                   ID804_CMD_RESET,                ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_00 },
    { "INITBIDIR",               ID804_CMD_INITBIDIR,            ID804_BC_DIS,   ID804_MC_DIS,   ID804_DATABIT_00 },
    { "CLRERROR",                ID804_CMD_CLRERROR,             ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_00 },
    { "GOSLEEP",                 ID804_CMD_GOSLEEP,              ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_00 },
    { "GOACTIVE",                ID804_CMD_GOACTIVE,             ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_00 },
    { "GODEEPSLEEP",             ID804_CMD_GODEEPSLEEP,          ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_00 },

    /* Set Configuration Commands */
    { "SET_SETUP1",              ID804_CMD_SET_SETUP1,           ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_SETUP2",              ID804_CMD_SET_SETUP2,           ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_MCAST",               ID804_CMD_SET_MCAST,            ID804_BC_DIS,   ID804_MC_DIS,   ID804_DATABIT_24 },
    { "SET_TEMPTH",              ID804_CMD_SET_TEMPTH,           ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMPHYS",             ID804_CMD_SET_TEMPHYS,          ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_CURR_MAX_LVL",        ID804_CMD_SET_CURR_MAX_LVL,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC1",        ID804_CMD_SET_TEMP_LUT_TC1,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC2",        ID804_CMD_SET_TEMP_LUT_TC2,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC3",        ID804_CMD_SET_TEMP_LUT_TC3,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC4",        ID804_CMD_SET_TEMP_LUT_TC4,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC5",        ID804_CMD_SET_TEMP_LUT_TC5,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC6",        ID804_CMD_SET_TEMP_LUT_TC6,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC7",        ID804_CMD_SET_TEMP_LUT_TC7,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC8",        ID804_CMD_SET_TEMP_LUT_TC8,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC9",        ID804_CMD_SET_TEMP_LUT_TC9,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TEMP_LUT_TC10",       ID804_CMD_SET_TEMP_LUT_TC10,    ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_TIMEOUT",             ID804_CMD_SET_TIMEOUT,          ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_12 },
    { "SET_RGB",                 ID804_CMD_SET_RGB,              ID804_BC_ABL,   ID804_MC_ABL,   ID804_DATABIT_24 },

    /* Read Commands */
    { "READ_STATUS1",            ID804_CMD_READ_STATUS1,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_STATUS2",            ID804_CMD_READ_STATUS2,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP",               ID804_CMD_READ_TEMP,            ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMPST",             ID804_CMD_READ_TEMPST,          ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_24 },
    { "READ_VEXT_TM",            ID804_CMD_READ_VEXT_TM,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_SETUP1",             ID804_CMD_READ_SETUP1,          ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_SETUP2",             ID804_CMD_READ_SETUP2,          ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_MCAST",              ID804_CMD_READ_MCAST,           ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_24 },
    { "READ_TEMPTH",             ID804_CMD_READ_TEMPTH,          ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMPHYS",            ID804_CMD_READ_TEMPHYS,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_PWM_RED_VAL",        ID804_CMD_READ_PWM_RED_VAL,     ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_PWM_GREEN_VAL",      ID804_CMD_READ_PWM_GREEN_VAL,   ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_PWM_BLUE_VAL",       ID804_CMD_READ_PWM_BLUE_VAL,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_CURR_MAX_LVL",       ID804_CMD_READ_CURR_MAX_LVL,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC1",       ID804_CMD_READ_TEMP_LUT_TC1,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC2",       ID804_CMD_READ_TEMP_LUT_TC2,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC3",       ID804_CMD_READ_TEMP_LUT_TC3,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC4",       ID804_CMD_READ_TEMP_LUT_TC4,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC5",       ID804_CMD_READ_TEMP_LUT_TC5,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC6",       ID804_CMD_READ_TEMP_LUT_TC6,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC7",       ID804_CMD_READ_TEMP_LUT_TC7,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC8",       ID804_CMD_READ_TEMP_LUT_TC8,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC9",       ID804_CMD_READ_TEMP_LUT_TC9,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TEMP_LUT_TC10",      ID804_CMD_READ_TEMP_LUT_TC10,   ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_TIMEOUT",            ID804_CMD_READ_TIMEOUT,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_12 },
    { "READ_RGB",                ID804_CMD_READ_RGB,             ID804_BC_ABL,   ID804_MC_DIS,   ID804_DATABIT_24 }
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief Get command information based on the command list.
 * @param cmd The command to look up.
 * @return Pointer to the command information structure if found, NULL otherwise.
 */
const static id804_cmd_info_t* id804_get_cmd_info(id804_cmd_list_t cmd)
{
    uint16_t table_size = sizeof(gt_id804_command) / sizeof(gt_id804_command[0]);

    for (uint16_t i = 0U; i < table_size; i++)
    {
        if (gt_id804_command[i].command == cmd)
        {
            return &gt_id804_command[i];
        }
    }
    return NULL;
}

/**
 * @brief Build a 24-bit data frame for the ID804 device.
 * @param dev_addr The device address.
 * @param cmd The command to send.
 * @param data The 24-bit data to include in the frame.
 * @param p_out_buf Pointer to the output buffer where the frame will be stored.
 * @param crc_func Function pointer to the CRC calculation function.
 * @return The length of the constructed frame.
 */
static uint8_t id804_build_frame_24bit(uint8_t dev_addr, uint8_t cmd, uint32_t data, uint8_t *p_out_buf, id804_crc_func_t crc_func)
{
    if (p_out_buf == NULL) return 0U;

    p_out_buf[0] = (uint8_t)(((ID804_PREAMBLE_VAL & 0x0FU) << 4U) | ((dev_addr >> 8U) & 0x0FU));
    p_out_buf[1] = (uint8_t)(dev_addr & 0xFFU);
    p_out_buf[2] = cmd;
    p_out_buf[3] = (uint8_t)((data >> 16U) & 0xFFU);
    p_out_buf[4] = (uint8_t)((data >> 8U) & 0xFFU);
    p_out_buf[5] = (uint8_t)(data & 0xFFU);

    if (crc_func != NULL)
    {
        p_out_buf[6] = crc_func(p_out_buf, 6U);
    }
    else
    {
        p_out_buf[6] = 0x00U;
    }

    return ID804_FRAME_LEN_24BIT;
}

/**
 * @brief Build a 12-bit data frame for the ID804 device.
 * @param dev_addr The device address.
 * @param cmd The command to send.
 * @param data The 12-bit data to include in the frame.
 * @param p_out_buf Pointer to the output buffer where the frame will be stored.
 * @param crc_func Function pointer to the CRC calculation function.
 * @return The length of the constructed frame.
 */
static uint8_t id804_build_frame_12bit(uint8_t dev_addr, uint8_t cmd, uint16_t data, uint8_t *p_out_buf, id804_crc_func_t crc_func)
{
    if (p_out_buf == NULL) return 0U;

    p_out_buf[0] = (uint8_t)(((ID804_PREAMBLE_VAL & 0x0FU) << 4U) | ((dev_addr >> 8U) & 0x0FU));
    p_out_buf[1] = (uint8_t)(dev_addr & 0xFFU);
    p_out_buf[2] = cmd;
    p_out_buf[3] = (uint8_t)((data >> 4U) & 0xFFU);
    p_out_buf[4] = (uint8_t)(((data & 0x0FU) << 4U) | 0x00U);

    if (crc_func != NULL)
    {
        p_out_buf[5] = crc_func(p_out_buf, 5U);
    }
    else
    {
        p_out_buf[5] = 0x00U;
    }

    return ID804_FRAME_LEN_12BIT;
}

/**
 * @brief Build a 0-bit data frame for the ID804 device.
 * @param dev_addr The device address.
 * @param cmd The command to send.
 * @param p_out_buf Pointer to the output buffer where the frame will be stored.
 * @param crc_func Function pointer to the CRC calculation function.
 * @return The length of the constructed frame.
 */
static uint8_t id804_build_frame_0bit(uint8_t dev_addr, uint8_t cmd, uint8_t *p_out_buf, id804_crc_func_t crc_func)
{
    if (p_out_buf == NULL) return 0U;

    p_out_buf[0] = (uint8_t)(((ID804_PREAMBLE_VAL & 0x0FU) << 4U) | ((dev_addr >> 8U) & 0x0FU));
    p_out_buf[1] = (uint8_t)(dev_addr & 0xFFU);
    p_out_buf[2] = cmd;

    if (crc_func != NULL)
    {
        p_out_buf[3] = crc_func(p_out_buf, 3U);
    }
    else
    {
        p_out_buf[3] = 0x00U;
    }

    return ID804_FRAME_LEN_0BIT;
}

/**
 * @brief Build a data packet for the ID804 device based on the command information.
 * @param dev_addr The device address.
 * @param cmd The command to send.
 * @param raw_data The raw data to include in the frame (if applicable).
 * @param p_out_buf Pointer to the output buffer where the frame will be stored.
 * @param crc_func Function pointer to the CRC calculation function.
 * @return The length of the constructed frame, or 0 if the command is invalid.
 */
static uint8_t id804_build_packet_by_cmd(uint8_t dev_addr, id804_cmd_list_t cmd, uint32_t raw_data, uint8_t *p_out_buf, id804_crc_func_t crc_func)
{
    uint8_t frame_len = 0U;
    const id804_cmd_info_t *p_info = id804_get_cmd_info(cmd);

    if (p_info == NULL)
    {
        return 0U;
    }

    if ((dev_addr == ID805_BC_DEV_ADDR) && (true != p_info->broadcast))
    {
        return 0U;
    }

    switch (p_info->databits)
    {
        case ID804_DATABIT_00:
        {
            frame_len = id804_build_frame_0bit(dev_addr, (uint8_t)p_info->command, p_out_buf, crc_func);
            break;
        }

        case ID804_DATABIT_12:
        {
            frame_len = id804_build_frame_12bit(dev_addr, (uint8_t)p_info->command, (uint16_t)raw_data, p_out_buf, crc_func);
            break;
        }

        case ID804_DATABIT_24:
        {
            frame_len = id804_build_frame_24bit(dev_addr, (uint8_t)p_info->command, raw_data, p_out_buf, crc_func);
            break;
        }

        default:
        {
            frame_len = 0U;
            break;
        }
    }

    return frame_len;
}

/**
 * @brief Encode a single byte into Manchester code.
 * @param src_byte The byte to encode.
 * @return The Manchester-encoded 16-bit value.
 */
static uint16_t id804_encode_byte_to_manchester(uint8_t src_byte)
{
    uint16_t encoded = 0x0000U;

    /* MSB부터 LSB까지 8개 비트를 각각 2비트로 확장 (MSB First) */
    for (int8_t i = 7; i >= 0; i--)
    {
        encoded <<= 2U; // 2비트 공간 확보

        if (((src_byte >> (uint8_t)i) & 0x01U) != 0U)
        {
            encoded |= 0x01U; // Bit '1' -> 0b01 (Low to High)
        }
        else
        {
            encoded |= 0x02U; // Bit '0' -> 0b10 (High to Low)
        }
    }

    return encoded;
}

/**
 * @brief Encode a buffer into Manchester code.
 * @param p_src Pointer to the source buffer.
 * @param src_len Length of the source buffer.
 * @param p_dst Pointer to the destination buffer to store Manchester-encoded data.
 * @return The length of the Manchester-encoded data in bytes.
 */
uint16_t id804_manchester_encode_buffer(const uint8_t *p_src, uint16_t src_len, uint8_t *p_dst)
{
    if ((p_src == NULL) || (p_dst == NULL) || (src_len == 0U))
    {
        return 0U;
    }

    uint16_t dst_idx = 0U;

    for (uint16_t i = 0U; i < src_len; ++i)
    {
        uint16_t enc_val = id804_encode_byte_to_manchester(p_src[i]);
        p_dst[dst_idx++] = (uint8_t)((enc_val >> 8U) & 0xFFU);
        p_dst[dst_idx++] = (uint8_t)((enc_val >> 0U) & 0xFFU);
    }

    return dst_idx;
}

/**
 * @brief Send a command to the ID804 device via SPI.
 * @param dev_addr The device address.
 * @param cmd The command to send.
 * @param data The data associated with the command.
 */
// not yet!!!!!
void id804_transmit_cmd_via_spi(uint8_t dev_addr, id804_cmd_list_t cmd, uint32_t data)
{
    uint8_t raw_packet[ID804_RAW_BUF_SIZE];
    uint8_t spi_tx_buffer[ID804_SPI_TX_BUF_SIZE];

    uint16_t raw_len;
    uint16_t spi_tx_len;

    raw_len = id804_build_packet_by_cmd(dev_addr, cmd, data, raw_packet, NULL/*my_crc8_calculator*/);

    if (raw_len == 0U)
    {
        return;
    }

    spi_tx_len = id804_manchester_encode_buffer(raw_packet, raw_len, spi_tx_buffer);

    /* 3단계: SPI 통신 전송 (MCU 내장 SPI 전송 API 호출) */
    // 예시: HAL_SPI_Transmit(&hspi1, spi_tx_buffer, (uint16_t)spi_tx_len, 100);
    // 예시: SPI_Master_Write(spi_tx_buffer, spi_tx_len);
}

// not yet!!!!!
uint32_t id804_receive_cmd_via_spi(uint8_t dev_addr, id804_cmd_list_t cmd, uint32_t data)
{
    uint8_t raw_packet[ID804_RAW_BUF_SIZE];
    uint8_t spi_tx_buffer[ID804_SPI_TX_BUF_SIZE];

    uint16_t raw_len;
    uint16_t spi_tx_len;

    raw_len = id804_build_packet_by_cmd(dev_addr, cmd, data, raw_packet, NULL/*my_crc8_calculator*/);

    if (raw_len == 0U)
    {
        return 0U;
    }

    spi_tx_len = id804_manchester_encode_buffer(raw_packet, raw_len, spi_tx_buffer);

    /* 3단계: SPI 통신 전송 (MCU 내장 SPI 전송 API 호출) */
    // 예시: HAL_SPI_Transmit(&hspi1, spi_tx_buffer, (uint16_t)spi_tx_len, 100);
    // 예시: SPI_Master_Write(spi_tx_buffer, spi_tx_len);
    return 0U;
}
/* USER CODE END 0 */
