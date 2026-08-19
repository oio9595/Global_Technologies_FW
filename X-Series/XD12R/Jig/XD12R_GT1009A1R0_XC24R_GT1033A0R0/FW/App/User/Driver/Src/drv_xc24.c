#include <stdio.h>
#include <string.h>

#include "crc.h"
#include "drv_spi.h"
#include "drv_xc24.h"
#include "drv_xd12.h"
#include "drv_ads124s08.h"
#include "comm_debugging.h"

#define FLL_BIT_SHIFT_LSB       (0U)
#define FLL_BIT_SHIFT_MSB       (16U)

#define FLL_BIT_B15_B0          (0x00FFFFUL)
#define FLL_BIT_B20_B16         (0x1F0000UL)

#define XC_FLL_PAD_VSYNC        (0U)
#define XC_FLL_PAD_FLLSYNC      (1U)

#define XC_MCLK_SEL_OSC_A       (1U)
#define XC_MCLK_SEL_OSC_B       (0U)

#define XC_V_MASK_ON_TIME_US    (4.5f)      /* 4.5us */
#define XC_V_MASK_OFF_TIME_US   (4.5f)      /* 4.5us */

#define XC_SVO_ON_TIME_US       (10.0f)     /* 10us */
#define XC_SVO1_OFF_TIME_US     (100.0f)    /* 100us */
#define XC_SVO2_OFF_TIME_US     (144.43f)   /* 144.43us */
#define XC_SVO3_OFF_TIME_US     (96.0f)     /* 96us */

#define XC_SPI_HEADER_SIZE      (1U)
#define XC_SPI_BURST_MAX_SIZE   (64U)
#define XC_SPI_BUFF_MAX_SIZE    (XC_SPI_HEADER_SIZE + XC_SPI_BURST_MAX_SIZE)

#define XC_TRIM_DAC_INPUT       (0xC8U)

#define XC_TEST_DAC_INPUT_P1    (0x0C8U)
#define XC_TEST_DAC_INPUT_P2    (0x3E8U)
#define XC_TEST_DAC_INPUT_P3    (0xBB8U)

#define XC_MAX_1V5_LDO_DIG      (0x001FU)
#define XC_MAX_DAC_3V0          (0x003FU)
#define XC_MAX_DAC1_OFS         (0x00FFU)
#define XC_MAX_DAC2_OFS         (0x00FFU)
#define XC_MAX_DAC3_OFS         (0x00FFU)
#define XC_MAX_1V5_LDO_OSC      (0x001FU)
#define XC_MAX_OSC_A            (0x001FU)
#define XC_MAX_OSC_B            (0x001FU)

#define XC_FUNCTION_DIS         (0U)
#define XC_FUNCTION_EN          (1U)

#if (XD_LD_DATA_BIT == XD_LD_DATA_12BIT)
    #define XC_LD_WIDTH         (LD_WIDTH1)
#elif (XD_LD_DATA_BIT == XD_LD_DATA_14BIT)
    #define XC_LD_WIDTH         (LD_WIDTH2)
#else
    #error "Unsupported XD_LD_DATA_BIT value"
#endif

#define XC_SYNCMODE_NONE        (0U)
#define XC_SYNCMODE_CMD_SVO     (1U)
#define XC_SYNCMODE_VO_SVO      (2U)
#define XC_SYNCMODE_RSO_CMD     (3U)

#define XC_SVO_ACTIVE_NONE      (0U)
#define XC_SVO_ACTIVE_NDF       (1U)
#define XC_SVO_ACTIVE_23        (2U)
#define XC_SVO_ACTIVE_123       (3U)

volatile bool gb_xc_ld_transfer_spi_dma_flag;

static uint8_t gn_xc_daisied_dev_blk_size;
static uint8_t gn_xc_channel_enable[XC_CH_SIZE_MAX];
static uint8_t gn_xc_channel_daisy_size[XC_CH_SIZE_MAX];
static uint8_t gn_xc_channel_block_size[XC_CH_SIZE_MAX];
static uint32_t gn_xc_fll_cnt[2];

static _xc_group1_regs_t gt_xc24_set_gr1_regs;
static _xc_group1_regs_t gt_xc24_get_gr1_regs;

static _xc_group2_regs_t gt_xc24_set_gr2_regs;
static _xc_group2_regs_t gt_xc24_get_gr2_regs;

static _xc_otp_control_regs_t gt_xc24_set_otp_regs; /* base address 0xF0 */
static _xc_otp_control_regs_t gt_xc24_get_otp_regs; /* base address 0xF0 */

static bool gb_xc_do_efuse;

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
static void xc24_spi_log_dump(const uint16_t* out, uint16_t len)
{
    if (out == NULL || len == 0)
    {
        return;
    }
    UART_PutChar('\r');
    UART_PutChar('\n');

    for (uint16_t i = 0; i < len; ++i)
    {
        uint16_t val = out[i];

        // "0x" 전송
        UART_PutChar('0');
        UART_PutChar('x');

        // 16진수 4자리 변환 및 전송 (상위 니블부터 하위 니블까지)
        for (int8_t shift = 12; shift >= 0; shift -= 4)
        {
            uint8_t nibble = (val >> shift) & 0x0F;
            char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));

            UART_PutChar(hex_char);
        }

        // 구분자 ',' 전송
        UART_PutChar(',');
    }
}
#endif

static void xc24_regs_init_table(void)
{
    _xc_group1_regs_t* _r1 = &gt_xc24_set_gr1_regs;
    for(xc_addr_grp1_t addr = XC_RESET; addr < XC_GRP1_MAX; ++addr)
    {
        switch(addr)
        {
        case XC_FAULT_READ_COMMAND:
            _r1->reg._r05.bit.ft_mode = 1U;
            break;
        case XC_LD_TRANSFER_COMMAND:
            _r1->reg._r06.bit.ld_type = LED_PER_BLOCK;
            break;
        case XC_SYNC_GEN_COMMAND:
            #if (XD_SYNC_MODE == XD_SYNC_MODE_CMD)
                _r1->reg._r07.bit.syncmode = XC_SYNCMODE_CMD_SVO;
            #elif(XD_SYNC_MODE == XD_SYNC_MODE_SVI)
                _r1->reg._r07.bit.syncmode = XC_SYNCMODE_VO_SVO;
            #else
                #error "Unsupported XD_SYNC_MODE value"
            #endif
            _r1->reg._r07.bit.enable = XC_FUNCTION_DIS;
            break;
        case XC_COMMAND_AUTO_ENABLE:
            #if (XD_SYNC_MODE == XD_SYNC_MODE_CMD)
                _r1->reg._r08.bit.sync_auto_en = XC_FUNCTION_EN;
            #elif(XD_SYNC_MODE == XD_SYNC_MODE_SVI)
                _r1->reg._r08.bit.sync_auto_en = XC_FUNCTION_DIS;
            #else
                #error "Unsupported XD_SYNC_MODE value"
            #endif
            _r1->reg._r08.bit.fault_auto_en = XC_FUNCTION_EN;
            _r1->reg._r08.bit.timeout_en = XC_FUNCTION_EN;
            break;
        case XC_LD_START_POINTER_TH:
            _r1->reg._r0C.bit.ld_transfer_start_pointer = 9U;
            break;
        case XC_FAULT_AUTO_READ_INTERVAL:
            _r1->reg._r11.bit.fault_auto_rd_interval = 0xFFFFU;
            break;
        case XC_FAULT_AUTO_READ_EVENT:
            _r1->reg._r12.bit.fault_auto_rd_interval = 1U;
            _r1->reg._r12.bit.fault_auto_rd_timer_event = 1U;
            break;
        case XC_INTERRUPT_ENABLE:
            _r1->reg._r13.bit.int_fb_en = XC_FUNCTION_EN;
            break;
        case XC_CLK_CONTROL_1:
            // _r1->reg._r1B.bit.serializer_skew_en = XC_FUNCTION_DIS;
            // _r1->reg._r1B.bit.osc1_spread_en = XC_FUNCTION_EN;
            // _r1->reg._r1B.bit.serializer_clk_sel1 = XC_FUNCTION_DIS;
            // _r1->reg._r1B.bit.sprd1_gain = XC_FUNCTION_DIS;
            // _r1->reg._r1B.bit.serializer_clk_sel2 = XC_FUNCTION_DIS;
            // _r1->reg._r1B.bit.ld_rd_clk_sel = XC_FUNCTION_DIS;
            // _r1->reg._r1B.bit.spread1_spd = XC_FUNCTION_DIS;
            _r1->reg._r1B.ALL = 0x0808U;
            break;
        case XC_CLK_CONTROL_2:
            //_r1->reg._r1C.bit.mclk_mode = XC_FUNCTION_DIS;
            //_r1->reg._r1C.bit.osc2_spread_en = XC_FUNCTION_DIS;
            //_r1->reg._r1C.bit.sprd2_gain = XC_FUNCTION_DIS;
            //_r1->reg._r1C.bit.spread2_spd = XC_FUNCTION_DIS;
            _r1->reg._r1C.ALL = 0x0000U;
            break;
        case XC_SERIALIZER_CLOCK_GEN:
            _r1->reg._r1D.bit.serial_clk_high = XC_SERIAL_CLK_HIGH;
            _r1->reg._r1D.bit.serial_clk_low = XC_SERIAL_CLK_LOW;
            break;
        case XC_LATENCY:
            _r1->reg._r1E.bit.cmd_latency = 0xC0U;
            _r1->reg._r1E.bit.serial_latency = 0x70U;
            break;
        case XC_TIMEOUT:
            _r1->reg._r1F.bit.timeout = 0x06D6U;
            break;
        case XC_DAISIED_DEVICE_CH_SIZE:
            _r1->reg._r20.bit.daisied_dev_blk_size = gn_xc_daisied_dev_blk_size;
            break;
        case XC_DAISY_SIZE_1:
            _r1->reg._r21.bit.daisy_size_ch1 = gn_xc_channel_daisy_size[0U];
            _r1->reg._r21.bit.daisy_size_ch2 = gn_xc_channel_daisy_size[1U];
            _r1->reg._r21.bit.daisy_size_ch3 = gn_xc_channel_daisy_size[2U];
            break;
        case XC_DAISY_SIZE_2:
            _r1->reg._r22.bit.daisy_size_ch4 = gn_xc_channel_daisy_size[3U];
            _r1->reg._r22.bit.daisy_size_ch5 = gn_xc_channel_daisy_size[4U];
            _r1->reg._r22.bit.daisy_size_ch6 = gn_xc_channel_daisy_size[5U];
            break;
        case XC_DAISY_SIZE_3:
            _r1->reg._r23.bit.daisy_size_ch7 = gn_xc_channel_daisy_size[6U];
            _r1->reg._r23.bit.daisy_size_ch8 = gn_xc_channel_daisy_size[7U];
            _r1->reg._r23.bit.daisy_size_ch9 = gn_xc_channel_daisy_size[8U];
            break;
        case XC_DAISY_SIZE_4:
            _r1->reg._r24.bit.daisy_size_ch10 = gn_xc_channel_daisy_size[9U];
            _r1->reg._r24.bit.daisy_size_ch11 = gn_xc_channel_daisy_size[10U];
            _r1->reg._r24.bit.daisy_size_ch12 = gn_xc_channel_daisy_size[11U];
            break;
        case XC_DAISY_SIZE_5:
            _r1->reg._r25.bit.daisy_size_ch13 = gn_xc_channel_daisy_size[12U];
            _r1->reg._r25.bit.daisy_size_ch14 = gn_xc_channel_daisy_size[13U];
            _r1->reg._r25.bit.daisy_size_ch15 = gn_xc_channel_daisy_size[14U];
            break;
        case XC_DAISY_SIZE_6:
            _r1->reg._r26.bit.daisy_size_ch16 = gn_xc_channel_daisy_size[15U];
            _r1->reg._r26.bit.daisy_size_ch17 = gn_xc_channel_daisy_size[16U];
            _r1->reg._r26.bit.daisy_size_ch18 = gn_xc_channel_daisy_size[17U];
            break;
        case XC_DAISY_SIZE_7:
            _r1->reg._r27.bit.daisy_size_ch19 = gn_xc_channel_daisy_size[18U];
            _r1->reg._r27.bit.daisy_size_ch20 = gn_xc_channel_daisy_size[19U];
            _r1->reg._r27.bit.daisy_size_ch21 = gn_xc_channel_daisy_size[20U];
            break;
        case XC_DAISY_SIZE_8:
            _r1->reg._r28.bit.daisy_size_ch22 = gn_xc_channel_daisy_size[21U];
            _r1->reg._r28.bit.daisy_size_ch23 = gn_xc_channel_daisy_size[22U];
            _r1->reg._r28.bit.daisy_size_ch24 = gn_xc_channel_daisy_size[23U];
            break;
        case XC_BLOCK_SIZE_1:
            _r1->reg._r29.bit.total_blk_size_ch1 = gn_xc_channel_block_size[0U];
            _r1->reg._r29.bit.total_blk_size_ch2 = gn_xc_channel_block_size[1U];
            break;
        case XC_BLOCK_SIZE_2:
            _r1->reg._r2A.bit.total_blk_size_ch3 = gn_xc_channel_block_size[2U];
            _r1->reg._r2A.bit.total_blk_size_ch4 = gn_xc_channel_block_size[3U];
            break;
        case XC_BLOCK_SIZE_3:
            _r1->reg._r2B.bit.total_blk_size_ch5 = gn_xc_channel_block_size[4U];
            _r1->reg._r2B.bit.total_blk_size_ch6 = gn_xc_channel_block_size[5U];
            break;
        case XC_BLOCK_SIZE_4:
            _r1->reg._r2C.bit.total_blk_size_ch7 = gn_xc_channel_block_size[6U];
            _r1->reg._r2C.bit.total_blk_size_ch8 = gn_xc_channel_block_size[7U];
            break;
        case XC_BLOCK_SIZE_5:
            _r1->reg._r2D.bit.total_blk_size_ch9 = gn_xc_channel_block_size[8U];
            _r1->reg._r2D.bit.total_blk_size_ch10 = gn_xc_channel_block_size[9U];
            break;
        case XC_BLOCK_SIZE_6:
            _r1->reg._r2E.bit.total_blk_size_ch11 = gn_xc_channel_block_size[10U];
            _r1->reg._r2E.bit.total_blk_size_ch12 = gn_xc_channel_block_size[11U];
            break;
        case XC_BLOCK_SIZE_7:
            _r1->reg._r2F.bit.total_blk_size_ch13 = gn_xc_channel_block_size[12U];
            _r1->reg._r2F.bit.total_blk_size_ch14 = gn_xc_channel_block_size[13U];
            break;
        case XC_BLOCK_SIZE_8:
            _r1->reg._r30.bit.total_blk_size_ch15 = gn_xc_channel_block_size[14U];
            _r1->reg._r30.bit.total_blk_size_ch16 = gn_xc_channel_block_size[15U];
            break;
        case XC_BLOCK_SIZE_9:
            _r1->reg._r31.bit.total_blk_size_ch17 = gn_xc_channel_block_size[16U];
            _r1->reg._r31.bit.total_blk_size_ch18 = gn_xc_channel_block_size[17U];
            break;
        case XC_BLOCK_SIZE_10:
            _r1->reg._r32.bit.total_blk_size_ch19 = gn_xc_channel_block_size[18U];
            _r1->reg._r32.bit.total_blk_size_ch20 = gn_xc_channel_block_size[19U];
            break;
        case XC_BLOCK_SIZE_11:
            _r1->reg._r33.bit.total_blk_size_ch21 = gn_xc_channel_block_size[20U];
            _r1->reg._r33.bit.total_blk_size_ch22 = gn_xc_channel_block_size[21U];
            break;
        case XC_BLOCK_SIZE_12:
            _r1->reg._r34.bit.total_blk_size_ch23 = gn_xc_channel_block_size[22U];
            _r1->reg._r34.bit.total_blk_size_ch24 = gn_xc_channel_block_size[23U];
            break;
        case XC_CHANNEL_ENABLE_1:
            _r1->reg._r35.bit.ch1_en = gn_xc_channel_enable[0U];
            _r1->reg._r35.bit.ch2_en = gn_xc_channel_enable[1U];
            _r1->reg._r35.bit.ch3_en = gn_xc_channel_enable[2U];
            _r1->reg._r35.bit.ch4_en = gn_xc_channel_enable[3U];
            _r1->reg._r35.bit.ch5_en = gn_xc_channel_enable[4U];
            _r1->reg._r35.bit.ch6_en = gn_xc_channel_enable[5U];
            _r1->reg._r35.bit.ch7_en = gn_xc_channel_enable[6U];
            _r1->reg._r35.bit.ch8_en = gn_xc_channel_enable[7U];
            _r1->reg._r35.bit.ch9_en = gn_xc_channel_enable[8U];
            _r1->reg._r35.bit.ch10_en = gn_xc_channel_enable[9U];
            _r1->reg._r35.bit.ch11_en = gn_xc_channel_enable[10U];
            _r1->reg._r35.bit.ch12_en = gn_xc_channel_enable[11U];
            _r1->reg._r35.bit.ch13_en = gn_xc_channel_enable[12U];
            _r1->reg._r35.bit.ch14_en = gn_xc_channel_enable[13U];
            _r1->reg._r35.bit.ch15_en = gn_xc_channel_enable[14U];
            _r1->reg._r35.bit.ch16_en = gn_xc_channel_enable[15U];
            break;
        case XC_CHANNEL_ENABLE_2:
            _r1->reg._r36.bit.ch17_en = gn_xc_channel_enable[16U];
            _r1->reg._r36.bit.ch18_en = gn_xc_channel_enable[17U];
            _r1->reg._r36.bit.ch19_en = gn_xc_channel_enable[18U];
            _r1->reg._r36.bit.ch20_en = gn_xc_channel_enable[19U];
            _r1->reg._r36.bit.ch21_en = gn_xc_channel_enable[20U];
            _r1->reg._r36.bit.ch22_en = gn_xc_channel_enable[21U];
            _r1->reg._r36.bit.ch23_en = gn_xc_channel_enable[22U];
            _r1->reg._r36.bit.ch24_en = gn_xc_channel_enable[23U];
            _r1->reg._r36.bit.ch_size = XC_CH_SIZE;
            _r1->reg._r36.bit.ld_width = XC_LD_WIDTH;
            break;
        case XC_FLLCNT11:
            _r1->reg._r37.bit.fll1cnt = ((gn_xc_fll_cnt[0] & FLL_BIT_B15_B0) >> FLL_BIT_SHIFT_LSB);
            break;
        case XC_FLLCNT12:
            _r1->reg._r38.bit.fll1cnt = ((gn_xc_fll_cnt[0] & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            _r1->reg._r38.bit.fll1_err_range = 0U;
            _r1->reg._r38.bit.fll1_range = 0U;
            _r1->reg._r38.bit.fllsync = XC_FLL_PAD_VSYNC;
            _r1->reg._r38.bit.fll1_en = XC_FUNCTION_EN;
            break;
        case XC_FLLCNT21:
            _r1->reg._r39.bit.fll2cnt = ((gn_xc_fll_cnt[1] & FLL_BIT_B15_B0) >> FLL_BIT_SHIFT_LSB);
            break;
        case XC_FLLCNT22:
            _r1->reg._r3A.bit.fll2cnt = ((gn_xc_fll_cnt[1] & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            _r1->reg._r3A.bit.fll2_err_range = 0U;
            _r1->reg._r3A.bit.fll2_range = 0U;
            _r1->reg._r3A.bit.fllsync = XC_FLL_PAD_VSYNC;
            _r1->reg._r3A.bit.fll2_en = XC_FUNCTION_EN;
            break;
        case XC_VO_DELAY:
            _r1->reg._r3B.bit.gate3_pol = 1U;
            _r1->reg._r3B.bit.gate2_pol = 1U;
            _r1->reg._r3B.bit.gate1_pol = 1U;
            _r1->reg._r3B.bit.vo_delay = 0U;
            break;
        case XC_VO_OFF_ON:
            _r1->reg._r3C.bit.vo_on = XC_CONV_US_TO_XC_MCLK(XC_V_MASK_ON_TIME_US);
            _r1->reg._r3C.bit.vo_off = XC_CONV_US_TO_XC_MCLK(XC_V_MASK_OFF_TIME_US);
            break;
        case XC_SVO_ON:
            _r1->reg._r3D.bit.svo_on = XC_CONV_US_TO_XC_MCLK(XC_SVO_ON_TIME_US);
            break;
        case XC_SVO1_OFF:
            //_r1->reg._r3E.bit.svo1_off = XC_CONV_US_TO_XC_MCLK(XC_SVO1_OFF_TIME_US);
            _r1->reg._r3E.bit.svo1_off = 0U;
            break;
        case XC_SVO2_OFF:
            _r1->reg._r3F.bit.svo2_off = XC_CONV_US_TO_XC_MCLK(XC_SVO2_OFF_TIME_US);
            break;
        case XC_SVO3_OFF:
            _r1->reg._r40.bit.svo3_off = XC_CONV_US_TO_XC_MCLK(XC_SVO3_OFF_TIME_US);
            break;
        case XC_SVO_NUMBER:
            _r1->reg._r41.bit.sv_no = XD_SV_NO;
            _r1->reg._r41.bit.sv_no_type = XC_SVO_ACTIVE_23;
            break;
        case XC_DAC_NF_CONTROL:
            //_r1->reg._r42.bit.dgrjt_en = 0U;
            //_r1->reg._r42.bit.bbkn_en = 0U;
            //_r1->reg._r42.bit.bbkn_th = 0U;
            //_r1->reg._r42.bit.dac_lvl = 0U;
            _r1->reg._r42.ALL = 0x0000U;
            break;
        case XC_DAC_CONTROL:
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
        case XC_DAC1_MIN_LIMIT:
            _r1->reg._r55.bit.dac1_min_limit = XC_CONV_DAC_V_TO_INPUT(0.3f);
            break;
        case XC_DAC1_MAX_LIMIT:
            _r1->reg._r56.bit.dac1_max_limit = XC_CONV_DAC_V_TO_INPUT(2.9f);
            break;
        case XC_DAC2_MIN_LIMIT:
            _r1->reg._r5C.bit.dac2_min_limit = XC_CONV_DAC_V_TO_INPUT(0.5f);
            break;
        case XC_DAC2_MAX_LIMIT:
            _r1->reg._r5D.bit.dac2_max_limit = XC_CONV_DAC_V_TO_INPUT(2.7f);
            break;
        case XC_DAC3_MIN_LIMIT:
            _r1->reg._r63.bit.dac3_min_limit = XC_CONV_DAC_V_TO_INPUT(0.7f);
            break;
        case XC_DAC3_MAX_LIMIT:
            _r1->reg._r64.bit.dac3_max_limit = XC_CONV_DAC_V_TO_INPUT(2.5f);
            break;
        case XC_OSC_FLL_MAN_A1:
            _r1->reg._r65.bit.OSC_MAN_EN_A = XC_FUNCTION_DIS;
            _r1->reg._r65.bit.FLT_GAIN_A = 2U;
            break;
        case XC_OSC_FLL_MAN_B1:
            _r1->reg._r67.bit.OSC_MAN_EN_B = XC_FUNCTION_DIS;
            _r1->reg._r67.bit.FLT_GAIN_B = 2U;
            break;
        default:
            continue;
        }
        xc24_write_grp1_reg(addr, &(_r1->reg._r00.ALL) + addr, 1U);
    }
#if 0
    _xc_group2_regs_t* _r2 = &gt_xc24_set_gr2_regs;
    for(xc_addr_grp2_t addr = XC_GRP2_DAC1_FB_VALID_CNT; addr < XC_GRP2_MAX; ++addr)
    {
        switch(addr)
        {
        default:
            continue;
        }
        xc24_write_grp2_reg(addr, &_r2->ALL[addr], 1U);
    }
#endif
    _xc_otp_control_regs_t* _rotp = &gt_xc24_set_otp_regs;
    for (xc_addr_otp_t addr = XC_TEST_CONTROL; addr < XC_OTP_MAX; ++addr)
    {
        switch (addr)
        {
            case XC_GATE_CONTROL:
                _rotp->reg._rFA.bit.gate1_H_en = XC_FUNCTION_EN;
                _rotp->reg._rFA.bit.gate1_L_en = XC_FUNCTION_DIS;
                _rotp->reg._rFA.bit.gate2_H_en = XC_FUNCTION_EN;
                _rotp->reg._rFA.bit.gate2_L_en = XC_FUNCTION_DIS;
                _rotp->reg._rFA.bit.gate3_H_en = XC_FUNCTION_EN;
                _rotp->reg._rFA.bit.gate3_L_en = XC_FUNCTION_DIS;
                break;
            case XC_GATE1_OFFSET:
                _rotp->reg._rFB.ALL = 0x0202U;
                break;
            case XC_GATE2_OFFSET:
                _rotp->reg._rFC.ALL = 0x0202U;
                break;
            case XC_GATE3_OFFSET:
                _rotp->reg._rFD.ALL = 0x0202U;
                break;
            case XC_SV_VAR_CONTROL1:
                _rotp->reg._rFE.ALL = 0x0000U;
                break;
            case XC_SV_VAR_CONTROL2:
                _rotp->reg._rFF.ALL = 0x0000U;
                break;
            default:
                continue;
        }
        xc24_write_otp_control(addr, &(_rotp->reg._rF0.ALL) + addr, 1U);
    }
}

static void xc24_regs_trim_init_table(void)
{
    _xc_group1_regs_t* _r1 = &gt_xc24_set_gr1_regs;
    for(xc_addr_grp1_t addr = XC_RESET; addr < XC_GRP1_MAX; ++addr)
    {
        switch(addr)
        {
            case XC_CLK_CONTROL_1:
            {
                _r1->reg._r1B.ALL = 0x0808U;
                break;
            }
            case XC_SERIALIZER_CLOCK_GEN:
            {
                _r1->reg._r1D.bit.serial_clk_high = XC_SERIAL_CLK_HIGH;
                _r1->reg._r1D.bit.serial_clk_low = XC_SERIAL_CLK_LOW;
                break;
            }
            default:
            {
                continue;
            }
        }
        xc24_write_grp1_reg(addr, &(_r1->reg._r00.ALL) + addr, 1U);
    }

    _xc_group2_regs_t* _r2 = &gt_xc24_set_gr2_regs;
    for(xc_addr_grp2_t addr = XC_GRP2_DAC1_FB_VALID_CNT; addr < XC_GRP2_MAX; ++addr)
    {
        switch(addr)
        {
            case XC_GRP2_DAC1_FB_VALID_CNT:
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
        xc24_write_grp2_reg(addr, &(_r2->reg._r00.ALL) + addr, 1U);
    }

    _xc_otp_control_regs_t* _rotp = &gt_xc24_set_otp_regs;
    for (xc_addr_otp_t addr = XC_TEST_CONTROL; addr < XC_OTP_MAX; ++addr)
    {
        switch (addr)
        {
            case XC_TEST_CONTROL:
                _rotp->reg._rF0.bit.TEST_EN = 1U;
                break;
            case XC_OTP_PROTECT:
                _rotp->reg._rF4.bit.PROTECT = 0x0A5AU;
                break;
            default:
                continue;
        }
        xc24_write_otp_control(addr, &(_rotp->reg._rF0.ALL) + addr, 1U);
    }
}

void xc24_reset(void)
{
    _v_reset_t _r00 = {0, };

    _r00.bit.rst1 = 1U;
    _r00.bit.rst2 = 0U;
    _r00.bit.rst3 = 0U;
    _r00.bit.vsync_rst_en1 = 0U;
    _r00.bit.vsync_rst_en2 = 0U;

    xc24_write_grp1_reg(XC_RESET, &_r00.ALL, 1U);
    us_delay(100U);
}

static void xc24_dump_registers(void)
{
#if 0
    comm_UART_Printf(LOG_LV_INFO, "\r\nXC24 GROUP1 Registers");
    for (xc_addr_grp1_t addr = XC_RESET; addr < XC_GRP1_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%04X", addr, gt_xc24_get_gr1_regs.ALL[addr]);
    }

    comm_UART_Printf(LOG_LV_INFO, "\r\nXC24 GROUP2 Registers");
    for (xc_addr_grp2_t addr = XC_GRP2_DAC1_FB_VALID_CNT; addr < XC_GRP2_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%04X", addr, gt_xc24_get_gr2_regs.ALL[addr]);
    }

    comm_UART_Printf(LOG_LV_INFO, "\r\nXC24 OTP Control Registers");
    for (xc_addr_otp_t addr = XC_TEST_CONTROL; addr < XC_OTP_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%04X", (XC_OTP_BASE_ADDR + addr), gt_xc24_get_otp_regs.ALL[addr]);
    }
#else
    char line_buf[128] = { 0 };
    int len = 0;

    // 1. XC24 GROUP1 Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n=== XC24 GROUP1 Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    for (uint16_t addr = 0; addr < (uint16_t)XC_GRP1_MAX; ++addr)
    {
        if ((addr % 16) == 0)
        {
            if (addr > 0)
            {
                comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
            }
            len = snprintf(line_buf, sizeof(line_buf), "\r\n0x%02X |", addr);
        }
        len += snprintf(&line_buf[len], sizeof(line_buf) - len, " %04X ", gt_xc24_get_gr1_regs.ALL[addr]);
    }
    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    // 2. XC24 GROUP2 Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n\r\n=== XC24 GROUP2 Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    len = 0;
    for (uint16_t addr = 0; addr < (uint16_t)XC_GRP2_MAX; ++addr)
    {
        if ((addr % 16) == 0)
        {
            if (addr > 0)
            {
                comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
            }
            len = snprintf(line_buf, sizeof(line_buf), "\r\n0x%02X |", addr);
        }
        len += snprintf(&line_buf[len], sizeof(line_buf) - len, " %04X ", gt_xc24_get_gr2_regs.ALL[addr]);
    }
    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    // 3. XC24 OTP Control Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n\r\n=== XC24 OTP Control Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    len = 0;
    for (uint16_t offset = 0; offset < (uint16_t)XC_OTP_MAX; ++offset)
    {
        uint16_t real_addr = (uint16_t)XC_OTP_BASE_ADDR + offset;
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

        len += snprintf(&line_buf[len], sizeof(line_buf) - len, " %04X ", gt_xc24_get_otp_regs.ALL[offset]);
    }
    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");
    comm_UART_Printf(LOG_LV_INFO, "\r\n");
#endif
}

static void xc24_memory_copy(void)
{
    memcpy(gt_xc24_set_gr1_regs.ALL, gt_xc24_get_gr1_regs.ALL, sizeof(gt_xc24_get_gr1_regs));
    memcpy(gt_xc24_set_gr2_regs.ALL, gt_xc24_get_gr2_regs.ALL, sizeof(gt_xc24_get_gr2_regs));
    memcpy(gt_xc24_set_otp_regs.ALL, gt_xc24_get_otp_regs.ALL, sizeof(gt_xc24_get_otp_regs));
}

void xc24_read_all(void)
{
    xc24_read_grp1_reg(XC_RESET +  0U, 56U); // 0x00 ~ 0x37, 56EA
    xc24_read_grp1_reg(XC_RESET + 56U, 56U); // 0x38 ~ 0x6F, 56EA

    xc24_read_grp2_reg(XC_GRP2_DAC1_FB_VALID_CNT, 19U); // 0x00 ~ 0x12, 19EA

    xc24_read_otp_control(XC_TEST_CONTROL, 16U);

    xc24_dump_registers();
    xc24_memory_copy();
}

void xc24_init_param(void)
{
    gn_xc_daisied_dev_blk_size = BLOCK_PER_XD;

    /* XC24R channel enable */
    for (uint8_t xc_ch = 0; xc_ch < XC_CH_SIZE_MAX; ++xc_ch)
    {
#if 1 //Sequential Enable
        if (xc_ch < XC_CH_SIZE)
        {
            gn_xc_channel_enable[xc_ch] = 1U;
            gn_xc_channel_daisy_size[xc_ch] = XD_DAISY_LENGTH;
            gn_xc_channel_block_size[xc_ch] = (gn_xc_channel_daisy_size[xc_ch] * gn_xc_daisied_dev_blk_size);
        }
        else
        {
            gn_xc_channel_enable[xc_ch] = 0U;
            gn_xc_channel_daisy_size[xc_ch] = 0U;
            gn_xc_channel_block_size[xc_ch] = 0U;
        }
#else //Selective Enable
        if (xc_ch == XC_CH_02 || xc_ch == XC_CH_17) // ch3, ch17
        {
            gn_xc_channel_enable[xc_ch] = 1U;
            gn_xc_channel_daisy_size[xc_ch] = XD_DAISY_LENGTH;
            gn_xc_channel_block_size[xc_ch] = (gn_xc_channel_daisy_size[xc_ch] * gn_xc_daisied_dev_blk_size);
        }
        else
        {
            gn_xc_channel_enable[xc_ch] = 0U;
            gn_xc_channel_daisy_size[xc_ch] = 0U;
            gn_xc_channel_block_size[xc_ch] = 0U;
        }
#endif
    }

    gn_xc_fll_cnt[0] = XC_CONV_FREQ_TO_XC_MCLK(TIM4_CLK);
    gn_xc_fll_cnt[1] = XC_CONV_FREQ_TO_XC_MCLK(TIM4_CLK);
}

void xc24_init(void)
{
    XC_NSS_HI();
    xc24_reset();
    xc24_regs_init_table();
    xc24_read_all();
}

void xc24_trim_init(void)
{
    XC_NSS_HI();
    xc24_reset();
    xc24_regs_trim_init_table();
    xc24_read_all();
}

const _xc_group1_regs_t* xc24_get_xc24_set_gr1_regs(void)
{
    return &gt_xc24_set_gr1_regs;
}
const _xc_group1_regs_t* xc24_get_xc24_get_gr1_regs(void)
{
    return &gt_xc24_get_gr1_regs;
}
const _xc_group2_regs_t* xc24_get_xc24_set_gr2_regs(void)
{
    return &gt_xc24_set_gr2_regs;
}
const _xc_group2_regs_t* xc24_get_xc24_get_gr2_regs(void)
{
    return &gt_xc24_get_gr2_regs;
}

static void xc24_change_rw_grp_type(xc_rw_grp_t in_grp)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    xc_rw_grp_t now_grp = (xc_rw_grp_t)(_rF0->bit.ADDR_EXT);

    if (in_grp >= XC_RW_GRP_MAX)
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
        cmd.bit.addr = (XC_OTP_BASE_ADDR + XC_TEST_CONTROL);
        cmd.bit.size = 1U;

        tx_buffer[0] = cmd.ALL;
        tx_buffer[1] = _rF0->ALL;

        XC_NSS_LO();
        uint8_t ret = spi_write(SPI1, tx_buffer, 2, 20U);
        XC_NSS_HI();

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
        xc24_spi_log_dump(tx_buffer, 2);
#endif
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nChange GRP TYPE to %u [0x%04X]", in_grp, gt_xc24_set_otp_regs.reg._rF0.ALL);

        if(SPI_TIMEOUT == ret)
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
        }
    }
}

uint16_t xc24_read_otp_control(uint16_t addr, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t rx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if (addr > XC_OTP_MAX)
    {
        FATAL_INVALID_INPUT(addr);
        return 0U;
    }

    if((addr + length) >= XC_OTP_MAX)
    {
        burst_size = (XC_OTP_MAX - addr);
    }
    if(burst_size > 63U)
    {
        burst_size = 63U;
    }

    uint16_t spi_len = 1U + burst_size; /* HDR + readout */

    cmd.bit.code = CMD_CODE1;
    cmd.bit.addr = (XC_OTP_BASE_ADDR + addr);
    cmd.bit.size = burst_size;

    tx_buffer[0U] = cmd.ALL;

    if (addr != XC_TEST_CONTROL)
    {
        xc24_change_rw_grp_type(XC_RW_GRP1);
    }

    XC_NSS_LO();
    uint8_t ret = spi_read(SPI1, tx_buffer, rx_buffer, spi_len, 20U);
    XC_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi read timeout");
    }
    else
    {
        _xc_otp_control_regs_t* _r = &gt_xc24_get_otp_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = rx_buffer[XC_SPI_HEADER_SIZE + i];
        }
    }
    return rx_buffer[XC_SPI_HEADER_SIZE];
}

void xc24_write_otp_control(uint16_t addr, const uint16_t* q, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if (addr > XC_OTP_MAX)
    {
        FATAL_INVALID_INPUT(addr);
        return;
    }

    if((addr + length) >= XC_OTP_MAX)
    {
        burst_size = (XC_OTP_MAX - addr);
    }
    if(burst_size > 63U)
    {
        burst_size = 63U;
    }

    uint16_t spi_len = 0U;

    cmd.bit.code = CMD_CODE2;
    cmd.bit.addr = (XC_OTP_BASE_ADDR + addr);
    cmd.bit.size = burst_size;

    tx_buffer[spi_len++] = cmd.ALL;

    for(uint16_t i = 0U; i < burst_size; ++i)
    {
        tx_buffer[spi_len++] = *q++;
    }

    if (addr != XC_TEST_CONTROL)
    {
        xc24_change_rw_grp_type(XC_RW_GRP1);
    }

    XC_NSS_LO();
    uint8_t ret = spi_write(SPI1, tx_buffer, spi_len, 20U);
    XC_NSS_HI();

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
    xc24_spi_log_dump(tx_buffer, spi_len);
#endif

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xc_otp_control_regs_t* _r = &gt_xc24_set_otp_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = tx_buffer[XC_SPI_HEADER_SIZE + i];
        }
    }
}

uint16_t xc24_read_grp1_reg(uint16_t addr, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t rx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if (addr > XC_GRP1_MAX)
    {
        FATAL_INVALID_INPUT(addr);
        return 0U;
    }

    if((addr + length) >= XC_GRP1_MAX)
    {
        burst_size = (XC_GRP1_MAX - addr);
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

    xc24_change_rw_grp_type(XC_RW_GRP1);

    XC_NSS_LO();
    uint8_t ret = spi_read(SPI1, tx_buffer, rx_buffer, spi_len, 20U);  /* hdr + crc16 + payload + crc16 */
    XC_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi read timeout");
    }
    else
    {
        _xc_group1_regs_t* _r = &gt_xc24_get_gr1_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = rx_buffer[XC_SPI_HEADER_SIZE + i];
        }
    }
    return rx_buffer[XC_SPI_HEADER_SIZE];
}

void xc24_write_grp1_reg(uint16_t addr, const uint16_t* q, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if (addr > XC_GRP1_MAX)
    {
        FATAL_INVALID_INPUT(addr);
        return;
    }

    if((addr + length) >= XC_GRP1_MAX)
    {
        burst_size = (XC_GRP1_MAX - addr);
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

    xc24_change_rw_grp_type(XC_RW_GRP1);

    XC_NSS_LO();
    uint8_t ret = spi_write(SPI1, tx_buffer, spi_len, 20U);
    XC_NSS_HI();

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
    xc24_spi_log_dump(tx_buffer, spi_len);
#endif

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xc_group1_regs_t* _r = &gt_xc24_set_gr1_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = tx_buffer[XC_SPI_HEADER_SIZE + i];
        }
    }
}

uint16_t xc24_read_grp2_reg(uint16_t addr, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t rx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if (addr > XC_GRP2_MAX)
    {
        FATAL_INVALID_INPUT(addr);
        return 0U;
    }

    if((addr + length) >= XC_GRP2_MAX)
    {
        burst_size = (XC_GRP2_MAX - addr);
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

    xc24_change_rw_grp_type(XC_RW_GRP2);

    XC_NSS_LO();
    uint8_t ret = spi_read(SPI1, tx_buffer, rx_buffer, spi_len, 20U);  /* hdr + payload + crc16 */
    XC_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi read timeout");
    }
    else
    {
        _xc_group2_regs_t* _r = &gt_xc24_get_gr2_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = rx_buffer[XC_SPI_HEADER_SIZE + i];
        }
    }
    return rx_buffer[XC_SPI_HEADER_SIZE];
}

void xc24_write_grp2_reg(uint16_t addr, const uint16_t* q, uint16_t length)
{
    _cmd_t cmd = { 0U };
    uint16_t tx_buffer[XC_SPI_BUFF_MAX_SIZE] = { 0U };
    uint16_t burst_size = length;

    if (addr > XC_GRP2_MAX)
    {
        FATAL_INVALID_INPUT(addr);
        return;
    }

    if((addr + length) >= XC_GRP2_MAX)
    {
        burst_size = (XC_GRP2_MAX - addr);
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

    xc24_change_rw_grp_type(XC_RW_GRP2);

    XC_NSS_LO();
    uint8_t ret = spi_write(SPI1, tx_buffer, spi_len, 20U);
    XC_NSS_HI();

#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
    xc24_spi_log_dump(tx_buffer, spi_len);
#endif

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xc_group2_regs_t* _r = &gt_xc24_set_gr2_regs;
        for(uint16_t i = 0U; i < burst_size; ++i)
        {
            _r->ALL[addr + i] = tx_buffer[1U + i];
        }
    }
}

void xc24_set_ld_transfer(uint16_t* buffer, uint16_t length)
{
#if (SPI_LOG_DUMP == SPI_LOG_DUMP_ENABLE)
    static bool is_first = true;
    if (is_first)
    {
        xc24_spi_log_dump(buffer, length);
        is_first = false;
    }
#endif

    if((NULL == buffer) || (0U == length))
    {
        return;
    }

    gb_xc_ld_transfer_spi_dma_flag = true;

    XC_NSS_LO();

    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_3, (uint32_t)buffer);

    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_3, length);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_3);
}

bool xc24_read_local(uint16_t ch_seg, uint16_t addr)
{
    uint16_t retry = 0U;

    _v_local_write_command_t _r03 = {0U, };
    _v_local_rw_pointer_reset_t _r10 = {0U, };
    _v_command_status_1_t* _r14 = &gt_xc24_get_gr1_regs.reg._r14;
    _v_receive_status_t* _r16 = &gt_xc24_get_gr1_regs.reg._r16;

    _r10.bit.local_transfer_pointer_rst = 1U;
    _r10.bit.local_receive_pointer_rst = 1U;
    xc24_write_grp1_reg(XC_LOCAL_RW_POINTER_RESET, &_r10.ALL, 1U);

    _r03.bit.addr = addr;
    _r03.bit.ch_seg = ch_seg;
    _r03.bit.enable = 1U;
    xc24_write_grp1_reg(XC_LOCAL_READ_COMMAND, &_r03.ALL, 1U);

    retry = 128U;
    do
    {
        //us_delay(1000); /* TODO : ???*/
        xc24_read_grp1_reg(XC_COMMAND_STATUS_1, 1U);
    }while((_r14->bit.local_r_doing == 1U) && --retry > 0U);

    retry = 128U;
    do
    {
        //us_delay(1000); /* TODO : ???*/
        xc24_read_grp1_reg(XC_RECEIVE_STATUS, 1U);
    }while((_r16->bit.rd_receive_doing == 1U) && --retry > 0U);

    return ((_r16->bit.rd_receive_done == 1U) && (_r16->bit.rd_receive_fail == 0U));
}

void xc24_write_local(uint16_t ch_seg, uint16_t addr, uint16_t* data, uint16_t len)
{
    if(0U < len)
    {
        uint16_t retry = 32U;
        uint16_t offset = 0U;

        _v_local_write_command_t _r02 = {0U, };
        _v_local_rw_pointer_reset_t _r10 = {0U, };
        _v_command_status_1_t* _r14 = &gt_xc24_get_gr1_regs.reg._r14;

        _r10.bit.local_transfer_pointer_rst = 1U;
        _r10.bit.local_receive_pointer_rst = 1U;
        xc24_write_grp1_reg(XC_LOCAL_RW_POINTER_RESET, &_r10.ALL, 1U);

        while(len > 0U)
        {
            uint16_t local_rw_len = (len > XC_SPI_RW_LEN) ? XC_SPI_RW_LEN : len;

            xc24_set_local_rw_data(XC_PORT1_LOCAL_RW_DATA1 + offset, data + offset, local_rw_len);

            offset += local_rw_len;
            len -= local_rw_len;
        }

        _r02.bit.addr = addr;
        _r02.bit.ch_seg = ch_seg;
        _r02.bit.enable = 1;
        xc24_write_grp1_reg(XC_LOCAL_WRITE_COMMAND, &_r02.ALL, 1U);

        do
        {
            //us_delay(1000); /* TODO : ???*/
            xc24_read_grp1_reg(XC_COMMAND_STATUS_1, 1U);
        }while((_r14->bit.local_r_doing == 1U) && --retry > 0U);
    }
}

void xc24_get_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len)
{
    uint16_t spi_buffer[XC_SPI_RW_LEN + 1 + 1] = { 0U, };
    uint16_t temp[XC_SPI_RW_LEN + 1 + 1] = { 0U, };

    _cmd_t _cmd = { 0U, };

    if(len > XC_SPI_RW_LEN)
    {
        len = XC_SPI_RW_LEN;
    }

    _cmd.bit.code = CMD_CODE1;
    _cmd.bit.addr = addr;
    _cmd.bit.size = (len - 0U);
    spi_buffer[0U] = _cmd.ALL;

    uint16_t crc16 = Calculate_CRC16_CCITT_False(spi_buffer, 1U);

    temp[1] = crc16;

    xc24_change_rw_grp_type(XC_RW_GRP1);

    XC_NSS_LO();
    uint8_t ret = spi_read(SPI1, spi_buffer, temp, (len + 1U + 1U), 20U);  /* hdr + crc16 + payload + crc16 */
    XC_NSS_HI();

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

void xc24_set_local_rw_data(uint16_t addr, uint16_t* p_data, uint16_t len)
{
    uint16_t spi_buffer[XC_SPI_RW_LEN + 1U + 1U] = { 0U, };

    _cmd_t _cmd = { 0U, };

    if(len > XC_SPI_RW_LEN)
    {
        len = XC_SPI_RW_LEN;
    }

    _cmd.bit.code = CMD_CODE2;
    _cmd.bit.addr = addr;
    _cmd.bit.size = (len - 0U);

    spi_buffer[0] = _cmd.ALL;
    memcpy(&spi_buffer[1], p_data, (len * sizeof(uint16_t)));

    uint16_t crc16 = Calculate_CRC16_CCITT_False(spi_buffer, len + 1U);

    spi_buffer[1U + len] = crc16;

    XC_NSS_LO();
    uint8_t ret = spi_write(SPI1, spi_buffer, len + 1U + 1U, 20U);
    XC_NSS_HI();
}

void xc24_set_fll_cnt(uint8_t fll_ch, uint32_t fll_cnt)
{
    if (fll_ch < 3U)
    {
        if (fll_ch == 0U)
        {
            _v_fllcnt11_t* _r37 = &gt_xc24_set_gr1_regs.reg._r37;
            _r37->bit.fll1cnt = ((fll_cnt & FLL_BIT_B15_B0) >>  FLL_BIT_SHIFT_LSB);
            xc24_write_grp1_reg(XC_FLLCNT11, &_r37->ALL, 1U);

            _v_fllcnt12_t* _r38 = &gt_xc24_set_gr1_regs.reg._r38;
            _r38->bit.fll1cnt = ((fll_cnt & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            xc24_write_grp1_reg(XC_FLLCNT12, &_r38->ALL, 1U);

            _v_fllcnt21_t* _r39 = &gt_xc24_set_gr1_regs.reg._r39;
            _r39->bit.fll2cnt = ((fll_cnt & FLL_BIT_B15_B0) >>  FLL_BIT_SHIFT_LSB);
            xc24_write_grp1_reg(XC_FLLCNT21, &_r39->ALL, 1U);

            _v_fllcnt22_t* _r3A = &gt_xc24_set_gr1_regs.reg._r3A;
            _r3A->bit.fll2cnt = ((fll_cnt & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            xc24_write_grp1_reg(XC_FLLCNT22, &_r3A->ALL, 1U);
        }
        else if (fll_ch == 1U)
        {
            _v_fllcnt11_t* _r37 = &gt_xc24_set_gr1_regs.reg._r37;
            _r37->bit.fll1cnt = ((fll_cnt & FLL_BIT_B15_B0) >>  FLL_BIT_SHIFT_LSB);
            xc24_write_grp1_reg(XC_FLLCNT11, &_r37->ALL, 1U);

            _v_fllcnt12_t* _r38 = &gt_xc24_set_gr1_regs.reg._r38;
            _r38->bit.fll1cnt = ((fll_cnt & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            xc24_write_grp1_reg(XC_FLLCNT12, &_r38->ALL, 1U);
        }
        else if (fll_ch == 2U)
        {
            _v_fllcnt21_t* _r39 = &gt_xc24_set_gr1_regs.reg._r39;
            _r39->bit.fll2cnt = ((fll_cnt & FLL_BIT_B15_B0) >>  FLL_BIT_SHIFT_LSB);
            xc24_write_grp1_reg(XC_FLLCNT21, &_r39->ALL, 1U);

            _v_fllcnt22_t* _r3A = &gt_xc24_set_gr1_regs.reg._r3A;
            _r3A->bit.fll2cnt = ((fll_cnt & FLL_BIT_B20_B16) >> FLL_BIT_SHIFT_MSB);
            xc24_write_grp1_reg(XC_FLLCNT22, &_r3A->ALL, 1U);
        }
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nChange FLL[ch:%d] count to %d", fll_ch, fll_cnt);
    }
    else
    {
        FATAL_INVALID_INPUT(fll_ch);
    }
}

void xc24_nINT_FT_handler(void)
{
    static uint16_t prev_cause_of_INT = 0xFFFFU;
    static uint16_t duplicate_vsync_cnt = 0U;

    uint16_t cause_of_INT = xc24_read_grp1_reg(XC_INTERRUPT_STATUS, 1U);

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
        comm_UART_Printf(LOG_LV_INFO, "\r\nXC24 nINT_FT interrupt 0x%02X: 0x%04X", XC_INTERRUPT_STATUS, cause_of_INT);

        for (uint8_t i = 0U; i < sizeof(int_flags) / sizeof(int_flags[0]); ++i)
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
        if ((++duplicate_vsync_cnt) == 240U) // Log every 240 duplicates to avoid flooding the log
        {
            comm_UART_Printf(LOG_LV_INFO, "\r\nXC24 nINT_FT interrupt 0x%02X: 0x%04X (duplicate %d times)", XC_INTERRUPT_STATUS, cause_of_INT, duplicate_vsync_cnt);
            duplicate_vsync_cnt = 0U;
        }
    }
}

void xc24_nINT_LD_handler(void)
{
    comm_UART_Printf(LOG_LV_INFO, "\r\nXC24 nINT_LD interrupt");
}

void xc24_trim_set_efuse_enable(bool en)
{
    gb_xc_do_efuse = en;
}

bool xc24_trim_get_efuse_enable(void)
{
    return gb_xc_do_efuse;
}

void xc24_trim_init_1v5_ldo_dig(void)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);
}

void xc24_trim_init_dac_3v0(void)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_current_target_dac1_t* _r44 = &gt_xc24_set_gr1_regs.reg._r44;
    _r44->bit.curr_tgt_dac1 = 0xFFF;
    xc24_write_grp1_reg(XC_CURRENT_TARGET_DAC1, &_r44->ALL, 1U);
}

void xc24_trim_init_dac1_ofs(void)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_current_target_dac1_t* _r44 = &gt_xc24_set_gr1_regs.reg._r44;
    _r44->bit.curr_tgt_dac1 = XC_TRIM_DAC_INPUT;
    xc24_write_grp1_reg(XC_CURRENT_TARGET_DAC1, &_r44->ALL, 1U);

    _v_dac_nf_control_t* _r42 = &gt_xc24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xc24_write_grp1_reg(XC_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xc24_trim_init_dac2_ofs(void)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_current_target_dac2_t* _r45 = &gt_xc24_set_gr1_regs.reg._r45;
    _r45->bit.curr_tgt_dac2 = XC_TRIM_DAC_INPUT;
    xc24_write_grp1_reg(XC_CURRENT_TARGET_DAC2, &_r45->ALL, 1U);

    _v_dac_nf_control_t* _r42 = &gt_xc24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xc24_write_grp1_reg(XC_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xc24_trim_init_dac3_ofs(void)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_current_target_dac3_t* _r46 = &gt_xc24_set_gr1_regs.reg._r46;
    _r46->bit.curr_tgt_dac3 = XC_TRIM_DAC_INPUT;
    xc24_write_grp1_reg(XC_CURRENT_TARGET_DAC3, &_r46->ALL, 1U);

    _v_dac_nf_control_t* _r42 = &gt_xc24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xc24_write_grp1_reg(XC_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xc24_trim_init_1v5_ldo_osc(void)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_ana_test_t* _r12 = &gt_xc24_set_gr2_regs.reg._r12;
    _r12->bit.TEST_ANA_EN = 6U;
    xc24_write_grp2_reg(XC_GRP2_ANA_TEST, &_r12->ALL, 1U);
}

void xc24_trim_init_osc_a(void)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;
    _rF0->bit.MCLK_SEL = XC_MCLK_SEL_OSC_A;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_a1_t* _r65 = &gt_xc24_set_gr1_regs.reg._r65;
    _r65->bit.OSC_MAN_EN_A = 1U;
    _r65->bit.FLT_CTL_A = 1U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_A1, &_r65->ALL, 1U);

    _v_osc_fll_man_a2_t* _r66 = &gt_xc24_set_gr1_regs.reg._r66;
    _r66->bit.OSC_FLL_MAN_A = 0x8000U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_A2, &_r66->ALL, 1U);
}

void xc24_trim_init_osc_b(void)
{
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;
    _rF0->bit.MCLK_SEL = XC_MCLK_SEL_OSC_B;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xc24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 1U;
    _r67->bit.FLT_CTL_B = 1U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_B1, &_r67->ALL, 1U);

    _v_osc_fll_man_b2_t* _r68 = &gt_xc24_set_gr1_regs.reg._r68;
    _r68->bit.OSC_FLL_MAN_B = 0x8000U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_B2, &_r68->ALL, 1U);
}

bool xc24_trim_set_1v5_ldo_dig(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XC_MAX_1V5_LDO_DIG)
    {
        _v_mirror1_t* _rF5 = &gt_xc24_set_otp_regs.reg._rF5;
        _rF5->bit.vctl_ldo = reg_val;
        xc24_write_otp_control(XC_MIRROR1, &_rF5->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xc24_trim_set_dac_3v0(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XC_MAX_DAC_3V0)
    {
        _v_mirror2_t* _rF6 = &gt_xc24_set_otp_regs.reg._rF6;
        _rF6->bit.dac_ctl = reg_val;
        xc24_write_otp_control(XC_MIRROR2, &_rF6->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xc24_trim_set_dac1_ofs(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XC_MAX_DAC1_OFS)
    {
        _v_mirror2_t* _rF6 = &gt_xc24_set_otp_regs.reg._rF6;
        _rF6->bit.dac1_ofs = reg_val;
        xc24_write_otp_control(XC_MIRROR2, &_rF6->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xc24_trim_set_dac2_ofs(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XC_MAX_DAC2_OFS)
    {
        _v_mirror3_t* _rF7 = &gt_xc24_set_otp_regs.reg._rF7;
        _rF7->bit.dac2_ofs = reg_val;
        xc24_write_otp_control(XC_MIRROR3, &_rF7->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xc24_trim_set_dac3_ofs(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XC_MAX_DAC3_OFS)
    {
        _v_mirror3_t* _rF7 = &gt_xc24_set_otp_regs.reg._rF7;
        _rF7->bit.dac3_ofs = reg_val;
        xc24_write_otp_control(XC_MIRROR3, &_rF7->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xc24_trim_set_1v5_ldo_osc(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XC_MAX_1V5_LDO_OSC)
    {
        _v_mirror4_t* _rF8 = &gt_xc24_set_otp_regs.reg._rF8;
        _rF8->bit.ldo_osc_ctl = reg_val;
        xc24_write_otp_control(XC_MIRROR4, &_rF8->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xc24_trim_set_osc_a(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XC_MAX_OSC_A)
    {
        _v_mirror4_t* _rF8 = &gt_xc24_set_otp_regs.reg._rF8;
        _rF8->bit.osc_rctl = reg_val;
        xc24_write_otp_control(XC_MIRROR4, &_rF8->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

bool xc24_trim_set_osc_b(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XC_MAX_OSC_B)
    {
        _v_mirror5_t* _rF9 = &gt_xc24_set_otp_regs.reg._rF9;
        _rF9->bit.osc_rctl2 = reg_val;
        xc24_write_otp_control(XC_MIRROR5, &_rF9->ALL, 1U);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }
    return ret;
}

void xc24_trim_init_efuse(void)
{
    _v_otp_pg_access_t* _rF1 = &gt_xc24_set_otp_regs.reg._rF1;
    _rF1->bit.OTP_PG_ACC_CYCLE = 0x03FFU;
    xc24_write_otp_control(XC_OTP_PG_ACCESS, &_rF1->ALL, 1U);

    _v_otp_write_t* _rF2 = &gt_xc24_set_otp_regs.reg._rF2;
    _rF2->bit.OTP_WSEL = 0x04U;
    _rF2->bit.OTP_RD = 0U;
    _rF2->bit.OTP_PG_DONE = 0U;
    xc24_write_otp_control(XC_OTP_WRITE, &_rF2->ALL, 1U);
}

void xc24_trim_start_efuse(void)
{
    _v_otp_rd_prog_t* _rF3 = &gt_xc24_set_otp_regs.reg._rF3;
    _rF3->bit.OTP_PG_S = 1U;
    xc24_write_otp_control(XC_OTP_RD_PROG, &_rF3->ALL, 1U);
}

void xc24_trim_save_mirror_register(void)
{
    xc24_read_otp_control(XC_MIRROR1, 5U);
}

uint32_t xc24_trim_verify_mirror_dump(void)
{
    uint32_t ret = 0U;
    for (xc_addr_otp_t mirror_addr = XC_MIRROR1; mirror_addr < XC_GATE_CONTROL; ++mirror_addr) // 0xF5 ~ 0xF9
    {
        uint16_t saved_reg = gt_xc24_get_otp_regs.ALL[mirror_addr];
        uint16_t read_reg = xc24_read_otp_control(mirror_addr, 1U);

        if (mirror_addr == XC_MIRROR5)
        {
            saved_reg &= 0x1FU; // masking low 5 bits for mirror5 register, as only 5 bits are valid for this register
            read_reg &= 0x1FU; // masking low 5 bits for mirror5 register, as only 5 bits are valid for this register
        }
        if (saved_reg != read_reg)
        {
            ret |= (1UL << mirror_addr);
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✕]%s ADDR [0x%02X] - [0x%04X - 0x%04X]", \
                ANSI_FONT_RED, ANSI_FONT_NONE, (XC_OTP_BASE_ADDR + mirror_addr), saved_reg, read_reg);
        }
        else
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✔]%s ADDR [0x%02X] - [0x%04X - 0x%04X]", \
                ANSI_FONT_GREEN, ANSI_FONT_NONE, (XC_OTP_BASE_ADDR + mirror_addr), saved_reg, read_reg);
        }
    }
    return ret;
}

void xc24_test_init_icc_stby(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_ICC_P, ADS114S08_CH_XC_ICC_N);
}

void xc24_test_init_icc_actv(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_ICC_P, ADS114S08_CH_XC_ICC_N);
    // set proper xc24 register
    xc24_trim_init();
}

void xc24_test_init_ldo(void)
{
    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_LDO, ADS_AINCOM);
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);
}

void xc24_test_init_ldo_fll_a(void)
{
    // turn on proper power if needed like VLE

    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_1V5, ADS_AINCOM);
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_ana_test_t* _r12 = &gt_xc24_set_gr2_regs.reg._r12;
    _r12->bit.TEST_ANA_EN = 6U;
    xc24_write_grp2_reg(XC_GRP2_ANA_TEST, &_r12->ALL, 1U);
}

void xc24_test_init_ldo_fll_b(void)
{
    // turn on proper power if needed like VLE

    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_1V5, ADS_AINCOM);
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_ana_test_t* _r12 = &gt_xc24_set_gr2_regs.reg._r12;
    _r12->bit.TEST_ANA_EN = 5U;
    xc24_write_grp2_reg(XC_GRP2_ANA_TEST, &_r12->ALL, 1U);
}

void xc24_test_init_fll_a_30m(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;
    _rF0->bit.MCLK_SEL = XC_MCLK_SEL_OSC_A;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_a1_t* _r65 = &gt_xc24_set_gr1_regs.reg._r65;
    _r65->bit.OSC_MAN_EN_A = 0U;
    _r65->bit.FLT_CTL_A = 1U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_A1, &_r65->ALL, 1U);

    const float xc_mclk = 30000000.0f; /* 30MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xc24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xc24_write_grp1_reg(XC_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xc24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    //_r38->bit.fllsync = XC_FLL_PAD_VSYNC;
    //_r38->bit.fll1_en = 1U;
    xc24_write_grp1_reg(XC_FLLCNT12, &_r38->ALL, 1U);
}

void xc24_test_init_fll_a_35m(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;
    _rF0->bit.MCLK_SEL = XC_MCLK_SEL_OSC_A;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_a1_t* _r65 = &gt_xc24_set_gr1_regs.reg._r65;
    _r65->bit.OSC_MAN_EN_A = 0U;
    _r65->bit.FLT_CTL_A = 1U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_A1, &_r65->ALL, 1U);

    const float xc_mclk = 35000000.0f; /* 35MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xc24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xc24_write_grp1_reg(XC_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xc24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    //_r38->bit.fllsync = XC_FLL_PAD_VSYNC;
    //_r38->bit.fll1_en = 1U;
    xc24_write_grp1_reg(XC_FLLCNT12, &_r38->ALL, 1U);
}

void xc24_test_init_fll_a_40m(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;
    _rF0->bit.MCLK_SEL = XC_MCLK_SEL_OSC_A;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_a1_t* _r65 = &gt_xc24_set_gr1_regs.reg._r65;
    _r65->bit.OSC_MAN_EN_A = 0U;
    _r65->bit.FLT_CTL_A = 1U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_A1, &_r65->ALL, 1U);

    const float xc_mclk = 40000000.0f; /* 40MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt11_t* _r37 = &gt_xc24_set_gr1_regs.reg._r37;
    _r37->bit.fll1cnt = ((fll_out & 0x00FFFF) >>  0U);
    xc24_write_grp1_reg(XC_FLLCNT11, &_r37->ALL, 1U);

    _v_fllcnt12_t* _r38 = &gt_xc24_set_gr1_regs.reg._r38;
    _r38->bit.fll1cnt = ((fll_out & 0x1F0000) >> 16U);
    //_r38->bit.fllsync = XC_FLL_PAD_VSYNC;
    //_r38->bit.fll1_en = 1U;
    xc24_write_grp1_reg(XC_FLLCNT12, &_r38->ALL, 1U);
}

void xc24_test_init_fll_b_30m(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;
    _rF0->bit.MCLK_SEL = XC_MCLK_SEL_OSC_B;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xc24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_B1, &_r67->ALL, 1U);

    const float xc_mclk = 30000000.0f; /* 30MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt21_t* _r39 = &gt_xc24_set_gr1_regs.reg._r39;
    _r39->bit.fll2cnt = ((fll_out & 0x00FFFF) >>  0U);
    xc24_write_grp1_reg(XC_FLLCNT21, &_r39->ALL, 1U);

    _v_fllcnt22_t* _r3A = &gt_xc24_set_gr1_regs.reg._r3A;
    _r3A->bit.fll2cnt = ((fll_out & 0x1F0000) >> 16U);
    //_r3A->bit.fllsync = XC_FLL_PAD_VSYNC;
    //_r3A->bit.fll2_en = 1U;
    xc24_write_grp1_reg(XC_FLLCNT22, &_r3A->ALL, 1U);
}

void xc24_test_init_fll_b_35m(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;
    _rF0->bit.MCLK_SEL = XC_MCLK_SEL_OSC_B;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xc24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_B1, &_r67->ALL, 1U);

    const float xc_mclk = 35000000.0f; /* 35MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt21_t* _r39 = &gt_xc24_set_gr1_regs.reg._r39;
    _r39->bit.fll2cnt = ((fll_out & 0x00FFFF) >>  0U);
    xc24_write_grp1_reg(XC_FLLCNT21, &_r39->ALL, 1U);

    _v_fllcnt22_t* _r3A = &gt_xc24_set_gr1_regs.reg._r3A;
    _r3A->bit.fll2cnt = ((fll_out & 0x1F0000) >> 16U);
    //_r3A->bit.fllsync = XC_FLL_PAD_VSYNC;
    //_r3A->bit.fll2_en = 1U;
    xc24_write_grp1_reg(XC_FLLCNT22, &_r3A->ALL, 1U);
}

void xc24_test_init_fll_b_40m(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 0U;
    _rF0->bit.DACO2_DIRECT = 0U;
    _rF0->bit.DACO3_DIRECT = 0U;
    //_rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 1U;
    _rF0->bit.MCLK_SEL = XC_MCLK_SEL_OSC_B;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xc24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xc24_write_grp1_reg(XC_OSC_FLL_MAN_B1, &_r67->ALL, 1U);

    const float xc_mclk = 40000000.0f; /* 40MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xc_mclk / vsync + 0.5f); /* round up */

    _v_fllcnt21_t* _r39 = &gt_xc24_set_gr1_regs.reg._r39;
    _r39->bit.fll2cnt = ((fll_out & 0x00FFFF) >>  0U);
    xc24_write_grp1_reg(XC_FLLCNT21, &_r39->ALL, 1U);

    _v_fllcnt22_t* _r3A = &gt_xc24_set_gr1_regs.reg._r3A;
    _r3A->bit.fll2cnt = ((fll_out & 0x1F0000) >> 16U);
    //_r3A->bit.fllsync = XC_FLL_PAD_VSYNC;
    //_r3A->bit.fll2_en = 1U;
    xc24_write_grp1_reg(XC_FLLCNT22, &_r3A->ALL, 1U);
}

void xc24_test_init_dac_p1(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->ALL = 0U;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    xc24_test_set_curr_tgt_dac(XC_TEST_DAC_INPUT_P1);

    _v_dac_nf_control_t* _r42 = &gt_xc24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xc24_write_grp1_reg(XC_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xc24_test_init_dac_p2(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    xc24_test_set_curr_tgt_dac(XC_TEST_DAC_INPUT_P2);

    _v_dac_nf_control_t* _r42 = &gt_xc24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xc24_write_grp1_reg(XC_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xc24_test_init_dac_p3(void)
{
    // set proper xc24 register
    _v_test_control_t* _rF0 = &gt_xc24_set_otp_regs.reg._rF0;
    _rF0->bit.TEST_EN = 1U;
    _rF0->bit.DACO1_DIRECT = 1U;
    _rF0->bit.DACO2_DIRECT = 1U;
    _rF0->bit.DACO3_DIRECT = 1U;
    xc24_write_otp_control(XC_TEST_CONTROL, &_rF0->ALL, 1U);

    xc24_test_set_curr_tgt_dac(XC_TEST_DAC_INPUT_P3);

    _v_dac_nf_control_t* _r42 = &gt_xc24_set_gr1_regs.reg._r42;
    _r42->bit.dac_lvl = 0U;
    xc24_write_grp1_reg(XC_DAC_NF_CONTROL, &_r42->ALL, 1U);
}

void xc24_test_start_icc_stby(void)
{
    ADS114S08_Set_Start(true);
}
void xc24_test_start_icc_actv(void)
{
    ADS114S08_Set_Start(true);
}
void xc24_test_start_ldo(void)
{
    ADS114S08_Set_Start(true);
}
void xc24_test_start_ldo_fll_a(void)
{
    ADS114S08_Set_Start(true);
}
void xc24_test_start_ldo_fll_b(void)
{
    ADS114S08_Set_Start(true);
}
void xc24_test_start_fll_a_30m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xc24_test_start_fll_a_35m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xc24_test_start_fll_a_40m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xc24_test_start_fll_b_30m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xc24_test_start_fll_b_35m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xc24_test_start_fll_b_40m(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xc24_test_start_dac_p1(void)
{
    ADS114S08_Set_Start(true);
}
void xc24_test_start_dac_p2(void)
{
    ADS114S08_Set_Start(true);
}
void xc24_test_start_dac_p3(void)
{
    ADS114S08_Set_Start(true);
}

void xc24_test_set_curr_tgt_dac(uint16_t curr_tgt_dac)
{
    if (curr_tgt_dac < 4096)
    {
        _v_current_target_dac1_t* _r44 = &gt_xc24_set_gr1_regs.reg._r44;
        _r44->bit.curr_tgt_dac1 = curr_tgt_dac;
        xc24_write_grp1_reg(XC_CURRENT_TARGET_DAC1, &_r44->ALL, 1U);

        _v_current_target_dac2_t* _r45 = &gt_xc24_set_gr1_regs.reg._r45;
        _r45->bit.curr_tgt_dac2 = curr_tgt_dac;
        xc24_write_grp1_reg(XC_CURRENT_TARGET_DAC2, &_r45->ALL, 1U);

        _v_current_target_dac3_t* _r46 = &gt_xc24_set_gr1_regs.reg._r46;
        _r46->bit.curr_tgt_dac3 = curr_tgt_dac;
        xc24_write_grp1_reg(XC_CURRENT_TARGET_DAC3, &_r46->ALL, 1U);
    }
    else
    {
        FATAL_INVALID_INPUT(curr_tgt_dac);
    }
}

void xc24_test(void)
{

}