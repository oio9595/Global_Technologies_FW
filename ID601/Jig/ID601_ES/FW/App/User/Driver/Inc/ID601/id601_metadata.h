/* USER CODE BEGIN Header */
/*
    * File:   id601_metadata.h
    * Author: GT
    *
    * Created on 2026. 09. 22.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ID601_METADATA_H__
#define __ID601_METADATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define ID601_INTERNAL_MCLK     (50000000UL)    /* 50.0MHz */
#define ID601_MODEL_NAME        "ID601 ES0"

/* General register 0x00: RESET_ID */
#define ID601_RESET_ID_RST_Pos                             (11U)
#define ID601_RESET_ID_RST_Msk                             (0x800U)
#define ID601_RESET_ID_VS_RST_Pos                          (10U)
#define ID601_RESET_ID_VS_RST_Msk                          (0x400U)
#define ID601_RESET_ID_E_RST_Pos                           (9U)
#define ID601_RESET_ID_E_RST_Msk                           (0x200U)
#define ID601_RESET_ID_LKG_E_Pos                           (8U)
#define ID601_RESET_ID_LKG_E_Msk                           (0x100U)
#define ID601_RESET_ID_ID_Pos                              (0U)
#define ID601_RESET_ID_ID_Msk                              (0x01FU)

/* General register 0x01: LD_CONTROL */
#define ID601_LD_CONTROL_SV_NO_Pos                         (6U)
#define ID601_LD_CONTROL_SV_NO_Msk                         (0xFC0U)
#define ID601_LD_CONTROL_LD_TYPE_Pos                       (5U)
#define ID601_LD_CONTROL_LD_TYPE_Msk                       (0x020U)
#define ID601_LD_CONTROL_DELAY_CH_EN_Pos                   (4U)
#define ID601_LD_CONTROL_DELAY_CH_EN_Msk                   (0x010U)
#define ID601_LD_CONTROL_SYNCMODE_Pos                      (3U)
#define ID601_LD_CONTROL_SYNCMODE_Msk                      (0x008U)
#define ID601_LD_CONTROL_PWM_RES_Pos                       (2U)
#define ID601_LD_CONTROL_PWM_RES_Msk                       (0x004U)
#define ID601_LD_CONTROL_LD_DIR_Pos                        (1U)
#define ID601_LD_CONTROL_LD_DIR_Msk                        (0x002U)
#define ID601_LD_CONTROL_LD_MODE_Pos                       (0U)
#define ID601_LD_CONTROL_LD_MODE_Msk                       (0x001U)

/* General register 0x02: LD_SIZE */
#define ID601_LD_SIZE_LD_SIZE_Pos                          (0U)
#define ID601_LD_SIZE_LD_SIZE_Msk                          (0x03FU)

/* General register 0x03: PWMCLK_DIV1_2 */
#define ID601_PWMCLK_DIV1_2_FPWM_DIV2_Pos                  (8U)
#define ID601_PWMCLK_DIV1_2_FPWM_DIV2_Msk                  (0xF00U)
#define ID601_PWMCLK_DIV1_2_FPWM_DIV1_Pos                  (0U)
#define ID601_PWMCLK_DIV1_2_FPWM_DIV1_Msk                  (0x0FFU)

/* General register 0x04: PWMCLK_DIV2_3 */
#define ID601_PWMCLK_DIV2_3_FPWM_DIV3_Pos                  (4U)
#define ID601_PWMCLK_DIV2_3_FPWM_DIV3_Msk                  (0xFF0U)
#define ID601_PWMCLK_DIV2_3_FPWM_DIV2_Pos                  (0U)
#define ID601_PWMCLK_DIV2_3_FPWM_DIV2_Msk                  (0x00FU)

/* General register 0x05: CHANNEL_ENABLE */
#define ID601_CHANNEL_ENABLE_CH12_EN_Pos                   (11U)
#define ID601_CHANNEL_ENABLE_CH12_EN_Msk                   (0x800U)
#define ID601_CHANNEL_ENABLE_CH11_EN_Pos                   (10U)
#define ID601_CHANNEL_ENABLE_CH11_EN_Msk                   (0x400U)
#define ID601_CHANNEL_ENABLE_CH10_EN_Pos                   (9U)
#define ID601_CHANNEL_ENABLE_CH10_EN_Msk                   (0x200U)
#define ID601_CHANNEL_ENABLE_CH9_EN_Pos                    (8U)
#define ID601_CHANNEL_ENABLE_CH9_EN_Msk                    (0x100U)
#define ID601_CHANNEL_ENABLE_CH8_EN_Pos                    (7U)
#define ID601_CHANNEL_ENABLE_CH8_EN_Msk                    (0x080U)
#define ID601_CHANNEL_ENABLE_CH7_EN_Pos                    (6U)
#define ID601_CHANNEL_ENABLE_CH7_EN_Msk                    (0x040U)
#define ID601_CHANNEL_ENABLE_CH6_EN_Pos                    (5U)
#define ID601_CHANNEL_ENABLE_CH6_EN_Msk                    (0x020U)
#define ID601_CHANNEL_ENABLE_CH5_EN_Pos                    (4U)
#define ID601_CHANNEL_ENABLE_CH5_EN_Msk                    (0x010U)
#define ID601_CHANNEL_ENABLE_CH4_EN_Pos                    (3U)
#define ID601_CHANNEL_ENABLE_CH4_EN_Msk                    (0x008U)
#define ID601_CHANNEL_ENABLE_CH3_EN_Pos                    (2U)
#define ID601_CHANNEL_ENABLE_CH3_EN_Msk                    (0x004U)
#define ID601_CHANNEL_ENABLE_CH2_EN_Pos                    (1U)
#define ID601_CHANNEL_ENABLE_CH2_EN_Msk                    (0x002U)
#define ID601_CHANNEL_ENABLE_CH1_EN_Pos                    (0U)
#define ID601_CHANNEL_ENABLE_CH1_EN_Msk                    (0x001U)

/* General register 0x06: FAULT_CONTROL0 */
#define ID601_FAULT_CONTROL_FT_MODE_Pos                    (9U)
#define ID601_FAULT_CONTROL_FT_MODE_Msk                    (0x600U)
#define ID601_FAULT_CONTROL_FB_MODE_Pos                    (7U)
#define ID601_FAULT_CONTROL_FB_MODE_Msk                    (0x180U)
#define ID601_FAULT_CONTROL_O_FB_E_Pos                     (6U)
#define ID601_FAULT_CONTROL_O_FB_E_Msk                     (0x040U)
#define ID601_FAULT_CONTROL_T_DET_E_Pos                    (5U)
#define ID601_FAULT_CONTROL_T_DET_E_Msk                    (0x020U)
#define ID601_FAULT_CONTROL_S_DET_E_Pos                    (4U)
#define ID601_FAULT_CONTROL_S_DET_E_Msk                    (0x010U)
#define ID601_FAULT_CONTROL_O_DET_E_Pos                    (3U)
#define ID601_FAULT_CONTROL_O_DET_E_Msk                    (0x008U)
#define ID601_FAULT_CONTROL_T_OFF_E_Pos                    (2U)
#define ID601_FAULT_CONTROL_T_OFF_E_Msk                    (0x004U)
#define ID601_FAULT_CONTROL_S_OFF_E_Pos                    (1U)
#define ID601_FAULT_CONTROL_S_OFF_E_Msk                    (0x002U)
#define ID601_FAULT_CONTROL_O_OFF_E_Pos                    (0U)
#define ID601_FAULT_CONTROL_O_OFF_E_Msk                    (0x001U)

/* General register 0x07: FB_LEVEL */
#define ID601_FB_LEVEL_FB3_LEVEL_Pos                       (6U)
#define ID601_FB_LEVEL_FB3_LEVEL_Msk                       (0x1C0U)
#define ID601_FB_LEVEL_FB2_LEVEL_Pos                       (3U)
#define ID601_FB_LEVEL_FB2_LEVEL_Msk                       (0x038U)
#define ID601_FB_LEVEL_FB1_LEVEL_Pos                       (0U)
#define ID601_FB_LEVEL_FB1_LEVEL_Msk                       (0x007U)

/* General register 0x08: SHORT_LEVEL */
#define ID601_SHORT_LEVEL_SHORT3_LEVEL_Pos                 (6U)
#define ID601_SHORT_LEVEL_SHORT3_LEVEL_Msk                 (0x1C0U)
#define ID601_SHORT_LEVEL_SHORT2_LEVEL_Pos                 (3U)
#define ID601_SHORT_LEVEL_SHORT2_LEVEL_Msk                 (0x038U)
#define ID601_SHORT_LEVEL_SHORT1_LEVEL_Pos                 (0U)
#define ID601_SHORT_LEVEL_SHORT1_LEVEL_Msk                 (0x007U)

/* General register 0x09: FAULT_STATUS0 */
#define ID601_FAULT_STATUS0_SHORT_Pos                      (4U)
#define ID601_FAULT_STATUS0_SHORT_Msk                      (0x010U)
#define ID601_FAULT_STATUS0_OPEN_Pos                       (3U)
#define ID601_FAULT_STATUS0_OPEN_Msk                       (0x008U)
#define ID601_FAULT_STATUS0_FB3_Pos                        (2U)
#define ID601_FAULT_STATUS0_FB3_Msk                        (0x004U)
#define ID601_FAULT_STATUS0_FB2_Pos                        (1U)
#define ID601_FAULT_STATUS0_FB2_Msk                        (0x002U)
#define ID601_FAULT_STATUS0_FB1_Pos                        (0U)
#define ID601_FAULT_STATUS0_FB1_Msk                        (0x001U)

/* General register 0x0A: FAULT_STATUS1 */
#define ID601_FAULT_STATUS1_RESP_ERR_Pos                   (11U)
#define ID601_FAULT_STATUS1_RESP_ERR_Msk                   (0x800U)
#define ID601_FAULT_STATUS1_CMD_ERR_Pos                    (10U)
#define ID601_FAULT_STATUS1_CMD_ERR_Msk                    (0x400U)
#define ID601_FAULT_STATUS1_WD_TIMEOUT_Pos                 (9U)
#define ID601_FAULT_STATUS1_WD_TIMEOUT_Msk                 (0x200U)
#define ID601_FAULT_STATUS1_VS_MISS_Pos                    (8U)
#define ID601_FAULT_STATUS1_VS_MISS_Msk                    (0x100U)
#define ID601_FAULT_STATUS1_CH_CTRL_MISMATCH_Pos           (7U)
#define ID601_FAULT_STATUS1_CH_CTRL_MISMATCH_Msk           (0x080U)
#define ID601_FAULT_STATUS1_VREF_OV_Pos                    (6U)
#define ID601_FAULT_STATUS1_VREF_OV_Msk                    (0x040U)
#define ID601_FAULT_STATUS1_VREF_UV_Pos                    (5U)
#define ID601_FAULT_STATUS1_VREF_UV_Msk                    (0x020U)
#define ID601_FAULT_STATUS1_OSC_LDO_OV_Pos                 (4U)
#define ID601_FAULT_STATUS1_OSC_LDO_OV_Msk                 (0x010U)
#define ID601_FAULT_STATUS1_OSC_LDO_UV_Pos                 (3U)
#define ID601_FAULT_STATUS1_OSC_LDO_UV_Msk                 (0x008U)
#define ID601_FAULT_STATUS1_LDO_OV_Pos                     (2U)
#define ID601_FAULT_STATUS1_LDO_OV_Msk                     (0x004U)
#define ID601_FAULT_STATUS1_LDO_UV_Pos                     (1U)
#define ID601_FAULT_STATUS1_LDO_UV_Msk                     (0x002U)
#define ID601_FAULT_STATUS1_THERMAL_Pos                    (0U)
#define ID601_FAULT_STATUS1_THERMAL_Msk                    (0x001U)

/* General register 0x0B: FAULT_STATUS2 */
#define ID601_FAULT_STATUS2_OPEN_CH12_Pos                  (11U)
#define ID601_FAULT_STATUS2_OPEN_CH12_Msk                  (0x800U)
#define ID601_FAULT_STATUS2_OPEN_CH11_Pos                  (10U)
#define ID601_FAULT_STATUS2_OPEN_CH11_Msk                  (0x400U)
#define ID601_FAULT_STATUS2_OPEN_CH10_Pos                  (9U)
#define ID601_FAULT_STATUS2_OPEN_CH10_Msk                  (0x200U)
#define ID601_FAULT_STATUS2_OPEN_CH9_Pos                   (8U)
#define ID601_FAULT_STATUS2_OPEN_CH9_Msk                   (0x100U)
#define ID601_FAULT_STATUS2_OPEN_CH8_Pos                   (7U)
#define ID601_FAULT_STATUS2_OPEN_CH8_Msk                   (0x080U)
#define ID601_FAULT_STATUS2_OPEN_CH7_Pos                   (6U)
#define ID601_FAULT_STATUS2_OPEN_CH7_Msk                   (0x040U)
#define ID601_FAULT_STATUS2_OPEN_CH6_Pos                   (5U)
#define ID601_FAULT_STATUS2_OPEN_CH6_Msk                   (0x020U)
#define ID601_FAULT_STATUS2_OPEN_CH5_Pos                   (4U)
#define ID601_FAULT_STATUS2_OPEN_CH5_Msk                   (0x010U)
#define ID601_FAULT_STATUS2_OPEN_CH4_Pos                   (3U)
#define ID601_FAULT_STATUS2_OPEN_CH4_Msk                   (0x008U)
#define ID601_FAULT_STATUS2_OPEN_CH3_Pos                   (2U)
#define ID601_FAULT_STATUS2_OPEN_CH3_Msk                   (0x004U)
#define ID601_FAULT_STATUS2_OPEN_CH2_Pos                   (1U)
#define ID601_FAULT_STATUS2_OPEN_CH2_Msk                   (0x002U)
#define ID601_FAULT_STATUS2_OPEN_CH1_Pos                   (0U)
#define ID601_FAULT_STATUS2_OPEN_CH1_Msk                   (0x001U)

/* General register 0x0C: FAULT_STATUS3 */
#define ID601_FAULT_STATUS3_SHORT_CH12_Pos                 (11U)
#define ID601_FAULT_STATUS3_SHORT_CH12_Msk                 (0x800U)
#define ID601_FAULT_STATUS3_SHORT_CH11_Pos                 (10U)
#define ID601_FAULT_STATUS3_SHORT_CH11_Msk                 (0x400U)
#define ID601_FAULT_STATUS3_SHORT_CH10_Pos                 (9U)
#define ID601_FAULT_STATUS3_SHORT_CH10_Msk                 (0x200U)
#define ID601_FAULT_STATUS3_SHORT_CH9_Pos                  (8U)
#define ID601_FAULT_STATUS3_SHORT_CH9_Msk                  (0x100U)
#define ID601_FAULT_STATUS3_SHORT_CH8_Pos                  (7U)
#define ID601_FAULT_STATUS3_SHORT_CH8_Msk                  (0x080U)
#define ID601_FAULT_STATUS3_SHORT_CH7_Pos                  (6U)
#define ID601_FAULT_STATUS3_SHORT_CH7_Msk                  (0x040U)
#define ID601_FAULT_STATUS3_SHORT_CH6_Pos                  (5U)
#define ID601_FAULT_STATUS3_SHORT_CH6_Msk                  (0x020U)
#define ID601_FAULT_STATUS3_SHORT_CH5_Pos                  (4U)
#define ID601_FAULT_STATUS3_SHORT_CH5_Msk                  (0x010U)
#define ID601_FAULT_STATUS3_SHORT_CH4_Pos                  (3U)
#define ID601_FAULT_STATUS3_SHORT_CH4_Msk                  (0x008U)
#define ID601_FAULT_STATUS3_SHORT_CH3_Pos                  (2U)
#define ID601_FAULT_STATUS3_SHORT_CH3_Msk                  (0x004U)
#define ID601_FAULT_STATUS3_SHORT_CH2_Pos                  (1U)
#define ID601_FAULT_STATUS3_SHORT_CH2_Msk                  (0x002U)
#define ID601_FAULT_STATUS3_SHORT_CH1_Pos                  (0U)
#define ID601_FAULT_STATUS3_SHORT_CH1_Msk                  (0x001U)

/* General register 0x0D: MAX_CURRENT_LEVEL */
#define ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL3_Pos        (8U)
#define ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL3_Msk        (0xF00U)
#define ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL2_Pos        (4U)
#define ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL2_Msk        (0x0F0U)
#define ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL1_Pos        (0U)
#define ID601_MAX_CURRENT_LEVEL_MAX_CURR_LEVEL1_Msk        (0x00FU)

/* General register 0x0E: MAX_CURRENT_VREF1 */
#define ID601_MAX_CURRENT_VREF1_MAX_CURR_VREF1_Pos         (0U)
#define ID601_MAX_CURRENT_VREF1_MAX_CURR_VREF1_Msk         (0xFFFU)

/* General register 0x0F: MAX_CURRENT_VREF2 */
#define ID601_MAX_CURRENT_VREF2_MAX_CURR_VREF2_Pos         (0U)
#define ID601_MAX_CURRENT_VREF2_MAX_CURR_VREF2_Msk         (0xFFFU)

/* General register 0x10: MAX_CURRENT_VREF3 */
#define ID601_MAX_CURRENT_VREF3_MAX_CURR_VREF3_Pos         (0U)
#define ID601_MAX_CURRENT_VREF3_MAX_CURR_VREF3_Msk         (0xFFFU)

/* General register 0x11: CH_LD_TYPE */
#define ID601_CH_LD_TYPE_CH12_LD_TYPE_Pos                  (11U)
#define ID601_CH_LD_TYPE_CH12_LD_TYPE_Msk                  (0x800U)
#define ID601_CH_LD_TYPE_CH11_LD_TYPE_Pos                  (10U)
#define ID601_CH_LD_TYPE_CH11_LD_TYPE_Msk                  (0x400U)
#define ID601_CH_LD_TYPE_CH10_LD_TYPE_Pos                  (9U)
#define ID601_CH_LD_TYPE_CH10_LD_TYPE_Msk                  (0x200U)
#define ID601_CH_LD_TYPE_CH9_LD_TYPE_Pos                   (8U)
#define ID601_CH_LD_TYPE_CH9_LD_TYPE_Msk                   (0x100U)
#define ID601_CH_LD_TYPE_CH8_LD_TYPE_Pos                   (7U)
#define ID601_CH_LD_TYPE_CH8_LD_TYPE_Msk                   (0x080U)
#define ID601_CH_LD_TYPE_CH7_LD_TYPE_Pos                   (6U)
#define ID601_CH_LD_TYPE_CH7_LD_TYPE_Msk                   (0x040U)
#define ID601_CH_LD_TYPE_CH6_LD_TYPE_Pos                   (5U)
#define ID601_CH_LD_TYPE_CH6_LD_TYPE_Msk                   (0x020U)
#define ID601_CH_LD_TYPE_CH5_LD_TYPE_Pos                   (4U)
#define ID601_CH_LD_TYPE_CH5_LD_TYPE_Msk                   (0x010U)
#define ID601_CH_LD_TYPE_CH4_LD_TYPE_Pos                   (3U)
#define ID601_CH_LD_TYPE_CH4_LD_TYPE_Msk                   (0x008U)
#define ID601_CH_LD_TYPE_CH3_LD_TYPE_Pos                   (2U)
#define ID601_CH_LD_TYPE_CH3_LD_TYPE_Msk                   (0x004U)
#define ID601_CH_LD_TYPE_CH2_LD_TYPE_Pos                   (1U)
#define ID601_CH_LD_TYPE_CH2_LD_TYPE_Msk                   (0x002U)
#define ID601_CH_LD_TYPE_CH1_LD_TYPE_Pos                   (0U)
#define ID601_CH_LD_TYPE_CH1_LD_TYPE_Msk                   (0x001U)

/* General register 0x12: DELAY_CH1_2 */
#define ID601_DELAY_CH1_2_DELAY_CH2_Pos                    (5U)
#define ID601_DELAY_CH1_2_DELAY_CH2_Msk                    (0x3E0U)
#define ID601_DELAY_CH1_2_DELAY_CH1_Pos                    (0U)
#define ID601_DELAY_CH1_2_DELAY_CH1_Msk                    (0x01FU)

/* General register 0x13: DELAY_CH3_4 */
#define ID601_DELAY_CH3_4_DELAY_CH4_Pos                    (5U)
#define ID601_DELAY_CH3_4_DELAY_CH4_Msk                    (0x3E0U)
#define ID601_DELAY_CH3_4_DELAY_CH3_Pos                    (0U)
#define ID601_DELAY_CH3_4_DELAY_CH3_Msk                    (0x01FU)

/* General register 0x14: DELAY_CH5_6 */
#define ID601_DELAY_CH5_6_DELAY_CH6_Pos                    (5U)
#define ID601_DELAY_CH5_6_DELAY_CH6_Msk                    (0x3E0U)
#define ID601_DELAY_CH5_6_DELAY_CH5_Pos                    (0U)
#define ID601_DELAY_CH5_6_DELAY_CH5_Msk                    (0x01FU)

/* General register 0x15: DELAY_CH7_8 */
#define ID601_DELAY_CH7_8_DELAY_CH8_Pos                    (5U)
#define ID601_DELAY_CH7_8_DELAY_CH8_Msk                    (0x3E0U)
#define ID601_DELAY_CH7_8_DELAY_CH7_Pos                    (0U)
#define ID601_DELAY_CH7_8_DELAY_CH7_Msk                    (0x01FU)

/* General register 0x16: DELAY_CH9_10 */
#define ID601_DELAY_CH9_10_DELAY_CH10_Pos                  (5U)
#define ID601_DELAY_CH9_10_DELAY_CH10_Msk                  (0x3E0U)
#define ID601_DELAY_CH9_10_DELAY_CH9_Pos                   (0U)
#define ID601_DELAY_CH9_10_DELAY_CH9_Msk                   (0x01FU)

/* General register 0x17: DELAY_CH11_12 */
#define ID601_DELAY_CH11_12_DELAY_CH12_Pos                 (5U)
#define ID601_DELAY_CH11_12_DELAY_CH12_Msk                 (0x3E0U)
#define ID601_DELAY_CH11_12_DELAY_CH11_Pos                 (0U)
#define ID601_DELAY_CH11_12_DELAY_CH11_Msk                 (0x01FU)

/* General register 0x18: FAULT_CONFIGURATION */
#define ID601_FAULT_CONFIGURATION_CRC_EN_Pos               (11U)
#define ID601_FAULT_CONFIGURATION_WD_DIMM_EN_Pos           (10U)
#define ID601_FAULT_CONFIGURATION_CRC_EN_Msk               (0x800U)
#define ID601_FAULT_CONFIGURATION_WD_DIMM_EN_Msk           (0x400U)
#define ID601_FAULT_CONFIGURATION_WD_TIMECFG_Pos           (8U)
#define ID601_FAULT_CONFIGURATION_WD_TIMECFG_Msk           (0x300U)
#define ID601_FAULT_CONFIGURATION_WD_EN_Pos                (7U)
#define ID601_FAULT_CONFIGURATION_WD_EN_Msk                (0x080U)
#define ID601_FAULT_CONFIGURATION_VS_MISS_DIMM_EN_Pos      (6U)
#define ID601_FAULT_CONFIGURATION_VS_MISS_DIMM_EN_Msk      (0x040U)
#define ID601_FAULT_CONFIGURATION_VS_MISS_EN_Pos           (5U)
#define ID601_FAULT_CONFIGURATION_VS_MISS_EN_Msk           (0x020U)
#define ID601_FAULT_CONFIGURATION_CH_CTRL_MISMATCH_EN_Pos  (4U)
#define ID601_FAULT_CONFIGURATION_CH_CTRL_MISMATCH_EN_Msk  (0x010U)
#define ID601_FAULT_CONFIGURATION_VREF_OVUV_OFF_EN_Pos     (3U)
#define ID601_FAULT_CONFIGURATION_VREF_OVUV_OFF_EN_Msk     (0x008U)
#define ID601_FAULT_CONFIGURATION_VREF_OVUV_EN_Pos         (2U)
#define ID601_FAULT_CONFIGURATION_VREF_OVUV_EN_Msk         (0x004U)
#define ID601_FAULT_CONFIGURATION_OSC_LDO_OVUV_EN_Pos      (1U)
#define ID601_FAULT_CONFIGURATION_OSC_LDO_OVUV_EN_Msk      (0x002U)
#define ID601_FAULT_CONFIGURATION_LDO_OVUV_EN_Pos          (0U)
#define ID601_FAULT_CONFIGURATION_LDO_OVUV_EN_Msk          (0x001U)

/* General register 0x19: BIST */
#define ID601_BIST_BIST_START_Pos                          (11U)
#define ID601_BIST_BIST_START_Msk                          (0x800U)
#define ID601_BIST_BIST_BUSY_R_O_Pos                       (2U)
#define ID601_BIST_BIST_BUSY_R_O_Msk                       (0x004U)
#define ID601_BIST_BIST_DONE_R_O_Pos                       (1U)
#define ID601_BIST_BIST_DONE_R_O_Msk                       (0x002U)
#define ID601_BIST_BIST_ERR_R_O_Pos                        (0U)
#define ID601_BIST_BIST_ERR_R_O_Msk                        (0x001U)

/* General register 0x1A: TEST_MODE0 */
#define ID601_TEST_MODE0_DMUX_SEL_Pos                      (0U)
#define ID601_TEST_MODE0_DMUX_SEL_Msk                      (0x01FU)

/* General register 0x1B: TEST_MODE1 */
#define ID601_TEST_MODE1_TESTMODE_Pos                      (11U)
#define ID601_TEST_MODE1_TESTMODE_Msk                      (0x800U)
#define ID601_TEST_MODE1_TM_CH_CTRL_INPUT_FLIP_Pos         (0U)
#define ID601_TEST_MODE1_TM_CH_CTRL_INPUT_FLIP_Msk         (0x001U)

/* General register 0x1C: SERIAL_CLK_GEN */
#define ID601_SERIAL_CLK_GEN_SERIAL_CLK_LOW_Pos            (6U)
#define ID601_SERIAL_CLK_GEN_SERIAL_CLK_LOW_Msk            (0xFC0U)
#define ID601_SERIAL_CLK_GEN_SERIAL_CLK_HIGH_Pos           (0U)
#define ID601_SERIAL_CLK_GEN_SERIAL_CLK_HIGH_Msk           (0x03FU)

/* General register 0x1D: SERIAL_LATENCY */
#define ID601_SERIAL_LATENCY_SERIAL_LATENCY_Pos            (0U)
#define ID601_SERIAL_LATENCY_SERIAL_LATENCY_Msk            (0x1FFU)

/* General register 0x1E: V_MASK */
#define ID601_V_MASK_V_MASK_Pos                            (0U)
#define ID601_V_MASK_V_MASK_Msk                            (0x3FFU)

/* General register 0x1F: SV_MASK */
#define ID601_SV_MASK_SV_MASK_EN_Pos                       (11U)
#define ID601_SV_MASK_SV_MASK_EN_Msk                       (0x800U)
#define ID601_SV_MASK_SV_MASK_Pos                          (0U)
#define ID601_SV_MASK_SV_MASK_Msk                          (0x3FFU)

/* General register 0x20: RSTCNT */
#define ID601_RSTCNT_RSTCNT_Pos                            (0U)
#define ID601_RSTCNT_RSTCNT_Msk                            (0x3FFU)

/* General register 0x21: TIMEOUT */
#define ID601_TIMEOUT_TIMEOUT_Pos                          (0U)
#define ID601_TIMEOUT_TIMEOUT_Msk                          (0x7FFU)

/* General register 0x22: FLLCNT1 */
#define ID601_FLLCNT1_FLLCNT_Pos                           (0U)
#define ID601_FLLCNT1_FLLCNT_Msk                           (0xFFFU)

/* General register 0x23: FLLCNT2 */
#define ID601_FLLCNT2_FLL_EN_Pos                           (11U)
#define ID601_FLLCNT2_FLL_EN_Msk                           (0x800U)
#define ID601_FLLCNT2_FLL_RANGE_Pos                        (9U)
#define ID601_FLLCNT2_FLL_RANGE_Msk                        (0x600U)
#define ID601_FLLCNT2_FLLCNT_Pos                           (0U)
#define ID601_FLLCNT2_FLLCNT_Msk                           (0x0FFU)

/* General register 0x24: WR_PROTECT */
#define ID601_WR_PROTECT_WR_PROTECT_Pos                    (0U)
#define ID601_WR_PROTECT_WR_PROTECT_Msk                    (0xFFFU)

/* General register 0x25: NF_CONTROL */
#define ID601_NF_CONTROL_BBKN_TH_Pos                       (6U)
#define ID601_NF_CONTROL_BBKN_TH_Msk                       (0xFC0U)
#define ID601_NF_CONTROL_O_EMI_REJ_EN_Pos                  (5U)
#define ID601_NF_CONTROL_O_EMI_REJ_EN_Msk                  (0x020U)
#define ID601_NF_CONTROL_SGRJT_EN2_Pos                     (4U)
#define ID601_NF_CONTROL_SGRJT_EN2_Msk                     (0x010U)
#define ID601_NF_CONTROL_SGRJT_EN1_Pos                     (3U)
#define ID601_NF_CONTROL_SGRJT_EN1_Msk                     (0x008U)
#define ID601_NF_CONTROL_BBKN_EN_Pos                       (2U)
#define ID601_NF_CONTROL_BBKN_EN_Msk                       (0x004U)
#define ID601_NF_CONTROL_DGRJT_EN2_Pos                     (1U)
#define ID601_NF_CONTROL_DGRJT_EN2_Msk                     (0x002U)
#define ID601_NF_CONTROL_DGRJT_EN1_Pos                     (0U)
#define ID601_NF_CONTROL_DGRJT_EN1_Msk                     (0x001U)

/* General register 0x26: CHOP_EN */
#define ID601_CHOP_EN_OPEN_MASK_OPT_Pos                    (11U)
#define ID601_CHOP_EN_OPEN_MASK_OPT_Msk                    (0x800U)
#define ID601_CHOP_EN_CHOP_EN_Pos                          (5U)
#define ID601_CHOP_EN_CHOP_EN_Msk                          (0x020U)
#define ID601_CHOP_EN_CHOP_DRV_EN_Pos                      (4U)
#define ID601_CHOP_EN_CHOP_DRV_EN_Msk                      (0x010U)
#define ID601_CHOP_EN_CHOP_OSC_LDO_EN_Pos                  (3U)
#define ID601_CHOP_EN_CHOP_OSC_LDO_EN_Msk                  (0x008U)
#define ID601_CHOP_EN_CHOP_OSC_EN_Pos                      (2U)
#define ID601_CHOP_EN_CHOP_OSC_EN_Msk                      (0x004U)
#define ID601_CHOP_EN_CHOP_DAC_EN_Pos                      (1U)
#define ID601_CHOP_EN_CHOP_DAC_EN_Msk                      (0x002U)
#define ID601_CHOP_EN_CHOP_BGR_EN_Pos                      (0U)
#define ID601_CHOP_EN_CHOP_BGR_EN_Msk                      (0x001U)

/* General register 0x27: TEMP */
#define ID601_TEMP_OV_SWAP_EN_Pos                          (7U)
#define ID601_TEMP_OV_SWAP_EN_Msk                          (0x080U)
#define ID601_TEMP_DAC_RNG_Pos                             (6U)
#define ID601_TEMP_DAC_RNG_Msk                             (0x040U)
#define ID601_TEMP_FLT_CTL_Pos                             (4U)
#define ID601_TEMP_FLT_CTL_Msk                             (0x030U)
#define ID601_TEMP_O_SLEW_Pos                              (2U)
#define ID601_TEMP_O_SLEW_Msk                              (0x00CU)
#define ID601_TEMP_FLT_GAIN_Pos                            (0U)
#define ID601_TEMP_FLT_GAIN_Msk                            (0x003U)

/* General register 0x28: OSC_FLL_MAN1 */
#define ID601_OSC_FLL_MAN1_OSC_FLL_MAN_Pos                 (0U)
#define ID601_OSC_FLL_MAN1_OSC_FLL_MAN_Msk                 (0xFFFU)

/* General register 0x29: OSC_FLL_MAN2 */
#define ID601_OSC_FLL_MAN2_OSC_MAN_EN_Pos                  (11U)
#define ID601_OSC_FLL_MAN2_OSC_MAN_EN_Msk                  (0x800U)
#define ID601_OSC_FLL_MAN2_OSC_FLL_ERR_RANGE_Pos           (4U)
#define ID601_OSC_FLL_MAN2_OSC_FLL_ERR_RANGE_Msk           (0x030U)
#define ID601_OSC_FLL_MAN2_OSC_FLL_MAN_Pos                 (0U)
#define ID601_OSC_FLL_MAN2_OSC_FLL_MAN_Msk                 (0x00FU)

/* General register 0x2A: OSC_SPREAD */
#define ID601_OSC_SPREAD_SPRD_EN_Pos                       (11U)
#define ID601_OSC_SPREAD_SPRD_EN_Msk                       (0x800U)
#define ID601_OSC_SPREAD_SPRD_SPD_Pos                      (4U)
#define ID601_OSC_SPREAD_SPRD_SPD_Msk                      (0x070U)
#define ID601_OSC_SPREAD_SPRD_GAIN_Pos                     (0U)
#define ID601_OSC_SPREAD_SPRD_GAIN_Msk                     (0x007U)

/* General register 0x2B: CLOCK_GATE_EN */
#define ID601_CLOCK_GATE_EN_OTP_MCLK_EN_Pos                (4U)
#define ID601_CLOCK_GATE_EN_OTP_MCLK_EN_Msk                (0x010U)
#define ID601_CLOCK_GATE_EN_FR2_MCLK_EN_Pos                (2U)
#define ID601_CLOCK_GATE_EN_FR2_MCLK_EN_Msk                (0x004U)
#define ID601_CLOCK_GATE_EN_FR1_MCLK_EN_Pos                (1U)
#define ID601_CLOCK_GATE_EN_FR1_MCLK_EN_Msk                (0x002U)
#define ID601_CLOCK_GATE_EN_DC_MCLK_EN_Pos                 (0U)
#define ID601_CLOCK_GATE_EN_DC_MCLK_EN_Msk                 (0x001U)

/* General register 0x2C: VREF_FIX1 */
#define ID601_VREF_FIX1_VREF_FIX1_Pos                      (0U)
#define ID601_VREF_FIX1_VREF_FIX1_Msk                      (0xFFFU)

/* General register 0x2D: VREF_FIX2 */
#define ID601_VREF_FIX2_VREF_FIX2_Pos                      (0U)
#define ID601_VREF_FIX2_VREF_FIX2_Msk                      (0xFFFU)

/* General register 0x2E: VREF_FIX3 */
#define ID601_VREF_FIX3_VREF_FIX3_Pos                      (0U)
#define ID601_VREF_FIX3_VREF_FIX3_Msk                      (0xFFFU)

/* General register 0x3F: OP_MODE */
#define ID601_OP_MODE_TEST_EN_Pos                          (11U)
#define ID601_OP_MODE_TEST_EN_Msk                          (0x800U)
#define ID601_OP_MODE_SW_SEL_Pos                           (6U)
#define ID601_OP_MODE_SW_SEL_Msk                           (0x0C0U)
#define ID601_OP_MODE_PWMOUT_FULL_Pos                      (5U)
#define ID601_OP_MODE_PWMOUT_FULL_Msk                      (0x020U)
#define ID601_OP_MODE_MCLK64_O_Pos                         (4U)
#define ID601_OP_MODE_MCLK64_O_Msk                         (0x010U)
#define ID601_OP_MODE_DTEST_MUX_EN_Pos                     (3U)
#define ID601_OP_MODE_DTEST_MUX_EN_Msk                     (0x008U)
#define ID601_OP_MODE_RD_EN_Pos                            (2U)
#define ID601_OP_MODE_RD_EN_Msk                            (0x004U)
#define ID601_OP_MODE_EXT_CLKIN_Pos                        (1U)
#define ID601_OP_MODE_EXT_CLKIN_Msk                        (0x002U)
#define ID601_OP_MODE_ADDR_EXT_Pos                         (0U)
#define ID601_OP_MODE_ADDR_EXT_Msk                         (0x001U)

/* General register 0x39: TEST_ANA_EN */
#define ID601_TEST_ANA_EN_TEST_ANA_EN_Pos                  (0U)
#define ID601_TEST_ANA_EN_TEST_ANA_EN_Msk                  (0x01FU)

/* General register 0x3A: OTP_ACCESS1 */
#define ID601_OTP_ACCESS1_OTP_PG_ACC_CYCLE_Pos             (0U)
#define ID601_OTP_ACCESS1_OTP_PG_ACC_CYCLE_Msk             (0x00FU)

/* General register 0x3B: OTP_ACCESS2 */
#define ID601_OTP_ACCESS2_OTP_PG_ACC_CYCLE_Pos             (0U)
#define ID601_OTP_ACCESS2_OTP_PG_ACC_CYCLE_Msk             (0xFFFU)

/* General register 0x3C: OTP_WRITE */
#define ID601_OTP_WRITE_OTP_RD_Pos                         (4U)
#define ID601_OTP_WRITE_OTP_RD_Msk                         (0x030U)
#define ID601_OTP_WRITE_OTP_WSEL_Pos                       (0U)
#define ID601_OTP_WRITE_OTP_WSEL_Msk                       (0x00FU)

/* General register 0x3D: OTP_RD_PROG */
#define ID601_OTP_RD_PROG_OTP_PG_DONE_Pos                  (11U)
#define ID601_OTP_RD_PROG_OTP_PG_DONE_Msk                  (0x800U)
#define ID601_OTP_RD_PROG_OTP_RD_S_Pos                     (1U)
#define ID601_OTP_RD_PROG_OTP_RD_S_Msk                     (0x002U)
#define ID601_OTP_RD_PROG_OTP_PG_S_Pos                     (0U)
#define ID601_OTP_RD_PROG_OTP_PG_S_Msk                     (0x001U)

/* General register 0x3E: OTP_PROTECT */
#define ID601_OTP_PROTECT_PROTECT_EN_Pos                   (0U)
#define ID601_OTP_PROTECT_PROTECT_EN_Msk                   (0xFFFU)

/* Mirror register 0x00: OTP1 */
#define ID601_OTP1_OTP_CRC_CHECKSUM_Pos                    (0U)
#define ID601_OTP1_OTP_CRC_CHECKSUM_Msk                    (0x0FFU)

/* Mirror register 0x01: OTP2 */
#define ID601_OTP2_IREF_CTL_Pos                            (5U)
#define ID601_OTP2_IREF_CTL_Msk                            (0x3E0U)
#define ID601_OTP2_BGR_TC_Pos                              (0U)
#define ID601_OTP2_BGR_TC_Msk                              (0x01FU)

/* Mirror register 0x02: OTP3 */
#define ID601_OTP3_IREF2_CTL_Pos                           (5U)
#define ID601_OTP3_IREF2_CTL_Msk                           (0x3E0U)
#define ID601_OTP3_BGR2_TC_Pos                             (0U)
#define ID601_OTP3_BGR2_TC_Msk                             (0x01FU)

/* Mirror register 0x03: OTP4 */
#define ID601_OTP4_OFS_TEMP_Pos                            (8U)
#define ID601_OTP4_OFS_TEMP_Msk                            (0xF00U)
#define ID601_OTP4_LDO_BGR2_CTL_Pos                        (0U)
#define ID601_OTP4_LDO_BGR2_CTL_Msk                        (0x01FU)

/* Mirror register 0x04: OTP5 */
#define ID601_OTP5_LDO_DAC_CTL_Pos                         (7U)
#define ID601_OTP5_LDO_DAC_CTL_Msk                         (0xF80U)
#define ID601_OTP5_OSC_RCTL_Pos                            (0U)
#define ID601_OTP5_OSC_RCTL_Msk                            (0x01FU)

/* Mirror register 0x05: OTP6 */
#define ID601_OTP6_LDO_OSC_CTL_Pos                         (4U)
#define ID601_OTP6_LDO_OSC_CTL_Msk                         (0x0F0U)
#define ID601_OTP6_LDO_CTL_Pos                             (0U)
#define ID601_OTP6_LDO_CTL_Msk                             (0x00FU)

/* Mirror register 0x06: OFFSET_CH01 */
#define ID601_OFFSET_CH01_OFS_CH1_Pos                      (0U)
#define ID601_OFFSET_CH01_OFS_CH1_Msk                      (0x1FFU)

/* Mirror register 0x07: OFFSET_CH02 */
#define ID601_OFFSET_CH02_OFS_CH2_Pos                      (0U)
#define ID601_OFFSET_CH02_OFS_CH2_Msk                      (0x1FFU)

/* Mirror register 0x08: OFFSET_CH03 */
#define ID601_OFFSET_CH03_OFS_CH3_Pos                      (0U)
#define ID601_OFFSET_CH03_OFS_CH3_Msk                      (0x1FFU)

/* Mirror register 0x09: OFFSET_CH04 */
#define ID601_OFFSET_CH04_OFS_CH4_Pos                      (0U)
#define ID601_OFFSET_CH04_OFS_CH4_Msk                      (0x1FFU)

/* Mirror register 0x0A: OFFSET_CH05 */
#define ID601_OFFSET_CH05_OFS_CH5_Pos                      (0U)
#define ID601_OFFSET_CH05_OFS_CH5_Msk                      (0x1FFU)

/* Mirror register 0x0B: OFFSET_CH06 */
#define ID601_OFFSET_CH06_OFS_CH6_Pos                      (0U)
#define ID601_OFFSET_CH06_OFS_CH6_Msk                      (0x1FFU)

/* Mirror register 0x0C: OFFSET_CH07 */
#define ID601_OFFSET_CH07_OFS_CH7_Pos                      (0U)
#define ID601_OFFSET_CH07_OFS_CH7_Msk                      (0x1FFU)

/* Mirror register 0x0D: OFFSET_CH08 */
#define ID601_OFFSET_CH08_OFS_CH8_Pos                      (0U)
#define ID601_OFFSET_CH08_OFS_CH8_Msk                      (0x1FFU)

/* Mirror register 0x0E: OFFSET_CH09 */
#define ID601_OFFSET_CH09_OFS_CH9_Pos                      (0U)
#define ID601_OFFSET_CH09_OFS_CH9_Msk                      (0x1FFU)

/* Mirror register 0x0F: OFFSET_CH10 */
#define ID601_OFFSET_CH10_OFS_CH10_Pos                     (0U)
#define ID601_OFFSET_CH10_OFS_CH10_Msk                     (0x1FFU)

/* Mirror register 0x10: OFFSET_CH11 */
#define ID601_OFFSET_CH11_OFS_CH11_Pos                     (0U)
#define ID601_OFFSET_CH11_OFS_CH11_Msk                     (0x1FFU)

/* Mirror register 0x11: OFFSET_CH12 */
#define ID601_OFFSET_CH12_OFS_CH12_Pos                     (0U)
#define ID601_OFFSET_CH12_OFS_CH12_Msk                     (0x1FFU)

/* Mirror register 0x12: GAIN_CH01 */
#define ID601_GAIN_CH01_GAIN_CH1_Pos                       (0U)
#define ID601_GAIN_CH01_GAIN_CH1_Msk                       (0x07FU)

/* Mirror register 0x13: GAIN_CH02 */
#define ID601_GAIN_CH02_GAIN_CH2_Pos                       (0U)
#define ID601_GAIN_CH02_GAIN_CH2_Msk                       (0x07FU)

/* Mirror register 0x14: GAIN_CH03 */
#define ID601_GAIN_CH03_GAIN_CH3_Pos                       (0U)
#define ID601_GAIN_CH03_GAIN_CH3_Msk                       (0x07FU)

/* Mirror register 0x15: GAIN_CH04 */
#define ID601_GAIN_CH04_GAIN_CH4_Pos                       (0U)
#define ID601_GAIN_CH04_GAIN_CH4_Msk                       (0x07FU)

/* Mirror register 0x16: GAIN_CH05 */
#define ID601_GAIN_CH05_GAIN_CH5_Pos                       (0U)
#define ID601_GAIN_CH05_GAIN_CH5_Msk                       (0x07FU)

/* Mirror register 0x17: GAIN_CH06 */
#define ID601_GAIN_CH06_GAIN_CH6_Pos                       (0U)
#define ID601_GAIN_CH06_GAIN_CH6_Msk                       (0x07FU)

/* Mirror register 0x18: GAIN_CH07 */
#define ID601_GAIN_CH07_GAIN_CH7_Pos                       (0U)
#define ID601_GAIN_CH07_GAIN_CH7_Msk                       (0x07FU)

/* Mirror register 0x19: GAIN_CH08 */
#define ID601_GAIN_CH08_GAIN_CH8_Pos                       (0U)
#define ID601_GAIN_CH08_GAIN_CH8_Msk                       (0x07FU)

/* Mirror register 0x1A: GAIN_CH09 */
#define ID601_GAIN_CH09_GAIN_CH9_Pos                       (0U)
#define ID601_GAIN_CH09_GAIN_CH9_Msk                       (0x07FU)

/* Mirror register 0x1B: GAIN_CH10 */
#define ID601_GAIN_CH10_GAIN_CH10_Pos                      (0U)
#define ID601_GAIN_CH10_GAIN_CH10_Msk                      (0x07FU)

/* Mirror register 0x1C: GAIN_CH11 */
#define ID601_GAIN_CH11_GAIN_CH11_Pos                      (0U)
#define ID601_GAIN_CH11_GAIN_CH11_Msk                      (0x07FU)

/* Mirror register 0x1D: GAIN_CH12 */
#define ID601_GAIN_CH12_GAIN_CH12_Pos                      (0U)
#define ID601_GAIN_CH12_GAIN_CH12_Msk                      (0x07FU)

/* Mirror register 0x1E: VERSION */
#define ID601_VERSION_VERSION0_Pos                         (0U)
#define ID601_VERSION_VERSION0_Msk                         (0x0FFU)


/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum tag_ID601_GENERAL_REGISTER_ADDR
{
    ID601_ADDR_GENERAL_RESET_ID = 0x00U,         /* 0x00 */
    ID601_ADDR_GENERAL_LD_CONTROL,               /* 0x01 */
    ID601_ADDR_GENERAL_LD_SIZE,                  /* 0x02 */
    ID601_ADDR_GENERAL_PWMCLK_DIV1_2,            /* 0x03 */
    ID601_ADDR_GENERAL_PWMCLK_DIV2_3,            /* 0x04 */
    ID601_ADDR_GENERAL_CHANNEL_ENABLE,           /* 0x05 */
    ID601_ADDR_GENERAL_FAULT_CONTROL,            /* 0x06 */
    ID601_ADDR_GENERAL_FB_LEVEL,                 /* 0x07 */
    ID601_ADDR_GENERAL_SHORT_LEVEL,              /* 0x08 */
    ID601_ADDR_GENERAL_FAULT_STATUS0,            /* 0x09 */
    ID601_ADDR_GENERAL_FAULT_STATUS1,            /* 0x0A */
    ID601_ADDR_GENERAL_FAULT_STATUS2,            /* 0x0B */
    ID601_ADDR_GENERAL_FAULT_STATUS3,            /* 0x0C */
    ID601_ADDR_GENERAL_MAX_CURR_LEVEL,           /* 0x0D */
    ID601_ADDR_GENERAL_MAX_CURR_VREF1,           /* 0x0E */
    ID601_ADDR_GENERAL_MAX_CURR_VREF2,           /* 0x0F */

    ID601_ADDR_GENERAL_MAX_CURR_VREF3 = 0x10U,   /* 0x10 */
    ID601_ADDR_GENERAL_CH_LD_TYPE,               /* 0x11 */
    ID601_ADDR_GENERAL_DELAY_CH1_2,              /* 0x12 */
    ID601_ADDR_GENERAL_DELAY_CH3_4,              /* 0x13 */
    ID601_ADDR_GENERAL_DELAY_CH5_6,              /* 0x14 */
    ID601_ADDR_GENERAL_DELAY_CH7_8,              /* 0x15 */
    ID601_ADDR_GENERAL_DELAY_CH9_10,             /* 0x16 */
    ID601_ADDR_GENERAL_DELAY_CH11_12,            /* 0x17 */
    ID601_ADDR_GENERAL_FAULT_CONFIGURATION,      /* 0x18 */
    ID601_ADDR_GENERAL_BIST,                     /* 0x19 */
    ID601_ADDR_GENERAL_TEST_MODE0,               /* 0x1A */
    ID601_ADDR_GENERAL_TEST_MODE1,               /* 0x1B */
    ID601_ADDR_GENERAL_SERIAL_CLK_GEN,           /* 0x1C */
    ID601_ADDR_GENERAL_SERIAL_LATENCY,           /* 0x1D */
    ID601_ADDR_GENERAL_V_MASK,                   /* 0x1E */
    ID601_ADDR_GENERAL_SV_MASK,                  /* 0x1F */

    ID601_ADDR_GENERAL_RSTCNT = 0x20U,           /* 0x20 */
    ID601_ADDR_GENERAL_TIMEOUT,                  /* 0x21 */
    ID601_ADDR_GENERAL_FLLCNT1,                  /* 0x22 */
    ID601_ADDR_GENERAL_FLLCNT2,                  /* 0x23 */
    ID601_ADDR_GENERAL_WR_PROTECT,               /* 0x24 */
    ID601_ADDR_GENERAL_NF_CONTROL,               /* 0x25 */
    ID601_ADDR_GENERAL_CHOP_EN,                  /* 0x26 */
    ID601_ADDR_GENERAL_TEMP,                     /* 0x27 */
    ID601_ADDR_GENERAL_OSC_FLL_MAN1,             /* 0x28 */
    ID601_ADDR_GENERAL_OSC_FLL_MAN2,             /* 0x29 */
    ID601_ADDR_GENERAL_OSC_SPREAD,               /* 0x2A */
    ID601_ADDR_GENERAL_CLOCK_GATE_EN,            /* 0x2B */
    ID601_ADDR_GENERAL_VREF_FIX1,                /* 0x2C */
    ID601_ADDR_GENERAL_VREF_FIX2,                /* 0x2D */
    ID601_ADDR_GENERAL_VREF_FIX3,                /* 0x2E */

    ID601_ADDR_GENERAL_TEST_ANA_EN = 0x39U,      /* 0x39 */
    ID601_ADDR_GENERAL_OTP_ACCESS1,              /* 0x3A */
    ID601_ADDR_GENERAL_OTP_ACCESS2,              /* 0x3B */
    ID601_ADDR_GENERAL_OTP_WRITE,                /* 0x3C */
    ID601_ADDR_GENERAL_OTP_RD_PROG,              /* 0x3D */
    ID601_ADDR_GENERAL_OTP_PROTECT,              /* 0x3E */
    ID601_ADDR_GENERAL_OP_MODE,                  /* 0x3F */

    ID601_ADDR_GENERAL_COUNT,
} id601_general_register_addr_t;

typedef enum tag_ID601_MIRROR_REGISTER_ADDR
{
    ID601_ADDR_MIRROR1 = 0x00U,          /* 0x00 */
    ID601_ADDR_MIRROR2,                  /* 0x01 */
    ID601_ADDR_MIRROR3,                  /* 0x02 */
    ID601_ADDR_MIRROR4,                  /* 0x03 */
    ID601_ADDR_MIRROR5,                  /* 0x04 */
    ID601_ADDR_MIRROR6,                  /* 0x05 */
    ID601_ADDR_MIRROR_OFS_CH01,          /* 0x06 */
    ID601_ADDR_MIRROR_OFS_CH02,          /* 0x07 */
    ID601_ADDR_MIRROR_OFS_CH03,          /* 0x08 */
    ID601_ADDR_MIRROR_OFS_CH04,          /* 0x09 */
    ID601_ADDR_MIRROR_OFS_CH05,          /* 0x0A */
    ID601_ADDR_MIRROR_OFS_CH06,          /* 0x0B */
    ID601_ADDR_MIRROR_OFS_CH07,          /* 0x0C */
    ID601_ADDR_MIRROR_OFS_CH08,          /* 0x0D */
    ID601_ADDR_MIRROR_OFS_CH09,          /* 0x0E */
    ID601_ADDR_MIRROR_OFS_CH10,          /* 0x0F */
    ID601_ADDR_MIRROR_OFS_CH11,          /* 0x10 */
    ID601_ADDR_MIRROR_OFS_CH12,          /* 0x11 */

    ID601_ADDR_MIRROR_GAIN_CH01 = 0x12U, /* 0x12 */
    ID601_ADDR_MIRROR_GAIN_CH02,         /* 0x13 */
    ID601_ADDR_MIRROR_GAIN_CH03,         /* 0x14 */
    ID601_ADDR_MIRROR_GAIN_CH04,         /* 0x15 */
    ID601_ADDR_MIRROR_GAIN_CH05,         /* 0x16 */
    ID601_ADDR_MIRROR_GAIN_CH06,         /* 0x17 */
    ID601_ADDR_MIRROR_GAIN_CH07,         /* 0x18 */
    ID601_ADDR_MIRROR_GAIN_CH08,         /* 0x19 */
    ID601_ADDR_MIRROR_GAIN_CH09,         /* 0x1A */
    ID601_ADDR_MIRROR_GAIN_CH10,         /* 0x1B */
    ID601_ADDR_MIRROR_GAIN_CH11,         /* 0x1C */
    ID601_ADDR_MIRROR_GAIN_CH12,         /* 0x1D */
    ID601_ADDR_MIRROR_VERSION,           /* 0x1E */

    ID601_ADDR_MIRROR_COUNT,
} id601_mirror_register_addr_t;

typedef struct tag_ID601_DUMMY
{
    uint16_t dummy :12;
    uint16_t       : 4;
} _id601_dummy_t;

typedef struct tag_ID601_RESET_ID
{
    uint16_t id     : 5;
    uint16_t        : 3;
    uint16_t lkg_e  : 1;
    uint16_t e_rst  : 1;
    uint16_t vs_rst : 1;
    uint16_t rst    : 1;
    uint16_t        : 4;
} _id601_reset_id_t;

typedef struct tag_ID601_LD_CONTROL
{
    uint16_t ld_mode     : 1;
    uint16_t ld_dir      : 1;
    uint16_t pwm_res     : 1;
    uint16_t syncmode    : 1;
    uint16_t delay_ch_en : 1;
    uint16_t ld_type     : 1;
    uint16_t sv_no       : 6;
    uint16_t             : 4;
} _id601_ld_control_t;

typedef struct tag_ID601_LD_SIZE
{
    uint16_t ld_size : 6;
    uint16_t         :10;
} _id601_ld_size_t;

typedef struct tag_ID601_PWMCLK_DIV1_2
{
    uint16_t fpwm_div1 : 8;
    uint16_t fpwm_div2 : 4;
    uint16_t           : 4;
} _id601_pwmclk_div1_2_t;

typedef struct tag_ID601_PWMCLK_DIV2_3
{
    uint16_t fpwm_div2 : 4;
    uint16_t fpwm_div3 : 8;
    uint16_t           : 4;
} _id601_pwmclk_div2_3_t;

typedef struct tag_ID601_CHANNEL_ENABLE
{
    uint16_t ch1_en  : 1;
    uint16_t ch2_en  : 1;
    uint16_t ch3_en  : 1;
    uint16_t ch4_en  : 1;
    uint16_t ch5_en  : 1;
    uint16_t ch6_en  : 1;
    uint16_t ch7_en  : 1;
    uint16_t ch8_en  : 1;
    uint16_t ch9_en  : 1;
    uint16_t ch10_en : 1;
    uint16_t ch11_en : 1;
    uint16_t ch12_en : 1;
    uint16_t         : 4;
} _id601_channel_enable_t;

typedef struct tag_ID601_FAULT_CONTROL0
{
    uint16_t o_off_e  : 1;
    uint16_t s_off_e  : 1;
    uint16_t t_off_e  : 1;
    uint16_t o_det_e  : 1;
    uint16_t s_det_e  : 1;
    uint16_t t_det_e  : 1;
    uint16_t o_fb_e   : 1;
    uint16_t fb_mode  : 2;
    uint16_t ft_mode  : 2;
    uint16_t          : 5;
} _id601_fault_control0_t;

typedef struct tag_ID601_FB_LEVEL
{
    uint16_t fb1_level : 3;
    uint16_t fb2_level : 3;
    uint16_t fb3_level : 3;
    uint16_t           : 7;
} _id601_fb_level_t;

typedef struct tag_ID601_SHORT_LEVEL
{
    uint16_t short1_level : 3;
    uint16_t short2_level : 3;
    uint16_t short3_level : 3;
    uint16_t              : 7;
} _id601_short_level_t;

typedef struct tag_ID601_FAULT_STATUS0
{
    uint16_t bit_fb1   : 1;
    uint16_t bit_fb2   : 1;
    uint16_t bit_fb3   : 1;
    uint16_t bit_open  : 1;
    uint16_t bit_short : 1;
    uint16_t           :11;
} _id601_fault_status0_t;

typedef struct tag_ID601_FAULT_STATUS1
{
    uint16_t thermal          : 1;
    uint16_t ldo_uv           : 1;
    uint16_t ldo_ov           : 1;
    uint16_t osc_ldo_uv       : 1;
    uint16_t osc_ldo_ov       : 1;
    uint16_t vref_uv          : 1;
    uint16_t vref_ov          : 1;
    uint16_t ch_ctrl_mismatch : 1;
    uint16_t vs_miss          : 1;
    uint16_t wd_timeout       : 1;
    uint16_t cmd_err          : 1;
    uint16_t resp_err         : 1;
    uint16_t                  : 4;
} _id601_fault_status1_t;

typedef struct tag_ID601_FAULT_STATUS2
{
    uint16_t open_ch1  : 1;
    uint16_t open_ch2  : 1;
    uint16_t open_ch3  : 1;
    uint16_t open_ch4  : 1;
    uint16_t open_ch5  : 1;
    uint16_t open_ch6  : 1;
    uint16_t open_ch7  : 1;
    uint16_t open_ch8  : 1;
    uint16_t open_ch9  : 1;
    uint16_t open_ch10 : 1;
    uint16_t open_ch11 : 1;
    uint16_t open_ch12 : 1;
    uint16_t           : 4;
} _id601_fault_status2_t;

typedef struct tag_ID601_FAULT_STATUS3
{
    uint16_t short_ch1  : 1;
    uint16_t short_ch2  : 1;
    uint16_t short_ch3  : 1;
    uint16_t short_ch4  : 1;
    uint16_t short_ch5  : 1;
    uint16_t short_ch6  : 1;
    uint16_t short_ch7  : 1;
    uint16_t short_ch8  : 1;
    uint16_t short_ch9  : 1;
    uint16_t short_ch10 : 1;
    uint16_t short_ch11 : 1;
    uint16_t short_ch12 : 1;
    uint16_t            : 4;
} _id601_fault_status3_t;

typedef struct tag_ID601_MAX_CURRENT_LEVEL
{
    uint16_t max_curr_level1 : 4;
    uint16_t max_curr_level2 : 4;
    uint16_t max_curr_level3 : 4;
    uint16_t                 : 4;
} _id601_max_current_level_t;

typedef struct tag_ID601_MAX_CURR_VREF
{
    uint16_t max_curr_vref :12;
    uint16_t               : 4;
} _id601_max_curr_vref_t;

typedef struct tag_ID601_CHX_LD_TYPE
{
    uint16_t ch1_ld_type  : 1;
    uint16_t ch2_ld_type  : 1;
    uint16_t ch3_ld_type  : 1;
    uint16_t ch4_ld_type  : 1;
    uint16_t ch5_ld_type  : 1;
    uint16_t ch6_ld_type  : 1;
    uint16_t ch7_ld_type  : 1;
    uint16_t ch8_ld_type  : 1;
    uint16_t ch9_ld_type  : 1;
    uint16_t ch10_ld_type : 1;
    uint16_t ch11_ld_type : 1;
    uint16_t ch12_ld_type : 1;
    uint16_t              : 4;
} _id601_chx_ld_type_t;

typedef struct tag_ID601_DELAY_CH
{
    uint16_t delay_ch_x1 : 5;
    uint16_t delay_ch_x2 : 5;
    uint16_t             : 6;
} _id601_delay_ch_t;

typedef struct tag_ID601_FAULT_CONFIGURATION
{
    uint16_t ldo_ovuv_en          : 1;
    uint16_t osc_ldo_ovuv_en      : 1;
    uint16_t vref_ovuv_en         : 1;
    uint16_t vref_ovuv_off_en     : 1;
    uint16_t ch_ctrl_mismatch_en  : 1;
    uint16_t vs_miss_en           : 1;
    uint16_t vs_miss_dimm_en      : 1;
    uint16_t wd_en                : 1;
    uint16_t wd_timecfg           : 2;
    uint16_t wd_dimm_en           : 1;
    uint16_t crc_en               : 1;
    uint16_t                      : 4;
} _id601_fault_configuration_t;

typedef struct tag_ID601_BIST
{
    uint16_t bist_err   : 1;
    uint16_t bist_done  : 1;
    uint16_t bist_busy  : 1;
    uint16_t            : 8;
    uint16_t bist_start : 1;
    uint16_t            : 4;
} _id601_bist_t;

typedef struct tag_ID601_TEST_MODE0
{
    uint16_t dmux_sel : 5;
    uint16_t          :11;
} _id601_test_mode0_t;

typedef struct tag_ID601_TEST_MODE1
{
    uint16_t tm_ch_ctrl_input_flip : 1;
    uint16_t                       :10;
    uint16_t testmode              : 1;
    uint16_t                       : 4;
} _id601_test_mode1_t;

typedef struct tag_ID601_SERIAL_CLK_GEN
{
    uint16_t serial_clk_high : 6;
    uint16_t serial_clk_low  : 6;
    uint16_t                 : 4;
} _id601_serial_clk_gen_t;

typedef struct tag_ID601_SERIAL_LATENCY
{
    uint16_t serial_latency : 9;
    uint16_t                : 7;
} _id601_serial_latency_t;

typedef struct tag_ID601_V_MASK
{
    uint16_t v_mask :10;
    uint16_t        : 6;
} _id601_v_mask_t;

typedef struct tag_ID601_SV_MASK
{
    uint16_t sv_mask    :10;
    uint16_t            : 1;
    uint16_t sv_mask_en : 1;
    uint16_t            : 4;
} _id601_sv_mask_t;

typedef struct tag_ID601_RSTCNT
{
    uint16_t rstcnt :10;
    uint16_t        : 6;
} _id601_rstcnt_t;

typedef struct tag_ID601_TIMEOUT
{
    uint16_t timeout :11;
    uint16_t         : 5;
} _id601_timeout_t;

typedef struct tag_ID601_FLLCNT1
{
    uint16_t fllcnt :12;
    uint16_t        : 4;
} _id601_fllcnt1_t;

typedef struct tag_ID601_FLLCNT2
{
    uint16_t fllcnt_high : 8;
    uint16_t             : 1;
    uint16_t fll_range   : 2;
    uint16_t fll_en      : 1;
    uint16_t             : 4;
} _id601_fllcnt2_t;

typedef struct tag_ID601_WR_PROTECT
{
    uint16_t wr_protect :12;
    uint16_t            : 4;
} _id601_wr_protect_t;

typedef struct tag_ID601_NF_CONTROL
{
    uint16_t dgrjt_en1    : 1;
    uint16_t dgrjt_en2    : 1;
    uint16_t bbkn_en      : 1;
    uint16_t sgrjt_en1    : 1;
    uint16_t sgrjt_en2    : 1;
    uint16_t o_emi_rej_en : 1;
    uint16_t bbkn_th      : 6;
    uint16_t              : 4;
} _id601_nf_control_t;

typedef struct tag_ID601_CHOP_EN
{
    uint16_t chop_bgr_en     : 1;
    uint16_t chop_dac_en     : 1;
    uint16_t chop_osc_en     : 1;
    uint16_t chop_osc_ldo_en : 1;
    uint16_t chop_drv_en     : 1;
    uint16_t chop_en         : 1;
    uint16_t                 : 5;
    uint16_t open_mask_opt   : 1;
    uint16_t                 : 1;
} _id601_chop_en_t;

typedef struct tag_ID601_TEMP
{
    uint16_t flt_gain   : 2;
    uint16_t o_slew     : 2;
    uint16_t flt_ctl    : 2;
    uint16_t dac_rng    : 1;
    uint16_t ov_swap_en : 1;
    uint16_t            : 8;
} _id601_temp_t;

typedef struct tag_ID601_OSC_FLL_MAN1
{
    uint16_t osc_fll_man :12;
    uint16_t             : 4;
} _id601_osc_fll_man1_t;

typedef struct tag_ID601_OSC_FLL_MAN2
{
    uint16_t osc_fll_man       : 4;
    uint16_t osc_fll_err_range : 2;
    uint16_t                   : 5;
    uint16_t osc_man_en        : 1;
    uint16_t                   : 4;
} _id601_osc_fll_man2_t;

typedef struct tag_ID601_OSC_SPREAD
{
    uint16_t sprd_gain : 3;
    uint16_t           : 1;
    uint16_t sprd_spd  : 3;
    uint16_t           : 4;
    uint16_t sprd_en   : 1;
    uint16_t           : 4;
} _id601_osc_spread_t;

typedef struct tag_ID601_CLOCK_GATE_EN
{
    uint16_t dc_mclk_en  : 1;
    uint16_t fr1_mclk_en : 1;
    uint16_t fr2_mclk_en : 1;
    uint16_t             : 1;
    uint16_t otp_mclk_en : 1;
    uint16_t             :11;
} _id601_clock_gate_en_t;

typedef struct tag_ID601_VREF_FIX
{
    uint16_t vref_fix :12;
    uint16_t          : 4;
} _id601_vref_fix_t;

typedef struct tag_ID601_TEST_ANA_EN
{
    uint16_t test_ana_en : 5;
    uint16_t             :11;
} _id601_test_ana_en_t;

typedef struct tag_ID601_OTP_ACCESS1
{
    uint16_t otp_pg_acc_cycle : 4;
    uint16_t                  :12;
} _id601_otp_access1_t;

typedef struct tag_ID601_OTP_ACCESS2
{
    uint16_t otp_pg_acc_cycle :12;
    uint16_t                  : 4;
} _id601_otp_access2_t;

typedef struct tag_ID601_OTP_WRITE
{
    uint16_t otp_wsel : 4;
    uint16_t otp_rd   : 2;
    uint16_t          :10;
} _id601_otp_write_t;

typedef struct tag_ID601_OTP_RD_PROG
{
    uint16_t otp_pg_s    : 1;
    uint16_t otp_rd_s    : 1;
    uint16_t             : 9;
    uint16_t otp_pg_done : 1;
    uint16_t             : 4;
} _id601_otp_rd_prog_t;

typedef struct tag_ID601_OTP_PROTECT
{
    uint16_t protect_en :12;
    uint16_t            : 4;
} _id601_otp_protect_t;

typedef struct tag_ID601_OP_MODE
{
    uint16_t addr_ext     : 1;
    uint16_t ext_clkin    : 1;
    uint16_t rd_en        : 1;
    uint16_t dtest_mux_en : 1;
    uint16_t mclk64_o     : 1;
    uint16_t pwmout_full  : 1;
    uint16_t sw_sel       : 2;
    uint16_t              : 3;
    uint16_t test_en      : 1;
    uint16_t              : 4;
} _id601_op_mode_t;

typedef struct tag_ID601_OTP_MIRROR1
{
    uint16_t otp_crc_checksum : 8;
    uint16_t                  : 8;
} _id601_otp_mirror1_t;

typedef struct tag_ID601_OTP_MIRROR2
{
    uint16_t bgr_tc   : 5;
    uint16_t iref_ctl : 5;
    uint16_t          : 6;
} _id601_otp_mirror2_t;

typedef struct tag_ID601_OTP_MIRROR3
{
    uint16_t bgr2_tc   : 5;
    uint16_t iref2_ctl : 5;
    uint16_t           : 6;
} _id601_otp_mirror3_t;

typedef struct tag_ID601_OTP_MIRROR4
{
    uint16_t ldo_bgr2_ctl : 5;
    uint16_t              : 3;
    uint16_t ofs_temp     : 4;
    uint16_t              : 4;
} _id601_otp_mirror4_t;

typedef struct tag_ID601_OTP_MIRROR5
{
    uint16_t osc_rctl    : 5;
    uint16_t             : 2;
    uint16_t ldo_dac_ctl : 5;
    uint16_t             : 4;
} _id601_otp_mirror5_t;

typedef struct tag_ID601_OTP_MIRROR6
{
    uint16_t ldo_ctl     : 4;
    uint16_t ldo_osc_ctl : 4;
    uint16_t             : 8;
} _id601_otp_mirror6_t;

typedef struct tag_ID601_OTP_MIRROR_OFS
{
    uint16_t ofs_ch : 9;
    uint16_t        : 7;
} _id601_otp_mirror_ofs_t;

typedef struct tag_ID601_OTP_MIRROR_GAIN
{
    uint16_t gain_ch : 7;
    uint16_t         : 9;
} _id601_otp_mirror_gain_t;

typedef struct tag_ID601_OTP_MIRROR_VERSION
{
    uint16_t version0 : 8;
    uint16_t          : 8;
} _id601_otp_mirror_version_t;

typedef struct tag_ID601_GENERAL_REGS
{
    _id601_reset_id_t               _r00;
    _id601_ld_control_t             _r01;
    _id601_ld_size_t                _r02;
    _id601_pwmclk_div1_2_t          _r03;
    _id601_pwmclk_div2_3_t          _r04;
    _id601_channel_enable_t         _r05;
    _id601_fault_control0_t         _r06;
    _id601_fb_level_t               _r07;
    _id601_short_level_t            _r08;
    _id601_fault_status0_t          _r09;
    _id601_fault_status1_t          _r0A;
    _id601_fault_status2_t          _r0B;
    _id601_fault_status3_t          _r0C;
    _id601_max_current_level_t      _r0D;
    _id601_max_curr_vref_t          _r0E;
    _id601_max_curr_vref_t          _r0F;

    _id601_max_curr_vref_t          _r10;
    _id601_chx_ld_type_t            _r11;
    _id601_delay_ch_t               _r12;
    _id601_delay_ch_t               _r13;
    _id601_delay_ch_t               _r14;
    _id601_delay_ch_t               _r15;
    _id601_delay_ch_t               _r16;
    _id601_delay_ch_t               _r17;
    _id601_fault_configuration_t    _r18;
    _id601_bist_t                   _r19;
    _id601_test_mode0_t             _r1A;
    _id601_test_mode1_t             _r1B;
    _id601_serial_clk_gen_t         _r1C;
    _id601_serial_latency_t         _r1D;
    _id601_v_mask_t                 _r1E;
    _id601_sv_mask_t                _r1F;
    _id601_rstcnt_t                 _r20;
    _id601_timeout_t                _r21;
    _id601_fllcnt1_t                _r22;
    _id601_fllcnt2_t                _r23;
    _id601_wr_protect_t             _r24;
    _id601_nf_control_t             _r25;
    _id601_chop_en_t                _r26;
    _id601_temp_t                   _r27;
    _id601_osc_fll_man1_t           _r28;
    _id601_osc_fll_man2_t           _r29;
    _id601_osc_spread_t             _r2A;
    _id601_clock_gate_en_t          _r2B;
    _id601_vref_fix_t               _r2C;
    _id601_vref_fix_t               _r2D;
    _id601_vref_fix_t               _r2E;
    _id601_dummy_t                  _r2F;

    _id601_dummy_t                  _r30;
    _id601_dummy_t                  _r31;
    _id601_dummy_t                  _r32;
    _id601_dummy_t                  _r33;
    _id601_dummy_t                  _r34;
    _id601_dummy_t                  _r35;
    _id601_dummy_t                  _r36;
    _id601_dummy_t                  _r37;
    _id601_dummy_t                  _r38;
    _id601_test_ana_en_t            _r39;
    _id601_otp_access1_t            _r3A;
    _id601_otp_access2_t            _r3B;
    _id601_otp_write_t              _r3C;
    _id601_otp_rd_prog_t            _r3D;
    _id601_otp_protect_t            _r3E;
    _id601_op_mode_t                _r3F;
} _id601_general_regs_t;

typedef struct tag_ID601_MIRROR_REGS
{
    _id601_otp_mirror1_t        _r00;
    _id601_otp_mirror2_t        _r01;
    _id601_otp_mirror3_t        _r02;
    _id601_otp_mirror4_t        _r03;
    _id601_otp_mirror5_t        _r04;
    _id601_otp_mirror6_t        _r05;
    _id601_otp_mirror_ofs_t     _r06;
    _id601_otp_mirror_ofs_t     _r07;
    _id601_otp_mirror_ofs_t     _r08;
    _id601_otp_mirror_ofs_t     _r09;
    _id601_otp_mirror_ofs_t     _r0A;
    _id601_otp_mirror_ofs_t     _r0B;
    _id601_otp_mirror_ofs_t     _r0C;
    _id601_otp_mirror_ofs_t     _r0D;
    _id601_otp_mirror_ofs_t     _r0E;
    _id601_otp_mirror_ofs_t     _r0F;

    _id601_otp_mirror_ofs_t     _r10;
    _id601_otp_mirror_ofs_t     _r11;
    _id601_otp_mirror_gain_t    _r12;
    _id601_otp_mirror_gain_t    _r13;
    _id601_otp_mirror_gain_t    _r14;
    _id601_otp_mirror_gain_t    _r15;
    _id601_otp_mirror_gain_t    _r16;
    _id601_otp_mirror_gain_t    _r17;
    _id601_otp_mirror_gain_t    _r18;
    _id601_otp_mirror_gain_t    _r19;
    _id601_otp_mirror_gain_t    _r1A;
    _id601_otp_mirror_gain_t    _r1B;
    _id601_otp_mirror_gain_t    _r1C;
    _id601_otp_mirror_gain_t    _r1D;
    _id601_otp_mirror_version_t _r1E;
} _id601_mirror_regs_t;
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

#endif /* __ID601_METADATA_H__ */
