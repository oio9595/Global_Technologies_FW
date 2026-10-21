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
#include "cmsis_os.h"
#include "version.h"
#include "drv_uart.h"
#include "drv_gpio.h"
#include "drv_id601.h"
#include "id601_test.h"
#include "id601_trim.h"
#include "common_def.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_CLI_CMD_LIST
{
    CLI_CMD_NONE = 0U,
    CLI_CMD_UNKNOWN,
    CLI_CMD_HELP,
    CLI_CMD_RESET,

    CLI_CMD_TRIM_START,
    CLI_CMD_TRIM_RESULT,
    CLI_CMD_TEST_START,
    CLI_CMD_TEST_ONE_CHANNEL,

    CLI_CMD_ID601_POWER,
    CLI_CMD_ID601_DAISY,
    CLI_CMD_ID601_COMMAND,
    CLI_CMD_ID601_WRITE,
    CLI_CMD_ID601_READ,
    CLI_CMD_ID601_DUMP,
    CLI_CMD_ID601_INIT,
    CLI_CMD_ID601_EFUSE,

    CLI_CMD_MAX
} cli_cmd_list_t;

typedef struct tag_CLI_CMD_ENTRY
{
    cli_cmd_list_t  command;
    const char      *name;
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
    {
        CLI_CMD_HELP,
        "help",
        "Display available commands and usage"
    },
    {
        CLI_CMD_HELP,
        "?",
        "Display available commands and usage"
    },
    {
        CLI_CMD_RESET,
        "reset",
        "Reset the MCU"
    },
    {
        CLI_CMD_TRIM_START,
        "trim_start",
        "Request the ID601 trim process to start"
    },
    {
        CLI_CMD_TRIM_RESULT,
        "trim_result",
        "Display the ID601 trim process result"
    },
    {
        CLI_CMD_TEST_START,
        "test_start",
        "test_start <sweep>"
        "Request the ID601 test process to start"
        "\r\n    sweep: 1=enable, else=skip"
    },
    {
        CLI_CMD_TEST_ONE_CHANNEL,
        "test_one_channel",
        "test_one_channel <option> <channel> <max_curr_lvl> <vref>"
        "Request the ID601 test process for one channel to start"
        "\r\n    option: 0=stop, 1=start"
        "\r\n    channel: Target channel number (1 ~ 12)"
        "\r\n    max_curr_lvl: Maximum current level"
        "\r\n    vref: Reference voltage"
    },
    {
        CLI_CMD_ID601_POWER,
        "id601_power",
        "id601_power <target> <state>"
        "\r\n    target 0: VCC  - state 0=OFF, 1=3.3V, 2=5.5V"
        "\r\n    target 1: VLED - state 0=OFF, 1=ON (voltage set by SW14/SW15)"
        "\r\n    Invalid state turns the selected power OFF."
    },
    {
        CLI_CMD_ID601_COMMAND,
        "id601_command",
        "id601_command [command]"
        "\r\n    command: 0=syncgen, 1=idgen"
    },
    {
        CLI_CMD_ID601_DAISY,
        "id601_daisy",
        "id601_daisy [length]"
        "\r\n    Leave the \"length\" field empty or enter 0 to display the current length."
        "\r\n    Enter 1-5 to set the number of connected ID601 devices."
    },
    {
        CLI_CMD_ID601_WRITE,
        "id601_write",
        "id601_write <bank> <ic> <addr> <data>"
        "\r\n    bank: 0=General, 1=Mirror"
        "\r\n          Bank switching is handled automatically."
        "\r\n    ic: Target IC number; must not exceed daisy length. 0 is not allowed."
        "\r\n    addr: General 0x00-0x3F, Mirror 0x00-0x1E"
        "\r\n    data: 0x000-0xFFF; numbers: decimal or 0x-prefixed hex"
    },
    {
        CLI_CMD_ID601_READ,
        "id601_read",
        "id601_read <bank> <ic> <addr>"
        "\r\n    bank: 0=General, 1=Mirror"
        "\r\n          Bank switching is handled automatically."
        "\r\n    ic: Target IC number; must not exceed daisy length. 0 is not allowed."
        "\r\n    addr: General 0x00-0x3F, Mirror 0x00-0x1E"
        "\r\n    Display the selected IC register value in decimal."
    },
    {
        CLI_CMD_ID601_DUMP,
        "id601_dump",
        "Display the ID601 register dump"
    },
    {
        CLI_CMD_ID601_INIT,
        "id601_init",
        "id601_init <mode>"
        "\r\n    Leave the \"mode\" field empty or enter 0 to display the possible mode list."
        "\r\n    mode: 0=display list, 1=Testmode Entry, 2=Analog Test, 3=FLL Test, 4=After EFUSE"
    },
    {
        CLI_CMD_ID601_EFUSE,
        "id601_efuse",
        "id601_efuse <enable>"
        "\r\n    enable: 0=disable, 1=enable"
    }
};

static cli_request_t gt_cli_request;
extern osThreadId_t ID601TrimTaskHandle;
extern osThreadId_t ID601TestTaskHandle;
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
    drv_uart_printf(LOG_LV_INFO, CLI_CLEAR_SCREEN);

    drv_uart_printf(LOG_LV_INFO, "\r\n====================================================");
    drv_uart_printf(LOG_LV_INFO, "\r\n  %s", "ID601 TEST Board");
    drv_uart_printf(LOG_LV_INFO, "\r\n====================================================");
    drv_uart_printf(LOG_LV_INFO, "\r\n  * Build Date  : %s, %s", __DATE__, __TIME__);
    drv_uart_printf(LOG_LV_INFO, "\r\n  * FW Version  : v%d.%d.%d", FW_VER_MAJOR, FW_VER_MINOR, FW_VER_BUILD);
    drv_uart_printf(LOG_LV_INFO, "\r\n  * FW Git Rev  : %s", FW_GIT_REV);
    drv_uart_printf(LOG_LV_INFO, "\r\n  * FW Log Level: %s", drv_uart_get_log_level());

    drv_uart_printf(LOG_LV_INFO, "\r\n====================================================");
    drv_uart_printf(LOG_LV_INFO, "\r\n  Type 'help' or '?' to view available CLI commands.");
    drv_uart_printf(LOG_LV_INFO, "\r\n====================================================");
    drv_uart_printf(LOG_LV_INFO, CLI_PROMPT);
}

/**
 * @brief  Display the help information for all available CLI commands.
 * @note   This function iterates through the command list and prints each command's name and description.
 */
static void cli_help(void)
{
    uint16_t max_name_len = 0U;
    const uint16_t total_cmds = sizeof(gt_cli_command) / sizeof(gt_cli_command[0]);

    for (uint16_t idx = 0U; idx < total_cmds; ++idx)
    {
        const uint16_t len = (uint16_t)strlen(gt_cli_command[idx].name);
        if (max_name_len < len)
        {
            max_name_len = len;
        }
    }

    const int description_indent = (int)max_name_len + 5;

    drv_uart_printf(LOG_LV_INFO, "\r\n====================================================");

    for (uint16_t idx = 0U; idx < total_cmds; ++idx)
    {
        const char *line = gt_cli_command[idx].description;
        bool first_line = true;

        while ('\0' != *line)
        {
            const size_t line_len = strcspn(line, "\r\n");

            if (true == first_line)
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n\"%-*s\" - %.*s", (int)max_name_len, gt_cli_command[idx].name, (int)line_len, line);
                first_line = false;
            }
            else
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n%*s%.*s", description_indent, "", (int)line_len, line);
            }

            line += line_len;

            if ('\r' == *line)
            {
                ++line;
            }
            if ('\n' == *line)
            {
                ++line;
            }

            /* 기존 설명 문자열의 들여쓰기 제거 */
            line += strspn(line, " \t");
        }
    }

    drv_uart_printf(LOG_LV_INFO, "\r\n====================================================");
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
    while ((NULL != p_str) && (token_cnt < CLI_MAX_TOKENS))
    {
        p_token[token_cnt++] = p_str;
        p_str = strtok(NULL, " ");
    }

    if ((0U == token_cnt) || (NULL == p_token[0]))
    {
        gt_cli_request.last_command = CLI_CMD_NONE;
        return;
    }

    cli_cmd_list_t command = CLI_CMD_UNKNOWN;
    const uint16_t CMD_LIST_SIZE = sizeof(gt_cli_command) / sizeof(gt_cli_command[0]);

    for (uint16_t idx = 0U; idx < CMD_LIST_SIZE; ++idx)
    {
        if (STR_MATCH == strcmp(p_token[0], gt_cli_command[idx].name))
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

static inline void cli_cmd_function_none(void)
{

}
static inline void cli_cmd_function_help(void)
{
    cli_help();
}
static inline void cli_cmd_function_reset(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    System resetting...");
    NVIC_SystemReset();
}
static inline void cli_cmd_function_trim_start(void)
{
    osThreadFlagsSet(ID601TrimTaskHandle, TRIM_START_FLAG);
}
static inline void cli_cmd_function_trim_result(void)
{
    id601_trim_display_search_registers();
}
static inline void cli_cmd_function_test_start(void)
{
    const uint32_t option = gt_cli_request.val_1;
    if (option > 1U)
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    Usage: test_start [0|1]");
        return;
    }
    test_set_vref_sweep_enable(1U == option);
    osThreadFlagsSet(ID601TestTaskHandle, TEST_START_FLAG);
}
static inline void cli_cmd_function_test_one_channel(void)
{
    const uint32_t option = gt_cli_request.val_1;
    const uint32_t channel = gt_cli_request.val_2;
    const uint32_t max_curr_lvl = gt_cli_request.val_3;
    const uint32_t vref = gt_cli_request.val_4;

    if ((option > 1U) || (channel > ID601_CH_MAX) || (channel < 1U) || (max_curr_lvl > 0x0F) || (vref > 4095))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    Usage: test_one_channel <option> <channel> <max_curr_lvl> <vref>");
        return;
    }

    if (1U == option)
    {
        id601_test_iout_one_channel_start((uint16_t)(channel - 1U), (uint16_t)max_curr_lvl, (uint16_t)vref);
    }
    else
    {
        id601_test_iout_one_channel_stop();
    }
}
static inline void cli_cmd_function_id601_power(void)
{
    const uint16_t val1 = (uint16_t)gt_cli_request.val_1;
    const uint16_t val2 = (uint16_t)gt_cli_request.val_2;

    switch (val1)
    {
        case 0U:
        {
            if (0U == val2)
            {
                drv_gpio_id601_vcc(ID601_VCC_OFF);
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 VCC turned off.");
            }
            else if (1U == val2)
            {
                drv_gpio_id601_vcc(ID601_VCC_3V3);
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 VCC set to 3.3V.");
            }
            else if (2U == val2)
            {
                drv_gpio_id601_vcc(ID601_VCC_5V5);
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 VCC set to 5.5V.");
            }
            else
            {
                drv_gpio_id601_vcc(ID601_VCC_OFF);
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %u is invalid, ID601 VCC turned off.", val2);
            }
            break;
        }
        case 1U:
        {
            if (0U == val2)
            {
                drv_gpio_id601_vled(ID601_VLED_OFF);
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 VLED turned off.");
            }
            else if (1U == val2)
            {
                drv_gpio_id601_vled(ID601_VLED_ON);
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 VLED turned on. Level depends on SW14, SW15.");
            }
            else
            {
                drv_gpio_id601_vled(ID601_VLED_OFF);
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %u is invalid, ID601 VLED turned off.", val2);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
static inline void cli_cmd_function_id601_daisy(void)
{
    const uint16_t daisy = (uint16_t)gt_cli_request.val_1;
    if (0U == daisy)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 daisy chain length is %u.", drv_id601_get_daisy_length());
    }
    else
    {
        if (true == drv_id601_set_daisy_length(daisy))
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 daisy chain length set to %u.", daisy);
        }
        else
        {
            drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to set ID601 daisy chain length to %u.", daisy);
        }
    }
}
static inline void cli_cmd_function_id601_command(void)
{
    const uint16_t command = (uint16_t)gt_cli_request.val_1;
    if (0U == command)
    {
        id601_sync_gen(drv_id601_get_crc_func());
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 syncgen command executed.");
    }
    else if (1U == command)
    {
        id601_id_gen(drv_id601_get_crc_func());
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 idgen command executed.");
    }
    else
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [CLI] Unknown ID601 command: %u.", command);
    }
}
static inline void cli_cmd_function_id601_write(void)
{
    const uint16_t bank = (uint16_t)gt_cli_request.val_1;
    const uint16_t ic_num = (uint16_t)gt_cli_request.val_2;
    const uint16_t addr = (uint16_t)gt_cli_request.val_3;
    const uint16_t data = (uint16_t)gt_cli_request.val_4;

    USER_ASSERT(bank < ID601_REGISTER_BANK_COUNT);
    USER_ASSERT(ic_num > 0U);
    USER_ASSERT(addr < ID601_ADDR_GENERAL_MAX_COUNT);
    USER_ASSERT(data < 4096);

    if (0U == drv_id601_get_daisy_length())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Daisy chain length is 0, cannot write to ID601.");
        return;
    }

    if (false == id601_write_register(bank, ic_num, addr, data, drv_id601_get_crc_func()))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
    }
    else
    {
        if (ID601_REGISTER_BANK_GENERAL == bank)
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Successfully wrote to ID601 General register. (IC: %u, Addr: %u, Data: %u, 0x%03X )", ic_num, addr, data, data);
        }
        else
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Successfully wrote to ID601 Mirror register. (IC: %u, Addr: %u, Data: %u, 0x%03X)", ic_num, addr, data, data);
        }
    }
}
static inline void cli_cmd_function_id601_read(void)
{
    const uint16_t bank = (uint16_t)gt_cli_request.val_1;
    const uint16_t ic_num = (uint16_t)gt_cli_request.val_2;
    const uint16_t addr = (uint16_t)gt_cli_request.val_3;

    USER_ASSERT(bank < ID601_REGISTER_BANK_COUNT);
    USER_ASSERT(ic_num > 0U);
    USER_ASSERT(addr < ID601_ADDR_GENERAL_MAX_COUNT);

    if (0U == drv_id601_get_daisy_length())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Daisy chain length is 0, cannot read from ID601.");
        return;
    }

    uint16_t read_data = 0U;
    if (false == id601_read_register(bank, ic_num, addr, drv_id601_get_crc_func(), &read_data))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to read from ID601 register.");
    }
    else
    {
        if (ID601_REGISTER_BANK_GENERAL == bank)
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Successfully read from ID601 General register. (IC: %u, Addr: %u, Data: %u, 0x%03X)", ic_num, addr, read_data, read_data);
        }
        else
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Successfully read from ID601 Mirror register. (IC: %u, Addr: %u, Data: %u, 0x%03X)", ic_num, addr, read_data, read_data);
        }
    }
}
static inline void cli_cmd_function_id601_dump(void)
{
    id601_dump_registers();
}
static inline void cli_cmd_function_id601_init(void)
{
    const uint16_t mode = (uint16_t)gt_cli_request.val_1;

    USER_ASSERT(mode <= 4U); // 1=Testmode Entry, 2=Analog Test, 3=FLL Test, 4=After EFUSE

    typedef bool (*id601_init_func_t)(void);
    const id601_init_func_t init_funcs[4] =
    {
        id601_init_testmode_entry,
        id601_init_analog_test_mode,
        id601_init_fll_test_mode,
        id601_init_after_efuse_mode
    };

    if (0U == mode)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 mode list: 1=Testmode Entry, 2=Analog Test, 3=FLL Test, 4=After EFUSE.");
    }
    else
    {
        switch (mode)
        {
            case 1U:
            case 2U:
            case 3U:
            case 4U:
            {
                if (true == init_funcs[mode - 1U]())
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Successfully initialized ID601 in mode %u.", mode);
                }
                else
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to initialize ID601 in mode %u.", mode);
                }
                break;
            }
            default:
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %u mode is invalid.", mode);
                break;
            }
        }
    }
}
static inline void cli_cmd_function_id601_efuse(void)
{
    const uint16_t enable = (uint16_t)gt_cli_request.val_1;
    if (enable > 1U)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Invalid EFUSE enable value: %u", enable);
        return;
    }
    id601_set_efuse_burn_enable((bool)enable);
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
            cli_cmd_function_none();
            break;
        }
        case CLI_CMD_HELP:
        {
            cli_cmd_function_help();
            break;
        }
        case CLI_CMD_RESET:
        {
            cli_cmd_function_reset();
            break;
        }
        case CLI_CMD_TRIM_START:
        {
            cli_cmd_function_trim_start();
            break;
        }
        case CLI_CMD_TRIM_RESULT:
        {
            cli_cmd_function_trim_result();
            break;
        }
        case CLI_CMD_TEST_START:
        {
            cli_cmd_function_test_start();
            break;
        }
        case CLI_CMD_TEST_ONE_CHANNEL:
        {
            cli_cmd_function_test_one_channel();
            break;
        }
        case CLI_CMD_ID601_POWER:
        {
            cli_cmd_function_id601_power();
            break;
        }
        case CLI_CMD_ID601_DAISY:
        {
            cli_cmd_function_id601_daisy();
            break;
        }
        case CLI_CMD_ID601_COMMAND:
        {
            cli_cmd_function_id601_command();
            break;
        }
        case CLI_CMD_ID601_WRITE:
        {
            cli_cmd_function_id601_write();
            break;
        }
        case CLI_CMD_ID601_READ:
        {
            cli_cmd_function_id601_read();
            break;
        }
        case CLI_CMD_ID601_DUMP:
        {
            cli_cmd_function_id601_dump();
            break;
        }
        case CLI_CMD_ID601_INIT:
        {
            cli_cmd_function_id601_init();
            break;
        }
        case CLI_CMD_ID601_EFUSE:
        {
            cli_cmd_function_id601_efuse();
            break;
        }
        default:
        {
            // Handle unknown command
            drv_uart_printf(LOG_LV_WARN, "\r\n    Unknown CMD.");
            break;
        }
    }
    drv_uart_printf(LOG_LV_INFO, CLI_PROMPT);
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

        if (NULL != p_msg)
        {
            cli_command_parse(p_msg);
            cli_command_execute();
        }
    }

    id601_test_iout_one_channel_process();
}
/* USER CODE END 0 */
