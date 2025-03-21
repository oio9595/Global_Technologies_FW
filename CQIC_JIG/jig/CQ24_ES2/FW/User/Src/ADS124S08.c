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
 * ADS124S08.c
 *
 */

/* Also include device specific header */
#include "ADS124S08.h"
#include "log.h"

extern SPI_HandleTypeDef hspi1;
#define USE_DISPLAY_DEVICE_REGS

/* Global variables */
uint8_t registers[REG_ADDR_MAX] =
{
    0x00, 0x80, 0x01, 0x00, 0x14, 0x10, 0x00, 0xFF, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
};

extern SPI_HandleTypeDef hspi2;
#if 1
static uint8_t xferWord(SPI_HandleTypeDef *hspi, uint8_t tx)
{
    uint8_t rx = 0;
#if 1
    HAL_SPI_TransmitReceive(hspi, &tx, &rx, 1, 10);
#else
    /* Set up data for the next xmit */
    while(!(hspi->Instance->SR & SPI_SR_TXE));
    *(volatile uint8_t *)&(hspi->Instance->DR) = tx;
    /* Wait for data to appear */
    while(!(hspi->Instance->SR & SPI_SR_RXNE));
    /* Grab that data*/
    rx = *(volatile uint8_t *)&(hspi->Instance->DR);
#endif

    return rx;
}
#endif

#ifdef USE_DISPLAY_DEVICE_REGS
static char* gp_delay_string[] = {
    "000 : 14 * tMOD (default)",
    "001 : 25 * tMOD",
    "010 : 64 * tMOD",
    "011 : 256 * tMOD",
    "100 : 1024 * tMOD",
    "101 : 2048 * tMOD",
    "110 : 4096 * tMOD",
    "111 : 1 * tMOD",
};

static char* gp_gain_string[] = {
    "000 : 1 (default)",
    "001 : 2",
    "010 : 4",
    "011 : 8",
    "100 : 16",
    "101 : 32",
    "110 : 64",
    "111 : 128",
};

static char* gp_datarate_string[] = {
    "0000 : 2.5 SPS",
    "0001 : 5 SPS",
    "0010 : 10 SPS",
    "0011 : 16.6 SPS",
    "0100 : 20 SPS (default)",
    "0101 : 50SPS",
    "0110 : 60 SPS",
    "0111 : 100 SPS",
    "1000 : 200 SPS",
    "1001 : 400 SPS",
    "1010 : 800 SPS",
    "1011 : 1000 SPS",
    "1100 : 2000 SPS",
    "1101 : 4000 SPS",
    "1110 : 4000 SPS",
    "1111 : Reserved",
};

static char* gp_ref_monitoring_config_string[] = {
    "00 : Disabled (default)",
    "01 : FL_REF_L0 monitor enabled, threshold 0.3 V",
    "10 : FL_REF_L0 and FL_REF_L1 monitors enabled, thresholds 0.3 V and 1/3 · (AVDD – AVSS)",
    "11 : FL_REF_L0 monitor and 10-MΩ pull-together enabled, threshold 0.3 V",
};

static char* gp_ref_input_selection_string[] = {
    "00 : REFP0, REFN0 (default)",
    "01 : REFP1, REFN1",
    "10 : Internal 2.5-V reference(1)",
    "11 : Reserved",
};

static char* gp_internal_voltage_ref_config_string[] = {
    "00 : Internal reference off (default)",
    "01 : Internal reference on, but powers down in power-down mode",
    "10 : Internal reference is always on, even in power-down mode",
    "11 : Reserved",
};

static char* gp_idac_magnitude_selection_string[] = {
    "0000 : Off (default)",
    "0001 : 10 μA",
    "0010 : 50 μA",
    "0011 : 100 μA",
    "0100 : 250 μA",
    "0101 : 500 μA",
    "0110 : 750 μA",
    "0111 : 1000 μA",
    "1000 : 1500 μA",
    "1001 : 2000 μA",
    "1010 : Off",
    "1011 : Off",
    "1100 : Off",
    "1101 : Off",
    "1110 : Off",
    "1111 : Off",
};

static char* gp_idac_output_channel_selection_string[] = {
    "0000 : AIN0",
    "0001 : AIN1",
    "0010 : AIN2",
    "0011 : AIN3",
    "0100 : AIN4",
    "0101 : AIN5",
    "0110 : AIN6 (ADS114S08 only), REFP1 (ADS114S06)",
    "0111 : AIN7 (ADS114S08 only), REFN1 (ADS114S06)",
    "1000 : AIN8 (ADS114S08 only)",
    "1001 : AIN9 (ADS114S08 only)",
    "1010 : AIN10 (ADS114S08 only)",
    "1011 : AIN11 (ADS114S08 only)",
    "1100 : AINCOM",
    "1101 : Disconnected (default)",
    "1110 : Disconnected (default)",
    "1111 : Disconnected (default)",
};

static char* gp_input_multiplexer_selection_string[] = {
    "0000 : AIN0 (default)",
    "0001 : AIN1",
    "0010 : AIN2",
    "0011 : AIN3",
    "0100 : AIN4",
    "0101 : AIN5",
    "0110 : AIN6 (ADS114S08 only)",
    "0111 : AIN7 (ADS114S08 only)",
    "1000 : AIN8 (ADS114S08 only)",
    "1001 : AIN9 (ADS114S08 only)",
    "1010 : AIN10 (ADS114S08 only)",
    "1011 : AIN11 (ADS114S08 only)",
    "1100 : AINCOM",
    "1101 : Reserved",
    "1110 : Reserved",
    "1111 : Reserved",
};

static char* gp_sys_monitor_config_string[] = {
    "000 : Disabled (default)",
    "001 : PGA inputs shorted to (AVDD + AVSS) / 2 and disconnected from AINx and the multiplexer; gain set by user",
    "010 : Internal temperature sensor measurement; PGA must be enabled (PGA_EN[1:0] = 01); gain set by user",
    "011 : (AVDD – AVSS) / 4 measurement; gain set to 1",
    "100 : DVDD / 4 measurement; gain set to 1",
    "101 : Burn-out current sources enabled, 0.2-μA setting",
    "110 : Burn-out current sources enabled, 1-μA setting",
    "111 : Burn-out current sources enabled, 10-μA setting",
};

static char* gp_calibration_sample_size_selection_string[] = {
    "00 : 1 sample",
    "01 : 4 samples",
    "10 : 8 samples (default)",
    "11 : 16 samples",
};

static void dump_regs(void)
{
    print("======== ADS114S08 regs value ========\r\n");
    {
        ads114s08_id_t id;
        ads114s08_staus_t status;
        ads114s08_inpmux_t inpmux;
        ads114s08_pga_t gain;
        ads114s08_datarate_t datarate;
        ads114s08_ref_t ref_ctrl;
        ads114s08_idacmag_t idacmag;
        ads114s08_idacmux_t idacmux;
        ads114s08_vbias_t vbias;
        ads114s08_sys_t sys;
        ads114s08_ofcal0_t ofcal0;
        ads114s08_ofcal0_t ofcal1;
        ads114s08_fscal0_t fscal0;
        ads114s08_fscal0_t fscal1;
        ads114s08_gpiodat_t gpiodat;
        ads114s08_gpiocon_t gpiocon;

        snprintf(msg_buffer, sizeof msg_buffer, "Device #1\r\n");
        print(msg_buffer);

        id.value = registers[REG_ADDR_ID];
        snprintf(msg_buffer, sizeof msg_buffer, "\tDevice identifier : 0x%X(%s)\r\n", id.u.dev_id, (id.u.dev_id == 0x04 ? "ADS114S08" : (id.u.dev_id == 0x05 ? "ADS114S06" : "UNKNOWN")));
        print(msg_buffer);

        status.value = registers[REG_ADDR_STATUS];
        print("\tDevice Status\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tPOR flag : %s\r\n", (status.u.fl_por == 0x00 ? "0 : Register has been cleared and no POR event has occurred." : "1 : POR event occurred and has not been cleared. Flag must be cleared by user register write (default)."));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tDevice ready flag : %s\r\n", (status.u.rdy == 0x00 ? "0 : ADC ready for communication (default)" : "1 : ADC not ready"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tPositive PGA output at positive rail flag : %s\r\n", (status.u.fl_p_railp == 0x00 ? "0 : No error (default)" : "1 : PGA positive output within 150 mV of AVDD"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tPositive PGA output at negative rail flag : %s\r\n", (status.u.fl_p_railn == 0x00 ? "0 : No error (default)" : "1 : PGA positive output within 150 mV of AVSS"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tNegative PGA output at positive rail flag : %s\r\n", (status.u.fl_n_railp == 0x00 ? "0 : No error (default)" : "1 : PGA negative output within 150 mV of AVDD"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tNegative PGA output at negative rail flag : %s\r\n", (status.u.fl_n_railn == 0x00 ? "0 : No error (default)" : "1 : PGA negative output within 150 mV of AVSS"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tReference voltage monitor flag, level 1 : %s\r\n", (status.u.fl_ref_l1 == 0x00 ? "0 : Differential reference voltage ≥ 1/3 · (AVDD - AVSS) (default)" : "1 : Differential reference voltage < 1/3 · (AVDD - AVSS)"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tReference voltage monitor flag, level 0 : %s\r\n", (status.u.fl_ref_l0 == 0x00 ? "0 : Differential reference voltage ≥ 0.3 V (default)" : "1 : Differential reference voltage < 0.3 V"));
        print(msg_buffer);

        inpmux.value = registers[REG_ADDR_INPMUX];
        print("\tInput Multiplexer\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tPositive ADC input selection : %s\r\n", gp_input_multiplexer_selection_string[inpmux.u.muxp]);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tNegative ADC input selection : %s\r\n", gp_input_multiplexer_selection_string[inpmux.u.muxn]);
        print(msg_buffer);

        gain.value = registers[REG_ADDR_PGA];
        print("\tGain Setting\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tProgrammable conversion delay selection : %s\r\n", gp_delay_string[gain.u.delay]);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tPGA enable : %s\r\n", (gain.u.pga_en == 0x00 ? "00 : PGA is powered down and bypassed. Enables single-ended measurements with unipolar supply (Set gain = 1(2)) (default)" : (gain.u.pga_en == 0x01 ? "01 : PGA enabled (gain = 1 to 128)" : "Reserved")));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tPGA gain selection : %s\r\n", gp_gain_string[gain.u.gain]);
        print(msg_buffer);

        datarate.value = registers[REG_ADDR_DATARATE];
        print("\tData Rate\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tGlobal chop enable : %s\r\n", (datarate.u.g_chop == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tClock source selection : %s\r\n", (datarate.u.clk == 0x00 ? "0 : Internal 4.096-MHz oscillator (default)" : "1 : External clock"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tConversion mode selection : %s\r\n", (datarate.u.mode == 0x00 ? "0 : Continuous conversion mode (default)" : "1 : Single-shot conversion mode"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tDigital filter selection : %s\r\n", (datarate.u.filter == 0x00 ? "0 : Sinc3 filter" : "1 : Low-latency filter (default)"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tData rate selection : %s\r\n", gp_datarate_string[datarate.u.dr]);
        print(msg_buffer);

        ref_ctrl.value = registers[REG_ADDR_REF];
        print("\tReference Control (REF)\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tReference monitor configuration : %s\r\n", gp_ref_monitoring_config_string[ref_ctrl.u.fl_ref_en]);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tPositive reference buffer bypass : %s\r\n", (ref_ctrl.u.refp_buf == 0x00 ? "0 : Enabled (default)" : "1 : Disabled"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tNegative reference buffer bypass : %s\r\n", (ref_ctrl.u.refn_buf == 0x00 ? "0 : Enabled" : "1 : Disabled (default)"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tReference input selection : %s\r\n", gp_ref_input_selection_string[ref_ctrl.u.refsel]);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tInternal voltage reference configuration : %s\r\n", gp_internal_voltage_ref_config_string[ref_ctrl.u.refsel]);
        print(msg_buffer);

        idacmag.value = registers[REG_ADDR_IDACMAG];
        print("\tExcitation Current #1\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tPGA output rail flag enable : %s\r\n", (idacmag.u.fl_rail_en == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tLow-side power switch : %s\r\n", (idacmag.u.psw == 0x00 ? "0 : Open (default)" : "1 : Closed"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tIDAC magnitude selection : %s\r\n", gp_idac_magnitude_selection_string[idacmag.u.imag]);
        print(msg_buffer);
    
        idacmux.value = registers[REG_ADDR_IDACMUX];
        print("\tExcitation Current #2\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tIDAC2 output channel selection : %s\r\n", gp_idac_output_channel_selection_string[idacmux.u.i2mux]);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tIDAC1 output channel selection : %s\r\n", gp_idac_output_channel_selection_string[idacmux.u.i1mux]);
        print(msg_buffer);

        vbias.value = registers[REG_ADDR_VBIAS];
        print("\tSensor Biasing\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tVBIAS level selection : %s\r\n", (vbias.u.vb_level == 0x00 ? "0 : (AVDD + AVSS) / 2 (default)" : "1 : (AVDD + AVSS) / 12"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tAINCOM VBIAS selection : %s\r\n", (vbias.u.vb_ainc == 0x00 ? "0 : VBIAS disconnected from AINCOM (default)" : "1 : VBIAS connected to AINCOM"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tAIN5 VBIAS selection : %s\r\n", (vbias.u.vb_ain5 == 0x00 ? "0 : VBIAS disconnected from AIN5 (default)" : "1 : VBIAS connected to AIN5"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tAIN4 VBIAS selection : %s\r\n", (vbias.u.vb_ain4 == 0x00 ? "0 : VBIAS disconnected from AIN4 (default)" : "1 : VBIAS connected to AIN4"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tAIN3 VBIAS selection : %s\r\n", (vbias.u.vb_ain3 == 0x00 ? "0 : VBIAS disconnected from AIN3 (default)" : "1 : VBIAS connected to AIN3"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tAIN2 VBIAS selection : %s\r\n", (vbias.u.vb_ain2 == 0x00 ? "0 : VBIAS disconnected from AIN2 (default)" : "1 : VBIAS connected to AIN2"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tAIN1 VBIAS selection : %s\r\n", (vbias.u.vb_ain1 == 0x00 ? "0 : VBIAS disconnected from AIN1 (default)" : "1 : VBIAS connected to AIN1"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tAIN0 VBIAS selection : %s\r\n", (vbias.u.vb_ain0 == 0x00 ? "0 : VBIAS disconnected from AIN0 (default)" : "1 : VBIAS connected to AIN0"));
        print(msg_buffer);

        sys.value = registers[REG_ADDR_SYS];
        print("\tSystem Control\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tSystem monitor configuration : %s\r\n", gp_sys_monitor_config_string[sys.u.sys_mon]);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tCalibration sample size selection : %s\r\n", gp_calibration_sample_size_selection_string[sys.u.cal_samp]);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tSPI timeout enable : %s\r\n", (sys.u.timeout == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tCRC enable : %s\r\n", (sys.u.timeout == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tSTATUS byte enable : %s\r\n", (sys.u.timeout == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));
        print(msg_buffer);
    
        ofcal0.value = registers[REG_ADDR_OFCAL0];
        snprintf(msg_buffer, sizeof msg_buffer, "\tOffset Calibration #1 : %u\r\n", ofcal0.value);
        print(msg_buffer);
        ofcal1.value = registers[REG_ADDR_OFCAL1];
        snprintf(msg_buffer, sizeof msg_buffer, "\tOffset Calibration #2 : %u\r\n", ofcal1.value);
        print(msg_buffer);
        fscal0.value = registers[REG_ADDR_FSCAL0];
        snprintf(msg_buffer, sizeof msg_buffer, "\tGain Calibration #1 : %u\r\n", fscal0.value);
        print(msg_buffer);
        fscal1.value = registers[REG_ADDR_FSCAL1];
        snprintf(msg_buffer, sizeof msg_buffer, "\tGain Calibration #2 : %u\r\n", fscal1.value);
        print(msg_buffer);

        gpiodat.value = registers[REG_ADDR_GPIODAT];
        print("\tGPIO Data\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tGPIO direction : 0x%X\r\n", gpiodat.u.dir);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tGPIO data : 0x%X\r\n", gpiodat.u.dat);
        print(msg_buffer);

        gpiocon.value = registers[REG_ADDR_GPIOCON];
        print("\tGPIO Configuration\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "\t\tGPIO pin configuration : 0x%X\r\n", gpiocon.u.con);
        print(msg_buffer);

    }
    print("======================================\r\n");
}
#endif

static void us_delay(uint32_t n_delay) /* not 1us delay */
{
    uint32_t i = 0;
    for( ; n_delay != 0 ; --n_delay)
    {
        for(i=0 ; i<1 ; ++i)
        {
            asm("NOP");
        }
    }
}

/*
 * Writes the nCS pin low and waits a while for the Tiva to finish working before
 * handing control back to the caller for a SPI transfer.
 */
static void ads114s08_cs_low(adc_ch_t ch)
{
	if((ch == ADC1_CS) || (ch == ADC_CS_ALL))
	{
		HAL_GPIO_WritePin(ADC1_CS_GPIO_Port, ADC1_CS_Pin, GPIO_PIN_RESET);
	}
		
	if((ch == ADC2_CS) || (ch == ADC_CS_ALL))
	{
		HAL_GPIO_WritePin(ADC2_CS_GPIO_Port, ADC2_CS_Pin, GPIO_PIN_RESET);
	}
}

/*
 * Pulls the nCS pin high. Performs no waiting.
 */
static void ads114s08_cs_high(adc_ch_t ch)
{
#if 0
	if((ch == ADC1_CS) || (ch == ADC_CS_ALL))
	{
		HAL_GPIO_WritePin(ADC1_CS_GPIO_Port, ADC1_CS_Pin, GPIO_PIN_SET);
	}

	
	if((ch == ADC2_CS) || (ch == ADC_CS_ALL))
	{
		HAL_GPIO_WritePin(ADC2_CS_GPIO_Port, ADC2_CS_Pin, GPIO_PIN_SET);
	}
#else
	if(ch == ADC1_CS)
	{
		HAL_GPIO_WritePin(ADC1_CS_GPIO_Port, ADC1_CS_Pin, GPIO_PIN_SET);
	}

	
	if(ch == ADC2_CS)
	{
		HAL_GPIO_WritePin(ADC2_CS_GPIO_Port, ADC2_CS_Pin, GPIO_PIN_SET);
	}

#endif
}
/*
 * Initializes device for use in the ADS124S08 EVM.
 *
 * \return True if device is in correct hardware defaults and is connected
 *
 */
void ads114s08_init(void)
{
	/* ADC1 Init */
    for(uint8_t reg=REG_ADDR_ID ; reg < REG_ADDR_MAX ; ++reg)
    {
        if(reg == REG_ADDR_RESERVED1 || reg == REG_ADDR_RESERVED2)
        {
        }
        else
        {
            registers[reg] = ads114s08_get_reg(reg, ADC1_CS);

            if(reg == REG_ADDR_ID)
            {
                if(registers[reg] == ADS_114S08)
                {
                }
                else
                {
                }
            }
            else if(reg == REG_ADDR_STATUS)
            {
                ads114s08_staus_t status;
                status.value = registers[reg];

                if(status.u.fl_por == 1)
                {
                    status.u.fl_por = 0;
                    ads114s08_set_reg(reg, status.value, ADC1_CS);
                    registers[reg] = ads114s08_get_reg(reg, ADC1_CS);
                }
            }
            else if(reg == REG_ADDR_INPMUX)
            {
#if 0
                ads114s08_inpmux_t t.value = registers[REG_ADDR_INPMUX];
                if(t.u.muxn != ADS_N_AINCOM)
                {
                    t.u.muxn = ADS_N_AINCOM;
                    ads114s08_set_reg(reg, t.value);

                    registers[reg] = ads114s08_set_reg(reg);
                }
#endif
            }
			else if(reg == REG_ADDR_PGA )
			{
#if 0		
				ads114s08_pga_t	pga;
				pga.value = registers[reg];

				pga.u.pga_en = 0;
				pga.u.gain = 0;
				pga.u.delay = 0;

				ads114s08_set_reg(reg, pga.value, ADC1_CS);
				registers[reg] = ads114s08_get_reg(reg, ADC1_CS);				
#endif				
			}
            else if(reg == REG_ADDR_DATARATE)
            {
#if 1
                ads114s08_datarate_t datarate;
                datarate.value = registers[reg];

#if 1
                if(datarate.u.dr != 9)  /* DatyaRate : 400 SPS, Low-Latency filter : 1, ? conversion mode */
                {
                    datarate.u.dr = 9;
                    //datarate.u.filter = 1;
                    //datarate.u.mode = 1;

                    ads114s08_set_reg(reg, datarate.value, ADC1_CS);
                    registers[reg] = ads114s08_get_reg(reg, ADC1_CS);
                }
#else
				datarate.u.dr = 9;
				//datarate.u.filter = 1;
				datarate.u.mode = 1;
				
				ads114s08_set_reg(reg, datarate.value, ADC1_CS);
				registers[reg] = ads114s08_get_reg(reg, ADC1_CS);
#endif
#endif
            }
        }
    }

    //ads114s08_set_command(dev, CMD_WAKEUP);

#ifdef USE_DISPLAY_DEVICE_REGS
	print("======== ADS114S08-1 regs value ========\r\n");
    dump_regs();
#endif

	/* ADC2 Init */
	for(uint8_t reg=REG_ADDR_ID ; reg < REG_ADDR_MAX ; ++reg)
	{
		if(reg == REG_ADDR_RESERVED1 || reg == REG_ADDR_RESERVED2)
		{
		}
		else
		{
			registers[reg] = ads114s08_get_reg(reg, ADC2_CS);

			if(reg == REG_ADDR_ID)
			{
				if(registers[reg] == ADS_114S08)
				{
				}
				else
				{
				}
			}
			else if(reg == REG_ADDR_STATUS)
			{
				ads114s08_staus_t status;
				status.value = registers[reg];

				if(status.u.fl_por == 1)
				{
					status.u.fl_por = 0;
					ads114s08_set_reg(reg, status.value, ADC2_CS);
					registers[reg] = ads114s08_get_reg(reg, ADC2_CS);
				}
			}
			else if(reg == REG_ADDR_INPMUX)
			{
#if 0
				ads114s08_inpmux_t t.value = registers[REG_ADDR_INPMUX];
				if(t.u.muxn != ADS_N_AINCOM)
				{
					t.u.muxn = ADS_N_AINCOM;
					ads114s08_set_reg(reg, t.value);

					registers[reg] = ads114s08_set_reg(reg);
				}
#endif
			}
			else if(reg == REG_ADDR_DATARATE)
			{
#if 1
				ads114s08_datarate_t datarate;
				datarate.value = registers[reg];

				if(datarate.u.dr != 9)	/* DatyaRate : 400 SPS, Low-Latency filter : 1, ? conversion mode */
				{
					datarate.u.dr = 9;
					//datarate.u.filter = 1;
					//datarate.u.mode = 1;

					ads114s08_set_reg(reg, datarate.value, ADC2_CS);
					registers[reg] = ads114s08_get_reg(reg, ADC2_CS);
				}
#endif
			}
		}
	}

	//ads114s08_set_command(dev, CMD_WAKEUP);

#ifdef USE_DISPLAY_DEVICE_REGS
	print("======== ADS114S08-2 regs value ========\r\n");
	dump_regs();
#endif

}
/*
 * Reads a single register contents from the specified address
 *
 * \param regnum identifies which address to read
 *
 */
uint8_t ads114s08_get_reg(unsigned int regnum, adc_ch_t ch)
{
    uint8_t i = 0;
    uint8_t ulDataTx[3] = {0, };
    uint8_t ulDataRx[3] = {0, };

    ulDataTx[0] = CMD_REGRD + (regnum & 0x1f);
    ulDataTx[1] = 0x00;
    ulDataTx[2] = 0x00;

    ads114s08_cs_low(ch);

    for(i = 0; i < 3; i++)
    {
        ulDataRx[i] = xferWord(&hspi1, ulDataTx[i]);
    }
    if(regnum < REG_ADDR_MAX)
    {
        registers[regnum] = ulDataRx[2];
    }
    ads114s08_cs_high(ch);

    return ulDataRx[2];
}
/*
 * Reads a group of registers starting at the specified address
 *
 * \param regnum is addr_mask 8-bit mask of the register from which we start reading
 * \param count The number of registers we wish to read
 * \param *location pointer to the location in memory to write the data
 *
 */
void ads114s08_get_regs(uint8_t regnum, uint8_t count, uint8_t *data, adc_ch_t ch)
{
    uint8_t i = 0;
    uint8_t ulDataTx[2] = {0, };

    ulDataTx[0] = CMD_REGRD + (regnum & 0x1f);
    ulDataTx[1] = count-1;

    ads114s08_cs_low(ch);

    xferWord(&hspi1, ulDataTx[0]);
    xferWord(&hspi1, ulDataTx[1]);
    for(i = 0; i < count; i++)
    {
        data[i] = xferWord(&hspi1, 0);
        if(regnum+i < REG_ADDR_MAX)
        {
            registers[regnum+i] = data[i];
        }
    }
    ads114s08_cs_high(ch);
}
/*
 * Writes a single of register with the specified data
 *
 * \param regnum addr_mask 8-bit mask of the register to which we start writing
 * \param data to be written
 *
 */
void ads114s08_set_reg(uint8_t regnum, uint8_t data, adc_ch_t ch)
{
    uint8_t ulDataTx[3];

    ulDataTx[0] = CMD_REGWR + (regnum & 0x1f);
    ulDataTx[1] = 0x00; /* the number of registers to write (minus 1): 000n nnnn */
    ulDataTx[2] = data;

    ads114s08_cs_low(ch);
    xferWord(&hspi1, ulDataTx[0]);
    xferWord(&hspi1, ulDataTx[1]);
    xferWord(&hspi1, ulDataTx[2]);
    ads114s08_cs_high(ch);
}
/*
 * Writes a group of registers starting at the specified address
 *
 * \param regnum is addr_mask 8-bit mask of the register from which we start writing
 * \param count The number of registers we wish to write
 * \param *location pointer to the location in memory to read the data
 *
 */
void  ads114s08_set_regs(uint8_t regnum, uint8_t howmuch, uint8_t *data, adc_ch_t ch)
{
    uint8_t i = 0;
    uint8_t ulDataTx[2];

    ulDataTx[0] = CMD_REGWR + (regnum & 0x1f);
    ulDataTx[1] = howmuch-1;

    ads114s08_cs_low(ch);
    xferWord(&hspi1, ulDataTx[0]);
    xferWord(&hspi1, ulDataTx[1]);
    for(i=0; i < howmuch; i++)
    {
        xferWord(&hspi1, data[i]);
        if(regnum+i < REG_ADDR_MAX)
        {
            registers[regnum+i] = data[i];
        }
    }
    ads114s08_cs_high(ch);
}

void ads114s08_set_input(uint8_t input_p, uint8_t input_n, adc_ch_t ch)
{
    ads114s08_inpmux_t t = {0,};

    t.u.muxp = input_p;
    t.u.muxn = input_n;
    
    ads114s08_set_reg(REG_ADDR_INPMUX, t.value, ch);
}

extern void cq24_ch_select(uint8_t in, adc_ch_t ch);

void ads114s08_select_single_ended_input(uint8_t input, adc_ch_t ch)
{
#if 0
    snprintf(msg_buffer, sizeof msg_buffer, "\n\r input [%5d]", input);
    print(msg_buffer);
#endif

		cq24_ch_select(input, ch);

    switch(input)
    {
    case 0:
        ads114s08_set_input(ADS_AIN0, ADS_AINCOM, ch);
        break;
    case 1:
        ads114s08_set_input(ADS_AIN1, ADS_AINCOM, ch);
        break;
    case 2:
        ads114s08_set_input(ADS_AIN2, ADS_AINCOM, ch);
        break;
    case 3:
        ads114s08_set_input(ADS_AIN3, ADS_AINCOM, ch);
        break;
    case 4:
        ads114s08_set_input(ADS_AIN4, ADS_AINCOM, ch);
        break;
    case 5:
        ads114s08_set_input(ADS_AIN5, ADS_AINCOM, ch);
        break;
    case 6:
        ads114s08_set_input(ADS_AIN6, ADS_AINCOM, ch);
        break;
    case 7:
        ads114s08_set_input(ADS_AIN7, ADS_AINCOM, ch);
        break;
    case 8:
        ads114s08_set_input(ADS_AIN8, ADS_AINCOM, ch);
        break;
    case 9:
        ads114s08_set_input(ADS_AIN9, ADS_AINCOM, ch);
        break;
    case 10:
        ads114s08_set_input(ADS_AIN10, ADS_AINCOM, ch);
        break;
    case 11:
        ads114s08_set_input(ADS_AIN11, ADS_AINCOM, ch);
        break;
    default:
        break;
    }
#if 0
    ads114s08_inpmux_t inpmux;

    registers[DEVICE_1][REG_ADDR_INPMUX] = ads114s08_get_reg(DEVICE_1, REG_ADDR_INPMUX);

    inpmux.value = registers[DEVICE_1][REG_ADDR_INPMUX];
    snprintf(msg_buffer, sizeof msg_buffer, "\t\tPositive ADC input selection : %s\r\n", gp_input_multiplexer_selection_string[inpmux.u.muxp]);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof msg_buffer, "\t\tNegative ADC input selection : %s\r\n", gp_input_multiplexer_selection_string[inpmux.u.muxn]);
    print(msg_buffer);
#endif
}

/*
 * Sends a command to the ADS114S08
 *
 * \param op_code is the command being issued
 *
 */
void ads114s08_set_command(uint8_t op_code, adc_ch_t ch)
{
    ads114s08_cs_low(ch);
    xferWord(&hspi1, op_code);
    ads114s08_cs_high(ch);
}
/*
 * Sends a STOP/START command sequence to the ADS124S08 to restart conversions (SYNC)
 *
 */
void ads114s08_set_restart(adc_ch_t ch)
{
    ads114s08_set_command(CMD_STOP, ch);
    ads114s08_set_command(CMD_START, ch);
}
/*
 * Sets the GPIO hardware RESET pin pulse
 *
 */
void ads114s08_set_reset(adc_ch_t ch)
{
#ifdef USE_GPIO_RESET
    HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_RESET);

    //us_delay(10);
    HAL_Delay(5);

    /* low for a minimum of 4 · tCLK· cycles */
    HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_SET);
#else
    ads114s08_set_command(CMD_RESET, ch);
#endif
}

/*
 * Sets the GPIO hardware START pin high or low
 * Conversions start at the rising edge of the START/SYNC pin or on the seventh SCLK falling edge for a START command.
 */
void ads114s08_set_start(uint8_t b_set, adc_ch_t ch )
{
#ifdef USE_GPIO_START
    if(b_set)
    {
    	if(ch == ADC1_CS)
        	HAL_GPIO_WritePin(ADC1_START_GPIO_Port, ADC1_START_Pin, GPIO_PIN_SET);

		if(ch == ADC2_CS)
			HAL_GPIO_WritePin(ADC2_START_GPIO_Port, ADC2_START_Pin, GPIO_PIN_SET);
    }
    else
    {
    	if(ch == ADC1_CS)
        	HAL_GPIO_WritePin(ADC1_START_GPIO_Port, ADC1_START_Pin, GPIO_PIN_RESET);

		if(ch == ADC2_CS)
			HAL_GPIO_WritePin(ADC2_START_GPIO_Port, ADC2_START_Pin, GPIO_PIN_RESET);
    }
#else
    if(b_set)
    {
        ads114s08_set_command(CMD_START, ch);
    }
    else
    {
        ads114s08_set_command(CMD_STOP, ch);
    }
#endif
}

/*
 *
 * Read the last conversion result
 *
 */
int32_t ads114s08_get_data(uint8_t *dStatus, uint8_t *dCRC, adc_ch_t ch)
{
    int32_t iData = 0;

    ads114s08_cs_low(ch);
    if((registers[REG_ADDR_SYS] & 0x01) == DATA_MODE_STATUS)
    {
        if(dStatus != NULL)
        {
            dStatus[0] = xferWord(&hspi1, 0x00);
        }
    }

    // get the conversion data
    iData = xferWord(&hspi1, 0x00);
    iData <<= 8;
    iData += xferWord(&hspi1, 0x00);
#ifdef ADS124S08
    iData <<= 8;
    iData += xferWord(&hspi1, 0x00);
#endif
    if((registers[REG_ADDR_SYS] & 0x02) == DATA_MODE_CRC)
    {
        if(dCRC != NULL)
        {
            dCRC[0] = xferWord(&hspi1, 0x00);
        }
    }
    ads114s08_cs_high(ch);
    return iData;
}

int32_t ads114s08_get_rdata(uint8_t *dStatus, uint8_t *dCRC, adc_ch_t ch)
{
    int32_t iData = 0;

    ads114s08_cs_low(ch);
    xferWord(&hspi1, CMD_RDATA);

    if((registers[REG_ADDR_SYS] & 0x01) == DATA_MODE_STATUS)
    {
        if(dStatus != NULL)
        {
            dStatus[0] = xferWord(&hspi1, 0x00);
        }
    }

    // get the conversion data
    iData = xferWord(&hspi1, 0x00);
    iData <<= 8;
    iData += xferWord(&hspi1, 0x00);
#ifdef ADS124S08
    iData <<= 8;
    iData += xferWord(&hspi1, 0x00);
#endif
    if((registers[REG_ADDR_SYS] & 0x02) == DATA_MODE_CRC)
    {
        if(dCRC != NULL)
        {
            dCRC[0] = xferWord(&hspi1, 0x00);
        }
    }
    ads114s08_cs_high(ch);
    return iData;
}

