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

#ifndef __UI_DRIVER_H
#define __UI_DRIVER_H

#include "uhsdr_board.h"

// used by UpdateFrequency* family of functions
enum UpdateFrequencyMode_t
{
    UFM_AUTOMATIC = 0,
    UFM_LARGE,
    UFM_SMALL_RX,
    UFM_SMALL_TX,
    UFM_SECONDARY

};


// SI570 startup value (receive freq * 4)
//#define 	STARTUP_FREQ					112000000
#define 	STARTUP_FREQ					56000000

#define 	T_STEP_1HZ						1
#define 	T_STEP_10HZ						10
#define 	T_STEP_100HZ					100
#define 	T_STEP_500HZ					500
#define 	T_STEP_1KHZ						1000
#define 	T_STEP_5KHZ						5000
#define 	T_STEP_9KHZ						9000
#define 	T_STEP_10KHZ					10000
#define 	T_STEP_100KHZ					100000
#define		T_STEP_1MHZ						1000000		// Used for transverter offset adjust
#define		T_STEP_10MHZ					10000000	// Used for transverter offset adjust
//
enum
{
    T_STEP_1HZ_IDX = 0,
    T_STEP_10HZ_IDX,
    T_STEP_100HZ_IDX,
    T_STEP_500HZ_IDX,
    T_STEP_1KHZ_IDX,
    T_STEP_5KHZ_IDX,
    T_STEP_9KHZ_IDX,
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
#define 	S_BLOCK_GAP						2
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

#define S_METER_MAX						34


//
// --------------------------------------------------------------------------
// Controls positions and some related colours
// --------------------

// Frequency display control
#define POS_TUNE_FREQ_X             		116
#define POS_TUNE_FREQ_Y             		100
//
#define POS_TUNE_SPLIT_FREQ_X           	POS_TUNE_FREQ_X+80//Shift with a small split to the right to close the frequency digits.
#define POS_TUNE_SPLIT_MARKER_X         	POS_TUNE_FREQ_X+40
#define POS_TUNE_SPLIT_FREQ_Y_TX        	POS_TUNE_FREQ_Y+12

//
#define SPLIT_ACTIVE_COLOUR         		Yellow      // colour of "SPLIT" indicator when active
#define SPLIT_INACTIVE_COLOUR           	Grey        // colour of "SPLIT" indicator when NOT active

// Second frequency display control
#define POS_TUNE_SFREQ_X            		(POS_TUNE_FREQ_X + 120)
#define POS_TUNE_SFREQ_Y            		(POS_TUNE_FREQ_Y - 20)

// Band selection control
#define POS_BAND_MODE_X             		(POS_TUNE_FREQ_X + 160)
#define POS_BAND_MODE_Y             		(POS_TUNE_FREQ_Y + 7)
#define POS_BAND_MODE_MASK_X            	(POS_BAND_MODE_X - 1)
#define POS_BAND_MODE_MASK_Y            	(POS_BAND_MODE_Y - 1)
#define POS_BAND_MODE_MASK_H            	13
#define POS_BAND_MODE_MASK_W            	33

// Demodulator mode control
#define POS_DEMOD_MODE_X            		(POS_TUNE_FREQ_X + 1)
#define POS_DEMOD_MODE_Y            		(POS_TUNE_FREQ_Y - 20)
#define POS_DEMOD_MODE_MASK_X           	(POS_DEMOD_MODE_X - 1)
#define POS_DEMOD_MODE_MASK_Y           	(POS_DEMOD_MODE_Y - 1)
#define POS_DEMOD_MODE_MASK_H           	13
#define POS_DEMOD_MODE_MASK_W           	41

// Tunning step control
#define POS_TUNE_STEP_X             		(POS_TUNE_FREQ_X + 45)
#define POS_TUNE_STEP_Y             		(POS_TUNE_FREQ_Y - 21)
#define POS_TUNE_STEP_MASK_H            	15
#define POS_TUNE_STEP_MASK_W            	(SMALL_FONT_WIDTH*7)

#define POS_RADIO_MODE_X            		4
#define POS_RADIO_MODE_Y            		5

// Bottom bar
#define POS_BOTTOM_BAR_X            		0
#define POS_BOTTOM_BAR_Y            		228
#define POS_BOTTOM_BAR_BUTTON_W         	62
#define POS_BOTTOM_BAR_BUTTON_H         	16

// Virtual Button 1
#define POS_BOTTOM_BAR_F1_X         		(POS_BOTTOM_BAR_X + 2)
#define POS_BOTTOM_BAR_F1_Y         		POS_BOTTOM_BAR_Y

// --------------------------------------------------
// Encoder controls indicator
#define POS_ENCODER_IND_X                	0
#define POS_ENCODER_IND_Y                	16


#define LEFTBOX_WIDTH 58 // used for the lower left side controls
// --------------------------------------------------
// Standalone controls
//
// DSP mode
// Lower DSP box
#define POS_LEFTBOXES_IND_X              	0
#define POS_LEFTBOXES_IND_Y              	130

// Power level
#define POS_PW_IND_X                		POS_DEMOD_MODE_X -1
#define POS_PW_IND_Y                		POS_DEMOD_MODE_Y - 16

#define POS_DIGMODE_IND_X              		0
#define POS_DIGMODE_IND_Y              		(163 + 16+12)

// S meter position
#define POS_SM_IND_X                		116
#define POS_SM_IND_Y                		0

// Supply Voltage indicator
#define POS_PWRN_IND_X              		0
#define POS_PWRN_IND_Y              		193

#define POS_PWR_IND_X               		4
#define POS_PWR_IND_Y               		(POS_PWRN_IND_Y + 15)
#define COL_PWR_IND                 		White

// Temperature Indicator
#define POS_TEMP_IND_X              		0
#define POS_TEMP_IND_Y              		0

// RTC
#define POS_RTC								79

#define POS_LOADANDDEBUG					95


//
// Location of numerical FWD/REV power indicator
//
#define	POS_PWR_NUM_IND_X					1
#define	POS_PWR_NUM_IND_Y					80
//



#define	PWR_DAMPENING_FACTOR				0.10		// dampening/averaging factor (e.g. amount of "new" reading each time) - for numerical power reading ONLY
//
// Coupling adjustment limits
//
#define	SWR_COUPLING_MIN					50
#define	SWR_COUPLING_MAX					150
#define	SWR_COUPLING_DEFAULT				100
//
//
//
#define	SWR_MIN_CALC_POWER					0.25	// Minimum forward power required for SWR calculation
//
//
#define	VSWR_DAMPENING_FACTOR				0.25		// dampening/averaging factor (e.g. amount of "new" reading each time) - for VSWR meter indication ONLY
//
#define MAX_VSWR_MOD_VALUE					75				// Maximum A/D value from FWD/REV power sensors before warning is displayed about not having done resistor modification
//
// Volt (DC power) meter
//
#define POWER_SAMPLES_CNT					32
//

//
#define	VOLTMETER_ADC_FULL_SCALE			4095
//
//
// Power supply
typedef struct PowerMeter
{
    uint32_t    pwr_aver;
    uint32_t    p_curr;

    uint32_t    voltage;

    bool        undervoltage_detected;
} PowerMeter;



//
// --------------------------------------------------------------------------
// Exports
void 	UiDriver_Init(void);
void 	UiDriver_MainHandler(void);

void 	UiDriver_CreateTemperatureDisplay();
void 	UiDriver_UpdateFrequency(bool force_update, enum UpdateFrequencyMode_t mode);
void    UiDriver_FrequencyUpdateLOandDisplay(bool full_update);
void    UiDriver_RefreshEncoderDisplay();
void    UiDriver_FButtonLabel(uint8_t button_num, const char* label, uint32_t label_color) ;
void 	UiDriver_ShowStep();
void 	UiDriver_ShowMode(void);
void	UiDriver_LcdBlankingStartTimer(void);
void    UiDriver_SpectrumZoomChangeLevel();

void	UiDriver_ShowDebugText(const char*);
void UiDriver_DebugInfo_DisplayEnable(bool enable);

void 	UiDriver_ChangeTuningStep(uchar is_up);
void	UiDriver_UpdateDisplayAfterParamChange();

void    UiDriver_StartUpScreenInit();
void    UiDriver_StartUpScreenFinish();

void    UiDriver_DoCrossCheck(char cross[],char* xt_corr, char* yt_corr);
void    UiDriver_ToggleVfoAB();
void    UiDriver_SetSplitMode(bool mode_active);


void UiDriver_StartupScreen_LogIfProblem(bool isError, const char* txt);

void    UiDriver_BacklightDimHandler();
//
// Items that are timed using ts.sysclock (operates at 100 Hz)
//
#define	DSP_STARTUP_DELAY					350		// Delay, in 100ths of seconds, after startup, before allowing DSP NR or Notch to be enabled.
#define	DSP_REENABLE_DELAY					13		// Delay, in 100ths of seconds, after return to RX before allowing DSP NR or Notch to be re-enabled
#define	DSP_BAND_CHANGE_DELAY				100		// Delay, in 100ths of a second, after changing bands before restoring DSP NR
//
#define TUNING_LARGE_STEP_MUTING_TIME_DSP_ON	5	// Delay, in 100ths of a second, for audio to be muted after a "large" frequency step (to avoid audio "POP") when DSP on
#define TUNING_LARGE_STEP_MUTING_TIME_DSP_OFF	3	// Delay, in 100ths of a second, for audio to be muted after a "large" frequency step (to avoid audio "POP") when DSP on
#define	RX_MUTE_START_DELAY					(DSP_STARTUP_DELAY + TUNING_LARGE_STEP_MUTING_TIME_DSP_ON)	// establish earliest time after system start before audio muting will work
//
#define	THREAD_TIMING_DELAY					1				// Delay, in 100ths of a second, between thread tasks
//
#define TXRX_SWITCH_AUDIO_MUTE_DELAY_DEFAULT     1          // Default, in 100ths of a second, that audio will be muted after PTT (key-up/key-down) to prevent "clicks" and "clunks"
#define	TXRX_SWITCH_AUDIO_MUTE_DELAY_MAX	25			// Maximum delay, in 100ths of a second, that audio will be muted after PTT (key-up/key-down) to prevent "clicks" and "clunks"


// UI Driver State machine definitions
enum
{
    STATE_S_METER = 0,
    STATE_SWR_METER,
    STATE_HANDLE_POWERSUPPLY,
    STATE_LO_TEMPERATURE,
    STATE_TASK_CHECK,
    STATE_UPDATE_FREQUENCY,
    STATE_PROCESS_KEYBOARD,
    STATE_DISPLAY_SAM_CARRIER,
    STATE_MAX
};

//
// Used for press-and-hold "temporary" step size adjust
//
#define	STEP_PRESS_OFF						0
#define	STEP_PRESS_MINUS					1
#define	STEP_PRESS_PLUS						2
//


// ------------------------------------------------
// Keypad state
extern __IO KeypadState				ks;

// ------------------------------------------------
// Power supply meter
extern PowerMeter				pwmt;

// ------------------------------------------------

extern const ulong tune_steps[T_STEP_MAX_STEPS];

#ifdef USE_FREEDV
int fdv_iq_buffer_peek(FDV_IQ_Buffer** c_ptr);
int fdv_iq_buffer_remove(FDV_IQ_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int fdv_iq_buffer_add(FDV_IQ_Buffer* c);
void fdv_iq_buffer_reset();
int8_t fdv_iq_has_data();
int32_t fdv_iq_has_room();


int fdv_audio_buffer_peek(FDV_Audio_Buffer** c_ptr);
int fdv_audio_buffer_remove(FDV_Audio_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int fdv_audio_buffer_add(FDV_Audio_Buffer* c);
void fdv_audio_buffer_reset();
int8_t fdv_audio_has_data();
int32_t fdv_audio_has_room();
#endif

#ifdef alternate_NR

int NR_in_buffer_peek(FDV_IQ_Buffer** c_ptr);
int NR_in_buffer_remove(FDV_IQ_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int NR_in_buffer_add(FDV_IQ_Buffer* c);
void NR_in_buffer_reset();
int8_t NR_in_has_data();
int32_t NR_in_has_room();


int NR_out_buffer_peek(FDV_IQ_Buffer** c_ptr);
int NR_out_buffer_remove(FDV_IQ_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int NR_out_buffer_add(FDV_IQ_Buffer* c);
void NR_out_buffer_reset();
int8_t NR_out_has_data();
int32_t NR_out_has_room();
#endif

#endif
