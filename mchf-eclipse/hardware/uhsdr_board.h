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
#ifndef __MCHF_BOARD_H
#define __MCHF_BOARD_H
#include "uhsdr_mcu.h"
/***
 * Please document all switches/parameters with what they are supposed to do and what values they can have.
 * Please use proper naming:
 * For capabilities of the software which can be enabled and disabled
 * use USE_<CAPABILITY/FEATURENAME>
 *
 * These should be defined using #define USE_CAPABILITY
 * or left undefined if not enabled so that these can be checked using #ifdef
 *
 * For related parameters DON'T USE USE_...
 *
 * In an ideal world please use PAR_<CAPABILITY/FEATURE>_<PARAMETERNAME> (we haven't done that yet)
 * Please don't define constant or local parameters here, only those a user (!) is supposed to change as part of
 * configuring a specific build variant.
 *
 */

// old LMS noise reduction
// will probably never used any more
//#define OBSOLETE_NR

// this switches on the autonotch filter based on LMS algorithm
// leave this switched on, until we have a new autonotch filter approach
#define USE_LMS_AUTONOTCH

// save processor time for the STM32F4
// changes lowpass decimation filters to 89 taps instead of 199 taps
// because they run at 48ksps, this is a considerable decrease in processing power
#ifdef STM32F4
//#define USE_SMALL_HILBERT_DECIMATION_FILTERS
#endif

// save processor time for the STM32F4
// changes lowpass decimation filters to 89 taps instead of 199 taps
// because they run at 48ksps, this is a considerable decrease in processing power
// this is ONLY relevant for STM32F4 which has SPI !
// in those machines when enabling NR and other features, ui slows down . . .
// thus we should enable the small decimation filter in those machines


/**
 * This parameter disables certain features / capabilites in order to achieve a minimum build size for
 * the 192k ram / 512k flash STM32F4 machines. Unless you have such a machine, leave this disabled.
 */
// #define IS_SMALL_BUILD

// some special switches
//#define 	DEBUG_BUILD

// if enabled the alternate (read new and better) noise reduction is active
// this is the standard NR now (Febr 2018)
#define USE_ALTERNATE_NR


  #define USE_GFX_ILI932x
  #define USE_GFX_ILI9486
  #define USE_DISP_480_320
  #define USE_FFT_1024
#ifndef IS_SMALL_BUILD
  #define USE_8bit_FONT
  #define USE_PREDEFINED_WINDOW_DATA
#endif

// OPTION
#define USE_RTTY_PROCESSOR

// OPTION
#define USE_USBHOST
#ifdef USE_USBHOST
// define additional USBHOST related "switches" only here!
	// #define USE_USBDRIVE
	#define USE_USBKEYBOARD
#endif

// OPTION
// with IS_SMALL_BUILD we are not automatically including USE_FREEDV as it uses lot of memory both RAM and flash
#ifndef IS_SMALL_BUILD
    #define USE_FREEDV
#endif
// #define DEBUG_FREEDV
// hardware specific switches


// Unified the 3 graphics drivers.

// OPTION

// use the STM32 internal RTC with an external quartz and
// M1 and F3 connected to PD14 and PD15 (D0 and D1 of LCD) instead of PC14 and PC15 (to which the 32768 Hz quartz has to be connected)
#define USE_RTC_LSE

// AT LEAST ONE GROUP START USE_OSC
// multiple oscillators may be enabled, but only the first detected oscillator is issued
// i.e. there is currently only support for a single oscillator in a TRX.
// Support for LO based on SI570
#define USE_OSC_SI570
// Support for LO based on SI5351
#define USE_OSC_SI5351A
// AT LEAST ONE GROUP END USE_OSC


// Option: If defined, high priority tasks are executed in the context of an PendSV interrupt
// which gives finishing these tasks a priority over "normal", less real-time critical longer running user control tasks
// such as display redraw.
// In general this should be defined but in case of issues one may want to execute High Prio tasks not concurrently
// to normal tasks, comment this in this case and see if the issue goes away. But this may cause other problems
// of course.
#define USE_PENDSV_FOR_HIGHPRIO_TASKS

#include "uhsdr_mcu.h"
// HW libs
#ifdef STM32F7
#include "stm32f7xx_hal_rcc.h"
#include "stm32f7xx_hal_gpio.h"
#include "stm32f7xx_hal_spi.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_i2c.h"
#include "stm32f7xx_hal_i2s.h"
#include "stm32f7xx_hal_adc.h"
#include "stm32f7xx_hal_dac.h"
#include "stm32f7xx_hal_tim.h"
#include "stm32f7xx_hal_rtc.h"
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_flash.h"
#include "core_cm7.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal_rcc.h"
#include "stm32h7xx_hal_gpio.h"
#include "stm32h7xx_hal_spi.h"
#include "stm32h7xx_hal_dma.h"
#include "stm32h7xx_hal_i2c.h"
#include "stm32h7xx_hal_i2s.h"
#include "stm32h7xx_hal_adc.h"
#include "stm32h7xx_hal_dac.h"
#include "stm32h7xx_hal_tim.h"
#include "stm32h7xx_hal_rtc.h"
#include "stm32h7xx_hal_pwr.h"
#include "stm32h7xx_hal_flash.h"
#include "core_cm7.h"
#else
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_dac.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_rtc.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_flash.h"
#include "core_cm4.h"
#endif

#include "freedv_api.h"

#include "uhsdr_types.h"
#include "audio_filter.h"
#include "osc_interface.h"
#include "ui_lcd_items.h"
#include "ui_lcd_hy28.h"

#include "comp.h"
#include "dac.h"

#include "uhsdr_board_config.h"

// Buttons map structure
typedef struct ButtonMap
{
    GPIO_TypeDef 	*port;
    ushort			button;
    const char*     label;

} ButtonMap;

typedef struct
{
    const ButtonMap* map;
    uint32_t num;
} mchf_buttons_t;

// Button definitions
//
enum
{
    BUTTON_M2_PRESSED = 0,  // 0
    BUTTON_G3_PRESSED,  	// 1
    BUTTON_G2_PRESSED,  	// 2
    BUTTON_BNDM_PRESSED,    // 3
    BUTTON_G4_PRESSED,  	// 4
    BUTTON_M3_PRESSED,  	// 5
    BUTTON_STEPM_PRESSED,   // 6
    BUTTON_STEPP_PRESSED,   // 7
    BUTTON_M1_PRESSED,  	// 8
    BUTTON_F3_PRESSED,  	// 9 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F1_PRESSED,  	// 10 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F2_PRESSED,  	// 11 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F4_PRESSED,  	// 12 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_BNDP_PRESSED,    // 13
    BUTTON_F5_PRESSED,  	// 14 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_G1_PRESSED,  	// 15
    BUTTON_POWER_PRESSED,   // 16 - Used for press and release
    TOUCHSCREEN_ACTIVE, 	// 17 - Touchscreen touched, needs to last entry before BUTTON_NUM,
    //      init code relies on this
    BUTTON_NUM // How many buttons we have defined
};

extern mchf_buttons_t  buttons;
extern const ButtonMap  bm_sets[2][BUTTON_NUM];

struct mchf_waterfall
{
    uint8_t	color_scheme;			// stores waterfall color scheme
    uint8_t	vert_step_size;		// vertical step size in waterfall mode
    // int32_t	offset;			// offset for waterfall display
    ulong	contrast;			// contrast setting for waterfall display
	uint8_t	speed;	// speed of update of the waterfall
	// uint8_t	nosig_adjust;			// Adjustment for no signal adjustment conditions for waterfall
	uint16_t scheduler;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define POWER_BUTTON_HOLD_TIME	1000000

#define TRX_MODE_RX				0
#define TRX_MODE_TX				1

#define DEMOD_USB				0
#define DEMOD_LSB				1
#define DEMOD_CW				2
#define DEMOD_AM				3
#define	DEMOD_SAM				4
#define	DEMOD_FM				5
#define DEMOD_DIGI				6
#ifdef USE_TWO_CHANNEL_AUDIO
#define DEMOD_SSBSTEREO			7
#define DEMOD_IQ				8
#define DEMOD_MAX_MODE			8
#else
#define DEMOD_MAX_MODE			6
#endif

// codec x demod
// analog USB LSB CW AM FM SAM
// FreeDV USB LSB -  -  -  -



#define RTC_OSC_FREQ			32768

#define	TCXO_OFF				0		// TXCO temperature compensation off
#define	TCXO_ON					1		// TCXO temperature compensation on
#define	TCXO_STOP				2		// Stop reading of temperature sensor
#define	TCXO_TEMP_STATE_MAX		2		// Maximum setting for TCXO setting state

// Transverter oscillator adds shift
#define		TRANSVT_FREQ_A	 	42000000

//
#define		MIN_FREQ_CAL		-1499		// Minimum and maximum range of frequency calibration in 10xppm
#define		MAX_FREQ_CAL		1499
//
// Total bands supported
//
#define	MIN_BANDS				0		// lowest band number
#define	MAX_BANDS				17		// Highest band number:  17 = General coverage (RX only) band
#define	MAX_BAND_NUM			(MAX_BANDS+1)		// Number of Bands

//  multiplier to convert between dial_freq and tune_freq
#define TUNE_MULT				4

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
typedef enum {
    ENC_ONE_MODE_AUDIO_GAIN	 = 0,
    ENC_ONE_MODE_RTTY_SPEED,
    ENC_ONE_MODE_ST_GAIN,
    ENC_ONE_MODE_CMP_LEVEL,
    ENC_ONE_NUM_MODES
} EncoderOneModes;
//
// encoder two
typedef enum {
    ENC_TWO_MODE_RF_GAIN =		0,
    ENC_TWO_MODE_RTTY_SHIFT,
    ENC_TWO_MODE_SIG_PROC,
    ENC_TWO_MODE_NR,
    ENC_TWO_MODE_NOTCH_F,
    ENC_TWO_MODE_PEAK_F,
    ENC_TWO_MODE_BASS_GAIN,
    ENC_TWO_MODE_TREBLE_GAIN,
    ENC_TWO_NUM_MODES
} EncoderTwoModes;
//
// encoder three
typedef enum {
    ENC_THREE_MODE_RIT =			0,
    ENC_THREE_MODE_CW_SPEED,
    ENC_THREE_MODE_INPUT_CTRL,
	ENC_THREE_MODE_PSK_SPEED,
    ENC_THREE_NUM_MODES
} EncoderThreeModes;


#define CW_KEYER_MODE_IAM_B				0
#define CW_KEYER_MODE_IAM_A				1
#define CW_KEYER_MODE_STRAIGHT			2
#define CW_KEYER_MODE_ULTIMATE			3
#define CW_KEYER_MAX_MODE				3

// PA power level setting enumeration
typedef enum
{
    PA_LEVEL_FULL = 0,
    PA_LEVEL_5W,
    PA_LEVEL_2W,
    PA_LEVEL_1W,
    PA_LEVEL_0_5W,
    PA_LEVEL_TUNE_KEEP_CURRENT
} mchf_power_level_t;
//
#define	PA_LEVEL_DEFAULT		PA_LEVEL_2W		// Default power level

#define	SSB_TUNE_FREQ			750	// Frequency at which the SSB TX IQ gain and phase adjustment is to be done
//
#define VOICE_TX2RX_DELAY_DEFAULT			450	// Delay for switching when going from TX to RX (this is 0.66uS units)
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
#define TX_POWER_FACTOR_MAX_INTERNAL 0.55 // we limit power factor  to 55 (.55) . This limit is independent of the possible scale factor 4 for the power factor
#define	TX_POWER_FACTOR_MAX		(TX_POWER_FACTOR_MAX_INTERNAL*400.0)		// Please keep in mind that this is only a setting value maximum. Depending on the flags this reduced by 4 before further use.
                                        //And the true maximum is defined above in TX_POWER_FACTOR_MAX_INTERNAL

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
typedef enum
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
} mchf_color_t;

typedef struct {
    const uint32_t value;
    const char* name;
} ColorNameValue;

// this data structure needs to be in sync with the color definitions above
// otherwise strange colors may become visible in the UI.
extern const ColorNameValue MchfColor_Id2ValueName[SPEC_MAX_COLOUR];

// Enumeration of transmit meter modes
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
#define LOW_POWER_SHUTDOWN_DELAY_TIME   6000        // number of DECISECONDS after power-up before low power auto shutdown is checked



// Enumeration of transmit tune  modes
typedef enum
{
    TUNE_TONE_SINGLE = 0,
    TUNE_TONE_TWO,
    TUNE_TONE_MAX,
} TuneToneMode;


typedef struct Gain_s
{
    uint8_t value;
    uint8_t max;
    uint8_t value_old;
    float   active_value;
} Gain;

typedef enum {
    IQ_TRANS_OFF = 0,
    IQ_TRANS_ON,
    IQ_TRANS_NUM
} iq_trans_idx_t;


typedef struct {
    int32_t value[IQ_TRANS_NUM];
}
iq_balance_data_t;

#define KEYER_BUTTONS 3
#define KEYER_BUTTON_NONE -1
#define KEYER_BUTTON_1 0
#define KEYER_BUTTON_2 1
#define KEYER_BUTTON_3 2
#define KEYER_MACRO_LEN 200
#define KEYER_CAP_LEN 6
typedef struct {
	bool active;
	int8_t button_recording;
	uint8_t macro[KEYER_BUTTONS][KEYER_MACRO_LEN];
	uint8_t cap[KEYER_BUTTONS][KEYER_CAP_LEN + 1];
}
keyer_mode_t;

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
extern BandRegs vfo[VFO_MAX];


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
    uint8_t lineout_gain;            // lineout gain to control lineout level


#define MAX_RF_CODEC_GAIN_VAL       9       // Maximum RF gain setting
#define DEFAULT_RF_CODEC_GAIN_VAL   9       // Default RF gain setting (9 = AUTO mode)
#define RF_CODEC_GAIN_AUTO   9       // Default RF gain setting (9 = AUTO mode)

    uint8_t	rf_codec_gain;		// gain for codec (A/D converter) in receive mode
    uint8_t 	nb_setting;
    uint8_t	cw_sidetone_gain;
    uint8_t	pa_bias;
    uint8_t	pa_cw_bias;

    // timer for muting of input into signal processing chains (TX/RX)
    uint16_t    audio_processor_input_mute_counter;
#define IQ_ADJUST_POINTS_NUM 4

    // corresponding frequencies are stored in const array iq_adjust_freq
#define IQ_80M 0
#define IQ_10M 1
#define IQ_20M 2

    iq_balance_data_t tx_iq_gain_balance[IQ_ADJUST_POINTS_NUM];  // setting for TX IQ gain balance
    iq_balance_data_t tx_iq_phase_balance[IQ_ADJUST_POINTS_NUM]; // setting for TX IQ phase balance
    iq_balance_data_t rx_iq_gain_balance[IQ_ADJUST_POINTS_NUM];  // setting for RX IQ gain balance
    iq_balance_data_t rx_iq_phase_balance[IQ_ADJUST_POINTS_NUM]; // setting for RX IQ phase balance

    iq_float_t tx_adj_gain_var[IQ_TRANS_NUM];    // active variables for adjusting tx gain balance
    iq_float_t rx_adj_gain_var;    // active variables for adjusting rx gain balance

    // Equalisation factor
    float32_t	tx_power_factor;

    int	freq_cal;				// frequency calibration

    // Frequency synthesizer
    ulong	tune_freq;			// main synthesizer frequency
    ulong	tune_freq_req;		// used to detect change of main synthesizer frequency

    // Transceiver calibration mode flag
    //uchar	calib_mode;

    // Transceiver menu mode variables
    uint8_t	menu_mode;		// TRUE if in menu mode
    int16_t	menu_item;		// Used to indicate specific menu item
    int		menu_var;		// Used to change specific menu item
    bool	menu_var_changed;	// TRUE if something changed in a menu and that an EEPROM save should be done!

    // Ham band public flag
    // index of bands table in Flash
    uint8_t 	band;
    bool	rx_temp_mute;
    uint8_t	filter_band;		// filter selection band:  1= 80, 2= 60/40, 3=30/20, 4=17/15/12/10 - used for selection of power detector coefficient selection.
    //
    // Receive/Transmit public flag
    uint8_t 	txrx_mode;

    // TX/RX IRQ lock, to prevent reentrance
    //uchar	txrx_lock;
    bool	ptt_req;
    bool tx_stop_req;


    // Demodulator mode public flag
    uint8_t 	dmod_mode;


    uint8_t 	enc_one_mode;
    uint8_t 	enc_two_mode;
    uint8_t 	enc_thr_mode;

    uint8_t	tx_meter_mode;				// meter mode

    // Audio filter ID
    // uchar	filter_id;
    //
    uint8_t   filter_select[AUDIO_FILTER_NUM];


#define FILTER_PATH_MEM_MAX 5
    uint16_t   filter_path_mem[FILTER_MODE_MAX][FILTER_PATH_MEM_MAX];

    uint16_t  filter_path;
    //

    uint8_t	filter_cw_wide_disable;		// TRUE if wide filters are disabled in CW mode
    uint8_t	filter_ssb_narrow_disable;	// TRUE if narrow filters are disabled in SSB modes
    //
    uint16_t	demod_mode_disable;			// TRUE if AM mode is to be disabled
#define DEMOD_AM_DISABLE    (0x0001)
#define DEMOD_CW_DISABLE    (0x0002)
#define DEMOD_DIGI_DISABLE  (0x0004)




    // AGC mode
//    uint8_t	agc_mode;
//    uint8_t	agc_custom_decay;

//    uint8_t	max_rf_gain;

    // Eth to UI driver requests flag
    uint8_t	LcdRefreshReq;

    // Eth to UI public flag
    uint8_t	new_band;
    uint8_t	new_mode;
    uint8_t	new_digi_mode;

    // Current CW mode
    uint8_t	cw_keyer_mode;
    uint8_t	cw_keyer_speed;
    uint8_t	cw_paddle_reverse;
    bool cw_text_entry;

    uint8_t cw_keyer_weight;   // cw dit/pause ratio 100 = 1.00 -> dit == pause == dah / 3
#define CW_KEYER_WEIGHT_DEFAULT (100)
#define CW_KEYER_WEIGHT_MAX     (150)
#define CW_KEYER_WEIGHT_MIN      (50)

    uint8_t cw_rx_delay; // break time
#define CW_TX2RX_DELAY_DEFAULT     8
#define CW_RX_DELAY_MAX         50  // Maximum TX to RX turnaround setting

    uint32_t cw_sidetone_freq;
#define CW_SIDETONE_FREQ_DEFAULT    750 // Default CW Audio Sidetone and TX offset frequency in Hz
#define CW_SIDETONE_FREQ_MIN        400
#define CW_SIDETONE_FREQ_MAX        1000

    ulong	audio_spkr_unmute_delay_count;

    uint8_t	power_level;

    uint8_t 	tx_audio_source;
    ulong	tx_mic_gain_mult;
    uint8_t	tx_gain[TX_AUDIO_NUM];
    int16_t	tx_comp_level;			// Used to hold compression level which is used to calculate other values for compression.  0 = manual.

    // Global tuning flag - in every demod mode
    uint8_t 	tune;

    uint16_t ee_init_stat;

    uint8_t	powering_down;

    // Spectrum Scope config - placed here since "sd." not defined at time of init

    uint8_t spectrum_size;              // size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved
    uint8_t	spectrum_filter;	// strength of filter in spectrum scope
    uint8_t spectrum_centre_line_colour;    // color of center line of scope grid
    uint8_t spectrum_freqscale_colour;  // color of spectrum scope frequency scale
    uint8_t spectrum_agc_rate;      // agc rate on the 'scope
    uint8_t spectrum_db_scale;  // db/Division scale setting on spectrum scope
    //  uint8_t   fft_window_type;            // type of windowing function applied to scope/waterfall.  At the moment, only lower 4 bits are used - upper 4 bits are reserved

    uint16_t scope_scheduler;        // timer for scheduling the next update of the spectrum scope update, updated at DMA rate
    uint8_t scope_speed;    // update rate for spectrum scope
    uint8_t	scope_trace_colour;	// color of spectrum scope trace;
    uint8_t	scope_grid_colour;	// saved color of spectrum scope grid;
    uint8_t scope_trace_BW_colour;	// color of BW highlighted spectrum scope trace
    uint8_t scope_backgr_BW_colour; // color of BW highlighted background of spectrum scope (% of white)
    // uint8_t  spectrum_scope_nosig_adjust;        // Adjustment for no signal adjustment conditions for spectrum scope

    struct mchf_waterfall waterfall;

    //
    bool	radio_config_menu_enable;	// TRUE if radio configuration menu is to be visible
    //
    uint8_t	xverter_mode;		// TRUE if transverter mode active
    ulong	xverter_offset;		// frequency offset for transverter (added to frequency display)

    bool	refresh_freq_disp;		// TRUE if frequency display display is to be refreshed
    //
    // Calibration factors for output power, in percent (100 = 1.00)
    //
#define ADJ_5W 0
#define ADJ_FULL_POWER 1
    uint8_t	pwr_adj[2][MAX_BAND_NUM];
    //
    ulong	alc_decay;					// adjustable ALC release time - EEPROM read/write version
    ulong	alc_decay_var;				// adjustable ALC release time - working variable version
    ulong	alc_tx_postfilt_gain;		// amount of gain after the TX audio filtering - EEPROM read/write version
    ulong	alc_tx_postfilt_gain_var;	// amount of gain after the TX audio filtering - working variable version

// we can use AT least the upper 8 bits of freq_step_config for other purpose since these have not been used and are all initialized with 0)
#define FREQ_STEP_SWAP_BTN	    0x10
#define FREQ_STEP_SHOW_MARKER   0x01
    uint16_t	freq_step_config;			// configuration of step size (line, step button reversal) - setting any of the 4 upper bits -> step button switch, any of the lower bits -> frequency marker display enabled

#define DSP_NR_ENABLE 	  		0x01	// DSP NR mode is on (| 1)
#define DSP_NR_POSTAGC_ENABLE 	0x02	// DSP NR is to occur post AGC (| 2)
#define DSP_NOTCH_ENABLE		0x04	// DSP Notch mode is on (| 4)
#define DSP_NB_ENABLE			0x08	// DSP is to be displayed on screen instead of NB (| 8)
#define DSP_MNOTCH_ENABLE		0x10	// Manual Notch enabled
#define DSP_MPEAK_ENABLE		0x20	// Manual Peak enabled

    uint8_t	dsp_active;					// Used to hold various aspects of DSP mode selection
    uint8_t	dsp_mode;					// holds the mode chosen in the DSP
	uint8_t	temp_nb;
    uint8_t 	digital_mode;				// holds actual digital mode
    uint8_t	dsp_active_toggle;			// holder used on the press-hold of button G2 to "remember" the previous setting
    uint8_t	dsp_nr_strength;			// "Strength" of DSP Noise reduction - to be converted to "Mu" factor
    ulong	dsp_nr_delaybuf_len;		// size of DSP noise reduction delay buffer
    uint8_t	dsp_nr_numtaps;				// Number of FFT taps on the DSP Noise reduction
    uint8_t	dsp_notch_numtaps;
    uint8_t	dsp_notch_mu;				// mu adjust of notch DSP LMS
    uint8_t	dsp_notch_delaybuf_len;		// size of DSP notch delay buffer
    uint8_t dsp_inhibit;				// if != 0, DSP (NR, Notch) functions are inhibited.  Used during power-up and switching


    uint8_t	lcd_backlight_brightness;	// LCD backlight brightness, 0-3:  0 = full, 3 = dimmest

#define LCD_BLANKING_ENABLE 0x80
#define LCD_BLANKING_TIMEMASK 0x0f
    uint8_t	lcd_backlight_blanking;		// for controlling backlight auto-off control



#define LOW_POWER_ENABLE 0x80    // bit7 shows enable / no enable
#define LOW_POWER_ENABLE_MASK 0x80

#define LOW_POWER_THRESHOLD_OFFSET 30    // value stored in the configuration variable is lower by this offset
#define LOW_POWER_THRESHOLD_MASK 0x7f
#define LOW_POWER_THRESHOLD_DEFAULT  0
#define LOW_POWER_THRESHOLD_MIN  0
#define LOW_POWER_THRESHOLD_MAX  126



    uint8_t   low_power_config;        // for voltage colours and auto shutdown
    ulong   low_power_shutdown_time;    // earliest time when auto shutdown can be executed
    //
    uint8_t	tune_step;					// Used for press-and-hold tune step adjustment
    ulong	tune_step_idx_holder;		// used to hold the original step size index during the press-and-hold
    //
    bool	frequency_lock;				// TRUE if frequency knob is locked
    //

#define TX_DISABLE_OFF          0
#define TX_DISABLE_ALWAYS       1
#define TX_DISABLE_USER         2
#define TX_DISABLE_OUTOFRANGE	4
    uint8_t	tx_disable;		// >0 if no transmit permitted, use RadioManagement_IsTxDisabled() to get boolean


    uint16_t	flags1;					// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS1"
#define FLAGS1_TX_AUTOSWITCH_UI_DISABLE 0x01    // if on-screen AFG/(STG/CMP) and WPM/(MIC/LIN) indicators are changed on TX
#define FLAGS1_SWAP_BAND_BTN			0x02    // if BAND-/BAND+ buttons are to be swapped in their positions
#define FLAGS1_MUTE_LINEOUT_TX			0x04    // if TX audio output from LINE OUT is to be muted during transmit (audio output only enabled when translate mode is DISABLED
#define FLAGS1_AM_TX_FILTER_DISABLE		0x08    // if AM TX has transmit filter DISABLED
#define FLAGS1_SWAP_FWDREV_SENSE		0x10    // if FWD/REV A/D inputs from RF power detectors are to be reversed
#define FLAGS1_FREQ_LIMIT_RELAX			0x20    // if Frequency tuning is to be relaxed
#define FLAGS1_SSB_TX_FILTER_DISABLE	0x40    // if SSB TX has transmit filter DISABLED
#define FLAGS1_WFALL_ENABLED		    0x80    // 1 = Waterfall display
#define FLAGS1_SCOPE_ENABLED	        0x100   // 1 = Scope display
#define FLAGS1_DYN_TUNE_ENABLE			0x200   // 0 = dynamic tune is disabled, 1 = dynamic tune is enabled
#define FLAGS1_SAM_ENABLE				0x400   // 0 = SAM mode is disabled, 1 = SAM mode is enabled
#define FLAGS1_CAT_IN_SANDBOX			0x800   // 0 = CAT works on band storage, 1 = CAT works in sandbox
#define FLAGS1_SCOPE_LIGHT_ENABLE		0x1000  // 0 = Spectrum normal, 1 = Spectrum light
#define FLAGS1_TX_OUTSIDE_BANDS			0x2000  // 1 = TX outside bands enabled
#define FLAGS1_REVERSE_X_TOUCHSCREEN	0x4000  // 1 = X direcction of touchscreen is mirrored
#define FLAGS1_REVERSE_Y_TOUCHSCREEN	0x8000  // 1 = Y direcction of touchscreen is mirrored

#ifdef UI_BRD_MCHF
    // the default screen needs no reversed touch
#define FLAGS1_CONFIG_DEFAULT (FLAGS1_WFALL_ENABLED|FLAGS1_SCOPE_ENABLED)
#define TOUCHSCREEN_DF_MIRROR	TOUCHSCREEN_NO_MIRROR_NOFLIP
#endif
#ifdef UI_BRD_OVI40
    // the default screen needs reversed x axis touch
#define FLAGS1_CONFIG_DEFAULT (FLAGS1_REVERSE_X_TOUCHSCREEN)
#define TOUCHSCREEN_DF_MIRROR	TOUCHSCREEN_X_MIRROR_NOFLIP
#endif


    uint16_t	flags2;							// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS2"
#define FLAGS2_FM_MODE_ENABLE 			0x01    // 0 if FM mode is DISABLED, 1 if FM mode is ENABLED
#define FLAGS2_FM_MODE_DEVIATION_5KHZ 	0x02    // 0 if 2.5 kHz FM deviation, 1 for 5 kHz FM deviation
#define FLAGS2_KEY_BEEP_ENABLE 			0x04    // 1 if key/button beep is enabled
#define FLAGS2_LOW_BAND_BIAS_REDUCE 	0x08    // 1 if bias values for lower bands  below 8Mhz have lower influence factor
#define FLAGS2_FREQ_MEM_LIMIT_RELAX 	0x10    // 1 if memory-save versus frequency restrictions are to be relaxed
#define FLAGS2_TOUCHSCREEN_FLIP_XY	 	0x20    // 1 if touchscreen x and y are flipped
#define FLAGS2_HIGH_BAND_BIAS_REDUCE    0x40    // 1 if bias values for higher bands  above 8Mhz have lower influence factor
#define FLAGS2_UI_INVERSE_SCROLLING		0x80    // 1 if inverted Enc2/Enc3 UI actions, clockwise goes previous UiMenu_RenderChangeItem, folds up menu groups
#define FLAGS2_CONFIG_DEFAULT (FLAGS2_HIGH_BAND_BIAS_REDUCE|FLAGS2_LOW_BAND_BIAS_REDUCE)

    uint32_t	sysclock;				// This counts up from zero when the unit is powered up at precisely 100 Hz over the long term.  This
    // is NEVER reset and is used for timing certain events.
    uint16_t	version_number_minor;		// version number - minor - used to hold version number and detect change
    uint16_t	version_number_major;		// version number - build - used to hold version number and detect change
    uint16_t	version_number_release;		// version number - release - used to hold version number and detect change
    uint8_t	nb_agc_time_const;			// used to calculate the AGC time constant
    uint8_t	cw_offset_mode;				// CW offset mode (USB, LSB, etc.)
    bool	cw_lsb;					// flag used to indicate that CW is to operate in LSB when TRUE
    uint8_t	iq_freq_mode;				// used to set/configure the I/Q frequency/conversion mode
    uint8_t	lsb_usb_auto_select;			// holds setting of LSB/USB auto-select above/below 10 MHz
    bool	conv_sine_flag;				// FALSE until the sine tables for the frequency conversion have been built (normally zero, force 0 to rebuild)
    ulong	last_tuning;				// this is a timer used to prevent too fast tuning per second
    ulong	lcd_blanking_time;			// this holds the system time after which the LCD is blanked - if blanking is enabled
    bool	lcd_blanking_flag;			// if TRUE, the LCD is blanked completely (e.g. backlight is off)
    bool	xvtr_adjust_flag;			// set TRUE if transverter offset adjustment is in process
    bool	SpectrumResize_flag;		// set TRUE if waterfall/spectrum resize request from touchscreen action
    uint32_t SpectrumResize_timer;		//
#define VFO_MEM_MODE_SPLIT 0x80
#define VFO_MEM_MODE_VFO_B 0x40
    ulong	vfo_mem_mode;				// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
    // LSB+6 (0x40):  0 = VFO A, 1 = VFO B
    // LSB+7 (0x80): 0 = normal mode, 1 = Split mode (e.g. LSB=0:  RX=A, TX=B;  LSB=1:  RX=B, TX=A)
    ulong	voltmeter_calibrate;			// used to calibrate the voltmeter


    bool	dvmode;					// TRUE if alternate (stripped-down) RX and TX functions (USB-only) are to be used
    uint8_t	txrx_switch_audio_muting_timing;			// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
    uint32_t	audio_dac_muting_timer;			// timer value used for muting TX audio when keying PTT to suppress "click" or "thump"
    uint32_t audio_dac_muting_buffer_count; // the audio dac out will be muted for number of buffers
    uint8_t	filter_disp_colour;			// used to hold the current color of the line that indicates the filter passband/bandwidth
    bool	audio_dac_muting_flag;			// when TRUE, audio is to be muted after PTT/keyup
    bool	vfo_mem_flag;				// when TRUE, memory mode is enabled
    bool	mem_disp;				// when TRUE, memory display is enabled
    bool	load_eeprom_defaults;			// when TRUE, load EEPROM defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!
    ulong	fm_subaudible_tone_gen_select;		// lookup ("tone number") used to index the table tone generation (0 corresponds to "tone disabled")
    uint8_t	fm_tone_burst_mode;			// this is the setting for the tone burst generator
    ulong	fm_tone_burst_timing;			// this is used to time/schedule the duration of a tone burst
    uint8_t	fm_sql_threshold;			// squelch threshold "dial" setting
//	uchar	fm_rx_bandwidth;			// bandwidth setting for FM reception
    ulong	fm_subaudible_tone_det_select;		// lookup ("tone number") used to index the table for tone detection (0 corresponds to "disabled")
    bool	beep_active;				// TRUE if beep is active
    ulong	beep_frequency;				// beep frequency, in Hz
    ulong	beep_timing;				// used to time/schedule the duration of a keyboard beep
    uint8_t	beep_loudness;				// loudness of the keyboard/CW sidetone test beep
    bool	load_freq_mode_defaults;		// when TRUE, load frequency/mode defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!

#define EEPROM_SER_NONE 0
#define EEPROM_SER_WRONG_SIG 1
#define EEPROM_SER_UNKNOWN 2
    uint8_t	ser_eeprom_type;			// serial eeprom type

#define CONFIGSTORE_IN_USE_I2C         0x00
#define CONFIGSTORE_IN_USE_ERROR       0x05
#define CONFIGSTORE_IN_USE_RAMCACHE    0xAA
#define CONFIGSTORE_IN_USE_FLASH       0xFF

    uint8_t	configstore_in_use;	    // use to determine non-volatile memory configuration

    mchf_touchscreen_t *tp;

    bool	show_debug_info;	// show coordinates on LCD
    bool	rfmod_present;			// 0 = not present
    bool	vhfuhfmod_present;		// 0 = not present
    uint8_t	multi;					// actual translate factor
    uint8_t	tune_power_level;		// TX power in antenna tuning function
    uint8_t	power_temp;				// temporary tx power if tune is different from actual tx power
    uint8_t	cat_band_index;			// buffered bandindex before first CAT command arrived
    uint8_t	xlat;					// CAT <> IQ-Audio
    ulong	notch_frequency;		// frequency of the manual notch filter
    ulong	peak_frequency;			// frequency of the manual peak filter
    int		bass_gain;				// gain of the low shelf EQ filter
    int		treble_gain;			// gain of the high shelf EQ filter
    int		tx_bass_gain;			// gain of the TX low shelf EQ filter
    int		tx_treble_gain;			// gain of the TX high shelf EQ filter

//    bool	dBm_Hz_Test;			// for testing only
//    ulong	dBm_count;				// timer for calculating RX dBm
    uint8_t 	display_dbm;			// display dbm or dbm/Hz or OFF
    uint8_t	s_meter;				// defines S-Meter style/configuration
	uint8_t	meter_colour_up;
	uint8_t	meter_colour_down;
	uint8_t   iq_auto_correction;     // switch variable for automatic IQ correction
	bool	display_rx_iq;
	uint8_t twinpeaks_tested;
//	uint8_t agc_wdsp;
	uint8_t agc_wdsp_mode;
	uint8_t agc_wdsp_slope;
	uint8_t agc_wdsp_hang_enable;
	int     agc_wdsp_thresh;
	int     agc_wdsp_hang_thresh;
	int agc_wdsp_hang_time;
	uint8_t agc_wdsp_action;
	uint8_t agc_wdsp_switch_mode;
	uint8_t agc_wdsp_hang_action;
	int dbm_constant;
	int agc_wdsp_tau_decay[6];
	int agc_wdsp_tau_hang_decay;

//#define DISPLAY_S_METER_STD   0
#define DISPLAY_S_METER_DBM   1
#define DISPLAY_S_METER_DBMHZ 2

//    #define TX_FILTER_NONE			0
    #define TX_FILTER_SOPRANO		1
    #define TX_FILTER_TENOR			2
    #define TX_FILTER_BASS			3
    uint8_t	tx_filter;				// which TX filter has been chosen?


    mchf_display_t*     display;

    uint32_t audio_int_counter;		// used for encoder timing - test DL2FW
    bool encoder3state;
    int bc_band;

    Oscillator_ResultCodes_t last_lo_result;			// used in dynamic tuning to hold frequency color

    TuneToneMode tune_tone_mode;

	uint16_t ramsize; // in KB, this is used to distinguish  between 192 and 256 kB models.

	uint8_t stream_tx_audio; // send tx audio via usb back
#define STREAM_TX_AUDIO_OFF     0  // send nothing
#define STREAM_TX_AUDIO_SRC     1  // send source audio stream (from CODEC)
#define STREAM_TX_AUDIO_FILT    2  // send processed audio stream (after filtering)
#define STREAM_TX_AUDIO_DIGIQ   3  // send final IQ signal
#define STREAM_TX_AUDIO_NUM   4  // how many choices

	// Freedv Test DL2FW
	bool	FDV_TX_encode_ready;
	int	FDV_TX_samples_ready;
	uint16_t FDV_TX_out_start_pt;
	uint16_t FDV_TX_in_start_pt;

    bool    digi_lsb;                 // flag used to indicate that mcHF is to operate in LSB when TRUE

    bool dial_moved; // dial was moved, used to communicate with spectrum display code


    uint32_t i2c_speed[2]; // store comm speed for the 2 I2C buses
#define I2C_BUS_1 0
#define I2C_BUS_2 1


    bool rtc_present; // a supported rtc was found and is active
    int16_t rtc_calib; // ppm variation value, unit 1 ppm
    bool vbat_present; // we detected a working vbat mod
    bool codec_present; // we detected a working codec
	bool new_nb; // new noise blanker
	bool rtty_atc_enable; // is ATC enabled for RTTY decoding? (for testing!)

	uint8_t enable_rtty_decode; // new rtty encoder (experimental)
	uint8_t cw_decoder_enable;
	bool cw_offset_shift_keep_signal; // experimental flag, shall we move shift by sidetone frequency to keep tuned signal?
	bool enable_ptt_rts; // disable/enable ptt via virtual serial port rts

	keyer_mode_t keyer_mode; // disable/enable keyer mode for F1-F5 buttons
	bool buffered_tx; // disable/enable buffered sending for CW and digital modes
#ifdef USE_TWO_CHANNEL_AUDIO
	bool stereo_enable; // enable/disable stereo demodulation (only in special hardware, NOT in mcHF)
#endif
#ifdef USE_LEAKY_LMS
	bool enable_leaky_LMS;
#endif
	float32_t nr_alpha; // alpha smoothing constant for spectral noise reduction
	int16_t nr_alpha_int;
	float32_t nr_beta; // beta smoothing constant for spectral noise reduction
	int16_t nr_beta_int;
//	float32_t nr_vad_thresh; // threshold for voice activity detector in spectral noise reduction
//	uint32_t nr_vad_thresh_int;
	bool nr_enable; // enable spectral noise reduction
	uint8_t nr_first_time; // set to 1 for initialization of the NR variables
//	bool nr_gain_smooth_enable; // enable gain smoothing
//	float32_t nr_gain_smooth_alpha; // smoothing constant for gain smoothing in the spectral noise reduction
//	int16_t nr_gain_smooth_alpha_int;
//	bool nr_long_tone_enable; // enable elimination of long tones in the spectral NR algorithm
//	float32_t nr_long_tone_alpha; // time constant for long tone detection
//	uint32_t nr_long_tone_alpha_int; // time constant for long tone detection
//	int16_t nr_long_tone_thresh; // threshold for long tone detection
//	bool nr_long_tone_reset; // used to reset gains of the long tone detection to 1.0
//	int16_t nr_vad_delay; // how many frames to delay the noise estimate after VAD has detected NOISE
//	int16_t nr_mode;
	bool nr_fft_256_enable; // debugging: enable FFT256 instead of FFT128 for spectral NR
	uint16_t NR_FFT_L; // resulting FFT length: 128 or 256
	uint8_t NR_FFT_LOOP_NO;
	bool NR_decimation_enable; // set to true, if we want to use another decimation step for the spectral NR leading to 6ksps sample rate
	uint8_t debug_si5351a_pllreset;
	uint16_t graticulePowerupYpos;	//initial (after powerup) position of graticule (frequency bar)
	const LcdLayout* Layout;				//current lcd layout (set by lcd detection routine)
	uint8_t FreqDisplayFont;		//0= old thin font, 1=new bold 8 bit (if available)
} TransceiverState;
//
extern __IO TransceiverState ts;

//DL2FW UGLY test for FREEDV - some globals :-(



//end DL2FW UGLY test for FREEDV - some globals :-(

#define	POWERDOWN_DELAY_COUNT	30	// Delay in main service loop for the "last second" before power-down - to allow EEPROM write to complete

//#define CODEC_USE_SPI

#define DEBUG_COM                        USART1

/**
 * @brief Introduces about 40ms of delay (load dependent, since interrupt eats some of the time.
 */
// TODO: Measure raw time for this loop

#define non_os_delay()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 1000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)


// ------------------------------------------------------------------
// Exports
typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON = 1,
    LED_STATE_TOGGLE = 2
} ledstate_t;

inline void Board_GreenLed(ledstate_t state)
{
    switch(state)
    {
    case LED_STATE_ON:
        GPIO_SetBits(GREEN_LED_PIO, GREEN_LED);
        break;
    case LED_STATE_OFF:
        GPIO_ResetBits(GREEN_LED_PIO, GREEN_LED);
        break;
    default:
        GPIO_ToggleBits(GREEN_LED_PIO, GREEN_LED);
        break;
    }
}

inline void Board_RedLed(ledstate_t state)
{
    switch(state)
    {
    case LED_STATE_ON:
        GPIO_SetBits(RED_LED_PIO, RED_LED);
        break;
    case LED_STATE_OFF:
        GPIO_ResetBits(RED_LED_PIO, RED_LED);
        break;
    default:
        GPIO_ToggleBits(RED_LED_PIO, RED_LED);
        break;
    }
}

#ifdef UI_BRD_OVI40
inline void Board_BlueLed(ledstate_t state)
{
    switch(state)
    {
    case LED_STATE_ON:
        GPIO_SetBits(BLUE_LED_PIO, BLUE_LED);
        break;
    case LED_STATE_OFF:
        GPIO_ResetBits(BLUE_LED_PIO, BLUE_LED);
        break;
    default:
        GPIO_ToggleBits(BLUE_LED_PIO, BLUE_LED);
        break;
    }
}
#endif
/**
 * @brief sets the hw ptt line and by this switches the mcHF board signal path between rx and tx configuration
 * @param tx_enable true == TX Paths, false == RX Paths
 */
inline void Board_EnableTXSignalPath(bool tx_enable)
{
    // to make switching as noiseless as possible, make sure the codec lineout is muted/produces zero output before switching
    if (tx_enable)
    {
        GPIO_SetBits(PTT_CNTR_PIO,PTT_CNTR);     // TX on and switch CODEC audio paths
        // Antenna Direction Output
        // BPF Direction Output (U1,U2)
        // PTT Optocoupler LED On (ACC Port) (U6)
        // QSD Mixer Output Disable (U15)
        // QSE Mixer Output Enable (U17)
        // Codec LineIn comes from mcHF LineIn Socket (U3)
        // Codec LineOut connected to QSE mixer (IQ Out) (U3a)
    }
    else
    {
        GPIO_ResetBits(PTT_CNTR_PIO,PTT_CNTR); // TX off
        // Antenna Direction Input
        // BPF Direction Input (U1,U2)
        // PTT Optocoupler LED Off (ACC Port) (U6)
        // QSD Mixer Output Enable (U15)
        // QSE Mixer Output Disable (U17)
        // Codec LineIn comes from RF Board QSD mixer (IQ In) (U3)
        // Codec LineOut disconnected from QSE mixer  (IQ Out) (U3a)
    }
}

/**
 * @brief set PA bias at the LM2931CDG (U18) using DAC Channel 2
 */
inline void Board_SetPaBiasValue(uint16_t bias)
{
    // Set DAC Channel 1 DHR12L register
    // DAC_SetChannel2Data(DAC_Align_8b_R,bias);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_8B_R, bias);

}

void Board_HandlePowerDown();

void Board_SelectLpfBpf(uint8_t group);

void Board_InitMinimal();
void Board_InitFull();
void Board_PostInit();
void Board_Reboot();
void Board_Powerdown();


/**
 * Is the hardware contact named DAH pressed
 */
inline bool Board_PttDahLinePressed() {
    return  !HAL_GPIO_ReadPin(PADDLE_DAH_PIO,PADDLE_DAH);
}

/**
 * Is the hardware contact named DIT pressed
 */
inline bool Board_DitLinePressed() {
    return  !HAL_GPIO_ReadPin(PADDLE_DIT_PIO,PADDLE_DIT);
}

unsigned int Board_RamSizeGet();
void Board_RamSizeDetection();
void Board_TouchscreenInit();

// in main.c
void CriticalError(ulong error);

bool is_vfo_b();

inline bool is_ssb_tx_filter_enabled() {
	return (ts.tx_filter != 0);
	//    return (ts.flags1 & FLAGS1_SSB_TX_FILTER_DISABLE) == false;
}

inline bool is_ssb(const uint32_t dmod_mode) {
    return (dmod_mode == DEMOD_LSB || dmod_mode == DEMOD_USB);
}

inline bool is_splitmode()
{
    return (ts.vfo_mem_mode & VFO_MEM_MODE_SPLIT) != 0;
}

inline bool is_scopemode()
{
    return (ts.flags1 & FLAGS1_SCOPE_ENABLED) != 0;
}

inline bool is_waterfallmode()
{
    return (ts.flags1 & FLAGS1_WFALL_ENABLED) != 0;
}

#ifdef USE_PENDSV_FOR_HIGHPRIO_TASKS
extern void UiDriver_TaskHandler_HighPrioTasks();
#endif

#endif
