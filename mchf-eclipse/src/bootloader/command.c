/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
 ******************************************************************************
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    28-October-2011
 * @brief   This file provides all the IAP command functions.
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

/* Includes ------------------------------------------------------------------*/
#include "command.h"
#include "stm32f4xx.h"
#include "fatfs.h"
#include "mchf_boot_hw.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define UPLOAD_FILENAME            "0:mchfold.bin"
#define DOWNLOAD_FILENAME          "0:mchf.bin"
#define VERSION                    "Version: 2.1.3-HAL"
#define AUTHOR                     "Author: DF8OE"

#define BUFFER_SIZE        ((uint16_t)512*64)

// 64k flash are in use for bootloader and config memory

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t RAM_Buf[BUFFER_SIZE] =
{
        0x00
};
static uint32_t TmpProgramCounter = 0x00, TmpReadSize = 0x00 , RamAddress = 0x00;
static uint32_t LastPGAddress = APPLICATION_ADDRESS;

static FIL file;
static FIL fileR;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

// #define STM_EVAL_LEDOff(a)
// #define STM_EVAL_LEDOn(a)
// #define STM_EVAL_LEDToggle(a)



void Wait(int time)
{
    HAL_Delay(time);
    // we show the error message until user presses power...
    if(STM_EVAL_PBGetState(BUTTON_POWER) == 0)
    {
        STM_EVAL_LEDOn(PWR_HOLD);
    }
}


// USB error handling
void FlashFail_Handler(mchf_bootloader_error_t redCount)
{
    STM_EVAL_LEDOff(LEDGREEN);
    STM_EVAL_LEDOff(LEDRED);

    while(1)
    {
        STM_EVAL_LEDOff(BACKLIGHT);
        Wait(600);
        for(int i = 0; i < redCount; i++)
        {
            STM_EVAL_LEDOff(LEDRED);
            Wait(300);
            STM_EVAL_LEDOn(LEDRED);
            Wait(300);
            STM_EVAL_LEDOff(LEDRED);
            Wait(300);
        }
        Wait(600);
        STM_EVAL_LEDOn(BACKLIGHT);
        Wait(900);
    }
}


// USB error handling
void BootFail_Handler(uint8_t count)
{
    STM_EVAL_LEDOff(BACKLIGHT);
    Wait(700);

    while(1)
    {
        for(uint8_t i = 0; i < count; i++)
        {
            Wait(300);
            STM_EVAL_LEDOn(BACKLIGHT);
            Wait(300);
            STM_EVAL_LEDOff(BACKLIGHT);
            Wait(700);
        }
        Wait(700);
    }
}


/**
 * @brief IAP Read all flash memory
 * @param  None
 * @retval None
 */
void COMMAND_UPLOAD(void)
{
    __IO uint32_t address = APPLICATION_ADDRESS;
    __IO uint32_t counterread = 0x00;

    uint32_t tmpcounter = 0x00, indexoffset = 0x00;
    FlagStatus readoutstatus = !0;
    UINT bytesWritten;
    FRESULT res = FR_OK;

    f_unlink(VERSION);
    f_unlink(AUTHOR);
    /* green LED on command upload */
    STM_EVAL_LEDOn(LEDGREEN);
    /* Get the read out protection status */
    readoutstatus = FLASH_If_ReadOutProtectionStatus();
    if (readoutstatus == 0)
    {
        /* Remove UPLOAD file if exist on flash disk */
        f_unlink (UPLOAD_FILENAME);

        /* Init written byte counter */
        indexoffset = (APPLICATION_ADDRESS - USER_FLASH_STARTADDRESS);

        /* Open binary file to write on it */
        res = f_open(&file, UPLOAD_FILENAME, FA_CREATE_ALWAYS | FA_WRITE);
        if (res == FR_OK)
        {
            /* Read flash memory */
            while ((indexoffset != USER_FLASH_SIZE() && res == FR_OK))
            {
                for (counterread = 0; counterread < BUFFER_SIZE; counterread++)
                {
                    /* Check the read bytes versus the end of flash */
                    if (indexoffset + counterread != USER_FLASH_SIZE())
                    {
                        tmpcounter = counterread;
                        RAM_Buf[tmpcounter] = (*(uint8_t*)(address++));
                    }
                    /* In this case all flash was read */
                    else
                    {
                        break;
                    }
                }
                /* Write buffer to file */
                res = f_write (&file, RAM_Buf, BUFFER_SIZE, &bytesWritten);

                /* Number of byte written  */
                indexoffset = indexoffset + counterread;
            }

            /* Close file and filesystem */
            f_close (&file);
        }
        if (res != FR_OK)
        {
            FlashFail_Handler(BL_ERR_WRITEDISK);
        }
    }
    else
    {
        FlashFail_Handler(BL_ERR_FLASHPROTECT);
    }
}

/**
 * @brief  IAP write memory
 * @param  None
 * @retval None
 */
void COMMAND_DOWNLOAD(void)
{
    /* Flash unlock */
    FLASH_If_FlashUnlock();

    /* Reading for flash active: Red LED on */
    STM_EVAL_LEDOn(LEDRED);
    /* Open the binary file to be downloaded */
    if (f_open(&fileR, DOWNLOAD_FILENAME, FA_READ) == FR_OK)
    {
        uint32_t res;
        /* Erase FLASH sectors to download image */
        if (fileR.fsize > USER_FLASH_SIZE())
        {
            FlashFail_Handler(BL_ERR_FLASHTOOSMALL);
        }
        if ( (res = FLASH_If_EraseSectors(APPLICATION_ADDRESS,fileR.fsize)) != HAL_OK)
        {
            /* Flash erase error */
            FlashFail_Handler(BL_ERR_FLASHERASE);
        }
        /* Program flash memory */
        COMMAND_ProgramFlashMemory();

        /* Close file and filesystem */
        f_close (&fileR);
        ///f_mount(0, NULL);
    }
    else
    {
        /* the binary file is not available */
        FlashFail_Handler(BL_ERR_NOIMAGE);
    }

    FLASH_If_FlashLock();

}

/**
 * @brief  IAP jump to user program
 * @param  None
 * @retval None
 */
void COMMAND_ResetMCU(void)
{
    *(__IO uint32_t*)(SRAM2_BASE) = 0x55;
    /* Software reset */
    NVIC_SystemReset();
}

/**
 * @brief  Programs the internal Flash memory
 * @param  None
 * @retval None
 */
void COMMAND_ProgramFlashMemory(void)
{
    __IO uint32_t programcounter = 0x00;
    uint8_t readflag = TRUE;
    uint16_t BytesRead;

    /* RAM Address Initialization */
    RamAddress = (uint32_t) & RAM_Buf;

    /* Erase address init */
    LastPGAddress = APPLICATION_ADDRESS;

    /* While file still contain data */
    while ((readflag == TRUE))
    {
        /* Read maximum 512 Kbyte from the selected file */
        f_read (&fileR, RAM_Buf, BUFFER_SIZE, (void *)&BytesRead);

        /* Temp variable */
        TmpReadSize = BytesRead;

        /* The read data < "BUFFER_SIZE" Kbyte */
        if (TmpReadSize < BUFFER_SIZE)
        {
            readflag = FALSE;
        }

        /* Program flash memory */
        for (programcounter = TmpReadSize; programcounter != 0; programcounter -= 4)
        {
            TmpProgramCounter = programcounter;
            /* Write word into flash memory */
            if (FLASH_If_ProgramWord((LastPGAddress - TmpProgramCounter + TmpReadSize), \
                    *(__IO uint32_t *)(RamAddress - programcounter + TmpReadSize)) != HAL_OK)
            {
                /* Flash programming error */
                FlashFail_Handler(BL_ERR_FLASHPROG);
            }
        }
        /* Update last programmed address value */
        LastPGAddress = LastPGAddress + TmpReadSize;
    }
}

/**
 * @}
 */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
