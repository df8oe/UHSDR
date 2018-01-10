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

#define NR_FFT_L (NR_FFT_SIZE) // for NR FFT size 128
//#define NR_FFT_L (NR_FFT_SIZE * 2) // for NR FFT size 256

typedef struct NoiseReduction // declaration
{
	float32_t 					last_iFFT_result [NR_FFT_L / 2];
	float32_t 					last_sample_buffer_L [NR_FFT_L / 2];
	float32_t 					Hk[NR_FFT_L / 2]; // gain factors
	float32_t 					FFT_buffer[NR_FFT_L * 2];
	float32_t 					Nest[NR_FFT_L / 2][2]; // noise estimates for the current and the last FFT frame
	float32_t 					vk; // saved 0.24kbytes
	float32_t 					SNR_prio[NR_FFT_L / 2];
	float32_t 					SNR_post[NR_FFT_L / 2];
	float32_t 					SNR_post_pos; // saved 0.24kbytes
	float32_t 					Hk_old[NR_FFT_L / 2];
	float32_t 					VAD;
	float32_t					VAD_Esch; // holds the VAD sum for the Esh & Vary 2009 type of VAD
	int16_t						gain_display; // 0 = do not display gains, 1 = display bin gain in spectrum display, 2 = display long_tone_gain
	//											 3 = display bin gain multiplied with long_tone_gain
//	float32_t					notch1_f;
//	bool						notch1_enable;
//	float32_t					notch2_f;
//	bool						notch2_enable;
	//ulong						long_tone_counter;
} NoiseReduction;

// we need another struct, because of the need for strict allocation of memory for users of the
// mcHF hardware with small RAM (192 kb)
//
typedef struct NoiseReduction2 // declaration
{
	float32_t 					X[NR_FFT_L/2][2]; // magnitudes of the current and the last FFT bins
	//float32_t 					X[NR_FFT_L/2]; // magnitudes of the current and the last FFT bins
	float32_t 					long_tone_gain[NR_FFT_L / 2];
	float32_t 					long_tone[NR_FFT_L / 2][2];
	int 						VAD_delay;
	int 						VAD_duration; //takes the duration of the last vowel
	uint32_t 					VAD_crash_detector; // this is counted upwards during speech detection, if noise is detected, it is reset to zero
	// if it exceeds a certain limit, noise estimate is done irrespective of the VAD value
	// this helps to get the noise estimate out of a very low position --> "VAD crash"
	uint8_t						VAD_type; // 0 = Sohn et al. VAD, 1 = Esch & Vary 2009 VAD
	bool 						notch_change; // indicates that notch filter has to be changed
	uint32_t					long_tone_counter[NR_FFT_L / 2]; // holds the notch index for every bin, the higher, the more notchworthy is a bin
	uint8_t						notch1_bin; // frequency bin where notch filter 1 has to work
	uint8_t						max_bin; // holds the bin number of the strongest persistent tone during tone detection
	float32_t					long_tone_max; // power value of the strongest persistent tone, used for max search
	bool						notch1_active; // is notch1 active?
	bool						notch2_active; // is notch21 active?
	bool						notch3_active; // is notch3 active?
	bool						notch4_active; // is notch4 active?
	float32_t					tax; // for NR devel2: noise output smoothing time constant = -tinc/ln(0.8)
	int16_t						tax_int;
	float32_t					tap; // for NR devel2: speech prob smoothing time constant = -tinc/ln(0.9) tinc = frame time (5.33ms)
	int16_t						tap_int;
	int16_t						asnr; // for NR devel2: active SNR in dB
	float32_t					ax;
	float32_t					ap;
	float32_t					xih1;
	float32_t					xih1r;
	float32_t					pfac;
	int16_t						snr_prio_min_int;
	float32_t					snr_prio_min;

} NoiseReduction2;

extern NoiseReduction __MCHF_SPECIALMEM 	NR; // declaration, definition is in audio_nr.c
extern NoiseReduction2 						NR2; // declaration, definition is in audio_nr.c


void alternateNR_handle();

void do_alternate_NR();
void alt_noise_blanking();
void spectral_noise_reduction();
void spectral_noise_reduction_2();
void spectral_noise_reduction_3();

void AudioNr_ActivateAutoNotch(uint8_t notch1_bin, bool notch1_active);


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
