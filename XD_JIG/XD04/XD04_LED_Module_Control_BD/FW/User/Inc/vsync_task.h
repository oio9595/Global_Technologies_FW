/** @file vsync_task.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __VSYNC_TASK_H__
#define __VSYNC_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "config.h"

extern bool gb_xd_led_enable_table[TOTAL_BLOCK_SIZE];

extern void Vsync_Timer_Start(void);
extern void Vsync_Timer_Stop(void);
extern void Vsync_Update_Handler(void);
extern void XDIC_Vsync_Task(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__VSYNC_TASK_H__ */
/*** end of file ***/
