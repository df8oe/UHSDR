/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
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
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __AUDIO_NR_H
#define __AUDIO_NR_H

#include "uhsdr_board.h"

#ifdef USE_ALTERNATE_NR
#include "freedv_uhsdr.h"

#define NR_FFT_L NR_FFT_SIZE

typedef struct NoiseReduction
{
	float32_t __MCHF_SPECIALMEM last_iFFT_result [NR_FFT_L / 2];
	float32_t __MCHF_SPECIALMEM last_sample_buffer_L [NR_FFT_L / 2];
	float32_t __MCHF_SPECIALMEM Hk[NR_FFT_L / 2]; // gain factors
	float32_t __MCHF_SPECIALMEM FFT_buffer[NR_FFT_L * 2];
	float32_t 					X[NR_FFT_L / 2][2]; // magnitudes of the current and the last FFT bins
	float32_t __MCHF_SPECIALMEM Nest[NR_FFT_L / 2][2]; // noise estimates for the current and the last FFT frame
	float32_t __MCHF_SPECIALMEM vk; // saved 0.24kbytes
	float32_t __MCHF_SPECIALMEM SNR_prio[NR_FFT_L / 2];
	float32_t __MCHF_SPECIALMEM SNR_post[NR_FFT_L / 2];
	float32_t __MCHF_SPECIALMEM SNR_post_pos; // saved 0.24kbytes
	float32_t __MCHF_SPECIALMEM Hk_old[NR_FFT_L / 2];
	float32_t __MCHF_SPECIALMEM VAD;
	float32_t 					long_tone_gain[NR_FFT_L / 2];
	float32_t 					long_tone[NR_FFT_L / 2][2];
	int 						VAD_delay;
	int 						VAD_duration; //takes the duration of the last vowel
	uint32_t 					VAD_crash_detector; // this is counted upwards during speech detection, if noise is detected, it is reset to zero
	// if it exceeds a certain limit, noise estimate is done irrespective of the VAD value
	// this helps to get the noise estimate out of a very low position --> "VAD crash"
} NoiseReduction;

NoiseReduction NR;

void alternateNR_handle();

void do_alternate_NR();
void alt_noise_blanking();
void spectral_noise_reduction();


int NR_in_buffer_peek(NR_Buffer** c_ptr);
int NR_in_buffer_remove(NR_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int NR_in_buffer_add(NR_Buffer* c);
void NR_in_buffer_reset();
int8_t NR_in_has_data();
int32_t NR_in_has_room();


int NR_out_buffer_peek(NR_Buffer** c_ptr);
int NR_out_buffer_remove(NR_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int NR_out_buffer_add(NR_Buffer* c);
void NR_out_buffer_reset();
int8_t NR_out_has_data();
int32_t NR_out_has_room();
#endif

#endif
