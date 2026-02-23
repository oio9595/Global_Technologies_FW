#ifndef CQIC_H
#define	CQIC_H

#ifdef __CQIC_C__
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef CQIC_H
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define ADC_114S08_STEP_V	(0.0002000f)	//(0.0001525f)
#define ADC_ST32F488_STEP_V	(0.0001525f)

#define DAC_1_5_STEP_V		(0.000650f)
#define DAC_2_5_STEP_V		DAC_1_5_STEP_V
#define DAC_4_5_STEP_V		(0.0018f)

#define DAC_1_5_TARGET_V 	(1.50f)	//(1.5f)
#define DAC_1_57_TARGET_V 	(1.57f)	//(1.5f)
#define DAC_2_5_TARGET_V 	(2.5f)
#define DAC_4_5_TARGET_V	(4.50f)		//(4.5f)

#if 0
#define VREF_OFFSET_STEP_V		(0.004395)
//#define DAC12_STEP_V		(0.001099)
#define DAC12_STEP_V		(0.00050)

#define DAC12_mSTEP_V		(0.0000687)
//#define DAC12_mSTEP_V		(0.00070)
#else
#define VREF_OFFSET_STEP_V	(0.0003f)	//(0.001099)
#define DAC12_STEP_V		(0.0003f)	//(0.001099)

#define DAC12_mSTEP_V		(0.0003f)	//(0.001099)
//#define DAC12_mSTEP_V		(0.00070)
#endif

#define DAC_MAX_TARGET_V 	(4.5f)
#define DAC_10B_T_MAX		(3.002f)
#define DAC_10B_T_MIN		(2.998f)
#define DAC_10B_T_DIFF_COEF	(2.0602f)
#define DAC_MIN_TARGET_V 	(0.5845f)	// method 2

#define Vmode_T				(2.5f)
#define Vmode_STEP			(0.030f)

enum
{
	DAC_VREF = 0,
	DAC_OFFSET
};

/* command type 1 Reg*/
enum
{
	SOFT_RESET	= 0,
	MODE_CONTROL,
	FIX_CONTROL,
	CS_HOLD_FIX
};


/* command type 2 Reg */
enum
{
	FRAME_SIZE				= 0,
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
	HOLD_SETUP,					//0x0B
	HOLD_ON,
	X_PERIOD,
	VSYNC_IN_COMP1,
	VSYNC_IN_COMP2,

	FRAME_HEADER,
	DUTY,
	LD_I,

	STB_DUP_EN,		//0x13
	STB_DUP_CH_R1,
	STB_DUP_CH_R2,
	STB_DUP_CH_R3,
	FULL_STEP_SIZE,
	FULL_STEP_DUTY,
	OUTPUT_CH_EN1,
	OUTPUT_CH_EN2,		//0x1A
	MCLK_LOCK_CNT_LSB,
	MCLK_LOCK_CNT_MSB,

	TRIM_CONTROL					= 0x30,
	OTP_WR_CONTROL,
	OTP_RD_PG_CONTROL,
	OTP_PROTECTION,

	OTP_CHECK_SUM					= 0x39,
	MODE_DAC_OFFSET				= 0x3A,
	V_ANA_GAIN						= 0x40,

	DAC_OFFSET_BASE				= 0x41,
	DAC_VREF_OFFSET_BASE 	= 0x59,
};

enum
{
  CH_01   = 0,
  CH_02,
  CH_03,
  CH_04,
  CH_05,
  CH_06,
  CH_07,
  CH_08,
  CH_09,
  CH_10,
  CH_11,
  CH_12,
  CH_13,
  CH_14,
  CH_15,
  CH_16,
  CH_17,
  CH_18,
  CH_19,
  CH_20,
  CH_21,
  CH_22,
  CH_23,
  CH_24,
  CH_MAX
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

enum
{
	CQIC_1	= 0,
	CQIC_2
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
	uint32_t value32;
	uint16_t value[2];
	struct
	{
		uint32_t addr		: 7;
		uint32_t dummy1		: 4;
		uint32_t rw			: 1;
		uint32_t dummy2		: 2;
		uint32_t cmd_id		: 2;

		uint32_t offset 	: 8;
		uint32_t sign		: 1;
		uint32_t dummy		: 7;
	}u;
}cqic_dac10b_type;

typedef union
{
	uint32_t value32;
	uint16_t value[2];
	struct
	{
		uint32_t addr		: 7;
		uint32_t dummy2		: 4;
		uint32_t rw			: 1;
		uint32_t dummy3		: 2;
		uint32_t cmd_id		: 2;

		uint32_t m_offset	: 8;
		uint32_t sign		: 1;
		uint32_t l_offset	: 4;
		uint32_t dummy1 	: 3;
	}u;
}cqic_dac12b_type;

typedef union
{
	uint64_t value64;
	uint16_t value[4];
	struct
	{
		uint64_t vref		:10;
		uint64_t cz			: 2;
		uint64_t dummy3		: 2;
		uint64_t cmd_id1	: 2;

		uint64_t duty_n1	: 8;
	    uint64_t duty_n		: 8;

		uint64_t ld_i		:12;
		uint64_t dummy2 	: 2;
		uint64_t cmd_id2	: 2;

		uint64_t dummy1 	:16;
	}frame;
}cqic_cmd3_type;

typedef union
{
	uint16_t value;
	struct
	{
		uint16_t vref		:12;
		uint16_t cz			: 2;
		uint16_t cmd_id		: 2;
	}u;
}cqic_cmd3_header_type;

typedef union
{
	uint16_t value;
	struct
	{
		uint16_t duty_n1	: 8;
	    uint16_t duty_n		: 8;
	}u;
}cqic_cmd3_duty_type;

typedef union
{
	uint16_t value;
	struct
	{
		uint16_t ld_i		:12;
		uint16_t dummy	 	: 2;
		uint16_t cmd_id		: 2;
	}u;
}cqic_cmd4_ldi_type;

static cqic_dac12b_type cqic_12b_dac_offset;

/* cqic command type 3 reg declaration  */
static cqic_cmd3_type dimm_value;



EXTERN void GUI_SEND_reg_all();
EXTERN void GUI_write_reg_cmd1(uint16_t in_addr, uint16_t in_val);
EXTERN void GUI_write_reg_cmd2(uint16_t in_addr, uint16_t in_val);
EXTERN void GUI_SEND_otp_written();
EXTERN void cqic_reg_read_all(void);

#define NUM_OF_REGISTER_CMD_1	0x3
#define NUM_OF_REGISTER_CMD_2	0x70
#define START_OF_OTP_REGISTER	0x41
#define END_OF_OTP_REGISTER		0x70


#endif
