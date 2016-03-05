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
#define MIN_TX_IQ_PHASE_BALANCE -32 // Minimum setting for TX IQ phase balance
#define MAX_TX_IQ_PHASE_BALANCE 32  // Maximum setting for TX IQ phase balance
//
#define MIN_RX_IQ_PHASE_BALANCE -32 // Minimum setting for RX IQ phase balance
#define MAX_RX_IQ_PHASE_BALANCE 32  // Maximum setting for RX IQ phase balance
//
#define XVERTER_MULT_MAX        10      // maximum LO multipler in xverter mode
#define XVERTER_OFFSET_MAX      999000000   // Maximum transverter offset (999 MHz)
//
#define AUTO_LSB_USB_OFF        0
#define AUTO_LSB_USB_ON         1
#define AUTO_LSB_USB_60M        2
#define AUTO_LSB_USB_MAX        2
#define AUTO_LSB_USB_DEFAULT    AUTO_LSB_USB_60M



#endif /* DRIVERS_UI_UI_CONFIGURATION_H_ */
