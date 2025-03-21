/** @file dimming_test.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __DIMMING_TEST_C__
#include "config.h"

#define MCLK                    (14745600.0f)
#define VSYNC                   (120.0f)
#define XD_ROW_NUM              (12)
#define LD_WIDTH_MAX            (0xFFFF)
#define LD_SUB_FRAME            (192)

#define Hz_TO_ms                ((float)1000)    
#define XD_LD_FRAME             (4)
#define XD_DELAY_LD             (uint8_t)(Hz_TO_ms/VSYNC * XD_LD_FRAME)

//#define DEVIATION_REF_AVG
#define DEVIATION_REF_SPEC

typedef enum
{
    XD12_DIMMING_NORMAL = 0,
    XD12_DIMMING_BFI,
    XD12_DIMMING_X8,
    XD12_DIMMING_MAX,
}_xd12_dimming_type_t;

static const char* gs_dimming_test_str[27] = 
{
    "ldim:",
    "Io_1:",
    "Io_2:",
    "Io_3:",
    "Io_4:",
    "Io_5:",
    "Io_6:",
    "Io_7:",
    "Io_8:",
    "Io_9:",
    "Io_10:",
    "Io_11:",
    "Io_12:",
    "Io_Spec:",
    "Io_Avg:",
    "Dev_1:",
    "Dev_2:",
    "Dev_3:",
    "Dev_4:",
    "Dev_5:",
    "Dev_6:",
    "Dev_7:",
    "Dev_8:",
    "Dev_9:",
    "Dev_10:",
    "Dev_11:",
    "Dev_12:",
};


/* Common */
static float gf_mclk;
static float gf_vsync_out;

static uint16_t gn_xd12_LD_out;
static uint16_t gn_sf_size;
static uint16_t gn_sf_x8_size;
static uint16_t gn_sf_period;
static uint16_t gn_sf_bfi;
static uint16_t gn_dclk_period;

static uint16_t gn_delay_size[XD_CH_SIZE] = {0, };

static _xd12_ld_mode_t gt_xd12_ld_mode;
static _xd12_fault_mode_t gt_xd12_fault_mode;
static _xd12_ld_control_t gt_xd12_ld_control;

static _xd12_dimming_type_t gt_xd12_dimming_type; 

static dimming_step_t gt_jig_dimming_step;
static dimming_step_t gt_jig_debug_dimming_step;

static uint8_t gn_dimming_step_delay;

static uint8_t gn_dimming_output_num;
static uint32_t gn_dimming_LD_data;

static uint16_t gn_dimming_adc[XD_CH_SIZE];
static float gf_dimming_curr[XD_CH_SIZE];

static float gf_dimming_curr_avg;
static float gf_dimming_curr_spec;

static current_gain_t gt_dimming_gain;

static uint16_t gn_xd12_max_vref;
static float gf_xd12_max_current;

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


static void vsync_timer_start()
{
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);
}

static void vsync_timer_stop()
{   
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM8);
}

void vsync_update_handler(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM8);
    gb_vsync_out = true;
}

void XD12_set_dimming_gain(current_gain_t in_gain)
{
    gt_dimming_gain = in_gain;
}

current_gain_t XD12_get_dimming_gain(void)
{
    return gt_dimming_gain;
}

void XD12_set_ldim(uint32_t in_ldim)
{
    gn_dimming_LD_data = in_ldim;
}

uint32_t XD12_get_ldim(void)
{
    return gn_dimming_LD_data;
}

static void XD12_reg_value_init()
{
    gf_mclk = MCLK;
    gf_vsync_out = VSYNC;
    gn_sf_size = 208;    
    
    gn_sf_period = (uint16_t)(gf_mclk / (gf_vsync_out * gn_sf_size) + 0.5f);
    
    gn_dclk_period = (uint16_t)((float)gn_sf_period / 4 + 0.5f);

    gt_xd12_ld_mode.val = 0;
    gt_xd12_dimming_type = XD12_DIMMING_NORMAL; 
    
    if (gt_xd12_dimming_type == XD12_DIMMING_NORMAL)
    {
        gt_xd12_ld_mode.normal = 1;
        gn_sf_bfi = 0;
    }
    else if (gt_xd12_dimming_type == XD12_DIMMING_BFI)
    {
        gt_xd12_ld_mode.bfi = 1;
        gn_sf_bfi = (uint16_t)((float)gn_sf_size/2 + 0.5f);
    }
    else if (gt_xd12_dimming_type == XD12_DIMMING_X8)
    {
        gt_xd12_ld_mode.x8 = 1;
        gn_sf_x8_size = gn_sf_size / 8;
    }

    for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
    {
        //gn_delay_size[ch] = (uint16_t)(((float)gn_sf_size / 12  * ch) + 0.5f);
    }

    gt_xd12_fault_mode.val = 0;
    gt_xd12_fault_mode.o_off_en = 0;
    gt_xd12_fault_mode.s_off_en = 0;
    gt_xd12_fault_mode.t_off_en = 0;
    gt_xd12_fault_mode.s1_en = 0;
    gt_xd12_fault_mode.s2_en = 0;
    gt_xd12_fault_mode.timeout_en = 1;

    gt_xd12_ld_control.val = 0;
    gt_xd12_ld_control.ofs_temp = 0xF;
    gt_xd12_ld_control.ictl_glb = 4;
    gt_xd12_ld_control.bgr_ctl = 8;
}


void XD12_set_LD_out(uint32_t in_ld_out)
{
    if (in_ld_out <= LD_WIDTH_MAX)
    {
        gn_xd12_LD_out = in_ld_out;
    }
    else
    {
        print(LOG_ERROR, "\r\n Out of LD_out [%u] [0 - %u]\r\n", in_ld_out, LD_WIDTH_MAX);
    }
}

uint16_t XD12_get_LD_out(void)
{
    return gn_xd12_LD_out;
}

void XD12_get_fault_status(void)
{
    uint16_t t_fault_status;
    static _xd12_fault_status_t _xd12_fault_status_;
    t_fault_status = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_STATUS, XD12_REG_TYPE_NON_TRIM);
    
    if (t_fault_status != _xd12_fault_status_.val)
    {
        _xd12_fault_status_.val = t_fault_status;
        if (!(_xd12_fault_status_.val))
        {
            print(LOG_INFO, "\r\n XD FAULT Nothing\r\n");
        }
        else
        {
            if (_xd12_fault_status_.bit_fb)
            {
                print(LOG_INFO, "\r\n XD FAULT Detected [FB]\r\n");
            }
            if (_xd12_fault_status_.bit_open)
            {
                print(LOG_INFO, "\r\n XD FAULT Detected [OPEN]\r\n");
            }
            if (_xd12_fault_status_.bit_short)
            {
                print(LOG_INFO, "\r\n XD FAULT Detected [SHORT]\r\n");
            }
            if (_xd12_fault_status_.bit_thermal)
            {
                print(LOG_INFO, "\r\n XD FAULT Detected [THERMAL]\r\n");
            }
        }
    }
}

static double XD12_calculate_deviation(float in_iout, float in_iout_ref)
{
	double ret = 0;
    ret = 100 * ((in_iout / in_iout_ref) - 1);  // unit : %
	return ret; 
}

void XD12_start_dimming_test(void)
{
    if (gt_jig_dimming_step == DIMMING_STEP_NONE)
    {
        gt_jig_dimming_step = DIMMING_STEP_TEST_INIT;
    }
}

void XD12_start_debug_dimming_test(void)
{
    if (gt_jig_debug_dimming_step == DIMMING_STEP_NONE)
    {
        gt_jig_debug_dimming_step = DIMMING_STEP_TEST_INIT;
    }
}


void XD12_set_FB_vled(float n_vled)
{
    if (n_vled > FAULT_FB_VLED_MAX)
    {
        print(LOG_ERROR, "\r\n Out of Fault FB VLED Level [%.2f] [0 - %.1f]\r\n", n_vled, FAULT_FB_VLED_MAX);
        return;
    }
    else
    {
        uint16_t data = 0;
        data = (uint16_t)((n_vled / MCP_V_REF * MCP_R_REF - MCP_R_W) * (MCP_RES / MCP_R_AB) + 0.5f);
        
        float f_res = 0;
        f_res = mcp_set_wiper_resist_value(MCP_WIPER_PORT_0, data);
        print(LOG_DEBUG, "\r\n MCP Wiper Port [FB][%u] - [%.2f]\r\n", MCP_WIPER_PORT_0, f_res);
    }
}

void XD12_set_short_vled(float n_vled)
{
    if (n_vled > FAULT_SHORT_VLED_MAX)
    {
        print(LOG_ERROR, "\r\n Out of Fault Short VLED Level [%.2f] [0 - %.1f]\r\n", n_vled, FAULT_SHORT_VLED_MAX);
        return;
    }
    else
    {
        uint16_t data = 0;
        data = (uint16_t)((n_vled / (MCP_V_REF * 16) * MCP_R_REF - MCP_R_W) * (MCP_RES / MCP_R_AB) + 0.5f);
        
        float f_res = 0;
        f_res = mcp_set_wiper_resist_value(MCP_WIPER_PORT_1, data);
        print(LOG_DEBUG, "\r\n MCP Wiper Port [SHORT][%u] - [%.2f]\r\n", MCP_WIPER_PORT_1, f_res);
    }
}

void dimming_procedure_run(void)
{
    switch (gt_jig_dimming_step)
    {
/* DIMMING_STEP_TEST_INIT  ***************************************/
        case DIMMING_STEP_TEST_INIT : 
            JigBd_IF_XD_VCC_EN(PWR_ON);
            JigBd_IF_VLED_9V_EN(PWR_ON);
            // XC : If use XC, turn on VCC and initialize 
            JigBd_IF_Init();
            
            JigBd_IF_current_discharge(CHARGE);
            JigBd_IF_select_output_ch(CH_MAX);
            
#ifdef USE_LDIM_TABLE
            gn_dimming_LD_table_cnt = 0;
            gn_dimming_LD_data = gn_dimming_LD_table[gn_dimming_LD_table_cnt];
#else
            gn_dimming_LD_data = 0;
#endif
            gn_dimming_output_num = 0;
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
                XD12_reg_value_init();
                
                XD12_Write_sf_period(gn_sf_period);
                XD12_Write_sf_size(((gn_sf_x8_size & 0x0F) >> 0), gn_sf_size);
                XD12_Write_sf_bfi(((gn_sf_x8_size & 0x10) >> 4), gn_sf_bfi);
                XD12_Write_dclk_period(gn_dclk_period);
                
                XD12_Write_ld_mode(gt_xd12_ld_mode);
                XD12_Write_fault_mode(gt_xd12_fault_mode);
                XD12_Write_ld_control(gt_xd12_ld_control);
                
                TargetIC_IF_Fault_Level_Init();
                TargetIC_IF_Serial_Clock_Init();
                TargetIC_IF_MAX_CURR_VREF_Init();
                TargetIC_IF_set_CHANNEL_ENABLE_All();
                for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
                {
                    TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH01 + ch, gn_delay_size[ch], XD12_REG_TYPE_NON_TRIM);
                }
#if 0
                TargetIC_IF_Trim_Reg_Init();
#endif
                TargetIC_IF_Read_Resgister_All(0);

                gn_xd12_max_vref = TargetIC_IF_Read_Register(XD12_ADDR_MAX_CURR_VREF, XD12_REG_TYPE_NON_TRIM);
                print(LOG_INFO, "max_vref:%4u\r\n", gn_xd12_max_vref);
                
                gf_xd12_max_current = XD12_get_max_current_level();
                print(LOG_INFO, "max_current:%3.0f:mA\r\n", gf_xd12_max_current);

                print(LOG_INFO, "%s", CMD_XDIC_GDIM);
                for (uint8_t i = 0 ; i < 27 ; ++i)
                {
                    print(LOG_INFO, "%s", gs_dimming_test_str[i]);
                }
                print(LOG_INFO, "\r\n");

                XD12_set_LD_out(0);
                vsync_timer_start();
                
                gn_dimming_step_delay = 20;
                gn_dimming_output_num = 0;
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
                
                if (gf_dimming_curr_spec < CURRENT_LIMIT_GAIN_LOW)
                {
                    gt_dimming_gain = GAIN_LOW;
                }
                else if (gf_dimming_curr_spec < CURRENT_LIMIT_GAIN_MID)
                {
                    gt_dimming_gain = GAIN_MID;
                }
                else// if (gf_dimming_curr_spec < CURRENT_LIMIT_GAIN_HIGH)
                {
                    gt_dimming_gain = GAIN_HIGH;
                }
                
                JigBd_IF_change_current_gain(gt_dimming_gain);
                JigBd_IF_select_output_ch(gn_dimming_output_num);
                XD12_set_LD_out(gn_dimming_LD_data);
                
                gn_dimming_step_delay = XD_DELAY_LD;
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
                gb_ads114s08_drdy_done = 0;
                gn_ads114s08_adc_temp = 0;
                gn_adc_read_count = ADS114S08_READ_COUNT;

                ads114s08_select_single_ended_input(0);
                ads114s08_set_start(1);
                gt_jig_dimming_step = DIMMING_STEP_GET_ADC;
            }
        break;
        
/* DIMMING_STEP_GET_ADC  ***************************************/
        case DIMMING_STEP_GET_ADC : 
            if(gb_ads114s08_drdy_done == 1)
            {
                gb_ads114s08_drdy_done = 0;
                
                gn_dimming_adc[gn_dimming_output_num] = get_adc_value();
                ++gn_dimming_output_num;
                XD12_set_LD_out(0);
                gt_jig_dimming_step = DIMMING_STEP_ALL_CHANNEL_IS_DONE;
            }
        break;
        
/* DIMMING_STEP_ALL_CHANNEL_IS_DONE  ***************************************/
        case DIMMING_STEP_ALL_CHANNEL_IS_DONE : 
            if (gn_dimming_output_num < CH_MAX)
            {
                gt_jig_dimming_step = DIMMING_STEP_SET_LD_DATA;
            }
            else
            {
                gn_dimming_output_num = 0;
                gt_jig_dimming_step = DIMMING_STEP_LOG;
            }
            gn_dimming_step_delay = XD_DELAY_LD;
        break;
        
/* DIMMING_STEP_LOG  ***************************************/
        case DIMMING_STEP_LOG : 
            print(LOG_INFO, "%s%5d", CMD_XDIC_GDIM, gn_dimming_LD_data);
            
            for (uint8_t ch = 0 ; ch < CH_MAX ; ++ch)
            {
                if (gt_dimming_gain == GAIN_LOW)
                {
                    //gn_dimming_adc[ch] -= gn_ads114s08_offset[ch];
                }
                gf_dimming_curr[ch] = convert_adc_to_current(gn_dimming_adc[ch], gt_dimming_gain);
                gf_dimming_curr_avg += (gf_dimming_curr[ch] / CH_MAX);
                
                print(LOG_INFO, ":%7.3f", gf_dimming_curr[ch]);
            }
            print(LOG_INFO, ":%7.3lf:%7.3lf", gf_dimming_curr_spec, gf_dimming_curr_avg);
            
            for (uint8_t ch = 0 ; ch < CH_MAX ; ++ch)
            {
#ifdef DEVIATION_REF_SPEC
                print(LOG_INFO, ":%7.2lf", XD12_calculate_deviation(gf_dimming_curr[ch], gf_dimming_curr_spec));
#else
                print(LOG_INFO, ":%7.2lf", XD12_calculate_deviation(gf_dimming_curr[ch], gf_dimming_curr_avg));
#endif
            }
            print(LOG_INFO, "\r\n");
            gf_dimming_curr_avg = 0.0f;
            
            if(gn_dimming_LD_data < LD_WIDTH_MAX)
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

                if (gn_dimming_LD_data > LD_WIDTH_MAX)
                {
                    gn_dimming_LD_data = LD_WIDTH_MAX;
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
            gn_dimming_output_num = 0;        
#ifdef USE_LDIM_TABLE
            gn_dimming_LD_table_cnt = 0;
#endif
            XD12_set_LD_out(0);

            vsync_timer_stop();

            JigBd_IF_VLED_9V_EN(PWR_OFF);
            JigBd_IF_XD_VCC_EN(PWR_OFF);
            JigBd_IF_XC_VCC_EN(PWR_OFF);
            print(LOG_INFO, "Dimming Test Done\r\n");
            gt_jig_dimming_step = DIMMING_STEP_NONE;
        break;
        
        default : 
        break;
    }
}

void global_dimming_start(void)
{
    JigBd_IF_XD_VCC_EN(PWR_ON);
    JigBd_IF_Init();
    LL_mDelay(10);

    JigBd_IF_xd12_reset_Command();
    us_tdelay(5);

    JigBd_IF_IdGen_Command();
    XD12_reg_value_init();

    XD12_Write_sf_period(gn_sf_period);
    XD12_Write_sf_size(((gn_sf_x8_size & 0x0F) >> 0), gn_sf_size);
    XD12_Write_sf_bfi(((gn_sf_x8_size & 0x10) >> 4), gn_sf_bfi);
    XD12_Write_dclk_period(gn_dclk_period);

    XD12_Write_ld_mode(gt_xd12_ld_mode);
    XD12_Write_fault_mode(gt_xd12_fault_mode);
    XD12_Write_ld_control(gt_xd12_ld_control);

    TargetIC_IF_Fault_Level_Init();
    TargetIC_IF_Serial_Clock_Init();
    TargetIC_IF_MAX_CURR_VREF_Init();
    TargetIC_IF_set_CHANNEL_ENABLE_All(); 
    
    for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
    {
        TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH01 + ch, gn_delay_size[ch], XD12_REG_TYPE_NON_TRIM);
    }
    
    TargetIC_IF_Read_Resgister_All(0);

    LL_mDelay(10);
    
    XD12_set_LD_out(0);
    vsync_timer_start();
}
/*** end of file ***/
