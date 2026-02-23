/** @file fnd.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __FND_H__
#define __FND_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "types.h"

void fnd_init(void);
void fnd_test_mode_update(test_mode_t t_test_mode);
void fnd_pwr_state_update(power_state_t t_power_state, auto_manual_t t_manual_test_state);
void fnd_test_result_update(test_mode_t t_mode, uint8_t n_result);

#ifdef __cplusplus
}
#endif

#endif /* ~__FND_H__ */

/*** end of file ***/


