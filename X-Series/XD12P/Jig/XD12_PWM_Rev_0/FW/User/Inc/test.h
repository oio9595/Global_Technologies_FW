/** @file xd12.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __TEST_H__
#define __TEST_H__
#ifdef DBG_TEST

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __TEST_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef __cplusplus
 extern "C" {
#endif


EXTERN void _test_IF_TEST(void);



#ifdef __cplusplus
}
#endif


#endif // DBG_TEST
#endif /* ~__TEST_H__ */

/*** end of file ***/


