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

#define RESET 0
#define SET 1

static uint32_t FLASH_If_GetSectorNumber(uint32_t Address);

void FLASH_If_FlashUnlock(void)
{
    HAL_FLASH_Unlock();
}

FlagStatus FLASH_If_ReadOutProtectionStatus(void)
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
uint32_t FLASH_If_EraseSectors(uint32_t Address, uint32_t Length)
{
    FLASH_EraseInitTypeDef flashEraseOp;
    uint32_t sectorError = 0;

    /* Erase Flash sectors */
    uint32_t startsector = FLASH_If_GetSectorNumber(Address);
    uint32_t endsector = FLASH_If_GetSectorNumber(Address+Length);


    /* Erase FLASH sectors to download image */
    flashEraseOp.Sector = startsector;
    flashEraseOp.NbSectors = endsector - startsector + 1;
    flashEraseOp.VoltageRange = VOLTAGE_RANGE_3;
    flashEraseOp.TypeErase = FLASH_TYPEERASE_SECTORS;

    return HAL_FLASHEx_Erase(&flashEraseOp, &sectorError);
}

/**
  * Programs a word at a specified address.
  * Address: specifies the address to be programmed.
  * Data: specifies the data to be programmed.
  * retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  * FLASH_ERROR_WRP, HAL_OK or FLASH_TIMEOUT.
  */
HAL_StatusTypeDef FLASH_If_ProgramWord(uint32_t Address, uint32_t Data)
{
    return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,Address, Data);
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
        sector = FLASH_SECTOR_0;
    }
    else if(Address < ADDR_FLASH_SECTOR_2 && Address >= ADDR_FLASH_SECTOR_1)
    {
        sector = FLASH_SECTOR_1;
    }
    else if(Address < ADDR_FLASH_SECTOR_3 && Address >= ADDR_FLASH_SECTOR_2)
    {
        sector = FLASH_SECTOR_2;
    }
    else if(Address < ADDR_FLASH_SECTOR_4 && Address >= ADDR_FLASH_SECTOR_3)
    {
        sector = FLASH_SECTOR_3;
    }
    else if(Address < ADDR_FLASH_SECTOR_5 && Address >= ADDR_FLASH_SECTOR_4)
    {
        sector = FLASH_SECTOR_4;
    }
    else if(Address < ADDR_FLASH_SECTOR_6 && Address >= ADDR_FLASH_SECTOR_5)
    {
        sector = FLASH_SECTOR_5;
    }
    else if(Address < ADDR_FLASH_SECTOR_7 && Address >= ADDR_FLASH_SECTOR_6)
    {
        sector = FLASH_SECTOR_6;
    }
    else if(Address < ADDR_FLASH_SECTOR_8 && Address >= ADDR_FLASH_SECTOR_7)
    {
        sector = FLASH_SECTOR_7;
    }
    else if(Address < ADDR_FLASH_SECTOR_9 && Address >= ADDR_FLASH_SECTOR_8)
    {
        sector = FLASH_SECTOR_8;
    }
    else if(Address < ADDR_FLASH_SECTOR_10 && Address >= ADDR_FLASH_SECTOR_9)
    {
        sector = FLASH_SECTOR_9;
    }
    else if(Address < ADDR_FLASH_SECTOR_11 && Address >= ADDR_FLASH_SECTOR_10)
    {
        sector = FLASH_SECTOR_10;
    }
    else
    {
        sector = FLASH_SECTOR_11;
    }
    return sector;
}
