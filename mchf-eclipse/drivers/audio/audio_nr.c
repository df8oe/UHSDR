/************************************************************************************
 **                                                                                 **
 **                               UHSDR Firmware                                    **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Description:                                                                   **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/

#include "uhsdr_board.h"
#include "freedv_uhsdr.h"
#include "arm_const_structs.h"
#include "profiling.h"

#ifdef USE_ALTERNATE_NR

__IO int32_t NR_in_head = 0;
__IO int32_t NR_in_tail = 0;
__IO int32_t NR_out_head = 0;
__IO int32_t NR_out_tail = 0;

FDV_IQ_Buffer* NR_in_buffers[FDV_BUFFER_IQ_FIFO_SIZE];

FDV_IQ_Buffer* NR_out_buffers[FDV_BUFFER_IQ_FIFO_SIZE];


int NR_in_buffer_peek(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_in_head != NR_in_tail)
    {
        FDV_IQ_Buffer* c = NR_in_buffers[NR_in_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int NR_in_buffer_remove(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_in_head != NR_in_tail)
    {
        FDV_IQ_Buffer* c = NR_in_buffers[NR_in_tail];
        NR_in_tail = (NR_in_tail + 1) % FDV_BUFFER_IQ_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int NR_in_buffer_add(FDV_IQ_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (NR_in_head + 1) % FDV_BUFFER_IQ_FIFO_SIZE;

    if (next_head != NR_in_tail)
    {
        /* there is room */
        NR_in_buffers[NR_in_head] = c;
        NR_in_head = next_head;
        ret ++;
    }
    return ret;
}

void NR_in_buffer_reset()
{
    NR_in_tail = NR_in_head;
}

int8_t NR_in_has_data()
{
    int32_t len = NR_in_head - NR_in_tail;
    return len < 0?len+FDV_BUFFER_IQ_FIFO_SIZE:len;
}

int32_t NR_in_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return FDV_BUFFER_IQ_FIFO_SIZE - 1 - NR_in_has_data();
}


//*********Out Buffer handling

int NR_out_buffer_peek(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_out_head != NR_out_tail)
    {
        FDV_IQ_Buffer* c = NR_out_buffers[NR_out_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int NR_out_buffer_remove(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_out_head != NR_out_tail)
    {
        FDV_IQ_Buffer* c = NR_out_buffers[NR_out_tail];
        NR_out_tail = (NR_out_tail + 1) % FDV_BUFFER_IQ_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int NR_out_buffer_add(FDV_IQ_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (NR_out_head + 1) % FDV_BUFFER_IQ_FIFO_SIZE;

    if (next_head != NR_out_tail)
    {
        /* there is room */
        NR_out_buffers[NR_out_head] = c;
        NR_out_head = next_head;
        ret ++;
    }
    return ret;
}

void NR_out_buffer_reset()
{
    NR_out_tail = NR_out_head;
}

int8_t NR_out_has_data()
{
    int32_t len = NR_out_head - NR_out_tail;
    return len < 0?len+FDV_BUFFER_IQ_FIFO_SIZE:len;
}

int32_t NR_out_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return FDV_BUFFER_IQ_FIFO_SIZE - 1 - NR_out_has_data();
}



void alternateNR_handle()
{
    static uint16_t NR_current_buffer_idx = 0;
    static bool NR_was_here = false;

    if (NR_was_here == false)
    {
        NR_was_here = true;
        NR_current_buffer_idx = 0;
        NR_in_buffer_reset();
        NR_out_buffer_reset();
    }

    if ( NR_in_has_data() && NR_out_has_room())
    {   // audio data is ready to be processed

        NR_current_buffer_idx %= FDV_BUFFER_IQ_NUM;

        FDV_IQ_Buffer* input_buf = NULL;
        NR_in_buffer_remove(&input_buf); //&input_buffer points to the current valid audio data

        // inside here do all the necessary noise reduction stuff!!!!!
        // here are the current input samples:  input_buf->samples
        // NR_output samples have to be placed here: fdv_iq_buff[NR_current_buffer_idx].samples
        // but starting at an offset of NR_FFT_SIZE as we are using the same buffer for in and out
        // here is the only place where we are referring to fdv_iq... as this is the name of the used freedv buffer

        profileTimedEventStart(ProfileTP8);

        do_alternate_NR(&input_buf->samples[0].real,&fdv_iq_buff[NR_current_buffer_idx].samples[NR_FFT_SIZE].real);

        profileTimedEventStop(ProfileTP8);

        NR_out_buffer_add(&fdv_iq_buff[NR_current_buffer_idx]);
        NR_current_buffer_idx++;

    }

}


void do_alternate_NR(float32_t* inputsamples, float32_t* outputsamples )
{

    float32_t* Energy=0;

    if(ts.new_nb)
    {
        alt_noise_blanking(inputsamples,NR_FFT_SIZE,Energy);
    }

    if(ts.nr_enable)
    {
        spectral_noise_reduction(inputsamples);
    }

    for (int k=0; k < NR_FFT_SIZE;  k++)
    {
        outputsamples[k] = inputsamples[k];
    }

}

#define NR_FFT_L NR_FFT_SIZE
//static float32_t NR_output_audio_buffer [NR_FFT_L]; // saved 0.5kbytes RAM
static float32_t NR_last_iFFT_result [NR_FFT_L / 2];
static float32_t NR_last_sample_buffer_L [NR_FFT_L / 2];
float32_t NR_FFT_buffer[NR_FFT_L * 2];
//float32_t NR_iFFT_buffer[NR_FFT_L * 2]; // saved 1kbyte RAM :-)
static float32_t NR_X[NR_FFT_L / 2][2]; // magnitudes of the current and the last FFT bins
static float32_t NR_Nest[NR_FFT_L / 2][2]; // noise estimates for the current and the last FFT frame
static float32_t NR_vk; // saved 0.24kbytes
static float32_t NR_Hk[NR_FFT_L / 2]; // gain factors
static float32_t NR_SNR_prio[NR_FFT_L / 2];
static float32_t NR_SNR_post[NR_FFT_L / 2];
static float32_t NR_SNR_post_pos; // saved 0.24kbytes
static float32_t NR_Hk_old[NR_FFT_L / 2];
static float32_t NR_VAD = 0.0;
static uint8_t NR_first_time = 1;

void spectral_noise_reduction (float* in_buffer)
{
// Frank DD4WH & Michael DL2FW, November 2017
// NOISE REDUCTION BASED ON SPECTRAL SUBTRACTION
// following Romanin et al. 2009 on the basis of Ephraim & Malah 1984 and Hu et al. 2001
// detailed technical description of the implemented algorithm
// can be found in our WIKI
// https://github.com/df8oe/UHSDR/wiki/Noise-reduction
//
//  half-overlapping input buffers (= overlap 50%)
// Hann window on 128 samples
// FFT128 - inverse FFT128
// overlap-add

    if(NR_first_time == 1)
    { // TODO: properly initialize all the variables
        for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
        {
            NR_last_sample_buffer_L[bindx] = 20.0;
            NR_Hk_old[bindx] = 0.1; // old gain
            NR_Nest[bindx][0] = 500.0;
            NR_Nest[bindx][1] = 400.0;
            NR_X[bindx][1] = 20.0;
            NR_SNR_post[bindx] = 2.0;
            NR_SNR_prio[bindx] = 1.0;
            NR_first_time = 2;
        }
    }

    for(int k = 0; k < 2; k++)
    {
    // NR_FFT_buffer is 256 floats big
    // interleaved r, i, r, i . . .
    // fill first half of FFT_buffer with last events audio samples
          for(int i = 0; i < NR_FFT_L / 2; i++)
          {
            NR_FFT_buffer[i * 2] = NR_last_sample_buffer_L[i]; // real
            NR_FFT_buffer[i * 2 + 1] = 0.0; // imaginary
          }
    // copy recent samples to last_sample_buffer for next time!
          for(int i = 0; i < NR_FFT_L  / 2; i++)
          {
             NR_last_sample_buffer_L [i] = in_buffer[i + k * (NR_FFT_L / 2)];
          }
    // now fill recent audio samples into second half of FFT_buffer
          for(int i = 0; i < NR_FFT_L / 2; i++)
          {
              NR_FFT_buffer[NR_FFT_L + i * 2] = in_buffer[i+ k * (NR_FFT_L / 2)]; // real
              NR_FFT_buffer[NR_FFT_L + i * 2 + 1] = 0.0;
          }
    /////////////////////////////////7
    // WINDOWING
    #if 1
    // perform windowing on 256 real samples in the NR_FFT_buffer
          for (int idx = 0; idx < NR_FFT_L; idx++)
          {     // Hann window
             float32_t temp_sample = 0.5 * (float32_t)(1.0 - (cosf(PI* 2.0 * (float32_t)idx / (float32_t)((NR_FFT_L) - 1))));
             NR_FFT_buffer[idx * 2] *= temp_sample;
          }
    #endif
    // NR_FFT 256
    // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
          arm_cfft_f32(&arm_cfft_sR_f32_len128, NR_FFT_buffer, 0, 1);

              for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
                    {
                        // this is magnitude for the current frame
                        NR_X[bindx][0] = sqrtf(NR_FFT_buffer[bindx * 2] * NR_FFT_buffer[bindx * 2] + NR_FFT_buffer[bindx * 2 + 1] * NR_FFT_buffer[bindx * 2 + 1]);
                    }
        // 2b.) voice activity detector
                  float32_t NR_temp_sum = 0.0;
                  for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // try 128:
                  {
                      float32_t D_squared = NR_Nest[bindx][0] * NR_Nest[bindx][0]; //
                      NR_temp_sum += (NR_X[bindx][0]/ (D_squared) ) - logf((NR_X[bindx][0] / (D_squared) )) - 1.0;
                  }
                  NR_VAD = NR_temp_sum / (NR_FFT_L / 2);
                      if(NR_VAD < ts.nr_vad_thresh || NR_first_time == 2)
                      {
                          // noise estimation with exponential averager
                          for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
                                {   // exponential averager for current noise estimate
                                      NR_Nest[bindx][0] = (1.0 - ts.nr_beta) * NR_X[bindx][0] + ts.nr_beta * NR_Nest[bindx][1]; //
                                      NR_Nest[bindx][1] = NR_Nest[bindx][0];
                                }
                          NR_first_time = 0;
                      }


        // 3    calculate SNRpost (n, bin[i]) = (X(n, bin[i])^2 / Nest(n, bin[i])^2) - 1 (eq. 13 of Schmitt et al. 2002)
              for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
                    {
                        // (Yk)^2 / Dk (eq 11, Romanin et al. 2009)
                        if(NR_Nest[bindx][0] != 0.0)
                        {   // do we have to square the noise estimate NR_M[bindx] or not? Schmitt says yes, Romanin says no . . .
                           NR_SNR_post[bindx] = NR_X[bindx][0] / (NR_Nest[bindx][0]);
                        }
                        // "half-wave rectification" of NR_SR_post_pos --> always >= 0
                        if(NR_SNR_post[bindx] >= 0.0)
                        {
                            NR_SNR_post_pos = NR_SNR_post[bindx];
                        }
                        else
                        {
                            NR_SNR_post_pos = 0.0;
                        }
        // 3    calculate SNRprio (n, bin[i]) = (1 - alpha) * Q(SNRpost(n, bin[i]) + alpha * (Hk(n - 1, bin[i]) * X(n - 1, bin[i])^2 / Nest(n, bin[i])^2 (eq. 14 of Schmitt et al. 2002, eq. 13 of Romanin et al. 2009) [Q[x] = x if x>=0, else Q[x] = 0]
        // again: do we have to square the noise estimate NR_M[bindx] or not? Schmitt says yes, Romanin says no . . .
                        if(NR_Nest[bindx][0] != 0.0)
                        {
                            NR_SNR_prio[bindx] = (1.0 - ts.nr_alpha) * NR_SNR_post_pos +
                                                 ts.nr_alpha * ((NR_Hk_old[bindx] * NR_Hk_old[bindx] * NR_X[bindx][1]) / (NR_Nest[bindx][0])); //
                        }
        // 4    calculate vk = SNRprio(n, bin[i]) / (SNRprio(n, bin[i]) + 1) * SNRpost(n, bin[i]) (eq. 12 of Schmitt et al. 2002, eq. 9 of Romanin et al. 2009)
                        NR_vk =  NR_SNR_post[bindx] * NR_SNR_prio[bindx] / (1.0 + NR_SNR_prio[bindx]);
                       // calculate Hk
        // 5    finally calculate the weighting function for each bin: Hk(n, bin[i]) = 1 / SNRpost(n, [i]) * sqrtf(0.7212 * vk + vk * vk) (eq. 26 of Romanin et al. 2009)
                        if(NR_vk > 0.0 && NR_SNR_post[bindx] != 0.0) // prevent sqrtf of negatives
                        {
                            NR_Hk[bindx] = 1.0 / NR_SNR_post[bindx] * sqrtf(0.7212 * NR_vk + NR_vk * NR_vk);
                        }
                        else
                        {
                            NR_Hk[bindx] = 1.0;
                        }
                        NR_Hk_old[bindx] = NR_Hk[bindx];
                        NR_X[bindx][1] = NR_X[bindx][0];
                    }

        // FINAL SPECTRAL WEIGHTING: Multiply current FFT results with NR_FFT_buffer for 128 bins with the 128 bin-specific gain factors G
              for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++) // try 128:
              {
                  NR_FFT_buffer[bindx * 2] = NR_FFT_buffer [bindx * 2] * NR_Hk[bindx]; // real part
                  NR_FFT_buffer[bindx * 2 + 1] = NR_FFT_buffer [bindx * 2 + 1] * NR_Hk[bindx]; // imag part
                  NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 2] = NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 2] * NR_Hk[bindx]; // real part conjugate symmetric
                  NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 1] = NR_FFT_buffer[NR_FFT_L * 2 - bindx * 2 - 1] * NR_Hk[bindx]; // imag part conjugate symmetric
              }


        /*****************************************************************
         * NOISE REDUCTION CODE ENDS HERE
         *****************************************************************/
#if 0
  for(int idx = 1; idx < 20; idx++)
  // bins 2 to 29 attenuated
  // set real values to 0.1 of their original value
  {
      NR_iFFT_buffer[idx * 2] *= 0.1;
      NR_iFFT_buffer[NR_FFT_L * 2 - ((idx + 1) * 2)] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
      NR_iFFT_buffer[idx * 2 + 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
      NR_iFFT_buffer[NR_FFT_L * 2 - ((idx + 1) * 2) + 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
  }
#endif

    // NR_iFFT
    // perform iFFT (in-place)
         arm_cfft_f32(&arm_cfft_sR_f32_len128, NR_FFT_buffer, 1, 1);
    // do the overlap & add
          for(int i = 0; i < NR_FFT_L / 2; i++)
          { // take real part of first half of current iFFT result and add to 2nd half of last iFFT_result
        	  //              NR_output_audio_buffer[i + k * (NR_FFT_L / 2)] = NR_FFT_buffer[i * 2] + NR_last_iFFT_result[i];
        	  in_buffer[i + k * (NR_FFT_L / 2)] = NR_FFT_buffer[i * 2] + NR_last_iFFT_result[i];
          }
          for(int i = 0; i < NR_FFT_L / 2; i++)
          {
              NR_last_iFFT_result[i] = NR_FFT_buffer[NR_FFT_L + i * 2];
          }
       // end of "for" loop which repeats the FFT_iFFT_chain two times !!!
    }

    /*      for(int i = 0; i < NR_FFT_L; i++)
          {
              in_buffer [i] = NR_output_audio_buffer[i];
              //float_buffer_R [i] = float_buffer_L [i];
          }*/
}


//alt noise blanking is trying to localize some impulse noise within the samples and after that
//trying to replace corrupted samples by linear predicted samples.
//therefore, first we calculate the lpc coefficients which represent the actual status of the
//speech or sound generating "instrument" (in case of speech this is an estimation of the current
//filter-function of the voice generating tract behind our lips :-) )
//after finding this function we inverse filter the actual samples by this function
//so we are eliminating the speech, but not the noise. Then we do a matched filtering an thereby detecting impulses
//After that we threshold the remaining samples by some
//level and so detecting impulse noise's positions within the current frame - if one (or more) impulses are there.
//finally some area around the impulse position will be replaced by predicted samples from both sides (forward and
//backward prediction)
//hopefully we have enough processor power left....

void alt_noise_blanking(float* insamp,int Nsam, int order, float* E )
{
#define boundary_blank 14 // for first trials very large!!!!
#define impulse_length 7 // has to be odd!!!! 7 / 3 should be enough
#define PL             3 // has to be (impulse_length-1)/2 !!!!
#define order         10 // lpc's order
    arm_fir_instance_f32 LPC;
    float32_t lpcs[order+1]; // we reserve one more than "order" because of a leading "1"
    float32_t reverse_lpcs[order+1]; //this takes the reversed order lpc coefficients
    float32_t firStateF32[NR_FFT_SIZE + order];
    float32_t tempsamp[NR_FFT_SIZE];
    float32_t sigma2; //taking the variance of the inpo
    float32_t lpc_power;
    float32_t impulse_threshold;
    int impulse_positions[5];  //we allow a maximum of 5 impulses per frame
    int search_pos=0;
    int impulse_count=0;
    static float32_t last_frame_end[order+PL]; //this takes the last samples from the previous frame to do the prediction within the boundaries
#ifdef debug_alternate_NR
    static int frame_count=0;  //only used for the distortion insertion - can alter be deleted
    int dist_level=0;//only used for the distortion insertion - can alter be deleted
    int nr_setting = 0;
#endif

    float32_t R[11];  // takes the autocorrelation results
    float32_t k,alfa;

    float32_t any[order+1];  //some internal buffers for the levinson durben algorithm

    float32_t Rfw[impulse_length+order]; // takes the forward predicted audio restauration
    float32_t Rbw[impulse_length+order]; // takes the backward predicted audio restauration
    float32_t Wfw[impulse_length],Wbw[impulse_length]; // taking linear windows for the combination of fwd and bwd

    float32_t s;

#ifdef debug_alternate_NR  // generate test frames to test the noise blanker function
    // using the NR-setting (0..55) to select the test frame
    // 00 = noise blanker active on orig. audio; threshold factor=3
    // 01 = frame of vocal "a" undistorted
    // 02 .. 05 = frame of vocal "a" with different impulse distortion levels
    // 06 .. 09 = frame of vocal "a" with different impulse distortion levels
    //            noise blanker operating!!
    //************
    // 01..09 are now using the original received audio and applying a rythmic "click" distortion
    // 06..09 is detecting and removing the click by restoring the predicted audio!!!
    //************
    // 5 / 9 is the biggest "click" and it is slightly noticeable in the restored audio (9)
    // 10 = noise blanker active on orig. audio threshold factor=3
    // 11  = sinusoidal signal undistorted
    // 12 ..15 = sinusoidal signal with different impulse distortion levels
    // 16 ..19 = sinusoidal signal with different impulse distortion levels
    //            noise blanker operating!!
    // 20 ..50   noise blanker active on orig. audio; threshold factor varying between 3 and 0.26

    nr_setting = (int)ts.dsp_nr_strength;
    //*********************************from here just debug impulse / signal generation
    if ((nr_setting > 0) && (nr_setting < 10)) // we use the vocal "a" frame
    {
        //for (int i=0; i<128;i++)          // not using vocal "a" but the original signal
        //    insamp[i]=NR_test_samp[i];

        if ((frame_count > 19) && (nr_setting > 1))    // insert a distorting pulse
        {
            dist_level=nr_setting;
            if (dist_level > 5) dist_level=dist_level-4; // distortion level is 1...5
            insamp[4]=insamp[4] + dist_level*3000; // overlaying a short  distortion pulse +/-
            insamp[5]=insamp[5] - dist_level*1000;
        }

    }

    if ((nr_setting > 10) && (nr_setting < 20)) // we use the sinus frame
    {
        for (int i=0; i<128;i++)
            insamp[i]=NR_test_sinus_samp[i];

        if ((frame_count > 19) && (nr_setting > 11))    // insert a distorting pulse
        {
            dist_level=nr_setting-10;
            if (dist_level > 5) dist_level=dist_level-4;
            insamp[24]=insamp[24] + dist_level*1000; // overlaying a short  distortion pulse +/-
            insamp[25]=insamp[25] + dist_level*500;
            insamp[26]=insamp[26] - dist_level*200; // overlaying a short  distortion pulse +/-
            insamp[27]=insamp[27] - dist_level*100;


        }



    }


    frame_count++;
    if (frame_count > 20) frame_count=0;

#endif

    //*****************************end of debug impulse generation

    //  start of test timing zone

    for (int i=0; i<impulse_length; i++)  // generating 2 Windows for the combination of the 2 predictors
    {                                     // will be a constant window later!
        Wbw[i]=1.0*i/(impulse_length-1);
        Wfw[impulse_length-i-1]=Wbw[i];
    }

    // calculate the autocorrelation of insamp (moving by max. of #order# samples)
    for(int i=0; i < (order+1); i++)
    {
        arm_dot_prod_f32(&insamp[0],&insamp[i],Nsam-i,&R[i]); // R is carrying the crosscorrelations
    }
    // end of autocorrelation



    //alternative levinson durben algorithm to calculate the lpc coefficients from the crosscorrelation

    R[0] = R[0] * (1.0 + 1.0e-9);

    lpcs[0] = 1;   //set lpc 0 to 1

    for (int i=1; i < order+1; i++)
        lpcs[i]=0;                      // fill rest of array with zeros - could be done by memfill

    alfa = R[0];

    for (int m = 1; m <= order; m++)
    {
        s = 0.0;
        for (int u = 1; u < m; u++)
            s = s + lpcs[u] * R[m-u];

        k = -(R[m] + s) / alfa;

        for (int v = 1;v < m; v++)
            any[v] = lpcs[v] + k * lpcs[m-v];

        for (int w = 1; w < m; w++)
            lpcs[w] = any[w];

        lpcs[m] = k;
        alfa = alfa * (1 - k * k);
    }

    // end of levinson durben algorithm

    for (int o = 0; o < order+1; o++ )             //store the reverse order coefficients separately
        reverse_lpcs[order-o]=lpcs[o];        // for the matched impulse filter

    arm_fir_init_f32(&LPC,order+1,&reverse_lpcs[0],&firStateF32[0],NR_FFT_SIZE);                                         // we are using the same function as used in freedv

    arm_fir_f32(&LPC,insamp,tempsamp,Nsam); //do the inverse filtering to eliminate voice and enhance the impulses

    arm_fir_init_f32(&LPC,order+1,&lpcs[0],&firStateF32[0],NR_FFT_SIZE);                                         // we are using the same function as used in freedv

    arm_fir_f32(&LPC,tempsamp,tempsamp,Nsam); // do a matched filtering to detect an impulse in our now voiceless signal


    arm_var_f32(tempsamp,NR_FFT_SIZE,&sigma2); //calculate sigma2 of the original signal ? or tempsignal

    arm_power_f32(lpcs,order,&lpc_power);  // calculate the sum of the squares (the "power") of the lpc's

    impulse_threshold = 2.5 * sqrtf(sigma2 * lpc_power);  //set a detection level (3 is not really a final setting)

    //if ((nr_setting > 20) && (nr_setting <51))
    //    impulse_threshold = impulse_threshold / (0.9 + (nr_setting-20.0)/10);  //scaling the threshold by 1 ... 0.26

    search_pos = order+PL;  // lower boundary problem has been solved! - so here we start from 1 or 0?
    impulse_count=0;

    do {        //going through the filtered samples to find an impulse larger than the threshold

        if ((tempsamp[search_pos] > impulse_threshold)||(tempsamp[search_pos] < (-impulse_threshold)))
        {
            impulse_positions[impulse_count]=search_pos - order;  // save the impulse positions and correct it by the filter delay
            impulse_count++;
            search_pos+=PL;   //  set search_pos a bit away, cause we are already repairing this area later
            //  and the next impulse should not be that close
        }

        search_pos++;

    } while ((search_pos < NR_FFT_SIZE-boundary_blank) && (impulse_count < 5));// avoid upper boundary

    //boundary handling has to be fixed later
    //as a result we now will not find any impulse in these areas

    // from here: reconstruction of the impulse-distorted audio part:

    // first we form the forward and backward prediction transfer functions from the lpcs
    // that is easy, as they are just the negated coefficients  without the leading "1"
    // we can do this in place of the lpcs, as they are not used here anymore and being recalculated in the next frame!

    arm_negate_f32(&lpcs[1],&lpcs[1],order);
    arm_negate_f32(&reverse_lpcs[0],&reverse_lpcs[0],order);


    for (int j=0; j<impulse_count; j++)
    {
        for (int k = 0; k<order; k++)   // we have to copy some samples from the original signal as
        {                           // basis for the reconstructions - could be done by memcopy

            if ((impulse_positions[j]-PL-order+k) < 0)// this solves the prediction problem at the left boundary
            {
                Rfw[k]=last_frame_end[impulse_positions[j]+k];//take the sample from the last frame
            }
            else
            {
                Rfw[k]=insamp[impulse_positions[j]-PL-order+k];//take the sample from this frame as we are away from the boundary
            }

            Rbw[impulse_length+k]=insamp[impulse_positions[j]+PL+k+1];



        }     //bis hier alles ok

        for (int i = 0; i < impulse_length; i++) //now we calculate the forward and backward predictions
        {
            arm_dot_prod_f32(&reverse_lpcs[0],&Rfw[i],order,&Rfw[i+order]);
            arm_dot_prod_f32(&lpcs[1],&Rbw[impulse_length-i],order,&Rbw[impulse_length-i-1]);

        }

        arm_mult_f32(&Wfw[0],&Rfw[order],&Rfw[order],impulse_length); // do the windowing, or better: weighing
        arm_mult_f32(&Wbw[0],&Rbw[0],&Rbw[0],impulse_length);



#ifdef debug_alternate_NR
        // in debug mode do the restoration only in some cases
        if (((ts.dsp_nr_strength > 0) && (ts.dsp_nr_strength < 6))||((ts.dsp_nr_strength > 10) && (ts.dsp_nr_strength < 16)))
        {
            // just let the distortion pass at setting 1...5 and 11...15
            //    arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);
        }
        else
        {
            //finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
            arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);
        }
#else
        //finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
        arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);

#endif
    }

    for (int p=0; p<(order+PL); p++)
    {
        last_frame_end[p]=insamp[NR_FFT_SIZE-1-order-PL+p];// store 13 samples from the current frame to use at the next frame
    }
    //end of test timing zone
}

#endif
