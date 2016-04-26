/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name: bootloader_it.c                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		CC BY-NC-SA 3.0                                            **
************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_hcd_int.h"
#include "usbh_usr.h"
#include "stm32f4xx_it.h"


extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                    USB_Host;

extern __IO uint32_t UploadCondition;
extern __IO uint32_t TimingDelay;
__IO uint8_t Counter = 0x00;

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    if (UploadCondition != 0x00)
    {
        /* Check if User button Pressed */
        if (STM_EVAL_PBGetState(BUTTON_BANDM) == Bit_RESET)
        {
            if (TimingDelay != 0x00)
            {
                TimingDelay--;
            }
            else
            {
                if (Counter < 100)
                {
                    Counter++;
                }
                else
                {
                    Counter = 0x00;
                }
            }
        }
        else
        {
            if (TimingDelay != 0x00)
            {
                UploadCondition = 0x00;
            }
        }
    }
    else
    {
        TimingDelay_Decrement();
    }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        USB_Host.usr_cb->OverCurrentDetected();
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void TIM2_IRQHandler(void)
{
    USB_OTG_BSP_TimerIRQ();
}

void OTG_FS_IRQHandler(void)
{
    USBH_OTG_ISR_Handler(&USB_OTG_Core);
}
