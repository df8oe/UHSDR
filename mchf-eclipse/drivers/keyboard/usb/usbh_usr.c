
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
	//printf("USBH_USR_Init\n\r");
	//while(1);

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
	printf("USBH_USR_DeviceAttached\n\r");

	// Device plugged
	kbs.new_state = 1;
}

void USBH_USR_UnrecoveredError (void)
{
	printf("USBH_USR_UnrecoveredError\n\r");
}

void USBH_USR_DeviceDisconnected (void)
{
	//printf("USBH_USR_DeviceDisconnected\n\r");
  
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
	  //printf("hi speed\n\r");
  }
  else if(DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
  {
	  //printf("full speed\n\r");
  }
  else if(DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
  {
	  //printf("low speed\n\r");
  }
  else
  {
	  //printf("speed error\n\r");
  }
}

void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
  USBH_DevDesc_TypeDef *hs;
  hs = DeviceDesc;  
  
  printf("VID : %04Xh\n\r" , (uint32_t)(*hs).idVendor);
  printf("PID : %04Xh\n\r" , (uint32_t)(*hs).idProduct);
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
  //printf("Manufacturer : %s\n\r", (char *)ManufacturerString);
}

void USBH_USR_Product_String(void *ProductString)
{
  //printf("Product : %s\n\r", (char *)ProductString);
}

void USBH_USR_SerialNum_String(void *SerialNumString)
{
  //printf("Serial Number : %s\n\r", (char *)SerialNumString);
} 

void USBH_USR_EnumerationDone(void)
{
	//printf("enumeration done\n\r");
} 

/**
* @brief  USBH_USR_DeviceNotSupported
*         Device is not supported
* @param  None
* @retval None
*/
void USBH_USR_DeviceNotSupported(void)
{
	printf("USB device not supported\n\r");
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
	 //printf("keyboard init\n\r");

	 // Keyboard plugged
	 kbs.new_state = 2;
}

void  USR_KEYBRD_ProcessData (uint8_t data)
{
//	char buf[10];

	//printf("%02x ",data);

	kbs.last_char = data;

//	buf[0] = (char)data;
//	buf[1] = 0;

	//if(isalpha(data)||isdigit(data))
//		printf("%s ",buf);
	//else
	//	printf("0x%02x\n\r",(char)data);
  
//  if(data == '\n')
  //{
    //KeybrdCharYpos = KYBRD_FIRST_COLUMN;
    
    /*Increment char X position*/
//    KeybrdCharXpos+=SMALL_FONT_LINE_WIDTH;
    
  //}
  //else if (data == '\r')
  //{
    /* Manage deletion of charactter and upadte cursor location*/
    //if( KeybrdCharYpos == KYBRD_FIRST_COLUMN)
    //{
      /*First character of first line to be deleted*/
      //if(KeybrdCharXpos == KYBRD_FIRST_LINE)
      //{
        //KeybrdCharYpos =KYBRD_FIRST_COLUMN;
     // }
     // else
      //{
//        KeybrdCharXpos-=SMALL_FONT_LINE_WIDTH;
//        KeybrdCharYpos =(KYBRD_LAST_COLUMN+SMALL_FONT_COLUMN_WIDTH);
      //}
    //}
    //else
    //{
 //     KeybrdCharYpos +=SMALL_FONT_COLUMN_WIDTH;
      
    //}
   // LCD_DisplayChar(KeybrdCharXpos,KeybrdCharYpos, ' ');
  //}
 // else
 // {
    //LCD_DisplayChar(KeybrdCharXpos,KeybrdCharYpos, data);
    /* Update the cursor position on LCD */
    
    /*Increment char Y position*/
//    KeybrdCharYpos -=SMALL_FONT_COLUMN_WIDTH;
    
    /*Check if the Y position has reached the last column*/
//    if(KeybrdCharYpos == KYBRD_LAST_COLUMN)
//    {
//      KeybrdCharYpos = KYBRD_FIRST_COLUMN;
      
      /*Increment char X position*/
  //    KeybrdCharXpos+=SMALL_FONT_LINE_WIDTH;
      
 //   }
  //}
}

void USBH_USR_DeInit(void)
{
}

