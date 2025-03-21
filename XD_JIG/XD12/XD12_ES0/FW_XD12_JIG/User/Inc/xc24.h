/** @file xd12.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XC24_H__
#define __XC24_H__

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __XC24_C__ 
    #define EXTERN
#else
    #define EXTERN extern
#endif

#ifdef __cplusplus
 extern "C" {
#endif

#define XC24_ADDR_SOFT_RESET                    0x00
#define XC24_ADDR_GLOBAL_WRITE_COMMAND          0x01
#define XC24_ADDR_LOCAL_READ                    0x03
#define XC24_ADDR_ID_GEN_COMMAND                0x04
#define XC24_ADDR_SYNC_GEN_COMMAND              0x07
#define XC24_ADDR_LOCAL_WR_TRANSFER_POINTER     0x0E
#define XC24_ADDR_LOCAL_RD_RECEIVE_POINTER      0x0F
#define XC24_ADDR_LOCAL_RW_POINTER_RESET        0x11
#define XC24_ADDR_SERIALIZER_CLOCK_GEN          0x14
#define XC24_ADDR_DAISY_SIZE1                   0x30
#define XC24_ADDR_BLOCK_SIZE1                   0x38
#define XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE   0x44
#define XC24_ADDR_CHANNEL_ENABLE_1              0x45
#define XC24_ADDR_CHANNEL_ENABLE_2              0x46
#define XC24_ADDR_GLOBAL_WRITE_DATA             0x60
#define XC24_ADDR_PORT1_LOCAL_RW_DATA1          0x70
#define XC24_ADDR_MAX                           0xF0

#define XC24_COMMAND_CODE_REGISTER_READ         0x01
#define XC24_COMMAND_CODE_REGISTER_WRITE        0x02

enum
{
    XC_CH_01   = 0,
    XC_CH_02,  
    XC_CH_03,
    XC_CH_04,  
    XC_CH_05,
    XC_CH_06,  
    XC_CH_07,
    XC_CH_08,  
    XC_CH_09,
    XC_CH_10,
    XC_CH_11,
    XC_CH_12,
    XC_CH_13,
    XC_CH_14,
    XC_CH_15,
    XC_CH_16,
    XC_CH_17,
    XC_CH_18,
    XC_CH_19,
    XC_CH_20,
    XC_CH_21,
    XC_CH_22,
    XC_CH_23,
    XC_CH_24,
    XC_CH_MAX
};

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t burst_size    	: 6;
        uint16_t addr       	: 8;
		uint16_t command_code   : 2;
	}u;
}xc24_command_format;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t rst_1      : 1;
		uint16_t rst_2      : 1;
		uint16_t rst_3      : 1;
		uint16_t            : 1;
		uint16_t vs_rst1    : 1;
		uint16_t vs_rst2    : 1;
		uint16_t            : 12;
	}u;
}xc24_reg_soft_reset;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t            : 15;
		uint16_t enable     : 1;
	}u;
}xc24_reg_idgen;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t            : 15;
		uint16_t enable     : 1;
	}u;
}xc24_reg_sync_gen;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t daisy_size_ch1 : 5;
		uint16_t daisy_size_ch2 : 5;
        uint16_t daisy_size_ch3 : 5;
        uint16_t                : 1;
	}u;
}xc24_reg_daisy_size;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t block_size_ch1 : 8;   /* BLOCK_SIZE_CH1 */
        uint16_t block_size_ch2 : 8;   /* BLOCK_SIZE_CH2 */
    }u;
}xc24_reg_block_size;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t addr       : 6;
		uint16_t            : 9;
		uint16_t enable		: 1;
	}u;
}xc24_reg_write;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t addr       : 6;
		uint16_t            : 2;
		uint16_t ch_seg     : 2;
		uint16_t            : 5;
		uint16_t enable     : 1;
	}u;
}xc24_reg_read;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t pointer_rst1   : 1;
		uint16_t                : 7;
		uint16_t pointer_rst2   : 1;
		uint16_t                : 7;
	}u;
}xc24_reg_local_rw_pointer_rst;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t serial_clk_high    : 4;
		uint16_t serial_clk_low     : 3;
		uint16_t                    : 9;
	}u;
}xc24_reg_serializer_clock_gen;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t ch01_en            : 1;
		uint16_t ch02_en            : 1;
		uint16_t ch03_en            : 1;
		uint16_t ch04_en            : 1;
		uint16_t ch05_en            : 1;
		uint16_t ch06_en            : 1;
		uint16_t ch07_en            : 1;
		uint16_t ch08_en            : 1;
		uint16_t ch09_en            : 1;
		uint16_t ch10_en            : 1;
		uint16_t ch11_en            : 1;
		uint16_t ch12_en            : 1;
		uint16_t ch13_en            : 1;
		uint16_t ch14_en            : 1;
		uint16_t ch15_en            : 1;
		uint16_t ch16_en            : 1;
	}u;
}xc24_reg_channel_enable_1;

typedef union
{
    uint16_t value;
	struct
	{
		uint16_t ch17_en            : 1;
		uint16_t ch18_en            : 1;
		uint16_t ch19_en            : 1;
		uint16_t ch20_en            : 1;
		uint16_t ch21_en            : 1;
		uint16_t ch22_en            : 1;
		uint16_t ch23_en            : 1;
		uint16_t ch24_en            : 1;
		uint16_t ch_size    	    : 5;
		uint16_t                    : 1;
        uint16_t ld_width    	    : 2;
	}u;
}xc24_reg_channel_enable_2;


/* BEGIN - INTERFACE FUNCTIONS */
EXTERN bool IS_XC24(void);
EXTERN void USE_XC24(bool b_support);

EXTERN void XC24_IF_IdGen(void);
EXTERN void XC24_IF_SyncGen(void);
EXTERN void XC24_Initialize(void);
EXTERN void XC24_IF_Write_XD12(uint8_t in_XD12_addr, uint16_t in_XD12_data);
EXTERN uint16_t XC24_IF_Read_XD12(uint8_t in_XD12_addr);
/* END   - INTERFACE FUNCTIONS */



#ifdef DBG_TEST
EXTERN void _dbg_XC24_Detect(void); // JIG BD Init Test
#endif //DBG_TEST


#ifdef __cplusplus
}
#endif

#endif /* ~__XD12_H__ */

/*** end of file ***/


