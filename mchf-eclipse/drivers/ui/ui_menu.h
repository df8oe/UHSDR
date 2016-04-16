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

#ifndef __UI_MENU_H
#define __UI_MENU_H
//
// Exports
//
void UiMenu_MapColors(uint32_t color ,char* options,volatile uint32_t* clr_ptr);


// The supported mode values
enum {
  MENU_RENDER_ONLY = 0,
  MENU_PROCESS_VALUE_CHANGE,
  MENU_PROCESS_VALUE_SETDEFAULT,
};

void UiMenu_RenderMenu(uint16_t mode);

void UiMenu_RenderChangeItemValue(int16_t pot_diff);
void UiMenu_RenderChangeItem(int16_t pot_diff);
void UiMenu_RenderLastScreen();
void UiMenu_RenderFirstScreen();
bool UiMenu_RenderNextScreen(); // returns true if screen was changed, i.e. not last screen
bool UiMenu_RenderPrevScreen(); // returns true if screen was changed, i.e. not first screen

//
#define	MENUSIZE	6				// number of menu items per page/screen
//
// Enumeration for main menu.
// These items MUST be listed below in the order that they appear on the screen!
//
//
enum {
	MENU_DSP_NR_STRENGTH = 0,
	MENU_AM_DISABLE,
	MENU_SSB_AUTO_MODE_SELECT,
	MENU_FM_MODE_ENABLE,
	MENU_FM_GEN_SUBAUDIBLE_TONE,
	MENU_FM_DET_SUBAUDIBLE_TONE,
	MENU_FM_TONE_BURST_MODE,
//	MENU_FM_RX_BANDWIDTH,
	MENU_FM_DEV_MODE,
	MENU_AGC_MODE,
	MENU_RF_GAIN_ADJ,
	MENU_CUSTOM_AGC,
	MENU_CODEC_GAIN_MODE,
	MENU_NOISE_BLANKER_SETTING,
	MENU_RX_FREQ_CONV,
	MENU_MIC_LINE_MODE,
	MENU_MIC_GAIN,
	MENU_LINE_GAIN,
	MENU_ALC_RELEASE,
	MENU_ALC_POSTFILT_GAIN,
	MENU_TX_COMPRESSION_LEVEL,
	MENU_KEYER_MODE,
	MENU_KEYER_SPEED,
	MENU_SIDETONE_GAIN,
	MENU_SIDETONE_FREQUENCY,
	MENU_PADDLE_REVERSE,
	MENU_CW_TX_RX_DELAY,
	MENU_CW_OFFSET_MODE,
	MENU_TCXO_MODE,
	MENU_TCXO_C_F,
	MENU_SPEC_SCOPE_SPEED,
	MENU_SCOPE_FILTER_STRENGTH,
	MENU_SCOPE_TRACE_COLOUR,
	MENU_SCOPE_GRID_COLOUR,
	MENU_SCOPE_SCALE_COLOUR,
	MENU_SCOPE_MAGNIFY,
	MENU_SCOPE_AGC_ADJUST,
	MENU_SCOPE_DB_DIVISION,
	MENU_SCOPE_CENTER_LINE_COLOUR,
	MENU_SCOPE_LIGHT_ENABLE,
	MENU_SCOPE_MODE,
	MENU_WFALL_COLOR_SCHEME,
	MENU_WFALL_STEP_SIZE,
	MENU_WFALL_OFFSET,
	MENU_WFALL_CONTRAST,
	MENU_WFALL_SPEED,
	MENU_SCOPE_NOSIG_ADJUST,
	MENU_WFALL_NOSIG_ADJUST,
	MENU_WFALL_SIZE,
	MENU_BACKUP_CONFIG,
	MENU_RESTORE_CONFIG,
	MENU_HARDWARE_INFO,
	MENU_DEMOD_SAM,
	MENU_DUMMY_LINE_2,
	MENU_DUMMY_LINE_3,
	MENU_DUMMY_LINE_4,
	MENU_DUMMY_LINE_5,
	MENU_CONFIG_ENABLE,
	//
	MAX_MENU_ITEM	// Number of menu items - This must ALWAYS remain as the LAST item!
};
//
//
// Enumeration for configuration menu.
// These items MUST be listed below in the order that they appear!
//
enum {
	CONFIG_FREQ_STEP_MARKER_LINE = MAX_MENU_ITEM,
	CONFIG_STEP_SIZE_BUTTON_SWAP,
	CONFIG_BAND_BUTTON_SWAP,
	CONFIG_TX_DISABLE,
	CONFIG_AUDIO_MAIN_SCREEN_MENU_SWITCH,
	CONFIG_MUTE_LINE_OUT_TX,
	CONFIG_TX_AUDIO_MUTE,
	CONFIG_LCD_AUTO_OFF_MODE,
	CONFIG_VOLTMETER_CALIBRATION,
	CONFIG_DISP_FILTER_BANDWIDTH,
	CONFIG_MAX_VOLUME,
	CONFIG_MAX_RX_GAIN,
	CONFIG_BEEP_ENABLE,
	CONFIG_BEEP_FREQ,
	CONFIG_BEEP_LOUDNESS,
	CONFIG_CAT_ENABLE,
	CONFIG_FREQUENCY_CALIBRATE,
	CONFIG_FREQ_LIMIT_RELAX,
	CONFIG_FREQ_MEM_LIMIT_RELAX,
	CONFIG_LSB_RX_IQ_GAIN_BAL,
	CONFIG_LSB_RX_IQ_PHASE_BAL,
	CONFIG_USB_RX_IQ_GAIN_BAL,
	CONFIG_USB_RX_IQ_PHASE_BAL,
	CONFIG_AM_RX_GAIN_BAL,
	CONFIG_AM_RX_PHASE_BAL,
	CONFIG_FM_RX_GAIN_BAL,
	CONFIG_LSB_TX_IQ_GAIN_BAL,
	CONFIG_LSB_TX_IQ_PHASE_BAL,
	CONFIG_USB_TX_IQ_GAIN_BAL,
	CONFIG_USB_TX_IQ_PHASE_BAL,
	CONFIG_AM_TX_GAIN_BAL,
	CONFIG_FM_TX_GAIN_BAL,
	CONFIG_CW_PA_BIAS,
	CONFIG_PA_BIAS,
	CONFIG_FWD_REV_PWR_DISP,
	CONFIG_RF_FWD_PWR_NULL,
	CONFIG_FWD_REV_COUPLING_2200M_ADJ,
	CONFIG_FWD_REV_COUPLING_630M_ADJ,
	CONFIG_FWD_REV_COUPLING_160M_ADJ,
	CONFIG_FWD_REV_COUPLING_80M_ADJ,
	CONFIG_FWD_REV_COUPLING_40M_ADJ,
	CONFIG_FWD_REV_COUPLING_20M_ADJ,
	CONFIG_FWD_REV_COUPLING_15M_ADJ,
	CONFIG_FWD_REV_COUPLING_6M_ADJ,
	CONFIG_FWD_REV_COUPLING_2M_ADJ,
	CONFIG_FWD_REV_COUPLING_70CM_ADJ,
	CONFIG_FWD_REV_COUPLING_23CM_ADJ,
	CONFIG_FWD_REV_SENSE_SWAP,
	CONFIG_XVTR_OFFSET_MULT,
	CONFIG_XVTR_FREQUENCY_OFFSET,
	CONFIG_2200M_5W_ADJUST,
	CONFIG_630M_5W_ADJUST,
	CONFIG_160M_5W_ADJUST,
	CONFIG_80M_5W_ADJUST,
	CONFIG_60M_5W_ADJUST,
	CONFIG_40M_5W_ADJUST,
	CONFIG_30M_5W_ADJUST,
	CONFIG_20M_5W_ADJUST,
	CONFIG_17M_5W_ADJUST,
	CONFIG_15M_5W_ADJUST,
	CONFIG_12M_5W_ADJUST,
	CONFIG_10M_5W_ADJUST,
	CONFIG_6M_5W_ADJUST,
	CONFIG_4M_5W_ADJUST,
	CONFIG_2M_5W_ADJUST,
	CONFIG_70CM_5W_ADJUST,
	CONFIG_23CM_5W_ADJUST,
	CONFIG_2200M_FULL_POWER_ADJUST,
	CONFIG_630M_FULL_POWER_ADJUST,
	CONFIG_160M_FULL_POWER_ADJUST,
	CONFIG_80M_FULL_POWER_ADJUST,
	CONFIG_60M_FULL_POWER_ADJUST,
	CONFIG_40M_FULL_POWER_ADJUST,
	CONFIG_30M_FULL_POWER_ADJUST,
	CONFIG_20M_FULL_POWER_ADJUST,
	CONFIG_17M_FULL_POWER_ADJUST,
	CONFIG_15M_FULL_POWER_ADJUST,
	CONFIG_12M_FULL_POWER_ADJUST,
	CONFIG_10M_FULL_POWER_ADJUST,
	CONFIG_6M_FULL_POWER_ADJUST,
	CONFIG_4M_FULL_POWER_ADJUST,
	CONFIG_2M_FULL_POWER_ADJUST,
	CONFIG_70CM_FULL_POWER_ADJUST,
	CONFIG_23CM_FULL_POWER_ADJUST,
	CONFIG_DSP_NR_DECORRELATOR_BUFFER_LENGTH,
	CONFIG_DSP_NR_FFT_NUMTAPS,
	CONFIG_DSP_NR_POST_AGC_SELECT,
	CONFIG_DSP_NOTCH_CONVERGE_RATE,
	CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH,
	CONFIG_DSP_NOTCH_FFT_NUMTAPS,
	CONFIG_AGC_TIME_CONSTANT,
	CONFIG_AM_TX_FILTER_DISABLE,
	CONFIG_SSB_TX_FILTER_DISABLE,
	CONFIG_TUNE_POWER_LEVEL,
	CONFIG_FFT_WINDOW_TYPE,
	CONFIG_RESET_SER_EEPROM,
	CONFIG_CAT_IN_SANDBOX,
	CONFIG_CAT_XLAT,
	CONFIG_REDUCE_POWER_ON_LOW_BANDS,
	MENU_FP_CW_01,
	MENU_FP_CW_02,
	MENU_FP_CW_03,
	MENU_FP_CW_04,
	MENU_FP_AM_01,
	MENU_FP_AM_02,
	MENU_FP_AM_03,
	MENU_FP_AM_04,
	MENU_FP_SSB_01,
	MENU_FP_SSB_02,
	MENU_FP_SSB_03,
	MENU_FP_SSB_04,
	MENU_FP_SAM_01,
	MENU_FP_SAM_02,
	MENU_FP_SAM_03,
	MENU_FP_SAM_04,
	//
	MAX_RADIO_CONFIG_ITEM	// Number of radio configuration menu items - This must ALWAYS remain as the LAST item!
};
//
// Starting position of configuration menu
//
#define POS_MENU_IND_X                      60      // X position of description of menu item being changed
#define POS_MENU_IND_Y                      128     // Y position of first (top) item being changed
#define POS_MENU_CHANGE_X                   244     // Position of variable being changed
#define POS_MENU_CURSOR_X                   311     // Position of cursor used to indicate selected item

#endif
