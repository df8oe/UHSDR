/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
#ifndef __MCHF_H
#define __MCHF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"


char use_usba;

typedef enum
{
    LEDGREEN = 0,
    LEDRED = 1,
    ON = 2,
    BLON = 3
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

#define LEDn                             4

#define LEDGREEN_PIN                     GPIO_Pin_9
#define LEDGREEN_GPIO_PORT               GPIOB
#define LEDGREEN_GPIO_CLK                RCC_AHB1Periph_GPIOB

#define LEDRED_PIN                       GPIO_Pin_12
#define LEDRED_GPIO_PORT                 GPIOB
#define LEDRED_GPIO_CLK                  RCC_AHB1Periph_GPIOB

#define ON_PIN                           GPIO_Pin_8
#define ON_GPIO_PORT                     GPIOC
#define ON_GPIO_CLK                      RCC_AHB1Periph_GPIOC

#define BLON_PIN                         GPIO_Pin_2
#define BLON_GPIO_PORT                   GPIOD
#define BLON_GPIO_CLK                    RCC_AHB1Periph_GPIOD

#define BUTTONn                          3

#define BANDM_BUTTON_PIN                GPIO_Pin_0
#define BANDM_BUTTON_GPIO_PORT          GPIOB
#define BANDM_BUTTON_GPIO_CLK           RCC_AHB1Periph_GPIOB
#define BANDM_BUTTON_EXTI_LINE          EXTI_Line0
#define BANDM_BUTTON_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOB
#define BANDM_BUTTON_EXTI_PIN_SOURCE    EXTI_PinSource0
#define BANDM_BUTTON_EXTI_IRQn          EXTI0_IRQn

#define BANDP_BUTTON_PIN                GPIO_Pin_2
#define BANDP_BUTTON_GPIO_PORT          GPIOB
#define BANDP_BUTTON_GPIO_CLK           RCC_AHB1Periph_GPIOB
#define BANDP_BUTTON_EXTI_LINE          EXTI_Line0
#define BANDP_BUTTON_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOB
#define BANDP_BUTTON_EXTI_PIN_SOURCE    EXTI_PinSource2
#define BANDP_BUTTON_EXTI_IRQn          EXTI0_IRQn

#define POWER_BUTTON_PIN                GPIO_Pin_13
#define POWER_BUTTON_GPIO_PORT          GPIOC
#define POWER_BUTTON_GPIO_CLK           RCC_AHB1Periph_GPIOC
#define POWER_BUTTON_EXTI_LINE          EXTI_Line0
#define POWER_BUTTON_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOC
#define POWER_BUTTON_EXTI_PIN_SOURCE    EXTI_PinSource13
#define POWER_BUTTON_EXTI_IRQn          EXTI0_IRQn


void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDToggle(Led_TypeDef Led);
void STM_EVAL_PBInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
uint32_t STM_EVAL_PBGetState(Button_TypeDef Button);

#endif   /* __MCHF_H */
