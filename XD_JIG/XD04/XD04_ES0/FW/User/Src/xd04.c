/** @file xd12.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XD12_C__
#include "config.h"

#define OTP_PROTECT_DISABLE     (0xA5A)
#define OTP_PROTECT_ENABLE      (0x5A5)

/* Trimming spec */
#define XD12_ERR_RATE           (1.0/100) /* +/-% */
#define XD12_OSC_TARGET         (39.3192) /* MHz */
#define XD12_VREF_TARGET        (2.2)     /* V */

#define XD12_CURRENT_TRIM_VREF  (4095)

#define XD12_ICTL_L_ERR_RATE    (0.5/100) /* +/-% */
#define XD12_ICTL_L_TARGET      (8.0000f)   /* mA */
#define XD12_ICTL_L_P1          (DEV_MAX_CURR_LEVEL_8mA)
#define XD12_ICTL_L_P2          (DEV_MAX_CURR_LEVEL_8mA)

#define XD12_ICTL_H_ERR_RATE    (0.5/100) /* +/-% */
#define XD12_ICTL_H_TARGET      (24.000f)  /* mA */
#define XD12_ICTL_H_P1          (DEV_MAX_CURR_LEVEL_24mA)
#define XD12_ICTL_H_P2          (DEV_MAX_CURR_LEVEL_24mA)

#define XD12_DEFAULT_OSC        (0x20)
#define XD12_DEFAULT_VREF_CTL   (0x20)
#define XD12_DEFAULT_ICTL_L     (0x40)
#define XD12_DEFAULT_ICTL_H     (0x40)

#define XD_MCLK                 (39319200.0f)
#define VSYNC                   (120.0f)

#define TIM8_FREQ               (144000000.0f)

#define XD_SUB_FRAME_SIZE       (64)

#define XD_LD_DIR_HEAD_SHIFT    (0)
#define XD_LD_DIR_TAIL_SHIFT    (1)

#define XD_PWM_RES_12BIT        (0)
#define XD_PWM_RES_14BIT        (1)

#define XD_VSYNC_INTERNAL       (0)
#define XD_VSYNC_EXTERNAL       (1)

#define XD_MCLK_FLL_ENABLE      (0)
#define XD_MCLK_FLL_DISABLE     (1)

#define XD_MCLK_LOCK_CNT_120Hz  /*(329167)*/(327600)

#ifdef __XD12_C__

static const char* gs_xd12_addr_str[XD12_ADDR_MAX] =
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
    "OTP_ACCESS_1",
    "OTP_ACCESS_2"
    "OTP_WRITE"
    "OTP_RD_PROG"
    "OTP_PROTECT",
    "OP_MODE"
};

static const char* gs_xd12_addr_trim_str[XD12_ADDR_TRIM_MAX] =
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

static const char* gs_xd12_reg_type[XD12_REG_TYPE_MAX] =
{
    "XD12_REG_TYPE_NORMAL",
    "XD12_REG_TYPE_TRIM",
};
#endif //__XD12_C__

static _xd12_regs_t gt_xd12_regs;
static _xd12_trim_regs_t gt_xd12_trim_regs;

static XD12_REG_TYPE gt_xd12_reg_type = XD12_REG_TYPE_MAX;

static uint16_t gn_xd_trim_reg[XD12_ADDR_TRIM_MAX];

static uint8_t gn_xd_dimming_ch;

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

static void XD12_Select_Reg_Type(XD12_REG_TYPE n_reg_type);
static void XD12_Write_REGISTER(uint8_t in_addr, uint16_t in_data, XD12_REG_TYPE n_reg_type);
static uint16_t XD12_Read_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type);
static uint16_t XD12_Get_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type);

static void XD12_OTP_Protect_Disable();
static void XD12_OTP_Protect_Enable();

static uint8_t XD12_Write_OSC(uint16_t in_val);
static uint8_t XD12_Write_VREF_CTL(uint16_t in_val);

static uint8_t XD12_Write_ICTL_L_CHx(uint8_t in_ch, uint16_t in_val);
static uint8_t XD12_Write_ICTL_H_CHx(uint8_t in_ch, uint16_t in_val);

void XD12_set_dimming_channel(uint8_t n_channel)
{
    if (n_channel < (CH_MAX + 1))
    {
        gn_xd_dimming_ch = n_channel;
    }
}

uint8_t XD12_get_dimming_channel(void)
{
    return gn_xd_dimming_ch;
}

static void XD12_Select_Reg_Type(XD12_REG_TYPE n_reg_type)
{
    volatile static uint16_t temp_op_mode_val = 0xFFFF;
    _xd12_op_mode_t* p_op_mode = &(gt_xd12_regs._r3F);

    if (n_reg_type == XD12_REG_TYPE_NON_TRIM)
    {
        p_op_mode->addr_ext = 0;
    }
    else if (n_reg_type == XD12_REG_TYPE_TRIM)
    {
        p_op_mode->addr_ext = 1;
    }

    if (p_op_mode->addr_ext != temp_op_mode_val)
    {
        JigBd_IF_Write_Command(XD12_ADDR_OTP_OP_MODE, p_op_mode->val);
    }

    temp_op_mode_val = p_op_mode->val;
}

void XD12_Set_Reg_Type(XD12_REG_TYPE n_reg_type)
{
    if (n_reg_type < XD12_REG_TYPE_MAX)
    {
        gt_xd12_reg_type = n_reg_type;
        XD12_Select_Reg_Type(n_reg_type);
    }
}

XD12_REG_TYPE XD12_Get_Reg_Type(void)
{
    return gt_xd12_reg_type;
}

static void XD12_Write_REGISTER(uint8_t in_addr, uint16_t in_data, XD12_REG_TYPE n_reg_type)
{
    uint16_t* p_xd12_reg_addr_base = NULL;
    gt_xd12_reg_type = n_reg_type;

    if (gt_xd12_reg_type == XD12_REG_TYPE_TRIM)
    {
        if (OTP_PROTECT_DISABLE != XD12_Get_REGISTER(XD12_ADDR_OTP_PROTECT, XD12_REG_TYPE_NON_TRIM))
        {
            XD12_OTP_Protect_Disable();
        }
    }

    XD12_Select_Reg_Type(gt_xd12_reg_type);

    switch (gt_xd12_reg_type)
    {
    case XD12_REG_TYPE_NON_TRIM :
        p_xd12_reg_addr_base = gt_xd12_regs.ALL;
        break;
    case XD12_REG_TYPE_TRIM :
        p_xd12_reg_addr_base = gt_xd12_trim_regs.ALL;
        break;
    }
    *(p_xd12_reg_addr_base + in_addr) = in_data;

    JigBd_IF_Write_Command(in_addr, *(p_xd12_reg_addr_base + in_addr));
}

static uint16_t XD12_Read_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type)
{
    uint16_t u16_reg_val = 0;
    uint16_t* p_xd12_reg_addr_base = NULL;

    gt_xd12_reg_type = n_reg_type;
    XD12_Select_Reg_Type(gt_xd12_reg_type);

    u16_reg_val = JigBd_IF_Read_Command(in_addr);

    switch (gt_xd12_reg_type)
    {
    case XD12_REG_TYPE_NON_TRIM :
        p_xd12_reg_addr_base = gt_xd12_regs.ALL;
        break;
    case XD12_REG_TYPE_TRIM :
        p_xd12_reg_addr_base = gt_xd12_trim_regs.ALL;
        break;
    }
    *(p_xd12_reg_addr_base + in_addr) = u16_reg_val;
    // print(LOG_DEBUG, "[%s][ 0x%02X : 0x%04X ]\r\n", gs_xd12_reg_type[n_reg_type], in_addr, *(p_xd12_reg_addr_base + in_addr));
    return u16_reg_val;
}

static uint16_t XD12_Get_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type)
{
    uint16_t u16_reg_val = 0;
    uint16_t* p_xd12_reg_addr_base = NULL;

    switch (n_reg_type)
    {
    case XD12_REG_TYPE_NON_TRIM :
        p_xd12_reg_addr_base = gt_xd12_regs.ALL;
        break;
    case XD12_REG_TYPE_TRIM :
        p_xd12_reg_addr_base = gt_xd12_trim_regs.ALL;
        break;
    }

    u16_reg_val = *(p_xd12_reg_addr_base + in_addr);
    //print(LOG_DEBUG, "\r\n XD12_Get_REGISTER(0x%2X, %s, 0x%02X) = 0x%X \r\n", in_addr, gs_xd12_reg_type[n_reg_type], u16_reg_val);
    return u16_reg_val;
}

static void XD12_Print_XD12_REGS(void)
{
    print(LOG_INFO, "\r\n");
    for(uint8_t addr_offset = 0 ; addr_offset < XD12_ADDR_MAX ; ++addr_offset)
    {
        switch (addr_offset)
        {
        case XD12_ADDR_RESET_ID :
            print(LOG_INFO, "RST - [%u], ID - [%u] (0x%03X)\r\n", gt_xd12_regs._r00.rst, gt_xd12_regs._r00.id, gt_xd12_regs._r00.val);
            break;
        case XD12_ADDR_LD_CONTROL :
            print(LOG_INFO, "CH_SIZE - [%u], EXT_VS_E - [%u], EXT_VS_POL - [%u], SCAN_NO - [%u], OVER_TO_E - [%u], PWM_RES - [%u], LD_DIR - [%u] (0x%03X)\r\n",\
            gt_xd12_regs._r01.ch_size, gt_xd12_regs._r01.ext_vs_e, gt_xd12_regs._r01.ext_vs_pol, gt_xd12_regs._r01.scan_no,\
            gt_xd12_regs._r01.over_to_e, gt_xd12_regs._r01.pwm_res, gt_xd12_regs._r01.ld_dir, gt_xd12_regs._r01.val);
            break;
        case XD12_ADDR_FPWM_DIVIDER :
            print(LOG_INFO, "FPWM_DIV - [%u] (0x%03X)\r\n", gt_xd12_regs._r02.fpwm_div, gt_xd12_regs._r02.val);
            break;
        case XD12_ADDR_CHANNEL_ENABLE :
            print(LOG_INFO, "CHANNEL_ENABLE - [0x%03X]\r\n", gt_xd12_regs._r03.val);
            break;
        case XD12_ADDR_FAULT_STATUS :
            print(LOG_INFO, "MISS_VS - [%u], THERMAL - [%u], SHORT - [%u], OPEN - [%u], FB - [%u] (0x%03X)\r\n",\
            gt_xd12_regs._r05.bit_miss_vs, gt_xd12_regs._r05.bit_thermal, gt_xd12_regs._r05.bit_short, gt_xd12_regs._r05.bit_open, gt_xd12_regs._r05.bit_fb, gt_xd12_regs._r05.val);
            break;
        case XD12_ADDR_FAULT_LEVEL :
            print(LOG_INFO, "DEV_MAX_CURR_LEVEL - [%u], SHORT_LEVEL - [%u], FB_LEVEL - [%u] (0x%03X)\r\n",\
            gt_xd12_regs._r06.dev_max_curr_level, gt_xd12_regs._r06.short_level, gt_xd12_regs._r06.fb_level, gt_xd12_regs._r06.val);
            break;
        case XD12_ADDR_FAULT_CONTROL :
            print(LOG_INFO, "TIMEOUT - [%u], FBO_E - [%u], MS_VS_LOCK - [%u], MS_VS_DIMM - [%u], MS_VS_DET_E - [%u], O_FB_E - [%u] "
            "O_DET_E - [%u], S_DET_E - [%u], T_OFF_E - [%u], S_OFF_E - [%u], O_OFF_E - [%u] (0x%03X)\r\n",\
            gt_xd12_regs._r07.timeout_e, gt_xd12_regs._r07.fbo_e, gt_xd12_regs._r07.ms_vs_lock, gt_xd12_regs._r07.ms_vs_dimm, gt_xd12_regs._r07.ms_vs_det_e, gt_xd12_regs._r07.o_fb_e,\
            gt_xd12_regs._r07.o_det_e, gt_xd12_regs._r07.s_det_e, gt_xd12_regs._r07.t_off_e, gt_xd12_regs._r07.s_off_e, gt_xd12_regs._r07.o_off_e, gt_xd12_regs._r07.val);
            break;
        case XD12_ADDR_MAX_CURR_VREF :
            print(LOG_INFO, "MAX_CURR_VREF - [%u] (0x%03X)\r\n", gt_xd12_regs._r08.max_curr_vref, gt_xd12_regs._r08.val);
            break;
        case XD12_ADDR_DELAY_CH_EXTEND_1 :
            print(LOG_INFO, "DELAY_CH06 - [%u], DELAY_CH05 - [%u], DELAY_CH04 - [%u], DELAY_CH03 - [%u], DELAY_CH02 - [%u], DELAY_CH01 - [%u] (0x%03X)\r\n",\
            gt_xd12_regs._r09.delay_ch6, gt_xd12_regs._r09.delay_ch5, gt_xd12_regs._r09.delay_ch4, gt_xd12_regs._r09.delay_ch3, gt_xd12_regs._r09.delay_ch2, gt_xd12_regs._r09.delay_ch1, gt_xd12_regs._r09.val);
            break;
        case XD12_ADDR_DELAY_CH_EXTEND_2 :
            print(LOG_INFO, "DELAY_CH12 - [%u], DELAY_CH11 - [%u], DELAY_CH10 - [%u], DELAY_CH09 - [%u], DELAY_CH08 - [%u], DELAY_CH07 - [%u] (0x%03X)\r\n",\
            gt_xd12_regs._r0A.delay_ch12, gt_xd12_regs._r0A.delay_ch11, gt_xd12_regs._r0A.delay_ch10, gt_xd12_regs._r0A.delay_ch9, gt_xd12_regs._r0A.delay_ch8, gt_xd12_regs._r0A.delay_ch7, gt_xd12_regs._r0A.val);
            break;
        case XD12_ADDR_DELAY_CH01 :
            print(LOG_INFO, "DELAY_CH01 - [%u]\r\n", gt_xd12_regs._r0B.delay_ch1);
            break;
        case XD12_ADDR_DELAY_CH02 :
            print(LOG_INFO, "DELAY_CH02 - [%u]\r\n", gt_xd12_regs._r0C.delay_ch2);
            break;
        case XD12_ADDR_DELAY_CH03 :
            print(LOG_INFO, "DELAY_CH03 - [%u]\r\n", gt_xd12_regs._r0D.delay_ch3);
            break;
        case XD12_ADDR_DELAY_CH04 :
            print(LOG_INFO, "DELAY_CH04 - [%u]\r\n", gt_xd12_regs._r0E.delay_ch4);
            break;
        case XD12_ADDR_DELAY_CH05 :
            print(LOG_INFO, "DELAY_CH05 - [%u]\r\n", gt_xd12_regs._r0F.delay_ch5);
            break;
        case XD12_ADDR_DELAY_CH06 :
            print(LOG_INFO, "DELAY_CH06 - [%u]\r\n", gt_xd12_regs._r10.delay_ch6);
            break;
        case XD12_ADDR_DELAY_CH07 :
            print(LOG_INFO, "DELAY_CH07 - [%u]\r\n", gt_xd12_regs._r11.delay_ch7);
            break;
        case XD12_ADDR_DELAY_CH08 :
            print(LOG_INFO, "DELAY_CH08 - [%u]\r\n", gt_xd12_regs._r12.delay_ch8);
            break;
        case XD12_ADDR_DELAY_CH09 :
            print(LOG_INFO, "DELAY_CH09 - [%u]\r\n", gt_xd12_regs._r13.delay_ch9);
            break;
        case XD12_ADDR_DELAY_CH10 :
            print(LOG_INFO, "DELAY_CH10 - [%u]\r\n", gt_xd12_regs._r14.delay_ch10);
            break;
        case XD12_ADDR_DELAY_CH11 :
            print(LOG_INFO, "DELAY_CH11 - [%u]\r\n", gt_xd12_regs._r15.delay_ch11);
            break;
        case XD12_ADDR_DELAY_CH12 :
            print(LOG_INFO, "DELAY_CH12 - [%u]\r\n", gt_xd12_regs._r16.delay_ch12);
            break;
        case XD12_ADDR_SERIAL_CLOCK_GEN :
            print(LOG_INFO, "SERIAL_LOW - [%u], SERIAL_HIGH - [%u] (0x%03X)\r\n", gt_xd12_regs._r25.serial_clk_low, gt_xd12_regs._r25.serial_clk_high, gt_xd12_regs._r25.val);
            break;
        case XD12_ADDR_SERIAL_LATENCY :
            print(LOG_INFO, "SERIAL_LATENCY - [%u] (0x%03X)\r\n", gt_xd12_regs._r26.val, gt_xd12_regs._r26.val);
            break;
        case XD12_ADDR_MCLK_LOCK_1 :
            print(LOG_INFO, "MCLK_LOCK_CNT - [%u] (0x%03X)\r\n", gt_xd12_regs._r27.val, gt_xd12_regs._r27.val);
            break;
        case XD12_ADDR_MCLK_LOCK_2 :
            print(LOG_INFO, "MCLK_LOCK_CNT_E - [%u], MCLK_LOCK_CNT - [%u] (0x%03X)\r\n", gt_xd12_regs._r28.mclk_lock_cnt_e, gt_xd12_regs._r28.mclk_lock_cnt, gt_xd12_regs._r28.val);
            break;
        case XD12_ADDR_TEMP :
            print(LOG_INFO, "OFS_TEMP - [%u], FLT_GAIN - [%u] (0x%03X)\r\n", gt_xd12_regs._r29.ofs_temp, gt_xd12_regs._r29.flt_gain, gt_xd12_regs._r29.val);
            break;
        case XD12_ADDR_OSC_FLL_MANUAL_1 :
            print(LOG_INFO, "OSC_FLL_MAN - [%u] (0x%03X)\r\n", gt_xd12_regs._r2A.osc_fll_man, gt_xd12_regs._r2A.val);
            break;
        case XD12_ADDR_OSC_FLL_MANUAL_2 :
            print(LOG_INFO, "OSC_MAN_E - [%u], OSC_FLL_MAN - [%u] (0x%03X)\r\n", gt_xd12_regs._r2B.osc_man_e, gt_xd12_regs._r2B.osc_fll_man, gt_xd12_regs._r2B.val);
            break;
        case XD12_ADDR_OTP_ACCESS_1 :
            print(LOG_INFO, "OTP_PG_ACCESS_CYCLE - [%u] (0x%03X)\r\n", gt_xd12_regs._r3A.otp_pg_access_cycle, gt_xd12_regs._r3A.val);
            break;
        case XD12_ADDR_OTP_ACCESS_2 :
            print(LOG_INFO, "OTP_PG_ACCESS_CYCLE - [%u] (0x%03X)\r\n", gt_xd12_regs._r3B.otp_pg_access_cycle, gt_xd12_regs._r3B.val);
            break;
        case XD12_ADDR_OTP_WRITE :
            print(LOG_INFO, "OTP_WSEL - [%u] (0x%03X)\r\n", gt_xd12_regs._r3C.otp_wsel, gt_xd12_regs._r3C.val);
            break;
        case XD12_ADDR_OTP_RD_PROG :
            print(LOG_INFO, "OTP_RD_START - [%u], OTP_PG_START - [%u] (0x%03X)\r\n", gt_xd12_regs._r3D.otp_rd_start, gt_xd12_regs._r3D.otp_pg_start, gt_xd12_regs._r3D.val);
            break;
        case XD12_ADDR_OTP_PROTECT :
            print(LOG_INFO, "PROTECT_EN/DIS - [%u] (0x%03X)\r\n", gt_xd12_regs._r3E.protect_en, gt_xd12_regs._r3E.val);
            break;
        case XD12_ADDR_OTP_OP_MODE :
            print(LOG_INFO, "TEST_EN - [%u], PWM_FULL_O - [%u], MCLK_O - [%u], VREF_O - [%u], ADDR_EXT - [%u] (0x%03X)\r\n",\
            gt_xd12_regs._r3F.test_en, gt_xd12_regs._r3F.pwm_full_o, gt_xd12_regs._r3F.mclk_o, gt_xd12_regs._r3F.vref_o, gt_xd12_regs._r3F.addr_ext, gt_xd12_regs._r3F.val);
            break;
        default :
            continue;
        }
    }
    for(uint8_t addr_offset = 0 ; addr_offset < XD12_ADDR_TRIM_MAX ; ++addr_offset)
    {
        switch (addr_offset)
        {
        case XD12_ADDR_TRIM_OTP_CRC :
            print(LOG_INFO, "OTP_CRC_CHECKSUM - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r00.otp_crc_checksum, gt_xd12_trim_regs._r00.val);
            break;
        case XD12_ADDR_TRIM_OSC :
            print(LOG_INFO, "OSC - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r01.osc, gt_xd12_trim_regs._r01.val);
            break;
        case XD12_ADDR_TRIM_VREF_CTL :
            print(LOG_INFO, "VREF_CTL - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r02.vref_ctl, gt_xd12_trim_regs._r02.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_1 :
            print(LOG_INFO, "ICTL_L_CH01 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r03.ictl_l_ch_x, gt_xd12_trim_regs._r03.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_2 :
            print(LOG_INFO, "ICTL_L_CH02 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r04.ictl_l_ch_x, gt_xd12_trim_regs._r04.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_3 :
            print(LOG_INFO, "ICTL_L_CH03 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r05.ictl_l_ch_x, gt_xd12_trim_regs._r05.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_4 :
            print(LOG_INFO, "ICTL_L_CH04 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r06.ictl_l_ch_x, gt_xd12_trim_regs._r06.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_5 :
            print(LOG_INFO, "ICTL_L_CH05 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r07.ictl_l_ch_x, gt_xd12_trim_regs._r07.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_6 :
            print(LOG_INFO, "ICTL_L_CH06 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r08.ictl_l_ch_x, gt_xd12_trim_regs._r08.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_7 :
            print(LOG_INFO, "ICTL_L_CH07 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r09.ictl_l_ch_x, gt_xd12_trim_regs._r09.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_8 :
            print(LOG_INFO, "ICTL_L_CH08 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0A.ictl_l_ch_x, gt_xd12_trim_regs._r0A.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_9 :
            print(LOG_INFO, "ICTL_L_CH09 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0B.ictl_l_ch_x, gt_xd12_trim_regs._r0B.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_10 :
            print(LOG_INFO, "ICTL_L_CH10 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0C.ictl_l_ch_x, gt_xd12_trim_regs._r0C.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_11 :
            print(LOG_INFO, "ICTL_L_CH11 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0D.ictl_l_ch_x, gt_xd12_trim_regs._r0D.val);
            break;
        case XD12_ADDR_TRIM_ICTL_L_CH_12 :
            print(LOG_INFO, "ICTL_L_CH12 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0E.ictl_l_ch_x, gt_xd12_trim_regs._r0E.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_1 :
            print(LOG_INFO, "ICTL_H_CH01 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1B.ictl_h_ch_x, gt_xd12_trim_regs._r1B.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_2 :
            print(LOG_INFO, "ICTL_H_CH02 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1C.ictl_h_ch_x, gt_xd12_trim_regs._r1C.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_3 :
            print(LOG_INFO, "ICTL_H_CH03 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1D.ictl_h_ch_x, gt_xd12_trim_regs._r1D.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_4 :
            print(LOG_INFO, "ICTL_H_CH04 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1E.ictl_h_ch_x, gt_xd12_trim_regs._r1E.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_5 :
            print(LOG_INFO, "ICTL_H_CH05 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1F.ictl_h_ch_x, gt_xd12_trim_regs._r1F.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_6 :
            print(LOG_INFO, "ICTL_H_CH06 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r20.ictl_h_ch_x, gt_xd12_trim_regs._r20.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_7 :
            print(LOG_INFO, "ICTL_H_CH07 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r21.ictl_h_ch_x, gt_xd12_trim_regs._r21.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_8 :
            print(LOG_INFO, "ICTL_H_CH08 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r22.ictl_h_ch_x, gt_xd12_trim_regs._r22.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_9 :
            print(LOG_INFO, "ICTL_H_CH09 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r23.ictl_h_ch_x, gt_xd12_trim_regs._r23.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_10 :
            print(LOG_INFO, "ICTL_H_CH10 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r24.ictl_h_ch_x, gt_xd12_trim_regs._r24.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_11 :
            print(LOG_INFO, "ICTL_H_CH11 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r25.ictl_h_ch_x, gt_xd12_trim_regs._r25.val);
            break;
        case XD12_ADDR_TRIM_ICTL_H_CH_12 :
            print(LOG_INFO, "ICTL_H_CH12 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r26.ictl_h_ch_x, gt_xd12_trim_regs._r26.val);
            break;
        default :
            continue;
        }
    }
}

static void XD12_OTP_Protect_Disable()
{
    _xd12_otp_protect_t* p_otp_protect = &(gt_xd12_regs._r3E);

    if(OTP_PROTECT_DISABLE != p_otp_protect->protect_en)
    {
        p_otp_protect->protect_en = OTP_PROTECT_DISABLE;
        JigBd_IF_Write_Command(XD12_ADDR_OTP_PROTECT, p_otp_protect->val);
    }
}

static void XD12_OTP_Protect_Enable()
{
    _xd12_otp_protect_t* p_otp_protect = &(gt_xd12_regs._r3E);

    if(OTP_PROTECT_ENABLE != p_otp_protect->protect_en)
    {
        p_otp_protect->protect_en = OTP_PROTECT_ENABLE;
        JigBd_IF_Write_Command(XD12_ADDR_OTP_PROTECT, p_otp_protect->val);
    }
}

static uint8_t XD12_Write_otp_pg_start(uint8_t in_val)
{
    _xd12_otp_rd_prog_t* p_otp_rd_prog = &(gt_xd12_regs._r3D);

    if(in_val > 1)
    {
        print(LOG_ERROR, "XD12_Write_otp_pg_start() input[%d] is Over %d\r\n", in_val, 1);
        return FALSE;
    }
    else
    {
        p_otp_rd_prog->otp_pg_start = in_val;
        XD12_Write_REGISTER(XD12_ADDR_OTP_RD_PROG, p_otp_rd_prog->val, XD12_REG_TYPE_NON_TRIM);
        return TRUE;
    }
}

uint8_t XD12_set_FAULT_LEVEL(dev_max_curr_level_t in_dev_max_curr, short_level_t in_short_level, fb_level_t in_fb_level)
{
    if(in_dev_max_curr > REG_LIMIT_DEV_MAX_CURR_LEVEL || in_short_level > REG_LIMIT_SHORT_LEVEL || in_fb_level > REG_LIMIT_FB_LEVEL)
    {
        print(LOG_ERROR, "Invalid Input[%u, %u, %u] - [%u, %u, %u]\r\n", \
        in_dev_max_curr, in_short_level, in_fb_level, \
        REG_LIMIT_DEV_MAX_CURR_LEVEL, REG_LIMIT_SHORT_LEVEL, REG_LIMIT_FB_LEVEL);
        return FALSE;
    }
    else
    {
        _xd12_fault_level_t* p_xd12_fault_level = &gt_xd12_regs._r06;

        p_xd12_fault_level->dev_max_curr_level = in_dev_max_curr;
        p_xd12_fault_level->short_level = in_short_level;
        p_xd12_fault_level->fb_level = in_fb_level;

        //print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
        XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p_xd12_fault_level->val, XD12_REG_TYPE_NON_TRIM);
        //print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
        //print(LOG_DEBUG, " XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
        return TRUE;
    }
}

uint8_t XD12_set_max_curr_level(dev_max_curr_level_t in_dev_max_curr)
{
    if(in_dev_max_curr > REG_LIMIT_DEV_MAX_CURR_LEVEL)
    {
        print(LOG_ERROR, "Invalid Input[%u] - [%u]\r\n", in_dev_max_curr, REG_LIMIT_DEV_MAX_CURR_LEVEL);
        return FALSE;
    }
    else
    {
        _xd12_fault_level_t* p_xd12_fault_level = &gt_xd12_regs._r06;
        p_xd12_fault_level->dev_max_curr_level = in_dev_max_curr;

        XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p_xd12_fault_level->val, XD12_REG_TYPE_NON_TRIM);
        return TRUE;
    }
}

uint8_t XD12_set_CHANNEL_ENABLE(uint8_t in_channel)
{
    _xd12_channel_enable_t* p_xd12_channel_enable = &gt_xd12_regs._r03;

    if(in_channel < CH_MAX)
    {
        // CHANNEL ENABLE
        p_xd12_channel_enable->val = (1 << in_channel);
        XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, p_xd12_channel_enable->val, XD12_REG_TYPE_NON_TRIM);
        return TRUE;
    }
    else
    {
        print(LOG_ERROR, "ERROR:input is over CH_MAX: XD12_set_CHANNEL_ENABLE(%d)\r\n", in_channel);
        return FALSE;
    }
}

void XD12_set_CHANNEL_DISABLE_All(void)
{
    _xd12_channel_enable_t* p_xd12_channel_enable = &gt_xd12_regs._r03;
    p_xd12_channel_enable->val = 0x00;
    XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, p_xd12_channel_enable->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_set_CHANNEL_ENABLE_All(void)
{
    _xd12_channel_enable_t* p_xd12_channel_enable = &gt_xd12_regs._r03;
    p_xd12_channel_enable->val = 0xFFF;
    XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, p_xd12_channel_enable->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_set_delay_ch(void)
{
    uint16_t delay_per_ch = 0;
    delay_per_ch = (uint16_t)(gn_xd_pwm_max_size / gn_xd_ch_size);
    for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
    {
        uint16_t delay_ch = delay_per_ch * ch;
        uint16_t delay_lsb = ((delay_ch & 0x0FFF) >>  0);
        uint16_t delay_msb = ((delay_ch & 0x3000) >> 12);
        delay_msb <<= (2 * (ch % 6));

        TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH01 + ch, delay_lsb, XD12_REG_TYPE_NON_TRIM);
        TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_1 + (ch / 6), delay_msb, XD12_REG_TYPE_NON_TRIM);
        #if 0
            switch (ch)
            {
            case 0 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH01, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_1, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 1 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH02, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_1, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 2 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH03, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_1, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 3 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH04, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_1, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 4 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH05, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_1, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 5 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH06, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_1, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 6 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH07, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_2, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 7 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH08, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_2, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 8 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH09, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_2, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 9 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH10, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_2, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 10 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH11, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_2, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            case 11 :
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH12, delay_lsb, XD12_REG_TYPE_NON_TRIM);
                TargetIC_IF_Write_Register(XD12_ADDR_DELAY_CH_EXTEND_2, delay_msb, XD12_REG_TYPE_NON_TRIM);
                break;
            }
        #endif
    }
}

static void XD12_set_serial_clock_gen(void)
{
    _xd12_serial_clock_gen_t* p_xd12_serial_clock_gen = &gt_xd12_regs._r25;

    p_xd12_serial_clock_gen->serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
    p_xd12_serial_clock_gen->serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
    XD12_Write_REGISTER(XD12_ADDR_SERIAL_CLOCK_GEN, p_xd12_serial_clock_gen->val, XD12_REG_TYPE_NON_TRIM);
}

static void XD12_set_test_en(bool in_Enable)
{
    _xd12_op_mode_t* p_op_mode = &(gt_xd12_regs._r3F);

    if(in_Enable == TRUE)
    {
        p_op_mode->test_en = 1;
    }
    else
    {
        p_op_mode->test_en = 0;
    }
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_op_mode->val, XD12_REG_TYPE_NON_TRIM);
}

static void XD12_set_pwm_full_o(bool in_Enable)
{
    _xd12_op_mode_t* p_op_mode = &(gt_xd12_regs._r3F);

    if(in_Enable == TRUE)
    {
        p_op_mode->pwm_full_o = 1;
    }
    else
    {
        p_op_mode->pwm_full_o = 0;
    }
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_op_mode->val, XD12_REG_TYPE_NON_TRIM);
}

static void XD12_set_mclk_o(bool in_Enable)
{
    _xd12_op_mode_t* p_op_mode = &(gt_xd12_regs._r3F);

    if(in_Enable == TRUE)
    {
        p_op_mode->mclk_o = 1;
    }
    else
    {
        p_op_mode->mclk_o = 0;
    }
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_op_mode->val, XD12_REG_TYPE_NON_TRIM);
}

static void XD12_set_vref_o(bool in_Enable)
{
    _xd12_op_mode_t* p_op_mode = &(gt_xd12_regs._r3F);

    if(in_Enable == TRUE)
    {
        p_op_mode->vref_o = 1;
    }
    else
    {
        p_op_mode->vref_o = 0;
    }
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_op_mode->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_set_max_curr_vref(uint16_t n_vref)
{
    _xd12_max_current_vref_t* p_xd12_max_current_vref = &(gt_xd12_regs._r08);

    if (n_vref < 4096)
    {
        p_xd12_max_current_vref->max_curr_vref = n_vref;
        XD12_Write_REGISTER(XD12_ADDR_MAX_CURR_VREF, p_xd12_max_current_vref->val, XD12_REG_TYPE_NON_TRIM);
    }
}

static void XD12_set_osc_manual_en(bool in_Enable)
{
    _xd12_osc_fll_manual_2_t *p_xd12_osc_fll_manual_2 = &(gt_xd12_regs._r2B);

    if (in_Enable == TRUE)
    {
        p_xd12_osc_fll_manual_2->osc_man_e = 1;
        XD12_Write_REGISTER(XD12_ADDR_OSC_FLL_MANUAL_2, p_xd12_osc_fll_manual_2->val, XD12_REG_TYPE_NON_TRIM);
    }
    else
    {
        p_xd12_osc_fll_manual_2->osc_man_e = 0;
        XD12_Write_REGISTER(XD12_ADDR_OSC_FLL_MANUAL_2, p_xd12_osc_fll_manual_2->val, XD12_REG_TYPE_NON_TRIM);
    }
}

static void XD12_set_osc_manual(uint16_t n_osc_fll_man)
{
    _xd12_osc_fll_manual_1_t *p_xd12_osc_fll_manual_1 = &(gt_xd12_regs._r2A);
    _xd12_osc_fll_manual_2_t *p_xd12_osc_fll_manual_2 = &(gt_xd12_regs._r2B);

    uint16_t osc_fll_man_lsb = ((n_osc_fll_man & 0x0FFF) >>  0);
    uint16_t osc_fll_man_msb = ((n_osc_fll_man & 0xF000) >> 12);

    p_xd12_osc_fll_manual_1->osc_fll_man = osc_fll_man_lsb;
    XD12_Write_REGISTER(XD12_ADDR_OSC_FLL_MANUAL_1, p_xd12_osc_fll_manual_1->val, XD12_REG_TYPE_NON_TRIM);

    p_xd12_osc_fll_manual_2->osc_fll_man = osc_fll_man_msb;
    p_xd12_osc_fll_manual_2->osc_man_e = 1;
    XD12_Write_REGISTER(XD12_ADDR_OSC_FLL_MANUAL_2, p_xd12_osc_fll_manual_2->val, XD12_REG_TYPE_NON_TRIM);
}

static void XD12_Read_All(void)
{
    uint8_t u8_addr = 0;
    uint16_t u16_data = 0;

    /* Read XD12-Normal Registers */
    for(u8_addr = XD12_ADDR_RESET_ID ; u8_addr < XD12_ADDR_MAX ; ++u8_addr)
    {
        u16_data = XD12_Read_REGISTER(u8_addr, XD12_REG_TYPE_NON_TRIM);
        *(gt_xd12_regs.ALL + u8_addr) = u16_data;
    }

    /* Read XD12-TRIM Registers */
    for(u8_addr = XD12_ADDR_TRIM_OTP_CRC ; u8_addr < XD12_ADDR_TRIM_MAX ; ++u8_addr)
    {
        u16_data = XD12_Read_REGISTER(u8_addr, XD12_REG_TYPE_TRIM);
        *(gt_xd12_trim_regs.ALL + u8_addr) = u16_data;
    }
    gt_xd12_regs._r3F.addr_ext = 0;
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, gt_xd12_regs._r3F.val, XD12_REG_TYPE_NON_TRIM);
}

static uint8_t XD12_Write_OSC(uint16_t in_val)
{
    _xd12_osc_t* p_xd12_osc = &(gt_xd12_trim_regs._r01);
    if(in_val > REG_LIMIT_OSC) // 6-bit
    {
        print(LOG_ERROR, "XD12_Write_OSC() input[%d] is Over %d\r\n", in_val, REG_LIMIT_OSC);
        return FALSE;
    }
    else
    {
        p_xd12_osc->osc = in_val;
        XD12_Write_REGISTER(XD12_ADDR_TRIM_OSC, p_xd12_osc->val, XD12_REG_TYPE_TRIM);
        return TRUE;
    }
}

static uint8_t XD12_Write_VREF_CTL(uint16_t in_val)
{
    _xd12_vref_ctl_t* p_xd12_vref_ctl = &(gt_xd12_trim_regs._r02);
    if(in_val > REG_LIMIT_VREF) // 6-bit
    {
        print(LOG_ERROR, "XD12_Write_VREF_CTL() input[%d] is Over %d\r\n", in_val, REG_LIMIT_VREF);
        return FALSE;
    }
    else
    {
        p_xd12_vref_ctl->vref_ctl = in_val;
        XD12_Write_REGISTER(XD12_ADDR_TRIM_VREF_CTL, p_xd12_vref_ctl->val, XD12_REG_TYPE_TRIM);
        return TRUE;
    }
}

static uint8_t XD12_Write_ICTL_L_CHx(uint8_t in_ch, uint16_t in_val)
{
    _xd12_ictl_l_ch_t* p_xd12_ictl_l_ch = &gt_xd12_trim_regs._r03 + in_ch;
    if(in_ch > CH_MAX || in_val > REG_LIMIT_ICTL_L)
    {
        print(LOG_ERROR, "XD12_Write_ICTL_L_CHx%d() input[%d] is Over %d\r\n", in_ch+1, in_val, REG_LIMIT_ICTL_L);
        return FALSE;
    }
    else
    {
        p_xd12_ictl_l_ch->ictl_l_ch_x = in_val;
        XD12_Write_REGISTER(XD12_ADDR_TRIM_ICTL_L_CH_1 + in_ch, p_xd12_ictl_l_ch->val, XD12_REG_TYPE_TRIM);
        return TRUE;
    }
}

static uint8_t XD12_Write_ICTL_H_CHx(uint8_t in_ch, uint16_t in_val)
{
    _xd12_ictl_h_ch_t* p_xd12_ictl_h_ch = &gt_xd12_trim_regs._r1B + in_ch;
    if(in_ch > CH_MAX || in_val > REG_LIMIT_ICTL_H)
    {
        print(LOG_ERROR, "XD12_Write_ICTL_H_CHx%d() input[%d] is Over %d\r\n",in_ch+1,in_val, REG_LIMIT_ICTL_H);
        return FALSE;
    }
    else
    {
        p_xd12_ictl_h_ch->ictl_h_ch_x = in_val;
        XD12_Write_REGISTER(XD12_ADDR_TRIM_ICTL_H_CH_1 + in_ch, p_xd12_ictl_h_ch->val, XD12_REG_TYPE_TRIM);
        return TRUE;
    }
}

/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/
void TargetIC_IF_Calculate_Trim_Spec(void)
{
    for(trim_mode_t mode = TRIM_VREF_CTL ; mode < TRIM_MAX ; ++mode)
    {
        p_gui_param p = trim_get_param_gui();

        switch(mode)
        {
        case TRIM_VREF_CTL:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_VREF_TARGET * (1 - XD12_ERR_RATE));
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_VREF_TARGET * (1 + XD12_ERR_RATE));
            p[mode][TRIM_PARA_P1] = 0;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_OSC_FREQUENCY:
            p[mode][TRIM_PARA_TARGET_MIN] = (38.5f);//(XD12_OSC_TARGET);
            p[mode][TRIM_PARA_TARGET_MAX] = (39.5f);//(XD12_OSC_TARGET + 1.5f);
            p[mode][TRIM_PARA_P1] = 0;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_ICTL_L_CHS:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_ICTL_L_TARGET * (1 - XD12_ICTL_L_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_ICTL_L_TARGET * (1 + XD12_ICTL_L_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_P1] = XD12_ICTL_L_P1;
            p[mode][TRIM_PARA_P2] = XD12_ICTL_L_P2;
            break;
        case TRIM_ICTL_H_CHS:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_ICTL_H_TARGET * (1 - XD12_ICTL_H_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_ICTL_H_TARGET * (1 + XD12_ICTL_H_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_P1] = XD12_ICTL_H_P1;
            p[mode][TRIM_PARA_P2] = XD12_ICTL_H_P2;
            break;
        }
    }
}

void TargetIC_IF_Trim_Reg_Init(void)
{
    XD12_Write_REGISTER(XD12_ADDR_OTP_PROTECT, 0xA5A, XD12_REG_TYPE_NON_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OSC, gn_xd_trim_reg[XD12_ADDR_TRIM_OSC], XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_VREF_CTL, gn_xd_trim_reg[XD12_ADDR_TRIM_VREF_CTL], XD12_REG_TYPE_TRIM);

    for (uint8_t ch = 0 ; ch < CH_MAX ; ++ch)
    {
        XD12_Write_REGISTER(XD12_ADDR_TRIM_ICTL_L_CH_1 + ch, gn_xd_trim_reg[XD12_ADDR_TRIM_ICTL_L_CH_1 + ch], XD12_REG_TYPE_TRIM);
        XD12_Write_REGISTER(XD12_ADDR_TRIM_ICTL_H_CH_1 + ch, gn_xd_trim_reg[XD12_ADDR_TRIM_ICTL_H_CH_1 + ch], XD12_REG_TYPE_TRIM);
    }
}

void TargetIC_IF_Init_Register(void)
{
    TargetIC_IF_TrimRegister_Set(0, TRIM_OSC_FREQUENCY, XD12_DEFAULT_OSC);
    TargetIC_IF_TrimRegister_Set(0, TRIM_VREF_CTL, XD12_DEFAULT_VREF_CTL);

    for(uint8_t i_channel = 0 ; i_channel < CH_MAX ; ++i_channel)
    {
        TargetIC_IF_TrimRegister_Set(i_channel, TRIM_ICTL_L_CHS, XD12_DEFAULT_ICTL_L);
    }

    for(uint8_t i_channel = 0 ; i_channel < CH_MAX ; ++i_channel)
    {
        TargetIC_IF_TrimRegister_Set(i_channel, TRIM_ICTL_H_CHS, XD12_DEFAULT_ICTL_H);
    }
}

void TargetIC_IF_XD_Regs_Read_Display(void)
{
    XD12_Read_All();
    XD12_Print_XD12_REGS();
}

void TargetIC_IF_Save_Trim_Reg(void)
{
    for (uint8_t addr = XD12_ADDR_TRIM_OTP_CRC ; addr < XD12_ADDR_TRIM_MAX ; ++addr)
    {
        gn_xd_trim_reg[addr] = TargetIC_IF_Read_Register(addr, XD12_REG_TYPE_TRIM);
    }

    print(LOG_INFO, "osc,%3u\r\n", gn_xd_trim_reg[1]);
    print(LOG_INFO, "vref,%3u\r\n", gn_xd_trim_reg[2]);

    print(LOG_INFO, "ictl_l,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n"
    , gn_xd_trim_reg[ 3], gn_xd_trim_reg[ 4], gn_xd_trim_reg[ 5] , gn_xd_trim_reg[ 6], gn_xd_trim_reg[ 7], gn_xd_trim_reg[ 8]
    , gn_xd_trim_reg[ 9], gn_xd_trim_reg[10], gn_xd_trim_reg[11] , gn_xd_trim_reg[12], gn_xd_trim_reg[13], gn_xd_trim_reg[14]
    );
    print(LOG_INFO, "ictl_h,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n"
    , gn_xd_trim_reg[27], gn_xd_trim_reg[28], gn_xd_trim_reg[29] , gn_xd_trim_reg[30], gn_xd_trim_reg[31], gn_xd_trim_reg[32]
    , gn_xd_trim_reg[33], gn_xd_trim_reg[34], gn_xd_trim_reg[35] , gn_xd_trim_reg[36], gn_xd_trim_reg[37], gn_xd_trim_reg[38]
    );
}

uint64_t TargetIC_IF_Compare_Trim_Regs(void)
{
    uint64_t ret = 0;
    uint16_t u16_reg_val = 0;

    XD12_Select_Reg_Type(XD12_REG_TYPE_TRIM);

    for (xd12_trim_addr_t trim_addr = XD12_ADDR_TRIM_OSC ; trim_addr < XD12_ADDR_TRIM_MAX ; ++trim_addr)
    {
        u16_reg_val = JigBd_IF_Read_Command(trim_addr);
        if (gn_xd_trim_reg[trim_addr] != u16_reg_val)
        {
            ret |= ((uint64_t)1 << trim_addr);
            print(LOG_INFO, "%s %s - NG", FONT_RED, gs_xd12_addr_trim_str[trim_addr]);
        }
        else
        {
            print(LOG_INFO, "%s %s - OK", FONT_GREEN, gs_xd12_addr_trim_str[trim_addr]);
        }
        print(LOG_INFO, "   [0x%03X] - [0x%03X] %s\r\n", gn_xd_trim_reg[trim_addr], u16_reg_val, FONT_NONE);
    }

    return ret;
}

void TargetIC_IF_Write_OTP_Start(void)
{
    XD12_OTP_Protect_Disable();

    /* Write OTP 0x26 ~ 0x3F *************/
    for(xd12_trim_addr_t addr = XD12_ADDR_TRIM_OSC ; addr < XD12_ADDR_TRIM_MAX ; ++addr)
    {
        XD12_Write_REGISTER(addr, *(gt_xd12_trim_regs.ALL + addr), XD12_REG_TYPE_TRIM);
    }

    // otp_pg_start : 1
    XD12_Write_otp_pg_start(1);
}

void TargetIC_IF_Write_OTP_End(void)
{
    // otp_pg_start : 0
    XD12_Write_otp_pg_start(0);

    // OTP PROTECT - Enable
    XD12_OTP_Protect_Enable();
}

void TargetIC_IF_Write_Register(uint8_t in_addr, uint16_t in_data, XD12_REG_TYPE n_reg_type)
{
    XD12_Write_REGISTER(in_addr, in_data, n_reg_type);
}

uint16_t TargetIC_IF_Read_Register(uint8_t in_addr, XD12_REG_TYPE n_reg_type)
{
    uint16_t u16_rtn_val = 0;
    u16_rtn_val = XD12_Read_REGISTER(in_addr, n_reg_type);
    return u16_rtn_val;
}

uint16_t TargetIC_IF_Get_Register(uint8_t in_addr, XD12_REG_TYPE n_reg_type)
{
    uint16_t u16_rtn_val = 0;
    u16_rtn_val = XD12_Get_REGISTER(in_addr, n_reg_type);
    return u16_rtn_val;
}

uint16_t TargetIC_IF_TrimRegister_Get(uint8_t ch_num, trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0xFFFF;
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        {
            _xd12_osc_t* p_xd12_osc = &gt_xd12_trim_regs._r01;
            rtn_val = p_xd12_osc->val;
        }
        break;

    case TRIM_VREF_CTL:
        {
            _xd12_vref_ctl_t* p_xd12_vref_ctl = &gt_xd12_trim_regs._r02;
            rtn_val = p_xd12_vref_ctl->val;
        }
        break;

    case TRIM_ICTL_L_CHS:
        {
            _xd12_ictl_l_ch_t* p_xd12_ictl_l_ch_x = &gt_xd12_trim_regs._r03 + ch_num;
            rtn_val = p_xd12_ictl_l_ch_x->val;
        }
        break;

    case TRIM_ICTL_H_CHS:
        {
            _xd12_ictl_h_ch_t* p_xd12_ictl_h_ch_x = &gt_xd12_trim_regs._r1B + ch_num;
            rtn_val = p_xd12_ictl_h_ch_x->val;
        }
        break;
    }

    return rtn_val;
}

uint8_t TargetIC_IF_TrimRegister_Set(uint8_t ch_num, trim_mode_t in_trim_mode, uint16_t in_reg_val)
{
    uint8_t rtn_val = FALSE;
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        if(in_reg_val > REG_LIMIT_OSC)
        {
            print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set TRIM_OSC_FREQUENCY- in_reg_val(%d) Over !!\r\n", in_reg_val);
        }
        else
        {
            rtn_val = XD12_Write_OSC(in_reg_val);
        }
        break;

    case TRIM_VREF_CTL:
        if(in_reg_val > REG_LIMIT_VREF)
        {
            print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set TRIM_VREF_CTL- in_reg_val(%d) Over !!\r\n", in_reg_val);
        }
        else
        {
            rtn_val = XD12_Write_VREF_CTL(in_reg_val);
        }
        break;

    case TRIM_ICTL_L_CHS:
        if(in_reg_val > REG_LIMIT_ICTL_L)
        {
            print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set TRIM_ICTL_L_CHS- in_reg_val(%d) Over !!\r\n", in_reg_val);
        }
        else
        {
            rtn_val = XD12_Write_ICTL_L_CHx(ch_num, in_reg_val);
        }
        break;

    case TRIM_ICTL_H_CHS:
        if(in_reg_val > REG_LIMIT_ICTL_H)
        {
            print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set TRIM_ICTL_H_CHS- in_reg_val(%d) Over !!\r\n", in_reg_val);
        }
        else
        {
            rtn_val = XD12_Write_ICTL_H_CHx(ch_num, in_reg_val);
        }
        break;
    }

    if(rtn_val == FALSE)
    {
        print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set(%d, %d, %d) - FALSE RETRUN\r\n", ch_num, in_trim_mode, in_reg_val);
    }
    return rtn_val;
}

uint16_t TargetIC_IF_TrimRegister_Limit_Get(uint8_t ch_num, trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0;
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        rtn_val = REG_LIMIT_OSC;
        //print(LOG_DEBUG, "OSC_LIMIT : %d\r\n", rtn_val);
        break;

    case TRIM_VREF_CTL:
        rtn_val = REG_LIMIT_VREF;
        //print(LOG_DEBUG, "VREF_LIMIT : %d\r\n", rtn_val);
        break;

    case TRIM_ICTL_L_CHS:
        rtn_val = REG_LIMIT_ICTL_L;
        //print(LOG_DEBUG, "ICTL_L_CH_%02d_LIMIT : %d\r\n", ch_num + 1, rtn_val);
        break;

    case TRIM_ICTL_H_CHS:
        rtn_val = REG_LIMIT_ICTL_H;
        //print(LOG_DEBUG, "ICTL_H_CH_%02d_LIMIT : %d\r\n", ch_num + 1, rtn_val);
        break;
    }
    return rtn_val;
}

void TargetIC_IF_TRIM_Init_VREF_CTL(void)
{
    XD12_set_vref_o(TRUE);
    XD12_set_mclk_o(FALSE);
    XD12_set_pwm_full_o(FALSE);

    XD12_set_max_curr_vref(4095);
}

void TargetIC_IF_TRIM_Init_OSC(void)
{
    XD12_set_vref_o(FALSE);
    XD12_set_mclk_o(TRUE);
    XD12_set_pwm_full_o(FALSE);

    XD12_set_osc_manual_en(TRUE);
    XD12_set_osc_manual(32768);
}

void TargetIC_IF_TRIM_Init_ICTL_L(void)
{
    XD12_set_vref_o(FALSE);
    XD12_set_mclk_o(FALSE);
    XD12_set_pwm_full_o(TRUE);

    XD12_set_max_curr_vref(XD12_CURRENT_TRIM_VREF);
}

void TargetIC_IF_TRIM_Init_ICTL_H(void)
{
    XD12_set_vref_o(FALSE);
    XD12_set_mclk_o(FALSE);
    XD12_set_pwm_full_o(TRUE);

    XD12_set_max_curr_vref(XD12_CURRENT_TRIM_VREF);
}

void TargetIC_IF_SEND_otp_written(void)
{
    uint16_t* p_xd12_reg_addr = NULL;
    p_xd12_reg_addr = gt_xd12_trim_regs.ALL;

    for(xd12_trim_addr_t addr = XD12_ADDR_TRIM_OSC ; addr < XD12_ADDR_TRIM_MAX ; ++addr)
    {
        print(LOG_INFO, ":%02x:%03x", addr, *(p_xd12_reg_addr + addr));
    }
}

void TargetIC_IF_Test_Enable(void)
{
    _xd12_op_mode_t* p_xd12_op_mode = &gt_xd12_regs._r3F;

    p_xd12_op_mode->test_en = 1;

    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_xd12_op_mode->val, XD12_REG_TYPE_NON_TRIM);
}

float XD12_get_max_current_level(void)
{
    float f_rtn = 0.0f;
    dev_max_curr_level_t dev_max_curr_lvl = (dev_max_curr_level_t)0;

    _xd12_fault_level_t xd12_fault_level = {0, };
    xd12_fault_level.val = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_LEVEL, XD12_REG_TYPE_NON_TRIM);
    dev_max_curr_lvl = (dev_max_curr_level_t)xd12_fault_level.dev_max_curr_level;

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

void XD12_detect_FBO(void)
{
    static uint16_t prev_fbo = 0xFFFF;
    uint16_t now_fbo = XD_FBO_READ();

    if (prev_fbo != now_fbo)
    {
        if (now_fbo)
        {
            print(LOG_INFO, "\r\n FBO --> HI\r\n");
        }
        else
        {
            print(LOG_INFO, "\r\n FBO --> LOW\r\n");
        }
        prev_fbo = now_fbo;
    }
}

static void XD12_reg_value_init()
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

    //gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_4mA;
    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_24mA;
    gt_xd_short_level = SHORT_LEVEL_6V;
    gt_xd_fb_level = FB_LEVEL_0V7;
}

void XD12_Initialize(void)
{
    XD12_reg_value_init();

    JigBd_IF_XD12_reset_Command();

    JigBd_IF_IdGen_Command();

    for (xd12_addr_t xd12_addr = XD12_ADDR_RESET_ID ; xd12_addr < XD12_ADDR_MAX ; ++xd12_addr)
    {
        uint16_t* p_xd_reg_base_addr = gt_xd12_regs.ALL;

        switch (xd12_addr)
        {
        case XD12_ADDR_LD_CONTROL :
            gt_xd12_regs._r01.ld_dir = XD_LD_DIR_TAIL_SHIFT;
            gt_xd12_regs._r01.pwm_res = gn_xd_pwm_res;
            gt_xd12_regs._r01.over_to_e = 1;
            gt_xd12_regs._r01.scan_no = gn_xd_scan_no;
            gt_xd12_regs._r01.ext_vs_pol = 0;
            gt_xd12_regs._r01.ext_vs_e = XD_VSYNC_EXTERNAL;
            gt_xd12_regs._r01.ch_size = gn_xd_ch_size;
            break;
        case XD12_ADDR_FPWM_DIVIDER :
            gt_xd12_regs._r02.fpwm_div = gn_xd_fpwm_div;
            break;
        case XD12_ADDR_CHANNEL_ENABLE :
            gt_xd12_regs._r03.val = 0xFFF;
            break;
        case XD12_ADDR_FAULT_LEVEL :
            gt_xd12_regs._r06.fb_level = gt_xd_fb_level;
            gt_xd12_regs._r06.short_level = gt_xd_short_level;
            gt_xd12_regs._r06.dev_max_curr_level = gt_xd_dev_max_curr_level;
            break;
        case XD12_ADDR_FAULT_CONTROL :
            gt_xd12_regs._r07.o_off_e = 0;
            gt_xd12_regs._r07.s_off_e = 0;
            gt_xd12_regs._r07.t_off_e = 0;
            gt_xd12_regs._r07.s_det_e = 1;
            gt_xd12_regs._r07.o_det_e = 1;
            gt_xd12_regs._r07.o_fb_e = 0;
            gt_xd12_regs._r07.ms_vs_det_e = 0;
            gt_xd12_regs._r07.ms_vs_dimm = 0;
            gt_xd12_regs._r07.ms_vs_lock = 0;
            gt_xd12_regs._r07.fbo_e = 0;
            gt_xd12_regs._r07.timeout_e = 1;
            break;
        case XD12_ADDR_MAX_CURR_VREF :
            gt_xd12_regs._r08.max_curr_vref = 0xFFF;
            break;
        case XD12_ADDR_SERIAL_CLOCK_GEN :
            gt_xd12_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
            gt_xd12_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
            break;
        case XD12_ADDR_SERIAL_LATENCY :
            gt_xd12_regs._r26.serial_latency = 60;
            break;
        case XD12_ADDR_MCLK_LOCK_1 :
            gt_xd12_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0x000FFF) >>  0);
            break;
        case XD12_ADDR_MCLK_LOCK_2 :
            gt_xd12_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0xFFF000) >> 12);
            gt_xd12_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_ENABLE;
            break;
        case XD12_ADDR_TEMP :
            gt_xd12_regs._r29.flt_gain = 0;
            gt_xd12_regs._r29.ofs_temp = 8;
            break;
        case XD12_ADDR_OSC_FLL_MANUAL_1 :
            gt_xd12_regs._r2A.osc_fll_man = 0x000;
            break;
        case XD12_ADDR_OSC_FLL_MANUAL_2 :
            gt_xd12_regs._r2B.osc_fll_man = 8;
            gt_xd12_regs._r2B.osc_man_e = 0;
            break;
        default :
            continue;
        }
        TargetIC_IF_Write_Register(xd12_addr, *(p_xd_reg_base_addr + xd12_addr), XD12_REG_TYPE_NON_TRIM);
    }
    //XD12_set_delay_ch();

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = XD_FB_IN_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(XD_FB_IN_GPIO_Port, &GPIO_InitStruct);
}

void XD12_Trim_Initialize(void)
{
    JigBd_IF_XD12_reset_Command();

    JigBd_IF_IdGen_Command();

    XD12_set_serial_clock_gen();

    XD12_set_test_en(TRUE);

    XD12_OTP_Protect_Disable();

    gt_xd_fb_level = FB_LEVEL_0V7;
    gt_xd_short_level = SHORT_LEVEL_36V;
    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_8mA;

    gt_xd12_regs._r06.fb_level = gt_xd_fb_level;
    gt_xd12_regs._r06.short_level = gt_xd_short_level;
    gt_xd12_regs._r06.dev_max_curr_level = gt_xd_dev_max_curr_level;
    TargetIC_IF_Write_Register(XD12_ADDR_FAULT_LEVEL, gt_xd12_regs._r06.val, XD12_REG_TYPE_NON_TRIM);

    XD12_set_osc_manual_en(TRUE);
    XD12_set_osc_manual(32768);
}


void vsync_update_frequency(float n_freq)
{
    uint32_t period = 0;
    uint32_t prescale = 0;

    prescale = LL_TIM_GetPrescaler(TIM8);
    period = (uint32_t)((TIM8_FREQ / (prescale + 1)) / n_freq - 1 + 0.5f);

    LL_TIM_SetAutoReload(TIM8, period);

    gf_vsync_out = n_freq;

    // 1. disable mclk_lock_cnt_e
    gt_xd12_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_DISABLE;
    TargetIC_IF_Write_Register(XD12_ADDR_MCLK_LOCK_2, gt_xd12_regs._r28.val, XD12_REG_TYPE_NON_TRIM);

    // 2. increase or decrease fpwm_div
    if (gn_xd_pwm_res == XD_PWM_RES_12BIT)
    {
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 12)));
    }
    else //if (gn_xd_pwm_res == XD_PWM_RES_14BIT)
    {
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 14)));
    }
    gt_xd12_regs._r02.fpwm_div = gn_xd_fpwm_div;
    TargetIC_IF_Write_Register(XD12_ADDR_FPWM_DIVIDER, gt_xd12_regs._r02.val, XD12_REG_TYPE_NON_TRIM);

    // 3. change mclk_lock_cnt
    gn_xd_mclk_lock_cnt = (uint32_t)((XD_MCLK_LOCK_CNT_120Hz) * (VSYNC / gf_vsync_out) + 0.5f);
    gt_xd12_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0x000FFF) >>  0);
    gt_xd12_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0xFFF000) >> 12);

    TargetIC_IF_Write_Register(XD12_ADDR_MCLK_LOCK_1, gt_xd12_regs._r27.val, XD12_REG_TYPE_NON_TRIM);
    TargetIC_IF_Write_Register(XD12_ADDR_MCLK_LOCK_2, gt_xd12_regs._r28.val, XD12_REG_TYPE_NON_TRIM);

    // 4. enable mclk_lock_cnt_e
    gt_xd12_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_ENABLE;
    TargetIC_IF_Write_Register(XD12_ADDR_MCLK_LOCK_2, gt_xd12_regs._r28.val, XD12_REG_TYPE_NON_TRIM);
}

/* END - INTERFACE FUNCTIONS ************************************************************************/

/* BEGIN - DBG_TEST SUPPORT *************************************************************/
#ifdef DBG_TEST
void _dbg_XD12_Print_RegisterMap(void)
{
#if _DBG_XD12
    for(xd12_addr_t addr = XD12_ADDR_RESET_ID ; addr < XD12_ADDR_MAX ; ++addr)
    {
        print(LOG_DEBUG, "0x%02X-XD12_ADDR_%s\r\n", addr, gs_xd12_addr_str[addr]);
    }

    for(xd12_trim_addr_t addr = XD12_ADDR_TRIM_OTP_CRC ; addr < XD12_ADDR_TRIM_MAX ; ++addr)
    {
        print(LOG_DEBUG, "0x%02X-XD12_ADDR_%s\r\n", addr, gs_xd12_addr_trim_str[addr]);
    }
#endif
}

void _dbg_XD12_Detect(void)
{
    // Read Freq
    JigBd_IF_TIM_Capture_Start();
    us_tdelay(50 * 1000);
    us_tdelay(50 * 1000);
    JigBd_IF_TIM_Capture_Stop();
    print(LOG_DEBUG, "_dbg_XD12_Detect - test_en DISABLE - Read Freq : %d\r\n", JigBd_IF_Freq_Get());

    // Power ON
    JigBd_IF_XD_VCC_EN(PWR_ON);
    JigBd_IF_XD_VCC_Level(PWR_ON_5V0);

    XD12_Read_All();

    // ID Gen
    JigBd_IF_IdGen_Command();

    // Enable test_en
    XD12_set_test_en(TRUE);

    // Read Freq
    JigBd_IF_TIM_Capture_Start();
    us_tdelay(50 * 1000);
    us_tdelay(50 * 1000);
    JigBd_IF_TIM_Capture_Stop();
    print(LOG_DEBUG, "_dbg_XD12_Detect - test_en ENABLE - Read Freq : %d\r\n", JigBd_IF_Freq_Get());
}

void _dbg_XD12_Test(void)
{
    _dbg_XD12_Print_RegisterMap();
    TargetIC_IF_SEND_otp_written();
}
#endif //DBG_TEST
/* END - DBG TEST ******************************************************************************/

/*** end of file ***/
