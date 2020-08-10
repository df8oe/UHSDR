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
#ifndef __MCHF_BOARD_H
#define __MCHF_BOARD_H

#include "uhsdr_board_config.h"
#include "uhsdr_mcu.h"

#include "uhsdr_types.h"
#include "audio_filter.h"
#include "osc_interface.h"
#include "ui_lcd_layouts.h"
#include "ui_lcd_hy28.h"
#include "ui_vkeybrd.h"
#include "audio_driver.h"

struct mchf_waterfall
{
    uint8_t	color_scheme;			// stores waterfall color scheme
    uint8_t	vert_step_size;		// vertical step size in waterfall mode
    // int32_t	offset;			// offset for waterfall display
    uint32_t	contrast;			// contrast setting for waterfall display
	uint8_t	speed;	// speed of update of the waterfall
	// uint8_t	nosig_adjust;			// Adjustment for no signal adjustment conditions for waterfall
	uint16_t scheduler;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define TRX_MODE_RX				0
#define TRX_MODE_TX				1

typedef enum {
    DEMOD_USB       =   0,
    DEMOD_LSB       =   1,
    DEMOD_CW        =   2,
    DEMOD_AM        =   3,
    DEMOD_SAM       =   4,
    DEMOD_FM        =   5,
    DEMOD_DIGI		=   6,
#ifdef USE_TWO_CHANNEL_AUDIO
    DEMOD_SSBSTEREO =   7,
    DEMOD_IQ        =   8,
#endif
    DEMOD_NUM_MODE
} DemodModes_t;

#define DEMOD_MAX_MODE (DEMOD_NUM_MODE-1)

// codec x demod
// analog USB LSB CW AM FM SAM
// FreeDV USB LSB -  -  -  -


#define		MIN_FREQ_CAL		-1499		// Minimum and maximum range of frequency calibration in 10xppm
#define		MAX_FREQ_CAL		1499

#define MAX_BANDS               17      // Highest band number:  17 = General coverage (RX only) band
#define MAX_BAND_NUM            (MAX_BANDS+1)       // Number of Bands


// opposed to the RF_BRD_MCHF / RF_BRD_OVI40 which are
// compile time constants, the FOUND_RF_BOARD_xx is a runtime detected property
typedef enum {
    FOUND_RF_BOARD_MCHF = 0,
    FOUND_RF_BOARD_OVI40 = 1,
} RfBoard_t;

#define CW_KEYER_MODE_IAM_B				0
#define CW_KEYER_MODE_IAM_A				1
#define CW_KEYER_MODE_STRAIGHT			2
#define CW_KEYER_MODE_ULTIMATE			3
#define CW_KEYER_MAX_MODE				3

#define	SSB_TUNE_FREQ			750	// Frequency at which the SSB TX IQ gain and phase adjustment is to be done
//
#define VOICE_TX2RX_DELAY_DEFAULT			450	// Delay for switching when going from TX to RX (this is 0.66uS units)
//

// IQ source RX demodulation
enum
{
    RX_IQ_CODEC = 0,    // IQ from codec
    RX_IQ_DIGIQ,        // IQ from USB audio
    RX_IQ_DIG,          // demodulated audio
    RX_IQ_NUM
};
// Audio sources for TX modulation
#define TX_AUDIO_MIC			0
#define TX_AUDIO_LINEIN_L		1
#define TX_AUDIO_LINEIN_R		2
#define TX_AUDIO_DIG			3
#define TX_AUDIO_DIGIQ			4
#define TX_AUDIO_MAX_ITEMS		4
#define TX_AUDIO_NUM			(TX_AUDIO_MAX_ITEMS +1)
//
#define	LINE_GAIN_MIN			3
#define	LINE_GAIN_MAX			31
#define	LINE_GAIN_DEFAULT		12		// Original fixed gain setting
//
#define	MIC_GAIN_MIN			2
#define	MIC_GAIN_MAX			99
#define	MIC_GAIN_DEFAULT		15		// Default value - close to original fixed setting
//
//
#define	TX_POWER_FACTOR_MIN		3		// Minimum power factor setting (3 = 0.03)
#define TX_POWER_FACTOR_MAX_INTERNAL 0.55 // we limit power factor  to 55 (.55) . This limit is independent of the possible scale factor 4 for the power factor
#define	TX_POWER_FACTOR_MAX		(TX_POWER_FACTOR_MAX_INTERNAL*400.0)		// Please keep in mind that this is only a setting value maximum. Depending on the flags this reduced by 4 before further use.
                                        //And the true maximum is defined above in TX_POWER_FACTOR_MAX_INTERNAL

//
// Default power factors for 5 watt and FULL settings in percent
// These power factors are based on the original fixed values
//
#define TX_POWER_FACTOR_80_DEFAULT	0
#define	TX_POWER_FACTOR_60_DEFAULT	0
#define	TX_POWER_FACTOR_40_DEFAULT	0
#define	TX_POWER_FACTOR_30_DEFAULT	0
#define	TX_POWER_FACTOR_20_DEFAULT	0
#define	TX_POWER_FACTOR_17_DEFAULT	0
#define	TX_POWER_FACTOR_15_DEFAULT	0
#define	TX_POWER_FACTOR_12_DEFAULT	0
#define	TX_POWER_FACTOR_10_DEFAULT	0
#define TX_POWER_FACTOR_6_DEFAULT	0
#define TX_POWER_FACTOR_4_DEFAULT	0
#define TX_POWER_FACTOR_2_DEFAULT	0
#define TX_POWER_FACTOR_70_DEFAULT	0
#define TX_POWER_FACTOR_23_DEFAULT	0
#define TX_POWER_FACTOR_2200_DEFAULT	0
#define TX_POWER_FACTOR_630_DEFAULT	0
#define TX_POWER_FACTOR_160_DEFAULT	0
//
// Enumeration of colours used in spectrum scope display
//
typedef enum
{
    SPEC_WHITE = 0,
    SPEC_GREY,
    SPEC_BLUE,
    SPEC_RED1,
    SPEC_RED2,
    SPEC_RED3,
    SPEC_MAGENTA,
    SPEC_GREEN,
    SPEC_CYAN,
    SPEC_YELLOW,
    SPEC_ORANGE,
    SPEC_CREAM,
    SPEC_BLACK,
    SPEC_GREY1,
    SPEC_GREY2,
    SPEC_GREY3,
    SPEC_GREY4,
    SPEC_GREY5,
    SPEC_GREY6,
    SPEC_MAX_COLOUR,
} mchf_color_t;

typedef struct {
    const uint32_t value;
    const char* name;
} ColorNameValue;

// this data structure needs to be in sync with the color definitions above
// otherwise strange colors may become visible in the UI.
extern const ColorNameValue MchfColor_Id2ValueName[SPEC_MAX_COLOUR];

// Enumeration of transmit meter modes
enum
{
    METER_SWR = 0,
    METER_AUDIO,
    METER_ALC,
    METER_MAX,
};
//
#define	BACKLIGHT_BLANK_TIMING_DEFAULT	8		// default number of SECONDS for backlight blanking
#define LCD_STARTUP_BLANKING_TIME	3000		// number of DECISECONDS (e.g. SECONDS * 100) after power-up before LCD blanking occurs if no buttons are pressed/knobs turned
#define LOW_POWER_SHUTDOWN_DELAY_TIME   6000        // number of DECISECONDS after power-up before low power auto shutdown is checked



// Enumeration of transmit tune  modes
typedef enum
{
    TUNE_TONE_SINGLE = 0,
    TUNE_TONE_TWO,
    TUNE_TONE_MAX,
} TuneToneMode;


typedef struct Gain_s
{
    uint8_t value;
    uint8_t max;
    uint8_t value_old;
    float32_t   active_value;
} Gain;

typedef struct {
    int32_t value[IQ_TRANS_NUM];
} iq_balance_data_t;

#define KEYER_BUTTONS 3
#define KEYER_BUTTON_NONE -1
#define KEYER_BUTTON_1 0
#define KEYER_BUTTON_2 1
#define KEYER_BUTTON_3 2
#define KEYER_MACRO_LEN 200
#define KEYER_CAP_LEN 6
typedef struct {
	bool active;
	int8_t button_recording;
	uint8_t macro[KEYER_BUTTONS][KEYER_MACRO_LEN];
	uint8_t cap[KEYER_BUTTONS][KEYER_CAP_LEN + 1];
}
keyer_mode_t;

//
// Bands tuning values - WORKING registers - used "live" during transceiver operation
// (May contain VFO A, B or "Memory" channel values)
//
typedef struct vfo_reg_s
{
    uint32_t dial_value;
    uint8_t  decod_mode;
    uint8_t  digital_mode;
//    uint32_t filter_mode;
} VfoReg;

typedef struct BandInfo BandInfo; // forward declaration of BandInfo data type, we need this to be able to make a pointer to it.

// Transceiver state public structure
typedef struct TransceiverState
{
    // Sampling rate public flag
    uint32_t 	samp_rate;

    // Virtual pots public values
    int16_t  	rit_value;

#define RX_AUDIO_SPKR 0
#define RX_AUDIO_DIG  1
    Gain    rx_gain[2]; //ts.rx_gain[RX_AUDIO_SPKR].value

    uint8_t lineout_gain;            // lineout gain to control lineout level


#define MAX_RF_CODEC_GAIN_VAL       9       // Maximum RF gain setting
#define DEFAULT_RF_CODEC_GAIN_VAL   9       // Default RF gain setting (9 = AUTO mode)
#define RF_CODEC_GAIN_AUTO   9       // Default RF gain setting (9 = AUTO mode)

    uint8_t	rf_codec_gain;		// gain for codec (A/D converter) in receive mode
    uint8_t	cw_sidetone_gain;
    uint8_t	pa_bias;
    uint8_t	pa_cw_bias;

    // timer for muting of input into signal processing chains (TX/RX)
    uint16_t    audio_processor_input_mute_counter;


    iq_float_t tx_adj_gain_var[IQ_TRANS_NUM];    // active variables for adjusting tx gain balance
    iq_float_t rx_adj_gain_var;    // active variables for adjusting rx gain balance

    // Equalisation factor
    float32_t	tx_power_factor;

    int	freq_cal;				// frequency calibration

    // Frequency synthesizer
    uint32_t	tune_freq;			// main synthesizer frequency
    uint32_t	tune_freq_req;		// used to detect change of main synthesizer frequency

    // Transceiver menu mode variables
    uint8_t	menu_mode;		// TRUE if in menu mode
    int16_t	menu_item;		// Used to indicate specific menu item
    int		menu_var;		// Used to change specific menu item
    bool	menu_var_changed;	// TRUE if something changed in a menu and that an EEPROM save should be done!

    // Ham band public flag
    // index of bands table in Flash
    const BandInfo*     band; // this band does not relate to the real frequency, it "just" a band memory.
    const BandInfo*     band_effective; // the band the currently selected frequency is in (which may be different from the band memory idx);

    bool	rx_temp_mute;
    uint8_t	filter_band;		// filter selection band:  1= 80, 2= 60/40, 3=30/20, 4=17/15/12/10 - used for selection of power detector coefficient selection.
#define FILTER_BAND_UNKNOWN 255 // used to indicate that we don't know how the BPF is set
    uint8_t coupling_band;      // which tx wattmeter coupling factor value to use
    //
    // Receive/Transmit public flag
    uint8_t 	txrx_mode;

    bool	ptt_req;    // setting this to true will inform the PTT handling to switch to TX. Setting this to false has no effect if TX has been started.
                         // to stop TX, use tx_stop_req, see below.
    bool tx_stop_req;   //  setting this to true will inform the PTT handling to switch to RX.
                        // Please note: ptt_req takes precedence over tx_stop_req if both are true.


    // Demodulator mode public flag
    uint8_t 	dmod_mode;


    uint8_t 	enc_one_mode;
    uint8_t 	enc_two_mode;
    uint8_t 	enc_thr_mode;

    uint8_t	tx_meter_mode;				// meter mode

    // Audio filter ID
    uint8_t   filter_select[AUDIO_FILTER_NUM];


#define FILTER_PATH_MEM_MAX 5
    uint16_t   filter_path_mem[FILTER_MODE_MAX][FILTER_PATH_MEM_MAX];

    uint16_t  filter_path;
    const FilterPathDescriptor *filters_p;

    uint16_t	demod_mode_disable;			// TRUE if AM mode is to be disabled
#define DEMOD_AM_DISABLE    (0x0001)
#define DEMOD_CW_DISABLE    (0x0002)
#define DEMOD_DIGI_DISABLE  (0x0004)




    // AGC mode
//    uint8_t	agc_mode;
//    uint8_t	agc_custom_decay;

//    uint8_t	max_rf_gain;

    // Eth to UI driver requests flag
    uint8_t	LcdRefreshReq;

    // Eth to UI public flag
    uint8_t	new_band;
    uint8_t	new_mode;
    uint8_t	new_digi_mode;

    // Current CW mode
    uint8_t	cw_keyer_mode;
    uint8_t	cw_keyer_speed;
    uint8_t	cw_paddle_reverse;
    bool cw_text_entry;

    uint8_t cw_keyer_weight;   // cw dit/pause ratio 100 = 1.00 -> dit == pause == dah / 3
#define CW_KEYER_WEIGHT_DEFAULT (100)
#define CW_KEYER_WEIGHT_MAX     (150)
#define CW_KEYER_WEIGHT_MIN      (50)

    uint8_t cw_rx_delay; // break time
#define CW_TX2RX_DELAY_DEFAULT     8
#define CW_RX_DELAY_MAX         50  // Maximum TX to RX turnaround setting

    uint32_t cw_sidetone_freq;
#define CW_SIDETONE_FREQ_DEFAULT    750 // Default CW Audio Sidetone and TX offset frequency in Hz
#define CW_SIDETONE_FREQ_MIN        400
#define CW_SIDETONE_FREQ_MAX        1000

    uint32_t	audio_spkr_unmute_delay_count;

    uint8_t	power_level; // an abstract power level id
    int32_t power; // the actual request power in mW
    bool    power_modified; // the actual power is lower than the requested power_level, e.g. because of out side band.

    uint8_t 	tx_audio_source;
    uint8_t     rx_iq_source;
//
    uint8_t     tx_mic_boost;		// in dB

#define MIC_BOOST_DEFAULT		 0	// 0 dB boost (no boost)
#define MIC_BOOST_MIN			 0
#define MIC_BOOST_DYNAMIC       14  // 14 dB boost ( 25.1)
#define MIC_BOOST_MAX			20	// 20 dB boost (100.0) {more is not better}
//
    uint32_t	tx_mic_gain_mult;
    uint8_t	tx_gain[TX_AUDIO_NUM];
    int16_t	tx_comp_level;			// Used to hold compression level which is used to calculate other values for compression.  0 = manual.

    // Global tuning flag - in every demod mode
    uint8_t 	tune;

    uint16_t ee_init_stat;

    uint8_t	powering_down;

    // Spectrum Scope config - placed here since "sd." not defined at time of init

    uint8_t spectrum_size;              // size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved
    uint8_t	spectrum_filter;	// strength of filter in spectrum scope
    uint8_t spectrum_centre_line_colour;    // color of center line of scope grid
    uint8_t spectrum_freqscale_colour;  // color of spectrum scope frequency scale
    uint8_t spectrum_agc_rate;      // agc rate on the 'scope
    uint8_t spectrum_db_scale;  // db/Division scale setting on spectrum scope
    //  uint8_t   fft_window_type;            // type of windowing function applied to scope/waterfall.  At the moment, only lower 4 bits are used - upper 4 bits are reserved

    uint16_t scope_scheduler;        // timer for scheduling the next update of the spectrum scope update, updated at DMA rate
    uint8_t scope_speed;    // update rate for spectrum scope
    uint8_t	scope_trace_colour;	// color of spectrum scope trace;
    uint8_t	scope_grid_colour;	// saved color of spectrum scope grid;
    uint8_t scope_trace_BW_colour;	// color of BW highlighted spectrum scope trace
    uint8_t scope_backgr_BW_colour; // color of BW highlighted background of spectrum scope (% of white)
    // uint8_t  spectrum_scope_nosig_adjust;        // Adjustment for no signal adjustment conditions for spectrum scope

    struct mchf_waterfall waterfall;

    uint32_t	xverter_mode;		// TRUE if transverter mode active
    uint32_t	xverter_offset;		// frequency offset for transverter (added to frequency display)
    uint32_t    xverter_offset_tx;  // used for tx if set, frequency offset for transverter (added to frequency display)
    //
    // Calibration factors for output power, in percent (100 = 1.00)
    //
#define ADJ_REF_PWR 0
#define ADJ_FULL_POWER 1
    uint8_t	pwr_adj[2][MAX_BAND_NUM];
    //
    uint32_t	alc_decay;					// adjustable ALC release time - EEPROM read/write version
    uint32_t	alc_decay_var;				// adjustable ALC release time - working variable version
    uint32_t	alc_tx_postfilt_gain;		// amount of gain after the TX audio filtering - EEPROM read/write version
    uint32_t	alc_tx_postfilt_gain_var;	// amount of gain after the TX audio filtering - working variable version

// we can use AT least the upper 8 bits of freq_step_config for other purpose since these have not been used and are all initialized with 0)
#define FREQ_STEP_SWAP_BTN	    0x10
#define FREQ_STEP_SHOW_MARKER   0x01
    uint16_t	freq_step_config;			// configuration of step size (line, step button reversal) - setting any of the 4 upper bits -> step button switch, any of the lower bits -> frequency marker display enabled

    uint8_t     digital_mode;               // holds actual digital mode

    dsp_params_t dsp;

    uint8_t	lcd_backlight_brightness;	// LCD backlight dimming, 0-LCD_DIMMING_LEVEL_MAX:  0 = full, LCD_DIMMING_LEVEL_MAX = dimmest
#define LCD_DIMMING_LEVEL_MAX 5
#define LCD_DIMMING_LEVEL_MIN 0

#define LCD_BLANKING_ENABLE 0x80
#define LCD_BLANKING_TIMEMASK 0x0f
    uint8_t	lcd_backlight_blanking;		// for controlling backlight auto-off control



#define LOW_POWER_ENABLE 0x80    // bit7 shows enable / no enable
#define LOW_POWER_ENABLE_MASK 0x80

#define LOW_POWER_THRESHOLD_OFFSET 30    // value stored in the configuration variable is lower by this offset
#define LOW_POWER_THRESHOLD_MASK 0x7f
#define LOW_POWER_THRESHOLD_DEFAULT  0
#define LOW_POWER_THRESHOLD_MIN  0
#define LOW_POWER_THRESHOLD_MAX  126



    uint8_t   low_power_config;        // for voltage colours and auto shutdown
    uint32_t   low_power_shutdown_time;    // earliest time when auto shutdown can be executed
    //
    uint8_t	tune_step;					// Used for press-and-hold tune step adjustment
    uint32_t	tune_step_idx_holder;		// used to hold the original step size index during the press-and-hold
    //
    bool	frequency_lock;				// TRUE if frequency knob is locked
    //

#define TX_DISABLE_OFF          0
#define TX_DISABLE_ALWAYS       1
#define TX_DISABLE_USER         2
#define TX_DISABLE_OUTOFRANGE	4
#define TX_DISABLE_RXMODE       8
    uint8_t	tx_disable;		// >0 if no transmit permitted, use RadioManagement_IsTxDisabled() to get boolean

    uint16_t	flags1;					// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS1"

#define FLAGS1_TX_AUTOSWITCH_UI_DISABLE 0x01    // if on-screen AFG/(STG/CMP) and WPM/(MIC/LIN) indicators are changed on TX
#define FLAGS1_SWAP_BAND_BTN			0x02    // if BAND-/BAND+ buttons are to be swapped in their positions
#define FLAGS1_MUTE_LINEOUT_TX			0x04    // if TX audio output from LINE OUT is to be muted during transmit (audio output only enabled when translate mode is DISABLED
#define FLAGS1_AM_TX_FILTER_DISABLE		0x08    // if AM TX has transmit filter DISABLED
#define FLAGS1_SWAP_FWDREV_SENSE		0x10    // if FWD/REV A/D inputs from RF power detectors are to be reversed
#define FLAGS1_FREQ_LIMIT_RELAX			0x20    // if Frequency tuning is to be relaxed
#define FLAGS1_SSB_TX_FILTER_DISABLE	0x40    // if SSB TX has transmit filter DISABLED
#define FLAGS1_WFALL_ENABLED		    0x80    // 1 = Waterfall display
#define FLAGS1_SCOPE_ENABLED	        0x100   // 1 = Scope display
#define FLAGS1_DYN_TUNE_ENABLE			0x200   // 0 = dynamic tune is disabled, 1 = dynamic tune is enabled
#define FLAGS1_SAM_ENABLE				0x400   // 0 = SAM mode is disabled, 1 = SAM mode is enabled
#define FLAGS1_CAT_IN_SANDBOX			0x800   // 0 = CAT works on band storage, 1 = CAT works in sandbox
#define FLAGS1_SCOPE_LIGHT_ENABLE		0x1000  // 0 = Spectrum normal, 1 = Spectrum light
#define FLAGS1_TX_OUTSIDE_BANDS			0x2000  // 1 = TX outside bands enabled
#define FLAGS1_REVERSE_X_TOUCHSCREEN	0x4000  // 1 = X direcction of touchscreen is mirrored
#define FLAGS1_REVERSE_Y_TOUCHSCREEN	0x8000  // 1 = Y direcction of touchscreen is mirrored

    uint16_t    expflags1;              // Used to hold flags for options in Debug/Expert menu, stored in EEPROM location "EEPROM_EXPFLAGS1"
#define EXPFLAGS1_SMOOTH_DYNAMIC_TUNE   0x01    // 1 = Smooth dynamic tune is ON
// #define EXPFLAGS1_RESERVE_1          0x02    // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_2          0x04    // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_3          0x08    // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_4          0x10    // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_5          0x20    // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_6          0x40    // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_7          0x80    // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_8          0x100   // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_9          0x200   // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_10         0x400   // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_11         0x800   // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_12         0x1000  // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_13         0x2000  // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_14         0x4000  // Reserve flag for options in Debug/Expert menu
// #define EXPFLAGS1_RESERVE_15         0x8000  // Reserve flag for options in Debug/Expert menu
#define EXPFLAGS1_CONFIG_DEFAULT        0x0000  // Default flags state

#ifdef UI_BRD_MCHF
    // the default screen needs no reversed touch
#define FLAGS1_CONFIG_DEFAULT (FLAGS1_WFALL_ENABLED|FLAGS1_SCOPE_ENABLED)
#define TOUCHSCREEN_DF_MIRROR	TOUCHSCREEN_NO_MIRROR_NOFLIP
#endif
#ifdef UI_BRD_OVI40
    // the default screen needs reversed x axis touch
#define FLAGS1_CONFIG_DEFAULT (FLAGS1_REVERSE_X_TOUCHSCREEN)
#define TOUCHSCREEN_DF_MIRROR	TOUCHSCREEN_X_MIRROR_NOFLIP
#endif


    uint16_t	flags2;							// Used to hold individual status flags, stored in EEPROM location "EEPROM_FLAGS2"
#define FLAGS2_FM_MODE_ENABLE 			0x01    // 0 if FM mode is DISABLED, 1 if FM mode is ENABLED
#define FLAGS2_FM_MODE_DEVIATION_5KHZ 	0x02    // 0 if 2.5 kHz FM deviation, 1 for 5 kHz FM deviation
#define FLAGS2_KEY_BEEP_ENABLE 			0x04    // 1 if key/button beep is enabled
#define FLAGS2_LOW_BAND_BIAS_REDUCE 	0x08    // 1 if bias values for lower bands  below 8Mhz have lower influence factor
#define FLAGS2_FREQ_MEM_LIMIT_RELAX 	0x10    // 1 if memory-save versus frequency restrictions are to be relaxed
#define FLAGS2_TOUCHSCREEN_FLIP_XY	 	0x20    // 1 if touchscreen x and y are flipped
#define FLAGS2_HIGH_BAND_BIAS_REDUCE    0x40    // 1 if bias values for higher bands  above 8Mhz have lower influence factor
#define FLAGS2_UI_INVERSE_SCROLLING		0x80    // 1 if inverted Enc2/Enc3 UI actions, clockwise goes previous UiMenu_RenderChangeItem, folds up menu groups
#define FLAGS2_CONFIG_DEFAULT (FLAGS2_HIGH_BAND_BIAS_REDUCE|FLAGS2_LOW_BAND_BIAS_REDUCE)

    uint32_t	sysclock;				// This counts up from zero when the unit is powered up at precisely 100 Hz over the long term.  This
    // is NEVER reset and is used for timing certain events.
    uint16_t	version_number_minor;		// version number - minor - used to hold version number and detect change
    uint16_t	version_number_major;		// version number - build - used to hold version number and detect change
    uint16_t	version_number_release;		// version number - release - used to hold version number and detect change
    uint8_t	cw_offset_mode;				// CW offset mode (USB, LSB, etc.)
    bool	cw_lsb;					// flag used to indicate that CW is to operate in LSB when TRUE
    int32_t	iq_freq_mode;				// used to set/configure the I/Q frequency/conversion mode
    uint8_t	lsb_usb_auto_select;			// holds setting of LSB/USB auto-select above/below 10 MHz
    uint32_t	last_tuning;				// this is a timer used to prevent too fast tuning per second
    uint32_t	lcd_blanking_time;			// this holds the system time after which the LCD is blanked - if blanking is enabled
    bool	lcd_blanking_flag;			// if TRUE, the LCD is blanked completely (e.g. backlight is off)
    bool	xvtr_adjust_flag;			// set TRUE if transverter offset adjustment is in process
    bool	SpectrumResize_flag;		// set TRUE if waterfall/spectrum resize request from touchscreen action
    bool	VirtualKeysShown_flag;		// set TRUE if virtual keypad displayed instead of spectrum/waterfall
    const VKeypad* VirtualKeyPad;				// pointer to virtual keyboard definition (if VirtualKeysShown_flag is set)
    uint32_t SpectrumResize_timer;		//
#define VFO_MEM_MODE_SPLIT 0x80
#define VFO_MEM_MODE_VFO_B 0x40
    uint32_t	vfo_mem_mode;				// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
    // LSB+6 (0x40):  0 = VFO A, 1 = VFO B
    // LSB+7 (0x80): 0 = normal mode, 1 = Split mode (e.g. LSB=0:  RX=A, TX=B;  LSB=1:  RX=B, TX=A)
    uint32_t	voltmeter_calibrate;			// used to calibrate the voltmeter


    bool	dvmode;					// TRUE if alternate (stripped-down) RX and TX functions (USB-only) are to be used
    uint8_t	txrx_switch_audio_muting_timing;			// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
    uint32_t	audio_dac_muting_timer;			// timer value used for muting TX audio when keying PTT to suppress "click" or "thump"
    uint32_t audio_dac_muting_buffer_count; // the audio dac out will be muted for number of buffers
    uint8_t	filter_disp_colour;			// used to hold the current color of the line that indicates the filter passband/bandwidth
    bool	audio_dac_muting_flag;			// when TRUE, audio is to be muted after PTT/keyup
    bool	vfo_mem_flag;				// when TRUE, memory mode is enabled
    bool	mem_disp;				// when TRUE, memory display is enabled

    uint32_t    fm_subaudible_tone_gen_select;		// lookup ("tone number") used to index the table tone generation (0 corresponds to "tone disabled")
    uint8_t     fm_tone_burst_mode;			// this is the setting for the tone burst generator
    uint32_t    fm_tone_burst_timing;			// this is used to time/schedule the duration of a tone burst
    uint8_t     fm_sql_threshold;			// squelch threshold "dial" setting
    uint32_t    fm_subaudible_tone_det_select;		// lookup ("tone number") used to index the table for tone detection (0 corresponds to "disabled")

    // key beep. Enabled via FLAGS2 !
    uint32_t    beep_frequency;				// beep frequency, in Hz
    uint8_t     beep_loudness;				// loudness of the key beep
    uint32_t    beep_timing;                // countdown timer, used to activate beep and time the duration of a keyboard beep, in 1/100 ms

#define EEPROM_SER_NONE 0
#define EEPROM_SER_WRONG_SIG 1
#define EEPROM_SER_UNKNOWN 2
    uint8_t	ser_eeprom_type;			// serial eeprom type

#define CONFIGSTORE_IN_USE_I2C         0x00
#define CONFIGSTORE_IN_USE_ERROR       0x05
#define CONFIGSTORE_IN_USE_RAMCACHE    0xAA
#define CONFIGSTORE_IN_USE_FLASH       0xFF

    uint8_t	configstore_in_use;	    // use to determine non-volatile memory configuration

    mchf_touchscreen_t *tp;

    bool	show_debug_info;	// show coordinates on LCD
    uint8_t	tune_power_level;		// TX power in antenna tuning function
    uint8_t	power_temp;				// temporary tx power if tune is different from actual tx power
    uint8_t	cat_band_index;			// buffered bandindex before first CAT command arrived
    uint8_t	xlat;					// CAT <> IQ-Audio

//    bool	dBm_Hz_Test;			// for testing only
//    uint32_t	dBm_count;				// timer for calculating RX dBm
    uint8_t 	display_dbm;			// display dbm or dbm/Hz or OFF
    uint8_t	s_meter;				// defines S-Meter style/configuration
	uint8_t	meter_colour_up;
	uint8_t	meter_colour_down;
	uint8_t   iq_auto_correction;     // switch variable for automatic IQ correction
	bool	display_rx_iq;

	// twinpeak_tested = 2 --> wait for system to warm up
    // twinpeak_tested = 0 --> go and test the IQ phase
    // twinpeak_tested = 1 --> tested, verified, go and have a nice day!
	// twinpeak_tested = 8 -> we are waiting for the main loop to execute I2S restart
#define TWINPEAKS_WAIT 2
#define TWINPEAKS_DONE 1
#define TWINPEAKS_SAMPLING 0
#define TWINPEAKS_UNCORRECTABLE 3
#define TWINPEAKS_CODEC_RESTART 4
	uint8_t twinpeaks_tested;


    int32_t dbm_constant;

//#define DISPLAY_S_METER_STD   0
#define DISPLAY_S_METER_DBM   1
#define DISPLAY_S_METER_DBMHZ 2

//    #define TX_FILTER_NONE			0
    #define TX_FILTER_SOPRANO		1
    #define TX_FILTER_TENOR			2
    #define TX_FILTER_BASS			3
    uint8_t	tx_filter;				// which TX filter has been chosen?


    mchf_display_t*     display;

    uint32_t audio_int_counter;		// used for encoder timing - test DL2FW
    bool encoder3state;
    int32_t bc_band;

    Oscillator_ResultCodes_t last_lo_result;			// used in dynamic tuning to hold frequency color

    TuneToneMode tune_tone_mode;

	uint16_t ramsize; // in KB, this is used to distinguish  between 192 and 256 kB models.

	uint8_t stream_tx_audio; // send tx audio via usb back
#define STREAM_TX_AUDIO_OFF     0  // send nothing
#define STREAM_TX_AUDIO_SRC     1  // send source audio stream (from CODEC)
#define STREAM_TX_AUDIO_FILT    2  // send processed audio stream (after filtering)
#define STREAM_TX_AUDIO_DIGIQ   3  // send final IQ signal
#define STREAM_TX_AUDIO_GENIQ   4  // generated "clean" IQ signal before final scaling and IQ phase/balance adjust
#define STREAM_TX_AUDIO_NUM     5  // how many choices

    bool    digi_lsb;                 // flag used to indicate that mcHF is to operate in LSB when TRUE

    bool dial_moved; // dial was moved, used to communicate with spectrum display code


    uint32_t i2c_speed[2]; // store comm speed for the 2 I2C buses
#define I2C_BUS_1 0
#define I2C_BUS_2 1


    bool rtc_present; // a supported rtc was found and is active
    int16_t rtc_calib; // ppm variation value, unit 1 ppm
    bool vbat_present; // we detected a working vbat mod
    bool codec_present; // we detected a working codec
	bool rtty_atc_enable; // is ATC enabled for RTTY decoding? (for testing!)

	uint8_t enable_rtty_decode; // new rtty encoder (experimental)
	uint8_t cw_decoder_enable;
	bool cw_offset_shift_keep_signal; // experimental flag, shall we move shift by sidetone frequency to keep tuned signal?
	bool enable_ptt_rts; // disable/enable ptt via virtual serial port rts

	keyer_mode_t keyer_mode; // disable/enable keyer mode for F1-F5 buttons
	bool buffered_tx; // disable/enable buffered sending for CW and digital modes
#ifdef USE_TWO_CHANNEL_AUDIO
	bool stereo_enable; // enable/disable stereo demodulation (only in special hardware, NOT in mcHF)
#endif
#ifdef USE_LEAKY_LMS
	bool enable_leaky_LMS;
#endif

	uint8_t debug_si5351a_pllreset;
	uint16_t graticulePowerupYpos;	//initial (after powerup) position of graticule (frequency bar)
	const LcdLayout* Layout;				//current lcd layout (set by lcd detection routine)
	uint8_t FreqDisplayFont;		//0= old thin font, 1=new bold 8 bit (if available)

	RfBoard_t rf_board; // the detected rf board connected to the control logic
	uint8_t special_functions_enabled;
	bool txrx_switching_enabled;

	bool paddles_active; // setting this to false disables processing of external gpio interrupts (right now just the paddles/PTT)

    uint8_t vswr_protection_threshold; // 1 - protection OFF

	// noise reduction gain display in spectrum
    int16_t  nr_gain_display; // 0 = do not display gains, 1 = display bin gain in spectrum display, 2 = display long_tone_gain
    //                                           3 = display bin gain multiplied with long_tone_gain

} TransceiverState;

extern __IO TransceiverState ts;

#define	POWERDOWN_DELAY_COUNT	30	// Delay in main service loop for the "last second" before power-down - to allow EEPROM write to complete

//#define CODEC_USE_SPI

#define DEBUG_COM                        USART1

/**
 * @brief Introduces about 40ms of delay (load dependent, since interrupt eats some of the time.
 */
// TODO: Measure raw time for this loop

#define non_os_delay()						\
do {							\
  register uint32_t i;				\
  for (i = 0; i < 1000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)


// ------------------------------------------------------------------
// Exports
typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON = 1,
    LED_STATE_TOGGLE = 2
} ledstate_t;



void Board_SetPaBiasValue(uint16_t bias);
void Board_HandlePowerDown(void);

void Board_SelectLpfBpf(uint8_t group);

void Board_InitMinimal(void);
void Board_InitFull(void);
void Board_PostInit(void);
void Board_Reboot(void);
void Board_Powerdown(void);

void Board_EnableTXSignalPath(bool tx_enable);

void Board_GreenLed(ledstate_t state);
void Board_RedLed(ledstate_t state);

#ifdef UI_BRD_OVI40
void Board_BlueLed(ledstate_t state);
#endif

bool Board_PttDahLinePressed(void);
bool Board_DitLinePressed(void);

uint32_t Board_RamSizeGet(void);
void Board_RamSizeDetection(void);
const char* Board_BootloaderVersion(void);

// in main.c
void CriticalError(uint32_t error);

bool is_vfo_b(void);

static inline bool is_ssb_tx_filter_enabled() {
	return (ts.tx_filter != 0);
	//    return (ts.flags1 & FLAGS1_SSB_TX_FILTER_DISABLE) == false;
}

static inline bool is_ssb(const uint32_t dmod_mode) {
    return (dmod_mode == DEMOD_LSB || dmod_mode == DEMOD_USB);
}

static inline bool is_splitmode()
{
    return (ts.vfo_mem_mode & VFO_MEM_MODE_SPLIT) != 0;
}

static inline bool is_scopemode()
{
    return (ts.flags1 & FLAGS1_SCOPE_ENABLED) != 0;
}

static inline bool is_waterfallmode()
{
    return (ts.flags1 & FLAGS1_WFALL_ENABLED) != 0;
}

bool is_dsp_nb_active(void);
bool is_dsp_nb(void);
bool is_dsp_nr(void);
bool is_dsp_nr_postagc(void);
bool is_dsp_notch(void);
bool is_dsp_mnotch(void);
bool is_dsp_mpeak(void);


#ifdef USE_PENDSV_FOR_HIGHPRIO_TASKS
extern void UiDriver_TaskHandler_HighPrioTasks(void);
#endif

#endif
