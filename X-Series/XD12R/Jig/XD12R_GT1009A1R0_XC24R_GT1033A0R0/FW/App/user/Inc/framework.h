#ifndef __FRAMEWORK_H__
#define __FRAMEWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/****** THREAD MANAGER *****/
typedef uint32_t                THREAD_ID;
#define INVALID_THREAD_ID       (0U)

typedef enum __MGRSTATUS__
{
	STATUS_UNPOWER = 0,
	STATUS_INIT,
	STATUS_BUSY,
	STATUS_NORMAL,
	STATUS_STANDBY,
	STATUS_ERROR,
	STATUS_END
}MGRSTATUS;

/* thread manager */
struct thread_data
{
    THREAD_ID id;

    uint32_t step;
    uint32_t tout;
    uint32_t last_t;
    bool (*func)(struct thread_data*);
};

extern void fw_systick_handler(void);
extern uint32_t get_system_tick(void);

/* fw_begin_thread: */
//extern THREAD_ID fw_begin_thread_trig_ex(bool (*const f)(struct thread_data*), uint32_t tout);
extern THREAD_ID fw_begin_thread_ex(bool (*const f)(struct thread_data*), uint32_t tout);
extern THREAD_ID fw_begin_thread(bool (*const f)(struct thread_data*));
extern void fw_thread_stop(THREAD_ID tid);

/* thread를 함수 형태로 사용하기
BOOL thread1(struct thread_data *tdata)
{
	...
	switch(tdata->step)
	{
	case 0:
		fw_begin_thread_trig_ex(tdata, thread2, 0);
		tdata->step ++;
		break;
	case 1:
		// thread2가 종료되면 여기로 들어옴.
		break;
	}
	...
}
*/

/****** MEMORY MANAGER *****/
extern void fw_memset(void* ptr, uint8_t val, uint32_t size);


/**************************** NEW VERSION *************************************/
typedef enum
{
    MGR_APP_INDEX = 0U,
    MGR_DET_INDEX,
    MGR_TRIM_INDEX,
    MGR_TEST_INDEX,
    NUMBER_OF_MGRS,
}mgr_type_t;

#define MGRET_ERR       (0U)
#define MGRET_OK        (1U)

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

typedef enum
{
    TRIM_CMD_XCR_START = 0U,
    TRIM_CMD_XDR_START,
}TrimCommand_t;

typedef enum
{
    TEST_CMD_XCR_START = 0U,
    TEST_CMD_XDR_START,
    TEST_CMD_XCR_SWEEP_START,
    TEST_CMD_XDR_SWEEP_START,
}TestCommand_t;

struct manager
{
    void (*power)(bool on);
    void (*enable)(bool en);
    MGRSTATUS (*status)(void);
    uint32_t (*cmd)(uint32_t, void*);
    uint32_t (*write)(uint32_t, void*, uint32_t);
    uint32_t (*read)(uint32_t, void*, uint32_t);
    uint32_t (*noti)(uint32_t, void*);
};

extern struct manager __app_mgr;
extern struct manager __det_mgr;
extern struct manager __trim_mgr;
extern struct manager __test_mgr;
extern struct manager *__managers[NUMBER_OF_MGRS];

#define MGR_APP()   __managers[MGR_APP_INDEX]
#define MGR_DET()   __managers[MGR_DET_INDEX]

#define MGR_TRIM()  __managers[MGR_TRIM_INDEX]
#define MGR_TEST()  __managers[MGR_TEST_INDEX]

#ifdef __cplusplus
}
#endif
#endif