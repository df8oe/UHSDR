/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                               mcHF QRP Transceiver                              **
 **                             K Atanassov - M0NKA 2014                            **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:		CC BY-NC-SA 3.0                                                **
 ************************************************************************************/

// Common
#include "mchf_board.h"
#include "ui_lcd_hy28.h"
#include "mchf_hw_i2c.h"
#include "mchf_hw_i2c2.h"

#define I2C2_FLAG_TIMEOUT          		((uint32_t)0x500)
#define I2C2_LONG_TIMEOUT          		((uint32_t)(300 * I2C2_FLAG_TIMEOUT))

#define I2C2_SPEED             			25000

// I2C peripheral configuration defines (control interface of the si570)
#define CODEC_I2C                      	I2C2
#define I2C2_CLK                  	RCC_APB1Periph_I2C2
#define CODEC_I2C_GPIO_AF              	GPIO_AF_I2C2

__IO uint32_t  I2C2_Timeout = I2C2_LONG_TIMEOUT;

//*----------------------------------------------------------------------------
//* Function Name       : mchf_hw_i2c2_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_hw_i2c2_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // CODEC_I2C SCL and SDA pins configuration
    GPIO_InitStructure.GPIO_Pin   = I2C2_SCL_PIN|I2C2_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;

    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		// we also have ext pullups
    GPIO_Init(I2C2_SCL_GPIO, &GPIO_InitStructure);

    // Connect pins to I2C peripheral
    GPIO_PinAFConfig(I2C2_SCL_GPIO, I2C2_SCL_PINSRC, CODEC_I2C_GPIO_AF);
    GPIO_PinAFConfig(I2C2_SDA_GPIO, I2C2_SDA_PINSRC, CODEC_I2C_GPIO_AF);

    // Enable the CODEC_I2C peripheral clock
    RCC_APB1PeriphClockCmd(I2C2_CLK, ENABLE);

    // CODEC_I2C peripheral configuration
    I2C_DeInit(CODEC_I2C);
    I2C_InitStructure.I2C_Mode 				= I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle 			= I2C_DutyCycle_16_9;
    I2C_InitStructure.I2C_OwnAddress1 			= 0x33;
    I2C_InitStructure.I2C_Ack 				= I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress 		= I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed 			= I2C2_SPEED;

    // Enable the I2C peripheral
    I2C_Cmd (CODEC_I2C, ENABLE);
    I2C_Init(CODEC_I2C, &I2C_InitStructure);
}

#define I2C2_FlagStatusOrReturn(FLAG, RETURN) { \
		uint32_t timeout = I2C2_FLAG_TIMEOUT;\
		while(I2C_GetFlagStatus(CODEC_I2C, (FLAG)))\
		{ if ((timeout--) == 0) { return (RETURN); } } }

#define I2C2_EventCompleteOrReturn(EVENT, RETURN) { \
		uint32_t timeout = I2C2_LONG_TIMEOUT;\
		while(!I2C_CheckEvent(CODEC_I2C, (EVENT)))\
		{ if ((timeout--) == 0) { return (RETURN); } } }


//*----------------------------------------------------------------------------
//* Function Name       : mchf_hw_i2c2_reset
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/* void mchf_hw_i2c2_reset(void)
{
	I2C_SoftwareResetCmd(CODEC_I2C, ENABLE);
	non_os_delay();
	I2C_SoftwareResetCmd(CODEC_I2C, DISABLE);

	I2C_Cmd (CODEC_I2C, DISABLE);
	non_os_delay();
	I2C_Cmd (CODEC_I2C, ENABLE);

	// Init again
	mchf_hw_i2c2_init();
}
 */


//*----------------------------------------------------------------------------
//* Function Name       : mchf_hw_i2c2_WriteRegister
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uint16_t mchf_hw_i2c2_WriteRegister(uchar I2CAddr,uchar RegisterAddr, uchar RegisterValue)
{
    return MCHF_I2C_WriteRegister(CODEC_I2C, I2CAddr, &RegisterAddr, 1, RegisterValue);
}

// serial eeprom functions by DF8OE


static void EEPROM_24Cxx_AdjustAddrs(const uint8_t Mem_Type, uint8_t* devaddr_ptr, uint32_t* Addr_ptr)
{

    *devaddr_ptr = MEM_DEVICE_WRITE_ADDR;

    if(Mem_Type == 17 && *Addr_ptr > 0xFFFF)
    {
        *devaddr_ptr = *devaddr_ptr + 8;			// 24LC1025
        *Addr_ptr = *Addr_ptr - 0x10000;
    }
    if(Mem_Type == 18 && *Addr_ptr > 0xFFFF)
    {
        *devaddr_ptr = *devaddr_ptr + 2;			// 24LC1026
        *Addr_ptr = *Addr_ptr - 0x10000;
    }
    if(Mem_Type == 19)
    {
        if(*Addr_ptr > 0xFFFF && *Addr_ptr < 0x20000)
        {
            *devaddr_ptr = *devaddr_ptr + 4;			// 24CM02
            *Addr_ptr = *Addr_ptr - 0x10000;
        }
        if(*Addr_ptr > 0x1FFFF && *Addr_ptr < 0x30000)
        {
            *devaddr_ptr = *devaddr_ptr + 2;			// 24CM02
            *Addr_ptr = *Addr_ptr - 0x20000;
        }
        if(*Addr_ptr > 0x2FFFF && *Addr_ptr < 0x40000)
        {
            *devaddr_ptr = *devaddr_ptr + 3;			// 24CM02
            *Addr_ptr = *Addr_ptr - 0x30000;
        }
    }
}


static uint16_t EEPROM_24Cxx_ackPollingSinglePoll(uint32_t Addr, uint8_t Mem_Type)
{
    uint8_t devaddr;

    EEPROM_24Cxx_AdjustAddrs(Mem_Type,&devaddr,&Addr);

    I2C_GenerateSTART(CODEC_I2C, ENABLE);
    I2C2_EventCompleteOrReturn(I2C_EVENT_MASTER_MODE_SELECT, 0xFF00);
    // Test on I2C2 EV5, Start transmitted successfully and clear it
    // Send Memory device slave Address for write
    I2C_Send7bitAddress(CODEC_I2C, devaddr, I2C_Direction_Transmitter);
    // Test on I2C2 EV6 and clear it
    I2C2_EventCompleteOrReturn(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 0xFD00)

    return 0;
}
static uint16_t EEPROM_24Cxx_ackPolling(uint32_t Addr, uint8_t Mem_Type)
{
    int i = 10;
    uint16_t retVal;

    for (; i; i--)
    {
        if ((retVal = EEPROM_24Cxx_ackPollingSinglePoll(Addr, Mem_Type)) != 0xFD00)
        {
            break;
        }
    }
    return retVal;
}
struct _EEPROM_24CXX_Descriptor
{
    uint8_t devaddr;
    uint8_t addr[2]; // 0 -> upper or single, 1 -> lower, second;
    uint16_t addr_size;
};
typedef struct _EEPROM_24CXX_Descriptor EEPROM_24CXX_Descriptor;

EEPROM_24CXX_Descriptor eeprom_desc;
// THIS CAN BE USED ONLY WITH SINGLE EEPROM AND SINGLE THREAD
// NOT THREAD SAFE, USE local variable instead then


static void EEPROM_24Cxx_StartTransfer_Prep(uint32_t Addr, uint8_t Mem_Type, EEPROM_24CXX_Descriptor* eeprom_desc_ptr)
{
    EEPROM_24Cxx_AdjustAddrs(Mem_Type,&eeprom_desc_ptr->devaddr,&Addr);

    if (Mem_Type > 8)
    {
        eeprom_desc_ptr->addr[1] = (uint8_t)((0x00FF)&(Addr));
        eeprom_desc_ptr->addr[0] = (uint8_t)((0x00FF)&((Addr)>>8));
        eeprom_desc_ptr->addr_size = 2;
    }
    else
    {
        eeprom_desc_ptr->addr[0] = (uint8_t)((0x00FF)&Addr);
        eeprom_desc_ptr->addr_size = 1;
    }
}

#if 0
static uint16_t EEPROM_24Cxx_StartTransfer(uint32_t Addr, uint8_t Mem_Type, bool isWrite)
{
    EEPROM_24Cxx_StartTransfer_Prep(Addr, Mem_Type,&eeprom_desc);
    return MCHF_I2C_StartTransfer(CODEC_I2C,eeprom_desc.devaddr,eeprom_desc.addr,eeprom_desc.addr_size,isWrite,true);
}
#endif

uint16_t Write_24Cxx(uint32_t Addr, uint8_t Data, uint8_t Mem_Type)
{
    EEPROM_24Cxx_StartTransfer_Prep(Addr, Mem_Type,&eeprom_desc);
    uint16_t retVal = MCHF_I2C_WriteRegister(CODEC_I2C,eeprom_desc.devaddr,&eeprom_desc.addr[0],eeprom_desc.addr_size,Data);
#if 0
    uint16_t retVal = EEPROM_24Cxx_StartTransfer(Addr,Mem_Type,true);
    if (!retVal)
    {
        // Send Data
        I2C_SendData(CODEC_I2C, Data);
        // Test on I2C2 EV8 and clear it
        I2C2_EventCompleteOrReturn(I2C_EVENT_MASTER_BYTE_TRANSMITTED, 0xFC00)
        // Send I2C2 STOP Condition
        I2C_GenerateSTOP(CODEC_I2C, ENABLE);
        I2C2_FlagStatusOrReturn(I2C_FLAG_STOPF, 0xF000)
    }
#endif

    if (!retVal)
    {
        retVal = EEPROM_24Cxx_ackPolling(Addr,Mem_Type);
    }

    return retVal;
}


uint16_t Read_24Cxx(uint32_t Addr, uint8_t Mem_Type)
{
    uint8_t value;
    EEPROM_24Cxx_StartTransfer_Prep(Addr, Mem_Type,&eeprom_desc);
    uint16_t retVal = MCHF_I2C_ReadRegister(CODEC_I2C,eeprom_desc.devaddr,&eeprom_desc.addr[0],eeprom_desc.addr_size,&value);
    if (!retVal)
    {
        retVal = value;
    }
    return retVal;
}

uint16_t Read_24Cxxseq(uint32_t start, uint8_t *buffer, uint16_t length, uint8_t Mem_Type)
{
    return(0xFFFF);
}

uint16_t Write_24Cxxseq(uint32_t Addr, uint8_t *buffer, uint16_t length, uint8_t Mem_Type)
{
    uint32_t page, count;
    uint16_t retVal = 0xFFFF;
    count = 0;

    if(Mem_Type == 12)
        page = 64;
    if(Mem_Type == 13)
        page = 32;
    if(Mem_Type == 14 || Mem_Type == 15)
        page = 64;
    if(Mem_Type > 15 && Mem_Type < 19)
        page = 128;
    if(Mem_Type == 19)
        page = 256;


    while(count < length)
    {
        EEPROM_24Cxx_StartTransfer_Prep(Addr + count, Mem_Type,&eeprom_desc);
        retVal = MCHF_I2C_WriteBlock(CODEC_I2C,eeprom_desc.devaddr,&eeprom_desc.addr[0],eeprom_desc.addr_size,&buffer[count],page);
        count+=page;
        if (retVal)
        {
            break;
        }
        retVal = EEPROM_24Cxx_ackPolling(Addr,Mem_Type);
    }
    return retVal;
}
