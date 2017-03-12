/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
#ifndef __MCHF_H
#define __MCHF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f407xx.h"

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
    BUTTON_POWER = 1
} Button_TypeDef;

typedef enum
{
    BUTTON_MODE_GPIO = 0,
    BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;

#define LEDn                             4

#define LEDGREEN_PIN                     GPIO_PIN_9
#define LEDGREEN_GPIO_PORT               GPIOB

#define LEDRED_PIN                       GPIO_PIN_12
#define LEDRED_GPIO_PORT                 GPIOB
#define LEDRED_GPIO_CLK                  RCC_AHB1Periph_GPIOB

#define POWER_ON_PIN                           GPIO_PIN_8
#define POWER_ON_GPIO_PORT                     GPIOC

#define BACKLIGHT_ON_PIN                         GPIO_PIN_2
#define BACKLIGHT_ON_GPIO_PORT                   GPIOD

#define BUTTONn                          2

#define BANDM_BUTTON_PIN                GPIO_PIN_0
#define BANDM_BUTTON_GPIO_PORT          GPIOB
#define BANDM_BUTTON_GPIO_CLK           RCC_AHB1Periph_GPIOB
#define BANDM_BUTTON_EXTI_IRQn          EXTI0_IRQn

#define POWER_BUTTON_PIN                GPIO_PIN_13
#define POWER_BUTTON_GPIO_PORT          GPIOC
#define POWER_BUTTON_GPIO_CLK           RCC_AHB1Periph_GPIOC
#define POWER_BUTTON_EXTI_LINE          EXTI_Line0
#define POWER_BUTTON_EXTI_IRQn          EXTI0_IRQn


void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDToggle(Led_TypeDef Led);
void STM_EVAL_PBInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
uint32_t STM_EVAL_PBGetState(Button_TypeDef Button);

inline void mcHF_PowerHoldOff()
{
    STM_EVAL_LEDOn(PWR_HOLD);
}

inline void mcHF_PowerHoldOn()
{
    STM_EVAL_LEDOff(PWR_HOLD);
}


#endif   /* __MCHF_H */
