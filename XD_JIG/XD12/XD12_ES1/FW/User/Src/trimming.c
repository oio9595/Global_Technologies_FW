
/** @file trimming.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __APIC_TRIMMING_C__
#include "config.h"

//#define TEST_IOUT_DELTA_COUNT   (20)

//#define ADC_REF_VOLTAGE     (5000)
//#define ADC_RESOLUTION      (32768-1)

#define ADJ_NONE    0
#define ADJ_PLUS    1
#define ADJ_MINUS   2
#define ADJ_DEFAULT 3

#define TRIM_REGISTER_SAVED_CNT  2
#define TRIM_OUT_RANGE_CNT  50

#ifdef CHK_TRIM_TIME
static uint32_t g_CHK_TRIM_TIME_tick;
static uint32_t g_CHK_TRIM_TIME_INIT;
static uint32_t g_CHK_TRIM_TIME_TRIM;
static uint32_t g_CHK_TRIM_TIME_OTP;
#endif

static volatile bool gb_GUI_ACTIVATED;
static volatile bool gb_GUI_REG_APPLY_FOR_TRIM;

static volatile bool gub_OTP_WRITE_ENABLE;
static volatile bool gub_WITHOUT_SCREEN;
static volatile bool gub_NO_TRIM_ENABLE;

static trimming_step_t gt_jig_trimming_step;

static trim_mode_t gt_trim_search_mode;
static uint8_t gn_trim_error_code;

static uint8_t gn_read_adc_vout_channel;

static uint16_t gn_step_delay;

static uint8_t gn_slope_cnt;
static uint16_t gn_slope_adc[CH_MAX][2];

static uint16_t gn_screen_adc[CH_MAX];

static double screen_iout_spec;
static double screen_iout_spec_max_curr;
static double screen_iout_spec_max_curr_vref;
static double screen_dev[CH_MAX];
static double screen_current[CH_MAX];

#ifdef __APIC_TRIMMING_C__ 
static const char* gstr_TRIM_MODE[TRIM_MAX]= 
{
    "TRIM_OSC_FREQUENCY",
    "TRIM_VREF_CTL",
    "TRIM_OFS_CHS",
    "TRIM_GAIN_CHS",
    "TRIM_SCREENING"
};
#endif

typedef struct
{
    uint16_t u16_saved_regi;
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
    _sTrimRange sTrimRange[TRIM_MAX]; //Input value - TRIMMING_STEP_TRIM_START
    trim_mode_t trim_mode; //Input value - TRIMMING_STEP_CHECK
    uint16_t u16_init_adc_per_regi; //Input value - TRIMMING_STEP_CHECK
    uint8_t u8_channel_cur; //Input value - TRIMMING_STEP_CHECK
    uint8_t u8_channel_max; //Input value - TRIMMING_STEP_CHECK
    uint8_t u8_loop_cnt;
    uint8_t u8_sTrimSaved_Cnt;
    uint16_t adc_cur[CH_MAX]; 
    uint16_t adc_pre[CH_MAX];     
    double current[CH_MAX];
    _sTrimSaved sTrimSaved[TRIM_REGISTER_SAVED_CNT];
    uint16_t trim_step[CH_MAX]; // Result
    uint16_t trim_adjust_flag[CH_MAX]; // Result
}_trim_algo_param;

static _trim_algo_param g_trim_algo_param;

static uint16_t gu16_screen_input_cur;

const static char* str_ALGO_BODY_STATE[4] =
{
    "NONE",
    "MINUS",
    "PLUS",
    "TARGET",
};

static double glf_TrimPara_GUI[TRIM_MAX][TRIM_PARA_MAX];

static uint8_t gn_trim_ignore_p2_flag;

void Trim_IF_GUI_SET_Activate(void)
{
    print(LOG_INFO, "GUI ACTIVAED");
    gb_GUI_ACTIVATED = true;

    if(gt_jig_trimming_step == TRIMMING_STEP_NONE)
    {
        gt_jig_trimming_step = TRIMMING_STEP_ACTIVATE_START;
    }
}

void Trim_IF_GUI_SET_TrimStart(uint32_t register_apply_flg)
{
    print(LOG_INFO, "GUI TRIM START:%d\r\n",register_apply_flg);
    
    gb_GUI_REG_APPLY_FOR_TRIM = (bool)register_apply_flg;
    gt_jig_trimming_step = TRIMMING_STEP_TRIM_START;
    gn_step_delay = 20;

    if(gub_NO_TRIM_ENABLE == true)
    {
        gt_trim_search_mode = TRIM_SCREENING;
    }
    else
    {
        gt_trim_search_mode = TRIM_OSC_FREQUENCY;
    }
}

void Trim_IF_GUI_SET_Verify_Done(void)
{
    gn_step_delay = 20;
    gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
}

void Trim_IF_GUI_OTP_WRITE_enable(uint8_t in_val)
{
    gub_OTP_WRITE_ENABLE = (bool)in_val;
    if(gub_OTP_WRITE_ENABLE)
    {
        print(LOG_INFO, "\r\nOTP_WRITE_ENABLE is ON.\r\n");
    }
    else
    {
        print(LOG_INFO, "\r\nOTP_WRITE_ENABLE is off.\r\n");
    }
}

void Trim_IF_GUI_WITHOUT_SCREEN(uint8_t in_val)
{
    gub_WITHOUT_SCREEN = (bool)in_val;
    if(gub_WITHOUT_SCREEN)
    {
        print(LOG_INFO, "\r\nWITHOUT_SCREEN is ON.\r\n");
    }
    else
    {
        print(LOG_INFO, "\r\nWITHOUT_SCREEN is off.\r\n");
    }
}

void Trim_IF_GUI_NO_TRIM_enable(uint8_t in_val)
{
    gub_NO_TRIM_ENABLE = (bool)in_val;
    if(gub_NO_TRIM_ENABLE == true)
    {
        print(LOG_INFO, "\r\nNO_TRIM is ON.\r\n");
    }
    else
    {
        print(LOG_INFO, "\r\nNO_TRIM is off.\r\n");
    }
}

void Trim_IF_GUI_TRIM_PARA_Set(double in_TrimPara[], double *gu16_TrimPara)
{
    if(gu16_TrimPara == &glf_TrimPara_GUI[TRIM_SCREENING][0])
    {
        gu16_TrimPara[TRIM_PARA_SCR_START]  = in_TrimPara[TRIM_PARA_SCR_START];
        gu16_TrimPara[TRIM_PARA_SCR_END]    = in_TrimPara[TRIM_PARA_SCR_END];
    }
    else
    {
        gu16_TrimPara[TRIM_PARA_TARGET_MIN] = in_TrimPara[TRIM_PARA_TARGET_MIN];
        gu16_TrimPara[TRIM_PARA_TARGET_MAX] = in_TrimPara[TRIM_PARA_TARGET_MAX];
    }
    gu16_TrimPara[TRIM_PARA_P1] = in_TrimPara[TRIM_PARA_P1];
    gu16_TrimPara[TRIM_PARA_P2] = in_TrimPara[TRIM_PARA_P2];

    #ifdef _DBG_ALL
        if( gu16_TrimPara == &glf_TrimPara_GUI[TRIM_OSC_FREQUENCY][0])
        {
            print(LOG_DEBUG, "glf_TrimPara_GUI[OSC][TRIM_PARA_P1] : %lf\r\n", glf_TrimPara_GUI[TRIM_OSC_FREQUENCY][TRIM_PARA_P1]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[OSC][TRIM_PARA_P2] : %lf\r\n", glf_TrimPara_GUI[TRIM_OSC_FREQUENCY][TRIM_PARA_P2]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[OSC][TRIM_PARA_TARGET_MIN] : %lf Mhz\r\n", glf_TrimPara_GUI[TRIM_OSC_FREQUENCY][TRIM_PARA_TARGET_MIN]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[OSC][TRIM_PARA_TARGET_MAX] : %lf Mhz\r\n", glf_TrimPara_GUI[TRIM_OSC_FREQUENCY][TRIM_PARA_TARGET_MAX]);
        }
        else if( gu16_TrimPara == &glf_TrimPara_GUI[TRIM_VREF_CTL][0])
        {
            print(LOG_DEBUG, "glf_TrimPara_GUI[VERF][TRIM_PARA_P1] : %lf\r\n", glf_TrimPara_GUI[TRIM_VREF_CTL][TRIM_PARA_P1]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[VERF][TRIM_PARA_P2] : %lf\r\n", glf_TrimPara_GUI[TRIM_VREF_CTL][TRIM_PARA_P2]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[VERF][TRIM_PARA_TARGET_MIN] : %lf V\r\n", glf_TrimPara_GUI[TRIM_VREF_CTL][TRIM_PARA_TARGET_MIN]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[VERF][TRIM_PARA_TARGET_MAX] : %lf V\r\n", glf_TrimPara_GUI[TRIM_VREF_CTL][TRIM_PARA_TARGET_MAX]);
        }
        else if( gu16_TrimPara == &glf_TrimPara_GUI[TRIM_OFS_CHS][0])
        {
            print(LOG_DEBUG, "glf_TrimPara_GUI[OFFSET][TRIM_PARA_P1] : %lf\r\n", glf_TrimPara_GUI[TRIM_OFS_CHS][TRIM_PARA_P1]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[OFFSET][TRIM_PARA_P2] : %lf\r\n", glf_TrimPara_GUI[TRIM_OFS_CHS][TRIM_PARA_P2]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[OFFSET][TRIM_PARA_TARGET_MIN] : %1.9f A\r\n", glf_TrimPara_GUI[TRIM_OFS_CHS][TRIM_PARA_TARGET_MIN]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[OFFSET][TRIM_PARA_TARGET_MAX] : %1.9f A\r\n", glf_TrimPara_GUI[TRIM_OFS_CHS][TRIM_PARA_TARGET_MAX]);
        }
        else if( gu16_TrimPara == &glf_TrimPara_GUI[TRIM_GAIN_CHS][0])
        {
            print(LOG_DEBUG, "glf_TrimPara_GUI[GAIN][TRIM_PARA_P1] : %lf\r\n", glf_TrimPara_GUI[TRIM_GAIN_CHS][TRIM_PARA_P1]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[GAIN][TRIM_PARA_P2] : %lf\r\n", glf_TrimPara_GUI[TRIM_GAIN_CHS][TRIM_PARA_P2]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[GAIN][TRIM_PARA_TARGET_MIN] : %1.9f A\r\n", glf_TrimPara_GUI[TRIM_GAIN_CHS][TRIM_PARA_TARGET_MIN]);
            print(LOG_DEBUG, "glf_TrimPara_GUI[GAIN][TRIM_PARA_TARGET_MAX] : %1.9f A\r\n", glf_TrimPara_GUI[TRIM_GAIN_CHS][TRIM_PARA_TARGET_MAX]);
        }
        else if( gu16_TrimPara == &glf_TrimPara_GUI[TRIM_SCREENING][0])
        {
            print(LOG_DEBUG, "glf_TrimPara_GUI[SCREEN][TRIM_PARA_P1] : %lf\r\n", glf_TrimPara_GUI[TRIM_SCREENING][TRIM_PARA_P1] );
            print(LOG_DEBUG, "glf_TrimPara_GUI[SCREEN][TRIM_PARA_P2] : %lf\r\n", glf_TrimPara_GUI[TRIM_SCREENING][TRIM_PARA_P2] );
            print(LOG_DEBUG, "glf_TrimPara_GUI[SCREEN][TRIM_PARA_SCR_START] : %lf\r\n", glf_TrimPara_GUI[TRIM_SCREENING][TRIM_PARA_SCR_START] );
            print(LOG_DEBUG, "glf_TrimPara_GUI[SCREEN][TRIM_PARA_SCR_END] : %lf\r\n", glf_TrimPara_GUI[TRIM_SCREENING][TRIM_PARA_SCR_END] );
        }
        else
        {
            print(LOG_ERROR, "\r\nERROR: gu16_TrimPara is WRONG\r\n");
        }
    #endif //_DBG_ALL
}

/* BEGIN - TRIMMING ALGORITHM   ***************************************/
static uint8_t check_valid_step(uint16_t in_step, uint8_t in_channel, uint8_t in_adj_type, trim_mode_t in_trim_mode)
{
    uint8_t ret = TRUE;

    if(in_adj_type == ADJ_PLUS)
    {
        uint16_t u16_register_limit = TargetIC_IF_TrimRegister_Limit_Get(in_channel, in_trim_mode);

        if( (TargetIC_IF_TrimRegister_Get(in_channel, in_trim_mode) + in_step) > (u16_register_limit-1) )
        {
            ret = FALSE;
        }        
    }
    else if(in_adj_type == ADJ_MINUS)
    {
        if( (TargetIC_IF_TrimRegister_Get(in_channel, in_trim_mode) - in_step) < (0+1) ) 
        {
            ret = FALSE;
        }
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

static void trim_algorithm_clear_buffer_channel(_trim_algo_param *ptr_Param)
{
    ptr_Param->u8_loop_cnt = 0;
    ptr_Param->u8_sTrimSaved_Cnt = 0;
    for(int i=0; i<TRIM_REGISTER_SAVED_CNT; ++i)
    {
        ptr_Param->sTrimSaved[i].u16_saved_adc = 0;
        ptr_Param->sTrimSaved[i].u16_saved_regi = 0;
    }
}

static void trim_algorithm_clear_buffer_All(_trim_algo_param *ptr_Param)
{
    trim_algorithm_clear_buffer_channel(ptr_Param);

    for(int i=0; i<CH_MAX; ++i)
    {
        ptr_Param->adc_cur[i]   = 0;
        ptr_Param->adc_pre[i]   = 0;
        ptr_Param->trim_step[i] = 0;
        ptr_Param->current[i]   = 0;
        ptr_Param->trim_adjust_flag[i] = ADJ_DEFAULT;        
    }
}

static void trim_init_algo_param(double (*pSetting)[TRIM_PARA_MAX])
{
    // Initialize g_trim_algo_param
#ifdef _DBG_ALL
        print(LOG_DEBUG, "Initialize g_trim_algo_param \r\n");
#endif
    for( trim_mode_t i_trim_mode = TRIM_OSC_FREQUENCY ; i_trim_mode<TRIM_MAX ; ++i_trim_mode)
    {
        current_gain_t g8_tmp_gain_level = GAIN_LOW;
        uint16_t u16_tmp_trim_range_adc_min = 0;
        uint16_t u16_tmp_trim_range_adc_max = 0;
        
        double d_tmp_min = pSetting[i_trim_mode][TRIM_PARA_TARGET_MIN];
        double d_tmp_max = pSetting[i_trim_mode][TRIM_PARA_TARGET_MAX];

        switch(i_trim_mode)
        {
            case TRIM_OSC_FREQUENCY:                            
                g8_tmp_gain_level = GAIN_LOW; //Don't Care
                break;            
            case TRIM_VREF_CTL:
                g8_tmp_gain_level = GAIN_LOW; //Don't Care
                break;                   
#if (XD12_TRIM_MAX_CURR == 8)
            case TRIM_OFS_CHS:
                g8_tmp_gain_level = GAIN_LOW;
                break;
            case TRIM_GAIN_CHS:
                g8_tmp_gain_level = GAIN_MID;
                break;
            case TRIM_SCREENING:
                g8_tmp_gain_level = GAIN_MID;
                gu16_screen_input_cur = (uint16_t)pSetting[i_trim_mode][TRIM_PARA_SCR_START];
                break;
#elif (XD12_TRIM_MAX_CURR == 46)
            case TRIM_OFS_CHS:
                g8_tmp_gain_level = GAIN_MID;
                break;                        
            case TRIM_GAIN_CHS:
                g8_tmp_gain_level = GAIN_HIGH;
                break;
            case TRIM_SCREENING:
                g8_tmp_gain_level = GAIN_HIGH;
                gu16_screen_input_cur = (uint16_t)pSetting[i_trim_mode][TRIM_PARA_SCR_START];
                break;
#elif (XD12_TRIM_MAX_CURR == 4)
            case TRIM_OFS_CHS:
                g8_tmp_gain_level = GAIN_LOW;
                break;                        
            case TRIM_GAIN_CHS:
                g8_tmp_gain_level = GAIN_MID;
                break;
            case TRIM_SCREENING:
                g8_tmp_gain_level = GAIN_MID;
                gu16_screen_input_cur = (uint16_t)pSetting[i_trim_mode][TRIM_PARA_SCR_START];
                break;
#else
#error "XD12_TRIM_MAX_CURR is not defined!!!!"
#endif
        }
    
        if(i_trim_mode == TRIM_OSC_FREQUENCY) // Freq
        {
            u16_tmp_trim_range_adc_min = JigBd_IF_Freq_ConvertByPrecaler(d_tmp_min);
            u16_tmp_trim_range_adc_max = JigBd_IF_Freq_ConvertByPrecaler(d_tmp_max);
        }
        else if(i_trim_mode == TRIM_VREF_CTL) // Internal ADC
        {
            u16_tmp_trim_range_adc_min = JigBd_IF_Convert_Volt_to_VREF_ADC(d_tmp_min);
            u16_tmp_trim_range_adc_max = JigBd_IF_Convert_Volt_to_VREF_ADC(d_tmp_max);
        }
        else //External ADC
        {
            u16_tmp_trim_range_adc_min = convert_current_to_adc(d_tmp_min, g8_tmp_gain_level);
            u16_tmp_trim_range_adc_max = convert_current_to_adc(d_tmp_max, g8_tmp_gain_level);
        }
    
        g_trim_algo_param.sTrimRange[i_trim_mode].u16_trim_range_adc_min    = u16_tmp_trim_range_adc_min;
        g_trim_algo_param.sTrimRange[i_trim_mode].u16_trim_range_adc_target = (uint16_t)(((float)(u16_tmp_trim_range_adc_min + u16_tmp_trim_range_adc_max) / 2) + 0.5f);
        g_trim_algo_param.sTrimRange[i_trim_mode].u16_trim_range_adc_max    = u16_tmp_trim_range_adc_max;
        g_trim_algo_param.sTrimRange[i_trim_mode].g8_trim_gain_level        = g8_tmp_gain_level;
    
    #ifdef _DBG_ALL
        print(LOG_DEBUG, "g_trim_algo_param.sTrimRange[%d].g8_trim_gain_level=%d\r\n", i_trim_mode, g8_tmp_gain_level);
        print(LOG_DEBUG, "g_trim_algo_param.sTrimRange[%d].u16_trim_rang_adc_min=%d\r\n", i_trim_mode, u16_tmp_trim_range_adc_min);
        print(LOG_DEBUG, "g_trim_algo_param.sTrimRange[%d].u16_trim_rang_adc_max=%d\r\n", i_trim_mode, u16_tmp_trim_range_adc_max);
    #endif
    }
}

static uint8_t trimming_algorithm_body(_trim_algo_param *ptr_Param)
{
    uint16_t u16_adc_range_min = 0;
    uint16_t u16_adc_range_target = 0;
    uint16_t u16_adc_range_max = 0;
    uint16_t u16_regi_value_cur = 0;
    uint8_t channel = 0;
    uint8_t u8_loop_cnt = 0;
    uint8_t u8_CH_MAX = 0;
    uint16_t u16_adc_cur = 0;
    uint16_t u16_adc_pre = 0;
    uint16_t trim_step = 0;
    uint8_t u8_rtn_val = TRIM_ALGORITHM_CONTINUE;
    double d_current = 0;
    const char *str_ADJ_result = str_ALGO_BODY_STATE[0];
    
    u16_adc_range_min = ptr_Param->sTrimRange[ptr_Param->trim_mode].u16_trim_range_adc_min;
    u16_adc_range_max = ptr_Param->sTrimRange[ptr_Param->trim_mode].u16_trim_range_adc_max;
    u16_adc_range_target = ptr_Param->sTrimRange[ptr_Param->trim_mode].u16_trim_range_adc_target;
    u8_CH_MAX = ptr_Param->u8_channel_max;

    ++ptr_Param->u8_loop_cnt;

    u8_loop_cnt = ptr_Param->u8_loop_cnt;
    channel = ptr_Param->u8_channel_cur;

    u16_adc_cur = ptr_Param->adc_cur[channel];
    u16_adc_pre = ptr_Param->adc_pre[channel];

    if(channel < u8_CH_MAX)    
    {
        uint8_t u8_tmp_sTrimSaved_Cnt = 0;
        uint16_t u16_adc_per_register = 0;
        current_gain_t u8_current_gain = ptr_Param->sTrimRange[ptr_Param->trim_mode].g8_trim_gain_level;

        u16_regi_value_cur = TargetIC_IF_TrimRegister_Get(channel, ptr_Param->trim_mode);
       
        if(ptr_Param->trim_mode == TRIM_OSC_FREQUENCY)
        {
            ptr_Param->current[channel] = JigBd_IF_Freq_Count_to_MHZ(u16_adc_cur);
        }
        else if(ptr_Param->trim_mode == TRIM_VREF_CTL)
        {
            ptr_Param->current[channel] = JigBd_IF_Convert_VREF_ADC_to_Volt(u16_adc_cur);
        }
        else
        {
            ptr_Param->current[channel] = convert_adc_to_current(u16_adc_cur, u8_current_gain);
        }
        d_current = ptr_Param->current[channel];

        if(u8_loop_cnt != 1) // Initial adc per register
        {
            u16_adc_per_register = (uint16_t)(((float)(abs(u16_adc_pre - u16_adc_cur)) / ptr_Param->trim_step[channel]) + 0.5f);
        }
        else
        {
            u16_adc_per_register = ptr_Param->u16_init_adc_per_regi;
        }
        
        #ifdef DBG_TRIM_ALGORITHM
            print(LOG_DEBUG, "( adc_pre[channel]:%d - adc_cur[channel]:%d ) / trim_step[channel]:%d = %d\r\n ",
                    u16_adc_pre, u16_adc_cur, ptr_Param->trim_step[channel], u16_adc_per_register);
        #endif //#ifdef DBG_TRIM_ALGORITHM

        // Copy current ADC to previous ADC
        ptr_Param->adc_pre[channel] = u16_adc_cur;

        u8_tmp_sTrimSaved_Cnt = (ptr_Param->u8_sTrimSaved_Cnt % TRIM_REGISTER_SAVED_CNT);

        // Check ADJ_PLUS
        if(u16_adc_cur < u16_adc_range_target)
        {
            ptr_Param->trim_adjust_flag[channel] = ADJ_PLUS;
            str_ADJ_result = str_ALGO_BODY_STATE[2];
            if(u16_adc_per_register == 0)
            {
                ptr_Param->trim_step[channel] = 1;
            }
            else
            {
                if (u16_adc_range_target - u16_adc_cur >= u16_adc_per_register*2)
                {
                    trim_step = (uint16_t)(((float)(abs(u16_adc_cur - u16_adc_range_target)) / u16_adc_per_register) + 0.5f);
                    if(check_valid_step( trim_step, channel, ADJ_PLUS, ptr_Param->trim_mode) )
                    {
                        ptr_Param->trim_step[channel] = (trim_step ? trim_step : 1);
                    }
                    else
                    {
                        ptr_Param->trim_step[channel] = 1;
                    }
                }
                else
                {
                    ptr_Param->trim_step[channel] = 1;
                }
            }

            // Check Additional Margin is matched.
            if(u16_adc_cur > u16_adc_range_min)
            {
                ptr_Param->sTrimSaved[u8_tmp_sTrimSaved_Cnt].u16_saved_regi = u16_regi_value_cur;
                ptr_Param->sTrimSaved[u8_tmp_sTrimSaved_Cnt].u16_saved_adc = u16_adc_cur;                    
                ++ptr_Param->u8_sTrimSaved_Cnt;

                #ifdef DBG_TRIM_ALGORITHM
                    print(LOG_DEBUG, "RANGE_MARGIN-ADJ_PLUS[%d]]\r\n",ptr_Param->u8_sTrimSaved_Cnt);
                #endif //#ifdef DBG_TRIM_ALGORITHM
            }
        }
        // Check ADJ_MINUS
        else if(u16_adc_cur > u16_adc_range_target)
        {
            ptr_Param->trim_adjust_flag[channel] = ADJ_MINUS;
            str_ADJ_result = str_ALGO_BODY_STATE[1];
            if(u16_adc_per_register == 0)
            {
                ptr_Param->trim_step[channel] = 1;
            }
            else
            {
                if (u16_adc_cur - u16_adc_range_target >= u16_adc_per_register*2)
                {
                    trim_step = (uint16_t)(((float)(abs(u16_adc_cur - u16_adc_range_target)) / u16_adc_per_register) + 0.5f);
                    if( check_valid_step( trim_step, channel, ADJ_MINUS, ptr_Param->trim_mode ) )
                    {
                        ptr_Param->trim_step[channel] = (trim_step ? trim_step : 1);
                    }
                    else
                    {
                        ptr_Param->trim_step[channel] = 1;
                    }
                }
                else
                {
                    ptr_Param->trim_step[channel] = 1;
                }
            }

            // Check Additional Margin is matched.
            if(u16_adc_cur < u16_adc_range_max)
            {
                ptr_Param->sTrimSaved[u8_tmp_sTrimSaved_Cnt].u16_saved_regi = u16_regi_value_cur;
                ptr_Param->sTrimSaved[u8_tmp_sTrimSaved_Cnt].u16_saved_adc = u16_adc_cur;

                print(LOG_DEBUG, "RANGE_MARGIN-ADJ_MINUS[%d]-[%u, %u]\r\n",ptr_Param->u8_sTrimSaved_Cnt, u16_regi_value_cur, u16_adc_cur);

                ++ptr_Param->u8_sTrimSaved_Cnt;
                #ifdef DBG_TRIM_ALGORITHM
                    print(LOG_DEBUG, "RANGE_MARGIN_OFS-ADJ_MINUS[%d]\r\n",ptr_Param->u8_sTrimSaved_Cnt);
                #endif //#ifdef DBG_TRIM_ALGORITHM
            }
        }
        // Check ADJ_NONE
        else
        {
            ptr_Param->trim_adjust_flag[channel] = ADJ_NONE;
            str_ADJ_result = str_ALGO_BODY_STATE[0];
            ptr_Param->trim_step[channel] = 0;

            for(int i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
            {                                
                if(ptr_Param->sTrimSaved[i].u16_saved_regi == u16_regi_value_cur) //If there is an 2 times matched
                {
                    ++ptr_Param->u8_channel_cur;
                    trim_algorithm_clear_buffer_channel(ptr_Param);
                    u8_rtn_val = TRIM_ALGORITHM_DONE_CHANNEL; // Done - Channel
                    break;
                }
            }

            // If there is not an 2 times matched,
            if(ptr_Param->u8_channel_cur == channel)
            {
                ptr_Param->sTrimSaved[u8_tmp_sTrimSaved_Cnt].u16_saved_regi = u16_regi_value_cur;
                ptr_Param->sTrimSaved[u8_tmp_sTrimSaved_Cnt].u16_saved_adc = u16_adc_cur;
                print(LOG_DEBUG, "RANGE_MARGIN-ADJ_MATCHED[%d]-[%u, %u]\r\n",ptr_Param->u8_sTrimSaved_Cnt, u16_regi_value_cur, u16_adc_cur);
                ++ptr_Param->u8_sTrimSaved_Cnt;
            }
        }
        #ifdef DBG_TRIM_ALGORITHM
            print(LOG_DEBUG, "u16_adc_per_register:%d / ptr_Param->trim_step[%d]:%d\r\n ",u16_adc_per_register, channel, ptr_Param->trim_step[channel]);
        #endif //#ifdef DBG_TRIM_ALGORITHM
    }

    // Check Vibration
    if(ptr_Param->u8_sTrimSaved_Cnt >= TRIM_REGISTER_SAVED_CNT)
    {
        uint16_t u16_adc_gap_closest = 0xFFFF;
        uint16_t u16_adc_gap_temp = 0;
        uint8_t u8_closest_adc_index = TRIM_REGISTER_SAVED_CNT;
        for(uint8_t i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
        {
            if (ptr_Param->trim_mode == TRIM_OSC_FREQUENCY)
            {
                u16_adc_gap_temp = abs(ptr_Param->sTrimSaved[i].u16_saved_adc - u16_adc_range_min);
            }
            else
            {
                u16_adc_gap_temp = abs(ptr_Param->sTrimSaved[i].u16_saved_adc - u16_adc_range_target);
            }
            if( u16_adc_gap_temp < u16_adc_gap_closest )
            {
                u16_adc_gap_closest = u16_adc_gap_temp;
                u8_closest_adc_index = i;
            }
        }

        if(u8_closest_adc_index == TRIM_REGISTER_SAVED_CNT)
        {
            print(LOG_ERROR, "********ADJUST_OVER_RANGE ERROR(%d,%d)********\r\n",channel+1, u8_closest_adc_index);
            trim_algorithm_clear_buffer_channel(ptr_Param);
            u8_rtn_val = TRIM_ALGORITHM_ERROR; // Done - Channel
        }
        else
        {
            // Write Register
            print(LOG_DEBUG, "********ADJUST_RANGE(%d,%d)********\r\n",channel+1, ptr_Param->sTrimSaved[u8_closest_adc_index].u16_saved_regi);
            TargetIC_IF_TrimRegister_Set(channel, ptr_Param->trim_mode, ptr_Param->sTrimSaved[u8_closest_adc_index].u16_saved_regi);

            ++ptr_Param->u8_channel_cur;
            trim_algorithm_clear_buffer_channel(ptr_Param);
            u8_rtn_val = TRIM_ALGORITHM_DONE_CHANNEL; // Done - Channel
        }
    }

    // Check Limit count of trying
    if(u8_loop_cnt > (TRIM_REGISTER_SAVED_CNT + TRIM_OUT_RANGE_CNT))
    {
        print(LOG_ERROR, "ERROR!! TRIM_CH[%d] : RETRY COUNT is OVER %d\r\n",channel+1, TRIM_REGISTER_SAVED_CNT + TRIM_OUT_RANGE_CNT);

        // Clear Buffers ALL
        trim_algorithm_clear_buffer_All(ptr_Param);
        return TRIM_ALGORITHM_ERROR;
    }

    // Print Status
    if(u8_loop_cnt == 1)
    {                
        print(LOG_INFO, "%s_%02d\r\n", gstr_TRIM_MODE[ptr_Param->trim_mode], channel+1);

        #ifdef DBG_TRIM_ALGORITHM
            print(LOG_INFO, "RANGE_MARGIN_ADJ_PLUS:%d\r\n",(u16_adc_range_min));
            print(LOG_INFO, "RANGE_MARGIN_ADJ_MINUS:%d\r\n", (u16_adc_range_max));
        #endif //#ifdef DBG_TRIM_ALGORITHM
        if(ptr_Param->trim_mode == TRIM_OSC_FREQUENCY)
        {
            print(LOG_INFO, "[Cnt]      [RANGE]      [ADC]    [MHz]    [REG]  [Check]\r\n");
        }
        else if(ptr_Param->trim_mode == TRIM_VREF_CTL)
        {
            print(LOG_INFO, "[Cnt]      [RANGE]      [ADC]     [V]      [REG]  [Check]\r\n");
        }
        else if(ptr_Param->trim_mode == TRIM_OFS_CHS)
        {
            print(LOG_INFO, "[Cnt]      [RANGE]      [ADC]    [uA]      [REG]  [Check]\r\n");
        }
        else
        {
            print(LOG_INFO, "[Cnt]      [RANGE]      [ADC]    [mA]      [REG]  [Check]\r\n");
        }
    }    
    print(LOG_INFO, "  %02d    %7u/%5u %7u   %7.3f   %4u  %7s [ %u, %u, %u ]\r\n", 
        u8_loop_cnt, u16_adc_range_min, u16_adc_range_max,
        u16_adc_cur, ((ptr_Param->trim_mode == TRIM_OFS_CHS) ? (d_current * 1000) : d_current), u16_regi_value_cur,
        str_ADJ_result, u16_adc_range_target, ptr_Param->trim_step[channel], ptr_Param->u8_sTrimSaved_Cnt);

    // Check Last Channel
    if(ptr_Param->u8_channel_cur >= u8_CH_MAX)
    {
        #ifdef CHK_TRIM_TIME
            g_CHK_TRIM_TIME_TRIM = HAL_GetTick() - g_CHK_TRIM_TIME_tick;
            print(LOG_DEBUG, "\r\n\r\nCHK_TRIM_TIME-TRIM-END:%d\r\n\r\n", g_CHK_TRIM_TIME_TRIM);
        #endif //CHK_TRIM_TIME
        
        print(LOG_DEBUG, "[%s]\r\n", gstr_TRIM_MODE[ptr_Param->trim_mode]);

        print(LOG_INFO, "[RANGE]   %7u   %7u   %7u\r\n",u16_adc_range_min, u16_adc_range_target, u16_adc_range_max);
        
        print(LOG_INFO,  "[ADC]  ");
        for(uint8_t i = 0 ; i < u8_CH_MAX ; ++i)
        {
            print(LOG_INFO, "   %7u",ptr_Param->adc_cur[i]);
        }
        print(LOG_INFO, "\r\n");
        
        if(ptr_Param->trim_mode == TRIM_OSC_FREQUENCY)
        {
            print(LOG_INFO, "[Freq] ");
        }
        else if(ptr_Param->trim_mode == TRIM_VREF_CTL)
        {
            print(LOG_INFO, "[mV]    ");
        }
        else
        {
            print(LOG_INFO, "[mA]    ");
        }
        for(uint8_t i = 0 ; i < u8_CH_MAX ; ++i)
        {
            print(LOG_INFO, "   %7.3f",ptr_Param->current[i]);
        }
        print(LOG_INFO, "\r\n");

        print(LOG_INFO, "[Check]");
        for(uint8_t i = 0 ; i < u8_CH_MAX ; ++i)
        {
            print(LOG_INFO, "   %7s", str_ADJ_result);
        }
        print(LOG_INFO, "\r\n");

        print(LOG_INFO, "\t CHANNEL[%d] DONE:EXIT\r\n\r\n", ptr_Param->u8_channel_cur);

        // Clear Buffers All
        trim_algorithm_clear_buffer_All(ptr_Param);
        u8_rtn_val = TRIM_ALGORITHM_DONE_MODE; // Done - Mode
    }
    else
    {
        #ifdef DBG_TRIM_ALGORITHM
            print(LOG_DEBUG, "\t  REPEAT \r\n");
        #endif //DBG_TRIM_ALGORITHM
    }

    return u8_rtn_val;
}
/* END - TRIMMING ALGORITHM   *****************************************/

/* BEGIN - TRIMMING_PROCEDURE_RUN   *****************************************/
void trimming_procedure_run(void)
{
    if(gt_jig_trimming_step != TRIMMING_STEP_NONE)
    {
        uint8_t channel = 0;
        uint16_t u16_tmp_init_adc_per_regi = 0;
        uint16_t u16_tmp_adc_cur = 0;
        trim_mode_t t_trim_search_mode_next;
        uint8_t u8_rtn_trim_algo = 0;
        uint8_t u8_tmp_channel_max = 0;
        uint16_t u16_tmp_regVal = 0;

        switch(gt_jig_trimming_step)
        {
            case TRIMMING_STEP_STANDBY:
                print(LOG_INFO, "\r\n======== STANDBY PWR-OFF ========\r\n");

                JigBd_IF_select_output_ch(CH_MAX);  /* Output OFF */
#if 0
                /* VLED_CTR_9V : OFF */
                JigBd_IF_VLED_9V_EN(PWR_OFF);
                JigBd_IF_XD_VCC_EN(PWR_OFF);
                JigBd_IF_XC_VCC_EN(PWR_OFF);
#endif            
                gn_step_delay = 10;
                gt_jig_trimming_step = TRIMMING_STEP_NONE;
                break;
/* TRIMMING_STEP_ACTIVATE_START  ***************************************/
            case TRIMMING_STEP_ACTIVATE_START:
                print(LOG_INFO, "\r\n======== TRIMMING_STEP_ACTIVATE_START ========\r\n");

                gn_trim_error_code = TRIM_ERROR_NONE;
                gn_read_adc_vout_channel = 0;

                JigBd_IF_select_output_ch(CH_MAX);  /* Output OFF */
                JigBd_IF_current_discharge(DISCHARGE);
                JigBd_IF_change_current_gain(GAIN_HIGH);

                gt_jig_trimming_step = TRIMMING_STEP_IC_PWR;
                break;

/* TRIMMING_STEP_IC_PWR  ***************************************/
            case TRIMMING_STEP_IC_PWR:
                #ifdef CHK_TRIM_TIME
                g_CHK_TRIM_TIME_tick = HAL_GetTick();                    
                print(LOG_DEBUG, "\r\n CHK_TRIM_TIME-INIT-START:%d\r\n ", g_CHK_TRIM_TIME_tick);
                #endif //CHK_TRIM_TIME

                JigBd_IF_XD_VCC_EN(PWR_ON);
                JigBd_IF_VLED_9V_EN(PWR_ON);
                
                // XC : If use XC, turn on VCC and initialize
                JigBd_IF_Init();
                
                JigBd_IF_current_discharge(CHARGE);
                JigBd_IF_select_output_ch(CH_MAX);
                
                gn_step_delay = 10;
                gt_jig_trimming_step = TRIMMING_STEP_ATIVATE_END;
                break;

/* TRIMMING_STEP_ATIVATE_END  ***************************************/
            case TRIMMING_STEP_ATIVATE_END:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    TargetIC_IF_Initialize();
                    TargetIC_IF_Read_Resgister_All(_GUI_SUPPORT_);
                    TargetIC_IF_Show_RegAll();
                    gn_step_delay = 10;

                    #ifdef CHK_TRIM_TIME
                        g_CHK_TRIM_TIME_INIT = HAL_GetTick() - g_CHK_TRIM_TIME_tick;
                        print(LOG_DEBUG, "\r\n\r\n CHK_TRIM_TIME-INIT-END:%d\r\n\r\n", g_CHK_TRIM_TIME_INIT);
                    #endif //CHK_TRIM_TIME
                    
                    print(LOG_INFO, "\r\n\t[trim_type] - [min/max] [p1/p2]");

                    for (trim_mode_t trim_mode = TRIM_OSC_FREQUENCY ; trim_mode < TRIM_SCREENING ; ++trim_mode)
                    {
                        print(LOG_INFO, "\r\n\t[%s] - [%.5f/%.5f] [%.0f/%.0f]", gstr_TRIM_MODE[trim_mode], 
                            1000 * glf_TrimPara_GUI[trim_mode][TRIM_PARA_TARGET_MIN], 1000 * glf_TrimPara_GUI[trim_mode][TRIM_PARA_TARGET_MAX], 
                            glf_TrimPara_GUI[trim_mode][TRIM_PARA_P1], glf_TrimPara_GUI[trim_mode][TRIM_PARA_P2]);
                    }

                    print(LOG_INFO, "\r\n======== TRIMMING_STEP_ATIVATE_END ========\r\n");
                    gt_jig_trimming_step = TRIMMING_STEP_STANDBY;
                }
                break;

/* TRIMMING_STEP_TRIM_START  ***************************************/
            case TRIMMING_STEP_TRIM_START:
                JigBd_IF_VLED_9V_EN(PWR_ON);

                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    #ifdef CHK_TRIM_TIME
                        g_CHK_TRIM_TIME_tick = HAL_GetTick();
                        print(LOG_DEBUG, "\r\n CHK_TRIM_TIME-TRIM-START:%d\r\n ", g_CHK_TRIM_TIME_tick);
                    #endif //CHK_TRIM_TIME

                    if(gb_GUI_REG_APPLY_FOR_TRIM)
                    {
                        print(LOG_INFO, "\r\nTRIM starts with Register Values changed by GUI!\r\n");
                    }
                    else
                    {
                        TargetIC_IF_Init_Register();
                    }

                    for (uint8_t ch = 0 ; ch < CH_MAX ; ch++)
                    {
                        g_trim_algo_param.trim_adjust_flag[ch] = ADJ_DEFAULT;
                    }

                    TargetIC_IF_Read_Resgister_All(_GUI_SUPPORT_);
                    trim_init_algo_param(glf_TrimPara_GUI);
                    
                    gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                }
                break;

/* TRIMMING_STEP_CHANGE_OUTPUT_INIT  ***************************************/
            case TRIMMING_STEP_CHANGE_OUTPUT_INIT:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    if((gub_WITHOUT_SCREEN == true) && (gt_trim_search_mode == TRIM_SCREENING))
                    {
                        gt_jig_trimming_step = TRIMMING_STEP_STANDBY;
                    }
                    else
                    {
                        if((gt_trim_search_mode == TRIM_SCREENING) &&
                           (gu16_screen_input_cur == glf_TrimPara_GUI[TRIM_SCREENING][TRIM_PARA_SCR_START]) &&
                           (gn_read_adc_vout_channel == 0))
                        {
                            // First Line of EXCEL
                            for(uint8_t i = 0 ; i < CH_MAX ; ++i)
                            {
                                print(LOG_INFO, ":0");
                            }
                            print(LOG_INFO, "%s\r\n", CMD_XDIC_SLOPE_START);

                            if (gn_trim_ignore_p2_flag == 1)
                            {
                                for (uint8_t idx = 0 ; idx < 12 ; ++idx)
                                {
                                    TargetIC_IF_Write_Register(XD12_ADDR_TRIM_GAIN_CH01 + idx, TargetIC_IF_Get_Register(XD12_ADDR_TRIM_OFS_CH01 + idx, XD12_REG_TYPE_TRIM), XD12_REG_TYPE_TRIM);
                                }
                            }
                            
                            TargetIC_IF_Show_Trim_Reg();
                            
                            print(LOG_INFO, "%s\r\n", "xdic:para:LD_FIX:Io_1:Io_2:Io_3:Io_4:Io_5:Io_6:Io_7:Io_8:Io_9:Io_10:Io_11:Io_12:Io_spec:dev_1:dev_2:dev_3:dev_4:dev_5:dev_6:dev_7:dev_8:dev_9:dev_10:dev_11:dev_12");
                        }

                        if(gt_trim_search_mode > TRIM_VREF_CTL)
                        {
                            // Input level of TEST CHIP ex) D-in voltage - 0 volt
                            TargetIC_IF_set_LD_FIX(0);                            
                            TargetIC_IF_set_CHANNEL_ENABLE_All();                            
                            // Select the tagert output channel of of TEST CHIP
                            JigBd_IF_select_output_ch(gn_read_adc_vout_channel);
                            JigBd_IF_change_current_gain(g_trim_algo_param.sTrimRange[gt_trim_search_mode].g8_trim_gain_level);
                            JigBd_IF_current_discharge(CHARGE);
                        }
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT;
                    }
                }
                break;

/* TRIMMING_STEP_CHANGE_OUTPUT  ***************************************/
            case TRIMMING_STEP_CHANGE_OUTPUT:
                switch(gt_trim_search_mode)
                {
                    case TRIM_OSC_FREQUENCY:
                        TargetIC_IF_TRIM_Init_OSC();
                        JigBd_IF_TIM_Capture_Start();
                        gn_step_delay = 100;
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_DONE;
                        break;

                    case TRIM_VREF_CTL:
                        TargetIC_IF_TRIM_Init_VREF_CTL();
                        gn_step_delay = 1;
                        gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_DONE;
                        break;
                        
                    case TRIM_OFS_CHS:
                    case TRIM_GAIN_CHS:
                        JigBd_IF_current_discharge(CHARGE);
                        if(gt_trim_search_mode == TRIM_OFS_CHS)
                        {
                            TargetIC_IF_TRIM_Init_OFS();
                        }
                        else //TRIM_GAIN_CHS
                        {
                            TargetIC_IF_TRIM_Init_GAIN();
                        }

                        if(gn_slope_cnt == 0)
                        {
                            TargetIC_IF_set_LD_FIX((uint16_t)glf_TrimPara_GUI[gt_trim_search_mode][TRIM_PARA_P1]);
                        }
                        else
                        {
                            TargetIC_IF_set_LD_FIX((uint16_t)glf_TrimPara_GUI[gt_trim_search_mode][TRIM_PARA_P2]);
                        }

                        gt_jig_trimming_step = TRIMMING_STEP_SET_ADC_CH;
                        gn_step_delay = 25;
                        break;

                    case TRIM_SCREENING:
                        TargetIC_IF_set_LD_FIX((uint16_t)glf_TrimPara_GUI[gt_trim_search_mode][TRIM_PARA_SCR_START] + gu16_screen_input_cur);                                                
                        screen_iout_spec_max_curr = XD12_get_max_current_level();
                        screen_iout_spec_max_curr_vref = TargetIC_IF_Get_Register(XD12_ADDR_MAX_CURR_VREF, XD12_REG_TYPE_NON_TRIM);
                        screen_iout_spec = (screen_iout_spec_max_curr * (screen_iout_spec_max_curr_vref / 0xFFF) * (glf_TrimPara_GUI[gt_trim_search_mode][TRIM_PARA_SCR_START] + gu16_screen_input_cur)) / REG_LIMIT_LD_FIX;

                        if (screen_iout_spec < CURRENT_LIMIT_GAIN_LOW)
                        {
                            g_trim_algo_param.sTrimRange[gt_trim_search_mode].g8_trim_gain_level = GAIN_LOW;
                        }
                        else if (screen_iout_spec < CURRENT_LIMIT_GAIN_MID)
                        {
                            g_trim_algo_param.sTrimRange[gt_trim_search_mode].g8_trim_gain_level = GAIN_MID;
                        }
                        else //if (screen_iout_spec < CURRENT_LIMIT_GAIN_HIGH)
                        {
                            g_trim_algo_param.sTrimRange[gt_trim_search_mode].g8_trim_gain_level = GAIN_HIGH;
                        }
                        JigBd_IF_change_current_gain(g_trim_algo_param.sTrimRange[gt_trim_search_mode].g8_trim_gain_level);
                        
                        gt_jig_trimming_step = TRIMMING_STEP_SET_ADC_CH;
                        gn_step_delay = 25;
                        break;
                    default:
                        break;
                }
                break;

            case TRIMMING_STEP_CHANGE_OUTPUT_DONE:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    switch(gt_trim_search_mode)
                    {
                        case TRIM_OSC_FREQUENCY:
                            JigBd_IF_TIM_Capture_Stop();
                            gt_jig_trimming_step = TRIMMING_STEP_CHECK;
                            break;

                        case TRIM_VREF_CTL:                    
                            JigBd_IF_VREF_ADC_StartStop();
                            gt_jig_trimming_step = TRIMMING_STEP_CHECK;
                            break;
                    }
                }
                break;
                
/* TRIMMING_STEP_CHANGE_REGISTER  ***************************************/
            case TRIMMING_STEP_CHANGE_REGISTER:
                channel = 0;
                u16_tmp_regVal = 0;
                switch(gt_trim_search_mode)
                {
                    case TRIM_OSC_FREQUENCY:
                    case TRIM_VREF_CTL:
                        channel = CH_01;
                        break;

                    case TRIM_OFS_CHS:
                    case TRIM_GAIN_CHS:
                        channel = gn_read_adc_vout_channel;
                        break;
                }

                if(g_trim_algo_param.trim_adjust_flag[channel] == ADJ_PLUS)
                {
                    uint16_t u16_tmp_limit = TargetIC_IF_TrimRegister_Limit_Get(channel, gt_trim_search_mode);
                    u16_tmp_regVal = TargetIC_IF_TrimRegister_Get(channel, gt_trim_search_mode);
                    u16_tmp_regVal += g_trim_algo_param.trim_step[channel];

                    if(u16_tmp_regVal <= u16_tmp_limit )
                    {
                        TargetIC_IF_TrimRegister_Set(channel, gt_trim_search_mode, u16_tmp_regVal);
                    }
                    else
                    {
                        print(LOG_ERROR, "ERROR: TRIM STOP - OVER LIMIT-%d:[%d/%d]", gt_trim_search_mode ,u16_tmp_regVal ,u16_tmp_limit);
                        gn_trim_error_code = TRIM_ERROR_OVER_COUNT;
                    }
                }
                else if(g_trim_algo_param.trim_adjust_flag[channel] == ADJ_MINUS)
                {
                    u16_tmp_regVal = TargetIC_IF_TrimRegister_Get(channel, gt_trim_search_mode);                            
                    
                    if( u16_tmp_regVal >= g_trim_algo_param.trim_step[channel] )
                    {
                        u16_tmp_regVal -= g_trim_algo_param.trim_step[channel];

                        TargetIC_IF_TrimRegister_Set(channel, gt_trim_search_mode, u16_tmp_regVal );
                    }
                    else
                    {
                        print(LOG_ERROR, "ERROR: TRIM STOP - UNDER ZERO-%d:[%d-%d]\r\n", gt_trim_search_mode ,u16_tmp_regVal ,g_trim_algo_param.trim_step[channel]);
                        gn_trim_error_code = TRIM_ERROR_UNDER_COUNT;
                    }
                }

                if(gn_trim_error_code == TRIM_ERROR_NONE)
                {             
                    gn_step_delay = 25;
                    //gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                    gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT;
                }
                else
                {
                    gt_jig_trimming_step = TRIMMING_STEP_RESULT;
                }
                break;

/* TRIMMING_STEP_SET_ADC_CH  ***************************************/
            case TRIMMING_STEP_SET_ADC_CH:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    ads114s08_select_single_ended_input(0);
                    gn_step_delay = 10;
                    gt_jig_trimming_step = TRIMMING_STEP_START_ADC_CONVERSION;
                }
                break;

/* TRIMMING_STEP_START_ADC_CONVERSION  ***************************************/
            case TRIMMING_STEP_START_ADC_CONVERSION:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    gb_ads114s08_drdy_done = 0;
                    gn_ads114s08_adc_temp = 0;

                    gn_adc_read_count = ADS114S08_READ_COUNT;
                    gt_jig_trimming_step = TRIMMING_STEP_GET_ADC_CH;

                    ads114s08_set_start(1);
                }
                break;

/* TRIMMING_STEP_GET_ADC_CH  ***************************************/
            case TRIMMING_STEP_GET_ADC_CH:
                if(gb_ads114s08_drdy_done == 1)
                {
                    switch(gt_trim_search_mode)
                    {
                        case TRIM_VREF_CTL:
                            gn_slope_adc[gn_read_adc_vout_channel][gn_slope_cnt] = get_adc_value();
                            gn_slope_cnt = 0;	                    
                            gt_jig_trimming_step = TRIMMING_STEP_CHECK;
                            break;
                        case TRIM_OFS_CHS:
                        case TRIM_GAIN_CHS:
                            gn_slope_adc[gn_read_adc_vout_channel][gn_slope_cnt] = get_adc_value();
                            if (g_trim_algo_param.sTrimRange[gt_trim_search_mode].g8_trim_gain_level == GAIN_LOW)
                            {
                                //gn_slope_adc[gn_read_adc_vout_channel][gn_slope_cnt] -= gn_ads114s08_offset[gn_read_adc_vout_channel];
                            }

                             ++gn_slope_cnt;
                            if (gn_slope_cnt >= 2)
                            {
                                gn_slope_cnt = 0;	                    
                                gt_jig_trimming_step = TRIMMING_STEP_CHECK;
                            }
                            else
                            {
                                gn_step_delay = 10;
                                gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT;
                            }
                             break;
                        case TRIM_SCREENING:
                            gn_screen_adc[gn_read_adc_vout_channel] = get_adc_value();
                            if (g_trim_algo_param.sTrimRange[gt_trim_search_mode].g8_trim_gain_level == GAIN_LOW)
                            {
                                //gn_screen_adc[gn_read_adc_vout_channel] -= gn_ads114s08_offset[gn_read_adc_vout_channel];
                            }
                            
                            TargetIC_IF_set_LD_FIX(0);
                            
                            ++gn_read_adc_vout_channel;
                            if(gn_read_adc_vout_channel >= CH_MAX)
                            {
                                JigBd_IF_select_output_ch(CH_MAX);  /* Output OFF */
                                gn_read_adc_vout_channel = 0;
                                gt_jig_trimming_step = TRIMMING_STEP_CHECK;
                            }
                            else
                            {
                                gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                            }
                            break;
                    }
                    JigBd_IF_current_discharge(DISCHARGE);
                    gn_step_delay = 1;
                    gb_ads114s08_drdy_done = 0;
                }
                break;

/* TRIMMING_STEP_CHECK  ***************************************/
            case TRIMMING_STEP_CHECK:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    switch(gt_trim_search_mode)
                    {
                        case TRIM_OSC_FREQUENCY:
                            u16_tmp_init_adc_per_regi = INIT_ADC_PER_REGI_OSC;
                            u8_tmp_channel_max = 1;
                            t_trim_search_mode_next = TRIM_VREF_CTL;
                            u16_tmp_adc_cur = JigBd_IF_Freq_Get();
                            break;
                        case TRIM_VREF_CTL:
                            u16_tmp_init_adc_per_regi = INIT_ADC_PER_REGI_VREF;
                            u8_tmp_channel_max = 1;
                            t_trim_search_mode_next = TRIM_OFS_CHS;
                            u16_tmp_adc_cur =  JigBd_IF_VREF_ADC_Get();
                            break;
                        case TRIM_OFS_CHS:
                            u16_tmp_init_adc_per_regi = INIT_ADC_PER_REGI_OFS_1;
                            u8_tmp_channel_max = CH_MAX;
                            t_trim_search_mode_next = TRIM_GAIN_CHS;
                            u16_tmp_adc_cur = (uint16_t)(((float)(gn_slope_adc[gn_read_adc_vout_channel][1] + gn_slope_adc[gn_read_adc_vout_channel][0]) / 2) + 0.5f);
                            break;
                        case TRIM_GAIN_CHS:
                            u16_tmp_init_adc_per_regi = INIT_ADC_PER_REGI_OFS_2;
                            u8_tmp_channel_max = CH_MAX;
                            t_trim_search_mode_next = TRIM_SCREENING;
                            u16_tmp_adc_cur = gn_slope_adc[gn_read_adc_vout_channel][0];
                            break;
                        case TRIM_SCREENING:
                            u16_tmp_init_adc_per_regi = 0;
                            u8_tmp_channel_max = 0;
                            t_trim_search_mode_next = TRIM_SCREENING;
                            break;
                    }

                    if(gt_trim_search_mode != TRIM_SCREENING)
                    {
                        g_trim_algo_param.u16_init_adc_per_regi = u16_tmp_init_adc_per_regi;
                        g_trim_algo_param.u8_channel_max = u8_tmp_channel_max;
                        g_trim_algo_param.trim_mode = gt_trim_search_mode;
                        g_trim_algo_param.u8_channel_cur = gn_read_adc_vout_channel;
                        g_trim_algo_param.adc_cur[gn_read_adc_vout_channel] = u16_tmp_adc_cur;

                        //Run Trim Algorithm
                        u8_rtn_trim_algo = trimming_algorithm_body(&g_trim_algo_param);

                        if(u8_rtn_trim_algo == TRIM_ALGORITHM_ERROR)
                        {
                            print(LOG_ERROR, "\t  ERROR STOP\r\n");
                            gt_jig_trimming_step = TRIMMING_STEP_STANDBY;
                        }
                        else if(u8_rtn_trim_algo == TRIM_ALGORITHM_CONTINUE)
                        {
                            print(LOG_DEBUG, "\t  REPEAT \r\n");
                            gt_jig_trimming_step = TRIMMING_STEP_CHANGE_REGISTER;
                        }
                        else if(u8_rtn_trim_algo == TRIM_ALGORITHM_DONE_CHANNEL)
                        {
                            print(LOG_DEBUG, "\t  Next Channel \r\n");
                            ++gn_read_adc_vout_channel;
                            gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                        }
                        else if(u8_rtn_trim_algo == TRIM_ALGORITHM_DONE_MODE)
                        {
                            print(LOG_DEBUG, "\t  Next trim_search_mode \r\n");
                            if( gub_OTP_WRITE_ENABLE && (t_trim_search_mode_next == TRIM_SCREENING) )
                            {                            
                                gt_jig_trimming_step = TRIMMING_STEP_E2P_PROGRAM;
                            }
                            else
                            {
                                gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                            }
                            gn_read_adc_vout_channel = 0;
                            gt_trim_search_mode = t_trim_search_mode_next;
                        }
                    }
                    else  //TRIM_SCREENING
                    {
                        uint8_t ch = 0;
                        for (ch = 0 ; ch < CH_MAX ; ++ch)
                        {
                            screen_current[ch] = convert_adc_to_current(gn_screen_adc[ch], g_trim_algo_param.sTrimRange[gt_trim_search_mode].g8_trim_gain_level);
                        }
                        
                        if (screen_iout_spec != 0)
                        {
                            for (ch = 0 ; ch < CH_MAX ; ++ch)
                            {
                                screen_dev[ch] = 100 * (screen_current[ch] - screen_iout_spec) / screen_iout_spec;
                            }    
                        }
                        else
                        {
                            for (ch = 0 ; ch < CH_MAX ; ++ch)
                            {
                                screen_dev[ch] = 0;
                            }
                        }

                        print(LOG_INFO, "%s%4d", CMD_XDIC_SLOPE_ING, gu16_screen_input_cur);
                        
                        for (ch = 0 ; ch < CH_MAX ; ++ch)
                        {
                            print(LOG_INFO, ":%7.4lf", screen_current[ch]);
                        }
                        print(LOG_INFO, ":%7.4lf", screen_iout_spec);
                        for (ch = 0 ; ch < CH_MAX ; ++ch)
                        {
                            print(LOG_INFO, ":%8.3lf", screen_dev[ch]);
                        }
                        print(LOG_INFO, "\r\n");

                        if(gu16_screen_input_cur >= glf_TrimPara_GUI[TRIM_SCREENING][TRIM_PARA_SCR_END])
                        {
                            gt_jig_trimming_step = TRIMMING_STEP_STANDBY;
                            gu16_screen_input_cur = (uint16_t)glf_TrimPara_GUI[TRIM_SCREENING][TRIM_PARA_SCR_START];
                            print(LOG_INFO, "\r\nEND - SCREENING\r\n");
                        }
                        else
                        {
                            gu16_screen_input_cur = TargetIC_IF_Inc_Screen_CurValue(gu16_screen_input_cur);
                            gt_trim_search_mode = TRIM_SCREENING;
                            gt_jig_trimming_step = TRIMMING_STEP_CHANGE_OUTPUT_INIT;
                        }
                    }
                }
                break;

/* TRIMMING_STEP_E2P_PROGRAM  ***************************************/
            case TRIMMING_STEP_E2P_PROGRAM:   /* E2P program */
                #ifdef CHK_TRIM_TIME
                    g_CHK_TRIM_TIME_tick = HAL_GetTick();                    
                    print(LOG_DEBUG, "\r\n CHK_TRIM_TIME-OTP-START:%d\r\n ", g_CHK_TRIM_TIME_tick);
                #endif //CHK_TRIM_TIME 
                
                TargetIC_IF_Show_Trim_Reg();

                if(gub_OTP_WRITE_ENABLE)
                {
                    // TargetIC Power ON - 5.7V
                    JigBd_IF_XD_VCC_Level(PWR_ON_5V7);
                    print(LOG_INFO, "\r\n OTP WRITE - PREPARE!! \r\n");
                    gn_step_delay = 5;
                    gt_jig_trimming_step = TRIMMING_STEP_E2P_PROGRAM_START;
                }
                else
                {
                    gn_step_delay = 5;
                    gt_jig_trimming_step = TRIMMING_STEP_STOP;
                }
                break;
            case TRIMMING_STEP_E2P_PROGRAM_START:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    print(LOG_INFO, "\r\n OTP WRITE - START!! \r\n");
                    TargetIC_IF_Write_OTP_Start();
                    gn_step_delay = 1000;
                    gt_jig_trimming_step = TRIMMING_STEP_E2P_PROGRAM_END;
                }
                break;
            case TRIMMING_STEP_E2P_PROGRAM_END:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    print(LOG_INFO, "\r\n OTP WRITE - DONE!! \r\n");
                    TargetIC_IF_Write_OTP_End();
                    // TargetIC Power ON - 5.0V
                    JigBd_IF_XD_VCC_Level(PWR_ON_5V0);
                    gn_step_delay = 5;
                    gt_jig_trimming_step = TRIMMING_STEP_STOP;
                }
                break;

/* TRIMMING_STEP_STOP  ***************************************/
            case TRIMMING_STEP_STOP:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    JigBd_IF_VLED_9V_EN(PWR_OFF);
                    
                    JigBd_IF_XD_VCC_EN(PWR_OFF);
                    JigBd_IF_XC_VCC_EN(PWR_OFF);

                    #ifdef CHK_TRIM_TIME
                    g_CHK_TRIM_TIME_OTP = HAL_GetTick() - g_CHK_TRIM_TIME_tick;
                    print(LOG_DEBUG, "\r\n\r\nCHK_TRIM_TIME-OTP-END:%d\r\n\r\n", g_CHK_TRIM_TIME_OTP);
                    #endif //CHK_TRIM_TIME

                    TargetIC_IF_SEND_otp_written();
                    gn_step_delay = 1000;

                    gt_jig_trimming_step = TRIMMING_STEP_REBOOT;
                }
                break;

/* TRIMMING_STEP_RESULT  ***************************************/
            case TRIMMING_STEP_RESULT:
                if(gn_trim_error_code == TRIM_ERROR_OVER_COUNT)
                {
                    print(LOG_ERROR, "======== TRIM_ERROR_OVER_COUNT ========\r\n");
                }
                else
                {
                    print(LOG_INFO, "======== TRIM END ========\r\n");
                }
                gt_jig_trimming_step = TRIMMING_STEP_STANDBY;
                break;
                
            case TRIMMING_STEP_REBOOT:
                if(gn_step_delay)
                {
                    --gn_step_delay;
                }
                else
                {
                    gt_jig_trimming_step = TRIMMING_STEP_NONE;
                    NVIC_SystemReset();
                }
                break;
                
            default:
                break;
        }
    }
}
/* END - TRIMMING_PROCEDURE_RUN   *****************************************/

p_gui_param trim_get_param_gui(void) 
{
    return glf_TrimPara_GUI;
}

uint8_t trim_get_trim_ignore_p2_flag(void)
{
    return gn_trim_ignore_p2_flag;
}

void trim_set_trim_ignore_p2_flag(uint8_t n_flag)
{
    gn_trim_ignore_p2_flag = n_flag;
}

#ifdef DBG_TEST

#endif //DBG_TEST

/*** end of file ***/
