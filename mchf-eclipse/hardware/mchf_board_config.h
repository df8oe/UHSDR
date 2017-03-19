/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/
#ifndef __MCHF_BOARD_CONFIG_H
#define __MCHF_BOARD_CONFIG_H

#include "mchf_types.h"
#include "stm32f4xx.h"

#if 0
#include "stm32f4xx_hal.h"
// HW libs
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
//#include "stm32f4xx_hal_exti.h"
//#include "stm32f4xx_hal_usart.h"
// #include "stm32f4xx_hal_syscfg.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_dac.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_rtc.h"
#include "stm32f4xx_hal_pwr.h"
// #include "stm32f4xx_hal_fsmc.h"
// #include "stm32f4xx_hal_wwdg.h"
#include "stm32f4xx_hal_flash.h"
#include "core_cm4.h"

#include "freedv_api.h"

#include "audio_filter.h"
#include "ui_si570.h"
//
#include "comp.h"
#include "dac.h"
#endif
//
// -----------------------------------------------------------------------------
#define		DEVICE_STRING			"mcHF QRP Transceiver"
#define 	AUTHOR_STRING   		"K. Atanassov - M\x60NKA 2014-2017"
//
#define		ATTRIB_STRING1			"Additional Contributions by"
#define		ATTRIB_STRING2			"KA7OEI, DF8OE and others."
#define		ATTRIB_STRING3			"Licensed under "TRX4M_LICENCE"      "
//
// -----------------------------------------------------------------------------
#define CODEC_I2C					I2C2
#define SERIALEEPROM_I2C			I2C2


#define		WD_REFRESH_WINDOW		80
#define		WD_REFRESH_COUNTER		127

// -----------------------------------------------------------------------------
//						PORT PINS ALLOCATION
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// ---- 						PORT A										----
// -----------------------------------------------------------------------------
// pin 0
#define ENC_THREE_CH1 			GPIO_PIN_0
#define ENC_THREE_CH1_SOURCE	GPIO_
#define ENC_THREE_CH1_PIO       GPIOA
// pin 1
#define ENC_THREE_CH2 			GPIO_PIN_1
#define ENC_THREE_CH2_SOURCE	GPIO_PinSource1
#define ENC_THREE_CH2_PIO       GPIOA
// pin 2
#define ADC3_FWD				GPIO_PIN_2
#define ADC3_FWD_SOURCE			GPIO_PinSource2
#define ADC3_FWD_PIO       		GPIOA
// pin 3
#define ADC2_RET				GPIO_PIN_3
#define ADC2_RET_SOURCE			GPIO_PinSource3
#define ADC2_RET_PIO       		GPIOA
// pin 4
#ifdef MCHF_BOARD_0P5
#define TP_IRQ					GPIO_PIN_14
#define TP_IRQ_SOURCE			GPIO_PinSource14
#else
#define TP_IRQ					GPIO_PIN_4
#define TP_IRQ_SOURCE			GPIO_PinSource4
#endif
#define TP_IRQ_PIO				GPIOA
#define DAC0 					GPIO_PIN_4
#define DAC0_SOURCE				GPIO_PinSource4
#define DAC0_PIO       			GPIOA
// pin 5
//#define DAC1 					GPIO_PIN_5
//#define DAC1_SOURCE				GPIO_PinSource5
//#define DAC1_PIO       			GPIOA
// pin 6
#define ADC1_PWR				GPIO_PIN_6
#define ADC1_PWR_SOURCE			GPIO_PinSource6
#define ADC1_PWR_PIO       		GPIOA
// pin 7
#define BAND0		 			GPIO_PIN_7
#define BAND0_SOURCE			GPIO_PinSource7
#define BAND0_PIO       		GPIOA
// pin 8
#define BAND1		 			GPIO_PIN_8
#define BAND1_SOURCE			GPIO_PinSource8
#define BAND1_PIO       		GPIOA
// pin 9
#define DEBUG_PRINT	 			GPIO_PIN_9
#define DEBUG_PRINT_SOURCE		GPIO_PinSource9
#define DEBUG_PRINT_PIO    		GPIOA
#ifdef MCHF_BOARD_0P5
#define TP_CS					GPIO_PIN_13
#define TP_CS_SOURCE			GPIO_PinSource13
#else
#define TP_CS					GPIO_PIN_9
#define TP_CS_SOURCE			GPIO_PinSource9
#endif
#define TP_CS_PIO				GPIOA
// pin 10
#define BAND2 					GPIO_PIN_10
#define BAND2_SOURCE			GPIO_PinSource10
#define BAND2_PIO       		GPIOA
// pin 11
// USB DFU
//
// pin 12
// USB DFU
//
//
// pin 13
// SWDIO
// pin 14
// SWCLK
//
//
// pin 15
#define CODEC_I2S_WS			GPIO_PIN_15
#define CODEC_I2S_WS_SOURCE		GPIO_PinSource15
#define CODEC_I2S_WS_PIO  		GPIOA
//
// -----------------------------------------------------------------------------
// ---- 						PORT B										----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_BNDM				GPIO_PIN_0
#define BUTTON_BNDM_SOURCE		GPIO_PinSource0
#define BUTTON_BNDM_PIO       	GPIOB
// pin 1
#define PTT_CNTR 				GPIO_PIN_1
#define PTT_CNTR_SOURCE			GPIO_PinSource1
#define PTT_CNTR_PIO       		GPIOB
// pin 2
#define BUTTON_BNDP 			GPIO_PIN_2
#define BUTTON_BNDP_SOURCE		GPIO_PinSource2
#define BUTTON_BNDP_PIO       	GPIOB
// pin 3
#define BUTTON_M2 				GPIO_PIN_3
#define BUTTON_M2_SOURCE		GPIO_PinSource3
#define BUTTON_M2_PIO       	GPIOB
// pin 4
#define ENC_ONE_CH1 			GPIO_PIN_4
#define ENC_ONE_CH1_SOURCE		GPIO_PinSource4
#define ENC_ONE_CH1_PIO       	GPIOB
// pin 5
#define ENC_ONE_CH2 			GPIO_PIN_5
#define ENC_ONE_CH2_SOURCE		GPIO_PinSource5
#define ENC_ONE_CH2_PIO       	GPIOB
// pin 6
#define I2C1_SCL_PIN            GPIO_PIN_6
#define I2C1_SCL_PINSRC         GPIO_PinSource6
#define I2C1_SCL_GPIO           GPIOB
// pin 7
#define I2C1_SDA_PIN            GPIO_PIN_7
#define I2C1_SDA_PINSRC         GPIO_PinSource7
#define I2C1_SDA_GPIO           GPIOB
// pin 8
#define BUTTON_G3 				GPIO_PIN_8
#define BUTTON_G3_SOURCE		GPIO_PinSource8
#define BUTTON_G3_PIO       	GPIOB
// pin 9
#define GREEN_LED 				GPIO_PIN_9
#define GREEN_LED_SOURCE		GPIO_PinSource9
#define GREEN_LED_PIO       	GPIOB
// pin 10
#define I2C2_SCL_PIN            GPIO_PIN_10
#define I2C2_SCL_PINSRC         GPIO_PinSource10
#define I2C2_SCL_GPIO           GPIOB
// pin 11
#define I2C2_SDA_PIN            GPIO_PIN_11
#define I2C2_SDA_PINSRC         GPIO_PinSource11
#define I2C2_SDA_GPIO           GPIOB
// pin 12
#define RED_LED 				GPIO_PIN_12
#define RED_LED_SOURCE			GPIO_PinSource12
#define RED_LED_PIO       		GPIOB
// pin 13
#define LCD_SCK 				GPIO_PIN_13
#define LCD_SCK_SOURCE			GPIO_PinSource13
#define LCD_SCK_PIO         	GPIOB
// pin 14
// USB HOST
//
// pin 15
// USB HOST
//
//
// -----------------------------------------------------------------------------
// ---- 						PORT C										----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_G4 				GPIO_PIN_0
#define BUTTON_G4_SOURCE		GPIO_PinSource0
#define BUTTON_G4_PIO       	GPIOC
// pin 1
#define BUTTON_M3 				GPIO_PIN_1
#define BUTTON_M3_SOURCE		GPIO_PinSource1
#define BUTTON_M3_PIO       	GPIOC
// pin 2
#define LCD_MISO 				GPIO_PIN_2
#define LCD_MISO_SOURCE			GPIO_PinSource2
#define LCD_MISO_PIO         	GPIOC
// pin 3
#define LCD_MOSI 				GPIO_PIN_3
#define LCD_MOSI_SOURCE			GPIO_PinSource3
#define LCD_MOSI_PIO         	GPIOC
// pin 4
#define BUTTON_STEPM			GPIO_PIN_4
#define BUTTON_STEPM_SOURCE		GPIO_PinSource4
#define BUTTON_STEPM_PIO       	GPIOC
// pin 5
#define BUTTON_STEPP			GPIO_PIN_5
#define BUTTON_STEPP_SOURCE		GPIO_PinSource5
#define BUTTON_STEPP_PIO       	GPIOC
// pin 6
#define FREQ_ENC_CH1 			GPIO_PIN_6
#define FREQ_ENC_CH1_SOURCE		GPIO_PinSource6
#define FREQ_ENC_CH1_PIO        GPIOC
// pin 7
#define FREQ_ENC_CH2 			GPIO_PIN_7
#define FREQ_ENC_CH2_SOURCE		GPIO_PinSource7
#define FREQ_ENC_CH2_PIO        GPIOC
// pin 8
#define POWER_DOWN 				GPIO_PIN_8
#define POWER_DOWN_SOURCE		GPIO_PinSource8
#define POWER_DOWN_PIO         	GPIOC
// pin 9
#define CODEC_CLOCK 			GPIO_PIN_9
#define CODEC_CLOCK_SOURCE		GPIO_PinSource9
#define CODEC_CLOCK_PIO         GPIOC
// pin 10
#define CODEC_I2S_SCK 			GPIO_PIN_10
#define CODEC_I2S_SCK_SOURCE	GPIO_PinSource10
#define CODEC_I2S_SCK_PIO       GPIOC
// pin 11
#define CODEC_I2S_SDI 			GPIO_PIN_11
#define CODEC_I2S_SDI_SOURCE	GPIO_PinSource11
#define CODEC_I2S_SDI_PIO       GPIOC
// pin 12
#define CODEC_I2S_SDO 			GPIO_PIN_12
#define CODEC_I2S_SDO_SOURCE	GPIO_PinSource12
#define CODEC_I2S_SDO_PIO       GPIOC
// pin 13
#define BUTTON_PWR				GPIO_PIN_13
#define BUTTON_PWR_SOURCE		GPIO_PinSource13
#define BUTTON_PWR_PIO       	GPIOC
// pin 14
#define BUTTON_M1				GPIO_PIN_14
#define BUTTON_M1_SOURCE		GPIO_PinSource14
#define BUTTON_M1_PIO       	GPIOC
// pin 15
#define BUTTON_F3				GPIO_PIN_15
#define BUTTON_F3_SOURCE		GPIO_PinSource15
#define BUTTON_F3_PIO       	GPIOC

#define BUTTON_M1_RTC               GPIO_PIN_14
#define BUTTON_M1_SOURCE        GPIO_PinSource14
#define BUTTON_M1_PIO_RTC           GPIOD
// pin 15
#define BUTTON_F3_RTC               GPIO_PIN_15
#define BUTTON_F3_SOURCE_RTC        GPIO_PinSource15
#define BUTTON_F3_PIO_RTC           GPIOD

//
// -----------------------------------------------------------------------------
// ---- 						PORT D										----
// -----------------------------------------------------------------------------
// pin 0
#define LCD_D2					GPIO_PIN_0
#define LCD_D2_SOURCE			GPIO_PinSource0
#define LCD_D2_PIO      		GPIOD
// pin 1
#define LCD_D3					GPIO_PIN_1
#define LCD_D3_SOURCE			GPIO_PinSource1
#define LCD_D3_PIO      		GPIOD
// pin 2
#define LCD_BACKLIGHT			GPIO_PIN_2
#define LCD_BACKLIGHT_SOURCE	GPIO_PinSource2
#define LCD_BACKLIGHT_PIO      	GPIOD
// pin 3
#define LCD_RESET				GPIO_PIN_3
#define LCD_RESET_SOURCE		GPIO_PinSource3
#define LCD_RESET_PIO      		GPIOD
// pin 4
#define LCD_RD					GPIO_PIN_4
#define LCD_RD_SOURCE			GPIO_PinSource4
#define LCD_RD_PIO      		GPIOD
// pin 5
#define LCD_WR					GPIO_PIN_5
#define LCD_WR_SOURCE			GPIO_PinSource5
#define LCD_WR_PIO      		GPIOD
// pin 6
#define BUTTON_F1				GPIO_PIN_6
#define BUTTON_F1_SOURCE		GPIO_PinSource6
#define BUTTON_F1_PIO       	GPIOD
// pin 7
#define LCD_CSA					GPIO_PIN_7
#define LCD_CSA_SOURCE			GPIO_PinSource7
#define LCD_CSA_PIO      		GPIOD
// pin 8
#define LCD_D15					GPIO_PIN_8
#define LCD_D15_SOURCE			GPIO_PinSource8
#define LCD_D15_PIO      		GPIOD
// pin 9
#define LCD_D16					GPIO_PIN_9
#define LCD_D16_SOURCE			GPIO_PinSource9
#define LCD_D16_PIO      		GPIOD
// pin 10
#define LCD_D17					GPIO_PIN_10
#define LCD_D17_SOURCE			GPIO_PinSource10
#define LCD_D17_PIO      		GPIOD
// pin 11
#define LCD_RS					GPIO_PIN_11
#define LCD_RS_SOURCE			GPIO_PinSource11
#define LCD_RS_PIO      		GPIOD
// pin 12
#define ENC_TWO_CH1 			GPIO_PIN_12
#define ENC_TWO_CH1_SOURCE		GPIO_PinSource12
#define ENC_TWO_CH1_PIO         GPIOD
// pin 13
#define ENC_TWO_CH2 			GPIO_PIN_13
#define ENC_TWO_CH2_SOURCE		GPIO_PinSource13
#define ENC_TWO_CH2_PIO         GPIOD
// pin 14
#define LCD_D0					GPIO_PIN_14
#define LCD_D0_SOURCE			GPIO_PinSource14
#define LCD_D0_PIO      		GPIOD
// pin 15
#define LCD_D1					GPIO_PIN_15
#define LCD_D1_SOURCE			GPIO_PinSource15
#define LCD_D1_PIO      		GPIOD
//
// -----------------------------------------------------------------------------
// ---- 						PORT E										----
// -----------------------------------------------------------------------------
// pin 0
#define PADDLE_DAH				GPIO_PIN_0
#define PADDLE_DAH_SOURCE		GPIO_PinSource0
#define PADDLE_DAH_PIO       	GPIOE
// pin 1
#define PADDLE_DIT				GPIO_PIN_1
#define PADDLE_DIT_SOURCE		GPIO_PinSource1
#define PADDLE_DIT_PIO       	GPIOE
// pin 2
#define BUTTON_F2				GPIO_PIN_2
#define BUTTON_F2_SOURCE		GPIO_PinSource2
#define BUTTON_F2_PIO       	GPIOE
// pin 3
#define BUTTON_F4				GPIO_PIN_3
#define BUTTON_F4_SOURCE		GPIO_PinSource3
#define BUTTON_F4_PIO       	GPIOE
// pin 4
#define BUTTON_G2				GPIO_PIN_4
#define BUTTON_G2_SOURCE		GPIO_PinSource4
#define BUTTON_G2_PIO       	GPIOE
// pin 5
#define BUTTON_F5				GPIO_PIN_5
#define BUTTON_F5_SOURCE		GPIO_PinSource5
#define BUTTON_F5_PIO       	GPIOE
// pin 6
#define BUTTON_G1				GPIO_PIN_6
#define BUTTON_G1_SOURCE		GPIO_PinSource6
#define BUTTON_G1_PIO       	GPIOE
// pin 7
#define LCD_D4					GPIO_PIN_7
#define LCD_D4_SOURCE			GPIO_PinSource7
#define LCD_D4_PIO      		GPIOE
// pin 8
#define LCD_D5					GPIO_PIN_8
#define LCD_D5_SOURCE			GPIO_PinSource8
#define LCD_D5_PIO      		GPIOE
// pin 9
#define LCD_D6					GPIO_PIN_9
#define LCD_D6_SOURCE			GPIO_PinSource9
#define LCD_D6_PIO      		GPIOE
// pin 10
#define LCD_D7					GPIO_PIN_10
#define LCD_D7_SOURCE			GPIO_PinSource10
#define LCD_D7_PIO      		GPIOE
// pin 11
#define LCD_D10					GPIO_PIN_11
#define LCD_D10_SOURCE			GPIO_PinSource11
#define LCD_D10_PIO      		GPIOE
// pin 12
#define LCD_D11					GPIO_PIN_12
#define LCD_D11_SOURCE			GPIO_PinSource12
#define LCD_D11_PIO        		GPIOE
// pin 13
#define LCD_D12					GPIO_PIN_13
#define LCD_D12_SOURCE			GPIO_PinSource13
#define LCD_D12_PIO      		GPIOE
// pin 14
#define LCD_D13					GPIO_PIN_14
#define LCD_D13_SOURCE			GPIO_PinSource14
#define LCD_D13_PIO      		GPIOE
// pin 15
#define LCD_D14					GPIO_PIN_15
#define LCD_D14_SOURCE			GPIO_PinSource15
#define LCD_D14_PIO      		GPIOE

#define GPIO_SetBits(PORT,PINS) { (PORT)->BSRR = (PINS); }
#define GPIO_ResetBits(PORT,PINS) { (PORT)->BSRR = (PINS) << 16U; }
#define GPIO_ReadInputDataBit(PORT,PINS) ((PORT)->BSRR = (PINS) << 16U)

#endif
