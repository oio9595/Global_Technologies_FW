

#include "drv_xdr12.h"
#include "drv_xcr24.h"
#include "drv_spi.h"

#include "comm_debugging.h"
#include "ldim_conversion.h"

#include "crc.h"

#define SPI_PACKET_DEBUG        (1U)

volatile bool gb_xcr_ld_transfer_spi_dma_flag;

static uint8_t gn_xcr_daisied_dev_ch_size;
static uint8_t gn_xcr_channel_enable[XCR_CH_SIZE];
static uint8_t gn_xcr_channel_daisy_size[XCR_CH_SIZE];
static uint8_t gn_xcr_channel_block_size[XCR_CH_SIZE];

static _xcr_group1_regs_t gt_xcr24_set_gr1_regs;
static _xcr_group2_regs_t gt_xcr24_set_gr2_regs;
static _xcr_group1_regs_t gt_xcr24_get_gr1_regs;
static _xcr_group2_regs_t gt_xcr24_get_gr2_regs;

static _xcr_otp_control_regs_t gt_xcr24_otp_access; /* base address 0xF0 */

#if (XC_MODEL_TYPE == XC_TYPE_XC24R)
#else
#endif

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
        //case XCR_GLOBAL_WRITE:
        //case XCR_LOCAL_WRITE:
        //case XCR_LOCAL_READ:
        //case XCR_ID_GEN:
        //case XCR_FAULT_READ:
        //case XCR_LD_TRANSFER:
        //case XCR_SYNC_GEN:
        case XCR_COMMAND_AUTO_ENABLE:
            _r1->reg._r08.bit.sync_auto_en = 0U;
            _r1->reg._r08.bit.fault_auto_en = 0U;
            break;
        //case XCR_LD_WRITE_POINTER:
        //case XCR_LD_READ_POINTER:
        //case XCR_LD_DIFFERENCE_POINTER:
        case XCR_LD_START_POINTER_TH:
            _r1->reg._r0C.bit.ld_diff_threshold = 0U;
            _r1->reg._r0C.bit.int_ld_sign = 0U;
            _r1->reg._r0C.bit.ld_transfer_start_pointer = 0U;
            break;
        //case XCR_LOCAL_WRITE_TRANSFER_POINTER:
        //case XCR_LOCAL_READ_RECEIVE_POINTER:
        //case XCR_LOCAL_RW_DIFFERENCE_POINTER:
        case XCR_LOCAL_RW_POINTER_RESET:
            _r1->reg._r10.bit.local_transfer_pointer_rst = 1U;
            _r1->reg._r10.bit.local_receive_pointer_rst = 1U;
            break;
        case XCR_FAULT_AUTO_READ_INTERVAL:
            _r1->reg._r11.bit.fault_auto_rd_interval = 0U;
            break;
        case XCR_FAULT_AUTO_READ_EVENT:
            _r1->reg._r12.bit.fault_auto_rd_interval = 0U;
            _r1->reg._r12.bit.fault_auto_rd_timer_event = 0U;
            break;
        case XCR_INTERRUPT_ENABLE:
#if (XC_MODEL_TYPE == XC_TYPE_XC24R)
            _r1->reg._r13.bit.int_fb_en = 0U;
            _r1->reg._r13.bit.int_open_en = 0U;
            _r1->reg._r13.bit.int_short_en = 0U;
            _r1->reg._r13.bit.int_thermal_en = 0U;
            _r1->reg._r13.bit.int_ld_en = 0U;
            _r1->reg._r13.bit.int_rd_rec_fail_en = 0U;
            _r1->reg._r13.bit.int_fault_auto_rec_fail_en = 0U;
            _r1->reg._r13.bit.int_fault_rec_fail_en = 0U;
            _r1->reg._r13.bit.int_timeout_err_en = 0U;
            _r1->reg._r13.bit.int_spi_pc_en = 0U;
            _r1->reg._r13.bit.int_acc_cnt_err_en = 0U;
            _r1->reg._r13.bit.int_cmd_pc_en = 0U;
            _r1->reg._r13.bit.int_uv15_en = 0U;
            _r1->reg._r13.bit.int_ov15_en = 0U;
            _r1->reg._r13.bit.int_uv50_en = 0U;
            _r1->reg._r13.bit.int_ov50_en = 0U;
#elif (XC_MODEL_TYPE == XC_TYPE_IC603)
            _r1->reg._r13.bit.int_source1_en = 0U;
            _r1->reg._r13.bit.int_source2_en = 0U;
            _r1->reg._r13.bit.int_source3_en = 0U;
            _r1->reg._r13.bit.int_source4_en = 0U;
            _r1->reg._r13.bit.int_ld_en = 0U;
            _r1->reg._r13.bit.int_vs_miss_en = 0U;
            _r1->reg._r13.bit.int_timeout_err_en = 0U;
            _r1->reg._r13.bit.int_ser_crc_en = 0U;
            _r1->reg._r13.bit.int_spi_crc_en = 0U;
            _r1->reg._r13.bit.int_ctrl_mismatch_en = 0U;
            _r1->reg._r13.bit.int_gate_short_en = 0U;
            _r1->reg._r13.bit.int_osc_err_en = 0U;
            _r1->reg._r13.bit.int_ldo_ovuv_en = 0U;
            _r1->reg._r13.bit.int_bgr_ovuv_en = 0U;
#endif
            break;
        //case XCR_COMMAND_STATUS_1:
        //case XCR_COMMAND_STATUS_2:
        //case XCR_RECEIVE_STATUS:
        //case XCR_INTERRUPT_STATUS:
#if (XC_MODEL_TYPE == XC_TYPE_XC24R)
        //case XCR_CMD_PARITY_ERR_STATUS1:
        //case XCR_CMD_PARITY_ERR_STATUS2:
        case XCR_SPI_FAULT_STATUS_CONTROL:
            _r1->reg._r1A.bit.spi_parity_err_det_en = 0U;
            _r1->reg._r1A.bit.spi_acc_cnt_err_det_en = 0U;
            break;
        case XCR_CLK_CONTROL_1:
            _r1->reg._r1B.bit.serializer_skew_en = 0U;
            _r1->reg._r1B.bit.osc1_spread_en = 0U;
            _r1->reg._r1B.bit.serializer_clk_sel1 = 0U;
            _r1->reg._r1B.bit.sprd1_gain = 0U;
            _r1->reg._r1B.bit.serializer_clk_sel2 = 0U;
            _r1->reg._r1B.bit.ld_rd_clk_sel = 0U;
            _r1->reg._r1B.bit.spread1_spd = 0U;
            break;
        case XCR_CLK_CONTROL_2:
            _r1->reg._r1C.bit.mclk_mode = 0U;
            _r1->reg._r1C.bit.osc2_spread_en = 0U;
            _r1->reg._r1C.bit.sprd2_gain = 0U;
            _r1->reg._r1C.bit.spread2_spd = 0U;
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
            _r1->reg._r20.bit.daisied_dev_blk_size = gn_xcr_daisied_dev_ch_size;
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
#elif (XC_MODEL_TYPE == XC_TYPE_IC603)
        //case XCR_ERR_STATUS:
        //case XCR_CH_CRC_ERR_STATUS:
        //case XCR_CH_TIMEOUT_ERR_STATUS:
        case XCR_BIST_CONTROL_STATUS:
            _r1->reg._r1B.bit.BIST_START = 0U;
            break;
        case XCR_COMMUNICATION_FAULT_CONTROL:
            _r1->reg._r1C.bit.spi_crc_en = 0U;
            _r1->reg._r1C.bit.ser_crc_en = 0U;
            _r1->reg._r1C.bit.vs_miss_en = 0U;
            break;
        case XCR_CLK_CONTROL_1:
            _r1->reg._r1D.bit.serializer_skew_en = 0U;
            _r1->reg._r1D.bit.osc1_spread_en = 0U;
            _r1->reg._r1D.bit.serializer_clk_sel1 = 0U;
            _r1->reg._r1D.bit.sprd1_gain = 0U;
            _r1->reg._r1D.bit.serializer_clk_sel2 = 0U;
            _r1->reg._r1D.bit.ld_rd_clk_sel = 0U;
            _r1->reg._r1D.bit.spread1_spd = 0U;
            break;
        case XCR_CLK_CONTROL_2:
            _r1->reg._r1E.bit.mclk_mode = 0U;
            _r1->reg._r1E.bit.osc2_spread_en = 0U;
            _r1->reg._r1E.bit.sprd2_gain = 0U;
            _r1->reg._r1E.bit.spread2_spd = 0U;
            break;
        case XCR_SERIALIZER_CLOCK_GEN:
            _r1->reg._r1F.bit.serial_clk_high = XCR_SERIAL_CLK_HIGH;
            _r1->reg._r1F.bit.serial_clk_low = XCR_SERIAL_CLK_LOW;
            break;
        case XCR_LATENCY:
            _r1->reg._r20.bit.cmd_latency = 0U;
            _r1->reg._r20.bit.serial_latency = 0U;
            break;
        case XCR_TIMEOUT:
            _r1->reg._r21.bit.timeout = 0U;
            break;
        case XCR_DAISIED_DEVICE_CH_SIZE:
            _r1->reg._r22.bit.daisied_dev_ch_size1 = gn_xcr_daisied_dev_ch_size;
            break;
        case XCR_DAISY_SIZE_1:
            _r1->reg._r23.bit.daisy_size_ch1 = gn_xcr_channel_daisy_size[0U];
            _r1->reg._r23.bit.daisy_size_ch2 = gn_xcr_channel_daisy_size[1U];
            _r1->reg._r23.bit.daisy_size_ch3 = gn_xcr_channel_daisy_size[2U];
            break;
        case XCR_DAISY_SIZE_2:
            _r1->reg._r24.bit.daisy_size_ch4 = gn_xcr_channel_daisy_size[3U];
            _r1->reg._r24.bit.daisy_size_ch5 = gn_xcr_channel_daisy_size[4U];
            _r1->reg._r24.bit.daisy_size_ch6 = gn_xcr_channel_daisy_size[5U];
            break;
        case XCR_DAISY_SIZE_3:
            _r1->reg._r25.bit.daisy_size_ch7 = gn_xcr_channel_daisy_size[6U];
            _r1->reg._r25.bit.daisy_size_ch8 = gn_xcr_channel_daisy_size[7U];
            _r1->reg._r25.bit.daisy_size_ch9 = gn_xcr_channel_daisy_size[8U];
            break;
        case XCR_DAISY_SIZE_4:
            _r1->reg._r26.bit.daisy_size_ch10 = gn_xcr_channel_daisy_size[9U];
            _r1->reg._r26.bit.daisy_size_ch11 = gn_xcr_channel_daisy_size[10U];
            _r1->reg._r26.bit.daisy_size_ch12 = gn_xcr_channel_daisy_size[11U];
            break;
        case XCR_DAISY_SIZE_5:
            _r1->reg._r27.bit.daisy_size_ch13 = gn_xcr_channel_daisy_size[12U];
            _r1->reg._r27.bit.daisy_size_ch14 = gn_xcr_channel_daisy_size[13U];
            _r1->reg._r27.bit.daisy_size_ch15 = gn_xcr_channel_daisy_size[14U];
            break;
        case XCR_DAISY_SIZE_6:
            _r1->reg._r28.bit.daisy_size_ch16 = gn_xcr_channel_daisy_size[15U];
            //_r1->reg._r28.bit.daisy_size_ch17 = gn_xcr_channel_daisy_size[16U];
            //_r1->reg._r28.bit.daisy_size_ch18 = gn_xcr_channel_daisy_size[17U];
            break;
#endif
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
#if (XC_MODEL_TYPE == XC_TYPE_XC24R)
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
#endif
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
#if (XC_MODEL_TYPE == XC_TYPE_XC24R)
            _r1->reg._r36.bit.ch17_en = gn_xcr_channel_enable[16U];
            _r1->reg._r36.bit.ch18_en = gn_xcr_channel_enable[17U];
            _r1->reg._r36.bit.ch19_en = gn_xcr_channel_enable[18U];
            _r1->reg._r36.bit.ch20_en = gn_xcr_channel_enable[19U];
            _r1->reg._r36.bit.ch21_en = gn_xcr_channel_enable[20U];
            _r1->reg._r36.bit.ch22_en = gn_xcr_channel_enable[21U];
            _r1->reg._r36.bit.ch23_en = gn_xcr_channel_enable[22U];
            _r1->reg._r36.bit.ch24_en = gn_xcr_channel_enable[23U];
#endif
            _r1->reg._r36.bit.ch_size = XCR_CH_SIZE;
            _r1->reg._r36.bit.ld_width = XCR_LD_WIDTH;
            break;
        case XCR_FLLCNT11:
            _r1->reg._r37.bit.fll1cnt = 0U;
            break;
        case XCR_FLLCNT12:
            _r1->reg._r38.bit.fll1cnt = 0U;
            _r1->reg._r38.bit.fll1_err_range = 0U;
            _r1->reg._r38.bit.fll1_range = 0U;
            _r1->reg._r38.bit.fllsync = 0U;
            _r1->reg._r38.bit.fll1_en = 0U;
            break;
        case XCR_FLLCNT21:
            _r1->reg._r39.bit.fll2cnt = 0U;
            break;
        case XCR_FLLCNT22:
            _r1->reg._r3A.bit.fll2cnt = 0U;
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
            _r1->reg._r3D.bit.svo_on = 0U;
            break;
        case XCR_SVO1_OFF:
            _r1->reg._r3E.bit.svo1_off = 0U;
            break;
        case XCR_SVO2_OFF:
            _r1->reg._r3F.bit.svo2_off = 0U;
            break;
        case XCR_SVO3_OFF:
            _r1->reg._r40.bit.svo3_off = 0U;
            break;
        case XCR_SVO_NUMBER:
            _r1->reg._r41.bit.sv_no = 0U;
#if (XC_MODEL_TYPE == XC_TYPE_XC24R)
            _r1->reg._r41.bit.sv_no_type = 0U;
#elif (XC_MODEL_TYPE == XC_TYPE_IC603)
            _r1->reg._r41.bit.sv_no_multipier = 0U;
#endif
            break;
        case XCR_DAC_NF_CONTROL:
            _r1->reg._r42.bit.dgrjt_en = 0U;
#if (XC_MODEL_TYPE == XC_TYPE_XC24R)
            _r1->reg._r42.bit.bbkn_en = 0U;
            _r1->reg._r42.bit.bbkn_th = 0U;
#endif
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

        xcr24_set_xcr24_gr1_reg(addr, &_r1->ALL[addr], 1U);
    }

    gt_xcr24_otp_access.reg._rF0.bit.ADDR_EXT = 1U;
    xcr24_set_otp_control(XCR_TEST_CONTROL, &gt_xcr24_otp_access.ALL[XCR_TEST_CONTROL], 1U);
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

        xcr24_set_xcr24_gr2_reg(addr, &_r2->ALL[addr], 1U);
    }
    gt_xcr24_otp_access.reg._rF0.bit.ADDR_EXT = 0U;
    xcr24_set_otp_control(XCR_TEST_CONTROL, &gt_xcr24_otp_access.ALL[XCR_TEST_CONTROL], 1U);
}

void xcr24_reset(void)
{
    _v_reset_t _r00 = {0, };

    _r00.bit.rst1 = 1U;
    _r00.bit.rst2 = 0U;
    _r00.bit.rst3 = 0U;
    _r00.bit.vsync_rst_en1 = 0U;
    _r00.bit.vsync_rst_en2 = 0U;

    xcr24_set_xcr24_gr1_reg(XCR_RESET, &_r00.ALL, 1U);
}

void xcr24_read_all(void)
{
    xcr24_get_xcr24_gr1_reg(XCR_RESET +  0U, 56U);
    xcr24_get_xcr24_gr1_reg(XCR_RESET + 56U, 56U);

    gt_xcr24_otp_access.reg._rF0.bit.ADDR_EXT = 1U;
    xcr24_set_otp_control(XCR_TEST_CONTROL, &gt_xcr24_otp_access.ALL[XCR_TEST_CONTROL], 1U);
    xcr24_get_xcr24_gr2_reg(XCR_GRP2_DAC1_FB_VALID_CNT, 19U);
    gt_xcr24_otp_access.reg._rF0.bit.ADDR_EXT = 0U;
    xcr24_set_otp_control(XCR_TEST_CONTROL, &gt_xcr24_otp_access.ALL[XCR_TEST_CONTROL], 1U);
}

void xcr_init_param(void)
{
    gn_xcr_daisied_dev_ch_size = XDR_CH_LENGTH;

    /* XC24R channel enable */
    gn_xcr_channel_enable[0U] = 1U;
    gn_xcr_channel_enable[1U] = 1U;
    gn_xcr_channel_enable[2U] = 1U;
    gn_xcr_channel_enable[3U] = 1U;
    gn_xcr_channel_enable[4U] = 1U;
    gn_xcr_channel_enable[5U] = 1U;
    gn_xcr_channel_enable[6U] = 1U;
    gn_xcr_channel_enable[7U] = 1U;
    gn_xcr_channel_enable[8U] = 1U;
    gn_xcr_channel_enable[9U] = 1U;
    gn_xcr_channel_enable[10U] = 1U;
    gn_xcr_channel_enable[11U] = 1U;
    gn_xcr_channel_enable[12U] = 1U;
    gn_xcr_channel_enable[13U] = 1U;
    gn_xcr_channel_enable[14U] = 1U;
    gn_xcr_channel_enable[15U] = 1U;
    gn_xcr_channel_enable[16U] = 1U;
    gn_xcr_channel_enable[17U] = 1U;
    gn_xcr_channel_enable[18U] = 1U;
    gn_xcr_channel_enable[19U] = 1U;
    gn_xcr_channel_enable[20U] = 1U;
    gn_xcr_channel_enable[21U] = 1U;
    gn_xcr_channel_enable[22U] = 1U;
    gn_xcr_channel_enable[23U] = 1U;

    gn_xcr_channel_daisy_size[0U] = 1U;
    gn_xcr_channel_daisy_size[1U] = 1U;
    gn_xcr_channel_daisy_size[2U] = 1U;
    gn_xcr_channel_daisy_size[3U] = 1U;
    gn_xcr_channel_daisy_size[4U] = 1U;
    gn_xcr_channel_daisy_size[5U] = 1U;
    gn_xcr_channel_daisy_size[6U] = 1U;
    gn_xcr_channel_daisy_size[7U] = 1U;
    gn_xcr_channel_daisy_size[8U] = 1U;
    gn_xcr_channel_daisy_size[9U] = 1U;
    gn_xcr_channel_daisy_size[10U] = 1U;
    gn_xcr_channel_daisy_size[11U] = 1U;
    gn_xcr_channel_daisy_size[12U] = 1U;
    gn_xcr_channel_daisy_size[13U] = 1U;
    gn_xcr_channel_daisy_size[14U] = 1U;
    gn_xcr_channel_daisy_size[15U] = 1U;
    gn_xcr_channel_daisy_size[16U] = 1U;
    gn_xcr_channel_daisy_size[17U] = 1U;
    gn_xcr_channel_daisy_size[18U] = 1U;
    gn_xcr_channel_daisy_size[19U] = 1U;
    gn_xcr_channel_daisy_size[20U] = 1U;
    gn_xcr_channel_daisy_size[21U] = 1U;
    gn_xcr_channel_daisy_size[22U] = 1U;
    gn_xcr_channel_daisy_size[23U] = 1U;

    gn_xcr_channel_block_size[0U] = (gn_xcr_channel_daisy_size[0U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[1U] = (gn_xcr_channel_daisy_size[1U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[2U] = (gn_xcr_channel_daisy_size[2U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[3U] = (gn_xcr_channel_daisy_size[3U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[4U] = (gn_xcr_channel_daisy_size[4U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[5U] = (gn_xcr_channel_daisy_size[5U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[6U] = (gn_xcr_channel_daisy_size[6U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[7U] = (gn_xcr_channel_daisy_size[7U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[8U] = (gn_xcr_channel_daisy_size[8U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[9U] = (gn_xcr_channel_daisy_size[9U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[10U] = (gn_xcr_channel_daisy_size[10U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[11U] = (gn_xcr_channel_daisy_size[11U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[12U] = (gn_xcr_channel_daisy_size[12U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[13U] = (gn_xcr_channel_daisy_size[13U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[14U] = (gn_xcr_channel_daisy_size[14U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[15U] = (gn_xcr_channel_daisy_size[15U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[16U] = (gn_xcr_channel_daisy_size[16U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[17U] = (gn_xcr_channel_daisy_size[17U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[18U] = (gn_xcr_channel_daisy_size[18U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[19U] = (gn_xcr_channel_daisy_size[19U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[20U] = (gn_xcr_channel_daisy_size[20U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[21U] = (gn_xcr_channel_daisy_size[21U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[22U] = (gn_xcr_channel_daisy_size[22U] * gn_xcr_daisied_dev_ch_size);
    gn_xcr_channel_block_size[23U] = (gn_xcr_channel_daisy_size[23U] * gn_xcr_daisied_dev_ch_size);
}

void xcr24_init(void)
{
    xcr24_reset();

    xcr24_get_otp_control(XCR_TEST_CONTROL, 16U);

    xcr24_regs_init_table();

    //xcr24_read_all();
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
//const _xd12_regs_t* xcr24_get_xdr12_set_regs(void);
//ret_xdr12_regs xcr24_get_xdr12_get_regs(void);

void xcr24_get_otp_control(uint16_t addr, uint16_t len)
{
    uint16_t temp[64U + 3U] = { 0U, };  /* hdr + crc16 */
    uint16_t buffer[64U + 3U] = { 0U, };  /* pyload + crc16 */

    uint16_t length = 0U;
    _cmd_t cmd = { 0U, };

    uint16_t size = len;

    if((addr + len) >= XCR_OTP_MAX)
    {
        size = (XCR_OTP_MAX - addr);
    }
    if(size > 63U)
    {
        size = 63U;
    }

    length = 1U + 1U + size + 1U;   /* HDR + crc16 + readout + crc16 for readout */

    cmd.bit.code = CMD_CODE1;
    cmd.bit.addr = (OTP_BASE_ADDR + addr);
    cmd.bit.size = size;

    temp[0U] = cmd.ALL;
    temp[1U] = Calculate_CRC16_CCITT_False(&temp[0U], 1U);

    XCR_NSS_LO();
    uint8_t ret = spi_read(SPI1, temp, buffer, length, 20U);  /* hdr + crc16 + payload + crc16 */
    XCR_NSS_HI();

    if(1U == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "%s%s%s", ANSI_FONT_RED, "spi read timeout\r\n", ANSI_FONT_NONE);
    }
    else
    {
        uint16_t crc16 = 0U;
        crc16 = Calculate_CRC16_CCITT_False(&buffer[2U], size);

#if (0U != SPI_PACKET_DEBUG)
        {
            char str[128U] = { 0, };
            const uint16_t line_cnt = (uint16_t)((size + 15U) / 16U);

            comm_UART_Printf(LOG_LV_INFO, "\r\n %s(%x, %u) [%x, %x] %x, %u, %x, %4x, %4x\r\n", __func__, addr, len, temp[0U], temp[1U], crc16, length, buffer[length], buffer[0U], buffer[1U]);

            for(uint16_t line = 0U; line < line_cnt ; ++line)
            {
                uint16_t cnt = 0U;
                uint16_t start = (16U * line);
                uint16_t end = start + 16U;

                if(end > size)
                {
                    end = size;
                }

                for(uint16_t i = start; i < end; ++i)
                {
                    cnt += (uint16_t)snprintf(str + cnt, sizeof(str) - cnt, "%4X ", buffer[2U + i]);
                }
                comm_UART_Printf(LOG_LV_INFO, "%2x : %s\r\n", addr + (line * 16U), str);
            }
        }
#endif

        if(crc16 == buffer[length])
        {
            _xcr_otp_control_regs_t* _r = &gt_xcr24_otp_access;
            for(uint16_t i=0U ; i<size ; ++i)
            {
                _r->ALL[addr + i] = buffer[2U + i];
            }
        }
    }
}

void xcr24_set_otp_control(uint16_t addr, const uint16_t* q, uint16_t len)
{
    uint16_t temp[64U + 2U] = { 0U, };  /* hdr + pyload + crc16 */
    uint16_t crc16 = 0U;
    uint16_t length = 0U;
    _cmd_t cmd = { 0U, };

    uint16_t size = len;

    if((addr + len) >= XCR_OTP_MAX)
    {
        size = (XCR_OTP_MAX - addr);
    }
    if(size > 63U)
    {
        size = 63U;
    }

    cmd.bit.code = CMD_CODE2;
    cmd.bit.addr = (OTP_BASE_ADDR + addr);
    cmd.bit.size = size;

    temp[length++] = cmd.ALL;

    for(uint16_t i=0U ; i<size ; ++i)
    {
        temp[length++] = *q++;
    }

    crc16 = Calculate_CRC16_CCITT_False(temp, length);

    temp[length++] = crc16;

#if (0U != SPI_PACKET_DEBUG)
    {
        char str[128U] = { 0, };
        const uint16_t line_cnt = (uint16_t)((size + 15U) / 16U);

        comm_UART_Printf(LOG_LV_INFO, "\r\n %s(%x, %u) : %x, %x : %x\r\n", __func__, addr, len, temp[0U], crc16, temp[(length-1U)]);

        for(uint16_t line = 0U; line < line_cnt ; ++line)
        {
            uint16_t cnt = 0U;
            uint16_t start = (16U * line);
            uint16_t end = start + 16U;

            if(end > size)
            {
                end = size;
            }

            for(uint16_t i = 0; i < size; ++i)
            {
                cnt += (uint16_t)snprintf(str + cnt, sizeof(str) - cnt, "%4X ", temp[1U + i]);
            }
            comm_UART_Printf(LOG_LV_INFO, "%2x : %s\r\n", addr + (line * 16U), str);
        }
    }
#endif

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, temp, length, 20U);
    XCR_NSS_HI();

    if(1U == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "%s%s%s", ANSI_FONT_RED, "spi write timeout\r\n", ANSI_FONT_NONE);
    }
    else
    {
        _xcr_otp_control_regs_t* _r = &gt_xcr24_otp_access;
        for(uint16_t i=0U ; i<size ; ++i)
        {
            _r->ALL[addr + i] = temp[1U + i];
        }
    }
}

void xcr24_get_xcr24_gr1_reg(uint16_t addr, uint16_t len)
{
    uint16_t temp[64U + 3U] = { 0U, };  /* hdr + crc16 */
    uint16_t buffer[64U + 3U] = { 0U, };  /* pyload + crc16 */

    uint16_t length = 0U;
    _cmd_t cmd = { 0U, };

    uint16_t size = len;

    if((addr + len) >= XCR_GRP1_MAX)
    {
        size = (XCR_GRP1_MAX - addr);
    }
    if(size > 63U)
    {
        size = 63U;
    }

    length = 1U + 1U + size + 1U;   /* HDR + crc16 + readout + crc16 for readout */

    cmd.bit.code = CMD_CODE1;
    cmd.bit.addr = addr;
    cmd.bit.size = size;

    temp[0U] = cmd.ALL;
    temp[1U] = Calculate_CRC16_CCITT_False(&temp[0U], 1U);

    XCR_NSS_LO();
    uint8_t ret = spi_read(SPI1, temp, buffer, length, 20U);  /* hdr + crc16 + payload + crc16 */
    XCR_NSS_HI();

    if(1U == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "%s%s%s", ANSI_FONT_RED, "spi read timeout\r\n", ANSI_FONT_NONE);
    }
    else
    {
        uint16_t crc16 = 0U;
        crc16 = Calculate_CRC16_CCITT_False(&buffer[2U], size);

#if (0U != SPI_PACKET_DEBUG)
        {
            char str[128U] = { 0, };
            const uint16_t line_cnt = (uint16_t)((size + 15U) / 16U);

            comm_UART_Printf(LOG_LV_INFO, "\r\n %s(%x, %u) [%x, %x] %x, %u, %x, %4x, %4x\r\n", __func__, addr, len, temp[0U], temp[1U], crc16, length, buffer[length], buffer[0U], buffer[1U]);

            for(uint16_t line = 0U; line < line_cnt ; ++line)
            {
                uint16_t cnt = 0U;
                uint16_t start = (16U * line);
                uint16_t end = start + 16U;

                if(end > size)
                {
                    end = size;
                }

                for(uint16_t i = start; i < end; ++i)
                {
                    cnt += (uint16_t)snprintf(str + cnt, sizeof(str) - cnt, "%4X ", buffer[2U + i]);
                }
                comm_UART_Printf(LOG_LV_INFO, "%2x : %s\r\n", addr + (line * 16U), str);
            }
        }
#endif

        if(crc16 == buffer[length])
        {
            uint16_t* p = &gt_xcr24_get_gr1_regs.ALL[addr];
            for(uint16_t i = 0U ; i < size ; ++i)
            {
                *p++ = buffer[2U + i];
            }
        }
    }
}

void xcr24_get_xcr24_gr2_reg(uint16_t addr, uint16_t len)
{
    uint16_t temp[64U + 3U] = { 0U, };  /* hdr + crc16 */
    uint16_t buffer[64U + 3U] = { 0U, };  /* pyload + crc16 */

    uint16_t length = 0U;
    _cmd_t cmd = { 0U, };

    uint16_t size = len;

    if((addr + len) >= XCR_GRP2_MAX)
    {
        size = (XCR_GRP2_MAX - addr);
    }
    if(size > 63U)
    {
        size = 63U;
    }

    length = 1U + 1U + size + 1U;   /* HDR + crc16 + readout + crc16 for readout */

    cmd.bit.code = CMD_CODE1;
    cmd.bit.addr = addr;
    cmd.bit.size = size;

    temp[0U] = cmd.ALL;
    temp[1U] = Calculate_CRC16_CCITT_False(&temp[0U], 1U);

    XCR_NSS_LO();
    uint8_t ret = spi_read(SPI1, temp, buffer, length, 20U);  /* hdr + payload + crc16 */
    XCR_NSS_HI();

    if(1U == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "%s%s%s", ANSI_FONT_RED, "spi read timeout\r\n", ANSI_FONT_NONE);
    }
    else
    {
        uint16_t crc16 = 0U;
        crc16 = Calculate_CRC16_CCITT_False(&buffer[2U], size);

#if (0U != SPI_PACKET_DEBUG)
        {
            char str[128U] = { 0, };
            const uint16_t line_cnt = (uint16_t)((size + 15U) / 16U);

            comm_UART_Printf(LOG_LV_INFO, "\r\n %s(%x, %u) [%x, %x] %x, %u, %x, %4x, %4x\r\n", __func__, addr, len, temp[0U], temp[1U], crc16, length, buffer[length], buffer[0U], buffer[1U]);

            for(uint16_t line = 0U; line < line_cnt ; ++line)
            {
                uint16_t cnt = 0U;
                uint16_t start = (16U * line);
                uint16_t end = start + 16U;

                if(end > size)
                {
                    end = size;
                }

                for(uint16_t i = start; i < end; ++i)
                {
                    cnt += (uint16_t)snprintf(str + cnt, sizeof(str) - cnt, "%4X ", buffer[2U + i]);
                }
                comm_UART_Printf(LOG_LV_INFO, "%2x : %s\r\n", addr + (line * 16U), str);
            }
        }
#endif

        if(crc16 == buffer[length])
        {
            uint16_t* p = &gt_xcr24_get_gr2_regs.ALL[addr];
            for(uint16_t i=0U ; i<size ; ++i)
            {
                *p++ = buffer[2U + i];
            }
        }
    }
}

void xcr24_set_xcr24_gr1_reg(uint16_t addr, const uint16_t* q, uint16_t len)
{
    uint16_t temp[64U + 2U] = { 0U, };  /* hdr + pyload + crc16 */
    uint16_t crc16 = 0U;
    uint16_t length = 0U;
    _cmd_t cmd = { 0U, };

    uint16_t size = len;

    if((addr + len) >= XCR_GRP1_MAX)
    {
        size = (XCR_GRP1_MAX - addr);
    }
    if(size > 63U)
    {
        size = 63U;
    }

    cmd.bit.code = CMD_CODE2;
    cmd.bit.addr = addr;
    cmd.bit.size = size;

    temp[length++] = cmd.ALL;

    for(uint16_t i=0U ; i<size ; ++i)
    {
        temp[length++] = *q++;
    }

    crc16 = Calculate_CRC16_CCITT_False(temp, length);

    temp[length++] = crc16;

#if (0U != SPI_PACKET_DEBUG)
    {
        char str[128U] = { 0, };
        const uint16_t line_cnt = (uint16_t)((size + 15U) / 16U);

        comm_UART_Printf(LOG_LV_INFO, "\r\n %s(%x, %u) : %x, %x : %x\r\n", __func__, addr, len, temp[0U], crc16, temp[(length-1U)]);

        for(uint16_t line = 0U; line < line_cnt ; ++line)
        {
            uint16_t cnt = 0U;
            uint16_t start = (16U * line);
            uint16_t end = start + 16U;

            if(end > size)
            {
                end = size;
            }

            for(uint16_t i = 0; i < size; ++i)
            {
                cnt += (uint16_t)snprintf(str + cnt, sizeof(str) - cnt, "%4X ", temp[1U + i]);
            }
            comm_UART_Printf(LOG_LV_INFO, "%2x : %s\r\n", addr + (line * 16U), str);
        }
    }
#endif

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, temp, length, 20U);
    XCR_NSS_HI();

    if(1U == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "%s%s%s", ANSI_FONT_RED, "spi write timeout\r\n", ANSI_FONT_NONE);
    }
    else
    {
        _xcr_group1_regs_t* _r = &gt_xcr24_set_gr1_regs;
        for(uint16_t i=0U ; i<size ; ++i)
        {
            _r->ALL[addr + i] = temp[1U + i];
        }
    }
}

void xcr24_set_xcr24_gr2_reg(uint16_t addr, const uint16_t* q, uint16_t len)
{
    uint16_t temp[64U + 2U] = { 0U, };  /* hdr + pyload + crc16 */
    uint16_t crc16 = 0U;
    uint16_t length = 0U;
    _cmd_t cmd = { 0U, };

    uint16_t size = len;

    if((addr + len) >= XCR_GRP2_MAX)
    {
        size = (XCR_GRP2_MAX - addr);
    }
    if(size > 63U)
    {
        size = 63U;
    }

    cmd.bit.code = CMD_CODE2;
    cmd.bit.addr = addr;
    cmd.bit.size = size;

    temp[length++] = cmd.ALL;

    for(uint16_t i=0U ; i<size ; ++i)
    {
        temp[length++] = *q++;
    }

    crc16 = Calculate_CRC16_CCITT_False(temp, length);

    temp[length++] = crc16;

#if (0U != SPI_PACKET_DEBUG)
    {
        char str[128U] = { 0, };
        const uint16_t line_cnt = (uint16_t)((size + 15U) / 16U);

        comm_UART_Printf(LOG_LV_INFO, "\r\n %s(%x, %u) : %x, %x : %x\r\n", __func__, addr, len, temp[0U], crc16, temp[(length-1U)]);

        for(uint16_t line = 0U; line < line_cnt ; ++line)
        {
            uint16_t cnt = 0U;
            uint16_t start = (16U * line);
            uint16_t end = start + 16U;

            if(end > size)
            {
                end = size;
            }

            for(uint16_t i = 0; i < size; ++i)
            {
                cnt += (uint16_t)snprintf(str + cnt, sizeof(str) - cnt, "%4X ", temp[1U + i]);
            }
            comm_UART_Printf(LOG_LV_INFO, "%2x : %s\r\n", addr + (line * 16U), str);
        }
    }
#endif

    XCR_NSS_LO();
    uint8_t ret = spi_write(SPI1, temp, length, 20U);
    XCR_NSS_HI();

    if(1U == ret)
    {
        comm_UART_Printf(LOG_LV_ERROR, "%s%s%s", ANSI_FONT_RED, "spi write timeout\r\n", ANSI_FONT_NONE);
    }
    else
    {
        _xcr_group2_regs_t* _r = &gt_xcr24_set_gr2_regs;
        for(uint16_t i=0U ; i<size ; ++i)
        {
            _r->ALL[addr + i] = temp[1U + i];
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
    xcr24_set_xcr24_gr1_reg(XCR_LOCAL_RW_POINTER_RESET, &_r10.ALL, 1U);

    _r03.bit.addr = addr;
    _r03.bit.ch_seg = ch_seg;
    _r03.bit.enable = 1U;
    xcr24_set_xcr24_gr1_reg(XCR_LOCAL_READ_COMMAND, &_r03.ALL, 1U);

    retry = 128U;
    do
    {
        //us_delay(1000); /* TODO : ???*/
        xcr24_get_xcr24_gr1_reg(XCR_COMMAND_STATUS_1, 1U);
    }while((_r14->bit.local_r_doing == 1U) && --retry > 0U);

    retry = 128U;
    do
    {
        //us_delay(1000); /* TODO : ???*/
        xcr24_get_xcr24_gr1_reg(XCR_RECEIVE_STATUS, 1U);
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
        xcr24_set_xcr24_gr1_reg(XCR_LOCAL_RW_POINTER_RESET, &_r10.ALL, 1U);

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
        xcr24_set_xcr24_gr1_reg(XCR_LOCAL_WRITE_COMMAND, &_r02.ALL, 1U);

        do
        {
            //us_delay(1000); /* TODO : ???*/
            xcr24_get_xcr24_gr1_reg(XCR_COMMAND_STATUS_1, 1U);
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

    XCR_NSS_LO();
    uint8_t ret = spi_read(SPI1, spi_buffer, temp, (len + 1U + 1U), 20U);  /* hdr + crc16 + payload + crc16 */
    XCR_NSS_HI();

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

