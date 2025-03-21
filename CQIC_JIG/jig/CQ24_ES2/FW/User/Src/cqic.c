/* 
 * File:   nmic.c
 * Author: GT
 *
 * Created on 2022, 07, 06
 */

#define __CQIC_C__
#include "main.h"
#include <stdio.h>
#include "common.h"
#include "cqic.h"
#include "cqic_test.h"
#include "log.h"
#include "ads124s08.h"

void cqic_init(void);
void cqic_read(uint8_t device_id, uint16_t* cmd, uint16_t* pData, uint16_t length);
void cqic_write(uint8_t device_id, uint16_t* pData, uint16_t length);
void spi_write(uint16_t* pData, uint16_t length);
void spi_read(uint16_t* pTxData, uint16_t* pRxData, uint16_t length);
void cqic_trim_reg_verify(void);
void cqic_trim_value_verify(void);
void cqic_trim_data_to_otp(void);
void cqic_otp_read(void);
void cqic_otp_write(void);
void aqic_mode_config(uint8_t mode);
void cqic_mode_out(uint16_t mode);
void set_cqic_power(uint8_t on);
void cqic_output_disable(void);
void cqic_output_enable(uint16_t verf, uint16_t din);
void cq24_ch_select(uint8_t in, adc_ch_t ch);


//static uint8_t gn_daisy_chain_number = 1;
extern SPI_HandleTypeDef hspi2;
extern uint16_t cqic_Vref_offset_value;
extern uint16_t cqic_vmode_value;
extern uint16_t cqic_trimming_value[24][2];

#define _Y__  TRUE
const uint8_t REGISTER_USED_MAP_CQ24[]={
/*          0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F */
/*0x00*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, 
/*0x10*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, 0x00, 0x00, 0x00, 
/*0x20*/    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/*0x30*/    _Y__, _Y__, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00, 
/*0x40*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__,
/*0x50*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__,
/*0x60*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__,
/*0x70*/    _Y__
};

void cqic_cs_enable(uint8_t num)
{
    switch (num)
    {
        case 0:
            CQIC1_NSCS_LO();
            break;
        case 1:
            CQIC2_NSCS_LO();
            break;
        default:
            CQIC1_NSCS_LO();
            CQIC2_NSCS_LO();
            break;
    }
}

void cqic_cs_disable(uint8_t num)
{
    switch (num)
    {
        case 0:
            CQIC1_NSCS_HI();
            break;
        case 1:
            CQIC2_NSCS_HI();
            break;
        default:
            CQIC1_NSCS_HI();
            CQIC2_NSCS_HI();
            break;
    }
}

uint16_t ret_val[24][2];
uint16_t data[4];

void cqic_init(void)
{
	cqic_cmd1_type cmd1 = {0, };
	
	set_cqic_5_0V_on();
	set_cqic_power_on();
	HAL_Delay(30);

	cmd1.u.cmd_id = CMD_01;
	cmd1.u.rw = SPI_WR;
	cmd1.u.addr = 0x00;
	cmd1.u.data = 0x001;	
	cqic_write(CQIC_1, &cmd1.value, 1);
	HAL_Delay(10);

	cmd1.u.cmd_id = CMD_01;
	cmd1.u.rw = SPI_WR;
	cmd1.u.addr = 0x00;
	cmd1.u.data = 0x000;
	cqic_write(CQIC_1, &cmd1.value, 1);
	HAL_Delay(3);
	
	cqic_output_enable(4095, 4095);

	cqic_otp_read();
	cqic_reg_read_all();
}

void set_cqic_power(uint8_t on)
{
    if(on == PWR_OFF) /* off */
    {
        /* AMIC_PWR_CTL : OFF, LOW */
        HAL_GPIO_WritePin(CQ24_VCC_EN_GPIO_Port, CQ24_VCC_EN_Pin, GPIO_PIN_RESET);
    }
    else
    {
        /* AMIC_PWR_CTL : ON, HIGH */
        HAL_GPIO_WritePin(CQ24_VCC_EN_GPIO_Port, CQ24_VCC_EN_Pin, GPIO_PIN_SET);
    }
}

void spi_write(uint16_t* pData, uint16_t length)
{
    HAL_SPI_Transmit(&hspi2, (uint8_t*)pData, length, 100);
}

void spi_read(uint16_t* pTxData, uint16_t* pRxData, uint16_t length)
{
    HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)pTxData, (uint8_t*)pRxData, length, 100);
}

void cqic_write(uint8_t device_id, uint16_t* pData, uint16_t length)
{
    cqic_cs_enable(device_id);
    spi_write(pData, length);
    cqic_cs_disable(device_id);
    
    //snprintf(msg_buffer, sizeof msg_buffer, "[WRITE]cqic_write %u [ 0x%X, 0x%X ] \r\n", length, pData[0], pData[1]);
    //print(msg_buffer);
}

void cqic_read(uint8_t device_id, uint16_t* cmd, uint16_t* pData, uint16_t length)
{
	uint16_t rx_buffer[5] = {0, };
    
    cqic_cs_enable(device_id);
    spi_read(cmd, rx_buffer, length);
    cqic_cs_disable(device_id);

    //snprintf(msg_buffer, sizeof msg_buffer, "[READ]nmic_read %u [ 0x%X, 0x%X ][ 0x%X, 0x%X ] \r\n", length, pData[0], pData[1], rx_buffer[0], rx_buffer[1]);
    //print(LOG_DEBUG, msg_buffer);

    if(length == 1)
    {
        pData[1] = rx_buffer[0];
    }
    else    
    {
        pData[1] = rx_buffer[1];
    }
}

void cq24_ch_select(uint8_t in, adc_ch_t ch)
{
	uint16_t num_ch;
	uint32_t data;
	cqic_cmd2_type cmd2;

	num_ch = (ch*12) + in;
	data = 0x00000001<<num_ch;

	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= OUTPUT_CH_EN1;
	cmd2.u.data 	= (uint16_t)(data & 0x0000FFFF);
	cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= OUTPUT_CH_EN2;
	cmd2.u.data 	= (uint16_t)(data >> 16);
	cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */

	return;
 	
}

void cqic_output_enable(uint16_t vref, uint16_t din)
{
	cqic_cmd1_type cmd1;
	cqic_cmd2_type cmd2;

	/* disable channel output */

	cmd1.value = 0;
	cmd2.value32 = 0;
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= OUTPUT_CH_EN1;
	cmd2.u.data 	= 0xFFFF;	
	//cmd2.u.data 	= 0x0000;	
	cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= OUTPUT_CH_EN2;
	cmd2.u.data 	= 0x00FF;	
	//cmd2.u.data 	= 0x0000;	
	cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= FRAME_HEADER;
	cmd2.u.data 	= vref;
	cqic_write(CQIC_1, cmd2.value, 2);	/* set ANA DAC 10bit */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= DUTY;
	cmd2.u.data 	= 0xFF; 
	cqic_write(CQIC_1, cmd2.value, 2);	/* set duty 100% */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= LD_I;
	cmd2.u.data 	= din;	//300mV 
	cqic_write(CQIC_1, cmd2.value, 2);	/* set Current DAC 12bit */
	
	cmd1.u.cmd_id	= CMD_01;	
	cmd1.u.addr 	= FIX_CONTROL;
	cmd1.u.rw		= SPI_WR;
	cmd1.u.data 	= 0x170;
	//cmd1.u.data 	= 0x100;
	cqic_write(CQIC_1, &cmd1.value, 1); /* set LD_FORCE bit & start analog data out */
	HAL_Delay(30); 	

	return;
}

void cqic_output_disable(void)
{
	cqic_cmd1_type cmd1;
	cqic_cmd2_type cmd2;

	cmd1.value = 0;
	cmd2.value32 = 0;
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= OUTPUT_CH_EN1;
	cmd2.u.data 	= 0x0000;
	cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= OUTPUT_CH_EN2;
	cmd2.u.data 	= 0x0000;
	cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= FRAME_HEADER;
	cmd2.u.data 	= 0;
	cqic_write(CQIC_1, cmd2.value, 2);	/* set ANA DAC 10bit */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= DUTY;
	cmd2.u.data 	= 0;
	cqic_write(CQIC_1, cmd2.value, 2);	/* set duty 100% */
	
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= LD_I;
	cmd2.u.data 	= 0;	
	cqic_write(CQIC_1, cmd2.value, 2);	/* set Current DAC 12bit */
	
	cmd1.u.cmd_id	= CMD_01;	
	cmd1.u.addr 	= FIX_CONTROL;
	cmd1.u.rw		= SPI_WR;
	cmd1.u.data 	= 0x0; 
	cqic_write(CQIC_1, &cmd1.value, 1); /* set LD_FORCE bit & start analog data out */

	return;
 }


void cqic_otp_write(void)
{
	cqic_cmd2_type cmd2;
	uint8_t addr;
        

	HAL_Delay(300);

	cmd2.u.addr 	= OTP_PROTECTION;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0xA5A5;
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(1);

	cmd2.u.addr	= TRIM_CONTROL;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0xFFF0;	// max prpgram cycle
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(1);

	cmd2.u.addr	= OTP_WR_CONTROL;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x0008;	// wsel max
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(1);

	cmd2.u.addr	= OTP_RD_PG_CONTROL;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x0001;	
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(10);

	cmd2.u.addr 	= MODE_DAC_OFFSET;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= cqic_vmode_value; //VREFO_4_5V_offset_count;
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(10);


	cmd2.u.addr 	= V_ANA_GAIN;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= cqic_Vref_offset_value; //VREFO_4_5V_offset_count;
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(10);

	for(addr=0; addr< CH_MAX ; addr++)
	{
		cmd2.u.addr 	= DAC_OFFSET_BASE + addr;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.data 	= cqic_trimming_value[addr][DAC_OFFSET];
		cqic_write(CQIC_1, cmd2.value, 2);
		HAL_Delay(1);
	}

	for(addr=0; addr< (CH_MAX); addr++)
	{
		cmd2.u.addr 	= DAC_VREF_OFFSET_BASE + addr;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.data 	= cqic_trimming_value[addr][DAC_VREF];
		cqic_write(CQIC_1, cmd2.value, 2);
		HAL_Delay(1);
	}

	cmd2.u.addr	= OTP_RD_PG_CONTROL;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x0000;	
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(1);

	cmd2.u.addr 	= OTP_PROTECTION;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x5A5A;
	cqic_write(CQIC_1, cmd2.value, 2);

	//HAL_Delay(100);

	return;	
}

void cqic_otp_read(void)
{
	cqic_cmd2_type cmd2;
	uint16_t rx_data[2]={0,0};
	uint8_t ch;

	print("\n\r CQIC OTP Data Dump...........");

	cmd2.u.addr 	= OTP_PROTECTION;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0xA5A5;
	cqic_write(CQIC_1, cmd2.value, 2);

	cmd2.u.addr	= TRIM_CONTROL;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x000F;		// max read cycle
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(1);
	

	cmd2.u.addr = OTP_RD_PG_CONTROL;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x0002;		
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(3);


	cmd2.u.addr 	= MODE_DAC_OFFSET;
	cmd2.u.rw		= SPI_RD;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0;
	cqic_read(CQIC_1, cmd2.value, rx_data, 2);
	HAL_Delay(1);
	
	snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_mode  Rqg_data[0x%x]", rx_data[1]);
	print(msg_buffer);

	
	cmd2.u.addr 	= V_ANA_GAIN;
	cmd2.u.rw		= SPI_RD;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0;
	cqic_read(CQIC_1, cmd2.value, rx_data, 2);
	HAL_Delay(1);
	
	snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_max  Rqg_data[0x%x]", rx_data[1]);
	print(msg_buffer);

	for(ch=0; ch<CH_MAX; ch++)
	{
		cmd2.u.addr 	= DAC_OFFSET_BASE+ch;
		cmd2.u.rw		= SPI_RD;
		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.data 	= 0;
		cqic_read(CQIC_1, cmd2.value, rx_data, 2);

		snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_12B_offset CH[%2d], Reg_data[0x%x]", ch+1, rx_data[1]);
		print(msg_buffer);
	}

	for(ch=0; ch<CH_MAX; ch++)
	{
		cmd2.u.addr 	= DAC_VREF_OFFSET_BASE+ch;
		cmd2.u.rw		= SPI_RD;
		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.data 	= 0;
		cqic_read(CQIC_1, cmd2.value, rx_data, 2);

		snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_10B_offset CH[%2d], Reg_data[0x%x]", ch+1, rx_data[1]);
		print(msg_buffer);
	}

	cmd2.u.addr = OTP_RD_PG_CONTROL;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x0000;		
	cqic_write(CQIC_1, cmd2.value, 2);

	cmd2.u.addr 	= OTP_PROTECTION;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x5A5A;
	cqic_write(CQIC_1, cmd2.value, 2);
}

void cqic_trim_data_to_otp(void)
{
	print("\n\r Program Start....");

  set_cqic_5_0V_off();
  HAL_Delay(10);
  set_cqic_5_7V_on();
  HAL_Delay(10);

	cqic_otp_write();
	HAL_Delay(1000);

  set_cqic_5_7V_off();
  HAL_Delay(10);
  set_cqic_5_0V_on();

	print("\n\r Program Done!!!");

	return;	
}

void cqic_trim_value_verify(void)
{
	uint8_t ch;
	
	print("\n\r Trimming Data !!!");

	snprintf(msg_buffer, sizeof msg_buffer, "\n\r MODE_DAC_OFFSET(2.0V) \t[0%4x]", cqic_vmode_value);
	print(msg_buffer);

	
	snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_ANA_GAIN(4.5V) \t[0%4x]", cqic_Vref_offset_value);
	print(msg_buffer);

	for(ch=0; ch<24; ch++)
	{
		snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_10B_offset CH[%2d],[0x%4x]", ch+1, cqic_trimming_value[ch][DAC_VREF]);
		print(msg_buffer);
	}
	
	for(ch=0; ch<24; ch++)
	{
		snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_12B_offset CH[%2d],[0x%4x]", ch+1, cqic_trimming_value[ch][DAC_OFFSET]);
		print(msg_buffer);
	}
}
void cqic_trim_reg_verify(void)
{
	cqic_cmd2_type cmd2;
	uint16_t rx_data[2];
	uint8_t ch;

	cmd2.value32 = 0;
	
	cmd2.u.addr 	= OTP_PROTECTION;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0xA5A5;
	cqic_write(CQIC_1, cmd2.value, 2);

	cmd2.u.addr 	= MODE_DAC_OFFSET;
	cmd2.u.rw 		= SPI_RD;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0;
	cqic_read(CQIC_1, cmd2.value, rx_data, 2);

	snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_mode  Trim_data[0x%4x], Reg_data[0x%4x], Result[%s]", cqic_vmode_value,rx_data[1], (cqic_vmode_value==rx_data[1] ? "OK!!!":"NG..."));
	print(msg_buffer);

	cmd2.u.addr 	= V_ANA_GAIN;
	cmd2.u.rw 		= SPI_RD;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0;
	cqic_read(CQIC_1, cmd2.value, rx_data, 2);

	snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_ana  Trim_data[0x%4x], Reg_data[0x%4x], Result[%s]", cqic_Vref_offset_value,rx_data[1], (cqic_Vref_offset_value==rx_data[1] ? "OK!!!":"NG..."));
	print(msg_buffer);

	for(ch=0; ch<CH_MAX; ch++)
	{
		cmd2.u.addr 	= DAC_VREF_OFFSET_BASE+ch;
		cmd2.u.rw 		= SPI_RD;
		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.data 	= 0;
		cqic_read(CQIC_1, cmd2.value, rx_data, 2);

		snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_10B_offset CH[%2d], Trim_data[0x%4x], Reg_data[0x%4x], Result[%s]", ch+1, cqic_trimming_value[ch][DAC_VREF], rx_data[1],(cqic_trimming_value[ch][DAC_VREF]==rx_data[1] ? "OK!!!":"NG..."));
		print(msg_buffer);
	}

	for(ch=0; ch<CH_MAX; ch++)
	{
		cmd2.u.addr 	= DAC_OFFSET_BASE+ch;
		cmd2.u.rw 		= SPI_RD;
		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.data 	= 0;
		cqic_read(CQIC_1, cmd2.value, rx_data, 2);

		snprintf(msg_buffer, sizeof msg_buffer, "\n\r V_12B_offset CH[%2d], Trim_data[0x%4x], Reg_data[0x%4x], Result[%s]", ch+1, cqic_trimming_value[ch][DAC_OFFSET],rx_data[1],(cqic_trimming_value[ch][DAC_OFFSET]==rx_data[1] ? "OK!!!":"NG..."));
		print(msg_buffer);
	}

	cmd2.u.addr	= OTP_RD_PG_CONTROL;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x0000;		
	cqic_write(CQIC_1, cmd2.value, 2);

	cmd2.u.addr 	= OTP_PROTECTION;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0x5A5A;
	cqic_write(CQIC_1, cmd2.value, 2);

	return;
}


const uint16_t aqic_mode_tbl[4][4] = 
{
	{1,2,3,0},
	{2,3,0,1},
	{3,0,1,2},
	{0,1,2,3}
};

void aqic_mode_config(uint8_t mode)
{
	cqic_cmd1_type cmd1;
	cqic_cmd2_type cmd2;
	uint16_t data;
	uint8_t i;

	cmd1.value = 0;
	data = 0x0505;
	
	cmd1.u.cmd_id 	= CMD_01;
	cmd1.u.rw		= SPI_WR;
	cmd1.u.addr 	= FIX_CONTROL;
	cmd1.u.data		= data;	// DUP:1, HOLD_OUT:1
	cqic_write(CQIC_1, &cmd1.value, 1);
	HAL_Delay(3);

	for(i=0; i<4; i++)
	{
		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.addr 	= FRAME_HEADER;
		cmd2.u.data = (aqic_mode_tbl[mode][i]<<10);
		cqic_write(CQIC_1, cmd2.value, 2);
		HAL_Delay(2);

		/* CS_OUT toggle */	
		data |= 0x0008; 	// CS_OUT: 1
		cmd1.u.data		= data;	
		cqic_write(CQIC_1, &cmd1.value, 1);
		HAL_Delay(2);

		data &= ~0x0008; 	// CS_OUT: 0
		cmd1.u.data 	= data; 
		cqic_write(CQIC_1, &cmd1.value, 1);
		HAL_Delay(2);
	}

	HAL_Delay(2);
	
	cmd1.u.cmd_id 	= CMD_01;
	cmd1.u.rw		= SPI_WR;
	cmd1.u.addr 	= FIX_CONTROL;
	cmd1.u.data		= 0x0400;	// DUP:1, HOLD_OUT:0
	cqic_write(CQIC_1, &cmd1.value, 1);

	return;
}

void cqic_mode_out(uint16_t mode)
{
	cqic_cmd1_type cmd1;
	cqic_cmd2_type cmd2;
	uint16_t data;

	cmd1.value = 0;
	data = 0x0505;
	
	cmd1.u.cmd_id 	= CMD_01;
	cmd1.u.rw		= SPI_WR;
	cmd1.u.addr 	= FIX_CONTROL;
	cmd1.u.data		= data;	// DUP:1, HOLD_OUT:1
	cqic_write(CQIC_1, &cmd1.value, 1);
	HAL_Delay(3);

	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= FRAME_HEADER;
	cmd2.u.data 	= mode<<12;
	cqic_write(CQIC_1, cmd2.value, 2);
	HAL_Delay(2);

#if 1
	/* CS_OUT toggle */	
	data |= 0x0008; 	// CS_OUT: 1
	cmd1.u.data		= data;	
	cqic_write(CQIC_1, &cmd1.value, 1);
	HAL_Delay(2);

	data &= ~0x0008; 	// CS_OUT: 0
	cmd1.u.data 	= data; 
	cqic_write(CQIC_1, &cmd1.value, 1);
	HAL_Delay(2);
#endif

	HAL_Delay(2);

	cmd1.u.cmd_id 	= CMD_01;
	cmd1.u.rw		= SPI_WR;
	cmd1.u.addr 	= FIX_CONTROL;
	cmd1.u.data		= 0x0400;	// DUP:1, HOLD_OUT:0
	cqic_write(CQIC_1, &cmd1.value, 1);

	return;
}

void GUI_SEND_reg_all()
{
	cqic_cmd1_type cmd1;
	cqic_cmd2_type cmd2;
	uint16_t rx_data[2]={0,0};

	print("\r\n");
	// CMD_1
	print(CMD_CQIC_CMD1_READ_REGISTER_ALL);
	for(int i=0; i<NUM_OF_REGISTER_CMD_1 + 1; i++)
	{
		// cmd2.u.addr 	= i;
		// cmd2.u.rw		= SPI_RD;
		// cmd2.u.cmd_id	= CMD_01;
		// cmd2.u.data 	= 0;
		// cqic_read(CQIC_1, cmd2.value, rx_data, 2);

		cmd1.u.addr 	= i;
		cmd1.u.cmd_id 	= CMD_01;
		cmd1.u.rw		= SPI_RD;	
		cmd1.u.data		= 0;
		cqic_read(CQIC_1, &cmd1.value, rx_data, 2);

		snprintf(msg_buffer, sizeof msg_buffer, ":%02x:%04x", i, rx_data[1]);
		print(msg_buffer);
	}
    print("\r\n");

	// CMD_2
	print(CMD_CQIC_CMD2_READ_REGISTER_ALL);
	for(int i=0; i<NUM_OF_REGISTER_CMD_2 + 1; i++)
	{
        if(REGISTER_USED_MAP_CQ24[i])
        {

			cmd2.u.addr 	= i;
			cmd2.u.rw		= SPI_RD;
			cmd2.u.cmd_id	= CMD_02;
			cmd2.u.data 	= 0;
			cqic_read(CQIC_1, cmd2.value, rx_data, 2);
			snprintf(msg_buffer, sizeof msg_buffer, ":%02x:%04x", i, rx_data[1]);
			print(msg_buffer);
        }
        else{
            snprintf(msg_buffer, sizeof msg_buffer, ":%02x:NA",i);
            print(msg_buffer);
        }
	}
    print("\r\n");
}


void GUI_write_reg_cmd1(uint16_t in_addr, uint16_t in_val)
{
	cqic_cmd1_type cmd1;

	snprintf(msg_buffer, sizeof msg_buffer, "GUI_write_reg_cmd1(0x%x,0x%x)\r\n", in_addr, in_val);
	print(msg_buffer);

	cmd1.u.addr 	= in_addr;
	cmd1.u.cmd_id 	= CMD_01;
	cmd1.u.rw		= SPI_WR;	
	cmd1.u.data		= in_val;
	cqic_write(CQIC_1, &cmd1.value, 1);
}

void GUI_write_reg_cmd2(uint16_t in_addr, uint16_t in_val)
{
	cqic_cmd2_type cmd2;

	snprintf(msg_buffer, sizeof msg_buffer, "GUI_write_reg_cmd2(0x%x,0x%x)\r\n", in_addr, in_val);
	print(msg_buffer);

	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.rw		= SPI_WR;
	cmd2.u.addr 	= in_addr;
	cmd2.u.data 	= in_val;
	cqic_write(CQIC_1, cmd2.value, 2);
}

void GUI_SEND_otp_written()
{
	cqic_cmd2_type cmd2;
	uint16_t rx_data[2]={0,0};

	print("\r\n");
    print(CMD_CQIC_OTP_WRITTEN);

	for(int i=0; i< (END_OF_OTP_REGISTER-START_OF_OTP_REGISTER + 1); i++)
	{
        if(REGISTER_USED_MAP_CQ24[i + START_OF_OTP_REGISTER])
        {

			cmd2.u.addr 	= i + START_OF_OTP_REGISTER;
			cmd2.u.rw		= SPI_RD;
			cmd2.u.cmd_id	= CMD_02;
			cmd2.u.data 	= 0;
			cqic_read(CQIC_1, cmd2.value, rx_data, 2);
			snprintf(msg_buffer, sizeof msg_buffer, ":%02x:%04x", i + START_OF_OTP_REGISTER, rx_data[1]);
			print(msg_buffer);
        }
        else{
            snprintf(msg_buffer, sizeof msg_buffer, ":%02x:NA",i);
            print(msg_buffer);
        }
	}
    print("\r\n");
}

void cqic_reg_read_all(void)
{
	cqic_cmd1_type cmd1;
	cqic_cmd2_type cmd2;
	uint16_t rx_data[2] = {0, };

	print("\r\n");
	// CMD_1
	print("CQ24 CMD1 REGISTERS\r\n");
	for(int i = 0 ; i < (NUM_OF_REGISTER_CMD_1 + 1) ; ++i)
	{
		cmd1.u.addr 	= i;
		cmd1.u.cmd_id 	= CMD_01;
		cmd1.u.rw		= SPI_RD;	
		cmd1.u.data		= 0;
		cqic_read(CQIC_1, &cmd1.value, rx_data, 2);

		snprintf(msg_buffer, sizeof msg_buffer, "[0x%02X] - [0x%03X]\r\n", i, rx_data[1]);
		print(msg_buffer);
	}
    print("\r\n");

	// CMD_2
	print("CQ24 CMD2 REGISTERS\r\n");
	for(int i = 0 ; i < (NUM_OF_REGISTER_CMD_2 + 1) ; ++i)
	{
        if(REGISTER_USED_MAP_CQ24[i])
        {
			cmd2.u.addr 	= i;
			cmd2.u.rw		= SPI_RD;
			cmd2.u.cmd_id	= CMD_02;
			cmd2.u.data 	= 0;
			cqic_read(CQIC_1, cmd2.value, rx_data, 2);
			
			snprintf(msg_buffer, sizeof msg_buffer, "[0x%02X] - [0x%04X]\r\n", i, rx_data[1]);
			print(msg_buffer);
        }
        else
		{
            snprintf(msg_buffer, sizeof msg_buffer, "[0x%02X] - [NONE]\r\n", i);
            print(msg_buffer);
        }
	}
    print("\r\n");
}
