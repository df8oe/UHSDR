/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name: main.c                                                              **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                  **
************************************************************************************/

#include "flash_if.h"
#include "mchf_boot_hw.h"
#include "command.h"
#include "fatfs.h"
#include <unistd.h>

pFunction Jump_To_Application;
uint32_t JumpAddress;

__IO uint32_t TimingDelay;
__IO uint32_t UploadCondition = 0x00;

FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
char USBDISKPath[4];          /* USB Host logical drive path */


int BL_MSC_Application(void)
{
    TimingDelay = 300;
    UploadCondition = 0x01;
    /* Register the file system object to the FatFs module */
    if(f_mount(&USBDISKFatFs, (TCHAR const*)USBDISKPath, 0) != FR_OK)
    {
        /* FatFs Initialization Error */
        UWP_Fail_Handler();
    }
    else
    {


        /* Reads all flash memory */
        COMMAND_UPLOAD();

        /* Initialize User_Button on mchf in the GPIO Mode ----------*/
        STM_EVAL_PBInit(BUTTON_BANDM, BUTTON_MODE_GPIO);
        STM_EVAL_PBInit(BUTTON_POWER, BUTTON_MODE_GPIO);

        /* Check if User Button is already pressed */
        if ((TimingDelay == 0x00) && (UploadCondition == 0x01))
        {
            /* Writes Flash memory */
            STM_EVAL_LEDOn(BLON);
            COMMAND_DOWNLOAD();

        }
        else
        {
            /* Set Off Orange LED : Download Done */
            /*      STM_EVAL_LEDOff(LED3);  */
            /* Set ON Green LED: Waiting User button pressed */
            /*      STM_EVAL_LEDOn(LED4);  */
        }

        STM_EVAL_LEDOff(BLON);
        UploadCondition = 0x00;

        /* Waiting User Button Released */
        while ((STM_EVAL_PBGetState(BUTTON_BANDM) == 0))
        {}

        /* Waiting User Button Pressed */
        while ((STM_EVAL_PBGetState(BUTTON_BANDM) == 1))
        {
            /* switch off if power button is pressed */
            if(STM_EVAL_PBGetState(BUTTON_POWER) == 0)
            {
                STM_EVAL_LEDOn(ON);
            }
        }

        /* Waiting User Button Released */
        while ((STM_EVAL_PBGetState(BUTTON_BANDM) == 0))
        {}

        /* Jumps to user application code located in the internal Flash memory */
        COMMAND_JUMP();
    }

    // this below is a trick to get sbrk() linked in at the right time so that
    // we don't get a link error
    sbrk(0);
    return(0);
}


void BSP_Init(void)
{
    /* Initialize LEDs and User_Button on mchf --------------------*/
    STM_EVAL_PBInit(BUTTON_BANDM, BUTTON_MODE_GPIO);
    STM_EVAL_PBInit(BUTTON_POWER, BUTTON_MODE_GPIO);

    STM_EVAL_LEDInit(LEDGREEN);
    STM_EVAL_LEDInit(LEDRED);
    STM_EVAL_LEDInit(ON);
    STM_EVAL_LEDInit(BLON);
}

int bootloader_main(void)
{
    /* initialization */
    BSP_Init();
    STM_EVAL_LEDOn(ON);

    double i,border;

// *(__IO uint32_t*)(SRAM2_BASE+5) = 0x29;	// signature for DF8OE development features
// *(__IO ;uint32_t*)(SRAM2_BASE+10) = 0x29;	// signature for special beta-testing features
    if( *(__IO uint32_t*)(SRAM2_BASE) != 0x55)		// reboot requested?
    {
        border = 300000;
    }
    else
    {
        border = 1000;								// yes: only short delay so that power is hold
    }

    for (i = 0; i < border; i++)
    {}

    STM_EVAL_LEDOff(ON);


    /* Flash unlock */
    FLASH_If_FlashUnlock();

    /* Test if BAND- button on mchf is pressed */
    if (STM_EVAL_PBGetState(BUTTON_BANDM) == 1)
    {
        /* Check Vector Table: Test if user code is programmed starting from address
           "APPLICATION_ADDRESS" */
        if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
        {
            /* Jump to user application */
            JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
            Jump_To_Application = (pFunction) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
            Jump_To_Application();
        }
    }

    /* Init upgrade mode display */
    STM_EVAL_LEDOn(BLON);

    return 0;
}


/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    {
        TimingDelay--;
    }
}

void HAL_SYSTICK_Callback()
{
    static __IO uint8_t Counter = 0x00;

    if (UploadCondition != 0x00)
    {
        /* Check if User button Pressed */
        if (STM_EVAL_PBGetState(BUTTON_BANDM) == 0)
        {
            if (TimingDelay != 0x00)
            {
                TimingDelay--;
            }
            else
            {
                if (Counter < 100)
                {
                    Counter++;
                }
                else
                {
                    Counter = 0x00;
                }
            }
        }
        else
        {
            if (TimingDelay != 0x00)
            {
                UploadCondition = 0x00;
            }
        }
    }
    else
    {
        TimingDelay_Decrement();
    }
}
