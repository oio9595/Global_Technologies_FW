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

extern void Vsync_Timer_Start(void);
extern void Vsync_Timer_Stop(void);
extern void Vsync_Update_Handler(void);
extern void Vsync_Task(void);

extern void LED_BLK_Set_LD(uint8_t in_blk_num, uint16_t in_ld_value);
extern void LED_Current_Select(float in_current);
extern void LED_Generate_Pattern(uint8_t in_pattern);
extern void LED_Change_Pattern(uint8_t in_pattern);
extern uint8_t LED_Get_Pattern(void);

extern void LED_Change_Pattern_to_P1(void);
extern void LED_Change_Pattern_to_P2(void);
extern void LED_Change_Pattern_to_P3(void);
extern void LED_Change_Pattern_to_None(void);

extern void LED_System_Init(void);
extern void LED_System_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__VSYNC_TASK_H__ */
/*** end of file ***/
