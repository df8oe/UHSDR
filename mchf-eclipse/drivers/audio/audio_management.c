/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */

#include "mchf_board.h"

#include "arm_math.h"
#include "math.h"
#include "audio_driver.h"

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

void AudioManagement_CalcAGCDecay(void)
{
    float tcalc;    // temporary holder - used to avoid conflict during operation

    // Set AGC rate - this needs to be moved to its own function (and the one in "ui_menu.c")
    //
    if(ts.agc_mode == AGC_SLOW)
        ads.agc_decay = AGC_SLOW_DECAY;
    else if(ts.agc_mode == AGC_FAST)
        ads.agc_decay = AGC_FAST_DECAY;
    else if(ts.agc_mode == AGC_CUSTOM)  {   // calculate custom AGC setting
        tcalc = (float)ts.agc_custom_decay;
        tcalc += 30;
        tcalc /= 10;
        tcalc = -tcalc;
        ads.agc_decay = powf(10, tcalc);
    }
    else
        ads.agc_decay = AGC_MED_DECAY;
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
    float tcalc;    // temporary holder - used to avoid conflict during operation

    // calculate ALC decay (release) time constant - this needs to be moved to its own function (and the one in "ui_menu.c")
    //
    tcalc = (float)ts.alc_decay_var;
    tcalc += 35;
    tcalc /= 10;
    tcalc *= -1;
    ads.alc_decay = powf(10, tcalc);
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
    if(ts.max_rf_gain <= MAX_RF_GAIN_MAX)   {
        ads.agc_knee = AGC_KNEE_REF * (float)(ts.max_rf_gain + 1);
        ads.agc_val_max = AGC_VAL_MAX_REF / ((float)(ts.max_rf_gain + 1));
        ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF / (float)(ts.max_rf_gain + 1);
    }
    else    {
        ads.agc_knee = AGC_KNEE_REF * MAX_RF_GAIN_DEFAULT+1;
        ads.agc_val_max = AGC_VAL_MAX_REF / MAX_RF_GAIN_DEFAULT+1;
        ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF /  (float)(ts.max_rf_gain + 1);
    }
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
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcRxIqGainAdj
//* Object              : Calculate RX IQ Gain adjustments
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_CalcRxIqGainAdj(void)
{
    if(ts.dmod_mode == DEMOD_AM)
        ts.rx_adj_gain_var_i = (float)ts.rx_iq_am_gain_balance;         // get current gain adjustment for AM
    if(ts.dmod_mode == DEMOD_FM)
        ts.rx_adj_gain_var_i = (float)ts.rx_iq_fm_gain_balance;         // get current gain adjustment for FM
    else if(ts.dmod_mode == DEMOD_LSB)
        ts.rx_adj_gain_var_i = (float)ts.rx_iq_lsb_gain_balance;        // get current gain adjustment setting for LSB
    else
        ts.rx_adj_gain_var_i = (float)ts.rx_iq_usb_gain_balance;        // get current gain adjustment setting  USB and other modes
    //
    ts.rx_adj_gain_var_i /= SCALING_FACTOR_IQ_AMPLITUDE_ADJUST;       // fractionalize it
    ts.rx_adj_gain_var_q = -ts.rx_adj_gain_var_i;               // get "invert" of it
    ts.rx_adj_gain_var_i += 1;      // offset it by one (e.g. 0 = unity)
    ts.rx_adj_gain_var_q += 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcTxIqGainAdj
//* Object              : Calculate TX IQ Gain adjustments
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_CalcTxIqGainAdj(void)
{
    // Note:  There is a fixed amount of offset due to the fact that the phase-added Hilbert (e.g. 0, 90) transforms are
    // slightly asymmetric that is added so that "zero" is closer to being the proper phase balance.
    //
    if(ts.dmod_mode == DEMOD_AM)    // is it AM mode?
        ts.tx_adj_gain_var_i = (float)ts.tx_iq_am_gain_balance;     // get current gain balance adjustment setting for AM
    else if(ts.dmod_mode == DEMOD_FM)   // is it in FM mode?
        ts.tx_adj_gain_var_i = (float)ts.tx_iq_fm_gain_balance;     // get current gain balance adjustment setting for FM
    else if(ts.dmod_mode == DEMOD_LSB)
        ts.tx_adj_gain_var_i = (float)ts.tx_iq_lsb_gain_balance;        // get current gain balance adjustment setting for LSB
    else
        ts.tx_adj_gain_var_i = (float)ts.tx_iq_usb_gain_balance;        // get current gain adjustment setting for USB and other non AM/FM modes

    //
    ts.tx_adj_gain_var_i /= SCALING_FACTOR_IQ_AMPLITUDE_ADJUST;       // fractionalize it
    ts.tx_adj_gain_var_q = -ts.tx_adj_gain_var_i;               // get "invert" of it
    ts.tx_adj_gain_var_i += 1;      // offset it by one (e.g. 0 = unity)
    ts.tx_adj_gain_var_q += 1;
}

//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcTxCompLevel
//* Object              : Set TX audio compression settings (gain and ALC decay rate) based on user setting
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
typedef struct AlcParams_s {
    uint32_t tx_postfilt_gain;
    uint32_t alc_decay;
} AlcParams;

static const AlcParams alc_params[] = {
    { 1, 15},
    { 2, 12},
    { 4, 10},
    { 6, 9},
    { 7, 7},
    { 10, 6},
    { 12, 5},
    { 15, 4},
    { 17, 3},
    { 20, 2},
    { 25, 1},
    { 25, 0},
};

void AudioManagement_CalcTxCompLevel(void)
{
    float tcalc;
    if (ts.tx_comp_level < 13)
    {
      ts.alc_tx_postfilt_gain_var = alc_params[ts.tx_comp_level].tx_postfilt_gain;      // restore "pristine" EEPROM values
      ts.alc_decay_var = alc_params[ts.tx_comp_level].alc_decay;

    } else if (ts.tx_comp_level == 13)  {               // get the speech compressor setting
        // read saved values from EEPROM
            ts.alc_tx_postfilt_gain_var = ts.alc_tx_postfilt_gain;      // restore "pristine" EEPROM values
            ts.alc_decay_var = ts.alc_decay;
    } else {
            ts.alc_tx_postfilt_gain_var = 4;
            ts.alc_decay_var = 10;
    }
    //
    tcalc = (float)ts.alc_decay_var;    // use temp var "tcalc" as audio function
    tcalc += 35;            // can be called mid-calculation!
    tcalc /= 10;
    tcalc *= -1;
    tcalc = powf(10, tcalc);
    ads.alc_decay = tcalc;
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
//
// TODO: MOVE TO AUDIO / RF Function
//*----------------------------------------------------------------------------
//* Function Name       : UiCalcSubaudibleDetFreq
//* Object              : Calculate frequency word for subaudible tone  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_CalcSubaudibleDetFreq(void)
{
    ulong size;

    size = BUFF_LEN;

    ads.fm_subaudible_tone_det_freq = fm_subaudible_tone_table[ts.fm_subaudible_tone_det_select];       // look up tone frequency (in Hz)
    //
    // Calculate Goertzel terms for tone detector(s)
    //
    // Terms for "above" detection frequency
    //
    ads.fm_goertzel_high_a = (0.5 + (ads.fm_subaudible_tone_det_freq * FM_GOERTZEL_HIGH) * FM_SUBAUDIBLE_GOERTZEL_WINDOW * (size/2)/48000);
    ads.fm_goertzel_high_b = (2*PI*ads.fm_goertzel_high_a)/(FM_SUBAUDIBLE_GOERTZEL_WINDOW*size/2);
    ads.fm_goertzel_high_sin = sin(ads.fm_goertzel_high_b);
    ads.fm_goertzel_high_cos = cos(ads.fm_goertzel_high_b);
    ads.fm_goertzel_high_r = 2 * ads.fm_goertzel_high_cos;
    //
    // Terms for "below" detection frequency
    //
    ads.fm_goertzel_low_a = (0.5 + (ads.fm_subaudible_tone_det_freq * FM_GOERTZEL_LOW) * FM_SUBAUDIBLE_GOERTZEL_WINDOW * (size/2)/48000);
    ads.fm_goertzel_low_b = (2*PI*ads.fm_goertzel_low_a)/(FM_SUBAUDIBLE_GOERTZEL_WINDOW*size/2);
    ads.fm_goertzel_low_sin = sin(ads.fm_goertzel_low_b);
    ads.fm_goertzel_low_cos = cos(ads.fm_goertzel_low_b);
    ads.fm_goertzel_low_r = 2 * ads.fm_goertzel_low_cos;
    //
    // Terms for the actual detection frequency
    //
    ads.fm_goertzel_ctr_a = (0.5 + ads.fm_subaudible_tone_det_freq * FM_SUBAUDIBLE_GOERTZEL_WINDOW * (size/2)/48000);
    ads.fm_goertzel_ctr_b = (2*PI*ads.fm_goertzel_ctr_a)/(FM_SUBAUDIBLE_GOERTZEL_WINDOW*size/2);
    ads.fm_goertzel_ctr_sin = sin(ads.fm_goertzel_ctr_b);
    ads.fm_goertzel_ctr_cos = cos(ads.fm_goertzel_ctr_b);
    ads.fm_goertzel_ctr_r = 2 * ads.fm_goertzel_ctr_cos;
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
    switch(ts.fm_tone_burst_mode)   {
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
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiLoadBeepFreq
//* Object              : Load beep frequency  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_LoadBeepFreq(void)
{
    float calc;

    if(ts.flags2 & FLAGS2_KEY_BEEP_ENABLE)  {   // is beep enabled?
        ads.beep_word = ts.beep_frequency * 65536;      // yes - calculated/load frequency
        ads.beep_word /= 48000;
    }
    else
        ads.beep_word = 0;      // not enabled - zero out frequency word
    //
    calc = (float)ts.beep_loudness;     // range 0-20
    calc /= 2;                          // range 0-10
    calc *= calc;                       // range 0-100
    calc += 3;                          // range 3-103
    ads.beep_loudness_factor = calc / 400;      // range from 0.0075 to 0.2575 - multiplied by DDS output
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiKeyBeep
//* Object              : Make beep  [KA7OEI October, 2015]
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioManagement_KeyBeep(void)
{
    AudioManagement_LoadBeepFreq();       // load and calculate beep frequency
    ts.beep_timing = ts.sysclock + BEEP_DURATION;       // set duration of beep
    ts.beep_active = 1;                                 // activate tone
}
