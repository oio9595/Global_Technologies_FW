/* --COPYRIGHT--,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*
 * ADS124S08.h
 *
 */
#ifndef ADS124S08_H_
#define ADS124S08_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "drv_gpio.h"

#define ADS_AIN0                (0x00U)
#define ADS_AIN1                (0x01U)
#define ADS_AIN2                (0x02U)
#define ADS_AIN3                (0x03U)
#define ADS_AIN4                (0x04U)
#define ADS_AIN5                (0x05U)
#define ADS_AIN6                (0x06U)
#define ADS_AIN7                (0x07U)
#define ADS_AIN8                (0x08U)
#define ADS_AIN9                (0x09U)
#define ADS_AIN10               (0x0AU)
#define ADS_AIN11               (0x0BU)
#define ADS_AINCOM              (0x0CU)

#define ADS114S08_CH_XD_IOUT    (ADS_AIN0)
#define ADS114S08_CH_XC_LDO     (ADS_AIN1)
#define ADS114S08_CH_XD_ICC_P   (ADS_AIN2)
#define ADS114S08_CH_XD_ICC_N   (ADS_AIN3)
#define ADS114S08_CH_XC_ICC_P   (ADS_AIN4)
#define ADS114S08_CH_XC_ICC_N   (ADS_AIN5)
#define ADS114S08_CH_XC_DAC_1   (ADS_AIN6)
#define ADS114S08_CH_XC_DAC_2   (ADS_AIN7)
#define ADS114S08_CH_XC_DAC_3   (ADS_AIN8)
#define ADS114S08_CH_XC_2uA     (ADS_AIN9)
#define ADS114S08_CH_XC_1V5     (ADS_AIN10)

#define ADS114S_VREF            (5000U)  /* 5V */
#define ADS114S_RESOLUTION      ((1U << (16 - 1)) - 1U)  /* 16bit */
#define ADC_VOLT_PER_STEP       ((double)ADS114S_VREF / ADS114S_RESOLUTION)

extern volatile uint16_t gn_ads114s08_read_timeout;

// Device command prototypes
extern void ADS114S08_Init(void);

extern void ADS114S08_Select_Input_CH(uint8_t input_p, uint8_t input_n);
extern void ADS114S08_Set_Start(bool b_start);
extern bool ADS114S08_Wait_Done(void);

extern void ADC_DRDY_INT_Handler(void);
extern uint16_t ADS114S08_Get_ADC_Value(void);

extern float JigBD_IF_Convert_Adc_To_Current(uint16_t adc, current_gain_t gain);
extern uint16_t JigBD_IF_Convert_Current_To_ADC(double current_A, current_gain_t gain);
extern float JigBD_IF_Convert_Adc_To_mVoltage(uint16_t adc);
extern float JigBD_IF_Convert_Adc_To_ICC(uint16_t adc);

#ifdef __cplusplus
}
#endif

#endif /* ADS124S08_H_ */
