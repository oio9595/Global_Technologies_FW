/* USER CODE BEGIN Header */
/*
    * File:   id804_metadata.h
    * Author: GT
    *
    * Created on 2026. 08. 31.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ID804_METADATA_H__
#define __ID804_METADATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
/* 2. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
typedef enum tag_ID804_CMD_LIST
{
    ID804_CMD_RESET                = 0xB1U,
    ID804_CMD_INITBIDIR            = 0xB2U,
    ID804_CMD_CLRERROR             = 0xB3U,
    ID804_CMD_GOSLEEP              = 0xB4U,
    ID804_CMD_GOACTIVE             = 0xB8U,
    ID804_CMD_GODEEPSLEEP          = 0xBCU,

    ID804_CMD_SET_SETUP1           = 0x86U,
    ID804_CMD_SET_SETUP2           = 0x87U,
    ID804_CMD_SET_MCAST            = 0x88U,
    ID804_CMD_SET_TEMPTH           = 0x8AU,
    ID804_CMD_SET_TEMPHYS          = 0x8BU,
    ID804_CMD_SET_CURR_MAX_LVL     = 0x8FU,
    ID804_CMD_SET_TEMP_LUT_TC1     = 0x90U,
    ID804_CMD_SET_TEMP_LUT_TC2     = 0x91U,
    ID804_CMD_SET_TEMP_LUT_TC3     = 0x92U,
    ID804_CMD_SET_TEMP_LUT_TC4     = 0x93U,
    ID804_CMD_SET_TEMP_LUT_TC5     = 0x94U,
    ID804_CMD_SET_TEMP_LUT_TC6     = 0x95U,
    ID804_CMD_SET_TEMP_LUT_TC7     = 0x96U,
    ID804_CMD_SET_TEMP_LUT_TC8     = 0x97U,
    ID804_CMD_SET_TEMP_LUT_TC9     = 0x98U,
    ID804_CMD_SET_TEMP_LUT_TC10    = 0x99U,
    ID804_CMD_SET_TIMEOUT          = 0x9AU,
    ID804_CMD_SET_RGB              = 0xA0U,

    ID804_CMD_READ_STATUS1         = 0x41U,
    ID804_CMD_READ_STATUS2         = 0x42U,
    ID804_CMD_READ_TEMP            = 0x43U,
    ID804_CMD_READ_TEMPST          = 0x44U,
    ID804_CMD_READ_VEXT_TM         = 0x45U,
    ID804_CMD_READ_SETUP1          = 0x46U,
    ID804_CMD_READ_SETUP2          = 0x47U,
    ID804_CMD_READ_MCAST           = 0x48U,
    ID804_CMD_READ_TEMPTH          = 0x4AU,
    ID804_CMD_READ_TEMPHYS         = 0x4BU,
    ID804_CMD_READ_PWM_RED_VAL     = 0x4CU,
    ID804_CMD_READ_PWM_GREEN_VAL   = 0x4DU,
    ID804_CMD_READ_PWM_BLUE_VAL    = 0x4EU,
    ID804_CMD_READ_CURR_MAX_LVL    = 0x4FU,
    ID804_CMD_READ_TEMP_LUT_TC1    = 0x50U,
    ID804_CMD_READ_TEMP_LUT_TC2    = 0x51U,
    ID804_CMD_READ_TEMP_LUT_TC3    = 0x52U,
    ID804_CMD_READ_TEMP_LUT_TC4    = 0x53U,
    ID804_CMD_READ_TEMP_LUT_TC5    = 0x54U,
    ID804_CMD_READ_TEMP_LUT_TC6    = 0x55U,
    ID804_CMD_READ_TEMP_LUT_TC7    = 0x56U,
    ID804_CMD_READ_TEMP_LUT_TC8    = 0x57U,
    ID804_CMD_READ_TEMP_LUT_TC9    = 0x58U,
    ID804_CMD_READ_TEMP_LUT_TC10   = 0x59U,
    ID804_CMD_READ_TIMEOUT         = 0x5AU,
    ID804_CMD_READ_RGB             = 0x60U,

    ID804_CMD_MAX
} id804_cmd_list_t;

/* ==========================================
    * Register Addresses
========================================== */

#define ID804_ADDR_STATUS1          (0x01U)
#define ID804_ADDR_STATUS2          (0x02U)
#define ID804_ADDR_TEMP             (0x03U)
#define ID804_ADDR_VEXT_TM          (0x05U)
#define ID804_ADDR_SETUP1           (0x06U)
#define ID804_ADDR_SETUP2           (0x07U)
#define ID804_ADDR_TEMPTH           (0x0AU)
#define ID804_ADDR_TEMPHYS          (0x0BU)
#define ID804_ADDR_CAL_PWM_RED      (0x0CU)
#define ID804_ADDR_CAL_PWM_GREEN    (0x0DU)
#define ID804_ADDR_CAL_PWM_BLUE     (0x0EU)
#define ID804_ADDR_CURR_MAX_LVL     (0x0FU)
#define ID804_ADDR_TEMP_LUT_TC1     (0x10U)
#define ID804_ADDR_TEMP_LUT_TC2     (0x11U)
#define ID804_ADDR_TEMP_LUT_TC3     (0x12U)
#define ID804_ADDR_TEMP_LUT_TC4     (0x13U)
#define ID804_ADDR_TEMP_LUT_TC5     (0x14U)
#define ID804_ADDR_TEMP_LUT_TC6     (0x15U)
#define ID804_ADDR_TEMP_LUT_TC7     (0x16U)
#define ID804_ADDR_TEMP_LUT_TC8     (0x17U)
#define ID804_ADDR_TEMP_LUT_TC9     (0x18U)
#define ID804_ADDR_TEMP_LUT_TC10    (0x19U)
#define ID804_ADDR_TIMEOUT          (0x1AU)
#define ID804_ADDR_MCAST_1          (0x08U)
#define ID804_ADDR_MCAST_2          (0x09U)
#define ID804_ADDR_RGB_1            (0x20U)
#define ID804_ADDR_RGB_2            (0x21U)

/* ==========================================
    * OTP Register Addresses (0xE0 ~ 0xF6)
========================================== */

#define ID804_ADDR_TRIM_CONTROL     (0xE0)
#define ID804_ADDR_OTP_CONTROL1     (0xE1)
#define ID804_ADDR_OTP_CONTROL2     (0xE2)
#define ID804_ADDR_OTP_CONTROL3     (0xE3)
#define ID804_ADDR_OTP1_MIRROR1     (0xE4)
#define ID804_ADDR_OTP1_MIRROR2     (0xE5)
#define ID804_ADDR_OTP1_MIRROR3     (0xE6)
#define ID804_ADDR_OTP1_MIRROR4     (0xE7)
#define ID804_ADDR_OTP1_MIRROR5     (0xE8)
#define ID804_ADDR_OTP1_MIRROR6     (0xE9)
#define ID804_ADDR_OTP1_MIRROR7     (0xEA)
#define ID804_ADDR_OTP1_MIRROR8     (0xEB)
#define ID804_ADDR_OTP1_MIRROR9     (0xEC)
#define ID804_ADDR_OTP1_MIRROR10    (0xED)
#define ID804_ADDR_OTP1_MIRROR11    (0xEE)
#define ID804_ADDR_OTP1_MIRROR12    (0xEF)
#define ID804_ADDR_OTP1_MIRROR13    (0xF0)
#define ID804_ADDR_OTP2_MIRROR14    (0xF1)
#define ID804_ADDR_OTP2_MIRROR15    (0xF2)
#define ID804_ADDR_OTP2_MIRROR16    (0xF3)
#define ID804_ADDR_OTP2_MIRROR17    (0xF4)
#define ID804_ADDR_OTP2_MIRROR18    (0xF5)
#define ID804_ADDR_OTP2_MIRROR19    (0xF6)

/* ==========================================
 * Register Bit Field Definitions (Shift & Mask)
 * ========================================== */

// STATUS1 (0x01) - RO
#define ID804_STATUS1_IC_STATE_POS          (4U)
#define ID804_STATUS1_IC_STATE_MSK          (0x007U << ID804_STATUS1_IC_STATE_POS)
#define ID804_STATUS1_SIO2_POS              (2U)
#define ID804_STATUS1_SIO2_MSK              (0x003U << ID804_STATUS1_SIO2_POS)
#define ID804_STATUS1_SIO1_POS              (0U)
#define ID804_STATUS1_SIO1_MSK              (0x003U << ID804_STATUS1_SIO1_POS)

// STATUS2 (0x02) - RC
#define ID804_STATUS2_OTPCRC_POS            (11U)
#define ID804_STATUS2_OTPCRC_MSK            (0x001U << ID804_STATUS2_OTPCRC_POS)
#define ID804_STATUS2_T_OUT_POS             (10U)
#define ID804_STATUS2_T_OUT_MSK             (0x001U << ID804_STATUS2_T_OUT_POS)
#define ID804_STATUS2_OPEN_R_POS            (9U)
#define ID804_STATUS2_OPEN_R_MSK            (0x001U << ID804_STATUS2_OPEN_R_POS)
#define ID804_STATUS2_OPEN_G_POS            (8U)
#define ID804_STATUS2_OPEN_G_MSK            (0x001U << ID804_STATUS2_OPEN_G_POS)
#define ID804_STATUS2_OPEN_B_POS            (7U)
#define ID804_STATUS2_OPEN_B_MSK            (0x001U << ID804_STATUS2_OPEN_B_POS)
#define ID804_STATUS2_SHORT_R_POS           (6U)
#define ID804_STATUS2_SHORT_R_MSK           (0x001U << ID804_STATUS2_SHORT_R_POS)
#define ID804_STATUS2_SHORT_G_POS           (5U)
#define ID804_STATUS2_SHORT_G_MSK           (0x001U << ID804_STATUS2_SHORT_G_POS)
#define ID804_STATUS2_SHORT_B_POS           (4U)
#define ID804_STATUS2_SHORT_B_MSK           (0x001U << ID804_STATUS2_SHORT_B_POS)
#define ID804_STATUS2_OT_FLT_POS            (3U)
#define ID804_STATUS2_OT_FLT_MSK            (0x001U << ID804_STATUS2_OT_FLT_POS)
#define ID804_STATUS2_UV_FLT_POS            (2U)
#define ID804_STATUS2_UV_FLT_MSK            (0x001U << ID804_STATUS2_UV_FLT_POS)
#define ID804_STATUS2_CRC_FLT_POS           (1U)
#define ID804_STATUS2_CRC_FLT_MSK           (0x001U << ID804_STATUS2_CRC_FLT_POS)
#define ID804_STATUS2_COM_FLT_POS           (0U)
#define ID804_STATUS2_COM_FLT_MSK           (0x001U << ID804_STATUS2_COM_FLT_POS)

// TEMP (0x03) - RO
#define ID804_TEMP_TEMPERATURE_POS          (0U)
#define ID804_TEMP_TEMPERATURE_MSK          (0x3FFU << ID804_TEMP_TEMPERATURE_POS)

// VEXT_TM (0x05) - RO
#define ID804_VEXT_TM_VEXT_TM_POS           (0U)
#define ID804_VEXT_TM_VEXT_TM_MSK           (0x3FFU << ID804_VEXT_TM_VEXT_TM_POS)

// SETUP1 (0x06) - RW
#define ID804_SETUP1_TC_E_POS               (11U)
#define ID804_SETUP1_TC_E_MSK               (0x001U << ID804_SETUP1_TC_E_POS)
#define ID804_SETUP1_OS_FLT_E_POS           (10U)
#define ID804_SETUP1_OS_FLT_E_MSK           (0x001U << ID804_SETUP1_OS_FLT_E_POS)
#define ID804_SETUP1_OT_FLT_E_POS           (9U)
#define ID804_SETUP1_OT_FLT_E_MSK           (0x001U << ID804_SETUP1_OT_FLT_E_POS)
#define ID804_SETUP1_UV_FLT_E_POS           (8U)
#define ID804_SETUP1_UV_FLT_E_MSK           (0x001U << ID804_SETUP1_UV_FLT_E_POS)
#define ID804_SETUP1_COM_FLT_E_POS          (7U)
#define ID804_SETUP1_COM_FLT_E_MSK          (0x001U << ID804_SETUP1_COM_FLT_E_POS)
#define ID804_SETUP1_CRC_E_POS              (6U)
#define ID804_SETUP1_CRC_E_MSK              (0x001U << ID804_SETUP1_CRC_E_POS)
#define ID804_SETUP1_PH_SHIFT_E_POS         (5U)
#define ID804_SETUP1_PH_SHIFT_E_MSK         (0x001U << ID804_SETUP1_PH_SHIFT_E_POS)
#define ID804_SETUP1_LG_E_POS               (4U)
#define ID804_SETUP1_LG_E_MSK               (0x001U << ID804_SETUP1_LG_E_POS)
#define ID804_SETUP1_F_PWM_DIV_POS          (0U)
#define ID804_SETUP1_F_PWM_DIV_MSK          (0x00FU << ID804_SETUP1_F_PWM_DIV_POS)

// SETUP2 (0x07) - RW
#define ID804_SETUP2_CLK_INV_E_POS          (6U)
#define ID804_SETUP2_CLK_INV_E_MSK          (0x001U << ID804_SETUP2_CLK_INV_E_POS)
#define ID804_SETUP2_VEXT_MON_E_POS         (5U)
#define ID804_SETUP2_VEXT_MON_E_MSK         (0x001U << ID804_SETUP2_VEXT_MON_E_POS)
#define ID804_SETUP2_EVENT_CYC_POS          (4U)
#define ID804_SETUP2_EVENT_CYC_MSK          (0x001U << ID804_SETUP2_EVENT_CYC_POS)
#define ID804_SETUP2_UV_LVL_POS             (2U)
#define ID804_SETUP2_UV_LVL_MSK             (0x003U << ID804_SETUP2_UV_LVL_POS)
#define ID804_SETUP2_SH_LVL_POS             (0U)
#define ID804_SETUP2_SH_LVL_MSK             (0x003U << ID804_SETUP2_SH_LVL_POS)

// TEMPTH (0x0A) - RW
#define ID804_TEMPTH_THRESHOLD_POS          (0U)
#define ID804_TEMPTH_THRESHOLD_MSK          (0x3FFU << ID804_TEMPTH_THRESHOLD_POS)

// TEMPHYS (0x0B) - RW
#define ID804_TEMPHYS_HYSTERESIS_POS        (0U)
#define ID804_TEMPHYS_HYSTERESIS_MSK        (0x3FFU << ID804_TEMPHYS_HYSTERESIS_POS)

// CAL_PWM_RED / GREEN / BLUE (0x0C, 0x0D, 0x0E) - RO
#define ID804_CAL_PWM_VAL_POS               (0U)
#define ID804_CAL_PWM_VAL_MSK               (0xFFFU << ID804_CAL_PWM_VAL_POS)

// CURR_MAX_LVL (0x0F) - RW
#define ID804_CURR_MAX_R_POS                (8U)
#define ID804_CURR_MAX_R_MSK                (0x00FU << ID804_CURR_MAX_R_POS)
#define ID804_CURR_MAX_G_POS                (4U)
#define ID804_CURR_MAX_G_MSK                (0x00FU << ID804_CURR_MAX_G_POS)
#define ID804_CURR_MAX_B_POS                (0U)
#define ID804_CURR_MAX_B_MSK                (0x00FU << ID804_CURR_MAX_B_POS)

// TEMP_LUT_TC1 ~ TC10 (0x10 ~ 0x19) - RW
#define ID804_TEMP_LUT_TC_POS               (0U)
#define ID804_TEMP_LUT_TC_MSK               (0x3FFU << ID804_TEMP_LUT_TC_POS)

// TIMEOUT (0x1A) - RW
#define ID804_TIMEOUT_VAL_POS               (0U)
#define ID804_TIMEOUT_VAL_MSK               (0xFFFU << ID804_TIMEOUT_VAL_POS)

// MCAST_1 (0x08) - RW
#define ID804_MCAST_1_ADDR_POS              (0U)
#define ID804_MCAST_1_ADDR_MSK              (0x00FU << ID804_MCAST_1_ADDR_POS)

// MCAST_2 (0x09) - RW
#define ID804_MCAST_2_ADDR_POS              (0U)
#define ID804_MCAST_2_ADDR_MSK              (0xFFFU << ID804_MCAST_2_ADDR_POS)

// RGB_1 (0x20) - RW
#define ID804_RGB_1_R_DATA_POS              (4U)
#define ID804_RGB_1_R_DATA_MSK              (0x0FFU << ID804_RGB_1_R_DATA_POS)
#define ID804_RGB_1_G_DATA_POS              (0U)
#define ID804_RGB_1_G_DATA_MSK              (0x00FU << ID804_RGB_1_G_DATA_POS)

// RGB_2 (0x21) - RW
#define ID804_RGB_2_G_DATA_POS              (8U)
#define ID804_RGB_2_G_DATA_MSK              (0x00FU << ID804_RGB_2_G_DATA_POS)
#define ID804_RGB_2_B_DATA_POS              (0U)
#define ID804_RGB_2_B_DATA_MSK              (0x0FFU << ID804_RGB_2_B_DATA_POS)

// TRIM_CONTROL (0xE0)
#define ID804_TRIM_TEST_E_POS               (11U)
#define ID804_TRIM_TEST_E_MSK               (0x001U << ID804_TRIM_TEST_E_POS)
#define ID804_TRIM_CKO_E_POS                (10U)
#define ID804_TRIM_CKO_E_MSK                (0x001U << ID804_TRIM_CKO_E_POS)
#define ID804_TRIM_ADC_CLK_POL_POS          (9U)
#define ID804_TRIM_ADC_CLK_POL_MSK          (0x001U << ID804_TRIM_ADC_CLK_POL_POS)
#define ID804_TRIM_ANA_MOD_SEL_POS          (6U)
#define ID804_TRIM_ANA_MOD_SEL_MSK          (0x007U << ID804_TRIM_ANA_MOD_SEL_POS)
#define ID804_TRIM_T_ANA_SEL_POS            (3U)
#define ID804_TRIM_T_ANA_SEL_MSK            (0x007U << ID804_TRIM_T_ANA_SEL_POS)
#define ID804_TRIM_PWM_MAX_R_E_POS          (2U)
#define ID804_TRIM_PWM_MAX_R_E_MSK          (0x001U << ID804_TRIM_PWM_MAX_R_E_POS)
#define ID804_TRIM_PWM_MAX_G_E_POS          (1U)
#define ID804_TRIM_PWM_MAX_G_E_MSK          (0x001U << ID804_TRIM_PWM_MAX_G_E_POS)
#define ID804_TRIM_PWM_MAX_B_E_POS          (0U)
#define ID804_TRIM_PWM_MAX_B_E_MSK          (0x001U << ID804_TRIM_PWM_MAX_B_E_POS)

// OTP_CONTROL1 (0xE1)
#define ID804_OTP_CTRL1_OTP_PG2_POS         (11U)
#define ID804_OTP_CTRL1_OTP_PG2_MSK         (0x001U << ID804_OTP_CTRL1_OTP_PG2_POS)
#define ID804_OTP_CTRL1_OTP_PG1_POS         (10U)
#define ID804_OTP_CTRL1_OTP_PG1_MSK         (0x001U << ID804_OTP_CTRL1_OTP_PG1_POS)
#define ID804_OTP_CTRL1_T_ANA_E_POS         (9U)
#define ID804_OTP_CTRL1_T_ANA_E_MSK         (0x001U << ID804_OTP_CTRL1_T_ANA_E_POS)
#define ID804_OTP_CTRL1_OTP_RD_POS          (8U)
#define ID804_OTP_CTRL1_OTP_RD_MSK          (0x001U << ID804_OTP_CTRL1_OTP_RD_POS)
#define ID804_OTP_CTRL1_OTP_WSEL_POS        (4U)
#define ID804_OTP_CTRL1_OTP_WSEL_MSK        (0x00FU << ID804_OTP_CTRL1_OTP_WSEL_POS)
#define ID804_OTP_CTRL1_TMUX_SEL_POS        (0U)
#define ID804_OTP_CTRL1_TMUX_SEL_MSK        (0x00FU << ID804_OTP_CTRL1_TMUX_SEL_POS)

// OTP_CONTROL2 (0xE2)
#define ID804_OTP_CTRL2_ACC_CYCLE_POS       (0U)
#define ID804_OTP_CTRL2_ACC_CYCLE_MSK       (0xFFFU << ID804_OTP_CTRL2_ACC_CYCLE_POS)

// OTP_CONTROL3 (0xE3)
#define ID804_OTP_CTRL3_PROTECT_POS         (0U)
#define ID804_OTP_CTRL3_PROTECT_MSK         (0xFFFU << ID804_OTP_CTRL3_PROTECT_POS)

// OTP1_MIRROR1 (0xE4)
#define ID804_MIRROR1_SIO1_TR_TX_BIAS_POS   (9U)
#define ID804_MIRROR1_SIO1_TR_TX_BIAS_MSK   (0x007U << ID804_MIRROR1_SIO1_TR_TX_BIAS_POS)
#define ID804_MIRROR1_SIO2_TR_TX_BIAS_POS   (6U)
#define ID804_MIRROR1_SIO2_TR_TX_BIAS_MSK   (0x007U << ID804_MIRROR1_SIO2_TR_TX_BIAS_POS)
#define ID804_MIRROR1_V_TRIM_BGR_POS        (0U)
#define ID804_MIRROR1_V_TRIM_BGR_MSK        (0x03FU << ID804_MIRROR1_V_TRIM_BGR_POS)

// OTP1_MIRROR2 (0xE5)
#define ID804_MIRROR2_SIO1_TR_RES_POS       (10U)
#define ID804_MIRROR2_SIO1_TR_RES_MSK       (0x003U << ID804_MIRROR2_SIO1_TR_RES_POS)
#define ID804_MIRROR2_SIO2_TR_RES_POS       (8U)
#define ID804_MIRROR2_SIO2_TR_RES_MSK       (0x003U << ID804_MIRROR2_SIO2_TR_RES_POS)
#define ID804_MIRROR2_VLDO_CTRL_POS         (5U)
#define ID804_MIRROR2_VLDO_CTRL_MSK         (0x007U << ID804_MIRROR2_VLDO_CTRL_POS)
#define ID804_MIRROR2_I_TRIM_BGR_POS        (0U)
#define ID804_MIRROR2_I_TRIM_BGR_MSK        (0x01FU << ID804_MIRROR2_I_TRIM_BGR_POS)

// OTP1_MIRROR3 (0xE6)
#define ID804_MIRROR3_TEMP_TRIM_BGR_POS     (4U)
#define ID804_MIRROR3_TEMP_TRIM_BGR_MSK     (0x03FU << ID804_MIRROR3_TEMP_TRIM_BGR_POS)
#define ID804_MIRROR3_BGR_R1_TRIM_POS       (0U)
#define ID804_MIRROR3_BGR_R1_TRIM_MSK       (0x00FU << ID804_MIRROR3_BGR_R1_TRIM_POS)

// OTP1_MIRROR4 (0xE7)
#define ID804_MIRROR4_OSC_CTL_POS           (4U)
#define ID804_MIRROR4_OSC_CTL_MSK           (0x0FFU << ID804_MIRROR4_OSC_CTL_POS)
#define ID804_MIRROR4_BGR_R2_TRIM_POS       (0U)
#define ID804_MIRROR4_BGR_R2_TRIM_MSK       (0x00FU << ID804_MIRROR4_BGR_R2_TRIM_POS)

// OTP1_MIRROR7 (0xEA)
#define ID804_MIRROR7_ADC_OFFSET_POS        (0U)
#define ID804_MIRROR7_ADC_OFFSET_MSK        (0x1FFU << ID804_MIRROR7_ADC_OFFSET_POS)

// OTP1_MIRROR8 (0xEB)
#define ID804_MIRROR8_ADC_GAIN_POS          (0U)
#define ID804_MIRROR8_ADC_GAIN_MSK          (0x01FU << ID804_MIRROR8_ADC_GAIN_POS)

// OTP1_MIRROR9 / 10 / 11 (0xEC, 0xED, 0xEE)
#define ID804_MIRROR_ITRIM_POS              (0U)
#define ID804_MIRROR_ITRIM_MSK              (0x0FFU << ID804_MIRROR_ITRIM_POS)

// OTP1_MIRROR12 (0xEF)
#define ID804_MIRROR12_TC_BASE_POS          (0U)
#define ID804_MIRROR12_TC_BASE_MSK          (0x3FFU << ID804_MIRROR12_TC_BASE_POS)

// OTP1_MIRROR13 (0xF0)
#define ID804_MIRROR13_TC_OFFSET_POS        (0U)
#define ID804_MIRROR13_TC_OFFSET_MSK        (0x3FFU << ID804_MIRROR13_TC_OFFSET_POS)

// OTP2_MIRROR14 / 15 / 16 (0xF1, 0xF2, 0xF3)
#define ID804_MIRROR_PWM_MAX_VAL_POS        (0U)
#define ID804_MIRROR_PWM_MAX_VAL_MSK        (0xFFFU << ID804_MIRROR_PWM_MAX_VAL_POS)

// OTP2_MIRROR17 (0xF4)
#define ID804_MIRROR17_OTP_CRC_POS          (0U)
#define ID804_MIRROR17_OTP_CRC_MSK          (0x0FFU << ID804_MIRROR17_OTP_CRC_POS)
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __ID804_METADATA_H__ */
