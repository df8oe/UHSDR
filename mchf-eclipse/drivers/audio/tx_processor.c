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
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/
#include <assert.h>
#include "uhsdr_board.h"
#include "profiling.h"

#include "audio_driver.h"
#include "radio_management.h"
#include "audio_management.h" // only for AudioManagement_CalcALCDecay
#include "rtty.h"
#include "psk.h"
#include "freedv_uhsdr.h"
#include "freq_shift.h"
#include "cw_gen.h"

#include "usbd_audio_if.h"


#include "filters.h"
#include "uhsdr_math.h"
#include "tx_processor.h"
#include "fm_subaudible_tone_table.h"


#define IIR_TX_STATE_ARRAY_SIZE    (IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES_MAX)

// variables for TX IIR filter
static float32_t       iir_tx_state[IIR_TX_STATE_ARRAY_SIZE];
static arm_iir_lattice_instance_f32    IIR_TXFilter;


// variables for TX bass & treble adjustment IIR biquad filter
static arm_biquad_casd_df1_inst_f32 IIR_TX_biquad =
{
        .numStages = 3,
        .pCoeffs = (float32_t *)(float32_t [])
        {
            1,0,0,0,0,  1,0,0,0,0,  1,0,0,0,0
        }, // 3 x 5 = 15 coefficients

        .pState = (float32_t *)(float32_t [])
        {
            0,0,0,0,   0,0,0,0,   0,0,0,0
        } // 3 x 4 = 12 state variables
};

static float32_t   __MCHF_SPECIALMEM audio_delay_buffer    [AUDIO_DELAY_BUFSIZE];

/**
 * This runs the preparation directly before going into transmit (runs in interrupt!)
 * Keep as short as possible
 */
void TxProcessor_PrepareRun()
{
    arm_fill_f32(0, audio_delay_buffer, AUDIO_DELAY_BUFSIZE);
}

/**
 * Must be called before going into transmit with a given mode, sets up filters correctly for that mode.
 * @param dmod_mode
 */
void TxProcessor_Set(uint8_t dmod_mode)
{

    ads.tx_filter_adjusting++;        // disable TX filtering during adjustment
    float32_t coeffs[5];

    // coefficient calculation for TX bass & treble adjustment
    // the TX treble filter is in IIR_TX_biquad and works at 48000ksps
    AudioDriver_CalcHighShelf(coeffs, 1700, 0.9, ts.dsp.tx_treble_gain, AUDIO_SAMPLE_RATE);
    AudioDriver_SetBiquadCoeffs(&IIR_TX_biquad.pCoeffs[0],coeffs);

    // the TX bass filter is in IIR_TX_biquad and works at 48000 sample rate
    AudioDriver_CalcLowShelf(coeffs, 300, 0.7, ts.dsp.tx_bass_gain, AUDIO_SAMPLE_RATE);
    AudioDriver_SetBiquadCoeffs(&IIR_TX_biquad.pCoeffs[5],coeffs);

    // Init TX audio filter - Do so "manually" since built-in init functions don't work with CONST coefficients
    const arm_iir_lattice_instance_f32* IIR_TXFilterSelected_ptr;

    if(dmod_mode != DEMOD_FM)                           // not FM - use bandpass filter that restricts low and, stops at 2.7 kHz
    {
        switch(ts.tx_filter)
        {
        case TX_FILTER_BASS:
            IIR_TXFilterSelected_ptr = &IIR_TX_WIDE_BASS;
            break;
        case TX_FILTER_TENOR:
            IIR_TXFilterSelected_ptr = &IIR_TX_WIDE_TREBLE;
            break;
        default:
            IIR_TXFilterSelected_ptr = &IIR_TX_SOPRANO;
        }
    }
    else        // This is FM - use a filter with "better" lows and highs more appropriate for FM
    {
        IIR_TXFilterSelected_ptr = &IIR_TX_2k7_FM;
    }

    arm_iir_lattice_init_f32(&IIR_TXFilter,
            IIR_TXFilterSelected_ptr->numStages,       // number of stages
            IIR_TXFilterSelected_ptr->pkCoeffs, // point to reflection coefficients
            IIR_TXFilterSelected_ptr->pvCoeffs, // point to ladder coefficients
            IIR_TXFilter.pState = iir_tx_state,
            IIR_RXAUDIO_BLOCK_SIZE);

    AudioFilter_SetTxHilbertFIR();

    ads.tx_filter_adjusting--;        // enable TX filtering after adjustment
}

/**
 * One-time init of the FM modulator (transmission side)
 * @param fm
 */
static void TxProcessor_FM_Init(fm_conf_t* fm)
{
     fm->tone_burst_active = false;       // this is TRUE of the tone burst is actively being generated
     AudioManagement_CalcSubaudibleGenFreq(fm_subaudible_tone_table[ts.fm_subaudible_tone_gen_select]);        // TX load/set current FM subaudible tone settings for generation
     AudioManagement_LoadToneBurstMode(); // TX load/set tone burst frequency
}
/**
 * All one-time initialization goes here
 * The configuration has already been loaded
 */
void TxProcessor_Init()
{
    ads.alc_val = 1;            // init TX audio auto-level-control (ALC)
    AudioManagement_CalcTxCompLevel();      // calculate current settings for TX speech compressors

    // Initialize CW generator/keyer
    CwGen_Init(); // TX

    TxProcessor_FM_Init(&ads.fm_conf);
 }



#if defined(UI_BRD_OVI40)
/**
 * Used only on the OVI40, there is no way on the MCHF to add the sidetone.
 * should be called the last in the chain.
 * ( src buffer should be not cleared or tremendously modified by previous stages)
 */
static void TxProcessor_FillSideToneAudioBuffer(audio_block_t const src, AudioSample_t* const dst, int16_t blockSize, float32_t gain, bool is_signal_active )
{
    float32_t final_gain = is_signal_active == true ? (gain * AUDIO_BIT_SCALE_UP) : 0;
    for( uint32_t i = 0; i < blockSize; i++ )
    {
        // we don't need correctHalfWord here because the H7 & F7 processors
        // handled 32 bit I2S just fine
        dst[i].r = dst[i].l = (src[i] * final_gain);
    }
}
#endif

/**
 * @brief audio speech compressor (look-ahead type) by KA7OEI
 * @param buffer input (and output) buffer for audio samples
 * @param blockSize number of samples to process
 * @param gain_scaling scaling applied to buffer
 *
 */
static void TxProcessor_VoiceCompressor(audio_block_t a_block, int16_t blockSize, float gain_scaling)
{
    static uint32_t alc_delay_inbuf = 0, alc_delay_outbuf;

    if (ts.tx_comp_level > -1)
    {
        if(!ts.tune)        // do post-filter gain calculations if we are NOT in TUNE mode
        {
            // perform post-filter gain operation
            // this is part of the compression
            float32_t gain_calc = ((float32_t)ts.alc_tx_postfilt_gain_var)/2.0 +0.5 ;

            // get post-filter gain setting
            // offset it so that 2 = unity
            arm_scale_f32(a_block, gain_calc, a_block, blockSize);      // use optimized function to apply scaling to I/Q buffers
        }

        // Do ALC processing on audio buffer - look-ahead type by KA7OEI
        if (false)
        {
            arm_fill_f32(ALC_VAL_MAX * gain_scaling, adb.agc_valbuf, blockSize);
        }
        else
        {
            for(uint16_t i = 0; i < blockSize; i++)
            {
                // perform ALC on post-filtered audio (You will notice the striking similarity to the AGC code!)

                // calculate current level by scaling it with ALC value
                float32_t alc_var = fabsf(a_block[i] * ads.alc_val)/ALC_KNEE - 1.0; // calculate difference between ALC value and "knee" value
                if(alc_var < 0)     // is audio below ALC "knee" value?
                {
                    // alc_var is a negative value, so the resulting expression is negative
                    // but we want to increase the alc_val -> we subtract it
                    ads.alc_val -= ads.alc_val * ads.alc_decay * alc_var;   // (ALC DECAY) Yes - Increase gain slowly
                }
                else
                {
                    // alc_var is a positive value
                    ads.alc_val -= ads.alc_val * ALC_ATTACK * alc_var;  // Fast attack to increase gain
                    if(ads.alc_val < ALC_VAL_MIN)   // Prevent zero or "negative" gain values
                    {
                        ads.alc_val = ALC_VAL_MIN;
                    }
                }
                if(ads.alc_val > ALC_VAL_MAX)   // limit to fixed values within the code
                {
                    ads.alc_val = ALC_VAL_MAX;
                }

                adb.agc_valbuf[i] = (ads.alc_val * gain_scaling);   // store in "running" ALC history buffer for later application to audio data
            }
        }

        // Delay the post-ALC audio slightly so that the ALC's "attack" will very slightly lead the audio being acted upon by the ALC.
        // This eliminates a "click" that can occur when a very strong signal appears due to the ALC lag.  The delay is adjusted based on
        // decimation rate so that it is constant for all settings.

        // Update the in/out pointers to the ALC delay buffer
        alc_delay_inbuf += blockSize;
        alc_delay_outbuf = alc_delay_inbuf + blockSize;
        alc_delay_inbuf %= AUDIO_DELAY_BUFSIZE;
        alc_delay_outbuf %= AUDIO_DELAY_BUFSIZE;

        arm_copy_f32(a_block, &audio_delay_buffer[alc_delay_inbuf], blockSize);  // put new data into the delay buffer
        arm_copy_f32(&audio_delay_buffer[alc_delay_outbuf], a_block, blockSize); // take old data out of the delay buffer

        arm_mult_f32(a_block, adb.agc_valbuf, a_block, blockSize);        // Apply ALC gain corrections to TX audio channels
    }
}



/**
 * Runs the morse keyer input handling for digital mode text input, generates "sidetone".
 *
 * @param a_blocks output audio signal
 * @param blockSize number of samples to generate
 */
static void TxProcessor_CwInputForDigitalModes(audio_block_t* a_blocks, uint16_t blockSize)
{
    // remove noise if no CW is keyed
    memset ( a_blocks[1], 0, sizeof( a_blocks[1][0]) * blockSize );
    if ( ts.cw_keyer_mode != CW_KEYER_MODE_STRAIGHT )
    {
        CwGen_Process ( a_blocks[1], a_blocks[1], blockSize );
#if defined(UI_BRD_OVI40)
        /*
         * We are scaling down RTTY and BPSK "tone" do not shadow CW sidetone
         * CW tone should has more volume than RTTY and BPSK
         */
        // FIXME if we leave two signals together, level of RTTY should be adjusted somehow.
        // now it's scaled down to really low level.
        arm_scale_f32 ( a_blocks[0], 0.05, a_blocks[0], blockSize );
        arm_add_f32 ( a_blocks[0], a_blocks[1], a_blocks[0], blockSize );
#endif // UI_BRD_OVI40
    }
}

/**
 * @brief Equalize based on band and simultaneously apply I/Q gain AND phase adjustments
 * The input is IQ data in the adb.iq.i_buffer and adb.iq.q_buffer
 *
 * @param dst output buffer for generated IQ audio samples
 * @param blockSize number of samples (input/output)
 * @param swap i and q buffers meaning
 * @param scaling gain applied to the samples before conversion to integer data
 *
 */
static void TxProcessor_IqFinalProcessing(float32_t scaling, bool swap, iq_buffer_t* iq_buf_p, IqSample_t* const dst, const uint16_t blockSize)
{
    int16_t trans_idx;

    scaling *= IQ_BIT_SCALE_UP;
    // this aligns resulting signal with 16 or 32 bit width integer MSB, see comment in
    // the first part of AudioDriver_RxProcessor

    if (RadioManagement_IsTxAtZeroIF(ts.dmod_mode, ts.digital_mode) || ts.iq_freq_mode == FREQ_IQ_CONV_MODE_OFF)
    {
        trans_idx = IQ_TRANS_OFF;
    }
    else
    {
        trans_idx = IQ_TRANS_ON;
    }

    float32_t *final_i_buffer, *final_q_buffer;

    float32_t final_i_gain = ts.tx_power_factor * ts.tx_adj_gain_var[trans_idx].i * scaling;
    float32_t final_q_gain = ts.tx_power_factor * ts.tx_adj_gain_var[trans_idx].q * scaling;

    // Output I and Q as stereo data
    if(swap == false)               // the resulting iq is "identical" to the original iq
    {
        final_i_buffer = iq_buf_p->i_buffer;
        final_q_buffer = iq_buf_p->q_buffer;
    }
    else        // the resulting output is mirrored at the center frequency (e.g. a signal 7 khz above center will be then 7khz below center AND mirrored, i.e. switch sidebands if it is not symmetrical)
    {
        final_i_buffer = iq_buf_p->q_buffer;
        final_q_buffer = iq_buf_p->i_buffer;
    }

    // this is the IQ gain / amplitude adjustment
    arm_scale_f32(final_i_buffer, final_i_gain, final_i_buffer, blockSize);
    arm_scale_f32(final_q_buffer, final_q_gain, final_q_buffer, blockSize);

    // this is the IQ phase adjustment
    AudioDriver_IQPhaseAdjust(ts.txrx_mode, final_i_buffer, final_q_buffer,blockSize);
    for(int i = 0; i < blockSize; i++)
    {
        // Prepare data for DAC
        dst[i].l = I2S_correctHalfWord(final_i_buffer[i]); // save left channel
        dst[i].r = I2S_correctHalfWord(final_q_buffer[i]); // save right channel
    }

}

/**
 * Fills the analog buffer with data from DMA buffer or USB or tune signal generator, depending on the TRX settings
 *
 * @param a_buffer audio output buffer
 * @param src  input samples from DMA buffer
 * @param blockSize audio input block size
 */
static void TxProcessor_AudioBufferFill(audio_block_t a_block, AudioSample_t * const src, int16_t blockSize)
{
    const uint8_t tx_audio_source = ts.tx_audio_source;


    if(ts.tune)     // TUNE mode?  If so, generate tone so we can adjust TX IQ phase and gain
    {
        softdds_runIQ(a_block, a_block, blockSize);     // load audio buffer with the tone - DDS produces quadrature channels, but we need only one
    }
    else
    {
        float32_t           gain_calc;

        switch(tx_audio_source)
        {
        case TX_AUDIO_LINEIN_L:
        case TX_AUDIO_LINEIN_R:      // Are we in LINE IN mode?
        {
            gain_calc = LINE_IN_GAIN_RESCALE;           // Yes - fixed gain scaling for line input - the rest is done in hardware
        }
        break;
        case TX_AUDIO_MIC:
        {
            gain_calc = ts.tx_mic_gain_mult;     // We are in MIC In mode:  Calculate Microphone gain
            gain_calc /= MIC_GAIN_RESCALE;       // rescale microphone gain to a reasonable range
	    if(ts.tx_mic_boost > 0)
	    { // value is either 14 dB (g=25.1) or 0 dB (g=1) right now
	      gain_calc += 25.1;
	    }
        }
        break;
        case TX_AUDIO_DIG:
        {
            gain_calc = 1; // since we later scale audio down, we need to scale it up here
        }
        break;
        default:
        {
            gain_calc = 1;
        }
        }

        gain_calc *= AUDIO_BIT_SCALE_DOWN;

        if(tx_audio_source == TX_AUDIO_LINEIN_R)         // Are we in LINE IN RIGHT CHANNEL mode?
        {
            // audio buffer with right sample channel
            for(int i = 0; i < blockSize; i++)
            {
                a_block[i] = I2S_correctHalfWord(src[i].r);
            }
        } else {
            // audio buffer with left sample channel
            for(int i = 0; i < blockSize; i++)
            {
                a_block[i] = I2S_correctHalfWord(src[i].l);
            }
        }

        if (gain_calc != 1.0)
        {
            arm_scale_f32(a_block, gain_calc, a_block, blockSize);  // apply gain
        }

        ads.peak_audio = Math_absmax(a_block, blockSize);
    }
}

/**
 * Filters audio before IQ is generated from
 *
 * @param do_bandpass Run a bandwidth limiting (2.7k) bandpass on the signal
 * @param do_bass_treble Run voice bass and treble adjustments
 * @param inBlock input audio
 * @param outBlock output audio, may be identical to input
 * @param blockSize number of samples to process
 */
static void TxProcessor_FilterAudio(bool do_bandpass, bool do_bass_treble, float32_t* inBlock, float32_t* outBlock, const uint16_t blockSize)
{
    if (do_bandpass)
    {
        arm_iir_lattice_f32(&IIR_TXFilter, inBlock, outBlock, blockSize);
    }

    // if the filters are being adjusted, we simply skip them for that brief period
    if (do_bass_treble && ads.tx_filter_adjusting == false)
    {
        // biquad filter for bass & treble --> NOT enabled when using USB Audio (eg. for Digimodes)
        arm_biquad_cascade_df1_f32 (&IIR_TX_biquad, outBlock,outBlock, blockSize);
    }
}

/**
 * Runs extraction, filtering and voice compression on audio input, outputs the analog samples ready for modulation, useful for voice modes
 *
 * @param a_buffer audio output buffer
 * @param src  input samples from DMA buffer
 * @param blockSize audio input block size
 * @param gain is applied to output data in order to provide properly scaled input for more
 * @param runFilter shall the audio voice bandpass filter be applied
 */
static void TxProcessor_PrepareVoice(audio_block_t a_buffer, AudioSample_t* src, size_t blockSize, float32_t gain, bool runFilter)
{
    TxProcessor_AudioBufferFill(a_buffer, src,blockSize);

    if (!ts.tune)
    {
        TxProcessor_FilterAudio(runFilter, ts.tx_audio_source != TX_AUDIO_DIG, a_buffer, a_buffer, blockSize);
    }

    TxProcessor_VoiceCompressor(a_buffer, blockSize, gain);  // Do the TX ALC and speech compression/processing
}

/**
 *
 * takes audio samples in adb.a_buffer[0] and
 * produces SSB in adb.iq.i_buffer/adb.iq.q_buffer which is then
 * transferred into the DMA buffers after final processing
 * audio samples should filtered before passed in here if necessary
 *
 * @param a_block       audio input buffer (not changed)
 * @param iq_buf        iq output buffer
 * @param blockSize     size of iq / audio buffers
 * @param translate_freq signed frequency shift in Hertz
 * @param is_lsb        is the input to be translated into lsb or usb
 * @return always true, to indicate permanent signal generation
 */

static bool TxProcessor_SSB(audio_block_t a_block, iq_buffer_t* iq_buf_p, const uint16_t blockSize, const int32_t translate_freq, const bool is_lsb)
{
    bool retval = false;

    if ( ads.tx_filter_adjusting == 0 )         // disable TX filtering during adjustment
    {
        // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
        // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
        // + 0 deg to I data
        // depending if we want to do LSB or USB we swap the filters
        arm_fir_instance_f32* i_filter = is_lsb? &Fir_Tx_Hilbert_Q : &Fir_Tx_Hilbert_I;
        arm_fir_instance_f32* q_filter = is_lsb? &Fir_Tx_Hilbert_I : &Fir_Tx_Hilbert_Q;

        arm_fir_f32(i_filter, a_block, iq_buf_p->i_buffer, blockSize);
        arm_fir_f32(q_filter, a_block, iq_buf_p->q_buffer, blockSize);

        if(translate_freq != 0)
        {
            FreqShift(iq_buf_p->i_buffer, iq_buf_p->q_buffer, blockSize, translate_freq);
        }
        retval = true;
    }
    return retval;
}

//
// FM Modulator parameters
#define FM_TX_HPF_ALPHA     0.05            // For FM modulator:  "Alpha" (high-pass) factor to pre-emphasis

// NOTE:  FM_MOD_SCALING_2K5 is rescaled (doubled) for 5 kHz deviation, as are modulation factors for subaudible tones and tone burst

#define FM_MOD_SCALING_2K5      16              // For FM modulator:  Scaling factor for NCO, after all processing, to achieve 2.5 kHz with a 1 kHz tone
#define FM_MOD_SCALING  FM_MOD_SCALING_2K5      // For FM modulator - system deviation
#define FM_MOD_AMPLITUDE_SCALING    0.875       // For FM modulator:  Scaling factor for output of modulator to set proper output power

// this value represents 2*PI, here 16 bit. It must be a power of two!
// Otherwise a simpel shift does not work as conversion
#define FM_MOD_ACC_BITS 16
#define FM_MOD_ACC_MAX_VALUE (1 << FM_MOD_ACC_BITS)

// this is the generic formula for the conversion from the accumulator to the
// table index
// #define FM_MOD_ACC_DIV (FM_MOD_ACC_MAX_VALUE/DDS_TBL_SIZE)
// but we simply state how many bits to shift to the right
#define FM_MOD_DDS_ACC_SHIFT   (FM_MOD_ACC_BITS-DDS_TBL_BITS)

//
// For subaudible and burst:  FM Tone word calculation:  freq / (sample rate/2^24) => freq / (IQ_SAMPLE_RATE/16777216) => freq * 349.52533333
//
#define FM_SUBAUDIBLE_TONE_AMPLITUDE_SCALING    0.00045 // Scaling factor for subaudible tone modulation - not pre-emphasized -to produce approx +/- 300 Hz deviation in 2.5kHz mode


#define FM_TONE_BURST_AMPLITUDE_SCALING (FM_MOD_SCALING/4266.0) // scale tone modulation (which is NOT pre-emphasized) for approx. 2/3rds of system modulation

#define FM_ALC_GAIN_CORRECTION  0.95

/**
 * Runs FM modulation on audio input signal, including sub tone and tone burst generation. This signal is correctly shifted away from center frequency by the receive frequency shift both by absolute value and direction
 * Audio sidetone is stored in a_block
 *
 * @param a_blocks      audio input buffer, a_blocks[0] is input audio (not changed), a_blocks[1] is filled with processed audio
 * @param iq_buf        iq output buffer
 * @param blockSize     size of iq / audio buffers
 * @param translate_freq signed frequency shift value in Hertz
 * @param dont_care     not used, sideband selection
 * @return always true, to indicate permanent signal generation
 */
static bool TxProcessor_FM(audio_block_t* a_blocks,  iq_buffer_t* iq_buf_p, uint16_t blockSize, const int32_t translate_freq)
{
    static float32_t    hpf_prev_a, hpf_prev_b;
    static uint32_t fm_mod_accum = 0;

    // Fill I and Q buffers with left channel(same as right)
    const float32_t fm_mod_mult = RadioManagement_FmDevIs5khz() ? 2 : 1;

    // Do differentiating high-pass filter to provide 6dB/octave pre-emphasis - which also removes any DC component!
    for(int i = 0; i < blockSize; i++)
    {
        float32_t a = a_blocks[0][i];

        hpf_prev_b = FM_TX_HPF_ALPHA * (hpf_prev_b + a - hpf_prev_a);    // do differentiation

        hpf_prev_a = a;     // save "[n-1] samples for next iteration

        a_blocks[1][i] = hpf_prev_b;    // save differentiated data in audio buffer
    }

    // do tone generation using the NCO (a.k.a. DDS) method.  This is used for subaudible tone generation and, if necessary, summing the result in "a".
    if((ads.fm_conf.subaudible_tone_gen_freq > 0) && (!ads.fm_conf.tone_burst_active))        // generate tone only if it is enabled (and not during a tone burst)
    {
        softdds_addSingleTone(&ads.fm_conf.subaudible_tone_dds, a_blocks[1], blockSize, FM_SUBAUDIBLE_TONE_AMPLITUDE_SCALING * fm_mod_mult);
     }

    // do tone  generation using the NCO (a.k.a. DDS) method.  This is used for tone burst ("whistle-up") generation, summing the result in "a".
    if(ads.fm_conf.tone_burst_active)                // generate tone burst only if it is enabled
    {
        softdds_addSingleTone(&ads.fm_conf.tone_burst_dds, a_blocks[1], blockSize, FM_TONE_BURST_AMPLITUDE_SCALING * fm_mod_mult);
    }

    // do audio frequency modulation using the NCO (a.k.a. DDS) method, carrier at selected shift.
    // Audio is in "a", the result being quadrature FM in "i" and "q".

    uint32_t fm_freq_mod_word = (FM_MOD_ACC_MAX_VALUE * abs(translate_freq))/IQ_SAMPLE_RATE;

    // we swap i and q buffers if we want a negative frequency shift
    float32_t* i_buffer = translate_freq < 0 ? iq_buf_p->q_buffer : iq_buf_p->i_buffer;
    float32_t* q_buffer = translate_freq < 0 ? iq_buf_p->i_buffer : iq_buf_p->q_buffer;

    for(int i = 0; i < blockSize; i++)
    {
        // Calculate next sample
        fm_mod_accum    += fm_freq_mod_word + (a_blocks[1][i] * FM_MOD_SCALING * fm_mod_mult);   // change frequency using scaled audio
        fm_mod_accum    %= FM_MOD_ACC_MAX_VALUE;             // limit to 64k range

        uint32_t fm_mod_idx = fm_mod_accum >> FM_MOD_DDS_ACC_SHIFT; // this value must be in valid table index range by construction
        i_buffer[i] = DDS_TABLE[fm_mod_idx];

        // do 90 degree shifted signal for Q
        q_buffer[i] = DDS_TABLE[softdds_phase_shift90(fm_mod_idx)];
    }

    return true;
}

#ifdef USE_FREEDV

/**
 * Runs FreeDV modulation on audio input signal. This signal is correctly shifted away from center frequency by the receive frequency shift both by absolute value and direction
 *
 * @param a_block       audio input buffer (not changed)
 * @param iq_buf        iq output buffer
 * @param blockSize     size of iq / audio buffers
 * @return true unless buffer underrun from the FreeDV generation occurs
 */
static bool TxProcessor_FreeDV (audio_block_t a_block, iq_buffer_t* iq_buf_p, int16_t blockSize)
{
    // Freedv DL2FW
    static int16_t modulus_Decimate = 0;
    static int16_t modulus_Interpolate = 0;

    const int32_t factor_Decimate = IQ_SAMPLE_RATE/8000; // 6x @ 48ksps
    const int32_t factor_Interpolate = AUDIO_SAMPLE_RATE/8000; // 6x @ 48ksps

    static bool bufferFilled = false;
    bool retval = false;

    // depending on the side band setting we switch the iq buffers accordingly to achieve the right sideband
    bool is_lsb = RadioManagement_LSBActive(ts.dmod_mode);
    float32_t* i_buffer = is_lsb ? iq_buf_p->i_buffer : iq_buf_p->q_buffer;
    float32_t* q_buffer = is_lsb ? iq_buf_p->q_buffer : iq_buf_p->i_buffer;


    // we assume the 48khz voice signal in a_buffer has been preprocessed by a decimation filter
    // BEFORE we decimate !
    // for decimation-by-6 the stopband frequency is 48/6*2 = 4kHz

    // DOWNSAMPLING
    for (int k = 0; k < blockSize; k++)
    {
        if (modulus_Decimate == 0)  //every 6th sample has to be catched -> downsampling by 6
        {
            int16_t sample = ((int32_t)a_block[k])/4;
            RingBuffer_PutSamples(&fdv_audio_rb,&sample,1);
        }

        // increment and wrap
        modulus_Decimate++;
        if (modulus_Decimate == factor_Decimate)
        {
            modulus_Decimate = 0;
        }
    }


    // we wait for at least two frames being processed until we start transmitting
    // we could switch to frame plus 1 block to minimize delay
    if (bufferFilled == false && RingBuffer_GetData(&fdv_iq_rb) > FreeDV_Iq_Get_FrameLen())
    {
        bufferFilled = true;
    }

    if (bufferFilled == true && RingBuffer_GetData(&fdv_iq_rb) >= (modulus_Interpolate == 1?5:6))
    {
        // Best thing here would be to use the arm_fir_decimate function! Why?
        // --> we need phase linear filters, because we have to filter I & Q and preserve their phase relationship
        // IIR filters are power saving, but they do not care about phase, so useless at this point
        // FIR filters are phase linear, but need processor power
        // so we now use the decimation function that upsamples like the code below, BUT at the same time filters
        // (and the routine knows that it does not have to multiply with 0 while filtering: if we do upsampling and subsequent
        // filtering, the filter does not know that and multiplies with zero 5 out of six times --> very inefficient)
        // BUT: we cannot use the ARM function, because decimation factor (6) has to be an integer divide of
        // block size (which is 32 in our case --> 32 / 6 = non-integer!)

        // UPSAMPLING [by hand]
        for (int j = 0; j < blockSize; j++) //  now we are doing upsampling by 6
        {
            if (modulus_Interpolate == 0) // put in sample pair
            {
                fdv_iq_rb_item_t sample;
                RingBuffer_GetSamples(&fdv_iq_rb, &sample, 1);
                i_buffer[j] = sample.real;
                q_buffer[j] = sample.imag;
            }
            else // in 5 of 6 cases just stuff in zeros = zero-padding / zero-stuffing
            {
                i_buffer[j] = 0;
                q_buffer[j] = 0;
            }

            // increment and wrap
            modulus_Interpolate++;
            if (modulus_Interpolate == factor_Interpolate)
            {
                modulus_Interpolate = 0;
            }
        }

        // Add interpolation filter here to suppress alias frequencies
        // we are upsampling from 8kHz to 48kHz, so we have to suppress all frequencies below 4kHz
        // our FreeDV signal is centred at 1500Hz ??? and is 1250Hz broad,
        // so a lowpass filter with cutoff frequency 2400Hz should be fine!

        // INTERPOLATION FILTER [after the interpolation has taken place]
        // the samples are now in adb.iq.i_buffer and adb.iq.q_buffer, so lets filter them
        if ( ads.tx_filter_adjusting == 0 )
        {
            // disable TX filtering during adjustment
            arm_fir_f32(&Fir_TxFreeDV_Interpolate_I, i_buffer, i_buffer,blockSize);
            arm_fir_f32(&Fir_TxFreeDV_Interpolate_Q, q_buffer, q_buffer, blockSize);
            retval = true; // yes, we have a signal
        }
    }
    else
    {
        profileEvent(FreeDVTXUnderrun);
        bufferFilled = false;
    }

    return retval;
}
#endif


#if defined(OBSOLETE_AM_SIDEBAND_CODE)
/***
 * takes the I Q input buffers containing the I Q audio for a single AM sideband and returns the IQ sideband
 * in the i/q input buffers given as argument.
 */
static void TxProcessor_AMSideband(float32_t* i_buffer, float32_t* q_buffer,  const int16_t blockSize) {

    // generate AM carrier by applying a "DC bias" to the audio
    arm_offset_f32(i_buffer, AM_CARRIER_LEVEL, i_buffer, blockSize);
    arm_offset_f32(q_buffer, (-1 * AM_CARRIER_LEVEL), q_buffer, blockSize);

}
#endif

/**
 * Generates AM signal. This signal is correctly shifted away from center frequency by the receive frequency shift both by absolute value and direction
 * Audio sidetone is stored in a_block
 *
 * @param a_block       audio input buffer, overwritten with processed audio
 * @param iq_buf        iq output buffer
 * @param blockSize     size of iq / audio buffers
 * @param translate_freq signed frequency shift value in Hertz
 * @return always true, to indicate permanent signal generation
 */
static bool TxProcessor_AM(audio_block_t a_block, iq_buffer_t* iq_buf,  uint16_t blockSize, const int32_t translate_freq)
{
    bool retval = false;

    if ( ads.tx_filter_adjusting == 0 )         // disable TX filtering during adjustment
    {
        //
        // Apply the TX equalization filtering:  This "flattens" the audio
        // prior to being applied to the Hilbert transformer as well as added low-pass filtering.
        // It does this by applying a "peak" to the bottom end to compensate for the roll-off caused by the Hilbert
        // and then a gradual roll-off toward the high end.  The net result is a very flat (to better than 1dB) response
        // over the 275-2500 Hz range.
        //
        //
        // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
        // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
        // Apply transformation AND audio filtering to buffer data
        //
        // + 0 deg to I data
        // AudioDriver_delay_f32((arm_fir_instance_f32 *)&FIR_I_TX,(float32_t *)(a_buffer),(float32_t *)(adb.iq.i_buffer),blockSize);

        arm_fir_f32(&Fir_Tx_Hilbert_I, a_block, iq_buf->i_buffer, blockSize);
        // - 90 deg to Q data
        arm_fir_f32(&Fir_Tx_Hilbert_Q, a_block, iq_buf->q_buffer, blockSize);


#if defined(OBSOLETE_AM_SIDEBAND_CODE)
        // COMMENT:  It would be trivial to add the option of generating AM with just a single (Upper or Lower) sideband since we are generating the two, separately anyway
        // and putting them back together!  [KA7OEI]

        // temporary buffers;
        float32_t  i2_buffer[blockSize];
        float32_t  q2_buffer[blockSize];


        arm_negate_f32(iq_buf->i_buffer, q2_buffer, blockSize); // this becomes the q buffer for the upper  sideband
        arm_negate_f32(iq_buf->q_buffer, i2_buffer, blockSize); // this becomes the i buffer for the upper  sideband


        // now generate LSB AM sideband signal
        AudioDriver_TxProcessorAMSideband(iq_buf->i_buffer, iq_buf->q_buffer, blockSize);
        // i/q now contain the LSB AM signal

        // now generate USB AM sideband signal
        AudioDriver_TxProcessorAMSideband(i2_buffer, q2_buffer, blockSize);
        arm_add_f32(i2_buffer, iq_buf->i_buffer, iq_buf->i_buffer,blockSize);
        arm_add_f32(q2_buffer, iq_buf->q_buffer, iq_buf->q_buffer,blockSize);
#else
        // in place generation of both AM sidebands
        for (size_t idx = 0; idx < blockSize; idx++)
        {
            float32_t i_am = (iq_buf->i_buffer[idx] - iq_buf->q_buffer[idx]) + (2 * AM_CARRIER_LEVEL);
            float32_t q_am = (iq_buf->q_buffer[idx] - iq_buf->i_buffer[idx]) - (2 * AM_CARRIER_LEVEL);

            iq_buf->i_buffer[idx] = i_am;
            iq_buf->q_buffer[idx] = q_am;
        }
#endif

        // apply correct translate mode
        FreqShift(iq_buf->i_buffer, iq_buf->q_buffer, blockSize, translate_freq);

        retval = true;
    }

    return retval;
}

/**
 * Generates RTTY signal. This signal is sideband correct and around center frequency, i.e. no frequency shift is applied.
 * Audio sidetone is stored in a_block
 *
 * @param a_block       sidetone audio output buffer
 * @param iq_buf        iq computation buffer
 * @param blockSize     size of iq / audio buffers
 * @return always true, to indicate permanent signal generation
 */
static bool TxProcessor_Rtty(audio_block_t a_block, iq_buffer_t* iq_buf_p, uint16_t blockSize)
{

    for (uint16_t idx =0; idx < blockSize; idx++)
    {
        a_block[idx] = Rtty_Modulator_GenSample();
    }
    TxProcessor_FilterAudio(true, false, a_block, a_block, blockSize);
    TxProcessor_SSB(a_block, iq_buf_p, blockSize, 0, ts.digi_lsb);

    return true;
}

/**
 * Generates BPSK signal. This signal is sideband correct and around center frequency, i.e. no frequency shift is applied.
 * Audio sidetone is stored in a_block
 *
 * @param a_block       sidetone audio output buffer
 * @param iq_buf        iq computation buffer
 * @param blockSize     size of iq / audio buffers
 * @return always true, to indicate permanent signal generation
 */
static bool TxProcessor_Psk(audio_block_t a_block, iq_buffer_t* iq_buf_p, uint16_t blockSize)
{

    for (uint16_t idx = 0; idx < blockSize; idx++)
    {
        a_block[idx] = Psk_Modulator_GenSample();
    }

    TxProcessor_FilterAudio(true,false, a_block, a_block, blockSize);
    TxProcessor_SSB(a_block, iq_buf_p, blockSize, 0, ts.digi_lsb);

    return true;
}

/**
 * Generates CW signal. This signal is sideband correct and around center frequency, i.e. no frequency shift is applied.
 * Audio sidetone is stored in a_block
 *
 * @param a_block       sidetone audio output buffer
 * @param iq_buf        iq computation buffer
 * @param blockSize     size of iq / audio buffers
 * @return signal active, if the CW generator produced samples (otherwise we have a break and should "transmit" silence
 */
static bool TxProcessor_CW(audio_block_t a_block, iq_buffer_t* iq_buf_p, uint16_t blockSize)
{
    bool signal_active = false;

    bool is_lsb = RadioManagement_LSBActive(ts.dmod_mode);

    float32_t* i_buffer = is_lsb ? iq_buf_p->i_buffer : iq_buf_p->q_buffer;
    float32_t* q_buffer = is_lsb ? iq_buf_p->q_buffer : iq_buf_p->i_buffer;

    if (ts.tune)
    {
        softdds_runIQ(i_buffer, q_buffer, blockSize);      // generate tone/modulation for TUNE
        // Equalize based on band and simultaneously apply I/Q gain & phase adjustments
        signal_active = true;
    }
    else
    {
        signal_active = CwGen_Process(i_buffer, q_buffer, blockSize);
        // Generate CW tone if necessary
    }

    if (signal_active)
    {
        arm_copy_f32(i_buffer, a_block, blockSize); // we copy the audio to the audio_buffer
        // used for sidetone (on UI_BRD_OVI40) and USB audio out (all devices)
    }
    else
    {
        memset ( a_block, 0, sizeof( a_block[0]) * blockSize );
    }

    return signal_active;
}


void TxProcessor_Run(AudioSample_t * const srcCodec, IqSample_t * const dst, AudioSample_t * const audioDst, uint16_t blockSize, bool external_mute)
{

    /*
     * Theory of operation:
     *  1. Audio processing modes Input:
     *     - (voice) modes requiring some kind of external audio input get that input depending on the settings from the audio codec or the USB audio
     *     - process the audio with filters
     *
     *  2. Data processing modes Input:
     *     - data comes either from cw paddle input or usb keyboard (depending on the mode) buffer
     *
     *  3. Modulation
     *       - produce modulated iq data with a known frequency shift, and correct sideband (if applicable)
     *
     *  4. Output
     *      - depending on the settings and mode and device capabilities, a sidetone is being generated
     *      - sidetone and iq data is being copied to varios "output channels" (audio, USB audio, ... )
     *      - final iq processing places the real transmission data in the DMA buffer for the codec
     */

    // this code (for now) assumes the delivered srcCodec/srcUSB block matches the required IQ blockSize
    assert(AUDIO_BLOCK_SIZE == IQ_BLOCK_SIZE);


    // we copy volatile variables which are used multiple times to local consts to let the compiler do its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t tx_audio_source = ts.tx_audio_source;
    const uint8_t tune = ts.tune;
    const int32_t iq_freq_mode = ts.iq_freq_mode;
    AudioSample_t srcUSB[blockSize];
    AudioSample_t * const src = (tx_audio_source == TX_AUDIO_DIG || tx_audio_source == TX_AUDIO_DIGIQ) ? srcUSB : srcCodec;

    float32_t iq_gain_comp = 1.0; // some modes require a gain compensation of the generate IQ data in order to have same power level

    bool signal_active = false; // unless this is set to true, zero output will be generated


    // tx_param_t tx_param = { .iq_scaling = 1.0, .iq_swap = false };

    // If source is digital usb in, pull from USB buffer, discard line or mic audio and
    // let the normal processing happen
    if (tx_audio_source == TX_AUDIO_DIG || tx_audio_source == TX_AUDIO_DIGIQ)
    {
        // audio sample rate must match the sample rate of USB audio if we read from USB
        assert(tx_audio_source != TX_AUDIO_DIG || AUDIO_SAMPLE_RATE == USBD_AUDIO_FREQ);

        // iq sample rate must match the sample rate of USB IQ audio if we read from USB
        assert(tx_audio_source != TX_AUDIO_DIGIQ || IQ_SAMPLE_RATE == USBD_AUDIO_FREQ);

        UsbdAudio_FillTxBuffer(srcUSB,blockSize);
    }

    if (external_mute)
    {
        // do nothing
    }
    else if (tx_audio_source == TX_AUDIO_DIGIQ && dmod_mode != DEMOD_CW && !tune && !is_demod_psk())
    {

        // If in CW mode or Tune  DIQ audio input is ignored
        // Output I and Q as stereo, fill buffer
        for(int i = 0; i < blockSize; i++)                  // Copy to single buffer
        {
            adb.iq_buf.i_buffer[i] = src[i].l;
            adb.iq_buf.q_buffer[i] = src[i].r;
        }
        signal_active = true;
    }
    else if (ts.dvmode == true)
    {
        switch(ts.digital_mode)
        {
#ifdef USE_FREEDV
        case DigitalMode_FreeDV:
            TxProcessor_PrepareVoice(adb.a_buffer[0], src, blockSize, SSB_ALC_GAIN_CORRECTION, true);
            signal_active = TxProcessor_FreeDV(adb.a_buffer[0], &adb.iq_buf, blockSize);
            iq_gain_comp = FREEDV_GAIN_COMP;
            break;
#endif
        case DigitalMode_RTTY:
            signal_active = TxProcessor_Rtty(adb.a_buffer[0], &adb.iq_buf, blockSize);
            TxProcessor_CwInputForDigitalModes(adb.a_buffer, blockSize);
            iq_gain_comp = SSB_GAIN_COMP;
            break;
        case DigitalMode_BPSK:
            signal_active = TxProcessor_Psk(adb.a_buffer[0], &adb.iq_buf, blockSize);
            TxProcessor_CwInputForDigitalModes(adb.a_buffer, blockSize);
            iq_gain_comp = 1.0;
            break;
        }
    }
    else if(dmod_mode == DEMOD_CW || ts.cw_text_entry)
    {
        signal_active = TxProcessor_CW(adb.a_buffer[0], &adb.iq_buf, blockSize);
    }
    else if(is_ssb(dmod_mode))
    {
        bool runFilter = (ts.flags1 & FLAGS1_SSB_TX_FILTER_DISABLE) == false;
        TxProcessor_PrepareVoice(adb.a_buffer[0], src, blockSize, SSB_ALC_GAIN_CORRECTION, runFilter);
        signal_active = TxProcessor_SSB(adb.a_buffer[0], &adb.iq_buf, blockSize, AudioDriver_GetTranslateFreq(), dmod_mode == DEMOD_LSB);
        iq_gain_comp = SSB_GAIN_COMP;
    }
    else if(dmod_mode == DEMOD_AM)
    {
        //  is frequency translation active (No AM possible unless in frequency translate mode!)
        if (ts.iq_freq_mode)
        {
            bool runFilter = (ts.flags1 & FLAGS1_AM_TX_FILTER_DISABLE) == false;
            TxProcessor_PrepareVoice(adb.a_buffer[0], src, blockSize, AM_ALC_GAIN_CORRECTION, runFilter);
            signal_active = TxProcessor_AM(adb.a_buffer[0], &adb.iq_buf, blockSize, AudioDriver_GetTranslateFreq());
            iq_gain_comp = AM_GAIN_COMP;
        }
    }
    else if(dmod_mode == DEMOD_FM)
    {
        //  is frequency translation active (No FM possible unless in frequency translate mode!)
        if (iq_freq_mode)
        {
            TxProcessor_PrepareVoice(adb.a_buffer[0], src, blockSize, FM_ALC_GAIN_CORRECTION, true);
            signal_active = TxProcessor_FM(adb.a_buffer, &adb.iq_buf, blockSize, AudioDriver_GetTranslateFreq());
            iq_gain_comp = FM_MOD_AMPLITUDE_SCALING;
        }
    }

    if (signal_active == false  || external_mute )
    {
        memset(adb.iq_buf.i_buffer,0,blockSize*sizeof(adb.iq_buf.i_buffer[0]));
        memset(adb.iq_buf.q_buffer,0,blockSize*sizeof(adb.iq_buf.q_buffer[0]));
     }

#ifdef UI_BRD_OVI40
    // we code the sidetone to the audio codec, since we have one for audio and one for iq
    TxProcessor_FillSideToneAudioBuffer( adb.a_buffer[0], audioDst, blockSize, 0.1, signal_active && RadioManagement_UsesTxSidetone());
#endif

    switch (ts.stream_tx_audio)
    {
    case STREAM_TX_AUDIO_OFF:
        // no usb output if not streaming tx output
        break;
    case STREAM_TX_AUDIO_GENIQ:
        for(int i = 0; i < blockSize; i++)
        {

            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            UsbdAudio_PutSample(adb.iq_buf.q_buffer[i]);
            UsbdAudio_PutSample(adb.iq_buf.i_buffer[i]);
        }
        break;
    case STREAM_TX_AUDIO_SRC:
        for(int i = 0; i < blockSize; i++)
        {
            // Native sample format to USB 16 bit format
            // scale down 32bit to 16bit USB audio (if using 32bits)
            // on F4 only:
            // back convert the weird format the F4 wants us to deliver 32bit I2S in (mixed endian)
            UsbdAudio_PutSample(I2S_AudioSample_2_Int16(src[i].r));
            UsbdAudio_PutSample(I2S_AudioSample_2_Int16(src[i].l));
        }
        break;
    case STREAM_TX_AUDIO_FILT:
        for(int i = 0; i < blockSize; i++)
        {
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIG
            // Certain modulation modes will use both buffers, hence we send out both
            // normally only the left buffer is of interest (adb.a_buffer[1]) goes there)
            UsbdAudio_PutSample(adb.a_buffer[0][i]);
            UsbdAudio_PutSample(adb.a_buffer[1][i]);
        }
    }

    // now do the final processing including adjusting the IQ according to the calibration data
    TxProcessor_IqFinalProcessing(iq_gain_comp, false, &adb.iq_buf, dst, blockSize);

    if (ts.stream_tx_audio == STREAM_TX_AUDIO_DIGIQ)
    {
        for(int i = 0; i < blockSize; i++)
        {
            // Native sample format to USB 16 bit format
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            UsbdAudio_PutSample(I2S_IqSample_2_Int16(dst[i].r));
            UsbdAudio_PutSample(I2S_IqSample_2_Int16(dst[i].l));
        }
    }
}
