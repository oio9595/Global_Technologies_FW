/*
 * File:   ldim_conversion.h
 * Author: GT
 * for XC24 & XD12 ES2
 * Created on 2026. 05. 14.
 */

#ifndef LDIM_CONVERSION_H
#define	LDIM_CONVERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "drv_xc24.h"
#include "drv_xd12.h"

typedef struct
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
} block_color_t;

#define LDIM_BLK_SIZE       (XC_CH_SIZE * (XD_DAISY_LENGTH * (BLOCK_PER_XD)))
#define LDIM_BLK_INDEX_ALL  (0U)

void ldim_block_map_init(void);
void ldim_block_map_print(void);

block_color_t* ldim_get_block_color_buffer(void);
void ldim_set_block_color_buffer(uint16_t index ,uint16_t red, uint16_t green, uint16_t blue);
void ldim_conversion_block_to_ldim(uint16_t block, uint16_t red, uint16_t green, uint16_t blue);

uint16_t* ldim_get_xc_ld_transfer_buffer(void);
uint16_t ldim_get_xc_ld_transfer_size(void);
uint16_t* ldim_get_xd_ld_transfer_buffer(void);
uint16_t ldim_get_xd_ld_transfer_size(void);

#ifdef __cplusplus
}
#endif

#endif	/* LDIM_CONVERSION_H */

