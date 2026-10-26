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
#include "drv_timer.h"
#include "common_def.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_CLI_CMD_LIST
{
    CLI_CMD_NONE = 0U,
    CLI_CMD_UNKNOWN,
    CLI_CMD_HELP,
    CLI_CMD_JIG_HELP,
    CLI_CMD_RESET,
    CLI_CMD_BOARD_TYPE,

    CLI_CMD_VSYNC_NORMAL,
    CLI_CMD_VSYNC_TEST,

    CLI_CMD_TRIM_START,
    CLI_CMD_TRIM_RESULT,
    CLI_CMD_TEST_START,
    CLI_CMD_TEST_IOUT_MONITOR,
    CLI_CMD_TEST_MANUAL,

    CLI_CMD_ID601_POWER,
    CLI_CMD_ID601_DAISY,
    CLI_CMD_ID601_COMMAND,
    CLI_CMD_ID601_WRITE,
    CLI_CMD_ID601_READ,
    CLI_CMD_ID601_DUMP,
    CLI_CMD_ID601_INIT,
    CLI_CMD_ID601_MODE,
    CLI_CMD_ID601_EFUSE,

    CLI_CMD_MAX
} cli_cmd_list_t;

typedef enum tag_MANUAL_TEST_LIST
{
    MANUAL_TEST_NONE = 0U,
    MANUAL_TEST_DRIVING_VOLTAGE,
    MANUAL_TEST_UV_VDD,
    MANUAL_TEST_UVOV_LDO,
    MANUAL_TEST_UVOV_DAC,
    MANUAL_TEST_UVOV_OSC,
    MANUAL_TEST_OPEN_DETECTOR,
    MANUAL_TEST_FB_DETECTOR,
    MANUAL_TEST_SHORT_DETECTOR,
    MANUAL_TEST_FAULT_FLAG,
    MANUAL_TEST_BIST,
    MANUAL_TEST_HVS,

    MANUAL_TEST_MAX
} manual_test_list_t;

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
        "Show commands and usage"
    },
    {
        CLI_CMD_HELP,
        "?",
        "Show commands and usage"
    },
    {
        CLI_CMD_JIG_HELP,
        "jig",
        "HW description"
    },
    {
        CLI_CMD_RESET,
        "reset",
        "Reset MCU"
    },
    {
        CLI_CMD_BOARD_TYPE,
        "board_type",
        "board_type <type>"
        "\r\n    0/omitted=list + current, 1=GT, 2=MOBIS"
    },
    {
        CLI_CMD_VSYNC_NORMAL,
        "vsync",
        "vsync <type>"
        "\r\n    0/omitted=list + current, 1=stop, 2=start"
    },
    {
        CLI_CMD_VSYNC_TEST,
        "vsync_test",
        "vsync_test <type>"
        "\r\n    0/omitted=list + current, 1=stop, 2=start"
    },
    {
        CLI_CMD_TRIM_START,
        "trim_start",
        "Start trim"
    },
    {
        CLI_CMD_TRIM_RESULT,
        "trim_result",
        "Show trim search results"
    },
    {
        CLI_CMD_TEST_START,
        "test_start",
        "test_start <option>"
        "\r\n    0/omitted=list, 1=test, 2=test + VREF sweep"
    },
    {
        CLI_CMD_TEST_IOUT_MONITOR,
        "test_iout_monitor",
        "test_iout_monitor <option> <ch> <level> <vref>"
        "\r\n    0/omitted=list, 1=stop, 2=start"
        "\r\n    Stop: 1 (no other args)"
        "\r\n    Start: 2 <ch:1-12> <curr_lvl:0-15> <vref:0-4095>"
    },
    {
        CLI_CMD_TEST_MANUAL,
        "test_manual",
        "test_manual <test>"
        "\r\n    0/omitted=list"
        "\r\n    for more details, use \"test_manual\" CLI command"
    },
    {
        CLI_CMD_ID601_POWER,
        "id601_power",
        "id601_power <target> <state>"
        "\r\n    Target: 0/omitted=list, 1=VCC, 2=VLED"
        "\r\n    VCC:  0=OFF, 1=3.3V, 2=5.5V (GT only)"
        "\r\n    VLED: 0=OFF, 1=ON (SW14/SW15)"
        "\r\n    Invalid state turns selected power OFF"
    },
    {
        CLI_CMD_ID601_COMMAND,
        "id601_command",
        "id601_command <cmd>"
        "\r\n    0/omitted=list, 1=SYNCGEN, 2=IDGEN"
    },
    {
        CLI_CMD_ID601_DAISY,
        "id601_daisy",
        "id601_daisy <length>"
        "\r\n    0/omitted=list + current, 1-5=set length"
    },
    {
        CLI_CMD_ID601_WRITE,
        "id601_write",
        "id601_write <bank> <ic> <addr> <data>"
        "\r\n    Bank: 0/omitted=list, 1=General, 2=Mirror"
        "\r\n    IC: 1 ~ daisy length"
        "\r\n    Addr: General 0x00-0x3F, Mirror 0x00-0x1E"
        "\r\n    Data: 0x000-0xFFF (decimal or 0x hex)"
    },
    {
        CLI_CMD_ID601_READ,
        "id601_read",
        "id601_read <bank> <ic> <addr>"
        "\r\n    Bank: 0/omitted=list, 1=General, 2=Mirror"
        "\r\n    IC: 1 ~ daisy length"
        "\r\n    Addr: General 0x00-0x3F, Mirror 0x00-0x1E"
    },
    {
        CLI_CMD_ID601_DUMP,
        "id601_dump",
        "Read and dump all registers"
    },
    {
        CLI_CMD_ID601_INIT,
        "id601_init",
        "id601_init <mode>"
        "\r\n    0/omitted=list, 1=Testmode Entry, 2=Dimming"
    },
    {
        CLI_CMD_ID601_MODE,
        "id601_mode",
        "id601_mode <mode>"
        "\r\n    0/omitted=list, 1=Analog Test, 2=FLL Test, 3=Dimming Type1, 4=Dimming Type2"
    },
    {
        CLI_CMD_ID601_EFUSE,
        "id601_efuse",
        "id601_efuse <option>"
        "\r\n    0/omitted=list, 1=disable burn, 2=enable burn"
    }
};

static cli_request_t gt_cli_request;
static uint8_t gn_cli_argument_count;
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
    const uint16_t id601_daisy = drv_id601_get_daisy_length();
    drv_uart_printf(LOG_LV_INFO, CLI_CLEAR_SCREEN);

    drv_uart_printf(LOG_LV_INFO, "\r\n====================================================");
    drv_uart_printf(LOG_LV_INFO, "\r\n  %s", "ID601 TEST Board");
    drv_uart_printf(LOG_LV_INFO, "\r\n====================================================");
    drv_uart_printf(LOG_LV_INFO, "\r\n  * Build Date   : %s, %s", __DATE__, __TIME__);
    drv_uart_printf(LOG_LV_INFO, "\r\n  * FW Version   : v%d.%d.%d", FW_VER_MAJOR, FW_VER_MINOR, FW_VER_BUILD);
    drv_uart_printf(LOG_LV_INFO, "\r\n  * FW Git Rev   : %s", FW_GIT_REV);
    drv_uart_printf(LOG_LV_INFO, "\r\n  * FW Log Level : %s", drv_uart_get_log_level());
    drv_uart_printf(LOG_LV_INFO, "\r\n  * Board Type   : %s", (BOARD_TYPE_GT == drv_gpio_get_board_type()) ? ("GT") : ("MOBIS"));
    drv_uart_printf(LOG_LV_INFO, "\r\n  * ID601 Daisy  : %u", id601_daisy);

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

    gn_cli_argument_count = 0U;

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

    gn_cli_argument_count = (uint8_t)(token_cnt - 1U);
    gt_cli_request.last_command = command;
    if (token_cnt > 1U) gt_cli_request.val_1 = (uint32_t)strtoul(p_token[1], NULL, 0);
    if (token_cnt > 2U) gt_cli_request.val_2 = (uint32_t)strtoul(p_token[2], NULL, 0);
    if (token_cnt > 3U) gt_cli_request.val_3 = (uint32_t)strtoul(p_token[3], NULL, 0);
    if (token_cnt > 4U) gt_cli_request.val_4 = (uint32_t)strtoul(p_token[4], NULL, 0);
}

/* The first argument is a selector: 0 (or omitted) displays usage only.
 * Later arguments keep their original units and zero-valued data/state meanings.
 */
static void cli_print_command_usage(cli_cmd_list_t command)
{
    const uint16_t count = sizeof(gt_cli_command) / sizeof(gt_cli_command[0]);
    for (uint16_t idx = 0U; idx < count; ++idx)
    {
        if (gt_cli_command[idx].command == command)
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] %s", gt_cli_command[idx].description);
            return;
        }
    }
}

static inline void cli_cmd_function_none(void)
{

}
static inline void cli_cmd_function_help(void)
{
    cli_help();
}
static inline void cli_cmd_function_jig_help(void)
{
    const char* pin_name[24] = {
        "VDD1", "VSS1", "VSS2", "VSS3", "DIO",  "SVI",
        "O12",  "O11",  "O10",  "O9",   "O8",   "O7",
        "SVO",  "DDIO", "VSS4", "VSS5", "TEST", "VDD2",
        "O6",   "O5",   "O4",   "O3",   "O2",   "O1"
    };
    const char* jumper[24] = {
        "J89",  "J91",  "J95",  "J99",  "J101", "J103",
        "J107", "J108", "J109", "J110", "J111", "J112",
        "J104", "J102", "J100", "J96",  "J94",  "J90",
        "J84",  "J83",  "J82",  "J81",  "J80",  "J79"
    };
    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Jig Pin Mapping:\r\n");

    /* Top jumpers and names: Pin 24 -> 19 */
    drv_uart_printf(LOG_LV_INFO, "\r\n                       ");
    for (uint8_t i = 24U; i > 18U; --i)
    {
        drv_uart_printf(LOG_LV_INFO, " %-5s", jumper[i - 1U]);
    }

    drv_uart_printf(LOG_LV_INFO, "\r\n                       ");
    for (uint8_t i = 24U; i > 18U; --i)
    {
        drv_uart_printf(LOG_LV_INFO, " %-5s", pin_name[i - 1U]);
    }

    drv_uart_printf(LOG_LV_INFO,"\r\n                       +------------------------------------+");

    /* Top pin numbers inside the box */
    drv_uart_printf(LOG_LV_INFO, "\r\n                       |");
    for (uint8_t i = 24U; i > 18U; --i)
    {
        drv_uart_printf(LOG_LV_INFO, " %2u   ", (unsigned int)i);
    }
    drv_uart_printf(LOG_LV_INFO, "|");

    /* Left: Pin 1 -> 6 / Right: Pin 18 -> 13 */
    for (uint8_t i = 0U; i < 6U; ++i)
    {
        const uint8_t right = 17U - i;
        drv_uart_printf(LOG_LV_INFO, "\r\n        %-4s %-4s -----| %2u                              %2u |----- %-4s %s",
            jumper[i], pin_name[i], (unsigned int)(i + 1U), (unsigned int)(right + 1U), pin_name[right], jumper[right]);
    }

    /* Bottom pin numbers inside the box */
    drv_uart_printf(LOG_LV_INFO, "\r\n                       |");
    for (uint8_t i = 6U; i < 12U; ++i)
    {
        drv_uart_printf(LOG_LV_INFO, " %2u   ", (unsigned int)(i + 1U));
    }
    drv_uart_printf(LOG_LV_INFO, "|");

    drv_uart_printf(LOG_LV_INFO,"\r\n                       +------------------------------------+");

    /* Bottom names and jumpers: Pin 7 -> 12 */
    drv_uart_printf(LOG_LV_INFO, "\r\n                       ");
    for (uint8_t i = 6U; i < 12U; ++i)
    {
        drv_uart_printf(LOG_LV_INFO, " %-5s", pin_name[i]);
    }

    drv_uart_printf(LOG_LV_INFO, "\r\n                       ");
    for (uint8_t i = 6U; i < 12U; ++i)
    {
        drv_uart_printf(LOG_LV_INFO, " %-5s", jumper[i]);
    }
    drv_uart_printf(LOG_LV_INFO, "\r\n");

    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Jig Power Path:");
    drv_uart_printf(LOG_LV_INFO, "\r\n        ID601_VCC -- VR11 -- R88 -- [J78] -- ID601_VCC_2\r\n");
}
static inline void cli_cmd_function_reset(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    System resetting...");
    NVIC_SystemReset();
}
static inline void cli_cmd_function_board_type(void)
{
    const uint32_t type = gt_cli_request.val_1;
    if (0U == type)
    {
        cli_print_command_usage(CLI_CMD_BOARD_TYPE);
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Board Type: %s", (BOARD_TYPE_GT == drv_gpio_get_board_type()) ? ("GT") : ("MOBIS"));
    }
    else if (1U == type)
    {
        drv_gpio_set_board_type(BOARD_TYPE_GT);
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Board Type Set to GT");
    }
    else if (2U == type)
    {
        drv_gpio_set_board_type(BOARD_TYPE_MOBIS);
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Board Type Set to MOBIS");
    }
    else
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [CLI] Unknown board type command: %u.", type);
    }
}
static inline void cli_cmd_function_vsync(void)
{
    const uint32_t type = gt_cli_request.val_1;
    if (0U == type)
    {
        cli_print_command_usage(CLI_CMD_VSYNC_NORMAL);
    }
    else if (1U == type)
    {
        // TIM8 stop
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Vsync stopped");
    }
    else if (2U == type)
    {
        // TIM8 start
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Vsync started");
    }
    else
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [CLI] Unknown vsync command: %u.", type);
    }
}
static inline void cli_cmd_function_vsync_test(void)
{
    const uint32_t type = gt_cli_request.val_1;
    if (0U == type)
    {
        cli_print_command_usage(CLI_CMD_VSYNC_TEST);
    }
    else if (1U == type)
    {
        drv_tim_test_vsync_stop();
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Vsync Test stopped");
    }
    else if (2U == type)
    {
        drv_tim_test_vsync_start();
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Vsync Test started");
    }
    else
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [CLI] Unknown vsync_test command: %u.", type);
    }
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
    if ((0U == option) || (option > 2U))
    {
        cli_print_command_usage(CLI_CMD_TEST_START);
        return;
    }
    test_set_vref_sweep_enable(2U == option);
    osThreadFlagsSet(ID601TestTaskHandle, TEST_START_FLAG);
}
static inline void cli_cmd_function_test_iout_monitor(void)
{
    const uint32_t option = gt_cli_request.val_1;
    const uint32_t channel = gt_cli_request.val_2;
    const uint32_t max_curr_lvl = gt_cli_request.val_3;
    const uint32_t vref = gt_cli_request.val_4;

    if ((0U == option) || (option > 2U))
    {
        cli_print_command_usage(CLI_CMD_TEST_IOUT_MONITOR);
        return;
    }
    if (1U == option)
    {
        id601_test_iout_monitor_stop();
        return;
    }
    if ((gn_cli_argument_count != 4U) || (channel < 1U) || (channel > ID601_CH_MAX) ||
        (max_curr_lvl > 0x0FU) || (vref > 4095U))
    {
        cli_print_command_usage(CLI_CMD_TEST_IOUT_MONITOR);
        return;
    }
    id601_test_iout_monitor_start((uint16_t)(channel - 1U), (uint16_t)max_curr_lvl, (uint16_t)vref);
}
static inline void cli_cmd_help_test_manual(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Test Manual Help:");
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: Driving Voltage <ch:1-12>", MANUAL_TEST_DRIVING_VOLTAGE);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: UV VDD", MANUAL_TEST_UV_VDD);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: UV/OV LDO     <option 1:INIT, 2:UV, 3:OV>", MANUAL_TEST_UVOV_LDO);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: UV/OV DAC     <option 1:INIT, 2:UV, 3:OV>", MANUAL_TEST_UVOV_DAC);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: UV/OV OSC     <option 1:INIT, 2:UV, 3:OV>", MANUAL_TEST_UVOV_OSC);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: OPEN DETECTOR, not yet", MANUAL_TEST_OPEN_DETECTOR);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: FB DETECTOR, not yet", MANUAL_TEST_FB_DETECTOR);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: SHORT DETECTOR, not yet", MANUAL_TEST_SHORT_DETECTOR);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: FAULT FLAG, not yet", MANUAL_TEST_FAULT_FLAG);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: BIST", MANUAL_TEST_BIST);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %u: HVS", MANUAL_TEST_HVS);
}
static inline void cli_cmd_function_test_manual(void)
{
    const uint32_t test = gt_cli_request.val_1;

    if ((0U == test))
    {
        cli_cmd_help_test_manual();
        return;
    }

    switch (test)
    {
        case MANUAL_TEST_DRIVING_VOLTAGE:
        {
            const uint32_t ch = gt_cli_request.val_2;
            if (ch < 1U || ch > ID601_CH_MAX)
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] \"test_manual %u %u\" is invalid.", gt_cli_request.val_1, gt_cli_request.val_2);
                return;
            }
            id601_test_iout_monitor_start((uint16_t)(ch - 1U), 1U, 4095U);
            break;
        }
        case MANUAL_TEST_UV_VDD:
        {
            drv_gpio_clear_id601();
            drv_gpio_id601_vcc(ID601_VCC_3V3);
            if (false == id601_init_tm_entry_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
            }
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00A, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00A, 0x00A);
            break;
        }
        case MANUAL_TEST_UVOV_LDO:
        {
            const uint32_t option = gt_cli_request.val_2;
            if (option < 1U || option > 3U)
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] \"test_manual %u %u\" is invalid.", gt_cli_request.val_1, gt_cli_request.val_2);
                return;
            }
            drv_gpio_clear_id601();
            drv_gpio_id601_vcc(ID601_VCC_3V3);
            if (false == id601_init_tm_entry_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
            }
            const uint16_t test_ana_en_data = 0x00D;
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, test_ana_en_data, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, test_ana_en_data, test_ana_en_data);
            switch (option)
            {
                case 1U:
                    break;
                case 2U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00A, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00A, 0x00A);
                    break;
                case 3U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x009, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x009, 0x009);
                    break;
                default:
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Invalid option for UV/OV LDO test.");
                    return;
            }
            break;
        }
        case MANUAL_TEST_UVOV_DAC:
        {
            const uint32_t option = gt_cli_request.val_2;
            if (option < 1U || option > 3U)
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] \"test_manual %u %u\" is invalid.", gt_cli_request.val_1, gt_cli_request.val_2);
                return;
            }
            drv_gpio_clear_id601();
            drv_gpio_id601_vcc(ID601_VCC_3V3);
            if (false == id601_init_tm_entry_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
            }
            const uint16_t test_ana_en_data = 0x00E;
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, test_ana_en_data, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, test_ana_en_data, test_ana_en_data);
            switch (option)
            {
                case 1U:
                    break;
                case 2U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00E, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00E, 0x00E);
                    break;
                case 3U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00D, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00D, 0x00D);
                    break;
                default:
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Invalid option for UV/OV DAC test.");
                    return;
            }
            break;
        }
        case MANUAL_TEST_UVOV_OSC:
        {
            const uint32_t option = gt_cli_request.val_2;
            if (option < 1U || option > 3U)
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] \"test_manual %u %u\" is invalid.", gt_cli_request.val_1, gt_cli_request.val_2);
                return;
            }
            drv_gpio_clear_id601();
            drv_gpio_id601_vcc(ID601_VCC_3V3);
            if (false == id601_init_tm_entry_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
            }
            const uint16_t test_ana_en_data = 0x00F;
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, test_ana_en_data, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, test_ana_en_data, test_ana_en_data);
            switch (option)
            {
                case 1U:
                    break;
                case 2U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00B, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00B, 0x00B);
                    break;
                case 3U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00C, 0x00C);
                    break;
                default:
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Invalid option for UV/OV DAC test.");
                    return;
            }
            break;
        }
        case MANUAL_TEST_OPEN_DETECTOR:
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Not available for Open Detector test.");
            break;
        }
        case MANUAL_TEST_FB_DETECTOR:
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Not available for FB Detector test.");
            break;
        }
        case MANUAL_TEST_SHORT_DETECTOR:
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Not available for Short Detector test.");
            break;
        }
        case MANUAL_TEST_FAULT_FLAG:
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Not available for Fault Flag test.");
            break;
        }
        case MANUAL_TEST_BIST:
        {
            drv_gpio_clear_id601();
            drv_gpio_id601_vcc(ID601_VCC_3V3);
            if (false == id601_init_tm_entry_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
            }

            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_FAULT_CONTROL, 0x000, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_FAULT_CONTROL, 0x000, 0x000);

            LL_GPIO_SetOutputPin(DEBUG1_GPIO_Port, DEBUG1_Pin);
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_BIST, 0x800, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", 1U, ID601_ADDR_GENERAL_BIST, 0x800, 0x800);

            drv_tim_delay_us(50U);

            uint16_t read_data = 0xFFF;
            if (false == id601_read_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_BIST, drv_id601_get_crc_func(), &read_data))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to read from ID601 register.");
            }
            LL_GPIO_ResetOutputPin(DEBUG1_GPIO_Port, DEBUG1_Pin);
            if (0x02U == read_data)
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [BIST] Addr:0x%02X, Data:%4u(0x%03X) (O)", ID601_ADDR_GENERAL_BIST, read_data, read_data);
            }
            else
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] [BIST] Addr:0x%02X, Data:%4u(0x%03X) (X)", ID601_ADDR_GENERAL_BIST, read_data, read_data);
            }

            break;
        }
        case MANUAL_TEST_HVS:
        {
            drv_id601_set_hvs_flag(true);
            test_set_vref_sweep_enable(false);
            osThreadFlagsSet(ID601TestTaskHandle, TEST_START_FLAG);
            break;
        }
        default:
        {
            cli_print_command_usage(CLI_CMD_TEST_MANUAL);
            return;
        }
    }
}
static inline void cli_cmd_function_id601_power(void)
{
    const uint32_t val1 =gt_cli_request.val_1;
    const uint32_t val2 =gt_cli_request.val_2;

    if ((0U == val1) || (val1 > 2U) || (gn_cli_argument_count != 2U))
    {
        cli_print_command_usage(CLI_CMD_ID601_POWER);
        return;
    }

    const uint8_t board_type = drv_gpio_get_board_type();

    switch (val1)
    {
        case 1U:
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
                if (BOARD_TYPE_GT == board_type)
                {
                    drv_gpio_id601_vcc(ID601_VCC_5V5);
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 VCC set to 5.5V.");
                }
                else
                {
                    drv_gpio_id601_vcc(ID601_VCC_OFF);
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %u is invalid, ID601 VCC turned off.", val2);
                }
            }
            else
            {
                drv_gpio_id601_vcc(ID601_VCC_OFF);
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %u is invalid, ID601 VCC turned off.", val2);
            }
            break;
        }
        case 2U:
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
    const uint32_t daisy = gt_cli_request.val_1;
    if (0U == daisy)
    {
        cli_print_command_usage(CLI_CMD_ID601_DAISY);
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 daisy chain length is %u.", drv_id601_get_daisy_length());
    }
    else
    {
        if ((daisy <= 5U) && (true == drv_id601_set_daisy_length((uint16_t)daisy)))
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
    const uint32_t command = gt_cli_request.val_1;
    if (1U == command)
    {
        id601_sync_gen(drv_id601_get_crc_func());
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 syncgen command executed.");
    }
    else if (2U == command)
    {
        id601_id_gen(drv_id601_get_crc_func());
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 idgen command executed.");
    }
    else
    {
        cli_print_command_usage(CLI_CMD_ID601_COMMAND);
    }
}
static inline void cli_cmd_function_id601_write(void)
{
    const uint32_t bank_option = gt_cli_request.val_1;
    if ((0U == bank_option) || (bank_option > 2U) || (gn_cli_argument_count != 4U))
    {
        cli_print_command_usage(CLI_CMD_ID601_WRITE);
        return;
    }
    /* CLI selectors 1/2 map to the driver's existing General/Mirror banks. */
    const uint16_t bank = (1U == bank_option) ? ID601_REGISTER_BANK_GENERAL : ID601_REGISTER_BANK_MIRROR;
    if ((gt_cli_request.val_2 < 1U) || (gt_cli_request.val_2 > drv_id601_get_daisy_length()) ||
        (gt_cli_request.val_3 >= ((ID601_REGISTER_BANK_GENERAL == bank) ? (ID601_ADDR_GENERAL_MAX_COUNT) : (ID601_ADDR_MIRROR_MAX_COUNT))) ||
        (gt_cli_request.val_4 > 0xFFFU))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Invalid arguments for id601_write command.");
        cli_print_command_usage(CLI_CMD_ID601_WRITE);
        return;
    }

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
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", ic_num, addr, data, data);
        }
        else
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [Mirror]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", ic_num, addr, data, data);
        }
    }
}
static inline void cli_cmd_function_id601_read(void)
{
    const uint32_t bank_option = gt_cli_request.val_1;
    if ((0U == bank_option) || (bank_option > 2U) || (gn_cli_argument_count != 3U))
    {
        cli_print_command_usage(CLI_CMD_ID601_READ);
        return;
    }
    /* CLI selectors 1/2 map to the driver's existing General/Mirror banks. */
    const uint16_t bank = (1U == bank_option) ? ID601_REGISTER_BANK_GENERAL : ID601_REGISTER_BANK_MIRROR;
    if ((gt_cli_request.val_2 < 1U) ||
        (gt_cli_request.val_2 > drv_id601_get_daisy_length()) ||
        (gt_cli_request.val_3 >= ((ID601_REGISTER_BANK_GENERAL == bank) ?
            ID601_ADDR_GENERAL_MAX_COUNT : ID601_ADDR_MIRROR_MAX_COUNT)))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Invalid arguments for id601_read command.");
        cli_print_command_usage(CLI_CMD_ID601_READ);
        return;
    }

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

    uint16_t read_data = 0xFFF;
    if (false == id601_read_register(bank, ic_num, addr, drv_id601_get_crc_func(), &read_data))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to read from ID601 register.");
    }
    else
    {
        if (ID601_REGISTER_BANK_GENERAL == bank)
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Read ] [General]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", ic_num, addr, read_data, read_data);
        }
        else
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Read ] [Mirror]: IC:%u, Addr:0x%02X, Data:%4u(0x%03X)", ic_num, addr, read_data, read_data);
        }
    }
}
static inline void cli_cmd_function_id601_dump(void)
{
    id601_dump_registers();
}
static inline void cli_cmd_function_id601_init(void)
{
    const uint32_t max_init = 2U; // tm_entry, dimming
    const uint32_t mode = gt_cli_request.val_1;

    if (mode > max_init)
    {
        cli_print_command_usage(CLI_CMD_ID601_INIT);
        return;
    }

    typedef bool (*id601_init_func_t)(void);
    const id601_init_func_t init_funcs[2] =
    {
        id601_init_tm_entry_mode,
        id601_init_dimming_mode
    };

    if (0U == mode)
    {
        cli_print_command_usage(CLI_CMD_ID601_INIT);
    }
    else
    {
        drv_gpio_clear_id601();
        drv_gpio_id601_vcc(ID601_VCC_3V3);
        switch (mode)
        {
            case 1U:
            case 2U:
            {
                if (true == init_funcs[mode - 1U]())
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Initialize OK");
                }
                else
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Initialize Failed (%u)", mode);
                }
                break;
            }
            default:
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Init %u is invalid.", mode);
                cli_print_command_usage(CLI_CMD_ID601_INIT);
                break;
            }
        }
    }
}
static inline void cli_cmd_function_id601_mode(void)
{
    const uint32_t max_mode = 4U; // analog test, FLL test, Dimming Type1, Dimming Type2
    const uint32_t mode = gt_cli_request.val_1;

    if (mode > max_mode)
    {
        cli_print_command_usage(CLI_CMD_ID601_MODE);
        return;
    }

    typedef bool (*id601_mode_func_t)(void);
    const id601_mode_func_t init_funcs[4] =
    {
        id601_set_analog_test_mode,
        id601_set_fll_test_mode,
        id601_set_dimming_type1_mode,
        id601_set_dimming_type2_mode,
    };

    if (0U == mode)
    {
        cli_print_command_usage(CLI_CMD_ID601_MODE);
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
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Set Mode OK");
                }
                else
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Set Mode Failed (%u)", mode);
                }
                break;
            }
            default:
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Mode %u is invalid.", mode);
                cli_print_command_usage(CLI_CMD_ID601_MODE);
                break;
            }
        }
    }
}
static inline void cli_cmd_function_id601_efuse(void)
{
    const uint32_t enable = gt_cli_request.val_1;
    if ((0U == enable) || (enable > 2U))
    {
        cli_print_command_usage(CLI_CMD_ID601_EFUSE);
        return;
    }
    id601_set_efuse_burn_enable(2U == enable);
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
        case CLI_CMD_JIG_HELP:
        {
            cli_cmd_function_jig_help();
            break;
        }
        case CLI_CMD_RESET:
        {
            cli_cmd_function_reset();
            break;
        }
        case CLI_CMD_BOARD_TYPE:
        {
            cli_cmd_function_board_type();
            break;
        }
        case CLI_CMD_VSYNC_NORMAL:
        {
            cli_cmd_function_vsync();
            break;
        }
        case CLI_CMD_VSYNC_TEST:
        {
            cli_cmd_function_vsync_test();
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
        case CLI_CMD_TEST_IOUT_MONITOR:
        {
            cli_cmd_function_test_iout_monitor();
            break;
        }
        case CLI_CMD_TEST_MANUAL:
        {
            cli_cmd_function_test_manual();
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
        case CLI_CMD_ID601_MODE:
        {
            cli_cmd_function_id601_mode();
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

    id601_test_iout_monitor_process();
}
/* USER CODE END 0 */
