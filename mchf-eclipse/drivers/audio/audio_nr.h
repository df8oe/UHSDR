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

#include "uhsdr_board_config.h"

#ifdef USE_ALTERNATE_NR
// this contains our shared buffer structure, since FreeDV and NR are mutually exclusive
// they can use the same set of buffers for exchanging data between audio interrupt and
// user code
#include "freedv_uhsdr.h"

#define NR_FFT_SIZE 128

// for the mcHF, we would uncomment all those four // !
//#if defined(STM32F7) || defined(STM32H7)
#define NR_FFT_L_2 (NR_FFT_SIZE * 2) // for NR FFT size 256
//#else
//#define NR_FFT_L_2 (NR_FFT_SIZE) // for NR FFT size 128
//#endif



// we need another struct, because of the need for strict allocation of memory for users of the
// mcHF hardware with small RAM (192 kb)
typedef struct NoiseReduction2 // declaration
{
    float32_t                   Hk[NR_FFT_L_2 / 2]; // gain factors
	float32_t 					X[NR_FFT_L_2 / 2][2]; // magnitudes of the current and the last FFT bins
	//float32_t 					X[NR_FFT_L/2]; // magnitudes of the current and the last FFT bins
//	float32_t 					long_tone_gain[NR_FFT_L_2 / 2];
//	float32_t 					long_tone[NR_FFT_L_2 / 2][2];
//	int 						VAD_delay;
//	int 						VAD_duration; //takes the duration of the last vowel
//	uint32_t 					VAD_crash_detector; // this is counted upwards during speech detection, if noise is detected, it is reset to zero
	// if it exceeds a certain limit, noise estimate is done irrespective of the VAD value
	// this helps to get the noise estimate out of a very low position --> "VAD crash"
//	uint8_t						VAD_type; // 0 = Sohn et al. VAD, 1 = Esch & Vary 2009 VAD
	bool 						notch_change; // indicates that notch filter has to be changed
//	uint32_t					long_tone_counter[NR_FFT_L_2 / 2]; // holds the notch index for every bin, the higher, the more notchworthy is a bin
	uint8_t						notch1_bin; // frequency bin where notch filter 1 has to work
	uint8_t						max_bin; // holds the bin number of the strongest persistent tone during tone detection
	float32_t					long_tone_max; // power value of the strongest persistent tone, used for max search
	bool						notch1_active; // is notch1 active?
	bool						notch2_active; // is notch21 active?
	bool						notch3_active; // is notch3 active?
	bool						notch4_active; // is notch4 active?
	//float32_t					tax; // for NR devel2: noise output smoothing time constant = -tinc/ln(0.8)
	//int16_t						tax_int;
	//float32_t					tap; // for NR devel2: speech prob smoothing time constant = -tinc/ln(0.9) tinc = frame time (5.33ms)
	//int16_t						tap_int;
	int16_t						asnr; // for NR devel2: active SNR in dB
	float32_t					ax;
	float32_t					ap;
	float32_t					xih1;
	float32_t					xih1r;
	float32_t					pfac;
	int16_t						snr_prio_min_int;
	float32_t					snr_prio_min;
	int16_t						NN;// for musical noise reduction
	int16_t						width;// for musical noise reduction
	float32_t 					pre_power;// for musical noise reduction
	float32_t					post_power;// for musical noise reduction
	float32_t					power_ratio; // for musical noise reduction
	float32_t					power_threshold;
	int16_t						power_threshold_int;
} NoiseReduction2;

extern NoiseReduction2  NR2; // declaration, definition is in audio_nr.c

typedef struct
{
    bool enable; // enable spectral noise reduction
    uint8_t first_time; // set to 1 for initialization of the NR variables

    float32_t alpha; // alpha smoothing constant for spectral noise reduction
    float32_t beta; // beta smoothing constant for spectral noise reduction

    //  bool gain_smooth_enable; // enable gain smoothing
    //  float32_t gain_smooth_alpha; // smoothing constant for gain smoothing in the spectral noise reduction
    //  bool long_tone_enable; // enable elimination of long tones in the spectral NR algorithm
    //  float32_t long_tone_alpha; // time constant for long tone detection
    //  int16_t long_tone_thresh; // threshold for long tone detection
    //  bool long_tone_reset; // used to reset gains of the long tone detection to 1.0
    //  int16_t vad_delay; // how many frames to delay the noise estimate after VAD has detected NOISE
    //  int16_t mode;
    //  float32_t vad_thresh; // threshold for voice activity detector in spectral noise reduction

    bool fft_256_enable; // debugging: enable FFT256 instead of FFT128 for spectral NR

    uint16_t NR_FFT_L; // resulting FFT length: 128 or 256
    uint8_t NR_FFT_LOOP_NO;
    bool NR_decimation_enable; // set to true, if we want to use another decimation step for the spectral NR leading to 6ksps sample rate
    bool NR_decimation_active; // set to true if the current buffer content is "double decimated" down to 6khz, set by the buffer producer

} audio_nr_params_t;

audio_nr_params_t nr_params;


void NR_Init(void);

void AudioNr_Prepare(void);
void AudioNr_HandleNoiseReduction(void);
void AudioNr_ActivateAutoNotch(uint8_t notch1_bin, bool notch1_active);


int NR_in_buffer_peek(NR_Buffer** c_ptr);
int NR_in_buffer_remove(NR_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int NR_in_buffer_add(NR_Buffer* c);
void NR_in_buffer_reset(void);
int8_t NR_in_has_data(void);
int32_t NR_in_has_room(void);


int NR_out_buffer_peek(NR_Buffer** c_ptr);
int NR_out_buffer_remove(NR_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int NR_out_buffer_add(NR_Buffer* c);
void NR_out_buffer_reset(void);
int8_t NR_out_has_data(void);
int32_t NR_out_has_room(void);
#endif

#endif
