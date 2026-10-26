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
#include <stdio.h>
/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_timer.h"
#include "drv_uart.h"
#include "drv_gpio.h"
#include "cmsis_os.h"
#include "common_def.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_ID601_TEST_ANA_EN
{
    ID601_TEST_ANA_IREF_TEST            = 0x00U, // 0x00
    ID601_TEST_ANA_IREF2_TEST           = 0x01U, // 0x01
    ID601_TEST_ANA_VREF1P5_BGR2_RES     = 0x02U, // 0x02
    ID601_TEST_ANA_VDD1P5_DIG_RES       = 0x03U, // 0x03
    ID601_TEST_ANA_VDD1P5_DAC_RES       = 0x04U, // 0x04
    ID601_TEST_ANA_VDD1P5_FLL_RES       = 0x05U, // 0x05
    ID601_TEST_ANA_VREF_TEST            = 0x06U, // 0x06
    ID601_TEST_ANA_VREF2_TEST           = 0x07U, // 0x07
    ID601_TEST_ANA_TEST_VREF2_ZTC_O1    = 0x08U, // 0x08
    ID601_TEST_ANA_TEST_VREF2_ZTC_O2    = 0x09U, // 0x09
    ID601_TEST_ANA_VDD_UV               = 0x0AU, // 0x0A
    ID601_TEST_ANA_VSS_0B               = 0x0BU, // 0x0B
    ID601_TEST_ANA_VSS_0C               = 0x0CU, // 0x0C
    ID601_TEST_ANA_1P5_UVOV_TEST        = 0x0DU, // 0x0D
    ID601_TEST_ANA_1P5_DAC_UVOV_TEST    = 0x0EU, // 0x0E
    ID601_TEST_ANA_1P5_OSC_UVOV_TEST    = 0x0FU, // 0x0F
    ID601_TEST_ANA_VSS_10               = 0x10U, // 0x10
    ID601_TEST_ANA_VSS_11               = 0x11U, // 0x11
    ID601_TEST_ANA_VSS_12               = 0x12U, // 0x12
    ID601_TEST_ANA_VSS_13               = 0x13U, // 0x13
    ID601_TEST_ANA_VSS_14               = 0x14U, // 0x14
    ID601_TEST_ANA_VSS_15               = 0x15U, // 0x15
    ID601_TEST_ANA_VREF_FB1             = 0x16U, // 0x16
    ID601_TEST_ANA_VREF_FB2             = 0x17U, // 0x17
    ID601_TEST_ANA_VREF_FB3             = 0x18U, // 0x18
    ID601_TEST_ANA_VREF2_OPEN           = 0x19U, // 0x19
    ID601_TEST_ANA_VREF2_SHORT1         = 0x1AU, // 0x1A
    ID601_TEST_ANA_VREF2_SHORT2         = 0x1BU, // 0x1B
    ID601_TEST_ANA_VREF2_SHORT3         = 0x1CU, // 0x1C
    ID601_TEST_ANA_VSS_1D               = 0x1DU, // 0x1D
    ID601_TEST_ANA_VSS_1E               = 0x1EU, // 0x1E
    ID601_TEST_ANA_VSS_1F               = 0x1FU, // 0x1F
} id601_test_ana_en_t;

typedef enum tag_ID601_MAX_CURRENT_LEVEL
{
    ID601_MAX_CURRENT_LEVEL_04mA = 0x00U,  // 0x00
    ID601_MAX_CURRENT_LEVEL_08mA = 0x01U,  // 0x01
    ID601_MAX_CURRENT_LEVEL_12mA = 0x02U,  // 0x02
    ID601_MAX_CURRENT_LEVEL_16mA = 0x03U,  // 0x03
    ID601_MAX_CURRENT_LEVEL_20mA = 0x04U,  // 0x04
    ID601_MAX_CURRENT_LEVEL_24mA = 0x05U,  // 0x05
    ID601_MAX_CURRENT_LEVEL_28mA = 0x06U,  // 0x06
    ID601_MAX_CURRENT_LEVEL_32mA = 0x07U,  // 0x07
    ID601_MAX_CURRENT_LEVEL_36mA = 0x08U,  // 0x08
    ID601_MAX_CURRENT_LEVEL_40mA = 0x09U,  // 0x09
    ID601_MAX_CURRENT_LEVEL_44mA = 0x0AU,  // 0x0A
    ID601_MAX_CURRENT_LEVEL_48mA = 0x0BU,  // 0x0B
    ID601_MAX_CURRENT_LEVEL_52mA = 0x0CU,  // 0x0C
    ID601_MAX_CURRENT_LEVEL_56mA = 0x0DU,  // 0x0D
    ID601_MAX_CURRENT_LEVEL_60mA = 0x0EU,  // 0x0E
    ID601_MAX_CURRENT_LEVEL_64mA = 0x0FU,  // 0x0F
} id601_max_current_level_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ID601_DAISY_LENGTH_INITIAL          (1U)
#define ID601_DAISY_LENGTH_MAX              (5U)
#define ID601_BROADCAST                     (0U)
#define ID601_CHANNEL_SIZE                  (12U)

#define ID601_SERIAL_TIMEOUT_MS             (100U)

#define ID601_SERIAL_RATIO_BIT0             (1U)
#define ID601_SERIAL_RATIO_BIT1             (2U)
#define ID601_SERIAL_RATIO_SUM              (ID601_SERIAL_RATIO_BIT0 + ID601_SERIAL_RATIO_BIT1)

/********************** ID601 CRC **********************/
#define ID601_CRC8_BIT_COUNT                (8U)
#define ID601_CRC8_Pos                      (0U)
#define ID601_CRC8_Msk                      (0x00FFU << ID601_CRC8_Pos)

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
#define ID601_WRITE_ADDRESS_Msk             (0x003FU << ID601_WRITE_ADDRESS_Pos)

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

#define ID601_TRIM_REG_IREF1_MAX            (0x001FU)
#define ID601_TRIM_REG_IREF2_MAX            (0x001FU)
#define ID601_TRIM_REG_LDO_BGR2_CTL_MAX     (0x001FU)
#define ID601_TRIM_REG_LDO_CTL_MAX          (0x000FU)
#define ID601_TRIM_REG_LDO_DAC_CTL_MAX      (0x001FU)
#define ID601_TRIM_REG_LDO_OSC_CTL_MAX      (0x000FU)
#define ID601_TRIM_REG_OFS_TEMP_MAX         (0x000FU)
#define ID601_TRIM_REG_OSC_RCTL_MAX         (0x001FU)
#define ID601_TRIM_REG_GAIN_MAX             (0x007FU)
#define ID601_TRIM_REG_OFFSET_MAX           (0x01FFU)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static osMutexId_t gh_id601_mutex_id;

static const osMutexAttr_t gs_id601_mutex_attr = {
    .name      = "id601Mutex",
    .attr_bits = osMutexPrioInherit,   /* 우선순위 역전 방지 */
    .cb_mem    = NULL,
    .cb_size   = 0U,
};

static uint16_t gn_id601_tx_ccr_buffer[ID601_TX_CCR_BUFFER_CAPACITY] = { 0U };
static uint16_t gn_id601_rx_ccr_rise_buffer[ID601_RX_CCR_BUFFER_CAPACITY] = { 0U };
static uint16_t gn_id601_rx_ccr_fall_buffer[ID601_RX_CCR_BUFFER_CAPACITY] = { 0U };

static uint16_t gn_id601_serial_bit0_ccr;
static uint16_t gn_id601_serial_bit1_ccr;

static uint16_t gn_id601_daisy_length;

static id601_crc_func_t gp_id601_crc_func = NULL;

const static uint16_t gn_id601_general_regs_initial_code[ID601_ADDR_GENERAL_MAX_COUNT] =
{
    [ID601_ADDR_GENERAL_LD_CONTROL] = 0x000,
    [ID601_ADDR_GENERAL_LD_SIZE] = 0x000,
    [ID601_ADDR_GENERAL_PWMCLK_DIV1_2] = 0x000,
    [ID601_ADDR_GENERAL_PWMCLK_DIV2_3] = 0x000,
    [ID601_ADDR_GENERAL_CHANNEL_ENABLE] = 0x000,
    [ID601_ADDR_GENERAL_FAULT_CONTROL] = 0x03F,
    [ID601_ADDR_GENERAL_FB_LEVEL] = 0x000,
    [ID601_ADDR_GENERAL_SHORT_LEVEL] = 0x000,
    [ID601_ADDR_GENERAL_FAULT_STATUS0] = 0x000,
    [ID601_ADDR_GENERAL_FAULT_STATUS1] = 0x000,
    [ID601_ADDR_GENERAL_FAULT_STATUS2] = 0x000,
    [ID601_ADDR_GENERAL_FAULT_STATUS3] = 0x000,
    [ID601_ADDR_GENERAL_MAX_CURR_LEVEL] = 0x000,
    [ID601_ADDR_GENERAL_MAX_CURR_VREF1] = 0x000,
    [ID601_ADDR_GENERAL_MAX_CURR_VREF2] = 0x000,
    [ID601_ADDR_GENERAL_MAX_CURR_VREF3] = 0x000,
    [ID601_ADDR_GENERAL_CH_LD_TYPE] = 0x000,
    [ID601_ADDR_GENERAL_DELAY_CH1_2] = 0x000,
    [ID601_ADDR_GENERAL_DELAY_CH3_4] = 0x000,
    [ID601_ADDR_GENERAL_DELAY_CH5_6] = 0x000,
    [ID601_ADDR_GENERAL_DELAY_CH7_8] = 0x000,
    [ID601_ADDR_GENERAL_DELAY_CH9_10] = 0x000,
    [ID601_ADDR_GENERAL_DELAY_CH11_12] = 0x000,
    [ID601_ADDR_GENERAL_FAULT_CONFIGURATION] = 0x31F,
    [ID601_ADDR_GENERAL_BIST] = 0x000,
    [ID601_ADDR_GENERAL_TEST_MODE0] = 0x000,
    [ID601_ADDR_GENERAL_TEST_MODE1] = 0x000,
    [ID601_ADDR_GENERAL_SERIAL_CLK_GEN] = 0x318,
    [ID601_ADDR_GENERAL_SERIAL_LATENCY] = 0x080,
    [ID601_ADDR_GENERAL_V_MASK] = 0x000,
    [ID601_ADDR_GENERAL_SV_MASK] = 0x000,
    [ID601_ADDR_GENERAL_RSTCNT] = 0x000,
    [ID601_ADDR_GENERAL_TIMEOUT] = 0x3FF,
    [ID601_ADDR_GENERAL_FLLCNT1] = 0x000,
    [ID601_ADDR_GENERAL_FLLCNT2] = 0xC32,
    [ID601_ADDR_GENERAL_WR_PROTECT] = 0x555,
    [ID601_ADDR_GENERAL_NF_CONTROL] = 0x000,
    [ID601_ADDR_GENERAL_CHOP_EN] = 0x000,
    [ID601_ADDR_GENERAL_TEMP] = 0x029,
    [ID601_ADDR_GENERAL_OSC_FLL_MAN1] = 0x000,
    [ID601_ADDR_GENERAL_OSC_FLL_MAN2] = 0x808,
    [ID601_ADDR_GENERAL_OSC_SPREAD] = 0x000,
    [ID601_ADDR_GENERAL_CLOCK_GATE_EN] = 0x017,
    [ID601_ADDR_GENERAL_VREF_FIX1] = 0x000,
    [ID601_ADDR_GENERAL_VREF_FIX2] = 0x000,
    [ID601_ADDR_GENERAL_VREF_FIX3] = 0x000,
    [ID601_ADDR_GENERAL_TEST_ANA_EN] = 0x000,
    [ID601_ADDR_GENERAL_OTP_ACCESS1] = 0x000,
    [ID601_ADDR_GENERAL_OTP_ACCESS2] = 0x3FF,
    [ID601_ADDR_GENERAL_OTP_WRITE] = 0x004,
    [ID601_ADDR_GENERAL_OTP_RD_PROG] = 0x000,
    [ID601_ADDR_GENERAL_OTP_PROTECT] = 0x5A5,
    [ID601_ADDR_GENERAL_OP_MODE] = 0x004,
};

const static uint16_t gn_id601_mirror_regs_initial_code[ID601_ADDR_MIRROR_MAX_COUNT] =
{
    [ID601_ADDR_MIRROR1] = 0x000,
    [ID601_ADDR_MIRROR2] = 0x210,
    [ID601_ADDR_MIRROR3] = 0x210,
    [ID601_ADDR_MIRROR4] = 0x810,
    [ID601_ADDR_MIRROR5] = 0x810,
    [ID601_ADDR_MIRROR6] = 0x080,
    [ID601_ADDR_MIRROR_OFS_CH01] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH02] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH03] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH04] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH05] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH06] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH07] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH08] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH09] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH10] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH11] = 0x000,
    [ID601_ADDR_MIRROR_OFS_CH12] = 0x000,
    [ID601_ADDR_MIRROR_GAIN_CH01] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH02] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH03] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH04] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH05] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH06] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH07] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH08] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH09] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH10] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH11] = 0x040,
    [ID601_ADDR_MIRROR_GAIN_CH12] = 0x040,
    [ID601_ADDR_MIRROR_VERSION] = 0x000,
};

static uint16_t gn_id601_general_regs[ID601_ADDR_GENERAL_MAX_COUNT][ID601_DAISY_LENGTH_MAX];
static uint16_t gn_id601_mirror_regs[ID601_ADDR_MIRROR_MAX_COUNT][ID601_DAISY_LENGTH_MAX];
static uint16_t gn_id601_trim_search_mirror_regs[ID601_ADDR_MIRROR_MAX_COUNT];
static uint16_t gn_id601_fault;
static bool gb_id601_hvs_flag;
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

    drv_id601_set_daisy_length(ID601_DAISY_LENGTH_INITIAL);

    for (uint16_t i = 0U; i < ID601_ADDR_MIRROR_MAX_COUNT; ++i)
    {
        gn_id601_trim_search_mirror_regs[i] = gn_id601_mirror_regs_initial_code[i];
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [MCU] ID601 initialized successfully.");
}

void drv_id601_make_mutex(void)
{
    gh_id601_mutex_id = osMutexNew(&gs_id601_mutex_attr);
    if (NULL == gh_id601_mutex_id)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Failed to create ID601 mutex.", __func__);
    }
}

bool drv_id601_set_daisy_length(uint16_t daisy_length)
{
    if (daisy_length > ID601_DAISY_LENGTH_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Invalid daisy length.", __func__);
        return false;
    }
    gn_id601_daisy_length = daisy_length;
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Set Daisy length: %u.", daisy_length);
    return true;
}
uint16_t drv_id601_get_daisy_length(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Get Daisy length: %u.", gn_id601_daisy_length);
    return gn_id601_daisy_length;
}
bool drv_id601_set_hvs_flag(bool hvs_flag)
{
    gb_id601_hvs_flag = hvs_flag;
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Set HVS flag: %s.", hvs_flag ? "true" : "false");
    return true;
}
bool drv_id601_get_hvs_flag(void)
{
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Get HVS flag: %s.", gb_id601_hvs_flag ? "true" : "false");
    return gb_id601_hvs_flag;
}

bool drv_id601_set_crc_func(id601_crc_func_t crc_func)
{
    gp_id601_crc_func = crc_func;
    return true;
}

id601_crc_func_t drv_id601_get_crc_func(void)
{
    return gp_id601_crc_func;
}

static inline bool id601_lock(void)
{
    return (osOK == osMutexAcquire(gh_id601_mutex_id, ID601_SERIAL_TIMEOUT_MS));
}

static inline void id601_unlock(void)
{
    osMutexRelease(gh_id601_mutex_id);
}

static uint8_t id601_calculate_crc(uint32_t data, uint16_t size)
{
    if (0U == size)
    {
        return 0U;
    }

    uint8_t crc = ID601_CRC8_INIT;
    uint32_t mask = 0U;
    for (uint32_t i = 0U; i < size; i++)
    {
        mask |= (1UL << i);
    }
    data &= mask;
    int8_t repeat = (size - 1U);

    for (int i = repeat; i >= 0; i--)
    {
        uint8_t data_bit = (data >> i) & 0x01;
        uint8_t feedback = ((crc >> 7) & 0x01) ^ data_bit;

        crc <<= 1U;

        if (feedback)
            crc ^= ID601_CRC8_POLY;
    }

    return crc ^ ID601_CRC8_XOROUT;
}

static uint16_t id601_encode_ccr_buffer_from_serial(uint32_t serial, uint16_t* p_cnt, uint16_t length)
{
    if ((NULL == p_cnt) || (0U == length))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Invalid CCR buffer or length.", __func__);
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
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Invalid CCR buffer or length.", __func__);
        return false;
    }
    uint32_t rise_cnt_sum = 0U;
    for (uint32_t i = 1U; i < length; i++) // exclude the first element
    {
        rise_cnt_sum += p_rise[i];
    }

    const uint32_t sample_count = (uint32_t)(length - 1U);
    const uint32_t numerator    = (rise_cnt_sum + 1U) + sample_count;
    const uint16_t cnt_half_duty = (uint16_t)(numerator / (2U * sample_count));

    for (uint32_t i = 0U; i < length; i++)
    {
        if (p_fall[i] > cnt_half_duty)
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

static inline uint32_t id601_set_field(uint32_t reg_value, uint32_t field_msk, uint32_t field_pos, uint32_t field_value)
{
    reg_value &= ~field_msk;
    reg_value |= ((field_value << field_pos) & field_msk);

    return reg_value;
}

static inline uint32_t id601_get_field(uint32_t reg_value, uint32_t field_msk, uint32_t field_pos)
{
    return ((reg_value & field_msk) >> field_pos);
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
    if (NULL == p_data)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Invalid data pointer.", __func__);
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
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Invalid data pointer.", __func__);
        return false;
    }

    uint16_t frame_length = ID601_READ_RECV_PAYLOAD_BIT_COUNT;
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
        if (false == id601_decode_serial_from_ccr_buffer(p_rise, p_fall, &serial_data, frame_length))
        {
            return false;
        }
        uint32_t payload = serial_data;

        uint16_t command = 0U;
        uint16_t ID = 0U;
        uint16_t data = 0U;
        uint16_t recv_crc = 0U;

        if (NULL != p_crc_func)
        {
            recv_crc = (serial_data & ID601_CRC8_Msk) >> ID601_CRC8_Pos;
            payload = (serial_data >> ID601_CRC8_BIT_COUNT);
            uint16_t calc_crc = p_crc_func(payload, ID601_READ_RECV_PAYLOAD_BIT_COUNT);
            if (recv_crc != calc_crc)
            {
                drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: CRC check failed. recv_crc: 0x%02X, calc_crc: 0x%02X", __func__, recv_crc, calc_crc);
                return false;
            }
        }

        command = id601_get_field(payload, ID601_READ_RECV_COMMAND_Msk, ID601_READ_RECV_COMMAND_Pos);
        ID = id601_get_field(payload, ID601_READ_RECV_DEVICE_ID_Msk, ID601_READ_RECV_DEVICE_ID_Pos);
        data = id601_get_field(payload, ID601_READ_RECV_DATA_Msk, ID601_READ_RECV_DATA_Pos);

        if (NULL != p_crc_func)
        {
            drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] RX: dev=%02u cmd=0x%02X id=0x%02X data=0x%03X crc=0x%02X", (unsigned)(device_index + 1U), (unsigned)command, (unsigned)ID, (unsigned)data, (unsigned)recv_crc);
        }
        else
        {
            drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] RX: dev=%02u cmd=0x%02X id=0x%02X data=0x%03X crc=Disabled", (unsigned)(device_index + 1U), (unsigned)command, (unsigned)ID, (unsigned)data);
        }
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
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Invalid status pointer.", __func__);
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
    if (false == id601_decode_serial_from_ccr_buffer(p_rise, p_fall, &serial_data, frame_length))
    {
        return false;
    }

    uint32_t payload = serial_data;

    uint16_t command = 0U;
    uint16_t fault = 0U;
    uint16_t recv_crc = 0U;

    if (NULL != p_crc_func)
    {
        recv_crc = (serial_data & ID601_CRC8_Msk) >> ID601_CRC8_Pos;
        payload = (serial_data >> ID601_CRC8_BIT_COUNT);
        uint16_t calc_crc = p_crc_func(payload, ID601_FAULT_RECV_PAYLOAD_BIT_COUNT);
        if (recv_crc != calc_crc)
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: CRC check failed. recv_crc: 0x%02X, calc_crc: 0x%02X", __func__, recv_crc, calc_crc);
            return false;
        }
    }

    command = id601_get_field(payload, ID601_FAULT_RECV_COMMAND_Msk, ID601_FAULT_RECV_COMMAND_Pos);
    fault = id601_get_field(payload, ID601_FAULT_RECV_STATUS_Msk, ID601_FAULT_RECV_STATUS_Pos);

    if (NULL != p_crc_func)
    {
        drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] FAULT RX: cmd=0x%02X fault=0x%03X crc=0x%02X", (unsigned)command, (unsigned)fault, (unsigned)recv_crc);
    }
    else
    {
        drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] FAULT RX: cmd=0x%02X fault=0x%03X crc=Disabled", (unsigned)command, (unsigned)fault);
    }
    *p_status = fault;

    return true;
}

static inline uint32_t id601_build_sync_gen_payload(void)
{
    uint32_t payload = 0U;
    payload = id601_set_field(payload, ID601_SYNCGEN_COMMAND_Msk, ID601_SYNCGEN_COMMAND_Pos, ID601_SYNCGEN_COMMAND);
    return payload;
}

static bool id601_sync_gen_command(id601_crc_func_t p_crc_func)
{
    uint16_t tx_ccr_count = 0U;
    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        uint32_t payload = id601_build_sync_gen_payload();
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

static inline uint32_t id601_build_id_gen_payload(void)
{
    uint32_t payload = 0U;
    payload = id601_set_field(payload, ID601_IDGEN_COMMAND_Msk, ID601_IDGEN_COMMAND_Pos, ID601_IDGEN_COMMAND);
    return payload;
}

static bool id601_id_gen_command(id601_crc_func_t p_crc_func)
{
    uint16_t tx_ccr_count = 0U;
    gn_id601_tx_ccr_buffer[tx_ccr_count++] = 0U; // initialize the buffer before use

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        uint32_t payload = id601_build_id_gen_payload();
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

static bool id601_select_register_bank(const uint16_t bank, id601_crc_func_t p_crc_func)
{
    static uint16_t prev_id601_bank = ID601_REGISTER_BANK_GENERAL;
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };
    uint16_t* p_op_mode = gn_id601_general_regs[ID601_ADDR_GENERAL_OP_MODE];

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_op_mode[device_index];
    }

    if (bank != prev_id601_bank)
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
            tx_data[device_index] = (uint16_t)id601_set_field(p_op_mode[device_index], ID601_OP_MODE_ADDR_EXT_Msk, ID601_OP_MODE_ADDR_EXT_Pos, addr_ext);
        }
        if (false == id601_write_command(ID601_ADDR_GENERAL_OP_MODE, tx_data, p_crc_func))
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to write command.", __func__);
            id601_unlock();
            return false;
        }
        drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] %s: Bank changed to %u. (0x%02X - 0x%03X)", __func__, bank, ID601_ADDR_GENERAL_OP_MODE, tx_data[0]);

        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            p_op_mode[device_index] = tx_data[device_index];
        }
        prev_id601_bank = bank;
    }
    return true;
}

bool id601_write_register(const uint16_t bank, const uint16_t device_select, const uint16_t addr, const uint16_t new_value, id601_crc_func_t p_crc_func)
{
    uint16_t* p_id601 = NULL;
    uint16_t tx_data[ID601_DAISY_LENGTH_MAX] = { 0U };

    if (ID601_REGISTER_BANK_GENERAL == bank)
    {
        if (addr >= ID601_ADDR_GENERAL_MAX_COUNT)
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Invalid general register address.", __func__);
            return false;
        }
        p_id601 = gn_id601_general_regs[addr];
    }
    else if (ID601_REGISTER_BANK_MIRROR == bank)
    {
        if (addr >= ID601_ADDR_MIRROR_MAX_COUNT)
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Invalid mirror register address.", __func__);
            return false;
        }
        p_id601 = gn_id601_mirror_regs[addr];
    }
    else
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Invalid register bank.", __func__);
        return false;
    }

    if (device_select > gn_id601_daisy_length)
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Invalid device select.", __func__);
        return false;
    }

    if (false == id601_lock())
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to acquire ID601 mutex.", __func__);
        return false;
    }

    if (false == id601_select_register_bank(bank, p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to select register bank.", __func__);
        id601_unlock();
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        tx_data[device_index] = p_id601[device_index];
    }

    if (ID601_BROADCAST == device_select)
    {
        for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
        {
            tx_data[device_index] = new_value;
        }
    }
    else
    {
        const uint16_t device_index = device_select - 1U;
        tx_data[device_index] = new_value;
    }

    if (false == id601_write_command(addr, tx_data, p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to write command.", __func__);
        id601_unlock();
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_id601[device_index] = tx_data[device_index];
        drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] WRITE: bank=%u dev=%02u addr=0x%02X value=0x%03X", (unsigned)bank, (unsigned)(device_index + 1U), (unsigned)addr, (unsigned)p_id601[device_index]);
    }

    id601_unlock();

    return true;
}

bool id601_read_register(const uint16_t bank, const uint16_t device_select, const uint16_t addr, id601_crc_func_t p_crc_func, uint16_t* p_single_read_data)
{
    uint16_t* p_id601 = NULL;
    uint16_t rx_data[ID601_DAISY_LENGTH_MAX] = { 0U };

    if (ID601_REGISTER_BANK_GENERAL == bank)
    {
        if (addr >= ID601_ADDR_GENERAL_MAX_COUNT)
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Invalid general register address.", __func__);
            return false;
        }
        p_id601 = gn_id601_general_regs[addr];
    }
    else if (ID601_REGISTER_BANK_MIRROR == bank)
    {
        if (addr >= ID601_ADDR_MIRROR_MAX_COUNT)
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Invalid mirror register address.", __func__);
            return false;
        }
        p_id601 = gn_id601_mirror_regs[addr];
    }
    else
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Invalid register bank.", __func__);
        return false;
    }

    if (device_select > gn_id601_daisy_length)
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Invalid device select.", __func__);
        return false;
    }

    if (false == id601_lock())
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to acquire ID601 mutex.", __func__);
        return false;
    }

    if ((NULL != p_single_read_data) && (ID601_BROADCAST == device_select))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Select one IC to return read data.", __func__);
        id601_unlock();
        return false;
    }

    if (false == id601_select_register_bank(bank, p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to select register bank.", __func__);
        id601_unlock();
        return false;
    }

    vTaskSuspendAll();
    if (false == id601_read_command(addr, p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to read command.", __func__);
        (void)xTaskResumeAll();
        id601_unlock();
        return false;
    }
    if (false == id601_read_receive_command(rx_data, p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to read receive command.", __func__);
        (void)xTaskResumeAll();
        id601_unlock();
        return false;
    }
    (void)xTaskResumeAll();

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        p_id601[device_index] = rx_data[device_index];
        drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] READ:  bank=%u dev=%02u addr=0x%02X value=0x%03X", (unsigned)bank, (unsigned)(device_index + 1U), (unsigned)addr, (unsigned)p_id601[device_index]);
    }

    if (NULL != p_single_read_data)
    {
        *p_single_read_data = rx_data[device_select - 1U];
    }

    id601_unlock();

    return true;
}

bool id601_ld_transfer(uint16_t* p_buff, id601_crc_func_t p_crc_func)
{
    return false;
}

bool id601_fault_read(id601_crc_func_t p_crc_func)
{
    uint16_t rx_data = 0U;
    if (false == id601_lock())
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to acquire ID601 mutex.", __func__);
        return false;
    }

    if (false == id601_fault_read_command(p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to fault read command.", __func__);
        id601_unlock();
        return false;
    }
    if (false == id601_fault_receive_command(&rx_data, p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to fault receive command.", __func__);
        id601_unlock();
        return false;
    }
    gn_id601_fault = rx_data;

    drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] %s: Fault read success.", __func__);
    drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] fault=0x%03X", (unsigned)gn_id601_fault);

    id601_unlock();

    return true;
}

bool id601_sync_gen(id601_crc_func_t p_crc_func)
{
    if (false == id601_lock())
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to acquire ID601 mutex.", __func__);
        return false;
    }

    if (false == id601_sync_gen_command(p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to syncgen command.", __func__);
        id601_unlock();
        return false;
    }

    id601_unlock();

    drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] %s: Sync gen success.", __func__);
    return true;
}

bool id601_id_gen(id601_crc_func_t p_crc_func)
{
    if (false == id601_lock())
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to acquire ID601 mutex.", __func__);
        return false;
    }

    if (false == id601_id_gen_command(p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to ID gen command.", __func__);
        id601_unlock();
        return false;
    }

    id601_unlock();

    drv_uart_printf(LOG_LV_TRACE, "\r\n    [ID601] %s: ID gen success.", __func__);
    return true;
}

static bool id601_reset(id601_crc_func_t p_crc_func)
{
    uint16_t reset[ID601_DAISY_LENGTH_MAX] = { 0U };

    if (false == id601_lock())
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to acquire ID601 mutex.", __func__);
        return false;
    }

    for (uint16_t device_index = 0U; device_index < gn_id601_daisy_length; ++device_index)
    {
        reset[device_index] = id601_set_field(reset[device_index], ID601_RESET_ID_RST_Msk, ID601_RESET_ID_RST_Pos, 1U);
    }

    if (false == id601_write_command(ID601_ADDR_GENERAL_RESET_ID, reset, p_crc_func))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to write command.", __func__);
        id601_unlock();
        return false;
    }

    id601_unlock();
    drv_tim_delay_ms(ID601_DELAY_RESET_MS);
    return true;
}

static inline uint32_t id601_build_reset_id(uint16_t rst, uint16_t vs_rst, uint16_t e_rst, uint16_t lkg_e, uint16_t id)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_RESET_ID_RST_Msk, ID601_RESET_ID_RST_Pos, (uint16_t)rst);
    register_value = id601_set_field(register_value, ID601_RESET_ID_VS_RST_Msk, ID601_RESET_ID_VS_RST_Pos, (uint16_t)vs_rst);
    register_value = id601_set_field(register_value, ID601_RESET_ID_E_RST_Msk, ID601_RESET_ID_E_RST_Pos, (uint16_t)e_rst);
    register_value = id601_set_field(register_value, ID601_RESET_ID_LKG_E_Msk, ID601_RESET_ID_LKG_E_Pos, (uint16_t)lkg_e);
    register_value = id601_set_field(register_value, ID601_RESET_ID_ID_Msk, ID601_RESET_ID_ID_Pos, (uint16_t)id);
    return register_value;
}

bool id601_write_reset_id(uint16_t device_select, uint16_t rst, uint16_t vs_rst, uint16_t e_rst, uint16_t lkg_e, uint16_t id)
{
    uint32_t value = id601_build_reset_id(rst, vs_rst, e_rst, lkg_e, id);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_RESET_ID, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_ld_control(uint16_t sv_no, uint16_t ld_type, uint16_t delay_ch_en, uint16_t syncmode, uint16_t pwm_res, uint16_t ld_dir, uint16_t ld_mode)
{
    uint32_t register_value = 0U;
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
    uint32_t value = id601_build_ld_control(sv_no, ld_type, delay_ch_en, syncmode, pwm_res, ld_dir, ld_mode);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_LD_CONTROL, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_ld_size(uint16_t ld_size)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_LD_SIZE_LD_SIZE_Msk, ID601_LD_SIZE_LD_SIZE_Pos, (uint16_t)ld_size);
    return register_value;
}

bool id601_write_ld_size(uint16_t device_select, uint16_t ld_size)
{
    uint32_t value = id601_build_ld_size(ld_size);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_LD_SIZE, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_pwmclk_div_1_2(uint16_t fpwm_div_2, uint16_t fpwm_div_1)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_PWMCLK_DIV1_2_FPWM_DIV2_Msk, ID601_PWMCLK_DIV1_2_FPWM_DIV2_Pos, (uint16_t)fpwm_div_2);
    register_value = id601_set_field(register_value, ID601_PWMCLK_DIV1_2_FPWM_DIV1_Msk, ID601_PWMCLK_DIV1_2_FPWM_DIV1_Pos, (uint16_t)fpwm_div_1);
    return register_value;
}

bool id601_write_pwmclk_div_1_2(uint16_t device_select, uint16_t fpwm_div_2, uint16_t fpwm_div_1)
{
    uint32_t value = id601_build_pwmclk_div_1_2(fpwm_div_2, fpwm_div_1);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_PWMCLK_DIV1_2, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_pwmclk_div_2_3(uint16_t fpwm_div_3, uint16_t fpwm_div_2)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_PWMCLK_DIV2_3_FPWM_DIV3_Msk, ID601_PWMCLK_DIV2_3_FPWM_DIV3_Pos, (uint16_t)fpwm_div_3);
    register_value = id601_set_field(register_value, ID601_PWMCLK_DIV2_3_FPWM_DIV2_Msk, ID601_PWMCLK_DIV2_3_FPWM_DIV2_Pos, (uint16_t)fpwm_div_2);
    return register_value;
}

bool id601_write_pwmclk_div_2_3(uint16_t device_select, uint16_t fpwm_div_3, uint16_t fpwm_div_2)
{
    uint32_t value = id601_build_pwmclk_div_2_3(fpwm_div_3, fpwm_div_2);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_PWMCLK_DIV2_3, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_channel_enable(uint16_t ch12_en, uint16_t ch11_en, uint16_t ch10_en, uint16_t ch9_en, uint16_t ch8_en, uint16_t ch7_en, uint16_t ch6_en, uint16_t ch5_en, uint16_t ch4_en, uint16_t ch3_en, uint16_t ch2_en, uint16_t ch1_en)
{
    uint32_t register_value = 0U;
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
    uint32_t value = id601_build_channel_enable(ch12_en, ch11_en, ch10_en, ch9_en, ch8_en, ch7_en, ch6_en, ch5_en, ch4_en, ch3_en, ch2_en, ch1_en);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_CHANNEL_ENABLE, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_fault_control(uint16_t ft_mode, uint16_t fb_mode, uint16_t o_fb_e, uint16_t t_det_e, uint16_t s_det_e, uint16_t o_det_e, uint16_t t_off_e, uint16_t s_off_e, uint16_t o_off_e)
{
    uint32_t register_value = 0U;
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
    uint32_t value = id601_build_fault_control(ft_mode, fb_mode, o_fb_e, t_det_e, s_det_e, o_det_e, t_off_e, s_off_e, o_off_e);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_FAULT_CONTROL, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_fb_level(uint16_t fb3_level, uint16_t fb2_level, uint16_t fb1_level)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_FB_LEVEL_FB3_LEVEL_Msk, ID601_FB_LEVEL_FB3_LEVEL_Pos, (uint16_t)fb3_level);
    register_value = id601_set_field(register_value, ID601_FB_LEVEL_FB2_LEVEL_Msk, ID601_FB_LEVEL_FB2_LEVEL_Pos, (uint16_t)fb2_level);
    register_value = id601_set_field(register_value, ID601_FB_LEVEL_FB1_LEVEL_Msk, ID601_FB_LEVEL_FB1_LEVEL_Pos, (uint16_t)fb1_level);
    return register_value;
}

bool id601_write_fb_level(uint16_t device_select, uint16_t fb3_level, uint16_t fb2_level, uint16_t fb1_level)
{
    uint32_t value = id601_build_fb_level(fb3_level, fb2_level, fb1_level);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_FB_LEVEL, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_short_level(uint16_t short3_level, uint16_t short2_level, uint16_t short1_level)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_SHORT_LEVEL_SHORT3_LEVEL_Msk, ID601_SHORT_LEVEL_SHORT3_LEVEL_Pos, (uint16_t)short3_level);
    register_value = id601_set_field(register_value, ID601_SHORT_LEVEL_SHORT2_LEVEL_Msk, ID601_SHORT_LEVEL_SHORT2_LEVEL_Pos, (uint16_t)short2_level);
    register_value = id601_set_field(register_value, ID601_SHORT_LEVEL_SHORT1_LEVEL_Msk, ID601_SHORT_LEVEL_SHORT1_LEVEL_Pos, (uint16_t)short1_level);
    return register_value;
}

bool id601_write_short_level(uint16_t device_select, uint16_t short3_level, uint16_t short2_level, uint16_t short1_level)
{
    uint32_t value = id601_build_short_level(short3_level, short2_level, short1_level);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_SHORT_LEVEL, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_fault_status0(void)
{
    return 0U; // read only
}

bool id601_write_fault_status0(void)
{
    id601_build_fault_status0();
    return false; // read only
}

static inline uint32_t id601_build_fault_status1(void)
{
    return 0U; // read only
}

bool id601_write_fault_status1(void)
{
    id601_build_fault_status1();
    return false; // read only
}

static inline uint32_t id601_build_fault_status2(void)
{
    return 0U; // read only
}

bool id601_write_fault_status2(void)
{
    id601_build_fault_status2();
    return false; // read only
}

static inline uint32_t id601_build_fault_status3(void)
{
    return 0U; // read only
}

bool id601_write_fault_status3(void)
{
    id601_build_fault_status3();
    return false; // read only
}

static inline uint32_t id601_build_max_current_level(uint16_t max_curr_level3, uint16_t max_curr_level2, uint16_t max_curr_level1)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL3_Msk, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL3_Pos, (uint16_t)max_curr_level3);
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL2_Msk, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL2_Pos, (uint16_t)max_curr_level2);
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL1_Msk, ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL1_Pos, (uint16_t)max_curr_level1);
    return register_value;
}

bool id601_write_max_current_level(uint16_t device_select, uint16_t max_curr_level3, uint16_t max_curr_level2, uint16_t max_curr_level1)
{
    uint32_t value = id601_build_max_current_level(max_curr_level3, max_curr_level2, max_curr_level1);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_MAX_CURR_LEVEL, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_max_curr_vref1(uint16_t max_curr_vref1)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_VREF1_MAX_CURR_VREF1_Msk, ID601_MAX_CURRENT_VREF1_MAX_CURR_VREF1_Pos, (uint16_t)max_curr_vref1);
    return register_value;
}

bool id601_write_max_curr_vref1(uint16_t device_select, uint16_t max_curr_vref1)
{
    uint32_t value = id601_build_max_curr_vref1(max_curr_vref1);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_MAX_CURR_VREF1, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_max_curr_vref2(uint16_t max_curr_vref2)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_VREF2_MAX_CURR_VREF2_Msk, ID601_MAX_CURRENT_VREF2_MAX_CURR_VREF2_Pos, (uint16_t)max_curr_vref2);
    return register_value;
}

bool id601_write_max_curr_vref2(uint16_t device_select, uint16_t max_curr_vref2)
{
    uint32_t value = id601_build_max_curr_vref2(max_curr_vref2);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_MAX_CURR_VREF2, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_max_curr_vref3(uint16_t max_curr_vref3)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_MAX_CURRENT_VREF3_MAX_CURR_VREF3_Msk, ID601_MAX_CURRENT_VREF3_MAX_CURR_VREF3_Pos, (uint16_t)max_curr_vref3);
    return register_value;
}

bool id601_write_max_curr_vref3(uint16_t device_select, uint16_t max_curr_vref3)
{
    uint32_t value = id601_build_max_curr_vref3(max_curr_vref3);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_MAX_CURR_VREF3, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_ch_ld_type(uint16_t ch12_ld_type, uint16_t ch11_ld_type, uint16_t ch10_ld_type, uint16_t ch9_ld_type, uint16_t ch8_ld_type, uint16_t ch7_ld_type,
                                                uint16_t ch6_ld_type, uint16_t ch5_ld_type, uint16_t ch4_ld_type, uint16_t ch3_ld_type, uint16_t ch2_ld_type, uint16_t ch1_ld_type)
{
    uint32_t register_value = 0U;
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
    uint32_t value = id601_build_ch_ld_type(ch12_ld_type, ch11_ld_type, ch10_ld_type, ch9_ld_type, ch8_ld_type, ch7_ld_type,
                                                            ch6_ld_type, ch5_ld_type, ch4_ld_type, ch3_ld_type, ch2_ld_type, ch1_ld_type);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_CH_LD_TYPE, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_delay_ch1_2(uint16_t delay_ch2, uint16_t delay_ch1)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH1_2_DELAY_CH2_Msk, ID601_DELAY_CH1_2_DELAY_CH2_Pos, (uint16_t)delay_ch2);
    register_value = id601_set_field(register_value, ID601_DELAY_CH1_2_DELAY_CH1_Msk, ID601_DELAY_CH1_2_DELAY_CH1_Pos, (uint16_t)delay_ch1);
    return register_value;
}

bool id601_write_delay_ch1_2(uint16_t device_select, uint16_t delay_ch2, uint16_t delay_ch1)
{
    uint32_t value = id601_build_delay_ch1_2(delay_ch2, delay_ch1);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_DELAY_CH1_2, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_delay_ch3_4(uint16_t delay_ch4, uint16_t delay_ch3)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH3_4_DELAY_CH4_Msk, ID601_DELAY_CH3_4_DELAY_CH4_Pos, (uint16_t)delay_ch4);
    register_value = id601_set_field(register_value, ID601_DELAY_CH3_4_DELAY_CH3_Msk, ID601_DELAY_CH3_4_DELAY_CH3_Pos, (uint16_t)delay_ch3);
    return register_value;
}

bool id601_write_delay_ch3_4(uint16_t device_select, uint16_t delay_ch4, uint16_t delay_ch3)
{
    uint32_t value = id601_build_delay_ch3_4(delay_ch4, delay_ch3);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_DELAY_CH3_4, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_delay_ch5_6(uint16_t delay_ch6, uint16_t delay_ch5)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH5_6_DELAY_CH6_Msk, ID601_DELAY_CH5_6_DELAY_CH6_Pos, (uint16_t)delay_ch6);
    register_value = id601_set_field(register_value, ID601_DELAY_CH5_6_DELAY_CH5_Msk, ID601_DELAY_CH5_6_DELAY_CH5_Pos, (uint16_t)delay_ch5);
    return register_value;
}

bool id601_write_delay_ch5_6(uint16_t device_select, uint16_t delay_ch6, uint16_t delay_ch5)
{
    uint32_t value = id601_build_delay_ch5_6(delay_ch6, delay_ch5);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_DELAY_CH5_6, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_delay_ch7_8(uint16_t delay_ch8, uint16_t delay_ch7)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH7_8_DELAY_CH8_Msk, ID601_DELAY_CH7_8_DELAY_CH8_Pos, (uint16_t)delay_ch8);
    register_value = id601_set_field(register_value, ID601_DELAY_CH7_8_DELAY_CH7_Msk, ID601_DELAY_CH7_8_DELAY_CH7_Pos, (uint16_t)delay_ch7);
    return register_value;
}

bool id601_write_delay_ch7_8(uint16_t device_select, uint16_t delay_ch8, uint16_t delay_ch7)
{
    uint32_t value = id601_build_delay_ch7_8(delay_ch8, delay_ch7);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_DELAY_CH7_8, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_delay_ch9_10(uint16_t delay_ch10, uint16_t delay_ch9)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH9_10_DELAY_CH10_Msk, ID601_DELAY_CH9_10_DELAY_CH10_Pos, (uint16_t)delay_ch10);
    register_value = id601_set_field(register_value, ID601_DELAY_CH9_10_DELAY_CH9_Msk, ID601_DELAY_CH9_10_DELAY_CH9_Pos, (uint16_t)delay_ch9);
    return register_value;
}

bool id601_write_delay_ch9_10(uint16_t device_select, uint16_t delay_ch10, uint16_t delay_ch9)
{
    uint32_t value = id601_build_delay_ch9_10(delay_ch10, delay_ch9);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_DELAY_CH9_10, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_delay_ch11_12(uint16_t delay_ch12, uint16_t delay_ch11)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_DELAY_CH11_12_DELAY_CH12_Msk, ID601_DELAY_CH11_12_DELAY_CH12_Pos, (uint16_t)delay_ch12);
    register_value = id601_set_field(register_value, ID601_DELAY_CH11_12_DELAY_CH11_Msk, ID601_DELAY_CH11_12_DELAY_CH11_Pos, (uint16_t)delay_ch11);
    return register_value;
}

bool id601_write_delay_ch11_12(uint16_t device_select, uint16_t delay_ch12, uint16_t delay_ch11)
{
    uint32_t value = id601_build_delay_ch11_12(delay_ch12, delay_ch11);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_DELAY_CH11_12, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_fault_configuration(uint16_t crc_en, uint16_t wd_dimm_en, uint16_t wd_timecfg, uint16_t wd_en, uint16_t vs_miss_dimm_en, uint16_t vs_miss_en, uint16_t ch_ctrl_mismatch_en, \
        uint16_t vref_ovuv_off_en, uint16_t vref_ovuv_en, uint16_t osc_ldo_ovuv_en, uint16_t ldo_ovuv_en)
{
    uint32_t register_value = 0U;
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
    uint32_t value = id601_build_fault_configuration(crc_en, wd_dimm_en, wd_timecfg, wd_en, vs_miss_dimm_en, vs_miss_en, ch_ctrl_mismatch_en, vref_ovuv_off_en, vref_ovuv_en, osc_ldo_ovuv_en, ldo_ovuv_en);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_FAULT_CONFIGURATION, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_bist(uint16_t bist_start)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_BIST_BIST_START_Msk, ID601_BIST_BIST_START_Pos, (uint16_t)bist_start);
    return register_value;
}

bool id601_write_bist(uint16_t device_select, uint16_t bist_start)
{
    uint32_t value = id601_build_bist(bist_start);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_BIST, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_test_mode0(uint16_t dmux_sel)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TEST_MODE0_DMUX_SEL_Msk, ID601_TEST_MODE0_DMUX_SEL_Pos, (uint16_t)dmux_sel);
    return register_value;
}

bool id601_write_test_mode0(uint16_t device_select, uint16_t dmux_sel)
{
    uint32_t value = id601_build_test_mode0(dmux_sel);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_TEST_MODE0, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_test_mode1(uint16_t testmode, uint16_t tm_ch_ctrl_input_flip)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TEST_MODE1_TESTMODE_Msk, ID601_TEST_MODE1_TESTMODE_Pos, (uint16_t)testmode);
    register_value = id601_set_field(register_value, ID601_TEST_MODE1_TM_CH_CTRL_INPUT_FLIP_Msk, ID601_TEST_MODE1_TM_CH_CTRL_INPUT_FLIP_Pos, (uint16_t)tm_ch_ctrl_input_flip);
    return register_value;
}

bool id601_write_test_mode1(uint16_t device_select, uint16_t testmode, uint16_t tm_ch_ctrl_input_flip)
{
    uint32_t value = id601_build_test_mode1(testmode, tm_ch_ctrl_input_flip);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_TEST_MODE1, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_serial_clk_gen(uint16_t serial_clk_low, uint16_t serial_clk_high)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_SERIAL_CLK_GEN_SERIAL_CLK_LOW_Msk, ID601_SERIAL_CLK_GEN_SERIAL_CLK_LOW_Pos, (uint16_t)serial_clk_low);
    register_value = id601_set_field(register_value, ID601_SERIAL_CLK_GEN_SERIAL_CLK_HIGH_Msk, ID601_SERIAL_CLK_GEN_SERIAL_CLK_HIGH_Pos, (uint16_t)serial_clk_high);
    return register_value;
}

bool id601_write_serial_clk_gen(uint16_t device_select, uint16_t serial_clk_low, uint16_t serial_clk_high)
{
    uint32_t value = id601_build_serial_clk_gen(serial_clk_low, serial_clk_high);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_SERIAL_CLK_GEN, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_serial_latency(uint16_t serial_latency)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_SERIAL_LATENCY_SERIAL_LATENCY_Msk, ID601_SERIAL_LATENCY_SERIAL_LATENCY_Pos, (uint16_t)serial_latency);
    return register_value;
}

bool id601_write_serial_latency(uint16_t device_select, uint16_t serial_latency)
{
    uint32_t value = id601_build_serial_latency(serial_latency);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_SERIAL_LATENCY, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_v_mask(uint16_t v_mask)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_V_MASK_V_MASK_Msk, ID601_V_MASK_V_MASK_Pos, (uint16_t)v_mask);
    return register_value;
}

bool id601_write_v_mask(uint16_t device_select, uint16_t v_mask)
{
    uint32_t value = id601_build_v_mask(v_mask);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_V_MASK, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_sv_mask(uint16_t sv_mask_en, uint16_t sv_mask)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_SV_MASK_SV_MASK_EN_Msk, ID601_SV_MASK_SV_MASK_EN_Pos, (uint16_t)sv_mask_en);
    register_value = id601_set_field(register_value, ID601_SV_MASK_SV_MASK_Msk, ID601_SV_MASK_SV_MASK_Pos, (uint16_t)sv_mask);
    return register_value;
}

bool id601_write_sv_mask(uint16_t device_select, uint16_t sv_mask_en, uint16_t sv_mask)
{
    uint32_t value = id601_build_sv_mask(sv_mask_en, sv_mask);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_SV_MASK, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_rstcnt(uint16_t rstcnt)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_RSTCNT_RSTCNT_Msk, ID601_RSTCNT_RSTCNT_Pos, (uint16_t)rstcnt);
    return register_value;
}

bool id601_write_rstcnt(uint16_t device_select, uint16_t rstcnt)
{
    uint32_t value = id601_build_rstcnt(rstcnt);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_RSTCNT, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_timeout(uint16_t timeout)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TIMEOUT_TIMEOUT_Msk, ID601_TIMEOUT_TIMEOUT_Pos, (uint16_t)timeout);
    return register_value;
}

bool id601_write_timeout(uint16_t device_select, uint16_t timeout)
{
    uint32_t value = id601_build_timeout(timeout);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_TIMEOUT, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_fllcnt1(uint16_t fllcnt)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_FLLCNT1_FLLCNT_Msk, ID601_FLLCNT1_FLLCNT_Pos, (uint16_t)fllcnt);
    return register_value;
}

bool id601_write_fllcnt1(uint16_t device_select, uint16_t fllcnt)
{
    uint32_t value = id601_build_fllcnt1(fllcnt);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_FLLCNT1, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_fllcnt2(uint16_t fll_en, uint16_t fll_range, uint16_t fllcnt)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_FLLCNT2_FLL_EN_Msk, ID601_FLLCNT2_FLL_EN_Pos, (uint16_t)fll_en);
    register_value = id601_set_field(register_value, ID601_FLLCNT2_FLL_RANGE_Msk, ID601_FLLCNT2_FLL_RANGE_Pos, (uint16_t)fll_range);
    register_value = id601_set_field(register_value, ID601_FLLCNT2_FLLCNT_Msk, ID601_FLLCNT2_FLLCNT_Pos, (uint16_t)fllcnt);
    return register_value;
}

bool id601_write_fllcnt2(uint16_t device_select, uint16_t fll_en, uint16_t fll_range, uint16_t fllcnt)
{
    uint32_t value = id601_build_fllcnt2(fll_en, fll_range, fllcnt);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_FLLCNT2, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_wr_protect(uint16_t wr_protect)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_WR_PROTECT_WR_PROTECT_Msk, ID601_WR_PROTECT_WR_PROTECT_Pos, (uint16_t)wr_protect);
    return register_value;
}

bool id601_write_wr_protect(uint16_t device_select, uint16_t wr_protect)
{
    uint32_t value = id601_build_wr_protect(wr_protect);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_WR_PROTECT, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_nf_control(uint16_t bbkn_th, uint16_t o_emi_rej_en, uint16_t sgrjt_en2, uint16_t sgrjt_en1, uint16_t bbkn_en, uint16_t dgrjt_en2, uint16_t dgrjt_en1)
{
    uint32_t register_value = 0U;
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
    uint32_t value = id601_build_nf_control(0U, o_emi_rej_en, sgrjt_en2, sgrjt_en1, bbkn_en, dgrjt_en2, dgrjt_en1);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_NF_CONTROL, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_chop_en(uint16_t open_mask_opt, uint16_t chop_en, uint16_t chop_drv_en, uint16_t chop_osc_ldo_en, uint16_t chop_osc_en, uint16_t chop_dac_en, uint16_t chop_bgr_en)
{
    uint32_t register_value = 0U;
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
    uint32_t value = id601_build_chop_en(open_mask_opt, chop_en, chop_drv_en, chop_osc_ldo_en, chop_osc_en, chop_dac_en, chop_bgr_en);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_CHOP_EN, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_temp(uint16_t ov_swap_en, uint16_t dac_rng, uint16_t flt_ctl, uint16_t o_slew, uint16_t flt_gain)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TEMP_OV_SWAP_EN_Msk, ID601_TEMP_OV_SWAP_EN_Pos, (uint16_t)ov_swap_en);
    register_value = id601_set_field(register_value, ID601_TEMP_DAC_RNG_Msk, ID601_TEMP_DAC_RNG_Pos, (uint16_t)dac_rng);
    register_value = id601_set_field(register_value, ID601_TEMP_FLT_CTL_Msk, ID601_TEMP_FLT_CTL_Pos, (uint16_t)flt_ctl);
    register_value = id601_set_field(register_value, ID601_TEMP_O_SLEW_Msk, ID601_TEMP_O_SLEW_Pos, (uint16_t)o_slew);
    register_value = id601_set_field(register_value, ID601_TEMP_FLT_GAIN_Msk, ID601_TEMP_FLT_GAIN_Pos, (uint16_t)flt_gain);
    return register_value;
}

bool id601_write_temp(uint16_t device_select, uint16_t ov_swap_en, uint16_t dac_rng, uint16_t flt_ctl, uint16_t o_slew, uint16_t flt_gain)
{
    uint32_t value = id601_build_temp(ov_swap_en, dac_rng, flt_ctl, o_slew, flt_gain);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_TEMP, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_osc_fll_man1(uint16_t osc_fll_man)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OSC_FLL_MAN1_OSC_FLL_MAN_Msk, ID601_OSC_FLL_MAN1_OSC_FLL_MAN_Pos, (uint16_t)osc_fll_man);
    return register_value;
}

bool id601_write_osc_fll_man1(uint16_t device_select, uint16_t osc_fll_man)
{
    uint32_t value = id601_build_osc_fll_man1(osc_fll_man);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OSC_FLL_MAN1, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_osc_fll_man2(uint16_t osc_man_en, uint16_t osc_fll_err_range, uint16_t osc_fll_man)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OSC_FLL_MAN2_OSC_MAN_EN_Msk, ID601_OSC_FLL_MAN2_OSC_MAN_EN_Pos, (uint16_t)osc_man_en);
    register_value = id601_set_field(register_value, ID601_OSC_FLL_MAN2_OSC_FLL_ERR_RANGE_Msk, ID601_OSC_FLL_MAN2_OSC_FLL_ERR_RANGE_Pos, (uint16_t)osc_fll_err_range);
    register_value = id601_set_field(register_value, ID601_OSC_FLL_MAN2_OSC_FLL_MAN_Msk, ID601_OSC_FLL_MAN2_OSC_FLL_MAN_Pos, (uint16_t)osc_fll_man);
    return register_value;
}

bool id601_write_osc_fll_man2(uint16_t device_select, uint16_t osc_man_en, uint16_t osc_fll_err_range, uint16_t osc_fll_man)
{
    uint32_t value = id601_build_osc_fll_man2(osc_man_en, osc_fll_err_range, osc_fll_man);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OSC_FLL_MAN2, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_osc_spread(uint16_t sprd_en, uint16_t sprd_spd, uint16_t sprd_gain)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OSC_SPREAD_SPRD_EN_Msk, ID601_OSC_SPREAD_SPRD_EN_Pos, (uint16_t)sprd_en);
    register_value = id601_set_field(register_value, ID601_OSC_SPREAD_SPRD_SPD_Msk, ID601_OSC_SPREAD_SPRD_SPD_Pos, (uint16_t)sprd_spd);
    register_value = id601_set_field(register_value, ID601_OSC_SPREAD_SPRD_GAIN_Msk, ID601_OSC_SPREAD_SPRD_GAIN_Pos, (uint16_t)sprd_gain);
    return register_value;
}

bool id601_write_osc_spread(uint16_t device_select, uint16_t sprd_en, uint16_t sprd_spd, uint16_t sprd_gain)
{
    uint32_t value = id601_build_osc_spread(sprd_en, sprd_spd, sprd_gain);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OSC_SPREAD, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_clock_gate_en(uint16_t otp_mclk_en, uint16_t fr2_mclk_en, uint16_t fr1_mclk_en, uint16_t dc_mclk_en)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_CLOCK_GATE_EN_OTP_MCLK_EN_Msk, ID601_CLOCK_GATE_EN_OTP_MCLK_EN_Pos, (uint16_t)otp_mclk_en);
    register_value = id601_set_field(register_value, ID601_CLOCK_GATE_EN_FR2_MCLK_EN_Msk, ID601_CLOCK_GATE_EN_FR2_MCLK_EN_Pos, (uint16_t)fr2_mclk_en);
    register_value = id601_set_field(register_value, ID601_CLOCK_GATE_EN_FR1_MCLK_EN_Msk, ID601_CLOCK_GATE_EN_FR1_MCLK_EN_Pos, (uint16_t)fr1_mclk_en);
    register_value = id601_set_field(register_value, ID601_CLOCK_GATE_EN_DC_MCLK_EN_Msk, ID601_CLOCK_GATE_EN_DC_MCLK_EN_Pos, (uint16_t)dc_mclk_en);
    return register_value;
}

bool id601_write_clock_gate_en(uint16_t device_select, uint16_t otp_mclk_en, uint16_t fr2_mclk_en, uint16_t fr1_mclk_en, uint16_t dc_mclk_en)
{
    uint32_t value = id601_build_clock_gate_en(otp_mclk_en, fr2_mclk_en, fr1_mclk_en, dc_mclk_en);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_CLOCK_GATE_EN, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_vref_fix1(uint16_t vref_fix1)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_VREF_FIX1_VREF_FIX1_Msk, ID601_VREF_FIX1_VREF_FIX1_Pos, (uint16_t)vref_fix1);
    return register_value;
}

bool id601_write_vref_fix1(uint16_t device_select, uint16_t vref_fix1)
{
    uint32_t value = id601_build_vref_fix1(vref_fix1);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_VREF_FIX1, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_vref_fix2(uint16_t vref_fix2)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_VREF_FIX2_VREF_FIX2_Msk, ID601_VREF_FIX2_VREF_FIX2_Pos, (uint16_t)vref_fix2);
    return register_value;
}

bool id601_write_vref_fix2(uint16_t device_select, uint16_t vref_fix2)
{
    uint32_t value = id601_build_vref_fix2(vref_fix2);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_VREF_FIX2, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_vref_fix3(uint16_t vref_fix3)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_VREF_FIX3_VREF_FIX3_Msk, ID601_VREF_FIX3_VREF_FIX3_Pos, (uint16_t)vref_fix3);
    return register_value;
}

bool id601_write_vref_fix3(uint16_t device_select, uint16_t vref_fix3)
{
    uint32_t value = id601_build_vref_fix3(vref_fix3);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_VREF_FIX3, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_test_ana_en(uint16_t test_ana_en)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_TEST_ANA_EN_TEST_ANA_EN_Msk, ID601_TEST_ANA_EN_TEST_ANA_EN_Pos, (uint16_t)test_ana_en);
    return register_value;
}

bool id601_write_test_ana_en(uint16_t device_select, uint16_t test_ana_en)
{
    uint32_t value = id601_build_test_ana_en(test_ana_en);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_TEST_ANA_EN, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_otp_access1(uint16_t otp_pg_acc_cycle)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_ACCESS1_OTP_PG_ACC_CYCLE_Msk, ID601_OTP_ACCESS1_OTP_PG_ACC_CYCLE_Pos, (uint16_t)otp_pg_acc_cycle);
    return register_value;
}

bool id601_write_otp_access1(uint16_t device_select, uint16_t otp_pg_acc_cycle)
{
    uint32_t value = id601_build_otp_access1(otp_pg_acc_cycle);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OTP_ACCESS1, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_otp_access2(uint16_t otp_pg_acc_cycle)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_ACCESS2_OTP_PG_ACC_CYCLE_Msk, ID601_OTP_ACCESS2_OTP_PG_ACC_CYCLE_Pos, (uint16_t)otp_pg_acc_cycle);
    return register_value;
}

bool id601_write_otp_access2(uint16_t device_select, uint16_t otp_pg_acc_cycle)
{
    uint32_t value = id601_build_otp_access2(otp_pg_acc_cycle);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OTP_ACCESS2, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_otp_write(uint16_t otp_rd, uint16_t otp_wsel)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_WRITE_OTP_RD_Msk, ID601_OTP_WRITE_OTP_RD_Pos, (uint16_t)otp_rd);
    register_value = id601_set_field(register_value, ID601_OTP_WRITE_OTP_WSEL_Msk, ID601_OTP_WRITE_OTP_WSEL_Pos, (uint16_t)otp_wsel);
    return register_value;
}

bool id601_write_otp_write(uint16_t device_select, uint16_t otp_rd, uint16_t otp_wsel)
{
    uint32_t value = id601_build_otp_write(otp_rd, otp_wsel);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OTP_WRITE, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_otp_rd_prog(uint16_t otp_pg_done, uint16_t otp_rd_s, uint16_t otp_pg_s)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_RD_PROG_OTP_PG_DONE_Msk, ID601_OTP_RD_PROG_OTP_PG_DONE_Pos, (uint16_t)otp_pg_done);
    register_value = id601_set_field(register_value, ID601_OTP_RD_PROG_OTP_RD_S_Msk, ID601_OTP_RD_PROG_OTP_RD_S_Pos, (uint16_t)otp_rd_s);
    register_value = id601_set_field(register_value, ID601_OTP_RD_PROG_OTP_PG_S_Msk, ID601_OTP_RD_PROG_OTP_PG_S_Pos, (uint16_t)otp_pg_s);
    return register_value;
}

bool id601_write_otp_rd_prog(uint16_t device_select, uint16_t otp_pg_done, uint16_t otp_rd_s, uint16_t otp_pg_s)
{
    uint32_t value = id601_build_otp_rd_prog(otp_pg_done, otp_rd_s, otp_pg_s);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OTP_RD_PROG, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_otp_protect(uint16_t protect_en)
{
    uint32_t register_value = 0U;
    register_value = id601_set_field(register_value, ID601_OTP_PROTECT_PROTECT_EN_Msk, ID601_OTP_PROTECT_PROTECT_EN_Pos, (uint16_t)protect_en);
    return register_value;
}

bool id601_write_otp_protect(uint16_t device_select, uint16_t protect_en)
{
    uint32_t value = id601_build_otp_protect(protect_en);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OTP_PROTECT, (uint16_t)value, gp_id601_crc_func);
}

static inline uint32_t id601_build_op_mode(uint16_t test_en, uint16_t sw_sel, uint16_t pwmout_full, uint16_t mclk64_o, uint16_t dtest_mux_en, uint16_t rd_en, uint16_t ext_clkin, uint16_t addr_ext)
{
    uint32_t register_value = 0U;
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
    uint32_t value = id601_build_op_mode(test_en, sw_sel, pwmout_full, mclk64_o, dtest_mux_en, rd_en, ext_clkin, addr_ext);
    return id601_write_register(ID601_REGISTER_BANK_GENERAL, device_select, ID601_ADDR_GENERAL_OP_MODE, (uint16_t)value, gp_id601_crc_func);
}

static bool id601_read_all_registers(void)
{
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_RESET_ID ; id601_addr < ID601_ADDR_GENERAL_MAX_COUNT ; ++id601_addr)
    {
        if (false == id601_read_register(ID601_REGISTER_BANK_GENERAL, ID601_BROADCAST, id601_addr, gp_id601_crc_func, NULL))
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to read general register 0x%03X", __func__, id601_addr);
            return false;
        }
    }

    for (uint16_t id601_addr = ID601_ADDR_MIRROR1 ; id601_addr < ID601_ADDR_MIRROR_MAX_COUNT ; ++id601_addr)
    {
        if (false == id601_read_register(ID601_REGISTER_BANK_MIRROR, ID601_BROADCAST, id601_addr, gp_id601_crc_func, NULL))
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to read mirror register 0x%03X", __func__, id601_addr);
            return false;
        }
    }
    return true;
}

bool id601_dump_registers(void)
{
    char buffer[MSG_BUFFER_SIZE];
    int length;

    id601_read_all_registers();

    length = snprintf(buffer, sizeof(buffer), "\r\n    [ID601] REGISTER DUMP\r\n    %-8s %4s %6s %6s", "BANK", "DEV", "ADDR", "VALUE");
    if ((length > 0) && ((size_t)length < sizeof(buffer)))
    {
        drv_uart_printf_direct((const uint8_t *)buffer, (uint16_t)length);
    }

    for (uint16_t addr = ID601_ADDR_GENERAL_RESET_ID; addr < ID601_ADDR_GENERAL_MAX_COUNT; ++addr)
    {
        for (uint16_t dev = 0U; dev < gn_id601_daisy_length; ++dev)
        {
            length = snprintf(buffer, sizeof(buffer), "\r\n    %-8s   %02u   0x%02X  0x%03X", "GENERAL", (unsigned)(dev + 1U), (unsigned)addr, (unsigned)gn_id601_general_regs[addr][dev]);

            if ((length > 0) && ((size_t)length < sizeof(buffer)))
            {
                drv_uart_printf_direct((const uint8_t *)buffer, (uint16_t)length);
            }
        }
    }

    for (uint16_t addr = ID601_ADDR_MIRROR1; addr < ID601_ADDR_MIRROR_MAX_COUNT; ++addr)
    {
        for (uint16_t dev = 0U; dev < gn_id601_daisy_length; ++dev)
        {
            length = snprintf(buffer, sizeof(buffer), "\r\n    %-8s   %02u   0x%02X  0x%03X", "MIRROR", (unsigned)(dev + 1U), (unsigned)addr, (unsigned)gn_id601_mirror_regs[addr][dev]);

            if ((length > 0) && ((size_t)length < sizeof(buffer)))
            {
                drv_uart_printf_direct((const uint8_t *)buffer, (uint16_t)length);
            }
        }
    }
    return true;
}

bool id601_compare_initial_code(void)
{
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_LD_CONTROL ; id601_addr < ID601_ADDR_GENERAL_MAX_COUNT; ++id601_addr) // skip reset_id, op_mode
    {
        if (gn_id601_general_regs_initial_code[id601_addr] != gn_id601_general_regs[id601_addr][0])
        {
            if (ID601_ADDR_GENERAL_TEST_MODE1 == id601_addr || ID601_ADDR_GENERAL_WR_PROTECT == id601_addr || ID601_ADDR_GENERAL_OP_MODE == id601_addr)
            {
                continue; // Skip comparison for test mode, write protect, and op mode registers
            }
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to compare general register 0x%02X with initial code (expected: 0x%03X, actual: 0x%03X)", __func__, id601_addr, gn_id601_general_regs_initial_code[id601_addr], gn_id601_general_regs[id601_addr][0]);
            return false;
        }
    }
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] General registers match initial values");
    for (uint16_t id601_addr = ID601_ADDR_MIRROR1 ; id601_addr < ID601_ADDR_MIRROR_MAX_COUNT ; ++id601_addr)
    {
        if (gn_id601_mirror_regs_initial_code[id601_addr] != gn_id601_mirror_regs[id601_addr][0])
        {
            drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to compare mirror register 0x%01X with initial code (expected: 0x%03X, actual: 0x%03X)", __func__, id601_addr, gn_id601_mirror_regs_initial_code[id601_addr], gn_id601_mirror_regs[id601_addr][0]);
            return false;
        }
    }
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Mirror registers match initial values");

    return true;
}

static bool id601_enter_test_mode(void)
{
    const uint16_t enter_test_mode_phase[2] = { 0xA5AU, 0x5A5U };
    if (false == id601_write_wr_protect(1U, enter_test_mode_phase[0]))
    {
        return false;
    }
    if (false == id601_write_wr_protect(1U, enter_test_mode_phase[1]))
    {
        return false;
    }
    return true;
}

bool id601_init_tm_entry_mode(void)
{
    drv_id601_set_daisy_length(1U);

    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_reset(gp_id601_crc_func);
    id601_id_gen(gp_id601_crc_func);

    if (false == id601_enter_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Entering test mode failed. ID601 Power Off.", __func__);
        drv_gpio_clear_id601();
        return false;
    }

    if (false == id601_read_all_registers())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Initial register read failed. ID601 Power Off.", __func__);
        drv_gpio_clear_id601();
        return false;
    }
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] TM Entry Init OK");
    return true;
}
bool id601_init_dimming_mode(void)
{
    id601_reset(gp_id601_crc_func);
    id601_id_gen(gp_id601_crc_func);

    if (false == id601_read_all_registers())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Initial register read failed. ID601 Power Off.", __func__);
        drv_gpio_clear_id601();
        return false;
    }
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Dimming Init OK");
    return true;
}

bool id601_init_after_efuse_mode(void)
{
    drv_id601_set_daisy_length(1U);

    USER_ASSERT(1U == gn_id601_daisy_length);

    id601_reset(gp_id601_crc_func);
    id601_id_gen(gp_id601_crc_func);

    if (false == id601_enter_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Entering test mode failed. ID601 Power Off.", __func__);
        drv_gpio_clear_id601();
        return false;
    }

    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] After EFUSE Init OK");
    return true;
}

bool id601_set_analog_test_mode(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_RESET_ID ; id601_addr < ID601_ADDR_GENERAL_MAX_COUNT ; ++id601_addr)
    {
        switch (id601_addr)
        {
            case ID601_ADDR_GENERAL_FAULT_CONTROL:
            {
                const uint16_t _r06_FT_MODE = 0U;
                const uint16_t _r06_FB_MODE = 1U;
                const uint16_t _r06_O_FB_E  = 1U;
                const uint16_t _r06_T_DET_E = 1U;
                const uint16_t _r06_S_DET_E = 1U;
                const uint16_t _r06_O_DET_E = 1U;
                const uint16_t _r06_T_OFF_E = 1U;
                const uint16_t _r06_S_OFF_E = 1U;
                const uint16_t _r06_O_OFF_E = 1U;
                if (false == id601_write_fault_control(1U, \
                    _r06_FT_MODE, _r06_FB_MODE, _r06_O_FB_E, _r06_T_DET_E, _r06_S_DET_E, _r06_O_DET_E, _r06_T_OFF_E, _r06_S_OFF_E, _r06_O_OFF_E))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_FLLCNT1:
            {
                const uint16_t _r22_FLL_CNT_1 = 0x0DCDU;
                if (false == id601_write_fllcnt1(1U, _r22_FLL_CNT_1))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_CHOP_EN:
            {
                const uint16_t _r26_OPEN_MASK_OPT = 0U;
                const uint16_t _r26_CHOP_EN = 1U;
                const uint16_t _r26_CHOP_DRV_EN = 1U;
                const uint16_t _r26_CHOP_OSC_LDO_EN = 1U;
                const uint16_t _r26_CHOP_OSC_EN = 1U;
                const uint16_t _r26_CHOP_DAC_EN = 1U;
                const uint16_t _r26_CHOP_BGR_EN = 1U;
                if (false == id601_write_chop_en(1U, \
                    _r26_OPEN_MASK_OPT, _r26_CHOP_EN, _r26_CHOP_DRV_EN, _r26_CHOP_OSC_LDO_EN, _r26_CHOP_OSC_EN, _r26_CHOP_DAC_EN, _r26_CHOP_BGR_EN))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_OTP_PROTECT:
            {
                const uint16_t _r3E_OTP_PROTECT = 0xA5AU;
                if (false == id601_write_otp_protect(1U, _r3E_OTP_PROTECT))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_OP_MODE:
            {
                const uint16_t _r3F_TEST_EN = 1U;
                const uint16_t _r3F_SW_SEL = 1U;
                const uint16_t _r3F_PWMOUT_FULL = 1U;
                const uint16_t _r3F_MCLK64_O = 0U;
                const uint16_t _r3F_DTEST_MUX_EN = 0U;
                const uint16_t _r3F_RD_EN = 1U;
                const uint16_t _r3F_EXT_CLKIN = 0U;
                const uint16_t _r3F_ADDR_EXT = 0U;
                if (false == id601_write_op_mode(1U, \
                    _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
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
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Analog Test Mode OK");
    return true;
}

bool id601_set_fll_test_mode(void)
{
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_RESET_ID ; id601_addr < ID601_ADDR_GENERAL_MAX_COUNT ; ++id601_addr)
    {
        switch (id601_addr)
        {
            case ID601_ADDR_GENERAL_CHOP_EN:
            {
                const uint16_t _r26_OPEN_MASK_OPT = 0U;
                const uint16_t _r26_CHOP_EN = 1U;
                const uint16_t _r26_CHOP_DRV_EN = 1U;
                const uint16_t _r26_CHOP_OSC_LDO_EN = 1U;
                const uint16_t _r26_CHOP_OSC_EN = 1U;
                const uint16_t _r26_CHOP_DAC_EN = 1U;
                const uint16_t _r26_CHOP_BGR_EN = 1U;
                if (false == id601_write_chop_en(1U, \
                    _r26_OPEN_MASK_OPT, _r26_CHOP_EN, _r26_CHOP_DRV_EN, _r26_CHOP_OSC_LDO_EN, _r26_CHOP_OSC_EN, _r26_CHOP_DAC_EN, _r26_CHOP_BGR_EN))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_OSC_FLL_MAN2:
            {
                const uint16_t _r29_OSC_MAN_EN = 0U;
                const uint16_t _r29_OSC_FLL_ERR_RANGE = 0U;
                const uint16_t _r29_OSC_FLL_MAN = 8U;
                if (false == id601_write_osc_fll_man2(1U, _r29_OSC_MAN_EN, _r29_OSC_FLL_ERR_RANGE, _r29_OSC_FLL_MAN))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_OTP_PROTECT:
            {
                const uint16_t _r3E_OTP_PROTECT = 0xA5AU;
                if (false == id601_write_otp_protect(1U, _r3E_OTP_PROTECT))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_OP_MODE:
            {
                const uint16_t _r3F_TEST_EN = 1U;
                const uint16_t _r3F_SW_SEL = 1U;
                const uint16_t _r3F_PWMOUT_FULL = 1U;
                const uint16_t _r3F_MCLK64_O = 1U;
                const uint16_t _r3F_DTEST_MUX_EN = 0U;
                const uint16_t _r3F_RD_EN = 1U;
                const uint16_t _r3F_EXT_CLKIN = 0U;
                const uint16_t _r3F_ADDR_EXT = 0U;
                if (false == id601_write_op_mode(1U, \
                    _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT))
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
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] FLL Test Mode OK");
    return true;
}

bool id601_set_dimming_type1_mode(void)
{
    const uint16_t daisy = gn_id601_daisy_length;
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_RESET_ID ; id601_addr < ID601_ADDR_GENERAL_MAX_COUNT ; ++id601_addr)
    {
        switch (id601_addr)
        {
            case ID601_ADDR_GENERAL_LD_CONTROL:
            {
                const uint16_t _r01_SV_NO = 32U;
                const uint16_t _r01_LD_TYPE = 1U;
                const uint16_t _r01_DELAY_CH_EN = 0U;
                const uint16_t _r01_SYNCMODE = 0U;
                const uint16_t _r01_LD_RES = 1U;
                const uint16_t _r01_LD_DIR = 1U;
                const uint16_t _r01_LD_MODE = 0U;
                if (false == id601_write_ld_control(daisy, \
                    _r01_SV_NO, _r01_LD_TYPE, _r01_DELAY_CH_EN, _r01_SYNCMODE, _r01_LD_RES, _r01_LD_DIR, _r01_LD_MODE))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_LD_SIZE:
            {
                const uint16_t _r02_LD_SIZE = 36U;
                if (false == id601_write_ld_size(daisy, _r02_LD_SIZE))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_PWMCLK_DIV1_2:
            {
                const uint16_t _r03_PWMCLK_DIV2 = 14U;
                const uint16_t _r03_PWMCLK_DIV1 = 35U;
                if (false == id601_write_pwmclk_div_1_2(daisy, _r03_PWMCLK_DIV2, _r03_PWMCLK_DIV1))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_PWMCLK_DIV2_3:
            {
                const uint16_t _r04_PWMCLK_DIV3 = 9U;
                const uint16_t _r04_PWMCLK_DIV2 = 0U;
                if (false == id601_write_pwmclk_div_2_3(daisy, _r04_PWMCLK_DIV3, _r04_PWMCLK_DIV2))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_CHANNEL_ENABLE:
            {
                const uint16_t _r05_CH12_EN = 1U;
                const uint16_t _r05_CH11_EN = 1U;
                const uint16_t _r05_CH10_EN = 1U;
                const uint16_t _r05_CH09_EN = 1U;
                const uint16_t _r05_CH08_EN = 1U;
                const uint16_t _r05_CH07_EN = 1U;
                const uint16_t _r05_CH06_EN = 1U;
                const uint16_t _r05_CH05_EN = 1U;
                const uint16_t _r05_CH04_EN = 1U;
                const uint16_t _r05_CH03_EN = 1U;
                const uint16_t _r05_CH02_EN = 1U;
                const uint16_t _r05_CH01_EN = 1U;
                if (false == id601_write_channel_enable(daisy, _r05_CH12_EN, _r05_CH11_EN, _r05_CH10_EN, _r05_CH09_EN, _r05_CH08_EN, _r05_CH07_EN, _r05_CH06_EN,\
                        _r05_CH05_EN, _r05_CH04_EN, _r05_CH03_EN, _r05_CH02_EN, _r05_CH01_EN))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_FAULT_CONTROL:
            {
                const uint16_t _r06_FT_MODE = 3U;
                const uint16_t _r06_FB_MODE = 2U;
                const uint16_t _r06_O_FB_E = 0U;
                const uint16_t _r06_T_DET_E = 1U;
                const uint16_t _r06_S_DET_E = 1U;
                const uint16_t _r06_O_DET_E = 1U;
                const uint16_t _r06_T_OFF_E = 0U;
                const uint16_t _r06_S_OFF_E = 0U;
                const uint16_t _r06_O_OFF_E = 0U;
                if (false == id601_write_fault_control(daisy, _r06_FT_MODE, _r06_FB_MODE, _r06_O_FB_E, _r06_T_DET_E, _r06_S_DET_E, _r06_O_DET_E, _r06_T_OFF_E, _r06_S_OFF_E, _r06_O_OFF_E))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_FB_LEVEL:
            {
                const uint16_t _r07_FB3_LEVEL = 0U;
                const uint16_t _r07_FB2_LEVEL = 0U;
                const uint16_t _r07_FB1_LEVEL = 0U;
                if (false == id601_write_fb_level(daisy, _r07_FB3_LEVEL, _r07_FB2_LEVEL, _r07_FB1_LEVEL))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_SHORT_LEVEL:
            {
                const uint16_t _r08_SHORT3_LEVEL = 7U;
                const uint16_t _r08_SHORT2_LEVEL = 7U;
                const uint16_t _r08_SHORT1_LEVEL = 7U;
                if (false == id601_write_short_level(daisy, _r08_SHORT3_LEVEL, _r08_SHORT2_LEVEL, _r08_SHORT1_LEVEL))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_MAX_CURR_LEVEL:
            {
                const uint16_t _r0D_MAX_CURR_LEVEL3 = 5U;
                const uint16_t _r0D_MAX_CURR_LEVEL2 = 5U;
                const uint16_t _r0D_MAX_CURR_LEVEL1 = 5U;
                if (false == id601_write_max_current_level(daisy, _r0D_MAX_CURR_LEVEL3, _r0D_MAX_CURR_LEVEL2, _r0D_MAX_CURR_LEVEL1))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_MAX_CURR_VREF1:
            {
                const uint16_t _r0E_MAX_CURR_VREF1 = 0xFFFU;
                if (false == id601_write_max_curr_vref1(daisy, _r0E_MAX_CURR_VREF1))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_MAX_CURR_VREF2:
            {
                const uint16_t _r0F_MAX_CURR_VREF2 = 0xFFFU;
                if (false == id601_write_max_curr_vref2(daisy, _r0F_MAX_CURR_VREF2))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_MAX_CURR_VREF3:
            {
                const uint16_t _r10_MAX_CURR_VREF3 = 0xFFFU;
                if (false == id601_write_max_curr_vref3(daisy, _r10_MAX_CURR_VREF3))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_CH_LD_TYPE:
            {
                const uint16_t _r11_CH12_LD_TYPE = 0U;
                const uint16_t _r11_CH11_LD_TYPE = 0U;
                const uint16_t _r11_CH10_LD_TYPE = 0U;
                const uint16_t _r11_CH09_LD_TYPE = 0U;
                const uint16_t _r11_CH08_LD_TYPE = 0U;
                const uint16_t _r11_CH07_LD_TYPE = 0U;
                const uint16_t _r11_CH06_LD_TYPE = 0U;
                const uint16_t _r11_CH05_LD_TYPE = 0U;
                const uint16_t _r11_CH04_LD_TYPE = 0U;
                const uint16_t _r11_CH03_LD_TYPE = 0U;
                const uint16_t _r11_CH02_LD_TYPE = 0U;
                const uint16_t _r11_CH01_LD_TYPE = 0U;
                if (false == id601_write_ch_ld_type(daisy, _r11_CH12_LD_TYPE, _r11_CH11_LD_TYPE, _r11_CH10_LD_TYPE, _r11_CH09_LD_TYPE,
                    _r11_CH08_LD_TYPE, _r11_CH07_LD_TYPE, _r11_CH06_LD_TYPE, _r11_CH05_LD_TYPE,
                    _r11_CH04_LD_TYPE, _r11_CH03_LD_TYPE, _r11_CH02_LD_TYPE, _r11_CH01_LD_TYPE))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_FAULT_CONFIGURATION:
            {
                const uint16_t _r18_CRC_EN = 0U;
                const uint16_t _r18_WD_DIMM_EN = 0U;
                const uint16_t _r18_WD_TIMECFG = 0U;
                const uint16_t _r18_WD_EN = 0U;
                const uint16_t _r18_VS_MISS_DIMM_EN = 0U;
                const uint16_t _r18_VS_MISS_EN = 0U;
                const uint16_t _r18_CH_CTRL_MISMATCH_EN = 0U;
                const uint16_t _r18_VREF_OVUV_OFF_EN = 0U;
                const uint16_t _r18_VREF_OVUV_EN = 0U;
                const uint16_t _r18_OSC_LDO_OVUV_EN = 0U;
                const uint16_t _r18_LDO_OVUV_EN = 0U;
                if (false == id601_write_fault_configuration(daisy, _r18_CRC_EN, _r18_WD_DIMM_EN, _r18_WD_TIMECFG, _r18_WD_EN,
                    _r18_VS_MISS_DIMM_EN, _r18_VS_MISS_EN, _r18_CH_CTRL_MISMATCH_EN, _r18_VREF_OVUV_OFF_EN,
                    _r18_VREF_OVUV_EN, _r18_OSC_LDO_OVUV_EN, _r18_LDO_OVUV_EN))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_SERIAL_CLK_GEN:
            {
                const uint16_t _r1C_SCLK_LOW = 12U;
                const uint16_t _r1C_SCLK_HIGH = 24U;
                if (false == id601_write_serial_clk_gen(daisy, _r1C_SCLK_LOW, _r1C_SCLK_HIGH))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_SERIAL_LATENCY:
            {
                const uint16_t _r1D_SERIAL_LATENCY = 0x080U;
                if (false == id601_write_serial_latency(daisy, _r1D_SERIAL_LATENCY))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_TIMEOUT:
            {
                const uint16_t _r21_TIMEOUT = 0x3FFU;
                if (false == id601_write_timeout(daisy, _r21_TIMEOUT))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_FLLCNT1:
            {
                const uint16_t _r22_FLLCNT = 0x27EU;
                if (false == id601_write_fllcnt1(daisy, _r22_FLLCNT))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_FLLCNT2:
            {
                const uint16_t _r23_FLL_EN = 1U;
                const uint16_t _r23_FLL_RANGE = 2U;
                const uint16_t _r23_FLLCNT = 0x68U;
                if (false == id601_write_fllcnt2(daisy, _r23_FLL_EN, _r23_FLL_RANGE, _r23_FLLCNT))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_CHOP_EN:
            {
                const uint16_t _r26_OPEN_MASK_OPT = 0U;
                const uint16_t _r26_CHOP_EN = 1U;
                const uint16_t _r26_CHOP_DRV_EN = 1U;
                const uint16_t _r26_CHOP_OSC_LDO_EN = 1U;
                const uint16_t _r26_CHOP_OSC_EN = 1U;
                const uint16_t _r26_CHOP_DAC_EN = 1U;
                const uint16_t _r26_CHOP_BGR_EN = 1U;
                if (false == id601_write_chop_en(daisy, \
                    _r26_OPEN_MASK_OPT, _r26_CHOP_EN, _r26_CHOP_DRV_EN, _r26_CHOP_OSC_LDO_EN, _r26_CHOP_OSC_EN, _r26_CHOP_DAC_EN, _r26_CHOP_BGR_EN))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_VREF_FIX2:
            {
                const uint16_t _r2D_VREF_FIX2 = 1U;
                    if (false == id601_write_vref_fix2(daisy, _r2D_VREF_FIX2))
                {
                    return false;
                }
                break;
            }
            case ID601_ADDR_GENERAL_VREF_FIX3:
            {
                const uint16_t _r2E_VREF_FIX3 = 2U;
                    if (false == id601_write_vref_fix3(daisy, _r2E_VREF_FIX3))
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
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Dimming Type1 Mode OK");
    return true;
}

bool id601_set_dimming_type2_mode(void)
{
    for (uint16_t id601_addr = ID601_ADDR_GENERAL_RESET_ID ; id601_addr < ID601_ADDR_GENERAL_MAX_COUNT ; ++id601_addr)
    {
        switch (id601_addr)
        {
            default:
            {
                break;
            }
        }
    }
    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Dimming Type2 Mode OK");
    return true;
}

void id601_trim_setup_iref1(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    drv_gpio_id601_test_relay(ID601_RELAY_CON);
}
void id601_trim_setup_iref2(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF2_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    drv_gpio_id601_test_relay(ID601_RELAY_CON);
}
void id601_trim_setup_ldo_det(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VREF1P5_BGR2_RES);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    drv_gpio_id601_test_relay(ID601_RELAY_DIS);
}
void id601_trim_setup_ldo_dig(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_DIG_RES);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_trim_setup_ldo_dac(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_DAC_RES);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_trim_setup_ldo_fll(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_FLL_RES);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_trim_setup_ofs_temp(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    // id601_write_test_ana_en(1U, ID601_TEST_ANA_TEST_VREF2_ZTC_O1);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_trim_setup_ofs_temp_o1(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_TEST_VREF2_ZTC_O1);
}
void id601_trim_setup_ofs_temp_o2(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_TEST_VREF2_ZTC_O2);
}
void id601_trim_setup_osc_rctl(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 1U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    const uint16_t osc_man_en = 1U;
    const uint16_t osc_fll_err_range = 0U;
    const uint16_t osc_fll_man = 8U;
    id601_write_osc_fll_man2(1U, osc_man_en, osc_fll_err_range, osc_fll_man);
}
void id601_trim_setup_gain(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    const uint16_t gain_max_current_level3 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    const uint16_t gain_max_current_level2 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    const uint16_t gain_max_current_level1 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    id601_write_max_current_level(1U, gain_max_current_level3, gain_max_current_level2, gain_max_current_level1);
}
void id601_trim_setup_gain_vref1(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_max_curr_vref1(1U, ID601_TRIM_GAIN_VREF1);
    id601_write_max_curr_vref2(1U, ID601_TRIM_GAIN_VREF1);
    id601_write_max_curr_vref3(1U, ID601_TRIM_GAIN_VREF1);
}
void id601_trim_setup_gain_vref2(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_max_curr_vref1(1U, ID601_TRIM_GAIN_VREF2);
    id601_write_max_curr_vref2(1U, ID601_TRIM_GAIN_VREF2);
    id601_write_max_curr_vref3(1U, ID601_TRIM_GAIN_VREF2);
}
void id601_trim_setup_offset(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    const uint16_t offset_max_current_level3 = ID601_TRIM_OFFSET_MAX_CURR_LVL;
    const uint16_t offset_max_current_level2 = ID601_TRIM_OFFSET_MAX_CURR_LVL;
    const uint16_t offset_max_current_level1 = ID601_TRIM_OFFSET_MAX_CURR_LVL;
    id601_write_max_current_level(1U, offset_max_current_level3, offset_max_current_level2, offset_max_current_level1);
}
void id601_trim_setup_offset_vref1(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_max_curr_vref1(1U, ID601_TRIM_OFFSET_VREF1);
    id601_write_max_curr_vref2(1U, ID601_TRIM_OFFSET_VREF1);
    id601_write_max_curr_vref3(1U, ID601_TRIM_OFFSET_VREF1);
}
void id601_trim_setup_offset_vref2(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_max_curr_vref1(1U, ID601_TRIM_OFFSET_VREF2);
    id601_write_max_curr_vref2(1U, ID601_TRIM_OFFSET_VREF2);
    id601_write_max_curr_vref3(1U, ID601_TRIM_OFFSET_VREF2);
}

uint16_t id601_trim_apply_iref1(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    (void)(ch);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_IREF1_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = trim_code; // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_IREF1_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: trim_code: %u, reg_value: %u", __func__, trim_code, reg_value);

    uint16_t _r01 = gn_id601_mirror_regs[ID601_ADDR_MIRROR2][0];
    _r01 = id601_set_field(_r01, ID601_OTP2_IREF_CTL_Msk, ID601_OTP2_IREF_CTL_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, ID601_ADDR_MIRROR2, _r01, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_iref2(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    (void)(ch);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_IREF2_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = trim_code; // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_IREF2_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: trim_code: %u, reg_value: %u", __func__, trim_code, reg_value);

    uint16_t _r02 = gn_id601_mirror_regs[ID601_ADDR_MIRROR3][0];
    _r02 = id601_set_field(_r02, ID601_OTP3_IREF2_CTL_Msk, ID601_OTP3_IREF2_CTL_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, ID601_ADDR_MIRROR3, _r02, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_ldo_det(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    (void)(ch);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_LDO_BGR2_CTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = trim_code; // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_LDO_BGR2_CTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: trim_code: %u, reg_value: %u", __func__, trim_code, reg_value);

    uint16_t _r03 = gn_id601_mirror_regs[ID601_ADDR_MIRROR4][0];
    _r03 = id601_set_field(_r03, ID601_OTP4_LDO_BGR2_CTL_Msk, ID601_OTP4_LDO_BGR2_CTL_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, ID601_ADDR_MIRROR4, _r03, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_ldo_dig(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    (void)(ch);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_LDO_CTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = (trim_code ^ 0x08U); // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_LDO_CTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: trim_code: %u, reg_value: %u", __func__, trim_code, reg_value);

    uint16_t _r05 = gn_id601_mirror_regs[ID601_ADDR_MIRROR6][0];
    _r05 = id601_set_field(_r05, ID601_OTP6_LDO_CTL_Msk, ID601_OTP6_LDO_CTL_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, ID601_ADDR_MIRROR6, _r05, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_ldo_dac(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    (void)(ch);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_LDO_DAC_CTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = trim_code; // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_LDO_DAC_CTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: trim_code: %u, reg_value: %u", __func__, trim_code, reg_value);

    uint16_t _r04 = gn_id601_mirror_regs[ID601_ADDR_MIRROR5][0];
    _r04 = id601_set_field(_r04, ID601_OTP5_LDO_DAC_CTL_Msk, ID601_OTP5_LDO_DAC_CTL_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, ID601_ADDR_MIRROR5, _r04, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_ldo_fll(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    (void)(ch);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_LDO_OSC_CTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = trim_code; // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_LDO_OSC_CTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: trim_code: %u, reg_value: %u", __func__, trim_code, reg_value);

    uint16_t _r05 = gn_id601_mirror_regs[ID601_ADDR_MIRROR6][0];
    _r05 = id601_set_field(_r05, ID601_OTP6_LDO_OSC_CTL_Msk, ID601_OTP6_LDO_OSC_CTL_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, ID601_ADDR_MIRROR6, _r05, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_ofs_temp(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    (void)(ch);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_OFS_TEMP_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = trim_code; // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_OFS_TEMP_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: trim_code: %u, reg_value: %u", __func__, trim_code, reg_value);

    uint16_t _r03 = gn_id601_mirror_regs[ID601_ADDR_MIRROR4][0];
    _r03 = id601_set_field(_r03, ID601_OTP4_OFS_TEMP_Msk, ID601_OTP4_OFS_TEMP_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, ID601_ADDR_MIRROR4, _r03, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_osc_rctl(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    (void)(ch);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_OSC_RCTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = trim_code; // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_OSC_RCTL_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: trim_code: %u, reg_value: %u", __func__, trim_code, reg_value);

    uint16_t _r04 = gn_id601_mirror_regs[ID601_ADDR_MIRROR5][0];
    _r04 = id601_set_field(_r04, ID601_OTP5_OSC_RCTL_Msk, ID601_OTP5_OSC_RCTL_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, ID601_ADDR_MIRROR5, _r04, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_gain(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_GAIN_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = trim_code; // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_GAIN_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: ch:%u, trim_code: %u, reg_value: %u", __func__, ch, trim_code, reg_value);

    const uint16_t addr_gain = ID601_ADDR_MIRROR_GAIN_CH01 + ch;
    uint16_t _r_GAIN = gn_id601_mirror_regs[addr_gain][0];
    _r_GAIN = id601_set_field(_r_GAIN, ID601_GAIN_CH01_GAIN_CH1_Msk, ID601_GAIN_CH01_GAIN_CH1_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, addr_gain, _r_GAIN, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
uint16_t id601_trim_apply_offset(uint16_t ch, uint16_t trim_code)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    uint16_t ret = 0xFFFFU;

    if (trim_code > ID601_TRIM_CODE_OFFSET_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: trim_code out of range: %u", __func__, trim_code);
        USER_ASSERT(0);
    }

    const uint16_t reg_value = (trim_code >= 256U) ? (trim_code - 256U) : (511U - trim_code); // refer to ID601_Test_Doc.xlsx
    if (reg_value > ID601_TRIM_REG_OFFSET_MAX)
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: reg_value out of range: %u", __func__, reg_value);
        USER_ASSERT(0);
    }

    drv_uart_printf(LOG_LV_DEBUG, "\r\n    [ID601] %s: ch:%u, trim_code: %u, reg_value: %u", __func__, ch, trim_code, reg_value);

    const uint16_t addr_offset = ID601_ADDR_MIRROR_OFS_CH01 + ch;
    uint16_t _r_OFFSET = gn_id601_mirror_regs[addr_offset][0];
    _r_OFFSET = id601_set_field(_r_OFFSET, ID601_OFFSET_CH01_OFS_CH1_Msk, ID601_OFFSET_CH01_OFS_CH1_Pos, reg_value);
    if (true == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, addr_offset, _r_OFFSET, gp_id601_crc_func))
    {
        ret = reg_value;
    }
    return ret;
}
void id601_trim_setup_current_chx(uint16_t ch)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    uint16_t channel[12] = { 0U };
    channel[ch] = 1U;
    if (false == id601_write_channel_enable(1U, channel[11], channel[10], channel[9], channel[8], channel[7], channel[6], channel[5], channel[4], channel[3], channel[2], channel[1], channel[0]))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: Failed to write channel enable", __func__);
    }
}
void id601_trim_setup_efuse(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 0U;
    const uint16_t _r3F_PWMOUT_FULL = 0U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    const uint16_t otp_pg_done = 0U;
    const uint16_t otp_rd_s = 0U;
    const uint16_t otp_pg_s = 1U;
    id601_write_otp_rd_prog(1U, otp_pg_done, otp_rd_s, otp_pg_s);
}
bool id601_trim_save_search_registers(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);

    for (uint16_t addr = 0U; addr < ID601_ADDR_MIRROR_MAX_COUNT; ++addr)
    {
        if (false == id601_read_register(ID601_REGISTER_BANK_MIRROR, 1U, addr, gp_id601_crc_func, NULL))
        {
            return false;
        }
        gn_id601_trim_search_mirror_regs[addr] = gn_id601_mirror_regs[addr][0];
    }
    return true;
}
static void id601_checksum_append_field(uint8_t *checksum, uint16_t *bit_count, uint32_t value, uint8_t width)
{
    for (uint8_t remaining = width; remaining > 0U; --remaining)
    {
        const uint8_t bit = (uint8_t)((value >> (remaining - 1U)) & 1U);
        const uint8_t position = (uint8_t)(7U - (*bit_count % 8U));
        *checksum ^= (uint8_t)(bit << position);
        ++(*bit_count);
    }
}
bool id601_trim_calculate_otp_checksum(uint8_t *checksum)
{
    if (NULL == checksum)
    {
        return false;
    }

    const uint16_t *mirror = gn_id601_trim_search_mirror_regs;

    uint8_t result = 0U;
    uint16_t bit_count = 0U;

    /* ===== OTP1: 128 bits ===== */

    /* GAIN_CH1~12: 각 7 bits */
    for (uint8_t ch = 0U; ch < 12U; ++ch)
    {
        id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR_GAIN_CH01 + ch], ID601_GAIN_CH01_GAIN_CH1_Msk, ID601_GAIN_CH01_GAIN_CH1_Pos), 7U);
    }

    /* LDO_OSC_CTL: 4 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR6], ID601_OTP6_LDO_OSC_CTL_Msk, ID601_OTP6_LDO_OSC_CTL_Pos), 4U);

    /* LDO_CTL: 4 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR6], ID601_OTP6_LDO_CTL_Msk, ID601_OTP6_LDO_CTL_Pos), 4U);

    /* OFS_CH1~4: 각 9 bits */
    for (uint8_t ch = 0U; ch < 4U; ++ch)
    {
        id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR_OFS_CH01 + ch], ID601_OFFSET_CH01_OFS_CH1_Msk, ID601_OFFSET_CH01_OFS_CH1_Pos), 9U);
    }

    /* ===== OTP2: checksum을 제외한 120 bits ===== */

    /* OFS_CH5~12: 각 9 bits */
    for (uint8_t ch = 0U; ch < 8U; ++ch)
    {
        id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR_OFS_CH05 + ch], ID601_OFFSET_CH01_OFS_CH1_Msk, ID601_OFFSET_CH01_OFS_CH1_Pos), 9U);
    }

    /* OSC_RCTL: 5 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR5], ID601_OTP5_OSC_RCTL_Msk, ID601_OTP5_OSC_RCTL_Pos), 5U);

    /* LDO_DAC_CTL: 5 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR5], ID601_OTP5_LDO_DAC_CTL_Msk, ID601_OTP5_LDO_DAC_CTL_Pos), 5U);

    /* IREF_CTL: 5 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR2], ID601_OTP2_IREF_CTL_Msk, ID601_OTP2_IREF_CTL_Pos), 5U);

    /* BGR_TC: 5 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR2], ID601_OTP2_BGR_TC_Msk, ID601_OTP2_BGR_TC_Pos), 5U);

    /* OFS_TEMP: 4 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR4], ID601_OTP4_OFS_TEMP_Msk, ID601_OTP4_OFS_TEMP_Pos), 4U);

    /* CHIP_INFO: 8 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR_VERSION], ID601_VERSION_VERSION0_Msk, ID601_VERSION_VERSION0_Pos), 8U);

    /* IREF2_CTL: 5 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR3], ID601_OTP3_IREF2_CTL_Msk, ID601_OTP3_IREF2_CTL_Pos), 5U);

    /* BGR2_TC: 5 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR3], ID601_OTP3_BGR2_TC_Msk, ID601_OTP3_BGR2_TC_Pos), 5U);

    /* LDO_BGR2_CTL: 5 bits */
    id601_checksum_append_field(&result, &bit_count, id601_get_field(mirror[ID601_ADDR_MIRROR4], ID601_OTP4_LDO_BGR2_CTL_Msk, ID601_OTP4_LDO_BGR2_CTL_Pos), 5U);

    /* OTP_PG_DONE: 1 bit */
    const uint32_t otp_pg_done = 1U;
    id601_checksum_append_field(&result, &bit_count, otp_pg_done, 1U);

    /* checksum 자신인 OTP2[7:0]은 제외 */
    *checksum = result;
    return true;
}

bool id601_trim_compare_search_registers(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    bool result = true;

    uint8_t expect_checksum = 0U;
    if (false == id601_trim_calculate_otp_checksum(&expect_checksum))
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to calculate OTP checksum");
        return false;
    }

    uint16_t mirror_reboot[ID601_ADDR_MIRROR_MAX_COUNT] = { 0U };

    // 1. read mirror registers after power reboot
    for (uint16_t addr = 0U; addr < ID601_ADDR_MIRROR_MAX_COUNT; ++addr)
    {
        if (false == id601_read_register(ID601_REGISTER_BANK_MIRROR, 1U, addr, gp_id601_crc_func, NULL))
        {
            return false;
        }
        mirror_reboot[addr] = gn_id601_mirror_regs[addr][0];
    }

    // 2. compare mirror registers with the saved ones
    for (uint16_t addr = 0U; addr < ID601_ADDR_MIRROR_MAX_COUNT; ++addr)
    {
        uint16_t expected = gn_id601_trim_search_mirror_regs[addr];
        uint16_t actual = mirror_reboot[addr];

        if (ID601_ADDR_MIRROR1 == addr)
        {
            expected = expect_checksum;
            actual = id601_get_field(actual, ID601_OTP1_OTP_CRC_CHECKSUM_Msk, ID601_OTP1_OTP_CRC_CHECKSUM_Pos);
        }

        if (actual != expected)
        {
            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] OTP MISMATCH: addr 0x%02X reboot=0x%03X saved=0x%03X", addr, actual, expected);
            result = false;
        }
        else
        {
            drv_uart_printf(LOG_LV_INFO,  "\r\n    [ID601] OTP MATCH:    addr 0x%02X reboot=0x%03X saved=0x%03X", addr, actual, expected);
        }
    }
    return result;
}
bool id601_trim_display_search_registers(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);

    for (uint16_t addr = 0U; addr < ID601_ADDR_MIRROR_MAX_COUNT; ++addr)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Mirror register: addr 0x%02X search=%u", addr, gn_id601_trim_search_mirror_regs[addr]);
    }
    return true;
}
bool id601_trim_overwrite_search_registers(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);

    drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Overwrite Mirror register");
    for (uint16_t addr = 0U; addr < ID601_ADDR_MIRROR_MAX_COUNT; ++addr)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Overwriting... addr 0x%02X value=0x%03X", addr, gn_id601_trim_search_mirror_regs[addr]);
        if (false == id601_write_register(ID601_REGISTER_BANK_MIRROR, 1U, addr, gn_id601_trim_search_mirror_regs[addr], gp_id601_crc_func))
        {
            drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] Failed to overwrite mirror register: addr 0x%02X", addr);
            return false;
        }
    }
    return true;
}
static bool id601_test_check_is_trimmed(void)
{
    uint16_t _r3D = 0x0U;
    if (false == id601_read_register(ID601_REGISTER_BANK_GENERAL, 1U, ID601_ADDR_GENERAL_OTP_RD_PROG, gp_id601_crc_func, &_r3D))
    {
        drv_uart_printf(LOG_LV_WARN, "\r\n    [ID601] %s: Failed to read general register 0x%03X", __func__, ID601_ADDR_GENERAL_OTP_RD_PROG);
        return false;
    }

    return ((_r3D & 0x0800U) != 0U);
}
void id601_test_setup_idds1(void)
{
    if (true == gb_id601_hvs_flag)
    {
        drv_gpio_id601_vcc(ID601_VCC_5V5);
        drv_tim_delay_ms(490U); // total 500ms (10ms @ drv_gpio_id601_vcc function)
    }
    drv_gpio_id601_vcc(ID601_VCC_3V3);
}
void id601_test_setup_iddd1(void)
{
    if (false == id601_init_tm_entry_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 TM Entry Mode Failed", __func__);
    }

    if (false == id601_set_analog_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
    }
}
void id601_test_setup_iref1(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);

    const bool already_trimmed = id601_test_check_is_trimmed();
    if (true == already_trimmed)
    {
        drv_uart_printf(LOG_LV_INFO, "\r\n    [ID601] Already trimmed. Skip overwrite");
    }
    else
    {
        id601_trim_overwrite_search_registers();
    }

    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    drv_gpio_id601_test_relay(ID601_RELAY_CON);
}
void id601_test_setup_iref2(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF2_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    drv_gpio_id601_test_relay(ID601_RELAY_CON);
}
void id601_test_setup_ldo_det(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VREF1P5_BGR2_RES);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    drv_gpio_id601_test_relay(ID601_RELAY_DIS);
}
void id601_test_setup_ldo_dig(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_DIG_RES);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_test_setup_ldo_dac(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_DAC_RES);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_test_setup_ldo_fll(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VDD1P5_FLL_RES);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_test_setup_bgr1(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_test_setup_bgr2(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_VREF2_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);
}
void id601_test_setup_osc(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 1U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    const uint16_t osc_man_en = 1U;
    const uint16_t osc_fll_err_range = 0U;
    const uint16_t osc_fll_man = 8U;
    id601_write_osc_fll_man2(1U, osc_man_en, osc_fll_err_range, osc_fll_man);
}
void id601_test_setup_fll1(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    if (false == id601_set_fll_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 FLL Test Mode Failed", __func__);
        return;
    }

    const uint64_t tgt_hz = 40000000ULL;
    const uint64_t fllcnt = (tgt_hz / (120U * 2U));
    const uint16_t fll_lsb = (uint16_t)(fllcnt & 0x0FFFU);
    const uint16_t fll_msb = (uint16_t)((fllcnt >> 12U) & 0x0FFFU);

    const uint16_t fllcnt1 = fll_lsb;
    id601_write_fllcnt1(1U, fllcnt1);

    const uint16_t fllcnt2_en = 0x1U;
    const uint16_t fllcnt2_rng = 0x2U;
    const uint16_t fllcnt2_cnt = fll_msb;
    id601_write_fllcnt2(1U, fllcnt2_en, fllcnt2_rng, fllcnt2_cnt);
}
void id601_test_setup_fll2(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    if (false == id601_set_fll_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 FLL Test Mode Failed", __func__);
        return;
    }

    const uint64_t tgt_hz = 60000000ULL;
    const uint64_t fllcnt = (tgt_hz / (120U * 2U));
    const uint16_t fll_lsb = (uint16_t)(fllcnt & 0x0FFFU);
    const uint16_t fll_msb = (uint16_t)((fllcnt >> 12U) & 0x0FFFU);

    const uint16_t fllcnt1 = fll_lsb;
    id601_write_fllcnt1(1U, fllcnt1);

    const uint16_t fllcnt2_en = 0x1U;
    const uint16_t fllcnt2_rng = 0x2U;
    const uint16_t fllcnt2_cnt = fll_msb;
    id601_write_fllcnt2(1U, fllcnt2_en, fllcnt2_rng, fllcnt2_cnt);
}
void id601_test_setup_fll3(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    if (false == id601_set_fll_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 FLL Test Mode Failed", __func__);
        return;
    }

    const uint64_t tgt_hz = 50000000ULL;
    const uint64_t fllcnt = (tgt_hz / (120U * 2U));
    const uint16_t fll_lsb = (uint16_t)(fllcnt & 0x0FFFU);
    const uint16_t fll_msb = (uint16_t)((fllcnt >> 12U) & 0x0FFFU);

    const uint16_t fllcnt1 = fll_lsb;
    id601_write_fllcnt1(1U, fllcnt1);

    const uint16_t fllcnt2_en = 0x1U;
    const uint16_t fllcnt2_rng = 0x2U;
    const uint16_t fllcnt2_cnt = fll_msb;
    id601_write_fllcnt2(1U, fllcnt2_en, fllcnt2_rng, fllcnt2_cnt);
}
void id601_test_setup_iout_3p(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);

    if (false == id601_set_analog_test_mode())
    {
        drv_uart_printf(LOG_LV_ERROR, "\r\n    [ID601] %s: ID601 Analog Test Mode Failed", __func__);
    }

    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    const uint16_t gain_max_current_level3 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    const uint16_t gain_max_current_level2 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    const uint16_t gain_max_current_level1 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    id601_write_max_current_level(1U, gain_max_current_level3, gain_max_current_level2, gain_max_current_level1);
}
void id601_test_setup_iout_3p_x(uint8_t ph)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    USER_ASSERT(ph < 3);

    const uint16_t vref_table[3] = {ID601_VFY_3P_VREF1, ID601_VFY_3P_VREF2, ID601_VFY_3P_VREF3};
    const uint8_t table_idx = ph;
    id601_write_max_curr_vref1(1U, vref_table[table_idx]);
    id601_write_max_curr_vref2(1U, vref_table[table_idx]);
    id601_write_max_curr_vref3(1U, vref_table[table_idx]);
}
void id601_test_setup_iout_sweep(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    id601_write_max_curr_vref1(1U, ID601_VFY_SWEEP_VREF);
    id601_write_max_curr_vref2(1U, ID601_VFY_SWEEP_VREF);
    id601_write_max_curr_vref3(1U, ID601_VFY_SWEEP_VREF);
}
void id601_test_setup_iout_sweep_x(uint8_t ph)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    USER_ASSERT(ph <= ID601_MAX_CURRENT_LEVEL_64mA);

    const uint16_t gain_max_current_level3 = (ID601_MAX_CURRENT_LEVEL_04mA + ph);
    const uint16_t gain_max_current_level2 = (ID601_MAX_CURRENT_LEVEL_04mA + ph);
    const uint16_t gain_max_current_level1 = (ID601_MAX_CURRENT_LEVEL_04mA + ph);
    id601_write_max_current_level(1U, gain_max_current_level3, gain_max_current_level2, gain_max_current_level1);
}
void id601_test_setup_iout_vref_sweep(void)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    id601_write_test_ana_en(1U, ID601_TEST_ANA_IREF_TEST);

    const uint16_t _r3F_TEST_EN = 1U;
    const uint16_t _r3F_SW_SEL = 1U;
    const uint16_t _r3F_PWMOUT_FULL = 1U;
    const uint16_t _r3F_MCLK64_O = 0U;
    const uint16_t _r3F_DTEST_MUX_EN = 0U;
    const uint16_t _r3F_RD_EN = 1U;
    const uint16_t _r3F_EXT_CLKIN = 0U;
    const uint16_t _r3F_ADDR_EXT = 0U;
    id601_write_op_mode(1U, _r3F_TEST_EN, _r3F_SW_SEL, _r3F_PWMOUT_FULL, _r3F_MCLK64_O, _r3F_DTEST_MUX_EN, _r3F_RD_EN, _r3F_EXT_CLKIN, _r3F_ADDR_EXT);

    const uint16_t gain_max_current_level3 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    const uint16_t gain_max_current_level2 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    const uint16_t gain_max_current_level1 = ID601_TRIM_GAIN_MAX_CURR_LVL;
    id601_write_max_current_level(1U, gain_max_current_level3, gain_max_current_level2, gain_max_current_level1);
}
void id601_test_setup_iout_vref_sweep_x(uint16_t vref)
{
    USER_ASSERT(1U == gn_id601_daisy_length);
    USER_ASSERT(vref <= 4095);

    id601_write_max_curr_vref1(1U, vref);
    id601_write_max_curr_vref2(1U, vref);
    id601_write_max_curr_vref3(1U, vref);
}

/* USER CODE END 0 */
