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

#define XC_SPI_TIMEOUT_MS           (100)

#define XC24_OTP_PROTECT_DISABLE    (0xA5A)
#define XC24_OTP_PROTECT_ENABLE     (0x5A5)

static SPI_TypeDef *g_hSPIx;

static bool gb_xc24_support;

volatile uint8_t gn_xc_spi_timeout = 0;

static _xc24_general_regs_t gt_xc24_general_regs;
static _xc24_mirror_regs_t gt_xc24_mirror_regs;

static _reg_map_t gt_xc24_general_maps[] =
{
    { XC24_ADDR_SOFT_RESET                  ,   "XC24_ADDR_SOFT_RESET                  ",   &gt_xc24_general_regs._r00 },
    { XC24_ADDR_GLOBAL_WRITE                ,   "XC24_ADDR_GLOBAL_WRITE                ",   &gt_xc24_general_regs._r01 },
    { XC24_ADDR_LOCAL_WRITE                 ,   "XC24_ADDR_LOCAL_WRITE                 ",   &gt_xc24_general_regs._r02 },
    { XC24_ADDR_LOCAL_READ                  ,   "XC24_ADDR_LOCAL_READ                  ",   &gt_xc24_general_regs._r03 },
    { XC24_ADDR_ID_GEN                      ,   "XC24_ADDR_ID_GEN                      ",   &gt_xc24_general_regs._r04 },
    { XC24_ADDR_FAULT_READ                  ,   "XC24_ADDR_FAULT_READ                  ",   &gt_xc24_general_regs._r05 },
    { XC24_ADDR_LD_TRANSFER                 ,   "XC24_ADDR_LD_TRANSFER                 ",   &gt_xc24_general_regs._r06 },
    { XC24_ADDR_SYNC_GEN                    ,   "XC24_ADDR_SYNC_GEN                    ",   &gt_xc24_general_regs._r07 },
    { XC24_ADDR_AUTO_ENABLE                 ,   "XC24_ADDR_AUTO_ENABLE                 ",   &gt_xc24_general_regs._r08 },
    { XC24_ADDR_DUMMY_09                    ,   "XC24_ADDR_DUMMY_09                    ",   &gt_xc24_general_regs._r09 },
    { XC24_ADDR_LD_WRITE_POINTER            ,   "XC24_ADDR_LD_WRITE_POINTER            ",   &gt_xc24_general_regs._r0A },
    { XC24_ADDR_LD_READ_POINTER             ,   "XC24_ADDR_LD_READ_POINTER             ",   &gt_xc24_general_regs._r0B },
    { XC24_ADDR_DIFFERENCE_POINTER          ,   "XC24_ADDR_DIFFERENCE_POINTER          ",   &gt_xc24_general_regs._r0C },
    { XC24_ADDR_LD_TRANSFER_START_POINTER_TH,   "XC24_ADDR_LD_TRANSFER_START_POINTER_TH",   &gt_xc24_general_regs._r0D },
    { XC24_ADDR_LOCAL_WR_TRANSFER_POINTER   ,   "XC24_ADDR_LOCAL_WR_TRANSFER_POINTER   ",   &gt_xc24_general_regs._r0E },
    { XC24_ADDR_LOCAL_RD_RECEIVE_POINTER    ,   "XC24_ADDR_LOCAL_RD_RECEIVE_POINTER    ",   &gt_xc24_general_regs._r0F },
    { XC24_ADDR_LOCAL_RW_DIFFERENCE_POINTER ,   "XC24_ADDR_LOCAL_RW_DIFFERENCE_POINTER ",   &gt_xc24_general_regs._r10 },
    { XC24_ADDR_LOCAL_RW_POINTER_RESET      ,   "XC24_ADDR_LOCAL_RW_POINTER_RESET      ",   &gt_xc24_general_regs._r11 },
    { XC24_ADDR_FAULT_AUTO_READ_TIMER       ,   "XC24_ADDR_FAULT_AUTO_READ_TIMER       ",   &gt_xc24_general_regs._r12 },
    { XC24_ADDR_FAULT_AUTO_READ_EVENT       ,   "XC24_ADDR_FAULT_AUTO_READ_EVENT       ",   &gt_xc24_general_regs._r13 },
    { XC24_ADDR_SERIALIZER_CLOCK_GEN        ,   "XC24_ADDR_SERIALIZER_CLOCK_GEN        ",   &gt_xc24_general_regs._r14 },
    { XC24_ADDR_INTERRUPT_ENABLE            ,   "XC24_ADDR_INTERRUPT_ENABLE            ",   &gt_xc24_general_regs._r15 },
    { XC24_ADDR_COMMAND_STATUS1             ,   "XC24_ADDR_COMMAND_STATUS1             ",   &gt_xc24_general_regs._r16 },
    { XC24_ADDR_COMMAND_STATUS2             ,   "XC24_ADDR_COMMAND_STATUS2             ",   &gt_xc24_general_regs._r17 },
    { XC24_ADDR_RECEIVE_STATUS              ,   "XC24_ADDR_RECEIVE_STATUS              ",   &gt_xc24_general_regs._r18 },
    { XC24_ADDR_INTERRUPT_STATUS            ,   "XC24_ADDR_INTERRUPT_STATUS            ",   &gt_xc24_general_regs._r19 },
    { XC24_ADDR_SPI_FAULT_STATUS_CONTROL    ,   "XC24_ADDR_SPI_FAULT_STATUS_CONTROL    ",   &gt_xc24_general_regs._r1A },
    { XC24_ADDR_CLK_CONTROL_1               ,   "XC24_ADDR_CLK_CONTROL_1               ",   &gt_xc24_general_regs._r1B },
    { XC24_ADDR_CLK_CONTROL_2               ,   "XC24_ADDR_CLK_CONTROL_2               ",   &gt_xc24_general_regs._r1C },
    { XC24_ADDR_VDD_LDO_STATUS              ,   "XC24_ADDR_VDD_LDO_STATUS              ",   &gt_xc24_general_regs._r1D },
    { XC24_ADDR_VDD_LDO_FAULT_LEVEL         ,   "XC24_ADDR_VDD_LDO_FAULT_LEVEL         ",   &gt_xc24_general_regs._r1E },
    { XC24_ADDR_COMMAND_LATENCY             ,   "XC24_ADDR_COMMAND_LATENCY             ",   &gt_xc24_general_regs._r1F },
    { XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1,   "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1",   &gt_xc24_general_regs._r20 },
    { XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE2,   "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE2",   &gt_xc24_general_regs._r21 },
    { XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE3,   "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE3",   &gt_xc24_general_regs._r22 },
    { XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE4,   "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE4",   &gt_xc24_general_regs._r23 },
    { XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE5,   "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE5",   &gt_xc24_general_regs._r24 },
    { XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE6,   "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE6",   &gt_xc24_general_regs._r25 },
    { XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE7,   "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE7",   &gt_xc24_general_regs._r26 },
    { XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE8,   "XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE8",   &gt_xc24_general_regs._r27 },
    { XC24_ADDR_DUMMY_28                    ,   "XC24_ADDR_DUMMY_28                    ",   &gt_xc24_general_regs._r28 },
    { XC24_ADDR_DUMMY_29                    ,   "XC24_ADDR_DUMMY_29                    ",   &gt_xc24_general_regs._r29 },
    { XC24_ADDR_DUMMY_2A                    ,   "XC24_ADDR_DUMMY_2A                    ",   &gt_xc24_general_regs._r2A },
    { XC24_ADDR_DUMMY_2B                    ,   "XC24_ADDR_DUMMY_2B                    ",   &gt_xc24_general_regs._r2B },
    { XC24_ADDR_DUMMY_2C                    ,   "XC24_ADDR_DUMMY_2C                    ",   &gt_xc24_general_regs._r2C },
    { XC24_ADDR_DUMMY_2D                    ,   "XC24_ADDR_DUMMY_2D                    ",   &gt_xc24_general_regs._r2D },
    { XC24_ADDR_DUMMY_2E                    ,   "XC24_ADDR_DUMMY_2E                    ",   &gt_xc24_general_regs._r2E },
    { XC24_ADDR_DUMMY_2F                    ,   "XC24_ADDR_DUMMY_2F                    ",   &gt_xc24_general_regs._r2F },
    { XC24_ADDR_DAISY_SIZE1                 ,   "XC24_ADDR_DAISY_SIZE1                 ",   &gt_xc24_general_regs._r30 },
    { XC24_ADDR_DAISY_SIZE2                 ,   "XC24_ADDR_DAISY_SIZE2                 ",   &gt_xc24_general_regs._r31 },
    { XC24_ADDR_DAISY_SIZE3                 ,   "XC24_ADDR_DAISY_SIZE3                 ",   &gt_xc24_general_regs._r32 },
    { XC24_ADDR_DAISY_SIZE4                 ,   "XC24_ADDR_DAISY_SIZE4                 ",   &gt_xc24_general_regs._r33 },
    { XC24_ADDR_DAISY_SIZE5                 ,   "XC24_ADDR_DAISY_SIZE5                 ",   &gt_xc24_general_regs._r34 },
    { XC24_ADDR_DAISY_SIZE6                 ,   "XC24_ADDR_DAISY_SIZE6                 ",   &gt_xc24_general_regs._r35 },
    { XC24_ADDR_DAISY_SIZE7                 ,   "XC24_ADDR_DAISY_SIZE7                 ",   &gt_xc24_general_regs._r36 },
    { XC24_ADDR_DAISY_SIZE8                 ,   "XC24_ADDR_DAISY_SIZE8                 ",   &gt_xc24_general_regs._r37 },
    { XC24_ADDR_BLOCK_SIZE1                 ,   "XC24_ADDR_BLOCK_SIZE1                 ",   &gt_xc24_general_regs._r38 },
    { XC24_ADDR_BLOCK_SIZE2                 ,   "XC24_ADDR_BLOCK_SIZE2                 ",   &gt_xc24_general_regs._r39 },
    { XC24_ADDR_BLOCK_SIZE3                 ,   "XC24_ADDR_BLOCK_SIZE3                 ",   &gt_xc24_general_regs._r3A },
    { XC24_ADDR_BLOCK_SIZE4                 ,   "XC24_ADDR_BLOCK_SIZE4                 ",   &gt_xc24_general_regs._r3B },
    { XC24_ADDR_BLOCK_SIZE5                 ,   "XC24_ADDR_BLOCK_SIZE5                 ",   &gt_xc24_general_regs._r3C },
    { XC24_ADDR_BLOCK_SIZE6                 ,   "XC24_ADDR_BLOCK_SIZE6                 ",   &gt_xc24_general_regs._r3D },
    { XC24_ADDR_BLOCK_SIZE7                 ,   "XC24_ADDR_BLOCK_SIZE7                 ",   &gt_xc24_general_regs._r3E },
    { XC24_ADDR_BLOCK_SIZE8                 ,   "XC24_ADDR_BLOCK_SIZE8                 ",   &gt_xc24_general_regs._r3F },
    { XC24_ADDR_BLOCK_SIZE9                 ,   "XC24_ADDR_BLOCK_SIZE9                 ",   &gt_xc24_general_regs._r40 },
    { XC24_ADDR_BLOCK_SIZE10                ,   "XC24_ADDR_BLOCK_SIZE10                ",   &gt_xc24_general_regs._r41 },
    { XC24_ADDR_BLOCK_SIZE11                ,   "XC24_ADDR_BLOCK_SIZE11                ",   &gt_xc24_general_regs._r42 },
    { XC24_ADDR_BLOCK_SIZE12                ,   "XC24_ADDR_BLOCK_SIZE12                ",   &gt_xc24_general_regs._r43 },
    { XC24_ADDR_DUMMY_44                    ,   "XC24_ADDR_DUMMY_44                    ",   &gt_xc24_general_regs._r44 },
    { XC24_ADDR_CHANNEL_ENABLE1             ,   "XC24_ADDR_CHANNEL_ENABLE1             ",   &gt_xc24_general_regs._r45 },
    { XC24_ADDR_CHANNEL_ENABLE2             ,   "XC24_ADDR_CHANNEL_ENABLE2             ",   &gt_xc24_general_regs._r46 },
    { XC24_ADDR_DUMMY_47                    ,   "XC24_ADDR_DUMMY_47                    ",   &gt_xc24_general_regs._r47 },
    { XC24_ADDR_DUMMY_48                    ,   "XC24_ADDR_DUMMY_48                    ",   &gt_xc24_general_regs._r48 },
    { XC24_ADDR_DUMMY_49                    ,   "XC24_ADDR_DUMMY_49                    ",   &gt_xc24_general_regs._r49 },
    { XC24_ADDR_DUMMY_4A                    ,   "XC24_ADDR_DUMMY_4A                    ",   &gt_xc24_general_regs._r4A },
    { XC24_ADDR_DUMMY_4B                    ,   "XC24_ADDR_DUMMY_4B                    ",   &gt_xc24_general_regs._r4B },
    { XC24_ADDR_DUMMY_4C                    ,   "XC24_ADDR_DUMMY_4C                    ",   &gt_xc24_general_regs._r4C },
    { XC24_ADDR_DAC_CONTROL                 ,   "XC24_ADDR_DAC_CONTROL                 ",   &gt_xc24_general_regs._r4D },
    { XC24_ADDR_DUMMY_4E                    ,   "XC24_ADDR_DUMMY_4E                    ",   &gt_xc24_general_regs._r4E },
    { XC24_ADDR_CURRENT_TARGET_DAC          ,   "XC24_ADDR_CURRENT_TARGET_DAC          ",   &gt_xc24_general_regs._r4F },
    { XC24_ADDR_PREVIOUS_TARGET_DAC         ,   "XC24_ADDR_PREVIOUS_TARGET_DAC         ",   &gt_xc24_general_regs._r50 },
    { XC24_ADDR_DAC_OUT                     ,   "XC24_ADDR_DAC_OUT                     ",   &gt_xc24_general_regs._r51 },
    { XC24_ADDR_DAC_INCREMENT1              ,   "XC24_ADDR_DAC_INCREMENT1              ",   &gt_xc24_general_regs._r52 },
    { XC24_ADDR_DAC_INCREMENT2_HOLD_LIMIT   ,   "XC24_ADDR_DAC_INCREMENT2_HOLD_LIMIT   ",   &gt_xc24_general_regs._r53 },
    { XC24_ADDR_DAC_DECREMENT_INC_WAIT      ,   "XC24_ADDR_DAC_DECREMENT_INC_WAIT      ",   &gt_xc24_general_regs._r54 },
    { XC24_ADDR_DAC_INCREMENT_HOLD_TH       ,   "XC24_ADDR_DAC_INCREMENT_HOLD_TH       ",   &gt_xc24_general_regs._r55 },
    { XC24_ADDR_SOA_N11_N1                  ,   "XC24_ADDR_SOA_N11_N1                  ",   &gt_xc24_general_regs._r56 },
    { XC24_ADDR_SOA_P2_P1                   ,   "XC24_ADDR_SOA_P2_P1                   ",   &gt_xc24_general_regs._r57 },
    { XC24_ADDR_SOA_P3_P2                   ,   "XC24_ADDR_SOA_P3_P2                   ",   &gt_xc24_general_regs._r58 },
    { XC24_ADDR_DAC_FB_VALID_TIMER          ,   "XC24_ADDR_DAC_FB_VALID_TIMER          ",   &gt_xc24_general_regs._r59 },
    { XC24_ADDR_DAC_MIN_LIMIT               ,   "XC24_ADDR_DAC_MIN_LIMIT               ",   &gt_xc24_general_regs._r5A },
    { XC24_ADDR_DAC_MAX_LIMIT               ,   "XC24_ADDR_DAC_MAX_LIMIT               ",   &gt_xc24_general_regs._r5B },
    { XC24_ADDR_DAC_STATE                   ,   "XC24_ADDR_DAC_STATE                   ",   &gt_xc24_general_regs._r5C },
    { XC24_ADDR_VALID_CNT                   ,   "XC24_ADDR_VALID_CNT                   ",   &gt_xc24_general_regs._r5D },
    { XC24_ADDR_DAC_INC_HOLD_WAIT_CNT       ,   "XC24_ADDR_DAC_INC_HOLD_WAIT_CNT       ",   &gt_xc24_general_regs._r5E },
    { XC24_ADDR_R2                          ,   "XC24_ADDR_R2                          ",   &gt_xc24_general_regs._r5F },
    { XC24_ADDR_GLOBAL_WRITE_DATA           ,   "XC24_ADDR_GLOBAL_WRITE_DATA           ",   &gt_xc24_general_regs._r60 },
    { XC24_ADDR_GLOBAL_FAULT_READ_DATA1     ,   "XC24_ADDR_GLOBAL_FAULT_READ_DATA1     ",   &gt_xc24_general_regs._r61 },
    { XC24_ADDR_GLOBAL_FAULT_READ_DATA2     ,   "XC24_ADDR_GLOBAL_FAULT_READ_DATA2     ",   &gt_xc24_general_regs._r62 },
    { XC24_ADDR_GLOBAL_FAULT_READ_DATA3     ,   "XC24_ADDR_GLOBAL_FAULT_READ_DATA3     ",   &gt_xc24_general_regs._r63 },
    { XC24_ADDR_GLOBAL_FAULT_READ_DATA4     ,   "XC24_ADDR_GLOBAL_FAULT_READ_DATA4     ",   &gt_xc24_general_regs._r64 },
    { XC24_ADDR_GLOBAL_FAULT_READ_DATA5     ,   "XC24_ADDR_GLOBAL_FAULT_READ_DATA5     ",   &gt_xc24_general_regs._r65 },
    { XC24_ADDR_GLOBAL_FAULT_READ_DATA6     ,   "XC24_ADDR_GLOBAL_FAULT_READ_DATA6     ",   &gt_xc24_general_regs._r66 },
    { XC24_ADDR_DUMMY_67                    ,   "XC24_ADDR_DUMMY_67                    ",   &gt_xc24_general_regs._r67 },
    { XC24_ADDR_DUMMY_68                    ,   "XC24_ADDR_DUMMY_68                    ",   &gt_xc24_general_regs._r68 },
    { XC24_ADDR_DUMMY_69                    ,   "XC24_ADDR_DUMMY_69                    ",   &gt_xc24_general_regs._r69 },
    { XC24_ADDR_DUMMY_6A                    ,   "XC24_ADDR_DUMMY_6A                    ",   &gt_xc24_general_regs._r6A },
    { XC24_ADDR_DUMMY_6B                    ,   "XC24_ADDR_DUMMY_6B                    ",   &gt_xc24_general_regs._r6B },
    { XC24_ADDR_DUMMY_6C                    ,   "XC24_ADDR_DUMMY_6C                    ",   &gt_xc24_general_regs._r6C },
    { XC24_ADDR_DUMMY_6D                    ,   "XC24_ADDR_DUMMY_6D                    ",   &gt_xc24_general_regs._r6D },
    { XC24_ADDR_DUMMY_6E                    ,   "XC24_ADDR_DUMMY_6E                    ",   &gt_xc24_general_regs._r6E },
    { XC24_ADDR_DUMMY_6F                    ,   "XC24_ADDR_DUMMY_6F                    ",   &gt_xc24_general_regs._r6F },
    { XC24_ADDR_PORT1_LOCAL_RW_DATA1        ,   "XC24_ADDR_PORT1_LOCAL_RW_DATA1        ",   &gt_xc24_general_regs._r70 },
};
static_assert(XC24_ADDR_MAX == (sizeof(gt_xc24_general_maps) / sizeof(_reg_map_t)), "XC24 General Address map mismatch!");

static _reg_map_t gt_xc24_mirror_maps[] =
{
    { XC24_ADDR_TRIM_TEST_CONTROL ,         "XC24_ADDR_TRIM_TEST_CONTROL ",                 &gt_xc24_mirror_regs._rF0 },
    { XC24_ADDR_TRIM_OTP_PG_ACCESS,         "XC24_ADDR_TRIM_OTP_PG_ACCESS",                 &gt_xc24_mirror_regs._rF1 },
    { XC24_ADDR_TRIM_OTP_WRITE    ,         "XC24_ADDR_TRIM_OTP_WRITE    ",                 &gt_xc24_mirror_regs._rF2 },
    { XC24_ADDR_TRIM_OTP_RD_PROG  ,         "XC24_ADDR_TRIM_OTP_RD_PROG  ",                 &gt_xc24_mirror_regs._rF3 },
    { XC24_ADDR_TRIM_OTP_PROTECT  ,         "XC24_ADDR_TRIM_OTP_PROTECT  ",                 &gt_xc24_mirror_regs._rF4 },
    { XC24_ADDR_TRIM_MIRROR1      ,         "XC24_ADDR_TRIM_MIRROR1      ",                 &gt_xc24_mirror_regs._rF5 },
    { XC24_ADDR_TRIM_MIRROR2      ,         "XC24_ADDR_TRIM_MIRROR2      ",                 &gt_xc24_mirror_regs._rF6 },
    { XC24_ADDR_TRIM_MIRROR3      ,         "XC24_ADDR_TRIM_MIRROR3      ",                 &gt_xc24_mirror_regs._rF7 },
};
static_assert((XC24_ADDR_TRIM_MAX - XC24_ADDR_TRIM_START) == (sizeof(gt_xc24_mirror_maps) / sizeof(_reg_map_t)), "XC24 Mirror Address map mismatch!");

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
            if (gn_xc_spi_timeout == 0)
            {
                print(LOG_ERROR, "SPI Timeout Error\r\n");
                break;
            }
        }
        LL_SPI_TransmitData16(SPIx, p_buffer[i]);
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx))
    {
        if (gn_xc_spi_timeout == 0)
        {
            print(LOG_ERROR, "SPI Timeout Error\r\n");
            break;
        }
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
            if (gn_xc_spi_timeout == 0)
            {
                print(LOG_ERROR, "SPI Timeout Error\r\n");
                break;
            }
        }
        LL_SPI_TransmitData16(SPIx, p_tx_buffer[i]);

        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx))
        {
            if (gn_xc_spi_timeout == 0)
            {
                print(LOG_ERROR, "SPI Timeout Error\r\n");
                break;
            }
        }
        p_rx_buffer[i] = LL_SPI_ReceiveData16(SPIx);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx))
    {
        if (gn_xc_spi_timeout == 0)
        {
            print(LOG_ERROR, "SPI Timeout Error\r\n");
            break;
        }
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
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, in_addr);
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
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, in_addr);
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
        const _reg_map_t* map = XC24_Get_General_Map_Pointer(xc_addr);
        if (map)
        {
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t VALUE : (0x%04X)\r\n\r\n",
                            map->name, map->address, *((uint16_t*)(map->reg_ptr)));
        }
    }

    for (uint8_t xc_addr = XC24_ADDR_TRIM_START ; xc_addr < XC24_ADDR_TRIM_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24_Get_Mirror_Map_Pointer(xc_addr);
        if (map)
        {
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t VALUE : (0x%04X)\r\n\r\n",
                            map->name, map->address, *((uint16_t*)(map->reg_ptr)));
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
            XC24_Write_Register(map->address, *((uint16_t*)(map->reg_ptr)));
            us_delay(10);
        }
    }
    print(LOG_DEBUG, " ...XC24 Initial Done...\r\n");
    XC24_Read_Register_All();
}

void XC24_Trim_Init(void)
{
    g_hSPIx = SPI1;

    JigBD_IF_XC_VCC_EN(PWR_ON);

    LL_mDelay(20);

    XC_NSCS_HI();
    XC24_Start_MCLK_Oscillation(TRUE);

    print(LOG_DEBUG, " ...XC24 Initial Start...\r\n");

    for (uint8_t xc_addr = 0 ; xc_addr < XC24_ADDR_MAX ; ++xc_addr)
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
            default :
                continue;
            }
            XC24_Write_Register(map->address, *((uint16_t*)(map->reg_ptr)));
            us_delay(10);
        }
    }

    for (uint8_t xc_addr = XC24_ADDR_TRIM_START ; xc_addr < XC24_ADDR_TRIM_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24_Find_Register_Map(xc_addr);
        if (map)
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
            XC24_Write_Register(map->address, *((uint16_t*)(map->reg_ptr)));
            us_delay(10);
        }
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