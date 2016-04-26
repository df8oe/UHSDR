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
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/
#ifndef __MCHF_BOARD_H
#define __MCHF_BOARD_H

// HW libs
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_wwdg.h"
#include "stm32f4xx_flash.h"
#include "misc.h"
#include "core_cm4.h"

#include "stm32f4xx.h"
#include "mchf_types.h"
#include "audio_filter.h"
#include "ui_si570.h"
//
//
//
// -----------------------------------------------------------------------------
#define		DEVICE_STRING			"mcHF QRP Transceiver"
#define 	AUTHOR_STRING   		"K. Atanassov - M\x60NKA 2014-2016"
//
#define 	TRX4M_VER_MAJOR			0
#define 	TRX4M_VER_MINOR			219
#define 	TRX4M_VER_RELEASE		27
//
#define 	TRX4M_VER_BUILD			12
//

#define		ATTRIB_STRING1			"Additional Contributions by"
#define		ATTRIB_STRING2			"KA7OEI, DF8OE and others."
#define		ATTRIB_STRING3			"Licensed under CC BY-NC-SA 3.0"
//
// -----------------------------------------------------------------------------
//#define 	DEBUG_BUILD

// hardware specific switches
//#define hY28BHISPEED			true		// uncomment for using new HY28B in SPI with bus speed 50MHz instead of 25MHz


#define		WD_REFRESH_WINDOW		80
#define		WD_REFRESH_COUNTER		127

// -----------------------------------------------------------------------------
//						PORT PINS ALLOCATION
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// ---- 						PORT A										----
// -----------------------------------------------------------------------------
// pin 0
#define ENC_THREE_CH1 		GPIO_Pin_0
#define ENC_THREE_CH1_SOURCE	GPIO_PinSource0
#define ENC_THREE_CH1_PIO       GPIOA
// pin 1
#define ENC_THREE_CH2 		GPIO_Pin_1
#define ENC_THREE_CH2_SOURCE	GPIO_PinSource1
#define ENC_THREE_CH2_PIO       GPIOA
// pin 2
#define ADC3_FWD		GPIO_Pin_2
#define ADC3_FWD_SOURCE		GPIO_PinSource2
#define ADC3_FWD_PIO       	GPIOA
// pin 3
#define ADC2_RET		GPIO_Pin_3
#define ADC2_RET_SOURCE		GPIO_PinSource3
#define ADC2_RET_PIO       	GPIOA
// pin 4
#define TP_IRQ			GPIO_Pin_4
#define TP_IRQ_SOURCE		GPIO_PinSource4
#define TP_IRQ_PIO		GPIOA
#define DAC0 			GPIO_Pin_4
#define DAC0_SOURCE		GPIO_PinSource4
#define DAC0_PIO       		GPIOA
// pin 5
#define DAC1 			GPIO_Pin_5
#define DAC1_SOURCE		GPIO_PinSource5
#define DAC1_PIO       		GPIOA
// pin 6
#define ADC1_PWR		GPIO_Pin_6
#define ADC1_PWR_SOURCE		GPIO_PinSource6
#define ADC1_PWR_PIO       	GPIOA
// pin 7
#define BAND0		 	GPIO_Pin_7
#define BAND0_SOURCE		GPIO_PinSource7
#define BAND0_PIO       	GPIOA
// pin 8
#define BAND1		 	GPIO_Pin_8
#define BAND1_SOURCE		GPIO_PinSource8
#define BAND1_PIO       	GPIOA
// pin 9
#define DEBUG_PRINT	 	GPIO_Pin_9
#define DEBUG_PRINT_SOURCE	GPIO_PinSource9
#define DEBUG_PRINT_PIO    	GPIOA
#define TP_CS			GPIO_Pin_9
#define TP_CS_SOURCE		GPIO_PinSource9
#define TP_CS_PIO		GPIOA
// pin 10
#define BAND2 			GPIO_Pin_10
#define BAND2_SOURCE		GPIO_PinSource10
#define BAND2_PIO       	GPIOA
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
#define CODEC_I2S_WS		GPIO_Pin_15
#define CODEC_I2S_WS_SOURCE	GPIO_PinSource15
#define CODEC_I2S_WS_PIO  	GPIOA
//
// -----------------------------------------------------------------------------
// ---- 						PORT B										----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_BNDM		GPIO_Pin_0
#define BUTTON_BNDM_SOURCE	GPIO_PinSource0
#define BUTTON_BNDM_PIO       	GPIOB
// pin 1
#define PTT_CNTR 		GPIO_Pin_1
#define PTT_CNTR_SOURCE		GPIO_PinSource1
#define PTT_CNTR_PIO       	GPIOB
// pin 2
#define BUTTON_BNDP 		GPIO_Pin_2
#define BUTTON_BNDP_SOURCE	GPIO_PinSource2
#define BUTTON_BNDP_PIO       	GPIOB
// pin 3
#define BUTTON_M2 		GPIO_Pin_3
#define BUTTON_M2_SOURCE	GPIO_PinSource3
#define BUTTON_M2_PIO       	GPIOB
// pin 4
#define ENC_ONE_CH1 		GPIO_Pin_4
#define ENC_ONE_CH1_SOURCE	GPIO_PinSource4
#define ENC_ONE_CH1_PIO       	GPIOB
// pin 5
#define ENC_ONE_CH2 		GPIO_Pin_5
#define ENC_ONE_CH2_SOURCE	GPIO_PinSource5
#define ENC_ONE_CH2_PIO       	GPIOB
// pin 6
#define I2C1_SCL_PIN            GPIO_Pin_6
#define I2C1_SCL_PINSRC         GPIO_PinSource6
#define I2C1_SCL_GPIO           GPIOB
// pin 7
#define I2C1_SDA_PIN            GPIO_Pin_7
#define I2C1_SDA_PINSRC         GPIO_PinSource7
#define I2C1_SDA_GPIO           GPIOB
// pin 8
#define BUTTON_G3 		GPIO_Pin_8
#define BUTTON_G3_SOURCE	GPIO_PinSource8
#define BUTTON_G3_PIO       	GPIOB
// pin 9
#define GREEN_LED 		GPIO_Pin_9
#define GREEN_LED_SOURCE	GPIO_PinSource9
#define GREEN_LED_PIO       	GPIOB
// pin 10
#define I2C2_SCL_PIN            GPIO_Pin_10
#define I2C2_SCL_PINSRC         GPIO_PinSource10
#define I2C2_SCL_GPIO           GPIOB
// pin 11
#define I2C2_SDA_PIN            GPIO_Pin_11
#define I2C2_SDA_PINSRC         GPIO_PinSource11
#define I2C2_SDA_GPIO           GPIOB
// pin 12
#define RED_LED 		GPIO_Pin_12
#define RED_LED_SOURCE		GPIO_PinSource12
#define RED_LED_PIO       	GPIOB
// pin 13
#define LCD_SCK 		GPIO_Pin_13
#define LCD_SCK_SOURCE		GPIO_PinSource13
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
#define BUTTON_G4 		GPIO_Pin_0
#define BUTTON_G4_SOURCE	GPIO_PinSource0
#define BUTTON_G4_PIO       	GPIOC
// pin 1
#define BUTTON_M3 		GPIO_Pin_1
#define BUTTON_M3_SOURCE	GPIO_PinSource1
#define BUTTON_M3_PIO       	GPIOC
// pin 2
#define LCD_MISO 		GPIO_Pin_2
#define LCD_MISO_SOURCE		GPIO_PinSource2
#define LCD_MISO_PIO         	GPIOC
// pin 3
#define LCD_MOSI 		GPIO_Pin_3
#define LCD_MOSI_SOURCE		GPIO_PinSource3
#define LCD_MOSI_PIO         	GPIOC
// pin 4
#define BUTTON_STEPM		GPIO_Pin_4
#define BUTTON_STEPM_SOURCE	GPIO_PinSource4
#define BUTTON_STEPM_PIO       	GPIOC
// pin 5
#define BUTTON_STEPP		GPIO_Pin_5
#define BUTTON_STEPP_SOURCE	GPIO_PinSource5
#define BUTTON_STEPP_PIO       	GPIOC
// pin 6
#define FREQ_ENC_CH1 		GPIO_Pin_6
#define FREQ_ENC_CH1_SOURCE	GPIO_PinSource6
#define FREQ_ENC_CH1_PIO        GPIOC
// pin 7
#define FREQ_ENC_CH2 		GPIO_Pin_7
#define FREQ_ENC_CH2_SOURCE	GPIO_PinSource7
#define FREQ_ENC_CH2_PIO        GPIOC
// pin 8
#define POWER_DOWN 		GPIO_Pin_8
#define POWER_DOWN_SOURCE	GPIO_PinSource8
#define POWER_DOWN_PIO         	GPIOC
// pin 9
#define CODEC_CLOCK 		GPIO_Pin_9
#define CODEC_CLOCK_SOURCE	GPIO_PinSource9
#define CODEC_CLOCK_PIO         GPIOC
// pin 10
#define CODEC_I2S_SCK 		GPIO_Pin_10
#define CODEC_I2S_SCK_SOURCE	GPIO_PinSource10
#define CODEC_I2S_SCK_PIO       GPIOC
// pin 11
#define CODEC_I2S_SDI 		GPIO_Pin_11
#define CODEC_I2S_SDI_SOURCE	GPIO_PinSource11
#define CODEC_I2S_SDI_PIO       GPIOC
// pin 12
#define CODEC_I2S_SDO 		GPIO_Pin_12
#define CODEC_I2S_SDO_SOURCE	GPIO_PinSource12
#define CODEC_I2S_SDO_PIO       GPIOC
// pin 13
#define BUTTON_PWR		GPIO_Pin_13
#define BUTTON_PWR_SOURCE	GPIO_PinSource13
#define BUTTON_PWR_PIO       	GPIOC
// pin 14
#define BUTTON_M1		GPIO_Pin_14
#define BUTTON_M1_SOURCE	GPIO_PinSource14
#define BUTTON_M1_PIO       	GPIOC
// pin 15
#define BUTTON_F3		GPIO_Pin_15
#define BUTTON_F3_SOURCE	GPIO_PinSource15
#define BUTTON_F3_PIO       	GPIOC
//
// -----------------------------------------------------------------------------
// ---- 						PORT D										----
// -----------------------------------------------------------------------------
// pin 0
#define LCD_D2			GPIO_Pin_0
#define LCD_D2_SOURCE		GPIO_PinSource0
#define LCD_D2_PIO      	GPIOD
// pin 1
#define LCD_D3			GPIO_Pin_1
#define LCD_D3_SOURCE		GPIO_PinSource1
#define LCD_D3_PIO      	GPIOD
// pin 2
#define LCD_BACKLIGHT		GPIO_Pin_2
#define LCD_BACKLIGHT_SOURCE	GPIO_PinSource2
#define LCD_BACKLIGHT_PIO      	GPIOD
// pin 3
#define LCD_RESET		GPIO_Pin_3
#define LCD_RESET_SOURCE	GPIO_PinSource3
#define LCD_RESET_PIO      	GPIOD
// pin 4
#define LCD_RD			GPIO_Pin_4
#define LCD_RD_SOURCE		GPIO_PinSource4
#define LCD_RD_PIO      	GPIOD
// pin 5
#define LCD_WR			GPIO_Pin_5
#define LCD_WR_SOURCE		GPIO_PinSource5
#define LCD_WR_PIO      	GPIOD
// pin 6
#define BUTTON_F1		GPIO_Pin_6
#define BUTTON_F1_SOURCE	GPIO_PinSource6
#define BUTTON_F1_PIO       	GPIOD
// pin 7
#define LCD_CSA			GPIO_Pin_7
#define LCD_CSA_SOURCE		GPIO_PinSource7
#define LCD_CSA_PIO      	GPIOD
// pin 8
#define LCD_D15			GPIO_Pin_8
#define LCD_D15_SOURCE		GPIO_PinSource8
#define LCD_D15_PIO      	GPIOD
// pin 9
#define LCD_D16			GPIO_Pin_9
#define LCD_D16_SOURCE		GPIO_PinSource9
#define LCD_D16_PIO      	GPIOD
// pin 10
#define LCD_D17			GPIO_Pin_10
#define LCD_D17_SOURCE		GPIO_PinSource10
#define LCD_D17_PIO      	GPIOD
// pin 11
#define LCD_RS			GPIO_Pin_11
#define LCD_RS_SOURCE		GPIO_PinSource11
#define LCD_RS_PIO      	GPIOD
// pin 12
#define ENC_TWO_CH1 		GPIO_Pin_12
#define ENC_TWO_CH1_SOURCE	GPIO_PinSource12
#define ENC_TWO_CH1_PIO         GPIOD
// pin 13
#define ENC_TWO_CH2 		GPIO_Pin_13
#define ENC_TWO_CH2_SOURCE	GPIO_PinSource13
#define ENC_TWO_CH2_PIO         GPIOD
// pin 14
#define LCD_D0			GPIO_Pin_14
#define LCD_D0_SOURCE		GPIO_PinSource14
#define LCD_D0_PIO      	GPIOD
// pin 15
#define LCD_D1			GPIO_Pin_15
#define LCD_D1_SOURCE		GPIO_PinSource15
#define LCD_D1_PIO      	GPIOD
//
// -----------------------------------------------------------------------------
// ---- 						PORT E										----
// -----------------------------------------------------------------------------
// pin 0
#define PADDLE_DAH		GPIO_Pin_0
#define PADDLE_DAH_SOURCE	GPIO_PinSource0
#define PADDLE_DAH_PIO       	GPIOE
// pin 1
#define PADDLE_DIT		GPIO_Pin_1
#define PADDLE_DIT_SOURCE	GPIO_PinSource1
#define PADDLE_DIT_PIO       	GPIOE
// pin 2
#define BUTTON_F2		GPIO_Pin_2
#define BUTTON_F2_SOURCE	GPIO_PinSource2
#define BUTTON_F2_PIO       	GPIOE
// pin 3
#define BUTTON_F4		GPIO_Pin_3
#define BUTTON_F4_SOURCE	GPIO_PinSource3
#define BUTTON_F4_PIO       	GPIOE
// pin 4
#define BUTTON_G2		GPIO_Pin_4
#define BUTTON_G2_SOURCE	GPIO_PinSource4
#define BUTTON_G2_PIO       	GPIOE
// pin 5
#define BUTTON_F5		GPIO_Pin_5
#define BUTTON_F5_SOURCE	GPIO_PinSource5
#define BUTTON_F5_PIO       	GPIOE
// pin 6
#define BUTTON_G1		GPIO_Pin_6
#define BUTTON_G1_SOURCE	GPIO_PinSource6
#define BUTTON_G1_PIO       	GPIOE
// pin 7
#define LCD_D4			GPIO_Pin_7
#define LCD_D4_SOURCE		GPIO_PinSource7
#define LCD_D4_PIO      	GPIOE
// pin 8
#define LCD_D5			GPIO_Pin_8
#define LCD_D5_SOURCE		GPIO_PinSource8
#define LCD_D5_PIO      	GPIOE
// pin 9
#define LCD_D6			GPIO_Pin_9
#define LCD_D6_SOURCE		GPIO_PinSource9
#define LCD_D6_PIO      	GPIOE
// pin 10
#define LCD_D7			GPIO_Pin_10
#define LCD_D7_SOURCE		GPIO_PinSource10
#define LCD_D7_PIO      	GPIOE
// pin 11
#define LCD_D10			GPIO_Pin_11
#define LCD_D10_SOURCE		GPIO_PinSource11
#define LCD_D10_PIO      	GPIOE
// pin 12
#define LCD_D11			GPIO_Pin_12
#define LCD_D11_SOURCE		GPIO_PinSource12
#define LCD_D11_PIO        	GPIOE
// pin 13
#define LCD_D12			GPIO_Pin_13
#define LCD_D12_SOURCE		GPIO_PinSource13
#define LCD_D12_PIO      	GPIOE
// pin 14
#define LCD_D13			GPIO_Pin_14
#define LCD_D13_SOURCE		GPIO_PinSource14
#define LCD_D13_PIO      	GPIOE
// pin 15
#define LCD_D14			GPIO_Pin_15
#define LCD_D14_SOURCE		GPIO_PinSource15
#define LCD_D14_PIO      	GPIOE
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Buttons map structure
typedef struct ButtonMap
{
    GPIO_TypeDef 	*port;
    ushort			button;

} ButtonMap;

// Button definitions
//
enum
{
    BUTTON_M2_PRESSED = 0,  // 0
    BUTTON_G3_PRESSED,  // 1
    BUTTON_G2_PRESSED,  // 2
    BUTTON_BNDM_PRESSED,    // 3
    BUTTON_G4_PRESSED,  // 4
    BUTTON_M3_PRESSED,  // 5
    BUTTON_STEPM_PRESSED,   // 6
    BUTTON_STEPP_PRESSED,   // 7
    BUTTON_M1_PRESSED,  // 8
    BUTTON_F3_PRESSED,  // 9 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F1_PRESSED,  // 10 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F2_PRESSED,  // 11 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F4_PRESSED,  // 12 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_BNDP_PRESSED,    // 13
    BUTTON_F5_PRESSED,  // 14 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_G1_PRESSED,  // 15
    BUTTON_POWER_PRESSED,   // 16 - Used for press and release
    TOUCHSCREEN_ACTIVE, // 17 - Touchscreen touched, needs to last entry before BUTTON_NUM,
    //      init code relies on this
    BUTTON_NUM // How many buttons we have defined
};

extern const ButtonMap  bm[BUTTON_NUM];

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define POWER_BUTTON_HOLD_TIME		1000000

#define TRX_MODE_RX			0
#define TRX_MODE_TX			1

#define DEMOD_USB			0
#define DEMOD_LSB			1
#define DEMOD_CW			2
#define DEMOD_AM			3
#define	DEMOD_FM			4
#define	DEMOD_SAM			5
#define DEMOD_DIGI			6
#define DEMOD_MAX_MODE			6

#define RTC_OSC_FREQ			32768

#define	TCXO_OFF			0		// TXCO temperature compensation off
#define	TCXO_ON				1		// TCXO temperature compensation on
#define	TCXO_STOP			2		// Stop reading of temperature sensor
#define	TCXO_TEMP_STATE_MAX		2		// Maximum setting for TCXO setting state

// Transverter oscillator adds shift
#define		TRANSVT_FREQ_A	 	42000000

//
#define		MIN_FREQ_CAL		-9999		// Minimum and maximum range of frequency calibration in Hz (referenced to 14.000 MHz)
#define		MAX_FREQ_CAL		9999
//
// Total bands supported
//
#define	MIN_BANDS			0		// lowest band number
#define	MAX_BANDS			17		// Highest band number:  17 = General coverage (RX only) band
#define	MAX_BAND_NUM		(MAX_BANDS+1)		// Number of Bands

//  multiplier to convert between dial_freq and tune_freq
#define TUNE_MULT 4

#define	KHZ_MULT			(TUNE_MULT*1000)	// multiplier to convert oscillator frequency or band size to display kHz, used below
//
// Bands definition
// - ID
// - SI570 startup freq
// - size in Hz
//
#define	BAND_MODE_80			0
#define	BAND_FREQ_80			3500*KHZ_MULT		// 3500 kHz
#define	BAND_SIZE_80			500*KHZ_MULT		// 500 kHz in size (Region 2)
//
#define	BAND_MODE_60			1
#define	BAND_FREQ_60			5250*KHZ_MULT		// 5250 kHz
#define	BAND_SIZE_60			200*KHZ_MULT		// 200 kHz in size to allow different allocations
//
#define	BAND_MODE_40			2
#define	BAND_FREQ_40			7000*KHZ_MULT		// 7000 kHz
#define	BAND_SIZE_40			300*KHZ_MULT		// 300 kHz in size (Region 2)
//
#define	BAND_MODE_30			3
#define	BAND_FREQ_30			10100*KHZ_MULT		// 10100 kHz
#define	BAND_SIZE_30			50*KHZ_MULT		// 50 kHz in size
//
#define	BAND_MODE_20			4
#define	BAND_FREQ_20			14000*KHZ_MULT		// 14000 kHz
#define	BAND_SIZE_20			350*KHZ_MULT		// 350 kHz in size
//
#define	BAND_MODE_17			5
#define	BAND_FREQ_17			18068*KHZ_MULT		// 18068 kHz
#define	BAND_SIZE_17			100*KHZ_MULT		// 100 kHz in size
//
#define	BAND_MODE_15			6
#define	BAND_FREQ_15			21000*KHZ_MULT		// 21000 kHz
#define	BAND_SIZE_15			450*KHZ_MULT		// 450 kHz in size
//
#define	BAND_MODE_12			7
#define	BAND_FREQ_12			24890*KHZ_MULT		// 24890 kHz
#define	BAND_SIZE_12			100*KHZ_MULT		// 100 kHz in size
//
#define	BAND_MODE_10			8
#define	BAND_FREQ_10			28000*KHZ_MULT		// 28000 kHz
#define	BAND_SIZE_10			1700*KHZ_MULT		// 1700 kHz in size
//
#define	BAND_MODE_6			9
#define	BAND_FREQ_6			50000*KHZ_MULT		// 50000 kHz
#define	BAND_SIZE_6			2000*KHZ_MULT		// 2000 kHz in size (Region 2)
//
#define	BAND_MODE_4			10
#define	BAND_FREQ_4			70000*KHZ_MULT		// 70000 kHz
#define	BAND_SIZE_4			500*KHZ_MULT		// 500 kHz in size (Region 2)
//
#define	BAND_MODE_2			11
#define	BAND_FREQ_2			144000*KHZ_MULT		// 144000 kHz
#define	BAND_SIZE_2			2000*KHZ_MULT		// 2000 kHz in size (Region 1)
//
#define	BAND_MODE_70			12
#define	BAND_FREQ_70			430000*KHZ_MULT		// 430000 kHz
#define	BAND_SIZE_70			10000*KHZ_MULT		// 10000 kHz in size (Region 1)
//
#define	BAND_MODE_23			13
#define	BAND_FREQ_23			450000*KHZ_MULT		// 1240000 kHz
#define	BAND_SIZE_23			10000*KHZ_MULT		// 60000 kHz in size (Region 1)
//
#define	BAND_MODE_2200			14
#define	BAND_FREQ_2200			135.7*KHZ_MULT		// 135.7 kHz
#define	BAND_SIZE_2200			2.1*KHZ_MULT		// 2.1 kHz in size (Region 1)
//
#define	BAND_MODE_630			15
#define	BAND_FREQ_630			472*KHZ_MULT		// 472 kHz
#define	BAND_SIZE_630			7*KHZ_MULT		// 7 kHz in size (Region 1)
//
#define	BAND_MODE_160			16
#define	BAND_FREQ_160			1800*KHZ_MULT		// 1810 kHz
#define	BAND_SIZE_160			190*KHZ_MULT		// 190 kHz in size (Region 1)
//
#define	BAND_MODE_GEN			17			// General Coverage
#define	BAND_FREQ_GEN			10000*KHZ_MULT		// 10000 kHz
#define	BAND_SIZE_GEN			1*KHZ_MULT		// Dummy variable

//
//
//
//	Frequency limits for filters, in Hz, for bandpass filter selection - MODIFY AT YOUR OWN RISK!
//
#define	BAND_FILTER_UPPER_160		2500000				// Upper limit for 160 meter filter
//
#define	BAND_FILTER_UPPER_80		4250000				// Upper limit for 80 meter filter
//
#define	BAND_FILTER_UPPER_40		8000000				// Upper limit for 40/60 meter filter
//
#define	BAND_FILTER_UPPER_20		16000000			// Upper limit for 20/30 meter filter

#define	BAND_FILTER_UPPER_10		32000000			// Upper limit for 10 meter filter
//
#define	BAND_FILTER_UPPER_6		40000000			// Upper limit for 6 meter filter
//
#define	BAND_FILTER_UPPER_4		70000000			// Upper limit for 4 meter filter
//
#define	DEFAULT_FREQ_OFFSET		4000				// Amount of offset (at LO freq) when loading "default" frequency
//
// encoder one
#define ENC_ONE_MODE_AUDIO_GAIN		0
#define ENC_ONE_MODE_ST_GAIN		1
#define ENC_ONE_MAX_MODE			1
//
// encoder two
#define ENC_TWO_MODE_RF_GAIN		0
#define ENC_TWO_MODE_SIG_PROC		1
#define ENC_TWO_MODE_NOTCH_F		2
#define ENC_TWO_MODE_PEAK_F			3
#define ENC_TWO_MODE_BASS_GAIN		4
#define ENC_TWO_MODE_TREBLE_GAIN	5
#define ENC_TWO_MAX_MODE			6
//
// encoder three
#define ENC_THREE_MODE_RIT			0
#define ENC_THREE_MODE_CW_SPEED		1
#define ENC_THREE_MAX_MODE			2
//
//
#define CW_MODE_IAM_B				0
#define CW_MODE_IAM_A				1
#define CW_MODE_STRAIGHT			2
#define CW_MAX_MODE					3

// PA power level setting enumeration
enum
{
    PA_LEVEL_FULL = 0,
    PA_LEVEL_5W,
    PA_LEVEL_2W,
    PA_LEVEL_1W,
    PA_LEVEL_0_5W,
    PA_LEVEL_MAX_ENTRY
};
//
#define	PA_LEVEL_DEFAULT		PA_LEVEL_2W		// Default power level

#define	US_DELAY			15  // 15 gives 1 uS delay in loop without optimization(O0)

#define	CW_SIDETONE_FREQ_DEFAULT	750	// Default CW Audio Sidetone and TX offset frequency
//
#define	CW_SIDETONE_FREQ_MIN		400
#define	CW_SIDETONE_FREQ_MAX		1000
//
#define	SSB_TUNE_FREQ			750	// Frequency at which the SSB TX IQ gain and phase adjustment is to be done
//
#define SSB_RX_DELAY			450	// Delay for switching when going from TX to RX
//
#define	CW_RX_DELAY_MAX			50	// Maximum TX to RX turnaround setting
#define	CW_RX_DELAY_DEFAULT		8
//

// Audio sources for TX modulation
#define TX_AUDIO_MIC			0
#define TX_AUDIO_LINEIN_L		1
#define TX_AUDIO_LINEIN_R		2
#define TX_AUDIO_DIG			3
#define TX_AUDIO_DIGIQ			4
#define TX_AUDIO_MAX_ITEMS		4
#define TX_AUDIO_NUM			(TX_AUDIO_MAX_ITEMS +1)
//
#define	LINE_GAIN_MIN			3
#define	LINE_GAIN_MAX			31
#define	LINE_GAIN_DEFAULT		12		// Original fixed gain setting
//
#define	MIC_GAIN_MIN			2
#define	MIC_GAIN_MAX			99
#define	MIC_GAIN_DEFAULT		15		// Default value - close to original fixed setting
//
//
#define	TX_POWER_FACTOR_MIN		3		// Minimum power factor setting (3 = 0.03)
#define	TX_POWER_FACTOR_MAX		85		// Maximum power factor setting (75 = 0.75)
//
// Default power factors for 5 watt and FULL settings in percent
// These power factors are based on the original fixed values
//
#define TX_POWER_FACTOR_80_DEFAULT	50
#define	TX_POWER_FACTOR_60_DEFAULT	20
#define	TX_POWER_FACTOR_40_DEFAULT	10
#define	TX_POWER_FACTOR_30_DEFAULT	13
#define	TX_POWER_FACTOR_20_DEFAULT	30
#define	TX_POWER_FACTOR_17_DEFAULT	40
#define	TX_POWER_FACTOR_15_DEFAULT	50
#define	TX_POWER_FACTOR_12_DEFAULT	75
#define	TX_POWER_FACTOR_10_DEFAULT	75
#define TX_POWER_FACTOR_6_DEFAULT	75
#define TX_POWER_FACTOR_4_DEFAULT	75
#define TX_POWER_FACTOR_2_DEFAULT	75
#define TX_POWER_FACTOR_70_DEFAULT	75
#define TX_POWER_FACTOR_23_DEFAULT	75
#define TX_POWER_FACTOR_2200_DEFAULT	50
#define TX_POWER_FACTOR_630_DEFAULT	50
#define TX_POWER_FACTOR_160_DEFAULT	50
//
// Enumeration of colours used in spectrum scope display
//
enum
{
    SPEC_WHITE = 0,
    SPEC_GREY,
    SPEC_BLUE,
    SPEC_RED1,
    SPEC_RED2,
    SPEC_RED3,
    SPEC_MAGENTA,
    SPEC_GREEN,
    SPEC_CYAN,
    SPEC_YELLOW,
    SPEC_ORANGE,
    SPEC_CREAM,
    SPEC_BLACK,
    SPEC_GREY1,
    SPEC_GREY2,
    SPEC_GREY3,
    SPEC_GREY4,
    SPEC_GREY5,
    SPEC_GREY6,
    SPEC_MAX_COLOUR,
};
//
#define	SPEC_COLOUR_TRACE_DEFAULT	SPEC_WHITE
#define	SPEC_COLOUR_GRID_DEFAULT	SPEC_GREY
#define SPEC_COLOUR_SCALE_DEFAULT	SPEC_GREY
#define	FILTER_DISP_COLOUR_DEFAULT	SPEC_GREY
//
// Enumeration of transmit meter modes
//
enum
{
    METER_SWR = 0,
    METER_AUDIO,
    METER_ALC,
    METER_MAX,
};
//
#define	BACKLIGHT_BLANK_TIMING_DEFAULT	8		// default number of SECONDS for backlight blanking
#define LCD_STARTUP_BLANKING_TIME	3000		// number of DECISECONDS (e.g. SECONDS * 100) after power-up before LCD blanking occurs if no buttons are pressed/knobs turned

#define FILT_DISPLAY_WIDTH      256     // width, in pixels, of the spectral display on the screen - this value used to calculate Hz/pixel for indicating width of filter


enum
{
    DISPLAY_NONE = 0,
    DISPLAY_HY28A_SPI,
    DISPLAY_HY28B_SPI,
    DISPLAY_HY28B_PARALLEL
};

typedef struct Gain_s
{
    uint8_t value;
    uint8_t max;
    uint8_t value_old;
    float   active_value;
} Gain;
//
// Bands tuning values - WORKING registers - used "live" during transceiver operation
// (May contain VFO A, B or "Memory" channel values)
//
struct vfo_reg_s
{
    uint32_t dial_value;
    uint32_t decod_mode;
//    uint32_t filter_mode;
};

typedef struct vfo_reg_s VfoReg;

struct band_regs_s
{
    VfoReg band[MAX_BANDS+1];
};
typedef struct band_regs_s BandRegs;

enum
{
    // VFO_WORK = 0
    VFO_A = 0,
    VFO_B,
    VFO_MAX
};
// Working register plus VFO A and VFO B registers.
extern __IO BandRegs vfo[VFO_MAX];


// Transceiver state public structure
typedef struct TransceiverState
{
    // Sampling rate public flag
    ulong 	samp_rate;

    // Virtual pots public values
    short  	rit_value;

#define RX_AUDIO_SPKR 0
#define RX_AUDIO_DIG  1
    Gain    rx_gain[2]; //ts.rx_gain[RX_AUDIO_SPKR].value

    int 	rf_gain;			// RF gain control
    uchar	rf_codec_gain;		// gain for codec (A/D converter) in receive mode
    uchar 	nb_setting;
    uchar	st_gain;
    uchar	pa_bias;
    uchar	pa_cw_bias;

    // flag to show delayed request for unmute afte TX->RX change (remove clicks)
    uchar	audio_unmute;
    bool	buffer_clear;

    int  	tx_iq_lsb_gain_balance;		// setting for TX IQ gain balance
    int  	tx_iq_usb_gain_balance;		// setting for TX IQ gain balance
    //
    int		tx_iq_lsb_phase_balance;	// setting for TX IQ phase balance
    int		tx_iq_usb_phase_balance;	// setting for TX IQ phase balance
    //
    int		tx_iq_am_gain_balance;		// setting for TX IQ gain balance
    int		tx_iq_fm_gain_balance;		// setting for TX IQ gain balance
    //
    float	tx_adj_gain_var_i;		// active variables for adjusting tx gain balance
    float	tx_adj_gain_var_q;

    int		rx_iq_lsb_gain_balance;		// setting for RX IQ gain balance
    int		rx_iq_usb_gain_balance;		// setting for RX IQ gain balance
    //
    int		rx_iq_am_gain_balance;		// setting for AM RX IQ gain balance
    int		rx_iq_am_phase_balance;		// setting for AM RX IQ phase balance
    int		rx_iq_fm_gain_balance;		// setting for FM RX IQ gain balance
    //
    //
    int		rx_iq_lsb_phase_balance;	// setting for RX IQ phase balance
    int		rx_iq_usb_phase_balance;	// setting for RX IQ phase balance

    float	rx_adj_gain_var_i;		// active variables for adjusting rx gain balance
    float	rx_adj_gain_var_q;
    //
    // Equalisation factor
    float	tx_power_factor;

    int	freq_cal;				// frequency calibration

    // Frequency synthesizer
    ulong	tune_freq;			// main synthesizer frequency
    // ulong	tune_freq_old;		// used to detect change of main synthesizer frequency

    // Transceiver calibration mode flag
    //uchar	calib_mode;

    // Transceiver menu mode variables
    uchar	menu_mode;		// TRUE if in menu mode
    int16_t	menu_item;		// Used to indicate specific menu item
    int		menu_var;		// Used to change specific menu item
    bool	menu_var_changed;	// TRUE if something changed in a menu and that an EEPROM save should be done!

    // Ham band public flag
    // index of bands table in Flash
    uchar 	band;
    bool	band_change;
    uchar	filter_band;		// filter selection band:  1= 80, 2= 60/40, 3=30/20, 4=17/15/12/10 - used for selection of power detector coefficient selection.
    //
    // Receive/Transmit public flag
    uchar 	txrx_mode;

    // TX/RX IRQ lock, to prevent reentrance
    //uchar	txrx_lock;
    uchar	ptt_req;


    // Demodulator mode public flag
    uchar 	dmod_mode;


    uchar 	enc_one_mode;
    uchar 	enc_two_mode;
    uchar 	enc_thr_mode;

    uchar	tx_meter_mode;				// meter mode

    // Audio filter ID
    // uchar	filter_id;
    //
    uint8_t   filter_select[AUDIO_FILTER_NUM];


#define FILTER_PATH_MEM_MAX 5
    uint16_t   filter_path_mem[FILTER_MODE_MAX][FILTER_PATH_MEM_MAX];

    uint16_t  filter_path;
    //

    uchar	filter_cw_wide_disable;		// TRUE if wide filters are disabled in CW mode
    uchar	filter_ssb_narrow_disable;	// TRUE if narrow filters are disabled in SSB modes
    //
    uchar	am_mode_disable;			// TRUE if AM mode is to be disabled

    // AGC mode
    uchar	agc_mode;
    uchar	agc_custom_decay;

    uchar	max_rf_gain;

    // Eth to UI driver requests flag
    uchar	LcdRefreshReq;

    // Eth to UI public flag
    uchar	new_band;
    uchar	new_mode;
    uchar	new_digi_mode;

    // Current CW mode
    uchar	keyer_mode;
    uchar	keyer_speed;
    ulong	sidetone_freq;
    uchar	paddle_reverse;
    uchar	cw_rx_delay;
    ulong	unmute_delay_count;

    uchar	power_level;

    uchar 	tx_audio_source;
    ulong	tx_mic_gain_mult;
    uchar	tx_gain[TX_AUDIO_NUM];
    uchar	tx_comp_level;			// Used to hold compression level which is used to calculate other values for compression.  0 = manual.

    // Microphone gain boost of +20dB via Codec command (TX)
    uchar	mic_boost;

    // Global tuning flag - in every demod mode
    uchar 	tune;

    uint16_t ee_init_stat;

    uchar	powering_down;

    // Spectrum Scope config - placed here since "sd." not defined at time of init

    uchar	scope_speed;	// update rate for spectrum scope

    uchar	scope_filter;	// strength of filter in spectrum scope

    uchar	scope_trace_colour;	// color of spectrum scope trace;
    uchar	scope_grid_colour;	// saved color of spectrum scope grid;
    ulong	scope_grid_colour_active;	// active color of spectrum scope grid;
    uchar	scope_centre_grid_colour;	// color of center line of scope grid
    ulong	scope_centre_grid_colour_active;	// active colour of the spectrum scope center grid line
    uchar	scope_scale_colour;	// color of spectrum scope frequency scale
    uchar	scope_rescale_rate;	// rescale rate on the 'scope
    uchar	scope_agc_rate;		// agc rate on the 'scope
    uchar	spectrum_db_scale;	// db/Division scale setting on spectrum scope
    uchar	waterfall_speed;	// speed of update of the waterfall
    //
    bool	radio_config_menu_enable;	// TRUE if radio configuration menu is to be visible
    //
    uchar	xverter_mode;		// TRUE if transverter mode active
    ulong	xverter_offset;		// frequency offset for transverter (added to frequency display)

    bool	refresh_freq_disp;		// TRUE if frequency display display is to be refreshed
    //
    // Calibration factors for output power, in percent (100 = 1.00)
    //
#define ADJ_5W 0
#define ADJ_FULL_POWER 1
    uchar	pwr_adj[2][MAX_BAND_NUM];
    //
    ulong	alc_decay;					// adjustable ALC release time - EEPROM read/write version
    ulong	alc_decay_var;				// adjustable ALC release time - working variable version
    ulong	alc_tx_postfilt_gain;		// amount of gain after the TX audio filtering - EEPROM read/write version
    ulong	alc_tx_postfilt_gain_var;	// amount of gain after the TX audio filtering - working variable version
    //
#define FREQ_STEP_SWAP_BTN	0xf0
    uchar	freq_step_config;			// configuration of step size (line, step button reversal) - setting any of the 4 upper bits -> step button switch, any of the lower bits -> frequency marker display enabled
    //
    bool	nb_disable;					// TRUE if noise blanker is to be disabled
    //

#define DSP_NR_ENABLE 	  0x01
#define DSP_NR_POSTAGC_ENABLE 	  0x02
#define DSP_NOTCH_ENABLE 0x04
#define DSP_NB_ENABLE 0x08


    uchar	dsp_active;					// Used to hold various aspects of DSP mode selection
    // LSB = 1 if DSP NR mode is on (| 1)
    // LSB+1 = 1 if DSP NR is to occur post AGC (| 2)
    // LSB+2 = 1 if DSP Notch mode is on (| 4)
    // LSB+3 = 0 if DSP is to be displayed on screen instead of NB (| 8)
    // MSB	 = 1 if button G2 toggle NOT initialized (| 128)
    uchar	dsp_mode;					// holds the mode chosen in the DSP
    uchar 	digital_mode;				// holds actual digital mode
    uchar	dsp_active_toggle;			// holder used on the press-hold of button G2 to "remember" the previous setting
    uchar	dsp_nr_strength;			// "Strength" of DSP Noise reduction - to be converted to "Mu" factor
    ulong	dsp_nr_delaybuf_len;		// size of DSP noise reduction delay buffer
    uchar	dsp_nr_numtaps;				// Number of FFT taps on the DSP Noise reduction
    uchar	dsp_notch_numtaps;
    uchar	dsp_notch_mu;				// mu adjust of notch DSP LMS
    uint8_t	dsp_notch_delaybuf_len;		// size of DSP notch delay buffer
    bool	dsp_inhibit;				// if TRUE, DSP (NR, Notch) functions are inhibited.  Used during power-up
    bool	dsp_inhibit_mute;			// holder for "dsp_inhibit" during muting operations to allow restoration of previous state
    bool	dsp_timed_mute;				// TRUE if DSP is to be muted for a timed amount
    ulong	dsp_inhibit_timing;			// used to time inhibiting of DSP when it must be turned off for some reason
    bool	reset_dsp_nr;				// TRUE if DSP NR coefficients are to be reset when "audio_driver_set_rx_audio_filter()" is called
    //
    uchar	lcd_backlight_brightness;	// LCD backlight brightness, 0-3:  0 = full, 3 = dimmest

#define LCD_BLANKING_ENABLE 0x80
#define LCD_BLANKING_TIMEMASK 0x0f
    uchar	lcd_backlight_blanking;		// for controlling backlight auto-off control
    //
    uchar	tune_step;					// Used for press-and-hold tune step adjustment
    ulong	tune_step_idx_holder;		// used to hold the original step size index during the press-and-hold
    //
    bool	frequency_lock;				// TRUE if frequency knob is locked
    //
#define TX_DISABLE_ALWAYS       1
#define TX_DISABLE_USER         2
#define TX_DISABLE_OUTOFRANGE	4
    uchar	tx_disable;					// TRUE if transmit is to be disabled


    uint16_t	flags1;					// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS1"
#define FLAGS1_TX_AUTOSWITCH_UI_DISABLE 0x01
#define FLAGS1_SWAP_BAND_BTN			0x02
#define FLAGS1_MUTE_LINEOUT_TX			0x04
#define FLAGS1_AM_TX_FILTER_DISABLE		0x08
#define FLAGS1_SWAP_FWDREV_SENSE		0x10
#define FLAGS1_FREQ_LIMIT_RELAX			0x20
#define FLAGS1_SSB_TX_FILTER_DISABLE	0x40
#define FLAGS1_WFALL_SCOPE_TOGGLE		0x80
#define FLAGS1_CAT_MODE_ACTIVE			0x100
#define FLAGS1_DYN_TUNE_ENABLE			0x200
#define FLAGS1_SAM_ENABLE				0x400
#define FLAGS1_CAT_IN_SANDBOX			0x800
#define FLAGS1_SPECTRUM_LIGHT_ENABLE	0x1000
    // LSB   = 0 if on-screen AFG/(STG/CMP) and WPM/(MIC/LIN) indicators are changed on TX
    // LSB+1 = 1 if BAND-/BAND+ buttons are to be swapped in their positions
    // LSB+2 = 1 if TX audio output from LINE OUT is to be muted during transmit (audio output only enabled when translate mode is DISABLED
    // LSB+3 = 1 if AM TX has transmit filter DISABLED
    // LSB+4 = 1 if FWD/REV A/D inputs from RF power detectors are to be reversed
    // LSB+5 = 1 if Frequency tuning is to be relaxed
    // LSB+6 = 1 if SSB TX has transmit filter DISABLED
    // LSB+7 = 0 = Spectrum Scope (analyzer), 1 = Waterfall display
    // LSB+8 = 0 = CAT is disabled, 1 = CAT is enabled
    // LSB+9 = 0 = dynamic tune is disabled, 1 = dynamic tune is enabled
    // LSB+10 = 0 = SAM mode is disabled, 1 = SAM mode is enabled
    // LSB+11 = 0 = CAT works on band storage, 1 = CAT works in sandbox
    // LSB+12 = 0 = Spectrum normal, 1 = Spectrum light

    uint16_t	flags2;						// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS2"
#define FLAGS2_FM_MODE_ENABLE 			0x01
#define FLAGS2_FM_MODE_DEVIATION_5KHZ 	0x02
#define FLAGS2_KEY_BEEP_ENABLE 			0x04
#define FLAGS2_LOW_BAND_BIAS_REDUCE 	0x08
#define FLAGS2_FREQ_MEM_LIMIT_RELAX 	0x10
    // LSB   = 0 if FM mode is DISABLED, 1 if FM mode is ENABLED
    // LSB+1 = 0 if 2.5 kHz FM deviation, 1 for 5 kHz FM deviation
    // LSB+2 = 1 if key/button beep is enabled
    // LSB+3 = 1 if bias values for lower bands  below 8Mhz have lower influence factor
    // LSB+4 = 1 if memory-save versus frequency restrictions are to be relaxed
    ulong	sysclock;				// This counts up from zero when the unit is powered up at precisely 100 Hz over the long term.  This
    // is NEVER reset and is used for timing certain events.
    uint16_t	version_number_minor;		// version number - minor - used to hold version number and detect change
    uint16_t	version_number_build;		// version number - build - used to hold version number and detect change
    uint16_t	version_number_release;		// version number - release - used to hold version number and detect change
    uchar	nb_agc_time_const;			// used to calculate the AGC time constant
    uchar	cw_offset_mode;				// CW offset mode (USB, LSB, etc.)
    bool	cw_lsb;					// flag used to indicate that CW is to operate in LSB when TRUE
    uchar	iq_freq_mode;				// used to set/configure the I/Q frequency/conversion mode
    uchar	lsb_usb_auto_select;			// holds setting of LSB/USB auto-select above/below 10 MHz
    bool	conv_sine_flag;				// FALSE until the sine tables for the frequency conversion have been built (normally zero, force 0 to rebuild)
    ulong	last_tuning;				// this is a timer used to prevent too fast tuning per second
    ulong	lcd_blanking_time;			// this holds the system time after which the LCD is blanked - if blanking is enabled
    bool	lcd_blanking_flag;			// if TRUE, the LCD is blanked completely (e.g. backlight is off)
    bool	freq_cal_adjust_flag;			// set TRUE if frequency calibration is in process
    bool	xvtr_adjust_flag;			// set TRUE if transverter offset adjustment is in process
    bool	rx_muting;				// set TRUE if audio output is to be muted
    ulong	rx_blanking_time;			// this is a timer used to delay the un-blanking of the audio after a large synthesizer tuning step

#define VFO_MEM_MODE_SPLIT 0x80
#define VFO_MEM_MODE_VFO_B 0x40
    ulong	vfo_mem_mode;				// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
    // LSB+6 (0x40):  0 = VFO A, 1 = VFO B
    // LSB+7 (0x80): 0 = normal mode, 1 = Split mode (e.g. LSB=0:  RX=A, TX=B;  LSB=1:  RX=B, TX=A)
    ulong	voltmeter_calibrate;			// used to calibrate the voltmeter
    bool	thread_timer;				// used to trigger the thread timing (e.g. "driver_thread()")
    uchar	waterfall_color_scheme;			// stores waterfall color scheme
    uchar	waterfall_vert_step_size;		// vertical step size in waterfall mode
    ulong	waterfall_offset;			// offset for waterfall display
    ulong	waterfall_contrast;			// contrast setting for waterfall display
    uchar	spectrum_scope_scheduler;		// timer for scheduling the next update of the spectrum scope update, updated at DMA rate
    uchar	spectrum_scope_nosig_adjust;		// Adjustment for no signal adjustment conditions for spectrum scope
    uchar	waterfall_nosig_adjust;			// Adjustment for no signal adjustment conditions for waterfall
    uchar	waterfall_size;				// size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved
    uchar	fft_window_type;			// type of windowing function applied to scope/waterfall.  At the moment, only lower 4 bits are used - upper 4 bits are reserved
    bool	dvmode;					// TRUE if alternate (stripped-down) RX and TX functions (USB-only) are to be used
    uchar	tx_audio_muting_timing;			// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
    ulong	tx_audio_muting_timer;			// timer value used for muting TX audio when keying PTT to suppress "click" or "thump"
    uchar	filter_disp_colour;			// used to hold the current color of the line that indicates the filter passband/bandwidth
    bool	tx_audio_muting_flag;			// when TRUE, audio is to be muted after PTT/keyup
    bool	vfo_mem_flag;				// when TRUE, memory mode is enabled
    bool	mem_disp;				// when TRUE, memory display is enabled
    bool	load_eeprom_defaults;			// when TRUE, load EEPROM defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!
    ulong	fm_subaudible_tone_gen_select;		// lookup ("tone number") used to index the table tone generation (0 corresponds to "tone disabled")
    uchar	fm_tone_burst_mode;			// this is the setting for the tone burst generator
    ulong	fm_tone_burst_timing;			// this is used to time/schedule the duration of a tone burst
    uchar	fm_sql_threshold;			// squelch threshold "dial" setting
//	uchar	fm_rx_bandwidth;			// bandwidth setting for FM reception
    ulong	fm_subaudible_tone_det_select;		// lookup ("tone number") used to index the table for tone detection (0 corresponds to "disabled")
    bool	beep_active;				// TRUE if beep is active
    ulong	beep_frequency;				// beep frequency, in Hz
    ulong	beep_timing;				// used to time/schedule the duration of a keyboard beep
    uchar	beep_loudness;				// loudness of the keyboard/CW sidetone test beep
    bool	load_freq_mode_defaults;		// when TRUE, load frequency/mode defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!
    bool	boot_halt_flag;				// when TRUE, boot-up is halted - used to allow various test functions
    bool	mic_bias;				// TRUE = mic bias on
    uchar	ser_eeprom_type;			// serial eeprom type
    uchar	ser_eeprom_in_use;			// 0xFF = not in use, 0x1 = in use
    uint8_t* eeprombuf;				// pointer to copy of config in RAM
    uchar 	tp_present;				// touchscreen present = 1, absent = 0
    char 	tp_x;					// touchscreen x coordinate
    char	tp_y;					// touchscreen y coordinate
    uchar	tp_state;				// touchscreen state machine
    bool	show_tp_coordinates;	// show coordinates on LCD
    uchar	rfmod_present;			// 0 = not present
    uchar	vhfuhfmod_present;		// 0 = not present
    uchar	multi;					// actual translate factor
    uchar	tune_power_level;		// TX power in antenna tuning function
    uchar	power_temp;				// temporary tx power if tune is different from actual tx power
    uchar	cat_band_index;			// buffered bandindex before first CAT command arrived
    bool 	notch_enabled;			// notch_filter enabled
    uchar	xlat;					// CAT <> IQ-Audio
    ulong	notch_frequency;		// frequency of the manual notch filter
    bool 	peak_enabled;			// indicates whether peak filter is enabled or not
    ulong	peak_frequency;			// frequency of the manual peak filter
    int		bass_gain;				// gain of the low shelf EQ filter
    int		treble_gain;			// gain of the high shelf EQ filter

    uint8_t display_type;           // existence/identification of display type
    uint32_t audio_int_counter;		// used for encoder timing - test DL2FW
    unsigned short DeviceCode;		// LCD ident code
    bool USE_NEW_PHASE_CORRECTION; 	// used to test new phase correction
    bool encoder3state;
    int bc_band;
    uchar c_line;					// position of center line
    Si570_ResultCodes last_lo_result;			// used in dynamic tuning to hold frequency color
} TransceiverState;
//
extern __IO TransceiverState ts;

#define	POWERDOWN_DELAY_COUNT	30	// Delay in main service loop for the "last second" before power-down - to allow EEPROM write to complete

//#define CODEC_USE_SPI

#define DEBUG_COM                        USART1

#define non_os_delay()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 1000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

#define non_os_delay_a()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 10000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

// ------------------------------------------------------------------
// Exports

void mchf_board_green_led(int state);

void mchf_board_power_off(void);

void mchf_board_init(void);
void mchf_board_post_init(void);

uint16_t Read_EEPROM(uint16_t addr, uint16_t *value);
uint16_t Write_EEPROM(uint16_t addr, uint16_t value);
uint16_t Read_SerEEPROM(uint16_t addr, uint16_t *value);
uint16_t Write_SerEEPROM(uint16_t addr, uint16_t value);
void copy_virt2ser(void);
void copy_ser2virt(void);
void verify_servirt(void);
void mchf_reboot();

// in main.c
void CriticalError(ulong error);

bool is_vfo_b();

#endif
