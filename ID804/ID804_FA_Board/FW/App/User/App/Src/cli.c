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
#include "drv_spi.h"
#include "drv_i2c.h"
#include "drv_gpio.h"
#include "drv_id804.h"
#include "id804_metadata.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_CLI_CMD_LIST
{
    CLI_CMD_NONE    = 0,
    CLI_CMD_UNKNOWN,
    CLI_CMD_HELP,
    CLI_CMD_RESET,
    CLI_CMD_DEBUG_SPI,
    CLI_CMD_ID804_VDD,
    CLI_CMD_ID804_VLED,
    CLI_CMD_ID804_TM0,
    CLI_CMD_ID804_SIO1,
    CLI_CMD_ID804_SIO2,
    CLI_CMD_ID804_BOOT,
    CLI_CMD_ID804_ME,
    CLI_CMD_ID804_I2C,
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

//#define CLI_PROMPT          "\r\nID804> "
#define CLI_PROMPT          "\r\n\r\nID804> "

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
    { "help", CLI_CMD_HELP, "Display help information" },
    { "?", CLI_CMD_HELP, "Display help information" },
    { "reset", CLI_CMD_RESET, "Reset the system" },
    { "debug_spi", CLI_CMD_DEBUG_SPI, "Debug SPI interface" },

    { "id804_vdd", CLI_CMD_ID804_VDD, "Control ID804 VDD <0: OFF, 1: 5V, 2: 5.5V>" },
    { "id804_vled", CLI_CMD_ID804_VLED, "Control ID804 VLED <0: OFF, 1: ON>" },
    { "id804_tm0", CLI_CMD_ID804_TM0, "Control ID804 TM0 <0: GND, 1: VDD>" },

    { "id804_sio1", CLI_CMD_ID804_SIO1, "Control ID804 SIO1 interface <1: MCU, 2: CAN, 3: LVDS, 4: I2C> <1: ENABLE, 0: DISABLE>" },
    { "id804_sio2", CLI_CMD_ID804_SIO2, "Control ID804 SIO2 interface <1: MCU, 2: CAN, 3: LVDS, 4: EOL> <1: ENABLE, 0: DISABLE>" },

    { "id804_boot", CLI_CMD_ID804_BOOT, "Control ID804 boot process <0: MCU, 1: I2C>" },

    { "id804_me", CLI_CMD_ID804_ME,   "Control ID804 ME interface  <0: WRITE, 1: READ> <Dev Addr> <CMD> <Data: only for WRITE>" },
    { "id804_i2c", CLI_CMD_ID804_I2C, "Control ID804 I2C interface <0: WRITE, 1: READ> <Address>  <Data: only for WRITE>" },
};

static cli_request_t gt_cli_request;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief  Print the CLI banner with firmware information.
 *         This includes build date, firmware version, and Git revision.
 *         Also displays the CLI prompt.
 */
static void cli_print_banner(void)
{
    // ANSI Escape Code를 이용해 화면을 정리하고 커서를 상단으로 옮김 (옵션)
    drv_uart_printf(CLI_CLEAR_SCREEN);

    drv_uart_printf("\r\n====================================================");
    drv_uart_printf("\r\n  %s", "ID804 Test Board");
    drv_uart_printf("\r\n====================================================");
    drv_uart_printf("\r\n  * Build Date  : %s, %s", __DATE__, __TIME__);
    drv_uart_printf("\r\n  * FW Version  : v%d.%d.%d", FW_MAJOR, FW_MINOR, FW_BUILD);
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

/**
 * @brief  Execute the parsed CLI command based on the current CLI request structure.
 * @note   This function should be called after parsing a command to perform the corresponding action.
 */
static void cli_command_execute(void)
{
    switch (gt_cli_request.last_command)
    {
        case CLI_CMD_NONE:
        {
            break;
        }
        case CLI_CMD_DEBUG_SPI:
        {
            uint8_t spi_data[4] = { gt_cli_request.val_1, gt_cli_request.val_2, gt_cli_request.val_3, gt_cli_request.val_4 };
            drv_uart_printf("\r\n    Debug SPI executed. (%u) (%u) (%u) (%u)", gt_cli_request.val_1, gt_cli_request.val_2, gt_cli_request.val_3, gt_cli_request.val_4);
            drv_spi_transmit_direct(spi_data, 4);
            break;
        }
        case CLI_CMD_ID804_VDD:
        {
            if (0U == gt_cli_request.val_1)
            {
                drv_gpio_id804_vcc(ID804_VCC_OFF);
                drv_uart_printf("\r\n    ID804 VDD turned OFF.");
            }
            else if (1U == gt_cli_request.val_1)
            {
                drv_gpio_id804_vcc(ID804_VCC_5V0);
                drv_uart_printf("\r\n    ID804 VDD set to 5V.");
            }
            else if (2U == gt_cli_request.val_1)
            {
                drv_gpio_id804_vcc(ID804_VCC_5V5);
                drv_uart_printf("\r\n    ID804 VDD set to 5.5V.");
            }
            else
            {
                drv_gpio_id804_vcc(ID804_VCC_OFF);
                drv_uart_printf("\r\n    Invalid VDD value. Use 0 for OFF, 1 for 5V, 2 for 5.5V.");
            }
            break;
        }
        case CLI_CMD_ID804_VLED:
        {
            if (0U == gt_cli_request.val_1)
            {
                drv_gpio_id804_vled(ID804_VLED_OFF);
                drv_uart_printf("\r\n    ID804 VLED turned OFF.");
            }
            else if (1U == gt_cli_request.val_1)
            {
                drv_gpio_id804_vled(ID804_VLED_ON);
                drv_uart_printf("\r\n    ID804 VLED turned ON.");
            }
            else
            {
                drv_gpio_id804_vled(ID804_VLED_OFF);
                drv_uart_printf("\r\n    Invalid VLED value. Use 0 for OFF, 1 for ON.");
            }
            break;
        }
        case CLI_CMD_ID804_TM0:
        {
            if (0U == gt_cli_request.val_1)
            {
                drv_gpio_id804_tm0_to_GND();
                drv_uart_printf("\r\n    ID804 TM0 set to GND.");
            }
            else if (1U == gt_cli_request.val_1)
            {
                drv_gpio_id804_tm0_to_VDD();
                drv_uart_printf("\r\n    ID804 TM0 set to VDD.");
            }
            else
            {
                drv_uart_printf("\r\n    Invalid TM0 value. Use 0 for GND (MCU), 1 for VDD (I2C).");
            }
            break;
        }
        case CLI_CMD_ID804_SIO1:
        {
            if (gt_cli_request.val_2 != 0U && gt_cli_request.val_2 != 1U)
            {
                drv_uart_printf("\r\n    Invalid enable/disable value. Use 1 for ENABLE, 0 for DISABLE.");
                break;
            }
            drv_gpio_id804_sio1_mcu(ID804_IO_DIS);
            drv_gpio_id804_sio1_can(ID804_IO_DIS);
            drv_gpio_id804_sio1_lvds(ID804_IO_DIS);
            drv_gpio_id804_sio1_i2c(ID804_IO_DIS);
            switch (gt_cli_request.val_1)
            {
                case 1U:
                    // Handle SIO1 MCU
                    drv_gpio_id804_sio1_mcu(gt_cli_request.val_2 ? ID804_IO_CON : ID804_IO_DIS);
                    drv_uart_printf("\r\n    ID804 SIO1 MCU set to %s.", gt_cli_request.val_2 ? "ENABLE" : "DISABLE");
                    break;
                case 2U:
                    // Handle SIO1 CAN
                    drv_gpio_id804_sio1_can(gt_cli_request.val_2 ? ID804_IO_CON : ID804_IO_DIS);
                    drv_uart_printf("\r\n    ID804 SIO1 CAN set to %s.", gt_cli_request.val_2 ? "ENABLE" : "DISABLE");
                    break;
                case 3U:
                    // Handle SIO1 LVDS
                    drv_gpio_id804_sio1_lvds(gt_cli_request.val_2 ? ID804_IO_CON : ID804_IO_DIS);
                    drv_uart_printf("\r\n    ID804 SIO1 LVDS set to %s.", gt_cli_request.val_2 ? "ENABLE" : "DISABLE");
                    break;
                case 4U:
                    // Handle SIO1 I2C
                    drv_gpio_id804_sio1_i2c(gt_cli_request.val_2 ? ID804_IO_CON : ID804_IO_DIS);
                    drv_uart_printf("\r\n    ID804 SIO1 I2C set to %s.", gt_cli_request.val_2 ? "ENABLE" : "DISABLE");
                    break;
                default:
                    drv_uart_printf("\r\n    Unknown SIO1 sub-command.");
                    break;
            }
            break;
        }
        case CLI_CMD_ID804_SIO2:
        {
            if (gt_cli_request.val_2 != 0U && gt_cli_request.val_2 != 1U)
            {
                drv_uart_printf("\r\n    Invalid enable/disable value. Use 1 for ENABLE, 0 for DISABLE.");
                break;
            }
            drv_gpio_id804_sio2_mcu(ID804_IO_DIS);
            drv_gpio_id804_sio2_can(ID804_IO_DIS);
            drv_gpio_id804_sio2_lvds(ID804_IO_DIS);
            drv_gpio_id804_sio2_eol(ID804_IO_DIS);
            switch (gt_cli_request.val_1)
            {
                case 1U:
                    // Handle SIO2 MCU
                    drv_gpio_id804_sio2_mcu(gt_cli_request.val_2 ? ID804_IO_CON : ID804_IO_DIS);
                    drv_uart_printf("\r\n    ID804 SIO2 MCU set to %s.", gt_cli_request.val_2 ? "ENABLE" : "DISABLE");
                    break;
                case 2U:
                    // Handle SIO2 CAN
                    drv_gpio_id804_sio2_can(gt_cli_request.val_2 ? ID804_IO_CON : ID804_IO_DIS);
                    drv_uart_printf("\r\n    ID804 SIO2 CAN set to %s.", gt_cli_request.val_2 ? "ENABLE" : "DISABLE");
                    break;
                case 3U:
                    // Handle SIO2 LVDS
                    drv_gpio_id804_sio2_lvds(gt_cli_request.val_2 ? ID804_IO_CON : ID804_IO_DIS);
                    drv_uart_printf("\r\n    ID804 SIO2 LVDS set to %s.", gt_cli_request.val_2 ? "ENABLE" : "DISABLE");
                    break;
                case 4U:
                    // Handle SIO2 EOL
                    drv_gpio_id804_sio2_eol(gt_cli_request.val_2 ? ID804_IO_CON : ID804_IO_DIS);
                    drv_uart_printf("\r\n    ID804 SIO2 EOL set to %s.", gt_cli_request.val_2 ? "ENABLE" : "DISABLE");
                    break;
                default:
                    drv_uart_printf("\r\n    Unknown SIO2 sub-command.");
                    break;
            }
            break;
        }
        case CLI_CMD_ID804_BOOT:
        {
            if (0U == gt_cli_request.val_1)
            {
                id804_boot_me();
                drv_uart_printf("\r\n    ID804 boot mcu executed.");
            }
            else if (1U == gt_cli_request.val_1)
            {
                id804_boot_i2c();
                drv_uart_printf("\r\n    ID804 boot i2c executed.");
            }
            else
            {
                drv_uart_printf("\r\n    ID804 boot command not recognized.");
            }
            break;
        }
        case CLI_CMD_ID804_ME:
        {
            uint16_t dev_addr = (uint16_t)gt_cli_request.val_2;
            uint16_t target = (uint16_t)gt_cli_request.val_3;
            uint32_t data = gt_cli_request.val_4;

            if (GET == gt_cli_request.val_1)
            {
                if (true == id804_read(dev_addr, target, &data))
                {
                    drv_uart_printf("\r\n    ID804 ME read successful. Addr: 0x%02X, Data: 0x%03X", target, data);
                }
                else
                {
                    drv_uart_printf("\r\n    ID804 ME read failed. Addr: 0x%02X, Data: 0x%03X", target, data);
                }
            }
            else if (SET == gt_cli_request.val_1)
            {
                if (true == id804_write(dev_addr, target, data))
                {
                    drv_uart_printf("\r\n    ID804 ME write successful. Addr: 0x%02X, Data: 0x%03X", target, data);
                }
                else
                {
                    drv_uart_printf("\r\n    ID804 ME write failed. Addr: 0x%02X, Data: 0x%03X", target, data);
                }
            }
            else
            {
                drv_uart_printf("\r\n    ID804 ME communication not recognized.");
            }
            break;
        }
        case CLI_CMD_ID804_I2C:
        {
            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = (uint16_t)gt_cli_request.val_2;
            uint32_t data = gt_cli_request.val_3;

            if (GET == gt_cli_request.val_1)
            {
                if (true == id804_read(dev_addr, target, &data))
                {
                    drv_uart_printf("\r\n    ID804 I2C read successful. Addr: 0x%02X, Data: 0x%03X", target, data);
                }
                else
                {
                    drv_uart_printf("\r\n    ID804 I2C read failed. Addr: 0x%02X, Data: 0x%03X", target, data);
                }
            }
            else if (SET == gt_cli_request.val_1)
            {
                if (true == id804_write(dev_addr, target, data))
                {
                    drv_uart_printf("\r\n    ID804 I2C write successful. Addr: 0x%02X, Data: 0x%03X", target, data);
                }
                else
                {
                    drv_uart_printf("\r\n    ID804 I2C write failed. Addr: 0x%02X, Data: 0x%03X", target, data);
                }
            }
            else
            {
                drv_uart_printf("\r\n    ID804 I2C communication not recognized.");
            }
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
