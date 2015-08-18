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

#ifndef __I2S_H
#define __I2S_H

void I2S_Block_Init(void);

void I2S_Block_Process(uint32_t txAddr, uint32_t rxAddr, uint32_t Size);
void I2S_Block_Stop(void);

#endif

