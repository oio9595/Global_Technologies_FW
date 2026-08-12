#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#include "drv_xdr12.h"

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

typedef struct tag_RW_INFO
{
    uint16_t rw_addr;
    uint16_t rw_data;
    bool read_flag;
    bool write_flag;
    uint8_t addr_type;
} rw_info_t;

static bool gb_vsync_out_running;

static bool gb_xd_xc_rw_flag;
static rw_info_t gt_xd_rw_info;

static bool gb_vsync_out_flag;
static float gf_vsync_out_freq;
static uint8_t gn_svsync_count;

static float gf_svsync_sub_green_freq;
static uint32_t gn_svsync_sub_green_period;
static float gf_svsync_sub_blue_freq;
static uint32_t gn_svsync_sub_blue_period;
static uint32_t gn_svsync_sub_duty;

static bool gb_ldim_blk_xform_flag;
static uint16_t gn_ldim_blk_xform_idx;

static void tim_read_write_xd(void);

static inline void Vled_GB_Common_ON(void)
{
    VLED_EN_HI();
}

static inline void Vled_GB_Common_Off(void)
{
    VLED_EN_LO();
}

static inline void Vled_R_ON(void)
{
    VLED_R_EN_HI();
}

static inline void Vled_R_Off(void)
{
    VLED_R_EN_LO();
}

static inline void Vled_G_ON(void)
{
    VLED_G_SW_HI();
    VLED_G_OFF_HI();
}

static inline void Vled_G_Off(void)
{
    VLED_G_SW_LO();
    VLED_G_OFF_LO();
}

static inline void Vled_B_ON(void)
{
    VLED_B_SW_HI();
    VLED_B_OFF_HI();
}

static inline void Vled_B_Off(void)
{
    VLED_B_SW_LO();
    VLED_B_OFF_LO();
}

static inline void tim_update_vsync_out_freq(void)
{
    uint32_t AutoReload = LL_TIM_GetAutoReload(TIM8);
    uint32_t Prescaler = LL_TIM_GetPrescaler(TIM8);
    gf_vsync_out_freq = (float)(APB2_TIM_CLK) / ((float)(AutoReload + 1U) * (float)(Prescaler + 1U));
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

    Vled_R_ON();
    Vled_GB_Common_ON();

    gb_vsync_out_running = true;
}

void tim_vsync_out_stop(void)
{
    LL_TIM_OC_SetCompareCH2(TIM8, 0U);
    LL_TIM_DisableCounter(TIM8);

    gb_vsync_out_running = false;
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
            Vled_B_Off();
            Vled_G_ON();
            period = gn_svsync_sub_green_period;
            CompareValue = gn_svsync_sub_duty;
            break;
        }
        case SVSYNC_PHASE_BLUE:
        {
            Vled_G_Off();
            Vled_B_ON();
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
    xdr12_syncgen();
    tim_svsync_timer_start();
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
        gn_ldim_blk_xform_idx = 0U;
        gb_ldim_blk_xform_flag = true;
        gb_vsync_out_flag = false;
    }

    if(true == gb_ldim_blk_xform_flag)
    {
        DEBUG_HI();
        block_color_t* p_blk_color_tbl = ldim_get_block_color_buffer();
        ldim_conversion_block_to_ldim(gn_ldim_blk_xform_idx, p_blk_color_tbl[gn_ldim_blk_xform_idx].r, p_blk_color_tbl[gn_ldim_blk_xform_idx].g, p_blk_color_tbl[gn_ldim_blk_xform_idx].b);
        ++gn_ldim_blk_xform_idx;
        if(LDIM_BLK_SIZE == gn_ldim_blk_xform_idx)
        {
            xdr12_ld_transfer();
            gb_ldim_blk_xform_flag = false;
            gn_ldim_blk_xform_idx = 0U;
            gb_xd_xc_rw_flag = true;
        }
        DEBUG_LO();
    }

    if (true == gb_xd_xc_rw_flag)
    {
        tim_read_write_xd();

        gb_xd_xc_rw_flag = false;
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

static void tim_read_write_xd(void)
{
    if (true == gt_xd_rw_info.read_flag)
    {
        switch (gt_xd_rw_info.addr_type)
        {
            case XD12R_ADDR_TYPE_GENERAL:
            {
                gt_xd_rw_info.rw_data = xdr12_read_by_type(gt_xd_rw_info.rw_addr, XD12R_ADDR_TYPE_GENERAL);
                comm_UART_Printf(LOG_LV_INFO, "\r\nXDIC General Read --> [ 0x%02X - 0x%03X ]\r\n\n\rJIG> \0", gt_xd_rw_info.rw_addr, gt_xd_rw_info.rw_data);
                break;
            }
            case XD12R_ADDR_TYPE_MIRROR:
            {
                gt_xd_rw_info.rw_data = xdr12_read_by_type(gt_xd_rw_info.rw_addr, XD12R_ADDR_TYPE_MIRROR);
                comm_UART_Printf(LOG_LV_INFO, "\r\nXDIC Mirror Read --> [ 0x%02X - 0x%03X ]\r\n\n\rJIG> \0", gt_xd_rw_info.rw_addr, gt_xd_rw_info.rw_data);
                break;
            }
            default:
            {
                FATAL_INVALID_INPUT(gt_xd_rw_info.addr_type);
                break;
            }
        }
        gt_xd_rw_info.read_flag = false;
    }

    if (true == gt_xd_rw_info.write_flag)
    {
        switch (gt_xd_rw_info.addr_type)
        {
            case XD12R_ADDR_TYPE_GENERAL:
            {
                xdr12_write_by_type(gt_xd_rw_info.rw_addr, gt_xd_rw_info.rw_data, XD12R_ADDR_TYPE_GENERAL);
                comm_UART_Printf(LOG_LV_INFO, "\n\rOK\n\rJIG> \0");
                break;
            }
            case XD12R_ADDR_TYPE_MIRROR:
            {
                xdr12_write_by_type(gt_xd_rw_info.rw_addr, gt_xd_rw_info.rw_data, XD12R_ADDR_TYPE_MIRROR);
                comm_UART_Printf(LOG_LV_INFO, "\n\rOK\n\rJIG> \0");
                break;
            }
            default:
            {
                FATAL_INVALID_INPUT(gt_xd_rw_info.addr_type);
                break;
            }
        }
        gt_xd_rw_info.write_flag = false;
    }
}

void tim_set_xd_read_info(uint16_t addr, uint8_t addr_type)
{
    gt_xd_rw_info.rw_addr = addr;
    gt_xd_rw_info.read_flag = true;
    gt_xd_rw_info.addr_type = addr_type;
}

void tim_set_xd_write_info(uint16_t addr, uint16_t data, uint8_t addr_type)
{
    gt_xd_rw_info.rw_addr = addr;
    gt_xd_rw_info.rw_data = data;
    gt_xd_rw_info.write_flag = true;
    gt_xd_rw_info.addr_type = addr_type;
}