#include <math.h>

#include "framework.h"
#include "drv_xd12.h"
#include "drv_xc24.h"
#include "comm_debugging.h"
#include "drv_gpio.h"
#include "drv_timer.h"
#include "drv_ads124s08.h"

#define SAVE_INFO_MAX_CNT                   (10)
#define MAX_TRIM_TRY_CNT                    (30)

#define XC_DELAY_DEFAULT                   (1U)
#define XC_DELAY_SETTLING                  (5U)
#define XC_DELAY_VSYNC_STOP                (100U)
#define XC_DELAY_MEASURE                   (10U)
#define XC_DELAY_PWR_ON                    (100U)
#define XC_DELAY_PWR_OFF                   (100U)
#define XC_DELAY_EFUSE_DONE                (500U)

#define XC_DEFAULT_SUB_VAL_1V5_LDO_DIG     (1U << 4U)
#define XC_DEFAULT_SUB_VAL_DAC_3V0         (1U << 5U)
#define XC_DEFAULT_SUB_VAL_DAC1_OFS        (1U << 7U)
#define XC_DEFAULT_SUB_VAL_DAC2_OFS        (1U << 7U)
#define XC_DEFAULT_SUB_VAL_DAC3_OFS        (1U << 7U)
#define XC_DEFAULT_SUB_VAL_1V5_LDO_OSC     (1U << 4U)
#define XC_DEFAULT_SUB_VAL_OSC_A           (1U << 4U)
#define XC_DEFAULT_SUB_VAL_OSC_B           (1U << 4U)

#define XC_TRIM_ERROR_RANGE                (0.005f)    /* 0.5% */
#define XC_TRIM_OSC_ERROR_RANGE            (0.0285f)   /* 2.85% */

#define XC_TRIM_TGT_1V5_LDO_DIG            (1.5f)      /* 1.5V */
#define XC_TRIM_TGT_DAC_3V0                (3.0f)      /* 3.0V */
#define XC_TRIM_TGT_DAC1_OFS               (0.1465f)   /* 146.5mV */
#define XC_TRIM_TGT_DAC2_OFS               (0.1465f)   /* 146.5mV */
#define XC_TRIM_TGT_DAC3_OFS               (0.1465f)   /* 146.5mV */
#define XC_TRIM_TGT_1V5_LDO_OSC            (1.5f)      /* 1.5V */
#define XC_TRIM_TGT_OSC_A                  (35.0f)     /* 35MHz */
#define XC_TRIM_TGT_OSC_B                  (35.0f)     /* 35MHz */

#define XC_TRIM_INIT_ADJ_1V5_LDO_DIG       (2U)
#define XC_TRIM_INIT_ADJ_DAC_3V0           (2U)
#define XC_TRIM_INIT_ADJ_DAC1_OFS          (10U)
#define XC_TRIM_INIT_ADJ_DAC2_OFS          (10U)
#define XC_TRIM_INIT_ADJ_DAC3_OFS          (10U)
#define XC_TRIM_INIT_ADJ_1V5_LDO_OSC       (1U)
#define XC_TRIM_INIT_ADJ_OSC_A             (1U)
#define XC_TRIM_INIT_ADJ_OSC_B             (1U)

/*******************************************************************************/

#define XD_DELAY_DEFAULT                   (1U)
#define XD_DELAY_SETTLING                  (5U)
#define XD_DELAY_MEASURE                   (10U)
#define XD_DELAY_PWR_ON                    (100U)
#define XD_DELAY_PWR_OFF                   (100U)
#define XD_DELAY_EFUSE_DONE                (500U)

#define XD_DEFAULT_SUB_VAL_CURRENT_REF     ((uint16_t)1U << 4U)
#define XD_DEFAULT_SUB_VAL_LDO_DIG         ((uint16_t)1U << 3U)
#define XD_DEFAULT_SUB_VAL_LDO_DAC         ((uint16_t)1U << 4U)
#define XD_DEFAULT_SUB_VAL_LDO_FLL         ((uint16_t)1U << 3U)
#define XD_DEFAULT_SUB_VAL_OSC             ((uint16_t)1U << 4U)
#define XD_DEFAULT_SUB_VAL_CH_GAIN         ((uint16_t)1U << 6U)
#define XD_DEFAULT_SUB_VAL_CH_OFS          ((uint16_t)1U << 8U)

#define XD_TRIM_GAIN_INPUT_1               (300U)
#define XD_TRIM_GAIN_INPUT_2               (1300U)

#define XD_TRIM_OFS_INPUT_1                (200U)
#define XD_TRIM_OFS_INPUT_2                (300U)

#define XD_TRIM_ERROR_RANGE                (0.02f)     /* 2% */
#define XD_TRIM_OSC_ERROR_RANGE            (0.05f)     /* 5% */
#define XD_TRIM_IOUT_ERROR_RANGE           (0.005f)    /* 0.5% */

#define XD_TRIM_TGT_CURRENT_REF            (1.4f)      /* 1.4 V */
#define XD_TRIM_TGT_LDO_DIG                (1.5f)      /* 1.5 V */
#define XD_TRIM_TGT_LDO_DAC                (1.5f)      /* 1.5 V */
#define XD_TRIM_TGT_LDO_FLL                (1.5f)      /* 1.5 V */
#define XD_TRIM_TGT_OSC                    (51.0f)     /* 51.0 MHz */
#define XD_TRIM_TGT_CH_GAIN                (24.0f * (XD_TRIM_GAIN_INPUT_2 - XD_TRIM_GAIN_INPUT_1) / 4095.0f)
#define XD_TRIM_TGT_CH_OFS                 (24.0f * ((XD_TRIM_OFS_INPUT_2 + XD_TRIM_OFS_INPUT_1) / 2.0f) / 4095.0f)

#define XD_TRIM_INIT_ADJ_CURRENT_REF       (3U)
#define XD_TRIM_INIT_ADJ_LDO_DIG           (3U)
#define XD_TRIM_INIT_ADJ_LDO_DAC           (3U)
#define XD_TRIM_INIT_ADJ_LDO_FLL           (3U)
#define XD_TRIM_INIT_ADJ_OSC               (10U)
#define XD_TRIM_INIT_ADJ_CH_GAIN           (10U)
#define XD_TRIM_INIT_ADJ_CH_OFS            (10U)

typedef struct tag_SAVED_INFO
{
    float saved_value[SAVE_INFO_MAX_CNT];
    uint16_t sub_val[SAVE_INFO_MAX_CNT];
    uint8_t saved_cnt;
    uint8_t try_cnt;
} saved_info_t; // 64 bytes

typedef struct tag_JUDGE_INFO
{
    bool in_range;
    bool up;
    bool down;
} judge_info_t; // 3 bytes

typedef struct tag_RANGE_INFO
{
    float target;
    float min;
    float max;
} range_info_t; // 12 bytes

typedef struct tag_MEASURE_INFO
{
    float temp_value[2];
    float value;
    uint16_t adc[2];
} measure_info_t; // 16 bytes

typedef struct tag_TRIM_INFO
{
    saved_info_t saved[XD_CH_MAX];
    measure_info_t measure[XD_CH_MAX];
    uint16_t sub_val[XD_CH_MAX];
    uint16_t reg_val[XD_CH_MAX];
    range_info_t range;
    current_gain_t gain;
    uint16_t input[2];
    uint16_t initial_adj_size;
    uint8_t repeat;
    uint8_t chx;
    bool trim_error;
} trim_info_t;

typedef enum tag_TRIM_STEP_T
{
    TRIM_STEP_PWR_ON = 0U,
    TRIM_STEP_INITIAL,
    TRIM_STEP_INITIAL_BY_LIST,
    TRIM_STEP_VSYNC_STOP,
    TRIM_STEP_START_MEASURE,
    TRIM_STEP_GET_MEASURED_VALUE,
    TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER,
    TRIM_STEP_PREPARE_EFUSE,
    TRIM_STEP_START_EFUSE,
    TRIM_STEP_END_EFUSE,
    TRIM_STEP_PWR_REBOOT,
    TRIM_STEP_VERIFY_OTP_DUMP,
    TRIM_STEP_LOG_SUMMARY,
    TRIM_STEP_PWR_OFF,
    TRIM_STEP_NONE,
    TRIM_STEP_MAX,
} trim_step_t;

typedef enum tag_XC_TRIM_LIST
{
    XC_TRIM_LIST_1V5_LDO_DIG = 0U,
    XC_TRIM_LIST_DAC_3V0,
    XC_TRIM_LIST_DAC1_OFS,
    XC_TRIM_LIST_DAC2_OFS,
    XC_TRIM_LIST_DAC3_OFS,
    XC_TRIM_LIST_1V5_LDO_OSC,
    XC_TRIM_LIST_OSC_A,
    XC_TRIM_LIST_OSC_B,
    XC_TRIM_LIST_MAX,
} xc_trim_list_t;

typedef enum tag_XD_TRIM_LIST
{
    XD_TRIM_LIST_CURRENT_REF = 0U,
    XD_TRIM_LIST_LDO_DIG,
    XD_TRIM_LIST_LDO_DAC,
    XD_TRIM_LIST_LDO_FLL,
    XD_TRIM_LIST_OSC,
    XD_TRIM_LIST_CH_GAIN,
    XD_TRIM_LIST_CH_OFS,
    XD_TRIM_LIST_MAX,
} xd_trim_list_t;

static struct{
    trim_info_t     t_xd_trim_info[XD_TRIM_LIST_MAX];
    trim_info_t     t_xc_trim_info[XC_TRIM_LIST_MAX];
    MGRSTATUS       status;
    xd_trim_list_t t_xd_trim_list;
    xc_trim_list_t t_xc_trim_list;
    THREAD_ID       trim_thr;
}__priv_trim; // declare & define private variable for trim manager

static const char* gs_trim_step[TRIM_STEP_MAX] =
{
    "TRIM_STEP_PWR_ON",
    "TRIM_STEP_INITIAL",
    "TRIM_STEP_INITIAL_BY_LIST",
    "TRIM_STEP_VSYNC_STOP",
    "TRIM_STEP_START_MEASURE",
    "TRIM_STEP_GET_MEASURED_VALUE",
    "TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER",
    "TRIM_STEP_PREPARE_EFUSE",
    "TRIM_STEP_START_EFUSE",
    "TRIM_STEP_END_EFUSE",
    "TRIM_STEP_PWR_REBOOT",
    "TRIM_STEP_VERIFY_OTP_DUMP",
    "TRIM_STEP_LOG_SUMMARY",
    "TRIM_STEP_PWR_OFF",
    "TRIM_STEP_NONE",
};

static const char* gs_xc_trim_list[XC_TRIM_LIST_MAX] =
{
    "XC_1V5_LDO_DIG",
    "XC_DAC_3V0    ",
    "XC_DAC1_OFS   ",
    "XC_DAC2_OFS   ",
    "XC_DAC3_OFS   ",
    "XC_1V5_LDO_OSC",
    "XC_OSC_A      ",
    "XC_OSC_B      ",
};

static const char* gs_xd_trim_list[XD_TRIM_LIST_MAX] =
{
    "XD_CURRENT_REF",
    "XD_LDO_DIG    ",
    "XD_LDO_DAC    ",
    "XD_LDO_FLL    ",
    "XD_OSC        ",
    "XD_CH_GAIN    ",
    "XD_CH_OFS     ",
};

static const char* xc_trim_list_to_string(xc_trim_list_t trim_list)
{
    if (trim_list < XC_TRIM_LIST_MAX)
    {
        return gs_xc_trim_list[trim_list];
    }

    return "XC_TRIM_LIST_INVALID";
}

static const char* xd_trim_list_to_string(xd_trim_list_t trim_list)
{
    if (trim_list < XD_TRIM_LIST_MAX)
    {
        return gs_xd_trim_list[trim_list];
    }

    return "XD_TRIM_LIST_INVALID";
}

static const char* trim_step_to_string(trim_step_t step)
{
    if (step < TRIM_STEP_MAX)
    {
        return gs_trim_step[step];
    }

    return "TRIM_STEP_INVALID";
}

static judge_info_t trim_compare_range(trim_info_t* p_trim_info_t)
{
    judge_info_t judge_info = { false , false, false };
    float measured_value = p_trim_info_t->measure[p_trim_info_t->chx].value;
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
    }
    else if (measured_value > target_value)
    {
        if (measured_value <= max_value)
        {
            judge_info.in_range = true;
        }
        judge_info.up = false;
        judge_info.down = true;
    }
    else
    {
        judge_info.in_range = true;
        judge_info.up = false;
        judge_info.down = false;
    }
    ++p_trim_info_t->saved[p_trim_info_t->chx].try_cnt;

    return judge_info;
}

static uint16_t trim_calculate_adjust_amount(trim_info_t* p_trim_info_t)
{
    uint16_t adj_amount = 0U;
    static float previous_value = 0.0f;
    static uint16_t previous_sub_val = 0U;

    if (p_trim_info_t->saved[p_trim_info_t->chx].try_cnt == 1U)
    {
        adj_amount = p_trim_info_t->initial_adj_size;
    }
    else
    {
        float now_value = p_trim_info_t->measure[p_trim_info_t->chx].value;
        uint16_t now_sub_val = p_trim_info_t->sub_val[p_trim_info_t->chx];
        float gap_from_tgt = fabsf(now_value - p_trim_info_t->range.target);

        float change_per_sub_val = fabsf((now_value - previous_value) / (now_sub_val - previous_sub_val));
        adj_amount = (uint16_t)(gap_from_tgt / change_per_sub_val + 0.5f);
    }

    previous_sub_val = p_trim_info_t->sub_val[p_trim_info_t->chx];
    previous_value = p_trim_info_t->measure[p_trim_info_t->chx].value;

    if (adj_amount == 0U)
    {
        adj_amount = 1U;
    }

    comm_UART_Printf(LOG_LV_DEBUG, "\n\r\t\tadj : %u", adj_amount);

    return adj_amount;
}

static void xc_trim_param_init(void)
{
    for (xc_trim_list_t trim_list = XC_TRIM_LIST_1V5_LDO_DIG; trim_list < XC_TRIM_LIST_MAX; ++trim_list)
    {
        switch (trim_list)
        {
            case XC_TRIM_LIST_1V5_LDO_DIG:
            {
                __priv_trim.t_xc_trim_info[trim_list].sub_val[0] = XC_DEFAULT_SUB_VAL_1V5_LDO_DIG;
                __priv_trim.t_xc_trim_info[trim_list].range.target = XC_TRIM_TGT_1V5_LDO_DIG;
                __priv_trim.t_xc_trim_info[trim_list].range.min = XC_TRIM_TGT_1V5_LDO_DIG * (1.0f - XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].range.max = XC_TRIM_TGT_1V5_LDO_DIG * (1.0f + XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].initial_adj_size = XC_TRIM_INIT_ADJ_1V5_LDO_DIG;
                break;
            }
            case XC_TRIM_LIST_DAC_3V0:
            {
                __priv_trim.t_xc_trim_info[trim_list].sub_val[0] = XC_DEFAULT_SUB_VAL_DAC_3V0;
                __priv_trim.t_xc_trim_info[trim_list].range.target = XC_TRIM_TGT_DAC_3V0;
                __priv_trim.t_xc_trim_info[trim_list].range.min = XC_TRIM_TGT_DAC_3V0 * (1.0f - XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].range.max = XC_TRIM_TGT_DAC_3V0 * (1.0f + XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].initial_adj_size = XC_TRIM_INIT_ADJ_DAC_3V0;
                break;
            }
            case XC_TRIM_LIST_DAC1_OFS:
            {
                __priv_trim.t_xc_trim_info[trim_list].sub_val[0] = XC_DEFAULT_SUB_VAL_DAC1_OFS;
                __priv_trim.t_xc_trim_info[trim_list].range.target = XC_TRIM_TGT_DAC1_OFS;
                __priv_trim.t_xc_trim_info[trim_list].range.min = XC_TRIM_TGT_DAC1_OFS * (1.0f - XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].range.max = XC_TRIM_TGT_DAC1_OFS * (1.0f + XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].initial_adj_size = XC_TRIM_INIT_ADJ_DAC1_OFS;
                break;
            }
            case XC_TRIM_LIST_DAC2_OFS:
            {
                __priv_trim.t_xc_trim_info[trim_list].sub_val[0] = XC_DEFAULT_SUB_VAL_DAC2_OFS;
                __priv_trim.t_xc_trim_info[trim_list].range.target = XC_TRIM_TGT_DAC2_OFS;
                __priv_trim.t_xc_trim_info[trim_list].range.min = XC_TRIM_TGT_DAC2_OFS * (1.0f - XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].range.max = XC_TRIM_TGT_DAC2_OFS * (1.0f + XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].initial_adj_size = XC_TRIM_INIT_ADJ_DAC2_OFS;
                break;
            }
            case XC_TRIM_LIST_DAC3_OFS:
            {
                __priv_trim.t_xc_trim_info[trim_list].sub_val[0] = XC_DEFAULT_SUB_VAL_DAC3_OFS;
                __priv_trim.t_xc_trim_info[trim_list].range.target = XC_TRIM_TGT_DAC3_OFS;
                __priv_trim.t_xc_trim_info[trim_list].range.min = XC_TRIM_TGT_DAC3_OFS * (1.0f - XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].range.max = XC_TRIM_TGT_DAC3_OFS * (1.0f + XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].initial_adj_size = XC_TRIM_INIT_ADJ_DAC3_OFS;
                break;
            }
            case XC_TRIM_LIST_1V5_LDO_OSC:
            {
                __priv_trim.t_xc_trim_info[trim_list].sub_val[0] = XC_DEFAULT_SUB_VAL_1V5_LDO_OSC;
                __priv_trim.t_xc_trim_info[trim_list].range.target = XC_TRIM_TGT_1V5_LDO_OSC;
                __priv_trim.t_xc_trim_info[trim_list].range.min = XC_TRIM_TGT_1V5_LDO_OSC * (1.0f - XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].range.max = XC_TRIM_TGT_1V5_LDO_OSC * (1.0f + XC_TRIM_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].initial_adj_size = XC_TRIM_INIT_ADJ_1V5_LDO_OSC;
                break;
            }
            case XC_TRIM_LIST_OSC_A:
            {
                __priv_trim.t_xc_trim_info[trim_list].sub_val[0] = XC_DEFAULT_SUB_VAL_OSC_A;
                __priv_trim.t_xc_trim_info[trim_list].range.target = XC_TRIM_TGT_OSC_A;
                __priv_trim.t_xc_trim_info[trim_list].range.min = XC_TRIM_TGT_OSC_A * (1.0f - XC_TRIM_OSC_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].range.max = XC_TRIM_TGT_OSC_A * (1.0f + XC_TRIM_OSC_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].initial_adj_size = XC_TRIM_INIT_ADJ_OSC_A;
                break;
            }
            case XC_TRIM_LIST_OSC_B:
            {
                __priv_trim.t_xc_trim_info[trim_list].sub_val[0] = XC_DEFAULT_SUB_VAL_OSC_B;
                __priv_trim.t_xc_trim_info[trim_list].range.target = XC_TRIM_TGT_OSC_B;
                __priv_trim.t_xc_trim_info[trim_list].range.min = XC_TRIM_TGT_OSC_B * (1.0f - XC_TRIM_OSC_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].range.max = XC_TRIM_TGT_OSC_B * (1.0f + XC_TRIM_OSC_ERROR_RANGE);
                __priv_trim.t_xc_trim_info[trim_list].initial_adj_size = XC_TRIM_INIT_ADJ_OSC_B;
                break;
            }
            default:
            {
                FATAL_INVALID_INPUT(trim_list);
                break;
            }
        }
    }
}

static void xc_trim_log_summary(void)
{
    for (xc_trim_list_t list = XC_TRIM_LIST_1V5_LDO_DIG; list < XC_TRIM_LIST_MAX; ++list)
    {
        trim_info_t* info = &__priv_trim.t_xc_trim_info[list];
        comm_UART_Printf(LOG_LV_INFO, "\r\n|%s|REG|%3u|SUB|%3u|VAL|%6.3f|", \
            gs_xc_trim_list[list], info->reg_val[0], info->sub_val[0], (double)(info->measure[0].value));
    }
}

static bool xc_trim_update_register_by_sub_val(xc_trim_list_t in_trim_list, trim_info_t* in_trim_info)
{
    bool ret = false;
    uint16_t reg_val = 0U;
    uint16_t sub_val = in_trim_info->sub_val[0];

    switch(in_trim_list)
    {
        case XC_TRIM_LIST_1V5_LDO_DIG:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[0] = reg_val;
            ret = xc24_trim_set_1v5_ldo_dig(reg_val);
            break;
        }
        case XC_TRIM_LIST_DAC_3V0:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[0] = reg_val;
            ret = xc24_trim_set_dac_3v0(reg_val);
            break;
        }
        case XC_TRIM_LIST_DAC1_OFS:
        {
            reg_val = (sub_val < 0x80U) ? (0xFFU - sub_val) : (sub_val - 0x80U);
            in_trim_info->reg_val[0] = reg_val;
            ret = xc24_trim_set_dac1_ofs(reg_val);
            break;
        }
        case XC_TRIM_LIST_DAC2_OFS:
        {
            reg_val = (sub_val < 0x80U) ? (0xFFU - sub_val) : (sub_val - 0x80U);
            in_trim_info->reg_val[0] = reg_val;
            ret = xc24_trim_set_dac2_ofs(reg_val);
            break;
        }
        case XC_TRIM_LIST_DAC3_OFS:
        {
            reg_val = (sub_val < 0x80U) ? (0xFFU - sub_val) : (sub_val - 0x80U);
            in_trim_info->reg_val[0] = reg_val;
            ret = xc24_trim_set_dac3_ofs(reg_val);
            break;
        }
        case XC_TRIM_LIST_1V5_LDO_OSC:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[0] = reg_val;
            ret = xc24_trim_set_1v5_ldo_osc(reg_val);
            break;
        }
        case XC_TRIM_LIST_OSC_A:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[0] = reg_val;
            ret = xc24_trim_set_osc_a(reg_val);
            break;
        }
        case XC_TRIM_LIST_OSC_B:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[0] = reg_val;
            ret = xc24_trim_set_osc_b(reg_val);
            break;
        }
        default:
        {
            FATAL_INVALID_INPUT(in_trim_list);
            break;
        }
    }
    if (ret == true)
    {
        comm_UART_Printf(LOG_LV_INFO, "\n\r\t\t[%sREG_UPDATE%s] (REG: %3u) (SUB: %3u)", ANSI_FONT_YELLOW, ANSI_FONT_NONE, reg_val, sub_val);
    }
    else
    {
        comm_UART_Printf(LOG_LV_FATAL, "\n\r\t\t[%sREG_UPDATE_FAIL%s] (REG: %3u) (SUB: %3u)", ANSI_FONT_RED, ANSI_FONT_NONE, reg_val, sub_val);
    }
    return ret;
}

static bool _xc_trim_thread(struct thread_data* td)
{
    if (td == NULL)
    {
        return false;
    }
    xc_trim_list_t* list = &__priv_trim.t_xc_trim_list;
    trim_info_t* info = &__priv_trim.t_xc_trim_info[*list];

    switch(td->step)
    {
        case TRIM_STEP_PWR_ON:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, trim_step_to_string((trim_step_t)td->step), td->tout);
            gpio_set_xc_vdd_5v(VCC_ON_3V3);
            td->step = TRIM_STEP_INITIAL;
            td->tout = XC_DELAY_PWR_ON;
            break;
        }

        case TRIM_STEP_INITIAL:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, trim_step_to_string((trim_step_t)td->step), td->tout);
            xc24_trim_init();
            td->step = TRIM_STEP_INITIAL_BY_LIST;
            td->tout = XC_DELAY_DEFAULT;
            break;
        }

        case TRIM_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            switch (*list)
            {
                case XC_TRIM_LIST_1V5_LDO_DIG:
                {
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_LDO, ADS_AINCOM);
                    xc24_trim_init_1v5_ldo_dig();
                    td->step = TRIM_STEP_START_MEASURE;
                    td->tout = XC_DELAY_SETTLING;
                    break;
                }
                case XC_TRIM_LIST_DAC_3V0:
                {
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC_1, ADS_AINCOM);
                    xc24_trim_init_dac_3v0();
                    tim_vsync_out_for_test_start();
                    td->step = TRIM_STEP_VSYNC_STOP;
                    td->tout = XC_DELAY_VSYNC_STOP;
                    break;
                }
                case XC_TRIM_LIST_DAC1_OFS:
                {
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC_1, ADS_AINCOM);
                    xc24_trim_init_dac1_ofs();
                    tim_vsync_out_for_test_start();
                    td->step = TRIM_STEP_VSYNC_STOP;
                    td->tout = XC_DELAY_VSYNC_STOP;
                    break;
                }
                case XC_TRIM_LIST_DAC2_OFS:
                {
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC_2, ADS_AINCOM);
                    xc24_trim_init_dac2_ofs();
                    tim_vsync_out_for_test_start();
                    td->step = TRIM_STEP_VSYNC_STOP;
                    td->tout = XC_DELAY_VSYNC_STOP;
                    break;
                }
                case XC_TRIM_LIST_DAC3_OFS:
                {
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_DAC_3, ADS_AINCOM);
                    xc24_trim_init_dac3_ofs();
                    tim_vsync_out_for_test_start();
                    td->step = TRIM_STEP_VSYNC_STOP;
                    td->tout = XC_DELAY_VSYNC_STOP;
                    break;
                }
                case XC_TRIM_LIST_1V5_LDO_OSC:
                {
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XC_1V5, ADS_AINCOM);
                    xc24_trim_init_1v5_ldo_osc();
                    td->step = TRIM_STEP_START_MEASURE;
                    td->tout = XC_DELAY_SETTLING;
                    break;
                }
                case XC_TRIM_LIST_OSC_A:
                {
                    xc24_trim_init_osc_a();
                    td->step = TRIM_STEP_START_MEASURE;
                    td->tout = XC_DELAY_SETTLING;
                    break;
                }
                case XC_TRIM_LIST_OSC_B:
                {
                    xc24_trim_init_osc_b();
#if 0 // skip osc_b trim
                    td->step = TRIM_STEP_START_MEASURE;
                    td->tout = XC_DELAY_SETTLING;
#else
                    td->step = TRIM_STEP_PREPARE_EFUSE;
                    td->tout = XC_DELAY_DEFAULT;
#endif
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, trim_step_to_string((trim_step_t)td->step), *list);
                    td->step = TRIM_STEP_PWR_OFF;
                    info->trim_error = true;
                    return true;
                }
            }
            break;
        }
        case TRIM_STEP_VSYNC_STOP:
        {
            tim_vsync_out_for_test_stop();
            td->step = TRIM_STEP_START_MEASURE;
            td->tout = XC_DELAY_SETTLING;
            break;
        }

        case TRIM_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            switch (*list)
            {
                case XC_TRIM_LIST_1V5_LDO_DIG:
                case XC_TRIM_LIST_DAC_3V0:
                case XC_TRIM_LIST_DAC1_OFS:
                case XC_TRIM_LIST_DAC2_OFS:
                case XC_TRIM_LIST_DAC3_OFS:
                case XC_TRIM_LIST_1V5_LDO_OSC:
                {
                    ADS114S08_Set_Start(true);
                    break;
                }
                case XC_TRIM_LIST_OSC_A:
                case XC_TRIM_LIST_OSC_B:
                {
                    mcu_peripheral_tim_input_capture_start();
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, trim_step_to_string((trim_step_t)td->step), *list);
                    td->step = TRIM_STEP_PWR_OFF;
                    info->trim_error = true;
                    return true;
                }
            }
            td->step = TRIM_STEP_GET_MEASURED_VALUE;
            td->tout = XC_DELAY_MEASURE;
            break;
        }

        case TRIM_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            switch (*list)
            {
                case XC_TRIM_LIST_1V5_LDO_DIG:
                case XC_TRIM_LIST_DAC_3V0:
                case XC_TRIM_LIST_DAC1_OFS:
                case XC_TRIM_LIST_DAC2_OFS:
                case XC_TRIM_LIST_DAC3_OFS:
                case XC_TRIM_LIST_1V5_LDO_OSC:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        info->measure[info->chx].adc[info->repeat] = ADS114S08_Get_ADC_Value();
                        info->measure[info->chx].value = JigBD_IF_Convert_Adc_To_Voltage(info->measure[info->chx].adc[info->repeat]);
                        td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                        td->tout = XC_DELAY_DEFAULT;
                    }
                    break;
                }
                case XC_TRIM_LIST_OSC_A:
                case XC_TRIM_LIST_OSC_B:
                {
                    info->measure[info->chx].value = mcu_peripheral_tim_conversion_freq() * XC_CONST_OSC;
                    td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                    td->tout = XC_DELAY_DEFAULT;
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, trim_step_to_string((trim_step_t)td->step), *list);
                    td->step = TRIM_STEP_PWR_OFF;
                    return true;
                }
            }
            break;
        }

        case TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            judge_info_t t_judge = trim_compare_range(info);
            uint8_t ch = info->chx;

            if (info->saved[ch].try_cnt > MAX_TRIM_TRY_CNT)
            {
                comm_UART_Printf(LOG_LV_ERROR, "\n\r\t[FAIL] list: %s (ch: %u) Max try count exceeded! (%u/%u)",\
                    xc_trim_list_to_string(*list), (ch + 1U), info->saved[ch].try_cnt, MAX_TRIM_TRY_CNT);

                info->trim_error = true;
                td->step = TRIM_STEP_PWR_OFF; // 또는 에러 처리 단계
                td->tout = XC_DELAY_DEFAULT;
                break;
            }

            if (true == t_judge.in_range)
            {
                /* In Range - Save Value */
                info->saved[ch].saved_value[info->saved[ch].saved_cnt] = info->measure[ch].value;
                info->saved[ch].sub_val[info->saved[ch].saved_cnt] = info->sub_val[ch];
                ++info->saved[ch].saved_cnt;
                comm_UART_Printf(LOG_LV_INFO, "\n\r[%s]\r\n\t\t(try: %3u)(ch: %2u) (Value: %6.3f) -> [%s✔ IN_RANGE (%2u)%s] (TGT: %6.3f, MIN: %6.3f, MAX: %6.3f)", \
                    xc_trim_list_to_string(*list), info->saved[ch].try_cnt, (ch + 1U), (double)(info->measure[ch].value), \
                    ANSI_FONT_GREEN, info->saved[ch].saved_cnt, ANSI_FONT_NONE, \
                    (double)(info->range.target), (double)(info->range.min), (double)(info->range.max));
            }
            else
            {
                comm_UART_Printf(LOG_LV_INFO, "\n\r[%s]\r\n\t\t(try: %3u)(ch: %2u) (Value: %6.3f) -> [%s✕   OUT_RANGE   %s] (TGT: %6.3f, MIN: %6.3f, MAX: %6.3f)", \
                    xc_trim_list_to_string(*list), info->saved[ch].try_cnt, (ch + 1U), (double)(info->measure[ch].value), \
                    ANSI_FONT_RED, ANSI_FONT_NONE, \
                    (double)(info->range.target), (double)(info->range.min), (double)(info->range.max));
            }

            uint16_t adj_mount = trim_calculate_adjust_amount(info);
            if (t_judge.up == true)
            {
                info->sub_val[ch] += adj_mount;
                comm_UART_Printf(LOG_LV_INFO, "\n\r\t\t[%s▲ TUNE_UP%s] (Next SUB: %3u)", \
                    ANSI_FONT_BLUE, ANSI_FONT_NONE, info->sub_val[ch]);
            }
            else if (t_judge.down == true)
            {
                info->sub_val[ch] -= adj_mount;
                comm_UART_Printf(LOG_LV_INFO, "\n\r\t\t[%s▼ TUNE_DN%s] (Next SUB: %3u)", \
                    ANSI_FONT_MAGENTA, ANSI_FONT_NONE, info->sub_val[ch]);
            }

            if (info->saved[ch].saved_cnt < SAVE_INFO_MAX_CNT)
            {
                comm_UART_Printf(LOG_LV_DEBUG, "\n\r\t Not Enough Save Count");
                if (true == xc_trim_update_register_by_sub_val(*list, info))
                {
                    td->step = TRIM_STEP_INITIAL_BY_LIST;
                    td->tout = XC_DELAY_DEFAULT;
                }
                else
                {
                    // go to error handling, can't update register, trim thread stop
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r\tlist : %s Failed to update trim register!", xc_trim_list_to_string(*list));
                    info->trim_error = true;
                    td->step = TRIM_STEP_PWR_OFF;
                    td->tout = XC_DELAY_DEFAULT;
                }
            }
            else
            {
                // find best value from saved value and update register
                if (info->saved[ch].saved_cnt > 0U)
                {
                    float target_value = info->range.target;
                    float closest_val = info->saved[ch].saved_value[0];
                    uint8_t closest_idx = 0U;

                    float best_diff = fabsf(closest_val - target_value);

                    for (uint8_t idx = 1U; idx < info->saved[ch].saved_cnt; ++idx)
                    {
                        float value = info->saved[ch].saved_value[idx];
                        float diff = fabsf(value - target_value);

                        if (diff < best_diff)
                        {
                            best_diff = diff;
                            closest_val = value;
                            closest_idx = idx;
                        }
                    }

                    info->measure[ch].value = closest_val;
                    info->sub_val[ch] = info->saved[ch].sub_val[closest_idx];
                    comm_UART_Printf(LOG_LV_INFO, "\n\r\t\t[%s★ CLOSEST%s] (Value: %6.3f) (Sub_Val: %3u)", \
                        ANSI_FONT_GREEN, ANSI_FONT_NONE, (double)(info->measure[ch].value), info->sub_val[ch]);
                    xc_trim_update_register_by_sub_val(*list, info);

                    if (++(*list) < XC_TRIM_LIST_MAX)
                    {
                        td->step = TRIM_STEP_INITIAL_BY_LIST;
                        comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<NEXT LIST>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
                    }
                    else
                    {
                        td->step = TRIM_STEP_PREPARE_EFUSE;
                    }
                    td->tout = XC_DELAY_DEFAULT;
                }
            }
            break;
        }

        case TRIM_STEP_PREPARE_EFUSE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            // save mirror register
            xc24_trim_save_mirror_register();
            if (true == xc24_trim_get_efuse_enable())
            {
                comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<E-FUSE START>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
                // register setting
                xc24_trim_init_efuse();
                //power control
                gpio_set_xc_vdd_5v(VCC_ON_5V5);
                td->step = TRIM_STEP_START_EFUSE;
                td->tout = XC_DELAY_DEFAULT;
            }
            else
            {
                comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<E-FUSE SKIP>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
                td->step = TRIM_STEP_LOG_SUMMARY;
                td->tout = XC_DELAY_DEFAULT;
            }
            break;
        }

        case TRIM_STEP_START_EFUSE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            xc24_trim_start_efuse();
            td->step = TRIM_STEP_END_EFUSE;
            td->tout = XC_DELAY_EFUSE_DONE;
            break;
        }

        case TRIM_STEP_END_EFUSE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            gpio_set_xc_vdd_5v(VCC_OFF);
            td->step = TRIM_STEP_PWR_REBOOT;
            td->tout = XC_DELAY_PWR_OFF;
            break;
        }

        case TRIM_STEP_PWR_REBOOT:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            gpio_set_xc_vdd_5v(VCC_ON_3V3);
            td->step = TRIM_STEP_VERIFY_OTP_DUMP;
            td->tout = XC_DELAY_PWR_ON;
            break;
        }

        case TRIM_STEP_VERIFY_OTP_DUMP:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            xc24_trim_init();
            if (0U == xc24_trim_verify_mirror_dump())
            {
                // verify OK
                comm_UART_Printf(LOG_LV_INFO, "\n\r\t%sVerify OK%s", ANSI_FONT_GREEN, ANSI_FONT_NONE);
            }
            else
            {
                // verify NG
                comm_UART_Printf(LOG_LV_INFO, "\n\r\t%sVerify NG%s", ANSI_FONT_RED, ANSI_FONT_NONE);
            }
            td->step = TRIM_STEP_LOG_SUMMARY;
            td->tout = XC_DELAY_DEFAULT;
            break;
        }

        case TRIM_STEP_LOG_SUMMARY:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<SUMMARY>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
            xc_trim_log_summary();
            td->step = TRIM_STEP_PWR_OFF;
            td->tout = XC_DELAY_DEFAULT;
            break;
        }

        case TRIM_STEP_PWR_OFF:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xc_trim_list_to_string(*list), td->tout);
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<POWER OFF>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
            gpio_set_xc_vdd_5v(VCC_OFF);
            if (true == info->trim_error)
            {
                comm_UART_Printf(LOG_LV_ERROR, "\n\r%s[<<<TRIM ERROR>>>]%s", ANSI_FONT_RED, ANSI_FONT_NONE);
            }
            else
            {
                comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<TRIM SUCCESS>>>]%s", ANSI_FONT_GREEN, ANSI_FONT_NONE);
            }
            td->step = TRIM_STEP_NONE;
            td->tout = XC_DELAY_DEFAULT;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, trim_step_to_string((trim_step_t)td->step), td->tout);
            __priv_trim.trim_thr = INVALID_THREAD_ID;
            return false;
        }
    }

    return true;
}

static void xd_trim_param_init(void)
{
    for (xd_trim_list_t trim_list = XD_TRIM_LIST_CURRENT_REF; trim_list < XD_TRIM_LIST_MAX; ++trim_list)
    {
        switch (trim_list)
        {
            case XD_TRIM_LIST_CURRENT_REF:
            {
                __priv_trim.t_xd_trim_info[trim_list].sub_val[0] = XD_DEFAULT_SUB_VAL_CURRENT_REF;
                __priv_trim.t_xd_trim_info[trim_list].range.target = XD_TRIM_TGT_CURRENT_REF;
                __priv_trim.t_xd_trim_info[trim_list].range.min = XD_TRIM_TGT_CURRENT_REF * (1.0f - XD_TRIM_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].range.max = XD_TRIM_TGT_CURRENT_REF * (1.0f + XD_TRIM_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].initial_adj_size = XD_TRIM_INIT_ADJ_CURRENT_REF;
                break;
            }
            case XD_TRIM_LIST_LDO_DIG:
            {
                __priv_trim.t_xd_trim_info[trim_list].sub_val[0] = XD_DEFAULT_SUB_VAL_LDO_DIG;
                __priv_trim.t_xd_trim_info[trim_list].range.target = XD_TRIM_TGT_LDO_DIG;
                __priv_trim.t_xd_trim_info[trim_list].range.min = XD_TRIM_TGT_LDO_DIG * (1.0f - XD_TRIM_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].range.max = XD_TRIM_TGT_LDO_DIG * (1.0f + XD_TRIM_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].initial_adj_size = XD_TRIM_INIT_ADJ_LDO_DIG;
                break;
            }
            case XD_TRIM_LIST_LDO_DAC:
            {
                __priv_trim.t_xd_trim_info[trim_list].sub_val[0] = XD_DEFAULT_SUB_VAL_LDO_DAC;
                __priv_trim.t_xd_trim_info[trim_list].range.target = XD_TRIM_TGT_LDO_DAC;
                __priv_trim.t_xd_trim_info[trim_list].range.min = XD_TRIM_TGT_LDO_DAC * (1.0f - XD_TRIM_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].range.max = XD_TRIM_TGT_LDO_DAC * (1.0f + XD_TRIM_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].initial_adj_size = XD_TRIM_INIT_ADJ_LDO_DAC;
                break;
            }
            case XD_TRIM_LIST_LDO_FLL:
            {
                __priv_trim.t_xd_trim_info[trim_list].sub_val[0] = XD_DEFAULT_SUB_VAL_LDO_FLL;
                __priv_trim.t_xd_trim_info[trim_list].range.target = XD_TRIM_TGT_LDO_FLL;
                __priv_trim.t_xd_trim_info[trim_list].range.min = XD_TRIM_TGT_LDO_FLL * (1.0f - XD_TRIM_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].range.max = XD_TRIM_TGT_LDO_FLL * (1.0f + XD_TRIM_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].initial_adj_size = XD_TRIM_INIT_ADJ_LDO_FLL;
                break;
            }
            case XD_TRIM_LIST_OSC:
            {
                __priv_trim.t_xd_trim_info[trim_list].sub_val[0] = XD_DEFAULT_SUB_VAL_OSC;
                __priv_trim.t_xd_trim_info[trim_list].range.target = XD_TRIM_TGT_OSC;
                __priv_trim.t_xd_trim_info[trim_list].range.min = XD_TRIM_TGT_OSC * (1.0f - XD_TRIM_OSC_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].range.max = XD_TRIM_TGT_OSC * (1.0f + XD_TRIM_OSC_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].initial_adj_size = XD_TRIM_INIT_ADJ_OSC;
                break;
            }
            case XD_TRIM_LIST_CH_GAIN:
            {
                for (uint8_t xd_ch = XD_CH_01; xd_ch < XD_CH_MAX; ++xd_ch)
                {
                    __priv_trim.t_xd_trim_info[trim_list].sub_val[xd_ch] = XD_DEFAULT_SUB_VAL_CH_GAIN;
                }
                __priv_trim.t_xd_trim_info[trim_list].range.target = XD_TRIM_TGT_CH_GAIN;
                __priv_trim.t_xd_trim_info[trim_list].range.min = XD_TRIM_TGT_CH_GAIN * (1.0f - XD_TRIM_IOUT_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].range.max = XD_TRIM_TGT_CH_GAIN * (1.0f + XD_TRIM_IOUT_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].initial_adj_size = XD_TRIM_INIT_ADJ_CH_GAIN;

                __priv_trim.t_xd_trim_info[trim_list].input[0] = XD_TRIM_GAIN_INPUT_1;
                __priv_trim.t_xd_trim_info[trim_list].input[1] = XD_TRIM_GAIN_INPUT_2;
                __priv_trim.t_xd_trim_info[trim_list].gain = GAIN_MID;
                break;
            }
            case XD_TRIM_LIST_CH_OFS:
            {
                for (uint8_t xd_ch = XD_CH_01; xd_ch < XD_CH_MAX; ++xd_ch)
                {
                    __priv_trim.t_xd_trim_info[trim_list].sub_val[xd_ch] = XD_DEFAULT_SUB_VAL_CH_OFS;
                }
                __priv_trim.t_xd_trim_info[trim_list].range.target = XD_TRIM_TGT_CH_OFS;
                __priv_trim.t_xd_trim_info[trim_list].range.min = XD_TRIM_TGT_CH_OFS * (1.0f - XD_TRIM_IOUT_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].range.max = XD_TRIM_TGT_CH_OFS * (1.0f + XD_TRIM_IOUT_ERROR_RANGE);
                __priv_trim.t_xd_trim_info[trim_list].initial_adj_size = XD_TRIM_INIT_ADJ_CH_OFS;

                __priv_trim.t_xd_trim_info[trim_list].input[0] = XD_TRIM_OFS_INPUT_1;
                __priv_trim.t_xd_trim_info[trim_list].input[1] = XD_TRIM_OFS_INPUT_2;
                __priv_trim.t_xd_trim_info[trim_list].gain = GAIN_MID;
                break;
            }
            default:
            {
                FATAL_INVALID_INPUT(trim_list);
                break;
            }
        }
    }
}

static void xd_trim_log_summary(void)
{
    for (xd_trim_list_t list = XD_TRIM_LIST_CURRENT_REF; list < XD_TRIM_LIST_MAX; ++list)
    {
        trim_info_t* info = &__priv_trim.t_xd_trim_info[list];
        uint8_t max_ch = (list < XD_TRIM_LIST_CH_GAIN) ? (XD_CH_01 + 1U) : ((uint8_t)XD_CH_MAX);
        for (uint8_t ch = XD_CH_01; ch < max_ch; ++ch)
        {
            if (ch == XD_CH_01)
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\n|%s|CH|%2u|REG|%3u|SUB|%3u|VAL|%6.3f|", \
                    gs_xd_trim_list[list], (ch + 1U), info->reg_val[ch], info->sub_val[ch], (double)(info->measure[ch].value));
            }
            else
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\n\t\t|CH|%2u|REG|%3u|SUB|%3u|VAL|%6.3f|", \
                    (ch + 1U), info->reg_val[ch], info->sub_val[ch], (double)(info->measure[ch].value));
            }
        }
    }
}

static bool xd_trim_update_register_by_sub_val(xd_trim_list_t in_trim_list, trim_info_t* in_trim_info)
{
    bool ret = false;
    uint16_t reg_val = 0U;
    uint8_t ch = in_trim_info->chx;
    uint16_t sub_val = in_trim_info->sub_val[ch];

    switch(in_trim_list)
    {
        case XD_TRIM_LIST_CURRENT_REF:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[ch] = reg_val;
            ret = xd12_trim_set_current_ref(reg_val);
            break;
        }
        case XD_TRIM_LIST_LDO_DIG:
        {
            reg_val = sub_val ^ 8U;
            in_trim_info->reg_val[ch] = reg_val;
            ret = xd12_trim_set_ldo_dig(reg_val);
            break;
        }
        case XD_TRIM_LIST_LDO_DAC:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[ch] = reg_val;
            ret = xd12_trim_set_ldo_dac(reg_val);
            break;
        }
        case XD_TRIM_LIST_LDO_FLL:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[ch] = reg_val;
            ret = xd12_trim_set_ldo_fll(reg_val);
            break;
        }
        case XD_TRIM_LIST_OSC:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[ch] = reg_val;
            ret = xd12_trim_set_osc(reg_val);
            break;
        }
        case XD_TRIM_LIST_CH_GAIN:
        {
            reg_val = sub_val;
            in_trim_info->reg_val[ch] = reg_val;
            ret = xd12_trim_set_ch_gain(reg_val, ch);
            break;
        }
        case XD_TRIM_LIST_CH_OFS:
        {
            reg_val = (sub_val < 256U) ? (511U - sub_val) : (sub_val - 256U);
            in_trim_info->reg_val[ch] = reg_val;
            ret = xd12_trim_set_ch_ofs(reg_val, ch);
            break;
        }
        default:
        {
            FATAL_INVALID_INPUT(in_trim_list);
            break;
        }
    }
    if (ret == true)
    {
        comm_UART_Printf(LOG_LV_INFO, "\n\r\t\t[%sREG_UPDATE%s] (REG: %3u) (SUB: %3u)", ANSI_FONT_YELLOW, ANSI_FONT_NONE, reg_val, sub_val);
    }
    else
    {
        comm_UART_Printf(LOG_LV_ERROR, "\n\r\t\t[%sREG_UPDATE_FAIL%s] (REG: %3u) (SUB: %3u)", ANSI_FONT_RED, ANSI_FONT_NONE, reg_val, sub_val);
    }
    return ret;
}

static bool _xd_trim_thread(struct thread_data* td)
{
    if (td == NULL)
    {
        return false;
    }
    xd_trim_list_t* list = &__priv_trim.t_xd_trim_list;
    trim_info_t* info = &__priv_trim.t_xd_trim_info[*list];

    switch(td->step)
    {
        case TRIM_STEP_PWR_ON:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, trim_step_to_string((trim_step_t)td->step), td->tout);
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            td->step = TRIM_STEP_INITIAL;
            td->tout = XD_DELAY_PWR_ON;
            break;
        }

        case TRIM_STEP_INITIAL:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            xd12_trim_init();
            td->step = TRIM_STEP_INITIAL_BY_LIST;
            td->tout = XD_DELAY_DEFAULT;
            break;
        }

        case TRIM_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            switch (*list)
            {
                case XD_TRIM_LIST_CURRENT_REF:
                {
                    xd12_trim_init_current_ref();
                    break;
                }
                case XD_TRIM_LIST_LDO_DIG:
                {
                    xd12_trim_init_ldo_dig();
                    break;
                }
                case XD_TRIM_LIST_LDO_DAC:
                {
                    xd12_trim_init_ldo_dac();
                    break;
                }
                case XD_TRIM_LIST_LDO_FLL:
                {
                    xd12_trim_init_ldo_fll();
                    break;
                }
                case XD_TRIM_LIST_OSC:
                {
                    xd12_trim_init_osc();
                    break;
                }
                case XD_TRIM_LIST_CH_GAIN:
                {
                    gpio_set_vled_9v(VLED_ON);
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);
                    xd12_trim_init_ch_gain();
                    xd12_trim_set_max_curr_vref(info->input[info->repeat]);
                    xd12_trim_set_channel_enable(info->chx);
                    gpio_set_demux_channel_selection((XD_CH_t)info->chx);
                    gpio_set_current_gain(info->gain);
                    break;
                }
                case XD_TRIM_LIST_CH_OFS:
                {
                    gpio_set_vled_9v(VLED_ON);
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);
                    xd12_trim_init_ch_ofs();
                    xd12_trim_set_max_curr_vref(info->input[info->repeat]);
                    xd12_trim_set_channel_enable(info->chx);
                    gpio_set_demux_channel_selection((XD_CH_t)info->chx);
                    gpio_set_current_gain(info->gain);
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, gs_trim_step[td->step], *list);
                    info->trim_error = true;
                    td->step = TRIM_STEP_PWR_OFF;
                    return true;
                }
            }
            td->step = TRIM_STEP_START_MEASURE;
            td->tout = XD_DELAY_SETTLING;
            break;
        }

        case TRIM_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            switch (*list)
            {
                case XD_TRIM_LIST_CURRENT_REF:
                case XD_TRIM_LIST_LDO_DIG:
                case XD_TRIM_LIST_LDO_DAC:
                case XD_TRIM_LIST_LDO_FLL:
                {
                    mcu_peripheral_adc_start();
                    break;
                }
                case XD_TRIM_LIST_OSC:
                {
                    mcu_peripheral_tim_input_capture_start();
                    break;
                }
                case XD_TRIM_LIST_CH_GAIN:
                case XD_TRIM_LIST_CH_OFS:
                {
                    ADS114S08_Set_Start(true);
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, trim_step_to_string((trim_step_t)td->step), *list);
                    info->trim_error = true;
                    td->step = TRIM_STEP_PWR_OFF;
                    return true;
                }
            }
            td->step = TRIM_STEP_GET_MEASURED_VALUE;
            td->tout = XD_DELAY_MEASURE;
            break;
        }

        case TRIM_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            float f_converted_value = 0.0f;
            uint8_t ch = info->chx;
            switch (*list)
            {
                case XD_TRIM_LIST_CURRENT_REF:
                case XD_TRIM_LIST_LDO_DIG:
                case XD_TRIM_LIST_LDO_DAC:
                case XD_TRIM_LIST_LDO_FLL:
                {
                    info->measure[ch].value = mcu_peripheral_adc_conversion_to_voltage(mcu_peripheral_adc_get());
                    td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                    break;
                }
                case XD_TRIM_LIST_OSC:
                {
                    info->measure[ch].value = mcu_peripheral_tim_conversion_freq() * XD_CONST_OSC;
                    td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                    break;
                }
                case XD_TRIM_LIST_CH_GAIN:
                case XD_TRIM_LIST_CH_OFS:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        f_converted_value = JigBD_IF_Convert_Adc_To_Current(ADS114S08_Get_ADC_Value(), info->gain);
                        info->measure[ch].temp_value[info->repeat] = f_converted_value;
                        ++info->repeat;

                        if (info->repeat < 2U)
                        {
                            td->step = TRIM_STEP_INITIAL_BY_LIST;
                        }
                        else
                        {
                            float val_0 = info->measure[ch].temp_value[0];
                            float val_1 = info->measure[ch].temp_value[1];

                            if (*list == XD_TRIM_LIST_CH_GAIN)
                            {
                                info->measure[ch].value = fabsf(val_0 - val_1);
                            }
                            else
                            {
                                info->measure[ch].value = ((val_0 + val_1) / 2.0f);
                            }
                            td->step = TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER;
                            info->repeat = 0U;
                        }
                    }
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, trim_step_to_string((trim_step_t)td->step), *list);
                    info->trim_error = true;
                    td->step = TRIM_STEP_PWR_OFF;
                    return true;
                }
            }
            td->tout = XD_DELAY_DEFAULT;
            break;
        }

        case TRIM_STEP_JUDGE_RANGE_UPDATE_REGISTER:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            judge_info_t t_judge = trim_compare_range(info);
            uint8_t ch = info->chx;

            if (info->saved[ch].try_cnt > MAX_TRIM_TRY_CNT)
            {
                comm_UART_Printf(LOG_LV_ERROR, "\n\r\t[FAIL] list: %s (ch: %u) Max try count exceeded! (%u/%u)",\
                    xd_trim_list_to_string(*list), (ch + 1U), info->saved[ch].try_cnt, MAX_TRIM_TRY_CNT);

                info->trim_error = true;
                td->step = TRIM_STEP_PWR_OFF; // 또는 에러 처리 단계
                td->tout = XD_DELAY_DEFAULT;
                break;
            }

            if (true == t_judge.in_range)
            {
                /* In Range - Save Value */
                info->saved[ch].saved_value[info->saved[ch].saved_cnt] = info->measure[ch].value;
                info->saved[ch].sub_val[info->saved[ch].saved_cnt] = info->sub_val[ch];
                ++info->saved[ch].saved_cnt;
                comm_UART_Printf(LOG_LV_INFO, "\n\r[%s]\r\n\t\t(try: %3u)(ch: %2u, Sub: %u) (Value: %6.3f) -> [%s✔ IN_RANGE (%2u)%s] (TGT: %6.3f, MIN: %6.3f, MAX: %6.3f)", \
                    xd_trim_list_to_string(*list), info->saved[ch].try_cnt, (ch + 1U), info->sub_val[ch], (double)(info->measure[ch].value), \
                    ANSI_FONT_GREEN, info->saved[ch].saved_cnt, ANSI_FONT_NONE, \
                    (double)(info->range.target), (double)(info->range.min), (double)(info->range.max));
            }
            else
            {
                comm_UART_Printf(LOG_LV_INFO, "\n\r[%s]\r\n\t\t(try: %3u)(ch: %2u, Sub: %u) (Value: %6.3f) -> [%s✕   OUT_RANGE   %s] (TGT: %6.3f, MIN: %6.3f, MAX: %6.3f)", \
                    xd_trim_list_to_string(*list), info->saved[ch].try_cnt, (ch + 1U), info->sub_val[ch], (double)(info->measure[ch].value), \
                    ANSI_FONT_RED, ANSI_FONT_NONE, \
                    (double)(info->range.target), (double)(info->range.min), (double)(info->range.max));
            }

            uint16_t adj_mount = trim_calculate_adjust_amount(info);
            if (t_judge.up == true)
            {
                info->sub_val[ch] += adj_mount;
                comm_UART_Printf(LOG_LV_INFO, "\n\r\t\t[%s▲ TUNE_UP%s] (Next SUB: %3u)", \
                    ANSI_FONT_BLUE, ANSI_FONT_NONE, info->sub_val[ch]);
            }
            else if (t_judge.down == true)
            {
                info->sub_val[ch] -= adj_mount;
                comm_UART_Printf(LOG_LV_INFO, "\n\r\t\t[%s▼ TUNE_DN%s] (Next SUB: %3u)", \
                    ANSI_FONT_MAGENTA, ANSI_FONT_NONE, info->sub_val[ch]);
            }

            if (info->saved[ch].saved_cnt < SAVE_INFO_MAX_CNT)
            {
                comm_UART_Printf(LOG_LV_DEBUG, "\n\r\t Not Enough Save Count");
                if (true == xd_trim_update_register_by_sub_val(*list, info))
                {
                    td->step = TRIM_STEP_INITIAL_BY_LIST;
                    td->tout = XD_DELAY_DEFAULT;
                }
                else
                {
                    // go to error handling, can't update register, trim thread stop
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r\tlist : %s Failed to update trim register!", xd_trim_list_to_string(*list));
                    info->trim_error = true;
                    td->step = TRIM_STEP_PWR_OFF;
                    td->tout = XD_DELAY_DEFAULT;
                }
            }
            else
            {
                // find best value from saved value and update register
                if (info->saved[ch].saved_cnt > 0U)
                {
                    float target_value = info->range.target;
                    float closest_val = info->saved[ch].saved_value[0];
                    uint8_t closest_idx = 0U;

                    float best_diff = fabsf(closest_val - target_value);

                    for (uint8_t idx = 1U; idx < info->saved[ch].saved_cnt; ++idx)
                    {
                        float value = info->saved[ch].saved_value[idx];
                        float diff = fabsf(value - target_value);

                        if (diff < best_diff)
                        {
                            best_diff = diff;
                            closest_val = value;
                            closest_idx = idx;
                        }
                    }

                    info->measure[ch].value = closest_val;
                    info->sub_val[ch] = info->saved[ch].sub_val[closest_idx];
                    comm_UART_Printf(LOG_LV_INFO, "\n\r\t\t[%s★ CLOSEST%s] (Value: %6.3f) (Sub_Val: %3u)", \
                        ANSI_FONT_GREEN, ANSI_FONT_NONE, (double)(info->measure[ch].value), info->sub_val[ch]);
                    xd_trim_update_register_by_sub_val(*list, info);

                    if (((*list == XD_TRIM_LIST_CH_GAIN) || (*list == XD_TRIM_LIST_CH_OFS)) && (ch < (XD_CH_MAX - 1U)))
                    {
                        ++(info->chx);
                        td->step = TRIM_STEP_INITIAL_BY_LIST;
                        comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<NEXT CHANNEL>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
                        td->tout = XD_DELAY_DEFAULT;
                    }
                    else
                    {
                        if (++(*list) < XD_TRIM_LIST_MAX)
                        {
                            td->step = TRIM_STEP_INITIAL_BY_LIST;
                            comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<NEXT LIST>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
                        }
                        else
                        {
                            td->step = TRIM_STEP_PREPARE_EFUSE;
                        }
                        td->tout = XD_DELAY_DEFAULT;
                    }
                }
            }
            break;
        }

        case TRIM_STEP_PREPARE_EFUSE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            // save mirror register
            xd12_trim_save_mirror_register();
            if (true == xd12_trim_get_efuse_enable())
            {
                comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<E-FUSE START>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
                // register setting
                xd12_trim_init_efuse();
                //power control
                gpio_set_xd_vdd_5v(VCC_ON_5V5);
                td->step = TRIM_STEP_START_EFUSE;
                td->tout = XD_DELAY_DEFAULT;
            }
            else
            {
                comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<E-FUSE SKIP>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
                td->step = TRIM_STEP_LOG_SUMMARY;
                td->tout = XD_DELAY_DEFAULT;
            }
            break;
        }

        case TRIM_STEP_START_EFUSE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            xd12_trim_start_efuse();
            td->step = TRIM_STEP_END_EFUSE;
            td->tout = XD_DELAY_EFUSE_DONE;
            break;
        }

        case TRIM_STEP_END_EFUSE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            gpio_set_xd_vdd_5v(VCC_OFF);
            td->step = TRIM_STEP_PWR_REBOOT;
            td->tout = XD_DELAY_PWR_OFF;
            break;
        }

        case TRIM_STEP_PWR_REBOOT:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            td->step = TRIM_STEP_VERIFY_OTP_DUMP;
            td->tout = XD_DELAY_PWR_ON;
            break;
        }

        case TRIM_STEP_VERIFY_OTP_DUMP:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            xd12_trim_init();
            if (0U == xd12_trim_verify_mirror_dump())
            {
                // verify OK
                comm_UART_Printf(LOG_LV_INFO, "\n\r\t%sVerify OK%s", ANSI_FONT_GREEN, ANSI_FONT_NONE);
            }
            else
            {
                // verify NG
                comm_UART_Printf(LOG_LV_INFO, "\n\r\t%sVerify NG%s", ANSI_FONT_RED, ANSI_FONT_NONE);
                info->trim_error = true;
            }

            td->step = TRIM_STEP_LOG_SUMMARY;
            td->tout = XD_DELAY_DEFAULT;
            break;
        }

        case TRIM_STEP_LOG_SUMMARY:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<SUMMARY>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
            xd_trim_log_summary();
            td->step = TRIM_STEP_PWR_OFF;
            td->tout = XD_DELAY_DEFAULT;
            break;
        }

        case TRIM_STEP_PWR_OFF:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", trim_step_to_string((trim_step_t)td->step), xd_trim_list_to_string(*list), td->tout);
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<POWER OFF>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
            gpio_set_xd_vdd_5v(VCC_OFF);
            gpio_set_vled_9v(VLED_OFF);
            if (true == info->trim_error)
            {
                comm_UART_Printf(LOG_LV_ERROR, "\n\r%s[<<<TRIM ERROR>>>]%s", ANSI_FONT_RED, ANSI_FONT_NONE);
            }
            else
            {
                comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<TRIM SUCCESS>>>]%s", ANSI_FONT_GREEN, ANSI_FONT_NONE);
            }
            td->step = TRIM_STEP_NONE;
            td->tout = XD_DELAY_DEFAULT;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, trim_step_to_string((trim_step_t)td->step), td->tout);
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
        xd_trim_param_init();
        xc_trim_param_init();

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
        case TRIM_CMD_XC_START:
        {
            if(__priv_trim.trim_thr == INVALID_THREAD_ID)
            {
                __priv_trim.trim_thr = fw_begin_thread_ex(_xc_trim_thread, 10U);    /* 10ms */
            }
            break;
        }
        case TRIM_CMD_XD_START:
        {
            if(__priv_trim.trim_thr == INVALID_THREAD_ID)
            {
                __priv_trim.trim_thr = fw_begin_thread_ex(_xd_trim_thread, 10U);    /* 10ms */
            }
            break;
        }
        default:
        {
            break;
        }
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