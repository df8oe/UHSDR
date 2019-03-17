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
#include "uhsdr_board_config.h"
#include "audio_agc.h"
#include "audio_driver.h" // ADC_CLIP_WARN_THRESHOLD
#include "uhsdr_math.h"

#define AGC_WDSP_RB_SIZE ((AUDIO_SAMPLE_RATE/1000)*4) // max buffer size based on max sample rate to be supported
// this translates to 192 at 48k SPS. We have FM using the AGC at full sampling speed


agc_wdsp_params_t agc_wdsp_conf;

typedef struct
{
    // AGC
    //#define MAX_SAMPLE_RATE     (24000.0)
    //#define MAX_N_TAU           (8)
    //#define MAX_TAU_ATTACK      (0.01)
    //#define RB_SIZE       (int) (MAX_SAMPLE_RATE * MAX_N_TAU * MAX_TAU_ATTACK + 1)
    //int8_t AGC_mode = 2;
    int pmode;// = 1; // if 0, calculate magnitude by max(|I|, |Q|), if 1, calculate sqrtf(I*I+Q*Q)
    float32_t out_sample[2];
    float32_t abs_out_sample;
    float32_t tau_attack;
    float32_t tau_decay;
    int n_tau;
    float32_t max_gain;
    float32_t var_gain;
    float32_t fixed_gain; // = 1.0;
    float32_t max_input;
    float32_t out_targ;
    float32_t tau_fast_backaverage;
    float32_t tau_fast_decay;
    float32_t pop_ratio;
    //uint8_t hang_enable;
    float32_t tau_hang_backmult;
    float32_t hangtime;
    float32_t hang_thresh;
    float32_t tau_hang_decay;
    float32_t ring[2 * AGC_WDSP_RB_SIZE]; //192]; //96];
    float32_t abs_ring[AGC_WDSP_RB_SIZE];// 192 //96]; // abs_ring is half the size of ring
    //assign constants
    int ring_buffsize; // = 96;
    //do one-time initialization
    int out_index; // = -1;
    float32_t ring_max; // = 0.0;
    float32_t volts; // = 0.0;
    float32_t save_volts; // = 0.0;
    float32_t fast_backaverage; // = 0.0;
    float32_t hang_backaverage; // = 0.0;
    int hang_counter; // = 0;
    uint8_t decay_type; // = 0;
    uint8_t state; // = 0;
    int attack_buffsize;
    uint32_t in_index;
    float32_t attack_mult;
    float32_t decay_mult;
    float32_t fast_decay_mult;
    float32_t fast_backmult;
    float32_t onemfast_backmult;
    float32_t out_target;
    float32_t min_volts;
    float32_t inv_out_target;
    float32_t tmp;
    float32_t slope_constant;
    float32_t inv_max_input;
    float32_t hang_level;
    float32_t hang_backmult;
    float32_t onemhang_backmult;
    float32_t hang_decay_mult;
    bool      remove_dc;
    float32_t sample_rate;
    bool      initialised;
} agc_variables_t;

agc_variables_t agc_wdsp;

/**
 * Sets the basic initial values for the WDSP AGC
 * Call only once at startup!
 */
void AudioAgc_AgcWdsp_Init()
{
    // the values below are all loaded from
    // EEPROM, which happens BEFORE we get here
    // so there is no point in setting these.
    // agc_wdsp_conf.mode = 2;
    // agc_wdsp_conf.slope = 70;
    // agc_wdsp_conf.hang_enable = 0;
    // agc_wdsp_conf.tau_decay[0] = 4000;
    // agc_wdsp_conf.tau_decay[1] = 2000;
    // agc_wdsp_conf.tau_decay[2] = 500;
    // agc_wdsp_conf.tau_decay[3] = 250;
    // agc_wdsp_conf.tau_decay[4] = 50;
    // agc_wdsp_conf.thresh = 20;
    // agc_wdsp_conf.tau_hang_decay = 500;

    // these are not stored in volatile memory
    // so let us initialize them here.
    agc_wdsp_conf.hang_time = 500;
    agc_wdsp_conf.hang_thresh = 45;
    agc_wdsp_conf.action = 0;
    agc_wdsp_conf.switch_mode = 1;
    agc_wdsp_conf.hang_action = 0;
    agc_wdsp_conf.tau_decay[5] = 1; // this is the OFF-Mode
}

/**
 *  Initializes the AGC data structures, has to be called when switching modes, filter changes
 *
 * @param sample_rate audio sample rate
 * @param remove_dc Should be set for AM demodulation (AM,SAM,DSB) If set to true, remove DC in output
 */
void AudioAgc_SetupAgcWdsp(float32_t sample_rate, bool remove_dc)
{
    // this is a quick and dirty hack
    // it initialises the AGC variables once again,
    // if the decimation rate is changed
    // this should prevent confusion between the distance of in_index and out_index variables
    // because these are freshly initialised
    // in_index and out_index have a distance of 48 (sample rate 12000) or 96 (sample rate 24000)
    // so that has to be defined very well when filter from 4k8 to 5k0 (changing decimation rate from 4 to 2)

    agc_wdsp.remove_dc = remove_dc;

    if(agc_wdsp.sample_rate != sample_rate)
    {
        agc_wdsp.initialised = false; // force initialisation
        agc_wdsp.sample_rate = sample_rate; // remember decimation rate for next time
    }
    // Start variables taken from wdsp
    // RXA.c !!!!
    /*
    0.001,                      // tau_attack
    0.250,                      // tau_decay
    4,                        // n_tau
    10000.0,                    // max_gain
    1.5,                      // var_gain
    1000.0,                     // fixed_gain
    1.0,                      // max_input
    1.0,                      // out_target
    0.250,                      // tau_fast_backaverage
    0.005,                      // tau_fast_decay
    5.0,                      // pop_ratio
    1,                        // hang_enable
    0.500,                      // tau_hang_backmult
    0.250,                      // hangtime
    0.250,                      // hang_thresh
    0.100);                     // tau_hang_decay
     */
    // one time initialization
    if(!agc_wdsp.initialised)
    {

        /*
         *
         *  //assign constants
    a->ring_buffsize = RB_SIZE;
    //do one-time initialization
    a->out_index = -1;
    a->ring_max = 0.0;
    a->volts = 0.0;
    a->save_volts = 0.0;
    a->fast_backaverage = 0.0;
    a->hang_backaverage = 0.0;
    a->hang_counter = 0;
    a->decay_type = 0;
    a->state = 0;
    a->ring = (double *)malloc0(RB_SIZE * sizeof(complex));
    a->abs_ring = (double *)malloc0(RB_SIZE * sizeof(double));
loadWcpAGC(a);
         *
         *
         * */

        agc_wdsp.ring_buffsize = AGC_WDSP_RB_SIZE; //192; //96;
        //do one-time initialization
        agc_wdsp.out_index = -1; //agc_wdsp.ring_buffsize; // or -1 ??
        agc_wdsp.fixed_gain = 1.0;
        agc_wdsp.ring_max = 0.0;
        agc_wdsp.volts = 0.0;
        agc_wdsp.save_volts = 0.0;
        agc_wdsp.fast_backaverage = 0.0;
        agc_wdsp.hang_backaverage = 0.0;
        agc_wdsp.hang_counter = 0;
        agc_wdsp.decay_type = 0;
        agc_wdsp.state = 0;
        for(int idx = 0; idx < AGC_WDSP_RB_SIZE; idx++)
        {
            agc_wdsp.ring[idx * 2 + 0] = 0.0;
            agc_wdsp.ring[idx * 2 + 1] = 0.0;
            agc_wdsp.abs_ring[idx] = 0.0;
        }



        agc_wdsp.tau_attack = 0.001;               // tau_attack
        //    tau_decay = agc_wdsp_conf.tau_decay / 1000.0; // 0.250;                // tau_decay
        agc_wdsp.n_tau = 4;                        // n_tau

        //    max_gain = 1000.0; // 1000.0; determines the AGC threshold = knee level
        //  max_gain is powf (10.0, (float32_t)agc_wdsp_conf.thresh / 20.0);
        //    fixed_gain = ads.agc_rf_gain; //0.7; // if AGC == OFF, this gain is used
        agc_wdsp.max_input = (float32_t)ADC_CLIP_WARN_THRESHOLD; // which is 4096 at the moment
        //32767.0; // maximum value of 16-bit audio //  1.0; //
        agc_wdsp.out_targ = (float32_t)ADC_CLIP_WARN_THRESHOLD; // 4096, tweaked, so that volume when switching between the two AGCs remains equal
        //12000.0; // target value of audio after AGC
        agc_wdsp.tau_fast_backaverage = 0.250;    // tau_fast_backaverage
        agc_wdsp.tau_fast_decay = 0.005;          // tau_fast_decay
        agc_wdsp.pop_ratio = 5.0;                 // pop_ratio
        //    hang_enable = 0;                 // hang_enable
        agc_wdsp.tau_hang_backmult = 0.500;       // tau_hang_backmult

        agc_wdsp.initialised = true;
    }
    //    var_gain = 32.0;  // slope of the AGC --> this is 10 * 10^(slope / 20) --> for 10dB slope, this is 30.0
    agc_wdsp.var_gain = pow10f((float32_t)agc_wdsp_conf.slope / 20.0 / 10.0); // 10^(slope / 200)

    //    hangtime = 0.250;                // hangtime
    agc_wdsp.hangtime = (float32_t)agc_wdsp_conf.hang_time / 1000.0;

    //    hang_thresh = 0.250;             // hang_thresh
    //    tau_hang_decay = 0.100;          // tau_hang_decay

    //calculate internal parameters
    if(agc_wdsp_conf.switch_mode)
    {
        switch (agc_wdsp_conf.mode)
        {
        case 5: //agcOFF
            break;
        case 1: //agcLONG
            agc_wdsp.hangtime = 2.000;
            //      agc_wdsp_conf.tau_decay = 2000;
            //      hang_thresh = 1.0;
            //      agc_wdsp_conf.hang_enable = 1;
            break;
        case 2: //agcSLOW
            agc_wdsp.hangtime = 1.000;
            //      hang_thresh = 1.0;
            //      agc_wdsp_conf.tau_decay = 500;
            //      agc_wdsp_conf.hang_enable = 1;
            break;
        case 3: //agcMED
            //      hang_thresh = 1.0;
            agc_wdsp.hangtime = 0.250;
            //      agc_wdsp_conf.tau_decay = 250;
            break;
        case 4: //agcFAST
            //      hang_thresh = 1.0;
            agc_wdsp.hangtime = 0.100;
            //      agc_wdsp_conf.tau_decay = 50;
            break;
        case 0: //agcFrank --> very long
            //      agc_wdsp_conf.hang_enable = 0;
            //      hang_thresh = 0.300; // from which level on should hang be enabled
            agc_wdsp.hangtime = 3.000; // hang time, if enabled
            agc_wdsp.tau_hang_backmult = 0.500; // time constant exponential averager
            //      agc_wdsp_conf.tau_decay = 4000; // time constant decay long
            agc_wdsp.tau_fast_decay = 0.05;          // tau_fast_decay
            agc_wdsp.tau_fast_backaverage = 0.250; // time constant exponential averager
            break;
        default:
            break;
        }
        agc_wdsp_conf.switch_mode = 0;
    }
    //  float32_t noise_offset = 10.0 * log10f(fhigh - rxa[channel].nbp0.p->flow)
    //          * size / rate);
    //  max_gain = out_target / var_gain * powf (10.0, (thresh + noise_offset) / 20.0));
    agc_wdsp.tau_hang_decay = (float32_t)agc_wdsp_conf.tau_hang_decay / 1000.0;
    agc_wdsp.tau_decay = (float32_t)agc_wdsp_conf.tau_decay[agc_wdsp_conf.mode] / 1000.0;
    agc_wdsp.max_gain = pow10f ((float32_t)agc_wdsp_conf.thresh / 20.0);
    agc_wdsp.fixed_gain = agc_wdsp.max_gain / 10.0;
    // attack_buff_size is 48 for sample rate == 12000 and
    // 96 for sample rate == 24000
    // 192 for sample rate == 48000
    agc_wdsp.attack_buffsize = ceilf(sample_rate * agc_wdsp.n_tau * agc_wdsp.tau_attack);

    agc_wdsp.in_index = agc_wdsp.attack_buffsize + agc_wdsp.out_index; // attack_buffsize + out_index can be more than 2x ring_bufsize !!!
    agc_wdsp.in_index %= agc_wdsp.ring_buffsize; // need to keep this within the index boundaries

    agc_wdsp.attack_mult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_attack));
    agc_wdsp.decay_mult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_decay));
    agc_wdsp.fast_decay_mult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_fast_decay));
    agc_wdsp.fast_backmult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_fast_backaverage));
    agc_wdsp.onemfast_backmult = 1.0 - agc_wdsp.fast_backmult;

    agc_wdsp.out_target = agc_wdsp.out_targ * (1.0 - expf(-(float32_t)agc_wdsp.n_tau)) * 0.9999;
    //  out_target = out_target * (1.0 - expf(-(float32_t)n_tau)) * 0.9999;
    agc_wdsp.min_volts = agc_wdsp.out_target / (agc_wdsp.var_gain * agc_wdsp.max_gain);
    agc_wdsp.inv_out_target = 1.0 / agc_wdsp.out_target;

    float32_t tmpA = log10f(agc_wdsp.out_target / (agc_wdsp.max_input * agc_wdsp.var_gain * agc_wdsp.max_gain));
    if (tmpA == 0.0)
    {
        tmpA = 1e-16;
    }
    agc_wdsp.slope_constant = (agc_wdsp.out_target * (1.0 - 1.0 / agc_wdsp.var_gain)) / tmpA;

    agc_wdsp.inv_max_input = 1.0 / agc_wdsp.max_input;

    if (agc_wdsp.max_input > agc_wdsp.min_volts)
    {
        float32_t convert
        = pow10f ((float32_t)agc_wdsp_conf.hang_thresh / 20.0);
        float32_t tmpB = (convert - agc_wdsp.min_volts) / (agc_wdsp.max_input - agc_wdsp.min_volts);
        if(tmpB < 1e-8)
        {
            tmpB = 1e-8;
        }
        agc_wdsp.hang_thresh = 1.0 + 0.125 * log10f (tmpB);
    }
    else
    {
        agc_wdsp.hang_thresh = 1.0;
    }

    float32_t tmpC = pow10f ((agc_wdsp.hang_thresh - 1.0) / 0.125);
    agc_wdsp.hang_level = (agc_wdsp.max_input * tmpC + (agc_wdsp.out_target /
            (agc_wdsp.var_gain * agc_wdsp.max_gain)) * (1.0 - tmpC)) * 0.637;

    agc_wdsp.hang_backmult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_hang_backmult));
    agc_wdsp.onemhang_backmult = 1.0 - agc_wdsp.hang_backmult;

    agc_wdsp.hang_decay_mult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_hang_decay));
}



/**
 *
 * @param blockSize
 * @param agcbuffer a pointer to the list of buffers of size blockSize containing the audio data
 * @param num_channels
 */
void AudioAgc_RunAgcWdsp(int16_t blockSize, float32_t (*agcbuffer)[AUDIO_BLOCK_SIZE], const bool use_stereo )
{
    // Be careful: the original source code has no comments,
    // all comments added by DD4WH, February 2017: comments could be wrong, misinterpreting or highly misleading!
    //
    if (agc_wdsp_conf.mode == 5)  // AGC OFF
    {
        for (uint16_t i = 0; i < blockSize; i++)
        {
            agcbuffer[0][i] = agcbuffer[0][i] * agc_wdsp.fixed_gain;
            if (use_stereo)
            {
                agcbuffer[1][i] = agcbuffer[1][i] * agc_wdsp.fixed_gain;
            }
        }
        return;
    }

    for (uint16_t i = 0; i < blockSize; i++)
    {
        if (++agc_wdsp.out_index >= agc_wdsp.ring_buffsize)
        {
            agc_wdsp.out_index -= agc_wdsp.ring_buffsize;
        }
        if (++agc_wdsp.in_index >= agc_wdsp.ring_buffsize)
        {
            agc_wdsp.in_index -= agc_wdsp.ring_buffsize;
        }

        agc_wdsp.out_sample[0] = agc_wdsp.ring[2 * agc_wdsp.out_index];
        if(use_stereo)
        {
            agc_wdsp.out_sample[1] = agc_wdsp.ring[2 * agc_wdsp.out_index + 1];
        }
        agc_wdsp.abs_out_sample = agc_wdsp.abs_ring[agc_wdsp.out_index];
        agc_wdsp.ring[2 * agc_wdsp.in_index] = agcbuffer[0][i];
        if(use_stereo)
        {
            agc_wdsp.ring[2 * agc_wdsp.in_index + 1] = agcbuffer[1][i];
        }
        agc_wdsp.abs_ring[agc_wdsp.in_index] = fabsf(agcbuffer[0][i]);
        if(use_stereo)
        {
            if(agc_wdsp.abs_ring[agc_wdsp.in_index] < fabsf(agcbuffer[1][i]))
            {
                agc_wdsp.abs_ring[agc_wdsp.in_index] = fabsf(agcbuffer[1][i]);
            }
        }

        agc_wdsp.fast_backaverage = agc_wdsp.fast_backmult * agc_wdsp.abs_out_sample + agc_wdsp.onemfast_backmult * agc_wdsp.fast_backaverage;
        agc_wdsp.hang_backaverage = agc_wdsp.hang_backmult * agc_wdsp.abs_out_sample + agc_wdsp.onemhang_backmult * agc_wdsp.hang_backaverage;
        if(agc_wdsp.hang_backaverage > agc_wdsp.hang_level)
        {
            agc_wdsp_conf.hang_action = 1;
        }
        else
        {
            agc_wdsp_conf.hang_action = 0;
        }

        if ((agc_wdsp.abs_out_sample >= agc_wdsp.ring_max) && (agc_wdsp.abs_out_sample > 0.0))
        {
            agc_wdsp.ring_max = 0.0;
            int k = agc_wdsp.out_index;

            for (uint16_t j = 0; j < agc_wdsp.attack_buffsize; j++)
            {
                if (++k == agc_wdsp.ring_buffsize)
                {
                    k = 0;
                }
                if (agc_wdsp.abs_ring[k] > agc_wdsp.ring_max)
                {
                    agc_wdsp.ring_max = agc_wdsp.abs_ring[k];
                }
            }
        }
        if (agc_wdsp.abs_ring[agc_wdsp.in_index] > agc_wdsp.ring_max)
        {
            agc_wdsp.ring_max = agc_wdsp.abs_ring[agc_wdsp.in_index];
        }

        if (agc_wdsp.hang_counter > 0)
        {
            --agc_wdsp.hang_counter;
        }

        switch (agc_wdsp.state)
        {
        case 0: // starting point after ATTACK
        {
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            { // ATTACK
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            { // DECAY
                if (agc_wdsp.volts > agc_wdsp.pop_ratio * agc_wdsp.fast_backaverage)
                { // short time constant detector
                    agc_wdsp.state = 1;
                    agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.fast_decay_mult;
                }
                else
                { // hang AGC enabled and being activated
                    if (agc_wdsp_conf.hang_enable  && (agc_wdsp.hang_backaverage > agc_wdsp.hang_level))
                    {
                        agc_wdsp.state = 2;
                        agc_wdsp.hang_counter = (int)(agc_wdsp.hangtime * agc_wdsp.sample_rate);
                        agc_wdsp.decay_type = 1;
                    }
                    else
                    {// long time constant detector
                        agc_wdsp.state = 3;
                        agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.decay_mult;
                        agc_wdsp.decay_type = 0;
                    }
                }
            }
            break;
        }
        case 1: // short time constant decay
        {
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            { // ATTACK
                agc_wdsp.state = 0;
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            {
                if (agc_wdsp.volts > agc_wdsp.save_volts)
                {// short time constant detector
                    agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.fast_decay_mult;
                }
                else
                {
                    if (agc_wdsp.hang_counter > 0)
                    {
                        agc_wdsp.state = 2;
                    }
                    else
                    {
                        if (agc_wdsp.decay_type == 0)
                        {// long time constant detector
                            agc_wdsp.state = 3;
                            agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.decay_mult;
                        }
                        else
                        { // hang time constant
                            agc_wdsp.state = 4;
                            agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.hang_decay_mult;
                        }
                    }
                }
            }
            break;
        }
        case 2: // Hang is enabled and active, hang counter still counting
        { // ATTACK
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            {
                agc_wdsp.state = 0;
                agc_wdsp.save_volts = agc_wdsp.volts;
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            {
                if (agc_wdsp.hang_counter == 0)
                { // hang time constant
                    agc_wdsp.state = 4;
                    agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.hang_decay_mult;
                }
            }
            break;
        }
        case 3: // long time constant decay in progress
        {
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            { // ATTACK
                agc_wdsp.state = 0;
                agc_wdsp.save_volts = agc_wdsp.volts;
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            { // DECAY
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.decay_mult;
            }
            break;
        }
        case 4: // hang was enabled and counter has counted to zero --> hang decay
        {
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            { // ATTACK
                agc_wdsp.state = 0;
                agc_wdsp.save_volts = agc_wdsp.volts;
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            { // HANG DECAY
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.hang_decay_mult;
            }
            break;
        }
        }
        if (agc_wdsp.volts < agc_wdsp.min_volts)
        {
            agc_wdsp.volts = agc_wdsp.min_volts; // no AGC action is taking place
            agc_wdsp_conf.action = 0;
        }
        else
        {
            // LED indicator for AGC action
            agc_wdsp_conf.action = 1;
        }

        float32_t vo =  Math_log10f_fast(agc_wdsp.inv_max_input * agc_wdsp.volts);
        if(vo > 0.0)
        {
            vo = 0.0;
        }

        float32_t mult = (agc_wdsp.out_target - agc_wdsp.slope_constant * vo) / agc_wdsp.volts;
        agcbuffer[0][i] = agc_wdsp.out_sample[0] * mult;
        if(use_stereo)
        {
            agcbuffer[1][i] = agc_wdsp.out_sample[1] * mult;
        }
    }

    if(agc_wdsp.remove_dc)
    {
        static float32_t    wold[2] = { 0.0, 0.0 };

        // eliminate DC in the audio after the AGC
        for(uint16_t i = 0; i < blockSize; i++)
        {
            float32_t w = agcbuffer[0][i] + wold[0] * 0.9999; // yes, I want a superb bass response ;-)
            agcbuffer[0][i] = w - wold[0];
            wold[0] = w;
            if(use_stereo)
            {
                float32_t w = agcbuffer[1][i] + wold[1] * 0.9999; // yes, I want a superb bass response ;-)
                agcbuffer[1][i] = w - wold[1];
                wold[1] = w;
            }
        }
    }
}
