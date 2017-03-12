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
#include "usb_host.h"

#include <unistd.h>


FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
char USBDISKPath[4];          /* USB Host logical drive path */

static void mcHF_PowerHoldOff()
{
    STM_EVAL_LEDOn(PWR_HOLD);
}

static void mcHF_PowerHoldOn()
{
    STM_EVAL_LEDOff(PWR_HOLD);
}

extern USBH_HandleTypeDef hUsbHostHS;
static uint8_t mcHF_USBConnected()
{
    return hUsbHostHS.device.is_connected;
}

void BL_Idle_Application(void)
{
    static uint32_t tick;
    uint32_t now = HAL_GetTick();

    if (now >= tick)
    {
        STM_EVAL_LEDToggle(LEDGREEN);
        tick = now + 1024;
    }
}

int BL_MSC_Application(void)
{

    /* Register the file system object to the FatFs module */
    if(f_mount(&USBDISKFatFs, (TCHAR const*)USBDISKPath, 0) != FR_OK)
    {
        /* FatFs Initialization Error */
        FlashFail_Handler(BL_ERR_USBPROBLEM);
    }
    else
    {
        // just before we start, we check the BANDM button.
        // If still pressed, we will also flash the memory afer reading it
        __IO uint32_t was_download = (STM_EVAL_PBGetState(BUTTON_BANDM) == 0);
        /* Reads all flash memory */
        COMMAND_UPLOAD();

        /* Check if BAND- Button was pressed */
        if (was_download)
        {
            /* Writes Flash memory */
            COMMAND_DOWNLOAD();
        }

        STM_EVAL_LEDOff(BACKLIGHT);

        /* Waiting User Button Released */
        while ((STM_EVAL_PBGetState(BUTTON_BANDM) == 0))
        {}

        /* Waiting User Button Pressed or usb drive removed. If drive is removed, we go straight to reboot */
        while ((STM_EVAL_PBGetState(BUTTON_BANDM) == 1) && mcHF_USBConnected() != 0)
        {
            /* switch off if power button is pressed */
            if(STM_EVAL_PBGetState(BUTTON_POWER) == 0)
            {
                mcHF_PowerHoldOff();
            }
        }

        /* Waiting User Button Released or usb drive removed.  If drive is removed, we go straight to reboot */
        while ((STM_EVAL_PBGetState(BUTTON_BANDM) == 0) && mcHF_USBConnected() != 0)
        {}

        /* Jumps to user application code located in the internal Flash memory */
        COMMAND_ResetMCU();
    }

    // this below is a trick to get sbrk() linked in at the right time so that
    // we don't get a link error
    sbrk(0);
    return(0);
}


void BSP_Init(void)
{
    // we assume that the clock for all required GPIO Ports has been turn on!!

    /* Initialize LEDs and User_Button on mchf --------------------*/
    STM_EVAL_PBInit(BUTTON_BANDM, BUTTON_MODE_GPIO);
    STM_EVAL_PBInit(BUTTON_POWER, BUTTON_MODE_GPIO);

    STM_EVAL_LEDInit(LEDGREEN);
    STM_EVAL_LEDInit(LEDRED);
    STM_EVAL_LEDInit(PWR_HOLD);
    STM_EVAL_LEDInit(BACKLIGHT);
}



int bootloader_main()
{
    /* initialization */
    BSP_Init();
    mcHF_PowerHoldOff();


// *(__IO uint32_t*)(SRAM2_BASE+5) = 0x29;	// signature for DF8OE development features
// *(__IO ;uint32_t*)(SRAM2_BASE+10) = 0x29;	// signature for special beta-testing features
    if( *(__IO uint32_t*)(SRAM2_BASE) != 0x55)		// no reboot requested?
    {
        // we wait for a longer time
        HAL_Delay(300);
    }
    mcHF_PowerHoldOn();

    /* Test if BAND- button on mchf is NOT pressed */
    if (STM_EVAL_PBGetState(BUTTON_BANDM) == 1)
    {
        /* Check Vector Table: Test if user code is programmed starting from address
           "APPLICATION_ADDRESS"
           We do that be reading the stack pointer value stored at APPLICATION_ADDRESS
           and roughly check if it points to a RAM SPACE address
            */

        if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FF00000 ) == 0x20000000)
        {
            pFunction Jump_To_Application;
            uint32_t JumpAddress;

            /* Jump to user application */
            JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
            Jump_To_Application = (pFunction) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
            Jump_To_Application();
        }
        while(1)
        {
            BootFail_Handler(2);
        }
    }

    /* Init upgrade mode display */
    STM_EVAL_LEDOn(BACKLIGHT);
    // now give user a chance to let go off the BAND- button
    HAL_Delay(3000);
    return 0;
}
