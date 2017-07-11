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
#include "uhsdr_boot_hw.h"
#include "hardware/uhsdr_board.h"
#include "command.h"
#include "fatfs.h"
#include "usb_host.h"
#include "bootloader_main.h"
#include "ui_lcd_hy28.h"

#include "dma.h"
#include "spi.h"
#include "gpio.h"

#ifdef STM32F7
#include "stm32f7xx_hal_flash_ex.h"
#else
#include "stm32f4xx_hal_flash_ex.h"
#endif



#include <unistd.h>


FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
char USBDISKPath[4];          /* USB Host logical drive path */

extern USBH_HandleTypeDef hUsbHostHS;
static uint8_t mcHF_USBConnected()
{
    return hUsbHostHS.device.is_connected;
}


static const char*  bl_help[] =
{
        TRX_NAME " Bootloader",
        "Firmware Name: " BASE_FILE,
        "USB Drive Mode",
        "Release Band- to skip firmware update",
        "",
        "DFU Update Mode",
        "Keep Power pressed until finish.",
        "Release Band+ to start DFU Mode.",
        "Screen may go white. This is ok.",
        "PC should recognize new USB device.",
};


static void BL_DisplayInit()
{
#ifdef STM32F4
    MX_DMA_Init();
    MX_SPI2_Init();
#endif
    MX_GPIO_Init();

#ifdef STM32F4
    // we need to set the touch screen CS signal to high, otherwise SPI displays
    // will not be detectable
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Pull     = GPIO_PULLUP;
    GPIO_InitStructure.Speed    = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Mode     = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pin = TP_CS;

    HAL_GPIO_Init(TP_CS_PIO, &GPIO_InitStructure);

    GPIO_SetBits(TP_CS_PIO, TP_CS);
#endif

    UiLcdHy28_Init();
    UiLcdHy28_LcdClear(Black);
    mchfBl_PinOn(BACKLIGHT);

}

// bin rw: 0 1 2 3 4
// dfu     0 1 5 4 6 7 8 9

void BL_InfoScreen()
{
    BL_DisplayInit();
    BL_PrintLine(bl_help[0]);
    BL_PrintLine(bl_help[1]);
    BL_PrintLine(bl_help[2]);
    BL_PrintLine(bl_help[4]);
    BL_PrintLine(bl_help[3]);
    BL_PrintLine(bl_help[4]);
}

void BL_InfoScreenDFU()
{
    BL_DisplayInit();
    BL_PrintLine(bl_help[0]);
    BL_PrintLine(bl_help[1]);
    BL_PrintLine(bl_help[5]);
    BL_PrintLine(bl_help[4]);
    BL_PrintLine(bl_help[6]);
    BL_PrintLine(bl_help[7]);
    BL_PrintLine(bl_help[8]);
    BL_PrintLine(bl_help[9]);
}

void BL_Idle_Application(void)
{
    static bool power_was_up = false;
    static uint32_t tick;
    uint32_t now = HAL_GetTick();

    if (now >= tick)
    {
        mchfBl_PinToggle(LEDGREEN);
        tick = now + 1024;
    }
    if(mchfBl_ButtonGetState(BUTTON_POWER) == 0)
    {
        // we only switch off, if power button was at least once seen as being released and pressed in the idle loop
        if (power_was_up == true)
        {
            mcHF_PowerOff();
        }
    }
    else
    {
        // okay, power button is not pressed currently
        power_was_up = true;
    }

}

int BL_MSC_Application(void)
{

    mcHF_PowerHoldOn(); // make sure we have uninterrupted power

    BL_PrintLine("USB Drive detected.");
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
		if(was_download)
		{
        BL_PrintLine("Firmware will be updated...");
		}
        BL_PrintLine("Saving Flash to \"" UPLOAD_FILE  "\"...");
        COMMAND_UPLOAD();

        /* Check if BAND- Button was pressed */
        if (was_download)
        {
            /* Writes Flash memory */
            BL_PrintLine("Updating firmware using \"" DOWNLOAD_FILE "\"...");
            COMMAND_DOWNLOAD();
        }
        else
        {
            BL_PrintLine("Skipping firmware update.");
        }

        BL_PrintLine("");
        BL_PrintLine("Finished.");
        BL_PrintLine("");
        BL_PrintLine("Remove drive or press Band- to reboot.");
        BL_PrintLine("Press Power to switch off.");

        // if we don't have a display, turn off backlight
        if (mchf_display.DeviceCode == 0x0000)
        {
            mchfBl_PinOff(BACKLIGHT);
        }

        /* Waiting User Button Released */
        while ((mchfBl_ButtonGetState(BUTTON_BANDM) == 0))
        {}

        /* Waiting User Button Pressed or usb drive removed. If drive is removed, we go straight to reboot */
        while ((mchfBl_ButtonGetState(BUTTON_BANDM) == 1) && mcHF_USBConnected() != 0)
        {
            /* switch off if power button is pressed */
            if(mchfBl_ButtonGetState(BUTTON_POWER) == 0)
            {
                mcHF_PowerOff();
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

// __attribute__ ( ( naked ) )
void mchfBl_JumpToApplication(uint32_t ApplicationAddress)
{
    uint32_t* const APPLICATION_PTR = (uint32_t*)ApplicationAddress;

    // check if the stackpointer points into a likely ram area (normal RAM start + 1MB)
    if (APPLICATION_PTR[0] <= 0x20000000 + (1024 * 1024) && ( APPLICATION_PTR[0] > 0x20000000))
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

	#ifdef SBLA
	  *(uint32_t*)(SRAM2_BASE+5) = 0x29;	// signature for DF8OE development features
	#endif
	#ifdef SBLS
	  *(uint32_t*)(SRAM2_BASE+10) = 0x29;	// signature for special beta-testing features
	#endif

#if 0
    mcHF_PowerHoldOff();


    if( *(uint32_t*)(SRAM2_BASE) != 0x55)		// no reboot requested?
    {
        // we wait for a longer time
        HAL_Delay(300);
    }
#else
#endif
    mcHF_PowerHoldOn();
    HAL_Delay(50);

    if (mchfBl_ButtonGetState(BUTTON_BANDP) == 0)
    {
        BL_InfoScreenDFU();
        // BANDP pressed, DFU boot requested
        while (mchfBl_ButtonGetState(BUTTON_BANDP) == 0) {};
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
    BL_InfoScreen();
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

/*
 * This does not work if data cache is enabled, since we then need to flush the SRAM2_BASE write
 * but here flash should not be enabled anyway!
 */
void mchfBl_CheckAndGoForDfuBoot()
{

    if(*(uint32_t*)(SRAM2_BASE) == 0x99)
    {
        *(uint32_t*)(SRAM2_BASE) = 0;
#ifndef STM32F7
        __HAL_REMAPMEMORY_SYSTEMFLASH();

        const uint32_t dfu_boot_start = 0x00000000;
#else
        const uint32_t dfu_boot_start = 0x1FF00000;
        // if in dual boot mode (which is required for proper operation
        // we need to
        #if defined (FLASH_OPTCR_nDBANK)
            HAL_FLASH_OB_Unlock();
            if ((FLASH->OPTCR & (FLASH_OPTCR_nDBANK_Msk|FLASH_OPTCR_nDBOOT_Msk)) != FLASH_OPTCR_nDBOOT)
            {
                FLASH->OPTCR |= FLASH_OPTCR_nDBOOT; // set == disable dual boot
                FLASH->OPTCR &= ~FLASH_OPTCR_nDBANK; // unset == enabled dual bank mode
            }
            HAL_FLASH_OB_Launch();
            HAL_FLASH_OB_Lock();
        #endif

#endif
        mchfBl_JumpToApplication(dfu_boot_start);
        // start the STM32Fxxx bootloader at address dfu_boot_start;
    }
}

#include "uhsdr_board_config.h"

static uint8_t current_line;

void BL_PrintLine(const char* txt)
{
    UiLcdHy28_PrintText(4,4+current_line*16,txt,Yellow,Black,0);
    current_line++;
}
