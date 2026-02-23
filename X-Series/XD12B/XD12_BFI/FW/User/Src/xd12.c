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
#define XD12_OSC_TARGET         (14.750) /* MHz */
#define XD12_VREF_TARGET        (2.2)     /* V */

/* OFFSET P1 */
#define XD12_OFS_ERR_RATE       (0.5/100) /* +/-% */
#define XD12_OFS_TARGET         (0.1250f)   /* mA */
#define XD12_OFS_P1             (1 << 11)
#define XD12_OFS_P2             (3 << 11)

/* OFFSET P2 */
#define XD12_GAIN_ERR_RATE      (0.5/100) /* +/-% */
#define XD12_GAIN_TARGET        (0.3750f)  /* mA */
#define XD12_GAIN_P1            (6 << 11)
#define XD12_GAIN_P2            (0 << 11)

#define XD12_DEFAULT_OSC        (0x40)
#define XD12_DEFAULT_VREF_CTL   (0x10)
#define XD12_DEFAULT_OFS        (0x40)
#define XD12_DEFAULT_GAIN       (0x40)

#define XD_MCLK                 (14750000.0f)
#define VSYNC                   (120.0f)

#define TIM8_FREQ               (144000000.0f)

#define XD_SUB_FRAME_SIZE       (64)

#define XD_LD_DIR_HEAD_SHIFT    (0)
#define XD_LD_DIR_TAIL_SHIFT    (1)

#define XD_LD_MODE_NORMAL       (0)
#define XD_LD_MODE_X8           (1)

#ifdef __XD12_C__

static const char* gs_xd12_addr_str[XD12_ADDR_MAX] =
{
    "RESET_ID",
    "LD_MODE",
    "SF_PERIOD",
    "SF_SIZE",
    "CH_SIZE_SF_X8_SIZE",
    "SF_BFI_SIZE",
    "LD_FIX_1",
    "LD_FIX_2",
    "MAX_CURRENT_VREF",
    "CHANNEL_ENABLE",
    "DUMMY",
    "FAULT_STATUS",
    "FAULT_LEVEL",
    "FAULT_MODE",
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

    "SERIAL_CLOCK_GEN",
    "LD_CONTROL",
    "DCLK_PERIOD",
    "MCLK_LOCK_1",
    "MCLK_LOCK_2",
    "OSC_COMP",
    "OSC_COMP_TH_P",
    "OSC_COMP_TH_N",

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

static const char* gs_xd12_addr_trim_str[XD12_ADDR_TRIM_MAX] =
{
    "OTP_CRC",
    "OSC",
    "VREF_CTL",
    "OFS_CH1",
    "OFS_CH2",
    "OFS_CH3",
    "OFS_CH4",
    "OFS_CH5",
    "OFS_CH6",
    "OFS_CH7",
    "OFS_CH8",
    "OFS_CH9",
    "OFS_CH10",
    "OFS_CH11",
    "OFS_CH12",
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
    "GAIN_CH1",
    "GAIN_CH2",
    "GAIN_CH3",
    "GAIN_CH4",
    "GAIN_CH5",
    "GAIN_CH6",
    "GAIN_CH7",
    "GAIN_CH8",
    "GAIN_CH9",
    "GAIN_CH10",
    "GAIN_CH11",
    "GAIN_CH12",
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

/* Variable for XD Registers */
static float gf_xd_mclk;
static float gf_vsync_out;
static uint16_t gn_xd_sf_period;
static uint16_t gn_xd_sf_size;
static uint16_t gn_xd_sf_x8_size;
static uint16_t gn_xd_ch_size;
static uint16_t gn_xd_sf_bfi_size;

static dev_max_curr_level_t gt_xd_dev_max_curr_level;
static short_level_t gt_xd_short_level;
static fb_level_t gt_xd_fb_level;

static void XD12_Select_Reg_Type(XD12_REG_TYPE n_reg_type);
static void XD12_Write_REGISTER(uint8_t in_addr, uint16_t in_data, XD12_REG_TYPE n_reg_type);
static uint16_t XD12_Read_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type);
static uint16_t XD12_Get_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type);

static void XD12_OTP_Protect_Disable();
static void XD12_OTP_Protect_Enable();

static uint8_t XD12_Write_OSC(uint16_t in_val);
static uint8_t XD12_Write_VREF_CTL(uint16_t in_val);
static uint8_t XD12_Write_OFS_CHx(uint8_t in_ch, uint16_t in_val);
static uint8_t XD12_Write_GAIN_CHx(uint8_t in_ch, uint16_t in_val);

static void XD12_Select_Reg_Type(XD12_REG_TYPE n_reg_type)
{
    static uint16_t addr_ext = 0xFFFF;
    _xd12_op_mode_t* p_op_mode = &(gt_xd12_regs._r3F);

    if (n_reg_type == XD12_REG_TYPE_NON_TRIM)
    {
        p_op_mode->addr_ext = 0;
    }
    else if (n_reg_type == XD12_REG_TYPE_TRIM)
    {
        p_op_mode->addr_ext = 1;
    }

    //if (addr_ext != p_op_mode->addr_ext)
    {
        JigBd_IF_Write_Command(XD12_ADDR_OTP_OP_MODE, p_op_mode->val);
    }

    addr_ext = p_op_mode->addr_ext;
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
    //print(LOG_DEBUG, "[%s][ 0x%02X : 0x%04X ]\r\n", gs_xd12_reg_type[n_reg_type], in_addr, *(p_xd12_reg_addr_base + in_addr));
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
        case XD12_ADDR_LD_MODE :
            print(LOG_INFO, "LD_DIR - [%u], LD_MODE - [%u] (0x%03X)\r\n", gt_xd12_regs._r01.ld_dir, gt_xd12_regs._r01.ld_mode, gt_xd12_regs._r01.val);
            break;
        case XD12_ADDR_SF_PERIOD :
            print(LOG_INFO, "SF_PERIOD - [%u] (0x%03X)\r\n", gt_xd12_regs._r02.sf_period, gt_xd12_regs._r02.val);
            break;
        case XD12_ADDR_SF_SIZE :
            print(LOG_INFO, "SF_SIZE - [%u] (0x%03X)\r\n", gt_xd12_regs._r03.sf_size, gt_xd12_regs._r03.val);
            break;
        case XD12_ADDR_CH_SIZE_SF_X8_SIZE :
            print(LOG_INFO, "CH_SIZE - [%u], SF_X8_SIZE - [%u] (0x%03X)\r\n", gt_xd12_regs._r04.ch_size, gt_xd12_regs._r04.sf_x8_size, gt_xd12_regs._r04.val);
            break;
        case XD12_ADDR_SF_BFI_SIZE :
            print(LOG_INFO, "SF_BFI_SIZE - [%u] (0x%03X)\r\n", gt_xd12_regs._r05.sf_bfi_size, gt_xd12_regs._r05.val);
            break;
        case XD12_ADDR_LD_FIX_1 :
            print(LOG_INFO, "LD_FIX - [%u] (0x%03X)\r\n", gt_xd12_regs._r06.ld_fix, gt_xd12_regs._r06.val);
            break;
        case XD12_ADDR_LD_FIX_2 :
            print(LOG_INFO, "LD_FIX - [%u] (0x%03X)\r\n", gt_xd12_regs._r07.ld_fix, gt_xd12_regs._r07.val);
            break;
        case XD12_ADDR_MAX_CURR_VREF :
            print(LOG_INFO, "MAX_CURR_VREF - [%u] (0x%03X)\r\n", gt_xd12_regs._r08.max_curr_vref, gt_xd12_regs._r08.val);
            break;
        case XD12_ADDR_CHANNEL_ENABLE :
            print(LOG_INFO, "CHANNEL_ENABLE - [0x%03X]\r\n", gt_xd12_regs._r09.val);
            break;
        case XD12_ADDR_FAULT_STATUS :
            print(LOG_INFO, "THERMAL - [%u], SHORT - [%u], OPEN - [%u], FB - [%u] (0x%03X)\r\n", \
            gt_xd12_regs._r0B.bit_thermal, gt_xd12_regs._r0B.bit_short, gt_xd12_regs._r0B.bit_open, gt_xd12_regs._r0B.bit_fb, gt_xd12_regs._r0B.val);
            break;
        case XD12_ADDR_FAULT_LEVEL :
            print(LOG_INFO, "DEV_MAX_CURR_LEVEL - [%u], SHORT_LEVEL - [%u], FB_LEVEL - [%u] (0x%03X)\r\n", \
            gt_xd12_regs._r0C.dev_max_curr_level, gt_xd12_regs._r0C.short_level, gt_xd12_regs._r0C.fb_level, gt_xd12_regs._r0C.val);
            break;
        case XD12_ADDR_FAULT_MODE :
            print(LOG_INFO, "TIMEOUT_EN - [%u], FBOUT_EN - [%u], O_EN - [%u], S2_EN - [%u], S1_EN - [%u], T_OFF_EN - [%u], S_OFF_EN - [%u], O_OFF_EN - [%u] (0x%03X)\r\n", \
            gt_xd12_regs._r0D.timeout_e, gt_xd12_regs._r0D.fbo_e, gt_xd12_regs._r0D.o_en, gt_xd12_regs._r0D.s2_en, \
            gt_xd12_regs._r0D.s1_en, gt_xd12_regs._r0D.t_off_e, gt_xd12_regs._r0D.s_off_e, gt_xd12_regs._r0D.o_off_e, gt_xd12_regs._r0D.val);
            break;
        case XD12_ADDR_DELAY_CH_1 :
            print(LOG_INFO, "DELAY_CH01 - [%u]\r\n", gt_xd12_regs._r0E.delay_ch1);
            break;
        case XD12_ADDR_DELAY_CH_2 :
            print(LOG_INFO, "DELAY_CH02 - [%u]\r\n", gt_xd12_regs._r0F.delay_ch2);
            break;
        case XD12_ADDR_DELAY_CH_3 :
            print(LOG_INFO, "DELAY_CH03 - [%u]\r\n", gt_xd12_regs._r10.delay_ch3);
            break;
        case XD12_ADDR_DELAY_CH_4 :
            print(LOG_INFO, "DELAY_CH04 - [%u]\r\n", gt_xd12_regs._r11.delay_ch4);
            break;
        case XD12_ADDR_DELAY_CH_5 :
            print(LOG_INFO, "DELAY_CH05 - [%u]\r\n", gt_xd12_regs._r12.delay_ch5);
            break;
        case XD12_ADDR_DELAY_CH_6 :
            print(LOG_INFO, "DELAY_CH06 - [%u]\r\n", gt_xd12_regs._r13.delay_ch6);
            break;
        case XD12_ADDR_DELAY_CH_7 :
            print(LOG_INFO, "DELAY_CH07 - [%u]\r\n", gt_xd12_regs._r14.delay_ch7);
            break;
        case XD12_ADDR_DELAY_CH_8 :
            print(LOG_INFO, "DELAY_CH08 - [%u]\r\n", gt_xd12_regs._r15.delay_ch8);
            break;
        case XD12_ADDR_DELAY_CH_9 :
            print(LOG_INFO, "DELAY_CH09 - [%u]\r\n", gt_xd12_regs._r16.delay_ch9);
            break;
        case XD12_ADDR_DELAY_CH_10 :
            print(LOG_INFO, "DELAY_CH10 - [%u]\r\n", gt_xd12_regs._r17.delay_ch10);
            break;
        case XD12_ADDR_DELAY_CH_11 :
            print(LOG_INFO, "DELAY_CH11 - [%u]\r\n", gt_xd12_regs._r18.delay_ch11);
            break;
        case XD12_ADDR_DELAY_CH_12 :
            print(LOG_INFO, "DELAY_CH12 - [%u]\r\n", gt_xd12_regs._r19.delay_ch12);
            break;
        case XD12_ADDR_SERIAL_CLOCK_GEN :
            print(LOG_INFO, "SERIAL_LOW - [%u], SERIAL_HIGH - [%u] (0x%03X)\r\n", gt_xd12_regs._r26.serial_clk_low, gt_xd12_regs._r26.serial_clk_high, gt_xd12_regs._r26.val);
            break;
        case XD12_ADDR_LD_CONTROL :
            print(LOG_INFO, "OFS_TEMP - [%u], ICTL_GLB - [%u], BGR_CTL - [%u] (0x%03X)\r\n", gt_xd12_regs._r27.ofs_temp, gt_xd12_regs._r27.ictl_glb, gt_xd12_regs._r27.bgr_ctl, gt_xd12_regs._r27.val);
            break;
        case XD12_ADDR_DCLK_PERIOD :
            print(LOG_INFO, "DCLK_RST_E - [%u], DCLK_PERIOD - [%u] (0x%03X)\r\n", gt_xd12_regs._r28.dclk_rst_e, gt_xd12_regs._r28.dclk_period, gt_xd12_regs._r28.val);
            break;
        case XD12_ADDR_MCLK_LOCK_1 :
            print(LOG_INFO, "MCLK_LOCK - [%u] (0x%03X)\r\n", gt_xd12_regs._r29.mclk_lock, gt_xd12_regs._r29.val);
            break;
        case XD12_ADDR_MCLK_LOCK_2 :
            print(LOG_INFO, "MCLK_LOCK_E - [%u], MCLK_LOCK - [%u] (0x%03X)\r\n", gt_xd12_regs._r2A.mclk_lock_e, gt_xd12_regs._r2A.mclk_lock, gt_xd12_regs._r2A.val);
            break;
        case XD12_ADDR_OSC_COMP :
            print(LOG_INFO, "OSC_POL - [%u], OSC_COMP - [%u] (0x%03X)\r\n", gt_xd12_regs._r2B.osc_pol, gt_xd12_regs._r2B.osc_comp, gt_xd12_regs._r2B.val);
            break;
        case XD12_ADDR_OSC_COMP_TH_P :
            print(LOG_INFO, "OSC_COMP_TH_P - [%u] (0x%03X)\r\n", gt_xd12_regs._r2C.osc_comp_th_p, gt_xd12_regs._r2C.val);
            break;
        case XD12_ADDR_OSC_COMP_TH_N :
            print(LOG_INFO, "OSC_COMP_TH_N - [%u] (0x%03X)\r\n", gt_xd12_regs._r2D.osc_comp_th_n, gt_xd12_regs._r2D.val);
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
            print(LOG_INFO, "TEST_EN - [%u], LD_FIX_EN - [%u], ADDR_EXT - [%u] (0x%03X)\r\n", gt_xd12_regs._r3F.test_en, gt_xd12_regs._r3F.ld_fix_en, gt_xd12_regs._r3F.addr_ext, gt_xd12_regs._r3F.val);
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
        case XD12_ADDR_TRIM_OFS_CH_1 :
            print(LOG_INFO, "OSF_CH01 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r03.ofs_ch_x, gt_xd12_trim_regs._r03.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_2 :
            print(LOG_INFO, "OSF_CH02 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r04.ofs_ch_x, gt_xd12_trim_regs._r04.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_3 :
            print(LOG_INFO, "OSF_CH03 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r05.ofs_ch_x, gt_xd12_trim_regs._r05.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_4 :
            print(LOG_INFO, "OSF_CH04 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r06.ofs_ch_x, gt_xd12_trim_regs._r06.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_5 :
            print(LOG_INFO, "OSF_CH05 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r07.ofs_ch_x, gt_xd12_trim_regs._r07.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_6 :
            print(LOG_INFO, "OSF_CH06 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r08.ofs_ch_x, gt_xd12_trim_regs._r08.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_7 :
            print(LOG_INFO, "OSF_CH07 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r09.ofs_ch_x, gt_xd12_trim_regs._r09.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_8 :
            print(LOG_INFO, "OSF_CH08 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0A.ofs_ch_x, gt_xd12_trim_regs._r0A.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_9 :
            print(LOG_INFO, "OSF_CH09 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0B.ofs_ch_x, gt_xd12_trim_regs._r0B.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_10 :
            print(LOG_INFO, "OSF_CH10 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0C.ofs_ch_x, gt_xd12_trim_regs._r0C.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_11 :
            print(LOG_INFO, "OSF_CH11 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0D.ofs_ch_x, gt_xd12_trim_regs._r0D.val);
            break;
        case XD12_ADDR_TRIM_OFS_CH_12 :
            print(LOG_INFO, "OSF_CH12 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r0E.ofs_ch_x, gt_xd12_trim_regs._r0E.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_1 :
            print(LOG_INFO, "GAIN_CH01 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1B.gain_ch_x, gt_xd12_trim_regs._r1B.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_2 :
            print(LOG_INFO, "GAIN_CH02 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1C.gain_ch_x, gt_xd12_trim_regs._r1C.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_3 :
            print(LOG_INFO, "GAIN_CH03 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1D.gain_ch_x, gt_xd12_trim_regs._r1D.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_4 :
            print(LOG_INFO, "GAIN_CH04 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1E.gain_ch_x, gt_xd12_trim_regs._r1E.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_5 :
            print(LOG_INFO, "GAIN_CH05 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r1F.gain_ch_x, gt_xd12_trim_regs._r1F.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_6 :
            print(LOG_INFO, "GAIN_CH06 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r20.gain_ch_x, gt_xd12_trim_regs._r20.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_7 :
            print(LOG_INFO, "GAIN_CH07 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r21.gain_ch_x, gt_xd12_trim_regs._r21.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_8 :
            print(LOG_INFO, "GAIN_CH08 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r22.gain_ch_x, gt_xd12_trim_regs._r22.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_9 :
            print(LOG_INFO, "GAIN_CH09 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r23.gain_ch_x, gt_xd12_trim_regs._r23.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_10 :
            print(LOG_INFO, "GAIN_CH10 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r24.gain_ch_x, gt_xd12_trim_regs._r24.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_11 :
            print(LOG_INFO, "GAIN_CH11 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r25.gain_ch_x, gt_xd12_trim_regs._r25.val);
            break;
        case XD12_ADDR_TRIM_GAIN_CH_12 :
            print(LOG_INFO, "GAIN_CH12 - [%u] (0x%03X)\r\n", gt_xd12_trim_regs._r26.gain_ch_x, gt_xd12_trim_regs._r26.val);
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
        _xd12_fault_level_t* p_xd12_fault_level = &gt_xd12_regs._r0C;

        p_xd12_fault_level->dev_max_curr_level = in_dev_max_curr;
        p_xd12_fault_level->short_level = in_short_level;
        p_xd12_fault_level->fb_level = in_fb_level;

        XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p_xd12_fault_level->val, XD12_REG_TYPE_NON_TRIM);
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
        _xd12_fault_level_t* p_xd12_fault_level = &gt_xd12_regs._r0C;

        p_xd12_fault_level->dev_max_curr_level = in_dev_max_curr;

        XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p_xd12_fault_level->val, XD12_REG_TYPE_NON_TRIM);
        return TRUE;
    }
}

uint8_t XD12_set_CHANNEL_ENABLE(uint8_t in_channel)
{
    _xd12_channel_enable_t* p_xd12_channel_enable = &gt_xd12_regs._r09;

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
    _xd12_channel_enable_t* p_xd12_channel_enable = &gt_xd12_regs._r09;
    p_xd12_channel_enable->val = 0x00;
    XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, p_xd12_channel_enable->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_set_CHANNEL_ENABLE_All(void)
{
    _xd12_channel_enable_t* p_xd12_channel_enable = &gt_xd12_regs._r09;
    p_xd12_channel_enable->val = 0xFFF;
    XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, p_xd12_channel_enable->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_set_delay_ch(void)
{
    uint8_t delay_per_ch = 0;
    delay_per_ch = (uint8_t)(gn_xd_sf_size / gn_xd_ch_size);
    for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
    {
        TargetIC_IF_Write_Register((XD12_ADDR_DELAY_CH_1 + ch), (delay_per_ch * ch), XD12_REG_TYPE_NON_TRIM);
    }
}

static void XD12_set_serial_clock_gen(void)
{
    _xd12_serial_clock_gen_t* p_xd12_serial_clock_gen = &gt_xd12_regs._r26;

    p_xd12_serial_clock_gen->serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
    p_xd12_serial_clock_gen->serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
    XD12_Write_REGISTER(XD12_ADDR_SERIAL_CLOCK_GEN, p_xd12_serial_clock_gen->val, XD12_REG_TYPE_NON_TRIM);
}

static void XD12_set_osc_comp(bool in_polarity, uint8_t in_osc_comp)
{
    _xd12_osc_comp_t* p_osc_comp = &(gt_xd12_regs._r2B);

    if(in_polarity == TRUE)
    {
        p_osc_comp->osc_pol = 1;
    }
    else
    {
        p_osc_comp->osc_pol = 0;
    }

    p_osc_comp->osc_comp = in_osc_comp;
    XD12_Write_REGISTER(XD12_ADDR_OSC_COMP, p_osc_comp->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_set_ld_fix(uint32_t in_ld_fix)
{
    if (in_ld_fix < 65536)
    {
        _xd12_ld_fix_1_t* p_xd12_ld_fix_1 = &gt_xd12_regs._r06;
        _xd12_ld_fix_2_t* p_xd12_ld_fix_2 = &gt_xd12_regs._r07;

        uint16_t ld_fix_1 = ((in_ld_fix & 0x0FFF) >>  0);
        uint16_t ld_fix_2 = ((in_ld_fix & 0xF000) >> 12);

        p_xd12_ld_fix_1->ld_fix = ld_fix_1;
        p_xd12_ld_fix_2->ld_fix = ld_fix_2;

        XD12_Write_REGISTER(XD12_ADDR_LD_FIX_1, p_xd12_ld_fix_1->val, XD12_REG_TYPE_NON_TRIM);
        XD12_Write_REGISTER(XD12_ADDR_LD_FIX_2, p_xd12_ld_fix_2->val, XD12_REG_TYPE_NON_TRIM);
    }
}

void XD12_set_ictl_glb(uint32_t in_ictl_glb)
{
    if (in_ictl_glb < 4)
    {
        _xd12_ld_control_t* p_xd12_ld_control = &gt_xd12_regs._r27;

        p_xd12_ld_control->ictl_glb = in_ictl_glb;

        XD12_Write_REGISTER(XD12_ADDR_LD_CONTROL, p_xd12_ld_control->val, XD12_REG_TYPE_NON_TRIM);
    }
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

void XD12_set_ld_fix_en(bool in_Enable)
{
    _xd12_op_mode_t* p_op_mode = &(gt_xd12_regs._r3F);

    if(in_Enable == TRUE)
    {
        p_op_mode->ld_fix_en = 1;
    }
    else
    {
        p_op_mode->ld_fix_en = 0;
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

void XD12_set_sf_bfi(uint16_t n_sf_bfi)
{
    _xd12_sf_bfi_size_t* p_xd12_sf_bfi_size = &(gt_xd12_regs._r05);

    if (n_sf_bfi <= gn_xd_sf_size)
    {
        p_xd12_sf_bfi_size->sf_bfi_size = n_sf_bfi;
        XD12_Write_REGISTER(XD12_ADDR_SF_BFI_SIZE, p_xd12_sf_bfi_size->val, XD12_REG_TYPE_NON_TRIM);
    }
    else
    {
        print(LOG_ERROR, "\r\n Out of sf_bfi [%u] [0 - %u]\r\n", n_sf_bfi, gn_xd_sf_size);
    }
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

static uint8_t XD12_Write_OFS_CHx(uint8_t in_ch, uint16_t in_val)
{
    _xd12_ofs_ch_t* p_xd12_ofs_ch = &gt_xd12_trim_regs._r03 + in_ch;
    if(in_ch > CH_MAX || in_val > REG_LIMIT_OFS)
    {
        print(LOG_ERROR, "XD12_Write_OFS_CH%d() input[%d] is Over %d\r\n", in_ch+1, in_val, REG_LIMIT_OFS);
        return FALSE;
    }
    else
    {
        p_xd12_ofs_ch->ofs_ch_x = in_val;
        XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH_1 + in_ch, p_xd12_ofs_ch->val, XD12_REG_TYPE_TRIM);
        return TRUE;
    }
}

static uint8_t XD12_Write_GAIN_CHx(uint8_t in_ch, uint16_t in_val)
{
    _xd12_gain_ch_t* p_xd12_gain_ch = &gt_xd12_trim_regs._r1B + in_ch;
    if(in_ch > CH_MAX || in_val > REG_LIMIT_GAIN)
    {
        print(LOG_ERROR, "XD12_Write_GAIN_CHx%d() input[%d] is Over %d\r\n",in_ch+1,in_val, REG_LIMIT_GAIN);
        return FALSE;
    }
    else
    {
        p_xd12_gain_ch->gain_ch_x = in_val;
        XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH_1 + in_ch, p_xd12_gain_ch->val, XD12_REG_TYPE_TRIM);
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
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_OSC_TARGET);
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_OSC_TARGET + 0.1f);
            p[mode][TRIM_PARA_P1] = 0;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_GAIN_CHS:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_GAIN_TARGET * (1 - XD12_GAIN_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_GAIN_TARGET * (1 + XD12_GAIN_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_P1] = XD12_GAIN_P1;
            p[mode][TRIM_PARA_P2] = XD12_GAIN_P2;
            break;
        case TRIM_OFS_CHS:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_OFS_TARGET * (1 - XD12_OFS_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_OFS_TARGET * (1 + XD12_OFS_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_P1] = XD12_OFS_P1;
            p[mode][TRIM_PARA_P2] = XD12_OFS_P2;
            break;
        }
    }
}

void TargetIC_IF_Trim_Reg_Init(void)
{
    #if 0
                               // 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12
    uint16_t ofs_ch_regs[12] =  {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};
    uint16_t gain_ch_regs[12] = {77, 58, 63, 79, 67, 74, 74, 81, 74, 68, 69, 80};
    XD12_Write_REGISTER(XD12_ADDR_OTP_PROTECT, 0xA5A, XD12_REG_TYPE_NON_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OSC, 81, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_VREF_CTL, 18, XD12_REG_TYPE_TRIM);

    for (uint8_t ch = 0 ; ch < CH_MAX ; ++ch)
    {
        XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH_1 + ch, ofs_ch_regs[ch], XD12_REG_TYPE_TRIM);
        XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH_1 + ch, gain_ch_regs[ch], XD12_REG_TYPE_TRIM);
    }
    #endif

    XD12_Write_REGISTER(XD12_ADDR_OTP_PROTECT, 0xA5A, XD12_REG_TYPE_NON_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OSC, gn_xd_trim_reg[XD12_ADDR_TRIM_OSC], XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_VREF_CTL, gn_xd_trim_reg[XD12_ADDR_TRIM_VREF_CTL], XD12_REG_TYPE_TRIM);

    for (uint8_t ch = 0 ; ch < CH_MAX ; ++ch)
    {
        XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH_1 + ch, gn_xd_trim_reg[XD12_ADDR_TRIM_OFS_CH_1 + ch], XD12_REG_TYPE_TRIM);
        XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH_1 + ch, gn_xd_trim_reg[XD12_ADDR_TRIM_GAIN_CH_1 + ch], XD12_REG_TYPE_TRIM);
    }
}

void TargetIC_IF_Init_Register(void)
{
    TargetIC_IF_TrimRegister_Set(0, TRIM_OSC_FREQUENCY, XD12_DEFAULT_OSC);
    TargetIC_IF_TrimRegister_Set(0, TRIM_VREF_CTL, XD12_DEFAULT_VREF_CTL);

    for(uint8_t i_channel = 0 ; i_channel < CH_MAX ; ++i_channel)
    {
        TargetIC_IF_TrimRegister_Set(i_channel, TRIM_OFS_CHS, XD12_DEFAULT_OFS);
    }

    for(uint8_t i_channel = 0 ; i_channel < CH_MAX ; ++i_channel)
    {
        TargetIC_IF_TrimRegister_Set(i_channel, TRIM_GAIN_CHS, XD12_DEFAULT_GAIN);
    }
}

void TargetIC_IF_XD_Regs_Read_Display(void)
{
    XD12_Read_All();
    XD12_Print_XD12_REGS();
}

void TargetIC_IF_Save_Trim_Reg(void)
{
    for (uint8_t addr = 0 ; addr < XD12_ADDR_TRIM_MAX ; ++addr)
    {
        gn_xd_trim_reg[addr] = TargetIC_IF_Read_Register(addr, XD12_REG_TYPE_TRIM);
    }

    print(LOG_INFO, "osc:%3u\r\n", gn_xd_trim_reg[1]);
    print(LOG_INFO, "vref:%3u\r\n", gn_xd_trim_reg[2]);

    print(LOG_INFO, "ofs:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u\r\n"
    , gn_xd_trim_reg[ 3], gn_xd_trim_reg[ 4], gn_xd_trim_reg[ 5] , gn_xd_trim_reg[ 6], gn_xd_trim_reg[ 7], gn_xd_trim_reg[ 8]
    , gn_xd_trim_reg[ 9], gn_xd_trim_reg[10], gn_xd_trim_reg[11] , gn_xd_trim_reg[12], gn_xd_trim_reg[13], gn_xd_trim_reg[14]
    );
    print(LOG_INFO, "gain:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u\r\n"
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
            print(LOG_INFO, "\033[0;31m %s - NG", gs_xd12_addr_trim_str[trim_addr]);
        }
        else
        {
            print(LOG_INFO, "\033[0;32m %s - OK", gs_xd12_addr_trim_str[trim_addr]);
        }
        print(LOG_INFO, "   [0x%03X] - [0x%03X] \033[0m\r\n", gn_xd_trim_reg[trim_addr], u16_reg_val);
    }

    return ret;
}
void TargetIC_IF_Set_Test_Enable(bool in_Enable)
{
    XD12_set_test_en(in_Enable);
    //XD12_set_ld_fix_en(in_Enable);
    LL_mDelay(10);
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
            //print(LOG_DEBUG, "GET-OSC : %d\r\n", rtn_val);
        }
        break;

    case TRIM_VREF_CTL:
        {
            _xd12_vref_ctl_t* p_xd12_vref_ctl = &gt_xd12_trim_regs._r02;
            rtn_val = p_xd12_vref_ctl->val;
            //print(LOG_DEBUG, "GET-VREF : %d\r\n", rtn_val);
        }
        break;

    case TRIM_GAIN_CHS:
        {
            _xd12_gain_ch_t* p_xd12_gain_ch_x = &gt_xd12_trim_regs._r1B + ch_num;
            rtn_val = p_xd12_gain_ch_x->val;
            //print(LOG_DEBUG, "GET-GAIN_CH%02d : %d\r\n", ch_num+ 1 , rtn_val);
        }
        break;

    case TRIM_OFS_CHS:
        {
            _xd12_ofs_ch_t* p_xd12_ofs_ch_x = &gt_xd12_trim_regs._r03 + ch_num;
            rtn_val = p_xd12_ofs_ch_x->val;
            //print(LOG_DEBUG, "GET-OFS_CH%02d : %d\r\n", ch_num + 1, rtn_val);
        }
        break;
    }

    return rtn_val;
}

uint8_t TargetIC_IF_TrimRegister_Set(uint8_t ch_num, trim_mode_t in_trim_mode, uint16_t in_regi_val)
{
    uint8_t rtn_val = FALSE;
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        if(in_regi_val > REG_LIMIT_OSC)
        {
            print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set TRIM_OSC_FREQUENCY- in_regi_val(%d) Over !!\r\n", in_regi_val);
        }
        else
        {
            rtn_val = XD12_Write_OSC(in_regi_val);
        }
        break;

    case TRIM_VREF_CTL:
        if(in_regi_val > REG_LIMIT_VREF)
        {
            print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set TRIM_VREF_CTL- in_regi_val(%d) Over !!\r\n", in_regi_val);
        }
        else
        {
            rtn_val = XD12_Write_VREF_CTL(in_regi_val);
        }
        break;

    case TRIM_GAIN_CHS:
        if(in_regi_val > REG_LIMIT_GAIN)
        {
            print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set TRIM_GAIN_CHS- in_regi_val(%d) Over !!\r\n", in_regi_val);
        }
        else
        {
            rtn_val = XD12_Write_GAIN_CHx(ch_num, in_regi_val);
        }
        break;

    case TRIM_OFS_CHS:
        if(in_regi_val > REG_LIMIT_OFS)
        {
            print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set TRIM_OFS_CHS- in_regi_val(%d) Over !!\r\n", in_regi_val);
        }
        else
        {
            rtn_val = XD12_Write_OFS_CHx(ch_num, in_regi_val);
        }
        break;
    }

    if(rtn_val == FALSE)
    {
        print(LOG_ERROR, "ERROR: TargetIC_IF_TrimRegister_Set(%d, %d, %d) - FALSE RETRUN\r\n", ch_num, in_trim_mode, in_regi_val);
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
        print(LOG_DEBUG, "OSC_LIMIT : %d\r\n", rtn_val);
        break;

    case TRIM_VREF_CTL:
        rtn_val = REG_LIMIT_VREF;
        print(LOG_DEBUG, "VREF_LIMIT : %d\r\n", rtn_val);
        break;

    case TRIM_GAIN_CHS:
        rtn_val = REG_LIMIT_GAIN;
        print(LOG_DEBUG, "GAIN_CH_%02d_LIMIT : %d\r\n", ch_num + 1, rtn_val);
        break;

    case TRIM_OFS_CHS:
        rtn_val = REG_LIMIT_OFS;
        print(LOG_DEBUG, "OFS_CH_%02d_LIMIT : %d\r\n", ch_num + 1, rtn_val);
        break;
    }
    return rtn_val;
}

void TargetIC_IF_TRIM_Init_VREF_CTL(void)
{
    XD12_set_test_en(TRUE);
}

void TargetIC_IF_TRIM_Init_OSC(void)
{
    XD12_set_test_en(TRUE);
    XD12_set_osc_comp(FALSE, 16);
}

void TargetIC_IF_TRIM_Init_OFS(void)
{
    XD12_set_test_en(TRUE);
    XD12_set_ld_fix_en(TRUE);
}

void TargetIC_IF_TRIM_Init_GAIN(void)
{
    XD12_set_test_en(TRUE);
    XD12_set_ld_fix_en(TRUE);
}

void TargetIC_IF_SEND_otp_written(void)
{
    uint16_t* p_xd12_reg_addr = NULL;
    p_xd12_reg_addr = gt_xd12_trim_regs.ALL;

    for(xd12_trim_addr_t addr = XD12_ADDR_TRIM_OSC ; addr < XD12_ADDR_TRIM_MAX ; ++addr)
    {
        print(LOG_INFO, ":%02x:%03x", addr, *(p_xd12_reg_addr + addr));
    }
    print(LOG_INFO, "\r\n%s\r\n", CMD_XD12_OTP_WRITTEN);
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

static void XD12_reg_value_init()
{
    gf_xd_mclk = XD_MCLK;
    gf_vsync_out = VSYNC;

    gn_xd_sf_size = XD_SUB_FRAME_SIZE;
    gn_xd_sf_period = (uint16_t)((gf_xd_mclk / (gf_vsync_out * gn_xd_sf_size)) + 0.5f);

    gn_xd_sf_bfi_size = 0;

    gn_xd_sf_x8_size = (uint16_t)(gn_xd_sf_size / 8);

    gn_xd_ch_size = XD_CH_SIZE;

    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_8mA;
    gt_xd_short_level = SHORT_LEVEL_36V;
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
        case XD12_ADDR_LD_MODE :
            gt_xd12_regs._r01.ld_mode = XD_LD_MODE_NORMAL;
            gt_xd12_regs._r01.ld_dir = XD_LD_DIR_TAIL_SHIFT;
            break;
        case XD12_ADDR_SF_PERIOD :
            gt_xd12_regs._r02.sf_period = gn_xd_sf_period;
            break;
        case XD12_ADDR_SF_SIZE :
            gt_xd12_regs._r03.sf_size = gn_xd_sf_size;
            break;
        case XD12_ADDR_CH_SIZE_SF_X8_SIZE :
            gt_xd12_regs._r04.ch_size = gn_xd_ch_size;
            gt_xd12_regs._r04.sf_x8_size = gn_xd_sf_x8_size;
            break;
        case XD12_ADDR_SF_BFI_SIZE :
            gt_xd12_regs._r05.sf_bfi_size = gn_xd_sf_bfi_size;
            break;
        case XD12_ADDR_MAX_CURR_VREF :
            gt_xd12_regs._r08.max_curr_vref = 0xFFF;
            break;
        case XD12_ADDR_CHANNEL_ENABLE :
            //gt_xd12_regs._r09.val = 0xFFF;
            gt_xd12_regs._r09.val = 1;
            break;
        case XD12_ADDR_FAULT_LEVEL :
            gt_xd12_regs._r0C.fb_level = gt_xd_fb_level;
            gt_xd12_regs._r0C.short_level = gt_xd_short_level;
            gt_xd12_regs._r0C.dev_max_curr_level = gt_xd_dev_max_curr_level;
            break;
        case XD12_ADDR_FAULT_MODE :
            gt_xd12_regs._r0D.o_off_e = 0;
            gt_xd12_regs._r0D.s_off_e = 0;
            gt_xd12_regs._r0D.t_off_e = 1;
            gt_xd12_regs._r0D.s1_en = 1;
            gt_xd12_regs._r0D.s2_en = 1;
            gt_xd12_regs._r0D.o_en = 1;
            gt_xd12_regs._r0D.fbo_e = 1;
            gt_xd12_regs._r0D.timeout_e = 1;
            break;
        case XD12_ADDR_SERIAL_CLOCK_GEN :
            gt_xd12_regs._r26.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
            gt_xd12_regs._r26.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
            break;
        case XD12_ADDR_LD_CONTROL :
            gt_xd12_regs._r27.bgr_ctl = 8;
            gt_xd12_regs._r27.ictl_glb = 4;
            gt_xd12_regs._r27.ofs_temp = 8;
            break;
        case XD12_ADDR_DCLK_PERIOD :
            gt_xd12_regs._r28.dclk_period = 64;
            gt_xd12_regs._r28.dclk_rst_e = 0;
            break;
        case XD12_ADDR_MCLK_LOCK_1 :
            gt_xd12_regs._r29.mclk_lock = 0;
            break;
        case XD12_ADDR_MCLK_LOCK_2 :
            gt_xd12_regs._r2A.mclk_lock = 0x1E;
            gt_xd12_regs._r2A.mclk_lock_e = 0;
            break;
        case XD12_ADDR_OSC_COMP :
            gt_xd12_regs._r2B.osc_comp = 0x0F;
            gt_xd12_regs._r2B.osc_pol = 0;
            break;
        case XD12_ADDR_OSC_COMP_TH_P :
            gt_xd12_regs._r2C.osc_comp_th_p = 0x80;
            break;
        case XD12_ADDR_OSC_COMP_TH_N :
            gt_xd12_regs._r2D.osc_comp_th_n = 0x80;
            break;
        default :
            continue;
        }
        TargetIC_IF_Write_Register(xd12_addr, *(p_xd_reg_base_addr + xd12_addr), XD12_REG_TYPE_NON_TRIM);
    }
    //XD12_set_delay_ch();
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
    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_4mA;

    gt_xd12_regs._r0C.fb_level = gt_xd_fb_level;
    gt_xd12_regs._r0C.short_level = gt_xd_short_level;
    gt_xd12_regs._r0C.dev_max_curr_level = gt_xd_dev_max_curr_level;
    TargetIC_IF_Write_Register(XD12_ADDR_FAULT_LEVEL, gt_xd12_regs._r0C.val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_Trim_prepare_read_otp_re_write(void)
{
    gt_xd12_regs._r3F.addr_ext = 0;
    TargetIC_IF_Write_Register(XD12_ADDR_OTP_OP_MODE, gt_xd12_regs._r3F.val, XD12_REG_TYPE_NON_TRIM);

    JigBd_IF_XD12_reset_Command();

    JigBd_IF_IdGen_Command();

    gt_xd12_regs._r3F.val = 0;
    TargetIC_IF_Write_Register(XD12_ADDR_OTP_OP_MODE, gt_xd12_regs._r3F.val, XD12_REG_TYPE_NON_TRIM);

    XD12_set_serial_clock_gen();

    XD12_OTP_Protect_Disable();

    for (xd12_trim_addr_t trim_addr = XD12_ADDR_TRIM_OSC ; trim_addr < XD12_ADDR_TRIM_MAX ; ++trim_addr)
    {
        TargetIC_IF_Write_Register(trim_addr, *(gt_xd12_trim_regs.ALL + trim_addr), XD12_REG_TYPE_TRIM);
    }
    JigBd_IF_VLED_9V_EN(PWR_ON);
}

void vsync_update_frequency(float n_freq)
{
    uint32_t period = 0;
    uint32_t prescale = 0;

    prescale = LL_TIM_GetPrescaler(TIM8);
    period = (uint32_t)((TIM8_FREQ / (prescale + 1)) / n_freq - 1 + 0.5f);

    LL_TIM_SetAutoReload(TIM8, period);

    gf_vsync_out = n_freq;

    gn_xd_sf_period = (uint16_t)((gf_xd_mclk / (gf_vsync_out * gn_xd_sf_size)) + 0.5f);
    gt_xd12_regs._r02.sf_period = gn_xd_sf_period;
    TargetIC_IF_Write_Register(XD12_ADDR_SF_PERIOD, gt_xd12_regs._r02.val, XD12_REG_TYPE_NON_TRIM);
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
