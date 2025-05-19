
/** @file xc_trim.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XC_TRIM_C__
#include "main.h"
#include "xc24.h"
#include "xc_trim.h"
#include "JigBd_IF.h"
#include "ads124s08.h"
#include "config.h"
#include "math.h"

#define ADJ_NONE                    (0)
#define ADJ_PLUS                    (1)
#define ADJ_MINUS                   (2)
#define ADJ_DEFAULT                 (3)

#define TRIM_REGISTER_SAVED_CNT     (5)
#define TRIM_OUT_RANGE_CNT          (25)

uint16_t trim_find_regs[TRIM_FIND_MAX] = {0, };
dac_gain_tgt_t dac_gain_tgt_buff = {0, };
float osc_value_buffer[128] = {0, };

static bool gb_xd_otp_write_flag;

static trimming_step_t gt_jig_trimming_step;

typedef struct
{
    uint16_t u16_saved_reg;
    uint16_t u16_saved_adc;
} _sTrimSaved;

typedef struct
{
    current_gain_t g8_trim_gain_level;
    uint16_t u16_trim_range_adc_min;
    uint16_t u16_trim_range_adc_target;
    uint16_t u16_trim_range_adc_max;
} _sTrimRange;

typedef struct
{
	uint8_t min_gap_index;
	float min_gap_freq;
} _osc_min_t;
_osc_min_t _osc_min;

void Trim_IF_Trimming_Start(void)
{
    if (gt_jig_trimming_step == TRIMMING_STEP_NONE)
    {
        gt_jig_trimming_step = TRIMMING_STEP_ACTIVATE_START;
    }
}

void Trim_IF_Set_OTP_Enable(bool in_flag)
{
    gb_xd_otp_write_flag = in_flag;
}

bool Trim_IF_Get_OTP_Enable(void)
{
    return gb_xd_otp_write_flag;
}

/* BEGIN - TRIMMING ALGORITHM   ***************************************/

/* BEGIN - TRIMMING_PROCEDURE_RUN   *****************************************/
void XC_Trimming_Procedure_Run(void)
{
	static uint8_t over_run_cnt = 1;

    if (gt_jig_trimming_step != TRIMMING_STEP_NONE)
    {
        switch(gt_jig_trimming_step)
        {
        case TRIMMING_STEP_STANDBY:
			print(LOG_INFO, "=============TRIMMING_STEP_STANDBY=============\r\n");
			gt_jig_trimming_step = TRIMMING_STEP_ACTIVATE_START;

        case TRIMMING_STEP_ACTIVATE_START:
			print(LOG_INFO, "=============TRIMMING_STEP_ACTIVATE_START=============\r\n");
			gt_jig_trimming_step = TRIMMING_STEP_IC_PWR;
			break;

		case TRIMMING_STEP_IC_PWR:
			print(LOG_INFO, "=============TRIMMING_STEP_IC_PWR=============\r\n");
			XC24_Trim_Init();
			gt_jig_trimming_step = TRIMMING_STEP_ACTIVATE_END;
			break;

        case TRIMMING_STEP_ACTIVATE_END:
			print(LOG_INFO, "=============TRIMMING_STEP_ACTIVATE_END=============\r\n");
			gt_jig_trimming_step = TRIMMING_STEP_VCTL_LDO;
			break;

		case TRIMMING_STEP_VCTL_LDO:
			print(LOG_INFO, "=============TRIMMING_STEP_VCTL_LDO=============\r\n");
			XC24_VCTL_LDO_TRIM_INIT();

			static uint8_t vctl_ldo_reg = 8;
			static float vctl_ldo_level = 0;

			ADS114S08_Select_Input_CH(1);
			HAL_Delay(1);
			gb_ads114s08_drdy_done = 0;
			gn_adc_read_count = ADS114S08_READ_COUNT;
			ADS114S08_Set_Start(1);
			while(1)
			{
				if (gb_ads114s08_drdy_done)
				{
					break;
				}
			}

			vctl_ldo_level = (ADS114S_VREF * (((float)(gn_ads114s08_adc_temp) / ADS114S08_READ_COUNT + 0.5f) / 32767)) / 1000; // Dacout convert to V
			gn_ads114s08_adc_temp = 0;

			if((over_run_cnt >= VCTL_LDO_TRIM_OVER_COUNT) || (vctl_ldo_reg >= 15) || (vctl_ldo_reg <= 0))
			{
				print(LOG_ERROR, "VCTL_LDO trim error\r\n");
				gt_jig_trimming_step = TRIMMING_STEP_STOP;
				break;
			}
			if(vctl_ldo_level >= VCTL_LDO_UPPER_LIMIT)
			{
				--vctl_ldo_reg;
			}
			else if(vctl_ldo_level <= VCTL_LDO_LOWER_LIMIT)
			{
				++vctl_ldo_reg;
			}
			else // in range
			{
				print(LOG_INFO, "Loop[%d] VCTL_LDO_REGS[%d] -> LDO_LEVEL : %f\r\n", over_run_cnt, vctl_ldo_reg, vctl_ldo_level);
				print(LOG_INFO, "VCTL_LDO Trim done\r\n");
				over_run_cnt = 1;
				gt_jig_trimming_step = TRIMMING_STEP_DAC_GAIN;
				break;
			}
			XC24_TRIM_WRITE_VCTL_LDO(vctl_ldo_reg);
			print(LOG_INFO, "Loop[%d] VCTL_LDO_REGS[%d] -> LDO_LEVEL : %f\r\n", over_run_cnt, vctl_ldo_reg, vctl_ldo_level);
			++over_run_cnt;
			break;

        case TRIMMING_STEP_DAC_GAIN:
			print(LOG_INFO, "=============TRIMMING_STEP_DAC_GAIN=============\r\n");
			static uint8_t dac_gain_reg = 32;

			XC24_DAC_GAIN_TRIM_INIT();

			XC24_Write_Register(XC24_ADDR_CURRENT_TARGET_DAC, XC24_DAC_GAIN_P1);

			ADS114S08_Select_Input_CH(2);
			HAL_Delay(1);
			gb_ads114s08_drdy_done = 0;
			gn_adc_read_count = ADS114S08_READ_COUNT;
			ADS114S08_Set_Start(1);
			while(1)
			{
				if (gb_ads114s08_drdy_done)
				{
					break;
				}
			}

			dac_gain_tgt_buff.dac_gain_tgt_p1 = (ADS114S_VREF * (((float)(gn_ads114s08_adc_temp) / ADS114S08_READ_COUNT + 0.5f) / 32767)) / 1000; // dacout convert to V
			gn_ads114s08_adc_temp = 0;

			XC24_Write_Register(XC24_ADDR_CURRENT_TARGET_DAC, XC24_DAC_GAIN_P2);

			gb_ads114s08_drdy_done = 0;
			gn_adc_read_count = ADS114S08_READ_COUNT;
			ADS114S08_Set_Start(1);
			while(1)
			{
				if (gb_ads114s08_drdy_done)
				{
					break;
				}
			}

			dac_gain_tgt_buff.dac_gain_tgt_p2 = (ADS114S_VREF * (((float)(gn_ads114s08_adc_temp) / ADS114S08_READ_COUNT + 0.5f) / 32767)) / 1000; // dacout convert to V
			gn_ads114s08_adc_temp = 0;

			if((over_run_cnt >= DAC_GAIN_TRIM_OVER_COUNT) || (dac_gain_reg >= 63) || (dac_gain_reg <= 0))
			{
				print(LOG_ERROR, "DAC_GAIN trim error\r\n");
				gt_jig_trimming_step = TRIMMING_STEP_STOP;

				break;
			}

			if((dac_gain_tgt_buff.dac_gain_tgt_p2 - dac_gain_tgt_buff.dac_gain_tgt_p1) >= DAC_GAIN_UPPER_LIMIT)
			{
				--dac_gain_reg;
			}
			else if ((dac_gain_tgt_buff.dac_gain_tgt_p2 - dac_gain_tgt_buff.dac_gain_tgt_p1) <= DAC_GAIN_LOWER_LIMIT)
			{
				++dac_gain_reg;
			}
			else
			{
				print(LOG_INFO, "Loop[%d] DAC_GAIN_REG[%d] -> DAC_GAIN_LEVEL : %f\r\n"
				, over_run_cnt, dac_gain_reg
				, dac_gain_tgt_buff.dac_gain_tgt_p2 - dac_gain_tgt_buff.dac_gain_tgt_p1);
				print(LOG_INFO, "DAC_GAIN Trim done\r\n");
				over_run_cnt = 1;
				gt_jig_trimming_step = TRIMMING_STEP_DAC_OFS;

				break;
			}

			XC24_TRIM_WRITE_DAC_GAIN(dac_gain_reg);
			print(LOG_INFO, "Loop[%d] DAC_GAIN_REG[%d] -> DAC_GAIN_LEVEL : %f\r\n"
				, over_run_cnt, dac_gain_reg
				, dac_gain_tgt_buff.dac_gain_tgt_p2 - dac_gain_tgt_buff.dac_gain_tgt_p1);
			++over_run_cnt;

			break;

		case TRIMMING_STEP_DAC_OFS:
			print(LOG_INFO, "=============TRIMMING_STEP_DAC_OFS=============\r\n");
			static float dac_ofs = 0;
			static uint8_t dac_ofs_reg = 0;
			XC24_DAC_OFS_TRIM_INIT();

			ADS114S08_Select_Input_CH(2);
			HAL_Delay(1);
			gb_ads114s08_drdy_done = 0;
			gn_adc_read_count = ADS114S08_READ_COUNT;
			ADS114S08_Set_Start(1);
			while(1)
			{
				if (gb_ads114s08_drdy_done)
				{
					break;
				}
			}

			dac_ofs = (ADS114S_VREF * (((float)(gn_ads114s08_adc_temp) / ADS114S08_READ_COUNT + 0.5f) / 32767)) / 1000; // dacout convert to V
			gn_ads114s08_adc_temp = 0;

			if((over_run_cnt >= DAC_OFS_TRIM_OVER_COUNT) || dac_ofs_reg >= 63 || dac_ofs_reg <= 0)
			{
				print(LOG_ERROR, "DAC_OFS trim error\r\n");
				gt_jig_trimming_step = TRIMMING_STEP_STOP;
				break;
			}

			if(dac_ofs >= DAC_OFS_UPPER_LIMIT)
			{
				--dac_ofs_reg;
			}
			else if (dac_ofs <= DAC_OFS_LOWER_LIMIT)
			{
				++dac_ofs_reg;
			}
			else
			{
				print(LOG_INFO, "Loop[%d] DAC_OFS_REG[%d] -> DAC_OFS_LEVEL : %f\r\n", over_run_cnt, dac_ofs_reg, dac_ofs);
				print(LOG_INFO, "DAC_OFS Trim done\r\n");
				over_run_cnt = 1;
				gt_jig_trimming_step = TRIMMING_STEP_OSC_FCTL;

				break;
			}

			XC24_TRIM_WRITE_DAC_OFS(dac_ofs_reg);
			print(LOG_INFO, "Loop[%d] DAC_OFS_REG[%d] -> DAC_OFS_LEVEL : %f\r\n", over_run_cnt, dac_ofs_reg, dac_ofs);
			++over_run_cnt;

			break;

        case TRIMMING_STEP_OSC_FCTL:
			print(LOG_INFO, "=============TRIMMING_STEP_OSC_FCTL=============\r\n");
			static uint8_t osc_fctl_reg = XC24_OSC_REG_DEFAULT;
			static double osc_freq = 0;
			XC24_OSC_FCTL_TRIM_INIT();

			for(uint8_t reg_index = 0 ; reg_index < 128 ; ++reg_index)
			{
				XC24_TRIM_WRITE_OSC_FCTL(reg_index);
				JigBD_IF_Start_Input_Capture();

				if(gb_timer_input_capture_done == 1)
				{
					JigBD_IF_Stop_Input_Capture();
					osc_freq = JigBD_IF_Get_Input_Capture_Freq();
					osc_freq *= XC24_CONST_FREQ_DIVIDE;
					osc_freq /= CONST_MHz_TO_Hz;
				}

				osc_value_buffer[reg_index] = osc_freq;
				print(LOG_INFO, "Loop[%d] OSC_FREQ REG[%d] -> OSC_FREQ Level : %f\r\n", reg_index, osc_fctl_reg, osc_freq);
			}

			GET_MINIMUM_OSC_FREQ(osc_value_buffer, XC24_OSC_TARGET);
			XC24_TRIM_WRITE_OSC_FCTL(_osc_min.min_gap_index);

			print(LOG_INFO, "OSC_FCTL Trim done\r\n");
			gt_jig_trimming_step = TRIMMING_STEP_E2P_PROGRAM;

			break;

		case TRIMMING_STEP_E2P_PROGRAM:
			print(LOG_INFO, "=============TRIMMING_STEP_E2P_PROGRAM=============\r\n");
			//trim regs save
			for(uint8_t addr = XC24_MIRROR_ADDR_MIRROR1 ; addr < (XC24_MIRROR_ADDR_MIRROR2 + 1) ; ++addr)
			{
				trim_find_regs[addr] = XC24_Read_Register(addr);
			}
			break;

        case TRIMMING_STEP_E2P_PROGRAM_START:
			print(LOG_INFO, "=============TRIMMING_STEP_E2P_PROGRAM_START=============\r\n");
			//set 5.7v & 50mS delay & otp write = 1 & 1Sec delay
			JigBD_IF_XC_VCC_Level(PWR_ON_5V5);
			LL_mDelay(50);
			XC24_Write_Register(XC24_MIRROR_ADDR_OTP_RD_PROG, 1);
			LL_mDelay(1000);

			gt_jig_trimming_step = TRIMMING_STEP_E2P_PROGRAM_END;
			break;

		case TRIMMING_STEP_E2P_PROGRAM_END:
			print(LOG_INFO, "=============TRIMMING_STEP_E2P_PROGRAM_END=============\r\n");
			//reset(off -> on)
			JigBD_IF_XC_VCC_EN(PWR_OFF);
			XC_NSCS_LO();
			XC24_Start_MCLK_Oscillation(FALSE);

			LL_mDelay(100);

			XC24_Trim_Init();

			gt_jig_trimming_step = TRIMMING_STEP_STOP;
			break;

        case TRIMMING_STEP_STOP:
			print(LOG_INFO, "=============TRIMMING_STEP_STOP=============\r\n");
			print(LOG_INFO, "=============TRIMMING_ERROR=============\r\n");
			while(1)
			{

			}
			gt_jig_trimming_step = TRIMMING_STEP_REBOOT;
			break;

        case TRIMMING_STEP_REBOOT:
			print(LOG_INFO, "=============TRIMMING_STEP_REBOOT=============\r\n");

			gt_jig_trimming_step = TRIMMING_STEP_COMPARE;
			break;

		case TRIMMING_STEP_COMPARE:
			print(LOG_INFO, "=============TRIMMING_STEP_COMPARE=============\r\n");
			//find reg compare to saved reg

			if(trim_find_regs[TRIM_FIND_MIRROR1] != XC24_Read_Register(XC24_MIRROR_ADDR_MIRROR1))
			{
				print(LOG_INFO, "TRIMMING_COMPARE MIRROR1 ERROR\r\n");
			}
			else if(trim_find_regs[TRIM_FIND_MIRROR2] != XC24_Read_Register(XC24_MIRROR_ADDR_MIRROR2))
			{
				print(LOG_INFO, "TRIMMING_COMPARE MIRROR2 ERROR\r\n");
			}
			else
			{
				print(LOG_INFO, "TRIMMING_COMPARE OK\r\n");
			}

			gt_jig_trimming_step = TRIMMING_STEP_PWR_OFF;
			break;

		case TRIMMING_STEP_PWR_OFF:
			print(LOG_INFO, "=============TRIMMING_STEP_PWR_OFF\r\n=============");
			//power off
			JigBD_IF_XC_VCC_EN(PWR_OFF);
			break;
        default:
            break;
        }
    }
}

void GET_MINIMUM_OSC_FREQ(float* freq_buffer, float target)
{
	_osc_min.min_gap_freq = freq_buffer[0];
	_osc_min.min_gap_index = 0;
	float min_diff = fabsf(target - freq_buffer[0]);

	for (uint8_t i = 1 ; i < 128 ; ++i)
	{
		float diff = fabsf(target - freq_buffer[i]);
		if (diff < min_diff)
		{
			min_diff = diff;
			_osc_min.min_gap_freq = freq_buffer[i];
			_osc_min.min_gap_index = i;
		}
	}

	print(LOG_INFO, "OSC_FREQ REG[%u] -> OSC_FREQ Level : %f\r\n", _osc_min.min_gap_index, _osc_min.min_gap_freq);
}

/* END - TRIMMING_PROCEDURE_RUN   *****************************************/
/*** end of file ***/
