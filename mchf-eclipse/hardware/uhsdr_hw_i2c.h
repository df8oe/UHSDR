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

#ifndef __MCHF_HW_I2C_H
#define __MCHF_HW_I2C_H

#include "i2c.h"

#define I2C_BUS_SPEED_MULT                      20000 // 20kHz is multiplier for config var

#define I2C1_SPEED_DEFAULT                      (100000/I2C_BUS_SPEED_MULT) // 20 * 20kHz == 200kHz 400000 does not work, may need resistor mod (now reduced to 100KHz)
#define I2C2_SPEED_DEFAULT                      (100000/I2C_BUS_SPEED_MULT) // 10 * 20kHz == 100kHz, was 25000, 400000 does not work, , may need resistor mod

// Generic Functions for Handling I2C Busses in MCHF

uint16_t UhsdrHw_I2C_WriteRegister(I2C_HandleTypeDef* i2c, uchar I2CAddr,uint16_t addr,uint16_t addr_size, uchar RegisterValue);
uint16_t UhsdrHw_I2C_WriteBlock(I2C_HandleTypeDef* i2c, uchar I2CAddr, uint16_t addr, uint16_t addr_size, const uint8_t* data, uint32_t size);
uint16_t UhsdrHw_I2C_ReadRegister(I2C_HandleTypeDef* i2c, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t *RegisterValue);
uint16_t UhsdrHw_I2C_ReadBlock(I2C_HandleTypeDef* i2c, uchar I2CAddr,uint16_t addr, uint16_t addr_size, uint8_t *data, uint32_t size);

uint16_t UhsdrHw_I2C_DeviceReady(I2C_HandleTypeDef* hi2c, uchar I2CAddr);

#ifdef STM32F4
// Special init and wrapper functions for I2C Bus 1
void UhsdrHw_I2C_ChangeSpeed(I2C_HandleTypeDef* hi2c);
#endif

// Flags do not really need a timeout according to my testing.
#define MCHF_I2C_FLAG_TIMEOUT                ((uint32_t)1)
// Event timeout according to testing is depending on the actual speed of the bus
// 200 kHz Bus -> 100 would be sufficient
// 100 kHz Bus -> 200, 50kHz -> 400, 25kHz -> 800
// other place I saw 20000 used as timeout
#define MCHF_I2C_LONG_TIMEOUT                ((uint32_t)1000)


// extern uint32_t i2c_flag_timeout_max; // lower values, higher timeout!
extern uint32_t i2c_event_timeout_max; // lower values, higher timeout!

#define I2C_FlagStatusOrReturn(BUS, FLAG, RETURN) { \
        uint32_t timeout = MCHF_I2C_FLAG_TIMEOUT;\
        while(I2C_GetFlagStatus((BUS), (FLAG)))\
        { if ((timeout--) == 0) { return (RETURN); } } \
        /* if (timeout < i2c_flag_timeout_max){ i2c_flag_timeout_max = timeout; }*/ \
        }

#define I2C_EventCompleteOrReturn(BUS,EVENT, RETURN) { \
        uint32_t timeout = MCHF_I2C_LONG_TIMEOUT;\
        while(!I2C_CheckEvent((BUS), (EVENT)))\
        { if ((timeout--) == 0) { return (RETURN); } } \
        if (timeout < i2c_event_timeout_max){ i2c_event_timeout_max = timeout; } }


#endif
