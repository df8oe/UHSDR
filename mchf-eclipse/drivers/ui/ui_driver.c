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

// Common
#include "mchf_board.h"

#include <stdio.h>
#include "arm_math.h"
#include "math.h"
#include "codec.h"
#include "ui_menu.h"
#include "waterfall_colours.h"
//
//
// LCD
#include "ui_lcd_hy28.h"

// Encoders
#include "ui_rotary.h"

// SI570 control
#include "ui_si570.h"
#include "ui_soft_tcxo.h"

// Codec control
#include "codec.h"
#include "softdds.h"

#include "audio_driver.h"
#include "hamm_wnd.h"
#include "ui_driver.h"
//#include "usbh_usr.h"

#include "cat_driver.h"

// Virtual eeprom
#include "eeprom.h"
//
//
// List of subaudible tones - used for FM
#include "fm_subaudible_tone_table.h"
//
#include "cw_gen.h"

// SSB/AM filters

#include "filters/q_rx_filter_10kHz.h"
#include "filters/i_rx_filter_10kHz.h"
//
#include "filters/q_rx_filter_7kHz5.h"
#include "filters/i_rx_filter_7kHz5.h"
//
#include "filters/q_rx_filter_6kHz.h"
#include "filters/i_rx_filter_6kHz.h"
//
#include "filters/q_rx_filter_5kHz.h"
#include "filters/i_rx_filter_5kHz.h"
//
#include "filters/q_rx_filter_3k6.h"
#include "filters/i_rx_filter_3k6.h"
//
#include "filters/q_tx_filter.h"
#include "filters/i_tx_filter.h"

#include "filters/iq_rx_filter_am_10kHz.h"
#include "filters/iq_rx_filter_am_7kHz5.h"
#include "filters/iq_rx_filter_am_6kHz.h"
#include "filters/iq_rx_filter_am_5kHz.h"
#include "filters/iq_rx_filter_am_3k6.h"
#include "filters/iq_rx_filter_am_2k3.h"

static void 	UiDriverPublicsInit(void);
static void 	UiDriverProcessKeyboard(void);
static void 	UiDriverPressHoldStep(uchar is_up);
static void 	UiDriverProcessFunctionKeyClick(ulong id);

//static void 	UiDriverShowMode(void);
//static void 	UiDriverShowStep(ulong step);
static void 	UiDriverShowBand(uchar band);
//static void 	UiDriverChangeBandFilter(uchar band,uchar bpf_only);
static void 	UiDriverCreateDesktop(void);
static void 	UiDriverCreateFunctionButtons(bool full_repaint);
//static void 	UiDriverCreateSpectrumScope(void);
//static void 	UiDriverCreateDigiPanel(void);
//
static void UiDriverDeleteSMeter(void);
static void 	UiDriverCreateSMeter(void);
static void 	UiDriverDrawWhiteSMeter(void);
static void 	UiDriverDrawRedSMeter(void);
//
static void 	UiDriverUpdateTopMeterA(uchar val,uchar old);
static void 	UiDriverUpdateBtmMeter(uchar val, uchar warn);

static void 	UiDriverInitFrequency(void);
//
static void 	UiDriverCheckFilter(ulong freq);
uchar 			UiDriverCheckBand(ulong freq, ushort update);
//static void 	UiDriverUpdateFrequency(char skip_encoder_check, uchar mode);
//static void 	UiDriverUpdateFrequencyFast(void);
static void 	UiDriverUpdateLcdFreq(ulong dial_freq,ushort color,ushort mode);
static void 	UiDriverUpdateSecondLcdFreq(ulong dial_freq);
//static void 	UiDriverChangeTuningStep(uchar is_up);
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
// encoder one
static void 	UiDriverChangeAfGain(uchar enabled);
//static void 	UiDriverChangeStGain(uchar enabled);
//static void 	UiDriverChangeKeyerSpeed(uchar enabled);
// encoder two
//static void 	UiDriverChangeRfGain(uchar enabled);
static void 	UiDriverChangeSigProc(uchar enabled);
// encoder three
static void 	UiDriverChangeRit(uchar enabled);
//static void 	UiDriverChangeFilter(uchar ui_only_update);
static void 	UiDriverProcessActiveFilterScan(void);
static void 	UiDriverChangeDSPMode(void);
static void 	UiDriverChangePowerLevel(void);
//static void 	UiDrawSpectrumScopeFrequencyBarText(void);
static void 	UiDriverFFTWindowFunction(char mode);
static void 	UiInitSpectrumScopeWaterfall();			// init spectrum scope FFT variables, clear screen, start up all-in-one
static void 	UiDriverInitSpectrumDisplay(void);
//static void 	UiDriverClearSpectrumDisplay(void);
static void 	UiDriverReDrawSpectrumDisplay(void);
static void 	UiDriverReDrawWaterfallDisplay(void);
static ulong 	UiDriverGetScopeTraceColour(void);
//static void 	UiDriverUpdateEthernetStatus(void);
//static void 	UiDriverUpdateUsbKeyboardStatus(void);
static void 	UiDriverHandleSmeter(void);
static void 	UiDriverHandleLowerMeter(void);
static void 	UiDriverHandlePowerSupply(void);
// LO TCXO routines
static void 	UiDriverUpdateLoMeter(uchar val,uchar active);
void 			UiDriverCreateTemperatureDisplay(uchar enabled,uchar create);
static void 	UiDriverRefreshTemperatureDisplay(uchar enabled,int temp);
static void 	UiDriverHandleLoTemperature(void);
//static void 	UiDriverEditMode(void);
static void 	UiDriverSwitchOffPtt(void);
//static void 	UiDriverSetBandPowerFactor(uchar band);
//
//static void		UiCalcTxIqGainAdj(void);
static void 	UiDriverLoadEepromValues(void);
void			UiDriverUpdateMenu(uchar mode);
void 			UiDriverUpdateMenuLines(uchar index, uchar mode);
void			UiDriverUpdateConfigMenuLines(uchar index, uchar mode);
void 			UiDriverSaveEepromValuesPowerDown(void);
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
T_STEP_10KHZ,
T_STEP_100KHZ,
T_STEP_1MHZ,
T_STEP_10MHZ
};


//
// Band definitions - band base frequency value
const ulong tune_bands[MAX_BANDS] = { BAND_FREQ_80,
									  BAND_FREQ_60,
									  BAND_FREQ_40,
									  BAND_FREQ_30,
									  BAND_FREQ_20,
									  BAND_FREQ_17,
									  BAND_FREQ_15,
									  BAND_FREQ_12,
									  BAND_FREQ_10};//,
//									  BAND_FREQ_GEN};

// Band definitions - band frequency size
const ulong size_bands[MAX_BANDS] = { BAND_SIZE_80,
									  BAND_SIZE_60,
									  BAND_SIZE_40,
									  BAND_SIZE_30,
									  BAND_SIZE_20,
									  BAND_SIZE_17,
									  BAND_SIZE_15,
									  BAND_SIZE_12,
									  BAND_SIZE_10};//,
//									  BAND_SIZE_GEN};

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
const float S_Meter_Cal[] =
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
//
// Bands tuning values - WORKING registers - used "live" during transceiver operation
// (May contain VFO A, B or "Memory" channel values)
//
__IO ulong band_dial_value[MAX_BANDS+1];
__IO ulong band_decod_mode[MAX_BANDS+1];
__IO ulong band_filter_mode[MAX_BANDS+1];
//
// VFO A registers
//
__IO ulong band_dial_value_a[MAX_BANDS+1];
__IO ulong band_decod_mode_a[MAX_BANDS+1];
__IO ulong band_filter_mode_a[MAX_BANDS+1];
//
// VFO B registers
//
__IO ulong band_dial_value_b[MAX_BANDS+1];
__IO ulong band_decod_mode_b[MAX_BANDS+1];
__IO ulong band_filter_mode_b[MAX_BANDS+1];
//
static int16_t test_ui_a[250];		// dummy variable - space holder
//
// ------------------------------------------------
// Transceiver state public structure
extern __IO TransceiverState 	ts;

// ------------------------------------------------
// Frequency public
__IO DialFrequency 				df;

// ------------------------------------------------
// Encoder one public
__IO EncoderOneSelection		eos;

// ------------------------------------------------
// Encoder two public
__IO EncoderTwoSelection		ews;

// ------------------------------------------------
// Encoder three public
__IO EncoderThreeSelection		ets;

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

// ------------------------------------------------
// Eeprom Saving
__IO EepromSave					es;

// ------------------------------------------------
// CAT driver state
__IO CatDriver					kd;

// move to struct ??
__IO ulong 						unmute_delay = 0;

// ------------------------------------------------
// Spectrum display
extern __IO	SpectrumDisplay		sd;

// ------------------------------------------------
// Public USB Keyboard status
extern __IO KeypadState		ks;

// ------------------------------------------------
// Public s meter
extern	__IO	SMeter			sm;
//
// Public Audio
extern __IO		AudioDriverState	ads;

// ------------------------------------------------
// Eeprom items
extern uint16_t VirtAddVarTab[NB_OF_VAR];


uchar drv_state = 0;
uchar drv_init = 0;

//
extern __IO	FilterCoeffs		fc;

//
// RX Hilbert transform (90 degree) FIR filter state tables and instances
//
static float32_t 		FirState_I[128];
extern __IO arm_fir_instance_f32 	FIR_I;
//
static float32_t 		FirState_Q[128];
extern __IO arm_fir_instance_f32 	FIR_Q;

//
// TX Hilbert transform (90 degree) FIR filter state tables and instances
//
static float 			FirState_I_TX[128];
extern __IO	arm_fir_instance_f32	FIR_I_TX;

static float 			FirState_Q_TX[128];
extern __IO	arm_fir_instance_f32	FIR_Q_TX;
//



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


#ifdef DEBUG_BUILD
	printf("ui driver init...\n\r");
#endif

	// Driver publics init
	UiDriverPublicsInit();

	// Load stored data from eeprom - some are needed for initialization
	UiDriverLoadEepromValues();

	// Init frequency publics
	UiDriverInitFrequency();

	// Load stored data from eeprom - again - as some of the values above would have been overwritten from the above
	UiDriverLoadEepromValues();
	//
	UiCalcTxCompLevel();		// calculate current settings for TX speech compressor
	//
	df.tune_new = band_dial_value[ts.band];		// init "tuning dial" frequency based on restored settings
	df.tune_old = df.tune_new;
	//
	UiCWSidebandMode();			// determine CW sideband mode from the restored frequency
	//
	UiCalcRxIqGainAdj();		// Init RX IQ gain
	//
	UiCalcRxPhaseAdj();			// Init RX IQ Phase (Hilbert transform/filter)
	//
	UiCalcTxPhaseAdj();			// Init TX IQ Phase (Hilbert transform)
	//
	UiCalcTxIqGainAdj();		// Init TX IQ gain
	//
	// Init spectrum display
	UiDriverInitSpectrumDisplay();
	UiInitSpectrumScopeWaterfall();
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
	UiDriverDisplayFilterBW();	// Update on-screen indicator of filter bandwidth

	// Set SoftDDS in CW mode
	if(ts.dmod_mode == DEMOD_CW)
		softdds_setfreq((float)ts.sidetone_freq,ts.samp_rate,0);	// set sidetone - and CW TX offset from carrier
	else
		softdds_setfreq(0.0,ts.samp_rate,0);						// no "DDS" in non-CW modes

	// Update codec volume
	//  0 - 16: via codec command
	// 17 - 30: soft gain after decoder
	Codec_Volume((ts.audio_gain*8));		// This is only approximate - it will be properly set later

	// Set TX power factor
	UiDriverSetBandPowerFactor(ts.band);

	// Reset inter driver requests flag
	ts.LcdRefreshReq	= 0;
	ts.new_band 		= ts.band;
	df.step_new 		= df.tuning_step;

	// Extra HW init
	mchf_board_post_init();

	// Acknowledge end of init for the main thread called
	// via irq(even before the init is done)
	// bit useless since 0.171 as IRQs are enabled in
	// mchf_board_post_init(), but still used by
	// ui_driver_toggle_tx() to prevent re-entrance
	drv_init = 1;

	// Do update of frequency display
	ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
	//
	if(ts.vfo_mem_mode & 0x80)	{	// in SPLIT mode?
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

if(ts.misc_flags1 & 128)	// is waterfall mode enabled?
		UiDriverReDrawWaterfallDisplay();	// yes - call waterfall update instead
	else
		UiDriverReDrawSpectrumDisplay();	// Spectrum Display enabled - do that!

	if(ts.thread_timer)			// bail out if it is not time to do this task
		return;

	ts.thread_timer = 1;		// reset flag to schedule next occurrance
	//

	switch(drv_state)
	{
		case STATE_S_METER:
			if(!ts.boot_halt_flag)

//		sprintf(txt, "%d ", (int)(ads.temp*100));		// scale to display power in milliwatts
//		UiLcdHy28_PrintText    (POS_PWR_NUM_IND_X, POS_PWR_NUM_IND_Y,txt,Grey,Black,0);

				UiDriverHandleSmeter();
			break;
		case STATE_SWR_METER:
			if(!ts.boot_halt_flag)
				UiDriverHandleLowerMeter();
			break;
		case STATE_HANDLE_POWERSUPPLY:
			UiDriverHandlePowerSupply();
			break;
		case STATE_LO_TEMPERATURE:
			if(!ts.boot_halt_flag)
				UiDriverHandleLoTemperature();
			break;
		case STATE_TASK_CHECK:
			UiDriverTimeScheduler();		// Handles live update of Calibrate between TX/RX and volume control
			break;
		case STATE_CHECK_ENC_ONE:
			if(!ts.boot_halt_flag)
				UiDriverCheckEncoderOne();
			break;
		case STATE_CHECK_ENC_TWO:
			if(!ts.boot_halt_flag)
				UiDriverCheckEncoderTwo();
			break;
		case STATE_CHECK_ENC_THREE:
			if(!ts.boot_halt_flag)
				UiDriverCheckEncoderThree();
			break;
		case STATE_UPDATE_FREQUENCY:
			if(!ts.boot_halt_flag)
				UiDriverUpdateFrequency(0,0);
			break;
		case STATE_PROCESS_KEYBOARD:
			UiDriverProcessKeyboard();
			break;
		case STATE_SWITCH_OFF_PTT:
			if(!ts.boot_halt_flag)
				UiDriverSwitchOffPtt();
			break;
		default:
			drv_state = 0;
			return;
	}
	drv_state++;

}
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************
// **************************  OBSOLETE  ***************************************

//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_irq - THIS FUNCTION IS OBSOLETE!
//* Object              : All real time processing here
//* Object              : only fast, non blocking operations
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_irq(void)
{

	// Do not run the state machine
	// before the driver init is done
	if(!drv_init)
		return;

	switch(drv_state)
	{
/*		case STATE_SPECTRUM_DISPLAY:
			if(ts.misc_flags1 & 128)		// is waterfall mode to be used?  call it instead
				UiDriverReDrawWaterfallDisplay();
			else
				UiDriverReDrawSpectrumDisplay();
			break;
*/
		case STATE_S_METER:
			UiDriverHandleSmeter();
			break;
		case STATE_SWR_METER:
			if(ts.txrx_mode == TRX_MODE_TX)	{		// do this if in TX mode
				UiDriverHandleLowerMeter();
				break;
			}
			drv_state++;					// drop through if in RX mode
		case STATE_HANDLE_POWERSUPPLY:
			UiDriverHandlePowerSupply();
			break;
		case STATE_LO_TEMPERATURE:
			UiDriverHandleLoTemperature();
			break;
		case STATE_TASK_CHECK:
			UiDriverTimeScheduler();		// Handles live update of Calibrate between TX/RX and volume control
			break;
		case STATE_CHECK_ENC_ONE:
			UiDriverCheckEncoderOne();
			break;
		case STATE_CHECK_ENC_TWO:
			UiDriverCheckEncoderTwo();
			break;
		case STATE_CHECK_ENC_THREE:
			UiDriverCheckEncoderThree();
			break;
		case STATE_UPDATE_FREQUENCY:
			UiDriverUpdateFrequency(0,0);
			break;
		case STATE_PROCESS_KEYBOARD:
			UiDriverProcessKeyboard();
			break;
		case STATE_SWITCH_OFF_PTT:
			UiDriverSwitchOffPtt();
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

	// Disable irq processing
	drv_init = 0;
	if(ts.txrx_mode == TRX_MODE_TX)
	{
		//
		// Below, in VOICE modes we mute the audio BEFORE we activate the PTT.  This is necessary since U3 is switched the instant that we do so,
		// rerouting audio paths and causing all sorts of disruption including CLICKs and squeaks.
		// We restore TX audio levels in the function "Codec_RX_TX()" according to operating mode
		//
		ts.dsp_inhibit = 1;								// disable DSP when going into TX mode
		//
		if(ts.dmod_mode == DEMOD_AM)	{		// is it AM mode?
			if(ts.power_level < PA_LEVEL_2W)	{	// is it over 2 watts?
				ts.power_level = PA_LEVEL_2W;	// yes - force to 2 watts
				//
				UiDriverChangePowerLevel();			// update the power level display
				if(ts.tune)		// recalculate sidetone gain only if transmitting/tune mode
					Codec_SidetoneSetgain();
				//
				if(ts.menu_mode)	// are we in menu mode?
					UiDriverUpdateMenu(0);	// yes, update display when we change power setting
			}
		}
		//
		if(ts.dmod_mode != DEMOD_CW)	{				// are we in a voice mode?
			if(ts.tx_audio_source != TX_AUDIO_MIC)	{	// yes - are we in LINE IN mode?
				Codec_Line_Gain_Adj(0);	// yes - momentarily mute LINE IN audio if in LINE IN mode until we have switched to TX
			}
			else	{	// we are in MIC IN mode
				Codec_Line_Gain_Adj(0);			// momentarily mute LINE IN audio until we switch modes because we will blast any connected LINE IN source until we switch
				ts.tx_mic_gain_mult_temp = ts.tx_mic_gain_mult;		// temporarily hold the mic gain value while we switch
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

	if(ts.vfo_mem_mode & 0x80)	{				// is SPLIT mode active?
		reset_freq = 1;							// yes - indicate that we WILL need to reset the synthesizer frequency
		if(ts.vfo_mem_mode & 0x40)	{				// is VFO-B active?
			if(ts.txrx_mode == TRX_MODE_TX)	{	// are we in TX mode?
				if(was_rx)	{						// did we just enter TX mode?
					band_dial_value_b[ts.band] = df.tune_new;	// yes - save current RX frequency in VFO location (B)
					was_rx = 0;						// indicate that we are now in transmit mode to prevent re-loading of frequency
				}
				df.tune_new = band_dial_value_a[ts.band];	// load with VFO-A frequency
			}
			else					// we are in RX mode
				df.tune_new = band_dial_value_b[ts.band];	// load with VFO-B frequency
		}
		else	{	// VFO-A is active
			if(ts.txrx_mode == TRX_MODE_TX)	{				// are we in TX mode?
				if(was_rx)	{								// did we just enter TX mode?
					band_dial_value_a[ts.band] = df.tune_new;	// yes - save current RX frequency in VFO location (A)
					was_rx = 0;						// indicate that we are now in transmit mode to prevent re-loading of frequency
					// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
					//
					if(sd.use_spi)
						ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
					//
				}
				df.tune_new = band_dial_value_b[ts.band];	// load with VFO-B frequency
			}
			else							// we are in RX mode
				df.tune_new = band_dial_value_a[ts.band];	// load with VFO-A frequency
		}
	}

	if((reset_freq) || (ts.rit_value) || ((ts.iq_freq_mode) && (ts.dmod_mode == DEMOD_CW)))		// Re-set frequency if RIT is non-zero or in CW mode with translate OR if in SPLIT mode and we had to retune
		UiDriverUpdateFrequencyFast();

	if((ts.menu_mode) || (was_menu))	{			// update menu when we are (or WERE) in MENU mode
		UiDriverUpdateMenu(0);
		was_menu = 1;
	}

	if(was_menu)		// if we'd displayed the menu previously, clear the flag
		was_menu = 0;
	//
	// Switch codec mode
	Codec_RX_TX();
	//
	// Enable irq processing
	drv_init = 1;
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

	// Init encoder one
	eos.value_old 			= 0;
	eos.value_new			= ENCODER_ONE_RANGE;
	eos.de_detent			= 0;

	// Init encoder two
	ews.value_old 			= 0;
	ews.value_new			= ENCODER_TWO_RANGE;
	ews.de_detent			= 0;

	// Init encoder three
	ets.value_old 			= 0;
	ets.value_new			= ENCODER_THR_RANGE;
	ets.de_detent			= 0;

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
	swrm.coupling_80m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_40m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_20m_calc		= SWR_COUPLING_DEFAULT;
	swrm.coupling_15m_calc		= SWR_COUPLING_DEFAULT;
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

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverProcessKeyboard
//* Object              : process hardcoded buttons click and hold
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverProcessKeyboard(void)
{
	bool btemp;
	uchar temp;

	if(ks.button_processed)	{
		ts.nb_disable = 1;	// disable noise blanker if button is pressed or held
		//
		UiLCDBlankTiming();	// calculate/process LCD blanking timing
		//
		//printf("button process: %02x, debounce time: %d\n\r",ks.button_id,ks.debounce_time);

		// Is it click or hold ?
		if(!ks.press_hold)	{
			// Process click
			UiKeyBeep();	// make keyboard beep, if enabled
			//
			switch(ks.button_id)
			{
				//
				case TOUCHSCREEN_ACTIVE:				// touchscreen functions
					if(ts.tp_x != 0xff)
					    {
					    if (ts.show_tp_coordinates)			// show coordinates for coding purposes
						{
						char text[10];
						sprintf(text,"%02x%s%02x",ts.tp_x," : ",ts.tp_y);
						UiLcdHy28_PrintText(POS_PWR_NUM_IND_X,POS_PWR_NUM_IND_Y,text,White,Black,0);
						}
					    if(!ts.menu_mode)		// normal operational screen
						{
						if(check_tp_coordinates(0x40,0x05,0x35,0x42))	// wf/scope bar right part
						    {
						    if(ts.misc_flags1 & 128)
							{		// is the waterfall mode active?
							ts.misc_flags1 &=  0x7f;	// yes, turn it off
							UiInitSpectrumScopeWaterfall();			// init spectrum scope
							}
						    else
							{	// waterfall mode was turned off
							ts.misc_flags1 |=  128;	// turn it on
							UiInitSpectrumScopeWaterfall();			// init spectrum scope
							}
						    UiDriverDisplayFilterBW();	// Update on-screen indicator of filter bandwidth
						    }
						if(check_tp_coordinates(0x67,0x40,0x35,0x42))	// wf/scope bar left part
						    {
						    sd.magnify = !sd.magnify;
						    ts.menu_var_changed = 1;
						    UiInitSpectrumScopeWaterfall();			// init spectrum scope
						    }
						if(check_tp_coordinates(0x67,0x0d,0x0f,0x2d))	// wf/scope frequency dial
						    {
						    df.tune_new = round((df.tune_new + 2264/(sd.magnify+1)*(0x32+0x0e*sd.magnify-ts.tp_x))/2000) * 2000;
						    ts.refresh_freq_disp = 1;			// update ALL digits
						    if(ts.vfo_mem_mode & 0x80)
							{						// SPLIT mode
							UiDriverUpdateFrequency(1,3);
							UiDriverUpdateFrequency(1,2);
							}
						    else
							UiDriverUpdateFrequency(1,0);		// no SPLIT mode
						    ts.refresh_freq_disp = 0;			// update ALL digits
						    } 
						if(check_tp_coordinates(0x10,0x05,0x74,0x80))	// right up "dB"
						    {
						    }
						}
					    else				// standard menu screen
						{
						if(check_tp_coordinates(0x10,0x05,0x74,0x80))	// right up "dB"
						    {
						    ts.show_tp_coordinates = !ts.show_tp_coordinates;
						    if(ts.show_tp_coordinates)
							UiLcdHy28_PrintText(POS_PWR_NUM_IND_X,POS_PWR_NUM_IND_Y,"enabled",Green,Black,0);
						    else
							UiLcdHy28_PrintText(POS_PWR_NUM_IND_X,POS_PWR_NUM_IND_Y,"       ",White,Black,0);
						    }
					    }
					ts.tp_x = 0xff;						// mark data as invalid
					}
					break;
				case BUTTON_G1_PRESSED:	// BUTTON_G1 - Change operational mode
					if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX))	{	// do NOT allow mode change in TUNE mode or transmit mode
						UiDriverChangeDemodMode(0);
						UiInitRxParms();				// re-init with change of mode
					}
					break;
				//
				case BUTTON_G2_PRESSED:		// BUTTON_G2
				{
					if(ts.dmod_mode != DEMOD_FM)	{ // allow selection/change of DSP only if NOT in FM
						if((!(ts.dsp_active & 1)) && (!(ts.dsp_active & 4)))	// both NR and notch are inactive
							ts.dsp_active |= 1;									// turn on NR
						else if((ts.dsp_active & 1) && (!(ts.dsp_active & 4))) {	// NR active, notch inactive
							if(ts.dmod_mode != DEMOD_CW)	{	// NOT in CW mode
								ts.dsp_active |= 4;									// turn on notch
								ts.dsp_active &= 0xfe;								// turn off NR
							}
							else	{	// CW mode - do not select notches, skip directly to "off"
								ts.dsp_active &= 0xfa;	// turn off NR and notch
							}
						}
						else if((!(ts.dsp_active & 1)) && (ts.dsp_active & 4))	//	NR inactive, notch active
							if((ts.dmod_mode == DEMOD_AM) && (ts.filter_id == AUDIO_WIDE))		// was it AM with a wide filter selected?
								ts.dsp_active &= 0xfa;			// it was AM + wide - turn off NR and notch
							else
								ts.dsp_active |= 1;				// no - turn on NR
						//
						else	{
							ts.dsp_active &= 0xfa;								// turn off NR and notch
						}
						//
						ts.dsp_active_toggle = ts.dsp_active;	// save update in "toggle" variable
						//
						ts.reset_dsp_nr = 1;				// reset DSP NR coefficients
						audio_driver_set_rx_audio_filter();	// update DSP/filter settings
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
//					}
					break;
				}
				//
				case BUTTON_G3_PRESSED:		{	// BUTTON_G3 - Change power setting
					ts.power_level++;
					//
					if(ts.dmod_mode == DEMOD_AM)	{			// in AM mode?
						if(ts.power_level >= PA_LEVEL_MAX_ENTRY)	// yes, power over 2 watts?
							ts.power_level = PA_LEVEL_2W;	// force to 2 watt mode when we "roll over"
					}
					else	{	// other modes, do not limit max power
						if(ts.power_level >= PA_LEVEL_MAX_ENTRY)
							ts.power_level = PA_LEVEL_FULL;
					}
					//
					UiDriverChangePowerLevel();
					if(ts.tune)		// recalculate sidetone gain only if transmitting/tune mode
						if(!ts.iq_freq_mode)	// Is translate mode *NOT* active?
							Codec_SidetoneSetgain();
					//
					if(ts.menu_mode)	// are we in menu mode?
						UiDriverUpdateMenu(0);	// yes, update display when we change power setting
					//
					break;
				}
				//
				case BUTTON_G4_PRESSED:		{		// BUTTON_G4 - Change filter bandwidth
					if((!ts.tune) && (ts.dmod_mode != DEMOD_FM))	{
						ts.filter_id++;
						//
						if(ts.filter_id >= AUDIO_MAX_FILTER)
							ts.filter_id = AUDIO_MIN_FILTER;
						//
						UiDriverProcessActiveFilterScan();	// make sure that filter is active - if not, find next active filter
						//
						// Change filter
						//
						UiDriverChangeFilter(0);
						//
						UiInitRxParms();		// re-init for change of filter
						//
						if(ts.menu_mode)	// are we in menu mode?
							UiDriverUpdateMenu(0);	// yes, update display when we change filters
						//
					}
					break;
				}
				//
				case BUTTON_M1_PRESSED:		// BUTTON_M1
					UiDriverChangeEncoderOneMode(0);
					break;
				//
				case BUTTON_M2_PRESSED:		// BUTTON_M2
					UiDriverChangeEncoderTwoMode(0);
					break;
				//
				case BUTTON_M3_PRESSED:		// BUTTON_M3
					UiDriverChangeEncoderThreeMode(0);
					break;
				//
				case BUTTON_STEPM_PRESSED:		// BUTTON_STEPM
					if(!(ts.freq_step_config & 0xf0))	// button swap NOT enabled
						UiDriverChangeTuningStep(0);	// decrease step size
					else		// button swap enabled
						UiDriverChangeTuningStep(1);	// increase step size
					break;
				//
				case BUTTON_STEPP_PRESSED:		// BUTTON_STEPP
					if(!(ts.freq_step_config & 0xf0))	// button swap NOT enabled
						UiDriverChangeTuningStep(1);	// increase step size
					else
						UiDriverChangeTuningStep(0);	// decrease step size
					break;
				//
				case BUTTON_BNDM_PRESSED:		// BUTTON_BNDM
					btemp = ads.af_dissabled;
					ads.af_dissabled = 0;
					//
					ts.dsp_timed_mute = 1;		// disable DSP when changing bands
					ts.dsp_inhibit = 1;
					ts.dsp_inhibit_timing = ts.sysclock + DSP_BAND_CHANGE_DELAY;	// set time to re-enable DSP
					//
					if(ts.misc_flags1 & 2)		// band up/down button swapped?
						UiDriverChangeBand(1);	// yes - go up
					else
						UiDriverChangeBand(0);	// not swapped, go down
					//
					UiInitRxParms();	// re-init because mode/filter may have changed
					//
					if(ts.menu_mode)	// are we in menu mode?
						UiDriverUpdateMenu(0);	// yes, update menu display when we change bands
					//
					ads.af_dissabled =  btemp;
					break;
				//
				case BUTTON_BNDP_PRESSED:	// BUTTON_BNDP
					btemp = ads.af_dissabled;
					ads.af_dissabled = 0;
					//
					ts.dsp_timed_mute = 1;		// disable DSP when changing bands
					ts.dsp_inhibit = 1;
					ts.dsp_inhibit_timing = ts.sysclock + DSP_BAND_CHANGE_DELAY;	// set time to re-enable DSP
					//
					if(ts.misc_flags1 & 2)		// band up/down button swapped?
						UiDriverChangeBand(0);	// yes, go down
					else
						UiDriverChangeBand(1);	// no, go up
					//
					UiInitRxParms();		// re-init because mode/filter may have changed
					//
					if(ts.menu_mode)	// are we in menu mode?
						UiDriverUpdateMenu(0);	// yes, update display when we change bands
					//
					ads.af_dissabled = btemp;
					break;
				//
				case BUTTON_POWER_PRESSED:
					if(!ts.boot_halt_flag)	{	// do brightness adjust ONLY if NOT in "boot halt" mode
						ts.lcd_backlight_brightness++;
						ts.lcd_backlight_brightness &= 3;	// limit range of brightness to 0-3
					}
					break;
				default:
					UiDriverProcessFunctionKeyClick(ks.button_id);
					break;
			}
		}
		else	{
			//
			UiKeyBeep();	// make keyboard beep, if enabled
			//
			// *******************************************************************************
			// Process press-and-hold of button(s).  Note that this can accommodate multiple buttons at once.
			// *******************************************************************************
			//
			switch(ks.button_id)	{
				case BUTTON_F1_PRESSED:	// Press-and-hold button F1:  Write settings to EEPROM
					if(ts.txrx_mode == TRX_MODE_RX)	{				// only allow EEPROM write in receive mode
						if(!ts.menu_mode)	{						// not in menu mode
							UiDriverClearSpectrumDisplay();			// clear display under spectrum scope
							UiLcdHy28_PrintText(80,160," Saving settings to EEPROM ",Cyan,Black,0);
							UiDriverSaveEepromValuesPowerDown();	// save settings to EEPROM
							for(temp = 0; temp < 6; temp++)			// delay so that it may be read
								non_os_delay();
								//
							UiInitSpectrumScopeWaterfall();			// init spectrum scope
							ts.menu_mode = 0;
						}
						else	// If in menu mode, just save data, but don't clear screen area
							UiDriverSaveEepromValuesPowerDown();	// save settings to EEPROM
						//
						ts.menu_var_changed = 0;					// clear "EEPROM SAVE IS NECESSARY" indicators
					}
					//
					if(!ts.menu_mode)	// are we in menu mode?
						UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y," MENU  ",White,Black,0);	// no - update menu button to reflect no memory save needed
					else
						UiDriverUpdateMenu(0);	// update menu display to remove indicator to do power-off to save EEPROM value
					break;
				case BUTTON_F3_PRESSED:	// Press-and-hold button F3
					// Move to the BEGINNING of the current menu structure
					if(ts.menu_mode)	{		// Are we in menu mode?
						if(ts.menu_item < MAX_MENU_ITEM)	{	// Yes - Is this within the main menu?
							if(ts.menu_item)	// is this NOT the first menu item?
								ts.menu_item = 0;	// yes - set it to the beginning of the first menu
							else	{			// this IS the first menu item
								if(ts.radio_config_menu_enable)		// yes - is the configuration menu enabled?
									ts.menu_item = (MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS)-1;	// move to the last config/adjustment menu item
								else								// configuration menu NOT enabled
									ts.menu_item = MAX_MENU_ITEM - 1;
							}
						}
						else	{		// we are within the CONFIGURATION menu
							if(ts.menu_item > MAX_MENU_ITEM)		// is this NOT at the first entry of the configuration menu?
								ts.menu_item = MAX_MENU_ITEM;	// yes, go to the first entry of the configuration item
							else		// this IS the first entry of the configuration menu
								ts.menu_item = MAX_MENU_ITEM - 1;	// go to the last entry of the main menu
						}
						UiDriverUpdateMenu(0);	// update menu display
						UiDriverUpdateMenu(1);	// update cursor
					}
					else	{			// not in menu mode - toggle between VFO/SPLIT and Memory mode
						if(!ts.vfo_mem_flag)	{		// is it in VFO mode now?
							ts.vfo_mem_flag = 1;		// yes, switch to memory mode
							UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,"  MEM ",White,Black,0);	// yes - indicate with color
						}
						else	{
							ts.vfo_mem_flag = 0;		// it was in memory mode - switch to VFO mode
							if(ts.vfo_mem_mode & 0x80)	// SPLIT mode active?
								UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y," SPLIT",SPLIT_ACTIVE_COLOUR,Black,0);	// yes - indicate with color
							else
								UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y," SPLIT",SPLIT_INACTIVE_COLOUR,Black,0);		// not active - grey
						}
						//
					}
					break;
				case BUTTON_F4_PRESSED:	// Press-and-hold button F4
					//
					// Move to the END of the current menu structure
					if(ts.menu_mode){		// are we in menu mode?
						if(ts.menu_item < MAX_MENU_ITEM)	{	// Yes - Is this within the main menu?
							if(ts.menu_item == MAX_MENU_ITEM-1)	{	// are we on the LAST menu item of the main menu?
								if(ts.radio_config_menu_enable)		// Yes - is the configuration menu enabled?
									ts.menu_item = MAX_MENU_ITEM;	// yes - go to the FIRST item of the configuration menu
								else								// configuration menu NOT enabled
									ts.menu_item = 0;				// go to the FIRST menu main menu item
							}
							else									// we had not been on the last item of the main menu
								ts.menu_item = MAX_MENU_ITEM-1;		// go to the last item in the main menu
						}
						else	{		// we were NOT in the main menu, but in the configuration menu!
							if(ts.menu_item == (MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS-1))		// are we on the last item of the configuration menu?
								ts.menu_item = 0;					// yes - go to the first item of the main menu
							else	{		// we are NOT on the last item of the configuration menu
								ts.menu_item = (MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS) - 1;		// go to the last item in the configuration menu
							}
						}
						UiDriverUpdateMenu(0);	// update menu display
						UiDriverUpdateMenu(1);	// update cursor
					}
					else	{	// not in menu mode:  Make VFO A = VFO B or VFO B = VFO A, as appropriate
						if(ts.vfo_mem_mode & 0x40)	{	// are we in VFO B mode?
							band_dial_value_a[ts.band] = df.tune_new;					// yes, copy frequency into A
							band_decod_mode_a[ts.band] = ts.dmod_mode;					// copy active VFO (B) settings into A
							band_filter_mode_a[ts.band] = ts.filter_id;
						}
						else	{	// we were in VFO A mode
							band_dial_value_b[ts.band] = df.tune_new;					// yes, copy frequency into B
							band_decod_mode_b[ts.band] = ts.dmod_mode;					// copy active VFO (A) settings into B
							band_filter_mode_b[ts.band] = ts.filter_id;
						}
						if(ts.vfo_mem_mode & 0x80)	{	// are we in SPLIT mode?
							ts.refresh_freq_disp = 1;	// yes, we need to update the TX frequency:  Make frequency display refresh all digits
							UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
							UiDriverUpdateFrequency(1,2);	// Update receive frequency
							ts.refresh_freq_disp = 0;	// disable refresh all digits flag
						}
						UiDriverClearSpectrumDisplay();			// clear display under spectrum scope
						if(ts.vfo_mem_mode & 0x40)	// VFO B active?
							UiLcdHy28_PrintText(80,160,"VFO B -> VFO A",Cyan,Black,1);		// yes, indicate copy of B into A
						else			// VFO A active
							UiLcdHy28_PrintText(80,160,"VFO A -> VFO B",Cyan,Black,1);		// indicate copy of A into B
						for(temp = 0; temp < 18; temp++)			// delay so that it may be read
							non_os_delay();
							//
						UiInitSpectrumScopeWaterfall();			// init spectrum scope
					}
					break;
				case BUTTON_F5_PRESSED:			// Button F5 was pressed-and-held - Toggle TX Disable
					if(ts.tx_disable)	{
						ts.tx_disable = 0;		// Enable TX
						UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,"  TUNE",White,Black,0);	// Make TUNE button White
					}
					else	{
						ts.tx_disable = 1;		// Disable TX
						UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,"  TUNE",Grey1,Black,0);	// Make TUNE button Grey
					}
					break;
				case BUTTON_G1_PRESSED:	// Press-and-hold button G1 - Change operational mode, but include "disabled" modes
					if((!ts.tune) && (ts.txrx_mode == TRX_MODE_RX))	{	// do NOT allow mode change in TUNE mode or transmit mode
						UiDriverChangeDemodMode(1);		// go to next mode, including disabled modes
						UiInitRxParms();
					}
						//
					break;
				//
				case BUTTON_G2_PRESSED:		// Press and hold of BUTTON_G2 - turn DSP off/on
					if(ts.dmod_mode != DEMOD_FM)	{		// do not allow change of mode when in FM
						if(ts.dsp_active & 5)	{			// is DSP NR or NOTCH active?
							ts.dsp_active_toggle = ts.dsp_active;	// save setting for future toggling
							ts.dsp_active &= 0xfa;				// turn off NR and notch
						}
						else	{		// neither notch or NR was active
							if(ts.dsp_active_toggle != 0xff)	{	// has this holder been used before?
								ts.dsp_active = ts.dsp_active_toggle;	// yes - load value
							}
						}
						audio_driver_set_rx_audio_filter();	// update DSP settings
						UiDriverChangeDSPMode();			// update on-screen display
						//
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
						ts.filter_id++;
						//
						if(ts.filter_id >= AUDIO_MAX_FILTER)
							ts.filter_id = AUDIO_MIN_FILTER;
						//
						// Change filter
						//
						UiDriverChangeFilter(0);
						UiCalcRxPhaseAdj();			// We may have changed something in the RX filtering as well - do an update
						UiDriverChangeDSPMode();	// Change DSP display setting as well
						UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator
						//
						if(ts.menu_mode)	// are we in menu mode?
							UiDriverUpdateMenu(0);	// yes, update display when we change filters
						//
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
					ts.dsp_active ^= 8;	// toggle whether or not DSP or NB is to be displayed
					//
					if(ts.enc_two_mode == ENC_TWO_MODE_RF_GAIN)
						UiDriverChangeSigProc(0);
					else
						UiDriverChangeSigProc(1);
					break;
				case BUTTON_M3_PRESSED:	// Press-and-hold button M3:  Switch display between MIC and Line-In mode
					if(ts.dmod_mode != DEMOD_CW)	{
						if(ts.tx_audio_source == TX_AUDIO_MIC)
							ts.tx_audio_source = TX_AUDIO_LINEIN;
						else
							ts.tx_audio_source = TX_AUDIO_MIC;
						//
						if(ts.enc_thr_mode == ENC_THREE_MODE_RIT)	// if encoder in RIT mode, grey out audio gain control
							UIDriverChangeAudioGain(0);
						else									// not RIT mode - don't grey out
							UIDriverChangeAudioGain(1);
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
							if(ts.misc_flags1 & 128)	{		// is the waterfall mode active?
								ts.misc_flags1 &=  0x7f;	// yes, turn it off
								UiInitSpectrumScopeWaterfall();			// init spectrum scope
							}
							else	{	// waterfall mode was turned off
								ts.misc_flags1 |=  128;	// turn it on
								UiInitSpectrumScopeWaterfall();			// init spectrum scope
							}
							UiDriverDisplayFilterBW();	// Update on-screen indicator of filter bandwidth
						}
					}
					break;
				case BUTTON_BNDP_PRESSED:			// BAND+ button pressed-and-held?
					if(!UiDriverButtonCheck(BUTTON_BNDM_PRESSED))	{	// and BAND-DOWN pressed at the same time?
						if(!ts.menu_mode)	{		// do not do this if in menu mode!
							if(ts.misc_flags1 & 128)	{		// is the waterfall mode active?
								ts.misc_flags1 &=  0x7f;	// yes, turn it off
								UiInitSpectrumScopeWaterfall();			// init spectrum scope
							}
							else	{	// waterfall mode was turned off
								ts.misc_flags1 |=  128;	// turn it on
								UiInitSpectrumScopeWaterfall();			// init spectrum scope
							}
							UiDriverDisplayFilterBW();	// Update on-screen indicator of filter bandwidth
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
						if(ts.vfo_mem_mode & 0x80)	{	// in SPLIT mode?
							UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
							UiDriverUpdateFrequency(1,2);	// update RX frequency
						}
						else	{	// not in SPLIT mode - standard update
							UiDriverUpdateFrequency(1,0);
						}
						ts.refresh_freq_disp = 0;	// restore selective update mode for frequency display
					}
					else	{
						if(!(ts.freq_step_config & 0xf0))	// button swap NOT enabled
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
						if(ts.vfo_mem_mode & 0x80)	{	// in SPLIT mode?
							UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
							UiDriverUpdateFrequency(1,2);	// update RX frequency
						}
						else	{	// not in SPLIT mode - standard update
							UiDriverUpdateFrequency(1,0);
						}
						ts.refresh_freq_disp = 0;	// restore selective update mode for frequency display
					}
					else	{
						if(!(ts.freq_step_config & 0xf0))	// button swap NOT enabled
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
UiInitRxParms(void)
{
	UiCWSidebandMode();
	if(ts.menu_mode)	// are we in menu mode?
		UiDriverUpdateMenu(0);	// yes, update display when we change modes
	//
	UiCalcTxIqGainAdj();		// update gain and phase values when changing modes
	UiCalcTxPhaseAdj();
	UiCalcRxPhaseAdj();
	UiDriverChangeRfGain(1);	// update RFG/SQL on screen
	Audio_TXFilter_Init();
	UiDriverChangeDSPMode();	// Change DSP display setting as well
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
		UIDriverChangeAudioGain(0);			// display Line/Mic gain and
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
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverProcessActiveFilterScan
//* Object              : verify that currently-selected filter is active and if not, select
//* Object              : next active filter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverProcessActiveFilterScan(void)
{
uchar filter_scan = 0;

bool	voice_mode, select_10k, select_3k6;

	if(ts.dmod_mode == DEMOD_FM)		// bail out if FM as filters are selected in configuration menu
		return;

	//
	// Scan through filters to determine if the selected filter is disabled - and skip if it is.
	// NOTE:  The 2.3 kHz filter CANNOT be disabled
	//
	// This also handles filters that are disabled according to mode (e.g. CW filters in SSB mode, SSB filters in CW mode)
	//

	if((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_AM))	// check to see if we are set to a "voice" mode
		voice_mode = 1;
	else					// not in voice mode
		voice_mode = 0;

	if((ts.filter_wide_select >= WIDE_FILTER_10K) || (ts.dmod_mode == DEMOD_AM))	// is 10k filter to be enabled and in AM or FM??
		select_10k = 1;				// yes - and it should always be available in AM/FM mode
	else
		select_10k = 0;				// it is not to be enabled

	if((ts.filter_3k6_select) || (ts.dmod_mode == DEMOD_AM))	// is 3.6k filter to be enabled or in AM mode?
		select_3k6 = 1;				// yes - and it should always be available in AM/FM mode
	else
		select_3k6 = 0;				// it is not to be enabled


	first_filter:
		//
		if((ts.filter_id == AUDIO_300HZ) && (!ts.filter_300Hz_select))
			ts.filter_id++;
		//
		if((ts.filter_id == AUDIO_300HZ) && ((ts.filter_ssb_narrow_disable) && (voice_mode)))
			ts.filter_id++;
		//
		if((ts.filter_id == AUDIO_500HZ) && (!ts.filter_500Hz_select))
			ts.filter_id++;
		//
		if((ts.filter_id == AUDIO_500HZ) && ((ts.filter_ssb_narrow_disable) && (voice_mode)))
			ts.filter_id++;
		//
		if((ts.filter_id == AUDIO_1P8KHZ) && (!ts.filter_1k8_select))
			ts.filter_id++;
		//
		// At this point we would hit the 2.3 kHz filter, which is ALWAYS enabled!
		//
		if((ts.filter_id == AUDIO_3P6KHZ) && (!select_3k6))
			ts.filter_id++;
		//
		if(ts.filter_id == AUDIO_3P6KHZ && ((ts.filter_cw_wide_disable) && (ts.dmod_mode == DEMOD_CW)))
			ts.filter_id++;
		//
		if(((ts.filter_id == AUDIO_WIDE) && (!select_10k)) || ((ts.filter_id == AUDIO_WIDE) && (ts.dmod_mode == DEMOD_CW) && ts.filter_cw_wide_disable))	{
			ts.filter_id = AUDIO_MIN_FILTER;
			filter_scan++;
			if(filter_scan <= 1)	// Is this the first time here?
				goto first_filter;	// Yes - wrap around to find the other filters
			else	// second time around?
				ts.filter_id = AUDIO_2P3KHZ;	// Force selection of 2.3 kHz filter as all others are disabled
		}
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
	static bool is_last_menu_item = 0;
	//printf("button: %02x\n\r",id);

	// --------------------------------------------
	// F1 process
	if(id == BUTTON_F1_PRESSED)
	{
		if(!ts.mem_disp)	{			// allow only if NOT in memory display mode
			if((!ts.menu_mode) && (!ts.boot_halt_flag))	{	// go into menu mode if NOT already in menu mode and not to halt on startup
				ts.menu_mode = 1;
				is_last_menu_item = 0;	// clear last screen detect flag
				UiDriverClearSpectrumDisplay();
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y," EXIT  ",Yellow,Black,0);
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y," DEFLT",Yellow,Black,0);
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,"  PREV",Yellow,Black,0);
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y,"  NEXT",Yellow,Black,0);
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
					UIDriverChangeAudioGain(0);
				//
				UiDriverChangeRit(0);
				//
				// Enable volume control when in MENU mode
				//
				UiDriverChangeAfGain(1);
				//
				ts.menu_var = 0;
				//
				UiDriverUpdateMenu(0);	// Draw the menu the first time
				UiDriverUpdateMenu(1);	// Do update of the first menu item
			}
			else	{	// already in menu mode - we now exit
				ts.menu_mode = 0;
				UiInitSpectrumScopeWaterfall();			// init spectrum scope
				//
				// Restore encoder displays to previous modes
				UiDriverChangeEncoderOneMode(0);
				UiDriverChangeEncoderTwoMode(0);
				UiDriverChangeEncoderThreeMode(0);
				// Call twice since the function TOGGLES, not just enables (need to fix this at some point!)
				UiDriverChangeEncoderOneMode(0);
				UiDriverChangeEncoderTwoMode(0);
				UiDriverChangeEncoderThreeMode(0);
				UiDriverChangeFilter(1);	// update bandwidth display
				//
				if(!ts.menu_var_changed)
					UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y," MENU  ",White,Black,0);
				else		// Asterisk indicates that a change has been made and that an EEPROM save should be done
					UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y," MENU *",Orange,Black,0);
				//
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y," METER",White,Black,0);
				/*
				switch(ts.tx_meter_mode)	{	// redraw button according to meter mode
				case METER_SWR:
					UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y,"  SWR ",White,Black,0);
					break;
				case METER_ALC:
					UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y,"  ALC ",White,Black,0);
					break;
				case METER_AUDIO:
					UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y," AUDIO",White,Black,0);
					break;
				default:
					break;
				}
				*/
				//
				if(!ts.vfo_mem_flag)	{	// in normal VFO mode?
					if(ts.vfo_mem_mode & 0x80)	// SPLIT mode active?
						UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y," SPLIT",SPLIT_ACTIVE_COLOUR,Black,0);	// yes - indicate with color
					else
						UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y," SPLIT",SPLIT_INACTIVE_COLOUR,Black,0);		// not active - grey
				}
				else	{					// in memory mode
					UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,"  MEM ",White,Black,0);	// yes - indicate with color
				}
				//
				if(ts.vfo_mem_mode & 0x40)		// VFO B active?
					UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y," VFO B",White,Black,0);	// VFO B active
				else
					UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y," VFO A",White,Black,0);
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
			UiDriverUpdateMenu(3);
			ts.menu_var_changed = 1;
		}
		else	{	// Not in MENU mode - select the METER mode
			ts.tx_meter_mode++;
			if(ts.tx_meter_mode >= METER_MAX)
				ts.tx_meter_mode = 0;
			//
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y," METER",White,Black,0);
			//
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
			is_last_menu_item = 0;	// clear last screen detect flag
			if(ts.menu_item < 6)	{	// are we less than one screen away from the beginning?
				if(!ts.radio_config_menu_enable)	// yes - config/adjust menu not enabled?
					ts.menu_item = MAX_MENU_ITEM-1;	// yes, go to last item in normal menu
				else
					ts.menu_item = (MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS)-1;	// move to the last config/adjustment menu item
			}
			else	{
				if(ts.menu_item < MAX_MENU_ITEM)	// are we in the config menu?
					if(ts.menu_item >= 6)			// yes - are we at least on the second screen?
						ts.menu_item -= 6;				// yes, go to the previous screen
					else							// we are on the first screen
						ts.menu_item = 0;			// go to the first item
				//
				else if(ts.menu_item > MAX_MENU_ITEM)	{	// are we within the adjustment menu by at least one entry?
					if((ts.menu_item - 6) < MAX_MENU_ITEM)	{	// yes, will the next step be outside the adjustment menu?
						ts.menu_item = MAX_MENU_ITEM;			// yes - go to bottom of adjustment menu
					}
					else							// we will stay within the adjustment menu
						ts.menu_item -= 6;			// go back to previous page
				}
				else if(ts.menu_item == MAX_MENU_ITEM)	// are we at the bottom of the adjustment menu?
					ts.menu_item --;				// yes - go to the last entry of the adjustment menu
			}
//				ts.menu_item -= 6;	// not less than 6, so we subtract!
			//
			ts.menu_var = 0;			// clear variable that is used to change a menu item
			UiDriverUpdateMenu(1);		// Update that menu item
		}
		else	{	// NOT menu mode
			if(!ts.vfo_mem_flag)	{		// update screen if in VFO (not memory) mode
				if(ts.vfo_mem_mode & 0x80)	{	// are we in SPLIT mode?
					ts.vfo_mem_mode &= 0x7f;	// yes - turn off MSB to turn off SPLIT
					UiDriverInitMainFreqDisplay();		// update the main frequency display to reflect the mode
					ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
					UiDriverUpdateFrequency(1,1);	// force update of large digits
					ts.refresh_freq_disp = 0;	// disable refresh all digits flag
				}
				else if(!(ts.vfo_mem_mode & 0x80))	{	// are we NOT in SPLIT mode?
					ts.vfo_mem_mode |= 0x80;		// yes - turn on MSB to activate SPLIT
					UiDriverInitMainFreqDisplay();		//
					ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
					UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
					UiDriverUpdateFrequency(1,2);	// force display of first (RX) VFO frequency
					ts.refresh_freq_disp = 0;	// disable refresh all digits flag
				}
			}
			else	{		// in memory mode
				UiDriverClearSpectrumDisplay();		// always clear displayclear display
				if(!ts.mem_disp)	{	// are we NOT in memory display mode at this moment?
					ts.mem_disp = 1;	// we are not - turn it on
				}
				else	{				// we are in memory display mode
					ts.mem_disp = 0;	// turn it off
					UiInitSpectrumScopeWaterfall();			// init spectrum scope
				}
			}
		}
	}

	// --------------------------------------------
	// F4 process
	if(id == BUTTON_F4_PRESSED)	{
		//
		if(ts.menu_mode)	{		// Next screen
			if(!ts.radio_config_menu_enable)	{	// Not in config/calibrate menu mode
				if(ts.menu_item == MAX_MENU_ITEM - 1)	{	// already at last item?
					is_last_menu_item = 0;				// make sure flag is clear
					ts.menu_item = 0;					// go to first item
				}
				else	{	// not at last item - go ahead
					ts.menu_item += 6;
					if(ts.menu_item >= MAX_MENU_ITEM - 1)	{	// were we at last item?
						if(!is_last_menu_item)	{	// have we NOT seen the last menu item flag before?
							ts.menu_item = MAX_MENU_ITEM - 1;	// set to last menu item
							is_last_menu_item = 1;		// set flag indicating that we are at last menu item
						}
						else	{	// last menu item flag was set
							ts.menu_item = 0;				// yes, wrap around
							is_last_menu_item = 0;				// clear flag
						}
					}
				}
			}
			else	{	// in calibrate/adjust menu mode
				if(ts.menu_item == (MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS-1))	{	// already at last item?
					is_last_menu_item = 0;				// make sure flag is clear
					ts.menu_item = 0;					// to to first item
				}
				else	{	// not at last item - go ahead
					if(ts.menu_item < MAX_MENU_ITEM - 1)	{	// are we starting from the adjustment menu?
						if((ts.menu_item + 6) >= MAX_MENU_ITEM)	{		// yes - is the next jump past the end of the menu?
							ts.menu_item = MAX_MENU_ITEM-1;		// yes - jump to the last item
						}
						else
							ts.menu_item += 6;	// not at last item - go to next screen
					}
					else	// not on adjustment menu
						ts.menu_item += 6;	// go to next configuration screen
					//
					if(ts.menu_item >= (MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS-1))	{	// were we at last item?
						if(!is_last_menu_item)	{	// have we NOT seen the last menu item flag before?
							ts.menu_item = MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS-1;	// set to last menu item
							is_last_menu_item = 1;		// set flag indicating that we are at last menu item
						}
						else	{	// last menu item flag was set
							ts.menu_item = 0;				// yes, wrap around
							is_last_menu_item = 0;				// clear flag
						}
					}
				}
			}
			//
			ts.menu_var = 0;			// clear variable that is used to change a menu item
			UiDriverUpdateMenu(1);		// Update that menu item
		}
		else	{	// NOT menu mode
			if(ts.vfo_mem_mode & 0x40)		{	// LSB on VFO mode byte set?
				ts.vfo_mem_mode &= 0xbf;	// yes, it's now VFO-B mode, so clear it, setting it to VFO A mode
				band_dial_value_b[ts.band] = df.tune_old;	//band_dial_value[ts.band];		// save "VFO B" settings
				band_decod_mode_b[ts.band] = ts.dmod_mode;	//band_decod_mode[ts.band];
				band_filter_mode_b[ts.band] = ts.filter_id;	//band_filter_mode[ts.band];
				//
				band_dial_value[ts.band] = band_dial_value_a[ts.band];		// load "VFO A" settings into working registers
				band_decod_mode[ts.band] = band_decod_mode_a[ts.band];
				band_filter_mode[ts.band] = band_filter_mode_a[ts.band];
				//
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y," VFO A",White,Black,0);
				//
			}
			else	{						// LSB on VFO mode byte NOT set?
				ts.vfo_mem_mode |= 0x40;			// yes, it's now in VFO-A mode, so set it, setting it to VFO B mode
				band_dial_value_a[ts.band] = df.tune_old;	//band_dial_value[ts.band];		// save "VFO A" settings
				band_decod_mode_a[ts.band] = ts.dmod_mode;	//band_decod_mode[ts.band];
				band_filter_mode_a[ts.band] = ts.filter_id;	//band_filter_mode[ts.band];
					//
				band_dial_value[ts.band] = band_dial_value_b[ts.band];		// load "VFO B" settings
				band_decod_mode[ts.band] = band_decod_mode_b[ts.band];
				band_filter_mode[ts.band] = band_filter_mode_b[ts.band];
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y," VFO B",White,Black,0);
				//
			}
			df.tune_new = band_dial_value[ts.band];
			//
			// do frequency/display update
			if(ts.vfo_mem_mode & 0x80)	{	// in SPLIT mode?
				if(!(ts.vfo_mem_mode & 0x40))	{
					UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X-(SMALL_FONT_WIDTH*5),POS_TUNE_FREQ_Y,"(A) RX->",RX_Grey,Black,0);	// Place identifying marker for RX frequency
					UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X-(SMALL_FONT_WIDTH*5),POS_TUNE_SPLIT_FREQ_Y_TX,"(B) TX->",TX_Grey,Black,0);	// Place identifying marker for TX frequency
				}
				else	{
					UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X-(SMALL_FONT_WIDTH*5),POS_TUNE_FREQ_Y,"(B) RX->",RX_Grey,Black,0);	// Place identifying marker for RX frequency
					UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X-(SMALL_FONT_WIDTH*5),POS_TUNE_SPLIT_FREQ_Y_TX,"(A) TX->",TX_Grey,Black,0);	// Place identifying marker for TX frequency
				}
				//
				ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
				UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency - do this first so small display shows RX freq
				UiDriverUpdateFrequency(1,2);	// update RX frequency
				ts.refresh_freq_disp = 0;
			}
			else	// not in SPLIT mode - standard update
				UiDriverUpdateFrequency(1,0);

			// Change decode mode if need to
			if(ts.dmod_mode != band_decod_mode[ts.band])
			{
				// Update mode
				ts.dmod_mode = band_decod_mode[ts.band];

				// Update Decode Mode (USB/LSB/AM/FM/CW)
				UiDriverShowMode();
			}

			// Change filter mode if need to
			if(ts.filter_id != band_filter_mode[ts.band])
			{
				ts.filter_id = band_filter_mode[ts.band];
				UiDriverChangeFilter(0);	// update display and change filter
				UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator
				audio_driver_set_rx_audio_filter();
				audio_driver_set_rx_audio_filter();	// we have to invoke the filter change several times for some unknown reason - 'dunno why!
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

			// Change button color
			if(ts.tune)
			{
				if((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB))
					softdds_setfreq(SSB_TUNE_FREQ, ts.samp_rate,0);		// generate tone for setting TX IQ phase
				// DDS on
				else
					softdds_setfreq(CW_SIDETONE_FREQ_DEFAULT,ts.samp_rate,0);

				// To TX
				ts.txrx_mode = TRX_MODE_TX;
				ui_driver_toggle_tx();				// tune

				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,"  TUNE",Red,Black,0);
				//
			}
			else
			{
				if((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB))	// DDS off if voice mode
					softdds_setfreq(0.0,ts.samp_rate,0);
				else if(ts.dmod_mode == DEMOD_CW)	{	// DDS reset to proper sidetone freq. if CW mode
					cw_gen_init();
					softdds_setfreq((float)ts.sidetone_freq,ts.samp_rate,0);
				}
				//
				// Back to RX
				ts.txrx_mode = TRX_MODE_RX;
				ui_driver_toggle_tx();				// tune

				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,"  TUNE",White,Black,0);
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

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	switch(ts.dmod_mode)
	{
		case DEMOD_USB:
			UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 8),POS_DEMOD_MODE_Y,"USB",Cream,Blue,0);
			break;
		case DEMOD_LSB:
			UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 8),POS_DEMOD_MODE_Y,"LSB",Cream,Blue,0);
			break;
		case DEMOD_AM:
			UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 12),POS_DEMOD_MODE_Y,"AM",Cream,Blue,0);
			break;
		case DEMOD_FM:
			if(ts.txrx_mode == TRX_MODE_RX)	{
				if(ads.fm_squelched)	// is audio squelched?
					UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 4),POS_DEMOD_MODE_Y," FM ",Cream,Blue,0);	// yes - print normally
				else	{
					if((ads.fm_subaudible_tone_detected) && (ts.fm_subaudible_tone_det_select))	// is tone decoding enabled AND a tone being detected?
						UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 4),POS_DEMOD_MODE_Y," FM ",Black,Red2,0);	// Not squelched, passing audio - change color!
					else	// tone decoder disabled - squelch only
						UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 4),POS_DEMOD_MODE_Y," FM ",Black,Cream,0);	// Not squelched, passing audio - change color, but different from tone
				}
			}
			else if(ts.txrx_mode == TRX_MODE_TX)	{	// in transmit mode?
				if(ads.fm_tone_burst_active)	{		// yes - is tone burst active?
					UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 4),POS_DEMOD_MODE_Y," FM ",Black,Yellow,0);	// Yes, make "FM" yellow
				}
				else	{
					UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 4),POS_DEMOD_MODE_Y," FM ",Cream,Blue,0);	// no - print normally
				}
			}
			break;
		case DEMOD_CW:
			if(ts.cw_lsb)	// determine if CW is USB or LSB mode
				UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 4),POS_DEMOD_MODE_Y,"CW-L",Cream,Blue,0);
			else
				UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 4),POS_DEMOD_MODE_Y,"CW-U",Cream,Blue,0);
			break;
		default:
			break;
	}
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

	if(ts.tune_step)		// is this a "Temporary" step size from press-and-hold?
		color = Cyan;	// yes - display step size in Cyan
	else				// normal mode
		color = White;	// step size in white

	if(step_line)	{	// Remove underline indicating step size if one had been drawn
		UiLcdHy28_DrawStraightLine((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * 3)),(POS_TUNE_FREQ_Y + 24),(LARGE_FONT_WIDTH*7),LCD_DIR_HORIZONTAL,Black);
		UiLcdHy28_DrawStraightLine((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * 3)),(POS_TUNE_FREQ_Y + 25),(LARGE_FONT_WIDTH*7),LCD_DIR_HORIZONTAL,Black);
	}

	// Blank old step size
	UiLcdHy28_DrawFullRect(POS_TUNE_STEP_MASK_X,POS_TUNE_STEP_MASK_Y,POS_TUNE_STEP_MASK_H,POS_TUNE_STEP_MASK_W,Black);

	// Create Step Mode
	switch(df.tuning_step)
	{
		case T_STEP_1HZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*3),POS_TUNE_STEP_Y,"1Hz",color,Black,0);
			line_loc = 9;
			break;
		case T_STEP_10HZ:
			line_loc = 8;
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*2),POS_TUNE_STEP_Y,"10Hz",color,Black,0);
			break;
		case T_STEP_100HZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*1),POS_TUNE_STEP_Y,"100Hz",color,Black,0);
			line_loc = 7;
			break;
		case T_STEP_1KHZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*2),POS_TUNE_STEP_Y,"1kHz",color,Black,0);
			line_loc = 5;
			break;
		case T_STEP_10KHZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*1),POS_TUNE_STEP_Y,"10kHz",color,Black,0);
			line_loc = 4;
			break;
		case T_STEP_100KHZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*0),POS_TUNE_STEP_Y,"100kHz",color,Black,0);
			line_loc = 3;
			break;
		case T_STEP_1MHZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*0),POS_TUNE_STEP_Y,"1MHz",color,Black,0);
			line_loc = 3;
			break;
		case T_STEP_10MHZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*0),POS_TUNE_STEP_Y,"10MHz",color,Black,0);
			line_loc = 3;
			break;
		default:
			break;
	}
	//
	if(ts.freq_step_config & 0x0f)	{		// is frequency step marker line enabled?
		UiLcdHy28_DrawStraightLine((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * line_loc)),(POS_TUNE_FREQ_Y + 24),(LARGE_FONT_WIDTH),LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLine((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * line_loc)),(POS_TUNE_FREQ_Y + 25),(LARGE_FONT_WIDTH),LCD_DIR_HORIZONTAL,White);
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
	// Clear control
	UiLcdHy28_DrawFullRect(POS_BAND_MODE_MASK_X,POS_BAND_MODE_MASK_Y,POS_BAND_MODE_MASK_H,POS_BAND_MODE_MASK_W,Black);

	// Create Band value
	switch(band)
	{
		case BAND_MODE_80:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"80m",Orange,Black,0);
			break;

		case BAND_MODE_60:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"60m",Orange,Black,0);
			break;

		case BAND_MODE_40:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"40m",Orange,Black,0);
			break;

		case BAND_MODE_30:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"30m",Orange,Black,0);
			break;

		case BAND_MODE_20:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"20m",Orange,Black,0);
			break;

		case BAND_MODE_17:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"17m",Orange,Black,0);
			break;

		case BAND_MODE_15:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"15m",Orange,Black,0);
			break;

		case BAND_MODE_12:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"12m",Orange,Black,0);
			break;

		case BAND_MODE_10:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"10m",Orange,Black,0);
			break;

		case BAND_MODE_GEN:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"Gen",Orange,Black,0);
			break;

		default:
			break;
	}
}

// -------------------------------------------
// 	 BAND		BAND0		BAND1		BAND2
//
//	 80m		1			1			x
//	 40m		1			0			x
//	 20/30m		0			0			x
//	 15-10m		0			1			x
//
// -------------------------------------------
//
void UiDriverChangeBandFilter(uchar band,uchar bpf_only)
{
	if(bpf_only)
		goto do_bpf;

	// ---------------------------------------------
	// Set LPFs
	// Set relays in groups, internal first, then external group
	// state change via two pulses on BAND2 line, then idle
	switch(band)
	{
		case BAND_MODE_80:
		{
			// Internal group - Set(High/Low)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			// External group -Set(High/High)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			break;
		}

		case BAND_MODE_60:
		case BAND_MODE_40:
		{
			// Internal group - Set(High/Low)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			// External group - Reset(Low/High)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			break;
		}

		case BAND_MODE_30:
		case BAND_MODE_20:
		{
			// Internal group - Reset(Low/Low)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			// External group - Reset(Low/High)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			break;
		}

		case BAND_MODE_17:
		case BAND_MODE_15:
		case BAND_MODE_12:
		case BAND_MODE_10:
		{
			// Internal group - Reset(Low/Low)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			// External group - Set(High/High)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			break;
		}

		default:
			break;
	}

do_bpf:

	// ---------------------------------------------
	// Set BPFs
	// Constant line states for the BPF filter,
	// always last - after LPF change
	switch(band)
	{
		case BAND_MODE_80:
		{
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;
			break;
		}

		case BAND_MODE_60:
		case BAND_MODE_40:
		{
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;
			break;
		}

		case BAND_MODE_30:
		case BAND_MODE_20:
		{
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;
			break;
		}

		case BAND_MODE_17:
		case BAND_MODE_15:
		case BAND_MODE_12:
		case BAND_MODE_10:
		{
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
	if(!(ts.vfo_mem_mode & 0x80))	{	// are we in SPLIT mode?
		if(!ts.vfo_mem_flag)	{	// update bottom of screen if in VFO (not memory) mode
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y," SPLIT",SPLIT_INACTIVE_COLOUR,Black,0);	// make SPLIT indicator grey to indicate off
		}
//		UiLcdHy28_PrintText(POS_TUNE_FREQ_X,POS_TUNE_FREQ_Y + 4,"          ",White,Black,1);	// clear area near frequency display
		UiLcdHy28_PrintText(POS_TUNE_FREQ_X,POS_TUNE_FREQ_Y,"  .   .   ",White,Black,1);	// clear frequency display and replace dots
	}
	else	{	// are we NOT in SPLIT mode?
		if(!ts.vfo_mem_flag)	{	// update bottom of screen if in VFO (not memory) mode
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y," SPLIT",White,Black,0);	// make SPLIT indicator YELLOW to indicate on
		}
		UiLcdHy28_PrintText(POS_TUNE_FREQ_X,POS_TUNE_FREQ_Y,"          ",White,Black,1);	// clear large frequency digits
		UiLcdHy28_PrintText(POS_TUNE_SPLIT_FREQ_X,POS_TUNE_FREQ_Y,"  .   .   ",White,Black,0);	// clear frequency display and replace dots for RX freq
		UiLcdHy28_PrintText(POS_TUNE_SPLIT_FREQ_X,POS_TUNE_SPLIT_FREQ_Y_TX,"  .   .   ",White,Black,0);	// clear frequency display and replace dots for TX freq
		if(!(ts.vfo_mem_mode & 0x40))	{
			UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X-(SMALL_FONT_WIDTH*5),POS_TUNE_FREQ_Y,"(A) RX->",RX_Grey,Black,0);	// Place identifying marker for RX frequency
			UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X-(SMALL_FONT_WIDTH*5),POS_TUNE_SPLIT_FREQ_Y_TX,"(B) TX->",TX_Grey,Black,0);	// Place identifying marker for TX frequency
		}
		else	{
			UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X-(SMALL_FONT_WIDTH*5),POS_TUNE_FREQ_Y,"(B) RX->",RX_Grey,Black,0);	// Place identifying marker for RX frequency
			UiLcdHy28_PrintText(POS_TUNE_SPLIT_MARKER_X-(SMALL_FONT_WIDTH*5),POS_TUNE_SPLIT_FREQ_Y_TX,"(A) TX->",TX_Grey,Black,0);	// Place identifying marker for TX frequency
		}
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
	UiDriverChangeBandFilter(ts.band,0);

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	UiDriverShowMode();

	// Create Step Mode
	UiDriverShowStep(df.tuning_step);

	// Frequency
	UiDriverInitMainFreqDisplay();

	// Second Frequency
	UiLcdHy28_PrintText(POS_TUNE_SFREQ_X,POS_TUNE_SFREQ_Y,"00.000.00 ",Grey,Black,0);	// clear area of display, placing decimal points

	// Function buttons
	UiDriverCreateFunctionButtons(true);

	// S-meter
	UiDriverCreateSMeter();

	// Spectrum scope
	UiInitSpectrumScopeWaterfall();
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
		UIDriverChangeAudioGain(0);
	//
	cw_gen_init();

	// DSP mode change
	UiDriverChangeDSPMode();

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
	UiLcdHy28_PrintText			(POS_PWR_IND_X,POS_PWR_IND_Y,   "12.00V",  COL_PWR_IND,Black,0);

	// Create temperature
	if((lo.sensor_present == 0) && (df.temp_enabled & 0x0f))
		UiDriverCreateTemperatureDisplay(1,1);
	else
		UiDriverCreateTemperatureDisplay(0,1);

	// Set correct frequency
	//UiDriverUpdateFrequency(1,0);
	ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
	//
	if(!(ts.vfo_mem_mode & 0x80))	{	// are we in SPLIT mode?
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
	char cap1[20],cap2[20],cap3[20],cap4[20],cap5[20];
	ulong	clr;

	// Create bottom bar
	if(full_repaint)
	{
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X +                             0),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*1 + 2),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*2 + 4),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*3 + 6),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*4 + 8),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,(POS_BOTTOM_BAR_BUTTON_W + 1),Grey);
	}

	strcpy(cap1,"  MENU");
	strcpy(cap2," METER");
	strcpy(cap3," SPLIT");
	if(ts.vfo_mem_mode & 0x40)		// VFO B mode?
		strcpy(cap4," VFO B");	// yes - indicate
	else
		strcpy(cap4," VFO A");	// VFO A mode otherwise
	strcpy(cap5,"  TUNE");
	//

	// Draw buttons text
	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y,cap1,White,Black,0);
	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y,cap2,White,Black,0);

	if(!ts.vfo_mem_flag)	{	// is it in VFO (not memory) mode?
		if(ts.vfo_mem_mode & 0x80)	// SPLIT mode active?
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,cap3,SPLIT_ACTIVE_COLOUR,Black,0);		// yes - make yellow
		else
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,cap3,SPLIT_INACTIVE_COLOUR,Black,0);		// SPLIT mode not active - grey
	}
	else	{	// it is in memory mode (not VFO) mode
		strcpy(cap3,"  MEM ");
		UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,cap3,White,Black,0);		// yes - make yellow
	}

	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y,cap4,White,Black,0);
	//
	if(ts.tx_disable)	// is transmit disabled?
		clr = Grey1;	// Yes - make TUNE button gray
	else
		clr = White;	// Not disabled, it is white

	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,cap5,clr,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDrawWhiteSMeter
//* Object              : draw the white part of the S meter to clear the red S-meter (below)
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverDrawWhiteSMeter(void)
{
	uchar 	i,v_s;

// Draw top line
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X +  18),(POS_SM_IND_Y + 20),92,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X +  18),(POS_SM_IND_Y + 21),92,LCD_DIR_HORIZONTAL,White);

	// Draw s markers on top white line
	for(i = 0; i < 10; i++)
	{
		// Draw s text, only odd numbers
		if(i%2)
		{
			v_s = 5;
		}
		else
			v_s = 3;

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,White);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,White);
	}
}
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDrawRedSMeter
//* Object              : draw the part of the S meter in red to indicate A/D overload
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverDrawRedSMeter(void)
{
	uchar 	i,v_s;

// Draw top line
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X +  18),(POS_SM_IND_Y + 20),92,LCD_DIR_HORIZONTAL,Red);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X +  18),(POS_SM_IND_Y + 21),92,LCD_DIR_HORIZONTAL,Red);

	// Draw s markers on top white line
	for(i = 0; i < 10; i++)
	{

		// Draw s text, only odd numbers
		if(i%2)
		{
			v_s = 5;
		}
		else
			v_s = 3;

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,Red);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,Red);
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
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X +  18),(POS_SM_IND_Y + 20),92,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X +  18),(POS_SM_IND_Y + 21),92,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 113),(POS_SM_IND_Y + 20),75,LCD_DIR_HORIZONTAL,Green);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 113),(POS_SM_IND_Y + 21),75,LCD_DIR_HORIZONTAL,Green);

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
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,White);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,White);
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
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 113) + i*20),(POS_SM_IND_Y + 15),5,LCD_DIR_VERTICAL,Green);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 114) + i*20),(POS_SM_IND_Y + 15),5,LCD_DIR_VERTICAL,Green);
		}
	}

	// Draw middle line
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 32),170,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 33),170,LCD_DIR_HORIZONTAL,White);

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
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*15),((POS_SM_IND_Y + 32) - 2),2,LCD_DIR_VERTICAL,White);
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*15),((POS_SM_IND_Y + 32) - 2),2,LCD_DIR_VERTICAL,White);
			}
			else
			{
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*15),((POS_SM_IND_Y + 32) - 7),7,LCD_DIR_VERTICAL,White);
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*15),((POS_SM_IND_Y + 32) - 7),7,LCD_DIR_VERTICAL,White);
			}
		}
	}


	if(ts.tx_meter_mode == METER_SWR)	{
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59),"SWR",Red2,Black,4);

		// Draw bottom line for SWR indicator
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55), 62,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 56), 62,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 83),(POS_SM_IND_Y + 55),105,LCD_DIR_HORIZONTAL,Red);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 83),(POS_SM_IND_Y + 56),105,LCD_DIR_HORIZONTAL,Red);
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

					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
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

		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55), 62,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 56), 62,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 83),(POS_SM_IND_Y + 55),105,LCD_DIR_HORIZONTAL,Red);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 83),(POS_SM_IND_Y + 56),105,LCD_DIR_HORIZONTAL,Red);

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

					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
				}
			}
		}
	}
	else if(ts.tx_meter_mode == METER_AUDIO)	{
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59),"AUD",Cyan,Black,4);

		// Draw bottom line for SWR indicator
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55), 108,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 56), 108,LCD_DIR_HORIZONTAL,White);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 129),(POS_SM_IND_Y + 55),59,LCD_DIR_HORIZONTAL,Red);
		UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 129),(POS_SM_IND_Y + 56),59,LCD_DIR_HORIZONTAL,Red);
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

					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
				}
				else
				{
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
					UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
				}
			}
		}
	}
	// Draw meters
	UiDriverUpdateTopMeterA(0,0);
	UiDriverUpdateBtmMeter(0, 0);

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateTopMeterA
//* Object              : redraw indicator, same like upper implementation
//* Input Parameters    : but no hold
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateTopMeterA(uchar val,uchar old)
{
	uchar 	i,v_s;
	int		col = Blue2;


	// Do not waste time redrawing if outside of the range or if the meter has not changed
	if((val > 34) || (val == old))
		return;

	// Indicator height
	v_s = 3;

	// Draw first indicator
	for(i = 1; i < 34; i++)
	{
		if(val < i)
			col = Grid;

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 20) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
	}
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
	uchar 	i,v_s;
	int		col = Cyan;

	// Do not waste time redrawing if outside of the range
	if(val > 34)
		return;

	// Indicator height
	v_s = 3;


	// Draw indicator
	for(i = 1; i < 34; i++)
	{
		if(val < i)
			col = Grid;
		else if((i >= warn) && warn)	// is level above "warning" color? (is "warn" is zero, disable warning)
			col = Red2;					// yes - display values above that color in red

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*5),((POS_SM_IND_Y + 51) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*5),((POS_SM_IND_Y + 51) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 20) + i*5),((POS_SM_IND_Y + 51) - v_s),v_s,LCD_DIR_VERTICAL,col);
	}
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
	if(ts.scope_scale_colour == SPEC_GREY)
		clr = Grey;
	else if(ts.scope_scale_colour == SPEC_BLUE)
		clr = Blue;
	else if(ts.scope_scale_colour == SPEC_RED)
		clr = Red;
	else if(ts.scope_scale_colour == SPEC_MAGENTA)
		clr = Magenta;
	else if(ts.scope_scale_colour == SPEC_GREEN)
		clr = Green;
	else if(ts.scope_scale_colour == SPEC_CYAN)
		clr = Cyan;
	else if(ts.scope_scale_colour == SPEC_YELLOW)
		clr = Yellow;
	else if(ts.scope_scale_colour == SPEC_BLACK)
		clr = Black;
	else if(ts.scope_scale_colour == SPEC_ORANGE)
		clr = Orange;
	else
		clr = White;

	freq_calc = df.tune_new/4;		// get current frequency in Hz

	if(!sd.magnify)	{		// if magnify is off, way *may* have the graticule shifted.  (If it is on, it is NEVER shifted from center.)
		if(ts.iq_freq_mode == 1)			// Is "RX LO HIGH" translate mode active?
			freq_calc += FREQ_SHIFT_MAG;	// Yes, shift receive frequency to left of center
		else if(ts.iq_freq_mode == 2)		// it is "RX LO LOW" in translate mode
			freq_calc -= FREQ_SHIFT_MAG;	// shift receive frequency to right of center
	}
	freq_calc = (freq_calc + 500)/1000;	// round graticule frequency to the nearest kHz

	//

	if((ts.iq_freq_mode == 0) || (sd.magnify))	{	// Translate mode is OFF or magnify is on
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
	else if((ts.iq_freq_mode == 1) && (!sd.magnify))	{	// Translate mode is ON (LO is HIGH, center is left of middle of display) AND magnify is off
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
	else if((ts.iq_freq_mode == 2) && (!sd.magnify))	{	// Translate mode is ON (LO is LOW, center is to the right of middle of display) AND magnify is off
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
//* Function Name       : UiDriverCreateSpectrumScope
//* Object              : draw the spectrum scope control
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverCreateSpectrumScope(void)
{
	ulong i, clr;
	char s[32];
	ulong slen;

	//
	// get grid colour of all but center line
	//
	if(ts.scope_grid_colour == SPEC_GREY)
		ts.scope_grid_colour_active = Grid;
	else if(ts.scope_grid_colour == SPEC_BLUE)
		ts.scope_grid_colour_active = Blue;
	else if(ts.scope_grid_colour == SPEC_RED)
		ts.scope_grid_colour_active = Red;
	else if(ts.scope_grid_colour == SPEC_MAGENTA)
		ts.scope_grid_colour_active = Magenta;
	else if(ts.scope_grid_colour == SPEC_GREEN)
		ts.scope_grid_colour_active = Green;
	else if(ts.scope_grid_colour == SPEC_CYAN)
		ts.scope_grid_colour_active = Cyan;
	else if(ts.scope_grid_colour == SPEC_YELLOW)
		ts.scope_grid_colour_active = Yellow;
	else if(ts.scope_grid_colour == SPEC_BLACK)
		ts.scope_grid_colour_active = Black;
	else if(ts.scope_grid_colour == SPEC_ORANGE)
		ts.scope_grid_colour_active = Orange;
	else if(ts.scope_grid_colour == SPEC_GREY2)
		ts.scope_grid_colour_active = Grey;
	else
		ts.scope_grid_colour_active = White;
	//
	//
	// Get color of center vertical line of spectrum scope
	//
	if(ts.scope_centre_grid_colour == SPEC_GREY)
		ts.scope_centre_grid_colour_active = Grid;
	else if(ts.scope_centre_grid_colour == SPEC_BLUE)
		ts.scope_centre_grid_colour_active = Blue;
	else if(ts.scope_centre_grid_colour == SPEC_RED)
		ts.scope_centre_grid_colour_active = Red;
	else if(ts.scope_centre_grid_colour == SPEC_MAGENTA)
		ts.scope_centre_grid_colour_active = Magenta;
	else if(ts.scope_centre_grid_colour == SPEC_GREEN)
		ts.scope_centre_grid_colour_active = Green;
	else if(ts.scope_centre_grid_colour == SPEC_CYAN)
		ts.scope_centre_grid_colour_active = Cyan;
	else if(ts.scope_centre_grid_colour == SPEC_YELLOW)
		ts.scope_centre_grid_colour_active = Yellow;
	else if(ts.scope_centre_grid_colour == SPEC_BLACK)
		ts.scope_centre_grid_colour_active = Black;
	else if(ts.scope_centre_grid_colour == SPEC_ORANGE)
		ts.scope_centre_grid_colour_active = Orange;
	else if(ts.scope_centre_grid_colour == SPEC_GREY2)
		ts.scope_centre_grid_colour_active = Grey;
	else
		ts.scope_centre_grid_colour_active = White;


	// Clear screen where frequency information will be under graticule
	//
	UiLcdHy28_PrintText(POS_SPECTRUM_IND_X - 2, POS_SPECTRUM_IND_Y + 60, "                                 ", Black, Black, 0);


	//
	strcpy(s, "SPECTRUM SCOPE ");
	slen = 0;	// init string length variable
	//
	switch(ts.spectrum_db_scale)	{	// convert variable to setting
		case DB_DIV_5:
			strcat(s, "(5dB/div)");
			slen = 30;	// fine-tune horizontal position (not sure why this is needed - look at this later)
			break;
		case DB_DIV_7:
			strcat(s, "(7.5dB/div)");
			slen = 24;	// fine-tune horizontal position (not sure why this is needed - look at this later)
			break;
		case DB_DIV_15:
			strcat(s, "(15dB/div)");
			slen = 28;	// fine-tune horizontal position (not sure why this is needed - look at this later)
			break;
		case DB_DIV_20:
			strcat(s, "(20dB/div)");
			slen = 28;	// fine-tune horizontal position (not sure why this is needed - look at this later)
			break;
		case S_1_DIV:
			strcat(s, "(1S-Unit/div)");
			break;
		case S_2_DIV:
			strcat(s, "(2S-Unit/div)");
			break;
		case S_3_DIV:
			strcat(s, "(3S-Unit/div)");
			break;
		case DB_DIV_10:
		default:
			strcat(s, "(10dB/div)");
			slen = 28;	// fine-tune horizontal position (not sure why this is needed - look at this later)
			break;
	}
	//
	slen += strlen(s);				// get width of entire banner string
	slen /= 2;						// scale it for half the width of the string

	// Draw top band
	for(i = 0; i < 16; i++)
		UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y - 20 + i),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

	if(!(ts.misc_flags1 & 128))	{	// Display Spectrum Scope banner if enabled

	// Top band text - middle caption
	UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X + slen),
									(POS_SPECTRUM_IND_Y - 18),
									s,
									Grey,
									RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);
	}
	else	{			// Waterfall Mode banner if that is enabled

		// Top band text - middle caption
		UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X + 68),
										(POS_SPECTRUM_IND_Y - 18),
										"WATERFALL DISPLAY",
										Grey,
										RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);
	}
	// Top band text - grid size
	//UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X +  2),
	//								(POS_SPECTRUM_IND_Y - 18),
	//								"Grid 6k",
	//								Grey,
	//								RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)),4);

	// Draw control left and right border
	for(i = 0; i < 2; i++)
	{
		UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X - 2 + i),
									(POS_SPECTRUM_IND_Y - 20),
									(POS_SPECTRUM_IND_H + 12),
									LCD_DIR_VERTICAL,
//									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
									ts.scope_grid_colour_active);

		UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X + POS_SPECTRUM_IND_W - 2 + i),
									(POS_SPECTRUM_IND_Y - 20),
									(POS_SPECTRUM_IND_H + 12),
									LCD_DIR_VERTICAL,
//									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
									ts.scope_grid_colour_active);
	}

	// Frequency bar separator
	UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 20),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

	// Draw Frequency bar text
	UiDrawSpectrumScopeFrequencyBarText();


	// Horizontal grid lines
	for(i = 1; i < 4; i++)
	{
		// Save y position for repaint
		sd.horz_grid_id[i - 1] = (POS_SPECTRUM_IND_Y - 5 + i*16);

		// Draw
		UiLcdHy28_DrawStraightLine(	POS_SPECTRUM_IND_X,
									sd.horz_grid_id[i - 1],
									POS_SPECTRUM_IND_W,
									LCD_DIR_HORIZONTAL,
//									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));
									ts.scope_grid_colour_active);
		//printf("vy: %d\n\r",sd.horz_grid_id[i - 1]);
	}

	// Vertical grid lines
	for(i = 1; i < 8; i++)		{

		// determine if we are drawing the "center" line on the spectrum  display
		if(!sd.magnify)	{
			if((ts.iq_freq_mode == FREQ_IQ_CONV_LO_LOW) && (i == 5))			// is it frequency translate RF LOW mode?  If so, shift right of center
				clr = ts.scope_centre_grid_colour_active;
			else if((ts.iq_freq_mode == FREQ_IQ_CONV_LO_HIGH) && (i == 3))		// shift left of center if RF HIGH translate mode
				clr = ts.scope_centre_grid_colour_active;
			else if ((ts.iq_freq_mode == FREQ_IQ_CONV_MODE_OFF) && (i == 4))	// center if translate mode not active
				clr = ts.scope_centre_grid_colour_active;
	        else
	     	   clr = ts.scope_grid_colour_active;								// normal color if other lines
		}
		else if(i == 4)
			clr = ts.scope_centre_grid_colour_active;
        else
     	   clr = ts.scope_grid_colour_active;								// normal color if other lines

		// Save x position for repaint
		sd.vert_grid_id[i - 1] = (POS_SPECTRUM_IND_X + 32*i + 1);

		// Draw
		UiLcdHy28_DrawStraightLine(	sd.vert_grid_id[i - 1],
									(POS_SPECTRUM_IND_Y -  4),
									(POS_SPECTRUM_IND_H - 15),
									LCD_DIR_VERTICAL,
//									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));
									clr);

		//printf("vx: %d\n\r",sd.vert_grid_id[i - 1]);
	}

	if(ts.misc_flags1 & 128)	{	// is it in waterfall mode?
		if(!ts.waterfall_speed)			// print "disabled" in the middle of the screen if the waterfall was disabled
			UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X + 72),
												(POS_SPECTRUM_IND_Y + 18),
												"   DISABLED   ",
												Grey,
												RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);
	}
	else	{
		if(!ts.scope_speed)		// print "disabled" in the middle of the screen if the spectrum scope was disabled
		UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X + 72),
											(POS_SPECTRUM_IND_Y + 18),
											"   DISABLED   ",
											Grey,
											RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);
	}




}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverClearSpectrumDisplay
//* Object              : Clears the spectrum display
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverClearSpectrumDisplay(void)
{
	ulong i;

	if(sd.use_spi)	{
		UiLcdHy28_DrawFullRect(POS_SPECTRUM_IND_X - 2, (POS_SPECTRUM_IND_Y - 22), 94, 264, Black);	// Clear screen under spectrum scope by drawing a single, black block (faster with SPI!)
	}
	else	{
		for(i = 0; i < 8; i++)	{
			UiLcdHy28_PrintText(POS_SPECTRUM_IND_X - 2, (POS_SPECTRUM_IND_Y - 22) + (i* 12), "                                 ", Black, Black, 0);
		}
	}
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
static void UiDriverInitFrequency(void)
{
	ulong i;

	// Clear band values array
	for(i = 0; i < MAX_BANDS; i++)
	{
		band_dial_value[i] = 0xFFFFFFFF;	// clear dial values
		band_decod_mode[i] = DEMOD_USB; 	// clear decode mode
		band_filter_mode[i] = AUDIO_DEFAULT_FILTER;	// clear filter mode
	}

	// Lower bands default to LSB mode
	for(i = 0; i < 4; i++)
		band_decod_mode[i] = DEMOD_LSB;

	// Init frequency publics(set diff values so update on LCD will be done)
	df.value_old	= 0;
	df.value_new	= 0;
	df.tune_old 	= tune_bands[ts.band];
	df.tune_new 	= tune_bands[ts.band];
	df.selected_idx = 3; 		// 1 Khz startup step
	df.tuning_step	= tune_steps[df.selected_idx];
	df.update_skip	= 0;		// skip value to compensate for fast dial rotation - test!!!
	df.temp_factor	= 0;
	df.temp_enabled = 0;		// startup state of TCXO

	//if(ts.band == BAND_MODE_4)
	//	df.transv_freq = TRANSVT_FREQ_A;
	//else
//	df.transv_freq	= 0;	// LO freq, zero on HF, 42 Mhz on 4m

	//df.tx_shift		= 0;		// offcet fo tx
	df.de_detent	= 0;

	// Set virtual segments initial value (diff than zero!)
	df.dial_100_mhz	= 0;
	df.dial_010_mhz	= 1;
	df.dial_001_mhz	= 4;
	df.dial_100_khz	= 0;
	df.dial_010_khz	= 0;
	df.dial_001_khz	= 0;
	df.dial_100_hz	= 0;
	df.dial_010_hz	= 0;
	df.dial_001_hz	= 0;
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
static void UiDriverCheckFilter(ulong freq)
{

	if(freq < BAND_FILTER_UPPER_80)	{	// are we low enough if frequency for the 80 meter filter?
		if(ts.filter_band != FILTER_BAND_80)	{
			UiDriverChangeBandFilter(BAND_MODE_80, 0);	// yes - set to 80 meters
			ts.filter_band = FILTER_BAND_80;
		}
	}
	else if(freq < BAND_FILTER_UPPER_40)	{
		if(ts.filter_band != FILTER_BAND_40)	{
			UiDriverChangeBandFilter(BAND_MODE_40, 0);	// yes - set to 40 meters
			ts.filter_band = FILTER_BAND_40;
		}
	}
	else if(freq < BAND_FILTER_UPPER_20)	{
		if(ts.filter_band != FILTER_BAND_20)	{
			UiDriverChangeBandFilter(BAND_MODE_20, 0);	// yes - set to 20 meters
			ts.filter_band = FILTER_BAND_20;
		}
	}
	else if(freq >= BAND_FILTER_UPPER_20)	{
		if(ts.filter_band != FILTER_BAND_15)	{
			UiDriverChangeBandFilter(BAND_MODE_10, 0);	// yes - set to 10 meters
			ts.filter_band = FILTER_BAND_15;
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
	if(ts.iq_freq_mode == 1)	// is frequency translate active and in "RX LO HIGH" mode?
		freq -= FREQ_SHIFT_MAG * 4;	// yes - subtract offset amount
	else if(ts.iq_freq_mode == 2)	// is frequency translate active and in "RX LO LOW" mode?
		freq += FREQ_SHIFT_MAG * 4;	// yes - add offset amount


	while((!flag) && (band_scan < MAX_BANDS))	{
		if((freq >= tune_bands[band_scan]) && (freq <= (tune_bands[band_scan] + size_bands[band_scan])))	// Is this frequency within this band?
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
		if(ts.vfo_mem_mode & 0x40)					// yes are we receiving with VFO B?
			loc_tune_new = band_dial_value_a[ts.band];		// yes - get VFO A frequency for TX
		else									// we must be receiving with VFO A
			loc_tune_new = band_dial_value_b[ts.band];		// get VFO B frequency for TX
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
		if(ts.iq_freq_mode == 1)
			ts.tune_freq += FREQ_SHIFT_MAG * 4;		// magnitude of shift is quadrupled at actual Si570 operating frequency
		else if(ts.iq_freq_mode == 2)
			ts.tune_freq -= FREQ_SHIFT_MAG * 4;
	}

	if(mode != 3)	{		// do not bother checking frequency limits if updating ONLY the TX frequency
		// Frequency range check, moved from si570 routine here
		if(!(ts.misc_flags1 & 32))	{	// is frequency tuning limit disabled?
			if((ts.tune_freq > SI570_MAX_FREQ) || (ts.tune_freq < SI570_MIN_FREQ))	// no - enforce limit
			{
				//printf("out of freq err: %d\n\r",tune_freq);
				df.tune_new = df.tune_old;						// reload old value
				return;
			}
		}
		else	{ 	// Tuning limits disabled - enforce "hard" limits on frequency tuning range to prevent crashing of system
			if((ts.tune_freq > SI570_HARD_MAX_FREQ) || (ts.tune_freq < SI570_HARD_MIN_FREQ))	// no - enforce limit
			{
				//printf("out of freq err: %d\n\r",tune_freq);
				df.tune_new = df.tune_old;						// reload old value
				return;
			}
		}
	}

	if(mode != 3)	{		// updating ONLY the TX frequency display?

		// Extra tuning actions
		if(ts.txrx_mode == TRX_MODE_RX)		{
			ts.tune_freq += (ts.rit_value*80);	// Add RIT on receive
		}
		//

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
				if(ts.dsp_active & 1)	// is DSP active?
					ts.rx_blanking_time = ts.sysclock + TUNING_LARGE_STEP_MUTING_TIME_DSP_ON;	// yes - schedule un-muting of audio when DSP is on
				else
					ts.rx_blanking_time = ts.sysclock + TUNING_LARGE_STEP_MUTING_TIME_DSP_OFF;	// no - schedule un-muting of audio when DSP is off
			}
		}

		// Set frequency
		if(ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 0))
		{
			if(ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 0))	// Try again if it didn't work the first time
				col = Red;	// Color in red if there was a problem setting frequency
		}
		//
		// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
		//
		if(sd.use_spi)
			ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
	}
	//
	// Update main frequency display
	//
		UiDriverUpdateLcdFreq(dial_freq,col, mode);
	//
	// Update second display to reflect RX frequency with RIT
	//
	if(mode != 3)	{		// do not update second display or check filters if we are updating TX frequency in SPLIT mode
		UiDriverUpdateSecondLcdFreq(second_freq/4);
		//
		UiDriverCheckFilter(ts.tune_freq/4);	// check the filter status with the new frequency update
		UiDriverCheckBand(ts.tune_freq, 1);		// check which band in which we are currently tuning and update the display
	}
	//
	// Allow clear of spectrum display in its state machine
	sd.dial_moved = 1;

	// Save current freq
	df.tune_old = loc_tune_new;


	// Save the tuning step used during the last dial update
	// - really important so we know what segments to clear
	// during tune step change
//	df.last_tune_step = df.tuning_step;
	//
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
		if(ts.iq_freq_mode == 1)
			ts.tune_freq += FREQ_SHIFT_MAG * 4;
		else if(ts.iq_freq_mode == 2)
			ts.tune_freq -= FREQ_SHIFT_MAG * 4;
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

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateLcdFreq
//* Object              : this function will split LCD freq display control
//* Object              : and update as it is 7 segments indicator
//* Input Parameters    : freq=freq (Hz), color=color, mode: 0 = auto, 1= force normal (large digits), 2= force upper, small, 3 = force lower, small
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateLcdFreq(ulong dial_freq,ushort color, ushort mode)
{
	uchar		d_100mhz,d_10mhz,d_1mhz;
	uchar		d_100khz,d_10khz,d_1khz;
	uchar		d_100hz,d_10hz,d_1hz;
	uchar		digit_size;
	ulong		pos_y_loc;
	ulong		pos_x_loc;
	ulong		font_width;

	char		digit[2];

	if(ts.xverter_mode)	{	// transverter mode active?
		dial_freq *= (ulong)ts.xverter_mode;	// yes - scale by LO multiplier
		dial_freq += ts.xverter_offset;	// add transverter frequency offset
		if(dial_freq > 1000000000)		// over 1000 MHz?
			dial_freq -= 1000000000;		// yes, offset to prevent overflow of display
		if(ts.xverter_mode)	// if in transverter mode, frequency is yellow
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
	//
	//
	if(ts.frequency_lock) 	// Frequency is locked - change color of display
		color = Grey;
	//
	// Terminate
	digit[1] = 0;

	if(!mode)	{
		if(ts.vfo_mem_mode & 0x80)	// in "split" mode?
			mode = 2;				// yes - update upper, small digits (receive frequency)
		else
			mode = 1;				// NOT in split mode:  large, normal-sized digits
	}


	if(mode == 2)	{		// small digits in upper location
		digit_size = 0;
		pos_y_loc = POS_TUNE_FREQ_Y;
		pos_x_loc = POS_TUNE_SPLIT_FREQ_X;
		font_width = SMALL_FONT_WIDTH;
	}
	else if(mode == 3)	{					// small digits in lower location
		digit_size = 0;
		pos_y_loc = POS_TUNE_SPLIT_FREQ_Y_TX;
		pos_x_loc = POS_TUNE_SPLIT_FREQ_X;
		font_width = SMALL_FONT_WIDTH;
	}
	else	{			// default:  normal sized (large) digits
		digit_size = 1;
		pos_y_loc = POS_TUNE_FREQ_Y;
		pos_x_loc = POS_TUNE_FREQ_X;
		font_width = LARGE_FONT_WIDTH;
	}

	// -----------------------
	// See if 100 Mhz needs update
	d_100mhz = (dial_freq/100000000);
	if((d_100mhz != df.dial_100_mhz) || ts.refresh_freq_disp)
	{
		//printf("100 mhz diff: %d\n\r",d_100mhz);

		// To string
		digit[0] = 0x30 + (d_100mhz & 0x0F);

		// Update segment
		if(d_100mhz)
			UiLcdHy28_PrintText((pos_x_loc - font_width),pos_y_loc,digit,color,Black,digit_size);
		else
			UiLcdHy28_PrintText((pos_x_loc - font_width),pos_y_loc,digit,Black,Black,digit_size);	// mask the zero

		// Save value
		df.dial_100_mhz = d_100mhz;
	}


	// -----------------------
	// See if 10 Mhz needs update
	d_10mhz = (dial_freq%100000000)/10000000;
	if((d_10mhz != df.dial_010_mhz) || ts.refresh_freq_disp)
	{
		//printf("10 mhz diff: %d\n\r",d_10mhz);

		// To string
		digit[0] = 0x30 + (d_10mhz & 0x0F);

		if(d_100mhz)	// update if 100 MHz digit is being displayed
			UiLcdHy28_PrintText((pos_x_loc + 0),pos_y_loc,digit,color,Black,digit_size);
		else	{
			if(d_10mhz)
				UiLcdHy28_PrintText((pos_x_loc + 0),pos_y_loc,digit,color,Black,digit_size);
			else
				UiLcdHy28_PrintText((pos_x_loc + 0),pos_y_loc,digit,Black,Black,digit_size);	// mask the zero
		}
		// Save value
		df.dial_010_mhz = d_10mhz;
	}

	// -----------------------
	// See if 1 Mhz needs update
	d_1mhz = (dial_freq%10000000)/1000000;
	if((d_1mhz != df.dial_001_mhz) || ts.refresh_freq_disp)
	{
		//printf("1 mhz diff: %d\n\r",d_1mhz);

		// To string
		digit[0] = 0x30 + (d_1mhz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((pos_x_loc + font_width),pos_y_loc,digit,color,Black,digit_size);

		// Save value
		df.dial_001_mhz = d_1mhz;
	}

	// -----------------------
	// See if 100 khz needs update
	d_100khz = (dial_freq%1000000)/100000;
	if((d_100khz != df.dial_100_khz) || ts.refresh_freq_disp)
	{
		//printf("100 khz diff: %d\n\r",d_100khz);

		// To string
		digit[0] = 0x30 + (d_100khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((pos_x_loc + font_width*3),pos_y_loc,digit,color,Black,digit_size);

		// Save value
		df.dial_100_khz = d_100khz;
	}

	// -----------------------
	// See if 10 khz needs update
	d_10khz = (dial_freq%100000)/10000;
	if((d_10khz != df.dial_010_khz) || ts.refresh_freq_disp)
	{
		//printf("10 khz diff: %d\n\r",d_10khz);

		// To string
		digit[0] = 0x30 + (d_10khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((pos_x_loc +font_width*4),pos_y_loc,digit,color,Black,digit_size);

		// Save value
		df.dial_010_khz = d_10khz;
	}

	// -----------------------
	// See if 1 khz needs update
	d_1khz = (dial_freq%10000)/1000;
	if((d_1khz != df.dial_001_khz) || ts.refresh_freq_disp)
	{
		//printf("1 khz diff: %d\n\r",d_1khz);

		// To string
		digit[0] = 0x30 + (d_1khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((pos_x_loc + font_width*5),pos_y_loc,digit,color,Black,digit_size);

		// Save value
		df.dial_001_khz = d_1khz;
	}

	// -----------------------
	// See if 100 hz needs update
	d_100hz = (dial_freq%1000)/100;
	if((d_100hz != df.dial_100_hz) || ts.refresh_freq_disp)
	{
		//printf("100 hz diff: %d\n\r",d_100hz);

		// To string
		digit[0] = 0x30 + (d_100hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((pos_x_loc + font_width*7),pos_y_loc,digit,color,Black,digit_size);

		// Save value
		df.dial_100_hz = d_100hz;
	}

	// -----------------------
	// See if 10 hz needs update
	d_10hz = (dial_freq%100)/10;
	if((d_10hz != df.dial_010_hz) || ts.refresh_freq_disp)
	{
		//printf("10 hz diff: %d\n\r",d_10hz);

		// To string
		digit[0] = 0x30 + (d_10hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((pos_x_loc + font_width*8),pos_y_loc,digit,color,Black,digit_size);

		// Save value
		df.dial_010_hz = d_10hz;
	}

	// -----------------------
	// See if 1 hz needs update
	d_1hz = (dial_freq%10)/1;
	if((d_1hz != df.dial_001_hz) || ts.refresh_freq_disp)
	{
		//printf("1 hz diff: %d\n\r",d_1hz);

		// To string
		digit[0] = 0x30 + (d_1hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((pos_x_loc + font_width*9),pos_y_loc,digit,color,Black,digit_size);

		// Save value
		df.dial_001_hz = d_1hz;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateSecondLcdFreq
//* Object              : second freq indicator
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateSecondLcdFreq(ulong dial_freq)
{
	uchar		d_100mhz,d_10mhz,d_1mhz;
	uchar		d_100khz,d_10khz,d_1khz;
	uchar		d_100hz,d_10hz,d_1hz;
	static bool	digit_100 = 1, digit_10 = 1;	// set active first time through to make sure that digit is erased, if it exists

	char		digit[2];

	if(ts.xverter_mode)	{	// transverter mode active?
		dial_freq += ts.xverter_offset;	// yes - add transverter frequency offset
		if(dial_freq > 1000000000)		// over 1000 MHz?
			dial_freq -= 1000000000;		// yes, offset to prevent overflow of display
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


	//
	// Terminate
	digit[1] = 0;

	//printf("--------------------\n\r");
	//printf("dial: %dHz\n\r",dial_freq);
	//printf("dial_001_mhz: %d\n\r",df.dial_001_mhz);
	//printf("dial_100_khz: %d\n\r",df.dial_100_khz);
	//printf("dial_010_khz: %d\n\r",df.dial_010_khz);
	//printf("dial_001_khz: %d\n\r",df.dial_001_khz);
	//printf("dial_100_hz:  %d\n\r",df.dial_100_hz);
	//printf("dial_010_hz:  %d\n\r",df.dial_010_hz);
	//printf("dial_001_hz:  %d\n\r",df.dial_001_hz);

	// Second Frequency
	//UiLcdHy28_PrintText((POS_TUNE_FREQ_X + 175),(POS_TUNE_FREQ_Y + 8),"14.000.000",Grey,Black,0);

	// -----------------------
	// See if 100 Mhz needs update
	d_100mhz = (dial_freq/100000000);
	if(d_100mhz != df.sdial_100_mhz)
	{
		//printf("100 mhz diff: %d\n\r",d_10mhz);

		// To string
		digit[0] = 0x30 + (d_100mhz & 0x0F);

		// Update segment
		if(d_100mhz)
			UiLcdHy28_PrintText((POS_TUNE_SFREQ_X - SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);
		else
			UiLcdHy28_PrintText((POS_TUNE_SFREQ_X - SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// mask the zero

		// Save value
		df.sdial_100_mhz = d_100mhz;
		digit_100 = 1;		// indicate that a 100 MHz digit has been painted
	}
	else if(!d_100mhz)	{	// no digit in the 100's MHz place?
		if(digit_100)	{	// was a digit present there before?
			UiLcdHy28_PrintText((POS_TUNE_SFREQ_X - SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// yes - mask the leading first digit
			digit_100 = 0;		// clear flag indicating that there was a digit so that we do not "paint" at that location again
		}
	}
	// -----------------------
	// See if 10 Mhz needs update
	d_10mhz = (dial_freq%100000000)/10000000;
	if(d_10mhz != df.sdial_010_mhz)
	{
		//printf("10 mhz diff: %d\n\r",d_10mhz);

		// To string
		digit[0] = 0x30 + (d_10mhz & 0x0F);

		// Update segment
		if(d_100mhz)
			UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);
		else	{
			if(d_10mhz)
				UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);
			else
				UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// mask the zero
		}
		// Save value
		df.sdial_010_mhz = d_10mhz;
		digit_10 = 1;		// indicate that a 10's MHz digit has been displayed
	}
	else if(!d_10mhz)	{	// no digit in the 10's MHz  place?
		if(digit_10)	{	// had a 10's MHz digit been painted?
			UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// yes - mask the leading first digit
			digit_10 = 0;	// clear indicator so that a "blank" digit is not repainted every time
		}
	}

	// -----------------------
	// See if 1 Mhz needs update
	d_1mhz = (dial_freq%10000000)/1000000;
	if(d_1mhz != df.sdial_001_mhz)
	{
		//printf("1 mhz diff: %d\n\r",d_1mhz);

		// To string
		digit[0] = 0x30 + (d_1mhz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_mhz = d_1mhz;
	}

	// -----------------------
	// See if 100 khz needs update
	d_100khz = (dial_freq%1000000)/100000;
	if(d_100khz != df.sdial_100_khz)
	{
		//printf("100 khz diff: %d\n\r",d_100khz);

		// To string
		digit[0] = 0x30 + (d_100khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*3),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_100_khz = d_100khz;
	}

	// -----------------------
	// See if 10 khz needs update
	d_10khz = (dial_freq%100000)/10000;
	if(d_10khz != df.sdial_010_khz)
	{
		//printf("10 khz diff: %d\n\r",d_10khz);

		// To string
		digit[0] = 0x30 + (d_10khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*4),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_010_khz = d_10khz;
	}

	// -----------------------
	// See if 1 khz needs update
	d_1khz = (dial_freq%10000)/1000;
	if(d_1khz != df.sdial_001_khz)
	{
		//printf("1 khz diff: %d\n\r",d_1khz);

		// To string
		digit[0] = 0x30 + (d_1khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*5),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_khz = d_1khz;
	}

	// -----------------------
	// See if 100 hz needs update
	d_100hz = (dial_freq%1000)/100;
	if(d_100hz != df.sdial_100_hz)
	{
		//printf("100 hz diff: %d\n\r",d_100hz);

		// To string
		digit[0] = 0x30 + (d_100hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*7),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_100_hz = d_100hz;
	}

	// -----------------------
	// See if 10 hz needs update
	d_10hz = (dial_freq%100)/10;
	if(d_10hz != df.sdial_010_hz)
	{
		//printf("10 hz diff: %d\n\r",d_10hz);

		// To string
		digit[0] = 0x30 + (d_10hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*8),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_010_hz = d_10hz;
	}

	// -----------------------
	// See if 1 hz needs update
	d_1hz = (dial_freq%10)/1;
	if(d_1hz != df.sdial_001_hz)
	{
		//printf("1 hz diff: %d\n\r",d_1hz);

		// To string
		digit[0] = 0x30 + (d_1hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*9),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_hz = d_1hz;
	}
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
			ts.audio_gain_active = 0;	// yes - null out audio
			Codec_Volume(0);
		}
		else if((ts.audio_gain != ts.audio_gain_change) || (unmute_flag) || ts.band_change)	{	// in normal mode - calculate volume normally
			ts.audio_gain_change = ts.audio_gain;
			ts.audio_gain_active = 1;		// software gain not active - set to unity
			if(ts.audio_gain <= 16)				// Note:  Gain > 16 adjusted in audio_driver.c via software
				Codec_Volume((ts.audio_gain*5));
			else	{	// are we in the "software amplification" range?
				Codec_Volume((80));		// set to fixed "maximum" gain
				ts.audio_gain_active = (float)ts.audio_gain;	// to float
				ts.audio_gain_active /= 2.5;	// rescale to reasonable step size
				ts.audio_gain_active -= 5.35;	// offset to get gain multiplier value
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
/*				else
					UiDriverChangeStGain(1);	// enable sidetone gain if CW mode
*/
				//
				// change display related to encoder one to TX mode (e.g. CW speed or MIC/LINE gain)
				//
				enc_three_mode = ts.enc_thr_mode;
				ts.enc_thr_mode = ENC_THREE_MODE_CW_SPEED;
				UiDriverChangeRit(0);
				if(ts.dmod_mode != DEMOD_CW)
					UIDriverChangeAudioGain(1);		// enable audio gain
/*				else
					UiDriverChangeKeyerSpeed(1);	// enable keyer speed if it was CW mode
*/
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
/*					else
						UiDriverChangeStGain(0);	// disable sidetone gain (if in CW mode)
*/
				}
				//
				ts.enc_thr_mode = enc_three_mode;
				if(ts.enc_thr_mode == ENC_THREE_MODE_RIT)	{		// are we to switch back to RIT mode?
					UiDriverChangeRit(1);			// enable RIT
					if(ts.dmod_mode != DEMOD_CW)
						UIDriverChangeAudioGain(0);		// disable audio gain if it was voice mode
/*					else
						UiDriverChangeKeyerSpeed(0);	// disable keyer speed if it was CW mode
*/
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
/*
	if(ts.dsp_check)	{
		ts.dsp_check = 0;
		char txt[16];
		sprintf(txt, " %d ", (ulong)(fabs(ads.dsp_nr_sample)));
		UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,txt,0xFFFF,0,0);
		sprintf(txt, " %d ", (ulong)(ads.dsp_zero_count));
		UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y,txt,0xFFFF,0,0);
	}
*/
	///
	// DSP crash detection
	//
	if((ts.dsp_active & 1) && (!(ts.dsp_active & 2)) && (!ads.af_dissabled) && (!ts.dsp_inhibit))	{	// Do this if enabled and "Pre-AGC" DSP NR enabled
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
			UiDriverClearSpectrumDisplay();			// clear display under spectrum scope
			UiLcdHy28_PrintText(110,156,"- New F/W detected -",Cyan,Black,0);
			UiLcdHy28_PrintText(110,168," Preparing EEPROM ",Cyan,Black,0);
			UiDriverSaveEepromValuesPowerDown();	// rewrite EEPROM values
			Write_EEPROM(EEPROM_VERSION_NUMBER, ts.version_number_release);	// save version number information to EEPROM
			Write_EEPROM(EEPROM_VERSION_BUILD, ts.version_number_build);	//
			Write_EEPROM(EEPROM_VERSION_MINOR, ts.version_number_minor);	//
			for(i = 0; i < 6; i++)			// delay so that it may be read
				non_os_delay();
			UiLcdHy28_PrintText(110,180,"      Done!       ",Cyan,Black,0);
			for(i = 0; i < 6; i++)			// delay so that it may be read
				non_os_delay();
			//
			UiInitSpectrumScopeWaterfall();			// init spectrum scope
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
			if(!(ts.misc_flags2 & 1))	// is FM to be disabled?
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
			if((ts.lsb_usb_auto_select == AUTO_LSB_USB_60M) && ((df.tune_new < USB_FREQ_THRESHOLD) && (ts.band != 1)))	{	// are we <10 MHz and NOT on 60 meters?
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


	// Finally update public flag
	ts.dmod_mode = loc_mode;

	// Set SoftDDS in CW mode
	if(ts.dmod_mode == DEMOD_CW)
		softdds_setfreq((float)ts.sidetone_freq,ts.samp_rate,0);
	else
		softdds_setfreq(0.0,ts.samp_rate,0);


	// Update Decode Mode (USB/LSB/AM/FM/CW)

	UiDriverShowMode();

	UiCalcRxPhaseAdj();		// set gain and phase values according to mode
	UiCalcRxIqGainAdj();
	//
	UiCalcTxPhaseAdj();
	UiCalcTxIqGainAdj();

	// Change function buttons caption
	//UiDriverCreateFunctionButtons(false);
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
		band_dial_value[curr_band_index] = df.tune_old;

		// Save decode mode
		band_decod_mode[curr_band_index] = ts.dmod_mode;

		// Save filter setting
		band_filter_mode[curr_band_index] = ts.filter_id;

		//printf("saved freq: %d and mode: %d\n\r",band_dial_value[curr_band_index],band_decod_mode[curr_band_index]);
	}

	// Handle direction
	if(is_up)
	{
		if(curr_band_index < (MAX_BANDS - 1))
		{
			//printf("going up band\n\r");

			// Increase
			new_band_freq  = tune_bands[curr_band_index + 1];
			new_band_index = curr_band_index + 1;
		}
		else	{	// wrap around to the lowest band
			new_band_freq = tune_bands[MIN_BANDS];
			new_band_index = MIN_BANDS;
		}
	}
	else
	{
		if(curr_band_index)
		{
			//printf("going down band\n\r");

			// Decrease
			new_band_freq  = tune_bands[curr_band_index - 1];
			new_band_index = curr_band_index - 1;
		}
		else	{	// wrap around to the highest band
			new_band_freq = tune_bands[MAX_BANDS - 1];
			new_band_index = MAX_BANDS -1;
		}
	}
	//printf("new band index: %d and freq: %d\n\r",new_band_index,new_band_freq);
	//
	// Load frequency value - either from memory or default for
	// the band if this is first band selection
	if(band_dial_value[new_band_index] != 0xFFFFFFFF)
	{
		//printf("load value from memory\n\r");

		// Load old frequency from memory
		df.tune_new = band_dial_value[new_band_index];
	}
	else
	{
		//printf("load default band freq\n\r");

		// Load default band startup frequency
		df.tune_new = new_band_freq;
	}

//	UiDriverUpdateFrequency(1,0);

//	// Also reset second freq display
//	UiDriverUpdateSecondLcdFreq(df.tune_new/4);

	// Change decode mode if need to
	if(ts.dmod_mode != band_decod_mode[new_band_index])
	{
		// Update mode
		ts.dmod_mode = band_decod_mode[new_band_index];

		// Update Decode Mode (USB/LSB/AM/FM/CW)
		UiDriverShowMode();
	}

	// Change filter mode if need to
	if(ts.filter_id != band_filter_mode[new_band_index])
	{
		ts.filter_id = band_filter_mode[new_band_index];
		UiDriverChangeFilter(0);	// update display and change filter
		UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator
		audio_driver_set_rx_audio_filter();
		audio_driver_set_rx_audio_filter();	// we have to invoke the filter change several times for some unknown reason - 'dunno why!
	}

	// Create Band value
	UiDriverShowBand(new_band_index);

	// Set TX power factor
	UiDriverSetBandPowerFactor(new_band_index);

	// Set filters
	UiDriverChangeBandFilter(new_band_index,0);

	// Finally update public flag
	ts.band = new_band_index;
	// Display frequency update
	//
	ts.refresh_freq_disp = 1;	// make frequency display refresh all digits
	//
	if(ts.vfo_mem_mode & 0x80)	{	// in SPLIT mode?
		UiDriverUpdateFrequency(1,3);	// force display of second (TX) VFO frequency
		UiDriverUpdateFrequency(1,2);	// update RX frequency
	}
	else	// not in SPLIT mode - standard update
		UiDriverUpdateFrequency(1,0);
	//
	ts.refresh_freq_disp = 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckFrequencyEncoder
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static bool UiDriverCheckFrequencyEncoder(void)
{
	int 		pot_diff;


	// Skip too regular read of the timer value, to avoid flickering
//	df.update_skip++;
//	if(df.update_skip < FREQ_UPDATE_SKIP)
//		return false;

//	df.update_skip = 0;

	// Load pot value
	df.value_new = TIM_GetCounter(TIM8);

	// Ignore lower value flickr
	if(df.value_new < ENCODER_FLICKR_BAND)
		return false;

	// Ignore higher value flickr
	if(df.value_new > (FREQ_ENCODER_RANGE/FREQ_ENCODER_LOG_D) + ENCODER_FLICKR_BAND)
		return false;

	// No change, return
	if(df.value_old == df.value_new)
		return false;

	UiLCDBlankTiming();	// calculate/process LCD blanking timing

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	df.de_detent++;
	if(df.de_detent < USE_DETENTED_VALUE)
	{
		df.value_old = df.value_new; // update and skip
		return false;
	}
	df.de_detent = 0;
#endif

	if(ts.txrx_mode != TRX_MODE_RX)		// do not allow tuning if in transmit mode
		return false;

	if(ks.button_just_pressed)		// press-and-hold - button just pressed for "temporary" step size change (not taken effect yet)
		return false;

	if(ts.frequency_lock)
		return false;						// frequency adjust is locked

	//printf("freq pot: %d \n\r",df.value_new);

	// Encoder value to difference
	if(df.value_new > df.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	//printf("pot diff: %d\n\r",pot_diff);

	// Finaly convert to frequency incr/decr
	if(pot_diff < 0)
		df.tune_new -= (df.tuning_step * 4);
	else
		df.tune_new += (df.tuning_step * 4);

	// Updated
	df.value_old = df.value_new;

	return true;
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
	char 	temp[10];
	int 	pot_diff;

	eos.value_new = TIM_GetCounter(TIM3);

	// Ignore lower value flickr
	if(eos.value_new < ENCODER_FLICKR_BAND)
		return;

	// Ignore lower value flickr
	if(eos.value_new > (ENCODER_ONE_RANGE/ENCODER_ONE_LOG_D) + ENCODER_FLICKR_BAND)
		return;

	// No change, return
	if(eos.value_old == eos.value_new)
		return;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	eos.de_detent++;
	if(eos.de_detent < USE_DETENTED_VALUE)
	{
		eos.value_old = eos.value_new; // update and skip
		return;
	}
	eos.de_detent = 0;
#endif

	//printf("gain pot: %d\n\r",gs.value_new);

	// Encoder value to difference
	if(eos.value_new > eos.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	//printf("pot diff: %d\n\r",pot_diff);

	UiLCDBlankTiming();	// calculate/process LCD blanking timing

	// Take appropriate action
	switch(ts.enc_one_mode)
	{
		// Update audio volume
		case ENC_ONE_MODE_AUDIO_GAIN:
		{
			// Convert to Audio Gain incr/decr
			if(pot_diff < 0)
			{
				if(ts.audio_gain)
					ts.audio_gain -= 1;
			}
			else
			{
				ts.audio_gain += 1;
				if(ts.audio_gain > ts.audio_max_volume)
					ts.audio_gain = ts.audio_max_volume;
			}

			// Value to string
			sprintf(temp,"%02d",ts.audio_gain);

			// Update screen indicator
			UiLcdHy28_PrintText((POS_AG_IND_X + 38),(POS_AG_IND_Y + 1), temp,White,Black,0);
			//
			// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
			//
			if(sd.use_spi)
				ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
			//
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
				//
				// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
				//
				if(sd.use_spi)
					ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
				//
				/*
				// Value to string
				sprintf(temp,"%02d",ts.st_gain);

				// Update screen indicator
				UiLcdHy28_PrintText((POS_SG_IND_X + 30),(POS_SG_IND_Y + 1), temp,White,Black,0);
				*/
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
				UiCalcTxCompLevel();		// calculate values for selection compression level
				UiDriverChangeCmpLevel(1);	// update on-screen display
				//
				// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
				//
				if(sd.use_spi)
					ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
				//
			}

			break;
		}

		default:
			break;
	}

	// Updated
	eos.value_old = eos.value_new;

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


	ews.value_new = TIM_GetCounter(TIM4);

	// Ignore lower value flickr
	if(ews.value_new < ENCODER_FLICKR_BAND)
		return;

	// Ignore lower value flickr
	if(ews.value_new > (ENCODER_TWO_RANGE/ENCODER_TWO_LOG_D) + ENCODER_FLICKR_BAND)
		return;

	// No change, return
	if(ews.value_old == ews.value_new)
		return;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	ews.de_detent++;
	if(ews.de_detent < USE_DETENTED_VALUE)
	{
		ews.value_old = ews.value_new; // update and skip
		return;
	}
	ews.de_detent = 0;
#endif

	//printf("gain pot: %d\n\r",gs.value_new);

	// Encoder value to difference
	if(ews.value_new > ews.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	//printf("pot diff: %d\n\r",pot_diff);

	UiLCDBlankTiming();	// calculate/process LCD blanking timing

	if(ts.menu_mode)	{
		if(pot_diff < 0)	{
			if(ts.menu_item)	{
				ts.menu_item--;
			}
			else	{
				if(!ts.radio_config_menu_enable)
					ts.menu_item = MAX_MENU_ITEM-1;	// move to the last menu item (e.g. "wrap around")
				else
					ts.menu_item = (MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS)-1;	// move to the last menu item (e.g. "wrap around")
			}
		}
		else	{
			ts.menu_item++;
			if(!ts.radio_config_menu_enable)	{
				if(ts.menu_item >= MAX_MENU_ITEM)	{
					ts.menu_item = 0;	// Note:  ts.menu_item is numbered starting at zero
				}
			}
			else	{
				if(ts.menu_item >= MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS)	{
					ts.menu_item = 0;	// Note:  ts.menu_item is numbered starting at zero
				}
			}
		}
		ts.menu_var = 0;			// clear variable that is used to change a menu item
		UiDriverUpdateMenu(1);		// Update that menu item
		//
		// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
		//
		if(sd.use_spi)
			ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
		//
		goto skip_update;
	}
	else if(ts.mem_disp)	{
		if(pot_diff < 0)	{
			if(ts.menu_item)	{
				ts.menu_item--;		// decrement selected item
			}
		}
		else	{
			if(ts.menu_item > 31)	{
				ts.menu_item = 31;
			}
			else	{
				ts.menu_item++;		// increment selected item
			}
		}	//
		UiDriverMemMenu();		// perform update of selected item
		//
		// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
		//
		if(sd.use_spi)
			ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
		//
		goto skip_update;
	}
	//
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
					UiCalcRFGain();		// convert from user RF gain value to "working" RF gain value
					UiDriverChangeRfGain(1);	// change on screen
					//
					// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
					//
					if(sd.use_spi)
						ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
					//
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
					//
					// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
					//
					if(sd.use_spi)
						ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
					//
					break;
				}
			}

			// Update DSP/NB setting
			case ENC_TWO_MODE_SIG_PROC:
			{
				if(ts.dsp_active & 8)	{	// is it in noise blanker mode?
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
				else if(ts.dsp_active & 1)	{	// only allow adjustment if DSP NR is active
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
				//
				// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
				//
				if(sd.use_spi)
					ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
				//
				break;
			}

			default:
				break;
		}
	}

skip_update:

	// Updated
	ews.value_old = ews.value_new;
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

	ets.value_new = TIM_GetCounter(TIM5);

	// Ignore lower value flicker
	if(ets.value_new < ENCODER_FLICKR_BAND)
		return;

	// Ignore higher value flicker
	if(ets.value_new > (ENCODER_THR_RANGE/ENCODER_THR_LOG_D) + ENCODER_FLICKR_BAND)
		return;

	// No change, return
	if(ets.value_old == ets.value_new)
		return;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	ets.de_detent++;
	if(ets.de_detent < USE_DETENTED_VALUE)
	{
		ets.value_old = ets.value_new; // update and skip
		return;
	}
	ets.de_detent = 0;
#endif

	//printf("fir pot: %d\n\r",fs.value_new);

	// Encoder value to difference
	if(ets.value_new > ets.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	//printf("pot diff: %d\n\r",pot_diff);

	UiLCDBlankTiming();	// calculate/process LCD blanking timing

	if(ts.menu_mode)	{
		if(pot_diff < 0)	{
			ts.menu_var--;		// increment selected item
		}
		else	{
			ts.menu_var++;		// decrement selected item
		}
		//
		UiDriverUpdateMenu(1);		// perform update of selected item
		//
		// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
		//
		if(sd.use_spi)
			ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
		//
		goto skip_update;
	}



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
				//
				// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
				//
				if(sd.use_spi)
					ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
				//
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
				//
				// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
				//
				if(sd.use_spi)
					ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
				//
			}
			else	{	// in voice mode, adjust audio gain
				if(ts.tx_audio_source != TX_AUDIO_MIC)	{		// in LINE-IN mode?
					if(pot_diff < 0)	{						// yes, adjust line gain
						ts.tx_line_gain--;
						if(ts.tx_line_gain < LINE_GAIN_MIN)
							ts.tx_line_gain = LINE_GAIN_MIN;
					}
					else	{
						ts.tx_line_gain++;
						if(ts.tx_line_gain > LINE_GAIN_MAX)
							ts.tx_line_gain = LINE_GAIN_MAX;
					}
					//
					if((ts.txrx_mode == TRX_MODE_TX) && (ts.dmod_mode != DEMOD_CW))		// in transmit and in voice mode?
						Codec_Line_Gain_Adj(ts.tx_line_gain);		// change codec gain
				}
				else	{
					if(pot_diff < 0)	{						// yes, adjust line gain
						ts.tx_mic_gain--;
						if(ts.tx_mic_gain < MIC_GAIN_MIN)
							ts.tx_mic_gain = MIC_GAIN_MIN;
					}
					else	{
						ts.tx_mic_gain++;
						if(ts.tx_mic_gain > MIC_GAIN_MAX)
							ts.tx_mic_gain = MIC_GAIN_MAX;
					}
					if(ts.tx_mic_gain > 50)	{		// actively adjust microphone gain and microphone boost
						ts.mic_boost = 1;	// software boost active
						ts.tx_mic_gain_mult = (ts.tx_mic_gain - 35)/3;			// above 50, rescale software amplification
						if((ts.txrx_mode == TRX_MODE_TX) && (ts.dmod_mode != DEMOD_CW))	{		// in transmit and in voice mode?
							Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0015);	// set mic boost on
						}
					}
					else	{
						ts.mic_boost = 0;	// software mic gain boost inactive
						ts.tx_mic_gain_mult = ts.tx_mic_gain;
						if((ts.txrx_mode == TRX_MODE_TX) && (ts.dmod_mode != DEMOD_CW))	{	// in transmit and in voice mode?
							Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0014);	// set mic boost off
						}
					}
				}
				UIDriverChangeAudioGain(1);
				//
				// If using a serial (SPI) LCD, hold off on updating the spectrum scope for a time AFTER we stop twiddling the tuning knob.
				//
				if(sd.use_spi)
					ts.hold_off_spectrum_scope	= ts.sysclock + SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE;	// schedule the time after which we again update the spectrum scope
				//
			}
			break;
		}

		default:
			break;
	}

skip_update:

	// Updated
	ets.value_old = ets.value_new;
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
				UIDriverChangeAudioGain(0);

			break;
		}

		case ENC_THREE_MODE_CW_SPEED:
		{
			// RIT
			UiDriverChangeRit(0);

			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeKeyerSpeed(1);
			else
				UIDriverChangeAudioGain(1);

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
				UIDriverChangeAudioGain(0);

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
static void UiDriverChangeAfGain(uchar enabled)
{
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_AG_IND_X,POS_AG_IND_Y,13,57,Grey);

	if(enabled)
		UiLcdHy28_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1),"AFG",Black,Grey,0);
	else
		UiLcdHy28_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1),"AFG",Grey1,Grey,0);

	sprintf(temp,"%02d",ts.audio_gain);
	UiLcdHy28_PrintText    ((POS_AG_IND_X + 38),(POS_AG_IND_Y + 1), temp,color,Black,0);
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
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_SG_IND_X,POS_SG_IND_Y,13,49,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_SG_IND_X + 1), (POS_SG_IND_Y + 1),"STG",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_SG_IND_X + 1), (POS_SG_IND_Y + 1),"STG",Grey1,Grey,0);

	sprintf(temp,"%02d",ts.st_gain);
	UiLcdHy28_PrintText    ((POS_SG_IND_X + 30),(POS_SG_IND_Y + 1), temp,color,Black,0);
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
	ushort 	color = White;
	char	temp[100];


	UiLcdHy28_DrawEmptyRect( POS_SG_IND_X,POS_SG_IND_Y,13,49,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_SG_IND_X + 1), (POS_SG_IND_Y + 1),"CMP",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_SG_IND_X + 1), (POS_SG_IND_Y + 1),"CMP",Grey1,Grey,0);

	if(ts.tx_comp_level < TX_AUDIO_COMPRESSION_MAX)	{	// 	display numbers for all but the highest value
		sprintf(temp,"%02d",ts.tx_comp_level);
	}
	else	{				// show "SV" (Stored Value) for highest value
		strcpy(temp, "SV");
		color = Yellow;	// Stored value - use yellow
	}

	if(enabled == 0)		// always display grey if disabled
		color = Grey;

	UiLcdHy28_PrintText    ((POS_SG_IND_X + 30),(POS_SG_IND_Y + 1), temp,color,Black,0);
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
	char txt[32];
	ulong	x_off = 0;

	// Draw line for upper box
	UiLcdHy28_DrawStraightLine(POS_DSPU_IND_X,(POS_DSPU_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	// Draw line for lower box
	UiLcdHy28_DrawStraightLine(POS_DSPL_IND_X,(POS_DSPL_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	//
	if(((ts.dsp_active & 1) || (ts.dsp_active & 4)))	// DSP active and NOT in FM mode?
		color = White;
	else	// DSP not active
		color = Grey2;
	//
	UiLcdHy28_PrintText((POS_DSPU_IND_X),(POS_DSPU_IND_Y),"  DSP  ",White,Orange,0);
	//
	if(ts.dmod_mode == DEMOD_FM)	{		// Grey out and display "off" if in FM mode
		sprintf(txt, "  OFF ");
		color = Grey2;
	}
	else if((ts.dsp_active & 1) && (ts.dsp_active & 4) && (ts.dmod_mode != DEMOD_CW))	{
		sprintf(txt, "NR+NOT");
		x_off = 4;
	}
	else if(ts.dsp_active & 1)	{
		sprintf(txt, "  NR  ");
		x_off = 4;
	}
	else if(ts.dsp_active & 4)	{
		sprintf(txt, " NOTCH");
		if(ts.dmod_mode == DEMOD_CW)
			color = Grey2;
	}
	else
		sprintf(txt, "  OFF");
	//
	UiLcdHy28_PrintText((POS_DSPU_IND_X),(POS_DSPL_IND_Y),"       ",White,Blue,0);
	UiLcdHy28_PrintText((POS_DSPL_IND_X+x_off),(POS_DSPL_IND_Y),txt,color,Blue,0);

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

	// Draw top line
	UiLcdHy28_DrawStraightLine(POS_PW_IND_X,(POS_PW_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);

	switch(ts.power_level)
	{
		case PA_LEVEL_5W:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"   5W  ",color,Blue,0);
			break;
		case PA_LEVEL_2W:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"   2W  ",color,Blue,0);
			break;
		case PA_LEVEL_1W:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"   1W  ",color,Blue,0);
			break;
		case PA_LEVEL_0_5W:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"  0.5W ",color,Blue,0);
			break;
		default:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"  FULL ",color,Blue,0);
			break;
	}

	// Set TX power factor - to reflect changed power
	UiDriverSetBandPowerFactor(ts.band);
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
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_KS_IND_X,POS_KS_IND_Y,13,49,Grey);

	if(enabled)
		UiLcdHy28_PrintText((POS_KS_IND_X + 1), (POS_KS_IND_Y + 1),"WPM",Black,Grey,0);
	else
		UiLcdHy28_PrintText((POS_KS_IND_X + 1), (POS_KS_IND_Y + 1),"WPM",Grey1,Grey,0);

	memset(temp,0,100);
	sprintf(temp,"%2d",ts.keyer_speed);

	UiLcdHy28_PrintText    ((POS_KS_IND_X + 30),(POS_KS_IND_Y + 1), temp,color,Black,0);

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
void UIDriverChangeAudioGain(uchar enabled)
{
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_KS_IND_X,POS_KS_IND_Y,13,49,Grey);

	if(ts.tx_audio_source == TX_AUDIO_MIC)		// Microphone gain
		strcpy(temp, "MIC");
	else										// Line gain
		strcpy(temp, "LIN");

	if(enabled)
		UiLcdHy28_PrintText((POS_KS_IND_X + 1), (POS_KS_IND_Y + 1),temp,Black,Grey,0);
	else
		UiLcdHy28_PrintText((POS_KS_IND_X + 1), (POS_KS_IND_Y + 1),temp,Grey1,Grey,0);

	memset(temp,0,100);

	if(ts.tx_audio_source == TX_AUDIO_MIC)		// Mic gain mode
		sprintf(temp,"%2d",ts.tx_mic_gain);
	else
		sprintf(temp,"%2d",ts.tx_line_gain);	// Line gain mode

	UiLcdHy28_PrintText    ((POS_KS_IND_X + 30),(POS_KS_IND_Y + 1), temp,color,Black,0);

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
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_RF_IND_X,POS_RF_IND_Y,13,57,Grey);

	if(ts.dmod_mode != DEMOD_FM)	{	// If not FM, use RF gain



		if(enabled)	{
			UiLcdHy28_PrintText((POS_RF_IND_X + 1), (POS_RF_IND_Y + 1),"RFG",Black,Grey,0);
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
		else
			UiLcdHy28_PrintText((POS_RF_IND_X + 1), (POS_RF_IND_Y + 1),"RFG",Grey1,Grey,0);

		sprintf(temp," %02d",ts.rf_gain);
	}
	else	{						// it is FM, display squelch instead

		if(enabled)
			UiLcdHy28_PrintText((POS_RF_IND_X + 1), (POS_RF_IND_Y + 1),"SQL",Black,Grey,0);
		else
			UiLcdHy28_PrintText((POS_RF_IND_X + 1), (POS_RF_IND_Y + 1),"SQL",Grey1,Grey,0);

		sprintf(temp," %02d",(int)ts.fm_sql_threshold);
	}
		UiLcdHy28_PrintText    ((POS_RF_IND_X + 32),(POS_RF_IND_Y + 1), temp,color,Black,0);

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
	ushort 	color = Grey;
	char	temp[100];

	UiLcdHy28_DrawEmptyRect( POS_RA_IND_X,POS_RA_IND_Y,13,49,Grey);		// draw box

	//
	// Noise blanker settings display
	//
	if(ts.dsp_active & 8)	{	// is noise blanker to be displayed
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
		//
		if((!enabled) || (ts.dmod_mode == DEMOD_AM) || (ts.dmod_mode == DEMOD_FM) || (ts.filter_id == AUDIO_WIDE))	// is NB disabled, at 10 kHZ and/or are we in AM mode?
			UiLcdHy28_PrintText    ((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1),"NB ",Grey1,Grey,0);	// yes - it is gray
		else
			UiLcdHy28_PrintText    ((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1),"NB ",Black,Grey,0);
		//
		sprintf(temp,"%02d",ts.nb_setting);
	}
	//
	// DSP settings display
	//
	else	{			// DSP settings are to be displayed
		if(enabled && (ts.dsp_active & 1))	{	// if this menu is enabled AND the DSP NR is also enabled...
			color = White;		// Make it white by default
			//
			if(ts.dsp_nr_strength >= DSP_STRENGTH_RED)
				color = Red;
			else if(ts.dsp_nr_strength >= DSP_STRENGTH_ORANGE)
				color = Orange;
			else if(ts.dsp_nr_strength >= DSP_STRENGTH_YELLOW)
				color = Yellow;
		}
		//
		if(enabled)
			UiLcdHy28_PrintText    ((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1),"DSP",Black,Grey,0);
		else
			UiLcdHy28_PrintText    ((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1),"DSP",Grey1,Grey,0);

		sprintf(temp,"%02d",ts.dsp_nr_strength);
	}
	//
	// display numerical value
	//
	UiLcdHy28_PrintText    ((POS_RA_IND_X + 30),(POS_RA_IND_Y + 1), temp,color,Black,0);
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
	char	temp[100];
	ushort 	color = Grey;

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_RIT_IND_X,POS_RIT_IND_Y,13,57,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 1),"RIT",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 1),"RIT",Grey1,Grey,0);

	if(ts.rit_value >= 0)
		sprintf(temp,"+%i",ts.rit_value);
	else
		sprintf(temp,"%i", ts.rit_value);

	UiLcdHy28_PrintText((POS_RIT_IND_X + 30),(POS_RIT_IND_Y + 1),"000",Black,Black,0); // clear screen
	UiLcdHy28_PrintText((POS_RIT_IND_X + 30),(POS_RIT_IND_Y + 1), temp,color,Black,0);
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
	ushort fcolor = Grey;
	char txt[16];

	fcolor = White;
	UiLcdHy28_PrintText(POS_FIR_IND_X,  POS_FIR_IND_Y,       " FILT  ",	White, 	Orange, 0);

	// Do a filter re-load
	if(!ui_only_update)
		audio_driver_set_rx_audio_filter();

	// Draw top line
	UiLcdHy28_DrawStraightLine(POS_FIR_IND_X,(POS_FIR_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);

	// Clear screen
	UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"00000", Black, Black,  0);

	// Update screen indicator
	if(ts.dmod_mode != DEMOD_FM)	{	// in modes OTHER than FM
		switch(ts.filter_id)
		{
			case AUDIO_300HZ:
				UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15)," 300Hz", fcolor,Black,0);
				break;

			case AUDIO_500HZ:
			UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15)," 500Hz", fcolor,Black,0);
			break;

			case AUDIO_1P8KHZ:
				UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"  1.8k", fcolor,Black,0);
				break;

			case AUDIO_2P3KHZ:
				UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"  2.3k", fcolor,Black,0);
				break;

			case AUDIO_3P6KHZ:
				UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"  3.6k", fcolor,Black,0);
				break;

			case AUDIO_WIDE:
				switch(ts.filter_wide_select)	{
					case WIDE_FILTER_5K:
					case WIDE_FILTER_5K_AM:
						sprintf(txt,"   5k ");
						break;
					case WIDE_FILTER_6K:
					case WIDE_FILTER_6K_AM:
						sprintf(txt,"   6k ");
						break;
					case WIDE_FILTER_7K5:
					case WIDE_FILTER_7K5_AM:
						sprintf(txt,"  7.5k");
						break;
					case WIDE_FILTER_10K:
					case WIDE_FILTER_10K_AM:
					default:
						sprintf(txt,"  10k ");
						break;
				}
				UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),txt,fcolor,Black,0);
				break;
				default:
					break;
		}
	}
	else	{		// This is the FM special case to display bandwidth
		switch(ts.fm_rx_bandwidth)	{
			case FM_RX_BANDWIDTH_7K2:
				sprintf(txt,"7k2 FM");
				break;
			case FM_RX_BANDWIDTH_12K:
				sprintf(txt,"12k FM");
				break;
//			case FM_RX_BANDWIDTH_15K:
//				sprintf(txt,"15k FM");
//				break;
			case FM_RX_BANDWIDTH_10K:
			default:
				sprintf(txt,"10k FM");
				break;
		}
		UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),txt,fcolor,Black,0);
	}
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
	ushort clr;

	if(ts.menu_mode)	// bail out if in menu mode
		return;


	// Update screen indicator - first get the width and center-frequency offset of the currently-selected filter
	//
	switch(ts.filter_id)	{
		case AUDIO_300HZ:	// 300 Hz CW filter
			switch(ts.filter_300Hz_select)	{
				case 1:
					offset = FILT300_1;
					break;
				case 2:
					offset = FILT300_2;
					break;
				case 3:
					offset = FILT300_3;
					break;
				case 4:
					offset = FILT300_4;
					break;
				case 5:
					offset = FILT300_5;
					break;
				case 6:
					offset = FILT300_6;
					break;
				case 7:
					offset = FILT300_7;
					break;
				case 8:
					offset = FILT300_8;
					break;
				case 9:
					offset = FILT300_9;
					break;
				default:
					offset = FILT300_6;
					break;
			}
			width = FILTER_300HZ_WIDTH;
			//
			break;
		case AUDIO_500HZ:	// 500 Hz CW filter
			switch(ts.filter_500Hz_select)	{
				case 1:
					offset = FILT500_1;
					break;
				case 2:
					offset = FILT500_2;
					break;
				case 3:
					offset = FILT500_3;
					break;
				case 4:
					offset = FILT500_4;
					break;
				case 5:
					offset = FILT500_5;
					break;
				default:
					offset = FILT500_3;
					break;
			}
			width = FILTER_500HZ_WIDTH;
			//
			break;
		case AUDIO_1P8KHZ:		// 1.8 kHz wide filter
			switch(ts.filter_1k8_select)	{
				case 1:
					offset = FILT1800_1;
					break;
				case 2:
					offset = FILT1800_2;
					break;
				case 3:
					offset = FILT1800_3;
					break;
				case 4:
					offset = FILT1800_4;
					break;
				case 5:
					offset = FILT1800_5;
					break;
				default:
					offset = FILT1800_3;
					break;
			}
			width = FILTER_1800HZ_WIDTH;
			//
			break;
		case AUDIO_2P3KHZ:		// 2.3 kHz wide filter
			switch(ts.filter_2k3_select)	{
				case 1:
					offset = FILT2300_1;
					break;
				case 2:
					offset = FILT2300_2;
					break;
				case 3:
					offset = FILT2300_3;
					break;
				case 4:
					offset = FILT2300_4;
					break;
				default:
					offset = FILT2300_2;
					break;
			}
			width = FILTER_2300HZ_WIDTH;
			//
			break;

		case AUDIO_3P6KHZ:	// 3.6 kHz wide filter
			offset = FILT3600;
			width = FILTER_3600HZ_WIDTH;
			break;

		case AUDIO_WIDE:	// selectable "wide" bandwidth filter
			switch(ts.filter_wide_select)	{
				case WIDE_FILTER_5K:
				case WIDE_FILTER_5K_AM:
					offset = FILT5000;
					width = FILTER_5000HZ_WIDTH;
					break;
				case WIDE_FILTER_6K:
				case WIDE_FILTER_6K_AM:
					offset = FILT6000;
					width = FILTER_6000HZ_WIDTH;
					break;
				case WIDE_FILTER_7K5:
				case WIDE_FILTER_7K5_AM:
					offset = FILT7500;
					width = FILTER_7500HZ_WIDTH;
					break;
				case WIDE_FILTER_10K:
				case WIDE_FILTER_10K_AM:
				default:
					offset = FILT10000;
					width = FILTER_10000HZ_WIDTH;
					break;
			}
			break;
		default:
			break;
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
			offset = FILT6000;												// display bandwidth of +/- 6 kHz = 12 kHz
			width = FILTER_6000HZ_WIDTH;
		}
//		else if(ts.fm_rx_bandwidth == FM_RX_BANDWIDTH_15K)	{
//			offset = FILT7500;												// display bandwidth of +/- 7.5 kHz = 15 kHz
//			width = FILTER_7500HZ_WIDTH;
//		}
		else	{			// this will be the 10 kHz BW mode - I hope!
			offset = FILT5000;												// display bandwidth of +/- 5 kHz = 10 kHz
			width = FILTER_5000HZ_WIDTH;
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
		if(!ts.iq_freq_mode)					// frequency translate mode is off
			lpos = 130;							// line is in center
		else if(ts.iq_freq_mode == 1)			// line is to left if in "RX LO HIGH" mode
			lpos = 98;
		else if(ts.iq_freq_mode == 2)			// line is to right if in "RX LO LOW" mode
			lpos = 162;
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

	//
	//	erase old line
	//
	UiLcdHy28_DrawStraightLine((POS_SPECTRUM_IND_X), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y), 256, LCD_DIR_HORIZONTAL, Black);
	UiLcdHy28_DrawStraightLine((POS_SPECTRUM_IND_X), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y + 1), 256, LCD_DIR_HORIZONTAL, Black);
	//
	//
	// get color for line
	//
	if(ts.filter_disp_colour == SPEC_GREY)
		clr = Grey;
	else if(ts.filter_disp_colour == SPEC_BLUE)
		clr = Blue;
	else if(ts.filter_disp_colour == SPEC_RED)
		clr = Red;
	else if(ts.filter_disp_colour == SPEC_MAGENTA)
		clr = Magenta;
	else if(ts.filter_disp_colour == SPEC_GREEN)
		clr = Green;
	else if(ts.filter_disp_colour == SPEC_CYAN)
		clr = Cyan;
	else if(ts.filter_disp_colour == SPEC_YELLOW)
		clr = Yellow;
	else if(ts.filter_disp_colour == SPEC_BLACK)
		clr = Black;
	else if(ts.filter_disp_colour == SPEC_ORANGE)
		clr = Orange;
	else
		clr = White;
	//
	// draw line
	//
	UiLcdHy28_DrawStraightLine((POS_SPECTRUM_IND_X + lpos), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y), (ushort)width, LCD_DIR_HORIZONTAL, clr);
	UiLcdHy28_DrawStraightLine((POS_SPECTRUM_IND_X + lpos), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y + 1), (ushort)width, LCD_DIR_HORIZONTAL, clr);

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
static void UiDriverFFTWindowFunction(char mode)
{
	ulong i;

	// Information on these windowing functions may be found on the internet - check the Wikipedia article "Window Function"
	// KA7OEI - 20150602

	switch(mode)	{
		case FFT_WINDOW_RECTANGULAR:	// No processing at all - copy from "Samples" buffer to "Windat" buffer
			arm_copy_f32((float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_Windat,FFT_IQ_BUFF_LEN);	// use FFT data as-is
			break;
		case FFT_WINDOW_COSINE:			// Sine window function (a.k.a. "Cosine Window").  Kind of wide...
			for(i = 0; i < FFT_IQ_BUFF_LEN; i++){
				sd.FFT_Windat[i] = arm_sin_f32((PI * (float32_t)i)/FFT_IQ_BUFF_LEN - 1) * sd.FFT_Samples[i];
			}
			break;
		case FFT_WINDOW_BARTLETT:		// a.k.a. "Triangular" window - Bartlett (or Fejér) window is special case where demonimator is "N-1". Somewhat better-behaved than Rectangular
			for(i = 0; i < FFT_IQ_BUFF_LEN; i++){
				sd.FFT_Windat[i] = (1 - fabs(i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF) * sd.FFT_Samples[i];
			}
			break;
		case FFT_WINDOW_WELCH:			// Parabolic window function, fairly wide, comparable to Bartlett
			for(i = 0; i < FFT_IQ_BUFF_LEN; i++){
				sd.FFT_Windat[i] = (1 - ((i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF)*((i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF)) * sd.FFT_Samples[i];
			}
			break;
		case FFT_WINDOW_HANN:			// Raised Cosine Window (non zero-phase version) - This has the best sidelobe rejection of what is here, but not as narrow as Hamming.
			for(i = 0; i < FFT_IQ_BUFF_LEN; i++){
			    sd.FFT_Windat[i] = 0.5 * (float32_t)((1 - (arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
			}
			break;
		case FFT_WINDOW_HAMMING:		// Another Raised Cosine window - This is the narrowest with reasonably good sidelobe rejection.
			for(i = 0; i < FFT_IQ_BUFF_LEN; i++){
			    sd.FFT_Windat[i] = (float32_t)((0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
			}
			break;
		case FFT_WINDOW_BLACKMAN:		// Approx. same "narrowness" as Hamming but not as good sidelobe rejection - probably best for "default" use.
			for(i = 0; i < FFT_IQ_BUFF_LEN; i++){
			    sd.FFT_Windat[i] = (0.42659 - (0.49656*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.076849*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];
			}
			break;
		case FFT_WINDOW_NUTTALL:		// Slightly wider than Blackman, comparable sidelobe rejection.
			for(i = 0; i < FFT_IQ_BUFF_LEN; i++){
			    sd.FFT_Windat[i] = (0.355768 - (0.487396*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.144232*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) - (0.012604*arm_cos_f32((6*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];
			}
			break;
	}
	//
	// used for debugging
//		char txt[32];
//		sprintf(txt, " %d    ", (int)(c1));
//		UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 20),txt,White,Grid,0);
}
//
//
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverInitSpectrumDisplay - for both "Spectrum Display" and "Waterfall" Display
//* Object              : FFT init
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverInitSpectrumDisplay(void)
{
	ulong i;
	arm_status	a;

	// Init publics
	sd.state 		= 0;
	sd.samp_ptr 	= 0;
	sd.skip_process = 0;
	sd.enabled		= 0;
	sd.dial_moved	= 0;
	//
	sd.rescale_rate = (float)ts.scope_rescale_rate;	// load rescale rate
	sd.rescale_rate = 1/sd.rescale_rate;				// actual rate is inverse of this setting
	//
	sd.agc_rate = (float)ts.scope_agc_rate;	// calculate agc rate
	sd.agc_rate = sd.agc_rate/SPECTRUM_AGC_SCALING;
	//
	sd.mag_calc = 1;				// initial setting of spectrum display scaling factor
	//
	sd.wfall_line_update = 0;		// init count used for incrementing number of lines for each vertical waterfall screen update
	//
	// load buffer containing waterfall colours
	//
	for(i = 0; i < NUMBER_WATERFALL_COLOURS; i++)	{
		switch(ts.waterfall_color_scheme)	{
			case WFALL_HOT_COLD:
				sd.waterfall_colours[i] = waterfall_cold_hot[i];
				break;
			case WFALL_RAINBOW:
				sd.waterfall_colours[i] = waterfall_rainbow[i];
				break;
			case WFALL_BLUE:
				sd.waterfall_colours[i] = waterfall_blue[i];
				break;
			case WFALL_GRAY_INVERSE:
				sd.waterfall_colours[i] = waterfall_grey_inverse[i];
				break;
			case WFALL_GRAY:
			default:
				sd.waterfall_colours[i] = waterfall_grey[i];
				break;
		}
	}
	//
	//
	// Load "top" color of palette (the 65th) with that to be used for the center grid color
	//
	sd.waterfall_colours[NUMBER_WATERFALL_COLOURS] = (ushort)ts.scope_centre_grid_colour_active;
	//
	//
/*
	//
	// Load waterfall data with "splash" showing palette
	//
	j = 0;					// init count of lines on display
	k = sd.wfall_line;		// start with line currently displayed in buffer
	while(j < SPECTRUM_HEIGHT)	{		// loop number of times of buffer
		for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{		// do this all of the way across, horizonally
			sd.waterfall[k][i] = (SPECTRUM_HEIGHT - j) % SPECTRUM_HEIGHT;	// place the color of the palette, indexed to vertical position
		}
		j++;		// update line count
		k++;		// update position within circular buffer - which also is used to calculate color
		k %= SPECTRUM_HEIGHT;	// limit to modulus count of circular buffer size
	}
	//
*/
	//
	switch(ts.spectrum_db_scale)	{
	case	DB_DIV_5:
		sd.db_scale = DB_SCALING_5;
		break;
	case	DB_DIV_7:
		sd.db_scale = DB_SCALING_7;
		break;
	case	DB_DIV_15:
		sd.db_scale = DB_SCALING_15;
		break;
	case	DB_DIV_20:
		sd.db_scale = DB_SCALING_20;
		break;
	case S_1_DIV:
		sd.db_scale = DB_SCALING_S1;
		break;
	case S_2_DIV:
		sd.db_scale = DB_SCALING_S2;
		break;
	case S_3_DIV:
		sd.db_scale = DB_SCALING_S3;
		break;
	case	DB_DIV_10:
	default:
		sd.db_scale = DB_SCALING_10;
		break;
	}
	//
	if(ts.waterfall_size == WATERFALL_NORMAL)	{						// waterfall the same size as spectrum scope
		sd.wfall_height = SPECTRUM_HEIGHT - SPECTRUM_SCOPE_TOP_LIMIT;
		sd.wfall_ystart = SPECTRUM_START_Y + SPECTRUM_SCOPE_TOP_LIMIT;
		sd.wfall_size = SPECTRUM_HEIGHT - SPECTRUM_SCOPE_TOP_LIMIT;
	}																	// waterfall larger, covering the word "Waterfall Display"
	else if(ts.waterfall_size == WATERFALL_MEDIUM)	{
		sd.wfall_height = SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL;
		sd.wfall_ystart = SPECTRUM_START_Y - WFALL_MEDIUM_ADDITIONAL;
		sd.wfall_size = SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL;
	}
	//
	//
	sd.wfall_contrast = (float)ts.waterfall_contrast;		// calculate scaling for contrast
	sd.wfall_contrast /= 100;

	// Init FFT structures
	a = arm_rfft_init_f32((arm_rfft_instance_f32 *)&sd.S,(arm_cfft_radix4_instance_f32 *)&sd.S_CFFT,FFT_IQ_BUFF_LEN,FFT_QUADRATURE_PROC,1);

	if(a != ARM_MATH_SUCCESS)
	{
		printf("fft init err: %d\n\r",a);
		return;
	}

	// Ready
	sd.enabled		= 1;
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverReDrawSpectrumDisplay
//* Object              : state machine implementation
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
// Spectrum Display code rewritten by C. Turner, KA7OEI, September 2014, May 2015
//
static void UiDriverReDrawSpectrumDisplay(void)
{
	ulong i, spec_width;
	uint32_t	max_ptr;	// throw-away pointer for ARM maxval and minval functions
	float32_t	gcalc;
	//

	// Only in RX mode and NOT while powering down or in menu mode or if displaying memory information
	if((ts.txrx_mode != TRX_MODE_RX) || (ts.powering_down) || (ts.menu_mode) || (ts.mem_disp) || (ts.boot_halt_flag))
		return;

	if((ts.spectrum_scope_scheduler) || (!ts.scope_speed))	// is it time to update the scan, or is this scope to be disabled?
		return;
	else
		ts.spectrum_scope_scheduler = (ts.scope_speed-1)*2;

	// No spectrum display in DIGI modes
	//if(ts.dmod_mode == DEMOD_DIGI)
	//	return;

	// Nothing to do here otherwise, or if scope is to be held off while other parts of the display are to be updated or the LCD is being blanked
	if((!sd.enabled) || (ts.hold_off_spectrum_scope > ts.sysclock) || (ts.lcd_blanking_flag))
		return;

	// The state machine will rest
	// in between states
//	sd.skip_process++;
//	if(sd.skip_process < 1000)
//		return;

//	sd.skip_process = 0;

	gcalc = 1/ads.codec_gain_calc;				// Get gain setting of codec and convert to multiplier factor

	// Process implemented as state machine
	switch(sd.state)
	{
		//
		// Apply gain to collected IQ samples and then do FFT
		//

		case 1:
		{
			arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)(gcalc * SCOPE_PREAMP_GAIN), (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN);	// scale input according to A/D gain
			//
			UiDriverFFTWindowFunction(ts.fft_window_type);		// do windowing function on input data to get less "Bin Leakage" on FFT data
			//
			arm_rfft_f32((arm_rfft_instance_f32 *)&sd.S,(float32_t *)(sd.FFT_Windat),(float32_t *)(sd.FFT_Samples));	// Do FFT
			//
		sd.state++;
		break;
		}
		//
		// Do magnitude processing and gain control (AGC) on input of FFT
		//
		case 2:
		{
			//
			// Save old display data - we will use later to mask pixel on the control
			//
			arm_copy_q15((q15_t *)sd.FFT_DspData, (q15_t *)sd.FFT_BkpData, FFT_IQ_BUFF_LEN/2);
			//
			// Calculate magnitude
			//
			arm_cmplx_mag_f32((float32_t *)(sd.FFT_Samples),(float32_t *)(sd.FFT_MagData),(FFT_IQ_BUFF_LEN/2));
			//
		sd.state++;
		break;
		}
		//
		//  Low-pass filter amplitude magnitude data
		//
		case 3:
		{
			uint32_t 	i;
			float32_t		filt_factor;
			//
			filt_factor = (float)ts.scope_filter;		// use stored filter setting
			filt_factor = 1/filt_factor;		// invert filter factor to allow multiplication
			//
			if(sd.dial_moved)	{	// Clear filter data if dial was moved in steps greater than 1 kHz
				sd.dial_moved = 0;	// Dial moved - reset indicator
				if(df.tuning_step > 1000)	{	// was tuning step greater than 1kHz?
					arm_copy_f32((float32_t *)sd.FFT_MagData, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// yes - copy current data into average buffer
				}
				//
				UiDrawSpectrumScopeFrequencyBarText();	// redraw frequency bar on the bottom of the display
				//
			}
			else	{	// dial was not moved - do normal IIR lowpass filtering to "smooth" display
				arm_scale_f32((float32_t *)sd.FFT_AVGData, (float32_t)filt_factor, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);	// get scaled version of previous data
				arm_sub_f32((float32_t *)sd.FFT_AVGData, (float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// subtract scaled information from old, average data
				arm_scale_f32((float32_t *)sd.FFT_MagData, (float32_t)filt_factor, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);	// get scaled version of new, input data
				arm_add_f32((float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_AVGData, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// add portion new, input data into average
				//
				for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{		//		// guarantee that the result will always be >= 0
					if(sd.FFT_AVGData[i] < 1)
						sd.FFT_AVGData[i] = 1;
				}
			}
		sd.state++;
		break;
		}
		//
		// De-linearize and normalize display data and do AGC processing
		//
		case 4:
		{
			q15_t	max1, max2, max3, min1, min2, min3;
			q15_t	mean1, mean2, mean3;
			float32_t	sig;
			//
			// De-linearize data with dB/division
			//
			for(i = 0; i < (FFT_IQ_BUFF_LEN/2); i++)	{
				sig = log10(sd.FFT_AVGData[i]) * sd.db_scale;		// take FFT data, do a log10 and multiply it to scale it to get desired dB/divistion
				sig += sd.display_offset;							// apply "AGC", vertical "sliding" offset (or brightness for waterfall)
				if(sig > 1)											// is the value greater than 1?
					sd.FFT_DspData[i] = (q15_t)sig;					// it was a useful value - save it
				else
					sd.FFT_DspData[i] = 1;							// not greater than 1 - assign it to a base value of 1 for sanity's sake
			}
			//
			arm_copy_q15((q15_t *)sd.FFT_DspData, (q15_t *)sd.FFT_TempData, FFT_IQ_BUFF_LEN/2);
			//
			// Find peak and average to vertically adjust display
			//
			if(sd.magnify)	{	// are we in magnify mode?  If so, find max/mean of only those portions of the spectrum magnified - which are NOT in the proper order, dammit!
				//
				if(!ts.iq_freq_mode)	{	// yes, are we NOT in translate mode?
					arm_max_q15((q15_t *)&sd.FFT_TempData[192], 64, &max1, &max_ptr);		// find maximum element in center portion
					arm_min_q15((q15_t *)&sd.FFT_TempData[192], 64, &min1, &max_ptr);		// find minimum element in center portion
					arm_mean_q15((q15_t *)&sd.FFT_TempData[192], 64, &mean1);				// find mean value in center portion
					//
					arm_max_q15((q15_t *)sd.FFT_TempData, 64, &max2, &max_ptr);		// find maximum element in center portion
					arm_min_q15((q15_t *)sd.FFT_TempData, 64, &min2, &max_ptr);		// find minimum element in center portion
					arm_mean_q15((q15_t *)sd.FFT_TempData, 64, &mean2);				// find mean value in center portion
					//
					if(max2 > max1)
						max1 = max2;
					//
					if(mean2 > mean1)
						mean1 = mean2;
					//
					if(min2 < min1)
						min1 = min2;
				}
				else if(ts.iq_freq_mode == 1)	{	// we are in RF LO HIGH mode (tuning is below center of screen)
					arm_max_q15((q15_t *)&sd.FFT_TempData[160], 64, &max1, &max_ptr);		// find maximum element in center portion
					arm_min_q15((q15_t *)&sd.FFT_TempData[160], 64, &min1, &max_ptr);		// find minimum element in center portion
					arm_mean_q15((q15_t *)&sd.FFT_TempData[160], 64, &mean1);				// find mean value in center portion
					//
					arm_max_q15((q15_t *)&sd.FFT_TempData[224], 32, &max2, &max_ptr);		// find maximum element in center portion
					arm_min_q15((q15_t *)&sd.FFT_TempData[224], 32, &min2, &max_ptr);		// find minimum element in center portion
					arm_mean_q15((q15_t *)&sd.FFT_TempData[224], 32, &mean2);				// find mean value in center portion
					//
					if(max2 > max1)
						max1 = max2;
					//
					if(min2 < min1)
						min1 = min2;
					//
					if(mean2 > mean1)
						mean1 = mean2;
					//
					arm_max_q15((q15_t *)&sd.FFT_TempData[0], 32, &max3, &max_ptr);		// find maximum element in center portion
					arm_min_q15((q15_t *)&sd.FFT_TempData[0], 32, &min3, &max_ptr);		// find minimum element in center portion
					arm_mean_q15((q15_t *)&sd.FFT_TempData[0], 32, &mean3);				// find mean value in center portion
					//
					if(max3 > max1)
						max1 = max3;
					//
					if(min3 < min1)
						min1 = min3;
					//
					if(mean3 > mean1)
						mean1 = mean3;
				}
				else if(ts.iq_freq_mode == 2)	{	// we are in RF LO LOW mode (tuning is above center of screen)
					arm_max_q15((q15_t *)&sd.FFT_TempData[224], 32, &max1, &max_ptr);		// find maximum element in center portion
					arm_min_q15((q15_t *)&sd.FFT_TempData[224], 32, &min1, &max_ptr);		// find minimum element in center portion
					arm_mean_q15((q15_t *)&sd.FFT_TempData[224], 32, &mean1);				// find mean value in center portion
					//
					arm_max_q15((q15_t *)&sd.FFT_TempData[0], 32, &max2, &max_ptr);		// find maximum element in center portion
					arm_min_q15((q15_t *)&sd.FFT_TempData[0], 32, &min2, &max_ptr);		// find minimum element in center portion
					arm_mean_q15((q15_t *)&sd.FFT_TempData[0], 32, &mean2);				// find mean value in center portion
					//
					if(max2 > max1)
						max1 = max2;
					//
					if(min2 < min1)
						min1 = min2;
					//
					if(mean2 > mean1)
						mean1 = mean2;
					//
					arm_max_q15((q15_t *)&sd.FFT_TempData[32], 64, &max3, &max_ptr);		// find maximum element in center portion
					arm_min_q15((q15_t *)&sd.FFT_TempData[32], 64, &min3, &max_ptr);		// find minimum element in center portion
					arm_mean_q15((q15_t *)&sd.FFT_TempData[32], 64, &mean3);				// find mean value in center portion
					//
					if(max3 > max1)
						max1 = max3;
					//
					if(min2 < min1)
						min1 = min2;
					//
					if(mean3 > mean1)
						mean1 = mean3;
				}
			}
			else	{
				spec_width = FFT_IQ_BUFF_LEN/2;
				arm_max_q15((q15_t *)sd.FFT_TempData, spec_width, &max1, &max_ptr);		// find maximum element
				arm_min_q15((q15_t *)sd.FFT_TempData, spec_width, &min1, &max_ptr);		// find minimum element
				arm_mean_q15((q15_t *)sd.FFT_TempData, spec_width, &mean1);				// find mean value
			}

			//
			// Vertically adjust spectrum scope so that the strongest signals are adjusted to the top
			//
			if(max1 > SPECTRUM_HEIGHT) {	// is result higher than display
				sd.display_offset -= sd.agc_rate;	// yes, adjust downwards quickly
//				if(max1 > SPECTRUM_HEIGHT+(SPECTRUM_HEIGHT/2))			// is it WAY above top of screen?
//					sd.display_offset -= sd.agc_rate*3;	// yes, adjust downwards REALLY quickly
			}
			//
			// Prevent "empty" spectrum display from filling with "noise" by checking the peak/average of what was found
			//
			else if(((max1*10/mean1) <= (q15_t)ts.spectrum_scope_nosig_adjust) && (max1 < SPECTRUM_HEIGHT+(SPECTRUM_HEIGHT/2)))	{	// was "average" signal ratio below set threshold and average is not insanely strong??
				if((min1 > 2) && (max1 > 2))	{		// prevent the adjustment from going downwards, "into the weeds"
					sd.display_offset -= sd.agc_rate;	// yes, adjust downwards
		            if(sd.display_offset < (-(SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET)))
		               sd.display_offset = (-(SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET));
				}
			}
			else
				sd.display_offset += (sd.agc_rate/3);	// no, adjust upwards more slowly
			//
			//
			if((min1 <= 2) && (max1 <= 2))	{	// ARGH - We must already be in the weeds, below the bottom - let's adjust upwards quickly to get it back onto the display!
				sd.display_offset += sd.agc_rate*10;
			}
			//
			// used for debugging
//				char txt[32];
//				sprintf(txt, " %d,%d,%d,%d ", (int)(max1*100/mean1), (int)(min1), (int)(max1),(int)mean1);
//				sprintf(txt, " %d,%d,%d,%d ", (int)sd.display_offset*100, (int)min1*100,(int)max1*100,(int)SPECTRUM_HEIGHT);
//				UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 20),txt,White,Grid,0);

			//
			//
			ushort ptr;
			//
			// Now, re-arrange the spectrum for the various magnify modes so that they display properly!
			//
			if(sd.magnify)	{	// is magnify mode on?
				if(!ts.iq_freq_mode)	{	// yes - frequency translate mode is off
					for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{	// expand data to fill entire screen - get lower half
						if(i > FFT_IQ_BUFF_LEN/4)	{
							ptr = (i/2)+128;
							if(ptr < FFT_IQ_BUFF_LEN/2)
								sd.FFT_DspData[i] = sd.FFT_TempData[ptr];
						}
						else	{
							ptr = (i/2);						// process upper half
							if(ptr < FFT_IQ_BUFF_LEN/2)
								sd.FFT_DspData[i] = sd.FFT_TempData[ptr];
						}
					}
				}
				else if(ts.iq_freq_mode == 1)	{	// frequency translate mode is in "RF LO HIGH" mode - tune below center of screen
					for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{	// expand data to fill entire screen - get lower half
						if(i > (FFT_IQ_BUFF_LEN/4))	{
							ptr = (i/2)+96;
							if(ptr < FFT_IQ_BUFF_LEN/2)	{
								sd.FFT_DspData[i] = sd.FFT_TempData[ptr];
							}
						}
						else	{
							ptr = (i/2);					// process upper half
							if(ptr < FFT_IQ_BUFF_LEN/2)	{
								sd.FFT_DspData[i] = sd.FFT_TempData[(ptr+224)%256];
							}
						}
					}
				}
				else if(ts.iq_freq_mode == 2)	{	// frequency translate mode is in "RF LO HIGH" mode - tune below center of screen
					for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{	// expand data to fill entire screen - get lower half
						if(i > (FFT_IQ_BUFF_LEN/4))	{
							ptr = (i/2);
							if(ptr < FFT_IQ_BUFF_LEN/2)	{
								sd.FFT_DspData[i] = sd.FFT_TempData[(ptr+160)%256];
							}
						}
						else	{
							ptr = (i/2) + 32;					// process upper half
							if(ptr < FFT_IQ_BUFF_LEN/2)	{
								sd.FFT_DspData[i] = sd.FFT_TempData[ptr];
							}
						}
					}
				}
			}
			else
				arm_copy_q15((q15_t *)sd.FFT_DspData, (q15_t *)sd.FFT_TempData, FFT_IQ_BUFF_LEN/2);
			//
			// After the above manipulation, clip the result to make sure that it is within the range of the palette table
			//
			for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{
				if(sd.FFT_DspData[i] >= SPECTRUM_HEIGHT)	// is there an illegal height value?
					sd.FFT_DspData[i] = SPECTRUM_HEIGHT - 1;	// yes - clip it

			}
			//
			//
		sd.state++;
		break;

		}
		//
		//  update LCD control
		//
		case 5:
		{
		ulong	clr;
        clr = UiDriverGetScopeTraceColour();
        // Left part of screen(mask and update in one operation to minimize flicker)
        UiLcdHy28_DrawSpectrum_Interleaved((q15_t *)(sd.FFT_BkpData + FFT_IQ_BUFF_LEN/4), (q15_t *)(sd.FFT_DspData + FFT_IQ_BUFF_LEN/4), Black, clr,0);
        // Right part of the screen (mask and update)
        UiLcdHy28_DrawSpectrum_Interleaved((q15_t *)(sd.FFT_BkpData), (q15_t *)(sd.FFT_DspData), Black, clr,1);
        sd.state = 0;   // Stage 0 - collection of data by the Audio driver
		break;
		}
		default:
			sd.state = 0;
			break;
	}
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverReDrawWaterfallDisplay
//* Object              : state machine implementation
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
// Waterfall Display code written by C. Turner, KA7OEI, May 2015 entirely from "scratch" - which is to say that I did not borrow any of it
// from anywhere else, aside from keeping some of the general functions found in "Case 1".
//
static void UiDriverReDrawWaterfallDisplay(void)
{
	ulong i, spec_width;
	uint32_t	max_ptr;	// throw-away pointer for ARM maxval AND minval functions
	float32_t	gcalc;
	//

	// Only in RX mode and NOT while powering down or in menu mode or if displaying memory information
	if((ts.txrx_mode != TRX_MODE_RX) || (ts.powering_down) || (ts.menu_mode) || (ts.mem_disp) || (ts.boot_halt_flag))
		return;

	if((ts.spectrum_scope_scheduler) || (!ts.waterfall_speed))	// is it time to update the scan, or is this scope to be disabled?
		return;
	else
		ts.spectrum_scope_scheduler = (ts.waterfall_speed-1)*2;


	// No spectrum display in DIGI modes
	//if(ts.dmod_mode == DEMOD_DIGI)
	//	return;

	// Nothing to do here otherwise, or if scope is to be held off while other parts of the display are to be updated or the LCD is being blanked
	if((!sd.enabled) || (ts.hold_off_spectrum_scope > ts.sysclock) || (ts.lcd_blanking_flag))
		return;

	// The state machine will rest
	// in between states
//	sd.skip_process++;
//	if(sd.skip_process < 1000)
//		return;

//	sd.skip_process = 0;

	gcalc = 1/ads.codec_gain_calc;				// Get gain setting of codec and convert to multiplier factor

	// Process implemented as state machine
	switch(sd.state)
	{
		//
		// Apply gain to collected IQ samples and then do FFT
		//
		case 1:		// Scale input according to A/D gain and apply Window function
		{
			arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)(gcalc * SCOPE_PREAMP_GAIN), (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN);	// scale input according to A/D gain
			//
			UiDriverFFTWindowFunction(ts.fft_window_type);		// do windowing function on input data to get less "Bin Leakage" on FFT data
			//
		sd.state++;
		break;
		}
		case 2:		// Do FFT and calculate complex magnitude
		{
			arm_rfft_f32((arm_rfft_instance_f32 *)&sd.S,(float32_t *)(sd.FFT_Windat),(float32_t *)(sd.FFT_Samples));	// Do FFT
			//
			// Calculate magnitude
			//
			arm_cmplx_mag_f32((float32_t *)(sd.FFT_Samples),(float32_t *)(sd.FFT_MagData),(FFT_IQ_BUFF_LEN/2));
			//
		sd.state++;
		break;
		}
		//
		//  Low-pass filter amplitude magnitude data
		//
		case 3:
		{
			uint32_t 	i;
			float32_t		filt_factor;
			//
			filt_factor = (float)ts.scope_filter;		// use stored filter setting
			filt_factor = 1/filt_factor;		// invert filter factor to allow multiplication
			//
			if(sd.dial_moved)	{	// Clear filter data if dial was moved in steps greater than 1 kHz
				sd.dial_moved = 0;	// Dial moved - reset indicator
				if(df.tuning_step > 1000)	{	// was tuning step greater than 1kHz
					arm_copy_f32((float32_t *)sd.FFT_MagData,(float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// copy current data into average buffer
				}
				//
				UiDrawSpectrumScopeFrequencyBarText();	// redraw frequency bar on the bottom of the display
				//
			}
			else	{	// dial was not moved - do IIR lowpass filtering to "smooth" display
				arm_scale_f32((float32_t *)sd.FFT_AVGData, (float32_t)filt_factor, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);	// get scaled version of previous data
				arm_sub_f32((float32_t *)sd.FFT_AVGData, (float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// subtract scaled information from old, average data
				arm_scale_f32((float32_t *)sd.FFT_MagData, (float32_t)filt_factor, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);	// get scaled version of new, input data
				arm_add_f32((float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_AVGData, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// add portion new, input data into average
				//
				for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{		//		// guarantee that the result will always be >= 0
					if(sd.FFT_AVGData[i] < 1)
						sd.FFT_AVGData[i] = 1;
				}
			}
		sd.state++;
		break;
		}
		//
		// De-linearize and normalize display data and do AGC processing
		//
		case 4:
		{
			float32_t	max, min, mean, offset;
			float32_t	sig;
			//
			// De-linearize data with dB/division
			//
			for(i = 0; i < (FFT_IQ_BUFF_LEN/2); i++)	{
				sig = log10(sd.FFT_AVGData[i]) * DB_SCALING_10;		// take FFT data, do a log10 and multiply it to scale 10dB (fixed)
				sig += sd.display_offset;							// apply "AGC", vertical "sliding" offset (or brightness for waterfall)
				if(sig > 1)											// is the value greater than 1?
					sd.FFT_DspData[i] = (q15_t)sig;					// it was a useful value - save it
				else
					sd.FFT_DspData[i] = 1;							// not greater than 1 - assign it to a base value of 1 for sanity's sake
			}
			//
			// Transfer data to the waterfall display circular buffer, putting the bins in frequency-sequential order!
			//
			for(i = 0; i < (FFT_IQ_BUFF_LEN/2); i++)	{
				if(i < (SPECTRUM_WIDTH/2))	{		// build left half of spectrum data
					sd.FFT_Samples[i] = sd.FFT_DspData[i + FFT_IQ_BUFF_LEN/4];	// get data
				}
				else	{							// build right half of spectrum data
					sd.FFT_Samples[i] = sd.FFT_DspData[i - FFT_IQ_BUFF_LEN/4];	// get data
				}
			}
			//
			// Find peak and average to vertically adjust display
			//
			if(sd.magnify)	{	// are we in magnify mode?
				spec_width = FFT_IQ_BUFF_LEN/4;	// yes - define new spectrum width
				//
				if(!ts.iq_freq_mode)	{	// yes, are we NOT in translate mode?
					arm_max_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8], spec_width, &max, &max_ptr);		// find maximum element in center portion
					arm_min_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8], spec_width, &min, &max_ptr);		// find minimum element in center portion
					arm_mean_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8], spec_width, &mean);				// find mean value in center portion
				}
				else if(ts.iq_freq_mode == 1)	{	// we are in RF LO HIGH mode (tuning is below center of screen)
					arm_max_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8 - 32], spec_width, &max, &max_ptr);		// find maximum element in center portion
					arm_min_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8 - 32], spec_width, &min, &max_ptr);		// find minimum element in center portion
					arm_mean_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8 - 32], spec_width, &mean);				// find mean value in center portion
				}
				else if(ts.iq_freq_mode == 2)	{	// we are in RF LO LOW mode (tuning is above center of screen)
					arm_max_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8 + 32], spec_width, &max, &max_ptr);		// find maximum element in center portion
					arm_min_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8 + 32], spec_width, &min, &max_ptr);		// find minimum element in center portion
					arm_mean_f32((float32_t *)&sd.FFT_Samples[FFT_IQ_BUFF_LEN/8 + 32], spec_width, &mean);				// find mean value in center portion
				}
			}
			else	{
				spec_width = FFT_IQ_BUFF_LEN/2;
				arm_max_f32((float32_t *)sd.FFT_Samples, spec_width, &max, &max_ptr);		// find maximum element
				arm_min_f32((float32_t *)sd.FFT_Samples, spec_width, &min, &max_ptr);		// find minimum element
				arm_mean_f32((float32_t *)sd.FFT_Samples, spec_width, &mean);				// find mean value
			}
			//
			// Calculate "brightness" offset for amplitude value
			//
			offset = (float)ts.waterfall_offset;
			offset -= 100;
			//
			//
			// Vertically adjust spectrum scope so that the strongest signals are adjusted to the top
			//
			if((max - offset) >= NUMBER_WATERFALL_COLOURS - 1)	{	// is result higher than display brightness
				sd.display_offset -= sd.agc_rate;	// yes, adjust downwards quickly
//				if(max1 > SPECTRUM_HEIGHT+(SPECTRUM_HEIGHT/2))			// is it WAY above top of screen?
//					sd.display_offset -= sd.agc_rate*3;	// yes, adjust downwards REALLY quickly
			}
			//
			// Prevent "empty" spectrum display from filling with "noise" by checking the peak/average of what was found
			//
			else if(((max*10/mean) <= (q15_t)ts.waterfall_nosig_adjust) && (max < SPECTRUM_HEIGHT+(SPECTRUM_HEIGHT/2)))	{	// was "average" signal ratio below set threshold and average is not insanely strong??
				if((min > 2) && (max > 2))	{		// prevent the adjustment from going downwards, "into the weeds"
					sd.display_offset -= sd.agc_rate;	// yes, adjust downwards
		            if(sd.display_offset < (-(SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET)))
		               sd.display_offset = (-(SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET));
				}
			}
			else
				sd.display_offset += (sd.agc_rate/3);	// no, adjust upwards more slowly
			//
			//
			if((min <= 2) && (max <= 2))	{	// ARGH - We must already be in the weeds, below the bottom - let's adjust upwards quickly to get it back onto the display!
				sd.display_offset += sd.agc_rate*10;
			}
			//
			// used for debugging
			//	char txt[32];
			//	sprintf(txt, " %d,%d,%d ", (int)sd.display_offset*100, (int)min*100,(int)max*100);
			//	UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 20),txt,White,Grid,0);
			//
			// Copy to holder for the waterfall buffer
			sd.state++;
			break;
		}
		case 5:	// rescale waterfall horizontally, apply brightness/contrast, process pallate and put vertical line on screen, if enabled.
		{
			//
			sd.wfall_line %= sd.wfall_size;	// make sure that the circular buffer is clipped to the size of the display area
			//
			//
			// Contrast:  100 = 1.00 multiply factor:  125 = multiply by 1.25 - "sd.wfall_contrast" already converted to 100=1.00
			//
			arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)sd.wfall_contrast, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);
			//
			ushort ptr;
			//
			if(sd.magnify)	{	// is magnify mode on?
				if(!ts.iq_freq_mode)	{	// yes - frequency translate mode is off
					for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{	// expand data to fill entire screen - get lower half
						ptr = (i/2)+64;
						if(ptr < FFT_IQ_BUFF_LEN/2)	{
							sd.wfall_temp[i] = sd.FFT_Samples[ptr];
						}
					}
				}
				else if(ts.iq_freq_mode == 1)	{	// frequency translate mode is in "RF LO HIGH" mode - tune below center of screen
					for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{	// expand data to fill entire screen - get lower half
						ptr = (i/2)+32;
						if(ptr < FFT_IQ_BUFF_LEN/2)	{
							sd.wfall_temp[i] = sd.FFT_Samples[ptr];
						}
					}
				}
				else if(ts.iq_freq_mode == 2)	{	// frequency translate mode is in "RF LO LOW" mode - tune below center of screen
					for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{	// expand data to fill entire screen - get lower half
						ptr = (i/2)+96;
						if(ptr < FFT_IQ_BUFF_LEN/2)	{
							sd.wfall_temp[i] = sd.FFT_Samples[ptr];
						}
					}
				}
				arm_copy_f32((float32_t *)sd.wfall_temp, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);		// copy the rescaled/shifted data into the main buffer
			}
			//
			// After the above manipulation, clip the result to make sure that it is within the range of the palette table
			//
			for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{
				if(sd.FFT_Samples[i] >= NUMBER_WATERFALL_COLOURS)	// is there an illegal color value?
					sd.FFT_Samples[i] = NUMBER_WATERFALL_COLOURS - 1;	// yes - clip it
				//
				sd.waterfall[sd.wfall_line][i] = (ushort)sd.FFT_Samples[i];	// save the manipulated value in the circular waterfall buffer
			}
			//
			// Place center line marker on screen:  Location [64] (the 65th) of the palette is reserved is a special color reserved for this
			//
			if(sd.magnify)	{
				sd.waterfall_colours[NUMBER_WATERFALL_COLOURS] = (ushort)ts.scope_centre_grid_colour_active;	// for some reason it is necessary to reload this entry of the palette!
				sd.waterfall[sd.wfall_line][128] = NUMBER_WATERFALL_COLOURS;	// set graticule in the middle
			}
			else if(!ts.iq_freq_mode)	// is frequency translate off OR magnification mode on
				sd.waterfall[sd.wfall_line][128] = NUMBER_WATERFALL_COLOURS;	// set graticule in the middle
			else if(ts.iq_freq_mode == 1)			// LO HIGH - set graticule below center
				sd.waterfall[sd.wfall_line][96] = NUMBER_WATERFALL_COLOURS;
			else if(ts.iq_freq_mode == 2)			// LO LOW - set graticule above center
				sd.waterfall[sd.wfall_line][160] = NUMBER_WATERFALL_COLOURS;

			//
			sd.wfall_line++;		// bump to the next line in the circular buffer for next go-around
			//
			// scan_top is used to limit AGC action to "magnified" portion
			//
		sd.state++;
		break;
		}
		//
		//  update LCD control
		//
		case 6:
		{
			uchar lptr = sd.wfall_line;		// get current line of "bottom" of waterfall in circular buffer
			uchar lcnt = 0;					// initialize count of number of lines of display

			//
			sd.wfall_line_update++;									// update waterfall line count
			sd.wfall_line_update %= ts.waterfall_vert_step_size;	// clip it to number of lines per iteration

			if(!sd.wfall_line_update)	{							// if it's count is zero, it's time to move the waterfall up
				//
				lptr %= sd.wfall_size;		// do modulus limit of spectrum high
				//
				// set up LCD for bulk write, limited only to area of screen with waterfall display.  This allow data to start from the
				// bottom-left corner and advance to the right and up to the next line automatically without ever needing to address
				// the location of any of the display data - as long as we "blindly" write precisely the correct number of pixels per
				// line and the number of lines.
				//
				UiLcdHy28_OpenBulkWrite(SPECTRUM_START_X, SPECTRUM_WIDTH, (sd.wfall_ystart + 1), sd.wfall_height);
				//
				while(lcnt < sd.wfall_size)	{				// set up counter for number of lines defining height of waterfall
					for(i = 0; i < (SPECTRUM_WIDTH); i+=4)	{	// scan to copy one line of spectral data - "unroll" to optimize for ARM processor
						UiLcdHy28_BulkWrite(sd.waterfall_colours[sd.waterfall[lptr][i]]);	// write to memory using waterfall color from palette
						UiLcdHy28_BulkWrite(sd.waterfall_colours[sd.waterfall[lptr][i+1]]);	// write to memory using waterfall color from palette
						UiLcdHy28_BulkWrite(sd.waterfall_colours[sd.waterfall[lptr][i+2]]);	// write to memory using waterfall color from palette
						UiLcdHy28_BulkWrite(sd.waterfall_colours[sd.waterfall[lptr][i+3]]);	// write to memory using waterfall color from palette
					}
					lcnt++;									// update count of lines we have done
					lptr++;									// point to next line in circular display buffer
					lptr %= sd.wfall_size;				// clip to display height
				}
				//
				UiLcdHy28_CloseBulkWrite();					// we are done updating the display - return to normal full-screen mode
				}
				sd.state = 0;	// Stage 0 - collection of data by the Audio driver
				break;
		}
		default:
			sd.state = 0;
			break;
	}
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverGetScopeTraceColour
//* Object              : Gets setting from trace color variable
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
static ulong UiDriverGetScopeTraceColour(void)
{
	if(ts.scope_trace_colour == SPEC_GREY)
		return Grey;
	else if(ts.scope_trace_colour == SPEC_BLUE)
		return Blue;
	else if(ts.scope_trace_colour == SPEC_RED)
		return Red;
	else if(ts.scope_trace_colour == SPEC_MAGENTA)
			return Magenta;
	else if(ts.scope_trace_colour == SPEC_GREEN)
		return Green;
	else if(ts.scope_trace_colour == SPEC_CYAN)
		return Cyan;
	else if(ts.scope_trace_colour == SPEC_YELLOW)
		return Yellow;
	else if(ts.scope_trace_colour == SPEC_ORANGE)
		return Orange;
	else
		return White;
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiInitSpectrumScopeWaterfall
//* Object              : Does all steps for clearing screen and initializing spectrum scope and waterfall
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
static void UiInitSpectrumScopeWaterfall(void)
{
	if(ts.boot_halt_flag)			// do not build spectrum display/waterfall if we are loading EEPROM defaults!
		return;

	UiDriverClearSpectrumDisplay();			// clear display under spectrum scope
	UiDriverCreateSpectrumScope();
	UiDriverInitSpectrumDisplay();
	UiDriverDisplayFilterBW();	// Update on-screen indicator of filter bandwidth
}



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
	while ((sm.gain_calc >= S_Meter_Cal[sm.s_count]) && (sm.s_count <= S_Meter_Cal_Size))	{	// find corresponding signal level
		sm.s_count++;;
	}
	val = (uchar)sm.s_count;
	if(!val)	// make sure that the S meter always reads something!
		val = 1;
	//
	UiDriverUpdateTopMeterA(val,sm.old);
	sm.old = val;
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
			UiDriverDrawRedSMeter();		// No, make the first portion of the S-meter red
			clip_indicate = 1;		// set flag indicating that we saw clipping and changed the screen (prevent continuous redraw)
		}
		ads.adc_clip = 0;		// reset clip detect flag
	}
	else	{		// clipping NOT occur?
		if(clip_indicate)	{	// had clipping occurred since we last visited this code?
			UiDriverDrawWhiteSMeter();					// yes - restore the S meter to a white condition
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
		case	FILTER_BAND_80:
			coupling_calc = (float)swrm.coupling_80m_calc;	// get coupling coefficient calibration for 80 meters
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
	if(scale_calc > 34)		// limit maximum reading
		scale_calc = 34;
	UiDriverUpdateTopMeterA(scale_calc, 33);

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
			if(scale_calc > 34)					// limit maximum scale
				scale_calc = 34;
			UiDriverUpdateBtmMeter((uchar)(scale_calc), 13);	// update the meter, setting the "red" threshold
		}
	}
	else if(ts.tx_meter_mode == METER_ALC)	{
		scale_calc = ads.alc_val;		// get TX ALC value
		scale_calc *= scale_calc;		// square the value
		scale_calc = log10f(scale_calc);	// get the log10
		scale_calc *= -10;		// convert it to DeciBels and switch sign and then scale it for the meter
		if(scale_calc > 34)		// limit range of values being sent to the meter
			scale_calc = 34;
		else if(scale_calc < 0)
			scale_calc = 0;
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
		if(scale_calc > 34)					// limit range of values being sent to the meter
			scale_calc = 34;
		else if(scale_calc < 0)
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
	uchar	v10,v100,v1000,v10000;
	//char 	cap1[50];
	char	digit[2];
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

//	debug
//	sprintf(txt, "%d ", (int)(val_p));
//	UiLcdHy28_PrintText    (POS_PWR_NUM_IND_X, POS_PWR_NUM_IND_Y,txt,Grey,Black,0);

	// Terminate
	digit[1] = 0;

	v10000 = (val_p%100000)/10000;
	v1000 = (val_p%10000)/1000;
	v100 = (val_p%1000)/100;
	v10 = (val_p%100)/10;
	//
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
	if((v10000 != pwmt.v10000) || (v1000 != pwmt.v1000) || (v100 != pwmt.v100) || (v10 != pwmt.v10))	{	// Time to update - or was this the first time it was called?


		// -----------------------
		// 10V update

		//printf("10V diff: %d\n\r",v10000);

		// To string
		digit[0] = 0x30 + (v10000 & 0x0F);

		// Update segment
		if(v10000)
			UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*0),POS_PWR_IND_Y,digit,col,Black,0);
		else
			UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*0),POS_PWR_IND_Y,digit,Black,Black,0);

		// Save value
		pwmt.v10000 = v10000;


		// -----------------------
		// 1V update

		//printf("1V diff: %d\n\r",v1000);

		// To string
		digit[0] = 0x30 + (v1000 & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*1),POS_PWR_IND_Y,digit,col,Black,0);

		// Save value
		pwmt.v1000 = v1000;

		// -----------------------
		// 0.1V update

		//printf("0.1V diff: %d\n\r",v100);

		// To string
		digit[0] = 0x30 + (v100 & 0x0F);

		// Update segment(skip the dot)
		UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*3),POS_PWR_IND_Y,digit,col,Black,0);

		// Save value
		pwmt.v100 = v100;

		// -----------------------
		// 0.01Vupdate

		//printf("0.01V diff: %d\n\r",v10);

		// To string
		digit[0] = 0x30 + (v10 & 0x0F);

		// Update segment(skip the dot)
		UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*4),POS_PWR_IND_Y,digit,col,Black,0);

		// Save value
		pwmt.v10 = v10;
	}


	// Reset accumulator
	pwmt.p_curr 	= 0;
	pwmt.pwr_aver 	= 0;
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
	uchar 	i,v_s;
	int		col = White;

	// Do not waste time redrawing if outside of the range
	if(val > 26)
		return;

	// Indicator height
	v_s = 3;

	// Draw first indicator
	for(i = 1; i < 26; i++)
	{
		if(val == i)
			col = Blue;
		else
			col = White;

		if(!active)
			col = Grey;

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_TEMP_IND_X + 1) + i*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_TEMP_IND_X + 2) + i*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_TEMP_IND_X + 3) + i*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,col);
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
	if(create)
	{
		// Top part - name and temperature display
		UiLcdHy28_DrawEmptyRect( POS_TEMP_IND_X,POS_TEMP_IND_Y,14,109,Grey);

		// LO tracking indicator
		UiLcdHy28_DrawEmptyRect( POS_TEMP_IND_X,POS_TEMP_IND_Y + 14,10,109,Grey);
		// Temperature - initial draw
		if(df.temp_enabled & 0xf0)
			UiLcdHy28_PrintText((POS_TEMP_IND_X + 50),(POS_TEMP_IND_Y + 1), "  77.0F",Grey,Black,0);
		else
			UiLcdHy28_PrintText((POS_TEMP_IND_X + 50),(POS_TEMP_IND_Y + 1), "  25.0C",Grey,Black,0);
	}

	if(enabled)
	{
		// Control name
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 1), (POS_TEMP_IND_Y + 1),"TCXO ",Black,Grey,0);

		// Lock indicator
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",Red,Black,0);
	}
	else
	{
		// Control name
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 1), (POS_TEMP_IND_Y + 1),"TCXO ",Grey1,Grey,0);

		// Lock indicator
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",Grey,Black,0);
	}
	//
	if((df.temp_enabled & 0x0f) == TCXO_STOP)	{	// if temperature update is disabled, don't update display!
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1), " ",Grey,Black,0);	// delete asterisc
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50),(POS_TEMP_IND_Y + 1), "STOPPED",Grey,Black,0);
	}
	//
	// Meter
	UiDriverUpdateLoMeter(13,enabled);
}

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
	if(comp != (-1))
	{
		df.temp_factor = comp;
		UiDriverUpdateFrequencyFast();

		// Save to public, to skip not needed update
		// when we are in 1C range
		lo.comp = comp;

		// Lock indicator
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",Blue,Black,0);
	}
	else
	{
		// No compensation - commented out - keep last compensation value
		//df.temp_factor = 0;
		//UiDriverUpdateFrequencyFast();

		// Lock indicator
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",Red,Black,0);
	}
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

	// Display clear
//	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) + 140),(POS_SM_IND_Y + 59),"PROT",Black,Black,4);

	//
	if(ts.power_level == PA_LEVEL_FULL)	{
		switch(band)	{		// get pre-loaded power output scaling factor for band
			case BAND_MODE_80:
				pf_temp = (float)ts.pwr_80m_full_adj;		// load full power level for 80m
				break;
			case BAND_MODE_60:
				pf_temp = (float)ts.pwr_60m_full_adj;		// load full power level for 60m
				break;
			case BAND_MODE_40:
				pf_temp = (float)ts.pwr_40m_full_adj;		// load full power level for 40m
				break;
			case BAND_MODE_30:
				pf_temp = (float)ts.pwr_30m_full_adj;		// load full power level for 30m
				break;
			case BAND_MODE_20:
				pf_temp = (float)ts.pwr_20m_full_adj;		// load full power level for 20m
				break;
			case BAND_MODE_17:
				pf_temp = (float)ts.pwr_17m_full_adj;		// load full power level for 17m
				break;
			case BAND_MODE_15:
				pf_temp = (float)ts.pwr_15m_full_adj;		// load full power level for 15m
				break;
			case BAND_MODE_12:
				pf_temp = (float)ts.pwr_12m_full_adj;		// load full power level for 12m
				break;
			case BAND_MODE_10:
				pf_temp = (float)ts.pwr_10m_full_adj;		// load full power level for 10m
				break;
			default:
				pf_temp = 50;
				break;
		}
	}
	else	{					// OTHER than FULL power!
		switch(band)	{		// get pre-loaded power output scaling factor for band
			case BAND_MODE_80:
				pf_temp = (float)ts.pwr_80m_5w_adj;		// load 5 watt power level for 80m
				break;
			case BAND_MODE_60:
				pf_temp = (float)ts.pwr_60m_5w_adj;		// load 5 watt power level for 60m
				break;
			case BAND_MODE_40:
				pf_temp = (float)ts.pwr_40m_5w_adj;		// load 5 watt power level for 40m
				break;
			case BAND_MODE_30:
				pf_temp = (float)ts.pwr_30m_5w_adj;		// load 5 watt power level for 30m
				break;
			case BAND_MODE_20:
				pf_temp = (float)ts.pwr_20m_5w_adj;		// load 5 watt power level for 20m
				break;
			case BAND_MODE_17:
				pf_temp = (float)ts.pwr_17m_5w_adj;		// load 5 watt power level for 17m
				break;
			case BAND_MODE_15:
				pf_temp = (float)ts.pwr_15m_5w_adj;		// load 5 watt power level for 15m
				break;
			case BAND_MODE_12:
				pf_temp = (float)ts.pwr_12m_5w_adj;		// load 5 watt power level for 12m
				break;
			case BAND_MODE_10:
				pf_temp = (float)ts.pwr_10m_5w_adj;		// load 5 watt power level for 10m
				break;
			default:
				pf_temp = 50;
				break;
		}
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
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcAGCDecay
//* Object              : Calculate Decay timing for AGC (RECEIVE!)
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiCalcAGCDecay(void)
{
	float tcalc;	// temporary holder - used to avoid conflict during operation

	// Set AGC rate - this needs to be moved to its own function (and the one in "ui_menu.c")
	//
	if(ts.agc_mode == AGC_SLOW)
		ads.agc_decay = AGC_SLOW_DECAY;
	else if(ts.agc_mode == AGC_FAST)
		ads.agc_decay = AGC_FAST_DECAY;
	else if(ts.agc_mode == AGC_CUSTOM)	{	// calculate custom AGC setting
		tcalc = (float)ts.agc_custom_decay;
		tcalc += 30;
		tcalc /= 10;
		tcalc = -tcalc;
		ads.agc_decay = powf(10, tcalc);
	}
	else
		ads.agc_decay = AGC_MED_DECAY;
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcALCDecay
//* Object              : Calculate Decay timing for ALC (TRANSMIT!)
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiCalcALCDecay(void)
{
	float tcalc;	// temporary holder - used to avoid conflict during operation

	// calculate ALC decay (release) time constant - this needs to be moved to its own function (and the one in "ui_menu.c")
	//
	tcalc = (float)ts.alc_decay_var;
	tcalc += 35;
	tcalc /= 10;
	tcalc *= -1;
	ads.alc_decay = powf(10, tcalc);
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcRFGain
//* Object              : Calculate RF Gain internal value from user setting
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiCalcRFGain(void)
{
	float tcalc;	// temporary value as "ads.agc_rf_gain" may be used during the calculation!

	// calculate working RF gain value
	tcalc = (float)ts.rf_gain;
	tcalc *= 1.4;
	tcalc -= 20;
	tcalc /= 10;
	ads.agc_rf_gain = powf(10, tcalc);

}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcAGCVals
//* Object              : Calculate internal AGC values from user settings
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiCalcAGCVals(void)
{
	if(ts.max_rf_gain <= MAX_RF_GAIN_MAX)	{
		ads.agc_knee = AGC_KNEE_REF * (float)(ts.max_rf_gain + 1);
		ads.agc_val_max = AGC_VAL_MAX_REF / ((float)(ts.max_rf_gain + 1));
		ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF / (float)(ts.max_rf_gain + 1);
	}
	else	{
		ads.agc_knee = AGC_KNEE_REF * MAX_RF_GAIN_DEFAULT+1;
		ads.agc_val_max = AGC_VAL_MAX_REF / MAX_RF_GAIN_DEFAULT+1;
		ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF /  (float)(ts.max_rf_gain + 1);
	}
}
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
	switch(ts.cw_offset_mode)	{
		case CW_OFFSET_USB_TX:
		case CW_OFFSET_USB_RX:
		case CW_OFFSET_USB_SHIFT:
			ts.cw_lsb = 0;				// Not LSB!
			break;
		case CW_OFFSET_LSB_TX:
		case CW_OFFSET_LSB_RX:
		case CW_OFFSET_LSB_SHIFT:
			ts.cw_lsb = 1;				// It is LSB
			break;
		case CW_OFFSET_AUTO_TX:						// For "auto" modes determine if we are above or below threshold frequency
		case CW_OFFSET_AUTO_RX:
		case CW_OFFSET_AUTO_SHIFT:
			if(df.tune_new >= USB_FREQ_THRESHOLD)	// is the current frequency above the USB threshold?
				ts.cw_lsb = 0;						// yes - indicate that it is USB
			else
				ts.cw_lsb = 1;						// no - LSB
			break;
		default:
			ts.cw_lsb = 0;
			break;
	}
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcNB_AGC
//* Object              : Calculate Noise Blanker AGC settings
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiCalcNB_AGC(void)
{
	float temp_float;

	temp_float = (float)ts.nb_agc_time_const;	// get user setting
	temp_float = NB_MAX_AGC_SETTING-temp_float;		// invert (0 = minimum))
	temp_float /= 1.1;								// scale calculation
	temp_float *= temp_float;						// square value
	temp_float += 1;								// offset by one
	temp_float /= 44000;							// rescale
	temp_float += 1;								// prevent negative log result
	ads.nb_sig_filt = log10f(temp_float);			// de-linearize and save in "new signal" contribution parameter
	ads.nb_agc_filt = 1 - ads.nb_sig_filt;			// calculate parameter for recyling "old" AGC value
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcRxIqGainAdj
//* Object              : Calculate RX IQ Gain adjustments
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiCalcRxIqGainAdj(void)
{
	if(ts.dmod_mode == DEMOD_AM)
		ts.rx_adj_gain_var_i = (float)ts.rx_iq_am_gain_balance;			// get current gain adjustment for AM
	if(ts.dmod_mode == DEMOD_FM)
		ts.rx_adj_gain_var_i = (float)ts.rx_iq_fm_gain_balance;			// get current gain adjustment for FM
	else if(ts.dmod_mode == DEMOD_LSB)
		ts.rx_adj_gain_var_i = (float)ts.rx_iq_lsb_gain_balance;		// get current gain adjustment setting for LSB
	else
		ts.rx_adj_gain_var_i = (float)ts.rx_iq_usb_gain_balance;		// get current gain adjustment setting	USB and other modes
	//
	ts.rx_adj_gain_var_i /= 1024;		// fractionalize it
	ts.rx_adj_gain_var_q = -ts.rx_adj_gain_var_i;				// get "invert" of it
	ts.rx_adj_gain_var_i += 1;		// offset it by one (e.g. 0 = unity)
	ts.rx_adj_gain_var_q += 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiCalcTxIqGainAdj
//* Object              : Calculate TX IQ Gain adjustments
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiCalcTxIqGainAdj(void)
{
	// Note:  There is a fixed amount of offset due to the fact that the phase-added Hilbert (e.g. 0, 90) transforms are
	// slightly asymmetric that is added so that "zero" is closer to being the proper phase balance.
	//
	if(ts.dmod_mode == DEMOD_AM)	// is it AM mode?
		ts.tx_adj_gain_var_i = (float)ts.tx_iq_am_gain_balance;		// get current gain balance adjustment setting for AM
	else if(ts.dmod_mode == DEMOD_FM)	// is it in FM mode?
		ts.tx_adj_gain_var_i = (float)ts.tx_iq_fm_gain_balance;		// get current gain balance adjustment setting for FM
	else if(ts.dmod_mode == DEMOD_LSB)
		ts.tx_adj_gain_var_i = (float)ts.tx_iq_lsb_gain_balance;		// get current gain balance adjustment setting for LSB
	else
		ts.tx_adj_gain_var_i = (float)ts.tx_iq_usb_gain_balance;		// get current gain adjustment setting for USB and other non AM/FM modes

	//
	ts.tx_adj_gain_var_i /= 1024;		// fractionalize it
	ts.tx_adj_gain_var_q = -ts.tx_adj_gain_var_i;				// get "invert" of it
	ts.tx_adj_gain_var_i += 1;		// offset it by one (e.g. 0 = unity)
	ts.tx_adj_gain_var_q += 1;
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcRxPhaseAdj
//* Object              : Calculate RX FFT coeffients based on adjustment settings
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiCalcRxPhaseAdj(void)
{
	float f_coeff, f_offset, var_norm, var_inv;
	ulong i;
	int phase;
	//
	// always make a fresh copy of the original Q and I coefficients
	// NOTE:  We are assuming that the I and Q filters are of the same length!
	//
	fc.rx_q_num_taps = Q_NUM_TAPS;
	fc.rx_i_num_taps = I_NUM_TAPS;
	//
	fc.rx_q_block_size = Q_BLOCK_SIZE;
	fc.rx_i_block_size = I_BLOCK_SIZE;
	//
	if(ts.dmod_mode == DEMOD_AM)	{		// AM - load low-pass, non Hilbert filters (e.g. no I/Q phase shift
		if(ts.filter_id == AUDIO_WIDE)	{	// Wide AM - selectable from menu
			for(i = 0; i < Q_NUM_TAPS; i++)	{
				switch(ts.filter_wide_select)	{
					case WIDE_FILTER_5K:
					case WIDE_FILTER_5K_AM:
						fc.rx_filt_q[i] = iq_rx_am_5k_coeffs[i];
						fc.rx_filt_i[i] = iq_rx_am_5k_coeffs[i];
						break;
					case WIDE_FILTER_6K:
					case WIDE_FILTER_6K_AM:
						fc.rx_filt_q[i] = iq_rx_am_6k_coeffs[i];
						fc.rx_filt_i[i] = iq_rx_am_6k_coeffs[i];
						break;
					case WIDE_FILTER_7K5:
					case WIDE_FILTER_7K5_AM:
						fc.rx_filt_q[i] = iq_rx_am_7k5_coeffs[i];
						fc.rx_filt_i[i] = iq_rx_am_7k5_coeffs[i];
						break;
					case WIDE_FILTER_10K:
					case WIDE_FILTER_10K_AM:
					default:
						fc.rx_filt_q[i] = iq_rx_am_10k_coeffs[i];
						fc.rx_filt_i[i] = iq_rx_am_10k_coeffs[i];
						break;
				}
			}
		}
		else if((ts.filter_id == AUDIO_3P6KHZ) || (ts.dmod_mode == DEMOD_FM))	{	// "Medium" AM - "3.6" kHz filter (total of 7.2 kHz bandwidth) - or if we are using FM
			for(i = 0; i < Q_NUM_TAPS; i++)	{
				fc.rx_filt_q[i] = iq_rx_am_3k6_coeffs[i];
				fc.rx_filt_i[i] = iq_rx_am_3k6_coeffs[i];
			}
		}
		else	{
			for(i = 0; i < Q_NUM_TAPS; i++)	{		// "Narrow" AM - "1.8" kHz filter (total of 3.6 kHz bandwidth)
				fc.rx_filt_q[i] = iq_rx_am_2k3_coeffs[i];
				fc.rx_filt_i[i] = iq_rx_am_2k3_coeffs[i];
			}
		}
	}
	else if(ts.dmod_mode == DEMOD_FM)	{		// FM - load low-pass, non Hilbert filters (e.g. no I/Q phase shift
		for(i = 0; i < Q_NUM_TAPS; i++)	{
			switch(ts.fm_rx_bandwidth)	{
				case FM_RX_BANDWIDTH_10K:
					fc.rx_filt_q[i] = iq_rx_am_5k_coeffs[i];	// load 5 kHz FIR (2 x 5kHz = 10 kHz)
					fc.rx_filt_i[i] = iq_rx_am_5k_coeffs[i];
					break;
				case FM_RX_BANDWIDTH_12K:
					fc.rx_filt_q[i] = iq_rx_am_6k_coeffs[i];	// load 6 kHz FIR (2 x 6 kHz = 12 kHz)
					fc.rx_filt_i[i] = iq_rx_am_6k_coeffs[i];
					break;
//				case FM_RX_BANDWIDTH_15K:
//					fc.rx_filt_q[i] = iq_rx_am_7k5_coeffs[i];	// load 7.5kHz FIR (2 x 7.5 kHz = 15 kHz)
//					fc.rx_filt_i[i] = iq_rx_am_7k5_coeffs[i];
//					break;
				case FM_RX_BANDWIDTH_7K2:
				default:
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						fc.rx_filt_q[i] = iq_rx_am_3k6_coeffs[i];	// load 3.6 kHz FIR (2 x 3.6 kHz = 7.2 kHz)
						fc.rx_filt_i[i] = iq_rx_am_3k6_coeffs[i];
					}
					break;
			}
		}
	}
	else	{		// Not AM or FM - load Hilbert transformation filters
		if(ts.filter_id == AUDIO_WIDE)	{
			for(i = 0; i < Q_NUM_TAPS; i++)	{
				switch(ts.filter_wide_select)	{
					case WIDE_FILTER_5K:
					case WIDE_FILTER_5K_AM:
						fc.rx_filt_q[i] = q_rx_5k_coeffs[i];
						fc.rx_filt_i[i] = i_rx_5k_coeffs[i];
						break;
					case WIDE_FILTER_6K:
					case WIDE_FILTER_6K_AM:
						fc.rx_filt_q[i] = q_rx_6k_coeffs[i];
						fc.rx_filt_i[i] = i_rx_6k_coeffs[i];
						break;
					case WIDE_FILTER_7K5:
					case WIDE_FILTER_7K5_AM:
						fc.rx_filt_q[i] = q_rx_7k5_coeffs[i];
						fc.rx_filt_i[i] = i_rx_7k5_coeffs[i];
						break;
					case WIDE_FILTER_10K:
					case WIDE_FILTER_10K_AM:
					default:
						fc.rx_filt_q[i] = q_rx_10k_coeffs[i];
						fc.rx_filt_i[i] = i_rx_10k_coeffs[i];
						break;
				}

			}
		}
		else	{
			for(i = 0; i < Q_NUM_TAPS; i++)	{
				fc.rx_filt_q[i] = q_rx_3k6_coeffs[i];
				fc.rx_filt_i[i] = i_rx_3k6_coeffs[i];	// phase shift in other modes
			}
		}
		//
		if(ts.dmod_mode == DEMOD_LSB)	// get phase setting appropriate to mode
			phase = ts.rx_iq_lsb_phase_balance;		// yes, get current gain adjustment setting for LSB
		else
			phase = ts.rx_iq_usb_phase_balance;		// yes, get current gain adjustment setting for USB and other mdoes
		//
		if(phase != 0)	{	// is phase adjustment non-zero?
			var_norm = (float)phase;
			var_norm = fabs(var_norm);		// get absolute value of this gain adjustment
			var_inv = 32 - var_norm;		// calculate "inverse" of number of steps
			var_norm /= 32;		// fractionalize by the number of steps
			var_inv /= 32;						// fractionalize this one, too
			if(phase < 0)	{	// was the phase adjustment negative?
				if(ts.filter_id == AUDIO_WIDE)	{
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						switch(ts.filter_wide_select)	{
							case WIDE_FILTER_5K:
							case WIDE_FILTER_5K_AM:
								f_coeff = var_inv * q_rx_5k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_5k_coeffs_minus[i];	// get fraction of 89.5 degree setting
								break;
							case WIDE_FILTER_6K:
							case WIDE_FILTER_6K_AM:
								f_coeff = var_inv * q_rx_6k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_6k_coeffs_minus[i];	// get fraction of 89.5 degree setting
								break;
							case WIDE_FILTER_7K5:
							case WIDE_FILTER_7K5_AM:
								f_coeff = var_inv * q_rx_7k5_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_7k5_coeffs_minus[i];	// get fraction of 89.5 degree setting
								break;
							case WIDE_FILTER_10K:
							case WIDE_FILTER_10K_AM:
							default:
								f_coeff = var_inv * q_rx_10k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_10k_coeffs_minus[i];	// get fraction of 89.5 degree setting
								break;
						}
						fc.rx_filt_q[i] = f_coeff + f_offset;	// synthesize new coefficient
					}
				}
				else	{
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						f_coeff = var_inv * q_rx_3k6_coeffs[i];	// get fraction of 90 degree setting
						f_offset = var_norm * q_rx_3k6_coeffs_minus[i];	// get fraction of 89.5 degree setting
						fc.rx_filt_q[i] = f_coeff + f_offset;	// synthesize new coefficient
					}
				}
			}
			else	{							// adjustment was positive
				if(ts.filter_id == AUDIO_WIDE)	{
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						switch(ts.filter_wide_select)	{
							case WIDE_FILTER_5K:
							case WIDE_FILTER_5K_AM:
								f_coeff = var_inv * q_rx_5k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_5k_coeffs_plus[i];	// get fraction of 90.5 degree setting
								break;
							case WIDE_FILTER_6K:
							case WIDE_FILTER_6K_AM:
								f_coeff = var_inv * q_rx_6k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_6k_coeffs_plus[i];	// get fraction of 90.5 degree setting
								break;
							case WIDE_FILTER_7K5:
							case WIDE_FILTER_7K5_AM:
								f_coeff = var_inv * q_rx_7k5_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_7k5_coeffs_plus[i];	// get fraction of 90.5 degree setting
								break;
							case WIDE_FILTER_10K:
							case WIDE_FILTER_10K_AM:
							default:
								f_coeff = var_inv * q_rx_10k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_10k_coeffs_plus[i];	// get fraction of 90.5 degree setting
								break;
						}
						fc.rx_filt_q[i] = f_coeff + f_offset;	// synthesize new coefficient
					}
				}
				else	{
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						f_coeff = var_inv * q_rx_3k6_coeffs[i];	// get fraction of 90 degree setting
						f_offset = var_norm * q_rx_3k6_coeffs_plus[i];	// get fraction of 90.5 degree setting
						fc.rx_filt_q[i] = f_coeff + f_offset;	// synthesize new coefficient
					}
				}
			}
		}
	}
	//
	// In AM mode we do NOT do 90 degree phase shift, so we do FIR low-pass instead of Hilbert, setting "I" channel the same as "Q"
	if(ts.dmod_mode == DEMOD_AM)		// use "Q" filter settings in AM mode for "I" channel
		arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I,fc.rx_q_num_taps,(float32_t *)&fc.rx_filt_i[0], &FirState_I[0],fc.rx_q_block_size);	// load "I" with "Q" coefficients
	else								// not in AM mode, but SSB or FM - use normal settings where I and Q are 90 degrees apart
		arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I,fc.rx_i_num_taps,(float32_t *)&fc.rx_filt_i[0], &FirState_I[0],fc.rx_i_block_size);	// load "I" with "I" coefficients
	//
	arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_Q,fc.rx_q_num_taps,(float32_t *)&fc.rx_filt_q[0], &FirState_Q[0],fc.rx_q_block_size);		// load "Q" with "Q" coefficients
	//
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcTxCompLevel
//* Object              : Set TX audio compression settings (gain and ALC decay rate) based on user setting
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiCalcTxCompLevel(void)
{
	float tcalc;
	ushort	value;

	switch(ts.tx_comp_level)	{				// get the speech compressor setting
		case 0:		// minimum compression
			ts.alc_tx_postfilt_gain_var = 1;		// set the gain for that processor value
			ts.alc_decay_var = 15;					// set the decay rate for that processor value
			break;
		case 1:
			ts.alc_tx_postfilt_gain_var = 2;
			ts.alc_decay_var = 12;
			break;
		case 2:
			ts.alc_tx_postfilt_gain_var = 4;
			ts.alc_decay_var = 10;
			break;
		case 3:
			ts.alc_tx_postfilt_gain_var = 6;
			ts.alc_decay_var = 9;
			break;
		case 4:
			ts.alc_tx_postfilt_gain_var = 8;
			ts.alc_decay_var = 8;
			break;
		case 5:
			ts.alc_tx_postfilt_gain_var = 7;
			ts.alc_decay_var = 7;
			break;
		case 6:
			ts.alc_tx_postfilt_gain_var = 10;
			ts.alc_decay_var = 6;
			break;
		case 7:
			ts.alc_tx_postfilt_gain_var = 12;
			ts.alc_decay_var = 5;
			break;
		case 8:
			ts.alc_tx_postfilt_gain_var = 15;
			ts.alc_decay_var = 4;
			break;
		case 9:
			ts.alc_tx_postfilt_gain_var = 17;
			ts.alc_decay_var = 3;
			break;
		case 10:
			ts.alc_tx_postfilt_gain_var = 20;
			ts.alc_decay_var = 2;
			break;
		case 11:
			ts.alc_tx_postfilt_gain_var = 25;
			ts.alc_decay_var = 1;
			break;
		case 12:		// Maximum compression
			ts.alc_tx_postfilt_gain_var = 25;
			ts.alc_decay_var = 0;
			break;
		case 13:	// read saved values from EEPROM
			ts.alc_tx_postfilt_gain_var = ts.alc_tx_postfilt_gain;		// restore "pristine" EEPROM values
			ts.alc_decay_var = ts.alc_decay;
			break;
		default:
			ts.alc_tx_postfilt_gain_var = 4;
			ts.alc_decay_var = 10;
			break;
	}
	//
	tcalc = (float)ts.alc_decay_var;	// use temp var "tcalc" as audio function
	tcalc += 35;			// can be called mid-calculation!
	tcalc /= 10;
	tcalc *= -1;
	tcalc = powf(10, tcalc);
	ads.alc_decay = tcalc;
}
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcSubaudibleGenFreq
//* Object              : Calculate frequency word for subaudible tone generation  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiCalcSubaudibleGenFreq(void)
{
	ads.fm_subaudible_tone_gen_freq = fm_subaudible_tone_table[ts.fm_subaudible_tone_gen_select];		// look up tone frequency (in Hz)
	ads.fm_subaudible_tone_word = (ulong)(ads.fm_subaudible_tone_gen_freq * FM_SUBAUDIBLE_TONE_WORD_CALC_FACTOR);	// calculate tone word
}
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcSubaudibleDetFreq
//* Object              : Calculate frequency word for subaudible tone  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiCalcSubaudibleDetFreq(void)
{
	ulong size;

	size = BUFF_LEN;

	ads.fm_subaudible_tone_det_freq = fm_subaudible_tone_table[ts.fm_subaudible_tone_det_select];		// look up tone frequency (in Hz)
	//
	// Calculate Goertzel terms for tone detector(s)
	//
	// Terms for "above" detection frequency
	//
	ads.fm_goertzel_high_a = (0.5 + (ads.fm_subaudible_tone_det_freq * FM_GOERTZEL_HIGH) * FM_SUBAUDIBLE_GOERTZEL_WINDOW * (size/2)/48000);
	ads.fm_goertzel_high_b = (2*PI*ads.fm_goertzel_high_a)/(FM_SUBAUDIBLE_GOERTZEL_WINDOW*size/2);
	ads.fm_goertzel_high_sin = sin(ads.fm_goertzel_high_b);
	ads.fm_goertzel_high_cos = cos(ads.fm_goertzel_high_b);
	ads.fm_goertzel_high_r = 2 * ads.fm_goertzel_high_cos;
	//
	// Terms for "below" detection frequency
	//
	ads.fm_goertzel_low_a = (0.5 + (ads.fm_subaudible_tone_det_freq * FM_GOERTZEL_LOW) * FM_SUBAUDIBLE_GOERTZEL_WINDOW * (size/2)/48000);
	ads.fm_goertzel_low_b = (2*PI*ads.fm_goertzel_low_a)/(FM_SUBAUDIBLE_GOERTZEL_WINDOW*size/2);
	ads.fm_goertzel_low_sin = sin(ads.fm_goertzel_low_b);
	ads.fm_goertzel_low_cos = cos(ads.fm_goertzel_low_b);
	ads.fm_goertzel_low_r = 2 * ads.fm_goertzel_low_cos;
	//
	// Terms for the actual detection frequency
	//
	ads.fm_goertzel_ctr_a = (0.5 + ads.fm_subaudible_tone_det_freq * FM_SUBAUDIBLE_GOERTZEL_WINDOW * (size/2)/48000);
	ads.fm_goertzel_ctr_b = (2*PI*ads.fm_goertzel_ctr_a)/(FM_SUBAUDIBLE_GOERTZEL_WINDOW*size/2);
	ads.fm_goertzel_ctr_sin = sin(ads.fm_goertzel_ctr_b);
	ads.fm_goertzel_ctr_cos = cos(ads.fm_goertzel_ctr_b);
	ads.fm_goertzel_ctr_r = 2 * ads.fm_goertzel_ctr_cos;
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiLoadToneBurstMode
//* Object              : Load tone burst mode  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLoadToneBurstMode(void)
{
	switch(ts.fm_tone_burst_mode)	{
		case FM_TONE_BURST_1750_MODE:
			ads.fm_tone_burst_word = FM_TONE_BURST_1750;
			break;
		case FM_TONE_BURST_2135_MODE:
			ads.fm_tone_burst_word = FM_TONE_BURST_2135;
			break;
		default:
			ads.fm_tone_burst_word = 0;
			break;
	}

}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiLoadBeepFreq
//* Object              : Load beep frequency  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLoadBeepFreq(void)
{
	float calc;

	if(ts.misc_flags2 & 4)	{	// is beep enabled?
		ads.beep_word = ts.beep_frequency * 65536;		// yes - calculated/load frequency
		ads.beep_word /= 48000;
	}
	else
		ads.beep_word = 0;		// not enabled - zero out frequency word
	//
	calc = (float)ts.beep_loudness;		// range 0-20
	calc /= 2;							// range 0-10
	calc *= calc;						// range 0-100
	calc += 3;							// range 3-103
	ads.beep_loudness_factor = calc / 400;		// range from 0.0075 to 0.2575 - multiplied by DDS output
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiKeyBeep
//* Object              : Make beep  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiKeyBeep(void)
{
	UiLoadBeepFreq();		// load and calculate beep frequency
	ts.beep_timing = ts.sysclock + BEEP_DURATION;		// set duration of beep
	ts.beep_active = 1;									// activate tone
}
//
void UiSideToneRef(void)
{
	if((ts.dmod_mode == DEMOD_CW) || (ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB))	{		// do sidetone beep only in modes that have a "BFO"
		ads.beep_word = ts.sidetone_freq * 65536;		// yes - calculated/load frequency
		ads.beep_word /= 48000;
		ts.beep_timing = ts.sysclock + SIDETONE_REF_BEEP_DURATION;	// set duration of beep used as a reference for the sidetone frequency
		ts.beep_active = 1;
	}

}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcTxPhaseAdj
//* Object              : Calculate TX FFT coeffients based on adjustment settings
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiCalcTxPhaseAdj(void)
{
	float f_coeff, f_offset, var_norm, var_inv;
	ulong i;
	int phase;
	//
	ads.tx_filter_adjusting = 1;		// disable TX I/Q filter during adjustment
	//

	// always make a fresh copy of the original Q and I coefficients
	// NOTE:  We are assuming that the I and Q filters are of the same length!
	//
	fc.tx_q_num_taps = Q_TX_NUM_TAPS;
	fc.tx_i_num_taps = I_TX_NUM_TAPS;
	//
	fc.tx_q_block_size = Q_TX_BLOCK_SIZE;
	fc.tx_i_block_size = I_TX_BLOCK_SIZE;
	//
	for(i = 0; i < Q_TX_NUM_TAPS; i++)	{
		fc.tx_filt_q[i] = q_tx_coeffs[i];
		fc.tx_filt_i[i] = i_tx_coeffs[i];
	}
	//
	if(ts.dmod_mode == DEMOD_LSB)
		phase = ts.tx_iq_lsb_phase_balance;		// yes, get current gain adjustment setting for LSB
	else
		phase = ts.tx_iq_usb_phase_balance;		// yes, get current gain adjustment setting
	//
	if(phase != 0)	{	// is phase adjustment non-zero?
		var_norm = (float)phase;		// yes, get current gain adjustment setting
		var_norm = fabs(var_norm);		// get absolute value of this gain adjustment
		var_inv = 32 - var_norm;		// calculate "inverse" of number of steps
		var_norm /= 32;		// fractionalize by the number of steps
		var_inv /= 32;						// fractionalize this one, too
		if(phase < 0)	{	// was the phase adjustment negative?
			for(i = 0; i < Q_TX_NUM_TAPS; i++)	{
				f_coeff = var_inv * q_tx_coeffs[i];	// get fraction of 90 degree setting
				f_offset = var_norm * q_tx_coeffs_minus[i];
				fc.tx_filt_q[i] = f_coeff + f_offset;
			}
		}
		else	{							// adjustment was positive
			for(i = 0; i < Q_TX_NUM_TAPS; i++)	{
				f_coeff = var_inv * q_tx_coeffs[i];	// get fraction of 90 degree setting
				f_offset = var_norm * q_tx_coeffs_plus[i];
				fc.tx_filt_q[i] = f_coeff + f_offset;
			}
		}
	}
	//
	arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I_TX,fc.tx_i_num_taps,(float32_t *)&fc.tx_filt_i[0], &FirState_I_TX[0],fc.tx_i_block_size);
	arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_Q_TX,fc.tx_q_num_taps,(float32_t *)&fc.tx_filt_q[0], &FirState_Q_TX[0],fc.tx_q_block_size);

	ads.tx_filter_adjusting = 0;		// re enable TX I/Q filter now that we are done
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
		//
		//printf("-->filter mode loaded\n\r");
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
{
char txt[64];

	uint16_t i;

	if((ts.version_number_build != TRX4M_VER_BUILD) || (ts.version_number_release != TRX4M_VER_RELEASE) || (ts.version_number_minor != TRX4M_VER_MINOR))	{	// Does the current version NOT match what was in the EEPROM?
		return;		// it does NOT match - DO NOT allow a "Load Default" operation this time!
	}

	if((!UiDriverButtonCheck(BUTTON_F1_PRESSED)) && (!UiDriverButtonCheck(BUTTON_F3_PRESSED)) && (!UiDriverButtonCheck(BUTTON_F5_PRESSED)))	{	// Are F1, F3 and F5 being held down?
		ts.load_eeprom_defaults = 1;						// yes, set flag to indicate that defaults will be loaded instead of those from EEPROM
		ts.boot_halt_flag = 1;								// set flag to halt boot-up
		UiDriverLoadEepromValues();							// call function to load values - default instead of EEPROM
		//
		UiLcdHy28_LcdClear(Red);							// clear the screen
		//													// now do all of the warnings, blah, blah...
		sprintf(txt,"   EEPROM DEFAULTS");
		UiLcdHy28_PrintText(2,05,txt,White,Red,1);
		sprintf(txt,"      LOADED!!!");
		UiLcdHy28_PrintText(2,35,txt,White,Red,1);
		//
		sprintf(txt,"  DISCONNECT power NOW if you do NOT");
		UiLcdHy28_PrintText(2,70,txt,Cyan,Red,0);
		//
		sprintf(txt,"  want to lose your current settings!");
		UiLcdHy28_PrintText(2,85,txt,Cyan,Red,0);
		//
		sprintf(txt,"  If you want to save default settings");
		UiLcdHy28_PrintText(2,120,txt,Green,Red,0);
		//
		sprintf(txt,"  press and hold POWER button to power");
		UiLcdHy28_PrintText(2,135,txt,Green,Red,0);
		//
		sprintf(txt,"   down and save settings to EEPROM.");
		UiLcdHy28_PrintText(2,150,txt,Green,Red,0);
		//
		// On screen delay									// delay a bit...
		for(i = 0; i < 10; i++)
		   non_os_delay();
		//
		sprintf(txt,"     YOU HAVE BEEN WARNED!");			// add this for emphasis
		UiLcdHy28_PrintText(50,195,txt,Yellow,Red,0);

		sprintf(txt,"               [Radio startup halted]");
		UiLcdHy28_PrintText(2,225,txt,White,Red,4);
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
{
char txt[64];

	uint16_t i;

	if((ts.version_number_build != TRX4M_VER_BUILD) || (ts.version_number_release != TRX4M_VER_RELEASE) || (ts.version_number_minor != TRX4M_VER_MINOR))	{	// Does the current version NOT match what was in the EEPROM?
		return;		// it does NOT match - DO NOT allow a "Load Default" operation this time!
	}

	if((!UiDriverButtonCheck(BUTTON_F2_PRESSED)) && (!UiDriverButtonCheck(BUTTON_F4_PRESSED)))	{	// Are F2, F4 being held down?
		ts.load_freq_mode_defaults = 1;						// yes, set flag to indicate that frequency/mode defaults will be loaded instead of those from EEPROM
		ts.boot_halt_flag = 1;								// set flag to halt boot-up
		UiDriverLoadEepromValues();							// call function to load values - default instead of EEPROM
		//
		UiLcdHy28_LcdClear(Yellow);							// clear the screen
		//													// now do all of the warnings, blah, blah...
		sprintf(txt,"   FREQUENCY/MODE");
		UiLcdHy28_PrintText(2,05,txt,Black,Yellow,1);
		sprintf(txt," DEFAULTS LOADED!!!");
		UiLcdHy28_PrintText(2,35,txt,Black,Yellow,1);
		//
		sprintf(txt,"  DISCONNECT power NOW if you do NOT");
		UiLcdHy28_PrintText(2,70,txt,Black,Yellow,0);
		//
		sprintf(txt,"want to lose your current frequencies!");
		UiLcdHy28_PrintText(2,85,txt,Black,Yellow,0);
		//
		sprintf(txt,"If you want to save default frequencies");
		UiLcdHy28_PrintText(2,120,txt,Black,Yellow,0);
		//
		sprintf(txt,"  press and hold POWER button to power");
		UiLcdHy28_PrintText(2,135,txt,Black,Yellow,0);
		//
		sprintf(txt,"   down and save settings to EEPROM.");
		UiLcdHy28_PrintText(2,150,txt,Black,Yellow,0);
		//
		// On screen delay									// delay a bit...
		for(i = 0; i < 10; i++)
		   non_os_delay();
		//
		sprintf(txt,"     YOU HAVE BEEN WARNED!");			// add this for emphasis
		UiLcdHy28_PrintText(50,195,txt,Black,Yellow,0);

		sprintf(txt,"               [Radio startup halted]");
		UiLcdHy28_PrintText(2,225,txt,Black,Yellow,4);
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
void UiCheckForPressedKey(void)
{
	ushort i, j, k, p_o_state, rb_state, new_state;
	uint32_t poweroffcount, rbcount;
	uchar	b;
	bool stat = 1;
	poweroffcount = rbcount = 0;
	p_o_state = rb_state = new_state = 0;
	char txt[32];

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
	sprintf(txt,"  Button Test  ");
	UiLcdHy28_PrintText(40,35,txt,White,Blue,1);
	//
	sprintf(txt,"press & hold POWER-button to poweroff");
	UiLcdHy28_PrintText(15,70,txt,White,Blue,0);
	sprintf(txt,"press & hold BANDM-button to reboot");
	UiLcdHy28_PrintText(20,90,txt,White,Blue,0);
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
				strcpy(txt, "POWER ");
				if(poweroffcount > 75)
				    {
				    strcpy(txt, "powering off");
				    p_o_state = 1;
				    }
				poweroffcount++;
				break;
			case	BUTTON_M1_PRESSED:
				strcpy(txt, "  M1  ");
				break;
			case	BUTTON_M2_PRESSED:
				strcpy(txt, "  M2  ");
				break;
			case	BUTTON_M3_PRESSED:
				strcpy(txt, "  M3  ");
				break;
			case	BUTTON_G1_PRESSED:
				strcpy(txt, "  G1  ");
				break;
			case	BUTTON_G2_PRESSED:
				strcpy(txt, "  G2  ");
				break;
			case	BUTTON_G3_PRESSED:
				strcpy(txt, "  G3  ");
				break;
			case	BUTTON_G4_PRESSED:
				strcpy(txt, "  G4  ");
				break;
			case	BUTTON_F1_PRESSED:
				strcpy(txt, "  F1  ");
				break;
			case	BUTTON_F2_PRESSED:
				strcpy(txt, "  F2  ");
				break;
			case	BUTTON_F3_PRESSED:
				strcpy(txt, "  F3  ");
				break;
			case	BUTTON_F4_PRESSED:
				strcpy(txt, "  F4  ");
				poweroffcount = 0;
				break;
			case	BUTTON_F5_PRESSED:
				strcpy(txt, "  F5  ");
				break;
			case	BUTTON_BNDM_PRESSED:
				strcpy(txt, " BNDM ");
				if(rbcount > 75)
				    {
				    strcpy(txt, "rebooting");
				    rb_state = 1;
				    }
				rbcount++;
				break;
			case	BUTTON_BNDP_PRESSED:
				strcpy(txt, " BNDP ");
				break;
			case	BUTTON_STEPM_PRESSED:
				strcpy(txt, "STEPM ");
				break;
			case	BUTTON_STEPP_PRESSED:
				strcpy(txt, "STEPP ");
				break;
			case	TOUCHSCREEN_ACTIVE: ;
				char out[32];
				get_touchscreen_coordinates();
				sprintf(out,"%02x%s%02x", ts.tp_x,"  ",ts.tp_y);	//show touched coordinates
				strcpy(txt, out);
				break;
			default:
				strcpy(txt, "<Null>");		// no button pressed
				poweroffcount = 0;
				rbcount = 0;
		}
		//
		UiLcdHy28_PrintText(120,120,txt,White,Blue,1);		// identify button on screen
		sprintf(txt, "# of buttons pressed: %d  ", (int)k);
		UiLcdHy28_PrintText(75,160,txt,White,Blue,0);		// show number of buttons pressed on screen

		if(p_o_state == 1)
		    {
		    GPIO_SetBits(POWER_DOWN_PIO,POWER_DOWN);
		    while (1 == 1) ;
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
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverLoadEepromValues
//* Object              : load saved values on driver start
//* Input Parameters    : Indirect:  If "ts.load_eeprom_defaults" is TRUE, default values will be loaded instead of EEPROM values.
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiDriverLoadEepromValues(void)
{
	ushort value,value1;
	uint16_t uint_val;
	int16_t	*int_val;	// Note:  This "signed" variable pointer actually points to an unsigned variable ("uint_val" as the EEPROM save function only "knows" about
						// unsigned variables.
	//
	int_val = &uint_val;	// Copy the memory location of the unsigned integer value to a "signed" pointer to allow us to reliabily read signed values from EEPROM to memory.
							// This results in a warning that I haven't been able to suppress (KA7OEI)

	// Do a sample reads to "prime the pump" before we start...
	// This is to make the function work reliabily after boot-up
	//
	Read_EEPROM(EEPROM_ZERO_LOC_UNRELIABLE, &value);	// Let's use location zero - which may not work reliably, anyway!
	//
	// ------------------------------------------------------------------------------------
	// Try to read Band and Mode saved values
	if(Read_EEPROM(EEPROM_BAND_MODE, &value) == 0)
	{
		ts.band = value & 0x00FF;
		if(ts.band > MAX_BANDS-1)			// did we get an insane value from EEPROM?
			ts.band = BAND_MODE_80;		//	yes - set to 80 meters
		//
		ts.dmod_mode = (value >> 8) & 0x0F;		// demodulator mode might not be right for saved band!
		if((ts.dmod_mode > DEMOD_MAX_MODE)  || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)		// valid mode value from EEPROM? or defaults loaded?
			ts.dmod_mode = DEMOD_LSB;			// no - set to LSB
		//
		ts.filter_id = (value >> 12) & 0x0F;	// get filter setting
		if((ts.filter_id >= AUDIO_MAX_FILTER) || (ts.filter_id < AUDIO_MIN_FILTER) || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)		// audio filter invalid or defaults to be loaded?
			ts.filter_id = AUDIO_DEFAULT_FILTER;	// set default audio filter
		//
		//printf("-->band and mode loaded\n\r");
	}
	// ------------------------------------------------------------------------------------
	// Try to read Freq saved values
	if(	(Read_EEPROM(EEPROM_FREQ_HIGH, &value) == 0) &&
		(Read_EEPROM(EEPROM_FREQ_LOW, &value1) == 0))
	{
		ulong saved = (value << 16) | (value1);

		// We have loaded from eeprom the last used band, but can't just
		// load saved frequency, as it could be out of band, so do a
		// boundary check first (also check to see if defaults should be loaded)
		if((!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults) && (saved >= tune_bands[ts.band]) && (saved <= (tune_bands[ts.band] + size_bands[ts.band])))
		{
			df.tune_new = saved;
			//printf("-->frequency loaded\n\r");
		}
		else if((ts.misc_flags2 & 16) && (saved >= SI570_MIN_FREQ) && (saved <= SI570_MAX_FREQ) && (!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults))	{	// relax memory-save frequency restrictions and is it within the allowed range?
			df.tune_new = saved;
			//printf("-->frequency loaded (relaxed)\n\r");
		}
		else
		{
			// Load default for this band
			df.tune_new = tune_bands[ts.band];
			//printf("-->base frequency loaded\n\r");
		}
	}
	//
	// Try to read saved per-band values for frequency, mode and filter
	//
	ulong i, saved;
	//
	for(i = 0; i < MAX_BANDS; i++)	{		// read from stored bands
		// ------------------------------------------------------------------------------------
		// Try to read Band and Mode saved values
		//
		if(Read_EEPROM(EEPROM_BAND0_MODE + i, &value) == 0)			{
			// Note that ts.band will, by definition, be equal to index "i"
			//
			band_decod_mode[i] = (value >> 8) & 0x0F;		// demodulator mode might not be right for saved band!
			if((ts.dmod_mode > DEMOD_MAX_MODE)  || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)		// valid mode value from EEPROM? or defaults loaded?
				band_decod_mode[i] = DEMOD_LSB;			// no - set to LSB
			//
			band_filter_mode[i] = (value >> 12) & 0x0F;	// get filter setting
			if((band_filter_mode[i] >= AUDIO_MAX_FILTER) || (ts.filter_id < AUDIO_MIN_FILTER) || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)		// audio filter invalid or defaults to be loaded??
				band_filter_mode[i] = AUDIO_DEFAULT_FILTER;	// set default audio filter
			//
			//printf("-->band, mode and filter setting loaded\n\r");
		}

		// ------------------------------------------------------------------------------------
		// Try to read Freq saved values
		if(	(Read_EEPROM(EEPROM_BAND0_FREQ_HIGH + i, &value) == 0) && (Read_EEPROM(EEPROM_BAND0_FREQ_LOW + i, &value1) == 0))	{
			saved = (value << 16) | (value1);
			//
			// We have loaded from eeprom the last used band, but can't just
			// load saved frequency, as it could be out of band, so do a
			// boundary check first (also check to see if defaults should be loaded)
			//
			if((!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults) && (saved >= tune_bands[i]) && (saved <= (tune_bands[i] + size_bands[i])))	{
				band_dial_value[i] = saved;
				//printf("-->frequency loaded\n\r");
			}
			else if((ts.misc_flags2 & 16) && (saved >= SI570_MIN_FREQ) && (saved <= SI570_MAX_FREQ) && (!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults))	{	// relax memory-save frequency restrictions and is it within the allowed range?
				band_dial_value[i] = saved;
				//printf("-->frequency loaded (relaxed)\n\r");
			}
			else	{
				// Load default for this band
				band_dial_value[i] = tune_bands[i] + DEFAULT_FREQ_OFFSET;
				//printf("-->base frequency loaded\n\r");
			}
		}
	}
	//
	for(i = 0; i < MAX_BANDS; i++)	{		// read from stored bands for VFO A
		// ------------------------------------------------------------------------------------
		// Try to read Band and Mode saved values for VFO A
		//
		if(Read_EEPROM(EEPROM_BAND0_MODE_A + i, &value) == 0)			{
			// Note that ts.band will, by definition, be equal to index "i"
			//
			band_decod_mode_a[i] = (value >> 8) & 0x0F;		// demodulator mode might not be right for saved band!
			if((ts.dmod_mode > DEMOD_MAX_MODE)  || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)		// valid mode value from EEPROM? or defaults loaded?
				band_decod_mode_a[i] = DEMOD_LSB;			// no - set to LSB
			//
			band_filter_mode_a[i] = (value >> 12) & 0x0F;	// get filter setting
			if((band_filter_mode_a[i] >= AUDIO_MAX_FILTER) || (ts.filter_id < AUDIO_MIN_FILTER) || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)		// audio filter invalid or are defaults to be loaded?
				band_filter_mode_a[i] = AUDIO_DEFAULT_FILTER;	// set default audio filter
			//
			//printf("-->band, mode and filter setting loaded\n\r");
		}

		// ------------------------------------------------------------------------------------
		// Try to read Freq saved values
		if(	(Read_EEPROM(EEPROM_BAND0_FREQ_HIGH_A + i, &value) == 0) && (Read_EEPROM(EEPROM_BAND0_FREQ_LOW_A + i, &value1) == 0))	{
			saved = (value << 16) | (value1);
			//
			// We have loaded from eeprom the last used band, but can't just
			// load saved frequency, as it could be out of band, so do a
			// boundary check first (also check to see if defaults should be loaded)
			//
			if((!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults) && (saved >= tune_bands[i]) && (saved <= (tune_bands[i] + size_bands[i])))	{
				band_dial_value_a[i] = saved;
				//printf("-->frequency loaded\n\r");
			}
			else if((ts.misc_flags2 & 16) && (saved >= SI570_MIN_FREQ) && (saved <= SI570_MAX_FREQ) && (!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults))	{	// relax memory-save frequency restrictions and is it within the allowed range?
				band_dial_value_a[i] = saved;
				//printf("-->frequency loaded (relaxed)\n\r");
			}
			else	{
				// Load default for this band
				band_dial_value_a[i] = tune_bands[i] + DEFAULT_FREQ_OFFSET;
				//printf("-->base frequency loaded\n\r");
			}
		}
	}
	//
	for(i = 0; i < MAX_BANDS; i++)	{		// read from stored bands for VFO B
		// ------------------------------------------------------------------------------------
		// Try to read Band and Mode saved values for VFO B
		//
		if(Read_EEPROM(EEPROM_BAND0_MODE_A + i, &value) == 0)			{
			// Note that ts.band will, by definition, be equal to index "i"
			//
			band_decod_mode_b[i] = (value >> 8) & 0x0F;		// demodulator mode might not be right for saved band!
			if((ts.dmod_mode > DEMOD_MAX_MODE)  || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)		// valid mode value from EEPROM? or defaults loaded?
				band_decod_mode_b[i] = DEMOD_LSB;			// no - set to LSB
			//
			band_filter_mode_b[i] = (value >> 12) & 0x0F;	// get filter setting
			if((band_filter_mode_b[i] >= AUDIO_MAX_FILTER) || (ts.filter_id < AUDIO_MIN_FILTER) || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)		// audio filter invalid or defaults to be loaded?
				band_filter_mode_b[i] = AUDIO_DEFAULT_FILTER;	// set default audio filter
			//
			//printf("-->band, mode and filter setting loaded\n\r");
		}

		// ------------------------------------------------------------------------------------
		// Try to read Freq saved values
		if(	(Read_EEPROM(EEPROM_BAND0_FREQ_HIGH_B + i, &value) == 0) && (Read_EEPROM(EEPROM_BAND0_FREQ_LOW_B + i, &value1) == 0))	{
			saved = (value << 16) | (value1);
			//
			// We have loaded from eeprom the last used band, but can't just
			// load saved frequency, as it could be out of band, so do a
			// boundary check first (also check to see if defaults should be loaded)
			//
			if((!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults) && (saved >= tune_bands[i]) && (saved <= (tune_bands[i] + size_bands[i])))	{
				band_dial_value_b[i] = saved;
				//printf("-->frequency loaded\n\r");
			}
			else if((ts.misc_flags2 & 16) && (saved >= SI570_MIN_FREQ) && (saved <= SI570_MAX_FREQ) && (!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults))	{	// relax memory-save frequency restrictions and is it within the allowed range?
				band_dial_value_b[i] = saved;
				//printf("-->frequency loaded (relaxed)\n\r");
			}
			else	{
				// Load default for this band
				band_dial_value_b[i] = tune_bands[i] + DEFAULT_FREQ_OFFSET;
				//printf("-->base frequency loaded\n\r");
			}
		}
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Step saved values
	if(Read_EEPROM(EEPROM_FREQ_STEP, &value) == 0)
	{
		if((value >= T_STEP_MAX_STEPS -1) || ts.load_eeprom_defaults)	// did we get step size value outside the range or default to be loaded?
			value = 3;						// yes - set to default size of 1 kHz steps
		//
		df.selected_idx = value;
		df.tuning_step	= tune_steps[df.selected_idx];
		//printf("-->freq step loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX Audio Source saved values
	if(Read_EEPROM(EEPROM_TX_AUDIO_SRC, &value) == 0)
	{
		if(ts.load_eeprom_defaults)					// default?
			ts.tx_audio_source = TX_AUDIO_MIC;		// yes, load
		else
			ts.tx_audio_source = value;
		//printf("-->TX audio source loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TCXO saved values
	if(Read_EEPROM(EEPROM_TCXO_STATE, &value) == 0)
	{
		if(ts.load_eeprom_defaults)				// default?
			df.temp_enabled = TCXO_ON;
		else
			df.temp_enabled = value;
		//printf("-->TCXO state loaded\n\r");
	}
	// ------------------------------------------------------------------------------------
	// Try to read PA BIAS saved values
	if(Read_EEPROM(EEPROM_PA_BIAS, &value) == 0)
	{
		if((value > MAX_PA_BIAS) || ts.load_eeprom_defaults)	// prevent garbage value for bias (or load default value)
			value = DEFAULT_PA_BIAS;
		//
		ts.pa_bias = value;
		//
		ulong bias_val;

		bias_val = BIAS_OFFSET + (ts.pa_bias * 2);
		if(bias_val > 255)
			bias_val = 255;

		// Set DAC Channel1 DHR12L register with bias value
		DAC_SetChannel2Data(DAC_Align_8b_R,bias_val);
		//printf("-->PA BIAS loaded: %d\n\r",ts.pa_bias);
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read PA BIAS saved values
	if(Read_EEPROM(EEPROM_PA_CW_BIAS, &value) == 0)
	{
		if((value > MAX_PA_BIAS) || ts.load_eeprom_defaults)	// prevent garbage value for bias (or load default value)
			value = DEFAULT_PA_BIAS;
		//
		ts.pa_cw_bias = value;
		//
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Audio Gain saved values
	if(Read_EEPROM(EEPROM_AUDIO_GAIN, &value) == 0)
	{
		if((value > MAX_AUDIO_GAIN) || ts.load_eeprom_defaults)	// set default gain if garbage value from EEPROM (or load default value)
			value = DEFAULT_AUDIO_GAIN;
		ts.audio_gain = value;
		//printf("-->Audio Gain loaded\n\r");
	}
	// ------------------------------------------------------------------------------------
	// Try to read RF Codec Gain saved values
	if(Read_EEPROM(EEPROM_RX_CODEC_GAIN, &value) == 0)
	{
		if((value > MAX_RF_CODEC_GAIN_VAL) || ts.load_eeprom_defaults)		// set default if invalid value (or load default value)
			value = DEFAULT_RF_CODEC_GAIN_VAL;
		//
		ts.rf_codec_gain = value;
		//printf("-->RF Codec Gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RF Codec Gain saved values
	if(Read_EEPROM(EEPROM_RX_GAIN, &value) == 0)
	{
		if((value > MAX_RF_GAIN) || ts.load_eeprom_defaults)			// set default if invalid value (or load default value)
			value = DEFAULT_RF_GAIN;
		//
		ts.rf_gain = value;
		//printf("-->RF Gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Noise Blanker saved values
	if(Read_EEPROM(EEPROM_NB_SETTING, &value) == 0)
	{
		if((value > MAX_RF_ATTEN) || ts.load_eeprom_defaults)	// invalid value?  (or load default value)
			value = 0;				// yes - set to zero
		//
		ts.nb_setting = value;
		//printf("-->Noise Blanker value loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Power level saved values
	if(Read_EEPROM(EEPROM_TX_POWER_LEVEL, &value) == 0)
	{
		if((value >= PA_LEVEL_MAX_ENTRY) || ts.load_eeprom_defaults)  // check for valid range (or load default value)
			value = PA_LEVEL_DEFAULT;
		//
		ts.power_level = value;
		//printf("-->Power level loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Keyer speed saved values
	if(Read_EEPROM(EEPROM_KEYER_SPEED, &value) == 0)
	{
		if((value < MIN_KEYER_SPEED) || (value > MAX_KEYER_SPEED) || ts.load_eeprom_defaults)	// value out of range? (or load default value)
			value = DEFAULT_KEYER_SPEED;
		//
		ts.keyer_speed = value;
		//printf("-->Keyer speed loaded\n\r");

		// Note - init will be done below, when Keyer Mode is loaded

	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Keyer mode saved values
	if(Read_EEPROM(EEPROM_KEYER_MODE, &value) == 0)
	{
		if((ts.keyer_mode >= CW_MAX_MODE) || ts.load_eeprom_defaults)	// invalid CW mode value? (or load default value)
			value = CW_MODE_IAM_B;	// set default mode
		//
		ts.keyer_mode = value;
		//printf("-->Keyer mode loaded\n\r");

		// Extra init needed
		cw_gen_init();

	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Gain saved values
	if(Read_EEPROM(EEPROM_SIDETONE_GAIN, &value) == 0)
	{
		if((value > SIDETONE_MAX_GAIN) || ts.load_eeprom_defaults)			// out of range of gain settings? (or load default value)
			value = DEFAULT_SIDETONE_GAIN;		// yes, use default
		//
		ts.st_gain = value;
		//printf("-->Sidetone Gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Audio Gain saved values
	if(Read_EEPROM(EEPROM_AUDIO_GAIN, &value) == 0)
	{
		if((value > MAX_AUDIO_GAIN) || ts.load_eeprom_defaults)			// out of range of gain settings? (or load default value)
			value = DEFAULT_AUDIO_GAIN;		// yes, use default
		//
		ts.audio_gain = value;
		//printf("-->Audio Gain loaded\n\r");
	}
	//
//	// ------------------------------------------------------------------------------------
//	// Try to read MIC BOOST saved values - DEPRICATED, functionality now built into "Codec_RX_TX()"
//	if(Read_EEPROM(EEPROM_MIC_BOOST, &value) == 0)
//	{
//		if(value < 2)
//			ts.mic_boost = value;
//	}
//	//
	// ------------------------------------------------------------------------------------
	// Try to read TX LSB Phase saved values
	if(Read_EEPROM(EEPROM_TX_IQ_LSB_PHASE_BALANCE, &uint_val) == 0)
	{
		if((*int_val < MIN_TX_IQ_PHASE_BALANCE) || (*int_val > MAX_TX_IQ_PHASE_BALANCE) || ts.load_eeprom_defaults)	// out of range (or load default value)
			*int_val = 0;		// yes, use zero
		//
		ts.tx_iq_lsb_phase_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX USB Phase saved values
	if(Read_EEPROM(EEPROM_TX_IQ_USB_PHASE_BALANCE, &uint_val) == 0)
	{
		if((*int_val < MIN_TX_IQ_PHASE_BALANCE) || (*int_val > MAX_TX_IQ_PHASE_BALANCE) || ts.load_eeprom_defaults)	// out of range (or load default value)
			*int_val = 0;		// yes, use zero
		//
		ts.tx_iq_usb_phase_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX LSB Gain saved values
	if(Read_EEPROM(EEPROM_TX_IQ_LSB_GAIN_BALANCE, &uint_val) == 0)
	{
		if((*int_val < MIN_TX_IQ_GAIN_BALANCE) || (*int_val > MAX_TX_IQ_GAIN_BALANCE) || ts.load_eeprom_defaults)	// out of range? (or load default value)
			*int_val = 0;		// yes, use zero
		//
		ts.tx_iq_lsb_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX USB Gain saved values
	if(Read_EEPROM(EEPROM_TX_IQ_USB_GAIN_BALANCE, &uint_val) == 0)
	{
		if((*int_val < MIN_TX_IQ_GAIN_BALANCE) || (*int_val > MAX_TX_IQ_GAIN_BALANCE) || ts.load_eeprom_defaults)	// out of range? (or load default value)
			*int_val = 0;		// yes, use zero
		//
		ts.tx_iq_usb_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX LSB Phase saved values
	if(Read_EEPROM(EEPROM_RX_IQ_LSB_PHASE_BALANCE, &uint_val) == 0)
	{
		if((*int_val < MIN_RX_IQ_PHASE_BALANCE) || (*int_val > MAX_RX_IQ_PHASE_BALANCE) || ts.load_eeprom_defaults)	// out of range (or load default value)
			*int_val = 0;		// yes - set default
		//
		ts.rx_iq_lsb_phase_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX USB Phase saved values
	if(Read_EEPROM(EEPROM_RX_IQ_USB_PHASE_BALANCE, &uint_val) == 0)
	{
		if((*int_val < MIN_RX_IQ_PHASE_BALANCE) || (*int_val > MAX_RX_IQ_PHASE_BALANCE) || ts.load_eeprom_defaults)	// out of range (or load default value)
			*int_val = 0;		// yes - set default
		//
		ts.rx_iq_usb_phase_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX LSB Gain saved values
	if(Read_EEPROM(EEPROM_RX_IQ_LSB_GAIN_BALANCE, &uint_val) == 0)
	{
		if((*int_val < MIN_RX_IQ_GAIN_BALANCE) || (*int_val > MAX_RX_IQ_GAIN_BALANCE) || ts.load_eeprom_defaults)	// out of range? (or load default value)
			*int_val = 0;	// yes - set default
		//
		ts.rx_iq_lsb_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX USB Gain saved values
	if(Read_EEPROM(EEPROM_RX_IQ_USB_GAIN_BALANCE, &uint_val) == 0)
	{
		if((*int_val < MIN_RX_IQ_GAIN_BALANCE) || (*int_val > MAX_RX_IQ_GAIN_BALANCE) || ts.load_eeprom_defaults)	// out of range? (or load default value)
			*int_val = 0;	// yes - set default
		//
		ts.rx_iq_usb_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX AM Gain saved values
	if(Read_EEPROM(EEPROM_RX_IQ_AM_GAIN_BALANCE, &uint_val) == 0)
	{
//		int_val = &uint_val;	// kludge here to preserve sign of restored value - I don't know how to prevent error as EEPROM function doesn't deal with signed variables
		if((*int_val < MIN_RX_IQ_GAIN_BALANCE) || (*int_val > MAX_RX_IQ_GAIN_BALANCE) || ts.load_eeprom_defaults)	// out of range? (or load default value)
			*int_val = 0;	// yes - set default
		//
		ts.rx_iq_am_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX AM Gain saved values
	if(Read_EEPROM(EEPROM_RX_IQ_FM_GAIN_BALANCE, &uint_val) == 0)
	{
//		int_val = &uint_val;	// kludge here to preserve sign of restored value - I don't know how to prevent error as EEPROM function doesn't deal with signed variables
		if((*int_val < MIN_RX_IQ_GAIN_BALANCE) || (*int_val > MAX_RX_IQ_GAIN_BALANCE) || ts.load_eeprom_defaults)	// out of range? (or load default value)
			*int_val = 0;	// yes - set default
		//
		ts.rx_iq_fm_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX AM Gain saved values
	if(Read_EEPROM(EEPROM_TX_IQ_AM_GAIN_BALANCE, &uint_val) == 0)
	{
//		int_val = &uint_val;	// kludge here to preserve sign of restored value - I don't know how to prevent error as EEPROM function doesn't deal with signed variables
		if((*int_val < MIN_TX_IQ_GAIN_BALANCE) || (*int_val > MAX_TX_IQ_GAIN_BALANCE) || ts.load_eeprom_defaults)	// out of range? (or load default value)
			*int_val = 0;	// yes - set default
		//
		ts.tx_iq_am_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX FM Gain saved values
	if(Read_EEPROM(EEPROM_TX_IQ_FM_GAIN_BALANCE, &uint_val) == 0)
	{
//		int_val = &uint_val;	// kludge here to preserve sign of restored value - I don't know how to prevent error as EEPROM function doesn't deal with signed variables
		if((*int_val < MIN_TX_IQ_GAIN_BALANCE) || (*int_val > MAX_TX_IQ_GAIN_BALANCE) || ts.load_eeprom_defaults)	// out of range? (or load default value)
			*int_val = 0;	// yes - set default
		//
		ts.tx_iq_fm_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX Frequency Calibration
	if(Read_EEPROM(EEPROM_FREQ_CAL, &uint_val) == 0)
	{
		if((*int_val < MIN_FREQ_CAL) || (*int_val > MAX_FREQ_CAL) || ts.load_eeprom_defaults)	// out of range (or load default value)
			*int_val = 0;		// yes - set default
		//
		ts.freq_cal = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read AGC mode saved values
	if(Read_EEPROM(EEPROM_AGC_MODE, &value) == 0)
	{
		if((value > AGC_MAX_MODE) || ts.load_eeprom_defaults)	// out of range of AGC settings? (or load default value)
			value = AGC_DEFAULT;				// yes, use default
		//
		ts.agc_mode = value;
		//printf("-->AGC mode loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read MIC gain saved values
	if(Read_EEPROM(EEPROM_MIC_GAIN, &value) == 0)
	{
		if((value > MIC_GAIN_MAX) || (value < MIC_GAIN_MIN) || ts.load_eeprom_defaults)		// out of range of MIC gain settings? (or load default value)
			value = MIC_GAIN_DEFAULT;				// yes, use default
		//
		ts.tx_mic_gain = value;
		//printf("-->MIC gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read LIEN gain saved values
	if(Read_EEPROM(EEPROM_LINE_GAIN, &value) == 0)
	{
		if((value > LINE_GAIN_MAX) || (value < LINE_GAIN_MIN) || ts.load_eeprom_defaults)		// out of range of LINE gain settings? (or load default value)
			value = LINE_GAIN_DEFAULT;				// yes, use default
		//
		ts.tx_line_gain = value;
		//printf("-->LINE gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Frequency saved values
	if(Read_EEPROM(EEPROM_SIDETONE_FREQ, &value) == 0)
	{
		if((value > CW_SIDETONE_FREQ_MAX) || (value < CW_SIDETONE_FREQ_MIN) || ts.load_eeprom_defaults)		// out of range of sidetone freq settings? (or load default value)
			value = CW_SIDETONE_FREQ_DEFAULT;				// yes, use default
		//
		ts.sidetone_freq = value;
		//printf("-->Sidetone freq. loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope Speed saved values
	if(Read_EEPROM(EEPROM_SPEC_SCOPE_SPEED, &value) == 0)
	{
		if((value > SPECTRUM_SCOPE_SPEED_MAX) || ts.load_eeprom_defaults) 	// out of range of spectrum scope speed settings? (or load default value)
			value = SPECTRUM_SCOPE_SPEED_DEFAULT;				// yes, use default
		//
		ts.scope_speed = value;
		//printf("-->Spectrum scope speed loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope Filter Strength saved values
	if(Read_EEPROM(EEPROM_SPEC_SCOPE_FILTER, &value) == 0)
	{
		if((value > SPECTRUM_SCOPE_FILTER_MAX) || (value < SPECTRUM_SCOPE_FILTER_MIN) || ts.load_eeprom_defaults)	// out of range of spectrum scope filter strength settings? (or load default value)
			value = SPECTRUM_SCOPE_FILTER_DEFAULT;				// yes, use default
		//
		ts.scope_filter = value;
		//printf("-->Spectrum scope filter strength loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Custom AGC Decay saved values
	if(Read_EEPROM(EEPROM_AGC_CUSTOM_DECAY, &value) == 0)
	{
		if((value > AGC_CUSTOM_MAX)	|| ts.load_eeprom_defaults)	// out of range Custom AGC Decay settings? (or load default value)
			value = AGC_CUSTOM_DEFAULT;				// yes, use default
		//
		ts.agc_custom_decay = value;
		//printf("-->Custom AGC Decay setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read color for spectrum scope trace saved values
	if(Read_EEPROM(EEPROM_SPECTRUM_TRACE_COLOUR, &value) == 0)
	{
		if((value > SPEC_MAX_COLOUR) || ts.load_eeprom_defaults)	// out of range Spectrum Scope color settings? (or load default value)
			value = SPEC_COLOUR_TRACE_DEFAULT;				// yes, use default
		//
		ts.scope_trace_colour = value;
		//printf("-->Spectrum Scope trace color loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read color for spectrum scope grid saved values
	if(Read_EEPROM(EEPROM_SPECTRUM_GRID_COLOUR, &value) == 0)
	{
		if((value > SPEC_MAX_COLOUR) || ts.load_eeprom_defaults)	// out of range Spectrum Scope color settings? (or load default value)
			value = SPEC_COLOUR_GRID_DEFAULT;				// yes, use default
		//
		ts.scope_grid_colour = value;
		//printf("-->Spectrum Scope grid color loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read color for spectrum scope center line grid saved values
	if(Read_EEPROM(EEPROM_SPECTRUM_CENTRE_GRID_COLOUR, &value) == 0)
	{
		if((value > SPEC_MAX_COLOUR) || ts.load_eeprom_defaults)	// out of range Spectrum Scope color settings? (or load default value)
			value = SPEC_COLOUR_GRID_DEFAULT;				// yes, use default
		//
		ts.scope_centre_grid_colour = value;
		//printf("-->Spectrum Scope centre grid line color loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read color for spectrum scope scale saved values
	if(Read_EEPROM(EEPROM_SPECTRUM_SCALE_COLOUR, &value) == 0)
	{
		if((value > SPEC_MAX_COLOUR) || ts.load_eeprom_defaults)	// out of range Spectrum Scope color settings? (or load default value)
			value = SPEC_COLOUR_SCALE_DEFAULT;				// yes, use default
		//
		ts.scope_scale_colour = value;
		//printf("-->Spectrum Scope scale color loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read paddle reverse saved values
	if(Read_EEPROM(EEPROM_PADDLE_REVERSE, &value) == 0)
	{
		if((value > 1) || ts.load_eeprom_defaults)	// out of range paddle reverse boolean settings? (or load default value)
			value = 0;				// yes, use default (off)
		//
		ts.paddle_reverse = value;
		//printf("-->Paddle Reverse setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read CW TX>RX Delay saved values
	if(Read_EEPROM(EEPROM_CW_RX_DELAY, &value) == 0)
	{
		if((value > CW_RX_DELAY_MAX) || ts.load_eeprom_defaults)	// out of range CW TX>RX Delay settings? (or load default value)
			value = CW_RX_DELAY_DEFAULT;	// yes, use default
		//
		ts.cw_rx_delay = value;
		//printf("-->CW TX>RX Delay setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read maximum volume  saved values
	if(Read_EEPROM(EEPROM_MAX_VOLUME, &value) == 0)
	{
		if((value < MAX_VOLUME_MIN) || (value > MAX_VOLUME_MAX) || ts.load_eeprom_defaults)	// out range of maximum volume settings? (or load default value)
			value = MAX_VOLUME_DEFAULT;	// yes, use default
		//
		ts.audio_max_volume = value;
		//printf("-->maximum volume setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 300 Hz filter saved values
	if(Read_EEPROM(EEPROM_FILTER_300HZ_SEL, &value) == 0)
	{
		if((value > MAX_300HZ_FILTER) || ts.load_eeprom_defaults)	// out range of filter settings? (or load default value)
			value = FILTER_300HZ_DEFAULT;	// yes, use default
		//
		ts.filter_300Hz_select = value;
		//printf("-->300 Hz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 500 Hz filter saved values
	if(Read_EEPROM(EEPROM_FILTER_500HZ_SEL, &value) == 0)
	{
		if((value > MAX_500HZ_FILTER) || ts.load_eeprom_defaults)	// out range of filter settings? (or load default value)
			value = FILTER_500HZ_DEFAULT;	// yes, use default
		//
		ts.filter_500Hz_select = value;
		//printf("-->500 Hz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 1.8 kHz filter saved values
	if(Read_EEPROM(EEPROM_FILTER_1K8_SEL, &value) == 0)
	{
		if((value > MAX_1K8_FILTER) || ts.load_eeprom_defaults)	// out range of filter settings? (or load default value)
			value = FILTER_1K8_DEFAULT;	// yes, use default
		//
		ts.filter_1k8_select = value;
		//printf("-->1.8 kHz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 2.3 kHz filter saved values
	if(Read_EEPROM(EEPROM_FILTER_2K3_SEL, &value) == 0)
	{
		if((value > MAX_2K3_FILTER)	|| ts.load_eeprom_defaults) // out range of filter settings? (or load default value)
			value = FILTER_2K3_DEFAULT;	// yes, use default
		//
		ts.filter_2k3_select = value;
		//printf("-->2.3 kHz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 3.6 kHz filter saved values
	if(Read_EEPROM(EEPROM_FILTER_3K6_SEL, &value) == 0)
	{
		if((value > MAX_3K6_FILTER)	|| ts.load_eeprom_defaults) // out range of filter settings? (or load default value)
			value = FILTER_3K6_DEFAULT;	// yes, use default
		//
		ts.filter_3k6_select = value;
		//printf("-->3.6 kHz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read "wide" filter saved values
	if(Read_EEPROM(EEPROM_FILTER_WIDE_SEL, &value) == 0)
	{
		if((value >= WIDE_FILTER_MAX) || ts.load_eeprom_defaults)	// out range of filter settings? (or load default value)
			value = FILTER_WIDE_DEFAULT;	// yes, use default
		//
		ts.filter_wide_select = value;
		//printf("-->Wide filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read calibration factor for forward power meter
	if(Read_EEPROM(EEPROM_SENSOR_NULL, &value) == 0)
	{
		if((value > SENSOR_NULL_MAX) || (value < SENSOR_NULL_MIN) || ts.load_eeprom_defaults)	// out range of calibration factor for forward power meter settings? (or load default value)
			value = SENSOR_NULL_DEFAULT;	// yes, use default
		//
		swrm.sensor_null = value;
		//printf("-->calibration factor for forward power meter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Transverter Offset Freq saved values
	if(	(Read_EEPROM(EEPROM_XVERTER_OFFSET_HIGH, &value) == 0) &&
		(Read_EEPROM(EEPROM_XVERTER_OFFSET_LOW, &value1) == 0))
	{
		ulong saved = (value << 16) | (value1);

		// We have loaded from eeprom the last used band, but can't just
		// load saved frequency, as it could be out of band, so do a
		// boundary check first
		if((saved <= XVERTER_OFFSET_MAX) && (!ts.load_eeprom_defaults))		// is offset within allowed limits and not loading defaults?
		{
			ts.xverter_offset = saved;			// yes, use this value
			//printf("-->frequency loaded\n\r");
		}
		else		// it's outside allowed limits or default to be loaded - force to zero
		{
			// Load default for this band
			ts.xverter_offset = 0;
			//printf("-->base frequency loaded\n\r");
		}
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read transverter mode enable/disable
	if(Read_EEPROM(EEPROM_XVERTER_DISP, &value) == 0)
	{
		if((value > XVERTER_MULT_MAX) || ts.load_eeprom_defaults)	// if above maximum multipler value, it was bogus (or load default value)
			value = 0;	// reset to "off"
		//
		ts.xverter_mode = value;
		//printf("-->transverter mode enable/disable setting loaded\n\r");
	}
	//
	//
	// ------------------------------------------------------------------------------------
	// Try to read 80 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND0_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_80_DEFAULT;	// reset to default for this band
		//
		ts.pwr_80m_5w_adj = value;
		//printf("-->80 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	//
	// ------------------------------------------------------------------------------------
	// Try to read 60 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND1_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_60_DEFAULT;	// reset to default for this band
		//
		ts.pwr_60m_5w_adj = value;
		//printf("-->60 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 40 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND2_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_40_DEFAULT;	// reset to default for this band
		//
		ts.pwr_40m_5w_adj = value;
		//printf("-->40 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 30 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND3_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_30_DEFAULT;	// reset to default for this band
		//
		ts.pwr_30m_5w_adj = value;
		//printf("-->30 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 20 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND4_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_20_DEFAULT;	// reset to default for this band
		//
		ts.pwr_20m_5w_adj = value;
		//printf("-->20 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 17 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND5_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_17_DEFAULT;	// reset to default for this band
		//
		ts.pwr_17m_5w_adj = value;
		//printf("-->17 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 15 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND6_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_15_DEFAULT;	// reset to default for this band
		//
		ts.pwr_15m_5w_adj = value;
		//printf("-->15 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 12 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND7_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_12_DEFAULT;	// reset to default for this band
		//
		ts.pwr_12m_5w_adj = value;
		//printf("-->12 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10 meter 5 watt power calibration setting
	if(Read_EEPROM(EEPROM_BAND8_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_10_DEFAULT;	// reset to default for this band
		//
		ts.pwr_10m_5w_adj = value;
		//printf("-->10 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 80 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND0_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_80_DEFAULT;	// reset to default for this band
		//
		ts.pwr_80m_full_adj = value;
		//printf("-->80 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 60 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND1_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_60_DEFAULT;	// reset to default for this band
		//
		ts.pwr_60m_full_adj = value;
		//printf("-->60 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 40 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND2_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_40_DEFAULT;	// reset to default for this band
		//
		ts.pwr_40m_full_adj = value;
		//printf("-->40 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 30 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND3_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_30_DEFAULT;	// reset to default for this band
		//
		ts.pwr_30m_full_adj = value;
		//printf("-->30 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 20 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND4_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_20_DEFAULT;	// reset to default for this band
		//
		ts.pwr_20m_full_adj = value;
		//printf("-->20 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 17 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND5_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_17_DEFAULT;	// reset to default for this band
		//
		ts.pwr_17m_full_adj = value;
		//printf("-->17 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 15 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND6_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_15_DEFAULT;	// reset to default for this band
		//
		ts.pwr_15m_full_adj = value;
		//printf("-->15 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 12 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND7_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_12_DEFAULT;	// reset to default for this band
		//
		ts.pwr_12m_full_adj = value;
		//printf("-->12 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10 meter FULL power calibration setting
	if(Read_EEPROM(EEPROM_BAND8_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN) || ts.load_eeprom_defaults)	// if out of range of power setting, it was bogus (or load default value)
			value = TX_POWER_FACTOR_10_DEFAULT;	// reset to default for this band
		//
		ts.pwr_10m_full_adj = value;
		//printf("-->10 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read spectrum scope magnify setting
	if(Read_EEPROM(EEPROM_SPECTRUM_MAGNIFY, &value) == 0)
	{
		if((value > 1) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		sd.magnify = value;
		//printf("-->spectrum scope magnify setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read CW wide filter disable setting
	if(Read_EEPROM(EEPROM_WIDE_FILT_CW_DISABLE, &value) == 0)
	{
		if((value > 1) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		ts.filter_cw_wide_disable = value;
		//printf("-->CW wide filter disable setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read SSB Narrow filter disable setting
	if(Read_EEPROM(EEPROM_NARROW_FILT_SSB_DISABLE, &value) == 0)
	{
		if((value > 1) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		ts.filter_ssb_narrow_disable = value;
		//printf("-->SSB Narrow disable setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read AM Mode disable setting
	if(Read_EEPROM(EEPROM_AM_MODE_DISABLE, &value) == 0)
	{
		if((value > 1) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		ts.am_mode_disable = value;
		//printf("-->SSB Narrow disable setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope dB/Division
	if(Read_EEPROM(EEPROM_SPECTRUM_DB_DIV, &value) == 0)
	{
		if((value > DB_DIV_ADJUST_MAX) || (value < DB_DIV_ADJUST_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = DB_DIV_ADJUST_DEFAULT;	// reset to default
		//
		ts.spectrum_db_scale = value;
		//printf("-->Spectrum Scope dB/Division loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope AGC rate
	if(Read_EEPROM(EEPROM_SPECTRUM_AGC_RATE, &value) == 0)
	{
		if((value > SPECTRUM_SCOPE_AGC_MAX) || (value < SPECTRUM_SCOPE_AGC_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = SPECTRUM_SCOPE_AGC_DEFAULT;	// reset to default
		//
		ts.scope_agc_rate = value;
		//printf("-->Spectrum Scope AGC rate loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read meter mode
	if(Read_EEPROM(EEPROM_METER_MODE, &value) == 0)
	{
		if((value >= METER_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = METER_SWR;	// reset to default
		//
		ts.tx_meter_mode = value;
		//printf("-->Meter mode loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX audio compressor setting
	if(Read_EEPROM(EEPROM_TX_AUDIO_COMPRESS, &value) == 0)
	{
		if((value > TX_AUDIO_COMPRESSION_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = TX_AUDIO_COMPRESSION_DEFAULT;	// reset to default
		//
		ts.tx_comp_level = value;
		//printf("-->TX audio compressor setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read ALC release (decay) time
	if(Read_EEPROM(EEPROM_ALC_DECAY_TIME, &value) == 0)
	{
		if((value > ALC_DECAY_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = ALC_DECAY_DEFAULT;	// reset to default
		//
		ts.alc_decay = value;		// "reserve" copy of variable
		ts.alc_decay_var = value;	// "working" copy of variable
		//printf("-->ALC release (decay) time loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX audio post-filter gain setting
	if(Read_EEPROM(EEPROM_ALC_POSTFILT_TX_GAIN, &value) == 0)
	{
		if((value > ALC_POSTFILT_GAIN_MAX) || (value < ALC_POSTFILT_GAIN_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = ALC_POSTFILT_GAIN_DEFAULT;	// reset to default
		//
		ts.alc_tx_postfilt_gain = value;		// "reserve" copy of variable
		ts.alc_tx_postfilt_gain_var = value;	// "working" copy of variable
		//printf("-->TX audio post-filter gain setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Frequency step line/button configuration setting
	if(Read_EEPROM(EEPROM_STEP_SIZE_CONFIG, &value) == 0)
	{
		if((value > 255) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default for
		//
		ts.freq_step_config = value;
		//printf("-->Frequency step marker line setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP configuration setting
	if(Read_EEPROM(EEPROM_DSP_MODE, &value) == 0)
	{
		if((value > 255) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		ts.dsp_active = value;
		//printf("-->DSP configuration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP Noise reduction setting
	if(Read_EEPROM(EEPROM_DSP_NR_STRENGTH, &value) == 0)
	{
		if((value > DSP_NR_STRENGTH_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = DSP_NR_STRENGTH_DEFAULT;	// reset to default
		//
		ts.dsp_nr_strength = value;
		//printf("-->DSP Noise reduction setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP Noise reduction de-correlator buffer length setting
	if(Read_EEPROM(EEPROM_DSP_NR_DECOR_BUFLEN, &value) == 0)
	{
		if((value > DSP_NR_BUFLEN_MAX) || (value < DSP_NR_BUFLEN_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = DSP_NR_BUFLEN_DEFAULT;	// reset to default
		//
		ts.dsp_nr_delaybuf_len = value;
		//printf("-->DSP Noise reduction de-correlator buffer length setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP Noise reduction FFT number of taps setting
	if(Read_EEPROM(EEPROM_DSP_NR_FFT_NUMTAPS, &value) == 0)
	{
		if((value > DSP_NR_NUMTAPS_MAX) || (value < DSP_NR_NUMTAPS_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = DSP_NR_NUMTAPS_DEFAULT;	// reset to default
		//
		ts.dsp_nr_numtaps = value;
		//printf("-->DSP Noise reduction FFT number of taps setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP Notch de-correlator buffer length setting
	if(Read_EEPROM(EEPROM_DSP_NOTCH_DECOR_BUFLEN, &value) == 0)
	{
		if((value > DSP_NOTCH_BUFLEN_MAX) || (value < DSP_NOTCH_BUFLEN_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = DSP_NOTCH_DELAYBUF_DEFAULT;	// reset to default
		//
		ts.dsp_notch_delaybuf_len = value;
		//printf("-->DSP Notch de-correlator buffer setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP Notch number of taps setting
	if(Read_EEPROM(EEPROM_DSP_NOTCH_FFT_NUMTAPS, &value) == 0)
	{
		if((value > DSP_NR_NUMTAPS_MAX) || (value < DSP_NR_NUMTAPS_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = DSP_NR_NUMTAPS_DEFAULT;	// reset to default
		//
		ts.dsp_notch_numtaps = value;
		//printf("-->DSP Notch number of taps setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP Notch convergence rate length setting
	if(Read_EEPROM(EEPROM_DSP_NOTCH_CONV_RATE, &value) == 0)
	{
		if((value > DSP_NOTCH_MU_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = DSP_NOTCH_MU_DEFAULT;	// reset to default
		//
		ts.dsp_notch_mu = value;
		//printf("-->DSP Notch convergence rate setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read maximum RF gain setting
	if(Read_EEPROM(EEPROM_MAX_RX_GAIN, &value) == 0)
	{
		if((value > MAX_RF_GAIN_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = MAX_RF_GAIN_DEFAULT;	// reset to default
		//
		ts.max_rf_gain = value;
		//printf("-->maximum RF gain setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX disable setting
	if(Read_EEPROM(EEPROM_TX_DISABLE, &value) == 0)
	{
		if((value > 1) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		ts.tx_disable = value;
		//printf("-->TX disable setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Misc. flags 1 setting
	if(Read_EEPROM(EEPROM_MISC_FLAGS1, &value) == 0)
	{
		if((value > 255) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		ts.misc_flags1 = value;
		//printf("-->Misc. flags 1 setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Misc. flags 2 setting
	if(Read_EEPROM(EEPROM_MISC_FLAGS2, &value) == 0)
	{
		if((value > 255) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		ts.misc_flags2 = value;
		//printf("-->Misc. flags 2 setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read "minor" version number
	if(Read_EEPROM(EEPROM_VERSION_MINOR, &value) == 0)
	{
		if(value > 255)	// if out of range, it was bogus (default loading not appropriate here!)
			value = 0;	// reset to default
		//
		ts.version_number_minor = value;
		//printf("-->'Minor' version number loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read version (release) number
	if(Read_EEPROM(EEPROM_VERSION_NUMBER, &value) == 0)
	{
		if(value > 255)	// if out of range, it was bogus (default loading not appropriate here!)
			value = 0;	// reset to default
		//
		ts.version_number_release = value;
		//printf("-->Version (release) number loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read version (build) number
	if(Read_EEPROM(EEPROM_VERSION_BUILD, &value) == 0)
	{
		if(value > 255)	// if out of range, it was bogus  (default loading not appropriate here!
			value = 0;	// reset to default
		//
		ts.version_number_build = value;
		//printf("-->Version (build) number loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Noise blanker AGC setting
	if(Read_EEPROM(EEPROM_NB_AGC_TIME_CONST, &value) == 0)
	{
		if((value > NB_MAX_AGC_SETTING) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = 0;	// reset to default
		//
		ts.nb_agc_time_const = value;
		//printf("-->Noise blanker AGC setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read CW offset setting
	if(Read_EEPROM(EEPROM_CW_OFFSET_MODE, &value) == 0)
	{
		if((value > CW_OFFSET_MAX) || ts.load_eeprom_defaults)				// if out of range, it was bogus (or load default value)
			value = CW_OFFSET_MODE_DEFAULT;		// reset to default
		//
		ts.cw_offset_mode = value;
		//printf("-->CW offset loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read I/Q Freq. conversion setting
	if(Read_EEPROM(EEPROM_FREQ_CONV_MODE, &value) == 0)
	{
		if((value > FREQ_IQ_CONV_MODE_MAX) || ts.load_eeprom_defaults)				// if out of range, it was bogus (or load default value)
			value = FREQ_IQ_CONV_MODE_DEFAULT;		// reset to default
		//
		ts.iq_freq_mode = value;
		//printf("-->I/Q Freq. conversion loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read auto LSB/USB select mode
	if(Read_EEPROM(EEPROM_LSB_USB_AUTO_SELECT, &value) == 0)
	{
		if((value > AUTO_LSB_USB_MAX) || ts.load_eeprom_defaults)				// if out of range, it was bogus (or load default value)
			value = AUTO_LSB_USB_DEFAULT;		// reset to default
		//
		ts.lsb_usb_auto_select = value;
		//printf("-->LSB/USB select mode loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read auto LCD backlight blanking mode
	if(Read_EEPROM(EEPROM_LCD_BLANKING_CONFIG, &value) == 0)
	{
		if((value > 255) || ts.load_eeprom_defaults)				// if out of range, it was bogus (or load default value)
			value = 0;		// reset to OFF
		//
		ts.lcd_backlight_blanking = value;
		//printf("-->auto LCD backlight blanking mode loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read VFO/SPLIT/Memory mode
	if(Read_EEPROM(EEPROM_VFO_MEM_MODE, &value) == 0)
	{
		if((value > 255) || ts.load_eeprom_defaults)				// if out of range, it was bogus (or load default value)
			value = 0;		// reset to OFF
		//
		ts.vfo_mem_mode = value;
		//printf("-->VFO/SPLIT/Memory mode loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power sensor coupling coefficient for 80m
	if(Read_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_80M, &value) == 0)
	{
		if((value > SWR_COUPLING_MAX) || (value < SWR_COUPLING_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = SWR_COUPLING_DEFAULT;		// reset to OFF
		//
		swrm.coupling_80m_calc = value;
		//printf("-->Power sensor coupling coefficient for 80m loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power sensor coupling coefficient for 40m
	if(Read_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_40M, &value) == 0)
	{
		if((value > SWR_COUPLING_MAX) || (value < SWR_COUPLING_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = SWR_COUPLING_DEFAULT;		// reset to OFF
		//
		swrm.coupling_40m_calc = value;
		//printf("-->Power sensor coupling coefficient for 40m loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power sensor coupling coefficient for 20m
	if(Read_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_20M, &value) == 0)
	{
		if((value > SWR_COUPLING_MAX) || (value < SWR_COUPLING_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = SWR_COUPLING_DEFAULT;		// reset to OFF
		//
		swrm.coupling_20m_calc = value;
		//printf("-->Power sensor coupling coefficient for 20m loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power sensor coupling coefficient for 15m
	if(Read_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_15M, &value) == 0)
	{
		if((value > SWR_COUPLING_MAX) || (value < SWR_COUPLING_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = SWR_COUPLING_DEFAULT;		// reset to OFF
		//
		swrm.coupling_15m_calc = value;
		//printf("-->Power sensor coupling coefficient for 15m loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read voltmeter calibration
	if(Read_EEPROM(EEPROM_VOLTMETER_CALIBRATE, &value) == 0)
	{
		if((value > POWER_VOLTMETER_CALIBRATE_MAX) || (value < POWER_VOLTMETER_CALIBRATE_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = POWER_VOLTMETER_CALIBRATE_DEFAULT;		// reset to OFF
		//
		ts.voltmeter_calibrate = value;
		//printf("-->Voltmeter calibration loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read waterfall color scheme
	if(Read_EEPROM(EEPROM_WATERFALL_COLOR_SCHEME, &value) == 0)
	{
		if((value > WATERFALL_COLOR_MAX) || (value < WATERFALL_COLOR_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = WATERFALL_COLOR_DEFAULT;		// reset to OFF
		//
		ts.waterfall_color_scheme = value;
		//printf("-->waterfall color scheme loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read waterfall vertical step size
	if(Read_EEPROM(EEPROM_WATERFALL_VERTICAL_STEP_SIZE, &value) == 0)
	{
		if((value > WATERFALL_STEP_SIZE_MAX) || (value < WATERFALL_STEP_SIZE_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = WATERFALL_STEP_SIZE_DEFAULT;		// reset to OFF
		//
		ts.waterfall_vert_step_size = value;
		//printf("-->waterfall vertical step size loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read waterfall amplitude offset
	if(Read_EEPROM(EEPROM_WATERFALL_OFFSET, &value) == 0)
	{
		if((value > WATERFALL_OFFSET_MAX) || (value < WATERFALL_OFFSET_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = WATERFALL_OFFSET_DEFAULT;		// reset to OFF
		//
		ts.waterfall_offset = value;
		//printf("-->waterfall amplitude offset loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read waterfall amplitude contrast
	if(Read_EEPROM(EEPROM_WATERFALL_CONTRAST, &value) == 0)
	{
		if((value > WATERFALL_CONTRAST_MAX) || (value < WATERFALL_CONTRAST_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = WATERFALL_CONTRAST_DEFAULT;		// reset to OFF
		//
		ts.waterfall_contrast = value;
		//printf("-->waterfall amplitude contrast loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read waterfall speed
	if(Read_EEPROM(EEPROM_WATERFALL_SPEED, &value) == 0)
	{
		if((value > WATERFALL_SPEED_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = WATERFALL_SPEED_DEFAULT_SPI;		// reset to default
		//
		ts.waterfall_speed = value;
		//printf("-->waterfall speed loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read spectrum scope no signal auto offset
	if(Read_EEPROM(EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST, &value) == 0)	{
		if((value > SPECTRUM_SCOPE_NOSIG_ADJUST_MAX) || (value < SPECTRUM_SCOPE_NOSIG_ADJUST_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT;		// reset to default
		//
		ts.spectrum_scope_nosig_adjust = value;
		//printf("-->spectrum scope no signal auto offset loaded\n\r");
	}
//
// ------------------------------------------------------------------------------------
// Try to read waterfall no signal auto offset
	if(Read_EEPROM(EEPROM_WATERFALL_NOSIG_ADJUST, &value) == 0)	{
		if((value > WATERFALL_NOSIG_ADJUST_MAX) || (value < WATERFALL_NOSIG_ADJUST_MIN) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = WATERFALL_NOSIG_ADJUST_DEFAULT;		// reset to default
		//
	ts.waterfall_nosig_adjust = value;
	//printf("-->waterfall no signal auto offset loaded\n\r");
	//
	// Next setting...
	}

	//
	// ------------------------------------------------------------------------------------
// Try to read waterfall size and other settings
	if(Read_EEPROM(EEPROM_WATERFALL_SIZE, &value) == 0)	{
		if((value >= WATERFALL_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = WATERFALL_SIZE_DEFAULT;		// reset to default
		//
		ts.waterfall_size = value;
	//printf("--waterfall size and other settings loaded\n\r");
	//
	// Next setting...
	}
//
// ------------------------------------------------------------------------------------
// Try to read FFT Window setting
	if(Read_EEPROM(EEPROM_FFT_WINDOW, &value) == 0)	{
		if((value >= FFT_WINDOW_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
			value = FFT_WINDOW_DEFAULT;		// reset to default
		//
		ts.fft_window_type = value;
	//printf("--FFT Window setting loaded\n\r");
	//
	}
	// Next setting...
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX PTT audio mute delay setting
		if(Read_EEPROM(EEPROM_TX_PTT_AUDIO_MUTE, &value) == 0)	{
			if((value >= TX_PTT_AUDIO_MUTE_DELAY_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = 0;		// reset to default
			//
			ts.tx_audio_muting_timing = value;
		//printf("--TX PTT audio mute delay setting loaded\n\r");
		//
		}
	// Next setting...
//
	// ------------------------------------------------------------------------------------
	// Try to read Filter Bandwidth Display setting
		if(Read_EEPROM(EEPROM_FILTER_DISP_COLOUR, &value) == 0)	{
			if((value > SPEC_MAX_COLOUR) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = FILTER_DISP_COLOUR_DEFAULT;	// reset to default
			//
			ts.filter_disp_colour = value;
		//printf("--Filter Bandwidth Display setting loaded\n\r");
		//
		}
	// Next setting...
	//
	// ------------------------------------------------------------------------------------
	// Try to read FM Subaudible Tone generate setting
		if(Read_EEPROM(EEPROM_FM_SUBAUDIBLE_TONE_GEN, &value) == 0)	{
			if((value >= NUM_SUBAUDIBLE_TONES) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = FM_SUBAUDIBLE_TONE_OFF;	// reset to default
			//
			ts.fm_subaudible_tone_gen_select = value;
		//printf("--FM Subaudible Tone generate setting loaded\n\r");
		//
		}
	// Next setting...
		//
		// ------------------------------------------------------------------------------------
	// Try to read FM Subaudible Tone detect setting
		if(Read_EEPROM(EEPROM_FM_SUBAUDIBLE_TONE_DET, &value) == 0)	{
			if((value >= NUM_SUBAUDIBLE_TONES) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = FM_SUBAUDIBLE_TONE_OFF;	// reset to default
			//
			ts.fm_subaudible_tone_det_select = value;
		//printf("--FM Subaudible Tone detect setting loaded\n\r");
		//
		}
	// Next setting...
	//
	// ------------------------------------------------------------------------------------
	// Try to read FM Tone burst setting
		if(Read_EEPROM(EEPROM_FM_TONE_BURST_MODE, &value) == 0)	{
			if((value > FM_TONE_BURST_MAX)	|| ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = FM_TONE_BURST_OFF;	// reset to default
			//
			ts.fm_tone_burst_mode = value;
		//printf("--FM Tone burst setting loaded\n\r");
		//
		}
	// Next setting...
	//
	// ------------------------------------------------------------------------------------
	// Try to read FM Squelch Threshold setting
		if(Read_EEPROM(EEPROM_FM_SQUELCH_SETTING, &value) == 0)	{
			if((value > FM_SQUELCH_MAX)	|| ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = FM_SQUELCH_DEFAULT;	// reset to default
			//
			ts.fm_sql_threshold = value;
		//printf("--FM Squelch Threshold setting loaded\n\r");
		//
		}
	// Next setting...
	//
	// ------------------------------------------------------------------------------------
	// Try to read FM RX bandwidth setting
		if(Read_EEPROM(EEPROM_FM_RX_BANDWIDTH, &value) == 0)	{
			if((value > FM_RX_BANDWIDTH_MAX) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = FM_BANDWIDTH_DEFAULT;	// reset to default
			//
			ts.fm_rx_bandwidth = value;
		//printf("--FM RX bandwidth setting loaded\n\r");
		//
		}
	// Next setting...
	//
	// ------------------------------------------------------------------------------------
	// Try to read Keyboard beep frequency setting
		if(Read_EEPROM(EEPROM_KEYBOARD_BEEP_FREQ, &value) == 0)	{
			if((value > MAX_BEEP_FREQUENCY) || (value < MIN_BEEP_FREQUENCY) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = DEFAULT_BEEP_FREQUENCY;	// reset to default
			//
			ts.beep_frequency = value;
		//printf("--Keyboard beep frequency setting loaded\n\r");
		//
		}
	// Next setting...
	//
	//
	// ------------------------------------------------------------------------------------
	// Try to read Beep loudness setting
		if(Read_EEPROM(EEPROM_BEEP_LOUDNESS, &value) == 0)	{
			if((value > MAX_BEEP_LOUDNESS) || ts.load_eeprom_defaults)	// if out of range, it was bogus (or load default value)
				value = DEFAULT_BEEP_LOUDNESS;	// reset to default
			//
			ts.beep_loudness = value;
		//printf("--Beep loudness setting loaded\n\r");
		//
		}
	// Next setting...
	//
}

//
// Below is a marker to make it easier to find the "Read" and "Save" EEPROM functions when scanning/scrolling the source code
//
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
// ********************************************************************************************************************
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSaveEepromValues
//* Object              : save all values to EEPROM - called on power-down.  Does not check to see if they have changed
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiDriverSaveEepromValuesPowerDown(void)
{
	uint16_t value,value1, i;

	if(ts.txrx_mode != TRX_MODE_RX)
		return;

	//printf("eeprom save activate\n\r");

if(ts.ser_eeprom_in_use == 0)
    {
    static uint8_t p[MAX_VAR_ADDR*2+2];
    ts.eeprombuf = p;
    uint16_t i, data;
    
    ts.eeprombuf[0] = Read_24Cxx(0,ts.ser_eeprom_type);
    ts.eeprombuf[1] = Read_24Cxx(1,ts.ser_eeprom_in_use);
    for(i=1; i<=MAX_VAR_ADDR; i++)
	{
	Read_SerEEPROM(i, &data);
	ts.eeprombuf[i*2+1] = (uint8_t)((0x00FF)&data);
	data = data>>8;
	ts.eeprombuf[i*2] = (uint8_t)((0x00FF)&data);
	}
    ts.ser_eeprom_in_use = 0xAA;
    // If serial EEPROM is in use copy all data first to memory
    // do there all compares and additions and after finishing that
    // process write complete block to serial EEPROM. Flag for this is
    // ser_eeprom_in_use == 0xAA
    }
	// ------------------------------------------------------------------------------------
	// Read Band and Mode saved values - update if changed
	if(Read_EEPROM(EEPROM_BAND_MODE, &value) == 0)
	{
		ushort new;

		new 	= 0;
		new 	= ts.band;
		new	   |= (ts.dmod_mode << 8);
		new	   |= (ts.filter_id << 12);
		Write_EEPROM(EEPROM_BAND_MODE, new);
		//printf("-->band and mode saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND_MODE,(ts.band |(ts.dmod_mode & 0x0f << 8) | (ts.filter_id << 12) ));
		//printf("-->band and mode var created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Freq saved values - update if changed
	if(	(Read_EEPROM(EEPROM_FREQ_HIGH, &value) == 0) && (Read_EEPROM(EEPROM_FREQ_LOW, &value1) == 0)) {
		Write_EEPROM(EEPROM_FREQ_HIGH,(df.tune_new >> 16));
		//printf("-->freq high saved\n\r");
		Write_EEPROM(EEPROM_FREQ_LOW,(df.tune_new & 0xFFFF));
		//printf("-->freq low saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FREQ_HIGH,(df.tune_new >> 16));
		Write_EEPROM(EEPROM_FREQ_LOW,(df.tune_new & 0xFFFF));
		//printf("-->freq vars created\n\r");
	}
	//
	// save current band/frequency/mode settings
	//
	// save frequency
	band_dial_value[ts.band] = df.tune_new;
	// Save decode mode
	band_decod_mode[ts.band] = ts.dmod_mode;
	// Save filter setting
	band_filter_mode[ts.band] = ts.filter_id;
	//
	// Save stored band/mode/frequency memory from RAM
	//
	for(i = 0; i < MAX_BANDS; i++)	{	// scan through each band's frequency/mode data
		// ------------------------------------------------------------------------------------
		// Read Band and Mode saved values - update if changed
		if(Read_EEPROM(EEPROM_BAND0_MODE + i, &value) == 0)
		{
			ushort new;
			new 	= 0;

			// We do NOT check the band stored in the bottom byte as we have, by definition, saved that band at this index.
			//
			new	   |= (band_decod_mode[i] << 8);
			new	   |= (band_filter_mode[i] << 12);
			Write_EEPROM(EEPROM_BAND0_MODE + i, new);
		}
		else	// create
		{
			Write_EEPROM(EEPROM_BAND0_MODE + i,(((band_decod_mode[i] & 0x0f) << 8) | (band_filter_mode[i] << 12) ));
			//printf("-->band and mode var created\n\r");
		}
		//
		// Try to read Freq saved values - update if changed
		//
		if((Read_EEPROM(EEPROM_BAND0_FREQ_HIGH + i, &value) == 0) && (Read_EEPROM(EEPROM_BAND0_FREQ_LOW + i, &value1) == 0))	{
			Write_EEPROM(EEPROM_BAND0_FREQ_HIGH + i,(band_dial_value[i] >> 16));
			//printf("-->freq high saved\n\r");
			Write_EEPROM(EEPROM_BAND0_FREQ_LOW + i,(band_dial_value[i] & 0xFFFF));
			//printf("-->freq low saved\n\r");
		}
		else	// create
		{
			Write_EEPROM(EEPROM_BAND0_FREQ_HIGH + i,(band_dial_value[i] >> 16));
			Write_EEPROM(EEPROM_BAND0_FREQ_LOW + i,(band_dial_value[i] & 0xFFFF));
			//printf("-->freq vars created\n\r");
		}
	}
	//
	// Save data for VFO A
	//
	for(i = 0; i < MAX_BANDS; i++)	{	// scan through each band's frequency/mode data
		// ------------------------------------------------------------------------------------
		// Read Band and Mode saved values - update if changed
		if(Read_EEPROM(EEPROM_BAND0_MODE_A + i, &value) == 0)
		{
			ushort new;
			new 	= 0;

			// We do NOT check the band stored in the bottom byte as we have, by definition, saved that band at this index.
			//
			new	   |= (band_decod_mode_a[i] << 8);
			new	   |= (band_filter_mode_a[i] << 12);
			Write_EEPROM(EEPROM_BAND0_MODE_A + i, new);
		}
		else	// create
		{
			Write_EEPROM(EEPROM_BAND0_MODE_A + i,(((band_decod_mode[i] & 0x0f) << 8) | (band_filter_mode[i] << 12) ));	// use generic band value when creating
			//printf("-->band and mode var created\n\r");
		}
		//
		// Try to read Freq saved values - update if changed
		//
		if((Read_EEPROM(EEPROM_BAND0_FREQ_HIGH_A + i, &value) == 0) && (Read_EEPROM(EEPROM_BAND0_FREQ_LOW_A + i, &value1) == 0))	{
			//
			Write_EEPROM(EEPROM_BAND0_FREQ_HIGH_A + i,(band_dial_value_a[i] >> 16));
			//printf("-->freq high saved\n\r");
			Write_EEPROM(EEPROM_BAND0_FREQ_LOW_A + i,(band_dial_value_a[i] & 0xFFFF));
			//printf("-->freq low saved\n\r");
		}
		else	// create
		{
			Write_EEPROM(EEPROM_BAND0_FREQ_HIGH_A + i,(band_dial_value[i] >> 16));		// use generic band value when creating
			Write_EEPROM(EEPROM_BAND0_FREQ_LOW_A + i,(band_dial_value[i] & 0xFFFF));
			//printf("-->freq vars created\n\r");
		}
	}
	//
	// Save data for VFO B
	//
	for(i = 0; i < MAX_BANDS; i++)	{	// scan through each band's frequency/mode data
		// ------------------------------------------------------------------------------------
		// Read Band and Mode saved values - update if changed
		if(Read_EEPROM(EEPROM_BAND0_MODE_B + i, &value) == 0)
		{
			ushort new;
			new 	= 0;

			// We do NOT check the band stored in the bottom byte as we have, by definition, saved that band at this index.
			//
			new	   |= (band_decod_mode_b[i] << 8);
			new	   |= (band_filter_mode_b[i] << 12);
			Write_EEPROM(EEPROM_BAND0_MODE_B + i, new);
		}
		else	// create
		{
			Write_EEPROM(EEPROM_BAND0_MODE_B + i,(((band_decod_mode[i] & 0x0f) << 8) | (band_filter_mode[i] << 12) ));	// use generic band value when creating
			//printf("-->band and mode var created\n\r");
		}
		//
		// Try to read Freq saved values - update if changed
		//
		if((Read_EEPROM(EEPROM_BAND0_FREQ_HIGH_B + i, &value) == 0) && (Read_EEPROM(EEPROM_BAND0_FREQ_LOW_B + i, &value1) == 0))	{
			//
			Write_EEPROM(EEPROM_BAND0_FREQ_HIGH_B + i,(band_dial_value_b[i] >> 16));
			//printf("-->freq high saved\n\r");
			Write_EEPROM(EEPROM_BAND0_FREQ_LOW_B + i,(band_dial_value_b[i] & 0xFFFF));
			//printf("-->freq low saved\n\r");
		}
		else	// create
		{
			Write_EEPROM(EEPROM_BAND0_FREQ_HIGH_B + i,(band_dial_value[i] >> 16));		// use generic band value when creating
			Write_EEPROM(EEPROM_BAND0_FREQ_LOW_B + i,(band_dial_value[i] & 0xFFFF));
			//printf("-->freq vars created\n\r");
		}
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Step saved values - update if changed
	if(Read_EEPROM(EEPROM_FREQ_STEP, &value) == 0)
	{
		Write_EEPROM(EEPROM_FREQ_STEP,df.selected_idx);
		//printf("-->freq step saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FREQ_STEP,3);
		//printf("-->freq step created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TX Audio Source saved values - update if changed
	if(Read_EEPROM(EEPROM_TX_AUDIO_SRC, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_AUDIO_SRC,ts.tx_audio_source);
		//printf("-->TX audio source saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_AUDIO_SRC,0);
		//printf("-->TX audio source created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TCXO saved values - update if changed
	if(Read_EEPROM(EEPROM_TCXO_STATE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TCXO_STATE,df.temp_enabled);
		//printf("-->TCXO state saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TCXO_STATE,0);
		//printf("-->TCXO state created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Audio Gain saved values - update if changed
	if(Read_EEPROM(EEPROM_AUDIO_GAIN, &value) == 0)
	{
		Write_EEPROM(EEPROM_AUDIO_GAIN,ts.audio_gain);
		//printf("-->Audio Gain saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_AUDIO_GAIN,DEFAULT_AUDIO_GAIN);
		//printf("-->Audio Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RF Codec Gain saved values - update if changed
	if(Read_EEPROM(EEPROM_RX_CODEC_GAIN, &value) == 0)
	{
		Write_EEPROM(EEPROM_RX_CODEC_GAIN,ts.rf_codec_gain);
		//printf("-->RF Codec Gain saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_RX_CODEC_GAIN,DEFAULT_RF_CODEC_GAIN_VAL);
		//printf("-->RF Codec Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RF Gain saved values - update if changed
	if(Read_EEPROM(EEPROM_RX_GAIN, &value) == 0)
	{
		Write_EEPROM(EEPROM_RX_GAIN,ts.rf_gain);
		//printf("-->RF Gain saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_RX_GAIN,DEFAULT_RF_GAIN);
		//printf("-->RF Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Noise Blanker saved values - update if changed
	if(Read_EEPROM(EEPROM_NB_SETTING, &value) == 0)
	{
		Write_EEPROM(EEPROM_NB_SETTING,ts.nb_setting);
		//printf("-->Attenuator saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_NB_SETTING,0);
		//printf("-->Attenuator created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power level saved values - update if changed
	if(Read_EEPROM(EEPROM_TX_POWER_LEVEL, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_POWER_LEVEL,ts.power_level);
		//printf("-->power level saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_POWER_LEVEL,PA_LEVEL_DEFAULT);
		//printf("-->power level created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Keyer speed saved values - update if changed
	if(Read_EEPROM(EEPROM_KEYER_SPEED, &value) == 0)
	{
		Write_EEPROM(EEPROM_KEYER_SPEED,ts.keyer_speed);
		//printf("-->Keyer speed saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_KEYER_SPEED,DEFAULT_KEYER_SPEED);
		//printf("-->Keyer speed created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Keyer mode saved values - update if changed
	if(Read_EEPROM(EEPROM_KEYER_MODE, &value) == 0)
	{
		Write_EEPROM(EEPROM_KEYER_MODE,ts.keyer_mode);
		//printf("-->Keyer mode saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_KEYER_MODE,CW_MODE_IAM_B);
		//printf("-->Keyer mode created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Gain saved values - update if changed
	if(Read_EEPROM(EEPROM_SIDETONE_GAIN, &value) == 0)
	{
		Write_EEPROM(EEPROM_SIDETONE_GAIN,ts.st_gain);
		//printf("-->Sidetone Gain saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SIDETONE_GAIN,DEFAULT_SIDETONE_GAIN);
		//printf("-->Sidetone Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Frequency Calibration saved values - update if changed
	if(Read_EEPROM(EEPROM_FREQ_CAL, &value) == 0)
	{
		Write_EEPROM(EEPROM_FREQ_CAL,ts.freq_cal);
		//printf("-->Frequency Calibration saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FREQ_CAL,0);
		//printf("-->Frequency Calibration\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read AGC Mode saved values - update if changed
	if(Read_EEPROM(EEPROM_AGC_MODE, &value) == 0)
	{
		Write_EEPROM(EEPROM_AGC_MODE,ts.agc_mode);
		//printf("-->AGC mode saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_AGC_MODE,AGC_DEFAULT);
		//printf("-->AGC mode created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Microphone Gain saved values - update if changed
	if(Read_EEPROM(EEPROM_MIC_GAIN, &value) == 0)
	{
		Write_EEPROM(EEPROM_MIC_GAIN,ts.tx_mic_gain);
		//printf("-->Microphone Gain saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_MIC_GAIN,MIC_GAIN_DEFAULT);
		//printf("-->Microphone Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Line Gain saved values - update if changed
	if(Read_EEPROM(EEPROM_LINE_GAIN, &value) == 0)
	{
		Write_EEPROM(EEPROM_LINE_GAIN,ts.tx_line_gain);
		//printf("-->Line Gain saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_LINE_GAIN,LINE_GAIN_DEFAULT);
		//printf("-->Line Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Frequency saved values - update if changed
	if(Read_EEPROM(EEPROM_SIDETONE_FREQ, &value) == 0)
	{
		Write_EEPROM(EEPROM_SIDETONE_FREQ,ts.sidetone_freq);
		//printf("-->Sidetone Freq saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SIDETONE_FREQ,CW_SIDETONE_FREQ_DEFAULT);
		//printf("-->Sidetone Freq created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope Speed saved values - update if changed
	if(Read_EEPROM(EEPROM_SPEC_SCOPE_SPEED, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPEC_SCOPE_SPEED,ts.scope_speed);
		//printf("-->Spectrum Scope Speed saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPEC_SCOPE_SPEED,SPECTRUM_SCOPE_SPEED_DEFAULT);
		//printf("-->Spectrum Scope Speed created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope Filter Strength saved values - update if changed
	if(Read_EEPROM(EEPROM_SPEC_SCOPE_FILTER, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPEC_SCOPE_FILTER,ts.scope_filter);
		//printf("-->Spectrum Scope Filter saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPEC_SCOPE_FILTER,SPECTRUM_SCOPE_FILTER_DEFAULT);
		//printf("-->Spectrum Scope Speed created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read AGC Custom Decay saved values - update if changed
	if(Read_EEPROM(EEPROM_AGC_CUSTOM_DECAY, &value) == 0)
	{
		Write_EEPROM(EEPROM_AGC_CUSTOM_DECAY,ts.agc_custom_decay);
		//printf("-->AGC Custom Decay value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_AGC_CUSTOM_DECAY,AGC_CUSTOM_DEFAULT);
		//printf("-->AGC Custom Decay value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Scope trace color saved values - update if changed
	if(Read_EEPROM(EEPROM_SPECTRUM_TRACE_COLOUR, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPECTRUM_TRACE_COLOUR,ts.scope_trace_colour);
		//printf("-->Scope Trace Color value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPECTRUM_TRACE_COLOUR,SPEC_COLOUR_TRACE_DEFAULT);
		//printf("-->Scope Trace Color value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Scope grid color saved values - update if changed
	if(Read_EEPROM(EEPROM_SPECTRUM_GRID_COLOUR, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPECTRUM_GRID_COLOUR,ts.scope_grid_colour);
		//printf("-->Scope Grid Color value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPECTRUM_GRID_COLOUR,SPEC_COLOUR_GRID_DEFAULT);
		//printf("-->Scope Grid Color value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Scope centre line grid color saved values - update if changed
	if(Read_EEPROM(EEPROM_SPECTRUM_CENTRE_GRID_COLOUR, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPECTRUM_CENTRE_GRID_COLOUR,ts.scope_centre_grid_colour);
		//printf("-->Scope Grid Center Line Color value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPECTRUM_CENTRE_GRID_COLOUR,SPEC_COLOUR_GRID_DEFAULT);
		//printf("-->Scope Grid Center Line Color value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Scope scale color saved values - update if changed
	if(Read_EEPROM(EEPROM_SPECTRUM_SCALE_COLOUR, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPECTRUM_SCALE_COLOUR,ts.scope_scale_colour);
		//printf("-->Scope Scale Color value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPECTRUM_SCALE_COLOUR,SPEC_COLOUR_SCALE_DEFAULT);
		//printf("-->Scope Scale Color value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Paddle Reversal saved values - update if changed
	if(Read_EEPROM(EEPROM_PADDLE_REVERSE, &value) == 0)
	{
		Write_EEPROM(EEPROM_PADDLE_REVERSE,ts.paddle_reverse);
		//printf("-->Scope Paddle Reverse value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_PADDLE_REVERSE,0);
		//printf("-->Paddle Reverse value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read CW TX>RX Delay saved values - update if changed
	if(Read_EEPROM(EEPROM_CW_RX_DELAY, &value) == 0)
	{
		Write_EEPROM(EEPROM_CW_RX_DELAY,ts.cw_rx_delay);
		//printf("-->CW TX>RX Delay value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_CW_RX_DELAY,CW_RX_DELAY_DEFAULT);
		//printf("-->CW TX>RX Delay value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Max. volume saved values - update if changed
	if(Read_EEPROM(EEPROM_MAX_VOLUME, &value) == 0)
	{
		Write_EEPROM(EEPROM_MAX_VOLUME,ts.audio_max_volume);
		//printf("-->Max. volume value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_MAX_VOLUME,MAX_VOLUME_DEFAULT);
		//printf("-->Max. volume value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 300 Hz filter values - update if changed
	if(Read_EEPROM(EEPROM_FILTER_300HZ_SEL, &value) == 0)
	{
		Write_EEPROM(EEPROM_FILTER_300HZ_SEL,ts.filter_300Hz_select);
		//printf("-->300 Hz filter value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FILTER_300HZ_SEL,FILTER_300HZ_DEFAULT);
		//printf("-->300 Hz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 500 Hz filter values - update if changed
	if(Read_EEPROM(EEPROM_FILTER_500HZ_SEL, &value) == 0)
	{
		Write_EEPROM(EEPROM_FILTER_500HZ_SEL,ts.filter_500Hz_select);
		//printf("-->500 Hz filter value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FILTER_500HZ_SEL,FILTER_500HZ_DEFAULT);
		//printf("-->500 Hz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 1.8 kHz filter values - update if changed
	if(Read_EEPROM(EEPROM_FILTER_1K8_SEL, &value) == 0)
	{
		Write_EEPROM(EEPROM_FILTER_1K8_SEL,ts.filter_1k8_select);
		//printf("-->1.8 kHz filter value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FILTER_1K8_SEL,FILTER_1K8_DEFAULT);
		//printf("-->1.8 kHz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 2.3 kHz filter values - update if changed
	if(Read_EEPROM(EEPROM_FILTER_2K3_SEL, &value) == 0)
	{
		Write_EEPROM(EEPROM_FILTER_2K3_SEL,ts.filter_2k3_select);
		//printf("-->2.3 kHz filter value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FILTER_2K3_SEL,FILTER_2K3_DEFAULT);
		//printf("-->2.3 kHz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 3.6 kHz filter values - update if changed
	if(Read_EEPROM(EEPROM_FILTER_3K6_SEL, &value) == 0)
	{
		Write_EEPROM(EEPROM_FILTER_3K6_SEL,ts.filter_3k6_select);
		//printf("-->3.6 kHz filter value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FILTER_3K6_SEL,FILTER_3K6_DEFAULT);
		//printf("-->3.6 kHz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Wide filter values - update if changed
	if(Read_EEPROM(EEPROM_FILTER_WIDE_SEL, &value) == 0)
	{
		Write_EEPROM(EEPROM_FILTER_WIDE_SEL,ts.filter_wide_select);
		//printf("-->10 kHz filter value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FILTER_WIDE_SEL,FILTER_WIDE_DEFAULT);
		//printf("-->Wide filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read PA Bias values - update if changed
	if(Read_EEPROM(EEPROM_PA_BIAS, &value) == 0)
	{
		Write_EEPROM(EEPROM_PA_BIAS,ts.pa_bias);
		//printf("-->PA Bias value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_PA_BIAS,DEFAULT_PA_BIAS);
		//printf("-->PA Bias value created\n\r");
	}
	//
	//
	// ------------------------------------------------------------------------------------
	// Try to read PA CW Bias values - update if changed
	if(Read_EEPROM(EEPROM_PA_CW_BIAS, &value) == 0)
	{
		Write_EEPROM(EEPROM_PA_CW_BIAS,ts.pa_cw_bias);
		//printf("-->PA CW Bias value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_PA_CW_BIAS,0);
		//printf("-->PA CW Bias value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ LSB Gain Balance values - update if changed
	if(Read_EEPROM(EEPROM_TX_IQ_LSB_GAIN_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_IQ_LSB_GAIN_BALANCE, ts.tx_iq_lsb_gain_balance);
		//printf("-->TX IQ LSB Gain balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_IQ_LSB_GAIN_BALANCE,0);
		//printf("-->TX IQ LSB Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ USB Gain Balance values - update if changed
	if(Read_EEPROM(EEPROM_TX_IQ_USB_GAIN_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_IQ_USB_GAIN_BALANCE, ts.tx_iq_usb_gain_balance);
		//printf("-->TX IQ USB Gain balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_IQ_USB_GAIN_BALANCE,0);
		//printf("-->TX IQ USB Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ LSB Phase Balance values - update if changed
	if(Read_EEPROM(EEPROM_TX_IQ_LSB_PHASE_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_IQ_LSB_PHASE_BALANCE, ts.tx_iq_lsb_phase_balance);
		//printf("-->TX IQ LSB Phase balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_IQ_LSB_PHASE_BALANCE,0);
		//printf("-->TX IQ LSB Phase balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ USB Phase Balance values - update if changed
	if(Read_EEPROM(EEPROM_TX_IQ_USB_PHASE_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_IQ_USB_PHASE_BALANCE, ts.tx_iq_usb_phase_balance);
		//printf("-->TX IQ USB Phase balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_IQ_USB_PHASE_BALANCE,0);
		//printf("-->TX IQ USB Phase balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ LSB Gain Balance values - update if changed
	if(Read_EEPROM(EEPROM_RX_IQ_LSB_GAIN_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_RX_IQ_LSB_GAIN_BALANCE, ts.rx_iq_lsb_gain_balance);
		//printf("-->RX IQ LSB Gain balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_RX_IQ_LSB_GAIN_BALANCE,0);
		//printf("-->RX IQ LSB Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ USB Gain Balance values - update if changed
	if(Read_EEPROM(EEPROM_RX_IQ_USB_GAIN_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_RX_IQ_USB_GAIN_BALANCE, ts.rx_iq_usb_gain_balance);
		//printf("-->RX IQ USB Gain balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_RX_IQ_USB_GAIN_BALANCE,0);
		//printf("-->RX IQ USB Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ LSB Phase Balance values - update if changed
	if(Read_EEPROM(EEPROM_RX_IQ_LSB_PHASE_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_RX_IQ_LSB_PHASE_BALANCE, ts.rx_iq_lsb_phase_balance);
		//printf("-->RX IQ LSB Phase balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_RX_IQ_LSB_PHASE_BALANCE,0);
		//printf("-->RX IQ LSB Phase balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ USB Phase Balance values - update if changed
	if(Read_EEPROM(EEPROM_RX_IQ_USB_PHASE_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_RX_IQ_USB_PHASE_BALANCE, ts.rx_iq_usb_phase_balance);
		//printf("-->RX IQ USB Phase balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_RX_IQ_USB_PHASE_BALANCE,0);
		//printf("-->RX IQ USB Phase balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ AM Gain Balance values - update if changed
	if(Read_EEPROM(EEPROM_RX_IQ_AM_GAIN_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_RX_IQ_AM_GAIN_BALANCE, ts.rx_iq_am_gain_balance);
		//printf("-->RX IQ AM Gain balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_RX_IQ_AM_GAIN_BALANCE,0);
		//printf("-->RX IQ AM Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ FM Gain Balance values - update if changed
	if(Read_EEPROM(EEPROM_RX_IQ_FM_GAIN_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_RX_IQ_FM_GAIN_BALANCE, ts.rx_iq_fm_gain_balance);
		//printf("-->RX IQ FM Gain balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_RX_IQ_FM_GAIN_BALANCE,0);
		//printf("-->RX IQ FM Gain balance value created\n\r");
	}
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ AM Gain Balance values - update if changed
	if(Read_EEPROM(EEPROM_TX_IQ_AM_GAIN_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_IQ_AM_GAIN_BALANCE, ts.tx_iq_am_gain_balance);
		//printf("-->TX IQ AM Gain balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_IQ_AM_GAIN_BALANCE,0);
		//printf("-->RX IQ AM Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ FM Gain Balance values - update if changed
	if(Read_EEPROM(EEPROM_TX_IQ_FM_GAIN_BALANCE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_IQ_FM_GAIN_BALANCE, ts.tx_iq_fm_gain_balance);
		//printf("-->TX IQ FM Gain balance saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_IQ_FM_GAIN_BALANCE,0);
		//printf("-->RX IQ FM Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read SWR Forward power meter calibration value - update if changed
	if(Read_EEPROM(EEPROM_SENSOR_NULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_SENSOR_NULL, swrm.sensor_null);
		//printf("-->SWR Forward power meter calibration value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SENSOR_NULL, SENSOR_NULL_DEFAULT);
		//printf("-->SWR Forward power meter calibration value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Transverter frequency offset saved values - update if changed
	if(	(Read_EEPROM(EEPROM_XVERTER_OFFSET_HIGH, &value) == 0) && (Read_EEPROM(EEPROM_XVERTER_OFFSET_LOW, &value1) == 0)) {
		Write_EEPROM(EEPROM_XVERTER_OFFSET_HIGH,(ts.xverter_offset >> 16));
		//printf("-->freq high saved\n\r");
		Write_EEPROM(EEPROM_XVERTER_OFFSET_LOW,(ts.xverter_offset & 0xFFFF));
		//printf("-->freq low saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_XVERTER_OFFSET_HIGH,(ts.xverter_offset >> 16));
		Write_EEPROM(EEPROM_XVERTER_OFFSET_LOW,(ts.xverter_offset & 0xFFFF));
		//printf("-->Transverter offset frequency created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Transverter display offset mode enable - update if changed
	if(Read_EEPROM(EEPROM_XVERTER_DISP, &value) == 0)
	{
		Write_EEPROM(EEPROM_XVERTER_DISP, ts.xverter_mode);
		//printf("-->Transverter display offset mode enable value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_XVERTER_DISP,0);
		//printf("-->Transverter display offset mode enable value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 80m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND0_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND0_5W, ts.pwr_80m_5w_adj);
		//printf("-->80m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND0_5W,TX_POWER_FACTOR_80_DEFAULT);
		//printf("-->80m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 60m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND1_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND1_5W, ts.pwr_60m_5w_adj);
		//printf("-->60m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND1_5W,TX_POWER_FACTOR_60_DEFAULT);
		//printf("-->60m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 40m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND2_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND2_5W, ts.pwr_40m_5w_adj);
		//printf("-->40m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND2_5W,TX_POWER_FACTOR_40_DEFAULT);
		//printf("-->40m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 30m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND3_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND3_5W, ts.pwr_30m_5w_adj);
		//printf("-->80m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND3_5W,TX_POWER_FACTOR_30_DEFAULT);
		//printf("-->30m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 20m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND4_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND4_5W, ts.pwr_20m_5w_adj);
		//printf("-->20m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND4_5W,TX_POWER_FACTOR_20_DEFAULT);
		//printf("-->20m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 17m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND5_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND5_5W, ts.pwr_17m_5w_adj);
		//printf("-->17m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND5_5W,TX_POWER_FACTOR_17_DEFAULT);
		//printf("-->17m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 15m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND6_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND6_5W, ts.pwr_15m_5w_adj);
		//printf("-->15m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND6_5W,TX_POWER_FACTOR_15_DEFAULT);
		//printf("-->15m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 12m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND7_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND7_5W, ts.pwr_12m_5w_adj);
		//printf("-->12m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND7_5W,TX_POWER_FACTOR_12_DEFAULT);
		//printf("-->12m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10m 5 watt power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND8_5W, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND8_5W, ts.pwr_10m_5w_adj);
		//printf("-->10m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND8_5W,TX_POWER_FACTOR_10_DEFAULT);
		//printf("-->10m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 80m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND0_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND0_FULL, ts.pwr_80m_full_adj);
		//printf("-->80m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND0_FULL,TX_POWER_FACTOR_80_DEFAULT);
		//printf("-->80m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 60m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND1_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND1_FULL, ts.pwr_60m_full_adj);
		//printf("-->60m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND1_FULL,TX_POWER_FACTOR_60_DEFAULT);
		//printf("-->60m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 40m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND2_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND2_FULL, ts.pwr_40m_full_adj);
		//printf("-->40m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND2_FULL,TX_POWER_FACTOR_40_DEFAULT);
		//printf("-->40m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 30m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND3_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND3_FULL, ts.pwr_30m_full_adj);
		//printf("-->80m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND3_FULL,TX_POWER_FACTOR_30_DEFAULT);
		//printf("-->30m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 20m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND4_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND4_FULL, ts.pwr_20m_full_adj);
		//printf("-->20m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND4_FULL,TX_POWER_FACTOR_20_DEFAULT);
		//printf("-->20m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 17m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND5_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND5_FULL, ts.pwr_17m_full_adj);
		//printf("-->17m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND5_FULL,TX_POWER_FACTOR_17_DEFAULT);
		//printf("-->17m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 15m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND6_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND6_FULL, ts.pwr_15m_full_adj);
		//printf("-->15m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND6_FULL,TX_POWER_FACTOR_15_DEFAULT);
		//printf("-->15m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 12m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND7_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND7_FULL, ts.pwr_12m_full_adj);
		//printf("-->12m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND7_FULL,TX_POWER_FACTOR_12_DEFAULT);
		//printf("-->12m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10m FULL power setting - update if changed
	if(Read_EEPROM(EEPROM_BAND8_FULL, &value) == 0)
	{
		Write_EEPROM(EEPROM_BAND8_FULL, ts.pwr_10m_full_adj);
		//printf("-->10m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BAND8_FULL,TX_POWER_FACTOR_10_DEFAULT);
		//printf("-->10m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read spectrum scope magnify mode - update if changed
	if(Read_EEPROM(EEPROM_SPECTRUM_MAGNIFY, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPECTRUM_MAGNIFY, sd.magnify);
		//printf("-->spectrum scope magnify mode value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPECTRUM_MAGNIFY,0);
		//printf("-->spectrum scope magnify mode value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read wide filter in CW mode disable - update if changed
	if(Read_EEPROM(EEPROM_WIDE_FILT_CW_DISABLE, &value) == 0)
	{
		Write_EEPROM(EEPROM_WIDE_FILT_CW_DISABLE, ts.filter_cw_wide_disable);
		//printf("-->wide filter in CW mode disable value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_WIDE_FILT_CW_DISABLE,1);	// wide filters disabled in CW mode by default
		//printf("-->wide filter in CW mode disable value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read narrow filter in SSB mode disable - update if changed
	if(Read_EEPROM(EEPROM_NARROW_FILT_SSB_DISABLE, &value) == 0)
	{
		Write_EEPROM(EEPROM_NARROW_FILT_SSB_DISABLE, ts.filter_ssb_narrow_disable);
		//printf("-->narrow filter in SSB mode disable value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_NARROW_FILT_SSB_DISABLE,1);		// CW filters disabled by default in SSB mode
		//printf("-->narrow filter in SSB mode disable value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read AM mode disable - update if changed
	if(Read_EEPROM(EEPROM_AM_MODE_DISABLE, &value) == 0)
	{
		Write_EEPROM(EEPROM_AM_MODE_DISABLE, ts.am_mode_disable);
		//printf("-->narrow filter in SSB mode disable value saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_AM_MODE_DISABLE,1);		// AM mode disabled by default
		//printf("-->narrow filter in SSB mode disable value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope dB/Division - update if changed
	if(Read_EEPROM(EEPROM_SPECTRUM_DB_DIV, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPECTRUM_DB_DIV, ts.spectrum_db_scale);
		//printf("-->Spectrum scope rescale rate saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPECTRUM_DB_DIV,DB_DIV_ADJUST_DEFAULT);
		//printf("-->Spectrum scope rescale rate value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum scope AGC - update if changed
	if(Read_EEPROM(EEPROM_SPECTRUM_AGC_RATE, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPECTRUM_AGC_RATE, ts.scope_agc_rate);
		//printf("-->Spectrum scope AGC saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPECTRUM_AGC_RATE,SPECTRUM_SCOPE_AGC_DEFAULT);
		//printf("-->Spectrum scope AGC value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read meter mode - update if changed
	if(Read_EEPROM(EEPROM_METER_MODE, &value) == 0)
	{
		Write_EEPROM(EEPROM_METER_MODE, ts.tx_meter_mode);
		//printf("-->METER MODE saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_METER_MODE,METER_SWR);
		//printf("-->METER MODE value created\n\r");
	}
	//
	// is the TX compressor enabled?  If so, do NOT overwrite the currently-saved values for ALC release time or post-filter TX gain
	//
	//if(!ts.tx_comp_level)	{
		// ------------------------------------------------------------------------------------
		// Try to read ALC release (decay) time - update if changed
		if(Read_EEPROM(EEPROM_ALC_DECAY_TIME, &value) == 0)
		{
			Write_EEPROM(EEPROM_ALC_DECAY_TIME, ts.alc_decay);
			//printf("-->ALC release time saved\n\r");
		}
		else	// create
		{
			Write_EEPROM(EEPROM_ALC_DECAY_TIME,ALC_DECAY_DEFAULT);
			//printf("-->ALC Release time value created\n\r");
		}
		//
		// ------------------------------------------------------------------------------------
		// Try to read TX audio post-filter gain setting - update if changed
		if(Read_EEPROM(EEPROM_ALC_POSTFILT_TX_GAIN, &value) == 0)
		{
			Write_EEPROM(EEPROM_ALC_POSTFILT_TX_GAIN, ts.alc_tx_postfilt_gain);
			//printf("-->TX audio post-filter gain setting saved\n\r");
		}
		else	// create
		{
			Write_EEPROM(EEPROM_ALC_POSTFILT_TX_GAIN,ALC_POSTFILT_GAIN_DEFAULT);
			//printf("-->TX audio post-filter gain setting value created\n\r");
		}
//	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read step size marker line setting - update if changed
	if(Read_EEPROM(EEPROM_STEP_SIZE_CONFIG, &value) == 0)
	{
		Write_EEPROM(EEPROM_STEP_SIZE_CONFIG, ts.freq_step_config);
		//printf("-->step size marker line setting saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_STEP_SIZE_CONFIG,0);
		//printf("-->step size marker line setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP mode setting - update if changed
	if(Read_EEPROM(EEPROM_DSP_MODE, &value) == 0)
	{
		Write_EEPROM(EEPROM_DSP_MODE, ts.dsp_active);
		//printf("-->DSP mode setting saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DSP_MODE,0);
		//printf("-->DSP mode setting setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP noise reduction strength - update if changed
	if(Read_EEPROM(EEPROM_DSP_NR_STRENGTH, &value) == 0)
	{
		Write_EEPROM(EEPROM_DSP_NR_STRENGTH, ts.dsp_nr_strength);
		//printf("-->DSP noise reduction strength saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DSP_NR_STRENGTH,DSP_NR_STRENGTH_DEFAULT);
		//printf("-->DSP noise reduction strength value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP noise reduction de-correlator buffer length - update if changed
	if(Read_EEPROM(EEPROM_DSP_NR_DECOR_BUFLEN, &value) == 0)
	{
		Write_EEPROM(EEPROM_DSP_NR_DECOR_BUFLEN, ts.dsp_nr_delaybuf_len);
		//printf("-->DSP noise reduction de-correlator buffer length saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DSP_NR_DECOR_BUFLEN, DSP_NR_BUFLEN_DEFAULT);
		//printf("-->DSP noise reduction de-correlator buffer length value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP noise reduction FFT length - update if changed
	if(Read_EEPROM(EEPROM_DSP_NR_FFT_NUMTAPS, &value) == 0)
	{
		Write_EEPROM(EEPROM_DSP_NR_FFT_NUMTAPS, ts.dsp_nr_numtaps);
		//printf("-->DSP noise reduction FFT length saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DSP_NR_FFT_NUMTAPS, DSP_NR_NUMTAPS_DEFAULT);
		//printf("-->DSP noise reduction FFT length value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP notch de-correlator buffer length - update if changed
	if(Read_EEPROM(EEPROM_DSP_NOTCH_DECOR_BUFLEN, &value) == 0)
	{
		Write_EEPROM(EEPROM_DSP_NOTCH_DECOR_BUFLEN, ts.dsp_notch_delaybuf_len);
		//printf("-->DSP notch de-correlator buffer length saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DSP_NOTCH_DECOR_BUFLEN, DSP_NOTCH_DELAYBUF_DEFAULT);
		//printf("-->DSP notch de-correlator buffer length value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP notch number of taps - update if changed
	if(Read_EEPROM(EEPROM_DSP_NOTCH_FFT_NUMTAPS, &value) == 0)
	{
		Write_EEPROM(EEPROM_DSP_NOTCH_FFT_NUMTAPS, ts.dsp_notch_numtaps);
		//printf("-->DSP notch de-correlator buffer length saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DSP_NOTCH_FFT_NUMTAPS, DSP_NOTCH_DELAYBUF_DEFAULT);
		//printf("-->DSP notch number of taps created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read DSP Notch convergence rate setting - update if changed
	if(Read_EEPROM(EEPROM_DSP_NOTCH_CONV_RATE, &value) == 0)
	{
		Write_EEPROM(EEPROM_DSP_NOTCH_CONV_RATE, ts.dsp_notch_mu);
		//printf("-->DSP Note convergence rate setting saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DSP_NOTCH_CONV_RATE, DSP_NOTCH_MU_DEFAULT);
		//printf("-->DSP Note convergence rate setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read maximum RF gain setting - update if changed
	if(Read_EEPROM(EEPROM_MAX_RX_GAIN, &value) == 0)
	{
		Write_EEPROM(EEPROM_MAX_RX_GAIN, ts.max_rf_gain);
		//printf("-->maximum RF gain setting saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_MAX_RX_GAIN, MAX_RF_GAIN_DEFAULT);
		//printf("-->maximum RF gain setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX compression setting - update if changed
	if(Read_EEPROM(EEPROM_TX_AUDIO_COMPRESS, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_AUDIO_COMPRESS, ts.tx_comp_level);
		//printf("-->TX compression setting saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_AUDIO_COMPRESS, TX_AUDIO_COMPRESSION_DEFAULT);
		//printf("-->TX compression setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX disable setting - update if changed
	if(Read_EEPROM(EEPROM_TX_DISABLE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_DISABLE, ts.tx_disable);
		//printf("-->TX disable setting saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_DISABLE, 0);
		//printf("-->TX disable setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Misc. flags 1 setting - update if changed
	if(Read_EEPROM(EEPROM_MISC_FLAGS1, &value) == 0)
	{
		Write_EEPROM(EEPROM_MISC_FLAGS1, ts.misc_flags1);
		//printf("-->Misc. flags 1 setting saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_MISC_FLAGS1, 0);
		//printf("-->Misc. flags 1 setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Misc. flags 2 setting - update if changed
	if(Read_EEPROM(EEPROM_MISC_FLAGS2, &value) == 0)
	{
		Write_EEPROM(EEPROM_MISC_FLAGS2, ts.misc_flags2);
		//printf("-->Misc. flags 2 setting saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_MISC_FLAGS2, 0);
		//printf("-->Misc. flags 2 setting value created\n\r");
	}
	//
	//
	// ------------------------------------------------------------------------------------
	// Try to read currently-stored version number - release - update if changed
	if(Read_EEPROM(EEPROM_VERSION_MINOR, &value) == 0)
	{
		Write_EEPROM(EEPROM_VERSION_MINOR, ts.version_number_minor);
		//printf("-->Version number saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_VERSION_MINOR, 0);
		//printf("-->Version number value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read currently-stored version number - release - update if changed
	if(Read_EEPROM(EEPROM_VERSION_NUMBER, &value) == 0)
	{
		Write_EEPROM(EEPROM_VERSION_NUMBER, ts.version_number_release);
		//printf("-->Version number saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_VERSION_NUMBER, 0);
		//printf("-->Version number value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read currently-stored version - build number - update if changed
	if(Read_EEPROM(EEPROM_VERSION_BUILD, &value) == 0)
	{
		Write_EEPROM(EEPROM_VERSION_BUILD, ts.version_number_build);
		//printf("-->Version number saved\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_VERSION_BUILD, 0);
		//printf("-->Version number value (build) created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read noise blanker time constant - update if changed
	if(Read_EEPROM(EEPROM_NB_AGC_TIME_CONST, &value) == 0)
	{
		Write_EEPROM(EEPROM_NB_AGC_TIME_CONST, ts.nb_agc_time_const);
		//printf("-->Noise blanker time constant\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_NB_AGC_TIME_CONST, NB_AGC_DEFAULT);
		//printf("-->Noise blanker time constant\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read CW offset mode - update if changed
	if(Read_EEPROM(EEPROM_CW_OFFSET_MODE, &value) == 0)
	{
		Write_EEPROM(EEPROM_CW_OFFSET_MODE, ts.cw_offset_mode);
		//printf("-->CW offset mode\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_CW_OFFSET_MODE, CW_OFFSET_MODE_DEFAULT);
		//printf("-->CW offset mode\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read I/Q Freq. conversion mode - update if changed
	if(Read_EEPROM(EEPROM_FREQ_CONV_MODE, &value) == 0)
	{
		Write_EEPROM(EEPROM_FREQ_CONV_MODE, ts.iq_freq_mode);
		//printf("-->I/Q Freq. conversion mode\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FREQ_CONV_MODE, FREQ_IQ_CONV_MODE_DEFAULT);
		//printf("-->I/Q Freq. conversion mode\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read LSB/USB auto select mode - update if changed
	if(Read_EEPROM(EEPROM_LSB_USB_AUTO_SELECT, &value) == 0)
	{
		Write_EEPROM(EEPROM_LSB_USB_AUTO_SELECT, ts.lsb_usb_auto_select);
		//printf("-->LSB/USB auto select mode\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_LSB_USB_AUTO_SELECT, AUTO_LSB_USB_DEFAULT);
		//printf("-->LSB/USB auto select mode\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read LCD blanking configuration - update if changed
	if(Read_EEPROM(EEPROM_LCD_BLANKING_CONFIG, &value) == 0)
	{
		Write_EEPROM(EEPROM_LCD_BLANKING_CONFIG, ts.lcd_backlight_blanking);
		//printf("-->LCD blanking configuration\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_LCD_BLANKING_CONFIG, 0);
		//printf("-->LCD blanking configuration\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read VFO/Split/Memory configuration - update if changed
	if(Read_EEPROM(EEPROM_VFO_MEM_MODE, &value) == 0)
	{
		Write_EEPROM(EEPROM_VFO_MEM_MODE, ts.vfo_mem_mode);
		//printf("-->VFO/Split/Memory configuration\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_VFO_MEM_MODE, 0);
		//printf("-->VFO/Split/Memory configuration\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power sensor coupling coefficient for 80m - update if changed
	if(Read_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_80M, &value) == 0)
	{
		Write_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_80M, swrm.coupling_80m_calc);
		//printf("-->Power sensor coupling 80m coefficient configuration written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_80M, SWR_COUPLING_DEFAULT);
		//printf("-->Power sensor couplingn 80m coefficient configuration created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power sensor coupling coefficient for 40m - update if changed
	if(Read_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_40M, &value) == 0)
	{
		Write_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_40M, swrm.coupling_40m_calc);
		//printf("-->Power sensor coupling 40m coefficient configuration written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_40M, SWR_COUPLING_DEFAULT);
		//printf("-->Power sensor couplingn 40m coefficient configuration created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power sensor coupling coefficient for 20m - update if changed
	if(Read_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_20M, &value) == 0)
	{
		Write_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_20M, swrm.coupling_20m_calc);
		//printf("-->Power sensor coupling 20m coefficient configuration written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_20M, SWR_COUPLING_DEFAULT);
		//printf("-->Power sensor couplingn 20m coefficient configuration created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power sensor coupling coefficient for 15m - update if changed
	if(Read_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_15M, &value) == 0)
	{
		Write_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_15M, swrm.coupling_15m_calc);
		//printf("-->Power sensor coupling 15m coefficient configuration written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_DETECTOR_COUPLING_COEFF_15M, SWR_COUPLING_DEFAULT);
		//printf("-->Power sensor couplingn 15m coefficient configuration created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the voltmeter calibration - update if changed
	if(Read_EEPROM(EEPROM_VOLTMETER_CALIBRATE, &value) == 0)
	{
		Write_EEPROM(EEPROM_VOLTMETER_CALIBRATE, ts.voltmeter_calibrate);
		//printf("-->Voltmeter calibration written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_VOLTMETER_CALIBRATE, POWER_VOLTMETER_CALIBRATE_DEFAULT);
		//printf("-->Voltmeter calibration created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the waterfall color scheme - update if changed
	if(Read_EEPROM(EEPROM_WATERFALL_COLOR_SCHEME, &value) == 0)
	{
		Write_EEPROM(EEPROM_WATERFALL_COLOR_SCHEME, ts.waterfall_color_scheme);
		//printf("-->waterfall color scheme written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_WATERFALL_COLOR_SCHEME, WATERFALL_COLOR_DEFAULT);
		//printf("-->waterfall color scheme created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the waterfall vertical step size - update if changed
	if(Read_EEPROM(EEPROM_WATERFALL_VERTICAL_STEP_SIZE, &value) == 0)
	{
		Write_EEPROM(EEPROM_WATERFALL_VERTICAL_STEP_SIZE, ts.waterfall_vert_step_size);
		//printf("-->waterfall vertical step size written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_WATERFALL_VERTICAL_STEP_SIZE, WATERFALL_STEP_SIZE_DEFAULT);
		//printf("-->waterfall vertical step size created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the waterfall 'amplitude' offset - update if changed
	if(Read_EEPROM(EEPROM_WATERFALL_OFFSET, &value) == 0)
	{
		Write_EEPROM(EEPROM_WATERFALL_OFFSET, ts.waterfall_offset);
		//printf("-->waterfall 'amplitude' offset written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_WATERFALL_OFFSET, WATERFALL_OFFSET_DEFAULT);
		//printf("-->waterfall 'amplitude' offset created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the waterfall contrast setting - update if changed
	if(Read_EEPROM(EEPROM_WATERFALL_CONTRAST, &value) == 0)
	{
		Write_EEPROM(EEPROM_WATERFALL_CONTRAST, ts.waterfall_contrast);
		//printf("-->waterfall contrast setting written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_WATERFALL_CONTRAST, WATERFALL_CONTRAST_DEFAULT);
		//printf("-->waterfall contrast setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the waterfall speed setting - update if changed
	if(Read_EEPROM(EEPROM_WATERFALL_SPEED, &value) == 0)
	{
		Write_EEPROM(EEPROM_WATERFALL_SPEED, ts.waterfall_speed);
		//printf("-->waterfall speed setting written\n\r");
	}
	else	// create
	{
		if(sd.use_spi)	// use different default if SPI interface is used
			Write_EEPROM(EEPROM_WATERFALL_SPEED, WATERFALL_SPEED_DEFAULT_SPI);
		else
			Write_EEPROM(EEPROM_WATERFALL_SPEED, WATERFALL_SPEED_DEFAULT_PARALLEL);
		//printf("-->waterfall speed setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the spectrum scope auto offset setting - update if changed
	if(Read_EEPROM(EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST, &value) == 0)
	{
		Write_EEPROM(EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST, ts.spectrum_scope_nosig_adjust);
		//printf("-->spectrum scope auto offset setting written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST, SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT);
		//printf("-->spectrum scope auto offset setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the waterfall auto offset setting - update if changed
	if(Read_EEPROM(EEPROM_WATERFALL_NOSIG_ADJUST, &value) == 0)
	{
		Write_EEPROM(EEPROM_WATERFALL_NOSIG_ADJUST, ts.waterfall_nosig_adjust);
		//printf("-->waterfall auto offset setting written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_WATERFALL_NOSIG_ADJUST, SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT);
		//printf("-->waterfall auto offset setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the waterfall size and other settings - update if changed
	if(Read_EEPROM(EEPROM_WATERFALL_SIZE, &value) == 0)
	{
		Write_EEPROM(EEPROM_WATERFALL_SIZE, ts.waterfall_size);
		//printf("-->waterfall size and other settings written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_WATERFALL_SIZE, WATERFALL_SIZE_DEFAULT);
		//printf("-->waterfall size and other settings created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the FFT window settings - update if changed
	if(Read_EEPROM(EEPROM_FFT_WINDOW, &value) == 0)
	{
		Write_EEPROM(EEPROM_FFT_WINDOW, ts.fft_window_type);
		//printf("-->FFT window settings written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FFT_WINDOW, FFT_WINDOW_DEFAULT);
		//printf("-->FFT window settings created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the TX audio mute delay setting - update if changed
	if(Read_EEPROM(EEPROM_TX_PTT_AUDIO_MUTE, &value) == 0)
	{
		Write_EEPROM(EEPROM_TX_PTT_AUDIO_MUTE, ts.tx_audio_muting_timing);
		//printf("-->TX audio mute delay setting written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_TX_PTT_AUDIO_MUTE, 0);		// Default value is zero (off)
		//printf("-->TX audio mute delay setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the Filter Display indicator - update if changed
	if(Read_EEPROM(EEPROM_FILTER_DISP_COLOUR, &value) == 0)
	{
		Write_EEPROM(EEPROM_FILTER_DISP_COLOUR, ts.filter_disp_colour);
		//printf("-->Filter Display indicator setting written\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FILTER_DISP_COLOUR, 0);		// Default value is zero (off)
		//printf("-->Filter Display indicator setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the FM Subaudible tone generate index - update if changed
	if(Read_EEPROM(EEPROM_FM_SUBAUDIBLE_TONE_GEN, &value) == 0)
	{
		Write_EEPROM(EEPROM_FM_SUBAUDIBLE_TONE_GEN, ts.fm_subaudible_tone_gen_select);
		//printf("-->FM Subaudible tone generate index setting\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FM_SUBAUDIBLE_TONE_GEN, 0);		// Default value is zero (off)
		//printf("-->FM Subaudible tone generate index setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the FM tone burst mode - update if changed
	if(Read_EEPROM(EEPROM_FM_TONE_BURST_MODE, &value) == 0)
	{
		Write_EEPROM(EEPROM_FM_TONE_BURST_MODE, ts.fm_tone_burst_mode);
		//printf("-->FM Tone burst setting\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FM_TONE_BURST_MODE, 0);		// Default value is zero (off)
		//printf("-->FM Tone burst setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the FM squelch setting - update if changed
	if(Read_EEPROM(EEPROM_FM_SQUELCH_SETTING, &value) == 0)
	{
		Write_EEPROM(EEPROM_FM_SQUELCH_SETTING, ts.fm_sql_threshold);
		//printf("-->FM squelch setting\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FM_SQUELCH_SETTING, FM_SQUELCH_DEFAULT);	// Set default value
		//printf("-->FM squelch setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the FM RX bandwidth setting - update if changed
	if(Read_EEPROM(EEPROM_FM_RX_BANDWIDTH, &value) == 0)
	{
		Write_EEPROM(EEPROM_FM_RX_BANDWIDTH, ts.fm_rx_bandwidth);
		//printf("-->FM RX bandwidth setting\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FM_RX_BANDWIDTH, FM_BANDWIDTH_DEFAULT);		// Set default value
		//printf("-->FM RX bandwidth setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the FM Subaudible tone detect index - update if changed
	if(Read_EEPROM(EEPROM_FM_SUBAUDIBLE_TONE_DET, &value) == 0)
	{
		Write_EEPROM(EEPROM_FM_SUBAUDIBLE_TONE_DET, ts.fm_subaudible_tone_det_select);
		//printf("-->FM Subaudible tone detect index setting\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_FM_SUBAUDIBLE_TONE_DET, 0);		// Default value is zero (off)
		//printf("-->FM Subaudible tone detect index setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the Keyboard beep frequency - update if changed
	if(Read_EEPROM(EEPROM_KEYBOARD_BEEP_FREQ, &value) == 0)
	{
		Write_EEPROM(EEPROM_KEYBOARD_BEEP_FREQ, ts.beep_frequency);
		//printf("-->Keyboard beep frequency setting\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_KEYBOARD_BEEP_FREQ, 0);		// Default value is zero (off)
		//printf("-->Keyboard beep frequency setting created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read the Beep loudness - update if changed
	if(Read_EEPROM(EEPROM_BEEP_LOUDNESS, &value) == 0)
	{
		Write_EEPROM(EEPROM_BEEP_LOUDNESS, ts.beep_loudness);
		//printf("-->Beep loudness setting\n\r");
	}
	else	// create
	{
		Write_EEPROM(EEPROM_BEEP_LOUDNESS, 0);		// Default value is zero (off)
		//printf("-->Beep loudness setting created\n\r");
	}
	//
	//
	// Next setting...

// if serial eeprom is in use write blocks to it and switch block write flag back
if(ts.ser_eeprom_in_use == 0xAA)
    {
    Write_24Cxxseq(0, ts.eeprombuf, MAX_VAR_ADDR*2+2, ts.ser_eeprom_type);
    ts.ser_eeprom_in_use = 0;
    }
}


// check if touched point is within rectange of valid action
bool check_tp_coordinates(uint8_t x_left, uint8_t x_right, uint8_t y_down, uint8_t y_up)
{
if(ts.tp_x < x_left && ts.tp_x > x_right && ts.tp_y > y_down && ts.tp_y < y_up)
    return true;
else
    return false;
}