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

#define FAULT_FB_VLED_MAX       (2.5f)
#define FAULT_SHORT_VLED_MAX    (40.0f)

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

EXTERN bool gb_vsync_out;

EXTERN void vsync_update_handler(void);

EXTERN void XD12_set_dimming_gain(current_gain_t in_gain);
EXTERN current_gain_t XD12_get_dimming_gain(void);
EXTERN void XD12_set_ldim(uint32_t in_ldim);
EXTERN uint32_t XD12_get_ldim(void);
EXTERN void XD12_set_LD_out(uint32_t in_ld_out);
EXTERN uint16_t XD12_get_LD_out(void);
EXTERN void dimming_procedure_run(void);
EXTERN void XD12_start_dimming_test(void);
EXTERN void global_dimming_start(void);
EXTERN void XD12_start_debug_dimming_test(void);

#if 1
EXTERN void XD12_get_fault_status(void);

EXTERN void XD12_set_FB_vled(float n_vled);
EXTERN void XD12_set_short_vled(float n_vled);
#endif


#endif /* ~__DIMMING_TEST_H__ */
/*** end of file ***/
