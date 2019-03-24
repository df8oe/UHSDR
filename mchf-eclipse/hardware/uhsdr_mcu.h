/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/
#ifndef __MCHF_MCU_H
#define __MCHF_MCU_H

#include "uhsdr_board_config.h"

typedef enum {
    CPU_NONE = 0,
    CPU_STM32F4 = 1,
    CPU_STM32F7 = 2,
    CPU_STM32H7 = 3,
} mchf_cpu_t;

inline mchf_cpu_t MchfHW_Cpu()
{
    mchf_cpu_t retval = CPU_NONE;
#if defined(STM32F4)
    retval = CPU_STM32F4;
#elif  defined(STM32F7)
    retval = CPU_STM32F7;
#elif  defined(STM32H7)
    retval = CPU_STM32H7;
#endif
    return retval;
}

#define STM32_GetRevision()     (*(uint16_t *) (UID_BASE + 2))
#define STM32_GetSignature()    ((*(uint16_t *) (DBGMCU_BASE)) & 0x0FFF)
#define STM32_UUID ((uint32_t *)UID_BASE)

#if defined(STM32F4) || defined(STM32F7)
    #define STM32_GetFlashSize()    (*(uint16_t *) (FLASHSIZE_BASE))
#elif defined(STM32H7)
    #define STM32_GetFlashSize()    (FLASH_SIZE/1024)
    #define FLASHSIZE_BASE 0x1FF1E880
    #define SRAM2_BASE 0x38000000

#endif

#if defined(STM32H7)
    #define GPIO_SetBits(PORT,PINS) { (PORT)->BSRRL = (PINS); }
    #define GPIO_ResetBits(PORT,PINS) { (PORT)->BSRRH = (PINS); }
#elif defined(STM32F7) || defined(STM32F4)
    #define GPIO_SetBits(PORT,PINS) { (PORT)->BSRR = (PINS); }
    #define GPIO_ResetBits(PORT,PINS) { (PORT)->BSRR = (PINS) << 16U; }
#endif

#define GPIO_ToggleBits(PORT,PINS) { (PORT)->ODR ^= (PINS); }
#define GPIO_ReadInputDataBit(PORT,PINS) { ((PORT)->IDR = (PINS); }

#endif
