/** @file xc24.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XC24_C__
#include "main.h"
#include "xc24.h"
#include "JigBd_IF.h"
#include "types.h"
#include "config.h"

#define XC24_OTP_PROTECT_DISABLE    (0xA5A)
#define XC24_OTP_PROTECT_ENABLE     (0x5A5)

static SPI_TypeDef *g_hSPIx;

static bool gb_xc24_support;

static _xc24_general_regs_t gt_xc24_general_regs;
static _xc24_mirror_regs_t gt_xc24_mirror_regs;

static const char* gs_xc24_addr_str[XC24_ADDR_MAX] =
{
    "SOFT_RESET",
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

    "LOCAL_RW_DIFFERENCE_POINTER",
    "LOCAL_RW_POINTER_RESET",
    "FAULT_AUTO_READ_TIMER",
    "FAULT_AUTO_READ_EVENT",
    "SERIALIZER_CLOCK_GEN",
    "INTERRUPT_ENABLE",
    "COMMAND_STATUS1",
    "COMMAND_STATUS2",
    "RECEIVE_STATUS",
    "INTERRUPT_STATUS",
    "SPI_FAULT_STATUS_CONTROL",
    "CLK_CONTROL_1",
    "CLK_CONTROL_2",
    "VDD_LDO_STATUS",
    "VDD_LDO_FAULT_LEVEL",
    "COMMAND_LATENCY",

    "DAISIED_DEVICE_CHANNEL_SIZE1",
    "DAISIED_DEVICE_CHANNEL_SIZE2",
    "DAISIED_DEVICE_CHANNEL_SIZE3",
    "DAISIED_DEVICE_CHANNEL_SIZE4",
    "DAISIED_DEVICE_CHANNEL_SIZE5",
    "DAISIED_DEVICE_CHANNEL_SIZE6",
    "DAISIED_DEVICE_CHANNEL_SIZE7",
    "DAISIED_DEVICE_CHANNEL_SIZE8",
    "DUMMY",
    "DUMMY",
    "DUMMY",
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
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DAC_CONTROL",
    "DUMMY",
    "CURRENT_TARGET_DAC",

    "PREVIOUS_TARGET_DAC",
    "DAC_OUT",
    "DAC_INCREMENT1",
    "DAC_INCREMENT2_HOLD_LIMIT",
    "DAC_DECREMENT_INC_WAIT",
    "DAC_INCREMENT_HOLD_TH",
    "SOA_N11_N1",
    "SOA_P2_P1",
    "SOA_P3_P2",
    "DAC_FB_VALID_TIMER",
    "DAC_MIN_LIMIT",
    "DAC_MAX_LIMIT",
    "DAC_STATE",
    "VALID_CNT",
    "DAC_INC_HOLD_WAIT_CNT",
    "R2",

    "GLOBAL_WRITE_V",
    "GLOBAL_FAULT_READ_DATA1",
    "GLOBAL_FAULT_READ_DATA2",
    "GLOBAL_FAULT_READ_DATA3",
    "GLOBAL_FAULT_READ_DATA4",
    "GLOBAL_FAULT_READ_DATA5",
    "GLOBAL_FAULT_READ_DATA6",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",

    "PORT_LOCAL_RW_DATA",
};

static const char* gs_xc24_trim_addr_str[XC24_ADDR_TRIM_MAX - XC24_ADDR_TRIM_START] =
{
    "TEST_CONTROL",
    "OTP_PG_ACCESS",
    "OTP_WRITE",
    "OTP_RD_PROG",
    "OTP_PROTECT",
    "MIRROR1",
    "MIRROR2",
    "MIRROR3",
};

__STATIC_INLINE void SPI_Write(SPI_TypeDef *SPIx, uint16_t* p_buffer, uint16_t len)
{
    XC_NSCS_LO();

    if (LL_SPI_IsEnabled(SPIx) != 1)
    {
        LL_SPI_Enable(SPIx);
    }

    for (volatile uint16_t i = 0 ; i < len ; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) {};
        LL_SPI_TransmitData16(SPIx, p_buffer[i]);
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx)) {};
    us_delay(1);

    XC_NSCS_HI();
}

__STATIC_INLINE void SPI_Read(SPI_TypeDef *SPIx, uint16_t* p_tx_buffer, uint16_t* p_rx_buffer, uint16_t len)
{
    XC_NSCS_LO();

    if (LL_SPI_IsEnabled(SPIx) != 1)
    {
        LL_SPI_Enable(SPIx);
    }

    while(SET == LL_SPI_IsActiveFlag_RXNE(SPIx))
    {
        volatile uint16_t temp  = LL_SPI_ReceiveData16(SPIx);
    }

    for (volatile uint16_t i = 0 ; i < len ; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx)) {};
        LL_SPI_TransmitData16(SPIx, p_tx_buffer[i]);

        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx)) {};
        p_rx_buffer[i] = LL_SPI_ReceiveData16(SPIx);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx)) {};
    us_delay(1);

    XC_NSCS_HI();
}

void XC24_Write_Register(uint16_t in_addr, uint16_t in_data)
{
    _xc24_cmd_t cmd_format = {0, };
    uint16_t tx_buffer[2] = {0,};

    cmd_format.code = CMD_CODE_REG_WRITE;
    cmd_format.addr = in_addr;
    cmd_format.size = 1;

    tx_buffer[0] = cmd_format.ALL;
    tx_buffer[1] = in_data;

    if (in_addr < XC24_ADDR_MAX)
    {
        *(gt_xc24_general_regs.ALL + in_addr) = in_data;
    }

    else if (in_addr >= XC24_ADDR_TRIM_START && in_addr < XC24_ADDR_TRIM_MAX)
    {
        *(gt_xc24_mirror_regs.ALL + (in_addr - XC24_ADDR_TRIM_TEST_CONTROL)) = in_data;
    }

    SPI_Write(g_hSPIx, tx_buffer, 2);
}

uint16_t XC24_Read_Register(uint8_t in_addr)
{
    _xc24_cmd_t cmd_format = {0, };
    uint16_t tx_buffer[2] = {0, };
    uint16_t rx_buffer[2] = {0, };

    cmd_format.code = CMD_CODE_REG_READ;
    cmd_format.addr = in_addr;
    cmd_format.size = 1;

    tx_buffer[0] = cmd_format.ALL;

    SPI_Read(g_hSPIx, tx_buffer, rx_buffer, 2);

    if (in_addr < XC24_ADDR_MAX)
    {
        *(gt_xc24_general_regs.ALL + in_addr) = rx_buffer[1];
    }
    else if (in_addr >= XC24_ADDR_TRIM_START && in_addr < XC24_ADDR_TRIM_MAX)
    {
        *(gt_xc24_mirror_regs.ALL + (in_addr - XC24_ADDR_TRIM_TEST_CONTROL)) = rx_buffer[1];
    }

    return rx_buffer[1];
}

void XC24_Read_Register_All(void)
{
    for (uint8_t xc_addr = 0 ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        XC24_Read_Register(xc_addr);
        us_delay(10);
    }

    for (uint8_t xc_addr = XC24_ADDR_TRIM_START ; xc_addr < XC24_ADDR_TRIM_MAX ; ++xc_addr)
    {
        XC24_Read_Register(xc_addr);
        us_delay(10);
    }
    XC24_Dump_All_Register();
}

void XC24_Dump_All_Register(void)
{
    for (uint8_t xc_addr = 0 ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        print(LOG_INFO, "[%s (0x%02X)]\r\n"
                        "\t VALUE : (0x%04X)\r\n\r\n",
                        gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_general_regs.ALL[xc_addr]);
    }

    for (uint8_t xc_addr = XC24_ADDR_TRIM_START ; xc_addr < XC24_ADDR_TRIM_MAX ; ++xc_addr)
    {
        print(LOG_INFO, "[%s (0x%02X)]\r\n"
                        "\t VALUE : (0x%04X)\r\n\r\n",
                        gs_xc24_trim_addr_str[xc_addr], xc_addr, gt_xc24_mirror_regs.ALL[xc_addr]);
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
            gt_xc24_general_regs._r00.rst1 = 1;
            gt_xc24_general_regs._r00.rst2 = 1;
            gt_xc24_general_regs._r00.rst3 = 1;
            break;
        case XC24_ADDR_AUTO_ENABLE:
            gt_xc24_general_regs._r08.timeout_en = 1;
            gt_xc24_general_regs._r08.sync_auto_en = 0;
            gt_xc24_general_regs._r08.fault_auto_en = 0;
            break;
        case XC24_ADDR_LD_TRANSFER_START_POINTER_TH :
            gt_xc24_general_regs._r0D.ld_trans_start_pointer = 4;
            gt_xc24_general_regs._r0D.ld_diff_threshold = 4;
            break;
        case XC24_ADDR_LOCAL_RW_POINTER_RESET :
            gt_xc24_general_regs._r11.local_rd_pointer_rst = 1;
            gt_xc24_general_regs._r11.local_wr_pointer_rst = 1;
            break;
        case XC24_ADDR_FAULT_AUTO_READ_TIMER :
            gt_xc24_general_regs._r12.fault_auto_rd_timer = 0xFFFF;
            break;
        case XC24_ADDR_FAULT_AUTO_READ_EVENT :
            gt_xc24_general_regs._r13.fault_auto_rd_interval = 1;
            gt_xc24_general_regs._r13.fault_auto_rd_event = 1;
            break;
        case XC24_ADDR_SERIALIZER_CLOCK_GEN :
            gt_xc24_general_regs._r14.sck_low = XC_SERIAL_CLK_CNT_LOW;
            gt_xc24_general_regs._r14.sck_high = XC_SERIAL_CLK_CNT_HIGH;
            break;
        case XC24_ADDR_INTERRUPT_ENABLE :
            gt_xc24_general_regs._r15.int_fb_en = 1;
            gt_xc24_general_regs._r15.int_open_en = 1;
            gt_xc24_general_regs._r15.int_short_en = 1;
            gt_xc24_general_regs._r15.int_thermal_en = 1;
            break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1 :
            gt_xc24_general_regs._r20.daisied_dev_ch_size_1 = XD_CH_SIZE;
            break;
        case XC24_ADDR_DAISY_SIZE1 :
            gt_xc24_general_regs._r30.daisy_size_ch1 = XD_DAISY_SIZE;
            break;
        case XC24_ADDR_BLOCK_SIZE1 :
            gt_xc24_general_regs._r38.block_size_ch1 = XD_DAISY_SIZE * XD_CH_SIZE;
            break;
        case XC24_ADDR_CHANNEL_ENABLE1 :
            gt_xc24_general_regs._r45.ch1_en = 1;
            break;
        case XC24_ADDR_CHANNEL_ENABLE2 :
            gt_xc24_general_regs._r46.ld_size = 1;
            gt_xc24_general_regs._r46.ld_width = 3;
            break;
        default :
            continue;
        }
        XC24_Write_Register(xc_addr, gt_xc24_general_regs.ALL[xc_addr]);
        us_delay(10);
    }
    print(LOG_DEBUG, " ...XC24 Initial Done...\r\n");
    XC24_Read_Register_All();
}

void XC24_Trim_Init(void)
{
    //to-do : trim init value set
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
            gt_xc24_general_regs._r00.rst1 = 1;
            gt_xc24_general_regs._r00.rst2 = 1;
            gt_xc24_general_regs._r00.rst3 = 1;
            break;
        default :
            continue;
        }
        XC24_Write_Register(xc_addr, gt_xc24_general_regs.ALL[xc_addr]);
        us_delay(10);
    }

    for (uint8_t xc_addr = XC24_ADDR_TRIM_START ; xc_addr < XC24_ADDR_TRIM_MAX ; ++xc_addr)
    {
        switch (xc_addr)
        {
        case XC24_ADDR_TRIM_TEST_CONTROL:
            gt_xc24_mirror_regs._rF0.test_en = 1;
            break;
        case XC24_ADDR_TRIM_OTP_PROTECT:
            gt_xc24_mirror_regs._rF4.protect = XC24_OTP_PROTECT_DISABLE;
            break;
        default :
            continue;
        }
        XC24_Write_Register(xc_addr, gt_xc24_mirror_regs.ALL[xc_addr]);
        us_delay(10);
    }
    XC24_Set_OTP_Protect(false);

    print(LOG_DEBUG, " ...XC24 Initial Done...\r\n");
    XC24_Read_Register_All();
}

void XC24_DAC_GAIN_TRIM_INIT(void)
{
    gt_xc24_mirror_regs._rF0.test_en = 1;
    gt_xc24_mirror_regs._rF0.daco_direct = 1;

    XC24_Write_Register(XC24_ADDR_TRIM_TEST_CONTROL, gt_xc24_mirror_regs._rF0.ALL);
}

void XC24_Set_OTP_Protect(bool en)
{
    if (en == true)
    {
        gt_xc24_mirror_regs._rF4.protect = XC24_OTP_PROTECT_ENABLE;
    }
    else
    {
        gt_xc24_mirror_regs._rF4.protect = XC24_OTP_PROTECT_DISABLE;
    }
    XC24_Write_Register(XC24_ADDR_TRIM_OTP_PROTECT, gt_xc24_mirror_regs._rF4.ALL);
}

void XC24_Start_MCLK_Oscillation(bool en)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0, };

    if (en)
    {
        GPIO_InitStruct.Pin = XC_MCLK_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
        LL_GPIO_Init(XC_MCLK_GPIO_Port, &GPIO_InitStruct);

        LL_mDelay(10);
    }
    else
    {
        GPIO_InitStruct.Pin = XC_MCLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        LL_GPIO_Init(XC_MCLK_GPIO_Port, &GPIO_InitStruct);
    }
}

/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/

bool IS_XC24_Support(void)
{
    return gb_xc24_support;
}

void USE_XC24(bool b_support)
{
    gb_xc24_support = b_support;
}

void XC24_IF_IdGen_Command(void)
{
    gt_xc24_general_regs._r04.ALL = 0;
    gt_xc24_general_regs._r04.start = 1;
    XC24_Write_Register(XC24_ADDR_ID_GEN, gt_xc24_general_regs._r04.ALL);
    us_delay(XDIC_IDGEN_DELAY);
}

void XC24_IF_SyncGen_Command(void)
{
    gt_xc24_general_regs._r07.ALL = 0;
    gt_xc24_general_regs._r07.start = 1;
    XC24_Write_Register(XC24_ADDR_SYNC_GEN, gt_xc24_general_regs._r07.ALL);
    us_delay(XDIC_SYNCGEN_DELAY);
}

uint16_t XC24_IF_Fault_Read_Command(void)
{
    gt_xc24_general_regs._r05.ALL = 0;
    gt_xc24_general_regs._r05.start = 1;
    XC24_Write_Register(XC24_ADDR_FAULT_READ, gt_xc24_general_regs._r05.ALL);
    us_delay(XDIC_FAULT_READ_DELAY + XDIC_FAULT_RECV_DELAY);

    gt_xc24_general_regs._r61.ALL = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA1);

    return gt_xc24_general_regs._r61.ALL;
}

void XC24_IF_Write_XDIC(uint8_t in_XDIC_addr, uint16_t in_XDIC_data)
{
    gt_xc24_general_regs._r01.ALL = 0;
    gt_xc24_general_regs._r01.start = 1;
    gt_xc24_general_regs._r01.addr = in_XDIC_addr;

    XC24_Write_Register(XC24_ADDR_GLOBAL_WRITE_DATA, in_XDIC_data);
    us_delay(1);

    XC24_Write_Register(XC24_ADDR_GLOBAL_WRITE, gt_xc24_general_regs._r01.ALL);
    us_delay(XDIC_RESET_DELAY);
}

uint16_t XC24_IF_Read_XDIC(uint8_t in_XDIC_addr)
{
    uint16_t u16_XDIC_data = 0;

    gt_xc24_general_regs._r11.ALL = 0;
    gt_xc24_general_regs._r11.local_wr_pointer_rst = 1;
    gt_xc24_general_regs._r11.local_rd_pointer_rst = 1;
    XC24_Write_Register(XC24_ADDR_LOCAL_RW_POINTER_RESET, gt_xc24_general_regs._r11.ALL);

    gt_xc24_general_regs._r03.ALL = 0;
    gt_xc24_general_regs._r03.start = 1;
    gt_xc24_general_regs._r03.ch_seg = 0;
    gt_xc24_general_regs._r03.addr = in_XDIC_addr;
    XC24_Write_Register(XC24_ADDR_LOCAL_READ, gt_xc24_general_regs._r03.ALL);
    us_delay(XDIC_READ_DELAY + XDIC_READ_RECV_DELAY);

    u16_XDIC_data = XC24_Read_Register(XC24_ADDR_PORT1_LOCAL_RW_DATA1);

    return u16_XDIC_data;
}

void XC24_IF_Write_LD(uint16_t in_LD_data)
{
    _xc24_cmd_t cmd_format;
    uint16_t tx_buffer[1 + XD_DAISY_SIZE * XD_CH_SIZE] = {0,};

    cmd_format.ALL = 0;
    cmd_format.code = CMD_CODE_LD_TRANS;
    cmd_format.addr = 0;
    cmd_format.size = XD_DAISY_SIZE * XD_CH_SIZE;

    tx_buffer[0] = cmd_format.ALL;
    for (uint8_t i = 0 ; i < (XD_DAISY_SIZE * XD_CH_SIZE) ; ++i)
    {
        tx_buffer[i + 1] = in_LD_data;
    }

    SPI_Write(g_hSPIx, tx_buffer, 1 + XD_DAISY_SIZE * XD_CH_SIZE);
    us_delay(XDIC_LD_TRANS_DELAY);
}

/* END - INTERFACE FUNCTIONS ************************************************************************/

/*** end of file ***/