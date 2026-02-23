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

#include "main.h"

#define USE_GPIO_START
#define USE_GPIO_RESET
//#define INTERNAL_CLK_DEF
#define EXTERNAL_CLK_DEF


#define ADS114S_VREF    (5000)  /* 5V */
#define ADS114S_RES     ((1U<<(16-1)) - 1)  /* 16bit */
#define ADS114S_CH      (12)
#define ADS114S_DEV_NUM (1)
#define ADC_VOLT_PER_STEP   ((double)ADS114S_VREF/ADS114S_RES)

/*
 * Address masks used for register addressing with
 * either a REGRD of REGWR mask
 *
 */
typedef enum _REG_ADDR_
{
    REG_ADDR_ID       = 0x00,
    REG_ADDR_STATUS,
    REG_ADDR_INPMUX,
    REG_ADDR_PGA,
    REG_ADDR_DATARATE,
    REG_ADDR_REF,
    REG_ADDR_IDACMAG,
    REG_ADDR_IDACMUX,
    REG_ADDR_VBIAS,
    REG_ADDR_SYS,
    REG_ADDR_RESERVED1,
    REG_ADDR_OFCAL0,
    REG_ADDR_OFCAL1,
    REG_ADDR_RESERVED2,
    REG_ADDR_FSCAL0,
    REG_ADDR_FSCAL1,
    REG_ADDR_GPIODAT,
    REG_ADDR_GPIOCON,
    REG_ADDR_MAX
}reg_addr_t;

typedef enum _ADC_CH_
{
	ADC1_CS	= 0,
	ADC2_CS,
	ADC_CS_ALL
} adc_ch_t;

typedef enum _ADC_DRDY_
{
	ADC1_DRDY	= 0,
	ADC2_DRDY 	
} adc_drdy_t;


/* Commands */
#define CMD_NOP             0x00
#define CMD_WAKEUP          0x02 /* or 0x03 */
#define CMD_POWER_DOWN      0x04 /* or 0x05 */
#define CMD_RESET           0x06 /* or 0x07 */
#define CMD_START           0x08 /* or 0x09 */
#define CMD_STOP            0x0A /* or 0x0B */
#define CMD_SYOCAL          0x16 /* System offset calibration */
#define CMD_SYGCAL          0x17 /* System gain calibration */
#define CMD_SFOCAL          0x19 /* Self offset calibration */
#define CMD_RDATA           0x12 /* or 0x13 */
#define CMD_REGRD           0x20 /* Read nnnnn registers starting at address rrrrr */
#define CMD_REGWR           0x40 /* Write nnnnn registers starting at address rrrrr */

/* 00h ID xxh RESERVED DEV_ID[2:0] */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t dev_id      : 3;
        uint8_t reserved    : 5;
    }u;
}ads114s08_id_t;

/* 01h STATUS 80h */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t fl_ref_l0   : 1;
        uint8_t fl_ref_l1   : 1;
        uint8_t fl_n_railn  : 1;
        uint8_t fl_n_railp  : 1;
        uint8_t fl_p_railn  : 1;
        uint8_t fl_p_railp  : 1;
        uint8_t rdy         : 1;
        uint8_t fl_por      : 1;
    }u;
}ads114s08_staus_t;

/* 02h INPMUX */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t muxn    : 4;
        uint8_t muxp    : 4;
    }u;
}ads114s08_inpmux_t;

/* 03h PGA */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t gain    : 3;
        uint8_t pga_en  : 2;
        uint8_t delay   : 3;
    }u;
}ads114s08_pga_t;

/* 04h DATARATE */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t dr      : 4;
        uint8_t filter  : 1;
        uint8_t mode    : 1;
        uint8_t clk     : 1;
        uint8_t g_chop  : 1;
    }u;
}ads114s08_datarate_t;

/* 05h REF */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t refcon      : 2;
        uint8_t refsel      : 2;
        uint8_t refn_buf    : 1;
        uint8_t refp_buf    : 1;
        uint8_t fl_ref_en   : 2;
    }u;
}ads114s08_ref_t;

/* 06h IDACMAG */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t imag        : 4;
        uint8_t zero1       : 1;
        uint8_t zero2       : 1;
        uint8_t psw         : 1;
        uint8_t fl_rail_en  : 1;
    }u;
}ads114s08_idacmag_t;

/* 07h IDACMUX */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t i1mux   : 4;
        uint8_t i2mux   : 4;
    }u;
}ads114s08_idacmux_t;

/* 08h VBIAS */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t vb_ain0     : 1;
        uint8_t vb_ain1     : 1;
        uint8_t vb_ain2     : 1;
        uint8_t vb_ain3     : 1;
        uint8_t vb_ain4     : 1;
        uint8_t vb_ain5     : 1;
        uint8_t vb_ainc     : 1;
        uint8_t vb_level    : 1;
    }u;
}ads114s08_vbias_t;

/* 09h SYS */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t sendstat    : 1;
        uint8_t crc         : 1;
        uint8_t timeout     : 1;
        uint8_t cal_samp    : 2;
        uint8_t sys_mon     : 3;
    }u;
}ads114s08_sys_t;

/* 0Bh OFCAL0 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofc : 8;
    }u;
}ads114s08_ofcal0_t;

/* 0Ch OFCAL1 */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t ofc : 8;
    }u;
}ads114s08_ofcal1_t;

/* FSCAL0 0Eh */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t fsc : 8;
    }u;
}ads114s08_fscal0_t;

/* FSCAL1 0Fh */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t fsc : 8;
    }u;
}ads114s08_fscal1_t;

/* GPIODAT 10h */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t dat : 4;
        uint8_t dir : 4;
    }u;
}ads114s08_gpiodat_t;

/* GPIOCON 11h */
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t con         : 4;
        uint8_t reserved    : 4;
    }u;
}ads114s08_gpiocon_t;


/* Register sub masks */
/* ADS124S08 Register 0 (ID) Definition
 *   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *        			RESERVED[4:0]      			 			 |  	      DEV_ID[2:0]
 *
 */
/* Define ID (revision) */
#define ADS_ID_A				0x00
#define ADS_ID_B				0x80
/* Define VER (device version) */
#define ADS_124S08				0x00U
#define ADS_124S06				0x01U
#define ADS_114S08				0x04U
#define ADS_114S06				0x05U
/* ADS124S08 Register 1 (STATUS) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *   FL_POR  |    nRDY   | FL_P_RAILP| FL_P_RAILN| FL_N_RAILP| FL_N_RAILN| FL_REF_L1 | FL_REF_L0
 *
 */
#define ADS_FL_POR				0x80
#define ADS_RDY					0x40
#define ADS_FL_P_RAILP			0x20
#define ADS_FL_P_RAILN			0x10
#define ADS_FL_N_RAILP			0x08
#define ADS_FL_N_RAILN			0x04
#define ADS_FL_REF_L1			0x02
#define ADS_FL_REF_L0			0x10
/* ADS124S08 Register 2 (INPMUX) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *         			MUXP[3:0]   				 |       			MUXN[3:0]
 *
 */
/* Define the ADC positive input channels (MUXP) */
#define ADS_P_AIN0				0x00
#define ADS_P_AIN1				0x10
#define ADS_P_AIN2				0x20
#define ADS_P_AIN3				0x30
#define ADS_P_AIN4				0x40
#define ADS_P_AIN5				0x50
#define ADS_P_AIN6				0x60
#define ADS_P_AIN7				0x70
#define ADS_P_AIN8				0x80
#define ADS_P_AIN9				0x90
#define ADS_P_AIN10				0xA0
#define ADS_P_AIN11				0xB0
#define ADS_P_AINCOM			0xC0
/* Define the ADC negative input channels (MUXN)*/
#define ADS_N_AIN0				0x00
#define ADS_N_AIN1				0x01
#define ADS_N_AIN2				0x02
#define ADS_N_AIN3				0x03
#define ADS_N_AIN4				0x04
#define ADS_N_AIN5				0x05
#define ADS_N_AIN6				0x06
#define ADS_N_AIN7				0x07
#define ADS_N_AIN8				0x08
#define ADS_N_AIN9				0x09
#define ADS_N_AIN10				0x0A
#define ADS_N_AIN11				0x0B
#define ADS_N_AINCOM			0x0C
/* ADS124S08 Register 3 (PGA) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *   		DELAY[2:0]		     	 |      PGA_EN[1:0]      |              GAIN[2:0]
 *
 */
/* Define conversion delay in tmod clock periods */
#define ADS_DELAY_14			0x00
#define ADS_DELAY_25			0x20
#define ADS_DELAY_64			0x40
#define ADS_DELAY_256			0x60
#define ADS_DELAY_1024			0x80
#define ADS_DELAY_2048			0xA0
#define ADS_DELAY_4096			0xC0
#define ADS_DELAY_1				0xE0
/* Define PGA control */
#define ADS_PGA_BYPASS			0x00
#define ADS_PGA_ENABLED			0x08
/* Define Gain */
#define ADS_GAIN_1				0x00
#define ADS_GAIN_2				0x01
#define ADS_GAIN_4				0x02
#define ADS_GAIN_8				0x03
#define ADS_GAIN_16				0x04
#define ADS_GAIN_32				0x05
#define ADS_GAIN_64				0x06
#define ADS_GAIN_128			0x07
/* ADS124S08 Register 4 (DATARATE) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *   G_CHOP  |    CLK    |    MODE   |   FILTER  | 				  DR[3:0]
 *
 */
#define ADS_GLOBALCHOP			0x80
#define ADS_CLKSEL_EXT			0x40
#define ADS_CONVMODE_SS			0x20
#define ADS_FILTERTYPE_LL		0x10
/* Define the data rate */
#define ADS_DR_2_5				0x00
#define ADS_DR_5				0x01
#define ADS_DR_10				0x02
#define ADS_DR_16				0x03
#define ADS_DR_20				0x04
#define ADS_DR_50				0x05
#define ADS_DR_60				0x06
#define ADS_DR_100				0x07
#define ADS_DR_200				0x08
#define ADS_DR_400				0x09
#define ADS_DR_800				0x0A
#define ADS_DR_1000				0x0B
#define ADS_DR_2000				0x0C
#define ADS_DR_4000				0x0D
/* ADS124S08 Register 5 (REF) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *	  FL_REF_EN[1:0]	 | nREFP_BUF | nREFN_BUF | 		REFSEL[1:0]		 | 		REFCON[1:0]
 *
 */
#define ADS_FLAG_REF_DISABLE	0x00
#define ADS_FLAG_REF_EN_L0		0x40
#define ADS_FLAG_REF_EN_BOTH	0x80
#define ADS_FLAG_REF_EN_10M		0xC0
#define ADS_REFP_BYP_DISABLE	0x20
#define ADS_REFP_BYP_ENABLE		0x00
#define ADS_REFN_BYP_DISABLE	0x10
#define ADS_REFN_BYP_ENABLE		0x00
#define ADS_REFSEL_P0			0x00
#define ADS_REFSEL_P1			0x04
#define ADS_REFSEL_INT			0x08
#define ADS_REFINT_OFF			0x00
#define ADS_REFINT_ON_PDWN		0x01
#define ADS_REFINT_ON_ALWAYS	0x02
/* ADS124S08 Register 6 (IDACMAG) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 * FL_RAIL_EN|	  PSW	 |     0     | 		0	 | 			    	IMAG[3:0]
 *
 */
#define ADS_FLAG_RAIL_ENABLE	0x80
#define ADS_FLAG_RAIL_DISABLE	0x00
#define ADS_PSW_OPEN			0x00
#define ADS_PSW_CLOSED			0x40
#define ADS_IDACMAG_OFF			0x00
#define ADS_IDACMAG_10			0x01
#define ADS_IDACMAG_50			0x02
#define ADS_IDACMAG_100			0x03
#define ADS_IDACMAG_250			0x04
#define ADS_IDACMAG_500			0x05
#define ADS_IDACMAG_750			0x06
#define ADS_IDACMAG_1000		0x07
#define ADS_IDACMAG_1500		0x08
#define ADS_IDACMAG_2000		0x09
/* ADS124S08 Register 7 (IDACMUX) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *                    I2MUX[3:0]                 |                   I1MUX[3:0]
 *
 */
/* Define IDAC2 Output */
#define ADS_IDAC2_A0			0x00
#define ADS_IDAC2_A1			0x10
#define ADS_IDAC2_A2			0x20
#define ADS_IDAC2_A3			0x30
#define ADS_IDAC2_A4			0x40
#define ADS_IDAC2_A5			0x50
#define ADS_IDAC2_A6			0x60
#define ADS_IDAC2_A7			0x70
#define ADS_IDAC2_A8			0x80
#define ADS_IDAC2_A9			0x90
#define ADS_IDAC2_A10			0xA0
#define ADS_IDAC2_A11			0xB0
#define ADS_IDAC2_AINCOM		0xC0
#define ADS_IDAC2_OFF			0xF0
/* Define IDAC1 Output */
#define ADS_IDAC1_A0			0x00
#define ADS_IDAC1_A1			0x01
#define ADS_IDAC1_A2			0x02
#define ADS_IDAC1_A3			0x03
#define ADS_IDAC1_A4			0x04
#define ADS_IDAC1_A5			0x05
#define ADS_IDAC1_A6			0x06
#define ADS_IDAC1_A7			0x07
#define ADS_IDAC1_A8			0x08
#define ADS_IDAC1_A9			0x09
#define ADS_IDAC1_A10			0x0A
#define ADS_IDAC1_A11			0x0B
#define ADS_IDAC1_AINCOM		0x0C
#define ADS_IDAC1_OFF			0x0F
/* ADS124S08 Register 8 (VBIAS) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *  VB_LEVEL | 	VB_AINC  |  VB_AIN5  |  VB_AIN4  |  VB_AIN3  |  VB_AIN2  |  VB_AIN1  |  VB_AIN0
 *
 */
#define ADS_VBIAS_LVL_DIV2		0x00
#define ADS_VBIAS_LVL_DIV12		0x80
/* Define VBIAS here */
#define ADS_VB_AINC				0x40
#define ADS_VB_AIN5				0x20
#define ADS_VB_AIN4				0x10
#define ADS_VB_AIN3				0x08
#define ADS_VB_AIN2				0x04
#define ADS_VB_AIN1				0x02
#define ADS_VB_AIN0				0x01
/* ADS124S08 Register 9 (SYS) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *			   SYS_MON[2:0]			 |	   CAL_SAMP[1:0]     |  TIMEOUT  | 	  CRC	 | SENDSTAT
 *
 */
#define ADS_SYS_MON_OFF			0x00
#define ADS_SYS_MON_SHORT		0x20
#define ADS_SYS_MON_TEMP		0x40
#define ADS_SYS_MON_ADIV4		0x60
#define ADS_SYS_MON_DDIV4		0x80
#define ADS_SYS_MON_BCS_2		0xA0
#define ADS_SYS_MON_BCS_1		0xC0
#define ADS_SYS_MON_BCS_10		0xE0
#define ADS_CALSAMPLE_1			0x00
#define ADS_CALSAMPLE_4			0x08
#define ADS_CALSAMPLE_8			0x10
#define ADS_CALSAMPLE_16		0x18
#define ADS_TIMEOUT_DISABLE		0x00
#define ADS_TIMEOUT_ENABLE		0x04
#define ADS_CRC_DISABLE			0x00
#define ADS_CRC_ENABLE			0x02
#define ADS_SENDSTATUS_DISABLE	0x00
#define ADS_SENDSTATUS_ENABLE	0x01
/* ADS124S08 Register A (OFCAL0) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *                                         OFC[7:0]
 *
 */
/* ADS124S08 Register B (OFCAL1) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *                                         OFC[15:8]
 *
 */
/* ADS124S08 Register C (OFCAL2) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *                                         OFC[23:16]
 *
 */
/* ADS124S08 Register D (FSCAL0) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *                                         FSC[7:0]
 *
 */
/* ADS124S08 Register E (FSCAL1) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *                                         FSC[15:8]
 *
 */
/* ADS124S08 Register F (FSCAL2) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *                                         FSC[23:16]
 *
 */
/* ADS124S08 Register 10 (GPIODAT) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *                      DIR[3:0]    			 | 					DAT[3:0]
 *
 */
/* Define GPIO direction (0-Output; 1-Input) here */
#define ADS_GPIO0_DIR_INPUT		0x10
#define ADS_GPIO1_DIR_INPUT		0x20
#define ADS_GPIO2_DIR_INPUT		0x40
#define ADS_GPIO3_DIR_INPUT		0x80
/*
 *
 */
/* Define GPIO data here */
/*
 *
 */
/* ADS124S08 Register 11 (GPIOCON) Definition */
/*   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
 *--------------------------------------------------------------------------------------------
 *     0	 |	   0	 |	   0	 |	   0     |                    CON[3:0]
 *
 */
/* Define GPIO configuration (0-Analog Input; 1-GPIO) here */
#define ADS_GPIO0_DIR_INPUT		0x10
#define ADS_GPIO1_DIR_INPUT		0x20
#define ADS_GPIO2_DIR_INPUT		0x40
#define ADS_GPIO3_DIR_INPUT		0x80
/*
 *
 */
/* Lengths of conversion data components */
#define DATA_LENGTH			3
#define STATUS_LENGTH		1
#define CRC_LENGTH			1
/*
 * The time we have to wait for the CS GPIO to actually
 * pull down before we start sending SCLKs
 *
 */
#define CHIP_SELECT_WAIT_TIME	0
/* Flag to signal that we are in the process of collecting data */
#define DATA_MODE_NORMAL	0x00
#define DATA_MODE_STATUS	0x01
#define DATA_MODE_CRC		0x02

#define ADS_AIN0            0x00
#define ADS_AIN1            0x01
#define ADS_AIN2            0x02
#define ADS_AIN3            0x03
#define ADS_AIN4            0x04
#define ADS_AIN5            0x05
#define ADS_AIN6            0x06
#define ADS_AIN7            0x07
#define ADS_AIN8            0x08
#define ADS_AIN9            0x09
#define ADS_AIN10           0x0A
#define ADS_AIN11           0x0B
#define ADS_AINCOM          0x0C

#if 0
/*
 * The xferWord is controller specific and will
 * need to adapt to the particular micro in use
 * \param tx id data to be transmitted
 *
 */
inline uint8_t xferWord(SPI_HandleTypeDef *hspi, uint8_t tx)
{
    uint8_t rx = 0;
#if 0
    HAL_SPI_TransmitReceive(hspi, &tx, &rx, 1, 10);
#else
    if ((hspi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
    {
      /* Enable SPI peripheral */
      __HAL_SPI_ENABLE(hspi);
    }

    /* Set up data for the next xmit */
    while(!(hspi->Instance->SR & SPI_SR_TXE)){};
    *(volatile uint8_t *)&(hspi->Instance->DR) = tx;
    /* Wait for data to appear */
    while(!(hspi->Instance->SR & SPI_SR_RXNE)){};
    /* Grab that data*/
    rx = *(volatile uint8_t *)&(hspi->Instance->DR);
#endif

    return rx;
}
#endif

// Device command prototypes
extern void ads114s08_init(void);
extern uint8_t ads114s08_get_reg(unsigned int regnum, adc_ch_t ch);
extern void ads114s08_get_regs(uint8_t regnum, uint8_t count, uint8_t *data, adc_ch_t ch);
extern void ads114s08_set_reg(uint8_t regnum, uint8_t data, adc_ch_t ch);
extern void  ads114s08_set_regs(uint8_t regnum, uint8_t howmuch, uint8_t *data, adc_ch_t ch);

extern void ads114s08_set_input(uint8_t input_p, uint8_t input_n, adc_ch_t ch);
extern void ads114s08_select_single_ended_input(uint8_t input, adc_ch_t ch);

extern void ads114s08_set_command(uint8_t op_code, adc_ch_t ch);
extern void ads114s08_set_restart(adc_ch_t ch);
extern void ads114s08_set_reset(adc_ch_t ch);
extern void ads114s08_set_start(uint8_t b_set, adc_ch_t ch);
extern int32_t ads114s08_get_data(uint8_t *dStatus, uint8_t *dCRC, adc_ch_t ch);
extern int32_t ads114s08_get_rdata(uint8_t *dStatus, uint8_t *dCRC, adc_ch_t ch);

#endif /* ADS124S08_H_ */
