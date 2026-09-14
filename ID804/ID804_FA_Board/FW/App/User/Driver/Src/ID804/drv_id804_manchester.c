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
/* 3. Project internal / System-related headers */
/* USER CODE END Includes */
#include "drv_spi.h"
#include "drv_uart.h"
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct tag_ID804_CMD_INFO
{
    const char          *name;
    id804_cmd_list_t    command;
    bool                broadcast;
    bool                multicast;
    uint8_t             req_bytes;
    uint8_t             rsp_bytes;
} id804_cmd_info_t;

typedef uint8_t (*id804_crc_func_t)(const uint8_t *p_data, uint16_t len);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define ID804_RW_DEBUG                  (0U)

#define ID805_BC_DEV_ADDR               (0U)

#define ID804_BC_ABL                    (true)  // Broadcast Able
#define ID804_BC_DIS                    (false) // Broadcast Disable

#define ID804_MC_ABL                    (true)  // Multicast Able
#define ID804_MC_DIS                    (false) // Multicast Disable

#define ID804_FRAME_PREAMBLE_VAL        (0x0AU)
#define ID804_FRAME_DUMMY_VAL           (0x0AU)

#define ID804_FRAME_BYTE_HEADER         (3U)
#define ID804_FRAME_BYTE_DATA_12BIT     (2U)
#define ID804_FRAME_BYTE_DATA_24BIT     (3U)
#define ID804_FRAME_BYTE_CRC            (1U)

#define ID804_FRAME_LEN_0BIT            (ID804_FRAME_BYTE_HEADER + ID804_FRAME_BYTE_CRC)
#define ID804_FRAME_LEN_12BIT           (ID804_FRAME_BYTE_HEADER + ID804_FRAME_BYTE_DATA_12BIT + ID804_FRAME_BYTE_CRC)
#define ID804_FRAME_LEN_24BIT           (ID804_FRAME_BYTE_HEADER + ID804_FRAME_BYTE_DATA_24BIT + ID804_FRAME_BYTE_CRC)

#define ID804_FRAME_BUF_SIZE            (8U)  /* 24-bit Data Frame: 7 Bytes + 1 Byte Margin */
#define ID804_SPI_TX_BUF_SIZE           (16U) /* 16-Byte (2-times the maximum raw frame size) */

#define ID804_SPI_DUMMY_HEADER_SIZE     (1U)

#define ID804_FRAME_TX_IDX              (0U)
#define ID804_FRAME_RX_IDX              (1U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static const uint8_t gn_id804_crc8_LUT[256] =
{
    0x00, 0x2F, 0x5E, 0x71, 0xBC, 0x93, 0xE2, 0xCD, 0x57, 0x78, 0x09, 0x26, 0xEB, 0xC4, 0xB5, 0x9A,
    0xAE, 0x81, 0xF0, 0xDF, 0x12, 0x3D, 0x4C, 0x63, 0xF9, 0xD6, 0xA7, 0x88, 0x45, 0x6A, 0x1B, 0x34,
    0x73, 0x5C, 0x2D, 0x02, 0xCF, 0xE0, 0x91, 0xBE, 0x24, 0x0B, 0x7A, 0x55, 0x98, 0xB7, 0xC6, 0xE9,
    0xDD, 0xF2, 0x83, 0xAC, 0x61, 0x4E, 0x3F, 0x10, 0x8A, 0xA5, 0xD4, 0xFB, 0x36, 0x19, 0x68, 0x47,
    0xE6, 0xC9, 0xB8, 0x97, 0x5A, 0x75, 0x04, 0x2B, 0xB1, 0x9E, 0xEF, 0xC0, 0x0D, 0x22, 0x53, 0x7C,
    0x48, 0x67, 0x16, 0x39, 0xF4, 0xDB, 0xAA, 0x85, 0x1F, 0x30, 0x41, 0x6E, 0xA3, 0x8C, 0xFD, 0xD2,
    0x95, 0xBA, 0xCB, 0xE4, 0x29, 0x06, 0x77, 0x58, 0xC2, 0xED, 0x9C, 0xB3, 0x7E, 0x51, 0x20, 0x0F,
    0x3B, 0x14, 0x65, 0x4A, 0x87, 0xA8, 0xD9, 0xF6, 0x6C, 0x43, 0x32, 0x1D, 0xD0, 0xFF, 0x8E, 0xA1,
    0xE3, 0xCC, 0xBD, 0x92, 0x5F, 0x70, 0x01, 0x2E, 0xB4, 0x9B, 0xEA, 0xC5, 0x08, 0x27, 0x56, 0x79,
    0x4D, 0x62, 0x13, 0x3C, 0xF1, 0xDE, 0xAF, 0x80, 0x1A, 0x35, 0x44, 0x6B, 0xA6, 0x89, 0xF8, 0xD7,
    0x90, 0xBF, 0xCE, 0xE1, 0x2C, 0x03, 0x72, 0x5D, 0xC7, 0xE8, 0x99, 0xB6, 0x7B, 0x54, 0x25, 0x0A,
    0x3E, 0x11, 0x60, 0x4F, 0x82, 0xAD, 0xDC, 0xF3, 0x69, 0x46, 0x37, 0x18, 0xD5, 0xFA, 0x8B, 0xA4,
    0x05, 0x2A, 0x5B, 0x74, 0xB9, 0x96, 0xE7, 0xC8, 0x52, 0x7D, 0x0C, 0x23, 0xEE, 0xC1, 0xB0, 0x9F,
    0xAB, 0x84, 0xF5, 0xDA, 0x17, 0x38, 0x49, 0x66, 0xFC, 0xD3, 0xA2, 0x8D, 0x40, 0x6F, 0x1E, 0x31,
    0x76, 0x59, 0x28, 0x07, 0xCA, 0xE5, 0x94, 0xBB, 0x21, 0x0E, 0x7F, 0x50, 0x9D, 0xB2, 0xC3, 0xEC,
    0xD8, 0xF7, 0x86, 0xA9, 0x64, 0x4B, 0x3A, 0x15, 0x8F, 0xA0, 0xD1, 0xFE, 0x33, 0x1C, 0x6D, 0x42
};

static const id804_cmd_info_t gt_id804_command[] =
{
    /* ========================================================================================================================= */
    /*  Name                    Command                         Broadcast       Multicast       Request Bytes           Response Bytes */
    /* ========================================================================================================================= */
    /* System Commands */
    { "RESET",                  ID804_CMD_RESET,                ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_0BIT },
    { "INITBIDIR",              ID804_CMD_INITBIDIR,            ID804_BC_DIS,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_0BIT },
    { "CLRERROR",               ID804_CMD_CLRERROR,             ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_0BIT },
    { "GOSLEEP",                ID804_CMD_GOSLEEP,              ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_0BIT },
    { "GOACTIVE",               ID804_CMD_GOACTIVE,             ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_0BIT },
    { "GODEEPSLEEP",            ID804_CMD_GODEEPSLEEP,          ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_0BIT },

    /* Set Configuration Commands */
    { "SET_SETUP1",             ID804_CMD_SET_SETUP1,           ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_SETUP2",             ID804_CMD_SET_SETUP2,           ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_MCAST",              ID804_CMD_SET_MCAST,            ID804_BC_DIS,   ID804_MC_DIS,   ID804_FRAME_LEN_24BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMPTH",             ID804_CMD_SET_TEMPTH,           ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMPHYS",            ID804_CMD_SET_TEMPHYS,          ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_CURR_MAX_LVL",       ID804_CMD_SET_CURR_MAX_LVL,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC1",       ID804_CMD_SET_TEMP_LUT_TC1,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC2",       ID804_CMD_SET_TEMP_LUT_TC2,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC3",       ID804_CMD_SET_TEMP_LUT_TC3,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC4",       ID804_CMD_SET_TEMP_LUT_TC4,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC5",       ID804_CMD_SET_TEMP_LUT_TC5,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC6",       ID804_CMD_SET_TEMP_LUT_TC6,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC7",       ID804_CMD_SET_TEMP_LUT_TC7,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC8",       ID804_CMD_SET_TEMP_LUT_TC8,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC9",       ID804_CMD_SET_TEMP_LUT_TC9,     ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TEMP_LUT_TC10",      ID804_CMD_SET_TEMP_LUT_TC10,    ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_TIMEOUT",            ID804_CMD_SET_TIMEOUT,          ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_12BIT,   ID804_FRAME_LEN_0BIT },
    { "SET_RGB",                ID804_CMD_SET_RGB,              ID804_BC_ABL,   ID804_MC_ABL,   ID804_FRAME_LEN_24BIT,   ID804_FRAME_LEN_0BIT },

    /* Read Commands */
    { "READ_STATUS1",           ID804_CMD_READ_STATUS1,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_STATUS2",           ID804_CMD_READ_STATUS2,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP",              ID804_CMD_READ_TEMP,            ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMPST",            ID804_CMD_READ_TEMPST,          ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_24BIT },
    { "READ_VEXT_TM",           ID804_CMD_READ_VEXT_TM,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_SETUP1",            ID804_CMD_READ_SETUP1,          ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_SETUP2",            ID804_CMD_READ_SETUP2,          ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_MCAST",             ID804_CMD_READ_MCAST,           ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_24BIT },
    { "READ_TEMPTH",            ID804_CMD_READ_TEMPTH,          ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMPHYS",           ID804_CMD_READ_TEMPHYS,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_PWM_RED_VAL",       ID804_CMD_READ_PWM_RED_VAL,     ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_PWM_GREEN_VAL",     ID804_CMD_READ_PWM_GREEN_VAL,   ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_PWM_BLUE_VAL",      ID804_CMD_READ_PWM_BLUE_VAL,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_CURR_MAX_LVL",      ID804_CMD_READ_CURR_MAX_LVL,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC1",      ID804_CMD_READ_TEMP_LUT_TC1,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC2",      ID804_CMD_READ_TEMP_LUT_TC2,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC3",      ID804_CMD_READ_TEMP_LUT_TC3,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC4",      ID804_CMD_READ_TEMP_LUT_TC4,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC5",      ID804_CMD_READ_TEMP_LUT_TC5,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC6",      ID804_CMD_READ_TEMP_LUT_TC6,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC7",      ID804_CMD_READ_TEMP_LUT_TC7,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC8",      ID804_CMD_READ_TEMP_LUT_TC8,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC9",      ID804_CMD_READ_TEMP_LUT_TC9,    ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TEMP_LUT_TC10",     ID804_CMD_READ_TEMP_LUT_TC10,   ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_TIMEOUT",           ID804_CMD_READ_TIMEOUT,         ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_12BIT },
    { "READ_RGB",               ID804_CMD_READ_RGB,             ID804_BC_ABL,   ID804_MC_DIS,   ID804_FRAME_LEN_0BIT,   ID804_FRAME_LEN_24BIT }
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief Calculate the CRC for the given data using the ID804 CRC lookup table.
 * @param p_data Pointer to the data buffer.
 * @param size Number of bytes to include in the CRC calculation.
 * @return The calculated CRC value.
 */
static uint8_t id804_calculate_crc(const uint8_t* p_data, uint16_t size)
{
    if ((p_data == NULL) || (size == 0U))
    {
        return 0U;
    }

    uint8_t crc = 0U;

    for (uint16_t i = 0U; i < size; i++)
    {
        crc ^= p_data[i];
        crc = gn_id804_crc8_LUT[crc];
    }
    //drv_uart_printf("\r\n    crc : 0x%02X", crc);
    return crc;
}

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
static uint16_t id804_build_frame_24bit(uint8_t dev_addr, uint8_t cmd, uint32_t data, uint8_t *p_out_buf, id804_crc_func_t crc_func)
{
    if (NULL == p_out_buf)
    {
        return 0U;
    }

    p_out_buf[0] = (uint8_t)(((ID804_FRAME_PREAMBLE_VAL & 0x0FU) << 4U) | ((dev_addr >> 8U) & 0x0FU));
    p_out_buf[1] = (uint8_t)(dev_addr & 0xFFU);
    p_out_buf[2] = cmd;
    p_out_buf[3] = (uint8_t)((data >> 16U) & 0xFFU);
    p_out_buf[4] = (uint8_t)((data >> 8U) & 0xFFU);
    p_out_buf[5] = (uint8_t)(data & 0xFFU);

    if (crc_func != NULL)
    {
        p_out_buf[6] = crc_func(p_out_buf, ID804_FRAME_BYTE_HEADER + ID804_FRAME_BYTE_DATA_24BIT);
        drv_uart_printf("\r\n    [ID804] [ME Write] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | Data:0x%06X | CRC:0x%02X]", \
            ID804_FRAME_PREAMBLE_VAL, dev_addr, cmd, data, p_out_buf[6]);
        return ID804_FRAME_LEN_24BIT;
    }
    else
    {
        p_out_buf[6] = 0x00U;
        drv_uart_printf("\r\n    [ID804] [ME Write] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | Data:0x%06X]", \
            ID804_FRAME_PREAMBLE_VAL, dev_addr, cmd, data);
        return (ID804_FRAME_LEN_24BIT - ID804_FRAME_BYTE_CRC);
    }
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
static uint16_t id804_build_frame_12bit(uint8_t dev_addr, uint8_t cmd, uint16_t data, uint8_t *p_out_buf, id804_crc_func_t crc_func)
{
    if (NULL == p_out_buf)
    {
        return 0U;
    }

    p_out_buf[0] = (uint8_t)(((ID804_FRAME_PREAMBLE_VAL & 0x0FU) << 4U) | ((dev_addr >> 8U) & 0x0FU));
    p_out_buf[1] = (uint8_t)(dev_addr & 0xFFU);
    p_out_buf[2] = cmd;
    p_out_buf[3] = (uint8_t)((data >> 4U) & 0xFFU);
    p_out_buf[4] = (uint8_t)(((data & 0x0FU) << 4U) | ID804_FRAME_DUMMY_VAL);

    if (crc_func != NULL)
    {
        p_out_buf[5] = crc_func(p_out_buf, ID804_FRAME_BYTE_HEADER + ID804_FRAME_BYTE_DATA_12BIT);
        drv_uart_printf("\r\n    [ID804] [ME Write] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | Data:0x%03X | CRC:0x%02X]", \
            ID804_FRAME_PREAMBLE_VAL, dev_addr, cmd, data, p_out_buf[5]);
        return ID804_FRAME_LEN_12BIT;
    }
    else
    {
        p_out_buf[5] = 0x00U;
        drv_uart_printf("\r\n    [ID804] [ME Write] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | Data:0x%03X]", \
            ID804_FRAME_PREAMBLE_VAL, dev_addr, cmd, data);
        return (ID804_FRAME_LEN_12BIT - ID804_FRAME_BYTE_CRC);
    }
}

/**
 * @brief Build a 0-bit data frame for the ID804 device.
 * @param dev_addr The device address.
 * @param cmd The command to send.
 * @param p_out_buf Pointer to the output buffer where the frame will be stored.
 * @param crc_func Function pointer to the CRC calculation function.
 * @return The length of the constructed frame.
 */
static uint16_t id804_build_frame_0bit(uint8_t dev_addr, uint8_t cmd, uint8_t *p_out_buf, id804_crc_func_t crc_func)
{
    if (NULL == p_out_buf)
    {
        return 0U;
    }

    p_out_buf[0] = (uint8_t)(((ID804_FRAME_PREAMBLE_VAL & 0x0FU) << 4U) | ((dev_addr >> 8U) & 0x0FU));
    p_out_buf[1] = (uint8_t)(dev_addr & 0xFFU);
    p_out_buf[2] = cmd;

    if (crc_func != NULL)
    {
        p_out_buf[3] = crc_func(p_out_buf, ID804_FRAME_BYTE_HEADER);
        drv_uart_printf("\r\n    [ID804] [ME Write] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | CRC:0x%02X]", \
            ID804_FRAME_PREAMBLE_VAL, dev_addr, cmd, p_out_buf[3]);
        return ID804_FRAME_LEN_0BIT;
    }
    else
    {
        p_out_buf[3] = 0x00U;
        drv_uart_printf("\r\n    [ID804] [ME Write] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X]", \
            ID804_FRAME_PREAMBLE_VAL, dev_addr, cmd);
        return (ID804_FRAME_LEN_0BIT - ID804_FRAME_BYTE_CRC);
    }
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
static bool id804_build_frame_by_cmd(uint8_t dev_addr, id804_cmd_list_t cmd, uint32_t raw_data, uint8_t *p_out_buf, id804_crc_func_t crc_func, uint16_t* frame_size)
{
    const id804_cmd_info_t *p_info = id804_get_cmd_info(cmd);

    if (NULL == p_info)
    {
        return false;
    }

    if ((ID805_BC_DEV_ADDR == dev_addr) && (true != p_info->broadcast))
    {
        drv_uart_printf("\r\n    Broadcast command not allowed for this command: 0x%02X", p_info->command);
        return false;
    }

    switch (p_info->req_bytes)
    {
        case ID804_FRAME_LEN_0BIT:
        {
            frame_size[ID804_FRAME_TX_IDX] = id804_build_frame_0bit(dev_addr, (uint8_t)p_info->command, p_out_buf, crc_func);
            break;
        }
        case ID804_FRAME_LEN_12BIT:
        {
            frame_size[ID804_FRAME_TX_IDX] = id804_build_frame_12bit(dev_addr, (uint8_t)p_info->command, (uint16_t)raw_data, p_out_buf, crc_func);
            break;
        }
        case ID804_FRAME_LEN_24BIT:
        {
            frame_size[ID804_FRAME_TX_IDX] = id804_build_frame_24bit(dev_addr, (uint8_t)p_info->command, raw_data, p_out_buf, crc_func);
            break;
        }
        default:
        {
            drv_uart_printf("\r\n    Invalid request bytes: %u", p_info->req_bytes);
            frame_size[ID804_FRAME_TX_IDX] = 0U;
            return false;
        }
    }

    switch (p_info->rsp_bytes)
    {
        case ID804_FRAME_LEN_0BIT:
        {
            if (crc_func != NULL)
            {
                frame_size[ID804_FRAME_RX_IDX] = ID804_FRAME_LEN_0BIT;
            }
            else
            {
                frame_size[ID804_FRAME_RX_IDX] = (ID804_FRAME_LEN_0BIT - ID804_FRAME_BYTE_CRC);
            }
            break;
        }
        case ID804_FRAME_LEN_12BIT:
        {
            if (crc_func != NULL)
            {
                frame_size[ID804_FRAME_RX_IDX] = ID804_FRAME_LEN_12BIT;
            }
            else
            {
                frame_size[ID804_FRAME_RX_IDX] = (ID804_FRAME_LEN_12BIT - ID804_FRAME_BYTE_CRC);
            }
            break;
        }
        case ID804_FRAME_LEN_24BIT:
        {
            if (crc_func != NULL)
            {
                frame_size[ID804_FRAME_RX_IDX] = ID804_FRAME_LEN_24BIT;
            }
            else
            {
                frame_size[ID804_FRAME_RX_IDX] = (ID804_FRAME_LEN_24BIT - ID804_FRAME_BYTE_CRC);
            }
            break;
        }
        default:
        {
            drv_uart_printf("\r\n    Invalid response bytes: %u", p_info->rsp_bytes);
            frame_size[ID804_FRAME_RX_IDX] = 0U;
            return false;
        }
    }

#ifdef ID804_RW_DEBUG
    char log_buf[MSG_BUFFER_SIZE];
    int offset = 0;

    drv_uart_printf("\r\n    Frame Build...");
    drv_uart_printf("\r\n    Length (Tx: %u / Rx: %u bytes)", frame_size[ID804_FRAME_TX_IDX], frame_size[ID804_FRAME_RX_IDX]);

    offset += snprintf(log_buf + offset, sizeof(log_buf) - offset, "\r\n    [ID804] Tx (%u B):", frame_size[ID804_FRAME_TX_IDX]);
    for (uint16_t i = 0U; i < frame_size[ID804_FRAME_TX_IDX] && offset < (int)sizeof(log_buf); ++i)
    {
        offset += snprintf(log_buf + offset, sizeof(log_buf) - offset, " 0x%02X", p_out_buf[i]);
    }
    drv_uart_printf("%s", log_buf);
#endif
    return true;
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
 * @brief Parse the received SPI packet for the ID804 device.
 * @param spi_rx_buffer Pointer to the received SPI data buffer.
 * @param spi_rx_len Length of the received SPI data buffer.
 * @return true if the packet was successfully parsed, false otherwise.
 */
static bool id804_spi_parse_rx_packet(uint8_t* spi_rx_buffer, uint16_t spi_rx_len)
{
    if ((NULL == spi_rx_buffer) || (spi_rx_len == 0U))
    {
        return false;
    }

#ifdef ID804_RW_DEBUG
    char log_buf[MSG_BUFFER_SIZE];
    int offset = 0;

    offset += snprintf(log_buf + offset, sizeof(log_buf) - offset, "\r\n    Received SPI data byte (%u bytes):", spi_rx_len);
    for (uint16_t i = 0U; i < spi_rx_len && offset < (int)sizeof(log_buf); ++i)
    {
        offset += snprintf(log_buf + offset, sizeof(log_buf) - offset, " 0x%02X", spi_rx_buffer[i]);
    }
    drv_uart_printf("%s", log_buf);
#endif

    uint32_t preamble = 0U;
    uint32_t dev_addr = 0U;
    uint32_t command = 0U;
    uint32_t data = 0U;
    // uint32_t dummy = 0U;
    uint32_t crc = 0U;
    uint32_t crc_calculated = 0U;

    switch (spi_rx_len)
    {
        case ID804_FRAME_LEN_0BIT:
        {
            preamble = (spi_rx_buffer[0] >> 4U);
            dev_addr = (((spi_rx_buffer[0] & 0x0FU) << 8U) | spi_rx_buffer[1]);
            command = spi_rx_buffer[2];
            crc = spi_rx_buffer[3];
            crc_calculated = id804_calculate_crc(spi_rx_buffer, 3U);
            if (crc == crc_calculated)
            {
                drv_uart_printf("\r\n    [ID804] [ME Read ] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | CRC:0x%02X]", preamble, dev_addr, command, crc);
            }
            else
            {
                drv_uart_printf("\r\n    [ID804] [ME Read  CRC Err] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | CRC:0x%02X | Calc:0x%02X]", preamble, dev_addr, command, crc, crc_calculated);
            }
            break;
        }
        case ID804_FRAME_LEN_12BIT:
        {
            preamble = (spi_rx_buffer[0] >> 4U);
            dev_addr = (((spi_rx_buffer[0] & 0x0FU) << 8U) | spi_rx_buffer[1]);
            command = spi_rx_buffer[2];
            data = (((spi_rx_buffer[3] << 8U) | (spi_rx_buffer[4] & 0xF0U)) >> 4U);
            // dummy = spi_rx_buffer[4] & 0x0FU;
            crc = spi_rx_buffer[5];
            crc_calculated = id804_calculate_crc(spi_rx_buffer, 5U);
            if (crc == crc_calculated)
            {
                drv_uart_printf("\r\n    [ID804] [ME Read ] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | Data:0x%03X | CRC:0x%02X]", preamble, dev_addr, command, data, crc);
            }
            else
            {
                drv_uart_printf("\r\n    [ID804] [ME Read  CRC Err] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | Data:0x%03X | CRC:0x%02X | Calc:0x%02X]", preamble, dev_addr, command, data, crc, crc_calculated);
            }
            break;
        }
        case ID804_FRAME_LEN_24BIT:
        {
            preamble = (spi_rx_buffer[0] >> 4U);
            dev_addr = (((spi_rx_buffer[0] & 0x0FU) << 8U) | spi_rx_buffer[1]);
            command = spi_rx_buffer[2];
            data = ((spi_rx_buffer[3] << 16U) | (spi_rx_buffer[4] << 8U) | (spi_rx_buffer[5] << 0U));
            crc = spi_rx_buffer[6];
            crc_calculated = id804_calculate_crc(spi_rx_buffer, 6U);
            if (crc == crc_calculated)
            {
                drv_uart_printf("\r\n    [ID804] [ME Read ] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | Data:0x%06X | CRC:0x%02X]", preamble, dev_addr, command, data, crc);
            }
            else
            {
                drv_uart_printf("\r\n    [ID804] [ME Read  CRC Err] [Pre:0x%X | Dev_addr:0x%03X | Cmd:0x%02X | Data:0x%06X | CRC:0x%02X | Calc:0x%02X]", preamble, dev_addr, command, data, crc, crc_calculated);
            }
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
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

    p_dst[dst_idx++] = 0xFFU; // Dummy header byte

    for (uint16_t i = 0U; i < src_len; ++i)
    {
        uint16_t enc_val = id804_encode_byte_to_manchester(p_src[i]);
        p_dst[dst_idx++] = (uint8_t)((enc_val >> 8U) & 0xFFU);
        p_dst[dst_idx++] = (uint8_t)((enc_val >> 0U) & 0xFFU);
    }

    if (dst_idx > ID804_SPI_TX_BUF_SIZE)
    {
        drv_uart_printf("\r\n    SPI Tx buffer overflow (dst_idx: %u, MAX: %u)", dst_idx, ID804_SPI_TX_BUF_SIZE);
        return 0U;
    }

#ifdef ID804_RW_DEBUG
    char log_buf[MSG_BUFFER_SIZE];
    int offset = 0;

    offset += snprintf(log_buf + offset, sizeof(log_buf) - offset, "\r\n    SPI Tx buffer: (%u bytes)", dst_idx);
    for (uint16_t i = 0U; i < dst_idx && offset < (int)sizeof(log_buf); ++i)
    {
        offset += snprintf(log_buf + offset, sizeof(log_buf) - offset, " 0x%02X", p_dst[i]);
    }
    drv_uart_printf("%s", log_buf);
#endif
    return dst_idx;
}

/**
 * @brief Send a command to the ID804 device via SPI.
 * @param dev_addr The device address.
 * @param cmd The command to send.
 * @param data The data associated with the command.
 */
id804_comm_result_t id804_write_via_me(uint16_t dev_addr, uint8_t cmd, uint32_t data)
{
    uint8_t raw_packet[ID804_FRAME_BUF_SIZE] = { 0U };
    uint8_t spi_tx_buffer[ID804_SPI_TX_BUF_SIZE] = { 0U };

    uint16_t frame_size[2] = { 0U };
    uint16_t spi_tx_len = 0U;

#ifdef ID804_RW_DEBUG
    drv_uart_printf("\r\n    ID804 Write via Manchester...");
    drv_uart_printf("\r\n    Device Address: 0x%04X, Command: 0x%02X, Data: 0x%08X", dev_addr, cmd, data);
#endif

    if (false == id804_build_frame_by_cmd(dev_addr, (id804_cmd_list_t)cmd, data, raw_packet, id804_calculate_crc, frame_size))
    {
        drv_uart_printf("\r\n    ID804 Write via Manchester... Failed at frame build");
        return ID804_COMM_ERR_CMD;
    }

    spi_tx_len = id804_manchester_encode_buffer(raw_packet, frame_size[ID804_FRAME_TX_IDX], spi_tx_buffer);

    if (false == drv_spi_transmit_dma(spi_tx_buffer, spi_tx_len))
    {
        drv_uart_printf("\r\n    SPI Fail at Tx");
        return ID804_COMM_ERR_SPI;
    }
    return ID804_COMM_WRITE_OK;
}

/**
 * @brief Read a command response from the ID804 device via SPI.
 * @param dev_addr The device address.
 * @param cmd The command to read.
 * @param p_data Pointer to store the received data.
 * @return The result of the communication attempt.
 */
id804_comm_result_t id804_read_via_me(uint16_t dev_addr, uint8_t cmd, uint32_t* p_data)
{
    uint8_t raw_packet[ID804_FRAME_BUF_SIZE] = { 0U };

    uint8_t spi_tx_buffer[ID804_SPI_TX_BUF_SIZE] = { 0U };
    uint8_t spi_rx_buffer[ID804_SPI_TX_BUF_SIZE] = { 0U };

    uint16_t frame_size[2] = { 0U };

    uint16_t spi_tx_len = 0U;
    uint16_t spi_rx_len = 0U;

#ifdef ID804_RW_DEBUG
    drv_uart_printf("\r\n    ID804 Read via Manchester...");
    drv_uart_printf("\r\n    Device Address: 0x%04X, Command: 0x%02X", dev_addr, cmd);
#endif

    if (false == id804_build_frame_by_cmd(dev_addr, (id804_cmd_list_t)cmd, *p_data, raw_packet, id804_calculate_crc, frame_size))
    {
        drv_uart_printf("\r\n    ID804 Read via Manchester... Failed at frame build");
        return ID804_COMM_ERR_CMD;
    }

    spi_tx_len = id804_manchester_encode_buffer(raw_packet, frame_size[ID804_FRAME_TX_IDX], spi_tx_buffer);

    spi_rx_len = frame_size[ID804_FRAME_RX_IDX];

    if (false == drv_spi_transmit_dma(spi_tx_buffer, spi_tx_len))
    {
        drv_uart_printf("\r\n    SPI Fail at Tx");
        return ID804_COMM_ERR_SPI;
    }

    if (false == drv_spi_receive_dma(spi_rx_buffer, spi_rx_len))
    {
        drv_uart_printf("\r\n    SPI Fail at Rx");
        return ID804_COMM_ERR_SPI;
    }

    if (false == id804_spi_parse_rx_packet(spi_rx_buffer, spi_rx_len))
    {
        drv_uart_printf("\r\n    SPI Fail at Parse");
        return ID804_COMM_ERR_SPI;
    }

    return ID804_COMM_READ_OK;
}
/* USER CODE END 0 */
