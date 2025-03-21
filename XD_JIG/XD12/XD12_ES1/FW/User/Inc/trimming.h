/** @file trimming.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __APIC_TRIMMING_H__
#define __APIC_TRIMMING_H__

#include "JigBd_IF.h"

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __APIC_TRIMMING_C__ 
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef __cplusplus
 extern "C" {
#endif

typedef enum
{
    TRIMMING_STEP_NONE = 0,
    TRIMMING_STEP_STANDBY,

    TRIMMING_STEP_ACTIVATE_START,
    TRIMMING_STEP_IC_PWR,
    TRIMMING_STEP_ATIVATE_END,

    TRIMMING_STEP_TRIM_START,
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
    TRIMMING_STEP_MAX,
}trimming_step_t;

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
    TRIM_PARA_SCR_START = 0,
    TRIM_PARA_SCR_END,
    TRIM_PARA_TARGET_MIN = 0,
    TRIM_PARA_TARGET_MAX,
    TRIM_PARA_P1,
    TRIM_PARA_P2,
    TRIM_PARA_MAX,
};

typedef enum
{
    TRIM_OSC_FREQUENCY = 0,
    TRIM_VREF_CTL,
    TRIM_OFS_CHS,
    TRIM_GAIN_CHS,
    TRIM_SCREENING,
    TRIM_MAX
}trim_mode_t;

#ifndef __APIC_TRIMMING_C__ 
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

EXTERN p_gui_param trim_get_param_gui(void);
EXTERN void trimming_procedure_run(void);

/* BEGIN - INTERFACE FUNCTIONS */
EXTERN void Trim_IF_GUI_SET_Activate(void);
EXTERN void Trim_IF_GUI_SET_TrimStart(uint32_t register_apply_flg);
EXTERN void Trim_IF_GUI_OTP_WRITE_enable(uint8_t in_val);
EXTERN void Trim_IF_GUI_WITHOUT_SCREEN(uint8_t in_val);
EXTERN void Trim_IF_GUI_TRIM_PARA_Set(double in_TrimPara[], double *gu16_TrimPara);
EXTERN void Trim_IF_GUI_NO_TRIM_enable(uint8_t in_val);

EXTERN void Trim_IF_GUI_SET_Verify_Done(void);

uint8_t trim_get_trim_ignore_p2_flag(void);
void trim_set_trim_ignore_p2_flag(uint8_t n_flag);

/* END   - INTERFACE FUNCTIONS */



#ifdef __cplusplus
}
#endif

#endif /* ~__APIC_TRIMMING_H__ */

/*** end of file ***/


