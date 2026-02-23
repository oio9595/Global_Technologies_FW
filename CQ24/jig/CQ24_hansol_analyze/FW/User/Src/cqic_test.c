/** @file amic_test.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#include "main.h"
#include "log.h"
#include "cqic_test.h"
#include "user_flash.h"
#include "fnd.h"
#include "ADS124S08.h"
#include "trimming.h"
#include "cqic.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim12;

uint16_t gn_auto_test_count;
extern uint16_t gn_auto_test_ok_count;
extern uint16_t gn_auto_test_ng_count;

test_mode_t gt_jig_test_mode;
test_mode_t gt_jig_manual_test_mode;
extern auto_manual_t gt_auto_manual_state;

extern cqic_test_step_t gt_jig_trimming_step;
uint16_t gt_cqic_test_step;

extern uint8_t gn_cqic_test_type;      /* 0 : normal test, 1 : 1 : trimming */

uint8_t gb_adc_start_dma;
uint16_t gn_adc_dma_buffer[4];
uint32_t gn_mode_temp_adc[AMIC_O_MAX];
uint16_t fill_data_buf[2048];

int16_t gn_adc_read_count_adc1;
int16_t gn_adc_read_count_adc2;
uint16_t ext_mode=0;

uint16_t gn_test_step_delay;
uint32_t gn_test_tickcount;

uint8_t gb_adc_drdy_flag1, gb_adc_drdy_flag2;
uint8_t gb_ads114s08_drdy_done;
int32_t gn_ads114s08_adc_temp[CH_MAX];
int32_t gn_ads114s08_adc_temp_l[CH_MAX];

float cqic_trim_result[CH_MAX][3];
uint16_t gt_jig_check_vsync;

uint8_t duty = 5;	/* 60% */
uint8_t num_of_ch = CH_MAX;
uint8_t num_of_line = 14;
volatile uint8_t gt_jig_test_loop = 10;

/* save result value for OTP write */
uint16_t cqic_trimming_value[CH_MAX][2];	/* [CH][0] : 10bit DAC, [CH][1] : 12bit result */
uint16_t cqic_Vref_offset_value;		/* DAC1.5V trimming vaule */
uint16_t cqic_vmode_value;
uint16_t cqic_otp_checksum_value;		/* check value for otp write */

uint8_t cqic_test_mode;
extern uint16_t dac_1_5V_adc_count;
extern uint8_t adc_ch;
extern uint8_t min_max;
uint16_t int_cnt;
extern uint8_t gt_trim_retry_cnt;

uint8_t gn_vsync_to_data_delay;

#define VSYNC_95Hz 680700
#define VSYNC_100Hz 646666
#define VSYNC_105Hz 615872
uint8_t gn_vsync = 1; // 0 : 95Hz, 1 : 100Hz, 2 : 105Hz,
uint8_t gn_vsync_direction; // 0 : inc, 1 : dec

#define LDIM_POINT_SIZE 4
uint8_t gn_ldim_point;
const uint16_t gn_ldim_table[LDIM_POINT_SIZE] =
{
    0x000, 0x500, 0xA00, 0xFFF
};

#define DUTY_POINT_SIZE 4
uint8_t gn_duty_point;
const uint16_t gn_duty_table[DUTY_POINT_SIZE] =
{
    0x1010, 0x2020, 0x3030, 0x3838
};
const uint16_t gn_frame_header_table[DUTY_POINT_SIZE] =
{
    0x43FF, 0x57FF, 0x6BFF, 0x7FFF
};

#define CQ_REG_SET_SIZE 9
static uint16_t cqic_screen_setup_tbl[CQ_REG_SET_SIZE][2] =
{
    {0x8000, 0x0640},
    {0x8001, 0x0101},
    {0x8002, 0x1800},
    {0x8004, 0x09BA},
    {0x8005, 0x019C},
    {0x8010, 0x2FFF},
    {0x8017, 0x0FFF},
    {0x8019, 0xFFFF},
    {0x801A, 0x00FF},
};

static uint16_t cqic_screening_ldim_duty[CQ_DUTY_SIZE] =
{
      0x6FFF,
     /*   1       2       3       4       5       6       7       8*/
/* 1*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/* 2*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/* 3*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/* 4*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/* 5*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/* 6*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/* 7*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/* 8*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/* 9*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*10*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*11*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*12*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*13*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*14*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*15*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*16*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*17*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*18*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*19*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*20*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
/*21*/0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C, 0x0C0C,
};

static uint16_t cqic_screening_ldim_pwm[CQ_LDIM_SIZE] =
{
     /*   1       2       3       4       5       6       7       8*/
/* 1*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/* 2*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/* 3*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/* 4*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/* 5*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/* 6*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/* 7*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/* 8*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/* 9*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*10*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*11*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*12*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*13*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*14*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*15*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*16*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*17*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*18*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*19*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*20*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*21*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*22*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*23*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*24*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*25*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*26*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*27*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*28*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*29*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*30*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*31*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*32*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*33*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*34*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*35*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*36*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*37*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*38*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*39*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*40*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*41*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
/*42*/0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF,
};

uint16_t* get_duty_addr(void)
{
    return cqic_screening_ldim_duty;
}

uint16_t* get_ldim_addr(void)
{
    return cqic_screening_ldim_pwm;
}

static uint16_t cqic_function_test_tbl[29][6] =
{
	/* [Reg#][6] = {s1x6, s2x3, s3x2, d1x6, d2x3, d3x2} */

	/*0x00 FRAME_SIZE*/				{0x0E80, 0,0,0,0,0},
	/*0x01 FRAME_START_TIME*/		{0x0001, 0,0,0,0,0},
	/*0x02 CH_SIZE_SVSYNC_DELAY*/	{0x1209, 0,0,0,0,0},

	/*0x03 SVSYNC_OFFSET*/			{0x0000, 0,0,0,0,0},
	/*0x04 SVSYNC_PERIOD*/			{0x041A, 0,0,0,0,0},
	/*0x05 SHSYNC_PERIOD*/			{0x004B, /*0x0050,*/ 0,0,0,0,0},

	/*0x06 HOLD_SIZE_R1*/ 			{0x3434, 0,0,0,0,0},
	/*0x07 HOLD_SIZE_R2*/ 			{0x0000, 0,0,0,0,0},
	/*0x08 HOLD_SIZE_R3*/ 			{0x0000, 0,0,0,0,0},

	/*0x09 HOLD_DUMMY_SIZE_S1*/		{0x0208, 0,0,0,0,0},
	/*0x0A HOLD_DUMMY_SIZE_S2*/		{0x0000, 0,0,0,0,0},

	/*0x0B HOLD_SETUP*/				{0x002C,/*0x0018,*/ 0,0,0,0,0},
	/*0x0C HOLD_ON*/				{0x001D, 0,0,0,0,0},

	/*0x0D X_PERIOD,*/				{0x0010, 0,0,0,0,0},
	/*0x0E VSYNC_IN_COMP1*/ 		{0x0000,/*0x0000,*/ 0,0,0,0,0},
	/*0x0F VSYNC_IN_COMP2*/ 		{0x0000,/*0x0000,*/ 0,0,0,0,0},

	/*0x10 FRAME_HD_FIX */			{0x0000, 0,0,0,0,0},
	/*0x11 DUTY_FIX*/				{0x0000, 0,0,0,0,0},
	/*0x12 LD_I_FIX*/				{0x0000, /*0x0824,*/ 0,0,0,0,0},

	/*0x13 STB_DUP_EN*/				{0x00F0, 0,0,0,0,0},
	/*0x14 STB_DUP_CH_R1*/			{0x0000, 0,0,0,0,0},
	/*0x15 STB_DUP_CH_R2*/			{0x4321, 0,0,0,0,0},
	/*0x16 STB_DUP_CH_R3*/			{0x0000, 0,0,0,0,0},

	/*0x17 FULL_STEP_SIZE*/			{0x0FFF, 0,0,0,0,0},
	/*0x18 FULL_STEP_DT_TH*/		{/*0x0000*/0x80, 0,0,0,0,0},

	/*0x19 OUTPUT_CH_EN1,*/			{0xFFFF, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff},	/* all channel-24ch- enable */
	/*0x1A OUTPUT_CH_EN2,*/			{0x0003, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff},

	/*0x1B MCLK_LOCK_CNT_LSB,*/		{0x0D00, 0x49f0, 0x49f0, 0x49f0, 0x49f0, 0x49f0},	/* mclk 16.1280MHz, Vsync-In 120Hz */
	/*0X1C MCLK_LOCK_CNT_MSB,*/		{0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002}
};

void cqic_set_cmd2_all_reg(uint8_t mode);
void fill_frame_data(uint8_t duty, uint8_t num_of_line, uint8_t num_of_ch);
void HAL_GPIO_DRDY1_Callback(uint16_t GPIO_Pin);
void HAL_GPIO_DRDY2_Callback(uint16_t GPIO_Pin);
void set_ok_led(uint8_t en);
void set_ng_led(uint8_t en);
void set_trimming_led(uint8_t en);
static void cq24_target_write_cmd2(void);
static void cq24_target_write_cmd1(void);

extern void PlayBeep(beep_freq_t t_type, uint16_t n_time);
extern void dac_set_voltage(float f_voltage, uint8_t ch);
extern void cqic_write(uint8_t device_id, uint16_t* pData, uint16_t length);
extern void cqic_read(uint8_t device_id, uint16_t* cmd, uint16_t* pData, uint16_t length);
extern void aqic_mode_config(uint8_t mode);
extern void cqic_init(void);
extern float cqic_adc_to_voltage(float cnt, uint8_t source);

uint16_t gn_cq_write_target_cmd1_addr;
uint16_t gn_cq_write_target_cmd1_data;
bool gb_cq_write_cmd1_flag;
static void cq24_target_write_cmd1(void)
{
    if (gb_cq_write_cmd1_flag)
    {
	    cqic_cmd1_type cmd1 = {0, };

        cmd1.u.cmd_id = CMD_01;
        cmd1.u.rw = SPI_WR;
        cmd1.u.addr = gn_cq_write_target_cmd1_addr;
        cmd1.u.data = gn_cq_write_target_cmd1_data;

        cqic_write(CQIC_1, &cmd1.value, 2);
        gb_cq_write_cmd1_flag = false;
    }
}

uint16_t gn_cq_write_target_cmd2_addr;
uint16_t gn_cq_write_target_cmd2_data;
bool gb_cq_write_cmd2_flag;
static void cq24_target_write_cmd2(void)
{
    if (gb_cq_write_cmd2_flag)
    {
	    cqic_cmd2_type cmd2 = {0, };

        cmd2.u.cmd_id = CMD_02;
        cmd2.u.rw = SPI_WR;
        cmd2.u.addr = gn_cq_write_target_cmd2_addr;
        cmd2.u.data = gn_cq_write_target_cmd2_data;

        cqic_write(CQIC_1, cmd2.value, 2);
        gb_cq_write_cmd2_flag = false;
    }
}

uint16_t gn_cq_read_target_addr;
bool gb_cq_read_flag;
static void cq24_target_read(void)
{
    if (gb_cq_read_flag)
    {
	    cqic_cmd2_type cmd2 = {0, };
	    uint16_t rx_data[2] = {0, };

        cmd2.u.addr 	= gn_cq_read_target_addr;
        cmd2.u.rw		= SPI_RD;
        cmd2.u.cmd_id	= CMD_02;
        cmd2.u.data 	= 0;
        cqic_read(CQIC_1, cmd2.value, rx_data, 2);

		snprintf(msg_buffer, sizeof msg_buffer, "[0x%02X] - [0x%03X]\r\n", gn_cq_read_target_addr, rx_data[1]);
		print(msg_buffer);

        gb_cq_read_flag = false;
    }
}

void set_ok_led(uint8_t en)
{
  return;
#if 0
  if(en == ON)
    {
        HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_RESET);
    }
#endif
}

void set_ng_led(uint8_t en)
{
    return;

#if 0
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_NG_GPIO_Port, LED_NG_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_NG_GPIO_Port, LED_NG_Pin, GPIO_PIN_RESET);
    }
#endif
}

void set_trimming_led(uint8_t en)
{
    if(en == ON)
    {
        HAL_GPIO_WritePin(LED_TRIM_GPIO_Port, LED_TRIM_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_TRIM_GPIO_Port, LED_TRIM_Pin, GPIO_PIN_RESET);
    }
}

void cqic_set_cmd2_all_reg(uint8_t mode)
{
	uint8_t addr;
	cqic_cmd2_type cmd2;

	cmd2.value32 = 0;
	cmd2.u.cmd_id = CMD_02;
	cmd2.u.rw	  = SPI_WR;

#if 1
    for(uint8_t i = 0 ; i < CQ_REG_SET_SIZE ; ++i)
    {
        cqic_write(CQIC_1, cqic_screen_setup_tbl[i], 2);
        snprintf(msg_buffer, sizeof msg_buffer, "0x%04X 0x%04X\n\r", *(cqic_screen_setup_tbl[i]), *(cqic_screen_setup_tbl[i] + 1));
        print(msg_buffer);
    }
#else
	for(addr = 0; addr < 0x1C; addr++)
	{
		cmd2.u.addr   = addr;
		cmd2.u.data   = cqic_function_test_tbl[addr][0];
	}
#endif
}

void fill_frame_data(uint8_t duty, uint8_t num_of_line, uint8_t num_of_ch)
{
#if 1 //dual drive
	cqic_write(CQIC_1, cqic_screening_ldim_duty, 49+48/*CQ_DUTY_SIZE*/);
	cqic_write(CQIC_1, cqic_screening_ldim_pwm, 96+96/*CQ_LDIM_SIZE*/);
#else //single drive
	cqic_write(CQIC_1, cqic_screening_ldim_duty, CQ_DUTY_SIZE);
	cqic_write(CQIC_1, cqic_screening_ldim_pwm, CQ_LDIM_SIZE);
#endif
}

//uint8_t test_duty = 64;
uint8_t test_duty = 2;

void test_procedure_run(void)
{
	cqic_cmd1_type cmd1;
	cqic_cmd2_type cmd2;

	switch(gt_cqic_test_step)
	{
	case MODE_TEST_START:
		print("\n\r Test Mode Start\n\r");
		print("++++++++++++++++ Start CQIC Test procedure +++++++++++++++\n\r");

		print("CQIC Initialize...\n\r");
		cqic_init();

		set_ok_led(OFF);
		set_ng_led(OFF);
		set_trimming_led(ON);

		//print("Start Vsync timer\n\r");
		//HAL_TIM_Base_Start_IT(&htim2);	/* v_sync_in start */

		gt_cqic_test_step = MODE_TEST_STEP00_00;
		break;
	case MODE_TEST_STEP00_00:
		print("++++ test mode step 0-0\n\r");

		/* set command type 1 reg */
		cmd1.u.cmd_id	= CMD_01;
		cmd1.u.addr 	= MODE_CONTROL;
		cmd1.u.rw		= SPI_WR;
		cmd1.u.data 	= 0x0091;		/* EXT CLK:1, DRV_MODE:1 */
		cqic_write(CQIC_1, &cmd1.value, 1);
        snprintf(msg_buffer, sizeof msg_buffer, "0x%4X\n\r", cmd1.value);
        print(msg_buffer);

		/* set command type 1 reg */
		cmd1.u.cmd_id	= CMD_01;
		cmd1.u.addr 	= FIX_CONTROL;
		cmd1.u.rw		= SPI_WR;
		cmd1.u.data 	= 0x0400;		/* DUP_O:1 */
		cqic_write(CQIC_1, &cmd1.value, 1);
        snprintf(msg_buffer, sizeof msg_buffer, "0x%4X\n\r", cmd1.value);
        print(msg_buffer);

		/* set command type 2 reg */
		print("CQIC parameter setting...\n\r");
		cqic_set_cmd2_all_reg(cqic_test_mode);

        snprintf(msg_buffer, sizeof msg_buffer, "otp_read ---- after ----\n\r");
        print(msg_buffer);
        cqic_reg_read_all();

		/* got to next step */
		gt_cqic_test_step = MODE_TEST_STEP00_01;
		break;
	case MODE_TEST_STEP00_01:
		print("++++ test mode step 0-1\n\r");

		ext_mode = 3;
		//fill_frame_data(test_duty,num_of_line, num_of_ch);
		snprintf(msg_buffer, sizeof msg_buffer, "Start Vsync timer(%d)\n\r", ext_mode);
		print(msg_buffer);

		gt_jig_test_loop = 0;
		gt_jig_check_vsync = 0;

        __HAL_TIM_SET_AUTORELOAD(&htim2, VSYNC_100Hz);
		HAL_TIM_Base_Start_IT(&htim2);	/* v_sync_in start */

		gt_cqic_test_step = MODE_TEST_STEP00_02;
		break;
	case MODE_TEST_STEP00_02:
		print("++++ test mode step 0-2\n\r");

		/* set external interrupt */
		//gt_jig_check_cs = 0;
		//gt_jig_check_hold = 0;

		gt_cqic_test_step = MODE_TEST_STEP01_00;
		break;
	case MODE_TEST_STEP01_00:
		//print("++++ test mode step 1-0\n\r");
		/* check vsync int */
		if(gt_jig_check_vsync)
		{
			gt_jig_check_vsync = 0;
#if 1
            HAL_Delay(gn_vsync_to_data_delay);
			fill_frame_data(test_duty, num_of_line, num_of_ch);

            if(gt_jig_test_loop == 0)
            {
                set_trimming_led(OFF);
                gt_jig_test_loop = 1;
            }
            else
            {
                set_trimming_led(ON);
                gt_jig_test_loop = 0;
            }
#endif

#if 0
            if (gn_vsync_direction == 0)
            {
                ++gn_vsync;
                if (gn_vsync == 2)
                {
                    gn_vsync_direction = 1;
                }
            }
            else if (gn_vsync_direction == 1)
            {
                --gn_vsync;
                if (gn_vsync == 0)
                {
                    gn_vsync_direction = 0;
                }
            }
            switch (gn_vsync)
            {
                case 0 :
                    __HAL_TIM_SET_AUTORELOAD(&htim2, VSYNC_95Hz);
                    break;
                case 1 :
                    __HAL_TIM_SET_AUTORELOAD(&htim2, VSYNC_100Hz);
                    break;
                case 2 :
                    __HAL_TIM_SET_AUTORELOAD(&htim2, VSYNC_105Hz);
                    break;
            }
#endif

#if 0
            uint16_t* p_pwm_addr = get_ldim_addr();
            for (uint16_t i = 0 ; i < CQ_LDIM_SIZE ; ++i)
            {
                *(p_pwm_addr + i) = gn_ldim_table[gn_ldim_point];
                *(p_pwm_addr + i) = 0xC00;
            }
            ++gn_ldim_point;
            if (gn_ldim_point == LDIM_POINT_SIZE)
            {
                gn_ldim_point = 0;
            }

            uint16_t* p_duty_addr = get_duty_addr();
            //*p_duty_addr = gn_frame_header_table[gn_duty_point];
            for (uint16_t i = 0 ; i < CQ_DUTY_SIZE ; ++i)
            {
                *(p_duty_addr + i + 1) = gn_duty_table[gn_duty_point];
                *(p_duty_addr + i + 1) = 0x4040;
            }
            ++gn_duty_point;
            if (gn_duty_point == DUTY_POINT_SIZE)
            {
                gn_duty_point = 0;
            }
#endif
            cq24_target_write_cmd1();
            cq24_target_write_cmd2();
            cq24_target_read();
		}
		break;
	case MODE_TEST_RESULT:
		/* result display */
		gt_cqic_test_step = MODE_TEST_OK;
		break;
	case MODE_TEST_OK:
		set_ok_led(ON);
		set_ng_led(OFF);
		set_trimming_led(OFF);
		gt_cqic_test_step = TEST_MODE_NONE;
		break;
	case  MODE_TEST_ERROR:
		set_ok_led(OFF);
		set_ng_led(ON);
		set_trimming_led(OFF);
		gt_cqic_test_step = TEST_MODE_NONE;
		break;
	default:
	break;
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        HAL_ADC_Stop_DMA(hadc);

        gn_mode_temp_adc[CQIC_DAC_1_5V]+= gn_adc_dma_buffer[CQIC_DAC_1_5V];
        gn_mode_temp_adc[CQIC_MODE_1]  += gn_adc_dma_buffer[CQIC_MODE_1];
        gn_mode_temp_adc[CQIC_MODE_2]  += gn_adc_dma_buffer[CQIC_MODE_2];

        //snprintf(msg_buffer, sizeof msg_buffer, "ADC3 HAL_ADC_ConvCpltCallback() %2d : %3d\r\n", gt_jig_test_mode, gn_adc_read_count);
        //print(msg_buffer);

        gb_adc_start_dma = 0;
    }
}

void HAL_GPIO_DRDY1_Callback(uint16_t GPIO_Pin)
{
	if(ADC1_DRDY_Pin)
	{
		int32_t temp = 0;
#if 1
	 	if(gn_adc_read_count_adc1 == 0) return;

		temp = ads114s08_get_data(NULL, NULL, ADC1_CS);
#else
		temp = ads114s08_get_rdata(NULL, NULL, ADC1_CS);
#endif
		if(temp > 32767)
		{
			temp = ~temp + 1;	// Take 2's compliment if number is negative
			snprintf(msg_buffer, sizeof msg_buffer, "\n\r tMinus [%5d]", temp);
			print(msg_buffer);
		}

		if(gn_adc_read_count_adc1 > 0)
		{
			if((min_max == 0x00) || (min_max == 0x03))
            {
				gn_ads114s08_adc_temp[adc_ch] += temp;
            }
			else
            {
                gn_ads114s08_adc_temp_l[adc_ch] += temp;
            }
			gn_adc_read_count_adc1--;
		}

		if(gn_adc_read_count_adc1 == 0)
		{
			ads114s08_set_start(RESET, ADC1_CS);	/* stop continuous conversion */
			gb_adc_drdy_flag1 = 1;
		}
	}
}

void HAL_GPIO_DRDY2_Callback(uint16_t GPIO_Pin)
{
	if(ADC2_DRDY_Pin)
	{
		int32_t temp = 0;
#if 1
		if(gn_adc_read_count_adc2 == 0) return;

		temp = ads114s08_get_data(NULL, NULL, ADC2_CS);
#else
		temp = ads114s08_get_rdata(NULL, NULL, ADC1_CS);
#endif
		if(temp > 32767)
		{
			temp = ~temp + 1;	// Take 2's compliment if number is negative
			snprintf(msg_buffer, sizeof msg_buffer, "\n\r tMinus [%5d]", temp);
			print(msg_buffer);
		}

		if(gn_adc_read_count_adc2 > 0)
		{
			if((min_max == 0x00) || (min_max==0x03))
            {
                gn_ads114s08_adc_temp[adc_ch] += temp;
            }
			else
            {
				gn_ads114s08_adc_temp_l[adc_ch] += temp;
            }
			gn_adc_read_count_adc2--;
		}

		if(gn_adc_read_count_adc2==0)
		{
			ads114s08_set_start(RESET, ADC2_CS);	/* stop continuous conversion */
			gb_adc_drdy_flag2 = 1;
		}
	}
}

/*** end of file ***/