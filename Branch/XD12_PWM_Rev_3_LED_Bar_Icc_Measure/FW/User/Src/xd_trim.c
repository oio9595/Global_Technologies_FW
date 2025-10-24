
/** @file xd_trim.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XD_TRIM_C__

#include "main.h"
#include "config.h"
#include "xdic.h"
#include "xd_trim.h"
#include "JigBd_IF.h"
#include "ads124s08.h"

/* Trim Spec */
#define XDIC_ERR_RATE               (1.0f / 100)   /* % */
#define XDIC_VREF_TARGET            (2.2)          /* V */
#define XDIC_OSC_TARGET             (XD_MCLK / 1000000)   /* MHz */

#define XDIC_OFS_ERR_RATE           (0.5f / 100)   /* % */
#define XDIC_OFS_P1                 (250)
#define XDIC_OFS_P2                 (500)
#define XDIC_OFS_CAL                (0.13f * 8.0f / 12.0f)
#define XDIC_OFS_TARGET             (8.0f * (XDIC_OFS_P1 + XDIC_OFS_P2) / (XDIC_VREF_MAX * 2.0f) + XDIC_OFS_CAL)   /* mA */

#define XDIC_GAIN_ERR_RATE          (0.5f / 100)   /* % */
#define XDIC_GAIN_P1                (1000)
#define XDIC_GAIN_P2                (3000)
#define XDIC_GAIN_TARGET            (8.0f * (XDIC_GAIN_P2 - XDIC_GAIN_P1) / (XDIC_VREF_MAX))   /* mA */

/* Screen Spec */
#define XDIC_SCREEN_POINT_SIZE      (9)

#define XDIC_SCREEN_VREF1           (350)
#define XDIC_SCREEN_VREF2           (1000)
#define XDIC_SCREEN_VREF3           (4000)

#define XDIC_SCREEN_MAX_CURR1       (DEV_MAX_CURR_LEVEL_4mA)
#define XDIC_SCREEN_MAX_CURR2       (DEV_MAX_CURR_LEVEL_8mA)
#define XDIC_SCREEN_MAX_CURR3       (DEV_MAX_CURR_LEVEL_24mA)

#define TRIM_REGISTER_SAVED_CNT     (5)
#define TRIM_OUT_OF_RANGE_CNT       (25)

#define XD_SCREEN_ANA               (0)
#define XD_SCREEN_MAX_CURRENT       (1)
#define XD_SCREEN_TYPE              XD_SCREEN_ANA

#define XD_SCREEN_ANA_GAP           ((0x0FFF + 1) / 256 - 1)

typedef struct tag_XDIC_SCREEN_PARAM_T
{
    uint16_t vref_point;
    dev_max_curr_level_t max_curr_lvl;
    current_gain_t gain;
    float f_measured[XD_CH_SIZE];
    void (*p_func)(void);
} screen_param_t;

typedef enum tag_TRIM_ADJUST_TYPE_T
{
    ADJ_NONE = 0,
    ADJ_PLUS,
    ADJ_MINUS,
    ADJ_DEFAULT,
    ADJ_MAX,
} trim_adjust_type_t;

typedef struct
{
    uint16_t u16_saved_reg;
    uint16_t u16_saved_adc;
} trim_saved_data;

typedef struct
{
    current_gain_t current_gain;
    uint16_t u16_trim_range_adc_min;
    uint16_t u16_trim_range_adc_target;
    uint16_t u16_trim_range_adc_max;
} trim_adc_range_t;

typedef struct
{
    trim_adc_range_t trim_adc_trange[XD_TRIM_MAX];
    xd_trim_mode_t trim_mode;
    uint8_t u8_channel_cur;
    uint8_t u8_channel_max;
    uint8_t u8_loop_cnt;
    uint8_t trim_saved_cnt;
    uint16_t adc_cur[XD_CH_MAX];
    uint16_t adc_pre[XD_CH_MAX];
    double value[XD_CH_MAX];
    trim_saved_data trim_saved_data[TRIM_REGISTER_SAVED_CNT];
    uint16_t adjust_amount[XD_CH_MAX]; // Result
    trim_adjust_type_t trim_adjust_flag[XD_CH_MAX]; // Result
} trim_algo_param_t;

typedef struct tag_XDIC_TRIM_CONDITION_T
{
    float f_target_min;
    float f_target_max;
    uint16_t u16_p1;
    uint16_t u16_p2;
} xdic_trim_condition_t;

static const char* gs_trim_mode[XD_TRIM_MAX] =
{
    "XD_TRIM_VREF_CTL",
    "XD_TRIM_OSC_FREQUENCY",
    "XD_TRIM_GAIN_CHS",
    "XD_TRIM_OFS_CHS",
};

const static char* gs_trim_algorithm_result[XD_TRIM_MAX] =
{
    "NONE",
    "MINUS",
    "PLUS",
    "TARGET",
};

static bool gb_xd_otp_write_flag;

static xd_trim_step_t gt_xd_trim_step;
static xd_trim_mode_t gt_xd_trim_search_mode;
static trim_error_code_t gt_trim_error_code;

static xd_screen_step_t gt_xd_screen_step;
static uint16_t gn_screen_adc[XD_CH_MAX];
static float gf_screen_current[XD_CH_MAX];
static current_gain_t gt_screen_gain;

#if (XD_SCREEN_TYPE == XD_SCREEN_ANA)
    static uint32_t gn_xd_screen_ana;
#else
    static uint16_t gn_xd_screen_max_current;
#endif

static uint8_t gn_xd_adc_channel;

static uint16_t gn_task_delay;

static uint8_t gn_slope_cnt;
static uint16_t gn_slope_adc[XD_CH_MAX][2];

static trim_algo_param_t gt_trim_algorithm;

static xdic_trim_condition_t gt_xdic_trim_condition[XD_TRIM_MAX];

static screen_param_t gt_xd_screen_param[XDIC_SCREEN_POINT_SIZE];

void XD_Screen_Param_Init(void)
{
    gt_xd_screen_param[0].vref_point = XDIC_SCREEN_VREF1;
    gt_xd_screen_param[0].max_curr_lvl = XDIC_SCREEN_MAX_CURR1;
    gt_xd_screen_param[0].gain = GAIN_MID;

    gt_xd_screen_param[1].vref_point = XDIC_SCREEN_VREF2;
    gt_xd_screen_param[1].max_curr_lvl = XDIC_SCREEN_MAX_CURR1;
    gt_xd_screen_param[1].gain = GAIN_MID;

    gt_xd_screen_param[2].vref_point = XDIC_SCREEN_VREF3;
    gt_xd_screen_param[2].max_curr_lvl = XDIC_SCREEN_MAX_CURR1;
    gt_xd_screen_param[2].gain = GAIN_MID;

    gt_xd_screen_param[3].vref_point = XDIC_SCREEN_VREF1;
    gt_xd_screen_param[3].max_curr_lvl = XDIC_SCREEN_MAX_CURR2;
    gt_xd_screen_param[3].gain = GAIN_HIGH;

    gt_xd_screen_param[4].vref_point = XDIC_SCREEN_VREF2;
    gt_xd_screen_param[4].max_curr_lvl = XDIC_SCREEN_MAX_CURR2;
    gt_xd_screen_param[4].gain = GAIN_HIGH;

    gt_xd_screen_param[5].vref_point = XDIC_SCREEN_VREF3;
    gt_xd_screen_param[5].max_curr_lvl = XDIC_SCREEN_MAX_CURR2;
    gt_xd_screen_param[5].gain = GAIN_HIGH;

    gt_xd_screen_param[6].vref_point = XDIC_SCREEN_VREF1;
    gt_xd_screen_param[6].max_curr_lvl = XDIC_SCREEN_MAX_CURR3;
    gt_xd_screen_param[6].gain = GAIN_HIGH;

    gt_xd_screen_param[7].vref_point = XDIC_SCREEN_VREF2;
    gt_xd_screen_param[7].max_curr_lvl = XDIC_SCREEN_MAX_CURR3;
    gt_xd_screen_param[7].gain = GAIN_HIGH;

    gt_xd_screen_param[8].vref_point = XDIC_SCREEN_VREF3;
    gt_xd_screen_param[8].max_curr_lvl = XDIC_SCREEN_MAX_CURR3;
    gt_xd_screen_param[8].gain = GAIN_HIGH;
}

void XD_Trim_IF_Trim_Start(void)
{
    if (gt_xd_trim_step == XD_TRIM_STEP_NONE)
    {
        gt_xd_trim_step = XD_TRIM_STEP_ACTIVATE_START;
    }
}

void XD_Trim_IF_Screen_Start(void)
{
    if (gt_xd_screen_step == XD_SCREEN_STEP_NONE)
    {
        gt_xd_screen_step = XD_SCREEN_STEP_PWR_ON;
    }
}

void XD_Trim_IF_Set_OTP_Enable(bool in_flag)
{
    gb_xd_otp_write_flag = in_flag;
}

bool XD_Trim_IF_Get_OTP_Enable(void)
{
    return gb_xd_otp_write_flag;
}

/* BEGIN - TRIM ALGORITHM   ***************************************/
static uint8_t XD_Trim_Check_Valid_Step(uint16_t in_step, uint8_t in_channel, uint8_t in_adj_type, xd_trim_mode_t in_trim_mode)
{
    uint8_t ret = TRUE;

    if (in_adj_type == ADJ_PLUS)
    {
        uint16_t u16_register_limit = XDIC_Get_Mirror_Register_Limit_By_Trim_Mode(in_channel, in_trim_mode);

        if ((XDIC_Get_Mirror_Register_By_Trim_Mode(in_channel, in_trim_mode) + in_step) > (u16_register_limit - 1))
        {
            ret = FALSE;
        }
    }
    else if (in_adj_type == ADJ_MINUS)
    {
        if ((XDIC_Get_Mirror_Register_By_Trim_Mode(in_channel, in_trim_mode) - in_step) < (0 + 1))
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

static void XD_Trim_Algorithm_Clear_Buffer_Channel(trim_algo_param_t *ptr_Param)
{
    ptr_Param->u8_loop_cnt = 0;
    ptr_Param->trim_saved_cnt = 0;
    for (int i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
    {
        ptr_Param->trim_saved_data[i].u16_saved_adc = 0;
        ptr_Param->trim_saved_data[i].u16_saved_reg = 0;
    }
}

static void XD_Trim_Algorithm_Clear_Buffer_All(trim_algo_param_t *ptr_Param)
{
    XD_Trim_Algorithm_Clear_Buffer_Channel(ptr_Param);

    for (int i = 0 ; i < XD_CH_MAX ; ++i)
    {
        ptr_Param->adc_cur[i] = 0;
        ptr_Param->adc_pre[i] = 0;
        ptr_Param->adjust_amount[i] = 0;
        ptr_Param->value[i] = 0;
        ptr_Param->trim_adjust_flag[i] = ADJ_DEFAULT;
    }
}

static void XD_Trim_Param_Algorithm_Init(void)
{
    for (xd_trim_mode_t i_trim_mode = XD_TRIM_START ; i_trim_mode < XD_TRIM_MAX ; ++i_trim_mode)
    {
        current_gain_t temp_gain_level = GAIN_LOW;
        uint16_t u16_tmp_trim_range_adc_min = 0;
        uint16_t u16_tmp_trim_range_adc_max = 0;

        double d_tmp_min = gt_xdic_trim_condition[i_trim_mode].f_target_min;
        double d_tmp_max = gt_xdic_trim_condition[i_trim_mode].f_target_max;

        switch(i_trim_mode)
        {
        case XD_TRIM_VREF_CTL:
            temp_gain_level = GAIN_LOW; //Don't Care
            break;
        case XD_TRIM_OSC_FREQUENCY:
            temp_gain_level = GAIN_LOW; //Don't Care
            break;
        case XD_TRIM_GAIN_CHS:
            temp_gain_level = GAIN_HIGH;
            break;
        case XD_TRIM_OFS_CHS:
            temp_gain_level = GAIN_MID;
            break;
        }

        if (i_trim_mode == XD_TRIM_OSC_FREQUENCY) // Freq
        {
            u16_tmp_trim_range_adc_min = JigBD_IF_Calculate_XDIC_Divided_Freq(d_tmp_min);
            u16_tmp_trim_range_adc_max = JigBD_IF_Calculate_XDIC_Divided_Freq(d_tmp_max);
        }
        else if (i_trim_mode == XD_TRIM_VREF_CTL) // Internal ADC
        {
            u16_tmp_trim_range_adc_min = JigBD_IF_Convert_Volt_To_MCU_ADC(d_tmp_min);
            u16_tmp_trim_range_adc_max = JigBD_IF_Convert_Volt_To_MCU_ADC(d_tmp_max);
        }
        else //External ADC
        {
            u16_tmp_trim_range_adc_min = JigBD_IF_Convert_Current_To_ADC(d_tmp_min, temp_gain_level);
            u16_tmp_trim_range_adc_max = JigBD_IF_Convert_Current_To_ADC(d_tmp_max, temp_gain_level);
        }

        if (i_trim_mode == XD_TRIM_OSC_FREQUENCY) // Freq
        {
            gt_trim_algorithm.trim_adc_trange[i_trim_mode].u16_trim_range_adc_target = u16_tmp_trim_range_adc_min;
        }
        else
        {
            gt_trim_algorithm.trim_adc_trange[i_trim_mode].u16_trim_range_adc_target = (uint16_t)(((float)(u16_tmp_trim_range_adc_min + u16_tmp_trim_range_adc_max) / 2) + 0.5f);
        }

        gt_trim_algorithm.trim_adc_trange[i_trim_mode].u16_trim_range_adc_min = u16_tmp_trim_range_adc_min;
        gt_trim_algorithm.trim_adc_trange[i_trim_mode].u16_trim_range_adc_max = u16_tmp_trim_range_adc_max;
        gt_trim_algorithm.trim_adc_trange[i_trim_mode].current_gain = temp_gain_level;
    }

    for (uint8_t ch = 0 ; ch < XD_CH_MAX ; ch++)
    {
        gt_trim_algorithm.trim_adjust_flag[ch] = ADJ_DEFAULT;
    }


    print(LOG_INFO, "\r\n\t[trim_type] - [min/max] [p1/p2]");
    for (xd_trim_mode_t trim_mode = XD_TRIM_START ; trim_mode < XD_TRIM_MAX ; ++trim_mode)
    {
        print(LOG_INFO, "\r\n\t[%s] - [%.3f/%.3f] [%u/%u]", gs_trim_mode[trim_mode],
            1000 * gt_xdic_trim_condition[trim_mode].f_target_min, 1000 * gt_xdic_trim_condition[trim_mode].f_target_max,
            gt_xdic_trim_condition[trim_mode].u16_p1, gt_xdic_trim_condition[trim_mode].u16_p2);
    }
}

static uint8_t XD_Trim_Algorithm_Body(trim_algo_param_t *ptr_Param)
{
    uint16_t u16_adc_range_min = 0;
    uint16_t u16_adc_range_max = 0;
    uint16_t u16_adc_range_target = 0;
    uint16_t u16_reg_value_cur = 0;
    uint8_t channel = 0;
    uint8_t u8_loop_cnt = 0;
    uint8_t u8_CH_MAX = 0;
    uint16_t u16_adc_cur = 0;
    uint16_t u16_adc_pre = 0;
    uint8_t u8_rtn_val = TRIM_ALGORITHM_CONTINUE;
    double temp_value = 0;
    const char *str_trim_result = gs_trim_algorithm_result[0];

    static uint16_t u16_reg_saved[12] = {0, };
    static uint16_t u16_data_gap[12] = {0, };

    u16_adc_range_min = ptr_Param->trim_adc_trange[ptr_Param->trim_mode].u16_trim_range_adc_min;
    u16_adc_range_max = ptr_Param->trim_adc_trange[ptr_Param->trim_mode].u16_trim_range_adc_max;
    u16_adc_range_target = ptr_Param->trim_adc_trange[ptr_Param->trim_mode].u16_trim_range_adc_target;
    u8_CH_MAX = ptr_Param->u8_channel_max;

    ++ptr_Param->u8_loop_cnt;

    u8_loop_cnt = ptr_Param->u8_loop_cnt;
    channel = ptr_Param->u8_channel_cur;

    u16_adc_cur = ptr_Param->adc_cur[channel];
    u16_adc_pre = ptr_Param->adc_pre[channel];

    if (channel < u8_CH_MAX)
    {
        uint8_t temp_saved_cnt = 0;
        uint16_t u16_adc_per_register = 0;
        current_gain_t temp_current_gain = ptr_Param->trim_adc_trange[ptr_Param->trim_mode].current_gain;

        u16_reg_value_cur = XDIC_Get_Mirror_Register_By_Trim_Mode(channel, ptr_Param->trim_mode);

        if (ptr_Param->trim_mode == XD_TRIM_OSC_FREQUENCY)
        {
            ptr_Param->value[channel] = JigBD_IF_Reconvert_XDIC_Original_Freq((double)u16_adc_cur);
        }
        else if (ptr_Param->trim_mode == XD_TRIM_VREF_CTL)
        {
            ptr_Param->value[channel] = JigBD_IF_Convert_MCU_ADC_To_Volt(u16_adc_cur);
        }
        else
        {
            ptr_Param->value[channel] = JigBD_IF_Convert_Adc_To_Current(u16_adc_cur, temp_current_gain);
        }
        temp_value = ptr_Param->value[channel];

        if (u8_loop_cnt != 1) // Initial adc per register
        {
            u16_adc_per_register = (uint16_t)(((float)(abs(u16_adc_pre - u16_adc_cur)) / ptr_Param->adjust_amount[channel]) + 0.5f);
        }
        else
        {
            u16_adc_per_register = 0;
        }

        // Copy current ADC to previous ADC
        ptr_Param->adc_pre[channel] = u16_adc_cur;

        temp_saved_cnt = (ptr_Param->trim_saved_cnt % TRIM_REGISTER_SAVED_CNT);

        // Check ADJ_PLUS
        if (u16_adc_cur < u16_adc_range_target)
        {
            ptr_Param->trim_adjust_flag[channel] = ADJ_PLUS;
            str_trim_result = gs_trim_algorithm_result[2];
            if (u16_adc_per_register == 0)
            {
                ptr_Param->adjust_amount[channel] = 1;
            }
            else
            {
                if (u16_adc_range_target - u16_adc_cur >= u16_adc_per_register * 2)
                {
                    uint16_t adjust_mount = (uint16_t)(((float)(abs(u16_adc_cur - u16_adc_range_target)) / u16_adc_per_register) + 0.5f);
                    if (XD_Trim_Check_Valid_Step( adjust_mount, channel, ADJ_PLUS, ptr_Param->trim_mode) )
                    {
                        ptr_Param->adjust_amount[channel] = (adjust_mount ? adjust_mount : 1);
                    }
                    else
                    {
                        ptr_Param->adjust_amount[channel] = 1;
                    }
                }
                else
                {
                    ptr_Param->adjust_amount[channel] = 1;
                }
            }

            // Check Additional Margin is matched.
            if (u16_adc_cur > u16_adc_range_min)
            {
                ptr_Param->trim_saved_data[temp_saved_cnt].u16_saved_reg = u16_reg_value_cur;
                ptr_Param->trim_saved_data[temp_saved_cnt].u16_saved_adc = u16_adc_cur;
                ++ptr_Param->trim_saved_cnt;
            }
        }
        // Check ADJ_MINUS
        else if (u16_adc_cur > u16_adc_range_target)
        {
            ptr_Param->trim_adjust_flag[channel] = ADJ_MINUS;
            str_trim_result = gs_trim_algorithm_result[1];
            if (u16_adc_per_register == 0)
            {
                ptr_Param->adjust_amount[channel] = 1;
            }
            else
            {
                if (u16_adc_cur - u16_adc_range_target >= u16_adc_per_register * 2)
                {
                    uint16_t adjust_mount = (uint16_t)(((float)(abs(u16_adc_cur - u16_adc_range_target)) / u16_adc_per_register) + 0.5f);
                    if (XD_Trim_Check_Valid_Step(adjust_mount, channel, ADJ_MINUS, ptr_Param->trim_mode ) )
                    {
                        ptr_Param->adjust_amount[channel] = (adjust_mount ? adjust_mount : 1);
                    }
                    else
                    {
                        ptr_Param->adjust_amount[channel] = 1;
                    }
                }
                else
                {
                    ptr_Param->adjust_amount[channel] = 1;
                }
            }

            // Check Additional Margin is matched.
            if (u16_adc_cur < u16_adc_range_max)
            {
                ptr_Param->trim_saved_data[temp_saved_cnt].u16_saved_reg = u16_reg_value_cur;
                ptr_Param->trim_saved_data[temp_saved_cnt].u16_saved_adc = u16_adc_cur;
                ++ptr_Param->trim_saved_cnt;
            }
        }
        // Check ADJ_NONE
        else
        {
            ptr_Param->trim_adjust_flag[channel] = ADJ_NONE;
            str_trim_result = gs_trim_algorithm_result[0];
            ptr_Param->adjust_amount[channel] = 0;

            for (int i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
            {
                if (ptr_Param->trim_saved_data[i].u16_saved_reg == u16_reg_value_cur) //If there is an 2 times matched
                {
                    // Write Register
                    print(LOG_DEBUG, "********Trim Done(%d,%d)********\r\n",channel + 1, ptr_Param->trim_saved_data[i].u16_saved_reg);
                    XDIC_Write_Mirror_Register_By_Trim_Mode(channel, ptr_Param->trim_mode, ptr_Param->trim_saved_data[i].u16_saved_reg);
                    u16_reg_saved[ptr_Param->u8_channel_cur] = ptr_Param->trim_saved_data[i].u16_saved_reg;
                    u16_data_gap[ptr_Param->u8_channel_cur] = 0;
                    ++ptr_Param->u8_channel_cur;
                    XD_Trim_Algorithm_Clear_Buffer_Channel(ptr_Param);
                    u8_rtn_val = TRIM_ALGORITHM_DONE_CHANNEL; // Done - Channel
                    break;
                }
            }

            // If there is not an 2 times matched,
            if (ptr_Param->u8_channel_cur == channel)
            {
                ptr_Param->trim_saved_data[temp_saved_cnt].u16_saved_reg = u16_reg_value_cur;
                ptr_Param->trim_saved_data[temp_saved_cnt].u16_saved_adc = u16_adc_cur;
                ++ptr_Param->trim_saved_cnt;
            }
        }
    }
    // Check Vibration
    if (ptr_Param->trim_saved_cnt >= TRIM_REGISTER_SAVED_CNT)
    {
        uint16_t u16_adc_gap_closest = 0xFFFF;
        uint16_t u16_adc_gap_temp = 0;
        uint8_t u8_closest_adc_index = TRIM_REGISTER_SAVED_CNT;
        for (uint8_t i = 0 ; i < TRIM_REGISTER_SAVED_CNT ; ++i)
        {
            u16_adc_gap_temp = abs(ptr_Param->trim_saved_data[i].u16_saved_adc - u16_adc_range_target);
            if (u16_adc_gap_temp < u16_adc_gap_closest)
            {
                u16_adc_gap_closest = u16_adc_gap_temp;
                u8_closest_adc_index = i;
            }
        }

        if (u8_closest_adc_index == TRIM_REGISTER_SAVED_CNT)
        {
            print(LOG_ERROR, "********ADJUST_OVER_RANGE ERROR(%d,%d)********\r\n", channel + 1, u8_closest_adc_index);
            XD_Trim_Algorithm_Clear_Buffer_Channel(ptr_Param);
            u8_rtn_val = TRIM_ALGORITHM_ERROR;
        }
        else
        {
            // Write Register
            print(LOG_DEBUG, "********Trim Done(%d,%d)********\r\n",channel + 1, ptr_Param->trim_saved_data[u8_closest_adc_index].u16_saved_reg);
            XDIC_Write_Mirror_Register_By_Trim_Mode(channel, ptr_Param->trim_mode, ptr_Param->trim_saved_data[u8_closest_adc_index].u16_saved_reg);
            u16_reg_saved[ptr_Param->u8_channel_cur] = ptr_Param->trim_saved_data[u8_closest_adc_index].u16_saved_reg;
            u16_data_gap[ptr_Param->u8_channel_cur] = u16_adc_gap_closest;
            ++ptr_Param->u8_channel_cur;
            XD_Trim_Algorithm_Clear_Buffer_Channel(ptr_Param);
            u8_rtn_val = TRIM_ALGORITHM_DONE_CHANNEL;
        }
    }

    // Check Limit count of trying
    if (u8_loop_cnt > (TRIM_REGISTER_SAVED_CNT + TRIM_OUT_OF_RANGE_CNT))
    {
        print(LOG_ERROR, "ERROR!! TRIM_CH[%d] : RETRY COUNT is OVER %d\r\n", channel + 1, u8_loop_cnt);
        XD_Trim_Algorithm_Clear_Buffer_All(ptr_Param);
        u8_rtn_val = TRIM_ALGORITHM_ERROR;
        return u8_rtn_val;
    }

    // Print Status
    if (u8_loop_cnt == 1)
    {
        print(LOG_INFO, "Mode : %s / CH : %02d\r\n", gs_trim_mode[ptr_Param->trim_mode], channel + 1);
        if (ptr_Param->trim_mode == XD_TRIM_OSC_FREQUENCY)
        {
            print(LOG_INFO, "[Cnt]\t\t[RANGE]\t\t\t[NOW]\t\t[MHz]\t\t[REG]\t\t[JUDGE]\t\t[Target, Save Count]\r\n");
        }
        else if (ptr_Param->trim_mode == XD_TRIM_VREF_CTL)
        {
            print(LOG_INFO, "[Cnt]\t\t[RANGE]\t\t\t[NOW]\t\t[V]\t\t[REG]\t\t[JUDGE]\t\t[Target, Save Count]\r\n");
        }
        else
        {
            print(LOG_INFO, "[Cnt]\t\t[RANGE]\t\t\t[NOW]\t\t[mA]\t\t[REG]\t\t[JUDGE]\t\t[Target, Save Count]\r\n");
        }
    }
    print(LOG_INFO, "%02d\t\t%5u/%5u\t\t%5u\t\t%.3f\t\t%4u\t\t%7s\t\t[ %5u, %u ]\r\n", u8_loop_cnt,
        u16_adc_range_min, u16_adc_range_max, u16_adc_cur, temp_value,
        u16_reg_value_cur, str_trim_result,
        u16_adc_range_target, ptr_Param->trim_saved_cnt);

    // Check Last Channel
    if (ptr_Param->u8_channel_cur >= u8_CH_MAX)
    {
        print(LOG_INFO, "[%s]\r\n", gs_trim_mode[ptr_Param->trim_mode]);

        print(LOG_INFO, "[RANGE]   %7u   %7u   %7u\r\n",u16_adc_range_min, u16_adc_range_target, u16_adc_range_max);

        print(LOG_INFO,  "[ADC]  ");
        for (uint8_t i = 0 ; i < u8_CH_MAX ; ++i)
        {
            print(LOG_INFO, "   %7u",ptr_Param->adc_cur[i]);
        }
        print(LOG_INFO, "\r\n");

        if (ptr_Param->trim_mode == XD_TRIM_OSC_FREQUENCY)
        {
            print(LOG_INFO, "[MHz] ");
        }
        else if (ptr_Param->trim_mode == XD_TRIM_VREF_CTL)
        {
            print(LOG_INFO, "[V]    ");
        }
        else
        {
            print(LOG_INFO, "[mA]    ");
        }
        for (uint8_t i = 0 ; i < u8_CH_MAX ; ++i)
        {
            print(LOG_INFO, "   %7.3f",ptr_Param->value[i]);
        }
        print(LOG_INFO, "\r\n[Reg]");
        for (uint8_t i = 0 ; i < u8_CH_MAX ; ++i)
        {
            print(LOG_INFO, "   %7u", u16_reg_saved[i]);
        }

        print(LOG_INFO, "\r\n[Gap]");
        for (uint8_t i = 0 ; i < u8_CH_MAX ; ++i)
        {
            print(LOG_INFO, "   %7u", u16_data_gap[i]);
        }
        print(LOG_INFO, "\r\n");

        // Clear Buffers All
        XD_Trim_Algorithm_Clear_Buffer_All(ptr_Param);
        u8_rtn_val = TRIM_ALGORITHM_DONE_MODE; // Done - Mode
    }

    return u8_rtn_val;
}

void XD_Trim_Calculate_Spec(void)
{
    for (xd_trim_mode_t mode = XD_TRIM_START ; mode < XD_TRIM_MAX ; ++mode)
    {
        switch(mode)
        {
        case XD_TRIM_VREF_CTL:
            gt_xdic_trim_condition[mode].f_target_min = (XDIC_VREF_TARGET * (1 - XDIC_ERR_RATE));
            gt_xdic_trim_condition[mode].f_target_max = (XDIC_VREF_TARGET * (1 + XDIC_ERR_RATE));
            gt_xdic_trim_condition[mode].u16_p1 = 0;
            gt_xdic_trim_condition[mode].u16_p2 = 0;
            break;
        case XD_TRIM_OSC_FREQUENCY:
            gt_xdic_trim_condition[mode].f_target_min = (XDIC_OSC_TARGET);
            gt_xdic_trim_condition[mode].f_target_max = (XDIC_OSC_TARGET + 1.5f);
            gt_xdic_trim_condition[mode].u16_p1 = 0;
            gt_xdic_trim_condition[mode].u16_p2 = 0;
            break;
        case XD_TRIM_GAIN_CHS:
            gt_xdic_trim_condition[mode].f_target_min = (XDIC_GAIN_TARGET * (1 - XDIC_GAIN_ERR_RATE)) / 1000;
            gt_xdic_trim_condition[mode].f_target_max = (XDIC_GAIN_TARGET * (1 + XDIC_GAIN_ERR_RATE)) / 1000;
            gt_xdic_trim_condition[mode].u16_p1 = XDIC_GAIN_P1;
            gt_xdic_trim_condition[mode].u16_p2 = XDIC_GAIN_P2;
            break;
        case XD_TRIM_OFS_CHS:
            gt_xdic_trim_condition[mode].f_target_min = (XDIC_OFS_TARGET * (1 - XDIC_OFS_ERR_RATE)) / 1000;
            gt_xdic_trim_condition[mode].f_target_max = (XDIC_OFS_TARGET * (1 + XDIC_OFS_ERR_RATE)) / 1000;
            gt_xdic_trim_condition[mode].u16_p1 = XDIC_OFS_P1;
            gt_xdic_trim_condition[mode].u16_p2 = XDIC_OFS_P2;
            break;
        }
    }
}

void XD_Display_Screen_Result(void)
{
    for (uint8_t point = 0 ; point < XDIC_SCREEN_POINT_SIZE ; ++point)
    {
        for (uint8_t ch = 0 ; ch < XD_CH_MAX ; ++ch)
        {
            print(LOG_INFO, "%7.3f,", gt_xd_screen_param[point].f_measured[ch]);
        }
    }
    print(LOG_INFO, "\r\n");
}
/* END - TRIM ALGORITHM   *****************************************/

/* BEGIN - TRIM_PROCEDURE_RUN   *****************************************/
void XD_Trim_Task(void)
{
    if (gt_xd_trim_step != XD_TRIM_STEP_NONE)
    {
        uint8_t channel = 0;
        uint16_t u16_tmp_adc_cur = 0;
        xd_trim_mode_t t_trim_search_mode_next = XD_TRIM_MAX;
        uint8_t trim_algorithm_result = 0;
        uint8_t u8_tmp_channel_max = 0;
        uint16_t u16_tmp_regVal = 0;
        uint64_t u64_tmp_xd_otp_burn_result = 0xFFFFFFFFFFFFFFFF;
        if (gn_task_delay)
        {
            --gn_task_delay;
        }
        else
        {
            switch(gt_xd_trim_step)
            {
            case XD_TRIM_STEP_ACTIVATE_START:
                gt_trim_error_code = TRIM_ERROR_NONE;
                gn_xd_adc_channel = XD_CH_01;

                JigBD_IF_Select_Output_Ch(XD_CH_MAX);
                JigBD_IF_Change_Current_Gain(GAIN_HIGH);
                XD_Trim_Param_Algorithm_Init();
                XD_Screen_Param_Init();

                print(LOG_INFO, "\r\n======== XD TRIM START ========\r\n");
                gt_xd_trim_step = XD_TRIM_STEP_IC_PWR;
                break;
            case XD_TRIM_STEP_IC_PWR:
                JigBD_IF_Detect_XC24();
                gn_task_delay = 10;
                gt_xd_trim_step = XD_TRIM_STEP_ACTIVATE_END;
                break;
            case XD_TRIM_STEP_ACTIVATE_END:
                XDIC_Trim_Init();
                JigBD_IF_VLED_9V_EN(PWR_ON);
                gn_task_delay = 10;
                gt_xd_trim_step = XD_TRIM_STEP_CHANGE_OUTPUT_INIT;
                break;
            case XD_TRIM_STEP_CHANGE_OUTPUT_INIT:
                switch(gt_xd_trim_search_mode)
                {
                case XD_TRIM_VREF_CTL:
                    XDIC_Trim_Init_VREF_CTL();
                    break;
                case XD_TRIM_OSC_FREQUENCY:
                    XDIC_Trim_Init_OSC();
                    break;
                case XD_TRIM_GAIN_CHS:
                    XDIC_Trim_Init_GAIN_CH();
                    JigBD_IF_Select_Output_Ch(gn_xd_adc_channel);
                    JigBD_IF_Change_Current_Gain(gt_trim_algorithm.trim_adc_trange[gt_xd_trim_search_mode].current_gain);
                    break;
                case XD_TRIM_OFS_CHS:
                    XDIC_Trim_Init_OFS_CH();
                    JigBD_IF_Select_Output_Ch(gn_xd_adc_channel);
                    JigBD_IF_Change_Current_Gain(gt_trim_algorithm.trim_adc_trange[gt_xd_trim_search_mode].current_gain);
                    break;
                }
                gt_xd_trim_step = XD_TRIM_STEP_CHANGE_OUTPUT;
                break;
            case XD_TRIM_STEP_CHANGE_OUTPUT:
                switch(gt_xd_trim_search_mode)
                {
                case XD_TRIM_VREF_CTL:
                    gn_task_delay = 50;
                    gt_xd_trim_step = XD_TRIM_STEP_CHANGE_OUTPUT_DONE;
                    break;
                case XD_TRIM_OSC_FREQUENCY:
                    JigBD_IF_Start_Input_Capture();
                    gn_task_delay = 10;
                    gt_xd_trim_step = XD_TRIM_STEP_CHANGE_OUTPUT_DONE;
                    break;
                case XD_TRIM_GAIN_CHS:
                    if (gn_slope_cnt == 0)
                    {
                        XDIC_Set_Max_Curr_Vref(gt_xdic_trim_condition[gt_xd_trim_search_mode].u16_p1);
                    }
                    else
                    {
                        XDIC_Set_Max_Curr_Vref(gt_xdic_trim_condition[gt_xd_trim_search_mode].u16_p2);
                    }
                    gt_xd_trim_step = XD_TRIM_STEP_SET_ADC_CH;
                    break;
                case XD_TRIM_OFS_CHS:
                    if (gn_slope_cnt == 0)
                    {
                        XDIC_Set_Max_Curr_Vref(gt_xdic_trim_condition[gt_xd_trim_search_mode].u16_p1);
                    }
                    else
                    {
                        XDIC_Set_Max_Curr_Vref(gt_xdic_trim_condition[gt_xd_trim_search_mode].u16_p2);
                    }
                    gt_xd_trim_step = XD_TRIM_STEP_SET_ADC_CH;
                    break;
                default:
                    XDIC_Read_All_Registers();
                    XDIC_Save_Mirror_Regs();
                    gt_xd_trim_step = XD_TRIM_STEP_PWR_OFF;
                    break;
                }
                break;
            case XD_TRIM_STEP_CHANGE_OUTPUT_DONE:
                switch(gt_xd_trim_search_mode)
                {
                case XD_TRIM_VREF_CTL:
                    JigBD_IF_Start_MCU_ADC();
                    gt_xd_trim_step = XD_TRIM_STEP_CHECK;
                    break;
                case XD_TRIM_OSC_FREQUENCY:
                    if (gb_timer_input_capture_done)
                    {
                        JigBD_IF_Stop_Input_Capture();
                        gt_xd_trim_step = XD_TRIM_STEP_CHECK;
                    }
                    break;
                }
                break;
            case XD_TRIM_STEP_SET_ADC_CH:
                ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
                gt_xd_trim_step = XD_TRIM_STEP_START_ADC_CONVERSION;
                break;
            case XD_TRIM_STEP_START_ADC_CONVERSION:
                ADS114S08_Set_Start(1);
                gt_xd_trim_step = XD_TRIM_STEP_GET_ADC_CH;
                break;
            case XD_TRIM_STEP_GET_ADC_CH:
                if (gb_ads114s08_drdy_done)
                {
                    switch(gt_xd_trim_search_mode)
                    {
                    case XD_TRIM_GAIN_CHS:
                    case XD_TRIM_OFS_CHS:
                        gn_slope_adc[gn_xd_adc_channel][gn_slope_cnt] = ADS114S08_Get_ADC_Value();
                        ++gn_slope_cnt;
                        if (gn_slope_cnt >= 2)
                        {
                            gn_slope_cnt = 0;
                            gt_xd_trim_step = XD_TRIM_STEP_CHECK;
                        }
                        else
                        {
                            gt_xd_trim_step = XD_TRIM_STEP_CHANGE_OUTPUT;
                        }
                        break;
                    }
                    gb_ads114s08_drdy_done = 0;
                }
                break;
            case XD_TRIM_STEP_CHECK:
                switch(gt_xd_trim_search_mode)
                {
                case XD_TRIM_VREF_CTL:
                    u8_tmp_channel_max = 1;
                    t_trim_search_mode_next = XD_TRIM_OSC_FREQUENCY;
                    u16_tmp_adc_cur =  JigBD_IF_Get_MCU_ADC();
                    break;
                case XD_TRIM_OSC_FREQUENCY:
                    u8_tmp_channel_max = 1;
                    t_trim_search_mode_next = XD_TRIM_GAIN_CHS;
                    u16_tmp_adc_cur = (uint16_t)(JigBD_IF_Get_Input_Capture_Freq()); // Intentionally rounded down the frequency to make it appear slower
                    break;
                case XD_TRIM_GAIN_CHS:
                    u8_tmp_channel_max = XD_CH_MAX;
                    t_trim_search_mode_next = XD_TRIM_OFS_CHS;
                    u16_tmp_adc_cur = gn_slope_adc[gn_xd_adc_channel][1] - gn_slope_adc[gn_xd_adc_channel][0]; //delta of 2-point
                    break;
                case XD_TRIM_OFS_CHS:
                    u8_tmp_channel_max = XD_CH_MAX;
                    t_trim_search_mode_next = XD_TRIM_MAX;
                    u16_tmp_adc_cur = (uint16_t)(((float)(gn_slope_adc[gn_xd_adc_channel][1] + gn_slope_adc[gn_xd_adc_channel][0]) / 2.0f) + 0.5f); //average of 2-point
                    break;
                }

                gt_trim_algorithm.u8_channel_max = u8_tmp_channel_max;
                gt_trim_algorithm.trim_mode = gt_xd_trim_search_mode;
                gt_trim_algorithm.u8_channel_cur = gn_xd_adc_channel;
                gt_trim_algorithm.adc_cur[gn_xd_adc_channel] = u16_tmp_adc_cur;

                //Run Trim Algorithm
                trim_algorithm_result = XD_Trim_Algorithm_Body(&gt_trim_algorithm);

                if (trim_algorithm_result == TRIM_ALGORITHM_ERROR)
                {
                    print(LOG_ERROR, "\t  ERROR STOP\r\n");
                    gt_xd_trim_step = XD_TRIM_STEP_PWR_OFF;
                }
                else if (trim_algorithm_result == TRIM_ALGORITHM_CONTINUE)
                {
                    //print(LOG_DEBUG, "\t  REPEAT \r\n");
                    gt_xd_trim_step = XD_TRIM_STEP_CHANGE_REGISTER;
                }
                else if (trim_algorithm_result == TRIM_ALGORITHM_DONE_CHANNEL)
                {
                    print(LOG_INFO, "\t  Next Channel \r\n");
                    ++gn_xd_adc_channel;
                    gt_xd_trim_step = XD_TRIM_STEP_CHANGE_OUTPUT_INIT;
                }
                else if (trim_algorithm_result == TRIM_ALGORITHM_DONE_MODE)
                {
                    print(LOG_INFO, "\t  Next trim_search_mode \r\n");
                    if (t_trim_search_mode_next == XD_TRIM_MAX)
                    {
                        gt_xd_trim_step = XD_TRIM_STEP_SCREEN;
                    }
                    else
                    {
                        gt_xd_trim_step = XD_TRIM_STEP_CHANGE_OUTPUT_INIT;
                    }
                    gn_xd_adc_channel = 0;
                    gt_xd_trim_search_mode = t_trim_search_mode_next;
                }
                break;
            case XD_TRIM_STEP_CHANGE_REGISTER:
                channel = 0;
                u16_tmp_regVal = 0;
                switch(gt_xd_trim_search_mode)
                {
                case XD_TRIM_VREF_CTL:
                case XD_TRIM_OSC_FREQUENCY:
                    channel = XD_CH_01;
                    break;
                case XD_TRIM_GAIN_CHS:
                case XD_TRIM_OFS_CHS:
                    channel = gn_xd_adc_channel;
                    break;
                }

                u16_tmp_regVal = XDIC_Get_Mirror_Register_By_Trim_Mode(channel, gt_xd_trim_search_mode);
                if (gt_trim_algorithm.trim_adjust_flag[channel] == ADJ_PLUS)
                {
                    uint16_t u16_tmp_limit = XDIC_Get_Mirror_Register_Limit_By_Trim_Mode(channel, gt_xd_trim_search_mode);
                    u16_tmp_regVal += gt_trim_algorithm.adjust_amount[channel];

                    if (u16_tmp_regVal <= u16_tmp_limit)
                    {
                        XDIC_Write_Mirror_Register_By_Trim_Mode(channel, gt_xd_trim_search_mode, u16_tmp_regVal);
                    }
                    else
                    {
                        print(LOG_ERROR, "ERROR: TRIM STOP - OVER LIMIT-%d:[%d/%d]\r\n", gt_xd_trim_search_mode ,u16_tmp_regVal ,u16_tmp_limit);
                        gt_trim_error_code = TRIM_ERROR_OVER_COUNT;
                    }
                }
                else if (gt_trim_algorithm.trim_adjust_flag[channel] == ADJ_MINUS)
                {
                    if (u16_tmp_regVal >= gt_trim_algorithm.adjust_amount[channel])
                    {
                        u16_tmp_regVal -= gt_trim_algorithm.adjust_amount[channel];
                        XDIC_Write_Mirror_Register_By_Trim_Mode(channel, gt_xd_trim_search_mode, u16_tmp_regVal);
                    }
                    else
                    {
                        print(LOG_ERROR, "ERROR: TRIM STOP - UNDER ZERO-%d:[%d-%d]\r\n", gt_xd_trim_search_mode ,u16_tmp_regVal ,gt_trim_algorithm.adjust_amount[channel]);
                        gt_trim_error_code = TRIM_ERROR_UNDER_COUNT;
                    }
                }
                if (gt_trim_error_code == TRIM_ERROR_NONE)
                {
                    gn_task_delay = 5;
                    gt_xd_trim_step = XD_TRIM_STEP_CHANGE_OUTPUT;
                }
                else
                {
                    gt_xd_trim_step = XD_TRIM_STEP_RESULT;
                }
                break;
            case XD_TRIM_STEP_SCREEN:
                ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
                for (uint8_t i = 0 ; i < XDIC_SCREEN_POINT_SIZE ; ++i)
                {
                    JigBD_IF_Change_Current_Gain(gt_xd_screen_param[i].gain);
                    XDIC_Set_Max_Current_Level(gt_xd_screen_param[i].max_curr_lvl);
                    for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
                    {
                        JigBD_IF_Select_Output_Ch(ch);
                        XDIC_Set_Max_Curr_Vref(gt_xd_screen_param[i].vref_point);
                        gb_ads114s08_drdy_done = 0;
                        gn_ads114s08_adc_temp = 0;
                        gn_adc_read_count = ADS114S08_READ_COUNT;
                        LL_mDelay(0);

                        ADS114S08_Set_Start(1);
                        while(1)
                        {
                            if (gb_ads114s08_drdy_done == 1)
                            {
                                gb_ads114s08_drdy_done = 0;
                                break;
                            }
                        }
                        XDIC_Set_Max_Curr_Vref(0);
                        uint16_t adc = ADS114S08_Get_ADC_Value();
                        gt_xd_screen_param[i].f_measured[ch] = JigBD_IF_Convert_Adc_To_Current(adc, gt_xd_screen_param[i].gain);
                    }
                }

                if (gb_xd_otp_write_flag)
                {
                    gt_xd_trim_step = XD_TRIM_STEP_E2P_PROGRAM;
                }
                else
                {
                    gt_xd_trim_step = XD_TRIM_STEP_PWR_OFF;
                }
                break;
            case XD_TRIM_STEP_E2P_PROGRAM:   /* E2P program */
                if (gb_xd_otp_write_flag)
                {
                    XDIC_Save_Mirror_Regs();
                    JigBD_IF_XD_VCC_Level(PWR_ON_5V5);
                    print(LOG_INFO, "\r\n OTP WRITE - ENABLE!! \r\n");
                    gn_task_delay = 100;
                    gt_xd_trim_step = XD_TRIM_STEP_E2P_PROGRAM_START;
                }
                else
                {
                    print(LOG_INFO, "\r\n OTP WRITE - DISABLE!! \r\n");
                    gt_xd_trim_step = XD_TRIM_STEP_PWR_OFF;
                }
                break;
            case XD_TRIM_STEP_E2P_PROGRAM_START:
                print(LOG_INFO, "\r\n OTP WRITE - START!! \r\n");
                XDIC_Set_OTP_PG_Start(true);
                gn_task_delay = 1000;
                gt_xd_trim_step = XD_TRIM_STEP_E2P_PROGRAM_END;
                break;
            case XD_TRIM_STEP_E2P_PROGRAM_END:
                print(LOG_INFO, "\r\n OTP WRITE - DONE!! \r\n");
                XDIC_Set_OTP_PG_Start(false);
                JigBD_IF_XD_VCC_Level(PWR_ON_5V0);
                gn_task_delay = 10;
                gt_xd_trim_step = XD_TRIM_STEP_STOP;
                break;
            case XD_TRIM_STEP_STOP:
                JigBD_IF_VLED_9V_EN(PWR_OFF);
                JigBD_IF_XD_VCC_EN(PWR_OFF);
                JigBD_IF_XC_VCC_EN(PWR_OFF);
                gn_task_delay = 100;
                gt_xd_trim_step = XD_TRIM_STEP_REBOOT;
                break;
            case XD_TRIM_STEP_REBOOT:
                JigBD_IF_Detect_XC24();
                XDIC_Trim_Init();
                JigBD_IF_VLED_9V_EN(PWR_ON);
                gn_task_delay = 100;
                gt_xd_trim_step = XD_TRIM_STEP_COMPARE;
                break;
            case XD_TRIM_STEP_COMPARE:
                u64_tmp_xd_otp_burn_result = XDIC_Compare_Mirror_Regs();
                if (u64_tmp_xd_otp_burn_result)
                {
                    print(LOG_ERROR, "======== TRIM_OTP_BURN_ERROR[%llu] ========\r\n", u64_tmp_xd_otp_burn_result);
                }
                else
                {
                    print(LOG_INFO, "%s======== TRIM_OTP_BURN_OK[%llu] ========%s\r\n", ANSI_FONT_GREEN, u64_tmp_xd_otp_burn_result, ANSI_FONT_NONE);
                }
                gt_xd_trim_step = XD_TRIM_STEP_PWR_OFF;
                break;
            case XD_TRIM_STEP_RESULT:
                if (gt_trim_error_code == TRIM_ERROR_OVER_COUNT)
                {
                    print(LOG_ERROR, "======== TRIM_ERROR_OVER_COUNT ========\r\n");
                }
                else
                {
                    print(LOG_INFO, "======== TRIM END ========\r\n");
                }
                gt_xd_trim_step = XD_TRIM_STEP_PWR_OFF;
                break;
            case XD_TRIM_STEP_PWR_OFF:
                XDIC_Save_Mirror_Regs();
                XDIC_Display_Mirror_Regs();
                XD_Display_Screen_Result();

                JigBD_IF_VLED_9V_EN(PWR_OFF);
                JigBD_IF_XD_VCC_EN(PWR_OFF);
                JigBD_IF_XC_VCC_EN(PWR_OFF);
                print(LOG_INFO, "======== TRIM END ========\r\n");
                gt_xd_trim_step = XD_TRIM_STEP_NONE;
                break;
            default:
                break;
            }
        }
    }
}
/* END - TRIM_PROCEDURE_RUN   *****************************************/
void XD_Screen_Task(void)
{
    if (gn_task_delay)
    {
        --gn_task_delay;
    }
    else
    {
        switch(gt_xd_screen_step)
        {
        case XD_SCREEN_STEP_PWR_ON :
            JigBD_IF_Detect_XC24();
            gn_xd_adc_channel = 0;
            gn_task_delay = 10;
            gt_xd_screen_step = XD_SCREEN_STEP_SETUP;
            break;
        case XD_SCREEN_STEP_SETUP :
            XDIC_Trim_Init();
            XDIC_Trim_Init_OFS_CH();
            XDIC_Overwrite_Mirror_Regs();

            XDIC_Set_Max_Current_Level(DEV_MAX_CURR_LEVEL_24mA);

            XDIC_Read_All_Registers();

            XDIC_Display_Mirror_Regs();

            gt_screen_gain = GAIN_HIGH;
            JigBD_IF_Change_Current_Gain(gt_screen_gain);
            JigBD_IF_VLED_9V_EN(PWR_ON);
            ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
            gt_xd_screen_step = XD_SCREEN_STEP_CHANGE_OUTPUT;
            #if (XD_SCREEN_TYPE == XD_SCREEN_ANA)
                print(LOG_INFO, "max_curr, %.3f\r\n", XDIC_Get_Max_Current_level());
            #else
                print(LOG_INFO, "vref, %4u\r\n", XDIC_CURRENT_TRIM_VREF);
            #endif
            print(LOG_INFO, "data,  io_1,  io_2,  io_3,  io_4,  io_5,  io_6,  io_7,  io_8,  io_9,  io_10,  io_11,  io_12\r\n");
            break;
        case XD_SCREEN_STEP_CHANGE_OUTPUT :
            #if (XD_SCREEN_TYPE == XD_SCREEN_ANA)
                XDIC_Set_Max_Curr_Vref(gn_xd_screen_ana);
            #else
                XDIC_Set_Max_Current_Level(gn_xd_screen_max_current);
            #endif
            gt_xd_screen_step = XD_SCREEN_STEP_SET_ADC_CH;
            break;
        case XD_SCREEN_STEP_SET_ADC_CH :
            JigBD_IF_Select_Output_Ch(gn_xd_adc_channel);
            gt_xd_screen_step = XD_SCREEN_STEP_START_ADC_CONVERSION;
            break;
        case XD_SCREEN_STEP_START_ADC_CONVERSION :
            ADS114S08_Set_Start(1);
            gt_xd_screen_step = XD_SCREEN_STEP_GET_ADC_CH;
            break;
        case XD_SCREEN_STEP_GET_ADC_CH :
            if (gb_ads114s08_drdy_done)
            {
                gn_screen_adc[gn_xd_adc_channel] = ADS114S08_Get_ADC_Value();
                ++gn_xd_adc_channel;
                if (gn_xd_adc_channel < XD_CH_MAX)
                {
                    gt_xd_screen_step = XD_SCREEN_STEP_SET_ADC_CH;
                }
                else
                {
                    gn_xd_adc_channel = 0;
                    gt_xd_screen_step = XD_SCREEN_STEP_CHANGE_OUTPUT;

                    for (uint8_t ch = 0 ; ch < XD_CH_MAX ; ++ch)
                    {
                        gf_screen_current[ch] = JigBD_IF_Convert_Adc_To_Current(gn_screen_adc[ch], gt_screen_gain);
                    }

                    #if (XD_SCREEN_TYPE == XD_SCREEN_ANA)
                        print(LOG_INFO, "%4u, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\r\n", gn_xd_screen_ana, \
                            gf_screen_current[ 0], gf_screen_current[ 1], gf_screen_current[ 2], gf_screen_current[ 3],
                            gf_screen_current[ 4], gf_screen_current[ 5], gf_screen_current[ 6], gf_screen_current[ 7],
                            gf_screen_current[ 8], gf_screen_current[ 9], gf_screen_current[10], gf_screen_current[11]);

                        gn_xd_screen_ana += XD_SCREEN_ANA_GAP;
                        if (gn_xd_screen_ana > 0xFFF)
                        {
                            gt_xd_screen_step = XD_SCREEN_STEP_STOP;
                        }
                    #else
                        print(LOG_INFO, "%4u, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\r\n", gn_xd_screen_max_current, \
                            gf_screen_current[ 0], gf_screen_current[ 1], gf_screen_current[ 2], gf_screen_current[ 3],
                            gf_screen_current[ 4], gf_screen_current[ 5], gf_screen_current[ 6], gf_screen_current[ 7],
                            gf_screen_current[ 8], gf_screen_current[ 9], gf_screen_current[10], gf_screen_current[11]);

                        ++gn_xd_screen_max_current;
                        if (gn_xd_screen_max_current > 8)
                        {
                            gt_xd_screen_step = XD_SCREEN_STEP_STOP;
                        }
                    #endif
                }
                gb_ads114s08_drdy_done = 0;
            }
            break;
        case XD_SCREEN_STEP_STOP :
            JigBD_IF_VLED_9V_EN(PWR_OFF);
            JigBD_IF_XD_VCC_EN(PWR_OFF);
            JigBD_IF_XC_VCC_EN(PWR_OFF);
            print(LOG_INFO, "======== SCREEN DONE ========\r\n");
            gt_xd_screen_step = XD_SCREEN_STEP_NONE;
            break;
        default :
            break;
        }
    }
}
/*** end of file ***/