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

#ifndef __MCHF_HW_I2C2_H
#define __MCHF_HW_I2C2_H

void 		mchf_hw_i2c2_init(void);
uint16_t 	mchf_hw_i2c2_WriteRegister(uchar I2CAddr,uchar RegisterAddr, uchar RegisterValue);

// serial eeprom definitions by DF8OE

#define MEM_DEVICE_WRITE_ADDR 0xA0
#define MEM_DEVICE_READ_ADDR 0xA1

void Delay(__IO uint32_t);

uint16_t Write_24Cxx(uint32_t, uint8_t, uint8_t);
uint16_t Read_24Cxx(uint32_t, uint8_t);
uint16_t Write_24Cxxseq(uint32_t, uint8_t*, uint16_t, uint8_t);
uint16_t Read_24Cxxseq(uint32_t, uint8_t*, uint16_t, uint8_t);

#endif
