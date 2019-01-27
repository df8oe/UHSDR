/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                               UHSDR FIRMWARE                                    **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Licence:        GNU GPLv3, see LICENSE.md                                                      **
 ************************************************************************************/

#include <assert.h>
#include "freq_shift.h"
#include "audio_driver.h" // only for IQ_BLOCK_SIZE and IQ_SAMPLE_RATE_F
#include <math.h>
#include <stdlib.h>

// The following refer to the software frequency conversion/translation done in receive and transmit to shift the signals away from the
// "DC" IF


typedef struct {
    float32_t Osc_Cos;
    float32_t Osc_Sin;
    float32_t Osc_Vect_Q;
    float32_t Osc_Vect_I;
} fs_nco_approx_t;

/**
 * Prepare the configuration for use with specified shift frequency.
 * @param confptr stores all configuration and operational data of the shift algorithm.
 * @param nco_freq shift frequency in Hz (please note that shift direction, i.e. the positive/negative sign is not taken into account here!)
 * @param sample_rate sample rate of the incoming data in Hz
 * @param blockSize size of data to be processed
 */
static void FreqShift_Approx_Prepare(fs_nco_approx_t* confptr, float32_t nco_freq, float32_t sample_rate)
{
    // Configure NCO for the frequency translate function

    /*
     *  using double here turned out in experiments to yield better precision of the results
     *  and since we use it only when changing the frequency, there is not much harm in
     */
    double rate = (2 * M_PI * nco_freq) / sample_rate;
    confptr->Osc_Cos = cos(rate);
    confptr->Osc_Sin = sin(rate);
    confptr->Osc_Vect_I = 0;
    confptr->Osc_Vect_Q = 1;
}

/**
 * Frequency shift using a low overhead algorithm using (simple) complex arithmetic and gain control approximation algorithm.
 * @param confptr stores all configuration and operational data of the shift algorithm. It must have been configured using FreqShift_Approx_Prepare before!
 * @param I_buffer incoming i data
 * @param Q_buffer incoming q data
 * @param blockSize size of data to be processed
 * @param dir dir == SHIFT_UP moves receive frequency to the right in the spectrum, SHIFT_DOWN opposite direction
 */
static void FreqShift_Approx(fs_nco_approx_t* confptr, float32_t* I_buffer, float32_t* Q_buffer, const size_t blockSize, freq_shift_dir_t dir)
{
    // KA7OEI: Below is the "on-the-fly" version of the frequency translator, generating a "live" version of the oscillator (NCO), which can be any
    // frequency
    // The values for the NCO are configured in the function "FreqShift_Approx_Prepare".

    /*
     * URLs for algorithm information:
     *     https://dsp.stackexchange.com/questions/124/how-to-implement-a-digital-oscillator (hint to recalc gain only from time to time)
     *     https://dspguru.com/dsp/howtos/how-to-create-oscillators-in-software/ (see complex oscillator)
     */

    /*
     *      *
     * Shift Down (shift == 1) => i = Q, q = I
     * [moves receive frequency to the right in the spectrum display]
     *
     * Shift Up   (shift == 0) => q = I, q = Q
     * [moves receive frequency to the right in the spectrum display]
     *
     */

    float32_t* i_buffer = dir == FREQ_SHIFT_UP? I_buffer : Q_buffer;
    float32_t* q_buffer = dir == FREQ_SHIFT_UP? Q_buffer : I_buffer;

    for(int i = 0; i < blockSize; i++)  {
        // generate local oscillator on-the-fly:  This takes a lot of processor time!
        float32_t Osc_Q = (confptr->Osc_Vect_Q * confptr->Osc_Cos) - (confptr->Osc_Vect_I * confptr->Osc_Sin);    // Q channel of oscillator
        float32_t Osc_I = (confptr->Osc_Vect_I * confptr->Osc_Cos) + (confptr->Osc_Vect_Q * confptr->Osc_Sin);    // I channel of oscillator

        // do actual frequency conversion
        float32_t q_temp = q_buffer[i];
        float32_t i_temp = i_buffer[i];   // save temporary copies of data
        q_buffer[i] = (q_temp * Osc_Q) - (i_temp * Osc_I);
        i_buffer[i] = (i_temp * Osc_Q) + (q_temp * Osc_I);  // multiply I/Q data by sine/cosine data to do translation
        confptr->Osc_Vect_Q = Osc_Q;
        confptr->Osc_Vect_I = Osc_I;
    }

    float32_t Osc_Gain = (3 - ((confptr->Osc_Vect_Q * confptr->Osc_Vect_Q) + (confptr->Osc_Vect_I * confptr->Osc_Vect_I)))/2;  // Amplitude control of oscillator
    // rotate vectors while maintaining constant oscillator amplitude
    confptr->Osc_Vect_Q = Osc_Gain * confptr->Osc_Vect_Q;
    confptr->Osc_Vect_I = Osc_Gain * confptr->Osc_Vect_I;

}

/*
 * The oscillator implementation below is straightforward and based on calculating sin and cos using the normal library function.
 * And then simply do the appropriate complex math operations using the arm lib.
 * It supports 2 modes: one is using continuous recalculation of the sin/cos coeffs which is suitable for all shift frequencies
 * It also supports a special mode for rates which are a power of 2, where the coeffs  are only calculated once, since the coeffs
 * repeat after blocksize length anyway.
 *
 * Downside is the memory usage which depends on the max blockSize (we need 6 x IQ_BLOCK_SIZE x sizeof(float32_t) as buffers)
 * and for continuous mode the high load due to permanent sin/cos calculations.
 */
typedef struct
{
    float32_t Osc_Sin_buffer[IQ_BLOCK_SIZE];
    float32_t Osc_Cos_buffer[IQ_BLOCK_SIZE];

    // we need to use double here for continuous mode
    // otherwise the summation error grows to quickly
    // no harm for single calc mode, since we run it only once
    double rad_calc;
    double rad_increment;
    bool single_osc_calc;
} fs_nco_sincos_t;

/**
 * Calculate the sin/cos values for the shift calculation.
 * @param confptr stores all configuration and operational data of the shift algorithm. It must have been configured using FreqShift_SinCos_Prepare before!
 * @param blockSize size of data to be processed
 */
static void FreqShift_SinCos_Coeffs(fs_nco_sincos_t* confptr, size_t blockSize)
{
    for(int i = 0; i < blockSize; i++)          // Now, let's do it!
    {
        confptr->rad_calc += confptr->rad_increment;
        if (confptr->rad_calc > 2*M_PI)
        {
            confptr->rad_calc -= 2*M_PI;
        }
        sincosf(confptr->rad_calc, &confptr->Osc_Sin_buffer[i], &confptr->Osc_Cos_buffer[i]);
    }
}

/**
 * Prepare the configuration for use with specified shift frequency.
 * @param confptr stores all configuration and operational data of the shift algorithm.
 * @param nco_freq shift frequency in Hz (please note that shift direction, i.e. the positive/negative sign is not taken into account here!)
 * @param sample_rate sample rate of the incoming data in Hz
 * @param single_osc_calc true == use precalculated values, requires frequency to be integer division of samplerate, false == arbitrary shift frequency
 * @param blockSize size of data to be processed
 */
static void FreqShift_SinCos_Prepare(fs_nco_sincos_t* confptr, float32_t nco_freq, float32_t sample_rate, bool single_osc_calc, size_t blockSize)
{
    double rate = (2 * M_PI * nco_freq) / sample_rate;
    confptr->rad_increment = rate;
    confptr->single_osc_calc = single_osc_calc;
    if (single_osc_calc)
    {
        confptr->rad_calc = 0;
        FreqShift_SinCos_Coeffs(confptr, blockSize);
    }

}

/**
 * Frequency shift using a trigonometric math. Single coeff calculation operation gives good performance, continuous mode not so much.
 * @param confptr stores all configuration and operational data of the shift algorithm. It must have been configured using FreqShift_SinCos_Prepare before!
 * @param I_buffer incoming i data
 * @param Q_buffer incoming q data
 * @param blockSize size of data to be processed
 * @param dir dir == SHIFT_UP moves receive frequency to the right in the spectrum, SHIFT_DOWN opposite direction
 */
static void FreqShift_SinCos(fs_nco_sincos_t* confptr, float32_t* I_buffer, float32_t* Q_buffer, size_t blockSize, uint16_t dir)
{

    /*
     * cos(i) + sin(q) for i channel
     * cos(q) - sin(i) for q channel
     *
     * Shift Down (shift == 1) => i = Q, q = I
     * [moves receive frequency to the right in the spectrum display]
     *
     * Shift Up   (shift == 0) => q = I, q = Q
     * [moves receive frequency to the right in the spectrum display]
     *
     */

    float32_t* i_buffer = dir == FREQ_SHIFT_UP? I_buffer : Q_buffer;
    float32_t* q_buffer = dir == FREQ_SHIFT_UP? Q_buffer : I_buffer;

    float32_t q_cos_buffer[blockSize];
    float32_t i_sin_buffer[blockSize];
    float32_t q_sin_buffer[blockSize];
    float32_t i_cos_buffer[blockSize];

    if (confptr->single_osc_calc == false)
    {
        FreqShift_SinCos_Coeffs(confptr, blockSize);
    }

    // Do frequency conversion using optimized ARM math functions [KA7OEI]
    arm_mult_f32(q_buffer, confptr->Osc_Cos_buffer, q_cos_buffer, blockSize); // multiply products for converted I channel
    arm_mult_f32(i_buffer, confptr->Osc_Sin_buffer, i_sin_buffer, blockSize);
    arm_mult_f32(q_buffer, confptr->Osc_Sin_buffer, q_sin_buffer, blockSize);
    arm_mult_f32(i_buffer, confptr->Osc_Cos_buffer, i_cos_buffer, blockSize);    // multiply products for converted Q channel

    arm_add_f32(i_cos_buffer, q_sin_buffer, i_buffer, blockSize);   // cos(I) + sin(Q) for I channel
    arm_sub_f32(q_cos_buffer, i_sin_buffer, q_buffer, blockSize);   // cos(Q) - sin(I) for Q channel
}

/**
 * Frequency shift using a just complex add and sub. Shifts by a quarter of the sample frequency. Best performance, single shift frequency.
 * @param confptr stores all configuration and operational data of the shift algorithm. It must have been configured using FreqShift_SinCos_Prepare before!
 * @param I_buffer incoming i data
 * @param Q_buffer incoming q data
 * @param blockSize size of data to be processed
 * @param dir dir == FREQ_SHIFT_UP moves receive frequency to the right in the spectrum, FREQ_SHIFT_DOWN opposite direction
 */
static void FreqShift_QuarterFs(float32_t* I_buffer, float32_t* Q_buffer, int16_t blockSize, int16_t dir)
{
    /**********************************************************************************
     *  Frequency translation by Fs/4 without multiplication
     *  Lyons (2011): chapter 13.1.2 page 646
     *  this is supposed to be much more efficient than a standard quadrature oscillator
     *  with precalculated sin waves
     *  Thanks, Clint, for pointing my interest to this method!, DD4WH 2016_12_28
     **********************************************************************************/

    /*
     *
     * Shift Down (shift == 1) => i = Q, q = I      *
     * [moves receive frequency to the right in the spectrum display]
     *
     * Shift Up   (shift == 0) => q = I, q = Q
     * [moves receive frequency to the right in the spectrum display]
     *
     */

    float32_t* i_buffer = dir == FREQ_SHIFT_UP? I_buffer : Q_buffer;
    float32_t* q_buffer = dir == FREQ_SHIFT_UP? Q_buffer : I_buffer;


    for(int i = 0; i < blockSize; i += 4)
    {   // xnew(0) =  xreal(0) + jximag(0)
        // leave as it is!
        // xnew(1) =  ximag(1) - jxreal(1)
        float32_t hh1 = q_buffer[i + 1];
        float32_t hh2 = - i_buffer[i + 1];
        i_buffer[i + 1] = hh1;
        q_buffer[i + 1] = hh2;
        // xnew(2) = -xreal(2) - jximag(2)
        hh1 = - i_buffer[i + 2];
        hh2 = - q_buffer[i + 2];
        i_buffer[i + 2] = hh1;
        q_buffer[i + 2] = hh2;
        // xnew(3) = -ximag(3) + jxreal(3)
        hh1 = - q_buffer[i + 3];
        hh2 = i_buffer[i + 3];
        i_buffer[i + 3] = hh1;
        q_buffer[i + 3] = hh2;
    }
}

/**
 * Frequency shift using the best available algorithm. Handles changes to the shift frequency on the fly. This is
 * the front end for all the shift algorithms. Sample rate is fixed to IQ_SAMPLE_RATE, max buffer is IQ_BLOCK_SIZE
 * It cannot be called twice at the same time (i.e. must not be used in code running in different interrupt levels
 * which may interrupt each other or mixed in normal code and interrupt code)
 *
 * @param I_buffer incoming i data
 * @param Q_buffer incoming q data
 * @param blockSize size of data to be processed
 * @param shift  > 0 SHIFT_UP moves receive frequency to the right in the spectrum, SHIFT_DOWN opposite direction
 */
void FreqShift(float32_t* i_buffer, float32_t* q_buffer, size_t blockSize, int32_t shift)
{
    // keeps the generated data for frequency conversion
    static int32_t conversion_freq = 0;
    static bool single_osc_calc = false;
    static bool is_quarter_of_fs = false;

    static fs_nco_sincos_t nco_sincos;
    static fs_nco_approx_t nco_approx;


    assert(blockSize <= IQ_BLOCK_SIZE);

    int32_t new_conversion_freq = abs(shift);
    if (conversion_freq != new_conversion_freq || single_osc_calc == false)
    {
        if (conversion_freq != new_conversion_freq)
        {
            float32_t rate = new_conversion_freq / IQ_SAMPLE_RATE_F;

            single_osc_calc = (rate == 0.25 || rate == 0.125 || rate == 0.0625 || rate == 0.03125);
            // if the sample frequency divider to get shift frequency is a power of two no larger
            // than the number of samples in the osc buffer
            // we can avoid continuous recalculation of sin/cos table because it repeats after blockSize entries

            is_quarter_of_fs = rate == 0.25;
            // if we have shift equals a quarter of sample frequency, we can use a highly optimized formula, see below

            conversion_freq = new_conversion_freq;

            FreqShift_Approx_Prepare(&nco_approx, new_conversion_freq, IQ_SAMPLE_RATE_F);
            // this algorithm works always, but takes more time

            single_osc_calc = false;
            if (single_osc_calc == true)
            {

                // Pre-calculate quadrature sine wave(s) ONCE for the conversion
                FreqShift_SinCos_Prepare(&nco_sincos, new_conversion_freq, IQ_SAMPLE_RATE_F, single_osc_calc, blockSize);
            }
        }
    }

    bool dir = shift > 0;
    if (is_quarter_of_fs == true)
    {
        FreqShift_QuarterFs(i_buffer, q_buffer, blockSize, dir);
    }
    else if (single_osc_calc == true)  // 'not so optimized' frequency translation using sin/cos (+6kHz or -6kHz)
    {
        // Below is the call to frequency translation code that uses a "pre-calculated" sine wave - which means that the translation must be done at a sub-
        // multiple of the sample frequency.  This pre-calculation eliminates the processor overhead required to generate a sine wave on the fly.
        // This also makes extensive use of the optimized ARM vector instructions for the calculation of the final I/Q vectors
        FreqShift_SinCos(&nco_sincos, i_buffer, q_buffer, blockSize, dir);
    }
    else
    {
        FreqShift_Approx(&nco_approx, i_buffer, q_buffer, blockSize, dir);
    }
}
