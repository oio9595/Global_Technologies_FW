/** @file xd12.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XD12_C__
#include "config.h"

#define OTP_PROTECT_DISABLE         (0xA5A)
#define OTP_PROTECT_ENABLE          (0x5A5)

/* Trimming spec */
#define XD12_ERR_RATE               (1.0/100)   /* +/-% */
#define XD12_OSC_TARGET             (14.7456)     /* MHz */
#define XD12_VREF_TARGET            (2.2)       /* V */

#if (XD12_TRIM_MAX_CURR == 8)
    #define XD12_OFS_ERR_RATE       (0.5/100)   /* +/-% */
    #define XD12_OFS_TARGET         (0.220472)  /* mA */
    #define XD12_OFS_P1             (2)
    #define XD12_OFS_P2             (5)

    #define XD12_GAIN_ERR_RATE      (0.5/100)   /* +/-% */
    #define XD12_GAIN_TARGET        (2.267717)  /* mA */
    #define XD12_GAIN_P1            (36)
    #define XD12_GAIN_P2            (0)
#elif (XD12_TRIM_MAX_CURR == 46)
    #define XD12_OFS_ERR_RATE       (0.5/100)   /* +/-% */
    #define XD12_OFS_TARGET         (1.267717)  /* mA */
    #define XD12_OFS_P1             (2)
    #define XD12_OFS_P2             (5)

    #define XD12_GAIN_ERR_RATE      (0.5/100)   /* +/-% */
    #define XD12_GAIN_TARGET        (13.039370)  /* mA */
    #define XD12_GAIN_P1            (36)
    #define XD12_GAIN_P2            (0)
#elif (XD12_TRIM_MAX_CURR == 4)
    #define XD12_OFS_ERR_RATE       (0.5/100)   /* +/-% */
    #define XD12_OFS_TARGET         (0.110236)  /* mA */
    #define XD12_OFS_P1             (2)
    #define XD12_OFS_P2             (5)

    #define XD12_GAIN_ERR_RATE      (0.5/100)   /* +/-% */
    #define XD12_GAIN_TARGET        (1.133858)  /* mA */
    #define XD12_GAIN_P1            (36)
    #define XD12_GAIN_P2            (0)
#else
    #error "XD12_TRIM_MAX_CURR is not defined!!!!"
#endif

#define XD12_DEFAULT_OSC        (0x40)
#define XD12_DEFAULT_VREF_CTL   (0x10)
#define XD12_DEFAULT_OFS        (0x40)
#define XD12_DEFAULT_GAIN       (0x40)


static _xd12_regs_t _xd12_regs_;
static _xd12_trim_regs_t _xd12_trim_regs_;

#ifdef __XD12_C__ 

static const char* gs_xd12_addr_str[XD12_ADDR_MAX] = 
{
    "RESET",
    "ID",
    "LD_MODE",
    "SF_PERIOD",
    "SF_SIZE",
    "SF_BFI",
    "LD_FIX_1",
    "LD_FIX_2",
    "MAX_CURR_VREF",
    "CHANNEL_ENABLE",
    "DUMMY",
    "FAULT_STATUS",
    "FAULT_LEVEL",
    "FAULT_MODE",
    "DELAY_CH1",
    "DELAY_CH2",
    "DELAY_CH3",
    "DELAY_CH4",
    "DELAY_CH5",
    "DELAY_CH6",
    "DELAY_CH7",
    "DELAY_CH8",
    "DELAY_CH9",
    "DELAY_CH10",
    "DELAY_CH11",
    "DELAY_CH12",
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
    "DUMMY",
    "DUMMY",
    "DUMMY",
    "OTP_ACCESS_1",
    "OTP_ACCESS_2",
    "OTP_WRITE",
    "OTP_RD/PROG",
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
    "XD12_REG_TYPE_NON_TRIM", 
    "XD12_REG_TYPE_TRIM", 
};
#endif //__XD12_C__ 

static XD12_REG_TYPE gt_xd12_reg_type = XD12_REG_TYPE_NON_TRIM;

static void XD12_Select_Reg_Type(XD12_REG_TYPE n_reg_type);
static void XD12_Write_REGISTER(uint8_t in_addr, uint16_t in_data, XD12_REG_TYPE n_reg_type);
static uint16_t XD12_Read_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type);
static uint16_t XD12_Get_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type);

static void XD12_OTP_PROTECT_Disable();
static void XD12_OTP_PROTECT_Enable();

static uint8_t XD12_Write_OSC(uint16_t in_val);
static uint8_t XD12_Write_VREF_CTL(uint16_t in_val);

static uint8_t XD12_Write_OFS_CHx(uint8_t in_ch, uint16_t in_val);
static uint8_t XD12_Write_GAIN_CHx(uint8_t in_ch, uint16_t in_val);
static uint8_t XD12_Write_OFS_TEMP(uint16_t in_val);

static void XD12_Select_Reg_Type(XD12_REG_TYPE n_reg_type)
{
    _xd12_op_mode_t* p_op_mode = &(_xd12_regs_._r3F);
    
    print(LOG_DEBUG, "[ op_mode.val : 0x%X)]\r\n", p_op_mode->val);
    if (n_reg_type == XD12_REG_TYPE_NON_TRIM)
    {
        p_op_mode->addr_ext = 0;
    }
    else if (n_reg_type == XD12_REG_TYPE_TRIM)
    {
        p_op_mode->addr_ext = 1;
    }    
    
    print(LOG_DEBUG, "[ Start Change XD Reg_Type to %s (op_mode.addr_ext : 0x%X)]\r\n", gs_xd12_reg_type[n_reg_type], p_op_mode->val);
    JigBd_IF_Write_Command(XD12_ADDR_OTP_OP_MODE, p_op_mode->val);
    print(LOG_DEBUG, "[  Done Change XD Reg_Type to %s (op_mode.addr_ext : 0x%X)]\r\n", gs_xd12_reg_type[n_reg_type], p_op_mode->val);
}

void XD12_Set_Reg_Type(XD12_REG_TYPE n_reg_type)
{
    if (n_reg_type < XD12_REG_TYPE_MAX)
    {
        gt_xd12_reg_type = n_reg_type;  
    }
}

XD12_REG_TYPE XD12_Get_Reg_Type(void)
{
    return gt_xd12_reg_type;
}

static void XD12_Write_REGISTER(uint8_t in_addr, uint16_t in_data, XD12_REG_TYPE n_reg_type)
{
    uint16_t* p_xd12_reg_addr = NULL;

    if (n_reg_type == XD12_REG_TYPE_TRIM)
    {
        if (OTP_PROTECT_DISABLE != XD12_Get_REGISTER(XD12_ADDR_OTP_PROTECT, XD12_REG_TYPE_NON_TRIM))
        {
            XD12_OTP_PROTECT_Disable();
        }
    }
    
    gt_xd12_reg_type = n_reg_type;
    XD12_Select_Reg_Type(n_reg_type);
    
    switch (n_reg_type)
    {
        case XD12_REG_TYPE_NON_TRIM : 
            p_xd12_reg_addr = _xd12_regs_.ALL;
            break;
        case XD12_REG_TYPE_TRIM : 
            p_xd12_reg_addr = _xd12_trim_regs_.ALL;
            break;
    }
    *(p_xd12_reg_addr + in_addr) = in_data;
    JigBd_IF_Write_Command(in_addr, *(p_xd12_reg_addr + in_addr)); 
}

static uint16_t XD12_Read_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type)
{
    uint16_t u16_reg_val = 0;    
    uint16_t* p_xd12_reg_addr = NULL;
    
    gt_xd12_reg_type = n_reg_type;
    XD12_Select_Reg_Type(n_reg_type);

    u16_reg_val = JigBd_IF_Read_Command(in_addr);

    switch (n_reg_type)
    {   
        case XD12_REG_TYPE_NON_TRIM : 
            p_xd12_reg_addr = _xd12_regs_.ALL;
            break;
        case XD12_REG_TYPE_TRIM : 
            p_xd12_reg_addr = _xd12_trim_regs_.ALL;
            break;
    }    
    *(p_xd12_reg_addr + in_addr) = u16_reg_val;
    print(LOG_DEBUG, "[%s][ 0x%02X : 0x%04X ]\r\n", gs_xd12_reg_type[n_reg_type], in_addr, *(p_xd12_reg_addr + in_addr));
    return u16_reg_val;
}

static uint16_t XD12_Get_REGISTER(uint8_t in_addr, XD12_REG_TYPE n_reg_type)
{
    uint16_t u16_reg_val = 0;
    uint16_t* p_xd12_reg_addr = NULL;
    
    switch (n_reg_type)
    {
        case XD12_REG_TYPE_NON_TRIM : 
            p_xd12_reg_addr = _xd12_regs_.ALL;
            break;
        case XD12_REG_TYPE_TRIM : 
            p_xd12_reg_addr = _xd12_trim_regs_.ALL;
            break;
    }
    
    u16_reg_val = *(p_xd12_reg_addr + in_addr);
    print(LOG_DEBUG, "\r\n XD12_Get_REGISTER(0x%2X, %s) = 0x%X \r\n", in_addr, gs_xd12_reg_type[n_reg_type], u16_reg_val);
    return u16_reg_val;
}

static void XD12_Print_XD12_REGS(uint8_t in_GUI_SUPPORT)
{
    uint8_t i=0;

    print(LOG_INFO, "\r\n");
    if(in_GUI_SUPPORT == _GUI_SUPPORT_)
    {
        print(LOG_INFO, CMD_XD12_READ_REGISTER_ALL);
        for(i = 0 ; i < XD12_ADDR_MAX ; ++i)
        {
            print(LOG_INFO, ":%02x:%03x", i, *(_xd12_regs_.ALL + i));
        }
        for(i = 0 ; i < XD12_ADDR_TRIM_MAX ; ++i)
        {
            print(LOG_INFO, ":%02x:%03x", i, *(_xd12_trim_regs_.ALL + i));
        }
        print(LOG_INFO, "\r\n");
    }
    else
    {
        for(i = 0 ; i < XD12_ADDR_MAX ; ++i)
        {
            print(LOG_INFO, "%s(0x%02X) : 0x%03X (%4u)\r\n", gs_xd12_addr_str[i], i, *(_xd12_regs_.ALL + i), *(_xd12_regs_.ALL + i));
        }
        for(i = 0 ; i < XD12_ADDR_TRIM_MAX ; ++i)
        {
            print(LOG_INFO, "%s(0x%02X) : 0x%03X (%4u)\r\n", gs_xd12_addr_trim_str[i], i, *(_xd12_trim_regs_.ALL + i), *(_xd12_trim_regs_.ALL + i));
        }
    }
}

static void XD12_OTP_PROTECT_Disable()
{
    _xd12_otp_protect_t* p_otp_protect = &(_xd12_regs_._r3E);
    
    if(OTP_PROTECT_DISABLE != p_otp_protect->protect_en)
    {
        p_otp_protect->protect_en = OTP_PROTECT_DISABLE;
        print(LOG_DEBUG, " Start OTP_PROTECT_DISABLE\r\n");
        JigBd_IF_Write_Command(XD12_ADDR_OTP_PROTECT, p_otp_protect->val);
        print(LOG_DEBUG, "  Done OTP_PROTECT_DISABLE\r\n");
    }
}

static void XD12_OTP_PROTECT_Enable()
{
    _xd12_otp_protect_t* p_otp_protect = &(_xd12_regs_._r3E);
    
    if(OTP_PROTECT_ENABLE != p_otp_protect->protect_en)
    {
        p_otp_protect->protect_en = OTP_PROTECT_ENABLE;
        print(LOG_DEBUG, " Start OTP_PROTECT_ENABLE\r\n");
        JigBd_IF_Write_Command(XD12_ADDR_OTP_PROTECT, p_otp_protect->val);
        print(LOG_DEBUG, "  Done OTP_PROTECT_ENABLE\r\n");
    }
}

static uint8_t XD12_Write_otp_pg_start(uint8_t in_val)
{
    _xd12_otp_rd_prog_t* p_otp_rd_prog = &(_xd12_regs_._r3D);

    if(in_val > 1)
    {
        print(LOG_ERROR, "XD12_Write_otp_pg_start() input[%d] is Over %d\r\n", in_val, 1);         
        return FALSE;
    }
    else
    {
        p_otp_rd_prog->otp_pg_start = in_val;
        print(LOG_DEBUG, " Start XD12_Write_otp_pg_start(%d)\r\n",in_val);
        XD12_Write_REGISTER(XD12_ADDR_OTP_RD_PROG, p_otp_rd_prog->val, XD12_REG_TYPE_NON_TRIM);
        print(LOG_DEBUG, "  Done XD12_Write_otp_pg_start(%d)\r\n",in_val);
        return TRUE;
    }
}

static void XD12_test_en_enable(uint8_t in_Enable)
{
    _xd12_op_mode_t* p_op_mode = &(_xd12_regs_._r3F);
    
    if(in_Enable == TRUE)
    {
        p_op_mode->test_en = 1;
    }
    else
    {
        p_op_mode->test_en = 0;
    }
    print(LOG_DEBUG, " Start XD12_test_en_enable(%d)\r\n", p_op_mode->test_en);
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_op_mode->val, XD12_REG_TYPE_NON_TRIM);
    print(LOG_DEBUG, "  Done XD12_test_en_enable(%d)\r\n", p_op_mode->test_en);
}

static void XD12_ld_fix_Enable(uint8_t in_Enable)
{
    _xd12_op_mode_t* p_op_mode = &(_xd12_regs_._r3F);
    
    if(in_Enable == TRUE)
    {
        p_op_mode->ld_fix_en = 1;
    }
    else
    {
        p_op_mode->ld_fix_en = 0;
    }
    print(LOG_DEBUG, " Start XD12_ld_fix_Enable(%d)\r\n", p_op_mode->ld_fix_en);
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_op_mode->val, XD12_REG_TYPE_NON_TRIM);
    print(LOG_DEBUG, "  Done XD12_ld_fix_Enable(%d)\r\n", p_op_mode->ld_fix_en);
}

static void XD12_Read_All(void)
{
    uint8_t u8_addr = 0;
    uint16_t u16_data = 0;

    print(LOG_INFO, "\r\n");
    
    /* Read XD12-Normal Registers */
    for(u8_addr = XD12_ADDR_RESET ; u8_addr < XD12_ADDR_MAX ; ++u8_addr)
    {
        print(LOG_DEBUG, "[ Start XD12 Non-Trim Reg Read %s - 0x%02X ]\r\n", gs_xd12_addr_str[u8_addr], u8_addr);
        u16_data = XD12_Read_REGISTER(u8_addr, XD12_REG_TYPE_NON_TRIM);
        print(LOG_DEBUG, "[  Done XD12 Non-Trim Reg Read %s - 0x%02X : 0x%03X ]\r\n", gs_xd12_addr_str[u8_addr], u8_addr, u16_data);
        print(LOG_DEBUG, "[ %s - 0x%02X : 0x%03X ]\r\n", gs_xd12_addr_str[u8_addr], u8_addr, u16_data);
    }
    
    /* Read XD12-TRIM Registers */
    for(u8_addr = XD12_ADDR_TRIM_OTP_CRC ; u8_addr < XD12_ADDR_TRIM_MAX ; ++u8_addr)
    {
        print(LOG_DEBUG, "[ Start XD12 Trim Reg Read %s - 0x%02X ]\r\n", gs_xd12_addr_trim_str[u8_addr], u8_addr);
        u16_data = XD12_Read_REGISTER(u8_addr, XD12_REG_TYPE_TRIM);
        print(LOG_DEBUG, "[  Done XD12 Trim Reg Read %s - 0x%02X : 0x%03X ]\r\n", gs_xd12_addr_trim_str[u8_addr], u8_addr, u16_data);
        print(LOG_DEBUG, "[ %s - 0x%02X : 0x%03X ]\r\n", gs_xd12_addr_trim_str[u8_addr], u8_addr, u16_data);
    }
}

static uint8_t XD12_Write_OSC(uint16_t in_val)
{
    _xd12_osc_t* p_xd12_osc = &_xd12_trim_regs_._r01;
    if(in_val > REG_LIMIT_OSC) // 7-bit
    {
        print(LOG_ERROR, "XD12_Write_OSC() input[%d] is Over %d\r\n", in_val, REG_LIMIT_OSC);         
        return FALSE;
    }
    else
    {
        p_xd12_osc->osc = in_val;
        print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_TRIM_OSC]: 0x%04X\r\n", p_xd12_osc->val);
        XD12_Write_REGISTER(XD12_ADDR_TRIM_OSC, p_xd12_osc->val, XD12_REG_TYPE_TRIM);
        print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_TRIM_OSC]: 0x%04X\r\n", p_xd12_osc->val);
        return TRUE;
    }
}

static uint8_t XD12_Write_VREF_CTL(uint16_t in_val)
{
    _xd12_vref_ctl_t* p_xd12_vref_ctl = &_xd12_trim_regs_._r02;
    if(in_val > REG_LIMIT_VREF) // 5-bit
    {
        print(LOG_ERROR, "XD12_Write_VREF_CTL() input[%d] is Over %d\r\n", in_val, REG_LIMIT_VREF); 
        return FALSE;
    }
    else
    {
        p_xd12_vref_ctl->vref_ctl = in_val;
        print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_TRIM_VREF_CTL]: 0x%04X\r\n", p_xd12_vref_ctl->val);
        XD12_Write_REGISTER(XD12_ADDR_TRIM_VREF_CTL, p_xd12_vref_ctl->val, XD12_REG_TYPE_TRIM);
        print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_TRIM_VREF_CTL]: 0x%04X\r\n", p_xd12_vref_ctl->val);
        return TRUE;
    }
}

static uint8_t XD12_Write_OFS_CHx(uint8_t in_ch, uint16_t in_val)
{
    _xd12_ofs_chx_t* p_xd12_ofs_ch = &_xd12_trim_regs_._r03 + in_ch;
    if(in_ch > CH_MAX)
    {
        print(LOG_ERROR, "XD12_Write_OFS_CH%d() input[%d] is Over %d\r\n", in_ch+1, in_val, REG_LIMIT_OFS);
        return FALSE;
    }

    if(in_val > REG_LIMIT_OFS) // 7-bit
    {
        print(LOG_ERROR, "XD12_Write_OFS_CH%d() input[%d] is Over %d\r\n", in_ch+1, in_val, REG_LIMIT_OFS);
        return FALSE;
    }
    else
    {
        p_xd12_ofs_ch->ofs_chx = in_val;
        print(LOG_DEBUG, " Start XD12_Write_OFS_CHx(%d, %d)\r\n", in_ch+1, p_xd12_ofs_ch->val); 
        XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH01 + in_ch, p_xd12_ofs_ch->val, XD12_REG_TYPE_TRIM);
        print(LOG_DEBUG, "  Done XD12_Write_OFS_CHx(%d, %d)\r\n", in_ch+1, p_xd12_ofs_ch->val); 
        return TRUE;
    }
}

static uint8_t XD12_Write_GAIN_CHx(uint8_t in_ch, uint16_t in_val)
{
    _xd12_gain_chx_t* p_xd12_gain_ch = &_xd12_trim_regs_._r1B + in_ch;
    if(in_ch > CH_MAX)
    {
        print(LOG_ERROR, "XD12_Write_GAIN_CHx%d() input[%d] is Over %d\r\n",in_ch+1,in_val, REG_LIMIT_GAIN);
        return FALSE;
    }

    if(in_val > REG_LIMIT_OFS) // 7-bit
    {
        print(LOG_ERROR, "XD12_Write_GAIN_CHx%d() input[%d] is Over %d\r\n",in_ch+1,in_val, REG_LIMIT_GAIN);
        return FALSE;
    }
    else
    {
        p_xd12_gain_ch->gain_chx = in_val;
        print(LOG_DEBUG, " Start XD12_Write_GAIN_CHx(%d, %d)\r\n", in_ch+1, p_xd12_gain_ch->val); 
        XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH01 + in_ch, p_xd12_gain_ch->val, XD12_REG_TYPE_TRIM);
        print(LOG_DEBUG, "  Done XD12_Write_GAIN_CHx(%d, %d)\r\n", in_ch+1, p_xd12_gain_ch->val); 
        return TRUE;
    }
}

static uint8_t XD12_Write_OFS_TEMP(uint16_t in_val)
{
    _xd12_ld_control_t* p_xd12_ld_control = &_xd12_regs_._r27;
    if(in_val > REG_LIMIT_OFS_TEMP) // 4-bit OFS_TEMP [3:0] b11 ~ b08
    {
        print(LOG_ERROR, "XD12_Write_OFS_TEMP() input[%d] is Over %d\r\n", in_val, REG_LIMIT_OFS_TEMP);
        return FALSE;
    }
    else
    {
        p_xd12_ld_control->ofs_temp = in_val;
        print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_LD_CONTROL]: 0x%04X\r\n", p_xd12_ld_control->val);
        XD12_Write_REGISTER(XD12_ADDR_LD_CONTROL, p_xd12_ld_control->val, XD12_REG_TYPE_NON_TRIM);
        print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_LD_CONTROL]: 0x%04X\r\n", p_xd12_ld_control->val);
        return TRUE;
    }
}

/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/
void TargetIC_IF_PowerOn_Init(void)
{
    for(trim_mode_t mode = TRIM_OSC_FREQUENCY ; mode < TRIM_MAX ; ++mode)
    {
        p_gui_param p = trim_get_param_gui();
        
        switch(mode)
        {
        case TRIM_OSC_FREQUENCY:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_OSC_TARGET);
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_OSC_TARGET + 0.1f);
            p[mode][TRIM_PARA_P1] = 0;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_VREF_CTL:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_VREF_TARGET * (1 - XD12_ERR_RATE));
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_VREF_TARGET * (1 + XD12_ERR_RATE));
            p[mode][TRIM_PARA_P1] = 0;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_OFS_CHS:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_OFS_TARGET * (1 - XD12_OFS_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_OFS_TARGET * (1 + XD12_OFS_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_P1] = XD12_OFS_P1;
            p[mode][TRIM_PARA_P2] = XD12_OFS_P2;
            break;
        case TRIM_GAIN_CHS:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_GAIN_TARGET * (1 - XD12_GAIN_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_GAIN_TARGET * (1 + XD12_GAIN_ERR_RATE)) / 1000;
            p[mode][TRIM_PARA_P1] = XD12_GAIN_P1;
            p[mode][TRIM_PARA_P2] = XD12_GAIN_P2;
            break;
        case TRIM_SCREENING:
            p[mode][TRIM_PARA_SCR_START] = 0;
            p[mode][TRIM_PARA_SCR_END]   = REG_LIMIT_LD_FIX;
            p[mode][TRIM_PARA_P1]        = 1;
            p[mode][TRIM_PARA_P2]        = 4;
            break;
        }
    }
}

void TargetIC_IF_set_short_level(uint8_t in_short_level)
{
    _xd12_fault_level_t* p_xd12_fault_level = &_xd12_regs_._r0C;
    
    switch (in_short_level)
    {
        case  0 : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_3V;
            break;
        case  1 : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_4V;
            break;
        case  2 : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_6V;
            break;
        case  3 : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_8V;
            break;
        case  4 : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_13V;
            break;
        case  5 : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_16V;
            break;
        case  6 : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_24V;
            break;
        case  7 : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_36V;
            break;
        default : 
            p_xd12_fault_level->short_level = SHORT_LEVEL_36V;
            break;
    }
    print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
    XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p_xd12_fault_level->val, XD12_REG_TYPE_NON_TRIM);
    print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
    print(LOG_DEBUG, " XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
}

void TargetIC_IF_set_FB_level(uint8_t in_fb_level)
{
    _xd12_fault_level_t* p_xd12_fault_level = &_xd12_regs_._r0C;   
    
    switch (in_fb_level)
    {
        case 0 : 
            p_xd12_fault_level->fb_level = FB_LEVEL_0V4;
            break;
        case 1 : 
            p_xd12_fault_level->fb_level = FB_LEVEL_0V5;
            break;
        case 2 : 
            p_xd12_fault_level->fb_level = FB_LEVEL_0V6;
            break;
        case 3 : 
            p_xd12_fault_level->fb_level = FB_LEVEL_0V7;
            break;
        case 4 : 
            p_xd12_fault_level->fb_level = FB_LEVEL_0V85;
            break;
        case 5 : 
            p_xd12_fault_level->fb_level = FB_LEVEL_1V0;
            break;
        case 6 : 
            p_xd12_fault_level->fb_level = FB_LEVEL_1V15;
            break;
        case 7 : 
            p_xd12_fault_level->fb_level = FB_LEVEL_1V3;
            break;        
        default : 
            p_xd12_fault_level->fb_level = FB_LEVEL_1V3;
            break;
    }
    print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
    XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p_xd12_fault_level->val, XD12_REG_TYPE_NON_TRIM);
    print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
    print(LOG_DEBUG, " XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
}

uint8_t TargetIC_IF_set_LD_FIX(uint16_t in_val)
{
    if(in_val > REG_LIMIT_LD_FIX)
    {
        print(LOG_ERROR, "TargetIC_IF_set_LD_FIX(%u) is Over %d\r\n", in_val); 
        return FALSE;
    }    

    _xd12_ld_fix_1_t* p_xd12_ld_fix_1 = &_xd12_regs_._r06;
    _xd12_ld_fix_2_t* p_xd12_ld_fix_2 = &_xd12_regs_._r07;
    
    p_xd12_ld_fix_1->ld_fix_PAM_LSB = ((in_val & 0x07) >> 0); //000_0111
    p_xd12_ld_fix_2->ld_fix_PAM_MSB = ((in_val & 0x78) >> 3); //111_1000
    
    print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_LD_FIX_1]: 0x%04X\r\n", p_xd12_ld_fix_1->val);
    XD12_Write_REGISTER(XD12_ADDR_LD_FIX1, p_xd12_ld_fix_1->val, XD12_REG_TYPE_NON_TRIM);  /*  PAM_LSB(3bit) + DUTY(9bit) */
    print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_LD_FIX_1]: 0x%04X\r\n", p_xd12_ld_fix_1->val);
    print(LOG_DEBUG, " XD12_REG[XD12_ADDR_LD_FIX_1]: 0x%04X\r\n", p_xd12_ld_fix_1->val);
    
    print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_LD_FIX_2]: 0x%04X\r\n", p_xd12_ld_fix_2->val);
    XD12_Write_REGISTER(XD12_ADDR_LD_FIX2, p_xd12_ld_fix_2->val, XD12_REG_TYPE_NON_TRIM);  /*  PAM_MSB(4bit) */
    print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_LD_FIX_2]: 0x%04X\r\n", p_xd12_ld_fix_2->val);
    print(LOG_DEBUG, " XD12_REG[XD12_ADDR_LD_FIX_2]: 0x%04X\r\n", p_xd12_ld_fix_2->val);
    return TRUE;
}

uint8_t TargetIC_IF_set_FAULT_LEVEL(dev_max_curr_level_t in_dev_max_curr, short_level_t in_short_level, fb_level_t in_fb_level)
{
    if(in_dev_max_curr > REG_LIMIT_DEV_MAX_CURR_LEVEL)
    {
        print(LOG_ERROR, "TargetIC_IF_set_FAULT_LEVEL() dev_max_curr input[%d] is Over %d\r\n", in_dev_max_curr, REG_LIMIT_DEV_MAX_CURR_LEVEL); 
        return FALSE;
    }

    if(in_short_level > REG_LIMIT_SHORT_LEVEL)
    {
        print(LOG_ERROR, "TargetIC_IF_set_FAULT_LEVEL() short_level input[%d] is Over %d\r\n", in_short_level, REG_LIMIT_SHORT_LEVEL); 
        return FALSE;
    }

    if(in_fb_level > REG_LIMIT_FB_LEVEL)
    {
        print(LOG_ERROR, "TargetIC_IF_set_FAULT_LEVEL() fb_level is Over %d\r\n", in_fb_level, REG_LIMIT_FB_LEVEL); 
        return FALSE;
    }
    
    _xd12_fault_level_t* p_xd12_fault_level = &_xd12_regs_._r0C;
    
    p_xd12_fault_level->dev_max_curr_level = in_dev_max_curr;
    p_xd12_fault_level->short_level = in_short_level;
    p_xd12_fault_level->fb_level = in_fb_level;

    print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
    XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p_xd12_fault_level->val, XD12_REG_TYPE_NON_TRIM);
    print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
    print(LOG_DEBUG, " XD12_REG[XD12_ADDR_FAULT_LEVEL]: 0x%04X\r\n", p_xd12_fault_level->val);
    return TRUE;
}

uint8_t TargetIC_IF_set_CHANNEL_ENABLE(uint8_t in_channel)
{
    _xd12_channel_enable_t* p_xd12_channel_enable = &_xd12_regs_._r09;    

    if(in_channel < CH_MAX)
    {
        // CHANNEL ENABLE
        p_xd12_channel_enable->val = (1 << in_channel);
        print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
        XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, p_xd12_channel_enable->val, XD12_REG_TYPE_NON_TRIM);
        print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
        print(LOG_DEBUG, " XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
        return TRUE;
    }
    else
    {
        print(LOG_ERROR, "ERROR:input is over CH_MAX: TargetIC_IF_set_CHANNEL_ENABLE(%d)\r\n", in_channel);
        return FALSE;
    }
}

void TargetIC_IF_set_CHANNEL_DISABLE_All(void)
{
    _xd12_channel_enable_t* p_xd12_channel_enable = &_xd12_regs_._r09;
    p_xd12_channel_enable->val = 0x00;
    print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
    XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, p_xd12_channel_enable->val, XD12_REG_TYPE_NON_TRIM);
    print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
    print(LOG_DEBUG, " XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
}

void TargetIC_IF_set_CHANNEL_ENABLE_All(void)
{
    _xd12_channel_enable_t* p_xd12_channel_enable = &_xd12_regs_._r09;
    p_xd12_channel_enable->val = 0xFFF;
    print(LOG_DEBUG, " Start XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
    XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, p_xd12_channel_enable->val, XD12_REG_TYPE_NON_TRIM);
    print(LOG_DEBUG, "  Done XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
    print(LOG_DEBUG, " XD12_REG[XD12_ADDR_CHANNEL_ENABLE]: 0x%04X\r\n", p_xd12_channel_enable->val);
}

void TargetIC_IF_Trim_Reg_Init(void)
{
    XD12_Write_REGISTER(XD12_ADDR_OTP_PROTECT, 0xA5A, XD12_REG_TYPE_NON_TRIM); //turn-off otp protect function
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OSC, 18, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_VREF_CTL, 17, XD12_REG_TYPE_TRIM);
    
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH01, 55, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH02, 58, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH03, 57, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH04, 53, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH05, 53, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH06, 52, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH07, 59, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH08, 59, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH09, 61, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH10, 52, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH11, 61, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_OFS_CH12, 59, XD12_REG_TYPE_TRIM);
    
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH01, 59, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH02, 58, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH03, 59, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH04, 60, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH05, 57, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH06, 59, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH07, 64, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH08, 65, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH09, 65, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH10, 58, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH11, 64, XD12_REG_TYPE_TRIM);
    XD12_Write_REGISTER(XD12_ADDR_TRIM_GAIN_CH12, 63, XD12_REG_TYPE_TRIM);
}

uint16_t TargetIC_IF_Inc_Screen_CurValue(uint16_t in_u16_screen_input_cur)
{
    uint16_t temp_step = 0;
    uint16_t u16_50_PERCENT_POINT_OF_LD_FIX = 0;
    uint16_t u16_MAX_OF_LD_FIX = 0;

    u16_50_PERCENT_POINT_OF_LD_FIX = LD_FIX_50_PERCENT_POINT;
    u16_MAX_OF_LD_FIX = REG_LIMIT_LD_FIX;

    if(in_u16_screen_input_cur < u16_50_PERCENT_POINT_OF_LD_FIX)
    {
        temp_step = (uint16_t)trim_get_param_gui()[TRIM_SCREENING][TRIM_PARA_P1];
    }
    else
    {
        temp_step = (uint16_t)trim_get_param_gui()[TRIM_SCREENING][TRIM_PARA_P2];
    }

    if(in_u16_screen_input_cur + temp_step >= u16_MAX_OF_LD_FIX)
    {
        in_u16_screen_input_cur = u16_MAX_OF_LD_FIX;
    }
    else
    {
        in_u16_screen_input_cur += temp_step;
    }

    return in_u16_screen_input_cur;
}

void TargetIC_IF_Initialize(void)
{    
    JigBd_IF_xd12_reset_Command();
    us_tdelay(5);

    JigBd_IF_IdGen_Command();
    LL_mDelay(10);

    TargetIC_IF_Serial_Clock_Init();
    TargetIC_IF_MAX_CURR_VREF_Init();    
    TargetIC_IF_set_CHANNEL_ENABLE_All();   
    TargetIC_IF_Fault_Level_Init();
    TargetIC_IF_LD_CONTROL_Init();
    TargetIC_IF_Fault_Mode_Init();
    
    Trim_IF_GUI_OTP_WRITE_enable(1);
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

void TargetIC_IF_Read_Resgister_All(uint8_t in_GUI_SUPPORT)
{
    XD12_Read_All();
    XD12_Print_XD12_REGS(in_GUI_SUPPORT);
}

void TargetIC_IF_Show_RegAll(void)
{    
    uint16_t* p_xd12_reg_addr = NULL;
    
    p_xd12_reg_addr = _xd12_regs_.ALL;
    for(int i = XD12_ADDR_RESET ; i < XD12_ADDR_MAX ; ++i)
    {
        print(LOG_INFO, "[ %18s(0x%02X) - 0x%03X (%4u) ]\r\n", gs_xd12_addr_str[i], i, *(p_xd12_reg_addr + i), *(p_xd12_reg_addr + i));
    }
    
    p_xd12_reg_addr = _xd12_trim_regs_.ALL;
    for(int i = XD12_ADDR_TRIM_OTP_CRC ; i < XD12_ADDR_TRIM_MAX ; ++i)
    {
        print(LOG_INFO, "[ %18s(0x%02X) - 0x%03X (%4u) ]\r\n", gs_xd12_addr_trim_str[i], i, *(p_xd12_reg_addr + i), *(p_xd12_reg_addr + i));
    }
}

void TargetIC_IF_Show_Trim_Reg(void)
{               
    uint16_t reg_temp[26] = {0, };
    reg_temp[0] = TargetIC_IF_Read_Register(XD12_ADDR_TRIM_OSC, XD12_REG_TYPE_TRIM);
    reg_temp[1] = TargetIC_IF_Read_Register(XD12_ADDR_TRIM_VREF_CTL, XD12_REG_TYPE_TRIM);
    print(LOG_INFO, "osc:%3u\r\n", reg_temp[0]);
    print(LOG_INFO, "vref:%3u\r\n", reg_temp[1]);

    for (uint8_t ch = 0 ; ch < CH_MAX ; ++ch)
    {
        reg_temp[ch + 2] = TargetIC_IF_Read_Register(XD12_ADDR_TRIM_OFS_CH01 + ch, XD12_REG_TYPE_TRIM);
        reg_temp[ch + 2 + CH_MAX] = TargetIC_IF_Read_Register(XD12_ADDR_TRIM_GAIN_CH01 + ch, XD12_REG_TYPE_TRIM);
    }

    print(LOG_INFO, "ofs:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u\r\n"
    , reg_temp[2], reg_temp[3], reg_temp[4] , reg_temp[5], reg_temp[6], reg_temp[7]
    , reg_temp[8], reg_temp[9], reg_temp[10] , reg_temp[11], reg_temp[12], reg_temp[13]
    );
    print(LOG_INFO, "gain:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u:%3u\r\n"
    , reg_temp[2 + CH_MAX], reg_temp[3 + CH_MAX], reg_temp[4 + CH_MAX] , reg_temp[5 + CH_MAX], reg_temp[6 + CH_MAX], reg_temp[7 + CH_MAX]
    , reg_temp[8 + CH_MAX], reg_temp[9 + CH_MAX], reg_temp[10 + CH_MAX] , reg_temp[11 + CH_MAX], reg_temp[12 + CH_MAX], reg_temp[13 + CH_MAX]
    );
}

void TargetIC_IF_Write_OTP(void)
{
    // ID GEN
    JigBd_IF_IdGen_Command();

    // OTP PROTECT - Disable
    XD12_OTP_PROTECT_Disable();

    // otp_pg_access_cycle [15:12] : 0
    _xd12_otp_access_1_t* p_xd12_otp_access_1 = &_xd12_regs_._r3A;
    p_xd12_otp_access_1->otp_pg_access_cycle = 0;
    XD12_Write_REGISTER(XD12_ADDR_OTP_ACCESS_1, p_xd12_otp_access_1->val, XD12_REG_TYPE_NON_TRIM);
    
    // otp_pg_access_cycle [11:0] : 320
    _xd12_otp_access_2_t* p_xd12_otp_access_2 = &_xd12_regs_._r3B;
    p_xd12_otp_access_2->otp_pg_access_cycle = 320;
    XD12_Write_REGISTER(XD12_ADDR_OTP_ACCESS_2, p_xd12_otp_access_2->val, XD12_REG_TYPE_NON_TRIM);
    
    // otp_wsel [3:0] : 4
    _xd12_otp_write_t* p_xd12_otp_write = &_xd12_regs_._r3C;
    p_xd12_otp_write->otp_wsel = 4;
    XD12_Write_REGISTER(XD12_ADDR_OTP_WRITE, p_xd12_otp_write->val, XD12_REG_TYPE_NON_TRIM);

    /* Write Trim 0x00 ~ 0x26 *************/
    for(uint8_t u8_addr = XD12_ADDR_TRIM_OSC ; u8_addr < XD12_ADDR_TRIM_MAX; ++u8_addr)
    {
        XD12_Write_REGISTER(u8_addr, *(_xd12_trim_regs_.ALL + u8_addr), XD12_REG_TYPE_TRIM);
    }

    // otp_pg_start : 1
    XD12_Write_otp_pg_start(1);
    HAL_Delay(1000);
    // otp_pg_start : 0
    XD12_Write_otp_pg_start(0);

    // OTP PROTECT - Enable
    XD12_OTP_PROTECT_Enable();
}

void TargetIC_IF_Write_OTP_Start(void)
{
    // ID GEN
    JigBd_IF_IdGen_Command();

    // OTP PROTECT - Disable
    XD12_OTP_PROTECT_Disable();

    // otp_pg_access_cycle [15:12] : 0
    _xd12_otp_access_1_t* p_xd12_otp_access_1 = &_xd12_regs_._r3A;
    p_xd12_otp_access_1->otp_pg_access_cycle = 0;
    XD12_Write_REGISTER(XD12_ADDR_OTP_ACCESS_1, p_xd12_otp_access_1->val, XD12_REG_TYPE_NON_TRIM);
    
    // otp_pg_access_cycle [11:0] : 320
    _xd12_otp_access_2_t* p_xd12_otp_access_2 = &_xd12_regs_._r3B;
    p_xd12_otp_access_2->otp_pg_access_cycle = 320;
    XD12_Write_REGISTER(XD12_ADDR_OTP_ACCESS_2, p_xd12_otp_access_2->val, XD12_REG_TYPE_NON_TRIM);
    
    // otp_wsel [3:0] : 4
    _xd12_otp_write_t* p_xd12_otp_write = &_xd12_regs_._r3C;
    p_xd12_otp_write->otp_wsel = 4;
    XD12_Write_REGISTER(XD12_ADDR_OTP_WRITE, p_xd12_otp_write->val, XD12_REG_TYPE_NON_TRIM);

    /* Write OTP 0x26 ~ 0x3F *************/
    for(uint8_t u8_addr = XD12_ADDR_TRIM_OSC ; u8_addr < XD12_ADDR_TRIM_MAX ; ++u8_addr)
    {
        XD12_Write_REGISTER(u8_addr, *(_xd12_trim_regs_.ALL + u8_addr), XD12_REG_TYPE_TRIM);
    }
    
    // otp_pg_start : 1
    XD12_Write_otp_pg_start(1);
}

void TargetIC_IF_Write_OTP_End(void)
{
    // otp_pg_start : 0
    XD12_Write_otp_pg_start(0);

    // OTP PROTECT - Enable
    XD12_OTP_PROTECT_Enable();
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
    uint16_t rtn_val;
    switch(in_trim_mode)
    {
        case TRIM_OSC_FREQUENCY:
            {
                _xd12_osc_t* p_xd12_osc = &_xd12_trim_regs_._r01;
                rtn_val = p_xd12_osc->val;
                print(LOG_DEBUG, "GET-OSC : %d\r\n", rtn_val);
            }
            break;
        
        case TRIM_VREF_CTL:
            {
                _xd12_vref_ctl_t* p_xd12_vref_ctl = &_xd12_trim_regs_._r02;
                rtn_val = p_xd12_vref_ctl->val;
                print(LOG_DEBUG, "GET-VREF : %d\r\n", rtn_val);
            }
            break;
        
        case TRIM_OFS_CHS:
            {
                _xd12_ofs_chx_t* p_xd12_ofs_chx = &_xd12_trim_regs_._r03 + ch_num;
                rtn_val = p_xd12_ofs_chx->val;
                print(LOG_DEBUG, "GET-OFS_CH%02d : %d\r\n", ch_num + 1, rtn_val);
            }
            break;
        
        case TRIM_GAIN_CHS:
            {
                _xd12_gain_chx_t* p_xd12_gain_chx = &_xd12_trim_regs_._r1B + ch_num;
                rtn_val = p_xd12_gain_chx->val;
                print(LOG_DEBUG, "GET-GAIN_CH%02d : %d\r\n", ch_num+ 1 , rtn_val);
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
            
        case TRIM_OFS_CHS:
            rtn_val = REG_LIMIT_OFS;
            print(LOG_DEBUG, "OFS_CH%02d_LIMIT : %d\r\n", ch_num + 1, rtn_val);
            break;
            
        case TRIM_GAIN_CHS:
            rtn_val = REG_LIMIT_GAIN;
            print(LOG_DEBUG, "GAIN_CH%02d_LIMIT : %d\r\n", ch_num + 1, rtn_val);
            break;
    }
    return rtn_val;
}

void TargetIC_IF_TRIM_Init_OSC(void)
{
    XD12_test_en_enable(TRUE);
}

void TargetIC_IF_TRIM_Init_VREF_CTL(void)
{
    XD12_test_en_enable(TRUE);
}
void TargetIC_IF_TRIM_Init_OFS(void)
{
    XD12_test_en_enable(FALSE);
    XD12_ld_fix_Enable(TRUE);
}

void TargetIC_IF_TRIM_Init_GAIN(void)
{
    XD12_test_en_enable(FALSE);
    XD12_ld_fix_Enable(TRUE);
}

void TargetIC_IF_SEND_otp_written(void)
{
    uint16_t* p_xd12_reg_addr = NULL;
    p_xd12_reg_addr = _xd12_trim_regs_.ALL;
    
    for(int i = XD12_ADDR_TRIM_OSC ; i < XD12_ADDR_TRIM_MAX ; ++i)
    {
        print(LOG_INFO, ":%02x:%03x", i, *(p_xd12_reg_addr + i));
    }
    print(LOG_INFO, "\r\n%s\r\n", CMD_XD12_OTP_WRITTEN);
}

void TargetIC_IF_Fault_Level_Init(void)
{
    _xd12_fault_level_t* p_xd12_fault_level = &_xd12_regs_._r0C;

#if (XD12_TRIM_MAX_CURR == 8)
    p_xd12_fault_level->dev_max_curr_level = DEV_MAX_CURR_LEVEL_8mA;
#elif (XD12_TRIM_MAX_CURR == 46)
    p_xd12_fault_level->dev_max_curr_level = DEV_MAX_CURR_LEVEL_46mA;
#elif (XD12_TRIM_MAX_CURR == 4)
    p_xd12_fault_level->dev_max_curr_level = DEV_MAX_CURR_LEVEL_4mA;
#else
    #error "XD12_TRIM_MAX_CURR is not defined!!!!"
#endif

    p_xd12_fault_level->short_level = SHORT_LEVEL_36V;
    p_xd12_fault_level->fb_level = FB_LEVEL_0V7;
        
    XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p_xd12_fault_level->val, XD12_REG_TYPE_NON_TRIM);
}

void TargetIC_IF_Serial_Clock_Init(void)
{   
    _xd12_serial_clock_gen_t* p_xd12_serial_clock_gen = &_xd12_regs_._r26;
    
    p_xd12_serial_clock_gen->serial_clk_high = SERIAL_CLK_CNT_HIGH;
    p_xd12_serial_clock_gen->serial_clk_low = SERIAL_CLK_CNT_LOW;
    
    XD12_Write_REGISTER(XD12_ADDR_SERIAL_CLOCK_GEN, p_xd12_serial_clock_gen->val, XD12_REG_TYPE_NON_TRIM);
}

void TargetIC_IF_MAX_CURR_VREF_Init(void)
{
    _xd12_max_current_vref_t* p_xd12_max_current_vref = &_xd12_regs_._r08;
    
#if (XD12_TRIM_MAX_CURR == 8)
    p_xd12_max_current_vref->max_curr_vref = 0xFFF;
#elif (XD12_TRIM_MAX_CURR == 46)
    p_xd12_max_current_vref->max_curr_vref = 0xFFF;
#elif (XD12_TRIM_MAX_CURR == 4)
    p_xd12_max_current_vref->max_curr_vref = 0xFFF;
#else
    #error "XD12_TRIM_MAX_CURR is not defined!!!!"
#endif
        
    XD12_Write_REGISTER(XD12_ADDR_MAX_CURR_VREF, p_xd12_max_current_vref->val, XD12_REG_TYPE_NON_TRIM);
}

void TargetIC_IF_LD_CONTROL_Init(void)
{
    _xd12_ld_control_t* p_xd12_ld_control = &_xd12_regs_._r27;

    p_xd12_ld_control->ofs_temp = 8;
    p_xd12_ld_control->ictl_glb = 4;
    p_xd12_ld_control->bgr_ctl = 8;
        
    XD12_Write_REGISTER(XD12_ADDR_LD_CONTROL, p_xd12_ld_control->val, XD12_REG_TYPE_NON_TRIM);
}

void TargetIC_IF_Fault_Mode_Init(void)
{
    _xd12_fault_mode_t* p_xd12_fault_mode = &_xd12_regs_._r0D;
    
    p_xd12_fault_mode->val = 0;
        
    XD12_Write_REGISTER(XD12_ADDR_FAULT_MODE, p_xd12_fault_mode->val, XD12_REG_TYPE_NON_TRIM);
}

void TargetIC_IF_Test_Enable(void)
{
    _xd12_op_mode_t* p_xd12_op_mode = &_xd12_regs_._r3F;
    
    p_xd12_op_mode->test_en = 1;
    
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_xd12_op_mode->val, XD12_REG_TYPE_NON_TRIM);
}

void TargetIC_IF_LD_FIX_Enable(void)
{
    _xd12_op_mode_t* p_xd12_op_mode = &_xd12_regs_._r3F;
    
    p_xd12_op_mode->ld_fix_en = 1;
    
    XD12_Write_REGISTER(XD12_ADDR_OTP_OP_MODE, p_xd12_op_mode->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_Write_sf_period(uint16_t in_sf_period)
{
    _xd12_sf_period_t* p_xd12_sf_period = &_xd12_regs_._r03;
    
    p_xd12_sf_period->sf_period = in_sf_period;
    
    XD12_Write_REGISTER(XD12_ADDR_SF_PERIOD, p_xd12_sf_period->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_Write_sf_size(uint16_t in_sf_x8_size, uint16_t in_sf_size)
{
    _xd12_sf_size_t* p_xd12_sf_size = &_xd12_regs_._r04;
    
    p_xd12_sf_size->sf_x8_size = in_sf_x8_size;
    p_xd12_sf_size->sf_size = in_sf_size;
    
    XD12_Write_REGISTER(XD12_ADDR_SF_SIZE, p_xd12_sf_size->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_Write_sf_bfi(uint16_t in_sf_x8_size, uint16_t in_sf_bfi)
{
    _xd12_sf_bfi_t* p_xd12_sf_bfi = &_xd12_regs_._r05;
    
    p_xd12_sf_bfi->sf_x8_size = in_sf_x8_size;
    p_xd12_sf_bfi->sf_bfi = in_sf_bfi;

    print(LOG_DEBUG, "\r\n in_sf_x8_size [%u] [0x%X]", in_sf_x8_size, in_sf_x8_size);
    print(LOG_DEBUG, "\r\n in_sf_bfi [%u] [0x%X]", in_sf_bfi, in_sf_bfi);
    
    print(LOG_DEBUG, "\r\n p_xd12_sf_bfi->sf_x8_size [%u] [0x%X]", p_xd12_sf_bfi->sf_x8_size, p_xd12_sf_bfi->sf_x8_size);
    print(LOG_DEBUG, "\r\n p_xd12_sf_bfi->sf_bfi [%u] [0x%X]\r\n", p_xd12_sf_bfi->sf_bfi, p_xd12_sf_bfi->sf_bfi);
    
    XD12_Write_REGISTER(XD12_ADDR_SF_BFI, p_xd12_sf_bfi->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_Write_dclk_period(uint16_t in_dclk_period)
{
    _xd12_dclk_period_t* p_xd12_dclk_period = &_xd12_regs_._r28;
    
    p_xd12_dclk_period->dclk_period = in_dclk_period;
    
    XD12_Write_REGISTER(XD12_ADDR_DCLK_PERIOD, p_xd12_dclk_period->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_Write_ld_mode(_xd12_ld_mode_t in_xd12_ld_mode)
{
    _xd12_ld_mode_t* p_xd12_ld_mode = &_xd12_regs_._r02;
    
    p_xd12_ld_mode->normal = in_xd12_ld_mode.normal;
    p_xd12_ld_mode->x8 = in_xd12_ld_mode.x8;
    p_xd12_ld_mode->bfi = in_xd12_ld_mode.bfi;
    
    XD12_Write_REGISTER(XD12_ADDR_LD_MODE, p_xd12_ld_mode->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_Write_fault_mode(_xd12_fault_mode_t in_xd12_fault_mode)
{
    _xd12_fault_mode_t* p_xd12_fault_mode = &_xd12_regs_._r0D;
    
    p_xd12_fault_mode->o_off_en = in_xd12_fault_mode.o_off_en;
    p_xd12_fault_mode->s_off_en = in_xd12_fault_mode.s_off_en;
    p_xd12_fault_mode->t_off_en = in_xd12_fault_mode.t_off_en;
    p_xd12_fault_mode->s1_en = in_xd12_fault_mode.s1_en;
    p_xd12_fault_mode->s2_en = in_xd12_fault_mode.s2_en;
    p_xd12_fault_mode->timeout_en = in_xd12_fault_mode.timeout_en;
    
    XD12_Write_REGISTER(XD12_ADDR_FAULT_MODE, p_xd12_fault_mode->val, XD12_REG_TYPE_NON_TRIM);
}

void XD12_Write_ld_control(_xd12_ld_control_t in_xd12_ld_control)
{
    _xd12_ld_control_t* p_xd12_ld_control = &_xd12_regs_._r27;

    p_xd12_ld_control->ofs_temp = in_xd12_ld_control.ofs_temp;
    p_xd12_ld_control->ictl_glb = in_xd12_ld_control.ictl_glb;
    p_xd12_ld_control->bgr_ctl = in_xd12_ld_control.bgr_ctl;
    
    XD12_Write_REGISTER(XD12_ADDR_LD_CONTROL, p_xd12_ld_control->val, XD12_REG_TYPE_NON_TRIM);
}

float XD12_get_max_current_level(void)
{
    float f_rtn = 0;
    
    _xd12_fault_level_t xd12_fault_level;
    xd12_fault_level.val = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_LEVEL, XD12_REG_TYPE_NON_TRIM);
    
    switch (xd12_fault_level.dev_max_curr_level)
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

/* END - INTERFACE FUNCTIONS ************************************************************************/

/* BEGIN - DBG_TEST SUPPORT *************************************************************/
#ifdef DBG_TEST
void _dbg_XD12_Print_RegisterMap(void)
{
#if _DBG_XD12
    for(uint8_t i = XD12_ADDR_RESET ; i < XD12_ADDR_MAX ; ++i)
    {
        print(LOG_DEBUG, "0x%02X-XD12_ADDR_%s\r\n", i, gs_xd12_addr_str[i]); 
    }
    
    for(uint8_t i = XD12_ADDR_TRIM_OTP_CRC ; i < XD12_ADDR_TRIM_MAX ; ++i)
    {
        print(LOG_DEBUG, "0x%02X-XD12_ADDR_%s\r\n", i, gs_xd12_addr_trim_str[i]); 
    }
#endif
}

uint16_t TargetIC_IF_Read_Command_through_PWM_to_XD12(uint8_t in_addr, XD12_REG_TYPE n_reg_type)
{
    return XD12_Read_REGISTER(in_addr, n_reg_type);
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
    XD12_test_en_enable(TRUE);

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
