/** @file xd12.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __XD12_H__
#define __XD12_H__

#ifdef EXTERN
    #undef EXTERN
#endif
#ifdef __XD12_C__ 
    #define EXTERN
#else
    #define EXTERN extern
#endif
#ifdef __cplusplus
 extern "C" {
#endif


#define _DBG_XD12       1

/* XD12 Dealy Config **********************************/
#define XD12_DELAY_SHORT     1
#define XD12_DELAY_MID       3
#define XD12_DELAY_LONG      10
#define XD12_DELAY_LONGLONG  30
/********************************************************/

#define LD_FIX_MAX   255
#define LD_FIX_50_PERCENT_POINT   (uint16_t)((LD_FIX_MAX * 0.5) + 0.5)

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
    CH_MAX
};

#define REGISTER_LIMIT_OSC                  (255) //2^8 OSC[9:8] is fixed to 2
#define REGISTER_LIMIT_VREF                 ( 63) //2^6
#define REGISTER_LIMIT_GLB                  (  7) //2^3
#define REGISTER_LIMIT_OFS_1                (255) //2^8
#define REGISTER_LIMIT_OFS_2                (255) //2^8,
#define REGISTER_LIMIT_OFS_TEMP             ( 15) //2^4 OFS_TEMP [3:0] b11 ~ b08
#define REGISTER_LIMIT_DEV_MAX_CURR_LEVEL   (  7) //2^3
#define REGISTER_LIMIT_SHORT_LEVEL          (  7) //2^3
#define REGISTER_LIMIT_FB_LEVEL             (  3) //2^2 

typedef enum
{
    DEV_MAX_CURR_LEVEL_4mA = 0,     // 3b'000
    DEV_MAX_CURR_LEVEL_8mA,         // 3b'001
    DEV_MAX_CURR_LEVEL_16mA,        // 3b'010
    DEV_MAX_CURR_LEVEL_24mA,        // 3b'011
    DEV_MAX_CURR_LEVEL_32mA,        // 3b'100
    DEV_MAX_CURR_LEVEL_64mA,        // 3b'101
}dev_max_curr_level_t;

typedef enum
{
    SHORT_LEVEL_2V = 0,     // 3b'000
    SHORT_LEVEL_4V,         // 3b'001
    SHORT_LEVEL_7V,         // 3b'010
    SHORT_LEVEL_12V,        // 3b'011
    SHORT_LEVEL_21V,        // 3b'100
    SHORT_LEVEL_24V,        // 3b'101
    SHORT_LEVEL_28V,        // 3b'110
    SHORT_LEVEL_38V,        // 3b'111
}short_level_t;

typedef enum
{
    FB_LEVEL_0V6 = 0,       // 2b'00
    FB_LEVEL_0V8,           // 2b'01
    FB_LEVEL_1V0,           // 2b'10
    FB_LEVEL_1V2,           // 2b'11
}fb_level_t;

enum
{
    XD12_ADDR_RESET   = 0x00,
    XD12_ADDR_ID,
    XD12_ADDR_DELAY_SIZE = 0x03,
    XD12_ADDR_DELAY_PERIOD = 0x04,
    XD12_ADDR_LD_FIX1 = 0x06,
    XD12_ADDR_LD_FIX2 = 0x07,
    XD12_ADDR_MAX_CURR_VREF,
    XD12_ADDR_CHANNEL_ENABLE,
    XD12_ADDR_FAULT_STATUS,
    XD12_ADDR_FAULT_LEVEL,
    XD12_ADDR_DELAY_CH01,
    XD12_ADDR_DELAY_CH02,
    XD12_ADDR_DELAY_CH03,
    XD12_ADDR_DELAY_CH04,
    XD12_ADDR_DELAY_CH05,
    XD12_ADDR_DELAY_CH06,
    XD12_ADDR_DELAY_CH07,
    XD12_ADDR_DELAY_CH08,
    XD12_ADDR_DELAY_CH09,
    XD12_ADDR_DELAY_CH10,
    XD12_ADDR_DELAY_CH11,
    XD12_ADDR_DELAY_CH12,
    XD12_ADDR_SERIAL_CLOCK_GEN,
    XD12_ADDR_SHORT_ENABLE_LEVEL,
    XD12_ADDR_LD_CONTROL,
    //
    XD12_ADDR_TRIM1 = 0x20,  //OTP-Control
    XD12_ADDR_TRIM2,
    XD12_ADDR_OTP_WRITE,
    XD12_ADDR_OTP_RD_PROG,
    XD12_ADDR_OTP_PROTECT,
    XD12_ADDR_OTP_CRC,
    //
    XD12_ADDR_OSC = 0x26,//OTP-Start
    XD12_ADDR_VREF_CTL,
    XD12_ADDR_OFS1_CH01 = 0x28,
    XD12_ADDR_OFS1_CH02,
    XD12_ADDR_OFS1_CH03,
    XD12_ADDR_OFS1_CH04,
    XD12_ADDR_OFS1_CH05,
    XD12_ADDR_OFS1_CH06,
    XD12_ADDR_OFS1_CH07,
    XD12_ADDR_OFS1_CH08,
    XD12_ADDR_OFS1_CH09,
    XD12_ADDR_OFS1_CH10,
    XD12_ADDR_OFS1_CH11,
    XD12_ADDR_OFS1_CH12,
    XD12_ADDR_OFS2_CH01,
    XD12_ADDR_OFS2_CH02,
    XD12_ADDR_OFS2_CH03,
    XD12_ADDR_OFS2_CH04,
    XD12_ADDR_OFS2_CH05,
    XD12_ADDR_OFS2_CH06,
    XD12_ADDR_OFS2_CH07,
    XD12_ADDR_OFS2_CH08,
    XD12_ADDR_OFS2_CH09,
    XD12_ADDR_OFS2_CH10,
    XD12_ADDR_OFS2_CH11,
    XD12_ADDR_OFS2_CH12,  //0x3F
    XD12_ADDR_MAX
};

/* BEGIN - INTERFACE FUNCTIONS */

void TargetIC_IF_PowerOn_Init();
EXTERN void TargetIC_IF_Initialize(void);
EXTERN void TargetIC_IF_Init_Register(void);
EXTERN uint8_t TargetIC_IF_LD_FIX_Set(uint16_t in_val);
EXTERN uint8_t TargetIC_IF_FAULT_LEVEL_Set(dev_max_curr_level_t in_dev_max_curr, short_level_t in_short_level, fb_level_t in_fb_level);
EXTERN void TargetIC_IF_CHANNEL_ENABLE_ResetAll();
EXTERN void TargetIC_IF_CHANNEL_ENABLE_SetAll();
EXTERN uint8_t TargetIC_IF_CHANNEL_ENABLE_Set(uint8_t in_channel);
EXTERN void TargetIC_IF_Read_Resgister_All(uint8_t in_GUI_SUPPORT);
EXTERN void TargetIC_IF_Show_RegAll();
EXTERN uint16_t TargetIC_IF_Inc_Screen_CurValue(uint16_t in_u16_screen_input_cur);
EXTERN void TargetIC_IF_Write_OTP();
EXTERN void TargetIC_IF_Write_OTP_Start();
EXTERN void TargetIC_IF_Write_OTP_End();
EXTERN void TargetIC_IF_Write_REGISTER(uint8_t in_addr, uint16_t in_data);
EXTERN void TargetIC_IF_TRIM_Init_VREF();
EXTERN void TargetIC_IF_TRIM_Init_OSC();
EXTERN void TargetIC_IF_TRIM_Init_GLB();
EXTERN void TargetIC_IF_TRIM_Init_OFS_1();
EXTERN void TargetIC_IF_TRIM_Init_OFS_2();
EXTERN uint16_t TargetIC_IF_TrimRegister_Limit_Get(uint8_t ch_num, trim_mode_t in_trim_mode);
EXTERN uint16_t TargetIC_IF_TrimRegister_Get(uint8_t ch_num, trim_mode_t in_trim_mode);
EXTERN uint8_t TargetIC_IF_TrimRegister_Set(uint8_t ch_num, trim_mode_t in_trim_mode, uint16_t in_regi_val);
EXTERN void TargetIC_IF_SEND_otp_written();

EXTERN void JigBd_IF_Write_Command_through_PWM_to_XD12(uint8_t in_addr, uint16_t in_data);
EXTERN uint16_t TargetIC_IF_Read_Command_through_PWM_to_XD12(uint8_t in_addr);
EXTERN uint16_t XD12_Get_REGISTER(uint8_t in_addr);



/* END   - INTERFACE FUNCTIONS */

#ifdef DBG_TEST
EXTERN void _dbg_XD12_Test(void);
EXTERN void _dbg_XD12_Print_RegisterMap(void);
EXTERN void _dbg_XD12_Detect(void);  // JIG BD Init Test
#endif //DBG_TEST

    
EXTERN void TargetIC_IF_Fault_Level_Init();
EXTERN void TargetIC_IF_Serial_Clock_Init();
EXTERN void TargetIC_IF_Temp_Level_Init();
EXTERN void TargetIC_IF_Test_Enable();
EXTERN void TargetIC_IF_MAX_CURR_VREF_Init();
EXTERN void TargetIC_IF_SHORT_ENABLE_LEVEL_Init();
EXTERN void TargetIC_IF_SHORT_ENABLE_Set(uint16_t in_short_en_level);
EXTERN void TargetIC_IF_LD_CONTROL_INIT();

//XD12 Dimming Test
EXTERN uint8_t XD12_Write_DELAY_SIZE(uint16_t in_delay_size);
EXTERN uint8_t XD12_Write_DELAY_PERIOD(uint16_t in_delay_period);
EXTERN void TargetIC_IF_TRIM_REG_INIT();


#ifdef __cplusplus
}
#endif

#endif /* ~__XD12_H__ */

/*** end of file ***/


