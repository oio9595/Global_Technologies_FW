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

#include "JigBd_IF.h"

#define ADS114S08_CH_XD_IOUT    (0)
#define ADS114S08_CH_XC_LDO     (1)
#define ADS114S08_CH_XC_DAC     (2)
#define ADS114S08_CH_XD_ICC_P   (3)
#define ADS114S08_CH_XD_ICC_N   (4)
#define ADS114S08_CH_XC_ICC_P   (8)
#define ADS114S08_CH_XC_ICC_N   (9)

#define ADS114S08_READ_COUNT    (16) /* must be power of 2 */

#define ADS114S_VREF            (5000)  /* 5V */
#define ADS114S_RESOLUTION      ((1U << (16 - 1)) - 1)  /* 16bit */
#define ADC_VOLT_PER_STEP       ((double)ADS114S_VREF / ADS114S_RESOLUTION)

#define ADS_AIN0                (0x00)
#define ADS_AIN1                (0x01)
#define ADS_AIN2                (0x02)
#define ADS_AIN3                (0x03)
#define ADS_AIN4                (0x04)
#define ADS_AIN5                (0x05)
#define ADS_AIN6                (0x06)
#define ADS_AIN7                (0x07)
#define ADS_AIN8                (0x08)
#define ADS_AIN9                (0x09)
#define ADS_AIN10               (0x0A)
#define ADS_AIN11               (0x0B)
#define ADS_AINCOM              (0x0C)

/* Commands */
#define CMD_NOP                 (0x00)
#define CMD_WAKEUP              (0x02) /* or 0x03 */
#define CMD_POWER_DOWN          (0x04) /* or 0x05 */
#define CMD_RESET               (0x06) /* or 0x07 */
#define CMD_START               (0x08) /* or 0x09 */
#define CMD_STOP                (0x0A) /* or 0x0B */
#define CMD_SYOCAL              (0x16) /* System offset calibration */
#define CMD_SYGCAL              (0x17) /* System gain calibration */
#define CMD_SFOCAL              (0x19) /* Self offset calibration */
#define CMD_RDATA               (0x12) /* or 0x13 */
#define CMD_RREG                (0x20) /* Read nnnnn registers starting at address rrrrr */
#define CMD_WREG                (0x40) /* Write nnnnn registers starting at address rrrrr */

typedef enum tag_REG_ADDR_T
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
}ads114s08_status_t;

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

typedef union
{
    uint8_t value;
    struct
    {
        uint8_t rsvd : 8;
    }u;
}ads114s08_reserved_t;

typedef struct
{
    ads114s08_id_t id;
    ads114s08_status_t status;
    ads114s08_inpmux_t inpmux;
    ads114s08_pga_t pga;
    ads114s08_datarate_t datarate;
    ads114s08_ref_t ref;
    ads114s08_idacmag_t idacmag;
    ads114s08_idacmux_t idacmux;
    ads114s08_vbias_t vbias;
    ads114s08_sys_t sys;
    ads114s08_reserved_t reserved1;
    ads114s08_ofcal0_t ofcal0;
    ads114s08_ofcal1_t ofcal1;
    ads114s08_reserved_t reserved2;
    ads114s08_fscal0_t fscal0;
    ads114s08_fscal1_t fscal1;
    ads114s08_gpiodat_t gpiodat;
    ads114s08_gpiocon_t gpiocon;
}ads114s08_regs_t;

// Device command prototypes
extern void ADS114S08_Init(void);

extern void ADS114S08_Select_Input_CH(uint8_t input);
extern void ADS114S08_Set_Start(uint8_t b_set);
extern void ADS114S08_Wait_Done(void);

extern void ADC_DRDY_INT_Handler(void);
extern uint16_t ADS114S08_Get_ADC_Value(void);

extern double JigBD_IF_Convert_Adc_To_Current(uint16_t adc, current_gain_t gain);
extern uint16_t JigBD_IF_Convert_Current_To_ADC(double current_A, current_gain_t gain);
extern double JigBD_IF_Convert_Adc_To_milli_Voltage(uint16_t adc);

#ifdef __cplusplus
}
#endif

#endif /* ADS124S08_H_ */
