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

// Common
#include "uhsdr_board.h"
#include <stdio.h>
#include <malloc.h>
#include "uhsdr_rtc.h"
#include "ui_spectrum.h"

#include "ui_configuration.h"
#include "config_storage.h"

// serial EEPROM driver
#include "uhsdr_hw_i2c.h"

#include "uhsdr_hmc1023.h"

// Audio Driver
#include "drivers/audio/audio_driver.h"
#include "drivers/audio/audio_management.h"
#include "drivers/audio/cw/cw_gen.h"
#include "drivers/audio/freedv_uhsdr.h"
#include "drivers/audio/audio_nr.h"

//cat
#include "drivers/cat/cat_driver.h"

// UI Driver
#include "drivers/ui/ui_driver.h"
#include "drivers/ui/lcd/ui_lcd_hy28.h"
#include "drivers/ui/menu/ui_menu.h"
#include "drivers/ui/oscillator/osc_interface.h"
#include "drivers/ui/oscillator/osc_si5351a.h"
#include "drivers/ui/oscillator/osc_si570.h"
#include "drivers/audio/codec/codec.h"
#include "misc/profiling.h"
// Keyboard Driver
// #include "keyb_driver.h"

// Misc
#include "drivers/audio/softdds/softdds.h"

// Eeprom
#include "misc/v_eprom/eeprom.h"
//
#include "drivers/ui/radio_management.h"
//

#include "misc/TestCPlusPlusInterface.h"

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin)
{
    if (ts.paddles_active != false)
    {
        switch(GPIO_Pin)
        {
        case BUTTON_PWR:
            break;
        case PADDLE_DAH:
            // Call handler
            if (Board_PttDahLinePressed() && RadioManagement_IsTxDisabledBy(TX_DISABLE_RXMODE) == false)
            {  // was PTT line low? Is it not a RX only mode
                RadioManagement_Request_TxOn();     // yes - ONLY then do we activate PTT!  (e.g. prevent hardware bug from keying PTT!)
                if(ts.dmod_mode == DEMOD_CW || is_demod_rtty() || ts.cw_text_entry)
                {
                    CwGen_DahIRQ();     // Yes - go to CW state machine
                }
            }
            break;
        case PADDLE_DIT:
            if((ts.dmod_mode == DEMOD_CW || is_demod_rtty() || ts.cw_text_entry) && Board_DitLinePressed())
            {
                RadioManagement_Request_TxOn();
                CwGen_DitIRQ();
            }
            break;
        }
    }
}

void TransceiverStateInit(void)
{
    // Defaults always
    ts.txrx_mode 		= TRX_MODE_RX;				// start in RX
    ts.samp_rate		= I2S_AUDIOFREQ_48K;			// set sampling rate

    ts.enc_one_mode 	= ENC_ONE_MODE_AUDIO_GAIN;
    ts.enc_two_mode 	= ENC_TWO_MODE_RF_GAIN;
    ts.enc_thr_mode		= ENC_THREE_MODE_RIT;

    ts.band		  		= BAND_MODE_20;			// band from eeprom
    ts.rx_temp_mute		= false;					// used in muting audio during band change
    ts.filter_band		= 0;					// used to indicate the bpf filter selection for power detector coefficient selection
    ts.dmod_mode 		= DEMOD_USB;				// demodulator mode
    ts.rx_gain[RX_AUDIO_SPKR].value = AUDIO_GAIN_DEFAULT;
    ts.rx_gain[RX_AUDIO_DIG].value		= DIG_GAIN_DEFAULT;

    ts.rx_gain[RX_AUDIO_SPKR].max	= MAX_VOLUME_DEFAULT;		// Set max volume default
    ts.rx_gain[RX_AUDIO_DIG].max   =  DIG_GAIN_MAX;			// Set max volume default

    ts.rx_gain[RX_AUDIO_SPKR].active_value = 1;			// this variable is used in the active RX audio processing function
    ts.rx_gain[RX_AUDIO_DIG].active_value = 1;			// this variable is used in the active RX audio processing function
    ts.rf_gain			= DEFAULT_RF_GAIN;		//
    ts.lineout_gain     = LINEOUT_GAIN_DEFAULT;
    ts.rf_codec_gain	= DEFAULT_RF_CODEC_GAIN_VAL;		// Set default RF gain (0 = lowest, 8 = highest, 9 = "Auto")
    ts.rit_value		= 0;					// RIT value

    ts.cw_sidetone_gain			= DEFAULT_SIDETONE_GAIN;	// Sidetone gain

    ts.cw_keyer_mode		= CW_KEYER_MODE_IAM_B;			// CW keyer mode
    ts.cw_keyer_speed		= CW_KEYER_SPEED_DEFAULT;			// CW keyer speed
    ts.cw_sidetone_freq	= CW_SIDETONE_FREQ_DEFAULT;		// CW sidetone and TX offset frequency
    ts.cw_paddle_reverse	= 0;					// Paddle defaults to NOT reversed
    ts.cw_rx_delay		= CW_TX2RX_DELAY_DEFAULT;			// Delay of TX->RX turnaround
    ts.cw_keyer_weight        = CW_KEYER_WEIGHT_DEFAULT;

    ts.audio_spkr_unmute_delay_count		= VOICE_TX2RX_DELAY_DEFAULT;			// TX->RX delay turnaround

    ts.nb_setting		= 0;					// Noise Blanker setting

    for (int i = 0; i < IQ_ADJUST_POINTS_NUM; i++)
    {
        for (int j = 0; j < IQ_TRANS_NUM; j++)
        {
            ts.tx_iq_gain_balance[i].value[j]   = IQ_BALANCE_OFF;                // Default settings for RX and TX gain and phase balance
            ts.tx_iq_phase_balance[i].value[j]   = IQ_BALANCE_OFF;                // Default settings for RX and TX gain and phase balance
            ts.rx_iq_gain_balance[i].value[j]   = IQ_BALANCE_OFF;                // Default settings for RX and TX gain and phase balance
            ts.rx_iq_phase_balance[i].value[j]   = IQ_BALANCE_OFF;                // Default settings for RX and TX gain and phase balance
        }
    }

    ts.tune_freq		= 0;
    //ts.tune_freq_old	= 0;

    //	ts.calib_mode		= 0;					// calibrate mode

    ts.menu_mode		= 0;					// menu mode
    ts.menu_item		= 0;					// menu item selection
    ts.menu_var			= 0;					// menu item change variable
    ts.menu_var_changed	= 0;					// TRUE if a menu variable was changed and that an EEPROM save should be done

    ts.tx_audio_source	= TX_AUDIO_MIC;				// default source is microphone
    ts.tx_mic_gain_mult	= MIC_GAIN_DEFAULT;			// actual operating value for microphone gain

    ts.tx_gain[TX_AUDIO_MIC]		= MIC_GAIN_DEFAULT;	// default line gain
    ts.tx_gain[TX_AUDIO_LINEIN_L]		= LINE_GAIN_DEFAULT;	// default line gain
    ts.tx_gain[TX_AUDIO_LINEIN_R]		= LINE_GAIN_DEFAULT;	// default line gain
    ts.tx_gain[TX_AUDIO_DIG]		= LINE_GAIN_DEFAULT;	// default line gain
    ts.tx_gain[TX_AUDIO_DIGIQ]		= LINE_GAIN_DEFAULT;	// default line gain

    ts.tune				= 0;				// reset tuning flag

    ts.tx_power_factor	= 0.50;					// TX power factor

    ts.pa_bias			= PA_BIAS_DEFAULT;		// Use lowest possible voltage as default
    ts.pa_cw_bias		= PA_BIAS_DEFAULT;			// Use lowest possible voltage as default (nonzero sets separate bias for CW mode)
    ts.freq_cal			= 0;				// Initial setting for frequency calibration
    ts.power_level		= PA_LEVEL_DEFAULT;			// See uhsdr_board.h for setting
    //
    //	ts.codec_vol		= 0;					// Holder for codec volume
    //	ts.codec_mute_state	= 0;					// Holder for codec mute state
    //	ts.codec_was_muted = 0;						// Indicator that codec *was* muted
    //
    ts.powering_down	= 0;						// TRUE if powering down
    //

    //
    ts.menu_item		= 0;					// start out with a reasonable menu item
    //
    ts.radio_config_menu_enable = 0;				// TRUE if radio configuration menu is to be enabled
    //
    ts.xverter_mode		= 0;					// TRUE if transverter mode is active (e.g. offset of display)
    ts.xverter_offset	= 0;					// Frequency offset in transverter mode (added to frequency display)
    //
    ts.refresh_freq_disp	= 1;					// TRUE if frequency/color display is to be refreshed when next called - NORMALLY LEFT AT 0 (FALSE)!!!
    // This is NOT reset by the LCD function, but must be enabled/disabled externally

    for (int idx=0; idx<MAX_BANDS; idx++)
    {
        ts.pwr_adj[ADJ_5W][idx] = 1;
        ts.pwr_adj[ADJ_FULL_POWER][idx] = 1;
    }

    ts.filter_cw_wide_disable		= 0;			// TRUE if wide filters are to be disabled in CW mode
    ts.filter_ssb_narrow_disable	= 0;				// TRUE if narrow (CW) filters are to be disabled in SSB mode
    ts.demod_mode_disable			= 0;		// TRUE if AM mode is to be disabled
    //
    ts.tx_meter_mode	= METER_SWR;
    //
    ts.alc_decay		= ALC_DECAY_DEFAULT;			// ALC Decay (release) default value
    ts.alc_decay_var	= ALC_DECAY_DEFAULT;			// ALC Decay (release) default value
    ts.alc_tx_postfilt_gain		= ALC_POSTFILT_GAIN_DEFAULT;	// Post-filter added gain default (used for speech processor/ALC)
    ts.alc_tx_postfilt_gain_var	= ALC_POSTFILT_GAIN_DEFAULT;	// Post-filter added gain default (used for speech processor/ALC)
    ts.tx_comp_level	= 0;					// 0=Release Time/Pre-ALC gain manually adjusted, >=1:  Value calculated by this parameter
    //
    ts.freq_step_config		= 0;				// disabled both marker line under frequency and swapping of STEP buttons
    //
    ts.dsp_active		= 0;					// TRUE if DSP noise reduction is to be enabled
    //    ts.dsp_active		= 0;					// if this line is enabled win peaks issue is present when starting mcHF with activated NB
    ts.digital_mode		= DigitalMode_None;					// digital modes OFF by default
    ts.dsp_active_toggle	= 0xff;					// used to hold the button G2 "toggle" setting.
    ts.dsp_nr_strength	= 50;					// "Strength" of DSP noise reduction (50 = medium)
#ifdef OBSOLETE_NR
    ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
    ts.dsp_nr_numtaps 	= DSP_NR_NUMTAPS_DEFAULT;		// default for number of FFT taps for noise reduction

    ts.dsp_notch_numtaps = DSP_NOTCH_NUMTAPS_DEFAULT;		// default for number of FFT taps for notch filter
    ts.dsp_notch_delaybuf_len =	DSP_NOTCH_DELAYBUF_DEFAULT;
#endif
#ifdef USE_LMS_AUTONOTCH
    ts.dsp_notch_numtaps = DSP_NOTCH_NUMTAPS_DEFAULT;		// default for number of FFT taps for notch filter
    ts.dsp_notch_delaybuf_len =	DSP_NOTCH_DELAYBUF_DEFAULT;
    ts.dsp_notch_mu = DSP_NOTCH_MU_DEFAULT;
#endif
    ts.dsp_inhibit		= 1;					// TRUE if DSP is to be inhibited - power up with DSP disabled

    ts.lcd_backlight_brightness = 0;			// = 0 full brightness
    ts.lcd_backlight_blanking = 0;				// MSB = 1 for auto-off of backlight, lower nybble holds time for auto-off in seconds
    ts.low_power_config = LOW_POWER_THRESHOLD_DEFAULT; // add LOW_POWER_THRESHOLD_OFFSET for voltage value
    //
    ts.tune_step		= 0;					// Used for press-and-hold step size changing mode
    ts.frequency_lock	= 0;					// TRUE if frequency knob is locked
    //
    ts.tx_disable		= TX_DISABLE_OFF;	    // > 0 if transmitter is to be disabled
    ts.flags1			= 0;					// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS1"
    ts.flags2			= 0;					// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS2"
    ts.sysclock			= 0;					// This counts up from zero when the unit is powered up at precisely 100 Hz over the long term.  This
    // is NEVER reset and is used for timing certain events.
    ts.version_number_release	= 0;			// version release - used to detect firmware change
    ts.version_number_major = 0;				// version build - used to detect firmware change
    ts.nb_agc_time_const	= 0;				// used to calculate the AGC time constant
    ts.cw_offset_mode	= CW_OFFSET_USB_RX;		// CW offset mode (USB, LSB, etc.)
    ts.cw_lsb			= false;				// Flag that indicates CW operates in LSB mode when TRUE
    ts.iq_freq_mode		= FREQ_IQ_CONV_MODE_DEFAULT;					// used to set/configure the I/Q frequency/conversion mode
    ts.conv_sine_flag	= 0;					// FALSE until the sine tables for the frequency conversion have been built (normally zero, force 0 to rebuild)
    ts.lsb_usb_auto_select	= 0;				// holds setting of LSB/USB auto-select above/below 10 MHz
    ts.last_tuning		= 0;					// this is a timer used to hold off updates of the spectrum scope when an SPI LCD display interface is used
    ts.lcd_blanking_time = 0;					// this holds the system time after which the LCD is blanked - if blanking is enabled
    ts.lcd_blanking_flag = 0;					// if TRUE, the LCD is blanked completely (e.g. backlight is off)
    ts.xvtr_adjust_flag = 0;					// set TRUE if transverter offset adjustment is in process
    ts.vfo_mem_mode = 0;						// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
    ts.voltmeter_calibrate	= POWER_VOLTMETER_CALIBRATE_DEFAULT;	// Voltmeter calibration constant

    // spectrum general settings
    ts.spectrum_filter      = SPECTRUM_FILTER_DEFAULT;  // default filter strength for spectrum scope
    ts.spectrum_centre_line_colour = SPEC_COLOUR_GRID_DEFAULT;      // color of center line of scope grid
    ts.spectrum_freqscale_colour    = SPEC_COLOUR_SCALE_DEFAULT;        // default colour for the spectrum scope frequency scale at the bottom
    ts.spectrum_db_scale = DB_DIV_10;               // default to 10dB/division
    ts.spectrum_size    = SPECTRUM_SIZE_DEFAULT;        // adjustment for waterfall size
    ts.spectrum_agc_rate   = SPECTRUM_SCOPE_AGC_DEFAULT;       // load default spectrum scope AGC rate

    // scope ui settings
    ts.scope_trace_colour   = SPEC_COLOUR_TRACE_DEFAULT;        // default colour for the spectrum scope trace
    ts.scope_grid_colour    = SPEC_COLOUR_GRID_DEFAULT;     // default colour for the spectrum scope grid
    ts.scope_speed      = SPECTRUM_SCOPE_SPEED_DEFAULT;     // default rate of spectrum scope update
    ts.scope_scheduler = 0;             // timer for scheduling the next update of the spectrum update

    // ts.spectrum_scope_nosig_adjust = SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT;   // Adjustment for no signal adjustment conditions for spectrum scope

    ts.waterfall.speed  = WATERFALL_SPEED_DEFAULT;      // default speed of update of the waterfall
    ts.waterfall.color_scheme = WATERFALL_COLOR_DEFAULT;		// color scheme for waterfall display
    ts.waterfall.vert_step_size = WATERFALL_STEP_SIZE_DEFAULT;	// step size in waterfall display
#if 0
    ts.waterfall.offset = WATERFALL_OFFSET_DEFAULT;			// Offset for waterfall display (brightness)
#endif
    ts.waterfall.contrast = WATERFALL_CONTRAST_DEFAULT;		// contrast setting for waterfall display
    ts.waterfall.scheduler=0;

#if 0
    ts.waterfall.nosig_adjust = WATERFALL_NOSIG_ADJUST_DEFAULT;	// Adjustment for no signal adjustment conditions for waterfall
#endif
//    ts.fft_window_type = FFT_WINDOW_DEFAULT;			// FFT Windowing type
    ts.dvmode = false;							        // disable "DV" mode RX/TX functions by default

    ts.txrx_switch_audio_muting_timing = 0;					// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
    ts.audio_dac_muting_timer = 0;					// timer used for muting TX audio when keying PTT to suppress "click" or "thump"
    ts.audio_dac_muting_flag = 0;					// when TRUE, audio is to be muted after PTT/keyup

    ts.filter_disp_colour = FILTER_DISP_COLOUR_DEFAULT;
    ts.vfo_mem_flag = 0;						// when TRUE, memory mode is enabled
    ts.mem_disp = 0;						// when TRUE, memory display is enabled
    ts.fm_subaudible_tone_gen_select = 0;				// lookup ("tone number") used to index the table generation (0 corresponds to "tone disabled")
    ts.fm_tone_burst_mode = 0;					// this is the setting for the tone burst generator
    ts.fm_tone_burst_timing = 0;					// used to time the duration of the tone burst
    ts.fm_sql_threshold = FM_SQUELCH_DEFAULT;			// squelch threshold
    ts.fm_subaudible_tone_det_select = 0;				// lookup ("tone number") used to index the table for tone detection (0 corresponds to "tone disabled")
    ts.beep_active = 1;						// TRUE if beep is active
    ts.beep_frequency = DEFAULT_BEEP_FREQUENCY;			// beep frequency, in Hz
    ts.beep_loudness = DEFAULT_BEEP_LOUDNESS;			// loudness of keyboard/CW sidetone test beep

    ts.ser_eeprom_type = 0;						// serial eeprom not present
    ts.configstore_in_use = CONFIGSTORE_IN_USE_FLASH;					// serial eeprom not in use

    ts.tp = &mchf_touchscreen;
    ts.display = &mchf_display;

    ts.show_debug_info = false;					// dont show coordinates on LCD
    ts.tune_power_level = 0;					// Tune with FULL POWER
    ts.xlat = 0;							// 0 = report base frequency, 1 = report xlat-frequency;
    ts.audio_int_counter = 0;					// test DL2FW
    ts.cat_band_index =255;						// no CAT command arrived
    ts.notch_frequency = 800;				// notch start frequency for manual notch filter
    ts.peak_frequency = 750;				// peak start frequency
    ts.bass_gain = 2;						// gain of the low shelf EQ filter
    ts.treble_gain = 0;						// gain of the high shelf EQ filter
    ts.tx_bass_gain = 4;					// gain of the TX low shelf EQ filter
    ts.tx_treble_gain = 4;					// gain of the TX high shelf EQ filter
    ts.s_meter = 1;							// S-Meter configuration, 0 = old school, 1 = dBm-based, 2=dBm/Hz-based
    ts.display_dbm = 0;						// style of dBm display, 0=OFF, 1= dbm, 2= dbm/Hz
//    ts.dBm_count = 0;						// timer start
    ts.tx_filter = 0;						// which TX filter has been chosen by the user
    ts.iq_auto_correction = 1;              // disable/enable automatic IQ correction
    ts.twinpeaks_tested = 2;                // twinpeak_tested = 2 --> wait for system to warm up
    // twinpeak_tested = 0 --> go and test the IQ phase
    // twinpeak_tested = 1 --> tested, verified, go and have a nice day!
//    ts.agc_wdsp = 0;
    ts.agc_wdsp_mode = 2;
    ts.agc_wdsp_slope = 70;
    ts.agc_wdsp_hang_enable = 0;
    ts.agc_wdsp_hang_time = 500;
    ts.agc_wdsp_hang_thresh = 45;
    ts.agc_wdsp_thresh = 60;
    ts.agc_wdsp_action = 0;
    ts.agc_wdsp_switch_mode = 1;
    ts.agc_wdsp_hang_action = 0;
    ts.agc_wdsp_tau_decay[0] = 4000;
    ts.agc_wdsp_tau_decay[1] = 2000;
    ts.agc_wdsp_tau_decay[2] = 500;
    ts.agc_wdsp_tau_decay[3] = 250;
    ts.agc_wdsp_tau_decay[4] = 50;
    ts.agc_wdsp_tau_decay[5] = 500;

    ts.agc_wdsp_tau_hang_decay = 200;
    ts.dbm_constant = 0;

    ts.FDV_TX_encode_ready = false;		// FREEDV handshaking test DL2FW
    ts.FDV_TX_samples_ready = 0;	// FREEDV handshaking test DL2FW
    ts.FDV_TX_out_start_pt=0;
    ts.FDV_TX_in_start_pt=0;
	ts.new_nb = false;	// new nb OFF at poweron
	ts.nr_alpha = 0.94; // spectral noise reduction
	ts.nr_alpha_int = 940;
	ts.nr_beta = 0.96;
	ts.nr_beta_int = 960;
//	ts.nr_vad_thresh = 4.0;
//	ts.nr_vad_thresh_int = 4000;
	ts.nr_enable = false;
	ts.NR_FFT_L = 256;
	ts.NR_FFT_LOOP_NO = 1;
//	ts.nr_gain_smooth_enable = false;
//	ts.nr_gain_smooth_alpha = 0.25;
//	ts.nr_gain_smooth_alpha_int = 250;
//	ts.nr_long_tone_enable = false;
//	ts.nr_long_tone_alpha_int = 99900;
//	ts.nr_long_tone_alpha = 0.999;
//	ts.nr_long_tone_thresh = 10000;
//	ts.nr_long_tone_reset = true;
	ts.nr_first_time = 1;
//	ts.nr_vad_delay = 7;
	ts.NR_decimation_enable = true;
	ts.nr_fft_256_enable = true;
	ts.special_functions_enabled = 0;
	NR2.width = 4;
	NR2.power_threshold = 0.40;
	NR2.power_threshold_int = 40;
	NR2.asnr = 30;


    ts.i2c_speed[I2C_BUS_1] = I2C1_SPEED_DEFAULT; // Si570, MCP9801
    ts.i2c_speed[I2C_BUS_2] = I2C2_SPEED_DEFAULT; // Codec, EEPROM

    ts.rtty_atc_enable = true;
    ts.keyer_mode.active = false;
    ts.keyer_mode.button_recording = KEYER_BUTTON_NONE;
    for (int idx = 0; idx<KEYER_BUTTONS; idx++)
    {

    	ts.keyer_mode.macro[idx][0] = '\0';
    	strcpy((char *) ts.keyer_mode.cap[idx], "BTN");

    }
    ts.buffered_tx = false;
    ts.cw_text_entry = false;
    ts.debug_si5351a_pllreset = 2;		//start with "reset on IQ Divider"
}

void MiscInit(void)
{
    // Init Soft DDS
    float freq[2] = { 0.0, 0.0 };
    softdds_configRunIQ(freq,ts.samp_rate,0);
}


static const uint8_t canary_word[16] = { 'D', 'O',' ' ,'N', 'O', 'T', ' ', 'O', 'V', 'E', 'R' , 'W', 'R' , 'I', 'T','E' };
uint8_t* canary_word_ptr;

// in hex 44 4f 20 4e 4f 54 20 4f 56 45 52 57 52 49 54 45
// this has to be called after all dynamic memory allocation has happened
void Canary_Create()
{
    canary_word_ptr = (uint8_t*)malloc(sizeof(canary_word));
    memcpy(canary_word_ptr,canary_word,16);
}
// in hex 44 4f 20 4e 4f 54 20 4f 56 45 52 57 52 49 54 45
// this has to be called after all dynamic memory allocation has happened
bool Canary_IsIntact()
{
    return memcmp(canary_word_ptr,canary_word,16) == 0;
}

uint8_t* Canary_GetAddr()
{
    return canary_word_ptr;
}


// #include "Trace.h"
#if 0
void timeTest1()
{
    static uint32_t time = 0;
    if (time != RTC->TR)
    {
        Board_RedLed(LED_STATE_TOGGLE);
        time = RTC->TR;
    }
}
void timeTest()
{
    MchfRtc_Start();
    while (1)
    {
        timeTest1();
    }
}
#endif
// Power on
int mchfMain(void)
{

    ///trace_puts("Hello mcHF World!");
    // trace_printf(" %u\n", 1u);


    *(__IO uint32_t*)(SRAM2_BASE) = 0x0;	// clearing delay prevent for bootloader

    // Set default transceiver state
    TransceiverStateInit();

     Board_RamSizeDetection();

#ifdef TESTCPLUSPLUS
    test_call_cpp();
#endif

    // HW init
    Board_InitMinimal();
    // Show logo & HW Info
    UiDriver_StartUpScreenInit(2000);

    if (ts.display != DISPLAY_NONE)
    {
        // TODO: Better indication of non-detected display
        Board_GreenLed(LED_STATE_ON);
    }

	if(Si570_IsPresent())
	{
	  ts.si570_is_present = true;
	}
	else
	{
	  ts.si570_is_present = false;
	}


    Board_InitFull();

    // MchfRtc_FullReset();
    ConfigStorage_Init();

    // init mchf_touchscreen to see if it is present
    // we don't care about the screen being reverse or not
    // here, so we simply set reverse to false
    UiLcdHy28_TouchscreenInit(0);

    // Extra init
    MiscInit();

    // Usb Host driver init
    //keyb_driver_init();

#if 1
	// detection routine for special bootloader version strings which do enable debug or development functions
	char out[14];
    for(uint8_t* begin = (uint8_t*)0x8000000; begin < (uint8_t*)EEPROM_START_ADDRESS-8; begin++)
    {
    	if (memcmp("Version: ",begin,9) == 0)
        {
        	snprintf(out,13, "%s", &begin[9]);
        	for (uint8_t i=1; i<13; i++)
        	{
        	  if (out[i] == '\0')
        	  {
        		if (out[i-1] == 'a')
        		{
				  ts.special_functions_enabled = 1;
				}
        		if (out[i-1] == 's')
        		{
				  ts.special_functions_enabled = 2;
				}
			  break;
			  }
			}
        break;
        }
	}
#endif

    // UI HW init
    UiDriver_Init();


    if(mchf_touchscreen.present)
    {
    	//preventing DSP functions mask to have not proper value
        if (ts.dsp_mode_mask == 0 || ts.dsp_mode_mask == 1)
        {
            // empty mask is invalid, set it to all entries enabled
            ts.dsp_mode_mask = DSP_SWITCH_MODEMASK_ENABLE_DEFAULT;
        }
        else
        {
            // just make sure DSP OFF is always on the list
            ts.dsp_mode_mask|=DSP_SWITCH_MODEMASK_ENABLE_DSPOFF;
        }

    	ts.dsp_mode_mask&=DSP_SWITCH_MODEMASK_ENABLE_MASK;
    }
    else
    {
    	ts.dsp_mode_mask=DSP_SWITCH_MODEMASK_ENABLE_DEFAULT;		//disable masking when no touchscreen controller detected
    }

    // we now reinit the I2C buses with the configured speed settings. Loading the EEPROM always uses the default speed!
    mchf_hw_i2c1_init();
    mchf_hw_i2c2_init();

	// disable rx iq settings in menu when autocorr is enabled
	if(ts.iq_auto_correction == 1)
	{
	  ts.display_rx_iq = false;
	}
	else
	{
	  ts.display_rx_iq = true;
	}
    profileTimedEventInit();

#ifdef USE_HMC1023
    hmc1023_init();
#endif

    // Audio HW init
    AudioDriver_Init();

    UiDriver_StartupScreen_LogIfProblem(ts.codec_present == false,
            "Audiocodec WM8731 NOT detected!");

    const char* bl_version = Board_BootloaderVersion();

    UiDriver_StartupScreen_LogIfProblem(
            (bl_version[0] == '1' || bl_version[0] == '2' || bl_version[0] == '3' || bl_version[0] == '4')  && bl_version[1] == '.',

                "Upgrade bootloader to 5.0.1 or newer");


    AudioManagement_CalcSubaudibleGenFreq();		// load/set current FM subaudible tone settings for generation
    AudioManagement_CalcSubaudibleDetFreq();		// load/set current FM subaudible tone settings	for detection
    AudioManagement_LoadToneBurstMode();	// load/set tone burst frequency
    AudioManagement_KeyBeepPrepare();		// load/set beep frequency

    AudioFilter_SetDefaultMemories();


    ts.rx_gain[RX_AUDIO_SPKR].value_old = 0;		// Force update of volume control

#ifdef USE_FREEDV
    FreeDV_mcHF_init();
    // we now try to place a marker after last dynamically
    // allocated memory
    Canary_Create();
#endif



    UiDriver_StartUpScreenFinish(2000);
    Board_RedLed(LED_STATE_OFF);

    // TODO: We need to set the digital mode once to make it active
    // if we just loaded the mode from EEPROM since we do not active ts.dvmode
    if (ts.dmod_mode == DEMOD_DIGI)
    {
    	UiDriver_SetDemodMode(ts.dmod_mode);
    }

    // now enable paddles/ptt, i.e. external input
    ts.paddles_active = true;

    // Transceiver main loop
    for(;;)
    {
        // UI events processing
        UiDriver_TaskHandler_MainTasks();
    }
    return 0;
}
