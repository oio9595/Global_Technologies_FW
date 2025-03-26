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

static bool gb_XC24_support;

static _xc24_regs_t gt_xc24_regs;
static _xc24_global_fault_t gt_xc24_fault;

static const char* gs_xc24_addr_str[XC24_ADDR_MAX] =
{
    "SOFT_RESET", //0x00
    "GLOBAL_WRITE",
    "LOCAL_WRITE",
    "LOCAL_READ",
    "ID_GEN",
    "FAULT_READ",
    "LD_TRANSFER",
    "SYNC_GEN",
    "CMD_AUTO_ENABLE",
    "DUMMY",
    "LD_WRITE_POINTER",
    "LD_READ_POINTER",
    "LD_DIFFERENCE_POINTER",
    "LD_TRANSFER_START_POINTER",
    "LOCAL_WR_TRANSFER_POINTER",
    "LOCAL_RD_RECEIVE_POINTER",

    "LOCAL_RW_DIFFERENCE_POINTER", //0x10
    "LOCAL_RW_POINTER_RESET",
    "FAULT_AUTO_READ_TIMER",
    "FAULT_AUTO_READ_EVENT",
    "SERIALIZER_CLOCK_GEN",
    "INTERRUPT_ENABLE",
    "COMMAND_STATUS1",
    "COMMAND_STATUS2",
    "RECEIVE_STATUS",
    "INTERRUPT_STATUS",
    "FB_PWM_PERIOD",

    "FB_PWM_DUTY_1",
    "FB_PWM_DUTY_2",
    "FB_PWM_DUTY_3",
    "DUMMY",
    "COMMAND_LATENCY",

    "DAISIED_DEVICE_CHANNEL_SIZE1", //0x20
    "DAISIED_DEVICE_CHANNEL_SIZE2",
    "DAISIED_DEVICE_CHANNEL_SIZE3",
    "DAISIED_DEVICE_CHANNEL_SIZE4",
    "DAISIED_DEVICE_CHANNEL_SIZE5",
    "DAISIED_DEVICE_CHANNEL_SIZE6",
    "DAISIED_DEVICE_CHANNEL_SIZE7",
    "DAISIED_DEVICE_CHANNEL_SIZE8",
    "DUMMY",
    "DUMMY",
    "LDO",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",

    "DAISY_SIZE1",
    "DAISY_SIZE2",
    "DAISY_SIZE3",
    "DAISY_SIZE4",
    "DAISY_SIZE5",
    "DAISY_SIZE6",
    "DAISY_SIZE7",
    "DAISY_SIZE8",

    "BLOCK_SIZE1",
    "BLOCK_SIZE2",
    "BLOCK_SIZE3",
    "BLOCK_SIZE4",
    "BLOCK_SIZE5",
    "BLOCK_SIZE6",
    "BLOCK_SIZE7",
    "BLOCK_SIZE8",

    "BLOCK_SIZE9",
    "BLOCK_SIZE10",
    "BLOCK_SIZE11",
    "BLOCK_SIZE12",
    "DUMMY",
    "CHANNEL_ENABLE1",
    "CHANNEL_ENABLE2",
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
    XC_NSCS_LO();

    if (LL_SPI_IsEnabled(SPIx) != 1)
    {
      /* Enable SPI peripheral */
        LL_SPI_Enable(SPIx);
    }

    for (volatile uint16_t i = 0 ; i < len ; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) {};
        LL_SPI_TransmitData16(SPIx, p_buffer[i]);
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx)) {};
    user_delay(2);

    XC_NSCS_HI();
}

__STATIC_INLINE void spi_read(SPI_TypeDef *SPIx, uint16_t* p_tx_buffer, uint16_t* p_rx_buffer, uint16_t len)
{
    XC_NSCS_LO();

#if 1
    if (LL_SPI_IsEnabled(SPIx) != 1)
    {
      /* Enable SPI peripheral */
        LL_SPI_Enable(SPIx);
    }

    while(SET == LL_SPI_IsActiveFlag_RXNE(SPIx))
    {
        volatile uint16_t temp  = LL_SPI_ReceiveData16(SPIx);
    }
#endif

    for (volatile uint16_t i = 0 ; i < len ; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) {};
        LL_SPI_TransmitData16(SPIx, p_tx_buffer[i]);

        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx)) {};
        p_rx_buffer[i] = LL_SPI_ReceiveData16(SPIx);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx)) {};
    //user_delay(2);

    XC_NSCS_HI();
}

void XC24_Write_Register(uint16_t in_addr, uint16_t in_data)
{
#if 1
    _xc24_cmd_t cmd_format;
    uint16_t tx_buffer[2] = {0,};

    if (g_hSPIx == NULL)
    {
        g_hSPIx = SPI1;
    }

    cmd_format.ALL = 0;
    cmd_format.code = CMD_CODE_REG_WRITE;
    cmd_format.addr = in_addr;
    cmd_format.size = 1;

    tx_buffer[0] = cmd_format.ALL;
    tx_buffer[1] = in_data;

    *(gt_xc24_regs.ALL + in_addr) = in_data;

    //print(LOG_DEBUG, " XC24_SPI_Write(0x%02X)(%s) - [%5u] [0x%4X] \r\n", in_addr, gs_xc24_addr_str[in_addr], in_data, in_data);
    //print(LOG_DEBUG, " tx_buffer[0] - 0x%04X // tx_buffer[1] - 0x%04X\r\n", tx_buffer[0], tx_buffer[1]);

    spi_write(g_hSPIx, tx_buffer, 2);
#else // xc daisy test
    _xc24_cmd_t cmd_format;
    uint16_t tx_buffer[4] = {0,};

    if (g_hSPIx == NULL)
    {
        g_hSPIx = SPI1;
    }

    cmd_format.ALL = 0;
    cmd_format.code = CMD_CODE_REG_WRITE;
    cmd_format.addr = in_addr;
    cmd_format.size = 1;

    tx_buffer[0] = cmd_format.ALL;
    tx_buffer[1] = in_data;
    tx_buffer[2] = cmd_format.ALL;
    tx_buffer[3] = in_data;

    *(gt_xc24_regs.ALL + in_addr) = in_data;

    //print(LOG_DEBUG, " XC24_SPI_Write(0x%02X)(%s) - [%5u] [0x%4X] \r\n", in_addr, gs_xc24_addr_str[in_addr], in_data, in_data);

    spi_write(g_hSPIx, tx_buffer, 4);
#endif
}

uint16_t XC24_Read_Register(uint8_t in_addr)
{
    _xc24_cmd_t cmd_format;
    uint16_t tx_buffer[2] = {0, };
	uint16_t rx_buffer[2] = {0, };

    if (g_hSPIx == NULL)
    {
        g_hSPIx = SPI1;
    }

    cmd_format.ALL = 0;
    cmd_format.code = CMD_CODE_REG_READ;
    cmd_format.addr = in_addr;
    cmd_format.size = 1;
    tx_buffer[0] = cmd_format.ALL;

    spi_read(g_hSPIx, tx_buffer, rx_buffer, 2);
    *(gt_xc24_regs.ALL + in_addr) = rx_buffer[1];

    // print(LOG_INFO, "XC24_Read_Register(0x%2X)(%s) - [%u] [0x%4X]\r\n", in_addr, gs_xc24_addr_str[in_addr], *(gt_xc24_regs.ALL + in_addr), *(gt_xc24_regs.ALL + in_addr));
    // print(LOG_DEBUG, "XC24_Read_Register(0x%2X) - [%u] [0x%4X]\r\n", in_addr, *(gt_xc24_regs.ALL + in_addr), *(gt_xc24_regs.ALL + in_addr));

    return rx_buffer[1];
}

void XC24_Read_Register_All(void)
{
    uint16_t t_xc_reg;
    for (int i = 0 ; i < XC24_ADDR_MAX ; ++i)
    {
        t_xc_reg = XC24_Read_Register(i);
        print(LOG_INFO, "%s(0x%02X): 0x%4X (%4u)\r\n", gs_xc24_addr_str[i], i, t_xc_reg, t_xc_reg);
    }
}

void XC24_Init(void)
{
    g_hSPIx = SPI1;

    JigBD_IF_XC_VCC_EN(PWR_ON);

    LL_mDelay(20);

    XC_NSCS_HI();
    XC24_Start_MCLK_Oscillation(TRUE);

    print(LOG_DEBUG, " ...XC24 Initial Start...\r\n");

    for (uint8_t xc_addr = 0 ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        switch (xc_addr)
        {
        case XC24_ADDR_SOFT_RESET:
            gt_xc24_regs._r00.rst1 = 1;
            gt_xc24_regs._r00.rst2 = 1;
            gt_xc24_regs._r00.rst3 = 1;
            break;
        case XC24_ADDR_LD_TRANSFER_START_POINTER_TH :
            gt_xc24_regs._r0D.ld_trans_start_pointer = 4;
            gt_xc24_regs._r0D.ld_diff_threshold = 4;
            break;
        case XC24_ADDR_LOCAL_RW_POINTER_RESET :
            gt_xc24_regs._r11.local_rd_pointer_rst = 1;
            gt_xc24_regs._r11.local_wr_pointer_rst = 1;
            break;
        case XC24_ADDR_FAULT_AUTO_READ_TIMER :
            gt_xc24_regs._r12.fault_auto_rd_timer = 0xFFFF;
            break;
        case XC24_ADDR_FAULT_AUTO_READ_EVENT :
            gt_xc24_regs._r13.fault_auto_rd_interval = 1;
            gt_xc24_regs._r13.fault_auto_rd_event = 1;
            break;
        case XC24_ADDR_SERIALIZER_CLOCK_GEN :
            gt_xc24_regs._r14.sck_low = XC_SERIAL_CLK_CNT_LOW;
            gt_xc24_regs._r14.sck_high = XC_SERIAL_CLK_CNT_HIGH;
            break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1 :
            gt_xc24_regs._r20.daisied_dev_ch_size_1 = XD_CH_SIZE;
            break;
        case XC24_ADDR_DAISY_SIZE1 :
            gt_xc24_regs._r30.daisy_size_ch1 = XD_DAISY_SIZE;
            break;
        case XC24_ADDR_BLOCK_SIZE1 :
            gt_xc24_regs._r38.block_size_ch1 = XD_DAISY_SIZE * XD_CH_SIZE;
            break;
        case XC24_ADDR_CHANNEL_ENABLE1 :
            gt_xc24_regs._r45.ch1_en = 1;
            break;
        case XC24_ADDR_CHANNEL_ENABLE2 :
            gt_xc24_regs._r46.ch_size = 1;
            gt_xc24_regs._r46.ld_width = 3;
            break;
        case XC24_ADDR_LDO :
            gt_xc24_regs._r2A.ldo = 8;
            break;
        case XC24_ADDR_AUTO_ENABLE :
            gt_xc24_regs._r08.timeout_en = 1;
            gt_xc24_regs._r08.fault_auto_en = 1;
            break;
        case XC24_ADDR_INTERRUPT_ENABLE :
            gt_xc24_regs._r15.int_fb_en = 1;
            break;
        default :
            continue;
        }
        XC24_Write_Register(xc_addr, gt_xc24_regs.ALL[xc_addr]);
        us_tdelay(XD04_WRITE_DELAY);
    }
    print(LOG_DEBUG, " ...XC24 Initial Done...\r\n");
    XC24_Read_Register_All();
}

void XC24_Start_MCLK_Oscillation(bool en)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (en)
    {
        GPIO_InitStruct.Pin = MCLK_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
        LL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
    }
    else
    {
        GPIO_InitStruct.Pin = MCLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        LL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
    }
}

/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/

bool IS_XC24(void)
{
    return gb_XC24_support;
}

void USE_XC24(bool b_support)
{
    gb_XC24_support = b_support;
}

void XC24_IF_IdGen_Command(void)
{
    _v_id_gen_t* p_v_id_gen = &gt_xc24_regs._r04;

    /* XC24 -> XD04 Set ID GEN */
    p_v_id_gen->ALL = 0;
    p_v_id_gen->start = 1;
    XC24_Write_Register(XC24_ADDR_ID_GEN, p_v_id_gen->ALL);
    us_tdelay(XD04_IDGEN_DELAY);
}

void XC24_IF_SyncGen_Command(void)
{
    _v_sync_gen_t* p_v_sync_gen = &gt_xc24_regs._r07;

    /* XC24 -> XD04 Set Sync GEN */
    p_v_sync_gen->ALL = 0;
    p_v_sync_gen->start = 1;
    XC24_Write_Register(XC24_ADDR_SYNC_GEN, p_v_sync_gen->ALL);
    us_tdelay(XD04_IDGEN_DELAY);
}

uint16_t XC24_IF_Fault_Read_Command(void)
{
    _v_fault_read_t* p_v_fault_read = &gt_xc24_regs._r05;

    /* XC24 -> XD04 Set Sync GEN */
    p_v_fault_read->ALL = 0;
    p_v_fault_read->start = 1;
    XC24_Write_Register(XC24_ADDR_FAULT_READ, p_v_fault_read->ALL);
    us_tdelay(XD04_IDGEN_DELAY);

    gt_xc24_fault._d1.ALL = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA1);

    return gt_xc24_fault._d1.ALL;
}

void XC24_IF_Write_XD04(uint8_t in_XD04_addr, uint16_t in_XD04_data)
{
    _v_global_write_t* p_v_global_write = &gt_xc24_regs._r01;

    p_v_global_write->ALL = 0;
    p_v_global_write->start = 1;
    p_v_global_write->addr = in_XD04_addr;

    //print(LOG_DEBUG, "\r\n========== XC -> XD Write Start (0x%02X) - [%u] [0x%4X] ==========\r\n", in_XD04_addr, in_XD04_data, in_XD04_data);

    /* 1st - Write the XD04_Data on the GLOBAL_WRITE_DATA Register of XC24 */
    XC24_Write_Register(XC24_ADDR_GLOBAL_WRITE_DATA, in_XD04_data);
    us_tdelay(1);

    /* 2nd - Write the XD04_Addr on the GLOBAL_WRITE_COMMAND Register of XC24 */
    XC24_Write_Register(XC24_ADDR_GLOBAL_WRITE, p_v_global_write->ALL);
    /* To Do : delay must more than 65us */
    us_tdelay(XD04_WRITE_DELAY);

    //print(LOG_DEBUG, "========== XC -> XD  Write Done (0x%02X) - [%u] [0x%4X] ==========\r\n\r\n", in_XD04_addr, in_XD04_data, in_XD04_data);
}

uint16_t XC24_IF_Read_XD04(uint8_t in_XD04_addr)
{
    _v_local_rw_pointer_reset_t* p_v_local_rw_pointer_reset = &gt_xc24_regs._r11;
    _v_local_read_t* p_v_local_read = &gt_xc24_regs._r03;

    uint16_t u16_XD04_data = 0;
    uint16_t u16_XD04_r = 0;
    uint32_t count = 5;

    //print(LOG_DEBUG, "\r\n========== XC -> XD Read Start (0x%02X) ==========\r\n", in_XD04_addr);

    /* 1st - Send Local RW Pointer Reset */
    p_v_local_rw_pointer_reset->ALL = 0;
    p_v_local_rw_pointer_reset->local_wr_pointer_rst = 1;
    p_v_local_rw_pointer_reset->local_rd_pointer_rst = 1;
    XC24_Write_Register(XC24_ADDR_LOCAL_RW_POINTER_RESET, p_v_local_rw_pointer_reset->ALL);

    us_tdelay(1);

    while(count)
    {
        u16_XD04_r = XC24_Read_Register(XC24_ADDR_LOCAL_RD_RECEIVE_POINTER);

        --count;

        if (u16_XD04_r == 0)
        {
            us_tdelay(1);
            break;
        }
        else
        {
            //print(LOG_DEBUG, "==============> LOCAL POINTER(%u) : 0x%04X <==============\r\n", count, u16_XD04_r);
            XC24_Write_Register(XC24_ADDR_LOCAL_RW_POINTER_RESET, p_v_local_rw_pointer_reset->ALL);
            us_tdelay(1);
        }
    }

    /* 2nd - Write the XD04_Addr on the LOCAL_READ Register of XC24 */
    p_v_local_read->ALL = 0;
    p_v_local_read->start = 1;
    p_v_local_read->ch_seg = 0;
    p_v_local_read->addr = in_XD04_addr;
    XC24_Write_Register(XC24_ADDR_LOCAL_READ, p_v_local_read->ALL);

    // Wait Ack
    us_tdelay(XD04_READ_DELAY + XD04_READ_RECV_DELAY);

    /* 3rd - Receive Data XD04_Data through XC24 */
    u16_XD04_data = XC24_Read_Register(XC24_ADDR_PORT1_LOCAL_RW_DATA1);
    //print(LOG_DEBUG, "XC24_IF_Read_XD04(0x%02X) - [%u] [0x%04X] \r\n",in_XD04_addr, u16_XD04_data, u16_XD04_data);

    //print(LOG_DEBUG, "\r\n========== XC -> XD Read  Done (0x%02X) ==========\r\n", in_XD04_addr);

    return u16_XD04_data;
}

void XC24_IF_Write_LD(uint16_t in_LD_data)
{
    _xc24_cmd_t cmd_format;
    uint16_t tx_buffer[1 + XD_DAISY_SIZE * XD_CH_SIZE] = {0,};

    if (g_hSPIx == NULL)
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
}

/* END - INTERFACE FUNCTIONS ************************************************************************/

/*** end of file ***/