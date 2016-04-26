/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USH_USR_H__
#define __USH_USR_H__


/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"
//#include "usbh_usr_lcd.h"
#include "usb_conf.h"
#include <stdio.h>


extern  USBH_Usr_cb_TypeDef USR_Callbacks;

// Keyboard Status
typedef struct KeyBoardStatus
{
    // UI public state
    uchar old_state;
    uchar new_state;

    // Edit mode
    uchar set_mode;

    // Last key pressed value
    uchar last_char;

    uchar edit_item_id;
    uchar edit_item_hor;

    uchar item_0[16];
    uchar item_1[16];
    uchar item_2[16];

} KeyBoardStatus;


void USBH_USR_ApplicationSelected(void);
void USBH_USR_Init(void);
void USBH_USR_DeInit(void);
void USBH_USR_DeviceAttached(void);
void USBH_USR_ResetDevice(void);
void USBH_USR_DeviceDisconnected (void);
void USBH_USR_OverCurrentDetected (void);
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed);
void USBH_USR_Device_DescAvailable(void *);
void USBH_USR_DeviceAddressAssigned(void);
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
        USBH_InterfaceDesc_TypeDef *itfDesc,
        USBH_EpDesc_TypeDef *epDesc);
void USBH_USR_Manufacturer_String(void *);
void USBH_USR_Product_String(void *);
void USBH_USR_SerialNum_String(void *);
void USBH_USR_EnumerationDone(void);
USBH_USR_Status USBH_USR_UserInput(void);
void USBH_USR_DeInit(void);
void USBH_USR_DeviceNotSupported(void);
void USBH_USR_UnrecoveredError(void);

#endif

