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
    bool supported; // i.e. big enough to be used
	uint16_t pagesize; // in Bytes
    const char* name;
} SerialEEPROM_EEPROMTypeDescriptor;

#define SERIAL_EEPROM_DESC_NUM 20
#define SERIAL_EEPROM_DESC_REAL 7
// the first type number not used as pseudo device (24xx128)
#define SERIAL_EEPROM_MIN_USEABLE_SIZE 32*1024

// WE CANNOT CHANGE THESE CODES AS THIS WILL BREAK EXISTING CONFIGURATIONS STORED IN EEPROM
#define SER_EEPROM_IN_USE           0x00
#define SER_EEPROM_NOT_IN_USE       0xFF
#define SER_EEPROM_TOO_SMALL        0x10



extern const SerialEEPROM_EEPROMTypeDescriptor SerialEEPROM_eepromTypeDescs[SERIAL_EEPROM_DESC_NUM];


// low level interface
bool     SerialEEPROM_24xx_Exists();
uint16_t SerialEEPROM_24Cxx_Write(uint32_t, uint8_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_Read(uint32_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_WriteBulk(uint32_t, const uint8_t*, uint16_t, uint8_t);
uint16_t SerialEEPROM_24Cxx_ReadBulk(uint32_t, uint8_t*, uint16_t, uint8_t);

uint16_t SerialEEPROM_ReadVariable(uint16_t addr, uint16_t *value);
uint16_t SerialEEPROM_WriteVariable(uint16_t addr, uint16_t value);
uint16_t SerialEEPROM_UpdateVariable(uint16_t addr, uint16_t value);

uint8_t  SerialEEPROM_Detect();
uint16_t SerialEEPROM_Set_UseStateInSignature(uint8_t state);
uint16_t SerialEEPROM_Get_UseStateInSignature();
void     SerialEEPROM_Clear_Signature();

#endif
