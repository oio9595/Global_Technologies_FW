/* USER CODE BEGIN Header */
/*
    * File:   drv_id601.h
    * Author: GT
    *
    * Created on 2026. 09. 22.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_ID601_H__
#define __DRV_ID601_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */
#include "id601_metadata.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define ID601_TRIM_GAIN_VREF1               (0x012CU)
#define ID601_TRIM_GAIN_VREF2               (0x0514U)
#define ID601_TRIM_GAIN_MAX_CURR_LVL        (ID601_MAX_CURRENT_LEVEL_08mA)

#define ID601_TRIM_OFFSET_VREF1             (0x00C8U)
#define ID601_TRIM_OFFSET_VREF2             (0x012CU)
#define ID601_TRIM_OFFSET_MAX_CURR_LVL      (ID601_MAX_CURRENT_LEVEL_08mA)

#define ID601_VFY_3P_VREF1                  (0x012CU)
#define ID601_VFY_3P_VREF2                  (0x02BCU)
#define ID601_VFY_3P_VREF3                  (0x0BB8U)

#define ID601_VFY_SWEEP_VREF                (0x012CU)

#define ID601_TRIM_CODE_IREF1_MAX           (0x001FU)
#define ID601_TRIM_CODE_IREF2_MAX           (0x001FU)
#define ID601_TRIM_CODE_LDO_BGR2_CTL_MAX    (0x001FU)
#define ID601_TRIM_CODE_LDO_CTL_MAX         (0x000FU)
#define ID601_TRIM_CODE_LDO_DAC_CTL_MAX     (0x001FU)
#define ID601_TRIM_CODE_LDO_OSC_CTL_MAX     (0x000FU)
#define ID601_TRIM_CODE_OFS_TEMP_MAX        (0x000FU)
#define ID601_TRIM_CODE_OSC_RCTL_MAX        (0x001FU)
#define ID601_TRIM_CODE_GAIN_MAX            (0x007FU)
#define ID601_TRIM_CODE_OFFSET_MAX          (0x01FFU)
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef uint8_t (*id601_crc_func_t)(uint32_t, uint16_t);

typedef enum tag_ID601_REGISTER_BANK
{
    ID601_REGISTER_BANK_GENERAL = 0U,
    ID601_REGISTER_BANK_MIRROR,
    ID601_REGISTER_BANK_COUNT,
} id601_register_bank_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern void drv_id601_init(void);
extern void drv_id601_make_mutex(void);

extern bool drv_id601_set_daisy_length(uint16_t daisy_length);
extern uint16_t drv_id601_get_daisy_length(void);

extern bool drv_id601_set_hvs_flag(bool hvs_flag);
extern bool drv_id601_get_hvs_flag(void);

extern bool drv_id601_set_crc_func(id601_crc_func_t crc_func);
extern id601_crc_func_t drv_id601_get_crc_func(void);

extern bool id601_write_register(const uint16_t bank, const uint16_t device_select, const uint16_t addr, const uint16_t new_value, id601_crc_func_t p_crc_func);
extern bool id601_read_register(const uint16_t bank, const uint16_t device_select, const uint16_t addr, id601_crc_func_t p_crc_func, uint16_t* p_single_read_data);
extern bool id601_ld_transfer(uint16_t* p_buff, id601_crc_func_t p_crc_func);
extern bool id601_fault_read(id601_crc_func_t p_crc_func);
extern bool id601_sync_gen(id601_crc_func_t p_crc_func);
extern bool id601_id_gen(id601_crc_func_t p_crc_func);

extern bool id601_write_reset_id(uint16_t device_select, uint16_t rst, uint16_t vs_rst, uint16_t e_rst, uint16_t lkg_e, uint16_t id);
extern bool id601_write_ld_control(uint16_t device_select, uint16_t sv_no, uint16_t ld_type, uint16_t delay_ch_en, uint16_t syncmode, uint16_t pwm_res, uint16_t ld_dir, uint16_t ld_mode);
extern bool id601_write_ld_size(uint16_t device_select, uint16_t ld_size);
extern bool id601_write_pwmclk_div_1_2(uint16_t device_select, uint16_t fpwm_div_2, uint16_t fpwm_div_1);
extern bool id601_write_pwmclk_div_2_3(uint16_t device_select, uint16_t fpwm_div_3, uint16_t fpwm_div_2);
extern bool id601_write_channel_enable(uint16_t device_select, uint16_t ch12_en, uint16_t ch11_en, uint16_t ch10_en, uint16_t ch9_en, uint16_t ch8_en, uint16_t ch7_en, uint16_t ch6_en, uint16_t ch5_en, uint16_t ch4_en, uint16_t ch3_en, uint16_t ch2_en, uint16_t ch1_en);
extern bool id601_write_fault_control(uint16_t device_select, uint16_t ft_mode, uint16_t fb_mode, uint16_t o_fb_e, uint16_t t_det_e, uint16_t s_det_e, uint16_t o_det_e, uint16_t t_off_e, uint16_t s_off_e, uint16_t o_off_e);
extern bool id601_write_fb_level(uint16_t device_select, uint16_t fb3_level, uint16_t fb2_level, uint16_t fb1_level);
extern bool id601_write_short_level(uint16_t device_select, uint16_t short3_level, uint16_t short2_level, uint16_t short1_level);
extern bool id601_write_fault_status0(void);
extern bool id601_write_fault_status1(void);
extern bool id601_write_fault_status2(void);
extern bool id601_write_fault_status3(void);
extern bool id601_write_max_current_level(uint16_t device_select, uint16_t max_curr_level3, uint16_t max_curr_level2, uint16_t max_curr_level1);
extern bool id601_write_max_curr_vref1(uint16_t device_select, uint16_t max_curr_vref1);
extern bool id601_write_max_curr_vref2(uint16_t device_select, uint16_t max_curr_vref2);

extern bool id601_write_max_curr_vref3(uint16_t device_select, uint16_t max_curr_vref3);
extern bool id601_write_ch_ld_type(uint16_t device_select, uint16_t ch12_ld_type, uint16_t ch11_ld_type, uint16_t ch10_ld_type, uint16_t ch9_ld_type, uint16_t ch8_ld_type, uint16_t ch7_ld_type,                                              uint16_t ch6_ld_type, uint16_t ch5_ld_type, uint16_t ch4_ld_type, uint16_t ch3_ld_type, uint16_t ch2_ld_type, uint16_t ch1_ld_type);
extern bool id601_write_delay_ch1_2(uint16_t device_select, uint16_t delay_ch2, uint16_t delay_ch1);
extern bool id601_write_delay_ch3_4(uint16_t device_select, uint16_t delay_ch4, uint16_t delay_ch3);
extern bool id601_write_delay_ch5_6(uint16_t device_select, uint16_t delay_ch6, uint16_t delay_ch5);
extern bool id601_write_delay_ch7_8(uint16_t device_select, uint16_t delay_ch8, uint16_t delay_ch7);
extern bool id601_write_delay_ch9_10(uint16_t device_select, uint16_t delay_ch10, uint16_t delay_ch9);
extern bool id601_write_delay_ch11_12(uint16_t device_select, uint16_t delay_ch12, uint16_t delay_ch11);
extern bool id601_write_fault_configuration(uint16_t device_select, uint16_t crc_en, uint16_t wd_dimm_en, uint16_t wd_timecfg, uint16_t wd_en, uint16_t vs_miss_dimm_en, uint16_t vs_miss_en, uint16_t ch_ctrl_mismatch_en,         uint16_t vref_ovuv_off_en, uint16_t vref_ovuv_en, uint16_t osc_ldo_ovuv_en, uint16_t ldo_ovuv_en);
extern bool id601_write_bist(uint16_t device_select, uint16_t bist_start);
extern bool id601_write_test_mode0(uint16_t device_select, uint16_t dmux_sel);
extern bool id601_write_test_mode1(uint16_t device_select, uint16_t testmode, uint16_t tm_ch_ctrl_input_flip);
extern bool id601_write_serial_clk_gen(uint16_t device_select, uint16_t serial_clk_low, uint16_t serial_clk_high);
extern bool id601_write_serial_latency(uint16_t device_select, uint16_t serial_latency);
extern bool id601_write_v_mask(uint16_t device_select, uint16_t v_mask);
extern bool id601_write_sv_mask(uint16_t device_select, uint16_t sv_mask_en, uint16_t sv_mask);

extern bool id601_write_rstcnt(uint16_t device_select, uint16_t rstcnt);
extern bool id601_write_timeout(uint16_t device_select, uint16_t timeout);
extern bool id601_write_fllcnt1(uint16_t device_select, uint16_t fllcnt);
extern bool id601_write_fllcnt2(uint16_t device_select, uint16_t fll_en, uint16_t fll_range, uint16_t fllcnt);
extern bool id601_write_wr_protect(uint16_t device_select, uint16_t wr_protect);
extern bool id601_write_nf_control(uint16_t device_select, uint16_t o_emi_rej_en, uint16_t sgrjt_en2, uint16_t sgrjt_en1, uint16_t bbkn_en, uint16_t dgrjt_en2, uint16_t dgrjt_en1);
extern bool id601_write_chop_en(uint16_t device_select, uint16_t open_mask_opt, uint16_t chop_en, uint16_t chop_drv_en, uint16_t chop_osc_ldo_en, uint16_t chop_osc_en, uint16_t chop_dac_en, uint16_t chop_bgr_en);
extern bool id601_write_temp(uint16_t device_select, uint16_t ov_swap_en, uint16_t dac_rng, uint16_t flt_ctl, uint16_t o_slew, uint16_t flt_gain);
extern bool id601_write_osc_fll_man1(uint16_t device_select, uint16_t osc_fll_man);
extern bool id601_write_osc_fll_man2(uint16_t device_select, uint16_t osc_man_en, uint16_t osc_fll_err_range, uint16_t osc_fll_man);
extern bool id601_write_osc_spread(uint16_t device_select, uint16_t sprd_en, uint16_t sprd_spd, uint16_t sprd_gain);
extern bool id601_write_clock_gate_en(uint16_t device_select, uint16_t otp_mclk_en, uint16_t fr2_mclk_en, uint16_t fr1_mclk_en, uint16_t dc_mclk_en);
extern bool id601_write_vref_fix1(uint16_t device_select, uint16_t vref_fix1);
extern bool id601_write_vref_fix2(uint16_t device_select, uint16_t vref_fix2);
extern bool id601_write_vref_fix3(uint16_t device_select, uint16_t vref_fix3);

extern bool id601_write_test_ana_en(uint16_t device_select, uint16_t test_ana_en);
extern bool id601_write_otp_access1(uint16_t device_select, uint16_t otp_pg_acc_cycle);
extern bool id601_write_otp_access2(uint16_t device_select, uint16_t otp_pg_acc_cycle);
extern bool id601_write_otp_write(uint16_t device_select, uint16_t otp_rd, uint16_t otp_wsel);
extern bool id601_write_otp_rd_prog(uint16_t device_select, uint16_t otp_pg_done, uint16_t otp_rd_s, uint16_t otp_pg_s);
extern bool id601_write_otp_protect(uint16_t device_select, uint16_t protect_en);
extern bool id601_write_op_mode(uint16_t device_select, uint16_t test_en, uint16_t sw_sel, uint16_t pwmout_full, uint16_t mclk64_o, uint16_t dtest_mux_en, uint16_t rd_en, uint16_t ext_clkin, uint16_t addr_ext);

extern bool id601_dump_registers(void);
extern bool id601_compare_initial_code(void);

extern bool id601_init_tm_entry_mode(void);
extern bool id601_init_dimming_mode(void);
extern bool id601_init_after_efuse_mode(void);

extern bool id601_set_analog_test_mode(void);
extern bool id601_set_fll_test_mode(void);
extern bool id601_set_dimming_type1_mode(void);
extern bool id601_set_dimming_type2_mode(void);

extern void id601_trim_setup_iref1(void);
extern void id601_trim_setup_iref2(void);
extern void id601_trim_setup_ldo_det(void);
extern void id601_trim_setup_ldo_dig(void);
extern void id601_trim_setup_ldo_dac(void);
extern void id601_trim_setup_ldo_fll(void);
extern void id601_trim_setup_ofs_temp(void);
extern void id601_trim_setup_osc_rctl(void);
extern void id601_trim_setup_gain(void);
extern void id601_trim_setup_offset(void);

extern void id601_trim_setup_gain_vref1(void);
extern void id601_trim_setup_gain_vref2(void);
extern void id601_trim_setup_offset_vref1(void);
extern void id601_trim_setup_offset_vref2(void);

extern void id601_trim_setup_ofs_temp_o1(void);
extern void id601_trim_setup_ofs_temp_o2(void);
extern void id601_trim_setup_current_chx(uint16_t ch);
extern void id601_trim_setup_efuse(void);

extern bool id601_trim_save_search_registers(void);
extern bool id601_trim_compare_search_registers(void);
extern bool id601_trim_display_search_registers(void);
extern bool id601_trim_overwrite_search_registers(void);

extern uint16_t id601_trim_apply_iref1(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_iref2(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_ldo_det(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_ldo_dig(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_ldo_dac(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_ldo_fll(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_ofs_temp(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_osc_rctl(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_gain(uint16_t ch, uint16_t trim_code);
extern uint16_t id601_trim_apply_offset(uint16_t ch, uint16_t trim_code);

extern void id601_test_setup_idds1(void);
extern void id601_test_setup_iddd1(void);
extern void id601_test_setup_iref1(void);
extern void id601_test_setup_iref2(void);
extern void id601_test_setup_ldo_det(void);
extern void id601_test_setup_ldo_dig(void);
extern void id601_test_setup_ldo_dac(void);
extern void id601_test_setup_ldo_fll(void);
extern void id601_test_setup_bgr1(void);
extern void id601_test_setup_bgr2(void);
extern void id601_test_setup_osc(void);
extern void id601_test_setup_fll1(void);
extern void id601_test_setup_fll2(void);
extern void id601_test_setup_fll3(void);
extern void id601_test_setup_iout_3p(void);
extern void id601_test_setup_iout_sweep(void);
extern void id601_test_setup_iout_vref_sweep(void);

extern void id601_test_setup_iout_3p_x(uint8_t ph);

extern void id601_test_setup_iout_sweep_x(uint8_t ph);
extern void id601_test_setup_iout_vref_sweep_x(uint16_t vref);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ID601_H__ */
