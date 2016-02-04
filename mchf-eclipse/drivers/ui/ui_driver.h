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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __UI_DRIVER_H
#define __UI_DRIVER_H

#include "mchf_board.h"


// SI570 startup value (receive freq * 4)
//#define 	STARTUP_FREQ					112000000
#define 	STARTUP_FREQ					56000000

#define 	T_STEP_1HZ						1
#define 	T_STEP_10HZ						10
#define 	T_STEP_100HZ					100
#define 	T_STEP_1KHZ						1000
#define 	T_STEP_10KHZ					10000
#define 	T_STEP_100KHZ					100000
#define		T_STEP_1MHZ						1000000		// Used for transverter offset adjust
#define		T_STEP_10MHZ					10000000	// Used for transverter offset adjust
//
enum {
	T_STEP_1HZ_IDX = 0,
	T_STEP_10HZ_IDX,
	T_STEP_100HZ_IDX,
	T_STEP_1KHZ_IDX,
	T_STEP_10KHZ_IDX,
	T_STEP_100KHZ_IDX,
	T_STEP_1MHZ_IDX,
	T_STEP_10MHZ_IDX,
	T_STEP_MAX_STEPS
};

//#define 	MAX_STEPS						6

// S meter
#define 	S_METER_V_POS					55
#define 	S_METER_SIZE					9
#define 	S_VERTI_SIZE					10
#define 	S_LEFT_SHIFT					20
#define 	S_BLOCK_GAP		 				2
#define 	S_COLOR_INCR					1
#define 	S_HEIGHT_INCR					12

// Button Definitions
#define 	BUTTON_NONE						0
#define		BUTTON_PRESS_DEBOUNCE			75		// time to debounce normal button press
#define 	BUTTON_HOLD_TIME				1000	// time of press-and-hold
#define 	AUTO_BLINK_TIME					200000
#define		DEBOUNCE_TIME_MAX				50000	// ceiling time of debounce counter to prevent overflow

typedef struct KeypadState
{
	// Actual time the button is pressed
	ulong	debounce_time;

	// ID of button as GPIO bit
	ulong	button_id;

	// Flag to indicate click event
	bool	button_pressed;

	// Flag to indicate release event
	bool	button_released;

	// Flag to indicate under process
	bool	button_processed;

	// Flag to indicate that the button had been continued to be pressed during debounce
	bool	button_just_pressed;

	// Flag to indicate that debounce check was complete
	bool	debounce_check_complete;

	// Flag to indicate a "press-and-hold" condition
	bool	press_hold;

	uchar	dummy;

} KeypadState;

#define	PRESS_HOLD_RELEASE_DELAY_TIME	10		// time after button being released before resetting press-and-hold flag

#define S_METER_MAX							34

// Spectrum scope operational constants
//
#define	SPECTRUM_SCOPE_TOP_LIMIT			5	// Top limit of spectrum scope magnitude
//
#define SPECTRUM_SCOPE_AGC_THRESHOLD		2000//400	// AGC "Knee" above which output from spectrum scope FFT  AGC will cause action
#define SPECTRUM_SCOPE_MAX_FFT_VAL			8192//16384 // Value above which input to spectrum scope FFT will cause AGC action
#define SPECTRUM_SCOPE_MIN_GAIN				0.001//0.1	// Minimum gain for spectrum scope FFT AGC loop
#define SPECTRUM_SCOPE_MAX_GAIN				140	// Maximum gain for spectrum scope FFT AGC loop
#define	SPECTRUM_SCOPE_AGC_ATTACK			0.5//0.1	// Attack rate for spectrum scope FFT AGC/gain
#define	SPECTRUM_SCOPE_AGC_DECAY			0.166//0.1	// Decay rate for spectrum scope FFT AGC/gain
//
#define	SPECTRUM_SCOPE_LPF_FACTOR			4	// IIR Factor for spectrum scope low-pass filtering
	// Higher = slower response.  3 = 33% input; 66% feedback, 4 = 25% input, 75% feedback, 5 = 20% input, 80% feedback
#define	SPECTRUM_SCOPE_LPF_FACTOR_SPI		2	// IIR Factor for spectrum scope low-pass filtering using SPI display (update rate is slower, use faster filter)
//
#define PK_AVG_RESCALE_THRESH		3		// This sets the minimum peak-to-average ratio of the spectrum display before it starts to rescale the peak
		// value from the top.  This prevents it from going completely "white" on noise/no-signal conditions.
//
#define	SPECTRUM_SCOPE_RESCALE_ATTACK_RATE	0.1	// Rate at which scaling of spectrum scope adapts to strong signals within its passband
#define	SPECTRUM_SCOPE_RESCALE_DECAY_RATE	0.033	// Rate at which scaling of spectrum scope decays after strong signals disappear from its passband
//
#define SPECTRUM_SCOPE_SPEED_MIN			1	// minimum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_MAX			25	// maximum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_DEFAULT		5
//
#define SPECTRUM_SCOPE_FILTER_MIN			1	// minimum filter setting
#define	SPECTRUM_SCOPE_FILTER_MAX			10	// maximum filter setting
#define SPECTRUM_SCOPE_FILTER_DEFAULT		4	// default filter setting
//
#define	SPECTRUM_SCOPE_AGC_MIN				1	// minimum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_MAX				50	// maximum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_DEFAULT			25	// default spectrum scope AGC rate setting
//
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MIN		10
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MAX		30
#define	SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT		20
//
#define	SPECTRUM_SCOPE_ADJUST_OFFSET	100
//
//
#define	SPECTRUM_AGC_SCALING				25	// scaling factor by which the on-screen Spec. AGC Adj. is divided for adjustment.
//
#define	SCOPE_PREAMP_GAIN					1000//200	// amount of "amplification" in front of the FFT used for the Spectrum Scope and Waterfall used to overcome mathematical "noise floor"
//
#define INIT_SPEC_AGC_LEVEL					-80	// Initial offset for AGC level for spectrum/waterfall display
//
#define SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE	25	// time, in 100's of second to inhibit spectrum scope update after adjusting tuning while in SPI mode
//
#define	NUMBER_WATERFALL_COLOURS			64		// number of colors in the waterfall table
//
// Settings for dB/division for spectrum display
//
enum	{
	DB_DIV_DEFAULT = 0,
	DB_DIV_5,
	DB_DIV_7,
	DB_DIV_10,
	DB_DIV_15,
	DB_DIV_20,
	S_1_DIV,
	S_2_DIV,
	S_3_DIV
};
//
#define	DB_DIV_ADJUST_MIN	DB_DIV_DEFAULT
#define	DB_DIV_ADJUST_MAX	S_3_DIV
#define	DB_DIV_ADJUST_DEFAULT	DB_DIV_10
//
// scaling factors for the various dB/division settings
//
#define	DB_SCALING_5	63.2456		// 5dB/division scaling
#define	DB_SCALING_7	42.1637		// 7.5dB/division scaling
#define	DB_SCALING_10	31.6228		// 10dB/division scaling
#define	DB_SCALING_15	21.0819		// 15dB/division scaling
#define	DB_SCALING_20	15.8114		// 20dB/division scaling
#define	DB_SCALING_S1	52.7046		// 1 S unit (6 dB)/division scaling
#define DB_SCALING_S2	26.3523		// 2 S unit (12 dB)/division scaling
#define	DB_SCALING_S3	17.5682		// 3 S unit (18 dB)/division scaling
//
// Enumeration to select which waterfall palette to use
//
enum	{
	WFALL_GRAY = 0,
	WFALL_HOT_COLD,
	WFALL_RAINBOW,
	WFALL_BLUE,
	WFALL_GRAY_INVERSE,
	WFALL_MAXVAL
};
//
#define	WATERFALL_COLOR_MIN			0
#define WATERFALL_COLOR_MAX			WFALL_MAXVAL
#define	WATERFALL_COLOR_DEFAULT		WFALL_GRAY
//
#define	WATERFALL_STEP_SIZE_MIN	1
#define	WATERFALL_STEP_SIZE_MAX	5
#define	WATERFALL_STEP_SIZE_DEFAULT	2
//
#define	WATERFALL_OFFSET_MIN	60
#define	WATERFALL_OFFSET_MAX	140
#define	WATERFALL_OFFSET_DEFAULT	100
//
#define	WATERFALL_CONTRAST_MIN	10
#define	WATERFALL_CONTRAST_MAX	225
#define	WATERFALL_CONTRAST_DEFAULT	120
//
#define	WATERFALL_SPEED_MIN		1
#define	WATERFALL_SPEED_MAX		30
#define	WATERFALL_SPEED_DEFAULT_PARALLEL	10
//
#define WATERFALL_NOSIG_ADJUST_MIN		10
#define WATERFALL_NOSIG_ADJUST_MAX		30
#define	WATERFALL_NOSIG_ADJUST_DEFAULT	20
//
// The following include warnings and settings for SPI interfaces, which needs less frequent updates or else the screen update will make button/dial response very sluggish!
//
#define WATERFALL_SPEED_DEFAULT_SPI	15
//
#define	WATERFALL_SPEED_WARN_SPI		10
#define	WATERFALL_SPEED_WARN1_SPI		14
//
// "faster" than this can make knobs/buttons sluggish with parallel
//
#define	WATERFALL_SPEED_WARN_PARALLEL	5
#define WATERFALL_SPEED_WARN1_PARALLEL  9
//
// Constants for waterfall size settings
//
enum	{
	WATERFALL_NORMAL=0,
	WATERFALL_MEDIUM,
	WATERFALL_MAX
};
//
enum	{
	FFT_WINDOW_RECTANGULAR=0,
	FFT_WINDOW_COSINE,
	FFT_WINDOW_BARTLETT,
	FFT_WINDOW_WELCH,
	FFT_WINDOW_HANN,
	FFT_WINDOW_HAMMING,
	FFT_WINDOW_BLACKMAN,
	FFT_WINDOW_NUTTALL,
	FFT_WINDOW_MAX
};
//
#define	FFT_WINDOW_DEFAULT	FFT_WINDOW_BLACKMAN
//
#define	WATERFALL_SIZE_DEFAULT	WATERFALL_NORMAL
#define	WFALL_MEDIUM_ADDITIONAL	12					// additional vertical height in pixels of "medium" waterfall
//
#define SWR_SAMPLES_SKP						1	//5000
#define SWR_SAMPLES_CNT						5//10
//
#define	SD_DB_DIV_SCALING					0.0316	// Scaling factor for number of dB/Division	0.0316 = 10dB/Division

// SWR and RF power meter public
typedef struct SWRMeter
{
	ulong	skip;

	float fwd_calc;			// forward power readings in A/D units
	float rev_calc;			// reverse power readings in A/D units
	float fwd_pwr;			// forward power in watts
	float rev_pwr;			// reverse power in watts
	float fwd_dbm;			// forward power in dBm
	float rev_dbm;			// reverse power in dBm
	float vswr;				// vswr
	float vswr_dampened;	// dampened VSWR reading
	bool  pwr_meter_disp;	// TRUE if numerical FWD/REV power metering (in milliwatts) is to be displayed
	bool  pwr_meter_was_disp;	// TRUE if numerical FWD/REV power metering WAS displayed (used to clear it)
	uchar	p_curr;			// count used to update power meter
	uchar	sensor_null;	// used to null out the sensor offset voltage
	uchar	coupling_2200m_calc;	// coupling coefficient for forward and reverse couplers for 2200 meters
	uchar	coupling_630m_calc;	// coupling coefficient for forward and reverse couplers for 630 meters
	uchar	coupling_160m_calc;	// coupling coefficient for forward and reverse couplers for 160 meters
	uchar	coupling_80m_calc;	// coupling coefficient for forward and reverse couplers for 80 meters
	uchar	coupling_40m_calc;	// coupling coefficient for forward and reverse couplers for 40/60 meters
	uchar	coupling_20m_calc;	// coupling coefficient for forward and reverse couplers for 30/20 meters
	uchar	coupling_15m_calc;	// coupling coefficient for forward and reverse couplers for 17/15/12/10 meters
	uchar	coupling_6m_calc;	// coupling coefficient for forward and reverse couplers for 6 meters
	uchar	coupling_2m_calc;	// coupling coefficient for forward and reverse couplers for 2 meters
	uchar	coupling_70cm_calc;	// coupling coefficient for forward and reverse couplers for 70 centimeters
	uchar	coupling_23cm_calc;	// coupling coefficient for forward and reverse couplers for 23 centimeters

} SWRMeter;

#define	SWR_CAL_MIN					75
#define	SWR_CAL_MAX					150
#define	SWR_CAL_DEFAULT					100
//
#define	SENSOR_NULL_MIN					75
#define	SENSOR_NULL_MAX					125
#define	SENSOR_NULL_DEFAULT				100
//
#define	FILTER_BAND_2200				11
#define	FILTER_BAND_630					12
#define	FILTER_BAND_160					5
#define	FILTER_BAND_80					1
#define	FILTER_BAND_40					2
#define	FILTER_BAND_20					3
#define FILTER_BAND_15					4
#define	FILTER_BAND_6					6
#define	FILTER_BAND_4					7
#define	FILTER_BAND_2					8
#define	FILTER_BAND_70					9
#define	FILTER_BAND_23					10
//
// Location of numerical FWD/REV power indicator
//
#define	POS_PWR_NUM_IND_X				1
#define	POS_PWR_NUM_IND_Y				80
//
#define	PWR_DAMPENING_FACTOR				0.10		// dampening/averaging factor (e.g. amount of "new" reading each time) - for numerical power reading ONLY
//
// Coupling adjustment limits
//
#define	SWR_COUPLING_MIN				50
#define	SWR_COUPLING_MAX				150
#define	SWR_COUPLING_DEFAULT				100
//
#define	SWR_ADC_FULL_SCALE				4095	// full scale of A/D converter (4095 = 10 bits)
#define	SWR_ADC_VOLT_REFERENCE				3.3		// NOMINAL A/D reference voltage.  The PRECISE value is calibrated by a menu item!  (Probably "FWD/REV ADC Cal.")
//
// coefficients for very low power (<75 milliwatt) power levels.  Do NOT use this above approx. 0.07 volts input!
//
#define	LOW_RF_PWR_COEFF_A				-0.0338205168744131		// constant (offset)
#define	LOW_RF_PWR_COEFF_B				5.02584652062682		// "b" coefficient (for x)
#define LOW_RF_PWR_COEFF_C				-106.610490958242		// "c" coefficient (for x^2)
#define	LOW_RF_PWR_COEFF_D				853.156505329744		// "d" coefficient (for x^3)
//
// coefficients for higher power levels (>50 milliwatts).  This is actually good down to 25 milliwatts or so.
//
#define	HIGH_RF_PWR_COEFF_A				0.01209	//0.0120972709513557		// constant (offset)
#define HIGH_RF_PWR_COEFF_B				0.8334	//0.833438917330908		// "b" coefficient (for x)
#define HIGH_RF_PWR_COEFF_C 				1.569	//1.56930042559198		// "c" coefficient (for x^2)
//
//
#define	SWR_MIN_CALC_POWER				0.25	// Minimum forward power required for SWR calculation
//
#define	LOW_POWER_CALC_THRESHOLD			0.05	// voltage from sensor below which we use the "low power" calculations, above
//
#define	VSWR_DAMPENING_FACTOR				0.25		// dampening/averaging factor (e.g. amount of "new" reading each time) - for VSWR meter indication ONLY
//
#define MAX_VSWR_MOD_VALUE				75				// Maximum A/D value from FWD/REV power sensors before warning is displayed about not having done resistor modification
//
// Volt (DC power) meter
//
#define POWER_SAMPLES_SKP				10	//1500
#define POWER_SAMPLES_CNT				32
//
// used to limit the voltmeter calibration parameters
//
#define	POWER_VOLTMETER_CALIBRATE_DEFAULT		100
#define	POWER_VOLTMETER_CALIBRATE_MIN			00
#define	POWER_VOLTMETER_CALIBRATE_MAX			200
//
#define	VOLTMETER_ADC_FULL_SCALE		4095
//
//
// Power supply
typedef struct PowerMeter
{
	ulong	skip;

	ulong	pwr_aver;
	uchar	p_curr;

	uchar	v10;
	uchar	v100;
	uchar	v1000;
	uchar	v10000;

} PowerMeter;

#define LO_COMP_SKP					50		//50000

// LO temperature compensation
typedef struct LoTcxo
{
	ulong	skip;

	// Current compensation value
	// loaded to LO
	int		comp;

	uchar	sensor_present;

	uchar	v1000;
	uchar	v10000;
	uchar	v100000;
	uchar	v1000000;

} LoTcxo;

// Once every 25s - 0xFFFFF
#define EEPROM_SAVE_SKP					0xFFFFF

// Eeprom saving routine
typedef struct EepromSave
{
	ulong	skip;

} EepromSave;

// --------------------------------------------------------------------------
// Controls positions and some related colours
// --------------------
#define SMALL_FONT_WIDTH				8
#define LARGE_FONT_WIDTH				16

// Frequency display control
#define POS_TUNE_FREQ_X					116
#define POS_TUNE_FREQ_Y					100
//
#define	POS_TUNE_SPLIT_FREQ_X				POS_TUNE_FREQ_X+72
#define	POS_TUNE_SPLIT_MARKER_X				POS_TUNE_FREQ_X+40
#define	POS_TUNE_SPLIT_FREQ_Y_TX			POS_TUNE_FREQ_Y+12

//
#define	SPLIT_ACTIVE_COLOUR		Yellow		// colour of "SPLIT" indicator when active
#define	SPLIT_INACTIVE_COLOUR	Grey		// colour of "SPLIT" indicator when NOT active

// Second frequency display control
#define POS_TUNE_SFREQ_X				(POS_TUNE_FREQ_X + 120)
#define POS_TUNE_SFREQ_Y				(POS_TUNE_FREQ_Y - 20)

// Band selection control
#define POS_BAND_MODE_X					(POS_TUNE_FREQ_X + 160)
#define POS_BAND_MODE_Y					(POS_TUNE_FREQ_Y + 7)
#define POS_BAND_MODE_MASK_X				(POS_BAND_MODE_X - 1)
#define POS_BAND_MODE_MASK_Y				(POS_BAND_MODE_Y - 1)
#define POS_BAND_MODE_MASK_H				13
#define POS_BAND_MODE_MASK_W				33

// Demodulator mode control
#define POS_DEMOD_MODE_X					(POS_TUNE_FREQ_X + 1)
#define POS_DEMOD_MODE_Y					(POS_TUNE_FREQ_Y - 20)
#define POS_DEMOD_MODE_MASK_X				(POS_DEMOD_MODE_X - 1)
#define POS_DEMOD_MODE_MASK_Y				(POS_DEMOD_MODE_Y - 1)
#define POS_DEMOD_MODE_MASK_H				13
#define POS_DEMOD_MODE_MASK_W				41

// Tunning step control
#define POS_TUNE_STEP_X						(POS_TUNE_FREQ_X + 50)
#define POS_TUNE_STEP_Y						(POS_TUNE_FREQ_Y - 20)
#define POS_TUNE_STEP_MASK_X				(POS_TUNE_STEP_X - 1)
#define POS_TUNE_STEP_MASK_Y				(POS_TUNE_STEP_Y - 1)
#define POS_TUNE_STEP_MASK_H				17
#define POS_TUNE_STEP_MASK_W				49

#define POS_RADIO_MODE_X					4
#define POS_RADIO_MODE_Y					5

// Bottom bar
#define POS_BOTTOM_BAR_X					0
#define POS_BOTTOM_BAR_Y					228
#define POS_BOTTOM_BAR_BUTTON_W				62
#define POS_BOTTOM_BAR_BUTTON_H				16

// Virtual Button 1
#define POS_BOTTOM_BAR_F1_X					(POS_BOTTOM_BAR_X +                              2)
#define POS_BOTTOM_BAR_F1_Y					POS_BOTTOM_BAR_Y

// Virtual Button 2
#define POS_BOTTOM_BAR_F2_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*1 +  4)
#define POS_BOTTOM_BAR_F2_Y					POS_BOTTOM_BAR_Y

// Virtual Button 3
#define POS_BOTTOM_BAR_F3_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*2 +  6)
#define POS_BOTTOM_BAR_F3_Y					POS_BOTTOM_BAR_Y

// Virtual Button 4
#define POS_BOTTOM_BAR_F4_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*3 +  8)
#define POS_BOTTOM_BAR_F4_Y					POS_BOTTOM_BAR_Y

// Virtual Button 5
#define POS_BOTTOM_BAR_F5_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*4 + 10)
#define POS_BOTTOM_BAR_F5_Y					POS_BOTTOM_BAR_Y

// --------------------------------------------------
// Encoder one controls indicator
// audio gain
#define POS_AG_IND_X						0
#define POS_AG_IND_Y						27
// sidetone gain
#define POS_SG_IND_X						60
#define POS_SG_IND_Y						27

// --------------------------------------------------
// Encoder two controls indicator
// RF Gain indicator
#define POS_RF_IND_X						0
#define POS_RF_IND_Y						43
// RF attenuator
#define POS_RA_IND_X						60
#define POS_RA_IND_Y						43

// --------------------------------------------------
// Encoder three controls indicator
// RIT indicator
#define POS_RIT_IND_X						0
#define POS_RIT_IND_Y						59
// keyer speed
#define POS_KS_IND_X						60
#define POS_KS_IND_Y						59

// --------------------------------------------------
// Calibration mode
//
// PA bias
#define POS_PB_IND_X						0
#define POS_PB_IND_Y						78
// IQ gain balance
#define POS_BG_IND_X						0
#define POS_BG_IND_Y						94
// IQ phase balance
#define POS_BP_IND_X						0
#define POS_BP_IND_Y						110
// Frequency Calibrate
#define POS_FC_IND_X						0
#define POS_FC_IND_Y						78

// --------------------------------------------------
// Standalone controls
//
// DSP mode
// Upper DSP box
#define POS_DSPU_IND_X						0
#define POS_DSPU_IND_Y						116
// Lower DSP box
#define POS_DSPL_IND_X						0
#define POS_DSPL_IND_Y						131

// Power level
#define POS_PW_IND_X						0
#define POS_PW_IND_Y						147
// Filter indicator
#define POS_FIR_IND_X						0
#define POS_FIR_IND_Y						163

// Spectrum display
#define POS_SPECTRUM_IND_X					60
#define POS_SPECTRUM_IND_Y					150
#define POS_SPECTRUM_IND_H					80
#define	POS_SPECTRUM_FREQ_BAR_Y				64	// reducing value moves upwards
#define	POS_SPECTRUM_FILTER_WIDTH_BAR_Y		61
#define POS_SPECTRUM_IND_W					258
#define COL_SPECTRUM_GRAD					0x40

// ETH position
#define POS_ETH_IND_X						220
#define POS_ETH_IND_Y						2

// USB Keyboard position
#define POS_KBD_IND_X						220
#define POS_KBD_IND_Y						18

// S meter position
#define POS_SM_IND_X						116
#define POS_SM_IND_Y						0

// Supply Voltage indicator
#define POS_PWRN_IND_X						0
#define POS_PWRN_IND_Y						193
#define POS_PWR_IND_X						5
#define POS_PWR_IND_Y						(POS_PWRN_IND_Y + 15)
#define COL_PWR_IND							Grey2

#define POS_TEMP_IND_X						0
#define POS_TEMP_IND_Y						0
//
// Starting position of configuration menu
//
#define	POS_MENU_IND_X						60		// X position of description of menu item being changed
#define	POS_MENU_IND_Y						128		// Y position of first (top) item being changed
#define	POS_MENU_CHANGE_X					244		// Position of variable being changed
#define	POS_MENU_CURSOR_X					311		// Position of cursor used to indicate selected item
//
// --------------------------------------------------------------------------
// Exports
void 	ui_driver_init(void);
void 	ui_driver_thread(void);
//void 	ui_driver_irq(void);
void 	ui_driver_toggle_tx(void);
void 	UiCalcRxPhaseAdj(void);
void 	UiCalcTxPhaseAdj(void);
void 	UiDriverLoadFilterValue(void);
void 	UiDriverClearSpectrumDisplay(void);
//
void 	UiDriverChangeBandFilter(uchar band,uchar bpf_only);
void 	UiDriverChangeFilter(uchar ui_only_update);
void 	UiDriverCreateTemperatureDisplay(uchar enabled,uchar create);
void 	UiDriverUpdateFrequency(char skip_encoder_check, uchar mode);
void 	UiDriverCreateSpectrumScope(void);
void 	UiDriverUpdateFrequencyFast(void);
void	UiCalcRxIqGainAdj(void);
void	UiCalcTxIqGainAdj(void);
void 	UiDriverSetBandPowerFactor(uchar band);
void 	UiDrawSpectrumScopeFrequencyBarText(void);
void 	UiCheckForEEPROMLoadDefaultRequest(void);
//
void 	UiDriverChangeBandFilter(uchar band,uchar bpf_only);
void 	UiDriverChangeFilter(uchar ui_only_update);
void 	UiDriverSetBandPowerFactor(uchar band);
void	UiCalcRxIqGainAdj(void);
void	UiCalcTxIqGainAdj(void);
//
void 	UiDriverChangeStGain(uchar enabled);
void 	UiDriverChangeCmpLevel(uchar enabled);
void 	UiDriverChangeKeyerSpeed(uchar enabled);
void	UiDriverChangeAudioGain(uchar enabled);
void 	UiDriverChangeRfGain(uchar enabled);
//
void 	UIDriverChangeAudioGain(uchar enabled);
//
void 	UiDriverShowStep(ulong step);
//
void 	UiCalcTxCompLevel(void);
void 	UiCalcNB_AGC(void);
void 	UiCWSidebandMode(void);
void 	UiDriverDisplayFilterBW(void);
void 	UiDriverShowMode(void);
void 	UiCalcAGCVals(void);
void 	UiCalcRFGain(void);
void 	UiCalcALCDecay(void);
void 	UiCalcAGCDecay(void);
//
void	UiLCDBlankTiming(void);
void 	UiDriverChangeTuningStep(uchar is_up);
//
void 	uiCodecMute(uchar val);
//
void 	UiDriverSaveEepromValuesPowerDown(void);
void	UiCheckForEEPROMLoadFreqModeDefaultRequest(void);
void	UiCheckForPressedKey(void);
void 	UiKeyBeep(void);
void	UiInitRxParms(void);
//
void 	UiCalcSubaudibleFreq(void);
void 	UiLoadToneBurstMode(void);
void	UiCalcSubaudibleGenFreq(void);		// load/set current FM subaudible tone settings for generation
//
void 	UiCalcSubaudibleDetFreq(void);		// load/set current FM subaudible tone settings	for detection
bool	check_tp_coordinates(uint8_t,uint8_t,uint8_t,uint8_t);

void UiDriverSetDemodMode(uint32_t new_mode); // switch to different demodulation mode.
//
//
#define	SIDETONE_MAX_GAIN	10		// Maximum sidetone gain
#define	DEFAULT_SIDETONE_GAIN	5	// Default sidetone gain
//
#define	MIN_KEYER_SPEED		5		// Minimum keyer speed
#define	MAX_KEYER_SPEED		48		// Maximum keyer speed
#define DEFAULT_KEYER_SPEED	20		// Default keyer speed
//
#define	CW_OFFSET_USB_TX	0		// CW in USB mode, display is TX frequency if received frequency was zero-beated
#define	CW_OFFSET_LSB_TX	1		// CW in LSB mode, display is TX frequency if received frequency was zero-beated
#define	CW_OFFSET_AUTO_TX	2		// Same as CW_OFFSET_USB_TX except LSB if frequency is < 10 MHz, USB if >= 10 MHz
#define	CW_OFFSET_USB_RX	3		// CW in USB mode, display is RX frequency if received signal is matched to sidetone
#define	CW_OFFSET_LSB_RX	4		// CW in LSB mode, display is RX frequency if received signal is matched to sidetone
#define CW_OFFSET_AUTO_RX	5		// Same as CW_OFFSET_USB_RX except LSB if frequency is < 10 MHz, USB if >= 10 MHz
#define CW_OFFSET_USB_SHIFT 6		// CW in USB mode, LO shifts, display is RX frequency if signal is matched to sidetone
#define	CW_OFFSET_LSB_SHIFT 7		// CW in LSB mode, LO shifts, display is RX frequency if signal is matched to sidetone
#define	CW_OFFSET_AUTO_SHIFT	8	// Same as "CW_OFFSET_USB_SHIFT" except LSB if frequency is <10 MHz, USB of >= 10 MHz
#define	CW_OFFSET_MAX		8		// Maximum menu setting
#define	CW_OFFSET_MODE_DEFAULT	0	// Default CW offset setting
//
#define	USB_FREQ_THRESHOLD	40000000	// LO frequency at and above which the default is USB, Hz*4  (e.g. 10 MHz = 40 MHz)
//
#define	MAX_RF_ATTEN		15		// Maximum setting for RF attenuation
//
#define	MIN_RIT_VALUE		-50		// Minimum RIT Value
#define	MAX_RIT_VALUE		50		// Maximum RIT Value
//
#define	MAX_RF_GAIN			50		// Maximum RF gain setting
#define	DEFAULT_RF_GAIN		50		// Default RF gain setting
//
#define	MAX_RF_CODEC_GAIN_VAL		9		// Maximum RF gain setting
#define	DEFAULT_RF_CODEC_GAIN_VAL	9		// Default RF gain setting (9 = AUTO mode)
//
#define	MAX_AUDIO_GAIN		30		// Maximum audio gain setting
#define	DEFAULT_AUDIO_GAIN	16		// Default audio gain
//
// The following are used in the max volume setting in the menu system
//
#define	MAX_VOLUME_MIN				8		// Minimum setting for maximum volume
#define	MAX_VOLUME_MAX				MAX_AUDIO_GAIN		// Maximum setting for maximum volume
#define	MAX_VOLUME_DEFAULT			DEFAULT_AUDIO_GAIN
//
#define	MAX_VOL_RED_THRESH	10		// "MAX VOLUME" setting at or below which number will be RED to warn user
#define	MAX_VOLT_YELLOW_THRESH	16	// "MAX VOLUME" setting at or below which number will be YELLOW to warn user
//
//
#define	MAX_PA_BIAS			115		// Maximum PA Bias Setting
#define	DEFAULT_PA_BIAS		0		// Default PA Bias setting
#define	MIN_BIAS_SETTING	20		// Minimum bias setting.  (Below this, number is red)
//
#define	BIAS_OFFSET			25		// Offset value to be added to bias setting
//  DA value = (OFFSET + (2*setting))  where DA value is 0-255
//
#define	MIN_TX_IQ_GAIN_BALANCE	-99	// Minimum setting for TX IQ gain balance
#define MAX_TX_IQ_GAIN_BALANCE	99	// Maximum setting for TX IQ gain balance
//
#define	MIN_RX_IQ_GAIN_BALANCE	-99	// Minimum setting for RX IQ gain balance
#define	MAX_RX_IQ_GAIN_BALANCE	99	// Maximum setting for RX IQ gain balance
//
#define	MIN_TX_IQ_PHASE_BALANCE	-32	// Minimum setting for TX IQ phase balance
#define	MAX_TX_IQ_PHASE_BALANCE	32	// Maximum setting for TX IQ phase balance
//
#define	MIN_RX_IQ_PHASE_BALANCE	-32	// Minimum setting for RX IQ phase balance
#define	MAX_RX_IQ_PHASE_BALANCE	32	// Maximum setting for RX IQ phase balance
//
#define	XVERTER_MULT_MAX		10		// maximum LO multipler in xverter mode
#define	XVERTER_OFFSET_MAX		999000000	// Maximum transverter offset (999 MHz)
//
#define	AUTO_LSB_USB_OFF		0
#define	AUTO_LSB_USB_ON			1
#define	AUTO_LSB_USB_60M		2
#define	AUTO_LSB_USB_MAX		2
#define	AUTO_LSB_USB_DEFAULT	AUTO_LSB_USB_60M
//
// Items that are timed using ts.sysclock (operates at 100 Hz)
//
#define	DSP_STARTUP_DELAY		350		// Delay, in 100ths of seconds, after startup, before allowing DSP NR or Notch to be enabled.
#define	DSP_REENABLE_DELAY		13		// Delay, in 100ths of seconds, after return to RX before allowing DSP NR or Notch to be re-enabled
#define	DSP_BAND_CHANGE_DELAY	100		// Delay, in 100ths of a second, after changing bands before restoring DSP NR
//
#define TUNING_LARGE_STEP_MUTING_TIME_DSP_ON	5	// Delay, in 100ths of a second, for audio to be muted after a "large" frequency step (to avoid audio "POP") when DSP on
#define TUNING_LARGE_STEP_MUTING_TIME_DSP_OFF	3	// Delay, in 100ths of a second, for audio to be muted after a "large" frequency step (to avoid audio "POP") when DSP on
#define	RX_MUTE_START_DELAY	(DSP_STARTUP_DELAY + TUNING_LARGE_STEP_MUTING_TIME_DSP_ON)	// establish earliest time after system start before audio muting will work
//
#define	THREAD_TIMING_DELAY		1				// Delay, in 100ths of a second, between thread tasks
//
#define	TX_PTT_AUDIO_MUTE_DELAY_MAX	25			// Maximum delay, in 100ths of a second, that audio will be muted after PTT (key-up) to prevent "clicks" and "clunks"
//
// Button definitions
//
enum {
BUTTON_M2_PRESSED = 0,	// 0
BUTTON_G3_PRESSED,	// 1
BUTTON_G2_PRESSED,	// 2
BUTTON_BNDM_PRESSED,	// 3
BUTTON_G4_PRESSED,	// 4
BUTTON_M3_PRESSED,	// 5
BUTTON_STEPM_PRESSED,	// 6
BUTTON_STEPP_PRESSED,	// 7
BUTTON_M1_PRESSED,	// 8
BUTTON_F3_PRESSED,	// 9 - Press and release handled in UiDriverProcessFunctionKeyClick()
BUTTON_F1_PRESSED,	// 10 - Press and release handled in UiDriverProcessFunctionKeyClick()
BUTTON_F2_PRESSED,	// 11 - Press and release handled in UiDriverProcessFunctionKeyClick()
BUTTON_F4_PRESSED,	// 12 - Press and release handled in UiDriverProcessFunctionKeyClick()
BUTTON_BNDP_PRESSED,	// 13
BUTTON_F5_PRESSED,	// 14 - Press and release handled in UiDriverProcessFunctionKeyClick()
BUTTON_G1_PRESSED,	// 15
BUTTON_POWER_PRESSED,	// 16 - Used for press and release
TOUCHSCREEN_ACTIVE	// 17 - Touchscreen touched
};
//
// UI Driver State machine definitions
enum {
//STATE_SPECTRUM_DISPLAY = 0,	//
STATE_S_METER = 0,				//
STATE_SWR_METER,				//
STATE_HANDLE_POWERSUPPLY,		//
STATE_LO_TEMPERATURE,			//
STATE_TASK_CHECK,				//
STATE_CHECK_ENC_ONE,			//
STATE_CHECK_ENC_TWO,			//
STATE_CHECK_ENC_THREE,			//
STATE_UPDATE_FREQUENCY,			//
STATE_PROCESS_KEYBOARD,			//
STATE_SWITCH_OFF_PTT			//
};
//
// Used for press-and-hold "temporary" step size adjust
//
#define	STEP_PRESS_OFF		0
#define	STEP_PRESS_MINUS	1
#define	STEP_PRESS_PLUS	2
//
#endif
