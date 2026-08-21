#include "framework.h"
#include "app_manager.h"
#include "drv_xc24.h"

extern void sys_init(void);
extern void sys_normal_mode(void);
extern void _system_idle(void);

struct manager *__managers[MGR_INDEX_MAX]=
{
    &__mgr_app,     /* APP */
    &__mgr_det,     /* DETECTORS (BTN, External IO, ...) */
    &__mgr_trim,    /* TRIM for XC/XD */
    &__mgr_test,    /* TEST for XC/XD */
};

void fw_run(void)
{
    sys_init();

    while(1)
    {
        fw_thread_init();
        sys_normal_mode();

        MGR_APP()->power(true);

        while(MGR_APP()->status() != STATUS_UNPOWER)
        {
            fw_threadmgr_do();
            _system_idle();
            xc24_ld_transfer_nss_release();
        }
    }
}