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

extern bool gb_jig_vsync_active;

extern void Vsync_Timer_Start(void);
extern void Vsync_Timer_Stop(void);
extern void Vsync_Update_Handler(void);
extern void XDIC_Vsync_Task(void);

extern void XDIC_Set_Write_Target_Reg(uint8_t addr, uint16_t data);
extern void XDIC_Set_Read_Target_Reg(uint8_t addr);

extern void XDIC_Set_LD_Data(uint32_t in_ld_out);
extern uint16_t XDIC_Get_LD_Data(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__VSYNC_TASK_H__ */
/*** end of file ***/
