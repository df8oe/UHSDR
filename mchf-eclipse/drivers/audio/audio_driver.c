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
#include "mchf_board.h"
#include "profiling.h"

#include <stdio.h>
#include <math.h>
#include "codec.h"

#ifdef STM32F4
#include "i2s.h"
#endif

#include "cw_gen.h"

#include <limits.h>
#include "softdds.h"

#include "audio_driver.h"
#include "audio_management.h"
#include "dds_table.h"
#include "radio_management.h"
#include "usbd_audio_if.h"
#include "ui_spectrum.h"
#include "filters.h"
#include "ui_lcd_hy28.h"
#include "ui_configuration.h"
#include "ui_driver.h"
#include "mchf_hw_i2s.h"

typedef struct {
    __packed int16_t l;
    __packed int16_t r;
} AudioSample_t;

// SSB filters - now handled in ui_driver to allow I/Q phase adjustment


static inline void AudioDriver_TxFilterAudio(bool do_bandpass, bool do_bass_treble, float32_t* inBlock, float32_t* outBlock, const uint16_t blockSize);

typedef struct
{

    float32_t   errsig1[64+10];
    float32_t   errsig2[64+10];

    // LMS Filters for RX
    arm_lms_norm_instance_f32	lms1Norm_instance;
    arm_lms_instance_f32	    lms1_instance;
    float32_t	                lms1StateF32[DSP_NR_NUMTAPS_MAX + BUFF_LEN];
    float32_t	                lms1NormCoeff_f32[DSP_NR_NUMTAPS_MAX + BUFF_LEN];
    float32_t                   lms1_nr_delay[LMS_NR_DELAYBUF_SIZE_MAX+BUFF_LEN];

    arm_lms_norm_instance_f32	lms2Norm_instance;
    arm_lms_instance_f32	    lms2_instance;
    float32_t	                lms2StateF32[DSP_NOTCH_NUMTAPS_MAX + BUFF_LEN];
    float32_t	                lms2NormCoeff_f32[DSP_NOTCH_NUMTAPS_MAX + BUFF_LEN];
    float32_t	                lms2_nr_delay[LMS_NOTCH_DELAYBUF_SIZE_MAX + BUFF_LEN];
} LMSData;


float32_t	__MCHF_SPECIALMEM audio_delay_buffer	[AUDIO_DELAY_BUFSIZE];

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


//static   float32_t               NR_FFT_buffer[2048];
//static   float32_t               NR_iFFT_buffer[2048];

//
// Audio RX - Decimator
static  arm_fir_decimate_instance_f32   DECIMATE_RX;
float32_t           __MCHF_SPECIALMEM decimState[FIR_RXAUDIO_BLOCK_SIZE + 43];//FIR_RXAUDIO_NUM_TAPS];
// Audio RX - Decimator in Q-path
static  arm_fir_decimate_instance_f32   DECIMATE_RX_Q;
float32_t           __MCHF_SPECIALMEM decimQState[FIR_RXAUDIO_BLOCK_SIZE + 43]; //FIR_RXAUDIO_NUM_TAPS];

// Decimator for Zoom FFT
static	arm_fir_decimate_instance_f32	DECIMATE_ZOOM_FFT_I;
float32_t			__MCHF_SPECIALMEM decimZoomFFTIState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

// Decimator for Zoom FFT
static	arm_fir_decimate_instance_f32	DECIMATE_ZOOM_FFT_Q;
float32_t			__MCHF_SPECIALMEM decimZoomFFTQState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

// Audio RX - Interpolator
static	arm_fir_interpolate_instance_f32 INTERPOLATE_RX;
float32_t			__MCHF_SPECIALMEM interpState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

// variables for RX IIR filters
static float32_t		iir_rx_state[IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES];
static arm_iir_lattice_instance_f32	IIR_PreFilter;

// variables for RX antialias IIR filter
static float32_t		iir_aa_state[IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES];
static arm_iir_lattice_instance_f32	IIR_AntiAlias;


// static float32_t Koeff[20];
// variables for RX manual notch, manual peak & bass shelf IIR biquad filter
static arm_biquad_casd_df1_inst_f32 IIR_biquad_1 =
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

// variables for RX treble shelf IIR biquad filter
static arm_biquad_casd_df1_inst_f32 IIR_biquad_2 =
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
        } // 2 x 4 = 8 state variables
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
        } // 2 x 4 = 8 state variables
};

static float32_t* mag_coeffs[6] =

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
static float32_t	iir_squelch_rx_state[IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES];
static arm_iir_lattice_instance_f32	IIR_Squelch_HPF;

// variables for TX IIR filter
float32_t		iir_tx_state[IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES];
arm_iir_lattice_instance_f32	IIR_TXFilter;
arm_iir_lattice_instance_f32	IIR_FreeDV_RX_Filter;  //DL2FW: temporary installed FreeDV RX Audio Filter
float32_t		iir_FreeDV_RX_state[IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES];

// S meter public
__IO SMeter					sm;

// Keypad driver publics
extern __IO	KeypadState				ks;

// ATTENTION: These data structures have been placed in CCM Memory (64k)
// IF THE SIZE OF  THE DATA STRUCTURE GROWS IT WILL QUICKLY BE OUT OF SPACE IN CCM
// Be careful! Check mchf-eclipse.map for current allocation
__IO AudioDriverState   __MCHF_SPECIALMEM ads;
AudioDriverBuffer  __MCHF_SPECIALMEM adb;
LMSData            __MCHF_SPECIALMEM lmsData;

#ifdef USE_SNAP
SnapCarrier   sc;
#endif

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
void AudioDriver_SetupAGC(void);
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

//
//*----------------------------------------------------------------------------
//* Function Name       : audio_driver_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioDriver_Init(void)
{
    const uint32_t word_size = WORD_SIZE_16;

    // CW module init
    CwGen_Init();

    // Audio filter disabled
    ts.dsp_inhibit = 1;
    ads.af_disabled = 1;

    // Reset S meter public
    sm.skip		= 0;
    sm.s_count	= 0;
    sm.curr_max	= 0;
    sm.gain_calc = 0;	// gain calculation used for S-meter

    ads.agc_val = 1;			// Post AF Filter gain (AGC)
    ads.agc_var = 1;			// used in AGC processing
    ads.agc_calc = 1;			// used in AGC processing
    ads.agc_holder = 1;			// initialize holder for AGC value
    for(uint32_t x = 0; x < BUFF_LEN; x++)	// initialize running buffer for AGC delay
    {
        adb.agc_valbuf[x] = 1;
    }

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
    AudioManagement_CalcAGCDecay();	// initialize AGC decay ("hang time") values
    //
    AudioManagement_CalcRFGain();		// convert from user RF gain value to "working" RF gain value
    //
    AudioManagement_CalcALCDecay();	// initialize ALC decay values
    //
    AudioManagement_CalcAGCVals();	// calculate AGC internal values from user settings
    //
    AudioManagement_CalcNB_AGC();		// set up noise blanker AGC values
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


    ts.codec_present = Codec_Reset(ts.samp_rate,word_size) == HAL_OK;

    // Start DMA transfers
    MchfHw_Codec_StartDMA();


#ifdef USE_SNAP
    // initialize FFT structure used for snap carrier
    //	arm_rfft_init_f32((arm_rfft_instance_f32 *)&sc.S,(arm_cfft_radix4_instance_f32 *)&sc.S_CFFT,FFT_IQ_BUFF_LEN2,1,1);
    arm_rfft_fast_init_f32((arm_rfft_fast_instance_f32 *)&sc.S, FFT_IQ_BUFF_LEN2);
#endif

    // Audio filter enabled
    ads.af_disabled = 0;
    ts.dsp_inhibit = 0;

}

void AudioDriver_SetSamPllParameters()
{

    // definitions and intializations for synchronous AM demodulation = SAM
    //    adb.DF = 1.0; //ads.decimation_rate;
    adb.DF = ads.decimation_rate;
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
    adb.g1 = (1.0 - exp(-2.0 * adb.omegaN * adb.zeta * adb.DF / IQ_SAMPLE_RATE_F)); //0.0082987073611; //
    adb.g2 = (- adb.g1 + 2.0 * (1 - exp(- adb.omegaN * adb.zeta * adb.DF / IQ_SAMPLE_RATE_F)
            * cosf(adb.omegaN * adb.DF / IQ_SAMPLE_RATE_F * sqrtf(1.0 - adb.zeta * adb.zeta)))); //0.01036367597097734813032783691644; //
    //fade leveler
    //    ads.tauR_int = 20; // -->  / 1000 = 0.02
    //    ads.tauI_int = 140; // --> / 100 = 1.4
    adb.tauR = 0.02; // ((float32_t)ads.tauR_int) / 1000.0; //0.02; // original 0.02;
    adb.tauI = 1.4; // ((float32_t)ads.tauI_int) / 100.0; //1.4; // original 1.4;
    adb.mtauR = (exp(- adb.DF / (IQ_SAMPLE_RATE_F * adb.tauR))); //0.99948;
    adb.onem_mtauR = (1.0 - adb.mtauR);
    adb.mtauI = (exp(- adb.DF / (IQ_SAMPLE_RATE_F * adb.tauI))); //0.99999255955;
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

#define B0 0
#define B1 1
#define B2 2
#define A1 3
#define A2 4
#define A0 5

void AudioDriver_SetBiquadCoeffs(float32_t* coeffsTo,const float32_t* coeffsFrom, float scaling)
{
    coeffsTo[0] = coeffsFrom[0]/scaling;
    coeffsTo[1] = coeffsFrom[1]/scaling;
    coeffsTo[2] = coeffsFrom[2]/scaling;
    coeffsTo[3] = coeffsFrom[3]/scaling;
    coeffsTo[4] = coeffsFrom[4]/scaling;
}

void AudioDriver_CalcHighShelf(float32_t coeffs[6], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t A = powf(10.0,gain/40.0); // gain ranges from -20 to 5
    float32_t alpha = sin(w0) / 2 * sqrt( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cos(w0);
    float32_t twoAa = 2 * sqrt(A) * alpha;
    // highShelf
    //
    coeffs[B0] = A *        ( (A + 1) + (A - 1) * cosw0 + twoAa );
    coeffs[B1] = - 2 * A *  ( (A - 1) + (A + 1) * cosw0         );
    coeffs[B2] = A *        ( (A + 1) + (A - 1) * cosw0 - twoAa );
    coeffs[A0] =              (A + 1) - (A - 1) * cosw0 + twoAa ;
    coeffs[A1] = - 2 *      ( (A - 1) - (A + 1) * cosw0         ); // already negated!
    coeffs[A2] = twoAa      - (A + 1) + (A - 1) * cosw0; // already negated!


    //    DCgain = 2; //
    //    DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (- coeffs[A1] - coeffs[A2])); // takes into account that coeffs[A1] and coeffs[A2] are already negated!
    float32_t DCgain = 1; //

    coeffs[B0] = coeffs[B0] / DCgain;
    coeffs[B1] = coeffs[B1] / DCgain;
    coeffs[B2] = coeffs[B2] / DCgain;

}

void AudioDriver_CalcLowShelf(float32_t coeffs[6], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{

    float32_t w0 = 2 * PI * f0 / FS;
    float32_t A = powf(10.0,gain/40.0); // gain ranges from -20 to 5

    float32_t alpha = sin(w0) / 2 * sqrt( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cos(w0);
    float32_t twoAa = 2 * sqrt(A) * alpha;

    // lowShelf
    coeffs[B0] = A *        ( (A + 1) - (A - 1) * cosw0 + twoAa );
    coeffs[B1] = 2 * A *    ( (A - 1) - (A + 1) * cosw0         );
    coeffs[B2] = A *        ( (A + 1) - (A - 1) * cosw0 - twoAa );
    coeffs[A0] =              (A + 1) + (A - 1) * cosw0 + twoAa ;
    coeffs[A1] = 2 *        ( (A - 1) + (A + 1) * cosw0         ); // already negated!
    coeffs[A2] = twoAa      - (A + 1) - (A - 1) * cosw0; // already negated!

    // scaling the feedforward coefficients for gain adjustment !
    // "DC gain of an IIR filter is the sum of the filters� feedforward coeffs divided by
    // 1 minus the sum of the filters� feedback coeffs" (Lyons 2011)
    //    float32_t DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (coeffs[A1] + coeffs[A2]));
    // does not work for some reason?
    // I take a divide by a constant instead !
    float32_t DCgain = 1; //
    //    DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (- coeffs[A1] - coeffs[A2])); // takes into account that coeffs[A1] and coeffs[A2] are already negated!
    coeffs[B0] = coeffs[B0] / DCgain;
    coeffs[B1] = coeffs[B1] / DCgain;
    coeffs[B2] = coeffs[B2] / DCgain;


}

static const float32_t biquad_passthrough[] = { 1, 0, 0, 0, 0 };

void AudioDriver_SetRxTxAudioProcessingAudioFilters(uint8_t dmod_mode)
{
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
    float32_t FSdec = 24000.0; // we need the sampling rate in the decimated path for calculation of the coefficients

    if (FilterPathInfo[ts.filter_path].sample_rate_dec == RX_DECIMATION_RATE_12KHZ)
    {
        FSdec = 12000.0;
    }

    const float32_t FS = IQ_SAMPLE_RATE; // we need this for the treble filter

    // the notch filter is in biquad 1 and works at the decimated sample rate FSdec

    float32_t coeffs[5];

    // setting the Coefficients in the notch filter instance
    // while not using pointers
    if (ts.dsp_active & DSP_MNOTCH_ENABLE)
    {
        float32_t f0 = ts.notch_frequency;
        float32_t Q = 10; // larger Q gives narrower notch
        float32_t w0 = 2 * PI * f0 / FSdec;
        float32_t alpha = sin(w0) / (2 * Q);

        coeffs[B0] = 1;
        coeffs[B1] = - 2 * cos(w0);
        coeffs[B2] = 1;
        coeffs[A0] = 1 + alpha;
        coeffs[A1] = 2 * cos(w0); // already negated!
        coeffs[A2] = alpha - 1; // already negated!

        AudioDriver_SetBiquadCoeffs(&IIR_biquad_1.pCoeffs[0],coeffs,coeffs[A0]);
    }
    else   // passthru
    {
        AudioDriver_SetBiquadCoeffs(&IIR_biquad_1.pCoeffs[0],biquad_passthrough,1);
    }

    // the peak filter is in biquad 1 and works at the decimated sample rate FSdec
    if(ts.dsp_active & DSP_MPEAK_ENABLE)
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
        float32_t f0 = ts.peak_frequency;
        float32_t Q = 4; //
        float32_t BW = 0.03;
        float32_t w0 = 2 * PI * f0 / FSdec;
        float32_t alpha = sin (w0) * sinh( log(2) / 2 * BW * w0 / sin(w0) ); //

        coeffs[B0] = Q * alpha;
        coeffs[B1] = 0;
        coeffs[B2] = - Q * alpha;
        coeffs[A0] = 1 + alpha;
        coeffs[A1] = 2 * cos(w0); // already negated!
        coeffs[A2] = alpha - 1; // already negated!

        AudioDriver_SetBiquadCoeffs(&IIR_biquad_1.pCoeffs[5],coeffs,coeffs[A0]);
    }
    else   //passthru
    {
        AudioDriver_SetBiquadCoeffs(&IIR_biquad_1.pCoeffs[5],biquad_passthrough,1);
    }

    // EQ shelving filters
    //
    // the bass filter is in biquad 1 and works at the decimated sample rate FSdec
    //
    // Bass
    // lowShelf
    AudioDriver_CalcLowShelf(coeffs, 250, 0.7, ts.bass_gain, FSdec);
    AudioDriver_SetBiquadCoeffs(&IIR_biquad_1.pCoeffs[10],coeffs,coeffs[A0]);

    // Treble = highShelf
    //
    // the treble filter is in biquad 2 and works at 48000ksps
    AudioDriver_CalcHighShelf(coeffs, 3500, 0.9, ts.treble_gain, FS);
    AudioDriver_SetBiquadCoeffs(&IIR_biquad_2.pCoeffs[0],coeffs,coeffs[A0]);

    // insert coefficient calculation for TX bass & treble adjustment here!
    // the TX treble filter is in IIR_TX_biquad and works at 48000ksps
    AudioDriver_CalcHighShelf(coeffs, 1700, 0.9, ts.tx_treble_gain, FS);
    AudioDriver_SetBiquadCoeffs(&IIR_TX_biquad.pCoeffs[0],coeffs,coeffs[A0]);

    // the TX bass filter is in TX_biquad and works at 48000 sample rate
    AudioDriver_CalcLowShelf(coeffs, 300, 0.7, ts.tx_bass_gain, FS);
    AudioDriver_SetBiquadCoeffs(&IIR_TX_biquad.pCoeffs[5],coeffs,coeffs[A0]);
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

    float	mu_calc;

    ts.dsp_inhibit++;
    ads.af_disabled++;

    // make sure we have a proper filter path for the given mode

    // the commented out part made the code  only look at last used/selected filter path if the current filter path is not applicable
    // with it commented out the filter path is ALWAYS loaded from the last used/selected memory
    // I.e. setting the ts.filter_path anywere else in the code is useless. You have to call AudioFilter_NextApplicableFilterPath in order to
    // select a new filter path as this sets the last used/selected memory for a demod mode.

    ts.filter_path = AudioFilter_NextApplicableFilterPath(PATH_ALL_APPLICABLE|PATH_LAST_USED_IN_MODE,AudioFilter_GetFilterModeFromDemodMode(dmod_mode),ts.filter_path);

    if (FilterPathInfo[ts.filter_path].pre_instance != NULL)
    {
        // if we turn on a filter, set the number of members to the number of elements last
        IIR_PreFilter.pkCoeffs = FilterPathInfo[ts.filter_path].pre_instance->pkCoeffs; // point to reflection coefficients
        IIR_PreFilter.pvCoeffs = FilterPathInfo[ts.filter_path].pre_instance->pvCoeffs; // point to ladder coefficients
        IIR_PreFilter.numStages = FilterPathInfo[ts.filter_path].pre_instance->numStages;        // number of stages
    }
    else
    {
        // if we turn off a filter, set the number of members to 0 first
        IIR_PreFilter.numStages = 0;        // number of stages
        IIR_PreFilter.pkCoeffs = NULL; // point to reflection coefficients
        IIR_PreFilter.pvCoeffs = NULL; // point to ladder coefficients
    }

    // Initialize IIR filter state buffer
    arm_fill_f32(0.0,iir_rx_state,IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES);

    IIR_PreFilter.pState = iir_rx_state;					// point to state array for IIR filter

    // Initialize IIR antialias filter state buffer
    if (FilterPathInfo[ts.filter_path].iir_instance != NULL)
    {
        // if we turn on a filter, set the number of members to the number of elements last
        IIR_AntiAlias.pkCoeffs = FilterPathInfo[ts.filter_path].iir_instance->pkCoeffs; // point to reflection coefficients
        IIR_AntiAlias.pvCoeffs = FilterPathInfo[ts.filter_path].iir_instance->pvCoeffs; // point to ladder coefficients
        IIR_AntiAlias.numStages = FilterPathInfo[ts.filter_path].iir_instance->numStages;        // number of stages
    }
    else
    {
        // if we turn off a filter, set the number of members to 0 first
        IIR_AntiAlias.numStages = 0;
        IIR_AntiAlias.pkCoeffs = NULL;
        IIR_AntiAlias.pvCoeffs = NULL;
    }

    arm_fill_f32(0.0,iir_aa_state,IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES);
    IIR_AntiAlias.pState = iir_aa_state;					// point to state array for IIR filter


    // TODO: We only have to do this, if the audio signal filter configuration changes
    // RX+ TX Bass, Treble, Peak, Notch
    AudioDriver_SetRxTxAudioProcessingAudioFilters(dmod_mode);

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
    arm_fill_f32(0.0,iir_squelch_rx_state,IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES);

    // Initialize LMS (DSP Noise reduction) filter
    // It is (sort of) initalized "twice" since this it what it seems to take for the LMS function to
    // start reliably and consistently!
    //
    uint16_t	calc_taps = ts.dsp_nr_numtaps;
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

    arm_fill_f32(0.0,lmsData.lms2_nr_delay,LMS_NOTCH_DELAYBUF_SIZE_MAX + BUFF_LEN);
    arm_fill_f32(0.0,lmsData.lms2StateF32,DSP_NOTCH_NUMTAPS_MAX + BUFF_LEN);

    if(reset_dsp_nr)             // are we to reset the coefficient buffer as well?
    {
        arm_fill_f32(0.0,lmsData.lms2NormCoeff_f32,DSP_NOTCH_NUMTAPS_MAX + BUFF_LEN);      // yes - zero coefficient buffers
    }

    if((ts.dsp_notch_delaybuf_len > DSP_NOTCH_BUFLEN_MAX) || (ts.dsp_notch_delaybuf_len < DSP_NOTCH_BUFLEN_MIN))
    {
        ts.dsp_nr_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
    }
    // AUTO NOTCH INIT END

    // Adjust decimation rate based on selected filter
    ads.decimation_rate = FilterPathInfo[ts.filter_path].sample_rate_dec;

    ads.agc_decimation_scaling = ads.decimation_rate;
    ads.agc_delay_buflen = AUDIO_DELAY_BUFSIZE/(ulong)ads.decimation_rate;	// calculate post-AGC delay based on post-decimation sampling rate

    // Set up ZOOM FFT FIR decimation filters
    // switch right FIR decimation filter depending on sd.magnify
    if(sd.magnify > 5)
    {
        sd.magnify = 0;
    }

    DECIMATE_ZOOM_FFT_I.numTaps = FirZoomFFTDecimate[sd.magnify].numTaps;
    DECIMATE_ZOOM_FFT_I.pCoeffs = FirZoomFFTDecimate[sd.magnify].pCoeffs;
    DECIMATE_ZOOM_FFT_I.M = (1 << sd.magnify);          // Decimation factor
    DECIMATE_ZOOM_FFT_I.pState = decimZoomFFTIState;            // Filter state variables
    arm_fill_f32(0.0,decimZoomFFTIState,FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS);

    DECIMATE_ZOOM_FFT_Q.numTaps = FirZoomFFTDecimate[sd.magnify].numTaps;
    DECIMATE_ZOOM_FFT_Q.pCoeffs = FirZoomFFTDecimate[sd.magnify].pCoeffs;
    DECIMATE_ZOOM_FFT_Q.M = (1 << sd.magnify);			// Decimation factor
    DECIMATE_ZOOM_FFT_Q.pState = decimZoomFFTQState;			// Filter state variables
    arm_fill_f32(0.0,decimZoomFFTQState,FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS);

    // Set up RX decimation/filter
    if (FilterPathInfo[ts.filter_path].dec != NULL)
    {
        DECIMATE_RX.numTaps = FilterPathInfo[ts.filter_path].dec->numTaps;      // Number of taps in FIR filter
        DECIMATE_RX.pCoeffs = FilterPathInfo[ts.filter_path].dec->pCoeffs;       // Filter coefficients
        DECIMATE_RX_Q.numTaps = FilterPathInfo[ts.filter_path].dec->numTaps;      // Number of taps in FIR filter
        DECIMATE_RX_Q.pCoeffs = FilterPathInfo[ts.filter_path].dec->pCoeffs;       // Filter coefficients
    }
    else
    {
        DECIMATE_RX.numTaps = 0;
        DECIMATE_RX.pCoeffs = NULL;
        DECIMATE_RX_Q.numTaps = 0;
        DECIMATE_RX_Q.pCoeffs = NULL;
    }

    DECIMATE_RX.M = ads.decimation_rate;            // Decimation factor  (48 kHz / 4 = 12 kHz)
    DECIMATE_RX.pState = decimState;            // Filter state variables
    arm_fill_f32(0.0,decimState,FIR_RXAUDIO_BLOCK_SIZE + 43);
    DECIMATE_RX_Q.M = ads.decimation_rate;            // Decimation factor  (48 kHz / 4 = 12 kHz)
    DECIMATE_RX_Q.pState = decimQState;            // Filter state variables
    arm_fill_f32(0.0,decimQState,FIR_RXAUDIO_BLOCK_SIZE + 43);


    // Set up RX interpolation/filter
    // NOTE:  Phase Length MUST be an INTEGER and is the number of taps divided by the decimation rate, and it must be greater than 1.
    if (FilterPathInfo[ts.filter_path].interpolate != NULL)
    {
        INTERPOLATE_RX.pCoeffs = FilterPathInfo[ts.filter_path].interpolate->pCoeffs; // Filter coefficients
        INTERPOLATE_RX.phaseLength = FilterPathInfo[ts.filter_path].interpolate->phaseLength/ads.decimation_rate;    // Phase Length ( numTaps / L )
    }
    else
    {
        INTERPOLATE_RX.phaseLength = 0;
        INTERPOLATE_RX.pCoeffs = NULL;
    }

    INTERPOLATE_RX.L = ads.decimation_rate;         // Interpolation factor, L  (12 kHz * 4 = 48 kHz)
    INTERPOLATE_RX.pState = interpState;        // Filter state variables
    arm_fill_f32(0.0,interpState,FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS);


    ads.dsp_zero_count = 0;		// initialize "zero" count to detect if DSP has crashed

    // if (dmod_mode == DEMOD_AM || dmod_mode == DEMOD_SAM)
    {
        AudioDriver_SetRxAudioProcessingSAM(dmod_mode);
    }

    AudioFilter_InitRxHilbertFIR(dmod_mode); // this switches the Hilbert/FIR-filters

    AudioDriver_SetupAGC();

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

    IIR_TXFilter.numStages = IIR_TXFilterSelected_ptr->numStages;       // number of stages
    IIR_TXFilter.pkCoeffs = IIR_TXFilterSelected_ptr->pkCoeffs; // point to reflection coefficients
    IIR_TXFilter.pvCoeffs = IIR_TXFilterSelected_ptr->pvCoeffs; // point to ladder coefficients
    IIR_TXFilter.pState = iir_tx_state;
    arm_fill_f32(0.0,iir_tx_state,IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES);

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
    arm_fill_f32(0.0,iir_FreeDV_RX_state,IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES);

}

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

//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_freq_conv [KA7OEI]
//* Object              : Does I/Q frequency conversion
//* Object              :
//* Input Parameters    : size of array on which to work; dir: determines direction of shift - see below;  Also uses variables in ads structure
//* Output Parameters   : uses variables in ads structure
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_FreqConversion(int16_t blockSize, int16_t dir)
{
    static bool recalculate_Osc = false;
    float32_t hh1 = 0.0;
    float32_t hh2 = 0.0;
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
    	for(i = 0; i < size/2; i++)	{
    		// generate local oscillator on-the-fly:  This takes a lot of processor time!
    		ads.Osc_Q = (ads.Osc_Vect_Q * ads.Osc_Cos) - (ads.Osc_Vect_I * ads.Osc_Sin);	// Q channel of oscillator
    		ads.Osc_I = (ads.Osc_Vect_I * ads.Osc_Cos) + (ads.Osc_Vect_Q * ads.Osc_Sin);	// I channel of oscillator
    		ads.Osc_Gain = 1.95 - ((ads.Osc_Vect_Q * ads.Osc_Vect_Q) + (ads.Osc_Vect_I * ads.Osc_Vect_I));	// Amplitude control of oscillator
    		// rotate vectors while maintaining constant oscillator amplitude
    		ads.Osc_Vect_Q = ads.Osc_Gain * ads.Osc_Q;
    		ads.Osc_Vect_I = ads.Osc_Gain * ads.Osc_I;
    		//
    		// do actual frequency conversion
    		i_temp = adb.i_buffer[i];	// save temporary copies of data
    		q_temp = adb.q_buffer[i];
    		adb.i_buffer[i] = (i_temp * ads.Osc_Q) + (q_temp * ads.Osc_I);	// multiply I/Q data by sine/cosine data to do translation
    		adb.q_buffer[i] = (q_temp * ads.Osc_Q) - (i_temp * ads.Osc_I);
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
        for(int i = 0; i < blockSize; i++)	 		// No, let's do it!
        {
            float32_t   rad_calc;
            rad_calc = i;		// convert to float the current position within the buffer
            rad_calc /= (blockSize);			// make this a fraction
            rad_calc *= (PI * 2);			// convert to radians
            rad_calc *= ts.multi;			// multiply by number of cycles that we want within this block (4 = 6 kHz offset)
            //
            sincosf(rad_calc, &adb.Osc_I_buffer[i], &adb.Osc_Q_buffer[i]);
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
                hh1 = - adb.q_buffer[i + 1];
                hh2 =   adb.i_buffer[i + 1];
                adb.i_buffer[i + 1] = hh1;
                adb.q_buffer[i + 1] = hh2;
                // xnew(2) = -xreal(2) - jximag(2)
                hh1 = - adb.i_buffer[i + 2];
                hh2 = - adb.q_buffer[i + 2];
                adb.i_buffer[i + 2] = hh1;
                adb.q_buffer[i + 2] = hh2;
                // xnew(3) = + ximag(3) - jxreal(3)
                hh1 =   adb.q_buffer[i + 3];
                hh2 = - adb.i_buffer[i + 3];
                adb.i_buffer[i + 3] = hh1;
                adb.q_buffer[i + 3] = hh2;
            }

        }

        else // dir == 0
        {
            // this is for -Fs/4 [moves receive frequency to the right in the spectrum display]
            for(int i = 0; i < blockSize; i += 4)
            {   // xnew(0) =  xreal(0) + jximag(0)
                // leave as it is!
                // xnew(1) =  ximag(1) - jxreal(1)
                hh1 = adb.q_buffer[i + 1];
                hh2 = - adb.i_buffer[i + 1];
                adb.i_buffer[i + 1] = hh1;
                adb.q_buffer[i + 1] = hh2;
                // xnew(2) = -xreal(2) - jximag(2)
                hh1 = - adb.i_buffer[i + 2];
                hh2 = - adb.q_buffer[i + 2];
                adb.i_buffer[i + 2] = hh1;
                adb.q_buffer[i + 2] = hh2;
                // xnew(3) = -ximag(3) + jxreal(3)
                hh1 = - adb.q_buffer[i + 3];
                hh2 = adb.i_buffer[i + 3];
                adb.i_buffer[i + 3] = hh1;
                adb.q_buffer[i + 3] = hh2;
            }

        }
    }
    else  // frequency translation +6kHz or -6kHz
    {
        // Do frequency conversion using optimized ARM math functions [KA7OEI]
        arm_mult_f32(adb.q_buffer, adb.Osc_Q_buffer, adb.c_buffer, blockSize); // multiply products for converted I channel
        arm_mult_f32(adb.i_buffer, adb.Osc_I_buffer, adb.d_buffer, blockSize);
        arm_mult_f32(adb.q_buffer, adb.Osc_I_buffer, adb.e_buffer, blockSize);
        arm_mult_f32(adb.i_buffer, adb.Osc_Q_buffer, adb.f_buffer, blockSize);    // multiply products for converted Q channel

        if(!dir)	 	// Conversion is "above" on RX (LO needs to be set lower)
        {
            arm_add_f32(adb.f_buffer, adb.e_buffer, adb.i_buffer, blockSize);	// summation for I channel
            arm_sub_f32(adb.c_buffer, adb.d_buffer, adb.q_buffer, blockSize);	// difference for Q channel
        }
        else	 	// Conversion is "below" on RX (LO needs to be set higher)
        {
            arm_add_f32(adb.c_buffer, adb.d_buffer, adb.q_buffer, blockSize);	// summation for I channel
            arm_sub_f32(adb.f_buffer, adb.e_buffer, adb.i_buffer, blockSize);	// difference for Q channel
        }
    }
}

#ifdef USE_FREEDV
static bool AudioDriver_RxProcessorFreeDV (AudioSample_t * const src, AudioSample_t * const dst, int16_t blockSize)
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


    if (ts.digital_mode==1)
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
                    fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].real = ((int32_t)adb.q_buffer[k]);
                    fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].imag = ((int32_t)adb.i_buffer[k]);
                }
                else
                {
                    fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].imag = ((int32_t)adb.q_buffer[k]);
                    fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].real = ((int32_t)adb.i_buffer[k]);
                }

                trans_count_in++;
            }
        }

        modulus_NF += 4; //  shift modulus to not loose any data while overlapping
        modulus_NF %= 6;//  reset modulus to 0 at modulus = 12

        if (trans_count_in == FDV_BUFFER_SIZE) //yes, we really hit exactly 320 - don't worry
        {
            //we have enough samples ready to start the FreeDV encoding

            fdv_iq_buffer_add(&fdv_iq_buff[FDV_TX_fill_in_pt]);
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

            arm_fill_f32(0,adb.b_buffer,blockSize);

            // UPSAMPLING [by hand]
            for (int j = 0; j < blockSize; j++) //  now we are doing upsampling by 6
            {
                if (modulus_MOD == 0) // put in sample pair
                {
                    adb.b_buffer[j] = out_buffer->samples[outbuff_count]; // + (sample_delta.real * (float32_t)modulus_MOD);
                    //sample_delta = (out_buffer->samples[outbuff_count]-last_sample)/6;
                }

                //adb.b_buffer[j] = out_buffer->samples[outbuff_count] + sample_delta*(float32_t)modulus_MOD;


#if 0
                else // in 5 of 6 cases just stuff in zeros = zero-padding / zero-stuffing
                    // or keep the last sample value - like a sample an' old
                {
                    //adb.b_buffer[j] = 0;
                    adb.b_buffer[j] = out_buffer->samples[outbuff_count];
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
            arm_iir_lattice_f32(&IIR_FreeDV_RX_Filter, adb.b_buffer,adb.b_buffer, blockSize);

            //AudioDriver_tx_filter_audio(true,false, adb.b_buffer,adb.b_buffer, blockSize);


        }
        else
        {
            profileEvent(FreeDVTXUnderrun);
            // in case of underrun -> produce silence
            arm_fill_f32(0,adb.b_buffer,blockSize);
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
                adb.b_buffer[j]=
                        FreeDV_FIR_interpolate[5-mod_count]*out_buffer->samples[outbuff_count] +
                        FreeDV_FIR_interpolate[11-mod_count]*out_buffer->samples[outbuff_count+1]+
                        FreeDV_FIR_interpolate[17-mod_count]*out_buffer->samples[outbuff_count+2]+
                        FreeDV_FIR_interpolate[23-mod_count]*out_buffer->samples[outbuff_count+3];
                // here we are actually calculation the interpolation for the current "up"-sample
            }
            else
            {
                //we are at an overlapping region and have to take care of history
                if (outbuff_count == -3)
                {
                    adb.b_buffer[j] =
                            FreeDV_FIR_interpolate[5-mod_count] * History[0] +
                            FreeDV_FIR_interpolate[11-mod_count] * History[1] +
                            FreeDV_FIR_interpolate[17-mod_count] * History[2] +
                            FreeDV_FIR_interpolate[23-mod_count] * out_buffer->samples[0];
                }
                else
                {
                    if (outbuff_count == -2)
                    {
                        adb.b_buffer[j] =
                                FreeDV_FIR_interpolate[5-mod_count] * History[1] +
                                FreeDV_FIR_interpolate[11-mod_count] * History[2] +
                                FreeDV_FIR_interpolate[17-mod_count] * out_buffer->samples[0] +
                                FreeDV_FIR_interpolate[23-mod_count] * out_buffer->samples[1];
                    }
                    else
                    {
                        adb.b_buffer[j] =
                                FreeDV_FIR_interpolate[5-mod_count] * History[2] +
                                FreeDV_FIR_interpolate[11-mod_count] * out_buffer->samples[0] +
                                FreeDV_FIR_interpolate[17-mod_count] * out_buffer->samples[1] +
                                FreeDV_FIR_interpolate[23-mod_count] * out_buffer->samples[2];
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
        arm_fill_f32(0,adb.b_buffer,blockSize);
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

/*******************************************************************************************************************
 *  AGC WDSP TEST
 *  code taken from wdsp lib by Warren Pratt
 *  http://svn.tapr.org/repos_sdr_hpsdr/trunk/W5WC/PowerSDR_HPSDR_mRX_PS/Source/wdsp/
 *  the AGC code is licensed under the GPL license
 *******************************************************************************************************************/
// AGC
//#define MAX_SAMPLE_RATE     (24000.0)
//#define MAX_N_TAU           (8)
//#define MAX_TAU_ATTACK      (0.01)
//#define RB_SIZE       (int) (MAX_SAMPLE_RATE * MAX_N_TAU * MAX_TAU_ATTACK + 1)
//int8_t AGC_mode = 2;
int pmode = 1; // if 0, calculate magnitude by max(|I|, |Q|), if 1, calculate sqrtf(I*I+Q*Q)
float32_t out_sample[2];
float32_t abs_out_sample;
float32_t tau_attack;
float32_t tau_decay;
int n_tau;
float32_t max_gain;
float32_t var_gain;
float32_t fixed_gain = 1.0;
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
float32_t ring[96];
float32_t abs_ring[96];
//assign constants
int ring_buffsize = 96;
//do one-time initialization
int out_index = -1;
float32_t ring_max = 0.0;
float32_t volts = 0.0;
float32_t save_volts = 0.0;
float32_t fast_backaverage = 0.0;
float32_t hang_backaverage = 0.0;
int hang_counter = 0;
uint8_t decay_type = 0;
uint8_t state = 0;
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

void AudioDriver_SetupAGC()
{
    float32_t tmp;
    float32_t sample_rate = IQ_SAMPLE_RATE_F / ads.decimation_rate;
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
    tau_attack = 0.001;               // tau_attack
    //    tau_decay = ts.agc_wdsp_tau_decay / 1000.0; // 0.250;                // tau_decay
    n_tau = 4;                        // n_tau

    //    max_gain = 1000.0; // 1000.0; determines the AGC threshold = knee level
    //  max_gain is powf (10.0, (float32_t)ts.agc_wdsp_thresh / 20.0);
    //    fixed_gain = ads.agc_rf_gain; //0.7; // if AGC == OFF, this gain is used
    max_input = (float32_t)ADC_CLIP_WARN_THRESHOLD * 2.0; // which is 8192 at the moment
    //32767.0; // maximum value of 16-bit audio //  1.0; //
    out_targ = (float32_t)ADC_CLIP_WARN_THRESHOLD; // 4096, tweaked, so that volume when switching between the two AGCs remains equal
    //12000.0; // target value of audio after AGC
    //    var_gain = 32.0;  // slope of the AGC --> this is 10 * 10^(slope / 20) --> for 10dB slope, this is 30.0
    var_gain = powf (10.0, (float32_t)ts.agc_wdsp_slope / 200.0); // 10 * 10^(slope / 20)
    tau_fast_backaverage = 0.250;    // tau_fast_backaverage
    tau_fast_decay = 0.005;          // tau_fast_decay
    pop_ratio = 5.0;                 // pop_ratio
    //    hang_enable = 0;                 // hang_enable
    tau_hang_backmult = 0.500;       // tau_hang_backmult
    //    hangtime = 0.250;                // hangtime
    hangtime = (float32_t)ts.agc_wdsp_hang_time / 1000.0;
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
            hangtime = 2.000;
            //      ts.agc_wdsp_tau_decay = 2000;
            //      hang_thresh = 1.0;
            //      ts.agc_wdsp_hang_enable = 1;
            break;
        case 2: //agcSLOW
            hangtime = 1.000;
            //      hang_thresh = 1.0;
            //      ts.agc_wdsp_tau_decay = 500;
            //      ts.agc_wdsp_hang_enable = 1;
            break;
        case 3: //agcMED
            //      hang_thresh = 1.0;
            hangtime = 0.250;
            //      ts.agc_wdsp_tau_decay = 250;
            break;
        case 4: //agcFAST
            //      hang_thresh = 1.0;
            hangtime = 0.100;
            //      ts.agc_wdsp_tau_decay = 50;
            break;
        case 0: //agcFrank --> very long
            //      ts.agc_wdsp_hang_enable = 0;
            //      hang_thresh = 0.300; // from which level on should hang be enabled
            hangtime = 3.000; // hang time, if enabled
            tau_hang_backmult = 0.500; // time constant exponential averager
            //      ts.agc_wdsp_tau_decay = 4000; // time constant decay long
            tau_fast_decay = 0.05;          // tau_fast_decay
            tau_fast_backaverage = 0.250; // time constant exponential averager
            break;
        default:
            break;
        }
        ts.agc_wdsp_switch_mode = 0;
    }
    //  float32_t noise_offset = 10.0 * log10f(fhigh - rxa[channel].nbp0.p->flow)
    //          * size / rate);
    //  max_gain = out_target / var_gain * powf (10.0, (thresh + noise_offset) / 20.0));
    tau_hang_decay = (float32_t)ts.agc_wdsp_tau_hang_decay / 1000.0;
    tau_decay = (float32_t)ts.agc_wdsp_tau_decay[ts.agc_wdsp_mode] / 1000.0;
    max_gain = powf (10.0, (float32_t)ts.agc_wdsp_thresh / 20.0);
    fixed_gain = max_gain / 10.0;
    attack_buffsize = (int)ceil(sample_rate * n_tau * tau_attack); // 48
    in_index = attack_buffsize + out_index;
    attack_mult = 1.0 - expf(-1.0 / (sample_rate * tau_attack));
    decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_decay));
    fast_decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_fast_decay));
    fast_backmult = 1.0 - expf(-1.0 / (sample_rate * tau_fast_backaverage));

    onemfast_backmult = 1.0 - fast_backmult;

    out_target = out_targ * (1.0 - expf(-(float32_t)n_tau)) * 0.9999;
    //  out_target = out_target * (1.0 - expf(-(float32_t)n_tau)) * 0.9999;
    min_volts = out_target / (var_gain * max_gain);
    inv_out_target = 1.0 / out_target;

    tmp = log10f(out_target / (max_input * var_gain * max_gain));
    if (tmp == 0.0)
        tmp = 1e-16;
    slope_constant = (out_target * (1.0 - 1.0 / var_gain)) / tmp;

    inv_max_input = 1.0 / max_input;

    if (max_input > min_volts)
    {
        float32_t convert = powf (10.0, (float32_t)ts.agc_wdsp_hang_thresh / 20.0);
        tmp = (convert - min_volts) / (max_input - min_volts);
        if(tmp < 1e-8) tmp = 1e-8;
        hang_thresh = 1.0 + 0.125 * log10f (tmp);
    }
    else
    {
        hang_thresh = 1.0;
    }

    tmp = powf (10.0, (hang_thresh - 1.0) / 0.125);
    hang_level = (max_input * tmp + (out_target /
            (var_gain * max_gain)) * (1.0 - tmp)) * 0.637;

    hang_backmult = 1.0 - expf(-1.0 / (sample_rate * tau_hang_backmult));
    onemhang_backmult = 1.0 - hang_backmult;

    hang_decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_hang_decay));
}


void AudioDriver_RxAGCWDSP(int16_t blockSize)
{
    // TODO:
    // "LED" that indicates that the AGC starts working (input signal above the "knee") --> has to be seen when in menu mode
    // --> DONE
    // hang time adjust
    // hang threshold adjust
    // "LED" that indicates that the input signal is higher than the hang threshold --> has to be seen when in menu mode
    //
    // Be careful: the original source code has no comments,
    // all comments added by DD4WH, February 2017: comments could be wrong, misinterpreting or highly misleading!
    //
    static float32_t    w = 0.0;
    static float32_t    wold = 0.0;
    int i, j, k;
    float32_t mult;

    if (ts.agc_wdsp_mode == 5)  // AGC OFF
    {
        for (i = 0; i < blockSize; i++)
        {
            adb.a_buffer[i] = adb.a_buffer[i] * fixed_gain;
        }
        return;
    }

    for (i = 0; i < blockSize; i++)
    {
        if (++out_index >= ring_buffsize)
            out_index -= ring_buffsize;
        if (++in_index >= ring_buffsize)
            in_index -= ring_buffsize;

        out_sample[0] = ring[out_index];
        abs_out_sample = abs_ring[out_index];
        ring[in_index] = adb.a_buffer[i];
        abs_ring[in_index] = fabs(adb.a_buffer[i]);

        fast_backaverage = fast_backmult * abs_out_sample + onemfast_backmult * fast_backaverage;
        hang_backaverage = hang_backmult * abs_out_sample + onemhang_backmult * hang_backaverage;
        if(hang_backaverage > hang_level)
        {
            ts.agc_wdsp_hang_action = 1;
        }
        else
        {
            ts.agc_wdsp_hang_action = 0;
        }

        if ((abs_out_sample >= ring_max) && (abs_out_sample > 0.0))
        {
            ring_max = 0.0;
            k = out_index;
            for (j = 0; j < attack_buffsize; j++)
            {
                if (++k == ring_buffsize)
                    k = 0;
                if (abs_ring[k] > ring_max)
                    ring_max = abs_ring[k];
            }
        }
        if (abs_ring[in_index] > ring_max)
            ring_max = abs_ring[in_index];

        if (hang_counter > 0)
            --hang_counter;

        switch (state)
        {
        case 0: // starting point after ATTACK
        {
            if (ring_max >= volts)
            { // ATTACK
                volts += (ring_max - volts) * attack_mult;
            }
            else
            { // DECAY
                if (volts > pop_ratio * fast_backaverage)
                { // short time constant detector
                    state = 1;
                    volts += (ring_max - volts) * fast_decay_mult;
                }
                else
                { // hang AGC enabled and being activated
                    if (ts.agc_wdsp_hang_enable  && (hang_backaverage > hang_level))
                    {
                        state = 2;
                        hang_counter = (int)(hangtime * IQ_SAMPLE_RATE_F / ads.decimation_rate);
                        decay_type = 1;
                    }
                    else
                    {// long time constant detector
                        state = 3;
                        volts += (ring_max - volts) * decay_mult;
                        decay_type = 0;
                    }
                }
            }
            break;
        }
        case 1: // short time constant decay
        {
            if (ring_max >= volts)
            { // ATTACK
                state = 0;
                volts += (ring_max - volts) * attack_mult;
            }
            else
            {
                if (volts > save_volts)
                {// short time constant detector
                    volts += (ring_max - volts) * fast_decay_mult;
                }
                else
                {
                    if (hang_counter > 0)
                    {
                        state = 2;
                    }
                    else
                    {
                        if (decay_type == 0)
                        {// long time constant detector
                            state = 3;
                            volts += (ring_max - volts) * decay_mult;
                        }
                        else
                        { // hang time constant
                            state = 4;
                            volts += (ring_max - volts) * hang_decay_mult;
                        }
                    }
                }
            }
            break;
        }
        case 2: // Hang is enabled and active, hang counter still counting
        { // ATTACK
            if (ring_max >= volts)
            {
                state = 0;
                save_volts = volts;
                volts += (ring_max - volts) * attack_mult;
            }
            else
            {
                if (hang_counter == 0)
                { // hang time constant
                    state = 4;
                    volts += (ring_max - volts) * hang_decay_mult;
                }
            }
            break;
        }
        case 3: // long time constant decay in progress
        {
            if (ring_max >= volts)
            { // ATTACK
                state = 0;
                save_volts = volts;
                volts += (ring_max - volts) * attack_mult;
            }
            else
            { // DECAY
                volts += (ring_max - volts) * decay_mult;
            }
            break;
        }
        case 4: // hang was enabled and counter has counted to zero --> hang decay
        {
            if (ring_max >= volts)
            { // ATTACK
                state = 0;
                save_volts = volts;
                volts += (ring_max - volts) * attack_mult;
            }
            else
            { // HANG DECAY
                volts += (ring_max - volts) * hang_decay_mult;
            }
            break;
        }
        }
        if (volts < min_volts)
        {
            volts = min_volts; // no AGC action is taking place
            ts.agc_wdsp_action = 0;
        }
        else
        {
            // LED indicator for AGC action
            ts.agc_wdsp_action = 1;
        }

        float32_t vo =  log10f_fast(inv_max_input * volts);
        if(vo > 0.0)
        {
            vo = 0.0;
        }
        mult = (out_target - slope_constant * vo) / volts;
        adb.a_buffer[i] = out_sample[0] * mult;

    }
    if(ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_SAM)
    {
        // eliminate DC in the audio after the AGC
        for(i = 0; i < blockSize; i++)
        {
            w = adb.a_buffer[i] + wold * 0.9999; // yes, I want a superb bass response ;-)
            adb.a_buffer[i] = w - wold;
            wold = w;
        }
    }
}



//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_agc_processor
//* Object              :
//* Object              : Processor for receiver AGC
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_RxAgcProcessor(int16_t blockSize)
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
                ads.agc_calc = fabs(adb.a_buffer[i]) * ads.agc_val;
            }
            else	 							// not AM - get the amplitude of the recovered audio
            {   // take the absolute value of the pre-AGC, post-filter audio signal
                // and multiply by the current AGC value
                ads.agc_calc = fabs(adb.a_buffer[i]) * ads.agc_val;
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

    arm_copy_f32(adb.a_buffer, &audio_delay_buffer[agc_delay_inbuf], blockSize);	// put new data into the delay buffer
    arm_copy_f32(&audio_delay_buffer[agc_delay_outbuf], adb.a_buffer, blockSize);	// take old data out of the delay buffer

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
            w = adb.a_buffer[i] + wold * 0.9999; // yes, I want a superb bass response ;-)
            adb.a_buffer[i] = w - wold;
            wold = w;
        }
    }
    // Now apply pre-calculated AGC values to delayed audio

    arm_mult_f32(adb.a_buffer, adb.agc_valbuf, adb.a_buffer, blockSize);		// do vector multiplication to apply delayed "running" AGC data

}

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
static void AudioDriver_DemodFM(int16_t blockSize)
{

    float r, s, angle, x, y, a, b;
    ulong i;
    bool tone_det_enabled;
    static float i_prev, q_prev, lpf_prev, hpf_prev_a, hpf_prev_b;		// used in FM detection and low/high pass processing
    static float gr[3] = {0, 0, 0 };
    static float gs[3] = {0, 0, 0 };
    static float gq[3] = {0, 0, 0 }; // Goertzel values

    static float subdet = 0;				// used for tone detection
    static uchar	count = 0, tdet = 0;	// used for squelch processing and debouncing tone detection, respectively
    static ulong	gcount = 0;				// used for averaging in tone detection

    if(!ts.iq_freq_mode)	// bail out if translate mode is not active
        return;


    if(ts.fm_subaudible_tone_det_select)		// set a quick flag for checking to see if tone detection is enabled
        tone_det_enabled = 1;					// the tone decode value was nonzero - decoding is enabled
    else
        tone_det_enabled = 0;					// decoding is not enabled


    for(i = 0; i < blockSize; i++)
    {
        //
        // first, calculate "x" and "y" for the arctan2, comparing the vectors of present data with previous data
        //
        y = (i_prev * adb.q_buffer[i]) - (adb.i_buffer[i] * q_prev);
        x = (i_prev * adb.i_buffer[i]) + (adb.q_buffer[i] * q_prev);
        /*        //
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
        angle = atan2f(y,x);
        //
        // we now have our audio in "angle"
        //
        adb.d_buffer[i] = angle;		// save audio in "d" buffer for squelch noise filtering/detection - done later
        //
        // Now do integrating low-pass filter to do FM de-emphasis
        //
        a = lpf_prev + (FM_RX_LPF_ALPHA * (angle - lpf_prev));	//
        lpf_prev = a;			// save "[n-1]" sample for next iteration
        //
        adb.c_buffer[i] = a;	// save in "c" for subaudible tone detection
        //
        if(((!ads.fm_squelched) && (!tone_det_enabled)) || ((ads.fm_subaudible_tone_detected) && (tone_det_enabled)) || ((!ts.fm_sql_threshold)))	 	// high-pass audio only if we are un-squelched (to save processor time)
        {
            //
            // Do differentiating high-pass filter to attenuate very low frequency audio components, namely subadible tones and other "speaker-rattling" components - and to remove any DC that might be present.
            //
            b = FM_RX_HPF_ALPHA * (hpf_prev_b + a - hpf_prev_a);	// do differentiation
            hpf_prev_a = a;		// save "[n-1]" samples for next iteration
            hpf_prev_b = b;
            //
            adb.a_buffer[i] = b;	// save demodulated and filtered audio in main audio processing buffer
        }
        else if((ads.fm_squelched) || ((!ads.fm_subaudible_tone_detected) && (tone_det_enabled)))	 		// were we squelched or tone NOT detected?
        {
            adb.a_buffer[i] = 0;	// do not filter receive audio - fill buffer with zeroes to mute it
        }
        //
        q_prev = adb.q_buffer[i];		// save "previous" value of each channel to allow detection of the change of angle in next go-around
        i_prev = adb.i_buffer[i];
    }
    if(!ts.agc_wdsp)
    {
        //
        ads.am_fm_agc = sqrtf((q_prev * q_prev) + (i_prev * i_prev)) * FM_AGC_SCALING;		// calculate amplitude of carrier to use for AGC indication only (we need it for nothing else!)
        //
        // Do "AGC" on FM signal:  Calculate/smooth signal level ONLY - no need for audio scaling
        //
        ads.agc_calc = ads.am_fm_agc * ads.agc_val;
        //
        if(ads.agc_calc < ads.agc_knee)	 	// is audio below AGC "knee" value?
        {
            ads.agc_var = ads.agc_knee - ads.agc_calc;	// calculate difference between agc value and "knee" value
            ads.agc_var /= ads.agc_knee;	// calculate ratio of difference between knee value and this value
            ads.agc_val += ads.agc_val* AGC_DECAY_FM * ads.agc_var;	// Yes - Increase gain for AGC DECAY (always fast in FM)
        }
        else
        {
            ads.agc_var = ads.agc_calc - ads.agc_knee;	// calculate difference between agc value and "knee" value
            ads.agc_var /= ads.agc_knee;	// calculate ratio of difference between knee value and this value
            ads.agc_val -= ads.agc_val * AGC_ATTACK_FM * ads.agc_var;	// Fast attack to increase attenuation (do NOT scale w/decimation or else oscillation results)
            if(ads.agc_val <= AGC_VAL_MIN)	// Prevent zero or "negative" gain values
            {
                ads.agc_val = AGC_VAL_MIN;
            }
        }
        if(ads.agc_val >= ads.agc_rf_gain)	 	// limit AGC to reasonable values when low/no signals present
        {
            ads.agc_val = ads.agc_rf_gain;
            if(ads.agc_val >= ads.agc_val_max)	// limit maximum gain under no-signal conditions
            {
                ads.agc_val = ads.agc_val_max;
            }
        }
    }
    //
    // *** Squelch Processing ***
    //
    arm_iir_lattice_f32(&IIR_Squelch_HPF, adb.d_buffer, adb.d_buffer, blockSize);		// Do IIR high-pass filter on audio so we may detect squelch noise energy
    //
    ads.fm_sql_avg = ((1 - FM_RX_SQL_SMOOTHING) * ads.fm_sql_avg) + (FM_RX_SQL_SMOOTHING * sqrtf(fabs(adb.d_buffer[0])));	// IIR filter squelch energy magnitude:  We need look at only one representative sample

    //
    // Squelch processing
    //
    // Determine if the (averaged) energy in "ads.fm_sql_avg" is above or below the squelch threshold
    //
    if(count == 0)	 		// do the squelch threshold calculation much less often than we are called to process this audio
    {
        if(ads.fm_sql_avg > 0.175)		// limit maximum noise value in averaging to keep it from going out into the weeds under no-signal conditions (higher = noisier)
        {
            ads.fm_sql_avg = 0.175;
        }

        b = ads.fm_sql_avg * 172;		// scale noise amplitude to range of squelch setting

        if(b > 24)						// limit noise amplitude range
        {
            b = 24;
        }
        //
        b = 22-b;						// "invert" the noise power so that high number now corresponds with quieter signal:  "b" may now be compared with squelch setting
        //
        // Now evaluate noise power with respect to squelch setting
        //
        if(!ts.fm_sql_threshold)	 	// is squelch set to zero?
        {
            ads.fm_squelched = false;		// yes, the we are un-squelched
        }
        else if(ads.fm_squelched)	 	// are we squelched?
        {
            if(b >= (float)(ts.fm_sql_threshold + FM_SQUELCH_HYSTERESIS))		// yes - is average above threshold plus hysteresis?
            {
                ads.fm_squelched = false;		//  yes, open the squelch
            }
        }
        else	 	// is the squelch open (e.g. passing audio)?
        {
            if(ts.fm_sql_threshold > FM_SQUELCH_HYSTERESIS)	 				// is setting higher than hysteresis?
            {
                if(b < (float)(ts.fm_sql_threshold - FM_SQUELCH_HYSTERESIS))		// yes - is average below threshold minus hysteresis?
                {
                    ads.fm_squelched = true;		// yes, close the squelch
                }
            }
            else	 				// setting is lower than hysteresis so we can't use it!
            {
                if(b < (float)ts.fm_sql_threshold)		// yes - is average below threshold?
                {
                    ads.fm_squelched = true;		// yes, close the squelch
                }
            }
        }
        //
        count++;		// bump count that controls how often the squelch threshold is checked
        count &= FM_SQUELCH_PROC_DECIMATION;	// enforce the count limit
    }

    //
    // *** Subaudible tone detection ***
    //
    if(tone_det_enabled)	 		// is subaudible tone detection enabled?  If so, do decoding
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
        gcount++;		// this counter is used for the accumulation of data over multiple cycles
        //
        for(i = 0; i < blockSize; i++)
        {

            // Detect above target frequency
            gr[0] = ads.fm_goertzel[FM_HIGH].r * gr[1] - gr[2] + adb.c_buffer[i];		// perform Goertzel function on audio in "c" buffer
            gr[2] = gr[1];
            gr[1] = gr[0];

            // Detect energy below target frequency
            gs[0] = ads.fm_goertzel[FM_LOW].r * gs[1] - gs[2] + adb.c_buffer[i];		// perform Goertzel function on audio in "c" buffer
            gs[2] = gs[1];
            gs[1] = gs[0];

            // Detect on-frequency energy
            gq[0] = ads.fm_goertzel[FM_CTR].r * gq[1] - gq[2] + adb.c_buffer[i];
            gq[2] = gq[1];
            gq[1] = gq[0];
        }

        if(gcount >= FM_SUBAUDIBLE_GOERTZEL_WINDOW)	 		// have we accumulated enough samples to do the final energy calculation?
        {
            a = (gr[1]-(gr[2] * ads.fm_goertzel[FM_HIGH].cos));								// yes - calculate energy at frequency above center and reset detection
            b = (gr[2] * ads.fm_goertzel[FM_HIGH].sin);
            r = sqrtf(a*a + b*b);
            s = r;
            gr[0] = 0;
            gr[1] = 0;
            gr[2] = 0;

            a = (gs[1]-(gs[2] * ads.fm_goertzel[FM_LOW].cos));								// yes - calculate energy at frequency below center and reset detection
            b = (gs[2] * ads.fm_goertzel[FM_LOW].sin);
            r = sqrtf(a*a + b*b);
            s += r;					// sum +/- energy levels:  s = "off frequency" energy reading
            gs[0] = 0;
            gs[1] = 0;
            gs[2] = 0;

            a = (gq[1]-(gq[2] * ads.fm_goertzel[FM_CTR].cos));								// yes - calculate on-frequency energy and reset detection
            b = (gq[2] * ads.fm_goertzel[FM_CTR].sin);
            r = sqrtf(a*a + b*b);							// r contains "on-frequency" energy
            subdet = ((1 - FM_TONE_DETECT_ALPHA) *subdet) + (r/(s/2) * FM_TONE_DETECT_ALPHA);	// do IIR filtering of the ratio between on and off-frequency energy
            gq[0] = 0;
            gq[1] = 0;
            gq[2] = 0;

            if(subdet > FM_SUBAUDIBLE_TONE_DET_THRESHOLD)	 	// is subaudible tone detector ratio above threshold?
            {
                tdet++;		// yes - increment count			// yes - bump debounce count
                if(tdet > FM_SUBAUDIBLE_DEBOUNCE_MAX)			// is count above the maximum?
                {
                    tdet = FM_SUBAUDIBLE_DEBOUNCE_MAX;			// yes - limit the count
                }
            }
            else	 			// it is below the threshold - reduce the debounce
            {
                if(tdet)		// - but only if already nonzero!
                {
                    tdet--;
                }
            }
            if(tdet >= FM_SUBAUDIBLE_TONE_DEBOUNCE_THRESHOLD)	// are we above the debounce threshold?
            {
                ads.fm_subaudible_tone_detected = 1;			// yes - a tone has been detected
            }
            else												// not above threshold
            {
                ads.fm_subaudible_tone_detected = 0;			// no tone detected
            }

            gcount = 0;		// reset accumulation counter
        }
    }
    else	 		// subaudible tone detection disabled
    {
        ads.fm_subaudible_tone_detected = 1;	// always signal that a tone is being detected if detection is disabled to enable audio gate
    }
}


//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_demod_am  (rewritten to use optimized ARM complex magnitude function, October 2015 - KA7OEI)
//* Object              : AM demodulator
//* Object              :
//* Input Parameters    : size - size of buffer on which to operate
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*
static void AudioDriver_DemodAM(int16_t blockSize)
{
    ulong i, j;
        j = 0;
        static float32_t last_dc_level;
        for(i = 0; i < blockSize; i++)	 					// interleave I and Q data, putting result in "b" buffer
        {
            adb.a_buffer[j] = adb.i_buffer[i];
            j++;
            adb.a_buffer[j] = adb.q_buffer[i];
            j++;
        }
        //
        // perform complex vector magnitude calculation on interleaved data in "b" to recover
        // instantaneous carrier power:  sqrtf(b[n]^2+b[n+1]^2) - put result in "a"
        //
        arm_cmplx_mag_f32(adb.a_buffer, adb.b_buffer, blockSize);	// use optimized (fast) ARM function
        last_dc_level = fastdcblock_ff(adb.b_buffer, adb.a_buffer, blockSize, last_dc_level);

    //
    // Now produce signal/carrier level for AGC
    //
//    arm_mean_f32(adb.a_buffer, blockSize, (float32_t *)&ads.am_fm_agc);	// get "average" value of "a" buffer - the recovered DC (carrier) value - for the AGC (always positive since value was squared!)
//    ads.am_fm_agc *= AM_SCALING;	// rescale AM AGC to match SSB scaling so that AGC comes out the same

}

static void AudioDriver_DemodAmExperimental(int16_t blockSize)
{
    ulong i;
    float32_t sqrt, w;
    static float32_t wold;
        //
        // uses optimized ARM sqrt function, but not the arm_cmplx_mag, because the latter needs the data in interleaved format!
        // this could possibly make this even faster than first interleaving and then calculating magnitude
    	// (because arm_cmplx_mag uses the same sqrt function )

    for(i = 0; i < blockSize; i++) {
       arm_sqrt_f32 (adb.i_buffer[i] * adb.i_buffer[i] + adb.q_buffer[i] * adb.q_buffer[i], &sqrt);
//       sqrt = sqrtf(adb.i_buffer[i] * adb.i_buffer[i] + adb.q_buffer[i] * adb.q_buffer[i]);
//    	 adb.a_buffer[i] = sqrt;
         // DC removal filter -----------------------
         w = sqrt + wold * 0.9999; // yes, I want a superb bass response ;-)
         adb.a_buffer[i] = w - wold;
         wold = w;
    }

    //
    // Now produce signal/carrier level for AGC
    //
//    arm_mean_f32(adb.a_buffer, blockSize, (float32_t *)&ads.am_fm_agc);	// get "average" value of "a" buffer - the recovered DC (carrier) value - for the AGC (always positive since value was squared!)
    //
//    ads.am_fm_agc *= AM_SCALING;	// rescale AM AGC to match SSB scaling so that AGC comes out the same

}
 */
////
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
static void AudioDriver_NotchFilter(int16_t blockSize)
{
    static ulong		lms2_inbuf = 0;
    static ulong		lms2_outbuf = 0;

    // DSP Automatic Notch Filter using LMS (Least Mean Squared) algorithm
    //
    arm_copy_f32(adb.a_buffer, &lmsData.lms2_nr_delay[lms2_inbuf], blockSize);	// put new data into the delay buffer
    //
    arm_lms_norm_f32(&lmsData.lms2Norm_instance, adb.a_buffer, &lmsData.lms2_nr_delay[lms2_outbuf], lmsData.errsig2, adb.a_buffer, blockSize);	// do automatic notch
    // Desired (notched) audio comes from the "error" term - "errsig2" is used to hold the discarded ("non-error") audio data
    //
    lms2_inbuf += blockSize;				// update circular de-correlation delay buffer
    lms2_outbuf = lms2_inbuf + blockSize;
    lms2_inbuf %= ts.dsp_notch_delaybuf_len;
    lms2_outbuf %= ts.dsp_notch_delaybuf_len;
    //
}
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
static void AudioDriver_NoiseReduction(int16_t blockSize)
{
    static ulong		lms1_inbuf = 0, lms1_outbuf = 0;

    arm_copy_f32(adb.a_buffer, &lmsData.lms1_nr_delay[lms1_inbuf], blockSize);	// put new data into the delay buffer
    //
    arm_lms_norm_f32(&lmsData.lms1Norm_instance, adb.a_buffer, &lmsData.lms1_nr_delay[lms1_outbuf], adb.a_buffer, lmsData.errsig1 ,blockSize);	// do noise reduction
    //
    // Detect if the DSP output has gone to (near) zero output - a sign of it crashing!
    //
    if((((ulong)fabs(adb.a_buffer[0])) * DSP_ZERO_DET_MULT_FACTOR) < DSP_OUTPUT_MINVAL)	 	// is DSP level too low?
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
    ads.dsp_nr_sample = adb.a_buffer[0];		// provide a sample of the DSP output for crash detection
    //
    lms1_inbuf += blockSize;	// bump input to the next location in our de-correlation buffer
    lms1_outbuf = lms1_inbuf + blockSize;	// advance output to same distance ahead of input
    lms1_inbuf %= ts.dsp_nr_delaybuf_len;
    lms1_outbuf %= ts.dsp_nr_delaybuf_len;
}



#ifdef USE_SNAP
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_snap_carrier [DD4WH, march 2016]
//* Object              :
//* Object              : when called, it determines the carrier frequency inside the filter bandwidth and tunes RX to that frequency
//* Input Parameters    : uses the new arm_rfft_fast_f32 for the FFT, that is 10 times (!!!) more accurate than the old arm_rfft_f32
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

static void AudioDriver_SnapCarrier (void)
{

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
    arm_rfft_fast_f32(&sc.S,sc.FFT_Samples,sc.FFT_Samples,0);	// Do FFT
    //
    // Calculate magnitude
    // as I understand this, this takes two samples and calculates ONE magnitude from this --> length is FFT_IQ_BUFF_LEN2 / 2
    arm_cmplx_mag_f32(sc.FFT_Samples, FFT_MagData,(buff_len_int/2));
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

    if(sd.state == 0)
    {
        if(sd.magnify == 0)        //
        {
            for(int i = 0; i < blockSize; i++)
            {
                // Collect I/Q samples // why are the I & Q buffers filled with I & Q, the FFT buffers are filled with Q & I?
                sd.FFT_Samples[sd.samp_ptr] = adb.q_buffer[i];    // get floating point data for FFT for spectrum scope/waterfall display
                sd.samp_ptr++;
                sd.FFT_Samples[sd.samp_ptr] = adb.i_buffer[i];
                sd.samp_ptr++;

                // On obtaining enough samples for spectrum scope/waterfall, update state machine, reset pointer and wait until we process what we have
                if(sd.samp_ptr >= FFT_IQ_BUFF_LEN-1) //*2)
                {
                    sd.samp_ptr = 0;
                    sd.state    = 1;
                }
            }
        }
    }
}
static void AudioDriver_SpectrumZoomProcessSamples(const uint16_t blockSize)
{
    if(sd.state == 0)
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

            float32_t x_buffer[IQ_BUFSZ];
            float32_t y_buffer[IQ_BUFSZ];

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

            for(int i = 0; i < blockSize/ (1<<sd.magnify); i++)
            {
                sd.FFT_Samples[sd.samp_ptr] = y_buffer[i];   // get floating point data for FFT for spectrum scope/waterfall display
                sd.samp_ptr++;
                sd.FFT_Samples[sd.samp_ptr] = x_buffer[i]; //
                sd.samp_ptr++;

                // On obtaining enough samples for spectrum scope/waterfall, update state machine, reset pointer and wait until we process what we have
                if(sd.samp_ptr >= FFT_IQ_BUFF_LEN-1) //*2)
                {
                    sd.samp_ptr = 0;
                    sd.state    = 1;
                }
            } // end for

            // loop to put samples from x and y buffer into sd.FFT_Samples

            // TODO: also insert sample collection for snap carrier here
            // and subsequently rebuild snap carrier to use a much smaller FFT (say 256 or 512)
            // in order to save many kilobytes of RAM ;-)

            // this works!
            /*      for(i = 0; i < blockSize/powf(2,sd.magnify); i++)
    {
        if(sd.state == 0)
        { //
            // take every 2nd, 4th, 8th, 16th, or 32nd sample depending on desired magnification --> decimation
            sd.FFT_Samples[sd.samp_ptr] = (float32_t)adb.y_buffer[i << sd.magnify]; // get floating point data for FFT for spectrum scope/waterfall display
            sd.samp_ptr++;
            sd.FFT_Samples[sd.samp_ptr] = (float32_t)adb.x_buffer[i << sd.magnify]; // (i << sd.magnify) is the same as (i * 2^sd.magnify)
            sd.samp_ptr++;

    // On obtaining enough samples for spectrum scope/waterfall, update state machine, reset pointer and wait until we process what we have
            if(sd.samp_ptr >= FFT_IQ_BUFF_LEN-1) // _ *2)
            {
                sd.samp_ptr = 0;
                sd.state    = 1;
            }
        }
    } // end for
             */

        }
    }
}

#if 0
static uint16_t modulus = 0;
// used to divide usb audio out sample rate, set to 0 for 48khz, do not change
#endif

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
static bool AudioDriver_RxProcessorDigital(AudioSample_t * const src, AudioSample_t * const dst, const uint16_t blockSize)
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
        arm_fill_f32(0.0,adb.b_buffer,blockSize);
    }
    // from here straight to final AUDIO OUT processing
    return retval;
}
#endif


//*----------------------------------------------------------------------------
//* Function Name       : SAM_demodulation [DD4WH, december 2016]
//* Object              : real synchronous AM demodulation with phase detector and PLL
//* Object              :
//* Input Parameters    : adb.i_buffer, adb.q_buffer
//* Output Parameters   : adb.a_buffer
//* Functions called    :
//*----------------------------------------------------------------------------
static float32_t AudioDriver_FadeLeveler(float32_t audio, float32_t corr)
{
    static float32_t dc27 = 0.0;
    static float32_t dc_insert = 0.0;

    dc27 = adb.mtauR * dc27 + adb.onem_mtauR * audio;
    dc_insert = adb.mtauI * dc_insert + adb.onem_mtauI * corr;
    audio = audio + dc_insert - dc27;

    return audio;
}


static void AudioDriver_DemodSAM(int16_t blockSize)
{
    //#define STAGES    7

    // new synchronous AM PLL & PHASE detector
    // wdsp Warren Pratt, 2016
    //*****************************

    // First of all: decimation of I and Q path
    arm_fir_decimate_f32(&DECIMATE_SAM_I, adb.i_buffer, adb.i_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
    arm_fir_decimate_f32(&DECIMATE_SAM_Q, adb.q_buffer, adb.q_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)

    switch(ts.dmod_mode)
    {
    case DEMOD_AM:
        for(int i = 0; i < blockSize / adb.DF; i++)
        {
            float32_t audio;

            arm_sqrt_f32 (adb.i_buffer[i] * adb.i_buffer[i] + adb.q_buffer[i] * adb.q_buffer[i], &audio);
            if(ads.fade_leveler)
            {
                audio = AudioDriver_FadeLeveler(audio,0);
            }
            adb.a_buffer[i] = audio;
        }
        break;

    case DEMOD_SAM:
    {

        static uint16_t  count = 0;

        static float32_t fil_out = 0.0;
        static float32_t lowpass = 0.0;
        static float32_t omega2 = 0.0;
        static float32_t phs = 0.0;

        // Wheatley 2011 cuteSDR & Warren Pratt�s WDSP, 2016
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

            float32_t audio;

            float32_t corr[2] = { ai + bq, -bi + aq };

            switch(ads.sam_sideband)
            {
            case SAM_SIDEBAND_BOTH:
            {
                audio = corr[0];
                break;
            }
            case SAM_SIDEBAND_USB:
            {
                audio = (ai_ps - bi_ps) + (aq_ps + bq_ps);
                break;
            }
            case SAM_SIDEBAND_LSB:
            {
                audio = (ai_ps + bi_ps) - (aq_ps - bq_ps);
                break;
            }
            }

            // "fade leveler", taken from Warren Pratts� WDSP / HPSDR, 2016
            // http://svn.tapr.org/repos_sdr_hpsdr/trunk/W5WC/PowerSDR_HPSDR_mRX_PS/Source/wdsp/
            if(ads.fade_leveler)
            {
                audio = AudioDriver_FadeLeveler(audio,corr[0]);
            }

            adb.a_buffer[i] = audio;

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

/*
// help functions for Spectral Noise Blanker
// aus lmath.c !!!
// WDSP library, Warren Pratt

//const int nnn = 4;

#define nnn 4
float32_t   zzz[(nnn-1) * 4];

void dR (int n, float32_t* r, float32_t* y)
{
  int i, j, k;
    float32_t alpha, beta, gamma;
  float32_t* z; // = (double *) malloc0 ((n - 1) * sizeof (double));
    y[0] = -r[1];
    alpha = -r[1];
    beta = 1.0;
    for (k = 0; k < n - 1; k++)
    {
        beta *= 1.0 - alpha * alpha;
        gamma = 0.0;
        for (i = k + 1, j = 0; i > 0; i--, j++)
            gamma += r[i] * y[j];
        alpha = - (r[k + 2] + gamma) / beta;
        for (i = 0, j = k; i <= k; i++, j--)
            z[i] = y[i] + alpha * y[j];
    memcpy (y, z, (k + 1) * sizeof (float32_t));
        y[k + 1] = alpha;
    }
}

void trI (
    int n,
    float32_t* r,
    float32_t* B
    )
{
    int i, j, ni, nj;
    float32_t gamma, t, scale, b;
  float32_t* y;// = (double *) malloc0 ((n - 1) * sizeof (double));
  float32_t* v;// = (double *) malloc0 ((n - 1) * sizeof (double));
    scale = 1.0 / r[0];
    for (i = 0; i < n; i++)
        r[i] *= scale;
    dR(n - 1, r, y);

    t = 0.0;
    for (i = 0; i < n - 1; i++)
        t += r[i + 1] * y[i];
    gamma = 1.0 / (1.0 + t);
    for (i = 0, j = n - 2; i < n - 1; i++, j--)
        v[i] = gamma * y[j];
    B[0] = gamma;
    for (i = 1, j = n - 2; i < n; i++, j--)
        B[i] = v[j];
    for (i = 1; i <= (n - 1) / 2; i++)
        for (j = i; j < n - i; j++)
            B[i * n + j] = B[(i - 1) * n + (j - 1)] + (v[n - j - 1] * v[n - i - 1] - v[i - 1] * v[j - 1]) / gamma;
    for (i = 0; i <= (n - 1)/2; i++)
        for (j = i; j < n - i; j++)
        {
            b = B[i * n + j] *= scale;
            B[j * n + i] = b;
            ni = n - i - 1;
            nj = n - j - 1;
            B[ni * n + nj] = b;
            B[nj * n + ni] = b;
        }
}


void asolve(int xsize, int asize, float32_t* x, float32_t* a)
{
    int i, j, k;
    float32_t beta, alpha, t;
  float32_t* r;// = (double *) malloc0 ((asize + 1) * sizeof (double));
  float32_t* z;// = (double *) malloc0 ((asize + 1) * sizeof (double));
    for (i = 0; i <= asize; i++)
    {
    for (j = 0; j < xsize; j++)
      r[i] += x[j] * x[j - i];
    }
    z[0] = 1.0;
    beta = r[0];
    for (k = 0; k < asize; k++)
    {
        alpha = 0.0;
        for (j = 0; j <= k; j++)
            alpha -= z[j] * r[k + 1 - j];
        alpha /= beta;
        for (i = 0; i <= (k + 1) / 2; i++)
        {
            t = z[k + 1 - i] + alpha * z[i];
            z[i] = z[i] + alpha * z[k + 1 - i];
            z[k + 1 - i] = t;
        }
        beta *= 1.0 - alpha * alpha;
    }
    for (i = 0; i < asize; i++)
  {
        a[i] = - z[i + 1];
    if (a[i] != a[i]) a[i] = 0.0;
  }
}

void median (int n, float32_t* a, float32_t* med)
{
    int S0, S1, i, j, m, k;
    float32_t x, t;
    S0 = 0;
    S1 = n - 1;
    k = n / 2;
    while (S1 > S0 + 1)
    {
        m = (S0 + S1) / 2;
        t = a[m];
        a[m] = a[S0 + 1];
        a[S0 + 1] = t;
        if (a[S0] > a[S1])
        {
            t = a[S0];
            a[S0] = a[S1];
            a[S1] = t;
        }
        if (a[S0 + 1] > a[S1])
        {
            t = a[S0 + 1];
            a[S0 + 1] = a[S1];
            a[S1] = t;
        }
        if (a[S0] > a[S0 + 1])
        {
            t = a[S0];
            a[S0] = a[S0 + 1];
            a[S0 + 1] = t;
        }
        i = S0 + 1;
        j = S1;
        x = a[S0 + 1];
    do i++; while (a[i] < x);
        do j--; while (a[j] > x);
        while (j >= i)
        {
            t = a[i];
            a[i] = a[j];
            a[j] = t;
      do i++; while (a[i] < x);
            do j--; while (a[j] > x);
        }
        a[S0 + 1] = a[j];
        a[j] = x;
        if (j >= k) S1 = j - 1;
        if (j <= k) S0 = i;
    }
    if (S1 == S0 + 1 && a[S1] < a[S0])
    {
        t = a[S0];
        a[S0] = a[S1];
        a[S1] = t;
    }
 *med = a[k];
}



void exec_SNB(uint16_t blockSize)
{
    if(0)
    {



    }
}

 */




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
        // in 17th Annual Workshop on Circuits, Nov. 2006, pp. 158�164.
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
    const int16_t blockSizeDecim = blockSize/(int16_t)ads.decimation_rate;

    // we copy volatile variables which are used multiple times to local consts to let the compiler to its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t tx_audio_source = ts.tx_audio_source;
    const uint8_t iq_freq_mode = ts.iq_freq_mode;
    const uint8_t  dsp_active = ts.dsp_active;


    static int beep_idx = 0;

    float post_agc_gain_scaling;

#ifdef alternate_NR
    static int trans_count_in=0;
    static int outbuff_count=0;
    static int NR_fill_in_pt=0;
    static FDV_IQ_Buffer* out_buffer = NULL;
//#define NR_FFT_SIZE   128
    #endif


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
        AudioDriver_NoiseBlanker(src, blockSize);     // do noise blanker function
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

#ifdef USE_SNAP
        if (sc.snap) {
            if (sc.state == 0)
            {
                for(i = 0; i < blockSize; i++)
                {
                    sc.counter += blockSize;
                    if (sc.counter >= 4864)  // wait for 4864 samples until you gather new data for the FFT
                    {

                        // collect samples for snap carrier FFT
                        sc.FFT_Samples[sc.samp_ptr] = adb.q_buffer[i];    // get floating point data for FFT for snap carrier
                        sc.samp_ptr++;
                        sc.FFT_Samples[sc.samp_ptr] = adb.i_buffer[i];
                        sc.samp_ptr++;
                        // obtain samples for snap carrier mode
                        if(sc.samp_ptr >= FFT_IQ_BUFF_LEN2-1) //*2)
                        {
                            sc.samp_ptr = 0;
                            sc.state    = 1;
                            sc.counter = 0;
                        }
                    }
                }
            }
            else if (sc.state == 1)
            {
                AudioDriver_SnapCarrier(); // tunes the mcHF to the largest signal in the filterpassband
            }
        }
#endif


        // artificial amplitude imbalance for testing of the automatic IQ imbalance correction
        //    arm_scale_f32 (adb.i_buffer, 0.6, adb.i_buffer, blockSize);


        AudioDriver_RxHandleIqCorrection(blockSize);


        // Spectrum display sample collect for magnify == 0
        AudioDriver_SpectrumNoZoomProcessSamples(blockSize);

        if(iq_freq_mode)            // is receive frequency conversion to be done?
        {
            if(iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ)           // Yes - "RX LO LOW" mode
            {
                AudioDriver_FreqConversion(blockSize, 1);
            }
            else                                // it is in "RX LO LOW" mode
            {
                AudioDriver_FreqConversion(blockSize, 0);
            }
        }

        // Spectrum display sample collect for magnify != 0
        AudioDriver_SpectrumZoomProcessSamples(blockSize);

        //  Demodulation, optimized using fast ARM math functions as much as possible

        bool dvmode_signal = false;

#ifdef USE_FREEDV
        if (ts.dvmode == true)
        {
            dvmode_signal = AudioDriver_RxProcessorDigital(src,dst,blockSize);
        }
#endif

        if (dvmode_signal == false)
        {
            // ------------------------
            // In SSB and CW - Do 0-90 degree Phase-added Hilbert Transform
            // In AM and SAM, the FIR below does ONLY low-pass filtering appropriate for the filter bandwidth selected, in
            // which case there is ***NO*** audio phase shift applied to the I/Q channels.
            //
            //
            if(dmod_mode != DEMOD_SAM && dmod_mode != DEMOD_AM) // || ads.sam_sideband == 0) // for SAM & one sideband, leave out this processor-intense filter
            {   // FilterPathInfo[ts.filter_path].FIR_I_coeff_file == &i_rx_new_coeffs

                //                if(ts.filter_path < 48 && dmod_mode != DEMOD_FM)
                if(FilterPathInfo[ts.filter_path].FIR_I_coeff_file == i_rx_new_coeffs && dmod_mode != DEMOD_FM)
                {
                // TODO HILBERT
                //    FilterPathInfo[ts.filter_path].id >= 12
                // decimation of both channels here for LSB/USB/CW, if Filter BW <= 3k6
                arm_fir_decimate_f32(&DECIMATE_RX, adb.i_buffer, adb.i_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
                arm_fir_decimate_f32(&DECIMATE_RX_Q, adb.q_buffer, adb.q_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
                arm_fir_f32(&FIR_I,adb.i_buffer, adb.i_buffer,blockSizeDecim);   // in AM: lowpass filter, in other modes: Hilbert lowpass 0 degrees
                arm_fir_f32(&FIR_Q,adb.q_buffer, adb.q_buffer,blockSizeDecim);   // in AM: lowpass filter, in other modes: Hilbert lowpass -90 degrees
                }
                else
                {
                arm_fir_f32(&FIR_I,adb.i_buffer, adb.i_buffer,blockSize);   // in AM: lowpass filter, in other modes: Hilbert lowpass 0 degrees
                arm_fir_f32(&FIR_Q,adb.q_buffer, adb.q_buffer,blockSize);   // in AM: lowpass filter, in other modes: Hilbert lowpass -90 degrees
                }
            }

            switch(dmod_mode)
            {
            case DEMOD_LSB:
//                if(ts.filter_path < 48)
                if(FilterPathInfo[ts.filter_path].FIR_I_coeff_file == i_rx_new_coeffs)
                {
                    arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSizeDecim);   // difference of I and Q - LSB
                }
                else
                {
                    arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // difference of I and Q - LSB
                }
                break;
            case DEMOD_CW:
                if(!ts.cw_lsb)  // is this USB RX mode?  (LSB of mode byte was zero)
                {
                    arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSizeDecim);   // sum of I and Q - USB
                }
                else    // No, it is LSB RX mode
                {
                    arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSizeDecim);   // difference of I and Q - LSB
                }
                break;
                /*        case DEMOD_AM:
            if (ts.AM_experiment)
            {
                AudioDriver_DemodAmExperimental(blockSize);
            }
            else
            {
                AudioDriver_DemodAM(blockSize);
            }
            break; */
            case DEMOD_AM:
            case DEMOD_SAM:
                AudioDriver_DemodSAM(blockSize); // lowpass filtering, decimation, and SAM demodulation
                // TODO: the above is "real" SAM, old SAM mode (below) could be renamed and implemented as DSB (double sideband mode)
                // if anybody needs that

                //            arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.f_buffer, blockSize);   // difference of I and Q - LSB
                //            arm_add_f32(adb.i_buffer, adb.q_buffer, adb.e_buffer, blockSize);   // sum of I and Q - USB
                //            arm_add_f32(adb.e_buffer, adb.f_buffer, adb.a_buffer, blockSize);   // sum of LSB & USB = DSB

                break;
            case DEMOD_FM:
                AudioDriver_DemodFM(blockSize);
                break;
            case DEMOD_DIGI:
                // if we are here, the digital codec (e.g. because of no signal) asked to decode
                // using analog demodulation in the respective sideband
                if (ts.digi_lsb)
                {
                    arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // difference of I and Q - LSB
                }
                else
                {
                    arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // sum of I and Q - USB
                }
                break;
            case DEMOD_USB:
            default:
//                if(ts.filter_path < 48)
                if(FilterPathInfo[ts.filter_path].FIR_I_coeff_file == i_rx_new_coeffs)
                {
                    arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSizeDecim);   // sum of I and Q - USB
                }
                else
                {
                    arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // sum of I and Q - USB
                }
                break;
            }

            if(dmod_mode != DEMOD_FM)       // are we NOT in FM mode?  If we are not, do decimation, filtering, DSP notch/noise reduction, etc.
            {
                // Do decimation down to lower rate to reduce processor load
                if (DECIMATE_RX.numTaps > 0 && dmod_mode != DEMOD_SAM && dmod_mode != DEMOD_AM &&
                        !((dmod_mode == DEMOD_LSB || dmod_mode == DEMOD_USB || dmod_mode == DEMOD_CW) && FilterPathInfo[ts.filter_path].FIR_I_coeff_file == i_rx_new_coeffs)) // in SAM mode, the decimation is done in both I & Q path --> AudioDriver_Demod_SAM
                {
                    // TODO HILBERT

                    // for filter BW <= 3k6 and LSB/USB/CW, don't do decimation, we are already in 12ksps

                    arm_fir_decimate_f32(&DECIMATE_RX, adb.a_buffer, adb.a_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
                }


                if (ts.dsp_inhibit == false)
                {
                    if((dsp_active & DSP_NOTCH_ENABLE) && (dmod_mode != DEMOD_CW) && !(ts.dmod_mode == DEMOD_SAM && (FilterPathInfo[ts.filter_path].sample_rate_dec) == RX_DECIMATION_RATE_24KHZ))       // No notch in CW
                    {
                        AudioDriver_NotchFilter(blockSizeDecim);     // Do notch filter
                    }

                    // DSP noise reduction using LMS (Least Mean Squared) algorithm
                    // This is the pre-filter/AGC instance


                        if((dsp_active & DSP_NR_ENABLE) && (!(dsp_active & DSP_NR_POSTAGC_ENABLE)) && !(ts.dmod_mode == DEMOD_SAM && (FilterPathInfo[ts.filter_path].sample_rate_dec) == RX_DECIMATION_RATE_24KHZ))      // Do this if enabled and "Pre-AGC" DSP NR enabled
                            {
                              AudioDriver_NoiseReduction(blockSizeDecim);
                            }

                  }

                // Apply audio  bandpass filter
                if ((IIR_PreFilter.numStages > 0))   // yes, we want an audio IIR filter
                {
                    arm_iir_lattice_f32(&IIR_PreFilter, adb.a_buffer, adb.a_buffer, blockSizeDecim);
                }


                //
                // This is the right place for the SNB
                // Spectral Noise Blanker
                //
                // we try to implement the SNB from the WDSP lib
                // by Warren Pratt
                //
                // I have no idea whether it will be possible to implement it, because of processor load and very complex implementation issues
                // DD4WH Feb, 22nd, 2017

                //            exec_SNB(blockSizeDecim);

                // now process the samples and perform the receiver AGC function
                if(ts.agc_wdsp)
                {
                    AudioDriver_RxAGCWDSP(blockSizeDecim);
                }
                else
                {
                    AudioDriver_RxAgcProcessor(blockSizeDecim);
                }

                // DSP noise reduction using LMS (Least Mean Squared) algorithm
                // This is the post-filter, post-AGC instance
                //
                if((dsp_active & DSP_NR_ENABLE) && (dsp_active & DSP_NR_POSTAGC_ENABLE) && (!ts.dsp_inhibit) && !(ts.dmod_mode == DEMOD_SAM && (FilterPathInfo[ts.filter_path].sample_rate_dec) == RX_DECIMATION_RATE_24KHZ))     // Do DSP NR if enabled and if post-DSP NR enabled
                {
                    AudioDriver_NoiseReduction(blockSizeDecim);
                }
                //

                if (ts.new_nb==true) //start of new nb
                {
                           // NR_in and _out buffers are using the same physical space than the freedv_iq_buffer
                           // the freedv_iq_buffer  consist of an array of 320 complex (2*float) samples
                           // for NR reduction we use a maximum of 256 real samples
                           // so we use the freedv_iq buffers in a way, that we use the first half of each array for the input
                           // and the second half for the output
                           // .real and .imag are loosing there meaning here as they represent consecutive real samples

                           if (ads.decimation_rate == 4)   //  to make sure, that we are at 12Ksamples

                           {
                             for (int k = 0; k < blockSizeDecim; k=k+2) //transfer our noisy audio to our NR-input buffer
                             {
                                 fdv_iq_buff[NR_fill_in_pt].samples[trans_count_in].real=adb.a_buffer[k];
                                 fdv_iq_buff[NR_fill_in_pt].samples[trans_count_in].imag=adb.a_buffer[k+1];
                                 //trans_count_in++;
                                 trans_count_in++; // count the samples towards FFT-size  -  2 samples per loop
                             }

                             if (trans_count_in >= (NR_FFT_SIZE/2))  // buffer limited to 320!! as in FreeDV used
                                                                 //NR_FFT_SIZE has to be an integer mult. of blockSizeDecim!!!
                             {
                                 NR_in_buffer_add(&fdv_iq_buff[NR_fill_in_pt]); // save pointer to full buffer
                                 trans_count_in=0;                              // set counter to 0
                                 NR_fill_in_pt++;                               // increase pointer index
                                 NR_fill_in_pt %= FDV_BUFFER_IQ_NUM;            // make sure, that index stays in range

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

                             if (out_buffer != NULL)  //NR-routine has finished it's job
                             {
                                 for (int j=0; j < blockSizeDecim; j=j+2) // transfer noise reduced data back to our buffer
                                 {
                                     adb.a_buffer[j]   = out_buffer->samples[outbuff_count+NR_FFT_SIZE].real; //here add the offset in the buffer
                                     adb.a_buffer[j+1] = out_buffer->samples[outbuff_count+NR_FFT_SIZE].imag; //here add the offset in the buffer
                                     outbuff_count++;
                                     //outbuff_count++;
                                 }

                                 if (outbuff_count >= (NR_FFT_SIZE/2)) // we reached the end of the buffer comming from NR
                                 {
                                     outbuff_count = 0;
                                     NR_out_buffer_remove(&out_buffer);
                                     out_buffer = NULL;
                                     NR_out_buffer_peek(&out_buffer);
                                 }
                             }

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
                    if(ts.agc_wdsp)
                    {
                        scale_gain = post_agc_gain_scaling * 0.5; // ignore ts.max_rf_gain  --> has no meaning with WDSP AGC; and take into account AM scaling factor
                    }
                    else
                    {
                        scale_gain = ads.post_agc_gain * post_agc_gain_scaling * (AM_SCALING * AM_AUDIO_SCALING);
                    }
                }
                else        // Not AM
                {
                    if(ts.agc_wdsp)
                    {
                        scale_gain = post_agc_gain_scaling * 0.333; // ignore ts.max_rf_gain --> has no meaning with WDSP AGC
                    }
                    else
                    {
                        scale_gain = ads.post_agc_gain * post_agc_gain_scaling;
                    }
                }
                arm_scale_f32(adb.a_buffer,scale_gain, adb.a_buffer, blockSizeDecim); // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC

                // this is the biquad filter, a notch, peak, and lowshelf filter
                arm_biquad_cascade_df1_f32 (&IIR_biquad_1, adb.a_buffer,adb.a_buffer, blockSizeDecim);

                // resample back to original sample rate while doing low-pass filtering to minimize audible aliasing effects
                if (INTERPOLATE_RX.phaseLength > 0)
                {
                    arm_fir_interpolate_f32(&INTERPOLATE_RX, adb.a_buffer,(float32_t *) adb.b_buffer, blockSizeDecim);
                }
                // additional antialias filter for specific bandwidths
                // IIR ARMA-type lattice filter
                if (IIR_AntiAlias.numStages > 0)   // yes, we want an interpolation IIR filter
                {
                    arm_iir_lattice_f32(&IIR_AntiAlias, adb.b_buffer, adb.b_buffer, blockSize);
                }

            } // end NOT in FM mode
            else if(ts.dmod_mode == DEMOD_FM)           // it is FM - we don't do any decimation, interpolation, filtering or any other processing - just rescale audio amplitude
            {
                arm_scale_f32(
                        adb.a_buffer,
                        RadioManagement_FmDevIs5khz() ? FM_RX_SCALING_5K : FM_RX_SCALING_2K5,
                                adb.b_buffer,
                                blockSizeDecim);  // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
                if(ts.agc_wdsp)
                {
                    AudioDriver_RxAGCWDSP(blockSizeDecim);
                }

            }

            // this is the biquad filter, a highshelf filter
            arm_biquad_cascade_df1_f32 (&IIR_biquad_2, adb.b_buffer,adb.b_buffer, blockSize);
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
        arm_fill_f32(0, adb.a_buffer, blockSize);
        arm_fill_f32(0, adb.b_buffer, blockSize);
        if (ts.audio_dac_muting_buffer_count > 0)
        {
            ts.audio_dac_muting_buffer_count--;
        }
    }
    else
    {
        arm_scale_f32(adb.b_buffer, (float32_t)LINE_OUT_SCALING_FACTOR, adb.a_buffer, blockSize);       // Do fixed scaling of audio for LINE OUT and copy to "a" buffer in one operation
        //
        // AF gain in "ts.audio_gain-active"
        //  0 - 16: via codec command
        // 17 - 20: soft gain after decoder
        //
        if(ts.rx_gain[RX_AUDIO_SPKR].value > 16)    // is volume control above highest hardware setting?
        {
            arm_scale_f32(adb.b_buffer, (float32_t)ts.rx_gain[RX_AUDIO_SPKR].active_value, adb.b_buffer, blockSize);    // yes, do software volume control adjust on "b" buffer
        }
    }



    float32_t usb_audio_gain = ts.rx_gain[RX_AUDIO_DIG].value/31.0;

    // Transfer processed audio to DMA buffer
    for(int i=0; i < blockSize; i++)                            // transfer to DMA buffer and do conversion to INT
    {
        // TODO: move to softdds ...
        if((ts.beep_active) && (ads.beep.step))         // is beep active?
        {
            // Yes - Calculate next sample

            beep_idx    =  softdds_step(&ads.beep); // shift accumulator to index sine table
            adb.b_buffer[i] += (float32_t)(DDS_TABLE[beep_idx] * ads.beep_loudness_factor); // load indexed sine wave value, adding it to audio, scaling the amplitude and putting it on "b" - speaker (ONLY)
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
            dst[i].l = adb.b_buffer[i];        // Speaker channel (variable level)
            dst[i].r = adb.a_buffer[i];        // LINE OUT (constant level)
        }
        // Unless this is DIGITAL I/Q Mode, we sent processed audio
        if (tx_audio_source != TX_AUDIO_DIGIQ)
        {
            float32_t val = adb.a_buffer[i] * usb_audio_gain;
            audio_in_put_buffer(val);
            audio_in_put_buffer(val);
        }
    }
    // calculate the first index we read so that we are not loosing
    // values.
    // For 1 and 2,4 we do not need to shift modulus
    // since (SIZE/2) % USBD_AUDIO_IN_OUT_DIV == 0
    // if someone needs lower rates, just add formula or values
    // but this would bring us down to less than 12khz bitrate
#if 0
    if (USBD_AUDIO_IN_OUT_DIV == 3)
    {
        modulus++;
        modulus%=USBD_AUDIO_IN_OUT_DIV;
    }
#endif
}


//
//*----------------------------------------------------------------------------
//* Function Name       : audio_tx_compressor (look-ahead type) by KA7OEI
//* Object              :
//* Object              : speech compressor/processor for TX audio
//* Input Parameters    : size of buffer to processes, gain scaling factor
//* Input Parameters    : also processes/compresses audio in "adb.i_buffer" and "adb.q_buffer" - it looks only at data in "i" buffer
//*                       reads adb.a_buffer as well.
//* Output Parameters   : data via "adb.i_buffer" and "adb.q_buffer"
//* Functions called    : none
//*----------------------------------------------------------------------------
static void AudioDriver_TxCompressor(float32_t* buffer, int16_t blockSize, float gain_scaling)
{
    static uint32_t		alc_delay_inbuf = 0, alc_delay_outbuf;

    if (ts.tx_comp_level > -1)
    {
        if(!ts.tune)        // do post-filter gain calculations if we are NOT in TUNE mode
        {
            // perform post-filter gain operation
            // this is part of the compression
            //
            float32_t gain_calc = ((float32_t)ts.alc_tx_postfilt_gain_var)/2.0 +0.5 ;
            // get post-filter gain setting
            // offset it so that 2 = unity

            arm_scale_f32(buffer, gain_calc, buffer, blockSize);      // use optimized function to apply scaling to I/Q buffers
        }



        // ------------------------
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

// Equalize based on band and simultaneously apply I/Q gain AND phase adjustments
static void AudioDriver_TxIqProcessingFinal(float scaling, bool swap, AudioSample_t* const dst, const uint16_t blockSize)
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

    float32_t final_i_gain = (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var[trans_idx].i * scaling);
    float32_t final_q_gain = (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var[trans_idx].q * scaling);
    // ------------------------
    // Output I and Q as stereo data
    if(swap == false)	 			// if is it "RX LO LOW" mode, save I/Q data without swapping, putting it in "upper" sideband (above the LO)
    {
        final_i_buffer = adb.i_buffer;
        final_q_buffer = adb.q_buffer;
    }
    else	 	// it is "RX LO HIGH" - swap I/Q data while saving, putting it in the "lower" sideband (below the LO)
    {
        // this is the IQ gain / amplitude adjustment
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
        softdds_runf(adb.a_buffer, adb.a_buffer,blockSize);     // load audio buffer with the tone - DDS produces quadrature channels, but we need only one
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
                adb.a_buffer[i] = src[i].r;
            }
        } else {
            // audio buffer with left sample channel
            for(int i = 0; i < blockSize; i++)
            {
                adb.a_buffer[i] = src[i].l;
            }
        }

        arm_scale_f32(adb.a_buffer, gain_calc, adb.a_buffer, blockSize);  // apply gain

        ads.peak_audio = AudioDriver_absmax(adb.a_buffer, blockSize);
    }
}

/***
 * takes the I Q input buffers containing the I Q audio for a single AM sideband and returns the frequency translated IQ sideband
 * in the i/q buffers.
 * Used temporary buffers:
 *  directly: i,q
 *  indirectly: c,d,e,f -> see audio_rx_freq_conv
 */
static void AudioDriver_TxProcessorAMSideband(float32_t* I_buffer, float32_t* Q_buffer,  const int16_t blockSize) {

    // generate AM carrier by applying a "DC bias" to the audio
    //
    arm_offset_f32(I_buffer, AM_CARRIER_LEVEL, adb.i_buffer, blockSize);
    arm_offset_f32(Q_buffer, (-1 * AM_CARRIER_LEVEL), adb.q_buffer, blockSize);
    //
    // check and apply correct translate mode
    //
    AudioDriver_FreqConversion(blockSize, (ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ));
}

static inline void AudioDriver_TxFilterAudio(bool do_bandpass, bool do_bass_treble, float32_t* inBlock, float32_t* outBlock, const uint16_t blockSize)
{
    if (do_bandpass)
    {
        arm_iir_lattice_f32(&IIR_TXFilter, inBlock, outBlock, blockSize);
    }
    if (do_bass_treble)
    { //
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

    AudioDriver_TxFilterAudio(true,ts.tx_audio_source != TX_AUDIO_DIG,adb.a_buffer,adb.i_buffer, blockSize);

    AudioDriver_TxCompressor(adb.i_buffer, blockSize, FM_ALC_GAIN_CORRECTION);  // Do the TX ALC and speech compression/processing
    //
    // Do differentiating high-pass filter to provide 6dB/octave pre-emphasis - which also removes any DC component!  Takes audio from "i" and puts it into "a".
    //
    for(int i = 0; i < blockSize; i++)
    {
        //
        //
        a = adb.i_buffer[i];
        //
        b = FM_TX_HPF_ALPHA * (hpf_prev_b + a - hpf_prev_a);    // do differentiation
        hpf_prev_a = a;     // save "[n-1] samples for next iteration
        hpf_prev_b = b;
        //
        adb.a_buffer[i] = b;    // save differentiated data in audio buffer
    }
    //
    // do tone generation using the NCO (a.k.a. DDS) method.  This is used for subaudible tone generation and, if necessary, summing the result in "a".
    //
    if((ads.fm_subaudible_tone_word) && (!ads.fm_tone_burst_active))        // generate tone only if it is enabled (and not during a tone burst)
    {
        for(int i = 0; i < blockSize; i++)
        {
            fm_tone_accum += ads.fm_subaudible_tone_word;   // generate tone using frequency word, calculating next sample
            fm_tone_accum &= 0xffffff;              // limit to 16 Meg range
            fm_tone_idx    = fm_tone_accum >> FM_TONE_DDS_ACC_SHIFT;    // shift accumulator to index sine table
            fm_tone_idx &= (DDS_TBL_SIZE-1);        // limit lookup to range of sine table
            adb.a_buffer[i] += ((float32_t)(DDS_TABLE[fm_tone_idx]) * FM_TONE_AMPLITUDE_SCALING * fm_mod_mult); // load indexed sine wave value, adding it to audio
        }
    }
    //
    // do tone  generation using the NCO (a.k.a. DDS) method.  This is used for tone burst ("whistle-up") generation, summing the result in "a".
    //
    if(ads.fm_tone_burst_active)                // generate tone burst only if it is enabled
    {
        for(int i = 0; i < blockSize; i++)
        {
            // Calculate next sample
            fm_tone_burst_accum += ads.fm_tone_burst_word;  // generate tone using frequency word, calculating next sample
            fm_tone_burst_accum &= 0xffffff;                // limit to 16 Meg range
            fm_tone_burst_idx    = fm_tone_burst_accum >> FM_TONE_DDS_ACC_SHIFT;    // shift accumulator to index sine table
            fm_tone_burst_idx &= (DDS_TBL_SIZE-1);      // limit lookup to range of sine table
            adb.a_buffer[i] += ((float32_t)((DDS_TABLE[fm_tone_burst_idx]) * FM_MOD_SCALING * fm_mod_mult) / FM_TONE_BURST_MOD_SCALING);    // load indexed sine wave value, adding it to audio
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
        fm_mod_accum += (ulong)(fm_freq_mod_word + (adb.a_buffer[i] * FM_MOD_SCALING * fm_mod_mult));   // change frequency using scaled audio
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

    if (ts.digital_mode==1)
    { //we are in freedv-mode

        AudioDriver_TxAudioBufferFill(src,blockSize);

        // *****************************   DV Modulator goes here - ads.a_buffer must be at 8 ksps

        // Freedv Test DL2FW

        // we have to add a decimation filter here BEFORE we decimate
        // for decimation-by-6 the stopband frequency is 48/6*2 = 4kHz
        // but our audio is at most 3kHz wide, so we should use 3k or 2k9


        // this is the correct DECIMATION FILTER (before the downsampling takes place):
        // use it ALWAYS, also with TUNE tone!!!
        AudioDriver_TxFilterAudio(true,false, adb.a_buffer,adb.a_buffer, blockSize);


        // DOWNSAMPLING
        for (int k = 0; k < blockSize; k++)
        {
            if (k % 6 == modulus_NF)  //every 6th sample has to be catched -> downsampling by 6
            {
                fdv_audio_buff[FDV_TX_fill_in_pt].samples[trans_count_in] = ((int32_t)adb.a_buffer[k])/4;
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
            arm_fir_f32(&FIR_I_FREEDV, adb.i_buffer, adb.i_buffer,blockSize);
            arm_fir_f32(&FIR_Q_FREEDV, adb.q_buffer, adb.q_buffer, blockSize);



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


#if 0
        //
        // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
        // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
        // + 0 deg to I data

        arm_fir_f32(&FIR_I_TX,adb.a_buffer, adb.i_buffer,blockSize);
        // - 90 deg to Q data
        arm_fir_f32(&FIR_Q_TX,adb.a_buffer, adb.q_buffer, blockSize);
        // audio_tx_compressor(blockSize, SSB_ALC_GAIN_CORRECTION);  // Do the TX ALC and speech compression/processing

        if(ts.iq_freq_mode)
        {
            // is transmit frequency conversion to be done?
            // USB && (-6kHz || -12kHz) --> false, else true
            // LSB && (+6kHz || +12kHz) --> false, else true
            bool swap = ts.dmod_mode == DEMOD_LSB && (ts.iq_freq_mode == FREQ_IQ_CONV_M6KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ);
            swap = swap || ((ts.dmod_mode == DEMOD_USB) && (ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ));

            AudioDriver_FreqConversion(blockSize, swap);
        }
#endif

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


//*----------------------------------------------------------------------------
//* Function Name       : audio_tx_processor
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void AudioDriver_TxProcessor(AudioSample_t * const srcCodec, AudioSample_t * const dst, uint16_t blockSize)
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
    bool external_tx_mute = ts.audio_dac_muting_flag || ts.audio_dac_muting_buffer_count >0 ;

    bool signal_active = false; // unless this is set to true, zero output will be generated

    // If source is digital usb in, pull from USB buffer, discard line or mic audio and
    // let the normal processing happen
    if (tx_audio_source == TX_AUDIO_DIG || tx_audio_source == TX_AUDIO_DIGIQ)
    {
        // FIXME: change type of audio_out_fill_tx_buffer to use audio sample struct
        audio_out_fill_tx_buffer((int16_t*)srcUSB,2*blockSize);
    }

    if (tx_audio_source == TX_AUDIO_DIGIQ && dmod_mode != DEMOD_CW && !tune)
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
    else
        if (ts.dvmode) {
#ifdef USE_FREEDV
            AudioDriver_TxProcessorDigital(src,dst,blockSize);
            signal_active = true;
#endif
        }
        else if(dmod_mode == DEMOD_CW)
        {
            if (tune)
            {
                softdds_runf(adb.i_buffer, adb.q_buffer,blockSize);      // generate tone/modulation for TUNE
                // Equalize based on band and simultaneously apply I/Q gain & phase adjustments
                signal_active = true;
            }
            else
            {
                // Generate CW tone if necessary
                if (external_tx_mute == false)
                {
                    signal_active = CwGen_Process(adb.i_buffer, adb.q_buffer,blockSize);

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
                    AudioDriver_TxFilterAudio(true,tx_audio_source != TX_AUDIO_DIG, adb.a_buffer,adb.a_buffer, blockSize);
                }


                // Do the TX ALC and speech compression/processing
                AudioDriver_TxCompressor(adb.a_buffer, blockSize, SSB_ALC_GAIN_CORRECTION);

                // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
                // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
                // + 0 deg to I data
                arm_fir_f32(&FIR_I_TX,adb.a_buffer,adb.i_buffer,blockSize);
                // - 90 deg to Q data
                arm_fir_f32(&FIR_Q_TX,adb.a_buffer,adb.q_buffer, blockSize);


                if(iq_freq_mode)
                {
                    // is transmit frequency conversion to be done?
                    // LSB && (-6kHz || -12kHz) --> true, else false
                    // USB && (+6kHz || +12kHz) --> true, else false
                    bool swap = dmod_mode == DEMOD_LSB && (iq_freq_mode == FREQ_IQ_CONV_M6KHZ || iq_freq_mode == FREQ_IQ_CONV_M12KHZ);
                    swap = swap || ((dmod_mode == DEMOD_USB) && (iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ));

                    AudioDriver_FreqConversion(blockSize, swap);
                }

                // apply I/Q amplitude & phase adjustments
                AudioDriver_TxIqProcessingFinal(SSB_GAIN_COMP, dmod_mode == DEMOD_LSB, dst, blockSize);
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

                AudioDriver_TxFilterAudio((ts.flags1 & FLAGS1_AM_TX_FILTER_DISABLE) == false,tx_audio_source != TX_AUDIO_DIG, adb.a_buffer,adb.a_buffer , blockSize);
                //
                // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
                // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
                // Apply transformation AND audio filtering to buffer data
                //
                // + 0 deg to I data
                // AudioDriver_delay_f32((arm_fir_instance_f32 *)&FIR_I_TX,(float32_t *)(adb.a_buffer),(float32_t *)(adb.i_buffer),blockSize);

                AudioDriver_TxCompressor(adb.a_buffer, blockSize, AM_ALC_GAIN_CORRECTION);    // Do the TX ALC and speech compression/processing

                arm_fir_f32(&FIR_I_TX,adb.a_buffer,adb.i_buffer,blockSize);
                // - 90 deg to Q data
                arm_fir_f32(&FIR_Q_TX,adb.a_buffer,adb.q_buffer, blockSize);

                //
                // COMMENT:  It would be trivial to add the option of generating AM with just a single (Upper or Lower) sideband since we are generating the two, separately anyway
                // and putting them back together!  [KA7OEI]
                //
                //
                // First, generate the LOWER sideband of the AM signal
                // copy contents to temporary holding buffers for later generation of USB AM carrier
                //
                arm_negate_f32(adb.i_buffer, adb.a_buffer, blockSize); // this becomes the q buffer for the upper  sideband
                arm_negate_f32(adb.q_buffer, adb.b_buffer, blockSize); // this becomes the i buffer for the upper  sideband

                // now generate USB AM sideband signal
                AudioDriver_TxProcessorAMSideband(adb.i_buffer,adb.q_buffer,blockSize);

                arm_copy_f32(adb.i_buffer, adb.i2_buffer, blockSize);
                arm_copy_f32(adb.q_buffer, adb.q2_buffer, blockSize);
                // i2/q2 now contain the LSB AM signal

                // now generate USB AM sideband signal
                AudioDriver_TxProcessorAMSideband(adb.b_buffer,adb.a_buffer,blockSize);

                arm_add_f32(adb.i2_buffer,adb.i_buffer,adb.i_buffer,blockSize);
                arm_add_f32(adb.q2_buffer,adb.q_buffer,adb.q_buffer,blockSize);

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
            audio_in_put_buffer(adb.a_buffer[i]);
            audio_in_put_buffer(adb.a_buffer[i]);
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
void AudioDriver_I2SCallback(int16_t *src, int16_t *dst, int16_t size, uint16_t ht)
#endif
{
    static bool to_rx = false;	// used as a flag to clear the RX buffer
    static bool to_tx = false;	// used as a flag to clear the TX buffer
    static ulong tcount = 0;
    bool muted = false;

    if(ts.show_tp_coordinates)
    {
        MchfBoard_GreenLed(LED_STATE_ON);
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
            float agc_holder = ads.agc_val;
            bool dsp_inhibit_holder = ts.dsp_inhibit;
            AudioDriver_RxProcessor((AudioSample_t*) src, (AudioSample_t*)dst,size/2);
            ads.agc_val = agc_holder;
            ts.dsp_inhibit = dsp_inhibit_holder;
        }
        else
        {
            AudioDriver_RxProcessor((AudioSample_t*) src, (AudioSample_t*)dst,size/2);
        }

        to_tx = true;		// Set flag to indicate that we WERE receiving when we go back to transmit mode
    }
    else  			// Transmit mode
    {
        if((to_tx) || (ts.audio_processor_input_mute_counter>0))	 	// the first time back to RX, or TX audio muting timer still active - clear the buffers to reduce the "crash"
        {
            muted = true;
            arm_fill_q15(0, src, size);
            if (to_rx)
            {
                AudioDriver_ClearAudioDelayBuffer();
            }
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
            AudioDriver_TxProcessor((AudioSample_t*) src, (AudioSample_t*)dst,size/2);
            ads.alc_val = alc_holder;
        }
        else
        {
            AudioDriver_TxProcessor((AudioSample_t*) src, (AudioSample_t*)dst,size/2);
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

    if(ts.spectrum_scheduler)		// update thread timer if non-zero
    {
        ts.spectrum_scheduler--;
    }

    if(ts.show_tp_coordinates)
    {
        MchfBoard_GreenLed(LED_STATE_OFF);
    }
}
