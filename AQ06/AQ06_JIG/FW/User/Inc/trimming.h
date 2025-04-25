/** @file trimming.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __APIC_TRIMMING_H__
#define __APIC_TRIMMING_H__

#ifdef __cplusplus
 extern "C" {
#endif

extern uint16_t gn_adc_compensate[3][6];

void trimming_procedure_start(void);
void trimming_procedure_run(void);

/* for test only */
void apic_get_regs_for_trimming(void);
void apic_program_e2p(void);

extern void AQIC_Select_Output_Ch(uint8_t ch);

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

#define I2C_AQIC_06     0x06
#define I2C_AQIC_08     0x08

EXTERN uint8_t GUI_link_reg_with_gval(uint8_t in_AQIC_TYPE, uint8_t in_reg_addr, uint8_t *in_reg_val);
EXTERN uint8_t GUI_SET_init_reg_tbl(uint8_t in_AQIC_TYPE);
EXTERN void GUI_SET_Activate();
EXTERN void GUI_SET_TrimStart(uint32_t register_apply_flg);
EXTERN void GUI_SET_RegisterReadAll();
EXTERN void GUI_print_reg_all();
EXTERN void GUI_SEND_reg_all();
EXTERN void GUI_write_reg(uint8_t in_addr, uint8_t in_val);
EXTERN void GUI_trim_enable(uint8_t in_val);
EXTERN void GUI_trim_VoltageTbl(float in_trim_volt[]);
EXTERN void GUI_trim_RangeTbl(uint32_t in_trim_rang[]);
EXTERN void GUI_SET_otp_regs(uint8_t in_AQIC_TYPE);
EXTERN void GUI_CZmax_CZn_Set(uint16_t Czmax, uint16_t Czn, uint16_t ScreenCzn);
EXTERN void GUI_SEND_otp_written();
EXTERN void GUI_din_mode_enable(uint8_t in_val);
EXTERN void GUI_GAIN_LEVEL(uint16_t ch_a_level);
EXTERN uint8_t check_valid_ictl_step(uint8_t in_ictl_step, uint8_t in_channel_num, uint8_t in_adj_type);
EXTERN uint8_t check_valid_ofs_step(uint8_t in_ofs_step, uint8_t in_channel_num, uint8_t in_adj_type);

EXTERN void Set_ofs_regi_value(uint8_t channel, uint16_t input_regi_val);
EXTERN void Set_ictl_regi_value(uint8_t channel, uint8_t input_regi_val);

EXTERN uint8_t Get_ictl_regi_value(uint8_t channel);
EXTERN uint16_t Get_ofs_regi_value(uint8_t channel);

#define INIT_OFS_LIN_CHS_ADC_PER_REGI   6
#define INIT_ICTL_CHS_ADC_PER_REGI      18

#ifdef __cplusplus
}
#endif

#endif /* ~__APIC_TRIMMING_H__ */

/*** end of file ***/


