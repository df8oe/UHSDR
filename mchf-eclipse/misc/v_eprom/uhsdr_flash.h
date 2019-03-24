/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
  ******************************************************************************
  * @file    EEPROM_Emulation/inc/eeprom.h
  * @author  MCD Application Team
   * @version V1.0.0
  * @date    10-October-2011
  * @brief   This file contains all the functions prototypes for the EEPROM
  *          emulation firmware library.
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
#ifndef __UHSDR_FLASH_H
#define __UHSDR_FLASH_H

/* Exported constants --------------------------------------------------------*/
#if defined(STM32F4) || defined(STM32F7)
    /* Define the size of the sectors to be used */
    #define PAGE_SIZE               ((uint32_t)0x4000)  /* Page size = 16KByte */


    /* EEPROM start address in Flash */
    #define EEPROM_START_ADDRESS  ((uint32_t)0x08008000) /* EEPROM emulation start address:
                                                            from sector2 : after 32KByte of used
                                                            Flash memory */
    #define PAGE0_ID               FLASH_SECTOR_2
    #define PAGE1_ID               FLASH_SECTOR_3
#elif defined(STM32H7)
    /* Define the size of the sectors to be used */
    #define PAGE_SIZE               (uint32_t)0x20000  /* Page size = 128KByte */
    /* EEPROM start address in Flash */
    #define EEPROM_START_ADDRESS  ((uint32_t)0x08020000) /* EEPROM emulation start address:
                                                            from sector1 : after 128KByte of used
                                                            Flash memory */
    #define PAGE0_ID               FLASH_SECTOR_1
    #define PAGE1_ID               FLASH_SECTOR_2
#endif


uint16_t Flash_Init();
uint16_t Flash_ReadVariable(uint16_t addr, uint16_t* value);
uint16_t Flash_WriteVariable(uint16_t addr, uint16_t value);
uint16_t Flash_UpdateVariable(uint16_t addr, uint16_t value);

#endif /* __UHSDR_FLASH_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
