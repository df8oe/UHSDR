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
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H

#include "uhsdr_board_config.h"
#include "uhsdr_types.h"
#include "arm_math.h"
#include "softdds.h"
#include "audio_filter.h"

#define IQ_SAMPLE_RATE_F ((float32_t)IQ_SAMPLE_RATE)

#define AUDIO_SAMPLE_RATE_F ((float32_t)AUDIO_SAMPLE_RATE)



#if  defined(USE_32_IQ_BITS)
    typedef int32_t iq_data_t;
#else
    typedef int16_t iq_data_t;
#endif

#if defined(USE_32_AUDIO_BITS)
    typedef int32_t audio_data_t;
#else
    typedef int16_t audio_data_t;
#endif


typedef struct {
    __packed audio_data_t l;
    __packed audio_data_t r;
} AudioSample_t;

typedef struct {
    __packed iq_data_t l;
    __packed iq_data_t r;
} IqSample_t;

#ifdef USE_CONVOLUTION
typedef struct {
   COMP samples[SAMPLE_BUFFER_SIZE];
}  Sample_Buffer;
#endif

// -----------------------------
// FFT buffer, this is double the size of the length of the FFT used for spectrum display and waterfall spectrum
#ifdef USE_FFT_1024
	#define FFT_IQ_BUFF_LEN		1024
#else
	#define FFT_IQ_BUFF_LEN		512
#endif
#define SPEC_BUFF_LEN (FFT_IQ_BUFF_LEN/2)

// twice the number of samples in the each iq block buffer
// (which is half of the total dma buffer, since in each interrupt we get half of the total dma buffer)
#define	IQ_BUFSZ 	(2*IQ_BLOCK_SIZE)
#define AUDIO_BUFSZ    (2*AUDIO_BLOCK_SIZE)

// Audio filter
#define FIR_RXAUDIO_BLOCK_SIZE		IQ_BLOCK_SIZE
#define FIR_RXAUDIO_NUM_TAPS		16 // maximum number of taps in the decimation and interpolation FIR filters
#define IIR_RXAUDIO_BLOCK_SIZE		IQ_BLOCK_SIZE
#define IIR_RXAUDIO_NUM_STAGES_MAX	12 // we use a maximum stage number of 10 at the moment, so this is 12 just to be safe
//
#define CODEC_DEFAULT_GAIN		0x1F	// Gain of line input to start with
#define	ADC_CLIP_WARN_THRESHOLD	4096	// This is at least 12dB below the clipping threshold of the A/D converter itself
//
//
//
#define SCALING_FACTOR_IQ_PHASE_ADJUST 2000.0
#define SCALING_FACTOR_IQ_AMPLITUDE_ADJUST 2731.0

#define SAM_PLL_HILBERT_STAGES 7


#ifdef USE_TWO_CHANNEL_AUDIO
    #define NUM_AUDIO_CHANNELS 2
#else
    #define NUM_AUDIO_CHANNELS 1
#endif

typedef struct
{

    // for SAM demodulation
    // DX adjustments: zeta = 0.15, omegaN = 100.0
    // very stable, but does not lock very fast
    // standard settings: zeta = 1.0, omegaN = 250.0
    // maybe user can choose between slow (DX), medium, fast SAM PLL
    // zeta / omegaN
    // DX = 0.2, 70
    // medium 0.6, 200
    // fast 1.2, 500


      //pll
    float32_t               omega_min; // (2.0 * 3.141592653589793f * pll_fmin * DF / IQ_SAMPLE_RATE_F);
    float32_t               omega_max; //(2.0 * 3.141592653589793f * pll_fmax * DF / IQ_SAMPLE_RATE_F);
    float32_t               g1; //(1.0 - exp(-2.0 * omegaN * zeta * DF / IQ_SAMPLE_RATE_F));
    float32_t               g2; //(- g1 + 2.0 * (1 - exp(- omegaN * zeta * DF / IQ_SAMPLE_RATE_F)
        //  * cosf(omegaN * DF / IQ_SAMPLE_RATE_F * sqrtf(1.0 - zeta * zeta))));

      //fade leveler
    float32_t               mtauR; //(exp(- DF / (IQ_SAMPLE_RATE_F * tauR))); //0.99948;
    float32_t               onem_mtauR;
    float32_t               mtauI; //(exp(- DF / (IQ_SAMPLE_RATE_F * tauI))); //0.99999255955;
    float32_t               onem_mtauI;
} demod_sam_param_t;

typedef struct
{
    float32_t               teta1;
    float32_t               teta2;
    float32_t               teta3;
    float32_t               teta1_old;
    float32_t               teta2_old;
    float32_t               teta3_old;
    float32_t               M_c1;
    float32_t               M_c2;
} iq_correction_data_t;

typedef  float32_t audio_block_t[AUDIO_BLOCK_SIZE];
typedef  float32_t iq_block_t[IQ_BLOCK_SIZE];

typedef struct
{
    iq_block_t               i_buffer;
    iq_block_t               q_buffer;
} iq_buffer_t;


typedef struct
{
    // Stereo buffers
    iq_buffer_t     iq_buf;
    float32_t       agc_valbuf[IQ_BLOCK_SIZE];   // holder for "running" AGC value

    audio_block_t   a_buffer[2];

    demod_sam_param_t sam;
    iq_correction_data_t iq_corr;
} AudioDriverBuffer;


typedef struct
{
    float       sql_avg;         // averaged squelch level (for FM)
    bool        squelched;       // TRUE if FM receiver audio is to be squelched

    float       subaudible_tone_gen_freq;    // frequency, in Hz, of currently-selected subaudible tone for generation
    soft_dds_t  subaudible_tone_dds;

    soft_dds_t  tone_burst_dds;
    bool        tone_burst_active;       // this is TRUE if the tone burst is actively being generated
    //
    float       subaudible_tone_det_freq;    // frequency, in Hz, of currently-selected subaudible tone for detection
    bool        subaudible_tone_detected;    // TRUE if subaudible tone has been detected

    Goertzel goertzel[3];
    #define FM_HIGH 0
    #define FM_LOW  1
    #define FM_CTR  2

} fm_conf_t;

typedef enum
{
  SAM_SIDEBAND_BOTH = 0,
  SAM_SIDEBAND_LSB,
  SAM_SIDEBAND_USB,
#ifdef USE_TWO_CHANNEL_AUDIO
  SAM_SIDEBAND_STEREO,
#endif
  SAM_SIDEBAND_MAX
} sam_sideband_t;


typedef struct
{
#define DSP_NR_ENABLE           0x01    // DSP NR mode is on (| 1)
#define DSP_NR_POSTAGC_ENABLE   0x02    // DSP NR is to occur post AGC (| 2)
#define DSP_NOTCH_ENABLE        0x04    // DSP Notch mode is on (| 4)
#define DSP_NB_ENABLE           0x08    // DSP is to be displayed on screen instead of NB (| 8)
#define DSP_MNOTCH_ENABLE       0x10    // Manual Notch enabled
#define DSP_MPEAK_ENABLE        0x20    // Manual Peak enabled

    uint8_t active;                 // Used to hold various aspects of DSP mode selection
    uint8_t mode;                   // holds the mode chosen in the DSP
    uint16_t mode_mask;             // holds the DSP mode mask (to be chosen by virtual dsp keyboard)
    uint8_t active_toggle;          // holder used on the press-hold of button G2 to "remember" the previous setting
    uint8_t nr_strength;            // "Strength" of DSP Noise reduction - to be converted to "Mu" factor
#if defined (USE_LMS_AUTONOTCH)
    uint8_t notch_numtaps;
    uint8_t notch_mu;
    // mu adjust of notch DSP LMS
    uint8_t notch_delaybuf_len;     // size of DSP notch delay buffer
#endif
    uint8_t inhibit;                // if != 0, DSP (NR, Notch) functions are inhibited.  Used during power-up and switching
    uint8_t nb_setting;
    ulong   notch_frequency;        // frequency of the manual notch filter
    ulong   peak_frequency;         // frequency of the manual peak filter

    int     bass_gain;              // gain of the low shelf EQ filter
    int     treble_gain;            // gain of the high shelf EQ filter
    int     tx_bass_gain;           // gain of the TX low shelf EQ filter
    int     tx_treble_gain;         // gain of the TX high shelf EQ filter

} dsp_params_t;

// Audio driver publics
typedef struct AudioDriverState
{
    //
    // Lock audio filter flag
    //
    volatile bool					af_disabled;			// if TRUE, audio filtering is disabled (used during filter bandwidth changing, etc.)
    volatile bool					tx_filter_adjusting;	// used to disable TX I/Q filter during phase adjustment

    float					codec_gain_calc;    // spectrum gain value

    bool					adc_clip;           // used to display warning in s meter
    bool					adc_half_clip;      // used to control input gain and spectrum gain
    bool					adc_quarter_clip;   // used to control input gain and spectrum gain
    float					peak_audio;			// used for audio metering to detect the peak audio level

    float					alc_val;			// "live" transmitter ALC value
    float					alc_decay;			// decay rate (speed) of ALC

    uchar					decimation_rate;		// current decimation/interpolation rate
    uint32_t                decimated_freq;        // resulting decimated sample frequency (used in iq and audio processing)

    fm_conf_t               fm_conf;       // configuration parameters for the fm demodulator

    soft_dds_t              beep;				// this is the actively-used DDS tone word for the radio's beep generator
    float					beep_loudness_factor;	// this is used to set the beep loudness

    /* SAM */
    // sam related output variables
    int                     carrier_freq_offset;

    // sam related configuration parameters, stored in config memory
    int                     pll_fmax_int;
    int                     zeta_int; // zeta * 100
    int                     omegaN_int;
    uint8_t                 fade_leveler; // boolean
    // sam related operation parameters, not stored in config memory
    sam_sideband_t          sam_sideband; // 0 = both, 1 = LSB, 2 = USB

    /* IQ Balance */
    float32_t               iq_phase_balance_rx;
    float32_t               iq_phase_balance_tx[IQ_TRANS_NUM];

    ulong snap_carrier_freq; // used for passing the estimated carrier freq in SNAP mode to the print routine in UI_Driver
    bool CW_signal; // if CW decoder is enabled and carrier snap is wanted, this indicates whenever a pulse is received
    // only in that case, the carrier frequency is estimated and the display refreshed
} AudioDriverState;

void AudioManagement_CalcIQPhaseAdjust(uint32_t freq);

// S meter public
typedef struct SMeter
{
    // configurable ALPHA = 1 - e^(-T/Tau)
    // we use alpha config value scaling of 100, i.e. 100 => alpha = 1.00
    // this construct permits us to use a single configuration store for both
    // it looks rather complex but this is necessary to ensure type safety checks are working
    union {
        uint16_t alphaCombined;
        struct
        {
            uint8_t DecayAlpha;
            uint8_t AttackAlpha;
        } alphaSplit;
    } config;

// first/upper 8 bits is AttackAlpha
// second/lower 8 bits is DecayAlpha
#define SMETER_ALPHA_ATTACK_DEFAULT 50
#define SMETER_ALPHA_DECAY_DEFAULT   5
#define SMETER_ALPHA_MIN             1 // used for both alphas
#define SMETER_ALPHA_MAX           100 // used for both alphas

    // averaged values, used for display
    float32_t dbm;
    float32_t dbmhz;

    // current measurements, used for averaging
    float32_t dbm_cur;
    float32_t dbmhz_cur;

    // internal variables for dbm low pass calculation
    float32_t AttackAvedbm;
    float32_t DecayAvedbm;
    float32_t AttackAvedbmhz;
    float32_t DecayAvedbmhz;

    uint32_t s_count; // number of S steps, used for display and CAT level
} SMeter;


#define MAX_BASS 		 	20
#define MIN_BASS 			-20
#define MAX_TREBLE 			20
#define MIN_TREBLE			-20
#define MAX_TX_BASS		 	5
#define MIN_TX_BASS			-20
#define MAX_TX_TREBLE 		5
#define MIN_TX_TREBLE		-20

#define MIN_PEAK_NOTCH_FREQ 200
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
#define	AM_SCALING		1.0		// was 2.0 // Amount of gain multiplication to apply to audio and AGC to make recovery equal to that of SSB
#define	AM_AUDIO_SCALING	1.4	// was 1.4 // Additional correction factor applied to audio demodulation to make amplitude equal to that of SSB demodulation
//
//#define	AGC_GAIN_CAL	155000.0//22440		// multiplier value (linear, not DB) to calibrate the S-Meter reading to the AGC value
//
#define	AUTO_RFG_DECREASE_LOCKOUT	1
#define	AUTO_RFG_INCREASE_TIMER		5//10
//
//#define	AGC_SLOW			0		// Mode setting for slow AGC
//#define	AGC_MED				1		// Mode setting for medium AGC
//#define	AGC_FAST			2		// Mode setting for fast AGC
//#define	AGC_CUSTOM			3		// Mode setting for custom AGC
//#define	AGC_OFF				4		// Mode setting for AGC off
//#define	AGC_MAX_MODE		4		// Maximum for mode setting for AGC
//#define	AGC_DEFAULT			AGC_MED	// Default!
//
//#define	AGC_CUSTOM_MAX		30		// Maximum (slowest) setting for custom AGC
//#define	AGC_CUSTOM_DEFAULT	12		// Default custom AGC setting (approx. equal to "medium")
//#define AGC_CUSTOM_FAST_WARNING	2	// Value at or below which setting the custom AGC is likely to degrade audio
//
#define	MAX_RF_GAIN_MAX		30		// Maximum setting for "Max RF gain"
#define	MAX_RF_GAIN_DEFAULT	10

// Noise blanker constants
#define	MAX_NB_SETTING		15
#define	NB_WARNING1_SETTING	7		// setting at or above which NB warning1 (yellow) is given
#define	NB_WARNING2_SETTING	12		// setting at or above which NB warning2 (orange) is given
#define	NB_WARNING3_SETTING	15		// setting at or above which NB warning3 (red) is given

// Values used for "custom" AGC settings
#define	LINE_OUT_SCALING_FACTOR	10 // multiplication of audio for fixed LINE out level (nominally 1vpp)
//
#define	LINE_IN_GAIN_RESCALE	20		// multiplier for line input gain
#define	MIC_GAIN_RESCALE	2	// divisor for microphone gain setting


// ALC (Auto Level Control) for transmitter, constants
#define	ALC_VAL_MAX			1		// Maximum ALC Value is 1 (e.g. it can NEVER amplify)
#define	ALC_VAL_MIN			0.001	// Minimum ALC Value - it can provide up to 60dB of attenuation
#define	ALC_ATTACK			0.1//0.033	// Attack time for the ALC's gain control
#define	ALC_KNEE			30000	// The audio value threshold for the ALC operation

// Decay (release time) for ALC/Audio compressor
#define	ALC_DECAY_MAX		20		// Maximum (slowest) setting for ALC decay
#define	ALC_DECAY_DEFAULT	10		// Default custom ALC setting (approx. equal to AGC "medium")

// Audio post-filter (pre-alc) gain adjust.  This effectively sets the min/max compression level.
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
#define FREEDV_GAIN_COMP   (20*SSB_GAIN_COMP)

// The following are calibration constants for AM (transmitter) modulation, carefully adjusted for proper D/A scaling to set
// maximum possible 95-100% AM modulation depth.
//
#define	AM_ALC_GAIN_CORRECTION	0.23			// This scales the output of the ALC for 100% modulation with respect to the carrier level
#define	AM_CARRIER_LEVEL		5100			// This sets the AM carrier level in DAC units, scaled for proper ALC operation and 100% modulation
//
// DO NOT change the above unless you know *EXACTLY* what you are doing!  If you screw with these numbers, you WILL wreck the
// AM modulation!!!  (No, I'm not kidding!)

// FM TX/RX
#define NUM_SUBAUDIBLE_TONES 56
#define FM_SUBAUDIBLE_TONE_OFF  0

// FM TX

#define FM_TONE_BURST_MAX   2
extern uint32_t fm_tone_burst_freq[FM_TONE_BURST_MAX+1];

#define FM_TONE_BURST_OFF   0

#define FM_TONE_BURST_DURATION  100         // duration, in 100ths of a second, of the tone burst

// FM RX
#define FM_SQUELCH_MAX      20              // maximum setting for FM squelch
#define FM_SQUELCH_DEFAULT  12              // default setting for FM squelch
#define FM_SUBAUDIBLE_GOERTZEL_WINDOW   400             // this sets the overall number of samples involved in the Goertzel decode windows (this value * "size/2")


#define	MIN_BEEP_FREQUENCY	200			// minimum beep frequency in Hz
#define	MAX_BEEP_FREQUENCY	3000		// maximum beep frequency in Hz
#define	DEFAULT_BEEP_FREQUENCY	1000	// default beep frequency in Hz

#define	BEEP_DURATION		2			// duration of beep in 100ths of a second

#define MAX_BEEP_LOUDNESS		21		// maximum setting for beep loudness
#define DEFAULT_BEEP_LOUDNESS	10		// default loudness for the keyboard beep

// Factors used in audio compressor adjustment limits
//
#define	TX_AUDIO_COMPRESSION_MIN		-1	// -1 = OFF
#define TX_AUDIO_COMPRESSION_MAX        13  // 0 = least compression, 12 = most, 13 = EEPROM values ("CUS" = CUSTOM) - custom selected by user
#define	TX_AUDIO_COMPRESSION_SV			13
#define	TX_AUDIO_COMPRESSION_DEFAULT	2
//
//
#define RX_DECIMATION_RATE_8KHZ			6		// Decimation/Interpolation rate in receive function for 8 kHz sample rate
#define	RX_DECIMATION_RATE_12KHZ		4		// Decimation/Interpolation rate in receive function for 12 kHz sample rate
#define	RX_DECIMATION_RATE_24KHZ		2		// Decimation/Interpolation rate in receive function for 24 kHz sample rate
#define RX_DECIMATION_RATE_48KHZ		1		// Deimcation/Interpolation rate in receive function for 48 kHz sample rate (e.g. no decimation!)

#define DSP_NR_STRENGTH_MIN		1
#define	DSP_NR_STRENGTH_MAX		200	// Maximum menu setting for DSP "Strength"
#define DSP_NR_STRENGTH_STEP	5
#define	DSP_NR_STRENGTH_DEFAULT	160	// Default setting

#ifdef USE_LMS_AUTONOTCH
//
// Automatic Notch Filter
//
#define	LMS_NOTCH_DELAYBUF_SIZE_MAX	512
//
#define	DSP_NOTCH_NUMTAPS_MAX		64//128
#define	DSP_NOTCH_NUMTAPS_MIN		64
#define	DSP_NOTCH_NUMTAPS_DEFAULT	64//96
//
#define	DSP_NOTCH_BUFLEN_MIN		128//64//48		// minimum length of decorrelation buffer for the notch filter FIR
#define	DSP_NOTCH_BUFLEN_MAX		128//192	// maximum decorrelation buffer length for the notch filter FIR
#define	DSP_NOTCH_DELAYBUF_DEFAULT	128//104	// default decorrelation buffer length for the notch filter FIR
//
#define	DSP_NOTCH_MU_MAX			40//40		// maximum "strength" (convergence) setting for the notch
#define	DSP_NOTCH_MU_DEFAULT		10//25		// default convergence setting for the notch
#endif

typedef enum
{
    DSP_SWITCH_OFF			=	0,
    DSP_SWITCH_NR,
    DSP_SWITCH_NOTCH,
    DSP_SWITCH_NR_AND_NOTCH,
    DSP_SWITCH_NOTCH_MANUAL,
    DSP_SWITCH_PEAK_FILTER,
    DSP_SWITCH_MAX,				// bass & treble not used here
    DSP_SWITCH_BASS          =   98,
    DSP_SWITCH_TREBLE        =   99,
} dsp_mode_t;

#define DSP_SWITCH_MODEMASK_ENABLE_MASK             ((1<<DSP_SWITCH_MAX)-1)
#define DSP_SWITCH_MODEMASK_ENABLE_DEFAULT              ((1<<DSP_SWITCH_MAX)-1)
#define DSP_SWITCH_MODEMASK_ENABLE_DSPOFF           (1<<DSP_SWITCH_OFF)

//
#define	AUDIO_DELAY_BUFSIZE		(IQ_BUFSZ)*5	// Size of AGC delaying audio buffer - Must be a multiple of IQ_BUFSZ.
// This is divided by the decimation rate so that the time delay is constant.


#define	FREQ_IQ_CONV_MODE_OFF		0	// No frequency conversion
#define FREQ_IQ_CONV_P6KHZ		1	// LO is 6KHz above receive frequency in RX mode
#define	FREQ_IQ_CONV_M6KHZ		2	// LO is 6KHz below receive frequency in RX mode
#define FREQ_IQ_CONV_P12KHZ		3	// LO is 12KHz above receive frequency in RX mode
#define	FREQ_IQ_CONV_M12KHZ		4	// LO is 12KHz below receive frequency in RX mode
//
#define	FREQ_IQ_CONV_MODE_DEFAULT	FREQ_IQ_CONV_M12KHZ		//FREQ_IQ_CONV_MODE_OFF
#define	FREQ_IQ_CONV_MODE_MAX		4

// Public Audio
extern AudioDriverState	ads;
extern SMeter       sm;

extern AudioDriverBuffer adb;

typedef struct SnapCarrier
{
    bool	snap;
} SnapCarrier;

extern SnapCarrier sc;




#ifdef USE_LEAKY_LMS
#define LEAKYLMSDLINE_SIZE 256 //512 // was 256 //2048   // dline_size
// 1024 funktioniert nicht
typedef struct
{// Automatic noise reduction
	// Variable-leak LMS algorithm
	// taken from (c) Warren Pratts wdsp library 2016
	// GPLv3 licensed
//	#define DLINE_SIZE 256 //512 //2048  // dline_size
	int16_t n_taps; // =     64; //64;                       // taps
	int16_t delay; // =    16; //16;                       // delay
	int dline_size; // = LEAKYLMSDLINE_SIZE;
	//int ANR_buff_size = FFT_length / 2.0;
	int position;// = 0;
	float32_t two_mu;// =   0.0001;   typical: 0.001 to 0.000001  = 1000 to 1 -> div by 1000000     // two_mu --> "gain"
	uint32_t two_mu_int;
	float32_t gamma;// =    0.1;      typical: 1.000 to 0.001  = 1000 to 1 -> div by 1000           // gamma --> "leakage"
	uint32_t gamma_int;
	float32_t lidx;// =     120.0;                      // lidx
	float32_t lidx_min;// = 0.0;                      // lidx_min
	float32_t lidx_max;// = 200.0;                      // lidx_max
	float32_t ngamma;// =   0.001;                      // ngamma
	float32_t den_mult;// = 6.25e-10;                   // den_mult
	float32_t lincr;// =    1.0;                      // lincr
	float32_t ldecr;// =    3.0;                     // ldecr
	//int ANR_mask = ANR_dline_size - 1;
	int mask;// = DLINE_SIZE - 1;
	int in_idx;// = 0;
	float32_t d [LEAKYLMSDLINE_SIZE];
	float32_t w [LEAKYLMSDLINE_SIZE];
	uint8_t on;// = 0;
	uint8_t notch;// = 0;
} lLMS;

extern lLMS leakyLMS;
#endif

// TODO: Discuss to drop 16 bit I2S support. Would simplify the incoming/outgoing data handling. We could
// align all scalings accordingly and would not have most of the ugly stuff below
// FIXME: This is ugly: The STM32F4 returns 32bit reads from 16 bit peripherals such as the SPI/I2S
// with the two half words in "mixed endian" instead of the wanted "little endian". This is documented in
// the data sheet, so the only thing we can do is to swap the halfwords. This is in fact a single ror16 operation
// if the compiler is smart enough to detect what we want.


// these constants are used to adjust the 32 bit integer samples to represent the same levels as if we sample 16 bit integers,
// effectively "shifting" them down or up.
// FIXME: switch to 16 bit extended mode for 16 bit samples  will eliminate the need for this at the expense of
// using the same DMA memory (two times the memory true 16 bit values take, in our case this is 2*(2*(IQ_BLOCK_SIZE*2samples*2bytes) = 512 bytes)
#ifdef USE_32_IQ_BITS
    #define IQ_BIT_SHIFT 16
    #define IQ_BIT_SCALE_DOWN (0.0000152587890625)
#else
    #define IQ_BIT_SHIFT 0
    #define IQ_BIT_SCALE_DOWN (1.0)
#endif
#define IQ_BIT_SCALE_UP (1<<IQ_BIT_SHIFT)

#ifdef USE_32_AUDIO_BITS
    #define AUDIO_BIT_SHIFT 16
    #define AUDIO_BIT_SCALE_DOWN (0.0000152587890625)
#else
    #define AUDIO_BIT_SHIFT 0
    #define AUDIO_BIT_SCALE_DOWN (1.0)
#endif
#define AUDIO_BIT_SCALE_UP (1<<AUDIO_BIT_SHIFT)

// we have to swap them only if we are having 32bit values from/to I2S and an STM32F4
#if defined(STM32F4) && defined(USE_32_IQ_BITS)
    static inline int32_t I2S_correctHalfWord(const int32_t word)
    {
        uint32_t uWord = (uint32_t)word;
        return uWord >> 16 | uWord << 16;
    }

    static inline int16_t I2S_IqSample_2_Int16(const iq_data_t sample) { return (I2S_correctHalfWord(sample))  >> IQ_BIT_SHIFT; }
    static inline int16_t I2S_AudioSample_2_Int16(const iq_data_t sample) { return (I2S_correctHalfWord(sample))  >> IQ_BIT_SHIFT; }
    static inline iq_data_t I2S_Int16_2_IqSample(const int16_t sample) { return (I2S_correctHalfWord(sample  << IQ_BIT_SHIFT)); }
    static inline iq_data_t I2S_Int16_2_AudioSample(const int16_t sample) { return (I2S_correctHalfWord(sample  << IQ_BIT_SHIFT)); }

#else
    #define I2S_correctHalfWord(a) (a)

    #if defined(USE_32_IQ_BITS)
        static inline int16_t I2S_IqSample_2_Int16(const iq_data_t sample) { return sample  >> 16; }
        static inline iq_data_t I2S_Int16_2_IqSample(const int16_t sample) { return sample  << 16; }
    #else
        static inline int16_t I2S_IqSample_2_Int16(const iq_data_t sample) { return sample; }
        static inline iq_data_t I2S_Int16_2_IqSample(const int16_t sample) { return sample; }
    #endif
    #if defined(USE_32_AUDIO_BITS)
        static inline int16_t I2S_AudioSample_2_Int16(const iq_data_t sample) { return sample  >> 16; }
        static inline iq_data_t I2S_Int16_2_AudioSample(const int16_t sample) { return sample  << 16; }
    #else
        static inline int16_t I2S_AudioSample_2_Int16(const iq_data_t sample) { return sample; }
        static inline iq_data_t I2S_Int16_2_AudioSample(const int16_t sample) { return sample; }
     #endif
#endif


// Exports
void AudioDriver_Init(void);
void AudioDriver_SetProcessingChain(uint8_t dmod_mode, bool reset_dsp_nr);
int32_t AudioDriver_GetTranslateFreq(void);
void AudioDriver_SetSamPllParameters (void);

void AudioDriver_I2SCallback(AudioSample_t *audio, IqSample_t *iq, AudioSample_t *audioDst, int16_t size);


void AudioDriver_CalcLowShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS);
void AudioDriver_CalcHighShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS);
void AudioDriver_CalcBandpass(float32_t coeffs[5], float32_t f0, float32_t FS);
void AudioDriver_SetBiquadCoeffs(float32_t* coeffsTo,const float32_t* coeffsFrom);

void AudioDriver_IQPhaseAdjust(uint16_t txrx_mode, float32_t* i_buffer, float32_t* q_buffer, const uint16_t blockSize);
void AudioDriver_AgcWdsp_Set(void);

#endif
