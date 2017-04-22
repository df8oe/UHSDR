/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define Button_F2_Pin GPIO_PIN_2
#define Button_F2_GPIO_Port GPIOE
#define Button_F4_Pin GPIO_PIN_3
#define Button_F4_GPIO_Port GPIOE
#define Button_G3_Pin GPIO_PIN_4
#define Button_G3_GPIO_Port GPIOE
#define Button_F5_Pin GPIO_PIN_5
#define Button_F5_GPIO_Port GPIOE
#define Button_M1_Pin GPIO_PIN_0
#define Button_M1_GPIO_Port GPIOF
#define opt_FRAM_CS_Pin GPIO_PIN_1
#define opt_FRAM_CS_GPIO_Port GPIOF
#define opt_FLASH_CS_Pin GPIO_PIN_2
#define opt_FLASH_CS_GPIO_Port GPIOF
#define CS_SD_Card_Pin GPIO_PIN_4
#define CS_SD_Card_GPIO_Port GPIOF
#define opt_SW_E2_Pin GPIO_PIN_5
#define opt_SW_E2_GPIO_Port GPIOF
#define opt_SW_E3_Pin GPIO_PIN_10
#define opt_SW_E3_GPIO_Port GPIOF
#define Button_M3_Pin GPIO_PIN_1
#define Button_M3_GPIO_Port GPIOC
#define opt_SW_SD_Pin GPIO_PIN_7
#define opt_SW_SD_GPIO_Port GPIOA
#define Button_SM_Pin GPIO_PIN_4
#define Button_SM_GPIO_Port GPIOC
#define Button_SP_Pin GPIO_PIN_5
#define Button_SP_GPIO_Port GPIOC
#define Button_BM_Pin GPIO_PIN_0
#define Button_BM_GPIO_Port GPIOB
#define PTT_ON_Pin GPIO_PIN_1
#define PTT_ON_GPIO_Port GPIOB
#define Button_BP_Pin GPIO_PIN_2
#define Button_BP_GPIO_Port GPIOB
#define opt_Button_S18_Pin GPIO_PIN_11
#define opt_Button_S18_GPIO_Port GPIOF
#define LED_BLUE_Pin GPIO_PIN_12
#define LED_BLUE_GPIO_Port GPIOF
#define opt_SW_E1_Pin GPIO_PIN_13
#define opt_SW_E1_GPIO_Port GPIOF
#define BAND0_Pin GPIO_PIN_0
#define BAND0_GPIO_Port GPIOG
#define opt_BAND3_Pin GPIO_PIN_1
#define opt_BAND3_GPIO_Port GPIOG
#define LED_RED_Pin GPIO_PIN_12
#define LED_RED_GPIO_Port GPIOB
#define Power_Button_Pin GPIO_PIN_2
#define Power_Button_GPIO_Port GPIOG
#define TP_IRQ_Pin GPIO_PIN_4
#define TP_IRQ_GPIO_Port GPIOG
#define Audio_PA_shutdown_Pin GPIO_PIN_5
#define Audio_PA_shutdown_GPIO_Port GPIOG
#define Button_G1_Pin GPIO_PIN_6
#define Button_G1_GPIO_Port GPIOG
#define opt_SW_E4_Pin GPIO_PIN_7
#define opt_SW_E4_GPIO_Port GPIOG
#define MicBias_Pin GPIO_PIN_8
#define MicBias_GPIO_Port GPIOG
#define POWER_DOWN_Pin GPIO_PIN_8
#define POWER_DOWN_GPIO_Port GPIOC
#define BAND1_Pin GPIO_PIN_8
#define BAND1_GPIO_Port GPIOA
#define TP_CS_Pin GPIO_PIN_9
#define TP_CS_GPIO_Port GPIOA
#define BAND2_Pin GPIO_PIN_10
#define BAND2_GPIO_Port GPIOA
#define Button_F1_Pin GPIO_PIN_15
#define Button_F1_GPIO_Port GPIOA
#define LCD_BL_CTRL_Pin GPIO_PIN_2
#define LCD_BL_CTRL_GPIO_Port GPIOD
#define LCD_RESET_Pin GPIO_PIN_3
#define LCD_RESET_GPIO_Port GPIOD
#define Button_G4_Pin GPIO_PIN_11
#define Button_G4_GPIO_Port GPIOG
#define Button_F3_Pin GPIO_PIN_15
#define Button_F3_GPIO_Port GPIOG
#define Button_G2_Pin GPIO_PIN_8
#define Button_G2_GPIO_Port GPIOB
#define LED_GREEN_Pin GPIO_PIN_9
#define LED_GREEN_GPIO_Port GPIOB
#define keyer_dash_Pin GPIO_PIN_0
#define keyer_dash_GPIO_Port GPIOE
#define keyer_dot_Pin GPIO_PIN_1
#define keyer_dot_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
