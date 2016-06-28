/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description: Serial EEPROM Functions by DF8OE                                                                  **
**  Last Modified:                                                                 **
**  Licence:        CC BY-NC-SA 3.0                                                **
************************************************************************************/

#ifndef __SERIAL_EEPROM_H
#define __SERIAL_EEPROM_H

void     SerialEEPROM_Clear();
bool     SerialEEPROM_Exists();

// low level interface
uint16_t SerialEEPROM_24Cxx_Write(uint32_t, uint8_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_Read(uint32_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_WriteBulk(uint32_t, uint8_t*, uint16_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_ReadBulk(uint32_t, uint8_t*, uint16_t, uint8_t);
uint8_t  SerialEEPROM_24Cxx_Detect();

#endif
