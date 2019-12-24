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

#include <unistd.h>
#include "dma.h"
#include "spi.h"
#include "gpio.h"

#include "uhsdr_board.h"
#include "flash_if.h"
#include "uhsdr_boot_hw.h"
#include "command.h"
#include "fatfs.h"
#include "usb_host.h"
#include "bootloader_main.h"
#include "ui_lcd_hy28.h"


FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
char USBDISKPath[4];          /* USB Host logical drive path */

bool boot_failed;

extern USBH_HandleTypeDef hUsbHostHS;
static uint8_t Bootloader_USBConnected()
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


static void Bootloader_DisplayInit()
{
    MX_DMA_Init();
    MX_SPI2_Init();
    MX_GPIO_Init();

    // we need to set the touch screen CS signal to high, otherwise SPI displays
    // will not be detectable
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Pull     = GPIO_PULLUP;
    GPIO_InitStructure.Speed    = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Mode     = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pin = TP_CS;

    HAL_GPIO_Init(TP_CS_PIO, &GPIO_InitStructure);

    GPIO_SetBits(TP_CS_PIO, TP_CS);

    UiLcdHy28_Init();
    UiLcdHy28_LcdClear(Black);
    mchfBl_PinOn(BACKLIGHT);

}

// bin rw: 0 1 2 3 4
// dfu     0 1 5 4 6 7 8 9

static void Bootloader_InfoScreen()
{
    Bootloader_DisplayInit();
    Bootloader_PrintLine(bl_help[0]);
    Bootloader_PrintLine(bl_help[1]);
    Bootloader_PrintLine(bl_help[2]);
    Bootloader_PrintLine(bl_help[4]);
    Bootloader_PrintLine(bl_help[3]);
    Bootloader_PrintLine(bl_help[4]);
}

static void Bootloader_InfoScreenDFU()
{
    Bootloader_DisplayInit();
    Bootloader_PrintLine(bl_help[0]);
    Bootloader_PrintLine(bl_help[1]);
    Bootloader_PrintLine(bl_help[5]);
    Bootloader_PrintLine(bl_help[4]);
    Bootloader_PrintLine(bl_help[6]);
    Bootloader_PrintLine(bl_help[7]);
    Bootloader_PrintLine(bl_help[8]);
    Bootloader_PrintLine(bl_help[9]);
}

static void Bootloader_UsbHost_Idle()
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

static int Bootloader_UsbMSCDevice_Application(void)
{

    Bootloader_PowerHoldOn(); // make sure we have uninterrupted power

    Bootloader_PrintLine("USB Drive detected.");
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
        Bootloader_PrintLine("Firmware will be updated...");
		}
        Bootloader_PrintLine("Saving Flash to \"" UPLOAD_FILE  "\"...");
        COMMAND_UPLOAD();

        /* Check if BAND- Button was pressed */
        if (was_download)
        {
            /* Writes Flash memory */
            Bootloader_PrintLine("Updating firmware using \"" DOWNLOAD_FILE "\"...");
            COMMAND_DOWNLOAD();
        }
        else
        {
            Bootloader_PrintLine("Skipping firmware update.");
        }

        Bootloader_PrintLine("");
        Bootloader_PrintLine("Finished.");
        Bootloader_PrintLine("");
        Bootloader_PrintLine("Remove drive or press Band- to reboot.");
        Bootloader_PrintLine("Press Power to switch off.");

        // if we don't have a display, turn off backlight
        if (mchf_display.DeviceCode == 0x0000)
        {
            mchfBl_PinOff(BACKLIGHT);
        }

        /* Waiting User Button Released */
        while ((mchfBl_ButtonGetState(BUTTON_BANDM) == 0))
        {}

        /* Waiting User Button Pressed or usb drive removed. If drive is removed, we go straight to reboot */
        while ((mchfBl_ButtonGetState(BUTTON_BANDM) == 1) && Bootloader_USBConnected() != 0)
        {
            /* switch off if power button is pressed */
            if(mchfBl_ButtonGetState(BUTTON_POWER) == 0)
            {
                mcHF_PowerOff();
            }
        }

        /* Waiting User Button Released or usb drive removed.  If drive is removed, we go straight to reboot */
        while ((mchfBl_ButtonGetState(BUTTON_BANDM) == 0) && Bootloader_USBConnected() != 0)
        {}

        /* Jumps to user application code located in the internal Flash memory */
        COMMAND_ResetMCU(BOOT_REBOOT);
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
extern const uint32_t* _linker_ram_start; // we get this from the linker script
static const uint32_t linker_ram_start_addr = (uint32_t)&_linker_ram_start;

/**
 * We check whether the supplied address is pointing to a valid vector table
 * Checks are simple (right now we just look if the stack pointer points in the region of RAM)
 *
 * @param ApplicationAddress an 32 bit value pointing to the start of a vector table
 * @return true we should try to use this address
 */
bool uhsdrBl_IsValidApplication(uint32_t ApplicationAddress)
{
    // TODO: Better checks
    // check if the stackpointer points into a likely ram area (normal RAM start + 1MB)
    uint32_t* const APPLICATION_PTR = (uint32_t*)ApplicationAddress;
    return  APPLICATION_PTR[0] <= (linker_ram_start_addr + (1024 * 1024)) && ( APPLICATION_PTR[0] >= linker_ram_start_addr);
}

/**
 * Jumps to new code using a reset vector,
 * brings processor into the default state (almost like in reset state, except for peripherals and their clocks)
 * set stackpointer and then jumps to the specific address. This function normally never returns unless
 * the IsValidApplication check fails.
 *
 * @param ApplicationAddress the start address of the reset vector (not the start of the code itself!)
 */
void UhsdrBl_JumpToApplication(uint32_t ApplicationAddress)
{

    if (uhsdrBl_IsValidApplication(ApplicationAddress))
    {
        uint32_t* const APPLICATION_PTR = (uint32_t*)ApplicationAddress;

        // disable all interrupts
        __disable_irq();

        // disable systick irq
        HAL_SuspendTick();

        // set clocks to default state
        HAL_RCC_DeInit();

        // clear Interrupt Enable Register & Interrupt Pending Register
        const size_t icer_count = sizeof(NVIC->ICER)/sizeof(NVIC->ICER[0]);

        for (size_t i = 0; i <icer_count; i++)
        {
            NVIC->ICER[i]=0xFFFFFFFF;
            NVIC->ICPR[i]=0xFFFFFFFF;
        }

        // enable all interrupts
        __enable_irq();

        __set_MSP(APPLICATION_PTR[0]);
        /* Jump to user application */
        ((pFunction) APPLICATION_PTR[1])();
    }
}

int Bootloader_Main()
{
    double i,border;
	/* prevention of erratical boot loop */
	if( *(__IO uint32_t*)(SRAM2_BASE) != BOOT_REBOOT)
	{
  	  border = 300000; // long delay if firmware was not running before or at powerdown
  	}
	else
	{
  	  border = 10; // short delay if reboot is requested
    }
  	for (i = 0; i < border; i++)
	{
	  ;
	}

    /* initialization */
    BSP_Init();

    Bootloader_PowerHoldOn();
    HAL_Delay(50);

    if (boot_failed == true)
    {
        BootFail_Handler(3);
    }
    else if (mchfBl_ButtonGetState(BUTTON_BANDP) == 0)
    {
        Bootloader_InfoScreenDFU();
        // BANDP pressed, DFU boot requested
        while (mchfBl_ButtonGetState(BUTTON_BANDP) == 0) {};
        COMMAND_ResetMCU(BOOT_DFU);
    }
    /* Test if BAND- button on mchf is NOT pressed */
    else if (mchfBl_ButtonGetState(BUTTON_BANDM) == 1)
    {
        /* Check Vector Table: Test if user code is programmed starting from address
           "APPLICATION_ADDRESS"
           We do that be reading the stack pointer value stored at APPLICATION_ADDRESS
           and roughly check if it points to a RAM SPACE address
         */
        // we go through restart here to have all interrupts turned off and everything as
        // pristine as it gets after reset, avoids issues with enabled interrupts etc.
        // we had that and it is pain to debug, see #1610 in GitHub!
        if (uhsdrBl_IsValidApplication(APPLICATION_ADDRESS))
        {
            UhsdrBl_JumpToApplication(APPLICATION_ADDRESS);
        }
        else
        {
            BootFail_Handler(2);
        }
        // never reached, is endless loop
    }

    /* Init upgrade mode display */
    Bootloader_InfoScreen();
    mchfBl_PinOn(BACKLIGHT);
    // now give user a chance to let go off the BAND- button
    HAL_Delay(3000);
    return 0;
}


extern ApplicationTypeDef Appli_state;

void Bootloader_UsbHostApplication()
{
    switch(Appli_state)
    {
    case APPLICATION_READY:
      Bootloader_UsbMSCDevice_Application();
      Appli_state = APPLICATION_IDLE;
      break;
    case APPLICATION_DISCONNECT:
      f_mount(NULL, (TCHAR const*)"", 0);
      Appli_state = APPLICATION_IDLE;
      break;
    case APPLICATION_IDLE:
    default:
      break;

    }

    Bootloader_UsbHost_Idle();
}

/*
 * This does not work if data cache is enabled, since we then need to flush the SRAM2_BASE write
 * but here flash should not be enabled anyway!
 */
void Bootloader_CheckAndGoForBootTarget()
{

    if(*(uint32_t*)(SRAM2_BASE) == BOOT_DFU)
    {
        *(uint32_t*)(SRAM2_BASE) = BOOT_CLEARED;
#if defined(STM32F4)
        __HAL_REMAPMEMORY_SYSTEMFLASH();

        const uint32_t dfu_boot_start = 0x00000000;
#elif defined(STM32F7)
        const uint32_t dfu_boot_start = 0x1FF00000;
        // if in dual boot mode (which is required for proper operation
        // we need to fix the setup
    #if defined (FLASH_OPTCR_nDBANK)
        if ((FLASH->OPTCR & (FLASH_OPTCR_nDBANK_Msk|FLASH_OPTCR_nDBOOT_Msk)) != FLASH_OPTCR_nDBOOT)
        {
            if (HAL_FLASH_OB_Unlock() == HAL_OK)
            {
                FLASH->OPTCR |= FLASH_OPTCR_nDBOOT; // set == disable dual boot
                FLASH->OPTCR &= ~FLASH_OPTCR_nDBANK; // unset == enabled dual bank mode
                HAL_FLASH_OB_Launch();
                HAL_FLASH_OB_Lock();
            }
        }
    #endif
#elif defined(STM32H7)
        const uint32_t dfu_boot_start = 0x1FF09800;
#endif
        UhsdrBl_JumpToApplication(dfu_boot_start);
        // start the STM32Fxxx bootloader at address dfu_boot_start;
    }

    if(*(uint32_t*)(SRAM2_BASE) == BOOT_FIRMWARE)
    {
        *(uint32_t*)(SRAM2_BASE) = BOOT_CLEARED;
        UhsdrBl_JumpToApplication(APPLICATION_ADDRESS);
        // start the STM32Fxxx bootloader at address dfu_boot_start;
        boot_failed = true;
    }

}

#include "uhsdr_board_config.h"

static uint8_t current_line;

void Bootloader_PrintLine(const char* txt)
{
    UiLcdHy28_PrintText(4,4+current_line*16,txt,Yellow,Black,0);
    current_line++;
}
