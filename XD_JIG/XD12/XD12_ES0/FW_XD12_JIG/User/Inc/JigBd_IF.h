/** @file JigBd_IF.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __JJIGBD_IF_H__
#define __JJIGBD_IF_H__

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __JJIGBD_IF_C__ 
    #define EXTERN
#else
    #define EXTERN extern
#endif


#ifdef __cplusplus
 extern "C" {
#endif

#define CURRENT_SENSE_R             (75.0f)        /* ohm */
#define CURRENT_SENSE_RIN           (1802.0f)      /* ohm */
#define CURRENT_SENSE_RO_HIGH       (2796.4f)      /* ohm */
#define CURRENT_SENSE_RO_MID        (6805.0f)      /* ohm */
#define CURRENT_SENSE_RO_LOW        (221860.0f)    /* ohm */

#define ADC_CONV_COEFF_HIGH         ((ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_HIGH * CURRENT_SENSE_R))   //Max 25mA
#define ADC_CONV_COEFF_MID          ((ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_MID * CURRENT_SENSE_R))    //Max 10mA
#define ADC_CONV_COEFF_LOW          ((ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO_LOW * CURRENT_SENSE_R))    //Max 0.3mA

#define XD12_INTERNAL_DIVIDER       (8)
#define FREQUENCY_DIVIDER           (128)

#define TIM_CAPTURE_EXT_PRECALER    (XD12_INTERNAL_DIVIDER * FREQUENCY_DIVIDER)

typedef enum
{
	GAIN_LOW = 0,   /* Max 0.3mA */
	GAIN_MID,       /* Max  10mA */
	GAIN_HIGH,      /* Max  25mA */
}current_gain_t;

typedef enum
{
	DISCHARGE = 0, 
	CHARGE,  
}current_charge_t;

EXTERN uint16_t gu16_pwm_risingBuffer[36*12];
EXTERN uint16_t gu16_pwm_fallingBuffer[36*12];

EXTERN uint8_t gu8_freq_input_capture_activated;

/* BEGIN - INTERFACE FUNCTIONS */
EXTERN void us_tdelay(uint16_t n_delay);
EXTERN void JigBd_IF_Init();
EXTERN void JigBd_IF_Vled_9V_power(uint8_t on);

EXTERN void JigBd_IF_change_current_gain(current_gain_t gain);
EXTERN void JigBd_IF_current_discharge(uint8_t mode);

EXTERN void JigBd_IF_Select_Output_Ch(uint8_t in_Output_Ch);

EXTERN void JigBd_IF_xd12_reset_Command();
EXTERN void JigBd_IF_IdGen_Command();
EXTERN void JigBd_IF_SyncGen_Command();

EXTERN uint16_t JigBd_IF_Read_Command(uint8_t in_addr);
EXTERN void JigBd_IF_Write_Command(uint8_t in_addr, uint16_t in_data);
EXTERN void JigBd_IF_LD_Trans_Command_through_PWM_to_XD12(uint16_t in_ld_width, uint16_t in_pwm);



//Read Frequency Hz
EXTERN void JigBd_IF_TIM_Capture_Start();
EXTERN void JigBd_IF_TIM_Capture_Stop();
EXTERN uint16_t JigBd_IF_Freq_Get();
EXTERN double JigBd_IF_Freq_Count_to_MHZ(uint16_t count);

EXTERN uint16_t JigBd_IF_Freq_ConvertByPrecaler(double in_freq);
EXTERN void JigBd_IF_VCC_EN_power(uint8_t on);
EXTERN void JigBd_IF_5_0V_power(power_vlot_t pwr);

EXTERN uint16_t JigBd_IF_Convert_Volt_to_VREF_ADC(double in_d_Volt);
EXTERN double JigBd_IF_Convert_VREF_ADC_to_Volt(uint16_t in_Adc);
EXTERN void JigBd_IF_VREF_ADC_StartStop();
EXTERN uint16_t JigBd_IF_VREF_ADC_Get();

/* END   - INTERFACE FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ~__JJIGBD_IF_H__ */

/*** end of file ***/


