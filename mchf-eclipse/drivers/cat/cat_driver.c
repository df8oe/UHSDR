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
#include "usbd_cdc_vcp.h"
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

#define CAT_BUFFER_SIZE 256
__IO uint8_t cat_buffer[CAT_BUFFER_SIZE];
__IO int32_t cat_head = 0;
__IO int32_t cat_tail = 0;

CatInterfaceState cat_driver_state() { return USBD_User_GetStatus(); }

int cat_buffer_remove(uint8_t* c_ptr) {
    if (cat_head != cat_tail) {
        int c = cat_buffer[cat_tail];
        cat_tail = (cat_tail + 1) % CAT_BUFFER_SIZE;
        *c_ptr = (uint8_t)c;
        return 1;
    } else {
        return 0;
    }
}

int cat_buffer_add(uint8_t c) {
    int32_t next_head = (cat_head + 1) % CAT_BUFFER_SIZE;
    if (next_head != cat_tail) {
        /* there is room */
        cat_buffer[cat_head] = c;
        cat_head = next_head;
        return 1;
    } else {
        /* no room left in the buffer */
        return 0;
    }
}


uint8_t cat_driver_get_data(uint8_t* Buf,uint32_t Len) {
	uint8_t res = 0;
	if (cat_driver_has_data() >= Len) {
		int i;
		for  (i = 0; i < Len; i++) {
			cat_buffer_remove(&Buf[i]);
		}
		res = 1;
	}
	return res;
}
uint8_t cat_driver_put_data(uint8_t* Buf,uint32_t Len) {
	uint8_t res = 0;
	if (USBD_User_GetStatus()) {
	VCP_DataTx(Buf,Len);
	res = 1;
	}
	return res;
}

uint8_t cat_driver_has_data() {
	int32_t len = cat_head - cat_tail;
	return len < 0?len+CAT_BUFFER_SIZE:len;
}
