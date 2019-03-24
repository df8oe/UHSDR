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
#include "uhsdr_board.h"
#include "uhsdr_hw_i2c.h"
#include "i2c.h"

uint16_t UhsdrHw_I2C_DeviceReady(I2C_HandleTypeDef* hi2c, uchar I2CAddr)
{
	return HAL_I2C_IsDeviceReady(hi2c, I2CAddr,100,100);
}

uint16_t UhsdrHw_I2C_WriteRegister(I2C_HandleTypeDef* hi2c, uchar I2CAddr,uint16_t addr,uint16_t addr_size, uchar RegisterValue)
{
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Write(hi2c,I2CAddr,addr,addr_size,&RegisterValue,1,100);

    return  i2cRet != HAL_OK?0xFF00:0;
}

uint16_t UhsdrHw_I2C_WriteBlock(I2C_HandleTypeDef* hi2c, uchar I2CAddr, uint16_t addr, uint16_t addr_size, const uint8_t* data, uint32_t size)
{
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Write(hi2c,I2CAddr,addr,addr_size,(uint8_t*)data,size,100);

    return  i2cRet != HAL_OK?0xFF00:0;
}

uint16_t UhsdrHw_I2C_ReadRegister(I2C_HandleTypeDef* hi2c, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t *RegisterValue)
{
    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Read(hi2c,I2CAddr,addr,addr_size,RegisterValue,1,100);

    return  i2cRet != HAL_OK?0xFF00:0;
}

uint16_t UhsdrHw_I2C_ReadBlock(I2C_HandleTypeDef* hi2c, uchar I2CAddr,uint16_t addr, uint16_t addr_size, uint8_t *data, uint32_t size)
{

    HAL_StatusTypeDef i2cRet = HAL_I2C_Mem_Read(hi2c,I2CAddr,addr,addr_size,data,size,100);

    return  i2cRet != HAL_OK?0xFF00:0;
}
#ifdef STM32F4
/**
 * @brief init I2C
 * @param speed in Hertz !!!
 */
void UhsdrHw_I2C_ChangeSpeed(I2C_HandleTypeDef* hi2c)
{

    /// FIXME: Support for more than I2C1 und I2C2
    uint8_t speedIdx;

    if (hi2c->Instance == I2C1)
    {
        speedIdx = I2C_BUS_1;
    }
    else
    {
        speedIdx = I2C_BUS_2;
    }


    HAL_I2C_DeInit(hi2c);
    // FIXME: F7PORT: I2C Clock Timing works differently on the F7, we need to supply correct register values instead of a simple speed value
    hi2c->Init.ClockSpeed = ts.i2c_speed[speedIdx] * I2C_BUS_SPEED_MULT;

    HAL_I2C_Init(hi2c);

}
#endif



