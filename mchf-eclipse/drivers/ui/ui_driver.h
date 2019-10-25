/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
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

typedef enum
{
    SPECTRUM_BLANK = 0,
    SPECTRUM_WATERFALL = 1,
    SPECTRUM_SCOPE = 2,
    SPECTRUM_DUAL = 3
} SpectrumMode_t;

typedef struct
{
	uint8_t dmod_mode;
	uint8_t digital_mode;
} ui_driver_mode_t;
//
// --------------------------------------------------------------------------
// Exports
void 	UiDriver_Init(void);
void 	UiDriver_TaskHandler_MainTasks();

void 	UiDriver_CreateTemperatureDisplay();
void 	UiDriver_UpdateFrequency(bool force_update, enum UpdateFrequencyMode_t mode);
void    UiDriver_FrequencyUpdateLOandDisplay(bool full_update);
void    UiDriver_RefreshEncoderDisplay();
void    UiDriver_DrawFButtonLabel(uint8_t button_num, const char* label, uint32_t label_color) ;
void 	UiDriver_DisplayFreqStepSize();
void 	UiDriver_DisplayDemodMode(void);
void	UiDriver_LcdBlankingStartTimer(void);
void    UiDriver_SpectrumChangeLayoutParameters();

void UiDriver_DebugInfo_DisplayEnable(bool enable);

void UiDriver_ChangeTuningStep(uchar is_up);
void UiDriver_UpdateDisplayAfterParamChange();
void UiDriver_UpdateDemodSpecificDisplayAfterParamChange();
void UiDriver_SetDemodMode(uint8_t new_mode);

void UiDriver_StartUpScreenInit();
void UiDriver_StartUpScreenFinish();

void UiDriver_DoCrossCheck(int16_t cross[]);
void UiAction_ToggleVfoAB();
void UiDriver_SetSplitMode(bool mode_active);


void UiDriver_StartupScreen_LogIfProblem(bool isError, const char* txt);

void UiDriver_BacklightDimHandler();

void UiDriver_TextMsgPutChar(char ch);
void UiDriver_TextMsgPutSign(const char *s);
void UiDriver_TextMsgDisplay();
void UiDriver_TextMsgClear();
void UiDriver_DisplayFButton_F1MenuExit();

void UiDriver_SetSpectrumMode(SpectrumMode_t mode);
SpectrumMode_t UiDriver_GetSpectrumMode();

//some exports for layout definitions
void UiAction_ChangeLowerMeterUp();
void UiAction_ToggleWaterfallScopeDisplay();
void UiAction_ChangeSpectrumSize();
void UiAction_ChangeSpectrumZoomLevelDown();
void UiAction_ChangeSpectrumZoomLevelUp();
void UiAction_CheckSpectrumTouchActions();
void UiAction_ChangeFrequencyToNextKhz();
void UiAction_ChangeDemodMode();
void UiAction_ChangePowerLevel();
void UiAction_ChangeAudioSource();
void UiAction_ChangeBandDownOrUp();
void UiAction_ChangeBandUpOrDown();
void UiAction_ChangeDigitalMode();
void UiAction_ChangeDynamicTuning();
void UiAction_ChangeDebugInfoDisplay();
void UiAction_ChangeRfModPresence();
void UiAction_ChangeVhfUhfModPresence();
void UiAction_ChangeFrequencyByTouch();
void Codec_RestartI2S();
uint32_t UiDriver_GetActiveDSPFunctions();
void UiDriver_UpdateDSPmode(uint8_t new_dsp_mode);
bool UiDriver_CheckTouchRegion(const UiArea_t* tr_p);
void UiDriver_Power2String(char* txt, size_t txt_len,uint32_t power_mW);

uint32_t UiDriver_GetNBColor();

void UiDriver_InitBandSet();
void UiDriver_SelectBandMemory(uint16_t vfo_sel, uint8_t new_band_index);


void UiDriver_Callback_AudioISR();

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

#endif
