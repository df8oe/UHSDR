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
#include "mchf_board.h"
#include "mchf_hw_i2c.h"

#include "serial_eeprom.h"

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
                .name = "24xx1026/24CM01"
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

    uint16_t retVal = HAL_I2C_IsDeviceReady(&hi2c2,devaddr,100,100); // != HAL_OK?0xFD00:0;
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
    uint16_t retVal = MCHF_I2C_WriteRegister(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,serialEeprom_desc.addr,serialEeprom_desc.addr_size,Data);

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
    uint16_t retVal = MCHF_I2C_ReadRegister(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,serialEeprom_desc.addr,serialEeprom_desc.addr_size,&value);
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
            retVal = MCHF_I2C_ReadBlock(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,serialEeprom_desc.addr,serialEeprom_desc.addr_size,&buffer[count],page);
            count+=page;
            if (retVal)
            {
                break;
            }
        }
    }
    return retVal;
}

uint16_t SerialEEPROM_24Cxx_WriteBulk(uint32_t Addr, uint8_t *buffer, uint16_t length, uint8_t Mem_Type)
{
    uint16_t retVal = 0;
    if (Mem_Type < SERIAL_EEPROM_DESC_NUM) {
        uint32_t page, count;
        count = 0;

        page =SerialEEPROM_eepromTypeDescs[Mem_Type].pagesize;

        while(retVal == 0 && count < length)
        {
            SerialEEPROM_24Cxx_StartTransfer_Prep(Addr + count, Mem_Type,&serialEeprom_desc);
            if (length - count < page)
            {
                page = length - count;
            }
            retVal = MCHF_I2C_WriteBlock(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,serialEeprom_desc.addr,serialEeprom_desc.addr_size,&buffer[count],page);
            count+=page;
            if (retVal)
            {
                break;
            }
            retVal = SerialEEPROM_24Cxx_ackPolling(Addr,Mem_Type);
        }
    }
    return retVal;
}

uint8_t SerialEEPROM_24Cxx_Detect() {

    uint8_t ser_eeprom_type = EEPROM_SER_UNKNOWN;

    // serial EEPROM init
    if(SerialEEPROM_24Cxx_Read(0,8) > 0xFF)  // Issue with Ser EEPROM, either not available or other problems
	{
		ser_eeprom_type = EEPROM_SER_NONE;             // no serial EEPROM available
		mchf_hw_i2c2_reset();
	}
    else
    {
        if(SerialEEPROM_24Cxx_Read(0,16) != 0xFF)
        {
            ser_eeprom_type = EEPROM_SER_WRONG_SIG;
            // unless we find a correct signature, we have to assume the EEPROM has incorrect/corrupted data

            uint16_t ser_eeprom_type_read = SerialEEPROM_24Cxx_Read(0,8);
            uint16_t ser_eeprom_sig = SerialEEPROM_24Cxx_Read(1,8);

            // all 8 bit (i.e. 256 or 128 Byte) EEPROMS are marked as "too small" during detection
            if(ser_eeprom_sig == SER_EEPROM_IN_USE_TOO_SMALL && ser_eeprom_type_read > 6 && ser_eeprom_type_read < 9)
            {
                ser_eeprom_type = ser_eeprom_type_read;
            }
            else
            {
                ser_eeprom_type_read = SerialEEPROM_24Cxx_Read(0,16);
                ser_eeprom_sig = SerialEEPROM_24Cxx_Read(1,16);

                // we either have a new EEPROM, just being initialized  ( ser_eeprom_sig = SER_EEPROM_IN_USE_NO && valid type) or
                // we have an used EEPROM (ser_eeprom_sig = SER_EEPROM_IN_USE_I2C && valid type)
                // please note, that even though no 16 bit EEPROM gets the "too small" signature written in, these may still be
                // consider too small by the later code. Only EEPROMS which have the "supported" flag set in the descriptor will
                // be consider of sufficient size for actual use
                if((ser_eeprom_sig == SER_EEPROM_IN_USE_I2C || ser_eeprom_sig == SER_EEPROM_IN_USE_NO) && ser_eeprom_type_read < SERIAL_EEPROM_DESC_NUM && ser_eeprom_type_read > 8)
                {
                    ser_eeprom_type = ser_eeprom_type_read;
                }
            }
        }
        else
        {
            const uint8_t testsignature1 = 0x66;
            const uint16_t testaddr1 = 0x0001;

            const uint8_t testsignature2 = 0x77; // has to be different from testsignature 1
            const uint16_t testaddr2 = 0x0100 + testaddr1; // same as before but 0x100 == 256 byte spacing

            uint16_t testbyte0 = SerialEEPROM_24Cxx_Read(testaddr1,8);
            SerialEEPROM_24Cxx_Write(testaddr1,testsignature2,8);

            // first decide if 8 or 16 bit addressable eeprom by trying to read with 8 or 16 bit algorithm
            // if an algorithm succeeds we know the address width
            if(testbyte0 < 0x100 && SerialEEPROM_24Cxx_Read(testaddr1,8) == testsignature2)
            {
                // 8 bit addressing
                SerialEEPROM_24Cxx_Write(testaddr1,testsignature1,8);              // write test signature
                ser_eeprom_type = 7;             // smallest possible 8 bit EEPROM (128Bytes)
                if(SerialEEPROM_24Cxx_Read(testaddr1 + 0x80,8) != testsignature1)
                {
                    ser_eeprom_type = 8;
                }
                SerialEEPROM_24Cxx_Write(0,ser_eeprom_type,8);
                SerialEEPROM_24Cxx_Write(1,SER_EEPROM_IN_USE_TOO_SMALL,8);
            }
            else
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

                    uint16_t testByte1 = SerialEEPROM_24Cxx_Read(testaddr1,16);         // read old content
                    uint16_t testByte2 = SerialEEPROM_24Cxx_Read(testaddr2,16);     // of test bytes

                    if (testByte1 < 0x100 && testByte2 < 0x100) {

                        // successful read from both locations, let us start
                        SerialEEPROM_24Cxx_Write(testaddr1, testsignature1,16);         // write testsignature 1
                        SerialEEPROM_24Cxx_Write(testaddr2, testsignature2,16);         // write testsignature 2
                        if(SerialEEPROM_24Cxx_Read(testaddr1,16) == testsignature1 && SerialEEPROM_24Cxx_Read(testaddr2,16) == testsignature2)
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
                                if(SerialEEPROM_24Cxx_Read(shift_test_addr,16) == testsignature1) {
                                    // now we write test signature 2 to make sure the match was no accident
                                    // i.e. match with same content in EEPROM
                                    SerialEEPROM_24Cxx_Write(shift_test_addr,testsignature2,16);
                                    if(SerialEEPROM_24Cxx_Read(testaddr1,16) == testsignature2) {
                                        // we found the looping,
                                        // now stop checking and set EEPROM type
                                        ser_eeprom_type = 9 + shift;
                                        break;
                                    }
                                    else
                                    {
                                        // oops: just an accident, so we write back old content and continue
                                        SerialEEPROM_24Cxx_Write(shift_test_addr,testsignature1,16);
                                    }
                                }
                            }

                        }
                        SerialEEPROM_24Cxx_Write(testaddr1,testByte1,16);         // write back old data
                        SerialEEPROM_24Cxx_Write(testaddr2,testByte2,16);
                    }
                }
                if (ser_eeprom_type != EEPROM_SER_UNKNOWN) {
                    SerialEEPROM_24Cxx_Write(0,ser_eeprom_type,ser_eeprom_type);
                }
            }
        }
    }
    // just to be save. Never ever deliver a type id outside the array boundaries.
    if (ser_eeprom_type >= SERIAL_EEPROM_DESC_NUM)
    {
        ser_eeprom_type = EEPROM_SER_UNKNOWN;
    }
    return ser_eeprom_type;
}
void  SerialEEPROM_Clear()
{
    SerialEEPROM_24Cxx_Write(0,0xFF,16);
    SerialEEPROM_24Cxx_Write(1,0xFF,16);
}

bool SerialEEPROM_Exists()
{
    return SerialEEPROM_24Cxx_Read(0,8) != 0xFE00;
}


//
// Interface for serial EEPROM functions
//
uint16_t SerialEEPROM_ReadVariable(uint16_t addr, uint16_t *value)      // reference to serial EEPROM read function
{
    uint16_t data;

    data = (uint16_t)(SerialEEPROM_24Cxx_Read(addr*2, ts.ser_eeprom_type)<<8);
    data = data + (uint8_t)(SerialEEPROM_24Cxx_Read(addr*2+1, ts.ser_eeprom_type));
    *value = data;

    return 0;
}

uint16_t SerialEEPROM_UpdateVariable(uint16_t addr, uint16_t value)
{
        uint16_t value_read = 0;
        uint16_t status = SerialEEPROM_ReadVariable(addr,&value_read);
        if  (status != 0 || value_read != value )
        {
            SerialEEPROM_WriteVariable(addr,value);
        }
        return 0;
}

uint16_t SerialEEPROM_WriteVariable(uint16_t addr, uint16_t value)      // reference to serial EEPROM write function, writing unsigned 16 bit
{
    uint8_t lowbyte, highbyte;

    lowbyte = (uint8_t)(value&(0x00FF));
    highbyte = (uint8_t)((value&(0xFF00))>>8);

    SerialEEPROM_24Cxx_Write(addr*2, highbyte, ts.ser_eeprom_type);
    SerialEEPROM_24Cxx_Write(addr*2+1, lowbyte, ts.ser_eeprom_type);

    return 0;
}
