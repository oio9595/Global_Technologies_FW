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

extern bool gb_jig_vsync_running_flag;

extern void Vsync_Timer_Start(void);
extern void Vsync_Timer_Stop(void);
extern void Vsync_Update_Handler(void);
extern void XD04_Vsync_Task(void);

extern void XD04_Set_Write_Target_Reg(uint8_t addr, uint16_t data);
extern void XD04_Set_Read_Target_Reg(uint8_t addr);

extern void XD04_set_dimming_gain(current_gain_t in_gain);
extern current_gain_t XD04_get_dimming_gain(void);

extern void XD04_set_LD_out(uint32_t in_ld_out);
extern uint16_t XD04_get_LD_out(void);


#endif /* ~__DIMMING_TEST_H__ */
/*** end of file ***/
