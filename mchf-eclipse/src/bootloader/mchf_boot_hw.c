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


static GPIO_TypeDef* GPIO_PORT[LEDn] = {LEDGREEN_GPIO_PORT, LEDRED_GPIO_PORT, POWER_ON_GPIO_PORT, BACKLIGHT_ON_GPIO_PORT};
static const uint16_t GPIO_PIN[LEDn] = {LEDGREEN_PIN, LEDRED_PIN, POWER_ON_PIN, BACKLIGHT_ON_PIN};

static GPIO_TypeDef* BUTTON_PORT[BUTTONn] = {BANDM_BUTTON_GPIO_PORT,POWER_BUTTON_GPIO_PORT, BANDP_BUTTON_GPIO_PORT};

static const uint16_t BUTTON_PIN[BUTTONn] = {BANDM_BUTTON_PIN,POWER_BUTTON_PIN, BANDP_BUTTON_PIN};

static const uint8_t BUTTON_IRQn[BUTTONn] = {BANDM_BUTTON_EXTI_IRQn,POWER_BUTTON_EXTI_IRQn,BANDP_BUTTON_EXTI_IRQn};

void mchfBl_PortClock(uint32_t port)
{

    /* Enable the BUTTON Clock */
    switch(port)
    {
    case (uint32_t)GPIOA:
        __HAL_RCC_GPIOA_CLK_ENABLE();
        break;
    case (uint32_t)GPIOB:
        __HAL_RCC_GPIOB_CLK_ENABLE();
        break;
    case (uint32_t)GPIOC:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
    case (uint32_t)GPIOD:
        __HAL_RCC_GPIOD_CLK_ENABLE();
        break;
#if 0
    case (uint32_t)GPIOE:
        __HAL_RCC_GPIOE_CLK_ENABLE();
        break;
    case (uint32_t)GPIOF:
        __HAL_RCC_GPIOF_CLK_ENABLE();
        break;
    case (uint32_t)GPIOG:
        __HAL_RCC_GPIOG_CLK_ENABLE();
        break;
    case (uint32_t)GPIOH:
        __HAL_RCC_GPIOH_CLK_ENABLE();
        break;
    case (uint32_t)GPIOI:
        __HAL_RCC_GPIOI_CLK_ENABLE();
        break;
#endif
    }
}

void mchfBl_LEDInit(Led_TypeDef Led)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    mchfBl_PortClock((uint32_t)GPIO_PORT[Led]);
    /* Enable the GPIO_LED Clock */

    /* Configure the GPIO_LED pin */
    GPIO_InitStructure.Pin = GPIO_PIN[Led];
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

    HAL_GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);
}

void mchfBl_PinOn(Led_TypeDef Led)
{
    GPIO_PORT[Led]->BSRR = GPIO_PIN[Led];
}

void mchfBl_PinOff(Led_TypeDef Led)
{
    GPIO_PORT[Led]->BSRR = GPIO_PIN[Led] << 16U;
}

void mchfBl_PinToggle(Led_TypeDef Led)
{
    GPIO_PORT[Led]->ODR ^= GPIO_PIN[Led];
}

void mchfBl_ButtonInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    mchfBl_PortClock((uint32_t)BUTTON_PORT[Button]);

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

uint32_t mchfBl_ButtonGetState(Button_TypeDef Button)
{
    return HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}
