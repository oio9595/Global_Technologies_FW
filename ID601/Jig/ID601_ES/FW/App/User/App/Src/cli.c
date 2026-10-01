/* USER CODE BEGIN Header */
/**
     ******************************************************************************
    * @file           : cli.c
    * @brief          : cli implementation
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
#include "cli.h"
/* 2. C standard library headers (Alphabetical order) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* 3. Project internal / System-related headers */
#include "main.h"
#include "version.h"
#include "drv_uart.h"
#include "drv_timer.h" // debug for pwm gen
#include "drv_spi.h" // debug for spi
#include "drv_gpio.h" // debug for GPIO control
#include "drv_id601.h" // debug for ID601
#include "drv_adc.h" // debug for ADC
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_CLI_CMD_LIST
{
    CLI_CMD_NONE = 0U,
    CLI_CMD_UNKNOWN,
    CLI_CMD_HELP,
    CLI_CMD_RESET,
    CLI_CMD_DEBUG_SPI8,
    CLI_CMD_DEBUG_SPI16,
    CLI_CMD_DEBUG_DELAY,
    CLI_CMD_DEBUG_ID601,
    CLI_CMD_DEBUG_TIM_GEN_SER,
    CLI_CMD_DEBUG_ADC,

    CLI_CMD_MAX
} cli_cmd_list_t;

typedef struct tag_CLI_CMD_ENTRY
{
    const char      *name;
    cli_cmd_list_t  command;
    const char      *description;
} cli_cmd_entry_t;

typedef struct tag_CLI_REQUEST
{
    cli_cmd_list_t  last_command;
    uint32_t        val_1;
    uint32_t        val_2;
    uint32_t        val_3;
    uint32_t        val_4;
} cli_request_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CLI_CLEAR_SCREEN    "\033[2J\033[H"

#define CLI_PROMPT          "\r\n\r\nID601> "

#define CLI_MAX_TOKENS      (5)

#define STR_MATCH           (0)
#define STR_MISMATCH        (1)

#define SET                 (0)
#define GET                 (1)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static const cli_cmd_entry_t gt_cli_command[] =
{
    /* ======================================================================================================= */
    /* Command              Command ID                    Description                                          */
    /* ======================================================================================================= */
    { "help",               CLI_CMD_HELP,               "Display help information"                              },
    { "?",                  CLI_CMD_HELP,               "Display help information"                              },
    { "reset",              CLI_CMD_RESET,              "Reset the system"                                      },
    { "debug_spi8",         CLI_CMD_DEBUG_SPI8,         "Debug for SPI 8-bit communication"                     },
    { "debug_spi16",        CLI_CMD_DEBUG_SPI16,        "Debug for SPI 16-bit communication"                    },
    { "debug_delay",        CLI_CMD_DEBUG_DELAY,        "Debug for delay_us/ms"                                 },
    { "debug_id601",        CLI_CMD_DEBUG_ID601,        "Debug for ID601 communication"                         },
    { "debug_tim_gen_ser",  CLI_CMD_DEBUG_TIM_GEN_SER,  "Debug for TIM generate serial"                         },
    { "debug_adc",          CLI_CMD_DEBUG_ADC,          "Debug for ADC"                                         },
};

static cli_request_t gt_cli_request;

uint16_t pwm_duty_table[22] = { 0U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U,
                                59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 0U }; // debug for pwm gen
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if 0
/**
 * @brief  Print the result of an ID804 communication attempt.
 * @param  comm_result  The result of the communication attempt.
 */
static void cli_print_id804_comm_result(id804_comm_result_t comm_result,  uint16_t dev_addr, uint16_t target, uint32_t data)
{
    switch (comm_result)
    {
        case ID804_COMM_NONE:         {drv_uart_printf("\r\n    [ID804] [Comm Result] None"); break;}
        case ID804_COMM_WRITE_OK:     {drv_uart_printf("\r\n    [ID804] [Comm Result] Write OK"); break;}
        case ID804_COMM_READ_OK:      {drv_uart_printf("\r\n    [ID804] [Comm Result] Read OK"); break;}
        case ID804_COMM_ERR_DEV_ADDR: {drv_uart_printf("\r\n    [ID804] [Comm Result] Error: Invalid Device Address [0x%04X | 0x%02X | 0x%06X]", dev_addr, target, data); break;}
        case ID804_COMM_ERR_CMD:      {drv_uart_printf("\r\n    [ID804] [Comm Result] Error: Invalid Command [0x%04X | 0x%02X | 0x%06X]", dev_addr, target, data); break;}
        case ID804_COMM_ERR_ADDR:     {drv_uart_printf("\r\n    [ID804] [Comm Result] Error: Invalid Address [0x%04X | 0x%02X | 0x%06X]", dev_addr, target, data); break;}
        case ID804_COMM_ERR_I2C:      {drv_uart_printf("\r\n    [ID804] [Comm Result] Error: I2C Failure [0x%02X | 0x%06X]", target, data); break;}
        case ID804_COMM_ERR_SPI:      {drv_uart_printf("\r\n    [ID804] [Comm Result] Error: SPI Failure [0x%04X | 0x%02X | 0x%06X]", dev_addr, target, data); break;}
        default:                      {drv_uart_printf("\r\n    [ID804] [Comm Result] Error: Unknown Error [0x%04X | 0x%02X | 0x%06X]", dev_addr, target, data); break;}
    }
}
#endif

/**
 * @brief  Print the CLI banner with firmware information.
 *         This includes build date, firmware version, and Git revision.
 *         Also displays the CLI prompt.
 */
static void cli_print_banner(void)
{
    drv_uart_printf(CLI_CLEAR_SCREEN);

    drv_uart_printf("\r\n====================================================");
    drv_uart_printf("\r\n  %s", "ID601 TEST Board");
    drv_uart_printf("\r\n====================================================");
    drv_uart_printf("\r\n  * Build Date  : %s, %s", __DATE__, __TIME__);
    drv_uart_printf("\r\n  * FW Version  : v%d.%d.%d", FW_VER_MAJOR, FW_VER_MINOR, FW_VER_BUILD);
    drv_uart_printf("\r\n  * FW Git Rev  : %s", FW_GIT_REV);

    drv_uart_printf("\r\n====================================================");
    drv_uart_printf("\r\n  Type 'help' or '?' to view available CLI commands.");
    drv_uart_printf("\r\n====================================================");
    drv_uart_printf(CLI_PROMPT);
}

/**
 * @brief  Display the help information for all available CLI commands.
 * @note   This function iterates through the command list and prints each command's name and description.
 */
static void cli_help(void)
{
    uint16_t max_name_len = 0U;
    uint16_t total_cmds = sizeof(gt_cli_command) / sizeof(gt_cli_command[0]);

    for (uint16_t idx = 0U; idx < total_cmds; ++idx)
    {
        uint16_t len = (uint16_t)strlen(gt_cli_command[idx].name);
        if (len > max_name_len)
        {
            max_name_len = len;
        }
    }

    drv_uart_printf("\r\n====================================================");
    for (uint16_t idx = 0U; idx < total_cmds; ++idx)
    {
        drv_uart_printf("\r\n\"%-*s\" - %s", max_name_len, gt_cli_command[idx].name, gt_cli_command[idx].description);
    }
    drv_uart_printf("\r\n====================================================");
}

/**
 * @brief  Initialize the CLI system, including UART and displaying the banner.
 * @note   This function should be called at the start of the application to set up the CLI environment.
 */
void cli_init(void)
{
    gt_cli_request = (cli_request_t){ CLI_CMD_NONE, 0U, 0U, 0U, 0U };
    cli_print_banner();
}

/**
 * @brief  Parse and process a CLI command from the received message buffer.
 * @param  p_msg  Pointer to the message buffer containing the received command.
 * @note   This function tokenizes the input message and updates the CLI request structure accordingly.
 */
static void cli_command_parse(msg_buffer_t* p_msg)
{
    gt_cli_request = (cli_request_t){ CLI_CMD_NONE, 0U, 0U, 0U, 0U };

    char temp[MSG_BUFFER_SIZE] = { 0U };
    memcpy(temp, p_msg->msg, MSG_BUFFER_SIZE);

    char* p_str = strtok(temp, " ");
    char* p_token[CLI_MAX_TOKENS] = { 0 };
    /* Tokenize the input message to extract the command */
    uint8_t token_cnt = 0U;
    while ((p_str != NULL) && (token_cnt < CLI_MAX_TOKENS))
    {
        p_token[token_cnt++] = p_str;
        p_str = strtok(NULL, " ");
    }

    if ((token_cnt == 0U) || (p_token[0] == NULL))
    {
        gt_cli_request.last_command = CLI_CMD_NONE;
        return;
    }

    cli_cmd_list_t command = CLI_CMD_UNKNOWN;
    const uint16_t CMD_LIST_SIZE = sizeof(gt_cli_command) / sizeof(gt_cli_command[0]);

    for (uint16_t idx = 0U; idx < CMD_LIST_SIZE; ++idx)
    {
        if (strcmp(p_token[0], gt_cli_command[idx].name) == STR_MATCH)
        {
            command = gt_cli_command[idx].command;
            break;
        }
    }

    gt_cli_request.last_command = command;
    if (token_cnt > 1U) gt_cli_request.val_1 = (uint32_t)strtoul(p_token[1], NULL, 0);
    if (token_cnt > 2U) gt_cli_request.val_2 = (uint32_t)strtoul(p_token[2], NULL, 0);
    if (token_cnt > 3U) gt_cli_request.val_3 = (uint32_t)strtoul(p_token[3], NULL, 0);
    if (token_cnt > 4U) gt_cli_request.val_4 = (uint32_t)strtoul(p_token[4], NULL, 0);
}

/**
 * @brief  Execute the parsed CLI command based on the current CLI request structure.
 * @note   This function should be called after parsing a command to perform the corresponding action.
 */
static void cli_command_execute(void)
{
    //id804_comm_result_t comm_result = ID804_COMM_NONE;
    switch (gt_cli_request.last_command)
    {
        case CLI_CMD_NONE:
        {
            break;
        }
        case CLI_CMD_HELP:
        {
            cli_help();
            break;
        }
        case CLI_CMD_RESET:
        {
            drv_uart_printf("\r\n    System resetting...");
            NVIC_SystemReset();
            break;
        }
        case CLI_CMD_DEBUG_SPI8:
        {
            bool spi_rw_type = (bool)(gt_cli_request.val_1);
            if (false == spi_rw_type)
            {
                uint8_t tx_buffer[3] = { 0x3FU, 0xA5U, 0x78U };
                uint8_t rx_buffer[3] = { 0U };

                drv_gpio_ic603_cs(false);
                if (false == drv_spi_receive_dma_8bit(SPI1, tx_buffer, rx_buffer, 3U, 20U))
                {
                    drv_gpio_ic603_cs(true);
                    drv_uart_printf("\r\n    SPI 8-bit read NG...");
                }
                drv_uart_printf("\r\n    SPI 8-bit read OK...");
            }
            else
            {
                uint8_t tx_buffer[3] = { 0x3FU, 0xA5U, 0x78U };

                drv_gpio_ic603_cs(false);
                if (false == drv_spi_transmit_dma_8bit(SPI1, tx_buffer, 3U, 20U))
                {
                    drv_gpio_ic603_cs(true);
                    drv_uart_printf("\r\n    SPI 8-bit write NG...");
                }
                drv_uart_printf("\r\n    SPI 8-bit write OK...");
            }
            break;
        }
        case CLI_CMD_DEBUG_SPI16:
        {
            bool spi_rw_type = (bool)(gt_cli_request.val_1);
            if (false == spi_rw_type)
            {
                uint16_t tx_buffer[3] = { 0x3F70U, 0x98A5U, 0x718CU };
                uint16_t rx_buffer[3] = { 0U };

                drv_gpio_ads114s08_dev_all_cs(false);
                if (false == drv_spi_receive_dma_16bit(SPI2, tx_buffer, rx_buffer, 3U, 20U))
                {
                    drv_gpio_ads114s08_dev_all_cs(true);
                    drv_uart_printf("\r\n    SPI 16-bit read NG...");
                }
                drv_uart_printf("\r\n    SPI 16-bit read OK...");
            }
            else
            {
                uint16_t tx_buffer[3] = { 0x3F70U, 0x98A5U, 0x718CU };

                drv_gpio_ads114s08_dev_all_cs(false);
                if (false == drv_spi_transmit_dma_16bit(SPI2, tx_buffer, 3U, 20U))
                {
                    drv_gpio_ads114s08_dev_all_cs(true);
                    drv_uart_printf("\r\n    SPI 16-bit write NG...");
                }
                drv_uart_printf("\r\n    SPI 16-bit write OK...");
            }
            break;
        }
        case CLI_CMD_DEBUG_DELAY:
        {
            bool delay_us_ms = (bool)(gt_cli_request.val_1);
            if (false == delay_us_ms)
            {
                LL_GPIO_ResetOutputPin(IC603_CS_GPIO_Port, IC603_CS_Pin);
                drv_tim_delay_us(10); // Example delay in microseconds
                LL_GPIO_SetOutputPin(IC603_CS_GPIO_Port, IC603_CS_Pin);
                drv_uart_printf("\r\n    Delay 10 us...");
            }
            else
            {
                LL_GPIO_ResetOutputPin(IC603_CS_GPIO_Port, IC603_CS_Pin);
                drv_tim_delay_ms(10); // Example delay in milliseconds
                LL_GPIO_SetOutputPin(IC603_CS_GPIO_Port, IC603_CS_Pin);
                drv_uart_printf("\r\n    Delay 10 ms...");
            }
            break;
        }
        case CLI_CMD_DEBUG_ID601:
        {
            /*
            bool serial_rw = (bool)(gt_cli_request.val_1);
            if (false == serial_rw)
            {
                //id601_read(0U, 0U);
                drv_uart_printf("\r\n    ID601 read executed...");
            }
            else
            {
                //id601_write(0U, 0U);
                drv_uart_printf("\r\n    ID601 write executed...");
            }
            */
            id601_reset();
            break;
        }
        case CLI_CMD_DEBUG_TIM_GEN_SER:
        {
            uint16_t temp[62] = { 0U,
                119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U,
                119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U,
                119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U,
                119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U,
                119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U,
                119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U, 119U, 59U,
                0U };
            drv_tim_generate_serial((uint16_t*)temp, 62U, 100U);
            break;
        }
        case CLI_CMD_DEBUG_ADC:
        {
            mcu_adc_set_conversion_enable(true);
            if (true == mcu_adc_wait_conversion_complete())
            {
                uint32_t adc_value = mcu_adc_get_average_conversion();
                drv_uart_printf("\r\n    ADC conversion complete. %lu", adc_value);
            }
            else
            {
                drv_uart_printf("\r\n    ADC conversion timeout.");
            }
            break;
        }

        default:
        {
            // Handle unknown command
            drv_uart_printf("\r\n    Unknown CMD.");
            break;
        }
    }
    drv_uart_printf(CLI_PROMPT);
}

/**
 * @brief  Process the CLI by handling UART TX and RX data.
 * @note   This function should be called periodically to manage CLI input and output.
 */
void cli_process(void)
{
    if (true == drv_uart_tx_data_pending())
    {
        //drv_uart_tx_dma_start(drv_uart_tx_ring_buffer_pop());
        msg_buffer_t* p_msg = drv_uart_tx_ring_buffer_pop();
        drv_uart_printf_direct((const uint8_t*)p_msg->msg, p_msg->size);

    }

    if (true == drv_uart_rx_data_pending())
    {
        msg_buffer_t* p_msg = drv_uart_rx_ring_buffer_pop();

        if (p_msg != NULL)
        {
            cli_command_parse(p_msg);
            cli_command_execute();
        }
    }
}
/* USER CODE END 0 */
