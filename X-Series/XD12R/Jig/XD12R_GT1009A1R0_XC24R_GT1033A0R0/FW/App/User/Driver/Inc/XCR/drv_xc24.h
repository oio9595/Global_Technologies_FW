/*
 * File:   drv_xc24.h
 * Author: GT
 * for XC24 & XD12 ES2
 * Created on 2026. 05. 07.
 */

#ifndef DRV_XC24_H
#define	DRV_XC24_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "xc24_struct.h"

#define XC_EFUSE_SKIP                       (0U)
#define XC_EFUSE_BURN                       (1U)
#define XC_EFUSE                            (XC_EFUSE_SKIP)

#define XC_CONV_FREQ_TO_XC_MCLK(Hz)         (uint32_t)(((float)XC_INTERNAL_MCLK) / (Hz) + 0.5f)
#define XC_CONV_US_TO_XC_MCLK(us)           (uint16_t)(((float)XC_INTERNAL_MCLK) / (1000000.0f / (us)) + 0.5f)
#define XC_CONV_DAC_V_TO_INPUT(voltage)     (uint16_t)((((voltage) * 4095.0f) / 3.0f) + 0.5f)

#define XC_CH_SIZE                          (1U)
#define XC_SEG_CH_SIZE       	            (8U)

#define MODEL_XC24_SEG                      ((uint16_t)((XC_CH_SIZE + XC_SEG_CH_SIZE - 1)  / XC_SEG_CH_SIZE))     /* make ceiled value */

#define XC_SERIAL_CLK_HIGH                  (24U)
#define XC_SERIAL_CLK_LOW                   (11U)
#define XC_SERIAL_CLK                       ((float)XC_INTERNAL_MCLK / (XC_SERIAL_CLK_HIGH + XC_SERIAL_CLK_LOW))

#define XC_SPI_RW_LEN    	                (48U)

typedef enum tag_XC_CH
{
    XC_CH_01 = 0U,
    XC_CH_02,
    XC_CH_03,
    XC_CH_04,
    XC_CH_05,
    XC_CH_06,
    XC_CH_07,
    XC_CH_08,
    XC_CH_09,
    XC_CH_10,
    XC_CH_11,
    XC_CH_12,
    XC_CH_13,
    XC_CH_14,
    XC_CH_15,
    XC_CH_16,
    XC_CH_17,
    XC_CH_18,
    XC_CH_19,
    XC_CH_20,
    XC_CH_21,
    XC_CH_22,
    XC_CH_23,
    XC_CH_24,
    XC_CH_SIZE_MAX,
} xc_ch_t;

typedef enum tag_XC_RW_GRP
{
    XC_RW_GRP1 = 0U,
    XC_RW_GRP2,
    XC_RW_GRP_MAX,
} xc_rw_grp_t;

extern volatile bool gb_xc_ld_transfer_spi_dma_flag;

void xc24_reset(void);

void xc24_read_all(void);
void xc24_init_param(void);

void xc24_init(void);
void xc24_trim_init(void);
void xc24_init_for_read(void);

const _xc_group1_regs_t* xc24_get_xc24_set_gr1_regs(void);
const _xc_group1_regs_t* xc24_get_xc24_get_gr1_regs(void);
const _xc_group2_regs_t* xc24_get_xc24_set_gr2_regs(void);
const _xc_group2_regs_t* xc24_get_xc24_get_gr2_regs(void);
//const _xd12_regs_t* xc24_get_xd12_set_regs(void);
//ret_xd12_regs xc24_get_xd12_get_regs(void);

uint16_t xc24_read_otp_control(uint16_t addr, uint16_t length);
void xc24_write_otp_control(uint16_t addr, const uint16_t* q, uint16_t length);

uint16_t xc24_read_grp1_reg(uint16_t addr, uint16_t length);
uint16_t xc24_read_grp2_reg(uint16_t addr, uint16_t length);
void xc24_write_grp1_reg(uint16_t addr, const uint16_t* q, uint16_t length);
void xc24_write_grp2_reg(uint16_t addr, const uint16_t* q, uint16_t length);

void xc24_set_ld_transfer(uint16_t* table, uint16_t length);

bool xc24_read_local(uint16_t ch_seg, uint16_t addr);
void xc24_write_local(uint16_t ch_seg, uint16_t addr, uint16_t* data, uint16_t len);

void xc24_get_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len);
void xc24_set_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len);

void xc24_set_fll_cnt(uint8_t fll_ch, uint32_t fll_cnt);

void xc24_nINT_FT_handler(void);
void xc24_nINT_LD_handler(void);

void xc24_trim_set_efuse_enable(bool en);
bool xc24_trim_get_efuse_enable(void);

void xc24_trim_init_1v5_ldo_dig(void);
void xc24_trim_init_dac_3v0(void);
void xc24_trim_init_dac1_ofs(void);
void xc24_trim_init_dac2_ofs(void);
void xc24_trim_init_dac3_ofs(void);
void xc24_trim_init_1v5_ldo_osc(void);
void xc24_trim_init_osc_a(void);
void xc24_trim_init_osc_b(void);

bool xc24_trim_set_1v5_ldo_dig(uint16_t reg_val);
bool xc24_trim_set_dac_3v0(uint16_t reg_val);
bool xc24_trim_set_dac1_ofs(uint16_t reg_val);
bool xc24_trim_set_dac2_ofs(uint16_t reg_val);
bool xc24_trim_set_dac3_ofs(uint16_t reg_val);
bool xc24_trim_set_1v5_ldo_osc(uint16_t reg_val);
bool xc24_trim_set_osc_a(uint16_t reg_val);
bool xc24_trim_set_osc_b(uint16_t reg_val);

void xc24_trim_init_efuse(void);
void xc24_trim_start_efuse(void);
void xc24_trim_save_mirror_register(void);
uint32_t xc24_trim_verify_mirror_dump(void);

void xc24_test_init_icc_stby(void);
void xc24_test_init_icc_actv(void);
void xc24_test_init_ldo(void);
void xc24_test_init_ldo_fll_a(void);
void xc24_test_init_ldo_fll_b(void);
void xc24_test_init_fll_a_30m(void);
void xc24_test_init_fll_a_35m(void);
void xc24_test_init_fll_a_40m(void);
void xc24_test_init_fll_b_30m(void);
void xc24_test_init_fll_b_35m(void);
void xc24_test_init_fll_b_40m(void);
void xc24_test_init_dac_p1(void);
void xc24_test_init_dac_p2(void);
void xc24_test_init_dac_p3(void);

void xc24_test_start_icc_stby(void);
void xc24_test_start_icc_actv(void);
void xc24_test_start_ldo(void);
void xc24_test_start_ldo_fll_a(void);
void xc24_test_start_ldo_fll_b(void);
void xc24_test_start_fll_a_30m(void);
void xc24_test_start_fll_a_35m(void);
void xc24_test_start_fll_a_40m(void);
void xc24_test_start_fll_b_30m(void);
void xc24_test_start_fll_b_35m(void);
void xc24_test_start_fll_b_40m(void);
void xc24_test_start_dac_p1(void);
void xc24_test_start_dac_p2(void);
void xc24_test_start_dac_p3(void);

void xc24_test_set_curr_tgt_dac(uint16_t curr_tgt_dac);

void xc24_test(void);

#ifdef __cplusplus
}
#endif

#endif	/* DRV_XC24_H */