/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
  ******************************************************************************
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   Header file for flash_layer.c
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _FLASH_ACCESS_LAYER_H
#define _FLASH_ACCESS_LAYER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);
/* Exported constants --------------------------------------------------------*/
/* extern const char* version;
extern const char* author; */

/* Define the flash memory start address */
#define USER_FLASH_STARTADDRESS    ((uint32_t)0x08000000) /* Flash Start Address */

/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x08007FFF is reserved for the Firmware upgrade code */
#define APPLICATION_ADDRESS        ((uint32_t)0x08010000)

/* End of the Flash address for the largest device, dynamically sized down based on real processor flash */
#define USER_FLASH_END_ADDRESS     ((uint32_t)0x081FFFFF)

#define STM32_FLASH_ADDRESS        0x1FFF7A22
#define STM32_GetFlashSize()    (*(uint16_t *) (STM32_FLASH_ADDRESS))
#define MCHF_FLASHRESERVED      64

/**
 * @brief real user flash size of microprocessor (flash size - bootloader/config flash size)
 * @returns size of user flash in bytes
 */
inline uint32_t flashIf_userFlashSize()  {  return ((STM32_GetFlashSize() - MCHF_FLASHRESERVED)*1024); }


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void flashIf_FlashUnlock();
void flashIf_FlashLock();
FlagStatus flashIf_ReadOutProtectionStatus();
uint32_t flashIf_EraseSectors(uint32_t Address, uint32_t Length);
HAL_StatusTypeDef flashIf_ProgramWord(uint32_t Address, uint32_t Data);


#ifdef __cplusplus
}
#endif

#endif  /* _FLASH_IF_H */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/


