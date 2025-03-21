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

static _xc24_regs_t _xc24_regs_;

static const char* gs_xc24_addr_str[XC24_ADDR_MAX + 32] = 
{
    "XC24_ADDR_SOFT_RESET", 
    "XC24_ADDR_GLOBAL_WRITE_CMD",
    "XC24_ADDR_LOCAL_WRITE_CMD",
    "XC24_ADDR_LOCAL_READ_CMD",
    "XC24_ADDR_ID_GEN_CMD",
    "XC24_ADDR_FAULT_READ_CMD",
    "XC24_ADDR_LD_TRANSFER_CMD",
    "XC24_ADDR_SYNC_GEN_CMD",
    "XC24_ADDR_AUTO_ENABLE_CMD",
    
    "DUMMY", 

    "XC24_ADDR_LD_WRITE_POINTER",
    "XC24_ADDR_LD_READ_POINTER",
    "XC24_ADDR_DIFFERENCE_POINTER",
    "XC24_ADDR_LD_TRANSFER_START_POINTER_TH",
    "XC24_ADDR_LOCAL_WR_TRANSFER_POINTER",
    "XC24_ADDR_LOCAL_RD_RECEIVE_POINTER",

    "XC24_ADDR_LOCAL_RW_DIFFERENCE_POINTER",
    "XC24_ADDR_LOCAL_RW_POINTER_RESET",
    "XC24_ADDR_FAULT_AUTO_READ_TIMER",
    "XC24_ADDR_FAULT_AUTO_READ_EVENT",
    "XC24_ADDR_SERIALIZER_CLOCK_GEN",
    "XC24_ADDR_INTERRUPT_ENABLE",
    "XC24_ADDR_COMMAND_STATUS1",
    "XC24_ADDR_COMMAND_STATUS2",
    "XC24_ADDR_RECEIVE_STATUS",
    "XC24_ADDR_INTERRUPT_STATUS",

    /* ========== ES1 ========= */
    "XC24_ADDR_FB_PWM_PERIOD",
    "XC24_ADDR_FB_PWM_DUTY",
    
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 

    "XC24_ADDR_COMMAND_LATENCY",

    "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1",
    "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE2",
    "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE3",
    "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE4",
    "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE5",
    "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE6",
    "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE7",
    "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE8",
    /* ========== ES1 ========= */

    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 

    "XC24_ADDR_DAISY_SIZE1",
    "XC24_ADDR_DAISY_SIZE2",
    "XC24_ADDR_DAISY_SIZE3",
    "XC24_ADDR_DAISY_SIZE4",
    "XC24_ADDR_DAISY_SIZE5",
    "XC24_ADDR_DAISY_SIZE6",
    "XC24_ADDR_DAISY_SIZE7",
    "XC24_ADDR_DAISY_SIZE8",
    "XC24_ADDR_BLOCK_SIZE1",
    "XC24_ADDR_BLOCK_SIZE2",
    "XC24_ADDR_BLOCK_SIZE3",
    "XC24_ADDR_BLOCK_SIZE4",
    "XC24_ADDR_BLOCK_SIZE5",
    "XC24_ADDR_BLOCK_SIZE6",
    "XC24_ADDR_BLOCK_SIZE7",
    "XC24_ADDR_BLOCK_SIZE8",
    "XC24_ADDR_BLOCK_SIZE9",
    "XC24_ADDR_BLOCK_SIZE10",
    "XC24_ADDR_BLOCK_SIZE11",
    "XC24_ADDR_BLOCK_SIZE12",

    "DUMMY", 

    "XC24_ADDR_CHANNEL_ENABLE1",
    "XC24_ADDR_CHANNEL_ENABLE2",
    
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    "DUMMY", 
    
    "XC24_ADDR_GLOBAL_WRITE_DATA",
    "XC24_ADDR_GLOBAL_FAULT_READ_DATA1",
    "XC24_ADDR_GLOBAL_FAULT_READ_DATA2",
    "XC24_ADDR_GLOBAL_FAULT_READ_DATA3",
    "XC24_ADDR_GLOBAL_FAULT_READ_DATA4",
    "XC24_ADDR_GLOBAL_FAULT_READ_DATA5",
    "XC24_ADDR_GLOBAL_FAULT_READ_DATA6",
};

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
    _xc24_cmd_t cmd_format;    
    uint16_t tx_buffer[2] = {0,};

    if(g_hSPIx == NULL)
    {
        g_hSPIx = SPI1;
    }

    cmd_format.ALL = 0;
    cmd_format.code = CMD_CODE_REG_WRITE;
    cmd_format.addr = in_addr;
    cmd_format.size = 1;
    
    tx_buffer[0] = cmd_format.ALL;
    tx_buffer[1] = in_data;

    print(LOG_DEBUG, " XC24_SPI_Write(0x%02X)(%s) - [%5u] [0x%4X] \r\n", in_addr, gs_xc24_addr_str[in_addr], in_data, in_data);

    spi_write(g_hSPIx, tx_buffer, 2);
}

uint16_t XC24_read_register(uint8_t in_addr)
{
    _xc24_cmd_t cmd_format;
    uint16_t tx_buffer[2] = {0, };
	uint16_t rx_buffer[2] = {0, };

    if(g_hSPIx == NULL)
    {
        g_hSPIx = SPI1;
    }
    
    cmd_format.ALL = 0;
    cmd_format.code = CMD_CODE_REG_READ;
    cmd_format.addr = in_addr;
    cmd_format.size = 1;
    tx_buffer[0] = cmd_format.ALL;

    spi_read(g_hSPIx, tx_buffer, rx_buffer, 2);
    *(_xc24_regs_.ALL + in_addr) = rx_buffer[1];
    
    // print(LOG_INFO, "XC24_read_register(0x%2X)(%s) - [%u] [0x%4X]\r\n", in_addr, gs_xc24_addr_str[in_addr], *(_xc24_regs_.ALL + in_addr), *(_xc24_regs_.ALL + in_addr));
    print(LOG_DEBUG, "XC24_read_register(0x%2X) - [%u] [0x%4X]\r\n", in_addr, *(_xc24_regs_.ALL + in_addr), *(_xc24_regs_.ALL + in_addr));

    return rx_buffer[1];
}

void XC24_read_register_all(void)
{
    uint16_t t_xc_reg;
    for(int i = 0 ; i < XC24_ADDR_MAX ; ++i)
    {
        t_xc_reg = XC24_read_register(i);
        print(LOG_INFO, "%s(0x%2X): 0x%4X (%4u)\r\n", gs_xc24_addr_str[i], i, t_xc_reg, t_xc_reg);
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
    _v_id_gen_t* p_v_id_gen = &_xc24_regs_._r04;

    /* XC24 -> XD12 Set ID GEN */
    p_v_id_gen->ALL = 0;
    p_v_id_gen->start = 1;
    XC24_write_register(XC24_ADDR_ID_GEN, p_v_id_gen->ALL);
    us_tdelay(XD12_IDGEN_DELAY);
}

void XC24_IF_SyncGen(void)
{
    _v_sync_gen_t* p_v_sync_gen = &_xc24_regs_._r07;

    /* XC24 -> XD12 Set Sync GEN */
    p_v_sync_gen->ALL = 0;
    p_v_sync_gen->start = 1;
    XC24_write_register(XC24_ADDR_SYNC_GEN, p_v_sync_gen->ALL);
    us_tdelay(XD12_IDGEN_DELAY);
}

void XC24_Initialize(void)
{
    g_hSPIx = SPI1;
    
    JigBd_IF_XC_VCC_EN(PWR_ON);    
    
    print(LOG_DEBUG, " ...XC24 Initial Start...\r\n");
    
    /* XC24 Reset */
    _v_soft_reset_t* p_v_soft_reset = &_xc24_regs_._r00;
    p_v_soft_reset->ALL = 0;
    p_v_soft_reset->rst1 = 1;
    p_v_soft_reset->rst2 = 1;
    p_v_soft_reset->rst3 = 1;
    p_v_soft_reset->vs_rst1 = 0;
    p_v_soft_reset->vs_rst2 = 0;
    XC24_write_register(XC24_ADDR_SOFT_RESET, p_v_soft_reset->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set LD trans start pointer & LD diff threshold */
    _v_ld_transfer_start_pointer_t* p_v_ld_transfer_start_pointer = &_xc24_regs_._r0D;
    p_v_ld_transfer_start_pointer->ALL = 0;
    p_v_ld_transfer_start_pointer->ld_trans_start_pointer = 4;
    p_v_ld_transfer_start_pointer->ld_diff_threshold = 4;
    XC24_write_register(XC24_ADDR_LD_TRANSFER_START_POINTER_TH, p_v_ld_transfer_start_pointer->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set RW Pointer Reset */
    _v_local_rw_pointer_reset_t* p_v_local_rw_pointer_reset = &_xc24_regs_._r11;
    p_v_local_rw_pointer_reset->ALL = 0;
    p_v_local_rw_pointer_reset->local_rd_pointer_rst = 1;
    p_v_local_rw_pointer_reset->local_wr_pointer_rst = 1;
    XC24_write_register(XC24_ADDR_LOCAL_RW_POINTER_RESET, p_v_local_rw_pointer_reset->ALL);
    us_tdelay(XD12_WRITE_DELAY);
    
    /* XC24 Set fault auto read timer */
    _v_fault_auto_read_timer_t* p_v_fault_auto_read_timer = &_xc24_regs_._r12;
    p_v_fault_auto_read_timer->ALL = 0;
    p_v_fault_auto_read_timer->fault_auto_rd_timer = 0xFFFF;
    XC24_write_register(XC24_ADDR_FAULT_AUTO_READ_TIMER, p_v_fault_auto_read_timer->ALL);
    us_tdelay(XD12_WRITE_DELAY);
    
    /* XC24 Set fault auto read event */
    _v_fault_auto_read_event_t* p_v_fault_auto_read_event = &_xc24_regs_._r13;
    p_v_fault_auto_read_event->ALL = 0;
    p_v_fault_auto_read_event->fault_auto_rd_interval = 1;
    p_v_fault_auto_read_event->fault_auto_rd_event = 1;
    XC24_write_register(XC24_ADDR_FAULT_AUTO_READ_EVENT, p_v_fault_auto_read_event->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set SERIAL CLK */
    _v_serializer_clock_gen_t* p_v_serializer_clock_gen = &_xc24_regs_._r14;
    p_v_serializer_clock_gen->ALL = 0;
    p_v_serializer_clock_gen->sck_low = 4;//SERIAL_CLK_CNT_LOW;
    p_v_serializer_clock_gen->sck_high = 8;//SERIAL_CLK_CNT_HIGH;
    XC24_write_register(XC24_ADDR_SERIALIZER_CLOCK_GEN, p_v_serializer_clock_gen->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set daised_dev_ch_size */
    _v_daisied_device_channel_size1_t* p_v_daisied_device_channel_size1 = &_xc24_regs_._r20;
    p_v_daisied_device_channel_size1->ALL = 0;
    p_v_daisied_device_channel_size1->daisied_dev_ch_size1 = XD_CH_SIZE;
    p_v_daisied_device_channel_size1->daisied_dev_ch_size2 = 0;
    p_v_daisied_device_channel_size1->daisied_dev_ch_size3 = 0;
    XC24_write_register(XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1, p_v_daisied_device_channel_size1->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set DAISY SIZE 1 */
    _v_daisy_size1_t* p_v_daisy_size1 = &_xc24_regs_._r30;
    p_v_daisy_size1->ALL = 0;
    p_v_daisy_size1->daisy_size_ch1 = XD_DAISY_SIZE;
    p_v_daisy_size1->daisy_size_ch2 = 0;
    p_v_daisy_size1->daisy_size_ch3 = 0;
    XC24_write_register(XC24_ADDR_DAISY_SIZE1, p_v_daisy_size1->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set BLOCK SIZE 1 */
    _v_block_size1_t* p_v_block_size1 = &_xc24_regs_._r38;
    p_v_block_size1->ALL = 0;
    p_v_block_size1->block_size_ch1 = XD_DAISY_SIZE * XD_CH_SIZE;
    p_v_block_size1->block_size_ch2 = 0;
    XC24_write_register(XC24_ADDR_BLOCK_SIZE1, p_v_block_size1->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set Channel Enable 1 */
    _v_channe_enablel_t* p_v_channe_enablel = &_xc24_regs_._r45;
    p_v_channe_enablel->ALL = 0;
    p_v_channe_enablel->ch1_en = 1;
    XC24_write_register(XC24_ADDR_CHANNEL_ENABLE1, p_v_channe_enablel->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set Channel Enable 2 */
    _v_channe_enable2_t* p_v_channe_enable2 = &_xc24_regs_._r46;
    p_v_channe_enable2->ALL = 0;
    p_v_channe_enable2->ch_size = 1;
    p_v_channe_enable2->ld_width = 3;
    XC24_write_register(XC24_ADDR_CHANNEL_ENABLE2, p_v_channe_enable2->ALL);
    us_tdelay(XD12_WRITE_DELAY);

    /* XC24 Set Internal LDO to 1.5V */
    XC24_write_register(XC24_ADDR_LDO, 5);
    us_tdelay(XD12_WRITE_DELAY);    

    _v_auto_enable_t* p_v_auto_enable = &_xc24_regs_._r08;
    p_v_auto_enable->ALL = 0;
    p_v_auto_enable->timeout_en = 1;
    XC24_write_register(XC24_ADDR_AUTO_ENABLE, p_v_auto_enable->ALL);
    us_tdelay(XD12_WRITE_DELAY);    
    
    print(LOG_DEBUG, " ...XC24 Initial Done...\r\n");
}

void XC24_IF_Write_XD12(uint8_t in_XD12_addr, uint16_t in_XD12_data)
{
    _v_global_write_t* p_v_global_write = &_xc24_regs_._r01;

    p_v_global_write->ALL = 0;
    p_v_global_write->start = 1;
    p_v_global_write->addr = in_XD12_addr;
    
    print(LOG_DEBUG, "\r\n========== XC -> XD Write Start (0x%02X) - [%u] [0x%4X] ==========\r\n", in_XD12_addr, in_XD12_data, in_XD12_data);
    
    /* 1st - Write the XD12_Data on the GLOBAL_WRITE_DATA Register of XC24 */
    XC24_write_register(XC24_ADDR_GLOBAL_WRITE_DATA, in_XD12_data);
    us_tdelay(1);

    /* 2nd - Write the XD12_Addr on the GLOBAL_WRITE_COMMAND Register of XC24 */
    XC24_write_register(XC24_ADDR_GLOBAL_WRITE, p_v_global_write->ALL);
    /* To Do : delay must more than 65us */
    us_tdelay(XD12_WRITE_DELAY);
    
    print(LOG_DEBUG, "========== XC -> XD  Write Done (0x%02X) - [%u] [0x%4X] ==========\r\n\r\n", in_XD12_addr, in_XD12_data, in_XD12_data);
}

uint16_t XC24_IF_Read_XD12(uint8_t in_XD12_addr)
{
    _v_local_rw_pointer_reset_t* p_v_local_rw_pointer_reset = &_xc24_regs_._r11;
    _v_local_read_t* p_v_local_read = &_xc24_regs_._r03;

    uint16_t u16_XD12_data = 0;
    uint16_t u16_XD12_r = 0;
    uint32_t count = 5;
    
    print(LOG_DEBUG, "\r\n========== XC -> XD Read Start (0x%02X) ==========\r\n", in_XD12_addr);
    
    /* 1st - Send Local RW Pointer Reset */
    p_v_local_rw_pointer_reset->ALL = 0;
    p_v_local_rw_pointer_reset->local_wr_pointer_rst = 1;
    p_v_local_rw_pointer_reset->local_rd_pointer_rst = 1;
    XC24_write_register(XC24_ADDR_LOCAL_RW_POINTER_RESET, p_v_local_rw_pointer_reset->ALL);

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
            print(LOG_DEBUG, "==============> LOCAL POINTER(%u) : 0x%04X <==============\r\n", count, u16_XD12_r);
            XC24_write_register(XC24_ADDR_LOCAL_RW_POINTER_RESET, p_v_local_rw_pointer_reset->ALL);
            us_tdelay(1);
        }
    }

    /* 2nd - Write the XD12_Addr on the LOCAL_READ Register of XC24 */
    p_v_local_read->ALL = 0;
    p_v_local_read->start = 1;
    p_v_local_read->ch_seg = 0;
    p_v_local_read->addr = in_XD12_addr;
    XC24_write_register(XC24_ADDR_LOCAL_READ, p_v_local_read->ALL);

    // Wait Ack
    us_tdelay(XD12_READ_DELAY + XD12_READ_RECV_DELAY);

    /* 3rd - Recieve Data XD12_Data through XC24 */
    u16_XD12_data = XC24_read_register(XC24_ADDR_PORT1_LOCAL_RW_DATA1);
    print(LOG_DEBUG, "XC24_IF_Read_XD12(0x%02X) - [%u] [0x%04X] \r\n",in_XD12_addr, u16_XD12_data, u16_XD12_data);

    print(LOG_DEBUG, "\r\n========== XC -> XD Read  Done (0x%02X) ==========\r\n", in_XD12_addr);

    return u16_XD12_data;
}

void XC24_IF_Write_LD(uint16_t in_LD_data)
{   
    _xc24_cmd_t cmd_format;    
    uint16_t tx_buffer[1 + XD_DAISY_SIZE * XD_CH_SIZE] = {0,};

    if(g_hSPIx == NULL)
    {
        g_hSPIx = SPI1;
    }

    cmd_format.ALL = 0;
    cmd_format.code = CMD_CODE_LD_TRANS;
    cmd_format.addr = 0;
    cmd_format.size = XD_DAISY_SIZE * XD_CH_SIZE;
    
    tx_buffer[0] = cmd_format.ALL;
    for (uint8_t i = 0 ; i < (XD_DAISY_SIZE * XD_CH_SIZE) ; ++i)
    {
        tx_buffer[i + 1] = in_LD_data;
    }

    spi_write(g_hSPIx, tx_buffer, 1 + XD_DAISY_SIZE * XD_CH_SIZE);
    // print(LOG_DEBUG, "XC24_IF_Write_LD(0x%02X)\r\n", in_LD_data);
}

/* END - INTERFACE FUNCTIONS ************************************************************************/

#ifdef DBG_TEST
void _dbg_XC24_Detect(void)
{
    uint16_t rtn = 0;

    g_hSPIx = SPI1;

    XC24_write_register(XC24_ADDR_GLOBAL_WRITE_DATA, 0xA5);
    rtn = XC24_read_register(XC24_ADDR_GLOBAL_WRITE_DATA);
    print(LOG_DEBUG, "_dbg_XC24_Detect() XC24_ADDR_GLOBAL_WRITE_DATA : 0x%x \r\n",rtn);
}
#endif //DBG_TEST

/*** end of file ***/

