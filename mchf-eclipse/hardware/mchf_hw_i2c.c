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

uint16_t MCHF_I2C_WriteRegister(I2C_TypeDef* bus, uchar I2CAddr,uint16_t addr,uint16_t addr_size, uchar RegisterValue)
{
    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Write(hi2c,I2CAddr,addr,addr_size,&RegisterValue,1,100);

    return  i2cRet != HAL_OK?0xFF00:0;
}

uint16_t MCHF_I2C_WriteBlock(I2C_TypeDef* bus, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t* data, uint32_t size)
{
    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Write(hi2c,I2CAddr,addr,addr_size,data,size,100);

    return  i2cRet != HAL_OK?0xFF00:0;
}

uint16_t MCHF_I2C_ReadRegister(I2C_TypeDef* bus, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t *RegisterValue)
{
    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Read(hi2c,I2CAddr,addr,addr_size,RegisterValue,1,100);

    return  i2cRet != HAL_OK?0xFF00:0;
}

uint16_t MCHF_I2C_ReadBlock(I2C_TypeDef* bus, uchar I2CAddr,uint16_t addr, uint16_t addr_size, uint8_t *data, uint32_t size)
{

    I2C_HandleTypeDef* hi2c = mchf_hw_I2c_Bus2Handle(bus);
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Read(hi2c,I2CAddr,addr,addr_size,data,size,100);

    return  i2cRet != HAL_OK?0xFF00:0;
}

/**
 * @brief init I2C
 * @param speed in Hertz !!!
 */
void MchfHw_I2C_ChangeSpeed(I2C_TypeDef* bus)
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
#ifndef STM32F7
    // FIXME: F7PORT: I2C Clock Timing works differently on the F7, we need to supply correct register values instead of a simple speed value
    hi2c->Init.ClockSpeed = ts.i2c_speed[speedIdx] * I2C_BUS_SPEED_MULT;
#endif
    HAL_I2C_Init(hi2c);

}

void MchfHw_I2C_Reset(I2C_TypeDef* bus)
{
    MchfHw_I2C_ChangeSpeed(bus);
};

/*
 * @brief full init of I2C1 including GPIO and bus clock
 */
void mchf_hw_i2c1_init()
{
    MchfHw_I2C_ChangeSpeed(I2C1);
}

/*
 * @brief full init of I2C2 including GPIO and bus clock
 */
void mchf_hw_i2c2_init()
{
    MchfHw_I2C_ChangeSpeed(I2C2);
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
    return MCHF_I2C_WriteRegister(I2C1, I2CAddr, RegisterAddr, 1, RegisterValue);
}

uint16_t mchf_hw_i2c1_WriteBlock(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size)
{
    return MCHF_I2C_WriteBlock(I2C1, I2CAddr,RegisterAddr, 1, data, size);
}

uint16_t mchf_hw_i2c1_ReadRegister(uchar I2CAddr,uchar RegisterAddr, uchar *RegisterValue)
{
    return MCHF_I2C_ReadRegister(I2C1, I2CAddr,RegisterAddr, 1, RegisterValue);
}

uint16_t mchf_hw_i2c1_ReadData(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size)
{
    return  MCHF_I2C_ReadBlock(I2C1, I2CAddr, RegisterAddr, 1, data, size);
}
