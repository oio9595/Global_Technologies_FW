#ifndef __APP_MANAGER_H__
#define __APP_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define MGRET_ERR           (0U)
#define MGRET_OK            (1U)

typedef enum tag_MGR_INDEX
{
    MGR_INDEX_APP = 0U,
    MGR_INDEX_DET,
    MGR_INDEX_TRIM,
    MGR_INDEX_TEST,
    MGR_INDEX_MAX,
} mgr_index_t;

typedef enum tag_MGR_STATUS
{
    STATUS_UNPOWER = 0,
    STATUS_INIT,
    STATUS_BUSY,
    STATUS_NORMAL,
    STATUS_STANDBY,
    STATUS_ERROR,
    STATUS_END
} mgr_status_t;

typedef enum tag_TRIM_COMMAND
{
    TRIM_CMD_XC_START = 0U,
    TRIM_CMD_XD_START,
} trim_command_t;

typedef enum tag_TEST_COMMAND
{
    TEST_CMD_XC_START = 0U,
    TEST_CMD_XD_START,
    TEST_CMD_XC_SWEEP_START,
    TEST_CMD_XD_SWEEP_START,
    TEST_CMD_XC_AGING_START,
    TEST_CMD_XD_AGING_START,
} test_command_t;

struct manager
{
    void (*power)(bool on);
    void (*enable)(bool en);
    mgr_status_t (*status)(void);
    uint32_t (*cmd)(uint32_t, void*);
    uint32_t (*write)(uint32_t, void*, uint32_t);
    uint32_t (*read)(uint32_t, void*, uint32_t);
    uint32_t (*noti)(uint32_t, void*);
};

extern struct manager __mgr_app;
extern struct manager __mgr_det;
extern struct manager __mgr_trim;
extern struct manager __mgr_test;
extern struct manager *__managers[MGR_INDEX_MAX];

#define MGR_APP()           __managers[MGR_INDEX_APP]
#define MGR_DET()           __managers[MGR_INDEX_DET]

#define MGR_TRIM()          __managers[MGR_INDEX_TRIM]
#define MGR_TEST()          __managers[MGR_INDEX_TEST]

#ifdef __cplusplus
}
#endif
#endif