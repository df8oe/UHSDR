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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

// Common
#include "mchf_board.h"

#include <stdio.h>

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"

#include "cat_driver.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END ;

extern USB_OTG_CORE_HANDLE           USB_OTG_dev;
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);

extern __IO CatDriver kd;

//void OTG_FS_WKUP_IRQHandler(void)
//{
	//if(USB_OTG_dev.cfg.low_power)
	//{
	//	*(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
	//	SystemInit();
	//	USB_OTG_UngateClock(&USB_OTG_dev);
	//}
	//EXTI_ClearITPendingBit(EXTI_Line18);
//}

void OTG_FS_IRQHandler(void)
{
	USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

void cat_driver_init(void)
{
	// Start driver
	USBD_Init(	&USB_OTG_dev,
	            USB_OTG_FS_CORE_ID,
	            &USR_desc,
	            &USBD_CDC_cb,
	            &USR_cb);

	printf("cat driver started\n\r");
}

void cat_driver_stop(void)
{
	// Stop driver
	USBD_DeInit(&USB_OTG_dev);

	printf("cat driver stopped\n\r");
}

void cat_driver_thread(void)
{
	if(!kd.enabled)
		return;
}
