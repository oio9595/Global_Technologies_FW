/** @file dimming_test.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __DIMMING_TEST_H__
#define __DIMMING_TEST_H__

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

#define FAULT_FB_VLED_MAX       (2.5f)
#define FAULT_SHORT_VLED_MAX    (40.0f)

extern bool gb_jig_vsync_active;

extern void Vsync_Timer_Start(void);
extern void Vsync_Timer_Stop(void);
extern void Vsync_Update_Handler(void);
extern void XD12_Vsync_Task(void);

extern void XD12_Set_Write_Target_Reg(uint8_t addr, uint16_t data);
extern void XD12_Set_Read_Target_Reg(uint8_t addr);

extern void XD12_set_dimming_gain(current_gain_t in_gain);
extern current_gain_t XD12_get_dimming_gain(void);

extern void XD12_Set_LD_Data(uint32_t in_ld_out);
extern uint16_t XD12_Get_LD_Data(void);

#if 1
extern void XD12_get_fault_status(void);
#endif


#endif /* ~__DIMMING_TEST_H__ */
/*** end of file ***/
