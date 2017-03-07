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
**  Licence:        GNU GPLv3                                                      **
************************************************************************************/
#include "ui_configuration.h"
#include "config_storage.h"

#include "ui_driver.h"

#include "audio_driver.h"

#include "ui_spectrum.h"
#include "radio_management.h"
#include "audio_management.h"
#include "ui.h" // bandInfo

// Virtual eeprom
#include "eeprom.h"
#include "mchf_hw_i2c.h"
#include "mchf_rtc.h"

// If more EEPROM variables are added, make sure that you add to this table - and the index to it in "eeprom.h"
// and correct MAX_VAR_ADDR in mchf_board.h


#define UI_C_EEPROM_BAND_5W_PF(bandNo,bandName1,bandName2) { ConfigEntry_UInt8, EEPROM_BAND##bandNo##_5W,&ts.pwr_adj[ADJ_5W][BAND_MODE_##bandName1],TX_POWER_FACTOR_##bandName1##_DEFAULT,0,TX_POWER_FACTOR_MAX },
#define UI_C_EEPROM_BAND_FULL_PF(bandNo,bandName1,bandName2) { ConfigEntry_UInt8, EEPROM_BAND##bandNo##_FULL,&ts.pwr_adj[ADJ_FULL_POWER][BAND_MODE_##bandName1],TX_POWER_FACTOR_##bandName1##_DEFAULT,0,TX_POWER_FACTOR_MAX },

// here all simple configuration values are defined
// in order to have a new configuration value being stored in nonvolatile memory
// just add an entry. It is important to specify the addres of a ram location from  where to read/write  the
// respective value. As these configuration descriptors are constant values, the address has to be the address
// of a global (or static variable) the compiler know about.
// It is also important to use the matching typeId otherwise data will not be (re)store in a number of cases
// Some configuration values are currently not in this structure since saving them is more complex. See LoadEeprom/SaveEeprom functions.

const ConfigEntryDescriptor ConfigEntryInfo[] =
{
    { ConfigEntry_UInt8, EEPROM_SPEC_SCOPE_SPEED,&ts.scope_speed,SPECTRUM_SCOPE_SPEED_DEFAULT,0,SPECTRUM_SCOPE_SPEED_MAX},
    { ConfigEntry_UInt32_16, EEPROM_FREQ_STEP,&df.selected_idx,3,0,T_STEP_MAX_STEPS-2},
    { ConfigEntry_UInt8, EEPROM_TX_AUDIO_SRC,&ts.tx_audio_source,0,0,TX_AUDIO_MAX_ITEMS},
    { ConfigEntry_UInt8, EEPROM_TCXO_STATE,&df.temp_enabled,TCXO_ON,0,255},
    { ConfigEntry_UInt8, EEPROM_AUDIO_GAIN,&ts.rx_gain[RX_AUDIO_SPKR].value,DEFAULT_AUDIO_GAIN,0,MAX_AUDIO_GAIN},
    { ConfigEntry_UInt8, EEPROM_RX_CODEC_GAIN,&ts.rf_codec_gain,DEFAULT_RF_CODEC_GAIN_VAL,0,MAX_RF_CODEC_GAIN_VAL},
    { ConfigEntry_Int32_16, EEPROM_RX_GAIN,&ts.rf_gain,DEFAULT_RF_GAIN,0,MAX_RF_GAIN},
    { ConfigEntry_UInt8, EEPROM_NB_SETTING,&ts.nb_setting,0,0,MAX_RF_ATTEN},
    { ConfigEntry_UInt8, EEPROM_TX_POWER_LEVEL,&ts.power_level,PA_LEVEL_DEFAULT,0,PA_LEVEL_TUNE_KEEP_CURRENT},
    { ConfigEntry_UInt8, EEPROM_KEYER_SPEED,&ts.keyer_speed,DEFAULT_KEYER_SPEED,MIN_KEYER_SPEED, MAX_KEYER_SPEED},
    { ConfigEntry_UInt8, EEPROM_KEYER_MODE,&ts.keyer_mode,CW_MODE_IAM_B, 0, CW_MAX_MODE},
    { ConfigEntry_UInt8, EEPROM_SIDETONE_GAIN,&ts.st_gain,DEFAULT_SIDETONE_GAIN,0, SIDETONE_MAX_GAIN},
    { ConfigEntry_Int32_16, EEPROM_FREQ_CAL,&ts.freq_cal,0,MIN_FREQ_CAL,MAX_FREQ_CAL},
    { ConfigEntry_UInt8, EEPROM_AGC_MODE,&ts.agc_mode,AGC_DEFAULT,0,AGC_MAX_MODE},
    { ConfigEntry_UInt8, EEPROM_AGC_WDSP_MODE,&ts.agc_wdsp_mode, 2,0,5},
    { ConfigEntry_UInt8, EEPROM_AGC_WDSP_SWITCH,&ts.agc_wdsp, 0,0,1},
    { ConfigEntry_UInt8, EEPROM_AGC_WDSP_HANG,&ts.agc_wdsp_hang_enable, 0,0,1},
    { ConfigEntry_Int32_16, EEPROM_AGC_WDSP_THRESH,&ts.agc_wdsp_thresh, 20,-20,120},
    { ConfigEntry_UInt8, EEPROM_AGC_WDSP_SLOPE,&ts.agc_wdsp_slope, 70,0,200},
    { ConfigEntry_Int32_16, EEPROM_AGC_WDSP_TAU_DECAY_0,&ts.agc_wdsp_tau_decay[0], 4000,100,5000},
    { ConfigEntry_Int32_16, EEPROM_AGC_WDSP_TAU_DECAY_1,&ts.agc_wdsp_tau_decay[1], 2000,100,5000},
    { ConfigEntry_Int32_16, EEPROM_AGC_WDSP_TAU_DECAY_2,&ts.agc_wdsp_tau_decay[2], 500,100,5000},
    { ConfigEntry_Int32_16, EEPROM_AGC_WDSP_TAU_DECAY_3,&ts.agc_wdsp_tau_decay[3], 250,100,5000},
    { ConfigEntry_Int32_16, EEPROM_AGC_WDSP_TAU_DECAY_4,&ts.agc_wdsp_tau_decay[4], 50,100,5000},
    { ConfigEntry_Int32_16, EEPROM_AGC_WDSP_TAU_HANG_DECAY,&ts.agc_wdsp_tau_hang_decay, 500,100,5000},
    { ConfigEntry_UInt8, EEPROM_MIC_GAIN,&ts.tx_gain[TX_AUDIO_MIC],MIC_GAIN_DEFAULT,MIC_GAIN_MIN,MIC_GAIN_MAX},
    { ConfigEntry_UInt8, EEPROM_LINE_GAIN,&ts.tx_gain[TX_AUDIO_LINEIN_L],LINE_GAIN_DEFAULT,LINE_GAIN_MIN,LINE_GAIN_MAX},
    { ConfigEntry_UInt32_16, EEPROM_SIDETONE_FREQ,&ts.sidetone_freq,CW_SIDETONE_FREQ_DEFAULT,CW_SIDETONE_FREQ_MIN,CW_SIDETONE_FREQ_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_FILTER,&ts.spectrum_filter,SPECTRUM_FILTER_DEFAULT,SPECTRUM_FILTER_MIN,SPECTRUM_FILTER_MAX},
    { ConfigEntry_UInt8, EEPROM_AGC_CUSTOM_DECAY,&ts.agc_custom_decay,AGC_CUSTOM_DEFAULT,0,AGC_CUSTOM_MAX},
    { ConfigEntry_UInt8, EEPROM_METER_COLOUR_UP,&ts.meter_colour_up,SPEC_BLUE, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_METER_COLOUR_DOWN,&ts.meter_colour_down,SPEC_CYAN, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_TRACE_COLOUR,&ts.scope_trace_colour,SPEC_COLOUR_TRACE_DEFAULT, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_GRID_COLOUR,&ts.scope_grid_colour,SPEC_COLOUR_GRID_DEFAULT, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_CENTRE_LINE_COLOUR,&ts.spectrum_centre_line_colour,SPEC_COLOUR_GRID_DEFAULT, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_SCALE_COLOUR,&ts.spectrum_freqscale_colour,SPEC_COLOUR_SCALE_DEFAULT, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_PADDLE_REVERSE,&ts.paddle_reverse,0,0,1},
    { ConfigEntry_UInt8, EEPROM_CW_RX_DELAY,&ts.cw_rx_delay,CW_RX_DELAY_DEFAULT,0,CW_RX_DELAY_MAX},
    { ConfigEntry_UInt8, EEPROM_MAX_VOLUME,&ts.rx_gain[RX_AUDIO_SPKR].max,MAX_VOLUME_DEFAULT,MAX_VOLUME_MIN,MAX_VOLUME_MAX},
    { ConfigEntry_UInt8, EEPROM_PA_BIAS,&ts.pa_bias,DEFAULT_PA_BIAS,0,MAX_PA_BIAS},
    { ConfigEntry_UInt8, EEPROM_PA_CW_BIAS,&ts.pa_cw_bias,DEFAULT_PA_BIAS,0,MAX_PA_BIAS},

    { ConfigEntry_UInt8, EEPROM_IQ_AUTO_CORRECTION,&ts.iq_auto_correction,0,0, 1},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_80M_GAIN_BALANCE,&ts.tx_iq_gain_balance[IQ_80M].value[IQ_TRANS_ON],IQ_BALANCE_OFF, MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_10M_GAIN_BALANCE,&ts.tx_iq_gain_balance[IQ_10M].value[IQ_TRANS_ON],IQ_BALANCE_OFF, MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_80M_PHASE_BALANCE,&ts.tx_iq_phase_balance[IQ_80M].value[IQ_TRANS_ON],IQ_BALANCE_OFF, MIN_IQ_PHASE_BALANCE, MAX_IQ_PHASE_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_10M_PHASE_BALANCE,&ts.tx_iq_phase_balance[IQ_10M].value[IQ_TRANS_ON],IQ_BALANCE_OFF, MIN_IQ_PHASE_BALANCE, MAX_IQ_PHASE_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_80M_GAIN_BALANCE,&ts.rx_iq_gain_balance[IQ_80M].value[IQ_TRANS_ON],IQ_BALANCE_OFF, MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_10M_GAIN_BALANCE,&ts.rx_iq_gain_balance[IQ_10M].value[IQ_TRANS_ON],IQ_BALANCE_OFF,  MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_80M_PHASE_BALANCE,&ts.rx_iq_phase_balance[IQ_80M].value[IQ_TRANS_ON],IQ_BALANCE_OFF,  MIN_IQ_PHASE_BALANCE, MAX_IQ_PHASE_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_10M_PHASE_BALANCE,&ts.rx_iq_phase_balance[IQ_10M].value[IQ_TRANS_ON],IQ_BALANCE_OFF,  MIN_IQ_PHASE_BALANCE, MAX_IQ_PHASE_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_80M_GAIN_BALANCE_TRANS_OFF,&ts.tx_iq_gain_balance[IQ_80M].value[IQ_TRANS_OFF],IQ_BALANCE_OFF,  MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_80M_PHASE_BALANCE_TRANS_OFF,&ts.tx_iq_phase_balance[IQ_80M].value[IQ_TRANS_OFF],IQ_BALANCE_OFF,  MIN_IQ_PHASE_BALANCE, MAX_IQ_PHASE_BALANCE},
    // FIXME: Change index of the fm balance entry to symbolic name!
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_FM_GAIN_BALANCE,&ts.rx_iq_gain_balance[3],IQ_BALANCE_OFF,  MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_10M_GAIN_BALANCE_TRANS_OFF,&ts.tx_iq_gain_balance[IQ_10M].value[IQ_TRANS_OFF],IQ_BALANCE_OFF, MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_10_PHASE_BALANCE_TRANS_OFF,&ts.tx_iq_phase_balance[IQ_10M].value[IQ_TRANS_OFF],IQ_BALANCE_OFF, MIN_IQ_PHASE_BALANCE, MAX_IQ_PHASE_BALANCE},
    { ConfigEntry_UInt8, EEPROM_SENSOR_NULL,&swrm.sensor_null,SENSOR_NULL_DEFAULT,SENSOR_NULL_MIN,SENSOR_NULL_MAX},
    { ConfigEntry_UInt8, EEPROM_XVERTER_DISP,&ts.xverter_mode,0,0,XVERTER_MULT_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_MAGNIFY,&sd.magnify,MAGNIFY_DEFAULT,MAGNIFY_MIN,MAGNIFY_MAX},
    { ConfigEntry_UInt8, EEPROM_WIDE_FILT_CW_DISABLE,&ts.filter_cw_wide_disable,1,0,1},
    { ConfigEntry_UInt8, EEPROM_NARROW_FILT_SSB_DISABLE,&ts.filter_ssb_narrow_disable,1,0,1},
    { ConfigEntry_UInt16, EEPROM_AM_MODE_DISABLE,&ts.demod_mode_disable,1,0,7},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_DB_DIV,&ts.spectrum_db_scale,DB_DIV_ADJUST_DEFAULT,DB_DIV_ADJUST_MIN, DB_DIV_ADJUST_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_AGC_RATE,&ts.scope_agc_rate,SPECTRUM_SCOPE_AGC_DEFAULT,SPECTRUM_SCOPE_AGC_MIN, SPECTRUM_SCOPE_AGC_MAX},
    { ConfigEntry_UInt8, EEPROM_METER_MODE,&ts.tx_meter_mode,METER_SWR,0,METER_MAX},
    { ConfigEntry_UInt32_16, EEPROM_ALC_DECAY_TIME,&ts.alc_decay,ALC_DECAY_DEFAULT,0,ALC_DECAY_MAX },
    { ConfigEntry_UInt32_16, EEPROM_ALC_POSTFILT_TX_GAIN,&ts.alc_tx_postfilt_gain,ALC_POSTFILT_GAIN_DEFAULT, ALC_POSTFILT_GAIN_MIN, ALC_POSTFILT_GAIN_MAX},
    { ConfigEntry_UInt8, EEPROM_STEP_SIZE_CONFIG,&ts.freq_step_config,0,0,255},
    { ConfigEntry_UInt8, EEPROM_DSP_MODE,&ts.dsp_active,0,0,255},
    { ConfigEntry_UInt8, EEPROM_DSP_NR_STRENGTH,&ts.dsp_nr_strength,DSP_NR_STRENGTH_DEFAULT,0, DSP_NR_STRENGTH_MAX},
    { ConfigEntry_UInt32_16, EEPROM_DSP_NR_DECOR_BUFLEN,&ts.dsp_nr_delaybuf_len,DSP_NR_BUFLEN_DEFAULT, DSP_NR_BUFLEN_MIN, DSP_NR_BUFLEN_MAX},
    { ConfigEntry_UInt8, EEPROM_DSP_NR_FFT_NUMTAPS,&ts.dsp_nr_numtaps,DSP_NR_NUMTAPS_DEFAULT, DSP_NR_NUMTAPS_MIN, DSP_NOTCH_NUMTAPS_MAX},
    { ConfigEntry_UInt8, EEPROM_DSP_NOTCH_DECOR_BUFLEN,&ts.dsp_notch_delaybuf_len,DSP_NOTCH_DELAYBUF_DEFAULT,DSP_NOTCH_BUFLEN_MIN,DSP_NOTCH_BUFLEN_MAX},
    { ConfigEntry_UInt8, EEPROM_DSP_NOTCH_FFT_NUMTAPS,&ts.dsp_notch_numtaps,DSP_NOTCH_NUMTAPS_DEFAULT, DSP_NOTCH_NUMTAPS_MIN,DSP_NOTCH_NUMTAPS_MAX},
    { ConfigEntry_UInt8, EEPROM_DSP_NOTCH_CONV_RATE,&ts.dsp_notch_mu,DSP_NOTCH_MU_DEFAULT,0,DSP_NOTCH_MU_MAX},
    { ConfigEntry_UInt8, EEPROM_MAX_RX_GAIN,&ts.max_rf_gain,MAX_RF_GAIN_DEFAULT,0,MAX_RF_GAIN_MAX},
    { ConfigEntry_Int16, EEPROM_TX_AUDIO_COMPRESS,&ts.tx_comp_level,TX_AUDIO_COMPRESSION_DEFAULT,TX_AUDIO_COMPRESSION_MIN,TX_AUDIO_COMPRESSION_MAX},
    { ConfigEntry_UInt8, EEPROM_TX_DISABLE,&ts.tx_disable,0,0,1},
    { ConfigEntry_UInt16, EEPROM_FLAGS1,&ts.flags1,0,0,0xffff},
    { ConfigEntry_UInt16, EEPROM_FLAGS2,&ts.flags2,0,0,0xffff},
    { ConfigEntry_UInt16, EEPROM_VERSION_MINOR,&ts.version_number_minor,0,0,255},
    { ConfigEntry_UInt16, EEPROM_VERSION_NUMBER,&ts.version_number_release,0,0,255},
    { ConfigEntry_UInt16, EEPROM_VERSION_BUILD,&ts.version_number_major,0,0,255},
    { ConfigEntry_UInt8, EEPROM_NB_AGC_TIME_CONST,&ts.nb_agc_time_const,NB_AGC_DEFAULT,0,NB_MAX_AGC_SETTING},
    { ConfigEntry_UInt8, EEPROM_CW_OFFSET_MODE,&ts.cw_offset_mode,CW_OFFSET_MODE_DEFAULT,0,CW_OFFSET_NUM-1},
    { ConfigEntry_UInt8, EEPROM_FREQ_CONV_MODE,&ts.iq_freq_mode,FREQ_IQ_CONV_MODE_DEFAULT,0,FREQ_IQ_CONV_MODE_MAX},
    { ConfigEntry_UInt8, EEPROM_LSB_USB_AUTO_SELECT,&ts.lsb_usb_auto_select,AUTO_LSB_USB_DEFAULT,0,AUTO_LSB_USB_MAX},
    { ConfigEntry_UInt8, EEPROM_LCD_BLANKING_CONFIG,&ts.lcd_backlight_blanking,0,0,255},
    { ConfigEntry_UInt32_16, EEPROM_VFO_MEM_MODE,&ts.vfo_mem_mode,0,0,255},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_2200M,&swrm.coupling_calc[COUPLING_2200M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_630M,&swrm.coupling_calc[COUPLING_630M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_160M,&swrm.coupling_calc[COUPLING_160M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_80M,&swrm.coupling_calc[COUPLING_80M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_40M,&swrm.coupling_calc[COUPLING_40M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_20M,&swrm.coupling_calc[COUPLING_20M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_15M,&swrm.coupling_calc[COUPLING_15M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_6M,&swrm.coupling_calc[COUPLING_6M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt32_16, EEPROM_VOLTMETER_CALIBRATE,&ts.voltmeter_calibrate,POWER_VOLTMETER_CALIBRATE_DEFAULT,POWER_VOLTMETER_CALIBRATE_MIN,POWER_VOLTMETER_CALIBRATE_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_COLOR_SCHEME,&ts.waterfall_color_scheme,WATERFALL_COLOR_DEFAULT,WATERFALL_COLOR_MIN,WATERFALL_COLOR_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_VERTICAL_STEP_SIZE,&ts.waterfall_vert_step_size,WATERFALL_STEP_SIZE_DEFAULT,WATERFALL_STEP_SIZE_MIN,WATERFALL_STEP_SIZE_MAX},
    { ConfigEntry_Int32_16, EEPROM_WATERFALL_OFFSET,&ts.waterfall_offset,WATERFALL_OFFSET_DEFAULT,WATERFALL_OFFSET_MIN,WATERFALL_OFFSET_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_SIZE,&ts.spectrum_size,SPECTRUM_SIZE_DEFAULT,0,SPECTRUM_BIG},
    { ConfigEntry_UInt32_16, EEPROM_WATERFALL_CONTRAST,&ts.waterfall_contrast,WATERFALL_CONTRAST_DEFAULT,WATERFALL_CONTRAST_MIN,WATERFALL_CONTRAST_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_SPEED,&ts.waterfall_speed,WATERFALL_SPEED_DEFAULT_PARALLEL,0,WATERFALL_SPEED_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST,&ts.spectrum_scope_nosig_adjust,SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT,SPECTRUM_SCOPE_NOSIG_ADJUST_MIN,SPECTRUM_SCOPE_NOSIG_ADJUST_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_NOSIG_ADJUST,&ts.waterfall_nosig_adjust,SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT,WATERFALL_NOSIG_ADJUST_MIN,WATERFALL_NOSIG_ADJUST_MAX},
    { ConfigEntry_UInt8, EEPROM_FFT_WINDOW,&ts.fft_window_type,FFT_WINDOW_DEFAULT,0,FFT_WINDOW_MAX},
    { ConfigEntry_UInt8, EEPROM_TXRX_SWITCH_AUDIO_MUTE_DELAY,&ts.txrx_switch_audio_muting_timing,TXRX_SWITCH_AUDIO_MUTE_DELAY_DEFAULT,0,TXRX_SWITCH_AUDIO_MUTE_DELAY_MAX},
    { ConfigEntry_UInt8, EEPROM_FILTER_DISP_COLOUR,&ts.filter_disp_colour,0,0,SPEC_MAX_COLOUR},
    { ConfigEntry_UInt32_16, EEPROM_FM_SUBAUDIBLE_TONE_GEN,&ts.fm_subaudible_tone_gen_select,FM_SUBAUDIBLE_TONE_OFF,0,NUM_SUBAUDIBLE_TONES},
    { ConfigEntry_UInt32_16, EEPROM_FM_SUBAUDIBLE_TONE_DET,&ts.fm_subaudible_tone_det_select,FM_SUBAUDIBLE_TONE_OFF,0,NUM_SUBAUDIBLE_TONES},
    { ConfigEntry_UInt8, EEPROM_FM_TONE_BURST_MODE,&ts.fm_tone_burst_mode,FM_TONE_BURST_OFF,0,FM_TONE_BURST_MAX},
    { ConfigEntry_UInt8, EEPROM_FM_SQUELCH_SETTING,&ts.fm_sql_threshold,FM_SQUELCH_DEFAULT,0,FM_SQUELCH_MAX},
    { ConfigEntry_UInt32_16, EEPROM_KEYBOARD_BEEP_FREQ,&ts.beep_frequency,DEFAULT_BEEP_FREQUENCY,MIN_BEEP_FREQUENCY,MAX_BEEP_FREQUENCY},
    { ConfigEntry_UInt8, EEPROM_BEEP_LOUDNESS,&ts.beep_loudness,DEFAULT_BEEP_LOUDNESS,0,MAX_BEEP_LOUDNESS},
    { ConfigEntry_UInt8, EEPROM_TUNE_POWER_LEVEL,&ts.tune_power_level,PA_LEVEL_TUNE_KEEP_CURRENT,PA_LEVEL_FULL,PA_LEVEL_TUNE_KEEP_CURRENT},
    { ConfigEntry_UInt8, EEPROM_CAT_XLAT,&ts.xlat,1,0,1},
    { ConfigEntry_UInt32_16, EEPROM_MANUAL_NOTCH,&ts.notch_frequency,800,200,5000},
    { ConfigEntry_UInt32_16, EEPROM_MANUAL_PEAK,&ts.peak_frequency,750,200,5000},
    { ConfigEntry_UInt8, EEPROM_DISPLAY_DBM,&ts.display_dbm,0,0,2},
    { ConfigEntry_Int32_16, EEPROM_DBM_CALIBRATE,&ts.dbm_constant,0,-100,100},
    { ConfigEntry_UInt8, EEPROM_S_METER,&ts.s_meter,0,0,2},
	{ ConfigEntry_Int32_16, EEPROM_BASS_GAIN,&ts.bass_gain,2,-20,20},
    { ConfigEntry_Int32_16, EEPROM_TREBLE_GAIN,&ts.treble_gain,0,-20,20},
    { ConfigEntry_UInt8, EEPROM_TX_FILTER,&ts.tx_filter,0,0,TX_FILTER_BASS},
	{ ConfigEntry_Int32_16, EEPROM_TX_BASS_GAIN,&ts.tx_bass_gain,4,-20,6},
    { ConfigEntry_Int32_16, EEPROM_TX_TREBLE_GAIN,&ts.tx_treble_gain,4,-20,6},
    { ConfigEntry_Int32_16, EEPROM_SAM_PLL_LOCKING_RANGE,&ads.pll_fmax_int,2500,50,8000},
    { ConfigEntry_Int32_16, EEPROM_SAM_PLL_STEP_RESPONSE,&ads.zeta_int,65,1,100},
    { ConfigEntry_Int32_16, EEPROM_SAM_PLL_BANDWIDTH,&ads.omegaN_int, 250,15,1000},
    { ConfigEntry_Int32_16, EEPROM_I2C1_SPEED,&ts.i2c_speed[0], I2C1_SPEED_DEFAULT,1,20},
    { ConfigEntry_Int32_16, EEPROM_I2C2_SPEED,&ts.i2c_speed[1], I2C2_SPEED_DEFAULT,1,20},
    { ConfigEntry_UInt8, EEPROM_SAM_FADE_LEVELER,&ads.fade_leveler,1,0,1},
    { ConfigEntry_UInt8, EEPROM_LINEOUT_GAIN,&ts.lineout_gain,LINEOUT_GAIN_DEFAULT,LINEOUT_GAIN_MIN,LINEOUT_GAIN_MAX},
    { ConfigEntry_Int16, EEPROM_RTC_CALIB,&ts.rtc_calib,RTC_CALIB_PPM_DEFAULT, RTC_CALIB_PPM_MIN, RTC_CALIB_PPM_MAX},

    UI_C_EEPROM_BAND_5W_PF( 0,80,m)
    UI_C_EEPROM_BAND_5W_PF(1,60,m)
    UI_C_EEPROM_BAND_5W_PF(2,40,m)
    UI_C_EEPROM_BAND_5W_PF(3,30,m)
    UI_C_EEPROM_BAND_5W_PF(4,20,m)
    UI_C_EEPROM_BAND_5W_PF(5,17,m)
    UI_C_EEPROM_BAND_5W_PF(6,15,m)
    UI_C_EEPROM_BAND_5W_PF(7,12,m)
    UI_C_EEPROM_BAND_5W_PF(8,10,m)
    UI_C_EEPROM_BAND_5W_PF(9,6,m)
    UI_C_EEPROM_BAND_5W_PF(10,4,m)
    UI_C_EEPROM_BAND_5W_PF(11,2,m)
    UI_C_EEPROM_BAND_5W_PF(12,70,cm)
    UI_C_EEPROM_BAND_5W_PF(13,23,cm)
    UI_C_EEPROM_BAND_5W_PF(14,2200,m)
    UI_C_EEPROM_BAND_5W_PF(15,630,m)
    UI_C_EEPROM_BAND_5W_PF(16,160,m)
    UI_C_EEPROM_BAND_FULL_PF(0,80,m)
    UI_C_EEPROM_BAND_FULL_PF(1,60,m)
    UI_C_EEPROM_BAND_FULL_PF(2,40,m)
    UI_C_EEPROM_BAND_FULL_PF(3,30,m)
    UI_C_EEPROM_BAND_FULL_PF(4,20,m)
    UI_C_EEPROM_BAND_FULL_PF(5,17,m)
    UI_C_EEPROM_BAND_FULL_PF(6,15,m)
    UI_C_EEPROM_BAND_FULL_PF(7,12,m)
    UI_C_EEPROM_BAND_FULL_PF(8,10,m)
    UI_C_EEPROM_BAND_FULL_PF(9,6,m)
    UI_C_EEPROM_BAND_FULL_PF(10,4,m)
    UI_C_EEPROM_BAND_FULL_PF(11,2,m)
    UI_C_EEPROM_BAND_FULL_PF(12,70,cm)
    UI_C_EEPROM_BAND_FULL_PF(13,23,cm)
    UI_C_EEPROM_BAND_FULL_PF(14,2200,m)
    UI_C_EEPROM_BAND_FULL_PF(15,630,m)
    UI_C_EEPROM_BAND_FULL_PF(16,160,m)
    // the entry below MUST be the last entry, and only at the last position Stop is allowed
    {
        ConfigEntry_Stop
    }
};

// TODO: LINEAR SEARCH IS VERY BAD FOR LARGER SETS, replace with more clever strategy.
// for use case in menu this acceptable, however
const ConfigEntryDescriptor* UiConfiguration_GetEntry(uint16_t id)
{
    int idx;
    const ConfigEntryDescriptor* retval = NULL;
    for (idx = 0; ConfigEntryInfo[idx].typeId != ConfigEntry_Stop; idx++)
    {
        if (ConfigEntryInfo[idx].id == id)
        {
            retval = &ConfigEntryInfo[idx];
            break;
        }
    }
    return retval;
}


/*
static void __attribute__ ((noinline)) UiReadSettingEEPROM_Bool(uint16_t addr, volatile bool* val_ptr, uint16_t default_val, uint16_t min_val, uint16_t max_val ) {
    uint16_t value;
    if(Read_EEPROM(addr, &value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults) {
            *val_ptr = default_val;
        }
    }
}
*/

static void __attribute__ ((noinline)) UiReadSettingEEPROM_UInt8(uint16_t addr, volatile uint8_t* val_ptr, uint16_t default_val, uint16_t min_val, uint16_t max_val )
{
    uint16_t value;
    if(ConfigStorage_ReadVariable(addr, &value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults)
        {
            *val_ptr = default_val;
        }
    }
}

static void __attribute__ ((noinline)) UiReadSettingEEPROM_UInt16(uint16_t addr, volatile uint16_t* val_ptr, uint16_t default_val, uint16_t min_val, uint16_t max_val )
{
    uint16_t value;
    if(ConfigStorage_ReadVariable(addr, &value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults)
        {
            *val_ptr = default_val;
        }
    }
}

static void __attribute__ ((noinline)) UiReadSettingEEPROM_Int16(uint16_t addr, volatile int16_t* val_ptr, int16_t default_val, int16_t min_val, int16_t max_val )
{
    int16_t value;
    if(ConfigStorage_ReadVariable(addr, (uint16_t*)&value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults)
        {
            *val_ptr = default_val;
        }
    }
}


static void __attribute__ ((noinline)) UiReadSettingEEPROM_UInt32_16(uint16_t addr, volatile uint32_t* val_ptr, uint16_t default_val, uint16_t min_val, uint16_t max_val )
{
    uint16_t value;
    if(ConfigStorage_ReadVariable(addr, &value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults)
        {
            *val_ptr = default_val;
        }
    }
}

static void __attribute__ ((noinline)) UiReadSettingEEPROM_Int32_16(uint16_t addr, volatile int32_t* val_ptr, int default_val, int min_val, int max_val )
{
    uint16_t value;
    if(ConfigStorage_ReadVariable(addr, &value) == 0)
    {
        *val_ptr = (int16_t)value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults)
        {
            *val_ptr = default_val;
        }
    }
}

static void UiReadSettingEEPROM_UInt32(uint16_t addrH, uint16_t addrL, volatile uint32_t* val_ptr, uint32_t default_val, uint32_t min_val, uint32_t max_val)
{
    uint16_t valueH,valueL;
    if(ConfigStorage_ReadVariable(addrH, &valueH) == 0 && ConfigStorage_ReadVariable(addrL, &valueL) == 0)
    {

        *val_ptr = valueH;
        *val_ptr <<=16;
        *val_ptr |= valueL;

        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults)
        {
            *val_ptr = default_val;
        }
    }
}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_UInt16(uint16_t addr, uint16_t set_val, uint16_t default_val )
{
    ConfigStorage_WriteVariable(addr, set_val);
}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_Int16(uint16_t addr, int16_t set_val, int16_t default_val )
{
    ConfigStorage_WriteVariable(addr, (uint16_t)set_val);
}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_UInt32(uint16_t addrH, uint16_t addrL, uint32_t set_val, uint32_t default_val )
{
    ConfigStorage_WriteVariable(addrH, (uint16_t)(set_val >> 16));
    ConfigStorage_WriteVariable(addrL, (uint16_t)(set_val));
}

void UiReadSettingsBandMode(const uint8_t i, const uint16_t band_mode, const uint16_t band_freq_high, const uint16_t  band_freq_low,__IO VfoReg* vforeg)
{
    uint32_t value32;
    uint16_t value16;

    UiReadSettingEEPROM_UInt16(band_mode + i,&value16,0,0,0xffff);
    {
        // Note that ts.band will, by definition, be equal to index "i"
        vforeg->decod_mode = (value16 >> 8) & 0x0F;     // demodulator mode might not be right for saved band!
        if((ts.dmod_mode > DEMOD_MAX_MODE)  || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)       // valid mode value from EEPROM? or defaults loaded?
            vforeg->decod_mode = DEMOD_LSB;         // no - set to LSB
        //
    }

    // Try to read Freq saved values
    UiReadSettingEEPROM_UInt32(band_freq_high + i, band_freq_low + i,&value32,bandInfo[i].tune + DEFAULT_FREQ_OFFSET,0,0xffffffff);
    {
        //
        // We have loaded from eeprom the last used band, but can't just
        // load saved frequency, as it could be out of band, so do a
        // boundary check first (also check to see if defaults should be loaded)
        //
        if((!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults) && (value32 >= bandInfo[i].tune) && (value32 <= (bandInfo[i].tune + bandInfo[i].size)))
        {
            vforeg->dial_value = value32;
        }
        else if((ts.flags2 & FLAGS2_FREQ_MEM_LIMIT_RELAX) && (!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults))
        {
            // xxxx relax memory-save frequency restrictions and is it within the allowed range?
            vforeg->dial_value = value32;
        }
        else
        {
            // Load default for this band
            vforeg->dial_value = bandInfo[i].tune + DEFAULT_FREQ_OFFSET;
        }
    }

}

/*
static void __attribute__ ((noinline)) UiWriteSettingEEPROM_Bool(uint16_t addr, bool set_val, bool default_val ) {
    UiWriteSettingEEPROM_UInt16(addr,(uint16_t)set_val,(uint16_t)default_val);
}
*/

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_UInt32_16(uint16_t addr, uint32_t set_val, uint16_t default_val )
{
    UiWriteSettingEEPROM_UInt16(addr,set_val,default_val);
}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_Int32_16(uint16_t addr, int32_t set_val, int32_t default_val )
{
    UiWriteSettingEEPROM_UInt16(addr,(uint16_t)(int16_t)set_val,default_val);
}


static void UiWriteSettingsBandMode(const uint16_t i,const uint16_t band_mode, const uint16_t band_freq_high, const uint16_t band_freq_low, __IO VfoReg* vforeg)
{

    // ------------------------------------------------------------------------------------
    // Read Band and Mode saved values - update if changed
    UiWriteSettingEEPROM_UInt16(band_mode + i,
                                (vforeg->decod_mode << 8),
                                ((vforeg->decod_mode & 0x0f) << 8)
                               );
    // Try to read Freq saved values - update if changed
    UiWriteSettingEEPROM_UInt32(band_freq_high+i,band_freq_low+i, vforeg->dial_value, vforeg->dial_value);
}

void UiWriteSettingEEPROM_Filter()
{
    int idx, mem_idx;
    for (idx = 0; idx < FILTER_MODE_MAX; idx++)
    {
        for (mem_idx = 0; mem_idx < FILTER_PATH_MEM_MAX; mem_idx++)
        {
            UiWriteSettingEEPROM_UInt16(EEPROM_FILTER_PATH_MAP_BASE+idx*FILTER_PATH_MEM_MAX+mem_idx,ts.filter_path_mem[idx][mem_idx],0);
        }
    }
}

void UiReadSettingEEPROM_Filter()
{
    int idx, mem_idx;
    for (idx = 0; idx < FILTER_MODE_MAX; idx++)
    {
        for (mem_idx = 0; mem_idx < FILTER_PATH_MEM_MAX; mem_idx++)
        {
            UiReadSettingEEPROM_UInt16(EEPROM_FILTER_PATH_MAP_BASE+idx*FILTER_PATH_MEM_MAX+mem_idx,&(ts.filter_path_mem[idx][mem_idx]),0,0,AUDIO_FILTER_PATH_NUM-1);
        }
    }
}

void UiConfiguration_ReadConfigEntryData(const ConfigEntryDescriptor* ced_ptr)
{
    switch(ced_ptr->typeId)
    {

    case ConfigEntry_UInt8:
        UiReadSettingEEPROM_UInt8(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
        break;
    case ConfigEntry_UInt16:
        UiReadSettingEEPROM_UInt16(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
        break;
    case ConfigEntry_UInt32_16:
        UiReadSettingEEPROM_UInt32_16(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
        break;
    case ConfigEntry_Int32_16:
        UiReadSettingEEPROM_Int32_16(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
        break;
    case ConfigEntry_Int16:
        UiReadSettingEEPROM_Int16(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
        break;

//  case ConfigEntry_Bool:
//    UiReadSettingEEPROM_Bool(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
//    break;

    }
}
void UiConfiguration_WriteConfigEntryData(const ConfigEntryDescriptor* ced_ptr)
{
    switch(ced_ptr->typeId)
    {

    case ConfigEntry_UInt8:
        UiWriteSettingEEPROM_UInt16(ced_ptr->id,*(uint8_t*)ced_ptr->val_ptr,ced_ptr->val_default);
        break;
    case ConfigEntry_UInt16:
        UiWriteSettingEEPROM_UInt16(ced_ptr->id,*(uint16_t*)ced_ptr->val_ptr,ced_ptr->val_default);
        break;
    case ConfigEntry_UInt32_16:
        UiWriteSettingEEPROM_UInt32_16(ced_ptr->id,*(uint32_t*)ced_ptr->val_ptr,ced_ptr->val_default);
        break;
    case ConfigEntry_Int32_16:
        UiWriteSettingEEPROM_Int32_16(ced_ptr->id,*(int32_t*)ced_ptr->val_ptr,ced_ptr->val_default);
        break;
    case ConfigEntry_Int16:
        UiWriteSettingEEPROM_Int16(ced_ptr->id,*(int16_t*)ced_ptr->val_ptr,ced_ptr->val_default);
        break;
//  case ConfigEntry_Bool:
//    UiWriteSettingEEPROM_Bool(ced_ptr->id,*(bool*)ced_ptr->val_ptr,ced_ptr->val_default);
//    break;
    }
}

void UiConfiguration_WriteConfigEntries()
{
    int idx;
    for (idx = 0; ConfigEntryInfo[idx].typeId != ConfigEntry_Stop ; idx++)
    {
        UiConfiguration_WriteConfigEntryData(&ConfigEntryInfo[idx]);
    }
}

static void __attribute__ ((noinline)) UiConfiguration_ReadConfigEntries()
{
    int idx;
    for (idx = 0; ConfigEntryInfo[idx].typeId != ConfigEntry_Stop ; idx++)
    {
        UiConfiguration_ReadConfigEntryData(&ConfigEntryInfo[idx]);
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
void UiConfiguration_LoadEepromValues(void)
{
    bool dspmode = ts.dsp_inhibit;
    ts.dsp_inhibit = 1;     // disable dsp while loading EEPROM data

    uint16_t value16;
    uint32_t value32;

    // Do a sample reads to "prime the pump" before we start...
    // This is to make the function work reliabily after boot-up
    UiReadSettingEEPROM_UInt16(EEPROM_ZERO_LOC_UNRELIABLE,&value16,0,0,0xffff);
    // Let's use location zero - which may not work reliably, anyway!

    UiReadSettingEEPROM_UInt16(EEPROM_FLAGS2,&ts.flags2,0,0,255);
    // ------------------------------------------------------------------------------------
    // Try to read Band and Mode saved values, but read freq-limit-settings before
    UiReadSettingEEPROM_UInt16(EEPROM_BAND_MODE,&value16,0,0,0xffff);
    {
        ts.band = value16 & 0x00FF;
        if(ts.band > MAX_BANDS-1)           // did we get an insane value from EEPROM?
            ts.band = BAND_MODE_80;     //  yes - set to 80 meters
        //
        ts.dmod_mode = (value16 >> 8) & 0x0F;       // demodulator mode might not be right for saved band!
        if((ts.dmod_mode > DEMOD_MAX_MODE)  || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)       // valid mode value from EEPROM? or defaults loaded?
            ts.dmod_mode = DEMOD_LSB;           // no - set to LSB

    }
    // ------------------------------------------------------------------------------------
    // Try to read Freq saved values
    UiReadSettingEEPROM_UInt32(EEPROM_FREQ_HIGH,EEPROM_FREQ_LOW,&value32,0,0,0xffffffff);
    {

        // We have loaded from eeprom the last used band, but can't just
        // load saved frequency, as it could be out of band, so do a
        // boundary check first (also check to see if defaults should be loaded)
        if((!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults) && (value32 >= bandInfo[ts.band].tune) && (value32 <= (bandInfo[ts.band].tune + bandInfo[ts.band].size)))
        {
            df.tune_new = value32;
        }
        else if((ts.flags2 & FLAGS2_FREQ_MEM_LIMIT_RELAX) && (!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults))       // xxxx relax memory-save frequency restrictions and is it within the allowed range?
        {
            df.tune_new = value32;
        }
        else
        {
            // Load default for this band
            df.tune_new = bandInfo[ts.band].tune;
        }
    }
    // Try to read saved per-band values for frequency, mode and filter


    uint8_t i;
    for(i = 0; i < MAX_BANDS; i++)
    {
        // read from stored bands
        // UiReadSettingsBandMode(i,EEPROM_BAND0_MODE,EEPROM_BAND0_FREQ_HIGH,EEPROM_BAND0_FREQ_LOW, &vfo[VFO_WORK].band[i]);
        UiReadSettingsBandMode(i,EEPROM_BAND0_MODE_A,EEPROM_BAND0_FREQ_HIGH_A,EEPROM_BAND0_FREQ_LOW_A, &vfo[VFO_A].band[i]);
        UiReadSettingsBandMode(i,EEPROM_BAND0_MODE_B,EEPROM_BAND0_FREQ_HIGH_B,EEPROM_BAND0_FREQ_LOW_B, &vfo[VFO_B].band[i]);
    }

    UiConfiguration_ReadConfigEntries();

    UiReadSettingEEPROM_UInt32( EEPROM_XVERTER_OFFSET_HIGH,EEPROM_XVERTER_OFFSET_LOW,&ts.xverter_offset,0,0,XVERTER_OFFSET_MAX);

    UiReadSettingEEPROM_Filter();

    // post configuration loading actions below
    df.tuning_step  = tune_steps[df.selected_idx];
    ts.tx_gain[TX_AUDIO_LINEIN_R] = ts.tx_gain[TX_AUDIO_LINEIN_L];
    // TODO: Right and Left Settings stored

    {
        ulong bias_val;
        bias_val = BIAS_OFFSET + (ts.pa_bias * 2);
        if(bias_val > 255)
            bias_val = 255;

        // Set DAC Channel1 DHR12L register with bias value
        HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_8B_R,bias_val);
    }

    ts.alc_decay_var = ts.alc_decay;

    ts.alc_tx_postfilt_gain_var =  ts.alc_tx_postfilt_gain; // "working" copy of variable

    // set xlate to -12KHz at first start
    if(ts.version_number_release == 0 && ts.version_number_minor == 0 && ts.version_number_major == 0 )
         {
        ts.iq_freq_mode = FREQ_IQ_CONV_MODE_DEFAULT;
    }

    ts.dsp_inhibit = dspmode;       // restore setting
}

// ********************************************************************************************************************
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSaveEepromValues
//* Object              : save all values to EEPROM - called on power-down.  Does not check to see if they have changed
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

uint16_t UiConfiguration_SaveEepromValues(void)
{
    uint16_t i, retval = 0x0;
    bool dspmode;
    uchar demodmode;

    if(ts.txrx_mode != TRX_MODE_RX)
    {
        retval = 0xFF00;
    }
    else
    {
        // disable DSP during write because it decreases speed tremendous
        dspmode = ts.dsp_inhibit;
        ts.dsp_inhibit = 1;
        //  ts.dsp_active &= 0xfa;  // turn off DSP

        // switch to SSB during write when in FM because it decreases speed tremendous
        demodmode = ts.dmod_mode;
        if(ts.dmod_mode == DEMOD_FM)
            ts.dmod_mode = DEMOD_USB;   // if FM switch to USB during write

        // TODO: THIS IS UGLY: We are switching to RAM based storage in order to gain speed
        // because we then can bulk write the data into the I2C later.
        // we don't do this for flash, since we cannot gain anything here.

        if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_I2C)
        {
            ConfigStorage_CopySerial2RAMCache();
        }

        if(ts.band < (MAX_BANDS) && ts.cat_band_index == 255)			// not in a sandbox
        {
            // save current band/frequency/mode settings
            vfo[is_vfo_b()?VFO_B:VFO_A].band[ts.band].dial_value = df.tune_new;
            // Save decode mode
            vfo[is_vfo_b()?VFO_B:VFO_A].band[ts.band].decod_mode = ts.dmod_mode;
            // use the "real" demod mode, instead of the possibly changed one (FM gets USB during save)
            // FIXME: Either we use demod_mode also here or we remove the special FM handling and use ts.dmod_mode everywehre
            // FIXME: Right now it is inconsistent and should be left like this.

            // TODO: move value to a static variable, so that it can be read/written with standard approach
            UiWriteSettingEEPROM_UInt16(EEPROM_BAND_MODE,
                                        (uint16_t)((uint16_t)ts.band| ((uint16_t)demodmode << 8)),
                                        (uint16_t)((uint16_t)ts.band |((uint16_t)demodmode << 8)));

            // TODO: move value to a static variable, so that it can be read/written with standard approach
            UiWriteSettingEEPROM_UInt32(EEPROM_FREQ_HIGH,EEPROM_FREQ_LOW, df.tune_new, df.tune_new);
        }
        else
            ts.cat_band_index = 255;

        // Save stored band/mode/frequency memory from RAM
        for(i = 0; i < MAX_BANDS; i++)      // scan through each band's frequency/mode data
        {
            // UiWriteSettingsBandMode(i,EEPROM_BAND0_MODE,EEPROM_BAND0_FREQ_HIGH,EEPROM_BAND0_FREQ_LOW,  &vfo[VFO_WORK].band[i]);
            UiWriteSettingsBandMode(i,EEPROM_BAND0_MODE_A,EEPROM_BAND0_FREQ_HIGH_A,EEPROM_BAND0_FREQ_LOW_A, &vfo[VFO_A].band[i]);
            UiWriteSettingsBandMode(i,EEPROM_BAND0_MODE_B,EEPROM_BAND0_FREQ_HIGH_B,EEPROM_BAND0_FREQ_LOW_B, &vfo[VFO_B].band[i]);
        }

        UiConfiguration_WriteConfigEntries();

        UiWriteSettingEEPROM_UInt32(EEPROM_XVERTER_OFFSET_HIGH,EEPROM_XVERTER_OFFSET_LOW,ts.xverter_offset,0);

        UiWriteSettingEEPROM_Filter();

        if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_RAMCACHE)
        {
            ConfigStorage_CopyRAMCache2Serial();
        }

        ts.dsp_inhibit = dspmode;   // restore DSP mode
        ts.dmod_mode = demodmode;   // restore active mode
    }
    return retval;
}
