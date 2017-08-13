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

#include "uhsdr_types.h"
#include "uhsdr_mcu.h"



#ifdef STM32F4

#ifndef TRX_NAME
  #define TRX_NAME "mcHF QRP"
#endif
#ifndef TRX_ID
  #define TRX_ID "mchf"
#endif

#ifndef TRX_HW_LIC
#define TRX_HW_LIC "CC BY-NC-SA 3.0"
#define TRX_HW_CREATOR "K. Atanassov, M0NKA, www.m0nka.co.uk"
#endif

#define UI_BRD_MCHF
#define RF_BRD_MCHF


// place tagged elements in CCM 64k extra RAM (no DMA)
#define __MCHF_SPECIALMEM __attribute__ ((section (".ccm")))

#define SI570_I2C               (&hi2c1)

#define CODEC_I2C               (&hi2c2)
#define CODEC_ANA_I2C               (&hi2c2)
#define CODEC_IQ_I2C                (&hi2c2)

#define SERIALEEPROM_I2C            (&hi2c2)

// -----------------------------------------------------------------------------
//						PORT PINS ALLOCATION
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// ---- 						PORT A										----
// -----------------------------------------------------------------------------
// pin 0
#define ENC_THREE_CH1 			GPIO_PIN_0
#define ENC_THREE_CH1_PIO       GPIOA
// pin 1
#define ENC_THREE_CH2 			GPIO_PIN_1
#define ENC_THREE_CH2_PIO       GPIOA
// pin 2
#define ADC3_FWD				GPIO_PIN_2
#define ADC3_FWD_PIO       		GPIOA
// pin 3
#define ADC2_RET				GPIO_PIN_3
#define ADC2_RET_PIO       		GPIOA
// pin 4
#ifdef MCHF_BOARD_0P5
#define TP_IRQ					GPIO_PIN_14
#else
#define TP_IRQ					GPIO_PIN_4
#endif
#define TP_IRQ_PIO				GPIOA

#define DAC0 					GPIO_PIN_4
#define DAC0_PIO       			GPIOA
// pin 5
//#define DAC1 					GPIO_PIN_5
//#define DAC1_SOURCE				GPIO_PinSource5
//#define DAC1_PIO       			GPIOA
// pin 6
#define ADC1_PWR				GPIO_PIN_6
#define ADC1_PWR_PIO       		GPIOA
// pin 7
#define BAND0		 			GPIO_PIN_7
#define BAND0_PIO       		GPIOA
// pin 8
#define BAND1		 			GPIO_PIN_8
#define BAND1_PIO       		GPIOA
// pin 9
#define DEBUG_PRINT	 			GPIO_PIN_9
#define DEBUG_PRINT_PIO    		GPIOA

#ifdef MCHF_BOARD_0P5
#define TP_CS					GPIO_PIN_13
#else
#define TP_CS					GPIO_PIN_9
#endif
#define TP_CS_PIO				GPIOA
// pin 10
#define BAND2 					GPIO_PIN_10
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
#define CODEC_I2S_WS_PIO  		GPIOA
//
// -----------------------------------------------------------------------------
// ---- 						PORT B										----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_BNDM				GPIO_PIN_0
#define BUTTON_BNDM_PIO       	GPIOB
// pin 1
#define PTT_CNTR 				GPIO_PIN_1
#define PTT_CNTR_PIO       		GPIOB
// pin 2
#define BUTTON_BNDP 			GPIO_PIN_2
#define BUTTON_BNDP_PIO       	GPIOB
// pin 3
#define BUTTON_M2 				GPIO_PIN_3
#define BUTTON_M2_PIO       	GPIOB
// pin 4
#define ENC_ONE_CH1 			GPIO_PIN_4
#define ENC_ONE_CH1_PIO       	GPIOB
// pin 5
#define ENC_ONE_CH2 			GPIO_PIN_5
#define ENC_ONE_CH2_PIO       	GPIOB
// pin 6
#define I2C1_SCL_PIN            GPIO_PIN_6
#define I2C1_SCL_GPIO           GPIOB
// pin 7
#define I2C1_SDA_PIN            GPIO_PIN_7
#define I2C1_SDA_GPIO           GPIOB
// pin 8
#define BUTTON_G2 				GPIO_PIN_8
#define BUTTON_G2_PIO       	GPIOB
// pin 9
#define GREEN_LED 				GPIO_PIN_9
#define GREEN_LED_PIO       	GPIOB
// pin 10
#define I2C2_SCL_PIN            GPIO_PIN_10
#define I2C2_SCL_GPIO           GPIOB
// pin 11
#define I2C2_SDA_PIN            GPIO_PIN_11
#define I2C2_SDA_GPIO           GPIOB
// pin 12
#define RED_LED 				GPIO_PIN_12
#define RED_LED_PIO       		GPIOB
// pin 13
#define LCD_SCK 				GPIO_PIN_13
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
#define BUTTON_G4_PIO       	GPIOC
// pin 1
#define BUTTON_M3 				GPIO_PIN_1
#define BUTTON_M3_PIO       	GPIOC
// pin 2
#define LCD_MISO 				GPIO_PIN_2
#define LCD_MISO_PIO         	GPIOC
// pin 3
#define LCD_MOSI 				GPIO_PIN_3
#define LCD_MOSI_PIO         	GPIOC
// pin 4
#define BUTTON_STEPM			GPIO_PIN_4
#define BUTTON_STEPM_PIO       	GPIOC
// pin 5
#define BUTTON_STEPP			GPIO_PIN_5
#define BUTTON_STEPP_PIO       	GPIOC
// pin 6
#define FREQ_ENC_CH1 			GPIO_PIN_6
#define FREQ_ENC_CH1_PIO        GPIOC
// pin 7
#define FREQ_ENC_CH2 			GPIO_PIN_7
#define FREQ_ENC_CH2_PIO        GPIOC
// pin 8
#define POWER_DOWN 				GPIO_PIN_8
#define POWER_DOWN_PIO         	GPIOC
// pin 9
#define CODEC_CLOCK 			GPIO_PIN_9
#define CODEC_CLOCK_PIO         GPIOC
// pin 10
#define CODEC_I2S_SCK 			GPIO_PIN_10
#define CODEC_I2S_SCK_PIO       GPIOC
// pin 11
#define CODEC_I2S_SDI 			GPIO_PIN_11
#define CODEC_I2S_SDI_PIO       GPIOC
// pin 12
#define CODEC_I2S_SDO 			GPIO_PIN_12
#define CODEC_I2S_SDO_PIO       GPIOC
// pin 13
#define BUTTON_PWR				GPIO_PIN_13
#define BUTTON_PWR_PIO       	GPIOC
// pin 14
#define BUTTON_M1				GPIO_PIN_14
#define BUTTON_M1_PIO       	GPIOC
// pin 15
#define BUTTON_F3				GPIO_PIN_15
#define BUTTON_F3_PIO       	GPIOC

#define BUTTON_M1_RTC           GPIO_PIN_14
#define BUTTON_M1_PIO_RTC       GPIOD
// pin 15
#define BUTTON_F3_RTC           GPIO_PIN_15
#define BUTTON_F3_PIO_RTC       GPIOD

//
// -----------------------------------------------------------------------------
// ---- 						PORT D										----
// -----------------------------------------------------------------------------
// pin 0
#define LCD_D2					GPIO_PIN_0
#define LCD_D2_PIO      		GPIOD
// pin 1
#define LCD_D3					GPIO_PIN_1
#define LCD_D3_PIO      		GPIOD
// pin 2
#define LCD_BACKLIGHT			GPIO_PIN_2
#define LCD_BACKLIGHT_PIO      	GPIOD
// pin 3
#define LCD_RESET				GPIO_PIN_3
#define LCD_RESET_PIO      		GPIOD
// pin 4
#define LCD_RD					GPIO_PIN_4
#define LCD_RD_PIO      		GPIOD
// pin 5
#define LCD_WR					GPIO_PIN_5
#define LCD_WR_PIO      		GPIOD
// pin 6
#define BUTTON_F1				GPIO_PIN_6
#define BUTTON_F1_PIO       	GPIOD
// pin 7
#define LCD_CSA					GPIO_PIN_7
#define LCD_CSA_PIO      		GPIOD
// pin 8
#define LCD_D15					GPIO_PIN_8
#define LCD_D15_PIO      		GPIOD
// pin 9
#define LCD_D16					GPIO_PIN_9
#define LCD_D16_PIO      		GPIOD
// pin 10
#define LCD_D17					GPIO_PIN_10
#define LCD_D17_PIO      		GPIOD
// pin 11
#define LCD_RS					GPIO_PIN_11
#define LCD_RS_PIO      		GPIOD
// pin 12
#define ENC_TWO_CH1 			GPIO_PIN_12
#define ENC_TWO_CH1_PIO         GPIOD
// pin 13
#define ENC_TWO_CH2 			GPIO_PIN_13
#define ENC_TWO_CH2_PIO         GPIOD
// pin 14
#define LCD_D0					GPIO_PIN_14
#define LCD_D0_PIO      		GPIOD
// pin 15
#define LCD_D1					GPIO_PIN_15
#define LCD_D1_PIO      		GPIOD
//
// -----------------------------------------------------------------------------
// ---- 						PORT E										----
// -----------------------------------------------------------------------------
// pin 0
#define PADDLE_DAH				GPIO_PIN_0
#define PADDLE_DAH_PIO       	GPIOE
// pin 1
#define PADDLE_DIT				GPIO_PIN_1
#define PADDLE_DIT_PIO       	GPIOE
// pin 2
#define BUTTON_F2				GPIO_PIN_2
#define BUTTON_F2_PIO       	GPIOE
// pin 3
#define BUTTON_F4				GPIO_PIN_3
#define BUTTON_F4_PIO       	GPIOE
// pin 4
#define BUTTON_G3				GPIO_PIN_4
#define BUTTON_G3_PIO       	GPIOE
// pin 5
#define BUTTON_F5				GPIO_PIN_5
#define BUTTON_F5_PIO       	GPIOE
// pin 6
#define BUTTON_G1				GPIO_PIN_6
#define BUTTON_G1_PIO       	GPIOE
// pin 7
#define LCD_D4					GPIO_PIN_7
#define LCD_D4_PIO      		GPIOE
// pin 8
#define LCD_D5					GPIO_PIN_8
#define LCD_D5_PIO      		GPIOE
// pin 9
#define LCD_D6					GPIO_PIN_9
#define LCD_D6_PIO      		GPIOE
// pin 10
#define LCD_D7					GPIO_PIN_10
#define LCD_D7_PIO      		GPIOE
// pin 11
#define LCD_D10					GPIO_PIN_11
#define LCD_D10_PIO      		GPIOE
// pin 12
#define LCD_D11					GPIO_PIN_12
#define LCD_D11_PIO        		GPIOE
// pin 13
#define LCD_D12					GPIO_PIN_13
#define LCD_D12_PIO      		GPIOE
// pin 14
#define LCD_D13					GPIO_PIN_14
#define LCD_D13_PIO      		GPIOE
// pin 15
#define LCD_D14					GPIO_PIN_15
#define LCD_D14_PIO      		GPIOE
#endif

#ifdef STM32F7

#ifndef TRX_NAME
#define TRX_NAME "OVI40"
#endif
#ifndef TRX_ID
#define TRX_ID "ovi40"
#endif

#ifndef TRX_HW_LIC
// #define TRX_HW_LIC "???"
// #define TRX_HW_CREATOR "???"
#endif


#define UI_BRD_OVI40
#define RF_BRD_MCHF

// compiler places tagged elements by its default rules
#define __MCHF_SPECIALMEM


#define SI570_I2C               (&hi2c1)


#define CODEC_ANA_I2C               (&hi2c2)
#define CODEC_ANA_SAI               SAI1

#define CODEC_IQ_I2C                (&hi2c4)
#define CODEC_IQ_SAI                SAI2

#define SERIALEEPROM_I2C            (&hi2c2)


// -----------------------------------------------------------------------------
//                      PORT PINS ALLOCATION
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// ----                         PORT A                                      ----
// -----------------------------------------------------------------------------
// pin 0
#define ENC_THREE_CH1           GPIO_PIN_0
#define ENC_THREE_CH1_PIO       GPIOA
// pin 1
#define ENC_THREE_CH2           GPIO_PIN_1
#define ENC_THREE_CH2_PIO       GPIOA
// pin 2
#define XADC3_FWD                GPIO_PIN_2
#define XXADC3_FWD_PIO            GPIOA
// pin 3
#define ADC2_RET                GPIO_PIN_3
#define ADC2_RET_PIO            GPIOA
// pin 4
#define DAC_CH1                 GPIO_PIN_4
#define DAC_CH1_PIO             GPIOA
// pin 5
#define DAC_CH2                 GPIO_PIN_5
#define DAC_CH2_PIO             GPIOA
// pin 6
#define ADC1_PWR                GPIO_PIN_6
#define ADC1_PWR_PIO            GPIOA
// pin 7
// pin 8
#define BAND1                   GPIO_PIN_8
#define BAND1_PIO               GPIOA
// pin 9
#define TP_CS                   GPIO_PIN_9
#define TP_CS_PIO               GPIOA
// pin 10
#define BAND2                   GPIO_PIN_10
#define BAND2_PIO               GPIOA
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
#define BUTTON_F1               GPIO_PIN_15
#define BUTTON_F1_PIO           GPIOA
//
// -----------------------------------------------------------------------------
// ----                         PORT B                                      ----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_BNDM             GPIO_PIN_0
#define BUTTON_BNDM_PIO         GPIOB
// pin 1
#define PTT_CNTR                GPIO_PIN_1
#define PTT_CNTR_PIO            GPIOB
// pin 2
#define BUTTON_BNDP             GPIO_PIN_2
#define BUTTON_BNDP_PIO         GPIOB
// pin 3
#define BUTTON_M2               GPIO_PIN_3
#define BUTTON_M2_PIO           GPIOB
// pin 4
#define ENC_ONE_CH1             GPIO_PIN_4
#define ENC_ONE_CH1_PIO         GPIOB
// pin 5
#define ENC_ONE_CH2             GPIO_PIN_5
#define ENC_ONE_CH2_PIO         GPIOB
// pin 6
#define I2C1_SCL_PIN            GPIO_PIN_6
#define I2C1_SCL_GPIO           GPIOB
// pin 7
#define I2C1_SDA_PIN            GPIO_PIN_7
#define I2C1_SDA_GPIO           GPIOB
// pin 8
#define BUTTON_G2               GPIO_PIN_8
#define BUTTON_G2_PIO           GPIOB
// pin 9
#define GREEN_LED               GPIO_PIN_9
#define GREEN_LED_PIO           GPIOB
// pin 10
#define I2C2_SCL_PIN            GPIO_PIN_10
#define I2C2_SCL_GPIO           GPIOB
// pin 11
#define I2C2_SDA_PIN            GPIO_PIN_11
#define I2C2_SDA_GPIO           GPIOB
// pin 12
#define RED_LED                 GPIO_PIN_12
#define RED_LED_PIO             GPIOB
// pin 13
#define LCD_SCK                 GPIO_PIN_13
#define LCD_SCK_PIO             GPIOB
// pin 14
// USB HOST
//
// pin 15
// USB HOST
//
//
// -----------------------------------------------------------------------------
// ----                         PORT C                                      ----
// -----------------------------------------------------------------------------
// pin 0
#define XBUTTON_G4               GPIO_PIN_0
#define XBUTTON_G4_PIO           GPIOC
// pin 1
#define BUTTON_M3               GPIO_PIN_1
#define BUTTON_M3_PIO           GPIOC
// pin 2
#define LCD_MISO                GPIO_PIN_2
#define LCD_MISO_PIO            GPIOC
// pin 3
#define LCD_MOSI                GPIO_PIN_3
#define LCD_MOSI_PIO            GPIOC
// pin 4
#define BUTTON_STEPM            GPIO_PIN_4
#define BUTTON_STEPM_PIO        GPIOC
// pin 5
#define BUTTON_STEPP            GPIO_PIN_5
#define BUTTON_STEPP_PIO        GPIOC
// pin 6
#define FREQ_ENC_CH1            GPIO_PIN_6
#define FREQ_ENC_CH1_PIO        GPIOC
// pin 7
#define FREQ_ENC_CH2            GPIO_PIN_7
#define FREQ_ENC_CH2_PIO        GPIOC
// pin 8
#define POWER_DOWN              GPIO_PIN_8
#define POWER_DOWN_PIO          GPIOC
// pin 9
#define CODEC_CLOCK             GPIO_PIN_9
#define CODEC_CLOCK_PIO         GPIOC
// pin 10
#define CODEC_I2S_SCK           GPIO_PIN_10
#define CODEC_I2S_SCK_PIO       GPIOC
// pin 11
#define CODEC_I2S_SDI           GPIO_PIN_11
#define CODEC_I2S_SDI_PIO       GPIOC
// pin 12
#define CODEC_I2S_SDO           GPIO_PIN_12
#define CODEC_I2S_SDO_PIO       GPIOC
// pin 13
#define XBUTTON_PWR              GPIO_PIN_13
#define XBUTTON_PWR_PIO          GPIOC
// pin 14
#define XBUTTON_M1               GPIO_PIN_14
#define XBUTTON_M1_PIO           GPIOC
// pin 15
#define XBUTTON_F3               GPIO_PIN_15
#define XBUTTON_F3_PIO           GPIOC

// pin 15

//
// -----------------------------------------------------------------------------
// ----                         PORT D                                      ----
// -----------------------------------------------------------------------------
// pin 0
#define LCD_D2                  GPIO_PIN_0
#define LCD_D2_PIO              GPIOD
// pin 1
#define LCD_D3                  GPIO_PIN_1
#define LCD_D3_PIO              GPIOD
// pin 2
#define LCD_BACKLIGHT           GPIO_PIN_2
#define LCD_BACKLIGHT_PIO       GPIOD
// pin 3
#define LCD_RESET               GPIO_PIN_3
#define LCD_RESET_PIO           GPIOD
// pin 4
#define LCD_RD                  GPIO_PIN_4
#define LCD_RD_PIO              GPIOD
// pin 5
#define LCD_WR                  GPIO_PIN_5
#define LCD_WR_PIO              GPIOD
// pin 6
#define XBUTTON_F1               GPIO_PIN_6
#define XBUTTON_F1_PIO           GPIOD
// pin 7
#define LCD_CSA                 GPIO_PIN_7
#define LCD_CSA_PIO             GPIOD
// pin 8
#define LCD_D15                 GPIO_PIN_8
#define LCD_D15_PIO             GPIOD
// pin 9
#define LCD_D16                 GPIO_PIN_9
#define LCD_D16_PIO             GPIOD
// pin 10
#define LCD_D17                 GPIO_PIN_10
#define LCD_D17_PIO             GPIOD
// pin 11
#define LCD_RS                  GPIO_PIN_11
#define LCD_RS_PIO              GPIOD
// pin 12
#define ENC_TWO_CH1             GPIO_PIN_12
#define ENC_TWO_CH1_PIO         GPIOD
// pin 13
#define ENC_TWO_CH2             GPIO_PIN_13
#define ENC_TWO_CH2_PIO         GPIOD
// pin 14
#define LCD_D0                  GPIO_PIN_14
#define LCD_D0_PIO              GPIOD
// pin 15
#define LCD_D1                  GPIO_PIN_15
#define LCD_D1_PIO              GPIOD
//
// -----------------------------------------------------------------------------
// ----                         PORT E                                      ----
// -----------------------------------------------------------------------------
// pin 0
#define PADDLE_DAH              GPIO_PIN_0
#define PADDLE_DAH_PIO          GPIOE
// pin 1
#define PADDLE_DIT              GPIO_PIN_1
#define PADDLE_DIT_PIO          GPIOE
// pin 2
#define BUTTON_F2               GPIO_PIN_2
#define BUTTON_F2_PIO           GPIOE
// pin 3
#define BUTTON_F4               GPIO_PIN_3
#define BUTTON_F4_PIO           GPIOE
// pin 4
#define BUTTON_G3               GPIO_PIN_4
#define BUTTON_G3_PIO           GPIOE
// pin 5
#define BUTTON_F5               GPIO_PIN_5
#define BUTTON_F5_PIO           GPIOE
// pin 6
#define XBUTTON_G1               GPIO_PIN_6
#define XBUTTON_G1_PIO           GPIOE
// pin 7
#define LCD_D4                  GPIO_PIN_7
#define LCD_D4_PIO              GPIOE
// pin 8
#define LCD_D5                  GPIO_PIN_8
#define LCD_D5_PIO              GPIOE
// pin 9
#define LCD_D6                  GPIO_PIN_9
#define LCD_D6_PIO              GPIOE
// pin 10
#define LCD_D7                  GPIO_PIN_10
#define LCD_D7_PIO              GPIOE
// pin 11
#define LCD_D10                 GPIO_PIN_11
#define LCD_D10_PIO             GPIOE
// pin 12
#define LCD_D11                 GPIO_PIN_12
#define LCD_D11_PIO             GPIOE
// pin 13
#define LCD_D12                 GPIO_PIN_13
#define LCD_D12_PIO             GPIOE
// pin 14
#define LCD_D13                 GPIO_PIN_14
#define LCD_D13_PIO             GPIOE
// pin 15
#define LCD_D14                 GPIO_PIN_15
#define LCD_D14_PIO             GPIOE

// -----------------------------------------------------------------------------
// ----                         PORT F                                      ----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_M1               GPIO_PIN_0
#define BUTTON_M1_PIO           GPIOF
#define BUTTON_M1_RTC           GPIO_PIN_0
#define BUTTON_M1_PIO_RTC       GPIOF

// pin 1
// pin 2
#define ADC3_FWD                GPIO_PIN_3
#define ADC3_FWD_PIO            GPIOF
// pin 3
// pin 4
// pin 5
#define BUTTON_E2               GPIO_PIN_5
#define BUTTON_E2_PIO           GPIOF
// pin 6
// pin 7
// pin 8
// pin 9
// pin 10
#define BUTTON_E3               GPIO_PIN_10
#define BUTTON_E3_PIO           GPIOF
// pin 11
#define BUTTON_S18              GPIO_PIN_11
#define BUTTON_S18_PIO          GPIOF
// pin 12
#define BLUE_LED                GPIO_PIN_12
#define BLUE_LED_PIO            GPIOF
// pin 13
#define BUTTON_E1               GPIO_PIN_13
#define BUTTON_E1_PIO           GPIOF
// pin 14
// pin 15
#define BUTTON_F3               GPIO_PIN_15
#define BUTTON_F3_PIO           GPIOG

#define BUTTON_F3_RTC           GPIO_PIN_15
#define BUTTON_F3_PIO_RTC       GPIOG

// -----------------------------------------------------------------------------
// ----                         PORT G                                      ----
// -----------------------------------------------------------------------------
// pin 0
#define BAND0                   GPIO_PIN_0
#define BAND0_PIO               GPIOG
// pin 1
#define BAND3                   GPIO_PIN_1
#define BAND3_PIO               GPIOG
// pin 2
#define BUTTON_PWR              GPIO_PIN_2
#define BUTTON_PWR_PIO          GPIOG
// pin 3
// pin 4
#define TP_IRQ                  GPIO_PIN_4
#define TP_IRQ_PIO              GPIOG
// pin 5
#define AUDIO_PA_EN             GPIO_PIN_5
#define AUDIO_PA_EN_PIO         GPIOG
// pin 6
#define BUTTON_G1               GPIO_PIN_6
#define BUTTON_G1_PIO           GPIOG
// pin 7
#define BUTTON_E4               GPIO_PIN_7
#define BUTTON_E4_PIO           GPIOG
// pin 8
#define AUDIO_MIC_BIAS          GPIO_PIN_8
#define AUDIO_MIC_BIAS_PIO      GPIOG
// pin 9
// pin 10
// pin 11
#define BUTTON_G4               GPIO_PIN_11
#define BUTTON_G4_PIO           GPIOG
// pin 12
// pin 13
// pin 14
// pin 15

#endif

//
// -----------------------------------------------------------------------------
#define     DEVICE_STRING           TRX_NAME " Transceiver"
//
// -----------------------------------------------------------------------------




#define GPIO_SetBits(PORT,PINS) { (PORT)->BSRR = (PINS); }
#define GPIO_ResetBits(PORT,PINS) { (PORT)->BSRR = (PINS) << 16U; }
#define GPIO_ToggleBits(PORT,PINS) { (PORT)->ODR ^= (PINS); }
#define GPIO_ReadInputDataBit(PORT,PINS) { ((PORT)->IDR = (PINS); }

// WE DO SET SOME CHOICES BASED ON THE UI BOARD
#ifdef UI_BRD_OVI40
#define USE_TWO_CHANNEL_AUDIO
#endif


/* CONFIGURATION LOGIC CHECKS */

#if defined(UI_BRD_MCHF) && defined(UI_BRD_OVI40)
#error Only one ui board can be selected: UI_BRD_MCHF, UI_BRD_OVI40
#endif
#if !defined(UI_BRD_MCHF) && !defined(UI_BRD_OVI40)
#error One ui board has to be selected: UI_BRD_MCHF, UI_BRD_OVI40
#endif

#if defined(RF_BRD_MCHF) && defined(RF_BRD_OVI40)
#error Only one rf board can be selected: RF_BRD_MCHF, RF_BRD_OVI40
#endif
#if !defined(RF_BRD_MCHF) && !defined(RF_BRD_OVI40)
#error One rf board has to be selected: RF_BRD_MCHF, RF_BRD_OVI40
#endif

#if defined(UI_BRD_MCHF) && defined(USE_TWO_CHANNEL_AUDIO)
#error UI_BRD_MCHF does not permit USE_TWO_CHANNEL_AUDIO
#endif

#endif
