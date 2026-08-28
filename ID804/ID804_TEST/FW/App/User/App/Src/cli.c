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
#include "drv_uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum TAG_CLI_CMD_LIST
{
    CLI_CMD_NONE = 0,
    CLI_CMD_UNKNOWN,
    CLI_CMD_HELP,
    CLI_CMD_EXAMPLE,
    CLI_CMD_MAX
} cli_cmd_list_t;

typedef struct tag_CLI_CMD_ENTRY
{
    const char      *name;
    cli_cmd_list_t   command;
    const char      *description;
} cli_cmd_entry_t;

typedef struct tag_CLI_REQUEST
{
    cli_cmd_list_t  last_command;
    uint16_t        val_1;
    uint16_t        val_2;
    uint16_t        val_3;
    uint16_t        val_4;
} cli_request_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CLI_PROMPT      "\r\nID804> "

#define CLI_MAX_TOKENS  (5)

#define STR_MATCH       (0)
#define STR_MISMATCH    (1)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static const cli_cmd_entry_t gt_cli_command[] =
{
    { "example", CLI_CMD_EXAMPLE, "Example command" },
    { "help", CLI_CMD_HELP, "Display help information" },
    { "?", CLI_CMD_HELP, "Display help information" },
};

static cli_request_t gt_cli_request;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void cli_print_banner(void)
{
    // ANSI Escape Code를 이용해 화면을 정리하고 커서를 상단으로 옮김 (옵션)
    drv_uart_printf("\033[2J\033[H");

    drv_uart_printf("\r\n");
    drv_uart_printf("==================================================\r\n");
    drv_uart_printf("  %s\r\n", "ID804 Test Board");
    drv_uart_printf("==================================================\r\n");
    //drv_uart_printf("  * FW Version  : v%d.%d.%d\r\n", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH);
    drv_uart_printf("  * Build Date  : %s, %s\r\n", __DATE__, __TIME__);

    drv_uart_printf("==================================================\r\n");
    drv_uart_printf("  Type 'help' or '?' to view available CLI commands.\r\n");
    drv_uart_printf("==================================================\r\n\r\n");
    drv_uart_printf(CLI_PROMPT);
}

static void cli_help(void)
{
    for (uint16_t idx = 0U; idx < sizeof(gt_cli_command) / sizeof(gt_cli_command[0]); ++idx)
    {
        drv_uart_printf("\r\n\"%s\" - %s", gt_cli_command[idx].name, gt_cli_command[idx].description);
    }
}

void cli_init(void)
{
    drv_uart_init();
    cli_print_banner();
}

static void cli_command_parse(msg_buffer_t* p_msg)
{
    char* p_str = strtok(p_msg->msg, " ");
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
    if (token_cnt > 1U) gt_cli_request.val_1 = (uint16_t)strtoul(p_token[1], NULL, 0);
    if (token_cnt > 2U) gt_cli_request.val_2 = (uint16_t)strtoul(p_token[2], NULL, 0);
    if (token_cnt > 3U) gt_cli_request.val_3 = (uint16_t)strtoul(p_token[3], NULL, 0);
    if (token_cnt > 4U) gt_cli_request.val_4 = (uint16_t)strtoul(p_token[4], NULL, 0);
}

static void cli_command_execute(void)
{
    switch (gt_cli_request.last_command)
    {
        case CLI_CMD_NONE:
            break;
        case CLI_CMD_EXAMPLE:
            drv_uart_printf("\r\nExample command executed. 0x%x(%u) 0x%x(%u)", gt_cli_request.val_1, gt_cli_request.val_1, gt_cli_request.val_2, gt_cli_request.val_2);
            break;
        case CLI_CMD_HELP:
            cli_help();
            break;
        default:
            // Handle unknown command
            drv_uart_printf("\r\nUnknown CMD.");
            break;
    }
    drv_uart_printf(CLI_PROMPT);
}

void cli_process(void)
{
    if (true == drv_uart_tx_data_pending())
    {
        drv_uart_tx_dma_start(drv_uart_tx_ring_buffer_pop());
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
