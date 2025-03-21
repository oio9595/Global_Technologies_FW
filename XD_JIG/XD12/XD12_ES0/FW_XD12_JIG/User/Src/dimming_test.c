/** @file dimming_test.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __DIMMING_TEST_C__
#include "config.h"

#define DIMMING_TEST_SUB_FRAME_NUM  128
#define MCLK 14750000 // 14.75MHz
//#define MCLK                        14784000 // 14.75MHz

#define ROW_NUM                     12

#define PWM_LD_WIDTH_3_SIZE  (0xFFFF)

//#define DEVIATION_REF_AVG
#define DEVIATION_REF_SPEC

/* Common */
static float gf_mclk;
static float gf_vsync_out;
static uint8_t gn_xd12_dimming_init_done;
static uint8_t gn_xd12_fault_test_run_flag;


/* Type2 Only */
static uint16_t gn_ld_width;
static uint16_t gn_xd12_pwm;
static uint16_t gn_delay_size;
static uint16_t gn_delay_period;

static dimming_step_t gt_jig_dimming_step;
static uint8_t gn_dimming_step_delay;

static uint8_t gn_dimming_test_output_num;
static uint32_t gn_dimming_LD_data;
static uint16_t gn_dimming_adc[12];
static uint16_t gn_dimming_adc_offset[12];
static float gf_dimming_curr[12];
static float gf_dimming_curr_avg;
static float gf_dimming_curr_spec;

static current_gain_t gn_dimming_gain;

static uint16_t gn_xd12_max_vref;
static float gf_xd12_max_current;

static uint32_t gn_dimming_LD_data_max_size;

static uint8_t gn_dimming_LD_tx_done_flag;
static uint16_t gn_dimming_ch_change_done_flag;

//#define USE_LDIM_TABLE

#ifdef USE_LDIM_TABLE
static uint16_t gn_dimming_LD_table[] = 
{
        0,    82,   164,   246,   328,   410,   820,  1157,  1639,  2458, 
     3277,  4096,  4916,  5735,  6554,  7373,  8192, 12288, 14390, 16384, 
    24576, 32768, 40960, 49152, 57344, 65535
};
static uint8_t gn_dimming_LD_table_cnt;
#endif

void XD12_set_dimming_gain(current_gain_t in_gain)
{
    gn_dimming_gain = in_gain;
}

current_gain_t XD12_get_dimming_gain()
{
    return gn_dimming_gain;
}

void XD12_set_ldim(uint32_t in_ldim)
{
    gn_dimming_LD_data = in_ldim;
}

uint32_t XD12_get_ldim()
{
    return gn_dimming_LD_data;
}

static uint8_t XD12_get_LD_trans_done_flag()
{
    return gn_dimming_LD_tx_done_flag;
}

void XD12_set_LD_tx_done_flag(uint8_t is_done)
{
    if(is_done)
    {
        gn_dimming_LD_tx_done_flag = 1;
    }
    else
    {
        gn_dimming_LD_tx_done_flag = 0;
    }
}

uint16_t XD12_get_ch_change_done_flag()
{
    return gn_dimming_ch_change_done_flag;
}

void XD12_set_ch_change_done_flag(uint16_t is_done)
{
    gn_dimming_ch_change_done_flag = is_done;
}

static void XD12_reg_value_init()
{
    gf_mclk = MCLK;     //Hz
    gf_vsync_out = 100; //Hz
    gn_ld_width = 3;
    gn_delay_size = (ROW_NUM - 1) * 4;
    gn_delay_period = (uint16_t)(gf_mclk/(ROW_NUM * gf_vsync_out * 4) + 0.5);
}

void XD12_dimming_test_init()
{    
    print("\r\nVCC 5V\r\n");
    JigBd_IF_5_0V_power(PWR_ON_5_0V);
    print("\r\nVCC Enable\r\n");
    JigBd_IF_VCC_EN_power(PWR_ON);
    print("\r\nVLED Enable\r\n");
    JigBd_IF_Vled_9V_power(PWR_ON);
    HAL_Delay(10);
    
    JigBd_IF_xd12_reset_Command();
    us_tdelay(5);
    
    JigBd_IF_IdGen_Command();
    TargetIC_IF_Fault_Level_Init();
    TargetIC_IF_Serial_Clock_Init();
    TargetIC_IF_Temp_Level_Init();
    TargetIC_IF_MAX_CURR_VREF_Init();
    TargetIC_IF_SHORT_ENABLE_LEVEL_Init();
    
    JigBd_IF_current_discharge(CHARGE);
    TargetIC_IF_CHANNEL_ENABLE_Set(CH_01);
    JigBd_IF_change_current_gain(GAIN_HIGH);
    JigBd_IF_Select_Output_Ch(0);

    XD12_reg_value_init();
    TargetIC_IF_LD_CONTROL_INIT();
    XD12_Write_DELAY_SIZE(gn_delay_size);
    XD12_Write_DELAY_PERIOD(gn_delay_period);
    gn_xd12_dimming_init_done = 1;
}

uint8_t XD12_get_dimming_init_done_flag()
{
    return gn_xd12_dimming_init_done;
}

void XD12_tx_LD_Data(uint16_t in_pwm)
{
    if (XD12_get_dimming_init_done_flag())
    {
        JigBd_IF_LD_Trans_Command_through_PWM_to_XD12(gn_ld_width, gn_xd12_pwm);
    }    
}

void XD12_set_PWM(uint32_t in_pwm_val)
{
    if (in_pwm_val <= 0xFFFF)
    {
        gn_xd12_pwm = in_pwm_val;
    }
    else
    {
        print("\r\ninvalid value PWM - 0 ~ 65535 (0xFFFF) [15:0]\r\n");
    }
}

uint16_t XD12_get_PWM()
{
    return gn_xd12_pwm;
}

static uint32_t XD12_get_LD_data_max_size(uint16_t in_ld_width)
{
    uint32_t ret_ld_size = 0;        
    ret_ld_size = PWM_LD_WIDTH_3_SIZE;
    
    return ret_ld_size;
}

uint8_t XD12_get_fault_test_run_flag()
{
    return gn_xd12_fault_test_run_flag;
}

void XD12_stop_fault_test()
{
    gn_xd12_fault_test_run_flag = 0;
    TargetIC_IF_CHANNEL_ENABLE_ResetAll();
}

void XD12_set_short_level(uint8_t in_short_level)
{
    JigBd_IF_Vled_9V_power(PWR_OFF);
    //TargetIC_IF_CHANNEL_ENABLE_SetAll();
    TargetIC_IF_CHANNEL_ENABLE_Set(0);
    TargetIC_IF_SHORT_ENABLE_Set(0);
    JigBd_IF_Select_Output_Ch(15);
    XD12_set_PWM(1920);
    
    switch (in_short_level)
    {
        case  2 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_2V, FB_LEVEL_0V6);
        break;
        
        case  4 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_4V, FB_LEVEL_0V6);
        break;
        
        case  7 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_7V, FB_LEVEL_0V6);
        break;
        
        case 12 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_12V, FB_LEVEL_0V6);
        break;
        
        case 21 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_21V, FB_LEVEL_0V6);
        break;
        
        case 24 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_24V, FB_LEVEL_0V6);
        break;
        
        case 28 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_28V, FB_LEVEL_0V6);
        break;
        
        case 38 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_38V, FB_LEVEL_0V6);
        break;
        
        default : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_38V, FB_LEVEL_0V6);
        break;
    }
    gn_xd12_fault_test_run_flag = 1;
}

void XD12_set_FB_level(uint8_t in_fb_level)
{    
    JigBd_IF_Vled_9V_power(PWR_OFF);
    //TargetIC_IF_CHANNEL_ENABLE_SetAll();
    TargetIC_IF_CHANNEL_ENABLE_Set(0);
    JigBd_IF_Select_Output_Ch(15);
    
    switch (in_fb_level)
    {
        case 0 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_38V, FB_LEVEL_0V6);
        break;
        
        case 1 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_38V, FB_LEVEL_0V8);
        break;
        
        case 2 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_38V, FB_LEVEL_1V0);
        break;
        
        case 3 : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_38V, FB_LEVEL_1V2);
        break;
        
        default : 
            TargetIC_IF_FAULT_LEVEL_Set(DEV_MAX_CURR_LEVEL_8mA, SHORT_LEVEL_38V, FB_LEVEL_1V2);
        break;
    }
    gn_xd12_fault_test_run_flag = 1;
}

void XD12_get_fault_status()
{
    uint16_t value = 0;
    value = (JigBd_IF_Read_Command(XD12_ADDR_FAULT_STATUS) & 0xF);

    if (!value)
    {
        print("\r\nFAULT Nothing\r\n");
    }
    else
    {
        if (((value >> 0) & 0x01) == 1)
        {   
            print("\r\nFAULT FB Detect\r\n");
        }
        if (((value >> 1) & 0x01) == 1)
        {
            print("\r\nFAULT OPEN Detect\r\n");
        }
        if (((value >> 2) & 0x01) == 1)
        {
            print("\r\nFAULT SHORT Detect\r\n");
        }
        if (((value >> 3) & 0x01) == 1)
        {
            print("\r\nFAULT THERMAL Detect\r\n");
        }
    }
}

static double XD12_convert_adc_to_current(uint16_t adc, current_gain_t gain)
{
	double ret = 0;
	switch (gain)
	{
		case GAIN_HIGH : 
			ret = ((double)adc * ADC_CONV_COEFF_HIGH);	/* max  25mA */	
			break;
		
		case GAIN_MID : 
			ret = ((double)adc * ADC_CONV_COEFF_MID);	/* max  10mA */
			break;
		
		case GAIN_LOW : 
			ret = ((double)adc * ADC_CONV_COEFF_LOW);	/* max 0.3mA */
			break;
	}
	return ret; //mA
}

static double XD12_calculate_deviation(float in_iout, float in_iout_ref)
{
	double ret = 0;
    ret = 100 * ((in_iout / in_iout_ref) - 1);  // unit : %
	return ret; 
}

void XD12_start_dimming_test()
{
    if (gt_jig_dimming_step == DIMMING_STEP_NONE)
    {
        gt_jig_dimming_step = DIMMING_STEP_TEST_INIT;
    }
}

__STATIC_INLINE void XD12_get_max_current()
{
    switch ((XD12_Get_REGISTER(XD12_ADDR_FAULT_LEVEL) & 0x700) >> 8)
    {
        case 0 : 
            gf_xd12_max_current = 4.0f; //4mA
            break;
        case 1 : 
            gf_xd12_max_current = 8.0f; //8mA
            break;
        case 2 : 
            gf_xd12_max_current = 16.0f; //16mA
            break;
        case 3 : 
            gf_xd12_max_current = 24.0f; //24mA
            break;
        case 4 : 
            gf_xd12_max_current = 32.0f; //32mA
            break;
        case 5 : 
            gf_xd12_max_current = 64.0f; //64mA
            break;
    }
}

void dimming_procedure_run()
{
    switch (gt_jig_dimming_step)
    {
/* DIMMING_STEP_TEST_INIT  ***************************************/
        case DIMMING_STEP_TEST_INIT : 
            JigBd_IF_5_0V_power(PWR_ON_5_0V);
            JigBd_IF_VCC_EN_power(PWR_ON);
            JigBd_IF_Vled_9V_power(PWR_ON);
            
            JigBd_IF_current_discharge(CHARGE);
            JigBd_IF_Select_Output_Ch(15);
            
#ifdef USE_LDIM_TABLE
            gn_dimming_LD_data = gn_dimming_LD_table[gn_dimming_LD_table_cnt];
#else
            gn_dimming_LD_data = 0;
#endif
            
            gn_dimming_step_delay = 10;
            gt_jig_dimming_step = DIMMING_STEP_INIT_PARAM;
        break;
        
/* DIMMING_STEP_INIT_PARAM  ***************************************/
        case DIMMING_STEP_INIT_PARAM : 
            if (gn_dimming_step_delay)
            {
                --gn_dimming_step_delay;
            }
            else
            {
                /* XD12 init */                
                JigBd_IF_xd12_reset_Command();
                us_tdelay(5);
                
                JigBd_IF_IdGen_Command();
                TargetIC_IF_Fault_Level_Init();
                TargetIC_IF_Serial_Clock_Init();
                TargetIC_IF_Temp_Level_Init();
                TargetIC_IF_MAX_CURR_VREF_Init();
                TargetIC_IF_SHORT_ENABLE_LEVEL_Init();
                TargetIC_IF_CHANNEL_ENABLE_SetAll();
                TargetIC_IF_TRIM_REG_INIT();
                
                XD12_reg_value_init();
                gn_xd12_dimming_init_done = 1; 
                
                TargetIC_IF_LD_CONTROL_INIT();
                XD12_Write_DELAY_SIZE(gn_delay_size);
                XD12_Write_DELAY_PERIOD(gn_delay_period);
                
                XD12_set_PWM(0);
                TargetIC_IF_Read_Resgister_All(0);
                gn_xd12_max_vref = JigBd_IF_Read_Command(XD12_ADDR_MAX_CURR_VREF);
                print("max_vref:%4u\r\n", gn_xd12_max_vref);
                
                XD12_get_max_current();
                print("max_current:%3.0f:mA\r\n", gf_xd12_max_current);
                
                gn_dimming_LD_data_max_size = XD12_get_LD_data_max_size(gn_ld_width);
                print("%s data:     Io_1:     Io_2:     Io_3:     Io_4:     Io_5:     Io_6:     Io_7:     Io_8:     Io_9:    Io_10:    Io_11:    Io_12:  Io_spec:   Io_avg:   Dev_1:   Dev_2:   Dev_3:   Dev_4:   Dev_5:   Dev_6:   Dev_7:   Dev_8:   Dev_9:  Dev_10:  Dev_11:  Dev_12\r\n", CMD_XDIC_GDIM);
                
                gn_dimming_step_delay = 5;
                gt_jig_dimming_step = DIMMING_STEP_SET_LD_DATA;
            }
        break;

/* DIMMING_STEP_SET_LD_DATA  ***************************************/
        case DIMMING_STEP_SET_LD_DATA : 
            if (gn_dimming_step_delay)
            {
                --gn_dimming_step_delay;
            }
            else
            {
                gf_dimming_curr_spec = (float)(gf_xd12_max_current * (float)gn_xd12_max_vref / 4095) * ((float)gn_dimming_LD_data / 0xFFFF);
#if 1
                if (gf_dimming_curr_spec < 0.3)
                {
                    gn_dimming_gain = GAIN_LOW;
                }
                else
                {
                    gn_dimming_gain = GAIN_MID;
                }
#endif
                JigBd_IF_change_current_gain(gn_dimming_gain);                
                JigBd_IF_Select_Output_Ch(gn_dimming_test_output_num);                
                XD12_set_ch_change_done_flag(1 << gn_dimming_test_output_num);
                
                XD12_set_PWM(gn_dimming_LD_data);
                gn_dimming_step_delay = 51;
                gt_jig_dimming_step = DIMMING_STEP_START_ADC;
            }
        break;
        
/* DIMMING_STEP_START_ADC  ***************************************/
        case DIMMING_STEP_START_ADC : 
            if (gn_dimming_step_delay)
            {
                --gn_dimming_step_delay;
            }
            else
            {
                if(XD12_get_LD_trans_done_flag())
                {
                    XD12_set_LD_tx_done_flag(0);
                    gb_ads114s08_drdy_done = 0;
                    gn_ads114s08_adc_temp = 0;
                    gn_adc_read_count = ADS114S08_READ_COUNT;

                    ads114s08_select_single_ended_input(0);
                    ads114s08_set_start(1);
                    gt_jig_dimming_step = DIMMING_STEP_GET_ADC;
                }
            }
        break;
        
/* DIMMING_STEP_GET_ADC  ***************************************/
        case DIMMING_STEP_GET_ADC : 
            if(gb_ads114s08_drdy_done == 1)
            {
                gn_dimming_adc[gn_dimming_test_output_num] = get_adc_value();
                ++gn_dimming_test_output_num;
                gb_ads114s08_drdy_done = 0;
                gt_jig_dimming_step = DIMMING_STEP_ALL_CHANNEL_IS_DONE;
                XD12_set_PWM(0);
            }
        break;
        
/* DIMMING_STEP_ALL_CHANNEL_IS_DONE  ***************************************/
        case DIMMING_STEP_ALL_CHANNEL_IS_DONE : 
            if (gn_dimming_test_output_num < 12)
            {
                gn_dimming_step_delay = 31;
                gt_jig_dimming_step = DIMMING_STEP_SET_LD_DATA;
            }
            else
            {
                gn_dimming_step_delay = 21;
                gn_dimming_test_output_num = 0;
                gt_jig_dimming_step = DIMMING_STEP_LOG;
            }
        break;
        
/* DIMMING_STEP_LOG  ***************************************/
        case DIMMING_STEP_LOG : 
            if(1)
            {
                int sz = 0;
                uint8_t ch = 0;
                char temp[512] = {0, };
                
                print("%s%5d", CMD_XDIC_GDIM, gn_dimming_LD_data);
                if (gn_dimming_LD_data == 0)
                {
                    for (uint8_t ch = 0 ; ch < 12 ; ++ch)
                    {
                        gn_dimming_adc_offset[ch] = gn_dimming_adc[ch];
                    }
                }
                
                for (ch = 0 ; ch < CH_MAX ; ++ch)
                {
                    if (gn_dimming_gain == GAIN_LOW)
                    {
                        gn_dimming_adc[ch] -= gn_dimming_adc_offset[ch];
                    }
                }
                
                for (ch = 0 ; ch < CH_MAX ; ++ch)
                {
                    gf_dimming_curr[ch] = XD12_convert_adc_to_current(gn_dimming_adc[ch], gn_dimming_gain);
                    gf_dimming_curr_avg += (gf_dimming_curr[ch] / CH_MAX);
                    
                    sz += snprintf(temp + sz, sizeof temp, ":%9.5f", gf_dimming_curr[ch]);
                }
                sz += snprintf(temp + sz, sizeof temp, ":%9.3lf", gf_dimming_curr_spec);
                sz += snprintf(temp + sz, sizeof temp, ":%9.3lf", gf_dimming_curr_avg);
                
                for (ch = 0 ; ch < CH_MAX ; ++ch)
                {
#ifdef DEVIATION_REF_SPEC
                    sz += snprintf(temp + sz, sizeof temp, ":%8.2lf", XD12_calculate_deviation(gf_dimming_curr[ch], gf_dimming_curr_spec));
#else
                    sz += snprintf(temp + sz, sizeof temp, ":%8.2lf", XD12_calculate_deviation(gf_dimming_curr[ch], gf_dimming_curr_avg));
#endif
                }
                print("%s\r\n", temp);
                gf_dimming_curr_avg = 0.0f;
            }

            
            if(gn_dimming_LD_data < gn_dimming_LD_data_max_size)
            {
#ifdef USE_LDIM_TABLE
                ++gn_dimming_LD_table_cnt;
                gn_dimming_LD_data = gn_dimming_LD_table[gn_dimming_LD_table_cnt];
#else
                
                if (gn_dimming_LD_data < 4096)
                {
                    gn_dimming_LD_data += 41;
                }
                else
                {
                    gn_dimming_LD_data += 819;
                }
                
#endif
                if (gn_dimming_LD_data > gn_dimming_LD_data_max_size)
                {
                    gn_dimming_LD_data = gn_dimming_LD_data_max_size;
                }
                gt_jig_dimming_step = DIMMING_STEP_SET_LD_DATA;
            }
            
            else
            {
                gt_jig_dimming_step = DIMMING_STEP_POWER_OFF;
            }
        break;
        
/* DIMMING_STEP_POWER_OFF  ***************************************/
        case DIMMING_STEP_POWER_OFF : 
            gn_dimming_LD_data = 0;
            gn_dimming_test_output_num = 0;
        
#ifdef USE_LDIM_TABLE
            gn_dimming_LD_table_cnt = 0;
#endif
            XD12_set_PWM(0);

            JigBd_IF_Vled_9V_power(PWR_OFF);
            JigBd_IF_VCC_EN_power(PWR_OFF);
            print("Dimming Test Done\r\n");
            gt_jig_dimming_step = DIMMING_STEP_NONE;
        break;
        
        default : 
        break;
    }
}

/*** end of file ***/
