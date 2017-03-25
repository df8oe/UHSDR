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
#include "ui_lcd_hy28.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define UPLOAD_FILENAME            "0:mchfold.bin"
#define DOWNLOAD_FILENAME          "0:mchf.bin"
#define VERSION                    "Version: 3.2.0"
#define AUTHOR                     "Author: DF8OE"

#define BUFFER_SIZE        ((uint16_t)512*64)

// 64k flash are in use for bootloader and config memory

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t RAM_Buf[BUFFER_SIZE] =
{
        0x00
};

static FIL file;
static FIL fileR;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void Wait(int time)
{
    HAL_Delay(time);
    // we show the error message until user presses power...
    if(mchfBl_ButtonGetState(BUTTON_POWER) == 0)
    {
        mcHF_PowerHoldOff();
    }
}

static const char* error_help[] =
{
        "No error",
        "USB problem",
        "mchf.bin not found",
        "Flash memory too small",
        "Problem writing mchfold.bin",
        "Problem reading mchf.bin",
        "Flash programming problem",
        "Flash erase problem",
        "Flash write protected",
};

// USB error handling
void FlashFail_Handler(mchf_bootloader_error_t redCount)
{
    mchfBl_PinOff(LEDGREEN);
    mchfBl_PinOff(LEDRED);

    char txt[16] = "Error Code  X";
    txt[12] = redCount + '0';

    BL_PrintLine("");
    BL_PrintLine(txt);
    BL_PrintLine(error_help[redCount]);


    while(1)
    {
        if (mchf_display.DeviceCode == 0x0000)
        {
            mchfBl_PinOff(BACKLIGHT);
        }
        Wait(600);
        for(int i = 0; i < redCount; i++)
        {
            mchfBl_PinOff(LEDRED);
            Wait(300);
            mchfBl_PinOn(LEDRED);
            Wait(300);
            mchfBl_PinOff(LEDRED);
            Wait(300);
        }
        Wait(600);
        mchfBl_PinOn(BACKLIGHT);
        Wait(900);
    }
}


// USB error handling
void BootFail_Handler(uint8_t count)
{
    mchfBl_PinOff(BACKLIGHT);
    Wait(700);

    while(1)
    {
        for(uint8_t i = 0; i < count; i++)
        {
            Wait(300);
            mchfBl_PinOn(BACKLIGHT);
            Wait(300);
            mchfBl_PinOff(BACKLIGHT);
            Wait(700);
        }
        Wait(700);
    }
}


/**
 * @brief  Programs the internal Flash memory
 */
mchf_bootloader_error_t COMMAND_ProgramFlashMemory()
{
    mchf_bootloader_error_t retval = BL_ERR_NONE;

    uint8_t readflag = TRUE;
    uint32_t LastPGAddress = APPLICATION_ADDRESS;

    do
    {
        UINT BytesRead;

        /* Read maximum 32 Kbyte from the selected file */
        if(f_read (&fileR, RAM_Buf, BUFFER_SIZE, &BytesRead) != FR_OK)
        {
            retval = BL_ERR_READDISK;
        }
        else
        {
            /* The read data < "BUFFER_SIZE" Kbyte */
            if (BytesRead < BUFFER_SIZE)
            {
                readflag = FALSE;
            }

            /* Program flash memory */
            for (   uint32_t programcounter = 0;
                    programcounter < BytesRead;
                    programcounter+=4,LastPGAddress += 4)
            {
                uint32_t* RAM_Address = (uint32_t*)&RAM_Buf[programcounter];

                if (flashIf_ProgramWord(LastPGAddress, *RAM_Address) != HAL_OK)
                {
                    /* Flash programming error */
                    retval = BL_ERR_FLASHPROG;
                }
            }
        }
    } while ((readflag == TRUE) && retval == BL_ERR_NONE);

    return retval;
}

/**
 * @brief read flash and write to image on disk
 */
void COMMAND_UPLOAD(void)
{

    UINT bytesWritten;
    FRESULT res = FR_OK;

    f_unlink(VERSION);
    f_unlink(AUTHOR);
    /* green LED on command upload */
    mchfBl_PinOn(LEDGREEN);
    /* Get the read out protection status */
    FlagStatus readoutstatus = flashIf_ReadOutProtectionStatus();
    if (readoutstatus == 0)
    {
        /* Remove UPLOAD file if exist on flash disk */
        f_unlink (UPLOAD_FILENAME);

        /* Init written byte counter */
        uint32_t offset = 0;
        uint8_t* address = (uint8_t*)APPLICATION_ADDRESS;


        /* Open binary file to write on it */
        res = f_open(&file, UPLOAD_FILENAME, FA_CREATE_ALWAYS | FA_WRITE);
        if (res == FR_OK)
        {
            uint32_t counterread = 0;
            /* Read flash memory */
            while ((offset != flashIf_userFlashSize() && res == FR_OK))
            {
                for (counterread = 0; counterread < BUFFER_SIZE && offset != flashIf_userFlashSize(); counterread++,offset++)
                {
                        RAM_Buf[counterread] = address[offset];
                }
                /* Write buffer to file */
                res = f_write (&file, RAM_Buf, counterread, &bytesWritten);
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
 * @brief  read file and flash it
 */
void COMMAND_DOWNLOAD(void)
{
    mchf_bootloader_error_t retval = BL_ERR_NONE;

    /* Flash unlock */
    flashIf_FlashUnlock();

    /* Reading for flash active: Red LED on */
    mchfBl_PinOn(LEDRED);

    /* Open the binary file to be downloaded */
    if (f_open(&fileR, DOWNLOAD_FILENAME, FA_READ) == FR_OK)
    {
        /* Erase FLASH sectors to download image */
        if (fileR.fsize > flashIf_userFlashSize())
        {
            retval = BL_ERR_FLASHTOOSMALL;
        } else if ( flashIf_EraseSectors(APPLICATION_ADDRESS,fileR.fsize) != HAL_OK)
        {
            /* Flash erase error */
            retval = BL_ERR_FLASHERASE;
        } else
        {
            /* Program flash memory */
            retval = COMMAND_ProgramFlashMemory();
        }

        /* Close file and filesystem */
        f_close (&fileR);
        ///f_mount(0, NULL);
    }
    else
    {
        /* the binary file is not available */
        retval = BL_ERR_NOIMAGE;
    }

    flashIf_FlashLock();

    if (retval != BL_ERR_NONE)
    {
        FlashFail_Handler(retval);
    }
}

/**
 * @brief  reset to restart mchf
 */
void COMMAND_ResetMCU(uint32_t code)
{
    *(__IO uint32_t*)(SRAM2_BASE) = code;
    /* Software reset */
    NVIC_SystemReset();
}
