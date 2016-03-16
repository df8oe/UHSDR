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


static void 	UiDriverPublicsInit(void);
static void 	UiDriverProcessKeyboard(void);
static void 	UiDriverPressHoldStep(uchar is_up);
static void 	UiDriverProcessFunctionKeyClick(ulong id);

static void 	UiDriverShowBand(uchar band);
static void 	UiDriverCreateDesktop(void);
static void 	UiDriverCreateFunctionButtons(bool full_repaint);
static void     UiDriverDeleteSMeter(void);
static void 	UiDriverCreateSMeter(void);
static void 	UiDriverDrawSMeter(ushort color);
//
static void 	UiDriverUpdateTopMeterA(uchar val);
static void 	UiDriverUpdateBtmMeter(uchar val, uchar warn);

static void 	UiDriverInitFrequency(void);
//
static void 	UiDriverCheckFilter(ulong freq);
uchar 			UiDriverCheckBand(ulong freq, ushort update);
static void 	UiDriverUpdateLcdFreq(ulong dial_freq,ushort color,ushort mode);
static uchar 	UiDriverButtonCheck(ulong button_num);
static void		UiDriverTimeScheduler(void);				// Also handles audio gain and switching of audio on return from TX back to RX
static void 	UiDriverChangeDemodMode(uchar noskip);
static void 	UiDriverChangeBand(uchar is_up);
static bool 	UiDriverCheckFrequencyEncoder(void);
static void 	UiDriverCheckEncoderOne(void);
static void 	UiDriverCheckEncoderTwo(void);
static void 	UiDriverCheckEncoderThree(void);
static void 	UiDriverChangeEncoderOneMode(uchar skip);
static void 	UiDriverChangeEncoderTwoMode(uchar skip);
static void 	UiDriverChangeEncoderThreeMode(uchar skip);
static void 	UiDriverChangeSigProc(uchar enabled);
static void 	UiDriverChangeRit(uchar enabled);
static void 	UiDriverChangeDSPMode(void);
static void 	UiDriverChangeDigitalMode(void);
static void 	UiDriverChangePowerLevel(void);
static void 	UiDriverHandleSmeter(void);
static void 	UiDriverHandleLowerMeter(void);
static void 	UiDriverHandlePowerSupply(void);
// LO TCXO routines
static void 	UiDriverUpdateLoMeter(uchar val,uchar active);
void 			UiDriverCreateTemperatureDisplay(uchar enabled,uchar create);
static void 	UiDriverRefreshTemperatureDisplay(uchar enabled,int temp);
static void 	UiDriverHandleLoTemperature(void);
static void 	UiDriverSwitchOffPtt(void);
static void 	UiDriverInitMainFreqDisplay(void);
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


// -------------------------------------------------------
// Constant declaration of the buttons map across ports
// - update if moving buttons around !!!
const ButtonMap	bm[16] =
{
		{BUTTON_M2_PIO,		BUTTON_M2},		// 0
		{BUTTON_G2_PIO,		BUTTON_G2},		// 1
		{BUTTON_G3_PIO,		BUTTON_G3},		// 2
		{BUTTON_BNDM_PIO,	BUTTON_BNDM},	// 3
		{BUTTON_G4_PIO,		BUTTON_G4},		// 4
		{BUTTON_M3_PIO,		BUTTON_M3},		// 5
		{BUTTON_STEPM_PIO,	BUTTON_STEPM},	// 6
		{BUTTON_STEPP_PIO,	BUTTON_STEPP},	// 7
		{BUTTON_M1_PIO,		BUTTON_M1},		// 8
		{BUTTON_F3_PIO,		BUTTON_F3},		// 9
		{BUTTON_F1_PIO,		BUTTON_F1},		// 10
		{BUTTON_F2_PIO,		BUTTON_F2},		// 11
		{BUTTON_F4_PIO,		BUTTON_F4},		// 12
		{BUTTON_BNDP_PIO,	BUTTON_BNDP},	// 13
		{BUTTON_F5_PIO,		BUTTON_F5},		// 14
		{BUTTON_G1_PIO,		BUTTON_G1}		// 15
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

//



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
	return (ts.tp_x != 0xff);
}

#define VFO_MEM_MODE_SPLIT 0x80
#define VFO_MEM_MODE_VFO_B 0x40


inline bool is_splitmode() {
	return (ts.vfo_mem_mode & VFO_MEM_MODE_SPLIT) != 0;
}
inline bool is_vfo_b() {
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
  if(ts.misc_flags1 & MISC_FLAGS1_WFALL_SCOPE_TOGGLE)
  {                       // is the waterfall mode active?
    ts.misc_flags1 &=  ~MISC_FLAGS1_WFALL_SCOPE_TOGGLE;     // yes, turn it off
    UiSpectrumInitSpectrumDisplay();   // init spectrum scope
  }
  else
  {                       // waterfall mode was turned off
    ts.misc_flags1 |=  MISC_FLAGS1_WFALL_SCOPE_TOGGLE;          // turn it on
    UiSpectrumInitSpectrumDisplay();   // init spectrum scope
  }
}

void UiDriver_HandleSwitchToNextDspMode()
{
	if(ts.dmod_mode != DEMOD_FM)	{ // allow selection/change of DSP only if NOT in FM
		if((!(is_dsp_nr())) && (!(is_dsp_notch())))	// both NR and notch are inactive
		{
			if(ts.dsp_enabled)
				ts.dsp_active |= DSP_NR_ENABLE;					// turn on NR
			else
				ts.dsp_active |= DSP_NOTCH_ENABLE;
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
			if((ts.dmod_mode == DEMOD_AM) && (ts.filter_id > AUDIO_4P8KHZ))		// was it AM with a filter > 4k8 selected?
				ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE);			// it was AM + wide - turn off NR and notch
			else
			{
				if(ts.dsp_enabled)
					ts.dsp_active |= DSP_NR_ENABLE;				// no - turn on NR
				else
					ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE);				// no - turn off NR and NOTCH
			}
		//
		else	{
			ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE);								// turn off NR and notch
		}
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
		if(ts.enc_two_mode == ENC_TWO_MODE_RF_GAIN)
			UiDriverChangeSigProc(0);
		else
			UiDriverChangeSigProc(1);
	}
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
		if(check_tp_coordinates(30,57,27,31))			// wf/scope bar right part
		{
		  UiDriver_ToggleWaterfallScopeDisplay();
		}
		if(check_tp_coordinates(10,28,27,31))			// wf/scope bar left part
		{
			sd.magnify = !sd.magnify;
			ts.menu_var_changed = 1;
			UiSpectrumInitSpectrumDisplay();		// init spectrum scope
		}
		if(check_tp_coordinates(8,60,5,19) && !ts.frequency_lock)// wf/scope frequency dial lower half spectrum/scope
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
			uint tunediff = ((1000)/(sd.magnify+1))*(ts.tp_x-line)*4;
			df.tune_new = lround((df.tune_new + tunediff)/step) * step;
			ts.refresh_freq_disp = 1;			// update ALL digits
			if(is_splitmode())
			{						// SPLIT mode
				UiDriverUpdateFrequency(1,3);
				UiDriverUpdateFrequency(1,2);
			}
			else
				UiDriverUpdateFrequency(1,0);		// no SPLIT mode
			ts.refresh_freq_disp = 0;			// update ALL digits
		}
		if(check_tp_coordinates(0,7,31,33))			// toggle digital modes
		{
		    incr_wrap_uint8(&ts.digital_mode,0,7);
			UiDriverChangeDigitalMode();
		}
		if(check_tp_coordinates(26,35,39,43))			// dynamic tuning activation
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
/*		if(check_tp_coordinates(x-left,x-right,y-down,y-up))	// new touchscreen action. LEAVE AS EXAMPLE. copy&paste for your purposes.
		{
		// here put action
		}
*/

	}
	else								// menu screen functions
	{
		if(check_tp_coordinates(54,57,55,57))			// enable tp coordinates show S-meter "dB"
		{
			ts.show_tp_coordinates = !ts.show_tp_coordinates;
			UiLcdHy28_PrintText(POS_PWR_NUM_IND_X,POS_PWR_NUM_IND_Y,ts.show_tp_coordinates?"enabled":"       ",Green,Black,0);
		}
		if(check_tp_coordinates(46,49,55,57))			// rf bands mod S-meter "40"
		    ts.rfmod_present = !ts.rfmod_present;
		if(check_tp_coordinates(50,53,55,57))			// vhf/uhf bands mod S-meter "60"
		    ts.vhfuhfmod_present = !ts.vhfuhfmod_present;
		if(ts.menu_mode)					// refresh menu
		    UiMenu_RenderMenu(MENU_RENDER_ONLY);
	}
	ts.tp_x = 0xff;							// prepare tp data for next touchscreen event
}


/**
 * @brief API Function, implements application logic for changing the power level including display updates
 *
 *
 * @param power_level The requested power level (as PA_LEVEL constants)
 */
void UiDriver_HandlePowerLevelChange(uint8_t power_level) {
	//
	if(ts.dmod_mode == DEMOD_AM)	{			// in AM mode?
		if(power_level >= PA_LEVEL_MAX_ENTRY)	// yes, power over 2 watts?
			power_level = PA_LEVEL_2W;	// force to 2 watt mode when we "roll over"
	}
	else	{	// other modes, do not limit max power
		if(power_level >= PA_LEVEL_MAX_ENTRY)
			power_level = PA_LEVEL_FULL;
	}
	//
	if (power_level != ts.power_level) {
		ts.power_level = power_level;
		UiDriverChangePowerLevel();
		if(ts.tune)		// recalculate sidetone gain only if transmitting/tune mode
			if(!ts.iq_freq_mode)	// Is translate mode *NOT* active?
				Codec_SidetoneSetgain();
		//
		if(ts.menu_mode)	// are we in menu mode?
			UiMenu_RenderMenu(MENU_RENDER_ONLY);	// yes, update display when we change power setting
		//
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
	if(ts.misc_flags1 & MISC_FLAGS1_SWAP_BAND_BTN)		// band up/down button swapped?
		UiDriverChangeBand(swapped);	// yes - go up
	else
		UiDriverChangeBand(normal);	// not swapped, go down
	//
	UiInitRxParms();	// re-init because mode/filter may have changed
	//
	if(ts.menu_mode)	// are we in menu mode?
		UiMenu_RenderMenu(MENU_RENDER_ONLY);	// yes, update menu display when we change bands
	//
	ads.af_disabled =  btemp;

}


//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_init(void)
{
	short res;

	// Driver publics init
	UiDriverPublicsInit();

	// Init frequency publics
	UiDriverInitFrequency();

	// Load stored data from eeprom
	UiConfiguration_LoadEepromValues();
	//
	AudioManagement_CalcTxCompLevel();		// calculate current settings for TX speech compressor
	//
	df.tune_new = vfo[VFO_WORK].band[ts.band].dial_value;		// init "tuning dial" frequency based on restored settings
	df.tune_old = df.tune_new;
	//
	UiCWSidebandMode();			// determine CW sideband mode from the restored frequency
	//
	AudioManagement_CalcRxIqGainAdj();		// Init RX IQ gain
	//
	AudioFilter_CalcRxPhaseAdj();			// Init RX IQ Phase (Hilbert transform/filter)
	//
	AudioFilter_CalcTxPhaseAdj();			// Init TX IQ Phase (Hilbert transform)
	//
	AudioManagement_CalcTxIqGainAdj();		// Init TX IQ gain
	//
	sd.display_offset = INIT_SPEC_AGC_LEVEL;		// initialize setting for display offset/AGC

	// Temp sensor setup
	lo.sensor_present = ui_si570_init_temp_sensor();

	// Read SI570 settings
	res = ui_si570_get_configuration();
	if(res != 0)
	{
		//printf("err I2C: %d\n\r",res);
	}

	// Create desktop screen
	UiDriverCreateDesktop();

	// Set SoftDDS in CW mode
	if(ts.dmod_mode == DEMOD_CW)
		softdds_setfreq((float)ts.sidetone_freq,ts.samp_rate,0);	// set sidetone - and CW TX offset from carrier
	else
		softdds_setfreq(0.0,ts.samp_rate,0);						// no "DDS" in non-CW modes

	// Update codec volume
	//  0 - 16: via codec command
	// 17 - 30: soft gain after decoder
	Codec_Volume((ts.rx_gain[RX_AUDIO_SPKR].value*8));		// This is only approximate - it will be properly set later

	// Set TX power factor
	UiDriverSetBandPowerFactor(ts.band);

	// Reset inter driver requests flag
	ts.LcdRefreshReq	= 0;
	ts.new_band 		= ts.band;
	df.step_new 		= df.tuning_step;

	// Extra HW init
	mchf_board_post_init();


	// Do update of frequency display
	ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
	//
	if(is_splitmode())	{	// in SPLIT mode?
		UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
		UiDriverUpdateFrequency(1,2);	// update RX frequency
	}
	else	// not in SPLIT mode - standard update
		UiDriverUpdateFrequency(1,0);

	ts.refresh_freq_disp = 0;	// clear flag that causes frequency display function to update ALL digits
	//
	//
	UiLCDBlankTiming();			// init timing for LCD blanking
	ts.lcd_blanking_time = ts.sysclock + LCD_STARTUP_BLANKING_TIME;

#ifdef DEBUG_BUILD
	printf("ui driver init ok\n\r");
#endif
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_thread
//* Object              : non urgent, time taking operations
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_thread(void)
{
//char txt[32];

if(ts.misc_flags1 & MISC_FLAGS1_WFALL_SCOPE_TOGGLE)	// is waterfall mode enabled?
		UiSpectrumReDrawWaterfall();	// yes - call waterfall update instead
	else
		UiSpectrumReDrawScopeDisplay();	// Spectrum Display enabled - do that!

	if(ts.thread_timer)			// bail out if it is not time to do this task
		return;

	ts.thread_timer = 1;		// reset flag to schedule next occurrance
	//

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
			if(!ts.boot_halt_flag) { UiDriverUpdateFrequency(0,0); }
			break;
		case STATE_PROCESS_KEYBOARD:
			UiDriverProcessKeyboard();
			break;
		case STATE_SWITCH_OFF_PTT:
			if(!ts.boot_halt_flag) { UiDriverSwitchOffPtt(); }
			break;
		default:
			drv_state = 0;
			return;
	}
	drv_state++;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_toggle_tx
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_toggle_tx(void)
{

	static bool was_menu = 0;		// used to detect if we *were* in the menu
	static bool was_rx = 1;
	static bool rx_muted = 0;
	bool	reset_freq = 0;
	ulong	calc_var;

	if(ts.txrx_mode == TRX_MODE_TX)
	{
		//
		// Below, in VOICE modes we mute the audio BEFORE we activate the PTT.  This is necessary since U3 is switched the instant that we do so,
		// rerouting audio paths and causing all sorts of disruption including CLICKs and squeaks.
		// We restore TX audio levels in the function "Codec_RX_TX()" according to operating mode
		//
		ts.dsp_inhibit = 1;								// disable DSP when going into TX mode
		//
		UiDriver_HandlePowerLevelChange(ts.power_level);
		// make sure that power level and mode fit together
		//
		if(ts.dmod_mode != DEMOD_CW)	{				// are we in a voice mode?
			if(ts.tx_audio_source != TX_AUDIO_MIC)	{	// yes - are we in LINE IN mode?
				Codec_Line_Gain_Adj(0);	// yes - momentarily mute LINE IN audio if in LINE IN mode until we have switched to TX
			}
			else	{	// we are in MIC IN mode
				Codec_Line_Gain_Adj(0);			// momentarily mute LINE IN audio until we switch modes because we will blast any connected LINE IN source until we switch
				ts.tx_mic_gain_mult = 0;		// momentarily set the mic gain to zero while we go to TX
				Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0016);	// Mute the microphone with the CODEC (this does so without a CLICK)
			}
			//
			if((ts.iq_freq_mode) && (!rx_muted))	{	// Is translate mode active and we have NOT already muted the audio?
				Codec_Volume(0);	// yes - mute the audio codec to suppress an approx. 6 kHz chirp when going in to TX mode
				rx_muted = 1;		// indicate that we've muted the audio so we don't do this every time through
			}
			//
			non_os_delay();		// pause an instant because the codec chip has its own delay before tasks complete!
		}
		//
		PTT_CNTR_PIO->BSRRL  	= PTT_CNTR;		// TX on and switch CODEC audio paths
		RED_LED_PIO->BSRRL 		= RED_LED;		// Red led on
		//
		// Set the PA bias according to mode
		//
		if((ts.pa_cw_bias) && (ts.dmod_mode == DEMOD_CW))	{	// is CW PA bias non-zero AND are we in CW mode?
			calc_var = BIAS_OFFSET + (ts.pa_cw_bias * 2);		// use special CW-mode bias setting
			if(calc_var > 255)
				calc_var = 255;
			//
			// Set DAC Channel 1 DHR12L register
			DAC_SetChannel2Data(DAC_Align_8b_R,calc_var);		// set PA bias
		}
		else	{
			calc_var = BIAS_OFFSET + (ts.pa_bias * 2);		// use "default" bias setting
			if(calc_var > 255)
				calc_var = 255;
			//
			// Set DAC Channel 1 DHR12L register
			DAC_SetChannel2Data(DAC_Align_8b_R,calc_var);		// set PA bias
		}
		//
		//	initialize everything in CW mode
		if(ts.dmod_mode == DEMOD_CW)	{
			softdds_setfreq((float)ts.sidetone_freq, ts.samp_rate,0);	// set sidetone frequency in CW mode (this also set TX shift)
		}
	}
	// RX Mode
	else
	{
		was_rx = 1;								// indicate that we WERE in RX mode
		PTT_CNTR_PIO->BSRRH  	= PTT_CNTR;		// TX off
		RED_LED_PIO->BSRRH 		= RED_LED;		// Red led off
		//
		UiDriverUpdateBtmMeter(0,0);		// clear bottom meter of any outstanding indication when going back to RX
		//
		rx_muted = 0;		// clear flag to indicate that we've muted the audio
	}

	reset_freq = 0;		// clear flag that indicates that we should reset the frequency

	if(is_splitmode())	{				// is SPLIT mode active?
		reset_freq = 1;							// yes - indicate that we WILL need to reset the synthesizer frequency
		if(is_vfo_b())	{				// is VFO-B active?
			if(ts.txrx_mode == TRX_MODE_TX)	{	// are we in TX mode?
				if(was_rx)	{						// did we just enter TX mode?
					vfo[VFO_B].band[ts.band].dial_value = df.tune_new;	// yes - save current RX frequency in VFO location (B)
					was_rx = 0;						// indicate that we are now in transmit mode to prevent re-loading of frequency
				}
				df.tune_new = vfo[VFO_A].band[ts.band].dial_value;	// load with VFO-A frequency
			}
			else					// we are in RX mode
				df.tune_new = vfo[VFO_B].band[ts.band].dial_value;	// load with VFO-B frequency
		}
		else	{	// VFO-A is active
			if(ts.txrx_mode == TRX_MODE_TX)	{				// are we in TX mode?
				if(was_rx)	{								// did we just enter TX mode?
					vfo[VFO_A].band[ts.band].dial_value = df.tune_new;	// yes - save current RX frequency in VFO location (A)
					was_rx = 0;						// indicate that we are now in transmit mode to prevent re-loading of frequency
				}
				df.tune_new = vfo[VFO_B].band[ts.band].dial_value;	// load with VFO-B frequency
			}
			else							// we are in RX mode
				df.tune_new = vfo[VFO_A].band[ts.band].dial_value;	// load with VFO-A frequency
		}
	}

	if((reset_freq) || (ts.rit_value) || ((ts.iq_freq_mode) && (ts.dmod_mode == DEMOD_CW)))		// Re-set frequency if RIT is non-zero or in CW mode with translate OR if in SPLIT mode and we had to retune
		UiDriverUpdateFrequencyFast();

	if((ts.menu_mode) || (was_menu))	{			// update menu when we are (or WERE) in MENU mode
		UiMenu_RenderMenu(MENU_RENDER_ONLY);
		was_menu = 1;
	}

	if(was_menu)		// if we'd displayed the menu previously, clear the flag
		was_menu = 0;
	//
	// Switch codec mode
	Codec_RX_TX();
	//

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverPublicsInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//*----------------------------------------------------------------------------
static void UiDriverPublicsInit(void)
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
	swrm.coupling_2200m_calc	= SWR_COUPLING_DEFAULT;
	swrm.coupling_630m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_160m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_80m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_40m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_20m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_15m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_6m_calc		= SWR_COUPLING_DEFAULT;
	swrm.pwr_meter_disp		= 0;	// Display of numerical FWD/REV power metering off by default
	swrm.pwr_meter_was_disp = 0;	// Used to indicate if FWD/REV numerical power metering WAS displayed

	// Power supply meter
	pwmt.skip 				= 0;
	pwmt.p_curr				= 0;
	pwmt.pwr_aver 			= 0;

	// LO tcxo
	lo.skip					= 0;
	lo.comp					= 0;
	lo.v1000000				= 0;
	lo.v100000				= 0;
	lo.v10000				= 0;
	lo.v1000				= 0;
}

// check if touched point is within rectange of valid action
inline bool check_tp_coordinates(uint8_t x_left, uint8_t x_right, uint8_t y_down, uint8_t y_up)
{
	return (ts.tp_x <= x_right && ts.tp_x >= x_left && ts.tp_y >= y_down && ts.tp_y <= y_up);
}

void UiDriverFButtonLabel(uint8_t button_num, const char* label, uint32_t label_color) {
  UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X + (button_num - 1)*64, POS_BOTTOM_BAR_F1_Y, label,
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
static void UiDriverProcessKeyboard(void)
{
	uchar temp;

	if(ks.button_processed)	{
		ts.nb_disable = 1;	// disable noise blanker if button is pressed or held
		//
		UiLCDBlankTiming();	// calculate/process LCD blanking timing
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
					UiInitRxParms();				// re-init with change of mode
				}
				break;
			case BUTTON_G2_PRESSED:		// BUTTON_G2
				UiDriver_HandleSwitchToNextDspMode();
				break;
			case BUTTON_G3_PRESSED:		// BUTTON_G3 - Change power setting
				UiDriver_HandlePowerLevelChange(ts.power_level+1);
				break;
			case BUTTON_G4_PRESSED:		{		// BUTTON_G4 - Change filter bandwidth
				if((!ts.tune) && (ts.dmod_mode != DEMOD_FM))	{
					ts.filter_id = AudioFilter_NextApplicableFilter();	// make sure that filter is active - if not, find next active filter

					// Change filter
					UiDriverChangeFilter(0);
					UiInitRxParms();		// re-init for change of filter

					if(ts.menu_mode) {	// are we in menu mode?
						UiMenu_RenderMenu(MENU_RENDER_ONLY);	// yes, update display when we change filters
					}
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
					if(!ts.menu_mode)	{						// not in menu mode
						UiSpectrumClearDisplay();			// clear display under spectrum scope
						if(ts.ser_eeprom_in_use == 0xFF)
							UiLcdHy28_PrintText(60,160,"Saving settings to virt. EEPROM",Cyan,Black,0);
						if(ts.ser_eeprom_in_use == 0x00)
							UiLcdHy28_PrintText(60,160,"Saving settings to serial EEPROM",Cyan,Black,0);
						UiConfiguration_SaveEepromValues();	// save settings to EEPROM
						for(temp = 0; temp < 6; temp++)			// delay so that it may be read
							non_os_delay();
						//
						UiSpectrumInitSpectrumDisplay();			// init spectrum scope
						ts.menu_mode = 0;
					}
					else	// If in menu mode, just save data, but don't clear screen area
						UiConfiguration_SaveEepromValues();	// save settings to EEPROM
					//
					ts.menu_var_changed = 0;					// clear "EEPROM SAVE IS NECESSARY" indicators
				}
				//
				if(!ts.menu_mode)	// are we in menu mode?
					UiDriverFButtonLabel(1," MENU  ",White);	// no - update menu button to reflect no memory save needed
				else
					UiMenu_RenderMenu(MENU_RENDER_ONLY);	// update menu display to remove indicator to do power-off to save EEPROM value
				break;
			case BUTTON_F2_PRESSED:	// Press-and-hold button F3
				// Move to the BEGINNING of the current menu structure
				if(ts.menu_mode)	{		// Are we in menu mode?
				  UiMenu_RenderFirstScreen();
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
						UiDriverFButtonLabel(3,"  MEM ",White);	// yes - indicate with color
					}
					else	{
						uint32_t color = is_splitmode()?SPLIT_ACTIVE_COLOUR:SPLIT_INACTIVE_COLOUR;
						ts.vfo_mem_flag = 0;		// it was in memory mode - switch to VFO mode
						UiDriverFButtonLabel(3," SPLIT",color);
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
					vfo_store->filter_mode = ts.filter_id;

					if(is_splitmode())	{	// are we in SPLIT mode?
						ts.refresh_freq_disp = 1;	// yes, we need to update the TX frequency:  Make frequency display refresh all digits
						UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
						UiDriverUpdateFrequency(1,2);	// Update receive frequency
						ts.refresh_freq_disp = 0;	// disable refresh all digits flag
					}
					UiSpectrumClearDisplay();			// clear display under spectrum scope
					UiLcdHy28_PrintText(80,160,is_vfo_b()?"VFO B -> VFO A":"VFO A -> VFO B",Cyan,Black,1);
					for(temp = 0; temp < 18; temp++)			// delay so that it may be read
						non_os_delay();

					UiSpectrumInitSpectrumDisplay();			// init spectrum scope
				}
				break;
			case BUTTON_F5_PRESSED:								// Button F5 was pressed-and-held - Toggle TX Disable
				if(ts.txrx_mode == TRX_MODE_RX && ts.tx_disable != 1)			// do NOT allow mode change in TUNE mode or transmit mode or disbled in config
				{
				  if(!ts.tx_disable)
				    ts.tx_disable = 2;
				  else
				    ts.tx_disable = 0;
				  UiDriverFButtonLabel(5,"  TUNE",ts.tx_disable?Grey1:White);		// Set TUNE button color according to ts.tx_disable
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
					if(is_dsp_nr()|| is_dsp_notch())	{			// is DSP NR or NOTCH active?
						ts.dsp_active_toggle = ts.dsp_active;	// save setting for future toggling
						ts.dsp_active &= ~(DSP_NR_ENABLE | DSP_NOTCH_ENABLE);				// turn off NR and notch
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
				break;
			case BUTTON_G3_PRESSED:		{	// Press-and-hold button G3
				UiInitRxParms();			// generate "reference" for sidetone frequency
				break;
			}
			case BUTTON_G4_PRESSED:		{	// Press-and-hold button G4 - Change filter bandwidth, allowing disabled filters, or do tone burst if in FM transmit
				if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX) && (ts.dmod_mode != DEMOD_FM))	{ // only allow in receive mode and when NOT in FM
				  incr_wrap_uint8(&ts.filter_id,AUDIO_MIN_FILTER,AUDIO_MAX_FILTER-1);

				  UiDriverChangeFilter(0);
				  AudioFilter_CalcRxPhaseAdj();			// We may have changed something in the RX filtering as well - do an update
				  UiDriverChangeDSPMode();	// Change DSP display setting as well
				  UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator
				  //
				  if(ts.menu_mode)	// are we in menu mode?
						UiMenu_RenderMenu(MENU_RENDER_ONLY);	// yes, update display when we change filters
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
				if(!UiDriverButtonCheck(BUTTON_BNDM_PRESSED))	{	// was button BAND- pressed at the same time?
					if(ts.lcd_backlight_blanking & 0x80)			// Yes - is MSB set, indicating "stealth" (backlight timed-off) mode?
						ts.lcd_backlight_blanking &= 0x7f;		// yes - clear that bit, turning off "stealth" mode
					else
					{
						if(ts.lcd_backlight_blanking & 0x0f)	// bit NOT set AND the timing set to NON-zero?
							ts.lcd_backlight_blanking |= 0x80;		// no - turn on MSB to activate "stealth" mode
					}
				}
				else
				{	// ONLY the POWER button was pressed
					if(ts.txrx_mode == TRX_MODE_RX)		// only allow power-off in RX mode
						mchf_board_power_off();
				}
				break;
			case BUTTON_BNDM_PRESSED:			// BAND- button pressed-and-held?
				if(!UiDriverButtonCheck(BUTTON_POWER_PRESSED))	{	// and POWER button pressed-and-held at the same time?
					if(ts.lcd_backlight_blanking & 0x80)			// Yes - is MSB set, indicating "stealth" (backlight timed-off) mode?
						ts.lcd_backlight_blanking &= 0x7f;		// yes - clear that bit, turning off "stealth" mode
					else if(ts.lcd_backlight_blanking & 0x0f)	// bit NOT set AND the timing set to NON-zero?
						ts.lcd_backlight_blanking |= 0x80;		// no - turn on MSB to activate "stealth" mode
				}
				else if(!UiDriverButtonCheck(BUTTON_BNDP_PRESSED))	{	// and BAND-UP pressed at the same time?
					if(!ts.menu_mode)	{			// do not do this in menu mode!
					  UiDriver_ToggleWaterfallScopeDisplay();
					}
				}
				break;
			case BUTTON_BNDP_PRESSED:			// BAND+ button pressed-and-held?
				if(!UiDriverButtonCheck(BUTTON_BNDM_PRESSED))	{	// and BAND-DOWN pressed at the same time?
					if(!ts.menu_mode)	{		// do not do this if in menu mode!
					  UiDriver_ToggleWaterfallScopeDisplay();
					}
				}
				if(!UiDriverButtonCheck(BUTTON_POWER_PRESSED))	{	// and POWER button pressed-and-held at the same time?
					ts.ser_eeprom_in_use = 0x20;			// power down without saving settings
					mchf_board_power_off();
				}
				break;
			case BUTTON_STEPM_PRESSED:
				if(!UiDriverButtonCheck(BUTTON_STEPP_PRESSED))	{	// was button STEP+ pressed at the same time?
					ts.frequency_lock = !ts.frequency_lock;
					// update frequency display
					ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
					//
					if(is_splitmode())	{	// in SPLIT mode?
						UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
						UiDriverUpdateFrequency(1,2);	// update RX frequency
					}
					else	{	// not in SPLIT mode - standard update
						UiDriverUpdateFrequency(1,0);
					}
					ts.refresh_freq_disp = 0;	// restore selective update mode for frequency display
				}
				else	{
					if(!(ts.freq_step_config & FREQ_STEP_SWAP_BTN))	// button swap NOT enabled
						UiDriverPressHoldStep(0);	// decrease step size
					else		// button swap enabled
						UiDriverPressHoldStep(1);	// increase step size
				}
				//
				break;
			case BUTTON_STEPP_PRESSED:
				if(!UiDriverButtonCheck(BUTTON_STEPM_PRESSED))	{	// was button STEP- pressed at the same time?
					ts.frequency_lock = !ts.frequency_lock;
					// update frequency display
					ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
					//
					if(is_splitmode())	{	// in SPLIT mode?
						UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
						UiDriverUpdateFrequency(1,2);	// update RX frequency
					}
					else	{	// not in SPLIT mode - standard update
						UiDriverUpdateFrequency(1,0);
					}
					ts.refresh_freq_disp = 0;	// restore selective update mode for frequency display
				}
				else	{
					if(!(ts.freq_step_config & FREQ_STEP_SWAP_BTN))	// button swap NOT enabled
						UiDriverPressHoldStep(1);	// increase step size
					else		// button swap enabled
						UiDriverPressHoldStep(0);	// decrease step size
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
void UiInitRxParms(void)
{
	UiCWSidebandMode();
	if(ts.menu_mode)	// are we in menu mode?
		UiMenu_RenderMenu(MENU_RENDER_ONLY);	// yes, update display when we change modes
	//
	AudioManagement_CalcTxIqGainAdj();		// update gain and phase values when changing modes
	AudioFilter_CalcTxPhaseAdj();
	AudioFilter_CalcRxPhaseAdj();
	UiDriverChangeRfGain(1);	// update RFG/SQL on screen
	Audio_TXFilter_Init();
	UiDriverChangeDSPMode();	// Change DSP display setting as well
	UiDriverChangeDigitalMode();	// Change Dgital display setting as well
	UiDriverChangeFilter(1);	// make certain that numerical on-screen bandwidth indicator is updated
	audio_driver_set_rx_audio_filter();	// update DSP/filter settings
	UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator (graphical)
	UiDriverUpdateFrequency(1,0);	// update frequency display without checking encoder
	//
	if(ts.dmod_mode == DEMOD_CW)	{		// update on-screen adjustments
		UiDriverChangeKeyerSpeed(0);		// emplace keyer speed (WPM) and
		UiDriverChangeStGain(0);			// sidetone gain when in CW mode
	}
	else	{
		UiDriverChangeAudioGain(0);			// display Line/Mic gain and
		UiDriverChangeCmpLevel(0);			// Compression level when in voice mode
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
				UiSpectrumClearDisplay();
                UiDriverFButtonLabel(1," EXIT  ", Yellow);
                UiDriverFButtonLabel(2,"  PREV",Yellow);
                UiDriverFButtonLabel(3,"  NEXT",Yellow);
                UiDriverFButtonLabel(4," DEFLT",Yellow);
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
				UiSpectrumInitSpectrumDisplay();			// init spectrum scope
				//
				// Restore encoder displays to previous modes
				UiDriverChangeEncoderOneMode(0);
				UiDriverChangeEncoderTwoMode(0);
				UiDriverChangeEncoderThreeMode(0);
				// FIXME: Call twice since the function TOGGLES, not just enables (need to fix this at some point!)
				UiDriverChangeEncoderOneMode(0);
				UiDriverChangeEncoderTwoMode(0);
				UiDriverChangeEncoderThreeMode(0);
				UiDriverChangeFilter(1);	// update bandwidth display
				// Label for Button F1
				{
					char* label;
					uint32_t color;
					if(!ts.menu_var_changed) {
						label = " MENU  ";
						color = White;
					} else {
						label = " MENU *";
						color = Orange;
					}
					UiDriverFButtonLabel(1,label,color);
				}
				// Label for Button F2
				UiDriverFButtonLabel(2," METER",White);

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
					char* label = is_vfo_b()?" VFO B":" VFO A";
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
		else	{	// Not in MENU mode - select the METER mode
		    incr_wrap_uint8(&ts.tx_meter_mode,0,METER_MAX-1);

		    UiDriverDeleteSMeter();
			UiDriverCreateSMeter();	// redraw meter
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
					ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
					UiDriverUpdateFrequency(1,1);	// force update of large digits
					ts.refresh_freq_disp = 0;	// disable refresh all digits flag
				}
				else if(!(is_splitmode()))	{	// are we NOT in SPLIT mode?
					ts.vfo_mem_mode |= 0x80;		// yes - turn on MSB to activate SPLIT
					UiDriverInitMainFreqDisplay();		//
					ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
					UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
					UiDriverUpdateFrequency(1,2);	// force display of first (RX) VFO frequency
					ts.refresh_freq_disp = 0;	// disable refresh all digits flag
				}
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
				vfo_name = " VFO A";
				ts.vfo_mem_mode &= 0xbf;	// yes, it's now VFO-B mode, so clear it, setting it to VFO A mode
			}
			else	{						// LSB on VFO mode byte NOT set?
				ts.vfo_mem_mode |= 0x40;			// yes, it's now in VFO-A mode, so set it, setting it to VFO B mode
				vfo_active = VFO_B;
				vfo_new = VFO_A;
				vfo_name = " VFO B";
			}
			vfo[vfo_new].band[ts.band].dial_value = df.tune_old;	//band_dial_value[ts.band];		// save "VFO B" settings
			vfo[vfo_new].band[ts.band].decod_mode = ts.dmod_mode;	//band_decod_mode[ts.band];
			vfo[vfo_new].band[ts.band].filter_mode = ts.filter_id;	//band_filter_mode[ts.band];
			//

			vfo[VFO_WORK].band[ts.band].dial_value = vfo[vfo_active].band[ts.band].dial_value;		// load "VFO A" settings into working registers
			vfo[VFO_WORK].band[ts.band].decod_mode = vfo[vfo_active].band[ts.band].decod_mode;
			vfo[VFO_WORK].band[ts.band].filter_mode = vfo[vfo_active].band[ts.band].filter_mode;
			//
			UiDriverFButtonLabel(4,vfo_name,White);


			df.tune_new = vfo[VFO_WORK].band[ts.band].dial_value;
			//
			// do frequency/display update
			if(is_splitmode())	{	// in SPLIT mode?
                UiDriverDisplaySplitFreqLabels();
                ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
				UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency - do this first so small display shows RX freq
				UiDriverUpdateFrequency(1,2);	// update RX frequency
				ts.refresh_freq_disp = 0;
			}
			else	// not in SPLIT mode - standard update
				UiDriverUpdateFrequency(1,0);

			// Change decode mode if need to
			if(ts.dmod_mode != vfo[VFO_WORK].band[ts.band].decod_mode)
			{
				// Update mode
				ts.dmod_mode = vfo[VFO_WORK].band[ts.band].decod_mode;

				// Update Decode Mode (USB/LSB/AM/FM/CW)
				UiDriverShowMode();
			}

			// Change filter mode if need to
			if(ts.filter_id != vfo[VFO_WORK].band[ts.band].filter_mode)
			{
				ts.filter_id = vfo[VFO_WORK].band[ts.band].filter_mode;
				UiDriverChangeFilter(0);	// update display and change filter
				UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator
				audio_driver_set_rx_audio_filter();
				audio_driver_set_rx_audio_filter();	// TODO: we have to invoke the filter change several times for some unknown reason - 'dunno why!
			}
		}
		//
		//
	}

	// --------------------------------------------
	// F5 process
	if(id == BUTTON_F5_PRESSED)
	{
		if(!ts.tx_disable)	{	// Allow TUNE mode only if TX is NOT disabled
			// Toggle tune
			ts.tune = !ts.tune;

			if((ts.dmod_mode == DEMOD_AM) || (ts.dmod_mode == DEMOD_FM))
				ts.tune = 0;	// no TUNE mode in AM or FM!

			// Enter TUNE mode
			if(ts.tune)
			{
				if(ts.tune_power_level != PA_LEVEL_MAX_ENTRY)
				    {
				    ts.power_temp = ts.power_level;				//store tx level and set tune level
				    ts.power_level = ts.tune_power_level;
				    UiDriverChangePowerLevel();
				    }
				if((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB))
					softdds_setfreq(SSB_TUNE_FREQ, ts.samp_rate,0);		// generate tone for setting TX IQ phase
				// DDS on
				else
					softdds_setfreq(CW_SIDETONE_FREQ_DEFAULT,ts.samp_rate,0);

				// To TX
				ts.txrx_mode = TRX_MODE_TX;
				ui_driver_toggle_tx();				// tune ON

				UiDriverFButtonLabel(5,"  TUNE",Red);
				//
			}
			else
			{
				if(ts.tune_power_level != PA_LEVEL_MAX_ENTRY)
				    {
				    ts.power_level = ts.power_temp;					// restore tx level
//				    UiDriverChangePowerLevel();
				    }
				if((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB))	// DDS off if voice mode
					softdds_setfreq(0.0,ts.samp_rate,0);
				else if(ts.dmod_mode == DEMOD_CW)	{	// DDS reset to proper sidetone freq. if CW mode
					cw_gen_init();
					softdds_setfreq((float)ts.sidetone_freq,ts.samp_rate,0);
				}
				//
				// Back to RX
				ts.txrx_mode = TRX_MODE_RX;
				ui_driver_toggle_tx();				// tune OFF
				UiDriverChangePowerLevel();			// former position was upper commented out position
										// at this position display error in CW when using LCD in parallel
										// mode and working power is FULL and TUNE power is 5W
										// WARNING THIS WORKAROUND IS UGLY

				UiDriverFButtonLabel(5,"  TUNE",White);
				//
			}
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverShowMode(void)	{
	// Clear control
	UiLcdHy28_DrawFullRect(POS_DEMOD_MODE_MASK_X,POS_DEMOD_MODE_MASK_Y,POS_DEMOD_MODE_MASK_H,POS_DEMOD_MODE_MASK_W,Blue);
	uint16_t offset = 4; // where to print the text
	char* txt = "MODE?";
	uint16_t clr_fg = White,clr_bg = Blue;

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	switch(ts.dmod_mode)
	{
		case DEMOD_USB:
			offset = 8;
			txt = "USB";
			break;
		case DEMOD_LSB:
			offset = 8;
			txt = "LSB";
			break;
		case DEMOD_AM:
			offset = 12;
			txt = "AM";
			break;
		case DEMOD_FM:
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
			offset = 4;
			txt = " FM ";
			break;
		}
		case DEMOD_CW:
			offset = 4;
			txt = ts.cw_lsb?"CW-L":"CW-U";
			break;
		default:
			break;
	}
	UiLcdHy28_PrintText((POS_DEMOD_MODE_X + offset),POS_DEMOD_MODE_Y,txt,clr_fg,clr_bg,0);
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

	ulong	line_loc;
	static	bool	step_line = 0;	// used to indicate the presence of a step line
	ulong	color;
	ulong 	stepsize_background;

	color = ts.tune_step?Cyan:White;		// is this a "Temporary" step size from press-and-hold?
	stepsize_background = ts.dynamic_tuning_active?Grey3:Black;
	// dynamic_tuning active -> yes, display on Grey3

	if(step_line)	{	// Remove underline indicating step size if one had been drawn
		UiLcdHy28_DrawStraightLineDouble((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * 3)),(POS_TUNE_FREQ_Y + 24),(LARGE_FONT_WIDTH*7),LCD_DIR_HORIZONTAL,Black);
	}

	// Blank old step size
	UiLcdHy28_DrawFullRect(POS_TUNE_STEP_MASK_X,POS_TUNE_STEP_MASK_Y,POS_TUNE_STEP_MASK_H,POS_TUNE_STEP_MASK_W,Black);

	{
		const char* step_name;

		// Create Step Mode
		switch(df.tuning_step)
		{
		case T_STEP_1HZ:
			step_name = "1Hz";
			line_loc = 9;
			break;
		case T_STEP_10HZ:
			line_loc = 8;
			step_name = "10Hz";
			break;
		case T_STEP_100HZ:
			step_name = "100Hz";
			line_loc = 7;
			break;
		case T_STEP_1KHZ:
			step_name = "1kHz";
			line_loc = 5;
			break;
		case T_STEP_5KHZ:
			step_name = "5kHz";
			line_loc = 5;
			break;
		case T_STEP_10KHZ:
			step_name = "10kHz";
			line_loc = 4;
			break;
		case T_STEP_100KHZ:
			step_name = "100kHz";
			line_loc = 3;
			break;
		case T_STEP_1MHZ:
			step_name = "1MHz";
			line_loc = 3;
			break;
		case T_STEP_10MHZ:
			step_name = "10MHz";
			line_loc = 3;
			break;
		default:
			step_name = "???";
			line_loc = 0; // default for unknown tuning step modes, disables the frequency marker display
			break;
		}
		UiLcdHy28_PrintTextRight((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*6),POS_TUNE_STEP_Y,step_name,color,stepsize_background,0);
	}
	//
	if((ts.freq_step_config & 0x0f) && line_loc > 0)	{		// is frequency step marker line enabled?
		UiLcdHy28_DrawStraightLineDouble((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * line_loc)),(POS_TUNE_FREQ_Y + 24),(LARGE_FONT_WIDTH),LCD_DIR_HORIZONTAL,White);
		step_line = 1;	// indicate that a line under the step size had been drawn
	}
	else	// marker line not enabled
		step_line = 0;	// we don't need to erase "step size" marker line in the future

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowBand
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverShowBand(uchar band)
{
	if (band < MAX_BAND_NUM) {
	// Clear control
		UiLcdHy28_DrawFullRect(POS_BAND_MODE_MASK_X,POS_BAND_MODE_MASK_Y,POS_BAND_MODE_MASK_H,POS_BAND_MODE_MASK_W,Black);
		UiLcdHy28_PrintTextRight(POS_BAND_MODE_X + 5*8,POS_BAND_MODE_Y,bandInfo[band].name,Orange,Black,0);
	}
}

void UiDriverChangeBandFilterPulseRelays() {
	BAND2_PIO->BSRRH = BAND2;
	non_os_delay();
	BAND2_PIO->BSRRL = BAND2;
}

void UiDriverChangeBandFilter(uchar band)
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

			UiDriverChangeBandFilterPulseRelays();

			// External group -Set(High/High)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			UiDriverChangeBandFilterPulseRelays();

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

			UiDriverChangeBandFilterPulseRelays();

			// External group - Reset(Low/High)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			UiDriverChangeBandFilterPulseRelays();

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

			UiDriverChangeBandFilterPulseRelays();

			// External group - Reset(Low/High)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			UiDriverChangeBandFilterPulseRelays();

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

			UiDriverChangeBandFilterPulseRelays();

			// External group - Set(High/High)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			UiDriverChangeBandFilterPulseRelays();

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
static void UiDriverInitMainFreqDisplay(void)
{
	if(!(is_splitmode()))	{	// are we in SPLIT mode?
		if(!ts.vfo_mem_flag)	{	// update bottom of screen if in VFO (not memory) mode
			UiDriverFButtonLabel(3," SPLIT",SPLIT_INACTIVE_COLOUR);	// make SPLIT indicator grey to indicate off
		}
	}
	else	{	// are we NOT in SPLIT mode?
		if(!ts.vfo_mem_flag)	{	// update bottom of screen if in VFO (not memory) mode
			UiDriverFButtonLabel(3," SPLIT",White);	// make SPLIT indicator YELLOW to indicate on
		}
		UiLcdHy28_PrintText(POS_TUNE_FREQ_X,POS_TUNE_FREQ_Y,"          ",White,Black,1);	// clear large frequency digits
		UiDriverDisplaySplitFreqLabels();
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateDesktop
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateDesktop(void)
{
	//char temp[10];

	// Backlight off - hide startup logo
	LCD_BACKLIGHT_PIO->BSRRH = LCD_BACKLIGHT;

	// Clear display
	UiLcdHy28_LcdClear(Black);

	// Create Band value
	UiDriverShowBand(ts.band);

	// Set filters
	UiDriverChangeBandFilter(ts.band);

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
	UiDriverChangePowerLevel();

	// FIR via keypad, not encoder mode
	UiDriverChangeFilter(1);

	UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator

	// Create USB Keyboard indicator
	//UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"KBD",Grey,Black,0);

	// Create RX/TX indicator
	//UiLcdHy28_PrintText(POS_TX_IND_X,POS_TX_IND_Y,	"RX", Green,Black,0);

	// Create voltage
	UiLcdHy28_DrawStraightLine	(POS_PWRN_IND_X,(POS_PWRN_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	UiLcdHy28_PrintText			(POS_PWRN_IND_X, POS_PWRN_IND_Y,"  VCC  ", Grey2, 	Blue, 0);
	UiLcdHy28_PrintText			(POS_PWR_IND_X,POS_PWR_IND_Y,   "--.--V",  COL_PWR_IND,Black,0);

	// Create temperature
	if((lo.sensor_present == 0) && (df.temp_enabled & 0x0f))
		UiDriverCreateTemperatureDisplay(1,1);
	else
		UiDriverCreateTemperatureDisplay(0,1);

	// Set correct frequency
	//UiDriverUpdateFrequency(1,0);
	ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
	//
	if(!(is_splitmode()))	{	// are we in SPLIT mode?
		UiDriverUpdateFrequency(1,1);	// yes - force update of large digits
	}
	else	{	// are we in SPLIT mode?
		UiDriverUpdateFrequency(1,3);	// yes - force display of second (TX) VFO frequency
		UiDriverUpdateFrequency(1,2);	// force display of first (RX) VFO frequency
	}
	//
	ts.refresh_freq_disp = 0;	// disable refresh all digits flag
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
	UiDriverFButtonLabel(1,"  MENU",White);
	// Button F2
	UiDriverFButtonLabel(2," METER",White);

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
	UiDriverFButtonLabel(4,is_vfo_b()?" VFO B":" VFO A",White);

	// Button F5
	clr = ts.tx_disable?Grey1:White;
	UiDriverFButtonLabel(5,"  TUNE",clr);
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
static void UiDriverDeleteSMeter(void)
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
static void UiDriverCreateSMeter(void)
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
//* Function Name       : UiDrawSpectrumScopeFrequencyBarText
//* Object              : Draw the frequency information on the frequency bar at the bottom of the spectrum scope based on the current frequency
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDrawSpectrumScopeFrequencyBarText(void)
{
	ulong	freq_calc;
	ulong	i, clr;
	char	txt[16], *c;
	ulong	grat;

	if(ts.scope_scale_colour == SPEC_BLACK)		// don't bother updating frequency scale if it is black (invisible)!
		return;

	grat = 6;	// Default - Magnify mode OFF, graticules spaced 6 kHz
	//
	if(sd.magnify)			// magnify mode on - only available when NOT in translate mode
		grat = 3;	// graticules spaced 3 kHz

	//
	// This function draws the frequency bar at the bottom of the spectrum scope, putting markers every at every graticule and the full frequency
	// (rounded to the nearest kHz) in the "center".  (by KA7OEI, 20140913)
	//
	// get color for frequency scale
	//
	UiMenu_MapColors(ts.scope_scale_colour,NULL, &clr);


	freq_calc = df.tune_new/4;		// get current frequency in Hz

	if(!sd.magnify)	{		// if magnify is off, way *may* have the graticule shifted.  (If it is on, it is NEVER shifted from center.)
		freq_calc += audio_driver_xlate_freq();
	}
	freq_calc = (freq_calc + 500)/1000;	// round graticule frequency to the nearest kHz

	//

	if((ts.iq_freq_mode == FREQ_IQ_CONV_MODE_OFF) || (sd.magnify))	{	// Translate mode is OFF or magnify is on
		sprintf(txt, "  %u  ", (unsigned)freq_calc);	// build string for center frequency
		i = 130-((strlen(txt)-2)*4);	// calculate position of center frequency text
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + i),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),txt,clr,Black,4);

		sprintf(txt, " %u ", (unsigned)freq_calc-(unsigned)grat);	// build string for left-of-center frequency
		c = &txt[strlen(txt)-3];  // point at 2nd character from the end
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +  90),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
		//

		sprintf(txt, " %u ", (unsigned)freq_calc+(unsigned)grat);	// build string for marker right of center
		c = &txt[strlen(txt)-3];  // point at 2nd character from the end
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 154),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
	}
	else if((ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ) && (!sd.magnify))	{	// Translate mode is ON (LO is HIGH, center is left of middle of display) AND magnify is off
		sprintf(txt, "  %u  ", (unsigned)freq_calc-(unsigned)grat);	// build string for center frequency
		i = 94-((strlen(txt)-2)*4);	// calculate position of center frequency text
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + i),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),txt,clr,Black,4);

		sprintf(txt, " %u ", (unsigned)freq_calc);	// build string for center marker
		c = &txt[strlen(txt)-3];  // point at 2nd character from the end
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +  122),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
		//
		sprintf(txt, " %u ", (unsigned)freq_calc+(unsigned)grat);	// build string for marker right of center
		c = &txt[strlen(txt)-3];  // point at 2nd character from the end
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 154),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
	}
	else if((ts.iq_freq_mode == FREQ_IQ_CONV_M6KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ) && (!sd.magnify))	{	// Translate mode is ON (LO is LOW, center is to the right of middle of display) AND magnify is off
		sprintf(txt, "  %u  ", (unsigned)freq_calc+(unsigned)grat);	// build string for center frequency
		i = 160-((strlen(txt)-2)*4);	// calculate position of center frequency text
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + i),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),txt,clr,Black,4);

		sprintf(txt, " %u ", (unsigned)freq_calc-(unsigned)grat);	// build string for left-of-center frequency
		c = &txt[strlen(txt)-3];  // point at 2nd character from the end
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +  90),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
		//

		sprintf(txt, " %u ", (unsigned)freq_calc);	// build string for frequency in center of display
		c = &txt[strlen(txt)-3];  // point at 2nd character from the end
		UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 122),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
	}

	// remainder of frequency/graticule markings

	sprintf(txt, " %u ", (unsigned)freq_calc-(2*(unsigned)grat));	// build string for middle-left frequency
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +  58),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
	//

	sprintf(txt, " %u ", (unsigned)freq_calc+(2*(unsigned)grat));	// build string for middle-right frequency
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 186),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);

	sprintf(txt, "%u ", (unsigned)freq_calc-(4*(unsigned)grat));	// build string for left-most frequency
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X ),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
	//

	sprintf(txt, "%u ", (unsigned)freq_calc-(3*(unsigned)grat));	// build string for right of left-most frequency
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +   26),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
	//

	sprintf(txt, " %u ", (unsigned)freq_calc+(3*(unsigned)grat));	// build string for left of far-right frequency
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 218),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
	//

	sprintf(txt, " %u", (unsigned)freq_calc+(4*(unsigned)grat));	// build string for far-right frequency
	c = &txt[strlen(txt)-2];  // point at 2nd character from the end
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 242),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);

}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateDigiPanel
//* Object              : draw the digital modes info panel
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverCreateDigiPanel(void)
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
static void UiDriverInitFrequency(void)
{
	ulong i;

	// Clear band values array
	for(i = 0; i < MAX_BANDS; i++)
	{
		vfo[VFO_WORK].band[i].dial_value = 0xFFFFFFFF;	// clear dial values
		vfo[VFO_WORK].band[i].decod_mode = DEMOD_USB; 	// clear decode mode
		vfo[VFO_WORK].band[i].filter_mode = AUDIO_DEFAULT_FILTER;	// clear filter mode
	}

	// Lower bands default to LSB mode
	// TODO: This needs to be checked, some even lower bands have higher numbers now
	for(i = 0; i < 4; i++)
		vfo[VFO_WORK].band[i].decod_mode = DEMOD_LSB;

	// Init frequency publics(set diff values so update on LCD will be done)
	df.tune_old 	= bandInfo[ts.band].tune;
	df.tune_new 	= bandInfo[ts.band].tune;
	df.selected_idx = 3; 		// 1 Khz startup step
	df.tuning_step	= tune_steps[df.selected_idx];
	df.temp_factor	= 0;
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
		{ BAND_FILTER_UPPER_160, FILTER_BAND_160, BAND_MODE_160 },
		{ BAND_FILTER_UPPER_80, FILTER_BAND_80, BAND_MODE_80 },
		{ BAND_FILTER_UPPER_40, FILTER_BAND_40, BAND_MODE_40 },
		{ BAND_FILTER_UPPER_20, FILTER_BAND_20, BAND_MODE_20 },
		{ BAND_FILTER_UPPER_10, FILTER_BAND_15, BAND_MODE_10 },
		{ BAND_FILTER_UPPER_6, FILTER_BAND_6, BAND_MODE_6 },
		{ BAND_FILTER_UPPER_4, FILTER_BAND_4, BAND_MODE_4 }
};


/**
 * @brief Select and activate the correct BPF for the frequency given in @p freq
 *
 *
 * @param freq The frequency to activate the BPF for in Hz
 *
 * @warning  If the frequency given in @p freq is too high for any of the filters, no filter change is executed.
 */
static void UiDriverCheckFilter(ulong freq)
{
	int idx;
	for (idx = 0; idx < BAND_FILTER_NUM; idx++) {
		if(freq < bandFilters[idx].upper)	{	// are we low enough if frequency for the 160 meter filter?
			if(ts.filter_band != bandFilters[idx].filter_band)	{
				UiDriverChangeBandFilter(bandFilters[idx].band_mode);	// yes - set to 160 meters
				ts.filter_band = bandFilters[idx].filter_band;

			}
			break;
		}
	}
}

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
	//
	freq -= audio_driver_xlate_freq()*4;

	while((!flag) && (band_scan < MAX_BANDS))	{
		if((freq >= bandInfo[band_scan].tune) && (freq <= (bandInfo[band_scan].tune + bandInfo[band_scan].size)))	// Is this frequency within this band?
			flag = 1;	// yes - stop the scan
		else	// no - not in this band
			band_scan++;	// scan the next band
	}
	//
	if(update)	{		// are we to update the display?
		if(band_scan != band_scan_old)		// yes, did the band actually change?
			UiDriverShowBand(band_scan);	// yes, update the display with the current band
	}
	band_scan_old = band_scan;	// update band change detector

	return band_scan;		// return with the band

}
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateFrequency
//* Object              :
//* Input Parameters    :skip_encoder_check: If TRUE, do not check encoder;  1 = do not check, 2 = do not check AND unconditionally update synthesizer EVEN IF frequency did not change
//*						:mode: =0 automatic, 1=force large, 2=force small, upper (RX), 3 = small, lower (TX)
//*                      WARNING:  If called with "mode = 3", you must ALWAYS call again with "mode = 2" to reset internal variables.
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverUpdateFrequency(char skip_encoder_check, uchar mode)
{
	ulong		loc_tune_new, dial_freq, second_freq;
	//uchar		old_rf_gain = ts.rf_gain;
	ushort		col = White;

	// On band change we don't have to check the encoder
	if(skip_encoder_check)
		goto skip_check;

	// Check encoder
	if(!UiDriverCheckFrequencyEncoder())
		return;

skip_check:

	// Get value, while blocking update

	if(mode == 3)	{				// are we updating the TX frequency (small, lower display)?
		if(is_vfo_b())					// yes are we receiving with VFO B?
			loc_tune_new = vfo[VFO_A].band[ts.band].dial_value;		// yes - get VFO A frequency for TX
		else									// we must be receiving with VFO A
			loc_tune_new = vfo[VFO_B].band[ts.band].dial_value;		// get VFO B frequency for TX
	}
	else	// everything else uses main VFO frequency
		loc_tune_new = df.tune_new;				// yes, get that frequency

	// Calculate display frequency
	dial_freq = loc_tune_new/4;

	//
	// Do "Icom" style frequency offset of the LO if in "CW OFFSET" mode.  (Display freq. is also offset!)
	//
	if(ts.dmod_mode == DEMOD_CW)	{		// In CW mode?
		if(ts.cw_offset_mode == CW_OFFSET_USB_SHIFT)	// Yes - USB?
			dial_freq -= ts.sidetone_freq;				// lower LO by sidetone amount
		else if(ts.cw_offset_mode == CW_OFFSET_LSB_SHIFT)	// LSB?
			dial_freq += ts.sidetone_freq;				// raise LO by sidetone amount
		else if(ts.cw_offset_mode == CW_OFFSET_AUTO_SHIFT)	{	// Auto mode?  Check flag
			if(ts.cw_lsb)
				dial_freq += ts.sidetone_freq;			// it was LSB - raise by sidetone amount
			else
				dial_freq -= ts.sidetone_freq;			// it was USB - lower by sidetone amount
		}
	}

	// Calculate actual tune frequency
	ts.tune_freq = dial_freq*4;
	second_freq = ts.tune_freq;					// get copy for secondary display
	//
	// Update second display for RIT offset
	if(ts.txrx_mode == TRX_MODE_RX)		{
		second_freq += (ts.rit_value*80);	// Add RIT on receive
	}

	//
	// Offset dial frequency if the RX/TX frequency translation is active and we are not transmitting in CW mode
	//
	if(!((ts.dmod_mode == DEMOD_CW) && (ts.txrx_mode == TRX_MODE_TX)))	{
		ts.tune_freq += audio_driver_xlate_freq()*4;		// magnitude of shift is quadrupled at actual Si570 operating frequency
	}

	if(mode != 3)	{		// updating ONLY the TX frequency display?

		// Extra tuning actions
		if(ts.txrx_mode == TRX_MODE_RX)		{
			ts.tune_freq += (ts.rit_value*80);	// Add RIT on receive
		}


		//printf("--------------------\n\r");
		//printf("dial: %dHz, tune: %dHz\n\r",dial_freq,tune_freq);

		if((ts.tune_freq == ts.tune_freq_old) && (!ts.refresh_freq_disp) && (skip_encoder_check != 2))	// has the frequency changed and full display refresh not requested AND we are in "AUTOMATIC" mode?
			return;								// no - bail out - save time by NOT updating synthesizer!

		ts.tune_freq_old = ts.tune_freq;		// frequency change required - update change detector


		if(ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 1))	{	// did the tuning require that a large tuning step occur?
			if(ts.sysclock > RX_MUTE_START_DELAY)	{	// has system start-up completed?
				ads.agc_holder = ads.agc_val;	// grab current AGC value as synthesizer "click" can momentarily desense radio as we tune
				ts.rx_muting = 1;				// yes - mute audio output
				ts.dsp_inhibit_mute = ts.dsp_inhibit;		// get current status of DSP muting and save for later restoration
				ts.dsp_inhibit = 1;				// disable DSP during tuning to avoid disruption
				if(is_dsp_nr())	// is DSP active?
					ts.rx_blanking_time = ts.sysclock + TUNING_LARGE_STEP_MUTING_TIME_DSP_ON;	// yes - schedule un-muting of audio when DSP is on
				else
					ts.rx_blanking_time = ts.sysclock + TUNING_LARGE_STEP_MUTING_TIME_DSP_OFF;	// no - schedule un-muting of audio when DSP is off
			}
		}

		if(ts.sysclock-ts.last_tuning > 2 || ts.last_tuning == 0)	// prevention for SI570 crash due too fast frequency changes
		    {
		    // Set frequency
		    if(ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 0))
			{
			char test = ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 0);
			if(test == 1)
			    col = Red;	// Color in red if there was a problem setting frequency
			if(test == 2)
			    col = Yellow;	// Color in yellow if there was a problem setting frequency
			}
		    ts.last_tuning = ts.sysclock;
		    }
	}

	//
	// Update main frequency display
	//
		UiDriverUpdateLcdFreq(dial_freq,col, mode);
	//
	// Update second display to reflect RX frequency with RIT
	//
	if(mode != 3)	{		// do not update second display or check filters if we are updating TX frequency in SPLIT mode
		UiDriverUpdateLcdFreq(second_freq/4,Grey,4);
		// set mode parameter to 4 to update secondary display
		//
		UiDriverCheckFilter(ts.tune_freq/4);	// check the filter status with the new frequency update
		UiDriverCheckBand(ts.tune_freq, 1);		// check which band in which we are currently tuning and update the display
	}

	// Save current freq
	df.tune_old = loc_tune_new;

	// new drawing of frequencyscale for WF / Scope
	sd.dial_moved = 1;
}

//*----------------------------------------------------------------------------
//* Function Name       :
//* Object              : like upper, but no UI update
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverUpdateFrequencyFast(void)
{
	ulong		loc_tune_new,dial_freq;

	// Get value, while blocking update
	loc_tune_new = df.tune_new;

	// Calculate display frequency
	dial_freq = loc_tune_new/4;

	//
	// Do "Icom" style frequency offset of the LO if in "CW OFFSET" mode.  (Display freq. is also offset!)
	//
	if(ts.dmod_mode == DEMOD_CW)	{		// In CW mode?
		if(ts.cw_offset_mode == CW_OFFSET_USB_SHIFT)	// Yes - USB?
			dial_freq -= ts.sidetone_freq;				// lower LO by sidetone amount
		else if(ts.cw_offset_mode == CW_OFFSET_LSB_SHIFT)	// LSB?
			dial_freq += ts.sidetone_freq;				// raise LO by sidetone amount
		else if(ts.cw_offset_mode == CW_OFFSET_AUTO_SHIFT)	{	// Auto mode?  Check flag
			if(ts.cw_lsb)
				dial_freq += ts.sidetone_freq;			// it was LSB - raise by sidetone amount
			else
				dial_freq -= ts.sidetone_freq;			// it was USB - lower by sidetone amount
		}
	}


	// Clear not used segments on display frequency
/*	Commented out so that numbers to right of the selected step are NOT cleared - 20140906 KA7OEI
	dial_freq /= df.tunning_step;
	dial_freq *= df.tunning_step;
*/
	// Calculate actual tune frequency
	ts.tune_freq = dial_freq*4;

	//
	// Offset dial frequency if the RX/TX frequency translation is active
	//
	if(!((ts.dmod_mode == DEMOD_CW) && (ts.txrx_mode == TRX_MODE_TX)))	{
			ts.tune_freq += audio_driver_xlate_freq()*4;
	}

	// Extra tuning actions
	if(ts.txrx_mode == TRX_MODE_RX)
	{
		// Add RIT on receive
		ts.tune_freq += (ts.rit_value*80);
		//
	}
	//
	// detect - and eliminate - unnecessary synthesizer frequency changes
	//
	if((ts.tune_freq == ts.tune_freq_old) && (!ts.refresh_freq_disp))	// did the frequency NOT change and display refresh NOT requested??
		return;		// yes - bail out - no need to do anything!

	ts.tune_freq_old = ts.tune_freq;		// frequency change is required - save change detector


	//printf("--------------------\n\r");
	//printf("dial: %dHz, tune: %dHz\n\r",dial_freq,tune_freq);

	// Set frequency
	ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 0);

	// Allow clear of spectrum display in its state machine
	sd.dial_moved = 1;

	// Save current freq
	df.tune_old = loc_tune_new;

	// Save the tunning step used during the last dial update
	// - really important so we know what segments to clear
	// during tune step change
//	df.last_tune_step = df.tuning_step;

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
        for (idx = 0;idx < 9;idx++){
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
	if(mode == 4 || ts.frequency_lock) {
		// Frequency is locked - change color of display
		color = Grey;
	}

	//
	if(!mode)	{
		if(is_splitmode()) {	// in "split" mode?
			mode = 2;				// yes - update upper, small digits (receive frequency)
		} else {
			mode = 1;				// NOT in split mode:  large, normal-sized digits
		}
	}

	if (mode != 4) {
		ts.refresh_freq_disp = true; //because of coloured digits...
	}
	if(ts.xverter_mode)	{	// transverter mode active?
		dial_freq *= (ulong)ts.xverter_mode;	// yes - scale by LO multiplier
		dial_freq += ts.xverter_offset;	// add transverter frequency offset
		if(dial_freq > 1000000000)		// over 1000 MHz?
			dial_freq -= 1000000000;		// yes, offset to prevent overflow of display
		if(ts.xverter_mode && mode != 4)	// if in transverter mode, frequency is yellow unless we do the secondary display
			color = Yellow;
	}
	//
	// Handle frequency display offset in "CW RX" modes
	//
	if(ts.dmod_mode == DEMOD_CW)	{		// In CW mode?
		if((ts.cw_offset_mode == CW_OFFSET_LSB_RX) || (ts.cw_offset_mode == CW_OFFSET_LSB_SHIFT))	// Yes - In an LSB mode with display offset?
			dial_freq -= ts.sidetone_freq;															// yes, lower display freq. by sidetone amount
		else if((ts.cw_offset_mode == CW_OFFSET_USB_RX) || (ts.cw_offset_mode == CW_OFFSET_USB_SHIFT))	// In a USB mode with display offset?
			dial_freq += ts.sidetone_freq;															// yes, raise display freq. by sidetone amount
		else if((ts.cw_offset_mode == CW_OFFSET_AUTO_RX) || (ts.cw_offset_mode == CW_OFFSET_AUTO_SHIFT))	{	// in "auto" mode with display offset?
			if(ts.cw_lsb)
				dial_freq -= ts.sidetone_freq;		// yes - LSB - lower display frequency by sidetone amount
			else
				dial_freq += ts.sidetone_freq;		// yes - USB - raise display frequency by sidetone amount
		}
	}
	switch(mode) {
	case 2:
		digits_ptr  = df.dial_digits;
		digit_size = 0;
		pos_y_loc = POS_TUNE_FREQ_Y;
		pos_x_loc = POS_TUNE_SPLIT_FREQ_X;
		font_width = SMALL_FONT_WIDTH;
		break;
	case 3:					// small digits in lower location
		digits_ptr  = df.dial_digits;
		digit_size = 0;
		pos_y_loc = POS_TUNE_SPLIT_FREQ_Y_TX;
		pos_x_loc = POS_TUNE_SPLIT_FREQ_X;
		font_width = SMALL_FONT_WIDTH;
		break;
	case 4:
		digits_ptr  = df.sdial_digits;
		digit_size = 0;
		pos_y_loc = POS_TUNE_SFREQ_Y;
		pos_x_loc = POS_TUNE_SFREQ_X;
		font_width = SMALL_FONT_WIDTH;
		break;
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

	if(is_up)
	{
		// Increase step index or reset
			idx++;
			if(idx >= T_STEP_MAX_STEPS)
				idx = 0;

	}
	else
	{
		// Decrease step index or reset
		if(idx)
			idx--;
		else
			idx = (T_STEP_MAX_STEPS - 1);
	}

	// Update publics
	if(ts.freq_cal_adjust_flag)	{		// are we in "calibrate" mode?
		if(idx > T_STEP_1KHZ_IDX)	{	// yes - limit to 1 kHz steps, maximum
			if(is_up)	// did we get there via an increase?
				idx = 0;	// yes - wrap back around to minimum step size
			else		// we go there via a decrease
				idx = T_STEP_1KHZ_IDX;
		}
	}
	//	if ts.freq_cal_adjust_flag is set, do NOTHING
	//
	else if((!ts.freq_cal_adjust_flag) && (!ts.xvtr_adjust_flag) && (!ts.xverter_mode))	{			// are we NOT in "transverter adjust" or "frequency calibrate adjust" or transverter mode *NOT* on?
		if(idx > T_STEP_100KHZ_IDX)	{	// yes - limit to 100 kHz steps, maximum
			if(is_up)	// did we get ther via an increase?
				idx = 0;	// yes - wrap back around to minimum step size
			else
				idx = T_STEP_100KHZ_IDX;
		}
		// Allow 1 and 10 MHz steps if the above "else-if" condition is NOT true.
	}

	df.tuning_step	= tune_steps[idx];
	df.selected_idx = idx;

	//printf("step_n: %d\n\r",  df.tunning_step);

	// Update step on screen
	UiDriverShowStep(idx);

	// Save to Eeprom
	//TRX4M_VE_WriteStep(idx);
}
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverButtonCheck
//* Object              : Scans buttons 0-16:  0-15 are normal buttons, 16 is power button
//* Input Parameters    : button_num - 0-16:  Anything >=16 returns power button status
//* Output Parameters   : FALSE if button is pressed
//* Functions called    :
//*----------------------------------------------------------------------------
//
static uchar UiDriverButtonCheck(ulong button_num)
{
	if(button_num < 16)
	    {				// buttons 0-15 are the normal keypad buttons
	    if(!ts.boot_halt_flag)		// are we NOT in "boot halt" mode?
		return(GPIO_ReadInputDataBit(bm[button_num].port,bm[button_num].button));		// in normal mode - return key value
	    else
		return(1);						// we ARE in "load defaults" mode - always return "not pressed" (1) for buttons 0-15
	    }
	if(button_num == 16)					// button 16 is the power button
	    return(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13));
	if(button_num == 17)					// 17 used for touchscreen
	    return(GPIO_ReadInputDataBit(TP_IRQ_PIO,TP_IRQ));

	return 1; // always return "not pressed" (1) for buttons which do not exist
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
static void UiDriverTimeScheduler(void)
{
	ulong i;
	static bool	 unmute_flag = 1;
	static bool	 trx_timer_set = 0;
	static bool was_tx = 0;			// used to detect if we have returned from TX (for switching main screen items)
	static bool was_rx = 0;			// used to detect if we have entered TX from RX
	static bool old_squelch = 0;	// used to detect change-of-state of squelch
	static bool old_tone_det = 0;	// used to detect change-of-state of tone decoder
	static bool old_tone_det_enable = 0;	// used to detect change-of-state of tone decoder enabling
	static bool old_burst_active = 0;		// used to detect state of change of tone burst generator
	static bool startup_flag = 0;
	static uchar enc_one_mode = ENC_ONE_MODE_AUDIO_GAIN;	// stores modes of encoder when we enter TX
	static uchar enc_three_mode = ENC_THREE_MODE_CW_SPEED;	// stores modes of encoder when we enter TX
	static bool	dsp_rx_reenable_flag = 0;
	static ulong dsp_rx_reenable_timer = 0;
	static uchar dsp_crash_count = 0;
	static uchar press_hold_release_delay = 0;

	//
	// TR->RX audio un-muting timer and Audio/AGC De-Glitching handler
	//
	if(ts.audio_unmute)	{						// are we returning from TX with muted audio?
		if(ts.dmod_mode == DEMOD_CW)	{		// yes - was it CW mode?
			ts.unmute_delay_count = (ulong)ts.cw_rx_delay + 1;	// yes - get CW TX->RX delay timing
			ts.unmute_delay_count++;
			ts.unmute_delay_count *= 40;	// rescale value and limit minimum delay value
			trx_timer_set = 1;				// indicate that tx->rx timer is set
		}
		else	{								// SSB mode
			ts.unmute_delay_count = SSB_RX_DELAY;	// set time delay in SSB mode
			ts.buffer_clear = 1;
			trx_timer_set = 1;				// indicate that tx->rx timer is set
		}
		//
		ts.audio_unmute = 0;					// clear flag that indicates return from CW mode
	}
	//
	if(!ts.unmute_delay_count && trx_timer_set)	{		//	// did timer hit zero the first time?
		unmute_flag = 1;
		ts.buffer_clear = 0;
		ads.agc_val = ads.agc_holder;		// restore AGC value that was present when we went to TX
		trx_timer_set = 0;					// indicate that we've now finished the timeout
	}
	//
	// Audio un-muting handler and volume control handler
	//
	if(ts.txrx_mode != TRX_MODE_TX)	{
		was_rx = 1;			// set flag to indicate that we are in RX mode
		if(ts.boot_halt_flag)	{	// are we halting boot?
			ts.rx_gain[RX_AUDIO_SPKR].active_value = 0;	// yes - null out audio
			Codec_Volume(0);
		}
		else if((ts.rx_gain[RX_AUDIO_SPKR].value != ts.rx_gain[RX_AUDIO_SPKR].value_old) || (unmute_flag) || ts.band_change)	{	// in normal mode - calculate volume normally
			ts.rx_gain[RX_AUDIO_SPKR].value_old = ts.rx_gain[RX_AUDIO_SPKR].value;
			ts.rx_gain[RX_AUDIO_SPKR].active_value = 1;		// software gain not active - set to unity
			if(ts.rx_gain[RX_AUDIO_SPKR].value <= 16)				// Note:  Gain > 16 adjusted in audio_driver.c via software
				Codec_Volume((ts.rx_gain[RX_AUDIO_SPKR].value*5));
			else	{	// are we in the "software amplification" range?
				Codec_Volume((80));		// set to fixed "maximum" gain
				ts.rx_gain[RX_AUDIO_SPKR].active_value = (float)ts.rx_gain[RX_AUDIO_SPKR].value;	// to float
				ts.rx_gain[RX_AUDIO_SPKR].active_value /= 2.5;	// rescale to reasonable step size
				ts.rx_gain[RX_AUDIO_SPKR].active_value -= 5.35;	// offset to get gain multiplier value
			}
			//
			unmute_flag = 0;
			if(ts.band_change)	{	// did we un-mute because of a band change
				ts.band_change = 0;		// yes, reset the flag
				ads.agc_val = ads.agc_holder;	// restore previously-stored AGC value before the band change (minimize "POP" desense)
			}
			//
			dsp_rx_reenable_flag = 1;		// indicate that we need to re-enable the DSP soon
			dsp_rx_reenable_timer = ts.sysclock + DSP_REENABLE_DELAY;	// establish time at which we re-enable the DSP
		}
	}
	//
	//
	// Check to see if we need to re-enable DSP after return to RX
	//
	if(dsp_rx_reenable_flag)	{	// have we returned to RX after TX?
		if(ts.sysclock > dsp_rx_reenable_timer)	{	// yes - is it time to re-enable DSP?
			ts.dsp_inhibit = 0;		// yes - re-enable DSP
			dsp_rx_reenable_flag = 0;	// clear flag so we don't do this again
		}
	}
	//
	// Check to see if we need to re-enabled DSP after disabling after a function that disables the DSP (e.g. band change)
	//
	if(ts.dsp_timed_mute)	{
		if(ts.sysclock > ts.dsp_inhibit_timing)	{
			ts.dsp_timed_mute = 0;
			ts.dsp_inhibit = 0;
		}
	}
	//
	if(!(ts.misc_flags1 & 1))	{			// If auto-switch on TX/RX is enabled
		if(ts.txrx_mode == TRX_MODE_TX)	{
			if(!was_tx)	{
				was_tx = 1;		// set flag so that we only change this once, as entering
				// change display related to encoder one to TX mode (e.g. Sidetone gain or Compression level)
				//
				enc_one_mode = ts.enc_one_mode;
				ts.enc_one_mode = ENC_ONE_MODE_ST_GAIN;
				UiDriverChangeAfGain(0);	// Audio gain disabled
				//
				if(ts.dmod_mode != DEMOD_CW)
					UiDriverChangeCmpLevel(1);	// enable compression adjust if voice mode
				else
					UiDriverChangeStGain(1);	// enable sidetone gain if CW mode

				//
				// change display related to encoder one to TX mode (e.g. CW speed or MIC/LINE gain)
				//
				enc_three_mode = ts.enc_thr_mode;
				ts.enc_thr_mode = ENC_THREE_MODE_CW_SPEED;
				UiDriverChangeRit(0);
				if(ts.dmod_mode != DEMOD_CW)
					UiDriverChangeAudioGain(1);		// enable audio gain
				else
					UiDriverChangeKeyerSpeed(1);	// enable keyer speed if it was CW mode

			}
		}
		else	{	// In RX mode
			if(was_tx)	{ 	// were we latched in TX mode?
				//
				// Yes, Switch to audio gain mode
				//
				ts.enc_one_mode = enc_one_mode;
				if(ts.enc_one_mode == ENC_ONE_MODE_AUDIO_GAIN)	{	// are we to switch back to audio mode?
					UiDriverChangeAfGain(1);	// Yes, audio gain enabled
					if(ts.dmod_mode != DEMOD_CW)
						UiDriverChangeCmpLevel(0);	// disable compression level (if in voice mode)
					else
						UiDriverChangeStGain(0);	// disable sidetone gain (if in CW mode)

				}
				//
				ts.enc_thr_mode = enc_three_mode;
				if(ts.enc_thr_mode == ENC_THREE_MODE_RIT)	{		// are we to switch back to RIT mode?
					UiDriverChangeRit(1);			// enable RIT
					if(ts.dmod_mode != DEMOD_CW)
						UiDriverChangeAudioGain(0);		// disable audio gain if it was voice mode
					else
						UiDriverChangeKeyerSpeed(0);	// disable keyer speed if it was CW mode

				}
				was_tx = 0;		// clear flag indicating that we'd entered TX mode
			}
		}
	}
	//

	//
	if((was_rx) && (ts.txrx_mode == TRX_MODE_TX) && (ts.dmod_mode != DEMOD_CW))	{	// did we just enter TX mode in voice mode?
		was_rx = 0;		// yes - clear flag
		ts.tx_audio_muting_timer = ts.tx_audio_muting_timing + ts.sysclock;				// calculate expiry time for audio muting
		ts.tx_audio_muting_flag = 1;
		ads.alc_val = 1;	// re-init AGC value
		ads.peak_audio = 0;	// clear peak reading of audio meter
	}
	//
	// Did the TX muting expire?
	//
	if(ts.sysclock > ts.tx_audio_muting_timer)	{
		ts.tx_audio_muting_flag = 0;				// Yes, un-mute the transmit audio
	}

	//
	// Has the timing for the tone burst expired?
	//
	if(ts.sysclock > ts.fm_tone_burst_timing)	{
		ads.fm_tone_burst_active = 0;				// yes, turn the tone off
	}
	//
	//
	// update the on-screen indicator of squelch/tone detection (the "FM" mode text) if there is a change of state of squelch/tone detection
	//
	if(!ts.boot_halt_flag)	{	// do this only if not in "boot halt" mode
		if((old_squelch != ads.fm_squelched) || (old_tone_det != ads.fm_subaudible_tone_detected) || (old_tone_det_enable != (bool)ts.fm_subaudible_tone_det_select) || ((ads.fm_tone_burst_active) != old_burst_active))	{	// did the squelch or tone detect state just change?
			UiDriverShowMode();							// yes - update on-screen indicator to show that squelch is open/closed
			old_squelch = ads.fm_squelched;
			old_tone_det = ads.fm_subaudible_tone_detected;
			old_tone_det_enable = (bool)ts.fm_subaudible_tone_det_select;
			old_burst_active = ads.fm_tone_burst_active;
		}
	}
	///
	// DSP crash detection
	//
	if((is_dsp_nr()) && (!(is_dsp_nr_postagc())) && (!ads.af_disabled) && (!ts.dsp_inhibit))	{	// Do this if enabled and "Pre-AGC" DSP NR enabled
		if((ads.dsp_nr_sample > DSP_HIGH_LEVEL)	|| (ads.dsp_nr_sample == -1)){		// is the DSP output very high, or wrapped around to -1?
			dsp_crash_count+=2;			// yes - increase detect count quickly
		}
		else	{						// not high level
			if(dsp_crash_count)			// decrease detect count more slowly
				dsp_crash_count--;
		}
		if((ads.dsp_zero_count > DSP_ZERO_COUNT_ERROR) || (dsp_crash_count > DSP_CRASH_COUNT_THRESHOLD))	{	// is "zero" output count OR high level count exceeding threshold?
			ts.reset_dsp_nr = 1;				// yes - DSP has likely crashed:  Set flag to reset DSP NR coefficients
			audio_driver_set_rx_audio_filter();	// update DSP settings
			ts.reset_dsp_nr = 0;				// clear "reset NR coefficients" flag
			dsp_crash_count = 0;				// clear crash count flag
		}
	}
	//
	// This delays the start-up of the DSP for several seconds to minimize the likelihood that the LMS function will get "jammed"
	// and stop working.  It also does a delayed detection - and action - on the presence of a new version of firmware being installed.
	//
	if((ts.sysclock > DSP_STARTUP_DELAY) && (!startup_flag))	{	// has it been long enough after startup?
		if((ts.version_number_build != TRX4M_VER_BUILD) || (ts.version_number_release != TRX4M_VER_RELEASE) || (ts.version_number_minor != TRX4M_VER_MINOR))	{	// Yes - check for new version
			ts.version_number_build = TRX4M_VER_BUILD;	// save new F/W version
			ts.version_number_release = TRX4M_VER_RELEASE;
			ts.version_number_minor = TRX4M_VER_MINOR;
			UiSpectrumClearDisplay();			// clear display under spectrum scope
			UiLcdHy28_PrintText(110,156,"- New F/W detected -",Cyan,Black,0);
			UiLcdHy28_PrintText(110,168," Preparing EEPROM ",Cyan,Black,0);
			UiConfiguration_SaveEepromValues();	// rewrite EEPROM values
			Write_EEPROM(EEPROM_VERSION_NUMBER, ts.version_number_release);	// save version number information to EEPROM
			Write_EEPROM(EEPROM_VERSION_BUILD, ts.version_number_build);	//
			Write_EEPROM(EEPROM_VERSION_MINOR, ts.version_number_minor);	//
			for(i = 0; i < 6; i++)			// delay so that it may be read
				non_os_delay();
			UiLcdHy28_PrintText(110,180,"      Done!       ",Cyan,Black,0);
			for(i = 0; i < 6; i++)			// delay so that it may be read
				non_os_delay();
			//
			UiSpectrumInitSpectrumDisplay();			// init spectrum scope
		}
		//
		ts.dsp_inhibit = 0;					// allow DSP to function
		unmute_flag = 1;					// set unmute flag to force audio to be un-muted - just in case it starts up muted!
		startup_flag = 1;					// set flag so that we do this only once
		Codec_Mute(0);						// make sure that audio is un-muted
		//
	}
	//

	if((ts.sysclock >= ts.rx_blanking_time) && (ts.rx_muting) && (ts.rx_blanking_time> RX_MUTE_START_DELAY))	{
			// is it time to un-mute audio AND have we NOT done it already AND is it long enough after start-up to allow muting?
		ads.agc_val = ads.agc_holder;			// restore AGC setting
		ts.dsp_inhibit = ts.dsp_inhibit_mute;	// restore previous state of DSP inhibit flag
		ts.rx_muting = 0;						// unmute receiver audio
	}


	//
	// Process LCD auto-blanking
	if(ts.lcd_backlight_blanking & 0x80)	{	// is LCD auto-blanking enabled?
		if(ts.sysclock > ts.lcd_blanking_time)	{	// has the time expired and the LCD should be blanked?
			ts.lcd_blanking_flag = 1;				// yes - blank the LCD

		}
		else									// time not expired
			ts.lcd_blanking_flag = 0;				// un-blank the LCD
	}
	else								// auto-blanking NOT enabled
		ts.lcd_blanking_flag = 0;				// always un-blank the LCD in this case
	//
	//
	// State machine - processing old click
	if(ks.button_processed)
		return;

	// State machine - click or release(debounce filter)
	if(!ks.button_pressed)	{
		// Scan inputs - 16 buttons in total, but on different ports
		for(i = 0; i < 18; i++)	{		// button "17" is touchscreen
			// Read each pin of the port, based on the declared pin map
			if(!UiDriverButtonCheck(i))	{
				// Change state to clicked
				ks.button_id		= i;
				ks.button_pressed	= 1;
				ks.button_released	= 0;
				ks.button_just_pressed	= 0;
				ks.debounce_time 	= 0;
				ks.debounce_check_complete	= 0;
				ks.press_hold 		= 0;
				//printf("button_pressed %02x\n\r",ks.button_id);
				// Exit, we process just one click at a time
				break;
			}
		}
	}
	else if((ks.debounce_time >= BUTTON_PRESS_DEBOUNCE) && (!ks.debounce_check_complete))	{
		if(!UiDriverButtonCheck(ks.button_id))	{	// button still pressed?
			ks.button_just_pressed = 1;	// yes!
			ks.debounce_check_complete = 1;	// indicate that the debounce check was completed
		}
		else
			ks.button_pressed = 0;			// debounce incomplete, button released - cancel detection
	}
	else if((ks.debounce_time >= BUTTON_HOLD_TIME) && (!ks.press_hold))	{	// press-and-hold processing
		ks.button_processed = 1;						// indicate that a button was processed
		ks.button_just_pressed = 0;					// clear this flag so that the release (below) won't be detected
		ks.press_hold = 1;
		press_hold_release_delay = PRESS_HOLD_RELEASE_DELAY_TIME;	// Set up a bit of delay for when press-and-hold is released
	}
	else if(ks.press_hold && (UiDriverButtonCheck(ks.button_id)))	{	// was there a press-and-hold and the button is now released?
		if(press_hold_release_delay)					// press-and-hold delay expired?
			press_hold_release_delay--;					// no - continue counting down before cancelling "press-and-hold" mode
		else	{							// Press-and-hold mode time expired!
			ks.button_pressed = 0;			// reset and exit press-and-hold mode, this to prevent extraneous button-presses when using multiple buttons
			ks.button_released = 0;
			ks.press_hold = 0;
			ks.button_just_pressed = 0;
		}
	}
	else if(UiDriverButtonCheck(ks.button_id) && (!ks.press_hold))	{	// button released and had been debounced?
		// Change state from click to released, and processing flag on - if the button had been held down adequately
		ks.button_pressed 	= 0;
		ks.button_released 	= 1;
		ks.button_processed	= 1;
		ks.button_just_pressed = 0;
		//printf("button_released %02x\n\r",ks.button_id);
	}
	//
	// Handle press-and-hold tuning step adjustment
	//
	if((ts.tune_step != 0) && (!ks.press_hold))	{	// are we in press-and-hold step size mode and did the button get released?
		ts.tune_step = STEP_PRESS_OFF;						// yes, cancel offset
		df.selected_idx = ts.tune_step_idx_holder;			// restore previous setting
		df.tuning_step	= tune_steps[df.selected_idx];
		UiDriverShowStep(df.selected_idx);
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

	// Set SoftDDS in CW mode
	if(ts.dmod_mode == DEMOD_CW)
		softdds_setfreq((float)ts.sidetone_freq,ts.samp_rate,0);
	else
		softdds_setfreq(0.0,ts.samp_rate,0);


	// Update Decode Mode (USB/LSB/AM/FM/CW)

	UiDriverShowMode();

	AudioFilter_CalcRxPhaseAdj();		// set gain and phase values according to mode
	AudioManagement_CalcRxIqGainAdj();
	//
	AudioFilter_CalcTxPhaseAdj();
	AudioManagement_CalcTxIqGainAdj();

	// Change function buttons caption
	//UiDriverCreateFunctionButtons(false);
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
			if((!(ts.misc_flags2 & 1)) || (ts.band != BAND_MODE_10 && ts.lsb_usb_auto_select))	// is FM to be disabled?
				loc_mode++;				// yes - go to next mode
		}
	}

	if((loc_mode == DEMOD_FM) && (!ts.iq_freq_mode))	{	// are we in FM and frequency translate is off?
		loc_mode++;		// yes - FM NOT permitted unless frequency translate is active, so skip!
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


	//printf("-----------> change band\n\r");

	// Do not allow band change during TX
	if(ts.txrx_mode == TRX_MODE_TX)
		return;

	Codec_Volume(0);		// Turn volume down to suppress click
	ts.band_change = 1;		// indicate that we need to turn the volume back up after band change
	ads.agc_holder = ads.agc_val;	// save the current AGC value to reload after the band change so that we can better recover
									// from the loud "POP" that will occur when we change bands

	curr_band_index = ts.band;

	//printf("current index: %d and freq: %d\n\r",curr_band_index,tune_bands[ts.band]);

	// Save old band values
	if(curr_band_index < (MAX_BANDS))
	{
		// Save dial
		vfo[VFO_WORK].band[curr_band_index].dial_value = df.tune_old;
		vfo[VFO_WORK].band[curr_band_index].decod_mode = ts.dmod_mode;
		vfo[VFO_WORK].band[curr_band_index].filter_mode = ts.filter_id;
	}

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
	if(vfo[VFO_WORK].band[new_band_index].dial_value != 0xFFFFFFFF)
	{
		// Load old frequency from memory
		df.tune_new = vfo[VFO_WORK].band[new_band_index].dial_value;
	}
	else
	{
		// Load default band startup frequency
		df.tune_new = new_band_freq;
	}

//	UiDriverUpdateFrequency(1,0);

//	// Also reset second freq display
//	UiDriverUpdateSecondLcdFreq(df.tune_new/4);

	// Change decode mode if need to
	if(ts.dmod_mode != vfo[VFO_WORK].band[new_band_index].decod_mode)
	{
		// Update mode
		ts.dmod_mode = vfo[VFO_WORK].band[new_band_index].decod_mode;

		// Update Decode Mode (USB/LSB/AM/FM/CW)
		UiDriverShowMode();
	}

	// Change filter mode if need to
	if(ts.filter_id != vfo[VFO_WORK].band[new_band_index].filter_mode)
	{
		ts.filter_id = vfo[VFO_WORK].band[new_band_index].filter_mode;
		UiDriverChangeFilter(0);	// update display and change filter
		UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator
		audio_driver_set_rx_audio_filter();
		audio_driver_set_rx_audio_filter();	// TODO: we have to invoke the filter change several times for some unknown reason - 'dunno why!
	}

	// Create Band value
	UiDriverShowBand(new_band_index);

	// Set TX power factor
	UiDriverSetBandPowerFactor(new_band_index);

	// Set filters
	UiDriverChangeBandFilter(new_band_index);

	// Finally update public flag
	ts.band = new_band_index;
	// Display frequency update
	//
	ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
	//
	if(is_splitmode())	{	// in SPLIT mode?
		UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
		UiDriverUpdateFrequency(1,2);	// update RX frequency
	}
	else	// not in SPLIT mode - standard update
		UiDriverUpdateFrequency(1,0);
	//
	ts.refresh_freq_disp = 0;
}

/**
 * @brief Read out the changes in the frequency encoder and initiate frequency change by setting a global variable.
 *
 * @returns true if a frequency change was detected and a new tuning frequency was set in a global variable.
 */
static bool UiDriverCheckFrequencyEncoder(void)
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

	    UiLCDBlankTiming();	// calculate/process LCD blanking timing

	}
	if (pot_diff != 0 &&
			ts.txrx_mode == TRX_MODE_RX
			&& ks.button_just_pressed == false
			&& ts.frequency_lock == false)	{
		// allow tuning only if in rx mode, no freq lock,
		if (delta_t > 300) enc_speed_avg = 0; //when leaving speedy turning set avg_speed to 0

		enc_speed = div(4000,delta_t).quot*pot_diff;  // app. 4000 tics per second -> calc. enc. speed.

		if (enc_speed > 500) enc_speed = 500;   //limit calculated enc. speed
		if (enc_speed < -500) enc_speed = -500;

		enc_speed_avg = 0.1*enc_speed + 0.9*enc_speed_avg; // averaging to smooth encoder speed

		enc_multiplier = 1; //set standard speed

		if (ts.dynamic_tuning_active)   // check if dynamic tuning has been activated by touchscreen
		{
			if ((enc_speed_avg > 80) || (enc_speed_avg < (-80))) enc_multiplier = 10; // turning medium speed -> increase speed by 10
			if ((enc_speed_avg > 300) || (enc_speed_avg < (-300))) enc_multiplier = 100; //turning fast speed -> increase speed by 100

			if ((df.tuning_step == 10000) && (enc_multiplier > 10)) enc_multiplier = 10; //limit speed to 100000kHz/step
			if ((df.tuning_step == 100000) && (enc_multiplier > 1)) enc_multiplier = 1; //limit speed to 100000kHz/step
		}


		// Finaly convert to frequency incr/decr

		if(pot_diff>0) {
			df.tune_new += (df.tuning_step * 4 * enc_multiplier);
			//itoa(enc_speed,num,6);
			//UiSpectrumClearDisplay();			// clear display under spectrum scope
			//UiLcdHy28_PrintText(110,156,num,Cyan,Black,0);


		} else {
			df.tune_new -= (df.tuning_step * 4 * enc_multiplier);
		}

		if (enc_multiplier == 10) {  df.tune_new = 4*10 *df.tuning_step * div((df.tune_new/4),10* df.tuning_step).quot; } // keep last digit to zero
		if (enc_multiplier == 100){ df.tune_new = 4*100*df.tuning_step * div((df.tune_new/4),100*df.tuning_step).quot;  }// keep last 2 digits to zero))

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
static void UiDriverCheckEncoderOne(void)
{
	int 	pot_diff;

	pot_diff = UiDriverEncoderRead(ENC1);

	if (pot_diff) {
		UiLCDBlankTiming();	// calculate/process LCD blanking timing
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
static void UiDriverCheckEncoderTwo(void)
{
  //char 	temp[10];
  int 	pot_diff;

  pot_diff = UiDriverEncoderRead(ENC2);

  if (pot_diff) {
    UiLCDBlankTiming();	// calculate/process LCD blanking timing
    if(ts.menu_mode)	{
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
static void UiDriverCheckEncoderThree(void)
{
  int 	pot_diff;

  pot_diff = UiDriverEncoderRead(ENC3);

  if (pot_diff) {
    UiLCDBlankTiming();	// calculate/process LCD blanking timing
    if(ts.menu_mode)	{
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
          UiDriverUpdateFrequency(1,0);
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
            Codec_MicBoostCheck();
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
			break;
		}

		case ENC_TWO_MODE_SIG_PROC:
		{
			// RF gain
			UiDriverChangeRfGain(0);

			// DSP/Noise Blanker
			UiDriverChangeSigProc(1);
			break;
		}

		// Disable all
		default:
		{
			// RF gain
			UiDriverChangeRfGain(0);

			// DSP/Noise Blanker
			UiDriverChangeSigProc(0);

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
static void UiDriverChangeDSPMode(void)
{
	ushort color = White;
	const char* txt;

	if(((is_dsp_nr()) || (is_dsp_notch()))) {	// DSP active and NOT in FM mode?
		color = White;
	} else	// DSP not active
		color = Grey2;
	if(ts.dmod_mode == DEMOD_FM)	{		// Grey out and display "off" if in FM mode
		txt = "DSP-OFF";
		color = Grey2;
	} else if((is_dsp_nr()) && (is_dsp_notch()) && (ts.dmod_mode != DEMOD_CW))	{
		txt = "NR+NOTC";
	} else if(is_dsp_nr())	{
		txt = "   NR  ";
	} else if(is_dsp_notch())	{
		txt = " NOTCH ";
		if(ts.dmod_mode == DEMOD_CW)
			color = Grey2;
	} else {
		txt = "DSP-OFF";
	}

	UiLcdHy28_DrawStraightLine(POS_DSPL_IND_X,(POS_DSPL_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	UiLcdHy28_PrintText((POS_DSPL_IND_X),(POS_DSPL_IND_Y),txt,color,Blue,0);
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
		{ "  RTTY ", false },
		{ "  SSTV ", false },
		{ "WSPR  A", false },
		{ "WSPR  P", false },
};

static void UiDriverChangeDigitalMode(void)
{
	ushort color = digimodes[ts.digital_mode].enabled?White:Grey2;
	const char* txt = digimodes[ts.digital_mode].label;

	// Draw line for box
	UiLcdHy28_DrawStraightLine(POS_DSPU_IND_X,(POS_DSPU_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	UiLcdHy28_PrintText((POS_DSPU_IND_X),(POS_DSPU_IND_Y),txt,color,Blue,0);
}
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangePowerLevel
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangePowerLevel(void)
{
	ushort color = White;
	const char* txt;

	switch(ts.power_level)
	{
		case PA_LEVEL_5W:
			txt = "   5W  ";
			break;
		case PA_LEVEL_2W:
			txt = "   2W  ";
			break;
		case PA_LEVEL_1W:
			txt = "   1W  ";
			break;
		case PA_LEVEL_0_5W:
			txt = "  0.5W ";
			break;
		default:
			txt = "  FULL ";
			break;
	}
    // Set TX power factor - to reflect changed power
    UiDriverSetBandPowerFactor(ts.band);

    // Draw top line
    UiLcdHy28_DrawStraightLine(POS_PW_IND_X,(POS_PW_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),txt,color,Blue,0);
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
		cw_gen_init();
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
//* Function Name       : UiDriverChangeFilter
//* Object              : change audio filter, based on public flag
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverChangeFilter(uchar ui_only_update)
{
	// Do a filter re-load
	if(!ui_only_update) {
		audio_driver_set_rx_audio_filter();
	}

	const char* filter_ptr;
	const FilterDescriptor* filter = &FilterInfo[ts.filter_id];

	// Update screen indicator
	if(ts.dmod_mode != DEMOD_FM)	{	// in modes OTHER than FM
	   filter_ptr = filter->name;
	}
	else	{		// This is the FM special case to display bandwidth
		switch(ts.fm_rx_bandwidth)	{
		case FM_RX_BANDWIDTH_7K2:
			filter_ptr = "7k2 FM";
			break;
		case FM_RX_BANDWIDTH_12K:
			filter_ptr = "12k FM";
			break;
			//			case FM_RX_BANDWIDTH_15K:
			//				filter_ptr = "15k FM";
			//				break;
		case FM_RX_BANDWIDTH_10K:
		default:
			filter_ptr = "10k FM";
			break;
		}
	}

    UiLcdHy28_PrintText(POS_FIR_IND_X,  POS_FIR_IND_Y,       "  FILT ", White,  Orange, 0);
	// Draw top line
    UiLcdHy28_DrawStraightLine(POS_FIR_IND_X,(POS_FIR_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),filter_ptr,White,Black,0);
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
void UiDriverDisplayFilterBW(void)
{
	float	width, offset, calc;
	ushort	lpos;
	bool	is_usb;
	uint32_t clr;

	if(ts.menu_mode)	// bail out if in menu mode
		return;



	// Update screen indicator - first get the width and center-frequency offset of the currently-selected filter
	//
	const FilterDescriptor* filter_p = &FilterInfo[ts.filter_id];
	const FilterConfig* config_p = &filter_p->config[ts.filter_select[ts.filter_id]];
	offset = config_p->offset;
	width = filter_p->width;

	// TODO: We cheat here a  little, until all filter configs are properly filled.
	if (ts.filter_select[ts.filter_id] != 0 && offset == 0) {
	  offset = width/2;
	}
	//
	// Special case for FM
	//
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
	if((ts.dmod_mode == DEMOD_AM) || (ts.dmod_mode == DEMOD_FM))	{	// special cases - AM and FM, which are double-sidebanded
		lpos -= width;					// line starts "width" below center
		width *= 2;						// the width is double in AM, above and below center
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
/*static void UiDriverUpdateUsbKeyboardStatus(void)
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
static void UiDriverHandleSmeter(void)
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
	//
	sm.s_count = 0;		// Init S-meter search counter
	while ((sm.gain_calc >= S_Meter_Cal[sm.s_count]) && (sm.s_count < S_Meter_Cal_Size))	{	// find corresponding signal level
		sm.s_count++;;
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

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleLowerMeter
//* Object              : Power, SWR, ALC and Audio indicator
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandleLowerMeter(void)
{
	ushort	val_p,val_s = 0;
	float	sensor_null, coupling_calc, scale_calc;
	char txt[32];
	static float fwd_pwr_avg, rev_pwr_avg;
	static uchar	old_power_level = 99;

	//float 	rho,swr;

	// Only in TX mode
	if(ts.txrx_mode != TRX_MODE_TX)	{
		swrm.vswr_dampened = 0;		// reset averaged readings when not in TX mode
		fwd_pwr_avg = -1;
		rev_pwr_avg = -1;
		return;
	}

	swrm.skip++;
	if(swrm.skip < SWR_SAMPLES_SKP)
		return;

	swrm.skip = 0;

	// Collect samples
	if(swrm.p_curr < SWR_SAMPLES_CNT)
	{
		// Get next sample
		if(!(ts.misc_flags1 & 16))	{	// is bit NOT set?  If this is so, do NOT swap FWD/REV inputs from power detectors
			val_p = ADC_GetConversionValue(ADC2);	// forward
			val_s = ADC_GetConversionValue(ADC3);	// return
		}
		else	{	// FWD/REV bits should be swapped
			val_p = ADC_GetConversionValue(ADC3);	// forward
			val_s = ADC_GetConversionValue(ADC2);	// return
		}

		// Add to accumulator to average A/D values
		swrm.fwd_calc += (float)val_p;
		swrm.rev_calc += (float)val_s;

		swrm.p_curr++;

		//printf("sample no %d\n\r",swrm.p_curr);
		return;
	}

	sensor_null = (float)swrm.sensor_null;	// get calibration factor
	sensor_null -= 100;						// offset it so that 100 = 0
	sensor_null /= 1000;					// divide so that each step = 1 millivolt

	// Compute average values

	swrm.fwd_calc /= SWR_SAMPLES_CNT;
	swrm.rev_calc /= SWR_SAMPLES_CNT;


	// Calculate voltage of A/D inputs

	swrm.fwd_calc *= SWR_ADC_VOLT_REFERENCE;	// get nominal A/D reference voltage
	swrm.fwd_calc /= SWR_ADC_FULL_SCALE;		// divide by full-scale A/D count to yield actual input voltage from detector
	swrm.fwd_calc += sensor_null;				// offset result

	swrm.rev_calc *= SWR_ADC_VOLT_REFERENCE;	// get nominal A/D reference voltage
	swrm.rev_calc /= SWR_ADC_FULL_SCALE;		// divide by full-scale A/D count to yield actual input voltage from detector
	swrm.rev_calc += sensor_null;


	// calculate forward and reverse RF power in watts (p = a + bx + cx^2) for high power (above 50-60 milliwatts) and (p = a + bx + cx^2 + dx^3) for low power.

	if(swrm.fwd_calc <= LOW_POWER_CALC_THRESHOLD)	// is this low power as evidenced by low voltage from the sensor?
		swrm.fwd_pwr = LOW_RF_PWR_COEFF_A + (LOW_RF_PWR_COEFF_B * swrm.fwd_calc) + (LOW_RF_PWR_COEFF_C * pow(swrm.fwd_calc,2 )) + (LOW_RF_PWR_COEFF_D * pow(swrm.fwd_calc, 3));
	else		// it is high power
		swrm.fwd_pwr = HIGH_RF_PWR_COEFF_A + (HIGH_RF_PWR_COEFF_B * swrm.fwd_calc) + (HIGH_RF_PWR_COEFF_C * pow(swrm.fwd_calc, 2));

	if(swrm.rev_calc <= LOW_POWER_CALC_THRESHOLD)	// is this low power as evidenced by low voltage from the sensor?
		swrm.rev_pwr = LOW_RF_PWR_COEFF_A + (LOW_RF_PWR_COEFF_B * swrm.rev_calc) + (LOW_RF_PWR_COEFF_C * pow(swrm.rev_calc, 2)) + (LOW_RF_PWR_COEFF_D * pow(swrm.rev_calc,3));
	else
		swrm.rev_pwr = HIGH_RF_PWR_COEFF_A + (HIGH_RF_PWR_COEFF_B * swrm.rev_calc) + (HIGH_RF_PWR_COEFF_C * pow(swrm.rev_calc, 2));
	//
	if(swrm.fwd_pwr < 0)	// prevent negative power readings from emerging from the equations - particularly at zero output power
		swrm.fwd_pwr = 0;
	//
	if(swrm.rev_pwr < 0)
		swrm.rev_pwr = 0;

	// obtain and calculate power meter coupling coefficients
	switch(ts.filter_band)	{
		case	FILTER_BAND_2200:
			coupling_calc = (float)swrm.coupling_2200m_calc;	// get coupling coefficient calibration for 2200 meters
			break;
		case	FILTER_BAND_630:
			coupling_calc = (float)swrm.coupling_630m_calc;	// get coupling coefficient calibration for 630 meters
			break;
		case	FILTER_BAND_160:
			coupling_calc = (float)swrm.coupling_160m_calc;	// get coupling coefficient calibration for 160 meters
			break;
		case	FILTER_BAND_80:
			coupling_calc = (float)swrm.coupling_80m_calc;	// get coupling coefficient calibration for 80 meters
			break;
		case	FILTER_BAND_6:
			coupling_calc = (float)swrm.coupling_6m_calc;	// get coupling coefficient calibration for 6 meters
			break;
		case	FILTER_BAND_20:
			coupling_calc = (float)swrm.coupling_20m_calc;	// get coupling coefficient calibration for 30/20 meters
			break;
		case	FILTER_BAND_15:
			coupling_calc = (float)swrm.coupling_15m_calc;	// get coupling coefficient calibration for 17/15/12/10 meters
			break;
		case	FILTER_BAND_40:
		default:
			coupling_calc = (float)swrm.coupling_40m_calc;	// get coupling coefficient calibration for 40/60 meters
			break;
	}
	//
	coupling_calc -= 100;						// offset to zero
	coupling_calc /= 10;						// rescale to 0.1 dB/unit

	// calculate forward and reverse RF power in dBm  (We are using dBm - just because!)

	swrm.fwd_dbm = (10 * (log10(swrm.fwd_pwr))) + 30 + coupling_calc;
	swrm.rev_dbm = (10 * (log10(swrm.rev_pwr))) + 30 + coupling_calc;

	// now convert back to watts with the coupling coefficient included

	swrm.fwd_pwr = pow10(swrm.fwd_dbm/10)/1000;
	swrm.rev_pwr = pow10(swrm.rev_dbm/10)/1000;

	swrm.vswr = swrm.fwd_dbm-swrm.rev_dbm;		// calculate VSWR

	// Calculate VSWR from power readings

	swrm.vswr = (1+sqrtf(swrm.rev_pwr/swrm.fwd_pwr))/(1-sqrt(swrm.rev_pwr/swrm.fwd_pwr));

	// display FWD, REV power, in milliwatts - used for calibration - IF ENABLED
	if(swrm.pwr_meter_disp)	{
		if((fwd_pwr_avg < 0) || (ts.power_level != old_power_level))	// initialize with current value if it was zero (e.g. reset) or power level changed
			fwd_pwr_avg = swrm.fwd_pwr;
		//
		fwd_pwr_avg = fwd_pwr_avg * (1-PWR_DAMPENING_FACTOR);	// apply IIR smoothing to forward power reading
		fwd_pwr_avg += swrm.fwd_pwr * PWR_DAMPENING_FACTOR;
		//
		if((rev_pwr_avg < 0) || (ts.power_level != old_power_level))	{	// initialize with current value if it was zero (e.g. reset) or power level changed
			rev_pwr_avg = swrm.rev_pwr;
		}
		//
		old_power_level = ts.power_level;		// update power level change detector
		//
		//
		rev_pwr_avg = rev_pwr_avg * (1-PWR_DAMPENING_FACTOR);	// apply IIR smoothing to reverse power reading
		rev_pwr_avg += swrm.rev_pwr * PWR_DAMPENING_FACTOR;
		//
		sprintf(txt, "%d,%d   ", (int)(fwd_pwr_avg*1000), (int)(rev_pwr_avg*1000));		// scale to display power in milliwatts
		UiLcdHy28_PrintText    (POS_PWR_NUM_IND_X, POS_PWR_NUM_IND_Y,txt,Grey,Black,0);
		swrm.pwr_meter_was_disp = 1;	// indicate the power meter WAS displayed
	}
	//
	if((swrm.pwr_meter_was_disp) && (!swrm.pwr_meter_disp))	{	// had the numerical display been enabled - and it is now disabled?
		UiLcdHy28_PrintText    (POS_PWR_NUM_IND_X, POS_PWR_NUM_IND_Y,"            ",White,Black,0);	// yes - overwrite location of numerical power meter display to blank it
		swrm.pwr_meter_was_disp = 0;	// clear flag so we don't do this again
	}

	//
	// used for debugging
//		char txt[32];
//		sprintf(txt, " %d,%d,%d   ", (ulong)(swrm.fwd_pwr*100), (ulong)(swrm.rev_pwr*100),(ulong)(swrm.vswr*10));
//		UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 20),txt,White,Grid,0);
	//

	//printf("aver power %d, aver ret %d\n\r", val_p,val_s);

	// Transmitter protection - not enabled yet
/*
	if(val_s > 2000)
	{
		// Display
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) + 140),(POS_SM_IND_Y + 59),"PROT",Red,Black,4);

		// Disable tx - not used for now
		//ts.tx_power_factor	= 0.0;
	}
*/
	// calculate and display RF power reading
	//
	scale_calc = (uchar)(swrm.fwd_pwr * 3);		// 3 dots-per-watt for RF power meter
	//
	UiDriverUpdateTopMeterA(scale_calc);

	//
	// Do selectable meter readings
	//
	if(ts.tx_meter_mode == METER_SWR)	{
		if(swrm.fwd_pwr >= SWR_MIN_CALC_POWER)	{		// is the forward power high enough for valid VSWR calculation?
														// (Do nothing/freeze old data if below this power level)
			//
			if(swrm.vswr_dampened < 1)	// initialize averaging if this is the first time (e.g. VSWR <1 = just returned from RX)
				swrm.vswr_dampened = swrm.vswr;
			else	{
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
		if(scale_calc < 0)
			scale_calc = 0;
		//
		UiDriverUpdateBtmMeter((uchar)(scale_calc), 22);	// update the meter, setting the "red" threshold
	}

	// Reset accumulators and variables for power measurements

	swrm.p_curr   = 0;
	swrm.fwd_calc = 0;
	swrm.rev_calc = 0;
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandlePowerSupply
//* Object              : display external voltage and to handle final power-off and delay
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandlePowerSupply(void)
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
	uchar	v1000,v10000,v100000,v1000000;
	char	digit[2];
	ulong	ttemp;

	if((df.temp_enabled & 0x0f) == TCXO_STOP)	{	// if temperature update is disabled, don't update display!
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1), " ",Grey,Black,0);	// delete asterisk
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50),(POS_TEMP_IND_Y + 1), "STOPPED",Grey,Black,0);
		return;
	}

	if((temp < 0) || (temp > 1000000))	{// is the temperature out of range?
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*1),(POS_TEMP_IND_Y + 1),"---",Grey,Black,0);
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*5),(POS_TEMP_IND_Y + 1),"-",Grey,Black,0);
	}
	else	{

		//printf("temp: %i\n\r",temp);

		// Terminate
		digit[1] = 0;

		ttemp = (ulong)temp;	// convert to long to make sure that our workspace is large enough

		if(df.temp_enabled & 0xf0)	{	// Is it Fahrenheit mode?
			ttemp *= 9;			// multiply by 1.8
			ttemp /= 5;
			ttemp += 320000;	// Add 32 degrees
		}

		// -----------------------
		// See if 100 needs update
		v1000000 = (ttemp%10000000)/1000000;
		//	if(v1000000 != lo.v1000000)
		//	{
		//printf("100 diff: %d\n\r",v1000000);

		// To string
		digit[0] = 0x30 + (v1000000 & 0x0F);

		// Update segment (optional)
		if(v1000000)
				UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*1),(POS_TEMP_IND_Y + 1),digit,Grey,Black,0);
			else
				UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*1),(POS_TEMP_IND_Y + 1),digit,Black,Black,0);

		// Save value
		lo.v1000000 = v1000000;
		//	}

		// -----------------------
		// See if 10 needs update
		v100000 = (ttemp%1000000)/100000;
		//	if(v100000 != lo.v100000)
		//	{
		//printf("10C diff: %d\n\r",v100000);

		// To string
		digit[0] = 0x30 + (v100000 & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*2),(POS_TEMP_IND_Y + 1),digit,Grey,Black,0);

		// Save value
		lo.v100000 = v100000;
		//	}

		// -----------------------
		// See if 1 needs update
		v10000 = (ttemp%100000)/10000;
		//	if(v10000 != lo.v10000)
		//	{
		//printf("1C diff: %d\n\r",v10000);

		// To string
		digit[0] = 0x30 + (v10000 & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*3),(POS_TEMP_IND_Y + 1),digit,Grey,Black,0);

		// Save value
		lo.v10000 = v10000;
		//	}

		// -----------------------
		// See if 0.1 needs update
		v1000 = (ttemp%10000)/1000;
		//	if(v1000 != lo.v1000)
		//	{
		//printf("0.1 diff: %d\n\r",v1000);

		// To string
		digit[0] = 0x30 + (v1000 & 0x0F);

		// Save value
		lo.v1000 = v1000;
		//
		// Update segment
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*5),(POS_TEMP_IND_Y + 1),digit,Grey,Black,0);
	}

//	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleLoTemperature
//* Object              : display LO temperature and compensate drift
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandleLoTemperature(void)
{
	int		temp = 0;
	int		comp, comp_p;
	float	dtemp, remain, t_index;
	uchar	tblp;
	uint32_t clr;

	// No need to process if no chip avail or updates are disabled
	if((lo.sensor_present) || ((df.temp_enabled & 0x0f) == TCXO_STOP))
		return;

	lo.skip++;
	if(lo.skip < LO_COMP_SKP)
		return;

	lo.skip = 0;

	// Get current temperature
	if(ui_si570_read_temp(&temp) != 0)
		return;

	// Get temperature from sensor with its maximum precision
	//
	dtemp = (float)temp;	// get temperature
	dtemp /= 10000;			// convert to decimal degrees
	remain = truncf(dtemp);	// get integer portion of temperature
	remain = dtemp - remain;	// get fractional portion

	// Refresh UI
	UiDriverRefreshTemperatureDisplay(1,temp);

	// Compensate only if enabled
	if(!(df.temp_enabled & 0x0f))
		return;

	//printf("temp: %i\n\r",temp);

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
	//printf("ptr: %d\n\r",tblp);

	// Update drift indicator, no overload
	UiDriverUpdateLoMeter(tblp - 30,1);

	// Check for overflow
	if(tblp > 99)
	{
		//printf("out of range\n\r");
		return;
	}

	// Value from freq table
	comp = tcxo_table_20m[tblp];				// get the first entry in the table
	comp_p = tcxo_table_20m[tblp + 1];			// get the next entry in the table to determine fraction of frequency step
	//printf("comp:  %d - %i\n\r",tblp,comp);

	comp_p = comp_p - comp;	//					// get frequency difference between the two steps

	dtemp = (float)comp_p;	// change it to float for the calculation
	dtemp *= remain;		// get proportion of temperature difference between the two steps using the fraction

	comp += (int)dtemp;		// add the compensation value to the lower of the two frequency steps

	// Change needed ?
	if(lo.comp == comp)		// is it the same value?
		return;				// yes - bail out, no change needed

	// Update frequency, without reflecting it on the LCD
	if(comp != (-1)) {
		df.temp_factor = comp;
		UiDriverUpdateFrequencyFast();

		// Save to public, to skip not needed update
		// when we are in 1C range
		lo.comp = comp;

		// Lock indicator
		clr = Blue;
	} else {
		// No compensation - commented out - keep last compensation value
		//df.temp_factor = 0;
		//UiDriverUpdateFrequencyFast();

		// Lock indicator
	    clr = Red;
	}
    UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",clr,Black,0);

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverEditMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverEditMode(void)
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
static void UiDriverSwitchOffPtt(void)
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
				ts.txrx_mode = TRX_MODE_TX;
				ui_driver_toggle_tx();
			}
		}

		ts.ptt_req = 0;
		return;
	}

	// When CAT driver is running
	// skip auto return to RX, but do the
	// delayed unmute
	if(kd.enabled)
		goto unmute_only;

	// PTT off
	if((ts.dmod_mode == DEMOD_USB)||(ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_AM) || (ts.dmod_mode == DEMOD_FM))
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
				ts.txrx_mode = TRX_MODE_RX;
				ui_driver_toggle_tx();				// PTT

				// Unlock
				//ts.txrx_lock = 0;
			}
		}
	}

unmute_only:
return;
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

	if(df.tune_new < 8000000 * 4)		// reduction for frequencies < 8 MHz
	    ts.tx_power_factor = ts.tx_power_factor / 4;
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
void UiLCDBlankTiming(void)
{
	ulong ltemp;

	if(ts.lcd_backlight_blanking & 0x80)	{	// is LCD blanking enabled?
		ltemp = (ulong)(ts.lcd_backlight_blanking & 0x0f);		// get setting of LCD blanking timing
		ltemp *= 100;		// multiply to convert to deciseconds
		ts.lcd_blanking_time = ltemp + ts.sysclock;		// calculate future time at which LCD is to be turned off
		ts.lcd_blanking_flag = 0;		// clear flag to make LCD turn on
	}
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
void UiCWSidebandMode(void)
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
//* Function Name       : UiDriverLoadFilterValue
//* Object              : Load stored filter value from EEPROM
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiDriverLoadFilterValue(void)	// Get filter value so we can init audio with it
{
	uint16_t value;

	if(Read_EEPROM(EEPROM_BAND_MODE, &value) == 0)
	{
		ts.filter_id = (value >> 12) & 0x0F;	// get filter setting
		if((ts.filter_id >= AUDIO_MAX_FILTER) || (ts.filter_id < AUDIO_MIN_FILTER))		// audio filter invalid?
			ts.filter_id = AUDIO_DEFAULT_FILTER;	// set default audio filter
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
void UiCheckForEEPROMLoadDefaultRequest(void)
{	uint16_t i;
	if((ts.version_number_build != TRX4M_VER_BUILD) || (ts.version_number_release != TRX4M_VER_RELEASE) || (ts.version_number_minor != TRX4M_VER_MINOR))	{	// Does the current version NOT match what was in the EEPROM?
		return;		// it does NOT match - DO NOT allow a "Load Default" operation this time!
	}

	if((!UiDriverButtonCheck(BUTTON_F1_PRESSED)) && (!UiDriverButtonCheck(BUTTON_F3_PRESSED)) && (!UiDriverButtonCheck(BUTTON_F5_PRESSED)))	{	// Are F1, F3 and F5 being held down?
		ts.load_eeprom_defaults = 1;						// yes, set flag to indicate that defaults will be loaded instead of those from EEPROM
		ts.boot_halt_flag = 1;								// set flag to halt boot-up
		UiConfiguration_LoadEepromValues();							// call function to load values - default instead of EEPROM
		//
		UiLcdHy28_LcdClear(Red);							// clear the screen
		//													// now do all of the warnings, blah, blah...
		UiLcdHy28_PrintText(2,05,"   EEPROM DEFAULTS",White,Red,1);
		UiLcdHy28_PrintText(2,35,"      LOADED!!!",White,Red,1);
		UiLcdHy28_PrintText(2,70,"  DISCONNECT power NOW if you do NOT",Cyan,Red,0);
		UiLcdHy28_PrintText(2,85,"  want to lose your current settings!",Cyan,Red,0);
		UiLcdHy28_PrintText(2,120,"  If you want to save default settings",Green,Red,0);
		UiLcdHy28_PrintText(2,135,"  press and hold POWER button to power",Green,Red,0);
		UiLcdHy28_PrintText(2,150,"   down and save settings to EEPROM.",Green,Red,0);
		//
		// On screen delay									// delay a bit...
		for(i = 0; i < 10; i++)
		   non_os_delay();
		//
		// add this for emphasis
		UiLcdHy28_PrintText(50,195,"     YOU HAVE BEEN WARNED!",Yellow,Red,0);
		UiLcdHy28_PrintText(2,225,"               [Radio startup halted]",White,Red,4);
	}
}
//

//
//*----------------------------------------------------------------------------
//* Function Name       : UiCheckForEEPROMLoadFreqModeDefaultRequest
//* Object              : Cause default values to be loaded for frequency/mode instead of EEPROM-stored values, show informational/warning splash screen, pause
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//* Comments            : The user MUST make a decision at that point, anyway:  To disconnect power
//  Comments            : preserve "old" settings or to power down using the POWER button to the new, default settings to EEPROM.
//  Comments            : WARNING:  Do *NOT* do this (press the buttons on power-up) when first loading a new firmware version as the EEPROM will be automatically be written over at startup!!!  [KA7OEI October, 2015]
//*----------------------------------------------------------------------------
//
void UiCheckForEEPROMLoadFreqModeDefaultRequest(void)
{	uint16_t i;
	if((ts.version_number_build != TRX4M_VER_BUILD) || (ts.version_number_release != TRX4M_VER_RELEASE) || (ts.version_number_minor != TRX4M_VER_MINOR))	{	// Does the current version NOT match what was in the EEPROM?
		return;		// it does NOT match - DO NOT allow a "Load Default" operation this time!
	}

	if((!UiDriverButtonCheck(BUTTON_F2_PRESSED)) && (!UiDriverButtonCheck(BUTTON_F4_PRESSED)))	{	// Are F2, F4 being held down?
		ts.load_freq_mode_defaults = 1;						// yes, set flag to indicate that frequency/mode defaults will be loaded instead of those from EEPROM
		ts.boot_halt_flag = 1;								// set flag to halt boot-up
		UiConfiguration_LoadEepromValues();							// call function to load values - default instead of EEPROM
		//
		UiLcdHy28_LcdClear(Yellow);							// clear the screen
		//													// now do all of the warnings, blah, blah...
		UiLcdHy28_PrintText(2,05,	"   FREQUENCY/MODE",Black,Yellow,1);
		UiLcdHy28_PrintText(2,35,	" DEFAULTS LOADED!!!",Black,Yellow,1);
		UiLcdHy28_PrintText(2,70,	"  DISCONNECT power NOW if you do NOT",Black,Yellow,0);
		UiLcdHy28_PrintText(2,85,	"want to lose your current frequencies!",Black,Yellow,0);
		UiLcdHy28_PrintText(2,120,	"If you want to save default frequencies",Black,Yellow,0);
		UiLcdHy28_PrintText(2,135,	"  press and hold POWER button to power",Black,Yellow,0);
		UiLcdHy28_PrintText(2,150,	"   down and save settings to EEPROM.",Black,Yellow,0);
		// On screen delay									// delay a bit...
		for(i = 0; i < 10; i++)
		   non_os_delay();

		// add this for emphasis
		UiLcdHy28_PrintText(50,195,"     YOU HAVE BEEN WARNED!",Black,Yellow,0);
		UiLcdHy28_PrintText(2,225,"               [Radio startup halted]",Black,Yellow,4);
	}
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
void UiDriver_KeyTestScreen(void)
{
	ushort i, j, k, p_o_state, rb_state, new_state;
	uint32_t poweroffcount, rbcount;
	bool stat = 1;
	poweroffcount = rbcount = 0;
	p_o_state = rb_state = new_state = 0;
	char txt_buf[40];
	char* txt;
	for(i = 0; i <= 17; i++)	{			// scan all buttons
		if(!UiDriverButtonCheck(i))	{		// is one button being pressed?
			stat = 0;						// yes - clear flag
		}
	}
	//
	if(stat)			// no buttons pressed
		return;			// bail out
	//
	UiLcdHy28_LcdClear(Blue);							// clear the screen

	//
	UiLcdHy28_PrintText(40,35,"  Button Test  ",White,Blue,1);
	UiLcdHy28_PrintText(15,70,"press & hold POWER-button to poweroff",White,Blue,0);
	UiLcdHy28_PrintText(20,90,"press & hold BANDM-button to reboot",White,Blue,0);
	//
	for(;;)	{		// get stuck here for test duration
		j = 99;		// load with flag value
		k = 0;

		for(i = 0; i <= 17; i++)
		{				// scan all buttons
			if(!UiDriverButtonCheck(i))
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

		switch(j)	{							// decode button to text
		case	BUTTON_POWER_PRESSED:
			txt = "POWER ";
			if(poweroffcount > 75)
			{
				txt = "powering off";
				p_o_state = 1;
			}
			poweroffcount++;
			break;
		case	BUTTON_M1_PRESSED:
			txt = "  M1  ";
			break;
		case	BUTTON_M2_PRESSED:
			txt = "  M2  ";
			break;
		case	BUTTON_M3_PRESSED:
			txt = "  M3  ";
			break;
		case	BUTTON_G1_PRESSED:
			txt = "  G1  ";
			break;
		case	BUTTON_G2_PRESSED:
			txt = "  G2  ";
			break;
		case	BUTTON_G3_PRESSED:
			txt = "  G3  ";
			break;
		case	BUTTON_G4_PRESSED:
			txt = "  G4  ";
			break;
		case	BUTTON_F1_PRESSED:
			txt = "  F1  ";
			break;
		case	BUTTON_F2_PRESSED:
			txt = "  F2  ";
			break;
		case	BUTTON_F3_PRESSED:
			txt = "  F3  ";
			break;
		case	BUTTON_F4_PRESSED:
			txt = "  F4  ";
			poweroffcount = 0;
			break;
		case	BUTTON_F5_PRESSED:
			txt = "  F5  ";
			break;
		case	BUTTON_BNDM_PRESSED:
			txt = " BNDM ";
			if(rbcount > 75)
			{
				txt = "rebooting";
				rb_state = 1;
			}
			rbcount++;
			break;
		case	BUTTON_BNDP_PRESSED:
			txt = " BNDP ";
			break;
		case	BUTTON_STEPM_PRESSED:
			txt = "STEPM ";
			break;
		case	BUTTON_STEPP_PRESSED:
			txt = "STEPP ";
			break;
		case	TOUCHSCREEN_ACTIVE: ;
			UiLcdHy28_GetTouchscreenCoordinates(1);
			sprintf(txt_buf,"%02d%s%02d%s",ts.tp_x,"  ",ts.tp_y,"  ");	//show touched coordinates
			txt = txt_buf;
			break;
		default:
			txt = "<none>";		// no button pressed
			poweroffcount = 0;
			rbcount = 0;
		}
		//
		UiLcdHy28_PrintText(120,120,txt,White,Blue,1);		// identify button on screen
		sprintf(txt_buf, "# of buttons pressed: %d  ", (int)k);
		UiLcdHy28_PrintText(75,160,txt_buf,White,Blue,0);		// show number of buttons pressed on screen

		if(p_o_state == 1)
		{
			GPIO_SetBits(POWER_DOWN_PIO,POWER_DOWN);
			for(;;);
		}
		if(rb_state == 1)
		{
			if(j != BUTTON_BNDM_PRESSED)
			{
				ui_si570_get_configuration();			// restore SI570 to factory default
				*(__IO uint32_t*)(SRAM2_BASE) = 0x55;
				NVIC_SystemReset();
			}
		}
	}
}
