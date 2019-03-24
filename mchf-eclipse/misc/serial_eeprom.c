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

// Common
#include "uhsdr_board.h"
#include "uhsdr_hw_i2c.h"

#include "serial_eeprom.h"

// for MAX_VAR_ADDR only
#include "ui_configuration.h"

typedef struct SerialEEPROM_Configuration
{
    uint8_t device_type;
    uint8_t use_type;
    bool    detection;
} SerialEEPROM_Configuration_t;

// static SerialEEPROM_Configuration_t ser_eeprom_config;

const SerialEEPROM_EEPROMTypeDescriptor SerialEEPROM_eepromTypeDescs[SERIAL_EEPROM_DESC_NUM] = {
        // 0
        {
                .size = 0,
                .supported = false,
                .pagesize = 0,
                .name = "No EEPROM"
        },
        // 1
        {
                .size = 0,
                .supported = false,
                .pagesize = 0,
                .name = "Wrong Signature"
        },
        // 2
        {
                .size = 0,
                .supported = false,
                .pagesize = 0,
                .name = "Unknown Type"
        },
        // 3
        {
                .size = 0,
                .supported = false,
                .pagesize = 0,
                .name = "Not used"
        },
        // 4
        {
                .size = 0,
                .supported = false,
                .pagesize = 64,
                .name = "Not used"
        },
        // 5
        {
                .size = 0,
                .supported = false,
                .pagesize = 0,
                .name = "Not used"
        },
        // 6
        {
                .size = 0,
                .supported = false,
                .pagesize = 0,
                .name = "Not used"
        },
        // 7
        {
                .size = 128,
                .supported = false,
                .pagesize = 8,
                .name = "24xx01"
        },
        // 8
        {
                .size = 256,
                .supported = false,
                .pagesize = 8,
                .name = "24xx02"
        },
        // 9
        {
                .size = 2*256,
                .supported = false,
                .pagesize = 16,
                .name = "24xx04"
        },
        // 10
        {
                .size = 1*1024,
                .supported = false,
                .pagesize = 16,
                .name = "24xx08"
        },
        // 11
        {
                .size = 2*1024,
                .supported = false,
                .pagesize = 16,
                .name = "24xx16"
        },
        // 12
        {
                .size = 4*1024,
                .supported = false,
                .pagesize = 32,
                .name = "24xx32"
        },
        // 13
        {
                .size = 8*1024,
                .supported = false,
                .pagesize = 32,
                .name = "24xx64"
        },
        // 14
        {
                .size = 16*1024,
                .supported = false,
                .pagesize = 64,
                .name = "24xx128"
        },
        // 15
        {
                .size = 32*1024,
                .supported = true,
                .pagesize = 64,
                .name = "24xx256"
        },
        // 16
        {
                .size = 64*1024,
                .supported = true,
                .pagesize = 128,
                .name = "24xx512"
        },
        // 17
        {
                .size = 128*1024,
                .supported = true,
                .pagesize = 128,
                .name = "24xx1025"
        },
        // 18
        {
                .size = 128 * 1024,
                .supported = true,
                .pagesize = 128,
                .name = "24xx1026/CM01"
        },
        // 19
        {
                .size = 256*1024,
                .supported = true,
                .pagesize = 256,
                .name = "24CM02"
        }
};

typedef struct
{
    uint8_t devaddr;
    uint16_t addr;
    uint16_t addr_size;
} SerialEEPROM_24CXX_Descriptor;

static SerialEEPROM_24CXX_Descriptor serialEeprom_desc;
// THIS CAN BE USED ONLY WITH SINGLE EEPROM AND SINGLE THREAD
// NOT THREAD SAFE, USE local variable instead then



#define MEM_DEVICE_WRITE_ADDR 0xA0
// serial eeprom functions by DF8OE

static uint16_t SerialEEPROM_24Cxx_DeviceConnected()
{
    uint16_t retVal = UhsdrHw_I2C_DeviceReady(&hi2c2,MEM_DEVICE_WRITE_ADDR);
    return retVal;
}


static void SerialEEPROM_24Cxx_AdjustAddrs(const uint8_t Mem_Type, uint8_t* devaddr_ptr, uint32_t* Addr_ptr)
{

    *devaddr_ptr = MEM_DEVICE_WRITE_ADDR;

    if (*Addr_ptr > 0xFFFF) {
        switch (Mem_Type)
        {
        case 17: // 24LC1025
            *devaddr_ptr = MEM_DEVICE_WRITE_ADDR + 8;            // 24LC1025
            break;
        case  18:
            *devaddr_ptr = MEM_DEVICE_WRITE_ADDR + 2;            // 24LC1026
            break;
        case  19: // 24CM02
            *devaddr_ptr = MEM_DEVICE_WRITE_ADDR + ((*Addr_ptr & 0x30000) >> 15); // the upper bits 16 and 17 determine the I2C address offset
            break;
        }
        *Addr_ptr &= 0xFFFF; // mask address to 16bits in all cases
    }
}

static uint16_t SerialEEPROM_24Cxx_ackPolling(uint32_t Addr, uint8_t Mem_Type)
{
    uint8_t devaddr;

    SerialEEPROM_24Cxx_AdjustAddrs(Mem_Type,&devaddr,&Addr);

    uint16_t retVal = UhsdrHw_I2C_DeviceReady(&hi2c2,devaddr); // != HAL_OK?0xFD00:0;
    return retVal;
}

static void SerialEEPROM_24Cxx_StartTransfer_Prep(uint32_t Addr, uint8_t Mem_Type, SerialEEPROM_24CXX_Descriptor* eeprom_desc_ptr)
{
    SerialEEPROM_24Cxx_AdjustAddrs(Mem_Type,&eeprom_desc_ptr->devaddr,&Addr);

    eeprom_desc_ptr->addr = (uint16_t)((0xFFFF)&(Addr));
    if (Mem_Type > 8)
    {
        eeprom_desc_ptr->addr_size = I2C_MEMADD_SIZE_16BIT;
    }
    else
    {
        eeprom_desc_ptr->addr_size = I2C_MEMADD_SIZE_8BIT;
    }
}

uint16_t SerialEEPROM_24Cxx_Write(uint32_t Addr, uint8_t Data, uint8_t Mem_Type)
{
    SerialEEPROM_24Cxx_StartTransfer_Prep(Addr, Mem_Type,&serialEeprom_desc);
    uint16_t retVal = UhsdrHw_I2C_WriteRegister(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,serialEeprom_desc.addr,serialEeprom_desc.addr_size,Data);

    if (!retVal)
    {
        retVal = SerialEEPROM_24Cxx_ackPolling(Addr,Mem_Type);
    }

    return retVal;
}


uint16_t SerialEEPROM_24Cxx_Read(uint32_t Addr, uint8_t Mem_Type)
{
    uint8_t value;
    SerialEEPROM_24Cxx_StartTransfer_Prep(Addr, Mem_Type,&serialEeprom_desc);
    uint16_t retVal = UhsdrHw_I2C_ReadRegister(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,serialEeprom_desc.addr,serialEeprom_desc.addr_size,&value);
    if (!retVal)
    {
        retVal = value;
    }
    return retVal;
}

uint16_t SerialEEPROM_24Cxx_ReadBulk(uint32_t Addr, uint8_t *buffer, uint16_t length, uint8_t Mem_Type)
{
    uint16_t retVal = 0xFFFF;
    if (Mem_Type < SERIAL_EEPROM_DESC_NUM) {
        uint32_t page, count;
        count = 0;

        page =SerialEEPROM_eepromTypeDescs[Mem_Type].pagesize;

        while(count < length)
        {
            if (length - count < page)
            {
                // make sure we do not read more than asked for! Buffer Overflow!
                page = (length - count);
            }
            SerialEEPROM_24Cxx_StartTransfer_Prep(Addr + count, Mem_Type,&serialEeprom_desc);
            retVal = UhsdrHw_I2C_ReadBlock(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,serialEeprom_desc.addr,serialEeprom_desc.addr_size,&buffer[count],page);
            count+=page;
            if (retVal)
            {
                break;
            }
        }
    }
    return retVal;
}

/**
 * Write a consecutive area of memory into the I2C eeprom starting with Addr.
 * @param Addr  the location of the first byte in the eeprom. May have any value within the memory size of  the eeprom
 * @param buffer memory to copy from
 * @param length how many bytes to copy
 * @param Mem_Type which eeprom type are we having
 * @return 0 if success, I2C error code otherwise
 */
uint16_t SerialEEPROM_24Cxx_WriteBulk(uint32_t Addr, const uint8_t *buffer, uint16_t length, uint8_t Mem_Type)
{
    uint16_t retVal = 0;
    if (Mem_Type < SERIAL_EEPROM_DESC_NUM) {
        uint32_t count = 0;

        const uint32_t page = SerialEEPROM_eepromTypeDescs[Mem_Type].pagesize;


        while(retVal == 0 && count < length)
        {
            SerialEEPROM_24Cxx_StartTransfer_Prep(Addr + count, Mem_Type,&serialEeprom_desc);

            uint32_t transfer_size = page;

            // correct the transfer_size to keep inside a single page
            // if the start address is not on a page boundary
            // this will happen only for the first bulkd write (if at all)
            // then the Addr + count will be aligned on page boundaries

            transfer_size -= ((Addr+count) % page);

            if (length - count < transfer_size)
            {
                transfer_size = length - count;
            }

            retVal = UhsdrHw_I2C_WriteBlock(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,serialEeprom_desc.addr,serialEeprom_desc.addr_size,&buffer[count],transfer_size);
            count+=transfer_size;
            if (retVal)
            {
                break;
            }
            retVal = SerialEEPROM_24Cxx_ackPolling(Addr,Mem_Type);
        }
    }
    return retVal;
}

/**
 * @brief in general a non-destructive probing to identify the used EEPROM, in some cases devices are written (and changed data restored)
 * this code uses hardware properties to identify the connected I2C eeprom, no previously signature etc. used or required
 * @returns identified eeprom chip type or EEPROM_SER_UNKNOWN
 */
static uint8_t SerialEEPROM_24Cxx_DetectProbeHardware()
{
    uint8_t ser_eeprom_type = EEPROM_SER_UNKNOWN;

    const uint8_t testsignatureA = 0x66;
    const uint8_t testsignatureB = 0x77; // has to be different from testsignature 1

    const uint16_t testaddr1 = 0x0001;
    const uint16_t testaddr1plus128 = testaddr1 + 0x80;

    // 8 Bit Test First
    uint16_t test8Byte1 = SerialEEPROM_24Cxx_Read(testaddr1,8);


    // first decide if 8 or 16 bit addressable eeprom by trying to read with 8 or 16 bit algorithm
    // if an algorithm succeeds we know the address width
    if(test8Byte1 < 0x100)
    {
        // 8 bit addressing
        uint16_t test8Byte2 = SerialEEPROM_24Cxx_Read(testaddr1plus128,8);

        SerialEEPROM_24Cxx_Write(testaddr1,testsignatureB,8);
        SerialEEPROM_24Cxx_Write(testaddr1plus128,testsignatureB,8);

        if (SerialEEPROM_24Cxx_Read(testaddr1,8) == testsignatureB)
        {
            SerialEEPROM_24Cxx_Write(testaddr1,testsignatureA,8);              // write test signature
            ser_eeprom_type = 7;             // smallest possible 8 bit EEPROM (128Bytes)
            if(SerialEEPROM_24Cxx_Read(testaddr1plus128,8) == testsignatureB)
            {
                ser_eeprom_type = 8;
            }
        }
        SerialEEPROM_24Cxx_Write(testaddr1,test8Byte1,8);         // write back old data
        SerialEEPROM_24Cxx_Write(testaddr1plus128,test8Byte2,8);
    }
    if (ser_eeprom_type == EEPROM_SER_UNKNOWN)
    {
        // 16 bit addressing check
        // the other banks are mapped to different I2C
        // device addresses. We simply try to read from them
        // and if it succeeds we know the device type
        // We need to check unique addresses for each EEPROM type
        // 0xA0 + 0x10000: 0xA8
        if(SerialEEPROM_24Cxx_Read(0x10000,17) < 0x100)
        {
            ser_eeprom_type = 17;            // 24LC1025
        }
        // 0xA0 + 0x10000: 0xA2
        if(SerialEEPROM_24Cxx_Read(0x10000,18) < 0x100)
        {
            ser_eeprom_type = 18;            // 24LC1026
        }
        // 0xA0 + 0x10000: 0xA2 + 0x20000: 0xA4 + 0x30000: 0xA6
        if(SerialEEPROM_24Cxx_Read(0x20000,19) < 0x100)
        {
            ser_eeprom_type = 19;            // 24CM02
        }

        // it is not a large EEPROM (i.e. >64KB Data)
        //
        // the following test requires writing to EEPROM,
        // we remember content of test locations
        // and write them back later
        if(ser_eeprom_type == EEPROM_SER_UNKNOWN)
        {
            const uint16_t testaddr1plus256 = 0x0100 + testaddr1; // same as before but 0x100 == 256 byte spacing
            uint16_t testByte1 = SerialEEPROM_24Cxx_Read(testaddr1,16);         // read old content
            uint16_t testByte2 = SerialEEPROM_24Cxx_Read(testaddr1plus256,16);     // of test bytes

            if (testByte1 < 0x100 && testByte2 < 0x100) {

                // successful read from both locations, let us start
                SerialEEPROM_24Cxx_Write(testaddr1, testsignatureA,16);         // write testsignature 1
                SerialEEPROM_24Cxx_Write(testaddr1plus256, testsignatureB,16);         // write testsignature 2
                if(SerialEEPROM_24Cxx_Read(testaddr1,16) == testsignatureA && SerialEEPROM_24Cxx_Read(testaddr1plus256,16) == testsignatureB)
                {
                    // 16 bit addressing
                    ser_eeprom_type = 9;         // smallest possible 16 bit EEPROM

                    // we look for the "looping" in data, i.e. we read back 0x66
                    // which we wrote to address  0x0003
                    // from a address we did not wrote it to
                    // once it occurs, we know the  size of the EEPROM
                    // since all of these EEPROMS  are ignoring the unused bits in  the address
                    for (int shift = 0; shift < 8; shift++) {
                        uint16_t shift_test_addr = (0x200 << shift)+testaddr1;
                        if(SerialEEPROM_24Cxx_Read(shift_test_addr,16) == testsignatureA) {
                            // now we write test signature 2 to make sure the match was no accident
                            // i.e. match with same content in EEPROM
                            SerialEEPROM_24Cxx_Write(shift_test_addr,testsignatureB,16);
                            if(SerialEEPROM_24Cxx_Read(testaddr1,16) == testsignatureB) {
                                // we found the looping,
                                // now stop checking and set EEPROM type
                                ser_eeprom_type = 9 + shift;
                                break;
                            }
                            else
                            {
                                // oops: just an accident, so we write back old content and continue
                                SerialEEPROM_24Cxx_Write(shift_test_addr,testsignatureA,16);
                            }
                        }
                    }

                }
                SerialEEPROM_24Cxx_Write(testaddr1,testByte1,16);         // write back old data
                SerialEEPROM_24Cxx_Write(testaddr1plus256,testByte2,16);
            }
        }
    }
    return ser_eeprom_type;
}

static void SerialEEPROM_Read_Signature(uint8_t ser_eeprom_type, uint16_t* type_p, uint16_t* state_p)
{
    *type_p = SerialEEPROM_24Cxx_Read(0,ser_eeprom_type);
    *state_p = SerialEEPROM_24Cxx_Read(1,ser_eeprom_type);
}

uint16_t SerialEEPROM_Set_UseStateInSignature(uint8_t state)
{
    return SerialEEPROM_24Cxx_Write(1,SER_EEPROM_IN_USE,ts.ser_eeprom_type);
}

uint16_t SerialEEPROM_Get_UseStateInSignature()
{
    return SerialEEPROM_24Cxx_Read(1,ts.ser_eeprom_type);
}

uint8_t SerialEEPROM_Detect() {

    uint16_t ser_eeprom_type = EEPROM_SER_UNKNOWN;

    // serial EEPROM init
    if(SerialEEPROM_24Cxx_DeviceConnected() != HAL_OK || SerialEEPROM_24xx_Exists() == false)  // Issue with Ser EEPROM, either not available or other problems
    {
        ser_eeprom_type = EEPROM_SER_NONE;             // no serial EEPROM available

    }
    else
    {

        // this initial read will bring us the first byte of any eeprom (8 or 16bits)
        // if we read anything but the 0xff == SER_EERPM_NOT_IN_USE value which indicates empty eeprom
        // we assume some configuration information tells us what EEPROM we have and how it is used
        if(SerialEEPROM_24Cxx_Read(0,16) != SER_EEPROM_NOT_IN_USE)
        {
            ser_eeprom_type = EEPROM_SER_WRONG_SIG;

            // unless we find a correct signature, we have to assume the EEPROM has incorrect/corrupted data

            uint16_t ser_eeprom_type_read;
            uint16_t ser_eeprom_sig;

            SerialEEPROM_Read_Signature(8, &ser_eeprom_type_read, &ser_eeprom_sig);

            // all 8 bit (i.e. 256 or 128 Byte) EEPROMS are marked as "too small" during detection
            if(ser_eeprom_sig == SER_EEPROM_TOO_SMALL && ser_eeprom_type_read >= SERIAL_EEPROM_DESC_REAL && ser_eeprom_type_read < 9)
            {
                ser_eeprom_type = ser_eeprom_type_read;
            }
            else
            {
                SerialEEPROM_Read_Signature(16, &ser_eeprom_type_read, &ser_eeprom_sig);

                // we either have a new EEPROM, just being initialized  ( ser_eeprom_sig = SER_EEPROM_NOT_IN_USE && valid type) or
                // we have an used EEPROM (ser_eeprom_sig = SER_EEPROM_IN_USE && valid type)
                // please note, that even though no 16 bit EEPROM gets the "too small" signature written in, these may still be
                // consider too small by the later code. Only EEPROMS which have the "supported" flag set in the descriptor will
                // be consider of sufficient size for actual use
                if((ser_eeprom_sig == SER_EEPROM_IN_USE || ser_eeprom_sig == SER_EEPROM_NOT_IN_USE) && ser_eeprom_type_read < SERIAL_EEPROM_DESC_NUM && ser_eeprom_type_read > 8)
                {
                    ser_eeprom_type = ser_eeprom_type_read;
                }
            }
        }
        else
        {
            ser_eeprom_type = SerialEEPROM_24Cxx_DetectProbeHardware();

            if (ser_eeprom_type >= SERIAL_EEPROM_DESC_REAL && ser_eeprom_type < SERIAL_EEPROM_DESC_NUM)
            {
                SerialEEPROM_24Cxx_Write(0,ser_eeprom_type,ser_eeprom_type);
                if (SerialEEPROM_eepromTypeDescs[ser_eeprom_type].supported == false)
                {
                    SerialEEPROM_24Cxx_Write(1,SER_EEPROM_TOO_SMALL,ser_eeprom_type);
                }
            }
        }
        // just to be save. Never ever deliver a type id outside the array boundaries.
        if (ser_eeprom_type >= SERIAL_EEPROM_DESC_NUM)
        {
            ser_eeprom_type = EEPROM_SER_UNKNOWN;
        }
    }

    return ser_eeprom_type;

}

static void SerialEEPROM_Clear_Variable(uint16_t addr)
{
        const uint8_t empty_var[2] = { 0xff, 0xff };
        SerialEEPROM_24Cxx_WriteBulk(addr*2, empty_var,2, ts.ser_eeprom_type);
}

void  SerialEEPROM_Clear_AllVariables()
{
    // variable 0 is the reserved signature variable
    for(uint16_t count=1; count <= MAX_VAR_ADDR; count++)
    {
        SerialEEPROM_Clear_Variable(count);
    }
}

void  SerialEEPROM_Clear_Signature()
{
    // variable 0 is the reserved signature variable
    ts.ser_eeprom_type = SerialEEPROM_24Cxx_DetectProbeHardware();
    SerialEEPROM_Clear_Variable(0);
}


bool SerialEEPROM_24xx_Exists()
{
    return SerialEEPROM_24Cxx_Read(0,8)  < 0x100;
}


//
// Interface for serial EEPROM functions
//
uint16_t SerialEEPROM_ReadVariable(uint16_t addr, uint16_t *value)      // reference to serial EEPROM read function
{
    uint8_t bytes[2];


    uint16_t retval = SerialEEPROM_24Cxx_ReadBulk(addr*2,bytes, 2, ts.ser_eeprom_type);

    if (retval == HAL_OK)
    {
        *value =  ((uint16_t)bytes[0])<<8;
        *value |= bytes[1];
    }

    return retval;
}

uint16_t SerialEEPROM_UpdateVariable(uint16_t addr, uint16_t value)
{
        uint16_t value_read = 0;
        uint16_t retval = SerialEEPROM_ReadVariable(addr,&value_read);
        if  (retval != 0 || value_read != value )
        {
            retval = SerialEEPROM_WriteVariable(addr,value);
        }
        return retval;
}

uint16_t SerialEEPROM_WriteVariable(uint16_t addr, uint16_t value)      // reference to serial EEPROM write function, writing unsigned 16 bit
{
    uint8_t bytes[2];

    bytes[0] = (uint8_t)((value&(0xFF00))>>8);
    bytes[1] = (uint8_t)(value&(0x00FF));

    return SerialEEPROM_24Cxx_WriteBulk(addr*2, bytes, 2, ts.ser_eeprom_type);
}
