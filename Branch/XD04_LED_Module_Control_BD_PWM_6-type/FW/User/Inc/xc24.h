/** @file xc24.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XC24_H__
#define __XC24_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CMD_CODE_NONE       0x00U    /* 0b00 : No operation */
#define CMD_CODE_REG_READ   0x01U    /* 0b01 : Register-read */
#define CMD_CODE_REG_WRITE  0x02U    /* 0b10 : Register-write */
#define CMD_CODE_LD_TRANS   0x03U    /* 0b11 : Local dimming data transfer */

#define CH_SEG_1            0x00U    /* CH01 ~ CH08 */
#define CH_SEG_2            0x01U    /* CH09 ~ CH16 */
#define CH_SEG_3            0x20U    /* CH17 ~ CH24 */

typedef union tag_XC24_CMD
{
    uint16_t ALL;
    struct
    {
        uint16_t size   :  6;   /* BURST SIZE : Amount of data to transfer or receive */
        uint16_t addr   :  8;   /* START ADDRESS : The start address of registers to access */
        uint16_t code   :  2;   /* COMMAND CODE : */
    };
}_xc24_cmd_t;

///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// DATA FORMAT ///////////////////////////////////////
typedef enum tag_XC24_ADDR_T
{
    XC24_ADDR_SOFT_RESET                    = 0x00U, // 0x00
    XC24_ADDR_GLOBAL_WRITE                  = 0x01U, // 0x01
    XC24_ADDR_LOCAL_WRITE                   = 0x02U, // 0x02
    XC24_ADDR_LOCAL_READ                    = 0x03U, // 0x03
    XC24_ADDR_ID_GEN                        = 0x04U, // 0x04
    XC24_ADDR_FAULT_READ                    = 0x05U, // 0x05
    XC24_ADDR_LD_TRANSFER                   = 0x06U, // 0x06
    XC24_ADDR_SYNC_GEN                      = 0x07U, // 0x07
    XC24_ADDR_AUTO_ENABLE                   = 0x08U, // 0x08
    XC24_ADDR_DUMMY_09                      = 0x09U, // 0x09
    XC24_ADDR_LD_WRITE_POINTER              = 0x0AU, // 0x0A
    XC24_ADDR_LD_READ_POINTER               = 0x0BU, // 0x0B
    XC24_ADDR_DIFFERENCE_POINTER            = 0x0CU, // 0x0C
    XC24_ADDR_LD_TRANSFER_START_POINTER_TH  = 0x0DU, // 0x0D
    XC24_ADDR_LOCAL_WR_TRANSFER_POINTER     = 0x0EU, // 0x0E
    XC24_ADDR_LOCAL_RD_RECEIVE_POINTER      = 0x0FU, // 0x0F

    XC24_ADDR_LOCAL_RW_DIFFERENCE_POINTER   = 0x10U, // 0x10
    XC24_ADDR_LOCAL_RW_POINTER_RESET        = 0x11U, // 0x11
    XC24_ADDR_FAULT_AUTO_READ_TIMER         = 0x12U, // 0x12
    XC24_ADDR_FAULT_AUTO_READ_EVENT         = 0x13U, // 0x13
    XC24_ADDR_SERIALIZER_CLOCK_GEN          = 0x14U, // 0x14
    XC24_ADDR_INTERRUPT_ENABLE              = 0x15U, // 0x15
    XC24_ADDR_COMMAND_STATUS1               = 0x16U, // 0x16
    XC24_ADDR_COMMAND_STATUS2               = 0x17U, // 0x17
    XC24_ADDR_RECEIVE_STATUS                = 0x18U, // 0x18
    XC24_ADDR_INTERRUPT_STATUS              = 0x19U, // 0x19
    XC24_ADDR_SPI_FAULT_STATUS_CONTROL      = 0x1AU, // 0x1A
    XC24_ADDR_CLK_CONTROL_1                 = 0x1BU, // 0x1B
    XC24_ADDR_CLK_CONTROL_2                 = 0x1CU, // 0x1C
    XC24_ADDR_VDD_LDO_STATUS                = 0x1DU, // 0x1D
    XC24_ADDR_VDD_LDO_FAULT_LEVEL           = 0x1EU, // 0x1E
    XC24_ADDR_COMMAND_LATENCY               = 0x1FU, // 0x1F

    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1  = 0x20U, // 0x20
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE2  = 0x21U, // 0x21
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE3  = 0x22U, // 0x22
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE4  = 0x23U, // 0x23
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE5  = 0x24U, // 0x24
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE6  = 0x25U, // 0x25
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE7  = 0x26U, // 0x26
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE8  = 0x27U, // 0x27
    XC24_ADDR_DUMMY_28                      = 0x28U, // 0x28
    XC24_ADDR_DUMMY_29                      = 0x29U, // 0x29
    XC24_ADDR_DUMMY_2A                      = 0x2AU, // 0x2A
    XC24_ADDR_DUMMY_2B                      = 0x2BU, // 0x2B
    XC24_ADDR_DUMMY_2C                      = 0x2CU, // 0x2C
    XC24_ADDR_DUMMY_2D                      = 0x2DU, // 0x2D
    XC24_ADDR_DUMMY_2E                      = 0x2EU, // 0x2E
    XC24_ADDR_DUMMY_2F                      = 0x2FU, // 0x2F

    XC24_ADDR_DAISY_SIZE1                   = 0x30U, // 0x30
    XC24_ADDR_DAISY_SIZE2                   = 0x31U, // 0x31
    XC24_ADDR_DAISY_SIZE3                   = 0x32U, // 0x32
    XC24_ADDR_DAISY_SIZE4                   = 0x33U, // 0x33
    XC24_ADDR_DAISY_SIZE5                   = 0x34U, // 0x34
    XC24_ADDR_DAISY_SIZE6                   = 0x35U, // 0x35
    XC24_ADDR_DAISY_SIZE7                   = 0x36U, // 0x36
    XC24_ADDR_DAISY_SIZE8                   = 0x37U, // 0x37
    XC24_ADDR_BLOCK_SIZE1                   = 0x38U, // 0x38
    XC24_ADDR_BLOCK_SIZE2                   = 0x39U, // 0x39
    XC24_ADDR_BLOCK_SIZE3                   = 0x3AU, // 0x3A
    XC24_ADDR_BLOCK_SIZE4                   = 0x3BU, // 0x3B
    XC24_ADDR_BLOCK_SIZE5                   = 0x3CU, // 0x3C
    XC24_ADDR_BLOCK_SIZE6                   = 0x3DU, // 0x3D
    XC24_ADDR_BLOCK_SIZE7                   = 0x3EU, // 0x3E
    XC24_ADDR_BLOCK_SIZE8                   = 0x3FU, // 0x3F

    XC24_ADDR_BLOCK_SIZE9                   = 0x40U, // 0x40
    XC24_ADDR_BLOCK_SIZE10                  = 0x41U, // 0x41
    XC24_ADDR_BLOCK_SIZE11                  = 0x42U, // 0x42
    XC24_ADDR_BLOCK_SIZE12                  = 0x43U, // 0x43
    XC24_ADDR_DUMMY_44                      = 0x44U, // 0x44
    XC24_ADDR_CHANNEL_ENABLE1               = 0x45U, // 0x45
    XC24_ADDR_CHANNEL_ENABLE2               = 0x46U, // 0x46
    XC24_ADDR_DUMMY_47                      = 0x47U, // 0x47
    XC24_ADDR_DUMMY_48                      = 0x48U, // 0x48
    XC24_ADDR_DUMMY_49                      = 0x49U, // 0x49
    XC24_ADDR_DUMMY_4A                      = 0x4AU, // 0x4A
    XC24_ADDR_DUMMY_4B                      = 0x4BU, // 0x4B
    XC24_ADDR_DUMMY_4C                      = 0x4CU, // 0x4C
    XC24_ADDR_DAC_CONTROL                   = 0x4DU, // 0x4D
    XC24_ADDR_DUMMY_4E                      = 0x4EU, // 0x4E
    XC24_ADDR_CURRENT_TARGET_DAC            = 0x4FU, // 0x4F

    XC24_ADDR_PREVIOUS_TARGET_DAC           = 0x50U, // 0x50
    XC24_ADDR_DAC_OUT                       = 0x51U, // 0x51
    XC24_ADDR_DAC_INCREMENT1                = 0x52U, // 0x52
    XC24_ADDR_DAC_INCREMENT2_HOLD_LIMIT     = 0x53U, // 0x53
    XC24_ADDR_DAC_DECREMENT_INC_WAIT        = 0x54U, // 0x54
    XC24_ADDR_DAC_INCREMENT_HOLD_TH         = 0x55U, // 0x55
    XC24_ADDR_SOA_N11_N1                    = 0x56U, // 0x56
    XC24_ADDR_SOA_P2_P1                     = 0x57U, // 0x57
    XC24_ADDR_SOA_P3_P2                     = 0x58U, // 0x58
    XC24_ADDR_DAC_FB_VALID_TIMER            = 0x59U, // 0x59
    XC24_ADDR_DAC_MIN_LIMIT                 = 0x5AU, // 0x5A
    XC24_ADDR_DAC_MAX_LIMIT                 = 0x5BU, // 0x5B
    XC24_ADDR_DAC_STATE                     = 0x5CU, // 0x5C
    XC24_ADDR_VALID_CNT                     = 0x5DU, // 0x5D
    XC24_ADDR_DAC_INC_HOLD_WAIT_CNT         = 0x5EU, // 0x5E
    XC24_ADDR_R2                            = 0x5FU, // 0x5F

    XC24_ADDR_GLOBAL_WRITE_DATA             = 0x60U, // 0x60
    XC24_ADDR_GLOBAL_FAULT_READ_DATA1       = 0x61U, // 0x61
    XC24_ADDR_GLOBAL_FAULT_READ_DATA2       = 0x62U, // 0x62
    XC24_ADDR_GLOBAL_FAULT_READ_DATA3       = 0x63U, // 0x63
    XC24_ADDR_GLOBAL_FAULT_READ_DATA4       = 0x64U, // 0x64
    XC24_ADDR_GLOBAL_FAULT_READ_DATA5       = 0x65U, // 0x65
    XC24_ADDR_GLOBAL_FAULT_READ_DATA6       = 0x66U, // 0x66
    XC24_ADDR_DUMMY_67                      = 0x67U, // 0x67
    XC24_ADDR_DUMMY_68                      = 0x68U, // 0x68
    XC24_ADDR_DUMMY_69                      = 0x69U, // 0x69
    XC24_ADDR_DUMMY_6A                      = 0x6AU, // 0x6A
    XC24_ADDR_DUMMY_6B                      = 0x6BU, // 0x6B
    XC24_ADDR_DUMMY_6C                      = 0x6CU, // 0x6C
    XC24_ADDR_DUMMY_6D                      = 0x6DU, // 0x6D
    XC24_ADDR_DUMMY_6E                      = 0x6EU, // 0x6E
    XC24_ADDR_DUMMY_6F                      = 0x6FU, // 0x6F
    XC24_ADDR_MAX                           = 0x70U, // 0x70
}xc24_addr_t;

typedef enum tag_XC24_MIRROR_ADDR_T
{
    XC24_MIRROR_ADDR_START                  = 0xF0U, // 0xF0
    XC24_MIRROR_ADDR_TEST_CONTROL           = 0xF0U, // 0xF0
    XC24_MIRROR_ADDR_OTP_PG_ACCESS          = 0xF1U, // 0xF1
    XC24_MIRROR_ADDR_OTP_WRITE              = 0xF2U, // 0xF2
    XC24_MIRROR_ADDR_OTP_RD_PROG            = 0xF3U, // 0xF3
    XC24_MIRROR_ADDR_OTP_PROTECT            = 0xF4U, // 0xF4
    XC24_MIRROR_ADDR_MIRROR1                = 0xF5U, // 0xF5
    XC24_MIRROR_ADDR_MIRROR2                = 0xF6U, // 0xF6
    XC24_MIRROR_ADDR_MIRROR3                = 0xF7U, // 0xF7
    XC24_MIRROR_ADDR_MAX                    = 0xF8U, // 0xF8
}xc24_mirror_addr_t;

typedef enum _xc24_data_addr_
{
    XC24_ADDR_PORT1_LOCAL_RW_DATA1  = 0x70U, // 0x70
    XC24_ADDR_PORT2_LOCAL_RW_DATA1  = 0x71U, // 0x71
    XC24_ADDR_PORT3_LOCAL_RW_DATA1  = 0x72U, // 0x72
    XC24_ADDR_PORT4_LOCAL_RW_DATA1  = 0x73U, // 0x73
    XC24_ADDR_PORT5_LOCAL_RW_DATA1  = 0x74U, // 0x74
    XC24_ADDR_PORT6_LOCAL_RW_DATA1  = 0x75U, // 0x75
    XC24_ADDR_PORT7_LOCAL_RW_DATA1  = 0x76U, // 0x76
    XC24_ADDR_PORT8_LOCAL_RW_DATA1  = 0x77U, // 0x77

    XC24_ADDR_PORT1_LOCAL_RW_DATA2  = 0x78U, // 0x78
    XC24_ADDR_PORT2_LOCAL_RW_DATA2  = 0x79U, // 0x79
    XC24_ADDR_PORT3_LOCAL_RW_DATA2  = 0x7AU, // 0x7A
    XC24_ADDR_PORT4_LOCAL_RW_DATA2  = 0x7BU, // 0x7B
    XC24_ADDR_PORT5_LOCAL_RW_DATA2  = 0x7CU, // 0x7C
    XC24_ADDR_PORT6_LOCAL_RW_DATA2  = 0x7DU, // 0x7D
    XC24_ADDR_PORT7_LOCAL_RW_DATA2  = 0x7EU, // 0x7E
    XC24_ADDR_PORT8_LOCAL_RW_DATA2  = 0x7FU, // 0x7F

    XC24_ADDR_PORT1_LOCAL_RW_DATA3  = 0x80U, // 0x80
    XC24_ADDR_PORT2_LOCAL_RW_DATA3  = 0x81U, // 0x81
    XC24_ADDR_PORT3_LOCAL_RW_DATA3  = 0x82U, // 0x82
    XC24_ADDR_PORT4_LOCAL_RW_DATA3  = 0x83U, // 0x83
    XC24_ADDR_PORT5_LOCAL_RW_DATA3  = 0x84U, // 0x84
    XC24_ADDR_PORT6_LOCAL_RW_DATA3  = 0x85U, // 0x85
    XC24_ADDR_PORT7_LOCAL_RW_DATA3  = 0x86U, // 0x86
    XC24_ADDR_PORT8_LOCAL_RW_DATA3  = 0x87U, // 0x87

    XC24_ADDR_PORT1_LOCAL_RW_DATA4  = 0x88U, // 0x88
    XC24_ADDR_PORT2_LOCAL_RW_DATA4  = 0x89U, // 0x89
    XC24_ADDR_PORT3_LOCAL_RW_DATA4  = 0x8AU, // 0x8A
    XC24_ADDR_PORT4_LOCAL_RW_DATA4  = 0x8BU, // 0x8B
    XC24_ADDR_PORT5_LOCAL_RW_DATA4  = 0x8CU, // 0x8C
    XC24_ADDR_PORT6_LOCAL_RW_DATA4  = 0x8DU, // 0x8D
    XC24_ADDR_PORT7_LOCAL_RW_DATA4  = 0x8EU, // 0x8E
    XC24_ADDR_PORT8_LOCAL_RW_DATA4  = 0x8FU, // 0x8F

    XC24_ADDR_PORT1_LOCAL_RW_DATA5  = 0x90U, // 0x90
    XC24_ADDR_PORT2_LOCAL_RW_DATA5  = 0x91U, // 0x91
    XC24_ADDR_PORT3_LOCAL_RW_DATA5  = 0x92U, // 0x92
    XC24_ADDR_PORT4_LOCAL_RW_DATA5  = 0x93U, // 0x93
    XC24_ADDR_PORT5_LOCAL_RW_DATA5  = 0x94U, // 0x94
    XC24_ADDR_PORT6_LOCAL_RW_DATA5  = 0x95U, // 0x95
    XC24_ADDR_PORT7_LOCAL_RW_DATA5  = 0x96U, // 0x96
    XC24_ADDR_PORT8_LOCAL_RW_DATA5  = 0x97U, // 0x97

    XC24_ADDR_PORT1_LOCAL_RW_DATA6  = 0x98U, // 0x98
    XC24_ADDR_PORT2_LOCAL_RW_DATA6  = 0x99U, // 0x99
    XC24_ADDR_PORT3_LOCAL_RW_DATA6  = 0x9AU, // 0x9A
    XC24_ADDR_PORT4_LOCAL_RW_DATA6  = 0x9BU, // 0x9B
    XC24_ADDR_PORT5_LOCAL_RW_DATA6  = 0x9CU, // 0x9C
    XC24_ADDR_PORT6_LOCAL_RW_DATA6  = 0x9DU, // 0x9D
    XC24_ADDR_PORT7_LOCAL_RW_DATA6  = 0x9EU, // 0x9E
    XC24_ADDR_PORT8_LOCAL_RW_DATA6  = 0x9FU, // 0x9F

    XC24_ADDR_PORT1_LOCAL_RW_DATA7  = 0xA0U, // 0xA0
    XC24_ADDR_PORT2_LOCAL_RW_DATA7  = 0xA1U, // 0xA1
    XC24_ADDR_PORT3_LOCAL_RW_DATA7  = 0xA2U, // 0xA2
    XC24_ADDR_PORT4_LOCAL_RW_DATA7  = 0xA3U, // 0xA3
    XC24_ADDR_PORT5_LOCAL_RW_DATA7  = 0xA4U, // 0xA4
    XC24_ADDR_PORT6_LOCAL_RW_DATA7  = 0xA5U, // 0xA5
    XC24_ADDR_PORT7_LOCAL_RW_DATA7  = 0xA6U, // 0xA6
    XC24_ADDR_PORT8_LOCAL_RW_DATA7  = 0xA7U, // 0xA7

    XC24_ADDR_PORT1_LOCAL_RW_DATA8  = 0xA8U, // 0xA8
    XC24_ADDR_PORT2_LOCAL_RW_DATA8  = 0xA9U, // 0xA9
    XC24_ADDR_PORT3_LOCAL_RW_DATA8  = 0xAAU, // 0xAA
    XC24_ADDR_PORT4_LOCAL_RW_DATA8  = 0xABU, // 0xAB
    XC24_ADDR_PORT5_LOCAL_RW_DATA8  = 0xACU, // 0xAC
    XC24_ADDR_PORT6_LOCAL_RW_DATA8  = 0xADU, // 0xAD
    XC24_ADDR_PORT7_LOCAL_RW_DATA8  = 0xAEU, // 0xAE
    XC24_ADDR_PORT8_LOCAL_RW_DATA8  = 0xAFU, // 0xAF

    XC24_ADDR_PORT1_LOCAL_RW_DATA9  = 0xB0U, // 0xB0
    XC24_ADDR_PORT2_LOCAL_RW_DATA9  = 0xB1U, // 0xB1
    XC24_ADDR_PORT3_LOCAL_RW_DATA9  = 0xB2U, // 0xB2
    XC24_ADDR_PORT4_LOCAL_RW_DATA9  = 0xB3U, // 0xB3
    XC24_ADDR_PORT5_LOCAL_RW_DATA9  = 0xB4U, // 0xB4
    XC24_ADDR_PORT6_LOCAL_RW_DATA9  = 0xB5U, // 0xB5
    XC24_ADDR_PORT7_LOCAL_RW_DATA9  = 0xB6U, // 0xB6
    XC24_ADDR_PORT8_LOCAL_RW_DATA9  = 0xB7U, // 0xB7

    XC24_ADDR_PORT1_LOCAL_RW_DATA10 = 0xB8U, // 0xB8
    XC24_ADDR_PORT2_LOCAL_RW_DATA10 = 0xB9U, // 0xB9
    XC24_ADDR_PORT3_LOCAL_RW_DATA10 = 0xBAU, // 0xBA
    XC24_ADDR_PORT4_LOCAL_RW_DATA10 = 0xBBU, // 0xBB
    XC24_ADDR_PORT5_LOCAL_RW_DATA10 = 0xBCU, // 0xBC
    XC24_ADDR_PORT6_LOCAL_RW_DATA10 = 0xBDU, // 0xBD
    XC24_ADDR_PORT7_LOCAL_RW_DATA10 = 0xBEU, // 0xBE
    XC24_ADDR_PORT8_LOCAL_RW_DATA10 = 0xBFU, // 0xBF

    XC24_ADDR_PORT1_LOCAL_RW_DATA11 = 0xC0U, // 0xC0
    XC24_ADDR_PORT2_LOCAL_RW_DATA11 = 0xC1U, // 0xC1
    XC24_ADDR_PORT3_LOCAL_RW_DATA11 = 0xC2U, // 0xC2
    XC24_ADDR_PORT4_LOCAL_RW_DATA11 = 0xC3U, // 0xC3
    XC24_ADDR_PORT5_LOCAL_RW_DATA11 = 0xC4U, // 0xC4
    XC24_ADDR_PORT6_LOCAL_RW_DATA11 = 0xC5U, // 0xC5
    XC24_ADDR_PORT7_LOCAL_RW_DATA11 = 0xC6U, // 0xC6
    XC24_ADDR_PORT8_LOCAL_RW_DATA11 = 0xC7U, // 0xC7

    XC24_ADDR_PORT1_LOCAL_RW_DATA12 = 0xC8U, // 0xC8
    XC24_ADDR_PORT2_LOCAL_RW_DATA12 = 0xC9U, // 0xC9
    XC24_ADDR_PORT3_LOCAL_RW_DATA12 = 0xCAU, // 0xCA
    XC24_ADDR_PORT4_LOCAL_RW_DATA12 = 0xCBU, // 0xCB
    XC24_ADDR_PORT5_LOCAL_RW_DATA12 = 0xCCU, // 0xCC
    XC24_ADDR_PORT6_LOCAL_RW_DATA12 = 0xCDU, // 0xCD
    XC24_ADDR_PORT7_LOCAL_RW_DATA12 = 0xCEU, // 0xCE
    XC24_ADDR_PORT8_LOCAL_RW_DATA12 = 0xCFU, // 0xCF

    XC24_ADDR_PORT1_LOCAL_RW_DATA13 = 0xD0U, // 0xD0
    XC24_ADDR_PORT2_LOCAL_RW_DATA13 = 0xD1U, // 0xD1
    XC24_ADDR_PORT3_LOCAL_RW_DATA13 = 0xD2U, // 0xD2
    XC24_ADDR_PORT4_LOCAL_RW_DATA13 = 0xD3U, // 0xD3
    XC24_ADDR_PORT5_LOCAL_RW_DATA13 = 0xD4U, // 0xD4
    XC24_ADDR_PORT6_LOCAL_RW_DATA13 = 0xD5U, // 0xD5
    XC24_ADDR_PORT7_LOCAL_RW_DATA13 = 0xD6U, // 0xD6
    XC24_ADDR_PORT8_LOCAL_RW_DATA13 = 0xD7U, // 0xD7

    XC24_ADDR_PORT1_LOCAL_RW_DATA14 = 0xD8U, // 0xD8
    XC24_ADDR_PORT2_LOCAL_RW_DATA14 = 0xD9U, // 0xD9
    XC24_ADDR_PORT3_LOCAL_RW_DATA14 = 0xDAU, // 0xDA
    XC24_ADDR_PORT4_LOCAL_RW_DATA14 = 0xDBU, // 0xDB
    XC24_ADDR_PORT5_LOCAL_RW_DATA14 = 0xDCU, // 0xDC
    XC24_ADDR_PORT6_LOCAL_RW_DATA14 = 0xDDU, // 0xDD
    XC24_ADDR_PORT7_LOCAL_RW_DATA14 = 0xDEU, // 0xDE
    XC24_ADDR_PORT8_LOCAL_RW_DATA14 = 0xDFU, // 0xDF

    XC24_ADDR_PORT1_LOCAL_RW_DATA15 = 0xE0U, // 0xE0
    XC24_ADDR_PORT2_LOCAL_RW_DATA15 = 0xE1U, // 0xE1
    XC24_ADDR_PORT3_LOCAL_RW_DATA15 = 0xE2U, // 0xE2
    XC24_ADDR_PORT4_LOCAL_RW_DATA15 = 0xE3U, // 0xE3
    XC24_ADDR_PORT5_LOCAL_RW_DATA15 = 0xE4U, // 0xE4
    XC24_ADDR_PORT6_LOCAL_RW_DATA15 = 0xE5U, // 0xE5
    XC24_ADDR_PORT7_LOCAL_RW_DATA15 = 0xE6U, // 0xE6
    XC24_ADDR_PORT8_LOCAL_RW_DATA15 = 0xE7U, // 0xE7

    XC24_ADDR_PORT1_LOCAL_RW_DATA16 = 0xE8U, // 0xE8
    XC24_ADDR_PORT2_LOCAL_RW_DATA16 = 0xE9U, // 0xE9
    XC24_ADDR_PORT3_LOCAL_RW_DATA16 = 0xEAU, // 0xEA
    XC24_ADDR_PORT4_LOCAL_RW_DATA16 = 0xEBU, // 0xEB
    XC24_ADDR_PORT5_LOCAL_RW_DATA16 = 0xECU, // 0xEC
    XC24_ADDR_PORT6_LOCAL_RW_DATA16 = 0xEDU, // 0xED
    XC24_ADDR_PORT7_LOCAL_RW_DATA16 = 0xEEU, // 0xEE
    XC24_ADDR_PORT8_LOCAL_RW_DATA16 = 0xEFU, // 0xEF
}xc24_addr_port_local_t;

/* SOFT RESET : default 0x00 */
typedef union tag_DUMMY
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy : 16;   /*  */
    };
}_v_dummy_t;


/* SOFT RESET : default 0x00 */
typedef union tag_SOFT_RESET
{
    uint16_t ALL;
    struct
    {
        uint16_t rst1    :  1;
        uint16_t rst2    :  1;
        uint16_t rst3    :  1;
        uint16_t         :  1;
        uint16_t vs_rst1 :  1;
        uint16_t vs_rst2 :  1;
        uint16_t         : 10;
    };
}_v_soft_reset_t;

/* GLOBAL-WRITE COMMAND : default 0x00 */
typedef union tag_GLOBAL_WRITE
{
    uint16_t ALL;
    struct
    {
        uint16_t addr  : 6;
        uint16_t       : 9;
        uint16_t start : 1;
    };
}_v_global_write_t;

/* LOCAL-WRITE COMMAND : default 0x00 */
typedef union tag_LOCAL_WRITE
{
    uint16_t ALL;
    struct
    {
        uint16_t addr   : 6;
        uint16_t        : 2;
        uint16_t ch_seg : 2;
        uint16_t        : 5;
        uint16_t start  : 1;
    };
}_v_local_write_t;

/* LOCAL-READ COMMAND : default 0x00 */
typedef union tag_LOCAL_READ
{
    uint16_t ALL;
    struct
    {
        uint16_t addr   : 6;
        uint16_t        : 2;
        uint16_t ch_seg : 2;
        uint16_t        : 5;
        uint16_t start  : 1;
    };
}_v_local_read_t;

/* ID-GEN COMMAND : default 0x00 */
typedef union tag_ID_GEN
{
    uint16_t ALL;
    struct
    {
        uint16_t       : 15;
        uint16_t start :  1;
    };
}_v_id_gen_t;

/* FAULT-READ COMMAND : default 0x00 */
typedef union tag_FAULT_READ
{
    uint16_t ALL;
    struct
    {
        uint16_t       : 15;
        uint16_t start :  1;
    };
}_v_fault_read_t;

/* LD-TRANSFER COMMAND : default 0x00 */
typedef union tag_LD_TRANSFER
{
    uint16_t ALL;
    struct
    {
        uint16_t       : 15;
        uint16_t start :  1;
    };
}_v_ld_transfer_t;

/* SYNC-GEN COMMAND : default 0x00 */
typedef union tag_SYNC_GEN
{
    uint16_t ALL;
    struct
    {
        uint16_t       : 15;
        uint16_t start :  1;
    };
}_v_sync_gen_t;

/* AUTO-ENABLE COMMAND : default 0x00 */
typedef union tag_CMD_AUTO_ENABLE
{
    uint16_t ALL;
    struct
    {
        uint16_t sync_auto_en  : 1;
        uint16_t               : 3;
        uint16_t fault_auto_en : 1;
        uint16_t               : 3;
        uint16_t timeout_en    : 1;
        uint16_t               : 7;
    };
}_v_cmd_auto_enable_t;

/* LD-WRITE POINTER COMMAND : Read-only */
typedef union tag_LD_WRITE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_wr_pointer : 9;
        uint16_t               : 7;
    };
}_v_ld_write_pointer_t;

/* LD-READ POINTER COMMAND : Read-only */
typedef union tag_LD_READ_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_rd_pointer : 9;
        uint16_t               : 7;
    };
}_v_ld_read_pointer_t;

/* LD-DIFFERENCE POINTER COMMAND : Read-only */
typedef union tag_LD_DIFFERENCE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_diff_pointer :  6;
        uint16_t                 : 10;
    };
}_v_ld_difference_pointer_t;

/* LD-TRANSFER START POINTER / THRESHOLD : default 0x00 */
typedef union tag_LD_TRANSFER_START_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_diff_threshold      : 6;
        uint16_t                        : 1;
        uint16_t int_ld_sign            : 1;
        uint16_t ld_trans_start_pointer : 6;
        uint16_t                        : 2;
    };
}_v_ld_transfer_start_pointer_t;

/* LOCAL-WR TRANSFER POINTER : Read-only */
typedef union tag_LOCAL_WR_TRANSFER_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_trans_pointer : 6;
        uint16_t                        : 2;
        uint16_t local_wr_out_pointer   : 6;
        uint16_t                        : 2;
    };
}_v_local_wr_transfer_pointer_t;

/* LOCAL-RD RECEIVE POINTER : Read-only */
typedef union tag_LOCAL_RD_RECEIVE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_rd_rec_pointer : 6;
        uint16_t                      : 2;
        uint16_t local_rd_out_pointer : 6;
        uint16_t                      : 2;
    };
}_v_local_rd_receive_pointer_t;

/* LOCAL-RW DIFFERENCE POINTER : Read-only */
typedef union tag_LOCAL_RW_DIFFERENCE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_diff_pointer : 6;
        uint16_t                       : 2;
        uint16_t local_rd_diff_pointer : 6;
        uint16_t                       : 2;
    };
}_v_local_rw_difference_pointer_t;

/* LOCAL-RW POINTER RESET : default 0x00 */
typedef union tag_LOCAL_RW_POINTER_RESET
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_pointer_rst : 1;
        uint16_t                      : 7;
        uint16_t local_rd_pointer_rst : 1;
        uint16_t                      : 7;
    };
}_v_local_rw_pointer_reset_t;

/* FAULT-AUTO-READ TIMER : Read-only */
typedef union tag_FAULT_AUTO_READ_TIMER
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_auto_rd_timer : 16;
    };
}_v_fault_auto_read_timer_t;

/* FAULT-AUTO-READ EVENT : default 0x00 */
typedef union tag_FAULT_AUTO_READ_EVENT
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_auto_rd_interval : 1;
        uint16_t                        : 7;
        uint16_t fault_auto_rd_event    : 8;
    };
}_v_fault_auto_read_event_t;

/* SERIALIZER CLOCK GEN : default 0x0408 */
typedef union tag_SERIALIZER_CLOCK_GEN
{
    uint16_t ALL;
    struct
    {
        uint16_t sck_high : 7;
        uint16_t          : 1;
        uint16_t sck_low  : 7;
        uint16_t          : 1;
    };
}_v_serializer_clock_gen_t;

/* INTERRUPT ENABLE : default 0x00 */
typedef union tag_INTERRUPT_ENABLE
{
    uint16_t ALL;
    struct
    {
        uint16_t int_fb_en                 : 1;
        uint16_t int_open_en               : 1;
        uint16_t int_short_en              : 1;
        uint16_t int_thermal_en            : 1;
        uint16_t int_ld_en                 : 1;
        uint16_t int_rd_rec_fail_en        : 1;
        uint16_t int_fault_auto_rd_fail_en : 1;
        uint16_t int_fault_rd_fail_en      : 1;
        uint16_t int_timeout_err_en        : 1;
        uint16_t int_parity_err_en		   : 1;
        uint16_t int_acc_cnt_err_en		   : 1;
        uint16_t                           : 1;
        uint16_t int_uv15_en			   : 1;
        uint16_t int_ov15_en			   : 1;
        uint16_t int_uv50_en			   : 1;
        uint16_t int_ov50_en			   : 1;
    };
}_v_interrupt_enable_t;

/* COMMAND STATUS 1 : Read-only */
typedef union tag_COMMAND_STATUS1
{
    uint16_t ALL;
    struct
    {
        uint16_t sync_doing       : 1;
        uint16_t sync_done        : 1;
        uint16_t sync_auto_doing  : 1;
        uint16_t sync_auto_done   : 1;
        uint16_t fault_auto_doing : 1;
        uint16_t fault_auto_done  : 1;
        uint16_t fault_doing      : 1;
        uint16_t fault_done       : 1;
        uint16_t ld_trans_doing   : 1;
        uint16_t ld_trans_done    : 1;
        uint16_t global_wr_doing  : 1;
        uint16_t global_wr_done   : 1;
        uint16_t local_wr_doing   : 1;
        uint16_t local_wr_done    : 1;
        uint16_t local_rd_doing   : 1;
        uint16_t local_rd_done    : 1;
    };
}_v_command_status1_t;

/* COMMAND STATUS 2 : Read-only */
typedef union tag_COMMAND_STATUS2
{
    uint16_t ALL;
    struct
    {
        uint16_t id_gen_doing :  1;
        uint16_t id_gen_done  :  1;
        uint16_t              : 14;
    };
}_v_command_status2_t;

/* RECEIVE STATUS : Read-only */
typedef union tag_RECEIVE_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t local_rec_doing      : 1;
        uint16_t local_rec_done       : 1;
        uint16_t fault_auto_rec_doing : 1;
        uint16_t fault_auto_rec_done  : 1;
        uint16_t fault_rec_doing      : 1;
        uint16_t fault_rec_done       : 1;
        uint16_t                      : 2;
        uint16_t timeout_err          : 1;
        uint16_t                      : 3;
        uint16_t local_rec_fail       : 1;
        uint16_t fault_auto_rec_fail  : 1;
        uint16_t fault_rec_fail       : 1;
        uint16_t                      : 1;
    };
}_v_receive_status_t;

/* INTERRUPT STATUS : Read-only */
typedef union tag_INTERRUPT_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t int_fault                   : 1;
        uint16_t int_ld                      : 1;
        uint16_t                             : 2;
        uint16_t int_fb_src                  : 1;
        uint16_t int_open_src                : 1;
        uint16_t int_short_src               : 1;
        uint16_t int_tml_src                 : 1;
        uint16_t int_ld_trans_src            : 1;
        uint16_t int_rd_rec_fail_src         : 1;
        uint16_t int_fault_auto_rec_fail_src : 1;
        uint16_t int_fault_rec_fail_src      : 1;
        uint16_t int_timeout_err_src         : 1;
        uint16_t int_parity_err_src          : 1;
        uint16_t int_acc_cnt_err_src         : 1;
        uint16_t int_ovuv_src                : 1;
    };
}_v_interrupt_status_t;

typedef union tag_SPI_FAULT_STATUS_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t parity_err_det_en  : 1;
        uint16_t acc_cnt_err_det_en : 1;
        uint16_t parity_err_mode    : 1;
        uint16_t                    : 1;
        uint16_t parity_err         : 1;
        uint16_t acc_cnt_err        : 1;
        uint16_t                    : 2;
        uint16_t acc_counter        : 8;
    };
}_v_spi_fault_status_control_t;

typedef union tag_CLK_CONTROL_1
{
    uint16_t ALL;
    struct
    {
        uint16_t mclk_mode              : 1;
        uint16_t serializer_skew_en     : 1;
        uint16_t osc_spread_en          : 1;
        uint16_t serializer_clk_sel     : 1;
        uint16_t spread_dir		        : 2;
        uint16_t spread_speed	        : 2;
        uint16_t spread_range	        : 2;
        uint16_t serialize_clk_status   : 1;
        uint16_t ld_b_rd_clk_sel	    : 1;
        uint16_t spread_range_a         : 3;
        uint16_t                        : 1;
    };
}_v_clk_control_1_t;

typedef union tag_CLK_CONTROL_2
{
    uint16_t ALL;
    struct
    {
        uint16_t                  : 8;
        uint16_t osc_force_static : 7;
        uint16_t                  : 1;
    };
}_v_clk_control_2_t;

typedef union tag_VDD_LDO_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t uv15_det : 1;
        uint16_t          : 3;
        uint16_t ov15_det : 1;
        uint16_t          : 3;
        uint16_t uv50_det : 1;
        uint16_t          : 3;
        uint16_t ov50_det : 1;
        uint16_t          : 3;
    };
}_v_vdd_ldo_status_t;


typedef union tag_VDD_LDO_FAULT_LEVEL
{
    uint16_t ALL;
    struct
    {
        uint16_t uv15_level : 3;
        uint16_t            : 1;
        uint16_t ov15_level : 3;
        uint16_t            : 1;
        uint16_t uv50_level : 3;
        uint16_t            : 1;
        uint16_t ov50_level : 3;
        uint16_t            : 1;
    };
}_v_vdd_ldo_fault_level_t;

typedef union tag_COMMAND_LATENCY
{
    uint16_t ALL;
    struct
    {
        uint16_t cmd_latency    : 8;
        uint16_t serial_latency : 8;
    };
}_v_command_latency_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE1
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_1 : 5;
        uint16_t daisied_dev_ch_size_2 : 5;
        uint16_t daisied_dev_ch_size_3 : 5;
        uint16_t                       : 1;
    };
}_v_daisied_device_channel_size1_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE2
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_4 : 5;
        uint16_t daisied_dev_ch_size_5 : 5;
        uint16_t daisied_dev_ch_size_6 : 5;
        uint16_t                       : 1;
    };
}_v_daisied_device_channel_size2_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE3
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_7 : 5;
        uint16_t daisied_dev_ch_size_8 : 5;
        uint16_t daisied_dev_ch_size_9 : 5;
        uint16_t                       : 1;
    };
}_v_daisied_device_channel_size3_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE4
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_10 : 5;
        uint16_t daisied_dev_ch_size_11 : 5;
        uint16_t daisied_dev_ch_size_12 : 5;
        uint16_t                        : 1;
    };
}_v_daisied_device_channel_size4_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE5
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_13 : 5;
        uint16_t daisied_dev_ch_size_14 : 5;
        uint16_t daisied_dev_ch_size_15 : 5;
        uint16_t                        : 1;
    };
}_v_daisied_device_channel_size5_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE6
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_16 : 5;
        uint16_t daisied_dev_ch_size_17 : 5;
        uint16_t daisied_dev_ch_size_18 : 5;
        uint16_t                        : 1;
    };
}_v_daisied_device_channel_size6_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE7
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_19 : 5;
        uint16_t daisied_dev_ch_size_20 : 5;
        uint16_t daisied_dev_ch_size_21 : 5;
        uint16_t                        : 1;
    };
}_v_daisied_device_channel_size7_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE8
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_22 : 5;
        uint16_t daisied_dev_ch_size_23 : 5;
        uint16_t daisied_dev_ch_size_24 : 5;
        uint16_t                        : 1;
    };
}_v_daisied_device_channel_size8_t;

/* DAISY SIZE 1 : default 0x00 */
typedef union tag_DAISY_SIZE1
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch1 : 5;
        uint16_t daisy_size_ch2 : 5;
        uint16_t daisy_size_ch3 : 5;
        uint16_t                : 1;
    };
}_v_daisy_size1_t;

/* DAISY SIZE 2 : default 0x00 */
typedef union tag_DAISY_SIZE2
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch4 : 5;
        uint16_t daisy_size_ch5 : 5;
        uint16_t daisy_size_ch6 : 5;
        uint16_t                : 1;
    };
}_v_daisy_size2_t;

/* DAISY SIZE 3 : default 0x00 */
typedef union tag_DAISY_SIZE3
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch7 : 5;
        uint16_t daisy_size_ch8 : 5;
        uint16_t daisy_size_ch9 : 5;
        uint16_t                : 1;
    };
}_v_daisy_size3_t;

/* DAISY SIZE 4 : default 0x00 */
typedef union tag_DAISY_SIZE4
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch10 : 5;
        uint16_t daisy_size_ch11 : 5;
        uint16_t daisy_size_ch12 : 5;
        uint16_t                 : 1;
    };
}_v_daisy_size4_t;

/* DAISY SIZE 5 : default 0x00 */
typedef union tag_DAISY_SIZE5
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch13 : 5;
        uint16_t daisy_size_ch14 : 5;
        uint16_t daisy_size_ch15 : 5;
        uint16_t                 : 1;
    };
}_v_daisy_size5_t;

/* DAISY SIZE 6 : default 0x00 */
typedef union tag_DAISY_SIZE6
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch16 : 5;
        uint16_t daisy_size_ch17 : 5;
        uint16_t daisy_size_ch18 : 5;
        uint16_t                 : 1;
    };
}_v_daisy_size6_t;

/* DAISY SIZE 7 : default 0x00 */
typedef union tag_DAISY_SIZE7
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch19 : 5;
        uint16_t daisy_size_ch20 : 5;
        uint16_t daisy_size_ch21 : 5;
        uint16_t                 : 1;
    };
}_v_daisy_size7_t;

/* DAISY SIZE 8 : default 0x00 */
typedef union tag_DAISY_SIZE8
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch22 : 5;
        uint16_t daisy_size_ch23 : 5;
        uint16_t daisy_size_ch24 : 5;
        uint16_t                 : 1;
    };
}_v_daisy_size8_t;

/* BLOCK SIZE 1 : default 0x00 */
typedef union tag_BLOCK_SIZE1
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch1 : 8;
        uint16_t block_size_ch2 : 8;
    };
}_v_block_size1_t;

/* BLOCK SIZE 2 : default 0x00 */
typedef union tag_BLOCK_SIZE2
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch3 : 8;
        uint16_t block_size_ch4 : 8;
    };
}_v_block_size2_t;

/* BLOCK SIZE 3 : default 0x00 */
typedef union tag_BLOCK_SIZE3
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch5 : 8;
        uint16_t block_size_ch6 : 8;
    };
}_v_block_size3_t;

/* BLOCK SIZE 4 : default 0x00 */
typedef union tag_BLOCK_SIZE4
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch7 : 8;
        uint16_t block_size_ch8 : 8;
    };
}_v_block_size4_t;

/* BLOCK SIZE 5 : default 0x00 */
typedef union tag_BLOCK_SIZE5
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch9  : 8;
        uint16_t block_size_ch10 : 8;
    };
}_v_block_size5_t;

/* BLOCK SIZE 6 : default 0x00 */
typedef union tag_BLOCK_SIZE6
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch11 : 8;
        uint16_t block_size_ch12 : 8;
    };
}_v_block_size6_t;

/* BLOCK SIZE 7 : default 0x00 */
typedef union tag_BLOCK_SIZE7
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch13 : 8;
        uint16_t block_size_ch14 : 8;
    };
}_v_block_size7_t;

/* BLOCK SIZE 8 : default 0x00 */
typedef union tag_BLOCK_SIZE8
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch15 : 8;
        uint16_t block_size_ch16 : 8;
    };
}_v_block_size8_t;

/* BLOCK SIZE 9 : default 0x00 */
typedef union tag_BLOCK_SIZE9
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch17 : 8;
        uint16_t block_size_ch18 : 8;
    };
}_v_block_size9_t;

/* BLOCK SIZE 10 : default 0x00 */
typedef union tag_BLOCK_SIZE10
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch19 : 8;
        uint16_t block_size_ch20 : 8;
    };
}_v_block_size10_t;

/* BLOCK SIZE 11 : default 0x00 */
typedef union tag_BLOCK_SIZE11
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch21 : 8;
        uint16_t block_size_ch22 : 8;
    };
}_v_block_size11_t;

/* BLOCK SIZE 12 : default 0x00 */
typedef union tag_BLOCK_SIZE12
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch23 : 8;
        uint16_t block_size_ch24 : 8;
    };
}_v_block_size12_t;

/* CHANNEL ENABLE 1 : default 0x00 */
typedef union tag_CHANNEL_ENABLE1
{
    uint16_t ALL;
    struct
    {
        uint16_t ch1_en  : 1;
        uint16_t ch2_en  : 1;
        uint16_t ch3_en  : 1;
        uint16_t ch4_en  : 1;
        uint16_t ch5_en  : 1;
        uint16_t ch6_en  : 1;
        uint16_t ch7_en  : 1;
        uint16_t ch8_en  : 1;
        uint16_t ch9_en  : 1;
        uint16_t ch10_en : 1;
        uint16_t ch11_en : 1;
        uint16_t ch12_en : 1;
        uint16_t ch13_en : 1;
        uint16_t ch14_en : 1;
        uint16_t ch15_en : 1;
        uint16_t ch16_en : 1;
    };
}_v_channel_enable1_t;

/* CHANNEL ENABLE 2/ SIZE/ LD WIDTH : default 0x00 */
typedef union tag_CHANNEL_ENABLE2
{
    uint16_t ALL;
    struct
    {
        uint16_t ch17_en  : 1;
        uint16_t ch18_en  : 1;
        uint16_t ch19_en  : 1;
        uint16_t ch20_en  : 1;
        uint16_t ch21_en  : 1;
        uint16_t ch22_en  : 1;
        uint16_t ch23_en  : 1;
        uint16_t ch24_en  : 1;
        uint16_t ld_size  : 5;
        uint16_t          : 1;
        uint16_t ld_width : 2;
    };
}_v_channel_enable2_t;

typedef union tag_DAC_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_auto        : 1;
        uint16_t                 : 3;
        uint16_t dac_sync_mode   : 1;
        uint16_t dac_fb_mode     : 1;
        uint16_t                 : 2;
        uint16_t dac_dec1_mode   : 1;
        uint16_t dac_hold_en     : 1;
        uint16_t                 : 2;
        uint16_t dac_thermal_off : 1;
        uint16_t                 : 3;
    };
}_v_dac_control_t;

typedef union tag_CURRENT_TARGET_DAC
{
    uint16_t ALL;
    struct
    {
        uint16_t curr_tgt_dac : 12;
        uint16_t              :  4;
    };
}_v_current_target_dac_t;

typedef union tag_PREVIOUS_TARGET_DAC
{
    uint16_t ALL;
    struct
    {
        uint16_t prev_tgt_dac : 12;
        uint16_t              :  4;
    };
}_v_previous_target_dac_t;

typedef union tag_DAC_OUT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_out : 12;
        uint16_t         :  4;
    };
}_v_dac_out_t;

typedef union tag_DAC_INCREMENT1
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc1 : 5;
        uint16_t dac_inc2 : 5;
        uint16_t dac_inc3 : 6;
    };
}_v_dac_increment1_t;

typedef union tag_DAC_INCREMENT2_HOLD_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc4           : 7;
        uint16_t dac_inc_hold_limit : 9;
    };
}_v_dac_increment2_hold_limit_t;

typedef union tag_DAC_DECREMENT_INC_WAIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc1_wait : 7;
        uint16_t               : 1;
        uint16_t dac_dec1      : 7;
        uint16_t               : 1;
    };
}_v_dac_decrement_inc_wait_t;

typedef union tag_DAC_INCREMENT_HOLD_TH
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc_hold_threshold       : 9;
        uint16_t dac_increment_hold_th_signed : 1;
        uint16_t                              : 6;
    };
}_v_dac_increment_hold_th_t;

typedef union tag_SOA_N11_N1
{
    uint16_t ALL;
    struct
    {
        uint16_t soa_n1  : 7;
        uint16_t soa_n11 : 9;
    };
}_v_soa_n11_n1_t;

typedef union tag_SOA_P2_P1
{
    uint16_t ALL;
    struct
    {
        uint16_t soa_p1 : 9;
        uint16_t        : 3;
        uint16_t soa_p2 : 4;
    };
}_v_soa_p2_p1_t;

typedef union tag_SOA_P3_P2
{
    uint16_t ALL;
    struct
    {
        uint16_t soa_p2 :  6;
        uint16_t soa_p3 : 10;
    };
}_v_soa_p3_p2_t;

typedef union tag_DAC_FB_VALID_TIMER
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_fb_valid_timer : 16;
    };
}_v_dac_fb_valid_timer_t;

typedef union tag_DAC_MIN_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_min_limit : 12;
        uint16_t               :  4;
    };
}_v_dac_min_limit_t;

typedef union tag_DAC_MAX_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_max_limit : 12;
        uint16_t               :  4;
    };
}_v_dac_max_limit_t;

typedef union tag_DAC_STATE
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_st_en : 4;
        uint16_t dac_state : 4;
        uint16_t           : 8;
    };
}_v_dac_state_t;

typedef union tag_VALID_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_fb_valid_cnt : 16;
    };
}_v_valid_cnt_t;

typedef union tag_DAC_INC_HOLD_WAIT_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc1_wait_cnt      : 7;
        uint16_t                        : 1;
        uint16_t dac_inc_hold_limit_cnt : 8;
    };
}_v_dac_inc_hold_wait_cnt_t;

typedef union tag_R2
{
    uint16_t ALL;
    struct
    {
        uint16_t r2        : 12;
        uint16_t r2_signed :  1;
        uint16_t           :  3;
    };
}_v_r2_t;

/* GLOBAL-WRITE DATA : default 0x00 */
typedef union tag_GLOBAL_WRITE_DATA
{
    uint16_t ALL;
    struct
    {
        uint16_t global_wr_data : 12;
        uint16_t                :  4;
    };
}_v_global_write_v_t;

/* GLOBAL FAULT-READ DATA 1 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA1
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch1 : 4;
        uint16_t fault_ch2 : 4;
        uint16_t fault_ch3 : 4;
        uint16_t fault_ch4 : 4;
    };
}_v_global_fault_read_data1_t;

/* GLOBAL FAULT-READ DATA 2 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA2
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch5 : 4;
        uint16_t fault_ch6 : 4;
        uint16_t fault_ch7 : 4;
        uint16_t fault_ch8 : 4;
    };
}_v_global_fault_read_data2_t;

/* GLOBAL FAULT-READ DATA 3 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA3
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch9  : 4;
        uint16_t fault_ch10 : 4;
        uint16_t fault_ch11 : 4;
        uint16_t fault_ch12 : 4;
    };
}_v_global_fault_read_data3_t;

/* GLOBAL FAULT-READ DATA 4 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA4
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch13 : 4;
        uint16_t fault_ch14 : 4;
        uint16_t fault_ch15 : 4;
        uint16_t fault_ch16 : 4;
    };
}_v_global_fault_read_data4_t;

/* GLOBAL FAULT-READ DATA 5 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA5
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch17 : 4;
        uint16_t fault_ch18 : 4;
        uint16_t fault_ch19 : 4;
        uint16_t fault_ch20 : 4;
    };
}_v_global_fault_read_data5_t;

/* GLOBAL FAULT-READ DATA 6 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA6
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch21 : 4;
        uint16_t fault_ch22 : 4;
        uint16_t fault_ch23 : 4;
        uint16_t fault_ch24 : 4;
    };
}_v_global_fault_read_data6_t;

/* PORTx LOCAL-RW DATA y : default 0x00 */
typedef union tag_PORT_LOCAL_RW_DATA
{
    uint16_t ALL;
    struct
    {
        uint16_t port_local_rw_data : 12;
        uint16_t                    :  4;
    };
}_v_port_local_rw_data_t;

typedef union tag_TEST_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t mclk32_o    : 1;
        uint16_t             : 3;
        uint16_t mclk1_o     : 1;
        uint16_t             : 3;
        uint16_t daco_direct : 1;
        uint16_t             : 6;
        uint16_t test_en     : 1;
    };
}_v_test_control_t;

typedef union tag_OTP_PG_ACCESS
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_acc_cycle : 16;
    };
}_v_otp_pg_access_t;

typedef union tag_OTP_WRITE
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_wsel :  4;
        uint16_t          : 12;
    };
}_v_otp_write_t;

typedef union tag_OTP_RD_PROG
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_s :  1;
        uint16_t otp_rd_s :  1;
        uint16_t          : 14;
    };
}_v_otp_rd_prog_t;

typedef union tag_OTP_PROTECT
{
    uint16_t ALL;
    struct
    {
        uint16_t protect : 12;
        uint16_t         :  4;
    };
}_v_otp_protect_t;

typedef union tag_MIRROR1
{
    uint16_t ALL;
    struct
    {
        uint16_t vctl_ldo : 4;
        uint16_t          : 4;
        uint16_t osc_fctl : 7;
        uint16_t          : 1;
    };
}_v_mirror1_t;

typedef union tag_MIRROR2
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_ofs_val    : 7;
        uint16_t dac_ofs_sign   : 1;
        uint16_t dac_gain       : 6;
        uint16_t                : 2;
    };
}_v_mirror2_t;

typedef union tag_MIRROR3
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_checksum :  6;
        uint16_t              : 10;
    };
}_v_mirror3_t;

typedef union _xc24_regs
{
    uint16_t ALL[XC24_ADDR_MAX];
    struct
    {
        _v_soft_reset_t                     _r00;
        _v_global_write_t			        _r01;
        _v_local_write_t		            _r02;
        _v_local_read_t                     _r03;
        _v_id_gen_t                         _r04;
        _v_fault_read_t                     _r05;
        _v_ld_transfer_t                    _r06;
        _v_sync_gen_t                       _r07;
        _v_cmd_auto_enable_t                _r08;
        _v_dummy_t                          _r09;
        _v_ld_write_pointer_t				_r0A;
        _v_ld_read_pointer_t                _r0B;
        _v_ld_difference_pointer_t          _r0C;
        _v_ld_transfer_start_pointer_t      _r0D;
        _v_local_wr_transfer_pointer_t      _r0E;
        _v_local_rd_receive_pointer_t       _r0F;

        _v_local_rw_difference_pointer_t    _r10;
        _v_local_rw_pointer_reset_t         _r11;
        _v_fault_auto_read_timer_t          _r12;
        _v_fault_auto_read_event_t          _r13;
        _v_serializer_clock_gen_t           _r14;
        _v_interrupt_enable_t               _r15;
        _v_command_status1_t                _r16;
        _v_command_status2_t                _r17;
        _v_receive_status_t                 _r18;
        _v_interrupt_status_t               _r19;
        _v_spi_fault_status_control_t       _r1A;
        _v_clk_control_1_t                  _r1B;
        _v_clk_control_2_t                  _r1C;
        _v_vdd_ldo_status_t                 _r1D;
        _v_vdd_ldo_fault_level_t            _r1E;
        _v_command_latency_t                _r1F;

        _v_daisied_device_channel_size1_t 	_r20;
        _v_daisied_device_channel_size2_t  	_r21;
        _v_daisied_device_channel_size3_t 	_r22;
        _v_daisied_device_channel_size4_t 	_r23;
        _v_daisied_device_channel_size5_t 	_r24;
        _v_daisied_device_channel_size6_t 	_r25;
        _v_daisied_device_channel_size7_t 	_r26;
        _v_daisied_device_channel_size8_t 	_r27;
        _v_dummy_t                          _r28;
        _v_dummy_t                          _r29;
        _v_dummy_t						    _r2A;
        _v_dummy_t                          _r2B;
        _v_dummy_t                          _r2C;
        _v_dummy_t                          _r2D;
        _v_dummy_t                          _r2E;
        _v_dummy_t                          _r2F;

        _v_daisy_size1_t 					_r30;
        _v_daisy_size2_t 					_r31;
        _v_daisy_size3_t 					_r32;
        _v_daisy_size4_t 					_r33;
        _v_daisy_size5_t 					_r34;
        _v_daisy_size6_t 					_r35;
        _v_daisy_size7_t 					_r36;
        _v_daisy_size8_t 					_r37;
        _v_block_size1_t 					_r38;
        _v_block_size2_t 					_r39;
        _v_block_size3_t 					_r3A;
        _v_block_size4_t 					_r3B;
        _v_block_size5_t 					_r3C;
        _v_block_size6_t 					_r3D;
        _v_block_size7_t 					_r3E;
        _v_block_size8_t 					_r3F;

        _v_block_size9_t 					_r40;
        _v_block_size10_t 					_r41;
        _v_block_size11_t 					_r42;
        _v_block_size12_t 					_r43;
        _v_dummy_t						    _r44;
        _v_channel_enable1_t 				_r45;
        _v_channel_enable2_t 				_r46;
        _v_dummy_t						    _r47;
        _v_dummy_t						    _r48;
        _v_dummy_t						    _r49;
        _v_dummy_t						    _r4A;
        _v_dummy_t						    _r4B;
        _v_dummy_t						    _r4C;
        _v_dac_control_t 					_r4D;
        _v_dummy_t						    _r4E;
        _v_current_target_dac_t 			_r4F;

        _v_previous_target_dac_t      		_r50;
        _v_dac_out_t 						_r51;
        _v_dac_increment1_t 				_r52;
        _v_dac_increment2_hold_limit_t 		_r53;
        _v_dac_decrement_inc_wait_t 		_r54;
        _v_dac_increment_hold_th_t 			_r55;
        _v_soa_n11_n1_t 					_r56;
        _v_soa_p2_p1_t 						_r57;
        _v_soa_p3_p2_t 						_r58;
        _v_dac_fb_valid_timer_t 			_r59;
        _v_dac_min_limit_t 					_r5A;
        _v_dac_max_limit_t 					_r5B;
        _v_dac_state_t 						_r5C;
        _v_valid_cnt_t 						_r5D;
        _v_dac_inc_hold_wait_cnt_t 			_r5E;
        _v_r2_t 							_r5F;

        _v_global_write_v_t 				_r60;
        _v_global_fault_read_data1_t 		_r61;
        _v_global_fault_read_data2_t 		_r62;
        _v_global_fault_read_data3_t 		_r63;
        _v_global_fault_read_data4_t 		_r64;
        _v_global_fault_read_data5_t 		_r65;
        _v_global_fault_read_data6_t 		_r66;
        _v_dummy_t     						_r67;
        _v_dummy_t     						_r68;
        _v_dummy_t     						_r69;
        _v_dummy_t     						_r6A;
        _v_dummy_t     						_r6B;
        _v_dummy_t     						_r6C;
        _v_dummy_t     						_r6D;
        _v_dummy_t     						_r6E;
        _v_dummy_t     						_r6F;

        _v_port_local_rw_data_t 			_r70;
    };
}_xc24_general_regs_t;

typedef union _xc24_mirror_regs
{
    uint16_t ALL[XC24_MIRROR_ADDR_MAX - XC24_MIRROR_ADDR_START];
    struct
    {
        _v_test_control_t                   _rF0;
        _v_otp_pg_access_t 		   		    _rF1;
        _v_otp_write_t 		   			    _rF2;
        _v_otp_rd_prog_t 		   		    _rF3;
        _v_otp_protect_t 		   		    _rF4;
        _v_mirror1_t 		   			    _rF5;
        _v_mirror2_t 		   			    _rF6;
        _v_mirror3_t 		   			    _rF7;
    };
}_xc24_mirror_regs_t;

/* BEGIN - INTERFACE FUNCTIONS */

extern volatile uint8_t gn_xc_spi_timeout;

extern uint16_t XC24_Read_Register(uint8_t in_addr);

extern void XC24_Update_Channel_Enable_By_XDIC_ID_Check(void);
extern void XC24_Init(void);
extern void XC24_DeInit(void);
extern void XC24_Test_Init(void);

extern void XC24_IF_IdGen_Command(void);
extern void XC24_IF_SyncGen_Command(void);
extern uint16_t XC24_IF_Fault_Read_Command(void);

extern void XC24_IF_Write_XDIC(uint8_t in_XDIC_addr, uint16_t in_XDIC_data);
extern uint16_t* XC24_IF_Read_XDIC(uint8_t in_XDIC_addr);
extern void XC24_IF_Write_LD(uint16_t ld_duty);

extern void XC24_Turn_Off_Sync_Auto(void);
extern void XC24_Turn_On_Sync_Auto(void);

/* END   - INTERFACE FUNCTIONS */
#ifdef __cplusplus
}
#endif

#endif /* ~__XC24_H__ */

/*** end of file ***/


