/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
  ******************************************************************************
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   Library configuration file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4xx_CONF_H
#define __STM32F4xx_CONF_H

#if defined  (HSE_VALUE)
/* Redefine the HSE value; it's equal to 16 MHz on the mchf */
#undef HSE_VALUE
#define HSE_VALUE    ((uint32_t)16000000)
#endif /* HSE_VALUE */

/* Includes ------------------------------------------------------------------*/
/* Uncomment the line below to enable peripheral header file inclusion */
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_wwdg.h"
#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* If an external clock source is used, then the value of the following define
   should be set to the value of the external clock source, else, if no external
   clock is used, keep this define commented */
/*#define I2S_EXTERNAL_CLOCK_VAL   12288000 */ /* Value of the external clock in Hz */


/* Uncomment the line below to expanse the "assert_param" macro in the
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#define assert_param(expr) ((void)0)

#endif /* __STM32F4xx_CONF_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
