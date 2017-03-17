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

uint16_t MCHF_I2C_StartTransfer(I2C_TypeDef* bus, uint8_t devaddr, uint8_t* data_ptr, uint16_t data_size, bool isWrite, bool isSingleByteRead);
uint16_t MCHF_I2C_WriteRegister(I2C_TypeDef* bus, uchar I2CAddr,uint16_t addr,uint16_t addr_size, uchar RegisterValue);
uint16_t MCHF_I2C_WriteBlock(I2C_TypeDef* bus, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t* data, uint32_t size);
uint16_t MCHF_I2C_ReadRegister(I2C_TypeDef* bus, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t *RegisterValue);
uint16_t MCHF_I2C_ReadBlock(I2C_TypeDef* bus, uchar I2CAddr,uint16_t addr, uint16_t addr_size, uint8_t *data, uint32_t size);



// Special init and wrapper functions for I2C Bus 1

void 	mchf_hw_i2c1_init();
void 	mchf_hw_i2c1_reset();
void 	mchf_hw_i2c2_reset();

void    mchf_hw_i2c2_init();

uint16_t 	mchf_hw_i2c1_WriteRegister(uchar I2CAddr,uchar RegisterAddr, uchar RegisterValue);
uint16_t 	mchf_hw_i2c1_WriteBlock(uchar I2CAddr,uchar RegisterAddr, uchar *data,ulong size);
uint16_t 	mchf_hw_i2c1_ReadRegister (uchar I2CAddr,uchar RegisterAddr, uchar *RegisterValue);
uint16_t 	mchf_hw_i2c1_ReadData(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size);

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
