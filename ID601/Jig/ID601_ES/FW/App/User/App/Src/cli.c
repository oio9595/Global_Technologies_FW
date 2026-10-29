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
#include "id601_dimming.h"
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
    CLI_CMD_TRIM_MANUAL,
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

    CLI_CMD_LD_BUFF,
    CLI_CMD_LD_CONFIG_FAULT_READ_ENABLE,
    CLI_CMD_LD_CONFIG_LD_TRANS_ENABLE,
    CLI_CMD_LD_CONFIG_PWM_RES,

    CLI_CMD_MAX
} cli_cmd_list_t;

typedef enum tag_MANUAL_TRIM_LIST
{
    MANUAL_TRIM_NONE = 0U,
    MANUAL_TRIM_IREF1,
    MANUAL_TRIM_IREF2,
    MANUAL_TRIM_LDO_DET,
    MANUAL_TRIM_LDO_DIG,
    MANUAL_TRIM_LDO_DAC,
    MANUAL_TRIM_LDO_FLL,
    MANUAL_TRIM_OFS_TEMP,
    MANUAL_TRIM_OSC_RCTL,
    MANUAL_TRIM_GAIN,
    MANUAL_TRIM_OFFSET,
    MANUAL_TRIM_MAX
} manual_trim_list_t;

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
        "\r\n    0/omitted=list + board type, 1=GT, 2=MOBIS"
    },
    {
        CLI_CMD_VSYNC_NORMAL,
        "vsync",
        "vsync <type>"
        "\r\n    0/omitted=list, 1=stop, 2=start"
    },
    {
        CLI_CMD_VSYNC_TEST,
        "vsync_test",
        "vsync_test <type>"
        "\r\n    0/omitted=list, 1=stop, 2=start"
    },
    {
        CLI_CMD_TRIM_START,
        "trim_start",
        "Start trim"
    },
    {
        CLI_CMD_TRIM_MANUAL,
        "trim_manual",
        "trim_manual <test>"
        "\r\n    Configure manual trim setup"
        "\r\n    0/omitted=list"
        "\r\n    for more details, use \"trim_manual\" CLI command"
    },
    {
        CLI_CMD_TRIM_RESULT,
        "trim_result",
        "Show trim search mirror register values"
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
        "test_iout_monitor <option> <ch> <curr_lvl> <vref>"
        "\r\n    0/omitted=list, 1=stop, 2=start"
        "\r\n    Stop: 1 (no other args)"
        "\r\n    Start: 2 <ch:1-12> <curr_lvl:0-15> <vref:0-4095>"
    },
    {
        CLI_CMD_TEST_MANUAL,
        "test_manual",
        "test_manual <test>"
        "\r\n    Configure manual test setup"
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
        CLI_CMD_ID601_DAISY,
        "id601_daisy",
        "id601_daisy <length>"
        "\r\n    0/omitted=list + current, 1-5=set length"
    },
    {
        CLI_CMD_ID601_COMMAND,
        "id601_command",
        "id601_command <cmd>"
        "\r\n    0/omitted=list, 1=SYNCGEN, 2=IDGEN"
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
    },
    {
        CLI_CMD_LD_BUFF,
        "ld_buff",
        "ld_buff <ic> <ch> <led> <value>"
        "\r\n    IC: 1 ~ daisy length"
        "\r\n    Ch: 1 ~ channel size"
        "\r\n    Led: 1 ~ led per channel"
        "\r\n    Value: 0x000-0xFFFF (decimal or 0x hex, depending on pwm_res)"
    },
    {
        CLI_CMD_LD_CONFIG_FAULT_READ_ENABLE,
        "ld_config_fault_read_enable",
        "ld_config_fault_read_enable <enable>"
        "\r\n    Configure the fault read enable for vsync task"
        "\r\n    0/omitted=list, 1=disable, 2=enable"
    },
    {
        CLI_CMD_LD_CONFIG_LD_TRANS_ENABLE,
        "ld_config_ld_trans_enable",
        "ld_config_ld_trans_enable <enable>"
        "\r\n    Configure the LD transfer enable for vsync task"
        "\r\n    0/omitted=list, 1=disable, 2=enable"
    },
    {
        CLI_CMD_LD_CONFIG_PWM_RES,
        "ld_config_pwm_res",
        "ld_config_pwm_res <res>"
        "\r\n    Configure the PWM resolution for the LD buffer"
        "\r\n    Possible when ID601 is not initiated"
        "\r\n    0/omitted=list, 1=12bit, 2=14bit"
    }
};

static cli_request_t gt_cli_request;
static char gs_cli_input[MSG_BUFFER_SIZE];

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
        if (0U < idx)
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n");
        }

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
    memcpy(gs_cli_input, temp, MSG_BUFFER_SIZE);

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
static void cli_print_invalid_input(void)
{
    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] \"%s\" is invalid.", gs_cli_input);
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
    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] System resetting...");
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
        cli_print_invalid_input();
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
        drv_tim_vsync_stop();
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Vsync stopped");
    }
    else if (2U == type)
    {
        drv_tim_vsync_start();
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Vsync started");
    }
    else
    {
        cli_print_invalid_input();
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
        cli_print_invalid_input();
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
static inline void cli_cmd_help_trim_manual(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Trim Manual Help:");
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: IREF1", MANUAL_TRIM_IREF1);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: IREF2", MANUAL_TRIM_IREF2);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: LDO_DET", MANUAL_TRIM_LDO_DET);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: LDO_DIG", MANUAL_TRIM_LDO_DIG);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: LDO_DAC", MANUAL_TRIM_LDO_DAC);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: LDO_FLL", MANUAL_TRIM_LDO_FLL);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: OFS_TEMP <option 1:V1, 2:V2>", MANUAL_TRIM_OFS_TEMP);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: OSC_RCTL", MANUAL_TRIM_OSC_RCTL);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: GAIN <option 1:P1, 2:P2> <ch:1-12>", MANUAL_TRIM_GAIN);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: OFFSET <option 1:P3, 2:P4> <ch:1-12>", MANUAL_TRIM_OFFSET);
}
static inline void cli_cmd_function_trim_manual(void)
{
    const uint32_t test = gt_cli_request.val_1;
    const uint32_t option = gt_cli_request.val_2;
    const uint32_t ch = gt_cli_request.val_3;
    bool param_valid = false;

    if ((0U == test))
    {
        cli_cmd_help_trim_manual();
        return;
    }

    switch (test)
    {
        case MANUAL_TRIM_IREF1:
        case MANUAL_TRIM_IREF2:
        case MANUAL_TRIM_LDO_DET:
        case MANUAL_TRIM_LDO_DIG:
        case MANUAL_TRIM_LDO_DAC:
        case MANUAL_TRIM_LDO_FLL:
        case MANUAL_TRIM_OSC_RCTL:
        {
            param_valid = (1U == gn_cli_argument_count);
            break;
        }
        case MANUAL_TRIM_OFS_TEMP:
        {
            param_valid = ((2U == gn_cli_argument_count) && (option >= 1U) && (option <= 2U));
            break;
        }
        case MANUAL_TRIM_GAIN:
        case MANUAL_TRIM_OFFSET:
        {
            param_valid = ((3U == gn_cli_argument_count) && (option >= 1U) && (option <= 2U) && (ch >= 1U) && (ch <= ID601_CH_MAX));
            break;
        }
        default:
        {
            break;
        }
    }

    if (false == param_valid)
    {
        cli_print_invalid_input();
        return;
    }

    drv_gpio_clear_id601();
    drv_gpio_id601_vcc(ID601_VCC_3V3);

    if (false == id601_init_tm_entry_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
        return;
    }

    if (false == id601_compare_initial_code())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Initial Code Compare Failed.", __func__);
        return;
    }

    if (false == id601_set_analog_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
        return;
    }

    switch (test)
    {
        case MANUAL_TRIM_IREF1:
        {
            id601_trim_setup_iref1();
            break;
        }
        case MANUAL_TRIM_IREF2:
        {
            id601_trim_setup_iref2();
            break;
        }
        case MANUAL_TRIM_LDO_DET:
        {
            id601_trim_setup_ldo_det();
            break;
        }
        case MANUAL_TRIM_LDO_DIG:
        {
            id601_trim_setup_ldo_dig();
            break;
        }
        case MANUAL_TRIM_LDO_DAC:
        {
            id601_trim_setup_ldo_dac();
            break;
        }
        case MANUAL_TRIM_LDO_FLL:
        {
            id601_trim_setup_ldo_fll();
            break;
        }
        case MANUAL_TRIM_OFS_TEMP:
        {
            const uint32_t option = gt_cli_request.val_2;
            id601_trim_setup_ofs_temp();
            if (1U == option)
            {
                id601_trim_setup_ofs_temp_o1();
            }
            else if (2U == option)
            {
                id601_trim_setup_ofs_temp_o2();
            }
            else
            {
                cli_print_invalid_input();
            }
            break;
        }
        case MANUAL_TRIM_OSC_RCTL:
        {
            id601_trim_setup_osc_rctl();
            break;
        }
        case MANUAL_TRIM_GAIN:
        {
            const uint32_t option = gt_cli_request.val_2;
            const uint32_t ch = gt_cli_request.val_3;

            id601_trim_setup_gain();

            if (1U == option)
            {
                id601_trim_setup_gain_vref1();
            }
            else if (2U == option)
            {
                id601_trim_setup_gain_vref2();
            }
            else
            {
                cli_print_invalid_input();
                break;
            }

            if (ch > 0U && ch <= ID601_CH_MAX)
            {
                id601_trim_setup_current_chx((uint16_t)(ch - 1U));
            }
            else
            {
                cli_print_invalid_input();
            }
            break;
        }
        case MANUAL_TRIM_OFFSET:
        {
            const uint32_t option = gt_cli_request.val_2;
            const uint32_t ch = gt_cli_request.val_3;
            id601_trim_setup_offset();
            if (1U == option)
            {
                id601_trim_setup_offset_vref1();
            }
            else if (2U == option)
            {
                id601_trim_setup_offset_vref2();
            }
            else
            {
                cli_print_invalid_input();
                break;
            }

            if (ch > 0U && ch <= ID601_CH_MAX)
            {
                id601_trim_setup_current_chx((uint16_t)(ch - 1U));
            }
            else
            {
                cli_print_invalid_input();
            }
            break;
        }
        default:
        {
            cli_print_invalid_input();
            return;
        }
    }
}
static inline void cli_cmd_function_test_start(void)
{
    const uint32_t option = gt_cli_request.val_1;
    if (0U == option)
    {
        cli_print_command_usage(CLI_CMD_TEST_START);
        return;
    }
    if (option > 2U)
    {
        cli_print_invalid_input();
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

    if (0U == option)
    {
        cli_print_command_usage(CLI_CMD_TEST_IOUT_MONITOR);
        return;
    }
    if (option > 2U)
    {
        cli_print_invalid_input();
        return;
    }
    if ((gn_cli_argument_count != 4U) || (channel < 1U) || (channel > ID601_CH_MAX) ||
        (max_curr_lvl > 0x0FU) || (vref > 4095U))
    {
        cli_print_invalid_input();
        return;
    }
    if (1U == option)
    {
        id601_test_iout_monitor_stop();
    }
    else
    {
        id601_test_iout_monitor_start((uint16_t)(channel - 1U), (uint16_t)max_curr_lvl, (uint16_t)vref);
    }
}
static inline void cli_cmd_help_test_manual(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Test Manual Help:");
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: Driving Voltage <ch:1-12>", MANUAL_TEST_DRIVING_VOLTAGE);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: UV VDD", MANUAL_TEST_UV_VDD);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: UV/OV LDO     <option 1:INIT, 2:UV, 3:OV>", MANUAL_TEST_UVOV_LDO);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: UV/OV DAC     <option 1:INIT, 2:UV, 3:OV>", MANUAL_TEST_UVOV_DAC);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: UV/OV OSC     <option 1:INIT, 2:UV, 3:OV>", MANUAL_TEST_UVOV_OSC);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: OPEN DETECTOR, not yet", MANUAL_TEST_OPEN_DETECTOR);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: FB DETECTOR, not yet", MANUAL_TEST_FB_DETECTOR);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: SHORT DETECTOR, not yet", MANUAL_TEST_SHORT_DETECTOR);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: FAULT FLAG, not yet", MANUAL_TEST_FAULT_FLAG);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: BIST", MANUAL_TEST_BIST);
    drv_uart_printf(LOG_LV_INFO, "\r\n    %02u: HVS", MANUAL_TEST_HVS);
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
                cli_print_invalid_input();
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
                return;
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
                return;
            }
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00A, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            else
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00A, 0x00A);
            }
            break;
        }
        case MANUAL_TEST_UVOV_LDO:
        {
            const uint32_t option = gt_cli_request.val_2;
            if (option < 1U || option > 3U)
            {
                cli_print_invalid_input();
                return;
            }
            drv_gpio_clear_id601();
            drv_gpio_id601_vcc(ID601_VCC_3V3);
            if (false == id601_init_tm_entry_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
                return;
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
                return;
            }
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00D, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            else
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00D, 0x00D);
            }
            switch (option)
            {
                case 1U:
                    break;
                case 2U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);
                    }

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00A, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_MODE0, 0x00A, 0x00A);
                    }
                    break;
                case 3U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);
                    }

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x009, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_MODE0, 0x009, 0x009);
                    }
                    break;
                default:
                    cli_print_invalid_input();
                    return;
            }
            break;
        }
        case MANUAL_TEST_UVOV_DAC:
        {
            const uint32_t option = gt_cli_request.val_2;
            if (option < 1U || option > 3U)
            {
                cli_print_invalid_input();
                return;
            }
            drv_gpio_clear_id601();
            drv_gpio_id601_vcc(ID601_VCC_3V3);
            if (false == id601_init_tm_entry_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
                return;
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
                return;
            }
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00E, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            else
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00E, 0x00E);
            }
            switch (option)
            {
                case 1U:
                    break;
                case 2U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);
                    }

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00E, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_MODE0, 0x00E, 0x00E);
                    }
                    break;
                case 3U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);
                    }

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00D, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_MODE0, 0x00D, 0x00D);
                    }
                    break;
                default:
                    cli_print_invalid_input();
                    return;
            }
            break;
        }
        case MANUAL_TEST_UVOV_OSC:
        {
            const uint32_t option = gt_cli_request.val_2;
            if (option < 1U || option > 3U)
            {
                cli_print_invalid_input();
                return;
            }
            drv_gpio_clear_id601();
            drv_gpio_id601_vcc(ID601_VCC_3V3);
            if (false == id601_init_tm_entry_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 TM Entry Mode Failed", __func__);
                return;
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
                return;
            }
            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00F, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
            }
            else
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_ANA_EN, 0x00F, 0x00F);
            }
            switch (option)
            {
                case 1U:
                    break;
                case 2U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);
                    }

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00B, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_MODE0, 0x00B, 0x00B);
                    }
                    break;
                case 3U:
                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OP_MODE, 0x86C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_OP_MODE, 0x86C, 0x86C);
                    }

                    if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_TEST_MODE0, 0x00C, drv_id601_get_crc_func()))
                    {
                        drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                    }
                    else
                    {
                        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_TEST_MODE0, 0x00C, 0x00C);
                    }
                    break;
                default:
                    cli_print_invalid_input();
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
                return;
            }
            if (false == id601_set_analog_test_mode())
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] %s: ID601 Analog Test Mode Failed", __func__);
                return;
            }

            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_FAULT_CONTROL, 0x000, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                return;
            }
            else
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_FAULT_CONTROL, 0x000, 0x000);
            }

            if (false == id601_write_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_BIST, 0x800, drv_id601_get_crc_func()))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to write to ID601 register.");
                return;
            }
            else
            {
                drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [Write] [General]: Addr:0x%02X, Data:%4u(0x%03X)", ID601_ADDR_GENERAL_BIST, 0x800, 0x800);
            }

            drv_tim_delay_us(50U);

            uint16_t read_data = 0xFFF;
            if (false == id601_read_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_BIST, drv_id601_get_crc_func(), &read_data))
            {
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Failed to read from ID601 register.");
            }
            else
            {
                if (0x02U == read_data)
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] [BIST] Addr:0x%02X, Data:%4u(0x%03X) (PASS)", ID601_ADDR_GENERAL_BIST, read_data, read_data);
                }
                else
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] [BIST] Addr:0x%02X, Data:%4u(0x%03X) (FAIL)", ID601_ADDR_GENERAL_BIST, read_data, read_data);
                }
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
            cli_print_invalid_input();
            return;
        }
    }
}
static inline void cli_cmd_function_id601_power(void)
{
    const uint32_t val1 =gt_cli_request.val_1;
    const uint32_t val2 =gt_cli_request.val_2;

    if (0U == val1)
    {
        cli_print_command_usage(CLI_CMD_ID601_POWER);
        return;
    }

    if ((val1 > 2U) || (gn_cli_argument_count != 2U))
    {
        cli_print_invalid_input();
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
                    cli_print_invalid_input();
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] ID601 VCC turned off.");
                }
            }
            else
            {
                drv_gpio_id601_vcc(ID601_VCC_OFF);
                cli_print_invalid_input();
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] ID601 VCC turned off.");
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
                cli_print_invalid_input();
                drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] ID601 VLED turned off.");
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
        if ((daisy <= ID601_DAISY_LENGTH_MAX) && (true == drv_id601_set_daisy_length((uint16_t)daisy)))
        {
            drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] ID601 daisy chain length set to %u.", daisy);
        }
        else
        {
            cli_print_invalid_input();
            return;
        }
    }
}
static inline void cli_cmd_function_id601_command(void)
{
    const uint32_t command = gt_cli_request.val_1;
    if (0U == command)
    {
        cli_print_command_usage(CLI_CMD_ID601_COMMAND);
        return;
    }
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
        cli_print_invalid_input();
    }
}
static inline void cli_cmd_function_id601_write(void)
{
    const uint32_t bank_option = gt_cli_request.val_1;
    if (0U == bank_option)
    {
        cli_print_command_usage(CLI_CMD_ID601_WRITE);
        return;
    }
    if ((bank_option > 2U) || (gn_cli_argument_count != 4U))
    {
        cli_print_invalid_input();
        return;
    }
    /* CLI selectors 1/2 map to the driver's existing General/Mirror banks. */
    const uint16_t bank = (1U == bank_option) ? ID601_REGISTER_BANK_GENERAL : ID601_REGISTER_BANK_MIRROR;
    if ((gt_cli_request.val_2 < 1U) || (gt_cli_request.val_2 > drv_id601_get_daisy_length()) ||
        (gt_cli_request.val_3 >= ((ID601_REGISTER_BANK_GENERAL == bank) ? (ID601_ADDR_GENERAL_MAX_COUNT) : (ID601_ADDR_MIRROR_MAX_COUNT))) ||
        (gt_cli_request.val_4 > 0xFFFU))
    {
        cli_print_invalid_input();
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
    if (0U == bank_option)
    {
        cli_print_command_usage(CLI_CMD_ID601_READ);
        return;
    }
    if ((bank_option > 2U) || (gn_cli_argument_count != 3U))
    {
        cli_print_invalid_input();
        return;
    }
    /* CLI selectors 1/2 map to the driver's existing General/Mirror banks. */
    const uint16_t bank = (1U == bank_option) ? ID601_REGISTER_BANK_GENERAL : ID601_REGISTER_BANK_MIRROR;
    if ((gt_cli_request.val_2 < 1U) ||
        (gt_cli_request.val_2 > drv_id601_get_daisy_length()) ||
        (gt_cli_request.val_3 >= ((ID601_REGISTER_BANK_GENERAL == bank) ?
            ID601_ADDR_GENERAL_MAX_COUNT : ID601_ADDR_MIRROR_MAX_COUNT)))
    {
        cli_print_invalid_input();
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
    const uint32_t mode = gt_cli_request.val_1;

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
        switch (mode)
        {
            case 1U:
            case 2U:
            {
                drv_gpio_clear_id601();
                drv_gpio_id601_vcc(ID601_VCC_3V3);
                if (true == init_funcs[mode - 1U]())
                {
                    drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Initialization Complete.");
                }
                else
                {
                    drv_uart_printf(LOG_LV_ERROR, "\r\n    [CLI] Initialization Failed. (%u)", mode);
                }
                break;
            }
            default:
            {
                cli_print_invalid_input();
                break;
            }
        }
    }
}
static inline void cli_cmd_function_id601_mode(void)
{
    const uint32_t mode = gt_cli_request.val_1;

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
                cli_print_invalid_input();
                break;
            }
        }
    }
}
static inline void cli_cmd_function_id601_efuse(void)
{
    const uint32_t enable = gt_cli_request.val_1;
    if (0U == enable)
    {
        cli_print_command_usage(CLI_CMD_ID601_EFUSE);
        return;
    }
    if (enable > 2U)
    {
        cli_print_invalid_input();
        return;
    }
    id601_set_efuse_burn_enable(2U == enable);
}
static inline void cli_cmd_function_ld_buff(void)
{
    const uint32_t daisy = gt_cli_request.val_1;
    const uint32_t ch = gt_cli_request.val_2;
    const uint32_t led = gt_cli_request.val_3;
    const uint32_t value = gt_cli_request.val_4;

    const uint16_t daisy_length = drv_id601_get_daisy_length();
    const uint16_t ch_size = drv_id601_get_ch_size();
    const uint16_t led_per_ch = drv_id601_get_led_per_ch();
    const uint16_t ld_size_max = id601_get_ld_size_max();

    if ((0U == daisy) || (0U == ch) || (0U == led))
    {
        cli_print_command_usage(CLI_CMD_LD_BUFF);
        return;
    }
    if (daisy > daisy_length || ch > ch_size || led > led_per_ch || value > ld_size_max)
    {
        cli_print_invalid_input();
        return;
    }
    id601_set_ld_buffer(daisy - 1U, ch - 1U, led - 1U, (uint16_t)value);
}
static inline void cli_cmd_function_ld_config_fault_read_enable(void)
{
    const uint32_t enable = gt_cli_request.val_1;
    const bool is_enable = (2U == enable);
    if (0U == enable)
    {
        cli_print_command_usage(CLI_CMD_LD_CONFIG_FAULT_READ_ENABLE);
        return;
    }
    if (enable > 2U)
    {
        cli_print_invalid_input();
        return;
    }
    if (true ==is_enable)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Fault Read Enabled");
        id601_config_fault_read(true);
    }
    else
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Fault Read Disabled");
        id601_config_fault_read(false);
    }
}
static inline void cli_cmd_function_ld_config_ld_trans_enable(void)
{
    const uint32_t enable = gt_cli_request.val_1;
    const bool is_enable = (2U == enable);
    if (0U == enable)
    {
        cli_print_command_usage(CLI_CMD_LD_CONFIG_LD_TRANS_ENABLE);
        return;
    }
    if (enable > 2U)
    {
        cli_print_invalid_input();
        return;
    }
    if (true == is_enable)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] LD Transfer Enabled");
        id601_config_ld_transfer(true);
    }
    else
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] LD Transfer Disabled");
        id601_config_ld_transfer(false);
    }
}
static inline void cli_cmd_function_ld_config_pwm_res(void)
{
    const uint32_t pwm_res = gt_cli_request.val_1;

    if (0U == pwm_res)
    {
        cli_print_command_usage(CLI_CMD_LD_CONFIG_PWM_RES);
        return;
    }
    if (pwm_res > 2U)
    {
        cli_print_invalid_input();
        return;
    }
    id601_ld_control_reg_t config =
    {
        .sv_no = ID601_SV_NO,
        .ld_type = ID601_LD_TYPE_DEFAULT,
        .delay_ch_en = ID601_DELAY_CH_EN_DEFAULT,
        .syncmode = ID601_SYNCMODE_DEFAULT,
        .pwm_res = ID601_PWM_RES_DEFAULT,
        .ld_dir = ID601_LD_DIR_DEFAULT,
        .ld_mode = ID601_LD_MODE_DEFAULT
    };
    config.pwm_res = (uint8_t)(pwm_res - 1U);
    if (0U == config.pwm_res)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Set PWM RES to 12bit");
    }
    if (1U == config.pwm_res)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [CLI] Set PWM RES to 14bit");
    }
    id601_config_ld_control(&config);
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
        case CLI_CMD_TRIM_MANUAL:
        {
            cli_cmd_function_trim_manual();
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
        case CLI_CMD_LD_BUFF:
        {
            cli_cmd_function_ld_buff();
            break;
        }
        case CLI_CMD_LD_CONFIG_FAULT_READ_ENABLE:
        {
            cli_cmd_function_ld_config_fault_read_enable();
            break;
        }
        case CLI_CMD_LD_CONFIG_LD_TRANS_ENABLE:
        {
            cli_cmd_function_ld_config_ld_trans_enable();
            break;
        }
        case CLI_CMD_LD_CONFIG_PWM_RES:
        {
            cli_cmd_function_ld_config_pwm_res();
            break;
        }
        default:
        {
            // Handle unknown command
            cli_print_invalid_input();
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
