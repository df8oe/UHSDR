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

#include "mchf_hw_i2c.h"

#define I2C1_FLAG_TIMEOUT          		((uint32_t)0x500)
#define I2C1_LONG_TIMEOUT          		((uint32_t)(300 * I2C1_FLAG_TIMEOUT))

//#define I2C1_SPEED             		400000
#define I2C1_SPEED             			100000
//#define I2C1_SPEED           			25000

// I2C peripheral configuration defines (control interface of the si570)
#define SI570_I2C                      	I2C1
#define I2C1_CLK                  		RCC_APB1Periph_I2C1
#define SI570_I2C_GPIO_AF              	GPIO_AF_I2C1

__IO uint32_t  I2C1_Timeout = I2C1_LONG_TIMEOUT;


#define I2C_FlagStatusOrReturn(BUS, FLAG, RETURN) { \
		uint32_t timeout = I2C1_FLAG_TIMEOUT;\
		while(I2C_GetFlagStatus((BUS), (FLAG)))\
		{ if ((timeout--) == 0) { return (RETURN); } } }

#define I2C_EventCompleteOrReturn(BUS,EVENT, RETURN) { \
		uint32_t timeout = I2C1_LONG_TIMEOUT;\
		while(!I2C_CheckEvent((BUS), (EVENT)))\
		{ if ((timeout--) == 0) { return (RETURN); } } }


uint16_t MCHF_I2C_StartTransfer(I2C_TypeDef* bus, uint8_t devaddr, uint8_t* data_ptr, uint16_t data_size, bool isWrite, bool isSingleByteRead)
{
    int data_idx;

    // Generate the Start Condition
    I2C_GenerateSTART(bus, ENABLE);
    I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_MODE_SELECT, 0xFF00);
    // Test on I2C2 EV5, Start transmitted successfully and clear it
    // Send Memory device slave Address for write
    I2C_Send7bitAddress(bus, devaddr, I2C_Direction_Transmitter);
    // Test on I2C2 EV6 and clear it
    I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 0xFD00);

    for(data_idx = 0; data_idx < data_size; data_idx++)
    {
        I2C_SendData(bus, data_ptr[data_idx]);
        // Test on I2C2 EV8 and clear it
        I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_BYTE_TRANSMITTED, 0xFC00);
        // After transmission of addr, we may have to switch to read mode
    }
    if (isWrite == false)
    {
        I2C_GenerateSTART(bus, ENABLE);

        I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_MODE_SELECT, 0xFB00);
        // Test on I2C2 EV5, Start transmitted successfully and clear it
        // Send Memory device slave Address for read
        I2C_Send7bitAddress(bus, devaddr+1, I2C_Direction_Receiver);
        if  (isSingleByteRead)
        {
            I2C_AcknowledgeConfig(bus, DISABLE);
        }
        else
        {
            I2C_AcknowledgeConfig(bus, ENABLE);
        }
        I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, 0xFA00);
    }
    return 0;
}

uint16_t MCHF_I2C_WriteRegister(I2C_TypeDef* bus, uchar I2CAddr,uint8_t* addr_ptr,uint16_t addr_size, uchar RegisterValue)
{
    uint16_t retVal = MCHF_I2C_StartTransfer(bus,I2CAddr,addr_ptr,addr_size,true,false);
    // is write only, is not a single byte READ

    if (!retVal)
    {
        // Prepare the register value to be sent
        I2C_SendData(bus, RegisterValue);

        I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_BYTE_TRANSMITTED, 0xFC00);
        // End the configuration sequence
        I2C_GenerateSTOP(bus, ENABLE);
        I2C_FlagStatusOrReturn(bus,I2C_FLAG_STOPF, 0xF000);
    }
    return retVal;
}

uint16_t MCHF_I2C_WriteBlock(I2C_TypeDef* bus, uchar I2CAddr, uint8_t* addr_ptr, uint16_t addr_size, uint8_t* data, uint32_t size)
{
    ulong i;

    //printf("i2c write 0x%02x 0x%02x 0x%02x\n\r",I2CAddr,RegisterAddr,RegisterValue);
    uint16_t retVal = MCHF_I2C_StartTransfer(bus,I2CAddr,addr_ptr,addr_size,true,false);
    // is write only, is not a single byte READ

    if (!retVal)
    {
        for(i = 0; i < size; i++)
        {
            // Prepare the register value to be sent
            I2C_SendData(bus, data[i]);
            I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_BYTE_TRANSMITTED, 0xFC00);
        }
        // End the configuration sequence
        I2C_GenerateSTOP(bus, ENABLE);
        I2C_FlagStatusOrReturn(bus,I2C_FLAG_STOPF, 0xF000);
    }
    return retVal;
}

uint16_t MCHF_I2C_ReadRegister(I2C_TypeDef* bus, uchar I2CAddr, uint8_t* addr_ptr, uint16_t addr_size, uint8_t *RegisterValue)
{
    //printf("i2c read\n\r");
    uint16_t retVal = MCHF_I2C_StartTransfer(bus,I2CAddr,addr_ptr,addr_size,false,true);
    // is read, IS a SINGLE byte READ

    if (!retVal)
    {
        I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_BYTE_RECEIVED, 0xF900);
        /* Prepare Stop after receiving data */
        I2C_GenerateSTOP(bus, ENABLE);
        I2C_FlagStatusOrReturn(bus,I2C_FLAG_STOPF, 0xF000);
        /* Receive the Data */
        *RegisterValue = I2C_ReceiveData(bus);
    }
    return retVal;
}

uint16_t MCHF_I2C_ReadBlock(I2C_TypeDef* bus, uchar I2CAddr,uint8_t* addr_ptr, uint16_t addr_size, uint8_t *data, uint32_t size)
{
    if (size == 0) return 0xFFFF; // invalid size

    uint16_t retVal = MCHF_I2C_StartTransfer(bus,I2CAddr,addr_ptr,addr_size,false,size == 1);
    // is read, is a not single byte READ unless size == 1

    if (!retVal)
    {
        int idx;
        for (idx = 0; idx < size - 1; idx++)
        {
            I2C_EventCompleteOrReturn(bus, I2C_EVENT_MASTER_BYTE_RECEIVED, 0xF900);
            data[idx] = I2C_ReceiveData(bus);
        }
        // we have just read the second last byte, now prepare for landing :-)
        if (size > 1)
        {
            I2C_AcknowledgeConfig(bus, DISABLE);
        }
        // in case of a single byte read, this has been done already in the start transfer
        I2C_EventCompleteOrReturn(bus, I2C_EVENT_MASTER_BYTE_RECEIVED, 0xF900);
        I2C_GenerateSTOP(bus, ENABLE);
        I2C_FlagStatusOrReturn(bus, I2C_FLAG_STOPF, 0xF000);
        data[size-1] = I2C_ReceiveData(bus);
    }

    //printf("read done\n\r");
    return retVal;
}





//*----------------------------------------------------------------------------
//* Function Name       :
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_hw_i2c_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // CODEC_I2C SCL and SDA pins configuration
    GPIO_InitStructure.GPIO_Pin = I2C1_SCL_PIN|I2C1_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;

    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;		// - strong ringing on the bus
    //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN; 	// - makes it better
    //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;	// - same as pull down

    GPIO_Init(I2C1_SCL_GPIO, &GPIO_InitStructure);

    // Connect pins to I2C peripheral
    GPIO_PinAFConfig(I2C1_SCL_GPIO, I2C1_SCL_PINSRC, SI570_I2C_GPIO_AF);
    GPIO_PinAFConfig(I2C1_SDA_GPIO, I2C1_SDA_PINSRC, SI570_I2C_GPIO_AF);

    // Enable the CODEC_I2C peripheral clock
    RCC_APB1PeriphClockCmd(I2C1_CLK, ENABLE);

    // CODEC_I2C peripheral configuration
    I2C_DeInit(SI570_I2C);
    I2C_InitStructure.I2C_Mode 					= I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle 			= I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 			= 0x33;
    I2C_InitStructure.I2C_Ack 					= I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress 	= I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed 			= I2C1_SPEED;

    // Enable the I2C peripheral
    I2C_Cmd (SI570_I2C, ENABLE);
    I2C_Init(SI570_I2C, &I2C_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : trx4m_i2c_reset
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_hw_i2c_reset(void)
{
    //printf("===========================\n\r");
    //printf("i2c reset bus\n\r");
    //printf("===========================\n\r");

    I2C_SoftwareResetCmd(SI570_I2C, ENABLE);
    non_os_delay();
    I2C_SoftwareResetCmd(SI570_I2C, DISABLE);

    I2C_Cmd (SI570_I2C, DISABLE);
    non_os_delay();
    I2C_Cmd (SI570_I2C, ENABLE);

    // Init again
    mchf_hw_i2c_init();
}



uint16_t mchf_hw_i2c_WriteRegister(uchar I2CAddr, uchar RegisterAddr, uchar RegisterValue)
{
    return MCHF_I2C_WriteRegister(SI570_I2C, I2CAddr, &RegisterAddr, 1, RegisterValue);
}

uint16_t mchf_hw_i2c_WriteBlock(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size)
{
    return MCHF_I2C_WriteBlock(SI570_I2C, I2CAddr,&RegisterAddr, 1, data, size);
}

uint16_t mchf_hw_i2c_ReadRegister(uchar I2CAddr,uchar RegisterAddr, uchar *RegisterValue)
{
    return MCHF_I2C_ReadRegister(SI570_I2C, I2CAddr,&RegisterAddr, 1, RegisterValue);
}

uint16_t mchf_hw_i2c_ReadData(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size)
{
    return MCHF_I2C_ReadBlock(SI570_I2C, I2CAddr, &RegisterAddr, 1, data, size);
}

