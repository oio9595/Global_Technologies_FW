/** @file common.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) GT. All rights reserved.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#define BUILD_YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
              + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))
 
#define BUILD_MONTH (__DATE__ [2] == 'n' ? 0 \
               : __DATE__ [2] == 'b' ? 1 \
               : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
               : __DATE__ [2] == 'y' ? 4 \
               : __DATE__ [2] == 'n' ? 5 \
               : __DATE__ [2] == 'l' ? 6 \
               : __DATE__ [2] == 'g' ? 7 \
               : __DATE__ [2] == 'p' ? 8 \
               : __DATE__ [2] == 't' ? 9 \
               : __DATE__ [2] == 'v' ? 10 : 11)
 
#define BUILD_DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))

#define VERSION_MAJOR       0x0000
#define VERSION_MINOR       0x0001
#define VERSION_REVISION    0x0001

#define _GUI_SUPPORT_           1

#define CMD_GUI_KEY_POWER_UP            "ui:kpup"
#define CMD_GUI_KEY_TRIMMING_UP         "ui:ktup"
#define CMD_GUI_KEY_MANUAL_STEP_UP      "ui:kmup"
#define CMD_GUI_ACTIVATE                "ui:gact"
#define CMD_GUI_TRIMSTART               "ui:gtrim"
#define CMD_GUI_READ_REGISTER_ALL       "ui:grreg"
#define CMD_GUI_TRIMSVOLT               "ui:gvolt"
#define CMD_GUI_TRIMSRANGE              "ui:grang"
#define CMD_GUI_CZMCZN                  "ui:czmn"
#define CMD_GUI_RESET                   "ui:reset"
#define CMD_GUI_TRIM_PARAM_OSC          "ui:p_osc "
#define CMD_GUI_TRIM_PARAM_VREF         "ui:p_vref "
#define CMD_GUI_TRIM_PARAM_GLB          "ui:p_glb "
#define CMD_GUI_TRIM_PARAM_OFFSET       "ui:p_offset "
#define CMD_GUI_TRIM_PARAM_GAIN         "ui:p_gain "
#define CMD_GUI_TRIM_PARAM_SCREEN       "ui:p_screen "
#define CMD_GUI_TRIM_START              "ui:gtrim "
#define CMD_GUI_NO_TRIM_MODE            "ui:din "
#define CMD_GUI_WRITE_OTP_ENABLE        "ui:wrotp "
#define CMD_GUI_WRITE_REGISTER          "ui:gwreg "
#define CMD_GUI_TEST                    "ui:test"
#define CMD_GUI_WITHOUT_SCREEN          "ui:noscr "
#define CMD_XD12_READ_REGISTER_ALL      "xdic:rega"
#define CMD_XD12_OTP_WRITTEN            "xdic:wotp"
#define CMD_XDIC_SLOPE_START            "xdic:slop"
#define CMD_XDIC_SLOPE_ING              "xdic:csvu:"
#define CMD_XDIC_GDIM                   "xdic:"

#endif /* ~__COMMON_H__ */

/*** end of file ***/