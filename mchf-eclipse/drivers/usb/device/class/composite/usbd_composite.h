/**
  ******************************************************************************
  * @file    USBD_COMP_cdc_comp.h
  * @author  MCD Application Team/ mcHF Devel
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   header file for the USBD_COMP.c file.
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
#ifndef __USB_COMPOSITE_H
#define __USB_COMPOSITE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"
#include "usbd_desc.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_COMP
  * @brief This file is the Header file for USBD_COMP.c
  * @{
  */ 


/** @defgroup USBD_COMP_Exported_Defines
  * @{
  */ 
    
/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

 typedef struct
 {
     USBD_ClassTypeDef* class;
     void* classData;
     void* userData;
     uint16_t ctrlIf;
     uint16_t minIf;
     uint16_t maxIf;
 } USBD_ClassCompInfo;

#define USBD_MAX_EP 3

 typedef struct
{
    uint8_t in[USBD_MAX_EP+1];
    uint8_t out[USBD_MAX_EP+1];
} usbd_ep_map_t;


#define CLASS_AUDIO 0
#define CLASS_CDC 1
#define CLASS_NUM 2
#define CLASS_UNUSED 0xFF

extern const usbd_ep_map_t usbdEpMap;

extern USBD_ClassCompInfo dev_instance[CLASS_NUM];

#define USB_AUDIO_CONFIG_DESC_SIZ                        (9+101+73 + 8 + 66 + 9 +7)
uint8_t USBD_COMP_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ];


typedef struct
{
}
USBD_COMP_HandleTypeDef;


typedef struct
{
}USBD_COMP_ItfTypeDef;

extern USBD_COMP_ItfTypeDef USBD_COMP_fops_FS;
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

extern USBD_ClassTypeDef  USBD_COMP;
#define USBD_COMP_CLASS    &USBD_COMP
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t  USBD_COMP_RegisterInterface  (USBD_HandleTypeDef   *pdev,
                                        USBD_COMP_ItfTypeDef *fops);

/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_COMPOSITE_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
