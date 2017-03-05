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
**  Licence:		GNU GPLv3                                                  **
************************************************************************************/


#include "mchf_boot_hw.h"


GPIO_TypeDef* GPIO_PORT[LEDn] = {LEDGREEN_GPIO_PORT, LEDRED_GPIO_PORT, ON_GPIO_PORT, BLON_GPIO_PORT};
const uint16_t GPIO_PIN[LEDn] = {LEDGREEN_PIN, LEDRED_PIN, ON_PIN, BLON_PIN};

GPIO_TypeDef* BUTTON_PORT[BUTTONn] = {BANDM_BUTTON_GPIO_PORT,POWER_BUTTON_GPIO_PORT};

const uint16_t BUTTON_PIN[BUTTONn] = {BANDM_BUTTON_PIN,POWER_BUTTON_PIN};

const uint8_t BUTTON_IRQn[BUTTONn] = {BANDM_BUTTON_EXTI_IRQn,POWER_BUTTON_EXTI_IRQn};

void STM_EVAL_LEDInit(Led_TypeDef Led)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Enable the GPIO_LED Clock */

    /* Configure the GPIO_LED pin */
    GPIO_InitStructure.Pin = GPIO_PIN[Led];
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

    HAL_GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);
}

void STM_EVAL_LEDOn(Led_TypeDef Led)
{
    GPIO_PORT[Led]->BSRR = GPIO_PIN[Led];
}

void STM_EVAL_LEDOff(Led_TypeDef Led)
{
    GPIO_PORT[Led]->BSRR = GPIO_PIN[Led] << 16U;
}

void STM_EVAL_LEDToggle(Led_TypeDef Led)
{
    GPIO_PORT[Led]->ODR ^= GPIO_PIN[Led];
}

void STM_EVAL_PBInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable the BUTTON Clock */

    /* Configure Button pin as input */
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Pin = BUTTON_PIN[Button];
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

    if (Button_Mode == BUTTON_MODE_EXTI)
    {
        /* Configure Button EXTI line */
        GPIO_InitStructure.Mode = GPIO_MODE_EVT_FALLING;


        /* Enable and set Button EXTI Interrupt to the lowest priority */
        HAL_NVIC_EnableIRQ(BUTTON_IRQn[Button]);
        HAL_NVIC_SetPriority(BUTTON_IRQn[Button],15,0);
    }
    HAL_GPIO_Init(BUTTON_PORT[Button], &GPIO_InitStructure);
}

uint32_t STM_EVAL_PBGetState(Button_TypeDef Button)
{
    return HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}
