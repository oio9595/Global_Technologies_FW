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

#define CMD_CODE_NONE       0x00    /* 0b00 : No operation */
#define CMD_CODE_REG_READ   0x01    /* 0b01 : Register-read */
#define CMD_CODE_REG_WRITE  0x02    /* 0b10 : Register-write */
#define CMD_CODE_LD_TRANS   0x03    /* 0b11 : Local dimming data transfer */

#define CH_SEG_1            0x00    /* CH01 ~ CH08 */
#define CH_SEG_2            0x01    /* CH09 ~ CH16 */
#define CH_SEG_3            0x20    /* CH17 ~ CH24 */

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
typedef enum _xc24_addr_
{
    XC24_ADDR_SOFT_RESET = 0x00,                   // 0x00
    XC24_ADDR_GLOBAL_WRITE = 0x01,                 // 0x01
    XC24_ADDR_LOCAL_WRITE = 0x02,                  // 0x02
    XC24_ADDR_LOCAL_READ = 0x03,                   // 0x03
    XC24_ADDR_ID_GEN = 0x04,                       // 0x04
    XC24_ADDR_FAULT_READ = 0x05,                   // 0x05
    XC24_ADDR_LD_TRANSFER = 0x06,                  // 0x06
    XC24_ADDR_SYNC_GEN = 0x07,                     // 0x07
    XC24_ADDR_AUTO_ENABLE = 0x08,                  // 0x08
    XC24_ADDR_LD_WRITE_POINTER = 0x0A,             // 0x0A
    XC24_ADDR_LD_READ_POINTER = 0x0B,              // 0x0B
    XC24_ADDR_DIFFERENCE_POINTER = 0x0C,           // 0x0C
    XC24_ADDR_LD_TRANSFER_START_POINTER_TH = 0x0D, // 0x0D
    XC24_ADDR_LOCAL_WR_TRANSFER_POINTER = 0x0E,    // 0x0E
    XC24_ADDR_LOCAL_RD_RECEIVE_POINTER = 0x0F,     // 0x0F

    XC24_ADDR_LOCAL_RW_DIFFERENCE_POINTER = 0x10,  // 0x10
    XC24_ADDR_LOCAL_RW_POINTER_RESET = 0x11,       // 0x11
    XC24_ADDR_FAULT_AUTO_READ_TIMER = 0x12,        // 0x12
    XC24_ADDR_FAULT_AUTO_READ_EVENT = 0x13,        // 0x13
    XC24_ADDR_SERIALIZER_CLOCK_GEN = 0x14,         // 0x14
    XC24_ADDR_INTERRUPT_ENABLE = 0x15,             // 0x15
    XC24_ADDR_COMMAND_STATUS1 = 0x16,              // 0x16
    XC24_ADDR_COMMAND_STATUS2 = 0x17,              // 0x17
    XC24_ADDR_RECEIVE_STATUS = 0x18,               // 0x18
    XC24_ADDR_INTERRUPT_STATUS = 0x19,             // 0x19
    XC24_ADDR_SPI_FAULT_STATUS_CONTROL = 0x1A,     // 0x1A
    XC24_ADDR_CLK_CONTROL_1 = 0x1B,                // 0x1B
    XC24_ADDR_CLK_CONTROL_2 = 0x1C,                // 0x1C
    XC24_ADDR_VDD_LDO_STATUS = 0x1D,               // 0x1D
    XC24_ADDR_VDD_LDO_FAULT_LEVEL = 0x1E,          // 0x1E
    XC24_ADDR_COMMAND_LATENCY = 0x1F,              // 0x1F

    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE1 = 0x20, // 0x20
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE2,        // 0x21
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE3,        // 0x22
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE4,        // 0x23
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE5,        // 0x24
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE6,        // 0x25
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE7,        // 0x26
    XC24_ADDR_DAISIED_DEVICE_CHANNEL_SIZE8,        // 0x27

    XC24_ADDR_DAISY_SIZE1 = 0x30,                  // 0x30
    XC24_ADDR_DAISY_SIZE2,                         // 0x31
    XC24_ADDR_DAISY_SIZE3,                         // 0x32
    XC24_ADDR_DAISY_SIZE4,                         // 0x33
    XC24_ADDR_DAISY_SIZE5,                         // 0x34
    XC24_ADDR_DAISY_SIZE6,                         // 0x35
    XC24_ADDR_DAISY_SIZE7,                         // 0x36
    XC24_ADDR_DAISY_SIZE8,                         // 0x37
    XC24_ADDR_BLOCK_SIZE1 = 0x38,                  // 0x38
    XC24_ADDR_BLOCK_SIZE2,                         // 0x39
    XC24_ADDR_BLOCK_SIZE3,                         // 0x3A
    XC24_ADDR_BLOCK_SIZE4,                         // 0x3B
    XC24_ADDR_BLOCK_SIZE5,                         // 0x3C
    XC24_ADDR_BLOCK_SIZE6,                         // 0x3D
    XC24_ADDR_BLOCK_SIZE7,                         // 0x3E
    XC24_ADDR_BLOCK_SIZE8,                         // 0x3F

    XC24_ADDR_BLOCK_SIZE9,                         // 0x40
    XC24_ADDR_BLOCK_SIZE10,                        // 0x41
    XC24_ADDR_BLOCK_SIZE11,                        // 0x42
    XC24_ADDR_BLOCK_SIZE12,                        // 0x43
    XC24_ADDR_CHANNEL_ENABLE1 = 0x45,              // 0x45
    XC24_ADDR_CHANNEL_ENABLE2,                     // 0x46
    XC24_ADDR_DAC_CONTROL = 0x4D,			       // 0x4D
    XC24_ADDR_CURRENT_TARGET_DAC = 0x4F,		   // 0x4F

    XC24_ADDR_PREVIOUS_TARGET_DAC = 0x50,		   // 0x50
    XC24_ADDR_DAC_OUT = 0x51, 			           // 0x51
    XC24_ADDR_DAC_INCREMENT1 = 0x52,		       // 0x52
    XC24_ADDR_DAC_INCREMENT2_HOLD_LIMIT = 0x53,	   // 0x53
    XC24_ADDR_DAC_DECREMENT_INC_WAIT = 0x54,	   // 0x54
    XC24_ADDR_DAC_INCREMENT_HOLD_TH = 0x55,	       // 0x55
    XC24_ADDR_SOA_N11_N1 = 0x56,			       // 0x56
    XC24_ADDR_SOA_P2_P1 = 0x57,			           // 0x57
    XC24_ADDR_SOA_P3_P2 = 0x58,			       	   // 0x58
    XC24_ADDR_DAC_FB_VALID_TIMER = 0x59,		   // 0x59
    XC24_ADDR_DAC_MIN_LIMIT = 0x5A,			       // 0x5A
    XC24_ADDR_DAC_MAX_LIMIT = 0x5B,			       // 0x5B
    XC24_ADDR_DAC_STATE = 0x5C,			           // 0x5C
    XC24_ADDR_VALID_CNT = 0x5D,			           // 0x5D
    XC24_ADDR_DAC_INC_HOLD_WAIT_CNT = 0x5E,	       // 0x5E
    XC24_ADDR_R2 = 0x5F,			       		   // 0x5F

    XC24_ADDR_GLOBAL_WRITE_DATA = 0x60,
    XC24_ADDR_GLOBAL_FAULT_READ_DATA1,
    XC24_ADDR_GLOBAL_FAULT_READ_DATA2,
    XC24_ADDR_GLOBAL_FAULT_READ_DATA3,
    XC24_ADDR_GLOBAL_FAULT_READ_DATA4,
    XC24_ADDR_GLOBAL_FAULT_READ_DATA5,
    XC24_ADDR_GLOBAL_FAULT_READ_DATA6,			   // 0x66

    XC24_ADDR_PORT1_LOCAL_RW_DATA1 = 0x70,         // 0x70
    XC24_ADDR_MAX,                                 // 0x71
}xc24_addr_t;

typedef enum tag_XC24_TRIM_ADDR
{
    XC24_ADDR_TRIM_START = 0xF0,                    // 0xF0
    XC24_ADDR_TRIM_TEST_CONTROL = 0xF0,             // 0xF0
    XC24_ADDR_TRIM_OTP_PG_ACCESS = 0xF1,            // 0xF1
    XC24_ADDR_TRIM_OTP_WRITE = 0xF2,                // 0xF2
    XC24_ADDR_TRIM_OTP_RD_PROG = 0xF3,              // 0xF3
    XC24_ADDR_TRIM_OTP_PROTECT = 0xF4,              // 0xF4
    XC24_ADDR_TRIM_MIRROR1 = 0xF5,                  // 0xF5
    XC24_ADDR_TRIM_MIRROR2 = 0xF6,                  // 0xF6
    XC24_ADDR_TRIM_MIRROR3 = 0xF7,                  // 0xF7
    XC24_ADDR_TRIM_MAX,                             // 0xF8
}xc24_addr_trim_t;

/*
typedef enum _xc24_data_addr_
{
    XC24_ADDR_PORT1_LOCAL_RW_DATA1 = 0x70,
    XC24_ADDR_PORT2_LOCAL_RW_DATA1,
    XC24_ADDR_PORT3_LOCAL_RW_DATA1,
    XC24_ADDR_PORT4_LOCAL_RW_DATA1,
    XC24_ADDR_PORT5_LOCAL_RW_DATA1,
    XC24_ADDR_PORT6_LOCAL_RW_DATA1,
    XC24_ADDR_PORT7_LOCAL_RW_DATA1,
    XC24_ADDR_PORT8_LOCAL_RW_DATA1,
    XC24_ADDR_PORT1_LOCAL_RW_DATA2,
    XC24_ADDR_PORT2_LOCAL_RW_DATA2,
    XC24_ADDR_PORT3_LOCAL_RW_DATA2,
    XC24_ADDR_PORT4_LOCAL_RW_DATA2,
    XC24_ADDR_PORT5_LOCAL_RW_DATA2,
    XC24_ADDR_PORT6_LOCAL_RW_DATA2,
    XC24_ADDR_PORT7_LOCAL_RW_DATA2,
    XC24_ADDR_PORT8_LOCAL_RW_DATA2,
    XC24_ADDR_PORT1_LOCAL_RW_DATA3,
    XC24_ADDR_PORT2_LOCAL_RW_DATA3,
    XC24_ADDR_PORT3_LOCAL_RW_DATA3,
    XC24_ADDR_PORT4_LOCAL_RW_DATA3,
    XC24_ADDR_PORT5_LOCAL_RW_DATA3,
    XC24_ADDR_PORT6_LOCAL_RW_DATA3,
    XC24_ADDR_PORT7_LOCAL_RW_DATA3,
    XC24_ADDR_PORT8_LOCAL_RW_DATA3,
    XC24_ADDR_PORT1_LOCAL_RW_DATA4,
    XC24_ADDR_PORT2_LOCAL_RW_DATA4,
    XC24_ADDR_PORT3_LOCAL_RW_DATA4,
    XC24_ADDR_PORT4_LOCAL_RW_DATA4,
    XC24_ADDR_PORT5_LOCAL_RW_DATA4,
    XC24_ADDR_PORT6_LOCAL_RW_DATA4,
    XC24_ADDR_PORT7_LOCAL_RW_DATA4,
    XC24_ADDR_PORT8_LOCAL_RW_DATA4,

    XC24_ADDR_PORT1_LOCAL_RW_DATA5,
    XC24_ADDR_PORT2_LOCAL_RW_DATA5,
    XC24_ADDR_PORT3_LOCAL_RW_DATA5,
    XC24_ADDR_PORT4_LOCAL_RW_DATA5,
    XC24_ADDR_PORT5_LOCAL_RW_DATA5,
    XC24_ADDR_PORT6_LOCAL_RW_DATA5,
    XC24_ADDR_PORT7_LOCAL_RW_DATA5,
    XC24_ADDR_PORT8_LOCAL_RW_DATA5,
    XC24_ADDR_PORT1_LOCAL_RW_DATA6,
    XC24_ADDR_PORT2_LOCAL_RW_DATA6,
    XC24_ADDR_PORT3_LOCAL_RW_DATA6,
    XC24_ADDR_PORT4_LOCAL_RW_DATA6,
    XC24_ADDR_PORT5_LOCAL_RW_DATA6,
    XC24_ADDR_PORT6_LOCAL_RW_DATA6,
    XC24_ADDR_PORT7_LOCAL_RW_DATA6,
    XC24_ADDR_PORT8_LOCAL_RW_DATA6,
    XC24_ADDR_PORT1_LOCAL_RW_DATA7,
    XC24_ADDR_PORT2_LOCAL_RW_DATA7,
    XC24_ADDR_PORT3_LOCAL_RW_DATA7,
    XC24_ADDR_PORT4_LOCAL_RW_DATA7,
    XC24_ADDR_PORT5_LOCAL_RW_DATA7,
    XC24_ADDR_PORT6_LOCAL_RW_DATA7,
    XC24_ADDR_PORT7_LOCAL_RW_DATA7,
    XC24_ADDR_PORT8_LOCAL_RW_DATA7,
    XC24_ADDR_PORT1_LOCAL_RW_DATA8,
    XC24_ADDR_PORT2_LOCAL_RW_DATA8,
    XC24_ADDR_PORT3_LOCAL_RW_DATA8,
    XC24_ADDR_PORT4_LOCAL_RW_DATA8,
    XC24_ADDR_PORT5_LOCAL_RW_DATA8,
    XC24_ADDR_PORT6_LOCAL_RW_DATA8,
    XC24_ADDR_PORT7_LOCAL_RW_DATA8,
    XC24_ADDR_PORT8_LOCAL_RW_DATA8,

    XC24_ADDR_PORT1_LOCAL_RW_DATA9,
    XC24_ADDR_PORT2_LOCAL_RW_DATA9,
    XC24_ADDR_PORT3_LOCAL_RW_DATA9,
    XC24_ADDR_PORT4_LOCAL_RW_DATA9,
    XC24_ADDR_PORT5_LOCAL_RW_DATA9,
    XC24_ADDR_PORT6_LOCAL_RW_DATA9,
    XC24_ADDR_PORT7_LOCAL_RW_DATA9,
    XC24_ADDR_PORT8_LOCAL_RW_DATA9,
    XC24_ADDR_PORT1_LOCAL_RW_DATA10,
    XC24_ADDR_PORT2_LOCAL_RW_DATA10,
    XC24_ADDR_PORT3_LOCAL_RW_DATA10,
    XC24_ADDR_PORT4_LOCAL_RW_DATA10,
    XC24_ADDR_PORT5_LOCAL_RW_DATA10,
    XC24_ADDR_PORT6_LOCAL_RW_DATA10,
    XC24_ADDR_PORT7_LOCAL_RW_DATA10,
    XC24_ADDR_PORT8_LOCAL_RW_DATA10,
    XC24_ADDR_PORT1_LOCAL_RW_DATA11,
    XC24_ADDR_PORT2_LOCAL_RW_DATA11,
    XC24_ADDR_PORT3_LOCAL_RW_DATA11,
    XC24_ADDR_PORT4_LOCAL_RW_DATA11,
    XC24_ADDR_PORT5_LOCAL_RW_DATA11,
    XC24_ADDR_PORT6_LOCAL_RW_DATA11,
    XC24_ADDR_PORT7_LOCAL_RW_DATA11,
    XC24_ADDR_PORT8_LOCAL_RW_DATA11,
    XC24_ADDR_PORT1_LOCAL_RW_DATA12,
    XC24_ADDR_PORT2_LOCAL_RW_DATA12,
    XC24_ADDR_PORT3_LOCAL_RW_DATA12,
    XC24_ADDR_PORT4_LOCAL_RW_DATA12,
    XC24_ADDR_PORT5_LOCAL_RW_DATA12,
    XC24_ADDR_PORT6_LOCAL_RW_DATA12,
    XC24_ADDR_PORT7_LOCAL_RW_DATA12,
    XC24_ADDR_PORT8_LOCAL_RW_DATA12,

    XC24_ADDR_PORT1_LOCAL_RW_DATA13,
    XC24_ADDR_PORT2_LOCAL_RW_DATA13,
    XC24_ADDR_PORT3_LOCAL_RW_DATA13,
    XC24_ADDR_PORT4_LOCAL_RW_DATA13,
    XC24_ADDR_PORT5_LOCAL_RW_DATA13,
    XC24_ADDR_PORT6_LOCAL_RW_DATA13,
    XC24_ADDR_PORT7_LOCAL_RW_DATA13,
    XC24_ADDR_PORT8_LOCAL_RW_DATA13,
    XC24_ADDR_PORT1_LOCAL_RW_DATA14,
    XC24_ADDR_PORT2_LOCAL_RW_DATA14,
    XC24_ADDR_PORT3_LOCAL_RW_DATA14,
    XC24_ADDR_PORT4_LOCAL_RW_DATA14,
    XC24_ADDR_PORT5_LOCAL_RW_DATA14,
    XC24_ADDR_PORT6_LOCAL_RW_DATA14,
    XC24_ADDR_PORT7_LOCAL_RW_DATA14,
    XC24_ADDR_PORT8_LOCAL_RW_DATA14,

    XC24_ADDR_PORT1_LOCAL_RW_DATA15,
    XC24_ADDR_PORT2_LOCAL_RW_DATA15,
    XC24_ADDR_PORT3_LOCAL_RW_DATA15,
    XC24_ADDR_PORT4_LOCAL_RW_DATA15,
    XC24_ADDR_PORT5_LOCAL_RW_DATA15,
    XC24_ADDR_PORT6_LOCAL_RW_DATA15,
    XC24_ADDR_PORT7_LOCAL_RW_DATA15,
    XC24_ADDR_PORT8_LOCAL_RW_DATA15,
    XC24_ADDR_PORT1_LOCAL_RW_DATA16,
    XC24_ADDR_PORT2_LOCAL_RW_DATA16,
    XC24_ADDR_PORT3_LOCAL_RW_DATA16,
    XC24_ADDR_PORT4_LOCAL_RW_DATA16,
    XC24_ADDR_PORT5_LOCAL_RW_DATA16,
    XC24_ADDR_PORT6_LOCAL_RW_DATA16,
    XC24_ADDR_PORT7_LOCAL_RW_DATA16,
    XC24_ADDR_PORT8_LOCAL_RW_DATA16,
}xc24_addr_port_local_t;
*/

/* SOFT RESET : default 0x00 */
typedef union tag_DUMMY
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy  : 16;   /*  */
    };
}_v_dummy_t;


/* SOFT RESET : default 0x00 */
typedef union tag_SOFT_RESET
{
    uint16_t ALL;
    struct
    {
        uint16_t rst1    :  1;   /* RST1 : Reset to all logics, Auto-cleared (No need to clear after set). It need to set this bit after POR to ensure XC-24 reset. */
        uint16_t rst2    :  1;   /* RST2 : Reset to all serializers’ and de-serializers’ state machine, Auto-cleared. Normally, no need to use this bit. */
        uint16_t rst3    :  1;   /* RST3 : Reset to all types of buffer, the frame buffers, the global and local r/w registers and the global fault read registers. Auto-cleared. Normally, no need to use this bit. */
        uint16_t         :  1;   /* reserved */
        uint16_t vs_rst1 :  1;   /* VS_RST1 : If this bit is set, all serializers’ sm are reset at VSYNC rising. Normally, no need to use this bit. */
        uint16_t vs_rst2 :  1;   /* VS_RST2 : If this bit is set, all de-serializers’ sm are reset at VSYNC rising. Normally, no need to use this bit. */
        uint16_t         : 10;   /* reserved */
    };
}_v_soft_reset_t;

/* GLOBAL-WRITE COMMAND : default 0x00 */
typedef union tag_GLOBAL_WRITE
{
    uint16_t ALL;
    struct
    {
        uint16_t addr   :  6;   /* ADDRESS : The same target address of all XD-12 daisied devices to write. */
        uint16_t        :  9;   /* reserved */
        uint16_t start  :  1;   /*  START : ‘0’   No operation.
                                            ‘1’   Start executing the global-write command with the GLOBAL WRITE DATA register, together, and this bit is auto-cleared after completing this execution.
                                            For correct operation, It needs to set the GLOBAL WRITE DATA register with desired value before setting this bit.
                                            The global-writing can be used in case that the target registers’ values of all XD-12 daisied devices are all the same. If the values are different from each other, then the local-write command should be used instead of the global-write command.
                                */
    };
}_v_global_write_t;

/* LOCAL-WRITE COMMAND : default 0x00 */
typedef union tag_LOCAL_WRITE
{
    uint16_t ALL;
    struct
    {
        uint16_t addr   :  6;   /* ADDRESS : The same target address of all XD-12 daisied devices to write. */
        uint16_t        :  2;   /* reserved */
        uint16_t ch_seg :  2;   /* CH_SEG : These bits are used to designate which channels are activated for this command.
                                            2’b00 : The channel 1 from 8 are activated.
                                            2’b01 : The channel 9 from 16 are activated.
                                            2’b10 : The channel 17 from 24 are activated.
                                            2’b11 : No allocated.
                                            Refer to previous chapter. how to allocate the LOCAL RW DATA registers according to CH_SEG
                                */
        uint16_t        :  5;   /* reserved */
        uint16_t start  :  1;   /* START : ‘0’   No operation.
                                           ‘1’   Start executing the local-write command with the LOCAL RW DATA registers together, and this bit is auto-cleared after completing this execution.
                                                 For correct operation, It needs to set the LOCAL RW DATA registers with desired values before setting this bit.
                                                 The local-write command is used in case that the target registers’ values of all XD-12 daisied devices are different from each other.
                                */
    };
}_v_local_write_t;

/* LOCAL-READ COMMAND : default 0x00 */
typedef union tag_LOCAL_READ
{
    uint16_t ALL;
    struct
    {
        uint16_t addr   :  6;   /* ADDRESS : The same target address of all XD-12 daisied devices to read out. */
        uint16_t        :  2;   /* reserved */
        uint16_t ch_seg :  2;   /* CH_SEG : These bits are used to designate which channels are activated for this command.
                                              2’b00 : The channel 1 from 8 are activated.
                                              2’b01 : The channel 9 from 16 are activated.
                                              2’b10 : The channel 17 from 24 are activated.
                                              2’b11 : No allocated.
                                              Refer to previous chapter. how to allocate the LOCAL RW DATA registers according to CH_SEG
                                */
        uint16_t        :  5;   /* reserved */
        uint16_t start  :  1;   /* START : ‘0’   No operation.
                                           ‘1’   Start executing the local-write command with the LOCAL RW DATA registers together, and this bit is auto-cleared after completing this execution.
                                                 For correct operation, It needs to set the LOCAL RW DATA registers with desired values before setting this bit.
                                                 The local-read command is used read out the target registers’ values of all XD-12 daisied devices are different from each other.
                                */
    };
}_v_local_read_t;

/* ID-GEN COMMAND : default 0x00 */
typedef union tag_ID_GEN
{
    uint16_t ALL;
    struct
    {
        uint16_t        : 15;   /* reserved */
        uint16_t start  :  1;   /* START :  ‘0’   No operation.
                                            ‘1’   Start executing the id-generating command to all XD-12 daisied devices and this bit is auto-cleared after completing this execution.
                                            All XD-12 daisied devices connected in each channel will have their own ID number after completing the id-generating command.
                                            This command is needed to read-out the XD-12 drivers’ registers data correctly in sequential, so, this command should be executed before executing the local-reading or the fault-reading command.
                                            It is recommended that the id-generating command should be executed firstly on system initialization.
                                */
    };
}_v_id_gen_t;

/* FAULT-READ COMMAND : default 0x00 */
typedef union tag_FAULT_READ
{
    uint16_t ALL;
    struct
    {
        uint16_t        : 15;   /* reserved */
        uint16_t start  :  1;   /* START : ‘0’   No operation.
                                           ‘1’   Start executing the fault-reading command to all XD-12 daisied devices to monitor the fault event like as the thermal occurrence, the short status, the open status and the fb
                                                 status of the XD-12 devices and this bit is auto-cleared after completing this execution.
                                                 This command can be also auto-executed at the VSYNC rising edge if the FAULT_AUTO_EN bit in the COMMAND AUTO ENABLE register is set.
                                */
    };
}_v_fault_read_t;

/* LD-TRANSFER COMMAND : default 0x00 */
typedef union tag_LD_TRANSFER
{
    uint16_t ALL;
    struct
    {
        uint16_t        : 15;   /* reserved */
        uint16_t start  :  1;   /* START : ‘0’   No operation.
                                           ‘1’   Start executing the local dimming data transfer command to all XD-12 daisied devices in order to transfer the LD data in burst from the XC-24 to the XD-12 drivers and this bit is auto-cleared after the completion.
                                                 This command can be also auto-executed immediately after the amount of LD data the transferred to the XC-24 through the SPI interface is reached to the value of the LD_RD_START_POINTER of the LD START POINTER/TH register.
                                */
    };
}_v_ld_transfer_t;

/* SYNC-GEN COMMAND : default 0x00 */
typedef union tag_SYNC_GEN
{
    uint16_t ALL;
    struct
    {
        uint16_t        : 15;   /* reserved */
        uint16_t start  :  1;   /* START : ‘0’   No operation.
                                           ‘1’   Start executing the sync-gen command to all XD-12 daisied devices in order to make the frame synchronization to the drivers and this bit is auto-cleared after this execution.
                                                 This command can be also auto-executed at the VSYNC rising edge, if the SYNC_AUTO_EN bit in the COMMAND AUTO ENABLE register is set.
                                                 It is recommended that the SYNC_AUTO_EN bit is set for good local dimming.
                                */
    };
}_v_sync_gen_t;

/* AUTO-ENABLE COMMAND : default 0x00 */
typedef union tag_CMD_AUTO_ENABLE
{
    uint16_t ALL;
    struct
    {
        uint16_t sync_auto_en   :  1;   /* SYNC+AUTO_EN : ‘0’   No operation.
                                                          ‘1’   Start executing the sync-gen command automatically.
                                                                If this bit is set, the sync-gen command is auto-executed at the time of VSYNC rising edge.
                                        */
        uint16_t                :  3;   /* reserved */
        uint16_t fault_auto_en  :  1;   /* FAULT_AUTO_EN : ‘0’   No operation.
                                                           ‘1’   Start executing the fault-reading command automatically.
                                                                 If this bit is set, the fault-reading command is auto-executed in two conditions.
                                                                 The first is occurred at the time of VSYNC rising edge and the second condition is occurred at the time of expiration of the FAULT AUTO READ TIMER register for the number of occurrence of the FAULT AUTO READ EVENT register.
                                        */
        uint16_t                :  3;   /* reserved */
        uint16_t timeout_en     :  1;   /* TIMEOUT_EN : ‘0’ Nooperation
                                                        ‘1’ The timeout function is activated during the execution of read-back commands. The timer starts from the end of completing whole command frame data out corresponding to all daisied device to first bit arrived time. The timer is expired at(MCLK*8,192).
                                        */
        uint16_t                :  7;   /* reserved */
    };
}_v_cmd_auto_enable_t;

/* LD-WRITE POINTER COMMAND : Read-only */
typedef union tag_LD_WRITE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_wr_pointer  :  9;   /* LD_WR_POINTER
                                        This register value represents the number of times the local dimming data are written into the internal LD buffers through the SPI interface.
                                        The number increases one by one whenever the local dimming data corresponding to (the number of enabled channels * one LD data width) are transferred to the internal LD buffers.
                                        This pointer value is reset at the VSYNC rising.

                                        LD_WIDTH of the CHANNEL ENABLE 2 register :
                                          2’b00 : one block LD data width = 24 bits
                                          2’b01 : one block  LD data width = 24 bits
                                          2’b10 : one block  LD data width = 24 bits
                                          2’b11 : one block  LD data width = 16 bits
                                        */
        uint16_t                :  7;   /* reserved */
    };
}_v_ld_write_pointer_t;

/* LD-READ POINTER COMMAND : Read-only */
typedef union tag_LD_READ_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_rd_pointer  :  9;   /* LD_RD_POINTER
                                        This register value represents the number of times the local dimming data are read-out to transfer the data from the internal LD buffers to the XD-12 drivers.
                                        The number increases one by one whenever the local dimming data corresponding to the one block LD data of the XD-12 drivers are read-out.
                                        This pointer value is reset at the VSYNC rising.
                                        */
        uint16_t                :  7;   /* reserved */
    };
}_v_ld_read_pointer_t;

/* LD-DIFFERENCE POINTER COMMAND : Read-only */
typedef union tag_LD_DIFFERENCE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_diff_pointer :  6;   /* LD_DIFF_POINTER
                                            This value is useful to monitor how much the LD data remains in the internal LD buffers for checking overrun or underrun.
                                              = LD_WR_POINTER – LD_RD_POINTER
                                        */
        uint16_t                 : 10;   /* reserved */
    };
}_v_ld_difference_pointer_t;

/* LD-TRANSFER START POINTER / THRESHOLD : default 0x00 */
typedef union tag_LD_TRANSFER_START_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t ld_diff_threshold          :  6;   /* LD_DIFF_THRESHOLD
                                                        The LD_DIFF_THRESHOLD is used to set in which condition the nINT_LD output signal will be issued to the host.  Whenever the LD_DIFF_POINTER is equal to or smaller than the LD_DIFF_THRESHOLD, then the nINT_LD signal will be issued.  This feature helps to check how much LD data remains in the internal LD buffer for monitoring overruns or underruns of the LD data.
                                                    */
        uint16_t                            :  1;   /* reserved */
        uint16_t int_ld_sign                :  1;   /* INT_LD_SIGN, Don’t set this bit. */
        uint16_t ld_trans_start_pointer     :  6;   /* LD_TRANS_START_POINTER
                                                        The LD_TRANS_START_POINTER determines when the transmit from the internal LD buffers to the XD-12 drivers will begin.
                                                        If the value is set to 8, then the transmit will begin as soon as the number of times the data through the SPI interface becomes 8.  Once the transmit is stated, the transaction will be continued until the number of transmit reaches to the BLOCK_SIZE value of the BLOCK SIZE registers.
                                                    */
        uint16_t                            :  2;   /* reserved */
    };
}_v_ld_transfer_start_pointer_t;

/* LOCAL-WR TRANSFER POINTER : Read-only */
typedef union tag_LOCAL_WR_TRANSFER_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_trans_pointer     :  6;   /* LOCAL_WR_TRANS_POINTER
                                                        This register value represents the number of times the data to be transferred to the XD-12 drivers are written to the LOCAL RW DATA registers through the SPI interface. The number increased one by one whenever the data are written to the LOCAL RW DATA registers.
                                                    */
        uint16_t                            :  2;   /* reserved */
        uint16_t local_wr_out_pointer       :  6;   /* LOCAL_WR_OUT_POINTER
                                                        This register value represents the number of times the data from the LOCAL RW DATA registers are read-out to write the data to the XD-12 drivers’ registers. The number increases one by one whenever the data are read-out from the LOCAL RW DATA registers.
                                                    */
        uint16_t                            :  2;   /* reserved */
    };
}_v_local_wr_transfer_pointer_t;

/* LOCAL-RD RECEIVE POINTER : Read-only */
typedef union tag_LOCAL_RD_RECEIVE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_rd_rec_pointer       :  6;   /* LOCAL_RD_REC_POINTER
                                                        This register value represents the number of times the data to be transferred through the SPI interface are written from the XD-12 drivers to the LOCAL RW DATA registers.
                                                        The number increased one by one whenever the data are written to the LOCAL RW DATA registers
                                                    */
        uint16_t                            :  2;   /* reserved */
        uint16_t local_rd_out_pointer       :  6;   /* LOCAL_RD_OUT_POINTER
                                                        This register value represents the number of times the data transferred from the XD-12 drivers are read out from the LOCAL RW DATA registers for transferring the data through the SPI interface.  The number increases one by one whenever the data are read-out from the LOCAL RW DATA registers.
                                                    */
        uint16_t                            :  2;   /* reserved */
    };
}_v_local_rd_receive_pointer_t;

/* LOCAL-RW DIFFERENCE POINTER : Read-only */
typedef union tag_LOCAL_RW_DIFFERENCE_POINTER
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_diff_pointer      :  6;   /* LOCAL_WR_DIFF_POINTER
                                                        This value is useful to monitor how much the data to be transferred to the XD-12 remains in the LOCAL RW DATA registers for checking overrun or underrun.
                                                        = LOCAL_WR_TRANS_POINTER – LOCAL_WR_OUT_POINTER
                                                    */
        uint16_t                            :  2;   /* reserved */
        uint16_t local_rd_diff_pointer      :  6;   /* LOCAL_RD_DIFF_POINTER
                                                        This value is useful to monitor how much the data from the XD-12 drivers remains in the LOCAL RW DATA registers for checking overrun or underrun.
                                                        = LOCAL_RD_REC_POINTER – LOCAL_RD_OUT_POINTER
                                                    */
        uint16_t                            :  2;   /* reserved */
    };
}_v_local_rw_difference_pointer_t;

/* LOCAL-RW POINTER RESET : default 0x00 */
typedef union tag_LOCAL_RW_POINTER_RESET
{
    uint16_t ALL;
    struct
    {
        uint16_t local_wr_pointer_rst       :  1;   /* LOCAL_WR_POINTER_RST
                                                        ‘0’ No operation
                                                        ‘1’ The LOCAL_WR_OUT_POINTER and the LOCAL_WR_TRANS_POINTER arereset. This bit is auto-cleared.
                                                    */
        uint16_t                            :  7;   /* reserved */
        uint16_t local_rd_pointer_rst       :  1;   /* LOCAL_RD_POINTER_RST
                                                        ‘0’ No operation
                                                        ‘1’ The LOCAL_RD_OUT_POINTER and the LOCAL_RD_REC_POINTER are reset. This bit is auto-cleared.
                                                    */
        uint16_t                            :  7;   /* reserved */
    };
}_v_local_rw_pointer_reset_t;

/* FAULT-AUTO-READ TIMER : Read-only */
typedef union tag_FAULT_AUTO_READ_TIMER
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_auto_rd_timer     : 16;   /* FAULT_AUTO_RD_TIMER : This register determines the execution interval for the fault auto-read commands. Counted in MCLK units. */
    };
}_v_fault_auto_read_timer_t;

/* FAULT-AUTO-READ EVENT : default 0x00 */
typedef union tag_FAULT_AUTO_READ_EVENT
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_auto_rd_interval     :  1;   /* FAULT_AUTO_RD_INTERVAL
                                                        The last bit of the FAULT_AUTO_RD_TIMER [16:0]
                                                    */
        uint16_t                            :  7;   /* reserved */
        uint16_t fault_auto_rd_event        :  8;   /* FAULT_AUTO_RD_EVENT
                                                        This register determines the execution times for the fault auto-read commands.
                                                        The value is reset at VSYNC rising edge.
                                                    */
    };
}_v_fault_auto_read_event_t;

/* SERIALIZER CLOCK GEN : default 0x0408 */
typedef union tag_SERIALIZER_CLOCK_GEN
{
    uint16_t ALL;
    struct
    {
        uint16_t sck_high   :  7;   /* SCK_HIGH
                                        This register determines the high pulse duration in case the serial data-out drives the high
                                        state.  Counted in MCLK units.
                                    */
        uint16_t            :  1;   /* reserved */
        uint16_t sck_low    :  7;   /* SCK_LOW
                                        This register determines the high pulse duration in case the serial data-out drives the low
                                        state.  Counted in MCLK units.
                                    */
        uint16_t            :  1;   /* reserved */
    };
}_v_serializer_clock_gen_t;

/* INTERRUPT ENABLE : default 0x00 */
typedef union tag_INTERRUPT_ENABLE
{
    uint16_t ALL;
    struct
    {
        uint16_t int_fb_en                  :  1;   /* INT_FB_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the fb condition happens among the XD-12 drivers during dimming operation.
                                                    */
        uint16_t int_open_en                :  1;   /* INT_OPEN_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the open condition happens among the XD-12 drivers during dimming operation.
                                                    */
        uint16_t int_short_en               :  1;   /* INT_SHORT_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the short condition happens among the XD-12 drivers during dimming operation.
                                                    */
        uint16_t int_thermal_en             :  1;   /* INT_THERMAL_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the thermal regulation happens among the XD-12 drivers during dimming operation.
                                                    */
        uint16_t int_ld_en                  :  1;   /* INT_LD_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the LD_DIFF_POINTER is equal to or smaller than the LD_DIFF_THRESHOLD during the execution of the LD transfer to the XD-12.
                                                    */
        uint16_t int_rd_rec_fail_en         :  1;   /* INT_RD_REC_FAIL_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that any violation happens during the execution for the local read-back from the XD-12.
                                                    */
        uint16_t int_fault_auto_rd_fail_en  :  1;   /* INT_FAULT_AUTO_RD_FAIL_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that any violation happens during the execution for the fault auto-read-back from the XD-12.
                                                    */
        uint16_t int_fault_rd_fail_en       :  1;   /* INT_FAULT_RD_FAIL_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that any violation happens during the execution for the fault read-back from the XD-12.
                                                    */
        uint16_t int_timeout_err_en         :  1;   /* INT_TIMEOUT_ERR_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the timer will be expired during the execution for read-backcommands. The expired time is mclk*8,192.
                                                    */
        uint16_t int_parity_err_en		    :  1;   /* INT_PARITY_ERR_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the parity error happens during the execution for read-back commands.
                                                    */
        uint16_t int_acc_cnt_err_en		    :  1;   /* INT_ACC_CNT_ERR_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the access count error happens during the execution for read-back commands.
                                                    */
        uint16_t                            :  1;   /* reserved */
        uint16_t int_uv15_en			    :  1;   /* INT_UV15_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the UV condition happens among the XD-12 drivers during dimming operation.
                                                    */
        uint16_t int_ov15_en			    :  1;   /* INT_OV15_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the OV condition happens among the XD-12 drivers during dimming operation.
                                                    */
        uint16_t int_uv50_en			    :  1;   /* INT_UV50_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the UV condition happens among the XD-12 drivers during dimming operation.
                                                    */
        uint16_t int_ov50_en			    :  1;   /* INT_OV50_EN
                                                        ‘0’ No operation
                                                        ‘1’ This bit enables to issue the interrupt through the nINT_FAULT pin in case that the OV condition happens among the XD-12 drivers during dimming operation.
                                                    */
    };
}_v_interrupt_enable_t;

/* COMMAND STATUS 1 : Read-only */
typedef union tag_COMMAND_STATUS1
{
    uint16_t ALL;
    struct
    {
        uint16_t sync_doing         :  1;   /* SYNC_DOING : This bit is set while the sync-gen command is working until it is complete. */
        uint16_t sync_done          :  1;   /* SYNC_DONE : This bit is set after the execution for the sync-gen command is completed. */
        uint16_t sync_auto_doing    :  1;   /* SYNC_AUTO_DOING : This bit is set while the sync auto-gen command is working until it is complete. */
        uint16_t sync_auto_done     :  1;   /* SYNC_AUTO_DONE : This bit is set after the execution for the sync auto-gen command is completed. */
        uint16_t fault_auto_doing   :  1;   /* FAULT_AUTO_DOING : This bit is set while the fault auto-reading command is working until it is complete. */
        uint16_t fault_auto_done    :  1;   /* FAULT_AUTO_DONE : This bit is set after the execution for the fault auto-reading command is completed. */
        uint16_t fault_doing        :  1;   /* FAULT_DOING : This bit is set while the fault-reading command is working until it is complete. */
        uint16_t fault_done         :  1;   /* FAULT_DONE : This bit is set after the execution for the fault-reading command is completed. */
        uint16_t ld_trans_doing     :  1;   /* LD_TRANS_DOING : This bit is set while the LD transferring command is working until it is complete. */
        uint16_t ld_trans_done      :  1;   /* LD_TRANS_DONE : This bit is set after the execution for the LD transferring command is completed. */
        uint16_t global_wr_doing    :  1;   /* GLOBAL_WR_DOING : This bit is set while the global-writing command is working until it is complete. */
        uint16_t global_wr_done     :  1;   /* GLOBAL_WR_DONE : This bit is set after the execution for the global-writing command is completed. */
        uint16_t local_wr_doing     :  1;   /* LOCAL_WR_DOING : This bit is set while the local-writing command is working until it is complete. */
        uint16_t local_wr_done      :  1;   /* LOCAL_WR_DONE : This bit is set after the execution for the local-writing command is completed. */
        uint16_t local_rd_doing     :  1;   /* LOCAL_RD_DOING : This bit is set while the local-reading command is working until it is complete. */
        uint16_t local_rd_done      :  1;   /* LOCAL_RD_DONE : This bit is set after the execution for the local-reading command is completed. */
    };
}_v_command_status1_t;

/* COMMAND STATUS 2 : Read-only */
typedef union tag_COMMAND_STATUS2
{
    uint16_t ALL;
    struct
    {
        uint16_t id_gen_doing       :  1;   /* ID_GEN_DOING : This bit is set while the id-gen command is working until it is complete. */
        uint16_t id_gen_done        :  1;   /* ID_GEN_DONE : This bit is set after the execution for the id-gen command is completed. */
        uint16_t                    : 14;   /* reserved */
    };
}_v_command_status2_t;

/* RECEIVE STATUS : Read-only */
typedef union tag_RECEIVE_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t local_rec_doing        :  1;   /* LOCAL_REC_DOING : This bit is set while the local read-receiving job is working until it is complete. */
        uint16_t local_rec_done         :  1;   /* LOCAL_REC_DONE : This bit is set after the local read-receiving job is completed. */
        uint16_t fault_auto_rec_doing   :  1;   /* FAULT_AUTO_REC_DOING : This bit is set while the fault auto-receiving job is working until it is complete. */
        uint16_t fault_auto_rec_done    :  1;   /* FAULT_AUTO_REC_DONE : This bit is set after the fault auto-receiving job is completed. */
        uint16_t fault_rec_doing        :  1;   /* FAULT_REC_DOING : This bit is set while the fault-receiving job is working until it is complete. */
        uint16_t fault_rec_done         :  1;   /* FAULT_REC_DONE : This bit is set after the fault-receiving job is completed. */
        uint16_t                        :  2;   /* reserved */
        uint16_t timeout_err            :  1;   /* TIMEOUT_ERR : This bit is set if the timer will be expired while the execution of read-back due to no response from the XD devices. */
        uint16_t                        :  3;   /* reserved */
        uint16_t local_rec_fail         :  1;   /* LOCAL_REC_FAIL : This bit is set if any violation happens while the local read-receiving job is working. */
        uint16_t fault_auto_rec_fail    :  1;   /* FAULT_AUTO_REC_FAIL : This bit is set if any violation happens while the fault auto-receiving job is working. */
        uint16_t fault_rec_fail         :  1;   /* FAULT_REC_FAIL : This bit is set if any violation happens while the fault-receiving job is working. */
        uint16_t                        :  1;   /* reserved */
    };
}_v_receive_status_t;

/* INTERRUPT STATUS : Read-only */
typedef union tag_INTERRUPT_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t int_fault                      :  1;   /* INT_FAULT */
        uint16_t int_ld                         :  1;   /* INT_LD */
        uint16_t                                :  2;   /* reserved */
        uint16_t int_fb_src                     :  1;   /* INT_FB_SRC */
        uint16_t int_open_src                   :  1;   /* INT_OPEN_SRC */
        uint16_t int_short_src                  :  1;   /* INT_SHORT_SRC */
        uint16_t int_tml_src                    :  1;   /* INT_TML_SRC */
        uint16_t int_ld_trans_src               :  1;   /* INT_LD_TRANS_SRC */
        uint16_t int_rd_rec_fail_src            :  1;   /* INT_RD_REC_FAIL_SRC */
        uint16_t int_fault_auto_rec_fail_src    :  1;   /* INT_FAULT_AUTO_REC_FAIL_SRC */
        uint16_t int_fault_rec_fail_src         :  1;   /* INT_FAULT_REC_FAIL_SRC */
        uint16_t int_timeout_err_src            :  1;   /* INT_TIMEOUT_ERR_SRC */
        uint16_t int_parity_err_src             :  1;   /* INT_PARITY_ERR_SRC */
        uint16_t int_acc_cnt_err_src            :  1;   /* INT_ACC_CNT_ERR_SRC */
        uint16_t int_ovuv_src                   :  1;   /* INT_OVUV_SRC */
    };
}_v_interrupt_status_t;

typedef union tag_SPI_FAULT_STATUS_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t parity_err_det_en   : 1;
        uint16_t acc_cnt_err_det_en  : 1;
        uint16_t parity_err_mode	 : 1;   /* 0: parity error mode is disabled. 1: parity error mode is enabled. */
        uint16_t                     : 1;   /* reserved */
        uint16_t parity_err    		 : 1;   /* 0: no parity error. 1: parity error. */
        uint16_t acc_cnt_err		 : 1;   /* 0: no access count error. 1: access count error. */
        uint16_t                     : 2;   /* reserved */
        uint16_t acc_counter		 : 8;   /* Access counter. This register is used to check the access count error. */
    };
}_v_spi_fault_status_control_t;

typedef union tag_CLK_CONTROL_1
{
    uint16_t ALL;
    struct
    {
        uint16_t mclk_mode              : 1;
        uint16_t serializer_skew_en     : 1;
        uint16_t osc_spread_en          : 1;   /* 0: spread spectrum disabled. 1: spread spectrum enabled. */
        uint16_t serializer_clk_sel	    : 1;   /* 0: serializer clock is generated by XC-24. 1: serializer clock is generated by XD-12. */
        uint16_t spread_dir		        : 2;   /* 0: down spread. 1: up spread. */
        uint16_t spread_speed	        : 2;   /* 0: 0.5%. 1: 1.0%. 2: 1.5%. 3: 2.0%. */
        uint16_t spread_range	        : 2;   /* 0: 0.5%. 1: 1.0%. 2: 1.5%. 3: 2.0%. */
        uint16_t                        : 1;   /* reserved */
        uint16_t b_rd_clk_sel	        : 1;   /* 0: b_rd_clk is generated by XC-24. 1: b_rd_clk is generated by XD-12. */
        uint16_t spread_range_a         : 3;   /* 0: 0.5%. 1: 1.0%. 2: 1.5%. 3: 2.0%. */
        uint16_t                        : 1;   /* reserved */
    };
}_v_clk_control_1_t;

typedef union tag_CLK_CONTROL_2
{
    uint16_t ALL;
    struct
    {
        uint16_t                      : 8; /* reserved */
        uint16_t osc_force_static     : 7;
        uint16_t                      : 1; /* reserved */
    };
}_v_clk_control_2_t;

typedef union tag_VDD_LDO_STATUS
{
    uint16_t ALL;
    struct
    {
        uint16_t uv15_det     :  1;
        uint16_t              :  3;
        uint16_t ov15_det     :  1;
        uint16_t              :  3;
        uint16_t uv50_det     :  1;
        uint16_t              :  3;
        uint16_t ov50_det     :  1;
        uint16_t              :  3;
    };
}_v_vdd_ldo_status_t;


typedef union tag_VDD_LDO_FAULT_LEVEL
{
    uint16_t ALL;
    struct
    {
        uint16_t uv15_level   :  3;
        uint16_t              :  1;
        uint16_t ov15_level   :  3;
        uint16_t              :  1;
        uint16_t uv50_level   :  3;
        uint16_t              :  1;
        uint16_t ov50_level   :  3;
        uint16_t              :  1;
    };
}_v_vdd_ldo_fault_level_t;

typedef union tag_COMMAND_LATENCY
{
    uint16_t ALL;
    struct
    {
        uint16_t cmd_latency     : 8;
        uint16_t serial_latency  : 8;
    };
}_v_command_latency_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE1
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_1 :  5;
        uint16_t daisied_dev_ch_size_2 :  5;
        uint16_t daisied_dev_ch_size_3 :  5;
        uint16_t                       :  1;
    };
}_v_daisied_device_channel_size1_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE2
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_4 :  5;
        uint16_t daisied_dev_ch_size_5 :  5;
        uint16_t daisied_dev_ch_size_6 :  5;
        uint16_t                       :  1;
    };
}_v_daisied_device_channel_size2_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE3
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_7 :  5;
        uint16_t daisied_dev_ch_size_8 :  5;
        uint16_t daisied_dev_ch_size_9 :  5;
        uint16_t                       :  1;
    };
}_v_daisied_device_channel_size3_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE4
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_10 :  5;
        uint16_t daisied_dev_ch_size_11 :  5;
        uint16_t daisied_dev_ch_size_12 :  5;
        uint16_t                        :  1;
    };
}_v_daisied_device_channel_size4_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE5
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_13 :  5;
        uint16_t daisied_dev_ch_size_14 :  5;
        uint16_t daisied_dev_ch_size_15 :  5;
        uint16_t                        :  1;
    };
}_v_daisied_device_channel_size5_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE6
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_16 :  5;
        uint16_t daisied_dev_ch_size_17 :  5;
        uint16_t daisied_dev_ch_size_18 :  5;
        uint16_t                        :  1;
    };
}_v_daisied_device_channel_size6_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE7
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_19 :  5;
        uint16_t daisied_dev_ch_size_20 :  5;
        uint16_t daisied_dev_ch_size_21 :  5;
        uint16_t                        :  1;
    };
}_v_daisied_device_channel_size7_t;

typedef union tag_DAISIED_DEVICE_CHANNEL_SIZE8
{
    uint16_t ALL;
    struct
    {
        uint16_t daisied_dev_ch_size_22 :  5;
        uint16_t daisied_dev_ch_size_23 :  5;
        uint16_t daisied_dev_ch_size_24 :  5;
        uint16_t                        :  1;
    };
}_v_daisied_device_channel_size8_t;

/* DAISY SIZE 1 : default 0x00 */
typedef union tag_DAISY_SIZE1
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch1     :  5;
        uint16_t daisy_size_ch2     :  5;
        uint16_t daisy_size_ch3     :  5;
        uint16_t                    :  1;
    };
}_v_daisy_size1_t;

/* DAISY SIZE 2 : default 0x00 */
typedef union tag_DAISY_SIZE2
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch4     :  5;
        uint16_t daisy_size_ch5     :  5;
        uint16_t daisy_size_ch6     :  5;
        uint16_t                    :  1;
    };
}_v_daisy_size2_t;

/* DAISY SIZE 3 : default 0x00 */
typedef union tag_DAISY_SIZE3
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch7     :  5;
        uint16_t daisy_size_ch8     :  5;
        uint16_t daisy_size_ch9     :  5;
        uint16_t                    :  1;
    };
}_v_daisy_size3_t;

/* DAISY SIZE 4 : default 0x00 */
typedef union tag_DAISY_SIZE4
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch10    :  5;   /* DAISY_SIZE_CH10 */
        uint16_t daisy_size_ch11    :  5;   /* DAISY_SIZE_CH11 */
        uint16_t daisy_size_ch12    :  5;   /* DAISY_SIZE_CH12 */
        uint16_t                    :  1;   /* reserved */
    };
}_v_daisy_size4_t;

/* DAISY SIZE 5 : default 0x00 */
typedef union tag_DAISY_SIZE5
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch13    :  5;   /* DAISY_SIZE_CH13 */
        uint16_t daisy_size_ch14    :  5;   /* DAISY_SIZE_CH14 */
        uint16_t daisy_size_ch15    :  5;   /* DAISY_SIZE_CH15 */
        uint16_t                    :  1;   /* reserved */
    };
}_v_daisy_size5_t;

/* DAISY SIZE 6 : default 0x00 */
typedef union tag_DAISY_SIZE6
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch16    :  5;   /* DAISY_SIZE_CH16 */
        uint16_t daisy_size_ch17    :  5;   /* DAISY_SIZE_CH17 */
        uint16_t daisy_size_ch18    :  5;   /* DAISY_SIZE_CH18 */
        uint16_t                    :  1;   /* reserved */
    };
}_v_daisy_size6_t;

/* DAISY SIZE 7 : default 0x00 */
typedef union tag_DAISY_SIZE7
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch19    :  5;   /* DAISY_SIZE_CH19 */
        uint16_t daisy_size_ch20    :  5;   /* DAISY_SIZE_CH20 */
        uint16_t daisy_size_ch21    :  5;   /* DAISY_SIZE_CH21 */
        uint16_t                    :  1;   /* reserved */
    };
}_v_daisy_size7_t;

/* DAISY SIZE 8 : default 0x00 */
typedef union tag_DAISY_SIZE8
{
    uint16_t ALL;
    struct
    {
        uint16_t daisy_size_ch22    :  5;   /* DAISY_SIZE_CH22 */
        uint16_t daisy_size_ch23    :  5;   /* DAISY_SIZE_CH23 */
        uint16_t daisy_size_ch24    :  5;   /* DAISY_SIZE_CH24 */
        uint16_t                    :  1;   /* reserved */
    };
}_v_daisy_size8_t;

/* BLOCK SIZE 1 : default 0x00 */
typedef union tag_BLOCK_SIZE1
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch1     :  8;   /* BLOCK_SIZE_CH1 : This register should be set with a valid number for correct operating. The number means how many block zones are daisied in the corresponding channel. The maximum 255 block zones per channel can be daisied. */
        uint16_t block_size_ch2     :  8;   /* BLOCK_SIZE_CH2 */
    };
}_v_block_size1_t;

/* BLOCK SIZE 2 : default 0x00 */
typedef union tag_BLOCK_SIZE2
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch3     :  8;   /* BLOCK_SIZE_CH3 */
        uint16_t block_size_ch4     :  8;   /* BLOCK_SIZE_CH4 */
    };
}_v_block_size2_t;

/* BLOCK SIZE 3 : default 0x00 */
typedef union tag_BLOCK_SIZE3
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch5     :  8;   /* BLOCK_SIZE_CH5 */
        uint16_t block_size_ch6     :  8;   /* BLOCK_SIZE_CH6 */
    };
}_v_block_size3_t;

/* BLOCK SIZE 4 : default 0x00 */
typedef union tag_BLOCK_SIZE4
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch7     :  8;   /* BLOCK_SIZE_CH7 */
        uint16_t block_size_ch8     :  8;   /* BLOCK_SIZE_CH8 */
    };
}_v_block_size4_t;

/* BLOCK SIZE 5 : default 0x00 */
typedef union tag_BLOCK_SIZE5
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch9     :  8;   /* BLOCK_SIZE_CH9 */
        uint16_t block_size_ch10    :  8;   /* BLOCK_SIZE_CH10 */
    };
}_v_block_size5_t;

/* BLOCK SIZE 6 : default 0x00 */
typedef union tag_BLOCK_SIZE6
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch11    :  8;   /* BLOCK_SIZE_CH11 */
        uint16_t block_size_ch12    :  8;   /* BLOCK_SIZE_CH12 */
    };
}_v_block_size6_t;

/* BLOCK SIZE 7 : default 0x00 */
typedef union tag_BLOCK_SIZE7
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch13    :  8;   /* BLOCK_SIZE_CH13 */
        uint16_t block_size_ch14    :  8;   /* BLOCK_SIZE_CH14 */
    };
}_v_block_size7_t;

/* BLOCK SIZE 8 : default 0x00 */
typedef union tag_BLOCK_SIZE8
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch15    :  8;   /* BLOCK_SIZE_CH15 */
        uint16_t block_size_ch16    :  8;   /* BLOCK_SIZE_CH16 */
    };
}_v_block_size8_t;

/* BLOCK SIZE 9 : default 0x00 */
typedef union tag_BLOCK_SIZE9
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch17    :  8;   /* BLOCK_SIZE_CH17 */
        uint16_t block_size_ch18    :  8;   /* BLOCK_SIZE_CH18 */
    };
}_v_block_size9_t;

/* BLOCK SIZE 10 : default 0x00 */
typedef union tag_BLOCK_SIZE10
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch19    :  8;   /* BLOCK_SIZE_CH19 */
        uint16_t block_size_ch20    :  8;   /* BLOCK_SIZE_CH20 */
    };
}_v_block_size10_t;

/* BLOCK SIZE 11 : default 0x00 */
typedef union tag_BLOCK_SIZE11
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch21    :  8;   /* BLOCK_SIZE_CH21 */
        uint16_t block_size_ch22    :  8;   /* BLOCK_SIZE_CH22 */
    };
}_v_block_size11_t;

/* BLOCK SIZE 12 : default 0x00 */
typedef union tag_BLOCK_SIZE12
{
    uint16_t ALL;
    struct
    {
        uint16_t block_size_ch23    :  8;   /* BLOCK_SIZE_CH23 */
        uint16_t block_size_ch24    :  8;   /* BLOCK_SIZE_CH24 */
    };
}_v_block_size12_t;

/* CHANNEL ENABLE 1 : default 0x00 */
typedef union tag_CHANNEL_ENABLE1
{
    uint16_t ALL;
    struct
    {
        uint16_t ch1_en     :  1;   /* CH1_EN : This bit should be set if each corresponding channel is to be activated for the local dimming operation.  If any channel is not used, then don’t set its bit. */
        uint16_t ch2_en     :  1;   /* CH2_EN */
        uint16_t ch3_en     :  1;   /* CH3_EN */
        uint16_t ch4_en     :  1;   /* CH4_EN */
        uint16_t ch5_en     :  1;   /* CH5_EN */
        uint16_t ch6_en     :  1;   /* CH6_EN */
        uint16_t ch7_en     :  1;   /* CH7_EN */
        uint16_t ch8_en     :  1;   /* CH8_EN */
        uint16_t ch9_en     :  1;   /* CH9_EN */
        uint16_t ch10_en    :  1;   /* CH10_EN */
        uint16_t ch11_en    :  1;   /* CH11_EN */
        uint16_t ch12_en    :  1;   /* CH12_EN */
        uint16_t ch13_en    :  1;   /* CH13_EN */
        uint16_t ch14_en    :  1;   /* CH14_EN */
        uint16_t ch15_en    :  1;   /* CH15_EN */
        uint16_t ch16_en    :  1;   /* CH16_EN */
    };
}_v_channel_enable1_t;

/* CHANNEL ENABLE 2/ SIZE/ LD WIDTH : default 0x00 */
typedef union tag_CHANNEL_ENABLE2
{
    uint16_t ALL;
    struct
    {
        uint16_t ch17_en    :  1;   /* CH17_EN */
        uint16_t ch18_en    :  1;   /* CH18_EN */
        uint16_t ch19_en    :  1;   /* CH19_EN */
        uint16_t ch20_en    :  1;   /* CH20_EN */
        uint16_t ch21_en    :  1;   /* CH21_EN */
        uint16_t ch22_en    :  1;   /* CH22_EN */
        uint16_t ch23_en    :  1;   /* CH23_EN */
        uint16_t ch24_en    :  1;   /* CH24_EN */
        uint16_t ld_size    :  5;   /* LD_SIZE : These bits represent how many channels are activated. */
        uint16_t            :  1;   /* reserved */
        uint16_t ld_width   :  2;   /* LD_WIDTH : These bits represent the number of bits contained in one LD data in one block zone.
                                                    2’b00 : 24 bits/block even if the 24 bits/block are transferred in SPI interface, the valid bits are PAM 10 bits and PWM 10 bits.
                                                    2’b01 : 24 bits/block, PAM 10 bits + PWM 11 bits.
                                                    2’b10 : 24 bits/block, PAM 10 bits + PWM 12 bits.
                                                    2’b11 : 16 bits/block
                                    */
    };
}_v_channel_enable2_t;

typedef union tag_DAC_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_auto         : 1;   /* GLOBAL_WR_DATA : This register represents the data to be written to the XD-12 drivers daisied. This register should be written with desired value before the global-writing command is executed. */
        uint16_t                  : 3;   /* reserved */
        uint16_t dac_sync_mode    : 1;
        uint16_t dac_fb_mode      : 1;
        uint16_t                  : 2;   /* reserved */
        uint16_t dac_dec1_mode    : 1;
        uint16_t dac_hold_en      : 1;
        uint16_t                  : 2;   /* reserved */
        uint16_t dac_thermal_off  : 1;
        uint16_t                  : 3;   /* reserved */
    };
}_v_dac_control_t;

typedef union tag_CURRENT_TARGET_DAC
{
    uint16_t ALL;
    struct
    {
        uint16_t curr_tgt_dac       : 12;
        uint16_t                    :  4;   /* reserved */
    };
}_v_current_target_dac_t;

typedef union tag_PREVIOUS_TARGET_DAC
{
    uint16_t ALL;
    struct
    {
        uint16_t prev_tgt_dac       : 12;
        uint16_t                    :  4;   /* reserved */
    };
}_v_previous_target_dac_t;

typedef union tag_DAC_OUT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_out           : 12;
        uint16_t                   :  4;   /* reserved */
    };
}_v_dac_out_t;

typedef union tag_DAC_INCREMENT1
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc1            :  5;
        uint16_t dac_inc2            :  5;
        uint16_t dac_inc3            :  6;
    };
}_v_dac_increment1_t;

typedef union tag_DAC_INCREMENT2_HOLD_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc4              : 7;
        uint16_t dac_inc_hold_limit    : 9;
    };
}_v_dac_increment2_hold_limit_t;

typedef union tag_DAC_DECREMENT_INC_WAIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc1_wait       : 7;
        uint16_t                     : 1;   /* reserved */
        uint16_t dac_dec1            : 7;
        uint16_t                     : 1;   /* reserved */
    };
}_v_dac_decrement_inc_wait_t;

typedef union tag_DAC_INCREMENT_HOLD_TH
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc_hold_threshold                : 9;
        uint16_t dac_increment_hold_th_signed          : 1;
        uint16_t                                       : 6;   /* reserved */
    };
}_v_dac_increment_hold_th_t;

typedef union tag_SOA_N11_N1
{
    uint16_t ALL;
    struct
    {
        uint16_t soa_n1      : 7;
        uint16_t soa_n11     : 9;
    };
}_v_soa_n11_n1_t;

typedef union tag_SOA_P2_P1
{
    uint16_t ALL;
    struct
    {
        uint16_t soa_p1 				  : 9;
        uint16_t                          : 3;   /* reserved */
        uint16_t soa_p2                   : 4;
    };
}_v_soa_p2_p1_t;

typedef union tag_SOA_P3_P2
{
    uint16_t ALL;
    struct
    {
        uint16_t soa_p2          :  6;
        uint16_t soa_p3          : 10;
    };
}_v_soa_p3_p2_t;

typedef union tag_DAC_FB_VALID_TIMER
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_fb_valid_timer   : 16;
    };
}_v_dac_fb_valid_timer_t;

typedef union tag_DAC_MIN_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_min_limit     : 12;
        uint16_t                   :  4;   /* reserved */
    };
}_v_dac_min_limit_t;

typedef union tag_DAC_MAX_LIMIT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_max_limit     : 12;
        uint16_t                   :  4;   /* reserved */
    };
}_v_dac_max_limit_t;

typedef union tag_DAC_STATE
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_st_en            : 4;
        uint16_t dac_state            : 4;
        uint16_t                      : 8;   /* reserved */
    };
}_v_dac_state_t;

typedef union tag_VALID_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t valid_cnt   : 16;
    };
}_v_valid_cnt_t;

typedef union tag_DAC_INC_HOLD_WAIT_CNT
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_inc1_wait_cnt        : 7;
        uint16_t                          : 1;   /* reserved */
        uint16_t dac_inc_hold_limit_cnt   : 8;
    };
}_v_dac_inc_hold_wait_cnt_t;

typedef union tag_R2
{
    uint16_t ALL;
    struct
    {
        uint16_t r2                     : 12;
        uint16_t r2_signed              :  1;
        uint16_t                        :  3;   /* reserved */
    };
}_v_r2_t;

/* GLOBAL-WRITE DATA : default 0x00 */
typedef union tag_GLOBAL_WRITE_DATA
{
    uint16_t ALL;
    struct
    {
        uint16_t global_wr_data : 12;   /* GLOBAL_WR_DATA : This register represents the data to be written to the XD-12 drivers daisied. This register should be written with desired value before the global-writing command is executed. */
        uint16_t                :  4;   /* reserved */
    };
}_v_global_write_v_t;

/* GLOBAL FAULT-READ DATA 1 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA1
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch1  :  4;   /* FAULT_CH1 :  */
        uint16_t fault_ch2  :  4;   /* FAULT_CH2 :  */
        uint16_t fault_ch3  :  4;   /* FAULT_CH3 :  */
        uint16_t fault_ch4  :  4;   /* FAULT_CH4 :  */
    };
}_v_global_fault_read_data1_t;

/* GLOBAL FAULT-READ DATA 2 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA2
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch5  :  4;   /* FAULT_CH5 :  */
        uint16_t fault_ch6  :  4;   /* FAULT_CH6 :  */
        uint16_t fault_ch7  :  4;   /* FAULT_CH7 :  */
        uint16_t fault_ch8  :  4;   /* FAULT_CH8 :  */
    };
}_v_global_fault_read_data2_t;

/* GLOBAL FAULT-READ DATA 3 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA3
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch9  :  4;   /* FAULT_CH9 :  */
        uint16_t fault_ch10 :  4;   /* FAULT_CH10 :  */
        uint16_t fault_ch11 :  4;   /* FAULT_CH11 :  */
        uint16_t fault_ch12 :  4;   /* FAULT_CH12 :  */
    };
}_v_global_fault_read_data3_t;

/* GLOBAL FAULT-READ DATA 4 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA4
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch13 :  4;   /* FAULT_CH13 :  */
        uint16_t fault_ch14 :  4;   /* FAULT_CH14 :  */
        uint16_t fault_ch15 :  4;   /* FAULT_CH15 :  */
        uint16_t fault_ch16 :  4;   /* FAULT_CH16 :  */
    };
}_v_global_fault_read_data4_t;

/* GLOBAL FAULT-READ DATA 5 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA5
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch17 :  4;   /* FAULT_CH17 :  */
        uint16_t fault_ch18 :  4;   /* FAULT_CH18 :  */
        uint16_t fault_ch19 :  4;   /* FAULT_CH19 :  */
        uint16_t fault_ch20 :  4;   /* FAULT_CH20 :  */
    };
}_v_global_fault_read_data5_t;

/* GLOBAL FAULT-READ DATA 6 : default 0x00 */
typedef union tag_GLOBAL_FAULT_READ_DATA6
{
    uint16_t ALL;
    struct
    {
        uint16_t fault_ch21 :  4;   /* FAULT_CH21 :  */
        uint16_t fault_ch22 :  4;   /* FAULT_CH22 :  */
        uint16_t fault_ch23 :  4;   /* FAULT_CH23 :  */
        uint16_t fault_ch24 :  4;   /* FAULT_CH24 :  */
    };
}_v_global_fault_read_data6_t;

/* PORTx LOCAL-RW DATA y : default 0x00 */
typedef union tag_PORT_LOCAL_RW_DATA
{
    uint16_t ALL;
    struct
    {
        uint16_t port_local_rw_data : 12;   /* PORTx_LOCAL_RW_DATA y :  */
        uint16_t                    :  4;   /* FAULT_CH24 :  */
    };
}_v_port_local_rw_data_t;

typedef union tag_TEST_CONTROL
{
    uint16_t ALL;
    struct
    {
        uint16_t mclk32_o           :  1;
        uint16_t                    :  3;
        uint16_t mclk1_o            :  1;
        uint16_t                    :  3;
        uint16_t daco_direct        :  1;
        uint16_t                    :  6;
        uint16_t test_en            :  1;
    };
}_v_test_control_t;

typedef union tag_OTP_PG_ACCESS
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_acc_cycle       :  16;
    };
}_v_otp_pg_access_t;

typedef union tag_OTP_WRITE
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_wsel       :  4;
        uint16_t                : 12;
    };
}_v_otp_write_t;

typedef union tag_OTP_RD_PROG
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_pg_s       :  1;
        uint16_t otp_rd_s       :  1;
        uint16_t                : 14;
    };
}_v_otp_rd_prog_t;

typedef union tag_OTP_PROTECT
{
    uint16_t ALL;
    struct
    {
        uint16_t protect       : 12;
        uint16_t               :  4;
    };
}_v_otp_protect_t;

typedef union tag_MIRROR1
{
    uint16_t ALL;
    struct
    {
        uint16_t vctl_ldo       :  4;
        uint16_t                :  4;
        uint16_t osc_fctl       :  7;
        uint16_t                :  1;
    };
}_v_mirror1_t;

typedef union tag_MIRROR2
{
    uint16_t ALL;
    struct
    {
        uint16_t dac_ofs        :  8;
        uint16_t dac_gain       :  6;
        uint16_t                :  2;
    };
}_v_mirror2_t;

typedef union tag_MIRROR3
{
    uint16_t ALL;
    struct
    {
        uint16_t otp_checksum   :  6;
        uint16_t                : 10;
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
    uint16_t ALL[XC24_ADDR_TRIM_MAX - XC24_ADDR_TRIM_START];
    struct
    {
        _v_test_control_t               _rF0;
        _v_otp_pg_access_t 		   		_rF1;
        _v_otp_write_t 		   			_rF2;
        _v_otp_rd_prog_t 		   		_rF3;
        _v_otp_protect_t 		   		_rF4;
        _v_mirror1_t 		   			_rF5;
        _v_mirror2_t 		   			_rF6;
        _v_mirror3_t 		   			_rF7;
    };
}_xc24_mirror_regs_t;

/* BEGIN - INTERFACE FUNCTIONS */
extern void XC24_Write_Register(uint16_t in_addr, uint16_t in_data);
extern uint16_t XC24_Read_Register(uint8_t in_addr);
extern void XC24_Read_Register_All(void);
extern void XC24_Dump_All_Register(void);
extern void XC24_Init(void);
extern void XC24_Trim_Init(void);

extern void XC24_Set_OTP_Protect(bool en);

extern void XC24_Start_MCLK_Oscillation(bool en);
extern bool IS_XC24_Support(void);
extern void USE_XC24(bool b_support);

extern void XC24_IF_IdGen_Command(void);
extern void XC24_IF_SyncGen_Command(void);
extern uint16_t XC24_IF_Fault_Read_Command(void);

extern void XC24_IF_Write_XDIC(uint8_t in_XDIC_addr, uint16_t in_XDIC_data);
extern uint16_t XC24_IF_Read_XDIC(uint8_t in_XDIC_addr);
extern void XC24_IF_Write_LD(uint16_t in_LD_data);

/* END   - INTERFACE FUNCTIONS */
#ifdef __cplusplus
}
#endif

#endif /* ~__XC24_H__ */

/*** end of file ***/


