/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
  ******************************************************************************
  * @file    EEPROM_Emulation/src/eeprom.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    10-October-2011
  * @brief   This file provides all the EEPROM emulation firmware functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/** @addtogroup EEPROM_Emulation
  * @{
  */


// Common
#include "mchf_board.h"
#include "stm32f4xx_hal_flash_ex.h"

/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define VAR_ADDR_START  (0xAA01)
// this value is required to remain unchanged in order to not break existing mcHF flash configuration
// readings. It is otherwise just an arbitrary number.

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Global variable used to store variable value in read sequence */
static uint16_t DataVar = 0;

/* Virtual address defined by the user: 0xFFFF value is prohibited */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static HAL_StatusTypeDef Flash_Format();
static uint16_t Flash_FindPage(uint8_t Operation);
static uint16_t Flash_WriteVariableToPage(uint16_t VirtAddress, uint16_t Data, uint8_t pageType);
static uint16_t Flash_PageTransfer(uint16_t VirtAddress);


HAL_StatusTypeDef Flash_Erase(uint32_t sector)
{
FLASH_EraseInitTypeDef flashEraseOp;
uint32_t sectorError = 0;

flashEraseOp.Sector = sector;
flashEraseOp.NbSectors = 1;
flashEraseOp.VoltageRange = VOLTAGE_RANGE_3;
flashEraseOp.TypeErase = FLASH_TYPEERASE_SECTORS;

return HAL_FLASHEx_Erase(&flashEraseOp, &sectorError);

}
/**
 * This code encapsulates the former direct use of mapping table
 * Since we never used the mapping table except for mapping to an offset,
 * we now use code to convert the id (start from 0) to addresses  (starting from VAR_ADDR_START)
 * This allows full backward compatibility of stored settings.
 */
uint16_t Flash_GetVirtAddrForId(uint16_t id) {
    return VAR_ADDR_START + id;
}

static bool Flash_PageIsErased(uint8_t page)
{
    bool retval = true;
    uint32_t* pagePtr = (uint32_t*)(page==PAGE0?PAGE0_BASE_ADDRESS:PAGE1_BASE_ADDRESS);
    for (uint16_t idx = 0; idx < PAGE_SIZE/sizeof(uint32_t);idx++)
    {
        if (pagePtr[idx] != 0xFFFFFFFF)
        {
            retval =false;
            break;
        }
    }
    return retval;
}
HAL_StatusTypeDef Flash_Check_And_EraseIfNeeded(uint8_t page)
{
    HAL_StatusTypeDef retval = HAL_OK;
    if (Flash_PageIsErased(page) == false)
    {
        retval =Flash_Erase(page == PAGE0? PAGE0_ID : PAGE1_ID);
    }
    return retval;
}


static uint16_t Flash_TransferFullPage(uint8_t fromPage, uint8_t toPage, bool skip, uint16_t skipAddress)
{
    uint32_t toPageBaseAddress = toPage==PAGE0?PAGE0_BASE_ADDRESS:PAGE1_BASE_ADDRESS;
    // uint32_t fromPageBaseAddress = fromPage==PAGE0?PAGE0_BASE_ADDRESS:PAGE1_BASE_ADDRESS;

    uint16_t retval = HAL_OK;

    /* Transfer data from Page0 to Page1 */
    for (uint16_t VarIdx = 0; retval == HAL_OK && VarIdx < NB_OF_VAR; VarIdx++)
    {
        uint16_t virtAddress = Flash_GetVirtAddrForId(VarIdx);

        if ((*(__IO uint16_t*)(toPageBaseAddress + 6)) != virtAddress && (!skip || virtAddress != skipAddress))
        {
            /* Read the last variables' updates */
            uint16_t ReadStatus = Flash_ReadVariable(virtAddress, &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1)
            {
                /* Transfer the variable to the Page1 */
                retval = Flash_WriteVariableToPage(virtAddress, DataVar, RECEIVE_WRITE_PAGE);
                /* If program operation was failed, a Flash error code is returned */
                if (retval != HAL_OK)
                {
                    break;
                }
            }
        }
    }
    /* Mark toPage as valid */
    retval = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,toPageBaseAddress, VALID_PAGE);
    /* If program operation was failed, a Flash error code is returned */
    if (retval == HAL_OK)
    {
        /* Erase Page0 */
        retval = Flash_Erase(fromPage);
        /* If erase operation was failed, a Flash error code is returned */
    }
    return retval;
}


/**
  * @brief  Restore the pages to a known good state in case of page's status
  *   corruption after a power loss.
  * @param  None.
  * @retval - Flash error code: on write Flash error
  *         - HAL_OK: on success
  */
uint16_t Flash_InitA(void)
{

    /*   1
     *     0: V  E  R  ?
     *   V    EB C0 T0 E0
     *   E    C1 V0 E0 E0
     *   R    T1 EB EB EB
     *   ?    E1 E1 EB EB
     *
     *  Cx: Check Erase x
     *  Ex: Erase x 0,1,Both
     *  Tx: Transfer to x
     *  V0: Make Page 0 valid page
     */
    uint16_t PageStatus0, PageStatus1;
    uint16_t retval = 0x80;

    /* Get Page0 status */
    PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);
    /* Get Page1 status */
    PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

    /* Check for invalid header states and repair if necessary */
    switch (PageStatus0)
    {
    case ERASED:
        if (PageStatus1 == VALID_PAGE) /* Page0 erased, Page1 valid */
        {
            /* Erase Page0 */
            retval = Flash_Check_And_EraseIfNeeded(PAGE0);
        }
        else if (PageStatus1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
        {
            /* Erase Page0 */
            retval = Flash_Check_And_EraseIfNeeded(PAGE0);
            /* If erase operation was failed, a Flash error code is returned */
            if (retval == HAL_OK)
            {
                /* Mark Page1 as valid */
                retval = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,PAGE1_BASE_ADDRESS, VALID_PAGE);
                /* If program operation was failed, a Flash error code is returned */
            }
        }
        break;

    case RECEIVE_DATA:
        if (PageStatus1 == VALID_PAGE) /* Page0 receive, Page1 valid */
        {
            retval = Flash_TransferFullPage(PAGE1_ID, PAGE0_ID, false, 0);
            /* Transfer data from Page1 to Page0 */
        }
        else if (PageStatus1 == ERASED) /* Page0 receive, Page1 erased */
        {
            /* Erase Page1 */
            retval = Flash_Check_And_EraseIfNeeded(PAGE0);
            /* If erase operation was failed, a Flash error code is returned */
            if (retval == HAL_OK)
            {
                /* Mark Page0 as valid */
                retval = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,PAGE0_BASE_ADDRESS, VALID_PAGE);
            }
            /* If program operation was failed, a Flash error code is returned */
        }
        break;

    case VALID_PAGE:
        if (PageStatus1 == VALID_PAGE) /* Invalid state -> format eeprom */
        {
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            retval = Flash_Format();
        }
        else if (PageStatus1 == ERASED) /* Page0 valid, Page1 erased */
        {
            /* Erase Page1 */
            retval = Flash_Check_And_EraseIfNeeded(PAGE1);
            /* If erase operation was failed, a Flash error code is returned */
        }
        else if (PageStatus1 == RECEIVE_DATA)/* Page0 valid, Page1 receive */
        {
            retval = Flash_TransferFullPage(PAGE0_ID, PAGE1_ID, false, 0);
        }
        else
        {
            /* Invalid state on page1 -> format eeprom page 1 */
            retval = Flash_Check_And_EraseIfNeeded(PAGE1);
        }
        break;

    default:
        if (PageStatus1 == VALID_PAGE) /* Invalid state on page0 -> format eeprom page 0 */
        {
            /* Erase Page0 */
            retval = Flash_Check_And_EraseIfNeeded(PAGE0);
        }
    }

    if (retval == 0x80)
    {
        /* In case of real trouble we try to recover by erasing the whole flash memory */
        retval = Flash_Format();
    }
    return retval;
}

// Proxy
uint16_t Flash_Init(void)
{
    uint16_t res;

    HAL_FLASH_Unlock();
    res = Flash_InitA();
    HAL_FLASH_Lock();

    return res;
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to
  *   the passed virtual address
  * @param  VirtAddress: Variable virtual address
  * @param  Data: Global variable contains the read variable value
  * @retval Success or error status:
  *           - 0: if variable was found
  *           - 1: if the variable was not found
  *           - NO_VALID_PAGE: if no valid page was found.
  */
uint16_t Flash_ReadVariable(uint16_t addr, uint16_t* value)
{
    uint16_t VirtAddress = Flash_GetVirtAddrForId(addr);
    uint16_t ValidPage;
    uint16_t AddressValue = 0x5555, ReadStatus = 1;
    uint32_t Address = EEPROM_START_ADDRESS, PageStartAddress = EEPROM_START_ADDRESS;

    /* Get active Page for read operation */
    ValidPage = Flash_FindPage(READ_FROM_VALID_PAGE);

    /* Check if there is no valid page */
    if (ValidPage == NO_VALID_PAGE)
    {
        return  NO_VALID_PAGE;
    }

    /* Get the valid Page start Address */
    PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

    /* Get the valid Page end Address */
    Address = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ValidPage) * PAGE_SIZE));

    /* Check each active page address starting from end */
    while (Address > (PageStartAddress + 2))
    {
        /* Get the current location content to be compared with virtual address */
        AddressValue = (*(__IO uint16_t*)Address);

        /* Compare the read address with the virtual address */
        if (AddressValue == VirtAddress)
        {
            /* Get content of Address-2 which is variable value */
            *value = (*(__IO uint16_t*)(Address - 2));

            /* In case variable value is read, reset ReadStatus flag */
            ReadStatus = 0;

            break;
        }
        else
        {
            /* Next address location */
            Address = Address - 4;
        }
    }

    /* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
    return ReadStatus;
}

/**
  * @brief  Writes/updates variable data in EEPROM if necessary.
  * @param  VirtAddress: Variable virtual address
  * @param  Data: 16 bit data to be written
  * @retval Success or error status:
  *           - HAL_OK: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
uint16_t Flash_UpdateVariable(uint16_t addr, uint16_t value)
{
    uint16_t DataRead = 0;
    uint16_t Status = 0;
    Status = Flash_ReadVariable(addr,&DataRead);
    // the variable was found and the data content is  equal to the new value
    // -> no need to write it again
    if (Status == 0 && DataRead == value)
    {
        Status = HAL_OK;
    }
    else
    {
        Status = Flash_WriteVariable(addr, value);
    }
    return Status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  * @param  VirtAddress: Variable virtual address
  * @param  Data: 16 bit data to be written
  * @retval Success or error status:
  *           - HAL_OK: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
uint16_t Flash_WriteVariable(uint16_t addr, uint16_t value)
{
    uint16_t retval = 0;
    uint16_t VirtAddress = Flash_GetVirtAddrForId(addr);

    HAL_FLASH_Unlock();

    /* Write the variable virtual address and value in the EEPROM */
    retval = Flash_WriteVariableToPage(VirtAddress, value,WRITE_TO_VALID_PAGE);

    /* In case the EEPROM active page is full */
    if (retval == PAGE_FULL)
    {
        /* Perform Page transfer and retry write*/
        retval = Flash_PageTransfer(VirtAddress);
        if (retval == HAL_OK)
        {
            retval = Flash_WriteVariableToPage(VirtAddress, value,WRITE_TO_VALID_PAGE);
        }
    }

    HAL_FLASH_Lock();

    /* Return last operation status */
    return retval;
}

/**
  * @brief  Erases PAGE and PAGE1 and writes VALID_PAGE header to PAGE
  * @param  None
  * @retval Status of the last operation (Flash write or erase) done during
  *         EEPROM formating
  */
static HAL_StatusTypeDef Flash_Format(void)
{
    HAL_StatusTypeDef FlashStatus;

    /* Erase Page0 */
    FlashStatus = Flash_Erase(PAGE0_ID);

    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != HAL_OK)
    {
        return FlashStatus;
    }

    /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
    FlashStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,PAGE0_BASE_ADDRESS, VALID_PAGE);

    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != HAL_OK)
    {
        return FlashStatus;
    }

    /* Erase Page1 */
    FlashStatus = Flash_Erase(PAGE1_ID);

    /* Return Page1 erase operation status */
    return FlashStatus;
}

/**
  * @brief  Find valid Page for write or read operation
  * @param  Operation: operation to achieve on the valid page.
  *   This parameter can be one of the following values:
  *     @arg READ_FROM_VALID_PAGE: read operation from valid page
  *     @arg WRITE_IN_VALID_PAGE: write operation from valid page
  * @retval Valid page number (PAGE or PAGE1) or NO_VALID_PAGE in case
  *   of no valid page was found
  */
static uint16_t Flash_FindPage(uint8_t Operation)
{
    uint16_t PageStatus0, PageStatus1;

    /* Get Page0 actual status */
    PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);

    /* Get Page1 actual status */
    PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

    uint16_t retval = NO_VALID_PAGE;

    /* Write or read operation */
    switch (Operation)
    {
    case RECEIVE_WRITE_PAGE:   /* ---- Write operation ---- */
        if (PageStatus1 == VALID_PAGE)
        {
            /* Page0 receiving data */
            if (PageStatus0 == RECEIVE_DATA)
            {
                retval = PAGE0;         /* Page0 valid */
            }
        }
        else if (PageStatus0 == VALID_PAGE)
        {
            /* Page1 receiving data */
            if (PageStatus1 == RECEIVE_DATA)
            {
                retval = PAGE1;         /* Page1 valid */
            }
        }
        break;

    case READ_FROM_VALID_PAGE:  /* ---- Read operation ---- */
        if (PageStatus0 == VALID_PAGE)
        {
            retval = PAGE0;           /* Page0 valid */
        }
        else if (PageStatus1 == VALID_PAGE)
        {
            retval = PAGE1;           /* Page1 valid */
        }
        break;
    case WRITE_TO_VALID_PAGE:  /* ---- Read operation ---- */
        if (PageStatus0 == VALID_PAGE)
        {
            retval = PAGE0;           /* Page0 valid */
        }
        else if (PageStatus1 == VALID_PAGE)
        {
            retval = PAGE1;           /* Page1 valid */
        }
    }

    return retval;
}

/**
  * @brief  Verify if active page is full and Writes variable in EEPROM.
  * @param  VirtAddress: 16 bit virtual address of the variable
  * @param  Data: 16 bit data to be written as variable value
  * @retval Success or error status:
  *           - HAL_OK: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t Flash_WriteVariableToPage(uint16_t VirtAddress, uint16_t Data, uint8_t pageType)
{
    uint16_t retval = PAGE_FULL;

    uint16_t ReceivePage;
    uint32_t Address = EEPROM_START_ADDRESS, PageEndAddress = EEPROM_START_ADDRESS+PAGE_SIZE;

    /* Get valid Page for write operation */
    ReceivePage = Flash_FindPage(pageType);

    /* Check if there is no valid page */
    if (ReceivePage == NO_VALID_PAGE)
    {
        retval = NO_VALID_PAGE;
    }
    else
    {
        /* Get the valid Page start Address */
        Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ReceivePage * PAGE_SIZE));

        /* Get the valid Page end Address */
        PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ReceivePage) * PAGE_SIZE));

        /* Check each active page address starting from begining */
        for (;Address < PageEndAddress;Address += 4)
        {
            /* Verify if Address and Address+2 contents are 0xFFFFFFFF */
            if ((*(__IO uint32_t*)Address) == 0xFFFFFFFF)
            {
                /* Set variable data */
                retval = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Address, Data);

                /* If program operation was okay, proceed */
                if (retval == HAL_OK)
                {
                    /* Set variable virtual address */
                    retval = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address + 2, VirtAddress);
                }

                break;
            }
        }
    }

    return retval;
}

/**
  * @brief  Transfers last updated variables data from the full Page to
  *   an empty one.
  * @param  VirtAddress: 16 bit virtual address of the variable
  * @param  Data: 16 bit data to be written as variable value
  * @retval Success or error status:
  *           - HAL_OK: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t Flash_PageTransfer(uint16_t VirtAddress)
{
    uint32_t NewPageAddress;
    uint16_t ValidPage;
    uint16_t NewPage;
    uint16_t retval = HAL_OK;

    /* Get active Page for read operation */
    ValidPage = Flash_FindPage(READ_FROM_VALID_PAGE);

    if (ValidPage == PAGE1)       /* Page1 valid */
    {
        /* New page address where variable will be moved to */
        NewPageAddress = PAGE0_BASE_ADDRESS;
        NewPage = PAGE0;
    }
    else if (ValidPage == PAGE0)  /* Page0 valid */
    {
        /* New page address  where variable will be moved to */
        NewPageAddress = PAGE1_BASE_ADDRESS;
        NewPage = PAGE1;
    }
    else
    {
        retval = NO_VALID_PAGE;       /* No valid Page */
    }

    if (retval == HAL_OK )
    {
        /* Set the new Page status to RECEIVE_DATA status */
        retval = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,NewPageAddress, RECEIVE_DATA);
        /* If program operation was failed, a Flash error code is returned */
        if (retval == HAL_OK)
        {
            retval = Flash_TransferFullPage(ValidPage,NewPage,true,VirtAddress);
        }
    }
    /* Return last operation flash status */
    return retval;
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
