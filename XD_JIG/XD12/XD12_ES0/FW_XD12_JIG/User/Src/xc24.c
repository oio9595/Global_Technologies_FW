/** @file xd12.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XC24_C__
#include "config.h"

static SPI_TypeDef *g_hSPIx;

static bool gb_XC24_SUPPORT;

__STATIC_INLINE void user_delay(volatile uint32_t count)
{
    while(count)
    {
        --count;
    }
}

__STATIC_INLINE void spi_write(SPI_TypeDef *SPIx, uint16_t* p_buffer, uint16_t len)
{
    LL_GPIO_ResetOutputPin(NSCS_GPIO_Port, NSCS_Pin);

    if(LL_SPI_IsEnabled(SPIx) != 1)
    {
      /* Enable SPI peripheral */
        LL_SPI_Enable(SPIx);
    }

    for(volatile uint16_t i=0; i<len; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) {};
        LL_SPI_TransmitData16(SPIx, p_buffer[i]);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx)) {};
    user_delay(2);

    LL_GPIO_SetOutputPin(NSCS_GPIO_Port, NSCS_Pin);
}

__STATIC_INLINE void spi_read(SPI_TypeDef *SPIx, uint16_t* p_tx_buffer, uint16_t* p_rx_buffer, uint16_t len)
{
    LL_GPIO_ResetOutputPin(NSCS_GPIO_Port, NSCS_Pin);

#if 1
    if(LL_SPI_IsEnabled(SPIx) != 1)
    {
      /* Enable SPI peripheral */
        LL_SPI_Enable(SPIx);
    }

    while(SET == LL_SPI_IsActiveFlag_RXNE(SPIx))
    {
        volatile uint16_t temp  = LL_SPI_ReceiveData16(SPIx);
    }
#endif

    for(volatile uint16_t i=0; i<len; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) {};
        LL_SPI_TransmitData16(SPIx, p_tx_buffer[i]);

        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx)) {};
        p_rx_buffer[i] = LL_SPI_ReceiveData16(SPIx);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx)) {};
    //user_delay(2);

    LL_GPIO_SetOutputPin(NSCS_GPIO_Port, NSCS_Pin);
}

void XC24_write_register(uint16_t in_addr, uint16_t in_data)
{
    xc24_command_format cmd_format;    
    uint16_t tx_buffer[2] = {0,};

    if(g_hSPIx == NULL)
    {
        g_hSPIx = SPI1;
    }

    cmd_format.value = 0;
    cmd_format.u.command_code = XC24_COMMAND_CODE_REGISTER_WRITE;
    cmd_format.u.burst_size = 1;
    cmd_format.u.addr = in_addr;
    tx_buffer[0] = cmd_format.value;
    tx_buffer[1] = in_data;

    spi_write(g_hSPIx, tx_buffer, 2);
}

uint16_t XC24_read_register(uint8_t in_addr)
{
    xc24_command_format cmd_format;
    uint16_t tx_buffer[2] = {0, };
	uint16_t rx_buffer[2] = {0,};

    if(g_hSPIx == NULL)
    {
        g_hSPIx = SPI1;
    }
    
    cmd_format.value = 0;
    cmd_format.u.command_code = XC24_COMMAND_CODE_REGISTER_READ;
    cmd_format.u.addr = in_addr;
    cmd_format.u.burst_size = 1;
    tx_buffer[0] = cmd_format.value;

    spi_read(g_hSPIx, tx_buffer, rx_buffer, 2);

    // print( "XC24_read_register(0x%02x)-[0x%04x/%d] \r\n",in_addr, rx_buffer[1], rx_buffer[1]);

    return rx_buffer[1];
}

void XC24_Show_all()
{
    for(int i=0; i<XC24_ADDR_MAX; i++)
    {
        print( "XC24_REG[ 0x%02x: 0x%04x ] \r\n",i, XC24_read_register(i));
    }
}


/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/

bool IS_XC24(void)
{
    return gb_XC24_SUPPORT;
}

void USE_XC24(bool b_support)
{
    gb_XC24_SUPPORT = b_support;
}

void XC24_IF_IdGen(void)
{
    xc24_reg_idgen idgen;

    // Set ID GEN
    idgen.value = 0;
    idgen.u.enable = 1;
    XC24_write_register(XC24_ADDR_ID_GEN_COMMAND, idgen.value);
    us_tdelay(XD12_IDGEN_DELAY);
}

void XC24_IF_SyncGen(void)
{
    xc24_reg_sync_gen sync_gen;

    // Set Sync GEN
    sync_gen.value = 0;
    sync_gen.u.enable = 1;
    XC24_write_register(XC24_ADDR_SYNC_GEN_COMMAND, sync_gen.value);
    us_tdelay(XD12_IDGEN_DELAY);
}

void XC24_Initialize(void)
{
    xc24_reg_soft_reset soft_reset;
    xc24_reg_daisy_size daisy_size_1;
    xc24_reg_block_size block_size_1;
    xc24_reg_serializer_clock_gen serializer_clock_gen;
    xc24_reg_channel_enable_1 channel_enable_1;
    xc24_reg_channel_enable_2 channel_enable_2;

    g_hSPIx = SPI1;

    // Reset XC24
    soft_reset.value = 0;
    soft_reset.u.rst_1 = 1;
    soft_reset.u.rst_2 = 1;
    soft_reset.u.rst_3 = 1;
    soft_reset.u.vs_rst1 = 0;
    soft_reset.u.vs_rst2 = 0;
    XC24_write_register(XC24_ADDR_SOFT_RESET, soft_reset.value);
    us_tdelay(XD12_WRITE_DELAY);

    // Set DAISY SIZE 1 of XC24
    daisy_size_1.value = 0;
    daisy_size_1.u.daisy_size_ch1 = 1;
    daisy_size_1.u.daisy_size_ch2 = 0;
    daisy_size_1.u.daisy_size_ch3 = 0;
    XC24_write_register(XC24_ADDR_DAISY_SIZE1, daisy_size_1.value);
    us_tdelay(XD12_WRITE_DELAY);

    // Set BLOCK SIZE 1 of XC24
    block_size_1.value = 0;
    block_size_1.u.block_size_ch1 = 12;
    block_size_1.u.block_size_ch2 = 0;
    XC24_write_register(XC24_ADDR_BLOCK_SIZE1, block_size_1.value);
    us_tdelay(XD12_WRITE_DELAY);

    // Set CLK SIZE of XC24
    serializer_clock_gen.value = 0;
    serializer_clock_gen.u.serial_clk_low = SERIAL_CLK_CNT_LOW;
    serializer_clock_gen.u.serial_clk_high = SERIAL_CLK_CNT_HIGH;
    XC24_write_register(XC24_ADDR_SERIALIZER_CLOCK_GEN, serializer_clock_gen.value);
    us_tdelay(XD12_WRITE_DELAY);

    // Set daised_dev_ch_size of XC24
    XC24_write_register(XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE, 0x01);
    us_tdelay(XD12_WRITE_DELAY);

    // Set Channel 1 Enable of XC24
    channel_enable_1.value = 0;
    channel_enable_1.u.ch01_en = 1;
    XC24_write_register(XC24_ADDR_CHANNEL_ENABLE_1, channel_enable_1.value);
    us_tdelay(XD12_WRITE_DELAY);

    // Set Channel size of XC24
    channel_enable_2.value = 0;
    channel_enable_2.u.ch_size = 1;
    XC24_write_register(XC24_ADDR_CHANNEL_ENABLE_2, channel_enable_2.value);
    us_tdelay(XD12_WRITE_DELAY);
}

void XC24_IF_Write_XD12(uint8_t in_XD12_addr, uint16_t in_XD12_data)
{
    xc24_reg_write global_write;

    global_write.value = 0;
    global_write.u.enable = 1;
    global_write.u.addr = in_XD12_addr;
    
    // 1st -  Write the XD12_Data on the GLOBAL_WRITE_DATA Register of XC24
    XC24_write_register(XC24_ADDR_GLOBAL_WRITE_DATA, in_XD12_data);

    us_tdelay(1);

    // 2nd - Write the XD12_Addr on the GLOBAL_WRITE_COMMAND Register of XC24
    XC24_write_register(XC24_ADDR_GLOBAL_WRITE_COMMAND, global_write.value);
    // print( "XC24_IF_Write_XD12(0x%02x)-[0x%04x/%d] \r\n",in_XD12_addr, in_XD12_data, in_XD12_data);
    us_tdelay(XD12_WRITE_DELAY);
}

uint16_t XC24_IF_Read_XD12(uint8_t in_XD12_addr)
{
    xc24_reg_local_rw_pointer_rst ptr_rst;
    xc24_reg_read reg_read;
    uint16_t u16_XD12_data = 0;
    uint16_t u16_XD12_r = 0;
    uint32_t count = 5;

    // 1rd - Send Local RW Pointer Reset
    ptr_rst.value = 0;
    ptr_rst.u.pointer_rst1 = 1;
    ptr_rst.u.pointer_rst2 = 1;
    XC24_write_register(XC24_ADDR_LOCAL_RW_POINTER_RESET, ptr_rst.value);

    us_tdelay(1);

    while(count)
    {
        u16_XD12_r = XC24_read_register(XC24_ADDR_LOCAL_RD_RECEIVE_POINTER);

        --count;

        if(u16_XD12_r == 0)
        {
            us_tdelay(1);
            break;
        }
        else
        {
            print("==============> LOCAL POINTER(%u) : 0x%04X <==============\r\n",count, u16_XD12_r);
            XC24_write_register(XC24_ADDR_LOCAL_RW_POINTER_RESET, ptr_rst.value);
            us_tdelay(1);
        }
    }

    // 2nd - Write the XD12_Addr on the LOCAL_READ Register of XC24
    reg_read.value = 0;
    reg_read.u.enable = 1;
    reg_read.u.ch_seg = 0;
    reg_read.u.addr = in_XD12_addr;
    XC24_write_register(XC24_ADDR_LOCAL_READ, reg_read.value);

    // Wait Ack
    us_tdelay(XD12_READ_DELAY + XD12_READ_RECV_DELAY);

    // 3rd - Recieve Data XD12_Data through XC24
    u16_XD12_data = XC24_read_register(XC24_ADDR_PORT1_LOCAL_RW_DATA1);
    // print( "XC24_IF_Read_XD12(0x%02x)-[0x%04x/%d] \r\n",in_XD12_addr, u16_XD12_data, u16_XD12_data);

    return u16_XD12_data;
}

/* END - INTERFACE FUNCTIONS ************************************************************************/


#ifdef DBG_TEST
void _dbg_XC24_Detect(void)
{
    uint16_t rtn = 0;

    g_hSPIx = SPI1;

    XC24_write_register(XC24_ADDR_GLOBAL_WRITE_DATA, 0xa5);
    rtn = XC24_read_register(XC24_ADDR_GLOBAL_WRITE_DATA);
    print("_dbg_XC24_Detect() XC24_ADDR_GLOBAL_WRITE_DATA : 0x%x \r\n",rtn);
}
#endif //DBG_TEST

/*** end of file ***/

