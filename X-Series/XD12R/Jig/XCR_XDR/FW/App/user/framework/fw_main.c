
#include "fw_internal.h"

extern void sys_init(void);
extern void sys_normal_mode(void);
extern void _system_idle(void);

struct manager *__managers[NUMBER_OF_MGRS]=
{
    &__app_mgr,     /* APP */
    &__det_mgr,     /* DETECTORS (BTN, External IO, ...) */

    &__trim_mgr,   /* TRIM for XCR/XDR */
};

void fw_run(void)
{
    sys_init();

    while(1)
    {
        _fw_thread_init();
        sys_normal_mode();

        MGR_APP()->power(true);

        while(MGR_APP()->status() != STATUS_UNPOWER)
        {
            _fw_threadmgr_do();
            _system_idle();
        }
    }
}