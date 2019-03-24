/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "uhsdr_board.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern HCD_HandleTypeDef hhcd_USB_OTG_HS;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern DMA_HandleTypeDef hdma_i2s3_ext_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;

/*
 * TODO: Well, we don't want leds in the bootloader fault handlers.  We should decide about a general fault debugging concept
 * for all platforms
 */

#if  !defined(BOOTLOADER_BUILD)
#define GreenLed(a) Board_GreenLed(a)
#define RedLed(a) Board_RedLed(a)
#else
#define GreenLed(a)
#define RedLed(a)
#endif

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
    GreenLed(LED_STATE_OFF);
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

// TODO: Factor out and make avail on all platforms
#if  !defined(BOOTLOADER_BUILD)

void Debug_WaitShort()
{
    for (int i = 20000000;i;i--)
    {
        asm("nop");
    }
}

void Debug_WaitLong()
{
    for(int i=0; i < 6; i++)
    {
        Debug_WaitShort();
    }
}

void Debug_Blink_Bit(uint32_t what)
{
    RedLed(LED_STATE_OFF);
    GreenLed(LED_STATE_OFF);
    Debug_WaitShort();

    GreenLed(LED_STATE_ON);
    if (what)
    {
        RedLed(LED_STATE_ON);
    }
    Debug_WaitShort();
    RedLed(LED_STATE_OFF);
    GreenLed(LED_STATE_OFF);
}
void Debug_Blink_RedLed()
{
    RedLed(LED_STATE_ON);
    Debug_WaitShort();
    RedLed(LED_STATE_OFF);
    Debug_WaitShort();
}

void Debug_Send_RedBlinks(int n)
{
    for (int i=n; i; i--)
    {
        Debug_Blink_RedLed();
    }
}

void Debug_Send_32Bits(uint32_t val)
{
    for (int i=0;i<32;i++)
    {
        Debug_Blink_Bit(val & 0x80000000);
        val<<=1;
    }
}

void Debug_Send_ValueAndId(uint32_t val, int id)
{
    RedLed(LED_STATE_OFF);
    GreenLed(LED_STATE_OFF);
    Debug_WaitLong();
    Debug_Send_RedBlinks(id);
    Debug_WaitLong();
    Debug_Send_32Bits(val);
}

volatile uint32_t r0  __attribute__ ((unused));
volatile uint32_t r1 __attribute__ ((unused));
volatile uint32_t r2 __attribute__ ((unused));
volatile uint32_t r3 __attribute__ ((unused));
volatile uint32_t r5 __attribute__ ((unused));
volatile uint32_t r12 __attribute__ ((unused));
volatile uint32_t lr __attribute__ ((unused)); /* Link register. */
volatile uint32_t pc __attribute__ ((unused)); /* Program counter. */
volatile uint32_t psr __attribute__ ((unused));/* Program status register. */
volatile uint32_t cfsr __attribute__ ((unused)); /* Program counter. */
volatile uint32_t bfar __attribute__ ((unused));/* Program status register. */

static void Debug_FaultGetRegistersFromStack( uint32_t *pulFaultStackAddress, uint32_t r5x)
{
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];
    cfsr = SCB->CFSR;
    bfar = SCB->BFAR;
    r5 = r5x;

    while (1)
    {
        Debug_Send_ValueAndId(SCB->CFSR,1);
        Debug_Send_ValueAndId(SCB->BFAR,2);
        Debug_Send_ValueAndId(pc,3);
        Debug_Send_ValueAndId(r0,4);
        Debug_Send_ValueAndId(r5,5);
    }
}

/**
* @brief This function handles Hard fault interrupt.
*/


void HardFault_Handler( void ) __attribute__( ( naked ) );

void HardFault_Handler(void)
{
    __asm volatile
        (
            " tst lr, #4                                                \n"
            " ite eq                                                    \n"
            " mrseq r0, msp                                             \n"
            " mrsne r0, psp                                             \n"
            " mov r1, r5                                                \n"
            " ldr r2, [r0, #24]                                         \n"
            " mov r3, %0                                                \n"
            " bx r3                                                     \n"
            : : "l" (Debug_FaultGetRegistersFromStack)
        );

   /* USER CODE BEGIN HardFault_IRQn 0 */
  // UiLcdHy28_PrintText(0,0,"Hello",White,Black,1);

  /* USER CODE END HardFault_IRQn 0 */
  /* USER CODE BEGIN HardFault_IRQn 1 */

  /* USER CODE END HardFault_IRQn 1 */
}
#else
void HardFault_Handler()
{
/* USER CODE BEGIN HardFault_IRQn 0 */
/* USER CODE END HardFault_IRQn 0 */
/* USER CODE BEGIN HardFault_IRQn 1 */
    while(1)
    {
    }
/* USER CODE END HardFault_IRQn 1 */
}
#endif

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  GreenLed(LED_STATE_OFF);
  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
  }
  /* USER CODE BEGIN MemoryManagement_IRQn 1 */

  /* USER CODE END MemoryManagement_IRQn 1 */
}

/**
* @brief This function handles Pre-fetch fault, memory access fault.
*/
#if 0
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
  }
  /* USER CODE BEGIN BusFault_IRQn 1 */

  /* USER CODE END BusFault_IRQn 1 */
}
#endif
/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
    RedLed(LED_STATE_OFF);
  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
  }
  /* USER CODE BEGIN UsageFault_IRQn 1 */

  /* USER CODE END UsageFault_IRQn 1 */
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */
    // GreenLed(LED_STATE_OFF);
  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
* @brief This function handles Pendable request for system service.
*/
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
#ifndef BOOTLOADER_BUILD
    UiDriver_TaskHandler_HighPrioTasks();
#endif
  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles EXTI line0 interrupt.
*/
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
* @brief This function handles EXTI line1 interrupt.
*/
void EXTI1_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI1_IRQn 0 */

  /* USER CODE END EXTI1_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
  /* USER CODE BEGIN EXTI1_IRQn 1 */

  /* USER CODE END EXTI1_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream2 global interrupt.
*/
void DMA1_Stream2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream2_IRQn 0 */

  /* USER CODE END DMA1_Stream2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_i2s3_ext_rx);
  /* USER CODE BEGIN DMA1_Stream2_IRQn 1 */

  /* USER CODE END DMA1_Stream2_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream4 global interrupt.
*/
void DMA1_Stream4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream4_IRQn 0 */

  /* USER CODE END DMA1_Stream4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi2_tx);
  /* USER CODE BEGIN DMA1_Stream4_IRQn 1 */

  /* USER CODE END DMA1_Stream4_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream5 global interrupt.
*/
void DMA1_Stream5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */

  /* USER CODE END DMA1_Stream5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi3_tx);
  /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */

  /* USER CODE END DMA1_Stream5_IRQn 1 */
}

/**
* @brief This function handles EXTI line[15:10] interrupts.
*/
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
* @brief This function handles USB On The Go FS global interrupt.
*/
#ifndef BOOTLOADER_BUILD
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}
#endif
#if defined(USE_USBHOST) || defined(BOOTLOADER_BUILD)
/**
* @brief This function handles USB On The Go HS global interrupt.
*/
void OTG_HS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_IRQn 0 */

  /* USER CODE END OTG_HS_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS);
  /* USER CODE BEGIN OTG_HS_IRQn 1 */

  /* USER CODE END OTG_HS_IRQn 1 */
}
/* USER CODE BEGIN 1 */
#endif
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
