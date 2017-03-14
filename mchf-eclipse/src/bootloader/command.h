/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
  ******************************************************************************
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   Header file for command.c
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
#ifndef _COMMAND_H
#define _COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    BL_ERR_NONE = 0,
    BL_ERR_USBPROBLEM = 1,
    BL_ERR_NOIMAGE = 2,
    BL_ERR_FLASHTOOSMALL = 3,
    BL_ERR_WRITEDISK = 4,
    BL_ERR_READDISK = 5,
    BL_ERR_FLASHPROG = 6,
    BL_ERR_FLASHERASE = 7,
    BL_ERR_FLASHPROTECT = 8,
} mchf_bootloader_error_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void COMMAND_UPLOAD(void);
void COMMAND_DOWNLOAD(void);
void COMMAND_ResetMCU(uint32_t code);



void BootFail_Handler(uint8_t count);
void FlashFail_Handler(mchf_bootloader_error_t errcode);

#ifdef __cplusplus
}
#endif

#endif  /* _COMMAND_H */
