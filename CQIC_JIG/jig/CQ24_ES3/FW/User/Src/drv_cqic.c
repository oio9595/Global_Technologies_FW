/*
 * File:   drv_cq24.c
 * Author: GT
 *
 * Created on 2025, 03, 18
 */

#define _DRV_CQIC_C__
#include "main.h"
#include "drv_cqic.h"
#include "ads124s08.h"

#define CQ24_OTP_PROTECT_ENABLE     (0x5A5A)
#define CQ24_OTP_PROTECT_DISABLE    (0xA5A5)

typedef struct
{
    uint8_t target_regs_addr;
    uint16_t target_regs_data;
    bool write_flag;
    bool read_flag;
}CQ24_RW_Info;
static CQ24_RW_Info gt_cq24_cmd1_rw_info;
static CQ24_RW_Info gt_cq24_cmd2_rw_info;

static cq24_cmd1_regs gt_cq24_cmd1_regs;
static cq24_cmd2_regs gt_cq24_cmd2_regs;

static cq24_cmd3_duty_transfer gt_cq24_cmd3_duty;
static cq24_cmd4_ld_i_transfer gt_cq24_cmd4_ld_i;

static uint8_t gn_cq24_duty;
static uint16_t gn_cq24_ld_i;

extern SPI_HandleTypeDef hspi2;
bool gb_cq24_vsync_flag;

static void Spi_Write(uint16_t* pData, uint16_t length);
static void Spi_Read(uint16_t* pTxData, uint16_t* pRxData, uint16_t length);
static void CQ24_Write(uint16_t* pData, uint16_t length);
static uint16_t CQ24_Read(uint16_t* cmd, uint16_t* pData, uint16_t length);
static void CQ24_Read_All(void);

static void CQ24_Write_CMD1_Reg(void);
static void CQ24_Read_CMD1_Reg(void);

static void CQ24_Write_CMD2_Reg(void);
static void CQ24_Read_CMD2_Reg(void);

static void CQ24_Write_CMD3_Duty(void);
static void CQ24_Write_CMD4_LD_I(void);

static void CQ24_Reset(void);
static void CQ24_Register_Init(void);
static void CQ24_OTP_Protection(bool en);
static void CQ24_OTP_Download_Start(void);

static void Spi_Write(uint16_t* pData, uint16_t length)
{
    HAL_SPI_Transmit(&hspi2, (uint8_t*)pData, length, 100);
}

static void Spi_Read(uint16_t* pTxData, uint16_t* pRxData, uint16_t length)
{
    HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)pTxData, (uint8_t*)pRxData, length, 100);
}

static void CQ24_Write(uint16_t* pData, uint16_t length)
{
    CQ24_NSCS_LO();
    Spi_Write(pData, length);
    CQ24_NSCS_HI();
    HAL_Delay(1 - 1);
}

static uint16_t CQ24_Read(uint16_t* cmd, uint16_t* pData, uint16_t length)
{
    uint16_t rx_buffer[2] = {0, };

    CQ24_NSCS_LO();
    Spi_Read(cmd, rx_buffer, length);
    CQ24_NSCS_HI();
    HAL_Delay(1 - 1);

    if (length == 1)
    {
        pData[1] = rx_buffer[0];
    }
    else
    {
        pData[1] = rx_buffer[1];
    }

    return pData[1];
}

static void CQ24_Read_All(void)
{
    cq24_cmd1_type cmd1;
    cq24_cmd2_type cmd2;
    uint16_t rx_data[2] = {0, };

    Print("CQ24 CMD1 REGISTERS\r\n");
    for(uint8_t cmd1_addr = 0 ; cmd1_addr < CQ24_CMD1_REG_ADDR_MAX ; ++cmd1_addr)
    {
        cmd1.u.addr 	= cmd1_addr;
        cmd1.u.cmd_id 	= CMD_01;
        cmd1.u.rw		= CQ24_RD;
        cmd1.u.data		= 0;
        *(&gt_cq24_cmd1_regs._r00.value + cmd1_addr) = CQ24_Read(&cmd1.value, rx_data, 2);

        Print("[0x%02X] - [0x%03X]\r\n", cmd1_addr, *(&gt_cq24_cmd1_regs._r00.value + cmd1_addr));
    }
    Print("\r\n");

    // CMD_2
    Print("CQ24 CMD2 REGISTERS\r\n");
    for(uint8_t cmd2_addr = 0 ; cmd2_addr < CQ24_CMD2_REG_ADDR_MAX ; ++cmd2_addr)
    {
        cmd2.u.addr 	= cmd2_addr;
        cmd2.u.rw		= CQ24_RD;
        cmd2.u.cmd_id	= CMD_02;
        cmd2.u.data 	= 0;
        *(&gt_cq24_cmd2_regs._r00.value + cmd2_addr) = CQ24_Read(cmd2.value, rx_data, 2);

        Print("[0x%02X] - [0x%04X]\r\n", cmd2_addr, *(&gt_cq24_cmd2_regs._r00.value + cmd2_addr));
    }
    Print("\r\n");
}

void CQ24_Set_CMD1_Target_Reg(uint8_t cq24_rw, uint8_t cmd1_addr, uint16_t cmd1_data)
{
    gt_cq24_cmd1_rw_info.target_regs_addr = cmd1_addr;
    gt_cq24_cmd1_rw_info.target_regs_data = cmd1_data;

    if (cq24_rw == CQ24_WR)
    {
        gt_cq24_cmd1_rw_info.write_flag = true;
    }
    else
    {
        gt_cq24_cmd1_rw_info.read_flag = true;
    }
}

static void CQ24_Write_CMD1_Reg(void)
{
    cq24_cmd1_type cmd1 = {0, };
    uint8_t cmd1_addr = 0;
    uint16_t cmd1_data = 0;

    if (gt_cq24_cmd1_rw_info.write_flag == true)
    {
        cmd1_addr = gt_cq24_cmd1_rw_info.target_regs_addr;
        cmd1_data = gt_cq24_cmd1_rw_info.target_regs_data;

        cmd1.u.cmd_id = CMD_01;
        cmd1.u.rw = CQ24_WR;
        cmd1.u.addr = cmd1_addr;
        cmd1.u.data = cmd1_data;
        CQ24_Write(&cmd1.value, 1);

        *(&gt_cq24_cmd1_regs._r00.value + cmd1_addr) = cmd1_data;
        gt_cq24_cmd1_rw_info.write_flag = false;

        Print("\t CQ24 CMD1 Write [0x%02X --> 0x%04X]\r\n", cmd1_addr, cmd1_data);
    }
}

static void CQ24_Read_CMD1_Reg(void)
{
    cq24_cmd1_type cmd1 = {0, };
    uint8_t cmd1_addr = 0;
    uint16_t cmd1_data = 0;
    uint16_t rx_data[2] = {0, };

    if (gt_cq24_cmd1_rw_info.read_flag == true)
    {
        cmd1_addr = gt_cq24_cmd1_rw_info.target_regs_addr;

        cmd1.u.cmd_id = CMD_01;
        cmd1.u.rw = CQ24_RD;
        cmd1.u.addr = cmd1_addr;
        cmd1.u.data = 0;

        cmd1_data = CQ24_Read(&cmd1.value, rx_data, 2);
        *(&gt_cq24_cmd1_regs._r00.value + cmd1_addr) = cmd1_data;
        gt_cq24_cmd1_rw_info.read_flag = false;

        Print("\t CQ24 CMD1 Read [0x%02X --> 0x%04X]\r\n", cmd1_addr, cmd1_data);
    }
}

void CQ24_Set_CMD2_Target_Reg(uint8_t cq24_rw, uint8_t cmd2_addr, uint16_t cmd2_data)
{
    gt_cq24_cmd2_rw_info.target_regs_addr = cmd2_addr;
    gt_cq24_cmd2_rw_info.target_regs_data = cmd2_data;

    if (cq24_rw == CQ24_WR)
    {
        gt_cq24_cmd2_rw_info.write_flag = true;
    }
    else
    {
        gt_cq24_cmd2_rw_info.read_flag = true;
    }
}

static void CQ24_Write_CMD2_Reg(void)
{
    cq24_cmd2_type cmd2 = {0, };
    uint8_t cmd2_addr = 0;
    uint16_t cmd2_data = 0;

    if (gt_cq24_cmd2_rw_info.write_flag == true)
    {
        cmd2_addr = gt_cq24_cmd2_rw_info.target_regs_addr;
        cmd2_data = gt_cq24_cmd2_rw_info.target_regs_data;

        cmd2.u.cmd_id = CMD_02;
        cmd2.u.rw = CQ24_WR;
        cmd2.u.addr = cmd2_addr;
        cmd2.u.data = cmd2_data;
        CQ24_Write(cmd2.value, 1);

        *(&gt_cq24_cmd2_regs._r00.value + cmd2_addr) = cmd2_data;
        gt_cq24_cmd2_rw_info.write_flag = false;

        Print("\t CQ24 CMD2 Write [0x%02X --> 0x%04X]\r\n", cmd2_addr, cmd2_data);
    }
}

static void CQ24_Read_CMD2_Reg(void)
{
    cq24_cmd2_type cmd2 = {0, };
    uint8_t cmd2_addr = 0;
    uint16_t cmd2_data = 0;
    uint16_t rx_data[2] = {0, };

    if (gt_cq24_cmd2_rw_info.read_flag == true)
    {
        cmd2_addr = gt_cq24_cmd2_rw_info.target_regs_addr;

        cmd2.u.cmd_id = CMD_02;
        cmd2.u.rw = CQ24_RD;
        cmd2.u.addr = cmd2_addr;
        cmd2.u.data = 0;

        cmd2_data = CQ24_Read(cmd2.value, rx_data, 2);
        *(&gt_cq24_cmd2_regs._r00.value + cmd2_addr) = cmd2_data;
        gt_cq24_cmd2_rw_info.read_flag = false;

        Print("\t CQ24 CMD2 Read [0x%02X --> 0x%04X]\r\n", cmd2_addr, cmd2_data);
    }
}

static void CQ24_Write_CMD3_Duty(void)
{
    for (uint8_t idx = 0 ; idx < DUTY_SIZE ; ++idx)
    {
        gt_cq24_cmd3_duty.duty[idx] = (gn_cq24_duty << 8 | gn_cq24_duty << 0);
    }
    CQ24_Write(&gt_cq24_cmd3_duty.cmd3.value, (DUTY_SIZE + 1));
}

void CQ24_Set_CMD3_Duty(uint16_t duty)
{
    if (duty < (0xFF + 1))
    {
        gn_cq24_duty = duty;
    }
    else
    {
        Print("Invalid Duty Input [%u] - [%u - %u]\r\n", duty, 0, 0xFF);
    }
}

static void CQ24_Write_CMD4_LD_I(void)
{
    for (uint16_t idx = 0 ; idx < LD_I_SIZE ; ++idx)
    {
        gt_cq24_cmd4_ld_i.ld_i[idx] = gn_cq24_ld_i;
    }
    CQ24_Write(gt_cq24_cmd4_ld_i.ld_i, LD_I_SIZE);
}

void CQ24_Set_CMD4_LD_I(uint16_t ld_i)
{
    if (ld_i < (0xFFF + 1))
    {
        gn_cq24_ld_i = ld_i;
    }
    else
    {
        Print("Invalid LD_I Input [%u] - [%u - %u]\r\n", ld_i, 0, 0xFFF);
    }
}

static void CQ24_Reset(void)
{
    cq24_cmd1_type cmd1 = {0, };

    gt_cq24_cmd1_regs._r00.u.rst1 = 1;

    cmd1.u.cmd_id = CMD_01;
    cmd1.u.rw = CQ24_WR;
    cmd1.u.addr = CQ24_CMD1_SOFT_RESET;
    cmd1.u.data = *(&gt_cq24_cmd1_regs._r00.value + CQ24_CMD1_SOFT_RESET);
    CQ24_Write(&cmd1.value, 1);
    HAL_Delay(10 - 1);

    gt_cq24_cmd1_regs._r00.u.rst1 = 0;

    cmd1.u.cmd_id = CMD_01;
    cmd1.u.rw = CQ24_WR;
    cmd1.u.addr = CQ24_CMD1_SOFT_RESET;
    cmd1.u.data = *(&gt_cq24_cmd1_regs._r00.value + CQ24_CMD1_SOFT_RESET);
    CQ24_Write(&cmd1.value, 1);
    HAL_Delay(10 - 1);
}

static void CQ24_Register_Init(void)
{
    cq24_cmd1_type cmd1 = {0, };
    cq24_cmd2_type cmd2 = {0, };

    for (uint8_t cmd1_addr = 0 ; cmd1_addr < CQ24_CMD1_REG_ADDR_MAX ; ++cmd1_addr)
    {
        switch (cmd1_addr)
        {
        /*case CQ24_CMD1_SOFT_RESET :
            break;
        */
        case CQ24_CMD1_MODE_CONTROL :
            gt_cq24_cmd1_regs._r01.u.drv_mode = 1;
            gt_cq24_cmd1_regs._r01.u.normal = 1;
            gt_cq24_cmd1_regs._r01.u.frame_keep = 1;
            break;
        case CQ24_CMD1_FIX_CONTROL :
            gt_cq24_cmd1_regs._r02.u.dup_o = 1;
            break;
        /*case CQ24_CMD1_CS_HOLD_FIX :
            break;
        */
        default :
            continue;
        }

        cmd1.u.cmd_id = CMD_01;
        cmd1.u.rw = CQ24_WR;
        cmd1.u.addr = cmd1_addr;
        cmd1.u.data = *(&gt_cq24_cmd1_regs._r00.value + cmd1_addr);
        CQ24_Write(&cmd1.value, 1);
    }

    for (uint8_t cmd2_addr = 0 ; cmd2_addr < CQ24_CMD2_REG_ADDR_MAX ; ++cmd2_addr)
    {
        switch (cmd2_addr)
        {
        case CQ24_CMD2_FRAME_SIZE :
            gt_cq24_cmd2_regs._r00.u.svsync_size = SVSYNC_SIZE;
            gt_cq24_cmd2_regs._r00.u.shsync_size = SHSYNC_SIZE;
            break;
        case CQ24_CMD2_FRAME_START_TIME :
            gt_cq24_cmd2_regs._r01.u.svsync_start = 1;
            gt_cq24_cmd2_regs._r01.u.shsync_start = 1;
            break;
        case CQ24_CMD2_CH_SIZE_SVSYNC_DELAY :
            gt_cq24_cmd2_regs._r02.u.output_ch_size = 24;
            break;
        /*case CQ24_CMD2_SVSYNC_OFFSET :
            break;
        */
        case CQ24_CMD2_SVSYNC_PERIOD :
            gt_cq24_cmd2_regs._r04.u.svsync_period = 0x98A;
            break;
        case CQ24_CMD2_SHSYNC_PERIOD :
            gt_cq24_cmd2_regs._r05.u.shsync_period = 0x19C;
            break;
        /*case CQ24_CMD2_HOLD_SIZE1 :
            break;
        case CQ24_CMD2_HOLD_SIZE2 :
            break;
        case CQ24_CMD2_HOLD_SIZE3 :
            break;
        case CQ24_CMD2_HOLD_DUMMY_SIZE1 :
            break;
        case CQ24_CMD2_HOLD_DUMMY_SIZE2 :
            break;
        case CQ24_CMD2_HOLD_SETUP_TIME :
            break;
        case CQ24_CMD2_HOLD_ON_TIME :
            break;
        case CQ24_CMD2_X_PERIOD :
            break;
        case CQ24_CMD2_VSYNC_COMP_TIME1 :
            break;
        case CQ24_CMD2_VSYNC_COMP_TIME2 :
            break;
        */
        case CQ24_CMD2_FRAME_HEADER_FIX :
            gt_cq24_cmd2_regs._r10.u.frame_vref = 4095;
            gt_cq24_cmd2_regs._r10.u.frame_cz = 2;
            break;
        /*case CQ24_CMD2_DUTY_FIX :
            break;
        case CQ24_CMD2_LD_I_FIX :
            break;
        case CQ24_CMD2_STB_DUP_CHANNEL_EN :
            break;
        case CQ24_CMD2_STB_DUP_CH_GROUP1 :
            break;
        case CQ24_CMD2_STB_DUP_CH_GROUP2 :
            break;
        case CQ24_CMD2_STB_DUP_CH_GROUP3 :
            break;
        */
        case CQ24_CMD2_FULL_STEP_SIZE :
            gt_cq24_cmd2_regs._r17.u.full_step_size = 0xFFF;
            break;
        /*case CQ24_CMD2_FULL_STEP_DUTY_TH :
            break;
        */
        case CQ24_CMD2_OUTPUT_CH_EN1 :
            gt_cq24_cmd2_regs._r19.value = 0xFFFF;
            break;
        case CQ24_CMD2_OUTPUT_CH_EN2 :
            gt_cq24_cmd2_regs._r1A.value = 0xFF;
            break;
        /*case CQ24_CMD2_MCLK_LOCK_CNT1 :
            break;
        case CQ24_CMD2_MCLK_LOCK_CNT2 :
            break;
        case CQ24_CMD2_TRIM_CONTROL :
            break;
        case CQ24_CMD2_OTP_WR_CONTROL :
            break;
        case CQ24_CMD2_OTP_RD_PG_CONTROL :
            break;
        case CQ24_CMD2_OTP_PROTECTION :
            break;
        case CQ24_CMD2_OTP_CHECK_SUM :
            break;
        */
        default :
            continue;
        }
        cmd2.u.cmd_id = CMD_02;
        cmd2.u.rw = CQ24_WR;
        cmd2.u.addr = cmd2_addr;
        cmd2.u.data = *(&gt_cq24_cmd2_regs._r00.value + cmd2_addr);
        CQ24_Write(cmd2.value, 2);
    }

    gt_cq24_cmd3_duty.cmd3.u.frame_vref = 0xFFF;
    gt_cq24_cmd3_duty.cmd3.u.frame_cz = 2;
    gt_cq24_cmd3_duty.cmd3.u.cmd_id = CMD_03;

    gn_cq24_duty = 0x0C;
    gn_cq24_ld_i = 0xFFF;
}

static void CQ24_OTP_Protection(bool en)
{
    cq24_cmd2_type cmd2 = {0, };

    if (en == true)
    {
        gt_cq24_cmd2_regs._r33.u.otp_protect = CQ24_OTP_PROTECT_ENABLE;
    }
    else
    {
        gt_cq24_cmd2_regs._r33.u.otp_protect = CQ24_OTP_PROTECT_DISABLE;
    }

    cmd2.u.cmd_id = CMD_02;
    cmd2.u.rw = CQ24_WR;
    cmd2.u.addr = CQ24_CMD2_OTP_PROTECTION;
    cmd2.u.data = *(&gt_cq24_cmd2_regs._r00.value + CQ24_CMD2_OTP_PROTECTION);
    CQ24_Write(cmd2.value, 2);
}

static void CQ24_OTP_Download_Start(void)
{
    CQ24_OTP_Protection(false);

    cq24_cmd2_type cmd2 = {0, };

    gt_cq24_cmd2_regs._r32.u.otp_rd_start = 1;
    cmd2.u.cmd_id = CMD_02;
    cmd2.u.rw = CQ24_WR;
    cmd2.u.addr = CQ24_CMD2_OTP_RD_PG_CONTROL;
    cmd2.u.data = *(&gt_cq24_cmd2_regs._r00.value + CQ24_CMD2_OTP_RD_PG_CONTROL);
    CQ24_Write(cmd2.value, 2);

    gt_cq24_cmd2_regs._r32.u.otp_rd_start = 0;
    cmd2.u.cmd_id = CMD_02;
    cmd2.u.rw = CQ24_WR;
    cmd2.u.addr = CQ24_CMD2_OTP_RD_PG_CONTROL;
    cmd2.u.data = *(&gt_cq24_cmd2_regs._r00.value + CQ24_CMD2_OTP_RD_PG_CONTROL);
    CQ24_Write(cmd2.value, 2);

    HAL_Delay(10 - 1);

    uint16_t otp_after_val = 0;

    Print("OTP Download         - [addr] - [before] - [after]\r\n");
    for(uint8_t cmd2_addr = 0 ; cmd2_addr < CQ24_CMD2_REG_ADDR_MAX ; ++cmd2_addr)
    {
        cmd2.u.addr 	= cmd2_addr;
        cmd2.u.rw		= CQ24_RD;
        cmd2.u.cmd_id	= CMD_02;
        cmd2.u.data 	= 0;

        otp_after_val = CQ24_Read(cmd2.value, cmd2.value, 2);

        if (*(&gt_cq24_cmd2_regs._r00.value + cmd2_addr) != otp_after_val)
        {
            Print("Error   - [0x%02X] - [0x%04X] - [0x%04X]\r\n", cmd2_addr, *(&gt_cq24_cmd2_regs._r00.value + cmd2_addr), otp_after_val);
        }
        else
        {
            Print("Success - [0x%02X] - [0x%04X] - [0x%04X]\r\n", cmd2_addr, *(&gt_cq24_cmd2_regs._r00.value + cmd2_addr), otp_after_val);
        }
        *(&gt_cq24_cmd2_regs._r00.value + cmd2_addr) = otp_after_val;
    }
    Print("\r\n");
}

void CQ24_Init(void)
{
    CQ24_VCC_5V_ON();
    CQ24_VCC_ON();
    HAL_Delay(30 - 1);
    CQ24_NSCS_HI();
    CQ24_Set_MCLK(true);
    HAL_Delay(10 - 1);

    CQ24_Reset();

    CQ24_Register_Init();
    CQ24_Read_All();

    CQ24_OTP_Download_Start();
    //CQ24_Read_All();

    CQ24_Set_Vsync(true);
}

void CQ24_Vsync_Task(void)
{
    if (gb_cq24_vsync_flag)
    {
        HAL_GPIO_TogglePin(DEBUG_GPIO_Port, DEBUG_Pin);

        CQ24_Write_CMD3_Duty();
        CQ24_Write_CMD4_LD_I();

        CQ24_Write_CMD1_Reg();
        CQ24_Read_CMD1_Reg();

        CQ24_Write_CMD2_Reg();
        CQ24_Read_CMD2_Reg();

        gb_cq24_vsync_flag = false;
    }
}
