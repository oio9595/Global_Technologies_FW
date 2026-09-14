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
#define ID804_STATUS1_IC_STATE_POS          (4U)
#define ID804_STATUS1_IC_STATE_MSK          (0x007U << ID804_STATUS1_IC_STATE_POS)
#define ID804_STATUS1_SIO2_POS              (2U)
#define ID804_STATUS1_SIO2_MSK              (0x003U << ID804_STATUS1_SIO2_POS)
#define ID804_STATUS1_SIO1_POS              (0U)
#define ID804_STATUS1_SIO1_MSK              (0x003U << ID804_STATUS1_SIO1_POS)

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

#define ID804_TEMP_TEMPERATURE_POS          (0U)
#define ID804_TEMP_TEMPERATURE_MSK          (0x3FFU << ID804_TEMP_TEMPERATURE_POS)

#define ID804_VEXT_TM_VEXT_TM_POS           (0U)
#define ID804_VEXT_TM_VEXT_TM_MSK           (0x3FFU << ID804_VEXT_TM_VEXT_TM_POS)

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

#define ID804_TEMPTH_THRESHOLD_POS          (0U)
#define ID804_TEMPTH_THRESHOLD_MSK          (0x3FFU << ID804_TEMPTH_THRESHOLD_POS)

#define ID804_TEMPHYS_HYSTERESIS_POS        (0U)
#define ID804_TEMPHYS_HYSTERESIS_MSK        (0x3FFU << ID804_TEMPHYS_HYSTERESIS_POS)

#define ID804_CAL_PWM_VAL_POS               (0U)
#define ID804_CAL_PWM_VAL_MSK               (0xFFFU << ID804_CAL_PWM_VAL_POS)

#define ID804_CURR_MAX_R_POS                (8U)
#define ID804_CURR_MAX_R_MSK                (0x00FU << ID804_CURR_MAX_R_POS)
#define ID804_CURR_MAX_G_POS                (4U)
#define ID804_CURR_MAX_G_MSK                (0x00FU << ID804_CURR_MAX_G_POS)
#define ID804_CURR_MAX_B_POS                (0U)
#define ID804_CURR_MAX_B_MSK                (0x00FU << ID804_CURR_MAX_B_POS)

#define ID804_TEMP_LUT_TC_POS               (0U)
#define ID804_TEMP_LUT_TC_MSK               (0x3FFU << ID804_TEMP_LUT_TC_POS)

#define ID804_TIMEOUT_VAL_POS               (0U)
#define ID804_TIMEOUT_VAL_MSK               (0xFFFU << ID804_TIMEOUT_VAL_POS)

#define ID804_MCAST_1_ADDR_POS              (0U)
#define ID804_MCAST_1_ADDR_MSK              (0x00FU << ID804_MCAST_1_ADDR_POS)

#define ID804_MCAST_2_ADDR_POS              (0U)
#define ID804_MCAST_2_ADDR_MSK              (0xFFFU << ID804_MCAST_2_ADDR_POS)

#define ID804_RGB_1_R_DATA_POS              (4U)
#define ID804_RGB_1_R_DATA_MSK              (0x0FFU << ID804_RGB_1_R_DATA_POS)
#define ID804_RGB_1_G_DATA_POS              (0U)
#define ID804_RGB_1_G_DATA_MSK              (0x00FU << ID804_RGB_1_G_DATA_POS)

#define ID804_RGB_2_G_DATA_POS              (8U)
#define ID804_RGB_2_G_DATA_MSK              (0x00FU << ID804_RGB_2_G_DATA_POS)
#define ID804_RGB_2_B_DATA_POS              (0U)
#define ID804_RGB_2_B_DATA_MSK              (0x0FFU << ID804_RGB_2_B_DATA_POS)

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

#define ID804_OTP_CTRL2_ACC_CYCLE_POS       (0U)
#define ID804_OTP_CTRL2_ACC_CYCLE_MSK       (0xFFFU << ID804_OTP_CTRL2_ACC_CYCLE_POS)

#define ID804_OTP_CTRL3_PROTECT_POS         (0U)
#define ID804_OTP_CTRL3_PROTECT_MSK         (0xFFFU << ID804_OTP_CTRL3_PROTECT_POS)

#define ID804_MIRROR1_SIO1_TR_TX_BIAS_POS   (9U)
#define ID804_MIRROR1_SIO1_TR_TX_BIAS_MSK   (0x007U << ID804_MIRROR1_SIO1_TR_TX_BIAS_POS)
#define ID804_MIRROR1_SIO2_TR_TX_BIAS_POS   (6U)
#define ID804_MIRROR1_SIO2_TR_TX_BIAS_MSK   (0x007U << ID804_MIRROR1_SIO2_TR_TX_BIAS_POS)
#define ID804_MIRROR1_V_TRIM_BGR_POS        (0U)
#define ID804_MIRROR1_V_TRIM_BGR_MSK        (0x03FU << ID804_MIRROR1_V_TRIM_BGR_POS)

#define ID804_MIRROR2_SIO1_TR_RES_POS       (10U)
#define ID804_MIRROR2_SIO1_TR_RES_MSK       (0x003U << ID804_MIRROR2_SIO1_TR_RES_POS)
#define ID804_MIRROR2_SIO2_TR_RES_POS       (8U)
#define ID804_MIRROR2_SIO2_TR_RES_MSK       (0x003U << ID804_MIRROR2_SIO2_TR_RES_POS)
#define ID804_MIRROR2_VLDO_CTRL_POS         (5U)
#define ID804_MIRROR2_VLDO_CTRL_MSK         (0x007U << ID804_MIRROR2_VLDO_CTRL_POS)
#define ID804_MIRROR2_I_TRIM_BGR_POS        (0U)
#define ID804_MIRROR2_I_TRIM_BGR_MSK        (0x01FU << ID804_MIRROR2_I_TRIM_BGR_POS)

#define ID804_MIRROR3_TEMP_TRIM_BGR_POS     (4U)
#define ID804_MIRROR3_TEMP_TRIM_BGR_MSK     (0x03FU << ID804_MIRROR3_TEMP_TRIM_BGR_POS)
#define ID804_MIRROR3_BGR_R1_TRIM_POS       (0U)
#define ID804_MIRROR3_BGR_R1_TRIM_MSK       (0x00FU << ID804_MIRROR3_BGR_R1_TRIM_POS)

#define ID804_MIRROR4_OSC_CTL_POS           (4U)
#define ID804_MIRROR4_OSC_CTL_MSK           (0x0FFU << ID804_MIRROR4_OSC_CTL_POS)
#define ID804_MIRROR4_BGR_R2_TRIM_POS       (0U)
#define ID804_MIRROR4_BGR_R2_TRIM_MSK       (0x00FU << ID804_MIRROR4_BGR_R2_TRIM_POS)

#define ID804_MIRROR7_ADC_OFFSET_POS        (0U)
#define ID804_MIRROR7_ADC_OFFSET_MSK        (0x1FFU << ID804_MIRROR7_ADC_OFFSET_POS)

#define ID804_MIRROR8_ADC_GAIN_POS          (0U)
#define ID804_MIRROR8_ADC_GAIN_MSK          (0x01FU << ID804_MIRROR8_ADC_GAIN_POS)

#define ID804_MIRROR_ITRIM_POS              (0U)
#define ID804_MIRROR_ITRIM_MSK              (0x0FFU << ID804_MIRROR_ITRIM_POS)

#define ID804_MIRROR12_TC_BASE_POS          (0U)
#define ID804_MIRROR12_TC_BASE_MSK          (0x3FFU << ID804_MIRROR12_TC_BASE_POS)

#define ID804_MIRROR13_TC_OFFSET_POS        (0U)
#define ID804_MIRROR13_TC_OFFSET_MSK        (0x3FFU << ID804_MIRROR13_TC_OFFSET_POS)

#define ID804_MIRROR_PWM_MAX_VAL_POS        (0U)
#define ID804_MIRROR_PWM_MAX_VAL_MSK        (0xFFFU << ID804_MIRROR_PWM_MAX_VAL_POS)

#define ID804_MIRROR17_OTP_CRC_POS          (0U)
#define ID804_MIRROR17_OTP_CRC_MSK          (0x0FFU << ID804_MIRROR17_OTP_CRC_POS)
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum tag_ID804_CMD_LIST
{
    ID804_CMD_RESET                 = 0xB1U, // 0xB1U
    ID804_CMD_INITBIDIR             = 0xB2U, // 0xB2U
    ID804_CMD_CLRERROR              = 0xB3U, // 0xB3U
    ID804_CMD_GOSLEEP               = 0xB4U, // 0xB4U
    ID804_CMD_GOACTIVE              = 0xB8U, // 0xB8U
    ID804_CMD_GODEEPSLEEP           = 0xBCU, // 0xBCU

    ID804_CMD_SET_START             = 0x86U, // 0x86U
    ID804_CMD_SET_SETUP1            = 0x86U, // 0x86U
    ID804_CMD_SET_SETUP2            = 0x87U, // 0x87U
    ID804_CMD_SET_MCAST             = 0x88U, // 0x88U
    ID804_CMD_SET_TEMPTH            = 0x8AU, // 0x8AU
    ID804_CMD_SET_TEMPHYS           = 0x8BU, // 0x8BU
    ID804_CMD_SET_CURR_MAX_LVL      = 0x8FU, // 0x8FU
    ID804_CMD_SET_TEMP_LUT_TC1      = 0x90U, // 0x90U
    ID804_CMD_SET_TEMP_LUT_TC2      = 0x91U, // 0x91U
    ID804_CMD_SET_TEMP_LUT_TC3      = 0x92U, // 0x92U
    ID804_CMD_SET_TEMP_LUT_TC4      = 0x93U, // 0x93U
    ID804_CMD_SET_TEMP_LUT_TC5      = 0x94U, // 0x94U
    ID804_CMD_SET_TEMP_LUT_TC6      = 0x95U, // 0x95U
    ID804_CMD_SET_TEMP_LUT_TC7      = 0x96U, // 0x96U
    ID804_CMD_SET_TEMP_LUT_TC8      = 0x97U, // 0x97U
    ID804_CMD_SET_TEMP_LUT_TC9      = 0x98U, // 0x98U
    ID804_CMD_SET_TEMP_LUT_TC10     = 0x99U, // 0x99U
    ID804_CMD_SET_TIMEOUT           = 0x9AU, // 0x9AU
    ID804_CMD_SET_RGB               = 0xA0U, // 0xA0U

    ID804_CMD_READ_START            = 0x41U, // 0x41U
    ID804_CMD_READ_STATUS1          = 0x41U, // 0x41U
    ID804_CMD_READ_STATUS2          = 0x42U, // 0x42U
    ID804_CMD_READ_TEMP             = 0x43U, // 0x43U
    ID804_CMD_READ_TEMPST           = 0x44U, // 0x44U
    ID804_CMD_READ_VEXT_TM          = 0x45U, // 0x45U
    ID804_CMD_READ_SETUP1           = 0x46U, // 0x46U
    ID804_CMD_READ_SETUP2           = 0x47U, // 0x47U
    ID804_CMD_READ_MCAST            = 0x48U, // 0x48U
    ID804_CMD_READ_TEMPTH           = 0x4AU, // 0x4AU
    ID804_CMD_READ_TEMPHYS          = 0x4BU, // 0x4BU
    ID804_CMD_READ_PWM_RED_VAL      = 0x4CU, // 0x4CU
    ID804_CMD_READ_PWM_GREEN_VAL    = 0x4DU, // 0x4DU
    ID804_CMD_READ_PWM_BLUE_VAL     = 0x4EU, // 0x4EU
    ID804_CMD_READ_CURR_MAX_LVL     = 0x4FU, // 0x4FU
    ID804_CMD_READ_TEMP_LUT_TC1     = 0x50U, // 0x50U
    ID804_CMD_READ_TEMP_LUT_TC2     = 0x51U, // 0x51U
    ID804_CMD_READ_TEMP_LUT_TC3     = 0x52U, // 0x52U
    ID804_CMD_READ_TEMP_LUT_TC4     = 0x53U, // 0x53U
    ID804_CMD_READ_TEMP_LUT_TC5     = 0x54U, // 0x54U
    ID804_CMD_READ_TEMP_LUT_TC6     = 0x55U, // 0x55U
    ID804_CMD_READ_TEMP_LUT_TC7     = 0x56U, // 0x56U
    ID804_CMD_READ_TEMP_LUT_TC8     = 0x57U, // 0x57U
    ID804_CMD_READ_TEMP_LUT_TC9     = 0x58U, // 0x58U
    ID804_CMD_READ_TEMP_LUT_TC10    = 0x59U, // 0x59U
    ID804_CMD_READ_TIMEOUT          = 0x5AU, // 0x5AU
    ID804_CMD_READ_RGB              = 0x60U, // 0x60U
    ID804_CMD_MAX
} id804_cmd_list_t;

typedef enum tag_ID804_REG_ADDR
{
    ID804_ADDR_STATUS1              = 0x01U, // 0x01U
    ID804_ADDR_STATUS2              = 0x02U, // 0x02U
    ID804_ADDR_TEMP                 = 0x03U, // 0x03U
    ID804_ADDR_VEXT_TM              = 0x05U, // 0x05U
    ID804_ADDR_SETUP1               = 0x06U, // 0x06U
    ID804_ADDR_SETUP2               = 0x07U, // 0x07U
    ID804_ADDR_MCAST_1              = 0x08U, // 0x08U
    ID804_ADDR_MCAST_2              = 0x09U, // 0x09U
    ID804_ADDR_TEMPTH               = 0x0AU, // 0x0AU
    ID804_ADDR_TEMPHYS              = 0x0BU, // 0x0BU
    ID804_ADDR_CAL_PWM_RED          = 0x0CU, // 0x0CU
    ID804_ADDR_CAL_PWM_GREEN        = 0x0DU, // 0x0DU
    ID804_ADDR_CAL_PWM_BLUE         = 0x0EU, // 0x0EU
    ID804_ADDR_CURR_MAX_LVL         = 0x0FU, // 0x0FU
    ID804_ADDR_TEMP_LUT_TC1         = 0x10U, // 0x10U
    ID804_ADDR_TEMP_LUT_TC2         = 0x11U, // 0x11U
    ID804_ADDR_TEMP_LUT_TC3         = 0x12U, // 0x12U
    ID804_ADDR_TEMP_LUT_TC4         = 0x13U, // 0x13U
    ID804_ADDR_TEMP_LUT_TC5         = 0x14U, // 0x14U
    ID804_ADDR_TEMP_LUT_TC6         = 0x15U, // 0x15U
    ID804_ADDR_TEMP_LUT_TC7         = 0x16U, // 0x16U
    ID804_ADDR_TEMP_LUT_TC8         = 0x17U, // 0x17U
    ID804_ADDR_TEMP_LUT_TC9         = 0x18U, // 0x18U
    ID804_ADDR_TEMP_LUT_TC10        = 0x19U, // 0x19U
    ID804_ADDR_TIMEOUT              = 0x1AU, // 0x1AU
    ID804_ADDR_RGB_1                = 0x20U, // 0x20U
    ID804_ADDR_RGB_2                = 0x21U, // 0x21U
    ID804_ADDR_MAX
} id804_reg_addr_t;

typedef enum tag_ID804_OTP_ADDR
{
    ID804_ADDR_OTP_START            = 0xE0U, // 0xE0U
    ID804_ADDR_TRIM_CONTROL         = 0xE0U, // 0xE0U
    ID804_ADDR_OTP_CONTROL1         = 0xE1U, // 0xE1U
    ID804_ADDR_OTP_CONTROL2         = 0xE2U, // 0xE2U
    ID804_ADDR_OTP_CONTROL3         = 0xE3U, // 0xE3U
    ID804_ADDR_OTP1_MIRROR1         = 0xE4U, // 0xE4U
    ID804_ADDR_OTP1_MIRROR2         = 0xE5U, // 0xE5U
    ID804_ADDR_OTP1_MIRROR3         = 0xE6U, // 0xE6U
    ID804_ADDR_OTP1_MIRROR4         = 0xE7U, // 0xE7U
    ID804_ADDR_OTP1_MIRROR5         = 0xE8U, // 0xE8U
    ID804_ADDR_OTP1_MIRROR6         = 0xE9U, // 0xE9U
    ID804_ADDR_OTP1_MIRROR7         = 0xEAU, // 0xEAU
    ID804_ADDR_OTP1_MIRROR8         = 0xEBU, // 0xEBU
    ID804_ADDR_OTP1_MIRROR9         = 0xECU, // 0xECU
    ID804_ADDR_OTP1_MIRROR10        = 0xEDU, // 0xEDU
    ID804_ADDR_OTP1_MIRROR11        = 0xEEU, // 0xEEU
    ID804_ADDR_OTP1_MIRROR12        = 0xEFU, // 0xEFU
    ID804_ADDR_OTP1_MIRROR13        = 0xF0U, // 0xF0U
    ID804_ADDR_OTP2_MIRROR14        = 0xF1U, // 0xF1U
    ID804_ADDR_OTP2_MIRROR15        = 0xF2U, // 0xF2U
    ID804_ADDR_OTP2_MIRROR16        = 0xF3U, // 0xF3U
    ID804_ADDR_OTP2_MIRROR17        = 0xF4U, // 0xF4U
    ID804_ADDR_OTP2_MIRROR18        = 0xF5U, // 0xF5U
    ID804_ADDR_OTP2_MIRROR19        = 0xF6U, // 0xF6U
    ID804_OTP_ADDR_MAX
} id804_otp_addr_t;

typedef enum tag_ID804_COMM_RESULT
{
    ID804_COMM_NONE         = 0x00U,
    ID804_COMM_WRITE_OK     = 0x01U,
    ID804_COMM_READ_OK      = 0x02U,
    ID804_COMM_ERR_DEV_ADDR = 0x03U,
    ID804_COMM_ERR_CMD      = 0x04U,
    ID804_COMM_ERR_ADDR     = 0x05U,
    ID804_COMM_ERR_I2C      = 0x06U,
    ID804_COMM_ERR_SPI      = 0x07U,
} id804_comm_result_t;
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
