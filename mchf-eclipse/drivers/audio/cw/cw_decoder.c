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
#include "ui_driver.h"
#include "cw_decoder.h"
#include "audio_driver.h"

Goertzel cw_goertzel;

cw_config_t cw_decoder_config =
{ .sampling_freq = 12000.0, .target_freq = 700.0,
		.speed = 25,
//		.average = 2,
		.thresh = 15000,
		.blocksize = CW_DECODER_BLOCKSIZE_DEFAULT,
//		.AGC_enable = 0,
		.noisecancel_enable = 1,
		.spikecancel = 0,
		.use_3_goertzels = false,
		.snap_enable = false
};

static void CW_Decode(void);

void CwDecode_FilterInit()
{
	// set Goertzel parameters for CW decoding
	AudioFilter_CalcGoertzel(&cw_goertzel, ts.cw_sidetone_freq , // cw_decoder_config.target_freq,
			cw_decoder_config.blocksize, 1.0, cw_decoder_config.sampling_freq);
}

#define SIGNAL_TAU			0.01
#define	ONEM_SIGNAL_TAU     (1.0 - SIGNAL_TAU)

#define CW_TIMEOUT			3  // Time, in seconds, to trigger display of last Character received
#define ONE_SECOND			(12000 / cw_decoder_config.blocksize) // sample rate / decimation rate / block size

#define CW_SPIKECANCEL_MAX_DURATION        8  // Cancel transients/spikes/drops that have max duration of number chosen.
// Typically 4 or 8 to select at time periods of 4 or 8 times 2.9ms.
// 0 to deselect.

//-----------------------------------------------------------------------------
// Decode of International Morse Code Symbols - a somewhat random collection of country specific symbols
// If you are not using these, then better not to enable.  The fewer unnecessary symbols - the more "meat"
// the Error Correction function gets to resolve
#define CW_ICELAND_SYMBOLS    0  // Þ Ð Æ Ö
#define CW_NOR_DEN_SYMBOLS    0  // Æ Å Ø   - Norway/Denmark - overlaps with Iceland symbols, only select one
#define CW_SWEDEN_SYMBOLS     0  // Ä Å Ö   - Sweden - overlaps with above symbols, only select one
#define CW_REST_SYMBOLS       1  // Ü Ch É Ñ  - a random colletion of symbols

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

static sigbuf data[CW_DATA_BUFSIZE]; // Buffer containing decoded dot/dash and time information
// for assembly into a character
static uint8_t data_len = 0;             // Length of incoming character data

static char code[CW_DATA_BUFSIZE]; // Decoded dot/dash info in symbol form, e.g. ".-.."

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
#define ring_distanceFromTo(from,to) (((to) < (from))? (CW_SIG_BUFSIZE - ((from) + (to))) : (to - from))

// RINGBUFFER HELPER MACROS END

static void CW_Decode_exe(void)
{
	static float32_t old_siglevel = 0.001;
	static float32_t speed_wpm_avg = 0.0;
	float32_t siglevel;                	// signal level from Goertzel calculation
//	float32_t lvl = 0;                 	// Multiuse variable
//	float32_t pklvl;                   	// Used for AGC calculations
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

	lvl = 0;
	for (uint8_t x = 0; x < cw_decoder_config.average; x++) // Average up all values within sliding window
	{
		lvl = lvl + avg_win[x];
	}
	siglevel = lvl / cw_decoder_config.average;

#else
	// better use exponential averager for averaging/smoothing here !? Let´s try!
	siglevel = siglevel * SIGNAL_TAU + ONEM_SIGNAL_TAU * old_siglevel;
	old_siglevel = magnitudeSquared;
#endif

	//    5.) signal state determination
	//----------------
	// Signal State sampling

	if(cw_decoder_config.noisecancel_enable)
	{
		static bool newstate, change; // reads to be the same to confirm a true change
		if (siglevel >= cw_decoder_config.thresh)
		{
			newstate = TRUE;
		}
		else
		{
			newstate = FALSE;
		}
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
		if (siglevel >= cw_decoder_config.thresh)
		{
			cw_state = TRUE;
		}
		else
		{
			cw_state = FALSE;
		}
	}
	if(cw_state == true)
	{
		Board_RedLed(LED_STATE_ON);
		ads.CW_signal = true;
	}
	else
	{
		Board_RedLed(LED_STATE_OFF);
		ads.CW_signal = false;
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
	CW_Decode();                                     // Do all the heavy lifting

	// calculation of speed of the received morse signal on basis of the standard "PARIS"
	float32_t spdcalc = 10.0 * cw_times.dot_avg + 4.0 * cw_times.dash_avg + 9.0 * cw_times.symspace_avg + 5.0 * cw_times.cwspace_avg;

	// prevent division by zero in first round
	if(spdcalc > 0)
	{

		// Convert to Milliseconds per Word
		float32_t speed_ms_per_word = spdcalc * 1000.0 / (cw_decoder_config.sampling_freq / (float32_t)cw_decoder_config.blocksize);

		float32_t speed_wpm_raw = (0.5 + 60000.0 / speed_ms_per_word); // calculate words per minute
		speed_wpm_avg = speed_wpm_raw * 0.1 + 0.9 * speed_wpm_avg; // a little lowpass filtering
		cw_decoder_config.speed = speed_wpm_avg; // convert to integer
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
	double t;                     // We do timing calculations in floating point
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
//        Board_RedLed(LED_STATE_OFF);

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
float32_t spikeCancel(float32_t t)
{
	static bool spike;

	if (cw_decoder_config.spikecancel != CW_SPIKECANCEL_MODE_OFF)
	{
		bool do_spike_cancel = false;

		if (cw_decoder_config.spikecancel == CW_SPIKECANCEL_MODE_SPIKE) // SPIKE CANCEL // Squash spikes/transients of short duration
		{
			do_spike_cancel = t <= CW_SPIKECANCEL_MAX_DURATION;
		}
		else if (cw_decoder_config.spikecancel == CW_SPIKECANCEL_MODE_SHORT) // SHORT CANCEL // Squash spikes shorter than 1/3rd dot duration
		{
			do_spike_cancel = ((3 * t < cw_times.dot_avg) && (b.initialized == TRUE)); // Only do this if we are not initializing dot/dash periods
		}

		if (do_spike_cancel == true)
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

		double t = spikeCancel(sig[sig_outcount].time); // Get time of the new state
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
					double eq4_12 = t
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
						double eq4_14 = t
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
		if (!sig[sig_incount].state && !processed)
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
	for (a = 0; a < data_len; a++)
	{
		if (data[a].state)
		{
			code[a] = '-';
		}
		else
		{
			code[a] = '.';
		}
	}
	code[a] = 0;                                      // Terminate string
	data_len = 0;                               // And make ready for a new Char
}

//------------------------------------------------------------------
//
// The Character Identification Function applies dot/dash pattern
// recognition to identify the received character.
//
// The function returns the ASCII code for the character received,
// or 0xff if pattern was not recognized.
//
//------------------------------------------------------------------
uint8_t CharacterIdFunc(void)
{
	uint8_t out;
	// FIXME: UHSDR CW Keyer Decoder has a more efficient way to do the same thing
	// and we should not have twice the "same" function. It uses a different way to encode
	// so that virtually all CW signs (all upto 8 symbols long dit/dash combinations) can be encoded in a 16bit uint.

	// Should never happen - Empty, spike suppression or similar
	if (code[0] == 0)
	{
		out = 0xfe;
	}
	// TODO, there are a number of ways to make this faster,
	// but this doesn't seem to be a bottleneck
	else if (strcmp(code, ".-") == 0)
		out = 'A';
	else if (strcmp(code, "-...") == 0)
		out = 'B';
	else if (strcmp(code, "-.-.") == 0)
		out = 'C';
	else if (strcmp(code, "-..") == 0)
		out = 'D';
	else if (strcmp(code, ".") == 0)
		out = 'E';
	else if (strcmp(code, "..-.") == 0)
		out = 'F';
	else if (strcmp(code, "--.") == 0)
		out = 'G';
	else if (strcmp(code, "....") == 0)
		out = 'H';
	else if (strcmp(code, "..") == 0)
		out = 'I';
	else if (strcmp(code, ".---") == 0)
		out = 'J';
	else if (strcmp(code, "-.-") == 0)
		out = 'K';
	else if (strcmp(code, ".-..") == 0)
		out = 'L';
	else if (strcmp(code, "--") == 0)
		out = 'M';
	else if (strcmp(code, "-.") == 0)
		out = 'N';
	else if (strcmp(code, "---") == 0)
		out = 'O';
	else if (strcmp(code, ".--.") == 0)
		out = 'P';
	else if (strcmp(code, "--.-") == 0)
		out = 'Q';
	else if (strcmp(code, ".-.") == 0)
		out = 'R';
	else if (strcmp(code, "...") == 0)
		out = 'S';
	else if (strcmp(code, "-") == 0)
		out = 'T';
	else if (strcmp(code, "..-") == 0)
		out = 'U';
	else if (strcmp(code, "...-") == 0)
		out = 'V';
	else if (strcmp(code, ".--") == 0)
		out = 'W';
	else if (strcmp(code, "-..-") == 0)
		out = 'X';
	else if (strcmp(code, "-.--") == 0)
		out = 'Y';
	else if (strcmp(code, "--..") == 0)
		out = 'Z';

	else if (strcmp(code, ".----") == 0)
		out = '1';
	else if (strcmp(code, "..---") == 0)
		out = '2';
	else if (strcmp(code, "...--") == 0)
		out = '3';
	else if (strcmp(code, "....-") == 0)
		out = '4';
	else if (strcmp(code, ".....") == 0)
		out = '5';
	else if (strcmp(code, "-....") == 0)
		out = '6';
	else if (strcmp(code, "--...") == 0)
		out = '7';
	else if (strcmp(code, "---..") == 0)
		out = '8';
	else if (strcmp(code, "----.") == 0)
		out = '9';
	else if (strcmp(code, "-----") == 0)
		out = '0';

	else if (strcmp(code, ".-.-.-") == 0)
		out = '.';
	else if (strcmp(code, "--..--") == 0)
		out = ',';
	else if (strcmp(code, "-....-") == 0)
		out = '-';
	else if (strcmp(code, "-...-") == 0)
		out = '=';
	else if (strcmp(code, "..--.-") == 0)
		out = '_';
	else if (strcmp(code, "-..-.") == 0)
		out = '/';
	else if (strcmp(code, ".-..-.") == 0)
		out = '\"';
	else if (strcmp(code, "..--..") == 0)
		out = '?';
	else if (strcmp(code, "-.-.--") == 0)
		out = '!';
	else if (strcmp(code, ".--.-.") == 0)
		out = '@';
	else if (strcmp(code, "---...") == 0)
		out = ':';
	else if (strcmp(code, "-.-.-.") == 0)
		out = ';';
	else if (strcmp(code, "-.--.-") == 0)
		out = ')';
	else if (strcmp(code, "...-..-") == 0)
		out = '$';

	// Prosign codes
	else if (strcmp(code, "-.--.") == 0)
		out = '(';   // ( and KN - Over to you only
	else if (strcmp(code, ".-...") == 0)
		out = '&';   // & and AS - Wait
	else if (strcmp(code, ".-.-.") == 0)
		out = '+';   // + and AR - End of message
	else if (strcmp(code, "-.-.-") == 0)
		out = '}';   // KA - Starting signal
	else if (strcmp(code, "...-.") == 0)
		out = '~';   // SN - Understood
	else if (strcmp(code, "...-.-") == 0)
		out = '>';   // SK . End of contact
	else if (strcmp(code, "-...-.-") == 0)
		out = '^';   // BK - Break
	else if (strcmp(code, "-.-..-..") == 0)
		out = '{';   // CL - Closing station
	else if (strcmp(code, "........") == 0)
		out = 0x7f; // Error / Correction
	else if (strcmp(code, "...---...") == 0)
		out = '!';  // SOS

	// This is a somewhat random collection of International Characters,
	// including Icelandic, Norwegian/Danish, Swedish...
#if ICELAND_SYMBOLS
	else if (strcmp(code,".--..") == 0) out = 0;     // 'Þ'
	else if (strcmp(code,"..--.") == 0) out = 1;// 'Ð'
#endif
#if ICELAND_SYMBOLS || NOR_DEN_SYMBOLS
	else if (strcmp(code,".-.-") == 0) out = 2;     // 'Æ' - overlaps with 'Ä'
#endif
#if SWEDEN_SYMBOLS
	else if (strcmp(code,".-.-") == 0) out = 6;     // 'Ä' - overlaps with 'Æ'
#endif
#if ICELAND_SYMBOLS || SWEDEN_SYMBOLS
	else if (strcmp(code,"---.") == 0) out = 3;     // 'Ö'  - overlaps with 'Ø'
#endif
#if NOR_DEN_SYMBOLS
	else if (strcmp(code,"---.") == 0) out = 4;     // 'Ø'  - overlaps with 'Ö'
#endif
#if NOR_DEN_SYMBOLS || SWEDEN_SYMBOLS
	else if (strcmp(code,".--.-") == 0) out = 5;     // 'Å'
#endif
#if REST_SYMBOLS
	else if (strcmp(code,"..--") == 0) out = 7;     // 'Ü'
	else if (strcmp(code,"..-..") == 0) out = 8;// 'É'
	else if (strcmp(code,"--.--") == 0) out = 9;// 'Ñ'
	else if (strcmp(code,"----") == 0) out = 10;// 'Ch'
#endif

	else                        // No code identified - error correction routine
	{
		out = 0xff;                           // 0xff selected to indicate ERROR
	}
	return out;
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
	// International UTF-8 encoded (non ASCII) characters
#if ICELAND_SYMBOLS || NOR_DAN_SYMBOLS || SWEDEN_SYMBOLS || REST_SYMBOLS
	char tmp[3];                         // Accommodate UTF-8 encoded characters
	if (c == 0)
	{
		strcpy(tmp,"Þ");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 1)
	{
		strcpy(tmp,"Ð");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 2)
	{
		strcpy(tmp,"Æ");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 3)
	{
		strcpy(tmp,"Ö");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 4)
	{
		strcpy(tmp,"Ø");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 5)
	{
		strcpy(tmp,"Å");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 6)
	{
		strcpy(tmp,"Ä");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 7)
	{
		strcpy(tmp,"Ü");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}

	else if (c == 8)
	{
		strcpy(tmp,"É");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 9)
	{
		strcpy(tmp,"Ñ");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
	else if (c == 10)
	{
		strcpy(tmp,"Ch");
		lcdLineScrollPrint(tmp[0]);
		lcdLineScrollPrint(tmp[1]);
	}
#endif

		//--------------------------------------
	 // Prosigns
	 if (c == '}')
	 {
	 lcdLineScrollPrint('k');
	 lcdLineScrollPrint('a');
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
	 else if (c == '!')
	 {
	 lcdLineScrollPrint('s');
	 lcdLineScrollPrint('o');
	 lcdLineScrollPrint('s');
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
		// Debug
//    Serial.println();
//    Serial.println("{{long_char}}");
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
		int32_t pduration = 32767; // Very high number to decrement for min pulse duration
		int32_t sduration = 0; // and a zero to increment for max symbol space duration

		// if cur_outcount is < CW_SIG_BUFSIZE, loop must terminate after CW_SIG_BUFSIZE -1 steps
		while (temp_outcount != cur_outcount)
		{
			//-----------------------------------------------------
			// Find shortest pulse duration. Only test key-down states
			if (sig[temp_outcount].state)
			{
				switch(cw_decoder_config.spikecancel)
				{
				case 0:
					if (sig[temp_outcount].time < pduration)
					{
						pduration = sig[temp_outcount].time;
						plocation = temp_outcount;
					}
				break;
				case 1:
					if ((sig[temp_outcount].time < pduration) && (sig[temp_outcount].time > CW_SPIKECANCEL_MAX_DURATION))
					{
						pduration = sig[temp_outcount].time;
						plocation = temp_outcount;
					}
				break;
				case 2:
					if ((sig[temp_outcount].time < pduration) && ((3*sig[temp_outcount].time)>cw_times.dot_avg))
					{
						pduration = sig[temp_outcount].time;
						plocation = temp_outcount;
					}
				break;
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

		uint8_t decoded[] =
		{ 0xff, 0xff };

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
			decoded[0] = CharacterIdFunc(); // Convert dot/dash data into a character
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
			decoded[0] = CharacterIdFunc(); // Convert dot/dash pattern into a character
			// Process second character delimited by character or word space

			while (DataRecognitionFunc(&dummy))
			{
				// nothing
			}
			CodeGenFunc();                 // Generate a dot/dash pattern string
			decoded[1] = CharacterIdFunc(); // Convert dot/dash pattern into a character

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

			uint8_t decoded = CharacterIdFunc();
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
				if (!ErrorCorrectionFunc())
				{
					b.initialized = FALSE;
				}
			}
		}
	}
}

void CW_Decoder_WPM_display(bool visible)
{
	static uint8_t old_speed = 0;
	char WPM_str[10];
    const char* label;
    uint16_t color1 = White;
    uint16_t color2 = Green;
    if(!visible)
    {
    	color1 = Black;
    	color2 = Black;
    }

    if(cw_decoder_config.speed != old_speed)
    {
		snprintf(WPM_str, 10, "%3u", cw_decoder_config.speed);
		label = "wpm";
		UiLcdHy28_PrintText(POS_CW_DECODER_WPM_X, POS_CW_DECODER_WPM_Y,WPM_str,color1,Black,0);
		UiLcdHy28_PrintText(POS_CW_DECODER_WPM_X + 27, POS_CW_DECODER_WPM_Y, label, color2, Black, 4);
    }
}

#if 0
static void CW_Decoder_SnapCarrier (void)
{

	/*

1. take the bins from the display FFT and search in the bins that are in the filter passband (which is usually quite narrow in CW)
   for the loudest signal (do averaging over several FFT loops). Note the number of that bin

2. take the signal values of that bin, the bin below and the bin above (do that several times
   and average to account for pauses between the dits and dahs)

3. calculate a three-bin quadratic interpolation to exactly determine the frequency of that signal (with the formula shown above)

4. calculate the deviation of the received CW signal from Rx frequency +- CW sidetone

5. show this deviation graphically inside the spectrum display or waterfall display
6. show frequency in small freq display
7. automatically tune-in that frequency on demand

	 */


    const float32_t buff_len = FFT_IQ_BUFF_LEN2;
    // the calculation of bin_BW is perfectly right at the moment, but we have to change it, if we switch to using the spectrum display zoom FFT to finetune
    //    const float32_t bin_BW = (float32_t) (48000.0 * 2.0 / (buff_len * (1 << sd.magnify))); // width of a 1024 tap FFT bin = 46.875Hz, if FFT_IQ_BUFF_LEN2 = 2048 --> 1024 tap FFT
    const float32_t bin_BW = (float32_t) (IQ_SAMPLE_RATE_F * 2.0 / (buff_len));
    const int buff_len_int = FFT_IQ_BUFF_LEN2;

    float32_t   FFT_MagData[FFT_IQ_BUFF_LEN2/2];

    float32_t bw_LSB = 0.0;
    float32_t bw_USB = 0.0;

    float32_t help_freq = (float32_t)df.tune_old / ((float32_t)TUNE_MULT);

    //	determine posbin (where we receive at the moment) from ts.iq_freq_mode
    const int posbin = buff_len_int/4  - (buff_len_int * (AudioDriver_GetTranslateFreq()/(IQ_SAMPLE_RATE/8)))/16;
    const float32_t width = FilterInfo[FilterPathInfo[ts.filter_path].id].width;
    const float32_t centre_f = FilterPathInfo[ts.filter_path].offset;
    const float32_t offset = centre_f - (width/2.0);

    //	determine Lbin and Ubin from ts.dmod_mode and FilterInfo.width
    //	= determine bandwith separately for lower and upper sideband
    switch(ts.dmod_mode)
    {
    case DEMOD_LSB:
    {
        bw_USB = 1000.0; // also "look" 1kHz away from carrier
        bw_LSB = width;
    }
    break;
    case DEMOD_USB:
    {
        bw_LSB = 1000.0; // also "look" 1kHz away from carrier
        bw_USB = width;
    }
    break;
    case DEMOD_CW:   // experimental feature for CW - morse code signals
    {
        if(ts.cw_offset_mode == CW_OFFSET_USB_SHIFT)  	// Yes - USB?
        {
            // set flag for USB-freq-correction
            // set limits for Lbin and Ubin according to filter_settings: offset = centre frequency!!!
            // Lbin = posbin + offset from 0Hz
            // offset = centre_f - (width/2)
            // Lbin = posbin + round (off/bin_BW)
            // Ubin = posbin + round((off + width)/bin_BW)
            bw_LSB = - 1.0 * offset;
            bw_USB = offset + width;
            //	        	Lbin = (float32_t)posbin + round (offset / bin_BW);
            //	        	Ubin = (float32_t)posbin + round ((offset + width)/bin_BW);
        }
        else if(ts.cw_offset_mode == CW_OFFSET_LSB_SHIFT) 	// LSB?
        {
            bw_USB = - 1.0 * offset;
            bw_LSB = offset + width;
            //	        	Ubin = (float32_t)posbin - round (offset / bin_BW);
            //		        Lbin = (float32_t)posbin - round ((offset + width)/bin_BW);
        }
        else if(ts.cw_offset_mode == CW_OFFSET_AUTO_SHIFT)	 	// Auto mode?  Check flag
        {
            if(ts.cw_lsb)
            {
                bw_USB = - 1.0 * offset;
                bw_LSB = offset + width;
                //			        Ubin = (float32_t)posbin - round (offset / bin_BW);
                //			        Lbin = (float32_t)posbin - round ((offset + width)/bin_BW);
            }
            else
            {
                bw_LSB = - 1.0 * offset;
                bw_USB = offset + width;
                //		        	Lbin = (float32_t)posbin + round (offset / bin_BW);
                //		        	Ubin = (float32_t)posbin + round ((offset + width)/bin_BW);
            }
        }
    }
    break;
    case DEMOD_SAM:
    case DEMOD_AM:
    {
        bw_LSB = width;
        bw_USB = width;
    }
    break;
    }
    // calculate upper and lower limit for determination of maximum magnitude
    const float32_t Lbin = (float32_t)posbin - round(bw_LSB / bin_BW);
    const float32_t Ubin = (float32_t)posbin + round(bw_USB / bin_BW); // the bin on the upper sideband side

    /* NEVER USE THIS, THIS CAUSES BIG PROBLEMS (but I dunno why . . )
     *
     *    if(Lbin < 0)
    {
    	Lbin = 0;
    }
    if (Ubin > 255)
    {
    	Ubin = 255;
    }
     */
    // 	FFT preparation
    // we do not need to scale for this purpose !
    // arm_scale_f32((float32_t *)sc.FFT_Samples, (float32_t)((1/ads.codec_gain_calc) * 1000.0), (float32_t *)sc.FFT_Samples, FFT_IQ_BUFF_LEN2);	// scale input according to A/D gain
    //
    // do windowing function on input data to get less "Bin Leakage" on FFT data
    //
    for(int i = 0; i < buff_len_int; i++)
    {
        //	Hanning 1.36
        //sc.FFT_Windat[i] = 0.5 * (float32_t)((1 - (arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN2-1)))) * sc.FFT_Samples[i]);
        // Hamming 1.22
        //sc.FFT_Windat[i] = (float32_t)((0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN2-1)))) * sc.FFT_Samples[i]);
        // Blackman 1.75
        float32_t help_sample = (0.42659 - (0.49656*arm_cos_f32((2.0*PI*(float32_t)i)/(buff_len-1.0))) + (0.076849*arm_cos_f32((4.0*PI*(float32_t)i)/(buff_len-1.0)))) * sc.FFT_Samples[i];
        sc.FFT_Samples[i] = help_sample;
    }

    // run FFT
    //		arm_rfft_f32((arm_rfft_instance_f32 *)&sc.S,(float32_t *)(sc.FFT_Windat),(float32_t *)(sc.FFT_Samples));	// Do FFT
    //		arm_rfft_fast_f32((arm_rfft_fast_instance_f32 *)&sc.S,(float32_t *)(sc.FFT_Windat),(float32_t *)(sc.FFT_Samples),0);	// Do FFT
    // arm_rfft_fast_f32(&sc.S,sc.FFT_Samples,sc.FFT_Samples,0);	// Do FFT
    //
    // Calculate magnitude
    // as I understand this, this takes two samples and calculates ONE magnitude from this --> length is FFT_IQ_BUFF_LEN2 / 2
    // arm_cmplx_mag_f32(sc.FFT_Samples, FFT_MagData,(buff_len_int/2));
    //
    // putting the bins in frequency-sequential order!
    // it puts the Magnitude samples into FFT_Samples again
    // the samples are centred at FFT_IQ_BUFF_LEN2 / 2, so go from FFT_IQ_BUFF_LEN2 / 2 to the right and fill the buffer sc.FFT_Samples,
    // when you have come to the end (FFT_IQ_BUFF_LEN2), continue from FFT_IQ_BUFF_LEN2 / 2 to the left until you have reached sample 0
    //
    for(int i = 0; i < (buff_len_int/2); i++)
    {
        if(i < (buff_len_int/4))	 		// build left half of magnitude data
        {
            sc.FFT_Samples[i] = FFT_MagData[i + buff_len_int/4];	// get data
        }
        else	 							// build right half of magnitude data
        {
            sc.FFT_Samples[i] = FFT_MagData[i - buff_len_int/4];	// get data
        }
    }
    //####################################################################
    if (sc.FFT_number == 0)
    {
        // look for maximum value and save the bin # for frequency delta calculation
        float32_t maximum = 0.0;
        float32_t maxbin = 1.0;
        float32_t delta = 0.0;

        for (int c = (int)Lbin; c <= (int)Ubin; c++)   // search for FFT bin with highest value = carrier and save the no. of the bin in maxbin
        {
            if (maximum < sc.FFT_Samples[c])
            {
                maximum = sc.FFT_Samples[c];
                maxbin = c;
            }
        }

        // ok, we have found the maximum, now save first delta frequency
        delta = (maxbin - (float32_t)posbin) * bin_BW;

        help_freq = help_freq + delta;

        //        if(ts.dmod_mode == DEMOD_CW) help_freq = help_freq + centre_f; // tuning in CW mode for passband centre!

        help_freq = help_freq * ((float32_t)TUNE_MULT);
        // set frequency of Si570 with 4 * dialfrequency
        df.tune_new = help_freq;
        // request a retune just by changing the frequency

        //        help_freq = (float32_t)df.tune_new / 4.0;
        sc.FFT_number = 1;
        sc.state    = 0;
        arm_fill_f32(0.0,sc.FFT_Samples,buff_len_int);
    }
    else
    {
        // ######################################################

        // and now: fine-tuning:
        //	get amplitude values of the three bins around the carrier

        float32_t bin1 = sc.FFT_Samples[posbin-1];
        float32_t bin2 = sc.FFT_Samples[posbin];
        float32_t bin3 = sc.FFT_Samples[posbin+1];

        if (bin1+bin2+bin3 == 0.0) bin1= 0.00000001; // prevent divide by 0

        // estimate frequency of carrier by three-point-interpolation of bins around maxbin
        // formula by (Jacobsen & Kootsookos 2007) equation (4) P=1.36 for Hanning window FFT function

        float32_t delta = (bin_BW * (1.75 * (bin3 - bin1)) / (bin1 + bin2 + bin3));
        if(delta > bin_BW) delta = 0.0;

        // set frequency variable with delta2
        help_freq = help_freq + delta;
        //       if(ts.dmod_mode == DEMOD_CW) help_freq = help_freq - centre_f; // tuning in CW mode for passband centre!

        help_freq = help_freq * ((float32_t)TUNE_MULT);
        // set frequency of Si570 with 4 * dialfrequency
        df.tune_new = help_freq;
        // request a retune just by changing the frequency

        sc.state = 0; // reset flag for FFT sample collection (used in audio_rx_driver)
        sc.snap = 0; // reset flag for button press (used in ui_driver)
        sc.FFT_number = 0; // reset flag to first FFT
    }
}
#endif
