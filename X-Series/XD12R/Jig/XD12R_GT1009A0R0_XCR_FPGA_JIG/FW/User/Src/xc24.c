/** @file xc24.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XC24R_C__
#include "main.h"
#include "xc24.h"
#include "JigBd_IF.h"
#include "types.h"
#include "config.h"

#define XC24R_GRP1_REG_ENTRY(addr, reg)     { addr, #addr, &gt_xc24r_group1_regs.reg }
#define XC24R_GRP2_REG_ENTRY(addr, reg)     { addr, #addr, &gt_xc24r_group2_regs.reg }

#define XC_SPI_TIMEOUT_MS               (10U)

#define XC24R_OTP_PROTECT_DISABLE        (0xA5AU)
#define XC24R_OTP_PROTECT_ENABLE         (0x5A5U)

#define XC24R_GLB_WR_DATA_LATCH_DELAY   (1U)

#define XC24R_GROUP1                    (0U)
#define XC24R_GROUP2                    (1U)

static bool gb_xc24r_support;

volatile uint8_t gn_xc_spi_timeout;

static xc24r_grp1_regs_t gt_xc24r_group1_regs;
static xc24r_grp2_regs_t gt_xc24r_group2_regs;

static _reg_map_t gt_xc24r_group1_maps[] =
{
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_RESET                        ,_r00 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_GLOBAL_WRITE                 ,_r01 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LOCAL_WRITE                  ,_r02 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LOCAL_READ                   ,_r03 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_ID_GEN                       ,_r04 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_FAULT_READ                   ,_r05 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LD_TRANSFER                  ,_r06 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_SYNC_GEN                     ,_r07 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_COMMAND_AUTO_ENABLE          ,_r08 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LD_WRITE_POINTER             ,_r09 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LD_READ_POINTER              ,_r0A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LD_DIFFERENCE_POINTER        ,_r0B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LD_START_POINTER_TH          ,_r0C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LOCAL_WRITE_TRANSFER_POINTER ,_r0D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LOCAL_READ_RECEIVE_POINTER   ,_r0E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LOCAL_RW_DIFFERENCE_POINTER  ,_r0F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LOCAL_RW_POINTER_RESET       ,_r10 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_FAULT_AUTO_READ_INTERVAL     ,_r11 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_FAULT_AUTO_READ_EVENT        ,_r12 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_INTERRUPT_ENABLE             ,_r13 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_COMMAND_STATUS_1             ,_r14 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_COMMAND_STATUS_2             ,_r15 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_RECEIVE_STATUS               ,_r16 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_INTERRUPT_STATUS             ,_r17 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CMD_PARITY_ERR_STATUS1       ,_r18 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CMD_PARITY_ERR_STATUS2       ,_r19 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_SPI_FAULT_STATUS_CONTROL     ,_r1A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CLK_CONTROL_1                ,_r1B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CLK_CONTROL_2                ,_r1C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_SERIALIZER_CLOCK_GEN         ,_r1D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_LATENCY                      ,_r1E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_TIMEOUT                      ,_r1F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISIED_DEVICE_CH_SIZE       ,_r20 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISY_SIZE_1                 ,_r21 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISY_SIZE_2                 ,_r22 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISY_SIZE_3                 ,_r23 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISY_SIZE_4                 ,_r24 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISY_SIZE_5                 ,_r25 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISY_SIZE_6                 ,_r26 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISY_SIZE_7                 ,_r27 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAISY_SIZE_8                 ,_r28 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_1                 ,_r29 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_2                 ,_r2A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_3                 ,_r2B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_4                 ,_r2C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_5                 ,_r2D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_6                 ,_r2E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_7                 ,_r2F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_8                 ,_r30 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_9                 ,_r31 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_10                ,_r32 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_11                ,_r33 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_BLOCK_SIZE_12                ,_r34 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CHANNEL_ENABLE_1             ,_r35 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CHANNEL_ENABLE_2             ,_r36 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_FLLCNT11                     ,_r37 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_FLLCNT12                     ,_r38 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_FLLCNT21                     ,_r39 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_FLLCNT22                     ,_r3A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_VO_DELAY                     ,_r3B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_VO_OFF_ON                    ,_r3C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_SVO_ON                       ,_r3D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_SVO1_OFF                     ,_r3E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_SVO2_OFF                     ,_r3F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_SVO3_OFF                     ,_r40 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_SVO_NUMBER                   ,_r41 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC_NF_CONTROL               ,_r42 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC_CONTROL                  ,_r43 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CURRENT_TARGET_DAC1          ,_r44 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CURRENT_TARGET_DAC2          ,_r45 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_CURRENT_TARGET_DAC3          ,_r46 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PREVIOUS_TARGET_DAC1         ,_r47 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PREVIOUS_TARGET_DAC2         ,_r48 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PREVIOUS_TARGET_DAC3         ,_r49 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_OUT                     ,_r4A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_OUT                     ,_r4B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_OUT                     ,_r4C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_STATE                   ,_r4D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_STATE                   ,_r4E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_STATE                   ,_r4F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_INCREMENT_1             ,_r50 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_INCREMENT_2_HOLD_LIMIT  ,_r51 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_DECREMENT_INC_WAIT      ,_r52 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_INCREMENT_HOLD_THRESHOLD,_r53 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_FB_VALID_TIMER          ,_r54 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_MIN_LIMIT               ,_r55 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC1_MAX_LIMIT               ,_r56 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_INCREMENT_1             ,_r57 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_INCREMENT_2_HOLD_LIMIT  ,_r58 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_DECREMENT_INC_WAIT      ,_r59 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_INCREMENT_HOLD_THRESHOLD,_r5A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_FB_VALID_TIMER          ,_r5B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_MIN_LIMIT               ,_r5C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC2_MAX_LIMIT               ,_r5D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_INCREMENT_1             ,_r5E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_INCREMENT_2_HOLD_LIMIT  ,_r5F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_DECREMENT_INC_WAIT      ,_r60 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_INCREMENT_HOLD_THRESHOLD,_r61 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_FB_VALID_TIMER          ,_r62 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_MIN_LIMIT               ,_r63 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_DAC3_MAX_LIMIT               ,_r64 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_OSC_FLL_MAN_A1               ,_r65 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_OSC_FLL_MAN_A2               ,_r66 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_OSC_FLL_MAN_B1               ,_r67 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_OSC_FLL_MAN_B2               ,_r68 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_GLOBAL_WRITE_DATA            ,_r69 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA1      ,_r6A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA2      ,_r6B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA3      ,_r6C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA4      ,_r6D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA5      ,_r6E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA6      ,_r6F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA1         ,_r70 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA1         ,_r71 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA1         ,_r72 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA1         ,_r73 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA1         ,_r74 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA1         ,_r75 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA1         ,_r76 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA1         ,_r77 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA2         ,_r78 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA2         ,_r79 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA2         ,_r7A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA2         ,_r7B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA2         ,_r7C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA2         ,_r7D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA2         ,_r7E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA2         ,_r7F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA3         ,_r80 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA3         ,_r81 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA3         ,_r82 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA3         ,_r83 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA3         ,_r84 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA3         ,_r85 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA3         ,_r86 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA3         ,_r87 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA4         ,_r88 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA4         ,_r89 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA4         ,_r8A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA4         ,_r8B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA4         ,_r8C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA4         ,_r8D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA4         ,_r8E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA4         ,_r8F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA5         ,_r90 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA5         ,_r91 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA5         ,_r92 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA5         ,_r93 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA5         ,_r94 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA5         ,_r95 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA5         ,_r96 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA5         ,_r97 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA6         ,_r98 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA6         ,_r99 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA6         ,_r9A ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA6         ,_r9B ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA6         ,_r9C ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA6         ,_r9D ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA6         ,_r9E ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA6         ,_r9F ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA7         ,_rA0 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA7         ,_rA1 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA7         ,_rA2 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA7         ,_rA3 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA7         ,_rA4 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA7         ,_rA5 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA7         ,_rA6 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA7         ,_rA7 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA8         ,_rA8 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA8         ,_rA9 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA8         ,_rAA ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA8         ,_rAB ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA8         ,_rAC ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA8         ,_rAD ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA8         ,_rAE ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA8         ,_rAF ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA9         ,_rB0 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA9         ,_rB1 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA9         ,_rB2 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA9         ,_rB3 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA9         ,_rB4 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA9         ,_rB5 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA9         ,_rB6 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA9         ,_rB7 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA10        ,_rB8 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA10        ,_rB9 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA10        ,_rBA ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA10        ,_rBB ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA10        ,_rBC ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA10        ,_rBD ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA10        ,_rBE ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA10        ,_rBF ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA11        ,_rC0 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA11        ,_rC1 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA11        ,_rC2 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA11        ,_rC3 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA11        ,_rC4 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA11        ,_rC5 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA11        ,_rC6 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA11        ,_rC7 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA12        ,_rC8 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA12        ,_rC9 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA12        ,_rCA ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA12        ,_rCB ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA12        ,_rCC ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA12        ,_rCD ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA12        ,_rCE ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA12        ,_rCF ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA13        ,_rD0 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA13        ,_rD1 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA13        ,_rD2 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA13        ,_rD3 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA13        ,_rD4 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA13        ,_rD5 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA13        ,_rD6 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA13        ,_rD7 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA14        ,_rD8 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA14        ,_rD9 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA14        ,_rDA ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA14        ,_rDB ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA14        ,_rDC ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA14        ,_rDD ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA14        ,_rDE ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA14        ,_rDF ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA15        ,_rE0 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA15        ,_rE1 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA15        ,_rE2 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA15        ,_rE3 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA15        ,_rE4 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA15        ,_rE5 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA15        ,_rE6 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA15        ,_rE7 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA16        ,_rE8 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT2_LOCAL_RW_DATA16        ,_rE9 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT3_LOCAL_RW_DATA16        ,_rEA ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT4_LOCAL_RW_DATA16        ,_rEB ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT5_LOCAL_RW_DATA16        ,_rEC ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT6_LOCAL_RW_DATA16        ,_rED ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT7_LOCAL_RW_DATA16        ,_rEE ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_PORT8_LOCAL_RW_DATA16        ,_rEF ),

    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_TEST_CONTROL          ,_rF0 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_PG_ACCESS         ,_rF1 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_WRITE             ,_rF2 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_RD_PROG           ,_rF3 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_PROTECT           ,_rF4 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR1           ,_rF5 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR2           ,_rF6 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR3           ,_rF7 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR4           ,_rF8 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_OTP_MIRROR5           ,_rF9 ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_GATE_CONTROL          ,_rFA ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_GATE1_OFFSET          ,_rFB ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_GATE2_OFFSET          ,_rFC ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_GATE3_OFFSET          ,_rFD ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_SV_VAR_CONTROL1       ,_rFE ),
    XC24R_GRP1_REG_ENTRY( XC24R_ADDR_GRP1_MIRROR_SV_VAR_CONTROL2       ,_rFF ),
};
_Static_assert(XC24R_ADDR_GRP1_MAX == (sizeof(gt_xc24r_group1_maps) / sizeof(_reg_map_t)), "XC24R GROUP1 Address map mismatch!");

static _reg_map_t gt_xc24r_group2_maps[] =
{
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_DAC1_FB_VALID_CNT            ,_r00 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_DAC1_INC_HOLD_WAIT_CNT       ,_r01 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_1R2                          ,_r02 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA1_N1_N11                  ,_r03 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA1_P2_P1                   ,_r04 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA1_P3_P2                   ,_r05 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_DAC2_FB_VALID_CNT            ,_r06 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_DAC2_INC_HOLD_WAIT_CNT       ,_r07 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_2R2                          ,_r08 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA2_N1_N11                  ,_r09 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA2_P2_P1                   ,_r0A ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA2_P3_P2                   ,_r0B ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_DAC3_FB_VALID_CNT            ,_r0C ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_DAC3_INC_HOLD_WAIT_CNT       ,_r0D ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_3R2                          ,_r0E ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA3_N1_N11                  ,_r0F ),

    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA3_P2_P1                   ,_r10 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_SOA3_P3_P2                   ,_r11 ),
    XC24R_GRP2_REG_ENTRY( XC24R_ADDR_GRP2_ANA_TEST                     ,_r12 ),
};
_Static_assert(XC24R_ADDR_GRP2_MAX == (sizeof(gt_xc24r_group2_maps) / sizeof(_reg_map_t)), "XC24R GROUP2 Address map mismatch!");

static inline bool SPI_Timeout_Handler(void)
{
    if (gn_xc_spi_timeout == 0U)
    {
        print(LOG_ERROR, "SPI Timeout Error\r\n");
        return false;
    }
    return true;
}

static inline void SPI_Write(SPI_TypeDef *SPIx, uint16_t* p_buffer, uint16_t len)
{
    gn_xc_spi_timeout = XC_SPI_TIMEOUT_MS;
    XC_NSCS_LO();
    us_delay(1U);

    if (LL_SPI_IsEnabled(SPIx) != true)
    {
        LL_SPI_Enable(SPIx);
    }
    for (volatile uint16_t i = 0U ; i < len ; ++i)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx))
        {
            if (!SPI_Timeout_Handler()) return;
        }
        LL_SPI_TransmitData16(SPIx, p_buffer[i]);
    }

    while(LL_SPI_IsActiveFlag_BSY(SPIx))
    {
        if (!SPI_Timeout_Handler()) return;
    };

    us_delay(1U);
    XC_NSCS_HI();
}

static inline void SPI_Read(SPI_TypeDef *SPIx, uint16_t* p_tx_buffer, uint16_t* p_rx_buffer, uint16_t len)
{
    gn_xc_spi_timeout = XC_SPI_TIMEOUT_MS;
    XC_NSCS_LO();
    us_delay(1U);

    if (LL_SPI_IsEnabled(SPIx) != 1)
    {
        LL_SPI_Enable(SPIx);
    }

    while(SET == LL_SPI_IsActiveFlag_RXNE(SPIx))
    {
        volatile uint16_t temp = LL_SPI_ReceiveData16(SPIx);
    }

    for (volatile uint16_t i = 0 ; i < len ; i++)
    {
        while(RESET == LL_SPI_IsActiveFlag_TXE(SPIx))
        {
            if (!SPI_Timeout_Handler()) return;
        }
        LL_SPI_TransmitData16(SPIx, p_tx_buffer[i]);

        while(RESET == LL_SPI_IsActiveFlag_RXNE(SPIx))
        {
            if (!SPI_Timeout_Handler()) return;
        }
        p_rx_buffer[i] = LL_SPI_ReceiveData16(SPIx);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPIx))
    {
        if (!SPI_Timeout_Handler()) return;
    }

    us_delay(1U);
    XC_NSCS_HI();
}

static const _reg_map_t* XC24R_Get_Group1_Map_Pointer(xc24r_addr_grp1_t addr)
{
    for (uint16_t i = 0U ; i < sizeof(gt_xc24r_group1_maps) / sizeof(gt_xc24r_group1_maps[0]); ++i)
    {
        if (gt_xc24r_group1_maps[i].address == addr)
        {
            return &gt_xc24r_group1_maps[i];
        }
    }
    return NULL;
}

static const _reg_map_t* XC24R_Get_Group2_Map_Pointer(xc24r_addr_grp2_t addr)
{
    for (uint16_t i = 0U ; i < sizeof(gt_xc24r_group2_maps) / sizeof(gt_xc24r_group2_maps[0]); ++i)
    {
        if (gt_xc24r_group2_maps[i].address == addr)
        {
            return &gt_xc24r_group2_maps[i];
        }
    }
    return NULL;
}

void XC24R_Write_Group1_Register(xc24r_addr_grp1_t in_addr, uint16_t in_data)
{
    xc24r_cmd_t cmd_format = { 0 };
    uint16_t tx_buffer[2] = { 0 };

    cmd_format.bit.code = CMD_CODE_REG_WRITE;
    cmd_format.bit.addr = in_addr;
    cmd_format.bit.size = 1U;

    tx_buffer[0] = cmd_format.ALL;
    tx_buffer[1] = in_data;

    const _reg_map_t* xc24r_map = XC24R_Get_Group1_Map_Pointer(in_addr);
    if (xc24r_map)
    {
        *((uint16_t*)(xc24r_map->reg_ptr)) = in_data;
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, in_addr);
    }

    SPI_Write(SPI1, tx_buffer, 2U);
    us_delay(10U);
}

uint16_t XC24R_Read_Group1_Register(xc24r_addr_grp1_t in_addr)
{
    xc24r_cmd_t cmd_format = { 0 };
    uint16_t tx_buffer[2] = { 0 };
    uint16_t rx_buffer[2] = { 0 };

    cmd_format.bit.code = CMD_CODE_REG_READ;
    cmd_format.bit.addr = in_addr;
    cmd_format.bit.size = 1U;

    tx_buffer[0] = cmd_format.ALL;

    SPI_Read(SPI1, tx_buffer, rx_buffer, 2U);

    const _reg_map_t* xc24r_map = XC24R_Get_Group1_Map_Pointer(in_addr);
    if (xc24r_map)
    {
        *((uint16_t*)(xc24r_map->reg_ptr)) = rx_buffer[1];
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, in_addr);
    }

    us_delay(10U);
    return rx_buffer[1];
}

void XC24R_Write_Group2_Register(xc24r_addr_grp2_t in_addr, uint16_t in_data)
{
    xc24r_cmd_t cmd_format = { 0 };
    uint16_t tx_buffer[2] = { 0 };

    cmd_format.bit.code = CMD_CODE_REG_WRITE;
    cmd_format.bit.addr = (uint16_t)(in_addr);
    cmd_format.bit.size = 1U;

    tx_buffer[0] = cmd_format.ALL;
    tx_buffer[1] = in_data;

    const _reg_map_t* xc24r_map = XC24R_Get_Group2_Map_Pointer(in_addr);
    if (xc24r_map)
    {
        *((uint16_t*)(xc24r_map->reg_ptr)) = in_data;
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, in_addr);
    }

    SPI_Write(SPI1, tx_buffer, 2U);
}

uint16_t XC24R_Read_Group2_Register(xc24r_addr_grp2_t in_addr)
{
    xc24r_cmd_t cmd_format = { 0 };
    uint16_t tx_buffer[2] = { 0 };
    uint16_t rx_buffer[2] = { 0 };

    cmd_format.bit.code = CMD_CODE_REG_READ;
    cmd_format.bit.addr = in_addr;
    cmd_format.bit.size = 0U;

    tx_buffer[0] = cmd_format.ALL;

    SPI_Read(SPI1, tx_buffer, rx_buffer, 2U);

    const _reg_map_t* xc24r_map = XC24R_Get_Group2_Map_Pointer(in_addr);
    if (xc24r_map)
    {
        *((uint16_t*)(xc24r_map->reg_ptr)) = rx_buffer[1];
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Not Found !!\r\n", __func__, in_addr);
    }

    us_delay(10U);
    return rx_buffer[1];
}

void XC24R_Read_Register_All(void)
{
    for (xc24r_addr_grp1_t xcr_addr_grp1 = XC24R_ADDR_GRP1_RESET ; xcr_addr_grp1 < XC24R_ADDR_GRP1_MAX ; ++xcr_addr_grp1)
    {
        XC24R_Read_Group1_Register(xcr_addr_grp1);
    }

    for (xc24r_addr_grp2_t xcr_addr_grp2 = XC24R_ADDR_GRP2_DAC1_FB_VALID_CNT ; xcr_addr_grp2 < XC24R_ADDR_GRP2_MAX ; ++xcr_addr_grp2)
    {
        XC24R_Read_Group2_Register(xcr_addr_grp2);
    }

    XC24R_Dump_All_Register();
}

void XC24R_Dump_All_Register(void)
{
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
    for (xc24r_addr_grp1_t xcr_addr_grp1 = XC24R_ADDR_GRP1_RESET ; xcr_addr_grp1 < XC24R_ADDR_GRP1_MAX ; ++xcr_addr_grp1)
    {
        const _reg_map_t* map = XC24R_Get_Group1_Map_Pointer(xcr_addr_grp1);
        if (map)
        {
            print(LOG_INFO, "[ %-40s 0x%02X | 0x%04X | %-6u ]\r\n", map->name, map->address, *((uint16_t*)(map->reg_ptr)), *((uint16_t*)(map->reg_ptr)));
        }
    }
    for (xc24r_addr_grp2_t xcr_addr_grp2 = XC24R_ADDR_GRP2_DAC1_FB_VALID_CNT ; xcr_addr_grp2 < XC24R_ADDR_GRP2_MAX ; ++xcr_addr_grp2)
    {
        const _reg_map_t* map = XC24R_Get_Group2_Map_Pointer(xcr_addr_grp2);
        if (map)
        {
            print(LOG_INFO, "[%s (0x%02X)]\r\n\t VALUE : %s(0x%04X)%s\r\n\r\n", map->name, map->address, ANSI_FONT_MAGENTA, *((uint16_t*)(map->reg_ptr)), ANSI_FONT_NONE);
        }
    }
    print(LOG_INFO, "\r\n-------------------------------------------------------------------\r\n");
}

void XC24R_Init(void)
{
    XC_NSCS_HI();

#if (XC24R_MCLK_MODE == XC24R_MCLK_EXTERNAL)
    XC24R_Start_MCLK_Oscillation(true);
#endif

    print(LOG_DEBUG, " ...XC24 Initial Start...\r\n");

    for (xc24r_addr_grp1_t xc_addr = XC24R_ADDR_GRP1_RESET ; xc_addr < XC24R_ADDR_GRP1_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24R_Get_Group1_Map_Pointer(xc_addr);
        if (map)
        {
            switch (xc_addr)
            {
            case XC24R_ADDR_GRP1_RESET:
                gt_xc24r_group1_regs._r00.bit.rst1 = 1U;
                gt_xc24r_group1_regs._r00.bit.rst2 = 1U;
                gt_xc24r_group1_regs._r00.bit.rst3 = 1U;
                break;
            default :
                continue;
            }
            XC24R_Write_Group1_Register((xc24r_addr_grp1_t)(map->address), *((uint16_t*)(map->reg_ptr)));
        }
    }

    for (xc24r_addr_grp2_t xc_addr = XC24R_ADDR_GRP2_DAC1_FB_VALID_CNT ; xc_addr < XC24R_ADDR_GRP2_MAX ; ++xc_addr)
    {
        const _reg_map_t* map = XC24R_Get_Group2_Map_Pointer(xc_addr);
        if (map)
        {
            switch (xc_addr)
            {
            case XC24R_ADDR_GRP2_DAC1_FB_VALID_CNT:
                gt_xc24r_group2_regs._r00.bit.dac1_fb_valid_cnt = 0U;
                break;
            default :
                continue;
            }
            XC24R_Write_Group2_Register((xc24r_addr_grp2_t)(map->address), *((uint16_t*)(map->reg_ptr)));
        }
    }

    print(LOG_DEBUG, " ...XC24 Initial Done...\r\n");
    XC24R_Read_Register_All();
}

void XC24R_Start_MCLK_Oscillation(bool en)
{
    if (en)
    {
        LL_GPIO_InitTypeDef GPIO_InitStruct = {0, };

        GPIO_InitStruct.Pin = FPGA_MCLK_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
        LL_GPIO_Init(FPGA_MCLK_GPIO_Port, &GPIO_InitStruct);
    }
    else
    {
        LL_GPIO_InitTypeDef GPIO_InitStruct = {0, };

        GPIO_InitStruct.Pin = FPGA_MCLK_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        LL_GPIO_Init(FPGA_MCLK_GPIO_Port, &GPIO_InitStruct);
        LL_GPIO_ResetOutputPin(FPGA_MCLK_GPIO_Port, FPGA_MCLK_Pin);
    }
    LL_mDelay(10U);
}

void XC24R_Start_FLLSync_Oscillation(bool en)
{
    if (en)
    {
        LL_GPIO_InitTypeDef GPIO_InitStruct = {0, };

        GPIO_InitStruct.Pin = FPGA_FLLSYNC_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
        LL_GPIO_Init(FPGA_FLLSYNC_GPIO_Port, &GPIO_InitStruct);
    }
    else
    {
        LL_GPIO_InitTypeDef GPIO_InitStruct = {0, };

        GPIO_InitStruct.Pin = FPGA_FLLSYNC_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        LL_GPIO_Init(FPGA_FLLSYNC_GPIO_Port, &GPIO_InitStruct);
        LL_GPIO_ResetOutputPin(FPGA_FLLSYNC_GPIO_Port, FPGA_FLLSYNC_Pin);
    }
    LL_mDelay(10U);
}

/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/

bool IS_XC24R_Support(void)
{
    return gb_xc24r_support;
}

void USE_XC24R(bool b_support)
{
    gb_xc24r_support = b_support;
}

void XC24R_IF_IdGen_Command(void)
{
    gt_xc24r_group1_regs._r04.bit.enable = 1U;
    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_ID_GEN, gt_xc24r_group1_regs._r04.ALL);
}

void XC24R_IF_SyncGen_Command(void)
{
    gt_xc24r_group1_regs._r07.bit.enable = 1U;
    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_SYNC_GEN, gt_xc24r_group1_regs._r07.ALL);
}

uint16_t XC24R_IF_Fault_Read_Command(void)
{
    gt_xc24r_group1_regs._r05.bit.enable = 1U;
    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_FAULT_READ, gt_xc24r_group1_regs._r05.ALL);
    us_delay(XDIC_FAULT_READ_DELAY + XDIC_FAULT_RECV_DELAY);

    gt_xc24r_group1_regs._r6A.ALL = XC24R_Read_Group1_Register(XC24R_ADDR_GRP1_GLOBAL_FAULT_READ_DATA1);

    return gt_xc24r_group1_regs._r6A.ALL;
}

void XC24R_IF_Write_XDIC(uint8_t in_XDIC_addr, uint16_t in_XDIC_data)
{
    gt_xc24r_group1_regs._r01.bit.addr = in_XDIC_addr;
    gt_xc24r_group1_regs._r01.bit.enable = 1U;

    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_GLOBAL_WRITE_DATA, in_XDIC_data);
    us_delay(XC24R_GLB_WR_DATA_LATCH_DELAY);

    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_GLOBAL_WRITE, gt_xc24r_group1_regs._r01.ALL);
}

uint16_t XC24R_IF_Read_XDIC(uint8_t in_XDIC_addr)
{
    uint16_t u16_XDIC_data = 0U;

    gt_xc24r_group1_regs._r10.bit.local_rd_pointer_rst = 1U;
    gt_xc24r_group1_regs._r10.bit.local_wr_pointer_rst = 1U;
    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_LOCAL_RW_POINTER_RESET, gt_xc24r_group1_regs._r10.ALL);

    gt_xc24r_group1_regs._r03.bit.addr = in_XDIC_addr;
    gt_xc24r_group1_regs._r03.bit.ch_seg = 0U;
    gt_xc24r_group1_regs._r03.bit.enable = 1U;
    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_LOCAL_READ, gt_xc24r_group1_regs._r03.ALL);
    us_delay(XDIC_READ_DELAY + XDIC_READ_RECV_DELAY);

    u16_XDIC_data = XC24R_Read_Group1_Register(XC24R_ADDR_GRP1_PORT1_LOCAL_RW_DATA1);

    return u16_XDIC_data;
}

void XC24R_IF_Write_LD(uint16_t* p_in_LD_data)
{
    xc24r_cmd_t cmd_format = {0, };
    uint16_t tx_buffer[1 + XDIC_DAISY_SIZE * XDIC_CH_SIZE] = {0,};

    cmd_format.bit.code = CMD_CODE_LD_TRANS;
    cmd_format.bit.addr = 0U;
    cmd_format.bit.size = XDIC_DAISY_SIZE * XDIC_CH_SIZE;

    tx_buffer[0] = cmd_format.ALL;
    for (uint16_t i = 0U ; i < (XDIC_DAISY_SIZE * XDIC_CH_SIZE) ; ++i)
    {
        tx_buffer[i + 1] = p_in_LD_data[i];
    }

    SPI_Write(SPI1, tx_buffer, 1U + XDIC_DAISY_SIZE * XDIC_CH_SIZE);
}

void XC24R_Turn_Off_Sync_Auto(void)
{
    gt_xc24r_group1_regs._r08.bit.sync_auto_en = 0U;
    gt_xc24r_group1_regs._r08.bit.fault_auto_en = 0U;
    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_COMMAND_AUTO_ENABLE, gt_xc24r_group1_regs._r08.ALL);
}

void XC24R_Turn_On_Sync_Auto(void)
{
    gt_xc24r_group1_regs._r08.bit.sync_auto_en = 1U;
    gt_xc24r_group1_regs._r08.bit.fault_auto_en = 0U;
    XC24R_Write_Group1_Register(XC24R_ADDR_GRP1_COMMAND_AUTO_ENABLE, gt_xc24r_group1_regs._r08.ALL);
}

/* END - INTERFACE FUNCTIONS ************************************************************************/

/*** end of file ***/