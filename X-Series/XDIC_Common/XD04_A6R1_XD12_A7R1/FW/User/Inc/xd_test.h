/** @file xd_test.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XDIC_TEST_H__
#define __XDIC_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tag_XDIC_TEST_STEP_T
{
    XDIC_TEST_STEP_NONE = 0,
    XDIC_TEST_STEP_IC_INIT,
    XDIC_TEST_STEP_MODE_INIT,
    XDIC_TEST_STEP_CHANGE_OUTPUT,
    XDIC_TEST_STEP_SET_ADC_CH,
    XDIC_TEST_STEP_START_ADC_CONVERSION,
    XDIC_TEST_STEP_GET_ADC_CH,
    XDIC_TEST_STEP_CONVERSION_ADC_TO_VALUE,
    XDIC_TEST_STEP_RESULT,
    XDIC_TEST_STEP_PWR_OFF,
    XDIC_TEST_STEP_MAX,
} xdic_test_step_t;

typedef enum tag_XDIC_TEST_MODE_T
{
    XDIC_TEST_START = 0,
    XDIC_TEST_VREF_CTL = 0,
    XDIC_TEST_OSC_FREQUENCY,
    XDIC_TEST_CURRENT_TYPE_A, //XD12 Gain / XD04 ICTL_L
    XDIC_TEST_CURRENT_TYPE_B, //XD12 OFS  / XD04 ICTL_H
    XDIC_TEST_ICC,
    XDIC_TEST_LDO_SWEEP,
    XDIC_TEST_MAX
} xdic_test_mode_t;

extern void XDIC_Test_Task(void);
extern void XDIC_Test_Start(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__XDIC_TEST_H__ */

/*** end of file ***/


