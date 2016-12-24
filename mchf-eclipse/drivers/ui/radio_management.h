/*
 * radio_management.h
 *
 *  Created on: 23.12.2016
 *      Author: danilo
 */

#ifndef DRIVERS_UI_RADIO_MANAGEMENT_H_
#define DRIVERS_UI_RADIO_MANAGEMENT_H_

#include "mchf_board.h"
// Frequency public structure
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

    // Virtual segments
    uint8_t dial_digits[9];
    // Second display
    uint8_t sdial_digits[9];

} DialFrequency;

// ------------------------------------------------
// Frequency public
extern __IO DialFrequency               df;


// LO temperature compensation
typedef struct LoTcxo
{
    ulong   skip;

    // Current compensation value
    // loaded to LO
    int     comp;

    int32_t temp;

    bool    sensor_absent;
    bool    lo_error;
    int   last;

} LoTcxo;


// ------------------------------------------------
// LO Tcxo
extern __IO LoTcxo                     lo;


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
    uint8_t default_pf;
    uint32_t tune;
    uint32_t size;
    const char* name;
} BandInfo;

extern const BandInfo bandInfo[MAX_BAND_NUM];
// FIXME: for technical reasons defined in ui_menu.c


// SWR and RF power meter public
typedef struct SWRMeter
{
    ulong   skip;

    float fwd_calc;         // forward power readings in A/D units
    float rev_calc;         // reverse power readings in A/D units
    float fwd_pwr;          // forward power in watts
    float rev_pwr;          // reverse power in watts
    float fwd_dbm;          // forward power in dBm
    float rev_dbm;          // reverse power in dBm
    float vswr;         // vswr
    float vswr_dampened;        // dampened VSWR reading
    bool  pwr_meter_disp;       // TRUE if numerical FWD/REV power metering (in milliwatts) is to be displayed
    bool  pwr_meter_was_disp;   // TRUE if numerical FWD/REV power metering WAS displayed (used to clear it)
    uchar   p_curr;         // count used to update power meter
    uchar   sensor_null;        // used to null out the sensor offset voltage

    uint8_t coupling_calc[COUPLING_MAX];

} SWRMeter;

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




uint8_t RadioManagement_GetBand(ulong freq);
bool RadioManagement_PowerLevelChange(uint8_t band, uint8_t power_level);
bool RadioManagement_Tune(bool tune);
bool RadioManagement_UpdatePowerAndVSWR();
void    RadioManagement_SetHWFiltersForFrequency(ulong freq);
void RadioManagement_ChangeCodec(uint32_t codec, bool enableCodec);
bool RadioManagement_ChangeFrequency(bool force_update, uint32_t dial_freq,uint8_t txrx_mode);
bool RadioManagement_HandleLoTemperatureDrift();
void RadioManagement_HandlePttOnOff();
void RadioManagement_MuteTemporarilyRxAudio();
uint32_t RadioManagement_NextDemodMode(uint32_t loc_mode, bool alternate_mode);
Si570_ResultCodes RadioManagement_ValidateFrequencyForTX(uint32_t dial_freq);
bool RadioManagement_IsApplicableDemodMode(uint32_t demod_mode);
void RadioManagement_SwitchTxRx(uint8_t txrx_mode, bool tune_mode);
void RadioManagement_SetBandPowerFactor(uchar band);
bool RadioManagement_LSBActive(uint16_t dmod_mode);
void    RadioManagement_SetBandPowerFactor(uchar band);
void    RadioManagement_SetPaBias();
bool    RadioManagement_CalculateCWSidebandMode(void);
void RadioManagement_SetDemodMode(uint32_t new_mode);





#endif /* DRIVERS_UI_RADIO_MANAGEMENT_H_ */
