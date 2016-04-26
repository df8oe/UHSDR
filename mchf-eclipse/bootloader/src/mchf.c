/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name: mchf.c                                                              **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		CC BY-NC-SA 3.0                                            **
************************************************************************************/


#include "mchf.h"


GPIO_TypeDef* GPIO_PORT[LEDn] = {LEDGREEN_GPIO_PORT, LEDRED_GPIO_PORT, ON_GPIO_PORT, BLON_GPIO_PORT};
const uint16_t GPIO_PIN[LEDn] = {LEDGREEN_PIN, LEDRED_PIN, ON_PIN, BLON_PIN};
const uint32_t GPIO_CLK[LEDn] = {LEDGREEN_GPIO_CLK, LEDRED_GPIO_CLK, ON_GPIO_CLK, BLON_GPIO_CLK};

GPIO_TypeDef* BUTTON_PORT[BUTTONn] = {BANDM_BUTTON_GPIO_PORT,POWER_BUTTON_GPIO_PORT};

const uint16_t BUTTON_PIN[BUTTONn] = {BANDM_BUTTON_PIN,POWER_BUTTON_PIN};

const uint32_t BUTTON_CLK[BUTTONn] = {BANDM_BUTTON_GPIO_CLK,POWER_BUTTON_GPIO_CLK};

const uint16_t BUTTON_EXTI_LINE[BUTTONn] = {BANDM_BUTTON_EXTI_LINE,POWER_BUTTON_EXTI_LINE};

const uint8_t BUTTON_PORT_SOURCE[BUTTONn] = {BANDM_BUTTON_EXTI_PORT_SOURCE,POWER_BUTTON_EXTI_PORT_SOURCE};

const uint8_t BUTTON_PIN_SOURCE[BUTTONn] = {BANDM_BUTTON_EXTI_PIN_SOURCE,POWER_BUTTON_EXTI_PIN_SOURCE};

const uint8_t BUTTON_IRQn[BUTTONn] = {BANDM_BUTTON_EXTI_IRQn,POWER_BUTTON_EXTI_IRQn};

NVIC_InitTypeDef   NVIC_InitStructure;

void STM_EVAL_LEDInit(Led_TypeDef Led)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Enable the GPIO_LED Clock */
    RCC_AHB1PeriphClockCmd(GPIO_CLK[Led], ENABLE);

    /* Configure the GPIO_LED pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);
}

void STM_EVAL_LEDOn(Led_TypeDef Led)
{
    GPIO_PORT[Led]->BSRRL = GPIO_PIN[Led];
}

void STM_EVAL_LEDOff(Led_TypeDef Led)
{
    GPIO_PORT[Led]->BSRRH = GPIO_PIN[Led];
}

void STM_EVAL_LEDToggle(Led_TypeDef Led)
{
    GPIO_PORT[Led]->ODR ^= GPIO_PIN[Led];
}

void STM_EVAL_PBInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the BUTTON Clock */
    RCC_AHB1PeriphClockCmd(BUTTON_CLK[Button], ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Configure Button pin as input */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN[Button];
    GPIO_Init(BUTTON_PORT[Button], &GPIO_InitStructure);

    if (Button_Mode == BUTTON_MODE_EXTI)
    {
        /* Connect Button EXTI Line to Button GPIO Pin */
        SYSCFG_EXTILineConfig(BUTTON_PORT_SOURCE[Button], BUTTON_PIN_SOURCE[Button]);

        /* Configure Button EXTI line */
        EXTI_InitStructure.EXTI_Line = BUTTON_EXTI_LINE[Button];
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* Enable and set Button EXTI Interrupt to the lowest priority */
        NVIC_InitStructure.NVIC_IRQChannel = BUTTON_IRQn[Button];
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

        NVIC_Init(&NVIC_InitStructure);
    }
}

uint32_t STM_EVAL_PBGetState(Button_TypeDef Button)
{
    return GPIO_ReadInputDataBit(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}
