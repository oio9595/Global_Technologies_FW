#include <math.h>

#include "framework.h"
#include "drv_xdr12.h"
#include "drv_xcr24.h"
#include "comm_debugging.h"

#define XCR_DELAY_DEFAULT                   (10U)
#define XCR_DELAY_MEASURE                   (100U)
#define XCR_DELAY_PWR_ON                    (100U)
#define XCR_DELAY_PWR_OFF                   (100U)
#define XCR_DELAY_EFUSE_DONE                (500U)

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

static struct{
    MGRSTATUS   status;
    test_step_t xc_test_step;
    test_step_t xd_test_step;
    THREAD_ID   test_thr;
}__priv_test;

static const uint8_t* gs_test_step[TEST_STEP_MAX] =
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

static bool _xcr_test_thread(struct thread_data* td)
{
    switch(td->step)
    {
        case TEST_STEP_PWR_ON:
        {
            td->step = TEST_STEP_INITIAL;
            td->tout = XCR_DELAY_PWR_ON;
            break;
        }

        case TEST_STEP_INITIAL:
        {
            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_START_MEASURE;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_GET_MEASURED_VALUE;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_JUDGE_RANGE_UPDATE_REGISTER;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_LOG_SUMMARY:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_PWR_OFF;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_PWR_OFF:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_NONE;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, gs_test_step[td->step], td->tout);
            __priv_test.test_thr = INVALID_THREAD_ID;
            return false;
        }
    }
    return true;
}

static bool _xdr_test_thread(struct thread_data* td)
{
    switch(td->step)
    {
        case TEST_STEP_PWR_ON:
        {
            gpio_set_xd_vdd_5v(VCC_ON_3V3);
            td->step = TEST_STEP_INITIAL;
            td->tout = XCR_DELAY_PWR_ON;
            break;
        }

        case TEST_STEP_INITIAL:
        {
            td->step = TEST_STEP_INITIAL_BY_LIST;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_INITIAL_BY_LIST:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_START_MEASURE;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_START_MEASURE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_GET_MEASURED_VALUE;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_GET_MEASURED_VALUE:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_JUDGE_RANGE_UPDATE_REGISTER;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_LOG_SUMMARY:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_PWR_OFF;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        case TEST_STEP_PWR_OFF:
        {
            comm_UART_Printf(LOG_LV_DEBUG, "\n\r\tstep : %s, list : %s, timeout : %u", gs_test_step[td->step], td->tout);
            td->step = TEST_STEP_NONE;
            td->tout = XCR_DELAY_DEFAULT;
            break;
        }

        default:
        {
            comm_UART_Printf(LOG_LV_INFO, "\n\r%s, id : %u, step : %s, timeout : %u", __func__, td->id, gs_test_step[td->step], td->tout);
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
        case TRIM_CMD_XCR_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xcr_test_thread, 10U);    /* 10ms */
            }
            break;
        }
        case TRIM_CMD_XDR_START:
        {
            if(__priv_test.test_thr == INVALID_THREAD_ID)
            {
                __priv_test.test_thr = fw_begin_thread_ex(_xdr_test_thread, 10U);    /* 10ms */
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