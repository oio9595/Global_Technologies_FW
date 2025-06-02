/** @file trimming.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XD12_TRIMMING_H__
#define __XD12_TRIMMING_H__

#include "JigBd_IF.h"

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __XD12_TRIMMING_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef __cplusplus
 extern "C" {
#endif

#define XD12_CURRENT_TRIM_VREF  (2048)
#define XD12_VREF_TRIM_VREF     (4095)
#define XD12_VREF_MAX           (4095)

#define XD12_ICTL_L_MAX_CURRENT_LVL (DEV_MAX_CURR_LEVEL_8mA)   /* mA */
#define XD12_ICTL_H_MAX_CURRENT_LVL (DEV_MAX_CURR_LEVEL_32mA)   /* mA */

typedef enum
{
    TRIMMING_STEP_NONE = 0,
    TRIMMING_STEP_STANDBY,

    TRIMMING_STEP_ACTIVATE_START,
    TRIMMING_STEP_IC_PWR,
    TRIMMING_STEP_ACTIVATE_END,

    TRIMMING_STEP_TRIM_REGISTER_VERIFY,
    TRIMMING_STEP_CHANGE_OUTPUT_INIT,
    TRIMMING_STEP_CHANGE_OUTPUT,
    TRIMMING_STEP_CHANGE_OUTPUT_DONE,
    TRIMMING_STEP_CHANGE_REGISTER,
    TRIMMING_STEP_SET_ADC_CH,
    TRIMMING_STEP_START_ADC_CONVERSION,
    TRIMMING_STEP_GET_ADC_CH,
    TRIMMING_STEP_CHECK,
    TRIMMING_STEP_SCREENING,
    TRIMMING_STEP_E2P_PROGRAM,
    TRIMMING_STEP_E2P_PROGRAM_START,
    TRIMMING_STEP_E2P_PROGRAM_END,
    TRIMMING_STEP_STOP,
    TRIMMING_STEP_RESULT,
    TRIMMING_STEP_REBOOT,
    TRIMMING_STEP_COMPARE,
    TRIMMING_STEP_PWR_OFF,
    TRIMMING_STEP_MAX,
}trimming_step_t;

typedef enum
{
    SCREEN_STEP_NONE,
    SCREEN_STEP_PWR_ON,
    SCREEN_STEP_SETUP,
    SCREEN_STEP_CHANGE_OUTPUT,
    SCREEN_STEP_SET_ADC_CH,
    SCREEN_STEP_START_ADC_CONVERSION,
    SCREEN_STEP_GET_ADC_CH,
    SCREEN_STEP_STOP,
    SCREEN_STEP_MAX,
}screening_step_t;

typedef enum
{
    TRIM_ERROR_NONE = 0,
    TRIM_ERROR_OVER_COUNT,
    TRIM_ERROR_UNDER_COUNT,
    TRIM_ERROR_MAX,
}trim_error_code_t;

/* BEGIN - Input parameter from GUI */

enum
{
    TRIM_PARA_TARGET_MIN = 0,
    TRIM_PARA_TARGET_MAX,
    TRIM_PARA_P1,
    TRIM_PARA_P2,
    TRIM_PARA_MAX,
};

typedef enum
{
    TRIM_VREF_CTL = 0,
    TRIM_OSC_FREQUENCY,
    TRIM_ICTL_L_CHS,
    TRIM_ICTL_H_CHS,
    TRIM_MAX
}trim_mode_t;

#ifndef __XD12_TRIMMING_C__
//EXTERN const char* gstr_TRIM_MODE[TRIM_MAX];
#endif

typedef double (*p_gui_param)[TRIM_PARA_MAX];

enum
{
    TRIM_ALGORITHM_CONTINUE   = 0,
    TRIM_ALGORITHM_DONE_CHANNEL,
    TRIM_ALGORITHM_DONE_MODE,
    TRIM_ALGORITHM_ERROR,
    TRIM_ALGORITHM_MAX
};

#define _MIN_    0
#define _MAX_    1
/* END -  Input parameter from GUI */

extern void Trim_Calculate_Spec(void);
EXTERN void Trimming_Procedure_Run(void);
EXTERN void Screening_Procedure_Run(void);

/* BEGIN - INTERFACE FUNCTIONS */
EXTERN void Trim_IF_Trimming_Start(void);
EXTERN void Trim_IF_Screening_Start(void);
EXTERN void Trim_IF_Set_OTP_Enable(bool in_flag);
EXTERN bool Trim_IF_Get_OTP_Enable(void);

uint8_t trim_get_trim_ignore_p2_flag(void);

/* END   - INTERFACE FUNCTIONS */



#ifdef __cplusplus
}
#endif

#endif /* ~__XD12_TRIMMING_H__ */

/*** end of file ***/


