/* 
 * File:   IC603_struct.h
 * Author: GT
 *
 * Created on 2026, 05 15
 * Revision : IC603 0515
 */

#ifndef _IC603_STRUCT_H_
#define	_IC603_STRUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define XCR_INTERNAL_MCLK   (50000000UL)    /* 50.0MHz */
#define XCR_MODEL_NAME      "IC603 ES0"

#define BAUD_HIGH_DURATION_MIN  (6U)            /* mclk */
#define BAUD_HIGH_DURATION_MAX  (63U)           /* mclk */
#define BAUD_LOW_DURATION_MIN   (3U)            /* mclk */
#define BAUD_LOW_DURATION_MAX   (32U)           /* mclk */

#define CMD_CODE0   (0U)    /* 0b00 : No operation */
#define CMD_CODE1   (1U)    /* 0b01 : Register-read */
#define CMD_CODE2   (2U)    /* 0b10 : Register-write */
#define CMD_CODE3   (3U)    /* 0b11 : Local dimming data transfer */

#define CH_SEG0     (0U)    /* XC24R CH01 ~ CH08 for Local-write/read command */
#define CH_SEG1     (1U)    /* XC24R CH09 ~ CH16 for Local-write/read command */
#define CH_SEG2     (2U)    /* XC24R CH17 ~ CH24 for Local-write/read command */

#define LD_WIDTH0   (0U)    /* 0b00, LD width - 24bit : PWM  8bit + PAM 16bit => PWM  8bit + PWM 14bit */
#define LD_WIDTH1   (1U)    /* 0b01, LD width - 24bit : PWM 12bit + PAM 12bit => PWM 10bit + PWM 11bit */
#define LD_WIDTH2   (2U)    /* 0b10, LD width - 24bit : PWM 12bit + PAM 112bit => PWM 10bit + PWM 12bit */
#define LD_WIDTH3   (3U)    /* 0b11, LD width - 16bit : 16bit => LD 16bit */

/* Command define */
typedef enum _xcr_addr_grp1_
{
    XCR_RESET = 0U,/*0x00*/
    XCR_GLOBAL_WRITE_COMMAND,/*0x01*/
    XCR_LOCAL_WRITE_COMMAND,/*0x02*/
    XCR_LOCAL_READ_COMMAND,/*0x03*/
    XCR_ID_GEN_COMMAND,/*0x04*/
    XCR_FAULT_READ_COMMAND,/*0x05*/
    XCR_LD_TRANSFER_COMMAND,/*0x06*/
    XCR_SYNC_GEN_COMMAND,/*0x07*/
    XCR_COMMAND_AUTO_ENABLE,/*0x08*/
    XCR_LD_WRITE_POINTER,/*0x09*/
    XCR_LD_READ_POINTER,/*0x0A*/
    XCR_LD_DIFFERENCE_POINTER,/*0x0B*/
    XCR_LD_START_POINTER_TH,/*0x0C*/
    XCR_LOCAL_WRITE_TRANSFER_POINTER,/*0x0D*/
    XCR_LOCAL_READ_RECEIVE_POINTER,/*0x0E*/
    XCR_LOCAL_RW_DIFFERENCE_POINTER,/*0x0F*/
    XCR_LOCAL_RW_POINTER_RESET,/*0x10*/
    XCR_FAULT_AUTO_READ_INTERVAL,/*0x11*/
    XCR_FAULT_AUTO_READ_EVENT,/*0x12*/
    XCR_INTERRUPT_ENABLE,/*0x13*/
    XCR_COMMAND_STATUS_1,/*0x14*/
    XCR_COMMAND_STATUS_2,/*0x15*/
    XCR_RECEIVE_STATUS,/*0x16*/
    XCR_INTERRUPT_STATUS,/*0x17*/
    XCR_ERR_STATUS,/*0x18*/
    XCR_CH_CRC_ERR_STATUS,/*0x19*/
    XCR_CH_TIMEOUT_ERR_STATUS,/*0x1A*/
    XCR_BIST_CONTROL_STATUS,/*0x1B*/
    XCR_COMMUNICATION_FAULT_CONTROL,/*0x1C*/
    XCR_CLK_CONTROL_1,/*0x1D*/
    XCR_CLK_CONTROL_2,/*0x1E*/
    XCR_SERIALIZER_CLOCK_GEN,/*0x1F*/
    XCR_LATENCY,/*0x20*/
    XCR_TIMEOUT,/*0x21*/
    XCR_DAISIED_DEVICE_CH_SIZE,/*0x22*/
    XCR_DAISY_SIZE_1,/*0x23*/
    XCR_DAISY_SIZE_2,/*0x24*/
    XCR_DAISY_SIZE_3,/*0x25*/
    XCR_DAISY_SIZE_4,/*0x26*/
    XCR_DAISY_SIZE_5,/*0x27*/
    XCR_DAISY_SIZE_6,/*0x28*/
    //XCR_DAISY_SIZE_7,/*0x27*/
    //XCR_DAISY_SIZE_8,/*0x28*/
    XCR_BLOCK_SIZE_1,/*0x29*/
    XCR_BLOCK_SIZE_2,/*0x2A*/
    XCR_BLOCK_SIZE_3,/*0x2B*/
    XCR_BLOCK_SIZE_4,/*0x2C*/
    XCR_BLOCK_SIZE_5,/*0x2D*/
    XCR_BLOCK_SIZE_6,/*0x2E*/
    XCR_BLOCK_SIZE_7,/*0x2F*/
    XCR_BLOCK_SIZE_8,/*0x30*/
    XCR_BLOCK_SIZE_9,/*0x31*/
    XCR_BLOCK_SIZE_10,/*0x32*/
    XCR_BLOCK_SIZE_11,/*0x33*/
    XCR_BLOCK_SIZE_12,/*0x34*/
    XCR_CHANNEL_ENABLE_1,/*0x35*/
    XCR_CHANNEL_ENABLE_2,/*0x36*/
    XCR_FLLCNT11,/*0x37*/
    XCR_FLLCNT12,/*0x38*/
    XCR_FLLCNT21,/*0x39*/
    XCR_FLLCNT22,/*0x3A*/
    XCR_VO_DELAY,/*0x3B*/
    XCR_VO_OFF_ON,/*0x3C*/
    XCR_SVO_ON,/*0x3D*/
    XCR_SVO1_OFF,/*0x3E*/
    XCR_SVO2_OFF,/*0x3F*/
    XCR_SVO3_OFF,/*0x40*/
    XCR_SVO_NUMBER,/*0x41*/
    XCR_DAC_NF_CONTROL,/*0x42*/
    XCR_DAC_CONTROL,/*0x43*/
    XCR_CURRENT_TARGET_DAC1,/*0x44*/
    XCR_CURRENT_TARGET_DAC2,/*0x45*/
    XCR_CURRENT_TARGET_DAC3,/*0x46*/
    XCR_PREVIOUS_TARGET_DAC1,/*0x47*/
    XCR_PREVIOUS_TARGET_DAC2,/*0x48*/
    XCR_PREVIOUS_TARGET_DAC3,/*0x49*/
    XCR_DAC1_OUT,/*0x4A*/
    XCR_DAC2_OUT,/*0x4B*/
    XCR_DAC3_OUT,/*0x4C*/
    XCR_DAC1_STATE,/*0x4D*/
    XCR_DAC2_STATE,/*0x4E*/
    XCR_DAC3_STATE,/*0x4F*/
    XCR_DAC1_INCREMENT_1,/*0x50*/
    XCR_DAC1_INCREMENT_2_HOLD_LIMIT,/*0x51*/
    XCR_DAC1_DECREMENT_INC_WAIT,/*0x52*/
    XCR_DAC1_INCREMENT_HOLD_THRESHOLD,/*0x53*/
    XCR_DAC1_FB_VALID_TIMER,/*0x54*/
    XCR_DAC1_MIN_LIMIT,/*0x55*/
    XCR_DAC1_MAX_LIMIT,/*0x56*/
    XCR_DAC2_INCREMENT_1,/*0x57*/
    XCR_DAC2_INCREMENT_2_HOLD_LIMIT,/*0x58*/
    XCR_DAC2_DECREMENT_INC_WAIT,/*0x59*/
    XCR_DAC2_INCREMENT_HOLD_THRESHOLD,/*0x5A*/
    XCR_DAC2_FB_VALID_TIMER,/*0x5B*/
    XCR_DAC2_MIN_LIMIT,/*0x5C*/
    XCR_DAC2_MAX_LIMIT,/*0x5D*/
    XCR_DAC3_INCREMENT_1,/*0x5E*/
    XCR_DAC3_INCREMENT_2_HOLD_LIMIT,/*0x5F*/
    XCR_DAC3_DECREMENT_INC_WAIT,/*0x60*/
    XCR_DAC3_INCREMENT_HOLD_THRESHOLD,/*0x61*/
    XCR_DAC3_FB_VALID_TIMER,/*0x62*/
    XCR_DAC3_MIN_LIMIT,/*0x63*/
    XCR_DAC3_MAX_LIMIT,/*0x64*/
    XCR_OSC_FLL_MAN_A1,/*0x65*/
    XCR_OSC_FLL_MAN_A2,/*0x66*/
    XCR_OSC_FLL_MAN_B1,/*0x67*/
    XCR_OSC_FLL_MAN_B2,/*0x68*/
    XCR_GLOBAL_WRITE_DATA,/*0x69*/
    XCR_GLOBAL_FAULT_READ_DATA1,/*0x6A*/
    XCR_GLOBAL_FAULT_READ_DATA2,/*0x6B*/
    XCR_GLOBAL_FAULT_READ_DATA3,/*0x6C*/
    XCR_GLOBAL_FAULT_READ_DATA4,/*0x6D*/
    XCR_GLOBAL_FAULT_READ_DATA5,/*0x6E*/
    XCR_GLOBAL_FAULT_READ_DATA6,/*0x6F*/

    XCR_GRP1_MAX,
}xcr_addr_grp1_t;

typedef enum _xcr_local_rw_data_addr_
{
    XCR_PORT1_LOCAL_RW_DATA1 = 0x70,/*0x70*/
    XCR_PORT2_LOCAL_RW_DATA1,/*0x71*/
    XCR_PORT3_LOCAL_RW_DATA1,/*0x72*/
    XCR_PORT4_LOCAL_RW_DATA1,/*0x73*/
    XCR_PORT5_LOCAL_RW_DATA1,/*0x74*/
    XCR_PORT6_LOCAL_RW_DATA1,/*0x75*/
    XCR_PORT7_LOCAL_RW_DATA1,/*0x76*/
    XCR_PORT8_LOCAL_RW_DATA1,/*0x77*/
    XCR_PORT1_LOCAL_RW_DATA2,/*0x78*/
    XCR_PORT2_LOCAL_RW_DATA2,/*0x79*/
    XCR_PORT3_LOCAL_RW_DATA2,/*0x7A*/
    XCR_PORT4_LOCAL_RW_DATA2,/*0x7B*/
    XCR_PORT5_LOCAL_RW_DATA2,/*0x7C*/
    XCR_PORT6_LOCAL_RW_DATA2,/*0x7D*/
    XCR_PORT7_LOCAL_RW_DATA2,/*0x7E*/
    XCR_PORT8_LOCAL_RW_DATA2,/*0x7F*/
    XCR_PORT1_LOCAL_RW_DATA3,/*0x80*/
    XCR_PORT2_LOCAL_RW_DATA3,/*0x81*/
    XCR_PORT3_LOCAL_RW_DATA3,/*0x82*/
    XCR_PORT4_LOCAL_RW_DATA3,/*0x83*/
    XCR_PORT5_LOCAL_RW_DATA3,/*0x84*/
    XCR_PORT6_LOCAL_RW_DATA3,/*0x85*/
    XCR_PORT7_LOCAL_RW_DATA3,/*0x86*/
    XCR_PORT8_LOCAL_RW_DATA3,/*0x87*/
    XCR_PORT1_LOCAL_RW_DATA4,/*0x88*/
    XCR_PORT2_LOCAL_RW_DATA4,/*0x89*/
    XCR_PORT3_LOCAL_RW_DATA4,/*0x8A*/
    XCR_PORT4_LOCAL_RW_DATA4,/*0x8B*/
    XCR_PORT5_LOCAL_RW_DATA4,/*0x8C*/
    XCR_PORT6_LOCAL_RW_DATA4,/*0x8D*/
    XCR_PORT7_LOCAL_RW_DATA4,/*0x8E*/
    XCR_PORT8_LOCAL_RW_DATA4,/*0x8F*/
    XCR_PORT1_LOCAL_RW_DATA5,/*0x90*/
    XCR_PORT2_LOCAL_RW_DATA5,/*0x91*/
    XCR_PORT3_LOCAL_RW_DATA5,/*0x92*/
    XCR_PORT4_LOCAL_RW_DATA5,/*0x93*/
    XCR_PORT5_LOCAL_RW_DATA5,/*0x94*/
    XCR_PORT6_LOCAL_RW_DATA5,/*0x95*/
    XCR_PORT7_LOCAL_RW_DATA5,/*0x96*/
    XCR_PORT8_LOCAL_RW_DATA5,/*0x97*/
    XCR_PORT1_LOCAL_RW_DATA6,/*0x98*/
    XCR_PORT2_LOCAL_RW_DATA6,/*0x99*/
    XCR_PORT3_LOCAL_RW_DATA6,/*0x9A*/
    XCR_PORT4_LOCAL_RW_DATA6,/*0x9B*/
    XCR_PORT5_LOCAL_RW_DATA6,/*0x9C*/
    XCR_PORT6_LOCAL_RW_DATA6,/*0x9D*/
    XCR_PORT7_LOCAL_RW_DATA6,/*0x9E*/
    XCR_PORT8_LOCAL_RW_DATA6,/*0x9F*/
    XCR_PORT1_LOCAL_RW_DATA7,/*0xA0*/
    XCR_PORT2_LOCAL_RW_DATA7,/*0xA1*/
    XCR_PORT3_LOCAL_RW_DATA7,/*0xA2*/
    XCR_PORT4_LOCAL_RW_DATA7,/*0xA3*/
    XCR_PORT5_LOCAL_RW_DATA7,/*0xA4*/
    XCR_PORT6_LOCAL_RW_DATA7,/*0xA5*/
    XCR_PORT7_LOCAL_RW_DATA7,/*0xA6*/
    XCR_PORT8_LOCAL_RW_DATA7,/*0xA7*/
    XCR_PORT1_LOCAL_RW_DATA8,/*0xA8*/
    XCR_PORT2_LOCAL_RW_DATA8,/*0xA9*/
    XCR_PORT3_LOCAL_RW_DATA8,/*0xAA*/
    XCR_PORT4_LOCAL_RW_DATA8,/*0xAB*/
    XCR_PORT5_LOCAL_RW_DATA8,/*0xAC*/
    XCR_PORT6_LOCAL_RW_DATA8,/*0xAD*/
    XCR_PORT7_LOCAL_RW_DATA8,/*0xAE*/
    XCR_PORT8_LOCAL_RW_DATA8,/*0xAF*/
    XCR_PORT1_LOCAL_RW_DATA9,/*0xB0*/
    XCR_PORT2_LOCAL_RW_DATA9,/*0xB1*/
    XCR_PORT3_LOCAL_RW_DATA9,/*0xB2*/
    XCR_PORT4_LOCAL_RW_DATA9,/*0xB3*/
    XCR_PORT5_LOCAL_RW_DATA9,/*0xB4*/
    XCR_PORT6_LOCAL_RW_DATA9,/*0xB5*/
    XCR_PORT7_LOCAL_RW_DATA9,/*0xB6*/
    XCR_PORT8_LOCAL_RW_DATA9,/*0xB7*/
    XCR_PORT1_LOCAL_RW_DATA10,/*0xB8*/
    XCR_PORT2_LOCAL_RW_DATA10,/*0xB9*/
    XCR_PORT3_LOCAL_RW_DATA10,/*0xBA*/
    XCR_PORT4_LOCAL_RW_DATA10,/*0xBB*/
    XCR_PORT5_LOCAL_RW_DATA10,/*0xBC*/
    XCR_PORT6_LOCAL_RW_DATA10,/*0xBD*/
    XCR_PORT7_LOCAL_RW_DATA10,/*0xBE*/
    XCR_PORT8_LOCAL_RW_DATA10,/*0xBF*/
    XCR_PORT1_LOCAL_RW_DATA11,/*0xC0*/
    XCR_PORT2_LOCAL_RW_DATA11,/*0xC1*/
    XCR_PORT3_LOCAL_RW_DATA11,/*0xC2*/
    XCR_PORT4_LOCAL_RW_DATA11,/*0xC3*/
    XCR_PORT5_LOCAL_RW_DATA11,/*0xC4*/
    XCR_PORT6_LOCAL_RW_DATA11,/*0xC5*/
    XCR_PORT7_LOCAL_RW_DATA11,/*0xC6*/
    XCR_PORT8_LOCAL_RW_DATA11,/*0xC7*/
    XCR_PORT1_LOCAL_RW_DATA12,/*0xC8*/
    XCR_PORT2_LOCAL_RW_DATA12,/*0xC9*/
    XCR_PORT3_LOCAL_RW_DATA12,/*0xCA*/
    XCR_PORT4_LOCAL_RW_DATA12,/*0xCB*/
    XCR_PORT5_LOCAL_RW_DATA12,/*0xCC*/
    XCR_PORT6_LOCAL_RW_DATA12,/*0xCD*/
    XCR_PORT7_LOCAL_RW_DATA12,/*0xCE*/
    XCR_PORT8_LOCAL_RW_DATA12,/*0xCF*/
    XCR_PORT1_LOCAL_RW_DATA13,/*0xD0*/
    XCR_PORT2_LOCAL_RW_DATA13,/*0xD1*/
    XCR_PORT3_LOCAL_RW_DATA13,/*0xD2*/
    XCR_PORT4_LOCAL_RW_DATA13,/*0xD3*/
    XCR_PORT5_LOCAL_RW_DATA13,/*0xD4*/
    XCR_PORT6_LOCAL_RW_DATA13,/*0xD5*/
    XCR_PORT7_LOCAL_RW_DATA13,/*0xD6*/
    XCR_PORT8_LOCAL_RW_DATA13,/*0xD7*/
    XCR_PORT1_LOCAL_RW_DATA14,/*0xD8*/
    XCR_PORT2_LOCAL_RW_DATA14,/*0xD9*/
    XCR_PORT3_LOCAL_RW_DATA14,/*0xDA*/
    XCR_PORT4_LOCAL_RW_DATA14,/*0xDB*/
    XCR_PORT5_LOCAL_RW_DATA14,/*0xDC*/
    XCR_PORT6_LOCAL_RW_DATA14,/*0xDD*/
    XCR_PORT7_LOCAL_RW_DATA14,/*0xDE*/
    XCR_PORT8_LOCAL_RW_DATA14,/*0xDF*/
    XCR_PORT1_LOCAL_RW_DATA15,/*0xE0*/
    XCR_PORT2_LOCAL_RW_DATA15,/*0xE1*/
    XCR_PORT3_LOCAL_RW_DATA15,/*0xE2*/
    XCR_PORT4_LOCAL_RW_DATA15,/*0xE3*/
    XCR_PORT5_LOCAL_RW_DATA15,/*0xE4*/
    XCR_PORT6_LOCAL_RW_DATA15,/*0xE5*/
    XCR_PORT7_LOCAL_RW_DATA15,/*0xE6*/
    XCR_PORT8_LOCAL_RW_DATA15,/*0xE7*/
    XCR_PORT1_LOCAL_RW_DATA16,/*0xE8*/
    XCR_PORT2_LOCAL_RW_DATA16,/*0xE9*/
    XCR_PORT3_LOCAL_RW_DATA16,/*0xEA*/
    XCR_PORT4_LOCAL_RW_DATA16,/*0xEB*/
    XCR_PORT5_LOCAL_RW_DATA16,/*0xEC*/
    XCR_PORT6_LOCAL_RW_DATA16,/*0xED*/
    XCR_PORT7_LOCAL_RW_DATA16,/*0xEE*/
    XCR_PORT8_LOCAL_RW_DATA16,/*0xEF*/
}xcr_local_rw_data_addr_t;

#define OTP_BASE_ADDR       (0xF0U)
typedef enum _xcr_addr_otp_
{
    XCR_TEST_CONTROL = 0U,/*0xF0*/
    XCR_OTP_PG_ACCESS,/*0xF1*/
    XCR_OTP_WRITE,/*0xF2*/
    XCR_OTP_RD_PROG,/*0xF3*/
    XCR_OTP_PROTECT,/*0xF4*/
    XCR_MIRROR1,/*0xF5*/
    XCR_MIRROR2,/*0xF6*/
    XCR_MIRROR3,/*0xF7*/
    XCR_MIRROR4,/*0xF8*/
    XCR_MIRROR5,/*0xF9*/
    XCR_MIRROR6,/*0xFA*/

    XCR_OTP_MAX
}xcr_addr_otp_t;

typedef enum _xcr_addr_grp2_
{
    XCR_GRP2_DAC1_FB_VALID_CNT = 0U,/*0x00*/
    XCR_GRP2_DAC1_INC_HOLD_WAIT_CNT,/*0x01*/
    XCR_GRP2_1R2,/*0x02*/
    XCR_GRP2_SOA1_N1_N11,/*0x03*/
    XCR_GRP2_SOA1_P2_P1,/*0x04*/
    XCR_GRP2_SOA1_P3_P2,/*0x05*/
    XCR_GRP2_DAC2_FB_VALID_CNT,/*0x06*/
    XCR_GRP2_DAC2_INC_HOLD_WAIT_CNT,/*0x07*/
    XCR_GRP2_2R2,/*0x08*/
    XCR_GRP2_SOA2_N1_N11,/*0x09*/
    XCR_GRP2_SOA2_P2_P1,/*0x0A*/
    XCR_GRP2_SOA2_P3_P2,/*0x0B*/
    XCR_GRP2_DAC3_FB_VALID_CNT,/*0x0C*/
    XCR_GRP2_DAC3_INC_HOLD_WAIT_CNT,/*0x0D*/
    XCR_GRP2_3R2,/*0x0E*/
    XCR_GRP2_SOA3_N1_N11,/*0x0F*/
    XCR_GRP2_SOA3_P2_P1,/*0x10*/
    XCR_GRP2_SOA3_P3_P2,/*0x11*/
    XCR_GRP2_ANA_TEST,/*0x12*/
    XCR_GRP2_AD_TEST,/*0x13*/

    XCR_GRP2_MAX,
}xcr_addr_grp2_t;

/* Command packet format */
typedef union tag_CMD
{
    uint16_t ALL;
    struct
    {
        uint16_t size   :  6;   /* BURST SIZE : Amount of data to transfer or receive */
        uint16_t addr   :  8;   /* START ADDRESS : The start address of registers to access */
        uint16_t code   :  2;   /* COMMAND CODE : */
    }bit;
}_cmd_t;

/* Dummy */
typedef union tag_DUMMY
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy  : 16;   /* for unused address */
    }bit;
}_v_dummy_t;

/* Name : RESET, access : r/w, default : 0x0030, address : 0x00 */
typedef union tag_RESET
{
    uint16_t ALL;
    struct
    {
        uint16_t rst1           : 1;    /* rst1 */
        uint16_t rst2           : 1;    /* rst2 */
        uint16_t rst3           : 1;    /* rst3 */
        uint16_t                : 1;    /* reserved */
        uint16_t vsync_rst_en1  : 1;    /* vsync_rst_en1 */
        uint16_t vsync_rst_en2  : 1;    /* vsync_rst_en2 */
        uint16_t                : 10;   /* reserved */
    }bit;
}_v_reset_t;

/* Name : GLOBAL_WRITE_COMMAND, access : r/w, default : 0x0000, address : 0x01 */
typedef union tag_GLOBAL_WRITE_COMMAND
{
    uint16_t ALL;
    struct
    {
        uint16_t addr           : 6;    /* addr [5:0] */
        uint16_t                : 9;    /* reserved */
        uint16_t enable         : 1;    /* enable */
    }bit;
}_v_global_write_command_t;

/* Name : LOCAL_WRITE_COMMAND, access : r/w, default : 0x0000, address : 0x02 */
typedef union tag_LOCAL_WRITE_COMMAND
{
    uint16_t ALL;
    struct
    {
        uint16_t addr           : 6;    /* addr [5:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t ch_seg         : 2;    /* ch_seg[1:0] */
        uint16_t                : 5;    /* reserved */
        uint16_t enable         : 1;    /* enable */
    }bit;
}_v_local_write_command_t;

/* Name : LOCAL_READ_COMMAND, access : r/w, default : 0x0000, address : 0x03 */
typedef union tag_LOCAL_READ_COMMAND
{
    uint16_t ALL;
    struct
    {
        uint16_t addr           : 6;    /* addr [5:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t ch_seg         : 2;    /* ch_seg[1:0] */
        uint16_t                : 5;    /* reserved */
        uint16_t enable         : 1;    /* enable */
    }bit;
}_v_local_read_command_t;

/* Name : ID_GEN_COMMAND, access : r/w, default : 0x0000, address : 0x04 */
typedef union tag_ID_GEN_COMMAND
{
    uint16_t ALL;
    struct
    {
        uint16_t                : 15;   /* reserved */
        uint16_t enable         : 1;    /* enable */
    }bit;
}_v_id_gen_command_t;

/* Name : FAULT_READ_COMMAND, access : r/w, default : 0x0000, address : 0x05 */
typedef union tag_FAULT_READ_COMMAND
{
    uint16_t ALL;
    struct
    {
        uint16_t                : 15;   /* reserved */
        uint16_t enable         : 1;    /* enable */
    }bit;
}_v_fault_read_command_t;

/* Name : LD_TRANSFER_COMMAND, access : r/w, default : 0x0000, address : 0x06 */
typedef union tag_LD_TRANSFER_COMMAND
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_type        : 2;    /* ld_type [1:0] */
        uint16_t                : 13;   /* reserved */
        uint16_t enable         : 1;    /* enable */
    }bit;
}_v_ld_transfer_command_t;

/* Name : SYNC_GEN_COMMAND, access : r/w, default : 0x0000, address : 0x07 */
typedef union tag_SYNC_GEN_COMMAND
{
    uint16_t ALL;
    struct
    {
        uint16_t syncmode       : 1;    /* syncmode : '0' cmd + svo, '1' vo + svo */
        uint16_t                : 14;   /* reserved */
        uint16_t enable         : 1;    /* enable */
    }bit;
}_v_sync_gen_command_t;

/* Name : COMMAND_AUTO_ENABLE, access : r/w, default : 0x0111, address : 0x08 */
typedef union tag_COMMAND_AUTO_ENABLE
{
    uint16_t ALL;
    struct
    {
        uint16_t sync_auto_en   : 1;    /* sync auto_en */
        uint16_t                : 3;    /* reserved */
        uint16_t fault_auto_en  : 1;    /* fault auto_en */
        uint16_t                : 11;   /* reserved */
    }bit;
}_v_command_auto_enable_t;

/* Name : LD_WRITE_POINTER, access : ro, default : 0x0000, address : 0x09 */
typedef union tag_LD_WRITE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_wr_pointer  : 9;    /* ld_wr_pointer [8:0] */
        uint16_t                : 7;    /* reserved */
    }bit;
}_v_ld_write_pointer_t;

/* Name : LD_READ_POINTER, access : ro, default : 0x0000, address : 0x0A */
typedef union tag_LD_READ_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_rd_pointer  : 9;    /* ld_rd_pointer [8:0] */
        uint16_t                : 7;    /* reserved */
    }bit;
}_v_ld_read_pointer_t;

/* Name : LD_DIFFERENCE_POINTER, access : ro, default : 0x0000, address : 0x0B */
typedef union tag_LD_DIFFERENCE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_diff_pointer : 9;    /* ld_diff_pointer [8:0] */
        uint16_t                 : 7;    /* reserved */
    }bit;
}_v_ld_difference_pointer_t;

/* Name : LD_START_POINTER/TH, access : r/w, default : 0x0000, address : 0x0C */
typedef union tag_LD_START_POINTER_TH
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_diff_threshold          : 6;    /* ld_diff_threshold [5:0] */
        uint16_t                            : 1;    /* reserved */
        uint16_t int_ld_sign                : 1;    /* int_ld_sign */
        uint16_t ld_transfer_start_pointer  : 6;    /* ld_transfer_start_pointer [5:0] */
        uint16_t                            : 2;    /* reserved */
    }bit;
}_v_ld_start_pointer_th_t;

/* Name : LOCAL_WRITE_TRANSFER_POINTER, access : ro, default : 0x0000, address : 0x0D */
typedef union tag_LOCAL_WRITE_TRANSFER_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_transfer_write_pointer : 6;   /* local_wr_transfer_write_pointer [5:0] */
        uint16_t                                 : 2;   /* reserved */
        uint16_t local_wr_transfer_read_pointer  : 6;   /* local_wr_transfer_read_pointer [5:0] */
        uint16_t                                 : 2;   /* reserved */
    }bit;
}_v_local_write_transfer_pointer_t;

/* Name : LOCAL_READ_RECEIVE_POINTER, access : ro, default : 0x0000, address : 0x0E */
typedef union tag_LOCAL_READ_RECEIVE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_rd_receive_write_pointer : 6;    /* local_rd_receive_write_pointer [5:0] */
        uint16_t                                : 2;    /* reserved */
        uint16_t local_rd_receive_read_pointer  : 6;    /* local_rd_receive_read_pointer [5:0] */
        uint16_t                                : 2;    /* reserved */
    }bit;
}_v_local_read_receive_pointer_t;

/* Name : LOCAL_RW_DIFFERENCE_POINTER, access : ro, default : 0x0000, address : 0x0F */
typedef union tag_LOCAL_RW_DIFFERENCE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_transfer_diff_pointer : 6;    /* local_wr_transfer_diff_pointer [5:0] */
        uint16_t                                : 2;    /* reserved */
        uint16_t local_rd_receive_diff_pointer  : 6;    /* local_rd_receive_diff_pointer [5:0] */
        uint16_t                                : 2;    /* reserved */
    }bit;
}_v_local_rw_difference_pointer_t;

/* Name : LOCAL_RW_POINTER_RESET, access : wo, default : 0x0000, address : 0x10 */
typedef union tag_LOCAL_RW_POINTER_RESET
{
    uint16_t ALL;
    struct
    {
        uint16_t local_transfer_pointer_rst : 1;    /* local_transfer_pointer_rst */
        uint16_t                            : 7;    /* reserved */
        uint16_t local_receive_pointer_rst  : 1;    /* local_receive_pointer_rst */
        uint16_t                            : 7;    /* reserved */
    }bit;
}_v_local_rw_pointer_reset_t;

/* Name : FAULT_AUTO_READ_INTERVAL, access : r/w, default : 0x0000, address : 0x11 */
typedef union tag_FAULT_AUTO_READ_INTERVAL
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_auto_rd_interval : 16;    /* fault_auto_rd_interval [15:0] */
    }bit;
}_v_fault_auto_read_interval_t;

/* Name : FAULT_AUTO_READ_EVENT, access : r/w, default : 0x0000, address : 0x12 */
typedef union tag_FAULT_AUTO_READ_EVENT
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_auto_rd_interval    : 1; /* fault_auto_rd_interval [16] */
        uint16_t                           : 7; /* reserved */
        uint16_t fault_auto_rd_timer_event : 8; /* fault_auto_rd_timer_event [7:0] */
    }bit;
}_v_fault_auto_read_event_t;

/* Name : INTERRUPT_ENABLE, access : r/w, default : 0xFF8F, address : 0x13 */
typedef union tag_INTERRUPT_ENABLE
{
    uint16_t ALL;
    struct
    {
        uint16_t int_source1_en : 1;    /* int_source1_en */
        uint16_t int_source2_en : 1;    /* int_source2_en */
        uint16_t int_source3_en : 1;    /* int_source3_en */
        uint16_t int_source4_en : 1;    /* int_source4_en */
        uint16_t int_ld_en      : 1;    /* int_ld_en */
        uint16_t                : 2;    /* reserved */
        uint16_t int_vs_miss_en : 1;    /* int_vs_miss_en */
        uint16_t int_timeout_err_en : 1;    /* int_timeout_err_en */
        uint16_t int_ser_crc_en : 1;    /* int_ser_crc_en */
        uint16_t int_spi_crc_en : 1;    /* int_spi_crc_en */
        uint16_t int_ctrl_mismatch_en : 1;    /* int_ctrl_mismatch_en */
        uint16_t int_gate_short_en : 1;    /* int_gate_short_en */
        uint16_t int_osc_err_en : 1;    /* int_osc_err_en */
        uint16_t int_ldo_ovuv_en : 1;    /* int_ldo_ovuv_en */
        uint16_t int_bgr_ovuv_en : 1;    /* int_bgr_ovuv_en */
    }bit;
}_v_interrupt_enable_t;

/* Name : COMMAND_STATUS_1, access : ro, default : 0x0000, address : 0x14 */
typedef union tag_COMMAND_STATUS_1
{
    uint16_t ALL;
    struct
    {
        uint16_t sync_doing     : 1;     /* sync_doing */
        uint16_t sync_done      : 1;     /* sync_done */
        uint16_t sync_auto_doing : 1;     /* sync_auto_doing */
        uint16_t sync_auto_done : 1;     /* sync_auto_done */
        uint16_t fault_auto_doing : 1;     /* fault_auto_doing */
        uint16_t fault_auto_done : 1;     /* fault_auto_done */
        uint16_t fault_doing    : 1;     /* fault_doing */
        uint16_t fault_done     : 1;     /* fault_done */
        uint16_t ld_doing       : 1;     /* ld_doing */
        uint16_t ld_done        : 1;     /* ld_done */
        uint16_t glbl_w_doing   : 1;     /* glbl_w_doing */
        uint16_t glbl_w_done    : 1;     /* glbl_w_done */
        uint16_t local_w_doing  : 1;     /* local_w_doing */
        uint16_t local_w_done   : 1;     /* local_w_done */
        uint16_t local_r_doing  : 1;     /* local_r_doing */
        uint16_t local_r_done   : 1;     /* local_r_done */
    }bit;
}_v_command_status_1_t;

/* Name : COMMAND_STATUS_2, access : ro, default : 0x0000, address : 0x15 */
typedef union tag_COMMAND_STATUS_2
{
    uint16_t ALL;
    struct
    {
        uint16_t id_doing       : 1;    /* id_doing */
        uint16_t id_done        : 1;    /* id_done */
        uint16_t                : 14;   /* reserved */
    }bit;
}_v_command_status_2_t;

/* Name : RECEIVE_STATUS, access : ro, default : 0x0000, address : 0x16 */
typedef union tag_RECEIVE_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t rd_receive_doing : 1;    /* rd_receive_doing */
        uint16_t rd_receive_done : 1;    /* rd_receive_done */
        uint16_t fault_auto_rec_doing : 1;    /* fault_auto_rec_doing */
        uint16_t fault_auto_rec_done : 1;    /* fault_auto_rec_done */
        uint16_t fault_receive_doing : 1;    /* fault_receive_doing */
        uint16_t fault_receive_done : 1;    /* fault_receive_done */
        uint16_t                : 6;    /* reserved */
        uint16_t rd_receive_fail : 1;    /* rd_receive_fail */
        uint16_t fault_auto_rec_fail : 1;    /* fault_auto_rec_fail */
        uint16_t fault_receive_fail : 1;    /* fault_receive_fail */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_receive_status_t;

/* Name : INTERRUPT_STATUS, access : ro, default : 0x0000, address : 0x17 */
typedef union tag_INTERRUPT_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t int_fault      : 1;     /* int_fault */
        uint16_t int_ld         : 1;     /* int_ld */
        uint16_t int_source1    : 1;     /* int_source1 */
        uint16_t int_source2    : 1;     /* int_source2 */
        uint16_t int_source3    : 1;     /* int_source3 */
        uint16_t int_source4    : 1;     /* int_source4 */
        uint16_t int_ld_source  : 1;     /* int_ld_source */
        uint16_t int_vs_miss_source : 1;     /* int_vs_miss_source */
        uint16_t int_timeout_err_source : 1;     /* int_timeout_err_source */
        uint16_t int_ser_crc_err_src : 1;     /* int_ser_crc_err_src */
        uint16_t int_spi_crc_err_src : 1;     /* int_spi_crc_err_src */
        uint16_t int_ctrl_mismatch_err_src : 1;     /* int_ctrl_mismatch_err_src */
        uint16_t int_gate_err_src : 1;     /* int_gate_err_src */
        uint16_t int_osc_err_src : 1;     /* int_osc_err_src */
        uint16_t int_ldo_err_src : 1;     /* int_ldo_err_src */
        uint16_t int_bgr_err_src : 1;     /* int_bgr_err_src */
    }bit;
}_v_interrupt_status_t;

/* Name : ERR_STATUS, access : ro, default : 0x0000, address : 0x18 */
typedef union tag_ERR_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t bgr_uv_det     : 1;    /* bgr_uv_det */
        uint16_t bgr_ov_det     : 1;    /* bgr_ov_det */
        uint16_t ldo_uv_det     : 1;    /* ldo_uv_det */
        uint16_t ldo_ov_det     : 1;    /* ldo_ov_det */
        uint16_t gate1_ext_short : 1;    /* gate1_ext_short */
        uint16_t gate2_ext_short : 1;    /* gate2_ext_short */
        uint16_t gate3_ext_short : 1;    /* gate3_ext_short */
        uint16_t gate1_disc_short : 1;    /* gate1_disc_short */
        uint16_t gate2_disc_short : 1;    /* gate2_disc_short */
        uint16_t gate3_disc_short : 1;    /* gate3_disc_short */
        uint16_t                : 6;    /* reserved */
    }bit;
}_v_err_status_t;

/* Name : CH_CRC_ERR_STATUS, access : ro, default : 0x0000, address : 0x19 */
typedef union tag_CH_CRC_ERR_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t ch1_crc_err    : 1;     /* ch1_crc_err */
        uint16_t ch2_crc_err    : 1;     /* ch2_crc_err */
        uint16_t ch3_crc_err    : 1;     /* ch3_crc_err */
        uint16_t ch4_crc_err    : 1;     /* ch4_crc_err */
        uint16_t ch5_crc_err    : 1;     /* ch5_crc_err */
        uint16_t ch6_crc_err    : 1;     /* ch6_crc_err */
        uint16_t ch7_crc_err    : 1;     /* ch7_crc_err */
        uint16_t ch8_crc_err    : 1;     /* ch8_crc_err */
        uint16_t ch9_crc_err    : 1;     /* ch9_crc_err */
        uint16_t ch10_crc_err   : 1;     /* ch10_crc_err */
        uint16_t ch11_crc_err   : 1;     /* ch11_crc_err */
        uint16_t ch12_crc_err   : 1;     /* ch12_crc_err */
        uint16_t ch13_crc_err   : 1;     /* ch13_crc_err */
        uint16_t ch14_crc_err   : 1;     /* ch14_crc_err */
        uint16_t ch15_crc_err   : 1;     /* ch15_crc_err */
        uint16_t ch16_crc_err   : 1;     /* ch16_crc_err */
    }bit;
}_v_ch_crc_err_status_t;

/* Name : CH_TIMEOUT_ERR_STATUS, access : ro, default : 0x0000, address : 0x1A */
typedef union tag_CH_TIMEOUT_ERR_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t ch1_timeout_err : 1;     /* ch1_timeout_err */
        uint16_t ch2_timeout_err : 1;     /* ch2_timeout_err */
        uint16_t ch3_timeout_err : 1;     /* ch3_timeout_err */
        uint16_t ch4_timeout_err : 1;     /* ch4_timeout_err */
        uint16_t ch5_timeout_err : 1;     /* ch5_timeout_err */
        uint16_t ch6_timeout_err : 1;     /* ch6_timeout_err */
        uint16_t ch7_timeout_err : 1;     /* ch7_timeout_err */
        uint16_t ch8_timeout_err : 1;     /* ch8_timeout_err */
        uint16_t ch9_timeout_err : 1;     /* ch9_timeout_err */
        uint16_t ch10_timeout_err : 1;     /* ch10_timeout_err */
        uint16_t ch11_timeout_err : 1;     /* ch11_timeout_err */
        uint16_t ch12_timeout_err : 1;     /* ch12_timeout_err */
        uint16_t ch13_timeout_err : 1;     /* ch13_timeout_err */
        uint16_t ch14_timeout_err : 1;     /* ch14_timeout_err */
        uint16_t ch15_timeout_err : 1;     /* ch15_timeout_err */
        uint16_t ch16_timeout_err : 1;     /* ch16_timeout_err */
    }bit;
}_v_ch_timeout_err_status_t;

/* Name : BIST_CONTROL_&_STATUS, access : r/w, default : 0x0000, address : 0x1B */
typedef union tag_BIST_CONTROL_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t bist_err       : 1;    /* bist_err */
        uint16_t bist_done      : 1;    /* bist_done */
        uint16_t bist_busy      : 1;    /* bist_busy */
        uint16_t                : 12;   /* reserved */
        uint16_t BIST_START     : 1;    /* BIST_START */
    }bit;
}_v_bist_control_status_t;

/* Name : COMMUNICATION_FAULT_CONTROL, access : r/w, default : 0x0000, address : 0x1C */
typedef union tag_COMMUNICATION_FAULT_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t spi_crc_en     : 1;    /* spi_crc_en */
        uint16_t ser_crc_en     : 1;    /* ser_crc_en */
        uint16_t vs_miss_en     : 1;    /* vs_miss_en */
        uint16_t                : 13;   /* reserved */
    }bit;
}_v_communication_fault_control_t;

/* Name : CLK_CONTROL_1, access : r/w, default : 0x0000, address : 0x1D */
typedef union tag_CLK_CONTROL_1
{
    uint16_t ALL;
    struct
    {
        uint16_t                        : 1;    /* reserved */
        uint16_t serializer_skew_en     : 1;    /* serializer_skew_en '0' disable '1' enable */
        uint16_t osc1_spread_en         : 1;    /* osc1_spread_en '0' disable '1' enable */
        uint16_t serializer_clk_sel1    : 1;    /* serializer_clk_sel1 '0' osc1 '1' depend on bit8 */
        uint16_t sprd1_gain             : 3;    /* sprd1_gain [2:0] */
        uint16_t                        : 1;    /* reserved */
        uint16_t serializer_clk_sel2    : 1;    /* serializer_clk_sel2 '0' osc2 '1' Ext. clk */
        uint16_t                        : 2;    /* reserved */
        uint16_t ld_rd_clk_sel          : 1;    /* ld_rd_clk_sel '0' osc1 '1' depend on bit8 */
        uint16_t spread1_spd            : 3;    /* spread1_spd [2:0] */
        uint16_t                        : 1;    /* reserved */
    }bit;
}_v_clk_control_1_t;

/* Name : CLK_CONTROL_2, access : r/w, default : 0x0000, address : 0x1E */
typedef union tag_CLK_CONTROL_2
{
    uint16_t ALL;
    struct
    {
        uint16_t mclk_mode      : 1;    /* mclk_mode '0' osc2 (default)  '1' Ext clk (if entered) */
        uint16_t                : 1;    /* reserved */
        uint16_t osc2_spread_en : 1;    /* osc2_spread_en '0' disable '1' enable */
        uint16_t                : 1;    /* reserved */
        uint16_t sprd2_gain     : 3;    /* sprd2_gain [2:0] */
        uint16_t                : 5;    /* reserved */
        uint16_t spread2_spd    : 3;    /* spread2_spd [2:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_clk_control_2_t;

/* Name : SERIALIZER_CLOCK_GEN, access : r/w, default : 0x0C18, address : 0x1F */
typedef union tag_SERIALIZER_CLOCK_GEN
{
    uint16_t ALL;
    struct
    {
        uint16_t serial_clk_high : 7;    /* serial_clk_high [6:0] */
        uint16_t                : 1;    /* reserved */
        uint16_t serial_clk_low : 7;    /* serial_clk_low [6:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_serializer_clock_gen_t;

/* Name : LATENCY, access : r/w, default : 0x70C0, address : 0x20 */
typedef union tag_LATENCY
{
    uint16_t ALL;
    struct
    {
        uint16_t cmd_latency    : 8;     /* cmd_latency  [7:0] */
        uint16_t serial_latency : 8;     /* serial_latency [7:0] */
    }bit;
}_v_latency_t;

/* Name : TIMEOUT, access : r/w, default : 0x7FFF, address : 0x21 */
typedef union tag_TIMEOUT
{
    uint16_t ALL;
    struct
    {
        uint16_t timeout        : 15;   /* timeout [14:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_timeout_t;

/* Name : DAISIED_DEVICE_CH_SIZE, access : r/w, default : 0x0000, address : 0x22 */
typedef union tag_DAISIED_DEVICE_CH_SIZE
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size1 : 5;    /* daisied_dev_ch_size1 [4:0] */
        uint16_t                : 11;   /* reserved */
    }bit;
}_v_daisied_device_ch_size_t;

/* Name : DAISY_SIZE_1, access : r/w, default : 0x0000, address : 0x23 */
typedef union tag_DAISY_SIZE_1
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch1 : 5;    /* daisy_size_ch1[4:0] */
        uint16_t daisy_size_ch2 : 5;    /* daisy_size_ch2[4:0] */
        uint16_t daisy_size_ch3 : 5;    /* daisy_size_ch3[4:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_daisy_size_1_t;

/* Name : DAISY_SIZE_2, access : r/w, default : 0x0000, address : 0x24 */
typedef union tag_DAISY_SIZE_2
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch4 : 5;    /* daisy_size_ch4[4:0] */
        uint16_t daisy_size_ch5 : 5;    /* daisy_size_ch5[4:0] */
        uint16_t daisy_size_ch6 : 5;    /* daisy_size_ch6[4:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_daisy_size_2_t;

/* Name : DAISY_SIZE_3, access : r/w, default : 0x0000, address : 0x25 */
typedef union tag_DAISY_SIZE_3
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch7 : 5;    /* daisy_size_ch7[4:0] */
        uint16_t daisy_size_ch8 : 5;    /* daisy_size_ch8[4:0] */
        uint16_t daisy_size_ch9 : 5;    /* daisy_size_ch9[4:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_daisy_size_3_t;

/* Name : DAISY_SIZE_4, access : r/w, default : 0x0000, address : 0x26 */
typedef union tag_DAISY_SIZE_4
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch10 : 5;    /* daisy_size_ch10[4:0] */
        uint16_t daisy_size_ch11 : 5;    /* daisy_size_ch11[4:0] */
        uint16_t daisy_size_ch12 : 5;    /* daisy_size_ch12[4:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_daisy_size_4_t;

/* Name : DAISY_SIZE_5, access : r/w, default : 0x0000, address : 0x27 */
typedef union tag_DAISY_SIZE_5
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch13 : 5;    /* daisy_size_ch13[4:0] */
        uint16_t daisy_size_ch14 : 5;    /* daisy_size_ch14[4:0] */
        uint16_t daisy_size_ch15 : 5;    /* daisy_size_ch15[4:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_daisy_size_5_t;

/* Name : DAISY_SIZE_6, access : r/w, default : 0x0000, address : 0x28 */
typedef union tag_DAISY_SIZE_6
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch16 : 5;    /* daisy_size_ch16[4:0] */
        uint16_t daisy_size_ch17 : 5;    /* daisy_size_ch17[4:0] */
        uint16_t daisy_size_ch18 : 5;    /* daisy_size_ch18[4:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_daisy_size_6_t;

/* Name : DAISY_SIZE_7, access : r/w, default : 0x0000, address : 0x27 */
typedef union tag_DAISY_SIZE_7
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch19 : 5;    /* daisy_size_ch19[4:0] */
        uint16_t daisy_size_ch20 : 5;    /* daisy_size_ch20[4:0] */
        uint16_t daisy_size_ch21 : 5;    /* daisy_size_ch21[4:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_daisy_size_7_t;

/* Name : DAISY_SIZE_8, access : r/w, default : 0x0000, address : 0x28 */
typedef union tag_DAISY_SIZE_8
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch22 : 5;    /* daisy_size_ch22[4:0] */
        uint16_t daisy_size_ch23 : 5;    /* daisy_size_ch23[4:0] */
        uint16_t daisy_size_ch24 : 5;    /* daisy_size_ch24[4:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_daisy_size_8_t;

/* Name : BLOCK_SIZE_1, access : r/w, default : 0x0000, address : 0x29 */
typedef union tag_BLOCK_SIZE_1
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch1 : 8;     /* block_size_ch1 [7:0] */
        uint16_t total_blk_size_ch2 : 8;     /* block_size_ch2 [7:0] */
    }bit;
}_v_block_size_1_t;

/* Name : BLOCK_SIZE_2, access : r/w, default : 0x0000, address : 0x2A */
typedef union tag_BLOCK_SIZE_2
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch3 : 8;     /* block_size_ch3 [7:0] */
        uint16_t total_blk_size_ch4 : 8;     /* block_size_ch4 [7:0] */
    }bit;
}_v_block_size_2_t;

/* Name : BLOCK_SIZE_3, access : r/w, default : 0x0000, address : 0x2B */
typedef union tag_BLOCK_SIZE_3
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch5 : 8;     /* block_size_ch5 [7:0] */
        uint16_t total_blk_size_ch6 : 8;     /* block_size_ch6 [7:0] */
    }bit;
}_v_block_size_3_t;

/* Name : BLOCK_SIZE_4, access : r/w, default : 0x0000, address : 0x2C */
typedef union tag_BLOCK_SIZE_4
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch7 : 8;     /* block_size_ch7 [7:0] */
        uint16_t total_blk_size_ch8 : 8;     /* block_size_ch8 [7:0] */
    }bit;
}_v_block_size_4_t;

/* Name : BLOCK_SIZE_5, access : r/w, default : 0x0000, address : 0x2D */
typedef union tag_BLOCK_SIZE_5
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch9 : 8;     /* block_size_ch9 [7:0] */
        uint16_t total_blk_size_ch10 : 8;     /* block_size_ch10 [7:0] */
    }bit;
}_v_block_size_5_t;

/* Name : BLOCK_SIZE_6, access : r/w, default : 0x0000, address : 0x2E */
typedef union tag_BLOCK_SIZE_6
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch11 : 8;     /* block_size_ch11 [7:0] */
        uint16_t total_blk_size_ch12 : 8;     /* block_size_ch12 [7:0] */
    }bit;
}_v_block_size_6_t;

/* Name : BLOCK_SIZE_7, access : r/w, default : 0x0000, address : 0x2F */
typedef union tag_BLOCK_SIZE_7
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch13 : 8;     /* block_size_ch13 [7:0] */
        uint16_t total_blk_size_ch14 : 8;     /* block_size_ch14 [7:0] */
    }bit;
}_v_block_size_7_t;

/* Name : BLOCK_SIZE_8, access : r/w, default : 0x0000, address : 0x30 */
typedef union tag_BLOCK_SIZE_8
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch15 : 8;     /* block_size_ch15 [7:0] */
        uint16_t total_blk_size_ch16 : 8;     /* block_size_ch16 [7:0] */
    }bit;
}_v_block_size_8_t;

/* Name : BLOCK_SIZE_9, access : r/w, default : 0x0000, address : 0x31 */
typedef union tag_BLOCK_SIZE_9
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch17 : 8;     /* block_size_ch17 [7:0] */
        uint16_t total_blk_size_ch18 : 8;     /* block_size_ch18 [7:0] */
    }bit;
}_v_block_size_9_t;

/* Name : BLOCK_SIZE_10, access : r/w, default : 0x0000, address : 0x32 */
typedef union tag_BLOCK_SIZE_10
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch19 : 8;     /* block_size_ch19 [7:0] */
        uint16_t total_blk_size_ch20 : 8;     /* block_size_ch20 [7:0] */
    }bit;
}_v_block_size_10_t;

/* Name : BLOCK_SIZE_11, access : r/w, default : 0x0000, address : 0x33 */
typedef union tag_BLOCK_SIZE_11
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch21 : 8;     /* block_size_ch21 [7:0] */
        uint16_t total_blk_size_ch22 : 8;     /* block_size_ch22 [7:0] */
    }bit;
}_v_block_size_11_t;

/* Name : BLOCK_SIZE_12, access : r/w, default : 0x0000, address : 0x34 */
typedef union tag_BLOCK_SIZE_12
{
    uint16_t ALL;
    struct
    {
        uint16_t total_blk_size_ch23 : 8;     /* block_size_ch23 [7:0] */
        uint16_t total_blk_size_ch24 : 8;     /* block_size_ch24 [7:0] */
    }bit;
}_v_block_size_12_t;

/* Name : CHANNEL_ENALBE_1, access : r/w, default : 0x0000, address : 0x35 */
typedef union tag_CHANNEL_ENALBE_1
{
    uint16_t ALL;
    struct
    {
        uint16_t ch1_en         : 1;     /* ch1_en */
        uint16_t ch2_en         : 1;     /* ch2_en */
        uint16_t ch3_en         : 1;     /* ch3_en */
        uint16_t ch4_en         : 1;     /* ch4_en */
        uint16_t ch5_en         : 1;     /* ch5_en */
        uint16_t ch6_en         : 1;     /* ch6_en */
        uint16_t ch7_en         : 1;     /* ch7_en */
        uint16_t ch8_en         : 1;     /* ch8_en */
        uint16_t ch9_en         : 1;     /* ch9_en */
        uint16_t ch10_en        : 1;     /* ch10_en */
        uint16_t ch11_en        : 1;     /* ch11_en */
        uint16_t ch12_en        : 1;     /* ch12_en */
        uint16_t ch13_en        : 1;     /* ch13_en */
        uint16_t ch14_en        : 1;     /* ch14_en */
        uint16_t ch15_en        : 1;     /* ch15_en */
        uint16_t ch16_en        : 1;     /* ch16_en */
    }bit;
}_v_channel_enalbe_1_t;

/* Name : CHANNEL_ENABLE_2, access : r/w, default : 0x0000, address : 0x36 */
typedef union tag_CHANNEL_ENABLE_2
{
    uint16_t ALL;
    struct
    {
        uint16_t ch17_en        : 1;     /* ch17_en */
        uint16_t ch18_en        : 1;     /* ch18_en */
        uint16_t ch19_en        : 1;     /* ch19_en */
        uint16_t ch20_en        : 1;     /* ch20_en */
        uint16_t ch21_en        : 1;     /* ch21_en */
        uint16_t ch22_en        : 1;     /* ch22_en */
        uint16_t ch23_en        : 1;     /* ch23_en */
        uint16_t ch24_en        : 1;     /* ch24_en */
        uint16_t ch_size        : 5;     /* ld_size [4:0] */
        uint16_t                : 1;     /* reserved */
        uint16_t ld_width       : 2;     /* ld_width [1:0] */
    }bit;
}_v_channel_enable_2_t;

/* Name : FLLCNT11, access : r/w, default : 0x0000, address : 0x37 */
typedef union tag_FLLCNT11
{
    uint16_t ALL;
    struct
    {
        uint16_t fll1cnt        : 16;    /* fll1cnt [15:0] */
    }bit;
}_v_fllcnt11_t;

/* Name : FLLCNT12, access : r/w, default : 0x0000, address : 0x38 */
typedef union tag_FLLCNT12
{
    uint16_t ALL;
    struct
    {
        uint16_t fll1cnt        : 5;    /* fll1cnt [20:16] */
        uint16_t                : 3;    /* reserved */
        uint16_t fll1_err_range : 2;    /* fll1_err_range[1:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t fll1_range     : 2;    /* fll1_range[1:0] */
        uint16_t fllsync        : 1;    /* fllsync */
        uint16_t fll1_en        : 1;    /* fll1_en */
    }bit;
}_v_fllcnt12_t;

/* Name : FLLCNT21, access : r/w, default : 0x0000, address : 0x39 */
typedef union tag_FLLCNT21
{
    uint16_t ALL;
    struct
    {
        uint16_t fll2cnt        : 16;    /* fll2cnt [15:0] */
    }bit;
}_v_fllcnt21_t;

/* Name : FLLCNT22, access : r/w, default : 0x0000, address : 0x3A */
typedef union tag_FLLCNT22
{
    uint16_t ALL;
    struct
    {
        uint16_t fll2cnt        : 5;    /* fll2cnt [20:16] */
        uint16_t                : 3;    /* reserved */
        uint16_t fll2_err_range : 2;    /* fll2_err_range[1:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t fll2_range     : 2;    /* fll2_range[1:0] */
        uint16_t fllsync        : 1;    /* fllsync */
        uint16_t fll2_en        : 1;    /* fll2_en */
    }bit;
}_v_fllcnt22_t;

/* Name : VO_DELAY, access : r/w, default : 0x7000, address : 0x3B */
typedef union tag_VO_DELAY
{
    uint16_t ALL;
    struct
    {
        uint16_t vo_delay       : 8;    /* vo_delay [7:0] ; max 7.28us */
        uint16_t                : 4;    /* reserved */
        uint16_t gate1_pol      : 1;    /* gate1_pol */
        uint16_t gate2_pol      : 1;    /* gate2_pol */
        uint16_t gate3_pol      : 1;    /* gate3_pol */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_vo_delay_t;

/* Name : VO_OFF/ON, access : r/w, default : 0x0000, address : 0x3C */
typedef union tag_VO_OFF_ON
{
    uint16_t ALL;
    struct
    {
        uint16_t vo_on          : 8;     /* vo_on [7:0] ; max max 7.28us */
        uint16_t vo_off         : 8;     /* vo_off [7:0] ; max max 7.28us */
    }bit;
}_v_vo_off_on_t;

/* Name : SVO_ON, access : r/w, default : 0x0000, address : 0x3D */
typedef union tag_SVO_ON
{
    uint16_t ALL;
    struct
    {
        uint16_t svo_on         : 12;   /* svo_on [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_svo_on_t;

/* Name : SVO1_OFF, access : r/w, default : 0x0000, address : 0x3E */
typedef union tag_SVO1_OFF
{
    uint16_t ALL;
    struct
    {
        uint16_t svo1_off       : 16;    /* svo1_off [15:0] : max 1872us */
    }bit;
}_v_svo1_off_t;

/* Name : SVO2_OFF, access : r/w, default : 0x0000, address : 0x3F */
typedef union tag_SVO2_OFF
{
    uint16_t ALL;
    struct
    {
        uint16_t svo2_off       : 16;    /* svo2_off [15:0] : max 1872us */
    }bit;
}_v_svo2_off_t;

/* Name : SVO3_OFF, access : r/w, default : 0x0000, address : 0x40 */
typedef union tag_SVO3_OFF
{
    uint16_t ALL;
    struct
    {
        uint16_t svo3_off       : 16;    /* svo3_off [15:0] : max 1872us */
    }bit;
}_v_svo3_off_t;

/* Name : SVO_NUMBER, access : r/w, default : 0x0000, address : 0x41 */
typedef union tag_SVO_NUMBER
{
    uint16_t ALL;
    struct
    {
        uint16_t sv_no          : 6;    /* sv_no [5:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t sv_no_multipier : 2;    /* sv_no_multipier [1:0] */
        uint16_t                : 6;    /* reserved */
    }bit;
}_v_svo_number_t;

/* Name : DAC_NF_CONTROL, access : r/w, default : 0x0000, address : 0x42 */
typedef union tag_DAC_NF_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t dgrjt_en       : 1;    /* dgrjt_en */
        uint16_t                : 11;   /* reserved */
        uint16_t dac_lvl        : 2;    /* dac_lvl [1:0] */
        uint16_t                : 2;    /* reserved */
    }bit;
}_v_dac_nf_control_t;

/* Name : DAC_CONTROL, access : r/w, default : 0x0707, address : 0x43 */
typedef union tag_DAC_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_auto      : 1;     /* dac1_auto */
        uint16_t dac2_auto      : 1;     /* dac2_auto */
        uint16_t dac3_auto      : 1;     /* dac3_auto */
        uint16_t dac_auto_type  : 1;     /* dac_auto_type */
        uint16_t dac_sync_mode  : 1;     /* dac_sync_mode */
        uint16_t dac1_fb_mode   : 1;     /* dac1_fb_mode */
        uint16_t dac2_fb_mode   : 1;     /* dac2_fb_mode */
        uint16_t dac3_fb_mode   : 1;     /* dac3_fb_mode */
        uint16_t dac1_dec1_mode : 1;     /* dac1_dec1_mode */
        uint16_t dac2_dec1_mode : 1;     /* dac2_dec1_mode */
        uint16_t dac3_dec1_mode : 1;     /* dac3_dec1_mode */
        uint16_t                : 1;     /* reserved */
        uint16_t dac1_hold_en   : 1;     /* dac1_hold_en */
        uint16_t dac2_hold_en   : 1;     /* dac2_hold_en */
        uint16_t dac3_hold_en   : 1;     /* dac3_hold_en */
        uint16_t dac_fault_off  : 1;     /* dac_fault_off */
    }bit;
}_v_dac_control_t;

/* Name : CURRENT_TARGET_DAC1, access : r/w, default : max_limit, address : 0x44 */
typedef union tag_CURRENT_TARGET_DAC1
{
    uint16_t ALL;
    struct
    {
        uint16_t curr_tgt_dac1  : 12;   /* curr_tgt_dac1 [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_current_target_dac1_t;

/* Name : CURRENT_TARGET_DAC2, access : r/w, default : max_limit, address : 0x45 */
typedef union tag_CURRENT_TARGET_DAC2
{
    uint16_t ALL;
    struct
    {
        uint16_t curr_tgt_dac2  : 12;   /* curr_tgt_dac2 [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_current_target_dac2_t;

/* Name : CURRENT_TARGET_DAC3, access : r/w, default : max_limit, address : 0x46 */
typedef union tag_CURRENT_TARGET_DAC3
{
    uint16_t ALL;
    struct
    {
        uint16_t curr_tgt_dac3  : 12;   /* curr_tgt_dac3 [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_current_target_dac3_t;

/* Name : PREVIOUS_TARGET_DAC1, access : ro, default : max_limit, address : 0x47 */
typedef union tag_PREVIOUS_TARGET_DAC1
{
    uint16_t ALL;
    struct
    {
        uint16_t prev_tgt_dac1  : 12;   /* prev_tgt_dac1 [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_previous_target_dac1_t;

/* Name : PREVIOUS_TARGET_DAC2, access : ro, default : max_limit, address : 0x48 */
typedef union tag_PREVIOUS_TARGET_DAC2
{
    uint16_t ALL;
    struct
    {
        uint16_t prev_tgt_dac2  : 12;   /* prev_tgt_dac2 [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_previous_target_dac2_t;

/* Name : PREVIOUS_TARGET_DAC3, access : ro, default : max_limit, address : 0x49 */
typedef union tag_PREVIOUS_TARGET_DAC3
{
    uint16_t ALL;
    struct
    {
        uint16_t prev_tgt_dac3  : 12;   /* prev_tgt_dac3 [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_previous_target_dac3_t;

/* Name : DAC1_OUT, access : ro, default : max_limit, address : 0x4A */
typedef union tag_DAC1_OUT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_out       : 12;   /* dac1_out [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_dac1_out_t;

/* Name : DAC2_OUT, access : ro, default : max_limit, address : 0x4B */
typedef union tag_DAC2_OUT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_out       : 12;   /* dac2_out [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_dac2_out_t;

/* Name : DAC3_OUT, access : ro, default : max_limit, address : 0x4C */
typedef union tag_DAC3_OUT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_out       : 12;    /* dac3_out [11:0] */
        uint16_t                : 4;     /* reserved */
    }bit;
}_v_dac3_out_t;

/* Name : DAC1_STATE, access : ro, default : 0x0000, address : 0x4D */
typedef union tag_DAC1_STATE
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_st_en     : 4;    /* dac1_st_en [3:0] */
        uint16_t dac1_state     : 4;    /* dac1_state [3:0] */
        uint16_t                : 8;    /* reserved */
    }bit;
}_v_dac1_state_t;

/* Name : DAC2_STATE, access : ro, default : 0x0000, address : 0x4E */
typedef union tag_DAC2_STATE
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_st_en     : 4;    /* dac2_st_en [3:0] */
        uint16_t dac2_state     : 4;    /* dac2_state [3:0] */
        uint16_t                : 8;    /* reserved */
    }bit;
}_v_dac2_state_t;

/* Name : DAC3_STATE, access : ro, default : 0x0000, address : 0x4F */
typedef union tag_DAC3_STATE
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_st_en     : 4;    /* dac3_st_en [3:0] */
        uint16_t dac3_state     : 4;    /* dac3_state [3:0] */
        uint16_t                : 8;    /* reserved */
    }bit;
}_v_dac3_state_t;

/* Name : DAC1_INCREMENT_1, access : r/w, default : 0x40C2, address : 0x50 */
typedef union tag_DAC1_INCREMENT_1
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc1      : 5;     /* dac1_inc1 [4:0] */
        uint16_t dac1_inc2      : 5;     /* dac1_inc2 [4:0] */
        uint16_t dac1_inc3      : 6;     /* dac1_inc3 [5:0] */
    }bit;
}_v_dac1_increment_1_t;

/* Name : DAC1_INCREMENT_2_&_HOLD_LIMIT, access : r/w, default : 0x0820, address : 0x51 */
typedef union tag_DAC1_INCREMENT_2_HOLD_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc4           : 7;     /* dac1_int4 [6:0] */
        uint16_t dac1_inc_hold_limit : 9;     /* dac1_inc_hold_limit [8:0] */
    }bit;
}_v_dac1_increment_2_hold_limit_t;

/* Name : DAC1_DECREMENT_&_INC_WAIT, access : r/w, default : 0x0400, address : 0x52 */
typedef union tag_DAC1_DECREMENT_INC_WAIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc1_wait : 7;    /* dac1_inc1_wait [6:0] */
        uint16_t                : 1;    /* reserved */
        uint16_t dac1_dec1      : 7;    /* dac1_dec1 [6:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_dac1_decrement_inc_wait_t;

/* Name : DAC1_INCREMENT_HOLD_THRESOLD, access : r/w, default : 0x020A, address : 0x53 */
typedef union tag_DAC1_INCREMENT_HOLD_THRESOLD
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc_hold_threshold : 9;   /* dac1_inc_hold_threshold [8:0] */
        uint16_t bit_signed                  : 1;   /* signed */
        uint16_t                         : 6;   /* reserved */
    }bit;
}_v_dac1_increment_hold_thresold_t;

/* Name : DAC1_FB_VALID_TIMER, access : r/w, default : 0x0000, address : 0x54 */
typedef union tag_DAC1_FB_VALID_TIMER
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_fb_valid_timer : 16;    /* dac1_fb_valid_timer [17:2] */
    }bit;
}_v_dac1_fb_valid_timer_t;

/* Name : DAC1_MIN_LIMIT, access : r/w, default : 0x0190, address : 0x55 */
typedef union tag_DAC1_MIN_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_min_limit : 12;   /* dac1_min_limit [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_dac1_min_limit_t;

/* Name : DAC1_MAX_LIMIT, access : r/w, default : 0x0FA0, address : 0x56 */
typedef union tag_DAC1_MAX_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_max_limit : 12;   /* dac1_max_limit [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_dac1_max_limit_t;

/* Name : DAC2_INCREMENT_1, access : r/w, default : 0x40C2, address : 0x57 */
typedef union tag_DAC2_INCREMENT_1
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc1      : 5;     /* dac2_inc1 [4:0] */
        uint16_t dac2_inc2      : 5;     /* dac2_inc2 [4:0] */
        uint16_t dac2_inc3      : 6;     /* dac2inc3 [5:0] */
    }bit;
}_v_dac2_increment_1_t;

/* Name : DAC2_INCREMENT_2_&_HOLD_LIMIT, access : r/w, default : 0x0820, address : 0x58 */
typedef union tag_DAC2_INCREMENT_2_HOLD_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc4           : 7;     /* dac2_int4 [6:0] */
        uint16_t dac2_inc_hold_limit : 9;     /* dac2_inc_hold_limit [8:0] */
    }bit;
}_v_dac2_increment_2_hold_limit_t;

/* Name : DAC2_DECREMENT_&_INC_WAIT, access : r/w, default : 0x0400, address : 0x59 */
typedef union tag_DAC2_DECREMENT_INC_WAIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc1_wait : 7;    /* dac2_inc1_wait [6:0] */
        uint16_t                : 1;    /* reserved */
        uint16_t dac2_dec1      : 7;    /* dac2_dec1 [6:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_dac2_decrement_inc_wait_t;

/* Name : DAC2_INCREMENT_HOLD_THRESOLD, access : r/w, default : 0x020A, address : 0x5A */
typedef union tag_DAC2_INCREMENT_HOLD_THRESOLD
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc_hold_threshold : 9;   /* dac2_inc_hold_threshold [8:0] */
        uint16_t bit_signed                  : 1;   /* signed */
        uint16_t                         : 6;   /* reserved */
    }bit;
}_v_dac2_increment_hold_thresold_t;

/* Name : DAC2_FB_VALID_TIMER, access : r/w, default : 0x0000, address : 0x5B */
typedef union tag_DAC2_FB_VALID_TIMER
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_fb_valid_timer : 16;    /* dac2_fb_valid_timer [17:2] */
    }bit;
}_v_dac2_fb_valid_timer_t;

/* Name : DAC2_MIN_LIMIT, access : r/w, default : 0x0190, address : 0x5C */
typedef union tag_DAC2_MIN_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_min_limit : 12;   /* dac2_min_limit [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_dac2_min_limit_t;

/* Name : DAC2_MAX_LIMIT, access : r/w, default : 0x0FA0, address : 0x5D */
typedef union tag_DAC2_MAX_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_max_limit : 12;   /* dac2_max_limit [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_dac2_max_limit_t;

/* Name : DAC3_INCREMENT_1, access : r/w, default : 0x40C2, address : 0x5E */
typedef union tag_DAC3_INCREMENT_1
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc1      : 5;     /* dac3_inc1 [4:0] */
        uint16_t dac3_inc2      : 5;     /* dac3_inc2 [4:0] */
        uint16_t dac3_inc3      : 6;     /* dac3_inc3 [5:0] */
    }bit;
}_v_dac3_increment_1_t;

/* Name : DAC3_INCREMENT_2_&_HOLD_LIMIT, access : r/w, default : 0x0820, address : 0x5F */
typedef union tag_DAC3_INCREMENT_2_HOLD_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc4           : 7;     /* dac3_int4 [6:0] */
        uint16_t dac3_inc_hold_limit : 9;     /* dac3_inc_hold_limit [8:0] */
    }bit;
}_v_dac3_increment_2_hold_limit_t;

/* Name : DAC3_DECREMENT_&_INC_WAIT, access : r/w, default : 0x0400, address : 0x60 */
typedef union tag_DAC3_DECREMENT_INC_WAIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc1_wait : 7;    /* dac3_inc1_wait [6:0] */
        uint16_t                : 1;    /* reserved */
        uint16_t dac3_dec1      : 7;    /* dac3_dec1 [6:0] */
        uint16_t                : 1;    /* reserved */
    }bit;
}_v_dac3_decrement_inc_wait_t;

/* Name : DAC3_INCREMENT_HOLD_THRESOLD, access : r/w, default : 0x020A, address : 0x61 */
typedef union tag_DAC3_INCREMENT_HOLD_THRESOLD
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc_hold_threshold : 9;   /* dac3_inc_hold_threshold [8:0] */
        uint16_t bit_signed                  : 1;   /* signed */
        uint16_t                         : 6;   /* reserved */
    }bit;
}_v_dac3_increment_hold_thresold_t;

/* Name : DAC3_FB_VALID_TIMER, access : r/w, default : 0x0000, address : 0x62 */
typedef union tag_DAC3_FB_VALID_TIMER
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_fb_valid_timer : 16;    /* dac3_fb_valid_timer [17:2] */
    }bit;
}_v_dac3_fb_valid_timer_t;

/* Name : DAC3_MIN_LIMIT, access : r/w, default : 0x0190, address : 0x63 */
typedef union tag_DAC3_MIN_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_min_limit : 12;   /* dac3_min_limit [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_dac3_min_limit_t;

/* Name : DAC3_MAX_LIMIT, access : r/w, default : 0x0FA0, address : 0x64 */
typedef union tag_DAC3_MAX_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_max_limit : 12;   /* dac3_max_limit [11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_dac3_max_limit_t;

/* Name : OSC_FLL_MAN_A1, access : r/w, default : 0x8010, address : 0x65 */
typedef union tag_OSC_FLL_MAN_A1
{
    uint16_t ALL;
    struct
    {
        uint16_t FLT_GAIN_A     : 2;    /* FLT_GAIN_A [1:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t FLT_CTL_A      : 2;    /* FLT_CTL_A [1:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t DAC_RNG_A      : 1;    /* DAC_RNG_A */
        uint16_t                : 6;    /* reserved */
        uint16_t OSC_MAN_EN_A   : 1;    /* OSC_MAN_EN_A */
    }bit;
}_v_osc_fll_man_a1_t;

/* Name : OSC_FLL_MAN_A2, access : r/w, default : 0x8000, address : 0x66 */
typedef union tag_OSC_FLL_MAN_A2
{
    uint16_t ALL;
    struct
    {
        uint16_t OSC_FLL_MAN_A  : 16;    /* OSC_FLL_MAN_A [15:0] */
    }bit;
}_v_osc_fll_man_a2_t;

/* Name : OSC_FLL_MAN_B1, access : r/w, default : 0x8010, address : 0x67 */
typedef union tag_OSC_FLL_MAN_B1
{
    uint16_t ALL;
    struct
    {
        uint16_t FLT_GAIN_B     : 2;    /* FLT_GAIN_B[1:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t FLT_CTL_B      : 2;    /* FLT_CTL_B [1:0] */
        uint16_t                : 2;    /* reserved */
        uint16_t DAC_RNG_B      : 1;    /* DAC_RNG_B */
        uint16_t                : 6;    /* reserved */
        uint16_t OSC_MAN_EN_B   : 1;    /* OSC_MAN_EN_B */
    }bit;
}_v_osc_fll_man_b1_t;

/* Name : OSC_FLL_MAN_B2, access : r/w, default : 0x8000, address : 0x68 */
typedef union tag_OSC_FLL_MAN_B2
{
    uint16_t ALL;
    struct
    {
        uint16_t OSC_FLL_MAN_B  : 16;    /* OSC_FLL_MAN_B [15:0] */
    }bit;
}_v_osc_fll_man_b2_t;

/* Name : GLOBAL_WRITE_DATA, access : r/w, default : 0x0000, address : 0x69 */
typedef union tag_GLOBAL_WRITE_DATA
{
    uint16_t ALL;
    struct
    {
        uint16_t data           : 12;   /* data[11:0] */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_global_write_data_t;

/* Name : GLOBAL_FAULT_READ_DATA1, access : ro, default : 0x0000, address : 0x6A */
typedef union tag_GLOBAL_FAULT_READ_DATA1
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch1      : 4;     /* fault_ch1 [3:0] */
        uint16_t fault_ch2      : 4;     /* fault_ch2 [3:0] */
        uint16_t fault_ch3      : 4;     /* fault_ch3 [3:0] */
        uint16_t fault_ch4      : 4;     /* fault_ch4 [3:0] */
    }bit;
}_v_global_fault_read_data1_t;

/* Name : GLOBAL_FAULT_READ_DATA2, access : ro, default : 0x0000, address : 0x6B */
typedef union tag_GLOBAL_FAULT_READ_DATA2
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch5      : 4;     /* fault_ch5 [3:0] */
        uint16_t fault_ch6      : 4;     /* fault_ch6 [3:0] */
        uint16_t fault_ch7      : 4;     /* fault_ch7 [3:0] */
        uint16_t fault_ch8      : 4;     /* fault_ch8 [3:0] */
    }bit;
}_v_global_fault_read_data2_t;

/* Name : GLOBAL_FAULT_READ_DATA3, access : ro, default : 0x0000, address : 0x6C */
typedef union tag_GLOBAL_FAULT_READ_DATA3
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch9      : 4;     /* fault_ch9 [3:0] */
        uint16_t fault_ch10     : 4;     /* fault_ch10 [3:0] */
        uint16_t fault_ch11     : 4;     /* fault_ch11 [3:0] */
        uint16_t fault_ch12     : 4;     /* fault_ch12 [3:0] */
    }bit;
}_v_global_fault_read_data3_t;

/* Name : GLOBAL_FAULT_READ_DATA4, access : ro, default : 0x0000, address : 0x6D */
typedef union tag_GLOBAL_FAULT_READ_DATA4
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch13     : 4;     /* fault_ch13 [3:0] */
        uint16_t fault_ch14     : 4;     /* fault_ch14 [3:0] */
        uint16_t fault_ch15     : 4;     /* fault_ch15 [3:0] */
        uint16_t fault_ch16     : 4;     /* fault_ch16 [3:0] */
    }bit;
}_v_global_fault_read_data4_t;

/* Name : GLOBAL_FAULT_READ_DATA5, access : ro, default : 0x0000, address : 0x6E */
typedef union tag_GLOBAL_FAULT_READ_DATA5
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch17     : 4;     /* fault_ch17 [3:0] */
        uint16_t fault_ch18     : 4;     /* fault_ch18 [3:0] */
        uint16_t fault_ch19     : 4;     /* fault_ch19 [3:0] */
        uint16_t fault_ch20     : 4;     /* fault_ch20 [3:0] */
    }bit;
}_v_global_fault_read_data5_t;

/* Name : GLOBAL_FAULT_READ_DATA6, access : ro, default : 0x0000, address : 0x6F */
typedef union tag_GLOBAL_FAULT_READ_DATA6
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch21     : 4;     /* fault_ch21 [3:0] */
        uint16_t fault_ch22     : 4;     /* fault_ch22 [3:0] */
        uint16_t fault_ch23     : 4;     /* fault_ch23 [3:0] */
        uint16_t fault_ch24     : 4;     /* fault_ch24 [3:0] */
    }bit;
}_v_global_fault_read_data6_t;


/* PORTx LOCAL-RW DATAy(0x70 ~ 0xEF) : default 0x0000 */
typedef union tag_PORT_LOCAL_RW_DATA
{
    uint16_t ALL;
    struct
    {
        uint16_t data               : 12;   /* data[11:0] */
        uint16_t                    :  4;   /* reserved */
    }bit;
}_v_port_local_rw_data_t;

/* OTP Control */
/* Name : TEST_CONTROL, access : r/w, default : 0x0000, address : 0xF0 */
typedef union tag_TEST_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t MCLK64_O       : 1;    /* MCLK64_O */
        uint16_t MCLK_SEL       : 1;    /* MCLK_SEL */
        uint16_t                : 2;    /* reserved */
        uint16_t MCLK1_O        : 1;    /* MCLK1_O */
        uint16_t                : 3;    /* reserved */
        uint16_t DACO1_DIRECT   : 1;    /* DACO1_DIRECT */
        uint16_t DACO2_DIRECT   : 1;    /* DACO2_DIRECT */
        uint16_t DACO3_DIRECT   : 1;    /* DACO3_DIRECT */
        uint16_t                : 3;    /* reserved */
        uint16_t ADDR_EXT       : 1;    /* ADDR_EXT */
        uint16_t TEST_EN        : 1;    /* TEST_EN */
    }bit;
}_v_test_control_t;

/* Name : OTP_PG_ACCESS, access : r/w, default : 0x03FF, address : 0xF1 */
typedef union tag_OTP_PG_ACCESS
{
    uint16_t ALL;
    struct
    {
        uint16_t OTP_PG_ACC_CYCLE : 16;    /* OTP_PG_ACC_CYCLE [15:0] */
    }bit;
}_v_otp_pg_access_t;

/* Name : OTP_WRITE, access : r/w, default : 0x0004, address : 0xF2 */
typedef union tag_OTP_WRITE
{
    uint16_t ALL;
    struct
    {
        uint16_t OTP_WSEL       : 4;    /* OTP_WSEL [3:0] */
        uint16_t OTP_RD         : 2;    /* OTP_RD[1:0] */
        uint16_t                : 9;    /* reserved */
        uint16_t OTP_PG_DONE    : 1;    /* OTP_PG_DONE */
    }bit;
}_v_otp_write_t;

/* Name : OTP_RD/PROG, access : r/w, default : 0x0000, address : 0xF3 */
typedef union tag_OTP_RD_PROG
{
    uint16_t ALL;
    struct
    {
        uint16_t OTP_PG_S       : 1;    /* OTP_PG_S */
        uint16_t OTP_RD_S       : 1;    /* OTP_RD_S */
        uint16_t                : 14;   /* reserved */
    }bit;
}_v_otp_rd_prog_t;

/* Name : OTP_PROTECT, access : r/w, default : 0x05A5, address : 0xF4 */
typedef union tag_OTP_PROTECT
{
    uint16_t ALL;
    struct
    {
        uint16_t PROTECT        : 12;   /* PROTECT_EN (0x5A5), PROTECT_DIS (0xA5A) */
        uint16_t                : 4;    /* reserved */
    }bit;
}_v_otp_protect_t;

/* Name : MIRROR1, access : r/w, default : 0x1010, address : 0xF5 */
typedef union tag_MIRROR1
{
    uint16_t ALL;
    struct
    {
        uint16_t vctl_ldo       : 5;    /* vctl_ldo [4:0] */
        uint16_t                : 3;    /* reserved */
        uint16_t bgr_tc         : 5;    /* bgr_tc [4:0] */
        uint16_t                : 3;    /* reserved */
    }bit;
}_v_mirror1_t;

/* Name : MIRROR2, access : r/w, default : 0x2010, address : 0xF6 */
typedef union tag_MIRROR2
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_ofs       : 8;    /* dac1_ofs[7:0] */
        uint16_t dac_ctl        : 6;    /* dac_ctl [5:0] */
        uint16_t                : 2;    /* reserved */
    }bit;
}_v_mirror2_t;

/* Name : MIRROR3, access : r/w, default : 0x0000, address : 0xF7 */
typedef union tag_MIRROR3
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_ofs       : 8;     /* dac2_ofs[7:0] */
        uint16_t dac3_ofs       : 8;     /* dac3_ofs[7:0] */
    }bit;
}_v_mirror3_t;

/* Name : MIRROR4, access : r/w, default : 0x1010, address : 0xF8 */
typedef union tag_MIRROR4
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_rctl       : 5;    /* osc_rctl [4:0] */
        uint16_t                : 3;    /* reserved */
        uint16_t ldo_osc_ctl    : 5;    /* ldo_osc_ctl [4:0] */
        uint16_t                : 3;    /* reserved */
    }bit;
}_v_mirror4_t;

/* Name : MIRROR5, access : r/w, default : 0x1010, address : 0xF9 */
typedef union tag_MIRROR5
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_rctl2      : 5;    /* osc_rctl2 [4:0] */
        uint16_t                : 11;   /* reserved */
    }bit;
}_v_mirror5_t;

/* Name : MIRROR6, access : r/w, default : 0x0000, address : 0xFA */
typedef union tag_MIRROR6
{
    uint16_t ALL;
    struct
    {
        uint16_t version        : 2;    /* version [1:0] */
        uint16_t                : 6;    /* reserved */
        uint16_t otp_checksum   : 6;    /* otp_checksum [5:0] */
        uint16_t                : 2;    /* reserved */
    }bit;
}_v_mirror6_t;


/* Register Group 2 */
/* Name : DAC1_FB_VALID_CNT, access : ro, default : 0x0000, address : 0x00 */
typedef union tag_DAC1_FB_VALID_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_fb_valid_cnt : 16;    /* dac1_fb_valid_cnt [17:2] */
    }bit;
}_v_dac1_fb_valid_cnt_t;

/* Name : DAC1_INC_HOLD/WAIT_CNT, access : ro, default : 0x0000, address : 0x01 */
typedef union tag_DAC1_INC_HOLD_WAIT_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc1_wait_cnt      : 7;     /* dac1_inc1_wait_cnt [6:0] */
        uint16_t dac1_inc_hold_limit_cnt : 9;     /* dac1_inc_hold_limit_cnt [8:0] */
    }bit;
}_v_dac1_inc_hold_wait_cnt_t;

/* Name : 1R2, access : ro, default : 0x0000, address : 0x02 */
typedef union tag_1R2
{
    uint16_t ALL;
    struct
    {
        uint16_t bit_1r2            : 12;   /* 1r2 [ 11:0] */
        uint16_t bit_signed         : 1;    /* signed */
        uint16_t                : 3;    /* reserved */
    }bit;
}_v_1r2_t;

/* Name : SOA1_N1_&_N11, access : r/w, default : 0x1908, address : 0x03 */
typedef union tag_SOA1_N1_N11
{
    uint16_t ALL;
    struct
    {
        uint16_t soa_n1         : 7;     /* soa_n1 [6:0] */
        uint16_t soa1_n11       : 9;     /* soa1_n11 [8:0] */
    }bit;
}_v_soa1_n1_n11_t;

/* Name : SOA1_P2/P1, access : r/w, default : 0xC07D, address : 0x04 */
typedef union tag_SOA1_P2_P1
{
    uint16_t ALL;
    struct
    {
        uint16_t soa1_p1        : 9;    /* soa1_p1 [8:0] */
        uint16_t                : 3;    /* reserved */
        uint16_t soa_p2         : 4;    /* soa_p2 [3:0] */
    }bit;
}_v_soa1_p2_p1_t;

/* Name : SOA1_P3/P2, access : r/w, default : 0x7D12, address : 0x05 */
typedef union tag_SOA1_P3_P2
{
    uint16_t ALL;
    struct
    {
        uint16_t soa1_p2        : 6;     /* soa1_p2 [9:4] */
        uint16_t soa1_p3        : 10;    /* soa1_p3 [9:0] */
    }bit;
}_v_soa1_p3_p2_t;

/* Name : DAC2_FB_VALID_CNT, access : ro, default : 0x0000, address : 0x06 */
typedef union tag_DAC2_FB_VALID_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_fb_valid_cnt : 16;    /* dac2_fb_valid_cnt [17:2] */
    }bit;
}_v_dac2_fb_valid_cnt_t;

/* Name : DAC2_INC_HOLD/WAIT_CNT, access : ro, default : 0x0000, address : 0x07 */
typedef union tag_DAC2_INC_HOLD_WAIT_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc1_wait_cnt      : 7;     /* dac2_inc1_wait_cnt [6:0] */
        uint16_t dac2_inc_hold_limit_cnt : 9;     /* dac2_inc_hold_limit_cnt [8:0] */
    }bit;
}_v_dac2_inc_hold_wait_cnt_t;

/* Name : 2R2, access : ro, default : 0x0000, address : 0x08 */
typedef union tag_2R2
{
    uint16_t ALL;
    struct
    {
        uint16_t bit_2r2            : 12;   /* 2r2 [11:0] */
        uint16_t bit_signed         : 1;    /* signed */
        uint16_t                : 3;    /* reserved */
    }bit;
}_v_2r2_t;

/* Name : SOA2_N1_&_N11, access : r/w, default : 0x1908, address : 0x09 */
typedef union tag_SOA2_N1_N11
{
    uint16_t ALL;
    struct
    {
        uint16_t soa2_n1        : 7;     /* soa2_n1 [6:0] */
        uint16_t soa2_n11       : 9;     /* soa2_n11 [8:0] */
    }bit;
}_v_soa2_n1_n11_t;

/* Name : SOA2_P2/P1, access : r/w, default : 0xC07D, address : 0x0A */
typedef union tag_SOA2_P2_P1
{
    uint16_t ALL;
    struct
    {
        uint16_t soa2_p1        : 9;    /* soa2_p1 [8:0] */
        uint16_t                : 3;    /* reserved */
        uint16_t soa2_p2        : 4;    /* soa2_p2 [3:0] */
    }bit;
}_v_soa2_p2_p1_t;

/* Name : SOA2_P3/P2, access : r/w, default : 0x7D12, address : 0x0B */
typedef union tag_SOA2_P3_P2
{
    uint16_t ALL;
    struct
    {
        uint16_t soa2_p2        : 6;     /* soa2_p2 [9:4] */
        uint16_t soa2_p3        : 10;    /* soa2_p3 [9:0] */
    }bit;
}_v_soa2_p3_p2_t;

/* Name : DAC3_FB_VALID_CNT, access : ro, default : 0x0000, address : 0x0C */
typedef union tag_DAC3_FB_VALID_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_fb_valid_cnt : 16;    /* dac3_fb_valid_cnt [17:2] */
    }bit;
}_v_dac3_fb_valid_cnt_t;

/* Name : DAC3_INC_HOLD/WAIT_CNT, access : ro, default : 0x0000, address : 0x0D */
typedef union tag_DAC3_INC_HOLD_WAIT_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc1_wait_cnt      : 7;     /* dac3_inc1_wait_cnt [6:0] */
        uint16_t dac3_inc_hold_limit_cnt : 9;     /* dac3_inc_hold_limit_cnt [8:0] */
    }bit;
}_v_dac3_inc_hold_wait_cnt_t;

/* Name : 3R2, access : ro, default : 0x0000, address : 0x0E */
typedef union tag_3R2
{
    uint16_t ALL;
    struct
    {
        uint16_t bit_3r2            : 12;   /* 3r2 [11:0] */
        uint16_t bit_signed         : 1;    /* signed */
        uint16_t                : 3;    /* reserved */
    }bit;
}_v_3r2_t;

/* Name : SOA3_N1_&_N11, access : r/w, default : 0x1908, address : 0x0F */
typedef union tag_SOA3_N1_N11
{
    uint16_t ALL;
    struct
    {
        uint16_t soa3_n1        : 7;     /* soa3_n1 [6:0] */
        uint16_t soa3_n11       : 9;     /* soa3_n11 [8:0] */
    }bit;
}_v_soa3_n1_n11_t;

/* Name : SOA3_P2/P1, access : r/w, default : 0xC07D, address : 0x10 */
typedef union tag_SOA3_P2_P1
{
    uint16_t ALL;
    struct
    {
        uint16_t soa3_p1        : 9;    /* soa3_p1 [8:0] */
        uint16_t                : 3;    /* reserved */
        uint16_t soa3_p2        : 4;    /* soa3_p2 [3:0] */
    }bit;
}_v_soa3_p2_p1_t;

/* Name : SOA3_P3/P2, access : r/w, default : 0x7D12, address : 0x11 */
typedef union tag_SOA3_P3_P2
{
    uint16_t ALL;
    struct
    {
        uint16_t soa3_p2        : 6;     /* soa3_p2 [9:4] */
        uint16_t soa3_p3        : 10;    /* soa3_p3 [9:0] */
    }bit;
}_v_soa3_p3_p2_t;

/* Name : ANA_TEST, access : r/w, default : 0x0000, address : 0x12 */
typedef union tag_ANA_TEST
{
    uint16_t ALL;
    struct
    {
        uint16_t TEST_ANA_EN    : 3;    /* TEST_ANA_EN[2:0] */
        uint16_t                : 1;    /* reserved */
        uint16_t CHOP_EN_BGR    : 1;    /* CHOP_EN_BGR */
        uint16_t                : 3;    /* reserved */
        uint16_t CHOP_EN_OSCLDO : 1;    /* CHOP_EN_OSCLDO */
        uint16_t                : 3;    /* reserved */
        uint16_t CHOP_EN        : 1;    /* CHOP_EN */
        uint16_t                : 3;    /* reserved */
    }bit;
}_v_ana_test_t;

/* Name : A&D_TEST(NEW), access : r/w, default : 0x0000, address : 0x13 */
typedef union tag_A_D_TEST
{
    uint16_t ALL;
    struct
    {
        uint16_t tm_osc_stuck   : 1;    /* tm_osc_stuck */
        uint16_t tm_bgr_ok      : 1;    /* tm_bgr_ok */
        uint16_t tm_ldo_pad     : 1;    /* tm_ldo_pad */
        uint16_t tm_ldo_ok      : 1;    /* tm_ldo_ok */
        uint16_t DMUX_EN        : 1;    /* DMUX_EN */
        uint16_t AMUX_EN        : 1;    /* AMUX_EN */
        uint16_t tm_gate_ctrl_i : 1;    /* tm_gate_ctrl_input_flip */
        uint16_t                : 1;    /* reserved */
        uint16_t DMUX_SEL       : 3;    /* DMUX_SEL [2:0] */
        uint16_t                : 1;    /* reserved */
        uint16_t AMUX_SEL       : 2;    /* AMUX_SEL [1:0] */
        uint16_t                : 2;    /* reserved */
    }bit;
}_v_a_d_test_t;


typedef union _xcr_Group1_regs
{
    uint16_t ALL[XCR_GRP1_MAX];
    struct
    {
        _v_reset_t                                  _r00;
        _v_global_write_command_t                   _r01;
        _v_local_write_command_t                    _r02;
        _v_local_read_command_t                     _r03;
        _v_id_gen_command_t                         _r04;
        _v_fault_read_command_t                     _r05;
        _v_ld_transfer_command_t                    _r06;
        _v_sync_gen_command_t                       _r07;
        _v_command_auto_enable_t                    _r08;
        _v_ld_write_pointer_t                       _r09;   /* Read-only */
        _v_ld_read_pointer_t                        _r0A;   /* Read-only */
        _v_ld_difference_pointer_t                  _r0B;   /* Read-only */
        _v_ld_start_pointer_th_t                    _r0C;
        _v_local_write_transfer_pointer_t           _r0D;   /* Read-only */
        _v_local_read_receive_pointer_t             _r0E;   /* Read-only */
        _v_local_rw_difference_pointer_t            _r0F;   /* Read-only */
        _v_local_rw_pointer_reset_t                 _r10;   /* Write-only */
        _v_fault_auto_read_interval_t               _r11;
        _v_fault_auto_read_event_t                  _r12;
        _v_interrupt_enable_t                       _r13;
        _v_command_status_1_t                       _r14;   /* Read-only */
        _v_command_status_2_t                       _r15;   /* Read-only */
        _v_receive_status_t                         _r16;   /* Read-only */
        _v_interrupt_status_t                       _r17;   /* Read-only */
        _v_err_status_t                             _r18;   /* Read-only */
        _v_ch_crc_err_status_t                      _r19;   /* Read-only */
        _v_ch_timeout_err_status_t                  _r1A;   /* Read-only */
        _v_bist_control_status_t                    _r1B;
        _v_communication_fault_control_t            _r1C;
        _v_clk_control_1_t                          _r1D;
        _v_clk_control_2_t                          _r1E;
        _v_serializer_clock_gen_t                   _r1F;
        _v_latency_t                                _r20;
        _v_timeout_t                                _r21;
        _v_daisied_device_ch_size_t                 _r22;
        _v_daisy_size_1_t                           _r23;
        _v_daisy_size_2_t                           _r24;
        _v_daisy_size_3_t                           _r25;
        _v_daisy_size_4_t                           _r26;
        _v_daisy_size_5_t                           _r27;
        _v_daisy_size_6_t                           _r28;
        //_v_daisy_size_7_t                           _r27;
        //_v_daisy_size_8_t                           _r28;
        _v_block_size_1_t                           _r29;
        _v_block_size_2_t                           _r2A;
        _v_block_size_3_t                           _r2B;
        _v_block_size_4_t                           _r2C;
        _v_block_size_5_t                           _r2D;
        _v_block_size_6_t                           _r2E;
        _v_block_size_7_t                           _r2F;
        _v_block_size_8_t                           _r30;
        _v_block_size_9_t                           _r31;
        _v_block_size_10_t                          _r32;
        _v_block_size_11_t                          _r33;
        _v_block_size_12_t                          _r34;
        _v_channel_enalbe_1_t                       _r35;
        _v_channel_enable_2_t                       _r36;
        _v_fllcnt11_t                               _r37;
        _v_fllcnt12_t                               _r38;
        _v_fllcnt21_t                               _r39;
        _v_fllcnt22_t                               _r3A;
        _v_vo_delay_t                               _r3B;
        _v_vo_off_on_t                              _r3C;
        _v_svo_on_t                                 _r3D;
        _v_svo1_off_t                               _r3E;
        _v_svo2_off_t                               _r3F;
        _v_svo3_off_t                               _r40;
        _v_svo_number_t                             _r41;
        _v_dac_nf_control_t                         _r42;
        _v_dac_control_t                            _r43;
        _v_current_target_dac1_t                    _r44;
        _v_current_target_dac2_t                    _r45;
        _v_current_target_dac3_t                    _r46;
        _v_previous_target_dac1_t                   _r47;   /* Read-only */
        _v_previous_target_dac2_t                   _r48;   /* Read-only */
        _v_previous_target_dac3_t                   _r49;   /* Read-only */
        _v_dac1_out_t                               _r4A;   /* Read-only */
        _v_dac2_out_t                               _r4B;   /* Read-only */
        _v_dac3_out_t                               _r4C;   /* Read-only */
        _v_dac1_state_t                             _r4D;   /* Read-only */
        _v_dac2_state_t                             _r4E;   /* Read-only */
        _v_dac3_state_t                             _r4F;   /* Read-only */
        _v_dac1_increment_1_t                       _r50;
        _v_dac1_increment_2_hold_limit_t            _r51;
        _v_dac1_decrement_inc_wait_t                _r52;
        _v_dac1_increment_hold_thresold_t           _r53;
        _v_dac1_fb_valid_timer_t                    _r54;
        _v_dac1_min_limit_t                         _r55;
        _v_dac1_max_limit_t                         _r56;
        _v_dac2_increment_1_t                       _r57;
        _v_dac2_increment_2_hold_limit_t            _r58;
        _v_dac2_decrement_inc_wait_t                _r59;
        _v_dac2_increment_hold_thresold_t           _r5A;
        _v_dac2_fb_valid_timer_t                    _r5B;
        _v_dac2_min_limit_t                         _r5C;
        _v_dac2_max_limit_t                         _r5D;
        _v_dac3_increment_1_t                       _r5E;
        _v_dac3_increment_2_hold_limit_t            _r5F;
        _v_dac3_decrement_inc_wait_t                _r60;
        _v_dac3_increment_hold_thresold_t           _r61;
        _v_dac3_fb_valid_timer_t                    _r62;
        _v_dac3_min_limit_t                         _r63;
        _v_dac3_max_limit_t                         _r64;
        _v_osc_fll_man_a1_t                         _r65;
        _v_osc_fll_man_a2_t                         _r66;
        _v_osc_fll_man_b1_t                         _r67;
        _v_osc_fll_man_b2_t                         _r68;
        _v_global_write_data_t                      _r69;
        _v_global_fault_read_data1_t                _r6A;   /* Read-only */
        _v_global_fault_read_data2_t                _r6B;   /* Read-only */
        _v_global_fault_read_data3_t                _r6C;   /* Read-only */
        _v_global_fault_read_data4_t                _r6D;   /* Read-only */
        _v_global_fault_read_data5_t                _r6E;   /* Read-only */
        _v_global_fault_read_data6_t                _r6F;   /* Read-only */
    }reg;
}_xcr_group1_regs_t;

static const char* xcr_grp1_regs_str[XCR_GRP1_MAX] =
{
    "RESET",
    "GLOBAL WRITE COMMAND",
    "LOCAL WRITE COMMAND",
    "LOCAL READ COMMAND",
    "ID GEN COMMAND",
    "FAULT READ COMMAND",
    "LD TRANSFER COMMAND",
    "SYNC GEN COMMAND",
    "COMMAND AUTO ENABLE",
    "LD WRITE POINTER",
    "LD READ POINTER",
    "LD DIFFERENCE POINTER",
    "LD START POINTER/TH",
    "LOCAL WRITE TRANSFER POINTER",
    "LOCAL READ RECEIVE POINTER",
    "LOCAL RW DIFFERENCE POINTER",
    "LOCAL RW POINTER RESET",
    "FAULT AUTO READ INTERVAL",
    "FAULT AUTO READ EVENT",
    "INTERRUPT ENABLE",
    "COMMAND STATUS 1",
    "COMMAND STATUS 2",
    "RECEIVE STATUS",
    "INTERRUPT STATUS",
    "ERR STATUS",
    "CH CRC ERR STATUS",
    "CH TIMEOUT ERR STATUS",
    "BIST CONTROL & STATUS",
    "COMMUNICATION FAULT CONTROL",
    "CLK CONTROL 1",
    "CLK CONTROL 2",
    "SERIALIZER CLOCK GEN",
    "LATENCY",
    "TIMEOUT",
    "DAISIED DEVICE CH SIZE",
    "DAISY SIZE 1",
    "DAISY SIZE 2",
    "DAISY SIZE 3",
    "DAISY SIZE 4",
    "DAISY SIZE 5",
    "DAISY SIZE 6",
    //"DAISY SIZE 7",
    //"DAISY SIZE 8",
    "BLOCK SIZE 1",
    "BLOCK SIZE 2",
    "BLOCK SIZE 3",
    "BLOCK SIZE 4",
    "BLOCK SIZE 5",
    "BLOCK SIZE 6",
    "BLOCK SIZE 7",
    "BLOCK SIZE 8",
    "BLOCK SIZE 9",
    "BLOCK SIZE 10",
    "BLOCK SIZE 11",
    "BLOCK SIZE 12",
    "CHANNEL ENABLE 1",
    "CHANNEL ENABLE 2",
    "FLLCNT11",
    "FLLCNT12",
    "FLLCNT21",
    "FLLCNT22",
    "VO DELAY",
    "VO OFF/ON",
    "SVO ON",
    "SVO1 OFF",
    "SVO2 OFF",
    "SVO3 OFF",
    "SVO NUMBER",
    "DAC NF CONTROL",
    "DAC CONTROL",
    "CURRENT TARGET DAC1",
    "CURRENT TARGET DAC2",
    "CURRENT TARGET DAC3",
    "PREVIOUS TARGET DAC1",
    "PREVIOUS TARGET DAC2",
    "PREVIOUS TARGET DAC3",
    "DAC1 OUT",
    "DAC2 OUT",
    "DAC3 OUT",
    "DAC1 STATE",
    "DAC2 STATE",
    "DAC3 STATE",
    "DAC1 INCREMENT 1",
    "DAC1 INCREMENT 2 & HOLD_LIMIT",
    "DAC1 DECREMENT & INC WAIT",
    "DAC1 INCREMENT HOLD THRESHOLD",
    "DAC1 FB VALID TIMER",
    "DAC1 MIN LIMIT",
    "DAC1 MAX LIMIT",
    "DAC2 INCREMENT 1",
    "DAC2 INCREMENT 2 & HOLD_LIMIT",
    "DAC2 DECREMENT & INC WAIT",
    "DAC2 INCREMENT HOLD THRESHOLD",
    "DAC2 FB VALID TIMER",
    "DAC2 MIN LIMIT",
    "DAC2 MAX LIMIT",
    "DAC3 INCREMENT 1",
    "DAC3 INCREMENT 2 & HOLD_LIMIT",
    "DAC3 DECREMENT & INC WAIT",
    "DAC3 INCREMENT HOLD THRESHOLD",
    "DAC3 FB VALID TIMER",
    "DAC3 MIN LIMIT",
    "DAC3 MAX LIMIT",
    "OSC FLL MAN_A1",
    "OSC FLL MAN_A2",
    "OSC FLL MAN_B1",
    "OSC FLL MAN_B2",
    "GLOBAL WRITE DATA",
    "GLOBAL FAULT READ DATA1",
    "GLOBAL FAULT READ DATA2",
    "GLOBAL FAULT READ DATA3",
    "GLOBAL FAULT READ DATA4",
    "GLOBAL FAULT READ DATA5",
    "GLOBAL FAULT READ DATA6",
};

typedef union _xcr_otp_control_regs
{
    uint16_t ALL[XCR_OTP_MAX];
    struct
    {
        _v_test_control_t       _rF0;
        _v_otp_pg_access_t      _rF1;
        _v_otp_write_t          _rF2;
        _v_otp_rd_prog_t        _rF3;
        _v_otp_protect_t        _rF4;
        _v_mirror1_t            _rF5;
        _v_mirror2_t            _rF6;
        _v_mirror3_t            _rF7;
        _v_mirror4_t            _rF8;
        _v_mirror5_t            _rF9;
        _v_mirror6_t            _rFA;
    }reg;
}_xcr_otp_control_regs_t;

static const char* xcr_otp_control_regs_str[XCR_GRP2_MAX] =
{
    "TEST CONTROL",
    "OTP PG ACCESS",
    "OTP WRITE",
    "OTP RD/PROG",
    "OTP PROTECT",
    "MIRROR1",
    "MIRROR2",
    "MIRROR3",
    "MIRROR4",
    "MIRROR5",
    "MIRROR6",
};

typedef union _xcr_Group2_regs
{
    uint16_t ALL[XCR_GRP2_MAX];
    struct
    {
        _v_dac1_fb_valid_cnt_t                      _r00;   /* Read-only */
        _v_dac1_inc_hold_wait_cnt_t                 _r01;   /* Read-only */
        _v_1r2_t                                    _r02;   /* Read-only */
        _v_soa1_n1_n11_t                            _r03;
        _v_soa1_p2_p1_t                             _r04;
        _v_soa1_p3_p2_t                             _r05;
        _v_dac2_fb_valid_cnt_t                      _r06;   /* Read-only */
        _v_dac2_inc_hold_wait_cnt_t                 _r07;   /* Read-only */
        _v_2r2_t                                    _r08;   /* Read-only */
        _v_soa2_n1_n11_t                            _r09;
        _v_soa2_p2_p1_t                             _r0A;
        _v_soa2_p3_p2_t                             _r0B;
        _v_dac3_fb_valid_cnt_t                      _r0C;   /* Read-only */
        _v_dac3_inc_hold_wait_cnt_t                 _r0D;   /* Read-only */
        _v_3r2_t                                    _r0E;   /* Read-only */
        _v_soa3_n1_n11_t                            _r0F;
        _v_soa3_p2_p1_t                             _r10;
        _v_soa3_p3_p2_t                             _r11;
        _v_ana_test_t                               _r12;
        _v_a_d_test_t                               _r13;
    }reg;
}_xcr_group2_regs_t;

static const char* xcr_grp2_regs_str[XCR_GRP2_MAX] =
{
    "DAC1 FB VALID CNT",
    "DAC1 INC HOLD/WAIT CNT",
    "1R2",
    "SOA1_N1 & N11",
    "SOA1_P2/P1",
    "SOA1_P3/P2",
    "DAC2 FB VALID CNT",
    "DAC2 INC HOLD/WAIT CNT",
    "2R2",
    "SOA2_N1 & N11",
    "SOA2_P2/P1",
    "SOA2_P3/P2",
    "DAC3 FB VALID CNT",
    "DAC3 INC HOLD/WAIT CNT",
    "3R2",
    "SOA3_N1 & N11",
    "SOA3_P2/P1",
    "SOA3_P3/P2",
    "ANA_TEST",
    "A&D_TEST(new)",
};

#ifdef __cplusplus
}
#endif

#endif	/* _IC603_STRUCT_H_ */

