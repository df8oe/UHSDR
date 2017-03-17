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
**  Licence:		GNU GPLv3                                                  **
************************************************************************************/

#include "flash_if.h"
#include "stm32f4xx_hal_flash_ex.h"

/* Base address of the Flash sectors */

static const uint32_t flash_sector_max = 24;
static const uint32_t flash_sector_addr[] = {
0x08000000, /* Base @ of Sector 0, 16 Kbyte */
0x08004000, /* Base @ of Sector 1, 16 Kbyte */
0x08008000, /* Base @ of Sector 2, 16 Kbyte */
0x0800C000, /* Base @ of Sector 3, 16 Kbyte */
0x08010000, /* Base @ of Sector 4, 64 Kbyte */
0x08020000, /* Base @ of Sector 5, 128 Kbyte */
0x08040000, /* Base @ of Sector 6, 128 Kbyte */
0x08060000, /* Base @ of Sector 7, 128 Kbyte */
0x08080000, /* Base @ of Sector 8, 128 Kbyte */
0x080A0000, /* Base @ of Sector 9, 128 Kbyte */
0x080C0000, /* Base @ of Sector 10, 128 Kbyte */
0x080E0000, /* Base @ of Sector 11, 128 Kbyte */
0x08100000, /* Base @ of Sector 12, 128 Kbyte */
0x08104000, /* Base @ of Sector 13, 16 Kbyte */
0x08108000, /* Base @ of Sector 14, 16 Kbyte */
0x0810C000, /* Base @ of Sector 15, 16 Kbyte */
0x08110000, /* Base @ of Sector 16, 64 Kbyte */
0x08120000, /* Base @ of Sector 17, 128 Kbyte */
0x08140000, /* Base @ of Sector 18, 128 Kbyte */
0x08160000, /* Base @ of Sector 19, 128 Kbyte */
0x08180000, /* Base @ of Sector 20, 128 Kbyte */
0x081A0000, /* Base @ of Sector 21, 128 Kbyte */
0x081C0000, /* Base @ of Sector 22, 128 Kbyte */
0x081E0000, /* Base @ of Sector 23, 128 Kbyte */
0x08200000, /* End+1@ of Sector 23, 128 Kbyte */
};



#define RESET 0
#define SET 1
#define FLASH_NO_SECTOR (0xFFFFFFFF)

static uint32_t FLASH_If_GetSectorNumber(uint32_t Address);

void flashIf_FlashUnlock()
{
    HAL_FLASH_Unlock();
}

void flashIf_FlashLock()
{
    HAL_FLASH_Lock();
}


FlagStatus flashIf_ReadOutProtectionStatus()
{
    FLASH_OBProgramInitTypeDef OBInit;

    HAL_FLASHEx_OBGetConfig(&OBInit);
    FlagStatus readoutstatus = RESET;

    if (OBInit.RDPLevel != OB_RDP_LEVEL0)
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
  *  HAL_OK: Erase sectors done with success
  *  Anything else: Erase error
  */
uint32_t flashIf_EraseSectors(uint32_t Address, uint32_t Length)
{
    FLASH_EraseInitTypeDef flashEraseOp;
    uint32_t sectorError = 0;
    uint32_t retval = HAL_ERROR;

    /* Erase Flash sectors */
    uint32_t startsector = FLASH_If_GetSectorNumber(Address);
    uint32_t endsector = FLASH_If_GetSectorNumber(Address+Length-1);

    if (startsector != FLASH_NO_SECTOR && endsector != FLASH_NO_SECTOR)

    {
        /* Erase FLASH sectors to download image */
        flashEraseOp.Sector = startsector;
        flashEraseOp.NbSectors = endsector - startsector + 1;
        flashEraseOp.VoltageRange = VOLTAGE_RANGE_3;
        flashEraseOp.TypeErase = FLASH_TYPEERASE_SECTORS;

        retval = HAL_FLASHEx_Erase(&flashEraseOp, &sectorError);
    }
    return retval;
}

/**
  * Programs a word at a specified address.
  * Address: specifies the address to be programmed.
  * Data: specifies the data to be programmed.
  * retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  * FLASH_ERROR_WRP, HAL_OK or FLASH_TIMEOUT.
  */
HAL_StatusTypeDef flashIf_ProgramWord(uint32_t Address, uint32_t Data)
{
    return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,Address, Data);
}

/**
  * Return the Flash sector Number of the address
  * retval The Flash sector Number of the address
  */
static uint32_t FLASH_If_GetSectorNumber(uint32_t Address)
{
    uint32_t sector = FLASH_NO_SECTOR;

    for (int idx = 0; idx < flash_sector_max; idx++)
    {
        if (Address >= flash_sector_addr[idx] && Address < flash_sector_addr[idx+1])
        {
            sector = idx;
            break;
        }
    }
    return sector;
}
