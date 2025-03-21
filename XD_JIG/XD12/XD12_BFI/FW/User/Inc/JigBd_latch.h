/** @file xd12.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __JIGBD_LATCH_H__
#define __JIGBD_LATCH_H__

#ifdef LATCH_SUPPORT

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __JIGBD_LATCH_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef __cplusplus
 extern "C" {
#endif

#define LATCH_PIN_RESET           0
#define LATCH_PIN_SET             1

#define LATCH_CP1                 1
#define LATCH_CP2                 2
#define LATCH_CP3                 3
#define LATCH_D0                  0
#define LATCH_D1                  1
#define LATCH_D2                  2
#define LATCH_D3                  3
#define LATCH_D4                  4
#define LATCH_D5                  5
#define LATCH_D6                  6
#define LATCH_D7                  7

#define MODE_SEL_LS_PORT          LATCH_D0
#define MODE_SEL_LS_CP            LATCH_CP1
#define ENABLE_SELECT1_LS_PORT    LATCH_D1
#define ENABLE_SELECT1_LS_CP      LATCH_CP1
#define ENABLE_SELECT2_LS_PORT    LATCH_D2
#define ENABLE_SELECT2_LS_CP      LATCH_CP1
#define ENABLE_SELECT3_LS_PORT    LATCH_D3
#define ENABLE_SELECT3_LS_CP      LATCH_CP1
#define LTC_LOW_CURRENT_LS_PORT   LATCH_D4
#define LTC_LOW_CURRENT_LS_CP     LATCH_CP1
#define LTC_MID_CURRENT_LS_PORT   LATCH_D5
#define LTC_MID_CURRENT_LS_CP     LATCH_CP1
#define LTC_HIGH_CURRENT_LS_PORT  LATCH_D6
#define LTC_HIGH_CURRENT_LS_CP    LATCH_CP1
#define ADC_START_LS_PORT         LATCH_D7
#define ADC_START_LS_CP           LATCH_CP1
#define DAC_CS_LS_PORT            LATCH_D0
#define DAC_CS_LS_CP              LATCH_CP2
#define ADC_RESET_LS_PORT         LATCH_D1
#define ADC_RESET_LS_CP           LATCH_CP2
#define ADC_CS_LS_PORT            LATCH_D2
#define ADC_CS_LS_CP              LATCH_CP2

EXTERN void LATCH__WritePin(uint8_t latch_num, uint16_t latch_bit, uint8_t ub_val);

#ifdef DBG_TEST
//
#endif //DBG_TEST


#ifdef __cplusplus
}
#endif

#endif /* ~__JIGBD_LATCH_H__ */

#endif //LATCH_SUPPORT
/*** end of file ***/