/** @file sal_driver.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */

#include "sal_driver.h"
#include "drv_sal.h"
#include <string.h>

#define PREAMBLE    (0xA)
#define DUMMY_BIT   (0xA)

#define WR                  (2U << 6U)
#define RD                  (1U << 6U)
#define SR                  (3U << 6U)

#define REG_STATUS1         0x01
#define REG_STATUS2         0x02
#define REG_TEMP            0x03
#define REG_VEXT_TM         0x05
#define REG_SETUP1          0x06
#define REG_SETUP2          0x07
#define REG_MCAST           0x08
#define REG_TEMPTH          0x0A
#define REG_TEMPHYS         0x0B
#define REG_CAL_PWM_RED     0x0C
#define REG_CAL_PWM_GREEN   0x0D
#define REG_CAL_PWM_BLUE    0x0E
#define REG_CURR_MAX_LVL    0x0F
#define REG_TEMP_LUT_TC1    0x10
#define REG_TEMP_LUT_TC2    0x11
#define REG_TEMP_LUT_TC3    0x12
#define REG_TEMP_LUT_TC4    0x13
#define REG_TEMP_LUT_TC5    0x14
#define REG_TEMP_LUT_TC6    0x15
#define REG_TEMP_LUT_TC7    0x16
#define REG_TEMP_LUT_TC8    0x17
#define REG_TEMP_LUT_TC9    0x18
#define REG_TEMP_LUT_TC10   0x19
#define REG_TIMEOUT         0x1A
#define REG_RGB             0x20

/* Write command */
#define SAL_REG_RESET               (WR | 0x31)
#define SAL_REG_INITBIDIR           (WR | 0x32)
#define SAL_REG_CLRERROR            (WR | 0x33)
#define SAL_REG_GOSLEEP             (WR | 0x34)
#define SAL_REG_GOACTIVE            (WR | 0x38)
#define SAL_REG_GODEEPSLEEP         (WR | 0x3C)
#define SAL_REG_SET_SETUP1          (WR | REG_SETUP1)
#define SAL_REG_SET_SETUP2          (WR | REG_SETUP2)
#define SAL_REG_SET_MCAST           (WR | REG_MCAST)
#define SAL_REG_SET_RGB             (WR | REG_RGB)
#define SAL_REG_SET_TEMPTH          (WR | REG_TEMPTH)
#define SAL_REG_SET_TEMPHYS         (WR | REG_TEMPHYS)
#define SAL_REG_SET_CURR_MAX_LVL    (WR | REG_CURR_MAX_LVL)
#define SAL_REG_SET_TEMP_LUT_TC1    (WR | REG_TEMP_LUT_TC1)
#define SAL_REG_SET_TEMP_LUT_TC2    (WR | REG_TEMP_LUT_TC2)
#define SAL_REG_SET_TEMP_LUT_TC3    (WR | REG_TEMP_LUT_TC3)
#define SAL_REG_SET_TEMP_LUT_TC4    (WR | REG_TEMP_LUT_TC4)
#define SAL_REG_SET_TEMP_LUT_TC5    (WR | REG_TEMP_LUT_TC5)
#define SAL_REG_SET_TEMP_LUT_TC6    (WR | REG_TEMP_LUT_TC6)
#define SAL_REG_SET_TEMP_LUT_TC7    (WR | REG_TEMP_LUT_TC7)
#define SAL_REG_SET_TEMP_LUT_TC8    (WR | REG_TEMP_LUT_TC8)
#define SAL_REG_SET_TEMP_LUT_TC9    (WR | REG_TEMP_LUT_TC9)
#define SAL_REG_SET_TEMP_LUT_TC10   (WR | REG_TEMP_LUT_TC10)
#define SAL_REG_SET_TIMEOUT         (WR | REG_TIMEOUT)
/* SR(Status Request) command */
#define SAL_REG_SR_CLRERROR         (SR | 0x33)
#define SAL_REG_SR_GOSLEEP          (SR | 0x34)
#define SAL_REG_SR_GOACTIVE         (SR | 0x38)
#define SAL_REG_SR_GODEEPSLEEP      (SR | 0x3C)
#define SAL_REG_SR_SETUP1           (SR | REG_SETUP1)
#define SAL_REG_SR_SETUP2           (SR | REG_SETUP2)
#define SAL_REG_SR_MCAST            (SR | REG_MCAST)
#define SAL_REG_SR_RGB              (SR | REG_RGB)
#define SAL_REG_SR_TEMPTH           (SR | REG_TEMPTH)
#define SAL_REG_SR_TEMPHYS          (SR | REG_TEMPHYS)
#define SAL_REG_SR_CURR_MAX_LVL     (SR | REG_CURR_MAX_LVL)
#define SAL_REG_SR_TEMP_LUT_TC1     (SR | REG_TEMP_LUT_TC1)
#define SAL_REG_SR_TEMP_LUT_TC2     (SR | REG_TEMP_LUT_TC2)
#define SAL_REG_SR_TEMP_LUT_TC3     (SR | REG_TEMP_LUT_TC3)
#define SAL_REG_SR_TEMP_LUT_TC4     (SR | REG_TEMP_LUT_TC4)
#define SAL_REG_SR_TEMP_LUT_TC5     (SR | REG_TEMP_LUT_TC5)
#define SAL_REG_SR_TEMP_LUT_TC6     (SR | REG_TEMP_LUT_TC6)
#define SAL_REG_SR_TEMP_LUT_TC7     (SR | REG_TEMP_LUT_TC7)
#define SAL_REG_SR_TEMP_LUT_TC8     (SR | REG_TEMP_LUT_TC8)
#define SAL_REG_SR_TEMP_LUT_TC9     (SR | REG_TEMP_LUT_TC9)
#define SAL_REG_SR_TEMP_LUT_TC10    (SR | REG_TEMP_LUT_TC10)
#define SAL_REG_SR_TIMEOUT          (SR | REG_TIMEOUT)

/* Read command */
#define SAL_REG_READ_STATUS1        (RD | REG_STATUS1)
#define SAL_REG_READ_STATUS2        (RD | REG_STATUS2)
#define SAL_REG_READ_TEMP           (RD | REG_TEMP)
#define SAL_REG_READ_TEMPST         (RD | 0x04)
#define SAL_REG_READ_VEXT_TM        (RD | REG_VEXT_TM)
#define SAL_REG_READ_SETUP1         (RD | REG_SETUP1)
#define SAL_REG_READ_SETUP2         (RD | REG_SETUP2)
#define SAL_REG_READ_MCAST          (RD | REG_MCAST)
#define SAL_REG_READ_RGB            (RD | REG_RGB)
#define SAL_REG_READ_TEMPTH         (RD | REG_TEMPTH)
#define SAL_REG_READ_TEMPHYS        (RD | REG_TEMPHYS)
#define SAL_REG_READ_PWM_RED_VAL    (RD | REG_CAL_PWM_RED)
#define SAL_REG_READ_PWM_GREEN_VAL  (RD | REG_CAL_PWM_GREEN)
#define SAL_REG_READ_PWM_BLUE_VAL   (RD | REG_CAL_PWM_BLUE)
#define SAL_REG_READ_CURR_MAX_LVL   (RD | REG_CURR_MAX_LVL)
#define SAL_REG_READ_TEMP_LUT_TC1   (RD | REG_TEMP_LUT_TC1)
#define SAL_REG_READ_TEMP_LUT_TC2   (RD | REG_TEMP_LUT_TC2)
#define SAL_REG_READ_TEMP_LUT_TC3   (RD | REG_TEMP_LUT_TC3)
#define SAL_REG_READ_TEMP_LUT_TC4   (RD | REG_TEMP_LUT_TC4)
#define SAL_REG_READ_TEMP_LUT_TC5   (RD | REG_TEMP_LUT_TC5)
#define SAL_REG_READ_TEMP_LUT_TC6   (RD | REG_TEMP_LUT_TC6)
#define SAL_REG_READ_TEMP_LUT_TC7   (RD | REG_TEMP_LUT_TC7)
#define SAL_REG_READ_TEMP_LUT_TC8   (RD | REG_TEMP_LUT_TC8)
#define SAL_REG_READ_TEMP_LUT_TC9   (RD | REG_TEMP_LUT_TC9)
#define SAL_REG_READ_TEMP_LUT_TC10  (RD | REG_TEMP_LUT_TC10)
#define SAL_REG_READ_TIMEOUT        (RD | REG_TIMEOUT)

/******************  Bit definition for SETUP1  *****************/

#define SAL_SETUP1_FPWM_DIV_Pos     (0U)
#define SAL_SETUP1_FPWM_DIV_Msk     (0xFU << SAL_SETUP1_FPWM_DIV_Pos)            /*!< 0x00F */
#define SAL_SETUP1_FPWM_DIV         SAL_SETUP1_FPWM_DIV_Msk                      /*!<PWM Frequency divider. The PWM Frequency is activated in only active mode. FPWM = mclk / (F_PWM_DIV+1) */

#define SAL_SETUP1_LG_E_Pos         (4U)
#define SAL_SETUP1_LG_E_Msk         (0x1U << SAL_SETUP1_LG_E_Pos)                /*!< 0x010 */
#define SAL_SETUP1_LG_E             SAL_SETUP1_LG_E_Msk                          /*!<Low Grayscale PWM disable/enable. */

#define SAL_SETUP1_PH_SHIFT_E_Pos   (5U)
#define SAL_SETUP1_PH_SHIFT_E_Msk   (0x1U << SAL_SETUP1_PH_SHIFT_E_Pos)          /*!< 0x020 */
#define SAL_SETUP1_PH_SHIFT_E       SAL_SETUP1_PH_SHIFT_E_Msk                    /*!<G starts driving the sink-output at 0 degree, and then R, 90 degree and B, 270 degree, respectively. */

#define SAL_SETUP1_CRC_E_Pos        (6U)
#define SAL_SETUP1_CRC_E_Msk        (0x1U << SAL_SETUP1_CRC_E_Pos)               /*!< 0x040 */
#define SAL_SETUP1_CRC_E            SAL_SETUP1_CRC_E_Msk                         /*!<CRC disabled/enable. */

#define SAL_SETUP1_COM_FLT_E_Pos    (7U)
#define SAL_SETUP1_COM_FLT_E_Msk    (0x1U << SAL_SETUP1_COM_FLT_E_Pos)           /*!< 0x080 */
#define SAL_SETUP1_COM_FLT_E        SAL_SETUP1_COM_FLT_E_Msk                     /*!<Communication fault flag. */

#define SAL_SETUP1_UV_FLT_E_Pos     (8U)
#define SAL_SETUP1_UV_FLT_E_Msk     (0x1U << SAL_SETUP1_UV_FLT_E_Pos)            /*!< 0x100 */
#define SAL_SETUP1_UV_FLT_E         SAL_SETUP1_UV_FLT_E_Msk                      /*!<Undervoltage fault flag. */

#define SAL_SETUP1_OT_FLT_E_Pos     (9U)
#define SAL_SETUP1_OT_FLT_E_Msk     (0x1U << SAL_SETUP1_OT_FLT_E_Pos)            /*!< 0x200 */
#define SAL_SETUP1_OT_FLT_E         SAL_SETUP1_OT_FLT_E_Msk                      /*!<Overtemperature fault flag. */

#define SAL_SETUP1_OS_FLT_E_Pos     (10U)
#define SAL_SETUP1_OS_FLT_E_Msk     (0x1U << SAL_SETUP1_OS_FLT_E_Pos)            /*!< 0x400 */
#define SAL_SETUP1_OS_FLT_E         SAL_SETUP1_OS_FLT_E_Msk                      /*!<LED open/short fault flag. */

#define SAL_SETUP1_TC_FLT_E_Pos     (11U)
#define SAL_SETUP1_TC_FLT_E_Msk     (0x1U << SAL_SETUP1_TC_FLT_E_Pos )           /*!< 0x800 */
#define SAL_SETUP1_TC_FLT_E         SAL_SETUP1_TC_FLT_E_Msk                      /*!<RED LED temperature compensation disable/enable. */

/******************  Bit definition for SETUP2  *****************/
#define SAL_SETUP2_SH_LVL_Pos       (0U)
#define SAL_SETUP2_SH_LVL_Msk       (0x3U << SAL_SETUP2_SH_LVL_Pos)              /*!< 0x003 */
#define SAL_SETUP2_SH_LVL           SAL_SETUP2_SH_LVL_Msk                        /*!<LED Short detection level. 0 : VS_TH = 3.75V */
#define SAL_SETUP2_SH_LVL_0         (0x1UL << SAL_SETUP2_SH_LVL_Pos)             /*!< 0x0001, VS_TH = 4.00V */
#define SAL_SETUP2_SH_LVL_1         (0x2UL << SAL_SETUP2_SH_LVL_Pos)             /*!< 0x0002, VS_TH = 4.25V (default) */
#define SAL_SETUP2_SH_LVL_2         (0x3UL << SAL_SETUP2_SH_LVL_Pos)             /*!< 0x0003, VS_TH = 4.50V */

#define SAL_SETUP2_UV_LVL_Pos       (2U)
#define SAL_SETUP2_UV_LVL_Msk       (0x3U << SAL_SETUP2_UV_LVL_Pos)              /*!< 0x00C */
#define SAL_SETUP2_UV_LVL           SAL_SETUP2_UV_LVL_Msk                        /*!<UVLO detection level. 2'b00 : VUV_TH = 3.6V */
#define SAL_SETUP2_UV_LVL_0         (0x1UL << SAL_SETUP2_UV_LVL_Pos)             /*!< 0x0004, 2'b01 : VUV_TH = 3.8V */
#define SAL_SETUP2_UV_LVL_1         (0x2UL << SAL_SETUP2_UV_LVL_Pos)             /*!< 0x0008, 2'b10 : VUV_TH = 4.0V */
#define SAL_SETUP2_UV_LVL_2         (0x3UL << SAL_SETUP2_UV_LVL_Pos)             /*!< 0x000C, 2'b11 : VUV_TH = 4.2V (default) */

#define SAL_SETUP2_EVENT_CYC_Pos    (4U)
#define SAL_SETUP2_EVENT_CYC_Msk    (0x1U << SAL_SETUP2_EVENT_CYC_Pos )          /*!< 0x010 */
#define SAL_SETUP2_EVENT_CYC        SAL_SETUP2_EVENT_CYC_Msk                     /*!<This bit determines a period to detect fault events or ADC clock. */

#define SAL_SETUP2_VEXT_MON_E_Pos   (5U)
#define SAL_SETUP2_VEXT_MON_E_Msk   (0x1U << SAL_SETUP2_VEXT_MON_E_Pos )         /*!< 0x020 */
#define SAL_SETUP2_VEXT_MON_E       SAL_SETUP2_VEXT_MON_E_Msk                    /*!<EXT_TM pin voltage monitoring. */

#define SAL_SETUP2_CLK_INV_E_Pos    (6U)
#define SAL_SETUP2_CLK_INV_E_Msk    (0x1U << SAL_SETUP2_CLK_INV_E_Pos )          /*!< 0x040 */
#define SAL_SETUP2_CLK_INV_E        SAL_SETUP2_CLK_INV_E_Msk                     /*!<CLK polarity for MCU mode. */

#define SAL_SETUP2_DCLK_DIV_Pos     (7U)
#define SAL_SETUP2_DCLK_DIV_Msk     (0x1FFU << SAL_SETUP2_DCLK_DIV_Pos )        /*!< 0xF80 */
#define SAL_SETUP2_DCLK_DIV         SAL_SETUP2_DCLK_DIV_Msk                     /*!<DCLK divier ratio. eq = dec(DCLK_DIV[4:0])*64 + 64 */
#define SAL_SETUP2_DCLK_DIV_0       (0x00 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 64 */
#define SAL_SETUP2_DCLK_DIV_1       (0x01 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 128 */
#define SAL_SETUP2_DCLK_DIV_2       (0x02 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 192 */
#define SAL_SETUP2_DCLK_DIV_3       (0x03 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 256 */
#define SAL_SETUP2_DCLK_DIV_4       (0x04 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 320 */
#define SAL_SETUP2_DCLK_DIV_5       (0x05 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 384 */
#define SAL_SETUP2_DCLK_DIV_6       (0x06 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 448 */
#define SAL_SETUP2_DCLK_DIV_7       (0x07 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 512 */
#define SAL_SETUP2_DCLK_DIV_8       (0x08 << SAL_SETUP2_DCLK_DIV_Pos)           /*!<divide by 576 */

/******************  Bit definition for MCAST  *****************/
#define SAL_MCAST_ADDR_Pos          (0U)
#define SAL_MCAST_ADDR_Msk          (0xFFFFU << SAL_MCAST_ADDR_Pos)              /*!< 0xFFFF */
#define SAL_MCAST_ADDR              SAL_MCAST_ADDR_Msk                           /*!<Multicast group address. */

/******************  Bit definition for RGB  *****************/
#define SAL_RGB_RDATA_Pos           (0U)
#define SAL_RGB_RDATA_Msk           (0xFFU << SAL_RGB_RDATA_Pos)                 /*!< 0xFF */
#define SAL_RGB_RDATA               SAL_RGB_RDATA_Msk                            /*!<R_DATA. */

#define SAL_RGB_GDATA_Pos           (8U)
#define SAL_RGB_GDATA_Msk           (0xFFU << SAL_RGB_GDATA_Pos)                 /*!< 0xFF00 */
#define SAL_RGB_GDATA               SAL_RGB_GDATA_Msk                            /*!<G_DATA. */

#define SAL_RGB_BDATA_Pos           (16U)
#define SAL_RGB_BDATA_Msk           (0xFFU << SAL_RGB_BDATA_Pos)                 /*!< 0xFF0000 */
#define SAL_RGB_BDATA               SAL_RGB_BDATA_Msk                            /*!<B_DATA. */

/******************  Bit definition for TEMPTH  *****************/
#define SAL_TEMP_THRESHOLD_Pos      (0U)
#define SAL_TEMP_THRESHOLD_Msk      (0x3FFU << SAL_TEMP_THRESHOLD_Pos)           /*!< 0x3FF */
#define SAL_TEMP_THRESHOLD          SAL_TEMP_THRESHOLD_Msk                       /*!<TEMP_THRESHOLD. */

/******************  Bit definition for TEMPHYS  *****************/
#define SAL_TEMP_HYSTERISIS_Pos     (0U)
#define SAL_TEMP_HYSTERISIS_Msk     (0x3FFU << SAL_TEMP_HYSTERISIS_Pos)          /*!< 0x3FF */
#define SAL_TEMP_HYSTERISIS         SAL_TEMP_HYSTERISIS_Msk                      /*!<TEMP_HYSTERISIS. */

/******************  Bit definition for PWM_R/G/B_VAL  *****************/
#define SAL_PWM_RGB_VAL_Pos         (0U)
#define SAL_PWM_RGB_VAL_Msk         (0xFFFU << SAL_PWM_RGB_VAL_Pos)              /*!< 0xFFF */
#define SAL_PWM_RGB_VAL             SAL_PWM_RGB_VAL_Msk                          /*!<PWM_R/G/B_VAL. */

/******************  Bit definition for CURR_MAX_LVL  *****************/
#define SAL_CURR_MAX_LVL_B_Pos      (0U)
#define SAL_CURR_MAX_LVL_B_Msk      (0xFU << SAL_CURR_MAX_LVL_B_Pos)             /*!< 0x00F */
#define SAL_CURR_MAX_LVL_B          SAL_CURR_MAX_LVL_B_Msk                       /*!<This determines the max current value of the output sink driver. */
#define SAL_CURR_MAX_LVL_B_0        (0x1UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b0001:  2.87mA */
#define SAL_CURR_MAX_LVL_B_1        (0x2UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b0010:  4.31mA */
#define SAL_CURR_MAX_LVL_B_2        (0x3UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b0011:  5.75mA */
#define SAL_CURR_MAX_LVL_B_3        (0x4UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b0100:  7.18mA */
#define SAL_CURR_MAX_LVL_B_4        (0x5UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b0101:  8.62mA */
#define SAL_CURR_MAX_LVL_B_5        (0x6UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b0110: 10.05mA */
#define SAL_CURR_MAX_LVL_B_6        (0x7UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b0111: 10.48mA */
#define SAL_CURR_MAX_LVL_B_7        (0x8UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b1000: 12.92mA */
#define SAL_CURR_MAX_LVL_B_8        (0x9UL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b1001: 14.35mA */
#define SAL_CURR_MAX_LVL_B_9        (0xAUL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b1010: 15.78mA */
#define SAL_CURR_MAX_LVL_B_10       (0xBUL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b1011: 17.21mA */
#define SAL_CURR_MAX_LVL_B_11       (0xCUL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b1100: 18.65mA */
#define SAL_CURR_MAX_LVL_B_12       (0xDUL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b1101: 20.08mA */
#define SAL_CURR_MAX_LVL_B_13       (0xEUL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b1110: 21.51mA */
#define SAL_CURR_MAX_LVL_B_14       (0xFUL << SAL_CURR_MAX_LVL_B_Pos)            /*!< 4'b1111: 22.94mA */

#define SAL_CURR_MAX_LVL_G_Pos      (4U)
#define SAL_CURR_MAX_LVL_G_Msk      (0xFU << SAL_CURR_MAX_LVL_G_Pos)             /*!< 0x0F0 */
#define SAL_CURR_MAX_LVL_G          SAL_CURR_MAX_LVL_G_Msk                       /*!<This determines the max current value of the output sink driver. */
#define SAL_CURR_MAX_LVL_G_0        (0x1UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b0001:  2.87mA */
#define SAL_CURR_MAX_LVL_G_1        (0x2UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b0010:  4.31mA */
#define SAL_CURR_MAX_LVL_G_2        (0x3UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b0011:  5.75mA */
#define SAL_CURR_MAX_LVL_G_3        (0x4UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b0100:  7.18mA */
#define SAL_CURR_MAX_LVL_G_4        (0x5UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b0101:  8.62mA */
#define SAL_CURR_MAX_LVL_G_5        (0x6UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b0110: 10.05mA */
#define SAL_CURR_MAX_LVL_G_6        (0x7UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b0111: 10.48mA */
#define SAL_CURR_MAX_LVL_G_7        (0x8UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b1000: 12.92mA */
#define SAL_CURR_MAX_LVL_G_8        (0x9UL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b1001: 14.35mA */
#define SAL_CURR_MAX_LVL_G_9        (0xAUL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b1010: 15.78mA */
#define SAL_CURR_MAX_LVL_G_10       (0xBUL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b1011: 17.21mA */
#define SAL_CURR_MAX_LVL_G_11       (0xCUL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b1100: 18.65mA */
#define SAL_CURR_MAX_LVL_G_12       (0xDUL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b1101: 20.08mA */
#define SAL_CURR_MAX_LVL_G_13       (0xEUL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b1110: 21.51mA */
#define SAL_CURR_MAX_LVL_G_14       (0xFUL << SAL_CURR_MAX_LVL_G_Pos)            /*!< 4'b1111: 22.94mA */

#define SAL_CURR_MAX_LVL_R_Pos      (8U)
#define SAL_CURR_MAX_LVL_R_Msk      (0xFU << SAL_CURR_MAX_LVL_R_Pos)             /*!< 0xF00 */
#define SAL_CURR_MAX_LVL_R          SAL_CURR_MAX_LVL_R_Msk                       /*!<This determines the max current value of the output sink driver. */
#define SAL_CURR_MAX_LVL_R_0        (0x1UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b0001:  2.87mA */
#define SAL_CURR_MAX_LVL_R_1        (0x2UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b0010:  4.31mA */
#define SAL_CURR_MAX_LVL_R_2        (0x3UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b0011:  5.75mA */
#define SAL_CURR_MAX_LVL_R_3        (0x4UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b0100:  7.18mA */
#define SAL_CURR_MAX_LVL_R_4        (0x5UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b0101:  8.62mA */
#define SAL_CURR_MAX_LVL_R_5        (0x6UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b0110: 10.05mA */
#define SAL_CURR_MAX_LVL_R_6        (0x7UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b0111: 10.48mA */
#define SAL_CURR_MAX_LVL_R_7        (0x8UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b1000: 12.92mA */
#define SAL_CURR_MAX_LVL_R_8        (0x9UL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b1001: 14.35mA */
#define SAL_CURR_MAX_LVL_R_9        (0xAUL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b1010: 15.78mA */
#define SAL_CURR_MAX_LVL_R_10       (0xBUL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b1011: 17.21mA */
#define SAL_CURR_MAX_LVL_R_11       (0xCUL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b1100: 18.65mA */
#define SAL_CURR_MAX_LVL_R_12       (0xDUL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b1101: 20.08mA */
#define SAL_CURR_MAX_LVL_R_13       (0xEUL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b1110: 21.51mA */
#define SAL_CURR_MAX_LVL_R_14       (0xFUL << SAL_CURR_MAX_LVL_R_Pos)            /*!< 4'b1111: 22.94mA */

/******************  Bit definition for TEMP_LUT_TCx  *****************/
#define SAL_TEMP_LUT_TCx_Pos        (0U)
#define SAL_TEMP_LUT_TCx_Msk        (0x3FFU << SAL_TEMP_LUT_TCx_Pos)             /*!< 0x3FF */
#define SAL_TEMP_LUT_TCx            SAL_TEMP_LUT_TCx_Msk                         /*!<TEMP_LUT_TCx. */

/******************  Bit definition for TIMEOUT  *****************/
#define SAL_TIMEOUT_Pos             (0U)
#define SAL_TIMEOUT_Msk             (0xFFFU << SAL_TIMEOUT_Pos)                  /*!< 0xFFF */
#define SAL_TIMEOUT                 SAL_TIMEOUT_Msk                              /*!<TIMEOUT. */

/******************  Bit definition for STATUS1  *****************/
#define SAL_STATUS1_SIO1_Pos        (0U)
#define SAL_STATUS1_SIO1_Msk        (0x3U << SAL_STATUS1_SIO1_Pos)               /*!< 0x003 */
#define SAL_STATUS1_SIO1            SAL_STATUS1_SIO1_Msk                         /*!<Communication mode of SIO1 */

#define SAL_STATUS1_SIO2_Pos        (2U)
#define SAL_STATUS1_SIO2_Msk        (0x3U << SAL_STATUS1_SIO2_Pos)               /*!< 0x00C */
#define SAL_STATUS1_SIO2            SAL_STATUS1_SIO2_Msk                         /*!<Communication mode of SIO2 */

#define SAL_STATUS1_IC_STATE_Pos    (4U)
#define SAL_STATUS1_IC_STATE_Msk    (0x7U << SAL_STATUS1_IC_STATE_Pos)           /*!< 0x070 */
#define SAL_STATUS1_IC_STATE        SAL_STATUS1_IC_STATE_Msk                     /*!<Represent current IC state. */
#define SAL_STATUS1_IC_STATE_0      (0x1UL << SAL_STATUS1_IC_STATE_Pos)          /*!< Uninitialized */
#define SAL_STATUS1_IC_STATE_1      (0x2UL << SAL_STATUS1_IC_STATE_Pos)          /*!< Sleep */
#define SAL_STATUS1_IC_STATE_2      (0x3UL << SAL_STATUS1_IC_STATE_Pos)          /*!< Active */
#define SAL_STATUS1_IC_STATE_3      (0x4UL << SAL_STATUS1_IC_STATE_Pos)          /*!< Deepsleep */

/******************  Bit definition for STATUS2  *****************/
#define SAL_STATUS2_COM_FLT_Pos     (0U)
#define SAL_STATUS2_COM_FLT_Msk     (0x1U << SAL_STATUS2_COM_FLT_Pos)               /*!< 0x001 */
#define SAL_STATUS2_COM_FLT         SAL_STATUS2_COM_FLT_Msk                         /*!<COM_FLT */

#define SAL_STATUS2_CRC_FLT_Pos     (1U)
#define SAL_STATUS2_CRC_FLT_Msk     (0x1U << SAL_STATUS2_CRC_FLT_Pos)               /*!< 0x002 */
#define SAL_STATUS2_CRC_FLT         SAL_STATUS2_CRC_FLT_Msk                         /*!<CRC_FLT */

#define SAL_STATUS2_UV_FLT_Pos      (2U)
#define SAL_STATUS2_UV_FLT_Msk      (0x1U << SAL_STATUS2_UV_FLT_Pos)                /*!< 0x004 */
#define SAL_STATUS2_UV_FLT          SAL_STATUS2_UV_FLT_Msk                          /*!<UV_FLT */

#define SAL_STATUS2_OT_FLT_Pos      (3U)
#define SAL_STATUS2_OT_FLT_Msk      (0x1U << SAL_STATUS2_OT_FLT_Pos)                /*!< 0x008 */
#define SAL_STATUS2_OT_FLT          SAL_STATUS2_OT_FLT_Msk                          /*!<OT_FLT */

#define SAL_STATUS2_SHORT_B_Pos     (4U)
#define SAL_STATUS2_SHORT_B_Msk     (0x1U << SAL_STATUS2_SHORT_B_Pos)               /*!< 0x010 */
#define SAL_STATUS2_SHORT_B         SAL_STATUS2_SHORT_B_Msk                         /*!<SHORT_B */

#define SAL_STATUS2_SHORT_G_Pos     (5U)
#define SAL_STATUS2_SHORT_G_Msk     (0x1U << SAL_STATUS2_SHORT_G_Pos)               /*!< 0x020 */
#define SAL_STATUS2_SHORT_G         SAL_STATUS2_SHORT_G_Msk                         /*!<SHORT_G */

#define SAL_STATUS2_SHORT_R_Pos     (6U)
#define SAL_STATUS2_SHORT_R_Msk     (0x1U << SAL_STATUS2_SHORT_R_Pos)               /*!< 0x040 */
#define SAL_STATUS2_SHORT_R         SAL_STATUS2_SHORT_R_Msk                         /*!<SHORT_R */

#define SAL_STATUS2_OPEN_B_Pos      (7U)
#define SAL_STATUS2_OPEN_B_Msk      (0x1U << SAL_STATUS2_OPEN_B_Pos)                /*!< 0x080 */
#define SAL_STATUS2_OPEN_B          SAL_STATUS2_OPEN_B_Msk                          /*!<OPEN_B */

#define SAL_STATUS2_OPEN_G_Pos      (8U)
#define SAL_STATUS2_OPEN_G_Msk      (0x1U << SAL_STATUS2_OPEN_G_Pos)                /*!< 0x100 */
#define SAL_STATUS2_OPEN_G          SAL_STATUS2_OPEN_G_Msk                          /*!<OPEN_G */

#define SAL_STATUS2_OPEN_R_Pos      (9U)
#define SAL_STATUS2_OPEN_R_Msk      (0x1U << SAL_STATUS2_OPEN_R_Pos)                /*!< 0x200 */
#define SAL_STATUS2_OPEN_R          SAL_STATUS2_OPEN_R_Msk                          /*!<OPEN_R */

#define SAL_STATUS2_T_OUT_Pos       (10U)
#define SAL_STATUS2_T_OUT_Msk       (0x1U << SAL_STATUS2_T_OUT_Pos)                 /*!< 0x400 */
#define SAL_STATUS2_T_OUT           SAL_STATUS2_T_OUT_Msk                           /*!<T_OUT */

#define SAL_STATUS2_OTPCRC_Pos      (11U)
#define SAL_STATUS2_OTPCRC_Msk      (0x1U << SAL_STATUS2_OTPCRC_Pos)                /*!< 0x800 */
#define SAL_STATUS2_OTPCRC          SAL_STATUS2_OTPCRC_Msk                          /*!<OTPCRC */

/******************  Bit definition for TEMP  *****************/
#define SAL_TEMPERATURE_Pos         (0U)
#define SAL_TEMPERATURE_Msk         (0x3FFU << SAL_TEMPERATURE_Pos)                 /*!< 0x3FF */
#define SAL_TEMPERATURE             SAL_TEMPERATURE_Msk                             /*!<TEMPERATURE. */

/******************  Bit definition for VEXT_TM  *****************/
#define SAL_VEXT_TM_Pos             (0U)
#define SAL_VEXT_TM_Msk             (0x3FFU << SAL_VEXT_TM_Pos)                     /*!< 0x3FF */
#define SAL_VEXT_TM                 SAL_VEXT_TM_Msk                                 /*!<VEXT_TM. */


#define NUMBER_OF_INTERFACES    1
#define NUMBER_OF_STRIPS_FLEXIO 1


typedef enum _SAL_CMD_LIST_
{
    LIST_RESET = 0,
    LIST_INITBIDIR,
    LIST_CLRERROR,
    LIST_GOSLEEP,
    LIST_GOACTIVE,
    LIST_GODEEPSLEEP,
    LIST_SET_SETUP1,
    LIST_SET_SETUP2,
    LIST_SET_MCAST,
    LIST_SET_RGB,
    LIST_SET_TEMPTH,
    LIST_SET_TEMPHYS,
    LIST_SET_CURR_MAX_LVL,
    LIST_SET_TEMP_LUT_TC1,
    LIST_SET_TEMP_LUT_TC2,
    LIST_SET_TEMP_LUT_TC3,
    LIST_SET_TEMP_LUT_TC4,
    LIST_SET_TEMP_LUT_TC5,
    LIST_SET_TEMP_LUT_TC6,
    LIST_SET_TEMP_LUT_TC7,
    LIST_SET_TEMP_LUT_TC8,
    LIST_SET_TEMP_LUT_TC9,
    LIST_SET_TEMP_LUT_TC10,
    LIST_SET_TIMEOUT,
    LIST_READ_STATUS1,
    LIST_READ_STATUS2,
    LIST_READ_TEMP,
    LIST_READ_TEMPST,
    LIST_READ_VEXT_TM,
    LIST_READ_SETUP1,
    LIST_READ_SETUP2,
    LIST_READ_MCAST,
    LIST_READ_RGB,
    LIST_READ_TEMPTH,
    LIST_READ_TEMPHYS,
    LIST_READ_PWM_RED_VAL,
    LIST_READ_PWM_GREEN_VAL,
    LIST_READ_PWM_BLUE_VAL,
    LIST_READ_CURR_MAX_LVL,
    LIST_READ_TEMP_LUT_TC1,
    LIST_READ_TEMP_LUT_TC2,
    LIST_READ_TEMP_LUT_TC3,
    LIST_READ_TEMP_LUT_TC4,
    LIST_READ_TEMP_LUT_TC5,
    LIST_READ_TEMP_LUT_TC6,
    LIST_READ_TEMP_LUT_TC7,
    LIST_READ_TEMP_LUT_TC8,
    LIST_READ_TEMP_LUT_TC9,
    LIST_READ_TEMP_LUT_TC10,
    LIST_READ_TIMEOUT,

    LIST_CMD_MAX,
}SAL_CMD_LIST_T;

typedef struct _SAL_CMD_T_
{
    uint8_t cmd;
    struct {
        uint8_t broadcast       : 1;    /* Broadcast possible */
        uint8_t                 : 1;
        uint8_t multicast       : 1;    /* Multicast possible */
        uint8_t                 : 1;
        uint8_t status_request  : 1;    /* Status request allowed(SR) */
        uint8_t                 : 3;
    };
    uint8_t request_bits;   /* Payload size (bits) */
    uint8_t response_bits;  /* Response payload size (bits) or SR */
    const char* str;
}SAL_CMD_T;

static const SAL_CMD_T gt_sal_cmd_item[LIST_CMD_MAX] =
{
    { SAL_REG_RESET,                { 1, 1, 0 },  0,  0, "RESET",               },
    { SAL_REG_INITBIDIR,            { 0, 0, 0 },  0, 12, "INITBIDIR",           },
    { SAL_REG_CLRERROR,             { 1, 1, 1 },  0, 12, "CLRERROR",            },
    { SAL_REG_GOSLEEP,              { 1, 1, 1 },  0, 12, "GOSLEEP",             },
    { SAL_REG_GOACTIVE,             { 1, 1, 1 },  0, 12, "GOACTIVE",            },
    { SAL_REG_GODEEPSLEEP,          { 1, 1, 1 },  0, 12, "GODEEPSLEEP",         },
    { SAL_REG_SET_SETUP1,           { 1, 1, 1 }, 12, 12, "SET_SETUP1",          },
    { SAL_REG_SET_SETUP2,           { 1, 1, 1 }, 12, 12, "SET_SETUP2",          },
    { SAL_REG_SET_MCAST,            { 0, 0, 1 }, 24, 12, "SET_MCAST",           },
    { SAL_REG_SET_RGB,              { 1, 1, 1 }, 24, 12, "SET_RGB",             },
    { SAL_REG_SET_TEMPTH,           { 1, 1, 1 }, 12, 12, "SET_TEMPTH",          },
    { SAL_REG_SET_TEMPHYS,          { 1, 1, 1 }, 12, 12, "SET_TEMPHYS",         },
    { SAL_REG_SET_CURR_MAX_LVL,     { 1, 1, 1 }, 12, 12, "SET_CURR_MAX_LVL",    },
    { SAL_REG_SET_TEMP_LUT_TC1,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC1",    },
    { SAL_REG_SET_TEMP_LUT_TC2,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC2",    },
    { SAL_REG_SET_TEMP_LUT_TC3,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC3",    },
    { SAL_REG_SET_TEMP_LUT_TC4,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC4",    },
    { SAL_REG_SET_TEMP_LUT_TC5,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC5",    },
    { SAL_REG_SET_TEMP_LUT_TC6,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC6",    },
    { SAL_REG_SET_TEMP_LUT_TC7,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC7",    },
    { SAL_REG_SET_TEMP_LUT_TC8,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC8",    },
    { SAL_REG_SET_TEMP_LUT_TC9,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC9",    },
    { SAL_REG_SET_TEMP_LUT_TC10,    { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC10",   },
    { SAL_REG_SET_TIMEOUT,          { 1, 1, 1 }, 12, 12, "SET_TIMEOUT",         },

    { SAL_REG_READ_STATUS1,         { 1, 0, 0 },  0, 12, "READ_STATUS1",        },
    { SAL_REG_READ_STATUS2,         { 1, 0, 0 },  0, 12, "READ_STATUS2",        },
    { SAL_REG_READ_TEMP,            { 1, 0, 0 },  0, 12, "READ_TEMP",           },
    { SAL_REG_READ_TEMPST,          { 1, 0, 0 },  0, 24, "READ_TEMPST",         },
    { SAL_REG_READ_VEXT_TM,         { 1, 0, 0 },  0, 12, "READ_VEXT_TM",        },
    { SAL_REG_READ_SETUP1,          { 1, 0, 0 },  0, 12, "READ_SETUP1",         },
    { SAL_REG_READ_SETUP2,          { 1, 0, 0 },  0, 12, "READ_SETUP2",         },
    { SAL_REG_READ_MCAST,           { 1, 0, 0 },  0, 24, "READ_MCAST",          },
    { SAL_REG_READ_RGB,             { 1, 0, 0 },  0, 24, "READ_RGB",            },
    { SAL_REG_READ_TEMPTH,          { 1, 0, 0 },  0, 12, "READ_TEMPTH",         },
    { SAL_REG_READ_TEMPHYS,         { 1, 0, 0 },  0, 12, "READ_TEMPHYS",        },
    { SAL_REG_READ_PWM_RED_VAL,     { 1, 0, 0 },  0, 12, "READ_PWM_RED_VAL",    },
    { SAL_REG_READ_PWM_GREEN_VAL,   { 1, 0, 0 },  0, 12, "READ_PWM_GREEN_VAL",  },
    { SAL_REG_READ_PWM_BLUE_VAL,    { 1, 0, 0 },  0, 12, "READ_PWM_BLUE_VAL",   },
    { SAL_REG_READ_CURR_MAX_LVL,    { 1, 0, 0 },  0, 12, "READ_CURR_MAX_LVL",   },
    { SAL_REG_READ_TEMP_LUT_TC1,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC1",   },
    { SAL_REG_READ_TEMP_LUT_TC2,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC2",   },
    { SAL_REG_READ_TEMP_LUT_TC3,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC3",   },
    { SAL_REG_READ_TEMP_LUT_TC4,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC4",   },
    { SAL_REG_READ_TEMP_LUT_TC5,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC5",   },
    { SAL_REG_READ_TEMP_LUT_TC6,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC6",   },
    { SAL_REG_READ_TEMP_LUT_TC7,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC7",   },
    { SAL_REG_READ_TEMP_LUT_TC8,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC8",   },
    { SAL_REG_READ_TEMP_LUT_TC9,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC9",   },
    { SAL_REG_READ_TEMP_LUT_TC10,   { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC10",  },
    { SAL_REG_READ_TIMEOUT,         { 1, 0, 0 },  0, 12, "READ_TIMEOUT",        },
};

//------------------- crc lookup table for polynom 0x2F --------------------//
static const uint8_t gn_crc8_LUT[256] =
{
    0x00, 0x2F, 0x5E, 0x71, 0xBC, 0x93, 0xE2, 0xCD, 0x57, 0x78, 0x09, 0x26, 0xEB, 0xC4, 0xB5, 0x9A,
    0xAE, 0x81, 0xF0, 0xDF, 0x12, 0x3D, 0x4C, 0x63, 0xF9, 0xD6, 0xA7, 0x88, 0x45, 0x6A, 0x1B, 0x34,
    0x73, 0x5C, 0x2D, 0x02, 0xCF, 0xE0, 0x91, 0xBE, 0x24, 0x0B, 0x7A, 0x55, 0x98, 0xB7, 0xC6, 0xE9,
    0xDD, 0xF2, 0x83, 0xAC, 0x61, 0x4E, 0x3F, 0x10, 0x8A, 0xA5, 0xD4, 0xFB, 0x36, 0x19, 0x68, 0x47,
    0xE6, 0xC9, 0xB8, 0x97, 0x5A, 0x75, 0x04, 0x2B, 0xB1, 0x9E, 0xEF, 0xC0, 0x0D, 0x22, 0x53, 0x7C,
    0x48, 0x67, 0x16, 0x39, 0xF4, 0xDB, 0xAA, 0x85, 0x1F, 0x30, 0x41, 0x6E, 0xA3, 0x8C, 0xFD, 0xD2,
    0x95, 0xBA, 0xCB, 0xE4, 0x29, 0x06, 0x77, 0x58, 0xC2, 0xED, 0x9C, 0xB3, 0x7E, 0x51, 0x20, 0x0F,
    0x3B, 0x14, 0x65, 0x4A, 0x87, 0xA8, 0xD9, 0xF6, 0x6C, 0x43, 0x32, 0x1D, 0xD0, 0xFF, 0x8E, 0xA1,
    0xE3, 0xCC, 0xBD, 0x92, 0x5F, 0x70, 0x01, 0x2E, 0xB4, 0x9B, 0xEA, 0xC5, 0x08, 0x27, 0x56, 0x79,
    0x4D, 0x62, 0x13, 0x3C, 0xF1, 0xDE, 0xAF, 0x80, 0x1A, 0x35, 0x44, 0x6B, 0xA6, 0x89, 0xF8, 0xD7,
    0x90, 0xBF, 0xCE, 0xE1, 0x2C, 0x03, 0x72, 0x5D, 0xC7, 0xE8, 0x99, 0xB6, 0x7B, 0x54, 0x25, 0x0A,
    0x3E, 0x11, 0x60, 0x4F, 0x82, 0xAD, 0xDC, 0xF3, 0x69, 0x46, 0x37, 0x18, 0xD5, 0xFA, 0x8B, 0xA4,
    0x05, 0x2A, 0x5B, 0x74, 0xB9, 0x96, 0xE7, 0xC8, 0x52, 0x7D, 0x0C, 0x23, 0xEE, 0xC1, 0xB0, 0x9F,
    0xAB, 0x84, 0xF5, 0xDA, 0x17, 0x38, 0x49, 0x66, 0xFC, 0xD3, 0xA2, 0x8D, 0x40, 0x6F, 0x1E, 0x31,
    0x76, 0x59, 0x28, 0x07, 0xCA, 0xE5, 0x94, 0xBB, 0x21, 0x0E, 0x7F, 0x50, 0x9D, 0xB2, 0xC3, 0xEC,
    0xD8, 0xF7, 0x86, 0xA9, 0x64, 0x4B, 0x3A, 0x15, 0x8F, 0xA0, 0xD1, 0xFE, 0x33, 0x1C, 0x6D, 0x42
};

//------------------- manchester encdoing lookup table --------------------//
static const uint16_t gn_manchester_LUT[256] =
{
    0xAAAA, 0xAAA9, 0xAAA6, 0xAAA5, 0xAA9A, 0xAA99, 0xAA96, 0xAA95,
    0xAA6A, 0xAA69, 0xAA66, 0xAA65, 0xAA5A, 0xAA59, 0xAA56, 0xAA55,
    0xA9AA, 0xA9A9, 0xA9A6, 0xA9A5, 0xA99A, 0xA999, 0xA996, 0xA995,
    0xA96A, 0xA969, 0xA966, 0xA965, 0xA95A, 0xA959, 0xA956, 0xA955,
    0xA6AA, 0xA6A9, 0xA6A6, 0xA6A5, 0xA69A, 0xA699, 0xA696, 0xA695,
    0xA66A, 0xA669, 0xA666, 0xA665, 0xA65A, 0xA659, 0xA656, 0xA655,
    0xA5AA, 0xA5A9, 0xA5A6, 0xA5A5, 0xA59A, 0xA599, 0xA596, 0xA595,
    0xA56A, 0xA569, 0xA566, 0xA565, 0xA55A, 0xA559, 0xA556, 0xA555,
    0x9AAA, 0x9AA9, 0x9AA6, 0x9AA5, 0x9A9A, 0x9A99, 0x9A96, 0x9A95,
    0x9A6A, 0x9A69, 0x9A66, 0x9A65, 0x9A5A, 0x9A59, 0x9A56, 0x9A55,
    0x99AA, 0x99A9, 0x99A6, 0x99A5, 0x999A, 0x9999, 0x9996, 0x9995,
    0x996A, 0x9969, 0x9966, 0x9965, 0x995A, 0x9959, 0x9956, 0x9955,
    0x96AA, 0x96A9, 0x96A6, 0x96A5, 0x969A, 0x9699, 0x9696, 0x9695,
    0x966A, 0x9669, 0x9666, 0x9665, 0x965A, 0x9659, 0x9656, 0x9655,
    0x95AA, 0x95A9, 0x95A6, 0x95A5, 0x959A, 0x9599, 0x9596, 0x9595,
    0x956A, 0x9569, 0x9566, 0x9565, 0x955A, 0x9559, 0x9556, 0x9555,
    0x6AAA, 0x6AA9, 0x6AA6, 0x6AA5, 0x6A9A, 0x6A99, 0x6A96, 0x6A95,
    0x6A6A, 0x6A69, 0x6A66, 0x6A65, 0x6A5A, 0x6A59, 0x6A56, 0x6A55,
    0x69AA, 0x69A9, 0x69A6, 0x69A5, 0x699A, 0x6999, 0x6996, 0x6995,
    0x696A, 0x6969, 0x6966, 0x6965, 0x695A, 0x6959, 0x6956, 0x6955,
    0x66AA, 0x66A9, 0x66A6, 0x66A5, 0x669A, 0x6699, 0x6696, 0x6695,
    0x666A, 0x6669, 0x6666, 0x6665, 0x665A, 0x6659, 0x6656, 0x6655,
    0x65AA, 0x65A9, 0x65A6, 0x65A5, 0x659A, 0x6599, 0x6596, 0x6595,
    0x656A, 0x6569, 0x6566, 0x6565, 0x655A, 0x6559, 0x6556, 0x6555,
    0x5AAA, 0x5AA9, 0x5AA6, 0x5AA5, 0x5A9A, 0x5A99, 0x5A96, 0x5A95,
    0x5A6A, 0x5A69, 0x5A66, 0x5A65, 0x5A5A, 0x5A59, 0x5A56, 0x5A55,
    0x59AA, 0x59A9, 0x59A6, 0x59A5, 0x599A, 0x5999, 0x5996, 0x5995,
    0x596A, 0x5969, 0x5966, 0x5965, 0x595A, 0x5959, 0x5956, 0x5955,
    0x56AA, 0x56A9, 0x56A6, 0x56A5, 0x569A, 0x5699, 0x5696, 0x5695,
    0x566A, 0x5669, 0x5666, 0x5665, 0x565A, 0x5659, 0x5656, 0x5655,
    0x55AA, 0x55A9, 0x55A6, 0x55A5, 0x559A, 0x5599, 0x5596, 0x5595,
    0x556A, 0x5569, 0x5566, 0x5565, 0x555A, 0x5559, 0x5556, 0x5555,
};

static uint8_t gn_use_crc;
static volatile uint8_t gn_tx_started;
static volatile uint8_t gn_rx_started;

static uint8_t gn_rx_packet_size[NUMBER_OF_INTERFACES];
static uint16_t gn_sal_chain_length[NUMBER_OF_INTERFACES];
static uint16_t gn_sal_chain_begin[NUMBER_OF_INTERFACES];
static uint16_t gn_sal_chain_curr[NUMBER_OF_INTERFACES];

static sal_rgb_t gt_sal_rgb_data[NUMBER_OF_INTERFACES][DEV_ADDR_UNICAST_MAX];     /* 12,234byte : 4078 * 3(r/g/b) */

static uint16_t gn_sal_param_setup[2];
static uint16_t gn_sal_param_current_max_level;
static uint32_t gn_sal_timeout[NUMBER_OF_INTERFACES];

static uint8_t gn_sal_read_request[NUMBER_OF_INTERFACES];
static uint8_t gn_sal_read_bits[NUMBER_OF_INTERFACES];

sal_ConfigType iseled1_InitConfig[NUMBER_OF_INTERFACES] =
{
    {
        .nrOfStrips = NUMBER_OF_STRIPS_FLEXIO,
        //sal_callback,
        //DIGLED_INTERFACE_FLEXIO,
        //pinConfigFlexIO,
        //timeoutChannelsFlexIO,
        //ISELED_USING_DMA,
        //15U,
        //14U,
    },
};

static uint32_t get_mcast_group(sal_MCAST_Group_t grp)
{
    uint32_t temp = 0;

    switch(grp)
    {
    case SAL_MCAST_GRP_01:
        temp = (0x0001U << 0U);
        break;
    case SAL_MCAST_GRP_02:
        temp = (0x0001U << 1U);
        break;
    case SAL_MCAST_GRP_03:
        temp = (0x0001U << 2U);
        break;
    case SAL_MCAST_GRP_04:
        temp = (0x0001U << 3U);
        break;
    case SAL_MCAST_GRP_05:
        temp = (0x0001U << 4U);
        break;
    case SAL_MCAST_GRP_06:
        temp = (0x0001U << 5U);
        break;
    case SAL_MCAST_GRP_07:
        temp = (0x0001U << 6U);
        break;
    case SAL_MCAST_GRP_08:
        temp = (0x0001U << 7U);
        break;
    case SAL_MCAST_GRP_09:
        temp = (0x0001U << 8U);
        break;
    case SAL_MCAST_GRP_10:
        temp = (0x0001U << 9U);
        break;
    case SAL_MCAST_GRP_11:
        temp = (0x0001U <<10U);
        break;
    case SAL_MCAST_GRP_12:
        temp = (0x0001U <<11U);
        break;
    case SAL_MCAST_GRP_13:
        temp = (0x0001U << 12U);
        break;
    case SAL_MCAST_GRP_14:
        temp = (0x0001U <<13U);
        break;
    case SAL_MCAST_GRP_15:
        temp = (0x0001U <<14U);
        break;
    case SAL_MCAST_GRP_16:
        temp = (0x0001U <<15U);
        break;
    default:
        break;
    }

    return temp;
}

static uint16_t get_sal_temperature(int16_t temp)   /* TODO : */
{
    return 0;
}

static uint8_t calculate_crc8(uint8_t* data, uint8_t size)
{
    uint8_t crc = 0;
    uint8_t* p = data;

    while(size) // Skip calculating the last byte since the last byte is crc.
    {
        crc ^= *p++;           //xor current crc with value of pointer
        crc = gn_crc8_LUT[crc];     //take new crc from lookup table

        --size;
    }

    return crc;
}

static uint8_t sal_make_stream_0bit(uint16_t addr, uint8_t cmd, uint8_t use_crc, uint8_t buffer[])
{
    buffer[0] = (PREAMBLE << 4) | ((addr >> 8) & 0x0F);
    buffer[1] = ((addr >> 0) & 0xFF);
    buffer[2] = cmd;
    if(use_crc) buffer[3] = calculate_crc8(buffer, 3);

    return (3 + use_crc);
}

static uint8_t sal_make_stream_12bit(uint16_t param, uint16_t addr, uint8_t cmd, uint8_t use_crc, uint8_t buffer[])
{
    buffer[0] = (PREAMBLE << 4) | ((addr >> 8) & 0x0F);
    buffer[1] = ((addr >> 0) & 0xFF);
    buffer[2] = cmd;
    buffer[3] = (param >> 4);
    buffer[4] = (param << 4) | DUMMY_BIT;
    if(use_crc) buffer[5] = calculate_crc8(buffer, 5);

    return (5 + use_crc);
}

static uint8_t sal_make_stream_24bit(uint32_t param, uint16_t addr, uint8_t cmd, uint8_t use_crc, uint8_t buffer[])
{
    buffer[0] = (PREAMBLE << 4) | ((addr >> 8) & 0x0F);
    buffer[1] = ((addr >> 0) & 0xFF);
    buffer[2] = cmd;
    buffer[3] = (param >> 16);
    buffer[4] = (param >>  8);
    buffer[5] = (param >>  0);
    if(use_crc) buffer[6] = calculate_crc8(buffer, 6);

    return (6 + use_crc);
}

static uint8_t sal_encode_manchester(uint8_t* in, uint8_t* out, uint8_t size)
{
    uint16_t lut = 0;

    /* manchester_encoding */
    for(uint8_t counter = 0 ; counter < size ; ++counter)
    {
        lut = gn_manchester_LUT[*in];
        *out++ = (lut >> 8);
        *out++ = (lut >> 0);

        ++in;
    }

    return (size * 2);
}

static uint8_t sal_encode_0bit_stream(uint16_t addr, uint8_t cmd, uint8_t buffer[])
{
    uint8_t temp[8] = {0, };
    uint8_t size = sal_make_stream_0bit(addr, cmd, gn_use_crc, temp);
    uint8_t encoded_size = sal_encode_manchester(temp, buffer, size);

    return encoded_size;
}

static uint8_t sal_encode_12bit_stream(uint32_t param, uint16_t addr, uint8_t cmd, uint8_t buffer[])
{
    uint8_t temp[8] = {0, };
    uint8_t size = sal_make_stream_12bit(param, addr, cmd, gn_use_crc, temp);
    uint8_t encoded_size = sal_encode_manchester(temp, buffer, size);

    return encoded_size;
}

static uint8_t sal_encode_24bit_stream(uint32_t param, uint16_t addr, uint8_t cmd, uint8_t buffer[])
{
    uint8_t temp[8] = {0, };
    uint8_t size = sal_make_stream_24bit(param, addr, cmd, gn_use_crc, temp);
    uint8_t encoded_size = sal_encode_manchester(temp, buffer, size);

    return encoded_size;
}

static void sal_write(uint8_t strip_num, uint8_t* p, uint8_t size)
{
    spi_write_dma(strip_num, p, size);
}

static void sal_read(uint8_t strip_num, uint8_t size)
{
    spi_read_dma(strip_num, size);

    gn_rx_packet_size[strip_num] = size;
}

static void sal_wait_tx_completed(uint8_t strip)
{
    while((gn_tx_started & (1U << strip)) == (1U << strip)) {};

    us_delay(8);  /* TBD : 8.5us */
}

static void sal_wait_rx_completed(uint8_t strip)
{
    while((gn_rx_started & (1U << strip)) == (1U << strip)) {};
}

sal_ReturnType sal_init_interface(uint8_t nrOfInterfaces, const sal_ConfigType *configStruct)
{
    sal_ReturnType ret = SALLED_OK;

    return ret;
}

sal_ReturnType sal_init_strip(const sal_InitType* ChainInitPtr, sal_ReadDataResultType* ChainInitResultPtr, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;

    gn_use_crc = 0; /* default crc setting after POR */

    gn_sal_chain_begin[strip_num] = ChainInitPtr->firstLedAdr;
    gn_sal_chain_curr[strip_num] = ChainInitPtr->firstLedAdr;

    sal_set_reset(DEV_ADDR_BROADCAST, strip_num);
    sal_wait_tx_completed(strip_num);

    us_delay(150);  /* TBD : 150us */

    sal_set_initbidir(gn_sal_chain_begin[strip_num], strip_num);
    sal_wait_tx_completed(strip_num);
    sal_read(strip_num, (5 + gn_use_crc));   /* w or w/o crc8 */
    sal_wait_rx_completed(strip_num);

    gn_sal_param_setup[0] = 0;
    if(ChainInitPtr->phaseShift == true)
    {
        gn_sal_param_setup[0] |= SAL_SETUP1_PH_SHIFT_E;
    }
    if(ChainInitPtr->crcEnable == true)
    {
        gn_sal_param_setup[0] |= SAL_SETUP1_CRC_E;
    }
    if(ChainInitPtr->tempCmpEnable == true)
    {
        gn_sal_param_setup[0] |= SAL_SETUP1_TC_FLT_E;
    }
    sal_set_setup1(gn_sal_param_setup[0], DEV_ADDR_BROADCAST, strip_num);
    sal_wait_tx_completed(strip_num);

    gn_use_crc = ((ChainInitPtr->crcEnable == true) ? 1 : 0);

    gn_sal_param_setup[1] = 0;
    gn_sal_param_setup[1] |= SAL_SETUP2_SH_LVL_1;
    gn_sal_param_setup[1] |= SAL_SETUP2_UV_LVL_2;
    gn_sal_param_setup[1] |= (0x10 << SAL_SETUP2_DCLK_DIV_Pos);     /* DCLK_DIV for B type, default value is 0x10(divide by 1088) */
    sal_set_setup2(gn_sal_param_setup[1], DEV_ADDR_BROADCAST, strip_num);
    sal_wait_tx_completed(strip_num);

    sal_set_temperature_threshold(105, DEV_ADDR_BROADCAST, strip_num);
    sal_set_temperature_hysterisis(95, DEV_ADDR_BROADCAST, strip_num);

    gn_sal_param_current_max_level = 0;
    gn_sal_param_current_max_level |= SAL_CURR_MAX_LVL_B_7;
    gn_sal_param_current_max_level |= SAL_CURR_MAX_LVL_G_7;
    gn_sal_param_current_max_level |= SAL_CURR_MAX_LVL_R_7;
    sal_set_current_max_level(gn_sal_param_current_max_level, DEV_ADDR_BROADCAST, strip_num);
    sal_wait_tx_completed(strip_num);

    sal_set_RGB(0, 0, 0, DEV_ADDR_BROADCAST, strip_num);
    sal_wait_tx_completed(strip_num);

    gn_sal_timeout[strip_num] = gn_sal_chain_length[strip_num] * 300;   /* TODO : */
    sal_set_timeout(gn_sal_timeout[strip_num], DEV_ADDR_BROADCAST, strip_num);

    ChainInitResultPtr->chainLength = gn_sal_chain_length[strip_num];
    ChainInitResultPtr->retData = NULL;

    return ret;
}

sal_ReturnType sal_set_mcast(sal_MCAST_Group_t grp, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;

    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_SET_MCAST;

        if((grp > SAL_MCAST_GRP_MAX) || (address == DEV_ADDR_BROADCAST && gt_sal_cmd_item[list].broadcast == 0) || (address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0))
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint32_t mcast_grp = get_mcast_group(grp);
            uint8_t size = sal_encode_24bit_stream(mcast_grp, address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_tx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_set_RGB(uint8_t red, uint8_t green, uint8_t blue, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_SET_RGB;

        uint8_t buffer[16] = {0, };
        uint8_t size = sal_encode_24bit_stream((red << 16 | green << 8 | blue << 0), address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_reset(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_RESET;

        uint8_t buffer[16] = {0, };
        uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_initbidir(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;

    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_INITBIDIR;

        if((address == DEV_ADDR_BROADCAST && gt_sal_cmd_item[list].broadcast == 0) || (address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0))
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_set_clear_error(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;

    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_CLRERROR;

        uint8_t buffer[16] = {0, };
        uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_power_mode(sal_PowerMode_t mode, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;

    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        if(mode < SAL_PWR_MAX)
        {
            SAL_CMD_LIST_T list = LIST_GOSLEEP;

            switch(mode)
            {
            case SAL_PWR_ACTIVE:
                list = LIST_GOACTIVE;
                break;
            case SAL_PWR_DEEPSLEEP:
                list = LIST_GODEEPSLEEP;
                break;
            default:
                list = LIST_GOSLEEP;
                break;
            }

            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_tx_started |= (1U << strip_num);
        }
        else
        {
            ret = SALLED_ERROR;
        }
    }

    return ret;
}

sal_ReturnType sal_set_setup1(uint16_t param, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_SET_SETUP1;

        uint8_t buffer[16] = {0, };
        uint8_t size = sal_encode_12bit_stream(param, address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_setup2(uint16_t param, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_SET_SETUP2;

        uint8_t buffer[16] = {0, };
        uint8_t size = sal_encode_12bit_stream(param, address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_temperature_threshold(int16_t temperature, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_SET_TEMPTH;

        uint8_t buffer[16] = {0, };
        uint16_t param = get_sal_temperature(temperature);
        uint8_t size = sal_encode_12bit_stream(param, address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_temperature_hysterisis(int16_t temperature, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_SET_TEMPHYS;

        uint8_t buffer[16] = {0, };
        uint16_t param = get_sal_temperature(temperature);
        uint8_t size = sal_encode_12bit_stream(param, address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_current_max_level(uint16_t param, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_SET_CURR_MAX_LVL;

        uint8_t buffer[16] = {0, };
        uint8_t size = sal_encode_12bit_stream(param, address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_temperature_lut_tcx(sal_TempLutTC_t tc_num, uint16_t param, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = (SAL_CMD_LIST_T)(LIST_SET_TEMP_LUT_TC1 + (uint8_t)tc_num);

        uint8_t buffer[16] = {0, };
        uint8_t size = sal_encode_12bit_stream(param, address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

sal_ReturnType sal_set_timeout(uint32_t param, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_SET_TIMEOUT;

        uint8_t buffer[16] = {0, };
        uint8_t size = sal_encode_12bit_stream(param, address, gt_sal_cmd_item[list].cmd, buffer);
        sal_write(strip_num, buffer, size);

        gn_tx_started |= (1U << strip_num);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
/******************************* READ **************************** */
///////////////////////////////////////////////////////////////////////////////////////////
sal_ReturnType sal_get_status1(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_STATUS1;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_status2(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_STATUS2;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_temperature(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_TEMP;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_temperature_st(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_TEMPST;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_vext_tm(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_VEXT_TM;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_setup1(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_SETUP1;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_setup2(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_SETUP2;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_mcast(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_MCAST;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_rgb(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_RGB;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_temperature_threshold(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_TEMPTH;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_temperature_hysterisis(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_TEMPHYS;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_pwm_red(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_PWM_RED_VAL;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_pwm_green(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_PWM_GREEN_VAL;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_pwm_blue(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_PWM_BLUE_VAL;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_current_max_level(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_CURR_MAX_LVL;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_temperature_lut_tcx(sal_TempLutTC_t tc_num, uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = (SAL_CMD_LIST_T)(LIST_SET_TEMP_LUT_TC1 + (uint8_t)tc_num);

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

sal_ReturnType sal_get_timeout(uint16_t address, uint8_t strip_num)
{
    sal_ReturnType ret = SALLED_OK;
    if((gn_tx_started & (1U << strip_num)) == (1U << strip_num))
    {
        ret = SALLED_BUSY;
    }
    else
    {
        SAL_CMD_LIST_T list = LIST_READ_TIMEOUT;

        if(address >= DEV_ADDR_MULTICAST_MIN && gt_sal_cmd_item[list].multicast == 0)
        {
            ret = SALLED_ERROR;
        }
        else
        {
            uint8_t buffer[16] = {0, };
            uint8_t size = sal_encode_0bit_stream(address, gt_sal_cmd_item[list].cmd, buffer);
            sal_write(strip_num, buffer, size);

            gn_sal_read_bits[strip_num] = gt_sal_cmd_item[list].response_bits;
            gn_sal_read_request[strip_num] = 0x01;
            if(DEV_ADDR_BROADCAST == address)
            {
                gn_sal_read_request[strip_num] |= 0x10;
            }

            gn_tx_started |= (1U << strip_num);
            gn_rx_started |= (1U << strip_num);
        }
    }

    return ret;
}

void sal_tx_completed(uint8_t strip_num)
{
    gn_tx_started &= ~(1U << strip_num);

    if(gn_sal_read_request[strip_num] & 0x01 == 0x01)
    {
        sal_read(strip_num, (((gn_sal_read_bits[strip_num] == 24 ? 7 : 6) + gn_use_crc)));
    }
}

void sal_rx_completed(uint8_t strip_num)
{
    if(gn_sal_read_request[strip_num] & 0x10 == 0x10)
    {
        ++gn_sal_chain_curr[strip_num];
        if(gn_sal_chain_curr[strip_num] >= gn_sal_chain_length[strip_num])
        {
            gn_sal_chain_curr[strip_num] = gn_sal_chain_begin[strip_num];

            gn_sal_read_request[strip_num] = 0;
            gn_rx_started &= ~(1U << strip_num);
        }

        sal_read(strip_num, (((gn_sal_read_bits[strip_num] == 24 ? 7 : 6) + gn_use_crc)));
    }
    else
    {
        gn_sal_read_request[strip_num] &= ~0x01;
        gn_rx_started &= ~(1U << strip_num);
    }
    sal_rx_parser(gn_spi_rx_buff[strip_num], gn_rx_packet_size[strip_num], strip_num);
}

uint32_t sal_rx_parser(uint8_t* in, uint8_t size, uint8_t strip_num)
{
    uint8_t preamble = (in[0] >> 4);
    uint32_t payload = 0;
    uint8_t crc8 = 0;

    if(preamble == PREAMBLE)
    {
        bool succeed = true;

        if(gb_sal_crc_en == true)
        {
            crc8 = calculate_crc8(in, (size - 1));
            if(crc8 != in[size - 1])
            {
                succeed = false;
            }
        }

        if(succeed == true)
        {
            uint16_t dev_address = (((in[0] & 0x0F) << 8) | (in[1] << 0));
            uint8_t command = in[2];

            switch(command)
            {
            case CMD_SAL_INITBIDIR:
                gn_sal_chain_length[strip_num] = dev_address;
                print(LOG_LV_INFO, "sal daisy num : %3u\r\n", dev_address);
                if (dev_address == 0 || dev_address >= 0xFEF)
                {
                    payload = ((in[3] << 16) | (in[4] << 8) | (in[5] << 0));    /* 24bit */
                }
                gb_sal_init_flag = true;
                break;
            case CMD_SAL_READ_TEMPST:
                payload = ((in[3] << 16) | (in[4] << 8) | (in[5] << 0));    /* 24bit */
                {
                    uint16_t temperature = ((payload >> 12) & 0xFFF);
                    uint16_t status2 = ((payload >>  0) & 0xFFF);
                }
                break;
            case CMD_SAL_READ_MCAST:
                payload = ((in[3] << 16) | (in[4] << 8) | (in[5] << 0));    /* 24bit */
                break;
            case CMD_SAL_READ_RGB:
                payload = ((in[3] << 16) | (in[4] << 8) | (in[5] << 0));    /* R/G/B */
                gt_sal_rgb_data[strip_num][dev_address - 1].r = in[3];
                gt_sal_rgb_data[strip_num][dev_address - 1].g = in[4];
                gt_sal_rgb_data[strip_num][dev_address - 1].b = in[5];
                break;
            case CMD_SAL_READ_STATUS2:
                payload = ((in[3] << 4) | (in[4] >> 4));
                {
                    uint16_t status2 = payload;
                }
                break;
            default:
                payload = ((in[3] << 4) | (in[4] >> 4));
                break;
            }

            if ((command & 0xC0) == 0xC0) // case SR_CMD
            {
                uint16_t temperature = 0;
                uint16_t status2 = 0;
                payload = ((in[3] << 16) | (in[4] << 8) | (in[5] << 0));    /* 24bit */
                {
                    temperature = ((payload >> 12) & 0xFFF);
                    status2 = ((payload >>  0) & 0xFFF);
                }

                if (!gb_sal_crc_en)
                {
                    print(LOG_LV_DEBUG, "rx : addr - %3u, cmd - 0x%02X, data - 0x%06X, temp - 0x%03X, status2 - 0x%03X\r\n", dev_address, command, payload, temperature, status2);
                }
                else
                {
                    print(LOG_LV_DEBUG, "rx : addr - %3u, cmd - 0x%02X, data - 0x%06X, crc - 0x%02X, temp - 0x%03X, status2 - 0x%03X\r\n", dev_address, command, payload, crc8, temperature, status2);
                }
            }
            else
            {
                if (!gb_sal_crc_en)
                {
                    #if 0
                    print(LOG_LV_DEBUG, "rx : addr - %3u, cmd - 0x%02X, data - 0x%06X \r\n", dev_address, command, payload);
                    #endif
                }
                else
                {
                    print(LOG_LV_DEBUG, "rx : addr - %3u, cmd - 0x%02X, data - 0x%06X, crc - 0x%02X\r\n", dev_address, command, payload, crc8);
                }
            }
        }
        else
        {
            print(LOG_LV_DEBUG, "[error] Invalid rx crc(%u)  : 0x%02X - 0x%02X \r\n", gb_sal_crc_en, crc8, in[size - 1]);
        }
    }
    else
    {
        print(LOG_LV_DEBUG, "[error] Invalid rx preamble  : 0x%02X \r\n", preamble);
    }

#if 0
    print(LOG_LV_DEBUG, "rx : 0x");
    for (uint8_t i = 0 ; i < size ; ++i)
    {
        print(LOG_LV_DEBUG, " %02X", in[i]);
    }
    print(LOG_LV_DEBUG, "\r\n");
#endif

    return payload;
}

sal_rgb_t* sal_get_rgb_data(uint8_t strip_num)
{
    return gt_sal_rgb_data[strip_num];
}

void display_command_list(void)
{
    for (uint8_t idx = 0 ; idx < LIST_CMD_MAX ; ++idx)
    {
        print(LOG_LV_INFO, "%18s - 0x%02X\r\n", gt_sal_cmd_item[idx].str, gt_sal_cmd_item[idx].cmd);
    }
}

uint16_t sal_get_chain_length(void)
{
    return gn_sal_chain_length[0];
}

void sal_set_chain_length(uint8_t length)
{
    gn_sal_chain_length[0] = length;
}

/*** end of file ***/
