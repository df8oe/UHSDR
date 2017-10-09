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

// Common
#include "uhsdr_board.h"
#include "profiling.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <src/uhsdr_version.h>
#include "hardware/uhsdr_board_config.h"
#include "ui_menu.h"
#include "uhsdr_rtc.h"
#include "adc.h"
//
//
#include "ui.h"
// LCD
#include "ui_lcd_hy28.h"
#include "ui_spectrum.h"

#include "freedv_uhsdr.h"

// Encoders
#include "ui_rotary.h"

#include "cat_driver.h"

// Codec control
#include "codec.h"

#include "audio_management.h"
#include "ui_driver.h"

#include "ui_configuration.h"
#include "config_storage.h"

#include "cw_gen.h"

#include "radio_management.h"
#include "soft_tcxo.h"

#include "rtty.h"
#include "cw_decoder.h"

// POSITIONS START
#define SPLIT_ACTIVE_COLOUR         		Yellow      // colour of "SPLIT" indicator when active
#define SPLIT_INACTIVE_COLOUR           	Grey        // colour of "SPLIT" indicator when NOT active
#define COL_PWR_IND                 		White

// Frequency display control
#define POS_TUNE_FREQ_X             		116
#define POS_TUNE_FREQ_Y             		100

#define POS_TUNE_SPLIT_FREQ_X           	POS_TUNE_FREQ_X+80//Shift with a small split to the right to close the frequency digits.
#define POS_TUNE_SPLIT_MARKER_X         	POS_TUNE_FREQ_X+40
#define POS_TUNE_SPLIT_FREQ_Y_TX        	POS_TUNE_FREQ_Y+12

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
//#define POS_TUNE_STEP_MASK_H            	15
#define POS_TUNE_STEP_MASK_W            	(SMALL_FONT_WIDTH*7)

//#define POS_RADIO_MODE_X            		4
//#define POS_RADIO_MODE_Y            		5

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


// --------------------------------------------------
// Standalone controls
//
// DSP mode
// Lower DSP box
#define POS_LEFTBOXES_IND_X              	0
#define POS_LEFTBOXES_IND_Y              	130
#define LEFTBOX_WIDTH 58 // used for the lower left side controls
#define LEFTBOX_ROW_H  (28)
#define LEFTBOX_ROW_2ND_OFF  (13)

// Power level
#define POS_PW_IND_X                		POS_DEMOD_MODE_X -1
#define POS_PW_IND_Y                		POS_DEMOD_MODE_Y - 16

#define POS_DIGMODE_IND_X              		0
#define POS_DIGMODE_IND_Y              		(191)

// S meter position
#define POS_SM_IND_X                		116
#define POS_SM_IND_Y                		0
#define SM_IND_W (200)
#define SM_IND_H (72 - BTM_MINUS)

// Supply Voltage indicator
//#define POS_PWRN_IND_X              		0
#define POS_PWRN_IND_Y              		193

#define POS_PWR_IND_X               		4
#define POS_PWR_IND_Y               		(POS_PWRN_IND_Y + 15)


// Temperature Indicator
#define POS_TEMP_IND_X              		0
#define POS_TEMP_IND_Y              		0

// RTC
#define POS_RTC								79

#define POS_LOADANDDEBUG_Y					95
#define POS_DEBUG_X							0
#define POS_LOAD_X							280


//
// Location of numerical FWD/REV power indicator
//
#define	POS_PWR_NUM_IND_X					1
#define	POS_PWR_NUM_IND_Y					80

#define POS_MEMORYLABEL_X (161+(SMALL_FONT_WIDTH * 11)+4)
#define POS_MEMORYLABEL_Y (64)
// POSITIONS END

static void     UiDriver_CreateMainFreqDisplay();

static void     UiDriver_CreateMeters();
static void     UiDriver_DeleteMeters();
static void 	UiDriver_DrawSMeter(ushort color);
//
static void 	UiDriver_UpdateTopMeterA(uchar val);
static void 	UiDriver_UpdateBtmMeter(float val, uchar warn);

static void 	UiDriver_InitFrequency();
//

static void     UiDriver_UpdateLcdFreq(ulong dial_freq,ushort color,ushort mode);
static bool 	UiDriver_IsButtonPressed(ulong button_num);
static void		UiDriver_TimeScheduler();				// Also handles audio gain and switching of audio on return from TX back to RX
static void 	UiDriver_ChangeToNextDemodMode(bool select_alternative_mode);
static void 	UiDriver_ChangeBand(uchar is_up);
static bool 	UiDriver_CheckFrequencyEncoder();

static void     UiDriver_DisplayBand(uchar band);
static uchar    UiDriver_DisplayBandForFreq(ulong freq);

static void     UiDriver_DisplayEncoderOneMode();
static void     UiDriver_DisplayEncoderTwoMode();
static void     UiDriver_DisplayEncoderThreeMode();

static void 	UiDriver_DisplayNoiseBlanker(bool encoder_active);
static void 	UiDriver_DisplayDSPMode(bool encoder_active);
static void 	UiDriver_DisplayTone(bool encoder_active);
static void 	UiDriver_DisplayRit(bool encoder_active);
static void     UiDriver_DisplayAfGain(bool encoder_active);
static void     UiDriver_DisplayRfGain(bool encoder_active);
static void     UiDriver_DisplaySidetoneGain(bool encoder_active);
static void     UiDriver_DisplayCmpLevel(bool encoder_active);
static void     UiDriver_DisplayKeyerSpeed(bool encoder_active);
static void     UiDriver_DisplayLineInModeAndGain(bool encoder_active);

static void     UiDriver_DisplayMemoryLabel();


static void 	UiDriver_DisplayDigitalMode();
static void 	UiDriver_DisplayPowerLevel();
static void     UiDriver_DisplayTemperature(int temp);
static void     UiDriver_DisplayVoltage();

static void 	UiDriver_HandleSMeter();
static void 	UiDriver_HandleTXMeters();
static bool     UiDriver_HandleVoltage();

#if 0
static void 	UiDriverUpdateLoMeter(uchar val,uchar active);
#endif
static void     UiDriver_CreateVoltageDisplay();

static void 	UiDriver_HandleLoTemperature();


static bool	    UiDriver_LoadSavedConfigurationAtStartup();
static bool	    UiDriver_TouchscreenCalibration();

static void     UiDriver_PowerDownCleanup(bool saveConfiguration);

static void UiDriver_HandlePowerLevelChange(uint8_t power_level);
static void UiDriver_HandleBandButtons(uint16_t button);

static void UiDriver_KeyTestScreen();

static bool UiDriver_SaveConfiguration();

static void UiDriver_DisplayRttySpeed(bool encoder_active);
static void UiDriver_DisplayRttyShift(bool encoder_active);

typedef struct
{
	uint8_t x_left;
	uint8_t x_right;
	uint8_t y_down;
	uint8_t y_up;
} touchscreen_region_t;


// Tuning steps
const ulong tune_steps[T_STEP_MAX_STEPS] =
{
		T_STEP_1HZ,
		T_STEP_10HZ,
		T_STEP_100HZ,
		T_STEP_500HZ,
		T_STEP_1KHZ,
		T_STEP_5KHZ,
		T_STEP_9KHZ,
		T_STEP_10KHZ,
		T_STEP_100KHZ,
		T_STEP_1MHZ,
		T_STEP_10MHZ
};


// The following are calibrations for the S-meter based on 6 dB per S-unit, 10 dB per 10 dB mark above S-9
// The numbers within are linear gain values, not logarithmic, starting with a zero signal level of 1
// There are 33 entries, one corresponding with each point on the S-meter
#define	S_Meter_Cal_Size	33	// number of entries in table below
const float S_Meter_Cal[S_Meter_Cal_Size] =
{
		14.1,	    //1.41,	        //1, S0.5, 3dB
		20,		    //2,		    //2, S1, 6dB
		28.1,	    //2.81,	        //3, S1.5, 9dB
		30,		    //3,		    //4, S2, 12dB
		56.2,	    //5.62,	        //5, S2.5, 15dB
		79.4,	    //7.94,	        //6, S3, 18dB
		112.2,	    //11.22,	    //7, S3.5, 21dB
		158.5,	    //15.85,	    //8, S4, 24dB
		223.9,	    //22.39,	    //9, S4.5, 27dB
		316.3,	    //31.63,	    //10, S5, 30dB
		446.7,	    //44.67,	    //11, S5.5, 33dB
		631,	    //63.10,	    //12, S6, 36dB
		891.3,	    //89.13,	    //13, S6.5, 39dB
		1258.9,	    //125.89,	    //14, S7, 42dB
		1778.3,	    //177.83,	    //15, S7.5, 45dB
		2511.9,	    //251.19,	    //16, S8, 48dB
		3548.1,	    //354.81,	    //17, S8.5, 51dB
		5011.9,	    //501.19,	    //18, S9, 54dB
		8912.5,	    //891.25,	    //19, +5, 59dB
		15848.9,	//1584.89,	    //20, +10, 64dB
		28183.8,	//2818.38,	    //21, +15, 69dB
		50118.7,	//5011.87,	    //22, +20, 74dB
		89125.1,	//8912.51,	    //23, +25, 79dB
		158489.3,	//15848.93,	    //24, +30, 84dB
		281838.2,	//28183.82,	    //25, +35, 89dB
		501187.2,	//50118.72,	    //26, +40, 94dB
		891250.9,	//89125.09,	    //27, +45, 99dB
		1585893.2,	//158489.32,	//28, +50, 104dB
		2818382.9,	//281838.29,	//29, +55, 109dB
		5011872.3,	//501187.23,	//30, +60, 114dB
		8912509.4,	//891250.94,	//31, +65, 119dB
		15848931.9,	//1584893.19,	//32, +70, 124dB
		28183829.3,	//2818382.93	//33, +75, 129dB
};

// if S-meter is based on FFT / dBm calculation
const float S_Meter_Cal_dbm[S_Meter_Cal_Size] =
{
		// dBm vs. S-value
		-124.0,	// S0.5
		-121.0,	// S1
		-118.0,	// S1.5
		-115.0,	// S2
		-112.0,	// S2.5
		-109.0,	// S3
		-106.0,	// S3.5
		-103.0,	// S4
		-100.0,	// S4.5
		-97.0,	// S5
		-94.0,	// S5.5
		-91.0,	// S6
		-88.0,	// S6.5
		-85.0,	// S7
		-82.0,	// S7.5
		-79.0,	// S8
		-76.0,	// S8.5
		-73.0,	// S9
		-68.0,	// S9+5
		-63.0,	// +10
		-58.0,	// +15
		-53.0,	// +20
		-48.0,	// +25
		-43.0,	// +30
		-38.0,	// +35
		-33.0,	// +40
		-28.0,	// +45
		-23.0,	// +50
		-18.0,	// +55
		-13.0,	// +60
		-8.0,	// +65
		-3.0,	// +70
		2.0,    // +75
};

BandRegs vfo[VFO_MAX];

// ------------------------------------------------
// Keypad state
__IO KeypadState				ks;

// ------------------------------------------------
// Power supply meter
PowerMeter					pwmt;


// ------------------------------------------------

uchar drv_state = 0;

bool filter_path_change = false;

// check if touched point is within rectangle of valid action
static bool UiDriver_CheckTouchRegion(const touchscreen_region_t* tr_p)
{
	return (ts.tp->x <= tr_p->x_right && ts.tp->x >= tr_p->x_left && ts.tp->y >= tr_p->y_down && ts.tp->y <= tr_p->y_up);
}


inline int32_t change_and_limit_int(volatile int32_t val, int32_t change, int32_t min, int32_t max)
{
	val +=change;
	if (val< min)
	{
		val = min;
	}
	else if (val>  max)
	{
		val = max;
	}
	return val;
}


inline uint32_t change_and_limit_uint(volatile uint32_t val, int32_t change, uint32_t min, uint32_t max)
{
	if (change < 0 && ( -change  > (val - min)))
	{
		val = min;
	}
	else if (change > 0 && change >  max - val)
	{
		val = max;
	}
	else
	{
		val +=change;
	}
	return val;
}

inline uint32_t change_and_wrap_uint(volatile uint32_t val, int32_t change, uint32_t min, uint32_t max)
{
	if (change  > ((int32_t)max - (int32_t)val))
	{
		val = min;
	}
	else if ((change + (int32_t)val) <  (int32_t)min)
	{
		val = max;
	}
	else
	{
		val +=change;
	}
	return val;
}

inline void incr_wrap_uint8(volatile uint8_t* ptr, uint8_t min, uint8_t max )
{
	*ptr = (change_and_wrap_uint(*ptr,+1,min,max))&0xff;
}
inline void incr_wrap_uint16(volatile uint16_t* ptr, uint16_t min, uint16_t max )
{
	*ptr = (change_and_wrap_uint(*ptr,+1,min,max))&0xff;
}
inline void decr_wrap_uint8(volatile uint8_t* ptr, uint8_t min, uint8_t max )
{
	*ptr = (change_and_wrap_uint(*ptr,-1,min,max))&0xff;
}
inline void decr_wrap_uint16(volatile uint16_t* ptr, uint16_t min, uint16_t max )
{
	*ptr = (change_and_wrap_uint(*ptr,-1,min,max))&0xff;
}

inline bool is_touchscreen_pressed()
{
	return (ts.tp->state == TP_DATASETS_VALID);	// touchscreen data available
}

bool is_vfo_b()
{
	return (ts.vfo_mem_mode & VFO_MEM_MODE_VFO_B) != 0;
}

inline bool is_dsp_nb()
{
	return (ts.dsp_active & DSP_NB_ENABLE) != 0;
}

inline bool is_dsp_nr()
{
	return (ts.dsp_active & DSP_NR_ENABLE) != 0;
}

inline bool is_dsp_nr_postagc()
{
	return (ts.dsp_active & DSP_NR_POSTAGC_ENABLE) != 0;
}

inline bool is_dsp_notch()
{
	return (ts.dsp_active & DSP_NOTCH_ENABLE) != 0;
}

inline bool is_dsp_mnotch()
{
	return (ts.dsp_active & DSP_MNOTCH_ENABLE) != 0;
}

inline bool is_dsp_mpeak()
{
	return (ts.dsp_active & DSP_MPEAK_ENABLE) != 0;
}


typedef struct
{
	touchscreen_region_t region;
	void (*function)();
} touchaction_descr_t;


#define KEYACTION_NOP    (NULL)				// This action for the pressed key is treated as being executed, but it is a no-operation
#define KEYACTION_PASS ((void(*)())-1)		// This action for the pressed key is treated as not present, i.e. we do not report the key event has been processed

typedef struct
{
	const touchaction_descr_t* actions;
	int32_t size;
} touchaction_list_descr_t;

typedef struct
{
	uint32_t key_code;
	// use KEYACTION_NONE and KEYACTION_PASS to handled nop and pass for further processing,
	// see comments for these constants
	void (*press_func)(); // executed if short press of key is detected
	void (*hold_func)();  // executed if press and hold of key is detected
} keyaction_descr_t;

typedef struct
{
	const keyaction_descr_t* actions;
	int32_t size;
} keyaction_list_descr_t;


/*
 * @brief find the matching region in a list of region and associated function
 * @returns: true, if a match for the touch coordinates region was found.
 */
bool UiDriver_ProcessTouchActions(const touchaction_list_descr_t* tld)
{
	bool retval = false;
	if (tld != NULL)
	{
		for (uint32_t idx = 0; idx < tld->size; idx++)
		{
			if (UiDriver_CheckTouchRegion(&tld->actions[idx].region))
			{
				if (tld->actions[idx].function != NULL)
				{
					(*tld->actions[idx].function)();
				}
				retval = true;
				break;
			}
		}
	}
	return retval;
}

/*
 * @brief find the matching keycode in a list of keycodes and associated functions
 * @returns: true, if a match for the keycode was found and a function (which can be a "no operation") has been executed.
 */

bool UiDriver_ProcessKeyActions(const keyaction_list_descr_t* kld)
{
	bool retval = false;
	if (kld != NULL)
	{
		for (uint32_t idx = 0; idx < kld->size; idx++)
		{
			if (kld->actions[idx].key_code == ks.button_id)
			{
				void (*func_ptr)() =  ks.press_hold == true ? kld->actions[idx].hold_func:kld->actions[idx].press_func;
				if (func_ptr != KEYACTION_NOP && func_ptr != KEYACTION_PASS)
				{
					(*func_ptr)();
				}
				retval = func_ptr != KEYACTION_PASS;
				break;
			}
		}
	}

	return retval;
}
/**
 * @brief restarts lcd blanking timer, called in all functions which detect user interaction with the device
 */
void UiDriver_LcdBlankingStartTimer()
{
	if(ts.lcd_backlight_blanking & LCD_BLANKING_ENABLE)     // is LCD blanking enabled?
	{
		uint32_t ltemp = (ulong)(ts.lcd_backlight_blanking & LCD_BLANKING_TIMEMASK);      // get setting of LCD blanking timing
		ltemp *= 100;       // multiply to convert to deciseconds
		ts.lcd_blanking_time = ltemp + ts.sysclock;     // calculate future time at which LCD is to be turned off
		ts.lcd_blanking_flag = false;       // clear flag to make LCD turn on
	}
}

static void   UiDriver_LcdBlankingProcessTimer()
{
	// Process LCD auto-blanking
	if(ts.lcd_backlight_blanking & LCD_BLANKING_ENABLE)      // is LCD auto-blanking enabled?
	{
		if(ts.sysclock > ts.lcd_blanking_time)      // has the time expired and the LCD should be blanked?
		{
			ts.lcd_blanking_flag = true;             // yes - blank the LCD
		}
		else                                        // time not expired
		{
			ts.lcd_blanking_flag = false;             // un-blank the LCD
		}
	}
	else                                  // auto-blanking NOT enabled
	{
		ts.lcd_blanking_flag = false;               // always un-blank the LCD in this case
	}
}

static char ui_txt_msg_buffer[ui_txt_msg_buffer_max+1]; // we need to be able store the '\0' as well.
static const char ui_txt_empty_line[ui_txt_msg_buffer_max+1] = "                                             ";
static int ui_txt_msg_idx= 0;
static bool ui_txt_msg_update = false;


void UiDriver_TextMsgClear()
{
    UiLcdHy28_PrintText(5,92, ui_txt_empty_line,Yellow,Black,4);
    ui_txt_msg_idx = 0;
    ui_txt_msg_update = true;
}

void UiDriver_TextMsgDisplay()
{
    if (ui_txt_msg_update == true)
    {
        ui_txt_msg_update = false;
        const char* txt_ptr = ui_txt_msg_idx == 0? ui_txt_empty_line:ui_txt_msg_buffer;
        UiLcdHy28_PrintText(5,92,txt_ptr,Yellow,Black,4);
    }
}

void UiDriver_TextMsgPutChar(char ch)
{
    if (ch=='\n' || ch == '\r')
    {
        ui_txt_msg_idx=0;
    	ui_txt_msg_buffer[ui_txt_msg_idx] = '\0';
    }
    else if (ui_txt_msg_idx < (ui_txt_msg_buffer_max))
    {
        ui_txt_msg_idx++;
    	ui_txt_msg_buffer[ui_txt_msg_idx] = '\0'; // set the line end before we add the character prevents unterminated strings
        ui_txt_msg_buffer[ui_txt_msg_idx-1]=ch; //fill from left to right
    }
    else
    {
        for (int shift_count = 0;shift_count < (ui_txt_msg_buffer_max-1);shift_count++)
        {
            ui_txt_msg_buffer[shift_count]=ui_txt_msg_buffer[shift_count+1];
        }
        ui_txt_msg_buffer[ui_txt_msg_buffer_max-1]=ch;
    }
    ui_txt_msg_update = true;
}

void UiDriver_TextMsgPutSign(const char *s)
{
	UiDriver_TextMsgPutChar('<');
	UiDriver_TextMsgPutChar(s[0]);
	UiDriver_TextMsgPutChar(s[1]);
	UiDriver_TextMsgPutChar('>');
}

static void UiDriver_LeftBoxDisplay(const uint8_t row, const char *label, bool encoder_active,
		const char* text, uint32_t color, uint32_t clr_val, bool text_is_value)
{

	uint32_t label_color = encoder_active?Black:color;

	// max visibility of active element
	uint32_t bg_color = encoder_active?Orange:Blue;
	uint32_t brdr_color = encoder_active?Orange:Blue;


	UiLcdHy28_DrawEmptyRect(POS_LEFTBOXES_IND_X, POS_LEFTBOXES_IND_Y + (row * LEFTBOX_ROW_H), LEFTBOX_ROW_H - 2, LEFTBOX_WIDTH - 2, brdr_color);
	UiLcdHy28_PrintTextCentered(POS_LEFTBOXES_IND_X + 1, POS_LEFTBOXES_IND_Y + (row * LEFTBOX_ROW_H) + 1,LEFTBOX_WIDTH - 3, label,
			label_color, bg_color, 0);

	// this causes flicker, but I am too lazy to fix that now
	UiLcdHy28_DrawFullRect(POS_LEFTBOXES_IND_X + 1, POS_LEFTBOXES_IND_Y + (row * LEFTBOX_ROW_H) + 1 + 12, LEFTBOX_ROW_H - 4 - 11, LEFTBOX_WIDTH - 3, text_is_value?Black:bg_color);
	if (text_is_value)
	{
		UiLcdHy28_PrintTextRight((POS_LEFTBOXES_IND_X + LEFTBOX_WIDTH - 4), (POS_LEFTBOXES_IND_Y + (row * LEFTBOX_ROW_H) + 1 + LEFTBOX_ROW_2ND_OFF), text,
				clr_val, text_is_value?Black:bg_color, 0);
	}
	else
	{
		UiLcdHy28_PrintTextCentered((POS_LEFTBOXES_IND_X + 1), (POS_LEFTBOXES_IND_Y + (row * LEFTBOX_ROW_H) + 1 + LEFTBOX_ROW_2ND_OFF),LEFTBOX_WIDTH - 3, text,
				color, bg_color, 0);
	}
}

static void UiDriver_LcdBlankingStealthSwitch()
{
	if(ts.lcd_backlight_blanking & LCD_BLANKING_ENABLE)
	{         // Yes - is MSB set, indicating "stealth" (backlight timed-off) mode?
		ts.lcd_backlight_blanking &= ~LCD_BLANKING_ENABLE;
	} // yes - clear that bit, turning off "stealth" mode
	else
	{
		if(ts.lcd_backlight_blanking & LCD_BLANKING_TIMEMASK)    // bit NOT set AND the timing set to NON-zero?
		{
			ts.lcd_backlight_blanking |= LCD_BLANKING_ENABLE;       // no - turn on MSB to activate "stealth" mode
		}
	}
}

void UiDriver_DisplayFilter()
{
	const char* filter_ptr;
	uint32_t font_clr= filter_path_change?Black:White;

	const char *filter_names[2];

	AudioFilter_GetNamesOfFilterPath(ts.filter_path,filter_names);
	if (filter_names[1] != NULL)
	{
		filter_ptr = filter_names[1];
	}
	else
	{
		filter_ptr = " ";
	}

	UiDriver_LeftBoxDisplay(1,filter_names[0],filter_path_change,filter_ptr,font_clr, font_clr,false);
}

// TODO: most of this belongs to radio management, not UI
static void UiDriver_ToggleDigitalMode()
{
	if (ts.digital_mode != DigitalMode_None)
	{
		// a valid digital mode is set but may not be active yet
		if (ts.dmod_mode != DEMOD_DIGI)
		{
			if (RadioManagement_IsApplicableDemodMode(DEMOD_DIGI))
			{
				// this will switch to the corresponding sideband if we come from
				// SSB, otherwise the automatically selected default (AUTO LSB/USB ON) or the previously used
				// will be the selected one.
				if (is_ssb(ts.dmod_mode))
				{
					ts.digi_lsb = RadioManagement_LSBActive(ts.dmod_mode);
				}
				RadioManagement_SetDemodMode(DEMOD_DIGI);
			}
		}
	}
	else
	{
		if (ts.dmod_mode == DEMOD_DIGI)
		{
			// we are in digital mode but the current digital mode is in fact
			// None, i.e. we are going analog now
			RadioManagement_SetDemodMode(ts.digi_lsb?DEMOD_LSB:DEMOD_USB);
		}
	}
	UiDriver_UpdateDisplayAfterParamChange();
}

/*
 * @brief Function will update LO and Display Digits, it will never change LO if not necessary
 *
 * @param full_update set to true in order to have the full display digits being updated
 *
 */
void UiDriver_FrequencyUpdateLOandDisplay(bool full_update)
{
	if (full_update)
	{
		ts.refresh_freq_disp = 1;           // update ALL digits
	}
	if(is_splitmode())
	{
		// SPLIT mode
		UiDriver_UpdateFrequency(false,UFM_SMALL_TX);
		UiDriver_UpdateFrequency(false,UFM_SMALL_RX);
	}
	else
	{
		UiDriver_UpdateFrequency(false,UFM_AUTOMATIC);
	}
	ts.refresh_freq_disp = 0;           // update ALL digits
}

void UiDriver_DebugInfo_DisplayEnable(bool enable)
{

	UiLcdHy28_PrintText(POS_DEBUG_X,POS_LOADANDDEBUG_Y,enable?"enabled":"       ",Green,Black,0);

	if (enable == false)
	{
		UiLcdHy28_PrintText(POS_LOAD_X,POS_LOADANDDEBUG_Y,"     ",White,Black,5); //  "    "    "     "
	}

	ts.show_debug_info = enable;

}

void UiDriver_SpectrumZoomChangeLevel()
{
	UiSpectrum_WaterfallClearData();
	AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);


	if (ts.menu_mode == false)
	{
		UiSpectrum_Init();      // init spectrum scope
	}
}




void UiDriver_HandlePowerLevelChange(uint8_t power_level)
{
	//
	if (RadioManagement_PowerLevelChange(ts.band,power_level))
	{
		UiDriver_DisplayPowerLevel();
		if (ts.menu_mode)
		{
			UiMenu_RenderMenu(MENU_RENDER_ONLY);
		}
	}
}

void UiDriver_HandleBandButtons(uint16_t button)
{

	static const int BAND_DOWN = 0;
	static const int BAND_UP = 1;

	bool buttondirSwap = (ts.flags1 & FLAGS1_SWAP_BAND_BTN)?true:false;
	uint8_t dir;


	if (button == BUTTON_BNDM)
	{
		dir = buttondirSwap ? BAND_UP : BAND_DOWN;
	}
	else
	{
		dir = buttondirSwap ? BAND_DOWN : BAND_UP;
	}

	UiDriver_ChangeBand(dir);
}


static void UiDriver_PublicsInit()
{
	// Button state structure init state
	ks.button_id			= BUTTON_NONE;
	ks.button_pressed		= 0;
	ks.button_released		= 0;
	ks.button_processed		= 0;
	ks.debounce_time		= 0;


	// Auto button blink state
	//abst.blink_flag 		= 0;
	//abst.blink_skip 		= 0;

	// SWR meter init
	swrm.p_curr				= 0;
	swrm.fwd_calc			= 0;
	swrm.rev_calc			= 0;
	swrm.fwd_pwr			= 0;
	swrm.rev_pwr			= 0;
	swrm.fwd_dbm			= 0;
	swrm.rev_dbm			= 0;
	swrm.vswr			 	= 0;
	swrm.sensor_null		= SENSOR_NULL_DEFAULT;
	{
		int idx;
		for (idx = 0; idx < COUPLING_MAX; idx++)
		{
			swrm.coupling_calc[idx]    = SWR_COUPLING_DEFAULT;
		}
	}
	swrm.pwr_meter_disp		= 0;	// Display of numerical FWD/REV power metering off by default
	swrm.pwr_meter_was_disp = 0;	// Used to indicate if FWD/REV numerical power metering WAS displayed

	// Power supply meter
	pwmt.p_curr				= 0;
	pwmt.pwr_aver 			= 0;
	pwmt.undervoltage_detected = false;

}



void UiDriver_Init()
{
	// Driver publics init
	UiDriver_PublicsInit();
	// Init frequency publics
	UiDriver_InitFrequency();

	// Load stored data from eeprom or calibrate touchscreen
	bool run_keytest = (UiDriver_LoadSavedConfigurationAtStartup() == false && UiDriver_TouchscreenCalibration() == false);

	UiDriver_StartupScreen_LogIfProblem(AudioDriver_GetTranslateFreq() == 0,
			"WARNING:  Freq. Translation is OFF!!!\nTranslation is STRONGLY recommended!!");

	// now run all inits which need to be done BEFORE going into test screen mode
	uint8_t mirtemp;
	if(ts.flags1 & FLAGS1_REVERSE_X_TOUCHSCREEN)
	{
		mirtemp = 1;
	}
	else
	{
		mirtemp = 0;
	}
	if(ts.flags1 & FLAGS1_REVERSE_Y_TOUCHSCREEN)
	{
		mirtemp += 2;
	}

	UiLcdHy28_TouchscreenInit(mirtemp);

	if (run_keytest)
	{
		UiDriver_KeyTestScreen();
	}


	Si570_SetPPM((float)ts.freq_cal/10.0);

	df.tune_new = vfo[is_vfo_b()?VFO_B:VFO_A].band[ts.band].dial_value;		// init "tuning dial" frequency based on restored settings
	df.tune_old = 0;

	ts.cw_lsb = RadioManagement_CalculateCWSidebandMode();			// determine CW sideband mode from the restored frequency

	AudioManagement_CalcTxCompLevel();      // calculate current settings for TX speech compressor

	AudioFilter_InitRxHilbertFIR(ts.dmod_mode);
	AudioFilter_InitTxHilbertFIR();

	AudioManagement_SetSidetoneForDemodMode(ts.dmod_mode,false);

	sd.display_offset = INIT_SPEC_AGC_LEVEL;		// initialize setting for display offset/AGC

	// Reset inter driver requests flag
	ts.LcdRefreshReq	= 0;
	ts.new_band 		= ts.band;
	df.step_new 		= df.tuning_step;

	// Extra HW init
	Board_PostInit();


	UiDriver_LcdBlankingStartTimer();			// init timing for LCD blanking
	ts.lcd_blanking_time = ts.sysclock + LCD_STARTUP_BLANKING_TIME;
	ts.low_power_shutdown_time = ts.sysclock + LOW_POWER_SHUTDOWN_DELAY_TIME;

#ifdef USE_DISP_480_320
	UiSpectrum_SetWaterfallMemoryPointer(ts.ramsize);
#endif
}

#define BOTTOM_BAR_LABEL_W (56)
void UiDriver_DrawFButtonLabel(uint8_t button_num, const char* label, uint32_t label_color)
{
	UiLcdHy28_PrintTextCentered(POS_BOTTOM_BAR_F1_X + (button_num - 1)*64, POS_BOTTOM_BAR_F1_Y, BOTTOM_BAR_LABEL_W, label,
			label_color, Black, 0);
}

#define ENC_COL_W (37)
#define ENC_ROW_H (30)
#define ENC_ROW_2ND_OFF (14)

void UiDriver_EncoderDisplay(const uint8_t row, const uint8_t column, const char *label, bool encoder_active,
		const char temp[5], uint32_t color)
{

	uint32_t label_color = encoder_active?Black:Grey1;

	// max visibility of active element
	uint32_t bg_color = encoder_active?Orange:Grey;
	uint32_t brdr_color = encoder_active?Orange:Grey;

	UiLcdHy28_DrawEmptyRect(POS_ENCODER_IND_X + ENC_COL_W * column, POS_ENCODER_IND_Y + row * ENC_ROW_H, ENC_ROW_H - 2, ENC_COL_W - 2, brdr_color);
	UiLcdHy28_PrintTextCentered((POS_ENCODER_IND_X + 1 + ENC_COL_W * column), (POS_ENCODER_IND_Y + 1 + row * ENC_ROW_H),ENC_COL_W - 3, label,
			label_color, bg_color, 0);
	UiLcdHy28_PrintTextRight((POS_ENCODER_IND_X + ENC_COL_W - 4 + ENC_COL_W * column), (POS_ENCODER_IND_Y + 1 + row * ENC_ROW_H + ENC_ROW_2ND_OFF), temp,
			color, Black, 0);
}


static void UiDriver_DisplayFButton_F1MenuExit()
{
	char* cap;
	uint32_t color;
	if (ts.keyer_mode.active)
	{
		if (ts.keyer_mode.button_recording == KEYER_BUTTON_1)
		{
			cap = "REC";
			color = Red;
		}
		else
		{
			cap = (char*) ts.keyer_mode.cap[0];
			color = White;
		}
	}
	else
	{
		if (!ts.menu_var_changed)
		{

			if (ts.menu_mode)
			{
				cap = "EXIT";
				color = Yellow;
			}
			else
			{
				cap = "MENU";
				color = White;
			}
		}
		else
		{
			cap = ts.menu_mode?"EXIT *":"MENU *";
			color = Orange;
		}
	}
	UiDriver_DrawFButtonLabel(1, cap, color);
}


static void UiDriver_DisplayFButton_F2SnapMeter()
{
	const char* cap;
	uint32_t color;
	if (ts.keyer_mode.active)
	{
		if (ts.keyer_mode.button_recording == KEYER_BUTTON_2)
		{
			cap = "REC";
			color = Red;
		}
		else
		{
			cap = (char *)ts.keyer_mode.cap[1];
			color = White;
		}
	}
	else
	{
#ifdef USE_SNAP
		cap = "SNAP";
		color = White;    // yes - indicate with color
#else
		color = White;
		cap = "METER";
#endif
	}
	UiDriver_DrawFButtonLabel(2,cap,color);
}



static void UiDriver_FButton_F3MemSplit()
{
	const char* cap;
	uint32_t color;

	if (ts.keyer_mode.active)
	{
		if (ts.keyer_mode.button_recording == KEYER_BUTTON_3)
		{
			cap = "REC";
			color = Red;
		}
		else
		{
			cap = (char *)ts.keyer_mode.cap[2];
			color = White;
		}

	}
	else
	{
		if (ts.vfo_mem_flag)            // is it in VFO mode now?
		{
			cap = "MEM";
			color = White;    // yes - indicate with color
		}
		else
		{
			color = is_splitmode() ?
					SPLIT_ACTIVE_COLOUR : SPLIT_INACTIVE_COLOUR;
			cap = "SPLIT";
		}
	}
	UiDriver_DrawFButtonLabel(3, cap, color);
}


static inline void UiDriver_FButton_F4ActiveVFO()
{
	const char* cap;
	if (ts.keyer_mode.active)
	{
		cap = " "; //FIXME This will be DEL
	}
	else
	{
		cap = is_vfo_b() ? "VFO B" : "VFO A";
	}
	UiDriver_DrawFButtonLabel(4, cap, White);
}

static inline void UiDriver_FButton_F5Tune()
{
	const char* cap;
	uint32_t color;
	color = RadioManagement_IsTxDisabled() ? Grey1 : (ts.tune ? Red : White);
	if (ts.keyer_mode.active)
	{
		if (ts.buffered_tx)
		{
			cap = " "; //FIXME this will be TX/RX
		}
		else
		{
			cap = " "; //FIXME this will be UNBUF
		}
	}
	else
	{
		cap = "TUNE";
	}
	UiDriver_DrawFButtonLabel(5, cap, color);
}


void UiDriver_EncoderDisplaySimple(const uint8_t column, const uint8_t row, const char *label, bool encoder_active,
		uint32_t value)
{

	char temp[5];
	uint32_t color = encoder_active?White:Grey;

	snprintf(temp,5," %2lu",value);
	UiDriver_EncoderDisplay(column, row, label, encoder_active,
			temp, color);
}

void UiDriver_DisplaySplitFreqLabels()
{
	// in SPLIT mode?
	const char *split_rx, *split_tx;
	if (!(is_vfo_b()))
	{
		split_rx = "(A) RX->";  // Place identifying marker for RX frequency
		split_tx = "(B) TX->";  // Place identifying marker for TX frequency
	}
	else
	{
		split_rx = "(B) RX->";  // Place identifying marker for RX frequency
		split_tx = "(A) TX->";  // Place identifying marker for TX frequency
	}
	UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X - (SMALL_FONT_WIDTH * 5),
			POS_TUNE_FREQ_Y, split_rx, RX_Grey, Black,
			0);  // Place identifying marker for RX frequency
	UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X - (SMALL_FONT_WIDTH * 5),
			POS_TUNE_SPLIT_FREQ_Y_TX, split_tx, TX_Grey, Black,
			0);  // Place identifying marker for TX frequency
}

void UiAction_CopyVfoAB()
{
	// not in menu mode:  Make VFO A = VFO B or VFO B = VFO A, as appropriate
	__IO VfoReg* vfo_store;
	if(is_vfo_b())      // are we in VFO B mode?
	{
		vfo_store = &vfo[VFO_A].band[ts.band];
	}
	else        // we were in VFO A mode
	{
		vfo_store = &vfo[VFO_B].band[ts.band];
	}
	vfo_store->dial_value = df.tune_new;
	vfo_store->decod_mode = ts.dmod_mode;                   // copy active VFO (A) settings into B

	UiDriver_FrequencyUpdateLOandDisplay(true);

	if (ts.menu_mode == false)
	{
		UiSpectrum_Clear();          // clear display under spectrum scope
		UiLcdHy28_PrintText(80,160,is_vfo_b()?"VFO B -> VFO A":"VFO A -> VFO B",Cyan,Black,1);
		HAL_Delay(3000);
		UiSpectrum_Init();           // init spectrum scope
	}
}

void UiAction_ToggleVfoAB()
{

	uint32_t old_dmod_mode = ts.dmod_mode;

	RadioManagement_ToggleVfoAB();

	UiDriver_FButton_F4ActiveVFO();

	// do frequency/display update
	if(is_splitmode())      // in SPLIT mode?
	{
		UiDriver_DisplaySplitFreqLabels();
	}

	// Change decode mode if need to
	if(ts.dmod_mode != old_dmod_mode)
	{
		UiDriver_UpdateDisplayAfterParamChange();
	}
	else
	{
		UiDriver_FrequencyUpdateLOandDisplay(true);
	}
}

void UiDriver_SetSplitMode(bool mode_active)
{
	if(mode_active)      // are we in SPLIT mode?
	{
		ts.vfo_mem_mode |= VFO_MEM_MODE_SPLIT;      // yes - turn on MSB to activate SPLIT
	}
	else // are we NOT in SPLIT mode?
	{
		ts.vfo_mem_mode &= ~VFO_MEM_MODE_SPLIT; // yes - turn off MSB to turn off SPLIT
	}
	UiDriver_CreateMainFreqDisplay();      //
	UiDriver_FrequencyUpdateLOandDisplay(true);
}

/**
 * @brief: process hardcoded buttons click and hold
 */
void UiDriver_RefreshEncoderDisplay()
{
	UiDriver_DisplayEncoderOneMode();
	UiDriver_DisplayEncoderTwoMode();
	UiDriver_DisplayEncoderThreeMode();
}

/**
 * @brief This is THE function to call after changing operational parameters such as frequency or demod mode
 * It will make sure to update the display AND also tunes to a newly selected frequency if not already tuned to it.
 */
typedef struct
{
	uint8_t dmod_mode;
	uint8_t digital_mode;
} ui_driver_mode_t;

ui_driver_mode_t ui_driver_state;

bool UiDriver_IsDemodModeChange()
{
	bool retval = (ts.dmod_mode != ui_driver_state.dmod_mode);
	retval |= ts.dmod_mode == DEMOD_DIGI && ts.digital_mode != ui_driver_state.digital_mode;

	return retval;
}

/**
 * @brief cleans out mode specific ui elements before switching to next mode
 */
void UiDriver_ModeSpecificDisplayClear(uint8_t dmod_mode, uint8_t digital_mode)
{
	switch(dmod_mode)
	{
	case DEMOD_CW:
		UiDriver_TextMsgClear();
		CwDecoder_WpmDisplayClearOrPrepare(false);
		UiSpectrum_InitCwSnapDisplay(false);
		break;
	case DEMOD_DIGI:
	{
		switch(digital_mode)
		{
		case DigitalMode_FreeDV:
#ifdef USE_FREEDV
			FreeDv_DisplayClear();
#endif
			break;
		case DigitalMode_RTTY:
			UiDriver_TextMsgClear();
			break;
		default:
			break;
		}
	}
	break;
	case DEMOD_AM:
	case DEMOD_SAM:
		UiSpectrum_InitCwSnapDisplay(false);
	break;

	default:
		break;
	}
}

/**
 * @brief prepares mode specific ui elements run the mode
 */
void UiDriver_ModeSpecificDisplayPrepare(uint8_t dmod_mode, uint8_t digital_mode)
{
	switch(dmod_mode)
	{
	case DEMOD_CW:
	    if (ts.cw_decoder_enable == true)
	    {
	        CwDecoder_WpmDisplayClearOrPrepare(true);
	    }
	    if(cw_decoder_config.snap_enable == true && ts.dmod_mode == DEMOD_CW)
	    {
	    	UiSpectrum_InitCwSnapDisplay(true);
	    }
		break;
	case DEMOD_DIGI:
	{
		switch(digital_mode)
		{
		case DigitalMode_FreeDV:
#ifdef USE_FREEDV
			FreeDv_DisplayPrepare();
#endif
			break;
		case DigitalMode_RTTY:
			UiDriver_TextMsgClear();
			break;
		default:
			break;
		}
	}
	break;
	case DEMOD_AM:
	case DEMOD_SAM:
	    if(cw_decoder_config.snap_enable == true)
	    {
	    	UiSpectrum_InitCwSnapDisplay(true);
	    }
	break;

	default:
		break;
	}
}

void UiDriver_UpdateDemodSpecificDisplayAfterParamChange()
{
    // clear display content specific for the old mode
    UiDriver_ModeSpecificDisplayClear(ui_driver_state.dmod_mode,ui_driver_state.digital_mode);
    // prepare mode specific UI elements used in the new mode
    UiDriver_ModeSpecificDisplayPrepare(ts.dmod_mode,ts.digital_mode);
    ui_driver_state.dmod_mode = ts.dmod_mode;
    ui_driver_state.digital_mode = ts.digital_mode;
}

void UiDriver_UpdateDisplayAfterParamChange()
{
    // TODO Maybe we should split this, so that we clear BEFORE doing the general stuff
    // and prepare after, but for now it should work this way
	if (UiDriver_IsDemodModeChange())
	{
	    UiDriver_UpdateDemodSpecificDisplayAfterParamChange();
	}

	// below are the always present UI elements
	UiDriver_FrequencyUpdateLOandDisplay(false);   // update frequency display without checking encoder

	UiDriver_DisplayDemodMode();

	UiDriver_DisplayMemoryLabel();

	UiDriver_DisplayFilter();    // make certain that numerical on-screen bandwidth indicator is updated

	UiSpectrum_DisplayFilterBW();  // update on-screen filter bandwidth indicator (graphical)

	UiDriver_RefreshEncoderDisplay();

	if(ts.menu_mode)    // are we in menu mode?
	{
		UiMenu_RenderMenu(MENU_RENDER_ONLY);    // yes, update display when we change modes
	}
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverPressHoldStep
//* Object              : Select the step size for the press-and-hold of the step size button
//* Input Parameters    : 0=Decrease step size, 1=Increase step size
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_PressHoldStep(uchar is_up)
{
	ulong	minus_idx, plus_idx;

	switch(df.selected_idx)	 		// select appropriate "alternate" step size based on current step size
	{
	case T_STEP_1HZ_IDX:	// 1Hz step size
	case T_STEP_10HZ_IDX:	// 10Hz step size
		minus_idx = T_STEP_1HZ_IDX;		// use 1 Hz as small step size
		plus_idx = T_STEP_1KHZ_IDX;		// use 1 kHz as large step size
		break;
	case T_STEP_100HZ_IDX:	// 100Hz step size
		minus_idx = T_STEP_10HZ_IDX;		// use 10 Hz as small step size
		plus_idx = T_STEP_1KHZ_IDX;		// use 1 kHz as large step size
		break;
	case T_STEP_10KHZ_IDX:	// 10 kHz step size
	case T_STEP_100KHZ_IDX:	// 100 kHz step size
		minus_idx = T_STEP_100HZ_IDX;	// use 100 Hz as small step size
		plus_idx = T_STEP_100KHZ_IDX;	// use 100 kHz as large step size
		break;
	case T_STEP_1KHZ_IDX:	// 1 kHz step size
	default:
		minus_idx = T_STEP_10HZ_IDX;	// use 10 Hz as small step size
		plus_idx = T_STEP_10KHZ_IDX;	// use 10 kHz as large step size
		break;
	}

	if(!is_up)	 		// temporary decrease of step size
	{
		ts.tune_step = STEP_PRESS_MINUS;
		ts.tune_step_idx_holder = df.selected_idx;
		if(df.selected_idx)
			df.tuning_step	= tune_steps[minus_idx];
		df.selected_idx = minus_idx;
	}
	else	 			// temporary increase of step size
	{
		ts.tune_step = STEP_PRESS_PLUS;
		ts.tune_step_idx_holder = df.selected_idx;
		df.tuning_step	= tune_steps[plus_idx];
		df.selected_idx = plus_idx;
	}
	//
	UiDriver_DisplayFreqStepSize();		// update display
}

void UiDriver_DisplayDemodMode()
{
	// Clear control
	char* txt = "???";
	uint16_t clr_fg = White,clr_bg = Blue;

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	switch(ts.dmod_mode)
	{
	case DEMOD_USB:
		txt = "USB";
		break;
	case DEMOD_LSB:
		txt = "LSB";
		break;
	case DEMOD_SAM:
		if(ads.sam_sideband == SAM_SIDEBAND_LSB)
		{
			txt = "SAM-L";
		}
		else if (ads.sam_sideband == SAM_SIDEBAND_USB)
		{
			txt = "SAM-U";
		}
		else
		{
			txt = "SAM";
		}
		break;
	case DEMOD_AM:
		txt = "AM";
		break;
	case DEMOD_FM:
		txt = RadioManagement_FmDevIs5khz() ? "FM-W" : "FM-N";
		{
			if(ts.txrx_mode == TRX_MODE_RX)
			{
				if(ads.fm_squelched == false)
				{
					// is audio not squelched?
					if((ads.fm_subaudible_tone_detected) && (ts.fm_subaudible_tone_det_select))
					{
						// is tone decoding enabled AND a tone being detected?
						clr_fg =  Black;
						clr_bg = Red2;	// Not squelched, passing audio - change color!
					}
					else  	// tone decoder disabled - squelch only
					{
						clr_fg = Black;
						clr_bg = White;	// Not squelched, passing audio - change color, but different from tone
					}
				}
			}
			else if(ts.txrx_mode == TRX_MODE_TX)	 	// in transmit mode?
			{
				if(ads.fm_tone_burst_active)	 		// yes - is tone burst active?
				{
					clr_fg = Black;
					clr_bg = Yellow;	// Yes, make "FM" yellow
				}
			}
			break;
		}
	case DEMOD_CW:
		txt = ts.cw_lsb?"CW-L":"CW-U";
		break;
	case DEMOD_DIGI:
		switch(ts.digital_mode)
		{
		case DigitalMode_RTTY:
			txt = ts.digi_lsb?"RT-L":"RT-U";
			break;
		default:
			txt = ts.digi_lsb?"DI-L":"DI-U";
		}
		break;
		default:
			break;
	}
	UiLcdHy28_PrintTextCentered(POS_DEMOD_MODE_MASK_X,POS_DEMOD_MODE_MASK_Y,POS_DEMOD_MODE_MASK_W,txt,clr_fg,clr_bg,0);

	UiDriver_DisplayDigitalMode();
}


void UiDriver_DisplayFreqStepSize()
{

	int	line_loc;
	static	bool	step_line = 0;	// used to indicate the presence of a step line
	uint32_t	color;
	uint32_t 	stepsize_background;

	color = ts.tune_step?Cyan:White;		// is this a "Temporary" step size from press-and-hold?
	stepsize_background = (ts.flags1 & FLAGS1_DYN_TUNE_ENABLE)?Blue:Black;
	// dynamic_tuning active -> yes, display on Grey3

	if(step_line)	 	// Remove underline indicating step size if one had been drawn
	{
		UiLcdHy28_DrawStraightLineDouble((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * 3)),(POS_TUNE_FREQ_Y + 24),(LARGE_FONT_WIDTH*7),LCD_DIR_HORIZONTAL,Black);
		UiLcdHy28_DrawStraightLineDouble((POS_TUNE_SPLIT_FREQ_X + (SMALL_FONT_WIDTH * 3)),(POS_TUNE_FREQ_Y + 24),(SMALL_FONT_WIDTH*7),LCD_DIR_HORIZONTAL,Black);
	}

	// Blank old step size
	// UiLcdHy28_DrawFullRect(POS_TUNE_STEP_X,POS_TUNE_STEP_Y-1,POS_TUNE_STEP_MASK_H,POS_TUNE_STEP_MASK_W,stepsize_background);

	{
		char step_name[10];

		// I know the code below will not win the price for the most readable code
		// ever. But it does the job of display any freq step somewhat reasonable.
		// khz/Mhz only whole  khz/Mhz is shown, no fraction
		// showing fractions would require some more coding, which is not yet necessary
		const uint32_t pow10 = log10f(df.tuning_step);
		line_loc = 9 - pow10 - pow10/3;
		if (line_loc < 0)
		{
			line_loc = 0;
		}
		const char* stepUnitPrefix[] = { "","k","M","G","T"};
		snprintf(step_name,10,"%d%sHz",(int)(df.tuning_step/exp10((pow10/3)*3)), stepUnitPrefix[pow10/3]);

		UiLcdHy28_PrintTextCentered(POS_TUNE_STEP_X,POS_TUNE_STEP_Y,POS_TUNE_STEP_MASK_W,step_name,color,stepsize_background,0);
	}
	//
	if((ts.freq_step_config & FREQ_STEP_SHOW_MARKER) && line_loc > 0)	 		// is frequency step marker line enabled?
	{
		if(is_splitmode())
		{
			UiLcdHy28_DrawStraightLineDouble((POS_TUNE_SPLIT_FREQ_X + (SMALL_FONT_WIDTH * line_loc)),(POS_TUNE_FREQ_Y + 24),(SMALL_FONT_WIDTH),LCD_DIR_HORIZONTAL,White);
		}
		else
		{
			UiLcdHy28_DrawStraightLineDouble((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * line_loc)),(POS_TUNE_FREQ_Y + 24),(LARGE_FONT_WIDTH),LCD_DIR_HORIZONTAL,White);
		}
		step_line = 1;	// indicate that a line under the step size had been drawn
	}
	else	// marker line not enabled
	{
		step_line = 0;	// we don't need to erase "step size" marker line in the future
	}
}


typedef struct
{
	uint32_t start;
	uint32_t end;
	const char* name;
} BandGenInfo;

const BandGenInfo bandGenInfo[] =
{
		{150000, 285000, "LW" },
		{525000, 1605000, "MW" },
		{2300000, 2495000, "120m" },
		{3200000, 3400000, "90m" },
		{3900000, 4000000, "75m" },
		{4750000, 5060000, "60m" },
		{5950000, 6200000, "49m" },
		{7300000, 7350000, "41m" },
		{9400000, 9900000, "31m" },
		{11600000, 12100000, "25m" },
		{13570000, 13870000, "22m" },
		{15100000, 15800000, "19m" },
		{17480000, 17900000, "16m" },
		{18900000, 19020000, "15m" },
		{21450000, 21750000, "13m" },
		{25670000, 26100000, "11m" },
		{26965000, 27405000, "11m" },
		{ 0,  0,             "Gen" }
};

//*----------------------------------------------------------------------------
static void UiDriver_DisplayMemoryLabel()
{
	char txt[12];
	uint32_t col = White;
	if (ts.band < MAX_BAND_NUM && ts.cat_band_index == 255)
	{
		snprintf(txt,12,"Bnd%s ", bandInfo[ts.band].name);
	}
	if (ts.cat_band_index != 255)		// no band storage place active because of "CAT running in sandbox"
	{
		snprintf(txt,12,"  CAT  ");
	}
	UiLcdHy28_PrintText(POS_MEMORYLABEL_X,  POS_MEMORYLABEL_Y,txt,col,Black,0);
}


static void UiDriver_DisplayBand(uchar band)
{
	const char* bandName;
	bool print_bc_name = true;
	int idx;

	if (band < MAX_BAND_NUM)
	{
		ulong col;
		// Clear control
		if (band == BAND_MODE_GEN)
		{
			for (idx = 0; bandGenInfo[idx].start !=0; idx++)
			{
				if (df.tune_old/TUNE_MULT >= bandGenInfo[idx].start && df.tune_old/TUNE_MULT < bandGenInfo[idx].end)
				{
					break; // found match
				}
			}

			if (bandGenInfo[idx].start !=0)
				// Print name of BC band in yellow, if frequency is within a broadcast band
				col = Yellow;
			else
				col = Orange;

			if  (bandGenInfo[idx].start == 26965000)
				col = Blue;		// CB radio == blue


			if (idx == ts.bc_band)
				print_bc_name = false;
			ts.bc_band =idx;

			bandName = bandGenInfo[idx].name;
		}
		else
		{
			print_bc_name = true;
			col = Orange;
			bandName = bandInfo[band].name;
			ts.bc_band = 0xff;
		}
		if (print_bc_name)
		{
			UiLcdHy28_DrawFullRect(POS_BAND_MODE_MASK_X,POS_BAND_MODE_MASK_Y,POS_BAND_MODE_MASK_H,POS_BAND_MODE_MASK_W,Black);
			UiLcdHy28_PrintTextRight(POS_BAND_MODE_X + 5*8,POS_BAND_MODE_Y,bandName,col,Black,0);
		}

	}
	// add indicator for broadcast bands here
	// if Band = "Gen" AND frequency inside one of the broadcast bands, print name of the band
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverInitMainFreqDisplay
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_CreateMainFreqDisplay()
{
	UiDriver_FButton_F3MemSplit();
	if((is_splitmode()))	 	// are we in SPLIT mode?
	{
		UiLcdHy28_PrintText(POS_TUNE_FREQ_X-16,POS_TUNE_FREQ_Y,"          ",White,Black,1);	// clear large frequency digits
		UiDriver_DisplaySplitFreqLabels();
	}
	UiDriver_DisplayFreqStepSize();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateFunctionButtons
//* Object              : function keys based on decoder mode
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_CreateFunctionButtons(bool full_repaint)
{
	// Create bottom bar
	if(full_repaint)
	{
		for (int i = 0; i < 5; i++)
		{
			UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + (POS_BOTTOM_BAR_BUTTON_W+1)*i),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		}
	}

	// Button F1
	UiDriver_DisplayFButton_F1MenuExit();

	// Button F2
	UiDriver_DisplayFButton_F2SnapMeter();

	// Button F3
	UiDriver_FButton_F3MemSplit();

	// Button F4
	UiDriver_FButton_F4ActiveVFO();

	// Button F5
	UiDriver_FButton_F5Tune();
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDrawSMeter
//* Object              : draw the part of the S meter
//* Input Parameters    : uchar color
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_CreateDesktop()
{
	// Backlight off - hide startup logo
	UiLcdHy28_BacklightEnable(false);

	// Clear display
	UiLcdHy28_LcdClear(Black);

	// Create Band value
	UiDriver_DisplayBand(ts.band);

	// Frequency
	UiDriver_CreateMainFreqDisplay();

	// Function buttons
	UiDriver_CreateFunctionButtons(true);

	// S-meter
	UiDriver_CreateMeters();

	// Spectrum scope
	UiSpectrum_Init();

	UiDriver_RefreshEncoderDisplay();

	// Power level
	UiDriver_DisplayPowerLevel();


	UiDriver_CreateVoltageDisplay();

	// Create temperature
	UiDriver_CreateTemperatureDisplay();

	// Set correct frequency
	UiDriver_FrequencyUpdateLOandDisplay(true);

	UiDriver_UpdateDisplayAfterParamChange();

	// Backlight on - only when all is drawn
	UiLcdHy28_BacklightEnable(true);
}


static void UiDriver_DrawSMeter(uint16_t color)
{
	// Draw top line
	UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X +  18),(POS_SM_IND_Y + 20),92,LCD_DIR_HORIZONTAL,color);
	// Draw s markers on top white line
	for(uint16_t i = 0; i < 10; i++)
	{
		uint8_t 	v_s;
		// Draw s text, only odd numbers
		if(i%2)
		{
			v_s = 5;
		}
		else
		{
			v_s = 3;
		}
		// Lines
		UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,color);
	}
}
//



#define BTM_MINUS 14

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDeleteSMeter
//* Object              : delete the S meter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_DeleteMeters()
{
	UiLcdHy28_DrawFullRect(POS_SM_IND_X+1,POS_SM_IND_Y+1,SM_IND_H ,SM_IND_W,Black);
}

static void UiDriver_DeleteSMeterLabels()
{
	UiLcdHy28_DrawFullRect(POS_SM_IND_X+1,POS_SM_IND_Y+1,21,SM_IND_W,Black);
}


static void UiDriver_DrawPowerMeterLabels()
{
	uchar   i;
	char    num[20];

	// Leading text
	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 5),"P",  White,Black,4);

	UiLcdHy28_PrintText((POS_SM_IND_X + 185),(POS_SM_IND_Y + 5)," W",White,Black,4);

	// Draw middle line
	UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 18),(POS_SM_IND_Y + 20),170,LCD_DIR_HORIZONTAL,White);
	// S Meter -> Y + 20

	// Draw s markers on middle white line
	for(i = 0; i < 12; i++)
	{
		uint8_t v_s;
		if(i < 10)
		{
			num[0] = i + 0x30;
			num[1] = 0;
		}
		else
		{
			num[0] = i/10 + 0x30;
			num[1] = i%10 + 0x30;
			num[2] = 0;
		}

		// Draw s text, only odd numbers
		if(!(i%2))
		{
			UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*15),(POS_SM_IND_Y + 5),num,White,Black,4);
		}
		// Lines
		v_s=(i%2)?3:5;
		UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*15),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,White);
	}


}
static void UiDriver_DrawSMeterLabels()
{
	uchar   i,v_s;
	char    num[20];

	// Draw top line
	UiLcdHy28_DrawFullRect((POS_SM_IND_X +  18),(POS_SM_IND_Y + 20),2,92,White);
	UiLcdHy28_DrawFullRect((POS_SM_IND_X +  113),(POS_SM_IND_Y + 20),2,75,Green);

	// Leading text
	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y +  5),"S",  White,Black,4);

	// Trailing text
	UiLcdHy28_PrintText((POS_SM_IND_X + 185),(POS_SM_IND_Y + 5), "dB",Green,Black,4);


	num[1] = 0;
	// Draw s markers on top white line
	for(i = 0; i < 10; i++)
	{
		num[0] = i + 0x30;

		// Draw s text, only odd numbers
		if(i%2)
		{
			UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 4 + i*10),(POS_SM_IND_Y + 5),num,White,Black,4);
			v_s = 5;
		}
		else
			v_s = 3;

		// Lines
		UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,White);
	}

	num[1] = 0x30;
	num[2] = 0x00;
	// Draw s markers on top green line
	for(i = 0; i < 4; i++)
	{
		// Prepare text
		num[0] = i*2 + 0x30;

		if(i)
		{
			// Draw text
			UiLcdHy28_PrintText(((POS_SM_IND_X + 108) - 6 + i*20),(POS_SM_IND_Y + 5),num,Green,Black,4);

			// Draw vert lines
			UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 108) + i*20),(POS_SM_IND_Y + 15),5,LCD_DIR_VERTICAL,Green);
		}
	}

}

static void UiDriver_CreateMeters()
{
	uchar 	i;
	char	num[20];
	int		col;

	UiLcdHy28_DrawEmptyRect(POS_SM_IND_X,POS_SM_IND_Y,SM_IND_H,SM_IND_W + 2,Grey);

	if (ts.txrx_mode == TRX_MODE_RX)
	{
		UiDriver_DrawSMeterLabels();
	}
	else
	{
		UiDriver_DrawPowerMeterLabels();
	}

	if(ts.tx_meter_mode == METER_SWR)
	{
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59 - BTM_MINUS),"SWR",Red2,Black,4);

		// Draw bottom line for SWR indicator
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55 - BTM_MINUS), 62,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 83),(POS_SM_IND_Y + 55 - BTM_MINUS),105,LCD_DIR_HORIZONTAL,Red);
		col = White;

		// Draw S markers on middle white line
		for(i = 0; i < 12; i++)
		{
			if(i > 6) col = Red;

			if(!(i%2))
			{
				if(i)
				{
					num[0] = i/2 + 0x30;
					num[1] = 0;

					// Text
					UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*10),(POS_SM_IND_Y + 59 - BTM_MINUS),num,White,Black,4);

					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55 - BTM_MINUS) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55 - BTM_MINUS) - 7),7,LCD_DIR_VERTICAL,col);
				}
			}
		}
	}
	else if(ts.tx_meter_mode == METER_ALC)
	{
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59 - BTM_MINUS),"ALC",Yellow,Black,4);


		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55 - BTM_MINUS), 62,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 83),(POS_SM_IND_Y + 55 - BTM_MINUS),105,LCD_DIR_HORIZONTAL,Red);

		col = White;

		// Draw markers on middle line
		for(i = 0; i < 17; i++)
		{
			if(i > 6) col = Red;
			if(!(i%2))
			{
				if(i)
				{
					snprintf(num,20,"%d",(i*2));
					// Text
					UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*10),(POS_SM_IND_Y + 59 - BTM_MINUS),num,White,Black,4);

					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55 - BTM_MINUS) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55 - BTM_MINUS) - 7),7,LCD_DIR_VERTICAL,col);
				}
			}
		}
	}
	else if(ts.tx_meter_mode == METER_AUDIO)
	{
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59 - BTM_MINUS),"AUD",Cyan,Black,4);

		// Draw bottom line for SWR indicator
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55 - BTM_MINUS), 108,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 129),(POS_SM_IND_Y + 55 - BTM_MINUS),59,LCD_DIR_HORIZONTAL,Red);
		col = White;

		// Draw markers on middle line
		for(i = 0; i < 17; i++)
		{
			if(i > 10) col = Red;
			if(!(i%2))
			{
				if(i)
				{
					snprintf(num,20,"%d",(i*2)-20);
					// Text
					UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*10),(POS_SM_IND_Y + 59 - BTM_MINUS),num,White,Black,4);

					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55 - BTM_MINUS) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55 - BTM_MINUS) - 7),7,LCD_DIR_VERTICAL,col);
				}
			}
		}
	}
	// Draw meters
	UiDriver_UpdateTopMeterA(34);
	UiDriver_UpdateTopMeterA(0);
	UiDriver_UpdateBtmMeter(34, 34);
	UiDriver_UpdateBtmMeter(0, 34);

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateTopMeterA
//* Object              : redraw indicator, same like upper implementation
//* Input Parameters    : but no hold
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
#define SMETER_MAX_LEVEL 33

enum
{
	METER_TOP = 0,
	METER_BTM,
	METER_NUM
};
typedef struct MeterState_s
{
	uint8_t last;
	uint8_t last_warn;
} MeterState;

static MeterState meters[METER_NUM];

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateBtmMeter
//* Object              : redraw indicator
//* Input Parameters    : val=indicated value, warn=red warning threshold
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

static void UiDriver_UpdateMeter(uchar val, uchar warn, uint32_t color_norm, uint8_t meterId)
{
	uchar     i;
	const uint8_t v_s = 3;
	uint32_t       col = color_norm;
	uint8_t from, to;
	uint8_t from_warn = 255;

	uint16_t ypos = meterId==METER_TOP?(POS_SM_IND_Y + 28):(POS_SM_IND_Y + 51 - BTM_MINUS);

	// limit meter
	if(val > SMETER_MAX_LEVEL)
	{
		val = SMETER_MAX_LEVEL;
	}
	if (warn == 0)
	{
		warn = SMETER_MAX_LEVEL+1;    // never warn if warn == 0
	}

	if(warn != meters[meterId].last_warn)
	{
		if (warn < meters[meterId].last_warn)
		{
			from_warn = warn;
		}
		else
		{
			from_warn = meters[meterId].last_warn;
		}
	}


	if(val != meters[meterId].last || from_warn != 255)
	{

		// decide if we need to draw more boxes or delete some
		if (val > meters[meterId].last)
		{
			// we will draw more active boxes
			from = meters[meterId].last;
			to = val+1;

		}
		else
		{
			from = val;
			to   = meters[meterId].last+1;
		}
		if (from_warn < from)
		{
			from = from_warn;
		}

		// Draw indicator
		// we never draw a zero, so we start from 1 min
		if (from == 0)
		{
			from = 1;
		}

		for(i = from; i < to; i++)
		{
			if (i>val)
			{
				col = Grid;    // switch to delete color
			}
			if((i >= warn) && warn && col != Grid)    // is level above "warning" color? (is "warn" is zero, disable warning)
			{
				col = Red2;                 // yes - display values above that color in red
			}
			// Lines
			UiLcdHy28_DrawStraightLineTriple(((POS_SM_IND_X + 18) + i*5),(ypos - v_s),v_s,LCD_DIR_VERTICAL,col);
		}

		meters[meterId].last = val;
		meters[meterId].last_warn = warn;
	}
}


static void UiDriver_UpdateTopMeterA(uchar val)
{
	ulong clr;
	UiMenu_MapColors(ts.meter_colour_up,NULL,&clr);
	UiDriver_UpdateMeter(val,SMETER_MAX_LEVEL+1,clr,METER_TOP);
}

/**
 * @brief updates the lower meter
 * @param val the value to display, max value is S_METER_MAX, min is 0, values will be limited to range
 * @param warn the level value from which the meter is going to show warning indication
 */
static void UiDriver_UpdateBtmMeter(float val, uchar warn)
{
	ulong clr;
	UiMenu_MapColors(ts.meter_colour_down,NULL,&clr);
	if (val < 0)
	{
		val = 0;
	}
	if (val > S_METER_MAX)
	{
		val = S_METER_MAX;
	}
	UiDriver_UpdateMeter(val,warn,clr,METER_BTM);
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverInitFrequency
//* Object              : set default values, some could be overwritten later
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_InitFrequency()
{
	ulong i;

	// Clear band values array
	for(i = 0; i < MAX_BANDS; i++)
	{
		vfo[VFO_A].band[i].dial_value = 0xFFFFFFFF;	// clear dial values
		vfo[VFO_A].band[i].decod_mode = DEMOD_USB; 	// clear decode mode
		vfo[VFO_B].band[i].dial_value = 0xFFFFFFFF;  // clear dial values
		vfo[VFO_B].band[i].decod_mode = DEMOD_USB;   // clear decode mode
	}

	// Lower bands default to LSB mode
	// TODO: This needs to be checked, some even lower bands have higher numbers now
	for(i = 0; i < 4; i++)
	{
		vfo[VFO_A].band[i].decod_mode = DEMOD_LSB;
		vfo[VFO_B].band[i].decod_mode = DEMOD_LSB;
	}
	// Init frequency publics(set diff values so update on LCD will be done)
	df.tune_old 	= bandInfo[ts.band].tune;
	df.tune_new 	= bandInfo[ts.band].tune;
	df.selected_idx = 3; 		// 1 Khz startup step
	df.tuning_step	= tune_steps[df.selected_idx];
	df.temp_factor	= 0;
	df.temp_factor_changed = false;
	df.temp_enabled = 0;		// startup state of TCXO

	// Set virtual segments initial value (diff than zero!)
	df.dial_digits[8]	= 0;
	df.dial_digits[7]	= 0;
	df.dial_digits[6]	= 0;
	df.dial_digits[5]	= 0;
	df.dial_digits[4]	= 0;
	df.dial_digits[3]	= 0;
	df.dial_digits[2]	= 0;
	df.dial_digits[1]	= 0;
	df.dial_digits[0]	= 0;
}

/**
 * @brief Checks in which band the current frequency lies and updates display only if changed
 *
 * @param freq frequency in Hz
 * @returns band index (0 - (MAX_BANDS-1))
 */

uchar UiDriver_DisplayBandForFreq(ulong freq)
{
	// here we maintain our local state of the last band shown
	static uint8_t ui_band_scan_old = 99;
	uint8_t band_scan = RadioManagement_GetBand(freq);
	if(band_scan != ui_band_scan_old || band_scan == BAND_MODE_GEN)        // yes, did the band actually change?
	{
		UiDriver_DisplayBand(band_scan);    // yes, update the display with the current band
	}
	ui_band_scan_old = band_scan;
	return band_scan;
}




/*
 * @brief Changes the tune frequency according to mode and other settings
 * @param dial_freq The desired dial frequency in Hz (not the tune frequency of the LO)
 * @returns true if the change was executed  (even if it is not tunable freq), false if the change is pending
 */
/*
 * @brief Check if a frequency is tunable
 * @returns SI570_OK, SI570_LARGE_STEP, SI570_TUNE_LIMITED if ok, SI570_TUNE_IMPOSSIBLE if not OK
 */
/*
 * @brief Used to update the individual vfo displays, not meant to be called directly except when changing LO
 * @brief parameters (in this case use (true,0)), use UiDriver_FrequencyUpdateLOandDisplay(full_update) instead
 *
 * @param force_update true = unconditionally update synthesizer EVEN IF frequency did not change
 * @param mode  =0 automatic, 1=force large, 2=force small, upper (RX), 3 = small, lower (TX)
 *
 * WARNING:  If called with "mode = 3", you must ALWAYS call again with "mode = 2" to reset internal variables.
 */
/*
 * @brief change LO freq to match df.tune_new freq according to mode without updating the ui
 *
 * @param trx_mode The mode which the frequency is being used for (TRX_MODE_TX/TRX_MODE_RX)
 */
void UiDriver_UpdateFrequency(bool force_update, enum UpdateFrequencyMode_t mode)
{

	// FIXME: Don't like the handling of lo_result if in Split mode and transmitting
	uint32_t		dial_freq;
	Si570_ResultCodes       lo_result = SI570_OK;
	bool        lo_change_not_pending = true;

	if(mode == UFM_SMALL_TX)
		// are we updating the TX frequency (small, lower display)?
	{
		dial_freq = RadioManagement_GetTXDialFrequency() / TUNE_MULT;

		// we check with the si570 code if the frequency is tunable, we do not tune to it.
		lo_result = RadioManagement_ValidateFrequencyForTX(dial_freq);

	}
	else
	{
		dial_freq = df.tune_new/TUNE_MULT;

		lo_change_not_pending =  RadioManagement_ChangeFrequency(force_update, dial_freq, ts.txrx_mode);
		lo_result = ts.last_lo_result;   // use last ts.lo_result
	}

	if (mode == UFM_SMALL_RX && ts.txrx_mode == TRX_MODE_TX )
		// we are not going to show the tx frequency here (aka dial_freq) so we cannot use dial_freq
	{
		dial_freq = RadioManagement_GetRXDialFrequency() / TUNE_MULT;

		// we check with the si570 code if the frequency is tunable, we do not tune to it.
		// lo_result = RadioManagement_ValidateFrequencyForTX(dial_freq);
	}

	// ALL UI CODE BELOW
	{
		uint32_t clr;

		if (lo_change_not_pending)
		{

			if (mode != UFM_SMALL_TX)
			{
				UiDriver_DisplayBandForFreq(dial_freq);
				// check which band in which we are currently tuning and update the display

				UiDriver_UpdateLcdFreq(RadioManagement_GetRXDialFrequency() / TUNE_MULT ,White, UFM_SECONDARY);
				// set mode parameter to UFM_SECONDARY to update secondary display (it shows real RX frequency if RIT is being used)
				// color argument is not being used by secondary display
			}

			switch(lo_result)
			{
			case SI570_TUNE_IMPOSSIBLE:
				clr = Orange; // Color in orange if there was a problem setting frequency
				break;
			case SI570_TUNE_LIMITED:
				clr = Yellow; // Color in yellow if there was a problem setting frequency exactly
				break;
			case SI570_LARGE_STEP:
			case SI570_OK:
				clr = White;
				break;
			default:
				clr = Red; // a serious error happened, i.e. I2C not working etc.
			}
		}
		else
		{
			// we did not execute the change, so we show the freq in Blue.
			// this will turn into the appropriate color the moment the tuning
			// happens.
			// Use white in releases, many complained about the  Blue digits
			clr = White; // Blue;
		}
		// Update frequency display
		UiDriver_UpdateLcdFreq(dial_freq, clr, mode);
	}
}



static void UiDriver_UpdateFreqDisplay(ulong dial_freq, uint8_t* dial_digits, ulong pos_x_loc, ulong font_width, ulong pos_y_loc, ushort color, uchar digit_size)
{
	{

#define MAX_DIGITS 9
		ulong dial_freq_temp;
		int8_t pos_mult[MAX_DIGITS] = {9, 8, 7, 5, 4, 3, 1, 0, -1};
		uint32_t idx;
		uint8_t digits[MAX_DIGITS];
		char digit[2];
		uint8_t last_non_zero = 0;

		// Terminate string for digit
		digit[1] = 0;
		// calculate the digits
		dial_freq_temp = dial_freq;
		for (idx = 0; idx < MAX_DIGITS; idx++)
		{
			digits[idx] = dial_freq_temp % 10;
			dial_freq_temp /= 10;
			if (digits[idx] != 0) last_non_zero = idx;
		}
		for (idx = 0; idx < MAX_DIGITS; idx++)
		{
			// -----------------------
			// See if digit needs update
			if ((digits[idx] != dial_digits[idx]) || ts.refresh_freq_disp)
			{
				bool noshow = idx > last_non_zero;
				// don't show leading zeros, except for the 0th digits
				digit[0] = noshow?' ':0x30 + (digits[idx] & 0x0F);
				// Update segment
				UiLcdHy28_PrintText((pos_x_loc + pos_mult[idx] * font_width), pos_y_loc, digit, color, Black, digit_size);
			}
		}

		for (idx = 3; idx < MAX_DIGITS; idx+=3)
		{
			bool noshow = last_non_zero < idx;
			digit[0] = noshow?' ':'.';
			UiLcdHy28_PrintText(pos_x_loc+ (pos_mult[idx]+1) * font_width,pos_y_loc,digit,color,Black,digit_size);
		}

	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateLcdFreq
//* Object              : this function will split LCD freq display control
//* Object              : and update as it is 7 segments indicator
//* Input Parameters    : freq=freq (Hz), color=color, mode: 0 = auto, 1= force normal (large digits), 2= force upper, small, 3 = force lower, small, 4 = secondary display
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_UpdateLcdFreq(ulong dial_freq,ushort color, ushort mode)
{
	uchar		digit_size;
	ulong		pos_y_loc;
	ulong		pos_x_loc;
	ulong		font_width;
	uint8_t*		digits_ptr;

	//
	//
	if(ts.frequency_lock)
	{
		// Frequency is locked - change color of display
		color = Grey;
	}

	//
	if(mode == UFM_AUTOMATIC)
	{
		if(is_splitmode())  	// in "split" mode?
		{
			mode = UFM_SMALL_RX;				// yes - update upper, small digits (receive frequency)
		}
		else
		{
			mode = UFM_LARGE;				// NOT in split mode:  large, normal-sized digits
		}
	}

	// if (mode != UFM_SECONDARY) {
	ts.refresh_freq_disp = true; //because of coloured digits...
	// }
	if(ts.xverter_mode)	 	// transverter mode active?
	{
		dial_freq *= (ulong)ts.xverter_mode;	// yes - scale by LO multiplier
		dial_freq += ts.xverter_offset;	// add transverter frequency offset
		if(dial_freq > 1000000000)		// over 1000 MHz?
			dial_freq -= 1000000000;		// yes, offset to prevent overflow of display
		if(ts.xverter_mode && mode != UFM_SECONDARY)	// if in transverter mode, frequency is yellow unless we do the secondary display
			color = Yellow;
	}

	// Handle frequency display offset in "CW RX" modes
	if(ts.dmod_mode == DEMOD_CW)	 		// In CW mode?
	{
		switch(ts.cw_offset_mode)
		{
		case CW_OFFSET_LSB_RX:	// Yes - In an LSB mode with display offset?
		case CW_OFFSET_USB_RX:	// In a USB mode with display offset?
		case CW_OFFSET_AUTO_RX:	// in "auto" mode with display offset?
			if(ts.cw_lsb)
			{
				dial_freq -= ts.cw_sidetone_freq;		// yes - LSB - lower display frequency by sidetone amount
			}
			else
			{
				dial_freq += ts.cw_sidetone_freq;		// yes - USB - raise display frequency by sidetone amount
			}
			break;
		}
	}

	switch(mode)
	{
	case UFM_SMALL_RX:
		digits_ptr  = df.dial_digits;
		digit_size = 0;
		pos_y_loc = POS_TUNE_FREQ_Y;
		pos_x_loc = POS_TUNE_SPLIT_FREQ_X;
		font_width = SMALL_FONT_WIDTH;
		break;
	case UFM_SMALL_TX:					// small digits in lower location
		digits_ptr  = df.dial_digits;
		digit_size = 0;
		pos_y_loc = POS_TUNE_SPLIT_FREQ_Y_TX;
		pos_x_loc = POS_TUNE_SPLIT_FREQ_X;
		font_width = SMALL_FONT_WIDTH;
		break;
	case UFM_SECONDARY:
		digits_ptr  = df.sdial_digits;
		digit_size = 0;
		pos_y_loc = POS_TUNE_SFREQ_Y;
		pos_x_loc = POS_TUNE_SFREQ_X;
		font_width = SMALL_FONT_WIDTH;
		break;
	case UFM_LARGE:
	default:			// default:  normal sized (large) digits
		digits_ptr  = df.dial_digits;
		digit_size = 1;
		pos_y_loc = POS_TUNE_FREQ_Y;
		pos_x_loc = POS_TUNE_FREQ_X;
		font_width = LARGE_FONT_WIDTH;
	}
	// in SAM mode, never display any RIT etc., but
	// use small display for display of the carrier frequency that the PLL has locked to
	if(((ts.dmod_mode == DEMOD_SAM && mode == UFM_SMALL_RX) || (ts.dmod_mode == DEMOD_SAM && mode == UFM_SECONDARY)))
	{
		digits_ptr  = df.sdial_digits;
		digit_size = 0;
		pos_y_loc = POS_TUNE_SFREQ_Y;
		pos_x_loc = POS_TUNE_SFREQ_X;
		font_width = SMALL_FONT_WIDTH;
		UiDriver_UpdateFreqDisplay(dial_freq + ads.carrier_freq_offset, digits_ptr, pos_x_loc, font_width, pos_y_loc, Yellow, digit_size);
	}
	else {
		UiDriver_UpdateFreqDisplay(dial_freq, digits_ptr, pos_x_loc, font_width, pos_y_loc, color, digit_size);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeTuningStep
//* Object              : Change tunning step
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriver_ChangeTuningStep(uchar is_up)
{
	ulong 	idx = df.selected_idx;
	uint8_t idx_limit = T_STEP_MAX_STEPS -1;

	if((!ts.xvtr_adjust_flag) && (!ts.xverter_mode))
	{
		// are we NOT in "transverter adjust" or transverter mode *NOT* on?
		idx_limit = T_STEP_100KHZ_IDX;
	}

	if(is_up)
	{
		idx= (idx>=idx_limit)?0:idx+1;
		// 9kHz step only on MW and LW
		if(idx == T_STEP_9KHZ_IDX && ((df.tune_old/4) > 1600001))
			idx ++;
	}
	else
	{
		idx= (idx==0)?idx_limit:idx-1;
		// 9kHz step only on MW and LW
		if(idx == T_STEP_9KHZ_IDX && ((df.tune_old/4) > 1600001))
			idx --;
	}

	df.tuning_step	= tune_steps[idx];
	df.selected_idx = idx;

	// Update step on screen
	UiDriver_DisplayFreqStepSize();

}


/*----------------------------------------------------------------------------
 * @brief Scans buttons 0-16:  0-15 are normal buttons, 16 is power button, 17 touch
 * @param button_num - 0-17
 * @returns true if button is pressed
 */

static bool UiDriver_IsButtonPressed(ulong button_num)
{
	bool retval = false;

	// TODO: This is fragile code, as it depends on being called multiple times in short periods (ms)
	// This works, since regularily the button matrix is queried.
	UiLcdHy28_TouchscreenDetectPress();

	if(button_num < BUTTON_NUM)  				// buttons 0-15 are the normal keypad buttons
	{
		retval = HAL_GPIO_ReadPin(buttons.map[button_num].port,buttons.map[button_num].button) == 0;		// in normal mode - return key value
	}
	return retval;
}


void UiDriver_KeyboardProcessOldClicks()
{
	unsigned int i;

	static uchar press_hold_release_delay = 0;

	// State machine - processing old click
	if(ks.button_processed == false)
	{
		// State machine - click or release(debounce filter)
		if(!ks.button_pressed)
		{
			// Scan inputs - 16 buttons in total, but on different ports
			for(i = 0; i < 18; i++)           // button "17" is touchscreen
			{
				// Read each pin of the port, based on the declared pin map
				if(UiDriver_IsButtonPressed(i))
				{
					// Change state to clicked
					ks.button_id      = i;
					ks.button_pressed = 1;
					ks.button_released    = 0;
					ks.button_just_pressed    = 0;
					ks.debounce_time  = 0;
					ks.debounce_check_complete    = 0;
					ks.press_hold         = 0;
					//printf("button_pressed %02x\n\r",ks.button_id);
					// Exit, we process just one click at a time
					break;
				}
			}
		}
		else if((ks.debounce_time >= BUTTON_PRESS_DEBOUNCE) && (!ks.debounce_check_complete))
		{
			if(UiDriver_IsButtonPressed(ks.button_id))        // button still pressed?
			{
				ks.button_just_pressed = 1; // yes!
				ks.debounce_check_complete = 1; // indicate that the debounce check was completed
			}
			else
				ks.button_pressed = 0;          // debounce incomplete, button released - cancel detection
		}
		else if((ks.debounce_time >= BUTTON_HOLD_TIME) && (!ks.press_hold))     // press-and-hold processing
		{
			ks.button_processed = 1;                      // indicate that a button was processed
			ks.button_just_pressed = 0;                   // clear this flag so that the release (below) won't be detected
			ks.press_hold = 1;
			press_hold_release_delay = PRESS_HOLD_RELEASE_DELAY_TIME; // Set up a bit of delay for when press-and-hold is released
		}
		else if(ks.press_hold && (!UiDriver_IsButtonPressed(ks.button_id)))     // was there a press-and-hold and the button is now released?
		{
			if(press_hold_release_delay)                  // press-and-hold delay expired?
				press_hold_release_delay--;                 // no - continue counting down before cancelling "press-and-hold" mode
			else                              // Press-and-hold mode time expired!
			{
				ks.button_pressed = 0;          // reset and exit press-and-hold mode, this to prevent extraneous button-presses when using multiple buttons
				ks.button_released = 0;
				ks.press_hold = 0;
				ks.button_just_pressed = 0;
			}
		}
		else if(!UiDriver_IsButtonPressed(ks.button_id) && (!ks.press_hold))        // button released and had been debounced?
		{
			// Change state from click to released, and processing flag on - if the button had been held down adequately
			ks.button_pressed     = 0;
			ks.button_released    = 1;
			ks.button_processed   = 1;
			ks.button_just_pressed = 0;
			//printf("button_released %02x\n\r",ks.button_id);
		}
		//
		// Handle press-and-hold tuning step adjustment
		//
		if((ts.tune_step != 0) && (!ks.press_hold))     // are we in press-and-hold step size mode and did the button get released?
		{
			ts.tune_step = STEP_PRESS_OFF;                        // yes, cancel offset
			df.selected_idx = ts.tune_step_idx_holder;            // restore previous setting
			df.tuning_step    = tune_steps[df.selected_idx];
			UiDriver_DisplayFreqStepSize();
		}
	}
}



enum TRX_States_t
{
	TRX_STATE_TX_TO_RX,
	TRX_STATE_RX,
	TRX_STATE_RX_TO_TX,
	TRX_STATE_TX,
};

static void UiDriver_TxRxUiSwitch(enum TRX_States_t state)
{
	static uchar enc_one_mode =     ENC_ONE_MODE_AUDIO_GAIN;  // stores modes of encoder when we enter TX
	static uchar enc_two_mode =     ENC_TWO_MODE_RF_GAIN;    // stores modes of encoder when we enter TX
	static uchar enc_three_mode =   ENC_THREE_MODE_CW_SPEED;    // stores modes of encoder when we enter TX


	{
		if(state == TRX_STATE_RX_TO_TX)
		{

			UiDriver_DeleteSMeterLabels();
			UiDriver_DrawPowerMeterLabels();

			if((ts.flags1 & FLAGS1_TX_AUTOSWITCH_UI_DISABLE) == false)                // If auto-switch on TX/RX is enabled
			{
				// change display related to encoder one to TX mode (e.g. Sidetone gain or Compression level)
				enc_one_mode = ts.enc_one_mode;
				enc_two_mode = ts.enc_two_mode;
				enc_three_mode = ts.enc_thr_mode;

				// we reconfigure the encoders according to the currently selected mode
				// for now this is only relevant for CW
				if (ts.dmod_mode == DEMOD_CW)
				{
					ts.enc_one_mode = ENC_ONE_MODE_ST_GAIN;
					ts.enc_thr_mode = ENC_THREE_MODE_CW_SPEED;
				}
				else // for all other modes we activate the compressor setting and input gain control
				{
					ts.enc_one_mode = ENC_ONE_MODE_CMP_LEVEL;
					ts.enc_thr_mode = ENC_THREE_MODE_INPUT_CTRL;
				}
			}

			// force redisplay of Encoder boxes and values
			UiDriver_RefreshEncoderDisplay();
		}
		else if (state == TRX_STATE_TX_TO_RX)
		{

			UiDriver_DeleteSMeterLabels();
			UiDriver_DrawSMeterLabels();
			if((ts.flags1 & FLAGS1_TX_AUTOSWITCH_UI_DISABLE) == false)                // If auto-switch on TX/RX is enabled
			{
				ts.enc_one_mode = enc_one_mode;
				ts.enc_two_mode = enc_two_mode;
				ts.enc_thr_mode = enc_three_mode;
			}

			// force redisplay of Encoder boxes and values
			UiDriver_RefreshEncoderDisplay();
		}
	}

	UiDriver_UpdateBtmMeter(0,0);        // clear bottom meter of any outstanding indication when going back to RX
	if((ts.menu_mode))              // update menu when we are (or WERE) in MENU mode
	{
		UiMenu_RenderMenu(MENU_RENDER_ONLY);
	}
}

/**
 * adjust volume and return to RX from TX and other time-related functions,
 * has to be called regularly
 */
static void UiDriver_TimeScheduler()
{
	static bool	 audio_spkr_volume_update_request = true;
	static bool  audio_spkr_delayed_unmute_active = false;

	static bool old_squelch = 0;	// used to detect change-of-state of squelch
	static bool old_tone_det = 0;	// used to detect change-of-state of tone decoder
	static bool old_tone_det_enable = 0;	// used to detect change-of-state of tone decoder enabling
	static bool old_burst_active = 0;		// used to detect state of change of tone burst generator
	static bool startup_done_flag = 0;
	static bool	dsp_rx_reenable_flag = 0;
	static ulong dsp_rx_reenable_timer = 0;
	static uchar dsp_crash_count = 0;

	static enum TRX_States_t last_state = TRX_STATE_RX; // we assume everything is
	enum TRX_States_t state;



	// let us figure out if we are in a stable state or if this
	// is the first run after a mode change
	if (ts.txrx_mode == TRX_MODE_TX)
	{
		if (last_state != TRX_STATE_TX)
		{
			state = TRX_STATE_RX_TO_TX;
		}
		else
		{
			state = TRX_STATE_TX;
		}
		last_state = TRX_STATE_TX;
	}
	else
	{
		if (last_state != TRX_STATE_RX)
		{
			state = TRX_STATE_TX_TO_RX;
		}
		else
		{
			state = TRX_STATE_RX;
		}
		last_state = TRX_STATE_RX;
	}


	/*** RX MODE ***/
	if(ts.txrx_mode == TRX_MODE_RX)
	{
		if (state == TRX_STATE_TX_TO_RX)
		{
			audio_spkr_delayed_unmute_active = true;
		}

		if(audio_spkr_delayed_unmute_active  && ts.audio_spkr_unmute_delay_count == 0)	 	// did timer hit zero
		{
			audio_spkr_delayed_unmute_active = false;
			audio_spkr_volume_update_request = true;
		}



		audio_spkr_volume_update_request |= ts.rx_gain[RX_AUDIO_SPKR].value != ts.rx_gain[RX_AUDIO_SPKR].value_old;


		if( audio_spkr_volume_update_request)	 	// in normal mode - calculate volume normally
		{

			ts.rx_gain[RX_AUDIO_SPKR].value_old = ts.rx_gain[RX_AUDIO_SPKR].value;
			ts.rx_gain[RX_AUDIO_SPKR].active_value = 1;		// software gain not active - set to unity
			if(ts.rx_gain[RX_AUDIO_SPKR].value <= CODEC_SPEAKER_MAX_VOLUME)  				// Note:  Gain > 16 adjusted in audio_driver.c via software
			{
				Codec_VolumeSpkr((ts.rx_gain[RX_AUDIO_SPKR].value));

			}
			else  	// are we in the "software amplification" range?
			{
				Codec_VolumeSpkr(CODEC_SPEAKER_MAX_VOLUME);		// set to fixed "maximum" gain
				ts.rx_gain[RX_AUDIO_SPKR].active_value = (((float32_t)ts.rx_gain[RX_AUDIO_SPKR].value)/2.5)-5.35;	// to float
			}

			audio_spkr_volume_update_request = false;

			dsp_rx_reenable_flag = true;		// indicate that we need to re-enable the DSP soon
			dsp_rx_reenable_timer = ts.sysclock + DSP_REENABLE_DELAY;	// establish time at which we re-enable the DSP
		}

		// Check to see if we need to re-enable DSP after return to RX
		if(dsp_rx_reenable_flag)	 	// have we returned to RX after TX?
		{
			if(ts.sysclock > dsp_rx_reenable_timer)	 	// yes - is it time to re-enable DSP?
			{
				ts.dsp_inhibit = false;		// yes - re-enable DSP
				dsp_rx_reenable_flag = false;	// clear flag so we don't do this again
			}
		}

		// update the on-screen indicator of squelch/tone detection (the "FM" mode text) if there is a change of state of squelch/tone detection
		if((old_squelch != ads.fm_squelched)
				|| (old_tone_det != ads.fm_subaudible_tone_detected)
				|| (old_tone_det_enable != (bool)ts.fm_subaudible_tone_det_select))       // did the squelch or tone detect state just change?
		{

			UiDriver_DisplayDemodMode();                           // yes - update on-screen indicator to show that squelch is open/closed
			old_squelch = ads.fm_squelched;
			old_tone_det = ads.fm_subaudible_tone_detected;
			old_tone_det_enable = (bool)ts.fm_subaudible_tone_det_select;
		}

		// DSP crash detection
		if(is_dsp_nr() && !is_dsp_nr_postagc() && !ads.af_disabled && !ts.dsp_inhibit)    // Do this if enabled and "Pre-AGC" DSP NR enabled
		{

			if((ads.dsp_nr_sample > DSP_HIGH_LEVEL) || (ads.dsp_nr_sample == -1))       // is the DSP output very high, or wrapped around to -1?
			{
				dsp_crash_count+=2;           // yes - increase detect count quickly
			}
			else                            // not high level
			{
				if(dsp_crash_count)           // decrease detect count more slowly
				{
					dsp_crash_count--;
				}
			}

			if((ads.dsp_zero_count > DSP_ZERO_COUNT_ERROR) || (dsp_crash_count > DSP_CRASH_COUNT_THRESHOLD))        // is "zero" output count OR high level count exceeding threshold?
			{
				AudioDriver_SetRxAudioProcessing(ts.dmod_mode, true);   // update DSP settings
				dsp_crash_count = 0;              // clear crash count flag
			}
		}
	}

	/*** TX MODE ONLY ***/
	if(ts.txrx_mode == TRX_MODE_TX)
	{

		if((state == TRX_STATE_RX_TO_TX))
		{
			// we just switched to TX
			if((ts.dmod_mode != DEMOD_CW))        // did we just enter TX mode in voice mode?
			{
				ads.alc_val = 1;    // re-init AGC value
				ads.peak_audio = 0; // clear peak reading of audio meter
			}
		}


		// Has the timing for the tone burst expired?
		if(ts.sysclock > ts.fm_tone_burst_timing)
		{
			ads.fm_tone_burst_active = 0;               // yes, turn the tone off
		}

		if(ads.fm_tone_burst_active != old_burst_active)       // did the squelch or tone detect state just change?
		{
			UiDriver_DisplayDemodMode();                           // yes - update on-screen indicator to show that tone burst is on/off
			old_burst_active = ads.fm_tone_burst_active;
		}
	}

	/*** TX+RX STATE CHANGE ONLY ***/
	// if we do change modes, some visuals need an update
	if(state == TRX_STATE_RX_TO_TX || state == TRX_STATE_TX_TO_RX)
	{
		// now update display according to the changed state
		UiDriver_TxRxUiSwitch(state);
	}

	/*** ALWAYS ***/
	UiDriver_LcdBlankingProcessTimer();


	// This delays the start-up of the DSP for several seconds to minimize the likelihood that the LMS function will get "jammed"
	// and stop working.

	/*** ONCE AFTER STARTUP DELAY ***/
	if( startup_done_flag == false && (ts.sysclock > DSP_STARTUP_DELAY))       // has it been long enough after startup?
	{
		startup_done_flag = true;                  // set flag so that we do this only once

		// TODO: Get this away by fixing the startup of the noise blanker
		if(ts.temp_nb < 0x80)       // load NB setting after processing first audio data
		{
			ts.nb_setting = ts.temp_nb;
			UiDriver_DisplayEncoderTwoMode();
			ts.temp_nb = 0xf0;
		}
		ts.dsp_inhibit = 0;                 // allow DSP to function



		audio_spkr_volume_update_request = 1;      // set unmute flag to force audio to be un-muted - just in case it starts up muted!
		Codec_MuteDAC(false);                      // make sure that audio is un-muted
	}
}


/*
 * Tells you which SSB Demod Mode is the preferred one for a given frequency in Hertz
 */
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeDemodMode
//* Object              : change demodulator mode
//* Input Parameters    : "noskip", if TRUE, disabled modes are to be included
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

/**
 * This function is responsible for make the changes to the UI layout
 * as required for a give new mode, such as enabling the right set of encoder boxes etc.
 */
typedef struct
{
	int16_t encoder_modes[3];
} encoder_mode_store_t;

void UiDriver_SetDemodMode(uint8_t new_mode)
{
	RadioManagement_SetDemodMode(new_mode);
#if 0
	static encoder_mode_store_t demod_modes[] =
	{
			{ ENC_ONE_MODE_AUDIO_GAIN, ENC_TWO_MODE_RF_GAIN, ENC_THREE_MODE_RIT }, // USB, LSB,(S)AM,FM,FreeDV
			{ ENC_ONE_MODE_ST_GAIN, -1, ENC_THREE_MODE_CW_SPEED }, // CW
			{ ENC_ONE_MODE_RTTY_SPEED, ENC_TWO_MODE_RTTY_SHIFT, -1 }, // RTTY
	};
#endif

	switch(ts.dmod_mode)
	{
	case DEMOD_DIGI:
	{
		switch(ts.digital_mode)
		{
		case DigitalMode_RTTY:
			if (ts.enc_one_mode != ENC_ONE_MODE_AUDIO_GAIN)
			{
				ts.enc_one_mode = ENC_ONE_MODE_RTTY_SPEED;
			}
			if (ts.enc_two_mode != ENC_TWO_MODE_RF_GAIN)
			{
				ts.enc_two_mode = ENC_TWO_MODE_RTTY_SHIFT;
			}
		}
	}
	break;

	case DEMOD_CW:
	{
		if (ts.enc_one_mode != ENC_ONE_MODE_AUDIO_GAIN)
		{
			ts.enc_one_mode = ENC_ONE_MODE_ST_GAIN;
		}
		if (ts.enc_thr_mode != ENC_THREE_MODE_RIT)
		{
			ts.enc_thr_mode = ENC_THREE_MODE_CW_SPEED;
		}
	}
	break;
	default:
		break;
	}
	UiDriver_UpdateDisplayAfterParamChange();
}

static void UiDriver_ChangeToNextDemodMode(bool select_alternative_mode)
{
	uint8_t new_mode = ts.dmod_mode;	// copy to local, so IRQ is not affected
	if (select_alternative_mode)
	{
		new_mode = RadioManagement_NextAlternativeDemodMode(new_mode);
	}
	else
	{
		new_mode = RadioManagement_NextNormalDemodMode(new_mode);
	}

	// TODO: We call this always, since we may have switched sidebands or the digital mode
	// if we would remember that, we would decide if to call this.
	UiDriver_SetDemodMode(new_mode);
}

/**
 * @brief initiate band change.
 * @param is_up select the next higher band, otherwise go to the next lower band
 */
static void UiDriver_ChangeBand(uchar is_up)
{

	// Do not allow band change during TX
	if(ts.txrx_mode != TRX_MODE_TX)
	{
		ulong   curr_band_index;    // index in band table of currently selected band
		ulong   new_band_index;     // index of the new selected band


		uint16_t vfo_sel = is_vfo_b()?VFO_B:VFO_A;

		curr_band_index = ts.band;

		// Save old band values
		if(curr_band_index < (MAX_BANDS) && ts.cat_band_index == 255)
		{
			// Save dial
			vfo[vfo_sel].band[curr_band_index].dial_value = df.tune_old;
			vfo[vfo_sel].band[curr_band_index].decod_mode = ts.dmod_mode;
		}
		else
		{
			ts.cat_band_index = 255;
		}

		// Handle direction
		if(is_up)
		{
			if(curr_band_index < (MAX_BANDS - 1))
			{
				// Increase
				new_band_index = curr_band_index + 1;
				if(ts.rfmod_present == 0 && ts.vhfuhfmod_present == 0 && curr_band_index == 8)
				{
					// jump 10m --> 160m
					new_band_index = MAX_BANDS-1;
				}
				if(ts.rfmod_present == 0 && ts.vhfuhfmod_present == 1 && curr_band_index == 8)
				{
					// jump 10m --> 2m
					new_band_index = 11;
				}
				if(ts.rfmod_present == 0 && ts.vhfuhfmod_present == 1 && curr_band_index == 13)
				{
					// jump 2200m --> 16m
					new_band_index = 16;
				}
				if(ts.rfmod_present == 1 && ts.vhfuhfmod_present == 0 && curr_band_index == 10)
				{
					// jump 4m --> 2200m
					new_band_index = 14;
				}

			}
			else	 	// wrap around to the lowest band
			{
				new_band_index = MIN_BANDS;
			}
		}
		else
		{
			if(curr_band_index)
			{
				// Decrease
				new_band_index = curr_band_index - 1;
				if(ts.rfmod_present == 0 && curr_band_index == MAX_BANDS-1)
				{
					// jump 160m --> 23cm
					new_band_index = 13;
				}
				if(ts.vhfuhfmod_present == 0 && new_band_index == 13)
				{
					// jump 2200m --> 6m
					new_band_index = 10;
				}
				if(ts.rfmod_present == 0 && new_band_index == 10)
				{
					// jump 2m --> 10m
					new_band_index = 8;
				}
			}
			else
			{
				// wrap around to the highest band
				new_band_index = MAX_BANDS-1;
			}
		}


		// TODO: There is a strong similarity to code in UiDriverProcessFunctionKeyClick around line 2053
		// Load frequency value - either from memory or default for
		// the band if this is first band selection
		if(vfo[vfo_sel].band[new_band_index].dial_value != 0xFFFFFFFF)
		{
			df.tune_new = vfo[vfo_sel].band[new_band_index].dial_value;	// Load value from VFO
		}
		else
		{
			df.tune_new = bandInfo[curr_band_index].tune; 					// Load new frequency from startup
		}

		bool new_lsb = RadioManagement_CalculateCWSidebandMode();

		uint16_t new_dmod_mode = vfo[vfo_sel].band[new_band_index].decod_mode;

		// we need to mute here since changing bands may cause audible click/pops
		RadioManagement_MuteTemporarilyRxAudio();


		if(ts.dmod_mode != new_dmod_mode || (new_dmod_mode == DEMOD_CW && ts.cw_lsb != new_lsb))
		{
			// Update mode
			ts.cw_lsb = new_lsb;
			RadioManagement_SetDemodMode(new_dmod_mode);
		}

		// Finally update public flag
		ts.band = new_band_index;

		UiDriver_UpdateDisplayAfterParamChange();    // because mode/filter may have changed
	}
}

/**
 * @brief Read out the changes in the frequency encoder and initiate frequency change by setting a global variable.
 *
 * @returns true if a frequency change was detected and a new tuning frequency was set in a global variable.
 */
static bool UiDriver_CheckFrequencyEncoder()
{
	int 		pot_diff;
	bool		retval = false;
	int		enc_multiplier;
	static float 	enc_speed_avg = 0.0;  //keeps the averaged encoder speed
	int		delta_t, enc_speed;

	pot_diff = UiDriverEncoderRead(ENCFREQ);


	if (pot_diff != 0)
	{
		delta_t = ts.audio_int_counter;  // get ticker difference since last enc. change
		ts.audio_int_counter = 0;		 //reset tick counter

		UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing

	}
	if (pot_diff != 0 &&
			ts.txrx_mode == TRX_MODE_RX
			&& ks.button_just_pressed == false
			&& ts.frequency_lock == false)
	{
		// allow tuning only if in rx mode, no freq lock,
		if (delta_t > 300)
		{
			enc_speed_avg = 0;    //when leaving speedy turning set avg_speed to 0
		}

		enc_speed = div(4000,delta_t).quot*pot_diff;  // app. 4000 tics per second -> calc. enc. speed.

		if (enc_speed > 500)
		{
			enc_speed = 500;    //limit calculated enc. speed
		}
		if (enc_speed < -500)
		{
			enc_speed = -500;
		}

		enc_speed_avg = 0.1*enc_speed + 0.9*enc_speed_avg; // averaging to smooth encoder speed

		enc_multiplier = 1; //set standard speed

		if (ts.flags1 & FLAGS1_DYN_TUNE_ENABLE)   // check if dynamic tuning has been activated by touchscreen
		{
			if ((enc_speed_avg > 80) || (enc_speed_avg < (-80)))
			{
				enc_multiplier = 10;    // turning medium speed -> increase speed by 10
			}

			if ((enc_speed_avg > 160) || (enc_speed_avg < (-160)))
			{
				enc_multiplier = 40;    //turning fast speed -> increase speed by 100
			}

			if ((enc_speed_avg > 300) || (enc_speed_avg < (-300)))
			{
				enc_multiplier = 100;    //turning fast speed -> increase speed by 100
			}

			if ((df.tuning_step == 10000) && (enc_multiplier > 10))
			{
				enc_multiplier = 10;    //limit speed to 100000kHz/step
			}
			if ((df.tuning_step == 100000) && (enc_multiplier > 1))
			{
				enc_multiplier = 1;    //limit speed to 100000kHz/step
			}
		}


		// Finally convert to frequency incr/decr

		if(pot_diff>0)
		{
			df.tune_new += (df.tuning_step * TUNE_MULT * enc_multiplier);
			//itoa(enc_speed,num,6);
			//UiSpectrumClearDisplay();			// clear display under spectrum scope
			//UiLcdHy28_PrintText(110,156,num,Cyan,Black,0);
		}
		else
		{
			df.tune_new -= (df.tuning_step * TUNE_MULT * enc_multiplier);
		}

		if (enc_multiplier != 1)
		{
			df.tune_new = TUNE_MULT*enc_multiplier*df.tuning_step * div((df.tune_new/TUNE_MULT),enc_multiplier*df.tuning_step).quot;    // keep last digit to zero
		}

		retval = true;
	}
	return retval;
}



//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckEncoderOne
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_CheckEncoderOne()
{
	int32_t pot_diff = UiDriverEncoderRead(ENC1);

	if (pot_diff)
	{
		int8_t pot_diff_step = pot_diff < 0?-1:1;

		UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing
		// Take appropriate action
		switch(ts.enc_one_mode)
		{
		case ENC_ONE_MODE_RTTY_SPEED:
			// Convert to Audio Gain incr/decr
			rtty_ctrl_config.speed_idx = change_and_limit_int(rtty_ctrl_config.speed_idx,pot_diff_step,0,RTTY_SPEED_NUM-1);
			RttyDecoder_Init();
			UiDriver_DisplayRttySpeed(true);
			break;
			// Update audio volume
		case ENC_ONE_MODE_AUDIO_GAIN:
			ts.rx_gain[RX_AUDIO_SPKR].value = change_and_limit_uint(ts.rx_gain[RX_AUDIO_SPKR].value,pot_diff_step,0,ts.rx_gain[RX_AUDIO_SPKR].max);
			UiDriver_DisplayAfGain(1);
			break;
		case ENC_ONE_MODE_ST_GAIN:
			ts.cw_sidetone_gain = change_and_limit_uint(ts.cw_sidetone_gain,pot_diff_step,0,SIDETONE_MAX_GAIN);
			Codec_TxSidetoneSetgain(ts.txrx_mode);
			UiDriver_DisplaySidetoneGain(true);
			break;
		case ENC_ONE_MODE_CMP_LEVEL:
			ts.tx_comp_level = change_and_limit_int(ts.tx_comp_level,pot_diff_step,TX_AUDIO_COMPRESSION_MIN,TX_AUDIO_COMPRESSION_MAX);
			AudioManagement_CalcTxCompLevel();		// calculate values for selection compression level
			UiDriver_DisplayCmpLevel(true);	// update on-screen display
			break;
		default:
			break;
		}
	}
}
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckEncoderTwo
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_CheckEncoderTwo()
{
	int32_t pot_diff = UiDriverEncoderRead(ENC2);

	if (pot_diff != 0)
	{
		UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing

		if(ts.menu_mode)
		{
			UiMenu_RenderChangeItem(pot_diff);
		}
		else
		{
			int8_t pot_diff_step = pot_diff < 0?-1:1;


			if(ts.txrx_mode == TRX_MODE_RX)
			{

				// dynamic encoder speed , used for notch and peak
				static float    enc_speed_avg = 0.0;  //keeps the averaged encoder speed
				int     delta_t, enc_speed;
				float32_t   enc_multiplier;

				delta_t = ts.audio_int_counter;  // get ticker difference since last enc. change
				ts.audio_int_counter = 0;        //reset tick counter

				if (delta_t > 300)
				{
					enc_speed_avg = 0;    //when leaving speedy turning set avg_speed to 0
				}

				enc_speed = div(4000,delta_t).quot*pot_diff;  // app. 4000 tics per second -> calc. enc. speed.

				if (enc_speed > 500)
				{
					enc_speed = 500;    //limit calculated enc. speed
				}
				if (enc_speed < -500)
				{
					enc_speed = -500;
				}

				enc_speed_avg = 0.1*enc_speed + 0.9*enc_speed_avg; // averaging to smooth encoder speed

				enc_multiplier = 1; //set standard speed

				if ((enc_speed_avg > 80) || (enc_speed_avg < (-80)))
				{
					enc_multiplier = 10;    // turning medium speed -> increase speed by 10
				}
				if ((enc_speed_avg > 150) || (enc_speed_avg < (-150)))
				{
					enc_multiplier = 30;    //turning fast speed -> increase speed by 100
				}
				if ((enc_speed_avg > 300) || (enc_speed_avg < (-300)))
				{
					enc_multiplier = 100;    //turning fast speed -> increase speed by 100
				}


				// used for notch and peak
				float32_t MAX_FREQ = 5000.0;

				if (FilterPathInfo[ts.filter_path].sample_rate_dec == RX_DECIMATION_RATE_24KHZ)
				{
					MAX_FREQ = 10000.0;
				}
				else if (FilterPathInfo[ts.filter_path].sample_rate_dec == RX_DECIMATION_RATE_12KHZ)
				{
					MAX_FREQ = 5000.0;
				}



				switch(ts.enc_two_mode)
				{
				case ENC_TWO_MODE_RTTY_SHIFT:
					rtty_ctrl_config.shift_idx = change_and_limit_int(rtty_ctrl_config.shift_idx,pot_diff_step,0,RTTY_SHIFT_NUM-1);
					RttyDecoder_Init();
					UiDriver_DisplayRttyShift(1);
					break;
				case ENC_TWO_MODE_RF_GAIN:
					if(ts.dmod_mode != DEMOD_FM)	 	// is this *NOT* FM?  Change RF gain
					{
						if(!ts.agc_wdsp)
						{
							// Convert to Audio Gain incr/decr
							ts.rf_gain = change_and_limit_int(ts.rf_gain,pot_diff_step,0,MAX_RF_GAIN);
							AudioManagement_CalcRFGain();		// convert from user RF gain value to "working" RF gain value
						}
						else
						{
							ts.agc_wdsp_thresh = change_and_limit_int(ts.agc_wdsp_thresh,pot_diff_step,-20,120);
							AudioDriver_SetupAgcWdsp();
						}
					}
					else	 		// it is FM - change squelch setting
					{
						ts.fm_sql_threshold = change_and_limit_uint(ts.fm_sql_threshold,pot_diff_step,0,FM_SQUELCH_MAX);
					}

					UiDriver_DisplayRfGain(1);    // change on screen
					break;

					// Update DSP/NB setting
				case ENC_TWO_MODE_SIG_PROC:
					if(ts.agc_wdsp == 0)
					{
						if(is_dsp_nb())	 	// is it in noise blanker mode?
						{
							ts.nb_setting = (uchar)change_and_limit_uint(ts.nb_setting,pot_diff_step,0,MAX_NB_SETTING);
						}
						// Signal processor setting
					}
					else
					{
						//                    ts.agc_wdsp_tau_decay = change_and_limit_int(ts.agc_wdsp_tau_decay,pot_diff_step * 100,100,5000);
						ts.agc_wdsp_mode = change_and_limit_uint(ts.agc_wdsp_mode,pot_diff_step,0,5);
						ts.agc_wdsp_switch_mode = 1; // set flag, so that mode switching really takes place in AGC_prep
						AudioDriver_SetupAgcWdsp();
					}
					UiDriver_DisplayNoiseBlanker(1);
					break;
				case ENC_TWO_MODE_NR:
					if (is_dsp_nr())        // only allow adjustment if DSP NR is active
					{
						ts.dsp_nr_strength = change_and_limit_uint(ts.dsp_nr_strength,pot_diff_step,0,DSP_NR_STRENGTH_MAX);
						AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
					}
					// Signal processor setting
					UiDriver_DisplayDSPMode(1);
					break;
				case ENC_TWO_MODE_NOTCH_F:
					if (is_dsp_mnotch())   // notch f is only adjustable when notch is enabled
					{
						if(pot_diff < 0)
						{
							ts.notch_frequency = ts.notch_frequency - 5.0 * enc_multiplier;
						}
						if(pot_diff > 0)
						{
							ts.notch_frequency = ts.notch_frequency + 5.0 * enc_multiplier;
						}

						if(ts.notch_frequency > MAX_FREQ)
						{
							ts.notch_frequency = MAX_FREQ;
						}
						if(ts.notch_frequency < MIN_PEAK_NOTCH_FREQ)
						{
							ts.notch_frequency = MIN_PEAK_NOTCH_FREQ;
						}
						// display notch frequency
						// set notch filter instance
						AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
						UiDriver_DisplayDSPMode(1);
					}
					break;
				case ENC_TWO_MODE_BASS_GAIN:
					ts.bass_gain = change_and_limit_int(ts.bass_gain,pot_diff_step,MIN_BASS,MAX_BASS);
					// set filter instance
					AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
					// display bass gain
					UiDriver_DisplayTone(true);
					break;
				case ENC_TWO_MODE_TREBLE_GAIN:
					ts.treble_gain = change_and_limit_int(ts.treble_gain,pot_diff_step,MIN_TREBLE,MAX_TREBLE);
					// set filter instance
					AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
					// display treble gain
					UiDriver_DisplayTone(true);
					break;

				case ENC_TWO_MODE_PEAK_F:
					if (is_dsp_mpeak())   // peak f is only adjustable when peak is enabled
					{
						if(pot_diff < 0)
						{
							ts.peak_frequency = ts.peak_frequency - 5.0 * enc_multiplier;
						}
						if(pot_diff > 0)
						{
							ts.peak_frequency = ts.peak_frequency + 5.0 * enc_multiplier;
						}
						if(ts.peak_frequency > MAX_FREQ)
						{
							ts.peak_frequency = MAX_FREQ;
						}
						if(ts.peak_frequency < MIN_PEAK_NOTCH_FREQ)
						{
							ts.peak_frequency = MIN_PEAK_NOTCH_FREQ;
						}
						// set notch filter instance
						AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
						// display peak frequency
						UiDriver_DisplayDSPMode(1);
					}
					break;
				default:
					break;
				}
			}
			else { // in TX case only bass & treble gain can be adjusted with encoder TWO

				// Take appropriate action
				switch(ts.enc_two_mode)
				{
				case ENC_TWO_MODE_BASS_GAIN:
					ts.tx_bass_gain = change_and_limit_int(ts.tx_bass_gain,pot_diff_step,MIN_TX_BASS,MAX_TX_BASS);
					// set filter instance
					AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
					// display bass gain
					UiDriver_DisplayTone(true);
					break;
				case ENC_TWO_MODE_TREBLE_GAIN:
					ts.tx_treble_gain = change_and_limit_int(ts.tx_treble_gain,pot_diff_step,MIN_TX_TREBLE,MAX_TX_TREBLE);
					// set filter instance
					AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
					// display treble gain
					UiDriver_DisplayTone(true);
					break;
				default:
					break;
				}
			}
		}
	}
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckEncoderThree
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_CheckEncoderThree()
{
	int 	pot_diff;

	pot_diff = UiDriverEncoderRead(ENC3);


	if (pot_diff)
	{
		int8_t pot_diff_step = pot_diff < 0?-1:1;

		UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing
		if (filter_path_change)
		{
			AudioFilter_NextApplicableFilterPath(PATH_ALL_APPLICABLE | (pot_diff < 0?PATH_DOWN:PATH_UP),AudioFilter_GetFilterModeFromDemodMode(ts.dmod_mode),ts.filter_path);
			// we store the new filter in the current active filter location
			AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
			// we activate it (in fact the last used one, which is the newly selected one);

			UiDriver_UpdateDisplayAfterParamChange();
		}
		else  if(ts.menu_mode)
		{
			UiMenu_RenderChangeItemValue(pot_diff);
		}
		else
		{
			// Take appropriate action
			switch(ts.enc_thr_mode)
			{
			// Update RIT value
			case ENC_THREE_MODE_RIT:
				if(ts.txrx_mode == TRX_MODE_RX)
				{
					int16_t old_rit_value = ts.rit_value;
					ts.rit_value = change_and_limit_int(ts.rit_value,pot_diff_step,MIN_RIT_VALUE,MAX_RIT_VALUE);

					ts.dial_moved = ts.rit_value != old_rit_value;

					// Update RIT
					UiDriver_DisplayRit(1);
					// Change frequency
					UiDriver_FrequencyUpdateLOandDisplay(false);
				}
				break;
				// Keyer speed
			case ENC_THREE_MODE_CW_SPEED:
				// Convert to Audio Gain incr/decr
				ts.cw_keyer_speed = change_and_limit_int(ts.cw_keyer_speed,pot_diff_step,CW_KEYER_SPEED_MIN,CW_KEYER_SPEED_MAX);
				CwGen_SetSpeed();
				UiDriver_DisplayKeyerSpeed(1);
				break;
			case ENC_THREE_MODE_INPUT_CTRL:
				// in voice mode, adjust audio input gain
			{
				uint16_t gain_max = ts.tx_audio_source == TX_AUDIO_MIC?MIC_GAIN_MAX:LINE_GAIN_MAX;
				uint16_t gain_min = ts.tx_audio_source == TX_AUDIO_MIC?MIC_GAIN_MIN:LINE_GAIN_MIN;

				ts.tx_gain[ts.tx_audio_source] = change_and_limit_int(ts.tx_gain[ts.tx_audio_source],pot_diff_step,gain_min,gain_max);

				if (ts.tx_audio_source == TX_AUDIO_MIC)
				{
					Codec_SwitchMicTxRxMode(ts.txrx_mode);
				}
				UiDriver_DisplayLineInModeAndGain(1);
			}
			break;
			default:
				break;
			}
		}
	}
}

static bool UiDriver_IsApplicableEncoderOneMode(uint8_t mode)
{
	bool retval = true;
	switch(mode)
	{
	case ENC_ONE_MODE_RTTY_SPEED:
		// only switch to rtty adjustment, if rtty enabled!
		retval = is_demod_rtty();
		break;
	case ENC_ONE_MODE_ST_GAIN:
		retval = ts.dmod_mode == DEMOD_CW;
		break;
	case ENC_ONE_MODE_CMP_LEVEL:
		retval = ts.dmod_mode != DEMOD_CW && ts.dmod_mode != DEMOD_DIGI;
		break;
	}
	return retval;
}


static void UiDriver_DisplayEncoderOneMode()
{
	// upper box
	UiDriver_DisplayAfGain(ts.enc_one_mode == ENC_ONE_MODE_AUDIO_GAIN);

	// lower box
	switch(ts.enc_one_mode)
	{
	case ENC_ONE_MODE_RTTY_SPEED:
		UiDriver_DisplayRttySpeed(1);
		break;
	case ENC_ONE_MODE_ST_GAIN:
		UiDriver_DisplaySidetoneGain(1);
		break;
	case ENC_ONE_MODE_CMP_LEVEL:
		UiDriver_DisplayCmpLevel(1);
		break;
	default:
		// what to display if lower box is not active
		if (is_demod_rtty())
		{
			UiDriver_DisplayRttySpeed(0);
		}
		else if(ts.dmod_mode == DEMOD_CW)
		{
			UiDriver_DisplaySidetoneGain(0);
		}
		else
		{
			UiDriver_DisplayCmpLevel(0);
		}
	}
}

static bool UiDriver_IsApplicableEncoderTwoMode(uint8_t mode)
{
	bool retval = true;
	switch(mode)
	{
	case ENC_TWO_MODE_RTTY_SHIFT:
		// only switch to rtty adjustment, if rtty enabled!
		retval = is_demod_rtty();
		break;
	case ENC_TWO_MODE_NOTCH_F:
		retval = is_dsp_mnotch();
		break;
	case ENC_TWO_MODE_PEAK_F:
		retval = is_dsp_mpeak();
		break;
	}
	return retval;
}


static void UiDriver_DisplayEncoderTwoMode()
{

	uint8_t inactive = ts.menu_mode?0:1;
	// we use this to disable all active displays once in menu mode
	switch(ts.enc_two_mode)
	{
	case ENC_TWO_MODE_RF_GAIN:
		UiDriver_DisplayRfGain(inactive);
		UiDriver_DisplayNoiseBlanker(0);
		UiDriver_DisplayDSPMode(0);
		break;
	case ENC_TWO_MODE_SIG_PROC:
		UiDriver_DisplayRfGain(0);
		UiDriver_DisplayNoiseBlanker(inactive);
		UiDriver_DisplayDSPMode(0);
		break;
	case ENC_TWO_MODE_PEAK_F:
	case ENC_TWO_MODE_NOTCH_F:
	case ENC_TWO_MODE_NR:
		UiDriver_DisplayRfGain(0);
		UiDriver_DisplayNoiseBlanker(0);
		UiDriver_DisplayDSPMode(inactive);
		break;
	case ENC_TWO_MODE_BASS_GAIN:
		UiDriver_DisplayDSPMode(0);
		UiDriver_DisplayTone(inactive);
		break;
	case ENC_TWO_MODE_TREBLE_GAIN:
		UiDriver_DisplayDSPMode(0);
		UiDriver_DisplayTone(inactive);
		break;
	case ENC_TWO_MODE_RTTY_SHIFT:
		UiDriver_DisplayRfGain(0);
		UiDriver_DisplayDSPMode(0);
		UiDriver_DisplayRttyShift(1);
		break;
	default:
		UiDriver_DisplayRfGain(0);
		UiDriver_DisplayNoiseBlanker(0);
		UiDriver_DisplayDSPMode(0);
		break;
	}

}


static bool UiDriver_IsApplicableEncoderThreeMode(uint8_t mode)
{
	bool retval = true;
	switch(mode)
	{
	case ENC_THREE_MODE_CW_SPEED:
		retval = ts.dmod_mode == DEMOD_CW;
		break;
	}
	return retval;
}


static void UiDriver_DisplayEncoderThreeMode()
{
	// upper box
	UiDriver_DisplayRit(ts.enc_thr_mode == ENC_THREE_MODE_RIT);

	// lower box
	switch(ts.enc_thr_mode)
	{
	case ENC_THREE_MODE_CW_SPEED:
		UiDriver_DisplayKeyerSpeed(1);
		break;
	case ENC_THREE_MODE_INPUT_CTRL:
		UiDriver_DisplayLineInModeAndGain(1);
		break;
	default:
		// this defines what is shown if the lower box is not actively selected
		if (ts.dmod_mode == DEMOD_CW)
		{
			UiDriver_DisplayKeyerSpeed(0);
		}
		else
		{
			UiDriver_DisplayLineInModeAndGain(0);
		}
		break;

	}
}


/**
 * Handles the execution of the change encoder logic for the 3 encoders
 */

static void UiDriver_ChangeEncoderMode(volatile uint8_t* mode_ptr, uint8_t num_modes, bool (*is_applicable_f)(uint8_t), void(*display_encoder_f)())
{
	if(ts.menu_mode == false)   // changes only when not in menu mode
	{
		uint8_t new_enc_mode = *mode_ptr;
		do
		{
			new_enc_mode++;
			new_enc_mode %= num_modes;
		} while ((*is_applicable_f)(new_enc_mode)  == false && new_enc_mode != *mode_ptr );
		if (new_enc_mode != *mode_ptr)
		{
			*mode_ptr = new_enc_mode;
			(*display_encoder_f)();
		}
	}
}

static void UiDriver_ChangeEncoderOneMode()
{
	UiDriver_ChangeEncoderMode(&ts.enc_one_mode, ENC_ONE_NUM_MODES, UiDriver_IsApplicableEncoderOneMode, UiDriver_DisplayEncoderOneMode);
}

static void UiDriver_ChangeEncoderTwoMode()
{
	UiDriver_ChangeEncoderMode(&ts.enc_two_mode, ENC_TWO_NUM_MODES, UiDriver_IsApplicableEncoderTwoMode, UiDriver_DisplayEncoderTwoMode);
}

static void UiDriver_ChangeEncoderThreeMode()
{
	UiDriver_ChangeEncoderMode(&ts.enc_thr_mode, ENC_THREE_NUM_MODES, UiDriver_IsApplicableEncoderThreeMode, UiDriver_DisplayEncoderThreeMode);
}

/**
 * @brief Displays audio speaker volume
 */
static void UiDriver_DisplayAfGain(bool encoder_active)
{
	UiDriver_EncoderDisplaySimple(0,0,"AFG", encoder_active, ts.rx_gain[RX_AUDIO_SPKR].value);
}

/**
 * @brief Display CW Sidetone gain (used during CW TX or training)
 */
static void UiDriver_DisplaySidetoneGain(bool encoder_active)
{
	UiDriver_EncoderDisplaySimple(1,0,"STG", encoder_active, ts.cw_sidetone_gain);
}

/**
 * @brief Display TX Compressor Level
 */
static void UiDriver_DisplayCmpLevel(bool encoder_active)
{
	ushort 	color = encoder_active?White:Grey;
	char	temp[5];
	const char* outs;

	if (ts.tx_comp_level == TX_AUDIO_COMPRESSION_MIN)
	{
		outs ="OFF";
	}
	else if(ts.tx_comp_level < TX_AUDIO_COMPRESSION_MAX)	 	// 	display numbers for all but the highest value
	{
		snprintf(temp,5," %02d",ts.tx_comp_level);
		outs = temp;
	}
	else
	{
		color = Yellow; // Custom value - use yellow
		outs ="CUS";
	}

	UiDriver_EncoderDisplay(1,0,"CMP" , encoder_active, outs, color);
}

uint32_t dsp_nr_color_map()
{
	uint32_t color = White;      // Make it white by default
	//
	if(ts.dsp_nr_strength >= DSP_STRENGTH_RED)
		color = Red;
	else if(ts.dsp_nr_strength >= DSP_STRENGTH_ORANGE)
		color = Orange;
	else if(ts.dsp_nr_strength >= DSP_STRENGTH_YELLOW)
		color = Yellow;

	return color;
}

static void UiDriver_DisplayDSPMode(bool encoder_active)
{
	uint32_t clr = White;
	uint32_t clr_val = White;

	char val_txt[7] = { 0x0 };
	bool txt_is_value = false;
	const char* txt[2] = { "DSP", NULL };

	uint32_t dsp_functions_active = ts.dsp_active & (DSP_NOTCH_ENABLE|DSP_NR_ENABLE|DSP_MNOTCH_ENABLE|DSP_MPEAK_ENABLE);


	switch (dsp_functions_active)
	{
	case 0: // all off
		clr = Grey2;
		txt[1] = "OFF";
		break;
	case DSP_NR_ENABLE:
		txt[0] = "NR";
		snprintf(val_txt,7,"%5u", ts.dsp_nr_strength);
		clr_val = dsp_nr_color_map();
		txt[1] = val_txt;
		txt_is_value = true;
		break;
	case DSP_NOTCH_ENABLE:
		txt[1] = "A-NOTCH";
		break;
	case DSP_NOTCH_ENABLE|DSP_NR_ENABLE:
	txt[0] = "NR+NOTC";
	snprintf(val_txt,7,"%5u", ts.dsp_nr_strength);
	clr_val = dsp_nr_color_map();
	txt[1] = val_txt;
	txt_is_value = true;
	break;
	case DSP_MNOTCH_ENABLE:
		txt[0] = "M-NOTCH";
		snprintf(val_txt,7,"%5lu", ts.notch_frequency);
		txt[1] = val_txt;
		txt_is_value = true;
		break;
	case DSP_MPEAK_ENABLE:
		txt[0] = "PEAK";
		snprintf(val_txt,7,"%5lu", ts.peak_frequency);
		txt[1] = val_txt;
		txt_is_value = true;
		break;
	default: // unsupported combination of DSP functions yield error display now
		clr = Grey2;
		txt[1] = "ERROR";
		break;
	}

	UiDriver_LeftBoxDisplay(0,txt[0],encoder_active,txt[1],clr,clr_val,txt_is_value);
}


static void UiDriver_DisplayKeyerSpeed(bool encoder_active)
{
	uint16_t 	color = encoder_active?White:Grey;
	const char* txt;
	char  txt_buf[5];

	txt = "WPM";
	snprintf(txt_buf,5,"%3d",ts.cw_keyer_speed);

	UiDriver_EncoderDisplay(1,2,txt, encoder_active, txt_buf, color);
}

static void UiDriver_DisplayRttySpeed(bool encoder_active)
{
	uint16_t  color = encoder_active?White:Grey;
	UiDriver_EncoderDisplay(1,0,"BD", encoder_active, rtty_speeds[rtty_ctrl_config.speed_idx].label, color);
}

static void UiDriver_DisplayRttyShift(bool encoder_active)
{
	uint16_t  color = encoder_active?White:Grey;
	UiDriver_EncoderDisplay(1,1,"SFT", encoder_active, rtty_shifts[rtty_ctrl_config.shift_idx].label, color);
}


static void UiDriver_DisplayLineInModeAndGain(bool encoder_active)
{
	ushort 	color = encoder_active?White:Grey;
	const char* txt;
	char  txt_buf[5];

	bool gain_external_control = false;
	// if true, gain is controlled externally and ENC3 encoder does not do anything.

	switch (ts.tx_audio_source)
	{
	case TX_AUDIO_MIC:
		txt = "MIC";
		break;
	case TX_AUDIO_LINEIN_L:										// Line gain
		txt = "L>L";
		break;
	case TX_AUDIO_LINEIN_R:										// Line gain
		txt = "L>R";
		break;
	case TX_AUDIO_DIG:										// Line gain
		txt = "DIG";
		gain_external_control = true;
		break;
	case TX_AUDIO_DIGIQ:
		txt = "DIQ";
		gain_external_control = true;
		break;
	default:
		txt = "???";
	}

	if (gain_external_control == true)
	{
		snprintf(txt_buf,5,"EXT");
	}
	else
	{
		snprintf(txt_buf,5,"%3d",ts.tx_gain[ts.tx_audio_source]);
	}

	UiDriver_EncoderDisplay(1,2,txt, encoder_active, txt_buf, color);
}

static void UiDriver_DisplayRfGain(bool encoder_active)
{
	uint32_t color = encoder_active?White:Grey;

	char	temp[5];
	const char* label = "???";
	int32_t value;
	if(ts.agc_wdsp && ts.dmod_mode != DEMOD_FM) // WDSP AGC AND NOT FM
	{
		label = "AGC";
		value = ts.agc_wdsp_thresh;
	}
	else if(ts.dmod_mode == DEMOD_FM) // in both AGCs, use SQL for FM
	{
		label = "SQL";
		value = ts.fm_sql_threshold;
	}
	else // this is Standard AGC and NOT FM
	{
		if(encoder_active)
		{
			//
			// set color as warning that RX sensitivity is reduced
			//
			if(ts.rf_gain < 20)
				color = Red;
			else if(ts.rf_gain < 30)
				color = Orange;
			else if(ts.rf_gain < 40)
				color = Yellow;
		}
		label = "RFG";
		value = ts.rf_gain;
	}
	/*
    if(ts.dmod_mode != DEMOD_FM) // && !ts.agc_wdsp)	 	// If not FM, use RF gain
    {
        if()
        if(encoder_active)
        {
            //
            // set color as warning that RX sensitivity is reduced
            //
            if(ts.rf_gain < 20)
                color = Red;
            else if(ts.rf_gain < 30)
                color = Orange;
            else if(ts.rf_gain < 40)
                color = Yellow;
        }
        value = ts.rf_gain;
    }
    else if(!ts.agc_wdsp)	 						// it is FM, display squelch instead
    {
        value = ts.fm_sql_threshold;
    }
    else // it is WDSP AGC and NOT FM
    {
        value = ts.agc_wdsp_thresh;
    }

	 */
	snprintf(temp,5," %02ld",value);

	UiDriver_EncoderDisplay(0,1,label, encoder_active, temp, color);

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeSigProc
//* Object              : Display settings related to signal processing - DSP NR or Noise Blanker strength
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_DisplayNoiseBlanker(bool encoder_active)
{
	uint32_t 	color = encoder_active?White:Grey;
	char	temp[5];
	const char *label, *val_txt;
	int32_t value = 0;
	bool is_active = false;
	label = "NB";
	//    label = "DEC";
	if(ts.agc_wdsp == 0)
	{

		//
		// Noise blanker settings display
		//
		if(is_dsp_nb())	 	// is noise blanker to be displayed
		{
			if(encoder_active)
			{
				if(ts.nb_setting >= NB_WARNING3_SETTING)
					color = Red;		// above this value, make it red
				else if(ts.nb_setting >= NB_WARNING2_SETTING)
					color = Orange;		// above this value, make it orange
				else if(ts.nb_setting >= NB_WARNING1_SETTING)
					color = Yellow;		// above this value, make it yellow
				else
					color = White;		// Otherwise, make it white
			}
			label = "NB";
			value = ts.nb_setting;
			is_active = true;
		}

		if (is_active == false)
		{
			val_txt = "off";
		}
		else
		{
			snprintf(temp,5,"%3ld",value);
			val_txt = temp;
		}

	}
	else
	{
		switch(ts.agc_wdsp_mode)
		{
		case 0:
			label = "vLO";
			break;
		case 1:
			label = "LON";
			break;
		case 2:
			label = "SLO";
			break;
		case 3:
			label = "MED";
			break;
		case 4:
			label = "FAS";
			break;
		case 5:
			label = "OFF";
			break;
		default:
			label = "???";
			break;
		}
		value = (int32_t)(ts.agc_wdsp_tau_decay[ts.agc_wdsp_mode] / 10.0);
		snprintf(temp,5,"%3ld",value);
		val_txt = temp;
	}
	UiDriver_EncoderDisplay(1,1,label, encoder_active, val_txt, color);
}

#define NOTCH_DELTA_Y (2*ENC_ROW_H)

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDisplayBass
//* Object              : Display settings related to bass & treble filter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_DisplayTone(bool encoder_active)
{

	// UiLcdHy28_DrawFullRect(POS_AG_IND_X, POS_AG_IND_Y + NOTCH_DELTA_Y, 16 + 12 , 112, Black);

	bool enable = (ts.enc_two_mode == ENC_TWO_MODE_BASS_GAIN);
	char temp[5];
	int bas,tre;

	if(ts.txrx_mode == TRX_MODE_TX) // if in TX_mode, display TX bass gain instead of RX_bass gain!
	{
		bas = ts.tx_bass_gain;
		tre = ts.tx_treble_gain;
	}
	else
	{
		bas = ts.bass_gain;
		tre = ts.treble_gain;
	}

	snprintf(temp,5,"%3d", bas);

	// use 2,1 for placement below existing boxes
	UiDriver_EncoderDisplay(0,1,"BAS", enable && encoder_active, temp, White);


	enable = (ts.enc_two_mode == ENC_TWO_MODE_TREBLE_GAIN);

	snprintf(temp,5,"%3d", tre);

	// use 2,2 for placement below existing boxes
	UiDriver_EncoderDisplay(1,1,"TRB", enable && encoder_active, temp, White);

} // end void UiDriverDisplayBass

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeRit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_DisplayRit(bool encoder_active)
{
	char	temp[5];

	uint32_t color = ts.rit_value? Green : (encoder_active ? White:Grey);

	snprintf(temp,5,ts.rit_value?"%+3i":"%3i", ts.rit_value);

	UiDriver_EncoderDisplay(0,2,"RIT", encoder_active, temp, color);
}

static void UiDriver_DisplayDigitalMode()
{

	ushort bgclr = ts.dvmode?Orange:Blue;
	ushort color = digimodes[ts.digital_mode].enabled?(ts.dvmode?Black:White):Grey2;

	const char* txt = digimodes[ts.digital_mode].label;

	// Draw line for box
	UiLcdHy28_DrawStraightLine(POS_DIGMODE_IND_X,(POS_DIGMODE_IND_Y - 1),LEFTBOX_WIDTH,LCD_DIR_HORIZONTAL,bgclr);
	UiLcdHy28_PrintTextCentered((POS_DIGMODE_IND_X),(POS_DIGMODE_IND_Y),LEFTBOX_WIDTH,txt,color,bgclr,0);

	//fdv_clear_display();
}

static void UiDriver_DisplayPowerLevel()
{
	ushort color = White;
	const char* txt;

	switch(ts.power_level)
	{
	case PA_LEVEL_5W:
		txt = "5W";
		break;
	case PA_LEVEL_2W:
		txt = "2W";
		break;
	case PA_LEVEL_1W:
		txt = "1W";
		break;
	case PA_LEVEL_0_5W:
		txt = "0.5W";
		break;
	default:
		txt = "FULL";
		break;
	}
	// Draw top line
	UiLcdHy28_PrintTextCentered((POS_PW_IND_X),(POS_PW_IND_Y),POS_DEMOD_MODE_MASK_W,txt,color,Blue,0);
}

static void UiDriver_HandleSMeter()
{

	// Only in RX mode
	if(ts.txrx_mode == TRX_MODE_RX)
	{

		RadioManagement_HandleRxIQSignalCodecGain();

		// ONLY UI CODE BELOW
		//
		// This makes a portion of the S-meter go red if A/D clipping occurs
		//
		{
			static bool         clip_indicate = 0;

			if (ts.s_meter == DISPLAY_S_METER_STD) // oldschool (os) S-meter scheme
			{
				sm.gain_calc = AGC_GAIN_CAL/(ads.agc_val * ads.codec_gain_calc);
				// AGC gain calibration factor divided by (AGC loop gain setting * known A/D gain setting)
			}
			else if (ts.s_meter == DISPLAY_S_METER_DBM) // based on dBm calculation
			{
				sm.gain_calc = sm.dbm;
			}
			else // based on dBm/Hz calculation
			{
				sm.gain_calc = sm.dbmhz;
			}

			const float *S_Meter_Cal_Ptr = (ts.s_meter == DISPLAY_S_METER_STD)?  S_Meter_Cal : S_Meter_Cal_dbm;

			// find corresponding signal level
			for (
					sm.s_count = 0;
					(sm.gain_calc >= S_Meter_Cal_Ptr[sm.s_count]) && (sm.s_count < S_Meter_Cal_Size);
					sm.s_count++)
			{
				// nothing to do here
			}

			if(ads.adc_clip)	 		// did clipping occur?
			{
				if(!clip_indicate)	 	// have we seen it clip before?
				{
					UiDriver_DrawSMeter(Red);		// No, make the first portion of the S-meter red to indicate A/D overload
					clip_indicate = true;		// set flag indicating that we saw clipping and changed the screen (prevent continuous redraw)
				}
				ads.adc_clip = false;		// reset clip detect flag
			}
			else	 		// clipping NOT occur?
			{
				if(clip_indicate)	 	// had clipping occurred since we last visited this code?
				{
					UiDriver_DrawSMeter(White);					// yes - restore the S meter to a white condition
					clip_indicate = false;							// clear the flag that indicated that clipping had occurred
				}
			}

			// make sure that the S meter always reads something!
			UiDriver_UpdateTopMeterA((sm.s_count>0) ? sm.s_count : 1);
		}
	}
}



/**
 *
 * Power, SWR, ALC and Audio indicator handling
 */
static void UiDriver_HandleTXMeters()
{
	// Only in TX mode
	if(ts.txrx_mode != TRX_MODE_TX)
	{
		swrm.vswr_dampened = 0;		// reset averaged readings when not in TX mode
		swrm.fwd_pwr_avg = -1;
		swrm.rev_pwr_avg = -1;
	}
	else if (RadioManagement_UpdatePowerAndVSWR())
	{

		static uint8_t    old_power_level = 99;

		// display FWD, REV power, in milliwatts - used for calibration - IF ENABLED
		if(swrm.pwr_meter_disp)
		{
			if((swrm.fwd_pwr_avg < 0) || (ts.power_level != old_power_level))  	// initialize with current value if it was zero (e.g. reset) or power level changed
			{
				swrm.fwd_pwr_avg = swrm.fwd_pwr;
			}
			else
			{
				swrm.fwd_pwr_avg = (swrm.fwd_pwr_avg * (1-PWR_DAMPENING_FACTOR)) + swrm.fwd_pwr * PWR_DAMPENING_FACTOR;	// apply IIR smoothing to forward power reading
			}

			if((swrm.rev_pwr_avg < 0) || (ts.power_level != old_power_level))  	// initialize with current value if it was zero (e.g. reset) or power level changed
			{
				swrm.rev_pwr_avg = swrm.rev_pwr;
			}
			else
			{
				swrm.rev_pwr_avg = (swrm.rev_pwr_avg * (1-PWR_DAMPENING_FACTOR)) + swrm.rev_pwr * PWR_DAMPENING_FACTOR; // apply IIR smoothing to reverse power reading
			}

			old_power_level = ts.power_level;		// update power level change detector
		}

		{
			char txt[16];
			const char* txp = NULL;
			if (swrm.pwr_meter_disp)
			{
				snprintf(txt,16, "%5d,%5d", (int)(swrm.fwd_pwr_avg*1000), (int)(swrm.rev_pwr_avg*1000));		// scale to display power in milliwatts
				txp = txt;
				swrm.pwr_meter_was_disp = 1;	// indicate the power meter WAS displayed
			}
			else if(swrm.pwr_meter_was_disp)	// had the numerical display been enabled - and it is now disabled?
			{
				txp = "           ";            // yes - overwrite location of numerical power meter display to blank it
				swrm.pwr_meter_was_disp = 0;	// clear flag so we don't do this again
			}
			if (txp != NULL)
			{
				UiLcdHy28_PrintText(POS_PWR_NUM_IND_X, POS_PWR_NUM_IND_Y,txp,Grey,Black,0);
			}
		}


		// Do selectable meter readings
		float   btm_mtr_val = 0.0;
		uint32_t btm_mtr_red_level = 13;


		if(ts.tx_meter_mode == METER_SWR)
		{
			if(swrm.fwd_pwr >= SWR_MIN_CALC_POWER)	 		// is the forward power high enough for valid VSWR calculation?
			{
				// (Do nothing/freeze old data if below this power level)
				if(swrm.vswr_dampened < 1)	// initialize averaging if this is the first time (e.g. VSWR <1 = just returned from RX)
				{
					swrm.vswr_dampened = swrm.vswr;
				}
				else
				{
					swrm.vswr_dampened = swrm.vswr_dampened * (1 - VSWR_DAMPENING_FACTOR) + swrm.vswr * VSWR_DAMPENING_FACTOR;
				}
				btm_mtr_val = swrm.vswr_dampened * 4;		// yes - four dots per unit of VSWR
			}
		}
		else if(ts.tx_meter_mode == METER_ALC)
		{
			btm_mtr_val = ads.alc_val;		// get TX ALC value
			btm_mtr_val *= btm_mtr_val;		// square the value
			btm_mtr_val = log10f(btm_mtr_val);	// get the log10
			btm_mtr_val *= -10;		// convert it to DeciBels and switch sign and then scale it for the meter
		}
		else if(ts.tx_meter_mode == METER_AUDIO)
		{
			btm_mtr_val = ads.peak_audio/10000;		// get a copy of the peak TX audio (maximum reference = 30000)
			ads.peak_audio = 0;					// reset the peak detect
			btm_mtr_val *= btm_mtr_val;			// square the value
			btm_mtr_val = log10f(btm_mtr_val);	// get the log10
			btm_mtr_val *= 10;					// convert to DeciBels and scale for the meter
			btm_mtr_val += 11;					// offset for meter

			btm_mtr_red_level = 22;	// setting the "red" threshold
		}

		// calculate and display RF power reading
		UiDriver_UpdateTopMeterA(swrm.fwd_pwr * 3);
		// show selected bottom meter value
		UiDriver_UpdateBtmMeter(btm_mtr_val, btm_mtr_red_level);   // update the meter, setting the "red" threshold
	}
}


static void UiDriver_CreateVoltageDisplay() {
	// Create voltage
	UiLcdHy28_PrintTextCentered (POS_PWR_IND_X,POS_PWR_IND_Y,LEFTBOX_WIDTH,   "--.- V",  COL_PWR_IND,Black,0);
}

static bool UiDriver_SaveConfiguration()
{
	bool savedConfiguration = true;

	const char* txp;
	uint16_t txc;

	switch (ts.configstore_in_use)
	{
	case CONFIGSTORE_IN_USE_FLASH:
		txp = "Saving settings to Flash Memory";
		break;
	case CONFIGSTORE_IN_USE_I2C:
		txp = "Saving settings to I2C EEPROM";
		break;
	default:
		txp = "Detected problems: Not saving";
		savedConfiguration = false;
	}
	UiLcdHy28_PrintTextCentered(60,176,260,txp,Blue,Black,0);

	if (savedConfiguration)
	{
		// save settings
		if (UiConfiguration_SaveEepromValues() == 0)
		{
			txp = "Saving settings finished";
			txc = Green;
		}
		else
		{
			txp = "Saving settings failed";
			txc = Red;
			savedConfiguration = false;
		}
		UiLcdHy28_PrintTextCentered(60,188,260,txp,txc,Black,0);
	}
	return savedConfiguration;
}


/*
 * @brief displays the visual information that power down is being executed and saves EEPROM if requested
 */
static void UiDriver_PowerDownCleanup(bool saveConfiguration)
{
	const char* txp;
	// Power off all - high to disable main regulator

	ts.powering_down = 1;   // indicate that we should be powering down

	UiSpectrum_Clear();   // clear display under spectrum scope

	// hardware based mute
	Codec_MuteDAC(true);  // mute audio when powering down

	txp = " ";

	UiLcdHy28_PrintTextCentered(60,148,240,txp,Blue2,Black,0);
	UiLcdHy28_PrintTextCentered(60,156,240,"Powering off...",Blue2,Black,0);
	UiLcdHy28_PrintTextCentered(60,168,240,txp,Blue2,Black,0);

	if (saveConfiguration)
	{
		UiDriver_SaveConfiguration();
	}
	else
	{
		UiLcdHy28_PrintTextCentered(60,176,260,"...without saving settings...",Blue,Black,0);
	}


	if(saveConfiguration)
	{
		UiConfiguration_SaveEepromValues();     // save EEPROM values
	}

	HAL_Delay(3000);
}



/*
 * @brief Display external voltage
 */
static void UiDriver_DisplayVoltage()
{
	uint32_t low_power_threshold = ((ts.low_power_config & LOW_POWER_THRESHOLD_MASK) + LOW_POWER_THRESHOLD_OFFSET) * 10;
	// did we detect a voltage change?

	uint32_t col = COL_PWR_IND;  // Assume normal voltage, so Set normal color

	if (pwmt.voltage < low_power_threshold + 50)
	{
		col = Red;
	}
	else if (pwmt.voltage < low_power_threshold + 100)
	{
		col = Orange;
	}
	else if (pwmt.voltage < low_power_threshold + 150)
	{
		col = Yellow;
	}

	static uint8_t voltage_blink = 0;
	// in case of low power shutdown coming, we let the voltage blink with 1hz
	if (pwmt.undervoltage_detected == true && voltage_blink < 1 )
	{
		col = Black;
	}
	voltage_blink++;
	if (voltage_blink == 2)
	{
		voltage_blink = 0;
	}

	char digits[6];
	snprintf(digits,6,"%2ld.%02ld",pwmt.voltage/100,pwmt.voltage%100);
	UiLcdHy28_PrintText(POS_PWR_IND_X,POS_PWR_IND_Y,digits,col,Black,0);
}

/**
 * @brief Measures Voltage and controls undervoltage detection
 * @returns true if display update is required, false if not
 */
static bool UiDriver_HandleVoltage()
{
	bool retval = false;
	// if this is set to true, we should update the display because something relevant for the user happened.

	// Collect samples
	if(pwmt.p_curr < POWER_SAMPLES_CNT)
	{
		// Add to accumulator
		pwmt.pwr_aver = pwmt.pwr_aver + HAL_ADC_GetValue(&hadc1);
		pwmt.p_curr++;
	}
	else
	{

		// Get average
		uint32_t val_p  = ((pwmt.pwr_aver/POWER_SAMPLES_CNT) * (ts.voltmeter_calibrate + 900))/2500;

		// Reset accumulator
		pwmt.p_curr     = 0;
		pwmt.pwr_aver   = 0;


		retval = pwmt.voltage != val_p;

		pwmt.voltage = val_p;


		uint32_t low_power_threshold = ((ts.low_power_config & LOW_POWER_THRESHOLD_MASK) + LOW_POWER_THRESHOLD_OFFSET) * 10;
		bool low_power_shutdown_enabled = (ts.low_power_config & LOW_POWER_ENABLE_MASK) == LOW_POWER_ENABLE;

		if (low_power_shutdown_enabled && (val_p < low_power_threshold ))
		{
			// okay, voltage is too low, we should indicate
			pwmt.undervoltage_detected = true;
			retval = true;

			if (ts.txrx_mode == TRX_MODE_RX)
			{
				if (ts.sysclock > ts.low_power_shutdown_time )         // only allow power-off in RX mode
				{
					UiDriver_PowerDownCleanup(true);
				}
			}
			else
			{
				ts.low_power_shutdown_time = ts.sysclock + LOW_POWER_SHUTDOWN_DELAY_TIME;
				// in tx mode, we extend the waiting time during the transmit, so that we don't switch off
				// right after a transmit but let the battery some time to "regenerate"
			}
		}
		else
		{
			if (pwmt.undervoltage_detected == true)
			{
				retval = true;
				pwmt.undervoltage_detected = false;
				Board_GreenLed(LED_STATE_ON);
			}
			ts.low_power_shutdown_time = ts.sysclock + LOW_POWER_SHUTDOWN_DELAY_TIME;
		}
	}

	return retval;
}

#if 0
/*
 * @brief Displays temp compensation value in a bar
 */
static void UiDriverUpdateLoMeter(uchar val,uchar active)
{
	static int last_active = 99;
	static uint32_t last_active_val = 99;
	uchar 	i,v_s = 3;
	int		clr = White;

	if (last_active != active)
	{
		last_active = active;
		last_active_val = val;
		// Full redraw
		for(i = 1; i < 26; i++)
		{
			if (active)
			{
				clr = val==i?Blue:White;
			}
			else
			{
				clr = Grey;
			}
			UiLcdHy28_DrawStraightLineTriple(((POS_TEMP_IND_X + 1) + i*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,clr);
		}
	}
	else if (active && (last_active_val != val))
	{
		// Partial redraw
		if (val>1 && val < 26) {
			UiLcdHy28_DrawStraightLineTriple(((POS_TEMP_IND_X + 1) + val*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,Blue);
		}
		if (last_active_val>1 && last_active_val < 26) {
			UiLcdHy28_DrawStraightLineTriple(((POS_TEMP_IND_X + 1) + last_active_val*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,White);
		}
		last_active_val = val;
	}
}
#endif

/**
 * \brief draws the the TCXO temperature display, has to be called once
 *
 * @param create set to true in order to draw the static parts of the UI too.
 * @param enabled set to true in order to enable actual display of temperature
 */
#define POS_TEMP_IND_X_DATA (POS_TEMP_IND_X + 43)
void UiDriver_CreateTemperatureDisplay()
{
	const char *label, *txt;
	uint32_t label_color, txt_color;

	bool enabled = lo.sensor_present == true && RadioManagement_TcxoIsEnabled();

	label = "TCXO";
	label_color = Black;

	// Top part - name and temperature display
	UiLcdHy28_DrawEmptyRect( POS_TEMP_IND_X,POS_TEMP_IND_Y,13,109,Grey);

	if (enabled)
	{
		txt = RadioManagement_TcxoIsFahrenheit()?"*---.-F":"*---.-C";
		txt_color = Grey;
	}
	else
	{
		if (lo.sensor_present == false)
		{
			txt = "SENSOR!";
			txt_color = Red;
		}
		else
		{
			txt = "*  OFF ";
			txt_color = Grey;
		}
	}

	// Label
	UiLcdHy28_PrintText((POS_TEMP_IND_X + 1), (POS_TEMP_IND_Y + 1),label,label_color,Grey,0);
	// Lock Indicator
	UiLcdHy28_PrintText(POS_TEMP_IND_X_DATA,(POS_TEMP_IND_Y + 1), txt,txt_color,Black,0);	// show base string
}

/**
 * @brief display measured temperature and current state of TCXO
 * @param temp in tenth of degrees Celsius (10 == 1 degree C)
 */
static void UiDriver_DisplayTemperature(int temp)
{
	static int last_disp_temp = -100;
	uint32_t clr =  RadioManagement_TcxoGetMode() ==TCXO_ON ? Blue:Red;

	UiLcdHy28_PrintText(POS_TEMP_IND_X_DATA,(POS_TEMP_IND_Y + 1),"*",clr,Black,0);

	if (temp != last_disp_temp)
	{
		char out[10];
		char* txt_ptr;
		if((temp < 0) || (temp > 1000))  // is the temperature out of range?
		{
			txt_ptr = "RANGE!";
		}
		else {
			last_disp_temp = temp;

			int32_t ttemp = last_disp_temp;
			if(RadioManagement_TcxoIsFahrenheit())
			{
				ttemp = ((ttemp *9)/5) + 320;			// multiply by 1.8 and add 32 degrees
			}
			snprintf(out,10,"%3ld.%1ld",ttemp/10,(ttemp)%10);
			txt_ptr = out;
		}
		UiLcdHy28_PrintText(POS_TEMP_IND_X_DATA + SMALL_FONT_WIDTH*1,(POS_TEMP_IND_Y + 1),txt_ptr,Grey,Black,0);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleLoTemperature
//* Object              : display LO temperature and compensate drift
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriver_HandleLoTemperature()
{
	if (SoftTcxo_HandleLoTemperatureDrift())
	{
		UiDriver_DisplayTemperature(lo.temp/1000); // precision is 0.1 represent by lowest digit
	}
}



//*----------------------------------------------------------------------------
//* Function Name       : UiDriverEditMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverEditMode()
{
	char symb[2];

	// Is edit mode ?
	if(kbs.set_mode != 1)
		return;

	// Key pressed
	if(kbs.last_char == 0)
		return;

	//printf("key = %02x ",kbs.last_char);

	// Handle CR
	if(kbs.last_char == 0x0a)
	{
		kbs.edit_item_id++;
		if(kbs.edit_item_id == 3)
			kbs.edit_item_id = 0;

		// Switch items
		switch(kbs.edit_item_id)
		{
			case 0:
			{
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y +  0),"Call:  ",White,Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 15),"Loc:   ",Grey, Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 30),"Power: ",Grey, Black,0);
				break;
			}

			case 1:
			{
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y +  0),"Call:  ",Grey,Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 15),"Loc:   ",White, Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 30),"Power: ",Grey, Black,0);
				break;
			}

			case 2:
			{
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y +  0),"Call:  ",Grey,Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 15),"Loc:   ",Grey, Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 30),"Power: ",White, Black,0);
				break;
			}

			default:
				break;
		}

		// Reset hor ptr
		kbs.edit_item_hor = 0;
	}
	else
	{
		symb[0] = kbs.last_char;
		symb[1] = 0;

		// Print items
		switch(kbs.edit_item_id)
		{
			case 0:
			{
				// Add to buffer
				kbs.item_0[kbs.edit_item_hor] = kbs.last_char;

				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 80 + (kbs.edit_item_hor*12)),(POS_SPECTRUM_IND_Y +  0),symb,Grey,Black,0);
				break;
			}

			case 1:
			{
				// Add to buffer
				kbs.item_1[kbs.edit_item_hor] = kbs.last_char;

				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 80 + (kbs.edit_item_hor*12)),(POS_SPECTRUM_IND_Y + 15),symb,Grey, Black,0);
				break;
			}

			case 2:
			{
				// Add to buffer
				kbs.item_2[kbs.edit_item_hor] = kbs.last_char;

				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 80 + (kbs.edit_item_hor*12)),(POS_SPECTRUM_IND_Y + 30),symb,Grey, Black,0);
				break;
			}

			default:
				break;
		}

		// Move cursor right
		kbs.edit_item_hor++;
		if(kbs.edit_item_hor == 10)
			kbs.edit_item_hor = 0;
	}

	// Clear public
	kbs.last_char = 0;
}*/

typedef enum
{
	CONFIG_DEFAULTS_KEEP = 0,
	CONFIG_DEFAULTS_LOAD_FREQ,
	CONFIG_DEFAULTS_LOAD_ALL
} CONFIG_DEFAULTS;


/*
 * @brief Handles the loading of the configuration at startup (including the load of defaults if requested)
 * @returns false if it is a normal startup, true if defaults have been loaded
 */
static bool UiDriver_LoadSavedConfigurationAtStartup()
{

	bool retval = false;
	CONFIG_DEFAULTS load_mode = CONFIG_DEFAULTS_KEEP;

	if (UiDriver_IsButtonPressed(BUTTON_F1_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F3_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F5_PRESSED))
	{
		load_mode = CONFIG_DEFAULTS_LOAD_ALL;
	}
	else if (UiDriver_IsButtonPressed(BUTTON_F2_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F4_PRESSED))
	{
		load_mode = CONFIG_DEFAULTS_LOAD_FREQ;
	}

	if(load_mode != CONFIG_DEFAULTS_KEEP)
	{
		// let us make sure, the user knows what he/she is doing
		// in case of change of mindes, do normal configuration load

		uint32_t clr_fg, clr_bg;
		const char* top_line;

		switch (load_mode)
		{
		case CONFIG_DEFAULTS_LOAD_ALL:
			clr_bg = Red;
			clr_fg = White;
			top_line = "ALL DEFAULTS";
			break;
		case CONFIG_DEFAULTS_LOAD_FREQ:
			clr_bg = Yellow;
			clr_fg = Black;
			top_line = "FREQ/MODE DEFAULTS";
			break;
		default:
			break;
		}


		UiLcdHy28_LcdClear(clr_bg);							// clear the screen
		// now do all of the warnings, blah, blah...
		UiLcdHy28_PrintTextCentered(2,05, 316, top_line,clr_fg,clr_bg,1);
		UiLcdHy28_PrintTextCentered(2,35, 316, "-> LOAD REQUEST <-",clr_fg,clr_bg,1);

		UiLcdHy28_PrintTextCentered(2,70, 316,
				"If you don't want to do this\n"
				"press POWER button to start normally.",clr_fg,clr_bg,0);

		UiLcdHy28_PrintTextCentered(2,120, 316,
				"If you want to load default settings\n"
				"press and hold BAND+ AND BAND-.\n"
				"Settings will be saved at POWEROFF",clr_fg,clr_bg,0);

		// On screen delay									// delay a bit...
		HAL_Delay(5000);

		// add this for emphasis
		UiLcdHy28_PrintTextCentered(2,195, 316,
				"Press BAND+ and BAND-\n"
				"to confirm loading",clr_fg,clr_bg,0);

		while((((UiDriver_IsButtonPressed(BUTTON_BNDM_PRESSED)) && (UiDriver_IsButtonPressed(BUTTON_BNDP_PRESSED))) == false) && UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED) == false)
		{
			HAL_Delay(10);
		}

		const char* txp;

		if(UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED))
		{
			clr_bg = Black;							// clear the screen
			clr_fg = White;
			txp = "...performing normal start...";

			load_mode = CONFIG_DEFAULTS_KEEP;
			retval = false;
		}
		else
		{
			txp = "...loading defaults in progress...";
			// call function to load values - default instead of EEPROM
			retval = true;
			ts.menu_var_changed = true;
		}
		UiLcdHy28_LcdClear(clr_bg);                         // clear the screen
		UiLcdHy28_PrintTextCentered(2,108,316,txp,clr_fg,clr_bg,0);
		HAL_Delay(5000);
	}

	switch (load_mode)
	{
	case CONFIG_DEFAULTS_LOAD_ALL:
		ts.load_eeprom_defaults = true;                           // yes, set flag to indicate that defaults will be loaded instead of those from EEPROM
		break;
	case CONFIG_DEFAULTS_LOAD_FREQ:
		ts.load_freq_mode_defaults = true;
		break;
	default:
		break;
	}

	UiConfiguration_LoadEepromValues();
	ts.load_eeprom_defaults = false;
	ts.load_freq_mode_defaults = false;

	return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiCheckForPressedKey
//* Object              : Used for testing keys on the front panel
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//* Comments            : If a button (other than "POWER") is held during power-up a screen is displayed that shows which key (if any)
//  Comments            : is being pressed.  If multiple keys are being pressed, only the one with the highest precedence is displayed.  The order of decreasing
//  Comments            : precedence is:  M2, G3, G2, BNDM, G4, M3, STEPM, STEPP, M1, M3, F1, F2, F4, BNDP, F5, G1 and POWER.  [KA7OEI October, 2015]
//*----------------------------------------------------------------------------
static void UiDriver_KeyTestScreen()
{
	ushort i, j, k, p_o_state, rb_state, new_state;
	uint32_t poweroffcount, rbcount, enccount;
	int direction;
	uint32_t stat = 0;
	poweroffcount = rbcount = enccount = 0;
	p_o_state = rb_state = new_state = 0;
	char txt_buf[40];
	const char* txt;

	for(i = 0; i < buttons.num ; i++)	 			// scan all buttons
	{
		if(UiDriver_IsButtonPressed(i))	 		// is one button being pressed?
		{
			stat |= 1<<i;						// yes - remember which one
		}
	}

	if(stat) {			// a button was pressed
		UiLcdHy28_LcdClear(Blue);							// clear the screen
		snprintf(txt_buf,40,"Initial keys: %lx",stat);
		UiLcdHy28_PrintText(0,0,txt_buf,White,Blue,1);
		UiLcdHy28_PrintText(10,35,"Input Elements Test",White,Blue,1);
		UiLcdHy28_PrintText(15,70,"press & hold POWER button to poweroff",White,Blue,0);
		UiLcdHy28_PrintText(20,90,"press & hold BAND- button to reboot",White,Blue,0);

		for(;;)	 		// get stuck here for test duration
		{
			j = 99;		// load with flag value
			k = 0;

			for(i = 0; i < buttons.num ; i++)
			{
				// scan all buttons
				if(UiDriver_IsButtonPressed(i))
				{
					// is this button pressed?
					k++;
					if(j == 99)						// is this the first button pressed?
						j = i;						// save button number
				}
			}

			if(j == BUTTON_BNDM_PRESSED && new_state == 0)	// delay if BANDM was used to enter button test mode
			{
				rbcount = 0;
				new_state = 1;
			}

			char t;
			for(t = 0; t < ENC_MAX; t++)
			{
				direction = UiDriverEncoderRead(t);
				if(direction)
				{
					enccount = 50;
					break;
				}
			}
			if(t != ENC_MAX)
			{
				snprintf(txt_buf,40," Encoder %d <%s>", t+1, direction>0 ? "right":"left");		// building string for encoders
				j = 18+t;					// add encoders behind buttons;
			}

			if (j < buttons.num)
			{
				txt = buttons.map[j].label;
			}
			else
			{
				txt = NULL;
			}
			switch(j)	 				// decode button to text
			{
			case	BUTTON_POWER_PRESSED:
				if(poweroffcount > 75)
				{
					txt = "powering off...";
					p_o_state = 1;
				}
				poweroffcount++;
				break;
			case	BUTTON_BNDM_PRESSED:
				if(rbcount > 75)
				{
					txt = "rebooting...";
					rb_state = 1;
				}
				rbcount++;
				break;
			case	TOUCHSCREEN_ACTIVE:

				if (UiLcdHy28_TouchscreenHasProcessableCoordinates())
				{
					snprintf(txt_buf,40,"x/y: %02d/%02d x/y raw: %04x/%04x",ts.tp->x,ts.tp->y,ts.tp->xraw,ts.tp->yraw);	//show touched coordinates
					UiLcdHy28_PrintTextCentered(10,216,300,txt_buf,White,Blue,0);           // identify button on screen
					txt = "Touch";
				}
				else
				{
					if (mchf_touchscreen.present)
					{
						txt = "Touch (no coord.)";
					}
					else
					{
						txt = "Touch (no cntrlr)";
					}
				}
				break;
			case	18+ENC1:							// handle encoder event
			case	18+ENC2:
			case	18+ENC3:
			case	18+ENCFREQ:
			txt = txt_buf;
			break;
			default:
				if (txt == NULL)
				{
					if(!enccount)
					{
						txt = "<no button>";				// no button pressed
					}
					else
					{
						txt = "";
						enccount--;
					}
					poweroffcount = 0;
					rbcount = 0;
				}
			}

			if(txt[0])
			{
				UiLcdHy28_PrintTextCentered(10,120,300,txt,White,Blue,1);			// identify button on screen
			}

			snprintf(txt_buf,40, "# of buttons pressed: %d  ", (int)k);
			UiLcdHy28_PrintText(75,160,txt_buf,White,Blue,0);			// show number of buttons pressed on screen

			if(ts.tp->present)			// show translation of touchscreen if present
			{
				txt = "Touch Coordinates:";
			}
			else
			{
				txt = "Touch Controller not present";
			}

			UiLcdHy28_PrintTextCentered(10,200,300,txt,White,Blue,0);

			if(p_o_state == 1)
			{
				Board_Powerdown();
				// never reached
			}
			if(rb_state == 1)
			{
				if(j != BUTTON_BNDM_PRESSED)
				{
					Board_Reboot();
				}
			}
		}
	}
}

/*
 * @brief Touchscreen Calibration function
 * @returns false if it is a normal startup, true if touchscreen has been calibrated
 */

static bool UiDriver_TouchscreenCalibration()
{

	bool retval = false;

	if (UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE) && UiDriver_IsButtonPressed(BUTTON_F5_PRESSED))
	{

		uint32_t clr_fg, clr_bg;
		clr_bg = Magenta;
		clr_fg = White;

		char cross1[2] = {3,55};
		char cross2[2] = {56,55};
		char cross3[2] = {3,4};
		char cross4[2] = {56,4};
		char cross5[2] = {25,32};

		char x_corr[1], y_corr[1];
		float diffx,diffy;

		*x_corr = 0;
		*y_corr = 0;

		UiLcdHy28_LcdClear(clr_bg);							// clear the screen
		//											// now do all of the warnings, blah, blah...
		UiLcdHy28_PrintText(0,05,"  TOUCH CALIBRATION",clr_fg,clr_bg,1);
		UiLcdHy28_PrintTextCentered(2, 70, 316, "If you don't want to do this\n"
				"press POWER button to start normally.\n"
				"press and hold BAND+ AND BAND-.\n"
				" Settings will be saved at POWEROFF"
				,clr_fg,clr_bg,0);
		//
		// On screen delay									// delay a bit...
		HAL_Delay(5000);

		// add this for emphasis
		UiLcdHy28_PrintTextCentered(2, 195, 316, "Press BAND+ and BAND-\n"
				"to start calibration",clr_fg,clr_bg,0);

		while((((UiDriver_IsButtonPressed(BUTTON_BNDM_PRESSED)) && (UiDriver_IsButtonPressed(BUTTON_BNDP_PRESSED))) == false) && UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED) == false)
		{
			HAL_Delay(10);
		}


		if(UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED))
		{
			UiLcdHy28_LcdClear(Black);							// clear the screen
			UiLcdHy28_PrintText(2,108,"      ...performing normal start...",White,Black,0);
			HAL_Delay(3000);
			retval = false;
		}
		else
		{
			UiLcdHy28_LcdClear(clr_bg);							// clear the screen
			UiLcdHy28_PrintTextCentered(2,70, 316,
					"On the next screen crosses will appear.\n"
					"Touch as exact as you can on the middle\n"
					"of each cross. After three valid\n"
					"samples position of cross changes.\n"
					"Repeat until the five test positions\n"
					"are finished.",clr_fg,clr_bg,0);

			UiLcdHy28_PrintText(35,195,"Touch at any position to start.",clr_fg,clr_bg,0);

			while(UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE) == false)
			{
				HAL_Delay(40);
			}
			UiLcdHy28_TouchscreenReadCoordinates();
			ts.tp->state = TP_DATASETS_NONE;

			UiLcdHy28_LcdClear(clr_bg);							// clear the screen
			UiLcdHy28_PrintText(10,10,"+",clr_fg,clr_bg,1);


			UiDriver_DoCrossCheck(cross1, x_corr, y_corr);

			UiLcdHy28_LcdClear(clr_bg);
			clr_fg = White;
			UiLcdHy28_PrintText(10,10,"                  +",clr_fg,clr_bg,1);

			//    sprintf(txt_buf,"temp_corr is  : %d/%d", *x_corr/3, *y_corr/3);
			//    UiLcdHy28_PrintText(10,140,txt_buf,clr_fg,clr_bg,0);

			UiDriver_DoCrossCheck(cross2, x_corr, y_corr);

			UiLcdHy28_LcdClear(clr_bg);
			clr_fg = White;
			UiLcdHy28_PrintText(10,210,"+",clr_fg,clr_bg,1);

			//    sprintf(txt_buf,"temp_corr is  : %d/%d", *x_corr/6, *y_corr/6);
			//    UiLcdHy28_PrintText(10,140,txt_buf,clr_fg,clr_bg,0);

			UiDriver_DoCrossCheck(cross3, x_corr, y_corr);

			UiLcdHy28_LcdClear(clr_bg);
			clr_fg = White;
			UiLcdHy28_PrintText(10,210,"                  +",clr_fg,clr_bg,1);

			//    sprintf(txt_buf,"temp_corr is  : %d/%d", *x_corr/9, *y_corr/9);
			//    UiLcdHy28_PrintText(10,140,txt_buf,clr_fg,clr_bg,0);

			UiDriver_DoCrossCheck(cross4, x_corr, y_corr);

			UiLcdHy28_LcdClear(clr_bg);
			clr_fg = White;
			UiLcdHy28_PrintText(10,110,"         +",clr_fg,clr_bg,1);

			//    sprintf(txt_buf,"temp_corr is  : %d/%d", *x_corr/12, *y_corr/12);
			//    UiLcdHy28_PrintText(10,140,txt_buf,clr_fg,clr_bg,0);

			UiDriver_DoCrossCheck(cross5, x_corr, y_corr);

			diffx = roundf(*x_corr / 15);
			diffy = roundf(*y_corr / 15);
			*x_corr = diffx;
			*y_corr = diffy;

			UiLcdHy28_LcdClear(clr_bg);

			//    sprintf(txt_buf,"correction is  : %d/%d", *x_corr, *y_corr);
			//    UiLcdHy28_PrintText(10,55,txt_buf,clr_fg,clr_bg,0);

			HAL_Delay(4000);
			retval = true;
			ts.menu_var_changed = true;
		}
	}
	return retval;
}



void UiDriver_DoCrossCheck(char cross[],char* xt_corr, char* yt_corr)
{
	uint32_t clr_fg, clr_bg;
	char txt_buf[40];
	uchar datavalid = 0, samples = 0;

	clr_bg = Magenta;
	clr_fg = White;

	do
	{
		while(UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE) == false)
		{
			HAL_Delay(40);
		}

		if (UiLcdHy28_TouchscreenHasProcessableCoordinates())
		{
			if(abs(ts.tp->x - cross[0]) < 4 && abs(ts.tp->y - cross[1]) < 4)
			{
				datavalid++;
				*xt_corr += (ts.tp->x - cross[0]);
				*yt_corr += (ts.tp->y - cross[1]);
				clr_fg = Green;
				snprintf(txt_buf,40,"Try (%d) error: x = %+d / y = %+d       ",datavalid,ts.tp->x-cross[0],ts.tp->y-cross[1]);	//show misajustments
			}
			else
			{
				clr_fg = Red;
				snprintf(txt_buf,40,"Try (%d) BIG error: x = %+d / y = %+d",samples,ts.tp->x-cross[0],ts.tp->y-cross[1]);	//show misajustments
			}
			samples++;
			UiLcdHy28_PrintText(10,70,txt_buf,clr_fg,clr_bg,0);
			ts.tp->state = TP_DATASETS_PROCESSED;
		}
	}
	while(datavalid < 3);

	HAL_Delay(4000);
}

static uint16_t startUpScreen_nextLineY;
static bool startUpError = false;

/**
 * @brief use this method to report initialization problems on splash screen, may only be used during splash screen presence (!)
 *
 * @param isProblem if set to true, the problem is reported on screen, otherwise nothing is done
 * @param txt pointer to a text string characterizing the problem detected
 *
 */
void UiDriver_StartupScreen_LogIfProblem(bool isProblem, const char* txt)
{
	if (isProblem)
	{
		startUpScreen_nextLineY = UiLcdHy28_PrintTextCentered(0,startUpScreen_nextLineY,320,txt,Black,Red3,0);
		startUpError = true;
	}
}

static uint16_t fw_version_number_major = 0;    // save new F/W version
static uint16_t fw_version_number_release = 0;
static uint16_t fw_version_number_minor = 0;

/**
 * @returns true if the firmware version is different from version in loaded configuraton settings.
 */
static bool UiDriver_FirmwareVersionCheck()
{

	fw_version_number_major = atoi(UHSDR_VER_MAJOR);    // save new F/W version
	fw_version_number_release = atoi(UHSDR_VER_RELEASE);
	fw_version_number_minor = atoi(UHSDR_VER_MINOR);

	return ((ts.version_number_major != fw_version_number_major) || (ts.version_number_release != fw_version_number_release) || (ts.version_number_minor != fw_version_number_minor));        // Yes - check for new version
}
/**
 * @brief basically does nothing but settiSng the firmware number of configuration to number of running fw
 */
static void UiDriver_FirmwareVersionUpdateConfig()
{

	if (UiDriver_FirmwareVersionCheck())
	{
		ts.version_number_major = fw_version_number_major;    // save new F/W version
		ts.version_number_release = fw_version_number_release;
		ts.version_number_minor = fw_version_number_minor;

	}
}


/**
 * @brief show initial splash screen, we run AFTER the configuration storage has been initialized!
 * @param hold_time how long the screen to be shown before proceeding (in ms)
 */
void UiDriver_StartUpScreenInit()
{
	char   tx[100];
	uint32_t clr;
	// Clear all
	UiLcdHy28_LcdClear(Black);
	uint16_t nextY = 10;
	snprintf(tx,100,"%s",DEVICE_STRING);
	nextY = UiLcdHy28_PrintTextCentered(0, nextY, 320, tx, Cyan, Black, 1);

#ifdef TRX_HW_LIC
	snprintf(tx,100,"Hardware License: %s",TRX_HW_LIC);
	nextY = UiLcdHy28_PrintTextCentered(0, nextY + 3, 320, tx, White,Black, 0);
#endif
#ifdef TRX_HW_CREATOR
	nextY = UiLcdHy28_PrintTextCentered(0, nextY, 320, TRX_HW_CREATOR, White,Black, 0);
#endif

	snprintf(tx,100,"%s%s","UHSDR Vers. ",UiMenu_GetSystemInfo(&clr,INFO_FW_VERSION));
	nextY = UiLcdHy28_PrintTextCentered(0, nextY + 8, 320, tx, Yellow, Black, 1);

	nextY = UiLcdHy28_PrintTextCentered(0, nextY + 3, 320, "Firmware License: " UHSDR_LICENCE "\n" UHSDR_REPO, White, Black, 0);

	// show important error status
	startUpScreen_nextLineY = nextY + 8; // reset y coord to first line of error messages

	UiLcdHy28_BacklightEnable(true);

}

void UiDriver_StartUpScreenFinish()
{
	const char* txp;
	char   tx[100];
	uint32_t clr, fg_clr;

	uint32_t hold_time;

	UiDriver_StartupScreen_LogIfProblem(Si570_IsPresent() == false, "Si570 Oscillator NOT Detected!");
	UiDriver_StartupScreen_LogIfProblem(lo.sensor_present == false, "MCP9801 Temp Sensor NOT Detected!");

	if(ts.ee_init_stat != HAL_OK)                                   // problem with EEPROM init
	{
		snprintf(tx,100, "EEPROM Init Error Code: %d", ts.ee_init_stat);
		UiDriver_StartupScreen_LogIfProblem(true, tx);
	}

	UiDriver_StartupScreen_LogIfProblem((HAL_ADC_GetValue(&hadc2) > MAX_VSWR_MOD_VALUE) && (HAL_ADC_GetValue(&hadc3) > MAX_VSWR_MOD_VALUE),
			"SWR Bridge resistor mod NOT completed!");

	if (UiDriver_FirmwareVersionCheck())
	{
		hold_time = 10000; // 15s
		txp = "Firmware change detected!\nPlease review settings!";
		startUpScreen_nextLineY = UiLcdHy28_PrintTextCentered(0,startUpScreen_nextLineY + 10,320,txp,White,Black,0);

		UiDriver_FirmwareVersionUpdateConfig();
	}

	if(startUpError == true)
	{
		hold_time = 15000; // 15s
		txp = "Boot Delay because of Errors or Warnings";
		clr = Red3;
		fg_clr = Black;
	}
	else
	{
		hold_time = 3000; // 3s
		txp = "...starting up normally...";
		clr =  Black;
		fg_clr = Green;
	}

	UiLcdHy28_PrintTextCentered(0,startUpScreen_nextLineY + 10,320,txp,fg_clr,clr,0);

	HAL_Delay(hold_time);

	UiDriver_CreateDesktop();
	UiDriver_UpdateDisplayAfterParamChange();
}

// UiAction_... are typically small functions to execute a specific ui function initiate by a key press or touch event
// they take no argument and return nothing. If possible, try to keep the function atomic and independent from the
// key or touch region it is assigned to. I.e. it is better not to implement 2 functions based on menu mode or not here
// this logic should done separately so that the resulting action is reusable in different activation scenarios (touch or key or even CAT)
// If execution of an action is not applicable all the times it is necessary to check if the function should check that
// by itself and become a "No operation" or even issues a message, or if this is to be implicitly handled by the use of the
// function only in certain modes of operation through the modes tables.

// TODO: Make Atomic
static void UiAction_ChangeLowerMeterDownOrSnap()
{
#ifdef USE_SNAP
	sc.snap = 1;
#else
	// Not in MENU mode - select the METER mode
	decr_wrap_uint8(&ts.tx_meter_mode,0,METER_MAX-1);
	UiDriver_DeleteMeters();
	UiDriver_CreateMeters();    // redraw meter
#endif
}

static void UiAction_ChangeLowerMeterUp()
{
	incr_wrap_uint8(&ts.tx_meter_mode,0,METER_MAX-1);
	UiDriver_DeleteMeters();
	UiDriver_CreateMeters();	// redraw meter
}

static void UiAction_ChangeToNextDspMode()
{
	if(ts.dmod_mode != DEMOD_FM)	  // allow selection/change of DSP only if NOT in FM
	{
		//
		// I think we should alter this to use a counter
		// What do we want to switch here:
		// NR ON/OFF		ts.dsp_active |= DSP_NR_ENABLE;	 // 	ts.dsp_active &= ~DSP_NR_ENABLE;
		// NOTCH ON/OFF		ts.dsp_active |= DSP_NOTCH_ENABLE; // 	ts.dsp_active &= ~DSP_NOTCH_ENABLE;
		// Manual Notch		ts.dsp_active |= DSP_MNOTCH_ENABLE
		// BASS				ts.bass // always "ON", gain ranges from -20 to +20 dB, "OFF" = 0dB
		// TREBLE			ts.treble // always "ON", gain ranges from -20 to +20 dB, "OFF" = 0dB

		ts.dsp_mode ++; // switch mode
		// 0 = everything OFF, 1 = NR, 2 = automatic NOTCH, 3 = NR + NOTCH, 4 = manual NOTCH, 5 = BASS adjustment, 6 = TREBLE adjustment
		if (ts.dsp_mode >= DSP_SWITCH_MAX) ts.dsp_mode = DSP_SWITCH_OFF; // flip round
		//
		// prevent certain modes to prevent CPU crash
		//
		// prevent NR AND NOTCH, when in CW
		if (ts.dsp_mode == DSP_SWITCH_NR_AND_NOTCH && ts.dmod_mode == DEMOD_CW) ts.dsp_mode ++;
		// prevent NOTCH, when in CW
		if (ts.dsp_mode == DSP_SWITCH_NOTCH && ts.dmod_mode == DEMOD_CW) ts.dsp_mode ++;
		// prevent NR AND NOTCH, when in AM and decimation rate equals 2 --> high CPU load)
		if (ts.dsp_mode == DSP_SWITCH_NR_AND_NOTCH && (ts.dmod_mode == DEMOD_AM) && (FilterPathInfo[ts.filter_path].sample_rate_dec == RX_DECIMATION_RATE_24KHZ )) ts.dsp_mode++;

		switch (ts.dsp_mode)
		{

		case DSP_SWITCH_OFF: // switch off everything
			ts.dsp_active =  (ts.dsp_active & ~(DSP_NR_ENABLE|DSP_NOTCH_ENABLE|DSP_MNOTCH_ENABLE|DSP_MPEAK_ENABLE));
			ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;
			break;
		case DSP_SWITCH_NR:
			ts.dsp_active =  DSP_NR_ENABLE | (ts.dsp_active & ~(DSP_NOTCH_ENABLE|DSP_MNOTCH_ENABLE|DSP_MPEAK_ENABLE));
			ts.enc_two_mode = ENC_TWO_MODE_NR;
			break;
		case DSP_SWITCH_NOTCH:
			ts.dsp_active =  DSP_NOTCH_ENABLE | (ts.dsp_active & ~(DSP_NR_ENABLE|DSP_MNOTCH_ENABLE|DSP_MPEAK_ENABLE));
			ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;
			break;
		case DSP_SWITCH_NR_AND_NOTCH:
			ts.dsp_active =  DSP_NOTCH_ENABLE | DSP_NR_ENABLE | (ts.dsp_active & ~(DSP_MNOTCH_ENABLE|DSP_MPEAK_ENABLE));
			ts.enc_two_mode = ENC_TWO_MODE_NR;
			break;
		case DSP_SWITCH_NOTCH_MANUAL:
			ts.dsp_active =  DSP_MNOTCH_ENABLE | (ts.dsp_active & ~(DSP_NR_ENABLE|DSP_NOTCH_ENABLE|DSP_MPEAK_ENABLE));
			ts.enc_two_mode = ENC_TWO_MODE_NOTCH_F;
			break;
		case DSP_SWITCH_PEAK_FILTER:
			ts.dsp_active =  DSP_MPEAK_ENABLE | (ts.dsp_active & ~(DSP_NR_ENABLE|DSP_NOTCH_ENABLE|DSP_MNOTCH_ENABLE));
			ts.enc_two_mode = ENC_TWO_MODE_PEAK_F;
			break;
		default:
			break;
		}

		ts.dsp_active_toggle = ts.dsp_active;  // save update in "toggle" variable
		// reset DSP NR coefficients
		AudioDriver_SetRxAudioProcessing(ts.dmod_mode, true);        // update DSP/filter settings
		UiDriver_DisplayEncoderTwoMode();         // DSP control is mapped to column 2
	}
}


static void UiAction_ChangeSpectrumZoomLevelDown()
{
	ts.menu_var_changed = 1;
	decr_wrap_uint8(&sd.magnify,MAGNIFY_MIN,MAGNIFY_MAX);

	UiDriver_SpectrumZoomChangeLevel();
}

static void UiAction_ChangeSpectrumZoomLevelUp()
{
	ts.menu_var_changed = 1;
	incr_wrap_uint8(&sd.magnify,MAGNIFY_MIN,MAGNIFY_MAX);
	UiDriver_SpectrumZoomChangeLevel();
}

static void UiAction_ChangeFrequencyToNextKhz()
{
	df.tune_new = floor(df.tune_new / (TUNE_MULT*1000)) * (TUNE_MULT*1000);	// set last three digits to "0"
	UiDriver_FrequencyUpdateLOandDisplay(true);
}

static void UiAction_ToggleWaterfallScopeDisplay()
{
	if(is_waterfallmode())
	{
		// is the waterfall mode active?
		ts.flags1 &=  ~FLAGS1_WFALL_SCOPE_TOGGLE;     // yes, turn it off
	}
	else
	{
		// waterfall mode was turned off
		ts.flags1 |=  FLAGS1_WFALL_SCOPE_TOGGLE;          // turn it on
	}
	UiSpectrum_Init();   // init spectrum display
}

static void UiAction_ChangeDemodMode()
{
	if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX))	 	// do NOT allow mode change in TUNE mode or transmit mode
	{
		UiDriver_ChangeToNextDemodMode(0);
	}
}
static void UiAction_ChangeDemodModeToAlternativeMode()
{
	if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX))	 	// do NOT allow mode change in TUNE mode or transmit mode
	{
		UiDriver_ChangeToNextDemodMode(1);
	}
}

static void UiAction_ChangePowerLevel()
{
	UiDriver_HandlePowerLevelChange(ts.power_level+1);
}

static void UiAction_ChangeAudioSource()
{
	if(ts.dmod_mode != DEMOD_CW)
	{
		incr_wrap_uint8(&ts.tx_audio_source,0,TX_AUDIO_MAX_ITEMS);

		UiDriver_DisplayEncoderThreeMode();
	}
}

// TODO: Decide if we really want to switch
// order like for the normal buttons
static void UiAction_ChangeBandDownOrUp()
{
	UiDriver_HandleBandButtons(BUTTON_BNDM);
}

static void UiAction_ChangeBandUpOrDown()
{
	UiDriver_HandleBandButtons(BUTTON_BNDP);
}

static void UiAction_SaveConfigurationToMemory()
{
	if(ts.txrx_mode == TRX_MODE_RX)	 				// only allow EEPROM write in receive mode
	{
		UiSpectrum_Clear();
		UiDriver_SaveConfiguration();
		HAL_Delay(3000);

		ts.menu_var_changed = 0;                    // clear "EEPROM SAVE IS NECESSARY" indicators
		UiDriver_DisplayFButton_F1MenuExit();

		if(ts.menu_mode)
		{
			UiMenu_RenderMenu(MENU_RENDER_ONLY);    // update menu display, was destroyed by message
		}
		else
		{
			UiSpectrum_Init();          // not in menu mode, redraw spectrum scope
		}
	}
}

static void UiAction_ChangeFrequencyByTouch()
{
	if (ts.frequency_lock == false)
	{
		int step = 2000;				// adjust to 500Hz

		if(sd.magnify == 3)
		{
			step = 400;					// adjust to 100Hz
		}
		if(sd.magnify == 4)
		{
			step = 40;					// adjust to 10Hz
		}
		if(sd.magnify == 5)
		{
			step = 4;					// adjust to 1Hz
		}
		if(ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_SAM)
		{
			step = 20000;				// adjust to 5KHz
		}

		uint16_t line = 29;				// x-position of rx frequency in middle position
		if(sd.magnify == 0)				// x-position differs in translated modes if not magnified
		{
			switch(ts.iq_freq_mode)
			{
			case FREQ_IQ_CONV_P6KHZ:
				line = 25;
				break;
			case FREQ_IQ_CONV_M6KHZ:
				line = 35;
				break;
			case FREQ_IQ_CONV_P12KHZ:
				line = 19;
				break;
			case FREQ_IQ_CONV_M12KHZ:
				line = 43;
				break;
			default:
				line = 29;
			}
		}
		else
		{
		  if(ts.dmod_mode == DEMOD_CW)	// x position differs if dmod_mode is CW, only relevant
		  {								// if magnify > 1
			uint32_t offset;
			offset = (ts.cw_sidetone_freq * (1 << (sd.magnify))) / 1000;
			offset = ts.cw_lsb?(-offset):offset;		// CW-L or CW-U?
			line = 29 + offset;
		  }
		}

		uint32_t tunediff = ((1000)/(1 << sd.magnify))*(ts.tp->x-line)*TUNE_MULT;
		df.tune_new = lround((df.tune_new + tunediff)/step) * step;
		UiDriver_FrequencyUpdateLOandDisplay(true);
	}
}

static void UiAction_ChangeDigitalMode()
{
	incr_wrap_uint8(&ts.digital_mode,0,DigitalMode_RTTY);
	// We limit the reachable modes to the ones truly available
	// which is FreeDV1 and RTTY for now
	UiDriver_ToggleDigitalMode();
}

static void UiAction_ChangeDynamicTuning()
{
	if (!(ts.flags1 & FLAGS1_DYN_TUNE_ENABLE))			// is it off??
	{
		ts.flags1 |= FLAGS1_DYN_TUNE_ENABLE;	// then turn it on
	}
	else
	{
		ts.flags1 &= ~FLAGS1_DYN_TUNE_ENABLE;	// then turn it off
	}

	UiDriver_DisplayFreqStepSize();
}

static void UiAction_ChangeDebugInfoDisplay()
{
	UiDriver_DebugInfo_DisplayEnable(!ts.show_debug_info);
}

static void UiAction_ChangeRfModPresence()
{
	ts.rfmod_present = !ts.rfmod_present;
}

static void UiAction_ChangeVhfUhfModPresence()
{
	ts.vhfuhfmod_present = !ts.vhfuhfmod_present;
}

static void UiAction_ChangeFilterBW()
{
	if(!ts.tune)
	{
		if (filter_path_change == true)
		{
			filter_path_change = false;
		}
		else
		{
			AudioFilter_NextApplicableFilterPath(PATH_USE_RULES,AudioFilter_GetFilterModeFromDemodMode(ts.dmod_mode),ts.filter_path);
			// we store the new filter in the current active filter location
			AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
			// we activate it (in fact the last used one, which is the newly selected one);
		}
		// Change filter
		UiDriver_UpdateDisplayAfterParamChange();		// re-init for change of filter including display updates
		UiDriver_DisplayEncoderThreeMode();
	}
}

static void UiAction_ChangeTuningStepDownOrUp()
{
	UiDriver_ChangeTuningStep(ts.freq_step_config & FREQ_STEP_SWAP_BTN? true: false);
}

static void UiAction_ChangeTuningStepUpOrDown()
{
	UiDriver_ChangeTuningStep(ts.freq_step_config & FREQ_STEP_SWAP_BTN? false: true);
}

static void UiAction_ChangeBacklightBrightness()
{
	incr_wrap_uint8(&ts.lcd_backlight_brightness,0,3);
}

static void UiAction_ToggleTxDisable()
{
	if(ts.txrx_mode == TRX_MODE_RX)			// do NOT allow mode change in TUNE mode or transmit mode
	{
		if( RadioManagement_IsTxDisabledBy(TX_DISABLE_USER) == false)
		{
			ts.tx_disable |= TX_DISABLE_USER;
		}
		else
		{
			ts.tx_disable &= ~TX_DISABLE_USER;
		}
		UiDriver_FButton_F5Tune();
	}
}

static void UiAction_ToggleVfoMem()
{
	RadioManagement_ToggleVfoMem();
	UiDriver_FButton_F3MemSplit();
}

static void UiAction_ToggleDspEnable()
{
	if(ts.dmod_mode != DEMOD_FM)	 		// do not allow change of mode when in FM
	{
		if(is_dsp_nr()|| is_dsp_notch() || is_dsp_mnotch() || is_dsp_mpeak())	 			// is any DSP function active?
		{
			ts.dsp_active_toggle = ts.dsp_active;	// save setting for future toggling

			ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE |DSP_MNOTCH_ENABLE | DSP_MPEAK_ENABLE);				// turn off NR and notch

			ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;
		}
		else	 		// neither notch or NR was active
		{
			if(ts.dsp_active_toggle != 0xff)	 	// has this holder been used before?
			{
				ts.dsp_active = ts.dsp_active_toggle;	// yes - load value
			}
		}
		AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);	// update DSP settings
		UiDriver_DisplayEncoderTwoMode();
	}
}

// TODO: Split into separate actions and a composition
static void UiAction_ChangeRxFilterOrFmToneBurst()
{
	if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX))	  // only allow in receive mode and when NOT in FM
	{
		filter_path_change = true;
		UiDriver_DisplayFilter();
		UiDriver_DisplayEncoderThreeMode();
	}
	else if((ts.txrx_mode == TRX_MODE_TX) && (ts.dmod_mode == DEMOD_FM))
	{
		if(ts.fm_tone_burst_mode != FM_TONE_BURST_OFF)	 	// is tone burst mode enabled?
		{
			ads.fm_tone_burst_active = 1;					// activate the tone burst
			ts.fm_tone_burst_timing = ts.sysclock + FM_TONE_BURST_DURATION;	// set the duration/timing of the tone burst
		}
	}
}

static void UiAction_ToggleNoiseblanker()
{
	ts.dsp_active ^= DSP_NB_ENABLE;	// toggle whether or not DSP or NB is to be displayed
	UiDriver_DisplayEncoderTwoMode();
}

static void UiAction_ToggleTuneMode()
{
	ts.tune = RadioManagement_Tune(!ts.tune);
	UiDriver_DisplayPowerLevel();           // tuning may change power level temporarily
	UiDriver_FButton_F5Tune();
}

static void UiAction_PlayKeyerBtnN(int8_t n)
{
	uint8_t *pmacro;
	uint16_t c = 0;

	if (ts.keyer_mode.button_recording == KEYER_BUTTON_NONE)
	{
		pmacro = (uint8_t *)ts.keyer_mode.macro[n];
		if (*pmacro != '\0') // If there is a macro
		{
			while (*pmacro != '\0')
			{
				DigiModes_TxBufferPutChar(*pmacro++);
			}

			if (ts.dmod_mode == DEMOD_CW && ts.cw_keyer_mode != CW_KEYER_MODE_STRAIGHT)
			{
				ts.ptt_req = true;
			}
		}

		UiDriver_TextMsgPutChar('>');
	}
	else if (ts.keyer_mode.button_recording == n)
	{
		ts.cw_text_entry = false;
		pmacro = (uint8_t *)ts.keyer_mode.macro[n];
		c = 0;
		while (DigiModes_TxBufferHasData())
		{
			if (++c > KEYER_MACRO_LEN - 1) {
				break;
			}
			DigiModes_TxBufferRemove(pmacro++);
		}
		*pmacro = '\0';

		if (c)
		{
			// Make button label from start of the macro
			pmacro = (uint8_t *)ts.keyer_mode.macro[n];
			c = 0;
			while(*pmacro != ' ' && *pmacro != '\0' && c < KEYER_CAP_LEN)
			{
				ts.keyer_mode.cap[n][c++] = *pmacro++;
			}
			ts.keyer_mode.cap[n][c] = '\0';
		}
		else
		{
			strcpy((char *) ts.keyer_mode.cap[n], "BTN");
		}

		UiDriver_TextMsgPutChar('<');
		ts.keyer_mode.button_recording = KEYER_BUTTON_NONE;
	}
	UiDriver_CreateFunctionButtons(false);
}

static void UiAction_PlayKeyerBtn1()
{
	UiAction_PlayKeyerBtnN(KEYER_BUTTON_1);
}

static void UiAction_PlayKeyerBtn2()
{
	UiAction_PlayKeyerBtnN(KEYER_BUTTON_2);
}

static void UiAction_PlayKeyerBtn3()
{
	UiAction_PlayKeyerBtnN(KEYER_BUTTON_3);
}

static void UiAction_RecordKeyerBtnN(int8_t n)
{
	if (ts.keyer_mode.button_recording == KEYER_BUTTON_NONE && ts.txrx_mode == TRX_MODE_RX && !ts.cw_text_entry)
	{
		ts.cw_text_entry = true;
		ts.keyer_mode.button_recording = n;
		DigiModes_TxBufferReset();
		UiDriver_TextMsgPutChar(':');
		UiDriver_CreateFunctionButtons(false);
	}
}

static void UiAction_RecordKeyerBtn1()
{
	UiAction_RecordKeyerBtnN(KEYER_BUTTON_1);
}

static void UiAction_RecordKeyerBtn2()
{
	UiAction_RecordKeyerBtnN(KEYER_BUTTON_2);
}

static void UiAction_RecordKeyerBtn3()
{
	UiAction_RecordKeyerBtnN(KEYER_BUTTON_3);
}

static void UiAction_ToggleBufferedTXMode()
{
	ts.buffered_tx = ! ts.buffered_tx;
	UiDriver_FButton_F5Tune();
}

static void UiAction_ToggleSplitModeOrToggleMemMode()
{
	if(!ts.vfo_mem_flag)	 		// update screen if in VFO (not memory) mode
	{
		UiDriver_SetSplitMode(!is_splitmode());
	}
	else	 		// in memory mode
	{
		UiSpectrum_Clear();		// always clear display
		ts.mem_disp = !ts.mem_disp;
		if (ts.mem_disp == 0 )
		{
			UiSpectrum_Init();	// init spectrum scope
		}
	}
}

static void UiAction_ToggleMenuMode()
{
	if(!ts.mem_disp)	 			// allow only if NOT in memory display mode
	{
		if(ts.menu_mode == false)	 	// go into menu mode if NOT already in menu mode and not to halt on startup
		{
			ts.menu_mode = true;
			ts.encoder3state = filter_path_change;
			filter_path_change = false;			// deactivate while in menu mode
			UiDriver_DisplayFilter();
			UiSpectrum_Clear();
			UiDriver_DisplayFButton_F1MenuExit();
			UiDriver_DrawFButtonLabel(2,"PREV",Yellow);
			UiDriver_DrawFButtonLabel(3,"NEXT",Yellow);
			UiDriver_DrawFButtonLabel(4,"DEFLT",Yellow);
			//
			//
			// Grey out adjustments and put encoders in known states
			//
			UiDriver_RefreshEncoderDisplay();

			ts.menu_var = 0;
			//
			UiMenu_RenderMenu(MENU_RENDER_ONLY);	// Draw the menu the first time
			UiMenu_RenderMenu(MENU_PROCESS_VALUE_CHANGE);	// Do update of the first menu item
		}
		else	 	// already in menu mode - we now exit
		{
			ts.menu_mode = false;
			filter_path_change = ts.encoder3state;
			UiDriver_DisplayFilter();
			UiSpectrum_Init();			// init spectrum scope

			// Restore encoder displays to previous modes
			UiDriver_RefreshEncoderDisplay();
			UiDriver_DisplayFilter();	// update bandwidth display
			UiDriver_CreateFunctionButtons(false);
		}
	}
}

static void UiAction_ToggleKeyerMode()
{
	ts.keyer_mode.active = !ts.keyer_mode.active;
	UiDriver_CreateFunctionButtons(false);
}

static void UiAction_BandMinusHold()
{
	if(UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED))	 	// and POWER button pressed-and-held at the same time?
	{
		UiDriver_LcdBlankingStealthSwitch();
	}
	else if(UiDriver_IsButtonPressed(BUTTON_BNDP_PRESSED))	 	// and BAND-UP pressed at the same time?
	{
		if(!ts.menu_mode)	 			// do not do this in menu mode!
		{
			UiAction_ToggleWaterfallScopeDisplay();
		}
	}
}

static void UiAction_BandPlusHold()
{
	if(UiDriver_IsButtonPressed(BUTTON_BNDM_PRESSED))	 	// and BAND-DOWN pressed at the same time?
	{
		if(!ts.menu_mode)	 		// do not do this if in menu mode!
		{
			UiAction_ToggleWaterfallScopeDisplay();
		}
	}
	if(UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED))	 	// and POWER button pressed-and-held at the same time?
	{
		UiDriver_PowerDownCleanup(false); // do not save the configuration
	}
}

static void UiAction_PowerHold()
{
	if(UiDriver_IsButtonPressed(BUTTON_BNDM_PRESSED))	 	// was button BAND- pressed at the same time?
	{
		UiDriver_LcdBlankingStealthSwitch();
	}
	else
	{
		// ONLY the POWER button was pressed
		if(ts.txrx_mode == TRX_MODE_RX)  		// only allow power-off in RX mode
		{
			UiDriver_PowerDownCleanup(true);
		}
	}
}

static void UiAction_StepMinusHold()
{
	if(UiDriver_IsButtonPressed(BUTTON_STEPP_PRESSED))	 	// was button STEP+ pressed at the same time?
	{
		ts.frequency_lock = !ts.frequency_lock;
		// update frequency display
		UiDriver_FrequencyUpdateLOandDisplay(true);
	}
	else
	{
		if(!(ts.freq_step_config & FREQ_STEP_SWAP_BTN))	  // button swap NOT enabled
		{
			UiDriver_PressHoldStep(0);	// decrease step size
		}
		else  		// button swap enabled
		{
			UiDriver_PressHoldStep(1);	// increase step size
		}
	}
}

static void UiAction_MenuSetDefaultValue()
{
	UiMenu_RenderMenu(MENU_PROCESS_VALUE_SETDEFAULT);
}

static void UiAction_StepPlusHold()
{
	if(UiDriver_IsButtonPressed(BUTTON_STEPM_PRESSED))	 	// was button STEP- pressed at the same time?
	{
		ts.frequency_lock = !ts.frequency_lock;
		// update frequency display
		UiDriver_FrequencyUpdateLOandDisplay(true);
	}
	else
	{
		if(!(ts.freq_step_config & FREQ_STEP_SWAP_BTN))  	// button swap NOT enabled
		{
			UiDriver_PressHoldStep(1);	// increase step size
		}
		else  		// button swap enabled
		{
			UiDriver_PressHoldStep(0);	// decrease step size
		}
	}
}
// these maps control the touch regions to function mapping in a specific mode
// this is the normal mode, available when not in menu mode
static const touchaction_descr_t touchactions_normal[] =
{
		{ { 19,60,48,60 }, UiAction_ChangeLowerMeterUp },  // Lower Meter: Meter Toggle
		{ { 10,28,27,31 }, UiAction_ToggleWaterfallScopeDisplay }, // Spectrum Bar Left Part: WaterfallScope Toggle
		{ { 29,33,26,32 }, UiAction_ChangeSpectrumZoomLevelDown }, // Spectrum Bar Middle Part: Decrease Zoom Level
		{ { 52,60,26,32 }, UiAction_ChangeSpectrumZoomLevelUp }, // Spectrum Bar Right Part: Increase Zoom Level
		{ { 43,60,00,04 }, UiAction_ChangeFrequencyToNextKhz }, // Tune button:Set last 3 digits to zero
		{ { 16,24,40,44 }, UiAction_ChangeDemodMode }, // Demod Mode Box: mode switch
		{ { 16,24,45,48 }, UiAction_ChangePowerLevel }, // Power Box: TX Power Increase
		{ { 10,16,44,50 }, UiAction_ChangeAudioSource }, // Audio In Box: Switch Source
		{ { 48,52,35,37 }, UiAction_ChangeBandDownOrUp }, // Left Part Band Display: Band down
		{ { 53,60,35,37 }, UiAction_ChangeBandUpOrDown }, // Right Part Band Display: Band up
		{ { 00,07,21,30 }, Codec_RestartI2S }, // DSP Box: Restart I2S
		{ { 8,60,11,19  }, UiAction_ChangeFrequencyByTouch }, // Scope Draw Area: Tune to Touch
		{ { 0,7,10,13   }, UiAction_ChangeDigitalMode }, // Digital Mode Box: Switch Digi Mode
		{ { 26,35,39,46 }, UiAction_ChangeDynamicTuning }, // Step Box: Dynamic Tuning Toggle
};

// this is the map for menu mode, right now only used for debugging/experimental purposes
static const touchaction_descr_t touchactions_menu[] =
{
		{ { 54,57,55,57 }, UiAction_ChangeDebugInfoDisplay}, // S-Meter db: toggle show tp coordinates
		{ { 46,49,55,57 }, UiAction_ChangeRfModPresence}, // S-Meter 40: toogle rf band mod present
		{ { 50,53,55,57 }, UiAction_ChangeVhfUhfModPresence}, // S-Meter 60: toggle vhf/uhf band mod present
};

static const touchaction_list_descr_t touch_regions[] =
{
		// ATTENTION: the size calculation only works for true arrays, not for pointers!
		{ touchactions_normal, sizeof(touchactions_normal)/sizeof(*touchactions_normal) },
		{ touchactions_menu, sizeof(touchactions_menu)/sizeof(*touchactions_menu) },
};

static void UiDriver_HandleTouchScreen()
{
	if(is_touchscreen_pressed())
	{
		if (ts.show_debug_info)					// show coordinates for coding purposes
		{
			char text[10];
			snprintf(text,10,"%02d%s%02d%s",ts.tp->x," : ",ts.tp->y,"  ");
			UiLcdHy28_PrintText(0,POS_LOADANDDEBUG_Y,text,White,Black,0);
		}

		uint32_t touchaction_idx = ts.menu_mode == true?1:0;

		UiDriver_ProcessTouchActions(&touch_regions[touchaction_idx]);

		ts.tp->state = TP_DATASETS_PROCESSED;							// set statemachine to data fetched
	}
}

static const keyaction_descr_t keyactions_normal[] =
{
		{ TOUCHSCREEN_ACTIVE, 	UiDriver_HandleTouchScreen },
		{ BUTTON_F1_PRESSED, 	UiAction_ToggleMenuMode, 					UiAction_SaveConfigurationToMemory },
		{ BUTTON_F2_PRESSED, 	UiAction_ChangeLowerMeterDownOrSnap, 		UiAction_ChangeLowerMeterUp },
		{ BUTTON_F3_PRESSED, 	UiAction_ToggleSplitModeOrToggleMemMode, 	UiAction_ToggleVfoMem },
		{ BUTTON_F4_PRESSED, 	UiAction_ToggleVfoAB, 						UiAction_CopyVfoAB },
		{ BUTTON_F5_PRESSED, 	UiAction_ToggleTuneMode,					UiAction_ToggleTxDisable },
		{ BUTTON_G1_PRESSED, 	UiAction_ChangeDemodMode,					UiAction_ChangeDemodModeToAlternativeMode },
		{ BUTTON_G2_PRESSED, 	UiAction_ChangeToNextDspMode,				UiAction_ToggleDspEnable },
		{ BUTTON_G3_PRESSED, 	UiAction_ChangePowerLevel,					KEYACTION_NOP },
		{ BUTTON_G4_PRESSED, 	UiAction_ChangeFilterBW,					UiAction_ChangeRxFilterOrFmToneBurst },
		{ BUTTON_M1_PRESSED, 	UiDriver_ChangeEncoderOneMode,				UiAction_ToggleKeyerMode },
		{ BUTTON_M2_PRESSED, 	UiDriver_ChangeEncoderTwoMode,				UiAction_ToggleNoiseblanker },
		{ BUTTON_M3_PRESSED, 	UiDriver_ChangeEncoderThreeMode,			UiAction_ChangeAudioSource },
		{ BUTTON_STEPM_PRESSED, UiAction_ChangeTuningStepDownOrUp,			UiAction_StepMinusHold },
		{ BUTTON_STEPP_PRESSED, UiAction_ChangeTuningStepUpOrDown,			UiAction_StepPlusHold },
		{ BUTTON_BNDM_PRESSED, 	UiAction_ChangeBandDownOrUp,				UiAction_BandMinusHold },
		{ BUTTON_BNDP_PRESSED,  UiAction_ChangeBandUpOrDown,				UiAction_BandPlusHold },
		{ BUTTON_POWER_PRESSED, UiAction_ChangeBacklightBrightness,			UiAction_PowerHold },
};

static const keyaction_descr_t keyactions_menu[] =
{
		{ BUTTON_F2_PRESSED, 	(void(*)())UiMenu_RenderPrevScreen, 		UiMenu_RenderFirstScreen },
		{ BUTTON_F3_PRESSED, 	(void(*)())UiMenu_RenderNextScreen, 		UiMenu_RenderLastScreen },
		{ BUTTON_F4_PRESSED, 	UiAction_MenuSetDefaultValue,				KEYACTION_NOP },
};

static const keyaction_descr_t keyactions_keyer[] =
{
		{ BUTTON_F1_PRESSED, 	UiAction_PlayKeyerBtn1, 		UiAction_RecordKeyerBtn1 },
		{ BUTTON_F2_PRESSED, 	UiAction_PlayKeyerBtn2, 		UiAction_RecordKeyerBtn2 },
		{ BUTTON_F3_PRESSED, 	UiAction_PlayKeyerBtn3, 		UiAction_RecordKeyerBtn3 },
		{ BUTTON_F4_PRESSED, 	KEYACTION_NOP, 		KEYACTION_NOP },
		{ BUTTON_F5_PRESSED, 	KEYACTION_NOP,		UiAction_ToggleBufferedTXMode },
};

static const keyaction_list_descr_t key_sets[] =
{
		// ATTENTION: the size calculation only works for true arrays, not for pointers!
		{ keyactions_normal, sizeof(keyactions_normal)/sizeof(*keyactions_normal) },
		{ keyactions_menu, sizeof(keyactions_menu)/sizeof(*keyactions_menu) },
		{ keyactions_keyer, sizeof(keyactions_keyer)/sizeof(*keyactions_keyer) },
};

static void UiDriver_HandleKeyboard()
{
	if(ks.button_processed)
	{
		UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing
		AudioManagement_KeyBeep();  // make keyboard beep, if enabled

		bool keyIsProcessed = false;
		if (ts.keyer_mode.active == true)
		{
			keyIsProcessed = UiDriver_ProcessKeyActions(&key_sets[2]);
		}
		if (keyIsProcessed == false && ts.menu_mode == true)
		{
			keyIsProcessed = UiDriver_ProcessKeyActions(&key_sets[1]);
		}
		if (keyIsProcessed == false)
		{
			keyIsProcessed = UiDriver_ProcessKeyActions(&key_sets[0]);
		}
		// Reset flag, allow other buttons to be checked
		ks.button_processed = 0;
		ks.debounce_time	= 0;
	}
}


typedef enum {
	SCTimer_ENCODER_KEYS =0, // 10ms
	SCTimer_RTC, // 100 * 10ms
	SCTimer_LODRIFT, // 64 * 10ms
	SCTimer_VOLTAGE, // 8 * 10ms
	SCTimer_SMETER, // 4 * 10ms
	SCTimer_MAIN, // 4 * 10ms
	SCTimer_AGC, // 25 * 10ms
	SCTimer_LEDBLINK, // 64 * 10ms
	SCTimer_NUM
} SysClockTimers;

uint32_t last_sysclock_seen[SCTimer_NUM];


/*
 * Implements a simple timeout timer.
 * Returns true if the current sysclock differs by equal or more than divider cycles.
 * Dividers should be powers of 2 to generate optimal code
 */
bool UiDriver_TimerIsExpired(SysClockTimers sct,uint32_t now, uint32_t divider)
{
	return (last_sysclock_seen[sct] != now/divider);
}

/**
 * @brief Implements a simple timeout timer. Sets the time to now/divider, so it will expire in now+divider cycles
 * Dividers should be powers of 2 to generate optimal code
 */
void UiDriver_TimerRewind(SysClockTimers sct,uint32_t now, uint32_t divider)
{
	last_sysclock_seen[sct] = now/divider;
}

/**
 * @brief Implements a simple timeout timer. If expired the timer is automatically restarted.
 *
 * @param sct the timer data structure
 * @param divider should be powers of 2 to generate optimal code
 * @param now current sysclock value
 *
 * @returns true if the now differs by equal or more than divider cycles.
 *
 */
bool UiDriver_TimerExpireAndRewind(SysClockTimers sct,uint32_t now, uint32_t divider)
{
	bool retval = UiDriver_TimerIsExpired(sct, now, divider);
	if (retval) {
		UiDriver_TimerRewind(sct, now, divider);
	}
	return retval;
}



#ifdef USE_USBHOST
#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_hid_keybd.h"
extern USBH_HandleTypeDef hUsbHostHS;
#endif

void UiDriver_MainHandler()
{

	uint32_t now = ts.sysclock;
	//        HAL_GetTick()/10;

	CatDriver_HandleProtocol();

#ifdef USE_USBHOST
	MX_USB_HOST_Process();

#ifdef USE_USBKEYBOARD
	if(USBH_HID_GetDeviceType(&hUsbHostHS) == HID_KEYBOARD)
	{

		HID_KEYBD_Info_TypeDef *k_pinfo;
		char kbdChar;
		k_pinfo = USBH_HID_GetKeybdInfo(&hUsbHostHS);

		if(k_pinfo != NULL)
		{
			kbdChar = USBH_HID_GetASCIICode(k_pinfo);
			switch(k_pinfo->keys[0])
			{
			case KEY_F1:
				ts.ptt_req = true;
				break;
			case KEY_F2:
				ts.tx_stop_req = true;
				break;
			}
			if (kbdChar != '\0')
			{
				if (is_demod_rtty())
				{
					DigiModes_TxBufferPutChar(kbdChar);
				}
				UiDriver_TextMsgPutChar(kbdChar);
			}
		}
	}
#endif
#endif

	// START CALLED AS OFTEN AS POSSIBLE
#ifdef USE_FREEDV
	if (ts.dmod_mode == DEMOD_DIGI && ts.digital_mode == DigitalMode_FreeDV)
	{
		FreeDv_HandleFreeDv();
	}
#endif // USE_FREEDV

#ifdef alternate_NR
	if ((ts.new_nb == true) && (ads.decimation_rate == 4))
	{

		alternateNR_handle();

	}

#endif


	if (ts.tx_stop_req == true  || ts.ptt_req == true)
	{
		RadioManagement_HandlePttOnOff();
	}
	// END CALLED AS OFTEN AS POSSIBLE

	// BELOW ALL CALLING IS BASED ON SYSCLOCK 10ms clock
	if (UiDriver_TimerExpireAndRewind(SCTimer_ENCODER_KEYS,now,1))
	{
		// 10ms have elapsed.
		// Now process events which should be handled regularly at a rate of 100 Hz
		// Remember to keep this as short as possible since this is executed in addition
		// to all other processing below.
		UiDriver_CheckEncoderOne();
		UiDriver_CheckEncoderTwo();
		UiDriver_CheckEncoderThree();
		UiDriver_CheckFrequencyEncoder();
		UiDriver_KeyboardProcessOldClicks();
		RadioManagement_HandlePttOnOff();
	}

	UiSpectrum_Redraw();

	// Expect the code below to be executed around every 40 - 80ms.
	// The exact time between two calls is unknown and varies with different
	// display options  (waterfall/scope, DSP settings etc.)
	// Nothing with short intervals < 100ms  and/or need for very regular intervals between calls
	// should be placed in here.
	if(UiDriver_TimerIsExpired(SCTimer_MAIN,now,1))            // bail out if it is not time to do this task
	{
		switch(drv_state)
		{
		case STATE_S_METER:
			if (UiDriver_TimerExpireAndRewind(SCTimer_SMETER,now,4))
			{
				UiDriver_HandleSMeter();
			}
			break;
		case STATE_SWR_METER:
			UiDriver_HandleTXMeters();
			break;
		case STATE_HANDLE_POWERSUPPLY:
			Board_HandlePowerDown();

			if (UiDriver_TimerExpireAndRewind(SCTimer_VOLTAGE,now,8))
			{
				if (UiDriver_HandleVoltage())
				{
					UiDriver_DisplayVoltage();
				}

				if (pwmt.undervoltage_detected == true) {
					if (UiDriver_TimerExpireAndRewind(SCTimer_LEDBLINK, now, 64)) {
						Board_GreenLed(LED_STATE_TOGGLE);
					}
				}
				UiDriver_TextMsgDisplay();
			}
			break;
		case STATE_LO_TEMPERATURE:
			if (UiDriver_TimerExpireAndRewind(SCTimer_LODRIFT,now,64))
			{
				UiDriver_HandleLoTemperature();
#if 1
				ProfilingTimedEvent* pe_ptr = profileTimedEventGet(ProfileAudioInterrupt);

				// Percent audio interrupt load  = Num of cycles per audio interrupt  / ((max num of cycles between two interrupts ) / 100 )
				//
				// Num of cycles per audio interrupt = cycles for all counted interrupts / number of interrupts
				// Max num of cycles between two interrupts / 100 = HCLK frequency / Interruptfrequenz -> e.g. 168000000 / 1500 / 100 = 1120
				// FIXME: Need to figure out which clock is being used, 168000000 in mcHF, I40 UI = 168.000.000 or 216.000.000 or something else...

				uint32_t load =  pe_ptr->duration / (pe_ptr->count * (1120));
				profileTimedEventReset(ProfileAudioInterrupt);
				char str[20];
				snprintf(str,20,"L%3u%%",(unsigned int)load);
				if(ts.show_debug_info)
				{
					UiLcdHy28_PrintText(280,POS_LOADANDDEBUG_Y,str,White,Black,5);
				}
#endif
			}
			if (UiDriver_TimerExpireAndRewind(SCTimer_RTC,now,100))
			{
				if (ts.rtc_present)
				{
					RTC_TimeTypeDef sTime;


					MchfRtc_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

					char str[20];
					snprintf(str,20,"%2u:%02u:%02u",sTime.Hours,sTime.Minutes,sTime.Seconds);
					UiLcdHy28_PrintText(0,POS_RTC,str,White,Black,0);
				}
			}
			break;
		case STATE_TASK_CHECK:
			UiDriver_TimeScheduler();
			// Handles live update of Calibrate between TX/RX and volume control
			break;
		case STATE_UPDATE_FREQUENCY:
			/* at this point we handle request for changing the frequency
			 * either from a difference in dial freq or a temp change
			 *  */
			if((df.tune_old != df.tune_new))
			{
				UiDriver_FrequencyUpdateLOandDisplay(false);
				UiDriver_DisplayMemoryLabel();				// this is because a frequency dialing via CAT must be indicated if "CAT in sandbox" is active
			}
			else if (df.temp_factor_changed  || ts.tune_freq != ts.tune_freq_req)
			{
				// this handles the cases where the dial frequency remains the same but the
				// LO tune frequency needs adjustment, e.g. in CW mode  or if temp of LO changes
				RadioManagement_ChangeFrequency(false,df.tune_new/TUNE_MULT, ts.txrx_mode);
			}
			break;
		case STATE_PROCESS_KEYBOARD:
			UiDriver_HandleKeyboard();
			break;
		case STATE_DISPLAY_SAM_CARRIER:
			if (UiDriver_TimerExpireAndRewind(SCTimer_RTC,now,25))
			{
				if(ts.dmod_mode == DEMOD_SAM)
				{
					UiDriver_UpdateLcdFreq(df.tune_old/TUNE_MULT, Yellow, UFM_SECONDARY);
				}
				else if (ts.dmod_mode == DEMOD_CW && cw_decoder_config.snap_enable)
				{
					//UiDriver_UpdateLcdFreq(ads.snap_carrier_freq, Green, UFM_SECONDARY);
				}
		// display AGC box and AGC state
				// we have 5 states -> We can collapse 1 and 2 -> you see this in the box title anyway
				// we use an asterisk to indicate action
				// 1 OFF -> WDSP AGC not active
				// 2 ON + NO_HANG + NO ACTION		no asterisk
				// 3 ON + HANG_ACTION + NO ACTION 	white asterisk
				// 4 ON + ACTION                	green asterisk
				// 5 ON + ACTION + HANG_ACTION  	blue asterisk
				const char* txt = "   ";
				uint16_t AGC_bg_clr = Black;
				uint16_t AGC_fg_clr = Black;
				if(ts.agc_wdsp == 1)
				{
					if(ts.agc_wdsp_hang_action == 1 && ts.agc_wdsp_hang_enable == 1)
					{
						AGC_bg_clr = White;
						AGC_fg_clr = Black;
					}
					else
					{
						AGC_bg_clr = Blue;
						AGC_fg_clr = White;
					}
					if(ts.agc_wdsp_action == 1)
					{
						txt = "AGC";
					}
				}
				UiLcdHy28_PrintTextCentered(POS_DEMOD_MODE_MASK_X - 41,POS_DEMOD_MODE_MASK_Y,POS_DEMOD_MODE_MASK_W-6,txt,AGC_fg_clr,AGC_bg_clr,0);
				// display CW decoder WPM speed
				if(ts.cw_decoder_enable && ts.dmod_mode == DEMOD_CW)
				{
					CwDecoder_WpmDisplayUpdate(false);
				}

			}
			break;
		default:
			break;

		}
		if (drv_state < STATE_MAX)
		{
			// advance to next state
			drv_state++;
		}
		else
		{
			UiDriver_TimerRewind(SCTimer_MAIN,now,1);
			// wrap state to first state
			drv_state = 0;
		}
	}
}

/*
 * This handler creates a software pwm for the LCD backlight. It needs to be called
 * very regular to work properly. Right now it is activated from the audio interrupt
 * at a rate of 1.5khz The rate itself is not too critical,
 * just needs to be high and very regular.
 */
void UiDriver_BacklightDimHandler()
{
	static uchar lcd_dim = 0, lcd_dim_prescale = 0;

	if(!ts.lcd_blanking_flag)       // is LCD *NOT* blanked?
	{
		if(!lcd_dim_prescale)       // Only update dimming PWM counter every fourth time through to reduce frequency below that of audible range
		{
			UiLcdHy28_BacklightEnable(lcd_dim >= ts.lcd_backlight_brightness);   // LCD backlight off or on

			lcd_dim++;
			lcd_dim %= 4;   // limit brightness PWM count to 0-3
		}
		lcd_dim_prescale++;
		lcd_dim_prescale %= 4;  // limit prescale count to 0-3
	}
	else if(!ts.menu_mode)
	{ // LCD is to be blanked - if NOT in menu mode
		UiLcdHy28_BacklightEnable(false);
	}
}
