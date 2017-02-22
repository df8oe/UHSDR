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
**  Licence:        GNU GPLv3                                                      **
************************************************************************************/

#ifndef __SERIAL_EEPROM_H
#define __SERIAL_EEPROM_H


typedef struct {
    uint32_t size; // in Bytes
    uint16_t pagesize; // in Bytes
    bool supported; // i.e. big enough to be used
    const char* name;
} SerialEEPROM_EEPROMTypeDescriptor;

#define SERIAL_EEPROM_DESC_NUM 20
extern const SerialEEPROM_EEPROMTypeDescriptor SerialEEPROM_eepromTypeDescs[SERIAL_EEPROM_DESC_NUM];

void     SerialEEPROM_Clear();
bool     SerialEEPROM_Exists();

// low level interface
uint16_t SerialEEPROM_24Cxx_Write(uint32_t, uint8_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_Read(uint32_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_WriteBulk(uint32_t, uint8_t*, uint16_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_ReadBulk(uint32_t, uint8_t*, uint16_t, uint8_t);
uint8_t  SerialEEPROM_24Cxx_Detect();

uint16_t SerialEEPROM_ReadVariable(uint16_t addr, uint16_t *value);
uint16_t SerialEEPROM_WriteVariable(uint16_t addr, uint16_t value);
uint16_t SerialEEPROM_UpdateVariable(uint16_t addr, uint16_t value);

#endif
