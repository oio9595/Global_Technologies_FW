/** @file xdic.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XDIC_C__
#include "config.h"

#define XDIC_REG_GENERAL            (0)
#define XDIC_REG_MIRROR             (1)

#define XDIC_OTP_PROTECT_DISABLE    (0xA5A)
#define XDIC_OTP_PROTECT_ENABLE     (0x5A5)

#define XDIC_DEFAULT_OSC            (0x20)
#define XDIC_DEFAULT_VREF_CTL       (0x20)
#define XDIC_DEFAULT_ICTL_L         (0x40)
#define XDIC_DEFAULT_ICTL_H         (0x40)

#define XD_MCLK                     (39319200.0f)
#define VSYNC                       (120.0f)

#define TIM8_FREQ                   (144000000.0f)

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

#ifdef __XDIC_C__

static const char* gs_xdic_general_regs_str[XDIC_ADDR_MAX] =
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
    "VREF_FIX",
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

static const char* gs_xdic_mirror_regs_str[XDIC_ADDR_TRIM_MAX] =
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

#endif //__XDIC_C__

static _xdic_general_regs_t gt_xdic_general_regs;
static _xdic_mirror_regs_t gt_xdic_mirror_regs;
static uint16_t gn_xdic_saved_trim_reg[XDIC_ADDR_TRIM_MAX];

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

static void XDIC_Set_Delay_CH(void);

void XDIC_Write_General_Reg(uint8_t addr, uint16_t data)
{
    if (gt_xdic_general_regs._r3F.addr_ext != XDIC_REG_GENERAL)
    {
        gt_xdic_general_regs._r3F.addr_ext = XDIC_REG_GENERAL;
        JigBD_IF_Write_Command(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
    }

    if (addr < XDIC_ADDR_MAX)
    {
        *(&gt_xdic_general_regs._r00.val + addr) = data;
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Over !!\r\n", __func__, addr);
    }

    JigBD_IF_Write_Command(addr, data);
}

uint16_t XDIC_Read_General_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    if (gt_xdic_general_regs._r3F.addr_ext != XDIC_REG_GENERAL)
    {
        gt_xdic_general_regs._r3F.addr_ext = XDIC_REG_GENERAL;
        JigBD_IF_Write_Command(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
    }

    xdic_reg_val = JigBD_IF_Read_Command(addr);
    if (addr < XDIC_ADDR_MAX)
    {
        *(&gt_xdic_general_regs._r00.val + addr) = xdic_reg_val;
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Over !!\r\n", __func__, addr);
    }

    print(LOG_DEBUG, "XDIC General Read --> [ 0x%02X - 0x%04X] \r\n", addr, *(&gt_xdic_general_regs._r00.val + addr));

    return *(&gt_xdic_general_regs._r00.val + addr);
}

uint16_t XDIC_Get_General_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    if (addr < XDIC_ADDR_MAX)
    {
        xdic_reg_val = *(&gt_xdic_general_regs._r00.val + addr);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Over !!\r\n", __func__, addr);
    }

    return xdic_reg_val;
}

void XDIC_Write_Mirror_Reg(uint8_t addr, uint16_t data)
{
    if (gt_xdic_general_regs._r3F.addr_ext != XDIC_REG_MIRROR)
    {
        gt_xdic_general_regs._r3F.addr_ext = XDIC_REG_MIRROR;
        JigBD_IF_Write_Command(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
    }

    if (addr < XDIC_ADDR_TRIM_MAX)
    {
        *(&gt_xdic_mirror_regs._r00.val + addr) = data;
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Over !!\r\n", __func__, addr);
    }
    JigBD_IF_Write_Command(addr, data);
}

uint16_t XDIC_Read_Mirror_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    if (gt_xdic_general_regs._r3F.addr_ext != XDIC_REG_MIRROR)
    {
        gt_xdic_general_regs._r3F.addr_ext = XDIC_REG_MIRROR;
        JigBD_IF_Write_Command(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
    }

    xdic_reg_val = JigBD_IF_Read_Command(addr);
    if (addr < XDIC_ADDR_TRIM_MAX)
    {
        *(&gt_xdic_mirror_regs._r00.val + addr) = xdic_reg_val;
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Over !!\r\n", __func__, addr);
    }

    print(LOG_DEBUG, "XDIC Mirror Read --> [ 0x%02X - 0x%04X] \r\n", addr, *(&gt_xdic_mirror_regs._r00.val + addr));

    return *(&gt_xdic_mirror_regs._r00.val + addr);
}

uint16_t XDIC_Get_Mirror_Reg(uint8_t addr)
{
    uint16_t xdic_reg_val = 0xFFFF;
    if (addr < XDIC_ADDR_TRIM_MAX)
    {
        xdic_reg_val = *(&gt_xdic_mirror_regs._r00.val + addr);
    }
    else
    {
        print(LOG_ERROR, "ERROR: %s - addr(0x%02X) Over !!\r\n", __func__, addr);
    }

    return xdic_reg_val;
}

void XDIC_Write_Mirror_Register_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode, uint16_t in_reg_val)
{
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        if (in_reg_val > REG_LIMIT_OSC)
        {
            print(LOG_ERROR, "ERROR: %s TRIM_OSC_FREQUENCY- in_reg_val(%d) Over !!\r\n", __func__, in_reg_val);
        }
        else
        {
            XDIC_Write_Mirror_Reg(XDIC_ADDR_TRIM_OSC, in_reg_val);
        }
        break;
    case TRIM_VREF_CTL:
        if (in_reg_val > REG_LIMIT_VREF)
        {
            print(LOG_ERROR, "ERROR: %s TRIM_VREF_CTL- in_reg_val(%d) Over !!\r\n", __func__, in_reg_val);
        }
        else
        {
            XDIC_Write_Mirror_Reg(XDIC_ADDR_TRIM_VREF_CTL, in_reg_val);
        }
        break;
    case TRIM_ICTL_L_CHS:
        if (in_reg_val > REG_LIMIT_ICTL_L)
        {
            print(LOG_ERROR, "ERROR: %s TRIM_ICTL_L_CHS- in_reg_val(%d) Over !!\r\n", __func__, in_reg_val);
        }
        else
        {
            XDIC_Write_Mirror_Reg(XDIC_ADDR_TRIM_ICTL_L_CH_1 + ch_num, in_reg_val);
        }
        break;
    case TRIM_ICTL_H_CHS:
        if (in_reg_val > REG_LIMIT_ICTL_H)
        {
            print(LOG_ERROR, "ERROR: %s TRIM_ICTL_H_CHS- in_reg_val(%d) Over !!\r\n", __func__, in_reg_val);
        }
        else
        {
            XDIC_Write_Mirror_Reg(XDIC_ADDR_TRIM_ICTL_H_CH_1 + ch_num, in_reg_val);
        }
        break;
    }
}

uint16_t XDIC_Get_Mirror_Register_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0xFFFF;
    switch(in_trim_mode)
    {
    case TRIM_OSC_FREQUENCY:
        rtn_val = gt_xdic_mirror_regs._r01.val;
        break;
    case TRIM_VREF_CTL:
        rtn_val = gt_xdic_mirror_regs._r02.val;
        break;
    case TRIM_ICTL_L_CHS:
        rtn_val = *(&gt_xdic_mirror_regs._r03.val + ch_num);
        break;
    case TRIM_ICTL_H_CHS:
        rtn_val = *(&gt_xdic_mirror_regs._r1B.val + ch_num);
        break;
    }

    return rtn_val;
}

uint16_t XDIC_Get_Mirror_Register_Limit_By_Trim_Mode(uint8_t ch_num, trim_mode_t in_trim_mode)
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

void XDIC_Read_All_Registers(void)
{
    for (uint8_t addr_offset = 0 ; addr_offset < XDIC_ADDR_MAX ; ++addr_offset)
    {
        XDIC_Read_General_Reg(addr_offset);
    }

    for (uint8_t addr_offset = 0 ; addr_offset < XDIC_ADDR_TRIM_MAX ; ++addr_offset)
    {
        XDIC_Read_Mirror_Reg(addr_offset);
    }

    XDIC_Dump_All_Registers();
}

void XDIC_Dump_All_Registers(void)
{
    for (uint8_t xd_general_addr = 0 ; xd_general_addr < XDIC_ADDR_MAX ; ++xd_general_addr)
    {
        print(LOG_INFO, "[%s (0x%02X)]\r\n"
                        "\t VALUE : (0x%04X)\r\n\r\n",
                        gs_xdic_general_regs_str[xd_general_addr], xd_general_addr, gt_xdic_general_regs.ALL[xd_general_addr]);
    }

    for (uint8_t xd_mirror_addr = 0 ; xd_mirror_addr < XDIC_ADDR_TRIM_MAX ; ++xd_mirror_addr)
    {
        print(LOG_INFO, "[%s (0x%02X)]\r\n"
                        "\t VALUE : (0x%04X)\r\n\r\n",
                        gs_xdic_mirror_regs_str[xd_mirror_addr], xd_mirror_addr, gt_xdic_mirror_regs.ALL[xd_mirror_addr]);
    }
}

void XDIC_Param_Init(void)
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

    //gn_xd_mclk_lock_cnt = (uint32_t)(XD_MCLK_LOCK_CNT_120Hz * (1 + 0.08f / 100));
    gn_xd_mclk_lock_cnt = XD_MCLK_LOCK_CNT_120Hz;

    gn_xd_ch_size = XD_CH_SIZE;

    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_24mA;
    gt_xd_short_level = SHORT_LEVEL_6V;
    gt_xd_fb_level = FB_LEVEL_0V5;
}

void XDIC_Init(void)
{
    XDIC_Param_Init();

    JigBD_IF_Reset_Command();
    JigBD_IF_IdGen_Command();

    for (xdic_addr_t xdic_addr = XDIC_ADDR_RESET_ID ; xdic_addr < XDIC_ADDR_MAX ; ++xdic_addr)
    {
        uint16_t* p_xd_reg_base_addr = gt_xdic_general_regs.ALL;

        switch (xdic_addr)
        {
        case XDIC_ADDR_LD_CONTROL :
            gt_xdic_general_regs._r01.ld_dir = XD_LD_DIR_TAIL_SHIFT;
            gt_xdic_general_regs._r01.pwm_res = gn_xd_pwm_res;
            gt_xdic_general_regs._r01.over_to_e = 0;
            gt_xdic_general_regs._r01.scan_no = gn_xd_scan_no;
            gt_xdic_general_regs._r01.io_mode = XD_IO_MODE_EXT_VYI_FBO;
            gt_xdic_general_regs._r01.ch_size = gn_xd_ch_size;
            break;
        case XDIC_ADDR_FPWM_DIVIDER :
            gt_xdic_general_regs._r02.fpwm_div = gn_xd_fpwm_div;
            break;
        case XDIC_ADDR_CHANNEL_ENABLE :
            gt_xdic_general_regs._r03.val = 0xFFF;
            break;
        case XDIC_ADDR_FAULT_LEVEL :
            gt_xdic_general_regs._r06.fb_level = gt_xd_fb_level;
            gt_xdic_general_regs._r06.short_level = gt_xd_short_level;
            gt_xdic_general_regs._r06.dev_max_curr_level = gt_xd_dev_max_curr_level;
            break;
        case XDIC_ADDR_FAULT_CONTROL :
            gt_xdic_general_regs._r07.o_off_e = 0;
            gt_xdic_general_regs._r07.s_off_e = 0;
            gt_xdic_general_regs._r07.t_off_e = 0;
            gt_xdic_general_regs._r07.s_det_e = 1;
            gt_xdic_general_regs._r07.o_det_e = 1;
            gt_xdic_general_regs._r07.o_fb_e = 0;
            gt_xdic_general_regs._r07.ms_vs_det_e = 0;
            gt_xdic_general_regs._r07.ms_vs_dimm = 0;
            gt_xdic_general_regs._r07.ms_vs_lock = 0;
            gt_xdic_general_regs._r07.timeout_e = 1;
            break;
        case XDIC_ADDR_MAX_CURR_VREF :
            gt_xdic_general_regs._r08.max_curr_vref = 0xFFF;
            break;
        case XDIC_ADDR_SERIAL_CLOCK_GEN :
            gt_xdic_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
            gt_xdic_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
            break;
        case XDIC_ADDR_SERIAL_LATENCY :
            gt_xdic_general_regs._r26.serial_latency = 60;
            break;
        case XDIC_ADDR_MCLK_LOCK_1 :
            gt_xdic_general_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0x000FFF) >>  0);
            break;
        case XDIC_ADDR_MCLK_LOCK_2 :
            gt_xdic_general_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0xFFF000) >> 12);
            gt_xdic_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_ENABLE;
            break;
            /*
        case XDIC_ADDR_TEMP :
            gt_xdic_general_regs._r29.flt_gain = 0;
            gt_xdic_general_regs._r29.flt_ctl = 1;
            gt_xdic_general_regs._r29.ofs_temp = 8;
            break;
            */
        case XDIC_ADDR_OSC_FLL_MANUAL_1 :
            gt_xdic_general_regs._r2A.osc_fll_man = 0x000;
            break;
        case XDIC_ADDR_OSC_FLL_MANUAL_2 :
            gt_xdic_general_regs._r2B.osc_fll_man = 8;
            gt_xdic_general_regs._r2B.osc_man_e = 0;
            break;
        /*case XDIC_ADDR_OSC_FLL_MONITOR :
            gt_xdic_general_regs._r2C.osc_fll_flt = 0;
            break;
        */
        default :
            continue;
        }
        XDIC_Write_General_Reg(xdic_addr, *(p_xd_reg_base_addr + xdic_addr));
    }

    XDIC_Set_Delay_CH();

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = XDIC_FB_IN_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(XDIC_FB_IN_GPIO_Port, &GPIO_InitStruct);

    XDIC_Read_All_Registers();
}

void XDIC_Trim_Param_Init(void)
{
    gt_xd_fb_level = FB_LEVEL_0V4;
    gt_xd_short_level = SHORT_LEVEL_36V;
    gt_xd_dev_max_curr_level = DEV_MAX_CURR_LEVEL_8mA;
}

void XDIC_Trim_Init(void)
{
    XDIC_Trim_Param_Init();

    JigBD_IF_Reset_Command();
    JigBD_IF_IdGen_Command();

    for (xdic_addr_t xdic_addr = XDIC_ADDR_RESET_ID ; xdic_addr < XDIC_ADDR_MAX ; ++xdic_addr)
    {
        uint16_t* p_xd_reg_base_addr = gt_xdic_general_regs.ALL;

        switch (xdic_addr)
        {
        case XDIC_ADDR_CHANNEL_ENABLE :
            gt_xdic_general_regs._r03.val = 0xFFF;
            break;
        case XDIC_ADDR_FAULT_LEVEL :
            gt_xdic_general_regs._r06.fb_level = gt_xd_fb_level;
            gt_xdic_general_regs._r06.short_level = gt_xd_short_level;
            gt_xdic_general_regs._r06.dev_max_curr_level = gt_xd_dev_max_curr_level;
            break;
        case XDIC_ADDR_MAX_CURR_VREF :
            gt_xdic_general_regs._r08.max_curr_vref = XDIC_VREF_TRIM_VREF;
            break;
        case XDIC_ADDR_SERIAL_CLOCK_GEN :
            gt_xdic_general_regs._r25.serial_clk_high = XD_SERIAL_CLK_CNT_HIGH;
            gt_xdic_general_regs._r25.serial_clk_low = XD_SERIAL_CLK_CNT_LOW;
            break;
        case XDIC_ADDR_SERIAL_LATENCY :
            gt_xdic_general_regs._r26.serial_latency = 60;
            break;
        case XDIC_ADDR_OTP_OP_MODE :
            gt_xdic_general_regs._r3F.test_en = 1;
            gt_xdic_general_regs._r3F.ddio_dis = 1;
            break;
        default :
            continue;
        }
        XDIC_Write_General_Reg(xdic_addr, *(p_xd_reg_base_addr + xdic_addr));
    }
    XDIC_Set_OTP_Protect(false);
    XDIC_Read_All_Registers();
}

/* ================================================================================================================================================= */
/* General Function */
/* ================================================================================================================================================= */
static void XDIC_Set_Delay_CH(void)
{
    uint16_t delay_per_ch = gn_xd_pwm_max_size / gn_xd_ch_size;
    uint16_t delay_msb_accumulator[2] = {0, };

    for (uint8_t ch = 0 ; ch < XD_CH_SIZE ; ++ch)
    {
        gn_xd_delay_ch[ch] = delay_per_ch * ch;
        uint16_t delay_lsb = ((gn_xd_delay_ch[ch] & 0x0FFF) >>  0);
        uint16_t delay_msb = ((gn_xd_delay_ch[ch] & 0x3000) >> 12);

        delay_msb_accumulator[ch / 6] |= (delay_msb << (2 * (ch % 6)));

        print(LOG_INFO, "[%s] delay_ch[%u] = %u / msb = %u / lsb = %u\r\n", __func__, ch, gn_xd_delay_ch[ch], delay_msb, delay_lsb);

        XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH01 + ch, delay_lsb);
    }

    XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_EXTEND_1, delay_msb_accumulator[0]);
    XDIC_Write_General_Reg(XDIC_ADDR_DELAY_CH_EXTEND_2, delay_msb_accumulator[1]);
}

void XDIC_Set_Max_Current_Level(dev_max_curr_level_t in_dev_max_curr)
{
    gt_xdic_general_regs._r06.dev_max_curr_level = in_dev_max_curr;
    XDIC_Write_General_Reg(XDIC_ADDR_FAULT_LEVEL, gt_xdic_general_regs._r06.val);
}

float XDIC_Get_Max_Current_level(void)
{
    float f_rtn = 0.0f;

    uint16_t fault_level = XDIC_Read_General_Reg(XDIC_ADDR_FAULT_LEVEL);
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

bool XDIC_Is_Vsync_Mode_External(void)
{
    if (gt_xdic_general_regs._r01.io_mode == XD_IO_MODE_EXT_VSYNC || gt_xdic_general_regs._r01.io_mode == XD_IO_MODE_EXT_VYI_FBO)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void XDIC_Set_OSC_Manual_En(bool in_Enable)
{
    if (in_Enable == TRUE)
    {
        gt_xdic_general_regs._r2B.osc_man_e = 1;
        XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_2, gt_xdic_general_regs._r2B.val);
    }
    else
    {
        gt_xdic_general_regs._r2B.osc_man_e = 0;
        XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_2, gt_xdic_general_regs._r2B.val);
    }
}

static void XDIC_Set_OSC_Manual(uint16_t n_osc_fll_man)
{
    uint16_t osc_fll_man_lsb = ((n_osc_fll_man & 0x0FFF) >>  0);
    uint16_t osc_fll_man_msb = ((n_osc_fll_man & 0xF000) >> 12);

    gt_xdic_general_regs._r2A.osc_fll_man = osc_fll_man_lsb;
    XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_1, gt_xdic_general_regs._r2A.val);

    gt_xdic_general_regs._r2B.osc_fll_man = osc_fll_man_msb;
    XDIC_Write_General_Reg(XDIC_ADDR_OSC_FLL_MANUAL_2, gt_xdic_general_regs._r2B.val);
}

void XDIC_Set_Max_Curr_Vref(uint16_t in_max_curr_vref)
{
    gt_xdic_general_regs._r08.max_curr_vref = in_max_curr_vref;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURR_VREF, gt_xdic_general_regs._r08.val);
}

void XDIC_Update_Vsync_Frequency(float n_freq)
{
    uint32_t prescale = LL_TIM_GetPrescaler(TIM8);
    uint32_t period = (uint32_t)((TIM8_FREQ / (prescale + 1)) / n_freq - 1 + 0.5f);

    LL_TIM_SetAutoReload(TIM8, period);

    gf_vsync_out = n_freq;

    // 1. disable mclk_lock_cnt_e
    gt_xdic_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_DISABLE;
    XDIC_Write_General_Reg(XDIC_ADDR_MCLK_LOCK_2, gt_xdic_general_regs._r28.val);

    // 2. increase or decrease fpwm_div
    if (gn_xd_pwm_res == XD_PWM_RES_12BIT)
    {
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 12)));
    }
    else //if (gn_xd_pwm_res == XD_PWM_RES_14BIT)
    {
        gn_xd_fpwm_div = (uint16_t)(((gf_xd_mclk / gf_vsync_out) / (1 << 14)));
    }
    gt_xdic_general_regs._r02.fpwm_div = gn_xd_fpwm_div;
    XDIC_Write_General_Reg(XDIC_ADDR_FPWM_DIVIDER, gt_xdic_general_regs._r02.val);

    // 3. change mclk_lock_cnt
    gn_xd_mclk_lock_cnt = (uint32_t)((XD_MCLK_LOCK_CNT_120Hz) * (VSYNC / gf_vsync_out) + 0.5f);
    gt_xdic_general_regs._r27.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0x000FFF) >>  0);
    gt_xdic_general_regs._r28.mclk_lock_cnt = ((gn_xd_mclk_lock_cnt & 0xFFF000) >> 12);

    XDIC_Write_General_Reg(XDIC_ADDR_MCLK_LOCK_1, gt_xdic_general_regs._r27.val);
    XDIC_Write_General_Reg(XDIC_ADDR_MCLK_LOCK_2, gt_xdic_general_regs._r28.val);

    // 4. enable mclk_lock_cnt_e
    gt_xdic_general_regs._r28.mclk_lock_cnt_e = XD_MCLK_FLL_ENABLE;
    XDIC_Write_General_Reg(XDIC_ADDR_MCLK_LOCK_2, gt_xdic_general_regs._r28.val);
}
/* ================================================================================================================================================= */
/* Trim Function */
/* ================================================================================================================================================= */

void XDIC_Save_Trim_Regs(void)
{
    for (uint8_t addr = XDIC_ADDR_TRIM_OTP_CRC ; addr < XDIC_ADDR_TRIM_MAX ; ++addr)
    {
        gn_xdic_saved_trim_reg[addr] = XDIC_Read_Mirror_Reg(addr);
    }

    print(LOG_INFO, "osc,%3u\r\n", gn_xdic_saved_trim_reg[1]);
    print(LOG_INFO, "vref,%3u\r\n", gn_xdic_saved_trim_reg[2]);

    print(LOG_INFO, "ictl_l,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n"
    , gn_xdic_saved_trim_reg[ 3], gn_xdic_saved_trim_reg[ 4], gn_xdic_saved_trim_reg[ 5] , gn_xdic_saved_trim_reg[ 6], gn_xdic_saved_trim_reg[ 7], gn_xdic_saved_trim_reg[ 8]
    , gn_xdic_saved_trim_reg[ 9], gn_xdic_saved_trim_reg[10], gn_xdic_saved_trim_reg[11] , gn_xdic_saved_trim_reg[12], gn_xdic_saved_trim_reg[13], gn_xdic_saved_trim_reg[14]
    );
    print(LOG_INFO, "ictl_h,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u,%3u\r\n"
    , gn_xdic_saved_trim_reg[27], gn_xdic_saved_trim_reg[28], gn_xdic_saved_trim_reg[29] , gn_xdic_saved_trim_reg[30], gn_xdic_saved_trim_reg[31], gn_xdic_saved_trim_reg[32]
    , gn_xdic_saved_trim_reg[33], gn_xdic_saved_trim_reg[34], gn_xdic_saved_trim_reg[35] , gn_xdic_saved_trim_reg[36], gn_xdic_saved_trim_reg[37], gn_xdic_saved_trim_reg[38]
    );
}

uint64_t XDIC_Compare_Trim_Regs(void)
{
    uint64_t ret = 0;
    uint16_t u16_reg_val = 0;

    for (xdic_trim_addr_t trim_addr = XDIC_ADDR_TRIM_OSC ; trim_addr < XDIC_ADDR_TRIM_MAX ; ++trim_addr)
    {
        u16_reg_val = XDIC_Read_Mirror_Reg(trim_addr);

        if (gn_xdic_saved_trim_reg[trim_addr] != u16_reg_val)
        {
            ret |= ((uint64_t)1 << trim_addr);
            print(LOG_ERROR, "%s %17s - NG", ANSI_FONT_RED, gs_xdic_mirror_regs_str[trim_addr]);
        }
        else
        {
            print(LOG_INFO, "%s %17s - OK", ANSI_FONT_GREEN, gs_xdic_mirror_regs_str[trim_addr]);
        }
        print(LOG_INFO, "   [0x%03X] - [0x%03X] %s\r\n", gn_xdic_saved_trim_reg[trim_addr], u16_reg_val, ANSI_FONT_NONE);
    }

    return ret;
}

void XDIC_Trim_Init_VREF_CTL(void)
{
    gt_xdic_general_regs._r3F.test_en = 1;
    gt_xdic_general_regs._r3F.vref_o = 1;
    gt_xdic_general_regs._r3F.mclk32_o = 0;
    gt_xdic_general_regs._r3F.pwm_full_o = 0;
    gt_xdic_general_regs._r3F.test_ana_en = 3;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);
}
void XDIC_Trim_Init_OSC(void)
{
    gt_xdic_general_regs._r3F.test_en = 1;
    gt_xdic_general_regs._r3F.vref_o = 0;
    gt_xdic_general_regs._r3F.mclk32_o = 1;
    gt_xdic_general_regs._r3F.pwm_full_o = 0;
    gt_xdic_general_regs._r3F.test_ana_en = 0;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    XDIC_Set_OSC_Manual_En(true);
    XDIC_Set_OSC_Manual(32768);
}

void XDIC_Trim_Init_ICTL(void)
{
    gt_xdic_general_regs._r3F.test_en = 1;
    gt_xdic_general_regs._r3F.vref_o = 0;
    gt_xdic_general_regs._r3F.mclk32_o = 0;
    gt_xdic_general_regs._r3F.pwm_full_o = 1;
    gt_xdic_general_regs._r3F.test_ana_en = 0;
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_OP_MODE, gt_xdic_general_regs._r3F.val);

    gt_xdic_general_regs._r08.max_curr_vref = XDIC_CURRENT_TRIM_VREF;
    XDIC_Write_General_Reg(XDIC_ADDR_MAX_CURR_VREF, gt_xdic_general_regs._r08.val);
}

void XDIC_Set_OTP_Protect(bool en)
{
    if (en == true)
    {
        gt_xdic_general_regs._r3E.protect_en = XDIC_OTP_PROTECT_ENABLE;
    }
    else
    {
        gt_xdic_general_regs._r3E.protect_en = XDIC_OTP_PROTECT_DISABLE;
    }
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_PROTECT, gt_xdic_general_regs._r3E.val);
}

void XDIC_Set_OTP_PG_Start(bool en)
{
    if (en == true)
    {
        gt_xdic_general_regs._r3D.otp_pg_start = 1;
    }
    else
    {
        gt_xdic_general_regs._r3D.otp_pg_start = 0;
    }
    XDIC_Write_General_Reg(XDIC_ADDR_OTP_RD_PROG, gt_xdic_general_regs._r3D.val);
}

void XDIC_Set_FLT_Gain(uint8_t flt_gain)
{
    gt_xdic_general_regs._r29.flt_gain = flt_gain;
    XDIC_Write_General_Reg(XDIC_ADDR_TEMP, gt_xdic_general_regs._r29.val);
}

void XDIC_Set_FLT_CTL(uint8_t flt_ctl)
{
    gt_xdic_general_regs._r29.flt_ctl = flt_ctl;
    XDIC_Write_General_Reg(XDIC_ADDR_TEMP, gt_xdic_general_regs._r29.val);
}