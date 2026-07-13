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
#include "drv_spi.h"
#include "ads124s08.h"
#include "comm_debugging.h"

//#define USE_DISPLAY_DEVICE_REGS
#define ADS114S08_READ_COUNT    (16U) /* must be power of 2 */

/* Commands */
#define CMD_NOP                 (0x00U)
#define CMD_WAKEUP              (0x02U) /* or 0x03 */
#define CMD_POWER_DOWN          (0x04U) /* or 0x05 */
#define CMD_RESET               (0x06U) /* or 0x07 */
#define CMD_START               (0x08U) /* or 0x09 */
#define CMD_STOP                (0x0AU) /* or 0x0B */
#define CMD_SYOCAL              (0x16U) /* System offset calibration */
#define CMD_SYGCAL              (0x17U) /* System gain calibration */
#define CMD_SFOCAL              (0x19U) /* Self offset calibration */
#define CMD_RDATA               (0x12U) /* or 0x13 */
#define CMD_RREG                (0x20U) /* Read nnnnn registers starting at address rrrrr */
#define CMD_WREG                (0x40U) /* Write nnnnn registers starting at address rrrrr */

#define LTC_R_RIN               (2200.0f)
#define LTC_R_ROUT              (3300.0f)
#define LTC_R_RS_HIGH           (13.1f)
//#define LTC_R_RS_MID            (200.0f)
#define LTC_R_RS_MID            (75.0f)
#define LTC_R_RS_LOW            (3900.0f)

#define ICC_R                   (22.0f)

#define ADC_VREF                (5000.0f) /* mV */
#define ADC_RES                 (32767U)
#define mVOLTAGE_PER_ADC        (ADC_VREF / (float)ADC_RES)
#define ADC_CONV_COEFF_HIGH     (mVOLTAGE_PER_ADC * (LTC_R_RIN / (LTC_R_RS_HIGH * LTC_R_ROUT)))
#define ADC_CONV_COEFF_MID      (mVOLTAGE_PER_ADC * (LTC_R_RIN / (LTC_R_RS_MID * LTC_R_ROUT)))
#define ADC_CONV_COEFF_LOW      (mVOLTAGE_PER_ADC * (LTC_R_RIN / (LTC_R_RS_LOW * LTC_R_ROUT)))

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

typedef union
{
    uint8_t ALL[REG_ADDR_MAX];
    struct {
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
    }regs;
}ads114s08_regs_t;

typedef enum tag_ADC_SPS_T
{
    ADS_SPS_2_5 = 0U,
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
static ads114s08_regs_t gt_ads114s08_regs;

static volatile bool gb_ads114s08_drdy_done;

static uint32_t gn_ads114s08_adc_temp;
static uint16_t gn_adc_read_count;
static uint16_t gn_ads114s08_offset[XD_CH_MAX];

volatile uint16_t gn_ads114s08_read_timeout;

#ifdef USE_DISPLAY_DEVICE_REGS
static void ADS114S08_Dump_Registers(void)
{
    comm_UART_Printf(LOG_LV_INFO, "\r\n======== ADS114S08 regs value ========");

    comm_UART_Printf(LOG_LV_INFO, "\r\nID : 0x%X", gt_ads114s08_regs.regs.id.value);
    comm_UART_Printf(LOG_LV_INFO, "\r\n\tDEV ID : %s", (gt_ads114s08_regs.regs.id.u.dev_id == 0x04 ? "ADS114S08" : (gt_ads114s08_regs.regs.id.u.dev_id == 0x05 ? "ADS114S06" : "UNKNOWN")));

    comm_UART_Printf(LOG_LV_INFO, "\r\nDevice Status : 0x%X", gt_ads114s08_regs.regs.status.value);
    comm_UART_Printf(LOG_LV_INFO, "\r\n\tPOR : %u", gt_ads114s08_regs.regs.status.u.fl_por);
    comm_UART_Printf(LOG_LV_INFO, "\r\n\tRDY : %u", gt_ads114s08_regs.regs.status.u.rdy);

    comm_UART_Printf(LOG_LV_INFO, "\r\nData Rate : 0x%X", gt_ads114s08_regs.regs.datarate.value);
    comm_UART_Printf(LOG_LV_INFO, "\r\n\tDR : %u", gt_ads114s08_regs.regs.datarate.u.dr);

    comm_UART_Printf(LOG_LV_INFO, "\r\nSYS : 0x%X", gt_ads114s08_regs.regs.sys.value);
    comm_UART_Printf(LOG_LV_INFO, "\r\n\tSENDSTAT : %u", gt_ads114s08_regs.regs.sys.u.sendstat);
    comm_UART_Printf(LOG_LV_INFO, "\r\n\tCRC : %u", gt_ads114s08_regs.regs.sys.u.crc);

    comm_UART_Printf(LOG_LV_INFO, "\r\n======================================");
}
#endif

static uint8_t ADS114S08_Read_Register(uint8_t reg_addr)
{
    uint8_t TxBuffer[3U] = {0, };
    uint8_t RxBuffer[3U] = {0, };

    TxBuffer[0U] = (CMD_RREG | (reg_addr & 0x1F));

    ADS_CS_LO();
    spi_read8(SPI2, TxBuffer, RxBuffer, 3U, 20U);
    ADS_CS_HI();

    return RxBuffer[2U];
}

static int32_t ADS114S08_Get_RData(void)
{
    uint8_t TxBuffer[3U] = {0, };
    uint8_t RxBuffer[3U] = {0, };

    TxBuffer[0U] = CMD_RDATA;

    ADS_CS_LO();
    spi_read8(SPI2, TxBuffer, RxBuffer, 3U, 20);
    ADS_CS_HI();

    return (int32_t)((RxBuffer[1U] << 8U) | RxBuffer[2U]);
}

static void ADS114S08_Write_Register(uint8_t reg_addr, uint8_t reg_data)
{
    uint8_t TxBuffer[3U];

    TxBuffer[0U] = (CMD_WREG | (reg_addr & 0x1F));
    TxBuffer[1U] = 0x00;
    TxBuffer[2U] = reg_data;

    ADS_CS_LO();
    spi_write8(SPI2, TxBuffer, 3U, 20);
    ADS_CS_HI();
}

static void ADS114S08_Set_CMD(uint8_t cmd_code)
{
    ADS_CS_LO();
    spi_write8(SPI2, &cmd_code, 1U, 20);
    ADS_CS_HI();
}

#pragma optimize=none
static void ADS114S08_Get_ADC_Offset()
{
    comm_UART_Printf(LOG_LV_DEBUG, "\r\n ...Get ADC Offset Start...\r\n");
    gpio_set_current_gain(GAIN_LOW);
    gpio_set_vled_9v(VLED_ON);

    for (XD_CH_t output_ch = XD_CH_01 ; output_ch < XD_CH_MAX ; ++output_ch)
    {
        gpio_set_demux_channel_selection(output_ch);
        ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);
        LL_mDelay(10);

        ADS114S08_Set_Start(true);
        if(true == ADS114S08_Wait_Done())
        {
            gn_ads114s08_offset[output_ch] = ADS114S08_Get_ADC_Value();
        }
    }

    gpio_set_vled_9v(VLED_OFF);
    gpio_set_demux_channel_selection(XD_CH_MAX);
    comm_UART_Printf(LOG_LV_DEBUG, "\r\n ...Get ADC Offset Done...0x%04X, \r\n", gn_ads114s08_offset[0]);
}

void ADS114S08_Select_Input_CH(uint8_t input_p, uint8_t input_n)
{
    ads114s08_inpmux_t t = {0,};

    t.u.muxp = input_p;
    t.u.muxn = input_n;

    ADS114S08_Write_Register(REG_ADDR_INPMUX, t.value);
}

static void ADS114S08_Reset(void)
{
    ADS114S08_Set_CMD(CMD_RESET);
}

void ADS114S08_Set_Start(bool b_start)
{
    if(true == b_start)
    {
        gb_ads114s08_drdy_done = false;
        gn_ads114s08_adc_temp = 0;
        gn_adc_read_count = ADS114S08_READ_COUNT;
        gn_ads114s08_read_timeout = 15; // 2000SPS * 16 EA = 8ms
        ADS114S08_Set_CMD(CMD_START);
    }
    else
    {
        ADS114S08_Set_CMD(CMD_STOP);
    }
}

bool ADS114S08_Wait_Done(void)
{
    while(false == gb_ads114s08_drdy_done)
    {
        if(0U == gn_ads114s08_read_timeout)
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\nFunction[%s] timeout!!!", __func__);
            return false;
        }
    }

    return true;
}

void ADS114S08_Init(void)
{
    ADS114S08_Reset();
    LL_mDelay(1);

    for(uint8_t reg = REG_ADDR_ID ; reg < REG_ADDR_MAX ; ++reg)
    {
        gt_ads114s08_regs.ALL[reg] = ADS114S08_Read_Register(reg);
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nreg[0x%02X] = 0x%02X", reg, gt_ads114s08_regs.ALL[reg]);
    }

    gt_ads114s08_regs.regs.status.u.fl_por = 0U; /* clear POR flag */
    gt_ads114s08_regs.regs.status.u.rdy = 0U; /* clear device ready flag */
    ADS114S08_Write_Register(REG_ADDR_STATUS, gt_ads114s08_regs.regs.status.value);

    gt_ads114s08_regs.regs.datarate.u.dr = ADS_SPS_2000; /* 2000 SPS */
    ADS114S08_Write_Register(REG_ADDR_DATARATE, gt_ads114s08_regs.regs.datarate.value);

    ADS114S08_Get_ADC_Offset();

#ifdef USE_DISPLAY_DEVICE_REGS
    ADS114S08_Dump_Registers();
#endif
    comm_UART_Printf(LOG_LV_DEBUG, "\r\nFunction[%s] Done\r\n", __func__);
}

void ADC_DRDY_INT_Handler(void)
{
    uint16_t temp = ADS114S08_Get_RData();

    if(temp > 32767U)
    {
        temp = 0U;
    }

    if(gn_adc_read_count)
    {
        gn_ads114s08_adc_temp += temp;
        --gn_adc_read_count;
    }

    if(0U == gn_adc_read_count)
    {
        gb_ads114s08_drdy_done = true;
        ADS114S08_Set_Start(false);    /* stop continuous conversion */
    }
}

uint16_t ADS114S08_Get_ADC_Value(void)
{
    return (uint16_t)((float)gn_ads114s08_adc_temp / ADS114S08_READ_COUNT + 0.5f);
}

float JigBD_IF_Convert_Adc_To_Current(uint16_t adc, current_gain_t gain)
{
    float ret = 0;
    switch (gain)
    {
        case GAIN_HIGH:
        {
            ret = ((float)adc * ADC_CONV_COEFF_HIGH);
            break;
        }
        case GAIN_MID:
        {
            ret = ((float)adc * ADC_CONV_COEFF_MID);
            break;
        }
        case GAIN_LOW:
        {
            ret = ((float)adc * ADC_CONV_COEFF_LOW);
            break;
        }
        default:
        {
            FATAL_INVALID_INPUT(gain);
            break;
        }
    }
    return ret; //mA
}

uint16_t JigBD_IF_Convert_Current_To_ADC(double current_A, current_gain_t gain)
{
    uint16_t ret = 0;
    switch (gain)
    {
        case GAIN_HIGH:
        {
            ret = (uint16_t)(current_A * 1000 / ADC_CONV_COEFF_HIGH + 0.5f); /* mA -> ADC */
            break;
        }
        case GAIN_MID:
        {
            ret = (uint16_t)(current_A * 1000 / ADC_CONV_COEFF_MID + 0.5f); /* mA -> ADC */
            break;
        }
        case GAIN_LOW:
        {
            ret = (uint16_t)(current_A * 1000 / ADC_CONV_COEFF_LOW + 0.5f); /* mA -> ADC */
            break;
        }
        default:
        {
            FATAL_INVALID_INPUT(gain);
            break;
        }
    }
    return ret; //adc
}

float JigBD_IF_Convert_Adc_To_Voltage(uint16_t adc)
{
    float ret = ((float)adc * mVOLTAGE_PER_ADC / 1000.0f); /* mV -> V */
    return ret; //mV
}

float JigBD_IF_Convert_Adc_To_ICC(uint16_t adc)
{
    float ret = ((float)adc * mVOLTAGE_PER_ADC) / ICC_R;
    return ret; //mA
}
