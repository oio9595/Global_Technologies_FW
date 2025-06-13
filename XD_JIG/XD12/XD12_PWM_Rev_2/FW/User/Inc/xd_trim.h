/** @file xd_trim.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XD_TRIM_H__
#define __XD_TRIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define XDIC_VREF_MAX                   (4095)
#define XDIC_ICTL_L_MAX_CURRENT_LVL     (DEV_MAX_CURR_LEVEL_8mA)
#define XDIC_ICTL_H_MAX_CURRENT_LVL     (DEV_MAX_CURR_LEVEL_32mA)

typedef enum tag_XD_TRIM_STEP_T
{
    XD_TRIM_STEP_NONE = 0,

    XD_TRIM_STEP_ACTIVATE_START,
    XD_TRIM_STEP_IC_PWR,
    XD_TRIM_STEP_ACTIVATE_END,

    XD_TRIM_STEP_TRIM_REGISTER_VERIFY,
    XD_TRIM_STEP_CHANGE_OUTPUT_INIT,
    XD_TRIM_STEP_CHANGE_OUTPUT,
    XD_TRIM_STEP_CHANGE_OUTPUT_DONE,
    XD_TRIM_STEP_CHANGE_REGISTER,
    XD_TRIM_STEP_SET_ADC_CH,
    XD_TRIM_STEP_START_ADC_CONVERSION,
    XD_TRIM_STEP_GET_ADC_CH,
    XD_TRIM_STEP_CHECK,
    XD_TRIM_STEP_E2P_PROGRAM,
    XD_TRIM_STEP_E2P_PROGRAM_START,
    XD_TRIM_STEP_E2P_PROGRAM_END,
    XD_TRIM_STEP_STOP,
    XD_TRIM_STEP_RESULT,
    XD_TRIM_STEP_REBOOT,
    XD_TRIM_STEP_COMPARE,
    XD_TRIM_STEP_PWR_OFF,
    XD_TRIM_STEP_MAX,
} xd_trim_step_t;

typedef enum tag_XD_SCREEN_STEP_T
{
    XD_SCREEN_STEP_NONE,
    XD_SCREEN_STEP_PWR_ON,
    XD_SCREEN_STEP_SETUP,
    XD_SCREEN_STEP_CHANGE_OUTPUT,
    XD_SCREEN_STEP_SET_ADC_CH,
    XD_SCREEN_STEP_START_ADC_CONVERSION,
    XD_SCREEN_STEP_GET_ADC_CH,
    XD_SCREEN_STEP_STOP,
    XD_SCREEN_STEP_MAX,
} xd_screen_step_t;

typedef enum tag_TRIM_ERROR_CODE_T
{
    TRIM_ERROR_NONE = 0,
    TRIM_ERROR_OVER_COUNT,
    TRIM_ERROR_UNDER_COUNT,
    TRIM_ERROR_MAX,
} trim_error_code_t;

typedef enum tag_XD_TRIM_MODE_T
{
    XD_TRIM_START = 0,
    XD_TRIM_VREF_CTL = 0,
    XD_TRIM_OSC_FREQUENCY,
    XD_TRIM_ICTL_L_CHS,
    XD_TRIM_ICTL_H_CHS,
    XD_TRIM_MAX
} xd_trim_mode_t;

typedef enum tag_TRIM_ALGORITHM_RESULT_T
{
    TRIM_ALGORITHM_CONTINUE   = 0,
    TRIM_ALGORITHM_DONE_CHANNEL,
    TRIM_ALGORITHM_DONE_MODE,
    TRIM_ALGORITHM_ERROR,
    TRIM_ALGORITHM_MAX
} trim_algorithm_result_t;

/* END -  Input parameter from GUI */

extern void XD_Trim_Task(void);
extern void XD_Screen_Task(void);

/* BEGIN - INTERFACE FUNCTIONS */
extern void XD_Trim_Calculate_Spec(void);
extern void XD_Trim_IF_Trim_Start(void);
extern void XD_Trim_IF_Screen_Start(void);
extern void XD_Trim_IF_Set_OTP_Enable(bool in_flag);
extern bool XD_Trim_IF_Get_OTP_Enable(void);

/* END   - INTERFACE FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ~__XD_TRIM_H__ */

/*** end of file ***/


