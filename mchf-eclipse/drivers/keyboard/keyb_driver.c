/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

// Common
#include "mchf_board.h"

#include "usbh_bsp.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_hid_core.h"

#include "keyb_driver.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core_dev __ALIGN_END ;
__ALIGN_BEGIN USBH_HOST                     USB_Host __ALIGN_END ;


//*----------------------------------------------------------------------------
//* Function Name       : TIM2_IRQHandler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void TIM2_IRQHandler(void)
{
    USBH_OTG_BSP_TimerIRQ();
}

//*----------------------------------------------------------------------------
//* Function Name       : OTG_HS_IRQHandler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void OTG_HS_IRQHandler(void)
{
    // Interrupt here even if driver not enabled, when the cat driver works
    //  to fix!
    //USBH_OTG_ISR_Handler(&USB_OTG_Core_dev);
}

void keyb_driver_init(void)
{
#if 0
    printf("keyb driver init...\n\r");

    // Init Host Library
    USBH_Init(	&USB_OTG_Core_dev,
                USB_OTG_HS_CORE_ID,
                &USB_Host,
                &HID_cb,
                &USR_Callbacks);

    printf("keyb driver init ok\n\r");
#endif
}

void keyb_driver_thread(void)
{
    // Host Task handler
    USBH_Process(&USB_OTG_Core_dev , &USB_Host);
}
