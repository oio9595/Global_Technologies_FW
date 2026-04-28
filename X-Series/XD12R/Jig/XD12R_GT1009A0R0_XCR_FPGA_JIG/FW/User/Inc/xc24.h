/** @file xc24.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XC24R_H__
#define __XC24R_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CMD_CODE_NONE                       (0x00U)    /* 0b00 : No operation */
#define CMD_CODE_REG_READ                   (0x01U)    /* 0b01 : Register-read */
#define CMD_CODE_REG_WRITE                  (0x02U)    /* 0b10 : Register-write */
#define CMD_CODE_LD_TRANS                   (0x03U)    /* 0b11 : Local dimming data transfer */

#define CH_SEG_1                            (0x00U)    /* CH01 ~ CH08 */
#define CH_SEG_2                            (0x01U)    /* CH09 ~ CH16 */
#define CH_SEG_3                            (0x20U)    /* CH17 ~ CH24 */

typedef enum tag_XC24R_ADDR_GRP1_T
{
    XC24R_ADDR_GRP1_RESET                        = 0x00U, // 0x00
    XC24R_ADDR_GRP1_GLOBAL_WRITE                 = 0x01U, // 0x01
    XC24R_ADDR_GRP1_LOCAL_WRITE                  = 0x02U, // 0x02
    XC24R_ADDR_GRP1_LOCAL_READ                   = 0x03U, // 0x03
    XC24R_ADDR_GRP1_ID_GEN                       = 0x04U, // 0x04
    XC24R_ADDR_GRP1_FAULT_READ                   = 0x05U, // 0x05
    XC24R_ADDR_GRP1_LD_TRANSFER                  = 0x06U, // 0x06
    XC24R_ADDR_GRP1_SYNC_GEN                     = 0x07U, // 0x07
    XC24R_ADDR_GRP1_COMMAND_AUTO_ENABLE          = 0x08U, // 0x08
    XC24R_ADDR_GRP1_LD_WRITE_POINTER             = 0x09U, // 0x09
    XC24R_ADDR_GRP1_LD_READ_POINTER              = 0x0AU, // 0x0A
    XC24R_ADDR_GRP1_LD_DIFFERENCE_POINTER        = 0x0BU, // 0x0B
    XC24R_ADDR_GRP1_LD_START_POINTER_TH          = 0x0CU, // 0x0C
    XC24R_ADDR_GRP1_LOCAL_WRITE_TRANSFER_POINTER = 0x0DU, // 0x0D
    XC24R_ADDR_GRP1_LOCAL_READ_RECEIVE_POINTER   = 0x0EU, // 0x0E
    XC24R_ADDR_GRP1_LOCAL_RW_DIFFERENCE_POINTER  = 0x0FU, // 0x0F

    XC24R_ADDR_GRP1_LOCAL_RW_POINTER_RESET       = 0x10U, // 0x10
    XC24R_ADDR_GRP1_FAULT_AUTO_READ_INTERVAL     = 0x11U, // 0x11
    XC24R_ADDR_GRP1_FAULT_AUTO_READ_EVENT        = 0x12U, // 0x12
    XC24R_ADDR_GRP1_INTERRUPT_ENABLE             = 0x13U, // 0x13
    XC24R_ADDR_GRP1_COMMAND_STATUS_1             = 0x14U, // 0x14
    XC24R_ADDR_GRP1_COMMAND_STATUS_2             = 0x15U, // 0x15
    XC24R_ADDR_GRP1_RECEIVE_STATUS               = 0x16U, // 0x16
    XC24R_ADDR_GRP1_INTERRUPT_STATUS             = 0x17U, // 0x17
    XC24R_ADDR_GRP1_CMD_PARITY_ERR_STATUS1       = 0x18U, // 0x18
    XC24R_ADDR_GRP1_CMD_PARITY_ERR_STATUS2       = 0x19U, // 0x19
    XC24R_ADDR_GRP1_SPI_FAULT_STATUS_CONTROL     = 0x1AU, // 0x1A
    XC24R_ADDR_GRP1_CLK_CONTROL_1                = 0x1BU, // 0x1B
    XC24R_ADDR_GRP1_CLK_CONTROL_2                = 0x1CU, // 0x1C
    XC24R_ADDR_GRP1_SERIALIZER_CLOCK_GEN         = 0x1DU, // 0x1D
    XC24R_ADDR_GRP1_LATENCY                      = 0x1EU, // 0x1E
    XC24R_ADDR_GRP1_TIMEOUT                      = 0x1FU, // 0x1F

    XC24R_ADDR_GRP1_DAISIED_DEVICE_CH_SIZE       = 0x20U, // 0x20
    XC24R_ADDR_GRP1_DAISY_SIZE_1                 = 0x21U, // 0x21
    XC24R_ADDR_GRP1_DAISY_SIZE_2                 = 0x22U, // 0x22
    XC24R_ADDR_GRP1_DAISY_SIZE_3                 = 0x23U, // 0x23
    XC24R_ADDR_GRP1_DAISY_SIZE_4                 = 0x24U, // 0x24
    XC24R_ADDR_GRP1_DAISY_SIZE_5                 = 0x25U, // 0x25
    XC24R_ADDR_GRP1_DAISY_SIZE_6                 = 0x26U, // 0x26
    XC24R_ADDR_GRP1_DAISY_SIZE_7                 = 0x27U, // 0x27
    XC24R_ADDR_GRP1_DAISY_SIZE_8                 = 0x28U, // 0x28
    XC24R_ADDR_GRP1_BLOCK_SIZE_1                 = 0x29U, // 0x29
    XC24R_ADDR_GRP1_BLOCK_SIZE_2                 = 0x2AU, // 0x2A
    XC24R_ADDR_GRP1_BLOCK_SIZE_3                 = 0x2BU, // 0x2B
    XC24R_ADDR_GRP1_BLOCK_SIZE_4                 = 0x2CU, // 0x2C
    XC24R_ADDR_GRP1_BLOCK_SIZE_5                 = 0x2DU, // 0x2D
    XC24R_ADDR_GRP1_BLOCK_SIZE_6                 = 0x2EU, // 0x2E
    XC24R_ADDR_GRP1_BLOCK_SIZE_7                 = 0x2FU, // 0x2F

    XC24R_ADDR_GRP1_BLOCK_SIZE_8                 = 0x30U, // 0x30
    XC24R_ADDR_GRP1_BLOCK_SIZE_9                 = 0x31U, // 0x31
    XC24R_ADDR_GRP1_BLOCK_SIZE_10                = 0x32U, // 0x32
    XC24R_ADDR_GRP1_BLOCK_SIZE_11                = 0x33U, // 0x33
    XC24R_ADDR_GRP1_BLOCK_SIZE_12                = 0x34U, // 0x34
    XC24R_ADDR_GRP1_CHANNEL_ENABLE_1             = 0x35U, // 0x35
    XC24R_ADDR_GRP1_CHANNEL_ENABLE_2             = 0x36U, // 0x36
    XC24R_ADDR_GRP1_FLLCNT11                     = 0x37U, // 0x37
    XC24R_ADDR_GRP1_FLLCNT12                     = 0x38U, // 0x38
    XC24R_ADDR_GRP1_FLLCNT21                     = 0x39U, // 0x39
    XC24R_ADDR_GRP1_FLLCNT22                     = 0x3AU, // 0x3A
    XC24R_ADDR_GRP1_VO_DELAY                     = 0x3BU, // 0x3B
    XC24R_ADDR_GRP1_VO_OFF_ON                    = 0x3CU, // 0x3C
    XC24R_ADDR_GRP1_SVO_ON                       = 0x3DU, // 0x3D
    XC24R_ADDR_GRP1_SVO1_OFF                     = 0x3EU, // 0x3E
    XC24R_ADDR_GRP1_SVO2_OFF                     = 0x3FU, // 0x3F

    XC24R_ADDR_GRP1_SVO3_OFF                     = 0x40U, // 0x40
    XC24R_ADDR_GRP1_SVO_NUMBER                   = 0x41U, // 0x41
    XC24R_ADDR_GRP1_DAC_NF_CONTROL               = 0x42U, // 0x42
    XC24R_ADDR_GRP1_DAC_CONTROL                  = 0x43U, // 0x43
    XC24R_ADDR_GRP1_CURRENT_TARGET_DAC1          = 0x44U, // 0x44
    XC24R_ADDR_GRP1_CURRENT_TARGET_DAC2          = 0x45U, // 0x45
    XC24R_ADDR_GRP1_CURRENT_TARGET_DAC3          = 0x46U, // 0x46
    XC24R_ADDR_GRP1_PREVIOUS_TARGET_DAC1         = 0x47U, // 0x47
    XC24R_ADDR_GRP1_PREVIOUS_TARGET_DAC2         = 0x48U, // 0x48
    XC24R_ADDR_GRP1_PREVIOUS_TARGET_DAC3         = 0x49U, // 0x49
    XC24R_ADDR_GRP1_DAC1_OUT                     = 0x4AU, // 0x4A
    XC24R_ADDR_GRP1_DAC2_OUT                     = 0x4BU, // 0x4B
    XC24R_ADDR_GRP1_DAC3_OUT                     = 0x4CU, // 0x4C
    XC24R_ADDR_GRP1_DAC1_STATE                   = 0x4DU, // 0x4D
    XC24R_ADDR_GRP1_DAC2_STATE                   = 0x4EU, // 0x4E
    XC24R_ADDR_GRP1_DAC3_STATE                   = 0x4FU, // 0x4F

    XC24R_ADDR_GRP1_DAC1_INCREMENT_1             = 0x50U, // 0x50
    XC24R_ADDR_GRP1_DAC1_INCREMENT_2_HOLD_LIMIT  = 0x51U, // 0x51
    XC24R_ADDR_GRP1_DAC1_DECREMENT_INC_WAIT      = 0x52U, // 0x52
    XC24R_ADDR_GRP1_DAC1_INCREMENT_HOLD_THRESHOLD= 0x53U, // 0x53
    XC24R_ADDR_GRP1_DAC1_FB_VALID_TIMER          = 0x54U, // 0x54
    XC24R_ADDR_GRP1_DAC1_MIN_LIMIT               = 0x55U, // 0x55
    XC24R_ADDR_GRP1_DAC1_MAX_LIMIT               = 0x56U, // 0x56
    XC24R_ADDR_GRP1_DAC2_INCREMENT_1             = 0x57U, // 0x57
    XC24R_ADDR_GRP1_DAC2_INCREMENT_2_HOLD_LIMIT  = 0x58U, // 0x58
    XC24R_ADDR_GRP1_DAC2_DECREMENT_INC_WAIT      = 0x59U, // 0x59
    XC24R_ADDR_GRP1_DAC2_INCREMENT_HOLD_THRESHOLD= 0x5AU, // 0x5A
    XC24R_ADDR_GRP1_DAC2_FB_VALID_TIMER          = 0x5BU, // 0x5B
    XC24R_ADDR_GRP1_DAC2_MIN_LIMIT               = 0x5CU, // 0x5C
    XC24R_ADDR_GRP1_DAC2_MAX_LIMIT               = 0x5DU, // 0x5D
    XC24R_ADDR_GRP1_DAC3_INCREMENT_1             = 0x5EU, // 0x5E
    XC24R_ADDR_GRP1_DAC3_INCREMENT_2_HOLD_LIMIT  = 0x5FU, // 0x5F

    XC24R_ADDR_GRP1_DAC3_DECREMENT_INC_WAIT      = 0x60U, // 0x60
    XC24R_ADDR_GRP1_DAC3_INCREMENT_HOLD_THRESHOLD= 0x61U, // 0x61
    XC24R_ADDR_GRP1_DAC3_FB_VALID_TIMER          = 0x62U, // 0x62
    XC24R_ADDR_GRP1_DAC3_MIN_LIMIT               = 0x63U, // 0x63
    XC24R_ADDR_GRP1_DAC3_MAX_LIMIT               = 0x64U, // 0x64
    XC24R_ADDR_GRP1_OSC_FLL_MAN_A1               = 0x65U, // 0x65
    XC24R_ADDR_GRP1_OSC_FLL_MAN_A2               = 0x66U, // 0x66
    XC24R_ADDR_GRP1_OSC_FLL_MAN_B1               = 0x67U, // 0x67
    XC24R_ADDR_GRP1_OSC_FLL_MAN_B2               = 0x68U, // 0x68
    XC24R_ADDR_GRP1_GLOBAL_WRITE_DATA            = 0x69U, // 0x69
    XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA1      = 0x6AU, // 0x6A
    XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA2      = 0x6BU, // 0x6B
    XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA3      = 0x6CU, // 0x6C
    XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA4      = 0x6DU, // 0x6D
    XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA5      = 0x6EU, // 0x6E
    XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA6      = 0x6FU, // 0x6F

    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA1         = 0x70U, // 0x70
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA1         = 0x71U, // 0x71
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA1         = 0x72U, // 0x72
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA1         = 0x73U, // 0x73
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA1         = 0x74U, // 0x74
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA1         = 0x75U, // 0x75
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA1         = 0x76U, // 0x76
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA1         = 0x77U, // 0x77
    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA2         = 0x78U, // 0x78
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA2         = 0x79U, // 0x79
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA2         = 0x7AU, // 0x7A
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA2         = 0x7BU, // 0x7B
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA2         = 0x7CU, // 0x7C
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA2         = 0x7DU, // 0x7D
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA2         = 0x7EU, // 0x7E
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA2         = 0x7FU, // 0x7F

    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA3         = 0x80U, // 0x80
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA3         = 0x81U, // 0x81
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA3         = 0x82U, // 0x82
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA3         = 0x83U, // 0x83
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA3         = 0x84U, // 0x84
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA3         = 0x85U, // 0x85
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA3         = 0x86U, // 0x86
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA3         = 0x87U, // 0x87
    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA4         = 0x88U, // 0x88
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA4         = 0x89U, // 0x89
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA4         = 0x8AU, // 0x8A
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA4         = 0x8BU, // 0x8B
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA4         = 0x8CU, // 0x8C
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA4         = 0x8DU, // 0x8D
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA4         = 0x8EU, // 0x8E
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA4         = 0x8FU, // 0x8F

    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA5         = 0x90U, // 0x90
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA5         = 0x91U, // 0x91
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA5         = 0x92U, // 0x92
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA5         = 0x93U, // 0x93
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA5         = 0x94U, // 0x94
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA5         = 0x95U, // 0x95
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA5         = 0x96U, // 0x96
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA5         = 0x97U, // 0x97
    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA6         = 0x98U, // 0x98
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA6         = 0x99U, // 0x99
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA6         = 0x9AU, // 0x9A
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA6         = 0x9BU, // 0x9B
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA6         = 0x9CU, // 0x9C
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA6         = 0x9DU, // 0x9D
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA6         = 0x9EU, // 0x9E
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA6         = 0x9FU, // 0x9F

    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA7         = 0xA0U, // 0xA0
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA7         = 0xA1U, // 0xA1
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA7         = 0xA2U, // 0xA2
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA7         = 0xA3U, // 0xA3
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA7         = 0xA4U, // 0xA4
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA7         = 0xA5U, // 0xA5
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA7         = 0xA6U, // 0xA6
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA7         = 0xA7U, // 0xA7
    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA8         = 0xA8U, // 0xA8
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA8         = 0xA9U, // 0xA9
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA8         = 0xAAU, // 0xAA
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA8         = 0xABU, // 0xAB
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA8         = 0xACU, // 0xAC
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA8         = 0xADU, // 0xAD
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA8         = 0xAEU, // 0xAE
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA8         = 0xAFU, // 0xAF

    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA9         = 0xB0U, // 0xB0
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA9         = 0xB1U, // 0xB1
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA9         = 0xB2U, // 0xB2
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA9         = 0xB3U, // 0xB3
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA9         = 0xB4U, // 0xB4
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA9         = 0xB5U, // 0xB5
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA9         = 0xB6U, // 0xB6
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA9         = 0xB7U, // 0xB7
    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA10        = 0xB8U, // 0xB8
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA10        = 0xB9U, // 0xB9
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA10        = 0xBAU, // 0xBA
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA10        = 0xBBU, // 0xBB
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA10        = 0xBCU, // 0xBC
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA10        = 0xBDU, // 0xBD
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA10        = 0xBEU, // 0xBE
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA10        = 0xBFU, // 0xBF

    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA11        = 0xC0U, // 0xC0
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA11        = 0xC1U, // 0xC1
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA11        = 0xC2U, // 0xC2
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA11        = 0xC3U, // 0xC3
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA11        = 0xC4U, // 0xC4
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA11        = 0xC5U, // 0xC5
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA11        = 0xC6U, // 0xC6
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA11        = 0xC7U, // 0xC7
    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA12        = 0xC8U, // 0xC8
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA12        = 0xC9U, // 0xC9
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA12        = 0xCAU, // 0xCA
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA12        = 0xCBU, // 0xCB
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA12        = 0xCCU, // 0xCC
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA12        = 0xCDU, // 0xCD
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA12        = 0xCEU, // 0xCE
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA12        = 0xCFU, // 0xCF

    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA13        = 0xD0U, // 0xD0
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA13        = 0xD1U, // 0xD1
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA13        = 0xD2U, // 0xD2
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA13        = 0xD3U, // 0xD3
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA13        = 0xD4U, // 0xD4
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA13        = 0xD5U, // 0xD5
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA13        = 0xD6U, // 0xD6
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA13        = 0xD7U, // 0xD7
    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA14        = 0xD8U, // 0xD8
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA14        = 0xD9U, // 0xD9
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA14        = 0xDAU, // 0xDA
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA14        = 0xDBU, // 0xDB
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA14        = 0xDCU, // 0xDC
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA14        = 0xDDU, // 0xDD
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA14        = 0xDEU, // 0xDE
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA14        = 0xDFU, // 0xDF

    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA15        = 0xE0U, // 0xE0
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA15        = 0xE1U, // 0xE1
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA15        = 0xE2U, // 0xE2
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA15        = 0xE3U, // 0xE3
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA15        = 0xE4U, // 0xE4
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA15        = 0xE5U, // 0xE5
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA15        = 0xE6U, // 0xE6
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA15        = 0xE7U, // 0xE7
    XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA16        = 0xE8U, // 0xE8
    XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA16        = 0xE9U, // 0xE9
    XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA16        = 0xEAU, // 0xEA
    XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA16        = 0xEBU, // 0xEB
    XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA16        = 0xECU, // 0xEC
    XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA16        = 0xEDU, // 0xED
    XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA16        = 0xEEU, // 0xEE
    XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA16        = 0xEFU, // 0xEF

    XC24R_ADDR_GRP1_MIRROR_TEST_CONTROL          = 0xF0U, // 0xF0
    XC24R_ADDR_GRP1_MIRROR_OTP_PG_ACCESS         = 0xF1U, // 0xF1
    XC24R_ADDR_GRP1_MIRROR_OTP_WRITE             = 0xF2U, // 0xF2
    XC24R_ADDR_GRP1_MIRROR_OTP_RD_PROG           = 0xF3U, // 0xF3
    XC24R_ADDR_GRP1_MIRROR_OTP_PROTECT           = 0xF4U, // 0xF4
    XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR1           = 0xF5U, // 0xF5
    XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR2           = 0xF6U, // 0xF6
    XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR3           = 0xF7U, // 0xF7
    XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR4           = 0xF8U, // 0xF8
    XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR5           = 0xF9U, // 0xF9
    XC24R_ADDR_GRP1_MIRROR_GATE_CONTROL          = 0xFAU, // 0xFA
    XC24R_ADDR_GRP1_MIRROR_GATE1_OFFSET          = 0xFBU, // 0xFB
    XC24R_ADDR_GRP1_MIRROR_GATE2_OFFSET          = 0xFCU, // 0xFC
    XC24R_ADDR_GRP1_MIRROR_GATE3_OFFSET          = 0xFDU, // 0xFD
    XC24R_ADDR_GRP1_MIRROR_SV_VAR_CONTROL1       = 0xFEU, // 0xFE
    XC24R_ADDR_GRP1_MIRROR_SV_VAR_CONTROL2       = 0xFFU, // 0xFF
    XC24R_ADDR_GRP1_MAX                     = 0x100U, // 0x100
} xc24r_addr_grp1_t;

typedef enum tag_XC24R_ADDR_GRP2_T
{
    XC24R_ADDR_GRP2_DAC1_FB_VALID_CNT            = 0x00U, // 0x00
    XC24R_ADDR_GRP2_DAC1_INC_HOLD_WAIT_CNT       = 0x01U, // 0x01
    XC24R_ADDR_GRP2_1R2                          = 0x02U, // 0x02
    XC24R_ADDR_GRP2_SOA1_N1_N11                  = 0x03U, // 0x03
    XC24R_ADDR_GRP2_SOA1_P2_P1                   = 0x04U, // 0x04
    XC24R_ADDR_GRP2_SOA1_P3_P2                   = 0x05U, // 0x05
    XC24R_ADDR_GRP2_DAC2_FB_VALID_CNT            = 0x06U, // 0x06
    XC24R_ADDR_GRP2_DAC2_INC_HOLD_WAIT_CNT       = 0x07U, // 0x07
    XC24R_ADDR_GRP2_2R2                          = 0x08U, // 0x08
    XC24R_ADDR_GRP2_SOA2_N1_N11                  = 0x09U, // 0x09
    XC24R_ADDR_GRP2_SOA2_P2_P1                   = 0x0AU, // 0x0A
    XC24R_ADDR_GRP2_SOA2_P3_P2                   = 0x0BU, // 0x0B
    XC24R_ADDR_GRP2_DAC3_FB_VALID_CNT            = 0x0CU, // 0x0C
    XC24R_ADDR_GRP2_DAC3_INC_HOLD_WAIT_CNT       = 0x0DU, // 0x0D
    XC24R_ADDR_GRP2_3R2                          = 0x0EU, // 0x0E
    XC24R_ADDR_GRP2_SOA3_N1_N11                  = 0x0FU, // 0x0F

    XC24R_ADDR_GRP2_SOA3_P2_P1                   = 0x10U, // 0x10
    XC24R_ADDR_GRP2_SOA3_P3_P2                   = 0x11U, // 0x11
    XC24R_ADDR_GRP2_ANA_TEST                     = 0x12U, // 0x12
    XC24R_ADDR_GRP2_MAX                     = 0x13U, // 0x13
} xc24r_addr_grp2_t;

typedef union tag_XC24R_CMD_T
{
    uint16_t ALL;
    struct
    {
        uint16_t size   : 6;
        uint16_t addr   : 8;
        uint16_t code   : 2;
    }bit;
} xc24r_cmd_t;

typedef union tag_XC24R_DUMMY_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy  :16;
    }bit;
} xc24r_dummy_t;

typedef union tag_XC24R_GRP1_SOFT_RESET_0x00_T
{
    uint16_t ALL;
    struct
    {
        uint16_t rst1           : 1;
        uint16_t rst2           : 1;
        uint16_t rst3           : 1;
        uint16_t                : 1;
        uint16_t vsync_rst_en1  : 1;
        uint16_t vsync_rst_en2  : 1;
        uint16_t                :10;
    }bit;
} xc24r_grp1_soft_reset_t;

typedef union tag_XC24R_GRP1_GLOBAL_WRITE_0x01_T
{
    uint16_t ALL;
    struct
    {
        uint16_t addr   : 6;
        uint16_t        : 9;
        uint16_t enable : 1;
    }bit;
} xc24r_grp1_global_write_t;

typedef union tag_XC24R_GRP1_LOCAL_WRITE_0x02_T
{
    uint16_t ALL;
    struct
    {
        uint16_t addr   : 6;
        uint16_t        : 2;
        uint16_t ch_seg : 2;
        uint16_t        : 5;
        uint16_t enable : 1;
    }bit;
} xc24r_grp1_local_write_t;

typedef union tag_XC24R_GRP1_LOCAL_READ_0x03_T
{
    uint16_t ALL;
    struct
    {
        uint16_t addr   : 6;
        uint16_t        : 2;
        uint16_t ch_seg : 2;
        uint16_t        : 5;
        uint16_t enable : 1;
    }bit;
} xc24r_grp1_local_read_t;

typedef union tag_XC24R_GRP1_ID_GEN_0x04_T
{
    uint16_t ALL;
    struct
    {
        uint16_t        :15;
        uint16_t enable : 1;
    }bit;
} xc24r_grp1_id_gen_t;

typedef union tag_XC24R_GRP1_FAULT_READ_0x05_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ft_mode: 2;
        uint16_t        :13;
        uint16_t enable : 1;
    }bit;
} xc24r_grp1_fault_read_t;

typedef union tag_XC24R_GRP1_LD_TRANSFER_0x06_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_type: 3;
        uint16_t        :12;
        uint16_t enable : 1;
    }bit;
} xc24r_grp1_ld_transfer_t;

typedef union tag_XC24R_GRP1_SYNC_GEN_0x07_T
{
    uint16_t ALL;
    struct
    {
        uint16_t syncmode   : 2;
        uint16_t            :13;
        uint16_t enable     : 1;
    }bit;
} xc24r_grp1_sync_gen_t;

typedef union tag_XC24R_GRP1_COMMAND_AUTO_ENABLE_0x08_T
{
    uint16_t ALL;
    struct
    {
        uint16_t sync_auto_en   : 1;
        uint16_t                : 3;
        uint16_t fault_auto_en  : 1;
        uint16_t                :11;
    }bit;
} xc24r_grp1_command_auto_enable_t;

typedef union tag_XC24R_GRP1_LD_WRITE_POINTER_0x09_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_wr_pointer  : 9;
        uint16_t                : 7;
    }bit;
} xc24r_grp1_ld_write_pointer_t;

typedef union tag_XC24R_GRP1_LD_READ_POINTER_0x0A_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_rd_pointer  : 9;
        uint16_t                : 7;
    }bit;
} xc24r_grp1_ld_read_pointer_t;

typedef union tag_XC24R_GRP1_LD_DIFFERENCE_POINTER_0x0B_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_diff_pointer: 6;
        uint16_t                :10;
    }bit;
} xc24r_grp1_ld_difference_pointer_t;

typedef union tag_XC24R_GRP1_LD_TRANSFER_START_POINTER_THRESHOLD_0x0C_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_diff_threshold          : 6;
        uint16_t                            : 1;
        uint16_t int_ld_sign                : 1;
        uint16_t ld_transfer_start_pointer  : 6;
        uint16_t                            : 2;
    }bit;
} xc24r_grp1_ld_transfer_start_pointer_threshold_t;

typedef union tag_XC24R_GRP1_LOCAL_WR_TRANSFER_POINTER_0x0D_T
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_transfer_write_pointer: 6;
        uint16_t                                : 2;
        uint16_t local_wr_transfer_read_pointer : 6;
        uint16_t                                : 2;
    }bit;
} xc24r_grp1_local_wr_transfer_pointer_t;

typedef union tag_XC24R_GRP1_LOCAL_RD_RECEIVE_POINTER_0x0E_T
{
    uint16_t ALL;
    struct
    {
        uint16_t local_rd_receive_write_pointer : 6;
        uint16_t                                : 2;
        uint16_t local_rd_receive_read_pointer  : 6;
        uint16_t                                : 2;
    }bit;
} xc24r_grp1_local_rd_receive_pointer_t;

typedef union tag_XC24R_GRP1_LOCAL_RW_DIFFERENCE_POINTER_0x0F_T
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_transfer_diff_pointer : 6;
        uint16_t                                : 2;
        uint16_t local_rd_receive_diff_pointer  : 6;
        uint16_t                                : 2;
    }bit;
} xc24r_grp1_local_rw_difference_pointer_t;

typedef union tag_XC24R_GRP1_LOCAL_RW_POINTER_RESET_0x10_T
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_pointer_rst   : 1;
        uint16_t                        : 7;
        uint16_t local_rd_pointer_rst   : 1;
        uint16_t                        : 7;
    }bit;
} xc24r_grp1_local_rw_pointer_reset_t;

typedef union tag_XC24R_GRP1_FAULT_AUTO_READ_INTERVAL_0x11_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_auto_rd_interval     :16;
    }bit;
} xc24r_grp1_fault_auto_read_interval_t;

typedef union tag_XC24R_GRP1_FAULT_AUTO_READ_EVENT_0x12_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_auto_rd_interval     : 1;
        uint16_t                            : 7;
        uint16_t fault_auto_rd_timer_event  : 8;
    }bit;
} xc24r_grp1_fault_auto_read_event_t;

typedef union tag_XC24R_GRP1_INTERRUPT_ENABLE_0x13_T
{
    uint16_t ALL;
    struct
    {
        uint16_t int_fb_en                  : 1;
        uint16_t int_open_en                : 1;
        uint16_t int_short_en               : 1;
        uint16_t int_thermal_en             : 1;
        uint16_t int_ld_en                  : 1;
        uint16_t int_rd_rec_fail_en         : 1;
        uint16_t int_fault_auto_rd_fail_en  : 1;
        uint16_t int_fault_rd_fail_en       : 1;
        uint16_t int_timeout_err_en         : 1;
        uint16_t int_spi_pc_en              : 1;
        uint16_t int_acc_cnt_err_en         : 1;
        uint16_t int_cmd_pc_en              : 1;
        uint16_t int_uv15_en                : 1;
        uint16_t int_ov15_en                : 1;
        uint16_t int_uv50_en                : 1;
        uint16_t int_ov50_en                : 1;
    }bit;
} xc24r_grp1_interrupt_enable_t;

typedef union tag_XC24R_GRP1_COMMAND_STATUS1_0x14_T
{
    uint16_t ALL;
    struct
    {
        uint16_t sync_doing         : 1;
        uint16_t sync_done          : 1;
        uint16_t sync_auto_doing    : 1;
        uint16_t sync_auto_done     : 1;
        uint16_t fault_auto_doing   : 1;
        uint16_t fault_auto_done    : 1;
        uint16_t fault_doing        : 1;
        uint16_t fault_done         : 1;
        uint16_t ld_trans_doing     : 1;
        uint16_t ld_trans_done      : 1;
        uint16_t global_wr_doing    : 1;
        uint16_t global_wr_done     : 1;
        uint16_t local_wr_doing     : 1;
        uint16_t local_wr_done      : 1;
        uint16_t local_rd_doing     : 1;
        uint16_t local_rd_done      : 1;
    }bit;
} xc24r_grp1_command_status1_t;

typedef union tag_XC24R_GRP1_COMMAND_STATUS2_0x15_T
{
    uint16_t ALL;
    struct
    {
        uint16_t id_doing   : 1;
        uint16_t id_done    : 1;
        uint16_t            :14;
    }bit;
} xc24r_grp1_command_status2_t;

typedef union tag_XC24R_GRP1_RECEIVE_STATUS_0x16_T
{
    uint16_t ALL;
    struct
    {
        uint16_t local_rec_doing        : 1;
        uint16_t local_rec_done         : 1;
        uint16_t fault_auto_rec_doing   : 1;
        uint16_t fault_auto_rec_done    : 1;
        uint16_t fault_rec_doing        : 1;
        uint16_t fault_rec_done         : 1;
        uint16_t                        : 2;
        uint16_t timeout_err            : 1;
        uint16_t                        : 3;
        uint16_t local_rec_fail         : 1;
        uint16_t fault_auto_rec_fail    : 1;
        uint16_t fault_rec_fail         : 1;
        uint16_t                        : 1;
    }bit;
} xc24r_grp1_receive_status_t;

typedef union tag_XC24R_GRP1_INTERRUPT_STATUS_0x17_T
{
    uint16_t ALL;
    struct
    {
        uint16_t int_fault                      : 1;
        uint16_t int_ld                         : 1;
        uint16_t int_source1                    : 1;
        uint16_t int_source2                    : 1;
        uint16_t int_source3                    : 1;
        uint16_t int_source4                    : 1;
        uint16_t                                : 2;
        uint16_t int_ld_tran_src                : 1;
        uint16_t int_rd_rec_fail_src            : 1;
        uint16_t int_fault_auto_rec_fail_src    : 1;
        uint16_t int_fault_rec_fail_src         : 1;
        uint16_t int_timeout_err_src            : 1;
        uint16_t int_parity_err_src             : 1;
        uint16_t int_acc_cnt_err_src            : 1;
        uint16_t int_ovuv_err_src               : 1;
    }bit;
} xc24r_grp1_interrupt_status_t;

typedef union tag_XC24R_GRP1_PARITY_ERR_STATUS1_0x18_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ch1_par_err    : 1;
        uint16_t ch2_par_err    : 1;
        uint16_t ch3_par_err    : 1;
        uint16_t ch4_par_err    : 1;
        uint16_t ch5_par_err    : 1;
        uint16_t ch6_par_err    : 1;
        uint16_t ch7_par_err    : 1;
        uint16_t ch8_par_err    : 1;
        uint16_t ch9_par_err    : 1;
        uint16_t ch10_par_err   : 1;
        uint16_t ch11_par_err   : 1;
        uint16_t ch12_par_err   : 1;
        uint16_t ch13_par_err   : 1;
        uint16_t ch14_par_err   : 1;
        uint16_t ch15_par_err   : 1;
        uint16_t ch16_par_err   : 1;
    }bit;
} xc24r_grp1_parity_err_status1_t;

typedef union tag_XC24R_GRP1_PARITY_ERR_STATUS2_0x19_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ch17_par_err   : 1;
        uint16_t ch18_par_err   : 1;
        uint16_t ch19_par_err   : 1;
        uint16_t ch20_par_err   : 1;
        uint16_t ch21_par_err   : 1;
        uint16_t ch22_par_err   : 1;
        uint16_t ch23_par_err   : 1;
        uint16_t ch24_par_err   : 1;
        uint16_t                : 4;
        uint16_t uv15_det       : 1;
        uint16_t ov15_det       : 1;
        uint16_t uv50_det       : 1;
        uint16_t ov50_det       : 1;
    }bit;
} xc24r_grp1_parity_err_status2_t;

typedef union tag_XC24R_GRP1_SPI_FAULT_STATUS_0x1A_T
{
    uint16_t ALL;
    struct
    {
        uint16_t spi_parity_err_det_en  : 1;
        uint16_t spi_acc_cnt_err_det_en : 1;
        uint16_t                        : 2;
        uint16_t parity_err_status      : 1;
        uint16_t acc_cnt_err_status     : 1;
        uint16_t                        : 2;
        uint16_t acc_counter            : 8;
    }bit;
} xc24r_grp1_spi_fault_status_t;

typedef union tag_XC24R_GRP1_CLK_CONTROL1_0x1B_T
{
    uint16_t ALL;
    struct
    {
        uint16_t                        : 1;
        uint16_t serializer_skew_en     : 1;
        uint16_t osc_spread_en          : 1;
        uint16_t serializer_clk_sel1    : 1;
        uint16_t sprd1_gain             : 3;
        uint16_t                        : 1;
        uint16_t serializer_clk_sel2    : 1;
        uint16_t svo_clk_sel            : 1;
        uint16_t                        : 1;
        uint16_t ld_rd_clk_sel          : 1;
        uint16_t spread1_spd            : 3;
        uint16_t                        : 1;
    }bit;
} xc24r_grp1_clk_control1_t;

typedef union tag_XC24R_GRP1_CLK_CONTROL2_0x1C_T
{
    uint16_t ALL;
    struct
    {
        uint16_t mclk_mode      : 1;
        uint16_t                : 1;
        uint16_t osc2_spread_en : 1;
        uint16_t                : 1;
        uint16_t sprd2_gain     : 3;
        uint16_t                : 5;
        uint16_t spread2_spd    : 3;
        uint16_t                : 1;
    }bit;
} xc24r_grp1_clk_control2_t;

typedef union tag_XC24R_GRP1_SERIALIZER_CLOCK_GEN_0x1D_T
{
    uint16_t ALL;
    struct
    {
        uint16_t serial_clk_high    : 7;
        uint16_t                    : 1;
        uint16_t serial_clk_low     : 7;
        uint16_t                    : 1;
    }bit;
} xc24r_grp1_serializer_clock_gen_t;

typedef union tag_XC24R_GRP1_LATENCY_0x1E_T
{
    uint16_t ALL;
    struct
    {
        uint16_t cmd_latency    : 8;
        uint16_t serial_latency : 8;
    }bit;
} xc24r_grp1_latency_t;

typedef union tag_XC24R_GRP1_TIMEOUT_0x1F_T
{
    uint16_t ALL;
    struct
    {
        uint16_t timeout    :16;
    }bit;
} xc24r_grp1_timeout_t;

typedef union tag_XC24R_GRP1_DAISIED_DEVICE_CH_SIZE_0x20_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size    : 5;
        uint16_t                        :11;
    }bit;
} xc24r_grp1_daisied_device_ch_size_t;

typedef union tag_XC24R_GRP1_DAISY_SIZE1_0x21_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch1 : 5;
        uint16_t daisy_size_ch2 : 5;
        uint16_t daisy_size_ch3 : 5;
        uint16_t                : 1;
    }bit;
} xc24r_grp1_daisy_size1_t;

typedef union tag_XC24R_GRP1_DAISY_SIZE2_0x22_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch4 : 5;
        uint16_t daisy_size_ch5 : 5;
        uint16_t daisy_size_ch6 : 5;
        uint16_t                : 1;
    }bit;
} xc24r_grp1_daisy_size2_t;

typedef union tag_XC24R_GRP1_DAISY_SIZE3_0x23_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch7 : 5;
        uint16_t daisy_size_ch8 : 5;
        uint16_t daisy_size_ch9 : 5;
        uint16_t                : 1;
    }bit;
} xc24r_grp1_daisy_size3_t;

typedef union tag_XC24R_GRP1_DAISY_SIZE4_0x24_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch10    : 5;
        uint16_t daisy_size_ch11    : 5;
        uint16_t daisy_size_ch12    : 5;
        uint16_t                    : 1;
    }bit;
} xc24r_grp1_daisy_size4_t;

typedef union tag_XC24R_GRP1_DAISY_SIZE5_0x25_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch13    : 5;
        uint16_t daisy_size_ch14    : 5;
        uint16_t daisy_size_ch15    : 5;
        uint16_t                    : 1;
    }bit;
} xc24r_grp1_daisy_size5_t;

typedef union tag_XC24R_GRP1_DAISY_SIZE6_0x26_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch16    : 5;
        uint16_t daisy_size_ch17    : 5;
        uint16_t daisy_size_ch18    : 5;
        uint16_t                    : 1;
    }bit;
} xc24r_grp1_daisy_size6_t;

typedef union tag_XC24R_GRP1_DAISY_SIZE7_0x27_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch19    : 5;
        uint16_t daisy_size_ch20    : 5;
        uint16_t daisy_size_ch21    : 5;
        uint16_t                    : 1;
    }bit;
} xc24r_grp1_daisy_size7_t;

typedef union tag_XC24R_GRP1_DAISY_SIZE8_0x28_T
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch22    : 5;
        uint16_t daisy_size_ch23    : 5;
        uint16_t daisy_size_ch24    : 5;
        uint16_t                    : 1;
    }bit;
}xc24r_grp1_daisy_size8_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE1_0x29_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch1 : 8;
        uint16_t block_size_ch2 : 8;
    }bit;
} xc24r_grp1_block_size1_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE2_0x2A_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch3 : 8;
        uint16_t block_size_ch4 : 8;
    }bit;
} xc24r_grp1_block_size2_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE3_0x2B_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch5 : 8;
        uint16_t block_size_ch6 : 8;
    }bit;
} xc24r_grp1_block_size3_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE4_0x2C_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch7 : 8;
        uint16_t block_size_ch8 : 8;
    }bit;
} xc24r_grp1_block_size4_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE5_0x2D_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch9     : 8;
        uint16_t block_size_ch10    : 8;
    }bit;
} xc24r_grp1_block_size5_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE6_0x2E_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch11    : 8;
        uint16_t block_size_ch12    : 8;
    }bit;
} xc24r_grp1_block_size6_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE7_0x2F_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch13    : 8;
        uint16_t block_size_ch14    : 8;
    }bit;
} xc24r_grp1_block_size7_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE8_0x30_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch15    : 8;
        uint16_t block_size_ch16    : 8;
    }bit;
} xc24r_grp1_block_size8_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE9_0x31_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch17    : 8;
        uint16_t block_size_ch18    : 8;
    }bit;
} xc24r_grp1_block_size9_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE10_0x32_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch19    : 8;
        uint16_t block_size_ch20    : 8;
    }bit;
} xc24r_grp1_block_size10_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE11_0x33_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch21    : 8;
        uint16_t block_size_ch22    : 8;
    }bit;
} xc24r_grp1_block_size11_t;

typedef union tag_XC24R_GRP1_BLOCK_SIZE12_0x34_T
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch23    : 8;
        uint16_t block_size_ch24    : 8;
    }bit;
} xc24r_grp1_block_size12_t;

typedef union tag_XC24R_GRP1_CHANNEL_ENABLE1_0x35_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ch1_en     : 1;
        uint16_t ch2_en     : 1;
        uint16_t ch3_en     : 1;
        uint16_t ch4_en     : 1;
        uint16_t ch5_en     : 1;
        uint16_t ch6_en     : 1;
        uint16_t ch7_en     : 1;
        uint16_t ch8_en     : 1;
        uint16_t ch9_en     : 1;
        uint16_t ch10_en    : 1;
        uint16_t ch11_en    : 1;
        uint16_t ch12_en    : 1;
        uint16_t ch13_en    : 1;
        uint16_t ch14_en    : 1;
        uint16_t ch15_en    : 1;
        uint16_t ch16_en    : 1;
    }bit;
} xc24r_grp1_channel_enable1_t;

typedef union tag_XC24R_GRP1_CHANNEL_ENABLE2_0x36_T
{
    uint16_t ALL;
    struct
    {
        uint16_t ch17_en    : 1;
        uint16_t ch18_en    : 1;
        uint16_t ch19_en    : 1;
        uint16_t ch20_en    : 1;
        uint16_t ch21_en    : 1;
        uint16_t ch22_en    : 1;
        uint16_t ch23_en    : 1;
        uint16_t ch24_en    : 1;
        uint16_t ch_size    : 5;
        uint16_t            : 1;
        uint16_t ld_width   : 2;
    }bit;
} xc24r_grp1_channel_enable2_t;

typedef union tag_XC24R_GRP1_FLLCNT11_0x37_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fll1cnt    :16;
    }bit;
} xc24r_grp1_fllcnt11_t;

typedef union tag_XC24R_GRP1_FLLCNT12_0x38_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fll1cnt        : 5;
        uint16_t                : 3;
        uint16_t fll1_err_range : 2;
        uint16_t                : 2;
        uint16_t fll1_range     : 2;
        uint16_t fll1_sync      : 1;
        uint16_t fll1_en        : 1;
    }bit;
} xc24r_grp1_fllcnt12_t;

typedef union tag_XC24R_GRP1_FLLCNT21_0x39_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fll2cnt    :16;
    }bit;
} xc24r_grp1_fllcnt21_t;

typedef union tag_XC24R_GRP1_FLLCNT22_0x3A_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fll2cnt        : 5;
        uint16_t                : 3;
        uint16_t fll2_err_range : 2;
        uint16_t                : 2;
        uint16_t fll2_range     : 2;
        uint16_t fll2_sync      : 1;
        uint16_t fll2_en        : 1;
    }bit;
} xc24r_grp1_fllcnt22_t;

typedef union tag_XC24R_GRP1_VO_DELAY_0x3B_T
{
    uint16_t ALL;
    struct
    {
        uint16_t vo_delay   : 8;
        uint16_t            : 4;
        uint16_t gate1_pol  : 1;
        uint16_t gate2_pol  : 1;
        uint16_t gate3_pol  : 1;
        uint16_t            : 1;
    }bit;
} xc24r_grp1_vo_delay_t;

typedef union tag_XC24R_GRP1_VO_ON_OFF_0x3C_T
{
    uint16_t ALL;
    struct
    {
        uint16_t vo_on  : 8;
        uint16_t vo_off : 8;
    }bit;
} xc24r_grp1_vo_on_off_t;

typedef union tag_XC24R_GRP1_SVO_ON_0x3D_T
{
    uint16_t ALL;
    struct
    {
        uint16_t svo_on :12;
        uint16_t        : 4;
    }bit;
} xc24r_grp1_svo_on_t;

typedef union tag_XC24R_GRP1_SVO1_OFF_0x3E_T
{
    uint16_t ALL;
    struct
    {
        uint16_t svo1_off   :16;
    }bit;
} xc24r_grp1_svo1_off_t;

typedef union tag_XC24R_GRP1_SVO2_OFF_0x3F_T
{
    uint16_t ALL;
    struct
    {
        uint16_t svo2_off   :16;
    }bit;
} xc24r_grp1_svo2_off_t;

typedef union tag_XC24R_GRP1_SVO3_OFF_0x40_T
{
    uint16_t ALL;
    struct
    {
        uint16_t svo3_off   :16;
    }bit;
} xc24r_grp1_svo3_off_t;

typedef union tag_XC24R_GRP1_SVO_NUMBER_0x41_T
{
    uint16_t ALL;
    struct
    {
        uint16_t sv_no      : 6;
        uint16_t            : 3;
        uint16_t sv_no_type : 2;
        uint16_t            : 5;
    }bit;
} xc24r_grp1_svo_number_t;

typedef union tag_XC24R_GRP1_DAC_NF_CONTROL_0x42_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dgrjt_en   : 1;
        uint16_t            :11;
        uint16_t dac_lvl    : 2;
        uint16_t            : 2;
    }bit;
} xc24r_grp1_dac_nf_control_t;

typedef union tag_XC24R_GRP1_DAC_CONTROL_0x43_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_auto      : 1;
        uint16_t dac2_auto      : 1;
        uint16_t dac3_auto      : 1;
        uint16_t dac_auto_type  : 1;
        uint16_t dac_sync_mode  : 1;
        uint16_t dac1_fb_mode   : 1;
        uint16_t dac2_fb_mode   : 1;
        uint16_t dac3_fb_mode   : 1;
        uint16_t dac1_dec1_mode : 1;
        uint16_t dac2_dec1_mode : 1;
        uint16_t dac3_dec1_mode : 1;
        uint16_t                : 1;
        uint16_t dac1_hold_en   : 1;
        uint16_t dac2_hold_en   : 1;
        uint16_t dac3_hold_en   : 1;
        uint16_t dac_fault_off  : 1;
    }bit;
} xc24r_grp1_dac_control_t;

typedef union tag_XC24R_GRP1_CURRENT_TARGET_DAC1_0x44_T
{
    uint16_t ALL;
    struct
    {
        uint16_t curr_tgt_dac1  :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_current_target_dac1_t;

typedef union tag_XC24R_GRP1_CURRENT_TARGET_DAC2_0x45_T
{
    uint16_t ALL;
    struct
    {
        uint16_t curr_tgt_dac2  :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_current_target_dac2_t;

typedef union tag_XC24R_GRP1_CURRENT_TARGET_DAC3_0x46_T
{
    uint16_t ALL;
    struct
    {
        uint16_t curr_tgt_dac3  : 12;
        uint16_t                :  4;
    }bit;
} xc24r_grp1_current_target_dac3_t;

typedef union tag_XC24R_GRP1_PREVIOUS_TARGET_DAC1_0x47_T
{
    uint16_t ALL;
    struct
    {
        uint16_t prev_tgt_dac1  :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_previous_target_dac1_t;

typedef union tag_XC24R_GRP1_PREVIOUS_TARGET_DAC2_0x48_T
{
    uint16_t ALL;
    struct
    {
        uint16_t prev_tgt_dac2  :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_previous_target_dac2_t;

typedef union tag_XC24R_GRP1_PREVIOUS_TARGET_DAC3_0x49_T
{
    uint16_t ALL;
    struct
    {
        uint16_t prev_tgt_dac3  :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_previous_target_dac3_t;

typedef union tag_XC24R_GRP1_DAC1_OUT_0x4A_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_out   :12;
        uint16_t            : 4;
    }bit;
} xc24r_grp1_dac1_out_t;

typedef union tag_XC24R_GRP1_DAC2_OUT_0x4B_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_out   :12;
        uint16_t            : 4;
    }bit;
} xc24r_grp1_dac2_out_t;

typedef union tag_XC24R_GRP1_DAC3_OUT_0x4C_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_out   :12;
        uint16_t            : 4;
    }bit;
} xc24r_grp1_dac3_out_t;

typedef union tag_XC24R_GRP1_DAC1_STATE_0x4D_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_st_en : 4;
        uint16_t dac1_state : 4;
        uint16_t            : 8;
    }bit;
} xc24r_grp1_dac1_state_t;

typedef union tag_XC24R_GRP1_DAC2_STATE_0x4E_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_st_en : 4;
        uint16_t dac2_state : 4;
        uint16_t            : 8;
    }bit;
} xc24r_grp1_dac2_state_t;

typedef union tag_XC24R_GRP1_DAC3_STATE_0x4F_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_st_en : 4;
        uint16_t dac3_state : 4;
        uint16_t            : 8;
    }bit;
} xc24r_grp1_dac3_state_t;

typedef union tag_XC24R_GRP1_DAC1_INCREMENT1_0x50_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc1  : 5;
        uint16_t dac1_inc2  : 5;
        uint16_t dac1_inc3  : 6;
    }bit;
} xc24r_grp1_dac1_increment1_t;

typedef union tag_XC24R_GRP1_DAC1_INCREMENT2_0x51_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc4              : 7;
        uint16_t dac1_inc_hold_limit    : 9;
    }bit;
} xc24r_grp1_dac1_increment2_t;

typedef union tag_XC24R_GRP1_DAC1_DECREMENT_0x52_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc1_wait : 7;
        uint16_t                : 1;
        uint16_t dac1_dec1      : 7;
        uint16_t                : 1;
    }bit;
} xc24r_grp1_dac1_decrement_t;

typedef union tag_XC24R_GRP1_DAC1_INCREMENT_HOLD_TH_0x53_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc_hold_th   : 9;
        uint16_t bit_signed         : 1;
        uint16_t                    : 6;
    }bit;
} xc24r_grp1_dac1_increment_hold_th_t;

typedef union tag_XC24R_GRP1_DAC1_FB_VALID_TIMER_0x54_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_fb_valid_timer    :16;
    }bit;
} xc24r_grp1_dac1_fb_valid_timer_t;

typedef union tag_XC24R_GRP1_DAC1_MIN_LIMIT_0x55_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_min_limit :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_dac1_min_limit_t;

typedef union tag_XC24R_GRP1_DAC1_MAX_LIMIT_0x56_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_max_limit :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_dac1_max_limit_t;

typedef union tag_XC24R_GRP1_DAC2_INCREMENT1_0x57_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc1  : 5;
        uint16_t dac2_inc2  : 5;
        uint16_t dac2_inc3  : 6;
    }bit;
} xc24r_grp1_dac2_increment1_t;

typedef union tag_XC24R_GRP1_DAC2_INCREMENT2_0x58_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc4              : 7;
        uint16_t dac2_inc_hold_limit    : 9;
    }bit;
} xc24r_grp1_dac2_increment2_t;

typedef union tag_XC24R_GRP1_DAC2_DECREMENT_0x59_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc1_wait : 7;
        uint16_t                : 1;
        uint16_t dac2_dec1      : 7;
        uint16_t                : 1;
    }bit;
} xc24r_grp1_dac2_decrement_t;

typedef union tag_XC24R_GRP1_DAC2_INCREMENT_HOLD_TH_0x5A_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc_hold_th   : 9;
        uint16_t bit_signed         : 1;
        uint16_t                    : 6;
    }bit;
} xc24r_grp1_dac2_increment_hold_th_t;

typedef union tag_XC24R_GRP1_DAC2_FB_VALID_TIMER_0x5B_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_fb_valid_timer    :16;
    }bit;
} xc24r_grp1_dac2_fb_valid_timer_t;

typedef union tag_XC24R_GRP1_DAC2_MIN_LIMIT_0x5C_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_min_limit :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_dac2_min_limit_t;

typedef union tag_XC24R_GRP1_DAC2_MAX_LIMIT_0x5D_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_max_limit :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_dac2_max_limit_t;

typedef union tag_XC24R_GRP1_DAC3_INCREMENT1_0x5E_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc1  : 5;
        uint16_t dac3_inc2  : 5;
        uint16_t dac3_inc3  : 6;
    }bit;
} xc24r_grp1_dac3_increment1_t;

typedef union tag_XC24R_GRP1_DAC3_INCREMENT2_0x5F_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc4              : 7;
        uint16_t dac3_inc_hold_limit    : 9;
    }bit;
} xc24r_grp1_dac3_increment2_t;

typedef union tag_XC24R_GRP1_DAC3_DECREMENT_0x60_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc1_wait : 7;
        uint16_t                : 1;
        uint16_t dac3_dec1      : 7;
        uint16_t                : 1;
    }bit;
} xc24r_grp1_dac3_decrement_t;

typedef union tag_XC24R_GRP1_DAC3_INCREMENT_HOLD_TH_0x61_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc_hold_th   : 9;
        uint16_t bit_signed         : 1;
        uint16_t                    : 6;
    }bit;
} xc24r_grp1_dac3_increment_hold_th_t;

typedef union tag_XC24R_GRP1_DAC3_FB_VALID_TIMER_0x62_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_fb_valid_timer    :16;
    }bit;
} xc24r_grp1_dac3_fb_valid_timer_t;

typedef union tag_XC24R_GRP1_DAC3_MIN_LIMIT_0x63_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_min_limit :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_dac3_min_limit_t;

typedef union tag_XC24R_GRP1_DAC3_MAX_LIMIT_0x64_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_max_limit :12;
        uint16_t                : 4;
    }bit;
} xc24r_grp1_dac3_max_limit_t;

typedef union tag_XC24R_GRP1_OSC_FLL_MAN_A1_0x65_T
{
    uint16_t ALL;
    struct
    {
        uint16_t flt_gain_a     : 2;
        uint16_t                : 2;
        uint16_t flt_ctl_a      : 2;
        uint16_t                : 2;
        uint16_t dac_rng_a      : 1;
        uint16_t                : 6;
        uint16_t osc_man_en_a   : 1;
    }bit;
} xc24r_grp1_osc_fll_man_a1_t;

typedef union tag_XC24R_GRP1_OSC_FLL_MAN_A2_0x66_T
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_fll_man_a  :16;
    }bit;
} xc24r_grp1_osc_fll_man_a2_t;

typedef union tag_XC24R_GRP1_OSC_FLL_MAN_B1_0x67_T
{
    uint16_t ALL;
    struct
    {
        uint16_t flt_gain_b     : 2;
        uint16_t                : 2;
        uint16_t flt_ctl_b      : 2;
        uint16_t                : 2;
        uint16_t dac_rng_b      : 1;
        uint16_t                : 6;
        uint16_t osc_man_en_b   : 1;
    }bit;
} xc24r_grp1_osc_fll_man_b1_t;

typedef union tag_XC24R_GRP1_OSC_FLL_MAN_B2_0x68_T
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_fll_man_b  :16;
    }bit;
} xc24r_grp1_osc_fll_man_b2_t;

typedef union tag_XC24R_GRP1_GLOBAL_WRITE_DATA_0x69_T
{
    uint16_t ALL;
    struct
    {
        uint16_t data   :12;
        uint16_t        : 4;
    }bit;
} xc24r_grp1_global_write_data_t;

typedef union tag_XC24R_GRP1_GLOBAL_FAULT_READ_DATA1_0x6A_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch1  : 4;
        uint16_t fault_ch2  : 4;
        uint16_t fault_ch3  : 4;
        uint16_t fault_ch4  : 4;
    }bit;
} xc24r_grp1_global_fault_read_data1_t;

typedef union tag_XC24R_GRP1_GLOBAL_FAULT_READ_DATA2_0x6B_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch5  : 4;
        uint16_t fault_ch6  : 4;
        uint16_t fault_ch7  : 4;
        uint16_t fault_ch8  : 4;
    }bit;
} xc24r_grp1_global_fault_read_data2_t;

typedef union tag_XC24R_GRP1_GLOBAL_FAULT_READ_DATA3_0x6C_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch9  : 4;
        uint16_t fault_ch10 : 4;
        uint16_t fault_ch11 : 4;
        uint16_t fault_ch12 : 4;
    }bit;
} xc24r_grp1_global_fault_read_data3_t;

typedef union tag_XC24R_GRP1_GLOBAL_FAULT_READ_DATA4_0x6D_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch13 : 4;
        uint16_t fault_ch14 : 4;
        uint16_t fault_ch15 : 4;
        uint16_t fault_ch16 : 4;
    }bit;
} xc24r_grp1_global_fault_read_data4_t;

typedef union tag_XC24R_GRP1_GLOBAL_FAULT_READ_DATA5_0x6E_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch17 : 4;
        uint16_t fault_ch18 : 4;
        uint16_t fault_ch19 : 4;
        uint16_t fault_ch20 : 4;
    }bit;
} xc24r_grp1_global_fault_read_data5_t;

typedef union tag_XC24R_GRP1_GLOBAL_FAULT_READ_DATA6_0x6F_T
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch21 : 4;
        uint16_t fault_ch22 : 4;
        uint16_t fault_ch23 : 4;
        uint16_t fault_ch24 : 4;
    }bit;
} xc24r_grp1_global_fault_read_data6_t;

typedef union tag_XC24R_GRP1_PORT_LOCAL_RW_DATA_0x70_0xEF_T
{
    uint16_t ALL;
    struct
    {
        uint16_t port_local_rw_data :12;
        uint16_t                    : 4;
    }bit;
} xc24r_grp1_port_local_rw_data_t;

typedef union tag_XC24R_GRP1_MIRROR_TEST_CONTROL_0xF0_T
{
    uint16_t ALL;
    struct
    {
        uint16_t mclk64_o       : 1;
        uint16_t mclk_sel       : 1;
        uint16_t                : 2;
        uint16_t mclk1_o        : 1;
        uint16_t                : 3;
        uint16_t daco1_direct   : 1;
        uint16_t daco2_direct   : 1;
        uint16_t daco3_direct   : 1;
        uint16_t                : 3;
        uint16_t addr_ext       : 1;
        uint16_t test_en        : 1;
    }bit;
} xc24r_grp1_mirror_test_control_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_PG_ACCESS_0xF1_T
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_acc_cycle   :16;
    }bit;
} xc24r_grp1_mirror_otp_pg_access_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_WRITE_0xF2_T
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_wsel       : 4;
        uint16_t otp_rd         : 2;
        uint16_t                : 9;
        uint16_t otp_pg_done    : 1;
    }bit;
} xc24r_grp1_mirror_otp_write_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_RD_PROG_0xF3_T
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_s   : 1;
        uint16_t otp_rd_s   : 1;
    }bit;
} xc24r_grp1_mirror_otp_rd_prog_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_PROTECT_0xF4_T
{
    uint16_t ALL;
    struct
    {
        uint16_t protect    :12;
        uint16_t            : 4;
    }bit;
} xc24r_grp1_mirror_otp_protect_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_MIRROR1_0xF5_T
{
    uint16_t ALL;
    struct
    {
        uint16_t vctl_ldo   : 5;
        uint16_t            : 3;
        uint16_t bgr_tc     : 5;
        uint16_t            : 3;
    }bit;
} xc24r_grp1_mirror_otp_mirror1_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_MIRROR2_0xF6_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_ofs   : 8;
        uint16_t dac_ctl    : 6;
        uint16_t            : 2;
    }bit;
} xc24r_grp1_mirror_otp_mirror2_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_MIRROR3_0xF7_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_ofs   : 8;
        uint16_t dac3_ofs   : 8;
    }bit;
} xc24r_grp1_mirror_otp_mirror3_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_MIRROR4_0xF8_T
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_rctl1      : 5;
        uint16_t                : 3;
        uint16_t ldo_osc_ctl    : 5;
        uint16_t                : 3;
    }bit;
} xc24r_grp1_mirror_otp_mirror4_t;

typedef union tag_XC24R_GRP1_MIRROR_OTP_MIRROR5_0xF9_T
{
    uint16_t ALL;
    struct
    {
        uint16_t osc_rctl2      : 5;
        uint16_t                : 3;
        uint16_t otp_checksum   : 6;
        uint16_t version        : 2;
    }bit;
} xc24r_grp1_mirror_otp_mirror5_t;

typedef union tag_XC24R_GRP1_GATE_CONTROL_0xFA_T
{
    uint16_t ALL;
    struct
    {
        uint16_t gate1L_en  : 1;
        uint16_t gate2L_en  : 1;
        uint16_t gate3L_en  : 1;
        uint16_t            : 1;
        uint16_t gate1H_en  : 1;
        uint16_t gate2H_en  : 1;
        uint16_t gate3H_en  : 1;
        uint16_t            : 9;
    }bit;
} xc24r_grp1_gate_control_t;

typedef union tag_XC24R_GRP1_GATE1_OFFSET_0xFB_T
{
    uint16_t ALL;
    struct
    {
        uint16_t gate1_offset1  : 8;
        uint16_t gate1_offset2  : 8;
    }bit;
} xc24r_grp1_gate1_offset_t;

typedef union tag_XC24R_GRP1_GATE2_OFFSET_0xFC_T
{
    uint16_t ALL;
    struct
    {
        uint16_t gate2_offset1  : 8;
        uint16_t gate2_offset2  : 8;
    }bit;
} xc24r_grp1_gate2_offset_t;

typedef union tag_XC24R_GRP1_GATE3_OFFSET_0xFD_T
{
    uint16_t ALL;
    struct
    {
        uint16_t gate3_offset1  : 8;
        uint16_t gate3_offset2  : 8;
    }bit;
} xc24r_grp1_gate3_offset_t;

typedef union tag_XC24R_GRP1_SV_VAR_CONTROL1_0xFE_T
{
    uint16_t ALL;
    struct
    {
        uint16_t                : 8;
        uint16_t sv_var_size1   : 8;
    }bit;
} xc24r_grp1_sv_var_control1_t;

typedef union tag_XC24R_GRP1_SV_VAR_CONTROL2_0xFF_T
{
    uint16_t ALL;
    struct
    {
        uint16_t sv_var_size2   : 8;
        uint16_t sv_var_size3   : 8;
    }bit;
} xc24r_grp1_sv_var_control2_t;

typedef union tag_XC24R_GRP2_DAC1_FB_VALID_CNT_0x00_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_fb_valid_cnt  :16;
    }bit;
} xc24r_grp2_dac1_fb_valid_cnt_t;

typedef union tag_XC24R_GRP2_DAC1_INC_HOLD_0x01_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac1_inc1_wait_cnt         : 7;
        uint16_t dac1_inc_hold_limit_cnt    : 9;
    }bit;
} xc24r_grp2_dac1_inc_hold_t;

typedef union tag_XC24R_GRP2_1R2_0x02_T
{
    uint16_t ALL;
    struct
    {
        uint16_t _1r2        :12;
        uint16_t bit_signed : 1;
        uint16_t            : 3;
    }bit;
} xc24r_grp2_1r2_t;

typedef union tag_XC24R_GRP2_SOA1_N1_0x03_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa1_n1    : 7;
        uint16_t soa1_n11   : 9;
    }bit;
} xc24r_grp2_soa1_n1_t;

typedef union tag_XC24R_GRP2_SOA1_P12_0x04_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa1_p1    : 9;
        uint16_t            : 3;
        uint16_t soa1_p2    : 4;
    }bit;
} xc24r_grp2_soa1_p12_t;

typedef union tag_XC24R_GRP2_SOA1_P23_0x05_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa1_p2    : 6;
        uint16_t soa1_p3    :10;
    }bit;
} xc24r_grp2_soa1_p23_t;

typedef union tag_XC24R_GRP2_DAC2_FB_VALID_CNT_0x06_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_fb_valid_cnt  :16;
    }bit;
} xc24r_grp2_dac2_fb_valid_cnt_t;

typedef union tag_XC24R_GRP2_DAC2_INC_HOLD_0x07_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac2_inc1_wait_cnt         : 7;
        uint16_t dac2_inc_hold_limit_cnt    : 9;
    }bit;
} xc24r_grp2_dac2_inc_hold_t;

typedef union tag_XC24R_GRP2_2R2_0x08_T
{
    uint16_t ALL;
    struct
    {
        uint16_t _2r2        :12;
        uint16_t bit_signed : 1;
        uint16_t            : 3;
    }bit;
} xc24r_grp2_2r2_t;

typedef union tag_XC24R_GRP2_SOA2_N1_0x09_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa2_n1    : 7;
        uint16_t soa2_n11   : 9;
    }bit;
} xc24r_grp2_soa2_n1_t;

typedef union tag_XC24R_GRP2_SOA2_P12_0x0A_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa2_p1    : 9;
        uint16_t            : 3;
        uint16_t soa2_p2    : 4;
    }bit;
} xc24r_grp2_soa2_p12_t;

typedef union tag_XC24R_GRP2_SOA2_P23_0x0B_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa2_p2    : 6;
        uint16_t soa2_p3    :10;
    }bit;
} xc24r_grp2_soa2_p23_t;

typedef union tag_XC24R_GRP2_DAC3_FB_VALID_CNT_0x0C_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_fb_valid_cnt  :16;
    }bit;
} xc24r_grp2_dac3_fb_valid_cnt_t;

typedef union tag_XC24R_GRP2_DAC3_INC_HOLD_0x0D_T
{
    uint16_t ALL;
    struct
    {
        uint16_t dac3_inc1_wait_cnt         : 7;
        uint16_t dac3_inc_hold_limit_cnt    : 9;
    }bit;
} xc24r_grp2_dac3_inc_hold_t;

typedef union tag_XC24R_GRP2_3R2_0x0E_T
{
    uint16_t ALL;
    struct
    {
        uint16_t _3r2        :12;
        uint16_t bit_signed : 1;
        uint16_t            : 3;
    }bit;
} xc24r_grp2_3r2_t;

typedef union tag_XC24R_GRP2_SOA3_N1_0x0F_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa3_n1    : 7;
        uint16_t soa3_n11   : 9;
    }bit;
} xc24r_grp2_soa3_n1_t;

typedef union tag_XC24R_GRP2_SOA3_P12_0x10_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa3_p1    : 9;
        uint16_t            : 3;
        uint16_t soa3_p2    : 4;
    }bit;
} xc24r_grp2_soa3_p12_t;

typedef union tag_XC24R_GRP2_SOA3_P23_0x11_T
{
    uint16_t ALL;
    struct
    {
        uint16_t soa3_p2    : 6;
        uint16_t soa3_p3    :10;
    }bit;
} xc24r_grp2_soa3_p23_t;

typedef union tag_XC24R_GRP2_ANA_TEST_0x12_T
{
    uint16_t ALL;
    struct
    {
        uint16_t test_ana_en    : 3;
        uint16_t                : 1;
        uint16_t chop_en_bgr    : 1;
        uint16_t                : 3;
        uint16_t chop_en_oscldo : 1;
        uint16_t                : 3;
        uint16_t chop_en        : 1;
        uint16_t                : 3;
    }bit;
} xc24r_grp2_ana_test_t;

typedef union tag_XC24R_GRP1_REGS_T
{
    uint16_t ALL[XC24R_ADDR_GRP1_MAX];
    struct
    {
        xc24r_grp1_soft_reset_t                             _r00;
        xc24r_grp1_global_write_t                           _r01;
        xc24r_grp1_local_write_t                            _r02;
        xc24r_grp1_local_read_t                             _r03;
        xc24r_grp1_id_gen_t                                 _r04;
        xc24r_grp1_fault_read_t                             _r05;
        xc24r_grp1_ld_transfer_t                            _r06;
        xc24r_grp1_sync_gen_t                               _r07;
        xc24r_grp1_command_auto_enable_t                    _r08;
        xc24r_grp1_ld_write_pointer_t                       _r09;
        xc24r_grp1_ld_read_pointer_t                        _r0A;
        xc24r_grp1_ld_difference_pointer_t                  _r0B;
        xc24r_grp1_ld_transfer_start_pointer_threshold_t    _r0C;
        xc24r_grp1_local_wr_transfer_pointer_t              _r0D;
        xc24r_grp1_local_rd_receive_pointer_t               _r0E;
        xc24r_grp1_local_rw_difference_pointer_t            _r0F;

        xc24r_grp1_local_rw_pointer_reset_t                 _r10;
        xc24r_grp1_fault_auto_read_interval_t               _r11;
        xc24r_grp1_fault_auto_read_event_t                  _r12;
        xc24r_grp1_interrupt_enable_t                       _r13;
        xc24r_grp1_command_status1_t                        _r14;
        xc24r_grp1_command_status2_t                        _r15;
        xc24r_grp1_receive_status_t                         _r16;
        xc24r_grp1_interrupt_status_t                       _r17;
        xc24r_grp1_parity_err_status1_t                     _r18;
        xc24r_grp1_parity_err_status2_t                     _r19;
        xc24r_grp1_spi_fault_status_t                       _r1A;
        xc24r_grp1_clk_control1_t                           _r1B;
        xc24r_grp1_clk_control2_t                           _r1C;
        xc24r_grp1_serializer_clock_gen_t                   _r1D;
        xc24r_grp1_latency_t                                _r1E;
        xc24r_grp1_timeout_t                                _r1F;

        xc24r_grp1_daisied_device_ch_size_t                 _r20;
        xc24r_grp1_daisy_size1_t                            _r21;
        xc24r_grp1_daisy_size2_t                            _r22;
        xc24r_grp1_daisy_size3_t                            _r23;
        xc24r_grp1_daisy_size4_t                            _r24;
        xc24r_grp1_daisy_size5_t                            _r25;
        xc24r_grp1_daisy_size6_t                            _r26;
        xc24r_grp1_daisy_size7_t                            _r27;
        xc24r_grp1_daisy_size8_t                            _r28;
        xc24r_grp1_block_size1_t                            _r29;
        xc24r_grp1_block_size2_t                            _r2A;
        xc24r_grp1_block_size3_t                            _r2B;
        xc24r_grp1_block_size4_t                            _r2C;
        xc24r_grp1_block_size5_t                            _r2D;
        xc24r_grp1_block_size6_t                            _r2E;
        xc24r_grp1_block_size7_t                            _r2F;

        xc24r_grp1_block_size8_t                            _r30;
        xc24r_grp1_block_size9_t                            _r31;
        xc24r_grp1_block_size10_t                           _r32;
        xc24r_grp1_block_size11_t                           _r33;
        xc24r_grp1_block_size12_t                           _r34;
        xc24r_grp1_channel_enable1_t                        _r35;
        xc24r_grp1_channel_enable2_t                        _r36;
        xc24r_grp1_fllcnt11_t                               _r37;
        xc24r_grp1_fllcnt12_t                               _r38;
        xc24r_grp1_fllcnt21_t                               _r39;
        xc24r_grp1_fllcnt22_t                               _r3A;
        xc24r_grp1_vo_delay_t                               _r3B;
        xc24r_grp1_vo_on_off_t                              _r3C;
        xc24r_grp1_svo_on_t                                 _r3D;
        xc24r_grp1_svo1_off_t                               _r3E;
        xc24r_grp1_svo2_off_t                               _r3F;

        xc24r_grp1_svo3_off_t                               _r40;
        xc24r_grp1_svo_number_t                             _r41;
        xc24r_grp1_dac_nf_control_t                         _r42;
        xc24r_grp1_dac_control_t                            _r43;
        xc24r_grp1_current_target_dac1_t                    _r44;
        xc24r_grp1_current_target_dac2_t                    _r45;
        xc24r_grp1_current_target_dac3_t                    _r46;
        xc24r_grp1_previous_target_dac1_t                   _r47;
        xc24r_grp1_previous_target_dac2_t                   _r48;
        xc24r_grp1_previous_target_dac3_t                   _r49;
        xc24r_grp1_dac1_out_t                               _r4A;
        xc24r_grp1_dac2_out_t                               _r4B;
        xc24r_grp1_dac3_out_t                               _r4C;
        xc24r_grp1_dac1_state_t                             _r4D;
        xc24r_grp1_dac2_state_t                             _r4E;
        xc24r_grp1_dac3_state_t                             _r4F;

        xc24r_grp1_dac1_increment1_t                        _r50;
        xc24r_grp1_dac1_increment2_t                        _r51;
        xc24r_grp1_dac1_decrement_t                         _r52;
        xc24r_grp1_dac1_increment_hold_th_t                 _r53;
        xc24r_grp1_dac1_fb_valid_timer_t                    _r54;
        xc24r_grp1_dac1_min_limit_t                         _r55;
        xc24r_grp1_dac1_max_limit_t                         _r56;
        xc24r_grp1_dac2_increment1_t                        _r57;
        xc24r_grp1_dac2_increment2_t                        _r58;
        xc24r_grp1_dac2_decrement_t                         _r59;
        xc24r_grp1_dac2_increment_hold_th_t                 _r5A;
        xc24r_grp1_dac2_fb_valid_timer_t                    _r5B;
        xc24r_grp1_dac2_min_limit_t                         _r5C;
        xc24r_grp1_dac2_max_limit_t                         _r5D;
        xc24r_grp1_dac3_increment1_t                        _r5E;
        xc24r_grp1_dac3_increment2_t                        _r5F;

        xc24r_grp1_dac3_decrement_t                         _r60;
        xc24r_grp1_dac3_increment_hold_th_t                 _r61;
        xc24r_grp1_dac3_fb_valid_timer_t                    _r62;
        xc24r_grp1_dac3_min_limit_t                         _r63;
        xc24r_grp1_dac3_max_limit_t                         _r64;
        xc24r_grp1_osc_fll_man_a1_t                         _r65;
        xc24r_grp1_osc_fll_man_a2_t                         _r66;
        xc24r_grp1_osc_fll_man_b1_t                         _r67;
        xc24r_grp1_osc_fll_man_b2_t                         _r68;
        xc24r_grp1_global_write_data_t                      _r69;
        xc24r_grp1_global_fault_read_data1_t                _r6A;
        xc24r_grp1_global_fault_read_data2_t                _r6B;
        xc24r_grp1_global_fault_read_data3_t                _r6C;
        xc24r_grp1_global_fault_read_data4_t                _r6D;
        xc24r_grp1_global_fault_read_data5_t                _r6E;
        xc24r_grp1_global_fault_read_data6_t                _r6F;

        xc24r_grp1_port_local_rw_data_t                     _r70;
        xc24r_grp1_port_local_rw_data_t                     _r71;
        xc24r_grp1_port_local_rw_data_t                     _r72;
        xc24r_grp1_port_local_rw_data_t                     _r73;
        xc24r_grp1_port_local_rw_data_t                     _r74;
        xc24r_grp1_port_local_rw_data_t                     _r75;
        xc24r_grp1_port_local_rw_data_t                     _r76;
        xc24r_grp1_port_local_rw_data_t                     _r77;
        xc24r_grp1_port_local_rw_data_t                     _r78;
        xc24r_grp1_port_local_rw_data_t                     _r79;
        xc24r_grp1_port_local_rw_data_t                     _r7A;
        xc24r_grp1_port_local_rw_data_t                     _r7B;
        xc24r_grp1_port_local_rw_data_t                     _r7C;
        xc24r_grp1_port_local_rw_data_t                     _r7D;
        xc24r_grp1_port_local_rw_data_t                     _r7E;
        xc24r_grp1_port_local_rw_data_t                     _r7F;

        xc24r_grp1_port_local_rw_data_t                     _r80;
        xc24r_grp1_port_local_rw_data_t                     _r81;
        xc24r_grp1_port_local_rw_data_t                     _r82;
        xc24r_grp1_port_local_rw_data_t                     _r83;
        xc24r_grp1_port_local_rw_data_t                     _r84;
        xc24r_grp1_port_local_rw_data_t                     _r85;
        xc24r_grp1_port_local_rw_data_t                     _r86;
        xc24r_grp1_port_local_rw_data_t                     _r87;
        xc24r_grp1_port_local_rw_data_t                     _r88;
        xc24r_grp1_port_local_rw_data_t                     _r89;
        xc24r_grp1_port_local_rw_data_t                     _r8A;
        xc24r_grp1_port_local_rw_data_t                     _r8B;
        xc24r_grp1_port_local_rw_data_t                     _r8C;
        xc24r_grp1_port_local_rw_data_t                     _r8D;
        xc24r_grp1_port_local_rw_data_t                     _r8E;
        xc24r_grp1_port_local_rw_data_t                     _r8F;

        xc24r_grp1_port_local_rw_data_t                     _r90;
        xc24r_grp1_port_local_rw_data_t                     _r91;
        xc24r_grp1_port_local_rw_data_t                     _r92;
        xc24r_grp1_port_local_rw_data_t                     _r93;
        xc24r_grp1_port_local_rw_data_t                     _r94;
        xc24r_grp1_port_local_rw_data_t                     _r95;
        xc24r_grp1_port_local_rw_data_t                     _r96;
        xc24r_grp1_port_local_rw_data_t                     _r97;
        xc24r_grp1_port_local_rw_data_t                     _r98;
        xc24r_grp1_port_local_rw_data_t                     _r99;
        xc24r_grp1_port_local_rw_data_t                     _r9A;
        xc24r_grp1_port_local_rw_data_t                     _r9B;
        xc24r_grp1_port_local_rw_data_t                     _r9C;
        xc24r_grp1_port_local_rw_data_t                     _r9D;
        xc24r_grp1_port_local_rw_data_t                     _r9E;
        xc24r_grp1_port_local_rw_data_t                     _r9F;

        xc24r_grp1_port_local_rw_data_t                     _rA0;
        xc24r_grp1_port_local_rw_data_t                     _rA1;
        xc24r_grp1_port_local_rw_data_t                     _rA2;
        xc24r_grp1_port_local_rw_data_t                     _rA3;
        xc24r_grp1_port_local_rw_data_t                     _rA4;
        xc24r_grp1_port_local_rw_data_t                     _rA5;
        xc24r_grp1_port_local_rw_data_t                     _rA6;
        xc24r_grp1_port_local_rw_data_t                     _rA7;
        xc24r_grp1_port_local_rw_data_t                     _rA8;
        xc24r_grp1_port_local_rw_data_t                     _rA9;
        xc24r_grp1_port_local_rw_data_t                     _rAA;
        xc24r_grp1_port_local_rw_data_t                     _rAB;
        xc24r_grp1_port_local_rw_data_t                     _rAC;
        xc24r_grp1_port_local_rw_data_t                     _rAD;
        xc24r_grp1_port_local_rw_data_t                     _rAE;
        xc24r_grp1_port_local_rw_data_t                     _rAF;

        xc24r_grp1_port_local_rw_data_t                     _rB0;
        xc24r_grp1_port_local_rw_data_t                     _rB1;
        xc24r_grp1_port_local_rw_data_t                     _rB2;
        xc24r_grp1_port_local_rw_data_t                     _rB3;
        xc24r_grp1_port_local_rw_data_t                     _rB4;
        xc24r_grp1_port_local_rw_data_t                     _rB5;
        xc24r_grp1_port_local_rw_data_t                     _rB6;
        xc24r_grp1_port_local_rw_data_t                     _rB7;
        xc24r_grp1_port_local_rw_data_t                     _rB8;
        xc24r_grp1_port_local_rw_data_t                     _rB9;
        xc24r_grp1_port_local_rw_data_t                     _rBA;
        xc24r_grp1_port_local_rw_data_t                     _rBB;
        xc24r_grp1_port_local_rw_data_t                     _rBC;
        xc24r_grp1_port_local_rw_data_t                     _rBD;
        xc24r_grp1_port_local_rw_data_t                     _rBE;
        xc24r_grp1_port_local_rw_data_t                     _rBF;

        xc24r_grp1_port_local_rw_data_t                     _rC0;
        xc24r_grp1_port_local_rw_data_t                     _rC1;
        xc24r_grp1_port_local_rw_data_t                     _rC2;
        xc24r_grp1_port_local_rw_data_t                     _rC3;
        xc24r_grp1_port_local_rw_data_t                     _rC4;
        xc24r_grp1_port_local_rw_data_t                     _rC5;
        xc24r_grp1_port_local_rw_data_t                     _rC6;
        xc24r_grp1_port_local_rw_data_t                     _rC7;
        xc24r_grp1_port_local_rw_data_t                     _rC8;
        xc24r_grp1_port_local_rw_data_t                     _rC9;
        xc24r_grp1_port_local_rw_data_t                     _rCA;
        xc24r_grp1_port_local_rw_data_t                     _rCB;
        xc24r_grp1_port_local_rw_data_t                     _rCC;
        xc24r_grp1_port_local_rw_data_t                     _rCD;
        xc24r_grp1_port_local_rw_data_t                     _rCE;
        xc24r_grp1_port_local_rw_data_t                     _rCF;

        xc24r_grp1_port_local_rw_data_t                     _rD0;
        xc24r_grp1_port_local_rw_data_t                     _rD1;
        xc24r_grp1_port_local_rw_data_t                     _rD2;
        xc24r_grp1_port_local_rw_data_t                     _rD3;
        xc24r_grp1_port_local_rw_data_t                     _rD4;
        xc24r_grp1_port_local_rw_data_t                     _rD5;
        xc24r_grp1_port_local_rw_data_t                     _rD6;
        xc24r_grp1_port_local_rw_data_t                     _rD7;
        xc24r_grp1_port_local_rw_data_t                     _rD8;
        xc24r_grp1_port_local_rw_data_t                     _rD9;
        xc24r_grp1_port_local_rw_data_t                     _rDA;
        xc24r_grp1_port_local_rw_data_t                     _rDB;
        xc24r_grp1_port_local_rw_data_t                     _rDC;
        xc24r_grp1_port_local_rw_data_t                     _rDD;
        xc24r_grp1_port_local_rw_data_t                     _rDE;
        xc24r_grp1_port_local_rw_data_t                     _rDF;

        xc24r_grp1_port_local_rw_data_t                     _rE0;
        xc24r_grp1_port_local_rw_data_t                     _rE1;
        xc24r_grp1_port_local_rw_data_t                     _rE2;
        xc24r_grp1_port_local_rw_data_t                     _rE3;
        xc24r_grp1_port_local_rw_data_t                     _rE4;
        xc24r_grp1_port_local_rw_data_t                     _rE5;
        xc24r_grp1_port_local_rw_data_t                     _rE6;
        xc24r_grp1_port_local_rw_data_t                     _rE7;
        xc24r_grp1_port_local_rw_data_t                     _rE8;
        xc24r_grp1_port_local_rw_data_t                     _rE9;
        xc24r_grp1_port_local_rw_data_t                     _rEA;
        xc24r_grp1_port_local_rw_data_t                     _rEB;
        xc24r_grp1_port_local_rw_data_t                     _rEC;
        xc24r_grp1_port_local_rw_data_t                     _rED;
        xc24r_grp1_port_local_rw_data_t                     _rEE;
        xc24r_grp1_port_local_rw_data_t                     _rEF;

        xc24r_grp1_mirror_test_control_t                    _rF0;
        xc24r_grp1_mirror_otp_pg_access_t                   _rF1;
        xc24r_grp1_mirror_otp_write_t                       _rF2;
        xc24r_grp1_mirror_otp_rd_prog_t                     _rF3;
        xc24r_grp1_mirror_otp_protect_t                     _rF4;
        xc24r_grp1_mirror_otp_mirror1_t                     _rF5;
        xc24r_grp1_mirror_otp_mirror2_t                     _rF6;
        xc24r_grp1_mirror_otp_mirror3_t                     _rF7;
        xc24r_grp1_mirror_otp_mirror4_t                     _rF8;
        xc24r_grp1_mirror_otp_mirror5_t                     _rF9;
        xc24r_grp1_gate_control_t                           _rFA;
        xc24r_grp1_gate1_offset_t                           _rFB;
        xc24r_grp1_gate2_offset_t                           _rFC;
        xc24r_grp1_gate3_offset_t                           _rFD;
        xc24r_grp1_sv_var_control1_t                        _rFE;
        xc24r_grp1_sv_var_control2_t                        _rFF;
    };
} xc24r_grp1_regs_t;

typedef union tag_XC24R_GRP2_REGS_T
{
    uint16_t ALL[XC24R_ADDR_GRP2_MAX];
    struct
    {
        xc24r_grp2_dac1_fb_valid_cnt_t  _r00;
        xc24r_grp2_dac1_inc_hold_t      _r01;
        xc24r_grp2_1r2_t                _r02;
        xc24r_grp2_soa1_n1_t            _r03;
        xc24r_grp2_soa1_p12_t           _r04;
        xc24r_grp2_soa1_p23_t           _r05;
        xc24r_grp2_dac2_fb_valid_cnt_t  _r06;
        xc24r_grp2_dac2_inc_hold_t      _r07;
        xc24r_grp2_2r2_t                _r08;
        xc24r_grp2_soa2_n1_t            _r09;
        xc24r_grp2_soa2_p12_t           _r0A;
        xc24r_grp2_soa2_p23_t           _r0B;
        xc24r_grp2_dac3_fb_valid_cnt_t  _r0C;
        xc24r_grp2_dac3_inc_hold_t      _r0D;
        xc24r_grp2_3r2_t                _r0E;
        xc24r_grp2_soa3_n1_t            _r0F;

        xc24r_grp2_soa3_p12_t           _r10;
        xc24r_grp2_soa3_p23_t           _r11;
        xc24r_grp2_ana_test_t           _r12;
    };
} xc24r_grp2_regs_t;

/* BEGIN - INTERFACE FUNCTIONS */

extern volatile uint8_t gn_xc_spi_timeout;

extern void XC24R_Write_Group1_Register(xc24r_addr_grp1_t in_addr, uint16_t in_data);
extern void XC24R_Write_Group2_Register(xc24r_addr_grp2_t in_addr, uint16_t in_data);
extern uint16_t XC24R_Read_Group1_Register(xc24r_addr_grp1_t in_addr);
extern uint16_t XC24R_Read_Group2_Register(xc24r_addr_grp2_t in_addr);
extern void XC24R_Read_Register_All(void);
extern void XC24R_Dump_All_Register(void);
extern void XC24R_Init(void);

extern void XC24R_Start_MCLK_Oscillation(bool en);
extern void XC24R_Start_FLLSync_Oscillation(bool en);
extern bool IS_XC24R_Support(void);
extern void USE_XC24R(bool b_support);

extern void XC24R_IF_IdGen_Command(void);
extern void XC24R_IF_SyncGen_Command(void);
extern uint16_t XC24R_IF_Fault_Read_Command(void);

extern void XC24R_IF_Write_XDIC(uint8_t in_XDIC_addr, uint16_t in_XDIC_data);
extern uint16_t XC24R_IF_Read_XDIC(uint8_t in_XDIC_addr);
extern void XC24R_IF_Write_LD(uint16_t* p_in_LD_data);

extern void XC24R_Turn_Off_Sync_Auto(void);
extern void XC24R_Turn_On_Sync_Auto(void);

/* END   - INTERFACE FUNCTIONS */
#ifdef __cplusplus
}
#endif

#endif /* ~__XC24R_H__ */

/*** end of file ***/