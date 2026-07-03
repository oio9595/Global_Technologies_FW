
#include <stdlib.h>

#include "framework.h"

typedef bool (*thread_func)(struct thread_data* tdata);

static struct{
    MGRSTATUS	status;

    THREAD_ID power_on_thr;
}__priv_app;

static bool _power_on_thread(struct thread_data* tdata)
{
    bool ret = true;

    switch(tdata->step)
    {
    case 0:
        MGR_DET()->power(true);
        break;
    case 1:
        MGR_TRIM()->power(true);
        break;
    case 2:
        MGR_TEST()->power(true);
        __priv_app.status = STATUS_NORMAL;
        __priv_app.power_on_thr = INVALID_THREAD_ID;
        ret = false;
        break;
    default:
        break;
    }

    ++(tdata->step);

    return ret;
}

static void _power(bool on)
{
    if(true == on)
    {
        __priv_app.power_on_thr = fw_begin_thread_ex(_power_on_thread, 10U);    /* 10ms */
        __priv_app.status = STATUS_INIT;
    }
    else
    {
        if(__priv_app.power_on_thr != INVALID_THREAD_ID)
        {
            fw_thread_stop(__priv_app.power_on_thr);
            __priv_app.power_on_thr = INVALID_THREAD_ID;
        }

        __priv_app.status = STATUS_UNPOWER;
    }
}

static void _enable(bool en)
{

}

static MGRSTATUS _status(void)
{
    return __priv_app.status;
}

static uint32_t _cmd(uint32_t cmd, void* val)
{
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
    switch(type)
    {
    case 0U:
        switch(*((BtnEvent_t*)val))
        {
        case BTN_EVT_PRESSED:
        case BTN_EVT_RELEASED:
        case BTN_EVT_SHORT_CLICK:
        case BTN_EVT_LONG_PRESS:
            break;
        default:
            break;
        }
    default:
        break;
    }

    return MGRET_OK;
}

struct manager __app_mgr=
{
    _power,
    _enable,
    _status,
    _cmd,
    _write,
    _read,
    _noti,
};

