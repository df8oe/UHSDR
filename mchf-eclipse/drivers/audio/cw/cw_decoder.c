/************************************************************************************
 **                                                                                 **
 **                               UHSDR Firmware Project                            **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Licence:		GNU GPLv3                                                       **
 ************************************************************************************/
//*********************************************************************************
//**
//** Project.........: Read Hand Sent Morse Code (tolerant of considerable jitter)
//**
//** Copyright (c) 2016  Loftur E. Jonasson  (tf3lj [at] arrl [dot] net)
//**
//** This program is free software: you can redistribute it and/or modify
//** it under the terms of the GNU General Public License as published by
//** the Free Software Foundation, either version 3 of the License, or
//** (at your option) any later version.
//**
//** This program is distributed in the hope that it will be useful,
//** but WITHOUT ANY WARRANTY; without even the implied warranty of
//** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//** GNU General Public License for more details.
//**
//** The GNU General Public License is available at
//** http://www.gnu.org/licenses/
//**
//** Substantive portions of the methodology used here to decode Morse Code are found in:
//**
//** "MACHINE RECOGNITION OF HAND-SENT MORSE CODE USING THE PDP-12 COMPUTER"
//** by Joel Arthur Guenther, Air Force Institute of Technology,
//** Wright-Patterson Air Force Base, Ohio
//** December 1973
//** http://www.dtic.mil/dtic/tr/fulltext/u2/786492.pdf
//**
//** Platform........: Teensy 3.1 / 3.2 and the Teensy Audio Shield
//**
//** Initial version.: 0.00, 2016-01-25  Loftur Jonasson, TF3LJ / VE2LJX
//**
//*********************************************************************************

#include "uhsdr_board.h"
#include "ui_lcd_layouts.h"
#include "ui_driver.h"
#include "cw_decoder.h"
#include "audio_driver.h"
#include "rtty.h"
#include "cw_gen.h"
#include <stdio.h>

Goertzel cw_goertzel;

cw_config_t cw_decoder_config =
{ .sampling_freq = 12000.0, .target_freq = 750.0,
		.speed = 25,
		//		.average = 2,
		.thresh = 32000,
		.blocksize = CW_DECODER_BLOCKSIZE_DEFAULT,
		//		.AGC_enable = 0,
		.noisecancel_enable = 1,
		.spikecancel = 0,
		.use_3_goertzels = false,
		.snap_enable = true,
		.show_CW_LED = true, // menu choice whether the user wants the CW LED indicator to be working or not
};

static void CW_Decode(void);

void CwDecode_Filter_Set()
{
	// set Goertzel parameters for CW decoding
	AudioFilter_CalcGoertzel(&cw_goertzel, ts.cw_sidetone_freq , // cw_decoder_config.target_freq,
			cw_decoder_config.blocksize, 1.0, cw_decoder_config.sampling_freq);
}

//#define SIGNAL_TAU			0.01
#define SIGNAL_TAU			0.1
#define	ONEM_SIGNAL_TAU     (1.0 - SIGNAL_TAU)

#define CW_TIMEOUT			3  // Time, in seconds, to trigger display of last Character received
#define ONE_SECOND			(12000 / cw_decoder_config.blocksize) // sample rate / decimation rate / block size

//#define CW_ONE_BIT_SAMPLE_COUNT (ONE_SECOND / 5.83) // standard word PARIS has 14 pulses & 14 spaces, assumed: 25WPM
#define CW_ONE_BIT_SAMPLE_COUNT (ONE_SECOND / 58.3) // = 6.4 works ! standard word PARIS has 14 pulses & 14 spaces, assumed: 25WPM
//#define CW_ONE_BIT_SAMPLE_COUNT (ONE_SECOND / 25.0) // does not work! standard word PARIS has 14 pulses & 14 spaces, assumed: 25WPM
//#define CW_ONE_BIT_SAMPLE_COUNT (ONE_SECOND / 583.0) // does not work. standard word PARIS has 14 pulses & 14 spaces, assumed: 25WPM
//#define CW_ONE_BIT_SAMPLE_COUNT (12000 / 5.83) // standard word PARIS has 14 pulses & 14 spaces, assumed: 25WPM
//#define CW_ONE_BIT_SAMPLE_COUNT (12000) // standard word PARIS has 14 pulses & 14 spaces, assumed: 25WPM
													// 14bits * 25words per min / (60 sec/min) = 5.83 bits/sec bitrate
													// (sample_rate / blocksize) / bitrate = samples per bit !

#define CW_SPIKECANCEL_MAX_DURATION        8  // Cancel transients/spikes/drops that have max duration of number chosen.
// Typically 4 or 8 to select at time periods of 4 or 8 times 2.9ms.
// 0 to deselect.


#define CW_SIG_BUFSIZE      256  // Size of a circular buffer of decoded input levels and durations
#define CW_DATA_BUFSIZE      40  // Size of a buffer of accumulated dot/dash information. Max is DATA_BUFSIZE-2
// Needs to be significantly longer than longest symbol 'sos'= ~30.

// FIXME: replace with true/false (since already defined elsewhere)
#define  TRUE  1
#define  FALSE 0

typedef struct
{
	unsigned state :1; // Pulse or space (sample buffer) OR Dot or Dash (data buffer)
	unsigned time :31; // Time duration
} sigbuf;

typedef struct
{
	unsigned initialized :1; // Do we have valid time duration measurements?
	unsigned dash :1; // Dash flag
	unsigned wspace :1; // Word Space flag
	unsigned timeout :1; // Timeout flag
	unsigned overload :1; // Overload flag
} bflags;

static bool cw_state;                   // Current decoded signal state
static sigbuf sig[CW_SIG_BUFSIZE]; // A circular buffer of decoded input levels and durations, input from

static int32_t sig_lastrx = 0; // Circular buffer in pointer, updated by SignalSampler
static int32_t sig_incount = 0; // Circular buffer in pointer, copy of sig_lastrx, used by CW Decode functions
static int32_t sig_outcount = 0; // Circular buffer out pointer, used by CW Decode functions
static int32_t sig_timer = 0; // Elapsed time of current signal state, dependent

// on sample rate, decimation factor and CW_DECODE_BLOCK_SIZE
// 48ksps & decimation-by-4 equals 12ksps
// if CW_DECODE_BLOCK_SIZE == 32, then we have 12000/32 = 375 blocks per second, which means
// one Goertzel magnitude is calculated 375 times a second, which means 2.67ms per timer_stepsize
// this is very similar to the original 2.9ms (when using FFT256 in the Teensy 3 original sketch)
// DD4WH 2017_09_08
static int32_t timer_stepsize = 1; // equivalent to 2.67ms, see above
static int32_t cur_time;                     // copy of sig_timer
static int32_t cur_outcount = 0; // Basically same as sig_outcount, for Error Correction functionality
static int32_t last_outcount = 0; // sig_outcount for previous character, used for Error Correction func

sigbuf data[CW_DATA_BUFSIZE]; // Buffer containing decoded dot/dash and time information
// for assembly into a character
static uint8_t data_len = 0;             // Length of incoming character data

static uint32_t code; // Decoded dot/dash info in pairs of bits, - is encoded as 11, and . is encoded as 10

static bflags b;                            // Various Operational state flags

typedef struct
{
	float32_t pulse_avg; // CW timing variables - pulse_avg is a composite value
	float32_t dot_avg;
	float32_t dash_avg;            // Dot and Dash Space averages
	float32_t symspace_avg;
	float32_t cwspace_avg; // Intra symbol Space and Character-Word Space
	int32_t w_space;                      // Last word space time
} cw_times_t;

static cw_times_t cw_times;

// audio signal buffer
static float32_t raw_signal_buffer[CW_DECODER_BLOCKSIZE_MAX];  //cw_decoder_config.blocksize];


// RINGBUFFER HELPER MACROS START
#define ring_idx_wrap_upper(value,size) (((value) >= (size)) ? (value) - (size) : (value))
#define ring_idx_wrap_zero(value,size) (((value) < (0)) ? (value) + (size) : (value))

/*
 * @brief adjust index "value" by "change" while keeping it in the ring buffer size limits of "size"
 * @returns the value changed by adding change to it and doing a modulo operation on it for the ring buffer size. So return value is always 0 <= result < size
 */
#define ring_idx_change(value,change,size) (change>0 ? ring_idx_wrap_upper((value)+(change),(size)):ring_idx_wrap_zero((value)+(change),(size)))

#define ring_idx_increment(value,size) ((value+1) == (size)?0:(value+1))

#define ring_idx_decrement(value,size) ((value) == 0?(size)-1:(value)-1)

// Determine number of states waiting to be processed
#define ring_distanceFromTo(from,to) (((to) < (from))? ((CW_SIG_BUFSIZE + (to)) - ((from) )) : (to - from))

// RINGBUFFER HELPER MACROS END

static void CW_Decode_exe(void)
{
	bool newstate;
	static float32_t CW_env = 0.0;
	static float32_t CW_mag = 0.0;
	static float32_t CW_noise = 0.0;
	float32_t CW_clipped = 0.0;

	static float32_t old_siglevel = 0.001;
	static float32_t speed_wpm_avg = 0.0;
	float32_t siglevel;                	// signal level from Goertzel calculation
	static bool prevstate; 				// Last recorded state of signal input (mark or space)

	//    1.) get samples
	// these are already in raw_signal_buffer

	//    2.) calculate Goertzel
	for (uint16_t index = 0; index < cw_decoder_config.blocksize; index++)
	{
		AudioFilter_GoertzelInput(&cw_goertzel, raw_signal_buffer[index]);
	}

	float32_t magnitudeSquared = AudioFilter_GoertzelEnergy(&cw_goertzel);

	// I am not sure whether we would need an AGC here, because the audio chain already has an AGC
	// Now I am sure, we do not need it
	//    3.) AGC

#if 0

	float32_t pklvl;                   	// Used for AGC calculations
	if (cw_decoder_config.AGC_enable)
	{
		pklvl = CW_agcvol * CW_vol * magnitudeSquared; // Get level at Goertzel frequency
		if (pklvl > AGC_MAX_PEAK)
			CW_agcvol = CW_agcvol * CW_AGC_ATTACK; // Decrease volume if above this level.
		if (pklvl < AGC_MIN_PEAK)
			CW_agcvol = CW_agcvol * CW_AGC_DECAY; // Increase volume if below this level.
		if (CW_agcvol > 1.0)
			CW_agcvol = 1.0;                 // Cap max at 1.0
		siglevel = CW_agcvol * CW_vol * pklvl;
	}
	else
#endif
	{
		siglevel = magnitudeSquared;
	}
	//    4.) signal averaging/smoothing

#if 0

	static float32_t avg_win[20]; // Sliding window buffer for signal averaging, if used
	static uint8_t avg_cnt = 0;                        // Sliding window counter

	avg_win[avg_cnt] = siglevel;     // Add value onto "sliding window" buffer
	avg_cnt = ring_idx_increment(avg_cnt, cw_decoder_config.average);

	float32_t lvl = 0;                 	// Multiuse variable
	for (uint8_t x = 0; x < cw_decoder_config.average; x++) // Average up all values within sliding window
	{
		lvl = lvl + avg_win[x];
	}
	siglevel = lvl / cw_decoder_config.average;

#else
	// better use exponential averager for averaging/smoothing here !? Let�s try!
//	siglevel = siglevel * SIGNAL_TAU + ONEM_SIGNAL_TAU * old_siglevel;
//	old_siglevel = magnitudeSquared;
#endif


	// 4b.) automatic threshold correction
	if(cw_decoder_config.use_3_goertzels)
	{
	CW_mag = siglevel;
	CW_env = decayavg(CW_env, CW_mag, (CW_mag > CW_env)?
			//				(CW_ONE_BIT_SAMPLE_COUNT / 4) : (CW_ONE_BIT_SAMPLE_COUNT * 16));
				(cw_decoder_config.thresh /1000 / 4) : (cw_decoder_config.thresh /1000 * 16));

	CW_noise = decayavg(CW_noise, CW_mag, (CW_mag < CW_noise)?
			//(CW_ONE_BIT_SAMPLE_COUNT / 4) : (CW_ONE_BIT_SAMPLE_COUNT * 48));
			(cw_decoder_config.thresh /1000 / 4) : (cw_decoder_config.thresh /1000 * 48));

	CW_clipped = CW_mag > CW_env? CW_env: CW_mag;

	if (CW_clipped < CW_noise)
	{
		CW_clipped = CW_noise;
	}

	float32_t v1 = (CW_clipped - CW_noise) * (CW_env - CW_noise) -
					0.8 * ((CW_env - CW_noise) * (CW_env - CW_noise));
	//				0.85 * ((CW_env - CW_noise) * (CW_env - CW_noise));
//				 ((CW_env - CW_noise) * (CW_env - CW_noise));
//	0.25 * ((CW_env - CW_noise) * (CW_env - CW_noise));

	//lowpass

//	v1 = RttyDecoder_lowPass(v1, rttyDecoderData.lpfConfig, &rttyDecoderData.lpfData);
		siglevel = v1 * SIGNAL_TAU + ONEM_SIGNAL_TAU * old_siglevel;
		old_siglevel = v1;
//	bool newstate = (siglevel > 0)? false:true;
	newstate = (siglevel < 0)? false:true;
	}
	//    5.) signal state determination
	//----------------
	// Signal State sampling

	// noise cancel requires at least two consecutive samples to be
	// of same (changed state) to accept change (i.e. a single sample change is ignored).
	else
	{
		siglevel = siglevel * SIGNAL_TAU + ONEM_SIGNAL_TAU * old_siglevel;
		old_siglevel = magnitudeSquared;
		newstate = (siglevel >= cw_decoder_config.thresh);
	}

	if(cw_decoder_config.noisecancel_enable)
	{
		static bool change; // reads to be the same to confirm a true change

		if (change == TRUE)
		{
			cw_state = newstate;
			change = FALSE;
		}
		else if (newstate != cw_state)
		{
			change = TRUE;
		}

	}
	else
	{// No noise canceling
		cw_state = newstate;
	}

	ads.CW_signal = cw_state;
//	if(ts.dmod_mode == DEMOD_CW)
	if(cw_decoder_config.show_CW_LED == true && ts.cw_decoder_enable && ts.dmod_mode == DEMOD_CW)
		{
			Board_RedLed(cw_state == true? LED_STATE_ON : LED_STATE_OFF);
		}

	//    6.) fill into circular buffer
	//----------------
	// Record state changes and durations onto circular buffer
	if (cw_state != prevstate)
	{
		// Enter the type and duration of the state change into the circular buffer
		sig[sig_lastrx].state = prevstate;
		sig[sig_lastrx].time = sig_timer;

		// Zero circular buffer when at max
		sig_lastrx = ring_idx_increment(sig_lastrx, CW_SIG_BUFSIZE);

		sig_timer = 0;                                // Zero the signal timer.
		prevstate = cw_state;                            // Update state
	}

	//----------------
	// Count signal state timer upwards based on which sampling rate is in effect
	sig_timer = sig_timer + timer_stepsize;

	if (sig_timer > ONE_SECOND * CW_TIMEOUT)
	{
		sig_timer = ONE_SECOND * CW_TIMEOUT; // Impose a MAXTIME second boundary for overflow time
	}

	sig_incount = sig_lastrx;                         // Current Incount pointer
	cur_time = sig_timer;

	//    7.) CW Decode
	if(ts.cw_decoder_enable && ts.dmod_mode == DEMOD_CW)
	{
	  CW_Decode();                                     // Do all the heavy lifting
	}
	// calculation of speed of the received morse signal on basis of the standard "PARIS"
	float32_t spdcalc =  10.0 * cw_times.dot_avg + 4.0 * cw_times.dash_avg + 9.0 * cw_times.symspace_avg + 5.0 * cw_times.cwspace_avg;

	// update only if initialized and prevent division  by zero
	if(b.initialized == true && spdcalc > 0)
	{
		// Convert to Milliseconds per Word
		float32_t speed_ms_per_word = spdcalc * 1000.0 / (cw_decoder_config.sampling_freq / (float32_t)cw_decoder_config.blocksize);
		float32_t speed_wpm_raw = (0.5 + 60000.0 / speed_ms_per_word); // calculate words per minute
		speed_wpm_avg = speed_wpm_raw * 0.3 + 0.7 * speed_wpm_avg; // a little lowpass filtering
	}
	else
	{
		speed_wpm_avg = 0; // we have no calculated speed, i.e. not synchronized to signal
	}

	cw_decoder_config.speed = speed_wpm_avg; // for external use, 0 indicates no signal condition

	if(ts.txrx_mode == TRX_MODE_TX)
	{	// just to ensure that during RX/TX switching the red LED remains lit in TX_mode
		Board_RedLed(LED_STATE_ON);
	}
}

void CwDecode_RxProcessor(float32_t * const src, int16_t blockSize)
{
	static uint16_t sample_counter = 0;
	for (uint16_t idx = 0; idx < blockSize; idx++)
	{
		raw_signal_buffer[sample_counter] = src[idx];
		sample_counter++;
	}
	if (sample_counter >= cw_decoder_config.blocksize)
	{
		CW_Decode_exe();
		sample_counter = 0;
	}
}


//------------------------------------------------------------------
//
// Initialization Function (non-blocking-style)
// Determine Pulse, Dash, Dot and initial
// Character-Word time averages
//
// Input is the circular buffer sig[], including in and out counters
// Output is variables containing dot dash and space averages
//
//------------------------------------------------------------------
static void InitializationFunc(void)
{
	static int16_t startpos, progress;   // Progress counter, size = SIG_BUFSIZE
	static bool initializing = FALSE; // Bool for first time init of progress counter
	int16_t processed;              // Number of states that have been processed
	float32_t t;                     // We do timing calculations in floating point
	// to gain a little bit of precision when low
	// sampling rate
	// Set up progress counter at beginning of initialize
	if (initializing == FALSE)
	{
		startpos = sig_outcount;        // We start at last processed mark/space
		progress = sig_outcount;
		initializing = TRUE;
		cw_times.pulse_avg = 0;                         // Reset CW timing variables to 0
		cw_times.dot_avg = 0;
		cw_times.dash_avg = 0;
		cw_times.symspace_avg = 0;
		cw_times.cwspace_avg = 0;
		cw_times.w_space = 0;
	}
	//    Board_RedLed(LED_STATE_ON);

	// Determine number of states waiting to be processed
	processed = ring_distanceFromTo(startpos,progress);

	if (processed >= 98)
	{
		b.initialized = TRUE;                  // Indicate we're done and return
		initializing = FALSE;          // Allow for correct setup of progress if
		// InitializaitonFunc is invoked a second time
		// Board_RedLed(LED_STATE_OFF);
	}
	if (progress != sig_incount)                      // Do we have a new state?
	{
		t = sig[progress].time;

		if (sig[progress].state)                               // Is it a pulse?
		{
			if (processed > 32)                  // More than 32, getting stable
			{
				if (t > cw_times.pulse_avg)
				{
					cw_times.dash_avg = cw_times.dash_avg + (t - cw_times.dash_avg) / 4.0;    // (e.q. 4.5)
				}
				else
				{
					cw_times.dot_avg = cw_times.dot_avg + (t - cw_times.dot_avg) / 4.0;       // (e.q. 4.4)
				}
			}
			else                           // Less than 32, still quite unstable
			{
				if (t > cw_times.pulse_avg)
				{
					cw_times.dash_avg = (t + cw_times.dash_avg) / 2.0;               // (e.q. 4.2)
				}
				else
				{
					cw_times.dot_avg = (t + cw_times.dot_avg) / 2.0;                 // (e.q. 4.1)
				}
			}
			cw_times.pulse_avg = (cw_times.dot_avg / 4 + cw_times.dash_avg) / 2.0; // Update pulse_avg (e.q. 4.3)
		}
		else          // Not a pulse - determine character_word space avg
		{
			if (processed > 32)
			{
				if (t > cw_times.pulse_avg)                              // Symbol space?
				{
					cw_times.cwspace_avg = cw_times.cwspace_avg + (t - cw_times.cwspace_avg) / 4.0; // (e.q. 4.8)
				}
				else
				{
					cw_times.symspace_avg = cw_times.symspace_avg + (t - cw_times.symspace_avg) / 4.0; // New EQ, to assist calculating Rate
				}
			}
		}

		progress = ring_idx_increment(progress,CW_SIG_BUFSIZE);                                // Increment progress counter
	}
}

//------------------------------------------------------------------
//
// Spike Cancel function
//
// Optionally selectable in CWReceive.h, used by Data Recognition
// function to identify and ignore spikes of short duration.
//
//------------------------------------------------------------------

bool CwDecoder_IsSpike(uint32_t t)
{
	bool retval = false;

	if (cw_decoder_config.spikecancel == CW_SPIKECANCEL_MODE_SPIKE) // SPIKE CANCEL // Squash spikes/transients of short duration
	{
		retval = t <= CW_SPIKECANCEL_MAX_DURATION;
	}
	else if (cw_decoder_config.spikecancel == CW_SPIKECANCEL_MODE_SHORT) // SHORT CANCEL // Squash spikes shorter than 1/3rd dot duration
	{
		retval = (3 * t < cw_times.dot_avg) && (b.initialized == TRUE); // Only do this if we are not initializing dot/dash periods
	}
	return retval;
}


float32_t spikeCancel(float32_t t)
{
	static bool spike;

	if (cw_decoder_config.spikecancel != CW_SPIKECANCEL_MODE_OFF)
	{
		if (CwDecoder_IsSpike(t) == true)
		{
			spike = TRUE;
			sig_outcount = ring_idx_increment(sig_outcount, CW_SIG_BUFSIZE); // If short, then do nothing
			t = 0.0;
		}
		else if (spike == TRUE) // Check if last state was a short Spike or Drop
		{
			spike = FALSE;
			// Add time of last three states together.
			t =		t
					+ sig[ring_idx_change(sig_outcount, -1, CW_SIG_BUFSIZE)].time
					+ sig[ring_idx_change(sig_outcount, -2, CW_SIG_BUFSIZE)].time;
		}
	}

	return t;
}

//------------------------------------------------------------------
//
// Data Recognition Function (non-blocking-style)
// Decode dots, dashes and spaces and group together
// into a character.
//
// Input is the circular buffer sig[], including in and out counters
// Variables containing dot, dash and space averages are maintained, and
// output is a data[] buffer containing decoded dot/dash information, a
// data_len variable containing length of incoming character data.
// The function returns TRUE when further calls will not yield a change or a complete new character has been decoded.
// The bool variable the parameter points to is set to true if a new character has been decoded
// In addition, b.wspace flag indicates whether long (word) space after char
//
//------------------------------------------------------------------
bool DataRecognitionFunc(bool* new_char_p)
{
	bool not_done = FALSE;                  // Return value
	static bool processed;

	*new_char_p = FALSE;

	//-----------------------------------
	// Do we have a new state to process?
	if (sig_outcount != sig_incount)
	{
		not_done = true;
		b.timeout = FALSE;           // Mainly used by Error Correction Function

		const float32_t t = spikeCancel(sig[sig_outcount].time); // Get time of the new state
		// Squash spikes/transients if enabled
		// Attention: Side Effect -> sig_outcount has been be incremented inside spikeCancel if result == 0, because of this we increment only if not 0

		if (t > 0) // not a spike (or spike processing not enabled)
		{
			const bool is_markstate = sig[sig_outcount].state;

			sig_outcount = ring_idx_increment(sig_outcount, CW_SIG_BUFSIZE); // Update process counter
			//-----------------------------------
			// Is it a Mark (keydown)?
			if (is_markstate == true)
			{
				processed = FALSE; // Indicate that incoming character is not processed

				// Determine if Dot or Dash (e.q. 4.10)
				if ((cw_times.pulse_avg - t) >= 0)                         // It is a Dot
				{
					b.dash = FALSE;                           // Clear Dash flag
					data[data_len].state = 0;                   // Store as Dot
					cw_times.dot_avg = cw_times.dot_avg + (t - cw_times.dot_avg) / 8.0; // Update cw_times.dot_avg (e.q. 4.6)
				}
				//-----------------------------------
				// Is it a Dash?
				else
				{
					b.dash = TRUE;                              // Set Dash flag
					data[data_len].state = 1;                   // Store as Dash
					if (t <= 5 * cw_times.dash_avg)        // Store time if not stuck key
					{
						cw_times.dash_avg = cw_times.dash_avg + (t - cw_times.dash_avg) / 8.0; // Update dash_avg (e.q. 4.7)
					}
				}

				data[data_len].time = (uint32_t) t;     // Store associated time
				data_len++;                         // Increment by one dot/dash
				cw_times.pulse_avg = (cw_times.dot_avg / 4 + cw_times.dash_avg) / 2.0; // Update pulse_avg (e.q. 4.3)
			}

			//-----------------------------------
			// Is it a Space?
			else
			{
				bool full_char_detected = true;
				if (b.dash == TRUE)                // Last character was a dash
				{
				    b.dash = false;
				    float32_t eq4_12 = t
				            - (cw_times.pulse_avg
				                    - ((uint32_t) data[data_len - 1].time
				                            - cw_times.pulse_avg) / 4.0); // (e.q. 4.12, corrected)
				    if (eq4_12 < 0) // Return on symbol space - not a full char yet
				    {
				        cw_times.symspace_avg = cw_times.symspace_avg + (t - cw_times.symspace_avg) / 8.0; // New EQ, to assist calculating Rat
				        full_char_detected = false;
				    }
				    else if (t <= 10 * cw_times.dash_avg) // Current space is not a timeout
				    {
				        float32_t eq4_14 = t
				                - (cw_times.cwspace_avg
				                        - ((uint32_t) data[data_len - 1].time
				                                - cw_times.pulse_avg) / 4.0); // (e.q. 4.14)
				        if (eq4_14 >= 0)                   // It is a Word space
				        {
				            cw_times.w_space = t;
				            b.wspace = TRUE;
				        }
				    }
				}
				else                                 // Last character was a dot
				{
					// (e.q. 4.11)
					if ((t - cw_times.pulse_avg) < 0) // Return on symbol space - not a full char yet
					{
						cw_times.symspace_avg = cw_times.symspace_avg + (t - cw_times.symspace_avg) / 8.0; // New EQ, to assist calculating Rate
						full_char_detected = false;
					}
					else if (t <= 10 * cw_times.dash_avg) // Current space is not a timeout
					{
						cw_times.cwspace_avg = cw_times.cwspace_avg + (t - cw_times.cwspace_avg) / 8.0; // (e.q. 4.9)

						// (e.q. 4.13)
						if ((t - cw_times.cwspace_avg) >= 0)        // It is a Word space
						{
							cw_times.w_space = t;
							b.wspace = TRUE;
						}
					}
				}
				// Process the character
				if (full_char_detected == true && processed == FALSE)
				{
					*new_char_p = TRUE; // Indicate there is a new char to be processed
				}
			}
		}
	}
	//-----------------------------------
	// Long key down or key up
	else if (cur_time > (10 * cw_times.dash_avg))
	{
		// If current state is Key up and Long key up then  Char finalized
		if (sig[sig_incount].state == false && processed == false)
		{
			processed = TRUE;
			b.wspace = TRUE;
			b.timeout = TRUE;
			*new_char_p = TRUE;                         // Process the character
		}
	}

	if (data_len > CW_DATA_BUFSIZE - 2)
	{
		data_len = CW_DATA_BUFSIZE - 2; // We're receiving garble, throw away
	}

	if (*new_char_p)       // Update circular buffer pointers for Error function
	{
		last_outcount = cur_outcount;
		cur_outcount = sig_outcount;
	}
	return not_done;  // FALSE if all data processed or new character, else TRUE
}

//------------------------------------------------------------------
//
// The Code Generation Function converts the received
// character to a string code[] of dots and dashes
//
//------------------------------------------------------------------
void CodeGenFunc()
{
	uint8_t a;
	code = 0;

	for (a = 0; a < data_len; a++)
	{
		code *= 4;
		if (data[a].state)
		{
			code += 3; // Dash
		}
		else
		{
			code += 2; // Dit
		}
	}
	data_len = 0;                               // And make ready for a new Char
}


void lcdLineScrollPrint(char c)
{
	UiDriver_TextMsgPutChar(c);
}

//------------------------------------------------------------------
//
// The Print Character Function prints to LCD and Serial (USB)
//
//------------------------------------------------------------------
void PrintCharFunc(uint8_t c)
{
	//--------------------------------------

	//--------------------------------------
	// Print Characters to LCD

	//--------------------------------------
	// Prosigns
	if (c == '}')
	{
		lcdLineScrollPrint('c');
		lcdLineScrollPrint('t');
	}
	else if (c == '(')
	{
		lcdLineScrollPrint('k');
		lcdLineScrollPrint('n');
	}
	else if (c == '&')
	{
		lcdLineScrollPrint('a');
		lcdLineScrollPrint('s');
	}
	else if (c == '~')
	{
		lcdLineScrollPrint('s');
		lcdLineScrollPrint('n');
	}
	else if (c == '>')
	{
		lcdLineScrollPrint('s');
		lcdLineScrollPrint('k');
	}
	else if (c == '+')
	{
		lcdLineScrollPrint('a');
		lcdLineScrollPrint('r');
	}
	else if (c == '^')
	{
		lcdLineScrollPrint('b');
		lcdLineScrollPrint('k');
	}
	else if (c == '{')
	{
		lcdLineScrollPrint('c');
		lcdLineScrollPrint('l');
	}
	else if (c == '^')
	{
		lcdLineScrollPrint('a');
		lcdLineScrollPrint('a');
	}
	else if (c == '%')
	{
		lcdLineScrollPrint('n');
		lcdLineScrollPrint('j');
	}
	else if (c == 0x7f)
	{
		lcdLineScrollPrint('e');
		lcdLineScrollPrint('r');
		lcdLineScrollPrint('r');
	}

	//--------------------------------------
	// # is our designated ERROR Symbol
	else if (c == 0xff)
	{
		lcdLineScrollPrint('#');
	}

	//--------------------------------------
	// Normal Characters


	/*	if (c == 0xfe || c == 0xff)
	{
		lcdLineScrollPrint('#');
	}
	 */
	else
	{
		lcdLineScrollPrint(c);
	}
}

//------------------------------------------------------------------
//
// The Word Space Function takes care of Word Spaces
// to LCD and Serial (USB).
// Word Space Correction is applied if certain characters, which
// are less likely to be at the end of a word, are received
// The characters tested are applicable to the English language
//
//------------------------------------------------------------------
void WordSpaceFunc(uint8_t c)
{
	if (b.wspace == TRUE)                             // Print word space
	{
		b.wspace = FALSE;

		// Word space correction routine - longer space required if certain characters
		if ((c == 'I') || (c == 'J') || (c == 'Q') || (c == 'U') || (c == 'V')
				|| (c == 'Z'))
		{
			int16_t x = (cw_times.cwspace_avg + cw_times.pulse_avg) - cw_times.w_space;      // (e.q. 4.15)
			if (x < 0)
			{
				lcdLineScrollPrint(' ');
			}
		}
		else
		{
			lcdLineScrollPrint(' ');
		}
	}

}

//------------------------------------------------------------------
//
// Error Correction Function has three parts
// 1) Exits with Error if character is too long (DATA_BUFSIZE-2)
// 2) If a dot duration is determined to be less than half expected,
// then this dot is eliminated by adding it and the two spaces on
// either side to for a new space duration, then new code is generated
// for pattern parsing.
// 3) If not 2) then separate two run-on characters caused by
// a short character space - Extend the char space and reprocess
//
// If not able to resolve anything, then return FALSE
// Return TRUE if something was resolved.
//
//------------------------------------------------------------------
bool ErrorCorrectionFunc(void)
{
	bool result = FALSE; // Result of Error resolution - FALSE if nothing resolved

	if (data_len >= CW_DATA_BUFSIZE - 2)     // Too long char received
	{
		PrintCharFunc(0xff);              // Print Error to LCD and Serial (USB)
		WordSpaceFunc(0xff); // Print Word Space to LCD and Serial when required
	}

	else
	{
		b.wspace = FALSE;
		//-----------------------------------------------------
		// Find the location of pulse with shortest duration
		// and the location of symbol space of longest duration
		int32_t temp_outcount = last_outcount; // Grab a copy of endpos for last successful decode
		int32_t slocation = last_outcount; // Long symbol space duration and location
		int32_t plocation = last_outcount; // Short pulse duration and location
		uint32_t pduration = UINT32_MAX; // Very high number to decrement for min pulse duration
		uint32_t sduration = 0; // and a zero to increment for max symbol space duration

		// if cur_outcount is < CW_SIG_BUFSIZE, loop must terminate after CW_SIG_BUFSIZE -1 steps
		while (temp_outcount != cur_outcount)
		{
			//-----------------------------------------------------
			// Find shortest pulse duration. Only test key-down states
			if (sig[temp_outcount].state)
			{
				bool is_shortest_pulse = sig[temp_outcount].time < pduration;
				// basic test -> shorter than all previously seen ones

				bool is_not_spike = CwDecoder_IsSpike(sig[temp_outcount].time) == false;

				if (is_shortest_pulse == true && is_not_spike == true)
				{
					pduration = sig[temp_outcount].time;
					plocation = temp_outcount;
				}
			}

			//-----------------------------------------------------
			// Find longest symbol space duration. Do not test first state
			// or last state and only test key-up states
			if ((temp_outcount != last_outcount)
					&& (temp_outcount != (cur_outcount - 1))
					&& (!sig[temp_outcount].state))
			{
				if (sig[temp_outcount].time > sduration)
				{
					sduration = sig[temp_outcount].time;
					slocation = temp_outcount;
				}
			}

			temp_outcount = ring_idx_increment(temp_outcount,CW_SIG_BUFSIZE);
		}

		uint8_t decoded[] = { 0xff, 0xff };

		//-----------------------------------------------------
		// Take corrective action by dropping shortest pulse
		// if shorter than half of cw_times.dot_avg
		// This can result in one or more valid characters - or Error
		if ((pduration < cw_times.dot_avg / 2) && (plocation != temp_outcount))
		{
			// Add up duration of short pulse and the two spaces on either side,
			// as space at pulse location + 1
			sig[ring_idx_change(plocation, +1, CW_SIG_BUFSIZE)].time =
					sig[ring_idx_change(plocation, -1, CW_SIG_BUFSIZE)].time
					+ sig[plocation].time
					+ sig[ring_idx_change(plocation, +1, CW_SIG_BUFSIZE)].time;

			// Shift the preceding data forward accordingly
			temp_outcount = ring_idx_change(plocation, -2 ,CW_SIG_BUFSIZE);

			// if last_outcount is < CW_SIG_BUFSIZE, loop must terminate after CW_SIG_BUFSIZE -1 steps
			while (temp_outcount != last_outcount)
			{
				sig[ring_idx_change(temp_outcount, +2, CW_SIG_BUFSIZE)].time =
						sig[temp_outcount].time;

				sig[ring_idx_change(temp_outcount, +2, CW_SIG_BUFSIZE)].state =
						sig[temp_outcount].state;


				temp_outcount = ring_idx_decrement(temp_outcount,CW_SIG_BUFSIZE);
			}
			// And finally shift the startup pointer similarly
			sig_outcount = ring_idx_change(last_outcount, +2,CW_SIG_BUFSIZE);
			//
			// Now we reprocess
			//
			// Pull out a character, using the adjusted sig[] buffer
			// Process character delimited by character or word space
			bool dummy;
			while (DataRecognitionFunc(&dummy))
			{
				// nothing
			}

			CodeGenFunc();                 // Generate a dot/dash pattern string
			decoded[0] = CwGen_CharacterIdFunc(code); // Convert dot/dash data into a character
			if (decoded[0] != 0xff)
			{
				PrintCharFunc(decoded[0]);
				result = TRUE;                // Error correction had success.
			}
			else
			{
				PrintCharFunc(0xff);
			}
		}
		//-----------------------------------------------------
		// Take corrective action by converting the longest symbol space to character space
		// This will result in two valid characters - or Error
		else
		{
			// Split char in two by adjusting time of longest sym space to a char space
			sig[slocation].time =
					((cw_times.cwspace_avg - 1) >= 1 ? cw_times.cwspace_avg - 1 : 1); // Make sure it is always larger than 0
			sig_outcount = last_outcount; // Set circ buffer reference to the start of previous failed decode
			//
			// Now we reprocess
			//
			// Debug - If timing is out of whack because of noise, with rate
			// showing at >99 WPM, then DataRecognitionFunc() occasionally fails.
			// Not found out why, but millis() is used to guards against it.

			// Process first character delimited by character or word space
			bool dummy;
			while (DataRecognitionFunc(&dummy))
			{
				// nothing
			}

			CodeGenFunc();                 // Generate a dot/dash pattern string
			decoded[0] = CwGen_CharacterIdFunc(code); // Convert dot/dash pattern into a character
			// Process second character delimited by character or word space

			while (DataRecognitionFunc(&dummy))
			{
				// nothing
			}
			CodeGenFunc();                 // Generate a dot/dash pattern string
			decoded[1] = CwGen_CharacterIdFunc(code); // Convert dot/dash pattern into a character

			if ((decoded[0] != 0xff) && (decoded[1] != 0xff)) // If successful error resolution
			{
				PrintCharFunc(decoded[0]);
				PrintCharFunc(decoded[1]);
				result = TRUE;                // Error correction had success.
			}
			else
			{
				PrintCharFunc(0xff);
			}
		}
	}
	return result;
}

//------------------------------------------------------------------
//
// CW Decode manages all the decode Functions.
// It establishes dot/dash/space periods through the Initialization
// function, and when initialized (or if excessive time when not fully
// initialized), then it runs DataRecognition, CodeGen and CharacterId
// functions to decode any incoming data.  If not successful decode
// then ErrorCorrection is attempted, and if that fails, then
// Initialization is re-performed.
//
//------------------------------------------------------------------
void CW_Decode(void)
{
	//-----------------------------------
	// Initialize pulse_avg, dot_avg, cw_times.dash_avg, cw_times.symspace_avg, cwspace_avg
	if (b.initialized == FALSE)
	{
		InitializationFunc();
	}

	//-----------------------------------
	// Process the works once initialized - or if timeout
	if ((b.initialized == TRUE) || (cur_time >= ONE_SECOND * CW_TIMEOUT)) //
	{
		bool received;                       // True on a symbol received
		DataRecognitionFunc(&received);      // True if new character received
		if (received && (data_len > 0))      // also make sure it is not a spike
		{
			CodeGenFunc();                 	// Generate a dot/dash pattern string

			uint8_t decoded = CwGen_CharacterIdFunc(code);
			// Identify the Character
			// 0xff if char not recognized

			if (decoded < 0xfe)        // 0xfe = spike suppression, 0xff = error
			{
				PrintCharFunc(decoded);         // Print to LCD and Serial (USB)
				WordSpaceFunc(decoded); 		// Print Word Space to LCD and Serial when required
			}
			else if (decoded == 0xff)                // Attempt Error Correction
			{
				// If Error Correction function cannot resolve, then reinitialize speed
				if (ErrorCorrectionFunc() == FALSE)
				{
					b.initialized = FALSE;
				}
			}
		}
	}
}

void CwDecoder_WpmDisplayClearOrPrepare(bool prepare)
{
    uint16_t color1 = prepare?White:Black;
    uint16_t color2 = prepare?Green:Black;

    UiLcdHy28_PrintText(ts.Layout->CW_DECODER_WPM.x, ts.Layout->CW_DECODER_WPM.y," --",color1,Black,0);
    UiLcdHy28_PrintText(ts.Layout->CW_DECODER_WPM.x + 27, ts.Layout->CW_DECODER_WPM.y, "wpm", color2, Black, 4);

    if (prepare == true)
    {
        CwDecoder_WpmDisplayUpdate(true);
    }
}

void CwDecoder_WpmDisplayUpdate(bool force_update)
{
	static uint8_t old_speed = 0;

	if(cw_decoder_config.speed != old_speed || force_update == true)
	{
	    char WPM_str[10];

	    snprintf(WPM_str, 10, cw_decoder_config.speed > 0? "%3u" : " --", cw_decoder_config.speed);

		UiLcdHy28_PrintText(ts.Layout->CW_DECODER_WPM.x, ts.Layout->CW_DECODER_WPM.y, WPM_str,White,Black,0);
	}
}

