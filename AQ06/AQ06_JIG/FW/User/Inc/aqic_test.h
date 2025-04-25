/** @file aqic_test.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __AQIC_TEST_H__
#define __AQIC_TEST_H__

#ifdef __cplusplus
    extern "C" {
#endif

extern float gf_measure_cz_volt;
extern float gf_measure_end_point;
extern float gf_manual_volt;
extern uint8_t gn_manual_gain;

EXTERN void test_Procedure_Start(void);
EXTERN void AQIC_Set_Operating_Mode(uint8_t mode);

EXTERN void test_procedure_run(void);
EXTERN void trimming_procedure_run(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__AQIC_TEST_H__ */

/*** end of file ***/


