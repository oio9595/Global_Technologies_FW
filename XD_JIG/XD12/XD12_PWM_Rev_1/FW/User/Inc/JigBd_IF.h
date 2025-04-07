/** @file JigBd_IF.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __JIG_BD_IF_H__
#define __JIG_BD_IF_H__

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __JIG_BD_IF_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif


#ifdef __cplusplus
 extern "C" {
#endif

#include "types.h"

#define CURRENT_SENSE_R_LOW         (3900.0f)   /* ohm */
#define CURRENT_SENSE_R_MID         (200.0f)    /* ohm */
#define CURRENT_SENSE_R_HIGH        (13.0f)     /* ohm */
#define CURRENT_SENSE_RIN           (2200.0f)   /* ohm */
#define CURRENT_SENSE_RO            (3300.0f)   /* ohm */

#define ADC_CONV_COEFF_LOW          ((ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO * CURRENT_SENSE_R_LOW))    // Max 0.5mA
#define ADC_CONV_COEFF_MID          ((ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO * CURRENT_SENSE_R_MID))    // Max  10mA
#define ADC_CONV_COEFF_HIGH         ((ADC_VOLT_PER_STEP * CURRENT_SENSE_RIN) / (CURRENT_SENSE_RO * CURRENT_SENSE_R_HIGH))   // Max 140mA

#define CURRENT_LIMIT_GAIN_LOW      (0.4f)  //Max 0.5mA
#define CURRENT_LIMIT_GAIN_MID      (9.0f)  //Max  10mA
#define CURRENT_LIMIT_GAIN_HIGH     (70.0f) //MAX 140mA

#define XD12_INTERNAL_DIVIDER       (32)
#define FREQUENCY_DIVIDER           (128)
#define TIM_CAPTURE_EXT_PRESCALER   (XD12_INTERNAL_DIVIDER * FREQUENCY_DIVIDER)

#define APB1_TIM_FREQ               (72.0) //MHz

typedef enum
{
	GAIN_LOW = 0,   /* Max 0.5mA */
	GAIN_MID,       /* Max  10mA */
	GAIN_HIGH,      /* Max 140mA */
	GAIN_MAX,
}current_gain_t;

typedef enum
{
	DISCHARGE = 0,
	CHARGE,
}current_charge_t;

EXTERN uint16_t gu16_pwm_tx_risingBuffer[36*12];
EXTERN uint16_t gu16_pwm_rx_risingBuffer[36*12];
EXTERN uint16_t gu16_pwm_rx_fallingBuffer[36*12];

EXTERN volatile uint16_t gn_xd_rx_timeout;

EXTERN bool gb_timer_input_capture_activated;
EXTERN bool gb_timer_input_capture_done;

/* BEGIN - INTERFACE FUNCTIONS */
EXTERN void us_tdelay(uint16_t n_delay);
EXTERN void JigBD_IF_Detect_XC24(void);
EXTERN void JigBD_IF_XC_VCC_EN(uint8_t on);
EXTERN void JigBD_IF_XD_VCC_EN(uint8_t on);
EXTERN void JigBD_IF_XD_VCC_Level(power_volt_t pwr);
EXTERN void JigBD_IF_VLED_9V_EN(uint8_t on);

EXTERN void JigBD_IF_Change_Current_Gain(current_gain_t gain);
EXTERN void JigBD_IF_Select_Output_Ch(uint8_t in_output_ch);

EXTERN void JigBD_IF_Reset_Command(void);
EXTERN void JigBD_IF_IdGen_Command(void);
EXTERN void JigBD_IF_SyncGen_Command(void);

EXTERN void JigBD_IF_Write_Command(uint8_t in_addr, uint16_t in_data);
EXTERN void JigBD_IF_Write_LD_Command(uint16_t in_LD_data);
EXTERN uint16_t JigBD_IF_Read_Command(uint8_t in_addr);
EXTERN uint16_t JigBD_IF_Fault_Read_Command(void);

//Read Frequency Hz
EXTERN void JigBD_IF_Input_Capture_Start(void);
EXTERN void JigBD_IF_Input_Capture_Stop(void);
EXTERN uint16_t JigBD_IF_Input_Capture_Get_Freq(void);
EXTERN void JigBD_IF_Input_Capture_Calculate_Freq(void);

EXTERN uint16_t JigBD_IF_Freq_ConvertByPrescaler(double in_freq);
EXTERN double JigBD_IF_Freq_Count_to_MHZ(uint16_t count);

EXTERN uint16_t JigBD_IF_Convert_Volt_to_VREF_ADC(double in_volt);
EXTERN double JigBD_IF_Convert_VREF_ADC_to_Volt(uint16_t in_adc);
EXTERN void JigBD_IF_VREF_ADC_StartStop(void);
EXTERN uint16_t JigBD_IF_VREF_ADC_Get(void);

EXTERN void MCU_IF_Set_XD12_Channel(uint8_t in_channel);

/* END   - INTERFACE FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ~__JIG_BD_IF_H__ */

/*** end of file ***/


