/*
 * File:   XD12R_ES1_struct.h
 * Author: GT
 *
 * Created on 2026, 05, 04
 * Revision : XD12R/XD04R ES0
 */

#ifndef _XD12R_ES1_STRUCT_H_
#define	_XD12R_ES1_STRUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define XD12R_INTERNAL_MCLK (50000000UL)    /* 50.0MHz */
#define XDR_MODEL_NAME      "XDR12D (ES1)"

/* Register Map */
typedef enum _xd12r_addr_
{
    XD12R_RESET_ID = 0,         /* 0x00 */
    XD12R_LD_CONTROL,           /* 0x01 */
    XD12R_LD_SIZE,              /* 0x02 */
                                /* 0x03, TBD */
                                /* 0x04, TBD */
    XD12R_CHANNEL_ENABLE = 5,   /* 0x05 */
    XD12R_FAULT_CONTROL,        /* 0x06 */
    XD12R_FB_LEVEL,             /* 0x07 */
    XD12R_SHORT_LEVEL,          /* 0x08 */
    XD12R_FAULT_STATUS0,        /* 0x09 */
    XD12R_MAX_CURR_VREF1,       /* 0x0A */
    XD12R_MAX_CURR_VREF2,       /* 0x0B */
    XD12R_MAX_CURR_VREF3,       /* 0x0C */
    XD12R_MAX_CURR_VREF4,       /* 0x0D */
    XD12R_MAX_CURR_VREF5,       /* 0x0E */
                                /* 0x0F : TBD */
    XD12R_DELAY_CH1_2 = 0x10,   /* 0x10 */
    XD12R_DELAY_CH3_4,          /* 0x11 */
    XD12R_DELAY_CH5_6,          /* 0x12 */
    XD12R_DELAY_CH7_8,          /* 0x13 */
    XD12R_DELAY_CH9_10,         /* 0x14 */
    XD12R_DELAY_CH11_12,        /* 0x15 */
    XD12R_FB_LEVEL,             /* 0x16 */
    XD12R_FB_SHORT_LEVEL,       /* 0x17 */
    XD12R_SHORT_LEVEL,          /* 0x18 */
    XD12R_MAX_CURRENT_LEVEL1,   /* 0x19 */
    XD12R_MAX_CURRENT_LEVEL2,   /* 0x1A */
    XD12R_PARITY_RD_EN,         /* 0x1B */
    XD12R_SERIAL_CLK_GEN,       /* 0x1C */
    XD12R_SERIAL_LATENCY,       /* 0x1D */
    XD12R_V_MASK,               /* 0x1E */
    XD12R_SV_MASK,              /* 0x1F */

    XD12R_RSTCNT = 0x20,        /* 0x20 */
    XD12R_TIMEOUT,              /* 0x21 */
    XD12R_FLLCNT1,              /* 0x22 */
    XD12R_FLLCNT2,              /* 0x23 */
    XD12R_WR_PROTECT,           /* 0x24 */
    XD12R_NF_CONTROL,           /* 0x25 */
    XD12R_CHOP_EN,              /* 0x26 */
    XD12R_TEMP,                 /* 0x27 */
    XD12R_OSC_FLL_MAN1,         /* 0x28 */
    XD12R_OSC_FLL_MAN2,         /* 0x29 */
    XD12R_OSC_SPREAD,           /* 0x2A */
    XD12R_CLOCK_GATE_EN,        /* 0x2B */

    XD12R_MAX,
}xd12_addr_t;

#define XD12R_OTP_CTRL_BASE     (0x3A)
typedef enum _xd12r_opt_ctrl_addr_
{
    XD12R_OTP_ACCESS_B15_B12 = 0U,  /* 0x3A */
    XD12R_OTP_ACCESS_B11_B0,        /* 0x3B */
    XD12R_OTP_WRITE,                /* 0x3C */
    XD12R_OTP_RD_PROG,              /* 0x3D */
    XD12R_OTP_PROTECT,              /* 0x3E */
    XD12R_OP_MODE,                  /* 0x3F */

    XD12R_OTP_MAX,
}xd12_otp_ctrl_addr_t;

typedef enum _max_curr_level_
{
    CURR_LEVEL_4 = 0U,  /* max 4mA for XD12, max 8mA for XD04 */
    CURR_LEVEL_8,       /* max 8mA for XD12, max 16mA for XD04 */
    CURR_LEVEL_12,      /* max 12mA for XD12, max 24mA for XD04 */
    CURR_LEVEL_16,      /* max 16mA for XD12, max 32mA for XD04 */
    CURR_LEVEL_24,      /* max 24mA for XD12, max 48mA for XD04 */
    CURR_LEVEL_32,      /* max 32mA for XD12, max 64mA for XD04 */
    CURR_LEVEL_46,      /* max 46mA for XD12, max 92mA for XD04 */
    CURR_LEVEL_64,      /* max 64mA for XD12, max 128mA for XD04 */
    CURR_LEVEL_MAX,
}max_curr_level_t;

typedef enum _short_level_
{
    SHORT_LEVEL_3 = 0U, /* above 3V for XD12, above 6V for XD04 */
    SHORT_LEVEL_4,      /* above 4V for XD12, above 8V for XD04 */
    SHORT_LEVEL_6,      /* above 6V for XD12, above 12V for XD04 */
    SHORT_LEVEL_8,      /* above 8V for XD12, above 16V for XD04 */
    SHORT_LEVEL_12,     /* above 12V for XD12, above 24V for XD04 */
    SHORT_LEVEL_16,     /* above 16V for XD12, above 32V for XD04 */
    SHORT_LEVEL_24,     /* above 24V for XD12, above 48V for XD04 */
    SHORT_LEVEL_36,     /* above 36V for XD12, above 70V for XD04 */
    SHORT_LEVEL_MAX,
}short_level_t;

typedef enum _fb_level_
{
    FB_LEVEL_4 = 0U,    /* below 0.40V for XD12, below 0.40V for XD04 */
    FB_LEVEL_5,         /* below 0.50V for XD12, below 0.60V for XD04 */
    FB_LEVEL_6,         /* below 0.60V for XD12, below 0.80V for XD04 */
    FB_LEVEL_7,         /* below 0.70V for XD12, below 1.00V for XD04 */
    FB_LEVEL_8,         /* below 0.85V for XD12, below 1.20V for XD04 */
    FB_LEVEL_10,        /* below 1.00V for XD12, below 1.40V for XD04 */
    FB_LEVEL_11,        /* below 1.15V for XD12, below 1.60V for XD04 */
    FB_LEVEL_13,        /* below 1.30V for XD12, below 1.80V for XD04 */
    FB_LEVEL_MAX,
}fb_level_t;

#define FLL_ENABLE          (0U)
#define FLL_DISABLE         (1U)

#define XD12R_REG_PROTECT_DISABLE    (0x155U)
#define XD12R_REG_PROTECT_ENABLE     (0x2AAU)

/* RESET&ID(0x00) : default 0x000 */
typedef union tag_XD12R_RESET_ID
{
    uint16_t ALL;
    struct
    {
        uint16_t id     :  5;   /* not writable from a host, ID auto-write #1~#31 ('0' invalid) */
        uint16_t        :  3;   /* reserved */
        uint16_t lkg_e  :  1;   /* ch leakage mode en (default = enabled ; typ 100nA) */
        uint16_t e_rst  :  1;   /* SER/DESER reset @ last svsync and rstcnt expired */
        uint16_t vs_rst :  1;   /* both cmd and svi(=vsync) signal ; SER/DESER reset @ vsync */
        uint16_t rst    :  1;   /* all logics reset, auto-cleared, but the otp value is not downloaded */
        uint16_t        :  4;   /* reserved */
    }bit;
}_v_xdr12_reset_id_t;

/* LD_CONTROL(0x01) : default 0x000 */
typedef union tag_XD12R_LD_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_mode    : 1;    /* '0' normal,  '1' x8 (repeat 8 times the same dimming pattern, SV_NO=8) */
        uint16_t ld_dir     : 1;    /* '0' Head Lighting, '1' Tail Lighting */
        uint16_t ld_res     : 1;    /* '0' 12-bit LD, '1' 14-bit LD */
        uint16_t syncmode   : 1;    /* '0' cmd+svi,  '1' vo+svi */
        uint16_t delay_ch_en: 1;    /*  */
        uint16_t sv_no      : 6;    /* '8' means total 16 times, (g=8, b=8) */
        uint16_t            : 5;    /* reserved */
    }bit;
}_v_xdr12_ld_control_t;

/* LD_SIZE(0x02) : default 0x000 */
typedef union tag_XD12R_LD_SIZE
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_size    :  5;   /* the number of LD data to be transmitted */
        uint16_t            : 11;   /* reserved */
    }bit;
}_v_xdr12_ld_size_t;

/* PWMCLK_DIV1/2(0x03) : default 0x000 */
typedef union tag_XD12R_PWMCLK_DIV1
{
    uint16_t ALL;
    struct
    {
        uint16_t  pwmclk_div1   : 8;    /* counted by a mclk, the value updated at every vsync */
        uint16_t  pwmclk_div2   : 4;    /* [3:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_xdr12_pwmclk_div12_t;

/* PWMCLK_DIV2/3(0x04) : default 0x000 */
typedef union tag_XD12R_PWMCLK_DIV2
{
    uint16_t ALL;
    struct
    {
        uint16_t  pwmclk_div2   : 4;    /* [7:4] */
        uint16_t  pwmclk_div3   : 8;
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_xdr12_pwmclk_div23_t;

/* CHANNEL ENABLE(0x05) : default 0x000 */
typedef union tag_XD12R_CHANNEL_ENABLE
{
    uint16_t ALL;
    struct
    {
        uint16_t  ch1_en        : 1;
        uint16_t  ch2_en        : 1;
        uint16_t  ch3_en        : 1;
        uint16_t  ch4_en        : 1;
        uint16_t  ch5_en        : 1;
        uint16_t  ch6_en        : 1;
        uint16_t  ch7_en        : 1;
        uint16_t  ch8_en        : 1;
        uint16_t  ch9_en        : 1;
        uint16_t  ch10_en       : 1;
        uint16_t  ch11_en       : 1;
        uint16_t  ch12_en       : 1;
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_xdr12_channel_enable_t;

/* FAULT_CONTROL(0x06) : default 0x000 */
typedef union tag_XD12R_FAULT_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t o_off_e            : 1;    /* O_OFF_E :  */
        uint16_t s_off_e            : 1;    /* S_OFF_E :  */
        uint16_t t_off_e            : 1;    /* T_OFF_E :  */
        uint16_t o_det_e            : 1;    /* S_DET_E :  */
        uint16_t s_det_e            : 1;    /* O_DET_E :  */
        uint16_t o_fb_e             : 1;    /* O_FB_E :  */
        uint16_t fb_mode            : 4;    /* MS_VS_DET_E :  */
        uint16_t auto_fault_fb_no   : 2;    /* MS_VS_DIMM :  */
        uint16_t                    : 4;    /* reserved */
    }bit;
}_v_xdr12_fault_control_t;

/* CHx_LD_TYPE0(0x07) : default 0x000 */
typedef union tag_XD12R_CHx_LD_TYPE0
{
    uint16_t ALL;
    struct
    {
        uint16_t ch7_ld_type        : 2;
        uint16_t ch8_ld_type        : 2;
        uint16_t ch9_ld_type        : 2;
        uint16_t ch10_ld_type       : 2;
        uint16_t ch11_ld_type       : 2;
        uint16_t ch12_ld_type       : 2;
        uint16_t                    : 4;    /* reserved */
    }bit;
}_v_xdr12_ch_ld_type0_t;

/* CHx_LD_TYPE1(0x08) : default 0x000 */
typedef union tag_XD12R_CHx_LD_TYPE1
{
    uint16_t ALL;
    struct
    {
        uint16_t ch1_ld_type        : 2;
        uint16_t ch2_ld_type        : 2;
        uint16_t ch3_ld_type        : 2;
        uint16_t ch4_ld_type        : 2;
        uint16_t ch5_ld_type        : 2;
        uint16_t ch6_ld_type        : 2;
        uint16_t                    : 4;    /* reserved */
    }bit;
}_v_xdr12_ch_ld_type1_t;

/* FAULT_STATUS0(0x09) : read-only, auto-cleared just after a readout */
typedef union tag_XD12R_FAULT_STATUS0
{
    uint16_t ALL;
    struct
    {
        uint16_t bit_fb1        :  1;
        uint16_t bit_fb2        :  1;
        uint16_t bit_fb3        :  1;
        uint16_t bit_open       :  1;
        uint16_t bit_short      :  1;
        uint16_t bit_thermal    :  1;
        uint16_t bit_ldo_uv     :  1;
        uint16_t bit_ldo_ov     :  1;
        uint16_t bit_vdd_uv     :  1;
        uint16_t bit_vdd_ov     :  1;
        uint16_t bit_parity_err :  1;
        uint16_t bit_timeout_err:  1;
        uint16_t                :  4;    /* reserved */
    }bit;
}_v_xdr12_fault_status0_t;

/* MAX_CURRENT_VREF1~5(0x0A~0x0E) : default 0x000, the value updated at every vsync */
typedef union tag_XD12R_MAX_CURRENT_VREF
{
    uint16_t ALL;
    struct
    {
        uint16_t max_curr_vref  : 12;   /* The MAX_CURR_VREF determines the maximum current value of a frame to be dimmed. If the max current level is set to 8mA, then the MAX_CURR_VREF value is proportional to the max current level, and so this value decides the max current reference of all blocks being dimmed for a frame. The setting value will come into effect at the next vsync assertion even if the value is set during current dimming frame earlier than the next vsync. */
        uint16_t                :  4;   /* reserved */
    }bit;
}_v_xdr12_max_curr_vref_t;

/* DELAY_CH1/2 ~ 11/12(0x10 ~ 0x15) : default 0x000 */
typedef union tag_XD12R_DELAY_CHx
{
    uint16_t ALL;
    struct
    {
        uint16_t delay_ch1  :  5;    /* DELAY_CHx [4:0] */
        uint16_t delay_ch2  :  5;    /* DELAY_CHx [4:0] */
        uint16_t            :  4;    /* reserved */
    }bit;
}_v_xdr12_delay_chx_t;

/* FB_LEVEL(0x16) : default 0x000 */
typedef union tag_XD12R_FB_LEVEL
{
    uint16_t ALL;
    struct
    {
        uint16_t fb1_level          : 3;
        uint16_t fb2_level          : 3;
        uint16_t fb3_level          : 3;
        uint16_t fb4_level          : 3;
        uint16_t                    : 4;    /* reserved */
    }bit;
}_v_xdr12_fb_level_t;

/* SHORT_LEVEL(0x17) : default 0x000 */
typedef union tag_XD12R_FB_SHORT_LEVEL
{
    uint16_t ALL;
    struct
    {
        uint16_t fb5_level          : 3;
        uint16_t short1_level       : 3;
        uint16_t short2_level       : 3;
        uint16_t short3_level       : 3;
        uint16_t                    : 4;    /* reserved */
    }bit;
}_v_xdr12_fb_short_level_t;

/* SHORT_LEVEL(0x18) : default 0x000 */
typedef union tag_XD12R_SHORT_LEVEL
{
    uint16_t ALL;
    struct
    {
        uint16_t short4_level       : 3;
        uint16_t short5_level       : 3;
        uint16_t                    : 7;    /* reserved */
    }bit;
}_v_xdr12_short_level_t;

/* MAX CURRENT LEVEL0(0x19) : default 0x000 */
typedef union tag_XD12R_MAX_CURRENT_LEVEL1

{
    uint16_t ALL;
    struct
    {
        uint16_t max_curr1_level    : 4;
        uint16_t max_curr2_level    : 4;
        uint16_t max_curr3_level    : 4;
        uint16_t                    : 4;    /* reserved */
    }bit;
}_v_xdr12_max_current_level1_t;

/* MAX CURRENT LEVEL1(0x1A) : default 0x000 */
typedef union tag_XD12R_MAX_CURRENT_LEVEL2

{
    uint16_t ALL;
    struct
    {
        uint16_t max_curr4_level    : 4;
        uint16_t max_curr5_level    : 4;
        uint16_t                    : 8;    /* reserved */
    }bit;
}_v_xdr12_max_current_level2_t;

/* PARITY & RD_EN(0x1B) : default 0x000 */
typedef union tag_XD12R_PARITY_RD_EN
{
    uint16_t ALL;
    struct
    {
        uint16_t reg_rd_en  :  1;
        uint16_t            : 10;
        uint16_t parity_e   :  1;
        uint16_t            :  4;    /* reserved */
    }bit;
}_v_xdr12_parity_rd_en_t;

/* SERIAL CLK GEN(0x1C) : default 0x318 */
typedef union tag_XD12R_SERIAL_CLK_GEN
{
    uint16_t ALL;
    struct
    {
        uint16_t serial_clk_high    : 6;    /* These bits determine the number of MCLK of VDD level in logic high value ‘1’ or the number of MCLK of 0 level in logic low value ‘0’ when the XD12 is driving the DIO pin on read-back.  For more details, refer to Table. 4, Logic high time. */
        uint16_t serial_clk_low     : 6;    /* These bits determine the number of MCLK of VDD level in logic low value ‘0’ or the number of MCLK of 0 level in logic high value ‘1’ when the XD12 is driving the DIO pin on read-back. For more details, refer to Table. 4, Logic low time. */
        uint16_t                    : 4;    /* reserved */
    }bit;
}_v_xdr12_serial_clk_gen_t;

/* SERIAL_LATENCY(0x1D) : default 0x080, counted by a mclk/2 */
typedef union tag_XD12R_SERIAL_LATENCY
{
    uint16_t ALL;
    struct
    {
        uint16_t serial_latency :  9;
        uint16_t                :  7;    /* reserved */
    }bit;
}_v_xdr12_serial_latency_t;

/* V_MASK(0x1E) : default 0x000 */
typedef union tag_XD12R_V_MASK
{
    uint16_t ALL;
    struct
    {
        uint16_t v_mask         : 10;    /* counted by a mclk/2, setting value must consider a frame frequency transition */
        uint16_t                :  6;    /* reserved */
    }bit;
}_v_xdr12_v_mask_t;

/* SV_MASK(0x1F) : default 0x000 */
typedef union tag_XD12R_sV_MASK
{
    uint16_t ALL;
    struct
    {
        uint16_t sv_mask        : 10;    /* counted by a mclk/2, setting value must consider a frame frequency transition */
        uint16_t                :  1;    /* reserved */
        uint16_t sv_mask_en     :  1;
        uint16_t                :  4;    /* reserved */
    }bit;
}_v_xdr12_sv_mask_t;

/* RSTCNT(0x20) : default 0x000 */
typedef union tag_XD12R_RSTCNT
{
    uint16_t ALL;
    struct
    {
        uint16_t rstcnt         : 10;    /* counted by a mclk/64 from a last svsync, reset at every vsync */
        uint16_t                :  6;    /* reserved */
    }bit;
}_v_xdr12_rstcnt_t;

/* TIMEOUT(0x21) : default 0x3FF */
typedef union tag_XD12R_TIMEOUT
{
    uint16_t ALL;
    struct
    {
        uint16_t timeout        : 11;    /* counted by a mclk/64 */
        uint16_t                :  5;    /* reserved */
    }bit;
}_v_xdr12_timeout_t;

/* FLLCNT1(0x22) : default 0x000 */
typedef union tag_XD12R_FLLCNT1
{
    uint16_t ALL;
    struct
    {
        uint16_t fllcnt         : 12;    /* counted by a mclk/2 */
        uint16_t                :  4;    /* reserved */
    }bit;
}_v_xdr12_fllcnt1_t;

/* FLLCNT2(0x23) : default 0xC32 */
typedef union tag_XD12R_FLLCNT2
{
    uint16_t ALL;
    struct
    {
        uint16_t fllcnt         :  8;   /* counted by a mclk/2 */
        uint16_t                :  1;
        uint16_t fll_range      :  2;
        uint16_t fll_en         :  1;   /* '0 disable, '1' enable (polarity changed) the value updated at every vsync, not svsync */
        uint16_t                :  4;   /* reserved */
    }bit;
}_v_xdr12_fllcnt2_t;

/* WR PROTECT(0x24) : default 0x555 */
typedef union tag_XD12R_WR_PROTECT
{
    uint16_t ALL;
    struct
    {
        uint16_t wr_protect     : 12;    /* 0xAAA : MAX_CURR_VREF1/2/3 만 writable, 0xCCC : LD_CONTROL, FLLCNT1/2, PWMCLK_DIV1/2/3, MAX_CURR_VREF1/2/3 만 writable, 0x555 : all reg writable,  others : un-writable, WR_PROTECT reg always writable */
        uint16_t                :  4;    /* reserved */
    }bit;
}_v_xd12_wr_protect_t;

/* NF CONTROL(0x25) : default 0x000 */
typedef union tag_XD12R_NF_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t DGRJT_EN1      :  1;    /* '1' 1 mclk data glitch rejection enable */
        uint16_t DGRJT_EN2      :  1;    /* '1' 2 mclk data glitch rejection enable */
        uint16_t BBKN_EN        :  1;    /* '1' bit broken detection enable --> DIO/ DDIO 신호 모두 적용 */
        uint16_t SGRJT_EN1      :  1;    /* '1' 3 mclk svi glitch rejection enable */
        uint16_t SGRJT_EN2      :  1;    /* '1' 4 mclk svi glitch rejection enable */
        uint16_t O_EMI_REJ_EN   :  1;
        uint16_t BBKN_TH        :  6;
        uint16_t                :  4;    /* reserved */
    }bit;
}_v_xdr12_nf_control_t;

/* CHOP_EN(0x26) : default 0x000 */
typedef union tag_XD12R_CHOP_EN
{
    uint16_t ALL;
    struct
    {
        uint16_t CHOP_BGR_EN    :  1;
        uint16_t CHOP_DAC_EN    :  1;
        uint16_t CHOP_OSC_EN    :  1;
        uint16_t CHOP_OSCLDO_EN :  1;
        uint16_t CHOP_DRV_EN    :  1;
        uint16_t CHOP_EN        :  1;
        uint16_t                : 10;   /* reserved */
    }bit;
}_v_xdr12_chop_en_t;

/* TEMP(0x27) : default 0x829 */
typedef union tag_XD12R_TEMP
{
    uint16_t ALL;
    struct
    {
        uint16_t flt_gain       :  2;   /* It is recommended to use the default value. */
        uint16_t o_slew         :  2;
        uint16_t flt_ctl        :  2;   /* It is recommended to set to 1. */
        uint16_t dac_rng        :  1;   /* It is recommended to use the default value. */
        uint16_t ov_swap_en     :  1;   /* reserved */
        uint16_t ofs_temp       :  4;   /* This determines the temperature level at which the thermal event occurs. The thermal threshold temperature may vary slightly from the set value. The thermal event threshold temperature according to the OFS_TEMP value is mapped as follows. 96/ 102/ 107/ 112/ 117/ 122/ 127/ 132/ 137(default) / 142/ 147/ 151/ 157/ 161/ 166/ 171℃ */
        uint16_t                :  4;   /* reserved */
    }bit;
}_v_xdr12_temp_t;

/* OSC_FLL_MAN1(0x28) : default 0x000 */
typedef union tag_XD12R_OSC_FLL_MAN1
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_fll_man    : 12;    /* OSC_FLL_MAN[11:0] : This determines the FLL target frequency value in manual mode. */
        uint16_t                :  4;    /* reserved */
    }bit;
}_v_xdr12_osc_fll_man1_t;

/* OSC_FLL_MAN2(0x29) : default 0x808 */
typedef union tag_XD12R_OSC_FLL_MAN2
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_fll_man        :  4;    /* OSC_FLL_MAN[15:12] : This determines the FLL target frequency value in manual mode. */
        uint16_t osc_fll_err_range  :  2;
        uint16_t                    :  5;    /* reserved */
        uint16_t osc_man_en         :  1;    /* Enable the manual mode of the FLL module. It is careful that this bit must be cleared during the initialization after POR. */
        uint16_t                    :  4;    /* reserved */
    }bit;
}_v_xdr12_osc_fll_man2_t;

/* OSC SPREAD(0x2A) : default 0x000 */
typedef union tag_XD12R_OSC_SPREAD
{
    uint16_t ALL;
    struct
    {
        uint16_t SPRD_GAIN    :  3;
        uint16_t              :  1;    /* reserved */
        uint16_t SPRD_SPD     :  3;
        uint16_t              :  4;    /* reserved */
        uint16_t SPRD_EN      :  1;
        uint16_t              :  4;    /* reserved */
    }bit;
}_v_xdr12_osc_spread_t;

/* CLOCK GATE EN(0x2B) : default 0x017, for test only */
typedef union tag_XD12R_CLOCK_GATE_EN
{
    uint16_t ALL;
    struct
    {
        uint16_t DC_MCLK_EN   :  1;
        uint16_t FR1_MCLK_EN  :  1;
        uint16_t FR2_MCLK_EN  :  1;
        uint16_t              :  1;    /* reserved */
        uint16_t OTP_MCLK_EN  :  1;
        uint16_t              : 11;    /* reserved */
    }bit;
}_v_xdr12_clock_gate_en_t;

/* OTP_ACCESS_1 (0x3A) : default 0x000 */
typedef union tag_XDR12_OTP_ACCESS_1
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_acc_cycle   :  4;
        uint16_t                    : 12;
    }bit;
}_v_xdr12_otp_access_1_t;

/* OTP_ACCESS_2 (0x3B) : default 0x3FF */
typedef union tag_XDR12_OTP_ACCESS_2
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_acc_cycle   : 12;
        uint16_t                    :  4;
    }bit;
}_v_xdr12_otp_access_2_t;

/* OTP_WRITE (0x3C) : default 0x004 */
typedef union tag_XDR12_OTP_WRITE
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_wsel   :  4;
        uint16_t otp_rd     :  2;
        uint16_t            : 10;
    }bit;
}_v_xdr12_otp_write_t;

/* OTP_RD_PROG (0x3D) : default 0x004 */
typedef union tag_XDR12_OTP_RD_PROG
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_s   :  1;
        uint16_t otp_rd_s   :  1;
        uint16_t            : 14;
    }bit;
}_v_xdr12_otp_rd_prog_t;

/* OTP_PROTECT (0x3E) : default 0x5A5 */
typedef union tag_XDR12_OTP_PROTECT
{
    uint16_t ALL;
    struct
    {
        uint16_t protect_en : 12;
        uint16_t            :  4;
    }bit;
}_v_xdr12_otp_protect_t;

/* OP_MODE (0x3F) : default 0x000 */
typedef union tag_XDR12_OP_MODE
{
    uint16_t ALL;
    struct
    {
        uint16_t addr_ext       :  1;
        uint16_t ext_clkin      :  1;
        uint16_t dtest_mux_en   :  1;
        uint16_t mclk64_o       :  1;
        uint16_t pwm_out_full   :  1;
        uint16_t sw_sel         :  3;
        uint16_t test_ana_en    :  3;
        uint16_t test_en        :  1;
        uint16_t                :  4;
    }bit;
}_v_xdr12_op_mode_t;

/* DUMMY */
typedef union tag_XD12R_DUMMY
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy      : 16;
    }bit;
}_v_xdr12_dummy_t;

typedef union _xdr12_regs
{
    uint16_t ALL[XD12R_MAX];
    struct
    {
        _v_xdr12_reset_id_t             _r00;
        _v_xdr12_ld_control_t           _r01;
        _v_xdr12_ld_size_t              _r02;
        _v_xdr12_pwmclk_div12_t         _r03;
        _v_xdr12_pwmclk_div23_t         _r04;
        _v_xdr12_channel_enable_t       _r05;
        _v_xdr12_fault_control_t        _r06;
        _v_xdr12_ch_ld_type0_t          _r07;
        _v_xdr12_ch_ld_type1_t          _r08;
        _v_xdr12_fault_status0_t        _r09;
        _v_xdr12_max_curr_vref_t        _r0A;
        _v_xdr12_max_curr_vref_t        _r0B;
        _v_xdr12_max_curr_vref_t        _r0C;
        _v_xdr12_max_curr_vref_t        _r0D;
        _v_xdr12_max_curr_vref_t        _r0E;
        _v_xdr12_dummy_t                _r0F;
        _v_xdr12_delay_chx_t            _r10;
        _v_xdr12_delay_chx_t            _r11;
        _v_xdr12_delay_chx_t            _r12;
        _v_xdr12_delay_chx_t            _r13;
        _v_xdr12_delay_chx_t            _r14;
        _v_xdr12_delay_chx_t            _r15;
        _v_xdr12_fb_level_t             _r16;
        _v_xdr12_fb_short_level_t       _r17;
        _v_xdr12_short_level_t          _r18;
        _v_xdr12_max_current_level1_t   _r19;
        _v_xdr12_max_current_level2_t   _r1A;
        _v_xdr12_parity_rd_en_t         _r1B;
        _v_xdr12_serial_clk_gen_t       _r1C;
        _v_xdr12_serial_latency_t       _r1D;
        _v_xdr12_v_mask_t               _r1E;
        _v_xdr12_sv_mask_t              _r1F;
        _v_xdr12_rstcnt_t               _r20;
        _v_xdr12_timeout_t              _r21;
        _v_xdr12_fllcnt1_t              _r22;
        _v_xdr12_fllcnt2_t              _r23;
        _v_xd12_wr_protect_t            _r24;
        _v_xdr12_nf_control_t           _r25;
        _v_xdr12_chop_en_t              _r26;
        _v_xdr12_temp_t                 _r27;
        _v_xdr12_osc_fll_man1_t         _r28;
        _v_xdr12_osc_fll_man2_t         _r29;
        _v_xdr12_osc_spread_t           _r2A;
        _v_xdr12_clock_gate_en_t        _r2B;
        _v_xdr12_dummy_t                _r2C;
        _v_xdr12_dummy_t                _r2D;
        _v_xdr12_dummy_t                _r2E;
        _v_xdr12_dummy_t                _r2F;
        _v_xdr12_dummy_t                _r30;
        _v_xdr12_dummy_t                _r31;
        _v_xdr12_dummy_t                _r32;
        _v_xdr12_dummy_t                _r33;
        _v_xdr12_dummy_t                _r34;
        _v_xdr12_dummy_t                _r35;
        _v_xdr12_dummy_t                _r36;
        _v_xdr12_dummy_t                _r37;
        _v_xdr12_dummy_t                _r38;
        _v_xdr12_dummy_t                _r39;
        _v_xdr12_otp_access_1_t         _r3A;
        _v_xdr12_otp_access_2_t         _r3B;
        _v_xdr12_otp_write_t            _r3C;
        _v_xdr12_otp_rd_prog_t          _r3D;
        _v_xdr12_otp_protect_t          _r3E;
        _v_xdr12_op_mode_t              _r3F;
    }reg;
}_xdr12_regs_t;

typedef union _xdr12_otp_ctrl_regs
{
    uint16_t ALL[XD12R_OTP_MAX];
    struct
    {
        _v_xdr12_otp_access_1_t         _r3A;
        _v_xdr12_otp_access_2_t         _r3B;
        _v_xdr12_otp_write_t            _r3C;
        _v_xdr12_otp_rd_prog_t          _r3D;
        _v_xdr12_otp_protect_t          _r3E;
        _v_xdr12_op_mode_t              _r3F;
    }reg;
}_xdr12_otp_ctrl_regs_t;

static const char* xdr12_regs_str[XD12R_MAX] =
{
    "RST & ID",
    "LD CONTROL",
    "LD_SIZE",
    "PWMCLK_DIV1/2",
    "PWMCLK_DIV2/3",
    "CHANNEL ENABLE",
    "FAULT CONTROL",
    "CHx_LD_TYPE0",
    "CHx_LD_TYPE1",
    "FAULT STATUS0",
    "MAX_CURR_VREF1",
    "MAX_CURR_VREF2",
    "MAX_CURR_VREF3",
    "MAX_CURR_VREF4",
    "MAX_CURR_VREF5",
    "TBD",

    "DELAY_CH1/2",
    "DELAY_CH3/4",
    "DELAY_CH5/6",
    "DELAY_CH7/8",
    "DELAY_CH9/10",
    "DELAY_CH11/12",
    "FB LEVEL",
    "FB/SHORT LEVEL",
    "SHORT LEVEL",
    "MAX CURRENT LEVEL0",
    "MAX CURRENT LEVEL1",
    "PARITY & RD_EN",
    "SERIAL CLK GEN",
    "SERIAL_LATENCY",
    "V_MASK",
    "SV_MASK",

    "RSTCNT",
    "TIMEOUT",
    "FLLCNT1",
    "FLLCNT2",
    "WR_PROTECT",
    "NF CONTROL",
    "CHOP_EN",
    "TEMP",
    "OSC_FLL_MAN1",
    "OSC_FLL_MAN2",
    "OSC SPREAD",
    "CLOCK GATE EN",
};

static const char* xdr12_otp_ctrl_regs_str[XD12R_OTP_MAX] =
{
    "OTP_ACCESS_B15_B12",
    "OTP_ACCESS_B11_B0,",
    "OTP_WRITE",
    "OTP_RD_PROG",
    "OTP_PROTECT",
    "OTP_MODE",
};

/********************************************* */

/* Register Map */
typedef enum _xd12r_mirror_addr_
{
    XD12R_MIRROR1 = 0,              /* 0x00 */
    XD12R_MIRROR2,                  /* 0x01 */
    XD12R_MIRROR3,                  /* 0x02 */
    XD12R_MIRROR_OFS_CH01,          /* 0x03 */
    XD12R_MIRROR_OFS_CH02,          /* 0x04 */
    XD12R_MIRROR_OFS_CH03,          /* 0x05 */
    XD12R_MIRROR_OFS_CH04,          /* 0x06 */
    XD12R_MIRROR_OFS_CH05,          /* 0x07 */
    XD12R_MIRROR_OFS_CH06,          /* 0x08 */
    XD12R_MIRROR_OFS_CH07,          /* 0x09 */
    XD12R_MIRROR_OFS_CH08,          /* 0x0A */
    XD12R_MIRROR_OFS_CH09,          /* 0x0B */
    XD12R_MIRROR_OFS_CH10,          /* 0x0C */
    XD12R_MIRROR_OFS_CH11,          /* 0x0D */
    XD12R_MIRROR_OFS_CH12,          /* 0x0E */

    XD12R_MIRROR_GAIN_CH01 = 0x10,  /* 0x10 */
    XD12R_MIRROR_GAIN_CH02,         /* 0x11 */
    XD12R_MIRROR_GAIN_CH03,         /* 0x12 */
    XD12R_MIRROR_GAIN_CH04,         /* 0x13 */
    XD12R_MIRROR_GAIN_CH05,         /* 0x14 */
    XD12R_MIRROR_GAIN_CH06,         /* 0x15 */
    XD12R_MIRROR_GAIN_CH07,         /* 0x16 */
    XD12R_MIRROR_GAIN_CH08,         /* 0x17 */
    XD12R_MIRROR_GAIN_CH09,         /* 0x18 */
    XD12R_MIRROR_GAIN_CH10,         /* 0x19 */
    XD12R_MIRROR_GAIN_CH11,         /* 0x1A */
    XD12R_MIRROR_GAIN_CH12,         /* 0x1B */
    XD12R_MIRROR_VERSION_0,         /* 0x1C */
    XD12R_MIRROR_VERSION_1,         /* 0x1D */

    XD12R_MIRROR_MAX,
}xd12_mirror_addr_t;
/* MIRROR1 (0x00) : default 0x000 */
typedef union tag_XDR12_MIRROR1
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_crc_checksum   :  8;
        uint16_t                    :  8;
    }bit;
}_v_xdr12_mirror1_t;

/* MIRROR2 (0x01) : default 0x210 */
typedef union tag_XDR12_MIRROR2
{
    uint16_t ALL;
    struct
    {
        uint16_t bgr_tc     :  5;
        uint16_t iref_ctl   :  5;
        uint16_t            :  6;
    }bit;
}_v_xdr12_mirror2_t;

/* MIRROR3 (0x02) : default 0x810 */
typedef union tag_XDR12_MIRROR3
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_rctl       :  5;
        uint16_t                :  2;
        uint16_t ldo_dac_ctl    :  5;
        uint16_t                :  4;
    }bit;
}_v_xdr12_mirror3_t;

/* OFS_CHx (0x03 ~ 0x0F) : default 0x080 */
typedef union tag_XDR12_OFS_CHx
{
    uint16_t ALL;
    struct
    {
        uint16_t ofs_chx    :  9;
        uint16_t            :  7;
    }bit;
}_v_xdr12_ofs_chx_t;

/* GAIN_CHx (0x10 ~ 0x1B) : default 0x0$0 */
typedef union tag_XDR12_GAIN_CHx
{
    uint16_t ALL;
    struct
    {
        uint16_t gain_chx   :  7;
        uint16_t            :  9;
    }bit;
}_v_xdr12_gain_chx_t;

/* VERSION0 (0x1C) : default 0x000 */
typedef union tag_XDR12_VERSION0
{
    uint16_t ALL;
    struct
    {
        uint16_t version0   : 12;
        uint16_t            :  4;
    }bit;
}_v_xdr12_version0_t;

/* VERSION1 (0x1D) : default 0x000 */
typedef union tag_XDR12_VERSION1
{
    uint16_t ALL;
    struct
    {
        uint16_t version1   : 12;
        uint16_t            :  4;
    }bit;
}_v_xdr12_version1_t;

typedef union _xdr12_mirror_regs
{
    uint16_t ALL[XD12R_MIRROR_MAX];
    struct
    {
        _v_xdr12_mirror1_t      _r00;
        _v_xdr12_mirror2_t      _r01;
        _v_xdr12_mirror3_t      _r02;
        _v_xdr12_ofs_chx_t      _r03;
        _v_xdr12_ofs_chx_t      _r04;
        _v_xdr12_ofs_chx_t      _r05;
        _v_xdr12_ofs_chx_t      _r06;
        _v_xdr12_ofs_chx_t      _r07;
        _v_xdr12_ofs_chx_t      _r08;
        _v_xdr12_ofs_chx_t      _r09;
        _v_xdr12_ofs_chx_t      _r0A;
        _v_xdr12_ofs_chx_t      _r0B;
        _v_xdr12_ofs_chx_t      _r0C;
        _v_xdr12_ofs_chx_t      _r0D;
        _v_xdr12_ofs_chx_t      _r0E;
        _v_xdr12_ofs_chx_t      _r0F;

        _v_xdr12_gain_chx_t     _r10;
        _v_xdr12_gain_chx_t     _r11;
        _v_xdr12_gain_chx_t     _r12;
        _v_xdr12_gain_chx_t     _r13;
        _v_xdr12_gain_chx_t     _r14;
        _v_xdr12_gain_chx_t     _r15;
        _v_xdr12_gain_chx_t     _r16;
        _v_xdr12_gain_chx_t     _r17;
        _v_xdr12_gain_chx_t     _r18;
        _v_xdr12_gain_chx_t     _r19;
        _v_xdr12_gain_chx_t     _r1A;
        _v_xdr12_gain_chx_t     _r1B;
        _v_xdr12_version0_t     _r1C;
        _v_xdr12_version1_t     _r1D;
    }reg;
}_xdr12_mirror_regs_t;

#ifdef __cplusplus
}
#endif

#endif	/* _XD12R_ES1_STRUCT_H_ */