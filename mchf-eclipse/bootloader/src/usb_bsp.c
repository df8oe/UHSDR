/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
  ******************************************************************************
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   This file implements the board support package for the USB host library
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

/* Includes ------------------------------------------------------------------*/
#include "usb_bsp.h"
#include "mchf.h"

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#define USE_ACCURATE_TIME
#define TIM_MSEC_DELAY       0x01
#define TIM_USEC_DELAY       0x02

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ErrorStatus HSEStartUpStatus;

#ifdef USE_ACCURATE_TIME
__IO uint32_t BSP_delay = 0;
#endif

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
#ifdef USE_ACCURATE_TIME
static void BSP_SetTime(uint8_t Unit);
static void BSP_Delay(uint32_t nTime, uint8_t Unit);
static void USB_OTG_BSP_TimeInit ( void );
#endif

/**
  * @brief  BSP_Init
  *         board user initializations
  * @param  None
  * @retval None
  */
void BSP_Init(void)
{
    /* Initialize LEDs and User_Button on mchf --------------------*/
    STM_EVAL_PBInit(BUTTON_BANDM, BUTTON_MODE_GPIO);
    STM_EVAL_PBInit(BUTTON_POWER, BUTTON_MODE_GPIO);

    STM_EVAL_LEDInit(LEDGREEN);
    STM_EVAL_LEDInit(LEDRED);
    STM_EVAL_LEDInit(ON);
    STM_EVAL_LEDInit(BLON);
}

/**
  * @brief  USB_OTG_BSP_Init
  *         Initilizes BSP configurations
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
    GPIO_InitTypeDef GPIO_InitStructure;
#ifdef USE_USB_OTG_FS

    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE);

    /* Configure SOF VBUS ID DM DP Pins */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9  |
                                  GPIO_Pin_11 |
                                  GPIO_Pin_12;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_OTG1_FS) ;
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ;
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;

    /* this for ID line debug */


    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_OTG1_FS) ;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ;
#else // USE_USB_OTG_HS 
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_14 | GPIO_Pin_15;  // Data - / +

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_OTG2_FS);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_OTG2_FS);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, ENABLE);

//	RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR,ENABLE);

#endif //USB_OTG_HS

    /* Intialize Timer for delay function */
    USB_OTG_BSP_TimeInit();
}

/**
  * @brief  USB_OTG_BSP_EnableInterrupt
  *         Configures USB Global interrupt
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{

    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable USB Interrupt */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

#ifdef USE_USB_OTG_FS
    NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
#else
    NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
#endif
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  state : VBUS states
  * @retval None
  */
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{
}

/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  None
  * @retval None
  */
void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_BSP_mDelay(200);   /* Delay is need for stabilising the Vbus Low
  in Reset Condition, when Vbus=1 and Reset-button is pressed by user */
}

/**
  * @brief  USB_OTG_BSP_TimeInit
  *         Initializes delay unit using Timer2
  * @param  None
  * @retval None
  */
static void USB_OTG_BSP_TimeInit (void)
{
#ifdef USE_ACCURATE_TIME
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Set the Vector Table base address at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);

    /* Configure the Priority Group to 2 bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* Enable the TIM2 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
#endif
}

/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
void USB_OTG_BSP_uDelay (const uint32_t usec)
{

#ifdef USE_ACCURATE_TIME
    BSP_Delay(usec, TIM_USEC_DELAY);
#else
    __IO uint32_t count = 0;
    const uint32_t utime = (120 * usec / 7);
    do
    {
        if ( ++count > utime )
        {
            return ;
        }
    }
    while (1);
#endif

}

/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
#ifdef USE_ACCURATE_TIME
    BSP_Delay(msec, TIM_MSEC_DELAY);
#else
    USB_OTG_BSP_uDelay(msec * 1000);
#endif
}

/**
  * @brief  USB_OTG_BSP_TimerIRQ
  *         Time base IRQ
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_TimerIRQ (void)
{
#ifdef USE_ACCURATE_TIME
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        if (BSP_delay > 0x00)
        {
            BSP_delay--;
        }
        else
        {
            TIM_Cmd(TIM2, DISABLE);
        }
    }
#endif
}

#ifdef USE_ACCURATE_TIME
/**
  * @brief  BSP_Delay
  *         Delay routine based on TIM2
  * @param  nTime : Delay Time
  * @param  unit : Delay Time unit : mili sec / micro sec
  * @retval None
  */
static void BSP_Delay(uint32_t nTime, uint8_t unit)
{

    BSP_delay = nTime;
    BSP_SetTime(unit);
    while (BSP_delay != 0);
    TIM_Cmd(TIM2, DISABLE);
}

/**
  * @brief  BSP_SetTime
  *         Configures TIM2 for delay routine based on TIM2
  * @param  unit : msec /usec
  * @retval None
  */
static void BSP_SetTime(uint8_t unit)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    TIM_Cmd(TIM2, DISABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);


    if (unit == TIM_USEC_DELAY)
    {
        TIM_TimeBaseStructure.TIM_Period = 11;
    }
    else if (unit == TIM_MSEC_DELAY)
    {
        TIM_TimeBaseStructure.TIM_Period = 11999;
    }
    TIM_TimeBaseStructure.TIM_Prescaler = 5;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    TIM_ARRPreloadConfig(TIM2, ENABLE);

    /* TIM IT enable */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    /* TIM2 enable counter */
    TIM_Cmd(TIM2, ENABLE);
}
#endif


/*
void OTG_HS_IRQHandler(void)
{
    USBD_OTG_ISR_Handler(&USB_OTG_dev);
}
*/
