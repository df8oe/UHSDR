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
#include "mchf_board.h"
#include <stdio.h>
#include "mchf_rtc.h"
#include "ui_spectrum.h"

#include "ui_configuration.h"
#include "config_storage.h"

// serial EEPROM driver
#include "mchf_hw_i2c.h"

// Audio Driver
#include "drivers/audio/audio_driver.h"
#include "drivers/audio/audio_management.h"
#include "drivers/audio/cw/cw_gen.h"

#include "drivers/audio/freedv_mchf.h"
// UI Driver
#include "drivers/ui/ui_driver.h"
#include "drivers/ui/lcd/ui_lcd_hy28.h"
#include "drivers/ui/menu/ui_menu.h"
#include "drivers/ui/oscillator/ui_si570.h"
#include "drivers/audio/codec/codec.h"
#include "misc/profiling.h"
// Keyboard Driver
// #include "keyb_driver.h"

// Misc
#include "drivers/audio/softdds/softdds.h"

// Eeprom
#include "misc/v_eprom/eeprom.h"
//
//
//

#include "cat_driver.h"



#include "misc/TestCPlusPlusInterface.h"
// ----------------------------------------------------
// Create a time reference incremented by 1 mS and 10mS
//__IO uint32_t LocalTime_1MS  = 0;
//__IO uint32_t LocalTime_10MS = 0;
//__IO uint32_t LocalTime_Over = 0;
// ----------------------------------------------------

// USB Host
//extern USB_OTG_CORE_HANDLE          USB_OTG_Core_dev;

// TIM5 publics
//extern __IO uint32_t PeriodValue;
//extern __IO uint32_t CaptureNumber;
//uint16_t tmpCC4[2] = {0, 0};
//


// System tick if needed
__IO uint32_t TimingDelay = 0;

uchar wd_init_enabled = 0;

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin)
{
    switch(GPIO_Pin)
    {
    case BUTTON_PWR:
        break;
    case PADDLE_DAH:
        // Call handler
        if(ts.dmod_mode == DEMOD_CW && mchf_ptt_dah_line_pressed())
        {   // was DAH line low?
            CwGen_DahIRQ();     // Yes - go to CW state machine
        }
        // PTT activate
        else if(ts.dmod_mode != DEMOD_SAM)
        {
            if(mchf_ptt_dah_line_pressed())
            {   // was PTT line low?
                ts.ptt_req = 1;     // yes - ONLY then do we activate PTT!  (e.g. prevent hardware bug from keying PTT!)
            }
        }
        break;
    case PADDLE_DIT:
        if(ts.dmod_mode == DEMOD_CW && mchf_dit_line_pressed())
        {
            CwGen_DitIRQ();
        }
        break;
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
    ts.rx_gain[RX_AUDIO_SPKR].value = DEFAULT_AUDIO_GAIN;
    ts.rx_gain[RX_AUDIO_DIG].value		= DEFAULT_DIG_GAIN;

    ts.rx_gain[RX_AUDIO_SPKR].max	= MAX_VOLUME_DEFAULT;		// Set max volume default
    ts.rx_gain[RX_AUDIO_DIG].max   =  MAX_DIG_GAIN;			// Set max volume default

    ts.rx_gain[RX_AUDIO_SPKR].active_value = 1;			// this variable is used in the active RX audio processing function
    ts.rx_gain[RX_AUDIO_DIG].active_value = 1;			// this variable is used in the active RX audio processing function
    ts.rf_gain			= DEFAULT_RF_GAIN;		//
    ts.lineout_gain     = LINEOUT_GAIN_DEFAULT;
    ts.max_rf_gain		= MAX_RF_GAIN_DEFAULT;			// setting for maximum gain (e.g. minimum S-meter reading)
    ts.rf_codec_gain	= DEFAULT_RF_CODEC_GAIN_VAL;		// Set default RF gain (0 = lowest, 8 = highest, 9 = "Auto")
    ts.rit_value		= 0;					// RIT value
    ts.agc_mode			= AGC_DEFAULT;			// AGC setting
    ts.agc_custom_decay	= AGC_CUSTOM_DEFAULT;			// Default setting for AGC custom setting - higher = slower

    ts.st_gain			= DEFAULT_SIDETONE_GAIN;	// Sidetone gain
    ts.keyer_mode		= CW_MODE_IAM_B;			// CW keyer mode
    ts.keyer_speed		= DEFAULT_KEYER_SPEED;			// CW keyer speed
    ts.sidetone_freq	= CW_SIDETONE_FREQ_DEFAULT;		// CW sidetone and TX offset frequency
    ts.paddle_reverse	= 0;					// Paddle defaults to NOT reversed
    ts.cw_rx_delay		= CW_RX_DELAY_DEFAULT;			// Delay of TX->RX turnaround
    ts.audio_spkr_unmute_delay_count		= SSB_RX_DELAY;			// Used to time TX->RX delay turnaround

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

    ts.pa_bias			= DEFAULT_PA_BIAS;		// Use lowest possible voltage as default
    ts.pa_cw_bias		= DEFAULT_PA_BIAS;			// Use lowest possible voltage as default (nonzero sets separate bias for CW mode)
    ts.freq_cal			= 0;				// Initial setting for frequency calibration
    ts.power_level		= PA_LEVEL_DEFAULT;			// See mchf_board.h for setting
    //
    //	ts.codec_vol		= 0;					// Holder for codec volume
    //	ts.codec_mute_state	= 0;					// Holder for codec mute state
    //	ts.codec_was_muted = 0;						// Indicator that codec *was* muted
    //
    ts.powering_down	= 0;						// TRUE if powering down
    //
    ts.scope_speed		= SPECTRUM_SCOPE_SPEED_DEFAULT;		// default rate of spectrum scope update

    ts.waterfall_speed	= WATERFALL_SPEED_DEFAULT_SPI;		// default speed of update of the waterfall for parallel displays
    //
    ts.spectrum_filter		= SPECTRUM_FILTER_DEFAULT;	// default filter strength for spectrum scope
    ts.scope_trace_colour	= SPEC_COLOUR_TRACE_DEFAULT;		// default colour for the spectrum scope trace
    ts.scope_grid_colour	= SPEC_COLOUR_GRID_DEFAULT;		// default colour for the spectrum scope grid
    ts.scope_grid_colour_active = Grid;
    ts.spectrum_centre_line_colour = SPEC_COLOUR_GRID_DEFAULT;		// color of center line of scope grid
    ts.scope_centre_grid_colour_active = Grid;
    ts.spectrum_freqscale_colour	= SPEC_COLOUR_SCALE_DEFAULT;		// default colour for the spectrum scope frequency scale at the bottom
    ts.scope_agc_rate	= SPECTRUM_SCOPE_AGC_DEFAULT;		// load default spectrum scope AGC rate
    ts.spectrum_db_scale = DB_DIV_10;				// default to 10dB/division
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
    ts.filter_ssb_narrow_disable	= 0;				// TRUE if narrow (CW) filters are to be disabled in SSB mdoe
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
    ts.digital_mode		= 0;					// digital modes OFF by default
    ts.dsp_active_toggle	= 0xff;					// used to hold the button G2 "toggle" setting.
    ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
    ts.dsp_nr_strength	= 0;					// "Strength" of DSP noise reduction (0 = weak)
    ts.dsp_nr_numtaps 	= DSP_NR_NUMTAPS_DEFAULT;		// default for number of FFT taps for noise reduction
    ts.dsp_notch_numtaps = DSP_NOTCH_NUMTAPS_DEFAULT;		// default for number of FFT taps for notch filter
    ts.dsp_notch_delaybuf_len =	DSP_NOTCH_DELAYBUF_DEFAULT;
    ts.dsp_inhibit		= 1;					// TRUE if DSP is to be inhibited - power up with DSP disabled

    ts.lcd_backlight_brightness = 0;			// = 0 full brightness
    ts.lcd_backlight_blanking = 0;				// MSB = 1 for auto-off of backlight, lower nybble holds time for auto-off in seconds
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
    ts.cw_offset_mode	= 0;					// CW offset mode (USB, LSB, etc.)
    ts.cw_lsb			= 0;					// Flag that indicates CW operates in LSB mode when TRUE
    ts.iq_freq_mode		= 0;					// used to set/configure the I/Q frequency/conversion mode
    ts.conv_sine_flag	= 0;					// FALSE until the sine tables for the frequency conversion have been built (normally zero, force 0 to rebuild)
    ts.lsb_usb_auto_select	= 0;				// holds setting of LSB/USB auto-select above/below 10 MHz
    ts.last_tuning		= 0;					// this is a timer used to hold off updates of the spectrum scope when an SPI LCD display interface is used
    ts.lcd_blanking_time = 0;					// this holds the system time after which the LCD is blanked - if blanking is enabled
    ts.lcd_blanking_flag = 0;					// if TRUE, the LCD is blanked completely (e.g. backlight is off)
    ts.xvtr_adjust_flag = 0;					// set TRUE if transverter offset adjustment is in process
    ts.vfo_mem_mode = 0;						// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
    ts.voltmeter_calibrate	= POWER_VOLTMETER_CALIBRATE_DEFAULT;	// Voltmeter calibration constant
    ts.waterfall_color_scheme = WATERFALL_COLOR_DEFAULT;		// color scheme for waterfall display
    ts.waterfall_vert_step_size = WATERFALL_STEP_SIZE_DEFAULT;	// step size in waterfall display
    ts.waterfall_offset = WATERFALL_OFFSET_DEFAULT;			// Offset for waterfall display (brightness)
    ts.waterfall_contrast = WATERFALL_CONTRAST_DEFAULT;		// contrast setting for waterfall display
    ts.spectrum_scheduler = 0;				// timer for scheduling the next update of the spectrum update
    ts.spectrum_scope_nosig_adjust = SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT;	// Adjustment for no signal adjustment conditions for spectrum scope
    ts.waterfall_nosig_adjust = WATERFALL_NOSIG_ADJUST_DEFAULT;	// Adjustment for no signal adjustment conditions for waterfall
    ts.spectrum_size	= SPECTRUM_SIZE_DEFAULT;		// adjustment for waterfall size
    ts.fft_window_type = FFT_WINDOW_DEFAULT;			// FFT Windowing type
    ts.dvmode = 0;							// disable "DV" mode RX/TX functions by default

    ts.txrx_switch_audio_muting_timing = 0;					// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
    ts.audio_dac_muting_timer = 0;					// timer used for muting TX audio when keying PTT to suppress "click" or "thump"
    ts.audio_dac_muting_flag = 0;					// when TRUE, audio is to be muted after PTT/keyup

    ts.filter_disp_colour = FILTER_DISP_COLOUR_DEFAULT;
    ts.vfo_mem_flag = 0;						// when TRUE, memory mode is enabled
    ts.mem_disp = 0;						// when TRUE, memory display is enabled
    ts.load_eeprom_defaults = 0;					// when TRUE, defaults are loaded when "UiDriverLoadEepromValues()" is called - must be saved by user w/power-down to be permanent!
    ts.fm_subaudible_tone_gen_select = 0;				// lookup ("tone number") used to index the table generation (0 corresponds to "tone disabled")
    ts.fm_tone_burst_mode = 0;					// this is the setting for the tone burst generator
    ts.fm_tone_burst_timing = 0;					// used to time the duration of the tone burst
    ts.fm_sql_threshold = FM_SQUELCH_DEFAULT;			// squelch threshold
    ts.fm_subaudible_tone_det_select = 0;				// lookup ("tone number") used to index the table for tone detection (0 corresponds to "tone disabled")
    ts.beep_active = 1;						// TRUE if beep is active
    ts.beep_frequency = DEFAULT_BEEP_FREQUENCY;			// beep frequency, in Hz
    ts.beep_loudness = DEFAULT_BEEP_LOUDNESS;			// loudness of keyboard/CW sidetone test beep
    ts.load_freq_mode_defaults = 0;					// when TRUE, load frequency defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!
    ts.ser_eeprom_type = 0;						// serial eeprom not present
    ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_NO;					// serial eeprom not in use

    ts.tp = &mchf_touchscreen;
    ts.display = &mchf_display;

    ts.show_tp_coordinates = 0;					// dont show coordinates on LCD
    ts.rfmod_present = 0;						// rfmod not present
    ts.vhfuhfmod_present = 0;					// VHF/UHF mod not present
    ts.multi = 0;							// non-translate
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
    ts.AM_experiment = 1;					// for AM demodulation experiments, not for "public" use
    ts.s_meter = 0;							// S-Meter configuration, 0 = old school, 1 = dBm-based, 2=dBm/Hz-based
    ts.display_dbm = 0;						// style of dBm display, 0=OFF, 1= dbm, 2= dbm/Hz
//    ts.dBm_count = 0;						// timer start
    ts.tx_filter = 0;						// which TX filter has been chosen by the user
    ts.iq_auto_correction = 1;              // disable/enable automatic IQ correction
    ts.twinpeaks_tested = 2;                // twinpeak_tested = 2 --> wait for system to warm up
    // twinpeak_tested = 0 --> go and test the IQ phase
    // twinpeak_tested = 1 --> tested, verified, go and have a nice day!
    ts.agc_wdsp = 0;
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

    // development setting for DF8OE
    if( *(__IO uint32_t*)(SRAM2_BASE+5) == 0x29)
    {
        ts.rfmod_present = 1;					// activate rfmod-board handling
    }


    ts.i2c_speed[I2C_BUS_1] = I2C1_SPEED_DEFAULT; // Si570, MCP9801
    ts.i2c_speed[I2C_BUS_2] = I2C2_SPEED_DEFAULT; // Codec, EEPROM
}

void MiscInit(void)
{
    // Init Soft DDS
    float freq[2] = { 0.0, 0.0 };
    softdds_setfreq_dbl(freq,ts.samp_rate,0);
}

/*
static void wd_reset(void)
{
	// Init WD
	if(!wd_init_enabled)
	{
		// Start watchdog
		WWDG_Enable(WD_REFRESH_COUNTER);

		// Reset
		wd_init_enabled = 1;
		TimingDelay 	= 0;

		return;
	}

	// 40mS flag for WD reset
	if(TimingDelay > 40)
	{
		TimingDelay = 0;
		//GREEN_LED_PIO->ODR ^= RED_LED;

		// Update WWDG counter
		WWDG_SetCounter(WD_REFRESH_COUNTER);
	}
}
 */
// #include "Trace.h"
#if 0
void timeTest1()
{
    static uint32_t time = 0;
    if (time != RTC->TR)
    {
        MchfBoard_RedLed(LED_STATE_TOGGLE);
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

    mchf_board_detect_ramsize();
#if 0

    //	FLASH_OB_Unlock();
    //	FLASH_OB_WRPConfig(OB_WRP_Sector_All,DISABLE);
    //	FLASH_OB_Launch();
    //	ts.test = FLASH_OB_GetWRP();
    //	ts.test = FLASH_OB_GetRDP();
    // Set unbuffered mode for stdout (newlib)
    //setvbuf( stdout, 0, _IONBF, 0 );


    //	SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);

#ifdef TESTCPLUSPLUS
    test_call_cpp();
#endif
#endif
    // HW init
    mchf_board_init();
    MchfBoard_GreenLed(LED_STATE_ON);


    // MchfRtc_FullReset();
    ConfigStorage_Init();

    // init mchf_touchscreen to see if it is present
    // we don't care about the screen being reverse or not
    // here, so we simply set reverse to false
    UiLcdHy28_TouchscreenInit(false);


    // Show logo & HW Info
    UiDriver_ShowStartUpScreen(100);


    // Extra init
    MiscInit();

    // Usb Host driver init
    //keyb_driver_init();

    // UI HW init
    UiDriver_Init();

    // we now reinit the I2C buses with the configured speed settings. Loading the EEPROM always uses the default speed!
    mchf_hw_i2c1_init();
    mchf_hw_i2c2_init();


    // temporarily remember the setting until dsp is going to be activated
    // TODO: Needs to be checked, if this is the best way to get the noise blanker to work properly
    ts.temp_nb = ts.nb_setting;
    ts.nb_setting = 0;

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


    // Audio HW init
    AudioDriver_Init();

    AudioManagement_CalcSubaudibleGenFreq();		// load/set current FM subaudible tone settings for generation
    AudioManagement_CalcSubaudibleDetFreq();		// load/set current FM subaudible tone settings	for detection
    AudioManagement_LoadToneBurstMode();	// load/set tone burst frequency
    AudioManagement_LoadBeepFreq();		// load/set beep frequency

    AudioFilter_SetDefaultMemories();

    UiDriver_UpdateDisplayAfterParamChange();

    ts.rx_gain[RX_AUDIO_SPKR].value_old = 0;		// Force update of volume control

#ifdef USE_FREEDV
    FreeDV_mcHF_init();
#endif

    MchfBoard_RedLed(LED_STATE_OFF);
    // Transceiver main loop
    for(;;)
    {
        // UI events processing
        UiDriver_MainHandler();
        // Reset WD - not working
        //wd_reset();
    }
    return 0;
}
