#include <stdio.h>
#include <string.h>

#include "crc.h"
#include "drv_spi.h"
#include "drv_xcr24.h"
#include "drv_xdr12.h"
#include "ads124s08.h"
#include "comm_debugging.h"

#define XCR_CONV_FREQ_TO_XCR_MCLK(Hz)   (uint32_t)(((float)XCR_INTERNAL_MCLK) / (float)(Hz) + 0.5f)
#define XCR_CONV_us_TO_XCR_MCLK(us)     (uint16_t)(((float)XCR_INTERNAL_MCLK) / (1000000.0f / (float)(us)) + 0.5f)

#define XCR_SVO_ON_TIME_US      (10U) /* 10us */
#define XCR_SVO1_OFF_TIME_US    (100U) /* 100us */
#define XCR_SVO2_OFF_TIME_US    (100U) /* 100us */
#define XCR_SVO3_OFF_TIME_US    (100U) /* 100us */

#define XCR_SPI_HEADER_SIZE     (1U)
#define XCR_SPI_BURST_MAX_SIZE  (64U)
#define XCR_SPI_BUFF_MAX_SIZE   (XCR_SPI_HEADER_SIZE + XCR_SPI_BURST_MAX_SIZE)

#define XCR_DAC_TRIM_INPUT      (0xC8U)

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

typedef enum tag_XCR_RW_GRP
{
    XCR_RW_GRP1 = 0U,
    XCR_RW_GRP2,
    XCR_RW_GRP_MAX,
} xcr_rw_grp_t;

volatile bool gb_xcr_ld_transfer_spi_dma_flag;

static uint8_t gn_xcr_daisied_dev_blk_size;
static uint8_t gn_xcr_channel_enable[24];
static uint8_t gn_xcr_channel_daisy_size[24];
static uint8_t gn_xcr_channel_block_size[24];
static uint32_t gn_xcr_fll_cnt[2];

static _xcr_group1_regs_t gt_xcr24_set_gr1_regs;
static _xcr_group1_regs_t gt_xcr24_get_gr1_regs;

static _xcr_group2_regs_t gt_xcr24_set_gr2_regs;
static _xcr_group2_regs_t gt_xcr24_get_gr2_regs;

static _xcr_otp_control_regs_t gt_xcr24_set_otp_regs; /* base address 0xF0 */
static _xcr_otp_control_regs_t gt_xcr24_get_otp_regs; /* base address 0xF0 */

static bool gb_xcr_do_efuse;

static void xcr24_change_rw_grp_type(xcr_rw_grp_t rw_grp);

static void xcr24_regs_init_table(void)
{
    _xcr_group1_regs_t* _r1 = &gt_xcr24_set_gr1_regs;
    _xcr_group2_regs_t* _r2 = &gt_xcr24_set_gr2_regs;

    for(xcr_addr_grp1_t addr = XCR_RESET ; addr < XCR_GRP1_MAX ; ++addr)
    {
        switch(addr)
        {
        case XCR_RESET:
            _r1->reg._r00.bit.rst1 = 0U;
            _r1->reg._r00.bit.rst2 = 0U;
            _r1->reg._r00.bit.rst3 = 0U;
            _r1->reg._r00.bit.vsync_rst_en1 = 1U;
            _r1->reg._r00.bit.vsync_rst_en2 = 1U;
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
            break;
        case XCR_LOCAL_RW_POINTER_RESET:
            _r1->reg._r10.bit.local_transfer_pointer_rst = 1U;
            _r1->reg._r10.bit.local_receive_pointer_rst = 1U;
            break;
        case XCR_INTERRUPT_ENABLE:
            _r1->reg._r13.bit.int_fb_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_open_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_short_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_thermal_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_ld_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_rd_rec_fail_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_fault_auto_rec_fail_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_fault_rec_fail_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_timeout_err_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_spi_pc_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_acc_cnt_err_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_cmd_pc_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_uv15_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_ov15_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_uv50_en = XCR_FUNCTION_DIS;
            _r1->reg._r13.bit.int_ov50_en = XCR_FUNCTION_DIS;
            break;
        case XCR_SPI_FAULT_STATUS_CONTROL:
            _r1->reg._r1A.bit.spi_parity_err_det_en = XCR_FUNCTION_DIS;
            _r1->reg._r1A.bit.spi_acc_cnt_err_det_en = XCR_FUNCTION_DIS;
            break;
        case XCR_CLK_CONTROL_1:
            _r1->reg._r1B.bit.serializer_skew_en = XCR_FUNCTION_DIS;
            _r1->reg._r1B.bit.osc1_spread_en = XCR_FUNCTION_EN;
            _r1->reg._r1B.bit.serializer_clk_sel1 = XCR_FUNCTION_DIS;
            _r1->reg._r1B.bit.sprd1_gain = XCR_FUNCTION_DIS;
            _r1->reg._r1B.bit.serializer_clk_sel2 = XCR_FUNCTION_DIS;
            _r1->reg._r1B.bit.ld_rd_clk_sel = XCR_FUNCTION_DIS;
            _r1->reg._r1B.bit.spread1_spd = XCR_FUNCTION_DIS;
            break;
        case XCR_CLK_CONTROL_2:
            _r1->reg._r1C.bit.mclk_mode = XCR_FUNCTION_DIS;
            _r1->reg._r1C.bit.osc2_spread_en = XCR_FUNCTION_DIS;
            _r1->reg._r1C.bit.sprd2_gain = XCR_FUNCTION_DIS;
            _r1->reg._r1C.bit.spread2_spd = XCR_FUNCTION_DIS;
            break;
        case XCR_SERIALIZER_CLOCK_GEN:
            _r1->reg._r1D.bit.serial_clk_high = XCR_SERIAL_CLK_HIGH;
            _r1->reg._r1D.bit.serial_clk_low = XCR_SERIAL_CLK_LOW;
            break;
        case XCR_LATENCY:
            _r1->reg._r1E.bit.cmd_latency = 0U;
            _r1->reg._r1E.bit.serial_latency = 0U;
            break;
        case XCR_TIMEOUT:
            _r1->reg._r1F.bit.timeout = 0U;
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
            _r1->reg._r37.bit.fll1cnt = ((gn_xcr_fll_cnt[0] & 0x000FFFU) >>  0U);
            break;
        case XCR_FLLCNT12:
            _r1->reg._r38.bit.fll1cnt = ((gn_xcr_fll_cnt[0] & 0x1FF000U) >> 12U);;
            _r1->reg._r38.bit.fll1_err_range = 0U;
            _r1->reg._r38.bit.fll1_range = 0U;
            _r1->reg._r38.bit.fllsync = 0U;
            _r1->reg._r38.bit.fll1_en = 0U;
            break;
        case XCR_FLLCNT21:
            _r1->reg._r39.bit.fll2cnt = ((gn_xcr_fll_cnt[1] & 0x000FFFU) >>  0U);
            break;
        case XCR_FLLCNT22:
            _r1->reg._r3A.bit.fll2cnt = ((gn_xcr_fll_cnt[1] & 0x1FF000U) >> 12U);
            _r1->reg._r3A.bit.fll2_err_range = 0U;
            _r1->reg._r3A.bit.fll2_range = 0U;
            _r1->reg._r3A.bit.fllsync = 0U;
            _r1->reg._r3A.bit.fll2_en = 0U;
            break;
        case XCR_VO_DELAY:
            _r1->reg._r3B.bit.vo_delay = 0U;
            _r1->reg._r3B.bit.gate1_pol = 0U;
            _r1->reg._r3B.bit.gate2_pol = 0U;
            _r1->reg._r3B.bit.gate3_pol = 0U;
            break;
        case XCR_VO_OFF_ON:
            _r1->reg._r3C.bit.vo_on = 0U;
            _r1->reg._r3C.bit.vo_off = 0U;
            break;

        case XCR_SVO_ON:
            _r1->reg._r3D.bit.svo_on = XCR_CONV_us_TO_XCR_MCLK(XCR_SVO_ON_TIME_US);
            break;
        case XCR_SVO1_OFF:
            _r1->reg._r3E.bit.svo1_off = XCR_CONV_us_TO_XCR_MCLK(XCR_SVO1_OFF_TIME_US);
            break;
        case XCR_SVO2_OFF:
            _r1->reg._r3F.bit.svo2_off = XCR_CONV_us_TO_XCR_MCLK(XCR_SVO2_OFF_TIME_US);
            break;
        case XCR_SVO3_OFF:
            _r1->reg._r40.bit.svo3_off = XCR_CONV_us_TO_XCR_MCLK(XCR_SVO3_OFF_TIME_US);
            break;
        case XCR_SVO_NUMBER:
            _r1->reg._r41.bit.sv_no = XDR_SV_NO;
            _r1->reg._r41.bit.sv_no_type = XCR_SVO_ACTIVE_23;
            break;
        case XCR_DAC_NF_CONTROL:
            _r1->reg._r42.bit.dgrjt_en = 0U;
            _r1->reg._r42.bit.bbkn_en = 0U;
            _r1->reg._r42.bit.bbkn_th = 0U;
            _r1->reg._r42.bit.dac_lvl = 0U;
            break;

        case XCR_DAC_CONTROL:
            _r1->reg._r43.bit.dac1_auto = 0U;
            _r1->reg._r43.bit.dac2_auto = 0U;
            _r1->reg._r43.bit.dac3_auto = 0U;
            _r1->reg._r43.bit.dac_auto_type = 0U;
            _r1->reg._r43.bit.dac_sync_mode = 0U;
            _r1->reg._r43.bit.dac1_fb_mode = 0U;
            _r1->reg._r43.bit.dac2_fb_mode = 0U;
            _r1->reg._r43.bit.dac3_fb_mode = 0U;
            _r1->reg._r43.bit.dac1_dec1_mode = 0U;
            _r1->reg._r43.bit.dac2_dec1_mode = 0U;
            _r1->reg._r43.bit.dac3_dec1_mode = 0U;
            _r1->reg._r43.bit.dac1_hold_en = 0U;
            _r1->reg._r43.bit.dac2_hold_en = 0U;
            _r1->reg._r43.bit.dac3_hold_en = 0U;
            _r1->reg._r43.bit.dac_fault_off = 0U;
            break;
        case XCR_CURRENT_TARGET_DAC1:
            _r1->reg._r44.bit.curr_tgt_dac1 = 0U;
            break;
        case XCR_CURRENT_TARGET_DAC2:
            _r1->reg._r45.bit.curr_tgt_dac2 = 0U;
            break;
        case XCR_CURRENT_TARGET_DAC3:
            _r1->reg._r46.bit.curr_tgt_dac3 = 0U;
            break;
        //case XCR_PREVIOUS_TARGET_DAC1:
        //case XCR_PREVIOUS_TARGET_DAC2:
        //case XCR_PREVIOUS_TARGET_DAC3:
        //case XCR_DAC1_OUT:
        //case XCR_DAC2_OUT:
        //case XCR_DAC3_OUT:
        //case XCR_DAC1_STATE:
        //case XCR_DAC2_STATE:
        //case XCR_DAC3_STATE:

        case XCR_DAC1_INCREMENT_1:
            _r1->reg._r50.bit.dac1_inc1 = 0U;
            _r1->reg._r50.bit.dac1_inc2 = 0U;
            _r1->reg._r50.bit.dac1_inc3 = 0U;
            break;
        case XCR_DAC1_INCREMENT_2_HOLD_LIMIT:
            _r1->reg._r51.bit.dac1_inc4 = 0U;
            _r1->reg._r51.bit.dac1_inc_hold_limit = 0U;
            break;
        case XCR_DAC1_DECREMENT_INC_WAIT:
            _r1->reg._r52.bit.dac1_inc1_wait = 0U;
            _r1->reg._r52.bit.dac1_dec1 = 0U;
            break;
        case XCR_DAC1_INCREMENT_HOLD_THRESHOLD:
            _r1->reg._r53.bit.dac1_inc_hold_threshold = 0U;
            _r1->reg._r53.bit.bit_signed = 0U;
            break;
        case XCR_DAC1_FB_VALID_TIMER:
            _r1->reg._r54.bit.dac1_fb_valid_timer = 0U;
            break;
        case XCR_DAC1_MIN_LIMIT:
            _r1->reg._r55.bit.dac1_min_limit = 0U;
            break;
        case XCR_DAC1_MAX_LIMIT:
            _r1->reg._r56.bit.dac1_max_limit = 0U;
            break;

        case XCR_DAC2_INCREMENT_1:
            _r1->reg._r57.bit.dac2_inc1 = 0U;
            _r1->reg._r57.bit.dac2_inc2 = 0U;
            _r1->reg._r57.bit.dac2_inc3 = 0U;
            break;
        case XCR_DAC2_INCREMENT_2_HOLD_LIMIT:
            _r1->reg._r58.bit.dac2_inc4 = 0U;
            _r1->reg._r58.bit.dac2_inc_hold_limit = 0U;
            break;
        case XCR_DAC2_DECREMENT_INC_WAIT:
            _r1->reg._r59.bit.dac2_inc1_wait = 0U;
            _r1->reg._r59.bit.dac2_dec1 = 0U;
            break;
        case XCR_DAC2_INCREMENT_HOLD_THRESHOLD:
            _r1->reg._r5A.bit.dac2_inc_hold_threshold = 0U;
            _r1->reg._r5A.bit.bit_signed = 0U;
            break;
        case XCR_DAC2_FB_VALID_TIMER:
            _r1->reg._r5B.bit.dac2_fb_valid_timer = 0U;
            break;
        case XCR_DAC2_MIN_LIMIT:
            _r1->reg._r5C.bit.dac2_min_limit = 0U;
            break;
        case XCR_DAC2_MAX_LIMIT:
            _r1->reg._r5D.bit.dac2_max_limit = 0U;
            break;

        case XCR_DAC3_INCREMENT_1:
            _r1->reg._r5E.bit.dac3_inc1 = 0U;
            _r1->reg._r5E.bit.dac3_inc2 = 0U;
            _r1->reg._r5E.bit.dac3_inc3 = 0U;
            break;
        case XCR_DAC3_INCREMENT_2_HOLD_LIMIT:
            _r1->reg._r5F.bit.dac3_inc4 = 0U;
            _r1->reg._r5F.bit.dac3_inc_hold_limit = 0U;
            break;
        case XCR_DAC3_DECREMENT_INC_WAIT:
            _r1->reg._r60.bit.dac3_inc1_wait = 0U;
            _r1->reg._r60.bit.dac3_dec1 = 0U;
            break;
        case XCR_DAC3_INCREMENT_HOLD_THRESHOLD:
            _r1->reg._r61.bit.dac3_inc_hold_threshold = 0U;
            _r1->reg._r61.bit.bit_signed = 0U;
            break;
        case XCR_DAC3_FB_VALID_TIMER:
            _r1->reg._r62.bit.dac3_fb_valid_timer = 0U;
            break;
        case XCR_DAC3_MIN_LIMIT:
            _r1->reg._r63.bit.dac3_min_limit = 0U;
            break;
        case XCR_DAC3_MAX_LIMIT:
            _r1->reg._r64.bit.dac3_max_limit = 0U;
            break;

        case XCR_OSC_FLL_MAN_A1:
            _r1->reg._r65.bit.FLT_GAIN_A = 0U;
            _r1->reg._r65.bit.FLT_CTL_A = 0U;
            _r1->reg._r65.bit.DAC_RNG_A = 0U;
            _r1->reg._r65.bit.OSC_MAN_EN_A = 0U;
            break;
        case XCR_OSC_FLL_MAN_A2:
            _r1->reg._r66.bit.OSC_FLL_MAN_A = 0U;
            break;
        case XCR_OSC_FLL_MAN_B1:
            _r1->reg._r67.bit.FLT_GAIN_B = 0U;
            _r1->reg._r67.bit.FLT_CTL_B = 0U;
            _r1->reg._r67.bit.DAC_RNG_B = 0U;
            _r1->reg._r67.bit.OSC_MAN_EN_B = 0U;
            break;
        case XCR_OSC_FLL_MAN_B2:
            _r1->reg._r68.bit.OSC_FLL_MAN_B = 0U;
            break;
        default:
            continue;
        }

        xcr24_write_grp1_reg(addr, &_r1->ALL[addr], 1U);
    }

    for(xcr_addr_grp2_t addr = XCR_GRP2_DAC1_FB_VALID_CNT ; addr < XCR_GRP2_MAX ; ++addr)
    {
        switch(addr)
        {
        //case XCR_GRP2_DAC1_FB_VALID_CNT:        /* Read-Only */
        //case XCR_GRP2_DAC1_INC_HOLD_WAIT_CNT:   /* Read-Only */
        //case XCR_GRP2_1R2:                      /* Read-Only */
        case XCR_GRP2_SOA1_N1_N11:
            _r2->reg._r03.bit.soa_n1 = 8U;
            _r2->reg._r03.bit.soa1_n11 = 50U;
            break;
        case XCR_GRP2_SOA1_P2_P1:
            _r2->reg._r04.bit.soa1_p1 = 125U;
            _r2->reg._r04.bit.soa_p2 = 12U;
            break;
        case XCR_GRP2_SOA1_P3_P2:
            _r2->reg._r05.bit.soa1_p2 = 18U;
            _r2->reg._r05.bit.soa1_p3 = 500U;
            break;
        //case XCR_GRP2_DAC2_FB_VALID_CNT:          /* Read-Only */
        //case XCR_GRP2_DAC2_INC_HOLD_WAIT_CNT:     /* Read-Only */
        //case XCR_GRP2_2R2:                        /* Read-Only */
        case XCR_GRP2_SOA2_N1_N11:
            _r2->reg._r09.bit.soa2_n1 = 8U;
            _r2->reg._r09.bit.soa2_n11 = 50U;
            break;
        case XCR_GRP2_SOA2_P2_P1:
            _r2->reg._r0A.bit.soa2_p1 = 125U;
            _r2->reg._r0A.bit.soa2_p2 = 12U;
            break;
        case XCR_GRP2_SOA2_P3_P2:
            _r2->reg._r0B.bit.soa2_p2 = 18U;
            _r2->reg._r0B.bit.soa2_p3 = 500U;
            break;
        //case XCR_GRP2_DAC3_FB_VALID_CNT:          /* Read-Only */
        //case XCR_GRP2_DAC3_INC_HOLD_WAIT_CNT:     /* Read-Only */
        //case XCR_GRP2_3R2:                        /* Read-Only */
        case XCR_GRP2_SOA3_N1_N11:
            _r2->reg._r0F.bit.soa3_n1 = 8U;
            _r2->reg._r0F.bit.soa3_n11 = 50U;
            break;
        case XCR_GRP2_SOA3_P2_P1:
            _r2->reg._r10.bit.soa3_p1 = 125U;
            _r2->reg._r10.bit.soa3_p2 = 12U;
            break;
        case XCR_GRP2_SOA3_P3_P2:
            _r2->reg._r11.bit.soa3_p2 = 18U;
            _r2->reg._r11.bit.soa3_p3 = 500U;
            break;
        case XCR_GRP2_ANA_TEST:
            _r2->reg._r12.bit.TEST_ANA_EN = 0U;
            _r2->reg._r12.bit.CHOP_EN_BGR = 0U;
            _r2->reg._r12.bit.CHOP_EN_OSCLDO = 0U;
            _r2->reg._r12.bit.CHOP_EN = 0U;
            break;
        default:
            continue;
            break;
        }

        xcr24_write_grp2_reg(addr, &_r2->ALL[addr], 1U);
    }
}

static void xcr24_regs_trim_init_table(void)
{
    _xcr_group1_regs_t* _r1 = &gt_xcr24_set_gr1_regs;
    _xcr_group2_regs_t* _r2 = &gt_xcr24_set_gr2_regs;
    _xcr_otp_control_regs_t* _rotp = &gt_xcr24_set_otp_regs;

    for(xcr_addr_grp1_t addr = XCR_RESET ; addr < XCR_GRP1_MAX ; ++addr)
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

    for(xcr_addr_grp2_t addr = XCR_GRP2_DAC1_FB_VALID_CNT ; addr < XCR_GRP2_MAX ; ++addr)
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

    for (xcr_addr_otp_t addr = XCR_TEST_CONTROL ; addr < XCR_OTP_MAX ; ++addr)
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
    comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 GROUP1 Registers");
    for (xcr_addr_grp1_t addr = XCR_RESET ; addr < XCR_GRP1_MAX ; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR [0x%02X] DATA [0x%03X]", addr, gt_xcr24_get_gr1_regs.ALL[addr]);
    }

    comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 GROUP2 Registers");
    for (xcr_addr_grp2_t addr = XCR_GRP2_DAC1_FB_VALID_CNT ; addr < XCR_GRP2_MAX ; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR [0x%02X] DATA [0x%03X]", addr, gt_xcr24_get_gr2_regs.ALL[addr]);
    }

    comm_UART_Printf(LOG_LV_INFO, "\r\nXCR24 OTP Control Registers");
    for (xcr_addr_otp_t addr = XCR_TEST_CONTROL ; addr < XCR_OTP_MAX ; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR [0x%02X] DATA [0x%03X]", addr, gt_xcr24_get_otp_regs.ALL[addr]);
    }
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

    xcr24_read_otp_control(XCR_TEST_CONTROL, 10U);

    xcr24_dump_registers();
    xcr24_memory_copy();
}

void xcr24_init_param(void)
{
    gn_xcr_daisied_dev_blk_size = BLOCK_PER_XDR;

    /* XC24R channel enable */
    gn_xcr_channel_enable[ 0U] = 1U;
    gn_xcr_channel_enable[ 1U] = 0U;
    gn_xcr_channel_enable[ 2U] = 0U;
    gn_xcr_channel_enable[ 3U] = 0U;
    gn_xcr_channel_enable[ 4U] = 0U;
    gn_xcr_channel_enable[ 5U] = 0U;
    gn_xcr_channel_enable[ 6U] = 0U;
    gn_xcr_channel_enable[ 7U] = 0U;
    gn_xcr_channel_enable[ 8U] = 0U;
    gn_xcr_channel_enable[ 9U] = 0U;
    gn_xcr_channel_enable[10U] = 0U;
    gn_xcr_channel_enable[11U] = 0U;
    gn_xcr_channel_enable[12U] = 0U;
    gn_xcr_channel_enable[13U] = 0U;
    gn_xcr_channel_enable[14U] = 0U;
    gn_xcr_channel_enable[15U] = 0U;
    gn_xcr_channel_enable[16U] = 0U;
    gn_xcr_channel_enable[17U] = 0U;
    gn_xcr_channel_enable[18U] = 0U;
    gn_xcr_channel_enable[19U] = 0U;
    gn_xcr_channel_enable[20U] = 0U;
    gn_xcr_channel_enable[21U] = 0U;
    gn_xcr_channel_enable[22U] = 0U;
    gn_xcr_channel_enable[23U] = 0U;

    gn_xcr_channel_daisy_size[ 0U] = XDR_DAISY_LENGTH;
    gn_xcr_channel_daisy_size[ 1U] = 0U;
    gn_xcr_channel_daisy_size[ 2U] = 0U;
    gn_xcr_channel_daisy_size[ 3U] = 0U;
    gn_xcr_channel_daisy_size[ 4U] = 0U;
    gn_xcr_channel_daisy_size[ 5U] = 0U;
    gn_xcr_channel_daisy_size[ 6U] = 0U;
    gn_xcr_channel_daisy_size[ 7U] = 0U;
    gn_xcr_channel_daisy_size[ 8U] = 0U;
    gn_xcr_channel_daisy_size[ 9U] = 0U;
    gn_xcr_channel_daisy_size[10U] = 0U;
    gn_xcr_channel_daisy_size[11U] = 0U;
    gn_xcr_channel_daisy_size[12U] = 0U;
    gn_xcr_channel_daisy_size[13U] = 0U;
    gn_xcr_channel_daisy_size[14U] = 0U;
    gn_xcr_channel_daisy_size[15U] = 0U;
    gn_xcr_channel_daisy_size[16U] = 0U;
    gn_xcr_channel_daisy_size[17U] = 0U;
    gn_xcr_channel_daisy_size[18U] = 0U;
    gn_xcr_channel_daisy_size[19U] = 0U;
    gn_xcr_channel_daisy_size[20U] = 0U;
    gn_xcr_channel_daisy_size[21U] = 0U;
    gn_xcr_channel_daisy_size[22U] = 0U;
    gn_xcr_channel_daisy_size[23U] = 0U;

    gn_xcr_channel_block_size[ 0U] = (gn_xcr_channel_daisy_size[ 0U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 1U] = (gn_xcr_channel_daisy_size[ 1U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 2U] = (gn_xcr_channel_daisy_size[ 2U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 3U] = (gn_xcr_channel_daisy_size[ 3U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 4U] = (gn_xcr_channel_daisy_size[ 4U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 5U] = (gn_xcr_channel_daisy_size[ 5U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 6U] = (gn_xcr_channel_daisy_size[ 6U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 7U] = (gn_xcr_channel_daisy_size[ 7U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 8U] = (gn_xcr_channel_daisy_size[ 8U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[ 9U] = (gn_xcr_channel_daisy_size[ 9U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[10U] = (gn_xcr_channel_daisy_size[10U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[11U] = (gn_xcr_channel_daisy_size[11U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[12U] = (gn_xcr_channel_daisy_size[12U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[13U] = (gn_xcr_channel_daisy_size[13U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[14U] = (gn_xcr_channel_daisy_size[14U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[15U] = (gn_xcr_channel_daisy_size[15U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[16U] = (gn_xcr_channel_daisy_size[16U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[17U] = (gn_xcr_channel_daisy_size[17U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[18U] = (gn_xcr_channel_daisy_size[18U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[19U] = (gn_xcr_channel_daisy_size[19U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[20U] = (gn_xcr_channel_daisy_size[20U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[21U] = (gn_xcr_channel_daisy_size[21U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[22U] = (gn_xcr_channel_daisy_size[22U] * gn_xcr_daisied_dev_blk_size);
    gn_xcr_channel_block_size[23U] = (gn_xcr_channel_daisy_size[23U] * gn_xcr_daisied_dev_blk_size);

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
    cmd.bit.addr = (OTP_BASE_ADDR + addr);
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
        for(uint16_t i = 0U ; i < burst_size ; ++i)
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
    cmd.bit.addr = (OTP_BASE_ADDR + addr);
    cmd.bit.size = burst_size;

    tx_buffer[spi_len++] = cmd.ALL;

    for(uint16_t i = 0U ; i < burst_size ; ++i)
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

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xcr_otp_control_regs_t* _r = &gt_xcr24_set_otp_regs;
        for(uint16_t i = 0U ; i < burst_size ; ++i)
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
        for(uint16_t i = 0U ; i < burst_size ; ++i)
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
        for(uint16_t i = 0U ; i < burst_size ; ++i)
        {
            _r->ALL[addr + i] = rx_buffer[XCR_SPI_HEADER_SIZE];
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

    for(uint16_t i = 0U ; i < burst_size ; ++i)
    {
        tx_buffer[spi_len++] = *q++;
    }

    xcr24_change_rw_grp_type(XCR_RW_GRP1);

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, tx_buffer, spi_len, 20U);
    XCR_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xcr_group1_regs_t* _r = &gt_xcr24_set_gr1_regs;
        for(uint16_t i = 0U ; i < burst_size ; ++i)
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
    cmd.bit.addr = addr;
    cmd.bit.size = burst_size;

    tx_buffer[spi_len++] = cmd.ALL;

    for(uint16_t i = 0U ; i < burst_size ; ++i)
    {
        tx_buffer[spi_len++] = *q++;
    }

    xcr24_change_rw_grp_type(XCR_RW_GRP2);

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, tx_buffer, spi_len, 20U);
    XCR_NSS_HI();

    if(SPI_TIMEOUT == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
    }
    else
    {
        _xcr_group2_regs_t* _r = &gt_xcr24_set_gr2_regs;
        for(uint16_t i = 0U ; i < burst_size ; ++i)
        {
            _r->ALL[addr + i] = tx_buffer[1U + i];
        }
    }
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
        cmd.bit.addr = (OTP_BASE_ADDR + XCR_TEST_CONTROL);
        cmd.bit.size = 1U;

        tx_buffer[0] = cmd.ALL;
        tx_buffer[1] = _rF0->ALL;

        XCR_NSS_LO();
        uint8_t ret = spi_write(SPI1, tx_buffer, 2, 20U);
        XCR_NSS_HI();

        if(SPI_TIMEOUT == ret)
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\nspi write timeout");
        }
    }
}

void xcr24_set_ld_transfer(uint16_t* buffer, uint16_t length)
{
    if((NULL == buffer) || (0U == length))
    {
        return ;
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
    _r44->bit.curr_tgt_dac1 = XCR_DAC_TRIM_INPUT;
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
    _r45->bit.curr_tgt_dac2 = XCR_DAC_TRIM_INPUT;
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
    _r46->bit.curr_tgt_dac3 = XCR_DAC_TRIM_INPUT;
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
    _rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK_SEL = 1U;
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
    _rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK_SEL = 0U;
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
    for (xcr_addr_otp_t mirror_addr = XCR_MIRROR1 ; mirror_addr < XCR_GATE_CONTROL ; ++mirror_addr) // 0xF5 ~ 0xF9
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
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✕]%s ADDR [0x%02X] - [0x%03X - 0x%03X]", \
                ANSI_FONT_RED, ANSI_FONT_NONE, mirror_addr, saved_reg, read_reg);
        }
        else
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✔]%s ADDR [0x%02X] - [0x%03X - 0x%03X]", \
                ANSI_FONT_GREEN, ANSI_FONT_NONE, mirror_addr, saved_reg, read_reg);
        }
    }
    return ret;
}

void xcr24_test_init_icc_stby(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P, ADS114S08_CH_XD_ICC_N);
}

void xcr24_test_init_icc_actv(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P, ADS114S08_CH_XD_ICC_N);
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
    _rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 0U;
    _rF0->bit.MCLK_SEL = 1U;
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
    _rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 0U;
    _rF0->bit.MCLK_SEL = 1U;
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
    _rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 0U;
    _rF0->bit.MCLK_SEL = 1U;
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
    _rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 0U;
    _rF0->bit.MCLK_SEL = 0U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xcr24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_A1, &_r67->ALL, 1U);

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
    _rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 0U;
    _rF0->bit.MCLK_SEL = 0U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xcr24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_A1, &_r67->ALL, 1U);

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
    _rF0->bit.MCLK64_O = 1U;
    _rF0->bit.MCLK1_O = 0U;
    _rF0->bit.MCLK_SEL = 0U;
    xcr24_write_otp_control(XCR_TEST_CONTROL, &_rF0->ALL, 1U);

    _v_osc_fll_man_b1_t* _r67 = &gt_xcr24_set_gr1_regs.reg._r67;
    _r67->bit.OSC_MAN_EN_B = 0U;
    _r67->bit.FLT_CTL_B = 1U;
    xcr24_write_grp1_reg(XCR_OSC_FLL_MAN_A1, &_r67->ALL, 1U);

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