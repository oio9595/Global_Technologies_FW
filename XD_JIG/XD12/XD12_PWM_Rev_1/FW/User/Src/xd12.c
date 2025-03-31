/** @file xd12.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XD12_C__
#include "config.h"

#define XD12_REG_GENERAL            (0)
#define XD12_REG_MIRROR             (1)

#define XD12_OTP_PROTECT_DISABLE     (0xA5A)
#define XD12_OTP_PROTECT_ENABLE      (0x5A5)

#define XD12_DEFAULT_OSC            (0x20)
#define XD12_DEFAULT_VREF_CTL       (0x20)
#define XD12_DEFAULT_ICTL_L         (0x40)
#define XD12_DEFAULT_ICTL_H         (0x40)

#define XD_MCLK                     (39319200.0f)
#define VSYNC                       (120.0f)

#define TIM8_FREQ                   (144000000.0f)

#define XD_SUB_FRAME_SIZE           (64)

#define XD_LD_DIR_HEAD_SHIFT        (0)
#define XD_LD_DIR_TAIL_SHIFT        (1)

#define XD_PWM_RES_12BIT            (0)
#define XD_PWM_RES_14BIT            (1)

#define XD_IO_MODE_NOP              (0)
#define XD_IO_MODE_EXT_VSYNC        (1)
#define XD_IO_MODE_FBO              (2) // not support
#define XD_IO_MODE_EXT_VYI_FBO      (3)

#define XD_MCLK_FLL_ENABLE          (0)
#define XD_MCLK_FLL_DISABLE         (1)

#define XD_MCLK_LOCK_CNT_120Hz      (327600)

#ifdef __XD12_C__

static const char* gs_xd12_general_regs_str[XD12_ADDR_MAX] =
{
    "RESET_ID",
    "LD_CONTROL",
    "FPWM_DIVIDER",
    "CHANNEL_ENABLE",
    "DUMMY",
    "FAULT_STATUS",
    "FAULT_LEVEL",
    "FAULT_CONTROL",
    "MAX_CURRENT_VREF",
    "DELAY_CH_EXTEND_1",
    "DELAY_CH_EXTEND_2",
    "DELAY_CH_1",
    "DELAY_CH_2",
    "DELAY_CH_3",
    "DELAY_CH_4",
    "DELAY_CH_5",
    "DELAY_CH_6",
    "DELAY_CH_7",
    "DELAY_CH_8",
    "DELAY_CH_9",
    "DELAY_CH_10",
    "DELAY_CH_11",
    "DELAY_CH_12",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "SERIAL_CLOCK_GEN",
    "SERIAL_LATENCY",
    "MCLK_LOCK_1",
    "MCLK_LOCK_2",
    "TEMP",
    "OSC_FLL_MANUAL_1",
    "OSC_FLL_MANUAL_2",
    "OSC_FLL_MONITOR",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "OTP_ACCESS_1",
    "OTP_ACCESS_2",
    "OTP_WRITE",
    "OTP_RD_PROG",
    "OTP_PROTECT",
    "OP_MODE",
};

static const char* gs_xd12_mirror_regs_str[XD12_ADDR_TRIM_MAX] =
{
    "OTP_CRC",
    "OSC",
    "VREF_CTL",
    "ICTL_L_CH1",
    "ICTL_L_CH2",
    "ICTL_L_CH3",
    "ICTL_L_CH4",
    "ICTL_L_CH5",
    "ICTL_L_CH6",
    "ICTL_L_CH7",
    "ICTL_L_CH8",
    "ICTL_L_CH9",
    "ICTL_L_CH10",
    "ICTL_L_CH11",
    "ICTL_L_CH12",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "ICTL_H_CH1",
    "ICTL_H_CH2",
    "ICTL_H_CH3",
    "ICTL_H_CH4",
    "ICTL_H_CH5",
    "ICTL_H_CH6",
    "ICTL_H_CH7",
    "ICTL_H_CH8",
    "ICTL_H_CH9",
    "ICTL_H_CH10",
    "ICTL_H_CH11",
    "ICTL_H_CH12",
};

#endif //__XD12_C__

static _xd12_general_regs_t gt_xd12_general_regs;
static _xd12_mirror_regs_t gt_xd12_mirror_regs;
static uint16_t gn_xd12_saved_trim_reg[XD12_ADDR_TRIM_MAX];

/* Variable for XD Registers */
static float gf_xd_mclk;
static float gf_vsync_out;
static uint16_t gn_xd_pwm_res;
static uint16_t gn_xd_scan_no;
static uint16_t gn_xd_ch_size;

static uint16_t gn_xd_pwm_max_size;
static uint16_t gn_xd_fpwm_div;
static uint32_t gn_xd_mclk_lock_cnt;

static dev_max_curr_level_t gt_xd_dev_max_curr_level;
static short_level_t gt_xd_short_level;
static fb_level_t gt_xd_fb_level;

static uint16_t gn_xd_delay_ch[XD_CH_SIZE] = {0, };

static void XD12_Set_Delay_CH(void);

void XD12_Write_General_Reg(uint8_t addr, uint16_t data)
{
    if (gt_xd12_general_regs._r3F.addr_ext != XD12_REG_GENERAL)
    {
        gt_xd12_general_regs._r3F.addr_ext = XD12_REG_GENERAL;
        JigBD_IF_Write_Command(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);
    }

    *(&gt_xd12_general_regs._r00.val + addr) = data;
    JigBD_IF_Write_Command(addr, data);
}

uint16_t XD12_Read_General_Reg(uint8_t addr)
{
    if (gt_xd12_general_regs._r3F.addr_ext != XD12_REG_GENERAL)
    {
        gt_xd12_general_regs._r3F.addr_ext = XD12_REG_GENERAL;
        JigBD_IF_Write_Command(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);
    }

    *(&gt_xd12_general_regs._r00.val + addr) = JigBD_IF_Read_Command(addr);

    print(LOG_INFO, "XD12 General Read --> [ 0x%02X - 0x%04X] \r\n", addr, *(&gt_xd12_general_regs._r00.val + addr));

    return *(&gt_xd12_general_regs._r00.val + addr);
}

uint16_t XD12_Get_General_Reg(uint8_t addr)
{
    return *(&gt_xd12_general_regs._r00.val + addr);
}

void XD12_Write_Mirror_Reg(uint8_t addr, uint16_t data)
{
    if (gt_xd12_general_regs._r3F.addr_ext != XD12_REG_MIRROR)
    {
        gt_xd12_general_regs._r3F.addr_ext = XD12_REG_MIRROR;
        JigBD_IF_Write_Command(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);
    }

    *(&gt_xd12_mirror_regs._r00.val + addr) = data;
    JigBD_IF_Write_Command(addr, data);
}

uint16_t XD12_Read_Mirror_Reg(uint8_t addr)
{
    if (gt_xd12_general_regs._r3F.addr_ext != XD12_REG_MIRROR)
    {
        gt_xd12_general_regs._r3F.addr_ext = XD12_REG_MIRROR;
        JigBD_IF_Write_Command(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);
    }

    *(&gt_xd12_mirror_regs._r00.val + addr) = JigBD_IF_Read_Command(addr);

    print(LOG_INFO, "XD12 Mirror Read --> [ 0x%02X - 0x%04X] \r\n", addr, *(&gt_xd12_mirror_regs._r00.val + addr));

    return *(&gt_xd12_mirror_regs._r00.val + addr);
}

uint16_t XD12_Get_Mirror_Reg(uint8_t addr)
{
    return *(&gt_xd12_mirror_regs._r00.val + addr);
}

void XD12_Write_Mirror_Register_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode, uint16_t in_reg_val)
{
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        if (in_reg_val > REG_LIMIT_OSC)
        {
            print(LOG_ERROR, "ERROR: XD12_Write_Mirror_Register_By_Trim_Mode TRIM_OSC_FREQUENCY- in_reg_val(%d) Over !!\r\n", in_reg_val);
        }
        else
        {
            XD12_Write_Mirror_Reg(XD12_ADDR_TRIM_OSC, in_reg_val);
        }
        break;
    case TRIM_VREF_CTL:
        if (in_reg_val > REG_LIMIT_VREF)
        {
            print(LOG_ERROR, "ERROR: XD12_Write_Mirror_Register_By_Trim_Mode TRIM_VREF_CTL- in_reg_val(%d) Over !!\r\n", in_reg_val);
        }
        else
        {
            XD12_Write_Mirror_Reg(XD12_ADDR_TRIM_VREF_CTL, in_reg_val);
        }
        break;
    case TRIM_ICTL_L_CHS:
        if (in_reg_val > REG_LIMIT_ICTL_L)
        {
            print(LOG_ERROR, "ERROR: XD12_Write_Mirror_Register_By_Trim_Mode TRIM_ICTL_L_CHS- in_reg_val(%d) Over !!\r\n", in_reg_val);
        }
        else
        {
            XD12_Write_Mirror_Reg(XD12_ADDR_TRIM_ICTL_L_CH_1 + ch_num, in_reg_val);
        }
        break;
    case TRIM_ICTL_H_CHS:
        if (in_reg_val > REG_LIMIT_ICTL_H)
        {
            print(LOG_ERROR, "ERROR: XD12_Write_Mirror_Register_By_Trim_Mode TRIM_ICTL_H_CHS- in_reg_val(%d) Over !!\r\n", in_reg_val);
        }
        else
        {
            XD12_Write_Mirror_Reg(XD12_ADDR_TRIM_ICTL_H_CH_1 + ch_num, in_reg_val);
        }
        break;
    }
}

uint16_t XD12_Get_Mirror_Register_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0xFFFF;
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        rtn_val = gt_xd12_mirror_regs._r01.val;
        break;
    case TRIM_VREF_CTL:
        rtn_val = gt_xd12_mirror_regs._r02.val;
        break;
    case TRIM_ICTL_L_CHS:
        rtn_val = *(&gt_xd12_mirror_regs._r03.val + ch_num);
        break;
    case TRIM_ICTL_H_CHS:
        rtn_val = *(&gt_xd12_mirror_regs._r1B.val + ch_num);
        break;
    }

    return rtn_val;
}

uint16_t XD12_Get_Mirror_Register_Limit_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0xFFFF;
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        rtn_val = REG_LIMIT_OSC;
        break;
    case TRIM_VREF_CTL:
        rtn_val = REG_LIMIT_VREF;
        break;
    case TRIM_ICTL_L_CHS:
        rtn_val = REG_LIMIT_ICTL_L;
        break;
    case TRIM_ICTL_H_CHS:
        rtn_val = REG_LIMIT_ICTL_H;
        break;
    }

    return rtn_val;
}

void XD12_Read_All_Registers(void)
{
    for (uint8_t addr_offset = 0 ; addr_offset < XD12_ADDR_MAX ; ++addr_offset)
    {
        XD12_Read_General_Reg(addr_offset);
    }

    for (uint8_t addr_offset = 0 ; addr_offset < XD12_ADDR_TRIM_MAX ; ++addr_offset)
    {
        XD12_Read_Mirror_Reg(addr_offset);
    }

    XD12_Dump_All_Registers();
}

void XD12_Dump_All_Registers(void)
{
    for (uint8_t addr_offset = 0 ; addr_offset < XD12_ADDR_MAX ; ++addr_offset)
    {
        switch (addr_offset)
        {
            case XD12_ADDR_RESET_ID :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t RST       : [%u]\r\n"
                            "\t ID        : [%u]\r\n"
                            "\t VALUE     : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r00.rst, gt_xd12_general_regs._r00.id, gt_xd12_general_regs._r00.val);
            break;
        case XD12_ADDR_LD_CONTROL :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t CH_SIZE   : [%u]\r\n"
                            "\t IO_MODE   : [%u]\r\n"
                            "\t SCAN_NO   : [%u]\r\n"
                            "\t OVER_TO_E : [%u]\r\n"
                            "\t PWM_RES   : [%u]\r\n"
                            "\t LD_DIR    : [%u]\r\n"
                            "\t VALUE     : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r01.ch_size, gt_xd12_general_regs._r01.io_mode, gt_xd12_general_regs._r01.scan_no,
                gt_xd12_general_regs._r01.over_to_e, gt_xd12_general_regs._r01.pwm_res, gt_xd12_general_regs._r01.ld_dir, gt_xd12_general_regs._r01.val);
            break;
        case XD12_ADDR_FPWM_DIVIDER :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t FPWM_DIV  : [%u]\r\n"
                            "\t VALUE     : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r02.fpwm_div, gt_xd12_general_regs._r02.val);
            break;
        case XD12_ADDR_CHANNEL_ENABLE :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t CHANNEL_ENABLE : [0x%03X]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r03.val);
            break;
        case XD12_ADDR_FAULT_STATUS :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t MISS_VS   : [%u]\r\n"
                            "\t THERMAL   : [%u]\r\n"
                            "\t SHORT     : [%u]\r\n"
                            "\t OPEN      : [%u]\r\n"
                            "\t FB        : [%u]\r\n"
                            "\t VALUE     : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r05.bit_miss_vs, gt_xd12_general_regs._r05.bit_thermal, gt_xd12_general_regs._r05.bit_short, gt_xd12_general_regs._r05.bit_open, gt_xd12_general_regs._r05.bit_fb, gt_xd12_general_regs._r05.val);
            break;
        case XD12_ADDR_FAULT_LEVEL :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DEV_MAX_CURR_LEVEL : [%u]\r\n"
                            "\t SHORT_LEVEL        : [%u]\r\n"
                            "\t FB_LEVEL           : [%u]\r\n"
                            "\t VALUE              : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r06.dev_max_curr_level, gt_xd12_general_regs._r06.short_level, gt_xd12_general_regs._r06.fb_level, gt_xd12_general_regs._r06.val);
            break;
        case XD12_ADDR_FAULT_CONTROL :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t TIMEOUT       : [%u]\r\n"
                            "\t MS_VS_LOCK    : [%u]\r\n"
                            "\t MS_VS_DIMM    : [%u]\r\n"
                            "\t MS_VS_DET_E   : [%u]\r\n"
                            "\t O_FB_E        : [%u]\r\n"
                            "\t O_DET_E       : [%u]\r\n"
                            "\t S_DET_E       : [%u]\r\n"
                            "\t T_OFF_E       : [%u]\r\n"
                            "\t S_OFF_E       : [%u]\r\n"
                            "\t O_OFF_E       : [%u]\r\n"
                            "\t VALUE         : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r07.timeout_e, gt_xd12_general_regs._r07.ms_vs_lock, gt_xd12_general_regs._r07.ms_vs_dimm, gt_xd12_general_regs._r07.ms_vs_det_e,
                gt_xd12_general_regs._r07.o_fb_e, gt_xd12_general_regs._r07.o_det_e, gt_xd12_general_regs._r07.s_det_e, gt_xd12_general_regs._r07.t_off_e, gt_xd12_general_regs._r07.s_off_e, gt_xd12_general_regs._r07.o_off_e, gt_xd12_general_regs._r07.val);
            break;
        case XD12_ADDR_MAX_CURR_VREF :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t MAX_CURR_VREF : [%u]\r\n"
                            "\t VALUE         : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r08.max_curr_vref, gt_xd12_general_regs._r08.val);
            break;
        case XD12_ADDR_DELAY_CH_EXTEND_1 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH06 : [%u]\r\n"
                            "\t DELAY_CH05 : [%u]\r\n"
                            "\t DELAY_CH04 : [%u]\r\n"
                            "\t DELAY_CH03 : [%u]\r\n"
                            "\t DELAY_CH02 : [%u]\r\n"
                            "\t DELAY_CH01 : [%u]\r\n"
                            "\t VALUE      : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r09.delay_ch6, gt_xd12_general_regs._r09.delay_ch5, gt_xd12_general_regs._r09.delay_ch4, gt_xd12_general_regs._r09.delay_ch3, gt_xd12_general_regs._r09.delay_ch2, gt_xd12_general_regs._r09.delay_ch1, gt_xd12_general_regs._r09.val);
            break;
        case XD12_ADDR_DELAY_CH_EXTEND_2 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH12 : [%u]\r\n"
                            "\t DELAY_CH11 : [%u]\r\n"
                            "\t DELAY_CH10 : [%u]\r\n"
                            "\t DELAY_CH09 : [%u]\r\n"
                            "\t DELAY_CH08 : [%u]\r\n"
                            "\t DELAY_CH07 : [%u]\r\n"
                            "\t VALUE      : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r0A.delay_ch12, gt_xd12_general_regs._r0A.delay_ch11, gt_xd12_general_regs._r0A.delay_ch10, gt_xd12_general_regs._r0A.delay_ch9, gt_xd12_general_regs._r0A.delay_ch8, gt_xd12_general_regs._r0A.delay_ch7, gt_xd12_general_regs._r0A.val);
            break;
        case XD12_ADDR_DELAY_CH01 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH01 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r0B.delay_ch1);
            break;
        case XD12_ADDR_DELAY_CH02 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH02 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r0C.delay_ch2);
            break;
        case XD12_ADDR_DELAY_CH03 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH03 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r0D.delay_ch3);
            break;
        case XD12_ADDR_DELAY_CH04 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH04 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r0E.delay_ch4);
            break;
        case XD12_ADDR_DELAY_CH05 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH05 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r0F.delay_ch5);
            break;
        case XD12_ADDR_DELAY_CH06 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH06 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r10.delay_ch6);
            break;
        case XD12_ADDR_DELAY_CH07 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH07 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r11.delay_ch7);
            break;
        case XD12_ADDR_DELAY_CH08 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH08 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r12.delay_ch8);
            break;
        case XD12_ADDR_DELAY_CH09 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH09 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r13.delay_ch9);
            break;
        case XD12_ADDR_DELAY_CH10 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH10 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r14.delay_ch10);
            break;
        case XD12_ADDR_DELAY_CH11 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH11 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r15.delay_ch11);
            break;
        case XD12_ADDR_DELAY_CH12 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t DELAY_CH12 : [%u]\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r16.delay_ch12);
            break;
        case XD12_ADDR_SERIAL_CLOCK_GEN :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t SERIAL_LOW  : [%u]\r\n"
                            "\t SERIAL_HIGH : [%u]\r\n"
                            "\t VALUE       : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r25.serial_clk_low, gt_xd12_general_regs._r25.serial_clk_high, gt_xd12_general_regs._r25.val);
            break;
        case XD12_ADDR_SERIAL_LATENCY :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t SERIAL_LATENCY : [%u]\r\n"
                            "\t VALUE          : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r26.serial_latency, gt_xd12_general_regs._r26.val);
            break;
        case XD12_ADDR_MCLK_LOCK_1 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t MCLK_LOCK_CNT : [%u]\r\n"
                            "\t VALUE         : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r27.mclk_lock_cnt, gt_xd12_general_regs._r27.val);
            break;
        case XD12_ADDR_MCLK_LOCK_2 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t MCLK_LOCK_CNT_E : [%u]\r\n"
                            "\t FLL_RANGE       : [%u]\r\n"
                            "\t MCLK_LOCK_CNT   : [%u]\r\n"
                            "\t VALUE           : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r28.mclk_lock_cnt_e, gt_xd12_general_regs._r28.fll_range, gt_xd12_general_regs._r28.mclk_lock_cnt, gt_xd12_general_regs._r28.val);
            break;
        case XD12_ADDR_TEMP :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OFS_TEMP : [%u]\r\n"
                            "\t DAC_RNG  : [%u]\r\n"
                            "\t FLT_CNT  : [%u]\r\n"
                            "\t FLT_GAIN : [%u]\r\n"
                            "\t VALUE    : (0x%03X)\r\n\r\n",
                gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r29.ofs_temp, gt_xd12_general_regs._r29.dac_rng, gt_xd12_general_regs._r29.flt_ctl, gt_xd12_general_regs._r29.flt_gain, gt_xd12_general_regs._r29.val);
            break;
        case XD12_ADDR_OSC_FLL_MANUAL_1 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OSC_FLL_MAN : [%u]\r\n"
                            "\t VALUE       : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r2A.osc_fll_man, gt_xd12_general_regs._r2A.val);
            break;
        case XD12_ADDR_OSC_FLL_MANUAL_2 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OSC_MAN_E   : [%u]\r\n"
                            "\t OSC_FLL_MAN : [%u]\r\n"
                            "\t VALUE       : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r2B.osc_man_e, gt_xd12_general_regs._r2B.osc_fll_man, gt_xd12_general_regs._r2B.val);
            break;
        case XD12_ADDR_OSC_FLL_MONITOR :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OSC_FLL_FLT : [%u]\r\n"
                            "\t VALUE       : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r2C.osc_fll_flt, gt_xd12_general_regs._r2C.val);
            break;
        case XD12_ADDR_OTP_ACCESS_1 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OTP_PG_ACCESS_CYCLE : [%u]\r\n"
                            "\t VALUE               : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r3A.otp_pg_access_cycle, gt_xd12_general_regs._r3A.val);
            break;
        case XD12_ADDR_OTP_ACCESS_2 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OTP_PG_ACCESS_CYCLE : [%u]\r\n"
                            "\t VALUE               : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r3B.otp_pg_access_cycle, gt_xd12_general_regs._r3B.val);
            break;
        case XD12_ADDR_OTP_WRITE :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OTP_WSEL            : [%u]\r\n"
                            "\t VALUE               : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r3C.otp_wsel, gt_xd12_general_regs._r3C.val);
            break;
        case XD12_ADDR_OTP_RD_PROG :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OTP_RD_START        : [%u]\r\n"
                            "\t OTP_PG_START        : [%u]\r\n"
                            "\t VALUE               : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r3D.otp_rd_start, gt_xd12_general_regs._r3D.otp_pg_start, gt_xd12_general_regs._r3D.val);
            break;
        case XD12_ADDR_OTP_PROTECT :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t PROTECT_EN/DIS      : [%u]\r\n"
                            "\t VALUE               : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r3E.protect_en, gt_xd12_general_regs._r3E.val);
            break;
        case XD12_ADDR_OTP_OP_MODE :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t TEST_EN             : [%u]\r\n"
                            "\t DDIO_DIS            : [%u]\r\n"
                            "\t TEST_ANA_EN         : [%u]\r\n"
                            "\t PWM_FULL_O          : [%u]\r\n"
                            "\t MCLK32_O            : [%u]\r\n"
                            "\t VREF_O              : [%u]\r\n"
                            "\t ADDR_EXT            : [%u]\r\n"
                            "\t VALUE               : (0x%03X)\r\n\r\n",
            gs_xd12_general_regs_str[addr_offset], addr_offset, gt_xd12_general_regs._r3F.test_en, gt_xd12_general_regs._r3F.ddio_dis, gt_xd12_general_regs._r3F.test_ana_en, gt_xd12_general_regs._r3F.pwm_full_o,
            gt_xd12_general_regs._r3F.mclk32_o, gt_xd12_general_regs._r3F.vref_o, gt_xd12_general_regs._r3F.addr_ext, gt_xd12_general_regs._r3F.val);
            break;
        default :
            continue;
        }
    }

    for (uint8_t addr_offset = 0 ; addr_offset < XD12_ADDR_TRIM_MAX ; ++addr_offset)
    {
        switch (addr_offset)
        {
        case XD12_ADDR_TRIM_OTP_CRC :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OTP_CRC_CHECKSUM : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r00.otp_crc_checksum, gt_xd12_mirror_regs._r00.val);
            break;
        case XD12_ADDR_TRIM_OSC :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t OSC              : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r01.osc, gt_xd12_mirror_regs._r01.val);
            break;
        case XD12_ADDR_TRIM_VREF_CTL :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t VREF_CTL         : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r02.vref_ctl, gt_xd12_mirror_regs._r02.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_1 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_1      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r03.ictl_l_ch_x, gt_xd12_mirror_regs._r03.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_2 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_2      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r04.ictl_l_ch_x, gt_xd12_mirror_regs._r04.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_3 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_3      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r05.ictl_l_ch_x, gt_xd12_mirror_regs._r05.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_4 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_4      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r06.ictl_l_ch_x, gt_xd12_mirror_regs._r06.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_5 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_5      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r07.ictl_l_ch_x, gt_xd12_mirror_regs._r07.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_6 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_6      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r08.ictl_l_ch_x, gt_xd12_mirror_regs._r08.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_7 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_7      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r09.ictl_l_ch_x, gt_xd12_mirror_regs._r09.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_8 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_8      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r0A.ictl_l_ch_x, gt_xd12_mirror_regs._r0A.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_9 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_9      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r0B.ictl_l_ch_x, gt_xd12_mirror_regs._r0B.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_10 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_10     : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r0C.ictl_l_ch_x, gt_xd12_mirror_regs._r0C.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_11 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_11     : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r0D.ictl_l_ch_x, gt_xd12_mirror_regs._r0D.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_12 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_L_CH_12     : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r0E.ictl_l_ch_x, gt_xd12_mirror_regs._r0E.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_1 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_1      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r1B.ictl_h_ch_x, gt_xd12_mirror_regs._r1B.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_2 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_2      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r1C.ictl_h_ch_x, gt_xd12_mirror_regs._r1C.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_3 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_3      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r1D.ictl_h_ch_x, gt_xd12_mirror_regs._r1D.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_4 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_4      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r1E.ictl_h_ch_x, gt_xd12_mirror_regs._r1E.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_5 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_5      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r1F.ictl_h_ch_x, gt_xd12_mirror_regs._r1F.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_6 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_6      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r20.ictl_h_ch_x, gt_xd12_mirror_regs._r20.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_7 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_7      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r21.ictl_h_ch_x, gt_xd12_mirror_regs._r21.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_8 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_8      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r22.ictl_h_ch_x, gt_xd12_mirror_regs._r22.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_9 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_9      : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r23.ictl_h_ch_x, gt_xd12_mirror_regs._r23.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_10 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_10     : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r24.ictl_h_ch_x, gt_xd12_mirror_regs._r24.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_11 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_11     : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r25.ictl_h_ch_x, gt_xd12_mirror_regs._r25.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_12 :
            print(LOG_INFO, "[%s (0x%02X)]\r\n"
                            "\t ICTL_H_CH_12     : [%u]\r\n"
                            "\t VALUE            : (0x%03X)\r\n\r\n",
            gs_xd12_mirror_regs_str[addr_offset], addr_offset, gt_xd12_mirror_regs._r26.ictl_h_ch_x, gt_xd12_mirror_regs._r26.val);
            break;
        default :
            continue;
        }
    }
}

void XD12_Param_Init(void)
{
    gf_xd_mclk = XD_MCLK;
    gf_vsync_out = VSYNC;

    gn_xd_pwm_res = XD_PWM_RES_14BIT;
    gn_xd_scan_no = 0;

    if (gn_xd_pwm_res == XD_PWM_RES_12BIT)
    {
        gn_xd_pwm_max_size = (1 << (12 - gn_xd_scan_no)) - 1;
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 12)));
    }
    else //if (gn_xd_pwm_res == XD_PWM_RES_14BIT)
    {
        gn_xd_pwm_max_size = (1 << (14 - gn_xd_scan_no)) - 1;
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 14)));
    }

    gn_xd_mclk_lock_cnt = XD_MCLK_LOCK_CNT_120Hz;

    gn_xd_ch_size = XD_CH_SIZE;

    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_24mA;
    gt_xd_short_level = SHORT_LEVEL_6V;
    gt_xd_fb_level = FB_LEVEL_0V5;
}

void XD12_Init(void)
{
    XD12_Param_Init();

    JigBD_IF_Reset_Command();
    JigBD_IF_IdGen_Command();

    for (xd12_addr_t xd12_addr = XD12_ADDR_RESET_ID ; xd12_addr < XD12_ADDR_MAX ; ++xd12_addr)
    {
        uint16_t* p_xd_reg_base_addr = gt_xd12_general_regs.ALL;

        switch (xd12_addr)
        {
        case XD12_ADDR_LD_CONTROL :
            gt_xd12_general_regs._r01.ld_dir = XD_LD_DIR_TAIL_SHIFT;
            gt_xd12_general_regs._r01.pwm_res = gn_xd_pwm_res;
            gt_xd12_general_regs._r01.over_to_e = 1;
            gt_xd12_general_regs._r01.scan_no = gn_xd_scan_no;
            gt_xd12_general_regs._r01.io_mode = XD_IO_MODE_NOP;
            gt_xd12_general_regs._r01.ch_size = gn_xd_ch_size;
            break;
        case XD12_ADDR_FPWM_DIVIDER :
            gt_xd12_general_regs._r02.fpwm_div = gn_xd_fpwm_div;
            break;
        case XD12_ADDR_CHANNEL_ENABLE :
            gt_xd12_general_regs._r03.val = 0xFFF;
            break;
        case XD12_ADDR_FAULT_LEVEL :
            gt_xd12_general_regs._r06.fb_level = gt_xd_fb_level;
            gt_xd12_general_regs._r06.short_level = gt_xd_short_level;
            gt_xd12_general_regs._r06.dev_max_curr_level = gt_xd_dev_max_curr_level;
            break;
        case XD12_ADDR_FAULT_CONTROL :
            gt_xd12_general_regs._r07.o_off_e = 0;
            gt_xd12_general_regs._r07.s_off_e = 0;
            gt_xd12_general_regs._r07.t_off_e = 0;
            gt_xd12_general_regs._r07.s_det_e = 1;
            gt_xd12_general_regs._r07.o_det_e = 1;
            gt_xd12_general_regs._r07.o_fb_e = 0;
            gt_xd12_general_regs._r07.ms_vs_det_e = 0;
            gt_xd12_general_regs._r07.ms_vs_dimm = 0;
            gt_xd12_general_regs._r07.ms_vs_lock = 0;
            gt_xd12_general_regs._r07.timeout_e = 1;
            break;
        case XD12_ADDR_MAX_CURR_VREF :
            gt_xd12_general_regs._r08.max_curr_vref = 0xFFF;
            break;
        case XD12_ADDR_SERIAL_CLOCK_GEN :
            gt_xd12_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
            gt_xd12_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
            break;
        case XD12_ADDR_SERIAL_LATENCY :
            gt_xd12_general_regs._r26.serial_latency = 60;
            break;
        case XD12_ADDR_MCLK_LOCK_1 :
            gt_xd12_general_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0x000FFF) >>  0);
            break;
        case XD12_ADDR_MCLK_LOCK_2 :
            gt_xd12_general_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0xFFF000) >> 12);
            gt_xd12_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_ENABLE;
            break;
        case XD12_ADDR_TEMP :
            gt_xd12_general_regs._r29.flt_gain = 0;
            gt_xd12_general_regs._r29.ofs_temp = 8;
            break;
        case XD12_ADDR_OSC_FLL_MANUAL_1 :
            gt_xd12_general_regs._r2A.osc_fll_man = 0x000;
            break;
        case XD12_ADDR_OSC_FLL_MANUAL_2 :
            gt_xd12_general_regs._r2B.osc_fll_man = 8;
            gt_xd12_general_regs._r2B.osc_man_e = 0;
            break;
        /*case XD12_ADDR_OSC_FLL_MONITOR :
            gt_xd12_general_regs._r2C.osc_fll_flt = 0;
            break;
        */
        default :
            continue;
        }
        XD12_Write_General_Reg(xd12_addr, *(p_xd_reg_base_addr + xd12_addr));
    }

    XD12_Set_Delay_CH();

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = XD_FB_IN_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(XD_FB_IN_GPIO_Port, &GPIO_InitStruct);

    XD12_Read_All_Registers();
}

void XD12_Trim_Param_Init(void)
{
    gt_xd_fb_level = FB_LEVEL_0V5;
    gt_xd_short_level = SHORT_LEVEL_36V;
    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_8mA;
}

void XD12_Trim_Init(void)
{
    XD12_Trim_Param_Init();

    JigBD_IF_Reset_Command();
    JigBD_IF_IdGen_Command();

    for (xd12_addr_t xd12_addr = XD12_ADDR_RESET_ID ; xd12_addr < XD12_ADDR_MAX ; ++xd12_addr)
    {
        uint16_t* p_xd_reg_base_addr = gt_xd12_general_regs.ALL;

        switch (xd12_addr)
        {
        case XD12_ADDR_CHANNEL_ENABLE :
            gt_xd12_general_regs._r03.val = 0xFFF;
            break;
        case XD12_ADDR_FAULT_LEVEL :
            gt_xd12_general_regs._r06.fb_level = gt_xd_fb_level;
            gt_xd12_general_regs._r06.short_level = gt_xd_short_level;
            gt_xd12_general_regs._r06.dev_max_curr_level = gt_xd_dev_max_curr_level;
            break;
        case XD12_ADDR_MAX_CURR_VREF :
            gt_xd12_general_regs._r08.max_curr_vref = XD12_CURRENT_TRIM_VREF;
            break;
        case XD12_ADDR_SERIAL_CLOCK_GEN :
            gt_xd12_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
            gt_xd12_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
            break;
        case XD12_ADDR_SERIAL_LATENCY :
            gt_xd12_general_regs._r26.serial_latency = 60;
            break;
        case XD12_ADDR_OTP_OP_MODE :
            gt_xd12_general_regs._r3F.test_en = 1;
            gt_xd12_general_regs._r3F.ddio_dis = 1;
            break;
        default :
            continue;
        }
        XD12_Write_General_Reg(xd12_addr, *(p_xd_reg_base_addr + xd12_addr));
    }
    XD12_Set_OTP_Protect(false);
    XD12_Read_All_Registers();
}

/* ================================================================================================================================================= */
/* General Function */
/* ================================================================================================================================================= */
static void XD12_Set_Delay_CH(void)
{
    uint16_t delay_per_ch = 0;
    delay_per_ch = (uint16_t)(gn_xd_pwm_max_size / gn_xd_ch_size);
    for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
    {
        gn_xd_delay_ch[ch] = delay_per_ch * ch;
        uint16_t delay_lsb = ((gn_xd_delay_ch[ch] & 0x0FFF) >>  0);
        uint16_t delay_msb = ((gn_xd_delay_ch[ch] & 0x3000) >> 12);
        delay_msb <<= (2 * (ch % 6));

        XD12_Write_General_Reg(XD12_ADDR_DELAY_CH01 + ch, delay_lsb);
        XD12_Write_General_Reg(XD12_ADDR_DELAY_CH_EXTEND_1 + (ch / 6), delay_msb);
    }
}

void XD12_Set_Max_Current_Level(dev_max_curr_level_t in_dev_max_curr)
{
    gt_xd12_general_regs._r06.dev_max_curr_level = in_dev_max_curr;
    XD12_Write_General_Reg(XD12_ADDR_FAULT_LEVEL, gt_xd12_general_regs._r06.val);
}

float XD12_Get_Max_Current_level(void)
{
    float f_rtn = 0.0f;

    uint16_t fault_level = XD12_Read_General_Reg(XD12_ADDR_FAULT_LEVEL);
    dev_max_curr_level_t dev_max_curr_lvl = (dev_max_curr_level_t)((fault_level & 0xF00) >> 8);

    switch (dev_max_curr_lvl)
    {
    case DEV_MAX_CURR_LEVEL_4mA :
        f_rtn = 4.0f;
        break;
    case DEV_MAX_CURR_LEVEL_8mA :
        f_rtn = 8.0f;
        break;
    case DEV_MAX_CURR_LEVEL_12mA :
        f_rtn = 12.0f;
        break;
    case DEV_MAX_CURR_LEVEL_16mA :
        f_rtn = 16.0f;
        break;
    case DEV_MAX_CURR_LEVEL_24mA :
        f_rtn = 24.0f;
        break;
    case DEV_MAX_CURR_LEVEL_32mA :
        f_rtn = 32.0f;
        break;
    case DEV_MAX_CURR_LEVEL_46mA :
        f_rtn = 46.0f;
        break;
    case DEV_MAX_CURR_LEVEL_64mA :
        f_rtn = 64.0f;
        break;
    }
    return f_rtn;
}

bool XD12_Is_Vsync_Mode_External(void)
{
    if (gt_xd12_general_regs._r01.io_mode == XD_IO_MODE_EXT_VSYNC)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void XD12_Set_OSC_Manual_En(bool in_Enable)
{
    if (in_Enable == TRUE)
    {
        gt_xd12_general_regs._r2B.osc_man_e = 1;
        XD12_Write_General_Reg(XD12_ADDR_OSC_FLL_MANUAL_2, gt_xd12_general_regs._r2B.val);
    }
    else
    {
        gt_xd12_general_regs._r2B.osc_man_e = 0;
        XD12_Write_General_Reg(XD12_ADDR_OSC_FLL_MANUAL_2, gt_xd12_general_regs._r2B.val);
    }
}

static void XD12_Set_OSC_Manual(uint16_t n_osc_fll_man)
{
    uint16_t osc_fll_man_lsb = ((n_osc_fll_man & 0x0FFF) >>  0);
    uint16_t osc_fll_man_msb = ((n_osc_fll_man & 0xF000) >> 12);

    gt_xd12_general_regs._r2A.osc_fll_man = osc_fll_man_lsb;
    XD12_Write_General_Reg(XD12_ADDR_OSC_FLL_MANUAL_1, gt_xd12_general_regs._r2A.val);

    gt_xd12_general_regs._r2B.osc_fll_man = osc_fll_man_msb;
    XD12_Write_General_Reg(XD12_ADDR_OSC_FLL_MANUAL_2, gt_xd12_general_regs._r2B.val);
}

void XD12_Update_Vsync_Frequency(float n_freq)
{
    uint32_t period = 0;
    uint32_t prescale = 0;

    prescale = LL_TIM_GetPrescaler(TIM8);
    period = (uint32_t)((TIM8_FREQ / (prescale + 1)) / n_freq - 1 + 0.5f);

    LL_TIM_SetAutoReload(TIM8, period);

    gf_vsync_out = n_freq;

    // 1. disable mclk_lock_cnt_e
    gt_xd12_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_DISABLE;
    XD12_Write_General_Reg(XD12_ADDR_MCLK_LOCK_2, gt_xd12_general_regs._r28.val);

    // 2. increase or decrease fpwm_div
    if (gn_xd_pwm_res == XD_PWM_RES_12BIT)
    {
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 12)));
    }
    else //if (gn_xd_pwm_res == XD_PWM_RES_14BIT)
    {
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 14)));
    }
    gt_xd12_general_regs._r02.fpwm_div = gn_xd_fpwm_div;
    XD12_Write_General_Reg(XD12_ADDR_FPWM_DIVIDER, gt_xd12_general_regs._r02.val);

    // 3. change mclk_lock_cnt
    gn_xd_mclk_lock_cnt = (uint32_t)((XD_MCLK_LOCK_CNT_120Hz) * (VSYNC / gf_vsync_out) + 0.5f);
    gt_xd12_general_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0x000FFF) >>  0);
    gt_xd12_general_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0xFFF000) >> 12);

    XD12_Write_General_Reg(XD12_ADDR_MCLK_LOCK_1, gt_xd12_general_regs._r27.val);
    XD12_Write_General_Reg(XD12_ADDR_MCLK_LOCK_2, gt_xd12_general_regs._r28.val);

    // 4. enable mclk_lock_cnt_e
    gt_xd12_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_ENABLE;
    XD12_Write_General_Reg(XD12_ADDR_MCLK_LOCK_2, gt_xd12_general_regs._r28.val);
}
/* ================================================================================================================================================= */
/* Trim Function */
/* ================================================================================================================================================= */

void XD12_Save_Trim_Regs(void)
{
    for (uint8_t addr = XD12_ADDR_TRIM_OTP_CRC ; addr < XD12_ADDR_TRIM_MAX ; ++addr)
    {
        gn_xd12_saved_trim_reg[addr] = XD12_Read_Mirror_Reg(addr);
    }

    print(LOG_INFO, "osc,%3u\r\n", gn_xd12_saved_trim_reg[1]);
    print(LOG_INFO, "vref,%3u\r\n", gn_xd12_saved_trim_reg[2]);

    print(LOG_INFO, "ictl_l,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n"
    , gn_xd12_saved_trim_reg[ 3], gn_xd12_saved_trim_reg[ 4], gn_xd12_saved_trim_reg[ 5] , gn_xd12_saved_trim_reg[ 6], gn_xd12_saved_trim_reg[ 7], gn_xd12_saved_trim_reg[ 8]
    , gn_xd12_saved_trim_reg[ 9], gn_xd12_saved_trim_reg[10], gn_xd12_saved_trim_reg[11] , gn_xd12_saved_trim_reg[12], gn_xd12_saved_trim_reg[13], gn_xd12_saved_trim_reg[14]
    );
    print(LOG_INFO, "ictl_h,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n"
    , gn_xd12_saved_trim_reg[27], gn_xd12_saved_trim_reg[28], gn_xd12_saved_trim_reg[29] , gn_xd12_saved_trim_reg[30], gn_xd12_saved_trim_reg[31], gn_xd12_saved_trim_reg[32]
    , gn_xd12_saved_trim_reg[33], gn_xd12_saved_trim_reg[34], gn_xd12_saved_trim_reg[35] , gn_xd12_saved_trim_reg[36], gn_xd12_saved_trim_reg[37], gn_xd12_saved_trim_reg[38]
    );
}

uint64_t XD12_Compare_Trim_Regs(void)
{
    uint64_t ret = 0;
    uint16_t u16_reg_val = 0;

    for (xd12_trim_addr_t trim_addr = XD12_ADDR_TRIM_OSC ; trim_addr < XD12_ADDR_TRIM_MAX ; ++trim_addr)
    {
        u16_reg_val = XD12_Read_Mirror_Reg(trim_addr);

        if (gn_xd12_saved_trim_reg[trim_addr] != u16_reg_val)
        {
            ret |= ((uint64_t)1 << trim_addr);
            print(LOG_INFO, "%s %17s - NG", ANSI_FONT_RED, gs_xd12_mirror_regs_str[trim_addr]);
        }
        else
        {
            print(LOG_INFO, "%s %17s - OK", ANSI_FONT_GREEN, gs_xd12_mirror_regs_str[trim_addr]);
        }
        print(LOG_INFO, "   [0x%03X] - [0x%03X] %s\r\n", gn_xd12_saved_trim_reg[trim_addr], u16_reg_val, ANSI_FONT_NONE);
    }

    return ret;
}

void XD12_Trim_Init_VREF_CTL(void)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.vref_o = 1;
    gt_xd12_general_regs._r3F.mclk32_o = 0;
    gt_xd12_general_regs._r3F.pwm_full_o = 0;
    gt_xd12_general_regs._r3F.test_ana_en = 3;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);
}
void XD12_Trim_Init_OSC(void)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.vref_o = 0;
    gt_xd12_general_regs._r3F.mclk32_o = 1;
    gt_xd12_general_regs._r3F.pwm_full_o = 0;
    gt_xd12_general_regs._r3F.test_ana_en = 0;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);

    XD12_Set_OSC_Manual_En(true);
    XD12_Set_OSC_Manual(32768);
}
void XD12_Trim_Init_ICTL(void)
{
    gt_xd12_general_regs._r3F.test_en = 1;
    gt_xd12_general_regs._r3F.vref_o = 0;
    gt_xd12_general_regs._r3F.mclk32_o = 0;
    gt_xd12_general_regs._r3F.pwm_full_o = 1;
    gt_xd12_general_regs._r3F.test_ana_en = 0;
    XD12_Write_General_Reg(XD12_ADDR_OTP_OP_MODE, gt_xd12_general_regs._r3F.val);

    gt_xd12_general_regs._r08.max_curr_vref = XD12_CURRENT_TRIM_VREF;
    XD12_Write_General_Reg(XD12_ADDR_MAX_CURR_VREF, gt_xd12_general_regs._r08.val);
}

void XD12_Set_OTP_Protect(bool en)
{
    if (en == true)
    {
        gt_xd12_general_regs._r3E.protect_en = XD12_OTP_PROTECT_ENABLE;
    }
    else
    {
        gt_xd12_general_regs._r3E.protect_en = XD12_OTP_PROTECT_DISABLE;
    }
    XD12_Write_General_Reg(XD12_ADDR_OTP_PROTECT, gt_xd12_general_regs._r3E.val);
}

void XD12_Set_OTP_PG_Start(bool en)
{
    if (en == true)
    {
        gt_xd12_general_regs._r3D.otp_pg_start = 1;
    }
    else
    {
        gt_xd12_general_regs._r3D.otp_pg_start = 0;
    }
    XD12_Write_General_Reg(XD12_ADDR_OTP_RD_PROG, gt_xd12_general_regs._r3D.val);
}