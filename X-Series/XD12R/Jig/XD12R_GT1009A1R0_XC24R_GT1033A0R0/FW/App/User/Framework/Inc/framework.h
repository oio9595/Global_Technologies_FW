#ifndef __FRAMEWORK_H__
#define __FRAMEWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/****** THREAD MANAGER *****/
typedef uint32_t            THREAD_ID;
#define INVALID_THREAD_ID   (0U)

typedef enum
{
    BTN_B1 = 0U,
    BTN_MAX,
}BtnType_t;

typedef enum
{
    BTN_EVT_IDLE = 0U,
    BTN_EVT_PRESSED,
    BTN_EVT_RELEASED,
    BTN_EVT_SHORT_CLICK,
    BTN_EVT_LONG_PRESS,
}BtnEvent_t;

struct thread_data
{
    THREAD_ID id;

    uint32_t step;
    uint32_t tout;
    uint32_t last_t;
    bool (*func)(struct thread_data*);
};

void fw_thread_init(void);
void fw_threadmgr_do(void);

extern void fw_systick_handler(void);
extern uint32_t get_system_tick(void);

extern THREAD_ID fw_begin_thread_ex(bool (*const f)(struct thread_data*), uint32_t tout);
extern THREAD_ID fw_begin_thread(bool (*const f)(struct thread_data*));
extern void fw_thread_stop(THREAD_ID tid);

#ifdef __cplusplus
}
#endif
#endif