/** @file amic_test.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#include "main.h"
#include "log.h"
#include "cqic_test.h"
#include "user_flash.h"
#include "fnd.h"
#include "ADS124S08.h"
#include "trimming.h"
#include "cqic.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim12;


uint16_t gn_auto_test_count;
extern uint16_t gn_auto_test_ok_count;
extern uint16_t gn_auto_test_ng_count;

test_mode_t gt_jig_test_mode;
test_mode_t gt_jig_manual_test_mode;
extern auto_manual_t gt_auto_manual_state;

extern cqic_test_step_t gt_jig_trimming_step;
uint16_t gt_cqic_test_step;

extern uint8_t gn_cqic_test_type;      /* 0 : normal test, 1 : 1 : trimming */

uint8_t gb_adc_start_dma;
uint16_t gn_adc_dma_buffer[4];
uint32_t gn_mode_temp_adc[AMIC_O_MAX];
uint16_t fill_data_buf[2048];

int16_t gn_adc_read_count_adc1;
int16_t gn_adc_read_count_adc2;
uint16_t ext_mode=0;


uint16_t gn_test_step_delay;
uint32_t gn_test_tickcount;

uint8_t gb_adc_drdy_flag1, gb_adc_drdy_flag2;
uint8_t gb_ads114s08_drdy_done;
int32_t gn_ads114s08_adc_temp[CH_MAX];
int32_t gn_ads114s08_adc_temp_l[CH_MAX];

float cqic_trim_result[CH_MAX][3];
uint16_t gt_jig_check_vsync;

uint8_t duty = 5;	/* 60% */	
uint8_t num_of_ch = CH_MAX;
uint8_t num_of_line = 14;
volatile uint8_t gt_jig_test_loop = 10;

/* save result value for OTP write */
uint16_t cqic_trimming_value[CH_MAX][2];	/* [CH][0] : 10bit DAC, [CH][1] : 12bit result */
uint16_t cqic_Vref_offset_value;		/* DAC1.5V trimming vaule */
uint16_t cqic_vmode_value;
uint16_t cqic_otp_checksum_value;		/* check value for otp write */

uint8_t cqic_test_mode;
extern uint16_t dac_1_5V_adc_count;
extern uint8_t adc_ch;
extern uint8_t min_max;
uint16_t int_cnt=0;
extern uint8_t gt_trim_retry_cnt;

static uint16_t cqic_function_test_tbl[29][6] = 
{
	/* [Reg#][6] = {s1x6, s2x3, s3x2, d1x6, d2x3, d3x2} */ 

	/*0x00 FRAME_SIZE*/						{0x0E80, 0,0,0,0,0},
	/*0x01 FRAME_START_TIME*/			{0x0001, 0,0,0,0,0},	
	/*0x02 CH_SIZE_SVSYNC_DELAY*/	{0x1209, 0,0,0,0,0},
	
	/*0x03 SVSYNC_OFFSET*/				{0x0000, 0,0,0,0,0},
	/*0x04 SVSYNC_PERIOD*/				{0x041A, 0,0,0,0,0},
	/*0x05 SHSYNC_PERIOD*/				{0x004B, /*0x0050,*/ 0,0,0,0,0},

	/*0x06 HOLD_SIZE_R1*/ 				{0x3434, 0,0,0,0,0},
	/*0x07 HOLD_SIZE_R2*/ 				{0x0000, 0,0,0,0,0},
	/*0x08 HOLD_SIZE_R3*/ 				{0x0000, 0,0,0,0,0},

	/*0x09 HOLD_DUMMY_SIZE_S1*/		{0x0208, 0,0,0,0,0},
	/*0x0A HOLD_DUMMY_SIZE_S2*/		{0x0000, 0,0,0,0,0},

	/*0x0B HOLD_SETUP*/						{0x002C,/*0x0018,*/ 0,0,0,0,0},
	/*0x0C HOLD_ON*/							{0x001D, 0,0,0,0,0},

	/*0x0D X_PERIOD,*/						{0x0010, 0,0,0,0,0},
	/*0x0E VSYNC_IN_COMP1*/ 			{0x0000,/*0x0000,*/ 0,0,0,0,0},
	/*0x0F VSYNC_IN_COMP2*/ 			{0x0000,/*0x0000,*/ 0,0,0,0,0},
	
	/*0x10 FRAME_HD_FIX */				{0x0000, 0,0,0,0,0},
	/*0x11 DUTY_FIX*/							{0x0000, 0,0,0,0,0},
	/*0x12 LD_I_FIX*/							{0x0000, /*0x0824,*/ 0,0,0,0,0},

	/*0x13 STB_DUP_EN*/						{0x00F0, 0,0,0,0,0},
	/*0x14 STB_DUP_CH_R1*/				{0x0000, 0,0,0,0,0},
	/*0x15 STB_DUP_CH_R2*/				{0x4321, 0,0,0,0,0},
	/*0x16 STB_DUP_CH_R3*/				{0x0000, 0,0,0,0,0},

	/*0x17 FULL_STEP_SIZE*/				{0x0FFF, 0,0,0,0,0},
	/*0x18 FULL_STEP_DT_TH*/			{/*0x0000*/0x80, 0,0,0,0,0},

	/*0x19 OUTPUT_CH_EN1,*/				{0xFFFF, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff},	/* all channel-24ch- enable */
	/*0x1A OUTPUT_CH_EN2,*/				{0x0003, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff},

	/*0x1B MCLK_LOCK_CNT_LSB,*/		{0x0D00, 0x49f0, 0x49f0, 0x49f0, 0x49f0, 0x49f0},	/* mclk 16.1280MHz, Vsync-In 120Hz */
	/*0X1C MCLK_LOCK_CNT_MSB,*/		{0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002}
};


void cqic_set_cmd2_all_reg(uint8_t mode);
void fill_frame_data(uint8_t duty, uint8_t num_of_line, uint8_t num_of_ch);
void HAL_GPIO_DRDY1_Callback(uint16_t GPIO_Pin);
void HAL_GPIO_DRDY2_Callback(uint16_t GPIO_Pin);
void set_ok_led(uint8_t en);
void set_ng_led(uint8_t en);
void set_trimming_led(uint8_t en);

extern void PlayBeep(beep_freq_t t_type, uint16_t n_time);
extern void dac_set_voltage(float f_voltage, uint8_t ch);
extern void cqic_write(uint8_t device_id, uint16_t* pData, uint16_t length);
extern void cqic_read(uint8_t device_id, uint16_t* cmd, uint16_t* pData, uint16_t length);
extern void aqic_mode_config(uint8_t mode);
extern void cqic_init(void);
extern float cqic_adc_to_voltage(float cnt, uint8_t source);


void set_ok_led(uint8_t en)
{
  return;
#if 0  
  if(en == ON)
    {
        HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_RESET);
    }
#endif  
}

void set_ng_led(uint8_t en)
{
    return;

#if 0    
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_NG_GPIO_Port, LED_NG_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_NG_GPIO_Port, LED_NG_Pin, GPIO_PIN_RESET);
    }
#endif    
}

void set_trimming_led(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_TRIM_GPIO_Port, LED_TRIM_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_TRIM_GPIO_Port, LED_TRIM_Pin, GPIO_PIN_RESET);
    }
}

void cqic_set_cmd2_all_reg(uint8_t mode)
{
	uint8_t addr;
	cqic_cmd2_type cmd2;

	cmd2.value32 = 0;
	cmd2.u.cmd_id = CMD_02;
	cmd2.u.rw	  = SPI_WR;

	for(addr = 0; addr < 0x1C; addr++)
	{
		cmd2.u.addr   = addr; 
		cmd2.u.data   = cqic_function_test_tbl[addr][0];
		cqic_write(CQIC_1, cmd2.value, 2);
	}

	return;
}

#define DAISY_TEST
//uint16_t test_vref_tbl[4] = {1000,2000,3000,4095};
uint16_t test_vref = 4095;

void fill_frame_data(uint8_t duty, uint8_t num_of_line, uint8_t num_of_ch)
{
	cqic_cmd3_header_type cmd3_header;
	cqic_cmd3_duty_type   cmd3_duty;
	cqic_cmd4_ldi_type    cmd4_ldi;
	uint16_t ch, line, z=0;

  cmd3_header.value = 0;
  cmd3_duty.value = 0;
  cmd4_ldi.value = 0;
        
	/* frame header for DEVICE A */
	cmd3_header.u.cmd_id 	= CMD_03;

	cmd3_header.u.cz	= 0;	//ext_mode;
	cmd3_header.u.vref	= 4095;	//481;	//test_vref_tbl[ext_mode];

	fill_data_buf[z++] = cmd3_header.value;

	/* duty data for device A */
	cmd3_duty.u.duty_n1 = 64;	//duty;
	cmd3_duty.u.duty_n	= 64;	//duty;

	for(line=0; line < (num_of_line*12); line++ )
	{
		fill_data_buf[z++] = cmd3_duty.value;
	}

#ifdef	DAISY_TEST
	/* frame header for device B */
	cmd3_header.u.cmd_id 	= CMD_03;
	cmd3_header.u.cz	= 0	;	//ext_mode;
	cmd3_header.u.vref	= 4095;	//test_vref_tbl[ext_mode];
	fill_data_buf[z++] = cmd3_header.value;

	/* duty data for device B */
	cmd3_duty.u.duty_n1 = 5;	//duty;
	cmd3_duty.u.duty_n	= 5;	//duty;

	for(line=0; line < (num_of_line*12); line++ )
	{
		if(line % 2 == 0)
			fill_data_buf[z++] = cmd3_duty.value;
		else	fill_data_buf[z++] = 0; 
	}
#endif

	cqic_write(CQIC_1, fill_data_buf, z);

	z = 0;
	cmd4_ldi.u.cmd_id 	= CMD_04;

	/* ld_i data for device A */
	for(line=0; line<num_of_line; line++ )
	{
		for(ch=0; ch<num_of_ch;ch++)
		{
			cmd4_ldi.u.ld_i		= 3000;	//4095;	//test_vref;
			fill_data_buf[z++] = cmd4_ldi.value;
		}
	}

#ifdef DAISY_TEST
	/* ld_i data for Device B */
	//for(line=0; line<(num_of_line*CH_MAX); line++ )
	for(line=0; line<num_of_line; line++ )
	{
		for(ch=0; ch<num_of_ch;ch++)
		{
			if(line%2 == 0)
				cmd4_ldi.u.ld_i		= 0x555;
			else
				cmd4_ldi.u.ld_i		= 0x0;

			fill_data_buf[z++] = cmd4_ldi.value;
		}
	}
#endif

	cqic_write(CQIC_1, fill_data_buf, z);

	return;
}

//uint8_t test_duty = 64;
uint8_t test_duty = 2;


void test_procedure_run(void)
{
	cqic_cmd1_type cmd1;
	cqic_cmd2_type cmd2;

	switch(gt_cqic_test_step)
	{
	case MODE_TEST_START:
		print("\n\r Test Mode Start\n\r");
		print("++++++++++++++++ Start CQIC Test procedure +++++++++++++++\n\r");

		print("CQIC Initialize...\n\r");
		cqic_init();

		set_ok_led(OFF);
		set_ng_led(OFF);
		set_trimming_led(ON);		

		//print("Start Vsync timer\n\r");
		//HAL_TIM_Base_Start_IT(&htim2);	/* v_sync_in start */

		gt_cqic_test_step = MODE_TEST_STEP00_00;
		break;
	case MODE_TEST_STEP00_00:
		print("++++ test mode step 0-0\n\r");
		/* set command type 1 reg */
		cmd1.u.cmd_id	= CMD_01;
		cmd1.u.addr 	= MODE_CONTROL;
		cmd1.u.rw		= SPI_WR;
		cmd1.u.data 	= 0x0090;		/* EXT CLK:1, DRV_MODE:1 */
		cqic_write(CQIC_1, &cmd1.value, 1);


		cmd1.u.cmd_id	= CMD_01;
		cmd1.u.addr 	= FIX_CONTROL;
		cmd1.u.rw		= SPI_WR;
		cmd1.u.data 	= 0x0000;
		cqic_write(CQIC_1, &cmd1.value, 1);

		HAL_Delay(3);

		cmd1.u.cmd_id	= CMD_01;
		cmd1.u.addr 	= FIX_CONTROL;
		cmd1.u.rw		= SPI_WR;
		cmd1.u.data 	= 0x400;		//0x0400;
		cqic_write(CQIC_1, &cmd1.value, 1);

		cmd1.u.cmd_id	= CMD_01;
		cmd1.u.addr 	= CS_HOLD_FIX;
		cmd1.u.rw		= SPI_WR;
		cmd1.u.data 	= 0x00FF;
		cqic_write(CQIC_1, &cmd1.value, 1);

    /* configuration aqic mode */
    print("AQIC mode configuraltion\n\r");
    //aqic_mode_config(ENTERY);
		aqic_mode_config(M3X2);
		
		/* set command type 2 reg */
		print("CQIC parameter setting...\n\r");
		cqic_set_cmd2_all_reg(cqic_test_mode);

#if 0
		cmd1.u.cmd_id	= CMD_01;
		cmd1.u.addr 	= FIX_CONTROL;
		cmd1.u.rw		= SPI_WR;
		cmd1.u.data 	= 0x470;		//0x0400;
		cqic_write(CQIC_1, &cmd1.value, 1);

		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.addr 	= FRAME_HEADER;	//LD_I;
		cmd2.u.data 	= 4095;	//ld_i_data;
		cqic_write(CQIC_1, cmd2.value, 2);	/* set frame header & DAC 10bit(gain) */ 
		HAL_Delay(10);

		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.addr 	= DUTY;	//LD_I;
		cmd2.u.data 	= 80;	//ld_i_data;
		cqic_write(CQIC_1, cmd2.value, 2);	/* set frame header & DAC 10bit(gain) */ 
		HAL_Delay(10);

		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.addr 	= LD_I;	//LD_I;
		cmd2.u.data 	= 1024;	//ld_i_data;
		cqic_write(CQIC_1, cmd2.value, 2);	/* set frame header & DAC 10bit(gain) */ 
		HAL_Delay(10);

		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.addr 	= FULL_STEP_DUTY;	//LD_I;
		cmd2.u.data 	= 2;	//ld_i_data;
		cqic_write(CQIC_1, cmd2.value, 2);	/* set frame header & DAC 10bit(gain) */ 
		HAL_Delay(10);

		while(1){};
#endif

		/* got to next step */
		gt_cqic_test_step = MODE_TEST_STEP00_01;
		break;
	case MODE_TEST_STEP00_01:
		print("++++ test mode step 0-1\n\r");
		/* fill 1 frame data */
		print("fill frame data\n\r");

		ext_mode = 3;
		//fill_frame_data(0x40,num_of_line, num_of_ch);
		fill_frame_data(test_duty,num_of_line, num_of_ch);
		snprintf(msg_buffer, sizeof msg_buffer, "Start Vsync timer(%d)\n\r", ext_mode);
		print(msg_buffer);

		gt_jig_test_loop = 30;
		gt_jig_check_vsync = 0;

		HAL_TIM_Base_Start_IT(&htim2);	/* v_sync_in start */

		gt_cqic_test_step = MODE_TEST_STEP00_02;		
		break;
	case MODE_TEST_STEP00_02:
		print("++++ test mode step 0-2\n\r");

		/* set external interrupt */
		//gt_jig_check_cs = 0;
		//gt_jig_check_hold = 0;

		gt_cqic_test_step = MODE_TEST_STEP01_00;
		break;
	case MODE_TEST_STEP01_00:
		print("++++ test mode step 1-0\n\r");
		/* check vsync int */
		if(gt_jig_check_vsync)
		{
			gt_jig_check_vsync = 0;

			//fill_frame_data(0x40, num_of_line, num_of_ch);
			fill_frame_data(test_duty, num_of_line, num_of_ch);
			snprintf(msg_buffer, sizeof msg_buffer, "Gen Vsync Interrupt flag & and fill next frame data(%d)\n\r", ext_mode);
			print(msg_buffer);

			//if(--gt_jig_test_loop == 0)
			if(gt_jig_test_loop == 0)	/* for endless test */
			{
				HAL_TIM_Base_Stop_IT(&htim2);
				gt_cqic_test_step = MODE_TEST_RESULT;
				print("++++ End of Test\n\r");
			}
			else
			{
				gt_cqic_test_step = MODE_TEST_STEP01_00;
				//snprintf(msg_buffer, sizeof msg_buffer, "Countinuos Test[%u] -- got to test step 1-0\n\r ", gt_jig_test_loop );
				//print(msg_buffer);
			}
		}
		break;
	case MODE_TEST_STEP01_01:
		{
			print("======== CQIC MODE TEST START ========\r\n");
		
			gn_mode_temp_adc[CQIC_MODE_1] = 0;
			gn_mode_temp_adc[CQIC_MODE_2] = 0;
			gb_adc_start_dma = 0;
			gn_adc_read_count_adc1 = ADC_READ_COUNT;
			gt_trim_retry_cnt = 0;
		
			cmd1.u.cmd_id	= CMD_01;	
			cmd1.u.addr 	= FIX_CONTROL;
			cmd1.u.rw		= SPI_WR;
			cmd1.u.data 	= 0x170;
			cqic_write(CQIC_1, &cmd1.value, 1); /* set LD_FORCE bit & start analog data out */
			
			cmd2.u.cmd_id	= CMD_02;
			cmd2.u.rw		= SPI_WR;
			cmd2.u.addr 	= FRAME_HEADER;
			cmd2.u.data 	= (0<<10);
			cqic_write(CQIC_1, cmd2.value, 2);	/* set ANA DAC 10bit */
			HAL_Delay(30);	
		
			gt_cqic_test_step = TRIMMING_STEP01_02; 							
		}
		break;
	case MODE_TEST_STEP01_02:
		/* internal adc init & start */
		if(gb_adc_start_dma == 0)
		{
			/* get adc data up to read count */
			if(gn_adc_read_count_adc1)
			{
				gb_adc_start_dma = 1;
				--gn_adc_read_count_adc1;
		
				HAL_Delay(3);
				
				if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)gn_adc_dma_buffer, 3) != HAL_OK)
				{
					print("\n\r error HAL_ADC_Start_DMA");
					gt_cqic_test_step = TEST_STEP_NONE;
					return;
				}
			}
			else
			{
				HAL_ADC_Stop_DMA(&hadc1);
				gt_cqic_test_step = TRIMMING_STEP01_03;
			}
		}			
		break;
	case MODE_TEST_STEP01_03:
		if(gb_adc_start_dma == 0)
		{
			float adc_count1 = (float)gn_mode_temp_adc[CQIC_MODE_1]/ADC_READ_COUNT;
			float adc_count2 = (float)gn_mode_temp_adc[CQIC_MODE_2]/ADC_READ_COUNT;
			float vref1 = cqic_adc_to_voltage(adc_count1, 0);
			float vref2 = cqic_adc_to_voltage(adc_count2, 0);
			
			gn_mode_temp_adc[CQIC_MODE_1] = gn_mode_temp_adc[CQIC_MODE_2] = 0;
			gt_trim_retry_cnt++;

			if(gt_trim_retry_cnt > 30)
			{
				cmd2.u.cmd_id	= CMD_02;
				cmd2.u.rw		= SPI_WR;
				cmd2.u.addr 	= FRAME_HEADER;
				cmd2.u.data 	= (3<<10);
				cqic_write(CQIC_1, cmd2.value, 2);	/* set ANA DAC 10bit */
				HAL_Delay(30);	

				snprintf(msg_buffer, sizeof msg_buffer, "\n\r MODE[3], MODE_1[%f], MODE_2[%f]!!", vref1, vref2);
				print(msg_buffer);
			}
			else if(gt_trim_retry_cnt > 20)
			{
				cmd2.u.cmd_id	= CMD_02;
				cmd2.u.rw		= SPI_WR;
				cmd2.u.addr 	= FRAME_HEADER;
				cmd2.u.data 	= (2<<10);
				cqic_write(CQIC_1, cmd2.value, 2);	/* set ANA DAC 10bit */
				HAL_Delay(30);	
				
				snprintf(msg_buffer, sizeof msg_buffer, "\n\r MODE[2], MODE_1[%f], MODE_2[%f]!!", vref1, vref2);
				print(msg_buffer);
			}
			else if(gt_trim_retry_cnt > 10)
			{
				cmd2.u.cmd_id	= CMD_02;
				cmd2.u.rw		= SPI_WR;
				cmd2.u.addr 	= FRAME_HEADER;
				cmd2.u.data 	= (1<<10);
				cqic_write(CQIC_1, cmd2.value, 2);	/* set ANA DAC 10bit */
				HAL_Delay(30);	

				snprintf(msg_buffer, sizeof msg_buffer, "\n\r MODE[1], MODE_1[%f], MODE_2[%f]!!", vref1, vref2);
				print(msg_buffer);
			}
			else
			{
				cmd2.u.cmd_id	= CMD_02;
				cmd2.u.rw		= SPI_WR;
				cmd2.u.addr 	= FRAME_HEADER;
				cmd2.u.data 	= (0<<10);
				cqic_write(CQIC_1, cmd2.value, 2);	/* set ANA DAC 10bit */
				HAL_Delay(30);	

				snprintf(msg_buffer, sizeof msg_buffer, "\n\r MODE[0], MODE_1[%f], MODE_2[%f]!!", vref1, vref2);
				print(msg_buffer);
			}
				
			if(gt_trim_retry_cnt > 40)
				gt_cqic_test_step = MODE_TEST_OK; 							
		}
	break;
	case MODE_TEST_RESULT:
		/* result display */
		gt_cqic_test_step = MODE_TEST_OK;
		break;
	case MODE_TEST_OK:
		set_ok_led(ON);
		set_ng_led(OFF);
		set_trimming_led(OFF);		

		gt_cqic_test_step = TEST_MODE_NONE;		
		break;
	case  MODE_TEST_ERROR:
		set_ok_led(OFF);
		set_ng_led(ON);
		set_trimming_led(OFF);		

		gt_cqic_test_step = TEST_MODE_NONE;	
		break;
	default:
	break;
	}
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        HAL_ADC_Stop_DMA(hadc);

        gn_mode_temp_adc[CQIC_DAC_1_5V]+= gn_adc_dma_buffer[CQIC_DAC_1_5V];
        gn_mode_temp_adc[CQIC_MODE_1]  += gn_adc_dma_buffer[CQIC_MODE_1];
        gn_mode_temp_adc[CQIC_MODE_2]  += gn_adc_dma_buffer[CQIC_MODE_2];

        //snprintf(msg_buffer, sizeof msg_buffer, "ADC3 HAL_ADC_ConvCpltCallback() %2d : %3d\r\n", gt_jig_test_mode, gn_adc_read_count);
        //print(msg_buffer);

        gb_adc_start_dma = 0;
    }
}

void HAL_GPIO_DRDY1_Callback(uint16_t GPIO_Pin)
{
	if(ADC1_DRDY_Pin)
	{
		int32_t temp = 0;
#if 1
	 	if(gn_adc_read_count_adc1 == 0) return;

		temp = ads114s08_get_data(NULL, NULL, ADC1_CS);
#else
		temp = ads114s08_get_rdata(NULL, NULL, ADC1_CS);
#endif
		if(temp > 32767)
		{
			temp = ~temp + 1;	// Take 2's compliment if number is negative
			snprintf(msg_buffer, sizeof msg_buffer, "\n\r tMinus [%5d]", temp);
			print(msg_buffer);
		}

		if(gn_adc_read_count_adc1 > 0)
		{
			if((min_max == 0x00) || (min_max==0x03))
				gn_ads114s08_adc_temp[adc_ch] += temp;
			else
				gn_ads114s08_adc_temp_l[adc_ch] += temp;
		
			gn_adc_read_count_adc1--;
		}
	
		if(gn_adc_read_count_adc1 == 0)
		{
			ads114s08_set_start(RESET, ADC1_CS);	/* stop continuous conversion */
			gb_adc_drdy_flag1 = 1;
		}
	}
}

void HAL_GPIO_DRDY2_Callback(uint16_t GPIO_Pin)
{
	if(ADC2_DRDY_Pin)
	{
		int32_t temp = 0;
#if 1
		if(gn_adc_read_count_adc2 == 0) return;
		
		temp = ads114s08_get_data(NULL, NULL, ADC2_CS);
#else
		temp = ads114s08_get_rdata(NULL, NULL, ADC1_CS);
#endif
		if(temp > 32767)
		{
			temp = ~temp + 1;	// Take 2's compliment if number is negative
			snprintf(msg_buffer, sizeof msg_buffer, "\n\r tMinus [%5d]", temp);
			print(msg_buffer);
		}

		if(gn_adc_read_count_adc2 > 0)
		{
			if((min_max == 0x00) || (min_max==0x03))
				gn_ads114s08_adc_temp[adc_ch] += temp;
			else
				gn_ads114s08_adc_temp_l[adc_ch] += temp;
		
			gn_adc_read_count_adc2--;
		}

		if(gn_adc_read_count_adc2==0)
		{
			ads114s08_set_start(RESET, ADC2_CS);	/* stop continuous conversion */
			gb_adc_drdy_flag2 = 1;
		}
	}
}

/*** end of file ***/

