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

#ifndef DRIVERS_UI_UI_CONFIGURATION_H_
#define DRIVERS_UI_UI_CONFIGURATION_H_
#include "uhsdr_board.h"

enum
{
    ConfigEntry_Stop = 0,
    ConfigEntry_UInt8,
    ConfigEntry_UInt16,
    ConfigEntry_UInt32_16,
    ConfigEntry_Int32_16,
    ConfigEntry_Int16,
	ConfigEntry_Int32,			//this type saves and reads two subsequent 16bit words
	ConfigEntry_UInt8x2,      // this type saves and reads two indepent 8 bit values stored in a 16bit structure. This is relevant for defaults, which are independent
//  ConfigEntry_Bool,
	Calib_Val = 0x10000000,     // if this is "or"ed with a normal ConfigEntry_... value, it indicates this parameter is a hardware calibration parameter.
	ConfigEntry_TypeMask = 0x0000ffff, // we support 2^16 different types, this is plenty.
};

typedef struct
{
    int typeId;
    uint16_t id;
    volatile void* val_ptr;
    int32_t val_default;
    int32_t val_min;
    int32_t val_max;
    // uint16_t item_count; // 0 or 1 indicates single value; higher values indicate arrays
    // uint16_t  item_data_spacing; // only used for 32bit values; indicates the distance of the higher 16 bits from the lower 16bits
} ConfigEntryDescriptor;

const ConfigEntryDescriptor* UiConfiguration_GetEntry(uint16_t id);

void        UiConfiguration_LoadEepromValues(bool load_freq_mode_defaults, bool load_eeprom_defaults);
uint16_t    UiConfiguration_SaveEepromValues(void);
void		UiConfiguration_UpdateMacroCap(void);

// Configuration Value Definitions Follow
//
//
#define SIDETONE_MAX_GAIN   15      // Maximum sidetone gain
#define DEFAULT_SIDETONE_GAIN   5   // Default sidetone gain
//
#define CW_KEYER_SPEED_MIN     5       // Minimum keyer speed
#define CW_KEYER_SPEED_MAX     48      // Maximum keyer speed
#define CW_KEYER_SPEED_DEFAULT 20      // Default keyer speed
//

//
#define USB_FREQ_THRESHOLD  (10000000)    // dial frequency at and above which the default is USB, Hz
//
//
#define MIN_RIT_VALUE       -60     // Minimum RIT Value -1.2 kHz
#define MAX_RIT_VALUE       60      // Maximum RIT Value +1.2 kHz

#define LINEOUT_GAIN_DEFAULT    0x49    // Default lineout gain setting in dB steps
#define LINEOUT_GAIN_MIN        1       //  min lineout gain setting in dB steps
#define LINEOUT_GAIN_MAX        0x50    //  max lineout gain setting in dB steps


#define AUDIO_GAIN_DEFAULT  16      // Default audio gain
#define AUDIO_GAIN_MAX      30      // Maximum audio gain setting

#define DIG_GAIN_MAX        31      // Maximum audio gain setting
#define DIG_GAIN_DEFAULT    16      // Default audio gain

//
// The following are used in the max volume setting in the menu system
//
#define MAX_VOLUME_MIN              8       // Minimum setting for maximum volume
#define MAX_VOLUME_MAX              AUDIO_GAIN_MAX      // Maximum setting for maximum volume
#define MAX_VOLUME_DEFAULT          AUDIO_GAIN_DEFAULT
//
#define MAX_VOLUME_RED_THRESH  10      // "MAX VOLUME" setting at or below which number will be RED to warn user
#define MAX_VOLUME_YELLOW_THRESH  16  // "MAX VOLUME" setting at or below which number will be YELLOW to warn user

#define MAGNIFY_MIN                 0
#define MAGNIFY_MAX                 5
#define MAGNIFY_DEFAULT             0
#define MAGNIFY_NUM                 (MAGNIFY_MAX+1)

//
#define PA_BIAS_MAX         115     // Maximum PA Bias Setting
#define PA_BIAS_DEFAULT     0       // Default PA Bias setting
#define PA_BIAS_LOW_LIMIT   20      // Minimum bias setting.  (Below this, number is red)
//
#define BIAS_OFFSET         25      // Offset value to be added to bias setting
//  DA value = (OFFSET + (2*setting))  where DA value is 0-255
//

#define XVERTER_MULT_MAX        10      // maximum LO multipler in xverter mode
#define XVERTER_OFFSET_MAX      999000000   // Maximum transverter offset (999 MHz)
//
#define AUTO_LSB_USB_OFF        0
#define AUTO_LSB_USB_ON         1
#define AUTO_LSB_USB_60M        2
#define AUTO_LSB_USB_MAX        2
#define AUTO_LSB_USB_DEFAULT    AUTO_LSB_USB_60M

// used to limit the voltmeter calibration parameters
//
#define POWER_VOLTMETER_CALIBRATE_DEFAULT   100
#define POWER_VOLTMETER_CALIBRATE_MIN       00
#define POWER_VOLTMETER_CALIBRATE_MAX       200

#define LOW_POWER_CONFIG_DEFAULT  (90-LOW_POWER_THRESHOLD_OFFSET)
#define LOW_POWER_CONFIG_MIN  0
#define LOW_POWER_CONFIG_MAX  (LOW_POWER_ENABLE_MASK + LOW_POWER_THRESHOLD_MAX)

#define SWR_CAL_MIN             75
#define SWR_CAL_MAX             150
#define SWR_CAL_DEFAULT             100
//
#define SENSOR_NULL_MIN             75
#define SENSOR_NULL_MAX             125
#define SENSOR_NULL_DEFAULT         100


#define SPEC_COLOUR_TRACE_DEFAULT   SPEC_WHITE
#define SPEC_COLOUR_TRACEBW_DEFAULT SPEC_WHITE
#define SPEC_COLOUR_BACKGRBW_DEFAULT 20
#define SPEC_COLOUR_GRID_DEFAULT    SPEC_GREY4
#define SPEC_COLOUR_SCALE_DEFAULT   SPEC_GREY4
#define FILTER_DISP_COLOUR_DEFAULT  SPEC_GREY4

//
// *************************************************************************************************************************
//
// EEPROM Items IDs
//
// These do NOT use "enum" as it is important that the number *NOT* change
// by the insertion of new variables:  All NEW variable should be placed at
// the END of the list to maintain compatibility with older versions and the settings!
// If a firmware no longer uses an ID, do not delete the ID entry, just comment it out!
// This allows to track former use of an ID for a particular purpose.
// DO NOT REUSE PREVIOUSLY USED IDs without thinking twice. Old entries from former
// use may cause unexpected behavior with the new firmware and vice versa
// if downgrading from a new one to an old firmware.
//
//

#define EEPROM_ZERO_LOC			            0
#define EEPROM_BAND_MODE					1
#define EEPROM_FREQ_HIGH					2
#define EEPROM_FREQ_LOW						3
#define EEPROM_FREQ_STEP					4
#define EEPROM_TX_AUDIO_SRC					5
#define EEPROM_TCXO_STATE					6
#define EEPROM_PA_BIAS						7
#define EEPROM_AUDIO_GAIN					8
#define EEPROM_RX_CODEC_GAIN				9
#define EEPROM_MAX_VOLUME					10
#define EEPROM_POWER_STATE					11
#define EEPROM_TX_POWER_LEVEL				12
#define EEPROM_CW_KEYER_SPEED				13
#define EEPROM_CW_KEYER_MODE				14
#define EEPROM_CW_SIDETONE_GAIN				15
#define EEPROM_MIC_BOOST					16
#define EEPROM_TX_IQ_80M_GAIN_BALANCE		17      // TX gain balance
#define EEPROM_TX_IQ_80M_PHASE_BALANCE		18      // TX phase balance
#define EEPROM_RX_IQ_80M_GAIN_BALANCE		19
#define EEPROM_RX_IQ_80M_PHASE_BALANCE		20

#if 0 /* config value locations below are no longer in use since 2.5.18 */
#define EEPROM_BAND0_MODE					21      // Band/mode/filter memory per-band - bands indexed from here
#define EEPROM_BAND1_MODE					22
#define EEPROM_BAND2_MODE					23
#define EEPROM_BAND3_MODE					24
#define EEPROM_BAND4_MODE					25
#define EEPROM_BAND5_MODE					26
#define EEPROM_BAND6_MODE					27
#define EEPROM_BAND7_MODE					28
#define EEPROM_BAND8_MODE					29
#define EEPROM_BAND9_MODE					30
#define EEPROM_BAND10_MODE					31
#define EEPROM_BAND11_MODE					32
#define EEPROM_BAND12_MODE					33
#define EEPROM_BAND13_MODE					34
#define EEPROM_BAND14_MODE					35
#define EEPROM_BAND15_MODE					36
#define EEPROM_BAND16_MODE					37
#define EEPROM_BAND17_MODE					38      // "Floating" General coverage band
//
//
#define EEPROM_BAND0_FREQ_HIGH				39      // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_HIGH				40
#define EEPROM_BAND2_FREQ_HIGH				41
#define EEPROM_BAND3_FREQ_HIGH				42
#define EEPROM_BAND4_FREQ_HIGH				43
#define EEPROM_BAND5_FREQ_HIGH				44
#define EEPROM_BAND6_FREQ_HIGH				45
#define EEPROM_BAND7_FREQ_HIGH				46
#define EEPROM_BAND8_FREQ_HIGH				47
#define EEPROM_BAND9_FREQ_HIGH				48
#define EEPROM_BAND10_FREQ_HIGH				49
#define EEPROM_BAND11_FREQ_HIGH				50
#define EEPROM_BAND12_FREQ_HIGH				51
#define EEPROM_BAND13_FREQ_HIGH				52
#define EEPROM_BAND14_FREQ_HIGH				53
#define EEPROM_BAND15_FREQ_HIGH				54
#define EEPROM_BAND16_FREQ_HIGH				55
#define EEPROM_BAND17_FREQ_HIGH				56      // "Floating" General coverage band
//
//
#define EEPROM_BAND0_FREQ_LOW				57      // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_LOW				58
#define EEPROM_BAND2_FREQ_LOW				59
#define EEPROM_BAND3_FREQ_LOW				60
#define EEPROM_BAND4_FREQ_LOW				61
#define EEPROM_BAND5_FREQ_LOW				62
#define EEPROM_BAND6_FREQ_LOW				63
#define EEPROM_BAND7_FREQ_LOW				64
#define EEPROM_BAND8_FREQ_LOW				65
#define EEPROM_BAND9_FREQ_LOW				66
#define EEPROM_BAND10_FREQ_LOW				67
#define EEPROM_BAND11_FREQ_LOW				68
#define EEPROM_BAND12_FREQ_LOW				69
#define EEPROM_BAND13_FREQ_LOW				70
#define EEPROM_BAND14_FREQ_LOW				71
#define EEPROM_BAND15_FREQ_LOW				72
#define EEPROM_BAND16_FREQ_LOW				73
#define EEPROM_BAND17_FREQ_LOW				74      // "Floating" General coverage band

#endif
//
//
#define EEPROM_FREQ_CAL						75      // Frequency calibration
#define EEPROM_NB_SETTING					76      // Noise Blanker Setting
//#define EEPROM_AGC_MODE						77      // AGC setting
#define EEPROM_MIC_GAIN						78      // Mic gain setting
#define EEPROM_LINE_GAIN					79      // Line gain setting
#define EEPROM_SIDETONE_FREQ				80      // Sidetone frequency (Hz)
#define EEPROM_SPEC_SCOPE_SPEED				81      // Spectrum Scope Speed
#define EEPROM_SPECTRUM_FILTER				82      // Spectrum Scope filter strength
//#define EEPROM_RX_GAIN						83      // RX Gain setting (e.g. minimum RF gain as might be used for manual AGC)
//#define EEPROM_AGC_CUSTOM_DECAY				84      // Custom setting for AGC decay rate
#define EEPROM_SPECTRUM_TRACE_COLOUR		85      // Custom setting for spectrum scope trace colour
#define EEPROM_SPECTRUM_GRID_COLOUR			86      // Custom setting for spectrum scope grid colour
#define EEPROM_SPECTRUM_SCALE_COLOUR		87      // Custom setting for spectrum scope frequency scale colour
#define EEPROM_PADDLE_REVERSE				88      // TRUE if paddle is to be reversed
#define EEPROM_CW_RX_DELAY					89      // Delay after last CW element before returning to receive
#define EEPROM_SPECTRUM_CENTRE_LINE_COLOUR	90  // Custom setting for spectrum scope grid center marker colour
//
#define EEPROM_DETECTOR_COUPLING_COEFF_2200M 91  // Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_630M	92  // Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_80M	93  // Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_40M	94  // Calibration coupling coefficient for FWD/REV power sensor for 60/40 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_20M	95  // Calibration coupling coefficient for FWD/REV power sensor for 30/20/17 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_15M	96  // Calibration coupling coefficient for FWD/REV power sensor for 15/12/10 meters
//
// The following are the coefficients used to set the RF output power settings
//
#define EEPROM_BAND0_5W						97      // 5 watt power setting, 80m
#define EEPROM_BAND1_5W						98      // 5 watt power setting, 60m
#define EEPROM_BAND2_5W						99      // 5 watt power setting, 40m
#define EEPROM_BAND3_5W						100     // 5 watt power setting, 30m
#define EEPROM_BAND4_5W						101     // 5 watt power setting, 20m
#define EEPROM_BAND5_5W						102     // 5 watt power setting, 17m
#define EEPROM_BAND6_5W						103     // 5 watt power setting, 15m
#define EEPROM_BAND7_5W						104     // 5 watt power setting, 12m
#define EEPROM_BAND8_5W						105     // 5 watt power setting, 10m
#define EEPROM_BAND9_5W						106     // 5 watt power setting, 6m
#define EEPROM_BAND10_5W					107     // 5 watt power setting, 4m
#define EEPROM_BAND11_5W					108     // 5 watt power setting, 2m
#define EEPROM_BAND12_5W					109     // 5 watt power setting, 70cm
#define EEPROM_BAND13_5W					110     // 5 watt power setting, 23cm
#define EEPROM_BAND14_5W					111     // 5 watt power setting, 2200m
#define EEPROM_BAND15_5W					112     // 5 watt power setting, 630m
#define EEPROM_BAND16_5W					113     // 5 watt power setting, 160m
#define EEPROM_BAND17_5W					114     // reserved
//
#define EEPROM_BAND0_FULL					115     // "FULL" power setting, 80m
#define EEPROM_BAND1_FULL					116     // "FULL" power setting, 60m
#define EEPROM_BAND2_FULL					117     // "FULL" power setting, 40m
#define EEPROM_BAND3_FULL					118     // "FULL" power setting, 30m
#define EEPROM_BAND4_FULL					119     // "FULL" power setting, 20m
#define EEPROM_BAND5_FULL					120     // "FULL" power setting, 17m
#define EEPROM_BAND6_FULL					121     // "FULL" power setting, 15m
#define EEPROM_BAND7_FULL					122     // "FULL" power setting, 12m
#define EEPROM_BAND8_FULL					123     // "FULL" power setting, 10m
#define EEPROM_BAND9_FULL					124     // "FULL" power setting, 6m
#define EEPROM_BAND10_FULL					125     // "FULL" power setting, 4m
#define EEPROM_BAND11_FULL					126     // "FULL" power setting, 2m
#define EEPROM_BAND12_FULL					127     // "FULL" power setting, 70cm
#define EEPROM_BAND13_FULL					128     // "FULL" power setting, 23cm
#define EEPROM_BAND14_FULL					129     // "FULL" power setting, 2200m
#define EEPROM_BAND15_FULL					130     // "FULL" power setting, 630m
#define EEPROM_BAND16_FULL					131     // "FULL" power setting, 160m
#define EEPROM_BAND17_FULL					132     // reserved
//
#if 0 // No longer being used, superseded by filter path
#define EEPROM_FILTER_300HZ_SEL				133     // Selection of 300 Hz filter
#define EEPROM_FILTER_500HZ_SEL				134     // Selection of 500 Hz filter
#define EEPROM_FILTER_1K8_SEL				135     // Selection of 1.8 kHz filter
#define EEPROM_FILTER_2K3_SEL				136     // Selection of 2.3 kHz filter
#define EEPROM_FILTER_3K6_SEL				137     // Selection of 3.6 kHz filter
#define EEPROM_FILTER_WIDE_SEL				138     // Selection of "Wide" filter (>3.6kHz)
//
#endif
#define EEPROM_TX_IQ_10M_GAIN_BALANCE		139     // TX gain balance
#define EEPROM_TX_IQ_10M_PHASE_BALANCE		140     // TX phase balance
#define EEPROM_RX_IQ_10M_GAIN_BALANCE		141
#define EEPROM_RX_IQ_10M_PHASE_BALANCE		142
#define EEPROM_SENSOR_NULL					143     // Power meter sensor null calibrate
//#define   EEPROM_REV_PWR_CAL				144     // REV power meter calibrate
//
#define EEPROM_XVERTER_DISP					145     // TRUE if display is offset with transverter frequency offset
#define EEPROM_XVERTER_OFFSET_HIGH			146     // Frequency by which the display is offset for transverter use, high byte
//
#define EEPROM_VFO_MEM_MODE					147     // settings of VFO/SPLIT/Memory configuration bits - see variable "vfo_mem_mode" for information.
//
#define EEPROM_XVERTER_OFFSET_LOW			148     // Low byte of above
//
#define EEPROM_SPECTRUM_MAGNIFY				149     // TRUE if spectrum scope is to be magnified
//
//#define EEPROM_WIDE_FILT_CW_DISABLE			150     // TRUE if wide filters are to be disabled in CW mode
//#define EEPROM_NARROW_FILT_SSB_DISABLE		151     // TRUE if narrow filters are to be disabled in SSB mode
//
#define EEPROM_AM_MODE_DISABLE				152     // TRUE if AM mode is to be disabled
//
#define EEPROM_PA_CW_BIAS					153     // If non-zero, this is the PA bias setting when in CW mode
//
#define EEPROM_SPECTRUM_DB_DIV				154     // Spectrum Scope dB/Division
#define EEPROM_SPECTRUM_AGC_RATE			155     // AGC setting for spectrum scope
//
#define EEPROM_METER_MODE					156     // Stored setting of meter mode
//
#define EEPROM_ALC_DECAY_TIME				157     // ALC Decay time
#define EEPROM_ALC_POSTFILT_TX_GAIN			158     // ALC post-filter TX audio gain
//
#define EEPROM_STEP_SIZE_CONFIG				159     // TRUE if there is to be a line under the frequency digit indicating step size
//
#define EEPROM_DSP_MODE						160     // Stores the DSP operational mode
#define EEPROM_DSP_NR_STRENGTH				161     // Stores the DSP Noise Reduction operational strength
#ifdef OBSOLETE_NR
#define EEPROM_DSP_NR_DECOR_BUFLEN			162     // DSP Noise Reduction De-correlator buffer length
#define EEPROM_DSP_NR_FFT_NUMTAPS			163     // DSP Noise Reduction FFT number of taps

#define EEPROM_DSP_NOTCH_DECOR_BUFLEN		164     // DSP Notch De-correlator buffer length
#define EEPROM_DSP_NOTCH_CONV_RATE			165     // DSP Notch convergence rate
#endif
#ifdef USE_LMS_AUTONOTCH
#define EEPROM_DSP_NOTCH_DECOR_BUFLEN		164     // DSP Notch De-correlator buffer length
#define EEPROM_DSP_NOTCH_CONV_RATE			165     // DSP Notch convergence rate
#endif
//
//#define EEPROM_MAX_RX_GAIN					166     // Maximum RX gain - adjusts maximum allowed AGC gain in S-units
#define EEPROM_TX_AUDIO_COMPRESS			167     // TX audio compressor setting, used to calculate other values
//
#define EEPROM_TX_IQ_80M_GAIN_BALANCE_TRANS_OFF		168     // IQ Gain balance for AM reception
//
#define EEPROM_TX_DISABLE					169     // TRUE of transmit is to be disabled
#define EEPROM_FLAGS1						170     // Miscellaneous status flag, saved in EEPROM - see variable "flags1"
#define EEPROM_VERSION_RELEASE				171     // Storage of current version release - used to detect change of firmware
#define EEPROM_NB_AGC_TIME_CONST			172     // Noise blanker AGC time constant setting
#define EEPROM_CW_OFFSET_MODE				173     // CW Offset mode
#define EEPROM_FREQ_CONV_MODE				174     // Frequency Conversion Mode (e.g. I/Q frequency conversion done in receive/transmit to offset from zero)
#define EEPROM_LSB_USB_AUTO_SELECT			175     // Auto selection of LSB/USB above/below 10 MHz (including 60 meters)
#define EEPROM_VERSION_MAJOR				176     // Storage of current version build number - used to detect change of firmware
#define EEPROM_LCD_BLANKING_CONFIG			177     // Configuration of automatic LCD blanking mode settings
#define EEPROM_VOLTMETER_CALIBRATE			178     // Holder for calibration of the on-screen voltmeter
#define EEPROM_WATERFALL_COLOR_SCHEME		179     // Color scheme for waterfall display
#define EEPROM_WATERFALL_VERTICAL_STEP_SIZE 180 // Number of vertical steps of waterfall per iteration
#define EEPROM_WATERFALL_OFFSET				181     // Palette offset for waterfall
#define EEPROM_WATERFALL_CONTRAST			182     // Palette contrast multiplier for waterfall
//
// VFO A storage
//
#define EEPROM_BAND0_MODE_A					183     // Band/mode/filter memory per-band - bands indexed from here
#define EEPROM_BAND1_MODE_A					184
#define EEPROM_BAND2_MODE_A					185
#define EEPROM_BAND3_MODE_A					186
#define EEPROM_BAND4_MODE_A					187
#define EEPROM_BAND5_MODE_A					188
#define EEPROM_BAND6_MODE_A					189
#define EEPROM_BAND7_MODE_A					190
#define EEPROM_BAND8_MODE_A					191
#define EEPROM_BAND9_MODE_A					192
#define EEPROM_BAND10_MODE_A				193
#define EEPROM_BAND11_MODE_A				194
#define EEPROM_BAND12_MODE_A				195
#define EEPROM_BAND13_MODE_A				196
#define EEPROM_BAND14_MODE_A				197
#define EEPROM_BAND15_MODE_A				198
#define EEPROM_BAND16_MODE_A				199
#define EEPROM_BAND17_MODE_A				200     // "Floating" General coverage band
//
#define EEPROM_BAND0_FREQ_HIGH_A			201     // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_HIGH_A			202
#define EEPROM_BAND2_FREQ_HIGH_A			203
#define EEPROM_BAND3_FREQ_HIGH_A			204
#define EEPROM_BAND4_FREQ_HIGH_A			205
#define EEPROM_BAND5_FREQ_HIGH_A			206
#define EEPROM_BAND6_FREQ_HIGH_A			207
#define EEPROM_BAND7_FREQ_HIGH_A			208
#define EEPROM_BAND8_FREQ_HIGH_A			209
#define EEPROM_BAND9_FREQ_HIGH_A			210
#define EEPROM_BAND10_FREQ_HIGH_A			211
#define EEPROM_BAND11_FREQ_HIGH_A			212
#define EEPROM_BAND12_FREQ_HIGH_A			213
#define EEPROM_BAND13_FREQ_HIGH_A			214
#define EEPROM_BAND14_FREQ_HIGH_A			215
#define EEPROM_BAND15_FREQ_HIGH_A			216
#define EEPROM_BAND16_FREQ_HIGH_A			217
#define EEPROM_BAND17_FREQ_HIGH_A			218     // "Floating" General coverage band
//
#define EEPROM_BAND0_FREQ_LOW_A				219     // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_LOW_A				220
#define EEPROM_BAND2_FREQ_LOW_A				221
#define EEPROM_BAND3_FREQ_LOW_A				222
#define EEPROM_BAND4_FREQ_LOW_A				223
#define EEPROM_BAND5_FREQ_LOW_A				224
#define EEPROM_BAND6_FREQ_LOW_A				225
#define EEPROM_BAND7_FREQ_LOW_A				226
#define EEPROM_BAND8_FREQ_LOW_A				227
#define EEPROM_BAND9_FREQ_LOW_A				228
#define EEPROM_BAND10_FREQ_LOW_A			229
#define EEPROM_BAND11_FREQ_LOW_A			230
#define EEPROM_BAND12_FREQ_LOW_A			231
#define EEPROM_BAND13_FREQ_LOW_A			232
#define EEPROM_BAND14_FREQ_LOW_A			233
#define EEPROM_BAND15_FREQ_LOW_A			234
#define EEPROM_BAND16_FREQ_LOW_A			235
#define EEPROM_BAND17_FREQ_LOW_A			236     // "Floating" General coverage band
//
// VFO B storage
//
#define EEPROM_BAND0_MODE_B					237     // Band/mode/filter memory per-band - bands indexed from here
#define EEPROM_BAND1_MODE_B					238
#define EEPROM_BAND2_MODE_B					239
#define EEPROM_BAND3_MODE_B					240
#define EEPROM_BAND4_MODE_B					241
#define EEPROM_BAND5_MODE_B					242
#define EEPROM_BAND6_MODE_B					243
#define EEPROM_BAND7_MODE_B					244
#define EEPROM_BAND8_MODE_B					245
#define EEPROM_BAND9_MODE_B					246
#define EEPROM_BAND10_MODE_B				247
#define EEPROM_BAND11_MODE_B				248
#define EEPROM_BAND12_MODE_B				249
#define EEPROM_BAND13_MODE_B				250
#define EEPROM_BAND14_MODE_B				251
#define EEPROM_BAND15_MODE_B				252
#define EEPROM_BAND16_MODE_B				253
#define EEPROM_BAND17_MODE_B				254     // "Floating" General coverage band
//
//
#define EEPROM_BAND0_FREQ_HIGH_B			255     // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_HIGH_B			256
#define EEPROM_BAND2_FREQ_HIGH_B			257
#define EEPROM_BAND3_FREQ_HIGH_B			258
#define EEPROM_BAND4_FREQ_HIGH_B			259
#define EEPROM_BAND5_FREQ_HIGH_B			260
#define EEPROM_BAND6_FREQ_HIGH_B			261
#define EEPROM_BAND7_FREQ_HIGH_B			262
#define EEPROM_BAND8_FREQ_HIGH_B			263
#define EEPROM_BAND9_FREQ_HIGH_B			264
#define EEPROM_BAND10_FREQ_HIGH_B			265
#define EEPROM_BAND11_FREQ_HIGH_B			266
#define EEPROM_BAND12_FREQ_HIGH_B			267
#define EEPROM_BAND13_FREQ_HIGH_B			268
#define EEPROM_BAND14_FREQ_HIGH_B			269
#define EEPROM_BAND15_FREQ_HIGH_B			270
#define EEPROM_BAND16_FREQ_HIGH_B			271
#define EEPROM_BAND17_FREQ_HIGH_B			272     // "Floating" General coverage band
//
//
#define EEPROM_BAND0_FREQ_LOW_B				273     // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_LOW_B				274
#define EEPROM_BAND2_FREQ_LOW_B				275
#define EEPROM_BAND3_FREQ_LOW_B				276
#define EEPROM_BAND4_FREQ_LOW_B				277
#define EEPROM_BAND5_FREQ_LOW_B				278
#define EEPROM_BAND6_FREQ_LOW_B				279
#define EEPROM_BAND7_FREQ_LOW_B				280
#define EEPROM_BAND8_FREQ_LOW_B				281
#define EEPROM_BAND9_FREQ_LOW_B				282
#define EEPROM_BAND10_FREQ_LOW_B			283
#define EEPROM_BAND11_FREQ_LOW_B			284
#define EEPROM_BAND12_FREQ_LOW_B			285
#define EEPROM_BAND13_FREQ_LOW_B			286
#define EEPROM_BAND14_FREQ_LOW_B			287
#define EEPROM_BAND15_FREQ_LOW_B			288
#define EEPROM_BAND16_FREQ_LOW_B			289
#define EEPROM_BAND17_FREQ_LOW_B			290     // "Floating" General coverage band
//
#define EEPROM_WATERFALL_SPEED				291     // Spectrum Scope Speed
#define EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST	292 // adjustment for no-signal conditions of spectrum scope
#define EEPROM_WATERFALL_NOSIG_ADJUST		293     // adjustment for no-signal conditions of waterfall
#define EEPROM_DSP_NOTCH_FFT_NUMTAPS		294     // DSP Notch FFT number of taps
#define EEPROM_SPECTRUM_SIZE				295     // size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved
#define EEPROM_FFT_WINDOW					296     // FFT Window information (lower nybble currently used - upper nybble reserved)
#define EEPROM_TXRX_SWITCH_AUDIO_MUTE_DELAY	297     // timer used for muting TX audio when keying PTT to suppress "click" or "thump"
#define EEPROM_FLAGS2						298     // Miscellaneous status flag, saved in EEPROM - see variable "flags2"
#define EEPROM_FILTER_DISP_COLOUR			299     // This contains the color of the line under the spectrum/waterfall display
#define EEPROM_TX_IQ_10M_GAIN_BALANCE_TRANS_OFF		300     // IQ Gain balance for AM transmission
#define EEPROM_TX_IQ_10M_PHASE_BALANCE_TRANS_OFF		301     // IQ Gain balance for FM transmission
#define EEPROM_FM_SUBAUDIBLE_TONE_GEN		302     // index for storage of subaudible tone generation
#define EEPROM_FM_TONE_BURST_MODE			303     // tone burst mode
#define EEPROM_FM_SQUELCH_SETTING			304     // FM squelch setting
#define EEPROM_FM_RX_BANDWIDTH				305     // bandwidth setting for FM reception
#define EEPROM_RX_IQ_FM_GAIN_BALANCE		306     // IQ Gain balance for AM reception
#define EEPROM_FM_SUBAUDIBLE_TONE_DET		307     // index for storage of subaudible tone detection
#define EEPROM_KEYBOARD_BEEP_FREQ			308     // keyboard beep frequency (in Hz)
#define EEPROM_BEEP_LOUDNESS				309     // loudness of beep (keyboard, sidetone test)
#define EEPROM_VERSION_MINOR				310     // Storage of current minor version number - used to detect change of firmware

#define EEPROM_DETECTOR_COUPLING_COEFF_160M	311 // Calibration coupling coefficient for FWD/REV power sensor for 160 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_6M	312 // Calibration coupling coefficient for FWD/REV power sensor for 6 meters
#define EEPROM_TUNE_POWER_LEVEL				313
// #define EEPROM_CAT_MODE_ACTIVE			314
#define EEPROM_CAT_XLAT						315
//#define EEPROM_CAT_IN_SANDBOX				316



#define EEPROM_METER_COLOUR_UP				327
#define EEPROM_METER_COLOUR_DOWN			328
#define EEPROM_FILTER_PATH_MAP_BASE			329 //
#define EEPROM_FILTER_PATH_MAP_END (329 + FILTER_MODE_MAX*FILTER_PATH_MEM_MAX)   // this is currently 5*5 = 25
//#define EEPROM_SPECTRUM_LIGHT_ENABLE		355
#define EEPROM_MANUAL_NOTCH					356
#define EEPROM_MANUAL_PEAK					357
#define EEPROM_TX_IQ_80M_PHASE_BALANCE_TRANS_OFF		358     // IQ Gain balance for AM reception
#define EEPROM_DISPLAY_DBM					359		// dbm display
#define EEPROM_BASS_GAIN					360		// bass gain lowShelf filter
#define EEPROM_TREBLE_GAIN					361		// treble gain highShelf filter
//#define	EEPROM_S_METER						362		// S-Meter configuration
#define EEPROM_TX_FILTER					363		// TX_Filter configuration
#define EEPROM_TX_BASS_GAIN					364		// TX bass gain lowShelf filter
#define EEPROM_TX_TREBLE_GAIN				365		// TX treble gain highShelf filter
#define EEPROM_SAM_PLL_LOCKING_RANGE        366     //
#define EEPROM_SAM_PLL_STEP_RESPONSE        367     //
#define EEPROM_SAM_PLL_BANDWIDTH            368     //
#define EEPROM_I2C1_SPEED                   369     //
#define EEPROM_I2C2_SPEED                   370     //
#define EEPROM_SAM_FADE_LEVELER             371
#define EEPROM_LINEOUT_GAIN                 372
#define EEPROM_IQ_AUTO_CORRECTION           373     // Receive IQ auto correction ON/OFF
//#define EEPROM_AGC_WDSP_SWITCH              374
#define EEPROM_AGC_WDSP_MODE                375
#define EEPROM_AGC_WDSP_THRESH              376
#define EEPROM_AGC_WDSP_SLOPE               377
#define EEPROM_AGC_WDSP_HANG                378
#define EEPROM_DBM_CALIBRATE                379
#define EEPROM_AGC_WDSP_TAU_DECAY_0         380
#define EEPROM_AGC_WDSP_TAU_HANG_DECAY      381
#define EEPROM_RTC_CALIB                    382
#define EEPROM_AGC_WDSP_TAU_DECAY_1         383
#define EEPROM_AGC_WDSP_TAU_DECAY_2         384
#define EEPROM_AGC_WDSP_TAU_DECAY_3         385
#define EEPROM_AGC_WDSP_TAU_DECAY_4         386

#define EEPROM_LOW_POWER_CONFIG          387
#define EEPROM_CW_KEYER_WEIGHT           388
#define EEPROM_DIGI_MODE_CONF			 389
#define EEPROM_Scope_TRACE_HL_BW			 390
#define EEPROM_Scope_TRACE_HL_BW_BGR			 391
#define EEPROM_TScal0_High						392	//callibration data for touchscreen (all data are int32_t divided in two 16bit chunks)
#define EEPROM_TScal0_Low						393
#define EEPROM_TScal1_High						394
#define EEPROM_TScal1_Low						395
#define EEPROM_TScal2_High						396
#define EEPROM_TScal2_Low						397
#define EEPROM_TScal3_High						398
#define EEPROM_TScal3_Low						399
#define EEPROM_TScal4_High						400
#define EEPROM_TScal4_Low						401
#define EEPROM_TScal5_High						402
#define EEPROM_TScal5_Low						403
#define EEPROM_CW_DECODER_ENABLE				404
#define EEPROM_Scope_Graticule_Ypos				405
#define EEPROM_Freq_Display_Font				406
#define EEPROM_NUMBER_OF_ENTRIES                407 // this is the index of the config value which holds the number of used config entries
                                                    // , not the value itself (which is in fact EEPROM_FIRST_UNUSED)

#define EEPROM_DSP_MODE_MASK				408
#define EEPROM_ENABLE_PTT_RTS				409
#define EEPROM_CW_DECODER_THRESH			410
#define EEPROM_CW_DECODER_BLOCKSIZE			411
#define EEPROM_SMETER_ALPHAS                412
#define EEPROM_ADJ_TX_IQ_SOMEBANDS	        413     // unused, may be reused due to short usage time in its original
#define EEPROM_TX_IQ_20M_GAIN_BALANCE		        414
#define EEPROM_TX_IQ_20M_PHASE_BALANCE		        415
#define EEPROM_TX_IQ_15M_GAIN_BALANCE		        416
#define EEPROM_TX_IQ_15M_PHASE_BALANCE		        417
#define EEPROM_TX_IQ_10M_UP_GAIN_BALANCE		    418
#define EEPROM_TX_IQ_10M_UP_PHASE_BALANCE		    419
#define EEPROM_TX_IQ_20M_GAIN_BALANCE_TRANS_OFF		420
#define EEPROM_TX_IQ_20M_PHASE_BALANCE_TRANS_OFF	421
#define EEPROM_TX_IQ_15M_GAIN_BALANCE_TRANS_OFF		422
#define EEPROM_TX_IQ_15M_PHASE_BALANCE_TRANS_OFF	423
#define EEPROM_TX_IQ_10M_UP_GAIN_BALANCE_TRANS_OFF	424
#define EEPROM_TX_IQ_10M_UP_PHASE_BALANCE_TRANS_OFF	425
#define EEPROM_VSWR_PROTECTION_THRESHOLD            426
#define EEPROM_EXPFLAGS1                            427     // Flags for options in Debug/Expert menu - see variable "expflags1"
#define EEPROM_FIRST_UNUSED                         428		// change this if new value ids are introduced, must be correct at any time

#define MAX_VAR_ADDR (EEPROM_FIRST_UNUSED - 1)

// Note: EEPROM addresses up to 383 are currently defined. If this value is passed you
// need to modify virtual EEPROM routines otherwise system may crash

#define EEPROM_KEYER_MEMORY_ADDRESS		0x1000

#endif /* DRIVERS_UI_UI_CONFIGURATION_H_ */
