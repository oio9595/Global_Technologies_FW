/** @file JigBd_IF.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __JIG_BD_IF_H__
#define __JIG_BD_IF_H__

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

#define XDIC_INTERNAL_DIVIDER       (32)        // BFI 16, PWM 32
#define XC24_INTERNAL_DIVIDER       (32)
#define JIG_FREQUENCY_DIVIDER       (128)
#define XDIC_CONST_FREQ_DIVIDE      (XDIC_INTERNAL_DIVIDER * JIG_FREQUENCY_DIVIDER)
#define XC24_CONST_FREQ_DIVIDE      (XC24_INTERNAL_DIVIDER * JIG_FREQUENCY_DIVIDER)

typedef enum tag_CURRENT_GAIN_T
{
	GAIN_LOW = 0,   /* Max 0.5mA */
	GAIN_MID,       /* Max  10mA */
	GAIN_HIGH,      /* Max 140mA */
	GAIN_MAX,
} current_gain_t;

extern volatile uint16_t gn_xd_rx_timeout;

extern bool gb_timer_input_capture_activated;
extern volatile bool gb_timer_input_capture_done;

/* BEGIN - INTERFACE FUNCTIONS */
extern void us_delay(uint16_t n_delay);
extern void JigBD_IF_Detect_XC24(void);
extern void JigBD_IF_XC_VCC_EN(uint8_t on);
extern void JigBD_IF_XD_VCC_EN(uint8_t on);
extern void JigBD_IF_XD_VCC_Level(power_volt_t pwr);
extern void JigBD_IF_XC_VCC_Level(power_volt_t pwr);
extern void JigBD_IF_VLED_9V_EN(uint8_t on);

extern void JigBD_IF_Change_Current_Gain(current_gain_t gain);
extern void JigBD_IF_Select_Output_Ch(uint8_t in_output_ch);

extern void JigBD_IF_Reset_Command(void);
extern void JigBD_IF_IdGen_Command(void);
extern void JigBD_IF_SyncGen_Command(void);

extern void JigBD_IF_Write_Command(uint8_t in_addr, uint16_t in_data);
extern void JigBD_IF_Write_LD_Command(uint16_t in_LD_data);
extern uint16_t JigBD_IF_Read_Command(uint8_t in_addr);
extern uint16_t JigBD_IF_Fault_Read_Command(void);

//Read Frequency Hz
extern void JigBD_IF_Link_DMA_With_Buffer(void);
extern void JigBD_IF_Start_Input_Capture(void);
extern void JigBD_IF_Stop_Input_Capture(void);
extern double JigBD_IF_Get_Input_Capture_Freq(void);
extern void JigBD_IF_Calculate_Input_Capture_Freq(void);

extern uint16_t JigBD_IF_Calculate_XDIC_Divided_Freq(double in_freq);
extern double JigBD_IF_Reconvert_XDIC_Original_Freq(double count);

extern uint16_t JigBD_IF_Convert_Volt_To_MCU_ADC(double in_volt);
extern double JigBD_IF_Convert_MCU_ADC_To_Volt(uint16_t in_adc);
extern void JigBD_IF_Start_MCU_ADC(void);
extern uint16_t JigBD_IF_Get_MCU_ADC(void);

extern void MCU_IF_Set_XDIC_Channel(uint8_t in_channel);

/* END   - INTERFACE FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ~__JIG_BD_IF_H__ */

/*** end of file ***/


