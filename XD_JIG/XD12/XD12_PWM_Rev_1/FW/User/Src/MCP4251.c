/** @file MCP4251.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */

/* Also include device specific header */
#include "config.h"

/*
            2.5
    VLED = ------ * VRW[0]
            2.5
*/

/*
             40
    VLED = ------ * VRW[1]
            2.5
*/

void mcp_wiper_data_write(MCP_WIPER_PORT_T n_MCP_WIPER_PORT, uint16_t n_data);

__STATIC_INLINE void delay_us(volatile uint32_t count)
{
    while(count)
    {
        --count;
    }
}

__STATIC_INLINE void spi3_write(SPI_TypeDef *SPIx, uint8_t* p_buffer, uint16_t len)
{
    LL_GPIO_ResetOutputPin(MCP_SPI_CS_GPIO_Port, MCP_SPI_CS_Pin);

    if(LL_SPI_IsEnabled(SPIx) != 1)
    {
      /* Enable SPI peripheral */
        LL_SPI_Enable(SPIx);
    }

    for (volatile uint16_t i = 0 ; i < len ; ++i)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) {};
        LL_SPI_TransmitData8(SPIx, p_buffer[i]);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx)) {};
    delay_us(2);

    LL_GPIO_SetOutputPin(MCP_SPI_CS_GPIO_Port, MCP_SPI_CS_Pin);
}

__STATIC_INLINE void spi3_read(SPI_TypeDef *SPIx, uint8_t* p_tx_buffer, uint8_t* p_rx_buffer, uint16_t len)
{
    LL_GPIO_ResetOutputPin(MCP_SPI_CS_GPIO_Port, MCP_SPI_CS_Pin);

#if 1
    if(LL_SPI_IsEnabled(SPIx) != 1)
    {
      /* Enable SPI peripheral */
        LL_SPI_Enable(SPIx);
    }

    while(SET == LL_SPI_IsActiveFlag_RXNE(SPIx))
    {
        volatile uint8_t temp = LL_SPI_ReceiveData8(SPIx);
    }
#endif

    for (volatile uint16_t i = 0 ; i < len ; ++i)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) {};
        LL_SPI_TransmitData8(SPIx, p_tx_buffer[i]);

        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx)) {};
        p_rx_buffer[i] = LL_SPI_ReceiveData8(SPIx);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx)) {};
    delay_us(2);

    LL_GPIO_SetOutputPin(MCP_SPI_CS_GPIO_Port, MCP_SPI_CS_Pin);
}

void mcp_wiper_level_incr(MCP_WIPER_PORT_T n_MCP_WIPER_PORT)
{
    mcp_8_bit_cmd_t mcp_8_bit_cmd;

    mcp_8_bit_cmd.val = 0;
    if (n_MCP_WIPER_PORT == MCP_WIPER_PORT_0)
    {
        mcp_8_bit_cmd.addr = MCP_ADDR_WIPER_0;
    }
    else
    {
        mcp_8_bit_cmd.addr = MCP_ADDR_WIPER_1;
    }
    mcp_8_bit_cmd.cmd = MCP_CMD_INCR;
    mcp_8_bit_cmd.data = 0;

    spi3_write(SPI3, &mcp_8_bit_cmd.val, 1);
}

void mcp_wiper_level_decr(MCP_WIPER_PORT_T n_MCP_WIPER_PORT)
{
    mcp_8_bit_cmd_t mcp_8_bit_cmd;

    mcp_8_bit_cmd.val = 0;
    if (n_MCP_WIPER_PORT == MCP_WIPER_PORT_0)
    {
        mcp_8_bit_cmd.addr = MCP_ADDR_WIPER_0;
    }
    else
    {
        mcp_8_bit_cmd.addr = MCP_ADDR_WIPER_1;
    }
    mcp_8_bit_cmd.cmd = MCP_CMD_DECR;
    mcp_8_bit_cmd.data = 0;

    spi3_write(SPI3, &mcp_8_bit_cmd.val, 1);
}

void mcp_wiper_data_write(MCP_WIPER_PORT_T n_MCP_WIPER_PORT, uint16_t n_data)
{
    mcp_16_bit_cmd_t mcp_16_bit_cmd;

    mcp_16_bit_cmd.val_16 = 0;
    mcp_16_bit_cmd.cmd = MCP_CMD_WRITE;
    if (n_MCP_WIPER_PORT == MCP_WIPER_PORT_0)
    {
        mcp_16_bit_cmd.addr = MCP_ADDR_WIPER_0;
    }
    else
    {
        mcp_16_bit_cmd.addr = MCP_ADDR_WIPER_1;
    }
    mcp_16_bit_cmd.data_lsb = n_data;

    spi3_write(SPI3, mcp_16_bit_cmd.val, 2);
}

uint16_t mcp_wiper_data_read(MCP_WIPER_PORT_T n_MCP_WIPER_PORT)
{
    uint8_t rx_buff[2] = {0, };
    uint16_t ret = 0;
    mcp_16_bit_cmd_t mcp_16_bit_cmd;

    mcp_16_bit_cmd.val_16 = 0;
    mcp_16_bit_cmd.cmd = MCP_CMD_READ;
    if (n_MCP_WIPER_PORT == MCP_WIPER_PORT_0)
    {
        mcp_16_bit_cmd.addr = MCP_ADDR_WIPER_0;
    }
    else
    {
        mcp_16_bit_cmd.addr = MCP_ADDR_WIPER_1;
    }

    spi3_read(SPI3, mcp_16_bit_cmd.val, rx_buff, 2);
    print(LOG_DEBUG, "rx_buff[0] - [%u] [0x%X]\r\n", rx_buff[0]);
    print(LOG_DEBUG, "rx_buff[1] - [%u] [0x%X]\r\n", rx_buff[1]);

    ret = (((rx_buff[0] & 0x03) << 8)|(rx_buff[1]));

    return ret;
}

void mcp_wiper_init(void)
{
    mcp_set_wiper_resist_value(MCP_WIPER_PORT_0, 0);
    mcp_set_wiper_resist_value(MCP_WIPER_PORT_1, 0);
}

float mcp_set_wiper_resist_value(MCP_WIPER_PORT_T n_MCP_WIPER_PORT, uint16_t n_data)
{
    float f_r_wb = 0.0f;

    print(LOG_DEBUG, "\r\n mcp_set_wiper_resist_value [%u] - [%u]\r\n", n_MCP_WIPER_PORT, n_data);
    mcp_wiper_data_write(n_MCP_WIPER_PORT, n_data);
    f_r_wb = (MCP_R_AB / MCP_RES * n_data + MCP_R_W);

    return f_r_wb;
}

void mcp_debug_show_vled(void)
{
    float f_r_wb = 0.0f;
    float f_vled_fb = 0.0f;
    float f_vled_sh = 0.0f;

    for (uint16_t i = 0 ; i < 257 ; ++i)
    {
        mcp_wiper_data_write(MCP_WIPER_PORT_0, i);
        f_r_wb = (MCP_R_AB / MCP_RES * i + MCP_R_W);
        f_vled_fb = (5.0f/20000.0f) * f_r_wb;
        f_vled_sh = 16 * (5.0f/20000.0f) * f_r_wb;
        print(LOG_DEBUG, "\r\n data [%u] R_WB [%.3f] VLED_FB[%.3f] VLED_SHORT[%.3f]\r\n", i, f_r_wb, f_vled_fb, f_vled_sh);
    }
}

