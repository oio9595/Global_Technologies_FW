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

    print(LOG_INFO, "XC24_Read_Register, 0x%02X, %s, 0x%04X\r\n", in_addr, gs_xc24_addr_str[in_addr], *(gt_xc24_regs.ALL + in_addr));
    // print(LOG_DEBUG, "XC24_Read_Register(0x%2X) - [%u] [0x%4X]\r\n", in_addr, *(gt_xc24_regs.ALL + in_addr), *(gt_xc24_regs.ALL + in_addr));

    return rx_buffer[1];
}

void XC24_Read_Register_All(void)
{
    for (uint8_t xc_addr = 0 ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        uint16_t t_xc_reg = XC24_Read_Register(xc_addr);
        //print(LOG_INFO, "%s(0x%02X): 0x%04X (%4u)\r\n", gs_xc24_addr_str[xc_addr], xc_addr, t_xc_reg, t_xc_reg);
        LL_mDelay(0);
    }
    XC24_Dump_All_Register();
}

void XC24_Dump_All_Register(void)
{
    for (uint8_t xc_addr = 0 ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        switch (xc_addr)
        {
        case XC24_ADDR_SOFT_RESET:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t VS_RST2                     : [%u]\r\n"
                            "\t VS_RST1                     : [%u]\r\n"
                            "\t RST3                        : [%u]\r\n"
                            "\t RST2                        : [%u]\r\n"
                            "\t RST1                        : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r00.vs_rst2, gt_xc24_regs._r00.vs_rst1, gt_xc24_regs._r00.rst3, gt_xc24_regs._r00.rst2, gt_xc24_regs._r00.rst1, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_GLOBAL_WRITE:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t START                       : [%u]\r\n"
                            "\t ADDRESS                     : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r01.start, gt_xc24_regs._r01.addr, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LOCAL_WRITE:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t START                       : [%u]\r\n"
                            "\t CH_SEG                      : [%u]\r\n"
                            "\t ADDRESS                     : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r02.start, gt_xc24_regs._r02.ch_seg, gt_xc24_regs._r02.addr, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LOCAL_READ:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t START                       : [%u]\r\n"
                            "\t CH_SEG                      : [%u]\r\n"
                            "\t ADDRESS                     : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r03.start, gt_xc24_regs._r03.ch_seg, gt_xc24_regs._r03.addr, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_ID_GEN:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t START                       : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r04.start, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_FAULT_READ:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t START                       : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r05.start, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LD_TRANSFER:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t START                       : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r06.start, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_SYNC_GEN:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t START                       : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r07.start, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_AUTO_ENABLE:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t TIMEOUT_EN                  : [%u]\r\n"
                            "\t FAULT_AUTO_EN               : [%u]\r\n"
                            "\t SYNC_AUTO_EN                : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r08.timeout_en, gt_xc24_regs._r08.fault_auto_en, gt_xc24_regs._r08.sync_auto_en, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LD_WRITE_POINTER:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LD_WR_POINTER               : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r0A.ld_wr_pointer, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LD_READ_POINTER:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LD_RD_POINTER               : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r0B.ld_rd_pointer, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DIFFERENCE_POINTER:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LD_DIFF_POINTER             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r0C.ld_diff_pointer, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LD_TRANSFER_START_POINTER_TH:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LD_TRANS_START_POINTER      : [%u]\r\n"
                            "\t INT_LD_SIGN                 : [%u]\r\n"
                            "\t LD_DIFF_THRESHOLD           : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r0D.ld_trans_start_pointer, gt_xc24_regs._r0D.int_ld_sign, gt_xc24_regs._r0D.ld_diff_threshold, gt_xc24_regs.ALL[xc_addr]);
                break;
        case XC24_ADDR_LOCAL_WR_TRANSFER_POINTER:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LOCAL_WR_OUT_POINTER        : [%u]\r\n"
                            "\t LOCAL_WR_TRANS_POINTER      : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r0E.local_wr_out_pointer, gt_xc24_regs._r0E.local_wr_trans_pointer, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LOCAL_RD_RECEIVE_POINTER:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LOCAL_RD_OUT_POINTER        : [%u]\r\n"
                            "\t LOCAL_RD_REC_POINTER        : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r0F.local_rd_out_pointer, gt_xc24_regs._r0F.local_rd_rec_pointer, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LOCAL_RW_DIFFERENCE_POINTER:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LOCAL_RD_DIFF_POINTER       : [%u]\r\n"
                            "\t LOCAL_WR_DIFF_POINTER       : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r10.local_rd_diff_pointer, gt_xc24_regs._r10.local_wr_diff_pointer, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LOCAL_RW_POINTER_RESET:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LOCAL_RD_POINTER_RST        : [%u]\r\n"
                            "\t LOCAL_WR_POINTER_RST        : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r11.local_rd_pointer_rst, gt_xc24_regs._r11.local_wr_pointer_rst, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_FAULT_AUTO_READ_TIMER:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t FAULT_AUTO_RD_TIMER         : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r12.fault_auto_rd_timer, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_FAULT_AUTO_READ_EVENT:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t FAULT_AUTO_RD_EVENT         : [%u]\r\n"
                            "\t FAULT_AUTO_RD_INTERVAL      : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r13.fault_auto_rd_event, gt_xc24_regs._r13.fault_auto_rd_interval, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_SERIALIZER_CLOCK_GEN:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t SCK_LOW                     : [%u]\r\n"
                            "\t SCK_HIGH                    : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r14.sck_low, gt_xc24_regs._r14.sck_high, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_INTERRUPT_ENABLE:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t INT_TIMEOUT_ERR_EN          : [%u]\r\n"
                            "\t INT_FAULT_RD_FAIL_EN        : [%u]\r\n"
                            "\t INT_FAULT_AUTO_RD_FAIL_EN   : [%u]\r\n"
                            "\t INT_RD_REC_FAIL_EN          : [%u]\r\n"
                            "\t INT_LD_EN                   : [%u]\r\n"
                            "\t INT_THERMAL_EN              : [%u]\r\n"
                            "\t INT_SHORT_EN                : [%u]\r\n"
                            "\t INT_OPEN_EN                 : [%u]\r\n"
                            "\t INT_FB_EN                   : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r15.int_timeout_err_en, gt_xc24_regs._r15.int_fault_rd_fail_en, gt_xc24_regs._r15.int_fault_auto_rd_fail_en,
                gt_xc24_regs._r15.int_rd_rec_fail_en, gt_xc24_regs._r15.int_ld_en, gt_xc24_regs._r15.int_thermal_en, gt_xc24_regs._r15.int_short_en, gt_xc24_regs._r15.int_open_en, gt_xc24_regs._r15.int_fb_en, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_COMMAND_STATUS1:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LOCAL_RD_DONE               : [%u]\r\n"
                            "\t LOCAL_RD_DOING              : [%u]\r\n"
                            "\t LOCAL_WR_DONE               : [%u]\r\n"
                            "\t LOCAL_WR_DOING              : [%u]\r\n"
                            "\t GLOBAL_WR_DONE              : [%u]\r\n"
                            "\t GLOBAL_WR_DOING             : [%u]\r\n"
                            "\t LD_TRANS_DONE               : [%u]\r\n"
                            "\t LD_TRANS_DOING              : [%u]\r\n"
                            "\t FAULT_DONE                  : [%u]\r\n"
                            "\t FAULT_DOING                 : [%u]\r\n"
                            "\t FAULT_AUTO_DONE             : [%u]\r\n"
                            "\t FAULT_AUTO_DOING            : [%u]\r\n"
                            "\t SYNC_AUTO_DONE              : [%u]\r\n"
                            "\t SYNC_AUTO_DOING             : [%u]\r\n"
                            "\t SYNC_DONE                   : [%u]\r\n"
                            "\t SYNC_DOING                  : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r16.local_rd_done, gt_xc24_regs._r16.local_rd_doing,
                gt_xc24_regs._r16.local_wr_done, gt_xc24_regs._r16.local_wr_doing, gt_xc24_regs._r16.global_wr_done, gt_xc24_regs._r16.global_wr_doing,
                gt_xc24_regs._r16.ld_trans_done, gt_xc24_regs._r16.ld_trans_doing, gt_xc24_regs._r16.fault_done, gt_xc24_regs._r16.fault_doing,
                gt_xc24_regs._r16.fault_auto_done, gt_xc24_regs._r16.fault_auto_doing, gt_xc24_regs._r16.sync_auto_done, gt_xc24_regs._r16.sync_auto_doing,
                gt_xc24_regs._r16.sync_done, gt_xc24_regs._r16.sync_doing, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_COMMAND_STATUS2:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ID_GEN_DONE                 : [%u]\r\n"
                            "\t ID_GEN_DOING                : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
            gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r17.id_gen_done, gt_xc24_regs._r17.id_gen_doing, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_RECEIVE_STATUS:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t FAULT_REC_FAIL              : [%u]\r\n"
                            "\t FAULT_AUTO_REC_FAIL         : [%u]\r\n"
                            "\t LOCAL_REC_FAIL              : [%u]\r\n"
                            "\t TIMEOUT_ERR                 : [%u]\r\n"
                            "\t FAULT_REC_DONE              : [%u]\r\n"
                            "\t FAULT_REC_DOING             : [%u]\r\n"
                            "\t FAULT_AUTO_REC_DONE         : [%u]\r\n"
                            "\t FAULT_AUTO_REC_DOING        : [%u]\r\n"
                            "\t LOCAL_REC_DONE              : [%u]\r\n"
                            "\t LOCAL_REC_DOING             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r18.fault_rec_fail, gt_xc24_regs._r18.fault_auto_rec_fail, gt_xc24_regs._r18.local_rec_fail,
                gt_xc24_regs._r18.timeout_err, gt_xc24_regs._r18.fault_rec_done, gt_xc24_regs._r18.fault_rec_doing, gt_xc24_regs._r18.fault_auto_rec_done, gt_xc24_regs._r18.fault_auto_rec_doing,
                gt_xc24_regs._r18.local_rec_done, gt_xc24_regs._r18.local_rec_doing, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_INTERRUPT_STATUS:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t INT_TIMEOUT_ERR_SRC         : [%u]\r\n"
                            "\t INT_FAULT_REC_FAIL_SRC      : [%u]\r\n"
                            "\t INT_FAULT_AUTO_REC_FAIL_SRC : [%u]\r\n"
                            "\t INT_LOCAL_REC_FAIL_SRC      : [%u]\r\n"
                            "\t INT_LD_TRANS_SRC            : [%u]\r\n"
                            "\t INT_TML_SRC                 : [%u]\r\n"
                            "\t INT_SHORT_SRC               : [%u]\r\n"
                            "\t INT_OPEN_SRC                : [%u]\r\n"
                            "\t INT_FB_SRC                  : [%u]\r\n"
                            "\t INT_LD                      : [%u]\r\n"
                            "\t INT_FAULT                   : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r19.int_timeout_err_src, gt_xc24_regs._r19.int_fault_rec_fail_src, gt_xc24_regs._r19.int_fault_auto_rec_fail_src,
                gt_xc24_regs._r19.int_local_rec_fail_src, gt_xc24_regs._r19.int_ld_trans_src, gt_xc24_regs._r19.int_tml_src, gt_xc24_regs._r19.int_short_src, gt_xc24_regs._r19.int_open_src,
                gt_xc24_regs._r19.int_fb_src, gt_xc24_regs._r19.int_ld, gt_xc24_regs._r19.int_fault, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_FB_PWM_PERIOD:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t FB_PWM_EN                   : [%u]\r\n"
                            "\t FB_PWM_POL                  : [%u]\r\n"
                            "\t T_FB_EN                     : [%u]\r\n"
                            "\t FB_PWM_PERIOD               : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r1A.fb_pwm_en, gt_xc24_regs._r1A.fb_pwm_pol, gt_xc24_regs._r1A.t_fb_en, gt_xc24_regs._r1A.fb_pwm_period, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_FB_PWM_DUTY_1:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t FB_PWM_DUTY_INC             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r1B.fb_pwm_duty_inc, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_FB_PWM_DUTY_2:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t FB_PWM_DUTY_WAIT            : [%u]\r\n"
                            "\t FB_PWM_DUTY_DEC             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r1C.fb_pwm_duty_wait, gt_xc24_regs._r1C.fb_pwm_duty_dec, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_FB_PWM_DUTY_3:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t FB_PWM_DUTY                 : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r1D.fb_pwm_duty, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_COMMAND_LATENCY:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t CMD_LATENCY                 : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r1F.cmd_latency, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_3       : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_2       : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_1       : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r20.daisied_dev_ch_size_3, gt_xc24_regs._r20.daisied_dev_ch_size_2, gt_xc24_regs._r20.daisied_dev_ch_size_1, gt_xc24_regs.ALL[xc_addr]);
            break;
            case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE2:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_6       : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_5       : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_4       : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r21.daisied_dev_ch_size_6, gt_xc24_regs._r21.daisied_dev_ch_size_5, gt_xc24_regs._r21.daisied_dev_ch_size_4, gt_xc24_regs.ALL[xc_addr]);
                break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE3:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_9       : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_8       : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_7       : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r22.daisied_dev_ch_size_9, gt_xc24_regs._r22.daisied_dev_ch_size_8, gt_xc24_regs._r22.daisied_dev_ch_size_7, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE4:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_12      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_11      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_10      : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r23.daisied_dev_ch_size_12, gt_xc24_regs._r23.daisied_dev_ch_size_11, gt_xc24_regs._r23.daisied_dev_ch_size_10, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE5:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_15      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_14      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_13      : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r24.daisied_dev_ch_size_15, gt_xc24_regs._r24.daisied_dev_ch_size_14, gt_xc24_regs._r24.daisied_dev_ch_size_13, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE6:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_18      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_17      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_16      : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r25.daisied_dev_ch_size_18, gt_xc24_regs._r25.daisied_dev_ch_size_17, gt_xc24_regs._r25.daisied_dev_ch_size_16, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE7:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_21      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_20      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_19      : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r26.daisied_dev_ch_size_21, gt_xc24_regs._r26.daisied_dev_ch_size_20, gt_xc24_regs._r26.daisied_dev_ch_size_19, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE8:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_24      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_23      : [%u]\r\n"
                            "\t DAISIED_DEV_CH_SIZE_22      : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r27.daisied_dev_ch_size_24, gt_xc24_regs._r27.daisied_dev_ch_size_23, gt_xc24_regs._r27.daisied_dev_ch_size_22, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_LDO:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t LDO                         : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r2A.ldo, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISY_SIZE1:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISY_SIZE_CH3              : [%u]\r\n"
                            "\t DAISY_SIZE_CH2              : [%u]\r\n"
                            "\t DAISY_SIZE_CH1              : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r30.daisy_size_ch3, gt_xc24_regs._r30.daisy_size_ch2, gt_xc24_regs._r30.daisy_size_ch1, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISY_SIZE2:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISY_SIZE_CH6              : [%u]\r\n"
                            "\t DAISY_SIZE_CH5              : [%u]\r\n"
                            "\t DAISY_SIZE_CH4              : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r31.daisy_size_ch6, gt_xc24_regs._r31.daisy_size_ch5, gt_xc24_regs._r31.daisy_size_ch4, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISY_SIZE3:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISY_SIZE_CH9              : [%u]\r\n"
                            "\t DAISY_SIZE_CH8              : [%u]\r\n"
                            "\t DAISY_SIZE_CH7              : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r32.daisy_size_ch9, gt_xc24_regs._r32.daisy_size_ch8, gt_xc24_regs._r32.daisy_size_ch7, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISY_SIZE4:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISY_SIZE_CH12             : [%u]\r\n"
                            "\t DAISY_SIZE_CH11             : [%u]\r\n"
                            "\t DAISY_SIZE_CH10             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r33.daisy_size_ch12, gt_xc24_regs._r33.daisy_size_ch11, gt_xc24_regs._r33.daisy_size_ch10, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISY_SIZE5:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISY_SIZE_CH15             : [%u]\r\n"
                            "\t DAISY_SIZE_CH14             : [%u]\r\n"
                            "\t DAISY_SIZE_CH13             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r34.daisy_size_ch15, gt_xc24_regs._r34.daisy_size_ch14, gt_xc24_regs._r34.daisy_size_ch13, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISY_SIZE6:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISY_SIZE_CH18             : [%u]\r\n"
                            "\t DAISY_SIZE_CH17             : [%u]\r\n"
                            "\t DAISY_SIZE_CH16             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r35.daisy_size_ch18, gt_xc24_regs._r35.daisy_size_ch17, gt_xc24_regs._r35.daisy_size_ch16, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISY_SIZE7:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISY_SIZE_CH21             : [%u]\r\n"
                            "\t DAISY_SIZE_CH20             : [%u]\r\n"
                            "\t DAISY_SIZE_CH19             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r36.daisy_size_ch21, gt_xc24_regs._r36.daisy_size_ch20, gt_xc24_regs._r36.daisy_size_ch19, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_DAISY_SIZE8:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DAISY_SIZE_CH24             : [%u]\r\n"
                            "\t DAISY_SIZE_CH23             : [%u]\r\n"
                            "\t DAISY_SIZE_CH22             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r37.daisy_size_ch24, gt_xc24_regs._r37.daisy_size_ch23, gt_xc24_regs._r37.daisy_size_ch22, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE1:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH2              : [%u]\r\n"
                            "\t BLOCK_SIZE_CH1              : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r38.block_size_ch2, gt_xc24_regs._r38.block_size_ch1, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE2:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH4              : [%u]\r\n"
                            "\t BLOCK_SIZE_CH3              : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r39.block_size_ch4, gt_xc24_regs._r39.block_size_ch3, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE3:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH6              : [%u]\r\n"
                            "\t BLOCK_SIZE_CH5              : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r3A.block_size_ch6, gt_xc24_regs._r3A.block_size_ch5, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE4:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH8              : [%u]\r\n"
                            "\t BLOCK_SIZE_CH7              : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r3B.block_size_ch8, gt_xc24_regs._r3B.block_size_ch7, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE5:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH10             : [%u]\r\n"
                            "\t BLOCK_SIZE_CH9              : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r3C.block_size_ch10, gt_xc24_regs._r3C.block_size_ch9, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE6:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH12             : [%u]\r\n"
                            "\t BLOCK_SIZE_CH11             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r3D.block_size_ch12, gt_xc24_regs._r3D.block_size_ch11, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE7:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH14             : [%u]\r\n"
                            "\t BLOCK_SIZE_CH13             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r3E.block_size_ch14, gt_xc24_regs._r3E.block_size_ch13, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE8:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH16             : [%u]\r\n"
                            "\t BLOCK_SIZE_CH15             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r3F.block_size_ch16, gt_xc24_regs._r3F.block_size_ch15, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE9:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH18             : [%u]\r\n"
                            "\t BLOCK_SIZE_CH17             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r40.block_size_ch18, gt_xc24_regs._r40.block_size_ch17, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE10:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH20             : [%u]\r\n"
                            "\t BLOCK_SIZE_CH19             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r41.block_size_ch20, gt_xc24_regs._r41.block_size_ch19, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE11:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH22             : [%u]\r\n"
                            "\t BLOCK_SIZE_CH21             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r42.block_size_ch22, gt_xc24_regs._r42.block_size_ch21, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_BLOCK_SIZE12:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t BLOCK_SIZE_CH24             : [%u]\r\n"
                            "\t BLOCK_SIZE_CH23             : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r43.block_size_ch24, gt_xc24_regs._r43.block_size_ch23, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_CHANNEL_ENABLE1:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t CH16_EN                     : [%u]\r\n"
                            "\t CH15_EN                     : [%u]\r\n"
                            "\t CH14_EN                     : [%u]\r\n"
                            "\t CH13_EN                     : [%u]\r\n"
                            "\t CH12_EN                     : [%u]\r\n"
                            "\t CH11_EN                     : [%u]\r\n"
                            "\t CH10_EN                     : [%u]\r\n"
                            "\t CH9_EN                      : [%u]\r\n"
                            "\t CH8_EN                      : [%u]\r\n"
                            "\t CH7_EN                      : [%u]\r\n"
                            "\t CH6_EN                      : [%u]\r\n"
                            "\t CH5_EN                      : [%u]\r\n"
                            "\t CH4_EN                      : [%u]\r\n"
                            "\t CH3_EN                      : [%u]\r\n"
                            "\t CH2_EN                      : [%u]\r\n"
                            "\t CH1_EN                      : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r45.ch16_en, gt_xc24_regs._r45.ch15_en, gt_xc24_regs._r45.ch14_en, gt_xc24_regs._r45.ch13_en,
                gt_xc24_regs._r45.ch12_en, gt_xc24_regs._r45.ch11_en, gt_xc24_regs._r45.ch10_en, gt_xc24_regs._r45.ch9_en, gt_xc24_regs._r45.ch8_en,
                gt_xc24_regs._r45.ch7_en, gt_xc24_regs._r45.ch6_en, gt_xc24_regs._r45.ch5_en, gt_xc24_regs._r45.ch4_en, gt_xc24_regs._r45.ch3_en,
                gt_xc24_regs._r45.ch2_en, gt_xc24_regs._r45.ch1_en, gt_xc24_regs.ALL[xc_addr]);
            break;
        case XC24_ADDR_CHANNEL_ENABLE2:
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t CH_SIZE                     : [%u]\r\n"
                            "\t CH24_EN                     : [%u]\r\n"
                            "\t LD_WIDTH                    : [%u]\r\n"
                            "\t CH23_EN                     : [%u]\r\n"
                            "\t CH22_EN                     : [%u]\r\n"
                            "\t CH21_EN                     : [%u]\r\n"
                            "\t CH20_EN                     : [%u]\r\n"
                            "\t CH19_EN                     : [%u]\r\n"
                            "\t CH18_EN                     : [%u]\r\n"
                            "\t CH17_EN                     : [%u]\r\n"
                            "\t VALUE                       : (0x%04X)\r\n\r\n",
                gs_xc24_addr_str[xc_addr], xc_addr, gt_xc24_regs._r46.ld_width, gt_xc24_regs._r46.ch_size, gt_xc24_regs._r46.ch24_en, gt_xc24_regs._r46.ch23_en,
                gt_xc24_regs._r46.ch22_en, gt_xc24_regs._r46.ch21_en, gt_xc24_regs._r46.ch20_en, gt_xc24_regs._r46.ch19_en, gt_xc24_regs._r46.ch18_en,
                gt_xc24_regs._r46.ch17_en, gt_xc24_regs.ALL[xc_addr]);
            break;
        }
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
        gt_xc24_regs.ALL[xc_addr] = 0x00;

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
            gt_xc24_regs._r08.sync_auto_en = 0;
            gt_xc24_regs._r08.fault_auto_en = 0;
            break;
        case XC24_ADDR_INTERRUPT_ENABLE :
            gt_xc24_regs._r15.int_fb_en = 1;
            break;
        default :
            continue;
        }
        XC24_Write_Register(xc_addr, gt_xc24_regs.ALL[xc_addr]);
        us_tdelay(XD12_WRITE_DELAY);
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
    /* XC24 -> XD12 Set ID GEN */
    gt_xc24_regs._r04.ALL = 0;
    gt_xc24_regs._r04.start = 1;
    XC24_Write_Register(XC24_ADDR_ID_GEN, gt_xc24_regs._r04.ALL);
    us_tdelay(XD12_IDGEN_DELAY);
}

void XC24_IF_SyncGen_Command(void)
{
    /* XC24 -> XD12 Set Sync GEN */
    gt_xc24_regs._r07.ALL = 0;
    gt_xc24_regs._r07.start = 1;
    XC24_Write_Register(XC24_ADDR_SYNC_GEN, gt_xc24_regs._r07.ALL);
    us_tdelay(XD12_IDGEN_DELAY);
}

uint16_t XC24_IF_Fault_Read_Command(void)
{
    /* XC24 -> XD12 Set Fault GEN */
    gt_xc24_regs._r05.ALL = 0;
    gt_xc24_regs._r05.start = 1;
    XC24_Write_Register(XC24_ADDR_FAULT_READ, gt_xc24_regs._r05.ALL);
    us_tdelay(XD12_IDGEN_DELAY);

    gt_xc24_fault._d1.ALL = XC24_Read_Register(XC24_ADDR_GLOBAL_FAULT_READ_DATA1);

    return gt_xc24_fault._d1.ALL;
}

void XC24_IF_Write_XD12(uint8_t in_XD12_addr, uint16_t in_XD12_data)
{
    gt_xc24_regs._r01.ALL = 0;
    gt_xc24_regs._r01.start = 1;
    gt_xc24_regs._r01.addr = in_XD12_addr;

    //print(LOG_DEBUG, "\r\n========== XC -> XD Write Start (0x%02X) - [%u] [0x%4X] ==========\r\n", in_XD12_addr, in_XD12_data, in_XD12_data);

    /* 1st - Write the XD12_Data on the GLOBAL_WRITE_DATA Register of XC24 */
    XC24_Write_Register(XC24_ADDR_GLOBAL_WRITE_DATA, in_XD12_data);
    us_tdelay(1);

    /* 2nd - Write the XD12_Addr on the GLOBAL_WRITE_COMMAND Register of XC24 */
    XC24_Write_Register(XC24_ADDR_GLOBAL_WRITE, gt_xc24_regs._r01.ALL);
    /* To Do : delay must more than 65us */
    us_tdelay(XD12_WRITE_DELAY);

    //print(LOG_DEBUG, "========== XC -> XD  Write Done (0x%02X) - [%u] [0x%4X] ==========\r\n\r\n", in_XD12_addr, in_XD12_data, in_XD12_data);
}

uint16_t XC24_IF_Read_XD12(uint8_t in_XD12_addr)
{
    /* XC24 -> XD12 Set Read Command */
    uint16_t u16_XD12_data = 0;
    uint16_t u16_XD12_r = 0;
    uint32_t count = 5;

    //print(LOG_DEBUG, "\r\n========== XC -> XD Read Start (0x%02X) ==========\r\n", in_XD12_addr);

    /* 1st - Send Local RW Pointer Reset */
    gt_xc24_regs._r11.ALL = 0;
    gt_xc24_regs._r11.local_wr_pointer_rst = 1;
    gt_xc24_regs._r11.local_rd_pointer_rst = 1;
    XC24_Write_Register(XC24_ADDR_LOCAL_RW_POINTER_RESET, gt_xc24_regs._r11.ALL);

    us_tdelay(1);

    while(count)
    {
        u16_XD12_r = XC24_Read_Register(XC24_ADDR_LOCAL_RD_RECEIVE_POINTER);

        --count;

        if (u16_XD12_r == 0)
        {
            us_tdelay(1);
            break;
        }
        else
        {
            //print(LOG_DEBUG, "==============> LOCAL POINTER(%u) : 0x%04X <==============\r\n", count, u16_XD12_r);
            XC24_Write_Register(XC24_ADDR_LOCAL_RW_POINTER_RESET, gt_xc24_regs._r11.ALL);
            us_tdelay(1);
        }
    }

    /* 2nd - Write the XD12_Addr on the LOCAL_READ Register of XC24 */
    gt_xc24_regs._r03.ALL = 0;
    gt_xc24_regs._r03.start = 1;
    gt_xc24_regs._r03.ch_seg = 0;
    gt_xc24_regs._r03.addr = in_XD12_addr;
    XC24_Write_Register(XC24_ADDR_LOCAL_READ, gt_xc24_regs._r03.ALL);

    // Wait Ack
    us_tdelay(XD12_READ_DELAY + XD12_READ_RECV_DELAY);

    /* 3rd - Receive Data XD12_Data through XC24 */
    u16_XD12_data = XC24_Read_Register(XC24_ADDR_PORT1_LOCAL_RW_DATA1);
    //print(LOG_DEBUG, "XC24_IF_Read_XD12(0x%02X) - [%u] [0x%04X] \r\n",in_XD12_addr, u16_XD12_data, u16_XD12_data);

    //print(LOG_DEBUG, "\r\n========== XC -> XD Read  Done (0x%02X) ==========\r\n", in_XD12_addr);

    return u16_XD12_data;
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