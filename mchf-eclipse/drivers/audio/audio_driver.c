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
#include "audio_agc.h"
#include "audio_management.h"
#include "radio_management.h"
#include "usbd_audio_if.h"
#include "ui_spectrum.h"
#include "filters.h"
#include "uhsdr_hw_i2s.h"
#include "rtty.h"
#include "psk.h"
#include "cw_decoder.h"
#include "freedv_uhsdr.h"
#include "freq_shift.h"
#include "audio_nr.h"
#ifdef USE_CONVOLUTION
#include "audio_convolution.h"
#endif

#include "fm_subaudible_tone_table.h" // hm.
#include "uhsdr_math.h"
#include "tx_processor.h"

// SSB filters - now handled in ui_driver to allow I/Q phase adjustment

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
static arm_biquad_casd_df1_inst_f32 IIR_biquad_1[NUM_AUDIO_CHANNELS] =
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
static arm_biquad_casd_df1_inst_f32 IIR_biquad_2[NUM_AUDIO_CHANNELS] =
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
        NULL,

        (float32_t*)(const float32_t[]) {
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
            -0.930973052446900984
        },

        (float32_t*)(const float32_t[])
        {
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
            -0.950935065984588657
        },

        (float32_t*)(const float32_t[])
        {
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
            -0.973184930412286708
        },

        (float32_t*)(const float32_t[])
        {
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
            -0.986288064973853129
        },

        (float32_t*)(const float32_t[])
        {
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
            -0.993055129539134551
        }
};

#ifdef USE_SIMPLE_FREEDV_FILTERS
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
#endif

//******* End of 2 set of filters for the I/Q FreeDV aliasing filter**********

// S meter public
SMeter	sm;

// ATTENTION: These data structures have been placed in CCM Memory (64k)
// IF THE SIZE OF  THE DATA STRUCTURE GROWS IT WILL QUICKLY BE OUT OF SPACE IN CCM
// Be careful! Check mchf-eclipse.map for current allocation
AudioDriverState   __MCHF_SPECIALMEM ads;
AudioDriverBuffer  __MCHF_SPECIALMEM adb;

#if defined(USE_LMS_AUTONOTCH)
LMSData            __MCHF_SPECIALMEM lmsData;
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


// RX variables for FM squelch IIR filters
static float32_t    iir_squelch_rx_state[IIR_RX_STATE_ARRAY_SIZE];
static arm_iir_lattice_instance_f32 IIR_Squelch_HPF;

static void AudioDriver_FM_Rx_Init(fm_conf_t* fm)
{
    // RX
    fm->sql_avg = 0;         // init FM squelch averaging
    fm->squelched = true;    // TRUE if FM receiver audio is to be squelched, we start squelched.

    fm->subaudible_tone_detected = false; // TRUE if subaudible tone has been detected
    AudioManagement_CalcSubaudibleDetFreq(fm_subaudible_tone_table[ts.fm_subaudible_tone_det_select]);        // RX load/set current FM subaudible tone settings for detection

    // Initialize high-pass filter used for the FM noise squelch
    IIR_Squelch_HPF.pkCoeffs = IIR_15k_hpf.pkCoeffs;   // point to reflection coefficients
    IIR_Squelch_HPF.pvCoeffs = IIR_15k_hpf.pvCoeffs;   // point to ladder coefficients
    IIR_Squelch_HPF.numStages = IIR_15k_hpf.numStages; // number of stages
    IIR_Squelch_HPF.pState = iir_squelch_rx_state;     // point to state array for IIR filter
    arm_fill_f32(0.0,iir_squelch_rx_state,IIR_RX_STATE_ARRAY_SIZE);
}

#ifdef USE_LEAKY_LMS
static void AudioDriver_LeakyLmsNr_Init()
{
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

}

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
//          leakyLMS.out_buff[2 * i + 1] = 0.0;

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

static void AudioDriver_Dsp_Init(volatile dsp_params_t* dsp_p)
{

    dsp_p->active_toggle    = 0xff;                 // used to hold the button G2 "toggle" setting.

    // Commented settings below are read from configuration store, no need to initialize them
    // dsp_p->active       = 0;                    // TRUE if DSP noise reduction is to be enabled
    // dsp_p->nr_strength  = 50;                   // "Strength" of DSP noise reduction (50 = medium)
#ifdef USE_LMS_AUTONOTCH
    // dsp_p->notch_numtaps = DSP_NOTCH_NUMTAPS_DEFAULT;       // default for number of FFT taps for notch filter
    // dsp_p->notch_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
    // dsp_p->notch_mu = DSP_NOTCH_MU_DEFAULT;
#endif
    // dsp_p->notch_frequency = 800;               // notch start frequency for manual notch filter
    // dsp_p->peak_frequency = 750;                // peak start frequency
    // dsp_p->nb_setting       = 0;                    // Noise Blanker setting

    // dsp_p->bass_gain = 2;                       // gain of the low shelf EQ filter
    // dsp_p->treble_gain = 0;                     // gain of the high shelf EQ filter
    // dsp_p->tx_bass_gain = 4;                    // gain of the TX low shelf EQ filter
    // dsp_p->tx_treble_gain = 4;                  // gain of the TX high shelf EQ filter
}
/**
 * One-time init of FreeDV codec's audio driver part, mostly filters
 */

#define FIR_RX_HILBERT_STATE_SIZE (IQ_RX_NUM_TAPS_MAX + IQ_RX_BLOCK_SIZE)
#ifndef USE_SIMPLE_FREEDV_FILTERS
static float32_t    __MCHF_SPECIALMEM Fir_FreeDV_Rx_Hilbert_State_I[FIR_RX_HILBERT_STATE_SIZE];
static float32_t    __MCHF_SPECIALMEM Fir_FreeDV_Rx_Hilbert_State_Q[FIR_RX_HILBERT_STATE_SIZE];
static arm_fir_instance_f32    Fir_FreeDV_Rx_Hilbert_I;
static arm_fir_instance_f32    Fir_FreeDV_Rx_Hilbert_Q;
#endif

static void AudioDriver_FreeDV_Rx_Init()
{
#ifdef USE_SIMPLE_FREEDV_FILTERS
    IIR_biquad_FreeDV_I.pCoeffs = FreeDV_coeffs[0];  // FreeDV Filter test -DL2FW-
    IIR_biquad_FreeDV_Q.pCoeffs = FreeDV_coeffs[0];
#else
#ifdef STM32F4
    arm_fir_init_f32(&Fir_FreeDV_Rx_Hilbert_I,IQ_RX_NUM_TAPS_LO,(float32_t*)i_rx_FREEDV_700D_F4_coeffs,Fir_FreeDV_Rx_Hilbert_State_I, IQ_BLOCK_SIZE);
    arm_fir_init_f32(&Fir_FreeDV_Rx_Hilbert_Q,IQ_RX_NUM_TAPS_LO,(float32_t*)q_rx_FREEDV_700D_F4_coeffs,Fir_FreeDV_Rx_Hilbert_State_Q, IQ_BLOCK_SIZE);
#else
    arm_fir_init_f32(&Fir_FreeDV_Rx_Hilbert_I,IQ_RX_NUM_TAPS,(float32_t*)i_rx_FREEDV_700D_coeffs,Fir_FreeDV_Rx_Hilbert_State_I, IQ_BLOCK_SIZE);
    arm_fir_init_f32(&Fir_FreeDV_Rx_Hilbert_Q,IQ_RX_NUM_TAPS,(float32_t*)q_rx_FREEDV_700D_coeffs,Fir_FreeDV_Rx_Hilbert_State_Q, IQ_BLOCK_SIZE);
#endif
#endif
}

void AudioDriver_AgcWdsp_Set()
{
    AudioAgc_SetupAgcWdsp(ads.decimated_freq, ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_SAM);
}

/**
 * Initializes rx audio related data structures, must be called before audio interrupt becomes active
 *
 * Called once during startup by AudioDriver_Init
 *
 */
static void RxProcessor_Init()
{
    AudioAgc_AgcWdsp_Init(); // RX

    NR_Init(); // RX

    // also belongs to the NR audio, this is our preprocessing / postprocessing
    // never changes, so we place it here
    // Set up RX decimation/filter
    // this filter instance is also used for Convolution !
    arm_fir_decimate_init_f32(&DECIMATE_NR, 4, 2, NR_decimate_coeffs, decimNRState, FIR_RXAUDIO_BLOCK_SIZE);
    // should be a very light lowpass @2k7

    // this filter instance is also used for Convolution !
    arm_fir_interpolate_init_f32(&INTERPOLATE_NR, 2, NR_INTERPOLATE_NO_TAPS, NR_interpolate_coeffs, interplNRState, FIR_RXAUDIO_BLOCK_SIZE);
    // should be a very light lowpass @2k7


#ifdef USE_LEAKY_LMS
    AudioDriver_LeakyLmsNr_Init(); // RX
#endif

    AudioDriver_FreeDV_Rx_Init();  // RX
    AudioDriver_FM_Rx_Init(&ads.fm_conf); // RX
    //    ads.fade_leveler = 0;
}

/**
 * Initializes most of the audio related data structures, must be called before audio interrupt becomes active
 * DO NOT ACTIVATE THE INTERRUPT BEFORE AudioDriver_SetProcessingChain() has been called, which handles the dynamic part
 * of the initialization based on the mode and filter settings etc.
 *
 * Do all one time initialization for codecs, (de)modulators, dsp, ... , from here. Nothing here should depend on a specific
 * dmod_mode being set. All dmod_mode specific setup/init must be done in AudioDriver_SetProcessingChain() or related functions
 *
 * Called once during startup
 *
 */
void AudioDriver_Init()
{
    // Audio filter disabled
    // although we are not have the interrupt running, some of the called code may
    // refuse to work if dsp.inhibit and/or af_disabled is 0
    ts.dsp.inhibit++;
    ads.af_disabled++;

    // DSP related init
    AudioDriver_Dsp_Init(&ts.dsp); // RX/TX

    // AUDIO KEY BEEPS (injected into audio output stream)
    AudioManagement_KeyBeepPrepare();  // load/set beep frequency

    // Codecs/Demod init
    Rtty_Modem_Init(ts.samp_rate); // RX/TX
    Psk_Modem_Init(ts.samp_rate);  // RX/TX

    RxProcessor_Init();
    TxProcessor_Init();

    // Audio filter enabled
    ads.af_disabled--;
    ts.dsp.inhibit--;

    assert(ads.af_disabled == 0);
    assert(ts.dsp.inhibit == 0);
}

/**
 * Configures SAM PLL according to settings
 */
void AudioDriver_SetSamPllParameters()
{

    // definitions and intializations for synchronous AM demodulation = SAM
    const float32_t decimSampleRate = ads.decimated_freq;
    const float32_t pll_fmax = ads.pll_fmax_int;

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

    float32_t omegaN = ads.omegaN_int; //200.0; // PLL bandwidth 50.0 - 1000.0
    float32_t zeta = (float32_t)ads.zeta_int / 100.0; // 0.01;// 0.001; // 0.1; //0.65; // PLL step response: smaller, slower response 1.0 - 0.1

    //pll
    adb.sam.omega_min = - (2.0 * PI * pll_fmax / decimSampleRate);
    adb.sam.omega_max = (2.0 * PI * pll_fmax / decimSampleRate);
    adb.sam.g1 = (1.0 - expf(-2.0 * omegaN * zeta / decimSampleRate));
    adb.sam.g2 = (- adb.sam.g1 + 2.0 * (1 - expf(- omegaN * zeta / decimSampleRate)
            * cosf(omegaN / decimSampleRate * sqrtf(1.0 - zeta * zeta)))); //0.01036367597097734813032783691644; //

    //fade leveler
    float32_t tauR = 0.02; // value emperically determined
    float32_t tauI = 1.4;  // value emperically determined
    adb.sam.mtauR = (expf(- 1 / (decimSampleRate * tauR)));
    adb.sam.onem_mtauR = (1.0 - adb.sam.mtauR);
    adb.sam.mtauI = (expf(- 1 / (decimSampleRate * tauI)));
    adb.sam.onem_mtauI = (1.0 - adb.sam.mtauI);
}

static void AudioDriver_SetRxIqCorrection()
{
    // these change during operation
    adb.iq_corr.M_c1 = 0.0;
    adb.iq_corr.M_c2 = 1.0;
    adb.iq_corr.teta1_old = 0.0;
    adb.iq_corr.teta2_old = 0.0;
    adb.iq_corr.teta3_old = 0.0;
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
    f0 = ts.dsp.peak_frequency;
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
    f0 = ts.dsp.peak_frequency;
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

#if 0
/**
 * @brief Biquad Filter Init Helper function to calculate a notch filter aka narrow bandstop filter with variable bandwidth
 */
static void AudioDriver_CalcNotch(float32_t coeffs[5], float32_t f0, float32_t BW, float32_t FS)
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
#endif

static const float32_t biquad_passthrough[] = { 1, 0, 0, 0, 0 };


/**
 * @brief Filter Init used for processing audio for RX and TX
 */
static void AudioDriver_SetRxTxAudioProcessingAudioFilters(uint8_t dmod_mode)
{
    float32_t FSdec = AUDIO_SAMPLE_RATE / (ts.filters_p->sample_rate_dec != 0 ? ts.filters_p->sample_rate_dec : 1) ;

    // the notch filter is in biquad 1 and works at the decimated sample rate FSdec

    float32_t coeffs[5];
    const float32_t *coeffs_ptr;

    // setting the Coefficients in the notch filter instance
    if (is_dsp_mnotch())
    {
        AudioDriver_CalcBandstop(coeffs, ts.dsp.notch_frequency , FSdec);
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
    //    if((is_dsp_notch()) && (ts.filters_p->sample_rate_dec == RX_DECIMATION_RATE_12KHZ))

    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 15, biquad_passthrough);

    // the peak filter is in biquad 1 and works at the decimated sample rate FSdec
    if(is_dsp_mpeak())
    {
        AudioDriver_CalcBandpass(coeffs, ts.dsp.peak_frequency, FSdec);
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
    AudioDriver_CalcLowShelf(coeffs, 250, 0.7, ts.dsp.bass_gain, FSdec);
    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 10, coeffs);

    // Treble = highShelf
    // the treble filter is in biquad 2 and works at 48000ksps
    AudioDriver_CalcHighShelf(coeffs, 3500, 0.9, ts.dsp.treble_gain, AUDIO_SAMPLE_RATE);
    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_2, 0, coeffs);

    TxProcessor_Set(dmod_mode);
}

/**
 * Sets up the spectrum filters according to the current zoom level
 */
static void AudioDriver_Spectrum_Set()
{
    // this sets the coefficients for the ZoomFFT decimation filter
    // according to the desired magnification mode sd.magnify.
    // The magnification is 2^sd.magnify (0 - 5 -> 1x - 32x)
    if(sd.magnify > MAGNIFY_MAX)
    {
        sd.magnify = MAGNIFY_MIN;
    }

    // for 0 the mag_coeffs will a NULL  ptr, since the filter is not going to be used in this  mode!
    IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[sd.magnify];
    IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[sd.magnify];


    // Set up ZOOM FFT FIR decimation filters
    // switch right FIR decimation filter depending on sd.magnify

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
}

/**
 * @brief configures filters/dsp etc. so that audio processing works according to the current configuration
 * @param dmod_mode needs to know the demodulation mode
 * @param reset_dsp_nr whether it is supposed to reset also DSP related filters (in most cases false is to be used here)
 */
void AudioDriver_SetProcessingChain(uint8_t dmod_mode, bool reset_dsp_nr)
{
    ts.dsp.inhibit++;
    ads.af_disabled++;

    // make sure we have a proper filter path for the given mode

    // the commented out part made the code  only look at last used/selected filter path if the current filter path is not applicable
    // with it commented out the filter path is ALWAYS loaded from the last used/selected memory
    // I.e. setting the ts.filter_path anywere else in the code is useless. You have to call AudioFilter_NextApplicableFilterPath in order to
    // select a new filter path as this sets the last used/selected memory for a demod mode.

    ts.filter_path = AudioFilter_NextApplicableFilterPath(PATH_ALL_APPLICABLE|PATH_LAST_USED_IN_MODE,AudioFilter_GetFilterModeFromDemodMode(dmod_mode),ts.filter_path);
    const FilterPathDescriptor* filters_p  = ts.filters_p = &FilterPathInfo[ts.filter_path];

    // Adjust decimation rate based on selected filter
    ads.decimation_rate = filters_p->sample_rate_dec;
    ads.decimated_freq = IQ_SAMPLE_RATE / ads.decimation_rate;

    for (int chan = 0; chan < NUM_AUDIO_CHANNELS; chan++)
    {
        if (ts.filters_p->pre_instance != NULL)
        {
            // if we turn on a filter, set the number of members to the number of elements last
            IIR_PreFilter[chan].pkCoeffs = filters_p->pre_instance->pkCoeffs; // point to reflection coefficients
            IIR_PreFilter[chan].pvCoeffs = filters_p->pre_instance->pvCoeffs; // point to ladder coefficients
            IIR_PreFilter[chan].numStages = filters_p->pre_instance->numStages;        // number of stages
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
        if (filters_p->iir_instance != NULL)
        {
            // if we turn on a filter, set the number of members to the number of elements last
            IIR_AntiAlias[chan].pkCoeffs = filters_p->iir_instance->pkCoeffs; // point to reflection coefficients
            IIR_AntiAlias[chan].pvCoeffs = filters_p->iir_instance->pvCoeffs; // point to ladder coefficients
            IIR_AntiAlias[chan].numStages = filters_p->iir_instance->numStages;        // number of stages
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
    AudioDriver_SetRxTxAudioProcessingAudioFilters(dmod_mode);

    // initialize the goertzel filter used to detect CW signals at a given frequency in the audio stream
    CwDecode_Filter_Set();


    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * End of coefficient calculation and setting for cascaded biquad
     ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifdef USE_LMS_AUTONOTCH
    // AUTO NOTCH INIT START
    // LMS instance 2 - Automatic Notch Filter

    // Calculate "mu" (convergence rate) from user "Notch ConvRate" setting
    float32_t  mu_calc = log10f(((ts.dsp.notch_mu + 1.0)/1500.0) + 1.0);		// get user setting (0 = slowest)

    // use "canned" init to initialize the filter coefficients
    arm_lms_norm_init_f32(&lmsData.lms2Norm_instance, ts.dsp.notch_numtaps, lmsData.lms2NormCoeff_f32, lmsData.lms2StateF32, mu_calc, IQ_BLOCK_SIZE);

    arm_fill_f32(0.0,lmsData.lms2_nr_delay,DSP_NOTCH_BUFLEN_MAX);

    if(reset_dsp_nr)             // are we to reset the coefficient buffer as well?
    {
        arm_fill_f32(0.0,lmsData.lms2NormCoeff_f32,DSP_NOTCH_NUMTAPS_MAX);      // yes - zero coefficient buffers
    }

    if((ts.dsp.notch_delaybuf_len > DSP_NOTCH_BUFLEN_MAX) || (ts.dsp.notch_delaybuf_len < DSP_NOTCH_BUFLEN_MIN))
    {
        ts.dsp.notch_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
    }
    // AUTO NOTCH INIT END
#endif


// NEW SPECTRAL NOISE REDUCTION
    // convert user setting of noise reduction to alpha NR parameter
    // alpha ranges from 0.9 to 0.999 [float32_t]
    // dsp_nr_strength is from 0 to 100 [uint8_t]
    //    nr_params.alpha = 0.899 + ((float32_t)ts.dsp.nr_strength / 1000.0);
    nr_params.alpha = 0.799 + ((float32_t)ts.dsp.nr_strength / 1000.0);

// NEW AUTONOTCH
    // not yet working !
    // set to passthrough
    //AudioNr_ActivateAutoNotch(0, 0);

    AudioDriver_Spectrum_Set();

#ifdef USE_CONVOLUTION
// TODO: insert interpolation filter settings for convolution filter HERE
#else
    // Set up RX interpolation/filter
    // NOTE:  Phase Length MUST be an INTEGER and is the number of taps divided by the decimation rate, and it must be greater than 1.
    for (int chan = 0; chan < NUM_AUDIO_CHANNELS; chan++)
    {
        if (filters_p->interpolate != NULL)
        {
            arm_fir_interpolate_init_f32(&INTERPOLATE_RX[chan],
                    ads.decimation_rate,
                    filters_p->interpolate->phaseLength,
                    filters_p->interpolate->pCoeffs,
                    interpState[chan], IQ_BLOCK_SIZE);
        }
        else
        {
            INTERPOLATE_RX[chan].phaseLength = 0;
            INTERPOLATE_RX[chan].pCoeffs = NULL;
        }
    }
#endif

#ifdef USE_CONVOLUTION
    // Convolution Filter
    // calculate coeffs
    // for first trial, use hard-coded USB filter from 250Hz to 2700Hz
    // hardcoded sample rate and Blackman-Harris 4th term
    cbs.size = 128; // 128 samples processed at one time, this defines the latency:
    // latency = cbs.size / sample rate = 128/48000 = 2.7ms
    cbs.nc = 1024; // use 1024 coefficients
    cbs.nfor = cbs.nc / cbs.size; // number of blocks used for the uniformly partitioned convolution
    cbs.buffidx = 0; // needs to be reset to zero each time new coeffs are being calculated
    AudioDriver_CalcConvolutionFilterCoeffs (cbs.nc, 250.0, 2700.0, IQ_SAMPLE_RATE_F, 0, 1, 1.0);
#endif

    AudioDriver_SetSamPllParameters();
    AudioDriver_SetRxIqCorrection();

    AudioFilter_SetRxHilbertAndDecimationFIR(dmod_mode); // this switches the Hilbert/FIR-filters

    AudioDriver_AgcWdsp_Set();

    // Unlock - re-enable filtering
    if  (ads.af_disabled) { ads.af_disabled--; }
    if (ts.dsp.inhibit) { ts.dsp.inhibit--; }

}



#if USE_OBSOLETE_NOISEBLANKER
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

    if((ts.nb_setting > 0) &&  (ts.dsp.active & DSP_NB_ENABLE)
            //            && (ts.dmod_mode != DEMOD_AM && ts.dmod_mode != DEMOD_FM)
            && (ts.dmod_mode != DEMOD_FM))
        //        && (filters_p->sample_rate_dec != RX_DECIMATION_RATE_24KHZ ))

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



#ifdef USE_FREEDV
/**
 * @returns: true if digital signal should be used (no analog processing should be done), false -> analog processing maybe used
 * since no digital signal was detected.
 */
static bool AudioDriver_RxProcessorFreeDV (iq_buffer_t* iq_buf_p, float32_t * const dst, int16_t blockSize)
{

    bool retval = false;

    // Freedv DL2FW
    static int16_t modulus_Decimate = 0, modulus_Interpolate = 0;
    static bool bufferFilled;
    static float32_t History[3]={0.0,0.0,0.0};

    const int32_t factor_Decimate = IQ_SAMPLE_RATE/8000; // 6x @ 48ksps
    const int32_t factor_Interpolate = AUDIO_SAMPLE_RATE/8000; // 6x @ 48ksps

    bool lsb_active = RadioManagement_LSBActive(ts.dmod_mode);

    // we switch I and Q here so that we later always use USB demodulation
    // this works because we have the frequency of interest centered.
    float32_t* real = (lsb_active == true)? iq_buf_p->q_buffer : iq_buf_p->i_buffer;
    float32_t* imag = (lsb_active == true)? iq_buf_p->i_buffer : iq_buf_p->q_buffer;

    float32_t real_buffer[blockSize];
    float32_t imag_buffer[blockSize];

#ifdef USE_SIMPLE_FREEDV_FILTERS
    arm_biquad_cascade_df1_f32 (&IIR_biquad_FreeDV_I, real, real_buffer, blockSize);
    arm_biquad_cascade_df1_f32 (&IIR_biquad_FreeDV_Q, imag, imag_buffer, blockSize);
#else
    // we run a hilbert transform including a low pass to avoid
    // aliasing artifacts
    arm_fir_f32(&Fir_FreeDV_Rx_Hilbert_I,real,real_buffer,blockSize);
    arm_fir_f32(&Fir_FreeDV_Rx_Hilbert_Q,imag,imag_buffer,blockSize);
#endif

    // DOWNSAMPLING
    for (int k = 0; k < blockSize; k++)
    {
        if (modulus_Decimate == 0)  //every nth sample has to be catched -> downsampling
        {

            const float32_t f32_to_i16_gain = 10;
            // 10 for normal RX, 1000 for USB PC debugging

#ifdef USE_SIMPLE_FREEDV_FILTERS
            fdv_iq_rb_item_t sample;
            // this is the USB demodulation I + Q

            sample.real = real_buffer[k] * f32_to_i16_gain;
            sample.imag = imag_buffer[k] * f32_to_i16_gain;
            RingBuffer_PutSamples(&fdv_iq_rb, &sample, 1);

#else
            int16_t sample;
            // this is the USB demodulation I + Q
            sample = (real_buffer[k] + imag_buffer[k]) * f32_to_i16_gain;
            RingBuffer_PutSamples(&fdv_demod_rb, &sample, 1);
#endif
        }

        // increment and wrap
        modulus_Decimate++;
        if (modulus_Decimate == factor_Decimate)
        {
            modulus_Decimate = 0;
        }
    }


    // if we run out  of buffers lately
    // we wait for availability of at least 2 buffers
    // so that in theory we have uninterrupted flow of audio
    // albeit with a delay of 80ms
    // we wait for at least one iq frame plus one speech block being processed until we start
    // sending audio to the speaker
    if (bufferFilled == false && RingBuffer_GetData(&fdv_audio_rb) > FreeDV_Iq_Get_FrameLen())
    {
        bufferFilled = true;
    }

    if (bufferFilled == true && RingBuffer_GetData(&fdv_audio_rb) >= (modulus_Interpolate != 0?5:6)) // freeDV encode has finished (running in ui_driver.c)?
    {
        // Best thing here would be to use the arm_fir_decimate function! Why?
        // --> we need phase linear filters, because we have to filter I & Q and preserve their phase relationship
        // IIR filters are power saving, but they do not care about phase, so useless at this point
        // FIR filters are phase linear, but need processor power
        // so we now use the decimation function that upsamples like the code below, BUT at the same time filters
        // (and the routine knows that it does not have to multiply with 0 while filtering: if we do upsampling and subsequent
        // filtering, the filter does not know that and multiplies with zero 5 out of six times --> very inefficient)
        // BUT: we cannot use the ARM function, because decimation factor (6) has to be an integer divide of
        // block size (which is 32 in our case --> 32 / 6 = non-integer!)


        for (int j=0; j < blockSize; j++) //upsampling with integrated interpolation-filter for M=6
            // avoiding multiplications by zero within the arm_iir_filter
        {
            static float32_t sample;

            if (modulus_Interpolate == 0)
            {
                int16_t sample_in;
                RingBuffer_GetSamples(&fdv_audio_rb, &sample_in, 1);
                sample = (float32_t)sample_in * 0.25;
                // we scale samples down to align this with other demodulators
                // TODO: find out correct scaling val, not just a rough estimation
            }
            dst[j] = Fir_Rx_FreeDV_Interpolate_Coeffs[5-modulus_Interpolate] * History[0] +
                     Fir_Rx_FreeDV_Interpolate_Coeffs[11-modulus_Interpolate] * History[1] +
                     Fir_Rx_FreeDV_Interpolate_Coeffs[17-modulus_Interpolate] * History[2] +
                     Fir_Rx_FreeDV_Interpolate_Coeffs[23-modulus_Interpolate] * sample;

            History[0] = History[1];
            History[1] = History[2];
            History[2] = sample;

            // increment and wrap
            modulus_Interpolate++;
            if (modulus_Interpolate == factor_Interpolate)
            {
                modulus_Interpolate = 0;
            }
        }

        retval = true; // yes we have output to share
    }
    else
    {
        bufferFilled = false;
        profileEvent(FreeDVTXUnderrun);
    }
    if (retval == false && freedv_conf.mute_if_squelched == true)
    {
        memset(dst,0,sizeof(dst[0])*blockSize);
        retval = true;
    }

    return retval;
}
#endif

// RTTY Experiment based on code from the DSP Tutorial at http://dp.nonoo.hu/projects/ham-dsp-tutorial/18-rtty-decoder-using-iir-filters/
// Used with permission from Norbert Varga, HA2NON under GPLv3 license
#ifdef USE_RTTY_PROCESSOR
static void AudioDriver_RxProcessor_Rtty(float32_t * const src, int16_t blockSize)
{
    for (uint16_t idx = 0; idx < blockSize; idx++)
    {
        Rtty_Demodulator_ProcessSample(src[idx]);
    }
}
#endif

static void AudioDriver_RxProcessor_Bpsk(float32_t * const src, int16_t blockSize)
{

    for (uint16_t idx = 0; idx < blockSize; idx++)
    {
        Psk_Demodulator_ProcessSample(src[idx]);
    }
}


// FM Demodulator parameters
#define FM_DEMOD_COEFF1     PI/4            // Factors used in arctan approximation used in FM demodulator
#define FM_DEMOD_COEFF2     PI*0.75
//
#define FM_RX_SCALING_2K5   10000   // 33800            // Amplitude scaling factor of demodulated FM audio (normalized for +/- 2.5 kHz deviation at 1 kHZ)
#define FM_RX_SCALING_5K    (FM_RX_SCALING_2K5/2)   // Amplitude scaling factor of demodulated FM audio (normalized for +/- 5 kHz deviation at 1 kHz)
//
#define FM_AGC_SCALING      2               // Scaling factor for AGC result when in FM (AGC used only for S-meter)
//
#define FM_RX_LPF_ALPHA     0.05            // For FM demodulator:  "Alpha" (low-pass) factor to result in -6dB "knee" at approx. 270 Hz
//
#define FM_RX_HPF_ALPHA     0.96            // For FM demodulator:  "Alpha" (high-pass) factor to result in -6dB "knee" at approx. 180 Hz
//
#define FM_RX_SQL_SMOOTHING 0.005           // Smoothing factor for IIR squelch noise averaging
#define FM_SQUELCH_HYSTERESIS   3           // Hysteresis for FM squelch
#define FM_SQUELCH_PROC_DECIMATION  ((uint32_t)(1/FM_RX_SQL_SMOOTHING))     // Number of times we go through the FM demod algorithm before we do a squelch calculation



#define FM_TONE_DETECT_ALPHA    0.9                     // setting for IIR filtering of ratiometric result from frequency-differential tone detection

#define FM_SUBAUDIBLE_TONE_DET_THRESHOLD    1.75        // threshold of "smoothed" output of Goertzel, above which a tone is considered to be "provisionally" detected pending debounce
#define FM_SUBAUDIBLE_DEBOUNCE_MAX          5           // maximum "detect" count in debounce
#define FM_SUBAUDIBLE_TONE_DEBOUNCE_THRESHOLD   2       // number of debounce counts at/above which a tone detection is considered valid


typedef struct
{
    float i_prev;
    float32_t q_prev;
    float32_t lpf_prev;
    float32_t hpf_prev_a;
    float32_t hpf_prev_b;// used in FM detection and low/high pass processing

    float subdet;                // used for tone detection
    uint8_t count;
    uint8_t tdet;// used for squelch processing and debouncing tone detection, respectively
    ulong gcount;            // used for averaging in tone detection

} demod_fm_data_t;

demod_fm_data_t fm_data;

/**
 * FM Demodulator, runs at IQ_SAMPLE_RATE and outputs at same rate, does subtone detection
 *
 * @author KA7OEI
 *
 * @param i_buffer
 * @param q_buffer
 * @param a_buffer
 * @param blockSize
 * @return
 */
 static bool AudioDriver_DemodFM(const float32_t* i_buffer, const float32_t* q_buffer, float32_t* a_buffer, const int16_t blockSize)
{
	float32_t goertzel_buf[blockSize], squelch_buf[blockSize];

	if (ts.iq_freq_mode != FREQ_IQ_CONV_MODE_OFF)// bail out if translate mode is not active
	{

		bool tone_det_enabled = ads.fm_conf.subaudible_tone_det_freq != 0;// set a quick flag for checking to see if tone detection is enabled

		for (uint16_t i = 0; i < blockSize; i++)
		{
			// first, calculate "x" and "y" for the arctan2, comparing the vectors of present data with previous data

			float32_t y = (fm_data.i_prev * q_buffer[i]) - (i_buffer[i] * fm_data.q_prev);
			float32_t x = (fm_data.i_prev * i_buffer[i]) + (q_buffer[i] * fm_data.q_prev);

            float32_t angle = atan2f(y, x);

            // we now have our audio in "angle"
			squelch_buf[i] = angle;	// save audio in "d" buffer for squelch noise filtering/detection - done later

			// Now do integrating low-pass filter to do FM de-emphasis
			float32_t a = fm_data.lpf_prev + (FM_RX_LPF_ALPHA * (angle - fm_data.lpf_prev));	//
			fm_data.lpf_prev = a;			// save "[n-1]" sample for next iteration

			goertzel_buf[i] = a;	// save in "c" for subaudible tone detection

			if (((!ads.fm_conf.squelched) && (!tone_det_enabled))
					|| ((ads.fm_conf.subaudible_tone_detected) && (tone_det_enabled))
					|| ((!ts.fm_sql_threshold)))// high-pass audio only if we are un-squelched (to save processor time)
			{

				// Do differentiating high-pass filter to attenuate very low frequency audio components, namely subadible tones and other "speaker-rattling" components - and to remove any DC that might be present.
				float32_t b = FM_RX_HPF_ALPHA * (fm_data.hpf_prev_b + a - fm_data.hpf_prev_a);// do differentiation
				fm_data.hpf_prev_a = a;		// save "[n-1]" samples for next iteration
				fm_data.hpf_prev_b = b;

				a_buffer[i] = b;// save demodulated and filtered audio in main audio processing buffer
			}
			else if ((ads.fm_conf.squelched)
					|| ((!ads.fm_conf.subaudible_tone_detected) && (tone_det_enabled)))// were we squelched or tone NOT detected?
			{
				a_buffer[i] = 0;// do not filter receive audio - fill buffer with zeroes to mute it
			}

			fm_data.q_prev = q_buffer[i];// save "previous" value of each channel to allow detection of the change of angle in next go-around
			fm_data.i_prev = i_buffer[i];
		}

		// *** Squelch Processing ***
		arm_iir_lattice_f32(&IIR_Squelch_HPF, squelch_buf, squelch_buf,
				blockSize);	// Do IIR high-pass filter on audio so we may detect squelch noise energy

		ads.fm_conf.sql_avg = ((1 - FM_RX_SQL_SMOOTHING) * ads.fm_conf.sql_avg)
				+ (FM_RX_SQL_SMOOTHING * sqrtf(fabsf(squelch_buf[0])));// IIR filter squelch energy magnitude:  We need look at only one representative sample

		//
		// Squelch processing
		//
		// Determine if the (averaged) energy in "ads.fm.sql_avg" is above or below the squelch threshold
		//
		fm_data.count++;// bump count that controls how often the squelch threshold is checked
		fm_data.count %= FM_SQUELCH_PROC_DECIMATION;    // enforce the count limit

		if (fm_data.count == 0)	// do the squelch threshold calculation much less often than we are called to process this audio
		{
			if (ads.fm_conf.sql_avg > 0.175)	// limit maximum noise value in averaging to keep it from going out into the weeds under no-signal conditions (higher = noisier)
			{
				ads.fm_conf.sql_avg = 0.175;
			}

			float32_t scaled_sql_avg = ads.fm_conf.sql_avg * 172;// scale noise amplitude to range of squelch setting

			if (scaled_sql_avg > 24)						// limit noise amplitude range
			{
				scaled_sql_avg = 24;
			}
			//
			scaled_sql_avg = 22 - scaled_sql_avg;
			// "invert" the noise power so that high number now corresponds with
			// quieter signal:  "scaled_sql_avg" may now be compared with squelch setting



			// Now evaluate noise power with respect to squelch setting
			if (ts.fm_sql_threshold == 0)	 	// is squelch set to zero?
			{
				ads.fm_conf.squelched = false;		// yes, the we are un-squelched
			}
			else
			{
			    if (ads.fm_conf.squelched == true)	 	// are we squelched?
			    {
			        if (scaled_sql_avg >= (float) (ts.fm_sql_threshold + FM_SQUELCH_HYSTERESIS))	// yes - is average above threshold plus hysteresis?
			        {
			            ads.fm_conf.squelched = false;		//  yes, open the squelch
			        }
			    }
			    else	 	// is the squelch open (e.g. passing audio)?
			    {
			        if (ts.fm_sql_threshold > FM_SQUELCH_HYSTERESIS)// is setting higher than hysteresis?
			        {
			            if (scaled_sql_avg < (float) (ts.fm_sql_threshold - FM_SQUELCH_HYSTERESIS))// yes - is average below threshold minus hysteresis?
			            {
			                ads.fm_conf.squelched = true;	// yes, close the squelch
			            }
			        }
			        else	 // setting is lower than hysteresis so we can't use it!
			        {
			            if (scaled_sql_avg < (float) ts.fm_sql_threshold)// yes - is average below threshold?
			            {
			                ads.fm_conf.squelched = true;	// yes, close the squelch
			            }
			        }
			    }
			}
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
		    fm_data.gcount++;// this counter is used for the accumulation of data over multiple cycles
			//
			for (uint16_t i = 0; i < blockSize; i++)
			{

				// Detect above target frequency
				AudioFilter_GoertzelInput(&ads.fm_conf.goertzel[FM_HIGH],goertzel_buf[i]);
				// Detect energy below target frequency
				AudioFilter_GoertzelInput(&ads.fm_conf.goertzel[FM_LOW],goertzel_buf[i]);
				// Detect on-frequency energy
				AudioFilter_GoertzelInput(&ads.fm_conf.goertzel[FM_CTR],goertzel_buf[i]);
			}

			if (fm_data.gcount >= FM_SUBAUDIBLE_GOERTZEL_WINDOW)// have we accumulated enough samples to do the final energy calculation?
			{
				float32_t s = AudioFilter_GoertzelEnergy(&ads.fm_conf.goertzel[FM_HIGH]) + AudioFilter_GoertzelEnergy(&ads.fm_conf.goertzel[FM_LOW]);
				// sum +/- energy levels:
				// s = "off frequency" energy reading

				float32_t r = AudioFilter_GoertzelEnergy(&ads.fm_conf.goertzel[FM_CTR]);
				fm_data.subdet = ((1 - FM_TONE_DETECT_ALPHA) * fm_data.subdet)
						+ (r / (s / 2) * FM_TONE_DETECT_ALPHA);	// do IIR filtering of the ratio between on and off-frequency energy

				if (fm_data.subdet > FM_SUBAUDIBLE_TONE_DET_THRESHOLD)// is subaudible tone detector ratio above threshold?
				{
				    fm_data.tdet++;	// yes - increment count			// yes - bump debounce count
					if (fm_data.tdet > FM_SUBAUDIBLE_DEBOUNCE_MAX)// is count above the maximum?
					{
					    fm_data.tdet = FM_SUBAUDIBLE_DEBOUNCE_MAX;// yes - limit the count
					}
				}
				else	 	// it is below the threshold - reduce the debounce
				{
					if (fm_data.tdet)		// - but only if already nonzero!
					{
					    fm_data.tdet--;
					}
				}
				if (fm_data.tdet >= FM_SUBAUDIBLE_TONE_DEBOUNCE_THRESHOLD)// are we above the debounce threshold?
				{
					ads.fm_conf.subaudible_tone_detected = 1;// yes - a tone has been detected
				}
				else									// not above threshold
				{
					ads.fm_conf.subaudible_tone_detected = 0;	// no tone detected
				}

				fm_data.gcount = 0;		// reset accumulation counter
			}
		}
		else	 		// subaudible tone detection disabled
		{
			ads.fm_conf.subaudible_tone_detected = 1;// always signal that a tone is being detected if detection is disabled to enable audio gate
		}
	}
	return ads.fm_conf.squelched == false;
}

#if defined (USE_LMS_AUTONOTCH)
 /**
  * @author Clint, KA7OEI
  *
  * @param blockSize
  * @param notchbuffer
  */
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
    lms2_inbuf %= ts.dsp.notch_delaybuf_len;
    lms2_outbuf %= ts.dsp.notch_delaybuf_len;
    //
}
#endif



static void AudioDriver_Mix(float32_t* src, float32_t* dst, float32_t scaling, const uint16_t blockSize)
{
    float32_t                   e3_buffer[blockSize];

    arm_scale_f32(src, scaling, e3_buffer, blockSize);
    arm_add_f32(dst, e3_buffer, dst, blockSize);
}

void AudioDriver_IQPhaseAdjust(uint16_t txrx_mode, float32_t* i_buffer, float32_t* q_buffer, const uint16_t blockSize)
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

/**
 * Takes an iq_buffer and puts the samples into a single interleaved spectrum ringbuffer
 * To be called from the audio interrupt side of things. Do not call otherwise as the code is not
 * reentrant will screw up things.

 * @param iq_puf_b pair of i/q buffers
 * @param blockSize how many samples in a single iq buffer, target gets 2*blockSize samples
 */
static void AudioDriver_SpectrumCopyIqBuffers(iq_buffer_t* iq_puf_b, const size_t blockSize)
{
    for(int i = 0; i < blockSize; i++)
    {
        // Collect I/Q samples
        sd.FFT_RingBuffer[sd.samp_ptr] = iq_puf_b->q_buffer[i];    // get floating point data for FFT for spectrum scope/waterfall display
        sd.samp_ptr++;
        sd.FFT_RingBuffer[sd.samp_ptr] = iq_puf_b->i_buffer[i];
        sd.samp_ptr++;

        if(sd.samp_ptr >= sd.fft_iq_len-1)
        {
            sd.samp_ptr = 0;
        }
    }
}

/**
 * Places IQ data in the spectrums IQ ringbuffer if no zoom level has been set. Otherwise does nothing
 * To be called from the audio interrupt side of things. Do not call otherwise as the code is not
 * reentrant and may screw up things.
 * Will place blockSize / 2^sd.magnify samples in ring buffer
 *
 * @param iq_buf_p iq data before frequency shift at IQ_SAMEPLE_RATE
 * @param blockSize IQ sample count
 */

static void AudioDriver_SpectrumNoZoomProcessSamples(iq_buffer_t* iq_puf_b, const uint16_t blockSize)
{

    if(sd.reading_ringbuffer == false && sd.fft_iq_len > 0)
    {
        if(sd.magnify == 0)        //
        {
            AudioDriver_SpectrumCopyIqBuffers(iq_puf_b, blockSize);
            sd.FFT_frequency = (ts.tune_freq); // spectrum shows all, LO is center frequency;
        }
    }
}

/**
 * Places IQ data in the spectrums IQ ringbuffer if a zoom level has been set. Otherwise does nothing
 * To be called from the audio interrupt side of things. Do not call otherwise as the code is not
 * reentrant will screw up things.
 * Will place blockSize / 2^sd.magnify samples in ring buffer
 *
 * @param iq_buf_p iq data with frequency of interest centered! (after frequency shift) at IQ_SAMEPLE_RATE
 * @param blockSize IQ sample count
 */
static void AudioDriver_SpectrumZoomProcessSamples(iq_buffer_t* iq_buf_p,  const uint16_t blockSize)
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

            iq_buffer_t decim_iq_buf;

            // lowpass Filtering
            // Mag 2x - 12k lowpass --> 24k bandwidth
            // Mag 4x - 6k lowpass --> 12k bandwidth
            // Mag 8x - 3k lowpass --> 6k bandwidth
            // Mag 16x - 1k5 lowpass --> 3k bandwidth
            // Mag 32x - 750Hz lowpass --> 1k5 bandwidth

            // 1st attempt - use low processor power biquad lowpass filters with 4 stages each
            arm_biquad_cascade_df1_f32 (&IIR_biquad_Zoom_FFT_I, iq_buf_p->i_buffer,decim_iq_buf.i_buffer, blockSize);
            arm_biquad_cascade_df1_f32 (&IIR_biquad_Zoom_FFT_Q, iq_buf_p->q_buffer,decim_iq_buf.q_buffer, blockSize);

            // decimation
            arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_I, decim_iq_buf.i_buffer, decim_iq_buf.i_buffer, blockSize);
            arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_Q, decim_iq_buf.q_buffer, decim_iq_buf.q_buffer, blockSize);
            // collect samples for spectrum display 256-point-FFT

            AudioDriver_SpectrumCopyIqBuffers(&decim_iq_buf, blockSize/ (1<<sd.magnify));
            sd.FFT_frequency = ts.tune_freq + AudioDriver_GetTranslateFreq(); // spectrum shows center at translate frequency, LO + Translate Freq  is center frequency;

            // TODO: also insert sample collection for snap carrier here
            // and subsequently rebuild snap carrier to use a much smaller FFT (say 256 or 512)
            // in order to save many kilobytes of RAM ;-)
        }
    }
}

static float32_t AudioDriver_FadeLeveler(int chan, float32_t audio, float32_t corr)
{
    assert (chan < NUM_AUDIO_CHANNELS);

    static float32_t dc27[NUM_AUDIO_CHANNELS]; // static will be initialized with 0
    static float32_t dc_insert[NUM_AUDIO_CHANNELS];

    dc27[chan] = adb.sam.mtauR * dc27[chan] + adb.sam.onem_mtauR * audio;
    dc_insert[chan] = adb.sam.mtauI * dc_insert[chan] + adb.sam.onem_mtauI * corr;
    audio = audio + dc_insert[chan] - dc27[chan];

    return audio;
}

typedef struct
{
    const float32_t               c0[SAM_PLL_HILBERT_STAGES];          // Filter coefficients - path 0
    const float32_t               c1[SAM_PLL_HILBERT_STAGES];          // Filter coefficients - path 1
} demod_sam_const_t;

//sideband separation, these values never change
const demod_sam_const_t demod_sam_const =
{
    .c0 = {
        -0.328201924180698,
        -0.744171491539427,
        -0.923022915444215,
        -0.978490468768238,
        -0.994128272402075,
        -0.998458978159551,
        -0.999790306259206,
    },

    .c1 = {
        -0.0991227952747244,
        -0.565619728761389,
        -0.857467122550052,
        -0.959123933111275,
        -0.988739372718090,
        -0.996959189310611,
        -0.999282492800792,
    },
};

typedef struct
{
    uint16_t  count;

    float32_t fil_out;
    float32_t lowpass;
    float32_t omega2;
    float32_t phs;

    float32_t dsI;             // delayed sample, I path
    float32_t dsQ;             // delayed sample, Q path

#define OUT_IDX   (3 * SAM_PLL_HILBERT_STAGES)

    float32_t a[OUT_IDX + 3];     // Filter a variables
    float32_t b[OUT_IDX + 3];     // Filter b variables
    float32_t c[OUT_IDX + 3];     // Filter c variables
    float32_t d[OUT_IDX + 3];     // Filter d variables

} demod_sam_data_t;

demod_sam_data_t sam_data;

/**
 * Demodulate IQ carrying AM into audio, expects input to be at decimated input rate.
 * Can do real synchronous AM demodulation with phase detector and PLL
 *
 * @author DD4WH
 * @date December 2016
 *
 * @param i_buffer
 * @param q_buffer
 * @param a_buffer
 * @param blockSize
 */
static void AudioDriver_DemodSAM(float32_t* i_buffer, float32_t* q_buffer, float32_t a_buffer[][IQ_BLOCK_SIZE], int16_t blockSize, float32_t sampleRate)
{
    // new synchronous AM PLL & PHASE detector
    // wdsp Warren Pratt, 2016
    //*****************************

    switch(ts.dmod_mode)
    {
        /*
         * DSB (double sideband mode) if anybody needs that
         *
         // DSB = LSB + USB = (I - Q) + (I + Q)
         case DEMOD_DSB:
         arm_sub_f32(i_buffer, q_buffer, a_buffer[0], blockSize);
         arm_add_f32(i_buffer, a_buffer[0], a_buffer[0], blockSize);
         arm_add_f32(q_buffer, a_buffer[0], a_buffer[0], blockSize);
         break;
         */
    case DEMOD_AM:
        for(int i = 0; i < blockSize; i++)
        {
            float32_t audio;

            arm_sqrt_f32 (i_buffer[i] * i_buffer[i] + q_buffer[i] * q_buffer[i], &audio);
            if(ads.fade_leveler)
            {
                audio = AudioDriver_FadeLeveler(0,audio,0);
            }
            a_buffer[0][i] = audio;
        }
        break;

    case DEMOD_SAM:
    {

        // Wheatley 2011 cuteSDR & Warren Pratts WDSP, 2016
        for(int i = 0; i < blockSize; i++)
        {   // NCO

            float32_t ai, bi, aq, bq;
            float32_t Sin, Cos;


            sincosf(sam_data.phs,&Sin,&Cos);
            ai = Cos * i_buffer[i];
            bi = Sin * i_buffer[i];
            aq = Cos * q_buffer[i];
            bq = Sin * q_buffer[i];

            float32_t audio[NUM_AUDIO_CHANNELS];

            // we initialize the often unused stereo channel
            // to keep the compiler happy
#ifdef USE_TWO_CHANNEL_AUDIO
            audio[1] = 0;
#endif

            float32_t corr[2] = { ai + bq, -bi + aq };

            if (ads.sam_sideband != SAM_SIDEBAND_BOTH)
            {

                sam_data.a[0] = sam_data.dsI;
                sam_data.b[0] = bi;
                sam_data.c[0] = sam_data.dsQ;
                sam_data.d[0] = aq;
                sam_data.dsI = ai;
                sam_data.dsQ = bq;

                for (int j = 0; j < SAM_PLL_HILBERT_STAGES; j++)
                {
                    int k = 3 * j;
                    sam_data.a[k + 3] = demod_sam_const.c0[j] * (sam_data.a[k] - sam_data.a[k + 5]) + sam_data.a[k + 2];
                    sam_data.b[k + 3] = demod_sam_const.c1[j] * (sam_data.b[k] - sam_data.b[k + 5]) + sam_data.b[k + 2];
                    sam_data.c[k + 3] = demod_sam_const.c0[j] * (sam_data.c[k] - sam_data.c[k + 5]) + sam_data.c[k + 2];
                    sam_data.d[k + 3] = demod_sam_const.c1[j] * (sam_data.d[k] - sam_data.d[k + 5]) + sam_data.d[k + 2];
                }

                float32_t ai_ps = sam_data.a[OUT_IDX];
                float32_t bi_ps = sam_data.b[OUT_IDX];
                float32_t bq_ps = sam_data.c[OUT_IDX];
                float32_t aq_ps = sam_data.d[OUT_IDX];

                // make room for next sample
                for (int j = OUT_IDX + 2; j > 0; j--)
                {
                    sam_data.a[j] = sam_data.a[j - 1];
                    sam_data.b[j] = sam_data.b[j - 1];
                    sam_data.c[j] = sam_data.c[j - 1];
                    sam_data.d[j] = sam_data.d[j - 1];
                }

                switch(ads.sam_sideband)
                {
                default:
                case SAM_SIDEBAND_USB:
                    audio[0] = (ai_ps - bi_ps) + (aq_ps + bq_ps);
                    break;
                case SAM_SIDEBAND_LSB:
                    audio[0] = (ai_ps + bi_ps) - (aq_ps - bq_ps);
                    break;
    #ifdef USE_TWO_CHANNEL_AUDIO
                case SAM_SIDEBAND_STEREO:
                    audio[0] = (ai_ps + bi_ps) - (aq_ps - bq_ps);
                    audio[1] = (ai_ps - bi_ps) + (aq_ps + bq_ps);
                    break;
    #endif
                }

            }
            else
            {
                audio[0] = corr[0];
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
                a_buffer[chan][i] = audio[chan];
            }

            // determine phase error
            float32_t phzerror = atan2f(corr[1], corr[0]);

            float32_t del_out = sam_data.fil_out;
            // correct frequency 1st step
            sam_data.omega2 = sam_data.omega2 + adb.sam.g2 * phzerror;
            if (sam_data.omega2 < adb.sam.omega_min)
            {
                sam_data.omega2 = adb.sam.omega_min;
            }
            else if (sam_data.omega2 > adb.sam.omega_max)
            {
                sam_data.omega2 = adb.sam.omega_max;
            }
            // correct frequency 2nd step
            sam_data.fil_out = adb.sam.g1 * phzerror + sam_data.omega2;
            sam_data.phs = sam_data.phs + del_out;

            // wrap round 2PI, modulus
            while (sam_data.phs >= 2.0 * PI) { sam_data.phs -= (2.0 * PI); }
            while (sam_data.phs < 0.0) { sam_data.phs += (2.0 * PI); }
        }

        sam_data.count++;

        if(sam_data.count > 50) // to display the exact carrier frequency that the PLL is tuned to
            // in the small frequency display
            // we calculate carrier offset here and the display function is
            // then called in UiDriver_MainHandler approx. every 40-80ms
        { // to make this smoother, a simple lowpass/exponential averager here . . .
            float32_t carrier = 0.1 * (sam_data.omega2 * sampleRate) / (2.0 * PI);
            carrier = carrier + 0.9 * sam_data.lowpass;
            ads.carrier_freq_offset = carrier;
            sam_data.count = 0;
            sam_data.lowpass = carrier;
        }
    }
    break;
    }
}

/**
 * Runs the interrupt part of the twin peaks detection and triggers codec restart
 *
 * @param iq_corr_p pointer to the previously calculated correction data
 */
static void AudioDriver_RxHandleTwinpeaks(const iq_correction_data_t* iq_corr_p)
{
    // Test and fix of the "twinpeak syndrome"
    // which occurs sporadically and can -to our knowledge- only be fixed
    // by a reset of the codec
    // It can be identified by a totally non-existing mirror rejection,
    // so I & Q have essentially the same phase
    // We use this to identify the snydrome and reset the codec accordingly:
    // calculate phase between I & Q

    static uint32_t  twinpeaks_counter = 0;
    static uint32_t  codec_restarts = 0;
    static float32_t phase_IQ; // used to determine the twin peaks issue state
    static uint32_t  phase_IQ_runs;

    if (ts.twinpeaks_tested == TWINPEAKS_WAIT)
    {
        twinpeaks_counter++;
    }

    if(twinpeaks_counter > 1000) // wait 0.667s for the system to settle: with 32 IQ samples per block and 48ksps (0.66667ms/block)
    {
        ts.twinpeaks_tested = TWINPEAKS_SAMPLING;
        twinpeaks_counter = 0;
        phase_IQ = 0.0;
        phase_IQ_runs = 0;
    }

    if(iq_corr_p->teta3 != 0.0 && ts.twinpeaks_tested == TWINPEAKS_SAMPLING) // prevent divide-by-zero
        // twinpeak_tested = 2 --> wait for system to warm up
        // twinpeak_tested = 0 --> go and test the IQ phase
        // twinpeak_tested = 1 --> tested, verified, go and have a nice day!
        // twinpeak_tested = 3 --> was not correctable
        // twinpeak_tested = 4 --> we request the main loop to run the codec restart
        //                         we can't do the restart inside the audio interrupt

    {   // Moseley & Slump (2006) eq. (33)
        // this gives us the phase error between I & Q in radians
        float32_t phase_IQ_cur = asinf(iq_corr_p->teta1 / iq_corr_p->teta3);

        // we combine 50 cycles (1/30s) to calculate the "final" phase_IQ
        if (phase_IQ_runs == 0)
        {
            phase_IQ = phase_IQ_cur;
        }
        else
        {
            phase_IQ = 0.05 * phase_IQ_cur + 0.95 * phase_IQ;
        }
        phase_IQ_runs ++;

        if (phase_IQ_runs == 50)
        {

            if (fabsf(phase_IQ) > (M_PI/8.0))
                // threshold of 22.5 degrees phase shift == PI / 8
                // hopefully your hardware is not so bad, that its phase error is more than 22 degrees ;-)
                // if it is that bad, adjust this threshold to maybe PI / 7 or PI / 6
            {
                ts.twinpeaks_tested = TWINPEAKS_CODEC_RESTART;
                codec_restarts++;

                if(codec_restarts >= 4)
                {
                    ts.twinpeaks_tested = TWINPEAKS_UNCORRECTABLE;
                    codec_restarts = 0;
                }
            }
            else
            {
                ts.twinpeaks_tested = TWINPEAKS_DONE;
                codec_restarts = 0;
            }
        }
    }
}

/**
 *
 * @param blockSize
 */
static void AudioDriver_RxHandleIqCorrection(float32_t* i_buffer, float32_t* q_buffer, const uint16_t blockSize)
{

    assert(blockSize >= 8);

    if(!ts.iq_auto_correction) // Manual IQ imbalance correction
    {
        // Apply I/Q amplitude correction
        arm_scale_f32(i_buffer, ts.rx_adj_gain_var.i, i_buffer, blockSize);
        arm_scale_f32(q_buffer, ts.rx_adj_gain_var.q, q_buffer, blockSize); // TODO: we need only scale one channel! DD4WH, Dec 2016

        // Apply I/Q phase correction
        AudioDriver_IQPhaseAdjust(ts.txrx_mode, i_buffer, q_buffer, blockSize);
    }

    else // Automatic IQ imbalance correction
    {   // Moseley, N.A. & C.H. Slump (2006): A low-complexity feed-forward I/Q imbalance compensation algorithm.
        // in 17th Annual Workshop on Circuits, Nov. 2006, pp. 158-164.
        // http://doc.utwente.nl/66726/1/moseley.pdf

        for(uint32_t i = 0; i < blockSize; i++)
        {
            adb.iq_corr.teta1 += Math_sign_new(i_buffer[i]) * q_buffer[i]; // eq (34)
            adb.iq_corr.teta2 += Math_sign_new(i_buffer[i]) * i_buffer[i]; // eq (35)
            adb.iq_corr.teta3 += Math_sign_new(q_buffer[i]) * q_buffer[i]; // eq (36)
        }

        adb.iq_corr.teta1 = -0.003 * (adb.iq_corr.teta1 / blockSize) + 0.997 * adb.iq_corr.teta1_old; // eq (34) and first order lowpass
        adb.iq_corr.teta2 =  0.003 * (adb.iq_corr.teta2 / blockSize) + 0.997 * adb.iq_corr.teta2_old; // eq (35) and first order lowpass
        adb.iq_corr.teta3 =  0.003 * (adb.iq_corr.teta3 / blockSize) + 0.997 * adb.iq_corr.teta3_old; // eq (36) and first order lowpass

        adb.iq_corr.M_c1 = (adb.iq_corr.teta2 != 0.0) ? adb.iq_corr.teta1 / adb.iq_corr.teta2 : 0.0; // eq (30)
        // prevent divide-by-zero

        float32_t help = (adb.iq_corr.teta2 * adb.iq_corr.teta2);

        if(help > 0.0)// prevent divide-by-zero
        {
            help = (adb.iq_corr.teta3 * adb.iq_corr.teta3 - adb.iq_corr.teta1 * adb.iq_corr.teta1) / help; // eq (31)
        }

        adb.iq_corr.M_c2 = (help > 0.0) ? sqrtf(help) : 1.0;  // eq (31)
        // prevent sqrtf of negative value

        AudioDriver_RxHandleTwinpeaks(&adb.iq_corr);

        adb.iq_corr.teta1_old = adb.iq_corr.teta1;
        adb.iq_corr.teta2_old = adb.iq_corr.teta2;
        adb.iq_corr.teta3_old = adb.iq_corr.teta3;
        adb.iq_corr.teta1 = 0.0;
        adb.iq_corr.teta2 = 0.0;
        adb.iq_corr.teta3 = 0.0;

        // first correct Q and then correct I --> this order is crucially important!
        for(uint32_t i = 0; i < blockSize; i++)
        {   // see fig. 5
            q_buffer[i] += adb.iq_corr.M_c1 * i_buffer[i];
        }
        // see fig. 5
        arm_scale_f32 (i_buffer, adb.iq_corr.M_c2, i_buffer, blockSize);
    }

}


/**
 * This function is the interrupt side interface of the Noise Reduction
 * @param blockSizeDecim
 * @param inout_buffer the buffer carrying both the input data (subject to processing) and the processed
 *        output samples. Since the noise reduction uses a large buffer to be run on bigger chunks of samples
 *        there is a several milliseconds delay between the input being passed here and being returned processed
 *        Due to the buffering this function itself just puts one block in the buffer and returns a buffer if available
 *        or silence if not.
 */
static void AudioDriver_RxProcessorNoiseReduction(uint16_t blockSizeDecim, float32_t* inout_buffer)
{
#ifdef USE_ALTERNATE_NR

    // we check if we can safely use the buffer, if not, we just do nothing
    // TODO: add a better way to control which functions are used (available) in which mode
#ifdef USE_FREEDV
    if ((ts.dmod_mode == DEMOD_DIGI && ts.digital_mode == DigitalMode_FreeDV) == false)
#endif
    {
        static int trans_count_in=0;
        static int outbuff_count=0;
        static int NR_fill_in_pt=0;
        static NR_Buffer* out_buffer = NULL;
        // this would be the right place for another decimation-by-2 to get down to 6ksps
        // in order to further improve the spectral noise reduction

        // anti-alias-filtering is already provided at this stage by the IIR main filter
        // Only allow another decimation-by-two, if filter bandwidth is <= 2k7
        //
        // Add decimation-by-two HERE

        //  decide whether filter < 2k7!!!
        uint32_t no_dec_samples = blockSizeDecim; // only used for the noise reduction decimation-by-two handling
        // buffer needs max blockSizeDecim , less if second decimation is done
        // see below

        nr_params.NR_decimation_active = nr_params.NR_decimation_enable && (FilterInfo[ts.filters_p->id].width < 2701);

        if (nr_params.NR_decimation_active == true)
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
        else
        {
            memset(NR_dec_buffer,0,sizeof(NR_dec_buffer));
        }


        // interpolation of a_buffer from 6ksps to 12ksps!
        // from NR_dec_buffer --> a_buffer
        // but only, if we have decimated to 6ksps, otherwise just copy the samples into a_buffer
        if (nr_params.NR_decimation_active == true)
        {
            arm_fir_interpolate_f32(&INTERPOLATE_NR, NR_dec_buffer, inout_buffer, no_dec_samples);
            arm_scale_f32(inout_buffer, 2.0, inout_buffer, blockSizeDecim);
        }
        else
        {
            arm_copy_f32(NR_dec_buffer, inout_buffer, blockSizeDecim);
        }
    }
#endif
}

static void RxProcessor_DemodAudioPostprocessing(float32_t (*a_buffer)[AUDIO_BLOCK_SIZE], const size_t blockSize, const size_t blockSizeDecim, const uint32_t sampleRateDecim, const bool use_stereo)
{
    const uint8_t  dsp_active = ts.dsp.active;
    const uint8_t dmod_mode = ts.dmod_mode;

    if (ts.dsp.inhibit == false)
    {
        if((dsp_active & DSP_NOTCH_ENABLE) && (dmod_mode != DEMOD_CW) && !(dmod_mode == DEMOD_SAM && sampleRateDecim == 24000))       // No notch in CW
        {
#ifdef USE_LEAKY_LMS
            if(ts.enable_leaky_LMS)
            {
                  AudioDriver_LeakyLmsNr(a_buffer[0], a_buffer[0], blockSizeDecim, 1);
            }
            else
#endif
            {
#ifdef USE_LMS_AUTONOTCH
                AudioDriver_NotchFilter(blockSizeDecim, a_buffer[0]);     // Do notch filter
#endif
            }
        }

#if defined(USE_LEAKY_LMS)
        // DSP noise reduction using LMS (Least Mean Squared) algorithm
        // This is the pre-filter/AGC instance
        if((dsp_active & DSP_NR_ENABLE) && (!(dsp_active & DSP_NR_POSTAGC_ENABLE)) && !(dmod_mode == DEMOD_SAM && sampleRateDecim == 24000))      // Do this if enabled and "Pre-AGC" DSP NR enabled
        {
            if(ts.enable_leaky_LMS)
            {
                  AudioDriver_LeakyLmsNr(a_buffer[0], a_buffer[0], blockSizeDecim, 0);
            }
        }
#endif
    }

    // Apply audio  bandpass filter
    if (IIR_PreFilter[0].numStages > 0)   // yes, we want an audio IIR filter
    {
        arm_iir_lattice_f32(&IIR_PreFilter[0], a_buffer[0], a_buffer[0], blockSizeDecim);
#ifdef USE_TWO_CHANNEL_AUDIO
        if(use_stereo)
        {
             arm_iir_lattice_f32(&IIR_PreFilter[1], a_buffer[1], a_buffer[1], blockSizeDecim);
        }
#endif
    }

    // now process the samples and perform the receiver AGC function
    AudioAgc_RunAgcWdsp(blockSizeDecim, a_buffer, use_stereo);


    // DSP noise reduction using LMS (Least Mean Squared) algorithm
    // This is the post-filter, post-AGC instance
#if defined(USE_LEAKY_LMS)

    if((is_dsp_nr()) && (is_dsp_nr_postagc()) && (!ts.dsp.inhibit) && !(dmod_mode == DEMOD_SAM && sampleRateDecim == 24000))     // Do DSP NR if enabled and if post-DSP NR enabled
    {
        if(ts.enable_leaky_LMS)
        {
              AudioDriver_LeakyLmsNr(a_buffer[0], a_buffer[0], blockSizeDecim, 0);
        }
    }
#endif

    if (sampleRateDecim == 12000 && (is_dsp_nb_active() || is_dsp_nr())) //start of new nb or new noise reduction
    {
        // NR_in and _out buffers are using the same physical space than the freedv_iq_buffer in a
        // shared MultiModeBuffer union.
        // for NR reduction we use a maximum of 256 real samples
        // so we use the freedv_iq buffers in a way, that we use the first half of each array for the input
        // and the second half for the output
        // .real and .imag are loosing there meaning here as they represent consecutive real samples
        AudioDriver_RxProcessorNoiseReduction(blockSizeDecim, a_buffer[0]);
    } // end of new nb

    // Calculate scaling based on decimation rate since this affects the audio gain
    const float32_t post_agc_gain_scaling =
            (ts.filters_p->sample_rate_dec == RX_DECIMATION_RATE_12KHZ) ?
            POST_AGC_GAIN_SCALING_DECIMATE_4 : POST_AGC_GAIN_SCALING_DECIMATE_2;


    // Scale audio according to AGC setting, demodulation mode and required fixed levels and scaling
    const float32_t scale_gain = post_agc_gain_scaling *
            ((dmod_mode == DEMOD_AM || dmod_mode == DEMOD_SAM) ?
            0.5 /* AM/SAM */ : 0.333  /* not AM/SAM */);

    // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
    arm_scale_f32(a_buffer[0],scale_gain, a_buffer[0], blockSizeDecim);

    // this is the biquad filter, a notch, peak, and lowshelf filter
    arm_biquad_cascade_df1_f32 (&IIR_biquad_1[0], a_buffer[0],a_buffer[0], blockSizeDecim);
#ifdef USE_TWO_CHANNEL_AUDIO
    if(use_stereo)
    {
        arm_scale_f32(a_buffer[1],scale_gain, a_buffer[1], blockSizeDecim); // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
        arm_biquad_cascade_df1_f32 (&IIR_biquad_1[1], a_buffer[1],a_buffer[1], blockSizeDecim);
    }
#endif


    // all of these modems only work with 12 khz Samplerate
    // TODO: User needs feedback if the modem is not activated due to wrong decimation rate
    if (sampleRateDecim == 12000)
    {
#ifdef USE_RTTY_PROCESSOR
        if (is_demod_rtty()) // only works when decimation rate is 4 --> sample rate == 12ksps
        {
            AudioDriver_RxProcessor_Rtty(a_buffer[0], blockSizeDecim);
        }
#endif
        if (is_demod_psk()) // only works when decimation rate is 4 --> sample rate == 12ksps
        {
            AudioDriver_RxProcessor_Bpsk(a_buffer[0], blockSizeDecim);
        }

        if((dmod_mode == DEMOD_CW || dmod_mode == DEMOD_AM || dmod_mode == DEMOD_SAM))
            // switch to use TUNE HELPER in AM/SAM
        {
            CwDecode_RxProcessor(a_buffer[0], blockSizeDecim);
        }
    }

    // resample back to original sample rate while doing low-pass filtering to minimize audible aliasing effects
    if (INTERPOLATE_RX[0].phaseLength > 0)
    {
#ifdef USE_TWO_CHANNEL_AUDIO
        float32_t temp_buffer[IQ_BLOCK_SIZE];
        if(use_stereo)
        {
            arm_fir_interpolate_f32(&INTERPOLATE_RX[1], a_buffer[1], temp_buffer, blockSizeDecim);
        }
#endif
        arm_fir_interpolate_f32(&INTERPOLATE_RX[0], a_buffer[0], a_buffer[1], blockSizeDecim);

#ifdef USE_TWO_CHANNEL_AUDIO
        if(use_stereo)
        {
            arm_copy_f32(temp_buffer,a_buffer[0],blockSize);
        }
#endif
    }

    // additional antialias filter for specific bandwidths
    // IIR ARMA-type lattice filter
    if (IIR_AntiAlias[0].numStages > 0)   // yes, we want an interpolation IIR filter
    {
        arm_iir_lattice_f32(&IIR_AntiAlias[0], a_buffer[1], a_buffer[1], blockSize);
#ifdef USE_TWO_CHANNEL_AUDIO
        if(use_stereo)
        {
            arm_iir_lattice_f32(&IIR_AntiAlias[1], a_buffer[0], a_buffer[0], blockSize);
        }
#endif
    }

}


/**
 * Gets IQ data as input, runs the rx processing on the input signal, leaves audio data in DMA buffer
 *
 * @param src iq input DMA buffer
 * @param dst audio output DMA buffer
 * @param blockSize number of input and output samples
 * @param external_mute request to produce silence
 */
static void AudioDriver_RxProcessor(IqSample_t * const srcCodec, AudioSample_t * const dst, const uint16_t blockSize, bool external_mute)
{
    // this is the main RX audio function
	// it is driven with 32 samples in the complex buffer scr, meaning 32 * I AND 32 * Q
	// blockSize is thus 32, DD4WH 2018_02_06

    // we copy volatile variables which are used multiple times to local consts to let the compiler to its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t tx_audio_source = ts.tx_audio_source;
    const uint8_t rx_iq_source = ts.rx_iq_source;

    const int32_t iq_freq_mode = ts.iq_freq_mode;
#ifdef USE_TWO_CHANNEL_AUDIO
    const bool use_stereo = ((dmod_mode == DEMOD_IQ || dmod_mode == DEMOD_SSBSTEREO || (dmod_mode == DEMOD_SAM && ads.sam_sideband == SAM_SIDEBAND_STEREO)) && ts.stereo_enable);
#else
    const bool use_stereo = false;
#endif

    IqSample_t srcUSB[blockSize];


    IqSample_t * const src = (rx_iq_source == RX_IQ_DIG || rx_iq_source == RX_IQ_DIGIQ) ? srcUSB : srcCodec;
    // If source is digital usb in, pull from USB buffer, discard codec iq and
    // let the normal processing happen
    if (rx_iq_source == RX_IQ_DIG || rx_iq_source == RX_IQ_DIGIQ)
    {
        // audio sample rate must match the sample rate of USB audio if we read from USB
        assert(rx_iq_source != RX_IQ_DIG || AUDIO_SAMPLE_RATE == USBD_AUDIO_FREQ);

        // iq sample rate must match the sample rate of USB IQ audio if we read from USB
        assert(tx_audio_source != RX_IQ_DIGIQ || IQ_SAMPLE_RATE == USBD_AUDIO_FREQ);

        UsbdAudio_FillTxBuffer((AudioSample_t*)srcUSB,blockSize);
    }

    if (tx_audio_source == TX_AUDIO_DIGIQ)
    {
        for(uint32_t i = 0; i < blockSize; i++)
        {
            // 16 bit format - convert to float and increment
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            UsbdAudio_PutSample(I2S_IqSample_2_Int16(src[i].l));
            UsbdAudio_PutSample(I2S_IqSample_2_Int16(src[i].r));
        }
    }

    bool signal_active = false; // tells us if the modulator produced audio to listen to.

    // if the audio filters are being reconfigured, we don't process audio at all
    if (ads.af_disabled == 0 )
    {
        #ifdef USE_OBSOLETE_NOISEBLANKER
            AudioDriver_NoiseBlanker(src, blockSize);     // do noise blanker function
        #endif

        for(uint32_t i = 0; i < blockSize; i++)
        {
            int32_t level = abs(I2S_correctHalfWord(src[i].l))>>IQ_BIT_SHIFT;

            if(level > ADC_CLIP_WARN_THRESHOLD/4)            // This is the release threshold for the auto RF gain
            {
                ads.adc_quarter_clip = 1;
                if(level > ADC_CLIP_WARN_THRESHOLD/2)            // This is the trigger threshold for the auto RF gain
                {
                    ads.adc_half_clip = 1;
                    if(level > ADC_CLIP_WARN_THRESHOLD)          // This is the threshold for the red clip indicator on S-meter
                    {
                        ads.adc_clip = 1;
                    }
                }
            }

            adb.iq_buf.i_buffer[i] = I2S_correctHalfWord(src[i].l);
            adb.iq_buf.q_buffer[i] = I2S_correctHalfWord(src[i].r);
        }
        if (IQ_BIT_SCALE_DOWN != 1.0)
        {
            // we scale everything into the range of +/-32767 if we are getting 32 bit input
            arm_scale_f32 (adb.iq_buf.i_buffer, IQ_BIT_SCALE_DOWN, adb.iq_buf.i_buffer, blockSize);
            arm_scale_f32 (adb.iq_buf.q_buffer, IQ_BIT_SCALE_DOWN, adb.iq_buf.q_buffer, blockSize);
        }
        AudioDriver_RxHandleIqCorrection(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, blockSize);

        // at this point we have phase corrected IQ @ IQ_SAMPLE_RATE, unshifted in adb.iq_buf.i_buffer, adb.iq_buf.q_buffer

        // Spectrum display sample collect for magnify == 0
        AudioDriver_SpectrumNoZoomProcessSamples(&adb.iq_buf, blockSize);


        if(iq_freq_mode)            // is receive frequency conversion to be done?
        {
            FreqShift(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, blockSize, AudioDriver_GetTranslateFreq());
        }

        // at this point we have phase corrected IQ @ IQ_SAMPLE_RATE, with our RX frequency in the center (i.e. at 0 Hertz Shift)
        // in adb.iq_buf.i_buffer, adb.iq_buf.q_buffer


        // Spectrum display sample collect for magnify != 0

        AudioDriver_SpectrumZoomProcessSamples(&adb.iq_buf, blockSize);
#ifdef USE_FREEDV
        if (ts.dvmode == true && ts.digital_mode == DigitalMode_FreeDV)
        {
            signal_active = AudioDriver_RxProcessorFreeDV(&adb.iq_buf, adb.a_buffer[1], blockSize);
        }
#endif
        if (signal_active == false)
        {
            signal_active = true;
            // by default assume the modulator returns a signal, most do, except FM which may be squelched
            // and sets this to false

            const bool use_decimatedIQ =
                    ((ts.filters_p->FIR_I_coeff_file == i_rx_new_coeffs)  // lower than 3k8 bandwidth: new filters with excellent sideband suppression
                    && dmod_mode != DEMOD_FM ) || dmod_mode == DEMOD_SAM || dmod_mode == DEMOD_AM  ;

            const int16_t blockSizeDecim = blockSize/ads.decimation_rate;

            const uint16_t blockSizeIQ = use_decimatedIQ? blockSizeDecim: blockSize;
            const uint32_t sampleRateIQ = use_decimatedIQ? ads.decimated_freq: IQ_SAMPLE_RATE;

            // in some case we decimate the IQ before passing to demodulator, in some don't
            // in any case, we do audio filtering on decimated data


            // ------------------------
            // In SSB and CW - Do 0-90 degree Phase-added Hilbert Transform
            // In AM and SAM, the FIR below does ONLY low-pass filtering appropriate for the filter bandwidth selected, in
            // which case there is ***NO*** audio phase shift applied to the I/Q channels.
            /*
             * AM -> wants decimated IQ input, (I and Q is shifted as part of decimation?)
             * FM -> wants full IQ_SAMPLE_RATE input, shifted I and Q shifted by 90 degrees
             * SSB-> wants for narrower bandwidth decimated IQ input, shifted I and Q shifted by 90 degrees
             * SSB-> wants for wider bandwidth full IQ_SAMPLE_RATE input, shifted I and Q shifted by 90 degrees
             */

            if(use_decimatedIQ)
            {
                arm_fir_decimate_f32(&DECIMATE_RX_I, adb.iq_buf.i_buffer, adb.iq_buf.i_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
                arm_fir_decimate_f32(&DECIMATE_RX_Q, adb.iq_buf.q_buffer, adb.iq_buf.q_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
            }

            if(dmod_mode != DEMOD_SAM && dmod_mode != DEMOD_AM) // for SAM & AM leave out this processor-intense filter
            {
            	// SECOND: Hilbert transform (for all but AM/SAM)
                arm_fir_f32(&Fir_Rx_Hilbert_I,adb.iq_buf.i_buffer, adb.iq_buf.i_buffer, blockSizeIQ);   // Hilbert lowpass +45 degrees
                arm_fir_f32(&Fir_Rx_Hilbert_Q,adb.iq_buf.q_buffer, adb.iq_buf.q_buffer, blockSizeIQ);   // Hilbert lowpass -45 degrees
            }
            // at this point we have (low pass filtered/decimated?) IQ, with our RX frequency in the center (i.e. at 0 Hertz Shift)
            // in adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, block size is in blockSizeIQ

            if (RadioManagement_UsesBothSidebands(dmod_mode) || dmod_mode == DEMOD_SAM  )
            {
                // we must go here in DEMOD_SAM even if we effectively output only a single sideband
                switch(dmod_mode)
                {
                case DEMOD_AM:
                case DEMOD_SAM:
                    AudioDriver_DemodSAM(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, adb.a_buffer, blockSizeIQ, sampleRateIQ); // lowpass filtering, decimation, and SAM demodulation
                    break;
                case DEMOD_FM:
                    signal_active = AudioDriver_DemodFM(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, adb.a_buffer[0], blockSize);
                    break;
#ifdef USE_TWO_CHANNEL_AUDIO
                case DEMOD_IQ:  // leave I & Q as they are!
                    arm_copy_f32(adb.iq_buf.i_buffer, adb.a_buffer[0], blockSizeIQ);
                    arm_copy_f32(adb.iq_buf.q_buffer, adb.a_buffer[1], blockSizeIQ);
                    break;
                case DEMOD_SSBSTEREO: // LSB-left, USB-right
                    arm_add_f32(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, adb.a_buffer[0], blockSizeIQ);   // sum of I and Q - USB
                    arm_sub_f32(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, adb.a_buffer[1], blockSizeIQ);   // difference of I and Q - LSB
                    break;
#endif
                }
            }
            else if (RadioManagement_LSBActive(dmod_mode))
            {
                // all LSB modes are demodulated the same way, we handed the special case DEMOD_SAM / SAM-L earlier
                arm_sub_f32(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, adb.a_buffer[0], blockSizeIQ);   // difference of I and Q - LSB
            }
            else // USB
            {
                // all USB modes are demodulated the same way, we handed the special case DEMOD_SAM / SAM-U earlier
                arm_add_f32(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, adb.a_buffer[0], blockSizeIQ);   // sum of I and Q - USB
            }


            // at this point we have our demodulated audio signal in adb.a_buffer[0]
            // it may or may not need decimation before we go on with filtering the
            // audio signal

            if(dmod_mode != DEMOD_FM)       // are we NOT in FM mode?
            {
                // If we are not, do decimation if not already done, filtering, DSP notch/noise reduction, etc.
                if (use_decimatedIQ == false) // we did not already decimate the input earlier
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

                // at this point we are at the decimated audio sample rate
                // we support multiple rates
                // here also the various digital mode modems are called
                RxProcessor_DemodAudioPostprocessing(adb.a_buffer, blockSize, blockSizeDecim, ads.decimated_freq, use_stereo);
                // we get back blockSize audio at full sample rate

            } // end NOT in FM mode
            else
            {
                // it is FM - we don't do any decimation, interpolation, filtering or any other processing - just rescale audio amplitude
                // resulting audio signal is in adb.a_buffer[1]
                arm_scale_f32(
                        adb.a_buffer[0],
                        RadioManagement_FmDevIs5khz() ? FM_RX_SCALING_5K : FM_RX_SCALING_2K5,
                        adb.a_buffer[1],
                        blockSize);  // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
                AudioAgc_RunAgcWdsp(blockSize, adb.a_buffer, false); // FM is not using stereo
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

    // at this point we have audio at AUDIO_SAMPLE_RATE in our adb.a_buffer[1] (and [0] if we are in stereo)
    // if signal_active is true and we're ready to send it out

    bool do_mute_output = external_mute == true || (signal_active == false);

    if (do_mute_output)
    // fill audio buffers with zeroes if we are to mute the receiver completely while still processing data OR it is in FM and squelched
    // or when filters are switched
    {
        arm_fill_f32(0, adb.a_buffer[0], blockSize);
        arm_fill_f32(0, adb.a_buffer[1], blockSize);
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

    // we may have to play a key beep. We apply it to the left and right channel in OVI40 (on mcHF etc on left only because it is the speakers channel, not line out)
    if((ts.beep_timing > 0))         // is beep active?
    {
#ifdef USE_TWO_CHANNEL_AUDIO
        softdds_addSingleToneToTwobuffers(&ads.beep, adb.a_buffer[0],adb.a_buffer[1], blockSize, ads.beep_loudness_factor);
#else
        softdds_addSingleTone(&ads.beep, adb.a_buffer[1], blockSize, ads.beep_loudness_factor);
#endif
    }

    // Transfer processed audio to DMA buffer
    for(int i=0; i < blockSize; i++)                            // transfer to DMA buffer and do conversion to INT
    {

        if (do_mute_output)
        {
            dst[i].l = 0;
            dst[i].r = 0;
        }
        else
        {
        	dst[i].l = adb.a_buffer[1][i];
        	dst[i].r = adb.a_buffer[0][i];

        	// in case we have to scale up our values from 16 to 32 bit range. Yes, we don't use the lower bits from the float
        	// but that probably does not make a difference and this way it is faster.
        	// the halfword correction is required when we are running on a STM32F4 with 32bit IQ, which at the moment (!) implies a AUDIO_BIT_SHIFT
        	// so we are safe FOR NOW! When we adjust everything to 32bit, the correction has to remain but can be moved to the float to int conversion

        	if (AUDIO_BIT_SHIFT != 0)
        	{
        	    dst[i].l = I2S_correctHalfWord(dst[i].l << AUDIO_BIT_SHIFT);
                dst[i].r = I2S_correctHalfWord(dst[i].r << AUDIO_BIT_SHIFT);
        	}
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

        	UsbdAudio_PutSample(vals[0]);
            UsbdAudio_PutSample(vals[1]);
        }
    }
}

static void AudioDriver_AudioFillSilence(AudioSample_t *s, size_t size)
{
    memset(s,0,size*sizeof(*s));
}

static void AudioDriver_IqFillSilence(IqSample_t *s, size_t size)
{
    memset(s,0,size*sizeof(*s));
}

//*----------------------------------------------------------------------------
//* Function Name       : I2S_RX_CallBack
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioDriver_I2SCallback(AudioSample_t *audio, IqSample_t *iq, AudioSample_t *audioDst, int16_t blockSize)
{
    static bool to_rx = false;	// used as a flag to clear the RX buffer
    static bool to_tx = false;	// used as a flag to clear the TX buffer
    bool muted = false;

    if(ts.show_debug_info)
    {
        Board_GreenLed(LED_STATE_ON);
    }

    if((ts.txrx_mode == TRX_MODE_RX))
    {
        if((to_rx) || ts.audio_processor_input_mute_counter > 0)	 	// the first time back to RX, clear the buffers to reduce the "crash"
        {
            muted = true;
            AudioDriver_IqFillSilence(iq, blockSize);
            if (to_rx)
            {
                UhsdrHwI2s_Codec_ClearTxDmaBuffer();
            }
            if ( ts.audio_processor_input_mute_counter >0)
            {
                ts.audio_processor_input_mute_counter--;
            }
            to_rx = false;                          // caused by the content of the buffers from TX - used on return from SSB TX
        }

#ifdef USE_CONVOLUTION
        AudioDriver_RxProcessorConvolution(iq, audio, blockSize, muted);
#else
        AudioDriver_RxProcessor(iq, audio, blockSize, muted);
#endif
        if (ts.audio_dac_muting_buffer_count > 0)
        {
            ts.audio_dac_muting_buffer_count--;
        }

        if (muted == false)
        {
            if (ts.cw_keyer_mode != CW_KEYER_MODE_STRAIGHT && (ts.cw_text_entry || ts.dmod_mode == DEMOD_CW)) // FIXME to call always when straight mode reworked
            {
                CwGen_Process(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, blockSize);
            }
        }

        to_tx = true;		// Set flag to indicate that we WERE receiving when we go back to transmit mode
    }
    else  			// Transmit mode
    {
        if (to_tx)
        {
            TxProcessor_PrepareRun(); // last actions before we go live
        }
        if((to_tx) || (ts.audio_processor_input_mute_counter > 0) || ts.audio_dac_muting_flag || ts.audio_dac_muting_buffer_count > 0)	 	// the first time back to TX, or TX audio muting timer still active - clear the buffers to reduce the "crash"
        {
            muted = true;
            AudioDriver_AudioFillSilence(audio, blockSize);
            to_tx = false;                          // caused by the content of the buffers from TX - used on return from SSB TX
            if (ts.audio_processor_input_mute_counter > 0)
            {
                ts.audio_processor_input_mute_counter--;
            }
        }

        TxProcessor_Run(audio, iq, audioDst,blockSize, muted);

        // Pause or inactivity
        if (ts.audio_dac_muting_buffer_count)
        {
            ts.audio_dac_muting_buffer_count--;
        }

        to_rx = true;		// Set flag to indicate that we WERE transmitting when we eventually go back to receive mode
    }

    UiDriver_Callback_AudioISR();

    if(ts.show_debug_info)
    {
        Board_GreenLed(LED_STATE_OFF);
    }

#ifdef USE_PENDSV_FOR_HIGHPRIO_TASKS
    // let us trigger a pendsv irq here in order to trigger execution of UiDriver_HighPrioHandler()
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
#endif
}


