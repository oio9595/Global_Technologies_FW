
/** @file config.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) GT. All rights reserved.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************/
#define VERSION_MAJOR       (0)
#define VERSION_MINOR       (0)
#define VERSION_PATCH       (1) // At every new build, increase this value
/********************************************************/
#define APB1_TIM_FREQ       (90.0f) //MHz
#define APB2_TIM_FREQ       (APB1_TIM_FREQ * 2) //MHz

#define CONST_MHz_TO_Hz     (1000000.0f)
#define CONST_mV_TO_V       (1000.0f)
/********************************************************/
#define ANSI_FONT_NONE      "\033[0m"
#define ANSI_FONT_RED       "\033[31m"
#define ANSI_FONT_GREEN     "\033[32m"
#define ANSI_FONT_YELLOW    "\033[33m"
#define ANSI_FONT_BLUE      "\033[34m"
#define ANSI_FONT_MAGENTA   "\033[35m"
/********************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ~__CONFIG_H__ */

/*** end of file ***/