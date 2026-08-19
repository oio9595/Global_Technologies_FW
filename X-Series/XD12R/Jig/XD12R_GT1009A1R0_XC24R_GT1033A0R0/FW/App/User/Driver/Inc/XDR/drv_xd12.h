/*
 * File:   drv_xd12.h
 * Author: GT
 * for XC24 & XD12 ES2
 * Created on 2026. 05. 07.
 */

#ifndef DRV_XD12_H
#define	DRV_XD12_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "drv_gpio.h"
#include "xd12_struct.h"

#define XD_EFUSE_SKIP           (0U)
#define XD_EFUSE_BURN           (1U)
#define XD_EFUSE                (XD_EFUSE_SKIP)

#define XD_CONTROLLED_MCU       (0U)
#define XD_CONTROLLED_XC24      (1U)
#define XD_CONTROL_TYPE         (XD_CONTROLLED_XC24)

#define XD_CH_SIZE              (12U)
#define XD_DAISY_LENGTH         (1U)

//#define XD_LD_DATA_12BIT      (12U)
#define XD_LD_DATA_14BIT        (14U)
#define XD_LD_DATA_BIT          (XD_LD_DATA_14BIT)

#define XD_SV_NO                (32U)

#define XD_SERIAL_CLK_HIGH      (24U)
#define XD_SERIAL_CLK_LOW       (12U)
#define XD_SERIAL_CLK           ((float)(XD12R_INTERNAL_MCLK / 2) / (XD_SERIAL_CLK_HIGH + XD_SERIAL_CLK_LOW))

#define XD_SYNC_MODE_CMD        (0U) /* cmd + svi */
#define XD_SYNC_MODE_SVI        (1U) /*  vo + svi */
#define XD_SYNC_MODE            (XD_SYNC_MODE_CMD)

#define XD_TYPE_A               (0U) /* NTS + TS */
#define XD_TYPE_B               (1U) /* NTS + NTS + TS */
#define XD_TYPE_C               (2U) /* NTS + TS + TS */
#define XD_TYPE_D               (3U) /* TS + TS */
#define XD_TYPE                 (XD_TYPE_A)

#if (XD_TYPE == XD_TYPE_A)
    #define LED_PER_BLOCK       (3U) /* NTS + TS */
    #define XD_CH_PER_BLOCK     (2U)
    #define BLOCK_PER_XD        (XD_CH_SIZE / XD_CH_PER_BLOCK) /* 6 blocks per XD */
    #define XD_LD_SIZE          (LED_PER_BLOCK * BLOCK_PER_XD) /* TYPE-A; 3[NTS(1) + TS(2)] * 6set */
    typedef enum tag_COLOR_ORDER
    {
        COLOR_RED = 0U,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_ORDER_MAX,
    } color_order_t;
#elif (XD_TYPE == XD_TYPE_B)
    #define LED_PER_BLOCK       (4U) /* NTS + NTS + TS */
    #define XD_CH_PER_BLOCK     (3U)
    #define BLOCK_PER_XD        (XD_CH_SIZE / XD_CH_PER_BLOCK) /* 4 blocks per XD */
    #define XD_LD_SIZE          (LED_PER_BLOCK * BLOCK_PER_XD) /* TYPE-B; 4[NTS(1) + NTS(1) + TS(2)] * 4set */
    typedef enum tag_COLOR_ORDER
    {
        COLOR_RED = 0U,
        COLOR_CYAN,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_ORDER_MAX,
    } color_order_t;
#elif (XD_TYPE == XD_TYPE_C)
    #define LED_PER_BLOCK       (5U) /* NTS + TS + TS */
    #define XD_CH_PER_BLOCK     (3U)
    #define BLOCK_PER_XD        (XD_CH_SIZE / XD_CH_PER_BLOCK) /* 4 blocks per XD */
    #define XD_LD_SIZE          (20U) /* TYPE-C; 5[NTS(1) + TS(2) + TS(2)] * 4set */
    typedef enum tag_COLOR_ORDER
    {
        COLOR_RED = 0U,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_CYAN,
        COLOR_MAGENTA,
        COLOR_ORDER_MAX,
    } color_order_t;
#elif (XD_TYPE == XD_TYPE_D)
    #define LED_PER_BLOCK       (4U) /* TS + TS */
    #define XD_CH_PER_BLOCK     (2U)
    #define BLOCK_PER_XD        (XD_CH_SIZE / XD_CH_PER_BLOCK) /* 4 blocks per XD */
    #define XD_LD_SIZE          (24U) /* TYPE-D; 4[TS(2) + TS(2)] * 6set */
    typedef enum tag_COLOR_ORDER
    {
        COLOR_GREEN = 0U,
        COLOR_BLUE,
        COLOR_ORDER_MAX,
    } color_order_t;
#else
    #error "XD_TYPE is not defined"
#endif

typedef enum tag_XD12R_ADDR_TYPE
{
    XD12R_ADDR_TYPE_GENERAL = 0U,
    XD12R_ADDR_TYPE_MIRROR = 1U
} xd12r_addr_type_t;

typedef enum tag_XD12R_SETTING_GRP
{
    XD12R_SETTING_GRP_ALL = 0U,
    XD12R_SETTING_GRP_1,
    XD12R_SETTING_GRP_2,
    XD12R_SETTING_GRP_3,
    XD12R_SETTING_GRP_4,
    XD12R_SETTING_GRP_5,
    XD12R_SETTING_GRP_MAX,
} xd12r_setting_grp_t;

extern volatile bool gb_xd_pwm_out_flag;
extern volatile bool gb_xd_pwm_in_flag;
extern volatile bool gb_xd_pwm_in_timeout;

void xd12_read_all(void);
void xd12_init_param(void);

void xd12_make_readable(void);
void xd12_reset(void);
void xd12_idgen(void);
void xd12_syncgen(void);
void xd12_init(void);
void xd12_trim_init(void);

void xd12_write_by_type(uint16_t addr, uint16_t param, xd12r_addr_type_t addr_type);
uint16_t xd12_read_by_type(uint16_t addr, xd12r_addr_type_t addr_type);

void xd12_ld_transfer(void);
void xd12_fault_readout(void);

void xd12_set_max_curr_vref(xd12r_setting_grp_t set_grp, uint16_t vref);
void xd12_set_max_curr_lvl(xd12r_setting_grp_t set_grp, max_curr_level_t curr_lvl);
void xd12_set_fb_lvl(xd12r_setting_grp_t set_grp, fb_level_t fb_lvl);
void xd12_set_short_lvl(xd12r_setting_grp_t set_grp, short_level_t short_lvl);

void xd12_trim_set_efuse_enable(bool en);
bool xd12_trim_get_efuse_enable(void);

void xd12_trim_init_current_ref(void);
void xd12_trim_init_ldo_dig(void);
void xd12_trim_init_ldo_dac(void);
void xd12_trim_init_ldo_fll(void);
void xd12_trim_init_osc(void);
void xd12_trim_init_ch_gain(void);
void xd12_trim_init_ch_ofs(void);

void xd12_trim_set_channel_enable(uint8_t chx);
void xd12_trim_set_max_curr_vref(uint16_t vref);
void xd12_trim_set_max_curr_lvl(uint16_t curr_lvl);

bool xd12_trim_set_current_ref(uint16_t reg_val);
bool xd12_trim_set_ldo_dig(uint16_t reg_val);
bool xd12_trim_set_ldo_dac(uint16_t reg_val);
bool xd12_trim_set_ldo_fll(uint16_t reg_val);
bool xd12_trim_set_osc(uint16_t reg_val);
bool xd12_trim_set_ch_gain(uint16_t reg_val, uint8_t chx);
bool xd12_trim_set_ch_ofs(uint16_t reg_val, uint8_t chx);

void xd12_trim_init_efuse(void);
void xd12_trim_start_efuse(void);

void xd12_trim_save_mirror_register(void);
uint16_t* xd12_get_trim_debug_reg(void);
uint32_t xd12_trim_verify_mirror_dump(void);

void xd12_test_init_icc_stby(void);
void xd12_test_init_icc_actv(void);
void xd12_test_init_current_ref(void);
void xd12_test_init_ldo_dig(void);
void xd12_test_init_ldo_dac(void);
void xd12_test_init_ldo_fll(void);
void xd12_test_init_osc(void);
void xd12_test_init_fll_40M(void);
void xd12_test_init_fll_50M(void);
void xd12_test_init_fll_60M(void);
void xd12_test_init_iout_3P(void);
void xd12_test_init_max_sweep(void);

void xd12_test_start_icc_stby(void);
void xd12_test_start_icc_actv(void);
void xd12_test_start_current_ref(void);
void xd12_test_start_ldo_dig(void);
void xd12_test_start_ldo_dac(void);
void xd12_test_start_ldo_fll(void);
void xd12_test_start_osc(void);
void xd12_test_start_fll_40M(void);
void xd12_test_start_fll_50M(void);
void xd12_test_start_fll_60M(void);
void xd12_test_start_iout_3P(void);
void xd12_test_start_max_sweep(void);

void xd12_test(void);

void xd12_aging_init_icc_test(void);
void xd12_aging_init_current_ref(void);
void xd12_aging_init_ldo_dig(void);
void xd12_aging_init_ldo_dac(void);
void xd12_aging_init_ldo_fll(void);
void xd12_aging_init_osc(void);
void xd12_aging_init_iout(void);

void xd12_aging_start_icc_test(void);
void xd12_aging_start_current_ref(void);
void xd12_aging_start_ldo_dig(void);
void xd12_aging_start_ldo_dac(void);
void xd12_aging_start_ldo_fll(void);
void xd12_aging_start_osc(void);
void xd12_aging_start_iout(void);
#ifdef __cplusplus
}
#endif

#endif	/* DRV_XD12_H */

