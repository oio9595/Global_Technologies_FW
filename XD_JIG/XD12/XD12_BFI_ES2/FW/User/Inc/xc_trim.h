/** @file xc_trim.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XC_TRIM_H__
#define __XC_TRIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Trimming spec */
#define XC24_ERR_RATE               (1.0/100)   /* % */
#define XC24_OSC_TARGET             (25.0)   	/* MHz */
#define XC24_OSC_REG_DEFAULT        (64)

#define XC24_VCTL_LDO_ERR_RATE     	(1.0/100)   /* % */
#define XC24_VCTL_LDO_TARGET        (1.5)       /* V */
#define VCTL_LDO_UPPER_LIMIT		(XC24_VCTL_LDO_TARGET * (1.0f + XC24_VCTL_LDO_ERR_RATE))		/* V */
#define VCTL_LDO_LOWER_LIMIT  		(XC24_VCTL_LDO_TARGET * (1.0f - XC24_VCTL_LDO_ERR_RATE))		/* V */

#define XC24_DAC_GAIN_ERR_RATE      (0.5/100) /* % */
#define XC24_DAC_GAIN_TARGET        (1.1)     /* V */
#define XC24_DAC_GAIN_P1            (1024)
#define XC24_DAC_GAIN_P2            (3072)
#define DAC_GAIN_UPPER_LIMIT		(XC24_DAC_GAIN_TARGET * (1.0f + XC24_DAC_GAIN_ERR_RATE))	/* V */
#define DAC_GAIN_LOWER_LIMIT		(XC24_DAC_GAIN_TARGET * (1.0f - XC24_DAC_GAIN_ERR_RATE))	/* V */

#define XC24_DAC_OFS_ERR_RATE       (0.5/100)   /* % */
#define XC24_DAC_OFS_TARGET         (1.65)      /* V */
#define XC24_DAC_OFS_TGT            (2048)
#define DAC_OFS_UPPER_LIMIT	(XC24_DAC_OFS_TARGET * (1.0f + XC24_DAC_OFS_ERR_RATE))	/* V */
#define DAC_OFS_LOWER_LIMIT	(XC24_DAC_OFS_TARGET * (1.0f - XC24_DAC_OFS_ERR_RATE))	/* V */

#define VCTL_LDO_TRIM_OVER_COUNT       			(50)
#define DAC_GAIN_TRIM_OVER_COUNT         		(50)
#define DAC_OFS_TRIM_OVER_COUNT         		(50)
#define OSC_FCTL_TRIM_OVER_COUNT         		(50)

typedef struct
{
	float dac_gain_tgt_p1;
	float dac_gain_tgt_p2;
} dac_gain_tgt_t;

typedef enum
{
    TRIMMING_STEP_NONE = 0,
    TRIMMING_STEP_STANDBY,

    TRIMMING_STEP_ACTIVATE_START,
    TRIMMING_STEP_IC_PWR,
    TRIMMING_STEP_ACTIVATE_END,

    TRIMMING_STEP_VCTL_LDO,
	TRIMMING_STEP_DAC_GAIN,
	TRIMMING_STEP_DAC_OFS,
	TRIMMING_STEP_OSC_FCTL,

    TRIMMING_STEP_E2P_PROGRAM,
    TRIMMING_STEP_E2P_PROGRAM_START,
    TRIMMING_STEP_E2P_PROGRAM_END,
    TRIMMING_STEP_STOP,
    TRIMMING_STEP_REBOOT,
    TRIMMING_STEP_COMPARE,
    TRIMMING_STEP_PWR_OFF,
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
    TRIM_PARA_TARGET_MIN = 0,
    TRIM_PARA_TARGET_MAX,
    TRIM_PARA_P1,
    TRIM_PARA_P2,
    TRIM_PARA_MAX,
};

typedef enum
{
	TRIM_FIND_MIRROR1 = 0,
	TRIM_FIND_MIRROR2,
	TRIM_FIND_MAX,
}xc_trim_find_regs_t;

typedef double (*p_gui_param)[TRIM_PARA_MAX];

enum
{
    TRIM_ALGORITHM_CONTINUE   = 0,
    TRIM_ALGORITHM_DONE_CHANNEL,
    TRIM_ALGORITHM_DONE_MODE,
    TRIM_ALGORITHM_ERROR,
    TRIM_ALGORITHM_MAX
};

/* END -  Input parameter from GUI */

extern void XC_Trimming_Procedure_Run(void);
extern void Screening_Procedure_Run(void);

/* BEGIN - INTERFACE FUNCTIONS */
extern void Trim_IF_Trimming_Start(void);
extern void Trim_IF_Set_OTP_Enable(bool in_flag);
extern bool Trim_IF_Get_OTP_Enable(void);
void GET_MINIMUM_OSC_FREQ(float* freq_buffer, float target);

/* END   - INTERFACE FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ~__XC_TRIM_H__ */

/*** end of file ***/


