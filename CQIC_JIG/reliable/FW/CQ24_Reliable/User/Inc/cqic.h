#ifndef CQIC_H
#define	CQIC_H

#define EXTERN extern

#define TRUE    1
#define FALSE   0

/* command type 1 Reg */
enum
{
	SOFT_RESET	    = 0,
	MODE_CONTROL,
	FIX_CONTROL,
	CS_HOLD_FIX
};

/* command type 2 Reg */
enum
{
	FRAME_SIZE             = 0x00,
	FRAME_START_TIME,
	CH_SIZE_SVSYNC_DELAY,
	SVSYNC_OFFSET,
	SVSYNC_PERIOD,
	SHSYNC_PERIOD,
	HOLD_SIZE1,
	HOLD_SIZE2,
	HOLD_SIZE3,
	HOLD_DUMMY_SIZE1,
	HOLD_DUMMY_SIZE2,
	HOLD_SETUP,
	HOLD_ON,
	X_PERIOD,
	VSYNC_IN_COMP1,
	VSYNC_IN_COMP2,

	FRAME_HEADER         = 0x10,
	DUTY,
	LD_I,
	STB_DUP_EN,
	STB_DUP_CH_R1,
	STB_DUP_CH_R2,
	STB_DUP_CH_R3,
	FULL_STEP_SIZE,
	FULL_STEP_DUTY,
	OUTPUT_CH_EN1,
	OUTPUT_CH_EN2,
	MCLK_LOCK_CNT_LSB,
	MCLK_LOCK_CNT_MSB,

	TRIM_CONTROL         = 0x30,
	OTP_WR_CONTROL,
	OTP_RD_PG_CONTROL,
	OTP_PROTECTION,
	OTP_CHECK_SUM        = 0x39,
	MODE_DAC_OFFSET      = 0x3A,
	V_ANA_GAIN           = 0x40,

	DAC_OFFSET_BASE      = 0x41,
	DAC_VREF_OFFSET_BASE = 0x59,
};

enum
{
    SPI_WR   = 0,
    SPI_RD
};

enum
{
    CMD_04   = 0,
    CMD_03,
    CMD_02,
    CMD_01
};

/* cqic command format */
typedef union
{
	uint16_t value;
	struct
	{
		uint16_t data 		:11;
		uint16_t rw 		: 1;
		uint16_t addr		: 2;
		uint16_t cmd_id		: 2;
	}u;
}cqic_cmd1_type;

typedef union
{
	uint32_t value32;
	uint16_t value[2];
	struct
	{
		uint32_t addr		: 7;
		uint32_t dummy1		: 4;
		uint32_t rw			: 1;
		uint32_t dummy2		: 2;
		uint32_t cmd_id		: 2;
		uint32_t data		:16;
	}u;
}cqic_cmd2_type;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t rst1       : 1;
        uint16_t rst2       : 1;
        uint16_t rst3       : 1;
        uint16_t            : 5;
        uint16_t broadcast  : 1;
        uint16_t            : 7;
    }u;
}_r00_soft_reset_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t drv_mode   : 1;
        uint16_t            : 3;
        uint16_t normal     : 1;
        uint16_t x8         : 1;
        uint16_t x2         : 1;
        uint16_t frame_keep : 1;
        uint16_t vs_comp_en : 1;
        uint16_t vs_ft_en   : 1;
        uint16_t int_clk    : 1;
        uint16_t            : 5;
    }u;
}_r01_mode_control_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t cs_hd_fix  : 1;
        uint16_t            : 1;
        uint16_t hold_out   : 1;
        uint16_t cs_out     : 1;
        uint16_t header_fix : 1;
        uint16_t duty_fix   : 1;
        uint16_t ld_i_fix   : 1;
        uint16_t            : 1;
        uint16_t ld_force   : 1;
        uint16_t            : 1;
        uint16_t dup_o      : 1;
        uint16_t            : 5;
    }u;
}_r02_fix_control_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t fix_ch1    : 1;
        uint16_t fix_ch2    : 1;
        uint16_t fix_ch3    : 1;
        uint16_t fix_ch4    : 1;
        uint16_t fix_ch5    : 1;
        uint16_t fix_ch6    : 1;
        uint16_t fix_ch7    : 1;
        uint16_t fix_ch8    : 1;
        uint16_t fix_ch9    : 1;
        uint16_t fix_ch10   : 1;
        uint16_t            : 6;
    }u;
}_r03_cs_hold_fix_channel_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t frame_vref :12;
        uint16_t frame_cz   : 2;
        uint16_t            : 2;
    }u;
}_r10_frame_header_fix_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t duty_fix   : 8;
        uint16_t            : 8;
    }u;
}_r11_duty_fix_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ld_i_fix   :12;
        uint16_t            : 4;
    }u;
}_r12_ld_i_fix_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ch1    : 1;
        uint16_t ch2    : 1;
        uint16_t ch3    : 1;
        uint16_t ch4    : 1;
        uint16_t ch5    : 1;
        uint16_t ch6    : 1;
        uint16_t ch7    : 1;
        uint16_t ch8    : 1;
        uint16_t ch9    : 1;
        uint16_t ch10   : 1;
        uint16_t ch11   : 1;
        uint16_t ch12   : 1;
        uint16_t ch13   : 1;
        uint16_t ch14   : 1;
        uint16_t ch15   : 1;
        uint16_t ch16   : 1;
    }u;
}_r19_output_channel_1_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ch17    : 1;
        uint16_t ch18    : 1;
        uint16_t ch19    : 1;
        uint16_t ch20    : 1;
        uint16_t ch21    : 1;
        uint16_t ch22    : 1;
        uint16_t ch23    : 1;
        uint16_t ch24    : 1;
        uint16_t         : 8;
    }u;
}_r1A_output_channel_2_t;

typedef struct
{
    _r00_soft_reset_t cmd1_r00;
    _r01_mode_control_t cmd1_r01;
    _r02_fix_control_t cmd1_r02;
    _r03_cs_hold_fix_channel_t cmd1_r03;
}_cmd1_reg_t_;

typedef struct
{
    _r10_frame_header_fix_t cmd2_r10;
    _r11_duty_fix_t cmd2_r11;
    _r12_ld_i_fix_t cmd2_r12;
    _r19_output_channel_1_t cmd2_r19;
    _r1A_output_channel_2_t cmd2_r1A;
}_cmd2_reg_t_;

typedef enum
{
    CZ_0V5 = 0,
    CZ_1V5,
    CZ_2V5,
    CZ_3V5,
}_cz_t_;

extern float gf_cq24_ado_voltage;

static void spi_write(uint16_t* pData, uint16_t length);
static void spi_read(uint16_t* pTxData, uint16_t* pRxData, uint16_t length);
static void cqic_write(uint16_t* pData, uint16_t length);
static void cqic_read(uint16_t* cmd, uint16_t* pData, uint16_t length);

static void cqic_output_enable();
static void cqic_output_disable();
static void cqic_set_vref(uint16_t in_vref);
static void cqic_set_cz(uint8_t in_cz);
static void cqic_set_duty_full();

void cqic_init(void);
void cqic_set_ado(float in_f_ado_voltage);
void dump_cq24_reg();


#endif
