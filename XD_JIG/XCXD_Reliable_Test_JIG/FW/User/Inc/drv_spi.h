/*
 * File:   drv_spi.h
 * Author: GT
 *
 * Created on 2023, 04, 06
 */

#ifndef DRV_SPI_H
#define	DRV_SPI_H

#include "main.h"

__STATIC_INLINE void xc24_cs_low(void)
{
    XC_NSCS_LO();
}

__STATIC_INLINE void xc24_cs_high(void)
{
    XC_NSCS_HI();
}

__STATIC_INLINE void spi_write(uint16_t* p_buffer, uint16_t len, uint8_t daisy)
{
    xc24_cs_low();

    while(daisy)
    {
        for(volatile uint16_t i=0; i<len; i++)
        {
            while(RESET == LL_SPI_IsActiveFlag_TXE(SPI1)) {};
            LL_SPI_TransmitData16(SPI1, p_buffer[i]);
        }

        --daisy;
    }

    while(LL_SPI_IsActiveFlag_BSY(SPI1)) {};
    us_tdelay(4);

    xc24_cs_high();
}

__STATIC_INLINE void spi_read(uint16_t* p_tx_buffer, uint16_t* p_rx_buffer, uint16_t len)
{
    xc24_cs_low();

#if 1
    while(SET == LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
        volatile uint16_t temp  = LL_SPI_ReceiveData16(SPI1);
    }
#endif

    for(volatile uint16_t i=0; i<len; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPI1)) {};
        LL_SPI_TransmitData16(SPI1, p_tx_buffer[i]);

        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
        p_rx_buffer[i] = LL_SPI_ReceiveData16(SPI1);
    }

    while(LL_SPI_IsActiveFlag_BSY(SPI1)) {};
    us_tdelay(4);

    xc24_cs_high();
}

#endif	/* DRV_SPI_H */

