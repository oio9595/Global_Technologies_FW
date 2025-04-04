/** @file drv_sal.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef _DRV_SAL_H_
#define _DRV_SAL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "config.h"
#include "sal_struct.h"
#include "sal_driver.h"

typedef struct
{
    uint16_t dev_id;
    uint8_t command;
    uint8_t data_size;
    uint32_t data;
}_sal_single_ended_info_t;

typedef enum
{
    DEMO_PAT_NONE = 0,
    DEMO_PAT_RED,
    DEMO_PAT_GREEN,
    DEMO_PAT_BLUE,
    DEMO_PAT_WHITE,
    DEMO_PAT_HMC61,
    DEMO_PAT_MAX,
}demo_pat_t;

EXTERN volatile uint8_t gn_sal_rx_timeout;

EXTERN demo_pat_t gt_demo_pat;

EXTERN bool gb_sal_crc_en;
EXTERN bool gb_sal_init_flag;

EXTERN bool gb_sal_sync_flag;

EXTERN void sal_init(void);
// EXTERN void sal_make_crc_enable(uint8_t crc_en);
EXTERN void sal_make_mcu_crc(bool crc_en);
EXTERN void sal_set_crc_manual(uint8_t manual_crc_en, uint8_t manual_crc);

EXTERN void sal_set_preamble(uint8_t n_preamble);
EXTERN uint8_t sal_get_preamble(void);

EXTERN void sal_write_reg_single_ended(_sal_single_ended_info_t *p_info);
EXTERN uint32_t sal_read_reg_single_ended(_sal_single_ended_info_t *p_info);
EXTERN uint8_t sal_write_SR_reg_single_ended(_sal_single_ended_info_t *p_info);

EXTERN uint32_t* sal_get_rgb_buffer(void);
EXTERN void sal_demo_process(void);
EXTERN void sal_set_read_target(uint32_t dev_id, uint32_t cmd);
EXTERN void sal_set_status2_read_enable(void);
EXTERN void sal_set_write_target(uint32_t dev_id, uint32_t cmd, uint32_t value);
EXTERN void sal_set_write_SR_target(uint32_t dev_id, uint32_t cmd, uint32_t value);

EXTERN void sal_set_pattern(demo_pat_t n_demo_pat);
/*
EXTERN void sal_crc_repeat_test(void);
*/
/* BEGIN - INTERFACE FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ~_DRV_SAL_H_ */

/*** end of file ***/
