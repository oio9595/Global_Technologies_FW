
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

#define XC_VCTL_LDO_DEFAULT     (0x08)
#define XC_DAC_GAIN_DEFAULT     (0x20)
#define XC_DAC_OFS_DEFAULT      (0x00)
#define XC_OSC_FCTL_DEFAULT     (0x40)

#define XC24_DAC_SCREEN_GAP     ((0xFFF + 1) / 256 - 1)
#define XC24_DAC_SCREEN_TABLE_SIZE (4)

uint16_t gn_dac_screen_table[XC24_DAC_SCREEN_TABLE_SIZE] = {248, 1241, 2482, 3723};
uint16_t gn_dac_screen_table_idx;

typedef struct
{
    uint8_t min_gap_index;
    float min_gap_freq;
} _closest_info_t;
_closest_info_t gt_xc_osc_closest;
_closest_info_t gt_xc_dac_ofs_closest;

dac_gain_tgt_t dac_gain_tgt_buff = {0, };
float osc_value_buffer[128] = {0, };
float dac_ofs_value_buffer[128] = {0, };
float gf_xc_screen_info[6] = {0.0f, };
uint8_t gn_xc_trim_regs[4] = {0.0f, };
static uint16_t gn_xc_dac_screen_point = 0;

static bool gb_xc_otp_write_flag;

static xc_trim_step_t gt_xc_trim_step;

static void XC_Get_Minimum_DAC_OFS(float* freq_buffer, float target)
{
    gt_xc_dac_ofs_closest.min_gap_index = 0;
    gt_xc_dac_ofs_closest.min_gap_freq = freq_buffer[0];
    float min_diff = fabsf(target - freq_buffer[0]);

    for (uint16_t i = 1 ; i < 256 ; ++i)
    {
        float diff = fabsf(target - freq_buffer[i]);
        if (diff < min_diff)
        {
            min_diff = diff;
            gt_xc_dac_ofs_closest.min_gap_freq = freq_buffer[i];
            gt_xc_dac_ofs_closest.min_gap_index = i;
        }
    }

    print(LOG_INFO, "CLOSEST DAC_OFS REG[%u] -> DAC_OFS Level : %f\r\n", gt_xc_dac_ofs_closest.min_gap_index, gt_xc_dac_ofs_closest.min_gap_freq);
}

static void XC_Get_Minimum_OSC_Freq(float* freq_buffer, float target)
{
    gt_xc_osc_closest.min_gap_index = 0;
    gt_xc_osc_closest.min_gap_freq = freq_buffer[0];
    float min_diff = fabsf(target - freq_buffer[0]);

    for (uint8_t i = 1 ; i < 128 ; ++i)
    {
        float diff = fabsf(target - freq_buffer[i]);
        if (diff < min_diff)
        {
            min_diff = diff;
            gt_xc_osc_closest.min_gap_freq = freq_buffer[i];
            gt_xc_osc_closest.min_gap_index = i;
        }
    }

    print(LOG_INFO, "CLOSEST OSC_FREQ REG[%u] -> OSC_FREQ Level : %f\r\n", gt_xc_osc_closest.min_gap_index, gt_xc_osc_closest.min_gap_freq);
}

void XC_Trim_IF_Trim_Start(void)
{
    if (gt_xc_trim_step == XC_TRIM_STEP_NONE)
    {
        gt_xc_trim_step = XC_TRIM_STEP_ACTIVATE_START;
    }
}

void XC_Trim_IF_Set_OTP_Enable(bool in_flag)
{
    gb_xc_otp_write_flag = in_flag;
}

bool XC_Trim_IF_Get_OTP_Enable(void)
{
    return gb_xc_otp_write_flag;
}

void XC_Trim_Task(void)
{
    static uint8_t over_run_cnt = 1;
    uint16_t ext_adc_value = 0;
    bool xc_compare_result = true;
    bool xc_over_under_flow = false;

    if (gt_xc_trim_step != XC_TRIM_STEP_NONE)
    {
        switch(gt_xc_trim_step)
        {
        case XC_TRIM_STEP_STANDBY:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_STANDBY=============\r\n");
            gt_xc_trim_step = XC_TRIM_STEP_ACTIVATE_START;
        }
        break;

        case XC_TRIM_STEP_ACTIVATE_START:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_ACTIVATE_START=============\r\n");
            gt_xc_trim_step = XC_TRIM_STEP_IC_PWR;
        }
        break;

        case XC_TRIM_STEP_IC_PWR:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_IC_PWR=============\r\n");
            XC24_Trim_Init();
            gt_xc_trim_step = XC_TRIM_STEP_ACTIVATE_END;
        }
        break;

        case XC_TRIM_STEP_ACTIVATE_END:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_ACTIVATE_END=============\r\n");
            gt_xc_trim_step = XC_TRIM_STEP_VCTL_LDO;
        }
        break;

        case XC_TRIM_STEP_VCTL_LDO:
        {
            static uint8_t vctl_ldo_reg = XC_VCTL_LDO_DEFAULT;

            print(LOG_INFO, "=============XC_TRIM_STEP_VCTL_LDO=============\r\n");
            XC24_Trim_Init_VCTL_LDO();

            ADS114S08_Select_Input_CH(ADS114S08_CH_XC_LDO);
            HAL_Delay(1);
            gb_ads114s08_drdy_done = 0;
            gn_ads114s08_adc_temp = 0;
            gn_adc_read_count = ADS114S08_READ_COUNT;
            ADS114S08_Set_Start(1);
            while(1)
            {
                if (gb_ads114s08_drdy_done)
                {
                    break;
                }
            }

            ext_adc_value = ADS114S08_Get_ADC_Value();
            float vctl_ldo_level = (float)(ADC_VOLT_PER_STEP * ext_adc_value) / CONST_mV_TO_V; // Dac out convert to V
            print(LOG_INFO, "Loop[%d] VCTL_LDO_REGS[%d] -> LDO_LEVEL : %.3f\r\n", over_run_cnt, vctl_ldo_reg, vctl_ldo_level);
            ++over_run_cnt;

            if (vctl_ldo_level > VCTL_LDO_UPPER_LIMIT)
            {
                if (vctl_ldo_reg > 0)
                {
                    --vctl_ldo_reg;
                }
                else
                {
                    print(LOG_INFO, "VCTL_LDO Trim underflow\r\n");
                    xc_over_under_flow = true;
                }
            }
            else if (vctl_ldo_level < VCTL_LDO_LOWER_LIMIT)
            {
                if (vctl_ldo_reg < 15)
                {
                    ++vctl_ldo_reg;
                }
                else
                {
                    xc_over_under_flow = true;
                    print(LOG_INFO, "VCTL_LDO Trim overflow\r\n");
                }
            }
            else // in range
            {
                print(LOG_INFO, "VCTL_LDO Trim done\r\n");
                over_run_cnt = 1;
                XC24_Trim_Write_VCTL_LDO(vctl_ldo_reg);
                gn_xc_trim_regs[0] = vctl_ldo_reg;
                gt_xc_trim_step = XC_TRIM_STEP_DAC_GAIN;
                break;
            }

            if (xc_over_under_flow == false)
            {
                XC24_Trim_Write_VCTL_LDO(vctl_ldo_reg);
                gt_xc_trim_step = XC_TRIM_STEP_VCTL_LDO;
            }
            else if ((over_run_cnt >= VCTL_LDO_TRIM_OVER_COUNT) || xc_over_under_flow)
            {
                print(LOG_ERROR, "VCTL_LDO trim error\r\n");
                gt_xc_trim_step = XC_TRIM_STEP_STOP;
            }
        }
        break;

        case XC_TRIM_STEP_DAC_GAIN:
        {
            static uint8_t dac_gain_reg = XC_DAC_GAIN_DEFAULT;

            print(LOG_INFO, "=============XC_TRIM_STEP_DAC_GAIN=============\r\n");
            XC24_Trim_Init_DAC_Gain();

            // Get DAC Gain P1
            XC24_Write_Register(XC24_ADDR_CURRENT_TARGET_DAC, XC24_DAC_GAIN_P1);
            ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC);
            HAL_Delay(1);
            gb_ads114s08_drdy_done = 0;
            gn_ads114s08_adc_temp = 0;
            gn_adc_read_count = ADS114S08_READ_COUNT;
            ADS114S08_Set_Start(1);
            while(1)
            {
                if (gb_ads114s08_drdy_done)
                {
                    break;
                }
            }
            ext_adc_value = ADS114S08_Get_ADC_Value();
            dac_gain_tgt_buff.dac_gain_tgt_p1 = (float)(ADC_VOLT_PER_STEP * ext_adc_value) / CONST_mV_TO_V;

            // Get DAC Gain P2
            XC24_Write_Register(XC24_ADDR_CURRENT_TARGET_DAC, XC24_DAC_GAIN_P2);
            ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC);
            HAL_Delay(1);

            gb_ads114s08_drdy_done = 0;
            gn_ads114s08_adc_temp = 0;
            gn_adc_read_count = ADS114S08_READ_COUNT;
            ADS114S08_Set_Start(1);
            while(1)
            {
                if (gb_ads114s08_drdy_done)
                {
                    break;
                }
            }
            ext_adc_value = ADS114S08_Get_ADC_Value();
            dac_gain_tgt_buff.dac_gain_tgt_p2 = (float)(ADC_VOLT_PER_STEP * ext_adc_value) / CONST_mV_TO_V;

            float dac_gain_delta = dac_gain_tgt_buff.dac_gain_tgt_p2 - dac_gain_tgt_buff.dac_gain_tgt_p1;
            print(LOG_INFO, "Loop[%d] DAC_GAIN_REG[%d] -> DAC_GAIN_LEVEL : %.3f\r\n" , over_run_cnt, dac_gain_reg, dac_gain_delta);
            ++over_run_cnt;

            if(dac_gain_delta > DAC_GAIN_UPPER_LIMIT)
            {
                if (dac_gain_reg > 0)
                {
                    --dac_gain_reg;
                }
                else
                {
                    print(LOG_INFO, "DAC_GAIN Trim underflow\r\n");
                    xc_over_under_flow = true;
                }
            }
            else if (dac_gain_delta < DAC_GAIN_LOWER_LIMIT)
            {
                if (dac_gain_reg < 63)
                {
                    ++dac_gain_reg;
                }
                else
                {
                    xc_over_under_flow = true;
                    print(LOG_INFO, "DAC_GAIN Trim overflow\r\n");
                }
            }
            else
            {
                print(LOG_INFO, "DAC_GAIN Trim done\r\n");
                over_run_cnt = 1;
                XC24_Trim_Write_DAC_GAIN(dac_gain_reg);
                gn_xc_trim_regs [1] = dac_gain_reg;
                gt_xc_trim_step = XC_TRIM_STEP_DAC_OFS;
                break;
            }
            if (xc_over_under_flow == false)
            {
                XC24_Trim_Write_DAC_GAIN(dac_gain_reg);
                gt_xc_trim_step = XC_TRIM_STEP_DAC_GAIN;
            }
            else if ((over_run_cnt >= DAC_GAIN_TRIM_OVER_COUNT) || xc_over_under_flow)
            {
                print(LOG_ERROR, "DAC_GAIN trim error\r\n");
                gt_xc_trim_step = XC_TRIM_STEP_STOP;
                break;
            }
        }
        break;

        case XC_TRIM_STEP_DAC_OFS:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_DAC_OFS=============\r\n");
            XC24_Trim_Init_DAC_OFS();
            for (uint16_t i = 0 ; i < 0x100 ; ++i)
            {
                XC24_Trim_Write_DAC_OFS(i);
                ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC);
                HAL_Delay(1);
                gb_ads114s08_drdy_done = 0;
                gn_ads114s08_adc_temp = 0;
                gn_adc_read_count = ADS114S08_READ_COUNT;
                ADS114S08_Set_Start(1);
                while(1)
                {
                    if (gb_ads114s08_drdy_done)
                    {
                        break;
                    }
                }

                ext_adc_value = ADS114S08_Get_ADC_Value();
                float dac_ofs = (float)(ADC_VOLT_PER_STEP * ext_adc_value) / CONST_mV_TO_V;
                print(LOG_INFO, "Loop[%d] DAC_OFS_LEVEL : %.5f\r\n", i, dac_ofs);
                dac_ofs_value_buffer[i] = dac_ofs;
            }
            XC_Get_Minimum_DAC_OFS(dac_ofs_value_buffer, XC24_DAC_OFS_TARGET);
            XC24_Trim_Write_DAC_OFS(gt_xc_dac_ofs_closest.min_gap_index);
            gn_xc_trim_regs[2] = gt_xc_dac_ofs_closest.min_gap_index;
            gt_xc_trim_step = XC_TRIM_STEP_OSC_FCTL;
        }
        break;

        case XC_TRIM_STEP_OSC_FCTL:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_OSC_FCTL=============\r\n");
            XC24_Trim_Init_OSC();
            uint16_t val = XC24_Read_Register(XC24_ADDR_CLK_CONTROL_1);
            print(LOG_INFO, "0x1B -> %u (0x%04X)\r\n", val, val);

            for(uint8_t reg_index = 0 ; reg_index < 128 ; ++reg_index)
            {
                XC24_Trim_Write_OSC_FCTL(reg_index);
                HAL_Delay(1);
                JigBD_IF_Start_Input_Capture();

                while(1)
                {
                    if (gb_timer_input_capture_done)
                    {
                        break;
                    }
                }

                JigBD_IF_Stop_Input_Capture();
                float osc_freq = JigBD_IF_Get_Input_Capture_Freq() * XC24_CONST_FREQ_DIVIDE / CONST_MHz_TO_Hz;
                osc_value_buffer[reg_index] = osc_freq;
                print(LOG_INFO, "%u, %.3f\r\n", reg_index, osc_freq);
                ++over_run_cnt;
            }

            XC_Get_Minimum_OSC_Freq(osc_value_buffer, XC24_OSC_TARGET);
            XC24_Trim_Write_OSC_FCTL(gt_xc_osc_closest.min_gap_index);
            gn_xc_trim_regs[3] = gt_xc_osc_closest.min_gap_index;

            print(LOG_INFO, "OSC_FCTL Trim done\r\n");
            gt_xc_trim_step = XC_TRIM_STEP_E2P_PROGRAM;
        }
        break;

        case XC_TRIM_STEP_E2P_PROGRAM:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_E2P_PROGRAM=============\r\n");
            //trim regs save
            if (gb_xc_otp_write_flag)
            {
                gt_xc_trim_step = XC_TRIM_STEP_E2P_PROGRAM_START;
            }
            else
            {
                gt_xc_trim_step = XC_TRIM_STEP_SCREEN_LDO;
            }
        }
        break;

        case XC_TRIM_STEP_E2P_PROGRAM_START:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_E2P_PROGRAM_START=============\r\n");
            //set 5.7v & 50mS delay & otp write = 1 & 1Sec delay
            JigBD_IF_XC_VCC_Level(PWR_ON_5V5);
            LL_mDelay(50);
            XC24_Write_Register(XC24_MIRROR_ADDR_OTP_RD_PROG, 1);
            LL_mDelay(1000);

            gt_xc_trim_step = XC_TRIM_STEP_SCREEN_LDO;
        }
        break;

        case XC_TRIM_STEP_E2P_PROGRAM_END:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_E2P_PROGRAM_END=============\r\n");

            JigBD_IF_XC_VCC_EN(PWR_OFF);

            LL_mDelay(100);

            gt_xc_trim_step = XC_TRIM_STEP_REBOOT;
        }
        break;

        case XC_TRIM_STEP_REBOOT:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_REBOOT=============\r\n");
            JigBD_IF_XC_VCC_Level(PWR_ON_5V0);
            XC24_Trim_Init();
            XC24_Write_Register(XC24_MIRROR_ADDR_OTP_RD_PROG, 2);
            HAL_Delay(10);

            gt_xc_trim_step = XC_TRIM_STEP_SCREEN_LDO;
        }
        break;

        case XC_TRIM_STEP_SCREEN_LDO:
        {
            ADS114S08_Select_Input_CH(ADS114S08_CH_XC_LDO);
            HAL_Delay(1);
            gb_ads114s08_drdy_done = 0;
            gn_ads114s08_adc_temp = 0;
            gn_adc_read_count = ADS114S08_READ_COUNT;
            ADS114S08_Set_Start(1);
            while(1)
            {
                if (gb_ads114s08_drdy_done)
                {
                    break;
                }
            }

            ext_adc_value = ADS114S08_Get_ADC_Value();
            float vctl_ldo_level = (float)(ADC_VOLT_PER_STEP * ext_adc_value) / CONST_mV_TO_V; // Dac out convert to V
            gf_xc_screen_info[0] = vctl_ldo_level; // Save VCTL_LDO level
            print(LOG_INFO, "Screen  LDO_ADC [%u] -> LDO_LEVEL : %.3f\r\n", ext_adc_value, vctl_ldo_level);
            gt_xc_trim_step = XC_TRIM_STEP_SCREEN_DAC;
        }
        break;

        case XC_TRIM_STEP_SCREEN_DAC:
        {
            XC24_Trim_Init_DAC_Gain();
#if 0
            XC24_Write_Register(XC24_ADDR_CURRENT_TARGET_DAC, gn_xc_dac_screen_point);
#else
            XC24_Write_Register(XC24_ADDR_CURRENT_TARGET_DAC, gn_dac_screen_table[gn_dac_screen_table_idx]);
#endif

            ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC);
            HAL_Delay(1);
            gb_ads114s08_drdy_done = 0;
            gn_ads114s08_adc_temp = 0;
            gn_adc_read_count = ADS114S08_READ_COUNT;
            ADS114S08_Set_Start(1);
            while(1)
            {
                if (gb_ads114s08_drdy_done)
                {
                    break;
                }
            }

            ext_adc_value = ADS114S08_Get_ADC_Value();
            float dac_val = (float)(ADC_VOLT_PER_STEP * ext_adc_value) / CONST_mV_TO_V;
            print(LOG_INFO, "%u, %.4f\r\n", gn_dac_screen_table[gn_dac_screen_table_idx], dac_val);
            ++gn_dac_screen_table_idx;
            gf_xc_screen_info[gn_dac_screen_table_idx] = dac_val; // Save DAC_GAIN level

            if (gn_dac_screen_table_idx >= XC24_DAC_SCREEN_TABLE_SIZE)
            {
                gn_dac_screen_table_idx = 0;
                gt_xc_trim_step = XC_TRIM_STEP_SCREEN_OSC;
            }

#if 0
            if (gn_xc_dac_screen_point == 0)
            {
                print(LOG_INFO, "INPUT, DAC_LEVEL\r\n");
            }
            print(LOG_INFO, "%u, %.4f\r\n", gn_xc_dac_screen_point, dac_val);
            gn_xc_dac_screen_point += XC24_DAC_SCREEN_GAP;
            if (gn_xc_dac_screen_point > 4095)
            {
                XC24_Write_Register(XC24_ADDR_CURRENT_TARGET_DAC, XC24_DAC_OFS_TGT);

                ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC);
                HAL_Delay(1);
                gb_ads114s08_drdy_done = 0;
                gn_ads114s08_adc_temp = 0;
                gn_adc_read_count = ADS114S08_READ_COUNT;
                ADS114S08_Set_Start(1);
                while(1)
                {
                    if (gb_ads114s08_drdy_done)
                    {
                        break;
                    }
                }
                ext_adc_value = ADS114S08_Get_ADC_Value();
                float dac_val = (float)(ADC_VOLT_PER_STEP * ext_adc_value) / CONST_mV_TO_V;
                gf_xc_screen_info[1] = dac_val; // Save DAC_GAIN level
                print(LOG_INFO, "%u, %.4f\r\n", XC24_DAC_OFS_TGT, dac_val);

                gt_xc_trim_step = XC_TRIM_STEP_SCREEN_OSC;
            }
#endif
        }
        break;

        case XC_TRIM_STEP_SCREEN_OSC:
        {
            XC24_Trim_Init_OSC();
            HAL_Delay(1);
            JigBD_IF_Start_Input_Capture();

            while(1)
            {
                if (gb_timer_input_capture_done)
                {
                    break;
                }
            }

            JigBD_IF_Stop_Input_Capture();
            float osc_freq = JigBD_IF_Get_Input_Capture_Freq() * XC24_CONST_FREQ_DIVIDE / CONST_MHz_TO_Hz;
            gf_xc_screen_info[5] = osc_freq; // Save OSC_FCTL level
            print(LOG_INFO, "Screen  OSC : %.3f\r\n", osc_freq);
            gt_xc_trim_step = XC_TRIM_STEP_PWR_OFF;
        }
        break;

        case XC_TRIM_STEP_STOP:
        {
            print(LOG_ERROR, "=============TRIMMING_ERROR=============\r\n");
            gt_xc_trim_step = XC_TRIM_STEP_PWR_OFF;
        }
        break;

        case XC_TRIM_STEP_PWR_OFF:
        {
            print(LOG_INFO, "=============XC_TRIM_STEP_PWR_OFF=============\r\n");
            print(LOG_INFO, "%.3f, %.3f, %.3f, %.3f, %.3f, %.3f\r\n", gf_xc_screen_info[0], gf_xc_screen_info[1], gf_xc_screen_info[2], gf_xc_screen_info[3], gf_xc_screen_info[4], gf_xc_screen_info[5]);
            if (gf_xc_screen_info[1] < 0.195f || gf_xc_screen_info[1] > 0.205f)
            {
                print(LOG_ERROR, "NG\r\n");
            }
            else
            {
                print(LOG_INFO, "OK\r\n");
            }
            //print(LOG_INFO, "%u, %u, %u, %u\r\n", gn_xc_trim_regs[0], gn_xc_trim_regs[1], gn_xc_trim_regs[2], gn_xc_trim_regs[3]);
            //JigBD_IF_XC_VCC_EN(PWR_OFF);
            gt_xc_trim_step = XC_TRIM_STEP_NONE;
        }
        break;

        default:
        {

        }
        break;
        }
    }
}
/*** end of file ***/