#include <stdio.h>
#include <string.h>

#include "crc.h"
#include "drv_spi.h"
#include "drv_xcr24.h"
#include "drv_xdr12.h"
#include "ads124s08.h"
#include "comm_debugging.h"

#define FLL_BIT_SHIFT_LSB       (0U)
#define FLL_BIT_SHIFT_MSB       (16U)

#define FLL_BIT_B15_B0          (0x00FFFFUL)
#define FLL_BIT_B20_B16         (0x1F0000UL)

#define XCR_FLL_PAD_VSYNC       (0U)
#define XCR_FLL_PAD_FLLSYNC     (1U)

#define XCR_MCLK_SEL_OSC_A      (1U)
#define XCR_MCLK_SEL_OSC_B      (0U)

#define XCR_V_MASK_ON_TIME_US   (4.5f)      /* 4.5us */
#define XCR_V_MASK_OFF_TIME_US  (4.5f)      /* 4.5us */

#define XCR_SVO_ON_TIME_US      (10.0f)     /* 10us */
#define XCR_SVO1_OFF_TIME_US    (100.0f)    /* 100us */
#define XCR_SVO2_OFF_TIME_US    (144.43f)   /* 144.43us */
#define XCR_SVO3_OFF_TIME_US    (96.0f)     /* 96us */

#define XCR_SPI_HEADER_SIZE     (1U)
#define XCR_SPI_BURST_MAX_SIZE  (64U)
#define XCR_SPI_BUFF_MAX_SIZE   (XCR_SPI_HEADER_SIZE + XCR_SPI_BURST_MAX_SIZE)

#define XCR_TRIM_DAC_INPUT      (0xC8U)

#define XCR_TEST_DAC_INPUT_P1   (0x0C8U)
#define XCR_TEST_DAC_INPUT_P2   (0x3E8U)
#define XCR_TEST_DAC_INPUT_P3   (0xBB8U)

#define XCR_MAX_1V5_LDO_DIG     (0x001FU)
#define XCR_MAX_DAC_3V0         (0x003FU)
#define XCR_MAX_DAC1_OFS        (0x00FFU)
#define XCR_MAX_DAC2_OFS        (0x00FFU)
#define XCR_MAX_DAC3_OFS        (0x00FFU)
#define XCR_MAX_1V5_LDO_OSC     (0x001FU)
#define XCR_MAX_OSC_A           (0x001FU)
#define XCR_MAX_OSC_B           (0x001FU)

#define XCR_FUNCTION_DIS        (0U)
#define XCR_FUNCTION_EN         (1U)

#if (XDR_LD_DATA_BIT == XDR_LD_DATA_12BIT)
    #define XCR_LD_WIDTH        (LD_WIDTH1)
#elif (XDR_LD_DATA_BIT == XDR_LD_DATA_14BIT)
    #define XCR_LD_WIDTH        (LD_WIDTH2)
#else
    #error "Unsupported XDR_LD_DATA_BIT value"
#endif

#define XCR_SYNCMODE_NONE       (0U)
#define XCR_SYNCMODE_CMD_SVO    (1U)
#define XCR_SYNCMODE_VO_SVO     (2U)
#define XCR_SYNCMODE_RSO_CMD    (3U)

#define XCR_SVO_ACTIVE_NONE     (0U)
#define XCR_SVO_ACTIVE_NDF      (1U)
#define XCR_SVO_ACTIVE_23       (2U)
#define XCR_SVO_ACTIVE_123      (3U)

volatile bool gb_xcr_ld_transfer_spi_dma_flag;

static uint8_t gn_xcr_daisied_dev_blk_size;
static uint8_t gn_xcr_channel_enable[XCR_CH_SIZE_MAX];
static uint8_t gn_xcr_channel_daisy_size[XCR_CH_SIZE_MAX];
static uint8_t gn_xcr_channel_block_size[XCR_CH_SIZE_MAX];
static uint32_t gn_xcr_fll_cnt[2];

static _xcr_group1_regs_t gt_xcr24_set_gr1_regs;
static _xcr_group1_regs_t gt_xcr24_get_gr1_regs;

static _xcr_group2_regs_t gt_xcr24_set_gr2_regs;
static _xcr_group2_regs_t gt_xcr24_get_gr2_regs;

static _xcr_otp_control_regs_t gt_xcr24_set_otp_regs; /* base address 0xF0 */
static _xcr_otp_control_regs_t gt_xcr24_get_otp_regs; /* base address 0xF0 */

static bool gb_xcr_do_efuse;

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
static void xcr24_spi_log_dump(const uint16_t* out, uint16_t len)
{
    if (out == NULL || len == 0)
    {
        return;
    }

    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
    LL_USART_TransmitData8(USART2, '\r');

    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
    LL_USART_TransmitData8(USART2, '\n');

    for (uint16_t i = 0; i < len; ++i)
    {
        uint16_t val = out[i];

        // "0x" 전송
        while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
        LL_USART_TransmitData8(USART2, '0');
        while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
        LL_USART_TransmitData8(USART2, 'x');

        // 16진수 4자리 변환 및 전송 (상위 니블부터 하위 니블까지)
        for (int8_t shift = 12; shift >= 0; shift -= 4)
        {
            uint8_t nibble = (val >> shift) & 0x0F;
            char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));

            while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
            LL_USART_TransmitData8(USART2, hex_char);
        }

        // 구분자 ',' 전송
        while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
        LL_USART_TransmitData8(USART2, ',');
    }
}
#endif

static void xcr24_regs_init_table(void)
{
    _xcr_group1_regs_t* _r1 = &gt_xcr24_set_gr1_regs;
    for(xcr_addr_grp1_t addr = XCR_RESET; addr < XCR_GRP1_MAX; ++addr)
    {
        switch(addr)
        {
        case XCR_FAULT_READ_COMMAND:
            _r1->reg._r05.bit.ft_mode = 1U;
            break;
        case XCR_LD_TRANSFER_COMMAND:
            _r1->reg._r06.bit.ld_type = LED_PER_BLOCK;
            break;
        case XCR_SYNC_GEN_COMMAND:
            _r1->reg._r07.bit.syncmode = XCR_SYNCMODE_VO_SVO;
            _r1->reg._r07.bit.enable = XCR_FUNCTION_DIS;
            break;
        case XCR_COMMAND_AUTO_ENABLE:
            _r1->reg._r08.bit.sync_auto_en = XCR_FUNCTION_DIS;
            _r1->reg._r08.bit.fault_auto_en = XCR_FUNCTION_EN;
            _r1->reg._r08.bit.timeout_en = XCR_FUNCTION_EN;
            break;
        case XCR_LD_START_POINTER_TH:
            _r1->reg._r0C.bit.ld_transfer_start_pointer = 9U;
            break;
        case XCR_FAULT_AUTO_READ_INTERVAL:
            _r1->reg._r11.bit.fault_auto_rd_interval = 0xFFFFU;
            break;
        case XCR_FAULT_AUTO_READ_EVENT:
            _r1->reg._r12.bit.fault_auto_rd_interval = 1U;
            _r1->reg._r12.bit.fault_auto_rd_timer_event = 1U;
            break;
        case XCR_INTERRUPT_ENABLE:
            _r1->reg._r13.bit.int_fb_en = XCR_FUNCTION_EN;
            break;
        case XCR_CLK_CONTROL_1:
            // _r1->reg._r1B.bit.serializer_skew_en = XCR_FUNCTION_DIS;
            // _r1->reg._r1B.bit.osc1_spread_en = XCR_FUNCTION_EN;
            // _r1->reg._r1B.bit.serializer_clk_sel1 = XCR_FUNCTION_DIS;
            // _r1->reg._r1B.bit.sprd1_gain = XCR_FUNCTION_DIS;
            // _r1->reg._r1B.bit.serializer_clk_sel2 = XCR_FUNCTION_DIS;
            // _r1->reg._r1B.bit.ld_rd_clk_sel = XCR_FUNCTION_DIS;
            // _r1->reg._r1B.bit.spread1_spd = XCR_FUNCTION_DIS;
            _r1->reg._r1B.ALL = 0x0808U;
            break;
        case XCR_CLK_CONTROL_2:
            //_r1->reg._r1C.bit.mclk_mode = XCR_FUNCTION_DIS;
            //_r1->reg._r1C.bit.osc2_spread_en = XCR_FUNCTION_DIS;
            //_r1->reg._r1C.bit.sprd2_gain = XCR_FUNCTION_DIS;
            //_r1->reg._r1C.bit.spread2_spd = XCR_FUNCTION_DIS;
            _r1->reg._r1C.ALL = 0x0000U;
            break;
        case XCR_SERIALIZER_CLOCK_GEN:
            _r1->reg._r1D.bit.serial_clk_high = XCR_SERIAL_CLK_HIGH;
            _r1->reg._r1D.bit.serial_clk_low = XCR_SERIAL_CLK_LOW;
            break;
        case XCR_LATENCY:
            _r1->reg._r1E.bit.cmd_latency = 0xC0U;
            _r1->reg._r1E.bit.serial_latency = 0x70U;
            break;
        case XCR_TIMEOUT:
            _r1->reg._r1F.bit.timeout = 0x06D6U;
            break;
        case XCR_DAISIED_DEVICE_CH_SIZE:
            _r1->reg._r20.bit.daisied_dev_blk_size = gn_xcr_daisied_dev_blk_size;
            break;
        case XCR_DAISY_SIZE_1:
            _r1->reg._r21.bit.daisy_size_ch1 = gn_xcr_channel_daisy_size[0U];
            _r1->reg._r21.bit.daisy_size_ch2 = gn_xcr_channel_daisy_size[1U];
            _r1->reg._r21.bit.daisy_size_ch3 = gn_xcr_channel_daisy_size[2U];
            break;
        case XCR_DAISY_SIZE_2:
            _r1->reg._r22.bit.daisy_size_ch4 = gn_xcr_channel_daisy_size[3U];
            _r1->reg._r22.bit.daisy_size_ch5 = gn_xcr_channel_daisy_size[4U];
            _r1->reg._r22.bit.daisy_size_ch6 = gn_xcr_channel_daisy_size[5U];
            break;
        case XCR_DAISY_SIZE_3:
            _r1->reg._r23.bit.daisy_size_ch7 = gn_xcr_channel_daisy_size[6U];
            _r1->reg._r23.bit.daisy_size_ch8 = gn_xcr_channel_daisy_size[7U];
            _r1->reg._r23.bit.daisy_size_ch9 = gn_xcr_channel_daisy_size[8U];
            break;
        case XCR_DAISY_SIZE_4:
            _r1->reg._r24.bit.daisy_size_ch10 = gn_xcr_channel_daisy_size[9U];
            _r1->reg._r24.bit.daisy_size_ch11 = gn_xcr_channel_daisy_size[10U];
            _r1->reg._r24.bit.daisy_size_ch12 = gn_xcr_channel_daisy_size[11U];
            break;
        case XCR_DAISY_SIZE_5:
            _r1->reg._r25.bit.daisy_size_ch13 = gn_xcr_channel_daisy_size[12U];
            _r1->reg._r25.bit.daisy_size_ch14 = gn_xcr_channel_daisy_size[13U];
            _r1->reg._r25.bit.daisy_size_ch15 = gn_xcr_channel_daisy_size[14U];
            break;
        case XCR_DAISY_SIZE_6:
            _r1->reg._r26.bit.daisy_size_ch16 = gn_xcr_channel_daisy_size[15U];
            _r1->reg._r26.bit.daisy_size_ch17 = gn_xcr_channel_daisy_size[16U];
            _r1->reg._r26.bit.daisy_size_ch18 = gn_xcr_channel_daisy_size[17U];
            break;
        case XCR_DAISY_SIZE_7:
            _r1->reg._r27.bit.daisy_size_ch19 = gn_xcr_channel_daisy_size[18U];
            _r1->reg._r27.bit.daisy_size_ch20 = gn_xcr_channel_daisy_size[19U];
            _r1->reg._r27.bit.daisy_size_ch21 = gn_xcr_channel_daisy_size[20U];
            break;
        case XCR_DAISY_SIZE_8:
            _r1->reg._r28.bit.daisy_size_ch22 = gn_xcr_channel_daisy_size[21U];
            _r1->reg._r28.bit.daisy_size_ch23 = gn_xcr_channel_daisy_size[22U];
            _r1->reg._r28.bit.daisy_size_ch24 = gn_xcr_channel_daisy_size[23U];
            break;
        case XCR_BLOCK_SIZE_1:
            _r1->reg._r29.bit.total_blk_size_ch1 = gn_xcr_channel_block_size[0U];
            _r1->reg._r29.bit.total_blk_size_ch2 = gn_xcr_channel_block_size[1U];
            break;
        case XCR_BLOCK_SIZE_2:
            _r1->reg._r2A.bit.total_blk_size_ch3 = gn_xcr_channel_block_size[2U];
            _r1->reg._r2A.bit.total_blk_size_ch4 = gn_xcr_channel_block_size[3U];
            break;
        case XCR_BLOCK_SIZE_3:
            _r1->reg._r2B.bit.total_blk_size_ch5 = gn_xcr_channel_block_size[4U];
            _r1->reg._r2B.bit.total_blk_size_ch6 = gn_xcr_channel_block_size[5U];
            break;
        case XCR_BLOCK_SIZE_4:
            _r1->reg._r2C.bit.total_blk_size_ch7 = gn_xcr_channel_block_size[6U];
            _r1->reg._r2C.bit.total_blk_size_ch8 = gn_xcr_channel_block_size[7U];
            break;
        case XCR_BLOCK_SIZE_5:
            _r1->reg._r2D.bit.total_blk_size_ch9 = gn_xcr_channel_block_size[8U];
            _r1->reg._r2D.bit.total_blk_size_ch10 = gn_xcr_channel_block_size[9U];
            break;
        case XCR_BLOCK_SIZE_6:
            _r1->reg._r2E.bit.total_blk_size_ch11 = gn_xcr_channel_block_size[10U];
            _r1->reg._r2E.bit.total_blk_size_ch12 = gn_xcr_channel_block_size[11U];
            break;
        case XCR_BLOCK_SIZE_7:
            _r1->reg._r2F.bit.total_blk_size_ch13 = gn_xcr_channel_block_size[12U];
            _r1->reg._r2F.bit.total_blk_size_ch14 = gn_xcr_channel_block_size[13U];
            break;
        case XCR_BLOCK_SIZE_8:
            _r1->reg._r30.bit.total_blk_size_ch15 = gn_xcr_channel_block_size[14U];
            _r1->reg._r30.bit.total_blk_size_ch16 = gn_xcr_channel_block_size[15U];
            break;
        case XCR_BLOCK_SIZE_9:
            _r1->reg._r31.bit.total_blk_size_ch17 = gn_xcr_channel_block_size[16U];
            _r1->reg._r31.bit.total_blk_size_ch18 = gn_xcr_channel_block_size[17U];
            break;
        case XCR_BLOCK_SIZE_10:
            _r1->reg._r32.bit.total_blk_size_ch19 = gn_xcr_channel_block_size[18U];
            _r1->reg._r32.bit.total_blk_size_ch20 = gn_xcr_channel_block_size[19U];
            break;
        case XCR_BLOCK_SIZE_11:
            _r1->reg._r33.bit.total_blk_size_ch21 = gn_xcr_channel_block_size[20U];
            _r1->reg._r33.bit.total_blk_size_ch22 = gn_xcr_channel_block_size[21U];
            break;
        case XCR_BLOCK_SIZE_12:
            _r1->reg._r34.bit.total_blk_size_ch23 = gn_xcr_channel_block_size[22U];
            _r1->reg._r34.bit.total_blk_size_ch24 = gn_xcr_channel_block_size[23U];
            break;
        case XCR_CHANNEL_ENABLE_1:
            _r1->reg._r35.bit.ch1_en = gn_xcr_channel_enable[0U];
            _r1->reg._r35.bit.ch2_en = gn_xcr_channel_enable[1U];
            _r1->reg._r35.bit.ch3_en = gn_xcr_channel_enable[2U];
            _r1->reg._r35.bit.ch4_en = gn_xcr_channel_enable[3U];
            _r1->reg._r35.bit.ch5_en = gn_xcr_channel_enable[4U];
            _r1->reg._r35.bit.ch6_en = gn_xcr_channel_enable[5U];
            _r1->reg._r35.bit.ch7_en = gn_xcr_channel_enable[6U];
            _r1->reg._r35.bit.ch8_en = gn_xcr_channel_enable[7U];
            _r1->reg._r35.bit.ch9_en = gn_xcr_channel_enable[8U];
            _r1->reg._r35.bit.ch10_en = gn_xcr_channel_enable[9U];
            _r1->reg._r35.bit.ch11_en = gn_xcr_channel_enable[10U];
            _r1->reg._r35.bit.ch12_en = gn_xcr_channel_enable[11U];
            _r1->reg._r35.bit.ch13_en = gn_xcr_channel_enable[12U];
            _r1->reg._r35.bit.ch14_en = gn_xcr_channel_enable[13U];
            _r1->reg._r35.bit.ch15_en = gn_xcr_channel_enable[14U];
            _r1->reg._r35.bit.ch16_en = gn_xcr_channel_enable[15U];
            break;
        case XCR_CHANNEL_ENABLE_2:
            _r1->reg._r36.bit.ch17_en = gn_xcr_channel_enable[16U];
            _r1->reg._r36.bit.ch18_en = gn_xcr_channel_enable[17U];
            _r1->reg._r36.bit.ch19_en = gn_xcr_channel_enable[18U];
            _r1->reg._r36.bit.ch20_en = gn_xcr_channel_enable[19U];
            _r1->reg._r36.bit.ch21_en = gn_xcr_channel_enable[20U];
            _r1->reg._r36.bit.ch22_en = gn_xcr_channel_enable[21U];
            _r1->reg._r36.bit.ch23_en = gn_xcr_channel_enable[22U];
            _r1->reg._r36.bit.ch24_en = gn_xcr_channel_enable[23U];
            _r1->reg._r36.bit.ch_size = XCR_CH_SIZE;
            _r1->reg._r36.bit.ld_width = XCR_LD_WIDTH;
            break;
        case XCR_FLLCNT11:
            _r1->reg._r37.bit.fll1cnt = ((gn_xcr_fll_cnt[0] & FLL_BIT_B15_B0) >> FLL_BIT_SHIFT_LSB);
            break;
        case XCR_FLLCNT12:
            _r1->reg._r38.bit.fll1cnt = ((gn_xcr_fll_cnt[0] & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            _r1->reg._r38.bit.fll1_err_range = 0U;
            _r1->reg._r38.bit.fll1_range = 0U;
            _r1->reg._r38.bit.fllsync = XCR_FLL_PAD_FLLSYNC;
            _r1->reg._r38.bit.fll1_en = XCR_FUNCTION_DIS;
            break;
        case XCR_FLLCNT21:
            _r1->reg._r39.bit.fll2cnt = ((gn_xcr_fll_cnt[1] & FLL_BIT_B15_B0) >> FLL_BIT_SHIFT_LSB);
            break;
        case XCR_FLLCNT22:
            _r1->reg._r3A.bit.fll2cnt = ((gn_xcr_fll_cnt[1] & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            _r1->reg._r3A.bit.fll2_err_range = 0U;
            _r1->reg._r3A.bit.fll2_range = 0U;
            _r1->reg._r3A.bit.fllsync = XCR_FLL_PAD_FLLSYNC;
            _r1->reg._r3A.bit.fll2_en = XCR_FUNCTION_DIS;
            break;
        case XCR_VO_DELAY:
            _r1->reg._r3B.bit.gate3_pol = 1U;
            _r1->reg._r3B.bit.gate2_pol = 1U;
            _r1->reg._r3B.bit.gate1_pol = 1U;
            _r1->reg._r3B.bit.vo_delay = 0U;
            break;
        case XCR_VO_OFF_ON:
            _r1->reg._r3C.bit.vo_on = XCR_CONV_US_TO_XCR_MCLK(XCR_V_MASK_ON_TIME_US);
            _r1->reg._r3C.bit.vo_off = XCR_CONV_US_TO_XCR_MCLK(XCR_V_MASK_OFF_TIME_US);
            break;
        case XCR_SVO_ON:
            _r1->reg._r3D.bit.svo_on = XCR_CONV_US_TO_XCR_MCLK(XCR_SVO_ON_TIME_US);
            break;
        case XCR_SVO1_OFF:
            //_r1->reg._r3E.bit.svo1_off = XCR_CONV_US_TO_XCR_MCLK(XCR_SVO1_OFF_TIME_US);
            _r1->reg._r3E.bit.svo1_off = 0U;
            break;
        case XCR_SVO2_OFF:
            _r1->reg._r3F.bit.svo2_off = XCR_CONV_US_TO_XCR_MCLK(XCR_SVO2_OFF_TIME_US);
            break;
        case XCR_SVO3_OFF:
            _r1->reg._r40.bit.svo3_off = XCR_CONV_US_TO_XCR_MCLK(XCR_SVO3_OFF_TIME_US);
            break;
        case XCR_SVO_NUMBER:
            _r1->reg._r41.bit.sv_no = XDR_SV_NO;
            _r1->reg._r41.bit.sv_no_type = XCR_SVO_ACTIVE_23;
            break;
        case XCR_DAC_NF_CONTROL:
            //_r1->reg._r42.bit.dgrjt_en = 0U;
            //_r1->reg._r42.bit.bbkn_en = 0U;
            //_r1->reg._r42.bit.bbkn_th = 0U;
            //_r1->reg._r42.bit.dac_lvl = 0U;
            _r1->reg._r42.ALL = 0x0000U;
            break;
        case XCR_DAC_CONTROL:
            _r1->reg._r43.bit.dac1_auto = 1U;
            _r1->reg._r43.bit.dac2_auto = 1U;
            _r1->reg._r43.bit.dac3_auto = 1U;
            _r1->reg._r43.bit.dac_auto_type = 0U;
            _r1->reg._r43.bit.dac_sync_mode = 1U;
            _r1->reg._r43.bit.dac1_fb_mode = 0U;
            _r1->reg._r43.bit.dac2_fb_mode = 0U;
            _r1->reg._r43.bit.dac3_fb_mode = 0U;
            _r1->reg._r43.bit.dac1_dec1_mode = 1U;
            _r1->reg._r43.bit.dac2_dec1_mode = 1U;
            _r1->reg._r43.bit.dac3_dec1_mode = 1U;
            _r1->reg._r43.bit.dac1_hold_en = 0U;
            _r1->reg._r43.bit.dac2_hold_en = 0U;
            _r1->reg._r43.bit.dac3_hold_en = 0U;
            _r1->reg._r43.bit.dac_fault_off = 0U;
            break;
        case XCR_DAC1_MIN_LIMIT:
            _r1->reg._r55.bit.dac1_min_limit = XCR_CONV_DAC_V_TO_INPUT(0.3f);
            break;
        case XCR_DAC1_MAX_LIMIT:
            _r1->reg._r56.bit.dac1_max_limit = XCR_CONV_DAC_V_TO_INPUT(2.9f);
            break;
        case XCR_DAC2_MIN_LIMIT:
            _r1->reg._r5C.bit.dac2_min_limit = XCR_CONV_DAC_V_TO_INPUT(0.5f);
            break;
        case XCR_DAC2_MAX_LIMIT:
            _r1->reg._r5D.bit.dac2_max_limit = XCR_CONV_DAC_V_TO_INPUT(2.7f);
            break;
        case XCR_DAC3_MIN_LIMIT:
            _r1->reg._r63.bit.dac3_min_limit = XCR_CONV_DAC_V_TO_INPUT(0.7f);
            break;
        case XCR_DAC3_MAX_LIMIT:
            _r1->reg._r64.bit.dac3_max_limit = XCR_CONV_DAC_V_TO_INPUT(2.5f);
            break;
        case XCR_OSC_FLL_MAN_A1:
            _r1->reg._r65.bit.OSC_MAN_EN_A = XCR_FUNCTION_EN;
            break;
        case XCR_OSC_FLL_MAN_B1:
            _r1->reg._r67.bit.OSC_MAN_EN_B = XCR_FUNCTION_EN;
            break;
        default:
            continue;
        }
        xcr24_write_grp1_reg(addr, &_r1->ALL[addr], 1U);
    }
#if 0
    _xcr_group2_regs_t* _r2 = &gt_xcr24_set_gr2_regs;
    for(xcr_addr_grp2_t addr = XCR_GRP2_DAC1_FB_VALID_CNT; addr < XCR_GRP2_MAX; ++addr)
    {
        switch(addr)
        {
        default:
            continue;
        }
        xcr24_write_grp2_reg(addr, &_r2->ALL[addr], 1U);
    }
#endif
    _xcr_otp_control_regs_t* _rotp = &gt_xcr24_set_otp_regs;
    for (xcr_addr_otp_t addr = XCR_TEST_CONTROL; addr < XCR_OTP_MAX; ++addr)
    {
        switch (addr)
        {
            case XCR_GATE_CONTROL:
                _rotp->reg._rFA.bit.gate1_H_en = XCR_FUNCTION_EN;
                _rotp->reg._rFA.bit.gate1_L_en = XCR_FUNCTION_DIS;
                _rotp->reg._rFA.bit.gate2_H_en = XCR_FUNCTION_EN;
                _rotp->reg._rFA.bit.gate2_L_en = XCR_FUNCTION_DIS;
                _rotp->reg._rFA.bit.gate3_H_en = XCR_FUNCTION_EN;
                _rotp->reg._rFA.bit.gate3_L_en = XCR_FUNCTION_DIS;
                break;
            case XCR_GATE1_OFFSET:
                _rotp->reg._rFB.ALL = 0x0202U;
                break;
            case XCR_GATE2_OFFSET:
                _rotp->reg._rFC.ALL = 0x0202U;
                break;
            case XCR_GATE3_OFFSET:
                _rotp->reg._rFD.ALL = 0x0202U;
                break;
            case XCR_SV_VAR_CONTROL1:
                _rotp->reg._rFE.ALL = 0x0000U;
                break;
            case XCR_SV_VAR_CONTROL2:
                _rotp->reg._rFF.ALL = 0x0000U;
                break;
            default:
                continue;
        }
        xcr24_write_otp_control(addr, &_rotp->ALL[addr], 1U);
    }
}

static void xcr24_regs_trim_init_table(void)
{
    _xcr_group1_regs_t* _r1 = &gt_xcr24_set_gr1_regs;
    for(xcr_addr_grp1_t addr = XCR_RESET; addr < XCR_GRP1_MAX; ++addr)
    {
        switch(addr)
        {
            case XCR_RESET:
            {
                _r1->reg._r00.bit.rst1 = 0U;
                _r1->reg._r00.bit.rst2 = 0U;
                _r1->reg._r00.bit.rst3 = 0U;
                _r1->reg._r00.bit.vsync_rst_en1 = 1U;
                _r1->reg._r00.bit.vsync_rst_en2 = 1U;
                break;
            }
            case XCR_LOCAL_RW_POINTER_RESET:
            {
                _r1->reg._r10.bit.local_transfer_pointer_rst = 1U;
                _r1->reg._r10.bit.local_receive_pointer_rst = 1U;
                break;
            }
            case XCR_CLK_CONTROL_1:
            {
                _r1->reg._r1B.bit.serializer_skew_en = 0U;
                _r1->reg._r1B.bit.osc1_spread_en = 0U;
                _r1->reg._r1B.bit.serializer_clk_sel1 = 0U;
                _r1->reg._r1B.bit.sprd1_gain = 0U;
                _r1->reg._r1B.bit.serializer_clk_sel2 = 0U;
                _r1->reg._r1B.bit.ld_rd_clk_sel = 0U;
                _r1->reg._r1B.bit.spread1_spd = 0U;
                break;
            }
            case XCR_CLK_CONTROL_2:
            {
                _r1->reg._r1C.bit.mclk_mode = 0U;
                _r1->reg._r1C.bit.osc2_spread_en = 0U;
                _r1->reg._r1C.bit.sprd2_gain = 0U;
                _r1->reg._r1C.bit.spread2_spd = 0U;
                break;
            }
            case XCR_SERIALIZER_CLOCK_GEN:
            {
                _r1->reg._r1D.bit.serial_clk_high = XCR_SERIAL_CLK_HIGH;
                _r1->reg._r1D.bit.serial_clk_low = XCR_SERIAL_CLK_LOW;
                break;
            }
            default:
            {
                continue;
            }
        }
        xcr24_write_grp1_reg(addr, &_r1->ALL[addr], 1U);
    }

    _xcr_group2_regs_t* _r2 = &gt_xcr24_set_gr2_regs;
    for(xcr_addr_grp2_t addr = XCR_GRP2_DAC1_FB_VALID_CNT; addr < XCR_GRP2_MAX; ++addr)
    {
        switch(addr)
        {
            case XCR_GRP2_DAC1_FB_VALID_CNT:
            {
                _r2->reg._r12.bit.TEST_ANA_EN = 0U;
                _r2->reg._r12.bit.CHOP_EN_BGR = 0U;
                _r2->reg._r12.bit.CHOP_EN_OSCLDO = 0U;
                _r2->reg._r12.bit.CHOP_EN = 0U;
                break;
            }
            default:
            {
                continue;
            }
        }
        xcr24_write_grp2_reg(addr, &_r2->ALL[addr], 1U);
    }

    _xcr_otp_control_regs_t* _rotp = &gt_xcr24_set_otp_regs;
    for (xcr_addr_otp_t addr = XCR_TEST_CONTROL; addr < XCR_OTP_MAX; ++addr)
    {
        switch (addr)
        {
            case XCR_TEST_CONTROL:
                _rotp->reg._rF0.bit.TEST_EN = 1U;
                break;
            case XCR_OTP_PROTECT:
                _rotp->reg._rF4.bit.PROTECT = 0x0A5AU;
                break;
            default:
                continue;
        }
        xcr24_write_otp_control(addr, &_rotp->ALL[addr], 1U);
    }
}

static void xcr24_regs_test_init_table(void)
{
    _xcr_group1_regs_t* _r1 = &gt_xcr24_set_gr1_regs;

    for(xcr_addr_grp1_t addr = XCR_RESET; addr < XCR_GRP1_MAX; ++addr)
    {
        switch(addr)
        {
            case XCR_SERIALIZER_CLOCK_GEN:
            {
                _r1->reg._r1D.ALL = 0x1010U;
                break;
            }
            case XCR_CHANNEL_ENABLE_1:
            {
                _r1->reg._r35.ALL = 0x0001U;
                break;
            }
            case XCR_DAISIED_DEVICE_CH_SIZE:
            {
                _r1->reg._r20.ALL = 0x001FU;
                break;
            }
            case XCR_BLOCK_SIZE_9:
            {
                _r1->reg._r31.ALL = 0x00FFU;
                break;
            }
            default:
            {
                continue;
            }
        }
        xcr24_write_grp1_reg(addr, &_r1->ALL[addr], 1U);
    }
}

void xcr24_reset(void)
{
    _v_reset_t _r00 = {0, };

    _r00.bit.rst1 = 1U;
    _r00.bit.rst2 = 0U;
    _r00.bit.rst3 = 0U;
    _r00.bit.vsync_rst_en1 = 0U;
    _r00.bit.vsync_rst_en2 = 0U;

    xcr24_write_grp1_reg(XCR_RESET, &_r00.ALL, 1U);
}

static void xcr24_dump_registers(void)
{
#if 0
    comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 GROUP1 Registers");
    for (xcr_addr_grp1_t addr = XCR_RESET; addr < XCR_GRP1_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%04X", addr, gt_xcr24_get_gr1_regs.ALL[addr]);
    }

    comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 GROUP2 Registers");
    for (xcr_addr_grp2_t addr = XCR_GRP2_DAC1_FB_VALID_CNT; addr < XCR_GRP2_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%04X", addr, gt_xcr24_get_gr2_regs.ALL[addr]);
    }

    comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 OTP Control Registers");
    for (xcr_addr_otp_t addr = XCR_TEST_CONTROL; addr < XCR_OTP_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%04X", (XCR_OTP_BASE_ADDR + addr), gt_xcr24_get_otp_regs.ALL[addr]);
    }
#else
    char line_buf[128];
    int len = 0;

    // 1. XCR24 GROUP1 Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n=== XCR24 GROUP1 Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    for (uint16_t addr = 0; addr < (uint16_t)XCR_GRP1_MAX; ++addr)
    {
        if ((addr % 16) == 0)
        {
            if (addr > 0)
            {
                comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
            }
            len = snprintf(line_buf, sizeof(line_buf), "\r\n0x%02X |", addr);
        }
        len += snprintf(&line_buf[len], sizeof(line_buf) - len, " %04X ", gt_xcr24_get_gr1_regs.ALL[addr]);
    }
    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    // 2. XCR24 GROUP2 Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n\r\n=== XCR24 GROUP2 Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    len = 0;
    for (uint16_t addr = 0; addr < (uint16_t)XCR_GRP2_MAX; ++addr)
    {
        if ((addr % 16) == 0)
        {
            if (addr > 0)
            {
                comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
            }
            len = snprintf(line_buf, sizeof(line_buf), "\r\n0x%02X |", addr);
        }
        len += snprintf(&line_buf[len], sizeof(line_buf) - len, " %04X ", gt_xcr24_get_gr2_regs.ALL[addr]);
    }
    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    // 3. XCR24 OTP Control Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n\r\n=== XCR24 OTP Control Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    len = 0;
    for (uint16_t offset = 0; offset < (uint16_t)XCR_OTP_MAX; ++offset)
    {
        uint16_t real_addr = (uint16_t)XCR_OTP_BASE_ADDR + offset;
        if ((offset % 16) == 0)
        {
            if (offset > 0)
            {
                comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
            }
            len = snprintf(line_buf, sizeof(line_buf), "\r\n0x%02X |", real_addr & 0xF0U);
        }

        if (offset == 0 && (real_addr % 16) != 0)
        {
            for (uint8_t pad = 0; pad < (real_addr % 16); ++pad)
            {
                len += snprintf(&line_buf[len], sizeof(line_buf) - len, " ---- ");
            }
        }

        len += snprintf(&line_buf[len], sizeof(line_buf) - len, " %04X ", gt_xcr24_get_otp_regs.ALL[offset]);
    }
    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");
    comm_UART_Printf(LOG_LV_INFO, "\r\n");
#endif
}

static void xcr24_memory_copy(void)
{
    memcpy(gt_xcr24_set_gr1_regs.ALL, gt_xcr24_get_gr1_regs.ALL, sizeof(gt_xcr24_get_gr1_regs));
    memcpy(gt_xcr24_set_gr2_regs.ALL, gt_xcr24_get_gr2_regs.ALL, sizeof(gt_xcr24_get_gr2_regs));
    memcpy(gt_xcr24_set_otp_regs.ALL, gt_xcr24_get_otp_regs.ALL, sizeof(gt_xcr24_get_otp_regs));
}

void xcr24_read_all(void)
{
    xcr24_read_grp1_reg(XCR_RESET +  0U, 56U); // 0x00 ~ 0x37, 56EA
    xcr24_read_grp1_reg(XCR_RESET + 56U, 56U); // 0x38 ~ 0x6F, 56EA

    xcr24_read_grp2_reg(XCR_GRP2_DAC1_FB_VALID_CNT, 19U); // 0x00 ~ 0x12, 19EA

    xcr24_read_otp_control(XCR_TEST_CONTROL, 16U);

    xcr24_dump_registers();
    xcr24_memory_copy();
}

void xcr24_init_param(void)
{
    gn_xcr_daisied_dev_blk_size = BLOCK_PER_XDR;

    /* XC24R channel enable */
    for (uint8_t xc_ch = 0; xc_ch < XCR_CH_SIZE_MAX; ++xc_ch)
    {
#if 1 //Sequential Enable
        if (xc_ch < XCR_CH_SIZE)
        {
            gn_xcr_channel_enable[xc_ch] = 1U;
            gn_xcr_channel_daisy_size[xc_ch] = XDR_DAISY_LENGTH;
            gn_xcr_channel_block_size[xc_ch] = (gn_xcr_channel_daisy_size[xc_ch] * gn_xcr_daisied_dev_blk_size);
        }
        else
        {
            gn_xcr_channel_enable[xc_ch] = 0U;
            gn_xcr_channel_daisy_size[xc_ch] = 0U;
            gn_xcr_channel_block_size[xc_ch] = 0U;
        }
#else //Selective Enable
        if (xc_ch == XCR_CH_02 || xc_ch == XCR_CH_17) // ch3, ch17
        {
            gn_xcr_channel_enable[xc_ch] = 1U;
            gn_xcr_channel_daisy_size[xc_ch] = XDR_DAISY_LENGTH;
            gn_xcr_channel_block_size[xc_ch] = (gn_xcr_channel_daisy_size[xc_ch] * gn_xcr_daisied_dev_blk_size);
        }
        else
        {
            gn_xcr_channel_enable[xc_ch] = 0U;
            gn_xcr_channel_daisy_size[xc_ch] = 0U;
            gn_xcr_channel_block_size[xc_ch] = 0U;
        }
#endif
    }

    gn_xcr_fll_cnt[0] = XCR_CONV_FREQ_TO_XCR_MCLK(TIM4_CLK);
    gn_xcr_fll_cnt[1] = XCR_CONV_FREQ_TO_XCR_MCLK(TIM4_CLK);
}

void xcr24_init(void)
{
    XCR_NSS_HI();
    xcr24_reset();
    xcr24_regs_init_table();
    xcr24_read_all();
}

void xcr24_trim_init(void)
{
    XCR_NSS_HI();
    xcr24_reset();
    xcr24_regs_trim_init_table();
    xcr24_read_all();
}

void xcr24_test_init(void)
{
    XCR_NSS_HI();
    xcr24_reset();
    xcr24_regs_test_init_table();
    xcr24_read_all();
}

const _xcr_group1_regs_t* xcr24_get_xcr24_set_gr1_regs(void)
{
    return &gt_xcr24_set_gr1_regs;
}
const _xcr_group1_regs_t* xcr24_get_xcr24_get_gr1_regs(void)
{
    return &gt_xcr24_get_gr1_regs;
}
const _xcr_group2_regs_t* xcr24_get_xcr24_set_gr2_regs(void)
{
    return &gt_xcr24_set_gr2_regs;
}
const _xcr_group2_regs_t* xcr24_get_xcr24_get_gr2_regs(void)
{
    return &gt_xcr24_get_gr2_regs;
}

static void xcr24_change_rw_grp_type(xcr_rw_grp_t in_grp)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    xcr_rw_grp_t now_grp = (xcr_rw_grp_t)(_rF0->bit.ADDR_EXT);

    if (in_grp >= XCR_RW_GRP_MAX)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\n%s invalid group type", __func__);
        return;
    }

    if (now_grp != in_grp)
    {
        _rF0->bit.ADDR_EXT = (uint16_t)(in_grp);
        _cmd_t cmd = { 0U };
        uint16_t tx_buffer[2] = { 0U };

        cmd.bit.code = CMD_CODE2;
        cmd.bit.addr = (XCR_OTP_BASE_ADDR + XCR_TEST_CONTROL);
        cmd.bit.size = 1U;

        tx_buffer[0] = cmd.ALL;
        tx_buffer[1] = _rF0->ALL;

        XCR_NSS_LO();
        uint8_t ret = spi_write(SPI1, tx_buffer, 2, 20U);
        XCR_NSS_HI();

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
        xcr24_spi_log_dump(tx_buffer, 2);
#endif
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nChange GRP TYPE to %u [0x%04X]", in_grp, gt_xcr24_set_otp_regs.reg._rF0.ALL);

        if(SPI_TIMEOUT == ret)
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
        }
    }
}

uint16_t xcr24_read_otp_control(uint16_t addr, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t rx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if((addr + length) >= XCR_OTP_MAX)
    {
        burst_size = (XCR_OTP_MAX - addr);
    }
    if(burst_size > 63U)
    {
        burst_size = 63U;
    }

    uint16_t spi_len = 1U + burst_size; /* HDR + readout */

    cmd.bit.code = CMD_CODE1;
    cmd.bit.addr = (XCR_OTP_BASE_ADDR + addr);
    cmd.bit.size = burst_size;

    tx_buffer[0U] = cmd.ALL;

    if (addr != XCR_TEST_CONTROL)
    {
        xcr24_change_rw_grp_type(XCR_RW_GRP1);
    }

    XCR_NSS_LO();
    uint8_t ret = spi_read(SPI1, tx_buffer, rx_buffer, spi_len, 20U);
    XCR_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi read timeout");
    }
    else
    {
        _xcr_otp_control_regs_t* _r = &gt_xcr24_get_otp_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = rx_buffer[XCR_SPI_HEADER_SIZE + i];
        }
    }
    return rx_buffer[XCR_SPI_HEADER_SIZE];
}

void xcr24_write_otp_control(uint16_t addr, const uint16_t* q, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if((addr + length) >= XCR_OTP_MAX)
    {
        burst_size = (XCR_OTP_MAX - addr);
    }
    if(burst_size > 63U)
    {
        burst_size = 63U;
    }

    uint16_t spi_len = 0U;

    cmd.bit.code = CMD_CODE2;
    cmd.bit.addr = (XCR_OTP_BASE_ADDR + addr);
    cmd.bit.size = burst_size;

    tx_buffer[spi_len++] = cmd.ALL;

    for(uint16_t i = 0U; i < burst_size; ++i)
    {
        tx_buffer[spi_len++] = *q++;
    }

    if (addr != XCR_TEST_CONTROL)
    {
        xcr24_change_rw_grp_type(XCR_RW_GRP1);
    }

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, tx_buffer, spi_len, 20U);
    XCR_NSS_HI();

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
    xcr24_spi_log_dump(tx_buffer, spi_len);
#endif

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xcr_otp_control_regs_t* _r = &gt_xcr24_set_otp_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = tx_buffer[XCR_SPI_HEADER_SIZE + i];
        }
    }
}

uint16_t xcr24_read_grp1_reg(uint16_t addr, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t rx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if((addr + length) >= XCR_GRP1_MAX)
    {
        burst_size = (XCR_GRP1_MAX - addr);
    }
    if(burst_size > 63U)
    {
        burst_size = 63U;
    }

    uint16_t spi_len = 1U + burst_size; /* HDR + readout */

    cmd.bit.code = CMD_CODE1;
    cmd.bit.addr = addr;
    cmd.bit.size = burst_size;

    tx_buffer[0U] = cmd.ALL;

    xcr24_change_rw_grp_type(XCR_RW_GRP1);

    XCR_NSS_LO();
    uint8_t ret = spi_read(SPI1, tx_buffer, rx_buffer, spi_len, 20U);  /* hdr + crc16 + payload + crc16 */
    XCR_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi read timeout");
    }
    else
    {
        _xcr_group1_regs_t* _r = &gt_xcr24_get_gr1_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = rx_buffer[XCR_SPI_HEADER_SIZE + i];
        }
    }
    return rx_buffer[XCR_SPI_HEADER_SIZE];
}

uint16_t xcr24_read_grp2_reg(uint16_t addr, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t rx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if((addr + length) >= XCR_GRP2_MAX)
    {
        burst_size = (XCR_GRP2_MAX - addr);
    }
    if(burst_size > 63U)
    {
        burst_size = 63U;
    }

    uint16_t spi_len = 1U + burst_size; /* HDR + readout */

    cmd.bit.code = CMD_CODE1;
    cmd.bit.addr = addr;
    cmd.bit.size = burst_size;

    tx_buffer[0U] = cmd.ALL;

    xcr24_change_rw_grp_type(XCR_RW_GRP2);

    XCR_NSS_LO();
    uint8_t ret = spi_read(SPI1, tx_buffer, rx_buffer, spi_len, 20U);  /* hdr + payload + crc16 */
    XCR_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi read timeout");
    }
    else
    {
        _xcr_group2_regs_t* _r = &gt_xcr24_get_gr2_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = rx_buffer[XCR_SPI_HEADER_SIZE + i];
        }
    }
    return rx_buffer[XCR_SPI_HEADER_SIZE];
}

void xcr24_write_grp1_reg(uint16_t addr, const uint16_t* q, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if((addr + length) >= XCR_GRP1_MAX)
    {
        burst_size = (XCR_GRP1_MAX - addr);
    }
    if(burst_size > 63U)
    {
        burst_size = 63U;
    }

    uint16_t spi_len = 0U;

    cmd.bit.code = CMD_CODE2;
    cmd.bit.addr = addr;
    cmd.bit.size = burst_size;

    tx_buffer[spi_len++] = cmd.ALL;

    for(uint16_t i = 0U; i < burst_size; ++i)
    {
        tx_buffer[spi_len++] = *q++;
    }

    xcr24_change_rw_grp_type(XCR_RW_GRP1);

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, tx_buffer, spi_len, 20U);
    XCR_NSS_HI();

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
    xcr24_spi_log_dump(tx_buffer, spi_len);
#endif

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xcr_group1_regs_t* _r = &gt_xcr24_set_gr1_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = tx_buffer[XCR_SPI_HEADER_SIZE + i];
        }
    }
}

void xcr24_write_grp2_reg(uint16_t addr, const uint16_t* q, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XCR_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if((addr + length) >= XCR_GRP2_MAX)
    {
        burst_size = (XCR_GRP2_MAX - addr);
    }
    if(burst_size > 63U)
    {
        burst_size = 63U;
    }

    uint16_t spi_len = 0U;

    cmd.bit.code = CMD_CODE2;
    cmd.bit.addr = addr;
    cmd.bit.size = burst_size;

    tx_buffer[spi_len++] = cmd.ALL;

    for(uint16_t i = 0U; i < burst_size; ++i)
    {
        tx_buffer[spi_len++] = *q++;
    }

    xcr24_change_rw_grp_type(XCR_RW_GRP2);

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, tx_buffer, spi_len, 20U);
    XCR_NSS_HI();

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
    xcr24_spi_log_dump(tx_buffer, spi_len);
#endif

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xcr_group2_regs_t* _r = &gt_xcr24_set_gr2_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = tx_buffer[1U + i];
        }
    }
}

void xcr24_set_ld_transfer(uint16_t* buffer, uint16_t length)
{
#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
    static bool is_first = true;
    if (is_first)
    {
        xcr24_spi_log_dump(buffer, length);
        is_first = false;
    }
#endif

    if((NULL == buffer) || (0U == length))
    {
        return;
    }

    gb_xcr_ld_transfer_spi_dma_flag = true;

    XCR_NSS_LO();

    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_3, (uint32_t)buffer);

    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_3, length);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_3);
}

bool xcr24_read_local(uint16_t ch_seg, uint16_t addr)
{
    uint16_t retry = 0U;

    _v_local_write_command_t _r03 = {0U, };
    _v_local_rw_pointer_reset_t _r10 = {0U, };
    _v_command_status_1_t* _r14 = &gt_xcr24_get_gr1_regs.reg._r14;
    _v_receive_status_t* _r16 = &gt_xcr24_get_gr1_regs.reg._r16;

    _r10.bit.local_transfer_pointer_rst = 1U;
    _r10.bit.local_receive_pointer_rst = 1U;
    xcr24_write_grp1_reg(XCR_LOCAL_RW_POINTER_RESET, &_r10.ALL, 1U);

    _r03.bit.addr = addr;
    _r03.bit.ch_seg = ch_seg;
    _r03.bit.enable = 1U;
    xcr24_write_grp1_reg(XCR_LOCAL_READ_COMMAND, &_r03.ALL, 1U);

    retry = 128U;
    do
    {
        //us_delay(1000); /* TODO : ???*/
        xcr24_read_grp1_reg(XCR_COMMAND_STATUS_1, 1U);
    }while((_r14->bit.local_r_doing == 1U) && --retry > 0U);

    retry = 128U;
    do
    {
        //us_delay(1000); /* TODO : ???*/
        xcr24_read_grp1_reg(XCR_RECEIVE_STATUS, 1U);
    }while((_r16->bit.rd_receive_doing == 1U) && --retry > 0U);

    return ((_r16->bit.rd_receive_done == 1U) && (_r16->bit.rd_receive_fail == 0U));
}

void xcr24_write_local(uint16_t ch_seg, uint16_t addr, uint16_t* data, uint16_t len)
{
    if(0U < len)
    {
        uint16_t retry = 32U;
        uint16_t offset = 0U;

        _v_local_write_command_t _r02 = {0, };
        _v_local_rw_pointer_reset_t _r10 = {0U, };
        _v_command_status_1_t* _r14 = &gt_xcr24_get_gr1_regs.reg._r14;

        _r10.bit.local_transfer_pointer_rst = 1U;
        _r10.bit.local_receive_pointer_rst = 1U;
        xcr24_write_grp1_reg(XCR_LOCAL_RW_POINTER_RESET, &_r10.ALL, 1U);

        while(len > 0U)
        {
            uint16_t local_rw_len = (len > XCR_SPI_RW_LEN) ? XCR_SPI_RW_LEN : len;

            xcr24_set_local_rw_data(XCR_PORT1_LOCAL_RW_DATA1 + offset, data + offset, local_rw_len);

            offset += local_rw_len;
            len -= local_rw_len;
        }

        _r02.bit.addr = addr;
        _r02.bit.ch_seg = ch_seg;
        _r02.bit.enable = 1;
        xcr24_write_grp1_reg(XCR_LOCAL_WRITE_COMMAND, &_r02.ALL, 1U);

        do
        {
            //us_delay(1000); /* TODO : ???*/
            xcr24_read_grp1_reg(XCR_COMMAND_STATUS_1, 1U);
        }while((_r14->bit.local_r_doing == 1U) && --retry > 0U);
    }
}

void xcr24_get_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len)
{
    uint16_t spi_buffer[XCR_SPI_RW_LEN + 1U] = { 0, };
    uint16_t temp[XCR_SPI_RW_LEN + 1U] = { 0, };

    _cmd_t _cmd = { 0U, };

    if(len > XCR_SPI_RW_LEN)
    {
        len = XCR_SPI_RW_LEN;
    }

    _cmd.bit.code = CMD_CODE1;
    _cmd.bit.addr = addr;
    _cmd.bit.size = (len - 0U);
    spi_buffer[0U] = _cmd.ALL;

    uint16_t crc16 = Calculate_CRC16_CCITT_False(spi_buffer, 1U);

    temp[1U] = crc16;

    xcr24_change_rw_grp_type(XCR_RW_GRP1);

    XCR_NSS_LO();
    uint8_t ret = spi_read(SPI1, spi_buffer, temp, (len + 1U + 1U), 20U);  /* hdr + crc16 + payload + crc16 */
    XCR_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi read timeout");
    }
    else
    {

    }
    if(len > 0U && p_data != NULL)
    {
        memcpy(p_data, &temp[1], len * sizeof(uint16_t));
    }
}

void xcr24_set_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len)
{
    uint16_t spi_buffer[XCR_SPI_RW_LEN + 1U] = { 0, };

    _cmd_t _cmd = { 0U, };

    if(len > XCR_SPI_RW_LEN)
    {
        len = XCR_SPI_RW_LEN;
    }

    _cmd.bit.code = CMD_CODE2;
    _cmd.bit.addr = addr;
    _cmd.bit.size = (len - 0U);

    spi_buffer[0] = _cmd.ALL;
    memcpy(&spi_buffer[1], p_data, (len * sizeof(uint16_t)));

    uint16_t crc16 = Calculate_CRC16_CCITT_False(spi_buffer, len + 1U);

    spi_buffer[1U + len] = crc16;

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, spi_buffer, len + 1U + 1U, 20U);
    XCR_NSS_HI();
}

void xcr24_set_fll_cnt(uint8_t fll_ch, uint32_t fll_cnt)
{
    if (fll_ch < 3)
    {
        if (fll_ch == 0)
        {
            _v_fllcnt11_t* _r37 = &gt_xcr24_set_gr1_regs.reg._r37;
            _r37->bit.fll1cnt = ((fll_cnt & FLL_BIT_B15_B0) >>  FLL_BIT_SHIFT_LSB);
            xcr24_write_grp1_reg(XCR_FLLCNT11, &_r37->ALL, 1U);

            _v_fllcnt12_t* _r38 = &gt_xcr24_set_gr1_regs.reg._r38;
            _r38->bit.fll1cnt = ((fll_cnt & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            xcr24_write_grp1_reg(XCR_FLLCNT12, &_r38->ALL, 1U);

            _v_fllcnt21_t* _r39 = &gt_xcr24_set_gr1_regs.reg._r39;
            _r39->bit.fll2cnt = ((fll_cnt & FLL_BIT_B15_B0) >>  FLL_BIT_SHIFT_LSB);
            xcr24_write_grp1_reg(XCR_FLLCNT21, &_r39->ALL, 1U);

            _v_fllcnt22_t* _r3A = &gt_xcr24_set_gr1_regs.reg._r3A;
            _r3A->bit.fll2cnt = ((fll_cnt & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            xcr24_write_grp1_reg(XCR_FLLCNT22, &_r3A->ALL, 1U);
        }
        else if (fll_ch == 1)
        {
            _v_fllcnt11_t* _r37 = &gt_xcr24_set_gr1_regs.reg._r37;
            _r37->bit.fll1cnt = ((fll_cnt & FLL_BIT_B15_B0) >>  FLL_BIT_SHIFT_LSB);
            xcr24_write_grp1_reg(XCR_FLLCNT11, &_r37->ALL, 1U);

            _v_fllcnt12_t* _r38 = &gt_xcr24_set_gr1_regs.reg._r38;
            _r38->bit.fll1cnt = ((fll_cnt & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            xcr24_write_grp1_reg(XCR_FLLCNT12, &_r38->ALL, 1U);
        }
        else if (fll_ch == 2)
        {
            _v_fllcnt21_t* _r39 = &gt_xcr24_set_gr1_regs.reg._r39;
            _r39->bit.fll2cnt = ((fll_cnt & FLL_BIT_B15_B0) >>  FLL_BIT_SHIFT_LSB);
            xcr24_write_grp1_reg(XCR_FLLCNT21, &_r39->ALL, 1U);

            _v_fllcnt22_t* _r3A = &gt_xcr24_set_gr1_regs.reg._r3A;
            _r3A->bit.fll2cnt = ((fll_cnt & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            xcr24_write_grp1_reg(XCR_FLLCNT22, &_r3A->ALL, 1U);
        }
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nChange FLL[ch:%d] count to %d", fll_ch, fll_cnt);
    }
    else
    {
        FATAL_INVALID_INPUT(fll_ch);
    }
}

void xcr24_nINT_FT_handler(void)
{
    static uint16_t prev_cause_of_INT = 0xFFFFU;
    static uint16_t duplicate_vsync_cnt = 0U;

    uint16_t cause_of_INT = xcr24_read_grp1_reg(XCR_INTERRUPT_STATUS, 1U);

    if (cause_of_INT != prev_cause_of_INT)
    {
        static const struct
        {
            uint16_t mask;
            const char *msg;
        } int_flags[] = {
            {0x20U, "int_fault_source_1"},
            {0x10U, "int_fb3"},
            {0x08U, "int_fb2"},
            {0x04U, "int_fb1"},
            {0x02U, "int_ld"},
            {0x01U, "int_fault_source_2"}
        };

        const uint8_t fault_1 = ((cause_of_INT & 0x20U) >> 5U);
        const uint8_t fb3 =     ((cause_of_INT & 0x10U) >> 4U);
        const uint8_t fb2 =     ((cause_of_INT & 0x08U) >> 3U);
        const uint8_t fb1 =     ((cause_of_INT & 0x04U) >> 2U);
        const uint8_t LD =      ((cause_of_INT & 0x02U) >> 1U);
        const uint8_t fault_2 = ((cause_of_INT & 0x01U) >> 0U);
        comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 nINT_FT interrupt 0x%02X: 0x%04X", XCR_INTERRUPT_STATUS, cause_of_INT);

        for (uint8_t i = 0; i < sizeof(int_flags) / sizeof(int_flags[0]); ++i)
        {
            if (cause_of_INT & int_flags[i].mask)
            {
                comm_UART_Printf(LOG_LV_INFO, int_flags[i].msg);
            }
        }

        prev_cause_of_INT = cause_of_INT;
        duplicate_vsync_cnt = 0U;
    }
    else
    {
        if ((++duplicate_vsync_cnt) == 240) // Log every 240 duplicates to avoid flooding the log
        {
            comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 nINT_FT interrupt 0x%02X: 0x%04X (duplicate %d times)", XCR_INTERRUPT_STATUS, cause_of_INT, duplicate_vsync_cnt);
            duplicate_vsync_cnt = 0;
        }
    }
}

void xcr24_nINT_LD_handler(void)
{
    comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 nINT_LD interrupt");
}

void xcr24_trim_set_efuse_enable(bool en)
{
    gb_xcr_do_efuse = en;
}

bool xcr24_trim_get_efuse_enable(void)
{
    return gb_xcr_do_efuse;
}

void xcr24_trim_init_1v5_ldo_dig(void)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);
}

void xcr24_trim_init_dac_3v0(void)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_current_target_dac1_t* _r44 = &gt_xcr24_set_gr1_regs.reg._r44;
    _r44->bit.curr_tgt_dac1 = 0xFFF;
    xcr24_write_grp1_reg(XCR_CURRENT_TARGET_DAC1, &_r44->ALL, 1U);
}

void xcr24_trim_init_dac1_ofs(void)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_current_target_dac1_t* _r44 = &gt_xcr24_set_gr1_regs.reg._r44;
    _r44->bit.curr_tgt_dac1 = XCR_TRIM_DAC_INPUT;
    xcr24_write_grp1_reg(XCR_CURRENT_TARGET_DAC1, &_r44->ALL, 1U);

    _v_dac_nf_control_t* _r42 = &gt_xcr24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xcr24_write_grp1_reg(XCR_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xcr24_trim_init_dac2_ofs(void)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_current_target_dac2_t* _r45 = &gt_xcr24_set_gr1_regs.reg._r45;
    _r45->bit.curr_tgt_dac2 = XCR_TRIM_DAC_INPUT;
    xcr24_write_grp1_reg(XCR_CURRENT_TARGET_DAC2, &_r45->ALL, 1U);

    _v_dac_nf_control_t* _r42 = &gt_xcr24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xcr24_write_grp1_reg(XCR_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xcr24_trim_init_dac3_ofs(void)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_current_target_dac3_t* _r46 = &gt_xcr24_set_gr1_regs.reg._r46;
    _r46->bit.curr_tgt_dac3 = XCR_TRIM_DAC_INPUT;
    xcr24_write_grp1_reg(XCR_CURRENT_TARGET_DAC3, &_r46->ALL, 1U);

    _v_dac_nf_control_t* _r42 = &gt_xcr24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xcr24_write_grp1_reg(XCR_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xcr24_trim_init_1v5_ldo_osc(void)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_ana_test_t* _r12 = &gt_xcr24_set_gr2_regs.reg._r12;
    _r12->bit.TEST_ANA_EN = 6U;
    xcr24_write_grp2_reg(XCR_GRP2_ANA_TEST, &_r12->ALL, 1U);
}

void xcr24_trim_init_osc_a(void)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;

    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;

    _rF0->bit.MCLK_SEL = XCR_MCLK_SEL_OSC_A;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_a1_t* _r65 = &gt_xcr24_set_gr1_regs.reg._r65;
    _r65->bit.OSC_MAN_EN_A = 1U;
    _r65->bit.FLT_CTL_A = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_A1, &_r65->ALL, 1U);

    _v_osc_fll_man_a2_t* _r66 = &gt_xcr24_set_gr1_regs.reg._r66;
    _r66->bit.OSC_FLL_MAN_A = 0x8000U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_A2, &_r66->ALL, 1U);
}

void xcr24_trim_init_osc_b(void)
{
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;

    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;

    _rF0->bit.MCLK_SEL = XCR_MCLK_SEL_OSC_B;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xcr24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 1U;
    _r67->bit.FLT_CTL_B = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_B1, &_r67->ALL, 1U);

    _v_osc_fll_man_b2_t* _r68 = &gt_xcr24_set_gr1_regs.reg._r68;
    _r68->bit.OSC_FLL_MAN_B = 0x8000U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_B2, &_r68->ALL, 1U);
}

bool xcr24_trim_set_1v5_ldo_dig(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XCR_MAX_1V5_LDO_DIG)
    {
        _v_mirror1_t* _rF5 = &gt_xcr24_set_otp_regs.reg._rF5;
        _rF5->bit.vctl_ldo = reg_val;
        xcr24_write_otp_control(XCR_MIRROR1, &_rF5->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xcr24_trim_set_dac_3v0(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XCR_MAX_DAC_3V0)
    {
        _v_mirror2_t* _rF6 = &gt_xcr24_set_otp_regs.reg._rF6;
        _rF6->bit.dac_ctl = reg_val;
        xcr24_write_otp_control(XCR_MIRROR2, &_rF6->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xcr24_trim_set_dac1_ofs(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XCR_MAX_DAC1_OFS)
    {
        _v_mirror2_t* _rF6 = &gt_xcr24_set_otp_regs.reg._rF6;
        _rF6->bit.dac1_ofs = reg_val;
        xcr24_write_otp_control(XCR_MIRROR2, &_rF6->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xcr24_trim_set_dac2_ofs(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XCR_MAX_DAC2_OFS)
    {
        _v_mirror3_t* _rF7 = &gt_xcr24_set_otp_regs.reg._rF7;
        _rF7->bit.dac2_ofs = reg_val;
        xcr24_write_otp_control(XCR_MIRROR3, &_rF7->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xcr24_trim_set_dac3_ofs(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XCR_MAX_DAC3_OFS)
    {
        _v_mirror3_t* _rF7 = &gt_xcr24_set_otp_regs.reg._rF7;
        _rF7->bit.dac3_ofs = reg_val;
        xcr24_write_otp_control(XCR_MIRROR3, &_rF7->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xcr24_trim_set_1v5_ldo_osc(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XCR_MAX_1V5_LDO_OSC)
    {
        _v_mirror4_t* _rF8 = &gt_xcr24_set_otp_regs.reg._rF8;
        _rF8->bit.ldo_osc_ctl = reg_val;
        xcr24_write_otp_control(XCR_MIRROR4, &_rF8->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xcr24_trim_set_osc_a(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XCR_MAX_OSC_A)
    {
        _v_mirror4_t* _rF8 = &gt_xcr24_set_otp_regs.reg._rF8;
        _rF8->bit.osc_rctl = reg_val;
        xcr24_write_otp_control(XCR_MIRROR4, &_rF8->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xcr24_trim_set_osc_b(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XCR_MAX_OSC_B)
    {
        _v_mirror5_t* _rF9 = &gt_xcr24_set_otp_regs.reg._rF9;
        _rF9->bit.osc_rctl2 = reg_val;
        xcr24_write_otp_control(XCR_MIRROR5, &_rF9->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

void xcr24_trim_init_efuse(void)
{
    _v_otp_pg_access_t* _rF1 = &gt_xcr24_set_otp_regs.reg._rF1;
    _rF1->bit.OTP_PG_ACC_CYCLE = 0x03FFU;
    xcr24_write_otp_control(XCR_OTP_PG_ACCESS, &_rF1->ALL, 1U);

    _v_otp_write_t* _rF2 = &gt_xcr24_set_otp_regs.reg._rF2;
    _rF2->bit.OTP_WSEL = 0x04U;
    _rF2->bit.OTP_RD = 0U;
    _rF2->bit.OTP_PG_DONE = 0U;
    xcr24_write_otp_control(XCR_OTP_WRITE, &_rF2->ALL, 1U);
}

void xcr24_trim_start_efuse(void)
{
    _v_otp_rd_prog_t* _rF3 = &gt_xcr24_set_otp_regs.reg._rF3;
    _rF3->bit.OTP_PG_S = 1U;
    xcr24_write_otp_control(XCR_OTP_RD_PROG, &_rF3->ALL, 1U);
}

void xcr24_trim_save_mirror_register(void)
{
    xcr24_read_otp_control(XCR_MIRROR1, 5U);
}

uint32_t xcr24_trim_verify_mirror_dump(void)
{
    uint32_t ret = 0U;
    for (xcr_addr_otp_t mirror_addr = XCR_MIRROR1; mirror_addr < XCR_GATE_CONTROL; ++mirror_addr) // 0xF5 ~ 0xF9
    {
        uint16_t saved_reg = gt_xcr24_get_otp_regs.ALL[mirror_addr];
        uint16_t read_reg = xcr24_read_otp_control(mirror_addr, 1U);

        if (mirror_addr == XCR_MIRROR5)
        {
            saved_reg &= 0x1FU; // masking low 5 bits for mirror5 register, as only 5 bits are valid for this register
            read_reg &= 0x1FU; // masking low 5 bits for mirror5 register, as only 5 bits are valid for this register
        }
        if (saved_reg != read_reg)
        {
            ret |= (1UL << mirror_addr);
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✕]%s ADDR [0x%02X] - [0x%04X - 0x%04X]", \
                ANSI_FONT_RED, ANSI_FONT_NONE, (XCR_OTP_BASE_ADDR + mirror_addr), saved_reg, read_reg);
        }
        else
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✔]%s ADDR [0x%02X] - [0x%04X - 0x%04X]", \
                ANSI_FONT_GREEN, ANSI_FONT_NONE, (XCR_OTP_BASE_ADDR + mirror_addr), saved_reg, read_reg);
        }
    }
    return ret;
}

void xcr24_test_init_icc_stby(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_ICC_P, ADS114S08_CH_XC_ICC_N);
}

void xcr24_test_init_icc_actv(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_ICC_P, ADS114S08_CH_XC_ICC_N);
    // set proper xdr12 register
    xcr24_trim_init();
}

void xcr24_test_init_ldo(void)
{
    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_LDO, ADS_AINCOM);
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

}
void xcr24_test_init_ldo_fll_a(void)
{
    // turn on proper power if needed like VLE

    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_1V5, ADS_AINCOM);
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_ana_test_t* _r12 = &gt_xcr24_set_gr2_regs.reg._r12;
    _r12->bit.TEST_ANA_EN = 6U;
    xcr24_write_grp2_reg(XCR_GRP2_ANA_TEST, &_r12->ALL, 1U);
}

void xcr24_test_init_ldo_fll_b(void)
{
    // turn on proper power if needed like VLE

    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_1V5, ADS_AINCOM);
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_ana_test_t* _r12 = &gt_xcr24_set_gr2_regs.reg._r12;
    _r12->bit.TEST_ANA_EN = 5U;
    xcr24_write_grp2_reg(XCR_GRP2_ANA_TEST, &_r12->ALL, 1U);
}

void xcr24_test_init_fll_a_30m(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;

    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;

    _rF0->bit.MCLK_SEL = XCR_MCLK_SEL_OSC_A;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_a1_t* _r65 = &gt_xcr24_set_gr1_regs.reg._r65;
    _r65->bit.OSC_MAN_EN_A = 0U;
    _r65->bit.FLT_CTL_A = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_A1, &_r65->ALL, 1U);

    const float xc_mclk = 30000000.0f; /* 30MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xcr24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xcr24_write_grp1_reg(XCR_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xcr24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    xcr24_write_grp1_reg(XCR_FLLCNT12, &_r38->ALL, 1U);
}

void xcr24_test_init_fll_a_35m(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;

    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;

    _rF0->bit.MCLK_SEL = XCR_MCLK_SEL_OSC_A;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_a1_t* _r65 = &gt_xcr24_set_gr1_regs.reg._r65;
    _r65->bit.OSC_MAN_EN_A = 0U;
    _r65->bit.FLT_CTL_A = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_A1, &_r65->ALL, 1U);

    const float xc_mclk = 35000000.0f; /* 35MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xcr24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xcr24_write_grp1_reg(XCR_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xcr24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    xcr24_write_grp1_reg(XCR_FLLCNT12, &_r38->ALL, 1U);
}

void xcr24_test_init_fll_a_40m(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;

    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;

    _rF0->bit.MCLK_SEL = XCR_MCLK_SEL_OSC_A;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_a1_t* _r65 = &gt_xcr24_set_gr1_regs.reg._r65;
    _r65->bit.OSC_MAN_EN_A = 0U;
    _r65->bit.FLT_CTL_A = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_A1, &_r65->ALL, 1U);

    const float xc_mclk = 40000000.0f; /* 40MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xcr24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xcr24_write_grp1_reg(XCR_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xcr24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    xcr24_write_grp1_reg(XCR_FLLCNT12, &_r38->ALL, 1U);
}

void xcr24_test_init_fll_b_30m(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;

    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;

    _rF0->bit.MCLK_SEL = XCR_MCLK_SEL_OSC_B;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xcr24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_B1, &_r67->ALL, 1U);

    const float xc_mclk = 30000000.0f; /* 30MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xcr24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xcr24_write_grp1_reg(XCR_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xcr24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    xcr24_write_grp1_reg(XCR_FLLCNT12, &_r38->ALL, 1U);
}

void xcr24_test_init_fll_b_35m(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;

    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;

    _rF0->bit.MCLK_SEL = XCR_MCLK_SEL_OSC_B;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xcr24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_B1, &_r67->ALL, 1U);

    const float xc_mclk = 35000000.0f; /* 35MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xcr24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xcr24_write_grp1_reg(XCR_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xcr24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    xcr24_write_grp1_reg(XCR_FLLCNT12, &_r38->ALL, 1U);
}

void xcr24_test_init_fll_b_40m(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;

    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;

    _rF0->bit.MCLK_SEL = XCR_MCLK_SEL_OSC_B;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xcr24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_B1, &_r67->ALL, 1U);

    const float xc_mclk = 40000000.0f; /* 40MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xcr24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xcr24_write_grp1_reg(XCR_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xcr24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    xcr24_write_grp1_reg(XCR_FLLCNT12, &_r38->ALL, 1U);
}

void xcr24_test_init_dac_p1(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->ALL = 0U;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    xcr24_test_set_curr_tgt_dac(XCR_TEST_DAC_INPUT_P1);

    _v_dac_nf_control_t* _r42 = &gt_xcr24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xcr24_write_grp1_reg(XCR_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xcr24_test_init_dac_p2(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    xcr24_test_set_curr_tgt_dac(XCR_TEST_DAC_INPUT_P2);

    _v_dac_nf_control_t* _r42 = &gt_xcr24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xcr24_write_grp1_reg(XCR_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xcr24_test_init_dac_p3(void)
{
    // set proper xdr12 register
    _v_test_control_t* _rF0 = &gt_xcr24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    xcr24_test_set_curr_tgt_dac(XCR_TEST_DAC_INPUT_P3);

    _v_dac_nf_control_t* _r42 = &gt_xcr24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xcr24_write_grp1_reg(XCR_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xcr24_test_start_icc_stby(void)
{
    ADS114S08_Set_Start(true);
}
void xcr24_test_start_icc_actv(void)
{
    ADS114S08_Set_Start(true);
}
void xcr24_test_start_ldo(void)
{
    ADS114S08_Set_Start(true);
}
void xcr24_test_start_ldo_fll_a(void)
{
    ADS114S08_Set_Start(true);
}
void xcr24_test_start_ldo_fll_b(void)
{
    ADS114S08_Set_Start(true);
}
void xcr24_test_start_fll_a_30m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xcr24_test_start_fll_a_35m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xcr24_test_start_fll_a_40m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xcr24_test_start_fll_b_30m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xcr24_test_start_fll_b_35m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xcr24_test_start_fll_b_40m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xcr24_test_start_dac_p1(void)
{
    ADS114S08_Set_Start(true);
}
void xcr24_test_start_dac_p2(void)
{
    ADS114S08_Set_Start(true);
}
void xcr24_test_start_dac_p3(void)
{
    ADS114S08_Set_Start(true);
}

void xcr24_test_set_curr_tgt_dac(uint16_t curr_tgt_dac)
{
    if (curr_tgt_dac < 4096)
    {
        _v_current_target_dac1_t* _r44 = &gt_xcr24_set_gr1_regs.reg._r44;
        _r44->bit.curr_tgt_dac1 = curr_tgt_dac;
        xcr24_write_grp1_reg(XCR_CURRENT_TARGET_DAC1, &_r44->ALL, 1U);

        _v_current_target_dac2_t* _r45 = &gt_xcr24_set_gr1_regs.reg._r45;
        _r45->bit.curr_tgt_dac2 = curr_tgt_dac;
        xcr24_write_grp1_reg(XCR_CURRENT_TARGET_DAC2, &_r45->ALL, 1U);

        _v_current_target_dac3_t* _r46 = &gt_xcr24_set_gr1_regs.reg._r46;
        _r46->bit.curr_tgt_dac3 = curr_tgt_dac;
        xcr24_write_grp1_reg(XCR_CURRENT_TARGET_DAC3, &_r46->ALL, 1U);
    }
    else
    {
        FATAL_INVALID_INPUT(curr_tgt_dac);
    }
}