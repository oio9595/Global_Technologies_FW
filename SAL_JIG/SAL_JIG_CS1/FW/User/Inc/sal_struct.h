/** @file sal_struct.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */

#ifndef _SAL_REGS_STRUCT_H_
#define _SAL_REGS_STRUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef union
{
    uint16_t ALL;
    struct
    {
        uint16_t dummy : 16;
    };
}_sal_dummy_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t sio_1    : 2;  /* Communication mode of SIO1. 2'b00 : LVDS, 2'b01 : EOL, 2'b10 : MCU, 2'b11 : CAN, Value is set automatically at startup and after reset. */
        uint16_t sio_2    : 2;  /* Communication mode of SIO2. 2'b00 : LVDS, 2'b01 : EOL, 2'b10 : MCU, 2'b11 : CAN, Value is set automatically at startup and after reset. */
        uint16_t ic_state : 3;  /* Represent current IC state. 3'b000 : POR. (default), 3'b001 : Uninitialized, 3'b010 : Sleep, 3'b011 : Active, 3'b100 : Deepsleep */
        uint16_t          : 9;
    };
}_sal_status1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t com_flt : 1;
        uint16_t crc_flt : 1;
        uint16_t uv_flt  : 1;
        uint16_t ot_flt  : 1;
        uint16_t short_b : 1;
        uint16_t short_g : 1;
        uint16_t short_r : 1;
        uint16_t open_b  : 1;
        uint16_t open_g  : 1;
        uint16_t open_r  : 1;
        uint16_t t_out   : 1;
        uint16_t otpcrc  : 1;
        uint16_t         : 4;
    };
}_sal_status2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp : 10; /* Temperature data */
        uint16_t      :  6;
    };
}_sal_temp_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t vext_tm : 10;  /* EXT_TM pin voltage. */
        uint16_t         :  6;
    };
}_sal_vext_tm_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t f_pwm_div  : 4;
        uint16_t lg_e       : 1;
        uint16_t ph_shift_e : 1;
        uint16_t crc_e      : 1;
        uint16_t com_flt_e  : 1;
        uint16_t uv_flt_e   : 1;
        uint16_t ot_flt_e   : 1;
        uint16_t os_flt_e   : 1;
        uint16_t tc_e       : 1;
        uint16_t            : 4;
    };
}_sal_setup1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t sh_lvl     : 2;    /* LED Short detection level. 2'b00 : VS_TH = 3.75V, 2'b01 : VS_TH = 4.00V, 2'b10 : VS_TH = 4.25V (default), 2'b11 : VS_TH = 4.50V */
        uint16_t uv_lvl     : 2;    /* UVLO detection level. 2'b00 : VUV_TH = 3.6V, 2'b01 : VUV_TH = 3.8V, 2'b10 : VUV_TH = 4.0V, 2'b11 : VUV_TH = 4.2V (default) */
        uint16_t event_cyc  : 1;    /* This bit determines a period to detect fault events or ADC clock. 0: 31.25khz (typical 32us), 1: 62.5khz (typical 16us) */
        uint16_t vext_mon_e : 1;    /* 0: EXT_TM pin voltage monitoring disable. 1: EXT_TM pin voltage monitoring enable. */
        uint16_t clk_inv_e  : 1;    /* CLK polarity for MCU mode 0: Inverted disabled. 1: Inverted enabled. */
        uint16_t            : 9;
    };
}_sal_setup2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_threshold : 10;   /* Overtemperature fault detection threshold. The OT_FLT is raised automatically the temperature increases above this value. */
        uint16_t                :  6;
    };
}_sal_tempth_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_hysterisis : 10;  /* Overtemperature fault release threshold. The OT_FLT can be cleared only when the temperature is below this value. */
        uint16_t                 :  6;
    };
}_sal_temphys_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t pwm_red_val : 12;  /* 12bit RED PWM calculated data value */
        uint16_t             :  4;
    };
}_sal_cal_pwm_red_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t pwm_green_val : 12;    /* 12bit GREEN calculated data value */
        uint16_t               :  4;
    };
}_sal_cal_pwm_green_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t pwm_blue_val : 12; /* 12bit BLUE PWM calculated data value */
        uint16_t              :  4;
    };
}_sal_cal_pwm_blue_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t b_curr_max_lvl : 4;    /* This determines the max current value of the output sink driver. */
        uint16_t g_curr_max_lvl : 4;    /* 4'b0000:  1.44mA, 4'b0001:  2.87mA, 4'b0010:  4.31mA, 4'b0011:  5.75mA, 4'b0100:  7.18mA, 4'b0101:  8.62mA, 4'b0110: 10.05mA, 4'b0111: 10.48mA,*/
        uint16_t r_curr_max_lvl : 4;    /* 4'b1000: 12.92mA, 4'b1001: 14.35mA, 4'b1010: 15.78mA, 4'b1011: 17.21mA, 4'b1100: 18.65mA, 4'b1101: 20.08mA, 4'b1110: 21.51mA, 4'b1111: 22.94mA */
        uint16_t                : 4;
    };
}_sal_curr_max_lvl_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc1 : 10; /* when the TEMPERATURE is from TC_BASE to TC_OFFSET, TEMP_LUT_TC1[9:0] is applied during TC equation */
        uint16_t              :  6;
    };
}_sal_temp_lut_tc1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc2 : 10;
        uint16_t              :  6;
    };
}_sal_temp_lut_tc2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc3 : 10;
        uint16_t              :  6;
    };
}_sal_temp_lut_tc3_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc4 : 10;
        uint16_t              :  6;
    };
}_sal_temp_lut_tc4_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc5 : 10;
        uint16_t              :  6;
    };
}_sal_temp_lut_tc5_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc6 : 10;
        uint16_t              :  6;
    };
}_sal_temp_lut_tc6_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc7 : 10;
        uint16_t              :  6;
    };
}_sal_temp_lut_tc7_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc8 : 10;
        uint16_t              :  6;
    };
}_sal_temp_lut_tc8_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc9 : 10;
        uint16_t              :  6;
    };
}_sal_temp_lut_tc9_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t temp_lut_tc10 : 10;
        uint16_t               :  6;
    };
}_sal_temp_lut_tc10_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t timeout : 12;  /* ref clk = mclk * 2^10 */
        uint16_t         :  4;
    };
}_sal_timeout_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t mcast_addr_msb :  4;   /* Multicast group address. In case that the device address indicates the multicast group address. */
        uint16_t                : 12;
    };
}_sal_mcast1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t mcast_addr_lsb : 12;
        uint16_t                :  4;
    };
}_sal_mcast2_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t g_data_msb : 4;    /* This data is converted into 12-bit PWM data toward a sink driver. PWM data = (R/G/B_DATA x PWM_MAX_R/G/B + 128) >> 8 */
        uint16_t r_data     : 8;
        uint16_t            : 4;
    };
}_sal_rgb1_t;

typedef union
{
    uint16_t val;
    struct
    {
        uint16_t b_data     : 8;
        uint16_t g_data_lsb : 4;
        uint16_t            : 4;
    };
}_sal_rgb2_t;

typedef enum _sal_regs_addr_
{
    SAL_ADDR_STATUS1        = 0x01,
    SAL_ADDR_STATUS2,
    SAL_ADDR_TEMP,
    SAL_ADDR_VEXT_TM        = 0x05,
    SAL_ADDR_SETUP1,
    SAL_ADDR_SETUP2,
    SAL_ADDR_MCAST1,
    SAL_ADDR_MCAST2,
    SAL_ADDR_TEMPTH,
    SAL_ADDR_TEMPHYS,
    SAL_ADDR_CAL_PWM_RED,
    SAL_ADDR_CAL_PWM_GREEN,
    SAL_ADDR_CAL_PWM_BLUE,
    SAL_ADDR_CURR_MAX_LVL,
    SAL_ADDR_TEMP_LUT_TC1,
    SAL_ADDR_TEMP_LUT_TC2,
    SAL_ADDR_TEMP_LUT_TC3,
    SAL_ADDR_TEMP_LUT_TC4,
    SAL_ADDR_TEMP_LUT_TC5,
    SAL_ADDR_TEMP_LUT_TC6,
    SAL_ADDR_TEMP_LUT_TC7,
    SAL_ADDR_TEMP_LUT_TC8,
    SAL_ADDR_TEMP_LUT_TC9,
    SAL_ADDR_TEMP_LUT_TC10,
    SAL_ADDR_TIMEOUT,
    SAL_ADDR_RGB1           = 0x20,
    SAL_ADDR_RGB2,

    SAL_ADDR_MAX,
}sal_regs_addr_t;

typedef union _sal_regs
{
    uint16_t ALL[SAL_ADDR_MAX];
    struct
    {
        _sal_dummy_t            _r00;
        _sal_status1_t          _r01;
        _sal_status2_t          _r02;
        _sal_temp_t             _r03;
        _sal_dummy_t            _r04;
        _sal_vext_tm_t          _r05;
        _sal_setup1_t           _r06;
        _sal_setup2_t           _r07;
        _sal_mcast1_t           _r08;
        _sal_mcast2_t           _r09;
        _sal_tempth_t           _r0A;
        _sal_temphys_t          _r0B;
        _sal_cal_pwm_red_t      _r0C;
        _sal_cal_pwm_green_t    _r0D;
        _sal_cal_pwm_blue_t     _r0E;
        _sal_curr_max_lvl_t     _r0F;
        _sal_temp_lut_tc1_t     _r10;
        _sal_temp_lut_tc2_t     _r11;
        _sal_temp_lut_tc3_t     _r12;
        _sal_temp_lut_tc4_t     _r13;
        _sal_temp_lut_tc5_t     _r14;
        _sal_temp_lut_tc6_t     _r15;
        _sal_temp_lut_tc7_t     _r16;
        _sal_temp_lut_tc8_t     _r17;
        _sal_temp_lut_tc9_t     _r18;
        _sal_temp_lut_tc10_t    _r19;
        _sal_timeout_t          _r1A;
        _sal_dummy_t            _r1B;
        _sal_dummy_t            _r1C;
        _sal_dummy_t            _r1D;
        _sal_dummy_t            _r1E;
        _sal_dummy_t            _r1F;
        _sal_rgb1_t             _r20;
        _sal_rgb2_t             _r21;
    };
}_sal_regs_t;

typedef union
{
    uint8_t buffer[8];
    uint64_t val;
    struct
    {
        uint64_t crc      :  8;
        uint64_t command  :  8;
        uint64_t dev_addr : 12;
        uint64_t preamble :  4;
        uint64_t          : 32;
    }bit0;
    struct
    {
        uint64_t crc      :  8;
        uint64_t dummy    :  4;
        uint64_t data     : 12;
        uint64_t command  :  8;
        uint64_t dev_addr : 12;
        uint64_t preamble :  4;
        uint64_t          : 16;
    }bit12;
    struct
    {
        uint64_t crc      :  8;
        uint64_t data     : 24;
        uint64_t command  :  8;
        uint64_t dev_addr : 12;
        uint64_t preamble :  4;
        uint64_t          :  8;
    }bit24;
}_data_frame_format_t; // 64-bit size

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t mcast  : 16;
        uint32_t        : 16;
    };
}_sal_cmd_mcast_t;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t r  : 8;
        uint32_t g  : 8;
        uint32_t b  : 8;
        uint32_t    : 8;
    };
}_sal_cmd_rgb_t;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t r  : 8;
        uint32_t g  : 8;
        uint32_t b  : 8;
        uint32_t    : 8;
    };
}_sal_cmd_tempst_t;


#define CMD_SAL_RESET               0xB1
#define CMD_SAL_INITBIDIR           0xB2
#define CMD_SAL_CLRERROR            0xB3
#define CMD_SAL_GOSLEEP             0xB4
#define CMD_SAL_GOACTIVE            0xB8
#define CMD_SAL_GODEEPSLEEP         0xBC
#define CMD_SAL_SET_SETUP1          0x86
#define CMD_SAL_SET_SETUP2          0x87
#define CMD_SAL_SET_MCAST           0x88
#define CMD_SAL_SET_RGB             0xA0
#define CMD_SAL_SET_TEMPTH          0x8A
#define CMD_SAL_SET_TEMPHYS         0x8B
#define CMD_SAL_SET_CURR_MAX_LVL    0x8F
#define CMD_SAL_SET_TEMP_LUT_TC1    0x90
#define CMD_SAL_SET_TEMP_LUT_TC2    0x91
#define CMD_SAL_SET_TEMP_LUT_TC3    0x92
#define CMD_SAL_SET_TEMP_LUT_TC4    0x93
#define CMD_SAL_SET_TEMP_LUT_TC5    0x94
#define CMD_SAL_SET_TEMP_LUT_TC6    0x95
#define CMD_SAL_SET_TEMP_LUT_TC7    0x96
#define CMD_SAL_SET_TEMP_LUT_TC8    0x97
#define CMD_SAL_SET_TEMP_LUT_TC9    0x98
#define CMD_SAL_SET_TEMP_LUT_TC10   0x99
#define CMD_SAL_SET_TIMEOUT         0x9A

#define CMD_SAL_READ_STATUS1        0x41
#define CMD_SAL_READ_STATUS2        0x42
#define CMD_SAL_READ_TEMP           0x43
#define CMD_SAL_READ_TEMPST         0x44
#define CMD_SAL_READ_VEXT_TM        0x45
#define CMD_SAL_READ_SETUP1         0x46
#define CMD_SAL_READ_SETUP2         0x47
#define CMD_SAL_READ_MCAST          0x48
#define CMD_SAL_READ_RGB            0x60
#define CMD_SAL_READ_TEMPTH         0x4A
#define CMD_SAL_READ_TEMPHYS        0x4B
#define CMD_SAL_READ_PWM_RED_VAL    0x4C
#define CMD_SAL_READ_PWM_GREEN_VAL  0x4D
#define CMD_SAL_READ_PWM_BLUE_VAL   0x4E
#define CMD_SAL_READ_CURR_MAX_LVL   0x4F
#define CMD_SAL_READ_TEMP_LUT_TC1   0x50
#define CMD_SAL_READ_TEMP_LUT_TC2   0x51
#define CMD_SAL_READ_TEMP_LUT_TC3   0x52
#define CMD_SAL_READ_TEMP_LUT_TC4   0x53
#define CMD_SAL_READ_TEMP_LUT_TC5   0x54
#define CMD_SAL_READ_TEMP_LUT_TC6   0x55
#define CMD_SAL_READ_TEMP_LUT_TC7   0x56
#define CMD_SAL_READ_TEMP_LUT_TC8   0x57
#define CMD_SAL_READ_TEMP_LUT_TC9   0x58
#define CMD_SAL_READ_TEMP_LUT_TC10  0x59
#define CMD_SAL_READ_TIMEOUT        0x5A

typedef enum
{
    SAL_MAX_CURR_1_mA_44 = 0,   //0x00
    SAL_MAX_CURR_2_mA_87,       //0x01
    SAL_MAX_CURR_4_mA_31,       //0x02
    SAL_MAX_CURR_5_mA_75,       //0x03
    SAL_MAX_CURR_7_mA_18,       //0x04
    SAL_MAX_CURR_8_mA_62,       //0x05
    SAL_MAX_CURR_10_mA_05,      //0x06
    SAL_MAX_CURR_11_mA_48,      //0x07
    SAL_MAX_CURR_12_mA_92,      //0x08
    SAL_MAX_CURR_14_mA_35,      //0x09
    SAL_MAX_CURR_15_mA_78,      //0x0A
    SAL_MAX_CURR_17_mA_21,      //0x0B
    SAL_MAX_CURR_18_mA_65,      //0x0C
    SAL_MAX_CURR_20_mA_08,      //0x0D
    SAL_MAX_CURR_21_mA_51,      //0x0E
    SAL_MAX_CURR_22_mA_94,      //0x0F
}sal_max_curr_t;

#define SAL_R_MAX_CURR      SAL_MAX_CURR_21_mA_51
#define SAL_G_MAX_CURR      SAL_MAX_CURR_14_mA_35
#define SAL_B_MAX_CURR      SAL_MAX_CURR_5_mA_75

/* BEGIN - INTERFACE FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* ~_SAL_REGS_STRUCT_H_ */

/*** end of file ***/
