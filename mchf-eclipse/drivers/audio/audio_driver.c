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

// Common
#include <assert.h>
#include "uhsdr_board.h"
#include "ui_driver.h"
#include "profiling.h"

#include <stdio.h>
#include <math.h>
#include "codec.h"

#include "cw_gen.h"

#include <limits.h>
#include "softdds.h"

#include "audio_driver.h"
#include "audio_nr.h"
#include "audio_management.h"
#include "radio_management.h"
#include "usbd_audio_if.h"
#include "ui_spectrum.h"
#include "filters.h"
#include "ui_lcd_hy28.h"
#include "ui_configuration.h"
#include "uhsdr_hw_i2s.h"
#include "rtty.h"
#include "psk.h"
#include "cw_decoder.h"
#include "freedv_uhsdr.h"


typedef struct
{
	// AGC
	//#define MAX_SAMPLE_RATE     (24000.0)
	//#define MAX_N_TAU           (8)
	//#define MAX_TAU_ATTACK      (0.01)
	//#define RB_SIZE       (int) (MAX_SAMPLE_RATE * MAX_N_TAU * MAX_TAU_ATTACK + 1)
#define AGC_WDSP_RB_SIZE 96
	//int8_t AGC_mode = 2;
	int pmode;// = 1; // if 0, calculate magnitude by max(|I|, |Q|), if 1, calculate sqrtf(I*I+Q*Q)
	float32_t out_sample[2];
	float32_t abs_out_sample;
	float32_t tau_attack;
	float32_t tau_decay;
	int n_tau;
	float32_t max_gain;
	float32_t var_gain;
	float32_t fixed_gain; // = 1.0;
	float32_t max_input;
	float32_t out_targ;
	float32_t tau_fast_backaverage;
	float32_t tau_fast_decay;
	float32_t pop_ratio;
	//uint8_t hang_enable;
	float32_t tau_hang_backmult;
	float32_t hangtime;
	float32_t hang_thresh;
	float32_t tau_hang_decay;
	float32_t ring[2*AGC_WDSP_RB_SIZE]; //192]; //96];
	float32_t abs_ring[AGC_WDSP_RB_SIZE];// 192 //96]; // abs_ring is half the size of ring
	//assign constants
	int ring_buffsize; // = 96;
	//do one-time initialization
	int out_index; // = -1;
	float32_t ring_max; // = 0.0;
	float32_t volts; // = 0.0;
	float32_t save_volts; // = 0.0;
	float32_t fast_backaverage; // = 0.0;
	float32_t hang_backaverage; // = 0.0;
	int hang_counter; // = 0;
	uint8_t decay_type; // = 0;
	uint8_t state; // = 0;
	int attack_buffsize;
	uint32_t in_index;
	float32_t attack_mult;
	float32_t decay_mult;
	float32_t fast_decay_mult;
	float32_t fast_backmult;
	float32_t onemfast_backmult;
	float32_t out_target;
	float32_t min_volts;
	float32_t inv_out_target;
	float32_t tmp;
	float32_t slope_constant;
	float32_t inv_max_input;
	float32_t hang_level;
	float32_t hang_backmult;
	float32_t onemhang_backmult;
	float32_t hang_decay_mult;
} agc_variables_t;

agc_variables_t agc_wdsp;


typedef struct {
    __packed int16_t l;
    __packed int16_t r;
} AudioSample_t;

// SSB filters - now handled in ui_driver to allow I/Q phase adjustment


static inline void AudioDriver_TxFilterAudio(bool do_bandpass, bool do_bass_treble, float32_t* inBlock, float32_t* outBlock, const uint16_t blockSize);

#ifdef OBSOLETE_NR
typedef struct
{
    float32_t   errsig1[IQ_BLOCK_SIZE];
    // LMS Filters for RX
    arm_lms_norm_instance_f32	lms1Norm_instance;
    arm_lms_instance_f32	    lms1_instance;
    float32_t	                lms1StateF32[DSP_NR_NUMTAPS_MAX + BUFF_LEN];
    float32_t	                lms1NormCoeff_f32[DSP_NR_NUMTAPS_MAX + BUFF_LEN];
    float32_t                   lms1_nr_delay[LMS_NR_DELAYBUF_SIZE_MAX+BUFF_LEN];
    float32_t   errsig2[64+10];
    arm_lms_norm_instance_f32	lms2Norm_instance;
    arm_lms_instance_f32	    lms2_instance;
    float32_t	                lms2StateF32[LMS2_NOTCH_STATE_ARRAY_SIZE];
    float32_t	                lms2NormCoeff_f32[DSP_NOTCH_NUMTAPS_MAX];
    float32_t	                lms2_nr_delay[DSP_NOTCH_BUFLEN_MAX];

} LMSData;
#endif

#define LMS2_NOTCH_STATE_ARRAY_SIZE (DSP_NOTCH_NUMTAPS_MAX + IQ_BLOCK_SIZE)

#ifdef USE_LMS_AUTONOTCH
typedef struct
{
    float32_t   errsig2[IQ_BLOCK_SIZE];
    arm_lms_norm_instance_f32	lms2Norm_instance;
    arm_lms_instance_f32	    lms2_instance;
    float32_t	                lms2StateF32[LMS2_NOTCH_STATE_ARRAY_SIZE];
    float32_t	                lms2NormCoeff_f32[DSP_NOTCH_NUMTAPS_MAX];
    float32_t	                lms2_nr_delay[DSP_NOTCH_BUFLEN_MAX];
} LMSData;
#endif

static float32_t	__MCHF_SPECIALMEM audio_delay_buffer	[AUDIO_DELAY_BUFSIZE];

static void AudioDriver_ClearAudioDelayBuffer()
{
    arm_fill_f32(0, audio_delay_buffer, AUDIO_DELAY_BUFSIZE);
}

// This is a fast approximation to log2()
// Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
//log10f is exactly log2(x)/log2(10.0f)
// log10f_fast(x) =(log2f_approx(x)*0.3010299956639812f)
float log10f_fast(float X) {
    float Y, F;
    int E;
    F = frexpf(fabsf(X), &E);
    Y = 1.23149591368684f;
    Y *= F;
    Y += -4.11852516267426f;
    Y *= F;
    Y += 6.02197014179219f;
    Y *= F;
    Y += -3.13396450166353f;
    Y += E;
    return(Y * 0.3010299956639812f);
}

//
// Audio RX - Decimator
static  arm_fir_decimate_instance_f32   DECIMATE_RX_I;
float32_t           __MCHF_SPECIALMEM decimState_I[FIR_RXAUDIO_BLOCK_SIZE + 43];
// Audio RX - Decimator in Q-path
static  arm_fir_decimate_instance_f32   DECIMATE_RX_Q;
float32_t           __MCHF_SPECIALMEM decimState_Q[FIR_RXAUDIO_BLOCK_SIZE + 43];

// Decimator for Zoom FFT
static	arm_fir_decimate_instance_f32	DECIMATE_ZOOM_FFT_I;
float32_t			__MCHF_SPECIALMEM decimZoomFFTIState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

// Decimator for Zoom FFT
static	arm_fir_decimate_instance_f32	DECIMATE_ZOOM_FFT_Q;
float32_t			__MCHF_SPECIALMEM decimZoomFFTQState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

// Audio RX - Interpolator
static	arm_fir_interpolate_instance_f32 INTERPOLATE_RX[NUM_AUDIO_CHANNELS];
float32_t			__MCHF_SPECIALMEM interpState[NUM_AUDIO_CHANNELS][FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];



#define NR_INTERPOLATE_NO_TAPS 40
static  arm_fir_decimate_instance_f32   DECIMATE_NR;
float32_t           decimNRState[FIR_RXAUDIO_BLOCK_SIZE + 4];

static	arm_fir_interpolate_instance_f32 INTERPOLATE_NR;
float32_t			interplNRState[FIR_RXAUDIO_BLOCK_SIZE + NR_INTERPOLATE_NO_TAPS];

#define IIR_RX_STATE_ARRAY_SIZE    (IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES_MAX)
// variables for RX IIR filters
static float32_t		iir_rx_state[NUM_AUDIO_CHANNELS][IIR_RX_STATE_ARRAY_SIZE];
static arm_iir_lattice_instance_f32	IIR_PreFilter[NUM_AUDIO_CHANNELS];

// variables for RX antialias IIR filter
static float32_t		iir_aa_state[NUM_AUDIO_CHANNELS][IIR_RX_STATE_ARRAY_SIZE];
static arm_iir_lattice_instance_f32	IIR_AntiAlias[NUM_AUDIO_CHANNELS];

// static float32_t Koeff[20];
// variables for RX manual notch, manual peak & bass shelf IIR biquad filter
arm_biquad_casd_df1_inst_f32 IIR_biquad_1[NUM_AUDIO_CHANNELS] =
{
        {
                .numStages = 4,
                .pCoeffs = (float32_t *)(float32_t [])
                {
                    1,0,0,0,0,  1,0,0,0,0,  1,0,0,0,0,  1,0,0,0,0
                }, // 4 x 5 = 20 coefficients

                .pState = (float32_t *)(float32_t [])
                {
                    0,0,0,0,   0,0,0,0,   0,0,0,0,   0,0,0,0
                } // 4 x 4 = 16 state variables
        },
#ifdef USE_TWO_CHANNEL_AUDIO
        {
                .numStages = 4,
                .pCoeffs = (float32_t *)(float32_t [])
                {
                    1,0,0,0,0,  1,0,0,0,0,  1,0,0,0,0,  1,0,0,0,0
                }, // 3 x 5 = 15 coefficients

                .pState = (float32_t *)(float32_t [])
                {
                    0,0,0,0,   0,0,0,0,   0,0,0,0,   0,0,0,0
                } // 3 x 4 = 12 state variables
        }
#endif
};


// variables for RX treble shelf IIR biquad filter
arm_biquad_casd_df1_inst_f32 IIR_biquad_2[NUM_AUDIO_CHANNELS] =
{
        {
                .numStages = 1,
                .pCoeffs = (float32_t *)(float32_t [])
                {
                    1,0,0,0,0
                }, // 1 x 5 = 5 coefficients

                .pState = (float32_t *)(float32_t [])
                {
                    0,0,0,0
                } // 1 x 4 = 4 state variables
        },
#ifdef USE_TWO_CHANNEL_AUDIO
        {
                .numStages = 1,
                .pCoeffs = (float32_t *)(float32_t [])
                {
                    1,0,0,0,0
                }, // 1 x 5 = 5 coefficients

                .pState = (float32_t *)(float32_t [])
                {
                    0,0,0,0
                } // 1 x 4 = 4 state variables
        }
#endif
};

// variables for TX bass & treble adjustment IIR biquad filter
static arm_biquad_casd_df1_inst_f32 IIR_TX_biquad =
{
        .numStages = 3,
        .pCoeffs = (float32_t *)(float32_t [])
        {
            1,0,0,0,0,  1,0,0,0,0,  1,0,0,0,0
        }, // 3 x 5 = 15 coefficients

        .pState = (float32_t *)(float32_t [])
        {
            0,0,0,0,   0,0,0,0,   0,0,0,0
        } // 3 x 4 = 12 state variables
};

// variables for ZoomFFT lowpass filtering
static arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_I =
{
        .numStages = 4,
        .pCoeffs = (float32_t *)(float32_t [])
        {
            1,0,0,0,0,  1,0,0,0,0 // passthru
        }, // 2 x 5 = 10 coefficients

        .pState = (float32_t *)(float32_t [])
        {
            0,0,0,0,   0,0,0,0,    0,0,0,0,   0,0,0,0
        } // 4 x 4 = 16 state variables
};

static arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_Q =
{
        .numStages = 4,
        .pCoeffs = (float32_t *)(float32_t [])
        {
            1,0,0,0,0,  1,0,0,0,0 // passthru
        }, // 2 x 5 = 10 coefficients

        .pState = (float32_t *)(float32_t [])
        {
            0,0,0,0,   0,0,0,0,   0,0,0,0,   0,0,0,0
        } // 4 x 4 = 16 state variables
};

// sr = 12ksps, Fstop = 2k7, we lowpass-filtered the audio already in the main aido path (IIR),
// so only the minimum size filter (4 taps) is used here
static float32_t NR_decimate_coeffs [4] = {0.099144206287089282, 0.492752007869707798, 0.492752007869707798, 0.099144206287089282};

// 12ksps, Fstop = 2k7, KAISER, 40 taps, a good interpolation filter after the interpolation, maybe too many taps?
static float32_t NR_interpolate_coeffs [NR_INTERPOLATE_NO_TAPS] = {-495.0757586677611930E-6, 0.001320676868426568, 0.001533845835568487,-0.002357633129357554,-0.003572560455091757, 0.003388797052024843, 0.007032840952358404,-0.003960820803871866,-0.012365795129023015, 0.003357357660531775, 0.020101326014980946,-475.4964584295063900E-6,-0.031094910247864812,-0.006597041050034579, 0.047436525317202147, 0.022324808965607446,-0.076541709512474090,-0.064246467306504046, 0.167750545742874818, 0.427794841657261171, 0.427794841657261171, 0.167750545742874818,-0.064246467306504046,-0.076541709512474090, 0.022324808965607446, 0.047436525317202147,-0.006597041050034579,-0.031094910247864812,-475.4964584295063900E-6, 0.020101326014980946, 0.003357357660531775,-0.012365795129023015,-0.003960820803871866, 0.007032840952358404, 0.003388797052024843,-0.003572560455091757,-0.002357633129357554, 0.001533845835568487, 0.001320676868426568,-495.0757586677611930E-6};

// this is wrong! Interpolation filters act at the sample rate AFTER the interpolation, in this case at 12ksps
// 6ksps, Fstop = 2k65, KAISER
//static float32_t NR_interpolate_coeffs [NR_INTERPOLATE_NO_TAPS] = {-903.6623076669911820E-6, 0.001594488333496738,-0.002320508982899863, 0.002832351511451895,-0.002797105957386612, 0.001852836963547170, 308.6133633078010230E-6,-0.003842008360761881, 0.008649943961959465,-0.014305251526745446, 0.020012524686320185,-0.024618364878703208, 0.026664997481476788,-0.024458388333600374, 0.016080841021827566, 818.1032282579135430E-6,-0.029933800539235892, 0.079833661336890141,-0.182038248016552551, 0.626273078268197225, 0.626273078268197225,-0.182038248016552551, 0.079833661336890141,-0.029933800539235892, 818.1032282579135430E-6, 0.016080841021827566,-0.024458388333600374, 0.026664997481476788,-0.024618364878703208, 0.020012524686320185,-0.014305251526745446, 0.008649943961959465,-0.003842008360761881, 308.6133633078010230E-6, 0.001852836963547170,-0.002797105957386612, 0.002832351511451895,-0.002320508982899863, 0.001594488333496738,-903.6623076669911820E-6};

static float32_t* mag_coeffs[MAGNIFY_NUM] =

{

        // for Index 0 [1xZoom == no zoom] the mag_coeffs will a NULL  ptr, since the filter is not going to be used in this  mode!
        (float32_t*)NULL,

        (float32_t*)(const float32_t[]){
            // 2x magnify - index 1
            // 12kHz, sample rate 48k, 60dB stopband, elliptic
            // a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
            // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
            0.228454526413293696,
            0.077639329099949764,
            0.228454526413293696,
            0.635534925142242080,
            -0.170083307068779194,

            0.436788292542003964,
            0.232307972937606161,
            0.436788292542003964,
            0.365885230717786780,
            -0.471769788739400842,

            0.535974654742658707,
            0.557035600464780845,
            0.535974654742658707,
            0.125740787233286133,
            -0.754725697183384336,

            0.501116342273565607,
            0.914877831284765408,
            0.501116342273565607,
            0.013862536615004284,
            -0.930973052446900984  },

            (float32_t*)(const float32_t[]){
                // 4x magnify - index 2
                // 6kHz, sample rate 48k, 60dB stopband, elliptic
                // a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
                // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
                0.182208761527446556,
                -0.222492493114674145,
                0.182208761527446556,
                1.326111070880959810,
                -0.468036100821178802,

                0.337123762652097259,
                -0.366352718812586853,
                0.337123762652097259,
                1.337053579516321200,
                -0.644948386007929031,

                0.336163175380826074,
                -0.199246162162897811,
                0.336163175380826074,
                1.354952684569386670,
                -0.828032873168141115,

                0.178588201750411041,
                0.207271695028067304,
                0.178588201750411041,
                1.386486967455699220,
                -0.950935065984588657  },

                (float32_t*)(const float32_t[]){
                    // 8x magnify - index 3
                    // 3kHz, sample rate 48k, 60dB stopband, elliptic
                    // a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
                    // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
                    0.185643392652478922,
                    -0.332064345389014803,
                    0.185643392652478922,
                    1.654637402827731090,
                    -0.693859842743674182,

                    0.327519300813245984,
                    -0.571358085216950418,
                    0.327519300813245984,
                    1.715375037176782860,
                    -0.799055553586324407,

                    0.283656142708241688,
                    -0.441088976843048652,
                    0.283656142708241688,
                    1.778230635987093860,
                    -0.904453944560528522,

                    0.079685368654848945,
                    -0.011231810140649204,
                    0.079685368654848945,
                    1.825046003243238070,
                    -0.973184930412286708  },

                    (float32_t*)(const float32_t[]){
                        // 16x magnify - index 4
                        // 1k5, sample rate 48k, 60dB stopband, elliptic
                        // a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
                        // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
                        0.194769868656866380,
                        -0.379098413160710079,
                        0.194769868656866380,
                        1.824436402073870810,
                        -0.834877726226893380,

                        0.333973874901496770,
                        -0.646106479315673776,
                        0.333973874901496770,
                        1.871892825636887640,
                        -0.893734096124207178,

                        0.272903880596429671,
                        -0.513507745397738469,
                        0.272903880596429671,
                        1.918161772571113750,
                        -0.950461788366234739,

                        0.053535383722369843,
                        -0.069683422367188122,
                        0.053535383722369843,
                        1.948900719896301760,
                        -0.986288064973853129 },

                        (float32_t*)(const float32_t[]){
                            // 32x magnify - index 5
                            // 750Hz, sample rate 48k, 60dB stopband, elliptic
                            // a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
                            // Iowa Hills IIR Filter Designer, DD4WH Aug 16th 2016
                            0.201507402588557594,
                            -0.400273615727755550,
                            0.201507402588557594,
                            1.910767558906650840,
                            -0.913508748356010480,

                            0.340295203367131205,
                            -0.674930558961690075,
                            0.340295203367131205,
                            1.939398230905991390,
                            -0.945058078678563840,

                            0.271859921641011359,
                            -0.535453706265515361,
                            0.271859921641011359,
                            1.966439529620203740,
                            -0.974705666636711099,

                            0.047026497485465592,
                            -0.084562104085501480,
                            0.047026497485465592,
                            1.983564238653704900,
                            -0.993055129539134551 }



};

//******* From here 2 set of filters for the I/Q FreeDV aliasing filter**********

// I- and Q- Filter instances for FreeDV downsampling aliasing filters

static arm_biquad_casd_df1_inst_f32 IIR_biquad_FreeDV_I =
{
        .numStages = 2,
        .pCoeffs = (float32_t *)(float32_t [])
        {
            1,0,0,0,0,  1,0,0,0,0 // passthru
        }, // 2 x 5 = 10 coefficients

        .pState = (float32_t *)(float32_t [])
        {
            0,0,0,0,   0,0,0,0
        } // 2 x 4 = 8 state variables
};

static arm_biquad_casd_df1_inst_f32 IIR_biquad_FreeDV_Q =
{
        .numStages = 2,
        .pCoeffs = (float32_t *)(float32_t [])
        {
            1,0,0,0,0,  1,0,0,0,0 // passthru
        }, // 2 x 5 = 10 coefficients

        .pState = (float32_t *)(float32_t [])
        {
            0,0,0,0,   0,0,0,0
        } // 2 x 4 = 8 state variables
};

static float32_t* FreeDV_coeffs[1] =
{
        (float32_t*)(const float32_t[]){
            // index 1
            // 2,4kHz, sample rate 48k, 50dB stopband, elliptic
            // only 2 stages!!!!!
            // a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
            // Iowa Hills IIR Filter Designer, DL2FW 20-10-16



            0.083165011486267731,
            -0.118387356334666696,
            0.083165011486267731,
            1.666027486884941840,
            -0.713970153522810569,

            0.068193683664968877,
            -0.007220581127135660,
            0.068193683664968877,
            1.763363677375461290,
            -0.892530463578263378
        }
};

//******* End of 2 set of filters for the I/Q FreeDV aliasing filter**********



// variables for FM squelch IIR filters
static float32_t	iir_squelch_rx_state[IIR_RX_STATE_ARRAY_SIZE];
static arm_iir_lattice_instance_f32	IIR_Squelch_HPF;

// variables for TX IIR filter
float32_t		iir_tx_state[IIR_RX_STATE_ARRAY_SIZE];
arm_iir_lattice_instance_f32	IIR_TXFilter;
arm_iir_lattice_instance_f32	IIR_FreeDV_RX_Filter;  //DL2FW: temporary installed FreeDV RX Audio Filter
float32_t		iir_FreeDV_RX_state[IIR_RX_STATE_ARRAY_SIZE];

// S meter public
__IO SMeter					sm;

// ATTENTION: These data structures have been placed in CCM Memory (64k)
// IF THE SIZE OF  THE DATA STRUCTURE GROWS IT WILL QUICKLY BE OUT OF SPACE IN CCM
// Be careful! Check mchf-eclipse.map for current allocation
AudioDriverState   __MCHF_SPECIALMEM ads;
AudioDriverBuffer  __MCHF_SPECIALMEM adb;
#ifdef OBSOLETE_NR
LMSData            __MCHF_SPECIALMEM lmsData;
#endif

#ifdef USE_LMS_AUTONOTCH
LMSData            __MCHF_SPECIALMEM lmsData;
//LMSData            lmsData;
#endif

#ifdef USE_LEAKY_LMS
lLMS leakyLMS;
#endif

SnapCarrier   sc;

/**
 * @returns offset frequency in Hz for current frequency translate mode
 */
int32_t AudioDriver_GetTranslateFreq()
{
    int32_t fdelta = 0;
    switch (ts.iq_freq_mode)
    {
    case FREQ_IQ_CONV_P6KHZ:
        fdelta = 6000;
        break;
    case FREQ_IQ_CONV_M6KHZ:
        fdelta = - 6000;
        break;
    case FREQ_IQ_CONV_P12KHZ:
        fdelta = 12000;
        break;
    case FREQ_IQ_CONV_M12KHZ:
        fdelta = -12000;
        break;
    }
    return fdelta;
}

static void AudioDriver_InitFilters(void);
void AudioDriver_SetupAgcWdsp(void);
//
// THE FOLLOWING FUNCTION HAS BEEN TESTED, BUT NOT USED - see the function "audio_rx_freq_conv"
//*----------------------------------------------------------------------------
//* Function Name       : audio_driver_config_nco
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*
void audio_driver_config_nco(void)
{
	// Configure NCO for the frequency translate function - NOT USED for the "Static" local oscillator!
	// see "audio_driver.h" for values
	ads.Osc_Cos = CONV_NCO_COS;
	ads.Osc_Sin = CONV_NCO_SIN;
	ads.Osc_Vect_Q = 1;
	ads.Osc_Vect_I = 0;
	ads.Osc_Gain = 0;
	ads.Osc_Q = 0;
	ads.Osc_I = 0;
}
//
 */

void AudioDriver_Init(void)
{
    const uint32_t word_size = WORD_SIZE_16;

    // CW module init
    CwGen_Init();

    RttyDecoder_Init();
    PskDecoder_Init();

    // Audio filter disabled
    ts.dsp_inhibit = 1;
    ads.af_disabled = 1;

    // Reset S meter public
    sm.skip		= 0;
    sm.s_count	= 0;
    sm.curr_max	= 0;
    sm.gain_calc = 0;	// gain calculation used for S-meter
#if 0
    ads.agc_val = 1;			// Post AF Filter gain (AGC)
    ads.agc_var = 1;			// used in AGC processing
    ads.agc_calc = 1;			// used in AGC processing
    ads.agc_holder = 1;			// initialize holder for AGC value
    for(uint32_t x = 0; x < BUFF_LEN; x++)	// initialize running buffer for AGC delay
    {
        adb.agc_valbuf[x] = 1;
    }
#endif
    ads.alc_val = 1;			// init TX audio auto-level-control (ALC)
    //
    ads.fm_sql_avg = 0;			// init FM squelch averaging
    ads.fm_subaudible_tone_word = 0;	// actively-used variable in producing the tone (0 disabled tone generation)
    ads.fm_tone_burst_word = 0;			// this is the actively-used DDS tone word in the tone burst frequency generator
    ads.fm_subaudible_tone_det_freq = 0;	// frequency, in Hz, of currently-selected subaudible tone for detection
    ads.fm_subaudible_tone_gen_freq = 0;	// frequency, in Hz, of currently-selected subaudible tone for generation
    ads.beep_loudness_factor = 0;			// scaling factor for beep loudness
    //
    ads.fm_tone_burst_active = 0;		// this is TRUE of the tone burst is actively being generated
    ads.fm_squelched = 0;				// TRUE if FM receiver audio is to be squelched
    ads.fm_subaudible_tone_detected = 0;	// TRUE if subaudible tone has been detected
    //
    ads.decimation_rate	=	RX_DECIMATION_RATE_12KHZ;		// Decimation rate, when enabled

    //    ads.fade_leveler = 0;

    //
    //
//    AudioManagement_CalcAGCDecay();	// initialize AGC decay ("hang time") values
    //
//    AudioManagement_CalcRFGain();		// convert from user RF gain value to "working" RF gain value
    //
    AudioManagement_CalcALCDecay();	// initialize ALC decay values
    //
//    AudioManagement_CalcAGCVals();	// calculate AGC internal values from user settings
    //
//    AudioManagement_CalcNB_AGC();		// set up noise blanker AGC values
    //
    ts.cw_lsb = RadioManagement_CalculateCWSidebandMode();	// set up CW sideband mode setting
    //
    // The "active" NCO in the frequency translate function is NOT used, but rather a "static" sine that is an integer divisor of the sample rate.
    //
    //audio_driver_config_nco();	// Configure the NCO in the frequency translate function
    //
    ads.tx_filter_adjusting = 0;	// used to disable TX I/Q filter during adjustment
    // Audio Filter Init init
    AudioDriver_InitFilters();

#ifdef USE_LEAKY_LMS
    /////////////////////// LEAKY LMS noise reduction
    leakyLMS.n_taps =     64; //64;                       // taps
    leakyLMS.delay =    16; //16;                       // delay
    leakyLMS.dline_size = LEAKYLMSDLINE_SIZE;
    //int ANR_buff_size = FFT_length / 2.0;
    leakyLMS.position = 0;
    leakyLMS.two_mu =   0.0001;                     // two_mu --> "gain"
    leakyLMS.two_mu_int = 100;
    leakyLMS.gamma =    0.1;                      // gamma --> "leakage"
    leakyLMS.gamma_int = 100;
    leakyLMS.lidx =     120.0;                      // lidx
    leakyLMS.lidx_min = 0.0;                      // lidx_min
    leakyLMS.lidx_max = 200.0;                      // lidx_max
    leakyLMS.ngamma =   0.001;                      // ngamma
    leakyLMS.den_mult = 6.25e-10;                   // den_mult
    leakyLMS.lincr =    1.0;                      // lincr
    leakyLMS.ldecr =    3.0;                     // ldecr
    //int leakyLMS.mask = leakyLMS.dline_size - 1;
    leakyLMS.mask = LEAKYLMSDLINE_SIZE - 1;
    leakyLMS.in_idx = 0;
    leakyLMS.on = 0;
    leakyLMS.notch = 0;
    /////////////////////// LEAKY LMS END
#endif

    ts.codec_present = Codec_Reset(ts.samp_rate,word_size) == HAL_OK;

    // Start DMA transfers
    UhsdrHwI2s_Codec_StartDMA();

    // Audio filter enabled
    ads.af_disabled = 0;
    ts.dsp_inhibit = 0;

}

void AudioDriver_SetSamPllParameters()
{

    // definitions and intializations for synchronous AM demodulation = SAM
    //    adb.DF = 1.0; //ads.decimation_rate;
    adb.DF = (float32_t)(ads.decimation_rate);
    //ads.pll_fmax_int = 2500;
    adb.pll_fmax = (float32_t)ads.pll_fmax_int;
    // DX adjustments: zeta = 0.15, omegaN = 100.0
    // very stable, but does not lock very fast
    // standard settings: zeta = 1.0, omegaN = 250.0
    // maybe user can choose between slow (DX), medium, fast SAM PLL
    // zeta / omegaN
    // DX = 0.2, 70
    // medium 0.6, 200
    // fast 1.0, 500
    //ads.zeta_int = 80; // zeta * 100 !!!
    // 0.01;// 0.001; // 0.1; //0.65; // PLL step response: smaller, slower response 1.0 - 0.1
    //ads.omegaN_int = 250; //200.0; // PLL bandwidth 50.0 - 1000.0
    adb.omegaN = (float32_t)ads.omegaN_int;
    adb.zeta = (float32_t)ads.zeta_int / 100.0;
    //pll
    adb.omega_min = - (2.0 * PI * adb.pll_fmax * adb.DF / IQ_SAMPLE_RATE_F); //-0.5235987756; //
    adb.omega_max = (2.0 * PI * adb.pll_fmax * adb.DF / IQ_SAMPLE_RATE_F); //0.5235987756; //
    adb.g1 = (1.0 - expf(-2.0 * adb.omegaN * adb.zeta * adb.DF / IQ_SAMPLE_RATE_F)); //0.0082987073611; //
    adb.g2 = (- adb.g1 + 2.0 * (1 - expf(- adb.omegaN * adb.zeta * adb.DF / IQ_SAMPLE_RATE_F)
            * cosf(adb.omegaN * adb.DF / IQ_SAMPLE_RATE_F * sqrtf(1.0 - adb.zeta * adb.zeta)))); //0.01036367597097734813032783691644; //
    //fade leveler
    //    ads.tauR_int = 20; // -->  / 1000 = 0.02
    //    ads.tauI_int = 140; // --> / 100 = 1.4
    adb.tauR = 0.02; // ((float32_t)ads.tauR_int) / 1000.0; //0.02; // original 0.02;
    adb.tauI = 1.4; // ((float32_t)ads.tauI_int) / 100.0; //1.4; // original 1.4;
    adb.mtauR = (expf(- adb.DF / (IQ_SAMPLE_RATE_F * adb.tauR))); //0.99948;
    adb.onem_mtauR = (1.0 - adb.mtauR);
    adb.mtauI = (expf(- adb.DF / (IQ_SAMPLE_RATE_F * adb.tauI))); //0.99999255955;
    adb.onem_mtauI = (1.0 - adb.mtauI);
}

void AudioDriver_SetRxAudioProcessingSAM(uint8_t dmod_mode)
{

    AudioDriver_SetSamPllParameters();

    //sideband separation, these values never change
    adb.c0[0] = -0.328201924180698;
    adb.c0[1] = -0.744171491539427;
    adb.c0[2] = -0.923022915444215;
    adb.c0[3] = -0.978490468768238;
    adb.c0[4] = -0.994128272402075;
    adb.c0[5] = -0.998458978159551;
    adb.c0[6] = -0.999790306259206;

    adb.c1[0] = -0.0991227952747244;
    adb.c1[1] = -0.565619728761389;
    adb.c1[2] = -0.857467122550052;
    adb.c1[3] = -0.959123933111275;
    adb.c1[4] = -0.988739372718090;
    adb.c1[5] = -0.996959189310611;
    adb.c1[6] = -0.999282492800792;

    // these change during operation
    adb.M_c1 = 0.0;
    adb.M_c2 = 1.0;
    adb.teta1_old = 0.0;
    adb.teta2_old = 0.0;
    adb.teta3_old = 0.0;
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Cascaded biquad (notch, peak, lowShelf, highShelf) [DD4WH, april 2016]
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// biquad_1 :   Notch & peak filters & lowShelf (Bass) in the decimated path
// biquad 2 :   Treble in the 48kHz path
// TX_biquad:   Bass & Treble in the 48kHz path
// DSP Audio-EQ-cookbook for generating the coeffs of the filters on the fly
// www.musicdsp.org/files/Audio-EQ-Cookbook.txt  [by Robert Bristow-Johnson]
//
// the ARM algorithm assumes the biquad form
// y[n] = coeffs[B0] * x[n] + coeffs[B1] * x[n-1] + coeffs[B2] * x[n-2] + coeffs[A1] * y[n-1] + a2 * y[n-2]
//
// However, the cookbook formulae by Robert Bristow-Johnson AND the Iowa Hills IIR Filter designer
// use this formula:
//
// y[n] = coeffs[B0] * x[n] + coeffs[B1] * x[n-1] + coeffs[B2] * x[n-2] - coeffs[A1] * y[n-1] - coeffs[A2] * y[n-2]
//
// Therefore, we have to use negated coeffs[A1] and coeffs[A2] for use with the ARM function
// notch implementation
//
// we also have to divide every coefficient by a0 !
// y[n] = coeffs[B0]/a0 * x[n] + coeffs[B1]/a0 * x[n-1] + coeffs[B2]/a0 * x[n-2] - coeffs[A1]/a0 * y[n-1] - coeffs[A2]/a0 * y[n-2]
//
//

#define B0 0
#define B1 1
#define B2 2
#define A1 3
#define A2 4

/**
 * @brief Biquad Filter Init Helper function which copies the biquad coefficients into the filter array itself
 */
void AudioDriver_SetBiquadCoeffs(float32_t* coeffsTo,const float32_t* coeffsFrom)
{
    coeffsTo[0] = coeffsFrom[0];
    coeffsTo[1] = coeffsFrom[1];
    coeffsTo[2] = coeffsFrom[2];
    coeffsTo[3] = coeffsFrom[3];
    coeffsTo[4] = coeffsFrom[4];
}

/**
 * @brief Biquad Filter Init Helper function which copies the biquad coefficients into all filter instances of the 1 or 2 channel audio
 * @param biquad_inst_array a pointer to an either 1 or 2 element sized array of biquad filter instances. Make sure the array has the expected size!
 * @param idx of first element of stage coefficients (i.e. 0, 5, 10, ... ) since we have 5 element coeff arrays per stage
 */
void AudioDriver_SetBiquadCoeffsAllInstances(arm_biquad_casd_df1_inst_f32 biquad_inst_array[NUM_AUDIO_CHANNELS], uint32_t idx, const float32_t* coeffsFrom)
{
    for (int chan = 0; chan < NUM_AUDIO_CHANNELS; chan++)
     {
         AudioDriver_SetBiquadCoeffs(&biquad_inst_array[chan].pCoeffs[idx],coeffsFrom);
     }
}

/**
 * @brief Biquad Filter Init Helper function which applies the filter specific scaling to calculated coefficients
 */
void AudioDriver_ScaleBiquadCoeffs(float32_t coeffs[5],const float32_t scalingA, const float32_t scalingB)
{
    coeffs[A1] = coeffs[A1] / scalingA;
    coeffs[A2] = coeffs[A2] / scalingA;

    coeffs[B0] = coeffs[B0] / scalingB;
    coeffs[B1] = coeffs[B1] / scalingB;
    coeffs[B2] = coeffs[B2] / scalingB;
}

/**
 * @brief Biquad Filter Init Helper function to calculate a notch filter aka narrow bandstop filter
 */
void AudioDriver_CalcBandstop(float32_t coeffs[5], float32_t f0, float32_t FS)
{
     float32_t Q = 10; // larger Q gives narrower notch
     float32_t w0 = 2 * PI * f0 / FS;
     float32_t alpha = sinf(w0) / (2 * Q);

     coeffs[B0] = 1;
     coeffs[B1] = - 2 * cosf(w0);
     coeffs[B2] = 1;
     float32_t scaling = 1 + alpha;
     coeffs[A1] = 2 * cosf(w0); // already negated!
     coeffs[A2] = alpha - 1; // already negated!

     AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, scaling);
}

/**
 * @brief Biquad Filter Init Helper function to calculate a peak filter aka a narrow bandpass filter
 */
void AudioDriver_CalcBandpass(float32_t coeffs[5], float32_t f0, float32_t FS)
{
    /*       // peak filter = peaking EQ
    f0 = ts.peak_frequency;
    //Q = 15; //
    // bandwidth in octaves between midpoint (Gain / 2) gain frequencies
    float32_t BW = 0.05;
    w0 = 2 * PI * f0 / FSdec;
    //alpha = sin(w0) / (2 * Q);
    alpha = sin (w0) * sinh( log(2) / 2 * BW * w0 / sin(w0) );
    float32_t Gain = 12;
    A = powf(10.0, (Gain/40.0));
    coeffs[B0] = 1 + (alpha * A);
    coeffs[B1] = - 2 * cos(w0);
    coeffs[B2] = 1 - (alpha * A);
    a0 = 1 + (alpha / A);
    coeffs[A1] = 2 * cos(w0); // already negated!
    coeffs[A2] = (alpha/A) - 1; // already negated!
     */
    /*        // test the BPF coefficients, because actually we want a "peak" filter without gain!
    // Bandpass filter constant 0dB peak gain
    // this filter was tested: "should have more gain and less Q"
    f0 = ts.peak_frequency;
    Q = 20; //
    w0 = 2 * PI * f0 / FSdec;
    alpha = sin(w0) / (2 * Q);
//        A = 1; // gain = 1
    //        A = 3; // 10^(10/40); 15dB gain

    coeffs[B0] = alpha;
    coeffs[B1] = 0;
    coeffs[B2] = - alpha;
    a0 = 1 + alpha;
    coeffs[A1] = 2 * cos(w0); // already negated!
    coeffs[A2] = alpha - 1; // already negated!
     */
    // BPF: constant skirt gain, peak gain = Q
    float32_t Q = 4; //
    float32_t BW = 0.03;
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t alpha = sinf (w0) * sinhf( log(2) / 2 * BW * w0 / sinf(w0) ); //

    coeffs[B0] = Q * alpha;
    coeffs[B1] = 0;
    coeffs[B2] = - Q * alpha;
    float32_t scaling = 1 + alpha;
    coeffs[A1] = 2 * cosf(w0); // already negated!
    coeffs[A2] = alpha - 1; // already negated!

    AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, scaling);

}

/**
 * @brief Biquad Filter Init Helper function to calculate a treble adjustment filter aka high shelf filter
 */
void AudioDriver_CalcHighShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t A = pow10f(gain/40.0); // gain ranges from -20 to 5
    float32_t alpha = sinf(w0) / 2 * sqrtf( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cosf(w0);
    float32_t twoAa = 2 * sqrtf(A) * alpha;
    // highShelf
    //
    coeffs[B0] = A *        ( (A + 1) + (A - 1) * cosw0 + twoAa );
    coeffs[B1] = - 2 * A *  ( (A - 1) + (A + 1) * cosw0         );
    coeffs[B2] = A *        ( (A + 1) + (A - 1) * cosw0 - twoAa );
    float32_t scaling =       (A + 1) - (A - 1) * cosw0 + twoAa ;
    coeffs[A1] = - 2 *      ( (A - 1) - (A + 1) * cosw0         ); // already negated!
    coeffs[A2] = twoAa      - (A + 1) + (A - 1) * cosw0; // already negated!


    //    DCgain = 2; //
    //    DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (- coeffs[A1] - coeffs[A2])); // takes into account that coeffs[A1] and coeffs[A2] are already negated!
    float32_t DCgain = 1.0 * scaling;

    AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, DCgain);
}

/**
 * @brief Biquad Filter Init Helper function to calculate a bass adjustment filter aka low shelf filter
 */
void AudioDriver_CalcLowShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{

    float32_t w0 = 2 * PI * f0 / FS;
    float32_t A = pow10f(gain/40.0); // gain ranges from -20 to 5

    float32_t alpha = sinf(w0) / 2 * sqrtf( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cosf(w0);
    float32_t twoAa = 2 * sqrtf(A) * alpha;

    // lowShelf
    coeffs[B0] = A *        ( (A + 1) - (A - 1) * cosw0 + twoAa );
    coeffs[B1] = 2 * A *    ( (A - 1) - (A + 1) * cosw0         );
    coeffs[B2] = A *        ( (A + 1) - (A - 1) * cosw0 - twoAa );
    float32_t scaling =       (A + 1) + (A - 1) * cosw0 + twoAa ;
    coeffs[A1] = 2 *        ( (A - 1) + (A + 1) * cosw0         ); // already negated!
    coeffs[A2] = twoAa      - (A + 1) - (A - 1) * cosw0; // already negated!

    // scaling the feedforward coefficients for gain adjustment !
    // "DC gain of an IIR filter is the sum of the filters� feedforward coeffs divided by
    // 1 minus the sum of the filters� feedback coeffs" (Lyons 2011)
    //    float32_t DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (coeffs[A1] + coeffs[A2]));
    // does not work for some reason?
    // I take a divide by a constant instead !
    //    DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (- coeffs[A1] - coeffs[A2])); // takes into account that coeffs[A1] and coeffs[A2] are already negated!

    float32_t DCgain = 1.0 * scaling; //


    AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, DCgain);

}

/**
 * @brief Biquad Filter Init Helper function to calculate a notch filter aka narrow bandstop filter with variable bandwidth
 */
void AudioDriver_CalcNotch(float32_t coeffs[5], float32_t f0, float32_t BW, float32_t FS)
{

    float32_t w0 = 2 * PI * f0 / FS;
    float32_t alpha = sinf(w0)*sinh( logf(2.0)/2.0 * BW * w0/sinf(w0) );

    coeffs[B0] = 1;
    coeffs[B1] = - 2 * cosf(w0);
    coeffs[B2] = 1;
    float32_t scaling = 1 + alpha;
    coeffs[A1] = 2 * cosf(w0); // already negated!
    coeffs[A2] = alpha - 1; // already negated!


    AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, scaling);
}

static const float32_t biquad_passthrough[] = { 1, 0, 0, 0, 0 };


/**
 * @brief Biquad Filter Init used for processing audio for RX and TX
 */
void AudioDriver_SetRxTxAudioProcessingAudioFilters()
{
    float32_t FSdec = 24000.0; // we need the sampling rate in the decimated path for calculation of the coefficients

    if (FilterPathInfo[ts.filter_path].sample_rate_dec == RX_DECIMATION_RATE_12KHZ)
    {
        FSdec = 12000.0;
    }

    const float32_t FS = IQ_SAMPLE_RATE; // we need this for the treble filter

    // the notch filter is in biquad 1 and works at the decimated sample rate FSdec

    float32_t coeffs[5];
    const float32_t *coeffs_ptr;

    // setting the Coefficients in the notch filter instance
    if (ts.dsp_active & DSP_MNOTCH_ENABLE)
    {
        AudioDriver_CalcBandstop(coeffs, ts.notch_frequency , FSdec);
        coeffs_ptr = coeffs;
    }
    else   // passthru
    {
        coeffs_ptr = biquad_passthrough;
    }

    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 0, coeffs_ptr);

    // this is an auto-notch-filter detected by the NR algorithm
    // biquad 1, 4th stage
    //    if(0) // temporarily deactivated
    //    if((ts.dsp_active & DSP_NOTCH_ENABLE) && (FilterPathInfo[ts.filter_path].sample_rate_dec == RX_DECIMATION_RATE_12KHZ))

    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 15, biquad_passthrough);

    // the peak filter is in biquad 1 and works at the decimated sample rate FSdec
    if(ts.dsp_active & DSP_MPEAK_ENABLE)
    {
        AudioDriver_CalcBandpass(coeffs, ts.peak_frequency, FSdec);
        coeffs_ptr = coeffs;
    }
    else   //passthru
    {
        coeffs_ptr = biquad_passthrough;
    }

    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 5, coeffs_ptr);

    // EQ shelving filters
    //
    // the bass filter is in biquad 1 and works at the decimated sample rate FSdec
    //
    // Bass / lowShelf
    AudioDriver_CalcLowShelf(coeffs, 250, 0.7, ts.bass_gain, FSdec);
    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 10, coeffs);

    // Treble = highShelf
    // the treble filter is in biquad 2 and works at 48000ksps
    AudioDriver_CalcHighShelf(coeffs, 3500, 0.9, ts.treble_gain, FS);
    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_2, 0, coeffs);


    // coefficient calculation for TX bass & treble adjustment
    // the TX treble filter is in IIR_TX_biquad and works at 48000ksps
    AudioDriver_CalcHighShelf(coeffs, 1700, 0.9, ts.tx_treble_gain, FS);
    AudioDriver_SetBiquadCoeffs(&IIR_TX_biquad.pCoeffs[0],coeffs);

    // the TX bass filter is in IIR_TX_biquad and works at 48000 sample rate
    AudioDriver_CalcLowShelf(coeffs, 300, 0.7, ts.tx_bass_gain, FS);
    AudioDriver_SetBiquadCoeffs(&IIR_TX_biquad.pCoeffs[5],coeffs);

}


/**
 * @brief configures filters/dsp etc. so that audio processing works according to the current configuration
 * @param dmod_mode needs to know the demodulation mode
 * @param reset_dsp_nr whether it is supposed to reset also DSP related filters (in most cases false is to be used here)
 */
void AudioDriver_SetRxAudioProcessing(uint8_t dmod_mode, bool reset_dsp_nr)
{
    // WARNING:  You CANNOT reliably use the built-in IIR and FIR "init" functions when using CONST-based coefficient tables!  If you do so, you risk filters
    //  not initializing properly!  If you use the "init" functions, you MUST copy CONST-based coefficient tables to RAM first!
    //  This information is from recommendations by online references for using ARM math/DSP functions
    ts.dsp_inhibit++;
    ads.af_disabled++;

    // make sure we have a proper filter path for the given mode

    // the commented out part made the code  only look at last used/selected filter path if the current filter path is not applicable
    // with it commented out the filter path is ALWAYS loaded from the last used/selected memory
    // I.e. setting the ts.filter_path anywere else in the code is useless. You have to call AudioFilter_NextApplicableFilterPath in order to
    // select a new filter path as this sets the last used/selected memory for a demod mode.

    ts.filter_path = AudioFilter_NextApplicableFilterPath(PATH_ALL_APPLICABLE|PATH_LAST_USED_IN_MODE,AudioFilter_GetFilterModeFromDemodMode(dmod_mode),ts.filter_path);

    for (int chan = 0; chan < NUM_AUDIO_CHANNELS; chan++)
    {
        if (FilterPathInfo[ts.filter_path].pre_instance != NULL)
        {
            // if we turn on a filter, set the number of members to the number of elements last
            IIR_PreFilter[chan].pkCoeffs = FilterPathInfo[ts.filter_path].pre_instance->pkCoeffs; // point to reflection coefficients
            IIR_PreFilter[chan].pvCoeffs = FilterPathInfo[ts.filter_path].pre_instance->pvCoeffs; // point to ladder coefficients
            IIR_PreFilter[chan].numStages = FilterPathInfo[ts.filter_path].pre_instance->numStages;        // number of stages
        }
        else
        {
            // if we turn off a filter, set the number of members to 0 first
            IIR_PreFilter[chan].numStages = 0;        // number of stages
            IIR_PreFilter[chan].pkCoeffs = NULL; // point to reflection coefficients
            IIR_PreFilter[chan].pvCoeffs = NULL; // point to ladder coefficients
        }

        // Initialize IIR filter state buffer
        arm_fill_f32(0.0,iir_rx_state[chan],IIR_RX_STATE_ARRAY_SIZE);

        IIR_PreFilter[chan].pState = iir_rx_state[chan];					// point to state array for IIR filter

        // Initialize IIR antialias filter state buffer
        if (FilterPathInfo[ts.filter_path].iir_instance != NULL)
        {
            // if we turn on a filter, set the number of members to the number of elements last
            IIR_AntiAlias[chan].pkCoeffs = FilterPathInfo[ts.filter_path].iir_instance->pkCoeffs; // point to reflection coefficients
            IIR_AntiAlias[chan].pvCoeffs = FilterPathInfo[ts.filter_path].iir_instance->pvCoeffs; // point to ladder coefficients
            IIR_AntiAlias[chan].numStages = FilterPathInfo[ts.filter_path].iir_instance->numStages;        // number of stages
        }
        else
        {
            // if we turn off a filter, set the number of members to 0 first
            IIR_AntiAlias[chan].numStages = 0;
            IIR_AntiAlias[chan].pkCoeffs = NULL;
            IIR_AntiAlias[chan].pvCoeffs = NULL;
        }

        arm_fill_f32(0.0,iir_aa_state[chan],IIR_RX_STATE_ARRAY_SIZE);
        IIR_AntiAlias[chan].pState = iir_aa_state[chan];					// point to state array for IIR filter
    }

    // TODO: We only have to do this, if the audio signal filter configuration changes
    // RX+ TX Bass, Treble, Peak, Notch
    AudioDriver_SetRxTxAudioProcessingAudioFilters();

    // initialize the goertzel filter used to detect CW signals at a given frequency in the audio stream
    CwDecode_FilterInit();

    // this sets the coefficients for the ZoomFFT decimation filter
    // according to the desired magnification mode sd.magnify
    // sd.magnify 0 = 1x magnification
    // sd.magnify 1 = 2x
    // sd.magnify 2 = 4x
    // sd.magnify 3 = 8x
    // sd.magnify 4 = 16x
    // sd.magnify 5 = 32x
    //
    // for 0 the mag_coeffs will a NULL  ptr, since the filter is not going to be used in this  mode!
    IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[sd.magnify];
    IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[sd.magnify];

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * End of coefficient calculation and setting for cascaded biquad
     ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    //
    // Initialize high-pass filter used for the FM noise squelch
    //
    IIR_Squelch_HPF.pkCoeffs = IIR_15k_hpf.pkCoeffs;   // point to reflection coefficients
    IIR_Squelch_HPF.pvCoeffs = IIR_15k_hpf.pvCoeffs;   // point to ladder coefficients
    IIR_Squelch_HPF.numStages = IIR_15k_hpf.numStages;      // number of stages
    IIR_Squelch_HPF.pState = iir_squelch_rx_state;                  // point to state array for IIR filter
    arm_fill_f32(0.0,iir_squelch_rx_state,IIR_RX_STATE_ARRAY_SIZE);

//    ts.nr_long_tone_reset = true; // reset information about existing notches in filter passband

#ifdef OBSOLETE_NR
    // Initialize LMS (DSP Noise reduction) filter
    // It is (sort of) initalized "twice" since this it what it seems to take for the LMS function to
    // start reliably and consistently!
    //
    calc_taps = ts.dsp_nr_numtaps;
    if((calc_taps < DSP_NR_NUMTAPS_MIN) || (calc_taps > DSP_NR_NUMTAPS_MAX))
    {
        calc_taps = DSP_NR_NUMTAPS_DEFAULT;
    }

    // Load settings into instance structure
    //
    // LMS instance 1 is pre-AGC DSP NR
    // LMS instance 3 is post-AGC DSP NR
    //
    lmsData.lms1Norm_instance.numTaps = calc_taps;
    lmsData.lms1Norm_instance.pCoeffs = lmsData.lms1NormCoeff_f32;
    lmsData.lms1Norm_instance.pState = lmsData.lms1StateF32;

    // Calculate "mu" (convergence rate) from user "DSP Strength" setting.  This needs to be significantly de-linearized to
    // squeeze a wide range of adjustment (e.g. several magnitudes) into a fairly small numerical range.
    mu_calc = ts.dsp_nr_strength;		// get user setting
    /*
    mu_calc = DSP_NR_STRENGTH_MAX-mu_calc;		// invert (0 = minimum))
    mu_calc /= 2.6;								// scale calculation
    mu_calc *= mu_calc;							// square value
    mu_calc += 1;								// offset by one
    mu_calc /= 40;								// rescale
    mu_calc += 1;								// prevent negative log result
    mu_calc = log10f(mu_calc);					// de-linearize
    lms1Norm_instance.mu = mu_calc;				//
     */

    // New DSP NR "mu" calculation method as of 0.0.214
    mu_calc /= 2;	// scale input value
    mu_calc += 2;	// offset zero value
    mu_calc /= 10;	// convert from "bels" to "deci-bels"
    mu_calc = powf(10,mu_calc);		// convert to ratio
    mu_calc = 1/mu_calc;			// invert to fraction
    lmsData.lms1Norm_instance.mu = mu_calc;

    arm_fill_f32(0.0,lmsData.lms1_nr_delay,LMS_NR_DELAYBUF_SIZE_MAX + BUFF_LEN);
    arm_fill_f32(0.0,lmsData.lms1StateF32,DSP_NR_NUMTAPS_MAX + BUFF_LEN);

    if(reset_dsp_nr)	 			// are we to reset the coefficient buffer as well?
    {
        arm_fill_f32(0.0,lmsData.lms1NormCoeff_f32,DSP_NR_NUMTAPS_MAX + BUFF_LEN);		// yes - zero coefficient buffers
    }

    // use "canned" init to initialize the filter coefficients
    arm_lms_norm_init_f32(&lmsData.lms1Norm_instance, calc_taps, &lmsData.lms1NormCoeff_f32[0], &lmsData.lms1StateF32[0], mu_calc, 64);

    if((ts.dsp_nr_delaybuf_len > DSP_NR_BUFLEN_MAX) || (ts.dsp_nr_delaybuf_len < DSP_NR_BUFLEN_MIN))
    {
        ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
    }

    // AUTO NOTCH INIT START
    // LMS instance 2 - Automatic Notch Filter

    calc_taps = ts.dsp_notch_numtaps;
    lmsData.lms2Norm_instance.numTaps = calc_taps;
    lmsData.lms2Norm_instance.pCoeffs = lmsData.lms2NormCoeff_f32;
    lmsData.lms2Norm_instance.pState = lmsData.lms2StateF32;

    // Calculate "mu" (convergence rate) from user "Notch ConvRate" setting
    mu_calc = ts.dsp_notch_mu;		// get user setting (0 = slowest)
    mu_calc += 1;
    mu_calc /= 1500;
    mu_calc += 1;
    mu_calc = log10f(mu_calc);

    // use "canned" init to initialize the filter coefficients
    arm_lms_norm_init_f32(&lmsData.lms2Norm_instance, calc_taps, &lmsData.lms2NormCoeff_f32[0], &lmsData.lms2StateF32[0], mu_calc, 64);

    arm_fill_f32(0.0,lmsData.lms2_nr_delay,DSP_NOTCH_BUFLEN_MAX);
    arm_fill_f32(0.0,lmsData.lms2StateF32,LMS2_NOTCH_STATE_ARRAY_SIZE);

    if(reset_dsp_nr)             // are we to reset the coefficient buffer as well?
    {
        arm_fill_f32(0.0,lmsData.lms2NormCoeff_f32,DSP_NOTCH_NUMTAPS_MAX);      // yes - zero coefficient buffers
    }

    if((ts.dsp_notch_delaybuf_len > DSP_NOTCH_BUFLEN_MAX) || (ts.dsp_notch_delaybuf_len < DSP_NOTCH_BUFLEN_MIN))
    {
        ts.dsp_nr_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
    }
    // AUTO NOTCH INIT END
#endif

#ifdef USE_LMS_AUTONOTCH
    // AUTO NOTCH INIT START
    // LMS instance 2 - Automatic Notch Filter

    // Calculate "mu" (convergence rate) from user "Notch ConvRate" setting
    float32_t  mu_calc = log10f(((ts.dsp_notch_mu + 1.0)/1500.0) + 1.0);		// get user setting (0 = slowest)

    // use "canned" init to initialize the filter coefficients
    arm_lms_norm_init_f32(&lmsData.lms2Norm_instance, ts.dsp_notch_numtaps, lmsData.lms2NormCoeff_f32, lmsData.lms2StateF32, mu_calc, IQ_BLOCK_SIZE);

    arm_fill_f32(0.0,lmsData.lms2_nr_delay,DSP_NOTCH_BUFLEN_MAX);

    if(reset_dsp_nr)             // are we to reset the coefficient buffer as well?
    {
        arm_fill_f32(0.0,lmsData.lms2NormCoeff_f32,DSP_NOTCH_NUMTAPS_MAX);      // yes - zero coefficient buffers
    }

    if((ts.dsp_notch_delaybuf_len > DSP_NOTCH_BUFLEN_MAX) || (ts.dsp_notch_delaybuf_len < DSP_NOTCH_BUFLEN_MIN))
    {
        ts.dsp_nr_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
    }
    // AUTO NOTCH INIT END
#endif


// NEW SPECTRAL NOISE REDUCTION
    // convert user setting of noise reduction to alpha NR parameter
    // alpha ranges from 0.9 to 0.999 [float32_t]
    // dsp_nr_strength is from 0 to 100 [uint8_t]
    //    ts.nr_alpha = 0.899 + ((float32_t)ts.dsp_nr_strength / 1000.0);
    ts.nr_alpha = 0.799 + ((float32_t)ts.dsp_nr_strength / 1000.0);

// NEW AUTONOTCH
    // set to passthrough
    //AudioNr_ActivateAutoNotch(0, 0);

    // Adjust decimation rate based on selected filter
    ads.decimation_rate = FilterPathInfo[ts.filter_path].sample_rate_dec;

//    ads.agc_decimation_scaling = ads.decimation_rate;
//    ads.agc_delay_buflen = AUDIO_DELAY_BUFSIZE/(ulong)ads.decimation_rate;	// calculate post-AGC delay based on post-decimation sampling rate

    // Set up ZOOM FFT FIR decimation filters
    // switch right FIR decimation filter depending on sd.magnify
    if(sd.magnify > MAGNIFY_MAX)
    {
        sd.magnify = MAGNIFY_MIN;
    }

    arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_I,
            FirZoomFFTDecimate[sd.magnify].numTaps,
            (1 << sd.magnify),          // Decimation factor
            FirZoomFFTDecimate[sd.magnify].pCoeffs,
            decimZoomFFTIState,            // Filter state variables
            FIR_RXAUDIO_BLOCK_SIZE);

    arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_Q,
            FirZoomFFTDecimate[sd.magnify].numTaps,
            (1 << sd.magnify),          // Decimation factor
            FirZoomFFTDecimate[sd.magnify].pCoeffs,
            decimZoomFFTQState,            // Filter state variables
            FIR_RXAUDIO_BLOCK_SIZE);

    // Set up RX decimation/filter
    if (FilterPathInfo[ts.filter_path].dec != NULL)
    {
        const arm_fir_decimate_instance_f32* dec = FilterPathInfo[ts.filter_path].dec;

#if defined(STM32F4) && defined(USE_LMS_AUTONOTCH)
        // FIXME: Better solution (e.g. improve graphics performance, better data structures ... )
        // this code is a hack to reduce processor load for STM32F4 && SPI display
        // which causes UI lag
        // in this case we simply use a less power-eating filter (lower number of taps)
        // one problem is that we use the not so good filter
        // even if the autonotch / nr is not active and we could use the good filter
        if (dec == &FirRxDecimate_sideband_supp && ts.display->use_spi == true)
        {
            dec = &FirRxDecimate;
        }
#endif

        arm_fir_decimate_init_f32(&DECIMATE_RX_I,

                dec->numTaps,      // Number of taps in FIR filter
                ads.decimation_rate,
                dec->pCoeffs,       // Filter coefficients
                decimState_I,            // Filter state variables
                FIR_RXAUDIO_BLOCK_SIZE);

        arm_fir_decimate_init_f32(&DECIMATE_RX_Q,
                dec->numTaps,      // Number of taps in FIR filter
                ads.decimation_rate,
                dec->pCoeffs,       // Filter coefficients
                decimState_Q,            // Filter state variables
                FIR_RXAUDIO_BLOCK_SIZE);
    }
    else
    {
        DECIMATE_RX_I.numTaps = 0;
        DECIMATE_RX_I.pCoeffs = NULL;
        DECIMATE_RX_Q.numTaps = 0;
        DECIMATE_RX_Q.pCoeffs = NULL;
    }

    // Set up RX interpolation/filter
    // NOTE:  Phase Length MUST be an INTEGER and is the number of taps divided by the decimation rate, and it must be greater than 1.
    for (int chan = 0; chan < NUM_AUDIO_CHANNELS; chan++)
    {
        if (FilterPathInfo[ts.filter_path].interpolate != NULL)
        {
            arm_fir_interpolate_init_f32(&INTERPOLATE_RX[chan],
                    ads.decimation_rate,
                    FilterPathInfo[ts.filter_path].interpolate->phaseLength,
                    FilterPathInfo[ts.filter_path].interpolate->pCoeffs,
                    interpState[chan], IQ_BLOCK_SIZE);
        }
        else
        {
            INTERPOLATE_RX[chan].phaseLength = 0;
            INTERPOLATE_RX[chan].pCoeffs = NULL;
        }
    }

    arm_fir_decimate_init_f32(&DECIMATE_NR, 4, 2, NR_decimate_coeffs, decimNRState, FIR_RXAUDIO_BLOCK_SIZE);
    // should be a very light lowpass @2k7

    arm_fir_interpolate_init_f32(&INTERPOLATE_NR, 2, NR_INTERPOLATE_NO_TAPS, NR_interpolate_coeffs, interplNRState, FIR_RXAUDIO_BLOCK_SIZE);
    // should be a very light lowpass @2k7

    ads.dsp_zero_count = 0;		// initialize "zero" count to detect if DSP has crashed

    // if (dmod_mode == DEMOD_AM || dmod_mode == DEMOD_SAM)
    {
        AudioDriver_SetRxAudioProcessingSAM(dmod_mode);
    }

    AudioFilter_InitRxHilbertFIR(dmod_mode); // this switches the Hilbert/FIR-filters

    AudioDriver_SetupAgcWdsp();

    // Unlock - re-enable filtering
    if  (ads.af_disabled) { ads.af_disabled--; }
    if (ts.dsp_inhibit) { ts.dsp_inhibit--; }

}

void AudioDriver_TxFilterInit(uint8_t dmod_mode)
{
    // Init TX audio filter - Do so "manually" since built-in init functions don't work with CONST coefficients

    const arm_iir_lattice_instance_f32* IIR_TXFilterSelected_ptr;

    if(dmod_mode != DEMOD_FM)	 						// not FM - use bandpass filter that restricts low and, stops at 2.7 kHz
    {
        switch(ts.tx_filter)
        {
        case TX_FILTER_BASS:
            IIR_TXFilterSelected_ptr = &IIR_TX_WIDE_BASS;
            break;
        case TX_FILTER_TENOR:
            IIR_TXFilterSelected_ptr = &IIR_TX_WIDE_TREBLE;
            break;
        default:
            IIR_TXFilterSelected_ptr = &IIR_TX_SOPRANO;
        }
    }
    else	 	// This is FM - use a filter with "better" lows and highs more appropriate for FM
    {
        IIR_TXFilterSelected_ptr = &IIR_TX_2k7_FM;
    }

    arm_iir_lattice_init_f32(&IIR_TXFilter,
            IIR_TXFilterSelected_ptr->numStages,       // number of stages
            IIR_TXFilterSelected_ptr->pkCoeffs, // point to reflection coefficients
            IIR_TXFilterSelected_ptr->pvCoeffs, // point to ladder coefficients
            IIR_TXFilter.pState = iir_tx_state,
            IIR_RXAUDIO_BLOCK_SIZE);
}

//*----------------------------------------------------------------------------
//* Function Name       : Audio_Init
//* Object              :
//* Object              : init filters
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_InitFilters(void)
{
    AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);

    AudioDriver_TxFilterInit(ts.dmod_mode);

    IIR_biquad_FreeDV_I.pCoeffs = FreeDV_coeffs[0];  // FreeDV Filter test -DL2FW-
    IIR_biquad_FreeDV_Q.pCoeffs = FreeDV_coeffs[0];

    // temporary installed audio filter's coefficients
    IIR_FreeDV_RX_Filter.numStages = IIR_TX_WIDE_TREBLE.numStages; // using the same for FreeDV RX Audio as for TX
    IIR_FreeDV_RX_Filter.pkCoeffs = IIR_TX_WIDE_TREBLE.pkCoeffs;   // but keeping it constant at "Tenor" to avoid
    IIR_FreeDV_RX_Filter.pvCoeffs = IIR_TX_WIDE_TREBLE.pvCoeffs;   // influence of TX setting in RX path
    IIR_FreeDV_RX_Filter.pState = iir_tx_state;
    arm_fill_f32(0.0,iir_FreeDV_RX_state,IIR_RX_STATE_ARRAY_SIZE);

}

#if 0
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_noise_blanker [KA7OEI]
//* Object              : noise blanker
//* Object              :
//* Input Parameters    : I/Q 16 bit audio data, size of buffer
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
#define AUDIO_RX_NB_DELAY_BUFFER_ITEMS (16)
#define AUDIO_RX_NB_DELAY_BUFFER_SIZE (AUDIO_RX_NB_DELAY_BUFFER_ITEMS*2)

static void AudioDriver_NoiseBlanker(AudioSample_t * const src, int16_t blockSize)
{
    static int16_t	delay_buf[AUDIO_RX_NB_DELAY_BUFFER_SIZE];
    static uchar	delbuf_inptr = 0, delbuf_outptr = 2;
    ulong	i;
    float	sig;
    float  nb_short_setting;
    //	static float avg_sig;
    static	uchar	nb_delay = 0;
    static float	nb_agc = 0;

    if((ts.nb_setting > 0) &&  (ts.dsp_active & DSP_NB_ENABLE)
            //            && (ts.dmod_mode != DEMOD_AM && ts.dmod_mode != DEMOD_FM)
            && (ts.dmod_mode != DEMOD_FM))
        //        && (FilterPathInfo[ts.filter_path].sample_rate_dec != RX_DECIMATION_RATE_24KHZ ))

        // bail out if noise blanker disabled, in AM/FM mode, or set to 10 kHz
    {
        nb_short_setting = ts.nb_setting;		// convert and rescale NB1 setting for higher resolution in adjustment
        nb_short_setting /= 2;

        for(i = 0; i < blockSize; i++)	 		// Noise blanker function
        {
            sig = fabs(src[i].l);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
            sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
            //		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
            delay_buf[delbuf_inptr++] = src[i].l;	    // copy first byte into delay buffer
            delay_buf[delbuf_inptr++] = src[i].r;	// copy second byte into delay buffer

            nb_agc = (ads.nb_agc_filt * nb_agc) + (ads.nb_sig_filt * sig);		// IIR-filtered "AGC" of current overall signal level

            if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	 	// did a pulse exceed the threshold?
            {
                nb_delay = AUDIO_RX_NB_DELAY_BUFFER_ITEMS;		// yes - set the blanking duration counter
            }

            if(!nb_delay)	 		// blank counter not active
            {
                src[i].l = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
                src[i].r = delay_buf[delbuf_outptr++];
            }
            else	 	// It is within the blanking pulse period
            {
                src[i].l = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
                src[i].r = 0; //(int16_t)avg_sig;
                nb_delay--;						// count down the number of samples that we are to blank
            }

            // RINGBUFFER
            delbuf_outptr %= AUDIO_RX_NB_DELAY_BUFFER_SIZE;
            delbuf_inptr %= AUDIO_RX_NB_DELAY_BUFFER_SIZE;
        }
    }
}
#endif

//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_freq_conv [KA7OEI]
//* Object              : Does I/Q frequency conversion
//* Object              :
//* Input Parameters    : size of array on which to work; dir: determines direction of shift - see below;  Also uses variables in ads structure
//* Output Parameters   : uses variables in ads structure
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_FreqConversion(float32_t* i_buffer, float32_t* q_buffer, int16_t blockSize, int16_t dir)
{
    static bool recalculate_Osc = false;
    // keeps the generated data for frequency conversion
    static float32_t                   Osc_I_buffer[IQ_BLOCK_SIZE];
    static float32_t                   Osc_Q_buffer[IQ_BLOCK_SIZE];

    assert(blockSize <= IQ_BLOCK_SIZE);


    //
    // Below is the "on-the-fly" version of the frequency translator, generating a "live" version of the oscillator (NCO), which can be any
    // frequency, based on the values of "ads.Osc_Cos" and "ads.Osc_Sin".  While this does function, the generation of the SINE takes a LOT
    // of processor time!
    //
    // The values for the NCO are configured in the function "audio_driver_config_nco()".
    //
    // This commented-out version also lacks the "dir" (direction) control which selects either high or low side translation.
    // This code is left here so that everyone can see how it is actually done in plain, "un-ARM" code.
    //
    /*
    	for(i = 0; i < blockSize; i++)	{
    		// generate local oscillator on-the-fly:  This takes a lot of processor time!
    		ads.Osc_Q = (ads.Osc_Vect_Q * ads.Osc_Cos) - (ads.Osc_Vect_I * ads.Osc_Sin);	// Q channel of oscillator
    		ads.Osc_I = (ads.Osc_Vect_I * ads.Osc_Cos) + (ads.Osc_Vect_Q * ads.Osc_Sin);	// I channel of oscillator
    		ads.Osc_Gain = 1.95 - ((ads.Osc_Vect_Q * ads.Osc_Vect_Q) + (ads.Osc_Vect_I * ads.Osc_Vect_I));	// Amplitude control of oscillator
    		// rotate vectors while maintaining constant oscillator amplitude
    		ads.Osc_Vect_Q = ads.Osc_Gain * ads.Osc_Q;
    		ads.Osc_Vect_I = ads.Osc_Gain * ads.Osc_I;
    		//
    		// do actual frequency conversion
    		i_temp = i_buffer[i];	// save temporary copies of data
    		q_temp = q_buffer[i];
    		i_buffer[i] = (i_temp * ads.Osc_Q) + (q_temp * ads.Osc_I);	// multiply I/Q data by sine/cosine data to do translation
    		q_buffer[i] = (q_temp * ads.Osc_Q) - (i_temp * ads.Osc_I);
    		//
    	}
     */
    // [KA7OEI]
    // Below is the frequency translation code that uses a "pre-calculated" sine wave - which means that the translation must be done at a sub-
    // multiple of the sample frequency.  This pre-calculation eliminates the processor overhead required to generate a sine wave on the fly.
    // This also makes extensive use of the optimized ARM vector instructions for the calculation of the final I/Q vectors
    //
    // Pre-calculate quadrature sine wave(s) ONCE for the conversion
    //
    switch(ts.iq_freq_mode)
    {
    case FREQ_IQ_CONV_P6KHZ:
    case FREQ_IQ_CONV_M6KHZ:
        if (ts.multi != 4)
        {
            ts.multi = 4; 		//(4 = 6 kHz offset)
            recalculate_Osc = true;
        }
        break;
    case FREQ_IQ_CONV_P12KHZ:
    case FREQ_IQ_CONV_M12KHZ:
        if (ts.multi != 8)
        {
            ts.multi = 8; 		// (8 = 12 kHz offset)
            recalculate_Osc = true;
        }
    }

    if(recalculate_Osc == true)	 		// have we already calculated the sine wave?
    {
        float32_t multiplier = (ts.multi * PI * 2) / ((float32_t)blockSize);

        for(int i = 0; i < blockSize; i++)	 		// No, let's do it!
        {
            float32_t rad_calc = (float32_t)i * multiplier;
            sincosf(rad_calc, &Osc_I_buffer[i], &Osc_Q_buffer[i]);
        }
        recalculate_Osc = false;	// signal that once we have generated the quadrature sine waves, we shall not do it again
    }


    if(ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ)
    {
        /**********************************************************************************
         *  Frequency translation by Fs/4 without multiplication
         *  Lyons (2011): chapter 13.1.2 page 646
         *  this is supposed to be much more efficient than a standard quadrature oscillator
         *  with precalculated sin waves
         *  Thanks, Clint, for pointing my interest to this method!, DD4WH 2016_12_28
         **********************************************************************************/
        if(dir)
        {
            // this is for +Fs/4 [moves receive frequency to the left in the spectrum display]
            for(int i = 0; i < blockSize; i += 4)
            {   // xnew(0) =  xreal(0) + jximag(0)
                // leave as it is!
                // xnew(1) =  - ximag(1) + jxreal(1)
                float32_t hh1 = - q_buffer[i + 1];
                float32_t hh2 =   i_buffer[i + 1];
                i_buffer[i + 1] = hh1;
                q_buffer[i + 1] = hh2;
                // xnew(2) = -xreal(2) - jximag(2)
                hh1 = - i_buffer[i + 2];
                hh2 = - q_buffer[i + 2];
                i_buffer[i + 2] = hh1;
                q_buffer[i + 2] = hh2;
                // xnew(3) = + ximag(3) - jxreal(3)
                hh1 =   q_buffer[i + 3];
                hh2 = - i_buffer[i + 3];
                i_buffer[i + 3] = hh1;
                q_buffer[i + 3] = hh2;
            }
        }

        else // dir == 0
        {
            // this is for -Fs/4 [moves receive frequency to the right in the spectrum display]
            for(int i = 0; i < blockSize; i += 4)
            {   // xnew(0) =  xreal(0) + jximag(0)
                // leave as it is!
                // xnew(1) =  ximag(1) - jxreal(1)
                float32_t hh1 = q_buffer[i + 1];
                float32_t hh2 = - i_buffer[i + 1];
                i_buffer[i + 1] = hh1;
                q_buffer[i + 1] = hh2;
                // xnew(2) = -xreal(2) - jximag(2)
                hh1 = - i_buffer[i + 2];
                hh2 = - q_buffer[i + 2];
                i_buffer[i + 2] = hh1;
                q_buffer[i + 2] = hh2;
                // xnew(3) = -ximag(3) + jxreal(3)
                hh1 = - q_buffer[i + 3];
                hh2 = i_buffer[i + 3];
                i_buffer[i + 3] = hh1;
                q_buffer[i + 3] = hh2;
            }
        }
    }
    else  // frequency translation +6kHz or -6kHz
    {
        float32_t c_buffer[blockSize];
        float32_t d_buffer[blockSize];
        float32_t e_buffer[blockSize];
        float32_t f_buffer[blockSize];

        // Do frequency conversion using optimized ARM math functions [KA7OEI]
        arm_mult_f32(q_buffer, Osc_Q_buffer, c_buffer, blockSize); // multiply products for converted I channel
        arm_mult_f32(i_buffer, Osc_I_buffer, d_buffer, blockSize);
        arm_mult_f32(q_buffer, Osc_I_buffer, e_buffer, blockSize);
        arm_mult_f32(i_buffer, Osc_Q_buffer, f_buffer, blockSize);    // multiply products for converted Q channel

        if(!dir)	 	// Conversion is "above" on RX (LO needs to be set lower)
        {
            arm_add_f32(f_buffer, e_buffer, i_buffer, blockSize);	// summation for I channel
            arm_sub_f32(c_buffer, d_buffer, q_buffer, blockSize);	// difference for Q channel
        }
        else	 	// Conversion is "below" on RX (LO needs to be set higher)
        {
            arm_add_f32(c_buffer, d_buffer, q_buffer, blockSize);	// summation for I channel
            arm_sub_f32(f_buffer, e_buffer, i_buffer, blockSize);	// difference for Q channel
        }
    }
}

#ifdef USE_FREEDV
static bool AudioDriver_RxProcessorFreeDV (AudioSample_t * const src, float32_t * const dst, int16_t blockSize)
{
    // Freedv Test DL2FW
    static int16_t outbuff_count = -3;  //set to -3 since we simulate that we start with history
    static int16_t trans_count_in = 0;
    static int16_t FDV_TX_fill_in_pt = 0;
    static FDV_Audio_Buffer* out_buffer = NULL;
    static int16_t modulus_NF = 0, mod_count=0;
    bool lsb_active = (ts.dmod_mode == DEMOD_LSB || (ts.dmod_mode == DEMOD_DIGI && ts.digi_lsb == true));


    static float32_t History[3]={0.0,0.0,0.0};

    // If source is digital usb in, pull from USB buffer, discard line or mic audio and
    // let the normal processing happen


    if (ts.digital_mode == DigitalMode_FreeDV)
    { //we are in freedv-mode


        // *****************************   DV Modulator goes here - ads.a_buffer must be at 8 ksps

        // Freedv Test DL2FW

        // we have to add a decimation filter here BEFORE we decimate
        // for decimation-by-6 the stopband frequency is 48/6*2 = 4kHz
        // but our audio is at most 3kHz wide, so we should use 3k or 2k9


        // this is the correct DECIMATION FILTER (before the downsampling takes place):
        // use it ALWAYS, also with TUNE tone!!!


        if (ts.filter_path != 65)
        {   // just for testing, when Filter #65 (10kHz LPF) is selected, antialiasing is switched off
            arm_biquad_cascade_df1_f32 (&IIR_biquad_FreeDV_I, adb.i_buffer,adb.i_buffer, blockSize);
            arm_biquad_cascade_df1_f32 (&IIR_biquad_FreeDV_Q, adb.q_buffer,adb.q_buffer, blockSize);
        }

        // DOWNSAMPLING
        for (int k = 0; k < blockSize; k++)
        {
            if (k % 6 == modulus_NF)  //every 6th sample has to be catched -> downsampling by 6
            {

                if (lsb_active == true)
                {
                    mmb.fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].real = ((int32_t)adb.q_buffer[k]);
                    mmb.fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].imag = ((int32_t)adb.i_buffer[k]);
                }
                else
                {
                    mmb.fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].imag = ((int32_t)adb.q_buffer[k]);
                    mmb.fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].real = ((int32_t)adb.i_buffer[k]);
                }

                trans_count_in++;
            }
        }

        modulus_NF += 4; //  shift modulus to not loose any data while overlapping
        modulus_NF %= 6;//  reset modulus to 0 at modulus = 12

        if (trans_count_in == FDV_BUFFER_SIZE) //yes, we really hit exactly 320 - don't worry
        {
            //we have enough samples ready to start the FreeDV encoding

            fdv_iq_buffer_add(&mmb.fdv_iq_buff[FDV_TX_fill_in_pt]);
            //handshake to external function in ui.driver_thread
            trans_count_in = 0;

            FDV_TX_fill_in_pt++;
            FDV_TX_fill_in_pt %= FDV_BUFFER_IQ_NUM;
        }

        // if we run out  of buffers lately
        // we wait for availability of at least 2 buffers
        // so that in theory we have uninterrupt flow of audio
        // albeit with a delay of 80ms
        if (out_buffer == NULL && fdv_audio_has_data() > 1)
        {
            fdv_audio_buffer_peek(&out_buffer);
        }

        if (out_buffer != NULL) // freeDV encode has finished (running in ui_driver.c)?
        {
            // Best thing here would be to use the arm_fir_decimate function! Why?
            // --> we need phase linear filters, because we have to filter I & Q and preserve their phase relationship
            // IIR filters are power saving, but they do not care about phase, so useless at this point
            // FIR filters are phase linear, but need processor power
            // so we now use the decimation function that upsamples like the code below, BUT at the same time filters
            // (and the routine knows that it does not have to multiply with 0 while filtering: if we do upsampling and subsequent
            // filtering, the filter does not know that and multiplies with zero 5 out of six times --> very inefficient)
            // BUT: we cannot use the ARM function, because decimation factor (6) has to be an integer divide of
            // block size (which is 64 in our case --> 64 / 6 = non-integer!)


#if 0	    // activate IIR FIlter

            arm_fill_f32(0,dst,blockSize);

            // UPSAMPLING [by hand]
            for (int j = 0; j < blockSize; j++) //  now we are doing upsampling by 6
            {
                if (modulus_MOD == 0) // put in sample pair
                {
                    dst[j] = out_buffer->samples[outbuff_count]; // + (sample_delta.real * (float32_t)modulus_MOD);
                    //sample_delta = (out_buffer->samples[outbuff_count]-last_sample)/6;
                }

                //adb.a_buffer[1][j] = out_buffer->samples[outbuff_count] + sample_delta*(float32_t)modulus_MOD;


#if 0
                else // in 5 of 6 cases just stuff in zeros = zero-padding / zero-stuffing
                    // or keep the last sample value - like a sample an' old
                {
                    //adb.a_buffer[1][j] = 0;
                    dst[j] = out_buffer->samples[outbuff_count];
                }
#endif

                modulus_MOD++;
                if (modulus_MOD == 6)
                {
                    // last_sample = out_buffer->samples[outbuff_count];
                    outbuff_count++;
                    modulus_MOD = 0;
                }
            }

            // Add interpolation filter here to suppress alias frequencies
            // we are upsampling from 8kHz to 48kHz, so we have to suppress all frequencies below 4kHz
            // our FreeDV signal is here already an reconstructed Audio Signal,
            // so a lowpass or bandpass filter with cutoff frequency 50/2800Hz should be fine!

            // This is a temporary installed - neutral - RX Audio Filter which will be replaced by a better
            // hopefully a polyphase interpolation filter - has to be adopted to our upsampling rate and buffersizes.....

            // Filter below uses the TX-"Tenor" filter shape for the RX upsampling filter
            arm_iir_lattice_f32(&IIR_FreeDV_RX_Filter, dst, dst, blockSize);

            //AudioDriver_tx_filter_audio(true,false, dst, dst, blockSize);


        }
        else
        {
            profileEvent(FreeDVTXUnderrun);
            // in case of underrun -> produce silence
            arm_fill_f32(0, dst, blockSize);
        }

        if (outbuff_count >= FDV_BUFFER_SIZE)
        {
            outbuff_count = 0;
            fdv_audio_buffer_remove(&out_buffer);
            // ok, this one is done with
            out_buffer = NULL;
            fdv_audio_buffer_peek(&out_buffer);
            // we may or may not get a buffer here
            // if not and we have a stall the code somewhere up
            // produces silence until 2 out buffers are available

        }

#else
        for (int j=0; j < blockSize; j++) //upsampling with integrated interpolation-filter for M=6
            // avoiding multiplications by zero within the arm_iir_filter
        {
            if (outbuff_count >=0)  // here we are not at an block-overlapping region
            {
                dst[j]=
                        Fir_Rx_FreeDV_Interpolate_Coeffs[5-mod_count]*out_buffer->samples[outbuff_count] +
                        Fir_Rx_FreeDV_Interpolate_Coeffs[11-mod_count]*out_buffer->samples[outbuff_count+1]+
                        Fir_Rx_FreeDV_Interpolate_Coeffs[17-mod_count]*out_buffer->samples[outbuff_count+2]+
                        Fir_Rx_FreeDV_Interpolate_Coeffs[23-mod_count]*out_buffer->samples[outbuff_count+3];
                // here we are actually calculation the interpolation for the current "up"-sample
            }
            else
            {
                //we are at an overlapping region and have to take care of history
                if (outbuff_count == -3)
                {
                    dst[j] =
                            Fir_Rx_FreeDV_Interpolate_Coeffs[5-mod_count] * History[0] +
                            Fir_Rx_FreeDV_Interpolate_Coeffs[11-mod_count] * History[1] +
                            Fir_Rx_FreeDV_Interpolate_Coeffs[17-mod_count] * History[2] +
                            Fir_Rx_FreeDV_Interpolate_Coeffs[23-mod_count] * out_buffer->samples[0];
                }
                else
                {
                    if (outbuff_count == -2)
                    {
                        dst[j] =
                                Fir_Rx_FreeDV_Interpolate_Coeffs[5-mod_count] * History[1] +
                                Fir_Rx_FreeDV_Interpolate_Coeffs[11-mod_count] * History[2] +
                                Fir_Rx_FreeDV_Interpolate_Coeffs[17-mod_count] * out_buffer->samples[0] +
                                Fir_Rx_FreeDV_Interpolate_Coeffs[23-mod_count] * out_buffer->samples[1];
                    }
                    else
                    {
                        dst[j] =
                                Fir_Rx_FreeDV_Interpolate_Coeffs[5-mod_count] * History[2] +
                                Fir_Rx_FreeDV_Interpolate_Coeffs[11-mod_count] * out_buffer->samples[0] +
                                Fir_Rx_FreeDV_Interpolate_Coeffs[17-mod_count] * out_buffer->samples[1] +
                                Fir_Rx_FreeDV_Interpolate_Coeffs[23-mod_count] * out_buffer->samples[2];
                    }
                }
            }


            mod_count++;
            if (mod_count==6)
            {
                outbuff_count++;
                mod_count=0;
            }
        }
    }
    else
    {
        profileEvent(FreeDVTXUnderrun);
        // in case of underrun -> produce silence
        arm_fill_f32(0, dst, blockSize);
    }

    // we used now FDV_BUFFER_SIZE samples (3 from History[], plus FDV_BUFFER_SIZE -3 from out_buffer->samples[])
    if (outbuff_count == (FDV_BUFFER_SIZE-3))//  -3???
    {
        outbuff_count=-3;

        History[0] = out_buffer->samples[FDV_BUFFER_SIZE-3]; // here we have to save historic samples
        History[1] = out_buffer->samples[FDV_BUFFER_SIZE-2]; // to calculate the interpolation in the
        History[2] = out_buffer->samples[FDV_BUFFER_SIZE-1]; // block overlapping region

        // ok, let us free the old buffer
        fdv_audio_buffer_remove(&out_buffer);
        out_buffer = NULL;
        fdv_audio_buffer_peek(&out_buffer);

    }

#endif  //activate FIR Filter
}
return true;
}
#endif

// RTTY Experiment based on code from the DSP Tutorial at http://dp.nonoo.hu/projects/ham-dsp-tutorial/18-rtty-decoder-using-iir-filters/
// Used with permission from Norbert Varga, HA2NON under GPLv3 license
#ifdef USE_RTTY_PROCESSOR
static void AudioDriver_RxProcessor_Rtty(float32_t * const src, int16_t blockSize)
{

    for (uint16_t idx = 0; idx < blockSize; idx++)
    {
        RttyDecoder_ProcessSample(src[idx]);
    }
}
#endif

static void AudioDriver_RxProcessor_Bpsk(float32_t * const src, int16_t blockSize)
{

    for (uint16_t idx = 0; idx < blockSize; idx++)
    {
        BpskDecoder_ProcessSample(src[idx]);
    }
}

void AudioDriver_SetupAgcWdsp()
{
    static bool initialised = false;
	float32_t tmp;
    float32_t sample_rate = IQ_SAMPLE_RATE_F / (float32_t)ads.decimation_rate;
    static uchar decimation_rate_old = 0; // will be set to current decimation rate in first round

    // this is a quick and dirty hack
    // it initialises the AGC variables once again,
    // if the decimation rate is changed
    // this should prevent confusion between the distance of in_index and out_index variables
    // because these are freshly initialised
    // in_index and out_index have a distance of 48 (sample rate 12000) or 96 (sample rate 24000)
    // so that has to be defined very well when filter from 4k8 to 5k0 (changing decimation rate from 4 to 2)
    if(decimation_rate_old != ads.decimation_rate)
    {
    	initialised = false; // force initialisation
    	decimation_rate_old = ads.decimation_rate; // remember decimation rate for next time
    }
    // Start variables taken from wdsp
    // RXA.c !!!!
    /*
    0.001,                      // tau_attack
    0.250,                      // tau_decay
    4,                        // n_tau
    10000.0,                    // max_gain
    1.5,                      // var_gain
    1000.0,                     // fixed_gain
    1.0,                      // max_input
    1.0,                      // out_target
    0.250,                      // tau_fast_backaverage
    0.005,                      // tau_fast_decay
    5.0,                      // pop_ratio
    1,                        // hang_enable
    0.500,                      // tau_hang_backmult
    0.250,                      // hangtime
    0.250,                      // hang_thresh
    0.100);                     // tau_hang_decay
     */
    // one time initialization
    if(!initialised)
    {
    	agc_wdsp.ring_buffsize = AGC_WDSP_RB_SIZE; //192; //96;
		//do one-time initialization
    	agc_wdsp.out_index = -1; //agc_wdsp.ring_buffsize; // or -1 ??
    	agc_wdsp.fixed_gain = 1.0;
    	agc_wdsp.ring_max = 0.0;
    	agc_wdsp.volts = 0.0;
    	agc_wdsp.save_volts = 0.0;
		agc_wdsp.fast_backaverage = 0.0;
		agc_wdsp.hang_backaverage = 0.0;
		agc_wdsp.hang_counter = 0;
		agc_wdsp.decay_type = 0;
		agc_wdsp.state = 0;
	    agc_wdsp.tau_attack = 0.001;               // tau_attack
	    //    tau_decay = ts.agc_wdsp_tau_decay / 1000.0; // 0.250;                // tau_decay
	    agc_wdsp.n_tau = 4;                        // n_tau

	    //    max_gain = 1000.0; // 1000.0; determines the AGC threshold = knee level
	    //  max_gain is powf (10.0, (float32_t)ts.agc_wdsp_thresh / 20.0);
	    //    fixed_gain = ads.agc_rf_gain; //0.7; // if AGC == OFF, this gain is used
	    agc_wdsp.max_input = (float32_t)ADC_CLIP_WARN_THRESHOLD; // which is 4096 at the moment
	    //32767.0; // maximum value of 16-bit audio //  1.0; //
	    agc_wdsp.out_targ = (float32_t)ADC_CLIP_WARN_THRESHOLD; // 4096, tweaked, so that volume when switching between the two AGCs remains equal
	    //12000.0; // target value of audio after AGC
	    agc_wdsp.tau_fast_backaverage = 0.250;    // tau_fast_backaverage
	    agc_wdsp.tau_fast_decay = 0.005;          // tau_fast_decay
	    agc_wdsp.pop_ratio = 5.0;                 // pop_ratio
	    //    hang_enable = 0;                 // hang_enable
	    agc_wdsp.tau_hang_backmult = 0.500;       // tau_hang_backmult

	    initialised = true;
    }
    //    var_gain = 32.0;  // slope of the AGC --> this is 10 * 10^(slope / 20) --> for 10dB slope, this is 30.0
    agc_wdsp.var_gain = powf (10.0, (float32_t)ts.agc_wdsp_slope / 20.0 / 10.0); // 10^(slope / 200)

    //    hangtime = 0.250;                // hangtime
    agc_wdsp.hangtime = (float32_t)ts.agc_wdsp_hang_time / 1000.0;
    //    hang_thresh = 0.250;             // hang_thresh

    //    tau_hang_decay = 0.100;          // tau_hang_decay

    //calculate internal parameters
    if(ts.agc_wdsp_switch_mode)
    {
        switch (ts.agc_wdsp_mode)
        {
        case 5: //agcOFF
            break;
        case 1: //agcLONG
            agc_wdsp.hangtime = 2.000;
            //      ts.agc_wdsp_tau_decay = 2000;
            //      hang_thresh = 1.0;
            //      ts.agc_wdsp_hang_enable = 1;
            break;
        case 2: //agcSLOW
            agc_wdsp.hangtime = 1.000;
            //      hang_thresh = 1.0;
            //      ts.agc_wdsp_tau_decay = 500;
            //      ts.agc_wdsp_hang_enable = 1;
            break;
        case 3: //agcMED
            //      hang_thresh = 1.0;
            agc_wdsp.hangtime = 0.250;
            //      ts.agc_wdsp_tau_decay = 250;
            break;
        case 4: //agcFAST
            //      hang_thresh = 1.0;
            agc_wdsp.hangtime = 0.100;
            //      ts.agc_wdsp_tau_decay = 50;
            break;
        case 0: //agcFrank --> very long
            //      ts.agc_wdsp_hang_enable = 0;
            //      hang_thresh = 0.300; // from which level on should hang be enabled
            agc_wdsp.hangtime = 3.000; // hang time, if enabled
            agc_wdsp.tau_hang_backmult = 0.500; // time constant exponential averager
            //      ts.agc_wdsp_tau_decay = 4000; // time constant decay long
            agc_wdsp.tau_fast_decay = 0.05;          // tau_fast_decay
            agc_wdsp.tau_fast_backaverage = 0.250; // time constant exponential averager
            break;
        default:
            break;
        }
        ts.agc_wdsp_switch_mode = 0;
    }
    //  float32_t noise_offset = 10.0 * log10f(fhigh - rxa[channel].nbp0.p->flow)
    //          * size / rate);
    //  max_gain = out_target / var_gain * powf (10.0, (thresh + noise_offset) / 20.0));
    agc_wdsp.tau_hang_decay = (float32_t)ts.agc_wdsp_tau_hang_decay / 1000.0;
    agc_wdsp.tau_decay = (float32_t)ts.agc_wdsp_tau_decay[ts.agc_wdsp_mode] / 1000.0;
    agc_wdsp.max_gain = powf (10.0, (float32_t)ts.agc_wdsp_thresh / 20.0);
    agc_wdsp.fixed_gain = agc_wdsp.max_gain / 10.0;
    // attack_buff_size is 48 for sample rate == 12000 and
    // 96 for sample rate == 24000
    agc_wdsp.attack_buffsize = (int)ceil(sample_rate * agc_wdsp.n_tau * agc_wdsp.tau_attack);

    agc_wdsp.in_index = agc_wdsp.attack_buffsize + agc_wdsp.out_index; // attack_buffsize + out_index can be more than 2x ring_bufsize !!!
    agc_wdsp.in_index %= agc_wdsp.ring_buffsize; // need to keep this within the index boundaries

    agc_wdsp.attack_mult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_attack));
    agc_wdsp.decay_mult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_decay));
    agc_wdsp.fast_decay_mult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_fast_decay));
    agc_wdsp.fast_backmult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_fast_backaverage));
    agc_wdsp.onemfast_backmult = 1.0 - agc_wdsp.fast_backmult;

    agc_wdsp.out_target = agc_wdsp.out_targ * (1.0 - expf(-(float32_t)agc_wdsp.n_tau)) * 0.9999;
    //  out_target = out_target * (1.0 - expf(-(float32_t)n_tau)) * 0.9999;
    agc_wdsp.min_volts = agc_wdsp.out_target / (agc_wdsp.var_gain * agc_wdsp.max_gain);
    agc_wdsp.inv_out_target = 1.0 / agc_wdsp.out_target;

    tmp = log10f(agc_wdsp.out_target / (agc_wdsp.max_input * agc_wdsp.var_gain * agc_wdsp.max_gain));
    if (tmp == 0.0)
    {
        tmp = 1e-16;
    }
    agc_wdsp.slope_constant = (agc_wdsp.out_target * (1.0 - 1.0 / agc_wdsp.var_gain)) / tmp;

    agc_wdsp.inv_max_input = 1.0 / agc_wdsp.max_input;

    if (agc_wdsp.max_input > agc_wdsp.min_volts)
    {
        float32_t convert
		= powf (10.0, (float32_t)ts.agc_wdsp_hang_thresh / 20.0);
        tmp = (convert - agc_wdsp.min_volts) / (agc_wdsp.max_input - agc_wdsp.min_volts);
        if(tmp < 1e-8)
        {
        	tmp = 1e-8;
        }
        agc_wdsp.hang_thresh = 1.0 + 0.125 * log10f (tmp);
    }
    else
    {
        agc_wdsp.hang_thresh = 1.0;
    }

    tmp = powf (10.0, (agc_wdsp.hang_thresh - 1.0) / 0.125);
    agc_wdsp.hang_level = (agc_wdsp.max_input * tmp + (agc_wdsp.out_target /
            (agc_wdsp.var_gain * agc_wdsp.max_gain)) * (1.0 - tmp)) * 0.637;

    agc_wdsp.hang_backmult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_hang_backmult));
    agc_wdsp.onemhang_backmult = 1.0 - agc_wdsp.hang_backmult;

    agc_wdsp.hang_decay_mult = 1.0 - expf(-1.0 / (sample_rate * agc_wdsp.tau_hang_decay));
}

#ifdef USE_TWO_CHANNEL_AUDIO
void AudioDriver_RxAgcWdsp(int16_t blockSize, float32_t *agcbuffer1, float32_t *agcbuffer2)
#else
void AudioDriver_RxAgcWdsp(int16_t blockSize, float32_t *agcbuffer1)
#endif
{
#ifdef USE_TWO_CHANNEL_AUDIO
    const uint8_t dmod_mode = ts.dmod_mode;
    const bool use_stereo = (dmod_mode == DEMOD_IQ || dmod_mode == DEMOD_SSBSTEREO || (dmod_mode == DEMOD_SAM && ads.sam_sideband == SAM_SIDEBAND_STEREO));
#endif
    // Be careful: the original source code has no comments,
    // all comments added by DD4WH, February 2017: comments could be wrong, misinterpreting or highly misleading!
    //
    if (ts.agc_wdsp_mode == 5)  // AGC OFF
    {
        for (uint16_t i = 0; i < blockSize; i++)
        {
        	//            adb.a_buffer[i] = adb.a_buffer[i] * agc_wdsp.fixed_gain;
            agcbuffer1[i] = agcbuffer1[i] * agc_wdsp.fixed_gain;
#ifdef USE_TWO_CHANNEL_AUDIO
            {
                agcbuffer2[i] = agcbuffer2[i] * agc_wdsp.fixed_gain;
            }
#endif
        }
        return;
    }

    for (uint16_t i = 0; i < blockSize; i++)
    {
        if (++agc_wdsp.out_index >= agc_wdsp.ring_buffsize)
        {
            agc_wdsp.out_index -= agc_wdsp.ring_buffsize;
        }
        if (++agc_wdsp.in_index >= agc_wdsp.ring_buffsize)
        {
            agc_wdsp.in_index -= agc_wdsp.ring_buffsize;
        }

//        agc_wdsp.out_sample[0] = agc_wdsp.ring[agc_wdsp.out_index];
        agc_wdsp.out_sample[0] = agc_wdsp.ring[2 * agc_wdsp.out_index];
#ifdef USE_TWO_CHANNEL_AUDIO
        if(use_stereo)
        	{
        		agc_wdsp.out_sample[1] = agc_wdsp.ring[2 * agc_wdsp.out_index + 1];
        	}
#endif
        agc_wdsp.abs_out_sample = agc_wdsp.abs_ring[agc_wdsp.out_index];
        //        agc_wdsp.ring[agc_wdsp.in_index] = adb.a_buffer[i];
        //        agc_wdsp.abs_ring[agc_wdsp.in_index] = fabsf(adb.a_buffer[i]);
//        agc_wdsp.ring[agc_wdsp.in_index] = agcbuffer[i];
        agc_wdsp.ring[2 * agc_wdsp.in_index] = agcbuffer1[i];
#ifdef USE_TWO_CHANNEL_AUDIO
        if(use_stereo)
        	{
        		agc_wdsp.ring[2 * agc_wdsp.in_index + 1] = agcbuffer2[i];
        	}
#endif
        //        agc_wdsp.abs_ring[agc_wdsp.in_index] = fabsf(agcbuffer[i]);
        agc_wdsp.abs_ring[agc_wdsp.in_index] = fabsf(agcbuffer1[i]);
#ifdef USE_TWO_CHANNEL_AUDIO
        if(use_stereo)
        {
			if(agc_wdsp.abs_ring[agc_wdsp.in_index] < fabsf(agcbuffer2[i]))
			{
				agc_wdsp.abs_ring[agc_wdsp.in_index] = fabsf(agcbuffer2[i]);
			}
        }
#endif

        agc_wdsp.fast_backaverage = agc_wdsp.fast_backmult * agc_wdsp.abs_out_sample + agc_wdsp.onemfast_backmult * agc_wdsp.fast_backaverage;
        agc_wdsp.hang_backaverage = agc_wdsp.hang_backmult * agc_wdsp.abs_out_sample + agc_wdsp.onemhang_backmult * agc_wdsp.hang_backaverage;
        if(agc_wdsp.hang_backaverage > agc_wdsp.hang_level)
        {
            ts.agc_wdsp_hang_action = 1;
        }
        else
        {
            ts.agc_wdsp_hang_action = 0;
        }

        if ((agc_wdsp.abs_out_sample >= agc_wdsp.ring_max) && (agc_wdsp.abs_out_sample > 0.0))
        {
            agc_wdsp.ring_max = 0.0;
            int k = agc_wdsp.out_index;

            for (uint16_t j = 0; j < agc_wdsp.attack_buffsize; j++)
            {
                if (++k == agc_wdsp.ring_buffsize)
                {
                    k = 0;
                }
                if (agc_wdsp.abs_ring[k] > agc_wdsp.ring_max)
                {
                    agc_wdsp.ring_max = agc_wdsp.abs_ring[k];
                }
            }
        }
        if (agc_wdsp.abs_ring[agc_wdsp.in_index] > agc_wdsp.ring_max)
        {
            agc_wdsp.ring_max = agc_wdsp.abs_ring[agc_wdsp.in_index];
        }

        if (agc_wdsp.hang_counter > 0)
        {
            --agc_wdsp.hang_counter;
        }

        switch (agc_wdsp.state)
        {
        case 0: // starting point after ATTACK
        {
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            { // ATTACK
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            { // DECAY
                if (agc_wdsp.volts > agc_wdsp.pop_ratio * agc_wdsp.fast_backaverage)
                { // short time constant detector
                    agc_wdsp.state = 1;
                    agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.fast_decay_mult;
                }
                else
                { // hang AGC enabled and being activated
                    if (ts.agc_wdsp_hang_enable  && (agc_wdsp.hang_backaverage > agc_wdsp.hang_level))
                    {
                        agc_wdsp.state = 2;
                        agc_wdsp.hang_counter = (int)(agc_wdsp.hangtime * IQ_SAMPLE_RATE_F / ads.decimation_rate);
                        agc_wdsp.decay_type = 1;
                    }
                    else
                    {// long time constant detector
                        agc_wdsp.state = 3;
                        agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.decay_mult;
                        agc_wdsp.decay_type = 0;
                    }
                }
            }
            break;
        }
        case 1: // short time constant decay
        {
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            { // ATTACK
                agc_wdsp.state = 0;
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            {
                if (agc_wdsp.volts > agc_wdsp.save_volts)
                {// short time constant detector
                    agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.fast_decay_mult;
                }
                else
                {
                    if (agc_wdsp.hang_counter > 0)
                    {
                        agc_wdsp.state = 2;
                    }
                    else
                    {
                        if (agc_wdsp.decay_type == 0)
                        {// long time constant detector
                            agc_wdsp.state = 3;
                            agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.decay_mult;
                        }
                        else
                        { // hang time constant
                            agc_wdsp.state = 4;
                            agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.hang_decay_mult;
                        }
                    }
                }
            }
            break;
        }
        case 2: // Hang is enabled and active, hang counter still counting
        { // ATTACK
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            {
                agc_wdsp.state = 0;
                agc_wdsp.save_volts = agc_wdsp.volts;
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            {
                if (agc_wdsp.hang_counter == 0)
                { // hang time constant
                    agc_wdsp.state = 4;
                    agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.hang_decay_mult;
                }
            }
            break;
        }
        case 3: // long time constant decay in progress
        {
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            { // ATTACK
                agc_wdsp.state = 0;
                agc_wdsp.save_volts = agc_wdsp.volts;
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            { // DECAY
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.decay_mult;
            }
            break;
        }
        case 4: // hang was enabled and counter has counted to zero --> hang decay
        {
            if (agc_wdsp.ring_max >= agc_wdsp.volts)
            { // ATTACK
                agc_wdsp.state = 0;
                agc_wdsp.save_volts = agc_wdsp.volts;
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.attack_mult;
            }
            else
            { // HANG DECAY
                agc_wdsp.volts += (agc_wdsp.ring_max - agc_wdsp.volts) * agc_wdsp.hang_decay_mult;
            }
            break;
        }
        }
        if (agc_wdsp.volts < agc_wdsp.min_volts)
        {
            agc_wdsp.volts = agc_wdsp.min_volts; // no AGC action is taking place
            ts.agc_wdsp_action = 0;
        }
        else
        {
            // LED indicator for AGC action
            ts.agc_wdsp_action = 1;
        }

        float32_t vo =  log10f_fast(agc_wdsp.inv_max_input * agc_wdsp.volts);
        if(vo > 0.0)
        {
            vo = 0.0;
        }

        float32_t mult = (agc_wdsp.out_target - agc_wdsp.slope_constant * vo) / agc_wdsp.volts;
        agcbuffer1[i] = agc_wdsp.out_sample[0] * mult;
#ifdef USE_TWO_CHANNEL_AUDIO
        if(use_stereo)
        {
        	agcbuffer2[i] = agc_wdsp.out_sample[1] * mult;
        }
#endif
    }

    if(ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_SAM)
    {
        static float32_t    wold = 0.0;
#ifdef USE_TWO_CHANNEL_AUDIO
        static float32_t    wold2 = 0.0;
#endif
        // eliminate DC in the audio after the AGC
        for(uint16_t i = 0; i < blockSize; i++)
        {
            float32_t w = agcbuffer1[i] + wold * 0.9999; // yes, I want a superb bass response ;-)
            agcbuffer1[i] = w - wold;
            wold = w;
#ifdef USE_TWO_CHANNEL_AUDIO
            if(use_stereo)
            {
				float32_t w2 = agcbuffer2[i] + wold2 * 0.9999; // yes, I want a superb bass response ;-)
				agcbuffer2[i] = w2 - wold2;
				wold2 = w2;
            }
#endif
        }
    }
}


#if 0
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_agc_processor
//* Object              :
//* Object              : Processor for receiver AGC
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_RxAgcProcessor(int16_t blockSize, float32_t *agcbuffer)
{
    static ulong 		i;
    static ulong		agc_delay_inbuf = 0, agc_delay_outbuf = 0;
    static float32_t    w = 0.0;
    static float32_t    wold = 0.0;

    //
    // AGC function - Look-ahead type by KA7OEI, revised September 2015 to eliminate possible low-order, low-frequency instabilities associated with steady-state signals.
    // Note that even though it gets only one AGC value per cycle, it *does* do "psize/2" calculations to iterate out the AGC value more precisely than it would
    // were it called once per DMA cycle.  If it is called once per DMA cycle it may tend to weakly oscillate under certain conditions and possibly overshoot/undershoot.
    //
    // FM does not need AGC and the S-meter reading is calculated within the FM demodulation function and not here.
    //
    if(ts.agc_mode != AGC_OFF)
    {
        for(i = 0; i < blockSize; i++)
        {
            if((ts.dmod_mode == DEMOD_AM))		// if in AM, get the recovered DC voltage from the detected carrier
            { // TODO: cleanup
                // but leave this here for the moment, we are still testing 2017-02-08
                //                ads.agc_calc = ads.am_fm_agc * ads.agc_val;
//                ads.agc_calc = fabs(adb.a_buffer[i]) * ads.agc_val;
                ads.agc_calc = fabs(agcbuffer[i]) * ads.agc_val;
            }
            else	 							// not AM - get the amplitude of the recovered audio
            {   // take the absolute value of the pre-AGC, post-filter audio signal
                // and multiply by the current AGC value
//                ads.agc_calc = fabs(adb.a_buffer[i]) * ads.agc_val;
                ads.agc_calc = fabs(agcbuffer[i]) * ads.agc_val;
                //agc_calc = max_signal * ads.agc_val;	// calculate current level by scaling it with AGC value
            }

            // if the absolute audio value * AGC value is above the AGC knee, reduce AGC value quickly --> attack: fixed at 0.033
            // if below, increase slowly --> decay
            float32_t agc_decay_scaled = (ads.agc_calc < ads.agc_knee) ? (ads.agc_decay * ads.agc_decimation_scaling) : AGC_ATTACK;
            // if agc_calc is lower than knee - Increase gain slowly for AGC DECAY - scale time constant with decimation

            // agc_val = agc_val + agc_val * ((agc_knee - audio_value * agc_val) / agc_knee ) * agc_decay_scaled;
            ads.agc_var = ads.agc_knee - ads.agc_calc;	// calculate difference between agc value and "knee" value
            ads.agc_var /= ads.agc_knee;	// calculate ratio of difference between knee value and this value
            ads.agc_val += ads.agc_val * agc_decay_scaled * ads.agc_var; // adjust agc_val

            // lower limit for AGC_val
            if(ads.agc_val <= AGC_VAL_MIN)	// Prevent zero or "negative" gain values
            {
                ads.agc_val = AGC_VAL_MIN;
            }

            // upper limit for AGC_val
            if(ads.agc_val >= ads.agc_rf_gain)	 	// limit AGC to reasonable values when low/no signals present
            {
                ads.agc_val = ads.agc_rf_gain;
                if(ads.agc_val >= ads.agc_val_max)	// limit maximum gain under no-signal conditions
                {
                    ads.agc_val = ads.agc_val_max;
                }
            }
            adb.agc_valbuf[i] = ads.agc_val;            // store in "running" AGC history buffer for later application to audio data
        }
    }
    else    // AGC Off - manual AGC gain
    {
        ads.agc_val = ads.agc_rf_gain;          // use logarithmic gain value in RF gain control
        arm_fill_f32(ads.agc_rf_gain,adb.agc_valbuf,blockSize);
    }

    // Delay the post-AGC audio slightly so that the AGC's "attack" will very slightly lead the audio being acted upon by the AGC.
    // This eliminates a "click" that can occur when a very strong signal appears due to the AGC lag.  The delay is adjusted based on
    // decimation rate so that it is constant for all settings.

    arm_copy_f32(agcbuffer, &audio_delay_buffer[agc_delay_inbuf], blockSize);	// put new data into the delay buffer
    arm_copy_f32(&audio_delay_buffer[agc_delay_outbuf], agcbuffer, blockSize);	// take old data out of the delay buffer

    // Update the in/out pointers to the AGC delay buffer
    agc_delay_inbuf += blockSize;						// update circular delay buffer
    agc_delay_outbuf = agc_delay_inbuf + blockSize;
    agc_delay_inbuf %= ads.agc_delay_buflen;
    agc_delay_outbuf %= ads.agc_delay_buflen;

    // DC elimination AFTER the AGC detection
    // because we need the carrier DC for the calmness and functioning of the AGC
    // DD4WH 2017-02-08
    if(ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_SAM)
    {
        // eliminate DC in the audio before application of AGC gain
        for(i = 0; i < blockSize; i++)
        {
            w = agcbuffer[i] + wold * 0.9999; // yes, I want a superb bass response ;-)
            agcbuffer[i] = w - wold;
            wold = w;
        }
    }
    // Now apply pre-calculated AGC values to delayed audio

    arm_mult_f32(agcbuffer, adb.agc_valbuf, agcbuffer, blockSize);		// do vector multiplication to apply delayed "running" AGC data

}
#endif
//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_demod_fm
//* Object              : FM demodulator (October, 2015 - KA7OEI)
//* Object              :
//* Input Parameters    : size - size of buffer on which to operate
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_DemodFM(const int16_t blockSize)
{

	float r, s, angle, x, y, a, b;
	float32_t goertzel_buf[blockSize], squelch_buf[blockSize];
	bool tone_det_enabled;
	static float i_prev, q_prev, lpf_prev, hpf_prev_a, hpf_prev_b;// used in FM detection and low/high pass processing

	static float subdet = 0;				// used for tone detection
	static uchar count = 0, tdet = 0;// used for squelch processing and debouncing tone detection, respectively
	static ulong gcount = 0;			// used for averaging in tone detection

	if (ts.iq_freq_mode != FREQ_IQ_CONV_MODE_OFF)// bail out if translate mode is not active
	{

		tone_det_enabled = ts.fm_subaudible_tone_det_select ? 1 : 0;// set a quick flag for checking to see if tone detection is enabled

		for (uint16_t i = 0; i < blockSize; i++)
		{
			// first, calculate "x" and "y" for the arctan2, comparing the vectors of present data with previous data

			y = (i_prev * adb.q_buffer[i]) - (adb.i_buffer[i] * q_prev);
			x = (i_prev * adb.i_buffer[i]) + (adb.q_buffer[i] * q_prev);

			/*        //
			  we do not use this approximation any more, because it does not contribute significantly to saving processor cycles,
			  and does not deliver as clean audio as we would expect.
			  with atan2f the audio (and especially the hissing noise!) is much cleaner, DD4WH

			 // What follows is adapted from "Fixed-Point Atan2 With Self Normalization", public domain code by "Jim Shima".
			 // The result is "approximate" - but plenty good enough for speech-grade communications!
			 //
			 // Do calculation of arc-tangent (with quadrant preservation) of of I and Q channels, comparing with previous sample.
			 // Because the result is absolute (we are using ratios!) there is no need to apply any sort of amplitude limiting
			 //
			 abs_y = fabs(y) + 2e-16;		// prevent us from taking "zero divided by zero" (indeterminate value) by setting this to be ALWAYS at least slightly higher than zero
			 //
			 if(x >= 0)	 					// Quadrant 1 or 4
			 {
			 r = (x - abs_y) / (x + abs_y);
			 angle = FM_DEMOD_COEFF1 - FM_DEMOD_COEFF1 * r;
			 }
			 else	 						// Quadrant 2 or 3
			 {
			 r = (x + abs_y) / abs_y - x;
			 angle = FM_DEMOD_COEFF2 - FM_DEMOD_COEFF1 * r;
			 }
			 //
			 if (y < 0)						// Quadrant 3 or 4 - flip sign
			 {
			 angle = -angle;
			 }

			 */
			angle = atan2f(y, x);

			// we now have our audio in "angle"
			squelch_buf[i] = angle;	// save audio in "d" buffer for squelch noise filtering/detection - done later

			// Now do integrating low-pass filter to do FM de-emphasis
			a = lpf_prev + (FM_RX_LPF_ALPHA * (angle - lpf_prev));	//
			lpf_prev = a;			// save "[n-1]" sample for next iteration

			goertzel_buf[i] = a;	// save in "c" for subaudible tone detection

			if (((!ads.fm_squelched) && (!tone_det_enabled))
					|| ((ads.fm_subaudible_tone_detected) && (tone_det_enabled))
					|| ((!ts.fm_sql_threshold)))// high-pass audio only if we are un-squelched (to save processor time)
			{

				// Do differentiating high-pass filter to attenuate very low frequency audio components, namely subadible tones and other "speaker-rattling" components - and to remove any DC that might be present.
				b = FM_RX_HPF_ALPHA * (hpf_prev_b + a - hpf_prev_a);// do differentiation
				hpf_prev_a = a;		// save "[n-1]" samples for next iteration
				hpf_prev_b = b;
				//
				adb.a_buffer[0][i] = b;// save demodulated and filtered audio in main audio processing buffer
			}
			else if ((ads.fm_squelched)
					|| ((!ads.fm_subaudible_tone_detected) && (tone_det_enabled)))// were we squelched or tone NOT detected?
			{
				adb.a_buffer[0][i] = 0;// do not filter receive audio - fill buffer with zeroes to mute it
			}

			q_prev = adb.q_buffer[i];// save "previous" value of each channel to allow detection of the change of angle in next go-around
			i_prev = adb.i_buffer[i];
		}
#if 0
		if (!ts.agc_wdsp)
		{
			//
			ads.am_fm_agc = sqrtf(
					(q_prev * q_prev) + (i_prev * i_prev)) * FM_AGC_SCALING;// calculate amplitude of carrier to use for AGC indication only (we need it for nothing else!)
			//
			// Do "AGC" on FM signal:  Calculate/smooth signal level ONLY - no need for audio scaling
			//
			ads.agc_calc = ads.am_fm_agc * ads.agc_val;
			//
			if (ads.agc_calc < ads.agc_knee)// is audio below AGC "knee" value?
			{
				ads.agc_var = ads.agc_knee - ads.agc_calc;// calculate difference between agc value and "knee" value
				ads.agc_var /= ads.agc_knee;// calculate ratio of difference between knee value and this value
				ads.agc_val += ads.agc_val * AGC_DECAY_FM * ads.agc_var;// Yes - Increase gain for AGC DECAY (always fast in FM)
			}
			else
			{
				ads.agc_var = ads.agc_calc - ads.agc_knee;// calculate difference between agc value and "knee" value
				ads.agc_var /= ads.agc_knee;// calculate ratio of difference between knee value and this value
				ads.agc_val -= ads.agc_val * AGC_ATTACK_FM * ads.agc_var;// Fast attack to increase attenuation (do NOT scale w/decimation or else oscillation results)
				if (ads.agc_val <= AGC_VAL_MIN)	// Prevent zero or "negative" gain values
				{
					ads.agc_val = AGC_VAL_MIN;
				}
			}
			if (ads.agc_val >= ads.agc_rf_gain)	// limit AGC to reasonable values when low/no signals present
			{
				ads.agc_val = ads.agc_rf_gain;
				if (ads.agc_val >= ads.agc_val_max)	// limit maximum gain under no-signal conditions
				{
					ads.agc_val = ads.agc_val_max;
				}
			}
		}
#endif

		// *** Squelch Processing ***
		arm_iir_lattice_f32(&IIR_Squelch_HPF, squelch_buf, squelch_buf,
				blockSize);	// Do IIR high-pass filter on audio so we may detect squelch noise energy

		ads.fm_sql_avg = ((1 - FM_RX_SQL_SMOOTHING) * ads.fm_sql_avg)
				+ (FM_RX_SQL_SMOOTHING * sqrtf(fabsf(squelch_buf[0])));// IIR filter squelch energy magnitude:  We need look at only one representative sample

		//
		// Squelch processing
		//
		// Determine if the (averaged) energy in "ads.fm_sql_avg" is above or below the squelch threshold
		//
		if (count == 0)	// do the squelch threshold calculation much less often than we are called to process this audio
		{
			if (ads.fm_sql_avg > 0.175)	// limit maximum noise value in averaging to keep it from going out into the weeds under no-signal conditions (higher = noisier)
			{
				ads.fm_sql_avg = 0.175;
			}

			b = ads.fm_sql_avg * 172;// scale noise amplitude to range of squelch setting

			if (b > 24)						// limit noise amplitude range
			{
				b = 24;
			}
			//
			b = 22 - b;	// "invert" the noise power so that high number now corresponds with quieter signal:  "b" may now be compared with squelch setting
			//
			// Now evaluate noise power with respect to squelch setting
			//
			if (!ts.fm_sql_threshold)	 	// is squelch set to zero?
			{
				ads.fm_squelched = false;		// yes, the we are un-squelched
			}
			else if (ads.fm_squelched)	 	// are we squelched?
			{
				if (b >= (float) (ts.fm_sql_threshold + FM_SQUELCH_HYSTERESIS))	// yes - is average above threshold plus hysteresis?
				{
					ads.fm_squelched = false;		//  yes, open the squelch
				}
			}
			else	 	// is the squelch open (e.g. passing audio)?
			{
				if (ts.fm_sql_threshold > FM_SQUELCH_HYSTERESIS)// is setting higher than hysteresis?
				{
					if (b
							< (float) (ts.fm_sql_threshold
									- FM_SQUELCH_HYSTERESIS))// yes - is average below threshold minus hysteresis?
					{
						ads.fm_squelched = true;	// yes, close the squelch
					}
				}
				else	 // setting is lower than hysteresis so we can't use it!
				{
					if (b < (float) ts.fm_sql_threshold)// yes - is average below threshold?
					{
						ads.fm_squelched = true;	// yes, close the squelch
					}
				}
			}
			//
			count++;// bump count that controls how often the squelch threshold is checked
			count &= FM_SQUELCH_PROC_DECIMATION;	// enforce the count limit
		}

		//
		// *** Subaudible tone detection ***
		//
		if (tone_det_enabled)// is subaudible tone detection enabled?  If so, do decoding
		{
			//
			// Use Goertzel algorithm for subaudible tone detection
			//
			// We will detect differentially at three frequencies:  Above, below and on-frequency.  The two former will be used to provide a sample of the total energy
			// present as well as improve nearby-frequency discrimination.  By dividing the on-frequency energy with the averaged off-frequency energy we'll
			// get a ratio that is irrespective of the actual detected audio amplitude:  A ratio of 1.00 is considered "neutral" and it goes above unity with the increasing
			// likelihood that a tone was present on the target frequency
			//
			// Goertzel constants for the three decoders are pre-calculated in the function "UiCalcSubaudibleDetFreq()"
			//
			// (Yes, I know that below could be rewritten to be a bit more compact-looking, but it would not be much faster and it would be less-readable)
			//
			// Note that the "c" buffer contains audio that is somewhat low-pass filtered by the integrator, above
			//
			gcount++;// this counter is used for the accumulation of data over multiple cycles
			//
			for (uint16_t i = 0; i < blockSize; i++)
			{

				// Detect above target frequency
				AudioFilter_GoertzelInput(&ads.fm_goertzel[FM_HIGH],goertzel_buf[i]);
				// Detect energy below target frequency
				AudioFilter_GoertzelInput(&ads.fm_goertzel[FM_LOW],goertzel_buf[i]);
				// Detect on-frequency energy
				AudioFilter_GoertzelInput(&ads.fm_goertzel[FM_CTR],goertzel_buf[i]);
			}

			if (gcount >= FM_SUBAUDIBLE_GOERTZEL_WINDOW)// have we accumulated enough samples to do the final energy calculation?
			{
				s = AudioFilter_GoertzelEnergy(&ads.fm_goertzel[FM_HIGH]) + AudioFilter_GoertzelEnergy(&ads.fm_goertzel[FM_LOW]);
				// sum +/- energy levels:
				// s = "off frequency" energy reading

				r = AudioFilter_GoertzelEnergy(&ads.fm_goertzel[FM_CTR]);
				subdet = ((1 - FM_TONE_DETECT_ALPHA) * subdet)
						+ (r / (s / 2) * FM_TONE_DETECT_ALPHA);	// do IIR filtering of the ratio between on and off-frequency energy

				if (subdet > FM_SUBAUDIBLE_TONE_DET_THRESHOLD)// is subaudible tone detector ratio above threshold?
				{
					tdet++;	// yes - increment count			// yes - bump debounce count
					if (tdet > FM_SUBAUDIBLE_DEBOUNCE_MAX)// is count above the maximum?
					{
						tdet = FM_SUBAUDIBLE_DEBOUNCE_MAX;// yes - limit the count
					}
				}
				else	 	// it is below the threshold - reduce the debounce
				{
					if (tdet)		// - but only if already nonzero!
					{
						tdet--;
					}
				}
				if (tdet >= FM_SUBAUDIBLE_TONE_DEBOUNCE_THRESHOLD)// are we above the debounce threshold?
				{
					ads.fm_subaudible_tone_detected = 1;// yes - a tone has been detected
				}
				else									// not above threshold
				{
					ads.fm_subaudible_tone_detected = 0;	// no tone detected
				}

				gcount = 0;		// reset accumulation counter
			}
		}
		else	 		// subaudible tone detection disabled
		{
			ads.fm_subaudible_tone_detected = 1;// always signal that a tone is being detected if detection is disabled to enable audio gate
		}
	}
}

#if defined (OBSOLETE_NR) || defined (USE_LMS_AUTONOTCH)
//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_lms_notch_filter  [KA7OEI October, 2015]
//* Object              :
//* Object              : automatic notch filter
//* Input Parameters    : psize - size of buffer on which to operate
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_NotchFilter(int16_t blockSize, float32_t *notchbuffer)
{
    static ulong		lms2_inbuf = 0;
    static ulong		lms2_outbuf = 0;

    // DSP Automatic Notch Filter using LMS (Least Mean Squared) algorithm
    //
    arm_copy_f32(notchbuffer, &lmsData.lms2_nr_delay[lms2_inbuf], blockSize);	// put new data into the delay buffer
    //
    arm_lms_norm_f32(&lmsData.lms2Norm_instance, notchbuffer, &lmsData.lms2_nr_delay[lms2_outbuf], lmsData.errsig2, notchbuffer, blockSize);	// do automatic notch
    // Desired (notched) audio comes from the "error" term - "errsig2" is used to hold the discarded ("non-error") audio data
    //
    lms2_inbuf += blockSize;				// update circular de-correlation delay buffer
    lms2_outbuf = lms2_inbuf + blockSize;
    lms2_inbuf %= ts.dsp_notch_delaybuf_len;
    lms2_outbuf %= ts.dsp_notch_delaybuf_len;
    //
}
#endif


#ifdef OBSOLETE_NR
//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_lms_noise_reduction  [KA7OEI October, 2015]
//* Object              :
//* Object              : DSP noise reduction using LMS algorithm
//* Input Parameters    : psize - size of buffer on which to operate
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_NoiseReduction(int16_t blockSize, float32_t *nrbuffer)
{
    static ulong		lms1_inbuf = 0, lms1_outbuf = 0;

    arm_copy_f32(nrbuffer, &lmsData.lms1_nr_delay[lms1_inbuf], blockSize);	// put new data into the delay buffer
    //
    arm_lms_norm_f32(&lmsData.lms1Norm_instance, nrbuffer, &lmsData.lms1_nr_delay[lms1_outbuf], nrbuffer, lmsData.errsig1 ,blockSize);	// do noise reduction
    //
    // Detect if the DSP output has gone to (near) zero output - a sign of it crashing!
    //
    if((((ulong)fabs(nrbuffer[0])) * DSP_ZERO_DET_MULT_FACTOR) < DSP_OUTPUT_MINVAL)	 	// is DSP level too low?
    {
        // For some stupid reason we can't just compare above to a small fractional value  (e.g. "x < 0.001") so we must multiply it first!
        if(ads.dsp_zero_count < MAX_DSP_ZERO_COUNT)
        {
            ads.dsp_zero_count++;
        }
    }
    else
        ads.dsp_zero_count = 0;
    //
    ads.dsp_nr_sample = nrbuffer[0];		// provide a sample of the DSP output for crash detection
    //
    lms1_inbuf += blockSize;	// bump input to the next location in our de-correlation buffer
    lms1_outbuf = lms1_inbuf + blockSize;	// advance output to same distance ahead of input
    lms1_inbuf %= ts.dsp_nr_delaybuf_len;
    lms1_outbuf %= ts.dsp_nr_delaybuf_len;
}
#endif

static void AudioDriver_Mix(float32_t* src, float32_t* dst, float32_t scaling, const uint16_t blockSize)
{
    float32_t                   e3_buffer[IQ_BUFSZ+1];

    arm_scale_f32(src, scaling, e3_buffer, blockSize);
    arm_add_f32(dst, e3_buffer, dst, blockSize);
}

static void AudioDriver_IQPhaseAdjust(uint16_t txrx_mode, float32_t* i_buffer, float32_t* q_buffer, const uint16_t blockSize)
{

    int16_t trans_idx;

    // right now only in TX used, may change in future
    if ((txrx_mode == TRX_MODE_TX && ts.dmod_mode == DEMOD_CW) || ts.iq_freq_mode == FREQ_IQ_CONV_MODE_OFF)
    {
        trans_idx = IQ_TRANS_OFF;
    }
    else
    {
        trans_idx = IQ_TRANS_ON;
    }

    float32_t iq_phase_balance =  (txrx_mode == TRX_MODE_RX)? ads.iq_phase_balance_rx: ads.iq_phase_balance_tx[trans_idx];

    if (iq_phase_balance < 0)   // we only need to deal with I and put a little bit of it into Q
    {
        AudioDriver_Mix(i_buffer,q_buffer, iq_phase_balance, blockSize);
    }
    else if (iq_phase_balance > 0)  // we only need to deal with Q and put a little bit of it into I
    {
        AudioDriver_Mix(q_buffer,i_buffer, iq_phase_balance, blockSize);
    }
}


static void AudioDriver_SpectrumNoZoomProcessSamples(const uint16_t blockSize)
{

    if(sd.reading_ringbuffer == false && sd.fft_iq_len > 0)
    {
        if(sd.magnify == 0)        //
        {
            for(int i = 0; i < blockSize; i++)
            {
                // Collect I/Q samples // why are the I & Q buffers filled with I & Q, the FFT buffers are filled with Q & I?
                sd.FFT_RingBuffer[sd.samp_ptr] = adb.q_buffer[i];    // get floating point data for FFT for spectrum scope/waterfall display
                sd.samp_ptr++;
                sd.FFT_RingBuffer[sd.samp_ptr] = adb.i_buffer[i];
                sd.samp_ptr++;

                // On obtaining enough samples for spectrum scope/waterfall, update state machine, reset pointer and wait until we process what we have
                if(sd.samp_ptr >= sd.fft_iq_len-1) //*2)
                {
                    sd.samp_ptr = 0;
                }
            }
            sd.FFT_frequency = (ts.tune_freq / TUNE_MULT); // spectrum shows all, LO is center frequency;
        }
    }
}
static void AudioDriver_SpectrumZoomProcessSamples(const uint16_t blockSize)
{
    if(sd.reading_ringbuffer == false && sd.fft_iq_len > 0)
    {
        if(sd.magnify != 0)        //
            // magnify 2, 4, 8, 16, or 32
        {
            // ZOOM FFT
            // is used here to have a very close look at a small part
            // of the spectrum display of the mcHF
            // The ZOOM FFT is based on the principles described in Lyons (2011)
            // 1. take the I & Q samples
            // 2. complex conversion to baseband (at this place has already been done in audio_rx_freq_conv!)
            // 3. lowpass I and lowpass Q separately (48kHz sample rate)
            // 4. decimate I and Q separately
            // 5. apply 256-point-FFT to decimated I&Q samples
            //
            // frequency resolution: spectrum bandwidth / 256
            // example: decimate by 8 --> 48kHz / 8 = 6kHz spectrum display bandwidth
            // frequency resolution of the display --> 6kHz / 256 = 23.44Hz
            // in 32x Mag-mode the resolution is 5.9Hz, not bad for such a small processor . . .
            //

            float32_t x_buffer[IQ_BLOCK_SIZE];
            float32_t y_buffer[IQ_BLOCK_SIZE];

            // lowpass Filtering
            // Mag 2x - 12k lowpass --> 24k bandwidth
            // Mag 4x - 6k lowpass --> 12k bandwidth
            // Mag 8x - 3k lowpass --> 6k bandwidth
            // Mag 16x - 1k5 lowpass --> 3k bandwidth
            // Mag 32x - 750Hz lowpass --> 1k5 bandwidth

            // 1st attempt - use low processor power biquad lowpass filters with 4 stages each
            arm_biquad_cascade_df1_f32 (&IIR_biquad_Zoom_FFT_I, adb.i_buffer,x_buffer, blockSize);
            arm_biquad_cascade_df1_f32 (&IIR_biquad_Zoom_FFT_Q, adb.q_buffer,y_buffer, blockSize);

            // arm_iir_lattice_f32(&IIR_TXFilter, adb.i_buffer, adb.x_buffer, blockSize);
            // arm_iir_lattice_f32(&IIR_TXFilter, adb.q_buffer, adb.y_buffer, blockSize);

            // decimation
            arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_I, x_buffer, x_buffer, blockSize);
            arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_Q, y_buffer, y_buffer, blockSize);
            // collect samples for spectrum display 256-point-FFT

            int16_t blockSizeDecim = blockSize/ (1<<sd.magnify);
            for(int16_t i = 0; i < blockSizeDecim; i++)
            {
                sd.FFT_RingBuffer[sd.samp_ptr] = y_buffer[i];   // get floating point data for FFT for spectrum scope/waterfall display
                sd.samp_ptr++;
                sd.FFT_RingBuffer[sd.samp_ptr] = x_buffer[i]; //
                sd.samp_ptr++;
                // On obtaining enough samples for spectrum scope/waterfall, update state machine, reset pointer and wait until we process what we have
                if(sd.samp_ptr >= sd.fft_iq_len-1) //*2)
                {
                    sd.samp_ptr = 0;
                }
            } // end for
            sd.FFT_frequency = (ts.tune_freq / TUNE_MULT) + AudioDriver_GetTranslateFreq(); // spectrum shows center at translate frequency, LO + Translate Freq  is center frequency;


            // TODO: also insert sample collection for snap carrier here
            // and subsequently rebuild snap carrier to use a much smaller FFT (say 256 or 512)
            // in order to save many kilobytes of RAM ;-)

        }
    }
}

#ifdef USE_FREEDV
//
// this is a experimental breakout of the audio_rx_processor for getting  FreeDV to working
// used to help us to figure out how to optimize performance. Lots of performance required for
// digital signal processing...
// then it will probably be most merged back into the rx_processor in  order to keep code duplication minimal

/*
 * @returns: true if digital signal should be used (no analog processing should be done), false -> analog processing maybe used
 * since no digital signal was detected.
 */
static bool AudioDriver_RxProcessorDigital(AudioSample_t * const src, float32_t * const dst, const uint16_t blockSize)
{
    bool retval = false;
    switch(ts.dmod_mode)
    {
    case DEMOD_LSB:
    case DEMOD_USB:
    case DEMOD_DIGI:
        retval = AudioDriver_RxProcessorFreeDV(src,dst,blockSize);
        break;
    default:
        // this is silence
        arm_fill_f32(0.0,dst,blockSize);
    }
    // from here straight to final AUDIO OUT processing
    return retval;
}
#endif


static float32_t AudioDriver_FadeLeveler(int chan, float32_t audio, float32_t corr)
{
    assert (chan < NUM_AUDIO_CHANNELS);

    static float32_t dc27[NUM_AUDIO_CHANNELS]; // static will be initialized with 0
    static float32_t dc_insert[NUM_AUDIO_CHANNELS];

    dc27[chan] = adb.mtauR * dc27[chan] + adb.onem_mtauR * audio;
    dc_insert[chan] = adb.mtauI * dc_insert[chan] + adb.onem_mtauI * corr;
    audio = audio + dc_insert[chan] - dc27[chan];

    return audio;
}

//*----------------------------------------------------------------------------
//* Function Name       : SAM_demodulation [DD4WH, december 2016]
//* Object              : real synchronous AM demodulation with phase detector and PLL
//* Object              :
//* Input Parameters    : adb.i_buffer, adb.q_buffer
//* Output Parameters   : adb.a_buffer[0]
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_DemodSAM(int16_t blockSize)
{
    //#define STAGES    7

    // new synchronous AM PLL & PHASE detector
    // wdsp Warren Pratt, 2016
    //*****************************

    // First of all: decimation of I and Q path
    arm_fir_decimate_f32(&FirDecim_RxSam_I, adb.i_buffer, adb.i_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
    arm_fir_decimate_f32(&FirDecim_RxSam_Q, adb.q_buffer, adb.q_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)

    switch(ts.dmod_mode)
    {
    case DEMOD_AM:
        for(int i = 0; i < blockSize / adb.DF; i++)
        {
            float32_t audio;

            arm_sqrt_f32 (adb.i_buffer[i] * adb.i_buffer[i] + adb.q_buffer[i] * adb.q_buffer[i], &audio);
            if(ads.fade_leveler)
            {
                audio = AudioDriver_FadeLeveler(0,audio,0);
            }
            adb.a_buffer[0][i] = audio;
        }
        break;

    case DEMOD_SAM:
    {

        static uint16_t  count = 0;

        static float32_t fil_out = 0.0;
        static float32_t lowpass = 0.0;
        static float32_t omega2 = 0.0;
        static float32_t phs = 0.0;

        // Wheatley 2011 cuteSDR & Warren Pratts WDSP, 2016
        for(int i = 0; i < blockSize / adb.DF; i++)
        {   // NCO

            float32_t ai, bi, aq, bq;
            float32_t ai_ps, bi_ps, aq_ps, bq_ps;
            float32_t Sin, Cos;

            static float32_t dsI;             // delayed sample, I path
            static float32_t dsQ;             // delayed sample, Q path


            sincosf(phs,&Sin,&Cos);
            ai = Cos * adb.i_buffer[i];
            bi = Sin * adb.i_buffer[i];
            aq = Cos * adb.q_buffer[i];
            bq = Sin * adb.q_buffer[i];

            if (ads.sam_sideband != SAM_SIDEBAND_BOTH)
            {

#define OUT_IDX   (3 * SAM_PLL_HILBERT_STAGES)

                static float32_t a[3 * SAM_PLL_HILBERT_STAGES + 3];     // Filter a variables
                static float32_t b[3 * SAM_PLL_HILBERT_STAGES + 3];     // Filter b variables
                static float32_t c[3 * SAM_PLL_HILBERT_STAGES + 3];     // Filter c variables
                static float32_t d[3 * SAM_PLL_HILBERT_STAGES + 3];     // Filter d variables

                a[0] = dsI;
                b[0] = bi;
                c[0] = dsQ;
                d[0] = aq;
                dsI = ai;
                dsQ = bq;

                for (int j = 0; j < SAM_PLL_HILBERT_STAGES; j++)
                {
                    int k = 3 * j;
                    a[k + 3] = adb.c0[j] * (a[k] - a[k + 5]) + a[k + 2];
                    b[k + 3] = adb.c1[j] * (b[k] - b[k + 5]) + b[k + 2];
                    c[k + 3] = adb.c0[j] * (c[k] - c[k + 5]) + c[k + 2];
                    d[k + 3] = adb.c1[j] * (d[k] - d[k + 5]) + d[k + 2];
                }

                ai_ps = a[OUT_IDX];
                bi_ps = b[OUT_IDX];
                bq_ps = c[OUT_IDX];
                aq_ps = d[OUT_IDX];

                // make room for next sample
                for (int j = OUT_IDX + 2; j > 0; j--)
                {
                    a[j] = a[j - 1];
                    b[j] = b[j - 1];
                    c[j] = c[j - 1];
                    d[j] = d[j - 1];
                }
            }

            float32_t audio[NUM_AUDIO_CHANNELS];

            float32_t corr[2] = { ai + bq, -bi + aq };

            switch(ads.sam_sideband)
            {
            case SAM_SIDEBAND_BOTH:
            {
                audio[0] = corr[0];
                break;
            }
            case SAM_SIDEBAND_USB:
            {
                audio[0] = (ai_ps - bi_ps) + (aq_ps + bq_ps);
                break;
            }
            case SAM_SIDEBAND_LSB:
            {
                audio[0] = (ai_ps + bi_ps) - (aq_ps - bq_ps);
                break;
            }
#ifdef USE_TWO_CHANNEL_AUDIO
            case SAM_SIDEBAND_STEREO:
            {
                audio[0] = (ai_ps + bi_ps) - (aq_ps - bq_ps);
                audio[1] = (ai_ps - bi_ps) + (aq_ps + bq_ps);
                break;
            }
#endif

            }

            // "fade leveler", taken from Warren Pratts WDSP / HPSDR, 2016
            // http://svn.tapr.org/repos_sdr_hpsdr/trunk/W5WC/PowerSDR_HPSDR_mRX_PS/Source/wdsp/
            if(ads.fade_leveler)
            {
                for (int chan = 0; chan < NUM_AUDIO_CHANNELS; chan++)
                {
                    audio[chan] = AudioDriver_FadeLeveler(chan, audio[chan], corr[0]);
                }
            }

 /*
            a->dc = a->mtauR * a->dc + a->onem_mtauR * audio;
            							a->dc_insert = a->mtauI * a->dc_insert + a->onem_mtauI * corr[0];
            audio += a->dc_insert - a->dc;
   */

            for (int chan = 0; chan < NUM_AUDIO_CHANNELS; chan++)
            {
                adb.a_buffer[chan][i] = audio[chan];
            }

            // determine phase error
            float32_t phzerror = atan2f(corr[1], corr[0]);

            float32_t del_out = fil_out;
            // correct frequency 1st step
            omega2 = omega2 + adb.g2 * phzerror;
            if (omega2 < adb.omega_min)
            {
                omega2 = adb.omega_min;
            }
            else if (omega2 > adb.omega_max)
            {
                omega2 = adb.omega_max;
            }
            // correct frequency 2nd step
            fil_out = adb.g1 * phzerror + omega2;
            phs = phs + del_out;

            // wrap round 2PI, modulus
            while (phs >= 2.0 * PI) phs -= (2.0 * PI);
            while (phs < 0.0) phs += (2.0 * PI);
        }
        count++;
        if(count > 50) // to display the exact carrier frequency that the PLL is tuned to
            // in the small frequency display
            // we calculate carrier offset here and the display function is
            // then called in UiDriver_MainHandler approx. every 40-80ms
        { // to make this smoother, a simple lowpass/exponential averager here . . .
            float32_t carrier = 0.1 * (omega2 * IQ_SAMPLE_RATE) / (adb.DF * 2.0 * PI);
            carrier = carrier + 0.9 * lowpass;
            ads.carrier_freq_offset =  (int)carrier;
            count = 0;
            lowpass = carrier;
        }
    }
    break;
    }
}


float32_t sign_new (float32_t x) {
    return x < 0 ? -1.0 : ( x > 0 ? 1.0 : 0.0);
}



static void AudioDriver_RxHandleIqCorrection(const uint16_t blockSize)
{

    static uint8_t  IQ_auto_counter = 0;
    static ulong    twinpeaks_counter = 0;
    static uint8_t  codec_restarts = 0;

    if(!ts.iq_auto_correction) // Manual IQ imbalance correction
    {
        // Apply I/Q amplitude correction
        arm_scale_f32(adb.i_buffer, ts.rx_adj_gain_var.i, adb.i_buffer, blockSize);
        arm_scale_f32(adb.q_buffer, ts.rx_adj_gain_var.q, adb.q_buffer, blockSize); // TODO: we need only scale one channel! DD4WH, Dec 2016

        // Apply I/Q phase correction
        AudioDriver_IQPhaseAdjust(ts.txrx_mode,adb.i_buffer, adb.q_buffer,blockSize);
    }

    else // Automatic IQ imbalance correction
    {   // Moseley, N.A. & C.H. Slump (2006): A low-complexity feed-forward I/Q imbalance compensation algorithm.
        // in 17th Annual Workshop on Circuits, Nov. 2006, pp. 158-164.
        // http://doc.utwente.nl/66726/1/moseley.pdf
        if (ts.twinpeaks_tested == 2)
        {
            twinpeaks_counter++;
        }
        if(twinpeaks_counter > 1000) // wait 0.667ms for the system to settle: with 32 IQ samples per block and 48ksps (0.66667ms/block)
        {
            ts.twinpeaks_tested = 0;
            twinpeaks_counter = 0;
        }
        for(uint32_t i = 0; i < blockSize; i++)
        {
            adb.teta1 += sign_new(adb.i_buffer[i]) * adb.q_buffer[i]; // eq (34)
            adb.teta2 += sign_new(adb.i_buffer[i]) * adb.i_buffer[i]; // eq (35)
            adb.teta3 += sign_new(adb.q_buffer[i]) * adb.q_buffer[i]; // eq (36)
            IQ_auto_counter++;
        }
        if(IQ_auto_counter >= 8)
        {
            adb.teta1 = -0.003 * (adb.teta1 / blockSize / 8.0 ) + 0.997 * adb.teta1_old; // eq (34) and first order lowpass
            adb.teta2 =  0.003 * (adb.teta2 / blockSize / 8.0 ) + 0.997 * adb.teta2_old; // eq (35) and first order lowpass
            adb.teta3 =  0.003 * (adb.teta3 / blockSize / 8.0 ) + 0.997 * adb.teta3_old; // eq (36) and first order lowpass
            if(adb.teta2 != 0.0)// prevent divide-by-zero
            {
                adb.M_c1 = adb.teta1 / adb.teta2; // eq (30)
            }
            else
            {
                adb.M_c1 = 0.0;
            }

            float32_t help = (adb.teta2 * adb.teta2);
            if(help > 0.0)// prevent divide-by-zero
            {
                help = (adb.teta3 * adb.teta3 - adb.teta1 * adb.teta1) / help; // eq (31)
            }
            if (help > 0.0)// prevent sqrtf of negative value
            {
                adb.M_c2 = sqrtf(help); // eq (31)
            }
            else
            {
                adb.M_c2 = 1.0;
            }
            // Test and fix of the "twinpeak syndrome"
            // which occurs sporadically and can -to our knowledge- only be fixed
            // by a reset of the codec
            // It can be identified by a totally non-existing mirror rejection,
            // so I & Q have essentially the same phase
            // We use this to identify the snydrome and reset the codec accordingly:
            // calculate phase between I & Q
            if(adb.teta3 != 0.0 && !ts.twinpeaks_tested) // prevent divide-by-zero
                // twinpeak_tested = 2 --> wait for system to warm up
                // twinpeak_tested = 0 --> go and test the IQ phase
                // twinpeak_tested = 1 --> tested, verified, go and have a nice day!
            {   // Moseley & Slump (2006) eq. (33)
                // this gives us the phase error between I & Q in radians
                float32_t phase_IQ = asinf(adb.teta1 / adb.teta3);
                if ((phase_IQ > 0.3926990817 || phase_IQ < -0.3926990817) && codec_restarts < 5)
                    // threshold of 22.5 degrees phase shift == PI / 8 == 0.3926990817
                    // hopefully your hardware is not so bad, that its phase error is more than 22 degrees ;-)
                    // if it is that bad, adjust this threshold to maybe PI / 7 or PI / 6
                {
                    Codec_RestartI2S();
                    ts.twinpeaks_tested = 2;
                    codec_restarts++;
                    // TODO: we should set a maximum number of codec resets
                    // and print out a message, if twinpeaks remains after the
                    // 5th reset for example --> could then be a severe hardware error !
                    if(codec_restarts >= 4)
                    {
                        // PRINT OUT WARNING MESSAGE

                    }
                }
                else
                {
                    ts.twinpeaks_tested = 1;
                }
            }
            adb.teta1_old = adb.teta1;
            adb.teta2_old = adb.teta2;
            adb.teta3_old = adb.teta3;
            adb.teta1 = 0.0;
            adb.teta2 = 0.0;
            adb.teta3 = 0.0;
            IQ_auto_counter = 0;
        }
        // first correct Q and then correct I --> this order is crucially important!
        for(uint32_t i = 0; i < blockSize; i++)
        {   // see fig. 5
            adb.q_buffer[i] += adb.M_c1 * adb.i_buffer[i];
        }
        // see fig. 5
        arm_scale_f32 (adb.i_buffer, adb.M_c2, adb.i_buffer, blockSize);
    }

}


#ifdef USE_LEAKY_LMS

// Automatic noise reduction
// Variable-leak LMS algorithm
// taken from (c) Warren Pratts wdsp library 2016
// GPLv3 licensed
void AudioDriver_LeakyLmsNr (float32_t *in_buff, float32_t *out_buff, int buff_size, bool notch)
{
    int i, j, idx;
    float32_t c0, c1;
    float32_t y, error, sigma, inv_sigp;
    float32_t nel, nev;
		for (i = 0; i < buff_size; i++)
		{
			leakyLMS.d[leakyLMS.in_idx] = in_buff[i];

			y = 0;
			sigma = 0;

			for (j = 0; j < leakyLMS.n_taps; j++)
			{
				idx = (leakyLMS.in_idx + j + leakyLMS.delay) & leakyLMS.mask;
				y += leakyLMS.w[j] * leakyLMS.d[idx];
				sigma += leakyLMS.d[idx] * leakyLMS.d[idx];
			}
			inv_sigp = 1.0 / (sigma + 1e-10);
			error = leakyLMS.d[leakyLMS.in_idx] - y;

			if(notch)
			{ // automatic notch filter
				out_buff[i] = error;
			}
			else
			{ // noise reduction
				out_buff[i] = y;
			}
//			leakyLMS.out_buff[2 * i + 1] = 0.0;

			if((nel = error * (1.0 - leakyLMS.two_mu * sigma * inv_sigp)) < 0.0) nel = -nel;
			if((nev = leakyLMS.d[leakyLMS.in_idx] - (1.0 - leakyLMS.two_mu * leakyLMS.ngamma) * y - leakyLMS.two_mu * error * sigma * inv_sigp) < 0.0) nev = -nev;
			if (nev < nel)
			{
				if((leakyLMS.lidx += leakyLMS.lincr) > leakyLMS.lidx_max) leakyLMS.lidx = leakyLMS.lidx_max;
			}
			else
			{
				if((leakyLMS.lidx -= leakyLMS.ldecr) < leakyLMS.lidx_min) leakyLMS.lidx = leakyLMS.lidx_min;
			}
			leakyLMS.ngamma = leakyLMS.gamma * (leakyLMS.lidx * leakyLMS.lidx) * (leakyLMS.lidx * leakyLMS.lidx) * leakyLMS.den_mult;

			c0 = 1.0 - leakyLMS.two_mu * leakyLMS.ngamma;
			c1 = leakyLMS.two_mu * error * inv_sigp;

			for (j = 0; j < leakyLMS.n_taps; j++)
			{
				idx = (leakyLMS.in_idx + j + leakyLMS.delay) & leakyLMS.mask;
				leakyLMS.w[j] = c0 * leakyLMS.w[j] + c1 * leakyLMS.d[idx];
			}
			leakyLMS.in_idx = (leakyLMS.in_idx + leakyLMS.mask) & leakyLMS.mask;
		}
}

#endif

void AudioDriver_RxProcessorNoiseReduction(uint16_t blockSizeDecim, float32_t* inout_buffer)
{
#ifdef USE_ALTERNATE_NR
    const uint8_t  dsp_active = ts.dsp_active;
    static int trans_count_in=0;
    static int outbuff_count=0;
    static int NR_fill_in_pt=0;
    static NR_Buffer* out_buffer = NULL;
    // this would be the right place for another decimation-by-2 to get down to 6ksps
    // in order to further improve the spectral noise reduction
    // TEST!
    //
    // anti-alias-filtering is already provided at this stage by the IIR main filter
    // Only allow another decimation-by-two, if filter bandwidth is <= 2k7
    //
    // Add decimation-by-two HERE

    //  decide whether filter < 2k7!!!
    uint32_t no_dec_samples = blockSizeDecim; // only used for the noise reduction decimation-by-two handling
    // buffer needs max blockSizeDecim , less if second decimation is done
    // see below

    bool doSecondDecimation = ts.NR_decimation_enable && (FilterInfo[FilterPathInfo[ts.filter_path].id].width < 2701) && (dsp_active & DSP_NR_ENABLE);

    if (doSecondDecimation == true)
    {
        no_dec_samples = blockSizeDecim / 2;
        // decimate-by-2, DECIMATE_NR, in place
        arm_fir_decimate_f32(&DECIMATE_NR, inout_buffer, inout_buffer, blockSizeDecim);
    }

    // attention -> change loop no into no_dec_samples!

    for (int k = 0; k < no_dec_samples; k=k+2) //transfer our noisy audio to our NR-input buffer
    {
        mmb.nr_audio_buff[NR_fill_in_pt].samples[trans_count_in].real=inout_buffer[k];
        mmb.nr_audio_buff[NR_fill_in_pt].samples[trans_count_in].imag=inout_buffer[k+1];
        //trans_count_in++;
        trans_count_in++; // count the samples towards FFT-size  -  2 samples per loop
    }

    if (trans_count_in >= (NR_FFT_SIZE/2))
        //NR_FFT_SIZE has to be an integer mult. of blockSizeDecim!!!
    {
        NR_in_buffer_add(&mmb.nr_audio_buff[NR_fill_in_pt]); // save pointer to full buffer
        trans_count_in=0;                              // set counter to 0
        NR_fill_in_pt++;                               // increase pointer index
        NR_fill_in_pt %= NR_BUFFER_NUM;            // make sure, that index stays in range

        //at this point we have transfered one complete block of 128 (?) samples to one buffer
    }

    //**********************************************************************************
    //don't worry!  in the mean time the noise reduction routine is (hopefully) doing it's job within ui
    //as soon as "fdv_audio_has_data" we can start harvesting the output
    //**********************************************************************************

    if (out_buffer == NULL && NR_out_has_data() > 1)
    {
        NR_out_buffer_peek(&out_buffer);
    }

    float32_t NR_dec_buffer[no_dec_samples];

    if (out_buffer != NULL)  //NR-routine has finished it's job
    {
        for (int j=0; j < no_dec_samples; j=j+2) // transfer noise reduced data back to our buffer
            //                            for (int j=0; j < blockSizeDecim; j=j+2) // transfer noise reduced data back to our buffer
        {
            NR_dec_buffer[j]   = out_buffer->samples[outbuff_count+NR_FFT_SIZE].real; //here add the offset in the buffer
            NR_dec_buffer[j+1] = out_buffer->samples[outbuff_count+NR_FFT_SIZE].imag; //here add the offset in the buffer
            outbuff_count++;
        }

        if (outbuff_count >= (NR_FFT_SIZE/2)) // we reached the end of the buffer coming from NR
        {
            outbuff_count = 0;
            NR_out_buffer_remove(&out_buffer);
            out_buffer = NULL;
            NR_out_buffer_peek(&out_buffer);
        }
    }


    // interpolation of a_buffer from 6ksps to 12ksps!
    // from NR_dec_buffer --> a_buffer
    // but only, if we have decimated to 6ksps, otherwise just copy the samples into a_buffer
    if (doSecondDecimation == true)
    {
        arm_fir_interpolate_f32(&INTERPOLATE_NR, NR_dec_buffer, inout_buffer, no_dec_samples);
        arm_scale_f32(inout_buffer, 2.0, inout_buffer, blockSizeDecim);
    }
    else
    {
        arm_copy_f32(NR_dec_buffer, inout_buffer, blockSizeDecim);
    }
#endif
}


//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_processor
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_RxProcessor(AudioSample_t * const src, AudioSample_t * const dst, const uint16_t blockSize)
{
    // this is the main RX audio function
	// it is driven with 32 samples in the complex buffer scr, meaning 32 * I AND 32 * Q
	// blockSize is thus 32, DD4WH 2018_02_06

	const int16_t blockSizeDecim = blockSize/(int16_t)ads.decimation_rate;
    // we copy volatile variables which are used multiple times to local consts to let the compiler to its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t tx_audio_source = ts.tx_audio_source;
    const uint8_t iq_freq_mode = ts.iq_freq_mode;
    const uint8_t  dsp_active = ts.dsp_active;
#ifdef USE_TWO_CHANNEL_AUDIO
    const bool use_stereo = ((dmod_mode == DEMOD_IQ || dmod_mode == DEMOD_SSBSTEREO || (dmod_mode == DEMOD_SAM && ads.sam_sideband == SAM_SIDEBAND_STEREO)) && ts.stereo_enable);
#endif
    float post_agc_gain_scaling;

    if (tx_audio_source == TX_AUDIO_DIGIQ)
    {

        for(uint32_t i = 0; i < blockSize; i++)
        {
            // 16 bit format - convert to float and increment
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            audio_in_put_buffer(src[i].l);
            audio_in_put_buffer(src[i].r);
        }
    }

    if (ads.af_disabled == 0 )
    {
        // AudioDriver_NoiseBlanker(src, blockSize);     // do noise blanker function
        // ------------------------
        // Split stereo channels
        for(uint32_t i = 0; i < blockSize; i++)
        {
            if(src[i].l > ADC_CLIP_WARN_THRESHOLD/4)            // This is the release threshold for the auto RF gain
            {
                ads.adc_quarter_clip = 1;
                if(src[i].l > ADC_CLIP_WARN_THRESHOLD/2)            // This is the trigger threshold for the auto RF gain
                {
                    ads.adc_half_clip = 1;
                    if(src[i].l > ADC_CLIP_WARN_THRESHOLD)          // This is the threshold for the red clip indicator on S-meter
                    {
                        ads.adc_clip = 1;
                    }
                }
            }
            adb.i_buffer[i] = (float32_t)src[i].l;
            adb.q_buffer[i] = (float32_t)src[i].r;
        }

        // artificial amplitude imbalance for testing of the automatic IQ imbalance correction
        //    arm_scale_f32 (adb.i_buffer, 0.6, adb.i_buffer, blockSize);


        AudioDriver_RxHandleIqCorrection(blockSize);


        // Spectrum display sample collect for magnify == 0
        AudioDriver_SpectrumNoZoomProcessSamples(blockSize);

        if(iq_freq_mode)            // is receive frequency conversion to be done?
        {
            AudioDriver_FreqConversion(adb.i_buffer, adb.q_buffer, blockSize, iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ);
        }

        // Spectrum display sample collect for magnify != 0
        AudioDriver_SpectrumZoomProcessSamples(blockSize);

        //  Demodulation, optimized using fast ARM math functions as much as possible

        bool dvmode_signal = false;

#ifdef USE_FREEDV
        if (ts.dvmode == true && ts.digital_mode == DigitalMode_FreeDV)
        {
            dvmode_signal = AudioDriver_RxProcessorDigital(src, adb.a_buffer[1], blockSize);
        }
#endif

        if (dvmode_signal == false)
        {
            const bool use_decimatedIQ =
                    FilterPathInfo[ts.filter_path].FIR_I_coeff_file == i_rx_new_coeffs // lower than 3k8 bandwidth: new filters with excellent sideband suppression
                    && dmod_mode != DEMOD_FM
                    && dmod_mode != DEMOD_SAM
                    && dmod_mode != DEMOD_AM;
            volatile const uint16_t blockSizeIQ = use_decimatedIQ? blockSizeDecim: blockSize;

            // ------------------------
            // In SSB and CW - Do 0-90 degree Phase-added Hilbert Transform
            // In AM and SAM, the FIR below does ONLY low-pass filtering appropriate for the filter bandwidth selected, in
            // which case there is ***NO*** audio phase shift applied to the I/Q channels.
            //
            //
            // we need this "if" although Danilo introduced "use_decimated_IQ"
            if(dmod_mode != DEMOD_SAM && dmod_mode != DEMOD_AM) // || ads.sam_sideband == 0) // for SAM & one sideband, leave out this processor-intense filter
            {
                if(use_decimatedIQ)
                {
                    // TODO HILBERT
                    arm_fir_decimate_f32(&DECIMATE_RX_I, adb.i_buffer, adb.i_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
                    arm_fir_decimate_f32(&DECIMATE_RX_Q, adb.q_buffer, adb.q_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
                }

                arm_fir_f32(&Fir_Rx_Hilbert_I,adb.i_buffer, adb.i_buffer, blockSizeIQ);   // in AM: lowpass filter, in other modes: Hilbert lowpass 0 degrees
                arm_fir_f32(&Fir_Rx_Hilbert_Q,adb.q_buffer, adb.q_buffer, blockSizeIQ);   // in AM: lowpass filter, in other modes: Hilbert lowpass -90 degrees

            }

            switch(dmod_mode)
            {
            case DEMOD_LSB:
                arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer[0], blockSizeIQ);   // difference of I and Q - LSB
                break;
            case DEMOD_CW:
                if(!ts.cw_lsb)  // is this USB RX mode?  (LSB of mode byte was zero)
                {
                    arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer[0], blockSizeIQ);   // sum of I and Q - USB
                }
                else    // No, it is LSB RX mode
                {
                    arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer[0], blockSizeIQ);   // difference of I and Q - LSB
                }
                break;

            case DEMOD_AM:
            case DEMOD_SAM:
                AudioDriver_DemodSAM(blockSize); // lowpass filtering, decimation, and SAM demodulation
                // TODO: the above is "real" SAM, old SAM mode (below) could be renamed and implemented as DSB (double sideband mode)
                // if anybody needs that

                //            arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.f_buffer, blockSize);   // difference of I and Q - LSB
                //            arm_add_f32(adb.i_buffer, adb.q_buffer, adb.e_buffer, blockSize);   // sum of I and Q - USB
                //            arm_add_f32(adb.e_buffer, adb.f_buffer, adb.a_buffer[0], blockSize);   // sum of LSB & USB = DSB

                break;
            case DEMOD_FM:
                AudioDriver_DemodFM(blockSize);
                break;
            case DEMOD_DIGI:
                // if we are here, the digital codec (e.g. because of no signal) asked to decode
                // using analog demodulation in the respective sideband
                if (ts.digi_lsb)
                {
                    arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer[0], blockSizeIQ);   // difference of I and Q - LSB
                }
                else
                {
                    arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer[0], blockSizeIQ);   // sum of I and Q - USB
                }
                break;
#ifdef USE_TWO_CHANNEL_AUDIO
            case DEMOD_IQ:	// leave I & Q as they are!
            	arm_copy_f32(adb.i_buffer, adb.a_buffer[0], blockSizeIQ);
            	arm_copy_f32(adb.q_buffer, adb.a_buffer[1], blockSizeIQ);
            	break;
            case DEMOD_SSBSTEREO: // LSB-left, USB-right
            	arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer[0], blockSizeIQ);   // sum of I and Q - USB
                arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer[1], blockSizeIQ);   // difference of I and Q - LSB
            	break;
#endif
            case DEMOD_USB:
            default:
                arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer[0], blockSizeIQ);   // sum of I and Q - USB
                break;
            }

            if(dmod_mode != DEMOD_FM)       // are we NOT in FM mode?  If we are not, do decimation, filtering, DSP notch/noise reduction, etc.
            {
                // Do decimation down to lower rate to reduce processor load
                if (    DECIMATE_RX_I.numTaps > 0
                        && use_decimatedIQ == false // we did not already decimate the input earlier
                        && dmod_mode != DEMOD_SAM
                        && dmod_mode != DEMOD_AM) // in AM/SAM mode, the decimation has been done in both I & Q path --> AudioDriver_Demod_SAM
                {
                    // TODO HILBERT
                    arm_fir_decimate_f32(&DECIMATE_RX_I, adb.a_buffer[0], adb.a_buffer[0], blockSizeIQ);      // LPF built into decimation (Yes, you can decimate-in-place!)
#ifdef USE_TWO_CHANNEL_AUDIO
                    if(use_stereo)
                    {
                        arm_fir_decimate_f32(&DECIMATE_RX_Q, adb.a_buffer[1], adb.a_buffer[1], blockSizeIQ);      // LPF built into decimation (Yes, you can decimate-in-place!)
                    }
#endif
                }

                if (ts.dsp_inhibit == false)
                {
                    if((dsp_active & DSP_NOTCH_ENABLE) && (dmod_mode != DEMOD_CW) && !(dmod_mode == DEMOD_SAM && (FilterPathInfo[ts.filter_path].sample_rate_dec) == RX_DECIMATION_RATE_24KHZ))       // No notch in CW
                    {
#ifdef USE_LEAKY_LMS
                    	if(ts.enable_leaky_LMS)
                    	{
                    	      AudioDriver_LeakyLmsNr(adb.a_buffer[0], adb.a_buffer[0], blockSizeDecim, 1);
                    	}
                        else
#endif
                        {
//#ifdef OBSOLETE_NR
#ifdef USE_LMS_AUTONOTCH
                        	AudioDriver_NotchFilter(blockSizeDecim, adb.a_buffer[0]);     // Do notch filter
#endif
                        }
                    }

                    // DSP noise reduction using LMS (Least Mean Squared) algorithm
                    // This is the pre-filter/AGC instance
                    if((dsp_active & DSP_NR_ENABLE) && (!(dsp_active & DSP_NR_POSTAGC_ENABLE)) && !(dmod_mode == DEMOD_SAM && (FilterPathInfo[ts.filter_path].sample_rate_dec) == RX_DECIMATION_RATE_24KHZ))      // Do this if enabled and "Pre-AGC" DSP NR enabled
                    {
#ifdef USE_LEAKY_LMS
                    	if(ts.enable_leaky_LMS)
                    	{
                    	      AudioDriver_LeakyLmsNr(adb.a_buffer[0], adb.a_buffer[0], blockSizeDecim, 0);
                    	}
                        else
#endif
                        {
#ifdef OBSOLETE_NR
                        	AudioDriver_NoiseReduction(blockSizeDecim, adb.a_buffer[0]);     //
#endif
                        }
                    }

                }

                // Apply audio  bandpass filter
                if ((IIR_PreFilter[0].numStages > 0))   // yes, we want an audio IIR filter
                {
                    arm_iir_lattice_f32(&IIR_PreFilter[0], adb.a_buffer[0], adb.a_buffer[0], blockSizeDecim);
#ifdef USE_TWO_CHANNEL_AUDIO
                    if(use_stereo && !ads.af_disabled)
                    {
                         arm_iir_lattice_f32(&IIR_PreFilter[1], adb.a_buffer[1], adb.a_buffer[1], blockSizeDecim);
                    }
#endif
                }

                // now process the samples and perform the receiver AGC function
#ifdef USE_TWO_CHANNEL_AUDIO
                    AudioDriver_RxAgcWdsp(blockSizeDecim, adb.a_buffer[0], adb.a_buffer[1]);
#else
                    AudioDriver_RxAgcWdsp(blockSizeDecim, adb.a_buffer[0]);
#endif


                // DSP noise reduction using LMS (Least Mean Squared) algorithm
                // This is the post-filter, post-AGC instance
                //
                if((dsp_active & DSP_NR_ENABLE) && (dsp_active & DSP_NR_POSTAGC_ENABLE) && (!ts.dsp_inhibit) && !(dmod_mode == DEMOD_SAM && (FilterPathInfo[ts.filter_path].sample_rate_dec) == RX_DECIMATION_RATE_24KHZ))     // Do DSP NR if enabled and if post-DSP NR enabled
                {
#ifdef USE_LEAKY_LMS
                	if(ts.enable_leaky_LMS)
                	{
                	      AudioDriver_LeakyLmsNr(adb.a_buffer[0], adb.a_buffer[0], blockSizeDecim, 0);
                	}
                    else
#endif
                    {
#ifdef OBSOLETE_NR
                    	AudioDriver_NoiseReduction(blockSizeDecim, adb.a_buffer[0]);     //
#endif
                    }
                }
                //
                //                if (ts.new_nb==true || ts.nr_enable == true) //start of new nb
                if (ts.nb_setting > 0 || (dsp_active & DSP_NR_ENABLE)) //start of new nb or new noise reduction
                {
                    // NR_in and _out buffers are using the same physical space than the freedv_iq_buffer in a
                    // shared MultiModeBuffer union.
                    // for NR reduction we use a maximum of 256 real samples
                    // so we use the freedv_iq buffers in a way, that we use the first half of each array for the input
                    // and the second half for the output
                    // .real and .imag are loosing there meaning here as they represent consecutive real samples

                    if (ads.decimation_rate == 4)   //  to make sure, that we are at 12Ksamples
                    {
                        AudioDriver_RxProcessorNoiseReduction(blockSizeDecim, adb.a_buffer[0]);

                    }
                } // end of new nb

                // Calculate scaling based on decimation rate since this affects the audio gain
                if ((FilterPathInfo[ts.filter_path].sample_rate_dec) == RX_DECIMATION_RATE_12KHZ)
                {
                    post_agc_gain_scaling = POST_AGC_GAIN_SCALING_DECIMATE_4;
                }
                else
                {
                    post_agc_gain_scaling = POST_AGC_GAIN_SCALING_DECIMATE_2;
                }

                // Scale audio according to AGC setting, demodulation mode and required fixed levels and scaling
                float32_t scale_gain;
                if(dmod_mode == DEMOD_AM || dmod_mode == DEMOD_SAM)
                {
                        scale_gain = post_agc_gain_scaling * 0.5; // ignore ts.max_rf_gain  --> has no meaning with WDSP AGC; and take into account AM scaling factor
                }
                else        // Not AM
                {
                        scale_gain = post_agc_gain_scaling * 0.333; // ignore ts.max_rf_gain --> has no meaning with WDSP AGC
                }
                arm_scale_f32(adb.a_buffer[0],scale_gain, adb.a_buffer[0], blockSizeDecim); // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
#ifdef USE_TWO_CHANNEL_AUDIO
                if(use_stereo)
                {
                    arm_scale_f32(adb.a_buffer[1],scale_gain, adb.a_buffer[1], blockSizeDecim); // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
                }
#endif
                // this is the biquad filter, a notch, peak, and lowshelf filter
                arm_biquad_cascade_df1_f32 (&IIR_biquad_1[0], adb.a_buffer[0],adb.a_buffer[0], blockSizeDecim);
#ifdef USE_TWO_CHANNEL_AUDIO
                if(use_stereo)
                {
                    arm_biquad_cascade_df1_f32 (&IIR_biquad_1[1], adb.a_buffer[1],adb.a_buffer[1], blockSizeDecim);
                }
#endif
#ifdef USE_RTTY_PROCESSOR
                if (is_demod_rtty() && blockSizeDecim == 8) // only works when decimation rate is 4 --> sample rate == 12ksps
                {
                    AudioDriver_RxProcessor_Rtty(adb.a_buffer[0], blockSizeDecim);
                }
#endif
                if (is_demod_psk() && blockSizeDecim == 8) // only works when decimation rate is 4 --> sample rate == 12ksps
                {
                    AudioDriver_RxProcessor_Bpsk(adb.a_buffer[0], blockSizeDecim);
                }
//                if(blockSizeDecim ==8 && dmod_mode == DEMOD_CW)
//                if(ts.cw_decoder_enable && blockSizeDecim ==8 && (dmod_mode == DEMOD_CW || dmod_mode == DEMOD_AM || dmod_mode == DEMOD_SAM))
                if(blockSizeDecim ==8 && (dmod_mode == DEMOD_CW || dmod_mode == DEMOD_AM || dmod_mode == DEMOD_SAM))
// switch to use TUNE HELPER in AM/SAM
                {
                	CwDecode_RxProcessor(adb.a_buffer[0], blockSizeDecim);
                }

                // resample back to original sample rate while doing low-pass filtering to minimize audible aliasing effects
                if (INTERPOLATE_RX[0].phaseLength > 0)
                {
#ifdef USE_TWO_CHANNEL_AUDIO
                    float32_t temp_buffer[IQ_BLOCK_SIZE];
                    if(use_stereo)
                    {
                        arm_fir_interpolate_f32(&INTERPOLATE_RX[1], adb.a_buffer[1], temp_buffer, blockSizeDecim);
                    }
#endif
                    arm_fir_interpolate_f32(&INTERPOLATE_RX[0], adb.a_buffer[0], adb.a_buffer[1], blockSizeDecim);

#ifdef USE_TWO_CHANNEL_AUDIO
                    if(use_stereo)
                    {
                        arm_copy_f32(temp_buffer,adb.a_buffer[0],blockSize);
                    }
#endif

                }
                // additional antialias filter for specific bandwidths
                // IIR ARMA-type lattice filter
                if (IIR_AntiAlias[0].numStages > 0)   // yes, we want an interpolation IIR filter
                {
                    arm_iir_lattice_f32(&IIR_AntiAlias[0], adb.a_buffer[1], adb.a_buffer[1], blockSize);
#ifdef USE_TWO_CHANNEL_AUDIO
                    if(use_stereo)
                    {
                        arm_iir_lattice_f32(&IIR_AntiAlias[1], adb.a_buffer[0], adb.a_buffer[0], blockSize);
                    }
#endif
                }

            } // end NOT in FM mode
            else if(dmod_mode == DEMOD_FM)           // it is FM - we don't do any decimation, interpolation, filtering or any other processing - just rescale audio amplitude
            {
                arm_scale_f32(
                        adb.a_buffer[0],
                        RadioManagement_FmDevIs5khz() ? FM_RX_SCALING_5K : FM_RX_SCALING_2K5,
                                adb.a_buffer[1],
                                blockSizeDecim);  // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
#ifdef USE_TWO_CHANNEL_AUDIO
                    AudioDriver_RxAgcWdsp(blockSizeDecim, adb.a_buffer[0], adb.a_buffer[1]);
#else
                    AudioDriver_RxAgcWdsp(blockSizeDecim, adb.a_buffer[0]);
#endif
            }

            // this is the biquad filter, a highshelf filter
            arm_biquad_cascade_df1_f32 (&IIR_biquad_2[0], adb.a_buffer[1],adb.a_buffer[1], blockSize);
#ifdef USE_TWO_CHANNEL_AUDIO
            if(use_stereo)
            {
                arm_biquad_cascade_df1_f32 (&IIR_biquad_2[1], adb.a_buffer[0],adb.a_buffer[0], blockSize);
            }
#endif
        }
    }

    bool do_mute_output =
            ts.audio_dac_muting_flag
            || ts.audio_dac_muting_buffer_count > 0
            || (ads.af_disabled)
            || ((dmod_mode == DEMOD_FM) && ads.fm_squelched);
    // this flag is set during rx tx transition, so once this is active we mute our output to the I2S Codec

    if (do_mute_output)
        // fill audio buffers with zeroes if we are to mute the receiver completely while still processing data OR it is in FM and squelched
        // or when filters are switched
    {
        arm_fill_f32(0, adb.a_buffer[0], blockSize);
        arm_fill_f32(0, adb.a_buffer[1], blockSize);
        if (ts.audio_dac_muting_buffer_count > 0)
        {
            ts.audio_dac_muting_buffer_count--;
        }
    }
    else
    {
#ifdef USE_TWO_CHANNEL_AUDIO
        // BOTH CHANNELS "FIXED" GAIN as input for audio amp and headphones/lineout
        // each output path has its own gain control.
    	// Do fixed scaling of audio for LINE OUT
    	arm_scale_f32(adb.a_buffer[1], LINE_OUT_SCALING_FACTOR, adb.a_buffer[1], blockSize);
    	if (use_stereo)
    	{
    		arm_scale_f32(adb.a_buffer[0], LINE_OUT_SCALING_FACTOR, adb.a_buffer[0], blockSize);
    	}
    	else
    	{
    		// we simply copy the data from the other channel
    		arm_copy_f32(adb.a_buffer[1], adb.a_buffer[0], blockSize);
    	}
#else
        // VARIABLE LEVEL FOR SPEAKER
        // LINE OUT (constant level)
    	arm_scale_f32(adb.a_buffer[1], LINE_OUT_SCALING_FACTOR, adb.a_buffer[0], blockSize);       // Do fixed scaling of audio for LINE OUT and copy to "a" buffer in one operation
#endif
    	//
        // AF gain in "ts.audio_gain-active"
        //  0 - 16: via codec command
        // 17 - 20: soft gain after decoder
        //
#ifdef UI_BRD_MCHF
        if(ts.rx_gain[RX_AUDIO_SPKR].value > CODEC_SPEAKER_MAX_VOLUME)    // is volume control above highest hardware setting?
        {
            arm_scale_f32(adb.a_buffer[1], (float32_t)ts.rx_gain[RX_AUDIO_SPKR].active_value, adb.a_buffer[1], blockSize);    // yes, do software volume control adjust on "b" buffer
        }
#endif
    }



    float32_t usb_audio_gain = ts.rx_gain[RX_AUDIO_DIG].value/31.0;

    // Transfer processed audio to DMA buffer
    for(int i=0; i < blockSize; i++)                            // transfer to DMA buffer and do conversion to INT
    {
        // TODO: move to softdds ...
        if((ts.beep_active) && (ads.beep.step))         // is beep active?
        {
            // Yes - Calculate next sample
            // shift accumulator to index sine table
            adb.a_buffer[1][i] += (float32_t)softdds_nextSample(&ads.beep) * ads.beep_loudness_factor; // load indexed sine wave value, adding it to audio, scaling the amplitude and putting it on "b" - speaker (ONLY)
        }
        else                    // beep not active - force reset of accumulator to start at zero to minimize "click" caused by an abrupt voltage transition at startup
        {
            ads.beep.acc = 0;
        }

        if (do_mute_output)
        {
            dst[i].l = 0;
            dst[i].r = 0;
        }
        else
        {
        	dst[i].l = adb.a_buffer[1][i];
        	dst[i].r = adb.a_buffer[0][i];
        }
        // Unless this is DIGITAL I/Q Mode, we sent processed audio
        if (tx_audio_source != TX_AUDIO_DIGIQ)
        {
        	int16_t vals[2];

#ifdef USE_TWO_CHANNEL_AUDIO
        	vals[0] = adb.a_buffer[0][i] * usb_audio_gain;
        	vals[1] = adb.a_buffer[1][i] * usb_audio_gain;
#else
        	vals[0] = vals[1] = adb.a_buffer[0][i] * usb_audio_gain;
#endif

        	audio_in_put_buffer(vals[0]);
            audio_in_put_buffer(vals[1]);
        }
    }
}


/**
 * @brief audio speech compressor (look-ahead type) by KA7OEI
 * @param buffer input (and output) buffer for audio samples
 * @param blockSize number of samples to process
 * @param gain_scaling scaling applied to buffer
 *
 */
static void AudioDriver_TxCompressor(float32_t* buffer, int16_t blockSize, float gain_scaling)
{
    static uint32_t	alc_delay_inbuf = 0, alc_delay_outbuf;

    if (ts.tx_comp_level > -1)
    {
        if(!ts.tune)        // do post-filter gain calculations if we are NOT in TUNE mode
        {
            // perform post-filter gain operation
            // this is part of the compression
            float32_t gain_calc = ((float32_t)ts.alc_tx_postfilt_gain_var)/2.0 +0.5 ;

            // get post-filter gain setting
            // offset it so that 2 = unity
            arm_scale_f32(buffer, gain_calc, buffer, blockSize);      // use optimized function to apply scaling to I/Q buffers
        }

        // Do ALC processing on audio buffer - look-ahead type by KA7OEI
        if (false)
        {
            arm_fill_f32(ALC_VAL_MAX * gain_scaling, adb.agc_valbuf, blockSize);
        }
        else
        {
            // since both values are marked as volatile, we copy them before using them, saves some cpu cycles.
            static float32_t alc_val;
            alc_val = ads.alc_val;
            float32_t alc_decay = ads.alc_decay;

            for(uint16_t i = 0; i < blockSize; i++)
            {
                // perform ALC on post-filtered audio (You will notice the striking similarity to the AGC code!)

                // calculate current level by scaling it with ALC value
                float32_t alc_var = fabsf(buffer[i] * alc_val)/ALC_KNEE - 1.0; // calculate difference between ALC value and "knee" value
                if(alc_var < 0)	 	// is audio below ALC "knee" value?
                {
                    // alc_var is a negative value, so the resulting expression is negative
                    // but we want to increase the alc_val -> we subtract it
                    alc_val -= alc_val * alc_decay * alc_var;	// (ALC DECAY) Yes - Increase gain slowly
                }
                else
                {
                    // alc_var is a positive value
                    alc_val -= alc_val * ALC_ATTACK * alc_var;	// Fast attack to increase gain
                    if(alc_val < ALC_VAL_MIN)	// Prevent zero or "negative" gain values
                    {
                        alc_val = ALC_VAL_MIN;
                    }
                }
                if(alc_val > ALC_VAL_MAX)	// limit to fixed values within the code
                {
                    alc_val = ALC_VAL_MAX;
                }

                adb.agc_valbuf[i] = (alc_val * gain_scaling);	// store in "running" ALC history buffer for later application to audio data
            }

            // copy final alc_val back into "storage"
            ads.alc_val = alc_val;

        }

        // Delay the post-ALC audio slightly so that the ALC's "attack" will very slightly lead the audio being acted upon by the ALC.
        // This eliminates a "click" that can occur when a very strong signal appears due to the ALC lag.  The delay is adjusted based on
        // decimation rate so that it is constant for all settings.

        // Update the in/out pointers to the ALC delay buffer
        alc_delay_inbuf += blockSize;
        alc_delay_outbuf = alc_delay_inbuf + blockSize;
        alc_delay_inbuf %= AUDIO_DELAY_BUFSIZE;
        alc_delay_outbuf %= AUDIO_DELAY_BUFSIZE;

        arm_copy_f32(buffer, &audio_delay_buffer[alc_delay_inbuf], blockSize);	// put new data into the delay buffer
        arm_copy_f32(&audio_delay_buffer[alc_delay_outbuf], buffer, blockSize);	// take old data out of the delay buffer

        arm_mult_f32(buffer, adb.agc_valbuf, buffer, blockSize);		// Apply ALC gain corrections to TX audio channels
    }
}

/**
 * @brief Equalize based on band and simultaneously apply I/Q gain AND phase adjustments
 * The input is IQ data in the adb.i_buffer and adb.q_buffer
 *
 * @param dst output buffer for generated IQ audio samples
 * @param blockSize number of samples (input/output)
 * @param swap i and q buffers meaning
 * @param scaling gain applied to the samples before conversion to integer data
 *
 */

static void AudioDriver_TxIqProcessingFinal(float32_t scaling, bool swap, AudioSample_t* const dst, const uint16_t blockSize)
{
    int16_t trans_idx;

    if (ts.dmod_mode == DEMOD_CW || ts.iq_freq_mode == FREQ_IQ_CONV_MODE_OFF)
    {
        trans_idx = IQ_TRANS_OFF;
    }
    else
    {
        trans_idx = IQ_TRANS_ON;
    }

    float32_t *final_i_buffer, *final_q_buffer;

    float32_t final_i_gain = ts.tx_power_factor * ts.tx_adj_gain_var[trans_idx].i * scaling;
    float32_t final_q_gain = ts.tx_power_factor * ts.tx_adj_gain_var[trans_idx].q * scaling;

    // Output I and Q as stereo data
    if(swap == false)	 			// if is it "RX LO LOW" mode, save I/Q data without swapping, putting it in "upper" sideband (above the LO)
    {
        final_i_buffer = adb.i_buffer;
        final_q_buffer = adb.q_buffer;
    }
    else	 	// it is "RX LO HIGH" - swap I/Q data while saving, putting it in the "lower" sideband (below the LO)
    {
        final_i_buffer = adb.q_buffer;
        final_q_buffer = adb.i_buffer;
    }

    // this is the IQ gain / amplitude adjustment
    arm_scale_f32(final_i_buffer, final_i_gain, final_i_buffer, blockSize);
    arm_scale_f32(final_q_buffer, final_q_gain, final_q_buffer, blockSize);

    // this is the IQ phase adjustment
    AudioDriver_IQPhaseAdjust(ts.txrx_mode, final_i_buffer, final_q_buffer,blockSize);
    for(int i = 0; i < blockSize; i++)
    {
        // Prepare data for DAC
        dst[i].l = final_i_buffer[i]; // save left channel
        dst[i].r = final_q_buffer[i]; // save right channel
    }

}


static float32_t AudioDriver_absmax(float32_t* buffer, int size) {
    float32_t min, max;
    uint32_t            pindex;

    arm_max_f32(buffer, size, &max, &pindex);      // find absolute value of audio in buffer after gain applied
    arm_min_f32(buffer, size, &min, &pindex);

    return -min>max?-min:max;
}

static void AudioDriver_TxAudioBufferFill(AudioSample_t * const src, int16_t blockSize)
{
    const uint8_t tx_audio_source = ts.tx_audio_source;


    if(ts.tune)     // TUNE mode?  If so, generate tone so we can adjust TX IQ phase and gain
    {
        softdds_runIQ(adb.a_buffer[0], adb.a_buffer[0],blockSize);     // load audio buffer with the tone - DDS produces quadrature channels, but we need only one
    }
    else
    {
        float32_t           gain_calc;

        switch(tx_audio_source)
        {
        case TX_AUDIO_LINEIN_L:
        case TX_AUDIO_LINEIN_R:      // Are we in LINE IN mode?
        {
            gain_calc = LINE_IN_GAIN_RESCALE;           // Yes - fixed gain scaling for line input - the rest is done in hardware
        }
        break;
        case TX_AUDIO_MIC:
        {
            gain_calc = ts.tx_mic_gain_mult;     // We are in MIC In mode:  Calculate Microphone gain
            gain_calc /= MIC_GAIN_RESCALE;              // rescale microphone gain to a reasonable range
        }
        break;
        case TX_AUDIO_DIG:
        {
            // gain_calc = ts.tx_gain[TX_AUDIO_DIG];     // We are in MIC In mode:  Calculate Microphone gain
            // gain_calc /= 16;              // rescale microphone gain to a reasonable range
            gain_calc = 1;
        }
        break;
        default:
        {
            gain_calc = 1;
        }
        }

        if(tx_audio_source == TX_AUDIO_LINEIN_R)         // Are we in LINE IN RIGHT CHANNEL mode?
        {
            // audio buffer with right sample channel
            for(int i = 0; i < blockSize; i++)
            {
                adb.a_buffer[0][i] = src[i].r;
            }
        } else {
            // audio buffer with left sample channel
            for(int i = 0; i < blockSize; i++)
            {
                adb.a_buffer[0][i] = src[i].l;
            }
        }

        if (gain_calc != 1.0)
        {
        	arm_scale_f32(adb.a_buffer[0], gain_calc, adb.a_buffer[0], blockSize);  // apply gain
        }

        ads.peak_audio = AudioDriver_absmax(adb.a_buffer[0], blockSize);
    }
}

/**
 * takes audio samples in adb.a_buffer[0] and produces SSB in adb.i_buffer/adb.q_buffer
 * audio samples should filtered before passed in here if necessary
 */

static void AudioDriver_TxProcessorModulatorSSB(AudioSample_t * const dst, const uint16_t blockSize, const uint8_t iq_freq_mode, const bool is_lsb)
{
    // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
    // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
    // + 0 deg to I data
    arm_fir_f32(&Fir_Tx_Hilbert_I, adb.a_buffer[0], adb.i_buffer, blockSize);
    // - 90 deg to Q data
    arm_fir_f32(&Fir_Tx_Hilbert_Q, adb.a_buffer[0], adb.q_buffer, blockSize);

    if(iq_freq_mode)
    {
        // is transmit frequency conversion to be done?
        // LSB && (-6kHz || -12kHz) --> true, else false
        // USB && (+6kHz || +12kHz) --> true, else false
        bool swap = is_lsb == true && (iq_freq_mode == FREQ_IQ_CONV_M6KHZ || iq_freq_mode == FREQ_IQ_CONV_M12KHZ);
        swap = swap || ((is_lsb == false) && (iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ));

        AudioDriver_FreqConversion(adb.i_buffer, adb.q_buffer, blockSize, swap);
    }

    // apply I/Q amplitude & phase adjustments
    AudioDriver_TxIqProcessingFinal(SSB_GAIN_COMP, is_lsb, dst, blockSize);

}

/***
 * takes the I Q input buffers containing the I Q audio for a single AM sideband and returns the frequency translated IQ sideband
 * in the i/q input buffers given as argument.
 */
static void AudioDriver_TxProcessorAMSideband(float32_t* i_buffer, float32_t* q_buffer,  const int16_t blockSize) {

    // generate AM carrier by applying a "DC bias" to the audio
    arm_offset_f32(i_buffer, AM_CARRIER_LEVEL, i_buffer, blockSize);
    arm_offset_f32(q_buffer, (-1 * AM_CARRIER_LEVEL), q_buffer, blockSize);

    // check and apply correct translate mode
    AudioDriver_FreqConversion(i_buffer, q_buffer, blockSize, (ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ));
}

static inline void AudioDriver_TxFilterAudio(bool do_bandpass, bool do_bass_treble, float32_t* inBlock, float32_t* outBlock, const uint16_t blockSize)
{
    if (do_bandpass)
    {
        arm_iir_lattice_f32(&IIR_TXFilter, inBlock, outBlock, blockSize);
    }
    if (do_bass_treble)
    {
        // biquad filter for bass & treble --> NOT enabled when using USB Audio (eg. for Digimodes)
        arm_biquad_cascade_df1_f32 (&IIR_TX_biquad, outBlock,outBlock, blockSize);
    }
}

static void AudioDriver_TxProcessorFM(AudioSample_t * const src, AudioSample_t * const dst, uint16_t blockSize)
{
    static float32_t    hpf_prev_a, hpf_prev_b;
    float32_t           a, b;
    const uint32_t iq_freq_mode = ts.iq_freq_mode;

    static uint32_t fm_mod_idx = 0, fm_mod_accum = 0, fm_tone_idx = 0, fm_tone_accum = 0, fm_tone_burst_idx = 0, fm_tone_burst_accum = 0;

    float32_t fm_mod_mult;
    // Fill I and Q buffers with left channel(same as right)
    //
    if(RadioManagement_FmDevIs5khz())   // are we in 5 kHz modulation mode?
    {
        fm_mod_mult = 2;    // yes - multiply all modulation factors by 2
    }
    else
    {
        fm_mod_mult = 1;    // not in 5 kHz mode - used default (2.5 kHz) modulation factors
    }

    AudioDriver_TxAudioBufferFill(src,blockSize);

    if (!ts.tune)
    {
        AudioDriver_TxFilterAudio(true,ts.tx_audio_source != TX_AUDIO_DIG,adb.a_buffer[0],adb.a_buffer[0], blockSize);
    }

    AudioDriver_TxCompressor(adb.a_buffer[0], blockSize, FM_ALC_GAIN_CORRECTION);  // Do the TX ALC and speech compression/processing

    // Do differentiating high-pass filter to provide 6dB/octave pre-emphasis - which also removes any DC component!  Takes audio from "a" and puts it into "a".
    for(int i = 0; i < blockSize; i++)
    {
        a = adb.a_buffer[0][i];

        b = FM_TX_HPF_ALPHA * (hpf_prev_b + a - hpf_prev_a);    // do differentiation
        hpf_prev_a = a;     // save "[n-1] samples for next iteration
        hpf_prev_b = b;

        adb.a_buffer[0][i] = b;    // save differentiated data in audio buffer
    }

    // do tone generation using the NCO (a.k.a. DDS) method.  This is used for subaudible tone generation and, if necessary, summing the result in "a".
    if((ads.fm_subaudible_tone_word) && (!ads.fm_tone_burst_active))        // generate tone only if it is enabled (and not during a tone burst)
    {
        for(int i = 0; i < blockSize; i++)
        {
            fm_tone_accum += ads.fm_subaudible_tone_word;   // generate tone using frequency word, calculating next sample
            fm_tone_accum &= 0xffffff;              // limit to 16 Meg range
            fm_tone_idx    = fm_tone_accum >> FM_TONE_DDS_ACC_SHIFT;    // shift accumulator to index sine table
            fm_tone_idx &= (DDS_TBL_SIZE-1);        // limit lookup to range of sine table
            adb.a_buffer[0][i] += ((float32_t)(DDS_TABLE[fm_tone_idx]) * FM_TONE_AMPLITUDE_SCALING * fm_mod_mult); // load indexed sine wave value, adding it to audio
        }
    }

    // do tone  generation using the NCO (a.k.a. DDS) method.  This is used for tone burst ("whistle-up") generation, summing the result in "a".
    if(ads.fm_tone_burst_active)                // generate tone burst only if it is enabled
    {
        for(int i = 0; i < blockSize; i++)
        {
            // Calculate next sample
            fm_tone_burst_accum += ads.fm_tone_burst_word;  // generate tone using frequency word, calculating next sample
            fm_tone_burst_accum &= 0xffffff;                // limit to 16 Meg range
            fm_tone_burst_idx    = fm_tone_burst_accum >> FM_TONE_DDS_ACC_SHIFT;    // shift accumulator to index sine table
            fm_tone_burst_idx &= (DDS_TBL_SIZE-1);      // limit lookup to range of sine table
            adb.a_buffer[0][i] += ((float32_t)((DDS_TABLE[fm_tone_burst_idx]) * FM_MOD_SCALING * fm_mod_mult) / FM_TONE_BURST_MOD_SCALING);    // load indexed sine wave value, adding it to audio
        }
    }
    //
    // do audio frequency modulation using the NCO (a.k.a. DDS) method, carrier at 6 kHz.  Audio is in "a", the result being quadrature FM in "i" and "q".
    //
    uint32_t fm_freq_mod_word = FM_FREQ_MOD_WORD *  ((iq_freq_mode == FREQ_IQ_CONV_P12KHZ || iq_freq_mode == FREQ_IQ_CONV_M12KHZ)?2:1);
    // in case of 12Khz offset from base we need to adjust the fm_freq_mod_word

    for(int i = 0; i < blockSize; i++)
    {
        // Calculate next sample
        fm_mod_accum += (ulong)(fm_freq_mod_word + (adb.a_buffer[0][i] * FM_MOD_SCALING * fm_mod_mult));   // change frequency using scaled audio
        fm_mod_accum &= 0xffff;             // limit to 64k range
        fm_mod_idx    = fm_mod_accum >> FM_MOD_DDS_ACC_SHIFT;
        fm_mod_idx &= (DDS_TBL_SIZE - 1);       // limit lookup to range of sine table
        adb.i_buffer[i] = (float32_t)(DDS_TABLE[fm_mod_idx]);               // Load I value
        fm_mod_idx += (DDS_TBL_SIZE/4); // do 90 degree shift by indexing 1/4 into sine table
        fm_mod_idx &= (DDS_TBL_SIZE - 1);       // limit lookup to range of sine table
        adb.q_buffer[i] = (float32_t)(DDS_TABLE[fm_mod_idx]);   // Load Q value
    }

    bool swap = (iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ);

    AudioDriver_TxIqProcessingFinal(FM_MOD_AMPLITUDE_SCALING, swap, dst, blockSize);
}

#ifdef USE_FREEDV
// DO NOT USE, HAS NOT BEEN KEPT UP TO DATE WITH DEVELOPMENT IN audio_tx_processor!
//
// This is a stripped-down TX processor - work in progress
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_dv_tx_processor
//* Object              :
//* Object              : audio sample processor for DV modes - USB only, that must be reconfigured to operate at 8ksps -  - but this will require the future addition of a circular buffer and queueing in ISR!
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_TxProcessorDigital (AudioSample_t * const src, AudioSample_t * const dst, int16_t blockSize)
{
    // Freedv Test DL2FW
    static int16_t outbuff_count = 0;
    static int16_t trans_count_in = 0;
    static int16_t FDV_TX_fill_in_pt = 0;
    static FDV_IQ_Buffer* out_buffer = NULL;
    static int16_t modulus_NF = 0, modulus_MOD = 0;

    // If source is digital usb in, pull from USB buffer, discard line or mic audio and
    // let the normal processing happen

    if (ts.digital_mode == DigitalMode_FreeDV)
    { //we are in freedv-mode

        AudioDriver_TxAudioBufferFill(src,blockSize);

        // *****************************   DV Modulator goes here - ads.a_buffer must be at 8 ksps

        // Freedv Test DL2FW

        // we have to add a decimation filter here BEFORE we decimate
        // for decimation-by-6 the stopband frequency is 48/6*2 = 4kHz
        // but our audio is at most 3kHz wide, so we should use 3k or 2k9


        // this is the correct DECIMATION FILTER (before the downsampling takes place):
        // use it ALWAYS, also with TUNE tone!!!
        AudioDriver_TxFilterAudio(true,false, adb.a_buffer[0],adb.a_buffer[0], blockSize);


        // DOWNSAMPLING
        for (int k = 0; k < blockSize; k++)
        {
            if (k % 6 == modulus_NF)  //every 6th sample has to be catched -> downsampling by 6
            {
                fdv_audio_buff[FDV_TX_fill_in_pt].samples[trans_count_in] = ((int32_t)adb.a_buffer[0][k])/4;
                // FDV_TX_in_buff[FDV_TX_fill_in_pt].samples[trans_count_in] = 0; // transmit "silence"
                trans_count_in++;
            }
        }

        modulus_NF += 4; //  shift modulus to not loose any data while overlapping
        modulus_NF %= 6;//  reset modulus to 0 at modulus = 12

        if (trans_count_in == FDV_BUFFER_SIZE) //yes, we really hit exactly 320 - don't worry
        {
            //we have enough samples ready to start the FreeDV encoding

            fdv_audio_buffer_add(&fdv_audio_buff[FDV_TX_fill_in_pt]);
            //handshake to external function in ui.driver_thread
            trans_count_in = 0;

            FDV_TX_fill_in_pt++;
            FDV_TX_fill_in_pt %= FDV_BUFFER_AUDIO_NUM;
        }

        if (out_buffer == NULL && fdv_iq_has_data() > 1) {
            fdv_iq_buffer_remove(&out_buffer);
        }

        if (out_buffer != NULL) // freeDV encode has finished (running in ui_driver.c)?
        {

            // Best thing here would be to use the arm_fir_decimate function! Why?
            // --> we need phase linear filters, because we have to filter I & Q and preserve their phase relationship
            // IIR filters are power saving, but they do not care about phase, so useless at this point
            // FIR filters are phase linear, but need processor power
            // so we now use the decimation function that upsamples like the code below, BUT at the same time filters
            // (and the routine knows that it does not have to multiply with 0 while filtering: if we do upsampling and subsequent
            // filtering, the filter does not know that and multiplies with zero 5 out of six times --> very inefficient)
            // BUT: we cannot use the ARM function, because decimation factor (6) has to be an integer divide of
            // block size (which is 64 in our case --> 64 / 6 = non-integer!)

            // UPSAMPLING [by hand]
            for (int j = 0; j < blockSize; j++) //  now we are doing upsampling by 6
            {
                if (modulus_MOD == 0) // put in sample pair
                {
                    adb.i_buffer[j] = out_buffer->samples[outbuff_count].real; // + (sample_delta.real * (float32_t)modulus_MOD);
                    adb.q_buffer[j] = out_buffer->samples[outbuff_count].imag; // + (sample_delta.imag * (float32_t)modulus_MOD);
                }
                else // in 5 of 6 cases just stuff in zeros = zero-padding / zero-stuffing
                {
                    adb.i_buffer[j] = 0;
                    adb.q_buffer[j] = 0;

                }
                modulus_MOD++;
                if (modulus_MOD == 6)
                {
                    // last_sample.real = FDV_TX_out_buff[modem_buffer_offset].samples[outbuff_count].real;
                    // last_sample.imag = FDV_TX_out_buff[modem_buffer_offset].samples[outbuff_count].imag;


                    outbuff_count++;
                    modulus_MOD = 0;
                }
            }

            // Add interpolation filter here to suppress alias frequencies
            // we are upsampling from 8kHz to 48kHz, so we have to suppress all frequencies below 4kHz
            // our FreeDV signal is centred at 1500Hz ??? and is 1250Hz broad,
            // so a lowpass filter with cutoff frequency 2400Hz should be fine!

            // INTERPOLATION FILTER [after the interpolation has taken place]
            // the samples are now in adb.i_buffer and adb.q_buffer, so lets filter them
            arm_fir_f32(&Fir_TxFreeDV_Interpolate_I, adb.i_buffer, adb.i_buffer,blockSize);
            arm_fir_f32(&Fir_TxFreeDV_Interpolate_Q, adb.q_buffer, adb.q_buffer, blockSize);



        }
        else
        {
            profileEvent(FreeDVTXUnderrun);
            // memset(dst,0,blockSize*sizeof(*dst));
        }

        if (outbuff_count >= FDV_BUFFER_SIZE)
        {
            outbuff_count = 0;
            out_buffer = NULL;
            fdv_iq_buffer_remove(&out_buffer);
        }

        // apply I/Q amplitude & phase adjustments
        bool swap = ts.dmod_mode == DEMOD_USB || (ts.dmod_mode == DEMOD_DIGI && ts.digi_lsb == false);
        AudioDriver_TxIqProcessingFinal(20.0*SSB_GAIN_COMP, swap, dst, blockSize);
    }
    else
    {
        memset(dst,0,blockSize*sizeof(*dst));
        // Pause or inactivity
    }
}
#endif

static void AudioDriver_TxProcessorRtty(AudioSample_t * const dst, uint16_t blockSize)
{

	for (uint16_t idx =0; idx < blockSize; idx++)
	{
		adb.a_buffer[0][idx] = Rtty_Modulator_GenSample();
	}
    AudioDriver_TxFilterAudio(true,false, adb.a_buffer[0], adb.a_buffer[0], blockSize);
    AudioDriver_TxProcessorModulatorSSB(dst, blockSize, ts.iq_freq_mode, ts.digi_lsb);

    // remove noise if no CW is keyed
    memset(adb.a_buffer[0],0,sizeof(adb.a_buffer[0][0])*blockSize);

    if (ts.cw_keyer_mode != CW_KEYER_MODE_STRAIGHT)
    {
    	CwGen_Process(adb.a_buffer[0], adb.a_buffer[0], blockSize);
    	// we just misuse adb.a_buffer[0], and generate a CW side tone
    }

}

static void AudioDriver_TxProcessorPsk(AudioSample_t * const dst, uint16_t blockSize)
{

	for (uint16_t idx =0; idx < blockSize; idx++)
	{
		adb.a_buffer[0][idx] = Psk_Modulator_GenSample();
	}

    AudioDriver_TxFilterAudio(true,false, adb.a_buffer[0], adb.a_buffer[0], blockSize);
	AudioDriver_TxProcessorModulatorSSB(dst, blockSize, false, false);
/*
    memset(adb.a_buffer[0],0,sizeof(adb.a_buffer[0])*blockSize);

    if (ts.cw_keyer_mode != CW_KEYER_MODE_STRAIGHT)
    {
    	CwGen_Process(adb.a_buffer[0], adb.a_buffer[0], blockSize);
    	// we just misuse adb.a_buffer[0], and generate a CW side tone
    }
*/
}

static void AudioDriver_TxProcessor(AudioSample_t * const srcCodec, AudioSample_t * const dst, AudioSample_t * const audioDst, uint16_t blockSize)
{
    // we copy volatile variables which are used multiple times to local consts to let the compiler do its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t tx_audio_source = ts.tx_audio_source;
    const uint8_t tune = ts.tune;
    const uint8_t iq_freq_mode = ts.iq_freq_mode;
    AudioSample_t srcUSB[blockSize];
    AudioSample_t * const src = (tx_audio_source == TX_AUDIO_DIG || tx_audio_source == TX_AUDIO_DIGIQ) ? srcUSB : srcCodec;

    // if we want to know if our signal will go out, look at this flag
    bool external_tx_mute = ts.audio_dac_muting_flag || ts.audio_dac_muting_buffer_count > 0;

    bool signal_active = false; // unless this is set to true, zero output will be generated
    bool cw_signal_active = false;

    // If source is digital usb in, pull from USB buffer, discard line or mic audio and
    // let the normal processing happen
    if (tx_audio_source == TX_AUDIO_DIG || tx_audio_source == TX_AUDIO_DIGIQ)
    {
        // FIXME: change type of audio_out_fill_tx_buffer to use audio sample struct
        audio_out_fill_tx_buffer((int16_t*)srcUSB,2*blockSize);
    }

    if (tx_audio_source == TX_AUDIO_DIGIQ && dmod_mode != DEMOD_CW && !tune && !is_demod_psk())
    {
        // If in CW mode or Tune  DIQ audio input is ignored
        // Output I and Q as stereo, fill buffer
        for(int i = 0; i < blockSize; i++)	 				// Copy to single buffer
        {
            adb.i_buffer[i] = src[i].l;
            adb.q_buffer[i] = src[i].r;
        }

        AudioDriver_TxIqProcessingFinal(1.0, false, dst, blockSize);
        signal_active = true;
    }
    else if (ts.dvmode == true)
    {
    	switch(ts.digital_mode)
    	{
#ifdef USE_FREEDV
    	case DigitalMode_FreeDV:
        {
            AudioDriver_TxProcessorDigital(src,dst,blockSize);
            signal_active = true;
        }
        break;
#endif
    	case DigitalMode_RTTY:
    	{
            AudioDriver_TxProcessorRtty(dst,blockSize);
            signal_active = true;
    	}
    	break;
    	case DigitalMode_BPSK:
    	{
    		AudioDriver_TxProcessorPsk(dst,blockSize);
    		signal_active = true;
    	}
    	break;
    	}
    }
    else if(dmod_mode == DEMOD_CW || ts.cw_text_entry)
    {
        if (tune)
        {
            softdds_runIQ(adb.i_buffer, adb.q_buffer, blockSize);      // generate tone/modulation for TUNE
            // Equalize based on band and simultaneously apply I/Q gain & phase adjustments
            signal_active = true;
        }
        else
        {
        	cw_signal_active = CwGen_Process(adb.i_buffer, adb.q_buffer, blockSize);
            // Generate CW tone if necessary
            if (external_tx_mute == false)
            {
            	signal_active = cw_signal_active;
            }
        }

        if (signal_active)
        {
            // apply I/Q amplitude & phase adjustments
            // Wouldn't it be necessary to include IF conversion here? DD4WH June 16th, 2016
            // Answer: NO, in CW that is done be changing the Si570 frequency during TX/RX switching . . .
            AudioDriver_TxIqProcessingFinal(1.0, ts.cw_lsb == 0, dst, blockSize);
        }
    }
    // SSB processor
    else if(is_ssb(dmod_mode))
    {
        if (ads.tx_filter_adjusting == false)
        {
            AudioDriver_TxAudioBufferFill(src,blockSize);

            if (!tune)
            {
                AudioDriver_TxFilterAudio(true,tx_audio_source != TX_AUDIO_DIG, adb.a_buffer[0], adb.a_buffer[0], blockSize);
            }

            // Do the TX ALC and speech compression/processing
            AudioDriver_TxCompressor(adb.a_buffer[0], blockSize, SSB_ALC_GAIN_CORRECTION);

            AudioDriver_TxProcessorModulatorSSB(dst, blockSize, iq_freq_mode, dmod_mode == DEMOD_LSB);
            signal_active = true;
        }
    }
    // -----------------------------
    // AM handler - Generate USB and LSB AM signals and combine  [KA7OEI]
    //
    else if(dmod_mode == DEMOD_AM)	 	//	Is it in AM mode *AND* is frequency translation active?
    {
        if(iq_freq_mode && ads.tx_filter_adjusting == false)	 				// is translation active?
        {
            AudioDriver_TxAudioBufferFill(src,blockSize);
            //
            // Apply the TX equalization filtering:  This "flattens" the audio
            // prior to being applied to the Hilbert transformer as well as added low-pass filtering.
            // It does this by applying a "peak" to the bottom end to compensate for the roll-off caused by the Hilbert
            // and then a gradual roll-off toward the high end.  The net result is a very flat (to better than 1dB) response
            // over the 275-2500 Hz range.
            //
            if (!tune)
            {
                AudioDriver_TxFilterAudio((ts.flags1 & FLAGS1_AM_TX_FILTER_DISABLE) == false,tx_audio_source != TX_AUDIO_DIG, adb.a_buffer[0], adb.a_buffer[0], blockSize);
            }
            //
            // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
            // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
            // Apply transformation AND audio filtering to buffer data
            //
            // + 0 deg to I data
            // AudioDriver_delay_f32((arm_fir_instance_f32 *)&FIR_I_TX,(float32_t *)(adb.a_buffer[0]),(float32_t *)(adb.i_buffer),blockSize);

            AudioDriver_TxCompressor(adb.a_buffer[0], blockSize, AM_ALC_GAIN_CORRECTION);    // Do the TX ALC and speech compression/processing

            arm_fir_f32(&Fir_Tx_Hilbert_I, adb.a_buffer[0], adb.i_buffer, blockSize);
            // - 90 deg to Q data
            arm_fir_f32(&Fir_Tx_Hilbert_Q, adb.a_buffer[0], adb.q_buffer, blockSize);

            // COMMENT:  It would be trivial to add the option of generating AM with just a single (Upper or Lower) sideband since we are generating the two, separately anyway
            // and putting them back together!  [KA7OEI]
            //
            //
            // First, generate the LOWER sideband of the AM signal

            // temporary buffers;
            float32_t  i2_buffer[blockSize];
            float32_t  q2_buffer[blockSize];


            arm_negate_f32(adb.i_buffer, q2_buffer, blockSize); // this becomes the q buffer for the upper  sideband
            arm_negate_f32(adb.q_buffer, i2_buffer, blockSize); // this becomes the i buffer for the upper  sideband

            // now generate USB AM sideband signal
            AudioDriver_TxProcessorAMSideband(adb.i_buffer, adb.q_buffer, blockSize);
            // i/q now contain the LSB AM signal

            // now generate USB AM sideband signal
            AudioDriver_TxProcessorAMSideband(i2_buffer, q2_buffer, blockSize);

            arm_add_f32(i2_buffer, adb.i_buffer, adb.i_buffer,blockSize);
            arm_add_f32(q2_buffer, adb.q_buffer, adb.q_buffer,blockSize);

            AudioDriver_TxIqProcessingFinal(AM_GAIN_COMP, false, dst, blockSize);
            signal_active = true;
        }
    }
    else if(dmod_mode == DEMOD_FM)	 	//	Is it in FM mode
    {
        //  *AND* is frequency translation active (No FM possible unless in frequency translate mode!)
        if (iq_freq_mode)
        {
            // FM handler  [KA7OEI October, 2015]
            AudioDriver_TxProcessorFM(src,dst,blockSize);
            signal_active = true;
        }
    }

    if (signal_active == false  || external_tx_mute )
    {
        memset(dst,0,blockSize*sizeof(*dst));
        // Pause or inactivity
        if (ts.audio_dac_muting_buffer_count)
        {
            ts.audio_dac_muting_buffer_count--;
        }
    }

#ifdef UI_BRD_OVI40
    // we code the sidetone to the audio codec, since we have one for audio and one for iq
    if (audioDst != dst)
    {
    	if (dmod_mode == DEMOD_CW)
    	{
    		for (uint16_t idx = 0; idx < blockSize; idx++)
    		{
    			audioDst[idx].r = audioDst[idx].l = dst[idx].l;
    		}
    	} else if (is_demod_rtty())
    	{
    		for (uint16_t idx = 0; idx < blockSize; idx++)
    		{
    			// we simulate the effect of the SSB tx iq processing to get a similar audio volume
    			// as the CW sidetone
    			audioDst[idx].r = audioDst[idx].l = adb.a_buffer[0][idx] * ts.tx_power_factor;
    		}
    	}
    }
#endif

    switch (ts.stream_tx_audio)
    {
    case STREAM_TX_AUDIO_OFF:
        break;
    case STREAM_TX_AUDIO_DIGIQ:
        for(int i = 0; i < blockSize; i++)
        {

            // 16 bit format - convert to float and increment
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            audio_in_put_buffer(dst[i].r);
            audio_in_put_buffer(dst[i].l);
        }
        break;
    case STREAM_TX_AUDIO_SRC:
        for(int i = 0; i < blockSize; i++)
        {
            // 16 bit format - convert to float and increment
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            audio_in_put_buffer(src[i].r);
            audio_in_put_buffer(src[i].l);
        }
        break;
    case STREAM_TX_AUDIO_FILT:
        for(int i = 0; i < blockSize; i++)
        {
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIG
            // TODO: certain modulation modes will destroy the "a_buffer" during IQ signal creation (AM does at least)
            audio_in_put_buffer(adb.a_buffer[0][i]);
            audio_in_put_buffer(adb.a_buffer[0][i]);
        }
    }
}


//*----------------------------------------------------------------------------
//* Function Name       : I2S_RX_CallBack
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
#ifdef USE_24_BITS
void AudioDriver_I2SCallback(int32_t *src, int32_t *dst, int16_t size, uint16_t ht)
#else
void AudioDriver_I2SCallback(int16_t *src, int16_t *dst, int16_t* audioDst, int16_t size)
#endif
{
    static bool to_rx = false;	// used as a flag to clear the RX buffer
    static bool to_tx = false;	// used as a flag to clear the TX buffer
    static ulong tcount = 0;
    bool muted = false;

    const int16_t blockSize = size/2;

    if(ts.show_debug_info)
    {
        Board_GreenLed(LED_STATE_ON);
    }

    if((ts.txrx_mode == TRX_MODE_RX))
    {
        if((to_rx) || ts.audio_processor_input_mute_counter > 0)	 	// the first time back to RX, clear the buffers to reduce the "crash"
        {
            muted = true;
            arm_fill_q15(0, src, size);
            if (to_rx)
            {
                AudioDriver_ClearAudioDelayBuffer();
                UhsdrHwI2s_Codec_ClearTxDmaBuffer();
            }
            if ( ts.audio_processor_input_mute_counter >0)
            {
                ts.audio_processor_input_mute_counter--;
            }
            to_rx = false;                          // caused by the content of the buffers from TX - used on return from SSB TX
        }

        if (muted)
        {
            // muted input should not modify the ALC so we simply restore it after processing
//            float agc_holder = ads.agc_val;
            bool dsp_inhibit_holder = ts.dsp_inhibit;
            AudioDriver_RxProcessor((AudioSample_t*) src, (AudioSample_t*)dst,blockSize);
//            ads.agc_val = agc_holder;
            ts.dsp_inhibit = dsp_inhibit_holder;
        }
        else
        {
            AudioDriver_RxProcessor((AudioSample_t*) src, (AudioSample_t*)dst,blockSize);
            if (ts.cw_keyer_mode != CW_KEYER_MODE_STRAIGHT && (ts.cw_text_entry || ts.dmod_mode == DEMOD_CW)) // FIXME to call always when straight mode reworked
            {
            	CwGen_Process(adb.i_buffer, adb.q_buffer, blockSize);
            }
        }

        to_tx = true;		// Set flag to indicate that we WERE receiving when we go back to transmit mode
    }
    else  			// Transmit mode
    {
        if((to_tx) || (ts.audio_processor_input_mute_counter>0))	 	// the first time back to TX, or TX audio muting timer still active - clear the buffers to reduce the "crash"
        {
            muted = true;
            arm_fill_q15(0, src, size);
            to_tx = false;                          // caused by the content of the buffers from TX - used on return from SSB TX
            if ( ts.audio_processor_input_mute_counter >0)
            {
                ts.audio_processor_input_mute_counter--;
            }
        }

        if (muted)
        {
            // muted input should not modify the ALC so we simply restore it after processing
            float alc_holder = ads.alc_val;
            AudioDriver_TxProcessor((AudioSample_t*) src, (AudioSample_t*)dst, (AudioSample_t*)audioDst,blockSize);
            ads.alc_val = alc_holder;
        }
        else
        {
            AudioDriver_TxProcessor((AudioSample_t*) src, (AudioSample_t*)dst, (AudioSample_t*)audioDst, blockSize);
        }

        to_rx = true;		// Set flag to indicate that we WERE transmitting when we eventually go back to receive mode
    }

    if(ts.audio_spkr_unmute_delay_count)		// this updates at 1.5 kHz - used to time TX->RX delay
    {
        ts.audio_spkr_unmute_delay_count--;
    }

    if(ks.debounce_time < DEBOUNCE_TIME_MAX)
    {
        ks.debounce_time++;   // keyboard debounce timer
    }

    // Perform LCD backlight PWM brightness function
    UiDriver_BacklightDimHandler();

    tcount+=CLOCKS_PER_DMA_CYCLE;		// add the number of clock cycles that would have passed between DMA cycles
    if(tcount > CLOCKS_PER_CENTISECOND)	 	// has enough clock cycles for 0.01 second passed?
    {
        tcount -= CLOCKS_PER_CENTISECOND;	// yes - subtract that many clock cycles
        ts.sysclock++;	// this clock updates at PRECISELY 100 Hz over the long term
        //
        // Has the timing for the keyboard beep expired?

        if(ts.sysclock > ts.beep_timing)
        {
            ts.beep_active = 0;				// yes, turn the tone off
            ts.beep_timing = 0;
        }
    }

    if(ts.scope_scheduler)		// update thread timer if non-zero
    {
        ts.scope_scheduler--;
    }

    if(ts.waterfall.scheduler)      // update thread timer if non-zero
    {
        ts.waterfall.scheduler--;
    }

    if(ts.show_debug_info)
    {
        Board_GreenLed(LED_STATE_OFF);
    }

#ifdef USE_PENDSV_FOR_HIGHPRIO_TASKS
    // let us trigger a pendsv irq here in order to trigger execution of UiDriver_HighPrioHandler()
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
#endif
}


