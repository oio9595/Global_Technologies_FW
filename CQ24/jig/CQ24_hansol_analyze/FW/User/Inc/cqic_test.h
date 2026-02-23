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
#include <stdbool.h>

#define CQ_DUTY_SIZE (12 * 14 * 1 + 1)
#define CQ_LDIM_SIZE (12 * 14 * 2)

void dac_set_voltage(float f_voltage, uint8_t ch);
void set_vref_voltage(float f_voltage);
void test_procedure_run(void);
void trimming_procedure_run(void);

uint16_t* get_duty_addr(void);
uint16_t* get_ldim_addr(void);

extern uint16_t gn_cq_write_target_cmd2_addr;
extern uint16_t gn_cq_write_target_cmd2_data;
extern bool gb_cq_write_cmd2_flag;

extern uint16_t gn_cq_write_target_cmd1_addr;
extern uint16_t gn_cq_write_target_cmd1_data;
extern bool gb_cq_write_cmd1_flag;

extern uint16_t gn_cq_read_target_addr;
extern bool gb_cq_read_flag;

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


