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
#ifndef USE_HAL_DRIVER
    #define USE_HAL_DRIVER
#endif
#include "uhsdr_board.h"
#include <stdint.h>
#include "config_storage.h"
#include "ui_configuration.h"
#include "serial_eeprom.h"
#include "uhsdr_flash.h"

static uint8_t config_ramcache[MAX_VAR_ADDR*2+2];


#ifdef USE_CONFIGSTORAGE_FLASH
void ConfigStorage_CopyFlash2RAMCache()
{
    uint16_t i, data;

    config_ramcache[0] = ts.ser_eeprom_type;
    config_ramcache[1] = ts.configstore_in_use;
    for(i=1; i <= MAX_VAR_ADDR; i++)
    {
        Flash_ReadVariable(i, &data);
        config_ramcache[i*2+1] = (uint8_t)((0x00FF)&data);
        data = data>>8;
        config_ramcache[i*2] = (uint8_t)((0x00FF)&data);
    }
    ts.configstore_in_use = CONFIGSTORE_IN_USE_RAMCACHE;
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
 * sets ts.configstore_in_use to CONFIGSTORE_IN_USE_ERROR if
 * it encounters a difference between flash and I2C
 * Used only for debugging
 */
static bool ConfigStorage_CheckSameContentSerialAndFlash(void)
{
    bool retval = true;
    for(uint16_t count=1; retval == true && count <= MAX_VAR_ADDR; count++)
    {
        uint16_t data1, data2;
        SerialEEPROM_ReadVariable(count, &data1);
        Flash_ReadVariable(count, &data2);
        if(data1 != data2)
        {
            retval = false;
            ts.configstore_in_use = CONFIGSTORE_IN_USE_ERROR; // mark data copy as faulty
        }
    }
    return retval;
}
#endif

//
// Interface for all EEPROM (ser/virt) functions and our code
//
// if configstore_in_use == 0 write/read to serial EEPROM,
// if its 0xAA use data in buffer
// otherwise use virtual EEPROM
uint16_t ConfigStorage_ReadVariable(uint16_t addr, uint16_t *value)
{
    uint16_t retval;
    switch(ts.configstore_in_use){
    case CONFIGSTORE_IN_USE_I2C:
        retval = SerialEEPROM_ReadVariable(addr, value);
        break;
    case CONFIGSTORE_IN_USE_FLASH:
    case SER_EEPROM_TOO_SMALL:
        retval = Flash_ReadVariable(addr, value);
        break;
    case CONFIGSTORE_IN_USE_RAMCACHE:
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
    HAL_StatusTypeDef status = HAL_ERROR;
    if(ts.configstore_in_use == CONFIGSTORE_IN_USE_I2C)
    {
        SerialEEPROM_UpdateVariable(addr, value);
        status = HAL_OK;
    }
    else if(ts.configstore_in_use == CONFIGSTORE_IN_USE_FLASH)
    {
        status = (Flash_UpdateVariable(addr, value));
    }
    else if(ts.configstore_in_use == CONFIGSTORE_IN_USE_RAMCACHE)
    {
        /**
         * Was found that during saving settings,
         * first two cells was corrupted.
         * Just temp. protection...
         */
        if ( addr == 0 )
        {
            status = HAL_ERROR;
        }
        else
        {
            uint8_t lowbyte;
            uint8_t highbyte;

            lowbyte = (uint8_t)((0x00FF)&value);
            highbyte = (uint8_t)((0x00FF)&(value >> 8));
            config_ramcache[addr*2] = highbyte;
            config_ramcache[addr*2+1] = lowbyte;
            status = HAL_OK;
        }
    }
    return status;
}

void ConfigStorage_Init()
{
    // virtual Eeprom init
#ifdef USE_CONFIGSTORAGE_FLASH
    ts.ee_init_stat = Flash_Init(); // get status of EEPROM initialization
#else
    ts.ee_init_stat = HAL_ERROR; // get status of EEPROM initialization
#endif

    ts.configstore_in_use = ts.ee_init_stat == HAL_OK? CONFIGSTORE_IN_USE_FLASH: CONFIGSTORE_IN_USE_ERROR;
    // at this point, we have a working flash config storage (or not)

    ts.ser_eeprom_type = SerialEEPROM_Detect();

    // incompatible EEPROMs, no EEPROM at all, EEPROM to small
    if(SerialEEPROM_eepromTypeDescs[ts.ser_eeprom_type].supported == true)
    {
        // we read the "in use" signature here, first we use a 16 bit value to keep the error code if any
        // this was report in #857
        uint16_t ser_eeprom_in_use = SerialEEPROM_Get_UseStateInSignature();

        if(ser_eeprom_in_use == SER_EEPROM_NOT_IN_USE) // empty EEPROM
        {

#ifdef USE_CONFIGSTORAGE_FLASH
            // if we have a flash storage available, copy its contents into EEPROM
            if (ts.ee_init_stat == HAL_OK)
            {
                ts.configstore_in_use = CONFIGSTORE_IN_USE_FLASH;

                ConfigStorage_CopyFlash2Serial();               // copy data from virtual to serial EEPROM
                ConfigStorage_CheckSameContentSerialAndFlash();             // just 4 debug purposes
            }
#endif
            SerialEEPROM_Set_UseStateInSignature(SER_EEPROM_IN_USE);      // serial EEPROM in use now
        }
        else
        {
            // if larger than 255 I2C code reported an error
            if (ser_eeprom_in_use > 0x00ff)
            {
                ts.configstore_in_use = CONFIGSTORE_IN_USE_ERROR;
            }
            else
            {
                ts.configstore_in_use = CONFIGSTORE_IN_USE_I2C;
            }
        }
    }
}


void ConfigStorage_CopySerial2RAMCache()
{
    SerialEEPROM_24Cxx_ReadBulk(0, config_ramcache, MAX_VAR_ADDR*2+2, ts.ser_eeprom_type);

    config_ramcache[0] = ts.ser_eeprom_type;
    config_ramcache[1] = ts.configstore_in_use;

    ts.configstore_in_use = CONFIGSTORE_IN_USE_RAMCACHE;
}

uint16_t ConfigStorage_CopyRAMCache2Serial()
{
    // we start behind the signature, which we don't want to change
    uint16_t retval = SerialEEPROM_24Cxx_WriteBulk(2, config_ramcache+2, MAX_VAR_ADDR*2, ts.ser_eeprom_type);
    if (retval == HAL_OK)
    {
        ts.configstore_in_use = CONFIGSTORE_IN_USE_I2C;
    }
    return retval;
}

//copy array directly to serial EEPROM
uint16_t ConfigStorage_CopyArray2Serial(uint32_t Addr, const uint8_t *buffer, uint16_t length)
{
    uint16_t retval = HAL_OK;
    if(ts.configstore_in_use == CONFIGSTORE_IN_USE_I2C)
    {
        retval = SerialEEPROM_24Cxx_WriteBulk(Addr, buffer, length, ts.ser_eeprom_type);
    }

    return retval;
}

//read array directly from serial EEPROM
void ConfigStorage_CopySerial2Array(uint32_t Addr, uint8_t *buffer, uint16_t length)
{
    if(ts.configstore_in_use == CONFIGSTORE_IN_USE_I2C)
    {
        SerialEEPROM_24Cxx_ReadBulk(Addr, buffer, length, ts.ser_eeprom_type);
    }

}
