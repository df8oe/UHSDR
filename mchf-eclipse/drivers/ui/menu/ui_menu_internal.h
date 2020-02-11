/*
 * ui_menu_internal.h
 *
 *  Created on: 24.12.2016
 *      Author: danilo
 */

#ifndef DRIVERS_UI_UI_MENU_INTERNAL_H_
#define DRIVERS_UI_UI_MENU_INTERNAL_H_

#include "uhsdr_types.h"
#define UiMenuDesc(a)

//
// Enumeration for menu.
enum
{
    MENU_DSP_NR_STRENGTH = 0,
    MENU_AM_DISABLE,
    MENU_CW_DISABLE,
    MENU_DIGI_DISABLE,
    MENU_SSB_AUTO_MODE_SELECT,
    MENU_CW_AUTO_MODE_SELECT,
    MENU_FM_MODE_ENABLE,
    MENU_FM_GEN_SUBAUDIBLE_TONE,
    MENU_FM_DET_SUBAUDIBLE_TONE,
    MENU_FM_TONE_BURST_MODE,
    MENU_FM_DEV_MODE,
//    MENU_AGC_MODE,
//    MENU_RF_GAIN_ADJ,
//    MENU_CUSTOM_AGC,
//    MENU_AGC_WDSP_SWITCH,
    MENU_AGC_WDSP_MODE,
    MENU_AGC_WDSP_SLOPE,
    MENU_AGC_WDSP_TAU_DECAY,
    MENU_AGC_WDSP_THRESH,
    MENU_AGC_WDSP_HANG_ENABLE,
    MENU_AGC_WDSP_HANG_TIME,
    MENU_AGC_WDSP_HANG_THRESH,
    MENU_AGC_WDSP_TAU_HANG_DECAY,
    MENU_CODEC_GAIN_MODE,
    MENU_NOISE_BLANKER_SETTING,
    MENU_RX_FREQ_CONV,
    MENU_MIC_LINE_MODE,
    MENU_MIC_TYPE,
    MENU_MIC_GAIN,
    MENU_LINE_GAIN,
    MENU_ALC_RELEASE,
    MENU_ALC_POSTFILT_GAIN,
    MENU_TX_COMPRESSION_LEVEL,
    MENU_KEYER_MODE,
    MENU_KEYER_SPEED,
    MENU_KEYER_WEIGHT,
    MENU_SIDETONE_GAIN,
    MENU_SIDETONE_FREQUENCY,
    MENU_PADDLE_REVERSE,
    MENU_CW_TX_RX_DELAY,
    MENU_CW_OFFSET_MODE,
	MENU_CW_DECODER,
	MENU_CW_DECODER_THRESH,
//	MENU_CW_DECODER_AVERAGE,
	MENU_CW_DECODER_BLOCKSIZE,
//	MENU_CW_DECODER_AGC,
	MENU_CW_DECODER_NOISECANCEL,
	MENU_CW_DECODER_SPIKECANCEL,
	MENU_CW_DECODER_USE_3_GOERTZEL,
	MENU_CW_DECODER_SNAP_ENABLE,
	MENU_CW_DECODER_SHOW_CW_LED,
    MENU_TCXO_MODE,
    MENU_TCXO_C_F,
    MENU_SCOPE_SPEED,
    MENU_SPECTRUM_FILTER_STRENGTH,
    MENU_SCOPE_TRACE_COLOUR,
    MENU_SCOPE_TRACE_HL_COLOUR,
	MENU_SCOPE_BACKGROUND_HL_COLOUR,
	MENU_SCOPE_GRID_COLOUR,
    MENU_SPECTRUM_FREQSCALE_COLOUR,
    MENU_SPECTRUM_MAGNIFY,
    MENU_SCOPE_AGC_ADJUST,
    MENU_SCOPE_DB_DIVISION,
    MENU_SPECTRUM_CENTER_LINE_COLOUR,
    MENU_SCOPE_LIGHT_ENABLE,
    MENU_SPECTRUM_MODE,
    MENU_WFALL_COLOR_SCHEME,
    MENU_WFALL_STEP_SIZE,
    MENU_WFALL_OFFSET,
    MENU_WFALL_CONTRAST,
    MENU_WFALL_SPEED,
//  MENU_SCOPE_NOSIG_ADJUST,
//  MENU_WFALL_NOSIG_ADJUST,
//  MENU_S_METER,
    MENU_METER_COLOUR_UP,
    MENU_METER_COLOUR_DOWN,
    MENU_DBM_DISPLAY,
    MENU_DBM_CALIBRATE,
	MENU_UI_INVERSE_SCROLLING,
	MENU_FREQ_FONT,
    MENU_SPECTRUM_SIZE,
    MENU_BACKUP_CONFIG,
    MENU_RESTORE_CONFIG,
    MENU_HARDWARE_INFO,
    MENU_DEMOD_SAM,
    MENU_SAM_PLL_LOCKING_RANGE,
    MENU_SAM_PLL_STEP_RESPONSE,
    MENU_SAM_PLL_BANDWIDTH,
    MENU_SAM_FADE_LEVELER,
//    CONFIG_SAM_PLL_TAUR,
//    CONFIG_SAM_PLL_TAUI,
//    CONFIG_SAM_SIDEBAND,
    MENU_CONFIG_ENABLE,
    MENU_RESTART_CODEC,
    CONFIG_FREQ_STEP_MARKER_LINE,
    CONFIG_STEP_SIZE_BUTTON_SWAP,
    CONFIG_BAND_BUTTON_SWAP,
    CONFIG_BANDEF_SELECT,
    CONFIG_TX_DISABLE,
    CONFIG_TX_OUT_ENABLE,
    CONFIG_AUDIO_MAIN_SCREEN_MENU_SWITCH,
    CONFIG_MUTE_LINE_OUT_TX,
    CONFIG_TXRX_SWITCH_AUDIO_MUTE,
    CONFIG_LCD_AUTO_OFF_MODE,
    CONFIG_VOLTMETER_CALIBRATION,
    MENU_LOW_POWER_SHUTDOWN,
    CONFIG_LOW_POWER_THRESHOLD,
    CONFIG_DISP_FILTER_BANDWIDTH,
    CONFIG_MAX_VOLUME,
//    CONFIG_MAX_RX_GAIN,
    CONFIG_LINEOUT_GAIN,
    CONFIG_BEEP_ENABLE,
    CONFIG_BEEP_FREQ,
    CONFIG_BEEP_VOLUME,
    CONFIG_CAT_ENABLE,
    CONFIG_FREQUENCY_CALIBRATE,
    CONFIG_FREQ_LIMIT_RELAX,
    CONFIG_FREQ_MEM_LIMIT_RELAX,
    CONFIG_IQ_AUTO_CORRECTION,

    CONFIG_80M_RX_IQ_GAIN_BAL,
    CONFIG_80M_RX_IQ_PHASE_BAL,
    CONFIG_10M_RX_IQ_GAIN_BAL,
    CONFIG_10M_RX_IQ_PHASE_BAL,
    CONFIG_80M_TX_IQ_GAIN_BAL,
    CONFIG_80M_TX_IQ_PHASE_BAL,
    CONFIG_10M_TX_IQ_GAIN_BAL,
    CONFIG_10M_TX_IQ_PHASE_BAL,
    CONFIG_80M_TX_IQ_GAIN_BAL_TRANS_OFF,
    CONFIG_80M_TX_IQ_PHASE_BAL_TRANS_OFF,
    CONFIG_10M_TX_IQ_GAIN_BAL_TRANS_OFF,
    CONFIG_10M_TX_IQ_PHASE_BAL_TRANS_OFF,

    CONFIG_20M_TX_IQ_GAIN_BAL,
    CONFIG_20M_TX_IQ_PHASE_BAL,
    CONFIG_15M_TX_IQ_GAIN_BAL,
    CONFIG_15M_TX_IQ_PHASE_BAL,
    CONFIG_10M_UP_TX_IQ_GAIN_BAL,
    CONFIG_10M_UP_TX_IQ_PHASE_BAL,
    CONFIG_20M_TX_IQ_GAIN_BAL_TRANS_OFF,
    CONFIG_20M_TX_IQ_PHASE_BAL_TRANS_OFF,
    CONFIG_15M_TX_IQ_GAIN_BAL_TRANS_OFF,
    CONFIG_15M_TX_IQ_PHASE_BAL_TRANS_OFF,
    CONFIG_10M_UP_TX_IQ_GAIN_BAL_TRANS_OFF,
    CONFIG_10M_UP_TX_IQ_PHASE_BAL_TRANS_OFF,


    CONFIG_VSWR_PROTECTION_THRESHOLD,
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
#ifdef OBSOLETE_NR
    CONFIG_DSP_NR_DECORRELATOR_BUFFER_LENGTH,
    CONFIG_DSP_NR_FFT_NUMTAPS,
    CONFIG_DSP_NR_POST_AGC_SELECT,
    CONFIG_DSP_NOTCH_CONVERGE_RATE,
    CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH,
    CONFIG_DSP_NOTCH_FFT_NUMTAPS,
#endif
#ifdef USE_LMS_AUTONOTCH
    CONFIG_DSP_NOTCH_CONVERGE_RATE,
    CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH,
    CONFIG_DSP_NOTCH_FFT_NUMTAPS,
#endif
//    CONFIG_AGC_TIME_CONSTANT,
    CONFIG_AM_TX_FILTER_DISABLE,
//    CONFIG_SSB_TX_FILTER_DISABLE,
    CONFIG_SSB_TX_FILTER,
    CONFIG_TUNE_POWER_LEVEL,
    CONFIG_TUNE_TONE_MODE,
//    CONFIG_SPECTRUM_FFT_WINDOW_TYPE,
    CONFIG_RESET_SER_EEPROM,
    CONFIG_RESET_SER_EEPROM_SIGNATURE,
    CONFIG_CAT_IN_SANDBOX,
    CONFIG_CAT_XLAT,
    CONFIG_CAT_PTT_RTS,
    CONFIG_REDUCE_POWER_ON_LOW_BANDS,
    CONFIG_REDUCE_POWER_ON_HIGH_BANDS,
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
//    MENU_FP_SAM_01,
//    MENU_FP_SAM_02,
//    MENU_FP_SAM_03,
//    MENU_FP_SAM_04,
    MENU_DEBUG_TX_AUDIO,
    CONFIG_I2C1_SPEED,
    CONFIG_I2C2_SPEED,
	MENU_DEBUG_ENABLE_INFO,
    MENU_DEBUG_CLONEOUT,
    MENU_DEBUG_CLONEIN,
//    MENU_DEBUG_NEW_NB,
//	MENU_DEBUG_NR_ENABLE,
//	MENU_DEBUG_NR_GAIN_SMOOTH_ENABLE,
	MENU_DEBUG_NR_GAIN_SHOW,
//	MENU_DEBUG_NR_LONG_TONE_ENABLE,
//	MENU_DEBUG_NR_LONG_TONE_THRESH,
//	MENU_DEBUG_NR_LONG_TONE_ALPHA,
//	MENU_DEBUG_NR_GAIN_SMOOTH_ALPHA,
//	MENU_DEBUG_NR_ALPHA,
//	MENU_DEBUG_NR_THRESH,
//	MENU_DEBUG_NR_VAD_TYPE,
//	MENU_DEBUG_NR_VAD_DELAY,
	MENU_DEBUG_NR_BETA,
//	MENU_DEBUG_NR_Mode,
//#if defined(STM32F7) || defined(STM32H7)
//	MENU_DEBUG_NR_FFT_SIZE,
//	MENU_DEBUG_NR_DEC_ENABLE,
//#endif
	MENU_DEBUG_NR_ASNR,
	MENU_DEBUG_NR_GAIN_SMOOTH_WIDTH,
	MENU_DEBUG_NR_GAIN_SMOOTH_THRESHOLD,
//	MENU_DEBUG_RTTY_ATC,
#ifdef USE_TWO_CHANNEL_AUDIO
	MENU_DEBUG_ENABLE_STEREO,
#endif
	//	MENU_DEBUG_CW_DECODER,
//	MENU_DEBUG_LEAKY_LMS,
//	MENU_DEBUG_ANR_TAPS,
//	MENU_DEBUG_ANR_DELAY,
//	MENU_DEBUG_ANR_GAIN,
//	MENU_DEBUG_ANR_LEAK,
	MENU_DEBUG_OSC_SI5351_PLLRESET,
	MENU_DEBUG_HMC1023_COARSE,
	MENU_DEBUG_HMC1023_FINE,
    MENU_DEBUG_HMC1023_GAIN,
    MENU_DEBUG_HMC1023_BYPASS,
    MENU_DEBUG_HMC1023_OPAMP,
    MENU_DEBUG_HMC1023_DRVR,
    MENU_DEBUG_TWINPEAKS_CORR_RUN,
    MENU_DEBUG_FREEDV_MODE,
    MENU_DEBUG_FREEDV_SQL_THRESHOLD,

    CONFIG_RTC_START,
    CONFIG_RTC_HOUR,
    CONFIG_RTC_MIN,
    CONFIG_RTC_SEC,
    CONFIG_RTC_RESET,
    CONFIG_RTC_CALIB,
    MENU_DYNAMICTUNE,
    MENU_DIGITAL_MODE_SELECT,
    MENU_DEBUG_CW_OFFSET_SHIFT_KEEP_SIGNAL,
    CONFIG_SMETER_ATTACK,
    CONFIG_SMETER_DECAY,
    MENU_DEBUG_SMOOTH_DYN_TUNE,
    MAX_RADIO_CONFIG_ITEM   // Number of radio configuration menu items - This must ALWAYS remain as the LAST item!
};


// menu entry kind constants
enum MENU_KIND
{
    MENU_STOP = 0, // last entry in a menu / group
    MENU_ITEM, // standard menu entry
    MENU_GROUP, // menu group entry
    MENU_INFO, // just like a normal entry (read-only) but just for display purposes.
    MENU_SEP, // separator line
    MENU_BLANK, // blank
    MENU_TEXT,	// text output only
};


struct  MenuGroupDescriptor_s;

// items are stored in RAM
// Each menu group has to have a MenuGroupItem pointing to the descriptor
// a MenuGroupItem is used to keep track of the fold/unfold state and to link the
// Descriptors are stored in flash

typedef struct
{
    const uint16_t menuId; // backlink to the menu we are part of. That implies, an entry can only be part of a single menu group
    const uint16_t kind; // use the enum defined above to indicate what this entry represents
    const uint16_t number; // this is an identification number which is passed to the menu entry handled
    // for standard items it is the id of the value to be changed, intepretation is left to handler
    // MENU_GROUP: for menu groups this MUST BE the index in the menu group table, THIS IS USED INTERNALLY
    volatile bool* enabled;      // pointer to a variable which contains a boolean value to tell us if the menu entry is enabled right now, set to NULL if always enabled
    // this was a visual 3 letter identification which may be display, depending on the render approach
    const char* label;     // this is the label which will be display, depending on the render approach
} MenuDescriptor;

typedef struct
{
    bool unfolded;            // runtime variable, tells if the user wants to have this groups items to be shown
    uint16_t count;           // number of menu entries. This will be filled automatically on first use by internal code
    // do not write to this variable unless you know what you are doing.
    const MenuDescriptor* me; // pointer to the MenuDescriptor of this menu group in its parent menu. This is the backlink to our parent.
    // This will be filled automatically on first use by internal code in order to avoid search through the menu structure.
    // do not write to this variable unless you know what you are doing.
} MenuGroupState;


// This data structure is intended to be placed in flash
// all data is placed here at compile time
typedef struct MenuGroupDescriptor_s
{
    const MenuDescriptor* entries;          // array of member entries in the menu group
    MenuGroupState* state;                  // writable data structure for menu management, pointer has to go into RAM
    const MenuDescriptor* parent;           // pointer to the first element of the array in which our menu group is located in. It does not have
    // to point to the MENU_GROUP item, wich can be at any position in this array. Used to calculate the real
    // pointer later and to identify the parent menu group of this menu.
    // use NULL for top level menus here (i.e. no parent).
} MenuGroupDescriptor;


// Runtime management of menu entries for onscreen rendering.
typedef struct
{
    const MenuDescriptor* entryItem;
} MenuDisplaySlot;

#define MENU_START_IDX 0

extern const MenuGroupDescriptor groups[];

void UiMenu_UpdateItem(uint16_t select, MenuProcessingMode_t mode, int pos, int var, char* options, const char** txt_ptr_ptr, uint32_t* clr_ptr);
void UiMenu_DisplayValue(const char* value,uint32_t clr,uint16_t pos);

#endif /* DRIVERS_UI_UI_MENU_INTERNAL_H_ */
