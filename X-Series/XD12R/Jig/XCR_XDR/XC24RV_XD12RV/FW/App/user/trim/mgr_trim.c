#include <math.h>

#include "framework.h"
#include "drv_gpio.h"
#include "drv_xdr12.h"
#include "comm_debugging.h"
#include "ads124s08.h"

#define SAVE_INFO_MAX_CNT    (100U)

#define XDR_DEFAULT_SUB_VAL_CURRENT_REF     (1U << 4U)
#define XDR_DEFAULT_SUB_VAL_LDO_DIG         (1U << 3U)
#define XDR_DEFAULT_SUB_VAL_LDO_DAC         (1U << 4U)
#define XDR_DEFAULT_SUB_VAL_LDO_FLL         (1U << 3U)
#define XDR_DEFAULT_SUB_VAL_OSC             (1U << 4U)
#define XDR_DEFAULT_SUB_VAL_CH_GAIN         (1U << 6U)
#define XDR_DEFAULT_SUB_VAL_CH_OFS          (1U << 8U)

#define XDR_TRIM_ERROR_RANGE        (5.0f)      /* 5% */

#define XDR_TRIM_TGT_CURRENT_REF    (1.4f)      /* 1.4 V */
#define XDR_TRIM_TGT_LDO_DIG        (1.5f)      /* 1.5 V */
#define XDR_TRIM_TGT_LDO_DAC        (1.5f)      /* 1.5 V */
#define XDR_TRIM_TGT_LDO_FLL        (1.5f)      /* 1.5 V */
#define XDR_TRIM_TGT_OSC            (40.0f)     /* 40.0 MHz */
#define XDR_TRIM_TGT_CH_GAIN        (4.688f)    /* 4.688 mA */
#define XDR_TRIM_TGT_CH_OFS         (1.172f)    /* 1.172 mA */

#define XDR_TRIM_GAIN_INPUT_1       (300U)
#define XDR_TRIM_GAIN_INPUT_2       (1500U)

#define XDR_TRIM_OFS_INPUT_1        (200U)
#define XDR_TRIM_OFS_INPUT_2        (400U)

#define XDR_DELAY_DEFAULT           (1U)
#define XDR_DELAY_MEASURE           (100U)
#define XDR_DELAY_PWR_ON            (100U)
#define XDR_DELAY_PWR_OFF           (100U)
#define XDR_DELAY_EFUSE_DONE        (500U)

typedef struct tag_SAVED_INFO
{
    float saved_value[SAVE_INFO_MAX_CNT];
    uint16_t sub_val[SAVE_INFO_MAX_CNT];
    uint8_t saved_cnt;
} saved_info_t;

typedef struct tag_JUDGE_INFO
{
    bool in_range;
    bool up;
    bool down;
} judge_info_t;

typedef struct tag_RANGE_INFO
{
    float target;
    float min;
    float max;
} range_info_t;

typedef struct tag_MEASURE_INFO
{
    uint16_t adc[2];
    float value[2];
} measure_info_t;

typedef struct tag_TRIM_INFO
{
    uint16_t input[2];
    uint8_t repeat;
    XD_CH_t chx;
    current_gain_t gain;
    measure_info_t measure;
    range_info_t range;
    saved_info_t saved;
    uint16_t sub_val;
} trim_info_t;

/* XCR Trim */
typedef enum tag_XCR_TRIM_STEP_T
{
    XCR_TRIM_STEP_PWR_ON = 0U,
    XCR_TRIM_STEP_EXAMPLE1,
    XCR_TRIM_STEP_EXAMPLE2,
    XCR_TRIM_STEP_EXAMPLE3,
    XCR_TRIM_STEP_NONE,
} xcr_trim_step_t;

/* XDR Trim */
typedef enum tag_XDR_TRIM_STEP_T
{
    XDR_TRIM_STEP_PWR_ON = 0U,
    XDR_TRIM_STEP_INITIAL,
    XDR_TRIM_STEP_INITIAL_BY_LIST,
    XDR_TRIM_STEP_START_MEASURE,
    XDR_TRIM_STEP_GET_MEASURED_VALUE,
    XDR_TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER,
    XDR_TRIM_STEP_PREPARE_EFUSE,
    XDR_TRIM_STEP_START_EFUSE,
    XDR_TRIM_STEP_END_EFUSE,
    XDR_TRIM_STEP_PWR_REBOOT,
    XDR_TRIM_STEP_VERIFY_OTP_DUMP,
    XDR_TRIM_STEP_LOG_SUMMARY,
    XDR_TRIM_STEP_PWR_OFF,
    XDR_TRIM_STEP_NONE,
    XDR_TRIM_STEP_MAX,
} xdr_trim_step_t;

typedef enum tag_XDR_TRIM_LIST
{
    XDR_TRIM_LIST_CURRENT_REF = 0U,
    XDR_TRIM_LIST_LDO_DIG,
    XDR_TRIM_LIST_LDO_DAC,
    XDR_TRIM_LIST_LDO_FLL,
    XDR_TRIM_LIST_OSC,
    XDR_TRIM_LIST_CH_GAIN,
    XDR_TRIM_LIST_CH_OFS,
    XDR_TRIM_LIST_MAX,
} xdr_trim_list_t;

static struct{
    MGRSTATUS   status;

    trim_info_t t_trim_info[XDR_TRIM_LIST_MAX];
    xdr_trim_list_t t_xdr_trim_list;

    THREAD_ID   trim_thr;
}__priv_trim;

static bool gb_xdr_do_efuse;

static const uint8_t* gs_xdr_trim_step[XDR_TRIM_STEP_MAX] =
{
    "XDR_TRIM_STEP_PWR_ON",
    "XDR_TRIM_STEP_INITIAL",
    "XDR_TRIM_STEP_INITIAL_BY_LIST",
    "XDR_TRIM_STEP_START_MEASURE",
    "XDR_TRIM_STEP_GET_MEASURED_VALUE",
    "XDR_TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER",
    "XDR_TRIM_STEP_PREPARE_EFUSE",
    "XDR_TRIM_STEP_START_EFUSE",
    "XDR_TRIM_STEP_END_EFUSE",
    "XDR_TRIM_STEP_PWR_REBOOT",
    "XDR_TRIM_STEP_VERIFY_OTP_DUMP",
    "XDR_TRIM_STEP_LOG_SUMMARY",
    "XDR_TRIM_STEP_PWR_OFF",
    "XDR_TRIM_STEP_NONE",
};

static const uint8_t* gs_xdr_trim_list[XDR_TRIM_LIST_MAX] =
{
    "XDR_TRIM_LIST_CURRENT_REF",
    "XDR_TRIM_LIST_LDO_DIG",
    "XDR_TRIM_LIST_LDO_DAC",
    "XDR_TRIM_LIST_LDO_FLL",
    "XDR_TRIM_LIST_OSC",
    "XDR_TRIM_LIST_CH_GAIN",
    "XDR_TRIM_LIST_CH_OFS",
};

static void xdr_trim_param_init(void)
{
    for (xdr_trim_list_t trim_list = XDR_TRIM_LIST_CURRENT_REF ; trim_list < XDR_TRIM_LIST_MAX; ++trim_list)
    {
        switch (trim_list)
        {
            case XDR_TRIM_LIST_CURRENT_REF:
                __priv_trim.t_trim_info[trim_list].sub_val = XDR_DEFAULT_SUB_VAL_CURRENT_REF;
                __priv_trim.t_trim_info[trim_list].range.target = XDR_TRIM_TGT_CURRENT_REF;
                __priv_trim.t_trim_info[trim_list].range.min = XDR_TRIM_TGT_CURRENT_REF - (XDR_TRIM_TGT_CURRENT_REF * XDR_TRIM_ERROR_RANGE / 100.0f);
                __priv_trim.t_trim_info[trim_list].range.max = XDR_TRIM_TGT_CURRENT_REF + (XDR_TRIM_TGT_CURRENT_REF * XDR_TRIM_ERROR_RANGE / 100.0f);
                break;
            case XDR_TRIM_LIST_LDO_DIG:
                __priv_trim.t_trim_info[trim_list].sub_val = XDR_DEFAULT_SUB_VAL_LDO_DIG;
                __priv_trim.t_trim_info[trim_list].range.target = XDR_TRIM_TGT_LDO_DIG;
                __priv_trim.t_trim_info[trim_list].range.min = XDR_TRIM_TGT_LDO_DIG - (XDR_TRIM_TGT_LDO_DIG * XDR_TRIM_ERROR_RANGE / 100.0f);
                __priv_trim.t_trim_info[trim_list].range.max = XDR_TRIM_TGT_LDO_DIG + (XDR_TRIM_TGT_LDO_DIG * XDR_TRIM_ERROR_RANGE / 100.0f);
                break;
            case XDR_TRIM_LIST_LDO_DAC:
                __priv_trim.t_trim_info[trim_list].sub_val = XDR_DEFAULT_SUB_VAL_LDO_DAC;
                __priv_trim.t_trim_info[trim_list].range.target = XDR_TRIM_TGT_LDO_DAC;
                __priv_trim.t_trim_info[trim_list].range.min = XDR_TRIM_TGT_LDO_DAC - (XDR_TRIM_TGT_LDO_DAC * XDR_TRIM_ERROR_RANGE / 100.0f);
                __priv_trim.t_trim_info[trim_list].range.max = XDR_TRIM_TGT_LDO_DAC + (XDR_TRIM_TGT_LDO_DAC * XDR_TRIM_ERROR_RANGE / 100.0f);
                break;
            case XDR_TRIM_LIST_LDO_FLL:
                __priv_trim.t_trim_info[trim_list].sub_val = XDR_DEFAULT_SUB_VAL_LDO_FLL;
                __priv_trim.t_trim_info[trim_list].range.target = XDR_TRIM_TGT_LDO_FLL;
                __priv_trim.t_trim_info[trim_list].range.min = XDR_TRIM_TGT_LDO_FLL - (XDR_TRIM_TGT_LDO_FLL * XDR_TRIM_ERROR_RANGE / 100.0f);
                __priv_trim.t_trim_info[trim_list].range.max = XDR_TRIM_TGT_LDO_FLL + (XDR_TRIM_TGT_LDO_FLL * XDR_TRIM_ERROR_RANGE / 100.0f);
                break;
            case XDR_TRIM_LIST_OSC:
                __priv_trim.t_trim_info[trim_list].sub_val = XDR_DEFAULT_SUB_VAL_OSC;
                __priv_trim.t_trim_info[trim_list].range.target = XDR_TRIM_TGT_OSC;
                __priv_trim.t_trim_info[trim_list].range.min = XDR_TRIM_TGT_OSC - (XDR_TRIM_TGT_OSC * XDR_TRIM_ERROR_RANGE / 100.0f);
                __priv_trim.t_trim_info[trim_list].range.max = XDR_TRIM_TGT_OSC + (XDR_TRIM_TGT_OSC * XDR_TRIM_ERROR_RANGE / 100.0f);
                break;
            case XDR_TRIM_LIST_CH_GAIN:
                __priv_trim.t_trim_info[trim_list].sub_val = XDR_DEFAULT_SUB_VAL_CH_GAIN;
                __priv_trim.t_trim_info[trim_list].range.target = XDR_TRIM_TGT_CH_GAIN;
                __priv_trim.t_trim_info[trim_list].range.min = XDR_TRIM_TGT_CH_GAIN - (XDR_TRIM_TGT_CH_GAIN * XDR_TRIM_ERROR_RANGE / 100.0f);
                __priv_trim.t_trim_info[trim_list].range.max = XDR_TRIM_TGT_CH_GAIN + (XDR_TRIM_TGT_CH_GAIN * XDR_TRIM_ERROR_RANGE / 100.0f);

                __priv_trim.t_trim_info[trim_list].input[0] = XDR_TRIM_GAIN_INPUT_1;
                __priv_trim.t_trim_info[trim_list].input[1] = XDR_TRIM_GAIN_INPUT_2;
                __priv_trim.t_trim_info[trim_list].gain = GAIN_HIGH;
                break;
            case XDR_TRIM_LIST_CH_OFS:
                __priv_trim.t_trim_info[trim_list].sub_val = XDR_DEFAULT_SUB_VAL_CH_OFS;
                __priv_trim.t_trim_info[trim_list].range.target = XDR_TRIM_TGT_CH_OFS;
                __priv_trim.t_trim_info[trim_list].range.min = XDR_TRIM_TGT_CH_OFS - (XDR_TRIM_TGT_CH_OFS * XDR_TRIM_ERROR_RANGE / 100.0f);
                __priv_trim.t_trim_info[trim_list].range.max = XDR_TRIM_TGT_CH_OFS + (XDR_TRIM_TGT_CH_OFS * XDR_TRIM_ERROR_RANGE / 100.0f);

                __priv_trim.t_trim_info[trim_list].input[0] = XDR_TRIM_OFS_INPUT_1;
                __priv_trim.t_trim_info[trim_list].input[1] = XDR_TRIM_OFS_INPUT_2;
                __priv_trim.t_trim_info[trim_list].gain = GAIN_HIGH;
                break;
        }
    }
}

static judge_info_t xdr_trim_compare_range(trim_info_t* p_trim_info_t)
{
    judge_info_t judge_info = { false , false, false };
    float measured_value = p_trim_info_t->measure.value[p_trim_info_t->repeat];
    float target_value = p_trim_info_t->range.target;
    float min_value = p_trim_info_t->range.min;
    float max_value = p_trim_info_t->range.max;

    if (measured_value < target_value)
    {
        if (measured_value >= min_value)
        {
            judge_info.in_range = true;
        }
        judge_info.up = true;
        judge_info.down = false;
        comm_UART_Printf(LOG_LV_INFO, "\n\rMeasured value : %.3f is smaller than target. (Target : %.3f, Min : %.3f, Max : %.3f)", measured_value, target_value, min_value, max_value);
    }
    else if (measured_value > target_value)
    {
        if (measured_value <= max_value)
        {
            judge_info.in_range = true;
        }
        judge_info.up = false;
        judge_info.down = true;
        comm_UART_Printf(LOG_LV_INFO, "\n\rMeasured value : %.3f is larger than target. (Target : %.3f, Min : %.3f, Max : %.3f)", measured_value, target_value, min_value, max_value);
    }
    else
    {
        judge_info.in_range = true;
        judge_info.up = false;
        judge_info.down = false;
        comm_UART_Printf(LOG_LV_INFO, "\n\rMeasured value : %.3f is same as target value! (Target : %.3f)", measured_value, target_value);
    }

    return judge_info;
}

static bool xdr_trim_update_register_by_sub_val(xdr_trim_list_t in_trim_list, trim_info_t* in_p_now_trim_info)
{
    bool ret = false;
    uint16_t reg_val = 0U;
    switch(in_trim_list)
    {
        case XDR_TRIM_LIST_CURRENT_REF:
            reg_val = in_p_now_trim_info->sub_val;
            ret = xdr12_trim_set_current_ref(reg_val);
            break;
        case XDR_TRIM_LIST_LDO_DIG:
            if (in_p_now_trim_info->sub_val < 8U)
            {
                reg_val = 15U - in_p_now_trim_info->sub_val;
            }
            else
            {
                reg_val = in_p_now_trim_info->sub_val - 8U;
            }
            ret = xdr12_trim_set_ldo_dig(reg_val);
            break;
        case XDR_TRIM_LIST_LDO_DAC:
            reg_val = in_p_now_trim_info->sub_val;
            ret = xdr12_trim_set_ldo_dac(reg_val);
            break;
        case XDR_TRIM_LIST_LDO_FLL:
            reg_val = in_p_now_trim_info->sub_val;
            ret = xdr12_trim_set_ldo_fll(reg_val);
            break;
        case XDR_TRIM_LIST_OSC:
            reg_val = in_p_now_trim_info->sub_val;
            ret = xdr12_trim_set_osc(reg_val);
            break;
        case XDR_TRIM_LIST_CH_GAIN:
            reg_val = in_p_now_trim_info->sub_val;
            ret = xdr12_trim_set_ch_gain(reg_val, in_p_now_trim_info->chx);
            break;
        case XDR_TRIM_LIST_CH_OFS:
            if (in_p_now_trim_info->sub_val < 256U)
            {
                reg_val = 511U - in_p_now_trim_info->sub_val;
            }
            else
            {
                reg_val = in_p_now_trim_info->sub_val - 256U;
            }
            ret = xdr12_trim_set_ch_ofs(reg_val, in_p_now_trim_info->chx);
            break;
    }
    return ret;
}

static bool _xcr_trim_thread(struct thread_data* td)
{
    switch(td->step)
    {
        case XCR_TRIM_STEP_PWR_ON:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %u, timeout : %u", __func__, td->id, td->step, td->tout);
            td->step = XCR_TRIM_STEP_EXAMPLE1;
            td->tout = 100;
            break;
        }

        case XCR_TRIM_STEP_EXAMPLE1:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
            td->step = XCR_TRIM_STEP_EXAMPLE2;
            td->tout = 250;
            break;
        }

        case XCR_TRIM_STEP_EXAMPLE2:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
            td->step = XCR_TRIM_STEP_EXAMPLE3;
            td->tout = 150;
            break;
        }

        case XCR_TRIM_STEP_EXAMPLE3:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
            td->step = XCR_TRIM_STEP_NONE;
            td->tout = 500;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
            __priv_trim.trim_thr = INVALID_THREAD_ID;
            return false;
        }
    }

    return true;
}

static bool _xdr_trim_thread(struct thread_data* td)
{
    xdr_trim_list_t* p_now_trim_list = &__priv_trim.t_xdr_trim_list;
    trim_info_t* p_now_trim_info = &__priv_trim.t_trim_info[*p_now_trim_list];

    switch(td->step)
    {
        case XDR_TRIM_STEP_PWR_ON:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, gs_xdr_trim_step[td->step], td->tout);
            /* Power On Sequence */
            gpio_set_xd_vdd_5v(XD_PWR_ON_5V0);
            td->step = XDR_TRIM_STEP_INITIAL;
            td->tout = XDR_DELAY_PWR_ON;
            break;
        }

        case XDR_TRIM_STEP_INITIAL:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, gs_xdr_trim_step[td->step], td->tout);
            /* XDR Initialization */
            xdr12_trim_init();
            td->step = XDR_TRIM_STEP_INITIAL_BY_LIST;
            td->tout = XDR_DELAY_DEFAULT;
            break;
        }

        case XDR_TRIM_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            /* XDR Prepare for Each Trim List */
            switch (*p_now_trim_list)
            {
                case XDR_TRIM_LIST_CURRENT_REF:
                    xdr12_trim_init_current_ref();
                    break;
                case XDR_TRIM_LIST_LDO_DIG:
                    xdr12_trim_init_ldo_dig();
                    break;
                case XDR_TRIM_LIST_LDO_DAC:
                    xdr12_trim_init_ldo_dac();
                    break;
                case XDR_TRIM_LIST_LDO_FLL:
                    xdr12_trim_init_ldo_fll();
                    break;
                case XDR_TRIM_LIST_OSC:
                    xdr12_trim_init_osc();
                    break;
                case XDR_TRIM_LIST_CH_GAIN:
                    gpio_set_power_9v(PWR_ON);
                    xdr12_trim_init_ch_gain();
                    xdr12_trim_set_max_curr_vref(p_now_trim_info->input[p_now_trim_info->repeat]);
                    xdr12_trim_set_channel_enable(p_now_trim_info->chx);
                    gpio_set_demux_channel_selection(p_now_trim_info->chx);
                    gpio_set_current_gain(p_now_trim_info->gain);
                    break;
                case XDR_TRIM_LIST_CH_OFS:
                    gpio_set_power_9v(PWR_ON);
                    xdr12_trim_init_ch_ofs();
                    xdr12_trim_set_max_curr_vref(p_now_trim_info->input[p_now_trim_info->repeat]);
                    xdr12_trim_set_channel_enable(p_now_trim_info->chx);
                    gpio_set_demux_channel_selection(p_now_trim_info->chx);
                    gpio_set_current_gain(p_now_trim_info->gain);
                    break;
            }
            td->step = XDR_TRIM_STEP_START_MEASURE;
            td->tout = XDR_DELAY_DEFAULT;
            break;
        }

        case XDR_TRIM_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            /* XDR Input Conditions for Each Trim List & Start Measure */
            switch (*p_now_trim_list)
            {
                case XDR_TRIM_LIST_CURRENT_REF:
                case XDR_TRIM_LIST_LDO_DIG:
                case XDR_TRIM_LIST_LDO_DAC:
                case XDR_TRIM_LIST_LDO_FLL:
                    mcu_peripheral_adc_start();
                    break;
                case XDR_TRIM_LIST_OSC:
                    mcu_peripheral_tim_input_capture_start();
                    break;
                case XDR_TRIM_LIST_CH_GAIN:
                    ADS114S08_Set_Start(true);
                    break;
                case XDR_TRIM_LIST_CH_OFS:
                    ADS114S08_Set_Start(true);
                    break;
            }
            td->step = XDR_TRIM_STEP_GET_MEASURED_VALUE;
            td->tout = XDR_DELAY_MEASURE;
            break;
        }

        case XDR_TRIM_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            float f_converted_value = 0.0f;
            /* XDR Measure */
            switch (*p_now_trim_list)
            {
                case XDR_TRIM_LIST_CURRENT_REF:
                case XDR_TRIM_LIST_LDO_DIG:
                case XDR_TRIM_LIST_LDO_DAC:
                case XDR_TRIM_LIST_LDO_FLL:
                    f_converted_value = mcu_peripheral_adc_conversion_to_value(mcu_peripheral_adc_get());
                    td->step = XDR_TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                    break;
                case XDR_TRIM_LIST_OSC:
                    f_converted_value = mcu_peripheral_tim_conversion_freq();
                    td->step = XDR_TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                    break;
                case XDR_TRIM_LIST_CH_GAIN:
                    if (true == ADS114S08_Wait_Done())
                    {
                        f_converted_value = JigBD_IF_Convert_Adc_To_Current(ADS114S08_Get_ADC_Value(), p_now_trim_info->gain);
                        ++p_now_trim_info->repeat;
                        if (p_now_trim_info->repeat < 2U)
                        {
                            td->step = XDR_TRIM_STEP_INITIAL_BY_LIST;
                        }
                        else
                        {
                            td->step = XDR_TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                            p_now_trim_info->repeat = 0U;
                        }
                    }
                    break;
                case XDR_TRIM_LIST_CH_OFS:
                    if (true == ADS114S08_Wait_Done())
                    {
                        f_converted_value = JigBD_IF_Convert_Adc_To_Current(ADS114S08_Get_ADC_Value(), p_now_trim_info->gain);
                        ++p_now_trim_info->repeat;
                        if (p_now_trim_info->repeat < 2U)
                        {
                            td->step = XDR_TRIM_STEP_INITIAL_BY_LIST;
                        }
                        else
                        {
                            td->step = XDR_TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                            p_now_trim_info->repeat = 0U;
                        }
                    }
                    break;
            }
            p_now_trim_info->measure.value[p_now_trim_info->repeat] = f_converted_value;
            td->tout = 500;
            break;
        }

        case XDR_TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            /* XDR Compare */
            judge_info_t t_judge = xdr_trim_compare_range(p_now_trim_info);
            if (true == t_judge.in_range)
            {
                /* In Range - Save Value */
                p_now_trim_info->saved.saved_value[p_now_trim_info->saved.saved_cnt] = p_now_trim_info->measure.value[p_now_trim_info->repeat];
                p_now_trim_info->saved.sub_val[p_now_trim_info->saved.saved_cnt] = p_now_trim_info->sub_val;
                ++p_now_trim_info->saved.saved_cnt;
            }

            if (t_judge.up == true)
            {
                p_now_trim_info->sub_val += 1U;
            }
            else if (t_judge.down == true)
            {
                p_now_trim_info->sub_val -= 1U;
            }

            if (p_now_trim_info->saved.saved_cnt < SAVE_INFO_MAX_CNT)
            {
                if (true == xdr_trim_update_register_by_sub_val(*p_now_trim_list, p_now_trim_info))
                {
                    td->step = XDR_TRIM_STEP_INITIAL_BY_LIST;
                    td->tout = XDR_DELAY_DEFAULT;
                }
                else
                {
                    // go to error handling, can't update register, trim thread stop
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r\tlist : %s Failed to update trim register!", gs_xdr_trim_list[*p_now_trim_list]);
                    td->step = XDR_TRIM_STEP_PWR_OFF;
                    td->tout = XDR_DELAY_DEFAULT;
                }
            }
            else
            {
                // find best value from saved value and update register
                if (p_now_trim_info->saved.saved_cnt > 0U)
                {
                    float target_value = p_now_trim_info->range.target;
                    float closest_val = p_now_trim_info->saved.saved_value[0];
                    uint8_t closest_idx = 0U;

                    float best_diff = fabsf(closest_val - target_value);

                    for (uint8_t idx = 1U; idx < p_now_trim_info->saved.saved_cnt; ++idx)
                    {
                        float value = p_now_trim_info->saved.saved_value[idx];
                        float diff = fabsf(value - target_value);

                        if (diff < best_diff)
                        {
                            best_diff = diff;
                            closest_val = value;
                            closest_idx = idx;
                        }
                    }

                    p_now_trim_info->measure.value[0] = closest_val;
                    p_now_trim_info->sub_val = p_now_trim_info->saved.sub_val[closest_idx];
                    xdr_trim_update_register_by_sub_val(*p_now_trim_list, p_now_trim_info);

                    if (*p_now_trim_list == XDR_TRIM_LIST_CH_GAIN || *p_now_trim_list == XDR_TRIM_LIST_CH_OFS)
                    {
                        if (p_now_trim_info->chx < (XD_CH_MAX - 1U))
                        {
                            ++p_now_trim_info->chx;
                            td->step = XDR_TRIM_STEP_INITIAL_BY_LIST;
                            td->tout = XDR_DELAY_DEFAULT;
                        }
                        else
                        {
                            if (++(*p_now_trim_list) < XDR_TRIM_LIST_MAX)
                            {
                                td->step = XDR_TRIM_STEP_INITIAL_BY_LIST;
                            }
                            else
                            {
                                td->step = XDR_TRIM_STEP_PREPARE_EFUSE;
                            }
                            td->tout = XDR_DELAY_DEFAULT;
                        }
                    }
                    else
                    {
                        if (++(*p_now_trim_list) < XDR_TRIM_LIST_MAX)
                        {
                            td->step = XDR_TRIM_STEP_INITIAL_BY_LIST;
                        }
                        else
                        {
                            td->step = XDR_TRIM_STEP_PREPARE_EFUSE;
                        }
                        td->tout = XDR_DELAY_DEFAULT;
                    }
                }
            }
            break;
        }

        case XDR_TRIM_STEP_PREPARE_EFUSE:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            if (gb_xdr_do_efuse)
            {
                // save mirror register
                xdr12_trim_save_mirror_register();
                // register setting
                xdr12_trim_init_efuse();
                //power control
                gpio_set_xd_vdd_5v(XD_PWR_ON_5V5);
                td->step = XDR_TRIM_STEP_START_EFUSE;
                td->tout = XDR_DELAY_DEFAULT;
            }
            else
            {
                // turn off
                td->step = XDR_TRIM_STEP_PWR_OFF;
                td->tout = XDR_DELAY_DEFAULT;
            }
            break;
        }

        case XDR_TRIM_STEP_START_EFUSE:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            xdr12_trim_start_efuse();
            td->step = XDR_TRIM_STEP_END_EFUSE;
            td->tout = XDR_DELAY_EFUSE_DONE;
            break;
        }

        case XDR_TRIM_STEP_END_EFUSE:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            gpio_set_xd_vdd_5v(XD_PWR_OFF);
            td->step = XDR_TRIM_STEP_PWR_REBOOT;
            td->tout = XDR_DELAY_PWR_OFF;
            break;
        }

        case XDR_TRIM_STEP_PWR_REBOOT:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            gpio_set_xd_vdd_5v(XD_PWR_ON_5V0);
            td->step = XDR_TRIM_STEP_VERIFY_OTP_DUMP;
            td->tout = XDR_DELAY_PWR_ON;
            break;
        }

        case XDR_TRIM_STEP_VERIFY_OTP_DUMP:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            xdr12_trim_init();
            if (0U == xdr12_trim_verify_mirror_dump())
            {
                // verify OK
            }
            else
            {
                // verify NG
            }

            td->step = XDR_TRIM_STEP_LOG_SUMMARY;
            td->tout = XDR_DELAY_DEFAULT;
            break;
        }

        case XDR_TRIM_STEP_LOG_SUMMARY:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            /* XDR eFuse Check */
            td->step = XDR_TRIM_STEP_PWR_OFF;
            td->tout = XDR_DELAY_DEFAULT;
            break;
        }

        case XDR_TRIM_STEP_PWR_OFF:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r\tstep : %s, list : %s, timeout : %u", gs_xdr_trim_step[td->step], gs_xdr_trim_list[*p_now_trim_list], td->tout);
            /* XDR Power Off */
            gpio_set_xd_vdd_5v(XD_PWR_OFF);
            gpio_set_power_9v(PWR_OFF);
            td->step = XDR_TRIM_STEP_NONE;
            td->tout = XDR_DELAY_DEFAULT;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, gs_xdr_trim_step[td->step], td->tout);
            __priv_trim.trim_thr = INVALID_THREAD_ID;
            return false;
        }
    }

    return true;
}

static void _power(bool on)
{
    if(true == on)
    {
        xdr_trim_param_init();

        __priv_trim.status = STATUS_NORMAL;
    }
    else
    {
        if(__priv_trim.trim_thr != INVALID_THREAD_ID)
        {
            fw_thread_stop(__priv_trim.trim_thr);
            __priv_trim.trim_thr = INVALID_THREAD_ID;
        }

        __priv_trim.status = STATUS_UNPOWER;
    }
}

static void _enable(bool en)
{

}

static MGRSTATUS _status(void)
{
    return __priv_trim.status;
}

static uint32_t _cmd(uint32_t cmd, void* val)
{
    switch(cmd)
    {
    case TRIM_CMD_XCR_START:
        if(__priv_trim.trim_thr == INVALID_THREAD_ID)
        {
            __priv_trim.trim_thr = fw_begin_thread_ex(_xcr_trim_thread, 10U);    /* 10ms */
        }
        break;
    case TRIM_CMD_XDR_START:
        if(__priv_trim.trim_thr == INVALID_THREAD_ID)
        {
            __priv_trim.trim_thr = fw_begin_thread_ex(_xdr_trim_thread, 10U);    /* 10ms */
        }
        break;
    default:
        break;
    }

    return MGRET_OK;
}

static uint32_t _write(uint32_t addr, void* val, uint32_t len)
{
    return MGRET_OK;
}

static uint32_t _read(uint32_t addr, void* val, uint32_t len)
{
    return 0;
}

static uint32_t _noti(uint32_t type, void* val)
{
    return MGRET_OK;
}

struct manager __trim_mgr=
{
    _power,
    _enable,
    _status,
    _cmd,
    _write,
    _read,
    _noti
};