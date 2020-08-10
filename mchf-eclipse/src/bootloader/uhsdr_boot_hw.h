/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __MCHF_H
#define __MCHF_H

/* Includes ------------------------------------------------------------------*/
#include "uhsdr_board_config.h"
#include "uhsdr_board.h"
#include "gpio.h"

typedef enum
{
    LEDGREEN = 0,
    LEDRED = 1,
    PWR_HOLD = 2,
    BACKLIGHT = 3,
    TPCS_PIN = 4
} Led_TypeDef;

typedef enum
{
    BUTTON_BANDM = 0,
    BUTTON_POWER = 1,
    BUTTON_BANDP = 2
} Button_TypeDef;

typedef enum
{
    BUTTON_MODE_GPIO = 0,
    BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;

#define LEDn                            5

#define LEDGREEN_PIN                    GPIO_PIN_9
#define LEDGREEN_GPIO_PORT              GPIOB

#define LEDRED_PIN                      GPIO_PIN_12
#define LEDRED_GPIO_PORT                GPIOB

#define POWER_ON_PIN                    GPIO_PIN_8
#define POWER_ON_GPIO_PORT              GPIOC

#define BACKLIGHT_ON_PIN                GPIO_PIN_2
#define BACKLIGHT_ON_GPIO_PORT          GPIOD


#define BUTTONn                         3

#define BANDM_BUTTON_PIN                GPIO_PIN_0
#define BANDM_BUTTON_GPIO_PORT          GPIOB
#define BANDM_BUTTON_EXTI_IRQn          EXTI0_IRQn

#define BANDP_BUTTON_PIN                GPIO_PIN_2
#define BANDP_BUTTON_GPIO_PORT          GPIOB
#define BANDP_BUTTON_EXTI_IRQn          EXTI0_IRQn

#if  defined(UI_BRD_MCHF)
#define POWER_BUTTON_PIN                GPIO_PIN_13
#define POWER_BUTTON_GPIO_PORT          GPIOC
#define POWER_BUTTON_EXTI_LINE          EXTI_Line0
#define POWER_BUTTON_EXTI_IRQn          EXTI0_IRQn
#elif  defined(UI_BRD_OVI40)
#define POWER_BUTTON_PIN                GPIO_PIN_2
#define POWER_BUTTON_GPIO_PORT          GPIOG
#define POWER_BUTTON_EXTI_LINE          EXTI_Line0
#define POWER_BUTTON_EXTI_IRQn          EXTI0_IRQn
#endif

void mchfBl_LEDInit(Led_TypeDef Led);
void mchfBl_PinOn(Led_TypeDef Led);
void mchfBl_PinOff(Led_TypeDef Led);
void mchfBl_PinToggle(Led_TypeDef Led);
void mchfBl_ButtonInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
uint32_t mchfBl_ButtonGetState(Button_TypeDef Button);

void mcHF_PowerOff(void);

/*
 * Just toggles the PowerHold Pin, but does not stop execution
 */
static inline void Bootloader_PowerHoldOff()
{
    mchfBl_PinOn(PWR_HOLD);
}

static inline void Bootloader_PowerHoldOn()
{
    mchfBl_PinOff(PWR_HOLD);
}


#endif   /* __MCHF_H */
