/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  Description:   Code for fast convolution filtering                            **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_CONVOLUTION_H
#define __AUDIO_CONVOLUTION_H

#include "arm_math.h"
//#include "softdds.h"
#include "uhsdr_hw_i2s.h"
#include "uhsdr_board.h"
#include "codec.h"
#include "audio_driver.h"
#include "radio_management.h"
#include "usbd_audio_if.h"


#define SAMPLE_BUFFER_SIZE 				256
#define SAMPLE_BUFFER_NUM 				4
#define SAMPLE_BUFFER_FIFO_SIZE 		(SAMPLE_BUFFER_NUM + 1)

extern arm_fir_decimate_instance_f32   DECIMATE_RX_I;
extern arm_fir_decimate_instance_f32   DECIMATE_RX_Q;
//extern AudioDriverBuffer adb;

#ifdef USE_CONVOLUTION
#define FFT_CONVOLUTION_SIZE 256
#define CONVOLUTION_MAX_NO_OF_BLOCKS 8
#define CONVOLUTION_MAX_NO_OF_COEFFS 2048
#endif

#ifdef USE_CONVOLUTION

typedef struct
{
    // for convolution filtering
    float32_t				i_buffer_convolution[FFT_CONVOLUTION_SIZE / 2];
    float32_t				q_buffer_convolution[FFT_CONVOLUTION_SIZE / 2];

    float32_t				fmask[CONVOLUTION_MAX_NO_OF_BLOCKS][FFT_CONVOLUTION_SIZE * 2];
    float32_t				fftin[FFT_CONVOLUTION_SIZE * 2];
    float32_t				fftout[CONVOLUTION_MAX_NO_OF_BLOCKS][FFT_CONVOLUTION_SIZE * 2];
    float32_t				ifftout[FFT_CONVOLUTION_SIZE * 2];
    float32_t				accum[FFT_CONVOLUTION_SIZE * 2];
    float32_t               a_buffer[2][FFT_CONVOLUTION_SIZE / 2]; // for convolution, we need an output buffer of 128 samples
} ConvolutionBuffers;


typedef struct
{
    // for convolution filtering
    int						nc; // no. of coefficients
    int 					size; // no. of input samples
    int						nfor; // no. of blocks in the convolution
    int						buffidx; // buffer pointer
    int						DF; // decimation factor
    float32_t				impulse[CONVOLUTION_MAX_NO_OF_COEFFS * 2]; // impulse response has real and imaginary components
    float32_t				maskgen[FFT_CONVOLUTION_SIZE * 2];
} ConvolutionBuffersShared;

extern ConvolutionBuffersShared cbs;

void AudioDriver_CalcConvolutionFilterCoeffs (int N, float32_t f_low, float32_t f_high, float32_t samplerate, int wintype, int rtype, float32_t scale);
void AudioDriver_RxProcessorConvolution(AudioSample_t * const src, AudioSample_t * const dst, const uint16_t blockSize);
void convolution_handle(void);

#endif

#endif
