#ifndef _DRV_CQIC_H
#define	_DRV_CQIC_H

extern bool gb_cq24_vsync_flag;

extern void CQ24_Set_CMD1_Target_Reg(uint8_t cq24_rw, uint8_t cmd1_addr, uint16_t cmd1_data);
extern void CQ24_Set_CMD2_Target_Reg(uint8_t cq24_rw, uint8_t cmd2_addr, uint16_t cmd2_data);

extern void CQ24_Set_CMD3_Duty(uint16_t duty);
extern void CQ24_Set_CMD4_LD_I(uint16_t ld_i);

extern void CQ24_Init(void);
extern void CQ24_Vsync_Task(void);

#define SVSYNC_SIZE     (64)
#define SHSYNC_SIZE     (6)

#define LINE_SIZE       (SHSYNC_SIZE * 2) //dual drive
#define CH_SIZE         (24)
#define BLOCK_SIZE      (LINE_SIZE * CH_SIZE)

#define DUTY_SIZE      (BLOCK_SIZE / 2)
#define LD_I_SIZE      (BLOCK_SIZE)

/* command type 1 Reg*/
typedef enum
{
	CQ24_CMD1_SOFT_RESET    = 0,
	CQ24_CMD1_MODE_CONTROL,
	CQ24_CMD1_FIX_CONTROL,
	CQ24_CMD1_CS_HOLD_FIX,
    CQ24_CMD1_REG_ADDR_MAX,
}CQ24_CMD1_REG_ADDR;

/* command type 2 Reg */
typedef enum
{
	CQ24_CMD2_FRAME_SIZE            = 0,
	CQ24_CMD2_FRAME_START_TIME,
	CQ24_CMD2_CH_SIZE_SVSYNC_DELAY,
	CQ24_CMD2_SVSYNC_OFFSET,
	CQ24_CMD2_SVSYNC_PERIOD,
	CQ24_CMD2_SHSYNC_PERIOD,
	CQ24_CMD2_HOLD_SIZE1,
	CQ24_CMD2_HOLD_SIZE2,
	CQ24_CMD2_HOLD_SIZE3,
	CQ24_CMD2_HOLD_DUMMY_SIZE1,
	CQ24_CMD2_HOLD_DUMMY_SIZE2,
	CQ24_CMD2_HOLD_SETUP_TIME,
	CQ24_CMD2_HOLD_ON_TIME,
	CQ24_CMD2_X_PERIOD,
	CQ24_CMD2_VSYNC_COMP_TIME1,
	CQ24_CMD2_VSYNC_COMP_TIME2,

	CQ24_CMD2_FRAME_HEADER_FIX,
	CQ24_CMD2_DUTY_FIX,
	CQ24_CMD2_LD_I_FIX,
	CQ24_CMD2_STB_DUP_CHANNEL_EN,
	CQ24_CMD2_STB_DUP_CH_GROUP1,
	CQ24_CMD2_STB_DUP_CH_GROUP2,
	CQ24_CMD2_STB_DUP_CH_GROUP3,
	CQ24_CMD2_FULL_STEP_SIZE,
	CQ24_CMD2_FULL_STEP_DUTY_TH,
	CQ24_CMD2_OUTPUT_CH_EN1,
	CQ24_CMD2_OUTPUT_CH_EN2,
	CQ24_CMD2_MCLK_LOCK_CNT1,
	CQ24_CMD2_MCLK_LOCK_CNT2,

	CQ24_CMD2_TRIM_CONTROL          = 0x30,
	CQ24_CMD2_OTP_WR_CONTROL,
	CQ24_CMD2_OTP_RD_PG_CONTROL,
	CQ24_CMD2_OTP_PROTECTION,
	CQ24_CMD2_OTP_CHECK_SUM         = 0x39,

	CQ24_CMD2_MODE_DAC_OFFSET       = 0x3A,
	CQ24_CMD2_V_ANA_GAIN            = 0x40,

	CQ24_CMD2_DAC_OFFSET_CH_BASE    = 0x41,
    CQ24_CMD2_VREF_OFFSET_CH_BASE   = 0x59,

    CQ24_CMD2_REG_ADDR_MAX = 0x71,
}CQ24_CMD2_REG_ADDR;

typedef enum
{
    CQ24_CH_01 = 0,
    CQ24_CH_02,
    CQ24_CH_03,
    CQ24_CH_04,
    CQ24_CH_05,
    CQ24_CH_06,
    CQ24_CH_07,
    CQ24_CH_08,
    CQ24_CH_09,
    CQ24_CH_10,
    CQ24_CH_11,
    CQ24_CH_12,
    CQ24_CH_13,
    CQ24_CH_14,
    CQ24_CH_15,
    CQ24_CH_16,
    CQ24_CH_17,
    CQ24_CH_18,
    CQ24_CH_19,
    CQ24_CH_20,
    CQ24_CH_21,
    CQ24_CH_22,
    CQ24_CH_23,
    CQ24_CH_24,
    CQ24_CH_MAX
}CQ24_CH;

typedef enum
{
    CQ24_WR = 0,
    CQ24_RD
}CQ24_RW;

typedef enum
{
    CMD_04   = 0,
    CMD_03,
    CMD_02,
    CMD_01
}CQ24_CMD_TYPE;

/* cq24 command format */
typedef union
{
    uint16_t value;
    struct
    {
        uint16_t data   :11;
        uint16_t rw     : 1;
        uint16_t addr   : 2;
        uint16_t cmd_id : 2;
    }u;
}cq24_cmd1_type;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t rst1       : 1;
        uint16_t rst2 		: 1;
        uint16_t rst3 		: 1;
        uint16_t 		    : 5;
        uint16_t broadcast  : 1;
        uint16_t            : 7;
    }u;
}cq24_cmd1_soft_reset_t;

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
}cq24_cmd1_mode_control_t;

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
}cq24_cmd1_fix_control_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t fix_ch01   : 1;
        uint16_t fix_ch02   : 1;
        uint16_t fix_ch03   : 1;
        uint16_t fix_ch04   : 1;
        uint16_t fix_ch05   : 1;
        uint16_t fix_ch06   : 1;
        uint16_t fix_ch07   : 1;
        uint16_t fix_ch08   : 1;
        uint16_t fix_ch09   : 1;
        uint16_t fix_ch10   : 1;
        uint16_t            : 6;
    }u;
}cq24_cmd1_cs_hold_fix_channel_t;

typedef struct
{
    cq24_cmd1_soft_reset_t          _r00;
    cq24_cmd1_mode_control_t        _r01;
    cq24_cmd1_fix_control_t         _r02;
    cq24_cmd1_cs_hold_fix_channel_t _r03;
}cq24_cmd1_regs;

typedef union
{
    uint32_t value32;
    uint16_t value[2];
    struct
    {
        uint32_t addr   : 7;
        uint32_t        : 4;
        uint32_t rw     : 1;
        uint32_t        : 2;
        uint32_t cmd_id : 2;
        uint32_t data   :16;
    }u;
}cq24_cmd2_type;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t svsync_size    : 8;
        uint16_t shsync_size    : 6;
        uint16_t                : 2;
    }u;
}cq24_cmd2_frame_size_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t svsync_start   : 7;
        uint16_t                : 1;
        uint16_t shsync_start   : 7;
        uint16_t                : 1;
    }u;
}cq24_cmd2_frame_start_time_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t svsync_delay   : 5;
        uint16_t                : 3;
        uint16_t output_ch_size : 5;
        uint16_t                : 3;
    }u;
}cq24_cmd2_ch_size_svsync_delay_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t svsync_offset  :15;
        uint16_t                : 1;
    }u;
}cq24_cmd2_svsync_offset_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t svsync_period  :15;
        uint16_t                : 1;
    }u;
}cq24_cmd2_svsync_period_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t shsync_period  :15;
        uint16_t                : 1;
    }u;
}cq24_cmd2_shsync_period_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t hold01_size    : 4;
        uint16_t hold02_size    : 4;
        uint16_t hold03_size    : 4;
        uint16_t hold04_size    : 4;
    }u;
}cq24_cmd2_hold_size1_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t hold05_size    : 4;
        uint16_t hold06_size    : 4;
        uint16_t hold07_size    : 4;
        uint16_t hold08_size    : 4;
    }u;
}cq24_cmd2_hold_size2_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t hold09_size    : 4;
        uint16_t hold10_size    : 4;
        uint16_t                : 8;
    }u;
}cq24_cmd2_hold_size3_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t hold01_dummy_size  : 3;
        uint16_t hold02_dummy_size  : 3;
        uint16_t hold03_dummy_size  : 3;
        uint16_t hold04_dummy_size  : 3;
        uint16_t hold05_dummy_size  : 3;
        uint16_t                    : 1;
    }u;
}cq24_cmd2_hold_dummy_size1_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t hold06_dummy_size  : 3;
        uint16_t hold07_dummy_size  : 3;
        uint16_t hold08_dummy_size  : 3;
        uint16_t hold09_dummy_size  : 3;
        uint16_t hold10_dummy_size  : 3;
        uint16_t                    : 1;
    }u;
}cq24_cmd2_hold_dummy_size2_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t hold_su    :10;
        uint16_t            : 6;
    }u;
}cq24_cmd2_hold_setup_time_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t hold_on    : 9;
        uint16_t            : 7;
    }u;
}cq24_cmd2_hold_on_time_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t x_period   : 7;
        uint16_t            : 9;
    }u;
}cq24_cmd2_x_period_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t vsync_comp1    :12;
        uint16_t                : 4;
    }u;
}cq24_cmd2_vsync_comp_time1_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t vsync_comp2    :10;
        uint16_t                : 6;
    }u;
}cq24_cmd2_vsync_comp_time2_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t frame_vref :12;
        uint16_t frame_cz   : 2;
        uint16_t            : 2;
    }u;
}cq24_cmd2_frame_header_fix_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t duty_fix   : 8;
        uint16_t            : 8;
    }u;
}cq24_cmd2_duty_fix_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ld_i_fix   :12;
        uint16_t            : 4;
    }u;
}cq24_cmd2_ld_i_fix_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ch01   : 1;
        uint16_t ch02   : 1;
        uint16_t ch03   : 1;
        uint16_t ch04   : 1;
        uint16_t ch05   : 1;
        uint16_t ch06   : 1;
        uint16_t ch07   : 1;
        uint16_t ch08   : 1;
        uint16_t ch09   : 1;
        uint16_t ch10   : 1;
        uint16_t        : 6;
    }u;
}cq24_cmd2_stb_dup_channel_en_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t stb_dup_ch01   : 4;
        uint16_t stb_dup_ch02   : 4;
        uint16_t stb_dup_ch03   : 4;
        uint16_t stb_dup_ch04   : 4;
    }u;
}cq24_cmd2_stb_dup_ch_group1_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t stb_dup_ch05   : 4;
        uint16_t stb_dup_ch06   : 4;
        uint16_t stb_dup_ch07   : 4;
        uint16_t stb_dup_ch08   : 4;
    }u;
}cq24_cmd2_stb_dup_ch_group2_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t stb_dup_ch09   : 4;
        uint16_t stb_dup_ch10   : 4;
        uint16_t                : 8;
    }u;
}cq24_cmd2_stb_dup_ch_group3_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t full_step_size :12;
        uint16_t                : 4;
    }u;
}cq24_cmd2_full_step_size_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t full_step_duty_th  : 8;
        uint16_t                    : 8;
    }u;
}cq24_cmd2_full_step_duty_threshold_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ch01   : 1;
        uint16_t ch02   : 1;
        uint16_t ch03   : 1;
        uint16_t ch04   : 1;
        uint16_t ch05   : 1;
        uint16_t ch06   : 1;
        uint16_t ch07   : 1;
        uint16_t ch08   : 1;
        uint16_t ch09   : 1;
        uint16_t ch10   : 1;
        uint16_t ch11   : 1;
        uint16_t ch12   : 1;
        uint16_t ch13   : 1;
        uint16_t ch14   : 1;
        uint16_t ch15   : 1;
        uint16_t ch16   : 1;
    }u;
}cq24_cmd2_output_channel_en1_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ch17   : 1;
        uint16_t ch18   : 1;
        uint16_t ch19   : 1;
        uint16_t ch20   : 1;
        uint16_t ch21   : 1;
        uint16_t ch22   : 1;
        uint16_t ch23   : 1;
        uint16_t ch24   : 1;
        uint16_t        : 8;
    }u;
}cq24_cmd2_output_channel_en2_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t mclk_lock_cnt  :16;
    }u;
}cq24_cmd2_mclk_lock_counter1_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t mclk_lock_cnt  : 5;
        uint16_t                :11;
    }u;
}cq24_cmd2_mclk_lock_counter2_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_rd_re_cycle        : 4;
        uint16_t otp_pg_access_cycle    :12;
    }u;
}cq24_cmd2_trim_control_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_wsel   : 4;
        uint16_t            :12;
    }u;
}cq24_cmd2_otp_wr_control_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_pg_start   : 1;
        uint16_t otp_rd_start   : 1;
        uint16_t                :14;
    }u;
}cq24_cmd2_otp_rd_pg_control_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_protect    :16;
    }u;
}cq24_cmd2_otp_protection_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_crc_checksum   :16;
    }u;
}cq24_cmd2_otp_crc_checksum_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t mode_ofs   : 6;
        uint16_t            :10;
    }u;
}cq24_cmd2_mode_dac_offset_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t v_ana_gain :10;
        uint16_t            : 6;
    }u;
}cq24_cmd2_v_ana_gain_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t dac_ofs_ch     : 8;
        uint16_t sign           : 1;
        uint16_t dac_lofs_ch    : 4;
        uint16_t                : 3;
    }u;
}cq24_cmd2_dac_offset_channel_base_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t vref_ofs_ch    : 8;
        uint16_t sign           : 1;
        uint16_t vref_lofs_ch   : 4;
        uint16_t                : 3;
    }u;
}cq24_cmd2_vref_offset_channel_base_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t reserved   :16;
    }u;
}cq24_cmd2_dummy_t;

typedef struct
{
    cq24_cmd2_frame_size_t                  _r00;
    cq24_cmd2_frame_start_time_t            _r01;
    cq24_cmd2_ch_size_svsync_delay_t        _r02;
    cq24_cmd2_svsync_offset_t               _r03;
    cq24_cmd2_svsync_period_t               _r04;
    cq24_cmd2_shsync_period_t               _r05;
    cq24_cmd2_hold_size1_t                  _r06;
    cq24_cmd2_hold_size2_t                  _r07;
    cq24_cmd2_hold_size3_t                  _r08;
    cq24_cmd2_hold_dummy_size1_t            _r09;
    cq24_cmd2_hold_dummy_size2_t            _r0A;
    cq24_cmd2_hold_setup_time_t             _r0B;
    cq24_cmd2_hold_on_time_t                _r0C;
    cq24_cmd2_x_period_t                    _r0D;
    cq24_cmd2_vsync_comp_time1_t            _r0E;
    cq24_cmd2_vsync_comp_time2_t            _r0F;

    cq24_cmd2_frame_header_fix_t            _r10;
    cq24_cmd2_duty_fix_t                    _r11;
    cq24_cmd2_ld_i_fix_t                    _r12;
    cq24_cmd2_stb_dup_channel_en_t          _r13;
    cq24_cmd2_stb_dup_ch_group1_t           _r14;
    cq24_cmd2_stb_dup_ch_group2_t           _r15;
    cq24_cmd2_stb_dup_ch_group3_t           _r16;
    cq24_cmd2_full_step_size_t              _r17;
    cq24_cmd2_full_step_duty_threshold_t    _r18;
    cq24_cmd2_output_channel_en1_t          _r19;
    cq24_cmd2_output_channel_en2_t          _r1A;
    cq24_cmd2_mclk_lock_counter1_t          _r1B;
    cq24_cmd2_mclk_lock_counter2_t          _r1C;
    cq24_cmd2_dummy_t                       _r1D;
    cq24_cmd2_dummy_t                       _r1E;
    cq24_cmd2_dummy_t                       _r1F;

    cq24_cmd2_dummy_t                       _r20;
    cq24_cmd2_dummy_t                       _r21;
    cq24_cmd2_dummy_t                       _r22;
    cq24_cmd2_dummy_t                       _r23;
    cq24_cmd2_dummy_t                       _r24;
    cq24_cmd2_dummy_t                       _r25;
    cq24_cmd2_dummy_t                       _r26;
    cq24_cmd2_dummy_t                       _r27;
    cq24_cmd2_dummy_t                       _r28;
    cq24_cmd2_dummy_t                       _r29;
    cq24_cmd2_dummy_t                       _r2A;
    cq24_cmd2_dummy_t                       _r2B;
    cq24_cmd2_dummy_t                       _r2C;
    cq24_cmd2_dummy_t                       _r2D;
    cq24_cmd2_dummy_t                       _r2E;
    cq24_cmd2_dummy_t                       _r2F;

    cq24_cmd2_trim_control_t                _r30;
    cq24_cmd2_otp_wr_control_t              _r31;
    cq24_cmd2_otp_rd_pg_control_t           _r32;
    cq24_cmd2_otp_protection_t              _r33;
    cq24_cmd2_dummy_t                       _r34;
    cq24_cmd2_dummy_t                       _r35;
    cq24_cmd2_dummy_t                       _r36;
    cq24_cmd2_dummy_t                       _r37;
    cq24_cmd2_dummy_t                       _r38;
    cq24_cmd2_otp_crc_checksum_t            _r39;
    cq24_cmd2_mode_dac_offset_t             _r3A;
    cq24_cmd2_dummy_t                       _r3B;
    cq24_cmd2_dummy_t                       _r3C;
    cq24_cmd2_dummy_t                       _r3D;
    cq24_cmd2_dummy_t                       _r3E;
    cq24_cmd2_dummy_t                       _r3F;

    cq24_cmd2_v_ana_gain_t                  _r40;
    cq24_cmd2_dac_offset_channel_base_t     _r41;
    cq24_cmd2_dac_offset_channel_base_t     _r42;
    cq24_cmd2_dac_offset_channel_base_t     _r43;
    cq24_cmd2_dac_offset_channel_base_t     _r44;
    cq24_cmd2_dac_offset_channel_base_t     _r45;
    cq24_cmd2_dac_offset_channel_base_t     _r46;
    cq24_cmd2_dac_offset_channel_base_t     _r47;
    cq24_cmd2_dac_offset_channel_base_t     _r48;
    cq24_cmd2_dac_offset_channel_base_t     _r49;
    cq24_cmd2_dac_offset_channel_base_t     _r4A;
    cq24_cmd2_dac_offset_channel_base_t     _r4B;
    cq24_cmd2_dac_offset_channel_base_t     _r4C;
    cq24_cmd2_dac_offset_channel_base_t     _r4D;
    cq24_cmd2_dac_offset_channel_base_t     _r4E;
    cq24_cmd2_dac_offset_channel_base_t     _r4F;

    cq24_cmd2_dac_offset_channel_base_t     _r50;
    cq24_cmd2_dac_offset_channel_base_t     _r51;
    cq24_cmd2_dac_offset_channel_base_t     _r52;
    cq24_cmd2_dac_offset_channel_base_t     _r53;
    cq24_cmd2_dac_offset_channel_base_t     _r54;
    cq24_cmd2_dac_offset_channel_base_t     _r55;
    cq24_cmd2_dac_offset_channel_base_t     _r56;
    cq24_cmd2_dac_offset_channel_base_t     _r57;
    cq24_cmd2_dac_offset_channel_base_t     _r58;
    cq24_cmd2_vref_offset_channel_base_t    _r59;
    cq24_cmd2_vref_offset_channel_base_t    _r5A;
    cq24_cmd2_vref_offset_channel_base_t    _r5B;
    cq24_cmd2_vref_offset_channel_base_t    _r5C;
    cq24_cmd2_vref_offset_channel_base_t    _r5D;
    cq24_cmd2_vref_offset_channel_base_t    _r5E;
    cq24_cmd2_vref_offset_channel_base_t    _r5F;

    cq24_cmd2_vref_offset_channel_base_t    _r60;
    cq24_cmd2_vref_offset_channel_base_t    _r61;
    cq24_cmd2_vref_offset_channel_base_t    _r62;
    cq24_cmd2_vref_offset_channel_base_t    _r63;
    cq24_cmd2_vref_offset_channel_base_t    _r64;
    cq24_cmd2_vref_offset_channel_base_t    _r65;
    cq24_cmd2_vref_offset_channel_base_t    _r66;
    cq24_cmd2_vref_offset_channel_base_t    _r67;
    cq24_cmd2_vref_offset_channel_base_t    _r68;
    cq24_cmd2_vref_offset_channel_base_t    _r69;
    cq24_cmd2_vref_offset_channel_base_t    _r6A;
    cq24_cmd2_vref_offset_channel_base_t    _r6B;
    cq24_cmd2_vref_offset_channel_base_t    _r6C;
    cq24_cmd2_vref_offset_channel_base_t    _r6D;
    cq24_cmd2_vref_offset_channel_base_t    _r6E;
    cq24_cmd2_vref_offset_channel_base_t    _r6F;

    cq24_cmd2_vref_offset_channel_base_t    _r70;
}cq24_cmd2_regs;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t frame_vref :12;
        uint16_t frame_cz   : 2;
        uint16_t cmd_id     : 2;
    }u;
}cq24_cmd3_type;

typedef struct
{
    cq24_cmd3_type cmd3;
    uint16_t duty[DUTY_SIZE];
}cq24_cmd3_duty_transfer;

typedef struct
{
    uint16_t ld_i[LD_I_SIZE];
}cq24_cmd4_ld_i_transfer;

#endif