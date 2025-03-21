//#include <stdio.h>
#include <stdint.h>

#include "types.h"
#include "my_i2c.h"

#if 0
static void MY_IIC_Start(void);
static void MY_IIC_Stop(void);
static void MY_IIC_Send_Byte(uint8_t txd);
static uint8_t MY_IIC_Read_Byte(uint8_t ack);
static uint8_t MY_IIC_Wait_Ack(void);
static void MY_IIC_Ack(void);
static void MY_IIC_NAck(void);

static void delay_us(volatile uint32_t us_delay)
{
    for( ; us_delay!=0 ; --us_delay)
    {
        asm("NOP");
    }
}

// initialization IIC
static void MY_IIC_Init(void)
{
    IIC_SCL_HIGH();
    IIC_SDA_HIGH();
}
// produce IIC Start signal 
static void MY_IIC_Start(void)
{
    SDA_OUT();     //sda Line out
    IIC_SDA_HIGH();
    IIC_SCL_HIGH();
    delay_us(4);
    IIC_SDA_LOW();//START:when CLK is high,DATA change form high to low
    delay_us(4);
    IIC_SCL_LOW();// Hold on I2C Bus , Ready to send or receive data
}
// produce IIC Stop signal
static void MY_IIC_Stop(void)
{
    SDA_OUT();//sda Line out
    IIC_SCL_LOW();
    IIC_SDA_LOW();//STOP:when CLK is high DATA change form low to high
    delay_us(4);
    IIC_SCL_HIGH();
    IIC_SDA_HIGH();// send out I2C Bus end signal
    delay_us(4);
}
// Waiting for the answer signal to arrive 
// Return value ：1, Failed to receive response 
//        0, Received response successfully 
static uint8_t MY_IIC_Wait_Ack(void)
{
    uint8_t ucErrTime=0;

    SDA_IN();      //SDA Set to input
    IIC_SDA_HIGH();
    delay_us(1);
    IIC_SCL_HIGH();
    delay_us(1);
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            MY_IIC_Stop();
            return 1;
        }
    }
    IIC_SCL_LOW();// Clock output 0

    return 0;
} 
// produce ACK The reply
static void MY_IIC_Ack(void)
{
    IIC_SCL_LOW();
    SDA_OUT();
    IIC_SDA_LOW();
    delay_us(2);
    IIC_SCL_HIGH();
    delay_us(2);
    IIC_SCL_LOW();
}
// Do not produce ACK The reply
static void MY_IIC_NAck(void)
{
    IIC_SCL_LOW();
    SDA_OUT();
    IIC_SDA_HIGH();
    delay_us(2);
    IIC_SCL_HIGH();
    delay_us(2);
    IIC_SCL_LOW();
}
//IIC Send a byte 
// Return whether the slave has a response 
//1, There's a response 
//0, No response 			  
static void MY_IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;   

    SDA_OUT(); 	    
    IIC_SCL_LOW();// Pull down the clock and start data transmission 
    for(t=0;t<8;t++)
    {
        if((txd&0x80)>>7)
        {
            IIC_SDA_HIGH();
        }
        else
        {
            IIC_SDA_LOW();
        }

        txd<<=1;
        delay_us(2);   // Yes TEA5767 All three delays are necessary
        IIC_SCL_HIGH();
        delay_us(2);
        IIC_SCL_LOW();
        delay_us(2);
    }	 
} 	    
// read 1 Bytes ,ack=1 when , send out ACK,ack=0, send out nACK   
static uint8_t MY_IIC_Read_Byte(uint8_t ack)
{
    uint8_t i,receive=0;

    SDA_IN();//SDA Set to input
    for(i=0;i<8;i++ )
    {
        IIC_SCL_LOW();
        delay_us(2);
        IIC_SCL_HIGH();
        receive<<=1;
        if(READ_SDA)
        {
            receive++;
        }
        delay_us(1);
    }	 

    if (!ack)
    {
        MY_IIC_NAck();// send out nACK
    }
    else
    {
        MY_IIC_Ack(); // send out ACK
    }

    return receive;
}

uint8_t MY_IIC_Write_One_Byte(uint8_t addr, uint8_t regaddr, uint8_t data)
{
    uint8_t returnack = TRUE;

    MY_IIC_Init();
    MY_IIC_Start();

    MY_IIC_Send_Byte(addr);
    if(MY_IIC_Wait_Ack())
    {
        returnack = FALSE;
    }

    delay_us(SW_I2C_WAIT_TIME);

    MY_IIC_Send_Byte(regaddr);
    if(MY_IIC_Wait_Ack())
    {
        returnack = FALSE;
    }

    delay_us(SW_I2C_WAIT_TIME);

    MY_IIC_Send_Byte(data);
    if(MY_IIC_Wait_Ack())
    {
        returnack = FALSE;
    }

    delay_us(SW_I2C_WAIT_TIME);

    MY_IIC_Stop();

    return returnack;
}

uint8_t MY_IIC_Read_One_Byte(uint8_t addr, uint8_t regaddr)
{
    uint8_t  readdata = 0;

    MY_IIC_Init();

    MY_IIC_Start();

    MY_IIC_Send_Byte(addr);
    MY_IIC_Wait_Ack();


    MY_IIC_Send_Byte(regaddr);
    MY_IIC_Wait_Ack();

    delay_us(SW_I2C_WAIT_TIME);

    MY_IIC_Start();

    MY_IIC_Send_Byte(addr | I2C_READ);
    MY_IIC_Wait_Ack();

    delay_us(SW_I2C_WAIT_TIME);

    readdata = MY_IIC_Read_Byte(1);

    MY_IIC_Stop();

    return readdata;
}

#else


#define  SW_I2C_WAIT_TIME  22	

#define  I2C_READ       0x01
#define  READ_CMD       1
#define  WRITE_CMD      0

#define SW_I2C1_SCL_GPIO  MCU_I2C_SCL_CS_GPIO_Port
#define SW_I2C1_SDA_GPIO  MCU_I2C_SDA_G_GPIO_Port
#define SW_I2C1_SCL_PIN   MCU_I2C_SCL_CS_Pin
#define SW_I2C1_SDA_PIN   MCU_I2C_SDA_G_Pin

void TIMER__Wait_us(__IO uint32_t nCount)
{
    for (; nCount != 0;nCount--);
}


void SW_I2C_initial(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin   = SW_I2C1_SCL_PIN;
    HAL_GPIO_Init(SW_I2C1_SCL_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.Pin   = SW_I2C1_SDA_PIN;
    HAL_GPIO_Init(SW_I2C1_SDA_GPIO, &GPIO_InitStructure);
}

void GPIO_SetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_SET);
}

void GPIO_ResetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_RESET);
}

uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    uint8_t Ret;
    Ret=(uint16_t)HAL_GPIO_ReadPin(GPIOx,GPIO_Pin);
    return Ret;
}
	
void sda_high(void)
{
    GPIO_SetBits(SW_I2C1_SDA_GPIO, SW_I2C1_SDA_PIN);
}

void sda_low(void)
{
    GPIO_ResetBits(SW_I2C1_SDA_GPIO, SW_I2C1_SDA_PIN);
}

void scl_high(void)
{
    GPIO_SetBits(SW_I2C1_SCL_GPIO, SW_I2C1_SCL_PIN);
}

void scl_low(void)
{
    GPIO_ResetBits(SW_I2C1_SCL_GPIO, SW_I2C1_SCL_PIN);
}

void sda_out(uint8_t out)
{
    if (out)
    {
        sda_high();
    }
    else
    {
        sda_low();
    }
}

void sda_in_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode=GPIO_MODE_INPUT;
    GPIO_InitStructure.Pin   = SW_I2C1_SDA_PIN;

    HAL_GPIO_Init(SW_I2C1_SDA_GPIO, &GPIO_InitStructure);
}

void sda_out_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode=GPIO_MODE_OUTPUT_OD;

    GPIO_InitStructure.Pin   = SW_I2C1_SDA_PIN;
    HAL_GPIO_Init(SW_I2C1_SDA_GPIO, &GPIO_InitStructure);
}

void scl_in_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode=GPIO_MODE_OUTPUT_OD;

    GPIO_InitStructure.Pin   = SW_I2C1_SCL_PIN;
    HAL_GPIO_Init(SW_I2C1_SCL_GPIO, &GPIO_InitStructure);
}

void scl_out_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode=GPIO_MODE_OUTPUT_OD;

    GPIO_InitStructure.Pin   = SW_I2C1_SCL_PIN;
    HAL_GPIO_Init(SW_I2C1_SCL_GPIO, &GPIO_InitStructure);
}

void i2c_clk_data_out(void)
{
    scl_high();
    TIMER__Wait_us(SW_I2C_WAIT_TIME);
    scl_low();
}

void i2c_port_initial(void)
{
    sda_high();
    scl_high();
}

void i2c_start_condition(void)
{
    sda_high();
    scl_high();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);
    sda_low();
    TIMER__Wait_us(SW_I2C_WAIT_TIME);
    scl_low();

    TIMER__Wait_us(SW_I2C_WAIT_TIME << 1);
}

void i2c_stop_condition(void)
{
    sda_low();
    scl_high();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);
    sda_high();
    TIMER__Wait_us(SW_I2C_WAIT_TIME);
}

uint8_t i2c_check_ack(void)
{
    uint8_t         ack;
    int             i;
    unsigned int    temp;

    sda_in_mode();

    scl_high();

    ack = 0;
    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    for (i = 10; i > 0; i--)
    {
        temp = !(SW_I2C_ReadVal_SDA());
        if (temp)	
        {
            ack = 1;
            break;
        }
    }
    scl_low();
    sda_out_mode();	

    TIMER__Wait_us(SW_I2C_WAIT_TIME);
    return ack;
}

void i2c_check_not_ack(void)
{
    sda_in_mode();
    i2c_clk_data_out();
    sda_out_mode();
    TIMER__Wait_us(SW_I2C_WAIT_TIME);
}

void i2c_check_not_ack_continue(void)
{
    i2c_clk_data_out();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);
}

void i2c_slave_address(uint8_t IICID, uint8_t readwrite)
{
    int x;

    if (readwrite)
    {
        IICID |= I2C_READ;
    }
    else
    {
        IICID &= ~I2C_READ;
    }

    scl_low();

    for (x = 7; x >= 0; x--)
    {
        sda_out(IICID & (1 << x));
        TIMER__Wait_us(SW_I2C_WAIT_TIME);
        i2c_clk_data_out();
    }
}

void i2c_register_address(uint8_t addr)
{
    int  x;

    scl_low();

    for (x = 7; x >= 0; x--)
    {
        sda_out(addr & (1 << x));
        TIMER__Wait_us(SW_I2C_WAIT_TIME);
        i2c_clk_data_out();
    }
}

void i2c_send_ack(void)
{
    sda_out_mode();
    sda_low();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);
    scl_high();

    TIMER__Wait_us(SW_I2C_WAIT_TIME << 1);

    sda_low();
    TIMER__Wait_us(SW_I2C_WAIT_TIME << 1);

    scl_low();

    sda_out_mode();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);
}

uint8_t SW_I2C_ReadVal_SDA(void)
{
    return GPIO_ReadInputDataBit(SW_I2C1_SDA_GPIO, SW_I2C1_SDA_PIN);
}

uint8_t SW_I2C_ReadVal_SCL(void)
{
    return GPIO_ReadInputDataBit(SW_I2C1_SCL_GPIO, SW_I2C1_SCL_PIN);
}

void SW_I2C_Write_Data(uint8_t data)
{
    int  x;

    scl_low();

    for (x = 7; x >= 0; x--)
    {
        sda_out(data & (1 << x));
        TIMER__Wait_us(SW_I2C_WAIT_TIME);
        i2c_clk_data_out();
    }
}

uint8_t SW_I2C_Read_Data(void)
{
    int      x;
    uint8_t  readdata = 0;

    sda_in_mode();

    for (x = 8; x--;)
    {
        scl_high();

        readdata <<= 1;
        if (SW_I2C_ReadVal_SDA())
            readdata |= 0x01;

        TIMER__Wait_us(SW_I2C_WAIT_TIME);
        scl_low();

        TIMER__Wait_us(SW_I2C_WAIT_TIME);
    }

    sda_out_mode();
    return readdata;
}

uint8_t SW_I2C_WriteControl_8Bit(uint8_t IICID, uint8_t regaddr, uint8_t data)
{
    uint8_t   returnack = TRUE;

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_register_address(regaddr);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    SW_I2C_Write_Data(data);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_stop_condition();

    return returnack;
}

uint8_t SW_I2C_WriteControl_8Bit_OnlyRegAddr(uint8_t IICID, uint8_t regaddr)
{
    uint8_t   returnack = TRUE;

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    i2c_register_address(regaddr);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    return returnack;
}

uint8_t SW_I2C_WriteControl_16Bit(uint8_t IICID, uint8_t regaddr, uint16_t data)
{
    uint8_t   returnack = TRUE;

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_register_address(regaddr);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    SW_I2C_Write_Data((data >> 8) & 0xFF);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    SW_I2C_Write_Data(data & 0xFF);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_stop_condition();

    return returnack;
}

uint8_t SW_I2C_ReadControl_8Bit_OnlyRegAddr(uint8_t IICID, uint8_t regaddr)
{
    uint8_t   returnack = TRUE;

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_register_address(regaddr);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_stop_condition();

    return returnack;
}

uint8_t SW_I2C_ReadControl_8Bit_OnlyData(uint8_t IICID)
{
    uint8_t  readdata = 0;

    i2c_port_initial();

    i2c_start_condition();

    i2c_slave_address(IICID, READ_CMD);
    i2c_check_ack();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    readdata = SW_I2C_Read_Data();

    i2c_check_not_ack();

    i2c_stop_condition();

    return readdata;
}

uint16_t SW_I2C_ReadControl_16Bit_OnlyData(uint8_t IICID)
{
    uint8_t  readimsi = 0;
    uint16_t  readdata = 0;

    i2c_start_condition();

    i2c_slave_address(IICID, READ_CMD);
    i2c_check_not_ack();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    readimsi = SW_I2C_Read_Data();
    i2c_check_not_ack_continue();

    readdata = readimsi<<8;

    readimsi = SW_I2C_Read_Data();
    i2c_check_not_ack();


    readdata |= readimsi;

    i2c_stop_condition();

    return readdata;
}

uint8_t SW_I2C_ReadControl_8Bit(uint8_t IICID, uint8_t regaddr)
{
    uint8_t  readdata = 0;

    i2c_port_initial();

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    i2c_check_ack();

    i2c_register_address(regaddr);
    i2c_check_ack();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_start_condition();

    i2c_slave_address(IICID, READ_CMD);
    i2c_check_ack();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    readdata = SW_I2C_Read_Data();

    i2c_check_not_ack();

    i2c_stop_condition();

    return readdata;
}

uint16_t SW_I2C_ReadControl_16Bit(uint8_t IICID, uint8_t regaddr)
{
    uint16_t  readdata = 0;

    i2c_port_initial();

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    i2c_check_ack();

    i2c_register_address(regaddr);
    i2c_check_ack();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_start_condition();

    i2c_slave_address(IICID, READ_CMD);
    i2c_check_ack();

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    readdata = SW_I2C_Read_Data();
    i2c_send_ack();
    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    readdata = ((readdata << 8) | SW_I2C_Read_Data());

    i2c_check_not_ack();

    i2c_stop_condition();

    return readdata;
}

uint8_t SW_I2C_ReadnControl_8Bit(uint8_t IICID, uint8_t regaddr, uint8_t rcnt, uint8_t (*pdata))
{
    uint8_t   returnack = TRUE;
    uint8_t  index;

    i2c_port_initial();

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    if (!i2c_check_ack()) { returnack = FALSE; }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_register_address(regaddr);
    if (!i2c_check_ack()) { returnack = FALSE; }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_start_condition();

    i2c_slave_address(IICID, READ_CMD);
    if (!i2c_check_ack()) { returnack = FALSE; }

    for ( index = 0 ; index < rcnt ; index++){
        TIMER__Wait_us(SW_I2C_WAIT_TIME);
        pdata[index] = SW_I2C_Read_Data();
    }

    pdata[rcnt-1] = SW_I2C_Read_Data();

    i2c_check_not_ack();

    i2c_stop_condition();

    return returnack;
}

uint8_t SW_I2C_Multi_ReadnControl_8Bit(uint8_t IICID, uint8_t regaddr, uint8_t rcnt, uint8_t (*pdata))
{
    uint8_t   returnack = TRUE;
    uint8_t  index;

    i2c_port_initial();

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    if (!i2c_check_ack()) { returnack = FALSE; }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_register_address(regaddr);
    if (!i2c_check_ack()) { returnack = FALSE; }

    TIMER__Wait_us(SW_I2C_WAIT_TIME);

    i2c_start_condition();

    i2c_slave_address(IICID, READ_CMD);
    if (!i2c_check_ack()) { returnack = FALSE; }

    for ( index = 0 ; index < (rcnt-1) ; index++){
        TIMER__Wait_us(SW_I2C_WAIT_TIME);
        pdata[index] = SW_I2C_Read_Data();
        i2c_send_ack();
    }

    pdata[rcnt-1] = SW_I2C_Read_Data();

    i2c_check_not_ack();

    i2c_stop_condition();

    return returnack;
}

uint8_t SW_I2C_Check_SlaveAddr(uint8_t IICID)
{
    uint8_t   returnack = TRUE;

    i2c_start_condition();

    i2c_slave_address(IICID, WRITE_CMD);
    if (!i2c_check_ack())
    {
        returnack = FALSE;
    }

    return returnack;
}

uint8_t SW_I2C_UTIL_WRITE(uint8_t IICID, uint8_t regaddr, uint8_t data)
{
    return SW_I2C_WriteControl_8Bit(IICID<<1, regaddr, data);
}

uint8_t SW_I2C_UTIL_Read(uint8_t IICID, uint8_t regaddr)
{
    return SW_I2C_ReadControl_8Bit(IICID<<1, regaddr);
}

uint8_t SW_I2C_UTIL_Write_Multi(uint8_t IICID, uint8_t regaddr, uint8_t wcnt, uint8_t *pdata)
{
    return 0;//SW_I2C_Multi_WritenControl_8Bit(IICID<<1, regaddr, wcnt, pdata);
}

uint8_t SW_I2C_UTIL_Read_Multi(uint8_t IICID, uint8_t regaddr, uint8_t rcnt, uint8_t *pdata)
{
    return SW_I2C_Multi_ReadnControl_8Bit(IICID<<1, regaddr, rcnt, pdata);
}

#endif
