/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/
#include "uhsdr_board_config.h"

#if defined(STM32F7)
    #include "stm32f7xx_hal_flash.h"
#elif defined(STM32H7)
    #include "stm32h7xx_hal_flash.h"
#elif defined(STM32F4)
    #include "stm32f4xx_hal_flash.h"
#else
    #error Unknown processor
#endif

#include "flash_if.h"


#define FLASH_NO_SECTOR (0xFFFFFFFF)

/* Base address of the Flash sectors */
typedef struct
{
    uint32_t addr;
} flash_sectorinfo_t;
// this struct can be used to store more information
// such as bank numbers, sector numbers etc. if flash layout gets more complex
// right now not yet used

static const flash_sectorinfo_t flash_sector_addr[] = {
#if defined(STM32F4) || defined(STM32F7)
{ 0x08000000 }, /* Base @ of Sector 0, 16 Kbyte */
{ 0x08004000 }, /* Base @ of Sector 1, 16 Kbyte */
{ 0x08008000 }, /* Base @ of Sector 2, 16 Kbyte */
{ 0x0800C000 }, /* Base @ of Sector 3, 16 Kbyte */
{ 0x08010000 }, /* Base @ of Sector 4, 64 Kbyte */
{ 0x08020000 }, /* Base @ of Sector 5, 128 Kbyte */
{ 0x08040000 }, /* Base @ of Sector 6, 128 Kbyte */
{ 0x08060000 }, /* Base @ of Sector 7, 128 Kbyte */
{ 0x08080000 }, /* Base @ of Sector 8, 128 Kbyte */
{ 0x080A0000 }, /* Base @ of Sector 9, 128 Kbyte */
{ 0x080C0000 }, /* Base @ of Sector 10, 128 Kbyte */
{ 0x080E0000 }, /* Base @ of Sector 11, 128 Kbyte */
{ 0x08100000 }, /* Base @ of Sector 12, 128 Kbyte */
{ 0x08104000 }, /* Base @ of Sector 13, 16 Kbyte */
{ 0x08108000 }, /* Base @ of Sector 14, 16 Kbyte */
{ 0x0810C000 }, /* Base @ of Sector 15, 16 Kbyte */
{ 0x08110000 }, /* Base @ of Sector 16, 64 Kbyte */
{ 0x08120000 }, /* Base @ of Sector 17, 128 Kbyte */
{ 0x08140000 }, /* Base @ of Sector 18, 128 Kbyte */
{ 0x08160000 }, /* Base @ of Sector 19, 128 Kbyte */
{ 0x08180000 }, /* Base @ of Sector 20, 128 Kbyte */
{ 0x081A0000 }, /* Base @ of Sector 21, 128 Kbyte */
{ 0x081C0000 }, /* Base @ of Sector 22, 128 Kbyte */
{ 0x081E0000 }, /* Base @ of Sector 23, 128 Kbyte */
{ 0x08200000 }, /* End+1@ of Sector 23, 128 Kbyte */
#elif defined(STM32H7)
{ 0x08000000 }, /* Base @ of Sector 0, bank 1, 128 Kbyte */
{ 0x08020000 }, /* Base @ of Sector 1, 128 Kbyte */
{ 0x08040000 }, /* Base @ of Sector 2, 128 Kbyte */
{ 0x08060000 }, /* Base @ of Sector 3, 128 Kbyte */
{ 0x08080000 }, /* Base @ of Sector 4, 128 Kbyte */
{ 0x080A0000 }, /* Base @ of Sector 5, 128 Kbyte */
{ 0x080C0000 }, /* Base @ of Sector 6, 128 Kbyte */
{ 0x080E0000 }, /* Base @ of Sector 7, 128 Kbyte */
{ 0x08100000 }, /* Base @ of Sector 0, bank 2, 128 Kbyte */
{ 0x08120000 }, /* Base @ of Sector 1, 128 Kbyte */
{ 0x08140000 }, /* Base @ of Sector 2, 128 Kbyte */
{ 0x08160000 }, /* Base @ of Sector 3, 128 Kbyte */
{ 0x08180000 }, /* Base @ of Sector 4, 128 Kbyte */
{ 0x081A0000 }, /* Base @ of Sector 5, 128 Kbyte */
{ 0x081C0000 }, /* Base @ of Sector 6, 128 Kbyte */
{ 0x081E0000 }, /* Base @ of Sector 7, 128 Kbyte */
{ 0x08200000 }, /* End+1@ of Sector 7, 128 Kbyte */
#endif
};


static const uint32_t flash_sector_max = sizeof(flash_sector_addr)/sizeof(flash_sectorinfo_t)-1;
// we have one extra entry at the end, hence we need to reduce number of sectors by one

#define RESET 0
#define SET 1

/**
  * Return the Flash sector Number of the address
  * retval The Flash sector Number of the address
  */
static uint32_t flashIf_GetSectorInfoIndex(uint32_t Address)
{
    uint32_t sector = FLASH_NO_SECTOR; // index of invalid sector, returned if address not in flash

    for (int idx = 0; idx < flash_sector_max; idx++)
    {
        if (Address >= flash_sector_addr[idx].addr && Address < flash_sector_addr[idx+1].addr)
        {
            sector = idx;
            break;
        }
    }
    return sector;
}

static uint32_t flashIf_GetSectorNumber(uint32_t idx)
{
    uint32_t retval = FLASH_NO_SECTOR;
    if (idx < flash_sector_max)
    {
#if defined(STM32F4) || defined(STM32F7)
        retval = idx; // idx == sector number
#elif defined(STM32H7)
        retval = idx % 8; // 2x8 sectors
#endif
    }
    return retval;
}
#if defined(STM32H7)
static uint32_t flashIf_GetSectorBank(uint32_t idx)
{
    uint32_t retval = FLASH_NO_SECTOR;
    if (idx < flash_sector_max)
    {
        retval = idx < 8? FLASH_BANK_1:FLASH_BANK_2; // 2x8 sectors
    }
    return retval;
}
#endif


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
    uint32_t startsector = flashIf_GetSectorInfoIndex(Address);
    uint32_t endsector = flashIf_GetSectorInfoIndex(Address+Length-1);

    if (startsector != FLASH_NO_SECTOR && endsector != FLASH_NO_SECTOR)

    {
        retval = HAL_OK;

        for (int idx = startsector; retval == HAL_OK && idx <= endsector; idx++)
        {
            /* Erase FLASH sectors to download image */
            flashEraseOp.Sector = flashIf_GetSectorNumber(idx);
            flashEraseOp.NbSectors = 1;
            flashEraseOp.VoltageRange = VOLTAGE_RANGE_3;
            flashEraseOp.TypeErase = FLASH_TYPEERASE_SECTORS;
#if defined(STM32H7)
            flashEraseOp.Banks = flashIf_GetSectorBank(idx);
#endif
            retval = HAL_FLASHEx_Erase(&flashEraseOp, &sectorError);
        }
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
HAL_StatusTypeDef flashIf_Program256Bit(uint32_t Address, uint32_t Data[8])
{
    HAL_StatusTypeDef retval = HAL_OK;
#ifndef STM32H7
    for (int i=0; retval == HAL_OK && i < 8; i++)
    {
        retval = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,Address+(i*4), Data[i]);
    }
#else
    retval = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,Address, (uint32_t)&Data[0]);
#endif
    return retval;
}

