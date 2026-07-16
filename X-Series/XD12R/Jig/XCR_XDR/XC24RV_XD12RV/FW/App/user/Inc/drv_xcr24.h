/*
 * File:   drv_xcr24.h
 * Author: GT
 * for XCR24 & XDR12 ES2
 * Created on 2026. 05. 07.
 */

#ifndef DRV_XCR24_H
#define	DRV_XCR24_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "xcr24_struct.h"

#define XCR_CONV_FREQ_TO_XCR_MCLK(Hz)   (uint32_t)(((float)XCR_INTERNAL_MCLK) / (float)(Hz) + 0.5f)
#define XCR_CONV_us_TO_XCR_MCLK(us)     (uint16_t)(((float)XCR_INTERNAL_MCLK) / (1000000.0f / (float)(us)) + 0.5f)

#define XCR_EFUSE_SKIP      (0U)
#define XCR_EFUSE_BURN      (1U)
#define XCR_EFUSE           (XCR_EFUSE_SKIP)

#define XCR_CH_SIZE             (1U)
#define XCR_SEG_CH_SIZE       	(8U)

#define MODEL_XCR24_SEG         ((uint16_t)((XCR_CH_SIZE + XCR_SEG_CH_SIZE - 1)  / XCR_SEG_CH_SIZE))     /* make ceiled value */

#define XCR_SERIAL_CLK_HIGH     (24U)
#define XCR_SERIAL_CLK_LOW      (11U)
#define XCR_SERIAL_CLK          ((float)XCR_INTERNAL_MCLK / (XCR_SERIAL_CLK_HIGH + XCR_SERIAL_CLK_LOW))

#define XCR_SPI_RW_LEN    	    (48U)

extern volatile bool gb_xcr_ld_transfer_spi_dma_flag;

void xcr24_read_all(void);
void xcr24_init_param(void);

void xcr24_init(void);
void xcr24_trim_init(void);
void xcr24_test_init(void);

const _xcr_group1_regs_t* xcr24_get_xcr24_set_gr1_regs(void);
const _xcr_group1_regs_t* xcr24_get_xcr24_get_gr1_regs(void);
const _xcr_group2_regs_t* xcr24_get_xcr24_set_gr2_regs(void);
const _xcr_group2_regs_t* xcr24_get_xcr24_get_gr2_regs(void);
//const _xd12_regs_t* xcr24_get_xdr12_set_regs(void);
//ret_xdr12_regs xcr24_get_xdr12_get_regs(void);

uint16_t xcr24_read_otp_control(uint16_t addr, uint16_t length);
void xcr24_write_otp_control(uint16_t addr, const uint16_t* q, uint16_t length);

uint16_t xcr24_read_grp1_reg(uint16_t addr, uint16_t length);
uint16_t xcr24_read_grp2_reg(uint16_t addr, uint16_t length);
void xcr24_write_grp1_reg(uint16_t addr, const uint16_t* q, uint16_t length);
void xcr24_write_grp2_reg(uint16_t addr, const uint16_t* q, uint16_t length);

//void xcr24_get_xcr12_reg(uint16_t addr);
//void xcr24_set_xcr12_reg(uint16_t addr, uint16_t data);

void xcr24_set_ld_color(uint16_t red, uint16_t green, uint16_t blue);
void xcr24_set_ld_transfer(uint16_t* table, uint16_t length);

bool xcr24_read_local(uint16_t ch_seg, uint16_t addr);
void xcr24_write_local(uint16_t ch_seg, uint16_t addr, uint16_t* data, uint16_t len);

void xcr24_get_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len);
void xcr24_set_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len);

void xcr24_set_fll_cnt(uint8_t fll_ch, uint32_t fll_cnt);

void xcr24_trim_set_efuse_enable(bool en);
bool xcr24_trim_get_efuse_enable(void);

void xcr24_trim_init_1v5_ldo_dig(void);
void xcr24_trim_init_dac_3v0(void);
void xcr24_trim_init_dac1_ofs(void);
void xcr24_trim_init_dac2_ofs(void);
void xcr24_trim_init_dac3_ofs(void);
void xcr24_trim_init_1v5_ldo_osc(void);
void xcr24_trim_init_osc_a(void);
void xcr24_trim_init_osc_b(void);

bool xcr24_trim_set_1v5_ldo_dig(uint16_t reg_val);
bool xcr24_trim_set_dac_3v0(uint16_t reg_val);
bool xcr24_trim_set_dac1_ofs(uint16_t reg_val);
bool xcr24_trim_set_dac2_ofs(uint16_t reg_val);
bool xcr24_trim_set_dac3_ofs(uint16_t reg_val);
bool xcr24_trim_set_1v5_ldo_osc(uint16_t reg_val);
bool xcr24_trim_set_osc_a(uint16_t reg_val);
bool xcr24_trim_set_osc_b(uint16_t reg_val);

void xcr24_trim_init_efuse(void);
void xcr24_trim_start_efuse(void);
void xcr24_trim_save_mirror_register(void);
uint32_t xcr24_trim_verify_mirror_dump(void);

void xcr24_test_init_icc_stby(void);
void xcr24_test_init_icc_actv(void);
void xcr24_test_init_ldo(void);
void xcr24_test_init_ldo_fll_a(void);
void xcr24_test_init_ldo_fll_b(void);
void xcr24_test_init_fll_a_30m(void);
void xcr24_test_init_fll_a_35m(void);
void xcr24_test_init_fll_a_40m(void);
void xcr24_test_init_fll_b_30m(void);
void xcr24_test_init_fll_b_35m(void);
void xcr24_test_init_fll_b_40m(void);

void xcr24_test_start_icc_stby(void);
void xcr24_test_start_icc_actv(void);
void xcr24_test_start_ldo(void);
void xcr24_test_start_ldo_fll_a(void);
void xcr24_test_start_ldo_fll_b(void);
void xcr24_test_start_fll_a_30m(void);
void xcr24_test_start_fll_a_35m(void);
void xcr24_test_start_fll_a_40m(void);
void xcr24_test_start_fll_b_30m(void);
void xcr24_test_start_fll_b_35m(void);
void xcr24_test_start_fll_b_40m(void);

void xcr24_test_set_curr_tgt_dac(uint16_t curr_tgt_dac);

#ifdef __cplusplus
}
#endif

#endif	/* DRV_XCR24_H */
