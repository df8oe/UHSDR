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
#include "bootloader_main.h"

#include <unistd.h>


FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
char USBDISKPath[4];          /* USB Host logical drive path */

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
        mchfBl_PinToggle(LEDGREEN);
        tick = now + 1024;
    }
    if(mchfBl_ButtonGetState(BUTTON_POWER) == 0)
    {
        mcHF_PowerHoldOff();
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
        __IO uint32_t was_download = (mchfBl_ButtonGetState(BUTTON_BANDM) == 0);
        /* Reads all flash memory */
        COMMAND_UPLOAD();

        /* Check if BAND- Button was pressed */
        if (was_download)
        {
            /* Writes Flash memory */
            COMMAND_DOWNLOAD();
        }

        mchfBl_PinOff(BACKLIGHT);

        /* Waiting User Button Released */
        while ((mchfBl_ButtonGetState(BUTTON_BANDM) == 0))
        {}

        /* Waiting User Button Pressed or usb drive removed. If drive is removed, we go straight to reboot */
        while ((mchfBl_ButtonGetState(BUTTON_BANDM) == 1) && mcHF_USBConnected() != 0)
        {
            /* switch off if power button is pressed */
            if(mchfBl_ButtonGetState(BUTTON_POWER) == 0)
            {
                mcHF_PowerHoldOff();
            }
        }

        /* Waiting User Button Released or usb drive removed.  If drive is removed, we go straight to reboot */
        while ((mchfBl_ButtonGetState(BUTTON_BANDM) == 0) && mcHF_USBConnected() != 0)
        {}

        /* Jumps to user application code located in the internal Flash memory */
        COMMAND_ResetMCU(0x55);
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
    mchfBl_ButtonInit(BUTTON_BANDM, BUTTON_MODE_GPIO);
    mchfBl_ButtonInit(BUTTON_POWER, BUTTON_MODE_GPIO);
    mchfBl_ButtonInit(BUTTON_BANDP, BUTTON_MODE_GPIO);

    mchfBl_LEDInit(LEDGREEN);
    mchfBl_LEDInit(LEDRED);
    mchfBl_LEDInit(PWR_HOLD);
    mchfBl_LEDInit(BACKLIGHT);
}

/**
 * @brief jump to a STM32 Application by giving the start address of the ISR Vector structure of that application
 */

__attribute__ ( ( naked ) ) void mchfBl_JumpToApplication(uint32_t ApplicationAddress)
{
    uint32_t* const APPLICATION_PTR = (uint32_t*)ApplicationAddress;

    if ( ( APPLICATION_PTR[0] & 0x2FF00000 ) == 0x20000000)
    {
        __set_MSP(APPLICATION_PTR[0]);
        /* Jump to user application */
        ((pFunction) APPLICATION_PTR[1])();
    }
}

int bootloader_main()
{
    /* initialization */
    BSP_Init();
    mcHF_PowerHoldOff();


// *(uint32_t*)(SRAM2_BASE+5) = 0x29;	// signature for DF8OE development features
// *(uint32_t*)(SRAM2_BASE+10) = 0x29;	// signature for special beta-testing features
    if( *(uint32_t*)(SRAM2_BASE) != 0x55)		// no reboot requested?
    {
        // we wait for a longer time
        HAL_Delay(300);
    }
    mcHF_PowerHoldOn();

    if (mchfBl_ButtonGetState(BUTTON_BANDP) == 0)
    {
        // BANDM pressed, DFU boot requested
        COMMAND_ResetMCU(0x99);
    }
    /* Test if BAND- button on mchf is NOT pressed */
    else if (mchfBl_ButtonGetState(BUTTON_BANDM) == 1)
    {
        /* Check Vector Table: Test if user code is programmed starting from address
           "APPLICATION_ADDRESS"
           We do that be reading the stack pointer value stored at APPLICATION_ADDRESS
           and roughly check if it points to a RAM SPACE address
         */
        mchfBl_JumpToApplication(APPLICATION_ADDRESS);
        BootFail_Handler(2);
        // never reached, is endless loop
    }

    /* Init upgrade mode display */
    mchfBl_PinOn(BACKLIGHT);
    // now give user a chance to let go off the BAND- button
    HAL_Delay(3000);
    return 0;
}


extern ApplicationTypeDef Appli_state;

void BL_Application()
{
    switch(Appli_state)
    {
    case APPLICATION_START:
      BL_MSC_Application();
      Appli_state = APPLICATION_IDLE;
      break;

    case APPLICATION_IDLE:
    default:
      break;

    }

    BL_Idle_Application();
}


void mchfBl_CheckAndGoForDfuBoot()
{

    if( *(uint32_t*)(SRAM2_BASE) == 0x99)
    {
        *(uint32_t*)(SRAM2_BASE) = 0;
        __HAL_REMAPMEMORY_SYSTEMFLASH();
        mchfBl_JumpToApplication(0);
        // start the STM32F4xx bootloader at address 0x00000000;
    }
}
