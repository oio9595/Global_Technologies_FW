#include "fw_internal.h"
#include "main.h"

#define NUM_OF_THREAD	(16U)

static struct thread_data __tdata[NUM_OF_THREAD];

static THREAD_ID __last_id;
static volatile uint32_t __system_clock;
static uint32_t __freeze_sys_clock;	/* thread를 run하는 시점과 run중 begin thread를 호출하는 시점이 달라질수 있기 때문에 이 변수를 사용 */

void fw_systick_handler(void)
{
    ++__system_clock;
    if(__system_clock == 0xffffffff)
    {
        __system_clock = 0U;
    }
}

uint32_t get_system_tick(void)
{
    return __system_clock;
}

static struct thread_data* _search_tdata(THREAD_ID tid)
{
    for(uint32_t i=0;i<NUM_OF_THREAD;++i)
    {
        if(__tdata[i].id==tid)
        {
            return __tdata+i;
        }
    }

    return NULL;
}

static THREAD_ID fw_begin_thread_trig_ex(bool (*const f)(struct thread_data*), uint32_t tout)
{
    uint32_t index = 0U;
    bool is_duplicate;

    for(index = 0U; index < NUM_OF_THREAD; ++index)
    {
        if(__tdata[index].id == INVALID_THREAD_ID)
        {
            break;
        }
    }

    if(index == NUM_OF_THREAD)
    {
        return INVALID_THREAD_ID;
    }

    do
    {
        ++__last_id;
        if(__last_id == INVALID_THREAD_ID)
        {
            __last_id = 1U;
        }

        is_duplicate = false;

        for(uint32_t i = 0U ; i < NUM_OF_THREAD ; ++i)
        {
            if(__tdata[i].id == __last_id)
            {
                is_duplicate = true;
                break;
            }
        }
    }while(is_duplicate);

    __tdata[index].id       = __last_id;
    __tdata[index].step     = 0U;
    __tdata[index].tout     = tout;
    __tdata[index].last_t   = __freeze_sys_clock;
    __tdata[index].func     = f;

    return __tdata[index].id;
}

THREAD_ID fw_begin_thread_ex(bool (*const f)(struct thread_data*), uint32_t tout)
{
    return fw_begin_thread_trig_ex(f, tout);
}

THREAD_ID fw_begin_thread(bool (*const f)(struct thread_data*))
{
    return fw_begin_thread_trig_ex(f, 0U);
}

static void _thread_stop(struct thread_data* const td)
{
    td->id = INVALID_THREAD_ID;
}

void fw_thread_stop(THREAD_ID tid)
{
    struct thread_data *td = _search_tdata(tid);
    if(td!=NULL)
    {
        _thread_stop(td);
    }
}

static void _fw_run_thread(struct thread_data* const thr, uint32_t sys_clock)
{
    if(thr->func(thr) == false)
    {
        _thread_stop(thr);
    }
    else
    {
        thr->last_t = sys_clock;
    }
}

void _fw_thread_init(void)
{
    fw_memset(__tdata, 0x00, sizeof(__tdata));
}

void _fw_threadmgr_do(void)
{
    __freeze_sys_clock = __system_clock;

    for(uint32_t i = 0U ; i < NUM_OF_THREAD ; ++i)
    {
        if(__tdata[i].id != INVALID_THREAD_ID)
        {
            if((__freeze_sys_clock - __tdata[i].last_t) >= __tdata[i].tout)
            {
                _fw_run_thread(__tdata + i, __freeze_sys_clock);
            }
        }
    }
}