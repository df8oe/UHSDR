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
/*
 * radio_management.h
 *
 *  Created on: 23.12.2016
 *      Author: danilo
 */

#ifndef DRIVERS_UI_RADIO_MANAGEMENT_H_
#define DRIVERS_UI_RADIO_MANAGEMENT_H_

#include "uhsdr_types.h"
#include "uhsdr_board.h"
// Frequency public structure

#define MAX_DIGITS 10

typedef struct DialFrequency
{
    // pot values
    //
    // user visible frequency AKA dial frequency
    // NOT the always LO frequency since we have an IQ TRX which
    // uses frequency translation in many cases.
    ulong   tune_old;           // current value
    ulong   tune_new;           // requested value

    // Current tuning step
    ulong   tuning_step;        // selected step by user
    ulong   selected_idx;       // id of step
    ulong   step_new;           // Eth driver req step

    ulong   update_skip;

    // TCXO routine factor/flag
    int     temp_factor;
    bool    temp_factor_changed;
    uchar   temp_enabled;
#define TCXO_MODE_MASK 0x0f
#define TCXO_UNIT_MASK 0x10
#define TCXO_UNIT_C 0x00
#define TCXO_UNIT_F 0x10

} DialFrequency;

// ------------------------------------------------
// Frequency public
extern DialFrequency               df;

typedef enum
{
    TCXO_OFF =              0,      // TXCO temperature compensation off,
    TCXO_ON  =              1,      // TCXO temperature compensation on
    TCXO_STOP =             2,      // TXCO temperature compensation off, but read temperature sensor
    TCXO_STATE_NUMBER               // Maximum setting for TCXO setting state
} Tcxo_Mode_t;


inline Tcxo_Mode_t RadioManagement_TcxoGetMode()
{
    return (df.temp_enabled & TCXO_MODE_MASK);
}
inline void RadioManagement_TcxoSetMode(Tcxo_Mode_t mode)
{
    df.temp_enabled = (df.temp_enabled & ~TCXO_MODE_MASK) | (mode & TCXO_MODE_MASK) ;
}

inline bool RadioManagement_TcxoIsEnabled()
{
    return (RadioManagement_TcxoGetMode())!= TCXO_OFF;
}

inline void RadioManagement_TcxoSetUnit(uint8_t unit)
{
    df.temp_enabled = (df.temp_enabled & ~TCXO_UNIT_MASK) | (unit & TCXO_UNIT_MASK);
}

inline bool RadioManagement_TcxoIsFahrenheit()
{
    return (df.temp_enabled & TCXO_UNIT_MASK) == TCXO_UNIT_F;
}

// PA power level setting enumeration
// this order MUST match the order of entries in power_levels in radio_management.c !
typedef enum
{
    PA_LEVEL_FULL = 0,
    PA_LEVEL_HIGH,
    PA_LEVEL_MEDIUM,
    PA_LEVEL_LOW,
    PA_LEVEL_MINIMAL,
    PA_LEVEL_TUNE_KEEP_CURRENT
} power_level_t;


typedef struct {
    power_level_t id;
    int32_t   mW;
} power_level_desc_t;

typedef struct {
    const power_level_desc_t* levels;
    const uint32_t count;
} pa_power_levels_info_t;

extern const pa_power_levels_info_t mchf_power_levelsInfo;

typedef struct
{
    char* name;
    float32_t  reference_power;
    uint32_t  max_freq;
    uint32_t  min_freq;
    int32_t max_am_power;
    int32_t max_power; // power level upper limit, used for display
} pa_info_t;

extern const pa_info_t mchf_pa;


#define PA_LEVEL_DEFAULT        PA_LEVEL_MEDIUM     // Default power level

#define DEFAULT_FREQ_OFFSET     3000              // Amount of offset (at LO freq) when loading "default" frequency

// this list MUST fit the order in the bandInfo structure defined in RadioManagement.h
typedef enum
{
    BAND_MODE_80 = 0,
    BAND_MODE_60 = 1,
    BAND_MODE_40 = 2,
    BAND_MODE_30 = 3,
    BAND_MODE_20 = 4,
    BAND_MODE_17 = 5,
    BAND_MODE_15 = 6,
    BAND_MODE_12 = 7,
    BAND_MODE_10 = 8,
    BAND_MODE_6 =  9,
    BAND_MODE_4 =  10,
    BAND_MODE_2 =  11,
    BAND_MODE_70 = 12,
    BAND_MODE_23 = 13,
    BAND_MODE_2200 = 14,
    BAND_MODE_630 = 15,
    BAND_MODE_160 = 16,
    BAND_MODE_GEN = 17          // General Coverage
} band_mode_t;


typedef enum
{
    COUPLING_2200M = 0,
    COUPLING_630M,
    COUPLING_160M,
    COUPLING_80M,
    COUPLING_40M,
    COUPLING_20M,
    COUPLING_15M,
    COUPLING_6M,
    COUPLING_2M,
    COUPLING_70CM,
    COUPLING_23CM,
    COUPLING_MAX
} filter_band_t;

typedef struct BandInfo
{
    uint32_t tune;
    uint32_t size;
    const char* name;
    uint32_t band_mode;
} BandInfo;

/**
 *
 * @param band
 * @return true if band is the so called generic band (everything which is not ham tx)
 */
static inline bool RadioManagement_IsGenericBand(const BandInfo* band)
{
    return band->size == 0;
}

typedef struct
{
    const BandInfo** bands;
    const char* name;
} BandInfoSet;

extern const BandInfoSet bandInfos[];
extern const int BAND_INFO_SET_NUM;
extern uint8_t bandinfo_idx;

typedef const BandInfo BandInfo_c;
extern BandInfo_c **bandInfo;

typedef struct band_regs_s
{
    VfoReg band[MAX_BAND_NUM];
} BandRegs;

bool band_enabled[MAX_BAND_NUM]; // we store which band is to be used (or ignored)

typedef enum
{
    // VFO_WORK = 0
    VFO_A = 0,
    VFO_B,
    VFO_MAX
} vfo_name_t;
// Working register plus VFO A and VFO B registers.
extern BandRegs vfo[VFO_MAX];

vfo_name_t get_active_vfo();


// SWR and RF power meter public
typedef struct SWRMeter
{
    float fwd_calc;         // forward power readings in A/D units
    float rev_calc;         // reverse power readings in A/D units
    float fwd_pwr;          // forward power in watts current measurement
    float rev_pwr;          // reverse power in watts current measurement
    float fwd_pwr_avg;      // forward power in watts averaged
    float rev_pwr_avg;      // reverse power in watts averaged

    float fwd_dbm;          // forward power in dBm
    float rev_dbm;          // reverse power in dBm
    float vswr;             // vswr
    float vswr_dampened;        // dampened VSWR reading
    bool  pwr_meter_disp;       // TRUE if numerical FWD/REV power metering (in milliwatts) is to be displayed
    bool  pwr_meter_was_disp;   // TRUE if numerical FWD/REV power metering WAS displayed (used to clear it)
    uchar   p_curr;         // count used to update power meter
    uchar   sensor_null;        // used to null out the sensor offset voltage

    uint8_t coupling_calc[COUPLING_MAX];

    bool high_vswr_detected;

} SWRMeter;

// New modes need to be added so that configuration compatibility is retained
// it is also necessary to extend the cw_mode_map in radio_management.c to include new modes
enum {
    CW_OFFSET_USB_TX  =   0,        // CW in USB mode, display is TX frequency if received frequency was zero-beated
    CW_OFFSET_LSB_TX,               // CW in LSB mode, display is TX frequency if received frequency was zero-beated
    CW_OFFSET_AUTO_TX,              // Same as CW_OFFSET_USB_TX except LSB if frequency is < 10 MHz, USB if >= 10 MHz
    CW_OFFSET_USB_RX,               // CW in USB mode, display is RX frequency if received signal is matched to sidetone
    CW_OFFSET_LSB_RX,               // CW in LSB mode, display is RX frequency if received signal is matched to sidetone
    CW_OFFSET_AUTO_RX,              // Same as CW_OFFSET_USB_RX except LSB if frequency is < 10 MHz, USB if >= 10 MHz
    CW_OFFSET_USB_SHIFT,            // CW in USB mode, LO shifts, display is RX frequency if signal is matched to sidetone
    CW_OFFSET_LSB_SHIFT,            // CW in LSB mode, LO shifts, display is RX frequency if signal is matched to sidetone
    CW_OFFSET_AUTO_SHIFT,           // Same as "CW_OFFSET_USB_SHIFT" except LSB if frequency is <10 MHz, USB of >= 10 MHz
    CW_OFFSET_NUM                   // Number of Modes
};

#define CW_OFFSET_MODE_DEFAULT  CW_OFFSET_USB_TX   // Default CW offset setting

typedef enum {
    CW_SB_USB = 0,
    CW_SB_LSB,
    CW_SB_AUTO
} cw_sb_t;

typedef enum {
    CW_OFFSET_TX = 0,
    CW_OFFSET_RX,
    CW_OFFSET_SHIFT
} cw_dial_t;

typedef enum
{
    DigitalMode_None = 0,
#ifdef USE_FREEDV
    DigitalMode_FreeDV,
#endif
    DigitalMode_RTTY,
    DigitalMode_BPSK,
    DigitalMode_Num_Modes
} digital_modes_t;

typedef struct
{
    const char* label;
    const uint32_t enabled;
} digital_mode_desc_t;

// The following descriptor table has to be in the order of the enum digital_modes_t in  radio_management.h
// This table is stored in flash (due to const) and cannot be written to
// for operational data per mode [r/w], use a different table with order of modes
extern const digital_mode_desc_t digimodes[DigitalMode_Num_Modes];


typedef struct
{
    cw_dial_t dial_mode;
    cw_sb_t sideband_mode;
} cw_mode_map_entry_t;

extern const cw_mode_map_entry_t cw_mode_map[];

// SWR/Power meter
extern SWRMeter                    swrm;

inline bool RadioManagement_IsTxDisabled()
{
    return (ts.tx_disable > 0);
}

inline bool RadioManagement_IsTxDisabledBy(uint8_t whom)
{
    return ((ts.tx_disable & (whom)) > 0);
}

uint32_t RadioManagement_GetRealFreqTranslationMode(uint32_t txrx_mode, uint32_t dmod_mode, uint32_t iq_freq_mode);
const BandInfo* RadioManagement_GetBand(ulong freq);
bool RadioManagement_FreqIsInBand(BandInfo_c * bandinfo, const uint32_t freq);
bool RadioManagement_FreqIsInEnabledBand ( uint32_t freq );
const BandInfo* RadioManagement_GetBandInfo(uint8_t new_band_index);
bool RadioManagement_SetPowerLevel(const BandInfo* band, power_level_t power_level);
bool RadioManagement_Tune(bool tune);
bool RadioManagement_UpdatePowerAndVSWR();
void RadioManagement_ChangeCodec(uint32_t codec, bool enableCodec);
bool RadioManagement_ChangeFrequency(bool force_update, uint32_t dial_freq,uint8_t txrx_mode);
void RadioManagement_HandlePttOnOff();
void RadioManagement_MuteTemporarilyRxAudio();

uint32_t RadioManagement_NextNormalDemodMode(uint32_t loc_mode);
uint32_t RadioManagement_NextAlternativeDemodMode(uint32_t loc_mode);

Oscillator_ResultCodes_t RadioManagement_ValidateFrequencyForTX(uint32_t dial_freq);
bool RadioManagement_IsApplicableDemodMode(uint32_t demod_mode);
void RadioManagement_SwitchTxRx(uint8_t txrx_mode, bool tune_mode);
bool RadioManagement_LSBActive(uint16_t dmod_mode);
bool RadioManagement_USBActive(uint16_t dmod_mode);
void RadioManagement_SetPaBias();
bool RadioManagement_CalculateCWSidebandMode(void);
void RadioManagement_SetDemodMode(uint8_t new_mode);
void RadioManagement_HandleRxIQSignalCodecGain();
const cw_mode_map_entry_t* RadioManagement_CWConfigValueToModeEntry(uint8_t cw_offset_mode);
uint8_t RadioManagement_CWModeEntryToConfigValue(const cw_mode_map_entry_t* mode_entry);
bool RadioManagement_UsesBothSidebands(uint16_t dmod_mode);
bool RadioManagement_IsPowerFactorReduce(uint32_t freq);
bool RadioManagement_UsesTxSidetone();
void RadioManagement_ToggleVfoAB();

bool RadioManagement_FmDevIs5khz();
void RadioManagement_FmDevSet5khz(bool is5khz);

uint32_t RadioManagement_GetTXDialFrequency();
uint32_t RadioManagement_GetRXDialFrequency();
int32_t  RadioManagement_GetCWDialOffset();

void RadioManagement_Request_TxOn();
void RadioManagement_Request_TxOff();

bool RadioManagement_SwitchTxRx_Possible();
bool RadioManagement_IsTxAtZeroIF(uint8_t dmod_mode, uint8_t digital_mode);

inline void RadioManagement_ToggleVfoMem()
{
    ts.vfo_mem_flag = ! ts.vfo_mem_flag;
}

inline bool is_demod_rtty()
{
	return ts.dmod_mode == DEMOD_DIGI && ts.digital_mode == DigitalMode_RTTY;
}

inline bool is_demod_psk()
{
	return ts.dmod_mode == DEMOD_DIGI && ts.digital_mode == DigitalMode_BPSK;
}

inline void RadioManagement_TxRxSwitching_Disable()
{
    ts.txrx_switching_enabled = false;
}

inline void RadioManagement_TxRxSwitching_Enable()
{
    ts.txrx_switching_enabled = true;
}

inline bool RadioManagement_TxRxSwitching_IsEnabled()
{
    return ts.txrx_switching_enabled;
}

#endif /* DRIVERS_UI_RADIO_MANAGEMENT_H_ */
