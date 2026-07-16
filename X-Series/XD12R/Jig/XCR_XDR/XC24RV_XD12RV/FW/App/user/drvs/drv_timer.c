#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#include "drv_xdr12.h"
#include "drv_xcr24.h"
#include "comm_debugging.h"
#include "ldim_conversion.h"

#define SVSYNC_PHASE_GREEN      (1U)
#define SVSYNC_PHASE_BLUE       (0U)

#define SVSYNC_VSYNC_MASK_H_US  (4U)
#define SVSYNC_VSYNC_MASK_L_US  (4U)
#define SVSYNC_GATING_TIME_US   (10U)
#define SVSYNC_CYCLE            (2U)
#define SVSYNC_SIZE             (XDR_SV_NO)
#define SVSYNC_TOTAL_CYCLE      (SVSYNC_CYCLE * SVSYNC_SIZE)

#define SVSYNC_RED_FREQ         (11520U)
#define SVSYNC_GREEN_FREQ       (6400U)
#define SVSYNC_BLUE_FREQ        (9600U)

typedef struct tag_XD_RW_INFO
{
    uint16_t xd_rw_addr;
    uint16_t xd_rw_data;
    bool xd_rw_flag;
} xd_rw_info_t;

static bool gb_vsync_out_running;
static xd_rw_info_t gt_xd_read_info;
static xd_rw_info_t gt_xd_write_info;

static bool gb_vsync_out_flag;
static float gf_vsync_out_freq;
static uint8_t gn_svsync_count;

static float gf_svsync_sub_green_freq;
static uint32_t gn_svsync_sub_green_period;
static float gf_svsync_sub_blue_freq;
static uint32_t gn_svsync_sub_blue_period;
static uint32_t gn_svsync_sub_duty;

static bool gb_xcr_ldim_block_conversion_flag;
static uint16_t gn_xcr_ldim_block_conversion_index;

static inline void tim_update_vsync_out_freq(void)
{
    uint32_t AutoReload = LL_TIM_GetAutoReload(TIM8);
    uint32_t Prescaler = LL_TIM_GetPrescaler(TIM8);
    gf_vsync_out_freq = (float)(APB2_TIM_CLK) / ((float)(AutoReload + 1U) * (float)(Prescaler + 1U));
    xcr24_set_fll_cnt(0U, XCR_CONV_FREQ_TO_XCR_MCLK(gf_vsync_out_freq));
}

static inline void tim_update_svsync_out_freq(void)
{
    gf_svsync_sub_green_freq = SVSYNC_GREEN_FREQ;
    gf_svsync_sub_blue_freq = SVSYNC_BLUE_FREQ;

    gn_svsync_sub_green_period = TIM3_PERIOD_HZ(gf_svsync_sub_green_freq);
    gn_svsync_sub_blue_period = TIM3_PERIOD_HZ(gf_svsync_sub_blue_freq);

    gn_svsync_sub_duty = SVSYNC_OUT_PULSE(SVSYNC_GATING_TIME_US);
}

static void tim_svsync_timer_start(void)
{
    LL_TIM_SetCounter(TIM3, 0);
    LL_TIM_ClearFlag_UPDATE(TIM3);
    LL_TIM_SetAutoReload(TIM3, TIM3_PERIOD_HZ((CONST_HZ_TO_MHZ) /(SVSYNC_VSYNC_MASK_H_US + SVSYNC_VSYNC_MASK_L_US)));
    LL_TIM_OC_SetCompareCH1(TIM3, SVSYNC_OUT_PULSE(SVSYNC_VSYNC_MASK_H_US));
    LL_TIM_EnableCounter(TIM3);

    tim_update_svsync_out_freq();

    gn_svsync_count = 0U;
}

static void tim_svsync_timer_stop(void)
{
    LL_TIM_OC_SetCompareCH1(TIM3, 0U);
    LL_TIM_DisableCounter(TIM3);

    gn_svsync_count = 0U;
}

void tim_vsync_out_start(void)
{
    tim_update_vsync_out_freq();

    LL_TIM_OC_SetCompareCH2(TIM8, VSYNC_OUT_PULSE);
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_EnableCounter(TIM8);

    gb_vsync_out_running = true;
}

void tim_vsync_out_stop(void)
{
    LL_TIM_OC_SetCompareCH2(TIM8, 0U);
    LL_TIM_DisableCounter(TIM8);

    gb_vsync_out_running = false;
}

void tim_fllsync_start(void)
{
    FLLSYNC_ENABLE();
    LL_TIM_SetCounter(TIM4, 0U);
    LL_TIM_OC_SetCompareCH2(TIM4, FLLSYNC_OUT_PULSE);
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM4);
}

void tim_fllsync_stop(void)
{
    FLLSYNC_DISABLE();
    LL_TIM_DisableCounter(TIM4);
    LL_TIM_CC_DisableChannel(TIM4, LL_TIM_CHANNEL_CH1);
    LL_TIM_SetCounter(TIM4, 0U);
}

void tim_svsync_out_handler(void)
{
    uint32_t period = 0U;
    uint32_t CompareValue = 0U;

    ++gn_svsync_count;

    uint32_t phase = (gn_svsync_count % SVSYNC_CYCLE);
    //uint32_t phase = (gn_svsync_count & (SVSYNC_CYCLE - 1U));

    switch(phase)
    {
        case SVSYNC_PHASE_GREEN:
        {
            period = gn_svsync_sub_green_period;
            CompareValue = gn_svsync_sub_duty;
            break;
        }
        case SVSYNC_PHASE_BLUE:
        {
            period = gn_svsync_sub_blue_period;
            CompareValue = gn_svsync_sub_duty;
            break;
        }
        default:
        {
            FATAL_INVALID_INPUT(phase);
            break;
        }
    }

    LL_TIM_SetAutoReload(TIM3, period);
    LL_TIM_OC_SetCompareCH1(TIM3, CompareValue);

    if((SVSYNC_TOTAL_CYCLE + 0U) == gn_svsync_count)
    {
        us_delay(21U);
        tim_svsync_timer_stop();
    }
}

void tim_vsync_out_handler(void)
{
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
        xdr12_syncgen();
    #endif

    #if (XDR_SYNC_MODE == XDR_SYNC_MODE_SVI)
        tim_svsync_timer_start();
    #endif
/*

    uint16_t test_data = 0x8000U;
    xcr24_write_grp1_reg(XCR_LD_TRANSFER_COMMAND, &test_data, 1U);
*/
    gb_vsync_out_flag = true;
}

void tim_set_vsync_out_freq(float f)
{
    uint32_t AutoReload = TIM8_PERIOD_HZ(f);

    LL_TIM_SetAutoReload(TIM8, AutoReload);

    tim_update_vsync_out_freq();
}

void tim_vsync_out_process(void)
{
    if(true == gb_vsync_out_flag)
    {
        gn_xcr_ldim_block_conversion_index = 0U;
        gb_xcr_ldim_block_conversion_flag = true;
        gb_vsync_out_flag = false;
    }

    if(true == gb_xcr_ldim_block_conversion_flag)
    {
        block_color_t* p_block_color_table = ldim_get_block_color_buffer();
        ldim_conversion_block_to_ldim(gn_xcr_ldim_block_conversion_index, \
            p_block_color_table[gn_xcr_ldim_block_conversion_index].r, \
            p_block_color_table[gn_xcr_ldim_block_conversion_index].g, \
            p_block_color_table[gn_xcr_ldim_block_conversion_index].b);
        ++gn_xcr_ldim_block_conversion_index;
        if(LDIM_BLK_SIZE == gn_xcr_ldim_block_conversion_index)
        {
            xdr12_ld_transfer();
            gb_xcr_ldim_block_conversion_flag = false;
            gn_xcr_ldim_block_conversion_index = 0U;
        }
    }

    if (true == gt_xd_read_info.xd_rw_flag)
    {
        gt_xd_read_info.xd_rw_data = xdr12_read_by_type(gt_xd_read_info.xd_rw_addr, XD12R_ADDR_TYPE_GENERAL);
        comm_UART_Printf(LOG_LV_INFO, "\r\nXDIC Read --> [ 0x%02X - 0x%03X ]\r\n", gt_xd_read_info.xd_rw_addr, gt_xd_read_info.xd_rw_data);
        comm_UART_Printf(LOG_LV_INFO, "\n\rJIG> \0");
        gt_xd_read_info.xd_rw_flag = false;
    }

    if (true == gt_xd_write_info.xd_rw_flag)
    {
        xdr12_write_by_type(gt_xd_write_info.xd_rw_addr, gt_xd_write_info.xd_rw_data, XD12R_ADDR_TYPE_GENERAL);
        comm_UART_Printf(LOG_LV_INFO, "\n\rJIG> \0");
        gt_xd_write_info.xd_rw_flag = false;
    }
}

void tim_set_vsync_out_running_flag(bool running)
{
    gb_vsync_out_running = running;
}

bool tim_get_vsync_out_running_flag(void)
{
    return gb_vsync_out_running;
}

void tim_set_xd_read_in_vsync(uint16_t addr)
{
    gt_xd_read_info.xd_rw_addr = addr;
    gt_xd_read_info.xd_rw_flag = true;
}

void tim_set_xd_write_in_vsync(uint16_t addr, uint16_t data)
{
    gt_xd_write_info.xd_rw_addr = addr;
    gt_xd_write_info.xd_rw_data = data;
    gt_xd_write_info.xd_rw_flag = true;
}