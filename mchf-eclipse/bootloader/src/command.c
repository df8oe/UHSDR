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


/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define UPLOAD_FILENAME            "0:mchfold.bin"
#define DOWNLOAD_FILENAME          "0:mchf.bin"
#define VERSION                    "Version: 1.0"
#define AUTHOR                     "Author: DF8OE"

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t RAM_Buf[BUFFER_SIZE] =
{
    0x00
};
static uint32_t TmpProgramCounter = 0x00, TmpReadSize = 0x00 , RamAddress = 0x00;
static uint32_t LastPGAddress = APPLICATION_ADDRESS;

extern FATFS fatfs;
extern FIL file;
extern FIL fileR;
extern DIR dir;
extern FILINFO fno;


extern USB_OTG_CORE_HANDLE          USB_OTG_Core;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

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
    FlagStatus readoutstatus = SET;
    uint16_t bytesWritten;

    f_unlink(VERSION);
    f_unlink(AUTHOR);
    /* green LED on command upload */
    STM_EVAL_LEDOn(LEDGREEN);
    /* Get the read out protection status */
    readoutstatus = FLASH_If_ReadOutProtectionStatus();
    if (readoutstatus == RESET)
    {
        /* Remove UPLOAD file if exist on flash disk */
        f_unlink (UPLOAD_FILENAME);

        /* Init written byte counter */
        indexoffset = (APPLICATION_ADDRESS - USER_FLASH_STARTADDRESS);

        /* Open binary file to write on it */
        if ((HCD_IsDeviceConnected(&USB_OTG_Core) == 1) && (f_open(&file, UPLOAD_FILENAME, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK))
        {
            /* Read flash memory */
            while ((indexoffset != USER_FLASH_SIZE) && (HCD_IsDeviceConnected(&USB_OTG_Core) == 1))
            {
                for (counterread = 0; counterread < BUFFER_SIZE; counterread++)
                {
                    /* Check the read bytes versus the end of flash */
                    if (indexoffset + counterread != USER_FLASH_SIZE)
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
                f_write (&file, RAM_Buf, BUFFER_SIZE, (void *)&bytesWritten);

                /* Number of byte written  */
                indexoffset = indexoffset + counterread;
            }

            /* Close file and filesystem */
            f_close (&file);
        }
    }
    else
    {
        UNS_Fail_Handler();
    }
}

/**
  * @brief  IAP write memory
  * @param  None
  * @retval None
  */
void COMMAND_DOWNLOAD(void)
{
    /* Reading for flash active: Red LED on */
    STM_EVAL_LEDOn(LEDRED);
    /* Open the binary file to be downloaded */
    if (f_open(&fileR, DOWNLOAD_FILENAME, FA_READ) == FR_OK)
    {
        if (fileR.fsize > USER_FLASH_SIZE)
        {
            /* Toggle backlight and LED red in infinite loop: No available Flash memory size for
               the binary file */
            FME_Fail_Handler();
        }
        else
        {

            /* Erase FLASH sectors to download image */
            if (FLASH_If_EraseSectors(APPLICATION_ADDRESS) != 0x00)
            {
                /* Toggle backlight, green + red LED in infinite loop: Flash erase error */
                FEE_Fail_Handler();
            }
            /* Program flash memory */
            COMMAND_ProgramFlashMemory();

            /* Close file and filesystem */
            f_close (&fileR);
            f_mount(0, NULL);
        }
    }
    else
    {
        /* Toggle backlight and red LED in counterwise infinite loop: the binary file is not available */
        FNF_Fail_Handler();
    }
}

/**
  * @brief  IAP jump to user program
  * @param  None
  * @retval None
  */
void COMMAND_JUMP(void)
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
    while ((readflag == TRUE) && (HCD_IsDeviceConnected(&USB_OTG_Core) == 1))
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
                                     *(__IO uint32_t *)(RamAddress - programcounter + TmpReadSize)) != FLASH_COMPLETE)
            {
                /* Toggle all LEDs and backlight in infinite loop: Flash programming error */
                FPE_Fail_Handler();
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
