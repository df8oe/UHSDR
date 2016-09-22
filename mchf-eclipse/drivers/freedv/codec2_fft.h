/*
 * codec2_fft.h
 *
 *  Created on: 17.09.2016
 *      Author: danilo
 */

#ifndef DRIVERS_FREEDV_CODEC2_FFT_H_
#define DRIVERS_FREEDV_CODEC2_FFT_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef ARM_MATH_CM4
  #include "stm32f4xx.h"
  #include "core_cm4.h"
  #include "arm_math.h"
  #include "arm_const_structs.h"
#endif

#include "defines.h"
#include "kiss_fft.h"
#include "kiss_fftr.h"
#include "comp.h"

#ifndef ARM_MATH_CM4
    #define USE_KISS_FFT
#endif


#ifdef USE_KISS_FFT
    typedef kiss_fftr_cfg codec2_fftr_cfg;
    typedef kiss_fft_cfg codec2_fft_cfg;
#else
  typedef struct {
      arm_rfft_fast_instance_f32* instance;
      int inverse;
  } codec2_fftr_struct;

  typedef codec2_fftr_struct* codec2_fftr_cfg;

  typedef struct {
        const arm_cfft_instance_f32* instance;
        int inverse;
    } codec2_fft_struct;
  typedef codec2_fft_struct* codec2_fft_cfg;
#endif

inline codec2_fftr_cfg codec2_fftr_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem)
{
    codec2_fftr_cfg retval;
#ifdef USE_KISS_FFT
    retval = kiss_fftr_alloc(nfft, inverse_fft, mem, lenmem);
#else
    retval = malloc(sizeof(codec2_fftr_struct));
    retval->inverse  = inverse_fft;
    retval->instance = malloc(sizeof(arm_rfft_fast_instance_f32));
    arm_rfft_fast_init_f32(retval->instance,nfft);
#endif
    return retval;
}
typedef kiss_fft_scalar codec2_fft_scalar;
typedef COMP    codec2_fft_cpx;

inline void codec2_fftr(codec2_fftr_cfg cfg, codec2_fft_scalar* in, codec2_fft_cpx* out)
{

#ifdef USE_KISS_FFT
      kiss_fftr(cfg, in, (kiss_fft_cpx*)out);
#else
    arm_rfft_fast_f32(cfg->instance,in,(float*)out,cfg->inverse);
    out->imag = 0; // remove out[FFT_ENC/2]->real stored in out[0].imag
#endif
}

inline void codec2_fftr_free(codec2_fftr_cfg cfg)
{
#ifdef USE_KISS_FFT
    KISS_FFT_FREE(cfg);
#else
    free(cfg->instance);
    free(cfg);
#endif
}


inline codec2_fft_cfg codec2_fft_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem)
{
    codec2_fft_cfg retval;
#ifdef USE_KISS_FFT
    retval = kiss_fft_alloc(nfft, inverse_fft, mem, lenmem);
#else
    retval = malloc(sizeof(codec2_fft_struct));
    retval->inverse  = inverse_fft;
    switch(nfft)
    {
    case 256:
        retval->instance = &arm_cfft_sR_f32_len256;
        break;
    case 512:
        retval->instance = &arm_cfft_sR_f32_len512;
        break;
    case 1024:
        retval->instance = &arm_cfft_sR_f32_len1024;
        break;
    default:
        abort();
    }
#endif
    return retval;
}

inline void codec2_fft(codec2_fft_cfg cfg, codec2_fft_cpx* in, codec2_fft_cpx* out)
{

#ifdef USE_KISS_FFT
      kiss_fft(cfg, (kiss_fft_cpx*)in, (kiss_fft_cpx*)out);
#else
    arm_cfft_f32(cfg->instance,(float*)in,cfg->inverse,0);
    // TODO: this is not nice, but for now required to keep changes minimal
    // however, since main goal is to reduce the memory usage
    // we should convert to an in place interface
    // on PC like platforms the overhead of using the "inplace" kiss_fft calls
    // is neglectable compared to the gain in memory usage on STM32 platforms
    memcpy(out,in,cfg->instance->fftLen*2*sizeof(float));
#endif
}

inline void codec2_fft_free(codec2_fft_cfg cfg)
{
#ifdef USE_KISS_FFT
    KISS_FFT_FREE(cfg);
#else
    free(cfg);
#endif
}

#endif /* DRIVERS_FREEDV_CODEC2_FFT_H_ */
