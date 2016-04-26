/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name: flash_if.c                                                          **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		CC BY-NC-SA 3.0                                            **
************************************************************************************/

#include "flash_if.h"
#include "usbh_usr.h"


static uint32_t FLASH_If_GetSectorNumber(uint32_t Address);

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;

void FLASH_If_FlashUnlock(void)
{
    FLASH_Unlock();
}

FlagStatus FLASH_If_ReadOutProtectionStatus(void)
{
    FlagStatus readoutstatus = RESET;
    if (FLASH_OB_GetRDP() == SET)
    {
        readoutstatus = SET;
    }
    else
    {
        readoutstatus = RESET;
    }
    return readoutstatus;
}

/**
  * Erases the required FLASH Sectors.
  * retval Sectors erase status:
  *  0: Erase sectors done with success
  *  1: Erase error
  */
uint32_t FLASH_If_EraseSectors(uint32_t Address)
{
    uint32_t sectorindex, startsector = 0x00;
    FLASH_Status erasestatus = FLASH_COMPLETE;

    /* Erase Flash sectors */
    startsector = FLASH_If_GetSectorNumber(Address);

    sectorindex = startsector;

    /* Erase FLASH sectors to download image */
    while ((erasestatus == FLASH_COMPLETE) && (sectorindex <= FLASH_Sector_11) && (HCD_IsDeviceConnected(&USB_OTG_Core) == 1))
    {
        erasestatus = FLASH_EraseSector(sectorindex, VoltageRange_3);
        sectorindex += 8;
    }

    if ((sectorindex != (FLASH_Sector_11 + 8)) || (erasestatus != FLASH_COMPLETE))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
  * Programs a word at a specified address.
  * Address: specifies the address to be programmed.
  * Data: specifies the data to be programmed.
  * retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  * FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
FLASH_Status FLASH_If_ProgramWord(uint32_t Address, uint32_t Data)
{
    FLASH_Status status = FLASH_COMPLETE;

    status = FLASH_ProgramWord(Address, Data);
    return status;
}

/**
  * Return the Flash sector Number of the address
  * retval The Flash sector Number of the address
  */
static uint32_t FLASH_If_GetSectorNumber(uint32_t Address)
{
    uint32_t sector = 0;

    if(Address < ADDR_FLASH_SECTOR_1 && Address >= ADDR_FLASH_SECTOR_0)
    {
        sector = FLASH_Sector_0;
    }
    else if(Address < ADDR_FLASH_SECTOR_2 && Address >= ADDR_FLASH_SECTOR_1)
    {
        sector = FLASH_Sector_1;
    }
    else if(Address < ADDR_FLASH_SECTOR_3 && Address >= ADDR_FLASH_SECTOR_2)
    {
        sector = FLASH_Sector_2;
    }
    else if(Address < ADDR_FLASH_SECTOR_4 && Address >= ADDR_FLASH_SECTOR_3)
    {
        sector = FLASH_Sector_3;
    }
    else if(Address < ADDR_FLASH_SECTOR_5 && Address >= ADDR_FLASH_SECTOR_4)
    {
        sector = FLASH_Sector_4;
    }
    else if(Address < ADDR_FLASH_SECTOR_6 && Address >= ADDR_FLASH_SECTOR_5)
    {
        sector = FLASH_Sector_5;
    }
    else if(Address < ADDR_FLASH_SECTOR_7 && Address >= ADDR_FLASH_SECTOR_6)
    {
        sector = FLASH_Sector_6;
    }
    else if(Address < ADDR_FLASH_SECTOR_8 && Address >= ADDR_FLASH_SECTOR_7)
    {
        sector = FLASH_Sector_7;
    }
    else if(Address < ADDR_FLASH_SECTOR_9 && Address >= ADDR_FLASH_SECTOR_8)
    {
        sector = FLASH_Sector_8;
    }
    else if(Address < ADDR_FLASH_SECTOR_10 && Address >= ADDR_FLASH_SECTOR_9)
    {
        sector = FLASH_Sector_9;
    }
    else if(Address < ADDR_FLASH_SECTOR_11 && Address >= ADDR_FLASH_SECTOR_10)
    {
        sector = FLASH_Sector_10;
    }
    else
    {
        sector = FLASH_Sector_11;
    }
    return sector;
}
