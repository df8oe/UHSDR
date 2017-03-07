/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */

#include "mchf_board.h"

#include "audio_management.h"
#include "math.h"
#include "audio_driver.h"
#include "softdds.h"
#include "fm_subaudible_tone_table.h"

//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcAGCDecay
//* Object              : Calculate Decay timing for AGC (RECEIVE!)
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//

void AudioManagement_CalcAGCDecay()
{
    switch (ts.agc_mode)
    {
    case AGC_SLOW:
        ads.agc_decay = AGC_SLOW_DECAY;
        break;
    case AGC_FAST:
        ads.agc_decay = AGC_FAST_DECAY;
        break;
    case AGC_CUSTOM:      // calculate custom AGC setting
    {
        ads.agc_decay = powf(10,-((((float32_t)ts.agc_custom_decay)+30.0)/10.0));
    }
    break;
    default:
        ads.agc_decay = AGC_MED_DECAY;
    }
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcALCDecay
//* Object              : Calculate Decay timing for ALC (TRANSMIT!)
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void AudioManagement_CalcALCDecay(void)
{
    // calculate ALC decay (release) time constant - this needs to be moved to its own function (and the one in "ui_menu.c")
    ads.alc_decay = powf(10,-((((float32_t)ts.alc_decay_var)+35.0)/10.0));
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcRFGain
//* Object              : Calculate RF Gain internal value from user setting
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void AudioManagement_CalcRFGain(void)
{
    float tcalc;    // temporary value as "ads.agc_rf_gain" may be used during the calculation!

    // calculate working RF gain value
    tcalc = (float)ts.rf_gain;
    tcalc *= 1.4;
    tcalc -= 20;
    tcalc /= 10;
    ads.agc_rf_gain = powf(10, tcalc);

}

//
//
//*----------------------------------------------------------------------------
//* Function Name       : AudioManagement_CalcAGCVals
//* Object              : Calculate internal AGC values from user settings
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void AudioManagement_CalcAGCVals(void)
{
    float max_rf_gain = 1 + (ts.max_rf_gain <= MAX_RF_GAIN_MAX ? ts.max_rf_gain : MAX_RF_GAIN_DEFAULT);

    ads.agc_knee = AGC_KNEE_REF * max_rf_gain;
    ads.agc_val_max = AGC_VAL_MAX_REF / max_rf_gain;
    ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF / (float)(ts.max_rf_gain + 1);
    // TODO: Why is here always ts.max_rf_gain used? Shouldn't it be max_rf_gain too?
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : AudioManagement_CalcNB_AGC
//* Object              : Calculate Noise Blanker AGC settings
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_CalcNB_AGC(void)
{
    float temp_float;

    temp_float = (float)ts.nb_agc_time_const;   // get user setting
    temp_float = NB_MAX_AGC_SETTING-temp_float;     // invert (0 = minimum))
    temp_float /= 1.1;                              // scale calculation
    temp_float *= temp_float;                       // square value
    temp_float += 1;                                // offset by one
    temp_float /= 44000;                            // rescale
    temp_float += 1;                                // prevent negative log result
    ads.nb_sig_filt = log10f(temp_float);           // de-linearize and save in "new signal" contribution parameter
    ads.nb_agc_filt = 1 - ads.nb_sig_filt;          // calculate parameter for recyling "old" AGC value
}
//

static float AudioManagement_CalcAdjustInFreqRangeHelper(float32_t adj_low, float32_t adj_high, float32_t freq, float32_t scaling)
{
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
    return ((adj_high - adj_low) / (28100000.0 - 3600000.0) * (freq - 3600000.0) + adj_low)/scaling;        // get current gain adjustment setting  USB and other modes
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

    ads.iq_phase_balance_rx = AudioManagement_CalcAdjustInFreqRangeHelper(
            ts.rx_iq_phase_balance[IQ_80M].value[IQ_TRANS_ON],
            ts.rx_iq_phase_balance[IQ_10M].value[IQ_TRANS_ON],
            freq,
            SCALING_FACTOR_IQ_PHASE_ADJUST);


    // please note that the RX adjustments for gain are negative
    // and the adjustments for TX (in the function AudioManagement_CalcTxIqGainAdj) are positive
    float32_t adj_i_rx = AudioManagement_CalcAdjustInFreqRangeHelper(
            -ts.rx_iq_gain_balance[IQ_80M].value[IQ_TRANS_ON],
            -ts.rx_iq_gain_balance[IQ_10M].value[IQ_TRANS_ON],
            freq,
            SCALING_FACTOR_IQ_AMPLITUDE_ADJUST);

    for (int i = 0; i < IQ_TRANS_NUM; i++)
    {
        ads.iq_phase_balance_tx[i] = AudioManagement_CalcAdjustInFreqRangeHelper(
                    ts.tx_iq_phase_balance[IQ_80M].value[i],
                    ts.tx_iq_phase_balance[IQ_10M].value[i],
                    freq,
                    SCALING_FACTOR_IQ_PHASE_ADJUST);

        float32_t adj_i_tx= AudioManagement_CalcAdjustInFreqRangeHelper(
                ts.tx_iq_gain_balance[IQ_80M].value[i],
                ts.tx_iq_gain_balance[IQ_10M].value[i],
                freq,
                SCALING_FACTOR_IQ_AMPLITUDE_ADJUST);

        AudioManagement_CalcIqGainAdjustVarHelper(&ts.tx_adj_gain_var[i],adj_i_tx);
    }

    AudioManagement_CalcIqGainAdjustVarHelper(&ts.rx_adj_gain_var,adj_i_rx);

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
void AudioManagement_CalcSubaudibleGenFreq(void)
{
    ads.fm_subaudible_tone_gen_freq = fm_subaudible_tone_table[ts.fm_subaudible_tone_gen_select];       // look up tone frequency (in Hz)
    ads.fm_subaudible_tone_word = (ulong)(ads.fm_subaudible_tone_gen_freq * FM_SUBAUDIBLE_TONE_WORD_CALC_FACTOR);   // calculate tone word
}

static void AudioManagement_CalcGoertzel(volatile Goertzel* gv, const uint32_t size, const float goertzel_coeff)
{
    // FIXME: Move the Goertzel data structures out of the volatile AudioDriverState ads or make ads no longer volatile
    Goertzel *g = (Goertzel*)gv;
    g->a = (0.5 + (ads.fm_subaudible_tone_det_freq * goertzel_coeff) * FM_SUBAUDIBLE_GOERTZEL_WINDOW * (size/2)/IQ_SAMPLE_RATE);
    g->b = (2*PI*g->a)/(FM_SUBAUDIBLE_GOERTZEL_WINDOW*size/2);
    g->sin = sin(g->b);
    g->cos = cos(g->b);
    g->r = 2 * g->cos;
}

/**
 * @brief Calculate frequency word for subaudible tone, call after change of detection frequency  [KA7OEI October, 2015]
 */
void AudioManagement_CalcSubaudibleDetFreq(void)
{
    const uint32_t size = BUFF_LEN;

    ads.fm_subaudible_tone_det_freq = fm_subaudible_tone_table[ts.fm_subaudible_tone_det_select];       // look up tone frequency (in Hz)

    // Calculate Goertzel terms for tone detector(s)
    AudioManagement_CalcGoertzel(&ads.fm_goertzel[FM_HIGH],size,FM_GOERTZEL_HIGH);
    AudioManagement_CalcGoertzel(&ads.fm_goertzel[FM_LOW],size,FM_GOERTZEL_LOW);
    AudioManagement_CalcGoertzel(&ads.fm_goertzel[FM_CTR],size,1.0);
}

//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiLoadToneBurstMode
//* Object              : Load tone burst mode  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_LoadToneBurstMode(void)
{
    switch(ts.fm_tone_burst_mode)
    {
    case FM_TONE_BURST_1750_MODE:
        ads.fm_tone_burst_word = FM_TONE_BURST_1750;
        break;
    case FM_TONE_BURST_2135_MODE:
        ads.fm_tone_burst_word = FM_TONE_BURST_2135;
        break;
    default:
        ads.fm_tone_burst_word = 0;
        break;
    }

}

/**
 * @brief Generates the sound for a given beep frequency, call after change of beep freq or before beep  [KA7OEI October, 2015]
 */
void AudioManagement_LoadBeepFreq()
{
    float calc;

    if(ts.flags2 & FLAGS2_KEY_BEEP_ENABLE)      // is beep enabled?
    {
        softdds_setfreq(&ads.beep, ts.beep_frequency,ts.samp_rate,false);
    }
    else
    {
        softdds_setfreq(&ads.beep, 0,ts.samp_rate,true); // not enabled - zero out frequency word
    }

    calc = (float)ts.beep_loudness;     // range 0-20
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
    // FIXME: Do we really need to call this here, or shall we rely on the calls
    // made when changing the freq/settings?
    // right now every beep runs the generator code
    AudioManagement_LoadBeepFreq();       // load and calculate beep frequency
    ts.beep_timing = ts.sysclock + BEEP_DURATION;       // set duration of beep
    ts.beep_active = 1;                                 // activate tone
}

void AudioManagement_SetSidetoneForDemodMode(uint16_t dmod_mode, bool tune_mode)
{
    float tonefreq[2] = {0.0, 0.0};
    switch(dmod_mode)
    {
    case DEMOD_CW:
        tonefreq[0] = tune_mode?CW_SIDETONE_FREQ_DEFAULT:ts.sidetone_freq;
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

    softdds_setfreq_dbl(tonefreq,ts.samp_rate,0);
}
