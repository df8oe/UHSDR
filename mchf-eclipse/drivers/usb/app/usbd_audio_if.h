/**
  ******************************************************************************
  * @file           : usbd_audio_if.h
  * @brief          : header file for the usbd_audio_if file
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
#ifndef __USBD_AUDIO_IF_H
#define __USBD_AUDIO_IF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_cdc_comp.h"
/* USER CODE BEGIN INCLUDE */
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_AUDIO_IF
  * @brief header 
  * @{
  */ 

/** @defgroup USBD_AUDIO_IF_Exported_Defines
  * @{
  */ 
/* USER CODE BEGIN EXPORTED_DEFINES */
/* USER CODE END  EXPORTED_DEFINES */

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_IF_Exported_Types
  * @{
  */  
/* USER CODE BEGIN EXPORTED_TYPES */
/* USER CODE END EXPORTED_TYPES */

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_IF_Exported_Macros
  * @{
  */ 
/* USER CODE BEGIN EXPORTED_MACRO */
/* USER CODE END EXPORTED_MACRO */

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_IF_Exported_Variables
  * @{
  */ 
  extern USBD_AUDIO_ItfTypeDef  USBD_AUDIO_fops_FS;

/* USER CODE BEGIN EXPORTED_VARIABLES */
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_IF_Exported_FunctionsPrototype
  * @{
  */ 
/**
  * @brief  Manages the DMA full Transfer complete event.
  * @param  None
  * @retval None
  */
  void TransferComplete_CallBack_FS(void);

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @param  None
  * @retval None
  */
  void HalfTransfer_CallBack_FS(void);

/* USER CODE BEGIN EXPORTED_FUNCTIONS */
  extern void audio_in_put_buffer(int16_t sample);
  extern void audio_out_fill_tx_buffer(int16_t *buffer, uint32_t len);
/* USER CODE END EXPORTED_FUNCTIONS */
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 
  
#ifdef __cplusplus
}
#endif

#endif /* __USBD_AUDIO_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
