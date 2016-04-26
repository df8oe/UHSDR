/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H

#include "arm_math.h"
// 16 or 24 bits from Codec
//
//#define USE_24_BITS

// -----------------------------
// With 128 words buffer we have
// 120uS processing time in the
// IRQ and 530uS idle time
// (48kHz sampling - USB)
//
// -----------------------------
// FFT buffer, this is double the size of the length of the FFT used for spectrum display and waterfall spectrum
#define FFT_IQ_BUFF_LEN		512


//
// Useful DEFINEs for windowing functions
//
#define	FFT_IQ_BUFF_M1_HALF		(FFT_IQ_BUFF_LEN-1)/2
#define	FFT_IQ_BUFF_P1_HALF		(FFT_IQ_BUFF_LEN+1)/2
//
#define BUFF_LEN 			128
//
//
// -----------------------------
// Half of total buffer
#define	IQ_BUFSZ 	(BUFF_LEN/2)

// Audio filter
#define FIR_RXAUDIO_BLOCK_SIZE		1
#define FIR_RXAUDIO_NUM_TAPS		48
//
#define CODEC_DEFAULT_GAIN		0x1F	// Gain of line input to start with
#define	ADC_CLIP_WARN_THRESHOLD	4096	// This is at least 12dB below the clipping threshold of the A/D converter itself
//
//
//
#define SCALING_FACTOR_IQ_PHASE_ADJUST 3000.0
#define SCALING_FACTOR_IQ_AMPLITUDE_ADJUST 4096.0

//
// Audio driver publics
typedef struct AudioDriverState
{
    // Stereo buffers
    float32_t					i_buffer[IQ_BUFSZ+1];
    float32_t 					q_buffer[IQ_BUFSZ+1];
    float32_t 					a_buffer[IQ_BUFSZ+1];
    float32_t 					a2_buffer[IQ_BUFSZ+1];
    float32_t 					a3_buffer[IQ_BUFSZ+1];
    float32_t 					b_buffer[(IQ_BUFSZ*2)+1];	// this is larger since we need interleaved data for magnitude calculation in AM demod and thus, twice as much space
    float32_t					c_buffer[IQ_BUFSZ+1];
    float32_t					d_buffer[IQ_BUFSZ+1];
    float32_t					e_buffer[IQ_BUFSZ+1];
    float32_t					f_buffer[IQ_BUFSZ+1];
    float32_t					e2_buffer[IQ_BUFSZ+1];
    float32_t					f2_buffer[IQ_BUFSZ+1];
    float32_t					e3_buffer[IQ_BUFSZ+1];
    float32_t					f3_buffer[IQ_BUFSZ+1];
    //
    float32_t					Osc_I_buffer[IQ_BUFSZ+1];
    float32_t					Osc_Q_buffer[IQ_BUFSZ+1];
    //
    // Lock audio filter flag
    //
    bool					af_disabled;			// if TRUE, audio filtering is disabled (used during filter bandwidth changing, etc.)
    bool					tx_filter_adjusting;	// used to disable TX I/Q filter during phase adjustment

    // AGC and audio related variables

    float 					agc_val;			// "live" receiver AGC value
    float					agc_var;
    float					agc_calc;
    float					agc_valbuf[BUFF_LEN];	// holder for "running" AGC value
    float					agc_holder;			// used to hold AGC value during transmit and tuning
    float					agc_decay;			// decay rate (speed) of AGC
    float					agc_rf_gain;		// manual RF gain (actual) - calculated from the value of "ts.rf_gain"
    float					agc_knee;			// "knee" for AGC operation
    float					agc_val_max;		// maximum AGC gain (at minimum signal)
    float					am_fm_agc;			// Signal/AGC level in AM and FM demod mode
    float					fm_sql_avg;			// averaged squelch level (for FM)
    bool					fm_squelched;		// TRUE if FM receiver audio is to be squelched
    //
    uchar					codec_gain;
    float					codec_gain_calc;
    bool					adc_clip;
    bool					adc_half_clip;
    bool					adc_quarter_clip;
    float					peak_audio;			// used for audio metering to detect the peak audio level

    float					alc_val;			// "live" transmitter ALC value
    float					alc_decay;			// decay rate (speed) of ALC
    float					post_agc_gain;		// post AGC gain scaling
    //
    uchar					decimation_rate;		// current decimation/interpolation rate
    ulong					agc_delay_buflen;		// AGC delay buffer length
    float					agc_decimation_scaling;	// used to adjust AGC timing based on sample rate
    //
    float					nb_agc_filt;			// used for the filtering/determination of the noise blanker AGC level
    float					nb_sig_filt;
    ulong					dsp_zero_count;			// used for detecting zero output from DSP which can occur if it crashes
    float					dsp_nr_sample;			// used for detecting a problem with the DSP (e.g. crashing)
    //
    float					fm_subaudible_tone_gen_freq;	// frequency, in Hz, of currently-selected subaudible tone for generation
    ulong					fm_subaudible_tone_word;	// actively-used variable in producing the tone
    //
    ulong					fm_tone_burst_word;			// this is the actively-used DDS tone word in the frequency generator
    bool					fm_tone_burst_active;		// this is TRUE if the tone burst is actively being generated
    //
    float					fm_subaudible_tone_det_freq;	// frequency, in Hz, of currently-selected subaudible tone for detection
    bool					fm_subaudible_tone_detected;	// TRUE if subaudible tone has been detected
    //
    ulong					beep_word;				// this is the actively-used DDS tone word for the radio's beep generator
    float					beep_loudness_factor;	// this is used to set the beep loudness
    //
    // The following are pre-calculated terms for the Goertzel functions used for subaudible tone detection
    //
    float					fm_goertzel_high_a;
    float					fm_goertzel_high_b;
    float					fm_goertzel_high_sin;
    float					fm_goertzel_high_cos;
    float					fm_goertzel_high_r;
    //
    float					fm_goertzel_low_a;
    float					fm_goertzel_low_b;
    float					fm_goertzel_low_sin;
    float					fm_goertzel_low_cos;
    float					fm_goertzel_low_r;
    //
    float					fm_goertzel_ctr_a;
    float					fm_goertzel_ctr_b;
    float					fm_goertzel_ctr_sin;
    float					fm_goertzel_ctr_cos;
    float					fm_goertzel_ctr_r;
    //
    ulong					fm_goertzel_size;
    //
//	float					Osc_Cos;
//	float					Osc_Sin;
//	float					Osc_Vect_Q;
//	float					Osc_Vect_I;
//	float					Osc_Q;
//	float					Osc_I;
//	float					Osc_Gain;

} AudioDriverState;

//
// FFT will work with quadrature signals
#define FFT_QUADRATURE_PROC	1

//
//
// -----------------------------
// S meter sample time
#define S_MET_SAMP_CNT		512

// S meter drag delay
#define S_MET_UPD_SKIP		10		//10000

// S meter public
typedef struct SMeter
{
    ulong	skip;

    ulong	s_count;
    float	gain_calc;
    int		curr_max;

} SMeter;
//
// AGC Time constants
// C. Turner, KA7OEI
//
#define	AGC_KNEE	1000//4000	// ADC "knee" threshold for AGC action
//
#define	AGC_KNEE_REF		1000
#define	AGC_VAL_MAX_REF		131072//4096
#define	POST_AGC_GAIN_SCALING_REF	1.333

#define	AGC_ATTACK		0.033	// Attack time multiplier for AGC
//
#define AGC_FAST_DECAY	0.0002	// Decay rate multiplier for "Fast" AGC
#define AGC_MED_DECAY	0.00006	// Decay rate multiplier for "Medium" AGC
#define AGC_SLOW_DECAY	0.00001 // Decay rate for multiplier  "Slow" AGC
//
#define	AGC_ATTACK_FM	0.0033	// Attack time for FM (S-meter reading only)
#define	AGC_DECAY_FM	0.0333	// Decay time for FM (S-meter reading only)
//

#define	AGC_VAL_MIN		0.02	// Minimum AGC gain multiplier (e.g. gain reduction of 34dB)
//#define AGC_VAL_MAX		4096//1024	// Maximum AGC gain multiplier (e.g. gain multiplication of 60dB)

#define	AGC_PREFILTER_MAXGAIN	5 	// Scaling factor for RF gain adjustment (e.g. factor by which RFG will be multiplied to yield actual RFG multiplier
#define AGC_PREFILTER_MINGAIN	0.5	// Minimum "RFG" gain multiplier (e.g. gain reduction of 6 dB)
//
#define AGC_PREFILTER_HISIG_THRESHOLD	0.1	// Threshold at which adjustment of RFGAIN (pre-filter) gain adjustment will occur
#define AGC_PREFILTER_LOWSIG_THRESHOLD	1.0	// Threshold at which adjustment of RFGAIN (pre-filter) gain adjustment will occur
#define AGC_PREFILTER_ATTACK_RATE		0.0002	// Attack rate for RFG reduction
#define AGC_PREFILTER_DECAY_RATE		0.000002	// Decay rate for RFG gain recovery
//
#define AGC_PREFILTER_MAX_SIGNAL	1		// maximum level of pre-filtered signal
//
#define POST_AGC_GAIN_SCALING	1.333//0.333	// Used to rescale the post-filter audio level to a value suitable for the codec.  This sets the line level output
// to approx. 1000mV peak-peak.
//
#define	POST_AGC_GAIN_SCALING_DECIMATE_4	3.46	// Used to scale audio from the decimation/interpolation-by-4 process (based square root of decimation factor)
//
#define	POST_AGC_GAIN_SCALING_DECIMATE_2	(POST_AGC_GAIN_SCALING_DECIMATE_4 * 0.6)	// Scales audio from decimation/interpolation-by-2 process
//
#define	AM_SCALING		2		// Amount of gain multiplication to apply to audio and AGC to make recovery equal to that of SSB
#define	AM_AUDIO_SCALING	1.4	// Additional correction factor applied to audio demodulation to make amplitude equal to that of SSB demodulation
//
#define	AGC_GAIN_CAL	155000//22440		// multiplier value (linear, not DB) to calibrate the S-Meter reading to the AGC value
//
#define	AUTO_RFG_DECREASE_LOCKOUT	1
#define	AUTO_RFG_INCREASE_TIMER		5//10
//
#define	AGC_SLOW			0		// Mode setting for slow AGC
#define	AGC_MED				1		// Mode setting for medium AGC
#define	AGC_FAST			2		// Mode setting for fast AGC
#define	AGC_CUSTOM			3		// Mode setting for custom AGC
#define	AGC_OFF				4		// Mode setting for AGC off
#define	AGC_MAX_MODE		4		// Maximum for mode setting for AGC
#define	AGC_DEFAULT			AGC_MED	// Default!
//
#define	AGC_CUSTOM_MAX		30		// Maximum (slowest) setting for custom AGC
#define	AGC_CUSTOM_DEFAULT	12		// Default custom AGC setting (approx. equal to "medium")
#define AGC_CUSTOM_FAST_WARNING	2	// Value at or below which setting the custom AGC is likely to degrade audio
//
#define	MAX_RF_GAIN_MAX		6		// Maximum setting for "Max RF gain"
#define	MAX_RF_GAIN_DEFAULT	3
//
// Noise blanker constants
//
#define	NBLANK_AGC_ATTACK	0.33	// Attack time multiplier for AGC
//
#define NBLANK_AGC_DECAY	0.002	// Decay rate multiplier for "Fast" AGC
//
#define	MAX_NB_SETTING		20
#define	NB_WARNING1_SETTING	8		// setting at or above which NB warning1 (yellow) is given
#define	NB_WARNING2_SETTING	12		// setting at or above which NB warning2 (orange) is given
#define	NB_WARNING3_SETTING	16		// setting at or above which NB warning3 (red) is given
#define	NB_DURATION			4
//
#define	NB_AGC_FILT			0.999	// Component of IIR filter for recyling previous AGC value
#define	NB_SIG_FILT			0.001	// Component of IIR filter for present signal value's contribution to AGC
//
#define	NB_AVG_WEIGHT		0.80	// Weighting applied to average based on past signal for NB2
#define	NB_SIG_WEIGHT		0.20	// Weighting applied to present signal for NB2
//
//
#define	NB_MAX_AGC_SETTING	35		// maximum setting for noise blanker setting
#define	NB_AGC_DEFAULT		20		// Default setting for noise blanker AGC time constant adjust
//
// Values used for "custom" AGC settings
//
#define	MIN_CUST_AGC_VAL	10	// Minimum and maximum RF gain settings
#define	MAX_CUST_AGC_VAL	30
#define	CUST_AGC_OFFSET_VAL	30	// RF Gain offset value used in calculations
#define	CUST_AGC_VAL_DEFAULT	17.8	// Value for "medium" AGC value
//
#define	LINE_OUT_SCALING_FACTOR	10		// multiplication of audio for fixed LINE out level (nominally 1vpp)
//
#define	LINE_IN_GAIN_RESCALE	20		// multiplier for line input gain
#define	MIC_GAIN_RESCALE	2	// divisor for microphone gain setting
//
// ALC (Auto Level Control) for transmitter, constants
//
#define	ALC_VAL_MAX			1		// Maximum ALC Value is 1 (e.g. it can NEVER amplify)
#define	ALC_VAL_MIN			0.001	// Minimum ALC Value - it can provide up to 60dB of attenuation
#define	ALC_ATTACK			0.1//0.033	// Attack time for the ALC's gain control
#define	ALC_KNEE			30000	// The audio value threshold for the ALC operation
//
// Decay (release time) for ALC/Audio compressor
//
#define	ALC_DECAY_MAX		20		// Maximum (slowest) setting for ALC decay
#define	ALC_DECAY_DEFAULT	10		// Default custom ALC setting (approx. equal to AGC "medium")
//
// Audio post-filter (pre-alc) gain adjust.  This effectively sets the min/max compression level.
//
#define	ALC_POSTFILT_GAIN_MIN	1
#define	ALC_POSTFILT_GAIN_MAX	25
#define	ALC_POSTFILT_GAIN_DEFAULT	1
//
#define	SSB_ALC_GAIN_CORRECTION	1.00			// This scales the output of the ALC for 100% modulation for SSB transmission
//
#define	SSB_GAIN_COMP		1.133				// This compensates for slight differences in gain processing in the SSB algorithm (empirically derived)
//
#define	AM_GAIN_COMP		1.133				// This compensates for slight differences in gain processing in the AM algorithm (empirically derived)
//
// The following are calibration constants for AM (transmitter) modulation, carefully adjusted for proper D/A scaling to set
// maximum possible 95-100% AM modulation depth.
//
#define	AM_ALC_GAIN_CORRECTION	0.23			// This scales the output of the ALC for 100% modulation with respect to the carrier level
#define	AM_CARRIER_LEVEL		5100			// This sets the AM carrier level in DAC units, scaled for proper ALC operation and 100% modulation
//
// DO NOT change the above unless you know *EXACTLY* what you are doing!  If you screw with these numbers, you WILL wreck the
// AM modulation!!!  (No, I'm not kidding!)
//
// FM Demodulator parameters
//
#define	FM_DEMOD_COEFF1		PI/4			// Factors used in arctan approximation used in FM demodulator
#define	FM_DEMOD_COEFF2		PI*0.75
//
#define	FM_RX_SCALING_2K5		33800			// Amplitude scaling factor of demodulated FM audio (normalized for +/- 2.5 kHz deviation at 1 kHZ)
#define FM_RX_SCALING_5K	(FM_RX_SCALING_2K5/2)	// Amplitude scaling factor of demodulated FM audio (normalized for +/- 5 kHz deviation at 1 kHz)
//
#define FM_AGC_SCALING		2				// Scaling factor for AGC result when in FM (AGC used only for S-meter)
//
#define FM_RX_LPF_ALPHA		0.05			// For FM demodulator:  "Alpha" (low-pass) factor to result in -6dB "knee" at approx. 270 Hz
//
#define FM_RX_HPF_ALPHA		0.96			// For FM demodulator:  "Alpha" (high-pass) factor to result in -6dB "knee" at approx. 180 Hz
//
#define FM_RX_SQL_SMOOTHING	0.005			// Smoothing factor for IIR squelch noise averaging
#define	FM_SQUELCH_HYSTERESIS	3			// Hysteresis for FM squelch
#define FM_SQUELCH_PROC_DECIMATION	50		// Number of times we go through the FM demod algorithm before we do a squelch calculation
#define	FM_SQUELCH_MAX		20				// maximum setting for FM squelch
#define	FM_SQUELCH_DEFAULT	12				// default setting for FM squelch
//
// FM Modulator parameters
//
#define FM_TX_HPF_ALPHA		0.05			// For FM modulator:  "Alpha" (high-pass) factor to pre-emphasis
//
// NOTE:  FM_MOD_SCALING_2K5 is rescaled (doubled) for 5 kHz deviation, as are modulation factors for subaudible tones and tone burst
//
#define	FM_MOD_SCALING_2K5		16				// For FM modulator:  Scaling factor for NCO, after all processing, to achieve 2.5 kHz with a 1 kHz tone
//
#define FM_MOD_SCALING	FM_MOD_SCALING_2K5		// For FM modulator - system deviation
#define	FM_MOD_AMPLITUDE_SCALING	0.875		// For FM modulator:  Scaling factor for output of modulator to set proper output power
#define	FM_FREQ_MOD_WORD			8192		// FM frequency modulator word for modulation DDS/NCO at 6 kHz (6 kHz = 1/8th sample rate, 1/8th of 65536 = 8192)
//
#define	FM_ALC_GAIN_CORRECTION	0.95
//
// For subaudible and burst:  FM Tone word calculation:  freq / (sample rate/2^24) => freq / (48000/16777216) => freq * 349.52533333
//
#define FM_TONE_AMPLITUDE_SCALING	0.00045	// Scaling factor for subaudible tone modulation - not pre-emphasized -to produce approx +/- 300 Hz deviation in 2.5kHz mode
#define FM_TONE_DDS_ACC_SHIFT	14			// number of left-shift bits to obtain lookup word
//
#define	NUM_SUBAUDIBLE_TONES 56
//
#define FM_SUBAUDIBLE_TONE_OFF	0
//
#define	FM_SUBAUDIBLE_TONE_WORD_CALC_FACTOR	(16777216/48000)	// scaling factor for calculating "tone word" for subaudible tone generator
//
#define	FM_TONE_BURST_OFF	0
#define	FM_TONE_BURST_1750_MODE	1
#define	FM_TONE_BURST_2135_MODE	2
#define	FM_TONE_BURST_MAX	2
//
#define	FM_BURST_TONE_WORD_CALC_FACTOR	(16777216/48000)	// scaling factor for calculating "tone word" for the tone burst generator
//
#define FM_TONE_BURST_1750	(1750 * FM_BURST_TONE_WORD_CALC_FACTOR)
#define FM_TONE_BURST_2135	(2135 * FM_BURST_TONE_WORD_CALC_FACTOR)
#define	FM_TONE_BURST_MOD_SCALING 	4266	// scale tone modulation (which is NOT pre-emphasized) for approx. 2/3rds of system modulation
//
#define FM_TONE_BURST_DURATION	100			// duration, in 100ths of a second, of the tone burst
//
// FM RX bandwidth settings
//
/*
enum	{
	FM_RX_BANDWIDTH_7K2 = 0,
	FM_RX_BANDWIDTH_10K,
	FM_RX_BANDWIDTH_12K,
//	FM_RX_BANDWIDTH_15K,		// 15K bandwidth has too much distortion with a "translation" frequency of + or - 6 kHz, likely due to the "Zero Hz Hole"
	FM_RX_BANDWIDTH_MAX
}; */
//
//#define	FM_BANDWIDTH_DEFAULT	FM_RX_BANDWIDTH_10K		// We will use the second-to-narrowest bandwidth as the "Default" FM RX bandwidth to be safe!
//
#define	FM_SUBAUDIBLE_GOERTZEL_WINDOW	400				// this sets the overall number of samples involved in the Goertzel decode windows (this value * "size/2")
#define	FM_TONE_DETECT_ALPHA	0.9						// setting for IIR filtering of ratiometric result from frequency-differential tone detection
//
#define FM_SUBAUDIBLE_TONE_DET_THRESHOLD	1.75		// threshold of "smoothed" output of Goertzel, above which a tone is considered to be "provisionally" detected pending debounce
#define FM_SUBAUDIBLE_DEBOUNCE_MAX			5			// maximum "detect" count in debounce
#define FM_SUBAUDIBLE_TONE_DEBOUNCE_THRESHOLD	2		// number of debounce counts at/above which a tone detection is considered valid
//
#define	FM_GOERTZEL_HIGH	1.04		// ratio of "high" detect frequency with respect to center
#define	FM_GOERTZEL_LOW		0.95		// ratio of "low" detect frequency with respect to center
//
#define	BEEP_SCALING	20				// audio scaling of keyboard beep
#define	BEEP_TONE_WORD_FACTOR			(65536/48000)	// scaling factor for beep frequency calculation
//
#define	MIN_BEEP_FREQUENCY	200			// minimum beep frequency in Hz
#define	MAX_BEEP_FREQUENCY	3000		// maximum beep frequency in Hz
#define	DEFAULT_BEEP_FREQUENCY	1000	// default beep frequency in Hz
#define	BEEP_DURATION		2			// duration of beep in 100ths of a second
#define	SIDETONE_REF_BEEP_DURATION	50	// duration of "Sidetone Reference Beep" (press-and-hold) in 100ths of a second
//
#define MAX_BEEP_LOUDNESS		20		// maximum setting for beep loudness
#define DEFAULT_BEEP_LOUDNESS	10		// default loudness for the keyboard/CW sidetone test beep
//
// Factors used in audio compressor adjustment limits
//
#define	TX_AUDIO_COMPRESSION_MAX		13	// 0 = least compression, 12 = most, 13 = EEPROM values ("SV") - custom selected by user
#define	TX_AUDIO_COMPRESSION_SV			13
#define	TX_AUDIO_COMPRESSION_DEFAULT	2
//
//
#define RX_DECIMATION_RATE_8KHZ			6		// Decimation/Interpolation rate in receive function for 8 kHz sample rate
#define	RX_DECIMATION_RATE_12KHZ		4		// Decimation/Interpolation rate in receive function for 12 kHz sample rate
#define	RX_DECIMATION_RATE_24KHZ		2		// Decimation/Interpolation rate in receive function for 24 kHz sample rate
#define RX_DECIMATION_RATE_48KHZ		1		// Deimcation/Interpolation rate in receive function for 48 kHz sample rate (e.g. no decimation!)
//
// ************
// DSP system parameters
//
// Noise reduction
//
#define	LMS_NR_DELAYBUF_SIZE_MAX		512	// maximum size of LMS delay buffer for the noise reduction
//
//
#define	DSP_NR_STRENGTH_MAX		55//35	// Maximum menu setting for DSP "Strength"
#define	DSP_NR_STRENGTH_DEFAULT	15	// Default setting
//
#define	DSP_STRENGTH_YELLOW		25	// Threshold at and above which DSP number is yellow
#define	DSP_STRENGTH_ORANGE		35	// Threshold at and above which DSP number is orange
#define DSP_STRENGTH_RED		45	// Threshold at and above which DSP number is red
//
//
#define	DSP_NR_BUFLEN_MIN		48		// minimum length of de-correlation buffer on the LMS NR DSP
#define	DSP_NR_BUFLEN_MAX		LMS_NR_DELAYBUF_SIZE_MAX	// maximum length of de-correlation buffer on the LMS NR DSP
#define	DSP_NR_BUFLEN_DEFAULT	192		// default length of de-correlation buffer on the LMS NR DSP
//
#define DSP_NR_NUMTAPS_MIN		32		// minimum number of FIR taps in the LMS NR DSP
#define	DSP_NR_NUMTAPS_MAX		128		// maximum number of FIR taps in the LMS NR DSP
#define	DSP_NR_NUMTAPS_DEFAULT	96		// default number of FIR taps in the LMS NR DSP
//
#define	MAX_DSP_ZERO_COUNT		2048
#define	DSP_ZERO_COUNT_ERROR	512
#define	DSP_ZERO_DET_MULT_FACTOR	10000000	// work-around because the stupid compiler wouldn't compare fractions!
#define	DSP_OUTPUT_MINVAL		1		// minimum out level from DSP LMS NR, indicating "quiet" crash
#define	DSP_HIGH_LEVEL			10000	// output level from DSP LMS NR, indicating "noisy" crash
#define	DSP_CRASH_COUNT_THRESHOLD	35	// "hit" detector/counter for determining if the DSP has crashed
//
// Automatic Notch Filter
//
#define	LMS_NOTCH_DELAYBUF_SIZE_MAX	512
//
#define	DSP_NOTCH_NUMTAPS_MAX	128
#define	DSP_NOTCH_NUMTAPS_MIN		32
#define	DSP_NOTCH_NUMTAPS_DEFAULT	96
//
#define	DSP_NOTCH_BUFLEN_MIN	48		// minimum length of decorrelation buffer for the notch filter FIR
#define	DSP_NOTCH_BUFLEN_MAX	192	// maximum decorrelation buffer length for the notch filter FIR
#define	DSP_NOTCH_DELAYBUF_DEFAULT	104	// default decorrelation buffer length for the notch filter FIR
//
#define	DSP_NOTCH_MU_MAX		40		// maximum "strength" (convergence) setting for the notch
#define	DSP_NOTCH_MU_DEFAULT	25		// default convergence setting for the notch

#define DSP_SWITCH_OFF				0
#define DSP_SWITCH_NR				1
#define DSP_SWITCH_NOTCH			2
#define DSP_SWITCH_NR_AND_NOTCH		3
#define DSP_SWITCH_NOTCH_MANUAL		4
#define DSP_SWITCH_PEAK_FILTER		5
#define DSP_SWITCH_BASS				98
#define DSP_SWITCH_TREBLE			99
#define DSP_SWITCH_MAX				6 // bass & treble not used here
//
#define	AGC_DELAY_BUFSIZE		(BUFF_LEN/2)*5	// Size of AGC delaying audio buffer - Must be a multiple of BUFF_LEN/2.
// This is divided by the decimation rate so that the time delay is constant.
//
#define	ALC_DELAY_BUFSIZE		(BUFF_LEN/2)*5		// Size of AGC delaying audio buffer - Must be a multiple of BUFF_LEN/2.
//
//
#define CLOCKS_PER_DMA_CYCLE	10656			// Number of 16 MHz clock cycles per DMA cycle
#define	CLOCKS_PER_CENTISECOND	160000			// Number of 16 MHz clock cycles per 0.01 second timing cycle
//
//
// The following refer to the software frequency conversion/translation done in receive and transmit to shift the signals away from the
// "DC" IF
//
// The following are terms used to set the NCO frequency of the conversion in the receiver - *IF* we were to use the on-the-fly sine generation
// Which we DON'T, since it is too processor-intensive!  Instead, we use a "fixed" geometrical scheme based on a pre-generated sine wave.
//
// rate = 2 * Pi * (NCO Freq) / (Sample Rate)
// CONV_NCO_SIN = sin(rate)
// CONV_NCO_COS = cos(rate)
//
#define	CONV_NCO_SIN	0.70710678118654752440084436210485	// value for 6 kHz
#define	CONV_NCO_COS	CONV_NCO_SIN				// value for 6 kHz - funny that they are the same, huh!
//
//
//
#define	FREQ_SHIFT_MAG	6000		// Amount of frequency shift, in Hz, when software frequency shift is enabled (exactly 1/8th of the sample rate prior to decimation!)
//
#define	FREQ_IQ_CONV_MODE_OFF		0	// No frequency conversion
#define FREQ_IQ_CONV_P6KHZ		1	// LO is 6KHz above receive frequency in RX mode
#define	FREQ_IQ_CONV_M6KHZ		2	// LO is 6KHz below receive frequency in RX mode
#define FREQ_IQ_CONV_P12KHZ		3	// LO is 12KHz above receive frequency in RX mode
#define	FREQ_IQ_CONV_M12KHZ		4	// LO is 12KHz below receive frequency in RX mode
//
#define	FREQ_IQ_CONV_MODE_DEFAULT	FREQ_IQ_CONV_M12KHZ		//FREQ_IQ_CONV_MODE_OFF
#define	FREQ_IQ_CONV_MODE_MAX		4
//
//
// Exports
void audio_driver_init(void);
void audio_driver_set_rx_audio_filter(void);
void Audio_TXFilter_Init(void);
uint32_t audio_driver_xlate_freq();
//uchar audio_check_nr_dsp_state(void);

#ifdef USE_24_BITS
void I2S_RX_CallBack(int32_t *src, int32_t *dst, int16_t size, uint16_t ht);
#else
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t size, uint16_t ht);
#endif

// Public Audio
extern __IO		AudioDriverState	ads;
extern __IO     SMeter              sm;
extern __IO FilterCoeffs        fc;
// change this to 2048 (=1024 tap FFT), if problems with spectrum display with 7k5 SAM mode persist!
#define FFT_IQ_BUFF_LEN2 2048
//#define FFT_IQ_BUFF_LEN2 4096 // = 2048 tap FFT !!! this is very very accurate
typedef struct SnapCarrier
{
    // FFT state
//    arm_rfft_instance_f32           S; // old, depricated FFT routine, do not use
//    arm_cfft_radix4_instance_f32    S_CFFT;  // old, depricated FFT routine, do not use

    arm_rfft_fast_instance_f32           S; // new and faster real FFT routine

    // Samples buffer
    //
    float32_t   FFT_Samples[FFT_IQ_BUFF_LEN2];
//    float32_t   FFT_Windat[FFT_IQ_BUFF_LEN2];
    float32_t   FFT_MagData[FFT_IQ_BUFF_LEN2/2];
    // Current data ptr
    ulong   samp_ptr;
    int8_t FFT_number;
    int16_t counter;
    // State machine current state
    uchar   state;
    bool	snap;

} SnapCarrier;

__IO SnapCarrier sc;


#endif
