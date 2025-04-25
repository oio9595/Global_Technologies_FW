/** @file jigbd_hw.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __JIGBD_HW_H__
#define __JIGBD_HW_H__

#ifdef __cplusplus
 extern "C" {
#endif

extern void LATCH__WritePin(uint8_t latch_num, uint16_t latch_bit, uint8_t ub_val);

extern void AQIC_VCC_EN(uint8_t on);
extern void AQIC_VLED_EN(uint8_t on);
extern void AQIC_Mode_Duty(uint8_t duty);
extern void AQIC_Mode_Setting(uint8_t op_mode);

extern void change_current_gain(uint8_t gain);
extern void current_discharge(uint8_t mode);

extern void set_ok_led(uint8_t en);
extern void set_ng_led(uint8_t en);
extern void set_trimming_led(uint8_t en);

#ifdef __cplusplus
}
#endif

#endif /* ~__JIGBD_HW_H__ */

/*** end of file ***/

