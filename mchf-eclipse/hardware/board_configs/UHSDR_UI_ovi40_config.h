/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/
#ifndef __UHSDR_UI_OVI40_CONFIG_H
#define __UHSDR_UI_OVI40_CONFIG_H


#if defined(CORTEX_M7)
    #if defined(STM32H743xx)
        #include "stm32h7xx.h"
        #include "core_cm7.h"
    #elif defined(STM32F767xx)
        #include "stm32f7xx.h"
        #include "core_cm7.h"
    #else
        #error Not supported CORTEX_M7 MCU specified.
    #endif
#else
    #error OVI40 boards support only CORTEX_M7 MCU!
#endif //CORTEX_M7

// NOT USED on OVI40 F7/H7 boards (defined as no-op)
// compiler places tagged elements by its default rules
#define __MCHF_SPECIALMEM

// place tagged elements in a memory to peripheral DMA-able memory region
// with the correct cache strategy set
#if defined(STM32H7)
    #define __UHSDR_DMAMEM __attribute__ ((section (".dmamem")))
#else
    #define __UHSDR_DMAMEM
#endif // STM32H7

#if defined(STM32H7)
    #define hdac hdac1
#endif // STM32H7

#define USE_TWO_CHANNEL_AUDIO
#define USE_HMC1023

#if defined(STM32F7)
    #ifndef TRX_NAME
        #define TRX_NAME "OVI40 F7"
    #endif
    #ifndef TRX_ID
        #define TRX_ID "40sdr"
    #endif
#elif defined(STM32H7)
    #ifndef TRX_NAME
        #define TRX_NAME "OVI40 H7"
    #endif
    #ifndef TRX_ID
        #define TRX_ID "ovi40"
    #endif
#endif

#ifndef TRX_HW_LIC
// #define TRX_HW_LIC "???"
// #define TRX_HW_CREATOR "???"
#endif // TRX_HW_LIC

#define SI570_I2C               (&hi2c1)
#define SI5351A_I2C             (&hi2c1)
#define MCP_I2C                 (&hi2c1)

#define CODEC_ANA_I2C               (&hi2c2)
#define CODEC_ANA_SAI               SAI1

#define CODEC_IQ_I2C                (&hi2c4)
#define CODEC_IQ_SAI                SAI2

#define CODEC_NUM               2

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
#define TXRX_CNTR                GPIO_PIN_1
#define TXRX_CNTR_PIO            GPIOB
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
#define IQ_CLOCK_DIV4_SIG             GPIO_PIN_9
#define IQ_CLOCK_DIV4_SIG_PIO         GPIOC
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
#define LCD_D13                 GPIO_PIN_8
#define LCD_D13_PIO             GPIOD
// pin 9
#define LCD_D14                 GPIO_PIN_9
#define LCD_D14_PIO             GPIOD
// pin 10
#define LCD_D15                 GPIO_PIN_10
#define LCD_D15_PIO             GPIOD
// pin 11
//SAI INTERFACE: SAI2_SD_A
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
#define LCD_D8                  GPIO_PIN_11
#define LCD_D8_PIO              GPIOE
// pin 12
#define LCD_D9                  GPIO_PIN_12
#define LCD_D9_PIO              GPIOE
// pin 13
#define LCD_D10                 GPIO_PIN_13
#define LCD_D10_PIO             GPIOE
// pin 14
#define LCD_D11                 GPIO_PIN_14
#define LCD_D11_PIO             GPIOE
// pin 15
#define LCD_D12                 GPIO_PIN_15
#define LCD_D12_PIO             GPIOE

// -----------------------------------------------------------------------------
// ----                         PORT F                                      ----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_M1               GPIO_PIN_0
#define BUTTON_M1_PIO           GPIOF

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
#define BUTTON_S19              GPIO_PIN_11
#define BUTTON_S19_PIO          GPIOF
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
#define LCD_RS                  GPIO_PIN_3
#define LCD_RS_PIO              GPIOG
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

// PORT H
// pin1
#define BUTTON_S18              GPIO_PIN_1
#define BUTTON_S18_PIO          GPIOH

#endif // __UHSDR_UI_OVI40_CONFIG_H
