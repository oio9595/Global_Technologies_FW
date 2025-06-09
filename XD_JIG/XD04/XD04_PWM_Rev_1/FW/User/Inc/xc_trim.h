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
#define XC24_ERR_RATE                   (1.0f/100)   /* % */
#define XC24_OSC_TARGET                 (25.0f)   	/* MHz */
#define XC24_OSC_REG_DEFAULT            (64)

#define XC24_VCTL_LDO_ERR_RATE          (1.0f/100)   /* % */
#define XC24_VCTL_LDO_TARGET            (1.5f)       /* V */
#define VCTL_LDO_UPPER_LIMIT            (XC24_VCTL_LDO_TARGET * (1.0f + XC24_VCTL_LDO_ERR_RATE))		/* V */
#define VCTL_LDO_LOWER_LIMIT            (XC24_VCTL_LDO_TARGET * (1.0f - XC24_VCTL_LDO_ERR_RATE))		/* V */

#define XC24_DAC_GAIN_ERR_RATE          (0.5f/100) /* % */
#define XC24_DAC_GAIN_TARGET            (1.1f)     /* V */
#define XC24_DAC_GAIN_P1                (1024)
#define XC24_DAC_GAIN_P2                (3072)
#define DAC_GAIN_UPPER_LIMIT            (XC24_DAC_GAIN_TARGET * (1.0f + XC24_DAC_GAIN_ERR_RATE))	/* V */
#define DAC_GAIN_LOWER_LIMIT            (XC24_DAC_GAIN_TARGET * (1.0f - XC24_DAC_GAIN_ERR_RATE))	/* V */

#define XC24_DAC_OFS_ERR_RATE           (0.5f/100)   /* % */
#define XC24_DAC_OFS_TARGET             (1.65f)      /* V */
#define XC24_DAC_OFS_TGT                (2048)
#define DAC_OFS_UPPER_LIMIT             (XC24_DAC_OFS_TARGET * (1.0f + XC24_DAC_OFS_ERR_RATE))	/* V */
#define DAC_OFS_LOWER_LIMIT             (XC24_DAC_OFS_TARGET * (1.0f - XC24_DAC_OFS_ERR_RATE))	/* V */

#define VCTL_LDO_TRIM_OVER_COUNT        (50)
#define DAC_GAIN_TRIM_OVER_COUNT        (50)
#define DAC_OFS_TRIM_OVER_COUNT         (50)
#define OSC_FCTL_TRIM_OVER_COUNT        (50)

typedef struct
{
    float dac_gain_tgt_p1;
    float dac_gain_tgt_p2;
} dac_gain_tgt_t;

typedef enum
{
    XC_TRIM_STEP_NONE = 0,
    XC_TRIM_STEP_STANDBY,

    XC_TRIM_STEP_ACTIVATE_START,
    XC_TRIM_STEP_IC_PWR,
    XC_TRIM_STEP_ACTIVATE_END,

    XC_TRIM_STEP_VCTL_LDO,
    XC_TRIM_STEP_DAC_GAIN,
    XC_TRIM_STEP_DAC_OFS,
    XC_TRIM_STEP_OSC_FCTL,

    XC_TRIM_STEP_E2P_PROGRAM,
    XC_TRIM_STEP_E2P_PROGRAM_START,
    XC_TRIM_STEP_E2P_PROGRAM_END,
    XC_TRIM_STEP_REBOOT,
    XC_TRIM_STEP_COMPARE,
    XC_TRIM_STEP_STOP,
    XC_TRIM_STEP_PWR_OFF,
    XC_TRIM_STEP_MAX,
} xc_trim_step_t;

typedef enum
{
    TRIM_FIND_MIRROR1 = 0,
    TRIM_FIND_MIRROR2,
    TRIM_FIND_MAX,
} xc_trim_find_regs_t;

extern void XC_Trim_Task(void);
extern void XC_Trim_IF_Trim_Start(void);
extern void XC_Trim_IF_Set_OTP_Enable(bool in_flag);
extern bool XC_Trim_IF_Get_OTP_Enable(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__XC_TRIM_H__ */

/*** end of file ***/