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

// Common
#include "mchf_board.h"

#include <stdio.h>
#include <stdlib.h>
#include "codec.h"
#include "ui_menu.h"
//
//
#include "ui.h"
// LCD
#include "ui_lcd_hy28.h"
#include "ui_spectrum.h"

// serial EEPROM driver
#include "mchf_hw_i2c2.h"

// Encoders
#include "ui_rotary.h"

// SI570 control
#include "ui_si570.h"
#include "ui_soft_tcxo.h"

// Codec control
#include "codec.h"
#include "softdds.h"

#include "audio_driver.h"
#include "audio_management.h"
#include "ui_driver.h"
//#include "usbh_usr.h"

#include "cat_driver.h"


#include "ui_configuration.h"
#include "cw_gen.h"

static void 	UiDriverPublicsInit();
static void 	UiDriverProcessKeyboard();
static void 	UiDriverPressHoldStep(uchar is_up);
static void 	UiDriverProcessFunctionKeyClick(ulong id);

static void 	UiDriverShowBand(uchar band);
static void 	UiDriverCreateDesktop();
static void 	UiDriverCreateFunctionButtons(bool full_repaint);
static void     UiDriverDeleteSMeter();
static void 	UiDriverCreateSMeter();
static void 	UiDriverDrawSMeter(ushort color);
//
static void 	UiDriverUpdateTopMeterA(uchar val);
static void 	UiDriverUpdateBtmMeter(uchar val, uchar warn);

static void 	UiDriverInitFrequency();
//
static void 	UiDriver_SetHWFiltersForFrequency(ulong freq);
uchar 			UiDriverCheckBand(ulong freq, ushort update);
static void 	UiDriverUpdateLcdFreq(ulong dial_freq,ushort color,ushort mode);
static bool 	UiDriver_IsButtonPressed(ulong button_num);
static void		UiDriverTimeScheduler();				// Also handles audio gain and switching of audio on return from TX back to RX
static void 	UiDriverChangeDemodMode(uchar noskip);
static void 	UiDriverChangeBand(uchar is_up);
static bool 	UiDriverCheckFrequencyEncoder();
static void 	UiDriverCheckEncoderOne();
static void 	UiDriverCheckEncoderTwo();
static void 	UiDriverCheckEncoderThree();
static void 	UiDriverChangeEncoderOneMode(uchar skip);
static void 	UiDriverChangeEncoderTwoMode(uchar skip);
static void 	UiDriverChangeEncoderThreeMode(uchar skip);
static void 	UiDriverChangeSigProc(uchar enabled);
static void 	UiDriverDisplayNotch(uchar enabled);
static void 	UiDriverDisplayBass();
static void 	UiDriverChangeRit(uchar enabled);
static void 	UiDriverChangeDSPMode();
static void 	UiDriverChangeDigitalMode();
static void 	UiDriver_DisplayPowerLevel();
static void 	UiDriverHandleSmeter();
static void 	UiDriverHandleLowerMeter();
static void     UiDriverHandlePowerSupply();
static void 	UiDriverUpdateLoMeter(uchar val,uchar active);
void 			UiDriverCreateTemperatureDisplay(uchar enabled,uchar create);
static void 	UiDriverRefreshTemperatureDisplay(uchar enabled,int temp);
static void 	UiDriverHandleLoTemperature();
static void 	UiDriver_HandlePttOnOff();
static void 	UiDriverInitMainFreqDisplay();

static bool	UiDriver_LoadSavedConfigurationAtStartup();
static bool	UiDriver_TouchscreenCalibration();
//
//
//
// Tuning steps
const ulong tune_steps[T_STEP_MAX_STEPS] = {
T_STEP_1HZ,
T_STEP_10HZ,
T_STEP_100HZ,
T_STEP_1KHZ,
T_STEP_5KHZ,
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
// - Dummy variable		1,		//0, S0, 0dB
		14.1,	//1.41,	//1, S0.5, 3dB
		20,		//2,		//2, S1, 6dB
		28.1,	//2.81,	//3, S1.5, 9dB
		30,		//3,		//4, S2, 12dB
		56.2,	//5.62,	//5, S2.5, 15dB
		79.4,	//7.94,	//6, S3, 18dB
		112.2,	//11.22,	//7, S3.5, 21dB
		158.5,	//15.85,	//8, S4, 24dB
		223.9,	//22.39,	//9, S4.5, 27dB
		316.3,	//31.63,	//10, S5, 30dB
		446.7,	//44.67,	//11, S5.5, 33dB
		631,	//63.10,	//12, S6, 36dB
		891.3,	//89.13,	//13, S6.5, 39dB
		1258.9,	//125.89,	//14, S7, 42dB
		1778.3,	//177.83,	//15, S7.5, 45dB
		2511.9,	//251.19,	//16, S8, 48dB
		3548.1,	//354.81,	//17, S8.5, 51dB
		5011.9,	//501.19,	//18, S9, 54dB
		8912.5,	//891.25,	//19, +5, 59dB
		15848.9,	//1584.89,	//20, +10, 64dB
		28183.8,	//2818.38,	//21, +15, 69dB
		50118.7,	//5011.87,	//22, +20, 74dB
		89125.1,	//8912.51,	//23, +25, 79dB
		158489.3,	//15848.93,	//24, +30, 84dB
		281838.2,	//28183.82,	//25, +35, 89dB
		501187.2,	//50118.72,	//26, +35, 94dB
		891250.9,	//89125.09,	//27, +40, 99dB
		1585893.2,	//158489.32,	//28, +45, 104dB
		2818382.9,	//281838.29,	//29, +50, 109dB
		5011872.3,	//501187.23,	//30, +55, 114dB
		8912509.4,	//891250.94,	//31, +60, 119dB
		15848931.9,	//1584893.19,	//32, +65, 124dB
		28183829.3,	//2818382.93	//33, +70, 129dB
};

__IO BandRegs vfo[VFO_MAX];


// ------------------------------------------------
// Frequency public
__IO DialFrequency 				df;


// ------------------------------------------------
// Keypad state
__IO KeypadState				ks;

// ------------------------------------------------
// Auto mode blinking text
//__IO AutoButtonState			abst;

// ------------------------------------------------
// On screen clock
//__IO ClockState 				cs;

// ------------------------------------------------
// SWR/Power meter
__IO SWRMeter					swrm;

// ------------------------------------------------
// Power supply meter
__IO PowerMeter					pwmt;

// ------------------------------------------------
// LO Tcxo
__IO LoTcxo						lo;

// CAT driver state
__IO CatDriver					kd;

// move to struct ??
__IO ulong 						unmute_delay = 0;

// ------------------------------------------------
// Spectrum display
extern __IO	SpectrumDisplay		sd;


uchar drv_state = 0;

bool filter_path_change = false;
//



// Snap carrier
//__IO   SnapCarrier     sc;



inline uint32_t change_and_limit_uint(volatile uint32_t val, int32_t change, uint32_t min, uint32_t max) {
  if (change  > (val - min)) { val = min; }
  else if (change >  max - val) { val = max; }
  else { val +=change;}
  return val;
}

inline uint32_t change_and_wrap_uint(volatile uint32_t val, int32_t change, uint32_t min, uint32_t max) {
  if (change  > (max - val)) { val = min; }
  else if (change + (int32_t)val <  min) { val = max; }
  else { val +=change; }
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





inline bool is_touchscreen_pressed() {
	return (ts.tp_state > 1 && ts.tp_state != 0xff);	// touchscreen data available
}



bool is_splitmode() {
	return (ts.vfo_mem_mode & VFO_MEM_MODE_SPLIT) != 0;
}

bool is_vfo_b() {
	return (ts.vfo_mem_mode & VFO_MEM_MODE_VFO_B) != 0;
}

inline bool is_dsp_nb() {
	return (ts.dsp_active & DSP_NB_ENABLE) != 0;
}

inline bool is_dsp_nr() {
	return (ts.dsp_active & DSP_NR_ENABLE) != 0;
}

inline bool is_dsp_nr_postagc() {
	return (ts.dsp_active & DSP_NR_POSTAGC_ENABLE) != 0;
}


inline bool is_dsp_notch() {
	return (ts.dsp_active & DSP_NOTCH_ENABLE) != 0;
}

void UiDriverShowDebugText(char* text)
{
UiLcdHy28_PrintText(POS_PWR_NUM_IND_X,POS_PWR_NUM_IND_Y+24,text,White,Black,0);
}

static void UiDriver_ToggleWaterfallScopeDisplay() {
  if(ts.flags1 & FLAGS1_WFALL_SCOPE_TOGGLE)
  {                       // is the waterfall mode active?
    ts.flags1 &=  ~FLAGS1_WFALL_SCOPE_TOGGLE;     // yes, turn it off
    UiSpectrumInitSpectrumDisplay();   // init spectrum scope
  }
  else
  {                       // waterfall mode was turned off
    ts.flags1 |=  FLAGS1_WFALL_SCOPE_TOGGLE;          // turn it on
    UiSpectrumInitSpectrumDisplay();   // init spectrum scope
  }
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiLCDBlankTiming
//* Object              : Do LCD Auto-Blank timing
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiDriver_LcdBlankingStartTimer()
{
    ulong ltemp;

    if(ts.lcd_backlight_blanking & LCD_BLANKING_ENABLE) {   // is LCD blanking enabled?
        ltemp = (ulong)(ts.lcd_backlight_blanking & LCD_BLANKING_TIMEMASK);      // get setting of LCD blanking timing
        ltemp *= 100;       // multiply to convert to deciseconds
        ts.lcd_blanking_time = ltemp + ts.sysclock;     // calculate future time at which LCD is to be turned off
        ts.lcd_blanking_flag = 0;       // clear flag to make LCD turn on
    }
}


static void   UiDriver_LcdBlankingProcessTimer() {
  // Process LCD auto-blanking
  if(ts.lcd_backlight_blanking & LCD_BLANKING_ENABLE)  {   // is LCD auto-blanking enabled?
    if(ts.sysclock > ts.lcd_blanking_time)  {   // has the time expired and the LCD should be blanked?
      ts.lcd_blanking_flag = 1;             // yes - blank the LCD
    } else {                                    // time not expired
      ts.lcd_blanking_flag = 0;             // un-blank the LCD
    }
  } else {                              // auto-blanking NOT enabled
    ts.lcd_blanking_flag = 0;               // always un-blank the LCD in this case
  }
}

static void UiDriver_LcdBlankingStealthSwitch() {
  if(ts.lcd_backlight_blanking & LCD_BLANKING_ENABLE)         // Yes - is MSB set, indicating "stealth" (backlight timed-off) mode?
    ts.lcd_backlight_blanking &= ~LCD_BLANKING_ENABLE;      // yes - clear that bit, turning off "stealth" mode
  else
  {
    if(ts.lcd_backlight_blanking & LCD_BLANKING_TIMEMASK)    // bit NOT set AND the timing set to NON-zero?
      ts.lcd_backlight_blanking |= LCD_BLANKING_ENABLE;       // no - turn on MSB to activate "stealth" mode
  }
}


void UiDriver_HandleSwitchToNextDspMode()
{
	if(ts.dmod_mode != DEMOD_FM)	{ // allow selection/change of DSP only if NOT in FM
		//
		// I think we should alter this to use a counter
		// What do we want to switch here:
		// NR ON/OFF		ts.dsp_active |= DSP_NR_ENABLE;	 // 	ts.dsp_active &= ~DSP_NR_ENABLE;
		// NOTCH ON/OFF		ts.dsp_active |= DSP_NOTCH_ENABLE; // 	ts.dsp_active &= ~DSP_NOTCH_ENABLE;
		// Manual Notch		ts.notch_enabled = 1; // ts.notch_enabled = 0;
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
		// prevent NR AND NOTCH, when in AM and filter-bandwidth > 4k8 (= decimation rate equals 2 --> high CPU load)
		if (ts.dsp_mode == DSP_SWITCH_NR_AND_NOTCH && (ts.dmod_mode == DEMOD_AM) && (FilterPathInfo[ts.filter_path].id > AUDIO_4P8KHZ)) ts.dsp_mode++;

		// display all as inactive (and then activate the right one, see below)
		UiDriverChangeRfGain(0);
	    // DSP/Noise Blanker
	    UiDriverChangeSigProc(0);

		switch (ts.dsp_mode) {

		case DSP_SWITCH_OFF: // switch off everything
			ts.dsp_active &= ~DSP_NR_ENABLE;
			ts.dsp_active &= ~DSP_NOTCH_ENABLE;
			ts.notch_enabled = 0;
			ts.peak_enabled = 0;				//off
			ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;
			UiDriverChangeRfGain(1);
			UiDriverDisplayNotch(0); // display
//			UiDriverDisplayBass();
			break;
		case DSP_SWITCH_NR:
			ts.dsp_active |= DSP_NR_ENABLE; 	//on
			ts.dsp_active &= ~DSP_NOTCH_ENABLE; //off
			ts.notch_enabled = 0;				//off
			ts.peak_enabled = 0;				//off
		    UiDriverChangeSigProc(1);
		    ts.enc_two_mode = ENC_TWO_MODE_SIG_PROC;
			break;
		case DSP_SWITCH_NOTCH:
			ts.dsp_active &= ~DSP_NR_ENABLE;	//off
			ts.dsp_active |= DSP_NOTCH_ENABLE;	//on
			ts.notch_enabled = 0;				//off
			ts.peak_enabled = 0;				//off
			break;
		case DSP_SWITCH_NR_AND_NOTCH:
			ts.dsp_active |= DSP_NR_ENABLE; 	//on
			ts.dsp_active |= DSP_NOTCH_ENABLE;	//on
			ts.notch_enabled = 0;				//off
			ts.peak_enabled = 0;				//off
			break;
		case DSP_SWITCH_NOTCH_MANUAL:
			ts.dsp_active &= ~DSP_NR_ENABLE;	//off
			ts.dsp_active &= ~DSP_NOTCH_ENABLE; //off
			ts.notch_enabled = 1;				//on
			ts.peak_enabled = 0;				//off
			ts.enc_two_mode = ENC_TWO_MODE_NOTCH_F;
		    UiDriverDisplayNotch(1);
			break;
		case DSP_SWITCH_PEAK_FILTER:
			ts.dsp_active &= ~DSP_NR_ENABLE;	//off
			ts.dsp_active &= ~DSP_NOTCH_ENABLE; //off
			ts.notch_enabled = 0;				//off
			ts.peak_enabled = 1;				//on
			ts.enc_two_mode = ENC_TWO_MODE_PEAK_F;
		    UiDriverDisplayNotch(1);
			break;
		case DSP_SWITCH_BASS:
			break;
		case DSP_SWITCH_TREBLE:
			break;
		}
/*
		if (ts.notch_enabled) {
			    ts.notch_enabled = 0; // switch off notch filter
			    UiDriverChangeRfGain(1);
			    // DSP/Noise Blanker
			    UiDriverChangeSigProc(0);
			    ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;
			    UiDriverDisplayNotch(0); // display
			}
			else {
			    ts.notch_enabled = 1;
			    // RF gain
			    UiDriverChangeRfGain(0);
			    // DSP/Noise Blanker
			    UiDriverChangeSigProc(0);
			    // notch display
			    UiDriverDisplayNotch(1);
			    ts.enc_two_mode = ENC_TWO_MODE_NOTCH_F;
			}
*/


/*		if((!(is_dsp_nr())) && (!(is_dsp_notch())))	// both NR and notch are inactive
		{
		ts.dsp_active |= DSP_NR_ENABLE;					// turn on NR
		}
		else if((is_dsp_nr()) && (!(is_dsp_notch()))) {	// NR active, notch inactive
			if(ts.dmod_mode != DEMOD_CW)	{	// NOT in CW mode
				ts.dsp_active |= DSP_NOTCH_ENABLE;									// turn on notch
				ts.dsp_active &= ~DSP_NR_ENABLE;								// turn off NR
			}
			else	{	// CW mode - do not select notches, skip directly to "off"
				ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE);	// turn off NR and notch
			}
		}
		else if((!(is_dsp_nr())) && (is_dsp_notch()))	//	NR inactive, notch active
			if((ts.dmod_mode == DEMOD_AM) && (FilterPathInfo[ts.filter_path].id > AUDIO_4P8KHZ))		// was it AM with a filter > 4k8 selected?
				ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE);			// it was AM + wide - turn off NR and notch
			else
			{
			ts.dsp_active |= DSP_NR_ENABLE;				// no - turn on NR
			}
		//
		else	{
			ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE);								// turn off NR and notch
		}
		*/

		//
		ts.dsp_active_toggle = ts.dsp_active;	// save update in "toggle" variable
		//
		ts.reset_dsp_nr = 1;				// reset DSP NR coefficients
		audio_driver_set_rx_audio_filter();		// update DSP/filter settings
		ts.reset_dsp_nr = 0;
		UiDriverChangeDSPMode();			// update on-screen display
		//
		// Update DSP/NB/RFG control display
		//
		// put all displays here??
		//
/*		if(ts.enc_two_mode == ENC_TWO_MODE_RF_GAIN)
			UiDriverChangeSigProc(0);
		else
			UiDriverChangeSigProc(1); */
	}
}

/*
 * @brief Function will update LO and Display Digits, it will never change LO if not necessary
 *
 * @param full_update set to true in order to have the full display digits being updated
 *
 */
void UiDriver_FrequencyUpdateLOandDisplay(bool full_update) {
    if (full_update) {
        ts.refresh_freq_disp = 1;           // update ALL digits
    }
    if(is_splitmode())
    {                       // SPLIT mode
        UiDriverUpdateFrequency(false,UFM_SMALL_TX);
        UiDriverUpdateFrequency(false,UFM_SMALL_RX);
    } else {
        UiDriverUpdateFrequency(false,UFM_AUTOMATIC);
    }
    ts.refresh_freq_disp = 0;           // update ALL digits
}



void UiDriver_HandleTouchScreen()
{
	if (ts.show_tp_coordinates)					// show coordinates for coding purposes
	    {
	    char text[10];
	    sprintf(text,"%02d%s%02d%s",ts.tp_x," : ",ts.tp_y,"  ");
	    UiLcdHy28_PrintText(POS_PWR_NUM_IND_X,POS_PWR_NUM_IND_Y,text,White,Black,0);
	    }
	if(!ts.menu_mode)						// normal operational screen
	{
		if(check_tp_coordinates(19,60,44,60))			// S-Meter
		    {
			incr_wrap_uint8(&ts.tx_meter_mode,0,METER_MAX-1);
			UiDriverDeleteSMeter();
		  UiDriverCreateSMeter();	// redraw meter
		    }
		if(check_tp_coordinates(30,60,27,31))			// wf/scope bar right part
		    {
		    UiDriver_ToggleWaterfallScopeDisplay();
		    }
		if(check_tp_coordinates(10,28,27,31))			// wf/scope bar left part
		    {
		    sd.magnify = !sd.magnify;
		    ts.menu_var_changed = 1;
		    UiSpectrumInitSpectrumDisplay();		// init spectrum scope
		    }
		if(check_tp_coordinates(43,60,00,04))			// TUNE button
		    {
		    df.tune_new = floor(df.tune_new / (TUNE_MULT*1000)) * (TUNE_MULT*1000);	// set last three digits to "0"
		    UiDriver_FrequencyUpdateLOandDisplay(true);
		    }
		if(check_tp_coordinates(8,60,11,19) && !ts.frequency_lock)// wf/scope frequency dial lower half spectrum/scope
		{
			int step = 2000;				// adjust to 500Hz
			if(ts.dmod_mode == DEMOD_AM)
				step = 20000;				// adjust to 5KHz
			uchar line = 29;				// x-position of rx frequency in middle position
			if(!sd.magnify)					// x-position differs in translated modes if not magnified
			{
				switch(ts.iq_freq_mode){
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
			uint tunediff = ((1000)/(sd.magnify+1))*(ts.tp_x-line)*TUNE_MULT;
			df.tune_new = lround((df.tune_new + tunediff)/step) * step;
			UiDriver_FrequencyUpdateLOandDisplay(true);
		}
		if(check_tp_coordinates(0,7,31,33))			// toggle digital modes
		{
		    incr_wrap_uint8(&ts.digital_mode,0,7);
			UiDriverChangeDigitalMode();
		}
		if(check_tp_coordinates(26,35,39,46))			// dynamic tuning activation
		{
			if (ts.dynamic_tuning_active)			// is it off??
			{
				ts.dynamic_tuning_active = false;	// then turn it on
			}
			else
			{
				ts.dynamic_tuning_active = true;	// if already on, turn it off
			}
			UiDriverShowStep(df.selected_idx);
		}
	}
	else								// menu screen functions
	{
		if(check_tp_coordinates(54,57,55,57))			// enable tp coordinates show S-meter "dB"
		{
			ts.show_tp_coordinates = !ts.show_tp_coordinates;
			UiLcdHy28_PrintText(POS_PWR_NUM_IND_X,POS_PWR_NUM_IND_Y,ts.show_tp_coordinates?"enabled":"       ",Green,Black,0);
		}
		if(check_tp_coordinates(46,49,55,57)) {			// rf bands mod S-meter "40"
		    ts.rfmod_present = !ts.rfmod_present;
		}
		if(check_tp_coordinates(50,53,55,57)) {			// vhf/uhf bands mod S-meter "60"
		    ts.vhfuhfmod_present = !ts.vhfuhfmod_present;
		}
		if(ts.menu_mode) {					// refresh menu
		    UiMenu_RenderMenu(MENU_RENDER_ONLY);
		}
	}
	ts.tp_state = 0xff;							// set statemachine to data fetched
}


// TODO: RF Management, move out
/** @brief API Function, implements application logic for changing the power level including display updates
 * @param power_level The requested power level (as PA_LEVEL constants)
 * @returns true if there was indeed a power level change
 */
bool Codec_PrepareTx(bool rx_muted, uint8_t txrx_mode) {
    if(ts.dmod_mode != DEMOD_CW)    {               // are we in a voice mode?
        if(ts.tx_audio_source != TX_AUDIO_MIC)  {   // yes - are we in LINE IN mode?
            Codec_Line_Gain_Adj(0); // yes - momentarily mute LINE IN audio if in LINE IN mode until we have switched to TX
        }
        else    {   // we are in MIC IN mode
            Codec_Line_Gain_Adj(0);         // momentarily mute MIC IN audio until we switch modes because we will blast any connected LINE IN source until we switch
            ts.tx_mic_gain_mult = 0;        // momentarily set the mic gain to zero while we go to TX
            Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0016);    // Mute the microphone with the CODEC (this does so without a CLICK)
        }
        //
        if((ts.iq_freq_mode) && (!rx_muted))    {   // Is translate mode active and we have NOT already muted the audio?
            Codec_Volume(0,txrx_mode);    // yes - mute the audio codec to suppress an approx. 6 kHz chirp when going in to TX mode
            rx_muted = 1;       // indicate that we've muted the audio so we don't do this every time through
        }
        //
        non_os_delay();     // pause an instant because the codec chip has its own delay before tasks complete!
    }
    return rx_muted;
}


bool RadioManagement_PowerLevelChange(uint8_t power_level) {
    bool retval = false;

    if(ts.dmod_mode == DEMOD_AM)    {           // in AM mode?
        if(power_level >= PA_LEVEL_MAX_ENTRY) {   // yes, power over 2 watts?
            power_level = PA_LEVEL_2W;  // force to 2 watt mode when we "roll over"
        }
    }
    else    {   // other modes, do not limit max power
        if(power_level >= PA_LEVEL_MAX_ENTRY) {
            power_level = PA_LEVEL_FULL;
        }
    }

    if (power_level != ts.power_level) {
        retval = true;
        ts.power_level = power_level;
        if(ts.tune && !ts.iq_freq_mode) {       // recalculate sidetone gain only if transmitting/tune mode
            Codec_SidetoneSetgain(ts.txrx_mode);
        }
    }
    return retval;
}

void UiDriver_HandlePowerLevelChange(uint8_t power_level) {
    //
    if (RadioManagement_PowerLevelChange(power_level)) {
        UiDriver_DisplayPowerLevel();
        if (ts.menu_mode) {
            UiMenu_RenderMenu(MENU_RENDER_ONLY);
        }
    }
}


void AudioManagement_SetSidetoneForDemodMode(uint16_t dmod_mode) {
    if(dmod_mode == DEMOD_CW)   {   // DDS reset to proper sidetone freq. if CW mode
        softdds_setfreq((float)ts.sidetone_freq,ts.samp_rate,0);
    } else {
        softdds_setfreq(0.0,ts.samp_rate,0);
    }
}

/**
 * @brief API Function, implements application logic for changing the power level including display updates
 *
 *
 * @param power_level The requested power level (as PA_LEVEL constants)
 */
bool RadioManagement_Tune(bool tune) {
    bool retval = tune;
    if(ts.tx_disable == false &&  ((ts.dmod_mode == DEMOD_CW) || (ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB))) {
        if(tune)
        {
            if(ts.tune_power_level != PA_LEVEL_MAX_ENTRY)
            {
                ts.power_temp = ts.power_level;             //store tx level and set tune level
                ts.power_level = ts.tune_power_level;
            }
            if(ts.dmod_mode == DEMOD_CW) {
                softdds_setfreq(CW_SIDETONE_FREQ_DEFAULT,ts.samp_rate,0);
            } else {
                softdds_setfreq(SSB_TUNE_FREQ, ts.samp_rate,0);     // generate tone for setting TX IQ phase
            }

            // DDS on
            RadioManagement_SwitchTXRX(TRX_MODE_TX);                // tune ON
            retval = (ts.txrx_mode == TRX_MODE_TX);
        }
        else
        {
            if(ts.tune_power_level != PA_LEVEL_MAX_ENTRY)
            {
                ts.power_level = ts.power_temp;                 // restore tx level
                //                  UiDriver_DisplayPowerLevel();
            }

            AudioManagement_SetSidetoneForDemodMode(ts.dmod_mode);

            RadioManagement_SwitchTXRX(TRX_MODE_RX);                // tune OFF
            retval = false; // no longer tuning
        }
    } else {
        retval = false;    // no TUNE mode in AM or FM or with disabled TX!
    }
    return retval;
}

uint32_t RadioManagement_Dial2TuneFrequency(const uint32_t dial_freq, uint8_t txrx_mode) {
    uint32_t tune_freq = dial_freq;

    //
    // Do "Icom" style frequency offset of the LO if in "CW OFFSET" mode.  (Display freq. is also offset!)
    //
    if(ts.dmod_mode == DEMOD_CW)    {       // In CW mode?
        switch(ts.cw_offset_mode) {
        case CW_OFFSET_USB_SHIFT:    // Yes - USB?
            tune_freq -= ts.sidetone_freq;
            // lower LO by sidetone amount
            break;
        case CW_OFFSET_LSB_SHIFT:   // LSB?
            tune_freq += ts.sidetone_freq;
            // raise LO by sidetone amount
            break;
        case CW_OFFSET_AUTO_SHIFT:  // Auto mode?  Check flag
            if(ts.cw_lsb)
                tune_freq += ts.sidetone_freq;          // it was LSB - raise by sidetone amount
            else
                tune_freq -= ts.sidetone_freq;          // it was USB - lower by sidetone amount
        }
    }
    // Offset dial frequency if the RX/TX frequency translation is active and we are not transmitting in CW mode

    if(!((ts.dmod_mode == DEMOD_CW) && (txrx_mode == TRX_MODE_TX)))  {
        tune_freq += audio_driver_xlate_freq();        // magnitude of shift is quadrupled at actual Si570 operating frequency
    }

    // Extra tuning actions
    if(txrx_mode == TRX_MODE_RX)        {
        tune_freq += (ts.rit_value*20); // Add RIT on receive
    }

    return tune_freq*TUNE_MULT;
}



void RadioManagement_EnablePABias() {
    uint32_t   calc_var;

    if((ts.pa_cw_bias) && (ts.dmod_mode == DEMOD_CW))   {   // is CW PA bias non-zero AND are we in CW mode?
        calc_var = ts.pa_cw_bias;       // use special CW-mode bias setting
    } else {
        calc_var = ts.pa_bias;      // use "default" bias setting
    }
    calc_var = calc_var *2 + BIAS_OFFSET;

    if(calc_var > 255) {
        calc_var = 255;
    }
    // Set DAC Channel 1 DHR12L register
    DAC_SetChannel2Data(DAC_Align_8b_R,calc_var);       // set PA bias
}





bool RadioManagement_ChangeFrequency(bool force_update, uint32_t dial_freq,uint8_t txrx_mode) {
    // everything else uses main VFO frequency
     uint32_t    tune_freq;
     bool lo_change_pending = false;

     // Calculate actual tune frequency
     tune_freq = RadioManagement_Dial2TuneFrequency(dial_freq, txrx_mode);

     if((ts.tune_freq != tune_freq) || (ts.refresh_freq_disp) || df.temp_factor_changed || force_update )  // did the frequency NOT change and display refresh NOT requested??
     {
         if(ui_si570_set_frequency(tune_freq,ts.freq_cal,df.temp_factor, 1) == SI570_LARGE_STEP) {   // did the tuning require that a large tuning step occur?
             if(ts.sysclock > RX_MUTE_START_DELAY)   {   // has system start-up completed?
                 ads.agc_holder = ads.agc_val;   // grab current AGC value as synthesizer "click" can momentarily desense radio as we tune
                 ts.rx_muting = 1;               // yes - mute audio output
                 ts.dsp_inhibit_mute = ts.dsp_inhibit;       // get current status of DSP muting and save for later restoration
                 ts.dsp_inhibit = 1;             // disable DSP during tuning to avoid disruption
                 ts.rx_blanking_time = ts.sysclock + (is_dsp_nr()? TUNING_LARGE_STEP_MUTING_TIME_DSP_ON : TUNING_LARGE_STEP_MUTING_TIME_DSP_OFF);    // yes - schedule un-muting of audio when DSP is on
             }
         }

         if(ts.sysclock-ts.last_tuning > 2 || ts.last_tuning == 0)   { // prevention for SI570 crash due too fast frequency changes
             // Set frequency
             ts.last_lo_result = ui_si570_set_frequency(tune_freq,ts.freq_cal,df.temp_factor, 0);
             df.temp_factor_changed = false;
             ts.last_tuning = ts.sysclock;
             ts.tune_freq = tune_freq;        // frequency change required - update change detector
             // Save current freq
             df.tune_old = dial_freq*TUNE_MULT;
             if (ts.last_lo_result == SI570_OK || ts.last_lo_result == SI570_TUNE_LIMITED) {
                 ts.tx_disable &= ~TX_DISABLE_OUTOFRANGE;
             } else {
                 ts.tx_disable |= TX_DISABLE_OUTOFRANGE;
             }

             UiDriver_SetHWFiltersForFrequency(ts.tune_freq/TUNE_MULT);  // check the filter status with the new frequency update
             // Inform Spectrum Display code that a frequency change has happened
             sd.dial_moved = 1;
         } else {
             lo_change_pending = true;
         }

     }

     // successfully executed the change
     return lo_change_pending == false;
}



Si570_ResultCodes RadioManagement_ValidateFrequencyForTX(uint32_t dial_freq) {
     // we check with the si570 code if the frequency is tunable, we do not tune to it.
     return ui_si570_set_frequency(RadioManagement_Dial2TuneFrequency(dial_freq, TRX_MODE_TX),ts.freq_cal,df.temp_factor, 1);
}



void RadioManagement_UpdateFrequencyFast(uint8_t txrx_mode)
{
	// Calculate actual tune frequency
    RadioManagement_ChangeFrequency(false,df.tune_new/TUNE_MULT, txrx_mode);
}



void RadioManagement_SwitchTXRX(uint8_t txrx_mode)
{

    static bool rx_muted = 0;
    uint32_t tune_new;
    bool tx_ok = false;

    if(is_splitmode())  {               // is SPLIT mode active?
        uint8_t vfo_tx,vfo_rx;
        if (is_vfo_b()) {
            vfo_rx = VFO_B;
            vfo_tx = VFO_A;
        } else {
            vfo_rx = VFO_A;
            vfo_tx = VFO_B;
        }
        if(txrx_mode == TRX_MODE_TX) {   // are we in TX mode?
            if(ts.txrx_mode == TRX_MODE_RX) {                       // did we want to enter TX mode?
                vfo[vfo_rx].band[ts.band].dial_value = df.tune_new; // yes - save current RX frequency in VFO location (B)
            }
            tune_new = vfo[vfo_tx].band[ts.band].dial_value;    // load with VFO-A frequency
        }
        else {                  // we are in RX mode
            tune_new = vfo[vfo_rx].band[ts.band].dial_value;    // load with VFO-B frequency
        }
    } else {
        // we just take the current one if not in split mode
        tune_new = df.tune_new;
    }

    if(txrx_mode == TRX_MODE_TX)
    {

        // FIXME: Not very robust code, make sure Validate always returns TUNE_IMPOSSIBLE in case of issues
        tx_ok = RadioManagement_ValidateFrequencyForTX(tune_new/TUNE_MULT) != SI570_TUNE_IMPOSSIBLE;

        if (tx_ok) {
            //
            // Below, in VOICE modes we mute the audio BEFORE we activate the PTT.  This is necessary since U3 is switched the instant that we do so,
            // rerouting audio paths and causing all sorts of disruption including CLICKs and squeaks.
            // We restore TX audio levels in the function "Codec_RX_TX()" according to operating mode
            //
            ts.tx_audio_muting_flag = 1; // let the audio being muted initially
            ts.dsp_inhibit = 1;                             // disable DSP when going into TX mode

            // FIXME: UI Level Interaction in RadioManagement
            UiDriver_HandlePowerLevelChange(ts.power_level);
            // make sure that power level and mode fit together

            rx_muted = Codec_PrepareTx(rx_muted, txrx_mode);

            PTT_CNTR_PIO->BSRRL     = PTT_CNTR;     // TX on and switch CODEC audio paths
            RED_LED_PIO->BSRRL      = RED_LED;      // Red led on

            AudioManagement_SetSidetoneForDemodMode(ts.dmod_mode);
            RadioManagement_EnablePABias();
        }
    }

    // only switch mode if tx was permitted or rx was requested
    if (txrx_mode == TRX_MODE_RX || tx_ok == true) {
        df.tune_new = tune_new;
        RadioManagement_UpdateFrequencyFast(txrx_mode);
    }

    if (txrx_mode == TRX_MODE_RX || tx_ok == false) {
        PTT_CNTR_PIO->BSRRH     = PTT_CNTR;     // TX off
        RED_LED_PIO->BSRRH      = RED_LED;      // Red led off
        rx_muted = 0;       // clear flag to indicate that we've muted the audio
    }


    if (tx_ok == true || (txrx_mode == TRX_MODE_RX  && ts.txrx_mode != TRX_MODE_RX)) {
        // Switch codec mode
        Codec_RX_TX(txrx_mode);
        ts.txrx_mode = txrx_mode;
    }

}






void UiDriver_HandleBandButtons(uint16_t button) {
	uint8_t normal,swapped;
	bool btemp;
	if (button == BUTTON_BNDM) {
		normal = 0;
		swapped = 1;
	} else {
		normal = 1;
		swapped = 0;
	}

	btemp = ads.af_disabled;
	ads.af_disabled = 0;
	//
	ts.dsp_timed_mute = 1;		// disable DSP when changing bands
	ts.dsp_inhibit = 1;
	ts.dsp_inhibit_timing = ts.sysclock + DSP_BAND_CHANGE_DELAY;	// set time to re-enable DSP
	//
	if(ts.flags1 & FLAGS1_SWAP_BAND_BTN)		// band up/down button swapped?
		UiDriverChangeBand(swapped);	// yes - go up
	else
		UiDriverChangeBand(normal);	// not swapped, go down
	//
	UiInitRxParms();	// re-init because mode/filter may have changed

	ads.af_disabled =  btemp;

}


static void UiDriverFButton_F1MenuExit()
{
    char* label;
    uint32_t color;
    if(!ts.menu_var_changed) {
      if (ts.menu_mode) {
        label = "EXIT";
        color = Yellow;
      } else {
        label = "MENU";
        color = White;
      }
    } else {
        label = ts.menu_mode?"EXIT *":"MENU *";
        color = Orange;
    }
    UiDriverFButtonLabel(1,label,color);
}


//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_init()
{
	// Driver publics init
	UiDriverPublicsInit();

	// Init frequency publics
	UiDriverInitFrequency();

	// Load stored data from eeprom or calibrate touchscreen
	if (UiDriver_LoadSavedConfigurationAtStartup() == false && UiDriver_TouchscreenCalibration() == false) {
	  UiDriver_KeyTestScreen();
	}

	AudioManagement_CalcTxCompLevel();		// calculate current settings for TX speech compressor

	df.tune_new = vfo[is_vfo_b()?VFO_B:VFO_A].band[ts.band].dial_value;		// init "tuning dial" frequency based on restored settings
	df.tune_old = df.tune_new;

	UiCWSidebandMode();			// determine CW sideband mode from the restored frequency

	AudioManagement_CalcRxIqGainAdj();		// Init RX IQ gain
	AudioFilter_InitRxHilbertFIR();
	AudioFilter_InitTxHilbertFIR();
	AudioManagement_CalcTxIqGainAdj();		// Init TX IQ gain

	sd.display_offset = INIT_SPEC_AGC_LEVEL;		// initialize setting for display offset/AGC

	// Temp sensor setup
	lo.sensor_absent = ui_si570_init_temp_sensor();

	// Read SI570 settings
	lo.lo_error = 0 != ui_si570_get_configuration();



	AudioManagement_SetSidetoneForDemodMode(ts.dmod_mode);

	// Update codec volume
	//  0 - 16: via codec command
	// 17 - 30: soft gain after decoder
	Codec_Volume((ts.rx_gain[RX_AUDIO_SPKR].value*8),ts.txrx_mode);		// This is only approximate - it will be properly set later

	// Set TX power factor
	UiDriverSetBandPowerFactor(ts.band);

	// Reset inter driver requests flag
	ts.LcdRefreshReq	= 0;
	ts.new_band 		= ts.band;
	df.step_new 		= df.tuning_step;

	// Extra HW init
	mchf_board_post_init();

    // Create desktop screen
    UiDriverCreateDesktop();

	UiDriver_LcdBlankingStartTimer();			// init timing for LCD blanking
	ts.lcd_blanking_time = ts.sysclock + LCD_STARTUP_BLANKING_TIME;
}
/*
 * @brief enables/disables tune mode. Checks if tuning can be enabled based on frequency.
 * I.e. it is not possible to tune on an unsupported frequency
 * @param tune  true if tune is to be enabled
 * @returns true if has been enabled, false if tune is disabled now
 */

//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_thread
//* Object              : non urgent, time taking operations
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_thread()
{
  if(ts.flags1 & FLAGS1_WFALL_SCOPE_TOGGLE) {	// is waterfall mode enabled?
    UiSpectrumReDrawWaterfall();	// yes - call waterfall update instead
  } else {
    UiSpectrumReDrawScopeDisplay();	// Spectrum Display enabled - do that!
  }
  if(ts.thread_timer == 0)			// bail out if it is not time to do this task
  {
    ts.thread_timer = 1;		// reset flag to schedule next occurrence

    switch(drv_state)
    {
    case STATE_S_METER:
      if(!ts.boot_halt_flag) { UiDriverHandleSmeter(); }
      break;
    case STATE_SWR_METER:
      if(!ts.boot_halt_flag) { UiDriverHandleLowerMeter(); }
      break;
    case STATE_HANDLE_POWERSUPPLY:
      UiDriverHandlePowerSupply();
      break;
    case STATE_LO_TEMPERATURE:
      if(!ts.boot_halt_flag) { UiDriverHandleLoTemperature(); }
      break;
    case STATE_TASK_CHECK:
      UiDriverTimeScheduler();
      // Handles live update of Calibrate between TX/RX and volume control
      break;
    case STATE_CHECK_ENC_ONE:
      if(!ts.boot_halt_flag) { UiDriverCheckEncoderOne(); }
      break;
    case STATE_CHECK_ENC_TWO:
      if(!ts.boot_halt_flag) { UiDriverCheckEncoderTwo(); }
      break;
    case STATE_CHECK_ENC_THREE:
      if(!ts.boot_halt_flag) { UiDriverCheckEncoderThree(); }
      break;
    case STATE_UPDATE_FREQUENCY:
        if(!ts.boot_halt_flag) {
            UiDriverCheckFrequencyEncoder();

            /* at this point we handle request for changing the frequency
             * either from a difference in dial freq or a temp change
             *  */
            if((df.tune_new != df.tune_old)) {
                UiDriver_FrequencyUpdateLOandDisplay(false);
            } else if (df.temp_factor_changed) {
                RadioManagement_UpdateFrequencyFast(ts.txrx_mode);
            }
        }
      break;
    case STATE_PROCESS_KEYBOARD:
      UiDriverProcessKeyboard();
      break;
    case STATE_SWITCH_OFF_PTT:
      if(!ts.boot_halt_flag) { UiDriver_HandlePttOnOff(); }
      break;
    default:
      drv_state = 0;
      return;
    }
    drv_state++;
  }
}

/*
 * @brief Set the PA bias according to mode
 */
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverPublicsInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//*----------------------------------------------------------------------------
static void UiDriverPublicsInit()
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
	swrm.skip 				= 0;

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
	  for (idx = 0; idx < COUPLING_MAX; idx++) {
	    swrm.coupling_calc[idx]    = SWR_COUPLING_DEFAULT;
	  }
	}
	swrm.pwr_meter_disp		= 0;	// Display of numerical FWD/REV power metering off by default
	swrm.pwr_meter_was_disp = 0;	// Used to indicate if FWD/REV numerical power metering WAS displayed

	// Power supply meter
	pwmt.skip 				= 0;
	pwmt.p_curr				= 0;
	pwmt.pwr_aver 			= 0;

	// LO tcxo
	lo.skip					= 0;
	lo.comp					= 0;
	lo.last                 = -100;
}

// check if touched point is within rectange of valid action
inline bool check_tp_coordinates(uint8_t x_left, uint8_t x_right, uint8_t y_down, uint8_t y_up)
{
	return (ts.tp_x <= x_right && ts.tp_x >= x_left && ts.tp_y >= y_down && ts.tp_y <= y_up);
}

void UiDriverFButtonLabel(uint8_t button_num, const char* label, uint32_t label_color) {
  UiLcdHy28_PrintTextCentered(POS_BOTTOM_BAR_F1_X + (button_num - 1)*64, POS_BOTTOM_BAR_F1_Y, 56, label,
                      label_color, Black, 0);
}


void UiDriverEncoderDisplay(const uint8_t column, const uint8_t row, const char *label, bool enabled,
                            char temp[5], uint32_t color) {

  uint32_t label_color = enabled?Black:Grey1;

  UiLcdHy28_DrawEmptyRect(POS_AG_IND_X + 56 * column, POS_AG_IND_Y + row * 16, 13, 53, Grey);
  UiLcdHy28_PrintText((POS_AG_IND_X + 1 + 56 * column), (POS_AG_IND_Y + 1 + row * 16), label,
                      label_color, Grey, 0);
  UiLcdHy28_PrintTextRight((POS_AG_IND_X + 52 + 56 * column), (POS_AG_IND_Y + 1 + row * 16), temp,
                           color, Black, 0);
}

void UiDriverEncoderDisplaySimple(const uint8_t column, const uint8_t row, const char *label, bool enabled,
                            uint32_t value) {

	char temp[5];
	uint32_t color = enabled?White:Grey;

	snprintf(temp,5,"%2lu",value);
	UiDriverEncoderDisplay(column, row, label, enabled,
	                            temp, color);
}
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverProcessKeyboard
//* Object              : process hardcoded buttons click and hold
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverProcessKeyboard()
{
	uchar temp;

	if(ks.button_processed)	{
		ts.nb_disable = 1;	// disable noise blanker if button is pressed or held
		//
		UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing
		//
		//printf("button process: %02x, debounce time: %d\n\r",ks.button_id,ks.debounce_time);
        //
        AudioManagement_KeyBeep();  // make keyboard beep, if enabled
		// Is it click or hold ?
		if(!ks.press_hold)	{
			switch(ks.button_id)
			{
			//
			case TOUCHSCREEN_ACTIVE:				// touchscreen functions
				if(is_touchscreen_pressed()) {
					UiDriver_HandleTouchScreen();
				}
				break;
			case BUTTON_G1_PRESSED:	// BUTTON_G1 - Change operational mode
				if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX))	{	// do NOT allow mode change in TUNE mode or transmit mode
					UiDriverChangeDemodMode(0);
                    UiInitRxParms();        // re-init for change of filter including display updates
				}
				break;
			case BUTTON_G2_PRESSED:		// BUTTON_G2
				UiDriver_HandleSwitchToNextDspMode();
				break;
			case BUTTON_G3_PRESSED:		// BUTTON_G3 - Change power setting
				UiDriver_HandlePowerLevelChange(ts.power_level+1);
				break;
			case BUTTON_G4_PRESSED:		{		// BUTTON_G4 - Change filter bandwidth
				if(!ts.tune) {
				    if (filter_path_change == true) {
				      filter_path_change = false;
				    } else {
				      AudioFilter_NextApplicableFilterPath(PATH_USE_RULES,AudioFilter_GetFilterModeFromDemodMode(ts.dmod_mode),ts.filter_path);
				    }
				  // Change filter
				  UiInitRxParms();		// re-init for change of filter including display updates
				}
				break;
			}
			case BUTTON_M1_PRESSED:		// BUTTON_M1
				UiDriverChangeEncoderOneMode(0);
				break;
			case BUTTON_M2_PRESSED:		// BUTTON_M2
				UiDriverChangeEncoderTwoMode(0);
				break;
			case BUTTON_M3_PRESSED:		// BUTTON_M3
				UiDriverChangeEncoderThreeMode(0);
				break;
			case BUTTON_STEPM_PRESSED:		// BUTTON_STEPM
				if(!(ts.freq_step_config & FREQ_STEP_SWAP_BTN))	// button swap NOT enabled
					UiDriverChangeTuningStep(0);	// decrease step size
				else		// button swap enabled
					UiDriverChangeTuningStep(1);	// increase step size
				break;
			case BUTTON_STEPP_PRESSED:		// BUTTON_STEPP
				if(!(ts.freq_step_config & FREQ_STEP_SWAP_BTN))	// button swap NOT enabled
					UiDriverChangeTuningStep(1);	// increase step size
				else
					UiDriverChangeTuningStep(0);	// decrease step size
				break;
			case BUTTON_BNDM_PRESSED:		// BUTTON_BNDM
				UiDriver_HandleBandButtons(BUTTON_BNDM);
				break;
			case BUTTON_BNDP_PRESSED:	// BUTTON_BNDP
				UiDriver_HandleBandButtons(BUTTON_BNDP);
				break;
			case BUTTON_POWER_PRESSED:
				if(!ts.boot_halt_flag)	{	// do brightness adjust ONLY if NOT in "boot halt" mode
				  incr_wrap_uint8(&ts.lcd_backlight_brightness,0,3);
				}
				break;
			default:
				UiDriverProcessFunctionKeyClick(ks.button_id);
				break;
			}
		}
		else	{
			// *******************************************************************************
			// Process press-and-hold of button(s).  Note that this can accommodate multiple buttons at once.
			// *******************************************************************************
			//
			switch(ks.button_id)	{
			    case BUTTON_F1_PRESSED:	// Press-and-hold button F1:  Write settings to EEPROM
			        if(ts.txrx_mode == TRX_MODE_RX)	{				// only allow EEPROM write in receive mode
			            UiSpectrumClearDisplay();			// clear display under spectrum scope
			            if(ts.ser_eeprom_in_use == 0xFF)
			                UiLcdHy28_PrintText(60,160,"Saving settings to virt. EEPROM",Cyan,Black,0);
			            if(ts.ser_eeprom_in_use == 0x00)
			                UiLcdHy28_PrintText(60,160,"Saving settings to serial EEPROM",Cyan,Black,0);
			            UiConfiguration_SaveEepromValues();	// save settings to EEPROM
			            for(temp = 0; temp < 6; temp++)			// delay so that it may be read
			                non_os_delay();

	                    ts.menu_var_changed = 0;                    // clear "EEPROM SAVE IS NECESSARY" indicators
	                    UiDriverFButton_F1MenuExit();

                        if(ts.menu_mode)   {
                            UiMenu_RenderMenu(MENU_RENDER_ONLY);    // update menu display, was destroyed by message
                        } else {
                            UiSpectrumInitSpectrumDisplay();          // not in menu mode, redraw spectrum scope
                        }
			        }
			        break;
			case BUTTON_F2_PRESSED:	// Press-and-hold button F2
				// Move to the BEGINNING of the current menu structure
				if(ts.menu_mode)	{		// Are we in menu mode?
				  UiMenu_RenderFirstScreen();
				} else {
					// Not in MENU mode - select the METER mode
				    incr_wrap_uint8(&ts.tx_meter_mode,0,METER_MAX-1);

				    UiDriverDeleteSMeter();
					UiDriverCreateSMeter();	// redraw meter
				}
				break;
			case BUTTON_F3_PRESSED:	// Press-and-hold button F3
				//
				// Move to the END of the current menu structure
				if(ts.menu_mode){		// are we in menu mode?
				    UiMenu_RenderLastScreen();
				}
				else	{			// not in menu mode - toggle between VFO/SPLIT and Memory mode
					if(!ts.vfo_mem_flag)	{		// is it in VFO mode now?
						ts.vfo_mem_flag = 1;		// yes, switch to memory mode
						UiDriverFButtonLabel(3,"MEM",White);	// yes - indicate with color
					}
					else	{
						uint32_t color = is_splitmode()?SPLIT_ACTIVE_COLOUR:SPLIT_INACTIVE_COLOUR;
						ts.vfo_mem_flag = 0;		// it was in memory mode - switch to VFO mode
						UiDriverFButtonLabel(3,"SPLIT",color);
					}
					//
				}
				break;
			case BUTTON_F4_PRESSED:	// Press-and-hold button F4
				if(!ts.menu_mode)
				    {	// not in menu mode:  Make VFO A = VFO B or VFO B = VFO A, as appropriate
					__IO VfoReg* vfo_store;
					if(is_vfo_b())	{	// are we in VFO B mode?
						vfo_store = &vfo[VFO_A].band[ts.band];
					}
					else	{	// we were in VFO A mode
						vfo_store = &vfo[VFO_B].band[ts.band];
					}
					vfo_store->dial_value = df.tune_new;
					vfo_store->decod_mode = ts.dmod_mode;					// copy active VFO (A) settings into B
					UiDriver_FrequencyUpdateLOandDisplay(true);					UiSpectrumClearDisplay();			// clear display under spectrum scope
					UiLcdHy28_PrintText(80,160,is_vfo_b()?"VFO B -> VFO A":"VFO A -> VFO B",Cyan,Black,1);
					for(temp = 0; temp < 18; temp++)			// delay so that it may be read
						non_os_delay();

					UiSpectrumInitSpectrumDisplay();			// init spectrum scope
				}
				break;
			case BUTTON_F5_PRESSED:								// Button F5 was pressed-and-held - Toggle TX Disable
				if(ts.txrx_mode == TRX_MODE_RX)			// do NOT allow mode change in TUNE mode or transmit mode
				{
				  if( (ts.tx_disable&TX_DISABLE_USER) == false) {
				    ts.tx_disable |= TX_DISABLE_USER;
				  } else {
				    ts.tx_disable &= ~TX_DISABLE_USER;
				  }
				  UiDriverFButtonLabel(5,"TUNE",ts.tx_disable?Grey1:White);		// Set TUNE button color according to ts.tx_disable
				}
				break;
			case BUTTON_G1_PRESSED:	// Press-and-hold button G1 - Change operational mode, but include "disabled" modes
				if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX))	{	// do NOT allow mode change in TUNE mode or transmit mode
					UiDriverChangeDemodMode(1);		// go to next mode, including disabled modes
					UiInitRxParms();
				}
				break;
			case BUTTON_G2_PRESSED:		// Press and hold of BUTTON_G2 - turn DSP off/on
				if(ts.dmod_mode != DEMOD_FM)	{		// do not allow change of mode when in FM
					if(is_dsp_nr()|| is_dsp_notch() || ts.notch_enabled || ts.peak_enabled)	{			// is DSP NR or NOTCH active?
						ts.dsp_active_toggle = ts.dsp_active;	// save setting for future toggling
						ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE);				// turn off NR and notch
						ts.dsp_mode = DSP_SWITCH_OFF;
						ts.notch_enabled = 0;
						ts.peak_enabled = 0;				//off
						ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;
						UiDriverChangeRfGain(1);
						UiDriverDisplayNotch(0); // display
					}
					else	{		// neither notch or NR was active
						if(ts.dsp_active_toggle != 0xff)	{	// has this holder been used before?
							ts.dsp_active = ts.dsp_active_toggle;	// yes - load value
						}
					}
					audio_driver_set_rx_audio_filter();	// update DSP settings
					UiDriverChangeDSPMode();			// update on-screen display

					// Update DSP/NB/RFG control display
					if(ts.enc_two_mode == ENC_TWO_MODE_RF_GAIN)
						UiDriverChangeSigProc(0);
					else
						UiDriverChangeSigProc(1);
				}
					UiDriverChangeDSPMode();			// update on-screen display
				break;
			case BUTTON_G3_PRESSED:		{	// Press-and-hold button G3
				UiInitRxParms();			// generate "reference" for sidetone frequency
/*				if (ts.notch_enabled) {
				    ts.notch_enabled = 0; // switch off notch filter
				    UiDriverChangeRfGain(1);
				    // DSP/Noise Blanker
				    UiDriverChangeSigProc(0);
				    ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;
				    UiDriverDisplayNotch(0); // display
				}
				else {
				    ts.notch_enabled = 1;
				    // RF gain
				    UiDriverChangeRfGain(0);
				    // DSP/Noise Blanker
				    UiDriverChangeSigProc(0);
				    // notch display
				    UiDriverDisplayNotch(1);
				    ts.enc_two_mode = ENC_TWO_MODE_NOTCH_F;
				}
				*/
				break;
			}
			case BUTTON_G4_PRESSED:		{	// Press-and-hold button G4 - Change filter bandwidth, allowing disabled filters, or do tone burst if in FM transmit
				if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX))	{ // only allow in receive mode and when NOT in FM
				  // incr_wrap_uint8(&ts.filter_id,AUDIO_MIN_FILTER,AUDIO_MAX_FILTER-1);
				  // ts.filter_path = AudioFilter_NextApplicableFilterPath(ALL_APPLICABLE_PATHS,ts.dmod_mode,ts.filter_path>0?ts.filter_path-1:0)+1;
				  filter_path_change = true;
				  UiDriverChangeFilterDisplay();
	              // UiInitRxParms();            // update rx internals accordingly including the necessary display updates
				}
				else if((ts.txrx_mode == TRX_MODE_TX) && (ts.dmod_mode == DEMOD_FM))	{
					if(ts.fm_tone_burst_mode != FM_TONE_BURST_OFF)	{	// is tone burst mode enabled?
						ads.fm_tone_burst_active = 1;					// activate the tone burst
						ts.fm_tone_burst_timing = ts.sysclock + FM_TONE_BURST_DURATION;	// set the duration/timing of the tone burst
					}
				}
				break;
			}
			case BUTTON_M2_PRESSED:	// Press-and-hold button M2:  Switch display between DSP "strength" setting and NB (noise blanker) mode
				ts.dsp_active ^= DSP_NB_ENABLE;	// toggle whether or not DSP or NB is to be displayed
				//
				if(ts.enc_two_mode == ENC_TWO_MODE_RF_GAIN)
					UiDriverChangeSigProc(0);
				else
					UiDriverChangeSigProc(1);
				break;
			case BUTTON_M3_PRESSED:	// Press-and-hold button M3:  Switch display between MIC and Line-In mode
				if(ts.dmod_mode != DEMOD_CW)	{
				    incr_wrap_uint8(&ts.tx_audio_source,0,TX_AUDIO_MAX_ITEMS);
					//
					if(ts.enc_thr_mode == ENC_THREE_MODE_RIT)	// if encoder in RIT mode, grey out audio gain control
						UiDriverChangeAudioGain(0);
					else									// not RIT mode - don't grey out
						UiDriverChangeAudioGain(1);
				}
				break;
			case BUTTON_POWER_PRESSED:
				if(UiDriver_IsButtonPressed(BUTTON_BNDM_PRESSED))	{	// was button BAND- pressed at the same time?
				  UiDriver_LcdBlankingStealthSwitch();
				}
				else
				{	// ONLY the POWER button was pressed
					if(ts.txrx_mode == TRX_MODE_RX) {		// only allow power-off in RX mode
						mchf_board_power_off();
					}
				}
				break;
			case BUTTON_BNDM_PRESSED:			// BAND- button pressed-and-held?
				if(UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED))	{	// and POWER button pressed-and-held at the same time?
                  UiDriver_LcdBlankingStealthSwitch();
				} else if(UiDriver_IsButtonPressed(BUTTON_BNDP_PRESSED))	{	// and BAND-UP pressed at the same time?
					if(!ts.menu_mode)	{			// do not do this in menu mode!
					  UiDriver_ToggleWaterfallScopeDisplay();
					}
				}
				break;
			case BUTTON_BNDP_PRESSED:			// BAND+ button pressed-and-held?
				if(UiDriver_IsButtonPressed(BUTTON_BNDM_PRESSED))	{	// and BAND-DOWN pressed at the same time?
					if(!ts.menu_mode)	{		// do not do this if in menu mode!
					  UiDriver_ToggleWaterfallScopeDisplay();
					}
				}
				if(UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED))	{	// and POWER button pressed-and-held at the same time?
					ts.ser_eeprom_in_use = 0x20;			// power down without saving settings
					mchf_board_power_off();
				}
				break;
			case BUTTON_STEPM_PRESSED:
				if(UiDriver_IsButtonPressed(BUTTON_STEPP_PRESSED))	{	// was button STEP+ pressed at the same time?
					ts.frequency_lock = !ts.frequency_lock;
					// update frequency display
					UiDriver_FrequencyUpdateLOandDisplay(true);
				} else	{
					if(!(ts.freq_step_config & FREQ_STEP_SWAP_BTN))	{ // button swap NOT enabled
						UiDriverPressHoldStep(0);	// decrease step size
					} else {		// button swap enabled
						UiDriverPressHoldStep(1);	// increase step size
					}
				}
				//
				break;
			case BUTTON_STEPP_PRESSED:
				if(UiDriver_IsButtonPressed(BUTTON_STEPM_PRESSED))	{	// was button STEP- pressed at the same time?
					ts.frequency_lock = !ts.frequency_lock;
					// update frequency display
					UiDriver_FrequencyUpdateLOandDisplay(true);
				} else	{
					if(!(ts.freq_step_config & FREQ_STEP_SWAP_BTN)) {	// button swap NOT enabled
						UiDriverPressHoldStep(1);	// increase step size
					} else {		// button swap enabled
						UiDriverPressHoldStep(0);	// decrease step size
					}
				}
				break;
			default:
				break;
			}
		}
		//
		ts.nb_disable = 0;	// re-enable noise blanker when done processing buttons
		//
		// Reset flag, allow other buttons to be checked
		ks.button_processed = 0;
		ks.debounce_time	= 0;
	}
}
//
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiInitRxParms
//* Object              : Initializes/sets all of the crap associated with filters, DSP, band settings, etc.
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiInitRxParms()
{

    // Init / Functional changes to operation in RX path
	UiCWSidebandMode();
	// I do not think we need the TX adjustments in RX ?! DD4WH
	// not sure, I leave them here

	UiDriverSetDemodMode(ts.dmod_mode);
	UiDriver_FrequencyUpdateLOandDisplay(false);   // update frequency display without checking encoder


    if(ts.dmod_mode == DEMOD_CW)	{		// update on-screen adjustments

       // FIXME: Separate Display Update and Function Change
      // Mixed Display Update and Function Change Code
		UiDriverChangeKeyerSpeed(0);		// emplace keyer speed (WPM) and

		// display only code
		UiDriverChangeStGain(0);			// sidetone gain when in CW mode
	}
	else	{
	  // display only code
		UiDriverChangeAudioGain(0);			// display Line/Mic gain and
		UiDriverChangeCmpLevel(0);			// Compression level when in voice mode
	}

    // Update Display Only Code
    UiDriverChangeFilterDisplay();    // make certain that numerical on-screen bandwidth indicator is updated
//    audio_driver_set_rx_audio_filter();
    UiDriverChangeDigitalMode();    // Change Dgital display setting as well
    UiDriverChangeDSPMode();  // Change DSP display setting as well
    UiDriverDisplayFilterBW();  // update on-screen filter bandwidth indicator (graphical)
    UiDriverChangeRfGain(1);    // update RFG/SQL on screen
    UiDriverDisplayNotch(0);
    //UiDriverDisplayBass();

    if(ts.menu_mode)    // are we in menu mode?
        UiMenu_RenderMenu(MENU_RENDER_ONLY);    // yes, update display when we change modes

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
static void UiDriverPressHoldStep(uchar is_up)
{
	ulong	minus_idx, plus_idx;

	switch(df.selected_idx)	{		// select appropriate "alternate" step size based on current step size
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

	if(!is_up)	{		// temporary decrease of step size
		ts.tune_step = STEP_PRESS_MINUS;
		ts.tune_step_idx_holder = df.selected_idx;
		if(df.selected_idx)
		df.tuning_step	= tune_steps[minus_idx];
		df.selected_idx = minus_idx;
	}
	else	{			// temporary increase of step size
		ts.tune_step = STEP_PRESS_PLUS;
		ts.tune_step_idx_holder = df.selected_idx;
		df.tuning_step	= tune_steps[plus_idx];
		df.selected_idx = plus_idx;
	}
	//
	UiDriverShowStep(df.selected_idx);		// update display
}

//

void UiDriverDisplaySplitFreqLabels() {
  // in SPLIT mode?
  const char *split_rx, *split_tx;
  if (!(is_vfo_b())) {
    split_rx = "(A) RX->";  // Place identifying marker for RX frequency
    split_tx = "(B) TX->";  // Place identifying marker for TX frequency
  } else {
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


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverProcessFunctionKeyClick
//* Object              : process function buttons click
//* Object              : based on current demod mode
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverProcessFunctionKeyClick(ulong id)
{
	//printf("button: %02x\n\r",id);

	// --------------------------------------------
	// F1 process
	if(id == BUTTON_F1_PRESSED)
	{
		if(!ts.mem_disp)	{			// allow only if NOT in memory display mode
			if((!ts.menu_mode) && (!ts.boot_halt_flag))	{	// go into menu mode if NOT already in menu mode and not to halt on startup
				ts.menu_mode = 1;
				ts.encoder3state = filter_path_change;
				filter_path_change = false;			// deactivate while in menu mode
				UiDriverChangeFilterDisplay();
				UiSpectrumClearDisplay();
				UiDriverFButton_F1MenuExit();
				UiDriverFButtonLabel(2,"PREV",Yellow);
				UiDriverFButtonLabel(3,"NEXT",Yellow);
				UiDriverFButtonLabel(4,"DEFLT",Yellow);
				//
				//
				// Grey out adjustments and put encoders in known states
				//
				if(ts.dmod_mode == DEMOD_CW)
					UiDriverChangeStGain(0);
				else
					UiDriverChangeCmpLevel(0);
				//
				UiDriverChangeSigProc(0);
				UiDriverChangeRfGain(0);
				if(ts.dmod_mode == DEMOD_CW)
					UiDriverChangeKeyerSpeed(0);
				else
					UiDriverChangeAudioGain(0);
				//
				UiDriverChangeRit(0);
				//
				// Enable volume control when in MENU mode
				//
				UiDriverChangeAfGain(1);
				//
				ts.menu_var = 0;
				//
				UiMenu_RenderMenu(MENU_RENDER_ONLY);	// Draw the menu the first time
				UiMenu_RenderMenu(MENU_PROCESS_VALUE_CHANGE);	// Do update of the first menu item
			}
			else	{	// already in menu mode - we now exit
				ts.menu_mode = 0;
				filter_path_change = ts.encoder3state;
				UiDriverChangeFilterDisplay();
				UiSpectrumInitSpectrumDisplay();			// init spectrum scope
				//
				// Restore encoder displays to previous modes
				UiDriverChangeEncoderOneMode(0);
				UiDriverChangeEncoderTwoMode(0);
				UiDriverChangeEncoderThreeMode(0);
				// FIXME: Call twice since the function TOGGLES, not just enables (need to fix this at some point!)
				UiDriverChangeEncoderOneMode(0);
				UiDriverChangeEncoderTwoMode(0);
				UiDriverChangeEncoderTwoMode(0); // we have three different things: RFG/NB/NOTCH !
				UiDriverChangeEncoderThreeMode(0);
				UiDriverChangeFilterDisplay();	// update bandwidth display
				// Label for Button F1
				UiDriverFButton_F1MenuExit();
				// Label for Button F2
				UiDriverFButtonLabel(2,"SNAP",White);

				// Display Label for Button F3
				{
					uint32_t color;
					char* label;

					if(!ts.vfo_mem_flag)	{	// in normal VFO mode?
						label = " SPLIT";
						color = is_splitmode()?SPLIT_ACTIVE_COLOUR:SPLIT_INACTIVE_COLOUR;
					}
					else	{					// in memory mode
						color = White;
						label = "  MEM ";
					}
					UiDriverFButtonLabel(3,label,color);	// yes - indicate with color
				}
				// Display Label for Button F4
				{
					char* label = is_vfo_b()?"VFO B":"VFO A";
					// VFO B active?
					UiDriverFButtonLabel(4,label,White);
				}
			}
		}
	}

	// --------------------------------------------
	// F2 process
	if(id == BUTTON_F2_PRESSED)
	{
		//
		// If in MENU mode, this restores the DEFAULT setting
		//
		if(ts.menu_mode)	{		// Button F2 restores default setting of selected item
          UiMenu_RenderPrevScreen();
		}
		else	{
			sc.snap = 1;


		}
	}

	// --------------------------------------------
	// F3 process
	if(id == BUTTON_F3_PRESSED)	{
		//
		//
		if(ts.menu_mode)	{		// Previous screen
          UiMenu_RenderNextScreen();
		}
		else	{	// NOT menu mode
			if(!ts.vfo_mem_flag)	{		// update screen if in VFO (not memory) mode
				if(is_splitmode())	{	// are we in SPLIT mode?
					ts.vfo_mem_mode &= 0x7f;	// yes - turn off MSB to turn off SPLIT
					UiDriverInitMainFreqDisplay();		// update the main frequency display to reflect the mode
				}
				else if(!(is_splitmode()))	{	// are we NOT in SPLIT mode?
					ts.vfo_mem_mode |= 0x80;		// yes - turn on MSB to activate SPLIT
					UiDriverInitMainFreqDisplay();		//
				}
				UiDriver_FrequencyUpdateLOandDisplay(true);
			}
			else	{		// in memory mode
				UiSpectrumClearDisplay();		// always clear displayclear display
				if(!ts.mem_disp)	{	// are we NOT in memory display mode at this moment?
					ts.mem_disp = 1;	// we are not - turn it on
				}
				else	{				// we are in memory display mode
					ts.mem_disp = 0;	// turn it off
					UiSpectrumInitSpectrumDisplay();			// init spectrum scope
				}
			}
		}
	}

	// --------------------------------------------
	// F4 process
	if(id == BUTTON_F4_PRESSED)	{

	   if (ts.menu_mode) {
         UiMenu_RenderMenu(MENU_PROCESS_VALUE_SETDEFAULT);
		}
		else	{	// NOT menu mode
			uint8_t vfo_active,vfo_new;
			char* vfo_name;

			if(is_vfo_b())		{	// LSB on VFO mode byte set?
				vfo_active = VFO_A;
				vfo_new = VFO_B;
				vfo_name = "VFO A";
				ts.vfo_mem_mode &= 0xbf;	// yes, it's now VFO-B mode, so clear it, setting it to VFO A mode
			}
			else	{						// LSB on VFO mode byte NOT set?
				ts.vfo_mem_mode |= 0x40;			// yes, it's now in VFO-A mode, so set it, setting it to VFO B mode
				vfo_active = VFO_B;
				vfo_new = VFO_A;
				vfo_name = "VFO B";
			}
			vfo[vfo_new].band[ts.band].dial_value = df.tune_old;	//band_dial_value[ts.band];		// save "VFO B" settings
			vfo[vfo_new].band[ts.band].decod_mode = ts.dmod_mode;	//band_decod_mode[ts.band];

			UiDriverFButtonLabel(4,vfo_name,White);


			df.tune_new = vfo[vfo_active].band[ts.band].dial_value;
			//
			// do frequency/display update
			if(is_splitmode())	{	// in SPLIT mode?
                UiDriverDisplaySplitFreqLabels();
			}

			// Change decode mode if need to
			if(ts.dmod_mode != vfo[vfo_active].band[ts.band].decod_mode)
			{
				// Update mode
				ts.dmod_mode = vfo[vfo_active].band[ts.band].decod_mode;

				// Update Decode Mode (USB/LSB/AM/FM/CW)
				UiDriverSetDemodMode(ts.dmod_mode);
				UiDriverChangeFilterDisplay(); // YES
				UiInitRxParms();
			} else {
			    UiDriver_FrequencyUpdateLOandDisplay(true);
			}

		}
	}

	// --------------------------------------------
	// F5 process
	if(id == BUTTON_F5_PRESSED)
	{
	    ts.tune = RadioManagement_Tune(!ts.tune);
	    UiDriver_DisplayPowerLevel();           // former position was upper commented out position
        UiDriverFButtonLabel(5,"TUNE",ts.tune?Red:White);
	}
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverShowMode()	{
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
			txt = "SAM";
			break;
		case DEMOD_AM:
			txt = "AM";
			break;
		case DEMOD_FM:
            txt = "FM";
		    {
              if(ts.txrx_mode == TRX_MODE_RX)	{
                if(!ads.fm_squelched) {
                  // is audio not squelched?
                  if((ads.fm_subaudible_tone_detected) && (ts.fm_subaudible_tone_det_select))	{
                    // is tone decoding enabled AND a tone being detected?
                    clr_fg =  Black;
                    clr_bg = Red2;	// Not squelched, passing audio - change color!
                  } else {	// tone decoder disabled - squelch only
                    clr_fg = Black;
                    clr_bg = White;	// Not squelched, passing audio - change color, but different from tone
                  }
                }
              }
              else if(ts.txrx_mode == TRX_MODE_TX)	{	// in transmit mode?
                if(ads.fm_tone_burst_active)	{		// yes - is tone burst active?
                  clr_fg = Black;
                  clr_bg = Yellow;	// Yes, make "FM" yellow
                }
              }
              break;
		}
		case DEMOD_CW:
			txt = ts.cw_lsb?"CW-L":"CW-U";
			break;
		default:
			break;
	}
	UiLcdHy28_PrintTextCentered(POS_DEMOD_MODE_MASK_X,POS_DEMOD_MODE_MASK_Y,POS_DEMOD_MODE_MASK_W,txt,clr_fg,clr_bg,0);
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowStep
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverShowStep(ulong step)
{

	int	line_loc;
	static	bool	step_line = 0;	// used to indicate the presence of a step line
	uint32_t	color;
	uint32_t 	stepsize_background;

	color = ts.tune_step?Cyan:White;		// is this a "Temporary" step size from press-and-hold?
	stepsize_background = ts.dynamic_tuning_active?Blue:Black;
	// dynamic_tuning active -> yes, display on Grey3

	if(step_line)	{	// Remove underline indicating step size if one had been drawn
		UiLcdHy28_DrawStraightLineDouble((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * 3)),(POS_TUNE_FREQ_Y + 24),(LARGE_FONT_WIDTH*7),LCD_DIR_HORIZONTAL,Black);
		UiLcdHy28_DrawStraightLineDouble((POS_TUNE_SPLIT_FREQ_X + (SMALL_FONT_WIDTH * 3)),(POS_TUNE_FREQ_Y + 24),(SMALL_FONT_WIDTH*7),LCD_DIR_HORIZONTAL,Black);
	}

	// Blank old step size
	UiLcdHy28_DrawFullRect(POS_TUNE_STEP_MASK_X,POS_TUNE_STEP_MASK_Y,POS_TUNE_STEP_MASK_H,POS_TUNE_STEP_MASK_W,Black);

	{
		char step_name[10];

		// I know the code below will not win the price for the most readable code
		// ever. But it does the job of display any freq step somewhat reasonable.
		// khz/Mhz only whole  khz/Mhz is shown, no fraction
		// showing fractions would require some more coding, which is not yet necessary
		const uint32_t pow10 = log10(df.tuning_step);
		line_loc = 9 - pow10 - pow10/3;
		if (line_loc < 0) { line_loc = 0; }
		const char* stepUnitPrefix[] = { "","k","M","G","T"};
		snprintf(step_name,10,"%d%sHz",(int)(df.tuning_step/exp10((pow10/3)*3)), stepUnitPrefix[pow10/3]);

		UiLcdHy28_PrintTextRight((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*6),POS_TUNE_STEP_Y,step_name,color,stepsize_background,0);
	}
	//
	if((ts.freq_step_config & 0x0f) && line_loc > 0)	{		// is frequency step marker line enabled?
	    if(is_splitmode())
		UiLcdHy28_DrawStraightLineDouble((POS_TUNE_SPLIT_FREQ_X + (SMALL_FONT_WIDTH * line_loc)),(POS_TUNE_FREQ_Y + 24),(SMALL_FONT_WIDTH),LCD_DIR_HORIZONTAL,White);
	    else
		UiLcdHy28_DrawStraightLineDouble((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * line_loc)),(POS_TUNE_FREQ_Y + 24),(LARGE_FONT_WIDTH),LCD_DIR_HORIZONTAL,White);
	    step_line = 1;	// indicate that a line under the step size had been drawn
	}
	else	// marker line not enabled
		step_line = 0;	// we don't need to erase "step size" marker line in the future

}


typedef struct {
    uint32_t start;
    uint32_t end;
    const char* name;
} BandGenInfo;

const BandGenInfo bandGenInfo[] =  {
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
//* Function Name       : UiDriverShowBand
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverShowBand(uchar band)
{
    const char* bandName;
	bool print_bc_name = true;
	int idx;
	
	if (band < MAX_BAND_NUM) {
		ulong col;
		// Clear control
		if (band == BAND_MODE_GEN){
		    for (idx = 0; bandGenInfo[idx].start !=0; idx++) {
		        if (df.tune_old/TUNE_MULT >= bandGenInfo[idx].start && df.tune_old/TUNE_MULT < bandGenInfo[idx].end) {
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
		} else {
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

// TODO: Move out to RF HAL
static void RadioManagement_BandFilterPulseRelays() {
	BAND2_PIO->BSRRH = BAND2;
	non_os_delay();
	BAND2_PIO->BSRRL = BAND2;
}

// TODO: Move out to RF HAL
void RadioManagement_ChangeBandFilter(uchar band)
{
	// -------------------------------------------
	// 	 BAND		BAND0		BAND1		BAND2
	//
	//	 80m		1			1			x
	//	 40m		1			0			x
	//	 20/30m		0			0			x
	//	 15-10m		0			1			x
	//
	// ---------------------------------------------
	// Set LPFs:
	// Set relays in groups, internal first, then external group
	// state change via two pulses on BAND2 line, then idle
	//
	// then
	//
	// Set BPFs
	// Constant line states for the BPF filter,
	// always last - after LPF change
	switch(band)
	{
		case BAND_MODE_2200:
		case BAND_MODE_630:
		case BAND_MODE_160:
		case BAND_MODE_80:
		{
			// Internal group - Set(High/Low)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			RadioManagement_BandFilterPulseRelays();

			// External group -Set(High/High)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			RadioManagement_BandFilterPulseRelays();

			// BPF
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			break;
		}

		case BAND_MODE_60:
		case BAND_MODE_40:
		{
			// Internal group - Set(High/Low)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			RadioManagement_BandFilterPulseRelays();

			// External group - Reset(Low/High)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			RadioManagement_BandFilterPulseRelays();

			// BPF
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			break;
		}

		case BAND_MODE_30:
		case BAND_MODE_20:
		{
			// Internal group - Reset(Low/Low)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			RadioManagement_BandFilterPulseRelays();

			// External group - Reset(Low/High)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			RadioManagement_BandFilterPulseRelays();

			// BPF
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			break;
		}

		case BAND_MODE_17:
		case BAND_MODE_15:
		case BAND_MODE_12:
		case BAND_MODE_10:
		case BAND_MODE_6:
		case BAND_MODE_4:
		{
			// Internal group - Reset(Low/Low)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			RadioManagement_BandFilterPulseRelays();

			// External group - Set(High/High)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			RadioManagement_BandFilterPulseRelays();

			// BPF
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			break;
		}

		default:
			break;
	}

}



//*----------------------------------------------------------------------------
//* Function Name       : UiDriverInitMainFreqDisplay
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverInitMainFreqDisplay()
{
	if(!(is_splitmode()))	{	// are we in SPLIT mode?
		if(!ts.vfo_mem_flag)	{	// update bottom of screen if in VFO (not memory) mode
			UiDriverFButtonLabel(3,"SPLIT",SPLIT_INACTIVE_COLOUR);	// make SPLIT indicator grey to indicate off
		}
	}
	else	{	// are we NOT in SPLIT mode?
		if(!ts.vfo_mem_flag)	{	// update bottom of screen if in VFO (not memory) mode
			UiDriverFButtonLabel(3,"SPLIT",White);	// make SPLIT indicator YELLOW to indicate on
		}
		UiLcdHy28_PrintText(POS_TUNE_FREQ_X,POS_TUNE_FREQ_Y,"          ",White,Black,1);	// clear large frequency digits
		UiDriverDisplaySplitFreqLabels();
	}
	UiDriverShowStep(df.selected_idx);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateDesktop
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateDesktop()
{
	//char temp[10];

	// Backlight off - hide startup logo
	LCD_BACKLIGHT_PIO->BSRRH = LCD_BACKLIGHT;

	// Clear display
	UiLcdHy28_LcdClear(Black);

	// Create Band value
	UiDriverShowBand(ts.band);

	// Set filters
	RadioManagement_ChangeBandFilter(ts.band);

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	UiDriverShowMode();

	// Create Step Mode
	UiDriverShowStep(df.tuning_step);

	// Frequency
	UiDriverInitMainFreqDisplay();

	// Function buttons
	UiDriverCreateFunctionButtons(true);

	// S-meter
	UiDriverCreateSMeter();

	// Spectrum scope
	UiSpectrumInitSpectrumDisplay();
//	UiDriverCreateSpectrumScope();
//	UiDriverInitSpectrumDisplay();

	// -----------------
	// Encoder one modes
	// -----------------
	//  Audio gain
	UiDriverChangeAfGain(1);
	//
	if(ts.dmod_mode == DEMOD_CW)
		UiDriverChangeStGain(0);
	else
		UiDriverChangeCmpLevel(0);
	//

	// -----------------
	// Encoder two modes
	// -----------------
	// RF gain
	UiDriverChangeRfGain(1);
	// RF Attenuator
	UiDriverChangeSigProc(0);

	// -----------------
	// Encoder three modes
	// -----------------
	// RIT
	UiDriverChangeRit(1);
	//
	if(ts.dmod_mode == DEMOD_CW)
		UiDriverChangeKeyerSpeed(0);
	else
		UiDriverChangeAudioGain(0);
	//
	cw_gen_init();

	// DSP mode change
	UiDriverChangeDSPMode();

	// Digital mode change
	UiDriverChangeDigitalMode();

	// Power level
	UiDriver_DisplayPowerLevel();

	// FIR via keypad, not encoder mode
	UiDriverChangeFilterDisplay();

	UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator

	// Create USB Keyboard indicator
	//UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"KBD",Grey,Black,0);

	// Create RX/TX indicator
	//UiLcdHy28_PrintText(POS_TX_IND_X,POS_TX_IND_Y,	"RX", Green,Black,0);

	// Create voltage
	UiLcdHy28_DrawStraightLine	(POS_PWRN_IND_X,(POS_PWRN_IND_Y - 1),UI_LEFT_BOX_WIDTH,LCD_DIR_HORIZONTAL,Blue);
	UiLcdHy28_PrintTextCentered	(POS_PWRN_IND_X, POS_PWRN_IND_Y,UI_LEFT_BOX_WIDTH,"VCC", White, 	Blue, 0);
	UiLcdHy28_PrintTextCentered	(POS_PWR_IND_X,POS_PWR_IND_Y,UI_LEFT_BOX_WIDTH,   "--.- V",  COL_PWR_IND,Black,0);

	// Create temperature
	if((lo.sensor_absent == 0) && (df.temp_enabled & 0x0f))
		UiDriverCreateTemperatureDisplay(1,1);
	else
		UiDriverCreateTemperatureDisplay(0,1);

	// Set correct frequency
	//UiDriverUpdateFrequency(1,0);
	UiDriver_FrequencyUpdateLOandDisplay(true);

	// Backlight on - only when all is drawn
	LCD_BACKLIGHT_PIO->BSRRL = LCD_BACKLIGHT;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateFunctionButtons
//* Object              : function keys based on decoder mode
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateFunctionButtons(bool full_repaint)
{
	const char* cap;
	uint32_t	clr;

	// Create bottom bar
	if(full_repaint)
	{
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X +                             0),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*1 + 2),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*2 + 4),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*3 + 6),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*4 + 8),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,(POS_BOTTOM_BAR_BUTTON_W + 1),Grey);
	}

	// Button F1
	UiDriverFButton_F1MenuExit();
	// Button F2
	UiDriverFButtonLabel(2,"SNAP",White);

	// Button F3
	if(!ts.vfo_mem_flag) {	// is it in VFO (not memory) mode?
		cap = " SPLIT";
		clr= is_splitmode()?SPLIT_ACTIVE_COLOUR:SPLIT_INACTIVE_COLOUR;
	} else	{	// it is in memory mode (not VFO) mode
		clr = White;
		cap = "  MEM ";
	}

	UiDriverFButtonLabel(3,cap,clr);

	// Button F4
	UiDriverFButtonLabel(4,is_vfo_b()?"VFO B":"VFO A",White);

	// Button F5
	clr = ts.tx_disable?Grey1:White;
	UiDriverFButtonLabel(5,"TUNE",clr);
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDrawSMeter
//* Object              : draw the part of the S meter
//* Input Parameters    : uchar color
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverDrawSMeter(ushort color)
{
	uchar 	i,v_s;

	// Draw top line
	UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X +  18),(POS_SM_IND_Y + 20),92,LCD_DIR_HORIZONTAL,color);
	// Draw s markers on top white line
	for(i = 0; i < 10; i++)
	{
		// Draw s text, only odd numbers
		if(i%2)
			v_s = 5;
		else
			v_s = 3;
		// Lines
		UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,color);
	}
}
//


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDeleteSMeter
//* Object              : delete the S meter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverDeleteSMeter()
{
	ulong i;

	for(i = POS_SM_IND_Y; i < POS_SM_IND_Y + 72; i+=8)	{	// Y coordinate of blanking line
		UiLcdHy28_PrintText(POS_SM_IND_X + 6,((POS_SM_IND_Y + 5) +  i),"                                ",  White,Black,4);
	}
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateSMeter
//* Object              : draw the S meter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateSMeter()
{
	uchar 	i,v_s;
	char	num[20];
	int		col;

	// W/H ratio ~ 3.5
	UiLcdHy28_DrawEmptyRect(POS_SM_IND_X,POS_SM_IND_Y,72,202,Grey);

	// Draw top line
	UiLcdHy28_DrawFullRect((POS_SM_IND_X +  18),(POS_SM_IND_Y + 20),2,92,White);
	UiLcdHy28_DrawFullRect((POS_SM_IND_X +  113),(POS_SM_IND_Y + 20),2,75,Green);

	// Leading text
	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y +  5),"S",  White,Black,4);
	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 36),"P",  White,Black,4);

	// Trailing text
	UiLcdHy28_PrintText((POS_SM_IND_X + 185),(POS_SM_IND_Y + 5), "dB",Green,Black,4);
	UiLcdHy28_PrintText((POS_SM_IND_X + 185),(POS_SM_IND_Y + 36)," W",White,Black,4);

	// Draw s markers on top white line
	for(i = 0; i < 10; i++)
	{
		num[0] = i + 0x30;
		num[1] = 0;

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

	// Draw s markers on top green line
	for(i = 0; i < 4; i++)
	{
		// Prepare text
		num[0] = i*2 + 0x30;
		num[1] = 0x30;
		num[2] = 0x00;

		if(i)
		{
			// Draw text
			UiLcdHy28_PrintText(((POS_SM_IND_X + 113) - 6 + i*20),(POS_SM_IND_Y + 5),num,Green,Black,4);

			// Draw vert lines
			UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 113) + i*20),(POS_SM_IND_Y + 15),5,LCD_DIR_VERTICAL,Green);
		}
	}

	// Draw middle line
	UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 18),(POS_SM_IND_Y + 32),170,LCD_DIR_HORIZONTAL,White);

	// Draw s markers on middle white line
	for(i = 0; i < 12; i++)
	{
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
			// Text
			UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*15),(POS_SM_IND_Y + 36),num,White,Black,4);

			// Lines
			if(i)
			{
				UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*15),((POS_SM_IND_Y + 32) - 2),2,LCD_DIR_VERTICAL,White);
			}
			else
			{
				UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*15),((POS_SM_IND_Y + 32) - 7),7,LCD_DIR_VERTICAL,White);
			}
		}
	}


	if(ts.tx_meter_mode == METER_SWR)	{
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59),"SWR",Red2,Black,4);

		// Draw bottom line for SWR indicator
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55), 62,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 83),(POS_SM_IND_Y + 55),105,LCD_DIR_HORIZONTAL,Red);
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
					UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*10),(POS_SM_IND_Y + 59),num,White,Black,4);

					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
				}
			}
		}
	}
	else if(ts.tx_meter_mode == METER_ALC)	{
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59),"ALC",Yellow,Black,4);

		// Draw bottom line for SWR indicator
//		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55), 108,LCD_DIR_HORIZONTAL,White);
//		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 56), 108,LCD_DIR_HORIZONTAL,White);
//		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 129),(POS_SM_IND_Y + 55),59,LCD_DIR_HORIZONTAL,Red);
//		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 129),(POS_SM_IND_Y + 56),59,LCD_DIR_HORIZONTAL,Red);

		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55), 62,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 83),(POS_SM_IND_Y + 55),105,LCD_DIR_HORIZONTAL,Red);

		col = White;

		// Draw markers on middle line
		for(i = 0; i < 17; i++)
		{
			if(i > 6) col = Red;
			if(!(i%2))
			{
				if(i)
				{
					sprintf(num,"%d",(i*2));
					// Text
					UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*10),(POS_SM_IND_Y + 59),num,White,Black,4);

					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
				}
			}
		}
	}
	else if(ts.tx_meter_mode == METER_AUDIO)	{
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59),"AUD",Cyan,Black,4);

		// Draw bottom line for SWR indicator
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55), 108,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLineDouble((POS_SM_IND_X + 129),(POS_SM_IND_Y + 55),59,LCD_DIR_HORIZONTAL,Red);
		col = White;

		// Draw markers on middle line
		for(i = 0; i < 17; i++)
		{
			if(i > 10) col = Red;
			if(!(i%2))
			{
				if(i)
				{
					sprintf(num,"%d",(i*2)-20);
					// Text
					UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*10),(POS_SM_IND_Y + 59),num,White,Black,4);

					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLineDouble(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
				}
			}
		}
	}
	// Draw meters
	UiDriverUpdateTopMeterA(34);
	UiDriverUpdateTopMeterA(0);
	UiDriverUpdateBtmMeter(34, 34);
	UiDriverUpdateBtmMeter(0, 34);

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateTopMeterA
//* Object              : redraw indicator, same like upper implementation
//* Input Parameters    : but no hold
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
#define SMETER_MAX_LEVEL 33

enum {
  METER_TOP = 0,
  METER_BTM,
  METER_NUM
};
typedef struct MeterState_s {
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

static void UiDriverUpdateMeter(uchar val, uchar warn, uint32_t color_norm, uint8_t meterId)
{
  uchar     i;
  const uint8_t v_s = 3;
  uint32_t       col = color_norm;
  uint8_t from, to;
  uint8_t from_warn = 255;

  uint16_t ypos = meterId==METER_TOP?(POS_SM_IND_Y + 28):(POS_SM_IND_Y + 51);

  // limit meter
  if(val > SMETER_MAX_LEVEL) { val = SMETER_MAX_LEVEL; }
  if (warn == 0) { warn = SMETER_MAX_LEVEL+1; } // never warn if warn == 0

  if(warn != meters[meterId].last_warn)
  {
    if (warn < meters[meterId].last_warn) {
      from_warn = warn;
    } else {
      from_warn = meters[meterId].last_warn;
    }
  }


  if(val != meters[meterId].last || from_warn != 255) {

    // decide if we need to draw more boxes or delete some
    if (val > meters[meterId].last) {
      // we will draw more active boxes
      from = meters[meterId].last;
      to = val+1;

    } else {
      from = val;
      to   = meters[meterId].last+1;
    }
    if (from_warn < from) { from = from_warn; }

    // Draw indicator
    // we never draw a zero, so we start from 1 min
    if (from == 0) { from = 1; }

    for(i = from; i < to; i++)
    {
      if (i>val) {col = Grid; } // switch to delete color
      if((i >= warn) && warn && col != Grid) {  // is level above "warning" color? (is "warn" is zero, disable warning)
        col = Red2;                 // yes - display values above that color in red
      }
      // Lines
      UiLcdHy28_DrawStraightLineTriple(((POS_SM_IND_X + 18) + i*5),(ypos - v_s),v_s,LCD_DIR_VERTICAL,col);
    }

    meters[meterId].last = val;
    meters[meterId].last_warn = warn;
  }
}


static void UiDriverUpdateTopMeterA(uchar val)
{
  UiDriverUpdateMeter(val,SMETER_MAX_LEVEL+1,Blue2,METER_TOP);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateBtmMeter
//* Object              : redraw indicator
//* Input Parameters    : val=indicated value, warn=red warning threshold
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateBtmMeter(uchar val, uchar warn)
{
  UiDriverUpdateMeter(val,warn,Cyan,METER_BTM);
}


//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateDigiPanel
//* Object              : draw the digital modes info panel
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverCreateDigiPanel()
{
	ulong i;

	// Draw top band
	for(i = 0; i < 16; i++)
		UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y - 20 + i),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

	// Top band text - middle caption
	UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X + 85),
									(POS_SPECTRUM_IND_Y - 18),
									"DIGI PANEL",
									Grey,
									RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);

	// Draw control left and right border
	for(i = 0; i < 2; i++)
	{
		UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X - 2 + i),
									(POS_SPECTRUM_IND_Y - 20),
									(POS_SPECTRUM_IND_H + 12),
									LCD_DIR_VERTICAL,
									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));

		UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X + POS_SPECTRUM_IND_W - 2 + i),
									(POS_SPECTRUM_IND_Y - 20),
									(POS_SPECTRUM_IND_H + 12),
									LCD_DIR_VERTICAL,
									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
	}

	// Clear old spectrum part + frequency bar
	UiLcdHy28_DrawFullRect(POS_SPECTRUM_IND_X,POS_SPECTRUM_IND_Y - 4,POS_SPECTRUM_IND_H - 2,POS_SPECTRUM_IND_W - 2,Black);
}*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverInitFrequency
//* Object              : set default values, some could be overwritten later
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

void dummy() {

}
static void UiDriverInitFrequency()
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
	for(i = 0; i < 4; i++) {
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

	//if(ts.band == BAND_MODE_4)
	//	df.transv_freq = TRANSVT_FREQ_A;
	//else
//	df.transv_freq	= 0;	// LO freq, zero on HF, 42 Mhz on 4m

	// Set virtual segments initial value (diff than zero!)
	df.dial_digits[8]	= 0;
	df.dial_digits[7]	= 1;
	df.dial_digits[6]	= 4;
	df.dial_digits[5]	= 0;
	df.dial_digits[4]	= 0;
	df.dial_digits[3]	= 0;
	df.dial_digits[2]	= 0;
	df.dial_digits[1]	= 0;
	df.dial_digits[0]	= 0;
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckFilter
//* Object              : Sets the filter appropriate for the currently-tuned frequency
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------



typedef struct BandFilterDescriptor {
	uint32_t upper;
	uint16_t filter_band;
	uint16_t band_mode;
} BandFilterDescriptor;

#define BAND_FILTER_NUM 7

// The descriptor array below has to be ordered from the lowest BPF frequency filter
// to the highest.
static const BandFilterDescriptor bandFilters[BAND_FILTER_NUM] = {
		{ BAND_FILTER_UPPER_160, COUPLING_160M, BAND_MODE_160 },
		{ BAND_FILTER_UPPER_80,  COUPLING_80M, BAND_MODE_80 },
		{ BAND_FILTER_UPPER_40,  COUPLING_40M, BAND_MODE_40 },
		{ BAND_FILTER_UPPER_20,  COUPLING_20M, BAND_MODE_20 },
		{ BAND_FILTER_UPPER_10,  COUPLING_15M, BAND_MODE_10 },
		{ BAND_FILTER_UPPER_6,  COUPLING_6M, BAND_MODE_6 }
};


/**
 * @brief Select and activate the correct BPF for the frequency given in @p freq
 *
 *
 * @param freq The frequency to activate the BPF for in Hz
 *
 * @warning  If the frequency given in @p freq is too high for any of the filters, no filter change is executed.
 */
static void UiDriver_SetHWFiltersForFrequency(ulong freq)
{
	int idx;
	for (idx = 0; idx < BAND_FILTER_NUM; idx++) {
		if(freq < bandFilters[idx].upper)	{	// are we low enough if frequency for this band filter?
			if(ts.filter_band != bandFilters[idx].filter_band)	{
				RadioManagement_ChangeBandFilter(bandFilters[idx].band_mode);	// yes - set to desired band configuration
				ts.filter_band = bandFilters[idx].filter_band;
			}
			break;
		}
	}
}


// TODO: Split into RF HAL Part and UI Part
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckBand
//* Object              : Checks in which band the current frequency lies
//* Input Parameters    : frequency in Hz * 4, update = TRUE if display should be updated
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar UiDriverCheckBand(ulong freq, ushort update)
{
	uchar	band_scan;
	bool flag;
	static	uchar band_scan_old = 99;

	band_scan = 0;
	flag = 0;

	freq -= audio_driver_xlate_freq()*TUNE_MULT;

	while((!flag) && (band_scan < MAX_BANDS))	{
		if((freq >= bandInfo[band_scan].tune) && (freq <= (bandInfo[band_scan].tune + bandInfo[band_scan].size))) {	// Is this frequency within this band?
			flag = 1;	// yes - stop the scan
		} else {	// no - not in this band
			band_scan++;	// scan the next band
		}
	}

	if(update)	{		// are we to update the display?
		if(band_scan != band_scan_old || band_scan == BAND_MODE_GEN) {		// yes, did the band actually change?
			UiDriverShowBand(band_scan);	// yes, update the display with the current band
		}
	}
	band_scan_old = band_scan;	// update band change detector

	return band_scan;		// return with the band

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
//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_toggle_tx
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverUpdateFrequency(bool force_update, enum UpdateFrequencyMode_t mode)
{

    uint32_t		dial_freq;
    Si570_ResultCodes       lo_result = SI570_OK;
    bool        lo_change_not_pending = true;

    if(mode == UFM_SMALL_TX)	{				// are we updating the TX frequency (small, lower display)?
        uint8_t tx_vfo = is_vfo_b()?VFO_A:VFO_B;

        // TX uses the other VFO; RX == A -> TX == B and vice versa
        dial_freq = vfo[tx_vfo].band[ts.band].dial_value / TUNE_MULT;

        // we check with the si570 code if the frequency is tunable, we do not tune to it.
        lo_result = RadioManagement_ValidateFrequencyForTX(dial_freq);

    } else {
        dial_freq = df.tune_new/TUNE_MULT;
        lo_change_not_pending =  RadioManagement_ChangeFrequency(force_update, dial_freq, ts.txrx_mode);
        lo_result = ts.last_lo_result;   // use last ts.lo_result
    }

    // ALL UI CODE BELOW
    {
        uint32_t clr;

        if (lo_change_not_pending){

            if (mode != UFM_SMALL_TX) {
                UiDriverCheckBand(ts.tune_freq, true);
                // check which band in which we are currently tuning and update the display

                UiDriverUpdateLcdFreq(dial_freq + ((ts.txrx_mode == TRX_MODE_RX)?(ts.rit_value*20):0) ,White, UFM_SECONDARY);
                // set mode parameter to UFM_SECONDARY to update secondary display (it shows real RX frequency if RIT is being used)
                // color argument is not being used by secondary display
            }

            switch(lo_result) {
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
        } else {
            // we did not execute the change, so we show the freq in Blue.
            // this will turn into the appropriate color the moment the tuning
            // happens.
            clr = Blue;
        }
        // Update frequency display
        UiDriverUpdateLcdFreq(dial_freq, clr, mode);
    }
}



static void UiDriverUpdateFreqDisplay(ulong dial_freq, volatile uint8_t* dial_digits, ulong pos_x_loc, ulong font_width, ulong pos_y_loc, ushort color, uchar digit_size)
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
        for (idx = 0;idx < MAX_DIGITS;idx++){
            digits[idx] = dial_freq_temp % 10;
            dial_freq_temp /= 10;
            if (digits[idx] != 0) last_non_zero = idx;
        }
        for (idx = 0;idx < MAX_DIGITS;idx++){
            // -----------------------
            // See if digit needs update
            if ((digits[idx] != dial_digits[idx]) || ts.refresh_freq_disp){
                bool noshow = idx > last_non_zero;
                // don't show leading zeros, except for the 0th digits
                digit[0] = noshow?' ':0x30 + (digits[idx] & 0x0F);
                // Update segment
                UiLcdHy28_PrintText((pos_x_loc + pos_mult[idx] * font_width), pos_y_loc, digit, color, Black, digit_size);
            }
        }

        for (idx = 3;idx < MAX_DIGITS;idx+=3){
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
static void UiDriverUpdateLcdFreq(ulong dial_freq,ushort color, ushort mode)
{
	uchar		digit_size;
	ulong		pos_y_loc;
	ulong		pos_x_loc;
	ulong		font_width;
	volatile 	uint8_t*		digits_ptr;

	//
	//
	if(ts.frequency_lock) {
		// Frequency is locked - change color of display
		color = Grey;
	}

	//
	if(mode == UFM_AUTOMATIC)	{
		if(is_splitmode()) {	// in "split" mode?
			mode = UFM_SMALL_RX;				// yes - update upper, small digits (receive frequency)
		} else {
			mode = UFM_LARGE;				// NOT in split mode:  large, normal-sized digits
		}
	}

	// if (mode != UFM_SECONDARY) {
		ts.refresh_freq_disp = true; //because of coloured digits...
	// }
	if(ts.xverter_mode)	{	// transverter mode active?
		dial_freq *= (ulong)ts.xverter_mode;	// yes - scale by LO multiplier
		dial_freq += ts.xverter_offset;	// add transverter frequency offset
		if(dial_freq > 1000000000)		// over 1000 MHz?
			dial_freq -= 1000000000;		// yes, offset to prevent overflow of display
		if(ts.xverter_mode && mode != UFM_SECONDARY)	// if in transverter mode, frequency is yellow unless we do the secondary display
			color = Yellow;
	}

	// Handle frequency display offset in "CW RX" modes
	if(ts.dmod_mode == DEMOD_CW)	{		// In CW mode?
	    switch(ts.cw_offset_mode) {
	    case CW_OFFSET_LSB_RX:	// Yes - In an LSB mode with display offset?
			dial_freq -= ts.sidetone_freq;
			// yes, lower display freq. by sidetone amount
			break;
	    case CW_OFFSET_USB_RX:	// In a USB mode with display offset?
			dial_freq += ts.sidetone_freq;
			// yes, raise display freq. by sidetone amount
			break;
	    case CW_OFFSET_AUTO_RX:	// in "auto" mode with display offset?
			if(ts.cw_lsb) {
				dial_freq -= ts.sidetone_freq;		// yes - LSB - lower display frequency by sidetone amount
			} else {
 				dial_freq += ts.sidetone_freq;		// yes - USB - raise display frequency by sidetone amount
			}
			break;
		}
	}

	switch(mode) {
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
	UiDriverUpdateFreqDisplay(dial_freq, digits_ptr, pos_x_loc, font_width, pos_y_loc, color, digit_size);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeTuningStep
//* Object              : Change tunning step
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeTuningStep(uchar is_up)
{
	ulong 	idx = df.selected_idx;
	uint8_t idx_limit = T_STEP_MAX_STEPS -1;

    if(ts.freq_cal_adjust_flag) {
        idx_limit = T_STEP_1KHZ_IDX;
    }   else if((!ts.freq_cal_adjust_flag) && (!ts.xvtr_adjust_flag) && (!ts.xverter_mode)) {
        // are we NOT in "transverter adjust" or "frequency calibrate adjust" or transverter mode *NOT* on?
        idx_limit = T_STEP_100KHZ_IDX;
    }

	if(is_up) {
        idx= (idx>=idx_limit)?0:idx+1;
	} else {
		idx= (idx==0)?idx_limit:idx-1;
	}

	df.tuning_step	= tune_steps[idx];
	df.selected_idx = idx;

	// Update step on screen
	UiDriverShowStep(idx);

}

/*----------------------------------------------------------------------------
 * @brief Scans buttons 0-16:  0-15 are normal buttons, 16 is power button, 17 touch
 * @param button_num - 0-17
 * @returns true if button is pressed
 */

static bool UiDriver_IsButtonPressed(ulong button_num)
{
    bool retval = false;

    if(!GPIO_ReadInputDataBit(TP_IRQ_PIO,TP_IRQ) && ts.tp_state != 0xff)	// fetch touchscreen data if not already processed
	UiLcdHy28_GetTouchscreenCoordinates(1);

    if(GPIO_ReadInputDataBit(TP_IRQ_PIO,TP_IRQ) && ts.tp_state == 0xff)		// clear statemachine when data is processed
	{
	ts.tp_state = 0;
	ts.tp_x = ts.tp_y = 0xff;
	}

	if(button_num < BUTTON_NUM) {				// buttons 0-15 are the normal keypad buttons
	    if(!ts.boot_halt_flag) {				// are we NOT in "boot halt" mode?
	      retval = GPIO_ReadInputDataBit(bm[button_num].port,bm[button_num].button) == 0;		// in normal mode - return key value
	    }
	}
	return retval;
}


void UiDriver_KeyboardProcessOldClicks() {
  unsigned int i;

  static uchar press_hold_release_delay = 0;

  // State machine - processing old click
  if(ks.button_processed == false) {
    // State machine - click or release(debounce filter)
    if(!ks.button_pressed)  {
      // Scan inputs - 16 buttons in total, but on different ports
      for(i = 0; i < 18; i++)   {       // button "17" is touchscreen
        // Read each pin of the port, based on the declared pin map
        if(UiDriver_IsButtonPressed(i)) {
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
    else if((ks.debounce_time >= BUTTON_PRESS_DEBOUNCE) && (!ks.debounce_check_complete))   {
      if(UiDriver_IsButtonPressed(ks.button_id))    {   // button still pressed?
        ks.button_just_pressed = 1; // yes!
        ks.debounce_check_complete = 1; // indicate that the debounce check was completed
      }
      else
        ks.button_pressed = 0;          // debounce incomplete, button released - cancel detection
    }
    else if((ks.debounce_time >= BUTTON_HOLD_TIME) && (!ks.press_hold)) {   // press-and-hold processing
      ks.button_processed = 1;                      // indicate that a button was processed
      ks.button_just_pressed = 0;                   // clear this flag so that the release (below) won't be detected
      ks.press_hold = 1;
      press_hold_release_delay = PRESS_HOLD_RELEASE_DELAY_TIME; // Set up a bit of delay for when press-and-hold is released
    }
    else if(ks.press_hold && (!UiDriver_IsButtonPressed(ks.button_id))) {   // was there a press-and-hold and the button is now released?
      if(press_hold_release_delay)                  // press-and-hold delay expired?
        press_hold_release_delay--;                 // no - continue counting down before cancelling "press-and-hold" mode
      else  {                           // Press-and-hold mode time expired!
        ks.button_pressed = 0;          // reset and exit press-and-hold mode, this to prevent extraneous button-presses when using multiple buttons
        ks.button_released = 0;
        ks.press_hold = 0;
        ks.button_just_pressed = 0;
      }
    }
    else if(!UiDriver_IsButtonPressed(ks.button_id) && (!ks.press_hold))    {   // button released and had been debounced?
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
    if((ts.tune_step != 0) && (!ks.press_hold)) {   // are we in press-and-hold step size mode and did the button get released?
      ts.tune_step = STEP_PRESS_OFF;                        // yes, cancel offset
      df.selected_idx = ts.tune_step_idx_holder;            // restore previous setting
      df.tuning_step    = tune_steps[df.selected_idx];
      UiDriverShowStep(df.selected_idx);
    }
  }
}


enum TRX_States_t {
  TRX_STATE_TX_TO_RX,
  TRX_STATE_RX,
  TRX_STATE_RX_TO_TX,
  TRX_STATE_TX,
};

static void UiDriver_TxRxUiSwitch(enum TRX_States_t state) {
  static uchar enc_one_mode = ENC_ONE_MODE_AUDIO_GAIN;  // stores modes of encoder when we enter TX
  static uchar enc_three_mode = ENC_THREE_MODE_CW_SPEED;    // stores modes of encoder when we enter TX

  if((ts.flags1 & FLAGS1_TX_AUTOSWITCH_UI_DISABLE) == false)    {           // If auto-switch on TX/RX is enabled
    if(state == TRX_STATE_RX_TO_TX)   {

      // change display related to encoder one to TX mode (e.g. Sidetone gain or Compression level)
      enc_one_mode = ts.enc_one_mode;
      ts.enc_one_mode = ENC_ONE_MODE_ST_GAIN;
      UiDriverChangeAfGain(0);    // Audio gain disabled

      if(ts.dmod_mode != DEMOD_CW) {
        UiDriverChangeCmpLevel(1);    // enable compression adjust if voice mode
      } else {
        UiDriverChangeStGain(1);  // enable sidetone gain if CW mode
      }

      // change display related to encoder one to TX mode (e.g. CW speed or MIC/LINE gain)
      enc_three_mode = ts.enc_thr_mode;
      ts.enc_thr_mode = ENC_THREE_MODE_CW_SPEED;
      UiDriverChangeRit(0);
      if(ts.dmod_mode != DEMOD_CW) {
        UiDriverChangeAudioGain(1);       // enable audio gain
      } else {
        UiDriverChangeKeyerSpeed(1);  // enable keyer speed if it was CW mode
      }
    }
    else if (state == TRX_STATE_TX_TO_RX) {

      // were we latched in TX mode?
      // Yes, Switch to audio gain mode
      ts.enc_one_mode = enc_one_mode;
      if(ts.enc_one_mode == ENC_ONE_MODE_AUDIO_GAIN)  {   // are we to switch back to audio mode?
        UiDriverChangeAfGain(1);  // Yes, audio gain enabled
        if(ts.dmod_mode != DEMOD_CW) {
          UiDriverChangeCmpLevel(0);  // disable compression level (if in voice mode)
        } else {
          UiDriverChangeStGain(0);    // disable sidetone gain (if in CW mode)
        }
      }

      ts.enc_thr_mode = enc_three_mode;
      if(ts.enc_thr_mode == ENC_THREE_MODE_RIT)   {       // are we to switch back to RIT mode?
        UiDriverChangeRit(1);         // enable RIT
        if(ts.dmod_mode != DEMOD_CW) {
          UiDriverChangeAudioGain(0);     // disable audio gain if it was voice mode
        } else {
          UiDriverChangeKeyerSpeed(0);    // disable keyer speed if it was CW mode
        }
      }
    }
  }

  UiDriverUpdateBtmMeter(0,0);        // clear bottom meter of any outstanding indication when going back to RX
  if((ts.menu_mode))  {           // update menu when we are (or WERE) in MENU mode
    UiMenu_RenderMenu(MENU_RENDER_ONLY);
  }
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverKeypadCheck, adjust volume and return to RX from TX and other time-related functions
//* Object              : implemented as state machine, to avoid (additional) interrupts
//* Object              : and stall of app loop
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverTimeScheduler()
{
  static bool	 unmute_flag = 1;
  static bool old_squelch = 0;	// used to detect change-of-state of squelch
  static bool old_tone_det = 0;	// used to detect change-of-state of tone decoder
  static bool old_tone_det_enable = 0;	// used to detect change-of-state of tone decoder enabling
  static bool old_burst_active = 0;		// used to detect state of change of tone burst generator
  static bool startup_done_flag = 0;
  static bool	dsp_rx_reenable_flag = 0;
  static ulong dsp_rx_reenable_timer = 0;
  static bool  unmute_delay_active = false;
  static uchar dsp_crash_count = 0;

  static enum TRX_States_t last_state = TRX_STATE_RX; // we assume everything is
  enum TRX_States_t state;


  // let us figure out if we are in a stable state or if this
  // is the first run after a mode change
  if (ts.txrx_mode == TRX_MODE_TX) {
    if (last_state != TRX_STATE_TX) {
      state = TRX_STATE_RX_TO_TX;
    } else {
      state = TRX_STATE_TX;
    }
    last_state = TRX_STATE_TX;
  } else {
    if (last_state != TRX_STATE_RX) {
      state = TRX_STATE_TX_TO_RX;
    } else {
      state = TRX_STATE_RX;
    }
    last_state = TRX_STATE_RX;
  }

  /*** RX MODE ***/
  if(ts.txrx_mode == TRX_MODE_RX) {

    if (state == TRX_STATE_TX_TO_RX) {

      // TR->RX audio un-muting timer and Audio/AGC De-Glitching handler
      if(ts.audio_unmute)	{						// are we returning from TX with muted audio?
        if(ts.dmod_mode == DEMOD_CW)	{		// yes - was it CW mode?
          ts.unmute_delay_count = (ulong)ts.cw_rx_delay + 1;	// yes - get CW TX->RX delay timing
          ts.unmute_delay_count++;
          ts.unmute_delay_count *= 40;	// rescale value and limit minimum delay value
        } else {								// SSB mode
          ts.unmute_delay_count = SSB_RX_DELAY;	// set time delay in SSB mode
          ts.buffer_clear = 1;
        }
        unmute_delay_active = true;
        ts.audio_unmute = 0;					// clear flag
      }
    }

    if(unmute_delay_active  && !ts.unmute_delay_count)	{	// did timer hit zero
      unmute_delay_active = false;
      unmute_flag = 1;
      ts.buffer_clear = 0;
      ads.agc_val = ads.agc_holder;		// restore AGC value that was present when we went to TX
    }

    if(ts.band_change)    {   // did we un-mute because of a band change
      ts.band_change = 0;     // yes, reset the flag
      ads.agc_val = ads.agc_holder;   // restore previously-stored AGC value before the band change (minimize "POP" desense)
      unmute_flag = 1;
    }


    // Audio un-muting handler and volume control handler
    if(ts.boot_halt_flag)	{	// are we halting boot?
      ts.rx_gain[RX_AUDIO_SPKR].active_value = 0;	// yes - null out audio
      Codec_Volume(0,ts.txrx_mode);
    }
    else if((ts.rx_gain[RX_AUDIO_SPKR].value != ts.rx_gain[RX_AUDIO_SPKR].value_old) || unmute_flag)	{	// in normal mode - calculate volume normally

      ts.rx_gain[RX_AUDIO_SPKR].value_old = ts.rx_gain[RX_AUDIO_SPKR].value;
      ts.rx_gain[RX_AUDIO_SPKR].active_value = 1;		// software gain not active - set to unity
      if(ts.rx_gain[RX_AUDIO_SPKR].value <= 16) {				// Note:  Gain > 16 adjusted in audio_driver.c via software
        Codec_Volume((ts.rx_gain[RX_AUDIO_SPKR].value*5),ts.txrx_mode);
      } else {	// are we in the "software amplification" range?
        Codec_Volume((80),ts.txrx_mode);		// set to fixed "maximum" gain
        ts.rx_gain[RX_AUDIO_SPKR].active_value = (float)ts.rx_gain[RX_AUDIO_SPKR].value;	// to float
        ts.rx_gain[RX_AUDIO_SPKR].active_value /= 2.5;	// rescale to reasonable step size
        ts.rx_gain[RX_AUDIO_SPKR].active_value -= 5.35;	// offset to get gain multiplier value
      }

      unmute_flag = 0;

      dsp_rx_reenable_flag = 1;		// indicate that we need to re-enable the DSP soon
      dsp_rx_reenable_timer = ts.sysclock + DSP_REENABLE_DELAY;	// establish time at which we re-enable the DSP
    }

    // Check to see if we need to re-enable DSP after return to RX
    if(dsp_rx_reenable_flag)	{	// have we returned to RX after TX?
      if(ts.sysclock > dsp_rx_reenable_timer)	{	// yes - is it time to re-enable DSP?
        ts.dsp_inhibit = 0;		// yes - re-enable DSP
        dsp_rx_reenable_flag = 0;	// clear flag so we don't do this again
      }
    }
    // Check to see if we need to re-enabled DSP after disabling after a function that disables the DSP (e.g. band change)
    if(ts.dsp_timed_mute) {
      if(ts.sysclock > ts.dsp_inhibit_timing) {
        ts.dsp_timed_mute = 0;
        ts.dsp_inhibit = 0;
      }
    }

    // update the on-screen indicator of squelch/tone detection (the "FM" mode text) if there is a change of state of squelch/tone detection
    if(!ts.boot_halt_flag)    {   // do this only if not in "boot halt" mode
      if((old_squelch != ads.fm_squelched)
        || (old_tone_det != ads.fm_subaudible_tone_detected)
        || (old_tone_det_enable != (bool)ts.fm_subaudible_tone_det_select))   {   // did the squelch or tone detect state just change?

        UiDriverShowMode();                           // yes - update on-screen indicator to show that squelch is open/closed
        old_squelch = ads.fm_squelched;
        old_tone_det = ads.fm_subaudible_tone_detected;
        old_tone_det_enable = (bool)ts.fm_subaudible_tone_det_select;
      }
    }

    // DSP crash detection
    if(is_dsp_nr() && !is_dsp_nr_postagc() && !ads.af_disabled && !ts.dsp_inhibit) {  // Do this if enabled and "Pre-AGC" DSP NR enabled

      if((ads.dsp_nr_sample > DSP_HIGH_LEVEL) || (ads.dsp_nr_sample == -1)) {     // is the DSP output very high, or wrapped around to -1?
        dsp_crash_count+=2;           // yes - increase detect count quickly
      } else {                        // not high level
        if(dsp_crash_count) {         // decrease detect count more slowly
          dsp_crash_count--;
        }
      }

      if((ads.dsp_zero_count > DSP_ZERO_COUNT_ERROR) || (dsp_crash_count > DSP_CRASH_COUNT_THRESHOLD))    {   // is "zero" output count OR high level count exceeding threshold?
        ts.reset_dsp_nr = 1;              // yes - DSP has likely crashed:  Set flag to reset DSP NR coefficients
        audio_driver_set_rx_audio_filter();   // update DSP settings
        ts.reset_dsp_nr = 0;              // clear "reset NR coefficients" flag
        dsp_crash_count = 0;              // clear crash count flag
      }
    }

    if((ts.sysclock >= ts.rx_blanking_time) && (ts.rx_muting) && (ts.rx_blanking_time> RX_MUTE_START_DELAY))  {
      // is it time to un-mute audio AND have we NOT done it already AND is it long enough after start-up to allow muting?
      ads.agc_val = ads.agc_holder;           // restore AGC setting
      ts.dsp_inhibit = ts.dsp_inhibit_mute;   // restore previous state of DSP inhibit flag
      ts.rx_muting = 0;                       // unmute receiver audio
    }
  }

  /*** TX MODE ONLY ***/
  if(ts.txrx_mode == TRX_MODE_TX) {

    if((state == TRX_STATE_RX_TO_TX)) {
      // we just switched to TX
      if((ts.dmod_mode != DEMOD_CW))    {   // did we just enter TX mode in voice mode?
        ts.tx_audio_muting_timer = ts.tx_audio_muting_timing + ts.sysclock;             // calculate expiry time for audio muting
        ts.tx_audio_muting_flag = 1;
        ads.alc_val = 1;    // re-init AGC value
        ads.peak_audio = 0; // clear peak reading of audio meter
      }

    }
    // Did the TX muting expire?
    if(ts.sysclock >= ts.tx_audio_muting_timer)  {
      ts.tx_audio_muting_flag = 0;                // Yes, un-mute the transmit audio
    }


    // Has the timing for the tone burst expired?
    if(ts.sysclock > ts.fm_tone_burst_timing) {
      ads.fm_tone_burst_active = 0;               // yes, turn the tone off
    }

    if(ads.fm_tone_burst_active != old_burst_active)   {   // did the squelch or tone detect state just change?
      UiDriverShowMode();                           // yes - update on-screen indicator to show that tone burst is on/off
      old_burst_active = ads.fm_tone_burst_active;
    }
  }

  /*** TX+RX STATE CHANGE ONLY ***/
  // if we do change modes, some visuals need an update
  if(state == TRX_STATE_RX_TO_TX || state == TRX_STATE_TX_TO_RX) {
    UiDriver_TxRxUiSwitch(state);
  }


  /*** ALWAYS ***/
  UiDriver_LcdBlankingProcessTimer();

  UiDriver_KeyboardProcessOldClicks();

  // This delays the start-up of the DSP for several seconds to minimize the likelihood that the LMS function will get "jammed"
  // and stop working.  It also does a delayed detection - and action - on the presence of a new version of firmware being installed.

  /*** ONCE AFTER STARTUP DELAY ***/
  if((ts.sysclock > DSP_STARTUP_DELAY) && (!startup_done_flag))   {   // has it been long enough after startup?
    startup_done_flag = 1;                  // set flag so that we do this only once

    ts.dsp_inhibit = 0;                 // allow DSP to function
    unmute_flag = 1;                    // set unmute flag to force audio to be un-muted - just in case it starts up muted!
    Codec_Mute(0);                      // make sure that audio is un-muted


    if((ts.version_number_build != TRX4M_VER_BUILD) || (ts.version_number_release != TRX4M_VER_RELEASE) || (ts.version_number_minor != TRX4M_VER_MINOR))    {   // Yes - check for new version

      ts.version_number_build = TRX4M_VER_BUILD;    // save new F/W version
      ts.version_number_release = TRX4M_VER_RELEASE;
      ts.version_number_minor = TRX4M_VER_MINOR;

      UiSpectrumClearDisplay();         // clear display under spectrum scope
      UiLcdHy28_PrintText(110,156,"- New F/W detected -",Cyan,Black,0);
      UiLcdHy28_PrintText(110,168,"  Settings adjusted ",Cyan,Black,0);

      {
        int i;
        for(i = 0; i < 6; i++) {            // delay so that it may be read
          non_os_delay();
        }
      }
      UiSpectrumInitSpectrumDisplay();          // init spectrum scope
    }

  }
}


// RF API
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSetDemodMode
//* Object              : change demodulator mode
//* Input Parameters    : new_mode contains the new mode
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverSetDemodMode(uint32_t new_mode)
{

	// Finally update public flag
	ts.dmod_mode = new_mode;

	AudioManagement_SetSidetoneForDemodMode(ts.dmod_mode);

	Audio_TXFilter_Init();
    AudioManagement_CalcRxIqGainAdj();
	AudioManagement_CalcTxIqGainAdj();

	audio_driver_set_rx_audio_filter();

    // Update Decode Mode (USB/LSB/AM/FM/CW)
    UiDriverShowMode();
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeDemodMode
//* Object              : change demodulator mode
//* Input Parameters    : "noskip", if TRUE, disabled modes are to be included
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeDemodMode(uchar noskip)
{
	ulong loc_mode = ts.dmod_mode;	// copy to local, so IRQ is not affected


	if((ts.lsb_usb_auto_select) && (noskip))	{	// noskip mode with auto-select enabled
		if(loc_mode == DEMOD_LSB)					// if LSB, flip to USB
			loc_mode = DEMOD_USB;
		else if(loc_mode == DEMOD_USB)				// if USB, flip to LSB
			loc_mode = DEMOD_LSB;
		else										// None of the above?
			loc_mode++;								// Increase mode
	}
	else				// Normal changing of the mode
		loc_mode++;		// Increase mode

	if(!noskip)	{		// Are we NOT to skip disabled modes?
		if(loc_mode == DEMOD_AM)	{	// yes - is this AM mode?
			if(ts.am_mode_disable)		// is AM to be disabled?
				loc_mode++;				// yes - go to next mode
		}
		if(loc_mode == DEMOD_FM)	{	// is this FM mode?
			if((!(ts.flags2 & FLAGS2_FM_MODE_ENABLE)) || (ts.band != BAND_MODE_10 && ts.lsb_usb_auto_select))	// is FM to be disabled?
				loc_mode++;				// yes - go to next mode
		}
	}

	if((loc_mode == DEMOD_FM) && (!ts.iq_freq_mode))	{	// are we in FM and frequency translate is off?
		loc_mode++;		// yes - FM NOT permitted unless frequency translate is active, so skip!
	}

	if(loc_mode == DEMOD_SAM)	{	// yes - is this SAM mode?
		if(!ts.sam_enabled)		// is SAM to be disabled?
			loc_mode++;				// yes - go to next mode
	}

	// Check for overflow
	if(loc_mode >= DEMOD_MAX_MODE)
		loc_mode = DEMOD_USB;


	if((ts.lsb_usb_auto_select) && (!noskip))	{	// is auto-select LSB/USB mode enabled AND mode-skip NOT enabled?
		if((loc_mode == DEMOD_USB) || (loc_mode == DEMOD_LSB))	{	// is this a voice mode, subject to "auto" LSB/USB select?
			if((ts.lsb_usb_auto_select == AUTO_LSB_USB_60M) && ((df.tune_new < USB_FREQ_THRESHOLD) && (ts.band != BAND_MODE_60)))	{	// are we <10 MHz and NOT on 60 meters?
				if(loc_mode == DEMOD_USB)	{		// are we in USB mode?
					loc_mode++;					// yes - bump to the next mode
				}
			}
			else if((ts.lsb_usb_auto_select == AUTO_LSB_USB_ON) && (df.tune_new < USB_FREQ_THRESHOLD))	{	// are we <10 MHz (not in 60 meter mode)
				if(loc_mode == DEMOD_USB)	{		// are we in USB mode?
					loc_mode++;					// yes - bump to the next mode
				}
			}
			else	{	// we must be > 10 MHz OR on 60 meters
				if(loc_mode == DEMOD_LSB)	{		// are we in LSB mode?
					loc_mode++;				// yes - bump to the next mode
				}
			}
		}
	}
	UiDriverSetDemodMode(loc_mode);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeBand
//* Object              : change band
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeBand(uchar is_up)
{
    ulong 	curr_band_index;	// index in band table of currently selected band
    ulong	new_band_index;		// index of the new selected band

    ulong 	new_band_freq;		// new dial frequency

    uint16_t vfo_sel = is_vfo_b()?VFO_B:VFO_A;

    //printf("-----------> change band\n\r");

    // Do not allow band change during TX
    if(ts.txrx_mode != TRX_MODE_TX)
    {

        Codec_Volume(0,ts.txrx_mode);		// Turn volume down to suppress click
        ts.band_change = 1;		// indicate that we need to turn the volume back up after band change
        ads.agc_holder = ads.agc_val;	// save the current AGC value to reload after the band change so that we can better recover
        // from the loud "POP" that will occur when we change bands

        curr_band_index = ts.band;

        //printf("current index: %d and freq: %d\n\r",curr_band_index,tune_bands[ts.band]);

        // Save old band values
        if(curr_band_index < (MAX_BANDS) && ts.cat_band_index == 255)
        {
            // Save dial
            vfo[vfo_sel].band[curr_band_index].dial_value = df.tune_old;
            vfo[vfo_sel].band[curr_band_index].decod_mode = ts.dmod_mode;
        }
        else
            ts.cat_band_index = 255;

        // Handle direction
        if(is_up)
        {
            if(curr_band_index < (MAX_BANDS - 1))
            {
                //printf("going up band\n\r");

                // Increase
                new_band_index = curr_band_index + 1;
                if(ts.rfmod_present == 0 && ts.vhfuhfmod_present == 0 && curr_band_index == 8)
                {						// jump 10m --> 160m
                    new_band_index = MAX_BANDS-1;
                }
                if(ts.rfmod_present == 0 && ts.vhfuhfmod_present == 1 && curr_band_index == 8)
                {						// jump 10m --> 2m
                    new_band_index = 11;
                }
                if(ts.rfmod_present == 0 && ts.vhfuhfmod_present == 1 && curr_band_index == 13)
                {						// jump 2200m --> 16m
                    new_band_index = 16;
                }
                if(ts.rfmod_present == 1 && ts.vhfuhfmod_present == 0 && curr_band_index == 10)
                {						// jump 4m --> 2200m
                    new_band_index = 14;
                }

            }
            else	{	// wrap around to the lowest band
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
                {		// jump 160m --> 23cm
                    new_band_index = 13;
                }
                if(ts.vhfuhfmod_present == 0 && new_band_index == 13)
                {		// jump 2200m --> 6m
                    new_band_index = 10;
                }
                if(ts.rfmod_present == 0 && new_band_index == 10)
                {		// jump 2m --> 10m
                    new_band_index = 8;
                }
            }
            else
            {	// wrap around to the highest band
                new_band_index = MAX_BANDS-1;
            }
        }
        new_band_freq  = bandInfo[curr_band_index].tune;

        // TODO: There is a strong similarity to code in UiDriverProcessFunctionKeyClick around line 2053
        // Load frequency value - either from memory or default for
        // the band if this is first band selection
        if(vfo[vfo_sel].band[new_band_index].dial_value != 0xFFFFFFFF)
            df.tune_new = vfo[vfo_sel].band[new_band_index].dial_value;	// Load value from VFO
        else
            df.tune_new = new_band_freq; 					// Load new frequency from startup

        //	UiDriverUpdateFrequency(1,0);

        //	// Also reset second freq display
        //	UiDriverUpdateSecondLcdFreq(df.tune_new/TUNE_MULT);

        // Change decode mode if need to
        if(ts.dmod_mode != vfo[vfo_sel].band[new_band_index].decod_mode)
        {
            // Update mode
            ts.dmod_mode = vfo[vfo_sel].band[new_band_index].decod_mode;

            // Update Decode Mode (USB/LSB/AM/FM/CW)
            UiDriverShowMode();
        }


        // Create Band value
        UiDriverShowBand(new_band_index);

        // Set TX power factor
        UiDriverSetBandPowerFactor(new_band_index);

        // Set filters
        RadioManagement_ChangeBandFilter(new_band_index);

        // Finally update public flag
        ts.band = new_band_index;

        UiDriver_FrequencyUpdateLOandDisplay(false);
    }
}

/**
 * @brief Read out the changes in the frequency encoder and initiate frequency change by setting a global variable.
 *
 * @returns true if a frequency change was detected and a new tuning frequency was set in a global variable.
 */
static bool UiDriverCheckFrequencyEncoder()
{
	int 		pot_diff;
	bool		retval = false;
	int		enc_multiplier;
	static float 	enc_speed_avg = 0.0;  //keeps the averaged encoder speed
	int		delta_t, enc_speed;
	// char	num[8];


	pot_diff = UiDriverEncoderRead(ENCFREQ);


	if (pot_diff != 0){
		delta_t = ts.audio_int_counter;  // get ticker difference since last enc. change
		ts.audio_int_counter = 0;		 //reset tick counter

	    UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing

	}
	if (pot_diff != 0 &&
			ts.txrx_mode == TRX_MODE_RX
			&& ks.button_just_pressed == false
			&& ts.frequency_lock == false)	{
		// allow tuning only if in rx mode, no freq lock,
		if (delta_t > 300) { enc_speed_avg = 0; } //when leaving speedy turning set avg_speed to 0

		enc_speed = div(4000,delta_t).quot*pot_diff;  // app. 4000 tics per second -> calc. enc. speed.

		if (enc_speed > 500) { enc_speed = 500; }   //limit calculated enc. speed
		if (enc_speed < -500){ enc_speed = -500; }

		enc_speed_avg = 0.1*enc_speed + 0.9*enc_speed_avg; // averaging to smooth encoder speed

		enc_multiplier = 1; //set standard speed

		if (ts.dynamic_tuning_active)   // check if dynamic tuning has been activated by touchscreen
		{
			if ((enc_speed_avg > 80) || (enc_speed_avg < (-80)))   { enc_multiplier = 10; } // turning medium speed -> increase speed by 10
			if ((enc_speed_avg > 300) || (enc_speed_avg < (-300))) { enc_multiplier = 100; } //turning fast speed -> increase speed by 100

			if ((df.tuning_step == 10000) && (enc_multiplier > 10)) { enc_multiplier = 10; } //limit speed to 100000kHz/step
			if ((df.tuning_step == 100000) && (enc_multiplier > 1)) { enc_multiplier = 1; } //limit speed to 100000kHz/step
		}


		// Finaly convert to frequency incr/decr

		if(pot_diff>0) {
			df.tune_new += (df.tuning_step * TUNE_MULT * enc_multiplier);
			//itoa(enc_speed,num,6);
			//UiSpectrumClearDisplay();			// clear display under spectrum scope
			//UiLcdHy28_PrintText(110,156,num,Cyan,Black,0);
		} else {
			df.tune_new -= (df.tuning_step * TUNE_MULT * enc_multiplier);
		}

		if (enc_multiplier != 1) {  df.tune_new = TUNE_MULT*enc_multiplier*df.tuning_step * div((df.tune_new/TUNE_MULT),enc_multiplier*df.tuning_step).quot; } // keep last digit to zero

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
static void UiDriverCheckEncoderOne()
{
	int 	pot_diff;

	pot_diff = UiDriverEncoderRead(ENC1);

	if (pot_diff) {
		UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing
		// Take appropriate action
		switch(ts.enc_one_mode)
		{
		// Update audio volume
		case ENC_ONE_MODE_AUDIO_GAIN:
		{
			// Convert to Audio Gain incr/decr
			if(pot_diff < 0) {
				if(ts.rx_gain[RX_AUDIO_SPKR].value)
					ts.rx_gain[RX_AUDIO_SPKR].value -= 1;
			} else {
				ts.rx_gain[RX_AUDIO_SPKR].value += 1;
				if(ts.rx_gain[RX_AUDIO_SPKR].value > ts.rx_gain[RX_AUDIO_SPKR].max)
					ts.rx_gain[RX_AUDIO_SPKR].value = ts.rx_gain[RX_AUDIO_SPKR].max;
			}

			UiDriverChangeAfGain(1);
			break;
		}

		// Sidetone gain or compression level
		case ENC_ONE_MODE_ST_GAIN:
		{
			if(ts.dmod_mode == DEMOD_CW)	{	// In CW mode - adjust sidetone gain
				// Convert to Audio Gain incr/decr
				if(pot_diff < 0)
				{
					if(ts.st_gain)
						ts.st_gain -= 1;
				}
				else
				{
					ts.st_gain += 1;
					if(ts.st_gain > SIDETONE_MAX_GAIN)		// limit value to proper range
						ts.st_gain = SIDETONE_MAX_GAIN;
				}
				UiDriverChangeStGain(1);
			}
			else	{		// In voice mode - adjust audio compression level
				// Convert to Audio Gain incr/decr
				if(pot_diff < 0)
				{
					if(ts.tx_comp_level)	// Do not allow setting below 1 from main screen
						ts.tx_comp_level--;
				}
				else
				{
					ts.tx_comp_level++;
					if(ts.tx_comp_level > TX_AUDIO_COMPRESSION_MAX)		// limit value to proper range
						ts.tx_comp_level = TX_AUDIO_COMPRESSION_MAX;
				}
				AudioManagement_CalcTxCompLevel();		// calculate values for selection compression level
				UiDriverChangeCmpLevel(1);	// update on-screen display
			}

			break;
		}

		default:
			break;
		}

		// Updated

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
static void UiDriverCheckEncoderTwo()
{
  //char 	temp[10];
  int 	pot_diff;

  pot_diff = UiDriverEncoderRead(ENC2);

  if (pot_diff) {
    UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing

    // I have taken the freedom to free encoder 2 from filter path
    // in order to free it for the notch frequency adjustment
/*    if (filter_path_change) {
      if(!ts.notch_enabled){
    	AudioFilter_NextApplicableFilterPath(PATH_NEXT_BANDWIDTH | (pot_diff < 0?PATH_DOWN:PATH_UP),AudioFilter_GetFilterModeFromDemodMode(ts.dmod_mode),ts.filter_path);
    	UiInitRxParms();
      }
    } else
  */
    	if(ts.menu_mode)    {
      UiMenu_RenderChangeItem(pot_diff);
    } else {
      if(ts.txrx_mode == TRX_MODE_RX)	{
        //
        // Take appropriate action
        switch(ts.enc_two_mode)
        {
        case ENC_TWO_MODE_RF_GAIN:
        {
          if(ts.dmod_mode != DEMOD_FM)	{	// is this *NOT* FM?  Change RF gain
            // Convert to Audio Gain incr/decr
            if(pot_diff < 0)
            {
              if(ts.rf_gain)
                ts.rf_gain -= 1;
            }
            else
            {
              ts.rf_gain += 1;
              if(ts.rf_gain > MAX_RF_GAIN)
                ts.rf_gain = MAX_RF_GAIN;
            }
            //
            // get RF gain value and calculate new value
            //
            AudioManagement_CalcRFGain();		// convert from user RF gain value to "working" RF gain value
            UiDriverChangeRfGain(1);	// change on screen
            break;
          }
          else	{		// it is FM - change squelch setting
            if(pot_diff < 0)
            {
              if(ts.fm_sql_threshold)
                ts.fm_sql_threshold -= 1;
            }
            else
            {
              ts.fm_sql_threshold += 1;
              if(ts.fm_sql_threshold > FM_SQUELCH_MAX)
                ts.fm_sql_threshold = FM_SQUELCH_MAX;
            }
            //
            // get RF gain value and calculate new value
            //
            UiDriverChangeRfGain(1);	// change on screen
            break;
          }
        }

        // Update DSP/NB setting
        case ENC_TWO_MODE_SIG_PROC:
        {
          if(is_dsp_nb())	{	// is it in noise blanker mode?
            // Convert to NB incr/decr
            if(pot_diff < 0)
            {
              if(ts.nb_setting)
                ts.nb_setting -= 1;
            }
            else
            {
              ts.nb_setting += 1;
              if(ts.nb_setting > MAX_NB_SETTING)
                ts.nb_setting = MAX_NB_SETTING;
            }
          }
          else if(is_dsp_nr())	{	// only allow adjustment if DSP NR is active
            // Convert to NB incr/decr
            if(pot_diff < 0)
            {
              if(ts.dsp_nr_strength)
                ts.dsp_nr_strength -= 1;
            }
            else
            {
              ts.dsp_nr_strength += 1;
              if(ts.dsp_nr_strength > DSP_NR_STRENGTH_MAX)
                ts.dsp_nr_strength = DSP_NR_STRENGTH_MAX;
            }
            audio_driver_set_rx_audio_filter();
          }
          // Signal processor setting
          UiDriverChangeSigProc(1);
          break;
        }
        case ENC_TWO_MODE_NOTCH_F: {
        	if (ts.notch_enabled) { // notch f is only adjustable when notch is enabled
        	if(pot_diff < 0) {
        		ts.notch_frequency = ts.notch_frequency - 10;
        		    	}
        	if(pot_diff > 0) {
        		ts.notch_frequency = ts.notch_frequency + 10;
        	}
        	if (ts.notch_frequency < 200) ts.notch_frequency = 200;
        	if (ts.notch_frequency > 12000) ts.notch_frequency = 12000;
        	// display notch frequency
        	UiDriverDisplayNotch(1);
        	// set notch filter instance
        	audio_driver_set_rx_audio_filter();
        }
        	break;
        }
        case ENC_TWO_MODE_BASS_GAIN: {
        	if(pot_diff < 0) {
        		ts.bass_gain = ts.bass_gain - 1;
        		    	}
        	if(pot_diff > 0) {
        		ts.bass_gain = ts.bass_gain + 1;
        	}
        	if (ts.bass_gain < -20) ts.bass_gain = -20;
        	if (ts.bass_gain > 20) ts.bass_gain = 20;
        	// display bass gain
        	UiDriverDisplayBass();
        	// set filter instance
        	audio_driver_set_rx_audio_filter();
        	break;
        }
        case ENC_TWO_MODE_TREBLE_GAIN: {
        	if(pot_diff < 0) {
        		ts.treble_gain = ts.treble_gain - 1;
        		    	}
        	if(pot_diff > 0) {
        		ts.treble_gain = ts.treble_gain + 1;
        	}
        	if (ts.treble_gain < -20) ts.treble_gain = -20;
        	if (ts.treble_gain > 20) ts.treble_gain = 20;
        	// display treble gain
        	UiDriverDisplayBass();
        	// set filter instance
        	audio_driver_set_rx_audio_filter();
        	break;
        }
        case ENC_TWO_MODE_PEAK_F: {
        	if (ts.peak_enabled) { // peak f is only adjustable when peak is enabled
        	if(pot_diff < 0) {
        		ts.peak_frequency = ts.peak_frequency - 10;
        		    	}
        	if(pot_diff > 0) {
        		ts.peak_frequency = ts.peak_frequency + 10;
        	}
        	if (ts.peak_frequency < 200) ts.peak_frequency = 200;
        	if (ts.peak_frequency > 12000) ts.peak_frequency = 12000;
        	// display peak frequency
        	UiDriverDisplayNotch(1);
        	// set notch filter instance
        	audio_driver_set_rx_audio_filter();
        }
        	break;
        }
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
static void UiDriverCheckEncoderThree()
{
  int 	pot_diff;

  pot_diff = UiDriverEncoderRead(ENC3);

  if (pot_diff) {
    UiDriver_LcdBlankingStartTimer();	// calculate/process LCD blanking timing
    if (filter_path_change) {
      AudioFilter_NextApplicableFilterPath(PATH_ALL_APPLICABLE | (pot_diff < 0?PATH_DOWN:PATH_UP),AudioFilter_GetFilterModeFromDemodMode(ts.dmod_mode),ts.filter_path);
      UiInitRxParms();
    } else  if(ts.menu_mode)	{
      UiMenu_RenderChangeItemValue(pot_diff);
    } else {



      // Take appropriate action
      switch(ts.enc_thr_mode)
      {
      // Update RIT value
      case ENC_THREE_MODE_RIT:
      {
        if(ts.txrx_mode == TRX_MODE_RX)	{
          // Convert to RIT incr/decr
          if(pot_diff < 0)
          {
            ts.rit_value -= 1;
            if(ts.rit_value < -50)
              ts.rit_value = MIN_RIT_VALUE;
          }
          else
          {
            ts.rit_value += 1;
            if(ts.rit_value > 50)
              ts.rit_value = MAX_RIT_VALUE;
          }

          // Update RIT
          UiDriverChangeRit(1);

          // Change frequency
          UiDriver_FrequencyUpdateLOandDisplay(false);
        }
        break;
      }

      // Keyer speed
      case ENC_THREE_MODE_CW_SPEED:
      {
        if(ts.dmod_mode == DEMOD_CW)	{		// in CW mode, adjust keyer speed
          // Convert to Audio Gain incr/decr
          if(pot_diff < 0)
          {
            ts.keyer_speed--;
            if(ts.keyer_speed < MIN_KEYER_SPEED)
              ts.keyer_speed = MIN_KEYER_SPEED;
          }
          else
          {
            ts.keyer_speed++;
            if(ts.keyer_speed > 48)
              ts.keyer_speed = MAX_KEYER_SPEED;
          }

          UiDriverChangeKeyerSpeed(1);
        }
        else	{	// in voice mode, adjust audio gain

          uint16_t gain_max = ts.tx_audio_source == TX_AUDIO_MIC?MIC_GAIN_MAX:LINE_GAIN_MAX;
          uint16_t gain_min = ts.tx_audio_source == TX_AUDIO_MIC?MIC_GAIN_MIN:LINE_GAIN_MIN;

          if(pot_diff < 0)	{						// yes, adjust line gain
            ts.tx_gain[ts.tx_audio_source]--;
            if(ts.tx_gain[ts.tx_audio_source] < gain_min) {
              ts.tx_gain[ts.tx_audio_source] = gain_min;
            }
          }
          else	{
            ts.tx_gain[ts.tx_audio_source]++;
            if(ts.tx_gain[ts.tx_audio_source] > gain_max)
              ts.tx_gain[ts.tx_audio_source] = gain_max;
          }
          if (ts.tx_audio_source == TX_AUDIO_MIC) {
            Codec_MicBoostCheck(ts.txrx_mode);
          }
          UiDriverChangeAudioGain(1);
        }
        break;
      }

      default:
        break;
      }

    }
  }
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeEncoderOneMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeEncoderOneMode(uchar skip)
{
	uchar l_mode;

	if(ts.menu_mode)	// bail out if in menu mode
		return;

	if(!skip)
	{
		ts.enc_one_mode++;
		if(ts.enc_one_mode > ENC_ONE_MAX_MODE)
			ts.enc_one_mode = ENC_ONE_MODE_AUDIO_GAIN;

		l_mode = ts.enc_one_mode;
	}
	else
	{
		ts.enc_one_mode = ENC_ONE_MAX_MODE + 1;
		l_mode 			= 100;
	}

	switch(l_mode)
	{
		case ENC_ONE_MODE_AUDIO_GAIN:
		{
			// Audio gain enabled
			UiDriverChangeAfGain(1);

			// Sidetone disabled
			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeStGain(0);
			else
				UiDriverChangeCmpLevel(0);
			//

			break;
		}

		case ENC_ONE_MODE_ST_GAIN:
		{
			// Audio gain disabled
			UiDriverChangeAfGain(0);

			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeStGain(1);
			else
				UiDriverChangeCmpLevel(1);
			//

			break;
		}

		// Disable all
		default:
		{
			// Audio gain disabled
			UiDriverChangeAfGain(0);

			// Sidetone enabled
			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeStGain(0);
			else
				UiDriverChangeCmpLevel(0);
			//

			break;
		}
	}
}
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeEncoderTwoMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeEncoderTwoMode(uchar skip)
{
	uchar 	l_mode;

	if(ts.menu_mode)	// bail out if in menu mode
		return;

	if(!skip)
	{
		ts.enc_two_mode++;
		// only switch to notch frequency adjustment, if notch enabled!
		if(ts.enc_two_mode == ENC_TWO_MODE_NOTCH_F && !ts.notch_enabled) ts.enc_two_mode++;

		// flip round
		if(ts.enc_two_mode >= ENC_TWO_MAX_MODE)
			ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;

		l_mode = ts.enc_two_mode;
	}
	else
	{
		ts.enc_two_mode = ENC_TWO_MAX_MODE;
		l_mode 			= 100;
	}

	switch(l_mode)
	{
		case ENC_TWO_MODE_RF_GAIN:
		{
			// RF gain
			UiDriverChangeRfGain(1);
			// DSP/Noise Blanker
			UiDriverChangeSigProc(0);
			// notch display
			UiDriverDisplayNotch(0);
			break;
		}

		case ENC_TWO_MODE_SIG_PROC:
		{
			// RF gain
			UiDriverChangeRfGain(0);
			// DSP/Noise Blanker
			UiDriverChangeSigProc(1);
			// notch display
			UiDriverDisplayNotch(0);
			break;
		}

		case ENC_TWO_MODE_NOTCH_F:
		{
			// RF gain
			UiDriverChangeRfGain(0);
			// DSP/Noise Blanker
			UiDriverChangeSigProc(0);
			// notch display
			UiDriverDisplayNotch(1);

			break;
		}
		case ENC_TWO_MODE_PEAK_F:
			{
				// RF gain
				UiDriverChangeRfGain(0);
				// DSP/Noise Blanker
				UiDriverChangeSigProc(0);
				// notch display
				UiDriverDisplayNotch(1);

			break;
		}
		case ENC_TWO_MODE_BASS_GAIN:
			{
				// RF gain
				UiDriverChangeRfGain(0);
				// DSP/Noise Blanker
				UiDriverChangeSigProc(0);
				// notch display
				UiDriverDisplayNotch(0);
				UiDriverDisplayBass();

			break;
		}
		case ENC_TWO_MODE_TREBLE_GAIN:
			{
				// RF gain
				UiDriverChangeRfGain(0);
				// DSP/Noise Blanker
				UiDriverChangeSigProc(0);
				// notch display
				UiDriverDisplayNotch(0);
				UiDriverDisplayBass();
			break;
		}

		// Disable all
		default:
		{
			// RF gain
			UiDriverChangeRfGain(0);

			// DSP/Noise Blanker
			UiDriverChangeSigProc(0);
			UiDriverDisplayNotch(0);
			//UiDriverDisplayBass();
			break;
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeEncoderThreeMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeEncoderThreeMode(uchar skip)
{
	uchar 	l_mode;

	if(ts.menu_mode)	// bail out if in menu mode
		return;

	if(!skip)
	{
		ts.enc_thr_mode++;
		if(ts.enc_thr_mode >= ENC_THREE_MAX_MODE)
			ts.enc_thr_mode = ENC_THREE_MODE_RIT;

		l_mode = ts.enc_thr_mode;
	}
	else
	{
		ts.enc_thr_mode = ENC_THREE_MAX_MODE;
		l_mode 			= 100;
	}

	switch(l_mode)
	{
		case ENC_THREE_MODE_RIT:
		{
			// RIT
			UiDriverChangeRit(1);

			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeKeyerSpeed(0);
			else
				UiDriverChangeAudioGain(0);

			break;
		}

		case ENC_THREE_MODE_CW_SPEED:
		{
			// RIT
			UiDriverChangeRit(0);

			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeKeyerSpeed(1);
			else
				UiDriverChangeAudioGain(1);

			break;
		}

		// Disable all
		default:
		{
			// RIT
			UiDriverChangeRit(0);

			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeKeyerSpeed(0);
			else
				UiDriverChangeAudioGain(0);

			break;
		}
	}
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeAfGain
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeAfGain(uchar enabled)
{
	UiDriverEncoderDisplaySimple(0,0,"AFG", enabled, ts.rx_gain[RX_AUDIO_SPKR].value);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeStGain
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeStGain(uchar enabled)
{
	UiDriverEncoderDisplaySimple(1,0,"STG", enabled, ts.st_gain);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeCmpLevel
//* Object              : Display TX audio compression level
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeCmpLevel(uchar enabled)
{
	ushort 	color = enabled?White:Grey;
	char	temp[5];

	if(ts.tx_comp_level < TX_AUDIO_COMPRESSION_MAX)	{	// 	display numbers for all but the highest value
		sprintf(temp,"%02d",ts.tx_comp_level);
	}
	else	{				// show "SV" (Stored Value) for highest value
		strcpy(temp, "SV");
		color = Yellow;	// Stored value - use yellow
	}

	UiDriverEncoderDisplay(1,0,"CMP" , enabled, temp, color);
}
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeDSPMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeDSPMode()
{
	ushort color = White;
	const char* txt;

	switch (ts.dsp_mode) {
	case DSP_SWITCH_OFF: //
		color = Grey2;
		txt = "DSP-OFF";
		break;
	case DSP_SWITCH_NR:
		txt = "NR";
		color = White;
		break;
	case DSP_SWITCH_NOTCH:
		color = White;
		txt = "NOTCH";
		break;
	case DSP_SWITCH_NR_AND_NOTCH:
		color = White;
		txt = "NR+NOTC";
		break;
	case DSP_SWITCH_NOTCH_MANUAL:
		color = White;
		txt = "M-NOTCH";
		break;
	case DSP_SWITCH_PEAK_FILTER:
		color = White;
		txt = "PEAK";
		break;
	case DSP_SWITCH_BASS:
		color = White;
		txt = "BASS";
		break;
	case DSP_SWITCH_TREBLE:
		color = White;
		txt = "TREBLE";
		break;
	default:
		color = Grey2;
		txt = "DSP-OFF";
		break;
	}

/*	if(((is_dsp_nr()) || (is_dsp_notch()))) {	// DSP active and NOT in FM mode?
		color = White;
	} else	// DSP not active
		color = Grey2;
	if(ts.dmod_mode == DEMOD_FM)	{		// Grey out and display "off" if in FM mode
		txt = "DSP-OFF";
		color = Grey2;
	} else if((is_dsp_nr()) && (is_dsp_notch()) && (ts.dmod_mode != DEMOD_CW))	{
		txt = "NR+NOTC";
	} else if(is_dsp_nr())	{
		txt = "NR";
	} else if(is_dsp_notch())	{
		txt = "NOTCH";
		if(ts.dmod_mode == DEMOD_CW)
			color = Grey2;
	} else {
		txt = "DSP-OFF";
	}
*/
	UiLcdHy28_DrawStraightLine(POS_DSPL_IND_X,(POS_DSPL_IND_Y - 1),UI_LEFT_BOX_WIDTH,LCD_DIR_HORIZONTAL,Blue);
	UiLcdHy28_PrintTextCentered((POS_DSPL_IND_X),(POS_DSPL_IND_Y),UI_LEFT_BOX_WIDTH,txt,color,Blue,0);
}
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeDigitalMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
typedef struct DigitalModeDescriptor_s {
	const char* label;
	const uint32_t enabled;
} DigitalModeDescriptor;


enum {
		Digital = 0,
		FreeDV1,
		FreeDV2,
		BPSK31,
		RTTY,
		SSTV,
		WSPR_A,
		WSPR_P,
		DigitalModeMax
};
// The following descriptor table has to be in the order of the enum above
// This table is stored in flash (due to const) and cannot be written to
// for operational data per mode [r/w], use a different table with order of modes
const DigitalModeDescriptor digimodes[DigitalModeMax] =
{
		{ "DIGITAL", false	},
		{ "FREEDV1", false },
		{ "FREEDV2", false },
		{ "BPSK 31", false },
		{ "RTTY", false },
		{ "SSTV", false },
		{ "WSPR A", false },
		{ "WSPR P", false },
};

static void UiDriverChangeDigitalMode()
{
	ushort color = digimodes[ts.digital_mode].enabled?White:Grey2;
	const char* txt = digimodes[ts.digital_mode].label;

	// Draw line for box
	UiLcdHy28_DrawStraightLine(POS_DSPU_IND_X,(POS_DSPU_IND_Y - 1),UI_LEFT_BOX_WIDTH,LCD_DIR_HORIZONTAL,Blue);
	UiLcdHy28_PrintTextCentered((POS_DSPU_IND_X),(POS_DSPU_IND_Y),UI_LEFT_BOX_WIDTH,txt,color,Blue,0);
}
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangePowerLevel
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
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
    // Set TX power factor - to reflect changed power
    UiDriverSetBandPowerFactor(ts.band);

    // Draw top line
    UiLcdHy28_DrawStraightLine(POS_PW_IND_X,(POS_PW_IND_Y - 1),UI_LEFT_BOX_WIDTH,LCD_DIR_HORIZONTAL,Blue);
	UiLcdHy28_PrintTextCentered((POS_PW_IND_X),(POS_PW_IND_Y),UI_LEFT_BOX_WIDTH,txt,color,Blue,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeKeyerSpeed
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeKeyerSpeed(uchar enabled)
{
	ushort 	color = enabled?White:Grey;
	const char* txt;
	char  txt_buf[5];

	if(enabled)
		color = White;

	txt = "WPM";
	sprintf(txt_buf,"%2d",ts.keyer_speed);

	UiDriverEncoderDisplay(1,2,txt, enabled, txt_buf, color);

	// Update CW gen module
	if(enabled)
		cw_set_speed();
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeAudioGain
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeAudioGain(uchar enabled)
{
	ushort 	color = enabled?White:Grey;
	const char* txt;
	char  txt_buf[5];

	switch (ts.tx_audio_source) {
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
		break;
	case TX_AUDIO_DIGIQ:
		txt = "DIQ";
		break;
	default:
		txt = "???";
	}

	sprintf(txt_buf,"%2d",ts.tx_gain[ts.tx_audio_source]);

	UiDriverEncoderDisplay(1,2,txt, enabled, txt_buf, color);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeRfGain
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeRfGain(uchar enabled)
{
	uint32_t color = enabled?White:Grey;

	char	temp[5];
	const char* label = ts.dmod_mode==DEMOD_FM?"SQL":"RFG";
	int32_t value;


	if(ts.dmod_mode != DEMOD_FM)	{	// If not FM, use RF gain
		if(enabled)	{
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
	} else	{						// it is FM, display squelch instead
		value = ts.fm_sql_threshold;
	}

	sprintf(temp," %02ld",value);

	UiDriverEncoderDisplay(0,1,label, enabled, temp, color);

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeSigProc
//* Object              : Display settings related to signal processing - DSP NR or Noise Blanker strength
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeSigProc(uchar enabled)
{
	uint32_t 	color = enabled?White:Grey;
	char	temp[5];
	const char* label;
	int32_t value;

	//
	// Noise blanker settings display
	//
	if(is_dsp_nb())	{	// is noise blanker to be displayed
		if(enabled)	{
			if(ts.nb_setting >= NB_WARNING3_SETTING)
				color = Red;		// above this value, make it red
			else if(ts.nb_setting >= NB_WARNING2_SETTING)
				color = Orange;		// above this value, make it orange
			else if(ts.nb_setting >= NB_WARNING1_SETTING)
				color = Yellow;		// above this value, make it yellow
			else
				color = White;		// Otherwise, make it white
		}
		label = "NB ";
		value = ts.nb_setting;
	}
	//
	// DSP settings display
	//
	else	{			// DSP settings are to be displayed
		if(enabled && (is_dsp_nr()))	{	// if this menu is enabled AND the DSP NR is also enabled...
			color = White;		// Make it white by default
			//
			if(ts.dsp_nr_strength >= DSP_STRENGTH_RED)
				color = Red;
			else if(ts.dsp_nr_strength >= DSP_STRENGTH_ORANGE)
				color = Orange;
			else if(ts.dsp_nr_strength >= DSP_STRENGTH_YELLOW)
				color = Yellow;
		}
		label = "DSP";
		value = ts.dsp_nr_strength;
	}

	//
	// display numerical value
	//
	sprintf(temp,"%02ld",value);

	UiDriverEncoderDisplay(1,1,label, enabled, temp, color);
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDisplayBass
//* Object              : Display settings related to bass & treble filter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverDisplayBass(void) {

	UiLcdHy28_DrawFullRect(POS_AG_IND_X, POS_AG_IND_Y + 3 * 16, 16, 112, Black);

//	ts.enc_two_mode == ENC_TWO_MODE_TREBLE_GAIN;

	bool enable = (ts.enc_two_mode == ENC_TWO_MODE_BASS_GAIN);
	uint32_t col_bass = enable?Black:Grey1;

	char temp[5];
			snprintf(temp,5,"%2d", ts.bass_gain);

			UiLcdHy28_DrawEmptyRect(POS_AG_IND_X, POS_AG_IND_Y + 3 * 16, 13, 53, Grey);
			UiLcdHy28_PrintText((POS_AG_IND_X + 1 ), (POS_AG_IND_Y + 1 + 3 * 16), "BAS",
		                      col_bass, Grey, 0);
			col_bass = enable?Orange:Grey1;
			UiLcdHy28_PrintTextRight((POS_AG_IND_X + 52), (POS_AG_IND_Y + 1 + 3 * 16), temp,
		                           col_bass, Black, 0);

			enable = (ts.enc_two_mode == ENC_TWO_MODE_TREBLE_GAIN);
			uint32_t col_treble = enable?Black:Grey1;

		  snprintf(temp,5,"%2d", ts.treble_gain);

		  UiLcdHy28_DrawEmptyRect(POS_AG_IND_X + 56, POS_AG_IND_Y + 3* 16, 13, 53, Grey);
		  UiLcdHy28_PrintText((POS_AG_IND_X + 1 + 56), (POS_AG_IND_Y + 1 + 3 * 16), "TRB",
		                      col_treble, Grey, 0);
		  col_treble = enable?Orange:Grey1;

		  UiLcdHy28_PrintTextRight((POS_AG_IND_X + 52 + 56), (POS_AG_IND_Y + 1 + 3 * 16), temp,
		                           col_treble, Black, 0);

/*
		  uint32_t label_color = enabled?Black:Grey1;
	  UiLcdHy28_DrawEmptyRect(POS_AG_IND_X, POS_AG_IND_Y + 3 * 16, 13, 53, Grey);

	  if (ts.notch_enabled)
	  UiLcdHy28_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1 + 3 * 16), "NOTCH ",
	                      label_color, Grey, 0);
	  else
	  UiLcdHy28_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1 + 3 * 16), "PEAK-F",
	                      label_color, Grey, 0);

	  UiLcdHy28_DrawFullRect(POS_AG_IND_X + 47, POS_AG_IND_Y + 3 * 16, 13, 7, Grey);

	  UiLcdHy28_DrawEmptyRect(POS_AG_IND_X + 56, POS_AG_IND_Y + 3 * 16, 13, 53, Grey);

	  char temp[6];
	  uint32_t color = enabled?White:Grey;
	  if(ts.notch_enabled || ts.peak_enabled) color = Yellow;
	  if (ts.notch_enabled)
	  snprintf(temp,6,"%5lu", (ulong)ts.notch_frequency);
	  else 	  snprintf(temp,6,"%5lu", (ulong)ts.peak_frequency);
	  UiLcdHy28_PrintTextRight((POS_AG_IND_X + 52 + 56), (POS_AG_IND_Y + 1 + 3 * 16), temp,
	  	                           color, Black, 0); */


} // end void UiDriverDisplayBass


/*##################
void UiDriverEncoderDisplay(const uint8_t column, const uint8_t row, const char *label, bool enabled,
                            char temp[5], uint32_t color) {

  uint32_t label_color = enabled?Black:Grey1;

  UiLcdHy28_DrawEmptyRect(POS_AG_IND_X + 56 * column, POS_AG_IND_Y + row * 16, 13, 53, Grey);
  UiLcdHy28_PrintText((POS_AG_IND_X + 1 + 56 * column), (POS_AG_IND_Y + 1 + row * 16), label,
                      label_color, Grey, 0);
  UiLcdHy28_PrintTextRight((POS_AG_IND_X + 52 + 56 * column), (POS_AG_IND_Y + 1 + row * 16), temp,
                           color, Black, 0);
}

void UiDriverEncoderDisplaySimple(const uint8_t column, const uint8_t row, const char *label, bool enabled,
                            uint32_t value) {

	char temp[5];
	uint32_t color = enabled?White:Grey;

	snprintf(temp,5,"%2lu",value);
	UiDriverEncoderDisplay(column, row, label, enabled,
	                            temp, color);

##################*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDisplayNotch
//* Object              : Display settings related to manual notch filter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverDisplayNotch(uchar enabled) {

	if(enabled || ts.notch_enabled || ts.peak_enabled)
	  {
	  UiLcdHy28_DrawFullRect(POS_AG_IND_X, POS_AG_IND_Y + 3 * 16, 16, 112, Black);
	  uint32_t label_color = enabled?Black:Grey1;
	  UiLcdHy28_DrawEmptyRect(POS_AG_IND_X, POS_AG_IND_Y + 3 * 16, 13, 53, Grey);

	  if (ts.notch_enabled)
	  UiLcdHy28_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1 + 3 * 16), "NOTCH ",
	                      label_color, Grey, 0);
	  else
	  UiLcdHy28_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1 + 3 * 16), "PEAK-F",
	                      label_color, Grey, 0);

	  UiLcdHy28_DrawFullRect(POS_AG_IND_X + 47, POS_AG_IND_Y + 3 * 16, 13, 7, Grey);

	  UiLcdHy28_DrawEmptyRect(POS_AG_IND_X + 56, POS_AG_IND_Y + 3 * 16, 13, 53, Grey);

	  char temp[6];
	  uint32_t color = enabled?White:Grey;
	  if(ts.notch_enabled || ts.peak_enabled) color = Yellow;
	  if (ts.notch_enabled)
	  snprintf(temp,6,"%5lu", (ulong)ts.notch_frequency);
	  else 	  snprintf(temp,6,"%5lu", (ulong)ts.peak_frequency);
	  UiLcdHy28_PrintTextRight((POS_AG_IND_X + 52 + 56), (POS_AG_IND_Y + 1 + 3 * 16), temp,
	  	                           color, Black, 0);
	  }
	else
	  UiLcdHy28_DrawFullRect(POS_AG_IND_X, POS_AG_IND_Y + 3 * 16, 16, 112, Black);

} // end void UiDriverDisplayNotch

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeRit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeRit(uchar enabled)
{
	char	temp[5];
	uint32_t color;
	if(ts.rit_value)
	    color = Green;
	else
	    color = enabled?White:Grey;

	if(ts.rit_value)
	    sprintf(temp,"%+3i", ts.rit_value);
	else
	    sprintf(temp,"%3i", ts.rit_value);

    UiDriverEncoderDisplay(0,2,"RIT", enabled, temp, color);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeFilterDisplay
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeFilterDisplay()
{
	// Do a filter re-load
//	if(!ui_only_update) {
//		audio_driver_set_rx_audio_filter();
//	}
	const char* filter_ptr;
	uint32_t bg_clr = Blue;
	uint32_t font_clr = White;

	  {
	    const char *filter_names[2];

	    bg_clr = filter_path_change?Orange:Blue;
	    font_clr= filter_path_change?Black:White;

	    AudioFilter_GetNamesOfFilterPath(ts.filter_path,filter_names);
	    UiLcdHy28_PrintTextCentered(POS_FIR_IND_X,POS_FIR_IND_Y,UI_LEFT_BOX_WIDTH,filter_names[0],font_clr,bg_clr,0);
	    if (filter_names[1] != NULL) {
	      filter_ptr = filter_names[1];
	    } else {
	      filter_ptr = " ";
	    }
	}
	// Draw top line
    UiLcdHy28_DrawStraightLine(POS_FIR_IND_X,(POS_FIR_IND_Y - 1),UI_LEFT_BOX_WIDTH,LCD_DIR_HORIZONTAL,bg_clr);
    UiLcdHy28_PrintTextCentered(POS_FIR_IND_X,POS_FIR_IND_Y+12,UI_LEFT_BOX_WIDTH,filter_ptr,font_clr,bg_clr,0);

}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDisplayFilterBW
//* Object              : Display/Update line under the Waterfall or Spectrum that graphically indicates filter bandwidth and relative position
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiDriverDisplayFilterBW()
{
	float	width, offset, calc;
	ushort	lpos;
	bool	is_usb;
	uint32_t clr;

	if(ts.menu_mode)	// bail out if in menu mode
		return;



	// Update screen indicator - first get the width and center-frequency offset of the currently-selected filter
	//
	  const FilterPathDescriptor* path_p = &FilterPathInfo[ts.filter_path];
	  const FilterDescriptor* filter_p = &FilterInfo[path_p->id];
	  offset = path_p->offset;
	  width = filter_p->width;

	  if (offset == 0) {
	  offset = width/2;
	}

/*	  //
	// Special case for FM
	// --> we do not need a special case any longer, as bandwidths are treated the same for all bandwidths in audio_filter.c
	if(ts.dmod_mode == DEMOD_FM)	{
		if(ts.fm_rx_bandwidth == FM_RX_BANDWIDTH_7K2)	{
			offset = HILBERT3600;											// display bandwidth of +/-3.6 kHz = 7.2 kHz
			width = HILBERT_3600HZ_WIDTH;
		}
		else if(ts.fm_rx_bandwidth == FM_RX_BANDWIDTH_12K)	{
			offset = 3000;												// display bandwidth of +/- 6 kHz = 12 kHz
			width = 6000;
		}
//		else if(ts.fm_rx_bandwidth == FM_RX_BANDWIDTH_15K)	{
//			offset = FILT7500;												// display bandwidth of +/- 7.5 kHz = 15 kHz
//			width = FILTER_7500HZ_WIDTH;
//		}
		else	{			// this will be the 10 kHz BW mode - I hope!
			offset = 2500;												// display bandwidth of +/- 5 kHz = 10 kHz
			width = 5000;
		}
	}
	*/
	//
	//
	switch(ts.dmod_mode)	{	// determine if the receiver is set to LSB or USB or FM
		case DEMOD_LSB:
			is_usb = 0;		// it is LSB
			break;
		case DEMOD_CW:
			if(!ts.cw_lsb)	// is this USB RX mode?  (LSB of mode byte was zero)
				is_usb = 1;	// it is USB
			else	// No, it is LSB RX mode
				is_usb = 0;	// it is LSB
			break;
		case DEMOD_USB:
		case DEMOD_DIGI:
		default:
			is_usb = 1;		// it is USB
			break;
	}
	//
	if(!sd.magnify)	{	// is magnify mode on?
		calc = 48000/FILT_DISPLAY_WIDTH;		// magnify mode not on - calculate number of Hz/pixel
		if(ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ)			// line is to left if in "RX LO HIGH" mode
			lpos = 98;
		else if(ts.iq_freq_mode == FREQ_IQ_CONV_M6KHZ)			// line is to right if in "RX LO LOW" mode
			lpos = 162;
		else if(ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ)			// line is to left if in "RX LO LOW" mode
			lpos = 66;
		else if(ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ)			// line is to right if in "RX LO LOW" mode
			lpos = 194;
		else					// frequency translate mode is off
			lpos = 130;			// line is in center

	}
	else	{	// magnify mode is on
		calc = 24000/FILT_DISPLAY_WIDTH;		// magnify mode is on
		lpos = 130;								// line is alway in center in "magnify" mode
	}
	//
	offset /= calc;							// calculate filter center frequency offset in pixels
	width /= calc;							// calculate width of line in pixels
	//
	//
	if((ts.dmod_mode == DEMOD_AM) ||(ts.dmod_mode == DEMOD_SAM) || (ts.dmod_mode == DEMOD_FM))	{	// special cases - AM, SAM and FM, which are double-sidebanded
		lpos -= width;					// line starts "width" below center
		width *= 2;						// the width is double in AM & SAM, above and below center
	}
	else if(!is_usb)	// not AM, but LSB:  calculate position of line, compensating for both width and the fact that SSB/CW filters are not centered
		lpos -= ((offset - (width/2)) + width);	// if LSB it will be below zero Hz
	else				// USB mode
		lpos += (offset - (width/2));			// if USB it will be above zero Hz

	// get color for line
	UiMenu_MapColors(ts.filter_disp_colour,NULL, &clr);
	//	erase old line by clearing whole area
	UiLcdHy28_DrawStraightLineDouble((POS_SPECTRUM_IND_X), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y), 256, LCD_DIR_HORIZONTAL, Black);
	// draw line
	UiLcdHy28_DrawStraightLineDouble((POS_SPECTRUM_IND_X + lpos), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y), (ushort)width, LCD_DIR_HORIZONTAL, clr);

}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverFFTWindowFunction
//* Object              : Do windowing functions for both the Spectrum Scope and Waterfall Displays
//* Input Parameters    : mode - select window function
//* Input Parameters    : array "sd.FFT_Samples" - input from A/D converter after gain adjustment
//* Output Parameters   : array "sd.FFT_Windat" - processed output to RFFT
//* Functions called    : none
//*----------------------------------------------------------------------------


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateUsbKeyboardStatus
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverUpdateUsbKeyboardStatus()
{
	// No change, nothing to process
	if(kbs.new_state == kbs.old_state)
		return;

	switch(kbs.new_state)
	{
		// Nothing connected
		case 0:
			UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"KBD",Grey,Black,0);
			break;

		// Some device attached
		case 1:
			UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"DEV",Grey,Black,0);
			break;

		// Keyboard detected
		case 2:
			UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"KBD",Blue,Black,0);
			break;

		default:
			break;
	}

	// Set as done
	kbs.old_state = kbs.new_state;
}*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleSmeter
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandleSmeter()
{
	uchar 	val;
	float 	rfg_calc;
	float gcalc;
	static bool 		clip_indicate = 0;
	static	float		auto_rfg = 8;
	static	uint16_t 	rfg_timer= 0;	// counter used for timing RFG control decay
//	char temp[10];
	//

	// Only in RX mode
	if(ts.txrx_mode != TRX_MODE_RX)
		return;

	sm.skip++;
	if(sm.skip < S_MET_UPD_SKIP)
		return;

	sm.skip = 0;

	// ************************
	// Update S-Meter and control the input gain of the codec to maximize A/D and receiver dynamic range
	// ************************
	//
	// Calculate attenuation of "RF Codec Gain" setting so that S-meter reading can be compensated.
	// for input RF attenuation setting
	//
	if(ts.rf_codec_gain == 9)		// Is RF gain in "AUTO" mode?
		rfg_calc = auto_rfg;
	else	{				// not in "AUTO" mode
		rfg_calc = (float)ts.rf_codec_gain;		// get copy of RF gain setting
		auto_rfg = rfg_calc;		// keep "auto" variable updated with manual setting when in manual mode
		rfg_timer = 0;
	}
	//
	rfg_calc += 1;	// offset to prevent zero
	rfg_calc *= 2;	// double the range of adjustment
	rfg_calc += 13;	// offset, as bottom of range of A/D gain control is not useful (e.g. ADC saturates before RX hardware)
	if(rfg_calc >31)	// limit calc to hardware range
		rfg_calc = 31;
	Codec_Line_Gain_Adj((uchar)rfg_calc);	// set the RX gain on the codec
	//
	// Now calculate the RF gain setting
	//
	gcalc = (float)rfg_calc;
	gcalc *= 1.5;	// codec has 1.5 dB/step
	gcalc -= 34.5;	// offset codec setting by 34.5db (full gain = 12dB)
	gcalc = pow10(gcalc/10);	// convert to power ratio
	ads.codec_gain_calc = sqrtf(gcalc);		// convert to voltage ratio - we now have current A/D (codec) gain setting
	//
	sm.gain_calc = ads.agc_val;		// get AGC loop gain setting
	sm.gain_calc /= AGC_GAIN_CAL;	// divide by AGC gain calibration factor
	//
	sm.gain_calc = 1/sm.gain_calc;	// invert gain to convert to amount of attenuation
	//
	sm.gain_calc /= ads.codec_gain_calc;	// divide by known A/D gain setting

	sm.s_count = 0;		// Init S-meter search counter
	//
	while ((sm.gain_calc >= S_Meter_Cal[sm.s_count]) && (sm.s_count < S_Meter_Cal_Size))	{	// find corresponding signal level
		sm.s_count++;
	}
	val = (uchar)sm.s_count;

	if(!val)	// make sure that the S meter always reads something!
		val = 1;
	//
	UiDriverUpdateTopMeterA(val);
	//
	// Now handle automatic A/D input gain control timing
	//
	rfg_timer++;	// bump RFG timer
	if(rfg_timer > 10000)	// limit count of RFG timer
		rfg_timer = 10000;
	//
	if(ads.adc_half_clip)	{	// did clipping almost occur?
		if(rfg_timer >=	AUTO_RFG_DECREASE_LOCKOUT)	{	// has enough time passed since the last gain decrease?
			if(auto_rfg)	{	// yes - is this NOT zero?
				auto_rfg -= 0.5;	// decrease gain one step, 1.5dB (it is multiplied by 2, above)
				//sprintf(temp, " %d ", auto_rfg);		// Display auto RFG for debug
				//UiLcdHy28_PrintText((POS_BG_IND_X + 82),(POS_BG_IND_Y + 1), temp,White,Black,0);
				rfg_timer = 0;	// reset the adjustment timer
			}
		}
	}
	else if(!ads.adc_quarter_clip)	{	// no clipping occurred
		if(rfg_timer >= AUTO_RFG_INCREASE_TIMER)	{	// has it been long enough since the last increase?
			auto_rfg += 0.5;	// increase gain by one step, 1.5dB (it is multiplied by 2, above)
			rfg_timer = 0;	// reset the timer to prevent this from executing too often
			if(auto_rfg > 8)	// limit it to 8
				auto_rfg = 8;
				//sprintf(temp, " %d ", auto_rfg);		// Display auto RFG for debug
				//UiLcdHy28_PrintText((POS_BG_IND_X + 82),(POS_BG_IND_Y + 1), temp,White,Black,0);
		}
	}
	ads.adc_half_clip = 0;		// clear "half clip" indicator that tells us that we should decrease gain
	ads.adc_quarter_clip = 0;	// clear indicator that, if not triggered, indicates that we can increase gain
	//
	// This makes a portion of the S-meter go red if A/D clipping occurs
	//
	if(ads.adc_clip)	{		// did clipping occur?
		if(!clip_indicate)	{	// have we seen it clip before?
			UiDriverDrawSMeter(Red);		// No, make the first portion of the S-meter red to indicate A/D overload
			clip_indicate = 1;		// set flag indicating that we saw clipping and changed the screen (prevent continuous redraw)
		}
		ads.adc_clip = 0;		// reset clip detect flag
	}
	else	{		// clipping NOT occur?
		if(clip_indicate)	{	// had clipping occurred since we last visited this code?
			UiDriverDrawSMeter(White);					// yes - restore the S meter to a white condition
			clip_indicate = 0;							// clear the flag that indicated that clipping had occurred
		}
	}
}

void UiDriver_PowerFromADCValue(float val, float sensor_null, float coupling_calc,volatile float* pwr_ptr, volatile float* dbm_ptr) {
  float pwr;
  float dbm;
  val *= SWR_ADC_VOLT_REFERENCE;    // get nominal A/D reference voltage
  val /= SWR_ADC_FULL_SCALE;        // divide by full-scale A/D count to yield actual input voltage from detector
  val += sensor_null;               // offset result

  if(val <= LOW_POWER_CALC_THRESHOLD) {   // is this low power as evidenced by low voltage from the sensor?
        pwr = LOW_RF_PWR_COEFF_A + (LOW_RF_PWR_COEFF_B * val) + (LOW_RF_PWR_COEFF_C * pow(val,2 )) + (LOW_RF_PWR_COEFF_D * pow(val, 3));
  } else {        // it is high power
        pwr = HIGH_RF_PWR_COEFF_A + (HIGH_RF_PWR_COEFF_B * val) + (HIGH_RF_PWR_COEFF_C * pow(val, 2));
  }
  // calculate forward and reverse RF power in watts (p = a + bx + cx^2) for high power (above 50-60

  if(pwr < 0) {   // prevent negative power readings from emerging from the equations - particularly at zero output power
        pwr = 0;
  }

  dbm = (10 * (log10(pwr))) + 30 + coupling_calc;
  pwr = pow10(dbm/10)/1000;
  *pwr_ptr = pwr;
  *dbm_ptr = dbm;
}

static bool UiDriver_UpdatePowerAndVSWR() {

  uint16_t  val_p,val_s = 0;
  float sensor_null, coupling_calc;
  bool retval = false;

  swrm.skip++;
  if(swrm.skip >= SWR_SAMPLES_SKP) {
    swrm.skip = 0;

    // Collect samples
    if(swrm.p_curr < SWR_SAMPLES_CNT)
    {
      // Get next sample
      if(!(ts.flags1 & FLAGS1_SWAP_FWDREV_SENSE))   {   // is bit NOT set?  If this is so, do NOT swap FWD/REV inputs from power detectors
        val_p = ADC_GetConversionValue(ADC2); // forward
        val_s = ADC_GetConversionValue(ADC3); // return
      }
      else    {   // FWD/REV bits should be swapped
        val_p = ADC_GetConversionValue(ADC3); // forward
        val_s = ADC_GetConversionValue(ADC2); // return
      }

      // Add to accumulator to average A/D values
      swrm.fwd_calc += (float)val_p;
      swrm.rev_calc += (float)val_s;

      swrm.p_curr++;
    } else {
      // obtain and calculate power meter coupling coefficients
      coupling_calc = swrm.coupling_calc[ts.filter_band];
      coupling_calc -= 100;                       // offset to zero
      coupling_calc /= 10;                        // rescale to 0.1 dB/unit


      sensor_null = (float)swrm.sensor_null;  // get calibration factor
      sensor_null -= 100;                     // offset it so that 100 = 0
      sensor_null /= 1000;                    // divide so that each step = 1 millivolt

      // Compute average values

      swrm.fwd_calc /= SWR_SAMPLES_CNT;
      swrm.rev_calc /= SWR_SAMPLES_CNT;

      UiDriver_PowerFromADCValue(swrm.fwd_calc, sensor_null, coupling_calc,&swrm.fwd_pwr, &swrm.fwd_dbm);
      UiDriver_PowerFromADCValue(swrm.rev_calc, sensor_null, coupling_calc,&swrm.rev_pwr, &swrm.rev_dbm);

      // Reset accumulators and variables for power measurements
      swrm.p_curr   = 0;
      swrm.fwd_calc = 0;
      swrm.rev_calc = 0;


      swrm.vswr = swrm.fwd_dbm-swrm.rev_dbm;      // calculate VSWR

      // Calculate VSWR from power readings

      swrm.vswr = (1+sqrtf(swrm.rev_pwr/swrm.fwd_pwr))/(1-sqrt(swrm.rev_pwr/swrm.fwd_pwr));
      retval = true;
    }
  }
  return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleLowerMeter
//* Object              : Power, SWR, ALC and Audio indicator
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandleLowerMeter()
{
  float	scale_calc;
  char txt[32];
  static float fwd_pwr_avg, rev_pwr_avg;
  static uchar	old_power_level = 99;

  // Only in TX mode
  if(ts.txrx_mode != TRX_MODE_TX)	{
    swrm.vswr_dampened = 0;		// reset averaged readings when not in TX mode
    fwd_pwr_avg = -1;
    rev_pwr_avg = -1;
  } else if (UiDriver_UpdatePowerAndVSWR()) {
    // FIXME: SCALCULATION ENDS HERE, NOW WE DO DISPLAY, SEPARATE

    // display FWD, REV power, in milliwatts - used for calibration - IF ENABLED
    if(swrm.pwr_meter_disp)	{
      if((fwd_pwr_avg < 0) || (ts.power_level != old_power_level)) {	// initialize with current value if it was zero (e.g. reset) or power level changed
        fwd_pwr_avg = swrm.fwd_pwr;
      }

      fwd_pwr_avg = fwd_pwr_avg * (1-PWR_DAMPENING_FACTOR);	// apply IIR smoothing to forward power reading
      fwd_pwr_avg += swrm.fwd_pwr * PWR_DAMPENING_FACTOR;

      if((rev_pwr_avg < 0) || (ts.power_level != old_power_level)) {	// initialize with current value if it was zero (e.g. reset) or power level changed
        rev_pwr_avg = swrm.rev_pwr;
      }

      old_power_level = ts.power_level;		// update power level change detector

      rev_pwr_avg = rev_pwr_avg * (1-PWR_DAMPENING_FACTOR);	// apply IIR smoothing to reverse power reading
      rev_pwr_avg += swrm.rev_pwr * PWR_DAMPENING_FACTOR;

      sprintf(txt, "%d,%d   ", (int)(fwd_pwr_avg*1000), (int)(rev_pwr_avg*1000));		// scale to display power in milliwatts
      UiLcdHy28_PrintText    (POS_PWR_NUM_IND_X, POS_PWR_NUM_IND_Y,txt,Grey,Black,0);
      swrm.pwr_meter_was_disp = 1;	// indicate the power meter WAS displayed
    }

    if((swrm.pwr_meter_was_disp) && (!swrm.pwr_meter_disp))	{	// had the numerical display been enabled - and it is now disabled?
      UiLcdHy28_PrintText    (POS_PWR_NUM_IND_X, POS_PWR_NUM_IND_Y,"            ",White,Black,0);	// yes - overwrite location of numerical power meter display to blank it
      swrm.pwr_meter_was_disp = 0;	// clear flag so we don't do this again
    }


    // calculate and display RF power reading

    scale_calc = (uchar)(swrm.fwd_pwr * 3);		// 3 dots-per-watt for RF power meter

    UiDriverUpdateTopMeterA(scale_calc);

    // Do selectable meter readings

    if(ts.tx_meter_mode == METER_SWR)	{
      if(swrm.fwd_pwr >= SWR_MIN_CALC_POWER)	{		// is the forward power high enough for valid VSWR calculation?
        // (Do nothing/freeze old data if below this power level)
        if(swrm.vswr_dampened < 1)	// initialize averaging if this is the first time (e.g. VSWR <1 = just returned from RX)
          swrm.vswr_dampened = swrm.vswr;
        else {
          swrm.vswr_dampened = swrm.vswr_dampened * (1 - VSWR_DAMPENING_FACTOR);
          swrm.vswr_dampened += swrm.vswr * VSWR_DAMPENING_FACTOR;
        }
        //
        scale_calc = (uchar)(swrm.vswr_dampened * 4);		// yes - four dots per unit of VSWR
        UiDriverUpdateBtmMeter((uchar)(scale_calc), 13);	// update the meter, setting the "red" threshold
      }
    }
    else if(ts.tx_meter_mode == METER_ALC)	{
      scale_calc = ads.alc_val;		// get TX ALC value
      scale_calc *= scale_calc;		// square the value
      scale_calc = log10f(scale_calc);	// get the log10
      scale_calc *= -10;		// convert it to DeciBels and switch sign and then scale it for the meter
      if(scale_calc < 0) {
        scale_calc = 0;
      }
      //
      UiDriverUpdateBtmMeter((uchar)(scale_calc), 13);	// update the meter, setting the "red" threshold
    }
    else if(ts.tx_meter_mode == METER_AUDIO)	{
      scale_calc = ads.peak_audio/10000;		// get a copy of the peak TX audio (maximum reference = 30000)
      ads.peak_audio = 0;					// reset the peak detect
      scale_calc *= scale_calc;			// square the value
      scale_calc = log10f(scale_calc);	// get the log10
      scale_calc *= 10;					// convert to DeciBels and scale for the meter
      scale_calc += 11;					// offset for meter
      //
      if(scale_calc < 0) {
        scale_calc = 0;
      }
      //
      UiDriverUpdateBtmMeter((uchar)(scale_calc), 22);	// update the meter, setting the "red" threshold
    }
  }
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandlePowerSupply
//* Object              : display external voltage and to handle final power-off and delay
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandlePowerSupply()
{
	ulong	val_p, calib;
	int		col;

//	char txt[32];

	static ulong	powerdown_delay = 0;

	if(ts.powering_down)	{	// are we powering down?
		powerdown_delay++;		// yes - do the powerdown delay
		if(powerdown_delay > POWERDOWN_DELAY_COUNT)	{	// is it time to power down
			POWER_DOWN_PIO->BSRRL = POWER_DOWN;			// yes - kill the power
			powerdown_delay = POWERDOWN_DELAY_COUNT;	// limit count if power button is being held down/stuck for a while
		}
	}

	if(ts.boot_halt_flag)		// bail out now if we are in "boot halt" mode
		return;

	pwmt.skip++;
	if(pwmt.skip < POWER_SAMPLES_SKP)
		return;

	pwmt.skip = 0;

	// Collect samples
	if(pwmt.p_curr < POWER_SAMPLES_CNT)
	{
		val_p = ADC_GetConversionValue(ADC1);

		// Add to accumulator
		pwmt.pwr_aver = pwmt.pwr_aver + val_p;
		pwmt.p_curr++;

		return;
	}


	// Get average
	val_p  = pwmt.pwr_aver/POWER_SAMPLES_CNT;

	calib = (ulong)ts.voltmeter_calibrate;	// get local copy of calibration factor
	calib += 900;					// offset to 1000 (nominal)
	val_p = (calib) * val_p;		// multiply by calibration factor, sample count and A/D scale full scale count
	val_p /= (1000);				// divide by 1000 (unity calibration factor), sample count and A/D full scale count

	// Correct for divider
	//val_p -= 550;
	val_p *= 4;

	// Reset accumulator
    pwmt.p_curr     = 0;
    pwmt.pwr_aver   = 0;



	//
	col = COL_PWR_IND;	// Assume normal voltage, so Set normal color
	//
	if(val_p < 9500)		// below 9.5 volts
		col = Red;			// display red digits
	else if(val_p < 10500)	// below 10.5 volts
		col = Orange;		// make them orange
	else if(val_p < 11000)	// below 11.0 volts
		col = Yellow;		// make them yellow

	//
	// did we detect a voltage change?
	//
	if(pwmt.voltage != val_p)	{	// Time to update - or was this the first time it was called?
		char digits[6];
		val_p /= 10;
		snprintf(digits,6,"%2ld.%02ld",val_p/100,val_p%100);
		UiLcdHy28_PrintText(POS_PWR_IND_X,POS_PWR_IND_Y,digits,col,Black,0);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateLoMeter
//* Object              : LO temperature compensation indicator
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
// FIXME: DO NOT REDRAW UNCHANGED BOXES
static void UiDriverUpdateLoMeter(uchar val,uchar active)
{
  if (val < 26) {
    //  Only redraw if inside of the range

    uchar 	i,v_s = 3;
    int		clr = White;

    // Draw first indicator
    for(i = 1; i < 26; i++)
    {
      if (active) {
          clr = val==i?Blue:White;
      } else {
        clr = Grey;
      }
      UiLcdHy28_DrawStraightLineTriple(((POS_TEMP_IND_X + 1) + i*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,clr);
    }
  }
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateTemperatureDisplay
//* Object              : draw ui
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverCreateTemperatureDisplay(uchar enabled,uchar create)
{
    const char *label, *txt, *value_str = NULL;
    uint32_t label_color, txt_color;

    label = "TCXO ";
    label_color = Black;
    txt = "*";
    txt_color = enabled?Red:Grey;

    if(create)
	{
		// Top part - name and temperature display
		UiLcdHy28_DrawEmptyRect( POS_TEMP_IND_X,POS_TEMP_IND_Y,14,109,Grey);

		// LO tracking indicator
		UiLcdHy28_DrawEmptyRect( POS_TEMP_IND_X,POS_TEMP_IND_Y + 14,10,109,Grey);
		// Temperature - initial draw
		value_str = (df.temp_enabled & 0xf0)?"  --.-F":"  --.-C";
	}

    if((df.temp_enabled & 0x0f) == TCXO_STOP)	{	// if temperature update is disabled, don't update display!
		txt = " ";
		value_str = "STOPPED";
	}

	// Label
	UiLcdHy28_PrintText((POS_TEMP_IND_X + 1), (POS_TEMP_IND_Y + 1),label,label_color,Grey,0);
	// Lock Indicator
	UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1), txt,txt_color,Black,0);	// show/delete asterisk
	// Show Initial Temp Value or "STOPPED"
	if (value_str) {
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50),(POS_TEMP_IND_Y + 1), value_str,Grey,Black,0);
	}
	// Meter
	UiDriverUpdateLoMeter(13,enabled);
}

// FIXME: This can be simplified, see FreqDisplay Code
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateTemperatureDisplay
//* Object              : refresh ui
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverRefreshTemperatureDisplay(uchar enabled,int temp)
{
    uint8_t temp_enabled = df.temp_enabled & 0x0f;
    bool is_fahrenheit = (df.temp_enabled & 0xf0) != false;

    if((temp_enabled) == TCXO_STOP)	{	// if temperature update is disabled, don't update display!
        UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1), " ",Grey,Black,0);	// delete asterisk
        UiLcdHy28_PrintText((POS_TEMP_IND_X + 49),(POS_TEMP_IND_Y + 1), "STOPPED",Grey,Black,0);
    } else
    {
        uint32_t clr = temp_enabled==TCXO_ON?Blue:Red;
        UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",clr,Black,0);

        if((temp < 0) || (temp > 1000))	{// is the temperature out of range?
            UiLcdHy28_PrintText((POS_TEMP_IND_X + 49 + SMALL_FONT_WIDTH*1),(POS_TEMP_IND_Y + 1),"---.-",Grey,Black,0);
        } else if (temp != lo.last) {
            lo.last = temp;
            char out[10];

            int32_t ttemp = lo.last;
            if(is_fahrenheit)	{
                ttemp *= 9;			// multiply by 1.8
                ttemp /= 5;
                ttemp += 320;	// Add 32 degrees
            }
            snprintf(out,10,"%3ld.%1ld",ttemp/10,(ttemp)%10);
            UiLcdHy28_PrintText((POS_TEMP_IND_X + 49 + SMALL_FONT_WIDTH*1),(POS_TEMP_IND_Y + 1),out,Grey,Black,0);
        }
    }
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleLoTemperature
//* Object              : display LO temperature and compensate drift
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandleLoTemperature()
{
    int		temp = 0;
    int		comp, comp_p;
    float	dtemp, remain, t_index;
    uchar	tblp;

    uint8_t temp_enabled = df.temp_enabled & 0x0f;

    // No need to process if no chip avail or updates are disabled
    if((lo.sensor_absent == false) &&(temp_enabled != TCXO_STOP))
    {
        lo.skip++;
        if(lo.skip >= LO_COMP_SKP) {
            lo.skip = 0;
            // Get current temperature
            if(ui_si570_read_temp(&temp) == 0) {

                // Get temperature from sensor with its maximum precision
                dtemp = (float)temp;	// get temperature
                dtemp /= 10000;			// convert to decimal degrees
                remain = truncf(dtemp);	// get integer portion of temperature
                remain = dtemp - remain;	// get fractional portion

                // Compensate only if enabled
                if((temp_enabled == TCXO_ON)) {
                    // Temperature to unsigned table pointer
                    t_index  = (uchar)((temp%1000000)/100000);
                    t_index *= 10;
                    t_index += (uchar)((temp%100000)/10000);

                    // Check for overflow - keep within the lookup table
                    if((t_index < 0) || (t_index > 150))	{	// the temperature sensor function "wraps around" below zero
                        t_index = 0;						// point at the bottom of the temperature table
                        dtemp = 0;							// zero out fractional calculations
                        remain = 0;
                    }
                    else if(t_index > 98)	{				// High temperature - limit to maximum
                        t_index = 98;						// Point to (near) top of table
                        dtemp = 0;							// zero out fractional calculations
                        remain = 0;
                    }

                    tblp = (uchar)t_index;						// convert to index
                    // Check for overflow
                    if(tblp < 100)
                    {
                        // Value from freq table
                        comp = tcxo_table_20m[tblp];				// get the first entry in the table
                        comp_p = tcxo_table_20m[tblp + 1];			// get the next entry in the table to determine fraction of frequency step

                        comp_p = comp_p - comp;	//					// get frequency difference between the two steps

                        dtemp = (float)comp_p;	// change it to float for the calculation
                        dtemp *= remain;		// get proportion of temperature difference between the two steps using the fraction

                        comp += (int)dtemp;		// add the compensation value to the lower of the two frequency steps

                        // Change needed ?
                        if(lo.comp != comp) {		// is it there a difference?
                            // Update frequency, without reflecting it on the LCD
                                df.temp_factor = comp;
                                df.temp_factor_changed = true;
                                lo.comp = comp;
                        }
                    }
                    // FIXME: Move this close to UiDriverRefreshTemperatureDisplay. This should be
                    // done once the called function is more efficient (i.e. if no change, no update)
                    UiDriverUpdateLoMeter(tblp - 30,1);
                 }
                // Refresh UI
                UiDriverRefreshTemperatureDisplay(1,temp/1000); // precision is 0.1 represent by lowest digit
            }
        }
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

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSwitchOffPtt
//* Object              : PTT button release handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
ulong ptt_break = 0;
static void UiDriver_HandlePttOnOff()
{
  // Not when tuning
  if(ts.tune)
    return;

  // PTT on
  if(ts.ptt_req)
  {
    if(ts.txrx_mode == TRX_MODE_RX)
    {
      if(!ts.tx_disable)	{
        RadioManagement_SwitchTXRX(TRX_MODE_TX);
      }
    }

    ts.ptt_req = 0;

  } else if (!kd.enabled){
    // When CAT driver is running
    // skip auto return to RX
    // PTT off for all non-CW modes
    if(ts.dmod_mode != DEMOD_CW)
    {
      // PTT flag on ?
      if(ts.txrx_mode == TRX_MODE_TX)
      {
        // PTT line released ?
        if(GPIO_ReadInputDataBit(PADDLE_DAH_PIO,PADDLE_DAH))
        {
          // Lock to prevent IRQ re-entrance
          //ts.txrx_lock = 1;

          ptt_break++;
          if(ptt_break < 15)
            return;

          ptt_break = 0;

          // Back to RX
          RadioManagement_SwitchTXRX(TRX_MODE_RX);				// PTT

          // Unlock
          //ts.txrx_lock = 0;
        }
      }
    }
  }
}


//*----------------------------------------------------------------------------
//* Function Name       : uiCodecMute
//* Object              : Make available the "Codec_Mute" function to "main.c"
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

void uiCodecMute(uchar val)
{
	Codec_Mute(val);					// make sure codec is un-muted
}



//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSetBandPowerFactor
//* Object              : TX chain gain is not const for the 3-30 Mhz range
//* Input Parameters    : so adjust here
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverSetBandPowerFactor(uchar band)
{
	float	pf_temp;	// used as a holder for percentage of power output scaling

	if (band >= MAX_BANDS) {
		pf_temp = 3; // use very low value in case of wrong call to this function
	} else {
		pf_temp = (float)ts.pwr_adj[ts.power_level == PA_LEVEL_FULL?ADJ_FULL_POWER:ADJ_5W][band];
	}
	//
	ts.tx_power_factor = pf_temp/100;	// preliminarily scale to percent, which is the default for 5 watts

	// now rescale to power levels <5 watts, is so-configured

	switch(ts.power_level)	{
		case	PA_LEVEL_0_5W:
			pf_temp = 0.316;		// rescale for 10% of 5 watts (0.5 watts)
			break;
		case	PA_LEVEL_1W:
			pf_temp = 0.447;		// rescale for 20% of 5 watts (1.0 watts)
			break;
		case	PA_LEVEL_2W:
			pf_temp = 0.6324;		// rescale for 40% of 5 watts (2 watts)
			break;
		default:					// 100% is 5 watts or full power!!
			pf_temp = 1;
			break;
	}
	//
	ts.tx_power_factor *= pf_temp;	// rescale this for the actual power level

	if((df.tune_new < 8000000 * 4) && (ts.flags2 & FLAGS2_LOW_BAND_BIAS_REDUCE))		// reduction for frequencies < 8 MHz
	    ts.tx_power_factor = ts.tx_power_factor / 4;
}

// TODO: MOVE TO AUDIO /RF Function
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCWSidebandMode
//* Object              : Determine CW sideband and offset mode settings
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiCWSidebandMode()
{
    switch(ts.cw_offset_mode)   {
        case CW_OFFSET_USB_TX:
        case CW_OFFSET_USB_RX:
        case CW_OFFSET_USB_SHIFT:
            ts.cw_lsb = 0;              // Not LSB!
            break;
        case CW_OFFSET_LSB_TX:
        case CW_OFFSET_LSB_RX:
        case CW_OFFSET_LSB_SHIFT:
            ts.cw_lsb = 1;              // It is LSB
            break;
        case CW_OFFSET_AUTO_TX:                     // For "auto" modes determine if we are above or below threshold frequency
        case CW_OFFSET_AUTO_RX:
        case CW_OFFSET_AUTO_SHIFT:
            if(df.tune_new >= USB_FREQ_THRESHOLD)   // is the current frequency above the USB threshold?
                ts.cw_lsb = 0;                      // yes - indicate that it is USB
            else
                ts.cw_lsb = 1;                      // no - LSB
            break;
        default:
            ts.cw_lsb = 0;
            break;
    }
}


//
//*----------------------------------------------------------------------------
//* Function Name       : UiCheckForEEPROMLoadDefaultRequest
//* Object              : Cause default values to be loaded instead of EEPROM-stored values, show informational/warning splash screen, pause
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//* Comments            : The user MUST make a decision at that point, anyway:  To disconnect power
//  Comments            : preserve "old" settings or to power down using the POWER button to the new, default settings to EEPROM.
//  Comments            : WARNING:  Do *NOT* do this (press the buttons on power-up) when first loading a new firmware version as the EEPROM will be automatically be written over at startup!!!  [KA7OEI October, 2015]
//*----------------------------------------------------------------------------
//

enum CONFIG_DEFAULTS{
CONFIG_DEFAULTS_KEEP = 0,
CONFIG_DEFAULTS_LOAD_FREQ,
CONFIG_DEFAULTS_LOAD_ALL
};


/*
 * @brief Handles the loading of the configuration at startup (including the load of defaults if requested);
 * @returns false if it is a normal startup, true if defaults have been loaded
 */

static bool UiDriver_LoadSavedConfigurationAtStartup()
{

  uint16_t i;
  bool retval = false;
  uint8_t load_mode = CONFIG_DEFAULTS_KEEP;

  if (UiDriver_IsButtonPressed(BUTTON_F1_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F3_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F5_PRESSED)) {
    load_mode = CONFIG_DEFAULTS_LOAD_ALL;
  } else if (UiDriver_IsButtonPressed(BUTTON_F2_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F4_PRESSED)) {
    load_mode = CONFIG_DEFAULTS_LOAD_FREQ;
  }

  if(load_mode != CONFIG_DEFAULTS_KEEP) {
    // let us make sure, the user knows what he/she is doing
    // in case of change of mindes, do normal configuration load

    uint32_t clr_fg, clr_bg;
    const char* top_line;

    switch (load_mode) {
    case CONFIG_DEFAULTS_LOAD_ALL:
      clr_bg = Red;
      clr_fg = White;
      top_line = "    ALL DEFAULTS";
      break;
    case CONFIG_DEFAULTS_LOAD_FREQ:
      clr_bg = Yellow;
      clr_fg = Black;
      top_line = " FREQ/MODE DEFAULTS";
      break;
    }


    UiLcdHy28_LcdClear(clr_bg);							// clear the screen
    //													// now do all of the warnings, blah, blah...
    UiLcdHy28_PrintText(2,05,top_line,clr_fg,clr_bg,1);
    UiLcdHy28_PrintText(2,35," -> LOAD REQUEST <-",clr_fg,clr_bg,1);
    UiLcdHy28_PrintText(2,70,"      If you don't want to do this",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,85," press POWER button to start normally.",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,120," If you want to load default settings",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,135,"    press and hold BAND+ AND BAND-.",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,150,"  Settings will be saved at POWEROFF",clr_fg,clr_bg,0);
    //
    // On screen delay									// delay a bit...
    for(i = 0; i < 100; i++) {
      non_os_delay();
    }
    //
    // add this for emphasis
    UiLcdHy28_PrintText(2,195,"          Press BAND+ and BAND-  ",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,207,"           to confirm loading    ",clr_fg,clr_bg,0);

    while((((UiDriver_IsButtonPressed(BUTTON_BNDM_PRESSED)) && (UiDriver_IsButtonPressed(BUTTON_BNDP_PRESSED))) == false) && UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED) == false){ non_os_delay(); }


    if(UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED)) {
      UiLcdHy28_LcdClear(Black);							// clear the screen
      UiLcdHy28_PrintText(2,108,"      ...performing normal start...",White,Black,0);
      for(i = 0; i < 100; i++)
        non_os_delay();
      load_mode = CONFIG_DEFAULTS_KEEP;
      retval = false;
    } else {
      UiLcdHy28_LcdClear(clr_bg);							// clear the screen
      UiLcdHy28_PrintText(2,108,"     loading defaults in progress...",clr_fg,clr_bg,0);
      for(i = 0; i < 100; i++)
        non_os_delay();
      // call function to load values - default instead of EEPROM
      retval = true;
      ts.menu_var_changed = true;
    }
  }

  switch (load_mode) {
  case CONFIG_DEFAULTS_LOAD_ALL:
    ts.load_eeprom_defaults = true;                           // yes, set flag to indicate that defaults will be loaded instead of those from EEPROM
    break;
  case CONFIG_DEFAULTS_LOAD_FREQ:
    ts.load_freq_mode_defaults = true;
    break;
  }

  UiConfiguration_LoadEepromValues();
  ts.load_eeprom_defaults = false;
  ts.load_freq_mode_defaults = false;

  return retval;
}
//
//
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
//
//
void UiDriver_KeyTestScreen()
{
	ushort i, j, k, p_o_state, rb_state, new_state;
	uint32_t poweroffcount, rbcount, enccount;
	int direction;
	bool stat = 1;
	poweroffcount = rbcount = enccount = 0;
	p_o_state = rb_state = new_state = 0;
	char txt_buf[40];
	char* txt;
	for(i = 0; i <= 17; i++)	{			// scan all buttons
		if(UiDriver_IsButtonPressed(i))	{		// is one button being pressed?
			stat = 0;						// yes - clear flag
		}
	}
	//
	if(stat)			// no buttons pressed
		return;			// bail out
	//
	UiLcdHy28_LcdClear(Blue);							// clear the screen

	//
	UiLcdHy28_PrintText(10,35,"Input Elements Test",White,Blue,1);
	UiLcdHy28_PrintText(15,70,"press & hold POWER button to poweroff",White,Blue,0);
	UiLcdHy28_PrintText(20,90,"press & hold BAND- button to reboot",White,Blue,0);
	//
	for(;;)	{		// get stuck here for test duration
		j = 99;		// load with flag value
		k = 0;

		for(i = 0; i <= 17; i++)
		{				// scan all buttons
			if(UiDriver_IsButtonPressed(i))
			{		// is this button pressed?
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
		    char encnum[3];
		    sprintf(txt_buf,"%s"," Encoder ");		// building string for encoders
		    sprintf(encnum,"%d",t+1);
		    strcat(txt_buf,encnum);
		    if(direction > 0)
			strcat(txt_buf," <right>");
		    else
			strcat(txt_buf," <left> ");
		    j = 18+t;					// add encoders behind buttons;
		    }

		switch(j)	{				// decode button to text
		case	BUTTON_POWER_PRESSED:
			txt = "        POWER       ";
			if(poweroffcount > 75)
			{
			txt = "  powering off...   ";
			p_o_state = 1;
			}
			poweroffcount++;
			break;
		case	BUTTON_M1_PRESSED:
			txt = "         M1         ";
			break;
		case	BUTTON_M2_PRESSED:
			txt = "         M2         ";
			break;
		case	BUTTON_M3_PRESSED:
			txt = "         M3         ";
			break;
		case	BUTTON_G1_PRESSED:
			txt = "         G1         ";
			break;
		case	BUTTON_G2_PRESSED:
			txt = "         G2         ";
			break;
		case	BUTTON_G3_PRESSED:
			txt = "         G3         ";
			break;
		case	BUTTON_G4_PRESSED:
			txt = "         G4         ";
			break;
		case	BUTTON_F1_PRESSED:
			txt = "         F1         ";
			break;
		case	BUTTON_F2_PRESSED:
			txt = "         F2         ";
			break;
		case	BUTTON_F3_PRESSED:
			txt = "         F3         ";
			break;
		case	BUTTON_F4_PRESSED:
			txt = "         F4         ";
			break;
		case	BUTTON_F5_PRESSED:
			txt = "         F5         ";
			break;
		case	BUTTON_BNDM_PRESSED:
			txt = "        BND-        ";
			if(rbcount > 75)
			{
			txt = "    rebooting...    ";
			rb_state = 1;
			}
			rbcount++;
			break;
		case	BUTTON_BNDP_PRESSED:
			txt = "        BND+        ";
			break;
		case	BUTTON_STEPM_PRESSED:
			txt = "       STEP-        ";
			break;
		case	BUTTON_STEPP_PRESSED:
			txt = "      STEP+         ";
			break;
		case	TOUCHSCREEN_ACTIVE: ;
			UiLcdHy28_GetTouchscreenCoordinates(1);
			if(ts.tp_state > 1 && ts.tp_state != 0xff)
			    {
			    sprintf(txt_buf,"Touchscr. x:%02d y:%02d",ts.tp_x,ts.tp_y);	//show touched coordinates
			    txt = txt_buf;
			    ts.tp_state = 0;		// tp data processed
			    }
			else
			    txt = "";
			break;
		case	18+ENC1:							// handle encoder event
		case	18+ENC2:
		case	18+ENC3:
		case	18+ENCFREQ:
			txt = txt_buf;
			break;
		default:
			if(!enccount)
			    txt = "       <none>       ";				// no button pressed
			else
			    {
			    txt = "";
			    enccount--;
			    }
			poweroffcount = 0;
			rbcount = 0;
		}
		//
		if(txt[0])
		    UiLcdHy28_PrintText(10,120,txt,White,Blue,1);			// identify button on screen
		sprintf(txt_buf, "# of buttons pressed: %d  ", (int)k);
		UiLcdHy28_PrintText(75,160,txt_buf,White,Blue,0);			// show number of buttons pressed on screen

		if(p_o_state == 1)
		{
			GPIO_SetBits(POWER_DOWN_PIO,POWER_DOWN);
			for(;;);
		}
		if(rb_state == 1)
		{
			if(j != BUTTON_BNDM_PRESSED)
			{
			    mchf_reboot();
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

  uint16_t i;
  bool retval = false;

  if (UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE) && UiDriver_IsButtonPressed(BUTTON_F5_PRESSED))
    {

    uint32_t clr_fg, clr_bg;
    clr_bg = Magenta;
    clr_fg = White;
//    char txt_buf[40];

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
    UiLcdHy28_PrintText(2,70,"      If you don't want to do this",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,85," press POWER button to start normally.",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,120," If you want to calibrate touchscreen",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,135,"    press and hold BAND+ AND BAND-.",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,150,"  Settings will be saved at POWEROFF",clr_fg,clr_bg,0);
    //
    // On screen delay									// delay a bit...
    for(i = 0; i < 100; i++) { non_os_delay(); }
    //
    // add this for emphasis
    UiLcdHy28_PrintText(2,195,"          Press BAND+ and BAND-  ",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,207,"          to start calibration   ",clr_fg,clr_bg,0);

    while((((UiDriver_IsButtonPressed(BUTTON_BNDM_PRESSED)) && (UiDriver_IsButtonPressed(BUTTON_BNDP_PRESSED))) == false) && UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED) == false){ non_os_delay(); }


    if(UiDriver_IsButtonPressed(BUTTON_POWER_PRESSED)) {
      UiLcdHy28_LcdClear(Black);							// clear the screen
      UiLcdHy28_PrintText(2,108,"      ...performing normal start...",White,Black,0);
      for(i = 0; i < 100; i++)
        non_os_delay();
      retval = false;
    } else {
    UiLcdHy28_LcdClear(clr_bg);							// clear the screen
    UiLcdHy28_PrintText(2,70,"On the next screen crosses will appear.",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,82,"Touch as exact as you can on the middle",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,94,"    of each cross. After three valid",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,106,"  samples position of cross changes.",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,118," Repeat until the five test positions",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(2,130,"             are finished.",clr_fg,clr_bg,0);
    UiLcdHy28_PrintText(35,195,"Touch at any position to start.",clr_fg,clr_bg,0);

    while(UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE) == false){ non_os_delay(); }
    UiLcdHy28_GetTouchscreenCoordinates(1);
    ts.tp_state = 0;

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

    diffx = round(*x_corr / 15);
    diffy = round(*y_corr / 15);
    *x_corr = diffx;
    *y_corr = diffy;

    UiLcdHy28_LcdClear(clr_bg);

//    sprintf(txt_buf,"correction is  : %d/%d", *x_corr, *y_corr);
//    UiLcdHy28_PrintText(10,55,txt_buf,clr_fg,clr_bg,0);

      for(i = 0; i < 100; i++)
        non_os_delay();
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
    uchar i, datavalid = 0, samples = 0;

    clr_bg = Magenta;
    clr_fg = White;

    do{
	while(UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE) == false){ non_os_delay(); }

	UiLcdHy28_GetTouchscreenCoordinates(1);

	if(ts.tp_state > 1 && ts.tp_state != 0xff)
	    {
	    if(abs(ts.tp_x - cross[0]) < 4 && abs(ts.tp_y - cross[1]) < 4)
		{
		datavalid++;
		*xt_corr += (ts.tp_x - cross[0]);
		*yt_corr += (ts.tp_y - cross[1]);
		clr_fg = Green;
		sprintf(txt_buf,"Try (%d) error: x = %+d / y = %+d       ",datavalid,ts.tp_x-cross[0],ts.tp_y-cross[1]);	//show misajustments
		}
	    else
		{
		clr_fg = Red;
		sprintf(txt_buf,"Try (%d) BIG error: x = %+d / y = %+d",samples,ts.tp_x-cross[0],ts.tp_y-cross[1]);	//show misajustments
		}
	    samples++;
	    UiLcdHy28_PrintText(10,70,txt_buf,clr_fg,clr_bg,0);
	    ts.tp_state = 0xff;				// touchscreen data processed
	    }
    } while(datavalid < 3);

    for(i = 0; i < 100; i++) {
	non_os_delay();
    }
}
