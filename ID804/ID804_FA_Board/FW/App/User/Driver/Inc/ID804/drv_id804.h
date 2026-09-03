/* USER CODE BEGIN Header */
/*
    * File:   drv_id804.h
    * Author: GT
    *
    * Created on 2026. 09. 01.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_ID804_H__
#define __DRV_ID804_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum tag_ID804_COMM_MODE
{
    ID804_COMM_MODE_ME = 0U,
    ID804_COMM_MODE_I2C
} id804_comm_mode_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern bool id804_RESET(void);
extern bool id804_INITBIDIR(void);
extern bool id804_CLRERROR(void);
extern bool id804_GOSLEEP(void);
extern bool id804_GOACTIVE(void);
extern bool id804_GODEEPSLEEP(void);

extern uint16_t id804_get_r01_STATUS1(void);

extern uint16_t id804_get_r02_STATUS2(void);

extern uint16_t id804_get_r03_TEMP(void);

extern uint32_t id804_get_r04_TEMPST(void);

extern uint16_t id804_get_r05_VEXT_TM(void);

extern uint16_t id804_get_r06_SETUP1(void);
extern bool id804_set_r06_SETUP1(uint16_t value);

extern uint16_t id804_get_r07_SETUP2(void);
extern bool id804_set_r07_SETUP2(uint16_t value);

extern uint32_t id804_get_MCAST(void);
extern bool id804_set_MCAST(uint32_t value);

extern uint16_t id804_get_r08_MCAST1(void);
extern bool id804_set_r08_MCAST1(uint16_t value);

extern uint16_t id804_get_r09_MCAST2(void);
extern bool id804_set_r09_MCAST2(uint16_t value);

extern uint16_t id804_get_r0A_TEMPTH(void);
extern bool id804_set_r0A_TEMPTH(uint16_t value);

extern uint16_t id804_get_r0B_TEMPHYS(void);
extern bool id804_set_r0B_TEMPHYS(uint16_t value);

extern uint16_t id804_get_r0C_CAL_PWM_RED(void);

extern uint16_t id804_get_r0D_CAL_PWM_GREEN(void);

extern uint16_t id804_get_r0E_CAL_PWM_BLUE(void);

extern uint16_t id804_get_r0F_CURR_MAX_LVL(void);
extern bool id804_set_r0F_CURR_MAX_LVL(uint16_t value);

extern uint16_t id804_get_r10_TEMP_LUT_TC1(void);
extern bool id804_set_r10_TEMP_LUT_TC1(uint16_t value);

extern uint16_t id804_get_r11_TEMP_LUT_TC2(void);
extern bool id804_set_r11_TEMP_LUT_TC2(uint16_t value);

extern uint16_t id804_get_r12_TEMP_LUT_TC3(void);
extern bool id804_set_r12_TEMP_LUT_TC3(uint16_t value);

extern uint16_t id804_get_r13_TEMP_LUT_TC4(void);
extern bool id804_set_r13_TEMP_LUT_TC4(uint16_t value);

extern uint16_t id804_get_r14_TEMP_LUT_TC5(void);
extern bool id804_set_r14_TEMP_LUT_TC5(uint16_t value);

extern uint16_t id804_get_r15_TEMP_LUT_TC6(void);
extern bool id804_set_r15_TEMP_LUT_TC6(uint16_t value);

extern uint16_t id804_get_r16_TEMP_LUT_TC7(void);
extern bool id804_set_r16_TEMP_LUT_TC7(uint16_t value);

extern uint16_t id804_get_r17_TEMP_LUT_TC8(void);
extern bool id804_set_r17_TEMP_LUT_TC8(uint16_t value);

extern uint16_t id804_get_r18_TEMP_LUT_TC9(void);
extern bool id804_set_r18_TEMP_LUT_TC9(uint16_t value);

extern uint16_t id804_get_r19_TEMP_LUT_TC10(void);
extern bool id804_set_r19_TEMP_LUT_TC10(uint16_t value);

extern uint16_t id804_get_r1A_TIMEOUT(void);
extern bool id804_set_r1A_TIMEOUT(uint16_t value);

extern uint32_t id804_get_RGB(void);
extern bool id804_set_RGB(uint32_t value);

extern uint16_t id804_get_r20_RGB1(void);
extern bool id804_set_r20_RGB1(uint16_t value);

extern uint16_t id804_get_r21_RGB2(void);
extern bool id804_set_r21_RGB2(uint16_t value);

extern uint16_t id804_get_rE0_TRIM_CONTROL(void);
extern bool id804_set_rE0_TRIM_CONTROL(uint16_t value);

extern uint16_t id804_get_rE1_OTP_CONTROL1(void);
extern bool id804_set_rE1_OTP_CONTROL1(uint16_t value);

extern uint16_t id804_get_rE2_OTP_CONTROL2(void);
extern bool id804_set_rE2_OTP_CONTROL2(uint16_t value);

extern uint16_t id804_get_rE3_OTP_CONTROL3(void);
extern bool id804_set_rE3_OTP_CONTROL3(uint16_t value);

extern uint16_t id804_get_rE4_OTP1_MIRROR1(void);
extern bool id804_set_rE4_OTP1_MIRROR1(uint16_t value);

extern uint16_t id804_get_rE5_OTP1_MIRROR2(void);
extern bool id804_set_rE5_OTP1_MIRROR2(uint16_t value);

extern uint16_t id804_get_rE6_OTP1_MIRROR3(void);
extern bool id804_set_rE6_OTP1_MIRROR3(uint16_t value);

extern uint16_t id804_get_rE7_OTP1_MIRROR4(void);
extern bool id804_set_rE7_OTP1_MIRROR4(uint16_t value);

extern uint16_t id804_get_rE8_OTP1_MIRROR5(void);
extern bool id804_set_rE8_OTP1_MIRROR5(uint16_t value);

extern uint16_t id804_get_rE9_OTP1_MIRROR6(void);
extern bool id804_set_rE9_OTP1_MIRROR6(uint16_t value);

extern uint16_t id804_get_rEA_OTP1_MIRROR7(void);
extern bool id804_set_rEA_OTP1_MIRROR7(uint16_t value);

extern uint16_t id804_get_rEB_OTP1_MIRROR8(void);
extern bool id804_set_rEB_OTP1_MIRROR8(uint16_t value);

extern uint16_t id804_get_rEC_OTP1_MIRROR9(void);
extern bool id804_set_rEC_OTP1_MIRROR9(uint16_t value);

extern uint16_t id804_get_rED_OTP1_MIRROR10(void);
extern bool id804_set_rED_OTP1_MIRROR10(uint16_t value);

extern uint16_t id804_get_rEE_OTP1_MIRROR11(void);
extern bool id804_set_rEE_OTP1_MIRROR11(uint16_t value);

extern uint16_t id804_get_rEF_OTP1_MIRROR12(void);
extern bool id804_set_rEF_OTP1_MIRROR12(uint16_t value);

extern uint16_t id804_get_rF0_OTP1_MIRROR13(void);
extern bool id804_set_rF0_OTP1_MIRROR13(uint16_t value);

extern uint16_t id804_get_rF1_OTP2_MIRROR14(void);
extern bool id804_set_rF1_OTP2_MIRROR14(uint16_t value);

extern uint16_t id804_get_rF2_OTP2_MIRROR15(void);
extern bool id804_set_rF2_OTP2_MIRROR15(uint16_t value);

extern uint16_t id804_get_rF3_OTP2_MIRROR16(void);
extern bool id804_set_rF3_OTP2_MIRROR16(uint16_t value);

extern uint16_t id804_get_rF4_OTP2_MIRROR17(void);
extern bool id804_set_rF4_OTP2_MIRROR17(uint16_t value);

extern uint16_t id804_get_rF5_OTP2_MIRROR18(void);
extern bool id804_set_rF5_OTP2_MIRROR18(uint16_t value);

extern uint16_t id804_get_rF6_OTP2_MIRROR19(void);
extern bool id804_set_rF6_OTP2_MIRROR19(uint16_t value);

extern uint16_t id804_get_ic_state(void);

extern uint16_t id804_get_sio2(void);

extern uint16_t id804_get_sio1(void);

extern uint16_t id804_get_otpcrc(void);

extern uint16_t id804_get_t_out(void);

extern uint16_t id804_get_open_r(void);

extern uint16_t id804_get_open_g(void);

extern uint16_t id804_get_open_b(void);

extern uint16_t id804_get_short_r(void);

extern uint16_t id804_get_short_g(void);

extern uint16_t id804_get_short_b(void);

extern uint16_t id804_get_ot_flt(void);

extern uint16_t id804_get_uv_flt(void);

extern uint16_t id804_get_crc_flt(void);

extern uint16_t id804_get_com_flt(void);

extern uint16_t id804_get_temperature(void);

extern uint16_t id804_get_vext_tm(void);

extern uint16_t id804_get_tc_e(void);
extern bool id804_set_tc_e(uint16_t value);

extern uint16_t id804_get_os_flt_e(void);
extern bool id804_set_os_flt_e(uint16_t value);

extern uint16_t id804_get_ot_flt_e(void);
extern bool id804_set_ot_flt_e(uint16_t value);

extern uint16_t id804_get_uv_flt_e(void);
extern bool id804_set_uv_flt_e(uint16_t value);

extern uint16_t id804_get_com_flt_e(void);
extern bool id804_set_com_flt_e(uint16_t value);

extern uint16_t id804_get_crc_e(void);
extern bool id804_set_crc_e(uint16_t value);

extern uint16_t id804_get_ph_shift_e(void);
extern bool id804_set_ph_shift_e(uint16_t value);

extern uint16_t id804_get_lg_e(void);
extern bool id804_set_lg_e(uint16_t value);

extern uint16_t id804_get_f_fpwm_div(void);
extern bool id804_set_f_fpwm_div(uint16_t value);

extern uint16_t id804_get_clk_inv_e(void);
extern bool id804_set_clk_inv_e(uint16_t value);

extern uint16_t id804_get_vext_mon_e(void);
extern bool id804_set_vext_mon_e(uint16_t value);

extern uint16_t id804_get_event_cyc(void);
extern bool id804_set_event_cyc(uint16_t value);

extern uint16_t id804_get_uv_lvl(void);
extern bool id804_set_uv_lvl(uint16_t value);

extern uint16_t id804_get_sh_lvl(void);
extern bool id804_set_sh_lvl(uint16_t value);

extern uint16_t id804_get_tempth(void);
extern bool id804_set_tempth(uint16_t value);

extern uint16_t id804_get_temphys(void);
extern bool id804_set_temphys(uint16_t value);

extern uint16_t id804_get_pwm_red_val(void);

extern uint16_t id804_get_pwm_green_val(void);

extern uint16_t id804_get_pwm_blue_val(void);

extern uint16_t id804_get_r_curr_max_lvl(void);
extern bool id804_set_r_curr_max_lvl(uint16_t value);

extern uint16_t id804_get_g_curr_max_lvl(void);
extern bool id804_set_g_curr_max_lvl(uint16_t value);

extern uint16_t id804_get_b_curr_max_lvl(void);
extern bool id804_set_b_curr_max_lvl(uint16_t value);

extern uint16_t id804_get_temp_lut_tc1(void);
extern bool id804_set_temp_lut_tc1(uint16_t value);

extern uint16_t id804_get_temp_lut_tc2(void);
extern bool id804_set_temp_lut_tc2(uint16_t value);

extern uint16_t id804_get_temp_lut_tc3(void);
extern bool id804_set_temp_lut_tc3(uint16_t value);

extern uint16_t id804_get_temp_lut_tc4(void);
extern bool id804_set_temp_lut_tc4(uint16_t value);

extern uint16_t id804_get_temp_lut_tc5(void);
extern bool id804_set_temp_lut_tc5(uint16_t value);

extern uint16_t id804_get_temp_lut_tc6(void);
extern bool id804_set_temp_lut_tc6(uint16_t value);

extern uint16_t id804_get_temp_lut_tc7(void);
extern bool id804_set_temp_lut_tc7(uint16_t value);

extern uint16_t id804_get_temp_lut_tc8(void);
extern bool id804_set_temp_lut_tc8(uint16_t value);

extern uint16_t id804_get_temp_lut_tc9(void);
extern bool id804_set_temp_lut_tc9(uint16_t value);

extern uint16_t id804_get_temp_lut_tc10(void);
extern bool id804_set_temp_lut_tc10(uint16_t value);

extern uint16_t id804_get_timeout(void);
extern bool id804_set_timeout(uint16_t value);

extern uint16_t id804_get_test_e(void);
extern bool id804_set_test_e(uint16_t value);

extern bool id804_get_cko_e(void);
extern bool id804_set_cko_e(uint16_t value);

extern uint16_t id804_get_adc_clk_pol(void);
extern bool id804_set_adc_clk_pol(uint16_t value);

extern uint16_t id804_get_ana_mod_sel(void);
extern bool id804_set_ana_mod_sel(uint16_t value);

extern uint16_t id804_get_t_ana_sel(void);
extern bool id804_set_t_ana_sel(uint16_t value);

extern uint16_t id804_get_pwm_max_r_e(void);
extern bool id804_set_pwm_max_r_e(uint16_t value);

extern uint16_t id804_get_pwm_max_g_e(void);
extern bool id804_set_pwm_max_g_e(uint16_t value);

extern uint16_t id804_get_pwm_max_b_e(void);
extern bool id804_set_pwm_max_b_e(uint16_t value);

extern uint16_t id804_get_otp_pg2(void);
extern bool id804_set_otp_pg2(uint16_t value);

extern uint16_t id804_get_otp_pg1(void);
extern bool id804_set_otp_pg1(uint16_t value);

extern uint16_t id804_get_t_ana_e(void);
extern bool id804_set_t_ana_e(uint16_t value);

extern uint16_t id804_get_otp_rd(void);
extern bool id804_set_otp_rd(uint16_t value);

extern uint16_t id804_get_otp_wsel(void);
extern bool id804_set_otp_wsel(uint16_t value);

extern uint16_t id804_get_tmux_sel(void);
extern bool id804_set_tmux_sel(uint16_t value);

extern uint16_t id804_get_otp_pg_acc_cycle(void);
extern bool id804_set_otp_pg_acc_cycle(uint16_t value);

extern uint16_t id804_get_protect(void);
extern bool id804_set_protect(uint16_t value);

extern uint16_t id804_get_sio1_tr_tx_bias(void);
extern bool id804_set_sio1_tr_tx_bias(uint16_t value);

extern uint16_t id804_get_sio2_tr_tx_bias(void);
extern bool id804_set_sio2_tr_tx_bias(uint16_t value);

extern uint16_t id804_get_v_trim_bgr(void);
extern bool id804_set_v_trim_bgr(uint16_t value);

extern uint16_t id804_get_sio1_tr_res(void);
extern bool id804_set_sio1_tr_res(uint16_t value);

extern uint16_t id804_get_sio2_tr_res(void);
extern bool id804_set_sio2_tr_res(uint16_t value);

extern uint16_t id804_get_vldo_ctrl(void);
extern bool id804_set_vldo_ctrl(uint16_t value);

extern uint16_t id804_get_i_trim_bgr(void);
extern bool id804_set_i_trim_bgr(uint16_t value);

extern uint16_t id804_get_temp_trim_bgr(void);
extern bool id804_set_temp_trim_bgr(uint16_t value);

extern uint16_t id804_get_bgr_r1_trim(void);
extern bool id804_set_bgr_r1_trim(uint16_t value);

extern uint16_t id804_get_osc_ctl(void);
extern bool id804_set_osc_ctl(uint16_t value);

extern uint16_t id804_get_bgr_r2_trim(void);
extern bool id804_set_bgr_r2_trim(uint16_t value);

extern uint16_t id804_get_adc_offset(void);
extern bool id804_set_adc_offset(uint16_t value);

extern uint16_t id804_get_adc_gain(void);
extern bool id804_set_adc_gain(uint16_t value);

extern uint16_t id804_get_itrim_r(void);
extern bool id804_set_itrim_r(uint16_t value);

extern uint16_t id804_get_itrim_g(void);
extern bool id804_set_itrim_g(uint16_t value);

extern uint16_t id804_get_itrim_b(void);
extern bool id804_set_itrim_b(uint16_t value);

extern uint16_t id804_get_tc_base(void);
extern bool id804_set_tc_base(uint16_t value);

extern uint16_t id804_get_tc_offset(void);
extern bool id804_set_tc_offset(uint16_t value);

extern uint16_t id804_get_pwm_max_r(void);
extern bool id804_set_pwm_max_r(uint16_t value);

extern uint16_t id804_get_pwm_max_g(void);
extern bool id804_set_pwm_max_g(uint16_t value);

extern uint16_t id804_get_pwm_max_b(void);
extern bool id804_set_pwm_max_b(uint16_t value);

extern uint16_t id804_get_otp_crc(void);
extern bool id804_set_otp_crc(uint16_t value);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ID804_H__ */
