/** @file dimming_test.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __DIMMING_TEST_H__
#define __DIMMING_TEST_H__

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __XD12_C__ 
    #define EXTERN
#else
    #define EXTERN extern
#endif
#ifdef __cplusplus
 extern "C" {
#endif

#ifdef __cplusplus
}
#endif


typedef enum
{
    DIMMING_STEP_NONE = 0, 
    DIMMING_STEP_TEST_INIT, 
    DIMMING_STEP_INIT_PARAM,
    DIMMING_STEP_SET_LD_DATA,
    DIMMING_STEP_START_ADC,
    DIMMING_STEP_GET_ADC,
    DIMMING_STEP_ALL_CHANNEL_IS_DONE,
    DIMMING_STEP_LOG,
    DIMMING_STEP_POWER_OFF,
    DIMMING_STEP_MAX,    
}dimming_step_t;

EXTERN void XD12_set_dimming_gain(current_gain_t in_gain);
EXTERN current_gain_t XD12_get_dimming_gain();
EXTERN void XD12_set_ldim(uint32_t in_ldim);
EXTERN uint32_t XD12_get_ldim();
EXTERN uint8_t XD12_get_dimming_init_done_flag();
EXTERN void XD12_dimming_test_init();
EXTERN void XD12_tx_LD_Data(uint16_t in_pwm);
EXTERN void XD12_set_PWM(uint32_t in_pwm_val);
EXTERN uint16_t XD12_get_PWM();
EXTERN void dimming_procedure_run();
EXTERN void XD12_start_dimming_test();

EXTERN void XD12_set_LD_tx_done_flag(uint8_t is_done);
EXTERN uint16_t XD12_get_ch_change_done_flag();
EXTERN void XD12_set_ch_change_done_flag(uint16_t is_done);

#if 1
EXTERN void XD12_set_short_level(uint8_t in_short_level);
EXTERN void XD12_set_FB_level(uint8_t in_fb_level);
EXTERN void XD12_get_fault_status();
EXTERN uint8_t XD12_get_fault_test_run_flag();
EXTERN void XD12_stop_fault_test();
#endif

#endif /* ~__DIMMING_TEST_H__ */

/*** end of file ***/
