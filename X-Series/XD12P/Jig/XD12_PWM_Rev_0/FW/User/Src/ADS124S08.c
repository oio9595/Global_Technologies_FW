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
#include "config.h"

//#define USE_DISPLAY_DEVICE_REGS

/* Global variables */
static SPI_TypeDef* gp_SPI;

static uint8_t registers[REG_ADDR_MAX] =
{
    0x00, 0x80, 0x01, 0x00, 0x14, 0x10, 0x00, 0xFF, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
};

#define ADS114S08_READ_OFS_COUNT    (128) /* must be power of 2 */
static uint8_t gn_ads114s08_init_flag;

/*
uint8_t gb_ads114s08_drdy_done;
uint64_t gn_ads114s08_adc_temp;
uint16_t gn_adc_read_count;
*/
void adc_drdy_int_handler(void)
{
    int32_t temp = 0;
    temp = ads114s08_get_data(NULL, NULL);

    if (temp < 0)
    {
        temp = 0;
    }
    if(temp > 32767)
    {
        temp = 32767;
    }

    if(gn_adc_read_count)
    {
        gn_ads114s08_adc_temp += temp;
        --gn_adc_read_count;
    }

    if(gn_adc_read_count == 0)
    {
        gb_ads114s08_drdy_done = 1;
        ads114s08_set_start(0);    /* stop continuous conversion */
    }
}

uint16_t get_adc_value(void)
{
    if (!gn_ads114s08_init_flag)
    {
        return (uint16_t)((float)gn_ads114s08_adc_temp / ADS114S08_READ_OFS_COUNT + 0.5f);
    }

    else
    {
        return (uint16_t)((float)gn_ads114s08_adc_temp / ADS114S08_READ_COUNT + 0.5f);
    }
}


#ifdef USE_DISPLAY_DEVICE_REGS
static char* gp_delay_string[] =
{
    "000 : 14 * tMOD (default)",
    "001 : 25 * tMOD",
    "010 : 64 * tMOD",
    "011 : 256 * tMOD",
    "100 : 1024 * tMOD",
    "101 : 2048 * tMOD",
    "110 : 4096 * tMOD",
    "111 : 1 * tMOD",
};

static char* gp_gain_string[] =
{
    "000 : 1 (default)",
    "001 : 2",
    "010 : 4",
    "011 : 8",
    "100 : 16",
    "101 : 32",
    "110 : 64",
    "111 : 128",
};

static char* gp_datarate_string[] =
{
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

static char* gp_ref_monitoring_config_string[] =
{
    "00 : Disabled (default)",
    "01 : FL_REF_L0 monitor enabled, threshold 0.3 V",
    "10 : FL_REF_L0 and FL_REF_L1 monitors enabled, thresholds 0.3 V and 1/3 · (AVDD – AVSS)",
    "11 : FL_REF_L0 monitor and 10-MΩ pull-together enabled, threshold 0.3 V",
};

static char* gp_ref_input_selection_string[] =
{
    "00 : REFP0, REFN0 (default)",
    "01 : REFP1, REFN1",
    "10 : Internal 2.5-V reference(1)",
    "11 : Reserved",
};

static char* gp_internal_voltage_ref_config_string[] =
{
    "00 : Internal reference off (default)",
    "01 : Internal reference on, but powers down in power-down mode",
    "10 : Internal reference is always on, even in power-down mode",
    "11 : Reserved",
};

static char* gp_idac_magnitude_selection_string[] =
{
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

static char* gp_idac_output_channel_selection_string[] =
{
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

static char* gp_input_multiplexer_selection_string[] =
{
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

static char* gp_sys_monitor_config_string[] =
{
    "000 : Disabled (default)",
    "001 : PGA inputs shorted to (AVDD + AVSS) / 2 and disconnected from AINx and the multiplexer; gain set by user",
    "010 : Internal temperature sensor measurement; PGA must be enabled (PGA_EN[1:0] = 01); gain set by user",
    "011 : (AVDD – AVSS) / 4 measurement; gain set to 1",
    "100 : DVDD / 4 measurement; gain set to 1",
    "101 : Burn-out current sources enabled, 0.2-μA setting",
    "110 : Burn-out current sources enabled, 1-μA setting",
    "111 : Burn-out current sources enabled, 10-μA setting",
};

static char* gp_calibration_sample_size_selection_string[] =
{
    "00 : 1 sample",
    "01 : 4 samples",
    "10 : 8 samples (default)",
    "11 : 16 samples",
};

static void dump_regs(void)
{
    print(LOG_DEBUG, "======== ADS114S08 regs value ========\r\n");
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

        print(LOG_DEBUG, "Device #1\r\n");

        id.value = registers[REG_ADDR_ID];
        print(LOG_DEBUG, "\tDevice identifier : 0x%X(%s)\r\n", id.u.dev_id, (id.u.dev_id == 0x04 ? "ADS114S08" : (id.u.dev_id == 0x05 ? "ADS114S06" : "UNKNOWN")));

        status.value = registers[REG_ADDR_STATUS];
        print(LOG_DEBUG, "\tDevice Status\r\n");
        print(LOG_DEBUG, "\t\tPOR flag : %s\r\n", (status.u.fl_por == 0x00 ? "0 : Register has been cleared and no POR event has occurred." : "1 : POR event occurred and has not been cleared. Flag must be cleared by user register write (default)."));

        print(LOG_DEBUG, "\t\tDevice ready flag : %s\r\n", (status.u.rdy == 0x00 ? "0 : ADC ready for communication (default)" : "1 : ADC not ready"));

        print(LOG_DEBUG, "\t\tPositive PGA output at positive rail flag : %s\r\n", (status.u.fl_p_railp == 0x00 ? "0 : No error (default)" : "1 : PGA positive output within 150 mV of AVDD"));

        print(LOG_DEBUG, "\t\tPositive PGA output at negative rail flag : %s\r\n", (status.u.fl_p_railn == 0x00 ? "0 : No error (default)" : "1 : PGA positive output within 150 mV of AVSS"));

        print(LOG_DEBUG, "\t\tNegative PGA output at positive rail flag : %s\r\n", (status.u.fl_n_railp == 0x00 ? "0 : No error (default)" : "1 : PGA negative output within 150 mV of AVDD"));

        print(LOG_DEBUG, "\t\tNegative PGA output at negative rail flag : %s\r\n", (status.u.fl_n_railn == 0x00 ? "0 : No error (default)" : "1 : PGA negative output within 150 mV of AVSS"));

        print(LOG_DEBUG, "\t\tReference voltage monitor flag, level 1 : %s\r\n", (status.u.fl_ref_l1 == 0x00 ? "0 : Differential reference voltage ≥ 1/3 · (AVDD - AVSS) (default)" : "1 : Differential reference voltage < 1/3 · (AVDD - AVSS)"));

        print(LOG_DEBUG, "\t\tReference voltage monitor flag, level 0 : %s\r\n", (status.u.fl_ref_l0 == 0x00 ? "0 : Differential reference voltage ≥ 0.3 V (default)" : "1 : Differential reference voltage < 0.3 V"));

        inpmux.value = registers[REG_ADDR_INPMUX];
        print(LOG_DEBUG, "\tInput Multiplexer\r\n");
        print(LOG_DEBUG, "\t\tPositive ADC input selection : %s\r\n", gp_input_multiplexer_selection_string[inpmux.u.muxp]);

        print(LOG_DEBUG, "\t\tNegative ADC input selection : %s\r\n", gp_input_multiplexer_selection_string[inpmux.u.muxn]);

        gain.value = registers[REG_ADDR_PGA];
        print(LOG_DEBUG, "\tGain Setting\r\n");
        print(LOG_DEBUG, "\t\tProgrammable conversion delay selection : %s\r\n", gp_delay_string[gain.u.delay]);

        print(LOG_DEBUG, "\t\tPGA enable : %s\r\n", (gain.u.pga_en == 0x00 ? "00 : PGA is powered down and bypassed. Enables single-ended measurements with unipolar supply (Set gain = 1(2)) (default)" : (gain.u.pga_en == 0x01 ? "01 : PGA enabled (gain = 1 to 128)" : "Reserved")));

        print(LOG_DEBUG, "\t\tPGA gain selection : %s\r\n", gp_gain_string[gain.u.gain]);

        datarate.value = registers[REG_ADDR_DATARATE];
        print(LOG_DEBUG, "\tData Rate\r\n");
        print(LOG_DEBUG, "\t\tGlobal chop enable : %s\r\n", (datarate.u.g_chop == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));

        print(LOG_DEBUG, "\t\tClock source selection : %s\r\n", (datarate.u.clk == 0x00 ? "0 : Internal 4.096-MHz oscillator (default)" : "1 : External clock"));

        print(LOG_DEBUG, "\t\tConversion mode selection : %s\r\n", (datarate.u.mode == 0x00 ? "0 : Continuous conversion mode (default)" : "1 : Single-shot conversion mode"));

        print(LOG_DEBUG, "\t\tDigital filter selection : %s\r\n", (datarate.u.filter == 0x00 ? "0 : Sinc3 filter" : "1 : Low-latency filter (default)"));

        print(LOG_DEBUG, "\t\tData rate selection : %s\r\n", gp_datarate_string[datarate.u.dr]);

        ref_ctrl.value = registers[REG_ADDR_REF];
        print(LOG_DEBUG, "\tReference Control (REF)\r\n");
        print(LOG_DEBUG, "\t\tReference monitor configuration : %s\r\n", gp_ref_monitoring_config_string[ref_ctrl.u.fl_ref_en]);

        print(LOG_DEBUG, "\t\tPositive reference buffer bypass : %s\r\n", (ref_ctrl.u.refp_buf == 0x00 ? "0 : Enabled (default)" : "1 : Disabled"));

        print(LOG_DEBUG, "\t\tNegative reference buffer bypass : %s\r\n", (ref_ctrl.u.refn_buf == 0x00 ? "0 : Enabled" : "1 : Disabled (default)"));

        print(LOG_DEBUG, "\t\tReference input selection : %s\r\n", gp_ref_input_selection_string[ref_ctrl.u.refsel]);

        print(LOG_DEBUG, "\t\tInternal voltage reference configuration : %s\r\n", gp_internal_voltage_ref_config_string[ref_ctrl.u.refsel]);

        idacmag.value = registers[REG_ADDR_IDACMAG];
        print(LOG_DEBUG, "\tExcitation Current #1\r\n");
        print(LOG_DEBUG, "\t\tPGA output rail flag enable : %s\r\n", (idacmag.u.fl_rail_en == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));

        print(LOG_DEBUG, "\t\tLow-side power switch : %s\r\n", (idacmag.u.psw == 0x00 ? "0 : Open (default)" : "1 : Closed"));

        print(LOG_DEBUG, "\t\tIDAC magnitude selection : %s\r\n", gp_idac_magnitude_selection_string[idacmag.u.imag]);

        idacmux.value = registers[REG_ADDR_IDACMUX];
        print(LOG_DEBUG, "\tExcitation Current #2\r\n");
        print(LOG_DEBUG, "\t\tIDAC2 output channel selection : %s\r\n", gp_idac_output_channel_selection_string[idacmux.u.i2mux]);

        print(LOG_DEBUG, "\t\tIDAC1 output channel selection : %s\r\n", gp_idac_output_channel_selection_string[idacmux.u.i1mux]);

        vbias.value = registers[REG_ADDR_VBIAS];
        print(LOG_DEBUG, "\tSensor Biasing\r\n");
        print(LOG_DEBUG, "\t\tVBIAS level selection : %s\r\n", (vbias.u.vb_level == 0x00 ? "0 : (AVDD + AVSS) / 2 (default)" : "1 : (AVDD + AVSS) / 12"));

        print(LOG_DEBUG, "\t\tAINCOM VBIAS selection : %s\r\n", (vbias.u.vb_ainc == 0x00 ? "0 : VBIAS disconnected from AINCOM (default)" : "1 : VBIAS connected to AINCOM"));

        print(LOG_DEBUG, "\t\tAIN5 VBIAS selection : %s\r\n", (vbias.u.vb_ain5 == 0x00 ? "0 : VBIAS disconnected from AIN5 (default)" : "1 : VBIAS connected to AIN5"));

        print(LOG_DEBUG, "\t\tAIN4 VBIAS selection : %s\r\n", (vbias.u.vb_ain4 == 0x00 ? "0 : VBIAS disconnected from AIN4 (default)" : "1 : VBIAS connected to AIN4"));

        print(LOG_DEBUG, "\t\tAIN3 VBIAS selection : %s\r\n", (vbias.u.vb_ain3 == 0x00 ? "0 : VBIAS disconnected from AIN3 (default)" : "1 : VBIAS connected to AIN3"));

        print(LOG_DEBUG, "\t\tAIN2 VBIAS selection : %s\r\n", (vbias.u.vb_ain2 == 0x00 ? "0 : VBIAS disconnected from AIN2 (default)" : "1 : VBIAS connected to AIN2"));

        print(LOG_DEBUG, "\t\tAIN1 VBIAS selection : %s\r\n", (vbias.u.vb_ain1 == 0x00 ? "0 : VBIAS disconnected from AIN1 (default)" : "1 : VBIAS connected to AIN1"));

        print(LOG_DEBUG, "\t\tAIN0 VBIAS selection : %s\r\n", (vbias.u.vb_ain0 == 0x00 ? "0 : VBIAS disconnected from AIN0 (default)" : "1 : VBIAS connected to AIN0"));


        sys.value = registers[REG_ADDR_SYS];
        print(LOG_DEBUG, "\tSystem Control\r\n");
        print(LOG_DEBUG, "\t\tSystem monitor configuration : %s\r\n", gp_sys_monitor_config_string[sys.u.sys_mon]);

        print(LOG_DEBUG, "\t\tCalibration sample size selection : %s\r\n", gp_calibration_sample_size_selection_string[sys.u.cal_samp]);

        print(LOG_DEBUG, "\t\tSPI timeout enable : %s\r\n", (sys.u.timeout == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));

        print(LOG_DEBUG, "\t\tCRC enable : %s\r\n", (sys.u.timeout == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));

        print(LOG_DEBUG, "\t\tSTATUS byte enable : %s\r\n", (sys.u.timeout == 0x00 ? "0 : Disabled (default)" : "1 : Enabled"));

        ofcal0.value = registers[REG_ADDR_OFCAL0];
        print(LOG_DEBUG, "\tOffset Calibration #1 : %u\r\n", ofcal0.value);

        ofcal1.value = registers[REG_ADDR_OFCAL1];
        print(LOG_DEBUG, "\tOffset Calibration #2 : %u\r\n", ofcal1.value);

        fscal0.value = registers[REG_ADDR_FSCAL0];
        print(LOG_DEBUG, "\tGain Calibration #1 : %u\r\n", fscal0.value);

        fscal1.value = registers[REG_ADDR_FSCAL1];
        print(LOG_DEBUG, "\tGain Calibration #2 : %u\r\n", fscal1.value);

        gpiodat.value = registers[REG_ADDR_GPIODAT];
        print(LOG_DEBUG, "\tGPIO Data\r\n");
        print(LOG_DEBUG, "\t\tGPIO direction : 0x%X\r\n", gpiodat.u.dir);

        print(LOG_DEBUG, "\t\tGPIO data : 0x%X\r\n", gpiodat.u.dat);

        gpiocon.value = registers[REG_ADDR_GPIOCON];
        print(LOG_DEBUG, "\tGPIO Configuration\r\n");
        print(LOG_DEBUG, "\t\tGPIO pin configuration : 0x%X\r\n", gpiocon.u.con);
    }
    print(LOG_DEBUG, "======================================\r\n");
}
#endif

static void ads114s08_set_reset(void);
static uint8_t ads114s08_get_reg(unsigned int regnum);
static void ads114s08_get_regs(uint8_t regnum, uint8_t count, uint8_t *data);
static void ads114s08_set_reg(uint8_t regnum, uint8_t data);
static void  ads114s08_set_regs(uint8_t regnum, uint8_t howmuch, uint8_t *data);
static void ads114s08_set_input(uint8_t input_p, uint8_t input_n);
static void ads114s08_set_command(uint8_t op_code);
static void ads114s08_get_adc_offset();


#if 1
/*
 * The xferWord is controller specific and will
 * need to adapt to the particular micro in use
 * \param tx id data to be transmitted
 *
 */
 __STATIC_INLINE uint8_t xferWord(SPI_TypeDef *SPIx, uint8_t tx)
{
    uint8_t rx = 0;

    if(LL_SPI_IsEnabled(SPIx) != 1)
    {
      /* Enable SPI peripheral */
        LL_SPI_Enable(SPIx);
    }

    /* Set up data for the next xmit */
    while(!LL_SPI_IsActiveFlag_TXE(SPIx)) {};
    LL_SPI_TransmitData8(SPIx, tx);
    /* Wait for data to appear */
    while(!LL_SPI_IsActiveFlag_RXNE(SPIx)) {};
    /* Grab that data*/
    rx = LL_SPI_ReceiveData8(SPIx);

    return rx;
}
#endif


/*
 * Writes the nCS pin low and waits a while for the Tiva to finish working before
 * handing control back to the caller for a SPI transfer.
 */
static void ads114s08_cs_low(void)
{
    LL_GPIO_ResetOutputPin(ADC_CS_GPIO_Port, ADC_CS_Pin);
}

/*
 * Pulls the nCS pin high. Performs no waiting.
 */
static void ads114s08_cs_high(void)
{
    LL_GPIO_SetOutputPin(ADC_CS_GPIO_Port, ADC_CS_Pin);
}
/*
 * Initializes device for use in the ADS124S08 EVM.
 *
 * \return True if device is in correct hardware defaults and is connected
 *
 */
void ads114s08_init(void)
{
    gp_SPI = SPI2;

    ads114s08_set_reset();
    us_tdelay(185); /* Delay time : 182us[4096tCLK(tCLK = 1 / fCLK(22.5MHz)], first SCLK rising edge after RESET rising edge (or 7th SCLK falling edge of RESET command) */

    //for(uint8_t dev=DEVICE_1 ; dev < ADS114S08_DEV_NUM ; ++dev)
    {
        for(uint8_t reg=REG_ADDR_ID ; reg < REG_ADDR_MAX ; ++reg)
        {
            if(reg == REG_ADDR_RESERVED1 || reg == REG_ADDR_RESERVED2)
            {
            }
            else
            {
                registers[reg] = ads114s08_get_reg(reg);

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
                        ads114s08_set_reg(reg, status.value);
                        registers[reg] = ads114s08_get_reg(reg);
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

                    if(datarate.u.dr != 12)  /* DataRate : 400 SPS, Low-Latency filter : 1, ? conversion mode */
                    {
                        datarate.u.dr = 12;
                        datarate.u.filter = 0;
                        //datarate.u.mode = 1;

                        ads114s08_set_reg(reg, datarate.value);
                        registers[reg] = ads114s08_get_reg(reg);
                    }
#endif
                }
            }
        }

        //ads114s08_set_command(dev, CMD_WAKEUP);
    }

    ads114s08_get_adc_offset();
    gn_ads114s08_init_flag = 1;
#ifdef USE_DISPLAY_DEVICE_REGS
    dump_regs();
#endif
}
/*
 * Reads a single register contents from the specified address
 *
 * \param regnum identifies which address to read
 *
 */
static uint8_t ads114s08_get_reg(unsigned int regnum)
{
    uint8_t i = 0;
    uint8_t ulDataTx[3] = {0, };
    uint8_t ulDataRx[3] = {0, };

    ulDataTx[0] = CMD_REGRD + (regnum & 0x1f);
    ulDataTx[1] = 0x00;
    ulDataTx[2] = 0x00;

    ads114s08_cs_low();

    for(i = 0; i < 3; i++)
    {
        ulDataRx[i] = xferWord(gp_SPI, ulDataTx[i]);
    }
    if(regnum < REG_ADDR_MAX)
    {
        registers[regnum] = ulDataRx[2];
    }
    ads114s08_cs_high();

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
static void ads114s08_get_regs(uint8_t regnum, uint8_t count, uint8_t *data)
{
    uint8_t i = 0;
    uint8_t ulDataTx[2] = {0, };

    ulDataTx[0] = CMD_REGRD + (regnum & 0x1f);
    ulDataTx[1] = count-1;

    ads114s08_cs_low();

    xferWord(gp_SPI, ulDataTx[0]);
    xferWord(gp_SPI, ulDataTx[1]);
    for(i = 0; i < count; i++)
    {
        data[i] = xferWord(gp_SPI, 0);
        if(regnum+i < REG_ADDR_MAX)
        {
            registers[regnum+i] = data[i];
        }
    }
    ads114s08_cs_high();
}
/*
 * Writes a single of register with the specified data
 *
 * \param regnum addr_mask 8-bit mask of the register to which we start writing
 * \param data to be written
 *
 */
static void ads114s08_set_reg(uint8_t regnum, uint8_t data)
{
    uint8_t ulDataTx[3];

    ulDataTx[0] = CMD_REGWR + (regnum & 0x1f);
    ulDataTx[1] = 0x00; /* the number of registers to write (minus 1): 000n nnnn */
    ulDataTx[2] = data;

    ads114s08_cs_low();
    xferWord(gp_SPI, ulDataTx[0]);
    xferWord(gp_SPI, ulDataTx[1]);
    xferWord(gp_SPI, ulDataTx[2]);
    ads114s08_cs_high();
}
/*
 * Writes a group of registers starting at the specified address
 *
 * \param regnum is addr_mask 8-bit mask of the register from which we start writing
 * \param count The number of registers we wish to write
 * \param *location pointer to the location in memory to read the data
 *
 */
static void  ads114s08_set_regs(uint8_t regnum, uint8_t howmuch, uint8_t *data)
{
    uint8_t i = 0;
    uint8_t ulDataTx[2];

    ulDataTx[0] = CMD_REGWR + (regnum & 0x1f);
    ulDataTx[1] = howmuch-1;

    ads114s08_cs_low();
    xferWord(gp_SPI, ulDataTx[0]);
    xferWord(gp_SPI, ulDataTx[1]);
    for(i=0; i < howmuch; i++)
    {
        xferWord(gp_SPI, data[i]);
        if(regnum+i < REG_ADDR_MAX)
        {
            registers[regnum+i] = data[i];
        }
    }
    ads114s08_cs_high();
}

static void ads114s08_set_input(uint8_t input_p, uint8_t input_n)
{
    ads114s08_inpmux_t t = {0,};

    t.u.muxp = input_p;
    t.u.muxn = input_n;

    ads114s08_set_reg(REG_ADDR_INPMUX, t.value);
}

void ads114s08_select_single_ended_input(uint8_t input)
{
#if 0
    print(LOG_DEBUG, "\t input [%5d]\r\n,", input);
#endif
    switch(input)
    {
        case 0:
            ads114s08_set_input(ADS_AIN0, ADS_AINCOM);
        break;
        case 1:
            ads114s08_set_input(ADS_AIN1, ADS_AINCOM);
        break;
        case 2:
            ads114s08_set_input(ADS_AIN2, ADS_AINCOM);
        break;
        case 3:
            ads114s08_set_input(ADS_AIN3, ADS_AINCOM);
        break;
        case 4:
            ads114s08_set_input(ADS_AIN4, ADS_AINCOM);
        break;
        case 5:
            ads114s08_set_input(ADS_AIN5, ADS_AINCOM);
        break;
        case 6:
            ads114s08_set_input(ADS_AIN6, ADS_AINCOM);
        break;
        case 7:
            ads114s08_set_input(ADS_AIN7, ADS_AINCOM);
        break;
        case 8:
            ads114s08_set_input(ADS_AIN8, ADS_AINCOM);
        break;
        case 9:
            ads114s08_set_input(ADS_AIN9, ADS_AINCOM);
        break;
        case 10:
            ads114s08_set_input(ADS_AIN10, ADS_AINCOM);
        break;
        case 11:
            ads114s08_set_input(ADS_AIN11, ADS_AINCOM);
        break;
        default:
        break;
    }
#if 0
    ads114s08_inpmux_t inpmux;

    registers[DEVICE_1][REG_ADDR_INPMUX] = ads114s08_get_reg(DEVICE_1, REG_ADDR_INPMUX);

    inpmux.value = registers[DEVICE_1][REG_ADDR_INPMUX];
    print(LOG_DEBUG, "\t\tPositive ADC input selection : %s\r\n", gp_input_multiplexer_selection_string[inpmux.u.muxp]);

    print(LOG_DEBUG, "\t\tNegative ADC input selection : %s\r\n", gp_input_multiplexer_selection_string[inpmux.u.muxn]);

#endif
}

/*
 * Sends a command to the ADS114S08
 *
 * \param op_code is the command being issued
 *
 */
static void ads114s08_set_command(uint8_t op_code)
{
    ads114s08_cs_low();
    xferWord(gp_SPI, op_code);
    ads114s08_cs_high();
}
/*
 * Sends a STOP/START command sequence to the ADS124S08 to restart conversions (SYNC)
 *
 */
void ads114s08_set_restart(void)
{
    ads114s08_set_command(CMD_STOP);
    ads114s08_set_command(CMD_START);
}
/*
 * Sets the GPIO hardware RESET pin pulse
 *
 */
static void ads114s08_set_reset(void)
{
#ifdef USE_GPIO_RESET
    LL_GPIO_ResetOutputPin(ADC_RESET_GPIO_Port, ADC_RESET_Pin);

    us_tdelay(1);

    /* low for a minimum of 4 · tCLK· cycles */
    LL_GPIO_SetOutputPin(ADC_RESET_GPIO_Port, ADC_RESET_Pin);
#else
    ads114s08_set_command(CMD_RESET);
#endif
}

/*
 * Sets the GPIO hardware START pin high or low
 * Conversions start at the rising edge of the START/SYNC pin or on the seventh SCLK falling edge for a START command.
 */
void ads114s08_set_start(uint8_t b_set)
{
#ifdef USE_GPIO_START
    if(b_set)
    {
        LL_GPIO_SetOutputPin(ADC_START_GPIO_Port, ADC_START_Pin);
    }
    else
    {
        LL_GPIO_ResetOutputPin(ADC_START_GPIO_Port, ADC_START_Pin);
    }
#else
    if(b_set)
    {
        ads114s08_set_command(CMD_START);
    }
    else
    {
        ads114s08_set_command(CMD_STOP);
    }
#endif
}

/*
 *
 * Read the last conversion result
 *
 */
int32_t ads114s08_get_data(uint8_t *dStatus, uint8_t *dCRC)
{
    int32_t iData = 0;

    ads114s08_cs_low();
    if((registers[REG_ADDR_SYS] & 0x01) == DATA_MODE_STATUS)
    {
        if(dStatus != NULL)
        {
            dStatus[0] = xferWord(gp_SPI, 0x00);
        }
    }

    // get the conversion data
    iData = xferWord(gp_SPI, 0x00);
    iData <<= 8;
    iData += xferWord(gp_SPI, 0x00);
#ifdef ADS124S08
    iData <<= 8;
    iData += xferWord(gp_SPI, 0x00);
#endif
    if((registers[REG_ADDR_SYS] & 0x02) == DATA_MODE_CRC)
    {
        if(dCRC != NULL)
        {
            dCRC[0] = xferWord(gp_SPI, 0x00);
        }
    }
    ads114s08_cs_high();
    return iData;
}

int32_t ads114s08_get_rdata(uint8_t *dStatus, uint8_t *dCRC)
{
    int32_t iData = 0;

    ads114s08_cs_low();
    xferWord(gp_SPI, CMD_RDATA);

    if((registers[REG_ADDR_SYS] & 0x01) == DATA_MODE_STATUS)
    {
        if(dStatus != NULL)
        {
            dStatus[0] = xferWord(gp_SPI, 0x00);
        }
    }

    // get the conversion data
    iData = xferWord(gp_SPI, 0x00);
    iData <<= 8;
    iData += xferWord(gp_SPI, 0x00);
#ifdef ADS124S08
    iData <<= 8;
    iData += xferWord(gp_SPI, 0x00);
#endif
    if((registers[REG_ADDR_SYS] & 0x02) == DATA_MODE_CRC)
    {
        if(dCRC != NULL)
        {
            dCRC[0] = xferWord(gp_SPI, 0x00);
        }
    }
    ads114s08_cs_high();
    return iData;
}

#pragma optimize=none
static void ads114s08_get_adc_offset()
{
    print(LOG_INFO, "\r\n ...Get ADC Offset Start...\r\n");
    JigBd_IF_change_current_gain(GAIN_LOW);
    JigBd_IF_VLED_9V_EN(PWR_ON);

    for (uint8_t ch = 0 ; ch < CH_MAX ; ++ch)
    {
        gb_ads114s08_drdy_done = 0;
        gn_ads114s08_adc_temp = 0;
        gn_adc_read_count = ADS114S08_READ_OFS_COUNT;

        JigBd_IF_select_output_ch(ch);
        ads114s08_select_single_ended_input(0);
        LL_mDelay(10);

        ads114s08_set_start(1);
        LL_mDelay(1);

        while(1)
        {
            // print(LOG_DEBUG, "\r\n drdy_done - [%u]\r\n", gb_ads114s08_drdy_done);
            if (gb_ads114s08_drdy_done)
            {
                break;
            }
        }
        gn_ads114s08_offset[ch] = get_adc_value();
    }

    gb_ads114s08_drdy_done = 0;
    gn_ads114s08_adc_temp = 0;
    gn_adc_read_count = ADS114S08_READ_COUNT;

    JigBd_IF_VLED_9V_EN(PWR_OFF);
    JigBd_IF_select_output_ch(CH_MAX);
    print(LOG_INFO, "\r\n ...Get ADC Offset Done...\r\n");
}

double convert_adc_to_current(uint16_t adc, current_gain_t gain)
{
	double ret = 0;
	switch (gain)
	{
		case GAIN_HIGH :
			ret = ((double)adc * ADC_CONV_COEFF_HIGH);	/* max 140mA */
			break;

		case GAIN_MID :
			ret = ((double)adc * ADC_CONV_COEFF_MID);	/* max  10mA */
			break;

		case GAIN_LOW :
			ret = ((double)adc * ADC_CONV_COEFF_LOW);	/* max 0.5mA */
			break;
	}
	return ret; //mA
}

uint16_t convert_current_to_adc(double current_A, current_gain_t gain)
{
	uint16_t ret = 0;
	switch (gain)
	{
		case GAIN_HIGH :
            ret = (uint16_t)(current_A * 1000 / ADC_CONV_COEFF_HIGH + 0.5f); /* mA -> ADC */
			break;

		case GAIN_MID :
			ret = (uint16_t)(current_A * 1000 / ADC_CONV_COEFF_MID + 0.5f); /* mA -> ADC */
			break;

		case GAIN_LOW :
			ret = (uint16_t)(current_A * 1000 / ADC_CONV_COEFF_LOW + 0.5f); /* mA -> ADC */
			break;
	}
	return ret; //adc
}
