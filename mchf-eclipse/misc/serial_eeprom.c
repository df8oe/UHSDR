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
 **  Licence:       CC BY-NC-SA 3.0                                                **
 ************************************************************************************/

// Common
#include "mchf_board.h"
#include "mchf_hw_i2c.h"
#include "mchf_hw_i2c2.h"

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
                .name = "Not used"
        },
        // 2
        {
                .size = 0,
                .supported = false,
                .pagesize = 0,
                .name = "Not used"
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
                .name = "24xx01/128B"
        },
        // 8
        {
                .size = 256,
                .supported = false,
                .pagesize = 8,
                .name = "24xx02/256B"
        },
        // 9
        {
                .size = 2*256,
                .supported = false,
                .pagesize = 16,
                .name = "24xx04/512B"
        },
        // 10
        {
                .size = 1*1024,
                .supported = false,
                .pagesize = 16,
                .name = "24xx08/1K"
        },
        // 11
        {
                .size = 2*1024,
                .supported = false,
                .pagesize = 16,
                .name = "24xx16/2K"
        },
        // 12
        {
                .size = 4*1024,
                .supported = false,
                .pagesize = 32,
                .name = "24xx32/4K"
        },
        // 13
        {
                .size = 8*1024,
                .supported = false,
                .pagesize = 32,
                .name = "24xx64/8K"
        },
        // 14
        {
                .size = 16*1024,
                .supported = false,
                .pagesize = 64,
                .name = "24xx128/16K"
        },
        // 15
        {
                .size = 32*1024,
                .supported = false,
                .pagesize = 64,
                .name = "24xx256/32K"
        },
        // 16
        {
                .size = 64*1024,
                .supported = true,
                .pagesize = 128,
                .name = "24xx512/64K"
        },
        // 17
        {
                .size = 128*1024,
                .supported = true,
                .pagesize = 128,
                .name = "24xx1025/64K"
        },
        // 18
        {
                .size = 128 * 1024,
                .supported = true,
                .pagesize = 128,
                .name = "24xx1026/128K"
        },
        // 19
        {
                .size = 2048*1024,
                .supported = true,
                .pagesize = 256,
                .name = "24CM02/2M"
        }

};

typedef struct
{
    uint8_t devaddr;
    uint8_t addr[2]; // 0 -> upper or single, 1 -> lower, second;
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

    if(Mem_Type == 17 && *Addr_ptr > 0xFFFF)
    {
        *devaddr_ptr = *devaddr_ptr + 8;            // 24LC1025
        *Addr_ptr = *Addr_ptr - 0x10000;
    }
    if(Mem_Type == 18 && *Addr_ptr > 0xFFFF)
    {
        *devaddr_ptr = *devaddr_ptr + 2;            // 24LC1026
        *Addr_ptr = *Addr_ptr - 0x10000;
    }
    if(Mem_Type == 19)
    {
        if(*Addr_ptr > 0xFFFF && *Addr_ptr < 0x20000)
        {
            *devaddr_ptr = *devaddr_ptr + 4;            // 24CM02
            *Addr_ptr = *Addr_ptr - 0x10000;
        }
        if(*Addr_ptr > 0x1FFFF && *Addr_ptr < 0x30000)
        {
            *devaddr_ptr = *devaddr_ptr + 2;            // 24CM02
            *Addr_ptr = *Addr_ptr - 0x20000;
        }
        if(*Addr_ptr > 0x2FFFF && *Addr_ptr < 0x40000)
        {
            *devaddr_ptr = *devaddr_ptr + 3;            // 24CM02
            *Addr_ptr = *Addr_ptr - 0x30000;
        }
    }
}


static uint16_t SerialEEPROM_24Cxx_ackPollingSinglePoll(uint32_t Addr, uint8_t Mem_Type)
{
    uint8_t devaddr;

    SerialEEPROM_24Cxx_AdjustAddrs(Mem_Type,&devaddr,&Addr);

    I2C_GenerateSTART(SERIALEEPROM_I2C, ENABLE);
    I2C_EventCompleteOrReturn(I2C2,I2C_EVENT_MASTER_MODE_SELECT, 0xFF00);
    // Test on I2C2 EV5, Start transmitted successfully and clear it
    // Send Memory device slave Address for write
    I2C_Send7bitAddress(SERIALEEPROM_I2C, devaddr, I2C_Direction_Transmitter);
    // Test on I2C2 EV6 and clear it
    I2C_EventCompleteOrReturn(I2C2,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 0xFD00)

    return 0;
}
static uint16_t SerialEEPROM_24Cxx_ackPolling(uint32_t Addr, uint8_t Mem_Type)
{
    int i = 10;
    uint16_t retVal;

    for (; i; i--)
    {
        if ((retVal = SerialEEPROM_24Cxx_ackPollingSinglePoll(Addr, Mem_Type)) != 0xFD00)
        {
            break;
        }
    }
    return retVal;
}

static void SerialEEPROM_24Cxx_StartTransfer_Prep(uint32_t Addr, uint8_t Mem_Type, SerialEEPROM_24CXX_Descriptor* eeprom_desc_ptr)
{
    SerialEEPROM_24Cxx_AdjustAddrs(Mem_Type,&eeprom_desc_ptr->devaddr,&Addr);

    if (Mem_Type > 8)
    {
        eeprom_desc_ptr->addr[1] = (uint8_t)((0x00FF)&(Addr));
        eeprom_desc_ptr->addr[0] = (uint8_t)((0x00FF)&((Addr)>>8));
        eeprom_desc_ptr->addr_size = 2;
    }
    else
    {
        eeprom_desc_ptr->addr[0] = (uint8_t)((0x00FF)&Addr);
        eeprom_desc_ptr->addr_size = 1;
    }
}

uint16_t SerialEEPROM_24Cxx_Write(uint32_t Addr, uint8_t Data, uint8_t Mem_Type)
{
    SerialEEPROM_24Cxx_StartTransfer_Prep(Addr, Mem_Type,&serialEeprom_desc);
    uint16_t retVal = MCHF_I2C_WriteRegister(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,&serialEeprom_desc.addr[0],serialEeprom_desc.addr_size,Data);

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
    uint16_t retVal = MCHF_I2C_ReadRegister(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,&serialEeprom_desc.addr[0],serialEeprom_desc.addr_size,&value);
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
            SerialEEPROM_24Cxx_StartTransfer_Prep(Addr + count, Mem_Type,&serialEeprom_desc);
            retVal = MCHF_I2C_ReadBlock(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,&serialEeprom_desc.addr[0],serialEeprom_desc.addr_size,&buffer[count],page);
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
    uint16_t retVal = 0xFFFF;
    if (Mem_Type < SERIAL_EEPROM_DESC_NUM) {
        uint32_t page, count;
        count = 0;

        page =SerialEEPROM_eepromTypeDescs[Mem_Type].pagesize;

        while(count < length)
        {
            SerialEEPROM_24Cxx_StartTransfer_Prep(Addr + count, Mem_Type,&serialEeprom_desc);
            retVal = MCHF_I2C_WriteBlock(SERIALEEPROM_I2C,serialEeprom_desc.devaddr,&serialEeprom_desc.addr[0],serialEeprom_desc.addr_size,&buffer[count],page);
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

    uint8_t ser_eeprom_type = 0xFF;

    // serial EEPROM init
    //  Write_24Cxx(0,0xFF,16);     //enable to reset EEPROM and force new copyvirt2ser
    if(SerialEEPROM_24Cxx_Read(0,8) > 0xFF)  // Issue with Ser EEPROM, either not available or other problems
        ser_eeprom_type = EEPROM_SER_NONE;             // no serial EEPROM available
    else
    {
        if(SerialEEPROM_24Cxx_Read(0,16) != 0xFF)
        {
            if(SerialEEPROM_24Cxx_Read(0,8) > 6 && SerialEEPROM_24Cxx_Read(0,8) < 9 && SerialEEPROM_24Cxx_Read(1,8) == 0x10)
            {
                ser_eeprom_type = SerialEEPROM_24Cxx_Read(0,8);
            }
            else
            {
                ser_eeprom_type = SerialEEPROM_24Cxx_Read(0,16);
            }
        }
        else
        {
            SerialEEPROM_24Cxx_Write(10,0xdd,8);
            // first decide if 8 or 16 bit addressable eeprom by trying to read with 8 or 16 bit algorithm
            // if an algorithm succeeds we know the address width
            if(SerialEEPROM_24Cxx_Read(10,8) == 0xdd)
            {
                // 8 bit addressing
                SerialEEPROM_24Cxx_Write(3,0x99,8);              // write test signature
                ser_eeprom_type = 7;             // smallest possible 8 bit EEPROM (128Bytes)
                if(SerialEEPROM_24Cxx_Read(0x83,8) != 0x99)
                {
                    ser_eeprom_type = 8;
                }
                SerialEEPROM_24Cxx_Write(0,ser_eeprom_type,8);
                SerialEEPROM_24Cxx_Write(1,0x10,8);
            }
            else
            {
                // 16 bit addressing
                if(SerialEEPROM_24Cxx_Read(0x10000,17) < 0x100)
                {
                    ser_eeprom_type = 17;            // 24LC1025
                    SerialEEPROM_24Cxx_Write(0,17,16);
                }
                if(SerialEEPROM_24Cxx_Read(0x10000,18) < 0x100)
                {
                    ser_eeprom_type = 18;            // 24LC1026
                    SerialEEPROM_24Cxx_Write(0,18,16);
                }
                if(SerialEEPROM_24Cxx_Read(0x10000,19) < 0x100)
                {
                    ser_eeprom_type = 19;            // 24CM02
                    SerialEEPROM_24Cxx_Write(0,19,16);
                }
                if(ser_eeprom_type < 17)
                {
                    SerialEEPROM_24Cxx_Write(3,0x66,16);         // write testsignature 1
                    SerialEEPROM_24Cxx_Write(0x103,0x77,16);         // write testsignature 2
                    if(SerialEEPROM_24Cxx_Read(3,16) == 0x66 && SerialEEPROM_24Cxx_Read(0x103,16) == 0x77)
                    {
                        // 16 bit addressing
                        ser_eeprom_type = 9;         // smallest possible 16 bit EEPROM
                        if(SerialEEPROM_24Cxx_Read(0x203,16) != 0x66)
                        {
                            ser_eeprom_type = 10;
                        }
                        if(SerialEEPROM_24Cxx_Read(0x403,16) != 0x66)
                        {
                            ser_eeprom_type = 11;
                        }
                        if(SerialEEPROM_24Cxx_Read(0x803,16) != 0x66)
                        {
                            ser_eeprom_type = 12;
                        }
                        if(SerialEEPROM_24Cxx_Read(0x1003,16) != 0x66)
                        {
                            ser_eeprom_type = 13;
                        }
                        if(SerialEEPROM_24Cxx_Read(0x2003,16) != 0x66)
                        {
                            ser_eeprom_type = 14;
                        }
                        if(SerialEEPROM_24Cxx_Read(0x4003,16) != 0x66)
                        {
                            ser_eeprom_type = 15;
                        }
                        if(SerialEEPROM_24Cxx_Read(0x8003,16) != 0x66)
                        {
                            ser_eeprom_type = 16;
                        }
                        SerialEEPROM_24Cxx_Write(0,ser_eeprom_type,16);
                    }
                }
            }
        }
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
