/**
  ******************************************************************************
  * @file    usbd_usr.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file includes the user application layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_usr.h"
#include "usbd_ioreq.h"

#include <stdio.h>

int usb_otg_cdc_status = 0;

USBD_Usr_cb_TypeDef USR_cb =
{
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,
  USBD_USR_DeviceConnected,
  USBD_USR_DeviceDisconnected,    
};

void USBD_USR_Init(void) {
	usb_otg_cdc_status = 0;
}

void USBD_USR_DeviceReset(uint8_t speed ) {
	usb_otg_cdc_status = 0;
}

void USBD_USR_DeviceConfigured (void) {
	usb_otg_cdc_status = 1;
}

void USBD_USR_DeviceSuspended(void) {
	usb_otg_cdc_status = 0;
}

void USBD_USR_DeviceResumed(void) {
	usb_otg_cdc_status = 0;
}

void USBD_USR_DeviceConnected (void) {
	usb_otg_cdc_status = 0;
}

void USBD_USR_DeviceDisconnected (void) {
	usb_otg_cdc_status = 0;
}

int USBD_User_GetStatus(void) {
	return usb_otg_cdc_status;
}

/**
* @}
*/ 

/**
* @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
