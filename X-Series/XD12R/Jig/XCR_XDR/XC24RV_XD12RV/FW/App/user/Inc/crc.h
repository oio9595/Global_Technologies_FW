/* 
 * File:   crc.h
 * Author: GT
 *
 * Created on 2026. 05. 01.
 */

#ifndef CRC_H
#define	CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

uint16_t Calculate_CRC16_CCITT_False(const uint16_t *pData, const uint32_t length);

uint8_t Calculate_crc8_sae(const uint8_t *data, uint32_t length);
uint8_t Calculate_crc8_sae_bit_oriented(const uint8_t *data, uint32_t total_bits);

uint8_t Get_parity_uint32(uint32_t value, bool even_parity);

#ifdef __cplusplus
}
#endif

#endif
