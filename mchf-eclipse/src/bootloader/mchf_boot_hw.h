/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
#ifndef __MCHF_H
#define __MCHF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f407xx.h"
#include "mchf_board_config.h"

typedef enum
{
    LEDGREEN = 0,
    LEDRED = 1,
    PWR_HOLD = 2,
    BACKLIGHT = 3
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

#define LEDn                            4

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


#define POWER_BUTTON_PIN                GPIO_PIN_13
#define POWER_BUTTON_GPIO_PORT          GPIOC
#define POWER_BUTTON_EXTI_LINE          EXTI_Line0
#define POWER_BUTTON_EXTI_IRQn          EXTI0_IRQn


void mchfBl_LEDInit(Led_TypeDef Led);
void mchfBl_PinOn(Led_TypeDef Led);
void mchfBl_PinOff(Led_TypeDef Led);
void mchfBl_PinToggle(Led_TypeDef Led);
void mchfBl_ButtonInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
uint32_t mchfBl_ButtonGetState(Button_TypeDef Button);

inline void mcHF_PowerHoldOff()
{
    mchfBl_PinOn(PWR_HOLD);
}

inline void mcHF_PowerHoldOn()
{
    mchfBl_PinOff(PWR_HOLD);
}


#endif   /* __MCHF_H */
