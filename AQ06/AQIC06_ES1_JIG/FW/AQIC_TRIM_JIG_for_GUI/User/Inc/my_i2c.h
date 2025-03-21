#ifndef __MYIIC_H
#define __MYIIC_H

#include "main.h"

#if 0
//#define LOW     (0)
//#define HIGH    (1)

// SCL : APIC_CS_GPIO_Port
// SDA : APIC_GATE_GPIO_Port
#define  SW_I2C_WAIT_TIME  22	
#define  I2C_READ       0x01


//IO Direction setting 
#define SDA_IN()        { APIC_GATE_GPIO_Port->MODER &= ~(3U<<(APIC_GATE_Pin*2U)); APIC_GATE_GPIO_Port->MODER |= 0U<<(APIC_GATE_Pin*2U); }
#define SDA_OUT()       { APIC_GATE_GPIO_Port->MODER &= ~(3U<<(APIC_GATE_Pin*2U)); APIC_GATE_GPIO_Port->MODER |= 1U<<(APIC_GATE_Pin*2U); }

//IO Operation function 	 
#define IIC_SCL_LOW()   { APIC_CS_GPIO_Port->BSRR = (uint32_t)APIC_CS_Pin << 16U; }
#define IIC_SCL_HIGH()  { APIC_CS_GPIO_Port->BSRR = APIC_CS_Pin; }
#define IIC_SDA_LOW()   { APIC_GATE_GPIO_Port->BSRR = (uint32_t)APIC_GATE_Pin << 16U; }
#define IIC_SDA_HIGH()  { APIC_GATE_GPIO_Port->BSRR = APIC_GATE_Pin; }

#define READ_SDA        HAL_GPIO_ReadPin(APIC_GATE_GPIO_Port, APIC_GATE_Pin)

//IIC All operation functions 
#if 0
void MY_IIC_Init(void);                // initialization IIC Of IO mouth		 
void MY_IIC_Start(void);				// send out IIC Start signal
void MY_IIC_Stop(void);	  			// send out IIC Stop signal
void MY_IIC_Send_Byte(uint8_t txd);			//IIC Send a byte
uint8_t MY_IIC_Read_Byte(uint8_t ack);//IIC Read a byte
uint8_t MY_IIC_Wait_Ack(void); 				//IIC wait for ACK The signal
void MY_IIC_Ack(void);					//IIC send out ACK The signal
void MY_IIC_NAck(void);				//IIC Do not send ACK The signal
#endif
uint8_t MY_IIC_Write_One_Byte(uint8_t daddr, uint8_t addr, uint8_t data);
uint8_t MY_IIC_Read_One_Byte(uint8_t daddr, uint8_t addr);

#else

/* functions */
void SW_I2C_initial(void);

void i2c_port_initial(void);	

uint8_t SW_I2C_ReadVal_SDA(void);

void SW_I2C_Write_Data(uint8_t data);
uint8_t SW_I2C_Read_Data(void);

uint8_t SW_I2C_WriteControl_8Bit(uint8_t IICID, uint8_t regaddr, uint8_t data);
uint8_t SW_I2C_WriteControl_8Bit_OnlyRegAddr(uint8_t IICID, uint8_t regaddr);
uint8_t SW_I2C_WriteControl_16Bit(uint8_t IICID, uint8_t regaddr, uint16_t data);

uint8_t SW_I2C_ReadControl_8Bit_OnlyRegAddr(uint8_t IICID, uint8_t regaddr);
uint8_t SW_I2C_ReadControl_8Bit_OnlyData(uint8_t IICID);
uint16_t SW_I2C_ReadControl_16Bit_OnlyData(uint8_t IICID);
uint8_t SW_I2C_ReadControl_8Bit(uint8_t IICID, uint8_t regaddr);
uint16_t SW_I2C_ReadControl_16Bit(uint8_t IICID, uint8_t regaddr);

uint8_t SW_I2C_ReadnControl_8Bit(uint8_t IICID, uint8_t regaddr, uint8_t rcnt, uint8_t (*pdata));
uint8_t SW_I2C_Multi_ReadnControl_8Bit(uint8_t IICID, uint8_t regaddr, uint8_t rcnt, uint8_t (*pdata));
uint8_t SW_I2C_Check_SlaveAddr(uint8_t IICID);

uint8_t SW_I2C_UTIL_WRITE(uint8_t IICID, uint8_t regaddr, uint8_t data);
uint8_t SW_I2C_UTIL_Read(uint8_t IICID, uint8_t regaddr);
uint8_t SW_I2C_UTIL_Write_Multi(uint8_t IICID, uint8_t regaddr, uint8_t wcnt, uint8_t *pdata);
uint8_t SW_I2C_UTIL_Read_Multi(uint8_t IICID, uint8_t regaddr, uint8_t rcnt, uint8_t *pdata);

#endif

#endif

