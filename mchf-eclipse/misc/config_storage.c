#include "config_storage.h"
#include "ui_configuration.h"
#include "mchf_hw_i2c2.h"


static uint8_t config_ramcache[MAX_VAR_ADDR*2+2];

//
// Interface for all EEPROM (ser/virt) functions and our code
//
// if ser_eeprom_in_use == 0 write/read to serial EEPROM,
// if its 0xAA use data in buffer
// otherwise use virtual EEPROM
uint16_t ConfigStorage_ReadVariable(uint16_t addr, uint16_t *value)
{
    if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_I2C)
        return SerialEEPROM_ReadVariable(addr, value);
    if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_NO || ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_TOO_SMALL)
        return(Flash_ReadVariable(addr, value));
    if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_RAMCACHE)
    {
        uint8_t lowbyte;
        uint8_t highbyte;
        uint16_t data;

        highbyte = config_ramcache[addr*2];
        lowbyte = config_ramcache[addr*2+1];
        data = lowbyte + (highbyte<<8);
        *value = data;
    }
    return 0;
}

static uint16_t SerialEEPROM_UpdateVariable(uint16_t addr, uint16_t value);
//*----------------------------------------------------------------------------
//* Function Name       : Write_EEPROM
//* Object              :
//* Object              :
//* Input Parameters    : addr to write to, 16 bit value as data
//* Output Parameters   : returns FLASH_COMPLETE if OK, otherwise various error codes.
//*                       FLASH_ERROR_OPERATION is also returned if eeprom_in_use contains bogus values.
//* Functions called    :
//*----------------------------------------------------------------------------
uint16_t ConfigStorage_WriteVariable(uint16_t addr, uint16_t value)
{
    FLASH_Status status = FLASH_ERROR_OPERATION;
    if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_I2C)
    {
        SerialEEPROM_UpdateVariable(addr, value);
        status = FLASH_COMPLETE;
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
        value = value>>8;
        highbyte = (uint8_t)((0x00FF)&value);
        config_ramcache[addr*2] = highbyte;
        config_ramcache[addr*2+1] = lowbyte;
        status = FLASH_COMPLETE;
    }
    return status;
}

//
// Interface for serial EEPROM functions
//
uint16_t SerialEEPROM_ReadVariable(uint16_t addr, uint16_t *value)      // reference to serial EEPROM read function
{
    uint16_t data;

    data = (uint16_t)(Read_24Cxx(addr*2, ts.ser_eeprom_type)<<8);
    data = data + (uint8_t)(Read_24Cxx(addr*2+1, ts.ser_eeprom_type));
    *value = data;

    return 0;
}

uint16_t SerialEEPROM_UpdateVariable(uint16_t addr, uint16_t value)
{
        uint16_t value_read = 0;
        uint16_t status = SerialEEPROM_ReadVariable(addr,&value_read);
        if  (status != 0 || value_read != value ) {
            SerialEEPROM_WriteVariable(addr,value);
        }
        return 0;
}

uint16_t SerialEEPROM_WriteVariable(uint16_t addr, uint16_t value)      // reference to serial EEPROM write function, writing unsigned 16 bit
{
    uint8_t lowbyte, highbyte;

    lowbyte = (uint8_t)(value&(0x00FF));
    highbyte = (uint8_t)((value&(0xFF00))>>8);

    Write_24Cxx(addr*2, highbyte, ts.ser_eeprom_type);
    Write_24Cxx(addr*2+1, lowbyte, ts.ser_eeprom_type);

    return 0;
}


// verify data serial / virtual EEPROM
void ConfigStorage_CheckSameContentSerialAndFlash(void)
{
    uint16_t count;
    uint16_t data1, data2;

    for(count=1; count <= MAX_VAR_ADDR; count++)
    {
        SerialEEPROM_ReadVariable(count, &data1);
        Flash_ReadVariable(count, &data2);
        if(data1 != data2)
            ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_ERROR; // mark data copy as faulty
    }
}

// TODO: This is not the best location, it belongs to
// the low level I2C EEPROM Code currently located elsewhere
#define EEPROM_SER_NONE 0

uint8_t SerialEEPROM_24Cxx_Detect() {

    uint8_t ser_eeprom_type = 0xFF;

    // serial EEPROM init
    //  Write_24Cxx(0,0xFF,16);     //enable to reset EEPROM and force new copyvirt2ser
    if(Read_24Cxx(0,8) > 0xFF)  // Issue with Ser EEPROM, either not available or other problems
        ser_eeprom_type = EEPROM_SER_NONE;             // no serial EEPROM available
    else
    {
        if(Read_24Cxx(0,16) != 0xFF)
        {
            if(Read_24Cxx(0,8) > 6 && Read_24Cxx(0,8) < 9 && Read_24Cxx(1,8) == 0x10)
            {
                ser_eeprom_type = Read_24Cxx(0,8);
            }
            else
            {
                ser_eeprom_type = Read_24Cxx(0,16);
            }
        }
        else
        {
            {
                Write_24Cxx(10,0xdd,8);
                if(Read_24Cxx(10,8) == 0xdd)
                {
                    // 8 bit addressing
                    Write_24Cxx(3,0x99,8);              // write testsignature
                    ser_eeprom_type = 7;             // smallest possible 8 bit EEPROM
                    if(Read_24Cxx(0x83,8) != 0x99)
                    {
                        ser_eeprom_type = 8;
                    }
                    Write_24Cxx(0,ser_eeprom_type,8);
                    Write_24Cxx(1,0x10,8);
                }
                else
                {
                    // 16 bit addressing
                    if(Read_24Cxx(0x10000,17) < 0x100)
                    {
                        ser_eeprom_type = 17;            // 24LC1025
                        Write_24Cxx(0,17,16);
                    }
                    if(Read_24Cxx(0x10000,18) < 0x100)
                    {
                        ser_eeprom_type = 18;            // 24LC1026
                        Write_24Cxx(0,18,16);
                    }
                    if(Read_24Cxx(0x10000,19) < 0x100)
                    {
                        ser_eeprom_type = 19;            // 24CM02
                        Write_24Cxx(0,19,16);
                    }
                    if(ser_eeprom_type < 17)
                    {
                        Write_24Cxx(3,0x66,16);         // write testsignature 1
                        Write_24Cxx(0x103,0x77,16);         // write testsignature 2
                        if(Read_24Cxx(3,16) == 0x66 && Read_24Cxx(0x103,16) == 0x77)
                        {
                            // 16 bit addressing
                            ser_eeprom_type = 9;         // smallest possible 16 bit EEPROM
                            if(Read_24Cxx(0x803,16) != 0x66)
                            {
                                ser_eeprom_type = 12;
                            }
                            if(Read_24Cxx(0x1003,16) != 0x66)
                            {
                                ser_eeprom_type = 13;
                            }
                            if(Read_24Cxx(0x2003,16) != 0x66)
                            {
                                ser_eeprom_type = 14;
                            }
                            if(Read_24Cxx(0x4003,16) != 0x66)
                            {
                                ser_eeprom_type = 15;
                            }
                            if(Read_24Cxx(0x8003,16) != 0x66)
                            {
                                ser_eeprom_type = 16;
                            }
                            Write_24Cxx(0,ser_eeprom_type,16);
                        }
                    }
                }
            }
        }
    }
    return ser_eeprom_type;
}

void ConfigStorage_Init()
{
    // virtual Eeprom init
    ts.ee_init_stat = Flash_Init(); // get status of EEPROM initialization

    ts.ser_eeprom_type = SerialEEPROM_24Cxx_Detect();

    if (ts.ser_eeprom_type != EEPROM_SER_NONE)
    {
        ts.ser_eeprom_in_use = Read_24Cxx(1,ts.ser_eeprom_type);
    }
    else
    {
        ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_NO;
    }

    if(ts.ser_eeprom_type < 16)             // incompatible EEPROMs
    {
        ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_TOO_SMALL;         // serial EEPROM too small
    }
    else {
        if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_NO) // empty EEPROM
        {
            ConfigStorage_CopyFlash2Serial();               // copy data from virtual to serial EEPROM
            ConfigStorage_CheckSameContentSerialAndFlash();             // just 4 debug purposes
            Write_24Cxx(1, 0, ts.ser_eeprom_type);      // serial EEPROM in use now
        }
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
    uint16_t i, data;

    config_ramcache[0] = ts.ser_eeprom_type;
    config_ramcache[1] = ts.ser_eeprom_in_use;
    for(i=1; i <= MAX_VAR_ADDR; i++)
    {
        SerialEEPROM_ReadVariable(i, &data);
        config_ramcache[i*2+1] = (uint8_t)((0x00FF)&data);
        data = data>>8;
        config_ramcache[i*2] = (uint8_t)((0x00FF)&data);
    }
    ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_RAMCACHE;
}

void ConfigStorage_CopyRAMCache2Serial()
{
    Write_24Cxxseq(0, config_ramcache, MAX_VAR_ADDR*2+2, ts.ser_eeprom_type);
    ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_I2C;
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
