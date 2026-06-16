/*
 * File:   drv_spi.c
 * Author: GT
 *
 * Created on 2026. 01. 16.
 */

#include "drv_spi.h"
#include <stdbool.h>

__STATIC_INLINE bool spi_timeout(uint32_t tick, const uint32_t timeout)
{
    if((HAL_GetTick() - tick) > timeout)
    {
        return false;
    }

    return true;
}

uint8_t spi_write(SPI_TypeDef *SPIx, const uint16_t* out, uint16_t len, const uint32_t timeout)
{
    uint32_t n_tick = HAL_GetTick();

    if(LL_SPI_IsEnabled(SPIx) != 1U)
    {
        LL_SPI_Enable(SPIx);
    }

    while(len--)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
        LL_SPI_TransmitData16(SPIx, *out++);
        n_tick = HAL_GetTick();
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx) == SET) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
    //us_delay(1U);
    user_delay(2U);

    return 0U;
}

uint8_t spi_read(SPI_TypeDef *SPIx, const uint16_t* out, uint16_t* in, uint16_t len, const uint32_t timeout)
{
    uint32_t n_tick = HAL_GetTick();

    if(LL_SPI_IsEnabled(SPIx) != 1U)
    {
        LL_SPI_Enable(SPIx);
    }

#if 1
    while(SET == LL_SPI_IsActiveFlag_RXNE(SPIx))
    {
        volatile uint16_t temp  = LL_SPI_ReceiveData16(SPIx);
        (void)temp;
    }
    LL_SPI_ClearFlag_OVR(SPIx);
#endif

    while(len--)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
        LL_SPI_TransmitData16(SPIx, *out++);
        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx)) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
        *in++ = LL_SPI_ReceiveData16(SPIx);
        n_tick = HAL_GetTick();
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx) == SET) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
    //us_delay(1U);
    user_delay(2U);

    return 0U;
}

uint8_t spi_write8(SPI_TypeDef *SPIx, const uint8_t* out, uint16_t len, const uint32_t timeout)
{
    uint32_t n_tick = HAL_GetTick();

    if(LL_SPI_IsEnabled(SPIx) != 1U)
    {
        LL_SPI_Enable(SPIx);
    }

    while(len--)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
        LL_SPI_TransmitData8(SPIx, *out++);
        n_tick = HAL_GetTick();
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx) == SET) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
    //us_delay(1U);
    user_delay(2U);

    return 0U;
}

uint8_t spi_read8(SPI_TypeDef *SPIx, const uint8_t* out, uint8_t* in, uint16_t len, const uint32_t timeout)
{
    uint32_t n_tick = HAL_GetTick();

    if(LL_SPI_IsEnabled(SPIx) != 1U)
    {
        LL_SPI_Enable(SPIx);
    }

#if 1
    while(SET == LL_SPI_IsActiveFlag_RXNE(SPIx))
    {
        volatile uint8_t temp  = LL_SPI_ReceiveData8(SPIx);
        (void)temp;
    }
    LL_SPI_ClearFlag_OVR(SPIx);
#endif

    while(len--)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
        LL_SPI_TransmitData8(SPIx, *out++);
        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx)) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
        *in++ = LL_SPI_ReceiveData8(SPIx);
        n_tick = HAL_GetTick();
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx) == SET) { if(false == spi_timeout(n_tick, timeout)) { return 1U; } };
    //us_delay(1U);
    user_delay(2U);

    return 0U;
}

