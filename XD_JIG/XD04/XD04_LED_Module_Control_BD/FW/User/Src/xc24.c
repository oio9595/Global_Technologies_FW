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
#include "vsync_task.h"
#include "config.h"

#define XDIC_LD_MAX         (16383U)

#define XC24_GENERAL_REG_ENTRY(addr, reg)   { addr, #addr, &gt_xc24_general_regs.reg }
#define XC24_MIRROR_REG_ENTRY(addr, reg)    { addr, #addr, &gt_xc24_mirror_regs.reg }

#define XC24_OSC_REG_DEFAULT        (64)
#define XC_SPI_TIMEOUT_MS           (10)
#define XC24_OTP_PROTECT_DISABLE    (0xA5A)
#define XC24_OTP_PROTECT_ENABLE     (0x5A5)

static SPI_TypeDef *g_hSPIx;
volatile uint8_t gn_xc_spi_timeout;

static _xc24_general_regs_t gt_xc24_general_regs;
static _xc24_mirror_regs_t gt_xc24_mirror_regs;

static _reg_map_t gt_xc24_general_maps[] =
{
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_SOFT_RESET                  , _r00 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_GLOBAL_WRITE                , _r01 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LOCAL_WRITE                 , _r02 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LOCAL_READ                  , _r03 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_ID_GEN                      , _r04 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_FAULT_READ                  , _r05 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LD_TRANSFER                 , _r06 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_SYNC_GEN                    , _r07 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_AUTO_ENABLE                 , _r08 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_09                    , _r09 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LD_WRITE_POINTER            , _r0A ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LD_READ_POINTER             , _r0B ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DIFFERENCE_POINTER          , _r0C ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LD_TRANSFER_START_POINTER_TH, _r0D ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LOCAL_WR_TRANSFER_POINTER   , _r0E ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LOCAL_RD_RECEIVE_POINTER    , _r0F ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LOCAL_RW_DIFFERENCE_POINTER , _r10 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_LOCAL_RW_POINTER_RESET      , _r11 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_FAULT_AUTO_READ_TIMER       , _r12 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_FAULT_AUTO_READ_EVENT       , _r13 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_SERIALIZER_CLOCK_GEN        , _r14 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_INTERRUPT_ENABLE            , _r15 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_COMMAND_STATUS1             , _r16 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_COMMAND_STATUS2             , _r17 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_RECEIVE_STATUS              , _r18 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_INTERRUPT_STATUS            , _r19 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_SPI_FAULT_STATUS_CONTROL    , _r1A ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_CLK_CONTROL_1               , _r1B ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_CLK_CONTROL_2               , _r1C ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_VDD_LDO_STATUS              , _r1D ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_VDD_LDO_FAULT_LEVEL         , _r1E ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_COMMAND_LATENCY             , _r1F ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1, _r20 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE2, _r21 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE3, _r22 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE4, _r23 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE5, _r24 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE6, _r25 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE7, _r26 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE8, _r27 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_28                    , _r28 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_29                    , _r29 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_2A                    , _r2A ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_2B                    , _r2B ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_2C                    , _r2C ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_2D                    , _r2D ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_2E                    , _r2E ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_2F                    , _r2F ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISY_SIZE1                 , _r30 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISY_SIZE2                 , _r31 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISY_SIZE3                 , _r32 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISY_SIZE4                 , _r33 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISY_SIZE5                 , _r34 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISY_SIZE6                 , _r35 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISY_SIZE7                 , _r36 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAISY_SIZE8                 , _r37 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE1                 , _r38 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE2                 , _r39 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE3                 , _r3A ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE4                 , _r3B ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE5                 , _r3C ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE6                 , _r3D ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE7                 , _r3E ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE8                 , _r3F ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE9                 , _r40 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE10                , _r41 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE11                , _r42 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_BLOCK_SIZE12                , _r43 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_44                    , _r44 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_CHANNEL_ENABLE1             , _r45 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_CHANNEL_ENABLE2             , _r46 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_47                    , _r47 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_48                    , _r48 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_49                    , _r49 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_4A                    , _r4A ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_4B                    , _r4B ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_4C                    , _r4C ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_CONTROL                 , _r4D ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_4E                    , _r4E ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_CURRENT_TARGET_DAC          , _r4F ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_PREVIOUS_TARGET_DAC         , _r50 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_OUT                     , _r51 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_INCREMENT1              , _r52 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_INCREMENT2_HOLD_LIMIT   , _r53 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_DECREMENT_INC_WAIT      , _r54 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_INCREMENT_HOLD_TH       , _r55 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_SOA_N11_N1                  , _r56 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_SOA_P2_P1                   , _r57 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_SOA_P3_P2                   , _r58 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_FB_VALID_TIMER          , _r59 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_MIN_LIMIT               , _r5A ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_MAX_LIMIT               , _r5B ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_STATE                   , _r5C ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_VALID_CNT                   , _r5D ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DAC_INC_HOLD_WAIT_CNT       , _r5E ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_R2                          , _r5F ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_GLOBAL_WRITE_DATA           , _r60 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_GLOBAL_FAULT_READ_DATA1     , _r61 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_GLOBAL_FAULT_READ_DATA2     , _r62 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_GLOBAL_FAULT_READ_DATA3     , _r63 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_GLOBAL_FAULT_READ_DATA4     , _r64 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_GLOBAL_FAULT_READ_DATA5     , _r65 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_GLOBAL_FAULT_READ_DATA6     , _r66 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_67                    , _r67 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_68                    , _r68 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_69                    , _r69 ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_6A                    , _r6A ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_6B                    , _r6B ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_6C                    , _r6C ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_6D                    , _r6D ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_6E                    , _r6E ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_DUMMY_6F                    , _r6F ),
    XC24_GENERAL_REG_ENTRY( XC24_ADDR_PORT1_LOCAL_RW_DATA1        , _r70 ),
};
static_assert(XC24_ADDR_MAX == (sizeof(gt_xc24_general_maps) / sizeof(_reg_map_t)), "XC24 General Address map mismatch!");

static _reg_map_t gt_xc24_mirror_maps[] =
{
    XC24_MIRROR_REG_ENTRY( XC24_MIRROR_ADDR_TEST_CONTROL , _rF0 ),
    XC24_MIRROR_REG_ENTRY( XC24_MIRROR_ADDR_OTP_PG_ACCESS, _rF1 ),
    XC24_MIRROR_REG_ENTRY( XC24_MIRROR_ADDR_OTP_WRITE    , _rF2 ),
    XC24_MIRROR_REG_ENTRY( XC24_MIRROR_ADDR_OTP_RD_PROG  , _rF3 ),
    XC24_MIRROR_REG_ENTRY( XC24_MIRROR_ADDR_OTP_PROTECT  , _rF4 ),
    XC24_MIRROR_REG_ENTRY( XC24_MIRROR_ADDR_MIRROR1      , _rF5 ),
    XC24_MIRROR_REG_ENTRY( XC24_MIRROR_ADDR_MIRROR2      , _rF6 ),
    XC24_MIRROR_REG_ENTRY( XC24_MIRROR_ADDR_MIRROR3      , _rF7 ),
};
static_assert((XC24_MIRROR_ADDR_MAX - XC24_MIRROR_ADDR_START) == (sizeof(gt_xc24_mirror_maps) / sizeof(_reg_map_t)), "XC24 Mirror Address map mismatch!");

__STATIC_INLINE bool SPI_Timeout_Handler(void)
{
    if (gn_xc_spi_timeout == 0)
    {
        print(LOG_PC, "SPI Timeout Error\r\n");
        return false;
    }
    return true;
}

__STATIC_INLINE void SPI_Write(SPI_TypeDef *SPIx, uint16_t* p_buffer, uint16_t len)
{
    gn_xc_spi_timeout = XC_SPI_TIMEOUT_MS;
    XC_NSCS_LO();

    if (LL_SPI_IsEnabled(SPIx) != 1)
    {
        LL_SPI_Enable(SPIx);
    }

    for (volatile uint16_t i = 0 ; i < len ; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx))
        {
            if (!SPI_Timeout_Handler()) return;
        }
        LL_SPI_TransmitData16(SPIx, p_buffer[i]);
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx))
    {
        if (!SPI_Timeout_Handler()) return;
    };
    us_delay(1);

    XC_NSCS_HI();
}

__STATIC_INLINE void SPI_Read(SPI_TypeDef *SPIx, uint16_t* p_tx_buffer, uint16_t* p_rx_buffer, uint16_t len)
{
    gn_xc_spi_timeout = XC_SPI_TIMEOUT_MS;
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
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx))
        {
            if (!SPI_Timeout_Handler()) return;
        }
        LL_SPI_TransmitData16(SPIx, p_tx_buffer[i]);

        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx))
        {
            if (!SPI_Timeout_Handler()) return;
        }
        p_rx_buffer[i] = LL_SPI_ReceiveData16(SPIx);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx))
    {
        if (!SPI_Timeout_Handler()) return;
    }
    us_delay(1);

    XC_NSCS_HI();
}

static const _reg_map_t* XC24_Get_General_Map_Pointer(uint8_t addr)
{
    for (uint8_t i = 0; i < sizeof(gt_xc24_general_maps) / sizeof(gt_xc24_general_maps[0]); ++i)
    {
        if (gt_xc24_general_maps[i].address == addr)
        {
            return &gt_xc24_general_maps[i];
        }
    }
    return NULL;
}

static const _reg_map_t* XC24_Get_Mirror_Map_Pointer(uint8_t addr)
{
    for (uint8_t i = 0; i < sizeof(gt_xc24_mirror_maps) / sizeof(gt_xc24_mirror_maps[0]); ++i)
    {
        if (gt_xc24_mirror_maps[i].address == addr)
        {
            return &gt_xc24_mirror_maps[i];
        }
    }
    return NULL;
}

static const _reg_map_t* XC24_Find_Register_Map(uint8_t addr)
{
    const _reg_map_t* map = XC24_Get_General_Map_Pointer(addr);
    if (!map)
    {
        map = XC24_Get_Mirror_Map_Pointer(addr);
    }

    return map;
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

    const _reg_map_t* xc24_map = XC24_Find_Register_Map(in_addr);
    if (xc24_map)
    {
        *((uint16_t*)(xc24_map->reg_ptr)) = in_data;
    }
    else
    {
        print(LOG_PC, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, in_addr);
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

    const _reg_map_t* xc24_map = XC24_Find_Register_Map(in_addr);
    if (xc24_map)
    {
        *((uint16_t*)(xc24_map->reg_ptr)) = rx_buffer[1];
    }
    else
    {
        print(LOG_PC, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, in_addr);
    }

    return rx_buffer[1];
}

void XC24_Read_Register_All(void)
{
    for (uint8_t xc_addr = XC24_ADDR_SOFT_RESET ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        XC24_Read_Register(xc_addr);
        us_delay(10);
    }
    XC24_Dump_All_Register();
}

void XC24_Dump_All_Register(void)
{
    for (uint8_t xc_addr = XC24_ADDR_SOFT_RESET ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24_Get_General_Map_Pointer(xc_addr);
        if (map)
        {
            print(LOG_PC, "[%s (0x%02X)]\r\n\t VALUE : %s(0x%04X)%s\r\n\r\n", map->name, map->address, ANSI_FONT_MAGENTA, *((uint16_t*)(map->reg_ptr)), ANSI_FONT_NONE);
        }
    }
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
    XC24_Write_Register(XC24_MIRROR_ADDR_OTP_PROTECT, gt_xc24_mirror_regs._rF4.ALL);
}

void XC24_Test_Init_OSC(void)
{
	gt_xc24_mirror_regs._rF0.test_en = 1;
    gt_xc24_mirror_regs._rF0.mclk32_o = 1;
	gt_xc24_mirror_regs._rF0.mclk1_o = 0;
	XC24_Write_Register(XC24_MIRROR_ADDR_TEST_CONTROL, gt_xc24_mirror_regs._rF0.ALL);

	gt_xc24_general_regs._r1B.spread_range_a = 4;
	gt_xc24_general_regs._r1B.osc_spread_en = 0;
	XC24_Write_Register(XC24_ADDR_CLK_CONTROL_1, gt_xc24_general_regs._r1B.ALL);

	gt_xc24_general_regs._r1C.osc_force_static = XC24_OSC_REG_DEFAULT;
	XC24_Write_Register(XC24_ADDR_CLK_CONTROL_2, gt_xc24_general_regs._r1C.ALL);
}

void XC24_Init(void)
{
    g_hSPIx = SPI1;

    XC_VCC_ON();

    LL_mDelay(20);

    XC_NSCS_HI();

    for (uint8_t xc_addr = XC24_ADDR_SOFT_RESET ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24_Find_Register_Map(xc_addr);
        if (map)
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
                gt_xc24_general_regs._r08.sync_auto_en = 1;
                gt_xc24_general_regs._r08.fault_auto_en = 1;
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
            case XC24_ADDR_CLK_CONTROL_1:
                gt_xc24_general_regs._r1B.serializer_clk_sel = 0;
                gt_xc24_general_regs._r1B.ld_b_rd_clk_sel = 0;
                gt_xc24_general_regs._r1B.osc_spread_en = 0;
                break;
            case XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1 :
                gt_xc24_general_regs._r20.daisied_dev_ch_size_1 = XD_CH_SIZE;
                break;
            case XC24_ADDR_DAISY_SIZE1 :
                gt_xc24_general_regs._r30.daisy_size_ch1 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r30.daisy_size_ch2 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r30.daisy_size_ch3 = XD_DAISY_SIZE;
                break;
            case XC24_ADDR_DAISY_SIZE2 :
                gt_xc24_general_regs._r31.daisy_size_ch4 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r31.daisy_size_ch5 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r31.daisy_size_ch6 = XD_DAISY_SIZE;
                break;
            case XC24_ADDR_DAISY_SIZE3 :
                gt_xc24_general_regs._r32.daisy_size_ch7 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r32.daisy_size_ch8 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r32.daisy_size_ch9 = XD_DAISY_SIZE;
                break;
            case XC24_ADDR_DAISY_SIZE4 :
                gt_xc24_general_regs._r33.daisy_size_ch10 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r33.daisy_size_ch11 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r33.daisy_size_ch12 = XD_DAISY_SIZE;
                break;
            case XC24_ADDR_DAISY_SIZE5 :
                gt_xc24_general_regs._r34.daisy_size_ch13 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r34.daisy_size_ch14 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r34.daisy_size_ch15 = XD_DAISY_SIZE;
                break;
            case XC24_ADDR_DAISY_SIZE6 :
                gt_xc24_general_regs._r35.daisy_size_ch16 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r35.daisy_size_ch17 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r35.daisy_size_ch18 = XD_DAISY_SIZE;
                break;
            case XC24_ADDR_DAISY_SIZE7 :
                gt_xc24_general_regs._r36.daisy_size_ch19 = XD_DAISY_SIZE;
                gt_xc24_general_regs._r36.daisy_size_ch20 = XD_DAISY_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE1 :
                gt_xc24_general_regs._r38.block_size_ch1 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r38.block_size_ch2 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE2 :
                gt_xc24_general_regs._r39.block_size_ch3 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r39.block_size_ch4 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE3 :
                gt_xc24_general_regs._r3A.block_size_ch5 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r3A.block_size_ch6 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE4 :
                gt_xc24_general_regs._r3B.block_size_ch7 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r3B.block_size_ch8 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE5 :
                gt_xc24_general_regs._r3C.block_size_ch9 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r3C.block_size_ch10 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE6 :
                gt_xc24_general_regs._r3D.block_size_ch11 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r3D.block_size_ch12 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE7 :
                gt_xc24_general_regs._r3E.block_size_ch13 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r3E.block_size_ch14 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE8 :
                gt_xc24_general_regs._r3F.block_size_ch15 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r3F.block_size_ch16 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE9 :
                gt_xc24_general_regs._r40.block_size_ch17 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r40.block_size_ch18 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_BLOCK_SIZE10 :
                gt_xc24_general_regs._r41.block_size_ch19 = XD_DAISY_SIZE * XD_CH_SIZE;
                gt_xc24_general_regs._r41.block_size_ch20 = XD_DAISY_SIZE * XD_CH_SIZE;
                break;
            case XC24_ADDR_CHANNEL_ENABLE1 :
                gt_xc24_general_regs._r45.ch1_en = 1;
                gt_xc24_general_regs._r45.ch2_en = 1;
                gt_xc24_general_regs._r45.ch3_en = 1;
                gt_xc24_general_regs._r45.ch4_en = 1;
                gt_xc24_general_regs._r45.ch5_en = 1;
                gt_xc24_general_regs._r45.ch6_en = 1;
                gt_xc24_general_regs._r45.ch7_en = 1;
                gt_xc24_general_regs._r45.ch8_en = 1;
                gt_xc24_general_regs._r45.ch9_en = 1;
                gt_xc24_general_regs._r45.ch10_en = 1;
                gt_xc24_general_regs._r45.ch11_en = 1;
                gt_xc24_general_regs._r45.ch12_en = 1;
                gt_xc24_general_regs._r45.ch13_en = 1;
                gt_xc24_general_regs._r45.ch14_en = 1;
                gt_xc24_general_regs._r45.ch15_en = 1;
                gt_xc24_general_regs._r45.ch16_en = 1;
                break;
            case XC24_ADDR_CHANNEL_ENABLE2 :
                gt_xc24_general_regs._r46.ch17_en = 1;
                gt_xc24_general_regs._r46.ch18_en = 1;
                gt_xc24_general_regs._r46.ch19_en = 1;
                gt_xc24_general_regs._r46.ch20_en = 1;
                gt_xc24_general_regs._r46.ld_size = 20;
                gt_xc24_general_regs._r46.ld_width = 3;
                break;
            default :
                continue;
            }
            XC24_Write_Register(map->address, *((uint16_t*)(map->reg_ptr)));
            us_delay(10);
        }
    }
    XC24_Write_Register(XC24_ADDR_SOFT_RESET, 0x30);
    XC24_Write_Register(XC24_MIRROR_ADDR_OTP_RD_PROG, 2);

    print(LOG_PC, " ...XC24 Initial Done...\r\n");
    XC24_Read_Register_All();
}

void XC24_DeInit(void)
{
    g_hSPIx = SPI1;

    for (uint8_t xc_addr = XC24_ADDR_SOFT_RESET ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24_Find_Register_Map(xc_addr);
        if (map)
        {
            switch (xc_addr)
            {
            case XC24_ADDR_SOFT_RESET:
                gt_xc24_general_regs._r00.rst1 = 1;
                gt_xc24_general_regs._r00.rst2 = 1;
                gt_xc24_general_regs._r00.rst3 = 1;
                break;
            case XC24_ADDR_GLOBAL_WRITE_DATA :
                gt_xc24_general_regs._r60.ALL = 0; // To reset SPI CLK, MOSI
                break;
            default :
                continue;
            }
            XC24_Write_Register(map->address, *((uint16_t*)(map->reg_ptr)));
            us_delay(10);
        }
    }

    XC_NSCS_LO();
    XC_VCC_OFF();
}

void XC24_Test_Init(void)
{
    g_hSPIx = SPI1;

    XC_VCC_ON();

    LL_mDelay(20);

    XC_NSCS_HI();

    for (uint8_t xc_addr = XC24_ADDR_SOFT_RESET ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24_Find_Register_Map(xc_addr);
        if (map)
        {
            switch (xc_addr)
            {
            case XC24_ADDR_SOFT_RESET:
                gt_xc24_general_regs._r00.rst1 = 1;
                gt_xc24_general_regs._r00.rst2 = 1;
                gt_xc24_general_regs._r00.rst3 = 1;
                break;
                break;
            default :
                continue;
            }
            XC24_Write_Register(map->address, *((uint16_t*)(map->reg_ptr)));
            us_delay(10);
        }
    }

    for (uint8_t xc_addr = XC24_MIRROR_ADDR_START ; xc_addr < XC24_MIRROR_ADDR_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24_Find_Register_Map(xc_addr);
        if (map)
        {
            switch (xc_addr)
            {
            case XC24_MIRROR_ADDR_TEST_CONTROL:
                gt_xc24_mirror_regs._rF0.test_en = 1;
                break;
            default :
                continue;
            }
            XC24_Write_Register(map->address, *((uint16_t*)(map->reg_ptr)));
            us_delay(10);
        }
    }
    XC24_Set_OTP_Protect(false);
    XC24_Write_Register(XC24_MIRROR_ADDR_OTP_RD_PROG, 2);
    XC24_Test_Init_OSC();
    XC24_Read_Register_All();
    print(LOG_PC, " ...XC24 Test Initial Done...\r\n");
}

/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/

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
    us_delay(XDIC_WRITE_DELAY);
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

void XC24_IF_Write_LD(void)
{
    _xc24_cmd_t cmd_format = {0, };
    uint16_t tx_buffer[1 + TOTAL_BLOCK_SIZE] = {0,};

    cmd_format.code = CMD_CODE_LD_TRANS;
    cmd_format.addr = 0;
    cmd_format.size = XD_DAISY_SIZE * XD_CH_SIZE;

    tx_buffer[0] = cmd_format.ALL;
    for (uint8_t i = 0 ; i < TOTAL_BLOCK_SIZE ; ++i)
    {
        if (gb_xd_led_enable_table[i])
        {
            tx_buffer[i + 1] = XDIC_LD_MAX;
        }
        else
        {
            tx_buffer[i + 1] = 0;
        }
    }

    SPI_Write(g_hSPIx, tx_buffer, 1 + TOTAL_BLOCK_SIZE);
    us_delay(XDIC_LD_TRANS_DELAY);
}

/* END - INTERFACE FUNCTIONS ************************************************************************/

/*** end of file ***/