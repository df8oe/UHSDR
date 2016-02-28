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
//
//
//
// -----------------------------------------------------------------------------
#define		DEVICE_STRING			"mcHF QRP Transceiver"
#define 	AUTHOR_STRING   		"K. Atanassov - M0NKA 2014-2016"
//
#define 	TRX4M_VER_MAJOR			0
#define 	TRX4M_VER_MINOR			219
#define 	TRX4M_VER_RELEASE		27
//
#define 	TRX4M_VER_BUILD			4
//

#define		ATTRIB_STRING1			"Additional Contributions by"
#define		ATTRIB_STRING2			"KA7OEI, DF8OE and others."
#define		ATTRIB_STRING3			"Licensed under CC BY-NC-SA 3.0"
//
// -----------------------------------------------------------------------------
//#define 	DEBUG_BUILD

#define USB_AUDIO_SUPPORT // uncomment this to get experimental USB AUDIO Support


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
#define DEMOD_DIGI			5
#define DEMOD_MAX_MODE			5

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

#define	KHZ_MULT			4000	// multiplier to convert oscillator frequency or band size to display kHz, used below
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
#define ENC_ONE_MAX_MODE		1
//
// encoder two
#define ENC_TWO_MODE_RF_GAIN		0
#define ENC_TWO_MODE_SIG_PROC		1
#define ENC_TWO_MAX_MODE		2
//
// encoder three
#define ENC_THREE_MODE_RIT		0
#define ENC_THREE_MODE_CW_SPEED		1
#define ENC_THREE_MAX_MODE		2
//
// Audio filter select enumeration
//
enum	{
	AUDIO_300HZ = 0,
	AUDIO_500HZ,
	AUDIO_1P8KHZ,
	AUDIO_2P3KHZ,
	AUDIO_2P7KHZ,
	AUDIO_2P9KHZ,
	AUDIO_3P6KHZ,
	AUDIO_WIDE
};
//
//
#define	AUDIO_DEFAULT_FILTER		AUDIO_2P3KHZ
//
// use below to define the lowest-used filter number
//
#define AUDIO_MIN_FILTER		0
//
// use below to define the highest-used filter number-1
//
#define AUDIO_MAX_FILTER		8
//
//
#define MIN_FILTER_SELECT_VAL		1		// Minimum value for selection of sub-filter
//
#define	MAX_300HZ_FILTER		9		// Highest number selection of 500 Hz filter
#define	FILTER_300HZ_DEFAULT		6		// Center frequency of 750 Hz
//
#define	MAX_500HZ_FILTER		5
#define	FILTER_500HZ_DEFAULT		3		// Center frequency of 750 Hz
//
#define	MAX_1K8_FILTER			6
#define	FILTER_1K8_DEFAULT		3		// Center frequency of 1425 Hz
//
#define	MAX_2K3_FILTER			5
#define	FILTER_2K3_DEFAULT		2		// Center frequency of 1412 Hz
//
#define	MAX_2K7_FILTER			2
#define	FILTER_2K7_DEFAULT		2		// Center frequency of 1412 Hz
//
#define	MAX_2K9_FILTER			2
#define	FILTER_2K9_DEFAULT		2		// Center frequency of 1412 Hz
//
#define	MAX_3K6_FILTER			2		// only on/off
#define	FILTER_3K6_DEFAULT		2		// 1 = Enabled
//
enum	{
	WIDE_FILTER_10K_AM = 0,
	WIDE_FILTER_7K5_AM,
	WIDE_FILTER_6K_AM,
	WIDE_FILTER_5K_AM,
	WIDE_FILTER_10K,
	WIDE_FILTER_7K5,
	WIDE_FILTER_6K,
	WIDE_FILTER_5K,
	WIDE_FILTER_MAX
};
//
//
#define	FILTER_WIDE_DEFAULT		WIDE_FILTER_10K		// 10k selected by default
//
//
// Define visual widths of audio filters for on-screen indicator in Hz
//
#define	FILTER_300HZ_WIDTH		300
#define	FILTER_500HZ_WIDTH		500
#define	FILTER_1800HZ_WIDTH		1800
#define FILTER_2300HZ_WIDTH		2300
#define FILTER_2700HZ_WIDTH		2700
#define FILTER_2900HZ_WIDTH		2900
#define FILTER_3600HZ_WIDTH		3600
#define	FILTER_5000HZ_WIDTH		5000
#define	FILTER_6000HZ_WIDTH		6000
#define FILTER_7500HZ_WIDTH		7500
#define	FILTER_10000HZ_WIDTH		10000
//
#define	HILBERT_3600HZ_WIDTH		3800	// Approximate bandwidth of 3.6 kHz wide Hilbert - This used to depict FM detection bandwidth
//
#define	FILT300_1			500
#define	FILT300_2			550
#define	FILT300_3			600
#define	FILT300_4			650
#define	FILT300_5			700
#define	FILT300_6			750
#define	FILT300_7			800
#define	FILT300_8			850
#define	FILT300_9			900
//
#define	FILT500_1			550
#define	FILT500_2			650
#define	FILT500_3			750
#define	FILT500_4			850
#define	FILT500_5			950
//
#define	FILT1800_1			1125
#define	FILT1800_2			1275
#define	FILT1800_3			1427
#define	FILT1800_4			1575
#define	FILT1800_5			1725
#define	FILT1800_6			 900
//
#define	FILT2300_1			1262
#define	FILT2300_2			1412
#define	FILT2300_3			1562
#define	FILT2300_4			1712
#define	FILT2300_5			1150
//
#define FILT2700_1			1350
#define	FILT2700_2			1425
//
#define FILT2900_1			1450
#define	FILT2900_2			1525
//
#define FILT3600_1			1800
#define	FILT3600_2			1875
#define	FILT3600			1800
//
#define	FILT5000			2500
//
#define	FILT6000			3000
//
#define	FILT7500			3750
//
#define	FILT10000			5000
//
#define	HILBERT3600			1900	// "width" of "3.6 kHz" Hilbert filter - This used to depict FM detection bandwidth
//
#define	FILT_DISPLAY_WIDTH		256		// width, in pixels, of the spectral display on the screen - this value used to calculate Hz/pixel for indicating width of filter
//
//
#define CW_MODE_IAM_B			0
#define CW_MODE_IAM_A			1
#define CW_MODE_STRAIGHT		2
#define CW_MAX_MODE			3

// PA power level setting enumeration
enum {
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
enum {
	SPEC_WHITE = 0,
	SPEC_GREY,
	SPEC_BLUE,
	SPEC_RED,
	SPEC_MAGENTA,
	SPEC_GREEN,
	SPEC_CYAN,
	SPEC_YELLOW,
	SPEC_ORANGE,
	SPEC_BLACK,
	SPEC_GREY2,
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
enum {
	METER_SWR = 0,
	METER_AUDIO,
	METER_ALC,
	METER_MAX,
};
//
#define	BACKLIGHT_BLANK_TIMING_DEFAULT	8		// default number of SECONDS for backlight blanking
#define LCD_STARTUP_BLANKING_TIME	3000		// number of DECISECONDS (e.g. SECONDS * 100) after power-up before LCD blanking occurs if no buttons are pressed/knobs turned

#define MAX_VAR_ADDR 383
//
// *************************************************************************************************************************
//
// Eeprom items IDs - if updating, make sure eeprom.h list
// is updated as well!!!
//
// These do NOT use "enum" as it is important that the number *NOT* change by the ineration of new variables:  All NEW variable should be placed at the END of the
// list to maintain compatibility with older versions and the settings!
//
#define EEPROM_ZERO_LOC_UNRELIABLE	0		// DO NOT USE LOCATION ZERO AS IT MAY BE UNRELIABLE!!!!
#define EEPROM_BAND_MODE		1
#define EEPROM_FREQ_HIGH		2
#define EEPROM_FREQ_LOW			3
#define EEPROM_FREQ_STEP		4
#define EEPROM_TX_AUDIO_SRC		5
#define EEPROM_TCXO_STATE		6
#define EEPROM_PA_BIAS			7
#define EEPROM_AUDIO_GAIN		8
#define EEPROM_RX_CODEC_GAIN		9
#define EEPROM_MAX_VOLUME		10
#define EEPROM_POWER_STATE		11
#define EEPROM_TX_POWER_LEVEL		12
#define EEPROM_KEYER_SPEED		13
#define EEPROM_KEYER_MODE		14
#define EEPROM_SIDETONE_GAIN		15
#define EEPROM_MIC_BOOST		16
#define	EEPROM_TX_IQ_LSB_GAIN_BALANCE	17		// TX gain balance
#define	EEPROM_TX_IQ_LSB_PHASE_BALANCE	18		// TX phase balance
#define	EEPROM_RX_IQ_LSB_GAIN_BALANCE	19
#define	EEPROM_RX_IQ_LSB_PHASE_BALANCE	20
//
#define	EEPROM_BAND0_MODE		21		// Band/mode/filter memory per-band - bands indexed from here
#define	EEPROM_BAND1_MODE		22
#define	EEPROM_BAND2_MODE		23
#define	EEPROM_BAND3_MODE		24
#define	EEPROM_BAND4_MODE		25
#define	EEPROM_BAND5_MODE		26
#define	EEPROM_BAND6_MODE		27
#define	EEPROM_BAND7_MODE		28
#define	EEPROM_BAND8_MODE		29
#define	EEPROM_BAND9_MODE		30
#define	EEPROM_BAND10_MODE		31
#define	EEPROM_BAND11_MODE		32
#define	EEPROM_BAND12_MODE		33
#define	EEPROM_BAND13_MODE		34
#define	EEPROM_BAND14_MODE		35
#define	EEPROM_BAND15_MODE		36
#define	EEPROM_BAND16_MODE		37
#define	EEPROM_BAND17_MODE		38		// "Floating" General coverage band
//
//
#define	EEPROM_BAND0_FREQ_HIGH		39		// Per-band frequency, high word - bands indexed from here
#define	EEPROM_BAND1_FREQ_HIGH		40
#define	EEPROM_BAND2_FREQ_HIGH		41
#define	EEPROM_BAND3_FREQ_HIGH		42
#define	EEPROM_BAND4_FREQ_HIGH		43
#define	EEPROM_BAND5_FREQ_HIGH		44
#define	EEPROM_BAND6_FREQ_HIGH		45
#define	EEPROM_BAND7_FREQ_HIGH		46
#define	EEPROM_BAND8_FREQ_HIGH		47
#define	EEPROM_BAND9_FREQ_HIGH		48
#define	EEPROM_BAND10_FREQ_HIGH		49
#define	EEPROM_BAND11_FREQ_HIGH		50
#define	EEPROM_BAND12_FREQ_HIGH		51
#define	EEPROM_BAND13_FREQ_HIGH		52
#define	EEPROM_BAND14_FREQ_HIGH		53
#define	EEPROM_BAND15_FREQ_HIGH		54
#define	EEPROM_BAND16_FREQ_HIGH		55
#define	EEPROM_BAND17_FREQ_HIGH		56		// "Floating" General coverage band
//
//
#define	EEPROM_BAND0_FREQ_LOW		57		// Per-band frequency, high word - bands indexed from here
#define	EEPROM_BAND1_FREQ_LOW		58
#define	EEPROM_BAND2_FREQ_LOW		59
#define	EEPROM_BAND3_FREQ_LOW		60
#define	EEPROM_BAND4_FREQ_LOW		61
#define	EEPROM_BAND5_FREQ_LOW		62
#define	EEPROM_BAND6_FREQ_LOW		63
#define	EEPROM_BAND7_FREQ_LOW		64
#define	EEPROM_BAND8_FREQ_LOW		65
#define	EEPROM_BAND9_FREQ_LOW		66
#define	EEPROM_BAND10_FREQ_LOW		67
#define	EEPROM_BAND11_FREQ_LOW		68
#define	EEPROM_BAND12_FREQ_LOW		69
#define	EEPROM_BAND13_FREQ_LOW		70
#define	EEPROM_BAND14_FREQ_LOW		71
#define	EEPROM_BAND15_FREQ_LOW		72
#define	EEPROM_BAND16_FREQ_LOW		73
#define	EEPROM_BAND17_FREQ_LOW		74		// "Floating" General coverage band
//
//
#define	EEPROM_FREQ_CAL			75		// Frequency calibration
#define	EEPROM_NB_SETTING		76		// Noise Blanker Setting
#define	EEPROM_AGC_MODE			77		// AGC setting
#define	EEPROM_MIC_GAIN			78		// Mic gain setting
#define	EEPROM_LINE_GAIN		79		// Line gain setting
#define	EEPROM_SIDETONE_FREQ		80		// Sidetone frequency (Hz)
#define	EEPROM_SPEC_SCOPE_SPEED		81		// Spectrum Scope Speed
#define	EEPROM_SPEC_SCOPE_FILTER	82		// Spectrum Scope filter strength
#define	EEPROM_RX_GAIN			83		// RX Gain setting (e.g. minimum RF gain as might be used for manual AGC)
#define	EEPROM_AGC_CUSTOM_DECAY		84		// Custom setting for AGC decay rate
#define	EEPROM_SPECTRUM_TRACE_COLOUR	85		// Custom setting for spectrum scope trace colour
#define	EEPROM_SPECTRUM_GRID_COLOUR	86		// Custom setting for spectrum scope grid colour
#define	EEPROM_SPECTRUM_SCALE_COLOUR	87		// Custom setting for spectrum scope frequency scale colour
#define	EEPROM_PADDLE_REVERSE		88		// TRUE if paddle is to be reversed
#define	EEPROM_CW_RX_DELAY		89		// Delay after last CW element before returning to receive
#define	EEPROM_SPECTRUM_CENTRE_GRID_COLOUR	90	// Custom setting for spectrum scope grid center marker colour
//
#define	EEPROM_DETECTOR_COUPLING_COEFF_2200M	91	// Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define	EEPROM_DETECTOR_COUPLING_COEFF_630M	92	// Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define	EEPROM_DETECTOR_COUPLING_COEFF_80M	93	// Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define	EEPROM_DETECTOR_COUPLING_COEFF_40M	94	// Calibration coupling coefficient for FWD/REV power sensor for 60/40 meters
#define	EEPROM_DETECTOR_COUPLING_COEFF_20M	95	// Calibration coupling coefficient for FWD/REV power sensor for 30/20/17 meters
#define	EEPROM_DETECTOR_COUPLING_COEFF_15M	96	// Calibration coupling coefficient for FWD/REV power sensor for 15/12/10 meters
//
// The following are the coefficients used to set the RF output power settings
//
#define	EEPROM_BAND0_5W			97		// 5 watt power setting, 80m
#define	EEPROM_BAND1_5W			98		// 5 watt power setting, 60m
#define	EEPROM_BAND2_5W			99		// 5 watt power setting, 40m
#define	EEPROM_BAND3_5W			100		// 5 watt power setting, 30m
#define	EEPROM_BAND4_5W			101		// 5 watt power setting, 20m
#define	EEPROM_BAND5_5W			102		// 5 watt power setting, 17m
#define	EEPROM_BAND6_5W			103		// 5 watt power setting, 15m
#define	EEPROM_BAND7_5W			104		// 5 watt power setting, 12m
#define	EEPROM_BAND8_5W			105		// 5 watt power setting, 10m
#define	EEPROM_BAND9_5W			106		// 5 watt power setting, 6m
#define	EEPROM_BAND10_5W		107		// 5 watt power setting, 4m
#define	EEPROM_BAND11_5W		108		// 5 watt power setting, 2m
#define	EEPROM_BAND12_5W		109		// 5 watt power setting, 70cm
#define	EEPROM_BAND13_5W		110		// 5 watt power setting, 23cm
#define	EEPROM_BAND14_5W		111		// 5 watt power setting, 2200m
#define	EEPROM_BAND15_5W		112		// 5 watt power setting, 630m
#define	EEPROM_BAND16_5W		113		// 5 watt power setting, 160m
#define	EEPROM_BAND17_5W		114		// reserved
//
#define	EEPROM_BAND0_FULL		115		// "FULL" power setting, 80m
#define	EEPROM_BAND1_FULL		116		// "FULL" power setting, 60m
#define	EEPROM_BAND2_FULL		117		// "FULL" power setting, 40m
#define	EEPROM_BAND3_FULL		118		// "FULL" power setting, 30m
#define	EEPROM_BAND4_FULL		119		// "FULL" power setting, 20m
#define	EEPROM_BAND5_FULL		120		// "FULL" power setting, 17m
#define	EEPROM_BAND6_FULL		121		// "FULL" power setting, 15m
#define	EEPROM_BAND7_FULL		122		// "FULL" power setting, 12m
#define	EEPROM_BAND8_FULL		123		// "FULL" power setting, 10m
#define	EEPROM_BAND9_FULL		124		// "FULL" power setting, 6m
#define	EEPROM_BAND10_FULL		125		// "FULL" power setting, 4m
#define	EEPROM_BAND11_FULL		126		// "FULL" power setting, 2m
#define	EEPROM_BAND12_FULL		127		// "FULL" power setting, 70cm
#define	EEPROM_BAND13_FULL		128		// "FULL" power setting, 23cm
#define	EEPROM_BAND14_FULL		129		// "FULL" power setting, 2200m
#define	EEPROM_BAND15_FULL		130		// "FULL" power setting, 630m
#define	EEPROM_BAND16_FULL		131		// "FULL" power setting, 160m
#define	EEPROM_BAND17_FULL		132		// reserved
//
#define	EEPROM_FILTER_300HZ_SEL		133		// Selection of 300 Hz filter
#define EEPROM_FILTER_500HZ_SEL		134		// Selection of 500 Hz filter
#define	EEPROM_FILTER_1K8_SEL		135		// Selection of 1.8 kHz filter
#define	EEPROM_FILTER_2K3_SEL		136		// Selection of 2.3 kHz filter
#define EEPROM_FILTER_3K6_SEL		137		// Selection of 3.6 kHz filter
#define	EEPROM_FILTER_WIDE_SEL		138		// Selection of "Wide" filter (>3.6kHz)
//
#define	EEPROM_TX_IQ_USB_GAIN_BALANCE	139		// TX gain balance
#define	EEPROM_TX_IQ_USB_PHASE_BALANCE	140		// TX phase balance
#define	EEPROM_RX_IQ_USB_GAIN_BALANCE	141
#define	EEPROM_RX_IQ_USB_PHASE_BALANCE	142
#define	EEPROM_SENSOR_NULL		143		// Power meter sensor null calibrate
//#define	EEPROM_REV_PWR_CAL	144		// REV power meter calibrate
//
#define	EEPROM_XVERTER_DISP		145		// TRUE if display is offset with transverter frequency offset
#define	EEPROM_XVERTER_OFFSET_HIGH	146		// Frequency by which the display is offset for transverter use, high byte
//
#define	EEPROM_VFO_MEM_MODE		147		// settings of VFO/SPLIT/Memory configuration bits - see variable "vfo_mem_mode" for information.
//
#define	EEPROM_XVERTER_OFFSET_LOW	148		// Low byte of above
//
#define EEPROM_SPECTRUM_MAGNIFY		149		// TRUE if spectrum scope is to be magnified
//
#define	EEPROM_WIDE_FILT_CW_DISABLE	150		// TRUE if wide filters are to be disabled in CW mode
#define	EEPROM_NARROW_FILT_SSB_DISABLE	151		// TRUE if narrow filters are to be disabled in SSB mode
//
#define	EEPROM_AM_MODE_DISABLE		152		// TRUE if AM mode is to be disabled
//
#define EEPROM_PA_CW_BIAS		153		// If non-zero, this is the PA bias setting when in CW mode
//
#define	EEPROM_SPECTRUM_DB_DIV		154		// Spectrum Scope dB/Division
#define	EEPROM_SPECTRUM_AGC_RATE	155		// AGC setting for spectrum scope
//
#define	EEPROM_METER_MODE		156		// Stored setting of meter mode
//
#define	EEPROM_ALC_DECAY_TIME		157		// ALC Decay time
#define	EEPROM_ALC_POSTFILT_TX_GAIN	158		// ALC post-filter TX audio gain
//
#define	EEPROM_STEP_SIZE_CONFIG		159		// TRUE if there is to be a line under the frequency digit indicating step size
//
#define	EEPROM_DSP_MODE			160		// Stores the DSP operational mode
#define	EEPROM_DSP_NR_STRENGTH		161		// Stores the DSP Noise Reduction operational strength
#define	EEPROM_DSP_NR_DECOR_BUFLEN	162		// DSP Noise Reduction De-correlator buffer length
#define EEPROM_DSP_NR_FFT_NUMTAPS	163		// DSP Noise Reduction FFT number of taps
#define	EEPROM_DSP_NOTCH_DECOR_BUFLEN	164		// DSP Notch De-correlator buffer length
#define	EEPROM_DSP_NOTCH_CONV_RATE	165		// DSP Notch convergence rate
//
#define EEPROM_MAX_RX_GAIN		166		// Maximum RX gain - adjusts maximum allowed AGC gain in S-units
#define	EEPROM_TX_AUDIO_COMPRESS	167		// TX audio compressor setting, used to calculate other values
//
#define	EEPROM_RX_IQ_AM_GAIN_BALANCE	168		// IQ Gain balance for AM reception
//
#define	EEPROM_TX_DISABLE		169		// TRUE of transmit is to be disabled
#define	EEPROM_MISC_FLAGS1		170		// Miscellaneous status flag, saved in EEPROM - see variable "misc_flags1"
#define	EEPROM_VERSION_NUMBER		171		// Storage of current version release - used to detect change of firmware
#define	EEPROM_NB_AGC_TIME_CONST	172		// Noise blanker AGC time constant setting
#define	EEPROM_CW_OFFSET_MODE		173		// CW Offset mode
#define	EEPROM_FREQ_CONV_MODE		174		// Frequency Conversion Mode (e.g. I/Q frequency conversion done in receive/transmit to offset from zero)
#define	EEPROM_LSB_USB_AUTO_SELECT	175		// Auto selection of LSB/USB above/below 10 MHz (including 60 meters)
#define	EEPROM_VERSION_BUILD		176		// Storage of current version build number - used to detect change of firmware
#define	EEPROM_LCD_BLANKING_CONFIG	177		// Configuration of automatic LCD blanking mode settings
#define	EEPROM_VOLTMETER_CALIBRATE	178		// Holder for calibration of the on-screen voltmeter
#define	EEPROM_WATERFALL_COLOR_SCHEME	179		// Color scheme for waterfall display
#define	EEPROM_WATERFALL_VERTICAL_STEP_SIZE	180	// Number of vertical steps of waterfall per iteration
#define	EEPROM_WATERFALL_OFFSET		181		// Palette offset for waterfall
#define	EEPROM_WATERFALL_CONTRAST	182		// Palette contrast multiplier for waterfall
//
// VFO A storage
//
#define	EEPROM_BAND0_MODE_A		183		// Band/mode/filter memory per-band - bands indexed from here
#define	EEPROM_BAND1_MODE_A		184
#define	EEPROM_BAND2_MODE_A		185
#define	EEPROM_BAND3_MODE_A		186
#define	EEPROM_BAND4_MODE_A		187
#define	EEPROM_BAND5_MODE_A		188
#define	EEPROM_BAND6_MODE_A		189
#define	EEPROM_BAND7_MODE_A		190
#define	EEPROM_BAND8_MODE_A		191
#define	EEPROM_BAND9_MODE_A		192
#define	EEPROM_BAND10_MODE_A		193
#define	EEPROM_BAND11_MODE_A		194
#define	EEPROM_BAND12_MODE_A		195
#define	EEPROM_BAND13_MODE_A		196
#define	EEPROM_BAND14_MODE_A		197
#define	EEPROM_BAND15_MODE_A		198
#define	EEPROM_BAND16_MODE_A		199
#define	EEPROM_BAND17_MODE_A		200		// "Floating" General coverage band
//
#define	EEPROM_BAND0_FREQ_HIGH_A	201		// Per-band frequency, high word - bands indexed from here
#define	EEPROM_BAND1_FREQ_HIGH_A	202
#define	EEPROM_BAND2_FREQ_HIGH_A	203
#define	EEPROM_BAND3_FREQ_HIGH_A	204
#define	EEPROM_BAND4_FREQ_HIGH_A	205
#define	EEPROM_BAND5_FREQ_HIGH_A	206
#define	EEPROM_BAND6_FREQ_HIGH_A	207
#define	EEPROM_BAND7_FREQ_HIGH_A	208
#define	EEPROM_BAND8_FREQ_HIGH_A	209
#define	EEPROM_BAND9_FREQ_HIGH_A	210
#define	EEPROM_BAND10_FREQ_HIGH_A	211
#define	EEPROM_BAND11_FREQ_HIGH_A	212
#define	EEPROM_BAND12_FREQ_HIGH_A	213
#define	EEPROM_BAND13_FREQ_HIGH_A	214
#define	EEPROM_BAND14_FREQ_HIGH_A	215
#define	EEPROM_BAND15_FREQ_HIGH_A	216
#define	EEPROM_BAND16_FREQ_HIGH_A	217
#define	EEPROM_BAND17_FREQ_HIGH_A	218		// "Floating" General coverage band
//
#define	EEPROM_BAND0_FREQ_LOW_A		219		// Per-band frequency, high word - bands indexed from here
#define	EEPROM_BAND1_FREQ_LOW_A		220
#define	EEPROM_BAND2_FREQ_LOW_A		221
#define	EEPROM_BAND3_FREQ_LOW_A		222
#define	EEPROM_BAND4_FREQ_LOW_A		223
#define	EEPROM_BAND5_FREQ_LOW_A		224
#define	EEPROM_BAND6_FREQ_LOW_A		225
#define	EEPROM_BAND7_FREQ_LOW_A		226
#define	EEPROM_BAND8_FREQ_LOW_A		227
#define	EEPROM_BAND9_FREQ_LOW_A		228
#define	EEPROM_BAND10_FREQ_LOW_A	229
#define	EEPROM_BAND11_FREQ_LOW_A	230
#define	EEPROM_BAND12_FREQ_LOW_A	231
#define	EEPROM_BAND13_FREQ_LOW_A	232
#define	EEPROM_BAND14_FREQ_LOW_A	233
#define	EEPROM_BAND15_FREQ_LOW_A	234
#define	EEPROM_BAND16_FREQ_LOW_A	235
#define	EEPROM_BAND17_FREQ_LOW_A	236		// "Floating" General coverage band
//
// VFO B storage
//
#define	EEPROM_BAND0_MODE_B		237		// Band/mode/filter memory per-band - bands indexed from here
#define	EEPROM_BAND1_MODE_B		238
#define	EEPROM_BAND2_MODE_B		239
#define	EEPROM_BAND3_MODE_B		240
#define	EEPROM_BAND4_MODE_B		241
#define	EEPROM_BAND5_MODE_B		242
#define	EEPROM_BAND6_MODE_B		243
#define	EEPROM_BAND7_MODE_B		244
#define	EEPROM_BAND8_MODE_B		245
#define	EEPROM_BAND9_MODE_B		246
#define	EEPROM_BAND10_MODE_B		247
#define	EEPROM_BAND11_MODE_B		248
#define	EEPROM_BAND12_MODE_B		249
#define	EEPROM_BAND13_MODE_B		250
#define	EEPROM_BAND14_MODE_B		251
#define	EEPROM_BAND15_MODE_B		252
#define	EEPROM_BAND16_MODE_B		253
#define	EEPROM_BAND17_MODE_B		254		// "Floating" General coverage band
//
//
#define	EEPROM_BAND0_FREQ_HIGH_B	255		// Per-band frequency, high word - bands indexed from here
#define	EEPROM_BAND1_FREQ_HIGH_B	256
#define	EEPROM_BAND2_FREQ_HIGH_B	257
#define	EEPROM_BAND3_FREQ_HIGH_B	258
#define	EEPROM_BAND4_FREQ_HIGH_B	259
#define	EEPROM_BAND5_FREQ_HIGH_B	260
#define	EEPROM_BAND6_FREQ_HIGH_B	261
#define	EEPROM_BAND7_FREQ_HIGH_B	262
#define	EEPROM_BAND8_FREQ_HIGH_B	263
#define	EEPROM_BAND9_FREQ_HIGH_B	264
#define	EEPROM_BAND10_FREQ_HIGH_B	265
#define	EEPROM_BAND11_FREQ_HIGH_B	266
#define	EEPROM_BAND12_FREQ_HIGH_B	267
#define	EEPROM_BAND13_FREQ_HIGH_B	268
#define	EEPROM_BAND14_FREQ_HIGH_B	269
#define	EEPROM_BAND15_FREQ_HIGH_B	270
#define	EEPROM_BAND16_FREQ_HIGH_B	271
#define	EEPROM_BAND17_FREQ_HIGH_B	272		// "Floating" General coverage band
//
//
#define	EEPROM_BAND0_FREQ_LOW_B		273		// Per-band frequency, high word - bands indexed from here
#define	EEPROM_BAND1_FREQ_LOW_B		274
#define	EEPROM_BAND2_FREQ_LOW_B		275
#define	EEPROM_BAND3_FREQ_LOW_B		276
#define	EEPROM_BAND4_FREQ_LOW_B		277
#define	EEPROM_BAND5_FREQ_LOW_B		278
#define	EEPROM_BAND6_FREQ_LOW_B		279
#define	EEPROM_BAND7_FREQ_LOW_B		280
#define	EEPROM_BAND8_FREQ_LOW_B		281
#define	EEPROM_BAND9_FREQ_LOW_B		282
#define	EEPROM_BAND10_FREQ_LOW_B	283
#define	EEPROM_BAND11_FREQ_LOW_B	284
#define	EEPROM_BAND12_FREQ_LOW_B	285
#define	EEPROM_BAND13_FREQ_LOW_B	286
#define	EEPROM_BAND14_FREQ_LOW_B	287
#define	EEPROM_BAND15_FREQ_LOW_B	288
#define	EEPROM_BAND16_FREQ_LOW_B	289
#define	EEPROM_BAND17_FREQ_LOW_B	290		// "Floating" General coverage band
//
#define	EEPROM_WATERFALL_SPEED		291		// Spectrum Scope Speed
#define	EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST	292	// adjustment for no-signal conditions of spectrum scope
#define	EEPROM_WATERFALL_NOSIG_ADJUST	293		// adjustment for no-signal conditions of waterfall
#define EEPROM_DSP_NOTCH_FFT_NUMTAPS	294		// DSP Notch FFT number of taps
#define	EEPROM_WATERFALL_SIZE		295		// size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved
#define EEPROM_FFT_WINDOW		296		// FFT Window information (lower nybble currently used - upper nybble reserved)
#define	EEPROM_TX_PTT_AUDIO_MUTE	297		// timer used for muting TX audio when keying PTT to suppress "click" or "thump"
#define	EEPROM_MISC_FLAGS2		298		// Miscellaneous status flag, saved in EEPROM - see variable "misc_flags2"
#define	EEPROM_FILTER_DISP_COLOUR	299		// This contains the color of the line under the spectrum/waterfall display
#define	EEPROM_TX_IQ_AM_GAIN_BALANCE	300		// IQ Gain balance for AM transmission
#define	EEPROM_TX_IQ_FM_GAIN_BALANCE	301		// IQ Gain balance for FM transmission
#define	EEPROM_FM_SUBAUDIBLE_TONE_GEN	302		// index for storage of subaudible tone generation
#define	EEPROM_FM_TONE_BURST_MODE	303		// tone burst mode
#define EEPROM_FM_SQUELCH_SETTING	304		// FM squelch setting
#define EEPROM_FM_RX_BANDWIDTH		305		// bandwidth setting for FM reception
#define	EEPROM_RX_IQ_FM_GAIN_BALANCE	306		// IQ Gain balance for AM reception
#define	EEPROM_FM_SUBAUDIBLE_TONE_DET	307		// index for storage of subaudible tone detection
#define	EEPROM_KEYBOARD_BEEP_FREQ	308		// keyboard beep frequency (in Hz)
#define EEPROM_BEEP_LOUDNESS		309		// loudness of beep (keyboard, sidetone test)
#define	EEPROM_VERSION_MINOR		310		// Storage of current minor version number - used to detect change of firmware

#define	EEPROM_DETECTOR_COUPLING_COEFF_160M	311	// Calibration coupling coefficient for FWD/REV power sensor for 160 meters
#define	EEPROM_DETECTOR_COUPLING_COEFF_6M	312	// Calibration coupling coefficient for FWD/REV power sensor for 6 meters
#define EEPROM_TUNE_POWER_LEVEL		313
#define EEPROM_CAT_MODE_ACTIVE		314
#define EEPROM_CAT_XLAT			315
#define	EEPROM_FILTER_2K7_SEL		316		// Selection of 2.7 kHz filter
#define	EEPROM_FILTER_2K9_SEL		317		// Selection of 2.9 kHz filter


//
//
// NOTE:  EEPROM addresses up to 383 are currently defined
//
// *******************************************************************************************************
//
typedef struct FilterCoeffs
{
	float	rx_filt_q[128];
	uint16_t	rx_q_num_taps;
	uint32_t	rx_q_block_size;
	float	rx_filt_i[128];
	uint16_t	rx_i_num_taps;
	uint32_t	rx_i_block_size;
	//
	float	tx_filt_q[128];
	uint16_t	tx_q_num_taps;
	uint32_t	tx_q_block_size;
	float	tx_filt_i[128];
	uint16_t	tx_i_num_taps;
	uint32_t	tx_i_block_size;
} FilterCoeffs;

// Transceiver state public structure
typedef struct TransceiverState
{
	// Sampling rate public flag
	ulong 	samp_rate;

	// Virtual pots public values
	short  	rit_value;
	uchar 	audio_gain;
	float	audio_gain_active;	// working variable for processing audio gain - used in rx audio function
	uchar	audio_max_volume;	// limit for maximum audio gain
	uchar   audio_gain_change;	// change detect for audio gain
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
	ulong	tune_freq_old;		// used to detect change of main synthesizer frequency

	// Transceiver calibration mode flag
	//uchar	calib_mode;

	// Transceiver menu mode variables
	uchar	menu_mode;		// TRUE if in menu mode
	uchar	menu_item;		// Used to indicate specific menu item
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

	// Unattended TX public flag
	//uchar 	auto_mode;

	// Demodulator mode public flag
	uchar 	dmod_mode;

	// Digital mode public flag
	//uchar 	digi_mode;

	// FIR encoder current mode
	//uchar 	fir_enc_mode;

	// Gain encoder current mode
	//uchar 	gain_enc_mode;			// old var, to be removed
	uchar 	enc_one_mode;
	uchar 	enc_two_mode;
	uchar 	enc_thr_mode;

	uchar	tx_meter_mode;				// meter mode

	// Audio filter ID
	uchar	filter_id;
	//
	uchar	filter_300Hz_select;
	uchar	filter_500Hz_select;
	uchar	filter_1k8_select;
	uchar	filter_2k3_select;
	uchar	filter_2k7_select;
	uchar	filter_2k9_select;
	uchar	filter_3k6_select;
	uchar	filter_wide_select;
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
	uchar   tx_line_channel;  // 1 LEFT 2 RIGHT
	uchar	tx_mic_gain;
	ulong	tx_mic_gain_mult;
	ulong	tx_mic_gain_mult_temp;	// used to temporarily hold the mic gain when going from RX to TX
	uchar	tx_line_gain;
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
	bool	cat_mode_active;	// TRUE if CAT mode is on
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
#if 0
	uchar	pwr_80m_5w_adj;			// calibration adjust for 80 meters, 5 watts
	uchar	pwr_60m_5w_adj;			// calibration adjust for 60 meters, 5 watts
	uchar	pwr_40m_5w_adj;			// calibration adjust for 40 meters, 5 watts
	uchar	pwr_30m_5w_adj;			// calibration adjust for 30 meters, 5 watts
	uchar	pwr_20m_5w_adj;			// calibration adjust for 20 meters, 5 watts
	uchar	pwr_17m_5w_adj;			// calibration adjust for 17 meters, 5 watts
	uchar	pwr_15m_5w_adj;			// calibration adjust for 15 meters, 5 watts
	uchar	pwr_12m_5w_adj;			// calibration adjust for 12 meters, 5 watts
	uchar	pwr_10m_5w_adj;			// calibration adjust for 10 meters, 5 watts
	uchar	pwr_6m_5w_adj;			// calibration adjust for 6 meters, 5 watts
	uchar	pwr_4m_5w_adj;			// calibration adjust for 4 meters, 5 watts
	uchar	pwr_2m_5w_adj;			// calibration adjust for 2 meters, 5 watts
	uchar	pwr_70cm_5w_adj;		// calibration adjust for 70 centimeters, 5 watts
	uchar	pwr_23cm_5w_adj;		// calibration adjust for 23 centimeters, 5 watts
	uchar	pwr_2200m_5w_adj;		// calibration adjust for 2200 meters, 5 watts
	uchar	pwr_630m_5w_adj;		// calibration adjust for 630 meters, 5 watts
	uchar	pwr_160m_5w_adj;		// calibration adjust for 160 meters, 5 watts
	//
	uchar	pwr_80m_full_adj;			// calibration adjust for 80 meters, full power
	uchar	pwr_60m_full_adj;			// calibration adjust for 60 meters, full power
	uchar	pwr_40m_full_adj;			// calibration adjust for 40 meters, full power
	uchar	pwr_30m_full_adj;			// calibration adjust for 30 meters, full power
	uchar	pwr_20m_full_adj;			// calibration adjust for 20 meters, full power
	uchar	pwr_17m_full_adj;			// calibration adjust for 17 meters, full power
	uchar	pwr_15m_full_adj;			// calibration adjust for 15 meters, full power
	uchar	pwr_12m_full_adj;			// calibration adjust for 12 meters, full power
	uchar	pwr_10m_full_adj;			// calibration adjust for 10 meters, full power
	uchar	pwr_6m_full_adj;			// calibration adjust for 6 meters, full power
	uchar	pwr_4m_full_adj;			// calibration adjust for 4 meters, full power
	uchar	pwr_2m_full_adj;			// calibration adjust for 2 meters, full power
	uchar	pwr_70cm_full_adj;			// calibration adjust for 70 centimeters, full power
	uchar	pwr_23cm_full_adj;			// calibration adjust for 23 centimeters, full power
	uchar	pwr_2200m_full_adj;			// calibration adjust for 2200 meters, full power
	uchar	pwr_630m_full_adj;			// calibration adjust for 630 meters, full power
	uchar	pwr_160m_full_adj;			// calibration adjust for 160 meters, full power
#endif
	//
	ulong	alc_decay;					// adjustable ALC release time - EEPROM read/write version
	ulong	alc_decay_var;				// adjustable ALC release time - working variable version
	ulong	alc_tx_postfilt_gain;		// amount of gain after the TX audio filtering - EEPROM read/write version
	ulong	alc_tx_postfilt_gain_var;	// amount of gain after the TX audio filtering - working variable version
	//
	uchar	freq_step_config;			// configuration of step size (line, step button reversal) - setting any of the 4 upper bits -> step button switch, any of the lower bits -> frequency marker display enabled
	//
	bool	nb_disable;					// TRUE if noise blanker is to be disabled
	//
	uchar	dsp_active;					// Used to hold various aspects of DSP mode selection
										// LSB = 1 if DSP NR mode is on (| 1)
										// LSB+1 = 1 if DSP NR is to occur post AGC (| 2)
										// LSB+2 = 1 if DSP Notch mode is on (| 4)
										// LSB+3 = 0 if DSP is to be displayed on screen instead of NB (| 8)
										// MSB	 = 1 if button G2 toggle NOT initialized (| 128)
	uchar digital_mode;					// holds actual digital mode
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
	uchar	lcd_backlight_blanking;		// for controlling backlight auto-off control
	//
	uchar	tune_step;					// Used for press-and-hold tune step adjustment
	ulong	tune_step_idx_holder;		// used to hold the original step size index during the press-and-hold
	//
	bool	frequency_lock;				// TRUE if frequency knob is locked
	//
	uchar	tx_disable;					// TRUE if transmit is to be disabled
	//
	uchar	misc_flags1;				// Used to hold individual status flags, stored in EEPROM location "EEPROM_MISC_FLAGS1"
										// LSB = 0 if on-screen AFG/(STG/CMP) and WPM/(MIC/LIN) indicators are changed on TX
										// LSB+1 = 1 if BAND-/BAND+ buttons are to be swapped in their positions
										// LSB+2 = 1 if TX audio output from LINE OUT is to be muted during transmit (audio output only enabled
											//	when translate mode is DISABLED
										// LSB+3 = 1 if AM TX has transmit filter DISABLED
										// LSB+4 = 1 if FWD/REV A/D inputs from RF power detectors are to be reversed
										// LSB+5 = 1 if Frequency tuning is to be relaxed
										// LSB+6 = 1 if SSB TX has transmit filter DISABLED
										// LSB+7 = 0 = Spectrum Scope (analyzer), 1 = Waterfall display
	uchar	misc_flags2;				// Used to hold individual status flags, stored in EEPROM location "EEPROM_MISC_FLAGS2"
										// LSB = 0 if FM mode is DISABLED, 1 if FM mode is ENABLED
										// LSB+1 = 0 if 2.5 kHz FM deviation, 1 for 5 kHz FM deviation
										// LSB+2 = 1 if key/button beep is enabled
										// LSB+3 = 1 if memory-save versus frequency restrictions are to be relaxed
	ulong	sysclock;					// This counts up from zero when the unit is powered up at precisely 100 Hz over the long term.  This
										// is NEVER reset and is used for timing certain events.
	uint16_t	version_number_minor;	// version number - minor - used to hold version number and detect change
	uint16_t	version_number_build;	// version number - build - used to hold version number and detect change
	uint16_t	version_number_release;	// version number - release - used to hold version number and detect change
	uchar	nb_agc_time_const;			// used to calculate the AGC time constant
	uchar	cw_offset_mode;				// CW offset mode (USB, LSB, etc.)
	bool	cw_lsb;						// flag used to indicate that CW is to operate in LSB when TRUE
	uchar	iq_freq_mode;				// used to set/configure the I/Q frequency/conversion mode
	uchar	lsb_usb_auto_select;		// holds setting of LSB/USB auto-select above/below 10 MHz
	bool	conv_sine_flag;				// FALSE until the sine tables for the frequency conversion have been built (normally zero, force 0 to rebuild)
	ulong	hold_off_spectrum_scope;	// this is a timer used to hold off updates of the spectrum scope when an SPI LCD display interface is used
	ulong	lcd_blanking_time;			// this holds the system time after which the LCD is blanked - if blanking is enabled
	bool	lcd_blanking_flag;			// if TRUE, the LCD is blanked completely (e.g. backlight is off)
	bool	freq_cal_adjust_flag;		// set TRUE if frequency calibration is in process
	bool	xvtr_adjust_flag;			// set TRUE if transverter offset adjustment is in process
	bool	rx_muting;					// set TRUE if audio output is to be muted
	ulong	rx_blanking_time;			// this is a timer used to delay the un-blanking of the audio after a large synthesizer tuning step
	ulong	vfo_mem_mode;				// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
										// LSB+6 (0x40):  0 = VFO A, 1 = VFO B
										// LSB+7 (0x80): 0 = normal mode, 1 = Split mode (e.g. LSB=0:  RX=A, TX=B;  LSB=1:  RX=B, TX=A)
	ulong	voltmeter_calibrate;		// used to calibrate the voltmeter
	bool	thread_timer;				// used to trigger the thread timing (e.g. "driver_thread()")
	uchar	waterfall_color_scheme;		// stores waterfall color scheme
	uchar	waterfall_vert_step_size;	// vertical step size in waterfall mode
	ulong	waterfall_offset;			// offset for waterfall display
	ulong	waterfall_contrast;			// contrast setting for waterfall display
	uchar	spectrum_scope_scheduler;	// timer for scheduling the next update of the spectrum scope update, updated at DMA rate
	uchar	spectrum_scope_nosig_adjust;	// Adjustment for no signal adjustment conditions for spectrum scope
	uchar	waterfall_nosig_adjust;		// Adjustment for no signal adjustment conditions for waterfall
	uchar	waterfall_size;				// size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved
	uchar	fft_window_type;			// type of windowing function applied to scope/waterfall.  At the moment, only lower 4 bits are used - upper 4 bits are reserved
	bool	dvmode;						// TRUE if alternate (stripped-down) RX and TX functions (USB-only) are to be used
	uchar	tx_audio_muting_timing;		// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
	ulong	tx_audio_muting_timer;		// timer value used for muting TX audio when keying PTT to suppress "click" or "thump"
	uchar	filter_disp_colour;			// used to hold the current color of the line that indicates the filter passband/bandwidth
	bool	tx_audio_muting_flag;		// when TRUE, audio is to be muted after PTT/keyup
	bool	vfo_mem_flag;				// when TRUE, memory mode is enabled
	bool	mem_disp;					// when TRUE, memory display is enabled
	bool	load_eeprom_defaults;		// when TRUE, load EEPROM defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!
	ulong	fm_subaudible_tone_gen_select;	// lookup ("tone number") used to index the table tone generation (0 corresponds to "tone disabled")
	uchar	fm_tone_burst_mode;			// this is the setting for the tone burst generator
	ulong	fm_tone_burst_timing;		// this is used to time/schedule the duration of a tone burst
	uchar	fm_sql_threshold;			// squelch threshold "dial" setting
	uchar	fm_rx_bandwidth;			// bandwidth setting for FM reception
	ulong	fm_subaudible_tone_det_select;	// lookup ("tone number") used to index the table for tone detection (0 corresponds to "disabled")
	bool	beep_active;				// TRUE if beep is active
	ulong	beep_frequency;				// beep frequency, in Hz
	ulong	beep_timing;				// used to time/schedule the duration of a keyboard beep
	uchar	beep_loudness;				// loudness of the keyboard/CW sidetone test beep
	bool	load_freq_mode_defaults;	// when TRUE, load frequency/mode defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!
	bool	boot_halt_flag;				// when TRUE, boot-up is halted - used to allow various test functions
	bool	mic_bias;			// TRUE = mic bias on
	uchar	ser_eeprom_type;		// serial eeprom type
	uchar	ser_eeprom_in_use;		// 0xFF = not in use, 0x1 = in use
	uint8_t* eeprombuf;			// pointer to copy of config in RAM
	uchar 	tp_present;			// touchscreen present = 1, absent = 0
	uint8_t tp_x;				// touchscreen x coordinate
	uint8_t tp_y;				// touchscreen y coordinate
	bool	show_tp_coordinates;		// show coordinates on LCD
	uchar	rfmod_present;			// 0 = not present
	uchar	vhfuhfmod_present;		// 0 = not present
	uchar	multi;				// actual translate factor
	uchar	tune_power_level;		// TX power in antenna tuning function
	uchar	power_temp;			// temporary tx power if tune is different from actual tx power
	bool	dsp_enabled;			// NR disabled
	uchar	xlat;				// CAT <> IQ-Audio
//	uint16_t df8oe_test;			// only debugging use
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

// in main.c
void CriticalError(ulong error);

#endif
