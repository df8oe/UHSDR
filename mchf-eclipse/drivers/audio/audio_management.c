/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */

#include "uhsdr_board.h"

#include "audio_management.h"
#include "math.h"
#include "audio_driver.h"
#include "softdds.h"
#include "fm_subaudible_tone_table.h"
#include "radio_management.h"

/**
 * Calculate the ALC Decay for the Tx Voice Compressor
 */
void AudioManagement_CalcALCDecay()
{
    // calculate ALC decay (release) time constant - this needs to be moved to its own function (and the one in "ui_menu.c")
    ads.alc_decay = pow10f(-((((float32_t)ts.alc_decay_var)+35.0)/10.0));
}

// make sure the frequencies below match the order of iq_freq_enum_t !

#define AUDIO_M_IQ_ADJUST_INIT(freqVal)         { .freq = freqVal , .adj = { .rx = { IQ_BALANCE_OFF, IQ_BALANCE_OFF },  .tx = { { IQ_BALANCE_OFF, IQ_BALANCE_OFF }, { IQ_BALANCE_OFF,IQ_BALANCE_OFF } } } },

freq_adjust_point_t iq_adjust[IQ_FREQ_NUM+1] =
{
        AUDIO_M_IQ_ADJUST_INIT( 3600000)
        AUDIO_M_IQ_ADJUST_INIT(14100000)
        AUDIO_M_IQ_ADJUST_INIT(21100000)
        AUDIO_M_IQ_ADJUST_INIT(28100000)
        AUDIO_M_IQ_ADJUST_INIT(29650000)
        // this must be last
        AUDIO_M_IQ_ADJUST_INIT(0)
};


static int32_t AudioManagement_GetBalanceValFromStruct(freq_adjust_point_t* point, iq_adjust_params_t what)
{
    int32_t retval = IQ_BALANCE_OFF;
    switch(what)
    {
    case IQ_RX_GAIN:
        retval = point->adj.rx.gain;
        break;
    case IQ_RX_PHASE:
         retval = point->adj.rx.phase;
         break;
    case IQ_TX_TRANS_ON_GAIN:
        retval = point->adj.tx[IQ_TRANS_ON].gain;
        break;
    case IQ_TX_TRANS_OFF_GAIN:
        retval = point->adj.tx[IQ_TRANS_OFF].gain;
        break;
    case IQ_TX_TRANS_ON_PHASE:
        retval = point->adj.tx[IQ_TRANS_ON].phase;
        break;
    case IQ_TX_TRANS_OFF_PHASE:
        retval = point->adj.tx[IQ_TRANS_OFF].phase;
        break;
    }
    return retval;
}


/**
 *
 * @param freq for which the two closest points shall be found
 * @return idx of lower frequency point, please note that the higher frequency point may be the stop value
 */

static uint32_t AudioManagement_GetNextValid(freq_adjust_point_t* points, uint32_t idx,  iq_adjust_params_t what)
{
    // uint32_t idx = 0;
    while (points[idx].freq != 0)
      {
          int32_t val = AudioManagement_GetBalanceValFromStruct(&points[idx], what);
          if (val != IQ_BALANCE_OFF)
          {
              break;
          }
          idx++;
      }
    return idx;
}

static void AudioManagement_FindFreqRange(freq_adjust_point_t* points, uint32_t freq,  iq_adjust_params_t what, float32_t* adj_low_ptr, float32_t* adj_high_ptr, float32_t* freq_low_ptr, float32_t* freq_high_ptr )
{
    // AudioManagement_GetBalanceValFromStruct(&points[idx], what);
    uint32_t idx = AudioManagement_GetNextValid(points, 0, what);
    uint32_t previous_idx = idx;
    uint32_t before_previous_idx = idx;
    uint32_t valid_points = 0;
    bool found_match = false;

    // searching for the frist valid calibration point just higher than then frequency
    while (points[idx].freq != 0)
    {
        valid_points++;
        if (freq < points[idx].freq)
        {
            // we found one valid calibration point which is higher.
            // now we need to see if there is a second point if possible.
            // if the idx is not equal previous_idx, there was a valid lower frequency calibration point
            // which is nice otherwise we try to find one more higher point
            if (previous_idx == idx)
            {
                // we are lower than the lowest entry, now check if there is one more valid calibration point
                idx = AudioManagement_GetNextValid(points, idx+1, what);
                if (points[idx].freq == 0) // stop entry
                {
                    // no more valid points, too bad
                    idx = previous_idx; // we keep both points identical then
                }
            }
            found_match = true;
            break;
        }
        before_previous_idx = previous_idx;
        previous_idx = idx;
        idx = AudioManagement_GetNextValid(points, idx+1, what);
    }

    if (found_match == false)
    {
        if (valid_points > 1)
        {
            // we had two or more valid points, but all are below or equal the frequency
            // just use the last two valid points, idx is not a valid point but the stop entry
            idx = previous_idx;
            previous_idx = before_previous_idx;
        }
        else
        {
            // if we had not a single valid calibration point, too bad
            // idx is same as previous_idx and both point to the last "stop" entry
            // nothing to be done now, but we handle it like the next case, makes no difference

            // we had only one valid point, and that must be in previous_idx
            // no problem, we use it for all frequencies
            idx = previous_idx;
        }
    }

    *adj_high_ptr = AudioManagement_GetBalanceValFromStruct(&points[idx], what);
    *adj_low_ptr = AudioManagement_GetBalanceValFromStruct(&points[previous_idx], what);
    *freq_high_ptr = points[idx].freq;
    *freq_low_ptr = points[previous_idx].freq;

}

static float AudioManagement_CalcAdjustInFreqRangeHelperNew(freq_adjust_point_t* points, iq_adjust_params_t what, float32_t freq, float32_t scaling)
{
    float32_t adj_low, adj_high;
    float32_t freq_low, freq_high;

    AudioManagement_FindFreqRange(points, freq, what, &adj_low, &adj_high, &freq_low, &freq_high);

    if (adj_high == IQ_BALANCE_OFF)
    {
        if (adj_low == IQ_BALANCE_OFF)
        {
            // no data available, set both to zero
            adj_high = adj_low = 0.0;
        }
        else
        {
            // we use the low value for both
            adj_high = adj_low;
        }
    } else if (adj_low == IQ_BALANCE_OFF)
    {
        // we use the high value for both
        adj_low = adj_high;
    }
    float32_t adj_delta =  (freq_high != freq_low)? (adj_high - adj_low) / (freq_high - freq_low) * (freq - freq_low) : 0;


    return (adj_delta + adj_low)/scaling;
    // get current gain adjustment setting  USB and other modes
}

static void AudioManagement_CalcIqGainAdjustVarHelper(volatile iq_float_t* var, float32_t adj)
{
        var->i = 1 + adj;
        var->q = 1 - adj;
}

void AudioManagement_CalcIqPhaseGainAdjust(float freq)
{
    //
    // the phase adjustment is done by mixing a little bit of I into Q or vice versa
    // this is justified because the phase shift between two signals of equal frequency can
    // be regulated by adjusting the amplitudes of the two signals!

    ads.iq_phase_balance_rx = AudioManagement_CalcAdjustInFreqRangeHelperNew(
            iq_adjust,
            IQ_RX_PHASE,
            freq,
            SCALING_FACTOR_IQ_PHASE_ADJUST);


    // please note that the RX adjustments for gain are negative
    // and the adjustments for TX (in the function AudioManagement_CalcTxIqGainAdj) are positive
    float32_t adj_i_rx = -AudioManagement_CalcAdjustInFreqRangeHelperNew(
            iq_adjust,
            IQ_RX_GAIN,
            freq,
            SCALING_FACTOR_IQ_AMPLITUDE_ADJUST);

    AudioManagement_CalcIqGainAdjustVarHelper(&ts.rx_adj_gain_var,adj_i_rx);


    ads.iq_phase_balance_tx[IQ_TRANS_ON] = AudioManagement_CalcAdjustInFreqRangeHelperNew(
            iq_adjust,
            IQ_TX_TRANS_ON_PHASE,
            freq,
            SCALING_FACTOR_IQ_PHASE_ADJUST);
    ads.iq_phase_balance_tx[IQ_TRANS_OFF] = AudioManagement_CalcAdjustInFreqRangeHelperNew(
            iq_adjust,
            IQ_TX_TRANS_OFF_PHASE,
            freq,
            SCALING_FACTOR_IQ_PHASE_ADJUST);

    AudioManagement_CalcIqGainAdjustVarHelper(&ts.tx_adj_gain_var[IQ_TRANS_ON],AudioManagement_CalcAdjustInFreqRangeHelperNew(
            iq_adjust,
            IQ_TX_TRANS_ON_GAIN,
            freq,
            SCALING_FACTOR_IQ_AMPLITUDE_ADJUST));

    AudioManagement_CalcIqGainAdjustVarHelper(
            &ts.tx_adj_gain_var[IQ_TRANS_OFF],
            AudioManagement_CalcAdjustInFreqRangeHelperNew(
                    iq_adjust,
                    IQ_TX_TRANS_OFF_GAIN,
                    freq,
                    SCALING_FACTOR_IQ_AMPLITUDE_ADJUST));


}

//*----------------------------------------------------------------------------
typedef struct AlcParams_s
{
    uint32_t tx_postfilt_gain;
    uint32_t alc_decay;
} AlcParams;

static const AlcParams alc_params[] =
{
    { 1, 15},
    { 2, 12},
    { 4, 10},
    { 6, 9},
    { 7, 8},
    { 8, 7},
	{ 10, 6},
    { 12, 5},
    { 15, 4},
    { 17, 3},
    { 20, 2},
    { 25, 1},
    { 25, 0},
};

/**
 * @brief Set TX audio compression settings (gain and ALC decay rate) based on user setting
 */
void AudioManagement_CalcTxCompLevel()
{
    int16_t tx_comp_level = ts.tx_comp_level;

    if (TX_AUDIO_COMPRESSION_MIN < tx_comp_level && tx_comp_level < TX_AUDIO_COMPRESSION_SV)
    {
        ts.alc_tx_postfilt_gain_var = alc_params[ts.tx_comp_level].tx_postfilt_gain;      // restore "pristine" EEPROM values
        ts.alc_decay_var = alc_params[ts.tx_comp_level].alc_decay;

    }
    else if (tx_comp_level == TX_AUDIO_COMPRESSION_SV)                    // get the speech compressor setting
    {
        // read saved values from EEPROM
        ts.alc_tx_postfilt_gain_var = ts.alc_tx_postfilt_gain;      // restore "pristine" EEPROM values
        ts.alc_decay_var = ts.alc_decay;
    }
    else
    {
        ts.alc_tx_postfilt_gain_var = 4;
        ts.alc_decay_var = 10;
    }
    AudioManagement_CalcALCDecay();
}

#include "fm_subaudible_tone_table.h"
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcSubaudibleGenFreq
//* Object              : Calculate frequency word for subaudible tone generation  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_CalcSubaudibleGenFreq(float32_t freq)
{
    ads.fm_conf.subaudible_tone_gen_freq = freq; // look up tone frequency (in Hz)
    softdds_setFreqDDS(&ads.fm_conf.subaudible_tone_dds, ads.fm_conf.subaudible_tone_gen_freq,ts.samp_rate,false);
}


#define FM_GOERTZEL_HIGH    1.04        // ratio of "high" detect frequency with respect to center
#define FM_GOERTZEL_LOW     0.95        // ratio of "low" detect frequency with respect to center
/**
 * @brief Calculate frequency word for subaudible tone, call after change of detection frequency  [KA7OEI October, 2015]
 */
void AudioManagement_CalcSubaudibleDetFreq(float32_t freq)
{
    const uint32_t size = AUDIO_BLOCK_SIZE;

    ads.fm_conf.subaudible_tone_det_freq = freq;       // look up tone frequency (in Hz)

    if (freq > 0)
    {
        // Calculate Goertzel terms for tone detector(s)
        AudioFilter_CalcGoertzel(&ads.fm_conf.goertzel[FM_HIGH], ads.fm_conf.subaudible_tone_det_freq, FM_SUBAUDIBLE_GOERTZEL_WINDOW*size,FM_GOERTZEL_HIGH, IQ_SAMPLE_RATE);
        AudioFilter_CalcGoertzel(&ads.fm_conf.goertzel[FM_LOW], ads.fm_conf.subaudible_tone_det_freq, FM_SUBAUDIBLE_GOERTZEL_WINDOW*size,FM_GOERTZEL_LOW, IQ_SAMPLE_RATE);
        AudioFilter_CalcGoertzel(&ads.fm_conf.goertzel[FM_CTR], ads.fm_conf.subaudible_tone_det_freq, FM_SUBAUDIBLE_GOERTZEL_WINDOW*size,1.0, IQ_SAMPLE_RATE);
    }
}

uint32_t fm_tone_burst_freq[FM_TONE_BURST_MAX+1] = { 0, 1750, 2135 };

//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiLoadToneBurstMode
//* Object              : Load tone burst mode  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_LoadToneBurstMode()
{
    uint16_t frequency = 0;

    if (ts.fm_tone_burst_mode <= FM_TONE_BURST_MAX)
    {
        frequency = fm_tone_burst_freq[ts.fm_tone_burst_mode];
    }

    softdds_setFreqDDS(&ads.fm_conf.tone_burst_dds, frequency, ts.samp_rate, false);
}

/**
 * @brief Generates the sound for a given beep frequency, call after change of beep freq or before beep  [KA7OEI October, 2015]
 */
void AudioManagement_KeyBeepPrepare()
{
    softdds_setFreqDDS(&ads.beep, ts.beep_frequency,ts.samp_rate,false);

    float32_t calc = (float)(ts.beep_loudness-1);  // range 0-20
    calc /= 2;                          // range 0-10
    calc *= calc;                       // range 0-100
    calc += 3;                          // range 3-103
    ads.beep_loudness_factor = calc / 400;      // range from 0.0075 to 0.2575 - multiplied by DDS output
}

/**
 * @brief Tell audio driver to make beeping sound  [KA7OEI October, 2015]
 */
void AudioManagement_KeyBeep()
{
    if((ts.flags2 & FLAGS2_KEY_BEEP_ENABLE) && (ads.beep.step > 0))      // is beep enabled and frequency non-zero?
    {
        ads.beep.acc = 0; // force reset of accumulator to start at zero to minimize "click" caused by an abrupt voltage transition at startup
        ts.beep_timing = BEEP_DURATION * (IQ_INTERRUPT_FREQ / 100);       // set duration of beep and activate it
    }
}

void AudioManagement_SetSidetoneForDemodMode(uint8_t dmod_mode, bool tune_mode)
{
    float tonefreq[2] = {0.0, 0.0};
    switch(dmod_mode)
    {
    case DEMOD_CW:
        tonefreq[0] = tune_mode?CW_SIDETONE_FREQ_DEFAULT:ts.cw_sidetone_freq;
        break;
    case DEMOD_DIGI:
    	if (ts.digital_mode == DigitalMode_RTTY || ts.digital_mode == DigitalMode_BPSK)
    	{
    		tonefreq[0] = tune_mode?CW_SIDETONE_FREQ_DEFAULT:ts.cw_sidetone_freq;
    	}
    	break;
    default:
        tonefreq[0] = tune_mode?SSB_TUNE_FREQ:0.0;

        if (tune_mode && ts.tune_tone_mode == TUNE_TONE_TWO)
        { // ARRL Standard is 700Hz and 1900Hz
        	// so I temporarily changed this to SSB_TUNE_FREQ + 1200, DD4WH 2016_07_14
        	// --> TWO_TONE = 750Hz and 1950Hz
        	//            tonefreq[1] = tune_mode?(SSB_TUNE_FREQ+600):0.0;
            tonefreq[1] = SSB_TUNE_FREQ+1200;
        }
    }

    softdds_configRunIQ(tonefreq,ts.samp_rate,0);
}
