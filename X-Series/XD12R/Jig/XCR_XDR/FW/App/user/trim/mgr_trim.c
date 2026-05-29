
#include "main.h"
#include "framework.h"

#define XDR_TRIM_GAIN_INPUT_1    (0U)
#define XDR_TRIM_GAIN_INPUT_2    (0U)

#define XDR_TRIM_OFS_INPUT_1    (0U)
#define XDR_TRIM_OFS_INPUT_2    (0U)

#define XDR_DELAY_DEFAULT    (1U)
#define XDR_DELAY_MEASURE    (100U)
#define XDR_DELAY_PWR_ON     (100U)

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
    XDR_TRIM_STEP_EXAMPLE01,
    XDR_TRIM_STEP_EXAMPLE02,
    XDR_TRIM_STEP_EXAMPLE03,
    XDR_TRIM_STEP_EXAMPLE04,
    XDR_TRIM_STEP_EXAMPLE05,
    XDR_TRIM_STEP_EXAMPLE06,
    XDR_TRIM_STEP_EXAMPLE07,
    XDR_TRIM_STEP_EXAMPLE08,
    XDR_TRIM_STEP_EXAMPLE09,
    XDR_TRIM_STEP_EXAMPLE10,
    XDR_TRIM_STEP_EXAMPLE11,
    XDR_TRIM_STEP_EXAMPLE12,
    XDR_TRIM_STEP_NONE,
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

static void xdr_trim_param_init(void)
{
    for (xdr_trim_list_t trim_list = XDR_TRIM_LIST_CURRENT_REF ; trim_list < XDR_TRIM_LIST_MAX; ++trim_list)
    {
        switch (trim_list)
        {
            case XDR_TRIM_LIST_CURRENT_REF:
                break;
            case XDR_TRIM_LIST_LDO_DIG:
                break;
            case XDR_TRIM_LIST_LDO_DAC:
                break;
            case XDR_TRIM_LIST_LDO_FLL:
                break;
            case XDR_TRIM_LIST_OSC:
                break;
            case XDR_TRIM_LIST_CH_GAIN:
                __priv_trim.t_trim_info[trim_list].input[0] = XDR_TRIM_GAIN_INPUT_1;
                __priv_trim.t_trim_info[trim_list].input[1] = XDR_TRIM_GAIN_INPUT_2;
                __priv_trim.t_trim_info[trim_list].gain = GAIN_HIGH;
                break;
            case XDR_TRIM_LIST_CH_OFS:
                __priv_trim.t_trim_info[trim_list].input[0] = XDR_TRIM_OFS_INPUT_1;
                __priv_trim.t_trim_info[trim_list].input[1] = XDR_TRIM_OFS_INPUT_2;
                __priv_trim.t_trim_info[trim_list].gain = GAIN_HIGH;
                break;
        }
    }
}

static bool _xcr_trim_thread(struct thread_data* td)
{
    switch(td->step)
    {
    case XCR_TRIM_STEP_PWR_ON:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %u, timeout : %u", __func__, td->id, td->step, td->tout);
        td->step = XCR_TRIM_STEP_EXAMPLE1;
        td->tout = 100;
        break;

    case XCR_TRIM_STEP_EXAMPLE1:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        td->step = XCR_TRIM_STEP_EXAMPLE2;
        td->tout = 250;
        break;

    case XCR_TRIM_STEP_EXAMPLE2:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        td->step = XCR_TRIM_STEP_EXAMPLE3;
        td->tout = 150;
        break;

    case XCR_TRIM_STEP_EXAMPLE3:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        td->step = XCR_TRIM_STEP_NONE;
        td->tout = 500;
        break;

    default:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        __priv_trim.trim_thr = INVALID_THREAD_ID;
        return false;
    }

    return true;
}

static bool _xdr_trim_thread(struct thread_data* td)
{
    switch(td->step)
    {
    case XDR_TRIM_STEP_PWR_ON:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %u, timeout : %u", __func__, td->id, td->step, td->tout);
        /* Power On Sequence */
        gpio_set_xd_vdd_5v(XD_PWR_ON_5V0);
        td->step = XDR_TRIM_STEP_EXAMPLE01;
        td->tout = XDR_DELAY_PWR_ON;
        break;

    case XDR_TRIM_STEP_EXAMPLE01:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Initialization */
        xdr12_trim_init();
        td->step = XDR_TRIM_STEP_EXAMPLE02;
        td->tout = XDR_DELAY_DEFAULT;
        break;

    case XDR_TRIM_STEP_EXAMPLE02:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Prepare for Each Trim List */
        switch (__priv_trim.t_xdr_trim_list)
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
                xdr12_trim_ch_gain();
                break;
            case XDR_TRIM_LIST_CH_OFS:
                xdr12_trim_ch_ofs();
                break;
        }
        td->step = XDR_TRIM_STEP_EXAMPLE03;
        td->tout = XDR_DELAY_DEFAULT;
        break;

    case XDR_TRIM_STEP_EXAMPLE03:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Input Conditions for Each Trim List & Start Measure */
        switch (__priv_trim.t_xdr_trim_list)
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
                xdr12_trim_set_max_curr_vref(__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].input[__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].repeat]);
                gpio_set_demux_channel_selection(__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].chx);
                gpio_set_current_gain(__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].gain);
                break;
            case XDR_TRIM_LIST_CH_OFS:
                xdr12_trim_set_max_curr_vref(__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].input[__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].repeat]);
                gpio_set_demux_channel_selection(__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].chx);
                gpio_set_current_gain(__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].gain);
                break;
        }
        td->step = XDR_TRIM_STEP_EXAMPLE04;
        td->tout = XDR_DELAY_MEASURE;
        break;

    case XDR_TRIM_STEP_EXAMPLE04:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Measure */
        switch (__priv_trim.t_xdr_trim_list)
        {
            case XDR_TRIM_LIST_CURRENT_REF:
            case XDR_TRIM_LIST_LDO_DIG:
            case XDR_TRIM_LIST_LDO_DAC:
            case XDR_TRIM_LIST_LDO_FLL:
                __priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].measure.adc[__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].repeat] = mcu_peripheral_adc_get();
                __priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].measure.value[__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].repeat] = mcu_peripheral_adc_conversion_to_value(__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].measure.adc[__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].repeat]);
                break;
            case XDR_TRIM_LIST_OSC:
                __priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].measure.value[__priv_trim.t_trim_info[__priv_trim.t_xdr_trim_list].repeat] = mcu_peripheral_tim_conversion_freq();
                break;
            case XDR_TRIM_LIST_CH_GAIN:
                break;
            case XDR_TRIM_LIST_CH_OFS:
                break;
        }
        td->step = XDR_TRIM_STEP_EXAMPLE05;
        td->tout = 500;
        break;

    case XDR_TRIM_STEP_EXAMPLE05:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Compare */
        td->step = XDR_TRIM_STEP_EXAMPLE06;
        td->tout = 500;
        break;

    case XDR_TRIM_STEP_EXAMPLE06:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Update Register */
        td->step = XDR_TRIM_STEP_EXAMPLE07;
        td->tout = 500;
        break;

    case XDR_TRIM_STEP_EXAMPLE07:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Check Remain Trim List */
        td->step = XDR_TRIM_STEP_EXAMPLE08;
        td->tout = 500;
        break;

    case XDR_TRIM_STEP_EXAMPLE08:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Prepare for eFuse */
        td->step = XDR_TRIM_STEP_EXAMPLE09;
        td->tout = 500;
        break;

    case XDR_TRIM_STEP_EXAMPLE09:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR eFuse */
        td->step = XDR_TRIM_STEP_EXAMPLE10;
        td->tout = 500;
        break;

    case XDR_TRIM_STEP_EXAMPLE10:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Power Reboot */
        td->step = XDR_TRIM_STEP_EXAMPLE11;
        td->tout = 500;
        break;

    case XDR_TRIM_STEP_EXAMPLE11:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR eFuse Check */
        td->step = XDR_TRIM_STEP_EXAMPLE12;
        td->tout = 500;
        break;

    case XDR_TRIM_STEP_EXAMPLE12:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        /* XDR Power Off */
        td->step = XDR_TRIM_STEP_NONE;
        td->tout = 500;
        break;

    default:
        comm_UART_Printf(LOG_LV_INFO, "\n\r%s, step : %u, timeout : %u", __func__, td->step, td->tout);
        __priv_trim.trim_thr = INVALID_THREAD_ID;
        return false;
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