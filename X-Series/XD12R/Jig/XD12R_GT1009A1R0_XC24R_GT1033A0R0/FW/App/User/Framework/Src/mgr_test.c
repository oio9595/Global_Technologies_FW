#include <string.h>
#include <stdio.h>

#include "framework.h"
#include "drv_xd12.h"
#include "drv_xc24.h"
#include "drv_timer.h"
#include "drv_ads124s08.h"
#include "comm_debugging.h"

#define TEST_LOG_PARTIAL        (0U)
#define TEST_LOG_ALL            (1U)
#define TEST_LOG_TYPE           (TEST_LOG_ALL)

#define STEP_DELAY_DEFAULT      (1U)
#define STEP_DELAY_SETTLING     (10U)
#define STEP_DELAY_VSYNC_STOP   (200U)
#define STEP_DELAY_MEASURE      (10U)
#define STEP_DELAY_VSYNC        (100U)
#define STEP_DELAY_PWR_ON       (100U)
#define STEP_DELAY_REPEAT       (222U)

#define XC_SWEEP_VREF_GAP      (10U)
#define XD_SWEEP_VREF_GAP      (10U)

typedef struct tag_MEASURE_INFO
{
    float value;
    uint16_t adc;
} measure_info_t;

typedef struct tag_TEST_INFO
{
    measure_info_t measure[XD_CH_MAX];
    current_gain_t gain;
    uint8_t chx;
} test_info_t;

typedef struct tag_TEST_SWEEP_INFO
{
    measure_info_t measure[XD_CH_MAX];
    current_gain_t gain;
    uint16_t vref;
    uint16_t vref_gap;
    uint8_t chx;
    uint8_t max_curr_level;
    uint8_t max_curr_level_target;
} sweep_info_t;

typedef struct tag_TEST_DAC_SWEEP_INFO
{
    measure_info_t measure[XC_DAC_CH_MAX];
    uint16_t tgt_dac;
    uint16_t tgt_dac_gap;
    uint8_t chx;
} dac_sweep_info_t;

typedef enum tag_TEST_STEP_T
{
    TEST_STEP_PWR_ON = 0U,
    TEST_STEP_INITIAL,
    TEST_STEP_INITIAL_BY_LIST,
    TEST_STEP_VSYNC_STOP,
    TEST_STEP_START_MEASURE,
    TEST_STEP_GET_MEASURED_VALUE,
    TEST_STEP_LOG_SUMMARY,
    TEST_STEP_PWR_OFF,
    TEST_STEP_NONE,
    TEST_STEP_MAX,
} test_step_t;

typedef enum tag_XC_TEST_LIST
{
    XC_TEST_LIST_ICC_STBY = 0U,
    XC_TEST_LIST_ICC_ACTV,
    XC_TEST_LIST_LDO,
    XC_TEST_LIST_LDO_FLL_A,
    XC_TEST_LIST_LDO_FLL_B,
    XC_TEST_LIST_FLL_A_30M,
    XC_TEST_LIST_FLL_A_35M,
    XC_TEST_LIST_FLL_A_40M,
    XC_TEST_LIST_FLL_B_30M,
    XC_TEST_LIST_FLL_B_35M,
    XC_TEST_LIST_FLL_B_40M,
    XC_TEST_LIST_DAC_P1,
    XC_TEST_LIST_DAC_P2,
    XC_TEST_LIST_DAC_P3,
    XC_TEST_LIST_MAX,
} xc_test_list_t;

typedef enum tag_XD_TEST_LIST
{
    XD_TEST_LIST_ICC_STBY = 0U,
    XD_TEST_LIST_ICC_ACTV,
    XD_TEST_LIST_CURRENT_REF,
    XD_TEST_LIST_LDO_DIG,
    XD_TEST_LIST_LDO_DAC,
    XD_TEST_LIST_LDO_FLL,
    XD_TEST_LIST_OSC,
    XD_TEST_LIST_FLL_40M,
    XD_TEST_LIST_FLL_50M,
    XD_TEST_LIST_FLL_60M,
    XD_TEST_LIST_IOUT_P1,
    XD_TEST_LIST_IOUT_P2,
    XD_TEST_LIST_IOUT_P3,
    XD_TEST_LIST_MAX_SWEEP_P01,
    XD_TEST_LIST_MAX_SWEEP_P02,
    XD_TEST_LIST_MAX_SWEEP_P03,
    XD_TEST_LIST_MAX_SWEEP_P04,
    XD_TEST_LIST_MAX_SWEEP_P05,
    XD_TEST_LIST_MAX_SWEEP_P06,
    XD_TEST_LIST_MAX_SWEEP_P07,
    XD_TEST_LIST_MAX_SWEEP_P08,
    XD_TEST_LIST_MAX_SWEEP_P09,
    XD_TEST_LIST_MAX_SWEEP_P10,
    XD_TEST_LIST_MAX_SWEEP_P11,
    XD_TEST_LIST_MAX_SWEEP_P12,
    XD_TEST_LIST_MAX_SWEEP_P13,
    XD_TEST_LIST_MAX_SWEEP_P14,
    XD_TEST_LIST_MAX_SWEEP_P15,
    XD_TEST_LIST_MAX_SWEEP_P16,
    XD_TEST_LIST_MAX,
} xd_test_list_t;

typedef enum tag_XC_AGING_LIST
{
    XC_AGING_LIST_ICC_STBY = 0U,
    XC_AGING_LIST_MAX,
} xc_aging_list_t;

typedef enum tag_XD_AGING_LIST
{
    XD_AGING_LIST_ICC_TEST = 0U,
    XD_AGING_LIST_CURRENT_REF,
    XD_AGING_LIST_LDO_DIG,
    XD_AGING_LIST_LDO_DAC,
    XD_AGING_LIST_LDO_FLL,
    XD_AGING_LIST_OSC,
    XD_AGING_LIST_IOUT,
    XD_AGING_LIST_MAX,
} xd_aging_list_t;

typedef void (*xc24_test_init_func)(void);
typedef void (*xc24_test_start_func)(void);

typedef void (*xd12_test_init_func)(void);
typedef void (*xd12_test_start_func)(void);

typedef void (*xc24_aging_init_func)(void);
typedef void (*xc24_aging_start_func)(void);

typedef void (*xd12_aging_init_func)(void);
typedef void (*xd12_aging_start_func)(void);

static struct{
    test_info_t         t_xc_test_info[XC_TEST_LIST_MAX];
    test_info_t         t_xd_test_info[XD_TEST_LIST_MAX];
    sweep_info_t        t_xd_sweep_test_info;
    dac_sweep_info_t    t_xc_dac_sweep_test_info;
    MGRSTATUS           status;
    xc_test_list_t     t_xc_test_list;
    xd_test_list_t     t_xd_test_list;

    xd_aging_list_t    t_xd_aging_list;
    THREAD_ID           test_thr;
}__priv_test; // declare & define private variable for test manager

static const char* gs_xc_test_list[XC_TEST_LIST_MAX] =
{
    "XC_ICC_STBY ",
    "XC_ICC_ACTV ",
    "XC_LDO      ",
    "XC_LDO_FLL_A",
    "XC_LDO_FLL_B",
    "XC_FLL_A_30M",
    "XC_FLL_A_35M",
    "XC_FLL_A_40M",
    "XC_FLL_B_30M",
    "XC_FLL_B_35M",
    "XC_FLL_B_40M",
    "XC_DAC_P1   ",
    "XC_DAC_P2   ",
    "XC_DAC_P3   ",
};

static const xc24_test_init_func gp_xc24_test_init_func[XC_TEST_LIST_MAX] =
{
    [XC_TEST_LIST_ICC_STBY] = xc24_test_init_icc_stby,
    [XC_TEST_LIST_ICC_ACTV] = xc24_test_init_icc_actv,
    [XC_TEST_LIST_LDO] = xc24_test_init_ldo,
    [XC_TEST_LIST_LDO_FLL_A] = xc24_test_init_ldo_fll_a,
    [XC_TEST_LIST_LDO_FLL_B] = xc24_test_init_ldo_fll_b,
    [XC_TEST_LIST_FLL_A_30M] = xc24_test_init_fll_a_30m,
    [XC_TEST_LIST_FLL_A_35M] = xc24_test_init_fll_a_35m,
    [XC_TEST_LIST_FLL_A_40M] = xc24_test_init_fll_a_40m,
    [XC_TEST_LIST_FLL_B_30M] = xc24_test_init_fll_b_30m,
    [XC_TEST_LIST_FLL_B_35M] = xc24_test_init_fll_b_35m,
    [XC_TEST_LIST_FLL_B_40M] = xc24_test_init_fll_b_40m,
    [XC_TEST_LIST_DAC_P1] = xc24_test_init_dac_p1,
    [XC_TEST_LIST_DAC_P2] = xc24_test_init_dac_p2,
    [XC_TEST_LIST_DAC_P3] = xc24_test_init_dac_p3,
};

static const xc24_test_start_func gp_xc24_test_start_func[XC_TEST_LIST_MAX] =
{
    [XC_TEST_LIST_ICC_STBY] = xc24_test_start_icc_stby,
    [XC_TEST_LIST_ICC_ACTV] = xc24_test_start_icc_actv,
    [XC_TEST_LIST_LDO] = xc24_test_start_ldo,
    [XC_TEST_LIST_LDO_FLL_A] = xc24_test_start_ldo_fll_a,
    [XC_TEST_LIST_LDO_FLL_B] = xc24_test_start_ldo_fll_b,
    [XC_TEST_LIST_FLL_A_30M] = xc24_test_start_fll_a_30m,
    [XC_TEST_LIST_FLL_A_35M] = xc24_test_start_fll_a_35m,
    [XC_TEST_LIST_FLL_A_40M] = xc24_test_start_fll_a_40m,
    [XC_TEST_LIST_FLL_B_30M] = xc24_test_start_fll_b_30m,
    [XC_TEST_LIST_FLL_B_35M] = xc24_test_start_fll_b_35m,
    [XC_TEST_LIST_FLL_B_40M] = xc24_test_start_fll_b_40m,
    [XC_TEST_LIST_DAC_P1] = xc24_test_start_dac_p1,
    [XC_TEST_LIST_DAC_P2] = xc24_test_start_dac_p2,
    [XC_TEST_LIST_DAC_P3] = xc24_test_start_dac_p3,
};

static const char* gs_xd_test_list[XD_TEST_LIST_MAX] =
{
    "XD_ICC_STBY     ",
    "XD_ICC_ACTV     ",
    "XD_CURRENT_REF  ",
    "XD_LDO_DIG      ",
    "XD_LDO_DAC      ",
    "XD_LDO_FLL      ",
    "XD_OSC          ",
    "XD_FLL_40M      ",
    "XD_FLL_50M      ",
    "XD_FLL_60M      ",
    "XD_IOUT_P1      ",
    "XD_IOUT_P2      ",
    "XD_IOUT_P3      ",
    "XD_MAX_SWEEP_P01",
    "XD_MAX_SWEEP_P02",
    "XD_MAX_SWEEP_P03",
    "XD_MAX_SWEEP_P04",
    "XD_MAX_SWEEP_P05",
    "XD_MAX_SWEEP_P06",
    "XD_MAX_SWEEP_P07",
    "XD_MAX_SWEEP_P08",
    "XD_MAX_SWEEP_P09",
    "XD_MAX_SWEEP_P10",
    "XD_MAX_SWEEP_P11",
    "XD_MAX_SWEEP_P12",
    "XD_MAX_SWEEP_P13",
    "XD_MAX_SWEEP_P14",
    "XD_MAX_SWEEP_P15",
    "XD_MAX_SWEEP_P16",
};

static const xd12_test_init_func gp_xd12_test_init_func[XD_TEST_LIST_MAX] =
{
    [XD_TEST_LIST_ICC_STBY] = xd12_test_init_icc_stby,
    [XD_TEST_LIST_ICC_ACTV] = xd12_test_init_icc_actv,
    [XD_TEST_LIST_CURRENT_REF] = xd12_test_init_current_ref,
    [XD_TEST_LIST_LDO_DIG] = xd12_test_init_ldo_dig,
    [XD_TEST_LIST_LDO_DAC] = xd12_test_init_ldo_dac,
    [XD_TEST_LIST_LDO_FLL] = xd12_test_init_ldo_fll,
    [XD_TEST_LIST_OSC] = xd12_test_init_osc,
    [XD_TEST_LIST_FLL_40M] = xd12_test_init_fll_40M,
    [XD_TEST_LIST_FLL_50M] = xd12_test_init_fll_50M,
    [XD_TEST_LIST_FLL_60M] = xd12_test_init_fll_60M,
    [XD_TEST_LIST_IOUT_P1] = xd12_test_init_iout_3P,
    [XD_TEST_LIST_IOUT_P2] = xd12_test_init_iout_3P,
    [XD_TEST_LIST_IOUT_P3] = xd12_test_init_iout_3P,
    [XD_TEST_LIST_MAX_SWEEP_P01] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P02] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P03] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P04] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P05] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P06] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P07] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P08] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P09] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P10] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P11] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P12] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P13] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P14] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P15] = xd12_test_init_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P16] = xd12_test_init_max_sweep,
};

static const xd12_test_start_func gp_xd12_test_start_func[XD_TEST_LIST_MAX] =
{
    [XD_TEST_LIST_ICC_STBY] = xd12_test_start_icc_stby,
    [XD_TEST_LIST_ICC_ACTV] = xd12_test_start_icc_actv,
    [XD_TEST_LIST_CURRENT_REF] = xd12_test_start_current_ref,
    [XD_TEST_LIST_LDO_DIG] = xd12_test_start_ldo_dig,
    [XD_TEST_LIST_LDO_DAC] = xd12_test_start_ldo_dac,
    [XD_TEST_LIST_LDO_FLL] = xd12_test_start_ldo_fll,
    [XD_TEST_LIST_OSC] = xd12_test_start_osc,
    [XD_TEST_LIST_FLL_40M] = xd12_test_start_fll_40M,
    [XD_TEST_LIST_FLL_50M] = xd12_test_start_fll_50M,
    [XD_TEST_LIST_FLL_60M] = xd12_test_start_fll_60M,
    [XD_TEST_LIST_IOUT_P1] = xd12_test_start_iout_3P,
    [XD_TEST_LIST_IOUT_P2] = xd12_test_start_iout_3P,
    [XD_TEST_LIST_IOUT_P3] = xd12_test_start_iout_3P,
    [XD_TEST_LIST_MAX_SWEEP_P01] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P02] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P03] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P04] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P05] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P06] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P07] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P08] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P09] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P10] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P11] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P12] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P13] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P14] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P15] = xd12_test_start_max_sweep,
    [XD_TEST_LIST_MAX_SWEEP_P16] = xd12_test_start_max_sweep,
};

static const char* gs_test_step[TEST_STEP_MAX] =
{
    "TEST_STEP_PWR_ON",
    "TEST_STEP_INITIAL",
    "TEST_STEP_INITIAL_BY_LIST",
    "TEST_STEP_VSYNC_STOP",
    "TEST_STEP_START_MEASURE",
    "TEST_STEP_GET_MEASURED_VALUE",
    "TEST_STEP_LOG_SUMMARY",
    "TEST_STEP_PWR_OFF",
    "TEST_STEP_NONE",
};

static const char* gs_xd_aging_list[XD_AGING_LIST_MAX] =
{
    "ICC_TEST",
    "CURR_REF",
    "LDO_DIG ",
    "LDO_DAC ",
    "LDO_FLL ",
    "OSC     ",
    "IOUT    ",
};

static const xd12_aging_init_func gp_xd12_aging_init_func[XD_AGING_LIST_MAX] =
{
    [XD_AGING_LIST_ICC_TEST] = xd12_aging_init_icc_test,
    [XD_AGING_LIST_CURRENT_REF] = xd12_aging_init_current_ref,
    [XD_AGING_LIST_LDO_DIG] = xd12_aging_init_ldo_dig,
    [XD_AGING_LIST_LDO_DAC] = xd12_aging_init_ldo_dac,
    [XD_AGING_LIST_LDO_FLL] = xd12_aging_init_ldo_fll,
    [XD_AGING_LIST_OSC] = xd12_aging_init_osc,
    [XD_AGING_LIST_IOUT] = xd12_aging_init_iout,
};

static const xd12_aging_start_func gp_xd12_aging_start_func[XD_AGING_LIST_MAX] =
{
    [XD_AGING_LIST_ICC_TEST] = xd12_aging_start_icc_test,
    [XD_AGING_LIST_CURRENT_REF] = xd12_aging_start_current_ref,
    [XD_AGING_LIST_LDO_DIG] = xd12_aging_start_ldo_dig,
    [XD_AGING_LIST_LDO_DAC] = xd12_aging_start_ldo_dac,
    [XD_AGING_LIST_LDO_FLL] = xd12_aging_start_ldo_fll,
    [XD_AGING_LIST_OSC] = xd12_aging_start_osc,
    [XD_AGING_LIST_IOUT] = xd12_aging_start_iout,
};

static uint16_t gn_xd_aging_max_curr_lvl;
static uint16_t gn_xd_aging_max_curr_vref;

static const char* xc_test_list_to_string(xc_test_list_t list)
{
    if (list < XC_TEST_LIST_MAX)
    {
        return gs_xc_test_list[list];
    }

    return "XC_TEST_LIST_INVALID";
}

static const char* xd_test_list_to_string(xd_test_list_t list)
{
    if (list < XD_TEST_LIST_MAX)
    {
        return gs_xd_test_list[list];
    }

    return "XD_TEST_LIST_INVALID";
}

static const char* test_step_to_string(test_step_t step)
{
    if (step < TEST_STEP_MAX)
    {
        return gs_test_step[step];
    }

    return "TEST_STEP_INVALID";
}

static const char* xd_aging_list_to_string(xd_aging_list_t list)
{
    if (list < XD_AGING_LIST_MAX)
    {
        return gs_xd_aging_list[list];
    }

    return "XD_AGING_LIST_INVALID";
}

static void xc_test_log_summary(void)
{
    char log_buf[350] = {0};
    int log_buf_len = 0U;

#if (TEST_LOG_TYPE == TEST_LOG_ALL)
    for (xc_test_list_t list = XC_TEST_LIST_ICC_STBY; list < XC_TEST_LIST_MAX; ++list)
    {
        test_info_t* info = &__priv_test.t_xc_test_info[list];
        uint8_t max_ch = (list < XC_TEST_LIST_DAC_P1) ? (uint8_t)(1U) : (uint8_t)(3U);
        for (uint8_t ch = 0U; ch < max_ch; ++ch)
        {
            if (ch == 0U)
            {
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "\r\n%s|VAL|%6.3f", xc_test_list_to_string(list), (double)(info->measure[ch].value));
            }
            else
            {
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "|%6.3f", (double)(info->measure[ch].value));
            }
        }
        comm_UART_Printf(LOG_LV_INFO, "%s", log_buf);
        memset(log_buf, 0, sizeof(log_buf));
        log_buf_len = 0U;
    }
#elif (TEST_LOG_TYPE == TEST_LOG_PARTIAL)
    for (xc_test_list_t list = XC_TEST_LIST_ICC_STBY; list < XC_TEST_LIST_MAX; ++list)
    {
        if ((list == XC_TEST_LIST_ICC_STBY) || (list == XC_TEST_LIST_FLL_B_35M) || (list == XC_TEST_LIST_DAC_P3))
        {
            test_info_t* info = &__priv_test.t_xc_test_info[list];
            uint8_t max_ch = (list < XC_TEST_LIST_DAC_P1) ? (uint8_t)(1U) : (uint8_t)(3U);
            for (uint8_t ch = 0U; ch < max_ch; ++ch)
            {
                if (ch == 0U)
                {
                    log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "\r\n%s|VAL|%6.3f", xc_test_list_to_string(list), (double)(info->measure[ch].value));
                }
                else
                {
                    log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "|%6.3f", (double)(info->measure[ch].value));
                }
            }
            comm_UART_Printf(LOG_LV_INFO, "%s", log_buf);
            memset(log_buf, 0, sizeof(log_buf));
            log_buf_len = 0U;
        }
    }
#else
        #error "TEST_LOG_TYPE is not defined"
#endif
    // test
    comm_UART_Printf(LOG_LV_INFO, "\r\n0x1B : %04X", xc24_read_grp1_reg(0x1B, 1));
}

static bool _xc_test_thread(struct thread_data* td)
{
    if (td == NULL)
    {
        return false;
    }
    xc_test_list_t* list = &__priv_test.t_xc_test_list;
    test_info_t *info = &__priv_test.t_xc_test_info[*list];
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
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xc_test_list_to_string(*list), td->tout);
            if (*list < XC_TEST_LIST_MAX)
            {
                if (*list == XC_TEST_LIST_FLL_A_30M)
                {
                    *list = XC_TEST_LIST_DAC_P1; // FLL_A_30M 이후 DAC 측정 항목으로 넘어가도록 설정
                }
                if (gp_xc24_test_init_func[*list] != NULL)
                {
                    gp_xc24_test_init_func[*list]();
                }
                else
                {
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r%s, id : %u, step : %s, timeout : %u, invalid list : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout, *list);
                    td->step = TEST_STEP_LOG_SUMMARY;
                    td->tout = STEP_DELAY_DEFAULT;
                    return false;
                }
            }
            if (*list < XC_TEST_LIST_FLL_A_30M)
            {
                td->step = TEST_STEP_START_MEASURE;
                td->tout = STEP_DELAY_SETTLING;
            }
            else
            {
                if ((*list == XC_TEST_LIST_DAC_P1) || (*list == XC_TEST_LIST_DAC_P2) || (*list == XC_TEST_LIST_DAC_P3))
                {
                    ADS114S08_Select_Input_CH((ADS114S08_CH_XC_DAC_1 + info->chx), ADS_AINCOM);
                }
                tim_vsync_out_for_test_start();
                td->step = TEST_STEP_VSYNC_STOP;
                td->tout = STEP_DELAY_VSYNC_STOP;
            }
            break;
        }

        case TEST_STEP_VSYNC_STOP:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xc_test_list_to_string(*list), td->tout);
            tim_vsync_out_for_test_stop();
            td->step = TEST_STEP_START_MEASURE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xc_test_list_to_string(*list), td->tout);
            if (*list < XC_TEST_LIST_MAX)
            {
                if (gp_xc24_test_start_func[*list] != NULL)
                {
                    gp_xc24_test_start_func[*list]();
                }
                else
                {
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r%s, id : %u, step : %s, timeout : %u, invalid list : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout, *list);
                    td->step = TEST_STEP_LOG_SUMMARY;
                    td->tout = STEP_DELAY_DEFAULT;
                    return false;
                }
            }
            td->step = TEST_STEP_GET_MEASURED_VALUE;
            td->tout = STEP_DELAY_MEASURE;
            break;
        }

        case TEST_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xc_test_list_to_string(*list), td->tout);
            uint16_t* p_adc_value = &info->measure[info->chx].adc;
            float* p_value = &info->measure[info->chx].value;
            switch (*list)
            {
                case XC_TEST_LIST_ICC_STBY:
                case XC_TEST_LIST_ICC_ACTV:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_ICC_XC(*p_adc_value);
                    }
                    break;
                }
                case XC_TEST_LIST_LDO:
                case XC_TEST_LIST_LDO_FLL_A:
                case XC_TEST_LIST_LDO_FLL_B:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_Voltage(*p_adc_value);
                    }
                    break;
                }
                case XC_TEST_LIST_FLL_A_30M:
                case XC_TEST_LIST_FLL_A_35M:
                case XC_TEST_LIST_FLL_A_40M:
                case XC_TEST_LIST_FLL_B_30M:
                case XC_TEST_LIST_FLL_B_35M:
                case XC_TEST_LIST_FLL_B_40M:
                {
                    *p_value = mcu_peripheral_tim_conversion_freq() * XC_CONST_OSC;
                    break;
                }
                case XC_TEST_LIST_DAC_P1:
                case XC_TEST_LIST_DAC_P2:
                case XC_TEST_LIST_DAC_P3:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_Voltage(*p_adc_value);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
            // 1. DAC 측정 항목 그룹인 경우 (DAC_P1, DAC_P2, DAC_P3)
            if ((*list >= XC_TEST_LIST_DAC_P1) && (*list < XC_TEST_LIST_MAX))
            {
                // 현재 DAC 항목에서 채널(chx: 0, 1, 2) 순회 중인 경우
                if (++(info->chx) < 3U)
                {
                    // list는 유지하고, 다음 채널 측정을 위해 START_MEASURE로 이동
                    td->step = TEST_STEP_INITIAL_BY_LIST;
                }
                else
                {
                    // 3개 채널(0,1,2) 측정이 모두 끝나면 chx 초기화 후 다음 list로 이동
                    info->chx = 0U;
                    ++(*list);

                    if (*list < XC_TEST_LIST_MAX)
                    {
                        td->step = TEST_STEP_INITIAL_BY_LIST;
                    }
                    else
                    {
                        td->step = TEST_STEP_LOG_SUMMARY;
                    }
                }
            }
            // 2. 일반 측정 항목인 경우 (ICC_STBY ~ FLL_B_40M)
            else if (*list < XC_TEST_LIST_DAC_P1)
            {
                ++(*list);

                if (*list < XC_TEST_LIST_MAX)
                {
                    td->step = TEST_STEP_INITIAL_BY_LIST;
                }
                else
                {
                    td->step = TEST_STEP_LOG_SUMMARY;
                }
            }
            // 3. 예외 상황 (이미 MAX 이상인 경우)
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
            xc_test_log_summary();
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

static bool _xc_sweep_test_thread(struct thread_data* td)
{
    if (td == NULL)
    {
        return false;
    }
    dac_sweep_info_t *info = &__priv_test.t_xc_dac_sweep_test_info;
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
            xc24_trim_init();
            xc24_trim_init_dac1_ofs();
            xc24_read_all();

            info->chx = XC_DAC_CH_01;
            info->tgt_dac = 0U;
            info->tgt_dac_gap = XC_SWEEP_VREF_GAP;

            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            ADS114S08_Select_Input_CH((ADS114S08_CH_XC_DAC_1 + info->chx), ADS_AINCOM);
            xc24_test_set_curr_tgt_dac(info->tgt_dac);
            tim_vsync_out_for_test_start();
            td->step = TEST_STEP_VSYNC_STOP;
            td->tout = STEP_DELAY_VSYNC_STOP;
            break;
        }

        case TEST_STEP_VSYNC_STOP:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            tim_vsync_out_for_test_stop();
            td->step = TEST_STEP_START_MEASURE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            ADS114S08_Set_Start(true);
            td->step = TEST_STEP_GET_MEASURED_VALUE;
            td->tout = STEP_DELAY_MEASURE;
            break;
        }

        case TEST_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            if (true == ADS114S08_Wait_Done())
            {
                info->measure[info->chx].adc = ADS114S08_Get_ADC_Value();
                info->measure[info->chx].value = JigBD_IF_Convert_Adc_To_Voltage(info->measure[info->chx].adc);

                if (++(info->chx) < XC_DAC_CH_MAX)
                {
                    td->step = TEST_STEP_INITIAL_BY_LIST;
                }
                else
                {
                    comm_UART_Printf(LOG_LV_INFO, "\r\n%4u, %6.3f, %6.3f, %6.3f",
                        info->tgt_dac, (double)(info->measure[XC_DAC_CH_01].value), (double)(info->measure[XC_DAC_CH_02].value), (double)(info->measure[XC_DAC_CH_03].value));
                    info->chx = XC_DAC_CH_01;
                    if (info->tgt_dac == 4095U)
                    {
                        /*
                        if (info->max_curr_level >= info->max_curr_level_target)
                        {
                            td->step = TEST_STEP_LOG_SUMMARY;
                        }
                        else
                        {
                            ++info->max_curr_level;
                            info->tgt_dac = 0U;
                            td->step = TEST_STEP_INITIAL_BY_LIST;
                        }
                        */
                        td->step = TEST_STEP_LOG_SUMMARY;
                    }
                    else
                    {
                        info->tgt_dac += info->tgt_dac_gap;
                        if (info->tgt_dac > 4095U)
                        {
                            info->tgt_dac = 4095U;
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
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<POWER OFF>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
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

static void xd_test_log_summary(void)
{
    char log_buf[350] = {0};
    int log_buf_len = 0U;
#if (TEST_LOG_TYPE == TEST_LOG_ALL)
    for (xd_test_list_t list = XD_TEST_LIST_ICC_STBY; list < XD_TEST_LIST_MAX; ++list)
    {
        test_info_t* info = &__priv_test.t_xd_test_info[list];
        uint8_t max_ch = (list < XD_TEST_LIST_IOUT_P1) ? (uint8_t)(XD_CH_01 + 1U) : (uint8_t)XD_CH_MAX;
        for (uint8_t ch = XD_CH_01; ch < max_ch; ++ch)
        {
            if (ch == XD_CH_01)
            {
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "\r\n%s|VAL|%6.3f", xd_test_list_to_string(list), (double)(info->measure[ch].value));
            }
            else
            {
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "|%6.3f", (double)(info->measure[ch].value));
            }
        }
        comm_UART_Printf(LOG_LV_INFO, "%s", log_buf);
        memset(log_buf, 0, sizeof(log_buf));
        log_buf_len = 0U;
    }
#elif (TEST_LOG_TYPE == TEST_LOG_PARTIAL)
    for (xd_test_list_t list = XD_TEST_LIST_ICC_STBY; list < XD_TEST_LIST_MAX; ++list)
    {
        if ((list == XD_TEST_LIST_ICC_STBY) || (list == XD_TEST_LIST_FLL_50M) || (list == XD_TEST_LIST_IOUT_P1))
        {
            test_info_t* info = &__priv_test.t_xd_test_info[list];
            uint8_t max_ch = (list < XD_TEST_LIST_IOUT_P1) ? (uint8_t)(XD_CH_01 + 1U) : (uint8_t)XD_CH_MAX;
            for (uint8_t ch = XD_CH_01; ch < max_ch; ++ch)
            {
                if (ch == XD_CH_01)
                {
                    log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "\r\n%s|VAL|%6.3f", xd_test_list_to_string(list), (double)(info->measure[ch].value));
                }
                else
                {
                    log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "|%6.3f", (double)(info->measure[ch].value));
                }
            }
            comm_UART_Printf(LOG_LV_INFO, "%s", log_buf);
            memset(log_buf, 0, sizeof(log_buf));
            log_buf_len = 0U;
        }
    }
#else
    #error "TEST_LOG_TYPE is not defined"
#endif
}

static bool _xd_test_thread(struct thread_data* td)
{
    if (td == NULL)
    {
        return false;
    }
    xd_test_list_t* list = &__priv_test.t_xd_test_list;
    test_info_t *info = &__priv_test.t_xd_test_info[*list];
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
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xd_test_list_to_string(*list), td->tout);
            if (*list < XD_TEST_LIST_MAX)
            {
                if (gp_xd12_test_init_func[*list] != NULL)
                {
                    gp_xd12_test_init_func[*list]();
                }
                else
                {
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r%s, id : %u, step : %s, timeout : %u, invalid list : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout, *list);
                    td->step = TEST_STEP_LOG_SUMMARY;
                    td->tout = STEP_DELAY_DEFAULT;
                    return false;
                }
                info->gain = GAIN_MID;
                if ((*list == XD_TEST_LIST_FLL_40M) || (*list == XD_TEST_LIST_FLL_50M) || (*list == XD_TEST_LIST_FLL_60M))
                {
                    tim_vsync_out_for_test_start();
                    td->step = TEST_STEP_VSYNC_STOP;
                    td->tout = STEP_DELAY_VSYNC_STOP;
                }
                else
                {
                    if (*list >= XD_TEST_LIST_IOUT_P1)
                    {
                        gpio_set_current_gain(info->gain);
                        xd12_trim_set_channel_enable(info->chx);
                        gpio_set_demux_channel_selection((XD_CH_t)info->chx);
                    }
                    td->step = TEST_STEP_START_MEASURE;
                    td->tout = STEP_DELAY_SETTLING;
                }
            }
            else
            {
                comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, test_step_to_string((test_step_t)td->step), *list);
            }
            break;
        }

        case TEST_STEP_VSYNC_STOP:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xd_test_list_to_string(*list), td->tout);
            tim_vsync_out_for_test_stop();
            td->step = TEST_STEP_START_MEASURE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xd_test_list_to_string(*list), td->tout);
            if (*list < XD_TEST_LIST_MAX)
            {
                if (*list == XD_TEST_LIST_IOUT_P1 || *list == XD_TEST_LIST_IOUT_P2 || *list == XD_TEST_LIST_IOUT_P3)
                {
                    const uint16_t iout_P3_vref_table[3] = { 300U, 700U, 3000U };
                    xd12_trim_set_max_curr_vref(iout_P3_vref_table[*list - XD_TEST_LIST_IOUT_P1]);
                }
                else if (*list >= XD_TEST_LIST_MAX_SWEEP_P01 && *list <= XD_TEST_LIST_MAX_SWEEP_P16)
                {
                    const uint16_t max_sweep_curr_lvl_table[16] =
                    {
                         CURR_LEVEL_4,  CURR_LEVEL_8, CURR_LEVEL_12, CURR_LEVEL_16,
                        CURR_LEVEL_20, CURR_LEVEL_24, CURR_LEVEL_28, CURR_LEVEL_32,
                        CURR_LEVEL_36, CURR_LEVEL_40, CURR_LEVEL_44, CURR_LEVEL_48,
                        CURR_LEVEL_52, CURR_LEVEL_56, CURR_LEVEL_60, CURR_LEVEL_64,
                    };
                    xd12_trim_set_max_curr_lvl(max_sweep_curr_lvl_table[*list - XD_TEST_LIST_MAX_SWEEP_P01]);
                }
                if (gp_xd12_test_start_func[*list] != NULL)
                {
                    gp_xd12_test_start_func[*list]();
                }
                else
                {
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r%s, id : %u, step : %s, timeout : %u, invalid list : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout, *list);
                    td->step = TEST_STEP_LOG_SUMMARY;
                    td->tout = STEP_DELAY_DEFAULT;
                    return false;
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
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xd_test_list_to_string(*list), td->tout);
            uint16_t* p_adc_value = &info->measure[info->chx].adc;
            float* p_value = &info->measure[info->chx].value;
            switch (*list)
            {
                case XD_TEST_LIST_ICC_STBY:
                case XD_TEST_LIST_ICC_ACTV:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_ICC_XD(*p_adc_value);
                    }
                    else
                    {
                        return true;
                    }
                    break;
                }
                case XD_TEST_LIST_CURRENT_REF:
                case XD_TEST_LIST_LDO_DIG:
                case XD_TEST_LIST_LDO_DAC:
                case XD_TEST_LIST_LDO_FLL:
                {
                    *p_adc_value = mcu_peripheral_adc_get();
                    *p_value = mcu_peripheral_adc_conversion_to_voltage(*p_adc_value);
                    break;
                }
                case XD_TEST_LIST_OSC:
                case XD_TEST_LIST_FLL_40M:
                case XD_TEST_LIST_FLL_50M:
                case XD_TEST_LIST_FLL_60M:
                {
                    *p_value = mcu_peripheral_tim_conversion_freq() * XD_CONST_OSC;
                    break;
                }
                case XD_TEST_LIST_IOUT_P1:
                case XD_TEST_LIST_IOUT_P2:
                case XD_TEST_LIST_IOUT_P3:
                case XD_TEST_LIST_MAX_SWEEP_P01:
                case XD_TEST_LIST_MAX_SWEEP_P02:
                case XD_TEST_LIST_MAX_SWEEP_P03:
                case XD_TEST_LIST_MAX_SWEEP_P04:
                case XD_TEST_LIST_MAX_SWEEP_P05:
                case XD_TEST_LIST_MAX_SWEEP_P06:
                case XD_TEST_LIST_MAX_SWEEP_P07:
                case XD_TEST_LIST_MAX_SWEEP_P08:
                case XD_TEST_LIST_MAX_SWEEP_P09:
                case XD_TEST_LIST_MAX_SWEEP_P10:
                case XD_TEST_LIST_MAX_SWEEP_P11:
                case XD_TEST_LIST_MAX_SWEEP_P12:
                case XD_TEST_LIST_MAX_SWEEP_P13:
                case XD_TEST_LIST_MAX_SWEEP_P14:
                case XD_TEST_LIST_MAX_SWEEP_P15:
                case XD_TEST_LIST_MAX_SWEEP_P16:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_Current(*p_adc_value, info->gain);
                    }
                    else
                    {
                        return true;
                    }
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, test_step_to_string((test_step_t)td->step), *list);
                    break;
                }
            }
            if (*list < XD_TEST_LIST_MAX)
            {
                td->step = TEST_STEP_INITIAL_BY_LIST;
                if ((*list < XD_TEST_LIST_IOUT_P1) || (info->chx >= (XD_CH_MAX - 1U)))
                {
                    ++(*list);
                }
                else
                {
                    ++(info->chx);
                }

                if (*list >= XD_TEST_LIST_MAX)
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
            xd_test_log_summary();
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

static bool _xd_sweep_test_thread(struct thread_data* td)
{
    if (td == NULL)
    {
        return false;
    }
    sweep_info_t *info = &__priv_test.t_xd_sweep_test_info;
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
            xd12_trim_init();
            xd12_trim_init_ch_gain();
            xd12_read_all();

            info->chx = XD_CH_01;
            info->max_curr_level = CURR_LEVEL_8; // start from 8mA
            info->max_curr_level_target = CURR_LEVEL_32; // end at 32mA
            info->vref = 0U;
            info->vref_gap = XD_SWEEP_VREF_GAP;

            gpio_set_vled_9v(VLED_ON);
            ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);

            info->gain = GAIN_HIGH;
            gpio_set_current_gain(info->gain);

            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            if (info->vref == 0U)
            {
                xd12_trim_set_max_curr_lvl(info->max_curr_level);
            }
            xd12_trim_set_max_curr_vref(info->vref);
            gpio_set_demux_channel_selection((XD_CH_t)info->chx);
            xd12_trim_set_channel_enable(info->chx);
            td->step = TEST_STEP_START_MEASURE;
            td->tout = STEP_DELAY_SETTLING;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            ADS114S08_Set_Start(true);
            td->step = TEST_STEP_GET_MEASURED_VALUE;
            td->tout = STEP_DELAY_MEASURE;
            break;
        }

        case TEST_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout);
            if (true == ADS114S08_Wait_Done())
            {
                info->measure[info->chx].adc = ADS114S08_Get_ADC_Value();
                info->measure[info->chx].value = JigBD_IF_Convert_Adc_To_Current(info->measure[info->chx].adc, info->gain);

                if (++(info->chx) < XD_CH_MAX)
                {
                    td->step = TEST_STEP_INITIAL_BY_LIST;
                }
                else
                {
                    comm_UART_Printf(LOG_LV_INFO, "\r\n%2u, %4u, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f, %06.3f",
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
                            if (info->max_curr_level < info->max_curr_level_target)
                            {
                                //++info->max_curr_level;
                                if (info->max_curr_level == CURR_LEVEL_8)
                                {
                                    info->max_curr_level = CURR_LEVEL_16;
                                }
                                else
                                {
                                    info->max_curr_level = CURR_LEVEL_32;
                                }
                            }
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
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s[<<<POWER OFF>>>]%s", ANSI_FONT_CYAN, ANSI_FONT_NONE);
            gpio_set_xd_vdd_5v(VCC_OFF);
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

static void xd_aging_log_summary(void)
{
#if (TEST_LOG_TYPE == TEST_LOG_ALL)
    static bool log_first_done = false;
    char log_buf[350] = {0};
    int log_buf_len = 0U;
    if (log_first_done == false)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\ninput_max_curr_lvl, %u", (gn_xd_aging_max_curr_lvl + 1U) * 4U);
        comm_UART_Printf(LOG_LV_INFO, "\r\ninput_max_curr_vref, %u", gn_xd_aging_max_curr_vref);

        for (xd_aging_list_t list = XD_AGING_LIST_ICC_TEST; list < XD_AGING_LIST_MAX; ++list)
        {
            if (list == XD_AGING_LIST_ICC_TEST)
            {
                comm_UART_Printf(LOG_LV_INFO, "\r\n%s,", xd_aging_list_to_string(list));
            }
            else
            {
                comm_UART_Printf(LOG_LV_INFO, "%s,", xd_aging_list_to_string(list));
            }
        }
    }
    for (xd_aging_list_t list = XD_AGING_LIST_ICC_TEST; list < XD_AGING_LIST_MAX; ++list)
    {
        test_info_t* info = &__priv_test.t_xd_test_info[list];
        //uint8_t max_ch = (list < XD_AGING_LIST_IOUT) ? (uint8_t)(XD_CH_01 + 1U) : (uint8_t)XD_CH_MAX; // 1 ~ 12ch
        uint8_t max_ch = (uint8_t)(XD_CH_01 + 1U); // only 1 ch
        for (uint8_t ch = XD_CH_01; ch < max_ch; ++ch)
        {
            if (list == XD_AGING_LIST_ICC_TEST)
            {
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "\r\n%6.3f,", (double)(info->measure[ch].value));
            }
            else
            {
                log_buf_len += snprintf(log_buf + log_buf_len, sizeof(log_buf) - log_buf_len, "%6.3f,", (double)(info->measure[ch].value));
            }
        }
        comm_UART_Printf(LOG_LV_INFO, "%s", log_buf);
        memset(log_buf, 0, sizeof(log_buf));
        log_buf_len = 0U;
    }
    log_first_done = true;
#elif (TEST_LOG_TYPE == TEST_LOG_PARTIAL)
#else
    #error "TEST_LOG_TYPE is not defined"
#endif
}

static bool _xd_aging_thread(struct thread_data* td)
{
    if (td == NULL)
    {
        return false;
    }
    xd_aging_list_t* list = &__priv_test.t_xd_aging_list;
    test_info_t *info = &__priv_test.t_xd_test_info[*list];
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
            xd12_trim_init();
            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xd_aging_list_to_string(*list), td->tout);
            if (*list < XD_AGING_LIST_MAX)
            {
                if (gp_xd12_aging_init_func[*list] != NULL)
                {
                    gp_xd12_aging_init_func[*list]();
                }
                else
                {
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r%s, id : %u, step : %s, timeout : %u, invalid list : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout, *list);
                    td->step = TEST_STEP_LOG_SUMMARY;
                    td->tout = STEP_DELAY_DEFAULT;
                    return false;
                }
                info->gain = GAIN_HIGH;
                if ((*list == XD_AGING_LIST_OSC))
                {
                    tim_vsync_out_for_test_start();
                    td->step = TEST_STEP_VSYNC_STOP;
                    td->tout = STEP_DELAY_VSYNC_STOP;
                }
                else
                {
                    if (*list >= XD_AGING_LIST_IOUT)
                    {
                        gpio_set_current_gain(info->gain);
                        xd12_trim_set_channel_enable(info->chx);
                        gpio_set_demux_channel_selection((XD_CH_t)info->chx);
                    }
                    td->step = TEST_STEP_START_MEASURE;
                    td->tout = STEP_DELAY_SETTLING;
                }
            }
            else
            {
                comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, test_step_to_string((test_step_t)td->step), *list);
            }
            break;
        }

        case TEST_STEP_VSYNC_STOP:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xd_aging_list_to_string(*list), td->tout);
            tim_vsync_out_for_test_stop();
            td->step = TEST_STEP_START_MEASURE;
            td->tout = STEP_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xd_aging_list_to_string(*list), td->tout);
            if (*list < XD_AGING_LIST_MAX)
            {
                xd12_trim_set_max_curr_vref(gn_xd_aging_max_curr_vref);
                xd12_trim_set_max_curr_lvl(gn_xd_aging_max_curr_lvl);

                if (gp_xd12_aging_start_func[*list] != NULL)
                {
                    gp_xd12_aging_start_func[*list]();
                }
                else
                {
                    comm_UART_Printf(LOG_LV_ERROR, "\n\r%s, id : %u, step : %s, timeout : %u, invalid list : %u", __func__, td->id, test_step_to_string((test_step_t)td->step), td->tout, *list);
                    td->step = TEST_STEP_LOG_SUMMARY;
                    td->tout = STEP_DELAY_DEFAULT;
                    return false;
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
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", test_step_to_string((test_step_t)td->step), xd_aging_list_to_string(*list), td->tout);
            uint16_t* p_adc_value = &info->measure[info->chx].adc;
            float* p_value = &info->measure[info->chx].value;
            switch (*list)
            {
                case XD_AGING_LIST_ICC_TEST:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_ICC_XD(*p_adc_value);
                    }
                    else
                    {
                        return true;
                    }
                    break;
                }
                case XD_AGING_LIST_CURRENT_REF:
                case XD_AGING_LIST_LDO_DIG:
                case XD_AGING_LIST_LDO_DAC:
                case XD_AGING_LIST_LDO_FLL:
                {
                    *p_adc_value = mcu_peripheral_adc_get();
                    *p_value = mcu_peripheral_adc_conversion_to_voltage(*p_adc_value);
                    break;
                }
                case XD_AGING_LIST_OSC:
                {
                    *p_value = mcu_peripheral_tim_conversion_freq() * XD_CONST_OSC;
                    break;
                }
                case XD_AGING_LIST_IOUT:
                {
                    if (true == ADS114S08_Wait_Done())
                    {
                        *p_adc_value = ADS114S08_Get_ADC_Value();
                        *p_value = JigBD_IF_Convert_Adc_To_Current(*p_adc_value, info->gain);
                    }
                    else
                    {
                        return true;
                    }
                    break;
                }
                default:
                {
                    comm_UART_Printf(LOG_LV_FATAL, "\n\r%s, %s invalid in_trim_list (%u)", __func__, test_step_to_string((test_step_t)td->step), *list);
                    break;
                }
            }
            if (*list < XD_AGING_LIST_MAX)
            {
                td->step = TEST_STEP_INITIAL_BY_LIST;
                //if ((*list < XD_AGING_LIST_IOUT) || (info->chx >= (XD_CH_MAX - 1U))) // 1 ~ 12ch
                if ((*list < XD_AGING_LIST_IOUT) || (info->chx >= (XD_CH_02))) // only 1 ch
                {
                    ++(*list);
                }
                else
                {
                    ++(info->chx);
                }

                if (*list >= XD_AGING_LIST_MAX)
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
            xd_aging_log_summary();
            // initial everything for next aging test
            xd12_trim_set_max_curr_vref(0U);
            xd12_trim_set_max_curr_lvl(0U);
            __priv_test.t_xd_test_info[XD_AGING_LIST_IOUT].chx = XD_CH_01;
            *list = XD_AGING_LIST_ICC_TEST;
            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = STEP_DELAY_REPEAT;
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
        case TEST_CMD_XC_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xc_test_thread, 10U);    /* 10ms */
            }
            break;
        }
        case TEST_CMD_XD_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xd_test_thread, 10U);    /* 10ms */
            }
            break;
        }
        case TEST_CMD_XC_SWEEP_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xc_sweep_test_thread, 10U);    /* 10ms */
            }
            break;
        }
        case TEST_CMD_XD_SWEEP_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xd_sweep_test_thread, 10U);    /* 10ms */
            }
            break;
        }
    #if 0
        case TEST_CMD_XC_AGING_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xc_aging_thread, 10U);    /* 10ms */
            }
            break;
        }
#endif
        case TEST_CMD_XD_AGING_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xd_aging_thread, 10U);    /* 10ms */
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
    if (addr == 0U)
    {
        gn_xd_aging_max_curr_lvl = *((uint16_t*)val);
    }
    else if (addr == 1U)
    {
        gn_xd_aging_max_curr_vref = *((uint16_t*)val);
    }
    else
    {
        FATAL_INVALID_INPUT(addr);
    }
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