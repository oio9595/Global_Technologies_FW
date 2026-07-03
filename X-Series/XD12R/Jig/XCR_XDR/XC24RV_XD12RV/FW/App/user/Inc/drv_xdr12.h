/*
 * File:   drv_xdr12.h
 * Author: GT
 * for XCR24 & XDR12 ES2
 * Created on 2026. 05. 07.
 */

#ifndef DRV_XDR12_H
#define	DRV_XDR12_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "drv_gpio.h"
#include "xdr12_struct.h"

#define XDR_CH_SIZE         (12U)

#define XDR_TYPE_A          (0U) /* NTS + TS */
#define XDR_TYPE_B          (1U) /* NTS + NTS + TS */
#define XDR_TYPE_C          (2U) /* NTS + TS + TS */
#define XDR_TYPE_D          (3U) /* TS + TS */
#define XDR_TYPE            XDR_TYPE_B

#if (XDR_TYPE == XDR_TYPE_A)
    #define LED_PER_BLOCK       (3U) /* NTS + TS */
    #define XDR_CH_PER_BLOCK    (2U)
    #define BLOCK_PER_XDR       (XDR_CH_SIZE / XDR_CH_PER_BLOCK) /* 6 blocks per XDR */
    #define XDR_LD_SIZE         (18U) /* TYPE-A; 3[NTS(1) + TS(2)] * 6set */
    typedef enum tag_COLOR_ORDER
    {
        COLOR_RED = 0U,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_ORDER_MAX,
    } color_order_t;
#elif (XDR_TYPE == XDR_TYPE_B)
    #define LED_PER_BLOCK       (4U) /* NTS + NTS + TS */
    #define XDR_CH_PER_BLOCK    (3U)
    #define BLOCK_PER_XDR       (XDR_CH_SIZE / XDR_CH_PER_BLOCK) /* 4 blocks per XDR */
    #define XDR_LD_SIZE         (LED_PER_BLOCK * BLOCK_PER_XDR) /* TYPE-B; 4[NTS(1) + NTS(1) + TS(2)] * 4set */
    typedef enum tag_COLOR_ORDER
    {
        COLOR_RED = 0U,
        COLOR_CYAN,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_ORDER_MAX,
    } color_order_t;
#elif (XDR_TYPE == XDR_TYPE_C)
    #define LED_PER_BLOCK       (5U) /* NTS + TS + TS */
    #define XDR_CH_PER_BLOCK    (3U)
    #define BLOCK_PER_XDR       (XDR_CH_SIZE / XDR_CH_PER_BLOCK) /* 4 blocks per XDR */
    #define XDR_LD_SIZE         (20U) /* TYPE-C; 5[NTS(1) + TS(2) + TS(2)] * 4set */
    typedef enum tag_COLOR_ORDER
    {
        COLOR_RED = 0U,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_CYAN,
        COLOR_MAGENTA,
        COLOR_ORDER_MAX,
    } color_order_t;
#elif (XDR_TYPE == XDR_TYPE_D)
    #define LED_PER_BLOCK       (4U) /* TS + TS */
    #define XDR_CH_PER_BLOCK    (2U)
    #define BLOCK_PER_XDR       (XDR_CH_SIZE / XDR_CH_PER_BLOCK) /* 4 blocks per XDR */
    #define XDR_LD_SIZE         (24U) /* TYPE-D; 4[TS(2) + TS(2)] * 6set */
    typedef enum tag_COLOR_ORDER
    {
        COLOR_GREEN = 0U,
        COLOR_BLUE,
        COLOR_ORDER_MAX,
    } color_order_t;
#else
    #error "XDR_TYPE is not defined"
#endif

#define XDR_DAISY_LENGTH            (1U)

//#define XDR_LD_DATA_12BIT           (12U)
#define XDR_LD_DATA_14BIT           (14U)
#define XDR_LD_DATA_BIT             XDR_LD_DATA_14BIT

#define XDR_SV_NO               	(32U)

#define XDR_SERIAL_CLK_HIGH     	(17U)
#define XDR_SERIAL_CLK_LOW      	(8U)
#define XDR_SERIAL_CLK          	((float)(XD12R_INTERNAL_MCLK / 2) / (XDR_SERIAL_CLK_HIGH + XDR_SERIAL_CLK_LOW))

typedef enum tag_XD12R_ADDR_TYPE
{
    XD12R_ADDR_TYPE_GENERAL = 0U,
    XD12R_ADDR_TYPE_MIRROR = 1U
} xd12r_addr_type_t;

extern volatile bool gb_xd_pwm_out_flag;
extern volatile bool gb_xd_pwm_in_flag;
extern volatile bool gb_xd_pwm_in_timeout;

void xdr12_init_param(void);

void xdr12_reset(void);
void xdr12_init(void);
void xdr12_trim_init(void);

void xdr12_write_by_type(uint16_t addr, uint16_t param, xd12r_addr_type_t addr_type);
uint16_t xdr12_read_by_type(uint16_t addr, xd12r_addr_type_t addr_type);

void xdr12_pwm_out_syncgen(void);
void xdr12_ld_transfer(uint16_t* p, uint16_t line_size);

void xdr12_trim_init_current_ref(void);
void xdr12_trim_init_ldo_dig(void);
void xdr12_trim_init_ldo_dac(void);
void xdr12_trim_init_ldo_fll(void);
void xdr12_trim_init_osc(void);
void xdr12_trim_init_ch_gain(void);
void xdr12_trim_init_ch_ofs(void);

void xdr12_trim_set_channel_enable(XD_CH_t chx);
void xdr12_trim_set_max_curr_vref(uint16_t vref);
void xdr12_trim_set_max_curr_lvl(uint16_t curr_lvl);

bool xdr12_trim_set_current_ref(uint16_t reg_val);
bool xdr12_trim_set_ldo_dig(uint16_t reg_val);
bool xdr12_trim_set_ldo_dac(uint16_t reg_val);
bool xdr12_trim_set_ldo_fll(uint16_t reg_val);
bool xdr12_trim_set_osc(uint16_t reg_val);
bool xdr12_trim_set_ch_gain(uint16_t reg_val, XD_CH_t chx);
bool xdr12_trim_set_ch_ofs(uint16_t reg_val, XD_CH_t chx);

void xdr12_trim_init_efuse(void);
void xdr12_trim_start_efuse(void);

void xdr12_trim_save_mirror_register(void);
uint32_t xdr12_trim_verify_mirror_dump(void);

void xdr12_test_init_icc_stby(void);
void xdr12_test_init_icc_actv(void);
void xdr12_test_init_current_ref(void);
void xdr12_test_init_ldo_dig(void);
void xdr12_test_init_ldo_dac(void);
void xdr12_test_init_ldo_fll(void);
void xdr12_test_init_osc(void);
void xdr12_test_init_fll_40M(void);
void xdr12_test_init_fll_50M(void);
void xdr12_test_init_fll_60M(void);
void xdr12_test_init_iout_3P(void);
void xdr12_test_init_max_sweep(void);

void xdr12_test_start_icc_stby(void);
void xdr12_test_start_icc_actv(void);
void xdr12_test_start_current_ref(void);
void xdr12_test_start_ldo_dig(void);
void xdr12_test_start_ldo_dac(void);
void xdr12_test_start_ldo_fll(void);
void xdr12_test_start_osc(void);
void xdr12_test_start_fll_40M(void);
void xdr12_test_start_fll_50M(void);
void xdr12_test_start_fll_60M(void);
void xdr12_test_start_iout_3P(void);
void xdr12_test_start_max_sweep(void);

#ifdef __cplusplus
}
#endif

#endif	/* DRV_XDR12_H */

