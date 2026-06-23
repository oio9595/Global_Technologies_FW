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

#define LD_TYPE                 	(2U)    /* R/G/B */

#define XDR_DAISY_LENGTH        	(1U)
#define XDR_CH_LENGTH           	(12U)
#define XD_LINE_LENGTH          	((XDR_DAISY_LENGTH * XDR_CH_LENGTH) / LD_TYPE)

#define MODEL_XDR_DAISY_SEG_SIZE    (XDR_DAISY_LENGTH * XCR_SEG_CH_SIZE)

#define XDR_SV_NO               	(32U)

#define XDR_SERIAL_CLK_HIGH     	(34U)
#define XDR_SERIAL_CLK_LOW      	(16U)
#define XDR_SERIAL_CLK          	((float)XD12R_INTERNAL_MCLK / (XDR_SERIAL_CLK_HIGH + XDR_SERIAL_CLK_LOW))

#define XDR_MAX_CURRENT_REF         (0x01F)
#define XDR_MAX_LDO_DIG             (0x00F)
#define XDR_MAX_LDO_DAC             (0x01F)
#define XDR_MAX_LDO_FLL             (0x00F)
#define XDR_MAX_OSC                 (0x01F)
#define XDR_MAX_CH_GAIN             (0x07F)
#define XDR_MAX_CH_OFS              (0x1FF)

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

#ifdef __cplusplus
}
#endif

#endif	/* DRV_XDR12_H */

