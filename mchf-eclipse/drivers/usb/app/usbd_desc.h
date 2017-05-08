/**
  ******************************************************************************
  * @file           : usbd_desc.h
  * @version        : v1.0_Cube
  * @brief          : Header for usbd_desc file.
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_DESC__H__
#define __USBD_DESC__H__

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "usbd_def.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USB_DESC
  * @brief general defines for the usb device library file
  * @{
  */ 
#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06

#define AUDIO_INTERFACE_DESC_SIZE                     9
#define USB_AUDIO_DESC_SIZ                            0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07

#define AUDIO_SAMPLE_FREQ(frq)      (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))
#define AUDIO_IN_PACKET                              (uint32_t)(((USBD_AUDIO_IN_FREQ * USBD_AUDIO_IN_CHANNELS * 2) /1000))
#define AUDIO_PACKET_SZE(frq,channels)          (uint8_t)(((frq * channels * 2)/1000) & 0xFF), \
                                       (uint8_t)((((frq * channels * 2)/1000) >> 8) & 0xFF)
#define SAMPLE_FREQ(frq)               (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))


#define USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE 0x0B
#define USB_INTERFACE_ASSOCIATION_DESC_SIZE 0x08

#define USB_DEVICE_CLASS_MISCELLANEOUS 0xEF


#define CDC_DESCRIPTOR_TYPE                     0x21

#define DEVICE_CLASS_CDC                        0x02
#define DEVICE_SUBCLASS_CDC                     0x00


#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define USB_ENDPOINT_TYPE_ISOCHRONOUS                 0x01
#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x0001
#define AUDIO_CONTROL_VOLUME                          0x0002



#define CDC_IN_EP                   0x81  /* EP1 for data IN */
#define CDC_OUT_EP                  0x01  /* EP1 for data OUT */
#define CDC_CMD_EP                  0x82  /* EP2 for CDC commands */
#define CDC_CTRL_IF                 0x00
#define CDC_DATA_IF                 0x01
#define CDC_TOTAL_IF_NUM            0x02

#define AUDIO_OUT_EP                0x02
#define AUDIO_IN_EP                 0x83
#define AUDIO_CTRL_IF               0x02
#define AUDIO_OUT_IF                0x03
#define AUDIO_IN_IF                 0x04
#define AUDIO_TOTAL_IF_NUM          0x03

/** @defgroup USB_DESC_Exported_Defines
  * @{
  */

/**
  * @}
  */ 

/** @defgroup USBD_DESC_Exported_TypesDefinitions
  * @{
  */
/**
  * @}
  */ 

/** @defgroup USBD_DESC_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USBD_DESC_Exported_Variables
  * @{
  */ 
extern USBD_DescriptorsTypeDef FS_Desc;
/**
  * @}
  */ 

/** @defgroup USBD_DESC_Exported_FunctionsPrototype
  * @{
  */ 
  
/**
  * @}
  */ 
#ifdef __cplusplus
}
#endif

#endif /* __USBD_DESC_H */

/**
  * @}
  */ 

/**
* @}
*/ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
