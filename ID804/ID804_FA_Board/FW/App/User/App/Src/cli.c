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
#include "drv_timer.h"
#include "drv_id804.h"
#include "id804_metadata.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_CLI_CMD_LIST
{
    CLI_CMD_NONE = 0U,
    CLI_CMD_UNKNOWN,
    CLI_CMD_HELP,
    CLI_CMD_RESET,
    CLI_CMD_ID804_VDD,
    CLI_CMD_ID804_VLED,
    CLI_CMD_ID804_TM0,
    CLI_CMD_ID804_SIO1,
    CLI_CMD_ID804_SIO2,
    CLI_CMD_ID804_BOOT_SINGLE,
    CLI_CMD_ID804_BOOT_FORWARD,
    CLI_CMD_ID804_BOOT_BACKWARD,
    CLI_CMD_ID804_ME,
    CLI_CMD_ID804_I2C,

    CLI_CMD_ID804_RESET,
    CLI_CMD_ID804_INITBIDIR,
    CLI_CMD_ID804_CLRERROR,
    CLI_CMD_ID804_GOSLEEP,
    CLI_CMD_ID804_GOACTIVE,
    CLI_CMD_ID804_GODEEPSLEEP,

    CLI_CMD_ID804_TEST_IIH_IIL,
    CLI_CMD_ID804_TEST_V_TEMP_SEN,
    CLI_CMD_ID804_TEST_V_BGR,
    CLI_CMD_ID804_TEST_1V5_LDO,
    CLI_CMD_ID804_TEST_OSC,
    CLI_CMD_ID804_TEST_ADC_GAIN,
    CLI_CMD_ID804_TEST_ADC_TEMP,
    CLI_CMD_ID804_TEST_DRIVER_R,
    CLI_CMD_ID804_TEST_DRIVER_G,
    CLI_CMD_ID804_TEST_DRIVER_B,

    CLI_CMD_ID804_TEST_DEEPSLEEP,
    CLI_CMD_ID804_TEST_SLEEP,
    CLI_CMD_ID804_TEST_ACTIVE,
    CLI_CMD_ID804_TEST_PROTOCOL_CAN,
    CLI_CMD_ID804_TEST_PROTOCOL_LVDS,

    CLI_CMD_ID804_TEST_VDD_OSC,
    CLI_CMD_ID804_TEST_UVLO,
    CLI_CMD_ID804_TEST_R_OPEN,
    CLI_CMD_ID804_TEST_R_SHORT,
    CLI_CMD_ID804_TEST_G_OPEN,
    CLI_CMD_ID804_TEST_G_SHORT,
    CLI_CMD_ID804_TEST_B_OPEN,
    CLI_CMD_ID804_TEST_B_SHORT,

    CLI_CMD_ID804_TEST_SIO1_SE,
    CLI_CMD_ID804_TEST_SIO2_SE,
    CLI_CMD_ID804_TEST_SIO1_LVDS_PD_RES,
    CLI_CMD_ID804_TEST_SIO1_LVDS_TX,
    CLI_CMD_ID804_TEST_SIO2_LVDS_PD_RES,
    CLI_CMD_ID804_TEST_SIO2_LVDS_TX,

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
    /* ========================================================================================================= */
    /* Command              Command ID                      Description                                          */
    /* ========================================================================================================= */
    { "help",                   CLI_CMD_HELP,                           "Display help information"                                                                  },
    { "?",                      CLI_CMD_HELP,                           "Display help information"                                                                  },
    { "reset",                  CLI_CMD_RESET,                          "Reset the system"                                                                          },

    { "id804_vdd",              CLI_CMD_ID804_VDD,                      "Control ID804 VDD <0: OFF, 1: 5V, 2: 5.5V>"                                                },
    { "id804_vled",             CLI_CMD_ID804_VLED,                     "Control ID804 VLED <0: OFF, 1: ON>"                                                        },
    { "id804_tm0",              CLI_CMD_ID804_TM0,                      "Control ID804 TM0 <0: GND, 1: VDD>"                                                        },
    { "id804_sio1",             CLI_CMD_ID804_SIO1,                     "Control ID804 SIO1 interface <1: MCU, 2: CAN, 3: LVDS, 4: I2C> <1: ENABLE, 0: DISABLE>"    },
    { "id804_sio2",             CLI_CMD_ID804_SIO2,                     "Control ID804 SIO2 interface <1: MCU, 2: CAN, 3: LVDS, 4: EOL> <1: ENABLE, 0: DISABLE>"    },

    { "id804_boot_single",      CLI_CMD_ID804_BOOT_SINGLE,              "ID804 boot process <0: MCU,  1: I2C>"                                                      },
    { "id804_boot_forward",     CLI_CMD_ID804_BOOT_FORWARD,             "ID804 boot process <0: LVDS, 1: CAN>"                                                      },
    { "id804_boot_backward",    CLI_CMD_ID804_BOOT_BACKWARD,            "ID804 boot process <0: LVDS, 1: CAN>"                                                      },

    { "id804_reset",            CLI_CMD_ID804_RESET,                    "ID804 Reset <Dev Addr>"                                                                    },
    { "id804_initbidir",        CLI_CMD_ID804_INITBIDIR,                "ID804 Initbidir <Dev Addr>"                                                                },
    { "id804_clrerror",         CLI_CMD_ID804_CLRERROR,                 "ID804 Clear Error <Dev Addr>"                                                              },
    { "id804_gosleep",          CLI_CMD_ID804_GOSLEEP,                  "ID804 Go Sleep <Dev Addr>"                                                                 },
    { "id804_goactive",         CLI_CMD_ID804_GOACTIVE,                 "ID804 Go Active <Dev Addr>"                                                                },
    { "id804_godeepsleep",      CLI_CMD_ID804_GODEEPSLEEP,              "ID804 Go Deep Sleep <Dev Addr>"                                                            },

    { "id804_me",               CLI_CMD_ID804_ME,                       "ID804 ME interface <0: WRITE, 1: READ> <Dev Addr> <CMD> <Data: only for WRITE>"            },
    { "id804_i2c",              CLI_CMD_ID804_I2C,                      "ID804 I2C interface <0: WRITE, 1: READ> <Address> <Data: only for WRITE>"                  },

    { "iih_iil_test",           CLI_CMD_ID804_TEST_IIH_IIL,             "ID804 IIH IIL Test"                                                                        },
    { "bgr_test",               CLI_CMD_ID804_TEST_V_TEMP_SEN,          "ID804 V_TEMP_SEN Test"                                                                     },
    { "bgr_r1_r2_test",         CLI_CMD_ID804_TEST_V_BGR,               "ID804 V_BGR Test"                                                                          },
    { "1p5_ldo_test",           CLI_CMD_ID804_TEST_1V5_LDO,             "ID804 1V5 LDO Test"                                                                        },
    { "osc_test",               CLI_CMD_ID804_TEST_OSC,                 "ID804 OSC Test"                                                                            },
    { "adc_gain_test",          CLI_CMD_ID804_TEST_ADC_GAIN,            "ID804 ADC Gain Test"                                                                       },
    { "adc_temp_test",          CLI_CMD_ID804_TEST_ADC_TEMP,            "ID804 ADC Temp Test"                                                                       },
    { "driver_r_test",          CLI_CMD_ID804_TEST_DRIVER_R,            "ID804 Driver R Test"                                                                       },
    { "driver_g_test",          CLI_CMD_ID804_TEST_DRIVER_G,            "ID804 Driver G Test"                                                                       },
    { "driver_b_test",          CLI_CMD_ID804_TEST_DRIVER_B,            "ID804 Driver B Test"                                                                       },

    { "deep_sleep_test",        CLI_CMD_ID804_TEST_DEEPSLEEP,           "ID804 Deep Sleep Test"                                                                     },
    { "sleep_test",             CLI_CMD_ID804_TEST_SLEEP,               "ID804 Sleep Test"                                                                          },
    { "active_test",            CLI_CMD_ID804_TEST_ACTIVE,              "ID804 Active Test"                                                                         },
    { "protocol_can_test",      CLI_CMD_ID804_TEST_PROTOCOL_CAN,        "ID804 Protocol CAN Test"                                                                   },
    { "protocol_lvds_test",     CLI_CMD_ID804_TEST_PROTOCOL_LVDS,       "ID804 Protocol LVDS Test"                                                                  },

    { "vdd_osc_test",           CLI_CMD_ID804_TEST_VDD_OSC,             "ID804 VDD OSC Test"                                                                        },
    { "uvlo_test",              CLI_CMD_ID804_TEST_UVLO,                "ID804 UVLO Test"                                                                           },

    { "r_open_test",            CLI_CMD_ID804_TEST_R_OPEN,              "ID804 R Open Test"                                                                         },
    { "g_open_test",            CLI_CMD_ID804_TEST_G_OPEN,              "ID804 G Open Test"                                                                         },
    { "b_open_test",            CLI_CMD_ID804_TEST_B_OPEN,              "ID804 B Open Test"                                                                         },
    { "r_short_test",           CLI_CMD_ID804_TEST_R_SHORT,             "ID804 R Short Test"                                                                        },
    { "g_short_test",           CLI_CMD_ID804_TEST_G_SHORT,             "ID804 G Short Test"                                                                        },
    { "b_short_test",           CLI_CMD_ID804_TEST_B_SHORT,             "ID804 B Short Test"                                                                        },

    { "se_sio1_test",           CLI_CMD_ID804_TEST_SIO1_SE,             "ID804 SIO1 SE Test"                                                                        },
    { "se_sio2_test",           CLI_CMD_ID804_TEST_SIO2_SE,             "ID804 SIO2 SE Test"                                                                        },
    { "lvds_sio1_pd_res_test",  CLI_CMD_ID804_TEST_SIO1_LVDS_PD_RES,    "ID804 SIO1 LVDS PD Res Test"                                                               },
    { "lvds_sio1_tx_test",      CLI_CMD_ID804_TEST_SIO1_LVDS_TX,        "ID804 SIO1 LVDS TX Test"                                                                   },
    { "lvds_sio2_pd_res_test",  CLI_CMD_ID804_TEST_SIO2_LVDS_PD_RES,    "ID804 SIO2 LVDS PD Res Test"                                                               },
    { "lvds_sio2_tx_test",      CLI_CMD_ID804_TEST_SIO2_LVDS_TX,        "ID804 SIO2 LVDS TX Test"                                                                   },
};

static cli_request_t gt_cli_request;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
    drv_uart_printf("\r\n  %s", "ID804 FA Board");
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
    id804_comm_result_t comm_result = ID804_COMM_NONE;
    switch (gt_cli_request.last_command)
    {
        case CLI_CMD_NONE:
        {
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
        case CLI_CMD_ID804_BOOT_SINGLE:
        {
            if (0U == gt_cli_request.val_1)
            {
                id804_boot_me();
                drv_uart_printf("\r\n    ID804 boot with \"SINGLE MCU\" executed.");
            }
            else if (1U == gt_cli_request.val_1)
            {
                id804_boot_i2c();
                drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");
            }
            else
            {
                drv_uart_printf("\r\n    ID804 boot SINGLE (%u) command not recognized.", gt_cli_request.val_1);
            }
            break;
        }
        case CLI_CMD_ID804_BOOT_FORWARD:
        {
            if (0U == gt_cli_request.val_1)
            {
                id804_boot_lvds(ID804_COMM_FORWARD);
                drv_uart_printf("\r\n    ID804 boot with \"FORWARD LVDS\" executed.");
            }
            else if (1U == gt_cli_request.val_1)
            {
                id804_boot_can(ID804_COMM_FORWARD);
                drv_uart_printf("\r\n    ID804 boot with \"FORWARD CAN\" executed.");
            }
            else
            {
                drv_uart_printf("\r\n    ID804 boot FORWARD (%u) command not recognized.", gt_cli_request.val_1);
            }
            break;
        }
        case CLI_CMD_ID804_BOOT_BACKWARD:
        {
            if (0U == gt_cli_request.val_1)
            {
                id804_boot_lvds(ID804_COMM_BACKWARD);
                drv_uart_printf("\r\n    ID804 boot with \"BACKWARD LVDS\" executed.");
            }
            else if (1U == gt_cli_request.val_1)
            {
                id804_boot_can(ID804_COMM_BACKWARD);
                drv_uart_printf("\r\n    ID804 boot with \"BACKWARD CAN\" executed.");
            }
            else
            {
                drv_uart_printf("\r\n    ID804 boot BACKWARD (%u) command not recognized.", gt_cli_request.val_1);
            }
            break;
        }
        case CLI_CMD_ID804_RESET:
        {
            comm_result = id804_write(gt_cli_request.val_1, ID804_CMD_RESET, 0U);
            cli_print_id804_comm_result(comm_result, gt_cli_request.val_1, ID804_CMD_RESET, 0U);
            break;
        }
        case CLI_CMD_ID804_INITBIDIR:
        {
            uint64_t dummy = 0U;
            comm_result = id804_read(gt_cli_request.val_1, ID804_CMD_INITBIDIR, &dummy);
            cli_print_id804_comm_result(comm_result, gt_cli_request.val_1, ID804_CMD_INITBIDIR, dummy);
            break;
        }
        case CLI_CMD_ID804_CLRERROR:
        {
            comm_result = id804_write(gt_cli_request.val_1, ID804_CMD_CLRERROR, 0U);
            cli_print_id804_comm_result(comm_result, gt_cli_request.val_1, ID804_CMD_CLRERROR, 0U);
            break;
        }
        case CLI_CMD_ID804_GOSLEEP:
        {
            comm_result = id804_write(gt_cli_request.val_1, ID804_CMD_GOSLEEP, 0U);
            cli_print_id804_comm_result(comm_result, gt_cli_request.val_1, ID804_CMD_GOSLEEP, 0U);
            break;
        }
        case CLI_CMD_ID804_GOACTIVE:
        {
            comm_result = id804_write(gt_cli_request.val_1, ID804_CMD_GOACTIVE, 0U);
            cli_print_id804_comm_result(comm_result, gt_cli_request.val_1, ID804_CMD_GOACTIVE, 0U);
            break;
        }
        case CLI_CMD_ID804_GODEEPSLEEP:
        {
            comm_result = id804_write(gt_cli_request.val_1, ID804_CMD_GODEEPSLEEP, 0U);
            cli_print_id804_comm_result(comm_result, gt_cli_request.val_1, ID804_CMD_GODEEPSLEEP, 0U);
            break;
        }
        case CLI_CMD_ID804_ME:
        {
            uint16_t dev_addr = (uint16_t)gt_cli_request.val_2;
            uint16_t target = (uint16_t)gt_cli_request.val_3;
            uint64_t data = gt_cli_request.val_4;
            if (GET == gt_cli_request.val_1)
            {
                comm_result = id804_read(dev_addr, target, &data);
                cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            }
            else if (SET == gt_cli_request.val_1)
            {
                comm_result = id804_write(dev_addr, target, data);
                cli_print_id804_comm_result(comm_result, dev_addr, target, data);
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
            uint64_t data = gt_cli_request.val_3;

            if (GET == gt_cli_request.val_1)
            {
                comm_result = id804_read(dev_addr, target, &data);
                cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            }
            else if (SET == gt_cli_request.val_1)
            {
                comm_result = id804_write(dev_addr, target, data);
                cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            }
            else
            {
                drv_uart_printf("\r\n    ID804 I2C communication not recognized.");
            }
            break;
        }
        case CLI_CMD_ID804_TEST_IIH_IIL:
        {
            drv_gpio_id804_vcc(ID804_VCC_5V0);
            drv_uart_printf("\r\n    ID804 VDD set to 5V.");

            drv_gpio_id804_vled(ID804_VLED_ON);
            drv_uart_printf("\r\n    ID804 VLED turned ON.");

            drv_gpio_id804_sio1_lvds(ID804_IO_CON);
            drv_uart_printf("\r\n    ID804 SIO1 LVDS set to ENABLE.");

            drv_gpio_id804_sio2_lvds(ID804_IO_CON);
            drv_uart_printf("\r\n    ID804 SIO2 LVDS set to ENABLE.");
            break;
        }
        case CLI_CMD_ID804_TEST_V_TEMP_SEN:
        {
            id804_boot_i2c();
            drv_gpio_buffer_output_disable();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0x808U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_V_BGR:
        {
            id804_boot_i2c();
            drv_gpio_buffer_output_disable();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0x818U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_1V5_LDO:
        {
            id804_boot_i2c();
            drv_gpio_buffer_output_disable();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0x820U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_OSC:
        {
            id804_boot_i2c();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0xC00U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_ADC_GAIN:
        {
            id804_boot_i2c();
            drv_gpio_buffer_output_disable();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0x810U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_ADC_TEMP:
        {
            id804_boot_i2c();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            drv_timer_delay_ms(1000U);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x03U;
            data = 0x000U;
            comm_result = id804_read(dev_addr, target, &data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            float temp = (30 + ((float)data - 440) / 4.4f);
            drv_uart_printf("\r\n    [ID804] [Temp] %.3f\r\n", (double)temp);
            break;
        }
        case CLI_CMD_ID804_TEST_DRIVER_R:
        {
            id804_boot_i2c();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            drv_gpio_id804_vled(ID804_VLED_OFF);
            drv_uart_printf("\r\n    ID804 VLED turned OFF.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE3U;
            data = 0xA5AU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xF1U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x0FU;
            data = 0xE93U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0x804U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x20U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x21U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_DRIVER_G:
        {
            id804_boot_i2c();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            drv_gpio_id804_vled(ID804_VLED_OFF);
            drv_uart_printf("\r\n    ID804 VLED turned OFF.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE3U;
            data = 0xA5AU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xF2U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x0FU;
            data = 0xE93U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0x802U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x20U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x21U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_DRIVER_B:
        {
            id804_boot_i2c();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            drv_gpio_id804_vled(ID804_VLED_OFF);
            drv_uart_printf("\r\n    ID804 VLED turned OFF.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE3U;
            data = 0xA5AU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xF3U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x0FU;
            data = 0xE93U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0x801U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x20U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0x21U;
            data = 0xFFFU;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_DEEPSLEEP:
        {
            id804_boot_me();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE MCU\" executed.");

            uint64_t dummy = 0U;
            comm_result = id804_read(1U, ID804_CMD_INITBIDIR, &dummy);
            cli_print_id804_comm_result(comm_result, 1U, ID804_CMD_INITBIDIR, dummy);

            comm_result = id804_write(0U, ID804_CMD_GODEEPSLEEP, 0U);
            cli_print_id804_comm_result(comm_result, 0U, ID804_CMD_GODEEPSLEEP, 0U);
            break;
        }
        case CLI_CMD_ID804_TEST_SLEEP:
        {
            id804_boot_me();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE MCU\" executed.");

            uint64_t dummy = 0U;
            comm_result = id804_read(1U, ID804_CMD_INITBIDIR, &dummy);
            cli_print_id804_comm_result(comm_result, 1U, ID804_CMD_INITBIDIR, dummy);

            comm_result = id804_write(0U, ID804_CMD_GOSLEEP, 0U);
            cli_print_id804_comm_result(comm_result, 0U, ID804_CMD_GOSLEEP, 0U);
            break;
        }
        case CLI_CMD_ID804_TEST_ACTIVE:
        {
            id804_boot_me();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE MCU\" executed.");

            uint64_t dummy = 0U;
            comm_result = id804_read(1U, ID804_CMD_INITBIDIR, &dummy);
            cli_print_id804_comm_result(comm_result, 1U, ID804_CMD_INITBIDIR, dummy);

            comm_result = id804_write(0U, ID804_CMD_GOACTIVE, 0U);
            cli_print_id804_comm_result(comm_result, 0U, ID804_CMD_GOACTIVE, 0U);
            break;
        }
        case CLI_CMD_ID804_TEST_PROTOCOL_CAN:
        {
            id804_boot_can(ID804_COMM_FORWARD);
            drv_uart_printf("\r\n    ID804 boot with \"FORWARD CAN\" executed.");

            uint64_t dummy = 0U;
            comm_result = id804_read(1U, ID804_CMD_INITBIDIR, &dummy);
            cli_print_id804_comm_result(comm_result, 1U, ID804_CMD_INITBIDIR, dummy);

            uint16_t dev_addr = 2U;
            uint16_t target = 0x41U;
            uint64_t data = 0x00U;
            comm_result = id804_read(dev_addr, target, &data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            drv_uart_printf("\r\n    [ID804] PROTOCOL CAN Result: ", data);
            for (uint16_t i = 0; i < 6; ++i)
            {
                drv_uart_printf("%02X ", (uint8_t)(data >> (40 - i * 8)) & 0xFFU);
            }
            break;
        }
        case CLI_CMD_ID804_TEST_PROTOCOL_LVDS:
        {
            id804_boot_lvds(ID804_COMM_FORWARD);
            drv_uart_printf("\r\n    ID804 boot with \"FORWARD LVDS\" executed.");

            uint64_t dummy = 0U;
            comm_result = id804_read(1U, ID804_CMD_INITBIDIR, &dummy);
            cli_print_id804_comm_result(comm_result, 1U, ID804_CMD_INITBIDIR, dummy);

            uint16_t dev_addr = 2U;
            uint16_t target = 0x41U;
            uint64_t data = 0x00U;
            comm_result = id804_read(dev_addr, target, &data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            drv_uart_printf("\r\n    [ID804] PROTOCOL LVDS Result: ", data);
            for (uint16_t i = 0; i < 6; ++i)
            {
                drv_uart_printf("%02X ", (uint8_t)(data >> (40 - i * 8)) & 0xFFU);
            }
            break;
        }
        case CLI_CMD_ID804_TEST_VDD_OSC:
        {
            id804_boot_i2c();
            drv_gpio_buffer_output_disable();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE0U;
            data = 0x830U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_UVLO:
        {
            id804_boot_i2c();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE1U;
            data = 0x001U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_R_OPEN:
        case CLI_CMD_ID804_TEST_G_OPEN:
        case CLI_CMD_ID804_TEST_B_OPEN:
        {
            id804_boot_i2c();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE1U;
            if (gt_cli_request.last_command == CLI_CMD_ID804_TEST_R_OPEN)
            {
                data = 0x002U;
            }
            else if (gt_cli_request.last_command == CLI_CMD_ID804_TEST_G_OPEN)
            {
                data = 0x003U;
            }
            else if (gt_cli_request.last_command == CLI_CMD_ID804_TEST_B_OPEN)
            {
                data = 0x004U;
            }
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_R_SHORT:
        case CLI_CMD_ID804_TEST_G_SHORT:
        case CLI_CMD_ID804_TEST_B_SHORT:
        {
            id804_boot_i2c();
            drv_uart_printf("\r\n    ID804 boot with \"SINGLE I2C\" executed.");

            uint16_t dev_addr = ID804_DEV_ADDR_DUMMY;
            uint16_t target = 0xE0U;
            uint64_t data = 0x800U;
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);

            dev_addr = ID804_DEV_ADDR_DUMMY;
            target = 0xE1U;
            if (gt_cli_request.last_command == CLI_CMD_ID804_TEST_R_SHORT)
            {
                data = 0x005U;
            }
            else if (gt_cli_request.last_command == CLI_CMD_ID804_TEST_G_SHORT)
            {
                data = 0x006U;
            }
            else if (gt_cli_request.last_command == CLI_CMD_ID804_TEST_B_SHORT)
            {
                data = 0x007U;
            }
            comm_result = id804_write(dev_addr, target, data);
            cli_print_id804_comm_result(comm_result, dev_addr, target, data);
            break;
        }
        case CLI_CMD_ID804_TEST_SIO1_SE:
        {
            drv_uart_printf("\r\n    SIO1_SE: Not Yet.");
            break;
        }
        case CLI_CMD_ID804_TEST_SIO2_SE:
        {
            drv_uart_printf("\r\n    SIO2_SE: Not Yet.");
            break;
        }
        case CLI_CMD_ID804_TEST_SIO1_LVDS_PD_RES:
        {
            drv_uart_printf("\r\n    SIO1_LVDS_PD_RES: Not Yet.");
            break;
        }
        case CLI_CMD_ID804_TEST_SIO1_LVDS_TX:
        {
            drv_uart_printf("\r\n    SIO1_LVDS_TX: Not Yet.");
            break;
        }
        case CLI_CMD_ID804_TEST_SIO2_LVDS_PD_RES:
        {
            drv_uart_printf("\r\n    SIO2_LVDS_PD_RES: Not Yet.");
            break;
        }
        case CLI_CMD_ID804_TEST_SIO2_LVDS_TX:
        {
            drv_uart_printf("\r\n    SIO2_LVDS_TX: Not Yet.");
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
