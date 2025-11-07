/** @file xdic.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XDIC_H__
#define __XDIC_H__

#ifdef __cplusplus
extern "C" {
#endif

extern volatile bool gb_xdic_type_is_xd04; //true: XD04, false: XD12
extern uint8_t gn_xdic_channel_size;

extern void XDIC_Detect_Type(void);

extern void XDIC_Write_General_Reg(uint8_t addr, uint16_t data);
extern uint16_t XDIC_Read_General_Reg(uint8_t addr);
extern uint16_t XDIC_Get_General_Reg(uint8_t addr);
extern void XDIC_Read_All_Registers(void);

extern void XDIC_Init(void);

extern void XDIC_Set_Max_Current_Level(uint8_t in_dev_max_curr);
extern void XDIC_Set_Max_Curr_Vref(uint16_t in_max_curr_vref);

extern void XDIC_Trim_Init_VREF_CTL(void);
extern void XDIC_Trim_Init_OSC(void);
extern void XDIC_Trim_Init_Current_Type_A(void);
extern void XDIC_Trim_Init_Current_Type_B(void);
extern void XDIC_Trim_Init_ICC(void);

#ifdef __cplusplus
}
#endif

#endif /* ~__XDIC_H__ */

/*** end of file ***/
