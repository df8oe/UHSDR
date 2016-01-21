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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __MCHF_HW_I2C_H
#define __MCHF_HW_I2C_H

void 	mchf_hw_i2c_init(void);
void 	mchf_hw_i2c_reset(void);

uchar 	mchf_hw_i2c_WriteRegister(uchar I2CAddr,uchar RegisterAddr, uchar RegisterValue);
uchar 	mchf_hw_i2c_WriteBlock(uchar I2CAddr,uchar RegisterAddr, uchar *data,ulong size);
uchar 	mchf_hw_i2c_ReadRegister (uchar I2CAddr,uchar RegisterAddr, uchar *RegisterValue);
uchar 	mchf_hw_i2c_ReadData(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size);

#endif
