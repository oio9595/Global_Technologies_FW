/*
 * File:   drv_spi.h
 * Author: GT
 *
 * Created on 2026. 01. 16.
 */

#ifndef DRV_SPI_H
#define	DRV_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define SPI_TIMEOUT 1U

uint8_t spi_write(SPI_TypeDef *SPIx, const uint16_t* out, uint16_t len, const uint32_t timeout);
uint8_t spi_read(SPI_TypeDef *SPIx, const uint16_t* out, uint16_t* in, uint16_t len, const uint32_t timeout);

uint8_t spi_write8(SPI_TypeDef *SPIx, const uint8_t* out, uint16_t len, const uint32_t timeout);
uint8_t spi_read8(SPI_TypeDef *SPIx, const uint8_t* out, uint8_t* in, uint16_t len, const uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif
