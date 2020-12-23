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

// Common
#include <assert.h>
#include "radio_management.h"
#include "profiling.h"
#include "adc.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <soft_tcxo.h>

#include "uhsdr_hw_i2c.h"

#include "freedv_uhsdr.h"
// SI570 control
#include "osc_interface.h"
#include "codec.h"
#include "audio_driver.h"
#include "audio_management.h"

#include "cat_driver.h"
#include "ui_spectrum.h"


#include "ui_configuration.h"
#include "ui_menu.h"  // for CONFIG_160M_FULL_POWER_ADJUST and Co.
#include "ui_driver.h" // for SWR_MIN_CALC_POWER

#include "config_storage.h"

#include "cw_gen.h"

#include "cw_decoder.h"

#include "psk.h"
#include "rtty.h"
#include "uhsdr_digi_buffer.h"

#include "audio_nr.h"

#define SWR_SAMPLES_SKP             1   //5000
#define SWR_SAMPLES_CNT             5//10
#define SWR_ADC_FULL_SCALE          4095    // full scale of A/D converter (4095 = 10 bits)
#define SWR_ADC_VOLT_REFERENCE          3.3     // NOMINAL A/D reference voltage.  The PRECISE value is calibrated by a menu item!  (Probably "FWD/REV ADC Cal.")
//
// coefficients for very low power (<75 milliwatt) power levels.  Do NOT use this above approx. 0.07 volts input!
//
#define LOW_RF_PWR_COEFF_A          -0.0338205168744131     // constant (offset)
#define LOW_RF_PWR_COEFF_B          5.02584652062682        // "b" coefficient (for x)
#define LOW_RF_PWR_COEFF_C          -106.610490958242       // "c" coefficient (for x^2)
#define LOW_RF_PWR_COEFF_D          853.156505329744        // "d" coefficient (for x^3)
//
// coefficients for higher power levels (>50 milliwatts).  This is actually good down to 25 milliwatts or so.
//
#define HIGH_RF_PWR_COEFF_A         0.01209 //0.0120972709513557        // constant (offset)
#define HIGH_RF_PWR_COEFF_B         0.8334  //0.833438917330908     // "b" coefficient (for x)
#define HIGH_RF_PWR_COEFF_C             1.569   //1.56930042559198      // "c" coefficient (for x^2)

#define LOW_POWER_CALC_THRESHOLD        0.05    // voltage from sensor below which we use the "low power" calculations, above


// SWR/Power meter
SWRMeter                    swrm;

// ------------------------------------------------
// Frequency public
DialFrequency               df;

//
// Bands definition
//
//
// We first define all individual band definitions
static const BandInfo bi_2200m_all =   { .tune = 135700,       .size = 2100,       .name = "2200m", BAND_MODE_2200};
static const BandInfo bi_630m_all =    { .tune = 472000,       .size = 7000,       .name = "630m",  BAND_MODE_630};
static const BandInfo bi_160m_all =    { .tune = 1810000,      .size = 190000,     .name = "160m",  BAND_MODE_160};
static const BandInfo bi_80m_r1 =      { .tune = 3500000,      .size = 300000,     .name = "80m",   BAND_MODE_80};
static const BandInfo bi_80m_r2 =      { .tune = 3500000,      .size = 500000,     .name = "80m",   BAND_MODE_80};
static const BandInfo bi_80m_r3 =      { .tune = 3500000,      .size = 400000,     .name = "80m",   BAND_MODE_80};
static const BandInfo bi_60m_gen =     { .tune = 5250000,      .size = 200000,     .name = "60m",   BAND_MODE_60};
static const BandInfo bi_40m_r1 =      { .tune = 7000000,      .size = 200000,     .name = "40m",   BAND_MODE_40};
static const BandInfo bi_40m_r2_3 =    { .tune = 7000000,      .size = 300000,     .name = "40m",   BAND_MODE_40};
static const BandInfo bi_30m_all =     { .tune = 10100000,     .size = 50000,      .name = "30m",   BAND_MODE_30};
static const BandInfo bi_20m_all =     { .tune = 14000000,     .size = 350000,     .name = "20m",   BAND_MODE_20};
static const BandInfo bi_17m_all =     { .tune = 18068000,     .size = 100000,     .name = "17m",   BAND_MODE_17};
static const BandInfo bi_15m_all =     { .tune = 21000000,     .size = 450000,     .name = "15m",   BAND_MODE_15};
static const BandInfo bi_12m_all =     { .tune = 24890000,     .size = 100000,     .name = "12m",   BAND_MODE_12};
static const BandInfo bi_10m_all =     { .tune = 28000000,     .size = 1700000,    .name = "10m",   BAND_MODE_10};
static const BandInfo bi_6m_r2_3 =     { .tune = 50000000,     .size = 4000000,    .name = "6m",    BAND_MODE_6};
static const BandInfo bi_4m_gen =      { .tune = 70000000,     .size = 500000,     .name = "4m",    BAND_MODE_4};
static const BandInfo bi_2m_r1 =       { .tune = 144000000,    .size = 2000000,    .name = "2m",    BAND_MODE_2};
static const BandInfo bi_2m_r2_3 =     { .tune = 144000000,    .size = 4000000,    .name = "2m",    BAND_MODE_2};
static const BandInfo bi_70cm_r1 =     { .tune = 430000000,    .size = 10000000,   .name = "70cm",  BAND_MODE_70};
static const BandInfo bi_70cm_r2_3 =   { .tune = 430000000,    .size = 20000000,   .name = "70cm",  BAND_MODE_70};
static const BandInfo bi_23cm_all =    { .tune = 1240000000,   .size = 60000000,   .name = "23cm",  BAND_MODE_23};

static const BandInfo bi_160m_r3_thai ={ .tune = 1800000,      .size = 200000,     .name = "160m",  BAND_MODE_160};
static const BandInfo bi_80m_r3_thai = { .tune = 3500000,      .size = 100000,     .name = "80m",   BAND_MODE_80};
static const BandInfo bi_2m_r3_thai =  { .tune = 144000000,    .size = 3000000,    .name = "2m",    BAND_MODE_2};
static const BandInfo bi_60m_rx =     { .tune = 5250000,      .size = 200000,     .name = "60m",   .band_mode = BAND_MODE_60, .rx_only = true };
static const BandInfo bi_gen_all =     { .tune = 0,            .size = 0,          .name = "Gen",   BAND_MODE_GEN};

// now we combine from the above defined bands the set of bands for each region
// to be backwards compatible we provide the original set of bands which are
// the maximum band size from the 3 different IARU regions
// IMPORTANT: Right now the order of bands in the list is fixed to the order of BAND_MODE_xxx (low to high)
// TODO: Make this list ordered by frequency
static const BandInfo *bandInfo_combined[MAX_BAND_NUM] = 
{
    &bi_80m_r2,
    &bi_60m_gen,
    &bi_40m_r2_3,
    &bi_30m_all,
    &bi_20m_all,
    &bi_17m_all,
    &bi_15m_all,
    &bi_12m_all,
    &bi_10m_all,
    &bi_6m_r2_3,
    &bi_4m_gen,
    &bi_2m_r2_3,
    &bi_70cm_r2_3,
    &bi_23cm_all,
    &bi_2200m_all,
    &bi_630m_all,
    &bi_160m_all,
    &bi_gen_all,
};

static const BandInfo* bandInfo_region1[MAX_BAND_NUM] =
{
        &bi_80m_r1,
        &bi_60m_gen, // should cover all regions
        &bi_40m_r1,
        &bi_30m_all,
        &bi_20m_all,
        &bi_17m_all,
        &bi_15m_all,
        &bi_12m_all,
        &bi_10m_all,
        &bi_6m_r2_3,
        &bi_4m_gen,
        &bi_2m_r1,
        &bi_70cm_r1,
        &bi_23cm_all,
        &bi_2200m_all,
        &bi_630m_all,
        &bi_160m_all,
        &bi_gen_all,
};

static const BandInfo* bandInfo_region2[MAX_BAND_NUM] =
{
        &bi_80m_r2,
        &bi_60m_gen, // should cover all regions
        &bi_40m_r2_3,
        &bi_30m_all,
        &bi_20m_all,
        &bi_17m_all,
        &bi_15m_all,
        &bi_12m_all,
        &bi_10m_all,
        &bi_6m_r2_3,
        &bi_4m_gen,
        &bi_2m_r2_3,
        &bi_70cm_r2_3,
        &bi_23cm_all,
        &bi_2200m_all,
        &bi_630m_all,
        &bi_160m_all,
        &bi_gen_all,
};

static const BandInfo* bandInfo_region3[MAX_BAND_NUM] =
{
        &bi_80m_r3,
        &bi_60m_gen, // should cover all regions
        &bi_40m_r2_3,
        &bi_30m_all,
        &bi_20m_all,
        &bi_17m_all,
        &bi_15m_all,
        &bi_12m_all,
        &bi_10m_all,
        &bi_6m_r2_3,
        &bi_4m_gen,
        &bi_2m_r2_3,
        &bi_70cm_r2_3,
        &bi_23cm_all,
        &bi_2200m_all,
        &bi_630m_all,
        &bi_160m_all,
        &bi_gen_all,
};

static const BandInfo* bandInfo_region3_thai[MAX_BAND_NUM] =
{
        &bi_80m_r3_thai,
        &bi_60m_rx, // should cover all regions
        &bi_40m_r1,
        &bi_30m_all,
        &bi_20m_all,
        &bi_17m_all,
        &bi_15m_all,
        &bi_12m_all,
        &bi_10m_all,
        &bi_6m_r2_3,
        &bi_4m_gen,
        &bi_2m_r3_thai,
        &bi_70cm_r2_3,
        &bi_23cm_all,
        &bi_2200m_all,
        &bi_630m_all,
        &bi_160m_r3_thai,
        &bi_gen_all,
};

// finally we list all of them in a table and give them names for the menu
const BandInfoSet bandInfos[] =
{
        { bandInfo_combined, "R1+2+3" },
        { bandInfo_region1,  "Region 1" },
        { bandInfo_region2,  "Region 2" },
        { bandInfo_region3,  "Region 3" },
        { bandInfo_region3_thai,  "Thailand" },
};

const int BAND_INFO_SET_NUM = sizeof(bandInfos)/sizeof(BandInfoSet);

BandInfo_c **bandInfo = bandInfo_combined;

uint8_t bandinfo_idx; // default init with 0 is fine

/**
 * Searches the band info for a given band memory index
 * This relieves us from ordering the vfo band memories exactly like the
 * band infos.
 *
 * @param new_band_index
 * @return the band with the provided index or if this is not found, the current bandInfo
 */
const BandInfo* RadioManagement_GetBandInfo(uint8_t new_band_index)
{
    const BandInfo* bi = ts.band;

    for (int idx = 0; idx < MAX_BANDS; idx ++)
    {
        if (bandInfo[idx]->band_mode == new_band_index)
        {
            bi = bandInfo[idx];
            break;
        }
    }
    return bi;
}

// this structure MUST match the order of entries in power_level_t !
static const power_level_desc_t mchf_rf_power_levels[] =
{
        { .id = PA_LEVEL_FULL,   .mW = 0,    }, // we use 0 to indicate max power
        { .id = PA_LEVEL_HIGH,   .mW = 5000, },
        { .id = PA_LEVEL_MEDIUM, .mW = 2000, },
        { .id = PA_LEVEL_LOW,    .mW = 1000, },
        { .id = PA_LEVEL_MINIMAL,.mW =  500, },
};


const pa_power_levels_info_t mchf_power_levelsInfo =
{
        .levels = mchf_rf_power_levels,
        .count = sizeof(mchf_rf_power_levels)/sizeof(*mchf_rf_power_levels),
};

#ifdef RF_BRD_MCHF
const pa_info_t mchf_pa =
{
        .name  = "mcHF PA",
        .reference_power = 5000.0,
        .max_freq = 32000000,
        .min_freq =  1800000,
        .max_am_power = 2000,
        .max_power = 10000,
};
#endif  // RF_BRD_MCHF


#ifdef RF_BRD_LAPWING
const pa_info_t mchf_pa =
{
        .name  = "Lapwing PA",
        .reference_power = 5000.0,
        .max_freq = 1300 * 1000000,
        .min_freq = 1240 * 1000000,
        .max_am_power = 2000,
        .max_power = 20000,
};
#endif // LAPWING



// The following descriptor table has to be in the order of the enum digital_modes_t in  radio_management.h
// This table is stored in flash (due to const) and cannot be written to
// for operational data per mode [r/w], use a different table with order of modes
const digital_mode_desc_t digimodes[DigitalMode_Num_Modes] =
{
    { "DIGITAL" , true },
#ifdef USE_FREEDV
    { "FreeDV"  , true },
#endif
    { "RTTY"    , true },
    { "BPSK"    , true },
};

static void RadioManagement_SetCouplingForFrequency(uint32_t freq);
static void RadioManagement_SetHWFiltersForFrequency(uint32_t freq);


/**
 * @brief returns the "real" frequency translation mode for a given transceiver state. This may differ from the configured one due to modulation demands
 *
 */
uint32_t RadioManagement_GetRealFreqTranslationMode(uint32_t txrx_mode, uint32_t dmod_mode, uint32_t iq_freq_mode)
{
    uint32_t retval = iq_freq_mode;
    if (dmod_mode == DEMOD_CW && txrx_mode == TRX_MODE_TX)
    {
        retval = FREQ_IQ_CONV_MODE_OFF;
    }
    return retval;
}


/**
 * @brief permits to dis/enable a digital codec (or get back to analog)
 */
void RadioManagement_ChangeCodec(uint32_t codec, bool enableCodec)
{
    // codec == 0 -> Analog Sound
    // all other codecs -> digital codec
    if (codec == 0)
    {
        ts.dvmode = false;
    }
    else
    {
        ts.dvmode = enableCodec;
    }
    ts.digital_mode = codec;
}







/**
 * Returns the scaling which needs to be applied to the standard signal level (which delivers the PA_REFERENCE_POWER)
 * in order to output  the request power.
 * @param powerMw requested power in mW. mW =< 0.0 returns scale 1
 * @return scaling (gain)
 */
float32_t RadioManagement_CalculatePowerFactorScale(float32_t powerMw)
{
    float32_t retval = 1.0;
    if (powerMw > 0)
    {
        retval = sqrtf(powerMw / mchf_pa.reference_power);
    }
    return retval;
}

/**
 *
 * Depends on globals: ts.pwr_adj, ts.power_level, df.tune_old, ts.flags2 & FLAGS2_LOW_BAND_BIAS_REDUCE
 * Impacts globals:    ts.tx_power_factor
 * @param band
 * @return true if the power factor value differs from previous
 */
static bool RadioManagement_SetBandPowerFactor(const BandInfo* band, int32_t power)
{
    float32_t   pf_bandvalue;    // used as a holder for percentage of power output scaling

    // FIXME: This code needs fixing, the hack for TX Outside should at least reduce power factor for lower bands
    if (RadioManagement_IsGenericBand(band)) // we are outside a TX band
    {
        // TX outside bands **very dirty hack**
        //  FIXME: calculate based on 2 frequency points close the selected frequency, should be inter-/extrapolated
        uint32_t freq_min = RadioManagement_GetBandInfo(BAND_MODE_80)->tune;
        float32_t adj_min = ts.pwr_adj[ADJ_REF_PWR][BAND_MODE_80] / (RadioManagement_IsPowerFactorReduce(freq_min)? 400.0: 100.0);

        uint32_t freq_max = RadioManagement_GetBandInfo(BAND_MODE_10)->tune;
        float32_t adj_max = ts.pwr_adj[ADJ_REF_PWR][BAND_MODE_10] / (RadioManagement_IsPowerFactorReduce(freq_max)? 400.0: 100.0);

        float32_t delta_f = (float32_t)df.tune_old - (float32_t)freq_min; // we must convert to a signed type
        float32_t delta_points = freq_max - freq_min;

        float32_t freq_mult = delta_f / delta_points;

        pf_bandvalue =  freq_mult * (adj_max - adj_min) + adj_min;
    }
    else
    {
        pf_bandvalue = ts.pwr_adj[power == 0?ADJ_FULL_POWER:ADJ_REF_PWR][band->band_mode];
        pf_bandvalue /= RadioManagement_IsPowerFactorReduce(df.tune_old)? 400: 100;
    }

    float32_t power_factor_scale = 1.0 ;
    // now rescale to power levels below reference power (i.e for mcHF <5 watts) if necessary.
    if (power != 0)
    {
        power_factor_scale = RadioManagement_CalculatePowerFactorScale(power);
    }

    float32_t power_factor = pf_bandvalue * power_factor_scale;

    // limit hard limit for power factor since it otherwise may overdrive the PA section

    const float32_t old_pf = ts.tx_power_factor;

    ts.tx_power_factor =
            (power_factor > TX_POWER_FACTOR_MAX_INTERNAL) ?
            TX_POWER_FACTOR_MAX_INTERNAL : power_factor;

    ts.power_modified |=  (power_factor == 0 || ts.tx_power_factor != power_factor);

    return ts.tx_power_factor != old_pf;
}

/**
 * Is the currently active modulation / transceiver mode able to generate a side tone during transmit
 * Can be called during RX and TX
 *
 * @return true if the selected modulation mode permits the use of a side tone
 */
bool RadioManagement_UsesTxSidetone()
{
    return ts.dmod_mode == DEMOD_CW || is_demod_rtty() || is_demod_psk() || (ts.tune && !ts.iq_freq_mode);
}

/**
 * @brief API Function, implements application logic for changing the power level including filter changes
 *
 *
 * @param power_level The requested power level (as PA_LEVEL constants). Invalid values are not accepted
 * @returns true if power level has been changed, false otherwise
 */
bool RadioManagement_SetPowerLevel(const BandInfo* band, power_level_t power_level)
{
    bool retval = false;
    bool power_modified = false;

    int32_t power = power_level < mchf_power_levelsInfo.count ? mchf_power_levelsInfo.levels[power_level].mW : -1;

    if (power != -1 && band != NULL)
    {
        if (RadioManagement_IsGenericBand(band))
        {
            if(ts.flags1 & FLAGS1_TX_OUTSIDE_BANDS)
            {
                power = 50; // ~50 mW limit;
                power_modified = true;
                // I never will use this function (DF8OE)
            }
            else
            {
                power = 5; // 5mW, use very low value in case of wrong call to this function
                power_modified = true;
            }
        }

        if(ts.dmod_mode == DEMOD_AM)                // in AM mode?
        {
            if(power > mchf_pa.max_am_power || power == 0)     // yes, power over am limits?
            {
                power = mchf_pa.max_am_power;  // force to keep am limits
                power_modified = true;
            }
        }
        else if(power > mchf_pa.reference_power)
        {
            power = 0; //  0 == full power
            power_level = PA_LEVEL_FULL;
        }


        // Calculate TX power factor - see if power changed
        bool pf_change = RadioManagement_SetBandPowerFactor(band, power);

        if (pf_change == true || power != ts.power || ts.power_level != power_level || ts.power_modified != power_modified)
        {
            retval = true;
            ts.power_level = power_level;
            ts.power = power;
            ts.power_modified = power_modified;

            if (RadioManagement_UsesTxSidetone())
            {
                Codec_TxSidetoneSetgain(ts.txrx_mode);
            }
        }
    }

    return retval;
}

bool RadioManagement_Tune(bool tune)
{
    bool retval = tune;
    if(RadioManagement_IsTxDisabled() == false &&  (ts.dmod_mode != DEMOD_SAM))
    {
        if(tune)
        {
            if(ts.tune_power_level != PA_LEVEL_TUNE_KEEP_CURRENT)
            {
                ts.power_temp  = ts.power_level;             //store tx level and set tune level
                ts.power_level = ts.tune_power_level;
            }

            RadioManagement_SwitchTxRx(TRX_MODE_TX,true);
            // tune ON, this will also update the power to use our
            // to our temporarily changed level

            retval = (ts.txrx_mode == TRX_MODE_TX);
        }
        else
        {
            RadioManagement_SwitchTxRx(TRX_MODE_RX,true);                // tune OFF
            if(ts.tune_power_level != PA_LEVEL_TUNE_KEEP_CURRENT)
            {
                RadioManagement_SetPowerLevel(RadioManagement_GetBand(df.tune_new), ts.power_temp);
            }

            retval = (ts.txrx_mode == TRX_MODE_TX); // no longer tuning
        }
    }
    else
    {
        retval = false;    // no TUNE mode in AM or FM or with disabled TX!
    }
    return retval;
}

/**
 * @returns offset of tuned frequency to dial frequency in CW mode, in Hz
 */
int32_t RadioManagement_GetCWDialOffset()
{

    int32_t retval = 0;

    switch(ts.cw_offset_mode)
    {
    case CW_OFFSET_USB_SHIFT:    // Yes - USB?
    case CW_OFFSET_LSB_SHIFT:   // LSB?
    case CW_OFFSET_AUTO_SHIFT:  // Auto mode?  Check flag
        if(ts.cw_lsb)
        {
            retval += ts.cw_sidetone_freq;          // it was LSB - raise by sidetone amount
        }
        else
        {
            retval -= ts.cw_sidetone_freq;          // it was USB - lower by sidetone amount
        }
    }

    return retval;
}

/**
 * @brief returns true if the mode parameters tell us we will TX at zero if as opposed to offset frequency
 */
bool RadioManagement_IsTxAtZeroIF(uint8_t dmod_mode, uint8_t digital_mode)
{
    return  (
                dmod_mode == DEMOD_CW ||
                (dmod_mode == DEMOD_DIGI &&
                    (
#ifdef USE_FREEDV
                            digital_mode == DigitalMode_FreeDV
#else
                            false
#endif
                            || is_demod_psk()
#ifdef USE_RTTY_PROCESSOR
                            || is_demod_rtty()
#endif
                    )
                )
             );
}
uint32_t RadioManagement_Dial2TuneFrequency(const uint32_t dial_freq, uint8_t txrx_mode)
{
    uint32_t tune_freq = dial_freq;

    //
    // Do "Icom" style frequency offset of the LO if in "CW OFFSET" mode.  (Display freq. is also offset!)
    if(ts.dmod_mode == DEMOD_CW)            // In CW mode?
    {
        tune_freq += RadioManagement_GetCWDialOffset();
    }


    // Offset dial frequency if the RX/TX frequency translation is active and we are not transmitting in mode which
    // does not use frequency translation, this permits to use the generated I or Q channel as audio sidetone
    // that is right now RTTY, FreeDV, CW, BPSK
    if(txrx_mode != TRX_MODE_TX || RadioManagement_IsTxAtZeroIF(ts.dmod_mode, ts.digital_mode) == false)
    {
        tune_freq += AudioDriver_GetTranslateFreq();
    }

    // Extra tuning actions
    if(txrx_mode == TRX_MODE_RX)
    {
        tune_freq += (ts.rit_value*20); // Add RIT on receive
    }

    return tune_freq;
}

/**
 * @brief switch off the PA Bias to mute HF output ( even if PTT is on )
 * Using this method the PA will be effectively muted no matter what setting
 * the main bias switch has (which directly connected to the PTT HW Signal)
 * Used to suppress signal path reconfiguration noise during rx/tx and tx/rx switching
 */
void RadioManagement_DisablePaBias()
{
    Board_SetPaBiasValue(0);
}

/**
 * @brief recalculate and set the PA Bias according to requested value
 *
 * Please note that at the mcHF BIAS is only applied if the PTT HW Signal
 * is active (which is controlled using MchfBoard_EnableTXSignalPath())
 */
void RadioManagement_SetPaBias()
{

    uint32_t   calc_var;

    if((ts.pa_cw_bias) && (ts.dmod_mode == DEMOD_CW))       // is CW PA bias non-zero AND are we in CW mode?
    {
        calc_var = ts.pa_cw_bias;       // use special CW-mode bias setting
    }
    else
    {
        calc_var = ts.pa_bias;      // use "default" bias setting
    }
    calc_var = calc_var *2 + BIAS_OFFSET;

    if(calc_var > 255)
    {
        calc_var = 255;
    }
    Board_SetPaBiasValue(calc_var);
}


bool RadioManagement_ChangeFrequency(bool force_update, uint32_t dial_freq,uint8_t txrx_mode)
{
    // everything else uses main VFO frequency
    // uint32_t    tune_freq;
    bool lo_change_pending = false;

    // Calculate actual tune frequency
    ts.tune_freq_req = RadioManagement_Dial2TuneFrequency(dial_freq, txrx_mode);

    if((ts.tune_freq != ts.tune_freq_req) || df.temp_factor_changed || force_update )  // did the frequency NOT change and display refresh NOT requested??
    {

        if(ts.sysclock-ts.last_tuning > 5 || ts.last_tuning == 0)     // prevention for SI570 crash due too fast frequency changes
        {
            Oscillator_ResultCodes_t lo_prep_result = osc->prepareNextFrequency(ts.tune_freq_req, df.temp_factor);
            // first check and mute output if a large step is to be done
            if(osc->isNextStepLarge() == true)     // did the tuning require that a large tuning step occur?
            {
                // 18 is a little more than 10ms (15 ==10ms) which is max for the Si570 to change the frequency
                if (ts.audio_dac_muting_buffer_count < 18)
                {
                    ts.audio_dac_muting_buffer_count = 18;
                }
                if (ts.audio_processor_input_mute_counter < 18)
                {
                    ts.audio_processor_input_mute_counter = 18;
                }
            }

            ts.last_tuning = ts.sysclock;


            ts.last_lo_result  = lo_prep_result;

            Oscillator_ResultCodes_t lo_exec_result = OSC_TUNE_IMPOSSIBLE;
            if (lo_prep_result != OSC_TUNE_IMPOSSIBLE)
            {
                // Set frequency
                lo_exec_result = osc->changeToNextFrequency();
            }

            // if i2c error or verify error, there is a chance that we can fix that, so we mark this
            // as NOT executed, in all other cases we assume the change has happened (but may prevent TX)
            if (lo_exec_result != OSC_COMM_ERROR && lo_exec_result != OSC_ERROR_VERIFY)
            {
                df.temp_factor_changed = false;
                ts.tune_freq = ts.tune_freq_req;        // frequency change required - update change detector
                // Save current freq
                df.tune_old = dial_freq;
            }
            else
            {
                ts.last_lo_result = lo_exec_result;
            }

            if (ts.last_lo_result == OSC_OK || ts.last_lo_result == OSC_TUNE_LIMITED)
            {

                ts.tx_disable &= ~TX_DISABLE_OUTOFRANGE;
            }
            else
            {
                ts.tx_disable |= TX_DISABLE_OUTOFRANGE;
            }

            uint32_t tune_freq_real = ts.tune_freq;

            RadioManagement_SetCouplingForFrequency(tune_freq_real);    // adjust wattmeter coupling factor
            RadioManagement_SetHWFiltersForFrequency(tune_freq_real);  // check the filter status with the new frequency update
            AudioManagement_CalcIqPhaseGainAdjust(tune_freq_real);

            // Inform Spectrum Display code that a frequency change has happened
            ts.dial_moved = 1;
        }
        else
        {
            lo_change_pending = true;
        }

    }

    // successfully executed the change
    return lo_change_pending == false;
}

/**
 * @brief temporary muting of the receiver when making changes which may cause audible pops etc., unmuting happens some 10s of milliseconds automatically later
 *
 */
void RadioManagement_MuteTemporarilyRxAudio()
{
    // save us from the loud "POP" that will occur when we change bands
    ts.audio_processor_input_mute_counter = 5 * 15;
    ts.audio_dac_muting_buffer_count = 13 * 15;
}

Oscillator_ResultCodes_t RadioManagement_ValidateFrequencyForTX(uint32_t dial_freq)
{
    // get bandinfo to check if this is rx_only
    const BandInfo* bi = RadioManagement_GetBand(dial_freq);

    // we also check if our PA is able to support this frequency
    bool pa_ok = dial_freq >= mchf_pa.min_freq && dial_freq <= mchf_pa.max_freq;

    Oscillator_ResultCodes_t retval = (pa_ok && bi != NULL && bi->rx_only == false)? OSC_OK : OSC_TUNE_IMPOSSIBLE;

    if (retval == OSC_OK)
    {
        // we check with the si570 code if the frequency is tunable, we do not tune to it.
        retval = osc->prepareNextFrequency(RadioManagement_Dial2TuneFrequency(dial_freq, TRX_MODE_TX), df.temp_factor);
    }

    return retval;
}

/**
 * @brief returns the current LO Tune Frequency for TX
 * @returns LO Frequency in Hz
 */
uint32_t RadioManagement_GetTXDialFrequency()
{
    uint32_t retval;
    if (ts.txrx_mode != TRX_MODE_TX)
    {
        if(is_splitmode())                  // is SPLIT mode active and?
        {
            uint8_t vfo_tx;
            if (is_vfo_b())
            {
                vfo_tx = VFO_A;
            }
            else
            {
                vfo_tx = VFO_B;
            }
            retval = vfo[vfo_tx].band[ts.band->band_mode].dial_value;    // load with VFO-A frequency
        }
        else
        {
            retval = df.tune_new;
        }
    }
    else
    {
        retval = df.tune_new;
    }
    return retval;
}
/**
 * @brief returns the current LO Dial Frequency for RX
 * @returns LO Frequency in Hz
 */
uint32_t RadioManagement_GetRXDialFrequency()
{
    uint32_t baseval;
    if (ts.txrx_mode != TRX_MODE_RX)
    {
        if(is_splitmode())                  // is SPLIT mode active?
        {
            uint8_t vfo_rx;
            if (is_vfo_b())
            {
                vfo_rx = VFO_B;
            }
            else
            {
                vfo_rx = VFO_A;
            }
            baseval = vfo[vfo_rx].band[ts.band->band_mode].dial_value;    // load with VFO-A frequency
        }
        else
        {
            baseval = df.tune_new;
        }
    }
    else
    {
        baseval = df.tune_new;
    }

    return baseval + (ts.rit_value*20);
}

// globals used:
// ts.trx_mode -> R / W
// ts.band -> R
// vfo[] -> r / w
// df.tune_new -> r
// ts.dmod_mode -> r
// ts.cw_text_entry -> r
// functions called:
// RadioManagement_ValidateFrequencyForTX
// RadioManagement_IsTxDisabled()
// ts.audio_dac_muting_buffer_count -> r/w
// ts.audio_dac_muting_flag = true; // let the audio being muted initially as long as we need it
// ads.agc_holder -> w
// ads.agc_val -> r
// RadioManagement_DisablePaBias(); // kill bias to mute the HF output quickly
// Board_RedLed(LED_STATE_ON); // TX
// Board_GreenLed(LED_STATE_OFF);
// Board_EnableTXSignalPath(true); // switch antenna to output and codec output to QSE mixer
// RadioManagement_ChangeFrequency(false,df.tune_new, txrx_mode_final);
// uint8_t tx_band = RadioManagement_GetBand(tune_new);
// RadioManagement_PowerLevelChange(tx_band,ts.power_level);
// RadioManagement_SetBandPowerFactor(tx_band);
// AudioManagement_SetSidetoneForDemodMode(ts.dmod_mode,txrx_mode_final == TRX_MODE_RX?false:tune_mode);
// Codec_SwitchTxRxMode(txrx_mode_final);
// RadioManagement_SetPaBias();
// ts.txrx_switch_audio_muting_timing -> r;
// ts.audio_processor_input_mute_counter -> w;
// ts.audio_dac_muting_buffer_count -> w
// ts.audio_dac_muting_flag -> w;
// ts.tx_audio_source -> r
// ts.power_level -> r

/**
 * @brief check if all resources are available to switch tx/rx mode in an interrupt
 * @return true if all resources are available to switch tx/rx mode in an interrupt
 */
static __IO bool  radioManagement_SwitchTxRx_running;

/**
 * This function should only return true if it is absolutely safe to switch between Tx and Rx in an interrupt. Better safe than sorry.
 * @return true if we can switch because no code is running in a critical section
 */
bool RadioManagement_SwitchTxRx_Possible()
{
    // we check all resources which may be locked by a user mode (non-interrupt activity)
    return RadioManagement_TxRxSwitching_IsEnabled() && osc->readyForIrqCall() && Codec_ReadyForIrqCall() && radioManagement_SwitchTxRx_running == false;
}

void RadioManagement_SwitchTxRx(uint8_t txrx_mode, bool tune_mode)
{
    radioManagement_SwitchTxRx_running = true;
    uint32_t tune_new;
    bool tx_ok = false;
    bool tx_pa_disabled = false;

    // ts.last_tuning = 0;                  // prevents transmitting on wrong frequency during "RX bk phases"

    if(is_splitmode())                  // is SPLIT mode active?
    {
        uint8_t vfo_tx,vfo_rx;
        if (is_vfo_b())
        {
            vfo_rx = VFO_B;
            vfo_tx = VFO_A;
        }
        else
        {
            vfo_rx = VFO_A;
            vfo_tx = VFO_B;
        }
        if(txrx_mode == TRX_MODE_TX)     // are we in TX mode?
        {
            if(ts.txrx_mode == TRX_MODE_RX)                         // did we want to enter TX mode?
            {
                vfo[vfo_rx].band[ts.band->band_mode].dial_value = df.tune_new; // yes - save current RX frequency in RX VFO location
            }
            tune_new = vfo[vfo_tx].band[ts.band->band_mode].dial_value;    // load with TX VFO frequency
        }
        else                    // we are in RX mode
        {
            tune_new = vfo[vfo_rx].band[ts.band->band_mode].dial_value;    // load with RX VFO frequency
        }
    }
    else
    {
        // we just take the current one if not in split mode
        tune_new = df.tune_new;
    }

    if(txrx_mode == TRX_MODE_TX)
    {
        // FIXME: Not very robust code, make sure Validate always returns TUNE_IMPOSSIBLE in case of issues
        tx_ok = RadioManagement_ValidateFrequencyForTX(tune_new) != OSC_TUNE_IMPOSSIBLE;


        // this code handles the ts.tx_disable
        // even if ts.tx_disble is set in CW and only in CW we still switch to TX
        // but leave the PA disabled. This is for support of CW training right with the mcHF.
        if (RadioManagement_IsTxDisabled() || ts.cw_text_entry)
        {
            if ((tx_ok == true && ts.dmod_mode == DEMOD_CW) || ts.cw_text_entry)
            {
                tx_pa_disabled = true;
            }
            else
            {
                // in any other case, it is not okay to transmit with ts.tx_disable == true
                tx_ok = false;
            }
        }

        if (is_demod_psk())
        {
        	Psk_Modulator_PrepareTx();
        }
    }

    uint8_t txrx_mode_final = tx_ok?txrx_mode:TRX_MODE_RX;

    // only switch mode if tx was permitted or rx was requested
    if (txrx_mode_final != ts.txrx_mode || txrx_mode_final == TRX_MODE_RX)
    {
        // there is in fact a switch happening
        // which may cause audio issues
        if (txrx_mode_final != ts.txrx_mode)
        {

            ts.audio_dac_muting_buffer_count = 2; // wait at least 2 buffer cycles
            ts.audio_dac_muting_flag = true; // let the audio being muted initially as long as we need it
            RadioManagement_DisablePaBias(); // kill bias to mute the HF output quickly
        }

        if(txrx_mode_final == TRX_MODE_TX)
        {


            // We mute the audio BEFORE we activate the PTT.
            // This is necessary since U3 is switched the instant that we do so,
            // rerouting audio paths and causing all sorts of disruption including CLICKs and squeaks.
            Codec_PrepareTx(ts.txrx_mode); // 5ms

            while (ts.audio_dac_muting_buffer_count >0)
            {
                // TODO: Find a better solution here
                asm("nop"); // just wait a little for the silence to come out of the audio path
                // this can take up to 1.2ms (time for processing two audio buffer dma requests
            }

            // this is here to allow CW training
            // with ts.tx_disabled on nothing will be transmitted but you can hear the sidetone
            if (tx_pa_disabled == false)
            {
                Board_RedLed(LED_STATE_ON); // TX
                Board_GreenLed(LED_STATE_OFF);
                Board_EnableTXSignalPath(true); // switch antenna to output and codec output to QSE mixer
            }
        }

        df.tune_new = tune_new;
        RadioManagement_ChangeFrequency(false,df.tune_new, txrx_mode_final);
        // ts.audio_dac_muting_flag = true; // let the audio being muted initially as long as we need it

        // there might have been a band change between the modes, make sure to have the power settings fitting the mode
        if (txrx_mode_final == TRX_MODE_TX)
        {
            RadioManagement_SetPowerLevel(RadioManagement_GetBand(tune_new),ts.power_level);
        }

        AudioManagement_SetSidetoneForDemodMode(ts.dmod_mode,txrx_mode_final == TRX_MODE_RX?false:tune_mode);
        // make sure the audio is set properly according to txrx and tune modes

        if (txrx_mode_final == TRX_MODE_RX)
        {

            while (ts.audio_dac_muting_buffer_count >0)
            {
                // TODO: Find a better solution here
                asm("nop"); // just wait a little for the silence to come out of the audio path
                // this can take up to 1.2ms (time for processing two audio buffer dma requests
            }

            Board_EnableTXSignalPath(false); // switch antenna to input and codec output to lineout
            Board_RedLed(LED_STATE_OFF);      // TX led off
            Board_GreenLed(LED_STATE_ON);      // TX led off
            ts.audio_dac_muting_flag = false; // unmute audio output
            //CwGen_PrepareTx(); // make sure the keyer is set correctly for next round
            // commented out as resetting now part of cw_gen state machine
        }

        if (ts.txrx_mode != txrx_mode_final)
        {
            Codec_SwitchTxRxMode(txrx_mode_final);

            if (txrx_mode_final == TRX_MODE_TX)
            {
                RadioManagement_SetPaBias();
                uint32_t input_mute_time = 0, dac_mute_time = 0, dac_mute_time_mode = 0, input_mute_time_mode = 0; // aka 1.3ms
                // calculate expire time for audio muting in interrupts, it is 15 interrupts per 10ms
                dac_mute_time = ts.txrx_switch_audio_muting_timing * 15;

                if (ts.dmod_mode != DEMOD_CW)
                {
                    switch(ts.tx_audio_source)
                    {

                    case TX_AUDIO_DIG:
                        dac_mute_time_mode = 0* 15; // Minimum time is 0ms
                        break;
                    case TX_AUDIO_LINEIN_L:
                    case TX_AUDIO_LINEIN_R:
                        dac_mute_time_mode = 4 * 15; // Minimum time is 40ms
                        input_mute_time_mode = dac_mute_time_mode;
                        break;
                    case TX_AUDIO_MIC:
                        dac_mute_time_mode = 4* 15; // Minimum time is 40ms
                        input_mute_time_mode = dac_mute_time_mode;
                        break;
                    }
                }

                dac_mute_time = (dac_mute_time > dac_mute_time_mode)? dac_mute_time : dac_mute_time_mode;
                input_mute_time = (input_mute_time > input_mute_time_mode)? input_mute_time : input_mute_time_mode;

                ts.audio_processor_input_mute_counter = input_mute_time;
                ts.audio_dac_muting_buffer_count =   dac_mute_time; // 15 == 10ms

                ts.audio_dac_muting_flag = false; // unmute audio output unless timed muting is active
            }
            ts.txrx_mode = txrx_mode_final;
        }
    }
    radioManagement_SwitchTxRx_running = false;
}



/*
 * @returns: false -> use USB, true -> use LSB
 */
bool RadioManagement_CalculateCWSidebandMode()
{
    bool retval = false;
    switch(RadioManagement_CWConfigValueToModeEntry(ts.cw_offset_mode)->sideband_mode)
    {
    case CW_SB_AUTO:                     // For "auto" modes determine if we are above or below threshold frequency
        // if (RadioManagement_SSB_AutoSideBand(df.tune_new) == DEMOD_USB)   // is the current frequency above the USB threshold?
        retval = (df.tune_new <= USB_FREQ_THRESHOLD && RadioManagement_GetBand(df.tune_new)->band_mode != BAND_MODE_60);
        // is the current frequency below the USB threshold AND is it not 60m? -> LSB
        break;
    case CW_SB_LSB:
        retval = true;
        break;
    case CW_SB_USB:
    default:
        retval = false;
        break;
    }
    return retval;
}


typedef struct BandFilterDescriptor
{
    uint32_t upper;
    uint16_t band_mode;
} BandFilterDescriptor;

// TODO: This code below approach assumes that all filter hardware uses a set of separate filter banks
// other approaches such as configurable filters need a different approach, should be factored out
// into some hardware abstraction at some point

// The descriptor array below has to be ordered from the lowest BPF frequency filter
// to the highest.
static const BandFilterDescriptor mchf_rf_bandFilters[] =
{
    {  4000000,  0 },
    {  8000000,  1 },
    { 16000000,  2 },
    { 32000000,  3 },
};

const int BAND_FILTER_NUM = sizeof(mchf_rf_bandFilters)/sizeof(BandFilterDescriptor);


/**
 * @brief Select and activate the correct BPF for the frequency given in @p freq
 *
 *
 * @param freq The frequency to activate the BPF for in Hz
 *
 * @warning  If the frequency given in @p freq is too high for any of the filters, no filter change is executed.
 */
static void RadioManagement_SetHWFiltersForFrequency(uint32_t freq)
{
    for (int idx = 0; idx < BAND_FILTER_NUM; idx++)
    {
        if(freq < mchf_rf_bandFilters[idx].upper)       // are we low enough if frequency for this band filter?
        {
            if(ts.filter_band != mchf_rf_bandFilters[idx].band_mode)
            {
                Board_SelectLpfBpf(mchf_rf_bandFilters[idx].band_mode);
                ts.filter_band = mchf_rf_bandFilters[idx].band_mode;
                nr_params.first_time = 1; // in case of any Bandfilter change restart the NR routine
            }
            break;
        }
    }
}

typedef struct
{
    uint32_t upper;
    uint16_t coupling_band;
} CouplingDescriptor;

static const CouplingDescriptor mchf_rf_coupling[] =
{
    { 200000,  COUPLING_2200M},
    { 600000,  COUPLING_630M},
    { 2500000, COUPLING_160M},
    { 4250000, COUPLING_80M},
    { 8000000, COUPLING_40M},
    { 16000000, COUPLING_20M},
    { 32000000, COUPLING_15M},
    { 60000000, COUPLING_6M},
};

const int COUPLING_NUM = sizeof(mchf_rf_coupling)/sizeof(CouplingDescriptor);


/**
 * @brief Select and activate the correct coupling factor for the wattmeter for the frequency given in @p freq
 *
 *
 * @param freq The frequency in Hz
 *
 * @warning  If the frequency given in @p freq is too high , no change is executed.
 */
static void RadioManagement_SetCouplingForFrequency(uint32_t freq)
{
    for (int idx = 0; idx < COUPLING_NUM; idx++)
    {
        if(freq < mchf_rf_coupling[idx].upper)       // are we low enough if frequency for this coupling factor?
        {
            ts.coupling_band = mchf_rf_coupling[idx].coupling_band;
            break;
        }
    }
}

/**
 * @brief shall the stored power factor be interpreted as coarse or fine (4*resolution of coarse)
 * @param freq in Hertz
 */
bool RadioManagement_IsPowerFactorReduce(uint32_t freq)
{
    return (((freq < 8000000) && (ts.flags2 & FLAGS2_LOW_BAND_BIAS_REDUCE))
        || ((freq >= 8000000) && (ts.flags2 & FLAGS2_HIGH_BAND_BIAS_REDUCE))) != 0;

}




/*
 * @brief returns the band according to the given frequency
 * @param freq  frequency to get band for. Unit is Hertz. This value is used without any further adjustments and should be the intended RX/TX frequency and NOT the IQ center frequency
 *
 */
void RadioManagement_SetDemodMode(uint8_t new_mode)
{
    ts.dsp.inhibit++;
    ads.af_disabled++;

    if (new_mode == DEMOD_DIGI)
    {
        if (ts.digital_mode == DigitalMode_None)
        {
            ts.digital_mode = 1;
            // this maps to the first available digital mode (if any)
        }
        RadioManagement_ChangeCodec(ts.digital_mode,1);
    }
    else if (ts.dmod_mode == DEMOD_DIGI)
    {
            RadioManagement_ChangeCodec(ts.digital_mode,0);
            // FIXME:  Maybe we should better handle the
            // mode switching centrally for all modes
            AudioNr_Prepare(); // prepare AudioNr for use after FreeDV using the same buffer
    }

    if (new_mode == DEMOD_FM && ts.dmod_mode != DEMOD_FM)
    {
        // ads.fm_squelched = true;
        // ads.fm_sql_avg = 1;
    }

    if (ts.cw_offset_shift_keep_signal == true && (ts.cw_offset_mode == CW_OFFSET_AUTO_SHIFT || ts.cw_offset_mode == CW_OFFSET_LSB_SHIFT || ts.cw_offset_mode == CW_OFFSET_USB_SHIFT))
    {
        static  int16_t sidetone_mult = 0;

        if (new_mode == DEMOD_CW && ts.dmod_mode != DEMOD_CW)
        {
            // we come from a non-CW mode
            if (RadioManagement_UsesBothSidebands(ts.dmod_mode) == false)
            {
                sidetone_mult = RadioManagement_LSBActive(ts.dmod_mode) ? -1 : 1;
                // if we have a sideband mode
                // adjust dial frequency by side tone offset
                // if the sidetone frequency is not change, we return exactly to the frequency we have been before
                // this is important if we just cycle through the modes.
                df.tune_new += sidetone_mult * ts.cw_sidetone_freq;
            }
            else
            {
                sidetone_mult = 0;
            }
        }
        if (new_mode != DEMOD_CW && ts.dmod_mode == DEMOD_CW)
        {
             // we revert now our frequency change
             // we go to a non-CW mode
             // adjust dial frequency by former side tone offset
             df.tune_new -= sidetone_mult * ts.cw_sidetone_freq;
         }
    }
    AudioDriver_SetProcessingChain(new_mode, false);
    AudioManagement_SetSidetoneForDemodMode(new_mode,false);

    if (new_mode == DEMOD_SAM)
    {
        ts.tx_disable |= TX_DISABLE_RXMODE; // set  bit
    }
    else
    {
        ts.tx_disable &= ~TX_DISABLE_RXMODE; // clear bit
    }
    ts.dmod_mode = new_mode;

    if  (ads.af_disabled) { ads.af_disabled--; }
    if (ts.dsp.inhibit) { ts.dsp.inhibit--; }
    nr_params.first_time = 1; // re-initialize spectral noise reduction, when dmod_mode was changed

}

/**
 *  * Is the given frequency in the limits of a band?
 * @param bandInfo* ptr to band info for this band
 * @param freq the frequency to check
 */
bool RadioManagement_FreqIsInBand(const BandInfo* bandinfo, const uint32_t freq)
{
    assert(bandinfo != NULL);
    return (freq >= bandinfo->tune) && (freq <= (bandinfo->tune + bandinfo->size));
}

/**
 * Is the given frequency in an enabled band?
 * @param freq the frequency to check
 * @returns true if in any of the currently enabled bands
 */
bool RadioManagement_FreqIsInEnabledBand ( uint32_t freq )
{
    bool retval = false;
    for ( int idx = 0; idx < MAX_BAND_NUM; idx++ )
    {
        if ( band_enabled[idx] )
        {
            retval = true;
            RadioManagement_FreqIsInBand( bandInfo[idx], freq);
            break; // we found the first enabled band following the current one
        }
    }
    return retval;
}

/**
 * Identify ham band for a given frequency
 * @param freq in Hz
 * @return the ham band info or generic band info if not a known ham band
 */
const BandInfo* RadioManagement_GetBand(uint32_t freq)
{
    static const BandInfo* band_scan_old = &bi_gen_all;

    const BandInfo*   band_scan = &bi_gen_all;
    // generic band, which we return if we can't find a match


    // first try the previously selected band, and see if it is an match
    if (RadioManagement_FreqIsInBand(band_scan_old, freq) == true)
    {
        band_scan = band_scan_old;
    }
    else
    {
        // search trough all bands, except the generic band (which is last)
        for(int band_idx = 0; band_idx < MAX_BANDS; band_idx++)
        {
            if(RadioManagement_FreqIsInBand(bandInfo[band_idx],freq))   // Is this frequency within this band?
            {
                band_scan = bandInfo[band_idx];
                break;  // yes - stop the scan
            }
        }
    }

    band_scan_old = band_scan; // remember last result

    return band_scan;       // return with the band
}

uint32_t RadioManagement_SSB_AutoSideBand(uint32_t freq) {
    uint32_t retval;

    if((ts.lsb_usb_auto_select == AUTO_LSB_USB_60M) && ((freq < USB_FREQ_THRESHOLD) && (RadioManagement_GetBand(freq)->band_mode != BAND_MODE_60)))       // are we <10 MHz and NOT on 60 meters?
    {
        retval = DEMOD_LSB;
    }
    else if((ts.lsb_usb_auto_select == AUTO_LSB_USB_ON) && (freq < USB_FREQ_THRESHOLD))      // are we <10 MHz (not in 60 meter mode)
    {
        retval = DEMOD_LSB;
    }
    else        // we must be > 10 MHz OR on 60 meters
    {
        retval = DEMOD_USB;
    }
    return retval;
}

/**
 * Call this to switch off transmit as soon as possible
 * Please note, this is an asynchronous request, there may be a delay until TX is switched off!
 */
void RadioManagement_Request_TxOn()
{
    ts.ptt_req = true;
}

/**
 * Call this to switch off transmit as soon as possible
 * Please note, this is an asynchronous request, there may be a delay until TX is switched off!
 */
void RadioManagement_Request_TxOff()
{

    // we have to do both here to handle a potential race condition if
    // the CAT code deasserts RTS before we actually switched to TX.
    ts.ptt_req = false;
    ts.tx_stop_req = true;
}

const int32_t ptt_debounce_time = 3; // n*10ms, delay for debouncing manually started TX (using the PTT button / input line)

#define PTT_BREAK_IDLE (-1)
void RadioManagement_HandlePttOnOff()
{
    static int64_t ptt_break_timer = PTT_BREAK_IDLE;

    // not when tuning, in this case we are TXing already anyway until tune is being stopped
    if(ts.tune == false)
    {
        // we are asked to start TX
        if(ts.ptt_req)
        {
            if(ts.txrx_mode == TRX_MODE_RX && (RadioManagement_IsTxDisabled() == false || ts.dmod_mode == DEMOD_CW || ts.cw_text_entry)) // FIXME cw_text_entry situation is not correctly processed
            {
                RadioManagement_SwitchTxRx(TRX_MODE_TX,false);
            }

            // if we have a ptt request, all stop requests are cancelled
            ts.tx_stop_req = false;
            // the ptt request has been processed
            ts.ptt_req = false;
        }
        else if (CatDriver_CatPttActive() == false)
        {
            // When CAT driver "pressed" PTT skip auto return to RX
        	if (ts.tx_stop_req == true && is_demod_psk() && Psk_Modulator_GetState() == PSK_MOD_ACTIVE)
        	{
        		Psk_Modulator_SetState(PSK_MOD_POSTAMBLE);
        		ts.tx_stop_req = false;
        	}
        	else if(!(ts.dmod_mode == DEMOD_CW || is_demod_rtty() || is_demod_psk() || ts.cw_text_entry) || ts.tx_stop_req == true)
            {
        	    // we get here either if there is an explicit request to stop transmission no matter which mode we are in
        	    // or if the mode relies on us to switch off after PTT has been released (we defines this by exclusion
        	    // of modes which control transmission state via paddles or keyboard)

        	    // If we are in TX
                if(ts.txrx_mode == TRX_MODE_TX)
                {
                    // ... the PTT line is released ...
                    if(Board_PttDahLinePressed() == false)
                    {
                        // ... we start the break timer if not started already!
                        if (ptt_break_timer == PTT_BREAK_IDLE)
                        {
                            ptt_break_timer = ts.sysclock + ptt_debounce_time;
                        }
                    }
                    else
                    {
                        // ... but if not released we stop the break timer
                        ptt_break_timer = PTT_BREAK_IDLE;
                    }

                    // ... if break time is over or we are forced to leave TX immediately ...
                    if((ptt_break_timer < ts.sysclock && ptt_break_timer != PTT_BREAK_IDLE) || ts.tx_stop_req == true) {
                        // ... go back to RX and ...
                        RadioManagement_SwitchTxRx(TRX_MODE_RX,false);
                        // ... stop the timer
                        ptt_break_timer = PTT_BREAK_IDLE;
                    }
                }
                // if we are here a stop request has been processed completely
                ts.tx_stop_req = false;
            }
        }
    }
}

bool RadioManagement_IsApplicableDemodMode(uint32_t demod_mode)
{
    bool retval = false;
    switch(demod_mode)
    {
    case DEMOD_LSB:
    case DEMOD_USB:
        if((ts.lsb_usb_auto_select))       // is auto-select LSB/USB mode enabled AND mode-skip NOT enabled?
        {
            retval = RadioManagement_SSB_AutoSideBand(df.tune_new) == demod_mode;       // is this a voice mode, subject to "auto" LSB/USB select?
        }
        else
        {
            retval = true;
        }
        break;
    case DEMOD_AM:
        retval = (ts.demod_mode_disable & DEMOD_AM_DISABLE) == 0;      // is AM enabled?
        break;
    case DEMOD_DIGI:
        retval = (ts.demod_mode_disable & DEMOD_DIGI_DISABLE) == 0;      // is DIGI enabled?
        if((ts.lsb_usb_auto_select) && retval == true)       // is auto-select LSB/USB mode enabled AND mode-skip NOT enabled?
        {
            // TODO: this is only true for FreeDV, but since we have only FreeDV...
            ts.digi_lsb = RadioManagement_SSB_AutoSideBand(df.tune_new) == DEMOD_LSB;
            // is this a voice mode, subject to "auto" LSB/USB select?
        }
        break;
    case DEMOD_CW:
        retval = (ts.demod_mode_disable & DEMOD_CW_DISABLE) == 0;      // is CW enabled?
        break;
    case DEMOD_FM:
        // FIXME: ts.lsb_usb_auto_select acts as fm select here. Rename!
        retval = (ts.iq_freq_mode != FREQ_IQ_CONV_MODE_OFF) && (((ts.flags2 & FLAGS2_FM_MODE_ENABLE) != 0) || (ts.band->band_mode == BAND_MODE_10 && ts.lsb_usb_auto_select));   // is FM enabled?
        break;
    case DEMOD_SAM:
        retval =( ts.flags1 & FLAGS1_SAM_ENABLE) != 0;        // is SAM enabled?
        break;
#ifdef USE_TWO_CHANNEL_AUDIO
    case DEMOD_SSBSTEREO:
    case DEMOD_IQ:
    	if(!ts.stereo_enable)
    	{
    		retval = false;
    	}
    	else
    	{
    		retval = true;
    	}
    	break;
#endif
    default:
        retval = true;
    }
    return retval;
}

/**
 * @brief finds next related alternative modes for a given mode (e.g. for USB it returns  LSB)
 * @returns next mode OR same mode if none found.
 */
uint32_t RadioManagement_NextAlternativeDemodMode(uint32_t loc_mode)
{
    uint32_t retval = loc_mode;
    // default is to simply return the original mode
    switch(loc_mode)
    {
    case DEMOD_USB:
        retval = DEMOD_LSB;
        break;
    case DEMOD_LSB:
        retval = DEMOD_USB;
        break;
    case DEMOD_CW:
        // FIXME: get rid of ts.cw_lsb
        // better use it generally to indicate selected side band (also in SSB)
        ts.cw_lsb = !ts.cw_lsb;
        break;
    case DEMOD_AM:
        retval = DEMOD_SAM;
        break;
    case DEMOD_SAM:
        ads.sam_sideband ++;

        // stereo SAM is only switchable if you have stereo modes enabled
#ifdef USE_TWO_CHANNEL_AUDIO
        if (ts.stereo_enable == false && ads.sam_sideband == SAM_SIDEBAND_STEREO)
        {
            // skip if we have stereo disabled
            ads.sam_sideband ++;
        }
#endif
        if (ads.sam_sideband >= SAM_SIDEBAND_MAX)
        {
            ads.sam_sideband = SAM_SIDEBAND_BOTH;
            retval = DEMOD_AM;
        }
        break;
    case DEMOD_DIGI:
        ts.digi_lsb = !ts.digi_lsb;
        break;
    case DEMOD_FM:
        // toggle between narrow and wide fm
        RadioManagement_FmDevSet5khz( !RadioManagement_FmDevIs5khz() );
    }
    // if there is no explicit alternative mode
    // we return the original mode.
    return retval;
}

/**
 * @brief find the next "different" demodulation mode which is enabled.
 * @returns new mode, or current mode if no other mode is available
 */
uint32_t RadioManagement_NextNormalDemodMode(uint32_t loc_mode)
{
    uint32_t retval = loc_mode;
    // default is to simply return the original mode
    do {
        retval++;
        if (retval > DEMOD_MAX_MODE)
        {
            retval = 0;
            // wrap around;
        }
    }    while (RadioManagement_IsApplicableDemodMode(retval) == false && retval != loc_mode);
    // if we loop around to the initial mode, there is no other option than the original mode
    // so we return it, otherwise we provide the new mode.

    return retval;
}


bool RadioManagement_UsesBothSidebands(uint16_t dmod_mode)
{
    bool retval =
            (
                    (dmod_mode == DEMOD_AM)
                    ||(dmod_mode == DEMOD_SAM && (ads.sam_sideband == SAM_SIDEBAND_BOTH))
                    || (dmod_mode == DEMOD_FM)
            );


#ifdef USE_TWO_CHANNEL_AUDIO
    // if we support two channel audio, then both bands are used by some additional modes
    retval = retval ||
            (
                    (dmod_mode == DEMOD_SSBSTEREO)
                    || (dmod_mode == DEMOD_IQ)
                    || (dmod_mode == DEMOD_SAM && ads.sam_sideband == SAM_SIDEBAND_STEREO )
            );
#endif
    return retval;
}

bool RadioManagement_LSBActive(uint16_t dmod_mode)
{
    bool    is_lsb;

    switch(dmod_mode)        // determine if the receiver is set to LSB or USB or FM
    {
    case DEMOD_SAM:
        is_lsb = ads.sam_sideband == SAM_SIDEBAND_LSB;
        break;
    case DEMOD_LSB:
        is_lsb = true;      // it is LSB
        break;
    case DEMOD_CW:
        is_lsb = ts.cw_lsb; // is this USB RX mode?  (LSB of mode byte was zero)
        break;
    case DEMOD_DIGI:
        is_lsb = ts.digi_lsb;
        break;
    case DEMOD_USB:
    default:
        is_lsb = false;     // it is USB
        break;
    }

    return is_lsb;
}

bool RadioManagement_USBActive(uint16_t dmod_mode)
{
    return RadioManagement_UsesBothSidebands(dmod_mode) == false && RadioManagement_LSBActive(dmod_mode) == false;
}


static void RadioManagement_PowerFromADCValue(float inval, float sensor_null, float coupling_calc,volatile float* pwr_ptr, volatile float* dbm_ptr)
{
    float32_t pwr;
    const float32_t val = sensor_null + ((inval * SWR_ADC_VOLT_REFERENCE) / SWR_ADC_FULL_SCALE);
    // get nominal A/D reference voltage
    // divide by full-scale A/D count to yield actual input voltage from detector
    // offset result

    if(val <= LOW_POWER_CALC_THRESHOLD)     // is this low power as evidenced by low voltage from the sensor?
    {
        pwr = LOW_RF_PWR_COEFF_A + (LOW_RF_PWR_COEFF_B * val) + (LOW_RF_PWR_COEFF_C * (val * val) ) + (LOW_RF_PWR_COEFF_D * powf(val, 3));
    }
    else            // it is high power
    {
        pwr = HIGH_RF_PWR_COEFF_A + (HIGH_RF_PWR_COEFF_B * val) + (HIGH_RF_PWR_COEFF_C * (val * val));
    }
    // calculate forward and reverse RF power in watts (p = a + bx + cx^2) for high power (above 50-60

    if(pwr < 0)     // prevent negative power readings from emerging from the equations - particularly at zero output power
    {
        pwr = 0;
    }

    const float32_t dbm = (10 * (log10f(pwr))) + 30 + coupling_calc;
    *dbm_ptr = dbm;
    *pwr_ptr = pow10f(dbm/10)/1000;
}

/*
 * @brief Measures and calculates TX Power Output and SWR, has to be called regularly
 * @returns true if new values have been calculated
 */
bool RadioManagement_UpdatePowerAndVSWR()
{

    uint16_t  val_p,val_s = 0;
    float sensor_null, coupling_calc;
    bool retval = false;

    // Collect samples
    if(swrm.p_curr < SWR_SAMPLES_CNT)
    {
        // Get next sample
        if(!(ts.flags1 & FLAGS1_SWAP_FWDREV_SENSE))       // is bit NOT set?  If this is so, do NOT swap FWD/REV inputs from power detectors
        {
            val_p = HAL_ADC_GetValue(&hadc2); // forward
            val_s = HAL_ADC_GetValue(&hadc3); // return
        }
        else        // FWD/REV bits should be swapped
        {
            val_p = HAL_ADC_GetValue(&hadc3); // forward
            val_s = HAL_ADC_GetValue(&hadc2); // return
        }

        // Add to accumulator to average A/D values
        swrm.fwd_calc += val_p;
        swrm.rev_calc += val_s;

        swrm.p_curr++;
    }
    else
    {
        // obtain and calculate power meter coupling coefficients
        coupling_calc = (swrm.coupling_calc[ts.coupling_band] - 100.0)/10.0;
        // offset to zero and rescale to 0.1 dB/unit


        sensor_null = (swrm.sensor_null - 100.0) / 1000 ;
        // get calibration factor
        // offset it so that 100 = 0
        // divide so that each step = 1 millivolt

        // Compute average values
        RadioManagement_PowerFromADCValue(swrm.fwd_calc / SWR_SAMPLES_CNT, sensor_null, coupling_calc,&swrm.fwd_pwr, &swrm.fwd_dbm);
        RadioManagement_PowerFromADCValue(swrm.rev_calc / SWR_SAMPLES_CNT, sensor_null, coupling_calc,&swrm.rev_pwr, &swrm.rev_dbm);

        // Reset accumulators and variables for power measurements
        swrm.p_curr   = 0;
        swrm.fwd_calc = 0;
        swrm.rev_calc = 0;
        // Calculate VSWR from power readings

        swrm.vswr = (1+sqrtf(swrm.rev_pwr/swrm.fwd_pwr))/(1-sqrtf(swrm.rev_pwr/swrm.fwd_pwr));

		// Perform VSWR protection iff threshold is > 1 AND enough forward power exists for a valid calculation
        if ( ts.vswr_protection_threshold > 1 && swrm.fwd_pwr >= SWR_MIN_CALC_POWER)
        {
            if ( swrm.vswr > ts.vswr_protection_threshold )
            {
                RadioManagement_DisablePaBias ( );
                swrm.high_vswr_detected = true;

                // change output power to "PA_LEVEL_0_5W" when VSWR protection is active
                RadioManagement_SetPowerLevel ( RadioManagement_GetBand ( df.tune_new), PA_LEVEL_MINIMAL );
            }
        }

        retval = true;
    }
    return retval;
}

/**
 * @brief Calculation and setting of RX Audio Codec gain for ADC of IQ signal and detects signal clipping, needs to be called regularly every 40ms.
 */
void RadioManagement_HandleRxIQSignalCodecGain()
{
    static  uint16_t    rfg_timer= 0;   // counter used for timing RFG control decay
    static  float       auto_rfg = 8;
    float   rfg_calc;
    float gcalc;

    // ************************
    // Update S-Meter and control the input gain of the codec to maximize A/D and receiver dynamic range
    // ************************
    //
    // Calculate attenuation of "RF Codec Gain" setting so that S-meter reading can be compensated.
    // for input RF attenuation setting
    //
    if(ts.rf_codec_gain == RF_CODEC_GAIN_AUTO)      // Is RF gain in "AUTO" mode?
    {
        rfg_calc = auto_rfg;
    }
    else                    // not in "AUTO" mode
    {
        rfg_calc = ts.rf_codec_gain;     // get copy of RF gain setting
        auto_rfg = rfg_calc;        // keep "auto" variable updated with manual setting when in manual mode
        rfg_timer = 0;
    }

    // rfg_calc = (auto_rfg + 1) * 2 + 13; --> 9 * 2 + 13 = 31 !
    rfg_calc += 1;  // offset to prevent zero
    rfg_calc *= 2;  // double the range of adjustment
    rfg_calc += 13; // offset, as bottom of range of A/D gain control is not useful (e.g. ADC saturates before RX hardware)
    if(rfg_calc >31)    // limit calc to hardware range
    {
        rfg_calc = 31;
    }

    Codec_IQInGainAdj(rfg_calc); // set the RX gain on the codec

    // Now calculate the RF gain setting
    gcalc = pow10(((rfg_calc * 1.5) - 34.5) / 10) ;

    // codec has 1.5 dB/step
    // offset codec setting by 34.5db (full gain = 12dB)
    // convert to power ratio

    ads.codec_gain_calc = sqrtf(gcalc);     // convert to voltage ratio - we now have current A/D (codec) gain setting

    // Now handle automatic A/D input gain control timing
    rfg_timer++;    // bump RFG timer

    if(rfg_timer > 10000)   // limit count of RFG timer
    {
        rfg_timer = 10000;
    }

    if(ads.adc_half_clip)       // did clipping almost occur?
    {
        if(rfg_timer >= AUTO_RFG_DECREASE_LOCKOUT)      // has enough time passed since the last gain decrease?
        {
            if(auto_rfg)        // yes - is this NOT zero?
            {
                auto_rfg -= 0.5;    // decrease gain one step, 1.5dB (it is multiplied by 2, above)
                rfg_timer = 0;  // reset the adjustment timer
            }
        }
    }
    else if(!ads.adc_quarter_clip)      // no clipping occurred
    {
        if(rfg_timer >= AUTO_RFG_INCREASE_TIMER)        // has it been long enough since the last increase?
        {
            auto_rfg += 0.5;    // increase gain by one step, 1.5dB (it is multiplied by 2, above)
            rfg_timer = 0;  // reset the timer to prevent this from executing too often
            if(auto_rfg > 8)    // limit it to 8
            {
                auto_rfg = 8;
            }
        }
    }

    ads.adc_half_clip = false;      // clear "half clip" indicator that tells us that we should decrease gain
    ads.adc_quarter_clip = false;   // clear indicator that, if not triggered, indicates that we can increase gain
}

const cw_mode_map_entry_t cw_mode_map[] =
{
        {CW_OFFSET_TX, CW_SB_USB},      // 0
        {CW_OFFSET_TX, CW_SB_LSB},      // 1
        {CW_OFFSET_TX, CW_SB_AUTO},     // 2
        {CW_OFFSET_RX, CW_SB_USB},      // 3
        {CW_OFFSET_RX, CW_SB_LSB},      // 4
        {CW_OFFSET_RX, CW_SB_AUTO},     // 5
        {CW_OFFSET_SHIFT, CW_SB_USB},   // 6
        {CW_OFFSET_SHIFT, CW_SB_LSB},   // 7
        {CW_OFFSET_SHIFT, CW_SB_AUTO},  // 8
};


const cw_mode_map_entry_t* RadioManagement_CWConfigValueToModeEntry(uint8_t cw_offset_mode)
{
    const cw_mode_map_entry_t* retval = &cw_mode_map[0];

    if (cw_offset_mode < CW_OFFSET_NUM)
    {
        retval = &cw_mode_map[cw_offset_mode];
    }
    return  retval;
}

uint8_t RadioManagement_CWModeEntryToConfigValue(const cw_mode_map_entry_t* mode_entry)
{
    uint8_t retval = CW_OFFSET_MODE_DEFAULT;

    for (uint8_t cw_offset_mode = 0; cw_offset_mode < CW_OFFSET_NUM; cw_offset_mode++)
    {
        if (cw_mode_map[cw_offset_mode].dial_mode == mode_entry->dial_mode
                && cw_mode_map[cw_offset_mode].sideband_mode == mode_entry->sideband_mode
        )
        {
            retval = cw_offset_mode;
            break;
        }
    }
    return  retval;
}

void RadioManagement_ToggleVfoAB()
{
    uint8_t vfo_new,vfo_active;

    if(is_vfo_b())          // LSB on VFO mode byte set?
    {
        vfo_new = VFO_A;
        vfo_active = VFO_B;
        ts.vfo_mem_mode &= ~VFO_MEM_MODE_VFO_B; // yes, it's now VFO-B mode, so clear it, setting it to VFO A mode
    }
    else                            // LSB on VFO mode byte NOT set?
    {
        ts.vfo_mem_mode |= VFO_MEM_MODE_VFO_B;          // yes, it's now in VFO-A mode, so set it, setting it to VFO B mode
        vfo_new = VFO_B;
        vfo_active = VFO_A;
    }
    vfo[vfo_active].band[ts.band->band_mode].dial_value = df.tune_old; //band_dial_value[ts.band];     // save "VFO B" settings
    vfo[vfo_active].band[ts.band->band_mode].decod_mode = ts.dmod_mode;    //band_decod_mode[ts.band];
    vfo[vfo_active].band[ts.band->band_mode].digital_mode = ts.digital_mode;

    df.tune_new = vfo[vfo_new].band[ts.band->band_mode].dial_value;

    bool digitalModeDiffers = ts.digital_mode != vfo[vfo_new].band[ts.band->band_mode].digital_mode;
    bool newIsDigitalMode = vfo[vfo_new].band[ts.band->band_mode].decod_mode == DEMOD_DIGI;
    ts.digital_mode = vfo[vfo_new].band[ts.band->band_mode].digital_mode;

    if(ts.dmod_mode != vfo[vfo_new].band[ts.band->band_mode].decod_mode || (newIsDigitalMode &&  digitalModeDiffers) )
    {
        RadioManagement_SetDemodMode(vfo[vfo_new].band[ts.band->band_mode].decod_mode);
    }
    nr_params.first_time = 1; // restart in case of VFO-Toggle
}

/**
 * @returns true == fm deviation 5khz, false == fm deviation = 2.5khz
 */
bool RadioManagement_FmDevIs5khz()
{
    return (ts.flags2 & FLAGS2_FM_MODE_DEVIATION_5KHZ) != 0;
}

/**
 * @param is5khz true == fm deviation 5khz, false == fm deviation = 2.5khz
 */
void RadioManagement_FmDevSet5khz(bool is5khz)
{
    if (is5khz)
    {
        ts.flags2 |= FLAGS2_FM_MODE_DEVIATION_5KHZ;
    }
    else
    {
        ts.flags2 &= ~FLAGS2_FM_MODE_DEVIATION_5KHZ;
    }
}

bool RadioManagement_TxPermitted()
{
    return ts.dmod_mode != DEMOD_SAM && RadioManagement_IsTxDisabled();
}

bool RadioManagement_Transverter_IsEnabled()
{
    return (ts.xverter_mode & 0xf) > 0;
    }

uint64_t RadioManagement_Transverter_GetFreq(const uint32_t dial_freq, const uint8_t trx_mode)
{
    uint32_t xverter_offset = (ts.xverter_offset_tx != 0 && trx_mode == TRX_MODE_TX) ? ts.xverter_offset_tx : ts.xverter_offset;

    uint64_t offset_multiplier = xverter_offset>XVERTER_OFFSET_MAX_HZ? 1000 : 1;
    uint64_t offset_offset = xverter_offset - (xverter_offset>XVERTER_OFFSET_MAX_HZ ? ((XVERTER_OFFSET_MAX_HZ)-XVERTER_OFFSET_MAX_HZ/1000)  : 0);

    return dial_freq * ts.xverter_mode + offset_offset * offset_multiplier;
}
