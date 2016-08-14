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
#include "ui_configuration.h"
#include "config_storage.h"
#include <stdio.h>

// serial EEPROM driver
#include "mchf_hw_i2c2.h"

// Audio Driver
#include "audio_driver.h"
#include "audio_management.h"
#include "cw_gen.h"

// UI Driver
#include "ui_driver.h"
#include "ui_rotary.h"
#include "ui_lcd_hy28.h"
#include "ui_menu.h"
#include "ui_si570.h"
#include "codec.h"

// Keyboard Driver
// #include "keyb_driver.h"

// Misc
#include "softdds.h"

// Eeprom
#include "eeprom.h"
//
//
//
#include "cat_driver.h"

// Freedv Test DL2FW
#include "freedv_api.h"
#include "codec2_fdmdv.h"
// end Freedv Test DL2FW



#include "TestCPlusPlusInterface.h"
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

//*----------------------------------------------------------------------------
//* Function Name       : CriticalError
//* Object              : should never be here, really
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void CriticalError(ulong error)
{
    NVIC_SystemReset();
}

void NMI_Handler(void)
{
    CriticalError(1);
}

void HardFault_Handler(void)
{
    CriticalError(2);
}

void MemManage_Handler(void)
{
    CriticalError(3);
}

/* void BusFault_Handler(void)
{
    CriticalError(4);
}
*/

void UsageFault_Handler(void)
{
    CriticalError(5);
}

void SVC_Handler(void)
{
    CriticalError(6);
}

void DebugMon_Handler(void)
{
    CriticalError(7);
}

void SysTick_Handler(void)
{

}

/*
 * @brief Interrupt Handler for Paddles DAH and/or PTT
 */
void EXTI0_IRQHandler(void)
{
	// Checks whether the User Button EXTI line is asserted
	//
	// WARNING:
	// Due to an apparent HARDWARE bug in the MCU this interrupt seems to be occasionally triggered by transitions
	// of lines OTHER than the PADDLE_DAH (PE0) line, specifically the PC4 and PC5 (Step- and Step+) lines.
	//
	if (EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		// Call handler
		if(ts.dmod_mode == DEMOD_CW && mchf_ptt_dah_line_pressed())
		{	// was DAH line low?
				cw_gen_dah_IRQ();		// Yes - go to CW state machine
		}
		// PTT activate
		else if((ts.dmod_mode == DEMOD_USB)||(ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_AM) || (ts.dmod_mode == DEMOD_FM))
		{
			if(mchf_ptt_dah_line_pressed())
			{	// was PTT line low?
				ts.ptt_req = 1;		// yes - ONLY then do we activate PTT!  (e.g. prevent hardware bug from keying PTT!)
			}
		}
	}

    // Clears the EXTI's line pending bit
    EXTI_ClearITPendingBit(EXTI_Line0);
}

/*
 * @brief Interrupt Handler for Paddles DIT
 */
void EXTI1_IRQHandler(void)
{
    // Checks whether the User Button EXTI line (paddle DIT) is asserted
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        // Call handler
        // was Dit line low?  (Validate to prevent extraneous interrupts)
        if(ts.dmod_mode == DEMOD_CW && mchf_dit_line_pressed())
        {
            cw_gen_dit_IRQ();
        }
    }	// do nothing if not in CW mode!

    // Clears the EXTI's line pending bit
    EXTI_ClearITPendingBit(EXTI_Line1);
}

/*
 * @brief Interrupt Handler for Power Button Press
 */
void EXTI15_10_IRQHandler(void)
{
    // power button interrupt
    if(EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
//		if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))	// Signal power off

    }
    // Clear interrupt pending bit
    EXTI_ClearITPendingBit(EXTI_Line13);
}

/*void TIM5_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM5, TIM_IT_CC4) != RESET)
  {
    // Get the Input Capture value
    tmpCC4[CaptureNumber++] = TIM_GetCapture4(TIM5);

    // Clear CC4 Interrupt pending bit
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);

    if (CaptureNumber >= 2)
    {
      // Compute the period length
      PeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    }
  }
}*/

void TransceiverStateInit(void)
{
    // Defaults always
    ts.txrx_mode 		= TRX_MODE_RX;				// start in RX
    ts.samp_rate		= I2S_AudioFreq_48k;			// set sampling rate

    ts.enc_one_mode 	= ENC_ONE_MODE_AUDIO_GAIN;
    ts.enc_two_mode 	= ENC_TWO_MODE_RF_GAIN;
    ts.enc_thr_mode		= ENC_THREE_MODE_RIT;

    ts.band		  		= BAND_MODE_20;			// band from eeprom
    ts.band_change		= 0;					// used in muting audio during band change
    ts.filter_band		= 0;					// used to indicate the bpf filter selection for power detector coefficient selection
    ts.dmod_mode 		= DEMOD_USB;				// demodulator mode
    ts.rx_gain[RX_AUDIO_SPKR].value = DEFAULT_AUDIO_GAIN;
    ts.rx_gain[RX_AUDIO_DIG].value		= DEFAULT_DIG_GAIN;

    ts.rx_gain[RX_AUDIO_SPKR].max	= MAX_VOLUME_DEFAULT;		// Set max volume default
    ts.rx_gain[RX_AUDIO_DIG].max   =  MAX_DIG_GAIN;			// Set max volume default

    ts.rx_gain[RX_AUDIO_SPKR].active_value = 1;			// this variable is used in the active RX audio processing function
    ts.rx_gain[RX_AUDIO_DIG].active_value = 1;			// this variable is used in the active RX audio processing function
    ts.rf_gain			= DEFAULT_RF_GAIN;		//
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
    ts.unmute_delay_count		= SSB_RX_DELAY;			// Used to time TX->RX delay turnaround
    //
    ts.nb_setting		= 0;					// Noise Blanker setting
    //
    ts.tx_iq_lsb_gain_balance 	= 0;				// Default settings for RX and TX gain and phase balance
    ts.tx_iq_usb_gain_balance 	= 0;
    ts.tx_iq_lsb_phase_balance 	= 0;
    ts.tx_iq_usb_phase_balance 	= 0;

    ts.rx_iq_lsb_gain_balance = 0;
    ts.rx_iq_usb_gain_balance = 0;
    ts.rx_iq_lsb_phase_balance = 0;
    ts.rx_iq_usb_phase_balance = 0;
    ts.rx_iq_am_phase_balance = 0;
    ts.rx_iq_am_gain_balance = 0;
    ts.rx_iq_fm_gain_balance = 0;
    ts.tx_iq_am_gain_balance = 0;
    ts.tx_iq_fm_gain_balance = 0;
    //
    ts.tune_freq		= 0;
    //ts.tune_freq_old	= 0;
    //
//	ts.calib_mode		= 0;					// calibrate mode
    ts.menu_mode		= 0;					// menu mode
    ts.menu_item		= 0;					// menu item selection
    ts.menu_var			= 0;					// menu item change variable
    ts.menu_var_changed	= 0;					// TRUE if a menu variable was changed and that an EEPROM save should be done

    //ts.txrx_lock		= 0;					// unlocked on start
    ts.audio_unmute		= 0;					// delayed un-mute not needed
    ts.buffer_clear		= 0;					// used on return from TX to purge the audio buffers

    ts.tx_audio_source	= TX_AUDIO_MIC;				// default source is microphone
    ts.tx_mic_gain_mult	= MIC_GAIN_DEFAULT;			// actual operating value for microphone gain
    ts.mic_boost		= 0;
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
    //
    {
        int idx;
        for (idx=0; idx<MAX_BANDS; idx++)
        {
            ts.pwr_adj[ADJ_5W][idx] = 1;
            ts.pwr_adj[ADJ_FULL_POWER][idx] = 1;
        }
    }
    //
    ts.filter_cw_wide_disable		= 0;			// TRUE if wide filters are to be disabled in CW mode
    ts.filter_ssb_narrow_disable	= 0;				// TRUE if narrow (CW) filters are to be disabled in SSB mdoe
    ts.am_mode_disable				= 0;		// TRUE if AM mode is to be disabled
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
    ts.dsp_inhibit_mute = 0;					// holder for "dsp_inhibit" during muting operations to allow restoration of previous state
    ts.dsp_timed_mute	= 0;					// TRUE if DSP is to be muted for a timed amount
    ts.dsp_inhibit_timing = 0;					// used to time inhibiting of DSP when it must be turned off for some reason
    ts.reset_dsp_nr		= 0;					// TRUE if DSP NR coefficients are to be reset when "audio_driver_set_rx_audio_filter()" is called
    ts.lcd_backlight_brightness = 0;			// = 0 full brightness
    ts.lcd_backlight_blanking = 0;				// MSB = 1 for auto-off of backlight, lower nybble holds time for auto-off in seconds
    //
    ts.tune_step		= 0;					// Used for press-and-hold step size changing mode
    ts.frequency_lock	= 0;					// TRUE if frequency knob is locked
    //
    ts.tx_disable		= 0;					// TRUE if transmitter is to be disabled
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
    ts.freq_cal_adjust_flag = 0;				// set TRUE if frequency calibration is in process
    ts.xvtr_adjust_flag = 0;					// set TRUE if transverter offset adjustment is in process
    ts.rx_muting = 0;							// set TRUE if audio output is to be muted
    ts.rx_blanking_time = 0;					// this is a timer used to delay the un-blanking of the audio after a large synthesizer tuning step
    ts.vfo_mem_mode = 0;						// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
    // LSB+6 (0x40) = 0:  VFO A,  1 = VFO B
    // LSB+7 (0x80) = 0:  Normal mode, 1 = SPLIT mode
    // Other bits are currently reserved
    ts.voltmeter_calibrate	= POWER_VOLTMETER_CALIBRATE_DEFAULT;	// Voltmeter calibration constant
    ts.thread_timer = 0;						// used to time thread
    ts.waterfall_color_scheme = WATERFALL_COLOR_DEFAULT;		// color scheme for waterfall display
    ts.waterfall_vert_step_size = WATERFALL_STEP_SIZE_DEFAULT;	// step size in waterfall display
    ts.waterfall_offset = WATERFALL_OFFSET_DEFAULT;			// Offset for waterfall display (brightness)
    ts.waterfall_contrast = WATERFALL_CONTRAST_DEFAULT;		// contrast setting for waterfall display
    ts.spectrum_scope_scheduler = 0;				// timer for scheduling the next update of the spectrum scope update
    ts.spectrum_scope_nosig_adjust = SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT;	// Adjustment for no signal adjustment conditions for spectrum scope
    ts.waterfall_nosig_adjust = WATERFALL_NOSIG_ADJUST_DEFAULT;	// Adjustment for no signal adjustment conditions for waterfall
    ts.spectrum_size	= SPECTRUM_SIZE_DEFAULT;		// adjustment for waterfall size
    ts.fft_window_type = FFT_WINDOW_DEFAULT;			// FFT Windowing type
    ts.dvmode = 0;							// disable "DV" mode RX/TX functions by default
    ts.tx_audio_muting_timing = 0;					// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
    ts.tx_audio_muting_timer = 0;					// timer used for muting TX audio when keying PTT to suppress "click" or "thump"
    ts.tx_audio_muting_flag = 0;					// when TRUE, audio is to be muted after PTT/keyup
    ts.filter_disp_colour = FILTER_DISP_COLOUR_DEFAULT;
    ts.vfo_mem_flag = 0;						// when TRUE, memory mode is enabled
    ts.mem_disp = 0;						// when TRUE, memory display is enabled
    ts.load_eeprom_defaults = 0;					// when TRUE, defaults are loaded when "UiDriverLoadEepromValues()" is called - must be saved by user w/power-down to be permanent!
    ts.fm_subaudible_tone_gen_select = 0;				// lookup ("tone number") used to index the table generation (0 corresponds to "tone disabled")
    ts.fm_tone_burst_mode = 0;					// this is the setting for the tone burst generator
    ts.fm_tone_burst_timing = 0;					// used to time the duration of the tone burst
    ts.fm_sql_threshold = FM_SQUELCH_DEFAULT;			// squelch threshold
//	ts.fm_rx_bandwidth = FM_BANDWIDTH_DEFAULT;			// bandwidth setting for FM reception
    ts.fm_subaudible_tone_det_select = 0;				// lookup ("tone number") used to index the table for tone detection (0 corresponds to "tone disabled")
    ts.beep_active = 1;						// TRUE if beep is active
    ts.beep_frequency = DEFAULT_BEEP_FREQUENCY;			// beep frequency, in Hz
    ts.beep_loudness = DEFAULT_BEEP_LOUDNESS;			// loudness of keyboard/CW sidetone test beep
    ts.load_freq_mode_defaults = 0;					// when TRUE, load frequency defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!
    ts.boot_halt_flag = 0;						// when TRUE, boot-up is halted - used to allow various test functions
    ts.mic_bias = 1;						// mic bias on
    ts.ser_eeprom_type = 0;						// serial eeprom not present
    ts.ser_eeprom_in_use = SER_EEPROM_IN_USE_NO;					// serial eeprom not in use
    ts.tp_present = 0;						// default no touchscreen
    ts.tp_x = 0xFF;							// invalid position
    ts.tp_y = 0xFF;							// invalid position
    ts.tp_state = 0;						// touchscreen state machine init
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
    ts.AM_experiment = 0;					// for AM demodulation experiments, not for "public" use
    ts.s_meter = 0;							// S-Meter configuration, 0 = old school, 1 = dBm-based, 2=dBm/Hz-based
    ts.display_dbm = 0;						// style of dBm display, 0=OFF, 1= dbm, 2= dbm/Hz
	ts.dBm_count = 0;						// timer start
	ts.tx_filter = 0;						// which TX filter has been chosen by the user

	// Freedv Test DL2FW
	ts.FDV_TX_encode_ready = false;		// FREEDV handshaking test DL2FW
	ts.FDV_TX_samples_ready = 0;	// FREEDV handshaking test DL2FW
	ts.FDV_TX_out_start_pt=0;
	ts.FDV_TX_in_start_pt=0;

	// end Freedv Test DL2FW





// development setting for DF8OE
	if( *(__IO uint32_t*)(SRAM2_BASE+5) == 0x29)
	  {
	  ts.rfmod_present = 1;					// activate rfmod-board handling
	  }


}

//*----------------------------------------------------------------------------
//* Function Name       : MiscInit
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
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

// Power on
int main(void)
{
    *(__IO uint32_t*)(SRAM2_BASE) = 0x0;	// clearing delay prevent for bootloader

    mchf_board_detect_ramsize();
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

    // HW init
    mchf_board_init();

    mchf_board_green_led(1);

    // Set default transceiver state
    TransceiverStateInit();

    ConfigStorage_Init();

    // test if touchscreen is present
    UiLcdHy28_TouchscreenPresenceDetection();

    // Show logo & HW Info
    UiDriver_ShowStartUpScreen(100);

    // Extra init
    MiscInit();

    // Usb Host driver init
    //keyb_driver_init();

    // UI HW init
    ui_driver_init();

	ts.temp_nb = ts.nb_setting;
	ts.nb_setting = 0;

	// Audio HW init
	audio_driver_init();

	AudioManagement_CalcSubaudibleGenFreq();		// load/set current FM subaudible tone settings for generation
	AudioManagement_CalcSubaudibleDetFreq();		// load/set current FM subaudible tone settings	for detection
	AudioManagement_LoadToneBurstMode();	// load/set tone burst frequency
	AudioManagement_LoadBeepFreq();		// load/set beep frequency

    AudioFilter_SetDefaultMemories();

	UiInitRxParms();

	ts.rx_gain[RX_AUDIO_SPKR].value_old = 99;		// Force update of volume control
	Codec_Mute(false);					// make sure codec is un-muted

    if (ts.flags1 & FLAGS1_CAT_MODE_ACTIVE)
    {
        cat_driver_init();
    }

#ifdef USE_FREEDV
    // Freedv Test DL2FW

    f_FREEDV = freedv_open(FREEDV_MODE_1600);
    // ts.dvmode = true;
    // ts.digital_mode = 1;
    // end Freedv Test DL2FW
#endif


    // Transceiver main loop
    for(;;)
    {
        // UI events processing
        ui_driver_thread();
        CatDriverFT817CheckAndExecute();
        // Audio driver processing
        //audio_driver_thread();

        // USB Host driver processing
        //usbh_driver_thread();

        // Reset WD - not working
        //wd_reset();

        // TODO: Make that nicer and move out from here
        // The observation is that enabling the NB too early somehow made it not working properly
        // Maybe that can be fixed at some point
        if(ts.temp_nb < 0x80 && ts.sysclock > 50)		// load NB setting after processing first audio data
        {
            ts.nb_setting = ts.temp_nb;
            ts.temp_nb = 0xff;
        }
    }
}
