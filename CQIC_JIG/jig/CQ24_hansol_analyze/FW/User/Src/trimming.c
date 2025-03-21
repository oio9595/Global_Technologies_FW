/** @file trimming.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __APIC_TRIMMING_C__
#include "main.h"
#include "cmsis_os.h"
#include "log.h"
#include "trimming.h"
#include "fnd.h"
#include "ADS124S08.h"
#include "cqic.h"
#include "math.h"
#include "stdlib.h"

// #define TRIM_MANUAL
#define FULL_TEST
//#define TRIM_BY_COEF
#define VERF_SLOP	0

/* slop test parameter */
#define VREF_0			0
#define VREF_879		879
#define VREF_680		680
#define	VREF_481		481

#define DAC12B_0		0
#define DAC12B_4095		4095

#if VREF_SLOP
#define	VREF_DEFAULT	VREF_0
#define DAC12B_DEFAULT	DAC12B_4095
#else
#define	VREF_DEFAULT	VREF_481
#define DAC12B_DEFAULT	DAC12B_0
#endif

#if 0
/* dac 10bit trimming parameter */
#if 0
#define	Vh_12B			4095
#define	Vl_12B  		450
#define VREF_12B_TRIM 	481
#define VREF_OFFSET_T_DIFF	2.2406f
#else
#define	Vh_12B					3000
#define	Vl_12B  				550
#define VREF_12B_TRIM 	2000
#define VREF_OFFSET_T_DIFF	0.5844f
#endif
#endif

#define DAC_10B_T		2.4898f

#if 0
/* dac 12bit trimming parameter */
#define	VREF_12B_TRIM	2000		
#define DAC_12B_TRIM	550		
#define DAC_12B_T	    0.1312
#endif

#ifdef CHK_TRIM_TIME
TickType_t g_CHK_TRIM_TIME_tick = 0;
TickType_t g_CHK_TRIM_TIME_INIT = 0;
TickType_t g_CHK_TRIM_TIME_MODE = 0;
TickType_t g_CHK_TRIM_TIME_VREF = 0;
TickType_t g_CHK_TRIM_TIME_GAIN = 0;
TickType_t g_CHK_TRIM_TIME_OFFSET = 0;
TickType_t g_CHK_TRIM_TIME_OTP = 0;
#endif

uint16_t VREFO_4_5V_offset_count;
int16_t vref_offset_count[CH_MAX];
int16_t dac_offset_count[CH_MAX];
uint8_t adc_ch=0;
uint16_t dac12b_val=0;
uint16_t Vref_val = 0;
float v_min[CH_MAX];
uint8_t min_max;
uint8_t gt_trim_retry_cnt;
uint16_t aq_mode=0;

uint8_t gub_loop_cnt;
uint8_t gub_saved_cnt;
#define TRIM_REGISTER_SAVED_CNT 10
// Vmode 2.5V Trimming
float gf_pre_vmode_diff = 0;
uint16_t gui_pre_cqic_vmode_value=0;
uint16_t gui_saved_cqic_vmode_value[TRIM_REGISTER_SAVED_CNT];
float gui_saved_vmode_diff[TRIM_REGISTER_SAVED_CNT];

// Vrefo 4.5V Trimming
float gf_pre_vref_diff = 0;
uint16_t gui_pre_cqic_vref_value=0;
uint16_t gui_saved_VREFO_4_5V_offset_count[TRIM_REGISTER_SAVED_CNT];
float gui_saved_VREFO_4_5V_diff[TRIM_REGISTER_SAVED_CNT];

// Vrefo 12V Trimming
float gf_pre_12bvref_diff = 0;
int16_t gi_pre_cqic_12bvref_value=0;
int16_t gi_saved_cqic_12bvref_value[TRIM_REGISTER_SAVED_CNT];
float gui_saved_cqic_12bvref_diff[TRIM_REGISTER_SAVED_CNT];

// Offset 12V Trimming
float gf_pre_12boffset_diff[24];
int16_t gi_pre_cqic_12boffset_value[24];
float gf_saved_12boffset_diff[24][TRIM_REGISTER_SAVED_CNT];
int16_t gi_saved_cqic_12boffset_value[24][TRIM_REGISTER_SAVED_CNT];
uint8_t gub_off12v_loop_cnt[24];
uint8_t gub_off12v_saved_cnt[24];

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim4;

extern uint16_t cqic_trimming_value[CH_MAX][2];	/* [CH][0] : 10bit DAC, [CH][1] : 12bit result */
extern uint8_t gb_adc_drdy_flag1, gb_adc_drdy_flag2;
extern char msg_buffer[256];
extern uint8_t gn_cqic_test_type;      /* 0 : normal test, 1 : 1 : trimming */
extern uint8_t gb_ads114s08_drdy_done;
extern int32_t gn_ads114s08_adc_temp[CH_MAX];
extern int32_t gn_ads114s08_adc_temp_l[CH_MAX];
extern int16_t gn_adc_read_count_adc1;
extern int16_t gn_adc_read_count_adc2;
extern uint8_t gb_adc_start_dma;
extern uint16_t gn_adc_dma_buffer[4];
extern uint32_t gn_mode_temp_adc[AMIC_O_MAX];
extern test_mode_t gt_jig_test_mode;
extern float cqic_trim_result[24][3];
extern uint16_t cqic_Vref_offset_value;		/* DAC1.5V trimming vaule */
extern uint16_t cqic_vmode_value;
extern uint16_t gt_cqic_test_step;

void gt_get_dac10b_adc_data(uint8_t ch);
void gt_adc_start_it(uint8_t ch);
void cqic_12b_VREF_offset_trimming(void);
void cqic_12b_DAC_offset_trimming(uint8_t ch);
float cqic_adc_to_voltage(float cnt, uint8_t source);


extern void cqic_read(uint8_t device_id, uint16_t* cmd, uint16_t* pData, uint16_t length);
extern void cqic_write(uint8_t device_id, uint16_t* pData, uint16_t length);

extern void set_ok_led(uint8_t en);
extern void set_ng_led(uint8_t en);
extern void set_trimming_led(uint8_t en);
extern void PlayBeep(beep_freq_t t_type, uint16_t n_time);

extern void set_cqic_power(uint8_t on);
extern void dac_set_voltage(float f_voltage, uint8_t ch);
extern void cqic_trim_data_to_otp(void);
extern void cqic_trim_reg_verify(void);
extern void cqic_trim_value_verify(void);
extern void cqic_output_disable(void);
extern void cqic_output_enable(uint16_t gain, uint16_t offset);
extern void cqic_mode_out(uint16_t mode);

enum
{
	I_ADC = 0,
	I_ADC_1,
	E_ADC	
};

void apic_test_mode_exit(void)
{
	return;
}

void apic_get_regs_for_trimming(void)
{
	return;
}

uint8_t aqic_set_reg_test1_sel_l_ch(uint8_t sel_l_ch)
{
	return 0;
}

uint8_t aqic_set_reg_test1_config_mode(uint8_t config_mode)
{
	return 0;
}

uint8_t aqic_set_reg_test2_cz_ch1(uint8_t cz_ch1)
{
	return 0;
}

uint8_t aqic_set_reg_test2_cz_ch2(uint8_t cz_ch2)
{
	return 0;
}

uint8_t aqic_set_reg_test3_cz_ch3(uint8_t cz_ch3)
{
	return 0;
}

uint8_t aqic_set_reg_test3_cz_ch4(uint8_t cz_ch4)
{
	return 0;
}

uint8_t aqic_set_reg_test4_cz_ch5(uint8_t cz_ch5)
{
	return 0;
}

uint8_t aqic_set_reg_test4_cz_ch6(uint8_t cz_ch6)
{
	return 0;
}

uint8_t aqic_set_reg_trim1_chkbit(uint8_t chkbit)
{
	return 0;
}

uint8_t aqic_set_reg_trim2_ofs_temp(uint8_t ofs_temp)
{
	return 0;
}

uint8_t aqic_set_reg_trim2_ictl_osc(uint8_t ictl_osc)
{
	return 0;
}

uint8_t aqic_set_reg_trim3_ofs_lin_ch1(uint16_t ofs_lin_ch1)
{
	return 0;
}

uint8_t aqic_set_reg_trim4_ofs_lin_ch2(uint16_t ofs_lin_ch2)
{
	return 0;
}

uint8_t aqic_set_reg_trim5_ofs_lin_ch3(uint16_t ofs_lin_ch3)
{
	return 0;
}

uint8_t aqic_set_reg_trim6_ofs_lin_ch4(uint16_t ofs_lin_ch4)
{
	return 0;
}

uint8_t aqic_set_reg_trim7_ofs_lin_ch5(uint16_t ofs_lin_ch5)
{
	return 0;
}

uint8_t aqic_set_reg_trim8_ofs_lin_ch6(uint16_t ofs_lin_ch6)
{
	return 0;
}

uint8_t aqic_set_reg_trim9_ofs_lin_ch1(uint8_t ofs_lin_ch1)
{
	return 0;
}

uint8_t aqic_set_reg_trim9_hdr_mode(uint8_t hdr_mode)
{
	return 0;
}

uint8_t aqic_set_reg_trim9_ofs_lin_ch2(uint8_t ofs_lin_ch2)
{
	return 0;
}

uint8_t aqic_set_reg_trim10_ofs_lin_ch3(uint8_t ofs_lin_ch3)
{
	return 0;
}

uint8_t aqic_set_reg_trim10_czmax1(uint8_t czmax1)
{
	return 0;
}

uint8_t aqic_set_reg_trim10_ofs_lin_ch4(uint8_t ofs_lin_ch4)
{
	return 0;
}

uint8_t aqic_set_reg_trim11_ofs_lin_ch5(uint8_t ofs_lin_ch5)
{
	return 0;
}

uint8_t aqic_set_reg_trim11_czmax2(uint8_t czmax2)
{
	return 0;
}

uint8_t aqic_set_reg_trim11_ofs_lin_ch6(uint8_t ofs_lin_ch6)
{
	return 0;
}

uint8_t aqic_set_reg_trim12_ictl_ch1(uint8_t ictl_ch1)
{
	return 0;
}

uint8_t aqic_set_reg_trim13_ictl_ch2(uint8_t ictl_ch2)
{
	return 0;
}

uint8_t aqic_set_reg_trim14_ictl_ch3(uint8_t ictl_ch3)
{
	return 0;
}

uint8_t aqic_set_reg_trim15_ictl_ch4(uint8_t ictl_ch4)
{
	return 0;
}

uint8_t aqic_set_reg_trim16_ictl_ch5(uint8_t ictl_ch5)
{
	return 0;
}

uint8_t aqic_set_reg_trim17_ictl_ch6(uint8_t ictl_ch6)
{
	return 0;
}


float cqic_adc_to_voltage(float cnt, uint8_t source)
{
	float ret;
	
	if(source == I_ADC)	
	{
		/* internal adc of mcu */ 
		ret = cnt/4096*3.3;
	}
	else if(source == I_ADC_1)	
	{
		/* internal adc of mcu */ 
		ret = cnt/4096*3.0;
	}
	else
	{
		/* adc124s08 made by TI */
		ret = cnt/32767*5.0;
	}

	return ret;
}

void gt_cqic_test_init(void)
{
	cqic_cmd2_type cmd2;
	
	//set_cqic_power_on();

	cmd2.value32 = 0;
	
	cmd2.u.addr 	= OTP_PROTECTION;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.cmd_id	= CMD_02;
	cmd2.u.data 	= 0xA5A5;
	cqic_write(CQIC_1, cmd2.value, 2);

	//set_ok_led(ON);
	//set_ng_led(OFF);
	//set_trimming_led(ON);

	return;
}

void gt_adc_start_it(uint8_t ch)
{
	gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;	
	gb_ads114s08_drdy_done = 0;
	

	if(ch <= CH_12)
	{
		gn_adc_read_count_adc1 = ADC_READ_COUNT;
		gn_adc_read_count_adc2 = 0;
	
		ads114s08_select_single_ended_input(adc_ch, ADC1_CS);
		HAL_Delay(3);
		ads114s08_set_start(SET, ADC1_CS);
	}
	else
	{
		gn_adc_read_count_adc1 = 0;
		gn_adc_read_count_adc2 = ADC_READ_COUNT;
	
		ads114s08_select_single_ended_input(adc_ch-CH_13, ADC2_CS);
		HAL_Delay(3);
		ads114s08_set_start(SET, ADC2_CS);
	}
}

#if 0
uint16_t ld_i_data = 0xFFF;
uint16_t head_data = 0xFFF;
#endif

void gt_get_dac10b_adc_data(uint8_t ch)
{
	cqic_cmd2_type cmd2;

	cmd2.value32 = 0;

#if 0
	cmd2.u.cmd_id = CMD_02;
	cmd2.u.rw 	= SPI_WR;
	cmd2.u.addr 	= FRAME_HEADER;
	cmd2.u.data 	= head_data;	
	cqic_write(CQIC_1, cmd2.value, 2);	/* set frame header & DAC 10bit(gain) */ 
	HAL_Delay(10);
#endif	

	if(min_max == 0x03)
	{
		//snprintf(msg_buffer, sizeof msg_buffer, "\n\r gt_set_dac10b_start_adc CH[%2d], MAX", ch+1);
		//print(msg_buffer);

		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.addr 	= LD_I;
		cmd2.u.data 	= gui_GAIN_DAC_vHigh;	//ld_i_data;
		cqic_write(CQIC_1, cmd2.value, 2);	/* set frame header & DAC 10bit(gain) */ 
		HAL_Delay(10);
	}
	else if(min_max == 0x01)
	{
		//snprintf(msg_buffer, sizeof msg_buffer, "\n\r gt_set_dac10b_start_adc CH[%2d], MIN", ch+1);
		//print(msg_buffer);

		cmd2.u.cmd_id	= CMD_02;
		cmd2.u.rw		= SPI_WR;
		cmd2.u.addr 	= LD_I;
		cmd2.u.data 	= gui_GAIN_DAC_vLow;   //ld_i_data;	
		cqic_write(CQIC_1, cmd2.value, 2);	/* set frame header & DAC 10bit(gain) */ 
		HAL_Delay(10);
	}

	HAL_Delay(30);
	gt_adc_start_it(adc_ch);
}

// uint8_t gub_off12v_loop_cnt[24];
// uint8_t gub_off12v_saved_cnt[24];
void cqic_12b_DAC_offset_trimming(uint8_t ch)
{
	cqic_dac12b_type dac12b_offset;
	uint16_t adc_trim_data=0,offset;

	uint16_t ui_reg_step = 0;
	float f_12boffset_val_per_reg = 0;


	float temp = (gn_ads114s08_adc_temp[ch]/ADC_READ_COUNT);
	float dac_min = cqic_adc_to_voltage(temp, E_ADC);
	float diff = gf_DATA_DAC_Voltage - dac_min;

	dac12b_offset.value32 = 0;

	if(gub_off12v_loop_cnt[ch] > 30)
	{
		if(gub_off12v_saved_cnt[ch] > 0)
		{
			//find closed to target
			uint8_t temp_closest_cnt = 0;
			for(uint8_t i=0; i < gub_off12v_saved_cnt[ch]; i++ )
			{								
				if( gf_saved_12boffset_diff[ch][i] < gf_saved_12boffset_diff[ch][temp_closest_cnt] )
				{
					temp_closest_cnt = i;
				}
			}

			adc_trim_data = (uint16_t)abs(gi_saved_cqic_12boffset_value[ch][temp_closest_cnt]);
			dac12b_offset.u.addr	 = (DAC_OFFSET_BASE + ch);
			dac12b_offset.u.cmd_id	 = CMD_02;
			dac12b_offset.u.rw		 = SPI_WR;								
			dac12b_offset.u.l_offset = (adc_trim_data & 0x000F);
			dac12b_offset.u.m_offset = ((adc_trim_data >> 4) & 0x00FF);
			if(dac_offset_count[ch] < 0)
			{
				dac12b_offset.u.sign = SET;
			}
			else
			{
				dac12b_offset.u.sign = RESET;
			}
		
			cqic_write(CQIC_1, dac12b_offset.value, 2);
			cqic_trimming_value[ch][DAC_OFFSET] = dac12b_offset.value[1];
		
			cqic_trim_result[ch][0] = 1;	/* set TRIMMING DONE flag */
			cqic_trim_result[ch][1] = dac_min;		
		
			snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d] trim done(%d)! V-min[%f], Diff[%f], T_value[0x%4x]"
			, ch+1, gub_off12v_saved_cnt[ch], cqic_trim_result[ch][1], fabs(gf_DATA_DAC_Voltage-cqic_trim_result[ch][1]),cqic_trimming_value[ch][DAC_OFFSET]);
			print(msg_buffer);
			gub_off12v_loop_cnt[ch] = 0;
			gub_off12v_saved_cnt[ch] = 0;
		}
		else
		{
			print("========== Voffset 12bV Trimming Fail!! =========== \r\n");
			gt_cqic_test_step = TRIMMING_STEP_ERROR;
		}
		return;
	}

	// TRIMING Enhanced Algorithm - Voffset 12bV
	if(gf_pre_12boffset_diff[ch] == 0)
	{
		ui_reg_step = 1;
		gub_off12v_saved_cnt[ch] = 0;
		gub_off12v_loop_cnt[ch] = 0;
	}
	else	
	{
		if((gi_pre_cqic_12boffset_value[ch] - dac_offset_count[ch]) == 0)
		{
			ui_reg_step = 1;
		}
		else
		{
			f_12boffset_val_per_reg = fabs( (gf_pre_12boffset_diff[ch] - diff)/(gi_pre_cqic_12boffset_value[ch] - dac_offset_count[ch]) );
			if(f_12boffset_val_per_reg == 0)
			{
				ui_reg_step = abs(gi_pre_cqic_12boffset_value[ch] - dac_offset_count[ch]);
			}
			else
			{
				ui_reg_step = (uint16_t)(fabs(diff)/f_12boffset_val_per_reg);
			}

			if(ui_reg_step == 0)
			{
				ui_reg_step = 1;
			}

			if(ch == 0) print("\r\n");

			// snprintf(msg_buffer, sizeof msg_buffer, "\r\n %f =( %f - %f )/( %d - %d ) ", 
			// 			f_12boffset_val_per_reg, gf_pre_12boffset_diff[ch], fabs(diff), gi_pre_cqic_12boffset_value[ch], dac_offset_count[ch]);
			// print(msg_buffer);

			// snprintf(msg_buffer, sizeof msg_buffer, "\r\n ui_reg_step : %d = %f / %f", 
			// 			ui_reg_step, fabs(diff), f_12boffset_val_per_reg);
			// print(msg_buffer);
		}
	}

	gf_pre_12boffset_diff[ch] = diff;
	gi_pre_cqic_12boffset_value[ch] = dac_offset_count[ch];
	gub_off12v_loop_cnt[ch]++;

	if(fabs(diff) < gf_DATA_DAC_Step)
	{
		cqic_trim_result[ch][0] = 1;	/* set TRIMMING DONE flag */
		cqic_trim_result[ch][1] = dac_min;		
	
		snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d] trim done! V-min[%f], Diff[%f], T_value[0x%4x]", ch+1, cqic_trim_result[ch][1], fabs(gf_DATA_DAC_Voltage-cqic_trim_result[ch][1]),cqic_trimming_value[ch][DAC_OFFSET]);
		print(msg_buffer);
		gub_off12v_loop_cnt[ch] = 0;
		gub_off12v_saved_cnt[ch] = 0;
	}
	else
	{		

		if(fabs(diff) < gf_DATA_DAC_Step*2)
		{
			gi_saved_cqic_12boffset_value[ch][gub_off12v_saved_cnt[ch]] = dac_offset_count[ch];
			gf_saved_12boffset_diff[ch][gub_off12v_saved_cnt[ch]] = diff;
			gub_off12v_saved_cnt[ch]++;
			snprintf(msg_buffer, sizeof msg_buffer, "\r\ngub_off12v_saved_cnt[%d](%d) \r\n",ch ,gub_off12v_saved_cnt[ch]);
			print(msg_buffer);
		}

		if(gub_off12v_saved_cnt[ch] >= TRIM_REGISTER_SAVED_CNT)
		{
			//find closed to target
			uint8_t temp_closest_cnt = 0;
			for(uint8_t i=0; i < TRIM_REGISTER_SAVED_CNT; i++ )
			{								
				if( gf_saved_12boffset_diff[ch][i] < gf_saved_12boffset_diff[ch][temp_closest_cnt] )
				{
					temp_closest_cnt = i;
				}
			}

			adc_trim_data = (uint16_t)abs(gi_saved_cqic_12boffset_value[ch][temp_closest_cnt]);
			dac12b_offset.u.addr	 = (DAC_OFFSET_BASE + ch);
			dac12b_offset.u.cmd_id	 = CMD_02;
			dac12b_offset.u.rw		 = SPI_WR;								
			dac12b_offset.u.l_offset = (adc_trim_data & 0x000F);
			dac12b_offset.u.m_offset = ((adc_trim_data >> 4) & 0x00FF);			
			if(dac_offset_count[ch] < 0)
			{
				dac12b_offset.u.sign = SET;
			}
			else
			{
				dac12b_offset.u.sign = RESET;
			}
		
			cqic_write(CQIC_1, dac12b_offset.value, 2);
			cqic_trimming_value[ch][DAC_OFFSET] = dac12b_offset.value[1];
		
			cqic_trim_result[ch][0] = 1;	/* set TRIMMING DONE flag */
			cqic_trim_result[ch][1] = dac_min;		
		
			snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d] trim done(%d)! V-min[%f], Diff[%f], T_value[0x%4x]"
			, ch+1, gub_off12v_saved_cnt[ch], cqic_trim_result[ch][1], fabs(gf_DATA_DAC_Voltage-cqic_trim_result[ch][1]),cqic_trimming_value[ch][DAC_OFFSET]);
			print(msg_buffer);
			gub_off12v_loop_cnt[ch] = 0;

			return;
		}

		if(diff > 0)
		{
			if(dac_offset_count[ch] + ui_reg_step > 4095)
			{
				snprintf(msg_buffer, sizeof msg_buffer, "\r\n(dac_offset_count[%d]:0x%x + ui_reg_step:0x%x) is OVER 0xffffffff ", 
							ch, dac_offset_count[ch], ui_reg_step);
				print(msg_buffer);
				dac_offset_count[ch] = 0xfff;
				snprintf(msg_buffer, sizeof msg_buffer, "\r\ndac_offset_count[%d]:0x%x", 
							ch, dac_offset_count[ch]);
				print(msg_buffer);
			}
			else
			{
				dac_offset_count[ch] += ui_reg_step;
			}
		}
		else
		{
			if(dac_offset_count[ch] - ui_reg_step < -4094)
			{
				snprintf(msg_buffer, sizeof msg_buffer, "\r\n(dac_offset_count[%d]:0x%x - ui_reg_step:0x%x) is UNDER 0x00000000", 
							ch, dac_offset_count[ch], ui_reg_step);
				print(msg_buffer);
				dac_offset_count[ch] = 0x00;
				snprintf(msg_buffer, sizeof msg_buffer, "\r\ndac_offset_count[%d]:0x%x", 
							ch, dac_offset_count[ch]);
				print(msg_buffer);
			}
			else
			{
				dac_offset_count[ch] -= ui_reg_step;
			}
		}
		// snprintf(msg_buffer, sizeof msg_buffer, "\r\ngui_pre_cqic_12boffset_value[%d]:0x%x", 
		// 			ch, gi_pre_cqic_12boffset_value[ch]);
		// print(msg_buffer);
		snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d] Trimming... V-DAC[%f], DIFF(%f), OFFSET(%d), T-data(%d), ADJ[%d]!!\r\n", 
					ch+1, dac_min, diff, dac_offset_count[ch],cqic_trimming_value[ch][DAC_OFFSET], ui_reg_step);
		print(msg_buffer);
		
		adc_trim_data = (uint16_t)abs(dac_offset_count[ch]);
		dac12b_offset.u.addr	 = (DAC_OFFSET_BASE + ch);
		dac12b_offset.u.cmd_id	 = CMD_02;
		dac12b_offset.u.rw		 = SPI_WR;								
		dac12b_offset.u.l_offset = (adc_trim_data & 0x000F);
		dac12b_offset.u.m_offset = ((adc_trim_data >> 4) & 0x00FF);
		
		if(dac_offset_count[ch] < 0)
		{
			dac12b_offset.u.sign = SET;
		}
		else
		{
			dac12b_offset.u.sign = RESET;
		}
	
		cqic_write(CQIC_1, dac12b_offset.value, 2);
		cqic_trimming_value[ch][DAC_OFFSET] = dac12b_offset.value[1];
	
		cqic_trim_result[ch][0] = 0;	/* set TRIMMING flag */
		cqic_trim_result[ch][1] = dac_min;
	
	}	
}

void cqic_12b_VREF_offset_trimming_12(uint16_t ch)
{
	float temp, max_10b, diff;
	int16_t diff_cnt;
	uint16_t adc_trim_data;
	cqic_dac10b_type dac10b_offset;

	dac10b_offset.value32 = 0;
	
	temp = (float)gn_ads114s08_adc_temp[ch]/ADC_READ_COUNT;
	max_10b = cqic_adc_to_voltage(temp, E_ADC);
	
	
	diff = DAC_10B_T-max_10b;

	if(fabs(diff) < VREF_OFFSET_STEP_V)
	{
		snprintf(msg_buffer, sizeof msg_buffer, "\n\r CH[%2d] Trimming Done!! ADC_3185[%fV], Diff[%fV], T_value[0x%4x]", ch+1,max_10b, diff,cqic_trimming_value[ch][DAC_VREF]);
		print(msg_buffer);

		cqic_trim_result[ch][0] = 1;	/* set TRIMMING DONE flag */
		cqic_trim_result[ch][1] = max_10b;	

		min_max = 0;
		//gt_cqic_test_step = TRIMMING_STEP02_01;
	}
	else
	{
		if(diff >= 0) diff += VREF_OFFSET_STEP_V/2;
		else	diff -= VREF_OFFSET_STEP_V/2;

		diff_cnt = (int16_t)(diff/VREF_OFFSET_STEP_V);

#if 0
		if(abs(diff_cnt) <= 5 )
		{
			diff_cnt = diff_cnt/abs(diff_cnt);		//1
		}
		else if(abs(diff_cnt) <= 20)
		{
			diff_cnt /= 2;
		}
		else
		{
			diff_cnt = diff_cnt*8/10;
		}
#endif

		vref_offset_count[ch] += diff_cnt;
		
		adc_trim_data = (uint16_t)((abs(vref_offset_count[ch])) & 0x00FF);
		dac10b_offset.u.addr	= (DAC_VREF_OFFSET_BASE + ch);
		dac10b_offset.u.cmd_id	= CMD_02;
		dac10b_offset.u.rw		= SPI_WR;
		dac10b_offset.u.offset	= adc_trim_data;

		if(vref_offset_count[ch] < 0)
		{
			dac10b_offset.u.sign = SET;
		}
		else
		{
			dac10b_offset.u.sign = RESET;
		}

		cqic_write(CQIC_1, dac10b_offset.value, 2);

		cqic_trimming_value[ch][DAC_VREF] = dac10b_offset.value[1];
		
		cqic_trim_result[ch][0] = 0;	/* set TRIMMING DONE flag */
		cqic_trim_result[ch][1] = max_10b;
		
		min_max = 0;
		gn_ads114s08_adc_temp[ch] = 0;
		gn_ads114s08_adc_temp_l[ch] = 0;

		//gt_cqic_test_step = TRIMMING_STEP12_01;

		snprintf(msg_buffer, sizeof msg_buffer, "\n\r Trimming... CH[%2d], ADC_684[%fV], Diff[%fV], count[%d], offset[0x%x], T_data[0x%4x]", ch+1,max_10b, diff,vref_offset_count[ch],adc_trim_data,cqic_trimming_value[ch][DAC_VREF]);
		print(msg_buffer);
	}
}


void cqic_12b_VREF_offset_trimming(void)
{
	float temp, max_10b, min_10b, diff;
	int16_t offset;
	uint16_t adc_trim_data;
	cqic_dac12b_type dac12b_offset;
	uint16_t ui_reg_step = 0;
	float f_12bvref_val_per_reg = 0;

	dac12b_offset.value32 = 0;
	
	temp = (float)gn_ads114s08_adc_temp[adc_ch]/ADC_READ_COUNT;
	max_10b = cqic_adc_to_voltage(temp, E_ADC);
	
	temp = (float)gn_ads114s08_adc_temp_l[adc_ch]/ADC_READ_COUNT;
	min_10b = cqic_adc_to_voltage(temp,E_ADC);
	
	diff = gf_GAIN_DAC_Delta-(max_10b - min_10b);

	if(gub_loop_cnt > 30)
	{
		if(gub_saved_cnt > 0)
		{
			//find closed to target
			uint8_t temp_closest_cnt = 0;
			for(uint8_t i=0; i < gub_saved_cnt; i++ )
			{								
				if( gui_saved_cqic_12bvref_diff[i] < gui_saved_cqic_12bvref_diff[temp_closest_cnt] )
				{
					temp_closest_cnt = i;
				}
			}
			vref_offset_count[adc_ch] = gi_saved_cqic_12bvref_value[temp_closest_cnt];

			adc_trim_data = (uint16_t)(abs(vref_offset_count[adc_ch]));
			dac12b_offset.u.addr	= (DAC_VREF_OFFSET_BASE + adc_ch);
			dac12b_offset.u.cmd_id	= CMD_02;
			dac12b_offset.u.rw		= SPI_WR;
			dac12b_offset.u.l_offset	= (adc_trim_data & 0x000F);
			dac12b_offset.u.m_offset	= ((adc_trim_data >> 4) & 0x00FF);

			if(vref_offset_count[adc_ch] < 0)
			{
				dac12b_offset.u.sign = SET;
			}
			else
			{
				dac12b_offset.u.sign = RESET;
			}

			cqic_write(CQIC_1, dac12b_offset.value, 2);

			cqic_trimming_value[adc_ch][DAC_VREF] = dac12b_offset.value[1];

			snprintf(msg_buffer, sizeof msg_buffer, "\n\r CH[%2d] Trimming Done!! ADC_MAX[%fV], ADC_MIN[%fV], Diff[%fV], T_value[0x%4x]"
				, adc_ch+1,max_10b,min_10b, gui_saved_cqic_12bvref_diff[temp_closest_cnt],cqic_trimming_value[adc_ch][DAC_VREF]);
			print(msg_buffer);

			cqic_trim_result[adc_ch][0] = 1;	/* set TRIMMING DONE flag */
			cqic_trim_result[adc_ch][1] = max_10b;	
			cqic_trim_result[adc_ch][2] = min_10b;	

			gf_pre_12bvref_diff = 0;
			gi_pre_cqic_12bvref_value = 0;

			adc_ch++;
			min_max = 0x03;
			gt_cqic_test_step = TRIMMING_STEP02_01;
			gub_saved_cnt = 0;
			gub_loop_cnt = 0;
		}
		else
		{
			print("========== Vrefo 12bV Trimming Fail!! =========== \r\n");
			gt_cqic_test_step = TRIMMING_STEP_ERROR;
		}
		return;
	}

	if(fabs(diff) < (gf_GAIN_DAC_Step))
	{
		snprintf(msg_buffer, sizeof msg_buffer, "\n\r CH[%2d] Trimming Done!! ADC_MAX[%fV], ADC_MIN[%fV], Diff[%fV], T_value[0x%4x]", adc_ch+1,max_10b,min_10b, diff,cqic_trimming_value[adc_ch][DAC_VREF]);
		print(msg_buffer);

		cqic_trim_result[adc_ch][0] = 1;	/* set TRIMMING DONE flag */
		cqic_trim_result[adc_ch][1] = max_10b;	
		cqic_trim_result[adc_ch][2] = min_10b;	

		gf_pre_12bvref_diff = 0;
		gi_pre_cqic_12bvref_value = 0;

		adc_ch++;
		min_max = 0x03;
		gt_cqic_test_step = TRIMMING_STEP02_01;
		gub_saved_cnt = 0;
		gub_loop_cnt = 0;
	}
	else 
	{
		if(fabs(diff) < (gf_GAIN_DAC_Step*2))
		{
			gi_saved_cqic_12bvref_value[gub_saved_cnt]= VREFO_4_5V_offset_count;
			gui_saved_cqic_12bvref_diff[gub_saved_cnt] = diff;
			gub_saved_cnt++;
			snprintf(msg_buffer, sizeof msg_buffer, "\r\ngub_saved_cnt{%d} \r\n",gub_saved_cnt);
			print(msg_buffer);
		}

		if(gub_saved_cnt >= TRIM_REGISTER_SAVED_CNT)
		{
			//find closed to target
			uint8_t temp_closest_cnt = 0;
			for(uint8_t i=0; i < gub_saved_cnt; i++ )
			{								
				if( gui_saved_cqic_12bvref_diff[i] < gui_saved_cqic_12bvref_diff[temp_closest_cnt] )
				{
					temp_closest_cnt = i;
				}
			}
			vref_offset_count[adc_ch] = gi_saved_cqic_12bvref_value[temp_closest_cnt];
			adc_trim_data = (uint16_t)(abs(vref_offset_count[adc_ch]));
			dac12b_offset.u.addr	= (DAC_VREF_OFFSET_BASE + adc_ch);
			dac12b_offset.u.cmd_id	= CMD_02;
			dac12b_offset.u.rw		= SPI_WR;
			dac12b_offset.u.l_offset	= (adc_trim_data & 0x000F);
			dac12b_offset.u.m_offset	= ((adc_trim_data >> 4) & 0x00FF);

			if(vref_offset_count[adc_ch] < 0)
			{
				dac12b_offset.u.sign = SET;
			}
			else
			{
				dac12b_offset.u.sign = RESET;
			}

			cqic_write(CQIC_1, dac12b_offset.value, 2);

			cqic_trimming_value[adc_ch][DAC_VREF] = dac12b_offset.value[1];

			snprintf(msg_buffer, sizeof msg_buffer, "\n\r CH[%2d] Trimming Done!! ADC_MAX[%fV], ADC_MIN[%fV], Diff[%fV], T_value[0x%4x]"
				, adc_ch+1,max_10b,min_10b, gui_saved_cqic_12bvref_diff[temp_closest_cnt],cqic_trimming_value[adc_ch][DAC_VREF]);
			print(msg_buffer);

			cqic_trim_result[adc_ch][0] = 1;	/* set TRIMMING DONE flag */
			cqic_trim_result[adc_ch][1] = max_10b;	
			cqic_trim_result[adc_ch][2] = min_10b;	

			gf_pre_12bvref_diff = 0;
			gi_pre_cqic_12bvref_value = 0;

			adc_ch++;
			min_max = 0x03;
			gt_cqic_test_step = TRIMMING_STEP02_01;
			gub_saved_cnt = 0;
			gub_loop_cnt = 0;
			return;
		}

		// TRIMING Enhanced Algorithm - Vrefo 12bV Gain
		if(gf_pre_12bvref_diff == 0)
		{
			ui_reg_step = 1;
			gub_saved_cnt = 0;
			gub_loop_cnt = 0;
			// print("========== Vrefo 12bV Trimming START!! =========== \r\n");
		}
		else
		{
			f_12bvref_val_per_reg = fabs( (gf_pre_12bvref_diff - diff)/(gi_pre_cqic_12bvref_value-vref_offset_count[adc_ch]) );
			if(f_12bvref_val_per_reg == 0)
			{
				ui_reg_step = abs(gi_pre_cqic_12bvref_value - vref_offset_count[adc_ch]);
			}
			else
			{
				ui_reg_step = (uint16_t)(fabs(diff)/f_12bvref_val_per_reg);
				if(ui_reg_step == 0)
				{
					ui_reg_step = 1;
				}
			}
		}
		gf_pre_12bvref_diff = diff;
		gi_pre_cqic_12bvref_value = vref_offset_count[adc_ch];
		gub_loop_cnt++;

		snprintf(msg_buffer, sizeof msg_buffer, "\n\r Trimming... CH[%2d], ADC_MAX[%fV], ADC_MIN[%fV], Diff[%fV], offset[%d][0x%x], T_data[0x%4x], ADJ[%d]", 
					adc_ch+1,max_10b,min_10b, diff,vref_offset_count[adc_ch],vref_offset_count[adc_ch],cqic_trimming_value[adc_ch][DAC_VREF],ui_reg_step);
		print(msg_buffer);
		
		if(diff > 0)
		{
			if(vref_offset_count[adc_ch] + ui_reg_step > 4095)
			{
				snprintf(msg_buffer, sizeof msg_buffer, "(vref_offset_count[%d]:0x%x + ui_reg_step:0x%x) is OVER 0xffffffff\n\r ", 
							adc_ch, vref_offset_count[adc_ch], ui_reg_step);
				print(msg_buffer);
				if(vref_offset_count[adc_ch]  < 0x0fff - 1)
				{
					vref_offset_count[adc_ch]++; 
				}
			}
			else
			{
				vref_offset_count[adc_ch] += ui_reg_step;
			}			
		}
		else
		{
			if(vref_offset_count[adc_ch] - ui_reg_step < -4094)
			{
				snprintf(msg_buffer, sizeof msg_buffer, "(vref_offset_count[%d]:0x%x - ui_reg_step:0x%x) is UNDER -4094\n\r ", 
							adc_ch, vref_offset_count[adc_ch], ui_reg_step);
				print(msg_buffer);
				if(vref_offset_count[adc_ch]  > -4093)
				{
					vref_offset_count[adc_ch]--; 
				}
			}
			else
			{
				vref_offset_count[adc_ch] -= ui_reg_step;
			}
		}
		adc_trim_data = (uint16_t)(abs(vref_offset_count[adc_ch]));
		dac12b_offset.u.addr	= (DAC_VREF_OFFSET_BASE + adc_ch);
		dac12b_offset.u.cmd_id	= CMD_02;
		dac12b_offset.u.rw		= SPI_WR;
		dac12b_offset.u.l_offset	= (adc_trim_data & 0x000F);
		dac12b_offset.u.m_offset	= ((adc_trim_data >> 4) & 0x00FF);

		if(vref_offset_count[adc_ch] < 0)
		{
			dac12b_offset.u.sign = SET;
		}
		else
		{
			dac12b_offset.u.sign = RESET;
		}

		cqic_write(CQIC_1, dac12b_offset.value, 2);

		cqic_trimming_value[adc_ch][DAC_VREF] = dac12b_offset.value[1];
		
		cqic_trim_result[adc_ch][0] = 0;	/* set TRIMMING DONE flag */
		cqic_trim_result[adc_ch][1] = max_10b;
		cqic_trim_result[adc_ch][2] = min_10b;	
		
		min_max = 0x03;
		gn_ads114s08_adc_temp[adc_ch] = 0;
		gn_ads114s08_adc_temp_l[adc_ch] = 0;

		gt_cqic_test_step = TRIMMING_STEP02_01;
	}	
}




void cqic_mode_trim(float diff, uint16_t vmode_reg_step)
{
	cqic_cmd2_type cmd2 = {0, };

	if((diff < 0) && (cqic_vmode_value - vmode_reg_step > 0)) 
	{
		cqic_vmode_value = cqic_vmode_value - vmode_reg_step;
	}
	else if((diff >= 0) && (cqic_vmode_value + vmode_reg_step <= 63))
	{
		cqic_vmode_value = cqic_vmode_value + vmode_reg_step;
	}

	cmd2.u.cmd_id = CMD_02;
	cmd2.u.rw 		= SPI_WR;
	cmd2.u.addr		= MODE_DAC_OFFSET;
	cmd2.u.data		= (cqic_vmode_value & 0x3F);
	cqic_write(CQIC_1, cmd2.value, 2);

	//snprintf(msg_buffer, sizeof msg_buffer, "\n\r Mode Trimming... MODE DAC OFFSET[0x%2x], rx_data[0x%2x]", cqic_vmode_value);
	//print(msg_buffer);

	return;	
}

void trimming_procedure_run(void)
{
	cqic_cmd1_type cmd1 = {0, };
	cqic_cmd2_type cmd2 = {0, };
	
    if(gt_cqic_test_step != TEST_STEP_NONE)
    {
		switch(gt_cqic_test_step)
		{
			case TRIMMING_STEP00_00:
				gt_cqic_test_init();
				gt_cqic_test_step = TRIMMING_STEP01_00;
				break;
			case TRIMMING_STEP01_00:
				{
					uint16_t rx_data[2];
					print("======== STEP01_00 VREFO 4.5V Trimming Start ========\r\n");

					#ifdef CHK_TRIM_TIME
					g_CHK_TRIM_TIME_VREF = xTaskGetTickCount();                    
					snprintf(msg_buffer, sizeof msg_buffer, "\r\n g_CHK_TRIM_TIME_VREF - START:%d\r\n ", g_CHK_TRIM_TIME_VREF);
					print(msg_buffer);
					#endif //CHK_TRIM_TIME

					gn_mode_temp_adc[CQIC_DAC_4_5V] = 0;
					gb_adc_start_dma = 0;
					gn_adc_read_count_adc1 = ADC_READ_COUNT;
					gn_adc_read_count_adc2 = 0;
#if 1					
					VREFO_4_5V_offset_count = 511;

					cmd2.u.cmd_id 	= CMD_02;
					cmd2.u.rw 		= SPI_WR;
					cmd2.u.addr 	= V_ANA_GAIN;
					cmd2.u.data 	= VREFO_4_5V_offset_count;		
					cqic_write(CQIC_1, cmd2.value, 2);	/* VREF 1.5V LDO INIT VAULE */
					HAL_Delay(30);
#else

				
				VREFO_4_5V_offset_count = 0;
				
				while(1)
					{
						cmd2.u.cmd_id 	= CMD_02;
						cmd2.u.rw 		= SPI_WR;
						cmd2.u.addr 	= V_ANA_GAIN;
						cmd2.u.data 	= VREFO_4_5V_offset_count;		
						cqic_write(CQIC_1, cmd2.value, 2);	/* VREF 1.5V LDO INIT VAULE */

						VREFO_4_5V_offset_count ++;		//= 16;

						if(VREFO_4_5V_offset_count >= 1024) VREFO_4_5V_offset_count = 0; 

						HAL_Delay(5);
					}

#endif
					gt_cqic_test_step = TRIMMING_STEP01_01;			
				}
			break;	
			case TRIMMING_STEP01_01:	
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
						gt_cqic_test_step = TRIMMING_STEP01_02;
					}
				}			
			break;
			case TRIMMING_STEP01_02:
				if(gb_adc_start_dma == 0)
				{
					float adc_count = (float)gn_mode_temp_adc[CQIC_DAC_4_5V]/ADC_READ_COUNT;
					float vref_o = cqic_adc_to_voltage(adc_count, I_ADC);
					//float vref = cqic_adc_to_voltage(adc_count, I_ADC)*3/2;
					float vref = vref_o*3/2;
					float diff = gf_VREFP_Target - vref;
					int16_t offset=0;
					uint16_t ui_reg_step = 0;
					float f_vref_val_per_reg = 0;

					gn_mode_temp_adc[CQIC_DAC_4_5V] = 0;

					if(gub_loop_cnt > 30)
					{
						if(gub_saved_cnt > 0)
						{
							//find closed to target
							uint8_t temp_closest_cnt = 0;
							for(uint8_t i=0; i < gub_saved_cnt; i++ )
							{								
								if( gui_saved_VREFO_4_5V_diff[i] < gui_saved_VREFO_4_5V_diff[temp_closest_cnt] )
								{
									temp_closest_cnt = i;
								}
							}
							VREFO_4_5V_offset_count = gui_saved_VREFO_4_5V_diff[temp_closest_cnt];
							//Write register
							cmd2.u.cmd_id	= CMD_02;
							cmd2.u.rw 		= SPI_WR;
							cmd2.u.addr		= V_ANA_GAIN;
							cmd2.u.data 	= (VREFO_4_5V_offset_count&0x03FF); 	
							cqic_write(CQIC_1, cmd2.value, 2);	/* VREF 1.5V LDO offset VAULE */
							HAL_Delay(100);

							cqic_Vref_offset_value = VREFO_4_5V_offset_count;
							gt_cqic_test_step = TRIMMING_STEP01_03;
							gub_saved_cnt = 0;
							gub_loop_cnt = 0;

							snprintf(msg_buffer, sizeof msg_buffer, "\n\r VREFO 4.5V Trimming Done- Double Step(%d)!! Diff[%f], OFFSET[%d]!!"
								,gub_saved_cnt ,gui_saved_VREFO_4_5V_diff[temp_closest_cnt], cqic_Vref_offset_value);
							print(msg_buffer);
						}
						else
						{
							print("\r\n Vrefo 4.5V Trimming FAIL!! \r\n");
							gt_cqic_test_step = TRIMMING_STEP_ERROR;
						}
						break;
					}

					if(fabs(diff) >= (float)gf_VREFP_Step)
					{
						// TRIMING Enhanced Algorithm - Vmode 4.5V
						if(gf_pre_vref_diff == 0)
						{
							ui_reg_step = 1;
							gub_loop_cnt = 0;
							gub_saved_cnt = 0;
							// print("========== Vrefo 4.5V Trimming START!! =========== \r\n");
						}
						else
						{
							if(fabs(diff) < (float)gf_VREFP_Step*2)
							{
								gui_saved_VREFO_4_5V_offset_count[gub_saved_cnt]= VREFO_4_5V_offset_count;
								gui_saved_VREFO_4_5V_diff[gub_saved_cnt] = diff;
								gub_saved_cnt++;
								snprintf(msg_buffer, sizeof msg_buffer, "gub_saved_cnt{%d} \r\n",gub_saved_cnt);
								print(msg_buffer);
							}

							if(gub_saved_cnt >= TRIM_REGISTER_SAVED_CNT)
							{
								//find closed to target
								uint8_t temp_closest_cnt = 0;
								for(uint8_t i=0; i < TRIM_REGISTER_SAVED_CNT; i++ )
								{								
									if( gui_saved_VREFO_4_5V_diff[i] < gui_saved_VREFO_4_5V_diff[temp_closest_cnt] )
									{
										temp_closest_cnt = i;
									}
								}
								VREFO_4_5V_offset_count = gui_saved_VREFO_4_5V_diff[temp_closest_cnt];
								//Write register
								cmd2.u.cmd_id	= CMD_02;
								cmd2.u.rw 		= SPI_WR;
								cmd2.u.addr		= V_ANA_GAIN;
								cmd2.u.data 	= (VREFO_4_5V_offset_count&0x03FF); 	
								cqic_write(CQIC_1, cmd2.value, 2);	/* VREF 1.5V LDO offset VAULE */
								HAL_Delay(100);

								cqic_Vref_offset_value = VREFO_4_5V_offset_count;
								gt_cqic_test_step = TRIMMING_STEP01_03;
								gub_saved_cnt = 0;
								gub_loop_cnt = 0;

								snprintf(msg_buffer, sizeof msg_buffer, "\n\r VREFO 4.5V Trimming Done- Double Step(%d)!! Diff[%f], OFFSET[%d]!!"
									,gub_saved_cnt ,gui_saved_VREFO_4_5V_diff[temp_closest_cnt], cqic_Vref_offset_value);
								print(msg_buffer);
								break;
							}
							else
							{
								f_vref_val_per_reg = fabs( (gf_pre_vref_diff - diff)/(gui_pre_cqic_vref_value-VREFO_4_5V_offset_count) );
								if(f_vref_val_per_reg == 0)
								{
									ui_reg_step = abs(gui_pre_cqic_vref_value-VREFO_4_5V_offset_count);
								}
								else
								{
									ui_reg_step = (uint16_t)(fabs(diff)/f_vref_val_per_reg);
								}
								if(ui_reg_step == 0)
								{
									ui_reg_step = 1;
								}
							}
						}

						gf_pre_vref_diff = fabs(diff);
						gui_pre_cqic_vref_value = VREFO_4_5V_offset_count;
						gub_loop_cnt++;

						snprintf(msg_buffer, sizeof msg_buffer, "VREF[%f], VREF_O[%f], DIFF[%f], OFFSET[%d], ADJ[%d]\n\r", vref, vref_o, diff, VREFO_4_5V_offset_count, ui_reg_step);
						print(msg_buffer);

						if(diff > 0)
						{
							VREFO_4_5V_offset_count += ui_reg_step;
						}
						else
						{
							VREFO_4_5V_offset_count -= ui_reg_step;
						}

						if(VREFO_4_5V_offset_count >= 1024)
						{
							snprintf(msg_buffer, sizeof msg_buffer, "\n\r VREFO 4.5V Trimming Error-Over count [%d], [%fV], Org[%fV] !!\r\n", VREFO_4_5V_offset_count, vref, vref_o);
							print(msg_buffer);

							gt_cqic_test_step = TRIMMING_STEP_ERROR;
							return;
						}

						cmd2.u.cmd_id	= CMD_02;
						cmd2.u.rw 		= SPI_WR;
						cmd2.u.addr		= V_ANA_GAIN;
						cmd2.u.data 	= (VREFO_4_5V_offset_count&0x03FF); 	
						cqic_write(CQIC_1, cmd2.value, 2);	/* VREF 1.5V LDO offset VAULE */
						HAL_Delay(100);

						gb_adc_start_dma = 0;
						gn_adc_read_count_adc1 = ADC_READ_COUNT;
						gt_cqic_test_step = TRIMMING_STEP01_01;
					}
					else
					{
						/* case of trimming done */
						cqic_Vref_offset_value = VREFO_4_5V_offset_count;
						gt_cqic_test_step = TRIMMING_STEP01_03;		
						gub_saved_cnt = 0;
						gub_loop_cnt = 0;

						snprintf(msg_buffer, sizeof msg_buffer, "\n\r VREFO 4.5V Trimming Done!! VREF[%4f], VREF_O[%f], Diff[%f], OFFSET[%d]!!",vref,vref_o, (vref-gf_VREFP_Target), cqic_Vref_offset_value);
						print(msg_buffer);						
					}	
				}
			break;
			case TRIMMING_STEP01_03:	
#ifdef TRIM_MANUAL
				gt_cqic_test_step = TRIMMING_STEP_OK;
#else
				gt_cqic_test_step = TRIMMING_STEP02_00;
				// gt_cqic_test_step = TRIMMING_STEP04_00;
#endif
				print("\n\r========== VREFO 4.5V Trimming Done!! =========== \r\n");

					#ifdef CHK_TRIM_TIME
					g_CHK_TRIM_TIME_VREF = xTaskGetTickCount() - g_CHK_TRIM_TIME_VREF;
					snprintf(msg_buffer, sizeof msg_buffer, "\r\n\r\n g_CHK_TRIM_TIME_VREF - END:%d\r\n\r\n", g_CHK_TRIM_TIME_VREF);
					print(msg_buffer);
					#endif //CHK_TRIM_TIME

			break;
			case TRIMMING_STEP01_10:
				gt_cqic_test_init();
				gt_cqic_test_step = TRIMMING_STEP01_20;
				break;
			case TRIMMING_STEP01_20:
				{
					print("======== TRIM START ========\r\n");
					print("======== STEP01_00 LDO Trimming Start(1.5V) ========\r\n");


					gn_mode_temp_adc[CQIC_DAC_1_5V] = 0;
					gb_adc_start_dma = 0;
					gn_adc_read_count_adc1 = ADC_READ_COUNT;
					gn_adc_read_count_adc2 = 0;
					VREFO_4_5V_offset_count = 512;

					cmd2.u.cmd_id 	= CMD_02;
					cmd2.u.rw 		= SPI_WR;
					cmd2.u.addr 	= V_ANA_GAIN;
					cmd2.u.data 	= VREFO_4_5V_offset_count;		
					cqic_write(CQIC_1, cmd2.value, 2);	/* VREF 1.5V LDO INIT VAULE */
					HAL_Delay(30);

					gt_cqic_test_step = TRIMMING_STEP01_30;								
				}
			break;	
			case TRIMMING_STEP01_30:	
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
						gt_cqic_test_step = TRIMMING_STEP01_40;
					}
				}			
			break;
			case TRIMMING_STEP01_40:
				if(gb_adc_start_dma == 0)
				{
					float adc_count = (float)gn_mode_temp_adc[CQIC_DAC_1_5V]/ADC_READ_COUNT;
					float vref = cqic_adc_to_voltage(adc_count, I_ADC);
					float diff = DAC_1_5_TARGET_V - vref;

					gn_mode_temp_adc[CQIC_DAC_1_5V] = 0;

					if(diff >= 0)	diff += DAC_1_5_STEP_V/2;
					else 	diff -= DAC_1_5_STEP_V/2;					

					if(++gt_trim_retry_cnt > RETRY_COUNT)
					{
						/* trimming retry over error */
						snprintf(msg_buffer, sizeof msg_buffer, "\n\r 1.5V LDO Trimming Error-Over Retry !! [%d], [%f] !!", VREFO_4_5V_offset_count,vref);
						print(msg_buffer);
						gt_cqic_test_step = TRIMMING_STEP_ERROR;
						break;
					}

					if(fabs(diff) >= (float)DAC_1_5_STEP_V)
					{
						VREFO_4_5V_offset_count = (uint16_t)(VREFO_4_5V_offset_count-(diff/DAC_1_5_STEP_V));
						
						if(VREFO_4_5V_offset_count > 1024)
						{
							snprintf(msg_buffer, sizeof msg_buffer, "\n\r 1.5V LDO Trimming Error-Over count [%d], [%fV] !!", VREFO_4_5V_offset_count, vref);
							print(msg_buffer);

							gt_cqic_test_step = TRIMMING_STEP_ERROR;
							return;
						}	
						
						cmd2.u.cmd_id	= CMD_02;
						cmd2.u.rw 		= SPI_WR;
						cmd2.u.addr		= V_ANA_GAIN;
						cmd2.u.data 	= (VREFO_4_5V_offset_count&0x03FF); 	
						cqic_write(CQIC_1, cmd2.value, 2);	/* VREF 1.5V LDO offset VAULE */
						HAL_Delay(10);

						snprintf(msg_buffer, sizeof msg_buffer, "\n\r 1.5V LDO Trimming... VREF[%f], DIFF[%f], OFFSET[0x%x]!!", vref, diff, VREFO_4_5V_offset_count);
						print(msg_buffer);

						gb_adc_start_dma = 0;
						gn_adc_read_count_adc1 = ADC_READ_COUNT;
						gt_cqic_test_step = TRIMMING_STEP01_30;								
					}
					else
					{
						/* case of trimming done */
						cqic_Vref_offset_value = VREFO_4_5V_offset_count;
						snprintf(msg_buffer, sizeof msg_buffer, "\n\r 1.5V LDO Trimming Done!! VREF[%4f], Diff[%f], OFFSET[0x%4x]!!",vref, diff, cqic_Vref_offset_value);
						print(msg_buffer);
						gt_cqic_test_step = TRIMMING_STEP01_50;
					}	
				}
			break;
			case TRIMMING_STEP01_50:	
#ifdef TRIM_MANUAL				
				gt_cqic_test_step = TRIMMING_STEP_OK;
#else
				gt_cqic_test_step = TRIMMING_STEP02_00;
#endif
				print("\n\r======== STEP01 LDO 1.5V Trimming End ========\r\n");

			break;

			case TRIMMING_STEP12_00:
				{
					uint16_t ch;
					
					print("\n\r ======== STEP12 24 Chennel 10BIT DAC OFFSET Trimming ========");
					gt_cqic_test_init();
					
					for(ch=0; ch < CH_MAX; ch++)
					{
						cqic_trim_result[ch][0] = 0;	/* set NG flag for init */
						cqic_trim_result[ch][1] = 0.0f;
						cqic_trim_result[ch][2] = 0.0f;
						dac_offset_count[ch] = 0;
						gn_ads114s08_adc_temp[ch] = 0;
						gn_ads114s08_adc_temp_l[ch] = 0;
						cqic_trimming_value[ch][DAC_VREF] = 0;
					}
					
					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;
					gb_ads114s08_drdy_done = 0;
					gn_adc_read_count_adc1 = ADC_READ_COUNT;
					gn_adc_read_count_adc2 = 0;
					gt_trim_retry_cnt = 0; 
					min_max = 0;
					adc_ch = 0;
					gt_cqic_test_step = TRIMMING_STEP12_01;
					
					print("\n\r Start ADC Conversion...");	
					
					//snprintf(msg_buffer, sizeof msg_buffer, "\n\r adc114s08 #1 ch[%2d], [%s]",adc_ch+1, "Get ADC data...");
					//print(msg_buffer);	
					
					Vref_val = gui_DATA_DAC_DAC_Input;
					dac12b_val = gui_GAIN_DAC_vHigh;
					cqic_output_enable(Vref_val, dac12b_val);
					ads114s08_select_single_ended_input(adc_ch, ADC1_CS);
					//HAL_Delay(10);
					HAL_Delay(30);
					ads114s08_set_start(SET, ADC1_CS);
				}
				break;


				
			case TRIMMING_STEP12_01:
				if(gb_adc_drdy_flag1 == 1)
				{
					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;

					if(++adc_ch < 12)
					{
						gn_adc_read_count_adc1 = ADC_READ_COUNT;
						gn_adc_read_count_adc2 = 0;
						min_max = 0;

						if(cqic_trim_result[adc_ch][0] == 1)
						{
							gb_adc_drdy_flag1 = 1;
							break;
						}

						ads114s08_select_single_ended_input(adc_ch, ADC1_CS);
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC1_CS);
					}
					else
					{
						ads114s08_set_start(RESET, ADC1_CS);
						adc_ch = CH_13;
						gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;

						gn_adc_read_count_adc1 = 0;
						gn_adc_read_count_adc2 = ADC_READ_COUNT;

						if(cqic_trim_result[adc_ch][0] == 1)
						{
							gb_ads114s08_drdy_done = 0x01;
							gb_adc_drdy_flag2 = 1;
							gt_cqic_test_step = TRIMMING_STEP12_11;
							break;
						}

						ads114s08_select_single_ended_input(adc_ch-CH_13, ADC2_CS);
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC2_CS);

						gb_ads114s08_drdy_done = 0x01;
						gt_cqic_test_step = TRIMMING_STEP12_11;
					}
				}
				
				break;
			case TRIMMING_STEP12_11:
				if(gb_adc_drdy_flag2 == 1)
				{
					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;

					if(++adc_ch < CH_MAX)
					{
						gn_adc_read_count_adc1 = 0;
						gn_adc_read_count_adc2 = ADC_READ_COUNT;

						if(cqic_trim_result[adc_ch][0] == 1)
						{
							gb_adc_drdy_flag2 = 1;
							break;
						}

						ads114s08_select_single_ended_input(adc_ch-CH_13, ADC2_CS);
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC2_CS);
					}
					else
					{
						ads114s08_set_start(RESET, ADC1_CS);
						ads114s08_set_start(RESET, ADC2_CS);
						
						gb_ads114s08_drdy_done |= 0x02;
						gt_cqic_test_step = TRIMMING_STEP12_02;
					}
				}
				break;
			case TRIMMING_STEP12_02:
				if(gb_ads114s08_drdy_done == 3)
				{
					uint8_t ch;
					float check_trim_done=0;

					print("\n\r Start OFFSET Trimming(Target 2.51718V)...");
					
					for(ch=0; ch < CH_MAX; ch++)
					{
						if(cqic_trim_result[ch][0] == 0)
						{
							cqic_12b_VREF_offset_trimming_12(ch);
						}
						else
						{
							snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d] trim done! V-min[%f], Diff[%f], T_value[0x%4x]", ch+1, cqic_trim_result[ch][1], (DAC_10B_T-cqic_trim_result[ch][1]),cqic_trimming_value[ch][DAC_VREF]);
							print(msg_buffer);						
						}

						check_trim_done += cqic_trim_result[ch][0];
						gn_ads114s08_adc_temp[ch] = 0;
						gn_ads114s08_adc_temp_l[ch] = 0;
					}

					//gt_trim_retry_cnt ++;
					
					if(check_trim_done == CH_MAX)	/* dac_max trimming done */
					{
						ads114s08_set_start(RESET, ADC1_CS);
						ads114s08_set_start(RESET, ADC2_CS);
						
						print("\n\r 10BIT DAC TRIMMING Done!!!");

						for(ch=0; ch< CH_MAX; ch++)
						{
							snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d], V-min[%f], Diff[%f], T_Value[0x%4x], Result[%s]", ch+1, cqic_trim_result[ch][1],(DAC_10B_T-cqic_trim_result[ch][1]), cqic_trimming_value[ch][DAC_VREF],(cqic_trim_result[ch][0] == 1.0 ? "OK!!" : "NG.."));
							print(msg_buffer);
						}

						gt_cqic_test_step = TRIMMING_STEP12_03; 	
					}
					else
					{
						gb_ads114s08_drdy_done = 0;
						gn_adc_read_count_adc1 = gn_adc_read_count_adc2 = ADC_READ_COUNT;
						gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;
						adc_ch = 0;

						if(gt_trim_retry_cnt > RETRY_COUNT)
						{
							ads114s08_set_start(RESET, ADC1_CS);
							ads114s08_set_start(RESET, ADC2_CS);
							gt_cqic_test_step = TRIMMING_STEP_ERROR;
							
							print("\n\r 12BIT DAC TRIMMING Error!!!");
							
							for(ch=0; ch< CH_MAX; ch++)
							{
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d], V-min[%f], Diff[%f], Result[%s]", ch+1, cqic_trim_result[ch][1],(gf_DATA_DAC_Voltage-cqic_trim_result[ch][1]), (cqic_trim_result[ch][0] == 1.0 ? "OK!!" : "NG.."));
								print(msg_buffer);
							}
						}
						else
						{
							snprintf(msg_buffer, sizeof msg_buffer, "\n\r 10BIT DAC TRIMMING Retry(%d)!!!", gt_trim_retry_cnt);
							print(msg_buffer);

							if(cqic_trim_result[adc_ch][0] == 1)
							{
								gb_adc_drdy_flag1 = 1;
								gt_cqic_test_step = TRIMMING_STEP12_01; 				
								
								//snprintf(msg_buffer, sizeof msg_buffer, "\n\r adc114s08 #1 ch[%2d], [%s]",adc_ch+1, "Trimming Done Already!!!");
								//print(msg_buffer);	
								break;
							}

							ads114s08_select_single_ended_input(adc_ch, ADC1_CS);
							//HAL_Delay(3);
							HAL_Delay(30);
							ads114s08_set_start(SET, ADC1_CS);
							gt_cqic_test_step = TRIMMING_STEP12_01; 				
						}
					}					
				}								
				break;
			case TRIMMING_STEP12_03:
				gt_cqic_test_step = TRIMMING_STEP_OK; 
				break;
			
			case TRIMMING_STEP02_00:
				{
					uint16_t ch;

					#ifdef CHK_TRIM_TIME
					g_CHK_TRIM_TIME_GAIN = xTaskGetTickCount();                    
					snprintf(msg_buffer, sizeof msg_buffer, "\r\n g_CHK_TRIM_TIME_GAIN - START:%d\r\n ", g_CHK_TRIM_TIME_GAIN);
					print(msg_buffer);
					#endif //CHK_TRIM_TIME

					print("\n\r ======== STEP02 CQ24 VREF OFFSET Trimming(DIFF 1.3149V) ========\r\n");
					gt_cqic_test_init();

					for(ch=0; ch < CH_MAX; ch++)
					{
						cqic_trim_result[ch][0] = 0;	/* set NG flag for init */
						cqic_trim_result[ch][1] = 0.0f;
						cqic_trim_result[ch][2] = 0.0f;
						vref_offset_count[ch] = 0;
						gn_ads114s08_adc_temp[ch] = 0;
						gn_ads114s08_adc_temp_l[ch] = 0;	
						cqic_trimming_value[ch][DAC_VREF] = 0;
					}

					cqic_output_enable(gui_GAIN_DAC_DAC_Input, 0);

					adc_ch = 0;
					gt_trim_retry_cnt = 0;
					min_max = 0x03;
					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;
					gt_cqic_test_step = TRIMMING_STEP02_01;
				}
				break;
			case TRIMMING_STEP02_01:
				/* set 10bit DAC 684/98 and start ADC interrupt */
				if((adc_ch<=CH_24) && (min_max != 0))
					gt_get_dac10b_adc_data(adc_ch);
				
				gt_cqic_test_step = TRIMMING_STEP02_11;
				
				/* check end of channel & change state machine */
			
				break;
			case TRIMMING_STEP02_11:
				if((gb_adc_drdy_flag1 == 1 ||  gb_adc_drdy_flag2 == 1))
				{
					if(min_max == 0) 
					{
						gt_cqic_test_step = TRIMMING_STEP02_02;
					}
					else
					{
						min_max = min_max>>1;
						gt_cqic_test_step = TRIMMING_STEP02_01;
					}
				}
				break;
			case TRIMMING_STEP02_02:
				if((gb_adc_drdy_flag1 == 1 ||  gb_adc_drdy_flag2 == 1))
				{
					if((adc_ch <= CH_24) && (min_max == 0))
					{
						cqic_12b_VREF_offset_trimming();
					}
					else
					{
						snprintf(msg_buffer, sizeof msg_buffer, "\n\r Trimming... CH[%2d]", adc_ch+1);
						print(msg_buffer);
						ads114s08_set_start(RESET, ADC1_CS);
						ads114s08_set_start(RESET, ADC2_CS);

						gt_cqic_test_step = TRIMMING_STEP02_03;
					}
				}

				/* DAC 10Bit Offset trimming & channel change */
				break;
			case TRIMMING_STEP02_03:
				{
					uint8_t ch;
					
					/* Reporting result */
					print("\n\r GAIN Trimming(DIFF 3.0V) Result!!");

					#ifdef CHK_TRIM_TIME
					g_CHK_TRIM_TIME_GAIN = xTaskGetTickCount() - g_CHK_TRIM_TIME_GAIN;
					snprintf(msg_buffer, sizeof msg_buffer, "\r\n\r\n g_CHK_TRIM_TIME_GAIN - END:%d\r\n\r\n", g_CHK_TRIM_TIME_GAIN);
					print(msg_buffer);
					#endif //CHK_TRIM_TIME

					for(ch=0; ch<24; ch++)
					{
						snprintf(msg_buffer, sizeof msg_buffer, "\n\r CH[%2d], ADC_879[%f], ADC_481[%f], Diff[%f], T-Val[0x%x], result[%s]", ch+1, cqic_trim_result[ch][1],cqic_trim_result[ch][2], (cqic_trim_result[ch][1]-cqic_trim_result[ch][2]), cqic_trimming_value[ch][DAC_VREF], (cqic_trim_result[ch][0]==1 ? "OK!!":"NG.."));
						print(msg_buffer);
					}
#ifdef TRIM_MANUAL
					gt_cqic_test_step = TRIMMING_STEP_OK;
#else
					gt_cqic_test_step = TRIMMING_STEP03_00;
#endif
				}
				break;
			case TRIMMING_STEP03_00:
				{
					uint16_t ch;

					#ifdef CHK_TRIM_TIME
					g_CHK_TRIM_TIME_OFFSET = xTaskGetTickCount();                    
					snprintf(msg_buffer, sizeof msg_buffer, "\r\n g_CHK_TRIM_TIME_OFFSET - START:%d\r\n ", g_CHK_TRIM_TIME_OFFSET);
					print(msg_buffer);
					#endif //CHK_TRIM_TIME

					print("\n\r ======== STEP03 24 Chennel DAC OFFSET Trimming(300mV) ========");
					gt_cqic_test_init();

					for(ch=0; ch < CH_MAX; ch++)
					{
						cqic_trim_result[ch][0] = 0;	/* set NG flag for init */
						cqic_trim_result[ch][1] = 0.0f;
						cqic_trim_result[ch][2] = 0.0f;
						dac_offset_count[ch] = 0;
						gn_ads114s08_adc_temp[ch] = 0;
						gn_ads114s08_adc_temp_l[ch] = 0;
						cqic_trimming_value[ch][DAC_OFFSET] = 0;
					}

					
					ads114s08_set_start(RESET, ADC1_CS);
					ads114s08_set_start(RESET, ADC2_CS);

					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;
					gb_ads114s08_drdy_done = 0;
					gn_adc_read_count_adc1 = ADC_READ_COUNT;
					gn_adc_read_count_adc2 = 0;
					gt_trim_retry_cnt = 0; 
					min_max = 0;
					adc_ch = 0;
					gt_cqic_test_step = TRIMMING_STEP03_01;

					print("\n\r Start ADC Conversion...");	

					//snprintf(msg_buffer, sizeof msg_buffer, "\n\r adc114s08 #1 ch[%2d], [%s]",adc_ch+1, "Get ADC data...");
					//print(msg_buffer);	

					Vref_val = gui_DATA_DAC_DAC_Input;
					dac12b_val = gui_DATA_DAC_Target_Offset_DAC;
					cqic_output_enable(Vref_val, dac12b_val);
					ads114s08_select_single_ended_input(adc_ch, ADC1_CS);
					HAL_Delay(30);
					ads114s08_set_start(SET, ADC1_CS);
				}
			break;	
			case TRIMMING_STEP03_01:
				if(gb_adc_drdy_flag1 == 1)
				{
					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;

					if(++adc_ch < 12)
					{
						gn_adc_read_count_adc1 = ADC_READ_COUNT;
						gn_adc_read_count_adc2 = 0;
						min_max = 0;

						if(cqic_trim_result[adc_ch][0] == 1)
						{
							gb_adc_drdy_flag1 = 1;
							break;
						}

						ads114s08_select_single_ended_input(adc_ch, ADC1_CS);
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC1_CS);
					}
					else
					{
						ads114s08_set_start(RESET, ADC1_CS);
						adc_ch = CH_13;
						gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;

						gn_adc_read_count_adc1 = 0;
						gn_adc_read_count_adc2 = ADC_READ_COUNT;

						if(cqic_trim_result[adc_ch][0] == 1)
						{
							gb_ads114s08_drdy_done = 0x01;
							gb_adc_drdy_flag2 = 1;
							gt_cqic_test_step = TRIMMING_STEP03_11;
							break;
						}

						ads114s08_select_single_ended_input(adc_ch-CH_13, ADC2_CS);
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC2_CS);

						gb_ads114s08_drdy_done = 0x01;
						gt_cqic_test_step = TRIMMING_STEP03_11;
					}
				}
				break;
			case TRIMMING_STEP03_11:
				if(gb_adc_drdy_flag2 == 1)
				{
					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;

					if(++adc_ch < CH_MAX)
					{
						gn_adc_read_count_adc1 = 0;
						gn_adc_read_count_adc2 = ADC_READ_COUNT;

						if(cqic_trim_result[adc_ch][0] == 1)
						{
							gb_adc_drdy_flag2 = 1;
							break;
						}

						ads114s08_select_single_ended_input(adc_ch-CH_13, ADC2_CS);
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC2_CS);
					}
					else
					{
						ads114s08_set_start(RESET, ADC1_CS);
						ads114s08_set_start(RESET, ADC2_CS);
						
						gb_ads114s08_drdy_done |= 0x02;
						gt_cqic_test_step = TRIMMING_STEP03_02;
					}
				}
				break;
			case TRIMMING_STEP03_02:
				if(gb_ads114s08_drdy_done == 3)
				{
					uint8_t ch;
					float check_trim_done=0;

					//print("\n\r Start OFFSET Trimming(Target 300mV)...");
					// ch = 0;
					for(ch=0; ch < CH_MAX; ch++)
					{
						if(cqic_trim_result[ch][0] == 0)
						{
							cqic_12b_DAC_offset_trimming(ch);
						}
						else
						{
							snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d] trim done! V-min[%f], Diff[%f], T_value[0x%4x]", ch+1, cqic_trim_result[ch][1], (gf_DATA_DAC_Voltage-cqic_trim_result[ch][1]),cqic_trimming_value[ch][DAC_OFFSET]);
							print(msg_buffer);						
						}

						check_trim_done += cqic_trim_result[ch][0];
						gn_ads114s08_adc_temp[ch] = 0;
						gn_ads114s08_adc_temp_l[ch] = 0;
					}

					//gt_trim_retry_cnt ++;
					
					if(check_trim_done == CH_MAX)	/* dac_max trimming done */
					{
						ads114s08_set_start(RESET, ADC1_CS);
						ads114s08_set_start(RESET, ADC2_CS);
						
						print("\n\r 12BIT DAC TRIMMING Done!!!\r\n");						

						for(ch=0; ch< CH_MAX; ch++)
						{
							snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d], V-min[%f], Diff[%f], T_Value[0x%4x], Result[%s]", ch+1, cqic_trim_result[ch][1],(gf_DATA_DAC_Voltage-cqic_trim_result[ch][1]), cqic_trimming_value[ch][DAC_OFFSET],(cqic_trim_result[ch][0] == 1.0 ? "OK!!" : "NG.."));
							print(msg_buffer);
						}

						GUI_SEND_reg_all();

						gt_cqic_test_step = TRIMMING_STEP03_03; 	
					}
					else
					{
						gb_ads114s08_drdy_done = 0;
						gn_adc_read_count_adc1 = gn_adc_read_count_adc2 = ADC_READ_COUNT;
						gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;
						adc_ch = 0;

						if(gt_trim_retry_cnt > RETRY_COUNT)
						{
							ads114s08_set_start(RESET, ADC1_CS);
							ads114s08_set_start(RESET, ADC2_CS);
							gt_cqic_test_step = TRIMMING_STEP_ERROR;
							
							print("\n\r 12BIT DAC TRIMMING Error!!!");
							
							for(ch=0; ch< CH_MAX; ch++)
							{
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r ch[%2d], V-min[%f], Diff[%f], Result[%s]", ch+1, cqic_trim_result[ch][1],(gf_DATA_DAC_Voltage-cqic_trim_result[ch][1]), (cqic_trim_result[ch][0] == 1.0 ? "OK!!" : "NG.."));
								print(msg_buffer);
							}
						}
						else
						{
							//snprintf(msg_buffer, sizeof msg_buffer, "\n\r 12BIT DAC TRIMMING Retry(%d)!!!", gt_trim_retry_cnt);
							//print(msg_buffer);

							if(cqic_trim_result[adc_ch][0] == 1)
							{
								gb_adc_drdy_flag1 = 1;
								gt_cqic_test_step = TRIMMING_STEP03_01; 				
								
								//snprintf(msg_buffer, sizeof msg_buffer, "\n\r adc114s08 #1 ch[%2d], [%s]",adc_ch+1, "Trimming Done Already!!!");
								//print(msg_buffer);	
								break;
							}

							ads114s08_select_single_ended_input(adc_ch, ADC1_CS);
							//HAL_Delay(3);
							HAL_Delay(30);
							ads114s08_set_start(SET, ADC1_CS);
							gt_cqic_test_step = TRIMMING_STEP03_01; 				
						}
					}					
				}				
			break;	
			case TRIMMING_STEP03_03:
#ifdef TRIM_MANUAL
				gt_cqic_test_step = TRIMMING_STEP_OK; 
#else //TRIM_MANUAL
				if(gb_OTP_WRITE == TRUE)
				{
					gt_cqic_test_step = TRIMMING_STEP_OTP_WRITE;
				}
				else
				{					
					gt_cqic_test_step = TRIMMING_STEP04_00;
				}
#endif //TRIM_MANUAL
				#ifdef CHK_TRIM_TIME
				snprintf(msg_buffer, sizeof msg_buffer, "\r\ng_CHK_TRIM_TIME_MODE  - END:%05d\r\n", g_CHK_TRIM_TIME_MODE);
				print(msg_buffer);
				snprintf(msg_buffer, sizeof msg_buffer, "g_CHK_TRIM_TIME_VREF  - END:%05d\r\n", g_CHK_TRIM_TIME_VREF);
				print(msg_buffer);
				snprintf(msg_buffer, sizeof msg_buffer, "g_CHK_TRIM_TIME_GAIN  - END:%05d\r\n", g_CHK_TRIM_TIME_GAIN);
				print(msg_buffer);
				g_CHK_TRIM_TIME_OFFSET = xTaskGetTickCount() - g_CHK_TRIM_TIME_OFFSET;
				snprintf(msg_buffer, sizeof msg_buffer, "g_CHK_TRIM_TIME_OFFSET - END:%05d\r\n\r\n", g_CHK_TRIM_TIME_OFFSET);
				print(msg_buffer);
				#endif //CHK_TRIM_TIME

				break;
			case TRIMMING_STEP04_00:
				{
					uint16_t ch;

					print("\n\r ======== STEP04 24 Chennel DAC Slop Test ========");

					for(ch=0; ch < CH_MAX; ch++)
					{
						gn_ads114s08_adc_temp[ch] = 0;
						gn_ads114s08_adc_temp_l[ch] = 0;
					}

					cqic_trim_reg_verify();

					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;
					gn_adc_read_count_adc1 = CONFIRM_COUNT;
					gn_adc_read_count_adc2 = 0;
					min_max = 0;
					adc_ch = CH_01;

 					Vref_val = gui_SLOPE_GAIN_DAC_INPUT;		//VREF_DEFAULT;
					dac12b_val = gui_TRIM_RANGE_MIN;			//DAC12B_DEFAULT;
					cqic_output_enable(Vref_val, dac12b_val);
					ads114s08_select_single_ended_input(adc_ch, ADC1_CS);
					//HAL_Delay(3);
					HAL_Delay(30);
					ads114s08_set_start(SET, ADC1_CS);
					print("\r\n");
#if VERF_SLOP
					snprintf(msg_buffer, sizeof msg_buffer, "\n\r Start DAC Output Test(VREF : 0 ~ 879, DAC-12Bit[%d]",dac12b_val);
					print(msg_buffer);	
					print("\r\n");
					snprintf(msg_buffer, sizeof msg_buffer, "\r\ncqic:slop:%d\r\n",Vref_val);
					print(msg_buffer);
					print("\r\n");
#else
					snprintf(msg_buffer, sizeof msg_buffer, "\n\r Start DAC Output Test(VREF[%d], DAC-12Bit : 0 ~ 4095",Vref_val);
					print(msg_buffer);	
					print("\r\n");
					snprintf(msg_buffer, sizeof msg_buffer, "\r\ncqic:slop:%d\r\n",Vref_val);
					print(msg_buffer);
					print("\r\n");
#endif				
					gt_cqic_test_step = TRIMMING_STEP04_01;
				}
				break;
			case TRIMMING_STEP04_01:
				if(gb_adc_drdy_flag1==1)
				{
					float temp;
					
					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;

					temp = (float)(gn_ads114s08_adc_temp[adc_ch]/CONFIRM_COUNT);
					v_min[adc_ch] = cqic_adc_to_voltage(temp, E_ADC);
					gn_ads114s08_adc_temp[adc_ch]=0;
					
					if(++adc_ch < 12)
					{
						gn_adc_read_count_adc1 = CONFIRM_COUNT;
						gn_adc_read_count_adc2 = 0;
						ads114s08_select_single_ended_input(adc_ch, ADC1_CS);	
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC1_CS);

						gt_cqic_test_step = TRIMMING_STEP04_01;
						
					}
					else
					{
						ads114s08_set_start(RESET, ADC1_CS);

						gn_adc_read_count_adc1 = 0;
						gn_adc_read_count_adc2 = CONFIRM_COUNT;
						adc_ch = CH_13;

						ads114s08_select_single_ended_input(adc_ch-CH_13, ADC2_CS);	
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC2_CS);
						
						gt_cqic_test_step = TRIMMING_STEP04_02;
					}
				}
				break;				
			case TRIMMING_STEP04_11:
				break;
			case TRIMMING_STEP04_02:
				if(gb_adc_drdy_flag2  == 1)
				{
					float temp;

					gb_adc_drdy_flag1 = gb_adc_drdy_flag2 = 0;

					temp = (float)gn_ads114s08_adc_temp[adc_ch]/CONFIRM_COUNT;
					v_min[adc_ch] = cqic_adc_to_voltage(temp, E_ADC);
					gn_ads114s08_adc_temp[adc_ch]=0;
					
					if(++adc_ch < CH_MAX)
					{
						gn_adc_read_count_adc1 = 0;
						gn_adc_read_count_adc2 = CONFIRM_COUNT;
						gt_cqic_test_step = TRIMMING_STEP04_02;

 						ads114s08_select_single_ended_input(adc_ch-CH_13, ADC2_CS);	
						HAL_Delay(3);
						ads114s08_set_start(SET, ADC2_CS);
					}
					else
					{
						ads114s08_set_start(RESET, ADC1_CS);
						ads114s08_set_start(RESET, ADC2_CS);

#if VREF_SLOP
						snprintf(msg_buffer, sizeof msg_buffer, "\n\r VREF[%4d], %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f", Vref_val,v_min[0],v_min[1],v_min[2],v_min[3],v_min[4],v_min[5],v_min[6],v_min[7],v_min[8],v_min[9],v_min[10],v_min[11],v_min[12],v_min[13],v_min[14],v_min[15],v_min[16],v_min[17],v_min[18],v_min[19],v_min[20],v_min[21],v_min[22],v_min[23] );
						print(msg_buffer);

						Vref_val += 10;
						if(Vref_val == 880)	Vref_val = 879;

						if(Vref_val <= 879)
						{
							cmd2.u.cmd_id	= CMD_02;
							cmd2.u.rw		= SPI_WR;
							cmd2.u.addr 	= FRAME_HEADER;
							cmd2.u.data 	= Vref_val;	
							cqic_write(CQIC_1, cmd2.value, 2);	/* set ANA DAC 10bit */
							HAL_Delay(3);
						
							gn_adc_read_count_adc1 = CONFIRM_COUNT;
							gn_adc_read_count_adc2 = 0;
							adc_ch = 0;

							ads114s08_select_single_ended_input(adc_ch, ADC1_CS); 
							//HAL_Delay(3);
							HAL_Delay(30);
							ads114s08_set_start(SET, ADC1_CS);
							
							gt_cqic_test_step = TRIMMING_STEP04_01;
						}
						else
						{
							print("\r\n\r\n====Done SLOP Test!!!====\r\n");
							gt_cqic_test_step = TRIMMING_STEP04_03;
						}
#else
						// snprintf(msg_buffer, sizeof msg_buffer, "\n\r DAC12B[%4d], %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f", dac12b_val,v_min[0],v_min[1],v_min[2],v_min[3],v_min[4],v_min[5],v_min[6],v_min[7],v_min[8],v_min[9],v_min[10],v_min[11],v_min[12],v_min[13],v_min[14],v_min[15],v_min[16],v_min[17],v_min[18],v_min[19],v_min[20],v_min[21],v_min[22],v_min[23] );
						snprintf(msg_buffer, sizeof msg_buffer, "\n\rcqic:csvu:%4d:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f", dac12b_val,v_min[0],v_min[1],v_min[2],v_min[3],v_min[4],v_min[5],v_min[6],v_min[7],v_min[8],v_min[9],v_min[10],v_min[11],v_min[12],v_min[13],v_min[14],v_min[15],v_min[16],v_min[17],v_min[18],v_min[19],v_min[20],v_min[21],v_min[22],v_min[23]);
						print(msg_buffer);

						dac12b_val += 10;
						if(dac12b_val == 4100)	dac12b_val = 4095;

  						if(dac12b_val <= gui_TRIM_RANGE_MAX)
						{
							cmd2.u.cmd_id	= CMD_02;
							cmd2.u.rw		= SPI_WR;
							cmd2.u.addr 	= LD_I;
							cmd2.u.data 	= dac12b_val;
							cqic_write(CQIC_1, cmd2.value, 2);	/* set Current DAC 12bit */
							HAL_Delay(3);
							gn_adc_read_count_adc1 = CONFIRM_COUNT;
							gn_adc_read_count_adc2 = 0;
							adc_ch = 0;

							ads114s08_select_single_ended_input(adc_ch, ADC1_CS); 
							//HAL_Delay(3);
							HAL_Delay(30);
							ads114s08_set_start(SET, ADC1_CS);
							
							gt_cqic_test_step = TRIMMING_STEP04_01;
						}
						else
						{
							print("\r\n\r\n====Done SLOP Test!!!====\r\n");
							gt_cqic_test_step = TRIMMING_STEP04_03;
						}
#endif		
					}
				}
				break;
			case TRIMMING_STEP04_03:
			case TRIMMING_STEP04_04:
				GUI_Reset_test_state();
				gt_cqic_test_step = TEST_STEP_NONE;
				break;
				
			/* MODE1/2 LEVEL TEST */
			case TRIMMING_STEP05_00:
				print("\n\r AQIC Mode Test-STEP 0");

				#ifdef CHK_TRIM_TIME
				g_CHK_TRIM_TIME_MODE = xTaskGetTickCount();                    
				snprintf(msg_buffer, sizeof msg_buffer, "\r\n g_CHK_TRIM_TIME_MODE - START:%d\r\n ", g_CHK_TRIM_TIME_MODE);
				print(msg_buffer);
				#endif //CHK_TRIM_TIME

				gt_cqic_test_init();
			
				aq_mode = 2;
				cqic_vmode_value = 31;
				gn_mode_temp_adc[CQIC_MODE_2] = 0;
				gn_mode_temp_adc[CQIC_MODE_1] = 0;
				gb_adc_start_dma = 0;
				gn_adc_read_count_adc1 = ADC_READ_COUNT;
			
			
				/* set command type 1 reg */
				cmd1.u.cmd_id	= CMD_01;
				cmd1.u.addr 	= MODE_CONTROL;
				cmd1.u.rw		= SPI_WR;
				//cmd1.u.data 	= 0x0401;		/* EXT CLK:1, DRV_MODE:1 */
				cmd1.u.data 	= 0x0001;		/* EXT CLK:0, DRV_MODE:1 */
				cqic_write(CQIC_1, &cmd1.value, 1);

				gt_cqic_test_step = TRIMMING_STEP05_01;
				break;
			case TRIMMING_STEP05_01:
			  cqic_mode_out(aq_mode);
				HAL_Delay(100);
				
				gt_cqic_test_step = TRIMMING_STEP05_02;
				break;
			case TRIMMING_STEP05_02:
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
						gt_cqic_test_step = TRIMMING_STEP05_03;
					}
				}			
				
				break;
			case TRIMMING_STEP05_03:
				if(gb_adc_start_dma == 0)
				{
					float adc_count;
					float diff, vmode_1,vmode_2;
					float f_vmode_val_per_reg = 0;
					uint16_t ui_reg_step = 0;;


					adc_count = (float)gn_mode_temp_adc[CQIC_MODE_1]/ADC_READ_COUNT;
					vmode_1 = cqic_adc_to_voltage(adc_count, I_ADC);

					adc_count = (float)gn_mode_temp_adc[CQIC_MODE_2]/ADC_READ_COUNT;
					vmode_2 = cqic_adc_to_voltage(adc_count, I_ADC);

					gn_mode_temp_adc[CQIC_MODE_1] = 0;
					gn_mode_temp_adc[CQIC_MODE_2] = 0;
					gn_adc_read_count_adc1 = ADC_READ_COUNT;
					gb_adc_start_dma = 0;

					if(gub_loop_cnt > 30)
					{
						if(gub_saved_cnt > 0)
						{
							//find closed to target
							uint8_t temp_closest_cnt = 0;
							for(uint8_t i=0; i < gub_saved_cnt; i++ )
							{								
								if( gui_saved_vmode_diff[i] < gui_saved_vmode_diff[temp_closest_cnt] )
								{
									temp_closest_cnt = i;
								}
							}
							//Write register
							cqic_mode_trim(gui_saved_cqic_vmode_value[temp_closest_cnt],0);

							snprintf(msg_buffer, sizeof msg_buffer, "\n\r Vmode 2.5V Trimming Done(%d)!! Vmode_DIFF[%f], OFFSET[%d], !!"
								,gub_saved_cnt ,gui_saved_vmode_diff[temp_closest_cnt],cqic_vmode_value);
							print(msg_buffer);
							print("\r\n ========== Vmode 2.5V Trimming Done - Doble Step!! =========== \r\n");

							cqic_mode_out(0);
							gt_cqic_test_step = TRIMMING_STEP05_04;
							gub_loop_cnt = 0;
							gub_saved_cnt = 0;
						}
						else
						{
							print("\r\n Vmode 2.5V Trimming FAIL!! \r\n");
							gt_cqic_test_step = TRIMMING_STEP_ERROR;

							// //Write register
							// cqic_mode_trim(63,0);
							// print("\r\n ========== Vmode 2.5V Trimming TEST =========== \r\n");

							// cqic_mode_out(0);
							// gt_cqic_test_step = TRIMMING_STEP05_04;
							// gub_loop_cnt = 0;
							// gub_saved_cnt = 0;

						}
						break;
					}

					// TRIMING Enhanced Algorithm - Vmode 2.5V
					diff = gf_MODE_DAC_Target - ((vmode_1+vmode_2)/2);
					if(gf_pre_vmode_diff == 0)
					{
						ui_reg_step = 1;
						gub_loop_cnt = 0;
						gub_saved_cnt = 0;
						print("\r\n ========== Vmode 2.5V Trimming START!! =========== \r\n");						
					}
					else
					{
						f_vmode_val_per_reg = fabs( fabs(gf_pre_vmode_diff - diff)/(gui_pre_cqic_vmode_value-cqic_vmode_value) );
						if(f_vmode_val_per_reg == 0)
						{
							ui_reg_step = abs(gui_pre_cqic_vmode_value-cqic_vmode_value);
						}
						else
						{
							ui_reg_step = (uint16_t)(fabs(diff)/f_vmode_val_per_reg);
							if(ui_reg_step == 0)
							{
								ui_reg_step = 1;
							}
						}

						if((diff < 0)) 
						{
							if(cqic_vmode_value - ui_reg_step < 0)
							{
								snprintf(msg_buffer, sizeof msg_buffer, "\r\nUnder Zero!! cqic_vmode_value(%d) - ui_reg_step(%d) < 0\r\n",cqic_vmode_value, ui_reg_step);
								print(msg_buffer);
								ui_reg_step = 1;
							}
						}
						else
						{
							if(cqic_vmode_value + ui_reg_step > 63)
							{
								snprintf(msg_buffer, sizeof msg_buffer, "\r\nOver 63!! cqic_vmode_value(%d) + ui_reg_step(%d) > 63\r\n",cqic_vmode_value, ui_reg_step);
								print(msg_buffer);
								ui_reg_step = 1;
							}
						}
					}
					gf_pre_vmode_diff = fabs(diff);
					gui_pre_cqic_vmode_value = cqic_vmode_value;
					gub_loop_cnt ++;

					snprintf(msg_buffer, sizeof msg_buffer, "Mode[%d] MODE_1 [%f], MODE_2 [%f] Vmode_DIFF[%f] Next_Step[%d] OFFSET[%d]\r\n", aq_mode, vmode_1,vmode_2, diff, ui_reg_step, cqic_vmode_value);
					print(msg_buffer);

					if(fabs(diff) > gf_MODE_DAC_Step)
					{
						if(fabs(diff) < gf_MODE_DAC_Step*2)
						{
							gui_saved_cqic_vmode_value[gub_saved_cnt] = cqic_vmode_value;
							gui_saved_vmode_diff[gub_saved_cnt] = diff;
							gub_saved_cnt++;
							print("\r\ngub_saved_cnt++\r\n");
						}

						if(gub_saved_cnt >= TRIM_REGISTER_SAVED_CNT)
						{
							//find closed to target
							uint8_t temp_closest_cnt = 0;
							for(uint8_t i=0; i < TRIM_REGISTER_SAVED_CNT; i++ )
							{								
								if( gui_saved_vmode_diff[i] < gui_saved_vmode_diff[temp_closest_cnt] )
								{
									temp_closest_cnt = i;
								}
							}
							//Write register
							cqic_mode_trim(gui_saved_cqic_vmode_value[temp_closest_cnt],0);

							snprintf(msg_buffer, sizeof msg_buffer, "\n\r Vmode 2.5V Trimming Done(%d)!! Vmode_DIFF[%f], OFFSET[%d], !!"
								,gub_saved_cnt ,gui_saved_vmode_diff[temp_closest_cnt],cqic_vmode_value);
							print(msg_buffer);
							print("\r\n ========== Vmode 2.5V Trimming Done - Doble Step!! =========== \r\n");

							cqic_mode_out(0);
							gt_cqic_test_step = TRIMMING_STEP05_04;
							gub_loop_cnt = 0;
							gub_saved_cnt = 0;
							break;
						}
						else
						{
							cqic_mode_trim(diff, ui_reg_step);
							gt_cqic_test_step = TRIMMING_STEP05_02;
						}
					}
					else
					{
						snprintf(msg_buffer, sizeof msg_buffer, "\n\r Vmode 2.5V Trimming Done!! Mode[%d], MODE_1 [%f], MODE_2 [%f] Vmode_DIFF[%f], OFFSET[%d], !!", aq_mode, vmode_1, vmode_2, diff,cqic_vmode_value);
						print(msg_buffer);
						print("\r\n ========== Vmode 2.5V Trimming Done!! =========== \r\n");

						cqic_mode_out(0);
						gt_cqic_test_step = TRIMMING_STEP05_04;
						gub_loop_cnt = 0;
						gub_saved_cnt = 0;
					}
				}
				break;
			case TRIMMING_STEP05_04:
				{
#if 0			/* mdoe output test 0~3 */
					uint16_t in_mode=0;
					while(1)
					{
						cqic_mode_out(in_mode);
						HAL_Delay(3);

						in_mode++;

						if(in_mode > 3) in_mode = 0;
						
					}
#endif				
#ifdef TRIM_MANUAL
					gt_cqic_test_step = TRIMMING_STEP_OK; 
#else
					gt_cqic_test_step = TRIMMING_STEP00_00;
#endif //TRIM_MANUAL

					#ifdef CHK_TRIM_TIME
					g_CHK_TRIM_TIME_MODE = xTaskGetTickCount() - g_CHK_TRIM_TIME_MODE;
					snprintf(msg_buffer, sizeof msg_buffer, "\r\n\r\n g_CHK_TRIM_TIME_MODE - END:%d\r\n\r\n", g_CHK_TRIM_TIME_MODE);
					print(msg_buffer);
					#endif //CHK_TRIM_TIME
				}
				break;
			case TRIMMING_STEP_OTP_WRITE:	/* spi r/w function change into nmic jig */
				#ifdef CHK_TRIM_TIME
				g_CHK_TRIM_TIME_OTP = xTaskGetTickCount();                    
				snprintf(msg_buffer, sizeof msg_buffer, "\r\n g_CHK_TRIM_TIME_OTP - START:%d\r\n ", g_CHK_TRIM_TIME_OTP);
				print(msg_buffer);
				#endif //CHK_TRIM_TIME

				/* otp write processing(please level up mcu supply power to 5.8V) */
				cqic_trim_value_verify();
				cqic_trim_data_to_otp();
				gt_cqic_test_step = TRIMMING_STEP_OK;				

				#ifdef CHK_TRIM_TIME
				g_CHK_TRIM_TIME_OTP = xTaskGetTickCount() - g_CHK_TRIM_TIME_OTP;
				snprintf(msg_buffer, sizeof msg_buffer, "\r\n\r\n g_CHK_TRIM_TIME_OTP - END:%d\r\n\r\n", g_CHK_TRIM_TIME_OTP);
				print(msg_buffer);
				#endif //CHK_TRIM_TIME
			break;
			case TRIMMING_STEP_OK:
			case TRIMMING_STEP_STOP:	
				//cqic_output_disable();
				set_ok_led(ON);
				set_ng_led(OFF);
				set_trimming_led(OFF);
				gt_cqic_test_step = TEST_STEP_NONE;

				GUI_SEND_otp_written();
            	HAL_Delay(1000);
            	NVIC_SystemReset();

				break;
			case TRIMMING_STEP_ERROR:
				//cqic_output_disable();

				set_ok_led(OFF);
				set_ng_led(ON);
				set_trimming_led(OFF);

				/* cqic power off */
				//set_cqic_power_off();

				gt_cqic_test_step = TEST_STEP_NONE;		
				break;
			default:
			break;
		}		
    }
}


/*** end of file ***/


