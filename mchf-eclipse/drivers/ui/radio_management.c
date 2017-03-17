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

// Common
#include "radio_management.h"
#include "mchf_board.h"
#include "profiling.h"
#include "adc.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <soft_tcxo.h>

#include "mchf_hw_i2c.h"

#include "freedv_mchf.h"
// SI570 control
#include "ui_si570.h"
#include "codec.h"
#include "audio_driver.h"
#include "audio_management.h"

#include "cat_driver.h"


#include "ui_configuration.h"
#include "ui_menu.h"  // for CONFIG_160M_FULL_POWER_ADJUST and Co.

#include "config_storage.h"

#include "cw_gen.h"
#include "freedv_api.h"
#include "codec2_fdmdv.h"

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
__IO DialFrequency               df;


#define BandInfoGenerate(BAND,SUFFIX,NAME) { TX_POWER_FACTOR_##BAND##_DEFAULT, BAND_FREQ_##BAND , BAND_SIZE_##BAND , NAME }

const BandInfo bandInfo[] =
{
    BandInfoGenerate(80,M,"80m") ,
    BandInfoGenerate(60,M,"60m"),
    BandInfoGenerate(40,M,"40m"),
    BandInfoGenerate(30,M,"30m"),
    BandInfoGenerate(20,M,"20m"),
    BandInfoGenerate(17,M,"17m"),
    BandInfoGenerate(15,M,"15m"),
    BandInfoGenerate(12,M,"12m"),
    BandInfoGenerate(10,M,"10m"),
    BandInfoGenerate(6,M,"6m"),
    BandInfoGenerate(4,M,"4m"),
    BandInfoGenerate(2,M,"2m"),
    BandInfoGenerate(70,CM,"70cm"),
    BandInfoGenerate(23,CM,"23cm"),
    BandInfoGenerate(2200,M,"2200m"),
    BandInfoGenerate(630,M,"630m"),
    BandInfoGenerate(160,M,"160m"),
    { 0, 0, 0, "Gen" } // Generic Band
};

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
 * @brief API Function, implements application logic for changing the power level including filter changes
 *
 *
 * @param power_level The requested power level (as PA_LEVEL constants)
 * @returns true if power level has been changed, false otherwise
 */
bool RadioManagement_PowerLevelChange(uint8_t band, uint8_t power_level)
{
    bool retval = false;

    if(ts.dmod_mode == DEMOD_AM)                // in AM mode?
    {
        if(power_level >= PA_LEVEL_TUNE_KEEP_CURRENT)     // yes, power over 2 watts?
        {
            power_level = PA_LEVEL_2W;  // force to 2 watt mode when we "roll over"
        }
    }
    else        // other modes, do not limit max power
    {
        if(power_level >= PA_LEVEL_TUNE_KEEP_CURRENT)
        {
            power_level = PA_LEVEL_FULL; //  0 == full power
        }
    }

    if (power_level != ts.power_level)
    {
        retval = true;
        ts.power_level = power_level;
        if(ts.tune && !ts.iq_freq_mode)         // recalculate sidetone gain only if transmitting/tune mode
        {
            Codec_TxSidetoneSetgain(ts.txrx_mode);
        }
        // Set TX power factor - to reflect changed power
        RadioManagement_SetBandPowerFactor(band);
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
                ts.power_temp = ts.power_level;             //store tx level and set tune level
                ts.power_level =ts.tune_power_level;
            }
            // DDS on
            RadioManagement_SwitchTxRx(TRX_MODE_TX,true);                // tune ON
            retval = (ts.txrx_mode == TRX_MODE_TX);
        }
        else
        {
            if(ts.tune_power_level != PA_LEVEL_TUNE_KEEP_CURRENT)
            {
                ts.power_level = ts.power_temp; // restore tx level
            }

            RadioManagement_SwitchTxRx(TRX_MODE_RX,true);                // tune OFF
            retval = false; // no longer tuning
        }
    }
    else
    {
        retval = false;    // no TUNE mode in AM or FM or with disabled TX!
    }
    return retval;
}

uint32_t RadioManagement_Dial2TuneFrequency(const uint32_t dial_freq, uint8_t txrx_mode)
{
    uint32_t tune_freq = dial_freq;

    //
    // Do "Icom" style frequency offset of the LO if in "CW OFFSET" mode.  (Display freq. is also offset!)
    if(ts.dmod_mode == DEMOD_CW)            // In CW mode?
    {
        switch(ts.cw_offset_mode)
        {
        case CW_OFFSET_USB_SHIFT:    // Yes - USB?
            tune_freq -= ts.sidetone_freq;
            // lower LO by sidetone amount
            break;
        case CW_OFFSET_LSB_SHIFT:   // LSB?
            tune_freq += ts.sidetone_freq;
            // raise LO by sidetone amount
            break;
        case CW_OFFSET_AUTO_SHIFT:  // Auto mode?  Check flag
            if(ts.cw_lsb)
                tune_freq += ts.sidetone_freq;          // it was LSB - raise by sidetone amount
            else
                tune_freq -= ts.sidetone_freq;          // it was USB - lower by sidetone amount
        }
    }


    // Offset dial frequency if the RX/TX frequency translation is active and we are not transmitting in CW mode
    // In CW TX mode we do not use frequency translation, this permits to use the generated I or Q channel as audio sidetone

    if(!((ts.dmod_mode == DEMOD_CW || (ts.dvmode == true)) && (txrx_mode == TRX_MODE_TX)))
    {
        tune_freq += AudioDriver_GetTranslateFreq();        // magnitude of shift is quadrupled at actual Si570 operating frequency
    }

    // Extra tuning actions
    if(txrx_mode == TRX_MODE_RX)
    {
        tune_freq += (ts.rit_value*20); // Add RIT on receive
    }

    return tune_freq*TUNE_MULT;
}

/**
 * @brief switch off the PA Bias to mute HF output ( even if PTT is on )
 * Using this method the PA will be effectively muted no matter what setting
 * the main bias switch has (which directly connected to the PTT HW Signal)
 * Used to suppress signal path reconfiguration noise during rx/tx and tx/rx switching
 */
void RadioManagement_DisablePaBias()
{
    MchfBoard_SetPaBiasValue(0);
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
    MchfBoard_SetPaBiasValue(calc_var);
}


bool RadioManagement_ChangeFrequency(bool force_update, uint32_t dial_freq,uint8_t txrx_mode)
{
    // everything else uses main VFO frequency
    // uint32_t    tune_freq;
    bool lo_change_pending = false;

    // Calculate actual tune frequency
    ts.tune_freq_req = RadioManagement_Dial2TuneFrequency(dial_freq, txrx_mode);

    if((ts.tune_freq != ts.tune_freq_req) || (ts.refresh_freq_disp) || df.temp_factor_changed || force_update )  // did the frequency NOT change and display refresh NOT requested??
    {

        if(ts.sysclock-ts.last_tuning > 5 || ts.last_tuning == 0)     // prevention for SI570 crash due too fast frequency changes
        {
            Si570_ResultCodes lo_prep_result = Si570_PrepareNextFrequency(ts.tune_freq_req, df.temp_factor);
            // first check and mute output if a large step is to be done
            if(Si570_IsNextStepLarge() == true)     // did the tuning require that a large tuning step occur?
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

            Si570_ResultCodes lo_exec_result = SI570_TUNE_IMPOSSIBLE;
            if (lo_prep_result != SI570_TUNE_IMPOSSIBLE)
            {
                // Set frequency
                lo_exec_result = Si570_ChangeToNextFrequency();
            }

            // if i2c error or verify error, there is a chance that we can fix that, so we mark this
            // as NOT executed, in all other cases we assume the change has happened (but may prevent TX)
            if (lo_exec_result != SI570_I2C_ERROR && lo_exec_result != SI570_ERROR_VERIFY)
            {
                df.temp_factor_changed = false;
                ts.tune_freq = ts.tune_freq_req;        // frequency change required - update change detector
                // Save current freq
                df.tune_old = dial_freq*TUNE_MULT;
            }
            else
            {
                ts.last_lo_result = lo_exec_result;
            }

            if (ts.last_lo_result == SI570_OK || ts.last_lo_result == SI570_TUNE_LIMITED)
            {

                ts.tx_disable &= ~TX_DISABLE_OUTOFRANGE;
            }
            else
            {
                ts.tx_disable |= TX_DISABLE_OUTOFRANGE;
            }

            uint32_t tune_freq_real = ts.tune_freq/TUNE_MULT;

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

Si570_ResultCodes RadioManagement_ValidateFrequencyForTX(uint32_t dial_freq)
{
    // we check with the si570 code if the frequency is tunable, we do not tune to it.
    return Si570_PrepareNextFrequency(RadioManagement_Dial2TuneFrequency(dial_freq, TRX_MODE_TX), df.temp_factor);
}


void RadioManagement_SwitchTxRx(uint8_t txrx_mode, bool tune_mode)
{
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
                vfo[vfo_rx].band[ts.band].dial_value = df.tune_new; // yes - save current RX frequency in VFO location (B)
            }
            tune_new = vfo[vfo_tx].band[ts.band].dial_value;    // load with VFO-A frequency
        }
        else                    // we are in RX mode
        {
            tune_new = vfo[vfo_rx].band[ts.band].dial_value;    // load with VFO-B frequency
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
        tx_ok = RadioManagement_ValidateFrequencyForTX(tune_new/TUNE_MULT) != SI570_TUNE_IMPOSSIBLE;


        // this code handles the ts.tx_disable
        // even if ts.tx_disble is set in CW and only in CW we still switch to TX
        // but leave the PA disabled. This is for support of CW training right with the mcHF.
        if (RadioManagement_IsTxDisabled())
        {
            if (tx_ok == true && ts.dmod_mode == DEMOD_CW)
            {
                tx_pa_disabled = true;
            }
            else
            {
                // in any other case, it is not okay to transmit with ts.tx_disable == true
                tx_ok = false;
            }
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

        if(txrx_mode_final == TRX_MODE_RX)
        {
                // remember current agc
                ads.agc_holder = ads.agc_val;
        }
        else
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
                MchfBoard_RedLed(LED_STATE_ON); // TX
                MchfBoard_EnableTXSignalPath(true); // switch antenna to output and codec output to QSE mixer
            }
        }

        df.tune_new = tune_new;
        RadioManagement_ChangeFrequency(false,df.tune_new/TUNE_MULT, txrx_mode_final);
        // ts.audio_dac_muting_flag = true; // let the audio being muted initially as long as we need it

        // there might have been a band change between the modes, make sure to have the power settings fitting the mode
        if (txrx_mode_final == TRX_MODE_TX)
        {
            uint8_t tx_band = RadioManagement_GetBand(tune_new/TUNE_MULT);
            RadioManagement_PowerLevelChange(tx_band,ts.power_level);
            RadioManagement_SetBandPowerFactor(tx_band);
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

            MchfBoard_EnableTXSignalPath(false); // switch antenna to input and codec output to lineout
            MchfBoard_RedLed(LED_STATE_OFF);      // TX led off
            ts.audio_dac_muting_flag = false; // unmute audio output
            CwGen_PrepareTx();
            // make sure the keyer is set correctly for next round
        }

        if (ts.txrx_mode != txrx_mode_final)
        {
            Codec_SwitchTxRxMode(txrx_mode_final);

            if (txrx_mode_final == TRX_MODE_RX)
            {
                // Assure that TX->RX timer gets reset at the end of an element
                // TR->RX audio un-muting timer and Audio/AGC De-Glitching handler
                if (ts.tx_audio_source == TX_AUDIO_MIC && (ts.dmod_mode != DEMOD_CW))
                {
                    ts.audio_spkr_unmute_delay_count = SSB_RX_DELAY;  // set time delay in SSB mode with MIC
                    ts.audio_processor_input_mute_counter = SSB_RX_DELAY;
                }

            }
            else
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
}



/*
 * @returns: false -> use USB, true -> use LSB
 */
bool RadioManagement_CalculateCWSidebandMode()
{
    bool retval = false;
    switch(ts.cw_offset_mode)
    {
    case CW_OFFSET_AUTO_TX:                     // For "auto" modes determine if we are above or below threshold frequency
    case CW_OFFSET_AUTO_RX:
    case CW_OFFSET_AUTO_SHIFT:
        // if (RadioManagement_SSB_AutoSideBand(df.tune_new/TUNE_MULT) == DEMOD_USB)   // is the current frequency above the USB threshold?
        if (df.tune_new/TUNE_MULT > USB_FREQ_THRESHOLD || RadioManagement_GetBand(df.tune_new/TUNE_MULT) == BAND_MODE_60)   // is the current frequency above the USB threshold or is it 60m?
        {
            retval = false;                      // yes - indicate that it is USB
        }
        else
        {
            retval = true;                      // no - LSB
        }
        break;
    case CW_OFFSET_LSB_TX:
    case CW_OFFSET_LSB_RX:
    case CW_OFFSET_LSB_SHIFT:
        retval = true;              // It is LSB
        break;
    case CW_OFFSET_USB_TX:
    case CW_OFFSET_USB_RX:
    case CW_OFFSET_USB_SHIFT:
    default:
        retval = false;
        break;
    }
    return retval;
}


void RadioManagement_ChangeBandFilter(uchar band)
{
    // here the bands are mapped to the hardware functions to enable the proper
    // filter configuration for a given band

    switch(band)
    {
    case BAND_MODE_2200:
    case BAND_MODE_630:
    case BAND_MODE_160:
    case BAND_MODE_80:
        MchfBoard_SelectLpfBpf(0);
        break;

    case BAND_MODE_60:
    case BAND_MODE_40:
        MchfBoard_SelectLpfBpf(1);
        break;

    case BAND_MODE_30:
    case BAND_MODE_20:
        MchfBoard_SelectLpfBpf(2);
        break;

    case BAND_MODE_17:
    case BAND_MODE_15:
    case BAND_MODE_12:
    case BAND_MODE_10:
    case BAND_MODE_6:
    case BAND_MODE_4:
        MchfBoard_SelectLpfBpf(3);
        break;

    default:
        break;
    }

}
typedef struct BandFilterDescriptor
{
    uint32_t upper;
    uint16_t filter_band;
    uint16_t band_mode;
} BandFilterDescriptor;

#define BAND_FILTER_NUM 7

// The descriptor array below has to be ordered from the lowest BPF frequency filter
// to the highest.
static const BandFilterDescriptor bandFilters[BAND_FILTER_NUM] =
{
    { BAND_FILTER_UPPER_160, COUPLING_160M, BAND_MODE_160 },
    { BAND_FILTER_UPPER_80,  COUPLING_80M, BAND_MODE_80 },
    { BAND_FILTER_UPPER_40,  COUPLING_40M, BAND_MODE_40 },
    { BAND_FILTER_UPPER_20,  COUPLING_20M, BAND_MODE_20 },
    { BAND_FILTER_UPPER_10,  COUPLING_15M, BAND_MODE_10 },
    { BAND_FILTER_UPPER_6,  COUPLING_6M, BAND_MODE_6 }
};



/**
 * @brief Select and activate the correct BPF for the frequency given in @p freq
 *
 *
 * @param freq The frequency to activate the BPF for in Hz
 *
 * @warning  If the frequency given in @p freq is too high for any of the filters, no filter change is executed.
 */
void RadioManagement_SetHWFiltersForFrequency(ulong freq)
{
    int idx;
    for (idx = 0; idx < BAND_FILTER_NUM; idx++)
    {
        if(freq < bandFilters[idx].upper)       // are we low enough if frequency for this band filter?
        {
            if(ts.filter_band != bandFilters[idx].filter_band)
            {
                RadioManagement_ChangeBandFilter(bandFilters[idx].band_mode);   // yes - set to desired band configuration
                ts.filter_band = bandFilters[idx].filter_band;
            }
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

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSetBandPowerFactor
//* Object              : TX chain gain is not const for the 3-30 Mhz range
//* Input Parameters    : so adjust here
//* Output Parameters   :

/* Depends on globals: ts.pwr_adj, ts.power_level, df.tune_new, ts.flags2 & FLAGS2_LOW_BAND_BIAS_REDUCE
 * Impacts globals:    ts.tx_power_factor
 */

void RadioManagement_SetBandPowerFactor(uchar band)
{
    float32_t   pf_bandvalue, pf_levelscale;    // used as a holder for percentage of power output scaling

    // FIXME: This code needs fixing, the hack for TX Outside should at least reduce power factor for lower bands
    if (band >= MAX_BANDS)
    {
        if(ts.flags1 & FLAGS1_TX_OUTSIDE_BANDS)
        {               // TX outside bands **very dirty hack**: I use constant for power
          pf_bandvalue = 12; // because no factor is known outside bands
        }               // I never will use this function (DF8OE)
        else
        {
          pf_bandvalue = 3; // use very low value in case of wrong call to this function
        }
    }
    else
    {
        pf_bandvalue = (float)ts.pwr_adj[ts.power_level == PA_LEVEL_FULL?ADJ_FULL_POWER:ADJ_5W][band];
    }

    pf_bandvalue /= RadioManagement_IsPowerFactorReduce(df.tune_old/TUNE_MULT)? 400: 100;


    // now rescale to power levels <5 watts, is so-configured
    switch(ts.power_level)
    {
    case    PA_LEVEL_0_5W:
        pf_levelscale = 0.316;        // rescale for 10% of 5 watts (0.5 watts)
        break;
    case    PA_LEVEL_1W:
        pf_levelscale = 0.447;        // rescale for 20% of 5 watts (1.0 watts)
        break;
    case    PA_LEVEL_2W:
        pf_levelscale = 0.6324;       // rescale for 40% of 5 watts (2 watts)
        break;
    default:                    // 100% is 5 watts or full power!!
        pf_levelscale = 1;
        break;
    }


    float32_t power_factor = pf_levelscale * pf_bandvalue;  // rescale this for the actual power level

    // limit hard limit for power factor since it otherwise may overdrive the PA section
    ts.tx_power_factor =
            (power_factor > TX_POWER_FACTOR_MAX_INTERNAL) ?
            TX_POWER_FACTOR_MAX_INTERNAL : power_factor;
}




/*
 * @brief returns the band according to the given frequency
 * @param freq  frequency to get band for. Unit is Hertz. This value is used without any further adjustments and should be the intended RX/TX frequency and NOT the IQ center frequency
 *
 */
void RadioManagement_SetDemodMode(uint32_t new_mode)
{


    ts.dsp_inhibit++;
    ads.af_disabled++;

    if (new_mode == DEMOD_DIGI)
    {
        if (ts.digital_mode == 0)
        {
            ts.digital_mode = 1;
            // TODOD: more clever selection of initial DV Mode, if none was previously selected
        }
        RadioManagement_ChangeCodec(ts.digital_mode,1);
    }
    else if (ts.dmod_mode == DEMOD_DIGI)
    {
            RadioManagement_ChangeCodec(ts.digital_mode,0);
    }

    if (new_mode == DEMOD_FM && ts.dmod_mode != DEMOD_FM)
    {
        // ads.fm_squelched = true;
        // ads.fm_sql_avg = 1;
    }

    AudioDriver_SetRxAudioProcessing(new_mode, false);
    AudioDriver_TxFilterInit(new_mode);
    AudioManagement_SetSidetoneForDemodMode(ts.dmod_mode,false);


    // Finally update public flag
    ts.dmod_mode = new_mode;

    if  (ads.af_disabled) { ads.af_disabled--; }
    if (ts.dsp_inhibit) { ts.dsp_inhibit--; }

}


uint8_t RadioManagement_GetBand(ulong freq)
{
    static uint8_t band_scan_old = 99;
    uchar   band_scan;

    band_scan = 0;

    freq *= TUNE_MULT;

    // first try the last band, and see if it is an match
    if (band_scan_old != 99 && freq >= bandInfo[band_scan_old].tune && freq <= (bandInfo[band_scan_old].tune + bandInfo[band_scan_old].size))
    {
        band_scan = band_scan_old;
    }
    else
    {
        for(band_scan = 0; band_scan < MAX_BANDS; band_scan++)
        {
            if((freq >= bandInfo[band_scan].tune) && (freq <= (bandInfo[band_scan].tune + bandInfo[band_scan].size)))   // Is this frequency within this band?
            {
                break;  // yes - stop the scan
            }
        }
    }
    band_scan_old = band_scan;  // update band change detector
    return band_scan;       // return with the band
}

const int ptt_break_time = 15;

uint32_t RadioManagement_SSB_AutoSideBand(uint32_t freq) {
    uint32_t retval;

    if((ts.lsb_usb_auto_select == AUTO_LSB_USB_60M) && ((freq < USB_FREQ_THRESHOLD) && (RadioManagement_GetBand(freq) != BAND_MODE_60)))       // are we <10 MHz and NOT on 60 meters?
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


void RadioManagement_HandlePttOnOff()
{
    static uint32_t ptt_break_timer = ptt_break_time;

    // Not when tuning
    if(ts.tune == false)
    {
        // PTT on
        if(ts.ptt_req)
        {
            if(ts.txrx_mode == TRX_MODE_RX && (RadioManagement_IsTxDisabled() == false || ts.dmod_mode == DEMOD_CW))
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

            // PTT off for all non-CW modes
            if(ts.dmod_mode != DEMOD_CW || ts.tx_stop_req == true)
            {
                // PTT flag on ?
                if(ts.txrx_mode == TRX_MODE_TX)
                {
                    // PTT line released ?
                    if(mchf_ptt_dah_line_pressed() == false)
                    {
                        ptt_break_timer--;
                    }
                    if(ptt_break_timer == 0 || ts.tx_stop_req == true) {
                        // Back to RX
                        RadioManagement_SwitchTxRx(TRX_MODE_RX,false);              // PTT
                        ptt_break_timer = ptt_break_time;
                    }
                }
                // if we are here the stop request has been processed
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
            retval = RadioManagement_SSB_AutoSideBand(df.tune_new / TUNE_MULT) == demod_mode;       // is this a voice mode, subject to "auto" LSB/USB select?
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
            ts.digi_lsb = RadioManagement_SSB_AutoSideBand(df.tune_new / TUNE_MULT) == DEMOD_LSB;
            // is this a voice mode, subject to "auto" LSB/USB select?
        }
        break;
    case DEMOD_CW:
        retval = (ts.demod_mode_disable & DEMOD_CW_DISABLE) == 0;      // is CW enabled?
        break;
    case DEMOD_FM:
        // FIXME: ts.lsb_usb_auto_select acts as fm select here. Rename!
        retval = (ts.iq_freq_mode != FREQ_IQ_CONV_MODE_OFF) && (((ts.flags2 & FLAGS2_FM_MODE_ENABLE) != 0) || (ts.band == BAND_MODE_10 && ts.lsb_usb_auto_select));   // is FM enabled?
        break;
    case DEMOD_SAM:
        retval =( ts.flags1 & FLAGS1_SAM_ENABLE) != 0;        // is SAM enabled?
        break;
    default:
        retval = true;
    }
    return retval;
}


uint32_t RadioManagement_NextDemodMode(uint32_t loc_mode, bool alternate_mode)
{
   uint32_t retval = loc_mode;
   // default is to simply return the original mode

    if(alternate_mode == true)
    {
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
                if (ads.sam_sideband > SAM_SIDEBAND_MAX)
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
                ts.flags2 ^= FLAGS2_FM_MODE_DEVIATION_5KHZ;
            }
            // if there is no explicit alternative mode
            // we return the original mode.
    }
    else
    {
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
    }

    return retval;
}


bool RadioManagement_UsesBothSidebands(uint16_t dmod_mode)
{
    return ((dmod_mode == DEMOD_AM) ||(dmod_mode == DEMOD_SAM && ads.sam_sideband == 0) || (dmod_mode == DEMOD_FM));
}

bool RadioManagement_LSBActive(uint16_t dmod_mode)
{
    bool    is_lsb;

    switch(dmod_mode)        // determine if the receiver is set to LSB or USB or FM
    {
    case DEMOD_SAM:
        is_lsb = ads.sam_sideband == 1;
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

static void RadioManagement_PowerFromADCValue(float val, float sensor_null, float coupling_calc,volatile float* pwr_ptr, volatile float* dbm_ptr)
{
    float pwr;
    float dbm;
    val *= SWR_ADC_VOLT_REFERENCE;    // get nominal A/D reference voltage
    val /= SWR_ADC_FULL_SCALE;        // divide by full-scale A/D count to yield actual input voltage from detector
    val += sensor_null;               // offset result

    if(val <= LOW_POWER_CALC_THRESHOLD)     // is this low power as evidenced by low voltage from the sensor?
    {
        pwr = LOW_RF_PWR_COEFF_A + (LOW_RF_PWR_COEFF_B * val) + (LOW_RF_PWR_COEFF_C * pow(val,2 )) + (LOW_RF_PWR_COEFF_D * pow(val, 3));
    }
    else            // it is high power
    {
        pwr = HIGH_RF_PWR_COEFF_A + (HIGH_RF_PWR_COEFF_B * val) + (HIGH_RF_PWR_COEFF_C * pow(val, 2));
    }
    // calculate forward and reverse RF power in watts (p = a + bx + cx^2) for high power (above 50-60

    if(pwr < 0)     // prevent negative power readings from emerging from the equations - particularly at zero output power
    {
        pwr = 0;
    }

    dbm = (10 * (log10(pwr))) + 30 + coupling_calc;
    pwr = pow10(dbm/10)/1000;
    *pwr_ptr = pwr;
    *dbm_ptr = dbm;
}

/*
 * @brief Measures and calculates TX Power Output and SWR, has to be called regularily
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
        swrm.fwd_calc += (float)val_p;
        swrm.rev_calc += (float)val_s;

        swrm.p_curr++;
    }
    else
    {
        // obtain and calculate power meter coupling coefficients
        coupling_calc = swrm.coupling_calc[ts.filter_band];
        coupling_calc -= 100;                       // offset to zero
        coupling_calc /= 10;                        // rescale to 0.1 dB/unit


        sensor_null = (float)swrm.sensor_null;  // get calibration factor
        sensor_null -= 100;                     // offset it so that 100 = 0
        sensor_null /= 1000;                    // divide so that each step = 1 millivolt

        // Compute average values

        swrm.fwd_calc /= SWR_SAMPLES_CNT;
        swrm.rev_calc /= SWR_SAMPLES_CNT;

        RadioManagement_PowerFromADCValue(swrm.fwd_calc, sensor_null, coupling_calc,&swrm.fwd_pwr, &swrm.fwd_dbm);
        RadioManagement_PowerFromADCValue(swrm.rev_calc, sensor_null, coupling_calc,&swrm.rev_pwr, &swrm.rev_dbm);

        // Reset accumulators and variables for power measurements
        swrm.p_curr   = 0;
        swrm.fwd_calc = 0;
        swrm.rev_calc = 0;


        // swrm.vswr = swrm.fwd_dbm-swrm.rev_dbm;      // calculate VSWR

        // Calculate VSWR from power readings

        swrm.vswr = (1+sqrtf(swrm.rev_pwr/swrm.fwd_pwr))/(1-sqrtf(swrm.rev_pwr/swrm.fwd_pwr));
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
        rfg_calc = (float)ts.rf_codec_gain;     // get copy of RF gain setting
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

    Codec_LineInGainAdj((uint8_t)rfg_calc); // set the RX gain on the codec

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
    vfo[vfo_active].band[ts.band].dial_value = df.tune_old; //band_dial_value[ts.band];     // save "VFO B" settings
    vfo[vfo_active].band[ts.band].decod_mode = ts.dmod_mode;    //band_decod_mode[ts.band];

    df.tune_new = vfo[vfo_new].band[ts.band].dial_value;

    if(ts.dmod_mode != vfo[vfo_new].band[ts.band].decod_mode)
    {
        RadioManagement_SetDemodMode(vfo[vfo_new].band[ts.band].decod_mode);
    }
}
