/** @file reliable_task.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __RELIABLE_TASK_H__
#define __RELIABLE_TASK_H__

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __XD12_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif
#ifdef __cplusplus
 extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#include "main.h"


#define FAULT_FB_VLED_MAX       (2.5f)
#define FAULT_SHORT_VLED_MAX    (40.0f)

extern bool gb_xd_id_read_error_flag;

extern void Vsync_Timer_Start(void);
extern void Vsync_Timer_Stop(void);
extern void Vsync_Update_Handler(void);
extern void XD12_Vsync_Task(void);

extern void Transmit_SPI_LD_Buffer(void);

extern void XD12_Set_Write_Target_Reg(uint8_t addr, uint16_t data);
extern void XD12_Set_Read_Target_Reg(uint8_t addr);

extern void XD12_set_LD_out(uint32_t in_ld_out);
extern uint16_t XD12_get_LD_out(void);

#if 1
extern void XD12_get_fault_status(void);
#endif


#endif /* ~__RELIABLE_TASK_H__ */
/*** end of file ***/
