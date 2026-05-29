/* 
 * File:   ldim_conversion.h
 * Author: GT
 * for XCR24 & XDR12 ES2
 * Created on 2026. 05. 14.
 */

#ifndef LDIM_CONVERSION_H
#define	LDIM_CONVERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void ldim_set_ld_color(uint16_t red, uint16_t green, uint16_t blue);

uint16_t* ldim_get_xcr_ld_transfer_buffer(void);
uint16_t ldim_get_xcr_ld_transfer_size(void);
uint16_t* ldim_get_xdr_ld_transfer_buffer(void);
uint16_t ldim_get_xdr_ld_transfer_size(void);

#ifdef __cplusplus
}
#endif

#endif	/* LDIM_CONVERSION_H */

