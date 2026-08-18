/*
 * File:   drv_timer.h
 * Author: GT
 * for XCR24 & XDR12 ES2
 * Created on 2026. 05. 15.
 */

#ifndef DRV_TIMER_H
#define	DRV_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>

void tim_vsync_out_start(void);
void tim_vsync_out_stop(void);

void tim_vsync_out_for_test_start(void);
void tim_vsync_out_for_test_stop(void);

void tim_fllsync_start(void);
void tim_fllsync_stop(void);

void tim_svsync_out_handler(void);
void tim_vsync_out_handler(void);

void tim_set_vsync_out_freq(float f);

void tim_vsync_out_process(void);

void tim_set_vsync_out_running_flag(bool running);
bool tim_get_vsync_out_running_flag(void);

void tim_set_xd_read_info(uint16_t addr, uint8_t addr_type);
void tim_set_xd_write_info(uint16_t addr, uint16_t data, uint8_t addr_type);

void tim_set_xc_read_info(uint16_t addr, uint8_t addr_type);
void tim_set_xc_write_info(uint16_t addr, uint16_t data, uint8_t addr_type);

#ifdef __cplusplus
}
#endif

#endif	/* DRV_TIMER_H */


