/** @file dimming_test.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __DIMMING_TEST_C__
#include "config.h"

#define LD_WIDTH_MAX            (0xFFFF)
#define LD_SUB_FRAME            (192)

#define Hz_TO_ms                ((float)1000)
#define XD_LD_FRAME             (4)
#define XD_DELAY_LD             (uint8_t)(Hz_TO_ms/VSYNC * XD_LD_FRAME + 0.5f)

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

static uint16_t gn_xd12_LD_out;

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
    static uint16_t vsync_tick = 0;
#if 0
    uint16_t now_fault_status_reg_read = 0;
    static _xd12_fault_status_t prev_xd12_fault_status_ = {1, };
    now_fault_status_reg_read = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_STATUS, XD12_REG_TYPE_NON_TRIM);

    if (now_fault_status_reg_read != prev_xd12_fault_status_.val)
    {
        prev_xd12_fault_status_.val = now_fault_status_reg_read;
        if (!(prev_xd12_fault_status_.val))
        {
            print(LOG_INFO, "\r\n [%u] XD FAULT Nothing [REG READ]\r\n", vsync_tick);
        }
        else
        {
            if (prev_xd12_fault_status_.bit_fb)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [FB] [REG READ]\r\n", vsync_tick);
            }
            if (prev_xd12_fault_status_.bit_open)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [OPEN] [REG READ]\r\n", vsync_tick);
            }
            if (prev_xd12_fault_status_.bit_short)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [SHORT] [REG READ]\r\n", vsync_tick);
            }
            if (prev_xd12_fault_status_.bit_thermal)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [THERMAL] [REG READ]\r\n", vsync_tick);
            }
            if (prev_xd12_fault_status_.bit_miss_vs)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [MISS_VS] [REG READ]\r\n", vsync_tick);
            }
        }
    }
#endif
    uint8_t now_fault_status_command_read = 0;
    static uint8_t prev_fault_status_command_read = 0xFF;
    now_fault_status_command_read = (JigBd_IF_Fault_Read_Command() & 0x0F);

    if (now_fault_status_command_read != prev_fault_status_command_read)
    {
        if (!now_fault_status_command_read)
        {
            print(LOG_INFO, "\r\n [%u] XD FAULT Nothing [CMD READ]\r\n", vsync_tick);
        }
        else
        {
            if (now_fault_status_command_read & 0x1)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [FB] [CMD READ]\r\n", vsync_tick);
            }
            if (now_fault_status_command_read & 0x2)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [OPEN] [CMD READ]\r\n", vsync_tick);
            }
            if (now_fault_status_command_read & 0x4)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [SHORT] [CMD READ]\r\n", vsync_tick);
            }
            if (now_fault_status_command_read & 0x8)
            {
                print(LOG_INFO, "\r\n [%u] XD FAULT Detected [THERMAL] [CMD READ]\r\n", vsync_tick);
            }
        }
        prev_fault_status_command_read = now_fault_status_command_read;
    }

    //XD12_detect_FBO();
#if 0
    static uint16_t prev_fbo = 0xFFFF;
    uint16_t now_fbo = XD_FBO_READ();

    if (prev_fbo != now_fbo)
    {
        if (now_fbo)
        {
            print(LOG_INFO, "\r\n [%u] FBO --> HI\r\n", vsync_tick);
        }
        else
        {
            print(LOG_INFO, "\r\n [%u] FBO --> LOW\r\n", vsync_tick);
        }
        prev_fbo = now_fbo;
    }
#endif
    ++vsync_tick;
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
            JigBd_IF_XD_VCC_Level(PWR_ON_5V0);
            JigBd_IF_XD_VCC_EN(PWR_ON);
            // XC : If use XC, turn on VCC and initialize
            JigBd_IF_Detect_XC24();

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
                XD12_Initialize();

                TargetIC_IF_XD_Regs_Read_Display();

                JigBd_IF_VLED_9V_EN(PWR_ON);

                gn_xd12_max_vref = TargetIC_IF_Read_Register(XD12_ADDR_MAX_CURR_VREF, XD12_REG_TYPE_NON_TRIM);
                print(LOG_INFO, "max_vref:%4u\r\n", gn_xd12_max_vref);

                gf_xd12_max_current = XD12_get_max_current_level();
                print(LOG_INFO, "max_current:%3.0f:mA\r\n", gf_xd12_max_current);

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
            print(LOG_INFO, "%5d", gn_dimming_LD_data);

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
    vsync_timer_start();
}
/*** end of file ***/
