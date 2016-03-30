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
**  Licence:        CC BY-NC-SA 3.0                                                **
************************************************************************************/
#ifndef DRIVERS_UI_UI_CONFIGURATION_H_
#define DRIVERS_UI_UI_CONFIGURATION_H_
#include "mchf_board.h"

void        UiConfiguration_LoadEepromValues(void);
uint16_t    UiConfiguration_SaveEepromValues(void);

// Configuration Value Definitons Follow
//
//
#define SIDETONE_MAX_GAIN   10      // Maximum sidetone gain
#define DEFAULT_SIDETONE_GAIN   5   // Default sidetone gain
//
#define MIN_KEYER_SPEED     5       // Minimum keyer speed
#define MAX_KEYER_SPEED     48      // Maximum keyer speed
#define DEFAULT_KEYER_SPEED 20      // Default keyer speed
//
#define CW_OFFSET_USB_TX    0       // CW in USB mode, display is TX frequency if received frequency was zero-beated
#define CW_OFFSET_LSB_TX    1       // CW in LSB mode, display is TX frequency if received frequency was zero-beated
#define CW_OFFSET_AUTO_TX   2       // Same as CW_OFFSET_USB_TX except LSB if frequency is < 10 MHz, USB if >= 10 MHz
#define CW_OFFSET_USB_RX    3       // CW in USB mode, display is RX frequency if received signal is matched to sidetone
#define CW_OFFSET_LSB_RX    4       // CW in LSB mode, display is RX frequency if received signal is matched to sidetone
#define CW_OFFSET_AUTO_RX   5       // Same as CW_OFFSET_USB_RX except LSB if frequency is < 10 MHz, USB if >= 10 MHz
#define CW_OFFSET_USB_SHIFT 6       // CW in USB mode, LO shifts, display is RX frequency if signal is matched to sidetone
#define CW_OFFSET_LSB_SHIFT 7       // CW in LSB mode, LO shifts, display is RX frequency if signal is matched to sidetone
#define CW_OFFSET_AUTO_SHIFT    8   // Same as "CW_OFFSET_USB_SHIFT" except LSB if frequency is <10 MHz, USB of >= 10 MHz
#define CW_OFFSET_MAX       8       // Maximum menu setting
#define CW_OFFSET_MODE_DEFAULT  0   // Default CW offset setting
//
#define USB_FREQ_THRESHOLD  40000000    // LO frequency at and above which the default is USB, Hz*4  (e.g. 10 MHz = 40 MHz)
//
#define MAX_RF_ATTEN        15      // Maximum setting for RF attenuation
//
#define MIN_RIT_VALUE       -50     // Minimum RIT Value
#define MAX_RIT_VALUE       50      // Maximum RIT Value
//
#define MAX_RF_GAIN         50      // Maximum RF gain setting
#define DEFAULT_RF_GAIN     50      // Default RF gain setting
//
#define MAX_RF_CODEC_GAIN_VAL       9       // Maximum RF gain setting
#define DEFAULT_RF_CODEC_GAIN_VAL   9       // Default RF gain setting (9 = AUTO mode)
//
#define MAX_AUDIO_GAIN      30      // Maximum audio gain setting
#define MAX_DIG_GAIN      31      // Maximum audio gain setting
#define DEFAULT_AUDIO_GAIN  16      // Default audio gain
#define DEFAULT_DIG_GAIN    16      // Default audio gain
//
// The following are used in the max volume setting in the menu system
//
#define MAX_VOLUME_MIN              8       // Minimum setting for maximum volume
#define MAX_VOLUME_MAX              MAX_AUDIO_GAIN      // Maximum setting for maximum volume
#define MAX_VOLUME_DEFAULT          DEFAULT_AUDIO_GAIN
//
#define MAX_VOL_RED_THRESH  10      // "MAX VOLUME" setting at or below which number will be RED to warn user
#define MAX_VOLT_YELLOW_THRESH  16  // "MAX VOLUME" setting at or below which number will be YELLOW to warn user
//
//
#define MAX_PA_BIAS         115     // Maximum PA Bias Setting
#define DEFAULT_PA_BIAS     0       // Default PA Bias setting
#define MIN_BIAS_SETTING    20      // Minimum bias setting.  (Below this, number is red)
//
#define BIAS_OFFSET         25      // Offset value to be added to bias setting
//  DA value = (OFFSET + (2*setting))  where DA value is 0-255
//
#define MIN_TX_IQ_GAIN_BALANCE  -99 // Minimum setting for TX IQ gain balance
#define MAX_TX_IQ_GAIN_BALANCE  99  // Maximum setting for TX IQ gain balance
//
#define MIN_RX_IQ_GAIN_BALANCE  -99 // Minimum setting for RX IQ gain balance
#define MAX_RX_IQ_GAIN_BALANCE  99  // Maximum setting for RX IQ gain balance
//
#define MIN_TX_IQ_PHASE_BALANCE -100 // Minimum setting for TX IQ phase balance
#define MAX_TX_IQ_PHASE_BALANCE 100  // Maximum setting for TX IQ phase balance
//
#define MIN_RX_IQ_PHASE_BALANCE -100 // Minimum setting for RX IQ phase balance
#define MAX_RX_IQ_PHASE_BALANCE 100  // Maximum setting for RX IQ phase balance
//
#define XVERTER_MULT_MAX        10      // maximum LO multipler in xverter mode
#define XVERTER_OFFSET_MAX      999000000   // Maximum transverter offset (999 MHz)
//
#define AUTO_LSB_USB_OFF        0
#define AUTO_LSB_USB_ON         1
#define AUTO_LSB_USB_60M        2
#define AUTO_LSB_USB_MAX        2
#define AUTO_LSB_USB_DEFAULT    AUTO_LSB_USB_60M

#define MAX_VAR_ADDR 383
//
// *************************************************************************************************************************
//
// Eeprom items IDs - if updating, make sure eeprom.h list
// is updated as well!!!
//
// These do NOT use "enum" as it is important that the number *NOT* change by the ineration of new variables:  All NEW variable should be placed at the END of the
// list to maintain compatibility with older versions and the settings!
//
#define EEPROM_ZERO_LOC_UNRELIABLE  0       // DO NOT USE LOCATION ZERO AS IT MAY BE UNRELIABLE!!!!
#define EEPROM_BAND_MODE        1
#define EEPROM_FREQ_HIGH        2
#define EEPROM_FREQ_LOW         3
#define EEPROM_FREQ_STEP        4
#define EEPROM_TX_AUDIO_SRC     5
#define EEPROM_TCXO_STATE       6
#define EEPROM_PA_BIAS          7
#define EEPROM_AUDIO_GAIN       8
#define EEPROM_RX_CODEC_GAIN        9
#define EEPROM_MAX_VOLUME       10
#define EEPROM_POWER_STATE      11
#define EEPROM_TX_POWER_LEVEL       12
#define EEPROM_KEYER_SPEED      13
#define EEPROM_KEYER_MODE       14
#define EEPROM_SIDETONE_GAIN        15
#define EEPROM_MIC_BOOST        16
#define EEPROM_TX_IQ_LSB_GAIN_BALANCE   17      // TX gain balance
#define EEPROM_TX_IQ_LSB_PHASE_BALANCE  18      // TX phase balance
#define EEPROM_RX_IQ_LSB_GAIN_BALANCE   19
#define EEPROM_RX_IQ_LSB_PHASE_BALANCE  20
//
#define EEPROM_BAND0_MODE       21      // Band/mode/filter memory per-band - bands indexed from here
#define EEPROM_BAND1_MODE       22
#define EEPROM_BAND2_MODE       23
#define EEPROM_BAND3_MODE       24
#define EEPROM_BAND4_MODE       25
#define EEPROM_BAND5_MODE       26
#define EEPROM_BAND6_MODE       27
#define EEPROM_BAND7_MODE       28
#define EEPROM_BAND8_MODE       29
#define EEPROM_BAND9_MODE       30
#define EEPROM_BAND10_MODE      31
#define EEPROM_BAND11_MODE      32
#define EEPROM_BAND12_MODE      33
#define EEPROM_BAND13_MODE      34
#define EEPROM_BAND14_MODE      35
#define EEPROM_BAND15_MODE      36
#define EEPROM_BAND16_MODE      37
#define EEPROM_BAND17_MODE      38      // "Floating" General coverage band
//
//
#define EEPROM_BAND0_FREQ_HIGH      39      // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_HIGH      40
#define EEPROM_BAND2_FREQ_HIGH      41
#define EEPROM_BAND3_FREQ_HIGH      42
#define EEPROM_BAND4_FREQ_HIGH      43
#define EEPROM_BAND5_FREQ_HIGH      44
#define EEPROM_BAND6_FREQ_HIGH      45
#define EEPROM_BAND7_FREQ_HIGH      46
#define EEPROM_BAND8_FREQ_HIGH      47
#define EEPROM_BAND9_FREQ_HIGH      48
#define EEPROM_BAND10_FREQ_HIGH     49
#define EEPROM_BAND11_FREQ_HIGH     50
#define EEPROM_BAND12_FREQ_HIGH     51
#define EEPROM_BAND13_FREQ_HIGH     52
#define EEPROM_BAND14_FREQ_HIGH     53
#define EEPROM_BAND15_FREQ_HIGH     54
#define EEPROM_BAND16_FREQ_HIGH     55
#define EEPROM_BAND17_FREQ_HIGH     56      // "Floating" General coverage band
//
//
#define EEPROM_BAND0_FREQ_LOW       57      // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_LOW       58
#define EEPROM_BAND2_FREQ_LOW       59
#define EEPROM_BAND3_FREQ_LOW       60
#define EEPROM_BAND4_FREQ_LOW       61
#define EEPROM_BAND5_FREQ_LOW       62
#define EEPROM_BAND6_FREQ_LOW       63
#define EEPROM_BAND7_FREQ_LOW       64
#define EEPROM_BAND8_FREQ_LOW       65
#define EEPROM_BAND9_FREQ_LOW       66
#define EEPROM_BAND10_FREQ_LOW      67
#define EEPROM_BAND11_FREQ_LOW      68
#define EEPROM_BAND12_FREQ_LOW      69
#define EEPROM_BAND13_FREQ_LOW      70
#define EEPROM_BAND14_FREQ_LOW      71
#define EEPROM_BAND15_FREQ_LOW      72
#define EEPROM_BAND16_FREQ_LOW      73
#define EEPROM_BAND17_FREQ_LOW      74      // "Floating" General coverage band
//
//
#define EEPROM_FREQ_CAL         75      // Frequency calibration
#define EEPROM_NB_SETTING       76      // Noise Blanker Setting
#define EEPROM_AGC_MODE         77      // AGC setting
#define EEPROM_MIC_GAIN         78      // Mic gain setting
#define EEPROM_LINE_GAIN        79      // Line gain setting
#define EEPROM_SIDETONE_FREQ        80      // Sidetone frequency (Hz)
#define EEPROM_SPEC_SCOPE_SPEED     81      // Spectrum Scope Speed
#define EEPROM_SPEC_SCOPE_FILTER    82      // Spectrum Scope filter strength
#define EEPROM_RX_GAIN          83      // RX Gain setting (e.g. minimum RF gain as might be used for manual AGC)
#define EEPROM_AGC_CUSTOM_DECAY     84      // Custom setting for AGC decay rate
#define EEPROM_SPECTRUM_TRACE_COLOUR    85      // Custom setting for spectrum scope trace colour
#define EEPROM_SPECTRUM_GRID_COLOUR 86      // Custom setting for spectrum scope grid colour
#define EEPROM_SPECTRUM_SCALE_COLOUR    87      // Custom setting for spectrum scope frequency scale colour
#define EEPROM_PADDLE_REVERSE       88      // TRUE if paddle is to be reversed
#define EEPROM_CW_RX_DELAY      89      // Delay after last CW element before returning to receive
#define EEPROM_SPECTRUM_CENTRE_GRID_COLOUR  90  // Custom setting for spectrum scope grid center marker colour
//
#define EEPROM_DETECTOR_COUPLING_COEFF_2200M    91  // Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_630M 92  // Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_80M  93  // Calibration coupling coefficient for FWD/REV power sensor for 80 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_40M  94  // Calibration coupling coefficient for FWD/REV power sensor for 60/40 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_20M  95  // Calibration coupling coefficient for FWD/REV power sensor for 30/20/17 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_15M  96  // Calibration coupling coefficient for FWD/REV power sensor for 15/12/10 meters
//
// The following are the coefficients used to set the RF output power settings
//
#define EEPROM_BAND0_5W         97      // 5 watt power setting, 80m
#define EEPROM_BAND1_5W         98      // 5 watt power setting, 60m
#define EEPROM_BAND2_5W         99      // 5 watt power setting, 40m
#define EEPROM_BAND3_5W         100     // 5 watt power setting, 30m
#define EEPROM_BAND4_5W         101     // 5 watt power setting, 20m
#define EEPROM_BAND5_5W         102     // 5 watt power setting, 17m
#define EEPROM_BAND6_5W         103     // 5 watt power setting, 15m
#define EEPROM_BAND7_5W         104     // 5 watt power setting, 12m
#define EEPROM_BAND8_5W         105     // 5 watt power setting, 10m
#define EEPROM_BAND9_5W         106     // 5 watt power setting, 6m
#define EEPROM_BAND10_5W        107     // 5 watt power setting, 4m
#define EEPROM_BAND11_5W        108     // 5 watt power setting, 2m
#define EEPROM_BAND12_5W        109     // 5 watt power setting, 70cm
#define EEPROM_BAND13_5W        110     // 5 watt power setting, 23cm
#define EEPROM_BAND14_5W        111     // 5 watt power setting, 2200m
#define EEPROM_BAND15_5W        112     // 5 watt power setting, 630m
#define EEPROM_BAND16_5W        113     // 5 watt power setting, 160m
#define EEPROM_BAND17_5W        114     // reserved
//
#define EEPROM_BAND0_FULL       115     // "FULL" power setting, 80m
#define EEPROM_BAND1_FULL       116     // "FULL" power setting, 60m
#define EEPROM_BAND2_FULL       117     // "FULL" power setting, 40m
#define EEPROM_BAND3_FULL       118     // "FULL" power setting, 30m
#define EEPROM_BAND4_FULL       119     // "FULL" power setting, 20m
#define EEPROM_BAND5_FULL       120     // "FULL" power setting, 17m
#define EEPROM_BAND6_FULL       121     // "FULL" power setting, 15m
#define EEPROM_BAND7_FULL       122     // "FULL" power setting, 12m
#define EEPROM_BAND8_FULL       123     // "FULL" power setting, 10m
#define EEPROM_BAND9_FULL       124     // "FULL" power setting, 6m
#define EEPROM_BAND10_FULL      125     // "FULL" power setting, 4m
#define EEPROM_BAND11_FULL      126     // "FULL" power setting, 2m
#define EEPROM_BAND12_FULL      127     // "FULL" power setting, 70cm
#define EEPROM_BAND13_FULL      128     // "FULL" power setting, 23cm
#define EEPROM_BAND14_FULL      129     // "FULL" power setting, 2200m
#define EEPROM_BAND15_FULL      130     // "FULL" power setting, 630m
#define EEPROM_BAND16_FULL      131     // "FULL" power setting, 160m
#define EEPROM_BAND17_FULL      132     // reserved
//
#define EEPROM_FILTER_300HZ_SEL     133     // Selection of 300 Hz filter
#define EEPROM_FILTER_500HZ_SEL     134     // Selection of 500 Hz filter
#define EEPROM_FILTER_1K8_SEL       135     // Selection of 1.8 kHz filter
#define EEPROM_FILTER_2K3_SEL       136     // Selection of 2.3 kHz filter
#define EEPROM_FILTER_3K6_SEL       137     // Selection of 3.6 kHz filter
#define EEPROM_FILTER_WIDE_SEL      138     // Selection of "Wide" filter (>3.6kHz)
//
#define EEPROM_TX_IQ_USB_GAIN_BALANCE   139     // TX gain balance
#define EEPROM_TX_IQ_USB_PHASE_BALANCE  140     // TX phase balance
#define EEPROM_RX_IQ_USB_GAIN_BALANCE   141
#define EEPROM_RX_IQ_USB_PHASE_BALANCE  142
#define EEPROM_SENSOR_NULL      143     // Power meter sensor null calibrate
//#define   EEPROM_REV_PWR_CAL  144     // REV power meter calibrate
//
#define EEPROM_XVERTER_DISP     145     // TRUE if display is offset with transverter frequency offset
#define EEPROM_XVERTER_OFFSET_HIGH  146     // Frequency by which the display is offset for transverter use, high byte
//
#define EEPROM_VFO_MEM_MODE     147     // settings of VFO/SPLIT/Memory configuration bits - see variable "vfo_mem_mode" for information.
//
#define EEPROM_XVERTER_OFFSET_LOW   148     // Low byte of above
//
#define EEPROM_SPECTRUM_MAGNIFY     149     // TRUE if spectrum scope is to be magnified
//
#define EEPROM_WIDE_FILT_CW_DISABLE 150     // TRUE if wide filters are to be disabled in CW mode
#define EEPROM_NARROW_FILT_SSB_DISABLE  151     // TRUE if narrow filters are to be disabled in SSB mode
//
#define EEPROM_AM_MODE_DISABLE      152     // TRUE if AM mode is to be disabled
//
#define EEPROM_PA_CW_BIAS       153     // If non-zero, this is the PA bias setting when in CW mode
//
#define EEPROM_SPECTRUM_DB_DIV      154     // Spectrum Scope dB/Division
#define EEPROM_SPECTRUM_AGC_RATE    155     // AGC setting for spectrum scope
//
#define EEPROM_METER_MODE       156     // Stored setting of meter mode
//
#define EEPROM_ALC_DECAY_TIME       157     // ALC Decay time
#define EEPROM_ALC_POSTFILT_TX_GAIN 158     // ALC post-filter TX audio gain
//
#define EEPROM_STEP_SIZE_CONFIG     159     // TRUE if there is to be a line under the frequency digit indicating step size
//
#define EEPROM_DSP_MODE         160     // Stores the DSP operational mode
#define EEPROM_DSP_NR_STRENGTH      161     // Stores the DSP Noise Reduction operational strength
#define EEPROM_DSP_NR_DECOR_BUFLEN  162     // DSP Noise Reduction De-correlator buffer length
#define EEPROM_DSP_NR_FFT_NUMTAPS   163     // DSP Noise Reduction FFT number of taps
#define EEPROM_DSP_NOTCH_DECOR_BUFLEN   164     // DSP Notch De-correlator buffer length
#define EEPROM_DSP_NOTCH_CONV_RATE  165     // DSP Notch convergence rate
//
#define EEPROM_MAX_RX_GAIN      166     // Maximum RX gain - adjusts maximum allowed AGC gain in S-units
#define EEPROM_TX_AUDIO_COMPRESS    167     // TX audio compressor setting, used to calculate other values
//
#define EEPROM_RX_IQ_AM_GAIN_BALANCE    168     // IQ Gain balance for AM reception
//
#define EEPROM_TX_DISABLE       169     // TRUE of transmit is to be disabled
#define EEPROM_MISC_FLAGS1      170     // Miscellaneous status flag, saved in EEPROM - see variable "misc_flags1"
#define EEPROM_VERSION_NUMBER       171     // Storage of current version release - used to detect change of firmware
#define EEPROM_NB_AGC_TIME_CONST    172     // Noise blanker AGC time constant setting
#define EEPROM_CW_OFFSET_MODE       173     // CW Offset mode
#define EEPROM_FREQ_CONV_MODE       174     // Frequency Conversion Mode (e.g. I/Q frequency conversion done in receive/transmit to offset from zero)
#define EEPROM_LSB_USB_AUTO_SELECT  175     // Auto selection of LSB/USB above/below 10 MHz (including 60 meters)
#define EEPROM_VERSION_BUILD        176     // Storage of current version build number - used to detect change of firmware
#define EEPROM_LCD_BLANKING_CONFIG  177     // Configuration of automatic LCD blanking mode settings
#define EEPROM_VOLTMETER_CALIBRATE  178     // Holder for calibration of the on-screen voltmeter
#define EEPROM_WATERFALL_COLOR_SCHEME   179     // Color scheme for waterfall display
#define EEPROM_WATERFALL_VERTICAL_STEP_SIZE 180 // Number of vertical steps of waterfall per iteration
#define EEPROM_WATERFALL_OFFSET     181     // Palette offset for waterfall
#define EEPROM_WATERFALL_CONTRAST   182     // Palette contrast multiplier for waterfall
//
// VFO A storage
//
#define EEPROM_BAND0_MODE_A     183     // Band/mode/filter memory per-band - bands indexed from here
#define EEPROM_BAND1_MODE_A     184
#define EEPROM_BAND2_MODE_A     185
#define EEPROM_BAND3_MODE_A     186
#define EEPROM_BAND4_MODE_A     187
#define EEPROM_BAND5_MODE_A     188
#define EEPROM_BAND6_MODE_A     189
#define EEPROM_BAND7_MODE_A     190
#define EEPROM_BAND8_MODE_A     191
#define EEPROM_BAND9_MODE_A     192
#define EEPROM_BAND10_MODE_A        193
#define EEPROM_BAND11_MODE_A        194
#define EEPROM_BAND12_MODE_A        195
#define EEPROM_BAND13_MODE_A        196
#define EEPROM_BAND14_MODE_A        197
#define EEPROM_BAND15_MODE_A        198
#define EEPROM_BAND16_MODE_A        199
#define EEPROM_BAND17_MODE_A        200     // "Floating" General coverage band
//
#define EEPROM_BAND0_FREQ_HIGH_A    201     // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_HIGH_A    202
#define EEPROM_BAND2_FREQ_HIGH_A    203
#define EEPROM_BAND3_FREQ_HIGH_A    204
#define EEPROM_BAND4_FREQ_HIGH_A    205
#define EEPROM_BAND5_FREQ_HIGH_A    206
#define EEPROM_BAND6_FREQ_HIGH_A    207
#define EEPROM_BAND7_FREQ_HIGH_A    208
#define EEPROM_BAND8_FREQ_HIGH_A    209
#define EEPROM_BAND9_FREQ_HIGH_A    210
#define EEPROM_BAND10_FREQ_HIGH_A   211
#define EEPROM_BAND11_FREQ_HIGH_A   212
#define EEPROM_BAND12_FREQ_HIGH_A   213
#define EEPROM_BAND13_FREQ_HIGH_A   214
#define EEPROM_BAND14_FREQ_HIGH_A   215
#define EEPROM_BAND15_FREQ_HIGH_A   216
#define EEPROM_BAND16_FREQ_HIGH_A   217
#define EEPROM_BAND17_FREQ_HIGH_A   218     // "Floating" General coverage band
//
#define EEPROM_BAND0_FREQ_LOW_A     219     // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_LOW_A     220
#define EEPROM_BAND2_FREQ_LOW_A     221
#define EEPROM_BAND3_FREQ_LOW_A     222
#define EEPROM_BAND4_FREQ_LOW_A     223
#define EEPROM_BAND5_FREQ_LOW_A     224
#define EEPROM_BAND6_FREQ_LOW_A     225
#define EEPROM_BAND7_FREQ_LOW_A     226
#define EEPROM_BAND8_FREQ_LOW_A     227
#define EEPROM_BAND9_FREQ_LOW_A     228
#define EEPROM_BAND10_FREQ_LOW_A    229
#define EEPROM_BAND11_FREQ_LOW_A    230
#define EEPROM_BAND12_FREQ_LOW_A    231
#define EEPROM_BAND13_FREQ_LOW_A    232
#define EEPROM_BAND14_FREQ_LOW_A    233
#define EEPROM_BAND15_FREQ_LOW_A    234
#define EEPROM_BAND16_FREQ_LOW_A    235
#define EEPROM_BAND17_FREQ_LOW_A    236     // "Floating" General coverage band
//
// VFO B storage
//
#define EEPROM_BAND0_MODE_B     237     // Band/mode/filter memory per-band - bands indexed from here
#define EEPROM_BAND1_MODE_B     238
#define EEPROM_BAND2_MODE_B     239
#define EEPROM_BAND3_MODE_B     240
#define EEPROM_BAND4_MODE_B     241
#define EEPROM_BAND5_MODE_B     242
#define EEPROM_BAND6_MODE_B     243
#define EEPROM_BAND7_MODE_B     244
#define EEPROM_BAND8_MODE_B     245
#define EEPROM_BAND9_MODE_B     246
#define EEPROM_BAND10_MODE_B        247
#define EEPROM_BAND11_MODE_B        248
#define EEPROM_BAND12_MODE_B        249
#define EEPROM_BAND13_MODE_B        250
#define EEPROM_BAND14_MODE_B        251
#define EEPROM_BAND15_MODE_B        252
#define EEPROM_BAND16_MODE_B        253
#define EEPROM_BAND17_MODE_B        254     // "Floating" General coverage band
//
//
#define EEPROM_BAND0_FREQ_HIGH_B    255     // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_HIGH_B    256
#define EEPROM_BAND2_FREQ_HIGH_B    257
#define EEPROM_BAND3_FREQ_HIGH_B    258
#define EEPROM_BAND4_FREQ_HIGH_B    259
#define EEPROM_BAND5_FREQ_HIGH_B    260
#define EEPROM_BAND6_FREQ_HIGH_B    261
#define EEPROM_BAND7_FREQ_HIGH_B    262
#define EEPROM_BAND8_FREQ_HIGH_B    263
#define EEPROM_BAND9_FREQ_HIGH_B    264
#define EEPROM_BAND10_FREQ_HIGH_B   265
#define EEPROM_BAND11_FREQ_HIGH_B   266
#define EEPROM_BAND12_FREQ_HIGH_B   267
#define EEPROM_BAND13_FREQ_HIGH_B   268
#define EEPROM_BAND14_FREQ_HIGH_B   269
#define EEPROM_BAND15_FREQ_HIGH_B   270
#define EEPROM_BAND16_FREQ_HIGH_B   271
#define EEPROM_BAND17_FREQ_HIGH_B   272     // "Floating" General coverage band
//
//
#define EEPROM_BAND0_FREQ_LOW_B     273     // Per-band frequency, high word - bands indexed from here
#define EEPROM_BAND1_FREQ_LOW_B     274
#define EEPROM_BAND2_FREQ_LOW_B     275
#define EEPROM_BAND3_FREQ_LOW_B     276
#define EEPROM_BAND4_FREQ_LOW_B     277
#define EEPROM_BAND5_FREQ_LOW_B     278
#define EEPROM_BAND6_FREQ_LOW_B     279
#define EEPROM_BAND7_FREQ_LOW_B     280
#define EEPROM_BAND8_FREQ_LOW_B     281
#define EEPROM_BAND9_FREQ_LOW_B     282
#define EEPROM_BAND10_FREQ_LOW_B    283
#define EEPROM_BAND11_FREQ_LOW_B    284
#define EEPROM_BAND12_FREQ_LOW_B    285
#define EEPROM_BAND13_FREQ_LOW_B    286
#define EEPROM_BAND14_FREQ_LOW_B    287
#define EEPROM_BAND15_FREQ_LOW_B    288
#define EEPROM_BAND16_FREQ_LOW_B    289
#define EEPROM_BAND17_FREQ_LOW_B    290     // "Floating" General coverage band
//
#define EEPROM_WATERFALL_SPEED      291     // Spectrum Scope Speed
#define EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST  292 // adjustment for no-signal conditions of spectrum scope
#define EEPROM_WATERFALL_NOSIG_ADJUST   293     // adjustment for no-signal conditions of waterfall
#define EEPROM_DSP_NOTCH_FFT_NUMTAPS    294     // DSP Notch FFT number of taps
#define EEPROM_WATERFALL_SIZE       295     // size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved
#define EEPROM_FFT_WINDOW       296     // FFT Window information (lower nybble currently used - upper nybble reserved)
#define EEPROM_TX_PTT_AUDIO_MUTE    297     // timer used for muting TX audio when keying PTT to suppress "click" or "thump"
#define EEPROM_MISC_FLAGS2      298     // Miscellaneous status flag, saved in EEPROM - see variable "misc_flags2"
#define EEPROM_FILTER_DISP_COLOUR   299     // This contains the color of the line under the spectrum/waterfall display
#define EEPROM_TX_IQ_AM_GAIN_BALANCE    300     // IQ Gain balance for AM transmission
#define EEPROM_TX_IQ_FM_GAIN_BALANCE    301     // IQ Gain balance for FM transmission
#define EEPROM_FM_SUBAUDIBLE_TONE_GEN   302     // index for storage of subaudible tone generation
#define EEPROM_FM_TONE_BURST_MODE   303     // tone burst mode
#define EEPROM_FM_SQUELCH_SETTING   304     // FM squelch setting
#define EEPROM_FM_RX_BANDWIDTH      305     // bandwidth setting for FM reception
#define EEPROM_RX_IQ_FM_GAIN_BALANCE    306     // IQ Gain balance for AM reception
#define EEPROM_FM_SUBAUDIBLE_TONE_DET   307     // index for storage of subaudible tone detection
#define EEPROM_KEYBOARD_BEEP_FREQ   308     // keyboard beep frequency (in Hz)
#define EEPROM_BEEP_LOUDNESS        309     // loudness of beep (keyboard, sidetone test)
#define EEPROM_VERSION_MINOR        310     // Storage of current minor version number - used to detect change of firmware

#define EEPROM_DETECTOR_COUPLING_COEFF_160M 311 // Calibration coupling coefficient for FWD/REV power sensor for 160 meters
#define EEPROM_DETECTOR_COUPLING_COEFF_6M   312 // Calibration coupling coefficient for FWD/REV power sensor for 6 meters
#define EEPROM_TUNE_POWER_LEVEL     313
#define EEPROM_CAT_MODE_ACTIVE      314
#define EEPROM_CAT_XLAT         315
#define EEPROM_FILTER_2K7_SEL       316     // Selection of 2.7 kHz filter
#define EEPROM_FILTER_2K9_SEL       317     // Selection of 2.9 kHz filter
#define EEPROM_FILTER_1K4_SEL       318     //
#define EEPROM_FILTER_1K6_SEL       319     //
#define EEPROM_FILTER_2K1_SEL       320     //
#define EEPROM_FILTER_2K5_SEL       321     //
#define EEPROM_FILTER_3K2_SEL       322     //
#define EEPROM_FILTER_3K4_SEL       323     //
#define EEPROM_FILTER_3K8_SEL       324     //
#define EEPROM_FILTER_1_SEL     325     // selection of filters 4k0 to 6k0
#define EEPROM_FILTER_2_SEL     326     // selection of filters 6k5 to 10k0
#define EEPROM_DYNAMIC_TUNING       327
#define EEPROM_SAM_ENABLE       328     // SAM demodulation enable
#define EEPROM_FILTER_PATH_MAP_BASE 329 //
#define EEPROM_FILTER_PATH_MAP_END (329 + FILTER_MODE_MAX*FILTER_PATH_MEM_MAX)   // this is currently 5*5 = 25
#define EEPROM_FIRST_UNUSED 329+25  // change this if new value ids are introduced


#endif /* DRIVERS_UI_UI_CONFIGURATION_H_ */
