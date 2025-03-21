/*
 * File:   nmic.c
 * Author: GT
 *
 * Created on 2022, 07, 06
 */

#define __CQIC_C__
#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include "cqic.h"

#define _Y__  TRUE
const uint8_t REGISTER_USED_MAP_CQ24[]={
/*          0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F */
/*0x00*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__,
/*0x10*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, 0x00, 0x00, 0x00,
/*0x20*/    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*0x30*/    _Y__, _Y__, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00, _Y__, _Y__, 0x00, 0x00, 0x00, 0x00, 0x00,
/*0x40*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__,
/*0x50*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__,
/*0x60*/    _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__, _Y__,
/*0x70*/    _Y__
};

static _cmd1_reg_t_ cq24_cmd1_set_reg;
static _cmd2_reg_t_ cq24_cmd2_set_reg;

static _cmd1_reg_t_ cq24_cmd1_get_reg;
static _cmd2_reg_t_ cq24_cmd2_get_reg;

float gf_cq24_ado_voltage;

#define CQ24_DAISY_NUM 21

static uint16_t gn_spi_cq_daisied_buffer[CQ24_DAISY_NUM * 2] = {0, };

static void spi_write(uint16_t* pData, uint16_t length)
{
    for (uint8_t i = 0 ; i < length ; ++i)
    {
        LL_SPI_TransmitData16(SPI1, *(pData + i));
        while(!LL_SPI_IsActiveFlag_TXE(SPI1));
    }
    while(LL_SPI_IsActiveFlag_BSY(SPI1));
}

static void spi_read(uint16_t* pTxData, uint16_t* pRxData, uint16_t length)
{
    volatile uint16_t dummy = 0;
    dummy = LL_SPI_ReceiveData16(SPI1);

    LL_SPI_TransmitData16(SPI1, *(pTxData));
    while(!LL_SPI_IsActiveFlag_TXE(SPI1));

    for (uint8_t i = 0 ; i < length ; ++i)
    {
        LL_SPI_TransmitData16(SPI1, 0);
        while(!LL_SPI_IsActiveFlag_TXE(SPI1));

        while(!LL_SPI_IsActiveFlag_RXNE(SPI1));
        *(pRxData + i) = LL_SPI_ReceiveData16(SPI1);
    }
    while(LL_SPI_IsActiveFlag_BSY(SPI1));
}

static void cqic_write(uint16_t* pData, uint16_t length)
{
    //snprintf(msg_buffer, sizeof(msg_buffer), "pData[0] = 0x%X\r\n", *pData);
    //print(msg_buffer);

    uint8_t spi_write_flag = 0;

    if ((*pData & 0xC000) == (CMD_01 << 14))
    {
        //snprintf(msg_buffer, sizeof(msg_buffer), "CMD1!!\r\n");
        //print(msg_buffer);
        for (uint8_t i = 0 ; i < CQ24_DAISY_NUM ; ++i)
        {
            gn_spi_cq_daisied_buffer[i] = *(pData + 0);
        }
    }
    else if ((*pData & 0xC000) == (CMD_02 << 14))
    {
        //snprintf(msg_buffer, sizeof(msg_buffer), "CMD2!!\r\n");
        //print(msg_buffer);
        for (uint8_t i = 0 ; i < CQ24_DAISY_NUM ; ++i)
        {
            gn_spi_cq_daisied_buffer[i * 2 + 0] = *(pData + 0);
            gn_spi_cq_daisied_buffer[i * 2 + 1] = *(pData + 1);
        }
    }
    else
    {
        snprintf(msg_buffer, sizeof(msg_buffer), "UNKNOWN CMD!!\r\n");
        print(msg_buffer);
        spi_write_flag = 1;
    }

    if (spi_write_flag == 0)
    {
        CQ24_CS_LO();
        spi_write(gn_spi_cq_daisied_buffer, length * CQ24_DAISY_NUM);
        CQ24_CS_HI();
        HAL_Delay(1);
    }
}

static void cqic_write_daisy(uint16_t* pData, uint16_t length, uint16_t daisy_size)
{
    CQ24_CS_LO();
    for (uint8_t i = 0 ; i < daisy_size ; ++i)
    {
        spi_write(pData, length);
    }
    CQ24_CS_HI();
    HAL_Delay(1);
}

static void cqic_read(uint16_t* cmd, uint16_t* pData, uint16_t length)
{
    uint16_t rx_buffer[5] = {0, };

    CQ24_CS_LO();
    spi_read(cmd, rx_buffer, length);
    CQ24_CS_HI();

    for (uint8_t i = 0 ; i < 5 ; ++i)
    {
        snprintf(msg_buffer, sizeof(msg_buffer), "rxbuff[%u] : [0x%4X]\r\n", i, rx_buffer[i]);
        print(msg_buffer);
    }

    if(length == 1)
    {
        pData[1] = rx_buffer[0];
    }
    else
    {
        pData[1] = rx_buffer[1];
    }
}

void cqic_init(void)
{
    cqic_cmd1_type cmd1;
    cmd1.value = 0;

/*
    cmd1.u.cmd_id   = CMD_01;
    cmd1.u.rw       = SPI_WR;
    cmd1.u.addr     = SOFT_RESET;
    cq24_cmd1_set_reg.cmd1_r00.u.rst1 = 1;
    cmd1.u.data     = cq24_cmd1_set_reg.cmd1_r00.value;
    cqic_write(&cmd1.value, 1);

    cmd1.u.cmd_id   = CMD_01;
    cmd1.u.rw       = SPI_WR;
    cmd1.u.addr     = SOFT_RESET;
    cq24_cmd1_set_reg.cmd1_r00.u.rst1 = 0;
    cmd1.u.data     = cq24_cmd1_set_reg.cmd1_r00.value;
    cqic_write(&cmd1.value, 1);
*/

    cmd1.u.cmd_id   = CMD_01;
    cmd1.u.rw       = SPI_WR;
    cmd1.u.addr     = FIX_CONTROL;
    cq24_cmd1_set_reg.cmd1_r02.u.ld_force = 1;
    cq24_cmd1_set_reg.cmd1_r02.u.ld_i_fix = 1;
    cq24_cmd1_set_reg.cmd1_r02.u.duty_fix = 1;
    cq24_cmd1_set_reg.cmd1_r02.u.header_fix = 1;
    cmd1.u.data     = cq24_cmd1_set_reg.cmd1_r02.value;
    cqic_write(&cmd1.value, 1);

    cqic_set_duty_full();
    cqic_output_enable();
    cqic_set_vref(0xFFF);
    cqic_set_cz(CZ_1V5);

    gf_cq24_ado_voltage = 1.3f;
}

static void cqic_output_enable()
{
    cqic_cmd2_type cmd2;
    cmd2.value32 = 0;

    cmd2.u.cmd_id   = CMD_02;
    cmd2.u.rw       = SPI_WR;
    cmd2.u.addr     = OUTPUT_CH_EN1;
    cq24_cmd2_set_reg.cmd2_r19.value = 0xFFFF;
    cmd2.u.data     = cq24_cmd2_set_reg.cmd2_r19.value;
    cqic_write(cmd2.value, 2);

    cmd2.u.cmd_id   = CMD_02;
    cmd2.u.rw       = SPI_WR;
    cmd2.u.addr     = OUTPUT_CH_EN2;
    cq24_cmd2_set_reg.cmd2_r1A.value = 0x00FF;
    cmd2.u.data     = cq24_cmd2_set_reg.cmd2_r1A.value;
    cqic_write(cmd2.value, 2);
}

static void cqic_output_disable()
{
    cqic_cmd2_type cmd2;
    cmd2.value32 = 0;

    cmd2.u.cmd_id   = CMD_02;
    cmd2.u.rw       = SPI_WR;
    cmd2.u.addr     = OUTPUT_CH_EN1;
    cq24_cmd2_set_reg.cmd2_r19.value = 0x0000;
    cmd2.u.data     = cq24_cmd2_set_reg.cmd2_r19.value;
    cqic_write(cmd2.value, 2);

    cmd2.u.cmd_id   = CMD_02;
    cmd2.u.rw       = SPI_WR;
    cmd2.u.addr     = OUTPUT_CH_EN2;
    cq24_cmd2_set_reg.cmd2_r1A.value = 0x0000;
    cmd2.u.data     = cq24_cmd2_set_reg.cmd2_r1A.value;
    cqic_write(cmd2.value, 2);
}

static void cqic_set_vref(uint16_t in_vref)
{
    cqic_cmd2_type cmd2;
    cmd2.value32 = 0;

    /* set VREF_ANA */
    cmd2.u.cmd_id   = CMD_02;
    cmd2.u.rw       = SPI_WR;
    cmd2.u.addr     = FRAME_HEADER;
    cq24_cmd2_set_reg.cmd2_r10.u.frame_vref = in_vref;
    cmd2.u.data     = cq24_cmd2_set_reg.cmd2_r10.value;
    cqic_write(cmd2.value, 2);
}

static void cqic_set_cz(uint8_t in_cz)
{
    cqic_cmd2_type cmd2;
    cmd2.value32 = 0;

    /* set VREF_ANA */
    cmd2.u.cmd_id   = CMD_02;
    cmd2.u.rw       = SPI_WR;
    cmd2.u.addr     = FRAME_HEADER;
    cq24_cmd2_set_reg.cmd2_r10.u.frame_cz = in_cz;
    cmd2.u.data     = cq24_cmd2_set_reg.cmd2_r10.value;
    cqic_write(cmd2.value, 2);
}

static void cqic_set_duty_full()
{
    cqic_cmd2_type cmd2;
    cmd2.value32 = 0;

    cq24_cmd2_set_reg.cmd2_r11.u.duty_fix = 0xFF;

    /* set LD_I */
    cmd2.u.cmd_id   = CMD_02;
    cmd2.u.rw       = SPI_WR;
    cmd2.u.addr     = DUTY;
    cmd2.u.data     = cq24_cmd2_set_reg.cmd2_r11.value;
    cqic_write(cmd2.value, 2);
}

void cqic_set_ado(float in_f_ado_voltage)
{
    cqic_cmd2_type cmd2;
    cmd2.value32 = 0;

    uint16_t temp_ld = (uint16_t)(0xFFF * in_f_ado_voltage / 4.5f + 0.5);

    cq24_cmd2_set_reg.cmd2_r12.u.ld_i_fix = temp_ld;

    /* set LD_I */
    cmd2.u.cmd_id   = CMD_02;
    cmd2.u.rw       = SPI_WR;
    cmd2.u.addr     = LD_I;
    cmd2.u.data     = cq24_cmd2_set_reg.cmd2_r12.value;
    cqic_write(cmd2.value, 2);
}

void dump_cq24_reg()
{
#if 0
    snprintf(msg_buffer, sizeof(msg_buffer), "DUMP CQ24 REGISTERS\r\n");
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "CMD1\r\n");
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r00 : 0x%4X [SOFT RESET]\r\n", cq24_cmd1_set_reg.cmd1_r00.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r01 : 0x%4X [MODE CONTROL]\r\n", cq24_cmd1_set_reg.cmd1_r01.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r02 : 0x%4X [FIX CONTROL]\r\n", cq24_cmd1_set_reg.cmd1_r02.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r03 : 0x%4X [CS/HOLD FIX CHANNEL]\r\n", cq24_cmd1_set_reg.cmd1_r03.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "\r\n");
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "CMD2\r\n");
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r10 : 0x%4X [FRAME HEADER FIX]\r\n", cq24_cmd2_set_reg.cmd2_r10.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r11 : 0x%4X [DUTY FIX]\r\n", cq24_cmd2_set_reg.cmd2_r11.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r12 : 0x%4X [LD_I FIX]\r\n", cq24_cmd2_set_reg.cmd2_r12.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r19 : 0x%4X [OUTPUT CHANNEL 1]\r\n", cq24_cmd2_set_reg.cmd2_r19.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r1A : 0x%4X [OUTPUT CHANNEL 2]\r\n", cq24_cmd2_set_reg.cmd2_r1A.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "\r\n");
    print(msg_buffer);
#else
    cqic_cmd1_type cmd1;
    cmd1.value = 0;
    cqic_cmd2_type cmd2;
    cmd2.value32 = 0;

    /* read cmd1 reg */
    cmd1.u.cmd_id = CMD_01;
    cmd1.u.rw     = SPI_RD;
    cmd1.u.addr   = SOFT_RESET;
    cqic_read(&cmd1.value, &cq24_cmd1_get_reg.cmd1_r00.value, 1);

    cmd1.u.cmd_id = CMD_01;
    cmd1.u.rw     = SPI_RD;
    cmd1.u.addr   = MODE_CONTROL;
    cqic_read(&cmd1.value, &cq24_cmd1_get_reg.cmd1_r01.value, 1);

    cmd1.u.cmd_id = CMD_01;
    cmd1.u.rw     = SPI_RD;
    cmd1.u.addr   = FIX_CONTROL;
    cqic_read(&cmd1.value, &cq24_cmd1_get_reg.cmd1_r02.value, 1);

    cmd1.u.cmd_id = CMD_01;
    cmd1.u.rw     = SPI_RD;
    cmd1.u.addr   = CS_HOLD_FIX;
    cqic_read(&cmd1.value, &cq24_cmd1_get_reg.cmd1_r03.value, 1);

    /* read cmd2 reg */
    cmd2.u.cmd_id = CMD_02;
    cmd2.u.rw     = SPI_RD;
    cmd2.u.addr   = FRAME_HEADER;
    cqic_read(cmd2.value, &cq24_cmd2_get_reg.cmd2_r10.value, 2);

    cmd2.u.cmd_id = CMD_02;
    cmd2.u.rw     = SPI_RD;
    cmd2.u.addr   = DUTY;
    cqic_read(cmd2.value, &cq24_cmd2_get_reg.cmd2_r11.value, 2);

    cmd2.u.cmd_id = CMD_02;
    cmd2.u.rw     = SPI_RD;
    cmd2.u.addr   = LD_I;
    cqic_read(cmd2.value, &cq24_cmd2_get_reg.cmd2_r12.value, 2);

    cmd2.u.cmd_id = CMD_02;
    cmd2.u.rw     = SPI_RD;
    cmd2.u.addr   = OUTPUT_CH_EN1;
    cqic_read(cmd2.value, &cq24_cmd2_get_reg.cmd2_r19.value, 2);

    cmd2.u.cmd_id = CMD_02;
    cmd2.u.rw     = SPI_RD;
    cmd2.u.addr   = OUTPUT_CH_EN2;
    cqic_read(cmd2.value, &cq24_cmd2_get_reg.cmd2_r1A.value, 2);

    snprintf(msg_buffer, sizeof(msg_buffer), "DUMP CQ24 REGISTERS\r\n");
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "CMD1\r\n");
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r00 : 0x%4X [SOFT RESET]\r\n", cq24_cmd1_get_reg.cmd1_r00.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r01 : 0x%4X [MODE CONTROL]\r\n", cq24_cmd1_get_reg.cmd1_r01.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r02 : 0x%4X [FIX CONTROL]\r\n", cq24_cmd1_get_reg.cmd1_r02.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r03 : 0x%4X [CS/HOLD FIX CHANNEL]\r\n", cq24_cmd1_get_reg.cmd1_r03.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "\r\n");
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "CMD2\r\n");
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r10 : 0x%4X [FRAME HEADER FIX]\r\n", cq24_cmd2_get_reg.cmd2_r10.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r11 : 0x%4X [DUTY FIX]\r\n", cq24_cmd2_get_reg.cmd2_r11.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r12 : 0x%4X [LD_I FIX]\r\n", cq24_cmd2_get_reg.cmd2_r12.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r19 : 0x%4X [OUTPUT CHANNEL 1]\r\n", cq24_cmd2_get_reg.cmd2_r19.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "r1A : 0x%4X [OUTPUT CHANNEL 2]\r\n", cq24_cmd2_get_reg.cmd2_r1A.value);
    print(msg_buffer);
    snprintf(msg_buffer, sizeof(msg_buffer), "\r\n");
    print(msg_buffer);
#endif
}
