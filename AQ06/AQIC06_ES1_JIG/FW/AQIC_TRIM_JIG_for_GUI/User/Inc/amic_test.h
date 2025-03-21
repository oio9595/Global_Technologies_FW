/** @file amic_test.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __AMIC_TEST_H__
#define __AMIC_TEST_H__

#ifdef __AMIC_TEST_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

extern float gf_measure_cz_volt;
extern float gf_measure_end_point;
extern float gf_manual_volt;
extern uint8_t gn_manual_gain;


EXTERN void dac_set_voltage(float f_voltage, uint8_t ch);
EXTERN void set_vled_9v(uint8_t on);
EXTERN void aqic_output_select(uint8_t ch);



EXTERN void test_procedure_run(void);
EXTERN void trimming_procedure_run(void);


#if 0
EXTERN void timer_for_virtual_vsync(void);
EXTERN void timer_for_output_channel(void); /* TIM5 */
EXTERN void timer_for_i_t_voltage(void); /* TIM9 */
EXTERN void timer_for_amic_gate_pulse(void);
EXTERN void timer_for_end(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ~__AMIC_TEST_H__ */

/*** end of file ***/


