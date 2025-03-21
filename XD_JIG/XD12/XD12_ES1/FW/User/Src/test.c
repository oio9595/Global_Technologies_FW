/** @file xd12.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __TEST_C__
#include "config.h"

#ifdef DBG_TEST

/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/

void _test_IF_TEST(void)
{
    // JIG BD Init Test
    if(IS_XC24())
    {
        _dbg_XC24_Detect();
    }
    else
    {
        _dbg_XD12_Detect();
    }
}

/* END - INTERFACE FUNCTIONS ******************************************************************/

#endif // DBG_TEST

/*** end of file ***/

