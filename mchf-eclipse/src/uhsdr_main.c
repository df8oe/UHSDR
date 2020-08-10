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
#include "uhsdr_rtc.h"
#include "ui_spectrum.h"

#include "ui_configuration.h"
#include "config_storage.h"

// serial EEPROM driver
#include "uhsdr_hw_i2c.h"
#include "uhsdr_hw_i2s.h"
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
#include "uhsdr_canary.h"
#include "audio_agc.h"
// Keyboard Driver
// #include "keyb_driver.h"

// Misc
#include "drivers/audio/softdds/softdds.h"

#include "uhsdr_flash.h" // only for EEPROM_START_ADDRESS
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
                if(ts.dmod_mode == DEMOD_CW || is_demod_rtty() || is_demod_psk() || ts.cw_text_entry)
                {
                    CwGen_DahIRQ();     // Yes - go to CW state machine
                }
            }
            break;
        case PADDLE_DIT:
            if((ts.dmod_mode == DEMOD_CW || is_demod_rtty() || is_demod_psk() || ts.cw_text_entry) && Board_DitLinePressed())
            {
                if (ts.cw_keyer_mode != CW_KEYER_MODE_STRAIGHT)
                {
                    RadioManagement_Request_TxOn();
                }
                CwGen_DitIRQ();
            }
            break;
        }
    }
}

/**
 * Detects if a special bootloader is used and configures some settings
 * Used for debugging and testing purposes only
 */
void Main_DetectSpecialBootloader()
{
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
}

/**
 * All data inside the ts data structure which needs a non-zero value set at startup AND
 * is either used before the configuration has been loaded OR which is not loaded from the persistent configuration
 * should be initialized here.
 * Values which are initialized by other init functions before use should not be initialized here.
 * We try to initialize a single variable only once, to avoid confusion.
 *
 */
void TransceiverStateInit(void)
{
    // Defaults always
    ts.txrx_mode 		= TRX_MODE_RX;				// start in RX
    ts.samp_rate		= IQ_SAMPLE_RATE;			// set sampling rate

    //CONFIG LOADED: ts.band		  		= BAND_MODE_20;			// band from eeprom
    ts.rx_temp_mute		= false;					// used in muting audio during band change
    ts.filter_band		= FILTER_BAND_UNKNOWN;	// used to indicate the bpf filter selection for power detector coefficient selection
    ts.dmod_mode 		= DEMOD_USB;				// demodulator mode
    //CONFIG LOADED: ts.rx_gain[RX_AUDIO_SPKR].value = AUDIO_GAIN_DEFAULT;
    ts.rx_gain[RX_AUDIO_DIG].value		= DIG_GAIN_DEFAULT;

    //CONFIG LOADED:ts.rx_gain[RX_AUDIO_SPKR].max	= MAX_VOLUME_DEFAULT;		// Set max volume default
    ts.rx_gain[RX_AUDIO_DIG].max   =  DIG_GAIN_MAX;			// Set max volume default

    ts.rx_gain[RX_AUDIO_SPKR].active_value = 1;			// this variable is used in the active RX audio processing function
    ts.rx_gain[RX_AUDIO_DIG].active_value = 1;			// this variable is used in the active RX audio processing function
    //CONFIG LOADED:ts.lineout_gain     = LINEOUT_GAIN_DEFAULT;
    //CONFIG LOADED:ts.rf_codec_gain	= DEFAULT_RF_CODEC_GAIN_VAL;		// Set default RF gain (0 = lowest, 8 = highest, 9 = "Auto")
    ts.rit_value		= 0;					// RIT value

    //CONFIG LOADED:ts.cw_sidetone_gain			= DEFAULT_SIDETONE_GAIN;	// Sidetone gain

    //CONFIG LOADED:ts.cw_keyer_mode		= CW_KEYER_MODE_IAM_B;			// CW keyer mode
    //CONFIG LOADED:ts.cw_keyer_speed		= CW_KEYER_SPEED_DEFAULT;			// CW keyer speed
    //CONFIG LOADED:ts.cw_sidetone_freq	= CW_SIDETONE_FREQ_DEFAULT;		// CW sidetone and TX offset frequency
    //CONFIG LOADED:ts.cw_paddle_reverse	= 0;					// Paddle defaults to NOT reversed
    //CONFIG LOADED:ts.cw_rx_delay		= CW_TX2RX_DELAY_DEFAULT;			// Delay of TX->RX turnaround
    //CONFIG LOADED:ts.cw_keyer_weight        = CW_KEYER_WEIGHT_DEFAULT;

    ts.audio_spkr_unmute_delay_count		= VOICE_TX2RX_DELAY_DEFAULT;			// TX->RX delay turnaround

    ts.tune_freq		= 0;

    ts.menu_mode		= 0;					// menu mode
    ts.menu_item		= 0;					// menu item selection
    ts.menu_var			= 0;					// menu item change variable
    ts.menu_var_changed	        = 0;				// TRUE if a menu variable was changed and that an EEPROM save should be done

    //NO INIT NEEDED? SET BEFORE USE? ts.tx_mic_boost             = MIC_BOOST_DEFAULT;		// no extra gain. electret element assumed

    //CONFIG LOADED:ts.tx_audio_source	= TX_AUDIO_MIC;				// default source is microphone
    //NO INIT NEEDED, SET BEFORE USE: ts.tx_mic_gain_mult	= MIC_GAIN_DEFAULT;			// actual operating value for microphone gain

    //CONFIG LOADED:ts.tx_gain[TX_AUDIO_MIC]		= MIC_GAIN_DEFAULT;	// default line gain
    //CONFIG LOADED:ts.tx_gain[TX_AUDIO_LINEIN_L]	= LINE_GAIN_DEFAULT;	// default line gain
    ts.tx_gain[TX_AUDIO_LINEIN_R]	= LINE_GAIN_DEFAULT;	// default line gain
    ts.tx_gain[TX_AUDIO_DIG]		= LINE_GAIN_DEFAULT;	// default line gain
    ts.tx_gain[TX_AUDIO_DIGIQ]		= LINE_GAIN_DEFAULT;	// default line gain

    ts.tune				= false;				// reset tuning flag

    //NO INIT NEEDED, SET BEFORE USE:ts.tx_power_factor	= 0.50;					// TX power factor

    //CONFIG LOADED:ts.pa_bias			= PA_BIAS_DEFAULT;		// Use lowest possible voltage as default
    //CONFIG LOADED:ts.pa_cw_bias		= PA_BIAS_DEFAULT;			// Use lowest possible voltage as default (nonzero sets separate bias for CW mode)
    //CONFIG LOADED:ts.freq_cal			= 0;				// Initial setting for frequency calibration
    //CONFIG LOADED:ts.power_level		= PA_LEVEL_DEFAULT;			// See uhsdr_board.h for setting

    ts.powering_down	= 0;						// TRUE if powering down

    //CONFIG LOADED:ts.xverter_mode		= 0;					// TRUE if transverter mode is active (e.g. offset of display)
    //CONFIG LOADED:ts.xverter_offset	= 0;					// Frequency offset in transverter mode (added to frequency display)

    // This is NOT reset by the LCD function, but must be enabled/disabled externally

    //CONFIG LOADED:ts.demod_mode_disable			= 0;		// TRUE if a specific mode is to be disabled

    //CONFIG LOADED:ts.tx_meter_mode	= METER_SWR;

    //CONFIG LOADED:ts.alc_decay		= ALC_DECAY_DEFAULT;			// ALC Decay (release) default value
    //NO INIT NEEDED, SET BEFORE USE:ts.alc_decay_var	= ALC_DECAY_DEFAULT;			// ALC Decay (release) default value
    //CONFIG LOADED:ts.alc_tx_postfilt_gain		= ALC_POSTFILT_GAIN_DEFAULT;	// Post-filter added gain default (used for speech processor/ALC)
    //NO INIT NEEDED, SET BEFORE USEts.alc_tx_postfilt_gain_var	= ALC_POSTFILT_GAIN_DEFAULT;	// Post-filter added gain default (used for speech processor/ALC)
    //CONFIG LOADED:ts.tx_comp_level	= 0;					// 0=Release Time/Pre-ALC gain manually adjusted, >=1:  Value calculated by this parameter

    ts.freq_step_config		= 0;				// disabled both marker line under frequency and swapping of STEP buttons

    ts.lcd_backlight_brightness = 0;			// = 0 full brightness
    ts.lcd_backlight_blanking = 0;				// MSB = 1 for auto-off of backlight, lower nybble holds time for auto-off in seconds
    //CONFIG LOADED:ts.low_power_config = LOW_POWER_THRESHOLD_DEFAULT; // add LOW_POWER_THRESHOLD_OFFSET for voltage value

    ts.tune_step		= 0;					// Used for press-and-hold step size changing mode
    ts.frequency_lock	= 0;					// TRUE if frequency knob is locked

    //CONFIG LOADED:ts.tx_disable		= TX_DISABLE_OFF;	    // > 0 if transmitter is to be disabled
    //CONFIG LOADED:ts.flags1			= 0;					// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS1"
    //CONFIG LOADED:ts.flags2			= 0;					// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS2"
    ts.sysclock			= 0;					// This counts up from zero when the unit is powered up at precisely 100 Hz over the long term.  This

    // is NEVER reset and is used for timing certain events.
    ts.version_number_release	= 0;			// version release - used to detect firmware change
    ts.version_number_major = 0;				// version build - used to detect firmware change
    //CONFIG LOADED:ts.cw_offset_mode	= CW_OFFSET_USB_RX;		// CW offset mode (USB, LSB, etc.)
    ts.cw_lsb			= false;				// Flag that indicates CW operates in LSB mode when TRUE
    //CONFIG LOADED:ts.iq_freq_mode		= FREQ_IQ_CONV_MODE_DEFAULT;					// used to set/configure the I/Q frequency/conversion mode
    //CONFIG LOADED:ts.lsb_usb_auto_select	= 0;				// holds setting of LSB/USB auto-select above/below 10 MHz
    ts.last_tuning		= 0;					// this is a timer used to hold off updates of the spectrum scope when an SPI LCD display interface is used
    ts.lcd_blanking_time = 0;					// this holds the system time after which the LCD is blanked - if blanking is enabled
    ts.lcd_blanking_flag = 0;					// if TRUE, the LCD is blanked completely (e.g. backlight is off)
    ts.xvtr_adjust_flag = 0;					// set TRUE if transverter offset adjustment is in process
    //CONFIG LOADED:ts.vfo_mem_mode = 0;						// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
    //CONFIG LOADED:ts.voltmeter_calibrate	= POWER_VOLTMETER_CALIBRATE_DEFAULT;	// Voltmeter calibration constant

    // spectrum general settings
    //CONFIG LOADED:ts.spectrum_filter      = SPECTRUM_FILTER_DEFAULT;  // default filter strength for spectrum scope
    //CONFIG LOADED:ts.spectrum_centre_line_colour = SPEC_COLOUR_GRID_DEFAULT;      // color of center line of scope grid
    //CONFIG LOADED:ts.spectrum_freqscale_colour    = SPEC_COLOUR_SCALE_DEFAULT;        // default colour for the spectrum scope frequency scale at the bottom
    //CONFIG LOADED:ts.spectrum_db_scale = DB_DIV_10;               // default to 10dB/division
    //CONFIG LOADED:ts.spectrum_size    = SPECTRUM_SIZE_DEFAULT;        // adjustment for waterfall size
    //CONFIG LOADED:ts.spectrum_agc_rate   = SPECTRUM_SCOPE_AGC_DEFAULT;       // load default spectrum scope AGC rate

    // scope ui settings
    //CONFIG LOADED:ts.scope_trace_colour   = SPEC_COLOUR_TRACE_DEFAULT;        // default colour for the spectrum scope trace
    //CONFIG LOADED:ts.scope_grid_colour    = SPEC_COLOUR_GRID_DEFAULT;     // default colour for the spectrum scope grid
    //CONFIG LOADED:ts.scope_speed      = SPECTRUM_SCOPE_SPEED_DEFAULT;     // default rate of spectrum scope update
    ts.scope_scheduler = 0;             // timer for scheduling the next update of the spectrum update

    //CONFIG LOADED:ts.waterfall.speed  = WATERFALL_SPEED_DEFAULT;      // default speed of update of the waterfall
    //CONFIG LOADED:ts.waterfall.color_scheme = WATERFALL_COLOR_DEFAULT;		// color scheme for waterfall display
    //CONFIG LOADED:ts.waterfall.vert_step_size = WATERFALL_STEP_SIZE_DEFAULT;	// step size in waterfall display
    //CONFIG LOADED:ts.waterfall.contrast = WATERFALL_CONTRAST_DEFAULT;		// contrast setting for waterfall display
    ts.waterfall.scheduler=0;

//    ts.fft_window_type = FFT_WINDOW_DEFAULT;			// FFT Windowing type
    ts.dvmode = false;							        // disable "DV" mode RX/TX functions by default

    ts.txrx_switch_audio_muting_timing = 0;					// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
    ts.audio_dac_muting_timer = 0;					// timer used for muting TX audio when keying PTT to suppress "click" or "thump"
    ts.audio_dac_muting_flag = 0;					// when TRUE, audio is to be muted after PTT/keyup

    //CONFIG LOADED:ts.filter_disp_colour = FILTER_DISP_COLOUR_DEFAULT;
    ts.vfo_mem_flag = 0;						// when TRUE, memory mode is enabled
    ts.mem_disp = 0;						// when TRUE, memory display is enabled
    //CONFIG LOADED:ts.fm_subaudible_tone_gen_select = 0;				// lookup ("tone number") used to index the table generation (0 corresponds to "tone disabled")
    //CONFIG LOADED:ts.fm_tone_burst_mode = 0;					// this is the setting for the tone burst generator
    ts.fm_tone_burst_timing = 0;					// used to time the duration of the tone burst
    //CONFIG LOADED:ts.fm_sql_threshold = FM_SQUELCH_DEFAULT;			// squelch threshold
    //CONFIG LOADED:ts.fm_subaudible_tone_det_select = 0;				// lookup ("tone number") used to index the table for tone detection (0 corresponds to "tone disabled")

    ts.ser_eeprom_type = 0;						// serial eeprom not present
    ts.configstore_in_use = CONFIGSTORE_IN_USE_FLASH;					// serial eeprom not in use

    ts.tp = &mchf_touchscreen;
    ts.display = &mchf_display;

    ts.show_debug_info = false;					// dont show coordinates on LCD
    //CONFIG LOADED:ts.tune_power_level = 0;					// Tune with FULL POWER
    //CONFIG LOADED:ts.xlat = 0;							// 0 = report base frequency, 1 = report xlat-frequency;
    ts.audio_int_counter = 0;					// test DL2FW
    ts.cat_band_index =255;						// no CAT command arrived


    ts.s_meter = 1;                         // S-Meter configuration, 0 = old school, 1 = dBm-based, 2=dBm/Hz-based
    //CONFIG LOADED:ts.display_dbm = 0;                     // style of dBm display, 0=OFF, 1= dbm, 2= dbm/Hz
    //    ts.dBm_count = 0;                     // timer start
    //CONFIG LOADED:ts.tx_filter = 0;                       // which TX filter has been chosen by the user
    //CONFIG LOADED: ts.iq_auto_correction = 1;              // disable/enable automatic IQ correction

    ts.twinpeaks_tested = TWINPEAKS_WAIT;

    //CONFIG LOADED:ts.dbm_constant = 0;

    ts.i2c_speed[I2C_BUS_1] = I2C1_SPEED_DEFAULT; // Si570, MCP9801
    ts.i2c_speed[I2C_BUS_2] = I2C2_SPEED_DEFAULT; // Codec, EEPROM

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

    ts.vswr_protection_threshold = 1; // OFF

    //CONFIG LOADED:ts.expflags1 = 0; // Used to hold flags for options in Debug/Expert menu, stored in EEPROM location "EEPROM_EXPFLAGS1"

    ts.band_effective = NULL; // this is an invalid band number, which will trigger a redisplay of the band name and the effective power
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

    // Set default transceiver state
    TransceiverStateInit();

     Board_RamSizeDetection();

#ifdef TESTCPLUSPLUS
    test_call_cpp();
#endif

    // HW init
    Board_InitMinimal();
    // Show logo & HW Info
    UiDriver_StartUpScreenInit();

    if (ts.display != DISPLAY_NONE)
    {
        // TODO: Better indication of non-detected display
        Board_GreenLed(LED_STATE_ON);
    }

    Board_InitFull();

    ConfigStorage_Init();

    // init mchf_touchscreen to see if it is present
    // we don't care about the screen being reverse or not
    // here, so we simply set reverse to false
    UiLcdHy28_TouchscreenInit(0);


    Main_DetectSpecialBootloader();

    UiDriver_Init();


#ifdef STM32F4
    // we now re-init the I2C buses with the configured speed settings. Loading the EEPROM always uses the default speed!
    // we can do this only on the STM32F4 as we are not able to change
    // the speed on the STM32F7/H7 easily via HAL in a portable way
    UhsdrHw_I2C_ChangeSpeed(&hi2c1);
    UhsdrHw_I2C_ChangeSpeed(&hi2c2);
#endif

	profileTimedEventInit();

#ifdef USE_HMC1023
    hmc1023_init();
#endif

    // Audio Software Init
    AudioDriver_Init();

    // Audio Driver Hardware Init
    ts.codec_present = Codec_Reset(ts.samp_rate) == HAL_OK;

    UiDriver_StartupScreen_LogIfProblem(ts.codec_present == false,
            "Audiocodec WM8731 NOT detected!");

    const char* bl_version = Board_BootloaderVersion();

    UiDriver_StartupScreen_LogIfProblem(
            (bl_version[0] == '1' || bl_version[0] == '2' || bl_version[0] == '3' || bl_version[0] == '4')  && bl_version[1] == '.',

                "Upgrade bootloader to 5.0.1 or newer");

    AudioFilter_SetDefaultMemories();


    ts.rx_gain[RX_AUDIO_SPKR].value_old = 0;		// Force update of volume control

#ifdef USE_FREEDV
    FreeDV_Init();
    // we now try to place a marker after last dynamically
    // allocated memory
    Canary_Create();
#endif

    UiDriver_StartUpScreenFinish();

    // We initialize the requested demodulation mode
    // and update the screen accordingly
    UiDriver_SetDemodMode(ts.dmod_mode);

    // Finally, start DMA transfers to get everything going
    UhsdrHwI2s_Codec_StartDMA();


    // now enable paddles/ptt, i.e. external input
    ts.paddles_active = true;

    Board_RedLed(LED_STATE_OFF);


    // Transceiver main loop
    for(;;)
    {
        // UI events processing
        UiDriver_TaskHandler_MainTasks();
    }
    return 0;
}
