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
**  Licence:		CC BY-NC-SA 3.0                                            **
************************************************************************************/

#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "flash_if.h"


USB_OTG_CORE_HANDLE          USB_OTG_Core;
USBH_HOST                    USB_Host;

pFunction Jump_To_Application;
uint32_t JumpAddress;

int main(void)
{
    /* initialization */
    BSP_Init();
    STM_EVAL_LEDOn(ON);

    double i,border;

// *(__IO uint32_t*)(SRAM2_BASE+10) = 0x29;	// signature for DG9BFC Beta-Testing
    if( *(__IO uint32_t*)(SRAM2_BASE) != 0x55)
        border = 300000;
    else
        border = 1000;

    for (i = 0; i < border; i++)
        ;
    STM_EVAL_LEDOff(ON);


    /* Flash unlock */
    FLASH_If_FlashUnlock();

    /* Test if BAND- button on mchf is pressed */
    if (STM_EVAL_PBGetState(BUTTON_BANDM) == Bit_SET)
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

#ifdef USE_USB_OTG_FS
    USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);
#else	// use HS_CORE
    USBH_Init(&USB_OTG_Core, USB_OTG_HS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);
#endif

    while (1)
    {
        /* Host Task handler */
        USBH_Process(&USB_OTG_Core, &USB_Host);
    }
}