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

#ifndef __MCHF_HW_I2C2_H
#define __MCHF_HW_I2C2_H

void 	mchf_hw_i2c2_init(void);
uchar 	mchf_hw_i2c2_WriteRegister(uchar I2CAddr,uchar RegisterAddr, uchar RegisterValue);

#endif
