/** @file ip805_i2c.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __IP805_I2C_H__
#define __IP805_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef union st_ip805_e0
{
    uint16_t val;
    struct
    {
        uint16_t tm_atb1_sel        : 3;
        uint16_t tm_atb_bias_en     : 1;
        uint16_t tm_tsd_vctat       : 1;
        uint16_t tm_tsd_ot          : 1;
        uint16_t tm_fm_dac_data     : 1;
        uint16_t tm_fm_dac_en       : 1;
        uint16_t tm_iddq_ldo_vdd    : 1;
        uint16_t tm_iddq_ldo_1p5v   : 1;
        uint16_t tm_iddq_all        : 1;
        uint16_t tm_lp_mask         : 1;
        uint16_t                    : 4;
    };
} ip805_e0_t;

typedef union st_ip805_e1
{
    uint16_t val;
    struct
    {
        uint16_t tm_gate_force_on       : 1;
        uint16_t tm_gate_force_off      : 1;
        uint16_t tm_buck_ss_latch_en    : 1;
        uint16_t tm_buck_ss_disable     : 1;
        uint16_t tm_cp_disable          : 1;
        uint16_t tm_cp_en               : 1;
        uint16_t tm_ota_gm_down_en      : 1;
        uint16_t tm_ota_offset          : 1;
        uint16_t tm_comp_msk            : 1;
        uint16_t tm_f_ss_reset_en       : 1;
        uint16_t tm_f_ss_reset_sel      : 1;
        uint16_t test_e                 : 1;
        uint16_t                        : 4;
    };
} ip805_e1_t;

typedef union st_ip805_e2
{
    uint16_t val;
    struct
    {
        uint16_t tm_pfm_dis         : 1;
        uint16_t tm_pfm_weak        : 1;
        uint16_t tm_pfm_os          : 2;
        uint16_t tm_buck_atb2_sel   : 2;
        uint16_t tm_buck_atb2_en    : 1;
        uint16_t tm_ilim_reduce     : 1;
        uint16_t tm_buck_atb1_sel   : 2;
        uint16_t tm_buck_atb1_en    : 1;
        uint16_t tm_fsw_sweep_en    : 1;
        uint16_t                    : 4;
    };
} ip805_e2_t;

typedef union st_ip805_e3
{
    uint16_t val;
    struct
    {
        uint16_t tm_clk_div_con     : 3;
        uint16_t tm_hiz_res_sw_on   : 1;
        uint16_t tm_sio_test_en     : 3;
        uint16_t tm_lvds_tx_in_test : 1;
        uint16_t tm_se_n_tx_in_test : 1;
        uint16_t tm_se_p_tx_in_test : 1;
        uint16_t tm_lf_clk_sel      : 2;
        uint16_t                    : 4;
    };
} ip805_e3_t;

typedef union st_ip805_e4
{
    uint16_t val;
    struct
    {
        uint16_t tm_lfp_en      : 1;
        uint16_t tm_lf_duty_en  : 1;
        uint16_t tm_hf_mode     : 1;
        uint16_t tm_hf_en       : 1;
        uint16_t tm_hfp_en      : 1;
        uint16_t tm_hf_duty_en  : 1;
        uint16_t tm_hf_clk_sel  : 2;
        uint16_t tm_hf_fm_freq  : 3;
        uint16_t tm_enable_lock : 1;
        uint16_t                : 4;
    };
} ip805_e4_t;

typedef union st_ip805_e5
{
    uint16_t val;
    struct
    {
        uint16_t dtb1_mux : 5;
        uint16_t dtb1_mux_sel : 1;
        uint16_t dtb1_mux_en : 1;
        uint16_t dtb1_mux_en : 1;
        uint16_t                : 4;
    };
} ip805_e5_t;

#ifdef __cplusplus
}
#endif

#endif /* ~__IP805_I2C_H__ */

/*** end of file ***/