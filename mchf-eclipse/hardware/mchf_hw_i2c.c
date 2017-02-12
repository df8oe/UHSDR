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
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

// Common
#include "mchf_board.h"
#include "mchf_hw_i2c.h"
#include "i2c.h"


// I2C peripheral configuration defines (control interface of the si570)
//#define I2C2_CLK                    RCC_APB1Periph_I2C2
#define I2C2_GPIO_AF                GPIO_AF_I2C2


// I2C peripheral configuration defines (control interface of the si570)
//w#define I2C1_CLK                  		RCC_APB1Periph_I2C1

// uint32_t i2c_flag_timeout_max = MCHF_I2C_FLAG_TIMEOUT; // lower values, higher timeout!
uint32_t i2c_event_timeout_max = MCHF_I2C_LONG_TIMEOUT; // lower values, higher timeout!

static inline I2C_HandleTypeDef* mchf_hw_I2c_Bus2Handle(I2C_TypeDef* bus) {
    I2C_HandleTypeDef* hi2c;
    if (bus == I2C1)
    {
        hi2c = &hi2c1;
    }
    else
    {
        hi2c = &hi2c2;
    }
    return hi2c;
}

#if 0
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
#endif

uint16_t MCHF_I2C_WriteRegister(I2C_TypeDef* bus, uchar I2CAddr,uint16_t addr,uint16_t addr_size, uchar RegisterValue)
{
    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Write(hi2c,I2CAddr,addr,addr_size,&RegisterValue,1,100);

    uint16_t retval =  i2cRet != HAL_OK?0xFF00:0;

#if 0
    retval = MCHF_I2C_StartTransfer(bus,I2CAddr,addr_ptr,addr_size,true,false);
    // is write only, is not a single byte READ

    if (!retval)
    {
        // Prepare the register value to be sent
        I2C_SendData(bus, RegisterValue);

        I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_BYTE_TRANSMITTED, 0xFC00);
        // End the configuration sequence
        I2C_GenerateSTOP(bus, ENABLE);
        I2C_FlagStatusOrReturn(bus,I2C_FLAG_STOPF, 0xF000);
    }
#endif
    return retval;
}

uint16_t MCHF_I2C_WriteBlock(I2C_TypeDef* bus, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t* data, uint32_t size)
{
    //printf("i2c write 0x%02x 0x%02x 0x%02x\n\r",I2CAddr,RegisterAddr,RegisterValue);
    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Write(hi2c,I2CAddr,addr,addr_size,data,size,100);

    uint16_t retval =  i2cRet != HAL_OK?0xFF00:0;

#if 0
    retval = MCHF_I2C_StartTransfer(bus,I2CAddr,addr_ptr,addr_size,true,false);
    // is write only, is not a single byte READ

    if (!retval)
    {
        for(uint32_t i = 0; i < size; i++)
        {
            // Prepare the register value to be sent
            I2C_SendData(bus, data[i]);
            I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_BYTE_TRANSMITTED, 0xFC00);
        }
        // End the configuration sequence
        I2C_GenerateSTOP(bus, ENABLE);
        I2C_FlagStatusOrReturn(bus,I2C_FLAG_STOPF, 0xF000);
    }
#endif
    return retval;
}

uint16_t MCHF_I2C_ReadRegister(I2C_TypeDef* bus, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t *RegisterValue)
{
    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Read(hi2c,I2CAddr,addr,addr_size,RegisterValue,1,100);

    uint16_t retval =  i2cRet != HAL_OK?0xFF00:0;
#if 0
    //printf("i2c read\n\r");
    uint16_t retval = MCHF_I2C_StartTransfer(bus,I2CAddr,addr_ptr,addr_size,false,true);
    // is read, IS a SINGLE byte READ

    if (!retval)
    {
        I2C_EventCompleteOrReturn(bus,I2C_EVENT_MASTER_BYTE_RECEIVED, 0xF900);
        /* Prepare Stop after receiving data */
        I2C_GenerateSTOP(bus, ENABLE);
        I2C_FlagStatusOrReturn(bus,I2C_FLAG_STOPF, 0xF000);
        /* Receive the Data */
        *RegisterValue = I2C_ReceiveData(bus);
    }
#endif
    return retval;
}

uint16_t MCHF_I2C_ReadBlock(I2C_TypeDef* bus, uchar I2CAddr,uint16_t addr, uint16_t addr_size, uint8_t *data, uint32_t size)
{

    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Read(hi2c,I2CAddr,addr,addr_size,data,size,100);

    uint16_t retval =  i2cRet != HAL_OK?0xFF00:0;
#if 0
    if (size != 0)
    {
        retval = MCHF_I2C_StartTransfer(bus,I2CAddr,addr_ptr,addr_size,false,size == 1);
        // is read, is a not single byte READ unless size == 1

        if (!retval)
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
    }
    //printf("read done\n\r");
#endif
    return retval;
}

/**
 * @brief init I2C
 * @param speed in Hertz !!!
 */
void MchfHw_I2C_Init(I2C_TypeDef* bus)
{

    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    uint8_t speedIdx;

    if (bus == I2C1)
    {
        speedIdx = I2C_BUS_1;
    }
    else
    {
        speedIdx = I2C_BUS_2;
    }


    HAL_I2C_DeInit(hi2c);
    hi2c->Init.ClockSpeed = ts.i2c_speed[speedIdx] * I2C_BUS_SPEED_MULT;
    HAL_I2C_Init(hi2c);

}

void MchfHw_I2C_Reset(I2C_TypeDef* bus)
{
#if 0
    I2C_SoftwareResetCmd(bus, ENABLE);
    I2C_SoftwareResetCmd(bus, DISABLE);

    I2C_Cmd (bus, DISABLE);
    non_os_delay();
    I2C_Cmd (bus, ENABLE);
#endif
    MchfHw_I2C_Init(bus);
};

void MchfHw_I2C_GpioInit(I2C_TypeDef* bus)
{
    // Handled By HAL
#if 0
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;       // - strong ringing on the bus
    //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;   // - makes it better
    //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; // - same as pull down


    // Connect pins to I2C peripheral

    if (bus == I2C1)
    {
        GPIO_InitStructure.Alternate = GPIO_AF4_I2C1;

        GPIO_InitStructure.Pin = I2C1_SCL_PIN;
        HAL_GPIO_Init(I2C1_SCL_GPIO, &GPIO_InitStructure);

        GPIO_InitStructure.Pin = I2C1_SDA_PIN;
        HAL_GPIO_Init(I2C1_SDA_GPIO, &GPIO_InitStructure);
    }
    else if (bus == I2C2)
    {
        GPIO_InitStructure.Alternate = GPIO_AF4_I2C2;

        GPIO_InitStructure.Pin = I2C2_SCL_PIN;
        HAL_GPIO_Init(I2C2_SCL_GPIO, &GPIO_InitStructure);

        GPIO_InitStructure.Pin = I2C2_SDA_PIN;
        HAL_GPIO_Init(I2C2_SDA_GPIO, &GPIO_InitStructure);
    }
#endif
}

/*
 * @brief full init of I2C1 including GPIO and bus clock
 */
void mchf_hw_i2c1_init()
{
    // I2C SCL and SDA pins configuration
    ///MchfHw_I2C_GpioInit(I2C1);

    // Enable the I2C peripheral clock
    ///RCC_APB1PeriphClockCmd(I2C1_CLK, ENABLE);

    // Enabled I2S peripheral
    ///MchfHw_I2C_Init(I2C1);
}

/*
 * @brief full init of I2C2 including GPIO and bus clock
 */
void mchf_hw_i2c2_init()
{
    ///MchfHw_I2C_GpioInit(I2C2);

    // Enable the I2C2 peripheral clock
    ///RCC_APB1PeriphClockCmd(I2C2_CLK, ENABLE);

    ///MchfHw_I2C_Init(I2C2);
}

void mchf_hw_i2c1_reset(void)
{
    MchfHw_I2C_Reset(I2C1);
}

void mchf_hw_i2c2_reset(void)
{
    MchfHw_I2C_Reset(I2C2);
}

uint16_t mchf_hw_i2c1_WriteRegister(uchar I2CAddr, uchar RegisterAddr, uchar RegisterValue)
{
    uint16_t res = MCHF_I2C_WriteRegister(I2C1, I2CAddr, RegisterAddr, 1, RegisterValue);
#ifdef DEBUG_I2C1_ISSUES
    while (res) { asm ("nop"); }
#endif
    return res;
}

uint16_t mchf_hw_i2c1_WriteBlock(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size)
{
    uint16_t res = MCHF_I2C_WriteBlock(I2C1, I2CAddr,RegisterAddr, 1, data, size);
#ifdef DEBUG_I2C1_ISSUES
    while (res) { asm ("nop"); }
#endif
    return res;
}

uint16_t mchf_hw_i2c1_ReadRegister(uchar I2CAddr,uchar RegisterAddr, uchar *RegisterValue)
{
    uint16_t res = MCHF_I2C_ReadRegister(I2C1, I2CAddr,RegisterAddr, 1, RegisterValue);
#ifdef DEBUG_I2C1_ISSUES
    while (res) { asm ("nop"); }
#endif
    return res;

}

uint16_t mchf_hw_i2c1_ReadData(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size)
{
    uint16_t res =  MCHF_I2C_ReadBlock(I2C1, I2CAddr, RegisterAddr, 1, data, size);
#ifdef DEBUG_I2C1_ISSUES
    while (res) { asm ("nop"); }
#endif
    return res;
}
