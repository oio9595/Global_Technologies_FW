/** @file trimming.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __APIC_TRIMMING_H__
#define __APIC_TRIMMING_H__

#ifdef __APIC_TRIMMING_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef __cplusplus
 extern "C" {
#endif

void trimming_procedure_run(void);

/* for test only */
void apic_test_mode_exit(void);
void apic_get_regs_for_trimming(void);
void gt_cq24_test_init(void);

uint8_t aqic_set_reg_test1_sel_l_ch(uint8_t sel_l_ch);
uint8_t aqic_set_reg_test1_config_mode(uint8_t config_mode);
uint8_t aqic_set_reg_test2_cz_ch1(uint8_t cz_ch1);
uint8_t aqic_set_reg_test2_cz_ch2(uint8_t cz_ch2);
uint8_t aqic_set_reg_test3_cz_ch3(uint8_t cz_ch3);
uint8_t aqic_set_reg_test3_cz_ch4(uint8_t cz_ch4);
uint8_t aqic_set_reg_test4_cz_ch5(uint8_t cz_ch5);
uint8_t aqic_set_reg_test4_cz_ch6(uint8_t cz_ch6);
uint8_t aqic_set_reg_trim1_chkbit(uint8_t chkbit);
uint8_t aqic_set_reg_trim2_ofs_temp(uint8_t ofs_temp);
uint8_t aqic_set_reg_trim2_ictl_osc(uint8_t ictl_osc);
uint8_t aqic_set_reg_trim3_ofs_lin_ch1(uint16_t ofs_lin_ch1);
uint8_t aqic_set_reg_trim4_ofs_lin_ch2(uint16_t ofs_lin_ch2);
uint8_t aqic_set_reg_trim5_ofs_lin_ch3(uint16_t ofs_lin_ch3);
uint8_t aqic_set_reg_trim6_ofs_lin_ch4(uint16_t ofs_lin_ch4);
uint8_t aqic_set_reg_trim7_ofs_lin_ch5(uint16_t ofs_lin_ch5);
uint8_t aqic_set_reg_trim8_ofs_lin_ch6(uint16_t ofs_lin_ch6);
uint8_t aqic_set_reg_trim9_ofs_lin_ch1(uint8_t ofs_lin_ch1);
uint8_t aqic_set_reg_trim9_hdr_mode(uint8_t hdr_mode);
uint8_t aqic_set_reg_trim9_ofs_lin_ch2(uint8_t ofs_lin_ch2);
uint8_t aqic_set_reg_trim10_ofs_lin_ch3(uint8_t ofs_lin_ch3);
uint8_t aqic_set_reg_trim10_czmax1(uint8_t czmax1);
uint8_t aqic_set_reg_trim10_ofs_lin_ch4(uint8_t ofs_lin_ch4);
uint8_t aqic_set_reg_trim11_ofs_lin_ch5(uint8_t ofs_lin_ch5);
uint8_t aqic_set_reg_trim11_czmax2(uint8_t czmax2);
uint8_t aqic_set_reg_trim11_ofs_lin_ch6(uint8_t ofs_lin_ch6);
uint8_t aqic_set_reg_trim12_ictl_ch1(uint8_t ictl_ch1);
uint8_t aqic_set_reg_trim13_ictl_ch2(uint8_t ictl_ch2);
uint8_t aqic_set_reg_trim14_ictl_ch3(uint8_t ictl_ch3);
uint8_t aqic_set_reg_trim15_ictl_ch4(uint8_t ictl_ch4);
uint8_t aqic_set_reg_trim16_ictl_ch5(uint8_t ictl_ch5);
uint8_t aqic_set_reg_trim17_ictl_ch6(uint8_t ictl_ch6);

EXTERN uint8_t gb_NO_TRIM;

EXTERN float gf_MODE_DAC_Target;
EXTERN float gf_MODE_DAC_Step;

EXTERN float gf_VREFP_Target; //DAC_4_5_TARGET_V
EXTERN float gf_VREFP_Step; //DAC_4_5_STEP_V

EXTERN uint16_t gui_GAIN_DAC_DAC_Input; //VREF_12B_TRIM
EXTERN float gf_GAIN_DAC_Step; //VREF_OFFSET_STEP_V
EXTERN uint16_t gui_GAIN_DAC_vHigh; //Vh_12B
EXTERN uint16_t gui_GAIN_DAC_vLow; //Vl_12B
EXTERN float gf_GAIN_DAC_Delta; //VREF_OFFSET_T_DIFF

EXTERN uint16_t gui_DATA_DAC_DAC_Input; //VREF_12B_TRIM
EXTERN float gf_DATA_DAC_Step; //DAC12_mSTEP_V
EXTERN uint16_t gui_DATA_DAC_Target_Offset_DAC; //DAC_12B_TRIM
EXTERN float gf_DATA_DAC_Voltage; //DAC_12B_T

EXTERN uint16_t gui_SLOPE_GAIN_DAC_INPUT;
EXTERN uint16_t gui_TRIM_RANGE_MIN;
EXTERN uint16_t gui_TRIM_RANGE_MAX;
EXTERN uint8_t gb_OTP_WRITE;

EXTERN uint8_t gui_TRIM_regi_changed;

#define CHK_TRIM_TIME

#ifdef __cplusplus
}
#endif

#endif /* ~__APIC_TRIMMING_H__ */

/*** end of file ***/


