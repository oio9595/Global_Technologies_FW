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

#include "main.h"

#include "drv_spi.h"

#include "xcr24_struct.h"
#include "xdr12_struct.h"


#define XCR_LD_WIDTH            (LD_WIDTH3)
#define XCR_CH_SIZE             (24U)
#define XCR_SEG_CH_SIZE       	(8U)

#define MODEL_XCR24_SEG         ((uint16_t)((XCR_CH_SIZE + XCR_SEG_CH_SIZE - 1)  / XCR_SEG_CH_SIZE))     /* make ceiled value */

#define XCR_SERIAL_CLK_HIGH     (16U)
#define XCR_SERIAL_CLK_LOW      (8U)
#define XCR_SERIAL_CLK          ((float)MCLK_INTERNAL_OSC / (XCR_SERIAL_CLK_HIGH + XCR_SERIAL_CLK_LOW))

#define XCR_SPI_RW_LEN    	    (48U)

extern volatile bool gb_xcr_ld_transfer_spi_dma_flag;

void xcr_init_param(void);

void xcr24_init(void);

const _xcr_group1_regs_t* xcr24_get_xcr24_set_gr1_regs(void);
const _xcr_group1_regs_t* xcr24_get_xcr24_get_gr1_regs(void);
const _xcr_group2_regs_t* xcr24_get_xcr24_set_gr2_regs(void);
const _xcr_group2_regs_t* xcr24_get_xcr24_get_gr2_regs(void);
//const _xd12_regs_t* xcr24_get_xdr12_set_regs(void);
//ret_xdr12_regs xcr24_get_xdr12_get_regs(void);

void xcr24_get_otp_control(uint16_t addr, uint16_t len);
void xcr24_set_otp_control(uint16_t addr, const uint16_t* q, uint16_t len);

void xcr24_get_xcr24_gr1_reg(uint16_t addr, uint16_t len);
void xcr24_get_xcr24_gr2_reg(uint16_t addr, uint16_t len);
void xcr24_set_xcr24_gr1_reg(uint16_t addr, const uint16_t* q, uint16_t len);
void xcr24_set_xcr24_gr2_reg(uint16_t addr, const uint16_t* q, uint16_t len);

//void xcr24_get_xcr12_reg(uint16_t addr);
//void xcr24_set_xcr12_reg(uint16_t addr, uint16_t data);

void xcr24_set_ld_color(uint16_t red, uint16_t green, uint16_t blue);
void xcr24_set_ld_transfer(uint16_t* table, uint16_t length);

bool xcr24_read_local(uint16_t ch_seg, uint16_t addr);
void xcr24_write_local(uint16_t ch_seg, uint16_t addr, uint16_t* data, uint16_t len);

void xcr24_get_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len);
void xcr24_set_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif	/* DRV_XCR24_H */
