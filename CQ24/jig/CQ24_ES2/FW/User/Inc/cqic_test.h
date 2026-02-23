/** @file amic_test.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __AMIC_TEST_H__
#define __AMIC_TEST_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

void dac_set_voltage(float f_voltage, uint8_t ch);
void set_vref_voltage(float f_voltage);
void test_procedure_run(void);
void trimming_procedure_run(void);


enum
{
	S1X6	= 0,
	S2X3,
	S3X2,
	D1X6,
	D2X3,
	D3X2
};

enum
{
	M1X6	= 0,
	M2X3,
	M3X2,
	ENTERY
};

#ifdef __cplusplus
}
#endif

#endif /* ~__AMIC_TEST_H__ */

/*** end of file ***/


