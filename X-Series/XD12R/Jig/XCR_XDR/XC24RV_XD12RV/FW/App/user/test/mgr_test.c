#include <string.h>
#include <stdio.h>

#include "framework.h"
#include "drv_xdr12.h"
#include "drv_xcr24.h"
#include "ads124s08.h"
#include "comm_debugging.h"

#define SEQUENCE_DEBUG

#define STEP_DELAY_DEFAULT      (10U)
#define STEP_DELAY_MEASURE      (100U)
#define STEP_DELAY_PWR_ON       (100U)

//#define XDR_SWEEP_VREF_GAP      (15U)
#define XDR_SWEEP_VREF_GAP      (1365U)

typedef struct tag_MEASURE_INFO
{
    uint16_t adc;
    float value;
} measure_info_t;

typedef struct tag_TEST_INFO
{
    XD_CH_t chx;
    measure_info_t measure[XD_CH_MAX];
} test_info_t;

typedef struct tag_TEST_SWEEP_INFO
{
    XD_CH_t chx;
    max_curr_level_t max_curr_level;
    max_curr_level_t max_curr_level_target;
    uint16_t vref;
    uint16_t vref_gap;
    measure_info_t measure[XD_CH_MAX];
} sweep_info_t;

typedef enum tag_TEST_STEP_T
{
    TEST_STEP_PWR_ON = 0U,
    TEST_STEP_INITIAL,
    TEST_STEP_INITIAL_BY_LIST,
    TEST_STEP_START_MEASURE,
    TEST_STEP_GET_MEASURED_VALUE,
    TEST_STEP_LOG_SUMMARY,
    TEST_STEP_PWR_OFF,
    TEST_STEP_NONE,
    TEST_STEP_MAX,
} test_step_t;

typedef enum tag_XCR_TEST_LIST
{
    XCR_TEST_LIST_ICC_STBY = 0U,
    XCR_TEST_LIST_ICC_ACTV,
    XCR_TEST_LIST_LDO,
    XCR_TEST_LIST_LDO_FLL_A,
    XCR_TEST_LIST_LDO_FLL_B,
    XCR_TEST_LIST_FLL_A_30M,
    XCR_TEST_LIST_FLL_A_35M,
    XCR_TEST_LIST_FLL_A_40M,
    XCR_TEST_LIST_FLL_B_30M,
    XCR_TEST_LIST_FLL_B_35M,
    XCR_TEST_LIST_FLL_B_40M,
    XCR_TEST_LIST_MAX,
} xcr_test_list_t;

typedef enum tag_XDR_TEST_LIST
{
    XDR_TEST_LIST_ICC_STBY = 0U,
    XDR_TEST_LIST_ICC_ACTV,
    XDR_TEST_LIST_CURRENT_REF,
    XDR_TEST_LIST_LDO_DIG,
    XDR_TEST_LIST_LDO_DAC,
    XDR_TEST_LIST_LDO_FLL,
    XDR_TEST_LIST_OSC,
    XDR_TEST_LIST_FLL_40M,
    XDR_TEST_LIST_FLL_50M,
    XDR_TEST_LIST_FLL_60M,
    XDR_TEST_LIST_IOUT_P1,
    XDR_TEST_LIST_IOUT_P2,
    XDR_TEST_LIST_IOUT_P3,
    XDR_TEST_LIST_MAX_SWEEP_P01,
    XDR_TEST_LIST_MAX_SWEEP_P02,
    XDR_TEST_LIST_MAX_SWEEP_P03,
    XDR_TEST_LIST_MAX_SWEEP_P04,
    XDR_TEST_LIST_MAX_SWEEP_P05,
    XDR_TEST_LIST_MAX_SWEEP_P06,
    XDR_TEST_LIST_MAX_SWEEP_P07,
    XDR_TEST_LIST_MAX_SWEEP_P08,
    XDR_TEST_LIST_MAX_SWEEP_P09,
    XDR_TEST_LIST_MAX_SWEEP_P10,
    XDR_TEST_LIST_MAX_SWEEP_P11,
    XDR_TEST_LIST_MAX_SWEEP_P12,
    XDR_TEST_LIST_MAX_SWEEP_P13,
    XDR_TEST_LIST_MAX_SWEEP_P14,
    XDR_TEST_LIST_MAX_SWEEP_P15,
    XDR_TEST_LIST_MAX_SWEEP_P16,
    XDR_TEST_LIST_MAX,
} xdr_test_list_t;

typedef void (*xcr24_test_init_func)(void);
typedef void (*xcr24_test_start_func)(void);

typedef void (*xdr12_test_init_func)(void);
typedef void (*xdr12_test_start_func)(void);

static struct{
    MGRSTATUS   status;
    test_info_t t_xdr_test_info[XDR_TEST_LIST_MAX];
    test_info_t t_xcr_test_info[XCR_TEST_LIST_MAX];
    sweep_info_t t_xdr_sweep_test_info;
    xdr_test_list_t t_xd_test_list;
    xcr_test_list_t t_xc_test_list;
    THREAD_ID   test_thr;
}__priv_test;

static const char* gs_xcr_test_list[XCR_TEST_LIST_MAX] =
{
    "XCR_ICC_STBY ",
    "XCR_ICC_ACTV ",
    "XCR_LDO      ",
    "XCR_LDO_FLL_A",
    "XCR_LDO_FLL_B",
    "XCR_FLL_A_30M",
    "XCR_FLL_A_35M",
    "XCR_FLL_A_40M",
    "XCR_FLL_B_30M",
    "XCR_FLL_B_35M",
    "XCR_FLL_B_40M",
};

static const xcr24_test_init_func gp_xcr24_test_init_func[XCR_TEST_LIST_MAX] =
{
    [XCR_TEST_LIST_ICC_STBY] = xcr24_test_init_icc_stby,
    [XCR_TEST_LIST_ICC_ACTV] = xcr24_test_init_icc_actv,
    [XCR_TEST_LIST_LDO] = xcr24_test_init_ldo,
    [XCR_TEST_LIST_LDO_FLL_A] = xcr24_test_init_ldo_fll_a,
    [XCR_TEST_LIST_LDO_FLL_B] = xcr24_test_init_ldo_fll_b,
    [XCR_TEST_LIST_FLL_A_30M] = xcr24_test_init_fll_a_30m,
    [XCR_TEST_LIST_FLL_A_35M] = xcr24_test_init_fll_a_35m,
    [XCR_TEST_LIST_FLL_A_40M] = xcr24_test_init_fll_a_40m,
    [XCR_TEST_LIST_FLL_B_30M] = xcr24_test_init_fll_b_30m,
    [XCR_TEST_LIST_FLL_B_35M] = xcr24_test_init_fll_b_35m,
    [XCR_TEST_LIST_FLL_B_40M] = xcr24_test_init_fll_b_40m,
};

static const xcr24_test_start_func gp_xcr24_test_start_func[XCR_TEST_LIST_MAX] =
{
    [XCR_TEST_LIST_ICC_STBY] = xcr24_test_start_icc_stby,
    [XCR_TEST_LIST_ICC_ACTV] = xcr24_test_start_icc_actv,
    [XCR_TEST_LIST_LDO] = xcr24_test_start_ldo,
    [XCR_TEST_LIST_LDO_FLL_A] = xcr24_test_start_ldo_fll_a,
    [XCR_TEST_LIST_LDO_FLL_B] = xcr24_test_start_ldo_fll_b,
    [XCR_TEST_LIST_FLL_A_30M] = xcr24_test_start_fll_a_30m,
    [XCR_TEST_LIST_FLL_A_35M] = xcr24_test_start_fll_a_35m,
    [XCR_TEST_LIST_FLL_A_40M] = xcr24_test_start_fll_a_40m,
    [XCR_TEST_LIST_FLL_B_30M] = xcr24_test_start_fll_b_30m,
    [XCR_TEST_LIST_FLL_B_35M] = xcr24_test_start_fll_b_35m,
    [XCR_TEST_LIST_FLL_B_40M] = xcr24_test_start_fll_b_40m,
};

static const char* gs_xdr_test_list[XDR_TEST_LIST_MAX] =
{
    "XDR_ICC_STBY     ",
    "XDR_ICC_ACTV     ",
    "XDR_CURRENT_REF  ",
    "XDR_LDO_DIG      ",
    "XDR_LDO_DAC      ",
    "XDR_LDO_FLL      ",
    "XDR_OSC          ",
    "XDR_FLL_40M      ",
    "XDR_FLL_50M      ",
    "XDR_FLL_60M      ",
    "XDR_IOUT_P1      ",
    "XDR_IOUT_P2      ",
    "XDR_IOUT_P3      ",
    "XDR_MAX_SWEEP_P01",
    "XDR_MAX_SWEEP_P02",
    "XDR_MAX_SWEEP_P03",
    "XDR_MAX_SWEEP_P04",
    "XDR_MAX_SWEEP_P05",
    "XDR_MAX_SWEEP_P06",
    "XDR_MAX_SWEEP_P07",
    "XDR_MAX_SWEEP_P08",
    "XDR_MAX_SWEEP_P09",
    "XDR_MAX_SWEEP_P10",
    "XDR_MAX_SWEEP_P11",
    "XDR_MAX_SWEEP_P12",
    "XDR_MAX_SWEEP_P13",
    "XDR_MAX_SWEEP_P14",
    "XDR_MAX_SWEEP_P15",
    "XDR_MAX_SWEEP_P16",
};

static const xdr12_test_init_func gp_xdr12_test_init_func[XDR_TEST_LIST_MAX] =
{
    [XDR_TEST_LIST_ICC_STBY] = xdr12_test_init_icc_stby,
    [XDR_TEST_LIST_ICC_ACTV] = xdr12_test_init_icc_actv,
    [XDR_TEST_LIST_CURRENT_REF] = xdr12_test_init_current_ref,
    [XDR_TEST_LIST_LDO_DIG] = xdr12_test_init_ldo_dig,
    [XDR_TEST_LIST_LDO_DAC] = xdr12_test_init_ldo_dac,
    [XDR_TEST_LIST_LDO_FLL] = xdr12_test_init_ldo_fll,
    [XDR_TEST_LIST_OSC] = xdr12_test_init_osc,
    [XDR_TEST_LIST_FLL_40M] = xdr12_test_init_fll_40M,
    [XDR_TEST_LIST_FLL_50M] = xdr12_test_init_fll_50M,
    [XDR_TEST_LIST_FLL_60M] = xdr12_test_init_fll_60M,
    [XDR_TEST_LIST_IOUT_P1] = xdr12_test_init_iout_3P,
    [XDR_TEST_LIST_IOUT_P2] = xdr12_test_init_iout_3P,
    [XDR_TEST_LIST_IOUT_P3] = xdr12_test_init_iout_3P,
    [XDR_TEST_LIST_MAX_SWEEP_P01] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P02] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P03] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P04] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P05] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P06] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P07] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P08] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P09] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P10] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P11] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P12] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P13] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P14] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P15] = xdr12_test_init_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P16] = xdr12_test_init_max_sweep,
};

static const xdr12_test_start_func gp_xdr12_test_start_func[XDR_TEST_LIST_MAX] =
{
    [XDR_TEST_LIST_ICC_STBY] = xdr12_test_start_icc_stby,
    [XDR_TEST_LIST_ICC_ACTV] = xdr12_test_start_icc_actv,
    [XDR_TEST_LIST_CURRENT_REF] = xdr12_test_start_current_ref,
    [XDR_TEST_LIST_LDO_DIG] = xdr12_test_start_ldo_dig,
    [XDR_TEST_LIST_LDO_DAC] = xdr12_test_start_ldo_dac,
    [XDR_TEST_LIST_LDO_FLL] = xdr12_test_start_ldo_fll,
    [XDR_TEST_LIST_OSC] = xdr12_test_start_osc,
    [XDR_TEST_LIST_FLL_40M] = xdr12_test_start_fll_40M,
    [XDR_TEST_LIST_FLL_50M] = xdr12_test_start_fll_50M,
    [XDR_TEST_LIST_FLL_60M] = xdr12_test_start_fll_60M,
    [XDR_TEST_LIST_IOUT_P1] = xdr12_test_start_iout_3P,
    [XDR_TEST_LIST_IOUT_P2] = xdr12_test_start_iout_3P,
    [XDR_TEST_LIST_IOUT_P3] = xdr12_test_start_iout_3P,
    [XDR_TEST_LIST_MAX_SWEEP_P01] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P02] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P03] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P04] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P05] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P06] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P07] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P08] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P09] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P10] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P11] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P12] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P13] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P14] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P15] = xdr12_test_start_max_sweep,
    [XDR_TEST_LIST_MAX_SWEEP_P16] = xdr12_test_start_max_sweep,
};

static const char* gs_test_step[TEST_STEP_MAX] =
{
    "TEST_STEP_PWR_ON",
    "TEST_STEP_INITIAL",
    "TEST_STEP_INITIAL_BY_LIST",
    "TEST_STEP_START_MEASURE",
    "TEST_STEP_GET_MEASURED_VALUE",
    "TEST_STEP_LOG_SUMMARY",
    "TEST_STEP_PWR_OFF",
    "TEST_STEP_NONE",
};

static const char* xcr_test_list_to_string(xcr_test_list_t list)
{
    if (list < XCR_TEST_LIST_MAX)
    {
        return gs_xcr_test_list[list];
    }

    return "XCR_TEST_LIST_INVALID";
}

static const char* xdr_test_list_to_string(xdr_test_list_t list)
{
    if (list < XDR_TEST_LIST_MAX)
    {
        return gs_xdr_test_list[list];
    }

    return "XDR_TEST_LIST_INVALID";
}

static const char* test_step_to_string(test_step_t step)
{
    if (step < TEST_STEP_MAX)
    {
        return gs_test_step[step];
    }

    return "TEST_STEP_INVALID";
}

static void xcr_test_log_summary(void)
{
    for (xcr_test_list_t list = XCR_TEST_LIST_ICC_STBY ; list < XCR_TEST_LIST_MAX ; ++list)
    {
        test_info_t* info = &__priv_test.t_xcr_test_info[list];
        comm_UART_Printf(LOG_LV_INFO, "\r\n[%s]\t[VAL: %06.3f]", xcr_test_list_to_string(list), (double)(info->measure[0].value));
    }
}

static bool _xcr_test_thread(struct thread_data* td)
{
    xcr_test_list_t* list = &__priv_test.t_xc_test_list;
    test_info_t *info = &__priv_test.t_xcr_test_info[*list];
    switch(td->step)
    {
        case TEST_STEP_PWR_ON:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            gpio_set_xc_vdd_5v(VCC_ON_3V3);
            td->step = TEST_STEP_INITIAL;
            td->tout = STEP_DELAY_PWR_ON;
            break;
        }

        case TEST_STEP_INITIAL:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xcr_test_list_to_string(*list), td->tout);
            gp_xcr24_test_init_func[*list]();
            td->step = TEST_STEP_START_MEASURE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xcr_test_list_to_string(*list), td->tout);
            gp_xcr24_test_start_func[*list]();
            td->step = TEST_STEP_GET_MEASURED_VALUE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xcr_test_list_to_string(*list), td->tout);
            uint16_t* p_adc_value = &info->measure[info->chx].adc;
            float* p_value = &info->measure[info->chx].value;
            switch (*list)
            {
                case XCR_TEST_LIST_ICC_STBY:
                case XCR_TEST_LIST_ICC_ACTV:
                case XCR_TEST_LIST_LDO:
                case XCR_TEST_LIST_LDO_FLL_A:
                case XCR_TEST_LIST_LDO_FLL_B:
                {
                    *p_adc_value = ADS114S08_Get_ADC_Value();
                    *p_value = JigBD_IF_Convert_Adc_To_ICC(*p_adc_value);
                    break;
                }
                case XCR_TEST_LIST_FLL_A_30M:
                case XCR_TEST_LIST_FLL_A_35M:
                case XCR_TEST_LIST_FLL_A_40M:
                case XCR_TEST_LIST_FLL_B_30M:
                case XCR_TEST_LIST_FLL_B_35M:
                case XCR_TEST_LIST_FLL_B_40M:
                {
                    *p_value = mcu_peripheral_tim_conversion_freq() * XCR_CONST_OSC;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (++(*list) < XCR_TEST_LIST_MAX)
            {
                td->step = TEST_STEP_INITIAL_BY_LIST;
            }
            else
            {
                td->step = TEST_STEP_LOG_SUMMARY;
            }
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_LOG_SUMMARY:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            xcr_test_log_summary();
            td->step = TEST_STEP_PWR_OFF;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_PWR_OFF:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            gpio_set_xc_vdd_5v(VCC_OFF);
            td->step = TEST_STEP_NONE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            __priv_test.test_thr = INVALID_THREAD_ID;
            return false;
        }
    }
    return true;
}

static void xdr_test_log_summary(void)
{
    char log_buf[350] = {0};
    int log_buf_len = 0U;
    for (xdr_test_list_t list = XDR_TEST_LIST_ICC_STBY ; list < XDR_TEST_LIST_MAX ; ++list)
    {
        test_info_t* info = &__priv_test.t_xdr_test_info[list];
        uint8_t max_ch = (list < XDR_TEST_LIST_IOUT_P1) ? (uint8_t)(XD_CH_01 + 1U) : (uint8_t)XD_CH_MAX;
        for (uint8_t ch = XD_CH_01 ; ch < max_ch ; ++ch)
        {
            if (ch == XD_CH_01)
            {
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "\r\n[%s]\t[VAL: %06.3f]", xdr_test_list_to_string(list), (double)(info->measure[ch].value));
            }
            else
            {
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, " [%06.3f]", (double)(info->measure[ch].value));
            }
        }
        comm_UART_Printf(LOG_LV_INFO, "%s", log_buf);
        memset(log_buf, 0, sizeof(log_buf));
        log_buf_len = 0U;
    }
}

static bool _xdr_test_thread(struct thread_data* td)
{
    xdr_test_list_t* list = &__priv_test.t_xd_test_list;
    test_info_t *info = &__priv_test.t_xdr_test_info[*list];
    switch(td->step)
    {
        case TEST_STEP_PWR_ON:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            td->step = TEST_STEP_INITIAL;
            td->tout = STEP_DELAY_PWR_ON;
            break;
        }

        case TEST_STEP_INITIAL:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xdr_test_list_to_string(*list), td->tout);
            if (*list < XDR_TEST_LIST_MAX)
            {
                gp_xdr12_test_init_func[*list]();
                if (*list >= XDR_TEST_LIST_IOUT_P1)
                {
                    xdr12_trim_set_channel_enable(info->chx);
                    gpio_set_demux_channel_selection(info->chx);
                }
            }
            else
            {
                comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, test_step_to_string((test_step_t)td->step), *list);
            }
            td->step = TEST_STEP_START_MEASURE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xdr_test_list_to_string(*list), td->tout);
            if (*list < XDR_TEST_LIST_MAX)
            {
                gp_xdr12_test_start_func[*list]();
                if (*list == XDR_TEST_LIST_IOUT_P1 || *list == XDR_TEST_LIST_IOUT_P2 || *list == XDR_TEST_LIST_IOUT_P3)
                {
                    const uint16_t iout_P3_vref_table[3] = { 300U, 700U, 1000U };
                    xdr12_trim_set_max_curr_vref(iout_P3_vref_table[*list - XDR_TEST_LIST_IOUT_P1]);
                }
                else if (*list >= XDR_TEST_LIST_MAX_SWEEP_P01 && *list <= XDR_TEST_LIST_MAX_SWEEP_P16)
                {
                    const uint16_t max_sweep_curr_lvl_table[16] =
                    {
                         CURR_LEVEL_4,  CURR_LEVEL_8, CURR_LEVEL_12, CURR_LEVEL_16,
                        CURR_LEVEL_20, CURR_LEVEL_24, CURR_LEVEL_28, CURR_LEVEL_32,
                        CURR_LEVEL_36, CURR_LEVEL_40, CURR_LEVEL_44, CURR_LEVEL_48,
                        CURR_LEVEL_52, CURR_LEVEL_56, CURR_LEVEL_60, CURR_LEVEL_64,
                    };
                    xdr12_trim_set_max_curr_lvl(max_sweep_curr_lvl_table[*list - XDR_TEST_LIST_MAX_SWEEP_P01]);
                }
            }
            else
            {
                comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, test_step_to_string((test_step_t)td->step), *list);
            }
            td->step = TEST_STEP_GET_MEASURED_VALUE;
            td->tout = STEP_DELAY_MEASURE;
            break;
        }

        case TEST_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xdr_test_list_to_string(*list), td->tout);
            uint16_t* p_adc_value = &info->measure[info->chx].adc;
            float* p_value = &info->measure[info->chx].value;
            switch (*list)
            {
                case XDR_TEST_LIST_ICC_STBY:
                case XDR_TEST_LIST_ICC_ACTV:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_ICC(*p_adc_value);
                    }
                    else
                    {
#ifndef SEQUENCE_DEBUG
                        return true;
#endif
                    }
                    break;
                }
                case XDR_TEST_LIST_CURRENT_REF:
                case XDR_TEST_LIST_LDO_DIG:
                case XDR_TEST_LIST_LDO_DAC:
                case XDR_TEST_LIST_LDO_FLL:
                {
                    *p_adc_value = mcu_peripheral_adc_get();
                    *p_value = mcu_peripheral_adc_conversion_to_voltage(*p_adc_value);
                    break;
                }
                case XDR_TEST_LIST_OSC:
                case XDR_TEST_LIST_FLL_40M:
                case XDR_TEST_LIST_FLL_50M:
                case XDR_TEST_LIST_FLL_60M:
                {
                    *p_value = mcu_peripheral_tim_conversion_freq() * XDR_CONST_OSC;
                    break;
                }
                case XDR_TEST_LIST_IOUT_P1:
                case XDR_TEST_LIST_IOUT_P2:
                case XDR_TEST_LIST_IOUT_P3:
                case XDR_TEST_LIST_MAX_SWEEP_P01:
                case XDR_TEST_LIST_MAX_SWEEP_P02:
                case XDR_TEST_LIST_MAX_SWEEP_P03:
                case XDR_TEST_LIST_MAX_SWEEP_P04:
                case XDR_TEST_LIST_MAX_SWEEP_P05:
                case XDR_TEST_LIST_MAX_SWEEP_P06:
                case XDR_TEST_LIST_MAX_SWEEP_P07:
                case XDR_TEST_LIST_MAX_SWEEP_P08:
                case XDR_TEST_LIST_MAX_SWEEP_P09:
                case XDR_TEST_LIST_MAX_SWEEP_P10:
                case XDR_TEST_LIST_MAX_SWEEP_P11:
                case XDR_TEST_LIST_MAX_SWEEP_P12:
                case XDR_TEST_LIST_MAX_SWEEP_P13:
                case XDR_TEST_LIST_MAX_SWEEP_P14:
                case XDR_TEST_LIST_MAX_SWEEP_P15:
                case XDR_TEST_LIST_MAX_SWEEP_P16:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_Current(*p_adc_value, GAIN_HIGH);
                    }
                    else
                    {
#ifndef SEQUENCE_DEBUG
                        return true;
#endif
                    }
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, test_step_to_string((test_step_t)td->step), *list);
                    break;
                }
            }
            if (*list < XDR_TEST_LIST_MAX)
            {
                td->step = TEST_STEP_INITIAL_BY_LIST;
                if ((*list < XDR_TEST_LIST_IOUT_P1) || (info->chx >= (XD_CH_MAX - 1U)))
                {
                    ++*list;
                }
                else
                {
                    ++info->chx;
                }

                if (*list >= XDR_TEST_LIST_MAX)
                {
                    td->step = TEST_STEP_LOG_SUMMARY;
                }
            }
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_LOG_SUMMARY:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            xdr_test_log_summary();
            td->step = TEST_STEP_PWR_OFF;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_PWR_OFF:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            gpio_set_xd_vdd_5v(VCC_OFF);
            gpio_set_vled_9v(VLED_OFF);
            td->step = TEST_STEP_NONE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            __priv_test.test_thr = INVALID_THREAD_ID;
            return false;
        }
    }
    return true;
}

static bool _xdr_sweep_test_thread(struct thread_data* td)
{
    sweep_info_t *info = &__priv_test.t_xdr_sweep_test_info;
    switch(td->step)
    {
        case TEST_STEP_PWR_ON:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            td->step = TEST_STEP_INITIAL;
            td->tout = STEP_DELAY_PWR_ON;
            break;
        }

        case TEST_STEP_INITIAL:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            xdr12_trim_init();
            xdr12_trim_init_ch_gain();

            info->chx = XD_CH_01;
            info->max_curr_level = CURR_LEVEL_4;
            info->max_curr_level_target = CURR_LEVEL_64;
            info->vref = 0U;
            info->vref_gap = XDR_SWEEP_VREF_GAP;

            gpio_set_vled_9v(VLED_ON);
            ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);
            gpio_set_current_gain(GAIN_HIGH);

            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            if (info->vref == 0U)
            {
                xdr12_trim_set_max_curr_lvl(info->max_curr_level);
            }
            xdr12_trim_set_max_curr_vref(info->vref);
            gpio_set_demux_channel_selection(info->chx);
            xdr12_trim_set_channel_enable(info->chx);
            td->step = TEST_STEP_START_MEASURE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            ADS114S08_Set_Start(true);
            td->step = TEST_STEP_GET_MEASURED_VALUE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            if (true == ADS114S08_Wait_Done())
            {
                info->measure[info->chx].adc = ADS114S08_Get_ADC_Value();
                info->measure[info->chx].value = JigBD_IF_Convert_Adc_To_Current(info->measure[info->chx].adc, GAIN_HIGH);

                if (++info->chx < XD_CH_MAX)
                {
                    td->step = TEST_STEP_INITIAL_BY_LIST;
                }
                else
                {
                    comm_UART_Printf(LOG_LV_INFO, "\r\n%u, %4u, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f",
                        ((info->max_curr_level + 1) * 4), info->vref,
                        (double)(info->measure[XD_CH_01].value), (double)(info->measure[XD_CH_02].value), (double)(info->measure[XD_CH_03].value),
                        (double)(info->measure[XD_CH_04].value), (double)(info->measure[XD_CH_05].value), (double)(info->measure[XD_CH_06].value),
                        (double)(info->measure[XD_CH_07].value), (double)(info->measure[XD_CH_08].value), (double)(info->measure[XD_CH_09].value),
                        (double)(info->measure[XD_CH_10].value), (double)(info->measure[XD_CH_11].value), (double)(info->measure[XD_CH_12].value));
                    info->chx = XD_CH_01;
                    if (info->vref == 4095U)
                    {
                        if (info->max_curr_level >= info->max_curr_level_target)
                        {
                            td->step = TEST_STEP_LOG_SUMMARY;
                        }
                        else
                        {
                            ++info->max_curr_level;
                            info->vref = 0U;
                            td->step = TEST_STEP_INITIAL_BY_LIST;
                        }
                    }
                    else
                    {
                        info->vref += info->vref_gap;
                        if (info->vref > 4095U)
                        {
                            info->vref = 4095U;
                        }
                        td->step = TEST_STEP_INITIAL_BY_LIST;
                    }
                }
            }
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_LOG_SUMMARY:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            td->step = TEST_STEP_PWR_OFF;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_PWR_OFF:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            td->step = TEST_STEP_NONE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            __priv_test.test_thr = INVALID_THREAD_ID;
            return false;
        }
    }
    return true;
}

static void _power(bool on)
{
    if(true == on)
    {
        __priv_test.status = STATUS_NORMAL;
    }
    else
    {
        if(__priv_test.test_thr != INVALID_THREAD_ID)
        {
            fw_thread_stop(__priv_test.test_thr);
            __priv_test.test_thr = INVALID_THREAD_ID;
        }

        __priv_test.status = STATUS_UNPOWER;
    }
}

static void _enable(bool en)
{

}

static MGRSTATUS _status(void)
{
    return __priv_test.status;
}

static uint32_t _cmd(uint32_t cmd, void* val)
{
    switch(cmd)
    {
        case TEST_CMD_XCR_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xcr_test_thread, 10U);    /* 10ms */
            }
            break;
        }
        case TEST_CMD_XDR_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xdr_test_thread, 10U);    /* 10ms */
            }
            break;
        }
        case TEST_CMD_XDR_SWEEP_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xdr_sweep_test_thread, 10U);    /* 10ms */
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

struct manager __test_mgr=
{
    _power,
    _enable,
    _status,
    _cmd,
    _write,
    _read,
    _noti
};