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

#define _Y__                    (0x01)
static const uint8_t XD12_REGISTER_MAP[XD12_ADDR_MAX]=
{
    /*          0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F */
    /*0x00*/    _Y__, _Y__, 0x00, _Y__, _Y__, 0x00, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, 
    /*0x10*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /*0x20*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, 
    /*0x30*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__ ,
};

/* Trimming spec */
#define XD12_ERR_RATE           (1.5/100)   /* +/-% */
#define XD12_OSC_TARGET         (14.75)    /* MHz */
#define XD12_VREF_TARGET        (2.2)       /* V */

#define XD12_GLB_TARGET         (32)        /* mA */
#define XD12_OFS1_ERR_RATE      (0.5/100)   /* +/-% */
#define XD12_OSF1_TARGET        (0.141176)    /* mA */
#define XD12_OFS2_ERR_RATE      (0.5/100)   /* +/-% */
#define XD12_OSF2_TARGET        (1.7569)    /* mA */

#define XD12_DEFAULT_OSC        (0x50)
#define XD12_DEFAULT_VREF_CTL   (32)
#define XD12_DEFAULT_GLB        (4)
#define XD12_DEFAULT_OFS1       (128)
#define XD12_DEFAULT_OFS2       (128)

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t dummy               :16;
    }u;
}_xd12_dummy_t;

/* XD12 TYPER_1 */
typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_pg_access_cycle : 4; // 3 ~ 0
        uint16_t                     : 7; // 10 ~ 4
        uint16_t test_en             : 1; // 11
        uint16_t                     : 4;
    }u;
}_xd12_trim1_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_pg_access_cycle :12; // 3 ~ 0
        uint16_t                     : 4;
    }u;
}_xd12_trim2_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_wsel            : 4; // 3 ~ 0
        uint16_t                     :12;
    }u;
}_xd12_trim3_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_pg_start       : 1; // 0
        uint16_t otp_rd_start       : 1; // 1
        uint16_t                    : 6; // 7 ~ 2
        uint16_t ofs_temp 		    : 4; // 11 ~ 8
        uint16_t                    : 4;
    }u;
}_xd12_otp_rd_prog_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t protect_en          :12;
        uint16_t                     : 4;
    }u;
}_xd12_protection_enable_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t otp_crc_checksum    :12;
        uint16_t                     : 4;
    }u;
}_xd12_otp_crc_checksum_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t osc_ctl_7_0         : 8;
        uint16_t osc_ctl_9_8         : 2;
        uint16_t                     : 6;
    }u;
}_xd12_osc_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t vref_ctrl           : 6;
        uint16_t ictl_glb            : 3;
        uint16_t                     : 7;
    }u;
}_xd12_vref_control_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t fb_level           : 2;
        uint16_t                    : 2;
        uint16_t short_level        : 3;
        uint16_t                    : 1;
        uint16_t dev_max_curr_level : 3;
        uint16_t                    : 5;
    }u;
}_xd12_fault_level_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t delay_size         : 7;
        uint16_t                    : 9;
    }u;
}_xd12_delay_size_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t delay_period       :12;
        uint16_t                    : 4;
    }u;
}_xd12_delay_period_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ofs1_ch             : 8;
        uint16_t                     : 8;
    }u;
}_xd12_ofs1_x_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ofs2_ch             : 8;
        uint16_t                     : 8;
    }u;
}_xd12_ofs2_x_t;

#if 0
typedef struct
{
    _xd12_dummy_t _r00;
    _xd12_dummy_t _r01;
    _xd12_dummy_t _r02;
    _xd12_dummy_t _r03;
    _xd12_dummy_t _r04;
    _xd12_dummy_t _r05;
    _xd12_dummy_t _r06;
    _xd12_dummy_t _r07;
    _xd12_dummy_t _r08;
    _xd12_dummy_t _r09;
    _xd12_dummy_t _r1A;
    _xd12_dummy_t _r1B;
    _xd12_dummy_t _r1C;
    _xd12_dummy_t _r1D;
    _xd12_dummy_t _r1E;
    _xd12_dummy_t _r1F;

    _xd12_trim1_t       _r20;
    _xd12_trim2_t       _r21;
    _xd12_trim3_t       _r22;
    _xd12_otp_rd_prog_t _r23;
    _xd12_protection_enable_t _r24;
    _xd12_otp_crc_checksum_t _r25;
    _xd12_osc_t _r26;
    _xd12_vref_control_t _r27;
    _xd12_ofs_x_t _r28;
    _xd12_ofs_x_t _r29;
    _xd12_ofs_x_t _r2A;
    _xd12_ofs_x_t _r2B;
    _xd12_ofs_x_t _r2C;
    _xd12_ofs_x_t _r2D;
    _xd12_ofs_x_t _r2E;
    _xd12_ofs_x_t _r2F;
    _xd12_ofs_x_t _r30;
    _xd12_ofs_x_t _r31;
    _xd12_ofs_x_t _r32;
    _xd12_ofs_x_t _r33;
    _xd12_gain_x_t _r34;
    _xd12_gain_x_t _r35;
    _xd12_gain_x_t _r36;
    _xd12_gain_x_t _r37;
    _xd12_gain_x_t _r38;
    _xd12_gain_x_t _r39;
    _xd12_gain_x_t _r3A;
    _xd12_gain_x_t _r3B;
    _xd12_gain_x_t _r3C;
    _xd12_gain_x_t _r3D;
    _xd12_gain_x_t _r3E;
    _xd12_gain_x_t _r3F;
}_xd12_type1_t;

typedef struct
{
    _xd12_dummy_t _r00;
    _xd12_dummy_t _r01;
    _xd12_dummy_t _r02;
    _xd12_dummy_t _r03;
    _xd12_dummy_t _r04;
    _xd12_dummy_t _r05;
    _xd12_dummy_t _r06;
    _xd12_dummy_t _r07;
    _xd12_dummy_t _r08;
    _xd12_dummy_t _r09;
    _xd12_dummy_t _r1A;
    _xd12_dummy_t _r1B;
    _xd12_dummy_t _r1C;
    _xd12_dummy_t _r1D;
    _xd12_dummy_t _r1E;
    _xd12_dummy_t _r1F;

    _xd12_trim1_t       _r20;
    _xd12_trim2_t       _r21;
    _xd12_trim3_t       _r22;
    _xd12_otp_rd_prog_t _r23;
    _xd12_protection_enable_t _r24;
    _xd12_otp_crc_checksum_t _r25;
    _xd12_osc_t _r26;
    _xd12_vref_control_t _r27;
    _xd12_ofs1_x_t _r28;
    _xd12_ofs1_x_t _r29;
    _xd12_ofs1_x_t _r2A;
    _xd12_ofs1_x_t _r2B;
    _xd12_ofs1_x_t _r2C;
    _xd12_ofs1_x_t _r2D;
    _xd12_ofs1_x_t _r2E;
    _xd12_ofs1_x_t _r2F;
    _xd12_ofs1_x_t _r30;
    _xd12_ofs1_x_t _r31;
    _xd12_ofs1_x_t _r32;
    _xd12_ofs1_x_t _r33;
    _xd12_ofs2_x_t _r34;
    _xd12_ofs2_x_t _r35;
    _xd12_ofs2_x_t _r36;
    _xd12_ofs2_x_t _r37;
    _xd12_ofs2_x_t _r38;
    _xd12_ofs2_x_t _r39;
    _xd12_ofs2_x_t _r3A;
    _xd12_ofs2_x_t _r3B;
    _xd12_ofs2_x_t _r3C;
    _xd12_ofs2_x_t _r3D;
    _xd12_ofs2_x_t _r3E;
    _xd12_ofs2_x_t _r3F;
}_xd12_type2_t;

typedef union
{
    uint16_t _XD12_REG[XD12_ADDR_MAX];
    union {
        _xd12_type1_t t1;
        _xd12_type1_t t2;
    };
}_xd12_regs_t;

static _xd12_regs_t _xd12_regs;
#endif

static uint16_t _XD12_REG[XD12_ADDR_MAX];

#ifdef __XD12_C__ 

static const char* _XD12_ADDR_STR_[XD12_ADDR_MAX]={
    "RESET",  //0x00
    "ID",
    "N/A",
    "DELAY_SIZE",
    "DELAY_PRERIOD",
    "N/A",
    "LD_FIX1",
    "LD_FIX2",
    "MAX_CURR_VREF",
    "CHANNEL_ENABLE",
    "FAULT_STATUS",
    "FAULT_LEVEL",
    "DELAY_CH01",
    "DELAY_CH02",
    "DELAY_CH03",
    "DELAY_CH04",
    "DELAY_CH05",
    "DELAY_CH06",
    "DELAY_CH07",
    "DELAY_CH08",
    "DELAY_CH09",
    "DELAY_CH10",
    "DELAY_CH11",
    "DELAY_CH12",
    "SERIAL_CLOCK_GEN",
    "SHORT_ENABLE_LEVEL",
    "LD_CONTROL", //0x1A
    "N/A",
    "N/A",
    "N/A",
    "N/A",
    "N/A",
    "TRIM1", //0x20
    "TRIM2",
    "OTP_WRITE",
    "OTP_RD_PROG",
    "OTP_PROTECT",
    "OTP_CRC",
    "OSC",
    "VREF_CTL",
    "OFS1_CH01",
    "OFS1_CH02",
    "OFS1_CH03",
    "OFS1_CH04",
    "OFS1_CH05",
    "OFS1_CH06",
    "OFS1_CH07",
    "OFS1_CH08",
    "OFS1_CH09",
    "OFS1_CH10",
    "OFS1_CH11",
    "OFS1_CH12",
    "OFS2_CH01",
    "OFS2_CH02",
    "OFS2_CH03",
    "OFS2_CH04",
    "OFS2_CH05",
    "OFS2_CH06",
    "OFS2_CH07",
    "OFS2_CH08",
    "OFS2_CH09",
    "OFS2_CH10",
    "OFS2_CH11",
    "OFS2_CH12",
};

static const uint16_t _REG_INIT_XD12[XD12_ADDR_MAX] =
{
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x058, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x200, 0x01f,
    0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080,
    0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080,
    0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080, 0x080,
};

#endif //__XD12_C__ 


static void XD12_Write_REGISTER(uint8_t in_addr, uint16_t in_data);
static uint16_t XD12_Read_REGISTER(uint8_t in_addr);

static bool IS_OTP_REGISTER(uint8_t in_addr)
{
    return (bool)(in_addr >= XD12_ADDR_OSC); //OTP Check
}

static void XD12_Print_XD12_REGS(uint8_t in_GUI_SUPPORT)
{
    char temp[2048] = {0,};
    int sz = 0;
    uint8_t i=0;

    const uint8_t *ptr = XD12_REGISTER_MAP;

    print("\r\n");
    if(in_GUI_SUPPORT == _GUI_SUPPORT_)
    {
        print(CMD_XD12_READ_REGISTER_ALL);
        for(i=0; i<XD12_ADDR_MAX; ++i)
        {
            if(ptr[i] == _Y__)
            {
                sz += snprintf(temp + sz, sizeof temp, ":%02x:%03x", i, _XD12_REG[i]);
            }
            else
            {
                sz += snprintf(temp + sz, sizeof temp, ":%02x:NA",i);
            }
        }
        print("\r\n");
    }
    else
    {
        const char ** ptr_XD12_ADDR_STR = _XD12_ADDR_STR_;
        for(i=0; i<XD12_ADDR_MAX; ++i)
        {
            if(ptr[i])
            {
                sz += snprintf(temp + sz, sizeof temp, "%18s(0x%02X) : 0x%03X (%4u)\r\n", ptr_XD12_ADDR_STR[i], i, _XD12_REG[i], _XD12_REG[i]);
            }
            else
            {
                sz += snprintf(temp + sz, sizeof temp, "%18s(0x%02X) : N/A\r\n", ptr_XD12_ADDR_STR[i], i);
            }            
        }
    }
    print("%s\r\n", temp);
}

static void XD12_OTP_PROTECT_Disable()
{
    if(_XD12_REG[XD12_ADDR_OTP_PROTECT] != OTP_PROTECT_DISABLE)
    {
        #if _DBG_XD12
        print("OTP_PROTECT_DISABLE\r\n");
        #endif //_DBG_XD12

        // OTP PROTECT - Disable
        XD12_Write_REGISTER(XD12_ADDR_OTP_PROTECT, OTP_PROTECT_DISABLE);
    }
}

static void XD12_OTP_PROTECT_Enable()
{
    if(_XD12_REG[XD12_ADDR_OTP_PROTECT] != OTP_PROTECT_ENABLE)
    {
        #if _DBG_XD12
        print("OTP_PROTECT_ENABLE\r\n");
        #endif //_DBG_XD12

        // OTP PROTECT - Enable
        XD12_Write_REGISTER(XD12_ADDR_OTP_PROTECT, OTP_PROTECT_ENABLE);
    }
}
#if 0
static void XD12_Write_otp_rd_start(bool in_val)
{
    #if _DBG_XD12
    print("XD12_Write_otp_rd_start(%d)\r\n",in_val);
    #endif //_DBG_XD12

    _xd12_otp_rd_prog_t* p = (_xd12_otp_rd_prog_t*)(_XD12_REG + XD12_ADDR_OTP_RD_PROG);
    p->u.otp_rd_start = in_val;

    // Write Register
    XD12_Write_REGISTER(XD12_ADDR_OTP_RD_PROG, p->value);
}
#endif
static uint8_t XD12_Write_otp_pg_start(uint8_t in_val)
{
    #if _DBG_XD12
        print("XD12_Write_otp_pg_start(%d)\r\n",in_val); 
    #endif //_DBG_XD12

    if(in_val > 1) // 1bit otp_pg_start b00
        return FALSE;

    _xd12_otp_rd_prog_t* p = (_xd12_otp_rd_prog_t*)(_XD12_REG + XD12_ADDR_OTP_RD_PROG);
    p->u.otp_pg_start = in_val;

    // Write Register
    XD12_Write_REGISTER(XD12_ADDR_OTP_RD_PROG, p->value);

    return TRUE;
}

static void XD12_test_en_Enable(uint8_t in_Enable)
{
    _xd12_trim1_t* p = (_xd12_trim1_t*)(_XD12_REG + XD12_ADDR_TRIM1);
    if(in_Enable != p->u.test_en)
    {
        if(in_Enable == TRUE)
        {
            p->u.test_en = 1;
        }
        else
        {
            p->u.test_en = 0;
        }

        XD12_Write_REGISTER(XD12_ADDR_TRIM1, p->value);
        //HAL_Delay(XD12_DELAY_SHORT);
    }
    else
    {
        #ifndef _DBG_XD12
            // print("Already-test_en:%d\r\n",p->u.test_en);
        #endif //_DBG_XD12
    }
}

static void XD12_Read_All(void)
{
    uint16_t u16_data = 0;
    uint8_t u8_addr = 0;
    const uint8_t *ptr = XD12_REGISTER_MAP;
#if _DBG_XD12
    const char **ptr_XD12_ADDR_STR = _XD12_ADDR_STR_;
#endif

    // OTP RD/PROG - Set OTP RD
    //XD12_Write_otp_rd_start(true);

    /* Read NonOTP 0x00 ~ 0x26 *************/
    print("\r\n");
    for(u8_addr=XD12_ADDR_RESET; u8_addr<XD12_ADDR_MAX/*XD12_ADDR_OSC*/; ++u8_addr)
    {
        if(ptr[u8_addr] == _Y__)
        {
            u16_data = XD12_Read_REGISTER(u8_addr);
            #if _DBG_XD12
            print("[%s - 0x%02X : 0x%03X ]\r\n", ptr_XD12_ADDR_STR[u8_addr] ,u8_addr ,u16_data);
            #endif
        }
        else
        {
            #if _DBG_XD12
            print("[%s - 0x%02X : NA ]\r\n", ptr_XD12_ADDR_STR[u8_addr] ,u8_addr);
            #endif
        }
    }
#if 0
    /* Read OTP 0x26 ~ 0x3F *************/
    // OTP RD/PROG - Reset OTP RD
    XD12_Write_otp_rd_start(false);

#if _DBG_XD12
    print("\r\n");
    for(u8_addr=XD12_ADDR_OSC; u8_addr<XD12_ADDR_MAX; ++u8_addr)
    {
        u16_data = XD12_Read_REGISTER(u8_addr);
        print("[%s - 0x%02X : 0x%03X ]\r\n", ptr_XD12_ADDR_STR[u8_addr] ,u8_addr ,u16_data);
    }
#endif
#endif
}

static uint16_t XD12_Read_REGISTER(uint8_t in_addr)
{
    uint16_t u16_reg_val = JigBd_IF_Read_Command(in_addr);    
    
    _XD12_REG[in_addr]  = u16_reg_val;
    print("[0x%02X : 0x%04X ]\r\n", in_addr ,u16_reg_val);

    return u16_reg_val;
}

uint16_t XD12_Get_REGISTER(uint8_t in_addr)
{
     return _XD12_REG[in_addr];
}

static void XD12_Write_REGISTER(uint8_t in_addr, uint16_t in_data)
{
    if(IS_OTP_REGISTER(in_addr))
    {
        // OTP PROTECT - Disable
        XD12_OTP_PROTECT_Disable();
    }

    _XD12_REG[in_addr] = in_data;
    JigBd_IF_Write_Command(in_addr, in_data); 
    //print("_XD12_REG[%s]: 0x%04X\r\n", ptr_XD12_ADDR_STR[in_addr], _XD12_REG[in_addr]);
}

static uint8_t XD12_Write_OSC(uint16_t in_val)
{
    if(in_val > REGISTER_LIMIT_OSC) //2^8 OSC[9:8] is fixed to 2
    {
        print("XD12_Write_OSC() input[%d] is Over %d\r\n", in_val, REGISTER_LIMIT_OSC);         
        return FALSE;
    }
    else
    {
        _xd12_osc_t* p = (_xd12_osc_t*)(_XD12_REG + XD12_ADDR_OSC);
        p->u.osc_ctl_7_0 = in_val;

        XD12_Write_REGISTER(XD12_ADDR_OSC, p->value);

        #if _DBG_XD12
        print("_XD12_REG[XD12_ADDR_OSC]: 0x%04X\r\n",_XD12_REG[XD12_ADDR_OSC]);
        #endif //_DBG_XD12
        return TRUE;
    }
}

static uint8_t XD12_Write_GLB(uint16_t in_val)
{
    uint8_t u8_rtn = 0;
    
    // GLB VALUE Control - ICTL_GLB[8:6]
    if(in_val > REGISTER_LIMIT_GLB) // 2^3
    {
        print("XD12_Write_GLB() input[%d] is Over %d\r\n",in_val, REGISTER_LIMIT_GLB); 
        u8_rtn =  FALSE;
    }

    _xd12_vref_control_t* p = (_xd12_vref_control_t*)(_XD12_REG + XD12_ADDR_VREF_CTL);
    p->u.ictl_glb = in_val;

    // Write Register
    XD12_Write_REGISTER(XD12_ADDR_VREF_CTL, p->value);

    #if _DBG_XD12
        print("_XD12_REG[XD12_ADDR_VREF_CTL]: 0x%04X\r\n",_XD12_REG[XD12_ADDR_VREF_CTL]);
    #endif //_DBG_XD12
    u8_rtn = TRUE;

    return u8_rtn;
}

static uint8_t XD12_Write_VREF(uint16_t in_val)
{
    if(in_val > REGISTER_LIMIT_VREF) // 2^6 VREF_CTL [5:0]
    {
        print("XD12_Write_VREF() input[%d] is Over %d\r\n",in_val, REGISTER_LIMIT_VREF); 
        return FALSE;
    }

    _xd12_vref_control_t* p = (_xd12_vref_control_t*)(_XD12_REG + XD12_ADDR_VREF_CTL);
    p->u.vref_ctrl = in_val;

    // Write Register
    XD12_Write_REGISTER(XD12_ADDR_VREF_CTL, p->value);

    #if _DBG_XD12
        print("_XD12_REG[XD12_ADDR_VREF_CTL]: 0x%04X\r\n",_XD12_REG[XD12_ADDR_VREF_CTL]);
    #endif //_DBG_XD12
    return TRUE;
}

static uint8_t XD12_Write_OFS_TEMP(uint16_t in_val)
{
    if(in_val > REGISTER_LIMIT_OFS_TEMP) // 2^4 OFS_TEMP [3:0] b11 ~ b08
    {
        print("XD12_Write_OFS_TEMP() input[%d] is Over %d\r\n",in_val, REGISTER_LIMIT_OFS_TEMP);
        return FALSE;
    }

    _xd12_otp_rd_prog_t* p = (_xd12_otp_rd_prog_t*)(_XD12_REG + XD12_ADDR_OTP_RD_PROG);
    p->u.ofs_temp = in_val;

    // Write Register
    XD12_Write_REGISTER(XD12_ADDR_OTP_RD_PROG, p->value);

    #if _DBG_XD12
        print("_XD12_REG[XD12_ADDR_OTP_RD_PROG]: 0x%04X\r\n",_XD12_REG[XD12_ADDR_OTP_RD_PROG]);
    #endif //_DBG_XD12
    return TRUE;
}

static uint8_t XD12_Write_OFS1_CHx(uint8_t in_ch, uint16_t in_val)
{
    if(in_ch > CH_MAX)
    {
        return FALSE;
    }

    if(in_val > REGISTER_LIMIT_OFS_1) // 2^8 OFS_CHX [7:0]
    {
        print("XD12_Write_OFS_CH%d() input[%d] is Over %d\r\n",in_ch+1,in_val, REGISTER_LIMIT_OFS_1);
        return FALSE;
    }

    _xd12_ofs1_x_t* p = (_xd12_ofs1_x_t*)(_XD12_REG + (XD12_ADDR_OFS1_CH01+in_ch));
    p->u.ofs1_ch = in_val;

    // Write Register
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH01+in_ch, in_val);

#if _DBG_XD12
    print("XD12_Write_OFS1_CHx(%d,%d)\r\n",in_ch+1,in_val); 
#endif //_DBG_XD12
    return TRUE;
}

static uint8_t XD12_Write_OFS2_CHx(uint8_t in_ch, uint16_t in_val)
{
    if(in_ch > CH_MAX || in_val > REGISTER_LIMIT_OFS_2)
    {
        print("XD12_Write_OFS2_CHx%d() input[%d] is Over %d\r\n",in_ch+1,in_val, REGISTER_LIMIT_OFS_2);
        return FALSE;
    }

    _xd12_ofs2_x_t* p = (_xd12_ofs2_x_t*)(_XD12_REG + XD12_ADDR_OFS2_CH01+in_ch);
    p->u.ofs2_ch = in_val;

    // Write Register
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH01+in_ch, in_val);

#if _DBG_XD12
    print("XD12_Write_OFS2_CHx(%d,%d)\r\n",in_ch,in_val); 
#endif //_DBG_XD12
    return TRUE;
}

/* BEGIN - INTERFACE FUNCTIONS ******************************************************************/
void TargetIC_IF_PowerOn_Init()
{
    for(trim_mode_t mode = TRIM_OSC_FREQUENCY ; mode < TRIM_MAX ; ++mode)
    {
        p_gui_param p = trim_get_param_gui();
        
        switch(mode)
        {
        case TRIM_OSC_FREQUENCY:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_OSC_TARGET * (1 - XD12_ERR_RATE));   /* 14.750MHz, Target 14.765MHz -0.1% */
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_OSC_TARGET * (1 + XD12_ERR_RATE));   /* 14.780MHz, Target 14.765MHz +0.1% */
            p[mode][TRIM_PARA_P1] = 0;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_VREF_CTL:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_VREF_TARGET * (1 - 0.01));    /* 2.167V, Target 2.2V -1.5% */
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_VREF_TARGET * (1 + 0.01));    /* 2.233V, Target 2.2V +1.5% */
            p[mode][TRIM_PARA_P1] = 0;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_ICTL_GLB:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_GLB_TARGET * (1 + XD12_ERR_RATE));    /* 31.36mA, Target 32mA -2% @ LD_FIX 255 */
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_GLB_TARGET * (1 + XD12_ERR_RATE));    /* 32.64mA, Target 32mA -2% @ LD_FIX 255 */
            p[mode][TRIM_PARA_P1] = 255;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_OFS1_CHS:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_OSF1_TARGET * (1 - XD12_OFS1_ERR_RATE)) / 1000;   /* 0.1391mA, Target 0.1412mA -1.5% @ (P1 + P2) / 2 */
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_OSF1_TARGET * (1 + XD12_OFS1_ERR_RATE)) / 1000;   /* 0.1433mA, Target 0.1412mA +1.5% @ (P1 + P2) / 2 */
            p[mode][TRIM_PARA_P1] = 3;
            p[mode][TRIM_PARA_P2] = 6;
            break;
        case TRIM_OFS2_CHS:
            p[mode][TRIM_PARA_TARGET_MIN] = (XD12_OSF2_TARGET * (1 - XD12_OFS2_ERR_RATE)) / 1000;   /* 1.7305mA, Target 1.7569mA -1.5% @ P1 */
            p[mode][TRIM_PARA_TARGET_MAX] = (XD12_OSF2_TARGET * (1 + XD12_OFS2_ERR_RATE)) / 1000;   /* 1.7833mA, Target 1.7569mA +1.5% @ P1 */
            p[mode][TRIM_PARA_P1] = 56;
            p[mode][TRIM_PARA_P2] = 0;
            break;
        case TRIM_SCREENING:
            p[mode][TRIM_PARA_SCR_START]  = 0;
            p[mode][TRIM_PARA_SCR_END]    = 255;
            p[mode][TRIM_PARA_P1]         = 1;
            p[mode][TRIM_PARA_P2]         = 4;
            break;
        }
    }
}

uint8_t TargetIC_IF_LD_FIX_Set(uint16_t in_val)
{
    if(in_val > 255)
    {
        return FALSE;
    }    

    // LD_FIX1, adr = 0x06
    XD12_Write_REGISTER(XD12_ADDR_LD_FIX1, ((in_val & 0x0F) << 8));  /*  PAM_LSB(4bit) + DUTY(8bit) */
    // LD_FIX2, adr = 0x07
    XD12_Write_REGISTER(XD12_ADDR_LD_FIX2, ((in_val & 0xF0) >> 4));  /*  PAM_MSB(4bit) */
    return TRUE;
}

uint8_t TargetIC_IF_FAULT_LEVEL_Set(dev_max_curr_level_t in_dev_max_curr, short_level_t in_short_level, fb_level_t in_fb_level)
{
    if(in_dev_max_curr > REGISTER_LIMIT_DEV_MAX_CURR_LEVEL) // 3-bit DEV_MAX_CURR_LEVEL [10:8]
    {
        print("TargetIC_IF_FAULT_LEVEL_Set() dev_max_curr input[%d] is Over %d\r\n", in_dev_max_curr, REGISTER_LIMIT_DEV_MAX_CURR_LEVEL); 
        return FALSE;
    }

    if(in_short_level > REGISTER_LIMIT_SHORT_LEVEL) // 3-bit SHORT_LEVEL [6:4]
    {
        print("TargetIC_IF_FAULT_LEVEL_Set() short_level input[%d] is Over %d\r\n", in_short_level, REGISTER_LIMIT_SHORT_LEVEL); 
        return FALSE;
    }

    if(in_fb_level > REGISTER_LIMIT_FB_LEVEL) // 2-bit FB_LEVEL [1:0]
    {
        print("TargetIC_IF_FAULT_LEVEL_Set() fb_level is Over %d\r\n", in_fb_level, REGISTER_LIMIT_FB_LEVEL); 
        return FALSE;
    }
    
    _xd12_fault_level_t* p = (_xd12_fault_level_t*)(_XD12_REG + XD12_ADDR_FAULT_LEVEL);
    /*
        4mA @ dev_max_curr_level = 3b'000
        8mA @ dev_max_curr_level = 3b'001
       16mA @ dev_max_curr_level = 3b'010
       24mA @ dev_max_curr_level = 3b'011
       32mA @ dev_max_curr_level = 3b'100
       64mA @ dev_max_curr_level = 3b'101
    */
    p->u.dev_max_curr_level = in_dev_max_curr;
    p->u.short_level = in_short_level;
    p->u.fb_level = in_fb_level;

    // Write Register
    XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, p->value);
    return TRUE;
}

uint8_t TargetIC_IF_CHANNEL_ENABLE_Set(uint8_t in_channel)
{
    uint8_t ret = FALSE;

    if(in_channel < CH_MAX)
    {
        // CHANNEL ENABLE
        XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, (1 << in_channel));
        ret = TRUE;
    }
    else
    {
        print("ERROR:input is over CH_MAX: TargetIC_IF_CHANNEL_ENABLE_Set(%d)\r\n", in_channel);
        ret = FALSE;
    }

    return ret;
}

void TargetIC_IF_CHANNEL_ENABLE_ResetAll()
{
    XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, 0x00);
}

void TargetIC_IF_CHANNEL_ENABLE_SetAll()
{
    XD12_Write_REGISTER(XD12_ADDR_CHANNEL_ENABLE, 0x0FFF);
}

void TargetIC_IF_LD_CONTROL_INIT()
{
    XD12_Write_REGISTER(XD12_ADDR_LD_CONTROL, 0x33);
}

void TargetIC_IF_TRIM_REG_INIT()
{
    XD12_Write_REGISTER(XD12_ADDR_OTP_PROTECT, 0xA5A); //turn-off otp protect function
    XD12_Write_REGISTER(XD12_ADDR_OSC, 584);
    XD12_Write_REGISTER(XD12_ADDR_VREF_CTL, 283);
    
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH01, 106);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH02, 107);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH03, 120);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH04, 102);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH05, 118);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH06, 118);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH07, 111);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH08, 107);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH09, 118);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH10, 104);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH11, 111);
    XD12_Write_REGISTER(XD12_ADDR_OFS1_CH12, 109);
    
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH01, 102);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH02, 110);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH03, 130);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH04, 109);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH05, 114);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH06, 120);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH07, 114);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH08, 106);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH09, 118);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH10, 106);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH11, 112);
    XD12_Write_REGISTER(XD12_ADDR_OFS2_CH12, 112);
}

uint16_t TargetIC_IF_Inc_Screen_CurValue(uint16_t in_u16_screen_input_cur)
{
    uint16_t temp_step = 0;
    uint16_t u16_50_PERCENT_POINT_OF_LD_FIX = 0;
    uint16_t u16_50_MAX_OF_LD_FIX = 0;

    u16_50_PERCENT_POINT_OF_LD_FIX = LD_FIX_50_PERCENT_POINT;
    u16_50_MAX_OF_LD_FIX = LD_FIX_MAX;

    if(in_u16_screen_input_cur < u16_50_PERCENT_POINT_OF_LD_FIX)
    {
        temp_step = (uint16_t)trim_get_param_gui()[TRIM_SCREENING][TRIM_PARA_P1];
    }
    else
    {
        temp_step = (uint16_t)trim_get_param_gui()[TRIM_SCREENING][TRIM_PARA_P2];
    }

    if(in_u16_screen_input_cur + temp_step >= u16_50_MAX_OF_LD_FIX)
    {
        in_u16_screen_input_cur = u16_50_MAX_OF_LD_FIX;
    }
    else
    {
        in_u16_screen_input_cur += temp_step;
    }

    return in_u16_screen_input_cur;
}

void TargetIC_IF_Initialize(void)
{
    // Init Gobal Val
    if(IS_XC24())
    {
        for(uint8_t i = 0 ; i < XD12_ADDR_MAX ; ++i)
        {
            _XD12_REG[i] = 0;
        }
    }
    else
    {
        for(uint8_t i = 0 ; i < XD12_ADDR_MAX ; ++i)
        {
            _XD12_REG[i] = _REG_INIT_XD12[i];
        }
    }

    // ID Gen
    JigBd_IF_IdGen_Command();
    TargetIC_IF_Fault_Level_Init();
    TargetIC_IF_Serial_Clock_Init();
    TargetIC_IF_Temp_Level_Init();
    //TargetIC_IF_MAX_CURR_VREF_Init();
    TargetIC_IF_SHORT_ENABLE_LEVEL_Init();
    TargetIC_IF_LD_CONTROL_INIT();
}

void TargetIC_IF_Init_Register(void)
{
    TargetIC_IF_TrimRegister_Set(0, TRIM_OSC_FREQUENCY, XD12_DEFAULT_OSC);
    TargetIC_IF_TrimRegister_Set(0, TRIM_VREF_CTL, XD12_DEFAULT_VREF_CTL);
    TargetIC_IF_TrimRegister_Set(0, TRIM_ICTL_GLB, XD12_DEFAULT_GLB);        
    
    for(uint8_t i_channel = 0 ; i_channel < CH_MAX ; ++i_channel)
    {
        TargetIC_IF_TrimRegister_Set(i_channel, TRIM_OFS1_CHS, XD12_DEFAULT_OFS1);
    }
    
    for(uint8_t i_channel = 0 ; i_channel < CH_MAX ; ++i_channel)
    {
        TargetIC_IF_TrimRegister_Set(i_channel, TRIM_OFS2_CHS, XD12_DEFAULT_OFS2);
    }
    
    XD12_Write_REGISTER(XD12_ADDR_MAX_CURR_VREF, 4095);
    XD12_Write_OFS_TEMP(15);
}

void TargetIC_IF_Read_Resgister_All(uint8_t in_GUI_SUPPORT)
{
    XD12_Read_All();
    XD12_Print_XD12_REGS(in_GUI_SUPPORT);
}

void TargetIC_IF_Show_RegAll()
{
    const char **ptr_XD12_ADDR_STR = _XD12_ADDR_STR_;

    for(int i = 0; i < XD12_ADDR_MAX ; ++i)
    {
        print("[ %s(0x%02X) - 0x%03X(%4d) ]\r\n",ptr_XD12_ADDR_STR[i], i, _XD12_REG[i], _XD12_REG[i]);
    }
}

void TargetIC_IF_Write_OTP()
{
    // ID GEN
    JigBd_IF_IdGen_Command();

    // OTP PROTECT - Disable
    XD12_OTP_PROTECT_Disable();

    // otp_pg_access_cycle [15:12] : 0
    _xd12_trim1_t* p = (_xd12_trim1_t*)(_XD12_REG + XD12_ADDR_TRIM1);
    p->u.otp_pg_access_cycle = 0;
    XD12_Write_REGISTER(XD12_ADDR_TRIM1, p->value);
    // otp_pg_access_cycle [11:0] : 320
    XD12_Write_REGISTER(XD12_ADDR_TRIM2, 320);
    // otp_wsel [3:0] : 4
    XD12_Write_REGISTER(XD12_ADDR_OTP_WRITE, 4);

    /* Write OTP 0x26 ~ 0x3F *************/
    for(uint8_t u8_addr = XD12_ADDR_OSC ; u8_addr < XD12_ADDR_MAX; ++u8_addr)
    {
        XD12_Write_REGISTER(u8_addr, _XD12_REG[u8_addr]);
    }

    // otp_pg_start : 1
    XD12_Write_otp_pg_start(1);
    HAL_Delay(1000);
    // otp_pg_start : 0
    XD12_Write_otp_pg_start(0);

    // OTP PROTECT - Enable
    XD12_OTP_PROTECT_Enable();
}

void TargetIC_IF_Write_OTP_Start()
{
    // ID GEN
    JigBd_IF_IdGen_Command();

    // OTP PROTECT - Disable
    XD12_OTP_PROTECT_Disable();

    // otp_pg_access_cycle [15:12] : 0
    _xd12_trim1_t* p = (_xd12_trim1_t*)(_XD12_REG + XD12_ADDR_TRIM1);
    p->u.otp_pg_access_cycle = 0;
    XD12_Write_REGISTER(XD12_ADDR_TRIM1, p->value);
    // otp_pg_access_cycle [11:0] : 320
    XD12_Write_REGISTER(XD12_ADDR_TRIM2, 320);
    // otp_wsel [3:0] : 4
    XD12_Write_REGISTER(XD12_ADDR_OTP_WRITE, 4);

    /* Write OTP 0x26 ~ 0x3F *************/
    for(uint8_t u8_addr = XD12_ADDR_OSC ; u8_addr < XD12_ADDR_MAX ; ++u8_addr)
    {
        XD12_Write_REGISTER(u8_addr, _XD12_REG[u8_addr]);
    }
    
    // otp_pg_start : 1
    XD12_Write_otp_pg_start(1);
}

void TargetIC_IF_Write_OTP_End()
{
    // otp_pg_start : 0
    XD12_Write_otp_pg_start(0);

    // OTP PROTECT - Enable
    XD12_OTP_PROTECT_Enable();
}


void TargetIC_IF_Write_REGISTER(uint8_t in_addr, uint16_t in_data)
{
    XD12_Write_REGISTER(in_addr, in_data);
}

uint16_t TargetIC_IF_TrimRegister_Get(uint8_t ch_num, trim_mode_t in_trim_mode)
{
    uint16_t rtn_val;
    switch(in_trim_mode)
    {
        case TRIM_OSC_FREQUENCY:
            {
                _xd12_osc_t* p = (_xd12_osc_t*)(_XD12_REG + XD12_ADDR_OSC);
                rtn_val = p->u.osc_ctl_7_0;
                print("GET-OSC : %d\r\n", rtn_val);
            }
            break;
        case TRIM_VREF_CTL:
            {
                _xd12_vref_control_t* p = (_xd12_vref_control_t*)(_XD12_REG + XD12_ADDR_VREF_CTL);
                rtn_val = p->u.vref_ctrl;
                print("GET-VREF : %d\r\n", rtn_val);
            }
            break;
        case TRIM_ICTL_GLB:
            {
                _xd12_vref_control_t* p = (_xd12_vref_control_t*)(_XD12_REG + XD12_ADDR_VREF_CTL);
                rtn_val = p->u.ictl_glb;
                print("GET-GLB : %d\r\n", rtn_val);
            }
            break;
        case TRIM_OFS1_CHS:
            rtn_val = _XD12_REG[XD12_ADDR_OFS1_CH01 + ch_num];
            print("GET-OFS1_CH%02d : %d\r\n", ch_num + 1, rtn_val);
            break;
        case TRIM_OFS2_CHS:
            rtn_val = _XD12_REG[XD12_ADDR_OFS2_CH01 + ch_num];
            print("GET-OFS2_CH%02d : %d\r\n", ch_num+ 1 , rtn_val);
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
            if(in_regi_val > REGISTER_LIMIT_OSC)
            {
                print("ERROR: TargetIC_IF_TrimRegister_Set TRIM_OSC_FREQUENCY- in_regi_val(%d) Over !!\r\n", in_regi_val);
            }
            else
            {
                rtn_val = XD12_Write_OSC(in_regi_val);
            }
            break;
        case TRIM_VREF_CTL:
            if(in_regi_val > REGISTER_LIMIT_VREF)
            {
                print("ERROR: TargetIC_IF_TrimRegister_Set TRIM_VREF_CTL- in_regi_val(%d) Over !!\r\n", in_regi_val);
            }
            else
            {
                rtn_val = XD12_Write_VREF(in_regi_val);
            }
            break;
        case TRIM_ICTL_GLB:
            if(in_regi_val > REGISTER_LIMIT_GLB)
            {
                print("ERROR: TargetIC_IF_TrimRegister_Set REGISTER_LIMIT_GLB- in_regi_val(%d) Over !!\r\n", in_regi_val);
            }
            {
                rtn_val = XD12_Write_GLB(in_regi_val);
            }
            break;
        case TRIM_OFS1_CHS:
            if(in_regi_val > REGISTER_LIMIT_OFS_1)
            {
                print("ERROR: TargetIC_IF_TrimRegister_Set TRIM_OFS1_CHS- in_regi_val(%d) Over !!\r\n", in_regi_val);
            }
            else
            {
                rtn_val = XD12_Write_OFS1_CHx(ch_num, in_regi_val);
            }
            break;
        case TRIM_OFS2_CHS:
            if(in_regi_val > REGISTER_LIMIT_OFS_2)
            {
                print("ERROR: TargetIC_IF_TrimRegister_Set TRIM_OFS2_CHS- in_regi_val(%d) Over !!\r\n", in_regi_val);
            }
            else
            {
                rtn_val = XD12_Write_OFS2_CHx(ch_num, in_regi_val);
            }
            break;
    }

    if(rtn_val == FALSE)
    {
        print("ERROR: TargetIC_IF_TrimRegister_Set(%d, %d, %d) - FALSE RETRUN\r\n", ch_num, in_trim_mode, in_regi_val);
    }
    return rtn_val;
}

uint16_t TargetIC_IF_TrimRegister_Limit_Get(uint8_t ch_num, trim_mode_t in_trim_mode)
{
    uint16_t rtn_val = 0;
    switch(in_trim_mode)
    {
        case TRIM_OSC_FREQUENCY:
            rtn_val = REGISTER_LIMIT_OSC;
            #if _DBG_XD12
                print("OSC_LIMIT : %d\r\n", rtn_val);
            #endif //_DBG_XD12
            break;
        case TRIM_VREF_CTL:
            rtn_val = REGISTER_LIMIT_VREF;
            #if _DBG_XD12
                print("VREF_LIMIT : %d\r\n", rtn_val);
            #endif //_DBG_XD12
            break;
        case TRIM_ICTL_GLB:
            rtn_val = REGISTER_LIMIT_GLB;
            #if _DBG_XD12
                print("GLB_LIMIT : %d\r\n", rtn_val);
            #endif //_DBG_XD12
            break;
        case TRIM_OFS1_CHS:
            rtn_val = REGISTER_LIMIT_OFS_1;
            #if _DBG_XD12
                print("OFS1_CH%02d_LIMIT : %d\r\n", ch_num + 1, rtn_val);
            #endif //_DBG_XD12
            break;
        case TRIM_OFS2_CHS:
            rtn_val = REGISTER_LIMIT_OFS_2;
            #if _DBG_XD12
                print("OFS2_CH%02d_LIMIT : %d\r\n", ch_num + 1, rtn_val);
            #endif //_DBG_XD12
            break;
    }

    return rtn_val;
}

void TargetIC_IF_TRIM_Init_OSC()
{
    XD12_test_en_Enable(TRUE);
}

void TargetIC_IF_TRIM_Init_VREF()
{
    XD12_test_en_Enable(TRUE);
    XD12_Write_REGISTER(XD12_ADDR_MAX_CURR_VREF, 4095);
}

void TargetIC_IF_TRIM_Init_GLB()
{
    XD12_test_en_Enable(TRUE);
}

void TargetIC_IF_TRIM_Init_OFS_1()
{
    XD12_test_en_Enable(TRUE);
}

void TargetIC_IF_TRIM_Init_OFS_2()
{
    XD12_test_en_Enable(TRUE);
}

void TargetIC_IF_SEND_otp_written()
{
    const uint8_t *ptr = XD12_REGISTER_MAP;

    char temp[512] = {0, };
    int sz = 0;
    for(int i = XD12_ADDR_OSC ; i < XD12_ADDR_MAX ; ++i)
    {
        if(ptr[i] == _Y__)
        {
            sz += snprintf(temp + sz, sizeof temp, ":%02x:%03x", i, _XD12_REG[i]);
        }
        else
        {
            sz += snprintf(temp + sz, sizeof temp, ":%02x:N/A", i);
        }
    }
    print("\r\n%s%s\r\n", CMD_XD12_OTP_WRITTEN, temp);
}

/* END - INTERFACE FUNCTIONS ************************************************************************/


/* BEGIN - DBG_TEST SUPPORT *************************************************************/
#ifdef DBG_TEST
void _dbg_XD12_Print_RegisterMap(void)
{
    const uint8_t *ptr = XD12_REGISTER_MAP;
    const char **ptr_XD12_ADDR_STR = _XD12_ADDR_STR_;

#if _DBG_XD12
    for(uint8_t i = XD12_ADDR_RESET ; i < XD12_ADDR_MAX ; ++i)
    {
        if(ptr[i] == _Y__)
        {
            print("0x%02X-XD12_ADDR_%s\r\n", i, ptr_XD12_ADDR_STR[i]); 
        }
        else
        {
            print("0x%02X-N/A\r\n", i); 
        }
    }
#endif
}

uint16_t TargetIC_IF_Read_Command_through_PWM_to_XD12(uint8_t in_addr)
{
    return XD12_Read_REGISTER(in_addr);
}

void _dbg_XD12_Detect(void)
{
    uint8_t i = 0;

    // Read Freq
    JigBd_IF_TIM_Capture_Start();
    us_tdelay(50 * 1000);
    us_tdelay(50 * 1000);
    JigBd_IF_TIM_Capture_Stop();
    print("_dbg_XD12_Detect - test_en DISABLE - Read Freq : %d\r\n", JigBd_IF_Freq_Get());

    // Power ON
    JigBd_IF_VCC_EN_power(PWR_ON);
    JigBd_IF_5_0V_power(PWR_ON_5_0V);

    // Init Gobal Val
    if(IS_XC24())
    {
        for(i = 0 ; i < XD12_ADDR_MAX ; ++i)
        {
            _XD12_REG[i] = 0;
        }
    }
    else
    {
        for(i = 0 ; i < XD12_ADDR_MAX ; ++i)
        {
            _XD12_REG[i] = _REG_INIT_XD12[i];
        }
    }

    // ID Gen
    JigBd_IF_IdGen_Command();

    // Enable test_en
    XD12_test_en_Enable(TRUE);

    // Read Freq
    JigBd_IF_TIM_Capture_Start();
    us_tdelay(50 * 1000);
    us_tdelay(50 * 1000);
    JigBd_IF_TIM_Capture_Stop();
    print("_dbg_XD12_Detect - test_en ENABLE - Read Freq : %d\r\n", JigBd_IF_Freq_Get());
}

void _dbg_XD12_Test(void)
{
    _dbg_XD12_Print_RegisterMap();
    TargetIC_IF_SEND_otp_written();
}

void TargetIC_IF_Fault_Level_Init()
{
    /* DEV_MAX_CURR_LEVEL : 3b'001,  8mA */
    /* SHORT_LEVEL        : 3b'111,  38V */
    /* FB_LEVEL           : 2b' 00, 0.6V */
    /* 0101_0110_0000 = 0x560 */
    _xd12_fault_level_t _xd12_fault_level_;
    _xd12_fault_level_.value = 0;
    
    _xd12_fault_level_.u.dev_max_curr_level = DEV_MAX_CURR_LEVEL_8mA;
    _xd12_fault_level_.u.short_level = SHORT_LEVEL_38V;
    _xd12_fault_level_.u.fb_level = FB_LEVEL_0V6;
        
    XD12_Write_REGISTER(XD12_ADDR_FAULT_LEVEL, _xd12_fault_level_.value);
}

void TargetIC_IF_Serial_Clock_Init()
{
    XD12_Write_REGISTER(XD12_ADDR_SERIAL_CLOCK_GEN, ((SERIAL_CLK_CNT_LOW << 4) | (SERIAL_CLK_CNT_HIGH << 0)));
}


void TargetIC_IF_Temp_Level_Init()
{
    XD12_Write_REGISTER(XD12_ADDR_OTP_RD_PROG, 0xF00);
}

void TargetIC_IF_MAX_CURR_VREF_Init()
{
    XD12_Write_REGISTER(XD12_ADDR_MAX_CURR_VREF, 0xFFF);
}

void TargetIC_IF_SHORT_ENABLE_LEVEL_Init()
{
    XD12_Write_REGISTER(XD12_ADDR_SHORT_ENABLE_LEVEL, 0xFFF);
}

void TargetIC_IF_Test_Enable()
{
    XD12_Write_REGISTER(XD12_ADDR_TRIM1, 0x800);
}

void TargetIC_IF_SHORT_ENABLE_Set(uint16_t in_short_en_level)
{
    if (in_short_en_level > 0xFFF)
    {
        return;
    }
    XD12_Write_REGISTER(XD12_ADDR_SHORT_ENABLE_LEVEL, in_short_en_level);
}

uint8_t XD12_Write_DELAY_SIZE(uint16_t in_delay_size)
{
    _xd12_delay_size_t _xd12_delay_size;
    _xd12_delay_size.value = 0;
    if (in_delay_size > 0x7F)
    {
        print("\r\n%4u is invalid value DELAY_SIZE - 0 ~ 127 (0x7F) [6:0]\r\n", in_delay_size);
        return FALSE;
    }
    _xd12_delay_size.u.delay_size = in_delay_size;
    XD12_Write_REGISTER(XD12_ADDR_DELAY_SIZE, _xd12_delay_size.value);
    
    return TRUE;
}

uint8_t XD12_Write_DELAY_PERIOD(uint16_t in_delay_period)
{
    _xd12_delay_period_t _xd12_delay_period;
    _xd12_delay_period.value = 0;
    if (in_delay_period > 0xFFF)
    {
        print("\r\n%4u is invalid value DELAY_PERIOD - 0 ~ 4095 (0xFFF) [11:0]\r\n", in_delay_period);
        return FALSE;
    }
    _xd12_delay_period.u.delay_period = in_delay_period;    
    XD12_Write_REGISTER(XD12_ADDR_DELAY_PERIOD, _xd12_delay_period.value);
    
    return TRUE;
}
#endif //DBG_TEST
/* END - DBG TEST ******************************************************************************/

/*** end of file ***/
