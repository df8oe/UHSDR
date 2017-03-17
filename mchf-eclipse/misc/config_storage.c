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
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "config_storage.h"
#include "ui_configuration.h"
#include "serial_eeprom.h"

static uint8_t config_ramcache[MAX_VAR_ADDR*2+2];


static void ConfigStorage_CheckSameContentSerialAndFlash(void);

//
// Interface for all EEPROM (ser/virt) functions and our code
//
// if ser_eeprom_in_use == 0 write/read to serial EEPROM,
// if its 0xAA use data in buffer
// otherwise use virtual EEPROM
uint16_t ConfigStorage_ReadVariable(uint16_t addr, uint16_t *value)
{
    uint16_t retval;
    switch(ts.ser_eeprom_in_use){
    case SER_EEPROM_IN_USE_I2C:
        retval = SerialEEPROM_ReadVariable(addr, value);
        break;
    case SER_EEPROM_IN_USE_NO:
    case SER_EEPROM_IN_USE_TOO_SMALL:
        retval = Flash_ReadVariable(addr, value);
        break;
    case SER_EEPROM_IN_USE_RAMCACHE:
    {
        uint8_t lowbyte;
        uint8_t highbyte;
        uint16_t data;

        highbyte = config_ramcache[addr*2];
        lowbyte = config_ramcache[addr*2+1];
        data = lowbyte + (highbyte<<8);
        *value = data;
        retval = 0;
        break;
    }
    default:
        retval = 0;
    }
    return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : Write_EEPROM
//* Object              :
//* Object              :
//* Input Parameters    : addr to write to, 16 bit value as data
//* Output Parameters   : returns HAL_OK if OK, otherwise various error codes.
//*                       FLASH_ERROR_OPERATION is also returned if eeprom_in_use contains bogus values.
//* Functions called    :
//*----------------------------------------------------------------------------
uint16_t ConfigStorage_WriteVariable(uint16_t addr, uint16_t value)
{
    HAL_StatusTypeDef status = FLASH_ERROR_OPERATION;
    if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_I2C)
    {
        SerialEEPROM_UpdateVariable(addr, value);
        status = HAL_OK;
    }
    else if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_NO || ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_TOO_SMALL)
    {
        status = (Flash_UpdateVariable(addr, value));
    }
    else if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_RAMCACHE)
    {
        uint8_t lowbyte;
        uint8_t highbyte;

        lowbyte = (uint8_t)((0x00FF)&value);
        highbyte = (uint8_t)((0x00FF)&(value >> 8));
        config_ramcache[addr*2] = highbyte;
        config_ramcache[addr*2+1] = lowbyte;
        status = HAL_OK;
    }
    return status;
}

void ConfigStorage_Init()
{
    // virtual Eeprom init
    ts.ee_init_stat = Flash_Init(); // get status of EEPROM initialization

    ts.ser_eeprom_type = SerialEEPROM_24Cxx_Detect();

    if (ts.ser_eeprom_type != EEPROM_SER_NONE && ts.ser_eeprom_type != EEPROM_SER_WRONG_SIG && ts.ser_eeprom_type != EEPROM_SER_UNKNOWN)
    {
        if(SerialEEPROM_eepromTypeDescs[ts.ser_eeprom_type].supported == false)             // incompatible EEPROMs
        {
            ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_TOO_SMALL;         // serial EEPROM too small
        }
        else
        {
            // we read the "in use" signature here
            ts.ser_eeprom_in_use = SerialEEPROM_24Cxx_Read(1,ts.ser_eeprom_type);
            if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_NO) // empty EEPROM
            {
                ConfigStorage_CopyFlash2Serial();               // copy data from virtual to serial EEPROM
                ConfigStorage_CheckSameContentSerialAndFlash();             // just 4 debug purposes
                SerialEEPROM_24Cxx_Write(1, SER_EEPROM_IN_USE_I2C, ts.ser_eeprom_type);      // serial EEPROM in use now
            }
        }
    }
    else
    {
        ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_NO;
    }
}

void ConfigStorage_CopyFlash2RAMCache()
{
    uint16_t i, data;

    config_ramcache[0] = ts.ser_eeprom_type;
    config_ramcache[1] = ts.ser_eeprom_in_use;
    for(i=1; i <= MAX_VAR_ADDR; i++)
    {
        Flash_ReadVariable(i, &data);
        config_ramcache[i*2+1] = (uint8_t)((0x00FF)&data);
        data = data>>8;
        config_ramcache[i*2] = (uint8_t)((0x00FF)&data);
    }
    ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_RAMCACHE;
}

void ConfigStorage_CopySerial2RAMCache()
{
    SerialEEPROM_24Cxx_ReadBulk(0, config_ramcache, MAX_VAR_ADDR*2+2, ts.ser_eeprom_type);

    config_ramcache[0] = ts.ser_eeprom_type;
    config_ramcache[1] = ts.ser_eeprom_in_use;

    ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_RAMCACHE;
}

uint16_t ConfigStorage_CopyRAMCache2Serial()
{
    uint16_t retval = SerialEEPROM_24Cxx_WriteBulk(0, config_ramcache, MAX_VAR_ADDR*2+2, ts.ser_eeprom_type);
    ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_I2C;
    return retval;
}

// copy data from flash storage to serial EEPROM
void ConfigStorage_CopyFlash2Serial(void)
{
    ConfigStorage_CopyFlash2RAMCache();
    ConfigStorage_CopyRAMCache2Serial();
}

// copy data from serial to virtual EEPROM
void ConfigStorage_CopySerial2Flash(void)
{
    uint16_t count;
    uint16_t data;

    for(count=1; count <= MAX_VAR_ADDR; count++)
    {
        SerialEEPROM_ReadVariable(count, &data);
        Flash_UpdateVariable(count, data);
    }
}

/**
 * verify data serial / virtual EEPROM
 * sets ts.ser_eeprom_in_use to SER_EEPROM_IN_USE_ERROR if
 * it encounters a difference between flash and I2C
 * Used only for debugging
 */
static void ConfigStorage_CheckSameContentSerialAndFlash(void)
{
    uint16_t count;
    uint16_t data1, data2;

    for(count=1; count <= MAX_VAR_ADDR; count++)
    {
        SerialEEPROM_ReadVariable(count, &data1);
        Flash_ReadVariable(count, &data2);
        if(data1 != data2)
        {
            ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_ERROR; // mark data copy as faulty
        }
    }
}
