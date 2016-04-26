/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */

// Common
#include "mchf_board.h"

#include "usbh_usr.h"
#include "usbh_hid_mouse.h"
#include "usbh_hid_keybd.h"

#define KYBRD_FIRST_COLUMN               (uint16_t)319
#define KYBRD_LAST_COLUMN                (uint16_t)7
#define KYBRD_FIRST_LINE                 (uint8_t)120
#define KYBRD_LAST_LINE                  (uint8_t)200

extern USB_OTG_CORE_HANDLE           USB_OTG_Core_dev;

USBH_Usr_cb_TypeDef USR_Callbacks =
{
    USBH_USR_Init,
    USBH_USR_DeInit,
    USBH_USR_DeviceAttached,
    USBH_USR_ResetDevice,
    USBH_USR_DeviceDisconnected,
    USBH_USR_OverCurrentDetected,
    USBH_USR_DeviceSpeedDetected,
    USBH_USR_Device_DescAvailable,
    USBH_USR_DeviceAddressAssigned,
    USBH_USR_Configuration_DescAvailable,
    USBH_USR_Manufacturer_String,
    USBH_USR_Product_String,
    USBH_USR_SerialNum_String,
    USBH_USR_EnumerationDone,
    USBH_USR_UserInput,
    NULL,
    USBH_USR_DeviceNotSupported,
    USBH_USR_UnrecoveredError
};

// Public keyboard status
__IO KeyBoardStatus		kbs;

void USBH_USR_Init(void)
{
    // Default state - nothing connected
    kbs.old_state = 0;
    kbs.new_state = 0;

    // Edit mode off
    kbs.set_mode  = 0;

    // Store keyboard char
    kbs.last_char = 0;

    // Edit item reset
    kbs.edit_item_id  = 0;
    kbs.edit_item_hor = 0;
}

void USBH_USR_DeviceAttached(void)
{
    // Device plugged
    kbs.new_state = 1;
}

void USBH_USR_UnrecoveredError (void)
{
}

void USBH_USR_DeviceDisconnected (void)
{
    // Device unplugged
    kbs.new_state = 0;
}

void USBH_USR_ResetDevice(void)
{
    /* Users can do their application actions here for the USB-Reset */
}

void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
    if(DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED)
    {
    }
    else if(DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
    {
    }
    else if(DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
    {
    }
    else
    {
    }
}

void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
    USBH_DevDesc_TypeDef *hs;
    hs = DeviceDesc;

    printf("VID : %04lXh\n\r" , (uint32_t)(*hs).idVendor);
    printf("PID : %04lXh\n\r" , (uint32_t)(*hs).idProduct);
}

void USBH_USR_DeviceAddressAssigned(void)
{

}

void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
        USBH_InterfaceDesc_TypeDef *itfDesc,
        USBH_EpDesc_TypeDef *epDesc)
{
    USBH_InterfaceDesc_TypeDef *id;

    id = itfDesc;

    if((*id).bInterfaceClass  == 0x08)
    {

    }
    else if((*id).bInterfaceClass  == 0x03)
    {

    }
}

void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
}

void USBH_USR_Product_String(void *ProductString)
{
}

void USBH_USR_SerialNum_String(void *SerialNumString)
{
}

void USBH_USR_EnumerationDone(void)
{
}

/**
* @brief  USBH_USR_DeviceNotSupported
*         Device is not supported
* @param  None
* @retval None
*/
void USBH_USR_DeviceNotSupported(void)
{
}


/**
* @brief  USBH_USR_UserInput
*         User Action for application state entry
* @param  None
* @retval USBH_USR_Status : User response for key button
*/
USBH_USR_Status USBH_USR_UserInput(void)
{
    USBH_USR_Status usbh_usr_status;

    usbh_usr_status = USBH_USR_NO_RESP;

    /*Key B3 is in polling mode to detect user action */
    //if(STM_EVAL_PBGetState(Button_KEY) == RESET)
    //{

    // emulate click
    usbh_usr_status = USBH_USR_RESP_OK;

    //}


    return usbh_usr_status;

}

void USBH_USR_OverCurrentDetected (void)
{
    // LCD_ErrLog ("Overcurrent detected.\n");

}

void USR_MOUSE_Init	(void)
{

}

void USR_MOUSE_ProcessData(HID_MOUSE_Data_TypeDef *data)
{

    uint8_t idx = 1;
    static uint8_t b_state[3] = { 0, 0 , 0};

    if ((data->x != 0) && (data->y != 0))
    {
//    HID_MOUSE_UpdatePosition(data->x , data->y);
    }

    for ( idx = 0 ; idx < 3 ; idx ++)
    {

        if(data->button & 1 << idx)
        {
            if(b_state[idx] == 0)
            {
                //      HID_MOUSE_ButtonPressed (idx);
                b_state[idx] = 1;
            }
        }
        else
        {
            if(b_state[idx] == 1)
            {
                //    HID_MOUSE_ButtonReleased (idx);
                b_state[idx] = 0;
            }
        }
    }


}

void  USR_KEYBRD_Init (void)
{
    // Keyboard plugged
    kbs.new_state = 2;
}

void  USR_KEYBRD_ProcessData (uint8_t data)
{
    kbs.last_char = data;
}

void USBH_USR_DeInit(void)
{
}

