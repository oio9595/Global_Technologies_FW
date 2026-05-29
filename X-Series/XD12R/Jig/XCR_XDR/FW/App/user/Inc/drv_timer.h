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

void tim_vsync_out_start(void);
void tim_vsync_out_stop(void);

void tim_svsync_out_handler(void);
void tim_vsync_out_handler(void);

void tim_set_vsync_out_freq(float f);

void tim_vsync_out_process(void);

#ifdef __cplusplus
}
#endif

#endif	/* DRV_TIMER_H */


