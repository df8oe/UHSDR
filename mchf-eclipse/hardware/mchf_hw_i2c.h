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

#ifndef __MCHF_HW_I2C_H
#define __MCHF_HW_I2C_H


// Generic Functions for Handling I2C Busses in MCHF

uint16_t MCHF_I2C_StartTransfer(I2C_TypeDef* bus, uint8_t devaddr, uint8_t* data_ptr, uint16_t data_size, bool isWrite, bool isSingleByteRead);
uint16_t MCHF_I2C_WriteRegister(I2C_TypeDef* bus, uchar I2CAddr,uint8_t* addr_ptr,uint16_t addr_size, uchar RegisterValue);
uint16_t MCHF_I2C_WriteBlock(I2C_TypeDef* bus, uchar I2CAddr, uint8_t* addr_ptr, uint16_t addr_size, uint8_t* data, uint32_t size);
uint16_t MCHF_I2C_ReadRegister(I2C_TypeDef* bus, uchar I2CAddr, uint8_t* addr_ptr, uint16_t addr_size, uint8_t *RegisterValue);
uint16_t MCHF_I2C_ReadBlock(I2C_TypeDef* bus, uchar I2CAddr,uint8_t* addr_ptr, uint16_t addr_size, uint8_t *data, uint32_t size);



// Special init and wrapper functions for I2C Bus 1

void 	mchf_hw_i2c_init(void);
void 	mchf_hw_i2c_reset(void);

uint16_t 	mchf_hw_i2c_WriteRegister(uchar I2CAddr,uchar RegisterAddr, uchar RegisterValue);
uint16_t 	mchf_hw_i2c_WriteBlock(uchar I2CAddr,uchar RegisterAddr, uchar *data,ulong size);
uint16_t 	mchf_hw_i2c_ReadRegister (uchar I2CAddr,uchar RegisterAddr, uchar *RegisterValue);
uint16_t 	mchf_hw_i2c_ReadData(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size);

#endif
