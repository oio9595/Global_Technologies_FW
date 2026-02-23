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

#include "main.h"
#include "ads124s08.h"
#include "xdic.h"

#define USE_DISPLAY_DEVICE_REGS
#define ADS114S08_READ_OFS_COUNT    (128) /* must be power of 2 */

typedef enum tag_ADC_SPS_T
{
    ADS_SPS_2_5 = 0,
    ADS_SPS_5,
    ADS_SPS_10,
    ADS_SPS_16_6,
    ADS_SPS_20,
    ADS_SPS_50,
    ADS_SPS_60,
    ADS_SPS_100,
    ADS_SPS_200,
    ADS_SPS_400,
    ADS_SPS_800,
    ADS_SPS_1000,
    ADS_SPS_2000,
    ADS_SPS_4000,
    ADS_SPS_RSVD,
}ads_sps_t;

/* Global variables */
static SPI_TypeDef* gp_SPI = SPI2;
static ads114s08_regs_t gt_ads114s08_regs;

static uint8_t gn_ads114s08_init_flag;
static volatile bool gb_ads114s08_drdy_done;

static uint64_t gn_ads114s08_adc_temp;
static uint16_t gn_adc_read_count;
static uint16_t gn_ads114s08_offset[12];

volatile uint16_t gn_ads114s08_read_timeout;

#ifdef USE_DISPLAY_DEVICE_REGS
static void ADS114S08_Dump_Registers(void)
{
    print(LOG_DEBUG, "======== ADS114S08 regs value ========\r\n");

    print(LOG_DEBUG, "ID : 0x%X\r\n", gt_ads114s08_regs.id.value);
    print(LOG_DEBUG, "\tDEV ID : %s\r\n", (gt_ads114s08_regs.id.u.dev_id == 0x04 ? "ADS114S08" : (gt_ads114s08_regs.id.u.dev_id == 0x05 ? "ADS114S06" : "UNKNOWN")));

    print(LOG_DEBUG, "Device Status : 0x%X\r\n", gt_ads114s08_regs.status.value);
    print(LOG_DEBUG, "\tPOR : %u\r\n", gt_ads114s08_regs.status.u.fl_por);
    print(LOG_DEBUG, "\tRDY : %u\r\n", gt_ads114s08_regs.status.u.rdy);

    print(LOG_DEBUG, "Data Rate : 0x%X\r\n", gt_ads114s08_regs.datarate.value);
    print(LOG_DEBUG, "\tDR : %u\r\n", gt_ads114s08_regs.datarate.u.dr);

    print(LOG_DEBUG, "SYS : 0x%X\r\n", gt_ads114s08_regs.sys.value);
    print(LOG_DEBUG, "\tSENDSTAT : %u\r\n", gt_ads114s08_regs.sys.u.sendstat);
    print(LOG_DEBUG, "\tCRC : %u\r\n", gt_ads114s08_regs.sys.u.crc);

    print(LOG_DEBUG, "======================================\r\n");
}
#endif

 __STATIC_INLINE uint8_t xferWord(SPI_TypeDef *SPIx, uint8_t tx)
{
    uint8_t rx = 0;

    if (LL_SPI_IsEnabled(SPIx) != 1)
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

static uint8_t ADS114S08_Read_Register(uint8_t reg_addr)
{
    uint8_t TxBuffer[3] = {0, };
    uint8_t RxBuffer[3] = {0, };

    TxBuffer[0] = (CMD_RREG | (reg_addr & 0x1F));

    ADS_CS_LO();

    for(uint8_t i = 0 ; i < 3 ; ++i)
    {
        RxBuffer[i] = xferWord(gp_SPI, TxBuffer[i]);
    }

    ADS_CS_HI();

    return RxBuffer[2];
}

static int32_t ADS114S08_Get_RData(void)
{
    uint8_t TxBuffer[3] = {0, };
    uint8_t RxBuffer[3] = {0, };
    int32_t iData = 0;

    TxBuffer[0] = CMD_RDATA;

    ADS_CS_LO();

    for (uint8_t i = 0 ; i < 3 ; ++i)
    {
        RxBuffer[i] = xferWord(gp_SPI, TxBuffer[i]);
    }

    iData = (RxBuffer[1] << 8 | RxBuffer[2]);

    ADS_CS_HI();
    return iData;
}

static void ADS114S08_Write_Register(uint8_t reg_addr, uint8_t reg_data)
{
    uint8_t TxBuffer[3];

    TxBuffer[0] = (CMD_WREG | (reg_addr & 0x1F));
    TxBuffer[1] = 0x00;
    TxBuffer[2] = reg_data;

    ADS_CS_LO();

    for(uint8_t i = 0 ; i < 3 ; ++i)
    {
        xferWord(gp_SPI, TxBuffer[i]);
    }

    ADS_CS_HI();
}

static void ADS114S08_Set_Input(uint8_t input_p, uint8_t input_n)
{
    ads114s08_inpmux_t t = {0,};

    t.u.muxp = input_p;
    t.u.muxn = input_n;

    ADS114S08_Write_Register(REG_ADDR_INPMUX, t.value);
}

void ADS114S08_Select_Input_CH(uint8_t input)
{
    switch(input)
    {
    case 0:
        ADS114S08_Set_Input(ADS_AIN0, ADS_AINCOM);
    break;
    case 1:
        ADS114S08_Set_Input(ADS_AIN1, ADS_AINCOM);
    break;
    case 2:
        ADS114S08_Set_Input(ADS_AIN2, ADS_AINCOM);
    break;
    case 3:
        ADS114S08_Set_Input(ADS_AIN3, ADS_AINCOM);
    break;
    case 4:
        ADS114S08_Set_Input(ADS_AIN4, ADS_AINCOM);
    break;
    case 5:
        ADS114S08_Set_Input(ADS_AIN5, ADS_AINCOM);
    break;
    case 6:
        ADS114S08_Set_Input(ADS_AIN6, ADS_AINCOM);
    break;
    case 7:
        ADS114S08_Set_Input(ADS_AIN7, ADS_AINCOM);
    break;
    case 8:
        ADS114S08_Set_Input(ADS_AIN8, ADS_AINCOM);
    break;
    case 9:
        ADS114S08_Set_Input(ADS_AIN9, ADS_AINCOM);
    break;
    case 10:
        ADS114S08_Set_Input(ADS_AIN10, ADS_AINCOM);
    break;
    case 11:
        ADS114S08_Set_Input(ADS_AIN11, ADS_AINCOM);
    break;
    default:
    break;
    }
}

static void ADS114S08_Set_CMD(uint8_t cmd_code)
{
    ADS_CS_LO();
    xferWord(gp_SPI, cmd_code);
    ADS_CS_HI();
}

static void ADS114S08_Reset(void)
{
    ADS114S08_Set_CMD(CMD_RESET);
}

void ADS114S08_Set_Start(uint8_t b_set)
{
    if (b_set)
    {
        gb_ads114s08_drdy_done = 0;
        gn_ads114s08_adc_temp = 0;
        gn_adc_read_count = ADS114S08_READ_COUNT;
        ADS114S08_Set_CMD(CMD_START);
    }
    else
    {
        ADS114S08_Set_CMD(CMD_STOP);
    }
}

void ADS114S08_Wait_Done(void)
{
    gn_ads114s08_read_timeout = 15; // 2000SPS * 16 EA = 8ms
    while(1)
    {
        if (gb_ads114s08_drdy_done)
        {
            break;
        }
        if (gn_ads114s08_read_timeout == 0)
        {
            print(LOG_ERROR, "%s timeout\r\n", __func__);
            break;
        }
    }
}

#pragma optimize=none
static void ADS114S08_Get_ADC_Offset()
{
    print(LOG_DEBUG, "\r\n ...Get ADC Offset Start...\r\n");
    JigBD_IF_Change_Current_Gain(GAIN_LOW);
    JigBD_IF_VLED_9V_EN(PWR_ON);

    for (uint8_t ch = 0 ; ch < XD_CH_MAX ; ++ch)
    {
        JigBD_IF_Select_Output_Ch(ch);
        ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
        LL_mDelay(10);

        ADS114S08_Set_Start(1);
        ADS114S08_Wait_Done();
        gn_ads114s08_offset[ch] = ADS114S08_Get_ADC_Value();
    }
    JigBD_IF_VLED_9V_EN(PWR_OFF);
    JigBD_IF_Select_Output_Ch(XD_CH_MAX);
    print(LOG_DEBUG, "\r\n ...Get ADC Offset Done...\r\n");
}

void ADS114S08_Init(void)
{
    gp_SPI = SPI2;

    ADS114S08_Reset();
    LL_mDelay(1);

    for (uint8_t reg = REG_ADDR_ID ; reg < REG_ADDR_MAX ; ++reg)
    {
        *(&gt_ads114s08_regs.id.value + reg) = ADS114S08_Read_Register(reg);
        print(LOG_DEBUG, "reg[0x%02X] = 0x%02X\r\n", reg, *(&gt_ads114s08_regs.id.value + reg));
    }

    gt_ads114s08_regs.status.u.fl_por = 0x00; /* clear POR flag */
    gt_ads114s08_regs.status.u.rdy = 0x00; /* clear device ready flag */
    ADS114S08_Write_Register(REG_ADDR_STATUS, gt_ads114s08_regs.status.value);

    gt_ads114s08_regs.datarate.u.dr = ADS_SPS_2000; /* 2000 SPS */
    ADS114S08_Write_Register(REG_ADDR_DATARATE, gt_ads114s08_regs.datarate.value);

    ADS114S08_Get_ADC_Offset();
    gn_ads114s08_init_flag = 1;

#ifdef USE_DISPLAY_DEVICE_REGS
    ADS114S08_Dump_Registers();
#endif
    print(LOG_DEBUG, "\r\n %s Done\r\n", __func__);
}

void ADC_DRDY_INT_Handler(void)
{
    uint16_t temp = 0;
    temp = ADS114S08_Get_RData();

    if (temp > 32767)
    {
        temp = 0;
    }

    if (gn_adc_read_count)
    {
        gn_ads114s08_adc_temp += temp;
        --gn_adc_read_count;
    }

    if (gn_adc_read_count == 0)
    {
        gb_ads114s08_drdy_done = 1;
        ADS114S08_Set_Start(0);    /* stop continuous conversion */
    }
}

uint16_t ADS114S08_Get_ADC_Value(void)
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

double JigBD_IF_Convert_Adc_To_Current(uint16_t adc, current_gain_t gain)
{
	double ret = 0;
	switch (gain)
	{
		case GAIN_HIGH :
			ret = ((double)adc * ADC_CONV_COEFF_HIGH);	/* max  40mA */
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

uint16_t JigBD_IF_Convert_Current_To_ADC(double current_A, current_gain_t gain)
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
