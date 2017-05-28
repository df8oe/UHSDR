/**
  ******************************************************************************
  * @file    usbd_audio_cdc_comp.h
  * @author  MCD Application Team/ mcHF Devel
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   header file for the usbd_audio.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
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
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_AUDIO_COMP_H
#define __USB_AUDIO_COMP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"
#include  "usbd_desc.h"
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_AUDIO
  * @brief This file is the Header file for usbd_audio.c
  * @{
  */ 


/** @defgroup USBD_AUDIO_Exported_Defines
  * @{
  */ 
///#define AUDIO_OUT_EP                                  0x01

// #define USB_AUDIO_CONFIG_DESC_SIZ                     109


#define USBD_AUDIO_OUT_CHANNELS 2
#define USBD_AUDIO_IN_CHANNELS 2

#define USBD_AUDIO_IN_OUT_DIV 1
// USBD_IN_AUDIO_IN_OUT_DIV must be set to an integer number between 3 and 1
// in order to keep within the limits of the existing code in audio_driver.c audio_rx_processor

#define USBD_AUDIO_IN_FREQ (USBD_AUDIO_FREQ/USBD_AUDIO_IN_OUT_DIV)


#define AUDIO_CONTROL_MUTE                            0x0001

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_III                         0x03

#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_SET_CUR                             0x01

#define AUDIO_OUT_STREAMING_CTRL                      0x02
#define AUDIO_IN_STREAMING_CTRL                       0x06



#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ * 2 * 2) /1000)) 
#define AUDIO_DEFAULT_VOLUME                          70
    
/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
#define AUDIO_OUT_PACKET_NUM                          16
/* Total size of the audio transfer buffer */
#define AUDIO_TOTAL_BUF_SIZE                          ((uint32_t)(AUDIO_OUT_PACKET * AUDIO_OUT_PACKET_NUM))
    
    /* Audio Commands enumeration */
typedef enum
{
  AUDIO_CMD_START = 1,
  AUDIO_CMD_PLAY,
  AUDIO_CMD_STOP,
  AUDIO_CMD_PAUSE
}AUDIO_CMD_TypeDef;


typedef enum
{
  AUDIO_OFFSET_NONE = 0,
  AUDIO_OFFSET_HALF,
  AUDIO_OFFSET_FULL,  
  AUDIO_OFFSET_UNKNOWN,    
}
AUDIO_OffsetTypeDef;
/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
 typedef struct
{
   uint8_t cmd;   
   uint8_t data[USB_MAX_EP0_SIZE];  
   uint8_t len;  
   uint8_t unit;    
}
USBD_AUDIO_ControlTypeDef; 



typedef struct
{
  __IO uint32_t             alt_setting[USBD_MAX_NUM_INTERFACES+1];
  // uint8_t                   buffer[AUDIO_TOTAL_BUF_SIZE];
  // AUDIO_OffsetTypeDef       offset;
  // uint8_t                    rd_enable;
  // uint16_t                   rd_ptr;
  // uint16_t                   wr_ptr;
  USBD_AUDIO_ControlTypeDef control;
  uint32_t SendFlag;
  uint32_t PlayFlag;
}
USBD_AUDIO_HandleTypeDef; 


typedef struct
{
    int8_t  (*Init)         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
    int8_t  (*DeInit)       (uint32_t options);
    int8_t  (*AudioCmd)     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
    int8_t  (*VolumeCtl)    (uint8_t vol);
    int8_t  (*MuteCtl)      (uint8_t cmd);
    int8_t  (*PeriodicTC)   (uint8_t cmd);
    int8_t  (*GetState)     (void);
    int8_t  (*InVolumeCtl)    (uint8_t vol);
}USBD_AUDIO_ItfTypeDef;
/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_ClassTypeDef  USBD_AUDIO;
#define USBD_AUDIO_CLASS    &USBD_AUDIO
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                        USBD_AUDIO_ItfTypeDef *fops);

void  USBD_AUDIO_Sync (USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset);
/**
  * @}
  */ 

typedef struct UsbAudioUnit_s
{
    uint8_t cs;
    uint8_t cn;
    int16_t min;
    int16_t max;
    uint16_t res;
    int16_t cur;
    __IO uint8_t* ptr; // pointer to data structure which is used elsewhere to store volume
} UsbAudioUnit;

// In / Out Volume;

enum
{
    UnitVolumeTX = 0,
    UnitVolumeRX,
    UnitMax
};
extern UsbAudioUnit usbUnits[UnitMax];

#ifdef __cplusplus
}
#endif

#endif  /* __USB_AUDIO_COMP_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
