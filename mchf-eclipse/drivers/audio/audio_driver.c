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

// Common
#include "mchf_board.h"
#include "profiling.h"

#include <stdio.h>
#include <math.h>
#include "codec.h"
#include "i2s.h"
#include "cw_gen.h"

#include <limits.h>
#include "softdds.h"

#include "audio_driver.h"
#include "audio_management.h"
#include "dds_table.h"
#include "ui_driver.h"
#include "usbd_audio_core.h"
#include "ui_spectrum.h"
#include "ui_rotary.h"
#include "filters.h"
#include "ui_lcd_hy28.h"
#include "ui_configuration.h"


typedef struct {
    __packed int16_t l;
    __packed int16_t r;
} AudioSample_t;

// SSB filters - now handled in ui_driver to allow I/Q phase adjustment

// ---------------------------------
// DMA buffers for I2S
__IO int16_t 	tx_buffer[BUFF_LEN+1];
__IO int16_t    rx_buffer[BUFF_LEN+1];

static inline void AudioDriver_tx_filter_audio(bool do_bandpass, bool do_bass_treble, float32_t* inBlock, float32_t* outBlock, const uint16_t blockSize);

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


float32_t	__attribute__ ((section (".ccm"))) agc_delay	[AGC_DELAY_BUFSIZE+16];

void audio_driver_ClearAGCDelayBuffer()
{
    arm_fill_f32(0, agc_delay, AGC_DELAY_BUFSIZE+16);
}


//
// Audio RX - Decimator
static	arm_fir_decimate_instance_f32	DECIMATE_RX;
float32_t			__attribute__ ((section (".ccm"))) decimState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

// Decimator for Zoom FFT
static	arm_fir_decimate_instance_f32	DECIMATE_ZOOM_FFT_I;
float32_t			__attribute__ ((section (".ccm"))) decimZoomFFTIState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

// Decimator for Zoom FFT
static	arm_fir_decimate_instance_f32	DECIMATE_ZOOM_FFT_Q;
float32_t			__attribute__ ((section (".ccm"))) decimZoomFFTQState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

// Audio RX - Interpolator
static	arm_fir_interpolate_instance_f32 INTERPOLATE_RX;
float32_t			__attribute__ ((section (".ccm"))) interpState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];

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
    	// a1 and a2 negated! order: b0, b1, b2, a1, a2
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
	    // a1 and a2 negated! order: b0, b1, b2, a1, a2
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
   	    // a1 and a2 negated! order: b0, b1, b2, a1, a2
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
	    // a1 and a2 negated! order: b0, b1, b2, a1, a2
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
  	    // a1 and a2 negated! order: b0, b1, b2, a1, a2
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
  	   -0.993055129539134551 },
};

// variables for FM squelch IIR filters
static float32_t	iir_squelch_rx_state[IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES];
static arm_iir_lattice_instance_f32	IIR_Squelch_HPF;

// variables for TX IIR filter
float32_t		iir_tx_state[IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES];
arm_iir_lattice_instance_f32	IIR_TXFilter;


// S meter public
__IO	SMeter					sm;

// Keypad driver publics
extern __IO	KeypadState				ks;
//

// ATTENTION: These data structures have been placed in CCM Memory (64k)
// IF THE SIZE OF  THE DATA STRUCTURE GROWS IT WILL QUICKLY BE OUT OF SPACE IN CCM
// Be careful! Check mchf-eclipse.map for current allocation
__IO AudioDriverState   __attribute__ ((section (".ccm")))  ads;
AudioDriverBuffer   __attribute__ ((section (".ccm")))  adb;
LMSData                 __attribute__ ((section (".ccm"))) lmsData;

#ifdef USE_SNAP
SnapCarrier   sc;
#endif

/*
 * @return offset frequency in Hz for current frequency translate mode
 */
int32_t audio_driver_xlate_freq()
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

static void Audio_Init(void);
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
void audio_driver_init(void)
{
    const uint32_t word_size = WORD_SIZE_16;

    // CW module init
    cw_gen_init();

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
    RadioManagement_CalculateCWSidebandMode();	// set up CW sideband mode setting
    //
    // The "active" NCO in the frequency translate function is NOT used, but rather a "static" sine that is an integer divisor of the sample rate.
    //
    //audio_driver_config_nco();	// Configure the NCO in the frequency translate function
    //
    ads.tx_filter_adjusting = 0;	// used to disable TX I/Q filter during adjustment
    // Audio init
    Audio_Init();
    Audio_TXFilter_Init(ts.dmod_mode);

    // Codec init
    Codec_MCUInterfaceInit(ts.samp_rate,word_size);

    // Codec settle delay
    non_os_delay();

    // I2S hardware init
    I2S_Block_Init();

    // Start DMA transfers
    I2S_Block_Process((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);

    Codec_Reset(ts.samp_rate,word_size);

#ifdef USE_SNAP
    // initialize FFT structure used for snap carrier
//	arm_rfft_init_f32((arm_rfft_instance_f32 *)&sc.S,(arm_cfft_radix4_instance_f32 *)&sc.S_CFFT,FFT_IQ_BUFF_LEN2,1,1);
    arm_rfft_fast_init_f32((arm_rfft_fast_instance_f32 *)&sc.S, FFT_IQ_BUFF_LEN2);
#endif

    // Audio filter enabled
     ads.af_disabled = 0;
     ts.dsp_inhibit = 0;

}

//*----------------------------------------------------------------------------
//* Function Name       : audio_driver_set_rx_audio_filter
//* Object              :
//* Object              : select audio filter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
// WARNING:  You CANNOT reliably use the built-in IIR and FIR "init" functions when using CONST-based coefficient tables!  If you do so, you risk filters
//	not initializing properly!  If you use the "init" functions, you MUST copy CONST-based coefficient tables to RAM first!
//  This information is from recommendations by online references for using ARM math/DSP functions
//

void audio_driver_set_rx_audio_filter(uint8_t dmod_mode)
{
    uint32_t	i;
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

    //
    // Initialize IIR filter state buffer
    for(i = 0; i < IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES; i++)	 	// initialize state buffer to zeroes
    {
        iir_rx_state[i] = 0;
    }
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

    for(i = 0; i < IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES; i++)	 	// initialize state buffer to zeroes
    {
        iir_aa_state[i] = 0;
    }
    IIR_AntiAlias.pState = iir_aa_state;					// point to state array for IIR filter

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
    // y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]
    //
    // However, the cookbook formulae by Robert Bristow-Johnson AND the Iowa Hills IIR Filter designer
    // use this formula:
    //
    // y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] - a1 * y[n-1] - a2 * y[n-2]
    //
    // Therefore, we have to use negated a1 and a2 for use with the ARM function
    // notch implementation
    //
    // we also have to divide every coefficient by a0 !
    // y[n] = b0/a0 * x[n] + b1/a0 * x[n-1] + b2/a0 * x[n-2] - a1/a0 * y[n-1] - a2/a0 * y[n-2]
    //
    //
    float32_t FSdec = 24000.0; // we need the sampling rate in the decimated path for calculation of the coefficients

    if (FilterPathInfo[ts.filter_path].sample_rate_dec == RX_DECIMATION_RATE_12KHZ)
    {
        FSdec = 12000.0;
    }

    float32_t FS = 48000; // we need this for the treble filter

    // the notch filter is in biquad 1 and works at the decimated sample rate FSdec
    float32_t f0 = ts.notch_frequency;
    float32_t Q = 10; // larger Q gives narrower notch
    float32_t w0 = 2 * PI * f0 / FSdec;
    float32_t alpha = sin(w0) / (2 * Q);
    float32_t a0 = 1; // gain scaling
    float32_t b0,b1,b2,a1,a2;
    float32_t A, S;

    b0 = 1;
    b1 = - 2 * cos(w0);
    b2 = 1;
    a0 = 1 + alpha;
    a1 = 2 * cos(w0); // already negated!
    a2 = alpha - 1; // already negated!

    // scaling the coefficients
    b0 = b0/a0;
    b1 = b1/a0;
    b2 = b2/a0;
    a1 = a1/a0;
    a2 = a2/a0;

    // setting the Coefficients in the notch filter instance
    // while not using pointers
    if (ts.dsp_active & DSP_MNOTCH_ENABLE)
    {
    	IIR_biquad_1.pCoeffs[0] = b0;
        IIR_biquad_1.pCoeffs[1] = b1;
        IIR_biquad_1.pCoeffs[2] = b2;
        IIR_biquad_1.pCoeffs[3] = a1;
        IIR_biquad_1.pCoeffs[4] = a2;
    }
    else   // passthru
    {
        IIR_biquad_1.pCoeffs[0] = 1;
        IIR_biquad_1.pCoeffs[1] = 0;
        IIR_biquad_1.pCoeffs[2] = 0;
        IIR_biquad_1.pCoeffs[3] = 0;
        IIR_biquad_1.pCoeffs[4] = 0;
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
        b0 = 1 + (alpha * A);
        b1 = - 2 * cos(w0);
        b2 = 1 - (alpha * A);
        a0 = 1 + (alpha / A);
        a1 = 2 * cos(w0); // already negated!
        a2 = (alpha/A) - 1; // already negated!
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

        b0 = alpha;
        b1 = 0;
        b2 = - alpha;
        a0 = 1 + alpha;
        a1 = 2 * cos(w0); // already negated!
        a2 = alpha - 1; // already negated!
*/
        // BPF: constant skirt gain, peak gain = Q
    	f0 = ts.peak_frequency;
        Q = 4; //
        float32_t BW = 0.03;
        w0 = 2 * PI * f0 / FSdec;
        alpha = sin (w0) * sinh( log(2) / 2 * BW * w0 / sin(w0) ); //

        b0 = Q * alpha;
        b1 = 0;
        b2 = - Q * alpha;
        a0 = 1 + alpha;
        a1 = 2 * cos(w0); // already negated!
        a2 = alpha - 1; // already negated!

        // scaling the coefficients for gain
        b0 = b0/a0;
        b1 = b1/a0;
        b2 = b2/a0;
        a1 = a1/a0;
        a2 = a2/a0;

        IIR_biquad_1.pCoeffs[5] = b0;
        IIR_biquad_1.pCoeffs[6] = b1;
        IIR_biquad_1.pCoeffs[7] = b2;
        IIR_biquad_1.pCoeffs[8] = a1;
        IIR_biquad_1.pCoeffs[9] = a2;
    }
    else   //passthru
    {
        IIR_biquad_1.pCoeffs[5] = 1;
        IIR_biquad_1.pCoeffs[6] = 0;
        IIR_biquad_1.pCoeffs[7] = 0;
        IIR_biquad_1.pCoeffs[8] = 0;
        IIR_biquad_1.pCoeffs[9] = 0;
    }

    // EQ shelving filters
    //
    // the bass filter is in biquad 1 and works at the decimated sample rate FSdec
    //
    // Bass
    // lowShelf
    //
    f0 = 250;
    w0 = 2 * PI * f0 / FSdec;
    A = powf(10.0,(ts.bass_gain/40.0)); // gain ranges from -20 to 20
    S = 0.7; // shelf slope, 1 is maximum value
    alpha = sin(w0) / 2 * sqrt( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cos(w0);
    float32_t twoAa = 2 * sqrt(A) * alpha;

    // lowShelf
    b0 = A * 		( (A + 1) - (A - 1) * cosw0 + twoAa );
    b1 = 2 * A * 	( (A - 1) - (A + 1) * cosw0 		);
    b2 = A * 		( (A + 1) - (A - 1) * cosw0 - twoAa );
    a0 = 	 		  (A + 1) + (A - 1) * cosw0 + twoAa ;
    a1 = 2 *  		( (A - 1) + (A + 1) * cosw0 		); // already negated!
    a2 = twoAa 		- (A + 1) - (A - 1) * cosw0; // already negated!

    // scaling the coefficients for gain
    b0 = b0/a0;
    b1 = b1/a0;
    b2 = b2/a0;
    a1 = a1/a0;
    a2 = a2/a0;

    // scaling the feedforward coefficients for gain adjustment !
    // "DC gain of an IIR filter is the sum of the filters� feedforward coeffs divided by
    // 1 minus the sum of the filters� feedback coeffs" (Lyons 2011)
    //    float32_t DCgain = (b0 + b1 + b2) / (1 - (a1 + a2));
    // does not work for some reason?
    // I take a divide by a constant instead !
    float32_t DCgain = 1; //
//    float32_t DCgain = (b0 + b1 + b2) / (1 - (- a1 - a2)); // takes into account that a1 and a2 are already negated!
    b0 = b0 / DCgain;
    b1 = b1 / DCgain;
    b2 = b2 / DCgain;

    IIR_biquad_1.pCoeffs[10] = b0;
    IIR_biquad_1.pCoeffs[11] = b1;
    IIR_biquad_1.pCoeffs[12] = b2;
    IIR_biquad_1.pCoeffs[13] = a1;
    IIR_biquad_1.pCoeffs[14] = a2;
    /*
      else {
    IIR_biquad_1.pCoeffs[10] = 1;
    IIR_biquad_1.pCoeffs[11] = 0;
    IIR_biquad_1.pCoeffs[12] = 0;
    IIR_biquad_1.pCoeffs[13] = 0;
    IIR_biquad_1.pCoeffs[14] = 0;
    }
    */
    // Treble = highShelf
    //
    // the treble filter is in biquad 2 and works at 48000ksps
    f0 = 3500;
    FS = 48000;
    w0 = 2 * PI * f0 / FS;
    A = powf(10.0,(ts.treble_gain/40.0)); // gain ranges from -20 to 20
    S = 0.9; // shelf slope, 1 is maximum value
    alpha = sin(w0) / 2 * sqrt( (A + 1/A) * (1/S - 1) + 2 );
    cosw0 = cos(w0);
    twoAa = 2 * sqrt(A) * alpha;
    // highShelf
    //
    b0 = A * 		( (A + 1) + (A - 1) * cosw0 + twoAa );
    b1 = - 2 * A * 	( (A - 1) + (A + 1) * cosw0 		);
    b2 = A * 		( (A + 1) + (A - 1) * cosw0 - twoAa );
    a0 = 	 		  (A + 1) - (A - 1) * cosw0 + twoAa ;
    a1 = - 2 * 		( (A - 1) - (A + 1) * cosw0 		); // already negated!
    a2 = twoAa 		- (A + 1) + (A - 1) * cosw0; // already negated!

    // scaling the coefficients for gain
    b0 = b0/a0;
    b1 = b1/a0;
    b2 = b2/a0;
    a1 = a1/a0;
    a2 = a2/a0;

    DCgain = 1; //

//    DCgain = 2; //
//    DCgain = (b0 + b1 + b2) / (1 + (a1 + a2)); // takes into account that a1 and a2 are already negated!

    b0 = b0 / DCgain;
    b1 = b1 / DCgain;
    b2 = b2 / DCgain;

    IIR_biquad_2.pCoeffs[0] = b0;
    IIR_biquad_2.pCoeffs[1] = b1;
    IIR_biquad_2.pCoeffs[2] = b2;
    IIR_biquad_2.pCoeffs[3] = a1;
    IIR_biquad_2.pCoeffs[4] = a2;

    /*	// pass-thru-coefficients
     	else {
      	IIR_biquad_2.pCoeffs[0] = 1;
    	IIR_biquad_2.pCoeffs[1] = 0;
    	IIR_biquad_2.pCoeffs[2] = 0;
    	IIR_biquad_2.pCoeffs[3] = 0;
    	IIR_biquad_2.pCoeffs[4] = 0;
    	}
    */

    // insert coefficient calculation for TX bass & treble adjustment here!
    // the TX treble filter is in IIR_TX_biquad and works at 48000ksps
    f0 = 1700;
    FS = 48000;
    w0 = 2 * PI * f0 / FS;
    A = powf(10.0,(ts.tx_treble_gain/40.0)); // gain ranges from -20 to 5
    S = 0.9; // shelf slope, 1 is maximum value
    alpha = sin(w0) / 2 * sqrt( (A + 1/A) * (1/S - 1) + 2 );
    cosw0 = cos(w0);
    twoAa = 2 * sqrt(A) * alpha;
    // highShelf
    //
    b0 = A * 		( (A + 1) + (A - 1) * cosw0 + twoAa );
    b1 = - 2 * A * 	( (A - 1) + (A + 1) * cosw0 		);
    b2 = A * 		( (A + 1) + (A - 1) * cosw0 - twoAa );
    a0 = 	 		  (A + 1) - (A - 1) * cosw0 + twoAa ;
    a1 = - 2 * 		( (A - 1) - (A + 1) * cosw0 		); // already negated!
    a2 = twoAa 		- (A + 1) + (A - 1) * cosw0; // already negated!

    // scaling the coefficients for gain
    b0 = b0/a0;
    b1 = b1/a0;
    b2 = b2/a0;
    a1 = a1/a0;
    a2 = a2/a0;

//    DCgain = 2; //
//    DCgain = (b0 + b1 + b2) / (1 - (- a1 - a2)); // takes into account that a1 and a2 are already negated!
    DCgain = 1; //

    b0 = b0 / DCgain;
    b1 = b1 / DCgain;
    b2 = b2 / DCgain;

    IIR_TX_biquad.pCoeffs[0] = b0;
    IIR_TX_biquad.pCoeffs[1] = b1;
    IIR_TX_biquad.pCoeffs[2] = b2;
    IIR_TX_biquad.pCoeffs[3] = a1;
    IIR_TX_biquad.pCoeffs[4] = a2;

    // the TX bass filter is in TX_biquad and works at 48000 sample rate
    //
    // Bass
    // lowShelf
    //
    FS = 48000;
    f0 = 300;
    w0 = 2 * PI * f0 / FSdec;
    A = powf(10.0,(ts.tx_bass_gain/40.0)); // gain ranges from -20 to 5
    S = 0.7; // shelf slope, 1 is maximum value
    alpha = sin(w0) / 2 * sqrt( (A + 1/A) * (1/S - 1) + 2 );
    cosw0 = cos(w0);
    twoAa = 2 * sqrt(A) * alpha;

    // lowShelf
    b0 = A * 		( (A + 1) - (A - 1) * cosw0 + twoAa );
    b1 = 2 * A * 	( (A - 1) - (A + 1) * cosw0 		);
    b2 = A * 		( (A + 1) - (A - 1) * cosw0 - twoAa );
    a0 = 	 		  (A + 1) + (A - 1) * cosw0 + twoAa ;
    a1 = 2 *  		( (A - 1) + (A + 1) * cosw0 		); // already negated!
    a2 = twoAa 		- (A + 1) - (A - 1) * cosw0; // already negated!

    // scaling the coefficients for gain
    b0 = b0/a0;
    b1 = b1/a0;
    b2 = b2/a0;
    a1 = a1/a0;
    a2 = a2/a0;

    // scaling the feedforward coefficients for gain adjustment !
    // "DC gain of an IIR filter is the sum of the filters� feedforward coeffs divided by
    // 1 minus the sum of the filters� feedback coeffs" (Lyons 2011)
    //    float32_t DCgain = (b0 + b1 + b2) / (1 - (a1 + a2));
    // does not work for some reason?
    // I take a divide by a constant instead !
    DCgain = 1; //
//    DCgain = (b0 + b1 + b2) / (1 - (- a1 - a2)); // takes into account that a1 and a2 are already negated!
    b0 = b0 / DCgain;
    b1 = b1 / DCgain;
    b2 = b2 / DCgain;

    IIR_TX_biquad.pCoeffs[5] = b0;
    IIR_TX_biquad.pCoeffs[6] = b1;
    IIR_TX_biquad.pCoeffs[7] = b2;
    IIR_TX_biquad.pCoeffs[8] = a1;
    IIR_TX_biquad.pCoeffs[9] = a2;

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
    //
    for(i = 0; i < IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES; i++)	 	// initialize state buffer to zeroes
    {
        iir_squelch_rx_state[i] = 0;
    }
    IIR_Squelch_HPF.pState = iir_squelch_rx_state;					// point to state array for IIR filter
    //
    // Initialize LMS (DSP Noise reduction) filter
    // It is (sort of) initalized "twice" since this it what it seems to take for the LMS function to
    // start reliably and consistently!
    //
    uint16_t	calc_taps;
    //
    if((ts.dsp_nr_numtaps < DSP_NR_NUMTAPS_MIN) || (ts.dsp_nr_numtaps > DSP_NR_NUMTAPS_MAX))
    {
        calc_taps = DSP_NR_NUMTAPS_DEFAULT;
    }
    else
    {
        calc_taps = (uint16_t)ts.dsp_nr_numtaps;
    }
    //
    // Load settings into instance structure
    //
    // LMS instance 1 is pre-AGC DSP NR
    // LMS instance 3 is post-AGC DSP NR
    //
    lmsData.lms1Norm_instance.numTaps = calc_taps;
    lmsData.lms1Norm_instance.pCoeffs = lmsData.lms1NormCoeff_f32;
    lmsData.lms1Norm_instance.pState = lmsData.lms1StateF32;
    //
    // Calculate "mu" (convergence rate) from user "DSP Strength" setting.  This needs to be significantly de-linearized to
    // squeeze a wide range of adjustment (e.g. several magnitudes) into a fairly small numerical range.
    //
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

    // Debug display of mu calculation
    //	char txt[16];
    //	snprintf(txt,16, " %d ", (ulong)(mu_calc * 10000));
    //	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,txt,0xFFFF,0,0);

    for(i = 0; i < LMS_NR_DELAYBUF_SIZE_MAX + BUFF_LEN; i++)	 		// clear LMS delay buffers
    {
        lmsData.lms1_nr_delay[i] = 0;
    }
    //
    for(i = 0; i < DSP_NR_NUMTAPS_MAX + BUFF_LEN; i++)	 		// clear LMS state buffer
    {
        lmsData.lms1StateF32[i] = 0;			// zero state buffer
        if(ts.reset_dsp_nr)	 			// are we to reset the coefficient buffer as well?
        {
            lmsData.lms1NormCoeff_f32[i] = 0;		// yes - zero coefficient buffers
        }
    }

    // use "canned" init to initialize the filter coefficients
    //
    arm_lms_norm_init_f32(&lmsData.lms1Norm_instance, calc_taps, &lmsData.lms1NormCoeff_f32[0], &lmsData.lms1StateF32[0], mu_calc, 64);
    //

    //
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

    for(i = 0; i < LMS_NOTCH_DELAYBUF_SIZE_MAX + BUFF_LEN; i++)		// clear LMS delay buffer
    {
        lmsData.lms2_nr_delay[i] = 0;
    }

    for(i = 0; i < DSP_NOTCH_NUMTAPS_MAX + BUFF_LEN; i++)	 		// clear LMS state and coefficient buffers
    {
        lmsData.lms2StateF32[i] = 0;			// zero state buffer
        if(ts.reset_dsp_nr)				// are we to reset the coefficient buffer?
        {
            lmsData.lms2NormCoeff_f32[i] = 0;		// yes - zero coefficient buffer
        }
    }

    if((ts.dsp_notch_delaybuf_len > DSP_NOTCH_BUFLEN_MAX) || (ts.dsp_notch_delaybuf_len < DSP_NOTCH_BUFLEN_MIN))
    {
        ts.dsp_nr_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
    }
    // AUTO NOTCH INIT END

    // Adjust decimation rate based on selected filter
    ads.decimation_rate = FilterPathInfo[ts.filter_path].sample_rate_dec;
    if (FilterPathInfo[ts.filter_path].dec != NULL)
    {
        DECIMATE_RX.numTaps = FilterPathInfo[ts.filter_path].dec->numTaps;      // Number of taps in FIR filter
        DECIMATE_RX.pCoeffs = FilterPathInfo[ts.filter_path].dec->pCoeffs;       // Filter coefficients
    }
    else
    {
        DECIMATE_RX.numTaps = 0;
        DECIMATE_RX.pCoeffs = NULL;
    }
    if (FilterPathInfo[ts.filter_path].interpolate != NULL)
    {
        INTERPOLATE_RX.pCoeffs = FilterPathInfo[ts.filter_path].interpolate->pCoeffs; // Filter coefficients
    }
    else
    {
        INTERPOLATE_RX.phaseLength = 0;
        INTERPOLATE_RX.pCoeffs = NULL;
    }

    if(dmod_mode == DEMOD_FM)
    {
        ads.decimation_rate = RX_DECIMATION_RATE_48KHZ;
    }
    //
    //
    ads.agc_decimation_scaling = ads.decimation_rate;
    ads.agc_delay_buflen = AGC_DELAY_BUFSIZE/(ulong)ads.decimation_rate;	// calculate post-AGC delay based on post-decimation sampling rate
    //
    // Set up ZOOM FFT FIR decimation filters

    // switch right FIR decimation filter depending on sd.magnify
    if(sd.magnify > 5)
    {
        sd.magnify = 0;
    }

    {
    	DECIMATE_ZOOM_FFT_I.numTaps = FirZoomFFTDecimate[sd.magnify].numTaps;
        DECIMATE_ZOOM_FFT_I.pCoeffs = FirZoomFFTDecimate[sd.magnify].pCoeffs;
        DECIMATE_ZOOM_FFT_Q.numTaps = FirZoomFFTDecimate[sd.magnify].numTaps;
        DECIMATE_ZOOM_FFT_Q.pCoeffs = FirZoomFFTDecimate[sd.magnify].pCoeffs;
    }

    DECIMATE_ZOOM_FFT_I.M = (1 << sd.magnify);			// Decimation factor
    DECIMATE_ZOOM_FFT_Q.M = (1 << sd.magnify);			// Decimation factor

    DECIMATE_ZOOM_FFT_I.pState = decimZoomFFTIState;			// Filter state variables
    DECIMATE_ZOOM_FFT_Q.pState = decimZoomFFTQState;			// Filter state variables


    // Set up RX decimation/filter
    DECIMATE_RX.M = ads.decimation_rate;			// Decimation factor  (48 kHz / 4 = 12 kHz)


    DECIMATE_RX.pState = decimState;			// Filter state variables
    //
    // Set up RX interpolation/filter
    // NOTE:  Phase Length MUST be an INTEGER and is the number of taps divided by the decimation rate, and it must be greater than 1.
    //
    INTERPOLATE_RX.L = ads.decimation_rate;			// Interpolation factor, L  (12 kHz * 4 = 48 kHz)

    if (FilterPathInfo[ts.filter_path].interpolate != NULL)
    {
        INTERPOLATE_RX.phaseLength = FilterPathInfo[ts.filter_path].interpolate->phaseLength/ads.decimation_rate;    // Phase Length ( numTaps / L )
    }
    else
    {
        INTERPOLATE_RX.phaseLength = 0;
    }

    INTERPOLATE_RX.pState = interpState;		// Filter state variables
    //
    for(i = 0; i < FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS; i++)	 	// Initialize all filter state variables
    {
        decimState[i] = 0;
        decimZoomFFTIState[i] = 0;
        decimZoomFFTQState[i] = 0;
        interpState[i] = 0;
    }
    //
    ads.dsp_zero_count = 0;		// initialize "zero" count to detect if DSP has crashed
    //
    AudioFilter_InitRxHilbertFIR(); // this switches the Hilbert/FIR-filters

    // Unlock - re-enable filtering
    if  (ads.af_disabled) { ads.af_disabled--; }
    if (ts.dsp_inhibit) { ts.dsp_inhibit--; }

}
//

//
//
//*----------------------------------------------------------------------------
//* Function Name       : Audio_TXFilter_Init
//* Object              :
//* Object              : init TX audio filters
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void Audio_TXFilter_Init(uint8_t dmod_mode)
{
    uint32_t	i;

    // -------------------
    // Init TX audio filter - Do so "manually" since built-in init functions don't work with CONST coefficients
    //
    //
    if(dmod_mode != DEMOD_FM)	 						// not FM - use bandpass filter that restricts low and, stops at 2.7 kHz
    {
    	if(ts.tx_filter == TX_FILTER_BASS)
    	{
            IIR_TXFilter.numStages = IIR_TX_WIDE_BASS.numStages;		// number of stages
            IIR_TXFilter.pkCoeffs = IIR_TX_WIDE_BASS.pkCoeffs;	// point to reflection coefficients
            IIR_TXFilter.pvCoeffs = IIR_TX_WIDE_BASS.pvCoeffs;	// point to ladder coefficients
    	}
    	else if (ts.tx_filter == TX_FILTER_TENOR)
    	{
              IIR_TXFilter.numStages = IIR_TX_WIDE_TREBLE.numStages;		// number of stages
              IIR_TXFilter.pkCoeffs = IIR_TX_WIDE_TREBLE.pkCoeffs;	// point to reflection coefficients
              IIR_TXFilter.pvCoeffs = IIR_TX_WIDE_TREBLE.pvCoeffs;	// point to ladder coefficients
    	}
    	else
    	{
    	      IIR_TXFilter.numStages = IIR_TX_SOPRANO.numStages;		// number of stages
    		  IIR_TXFilter.pkCoeffs = IIR_TX_SOPRANO.pkCoeffs;	// point to reflection coefficients
  		      IIR_TXFilter.pvCoeffs = IIR_TX_SOPRANO.pvCoeffs;	// point to ladder coefficients

    	}
   	}
    else	 	// This is FM - use a filter with "better" lows and highs more appropriate for FM
    {
        IIR_TXFilter.numStages = IIR_TX_2k7_FM.numStages;		// number of stages
        IIR_TXFilter.pkCoeffs = IIR_TX_2k7_FM.pkCoeffs;	// point to reflection coefficients
        IIR_TXFilter.pvCoeffs = IIR_TX_2k7_FM.pvCoeffs;	// point to ladder coefficients
    }

    for(i = 0; i < IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES-1; i++)	 	// initialize state buffer to zeroes
    {
        iir_tx_state[i] = 0;
    }
    IIR_TXFilter.pState = iir_tx_state;


    // Decimation/Interpolation is set up "manually" because the built-in functions do NOT work reliably with coefficients
    // stored in CONST memory!
}
//
//*----------------------------------------------------------------------------
//* Function Name       : Audio_Init
//* Object              :
//* Object              : init filters
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void Audio_Init(void)
{

    //
    if((ts.dsp_nr_delaybuf_len < DSP_NR_BUFLEN_MIN) || (ts.dsp_nr_delaybuf_len > DSP_NR_BUFLEN_MAX))
    {
        ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
    }
    // Init RX audio filters
    audio_driver_set_rx_audio_filter(ts.dmod_mode);
}
//
//
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

static void audio_rx_noise_blanker(AudioSample_t * const src, int16_t blockSize)
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
static void audio_rx_freq_conv(int16_t blockSize, int16_t dir)
{
    static bool recalculate_Osc = false;
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

#ifdef USE_FREEDV
static void audio_freedv_rx_processor (AudioSample_t * const src, AudioSample_t * const dst, int16_t blockSize)
{
    // Freedv Test DL2FW
    static int16_t outbuff_count = 0;
    static int16_t trans_count_in = 0;
    static int16_t FDV_TX_fill_in_pt = 0;
    static FDV_Audio_Buffer* out_buffer = NULL;
    static int16_t modulus_NF = 0, modulus_MOD = 0;

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
        // AudioDriver_tx_filter_audio(true,false, adb.a_buffer,adb.a_buffer, blockSize);


        // DOWNSAMPLING
        for (int k = 0; k < blockSize; k++)
        {
            if (k % 6 == modulus_NF)  //every 6th sample has to be catched -> downsampling by 6
            {

        	if (ts.dmod_mode == DEMOD_LSB)
        	  {

        	    fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].real = ((int32_t)adb.q_buffer[k]);
        	    fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].imag = ((int32_t)adb.i_buffer[k]);
        	  }
        	else
        	  {
        	    fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].imag = ((int32_t)adb.q_buffer[k]);
        	    fdv_iq_buff[FDV_TX_fill_in_pt].samples[trans_count_in].real = ((int32_t)adb.i_buffer[k]);
        	  }

        	    // FDV_TX_in_buff[FDV_TX_fill_in_pt].samples[trans_count_in] = 0; // transmit "silence"

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
        if (out_buffer == NULL && fdv_audio_has_data() > 1) {
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

            arm_fill_f32(0,adb.b_buffer,blockSize);
            // UPSAMPLING [by hand]
            for (int j = 0; j < blockSize; j++) //  now we are doing upsampling by 6
            {
                if (modulus_MOD == 0) // put in sample pair
                {
                    adb.b_buffer[j] = out_buffer->samples[outbuff_count]; // + (sample_delta.real * (float32_t)modulus_MOD);
                }
#if 0
                else // in 5 of 6 cases just stuff in zeros = zero-padding / zero-stuffing
                {
                    adb.b_buffer[j] = 0;
                    // adb.b_buffer[j] = out_buffer->samples[outbuff_count];
                }
#endif
                modulus_MOD++;
                if (modulus_MOD == 6)
                {
                    outbuff_count++;
                    modulus_MOD = 0;
                }
            }

            // Add interpolation filter here to suppress alias frequencies
            // we are upsampling from 8kHz to 48kHz, so we have to suppress all frequencies below 4kHz
            // our FreeDV signal is centred at 1500Hz ??? and is 1250Hz broad,
            // so a lowpass filter with cutoff frequency 2400Hz should be fine!
            AudioDriver_tx_filter_audio(true,false, adb.b_buffer,adb.b_buffer, blockSize);
            // INTERPOLATION FILTER [after the interpolation has taken place]
            // the samples are now in adb.i_buffer and adb.q_buffer, so lets filter them
            // arm_fir_f32(&FIR_I_FREEDV, adb.a_buffer, adb.a_buffer,blockSize);
            // arm_fir_f32(&FIR_Q_FREEDV, adb.q_buffer, adb.q_buffer, blockSize);
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
    }
}
#endif

//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_agc_processor
//* Object              :
//* Object              : Processor for receiver AGC
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_agc_processor(int16_t blockSize)
{
    static ulong 		i;
    static ulong		agc_delay_inbuf = 0, agc_delay_outbuf = 0;

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
            {
                ads.agc_calc = ads.am_fm_agc * ads.agc_val;
            }
            else	 							// not AM - get the amplitude of the recovered audio
            {
                ads.agc_calc = fabs(adb.a_buffer[i]) * ads.agc_val;
                //agc_calc = max_signal * ads.agc_val;	// calculate current level by scaling it with AGC value
            }

            float32_t agc_decay_scaled = (ads.agc_calc < ads.agc_knee) ? (ads.agc_decay * ads.agc_decimation_scaling) : AGC_ATTACK;
            // if agc_calc is lower than knee - Increase gain slowly for AGC DECAY - scale time constant with decimation

            ads.agc_var = ads.agc_knee - ads.agc_calc;	// calculate difference between agc value and "knee" value
            ads.agc_var /= ads.agc_knee;	// calculate ratio of difference between knee value and this value
            ads.agc_val += ads.agc_val * agc_decay_scaled * ads.agc_var; // adjust agc_val

            if(ads.agc_val <= AGC_VAL_MIN)	// Prevent zero or "negative" gain values
            {
                ads.agc_val = AGC_VAL_MIN;
            }
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

    arm_copy_f32(adb.a_buffer, &agc_delay[agc_delay_inbuf], blockSize);	// put new data into the delay buffer
    arm_copy_f32(&agc_delay[agc_delay_outbuf], adb.a_buffer, blockSize);	// take old data out of the delay buffer

    // Update the in/out pointers to the AGC delay buffer
    agc_delay_inbuf += blockSize;						// update circular delay buffer
    agc_delay_outbuf = agc_delay_inbuf + blockSize;
    agc_delay_inbuf %= ads.agc_delay_buflen;
    agc_delay_outbuf %= ads.agc_delay_buflen;

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
static void audio_demod_fm(int16_t blockSize)
{

    float r, s, angle, abs_y, x, y, a, b;
    ulong i;
    bool tone_det_enabled;
    static float i_prev, q_prev, lpf_prev, hpf_prev_a, hpf_prev_b;		// used in FM detection and low/high pass processing
    static float q0 = 0, q1 = 0, q2 = 0, r0 = 0, r1 = 0, r2 = 0, s0 = 0, s1 = 0, s2 = 0;		// Goertzel values
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
        //
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
    if(!count)	 		// do the squelch threshold calculation much less often than we are called to process this audio
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
            ads.fm_squelched = 0;		// yes, the we are un-squelched
        }
        else if(ads.fm_squelched)	 	// are we squelched?
        {
            if(b >= (float)(ts.fm_sql_threshold + FM_SQUELCH_HYSTERESIS))		// yes - is average above threshold plus hysteresis?
            {
                ads.fm_squelched = 0 ;		//  yes, open the squelch
            }
        }
        else	 	// is the squelch open (e.g. passing audio)?
        {
            if(ts.fm_sql_threshold > FM_SQUELCH_HYSTERESIS)	 				// is setting higher than hysteresis?
            {
                if(b < (float)(ts.fm_sql_threshold - FM_SQUELCH_HYSTERESIS))		// yes - is average below threshold minus hysteresis?
                {
                    ads.fm_squelched = 1;		// yes, close the squelch
                }
            }
            else	 				// setting is lower than hysteresis so we can't use it!
            {
                if(b < (float)ts.fm_sql_threshold)		// yes - is average below threshold?
                {
                    ads.fm_squelched = 1;		// yes, close the squelch
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
            r0 = ads.fm_goertzel_high_r * r1 - r2 + adb.c_buffer[i];		// perform Goertzel function on audio in "c" buffer
            r2 = r1;
            r1 = r0;

            // Detect energy below target frequency
            s0 = ads.fm_goertzel_low_r * s1 - s2 + adb.c_buffer[i];		// perform Goertzel function on audio in "c" buffer
            s2 = s1;
            s1 = s0;

            // Detect on-frequency energy
            q0 = ads.fm_goertzel_ctr_r * q1 - q2 + adb.c_buffer[i];
            q2 = q1;
            q1 = q0;
        }

        if(gcount >= FM_SUBAUDIBLE_GOERTZEL_WINDOW)	 		// have we accumulated enough samples to do the final energy calculation?
        {
            a = (r1-(r2 * ads.fm_goertzel_high_cos));								// yes - calculate energy at frequency above center and reset detection
            b = (r2 * ads.fm_goertzel_high_sin);
            r = sqrtf(a*a + b*b);
            s = r;
            r0 = 0;
            r1 = 0;
            r2 = 0;

            a = (s1-(s2 * ads.fm_goertzel_low_cos));								// yes - calculate energy at frequency below center and reset detection
            b = (s2 * ads.fm_goertzel_low_sin);
            r = sqrtf(a*a + b*b);
            s += r;					// sum +/- energy levels:  s = "off frequency" energy reading
            s0 = 0;
            s1 = 0;
            s2 = 0;

            a = (q1-(q2 * ads.fm_goertzel_ctr_cos));								// yes - calculate on-frequency energy and reset detection
            b = (q2 * ads.fm_goertzel_ctr_sin);
            r = sqrtf(a*a + b*b);							// r contains "on-frequency" energy
            subdet = ((1 - FM_TONE_DETECT_ALPHA) *subdet) + (r/(s/2) * FM_TONE_DETECT_ALPHA);	// do IIR filtering of the ratio between on and off-frequency energy
            q0 = 0;
            q1 = 0;
            q2 = 0;
            //
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
 static void audio_demod_am(int16_t blockSize)
{
    ulong i, j;
    bool testSAM = 0; // put 0 for normal function, only put 1 with very low RF gain and manual (off) AGC
    if(!testSAM)  // this is DSB demodulation WITHOUT phasing, this is NOT used in the mcHF at the moment
    {
        j = 0;
        for(i = 0; i < blockSize; i++)	 					// interleave I and Q data, putting result in "b" buffer
        {
            adb.b_buffer[j] = adb.i_buffer[i];
            j++;
            adb.b_buffer[j] = adb.q_buffer[i];
            j++;
        }
        //
        // perform complex vector magnitude calculation on interleaved data in "b" to recover
        // instantaneous carrier power:  sqrtf(b[n]^2+b[n+1]^2) - put result in "a"
        //
        arm_cmplx_mag_f32(adb.b_buffer, adb.a_buffer, blockSize);	// use optimized (fast) ARM function
    }
    // this is the very experimental demodulator for DSB
    // demodulates only the real part = I
    //
    if(testSAM)  // this is DSB demodulation WITHOUT phasing, this is NOT used in the mcHF at the moment
    {
        for(i = 0; i < blockSize; i++)	 			// put I into buffer a
        {
            adb.a_buffer[i] = adb.i_buffer[i];
        }
    }
    //
    // Now produce signal/carrier level for AGC
    //
    arm_mean_f32(adb.a_buffer, blockSize, (float32_t *)&ads.am_fm_agc);	// get "average" value of "a" buffer - the recovered DC (carrier) value - for the AGC (always positive since value was squared!)
    ads.am_fm_agc *= AM_SCALING;	// rescale AM AGC to match SSB scaling so that AGC comes out the same

}


//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_demod_am_exp  (experimental! rewritten to eliminate the need for interleaving, DD4WH april 2016)
//* Object              : AM demodulator
//* Object              :
//* Input Parameters    : size - size of buffer on which to operate
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_demod_am_exp(int16_t blockSize)
{
    ulong i;
    float32_t sqrt;
        //
        // uses optimized ARM sqrt function, but not the arm_cmplx_mag, because the latter needs the data in interleaved format!
        // this could possibly make this even faster than first interleaving and then calculating magnitude
    	// (because arm_cmplx_mag uses the same sqrt function )

    for(i = 0; i < blockSize; i++) {
    	 arm_sqrt_f32 (adb.i_buffer[i] * adb.i_buffer[i] + adb.q_buffer[i] * adb.q_buffer[i], &sqrt);
    	 adb.a_buffer[i] = sqrt;
    }

    //
    // Now produce signal/carrier level for AGC
    //
    arm_mean_f32(adb.a_buffer, blockSize, (float32_t *)&ads.am_fm_agc);	// get "average" value of "a" buffer - the recovered DC (carrier) value - for the AGC (always positive since value was squared!)
    //
    ads.am_fm_agc *= AM_SCALING;	// rescale AM AGC to match SSB scaling so that AGC comes out the same

}

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
static void audio_lms_notch_filter(int16_t blockSize)
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
static void audio_lms_noise_reduction(int16_t blockSize)
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

static void audio_snap_carrier (void)
{

    const float32_t buff_len = FFT_IQ_BUFF_LEN2;
    // the calculation of bin_BW is perfectly right at the moment, but we have to change it, if we switch to using the spectrum display zoom FFT to finetune
//    const float32_t bin_BW = (float32_t) (48000.0 * 2.0 / (buff_len * (1 << sd.magnify))); // width of a 1024 tap FFT bin = 46.875Hz, if FFT_IQ_BUFF_LEN2 = 2048 --> 1024 tap FFT
    const float32_t bin_BW = (float32_t) (48000.0 * 2.0 / (buff_len));
    const int buff_len_int = FFT_IQ_BUFF_LEN2;

    float32_t   FFT_MagData[FFT_IQ_BUFF_LEN2/2];

    float32_t bw_LSB = 0.0;
    float32_t bw_USB = 0.0;

    float32_t help_freq = (float32_t)df.tune_old / ((float32_t)TUNE_MULT);

    //	determine posbin (where we receive at the moment) from ts.iq_freq_mode
    // FIXME: this is not the right calculation, at least it is a professional programmers blabla . . .
    // In order for me to understand and to be sure it is the right calculation, I have got to change it, even if it is not a const anymore, sorry! DD4WH, 2016_08_30
    const int posbin = buff_len_int/4  - (buff_len_int * (audio_driver_xlate_freq()/(48000/8)))/16;
    // maybe this would be right AND satisfy the professional programmers search for "elegance" ;-)
    /*
     if (sd.magnify == 0)
     {
     const int posbin = buff_len_int/4  - (buff_len_int * (audio_driver_xlate_freq()/(48000/8)))/16;
     }
     else
     {
          const int posbin = buff_len_int/4;
     }
    */

    // for now, I use this:
/*
    int posbin = buff_len_int/4; // when sd.magnify is != 0 OR freq translation is on

    if(sd.magnify == 0)
    {
    if(ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ)	 	// we are in RF LO HIGH mode (tuning is below center of screen)
  	  {
      posbin = (buff_len_int / 4) - (buff_len_int / 16);
  	  }
    else if(ts.iq_freq_mode == FREQ_IQ_CONV_M6KHZ)	 	// we are in RF LO LOW mode (tuning is above center of screen)
  	  {
      posbin = (buff_len_int / 4) + (buff_len_int / 16);
  	  }
    else if(ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ)	 	// we are in RF LO HIGH mode (tuning is below center of screen)
  	  {
      posbin = (buff_len_int / 4) - (buff_len_int / 8);
  	  }
    else if(ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ)	 	// we are in RF LO LOW mode (tuning is above center of screen)
  	  {
      posbin = (buff_len_int / 4) + (buff_len_int / 8);
  	  }
    }
*/
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

void AudioDriver_CalcIQPhaseAdjust(uint8_t dmod_mode, uint8_t txrx_mode, uint32_t freq)
{
    //
    // the phase adjustment is done by mixing a little bit of I into Q or vice versa
    // this is justified because the phase shift between two signals of equal frequency can
    // be regulated by adjusting the amplitudes of the two signals!
    int iq_phase_balance = 0;

    switch(dmod_mode)
    {
    case DEMOD_USB:
        iq_phase_balance = txrx_mode==TRX_MODE_RX?ts.rx_iq_usb_phase_balance:ts.tx_iq_usb_phase_balance;
        break;
    case DEMOD_LSB:
        iq_phase_balance = txrx_mode==TRX_MODE_RX?ts.rx_iq_lsb_phase_balance:ts.tx_iq_lsb_phase_balance;
        break;
    case DEMOD_AM:
         iq_phase_balance = txrx_mode==TRX_MODE_RX?ts.rx_iq_am_phase_balance:0;
         break;
    default:
        // FM, SAM
        iq_phase_balance = txrx_mode==TRX_MODE_RX?ts.rx_iq_usb_phase_balance:0;
        break;
    }
    ads.iq_phase_balance = ((float32_t)(iq_phase_balance))/SCALING_FACTOR_IQ_PHASE_ADJUST;
}

static void AudioDriver_IQPhaseAdjust(uint8_t dmod_mode, uint8_t txrx_mode, const uint16_t blockSize)
{
    AudioDriver_CalcIQPhaseAdjust(dmod_mode, txrx_mode, ts.tune_freq);

    if (ads.iq_phase_balance < 0)   // we only need to deal with I and put a little bit of it into Q
    {
        AudioDriver_Mix(adb.i_buffer,adb.q_buffer, ads.iq_phase_balance, blockSize);
    }
    else if (ads.iq_phase_balance > 0)  // we only need to deal with Q and put a little bit of it into I
    {
        AudioDriver_Mix(adb.q_buffer,adb.i_buffer, ads.iq_phase_balance, blockSize);
    }
}


void AudioDriver_SpectrumNoZoomProcessSamples(const uint16_t blockSize)
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
void AudioDriver_SpectrumZoomProcessSamples(const uint16_t blockSize)
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

static uint16_t modulus = 0;
// used to divide usb audio out sample rate, set to 0 for 48khz, do not change

#ifdef USE_FREEDV
//
// this is a experimental breakout of the audio_rx_processor for getting  FreeDV to working
// used to help us to figure out how to optimize performance. Lots of performance required for
// digital signal processing...
// then it will probably be most merged back into the rx_processor in  order to keep code duplication minimal
static void audio_dv_rx_processor(AudioSample_t * const src, AudioSample_t * const dst, const uint16_t blockSize)
{
    // we copy volatile variables which are used multiple times to local consts to let the compiler to its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t iq_freq_mode = ts.iq_freq_mode;

#if 0
    audio_rx_noise_blanker(src, blockSize);		// do noise blanker function
#endif
    //
    //
    //
    // ------------------------
    // Split stereo channels
    for(int i = 0; i < blockSize; i++)
    {
        if(src[i].l > ADC_CLIP_WARN_THRESHOLD/4)	 		// This is the release threshold for the auto RF gain
        {
            ads.adc_quarter_clip = 1;
            if(src[i].l > ADC_CLIP_WARN_THRESHOLD/2)	 		// This is the trigger threshold for the auto RF gain
            {
                ads.adc_half_clip = 1;
                if(src[i].l > ADC_CLIP_WARN_THRESHOLD)			// This is the threshold for the red clip indicator on S-meter
                {
                    ads.adc_clip = 1;
                }
            }
        }
        adb.i_buffer[i] = (float32_t)src[i].l;
        adb.q_buffer[i] = (float32_t)src[i].r;
        // HACK: we have 48 khz sample frequency
        //
    }

    AudioDriver_SpectrumNoZoomProcessSamples(blockSize);

    // Apply I/Q amplitude correction
    arm_scale_f32(adb.i_buffer, (float32_t)ts.rx_adj_gain_var_i, adb.i_buffer, blockSize);
    arm_scale_f32(adb.q_buffer, (float32_t)ts.rx_adj_gain_var_q, adb.q_buffer, blockSize);

    // Apply I/Q phase correction
    AudioDriver_IQPhaseAdjust(dmod_mode, ts.txrx_mode,blockSize);

    if(iq_freq_mode)	 		// is receive frequency conversion to be done?
    {
        if(iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ)			// Yes - "RX LO LOW" mode
        {
            audio_rx_freq_conv(blockSize, 1);
        }
        else								// it is in "RX LO LOW" mode
        {
            audio_rx_freq_conv(blockSize, 0);
        }
    }
    AudioDriver_SpectrumZoomProcessSamples(blockSize);

    // ------------------------
    // IQ SSB processing - Do 0-90 degree Phase-added Hilbert Transform
    // *** *EXCEPT* in AM mode
    //    In AM, the FIR below does ONLY low-pass filtering appropriate for the filter bandwidth selected when in AM mode, in
    //	  which case there is ***NO*** audio phase shift applied to the I/Q channels.
#if 0
    arm_fir_f32(&FIR_I,adb.i_buffer, adb.i_buffer,blockSize);	// shift 0 degree FIR+LPF
    arm_fir_f32(&FIR_Q,adb.q_buffer, adb.q_buffer,blockSize);	// shift -90 degrees FIR+LPF
#endif

    switch(dmod_mode)
    {
    case DEMOD_LSB:
        // arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // difference of I and Q - LSB
        audio_freedv_rx_processor(src,dst,blockSize);
        break;
    case DEMOD_USB:
    case DEMOD_DIGI:
        // arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // sum of I and Q - USB
        audio_freedv_rx_processor(src,dst,blockSize);
        break;
    default:
        // this is silence
        arm_fill_f32(0.0,adb.b_buffer,blockSize);
    }
    // from here straight to final AUDIO OUT processing
}

#endif

//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_processor
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_processor(AudioSample_t * const src, AudioSample_t * const dst, const uint16_t blockSize)
{
    const int16_t blockSizeDecim = blockSize/(int16_t)ads.decimation_rate;

    // we copy volatile variables which are used multiple times to local consts to let the compiler to its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t tx_audio_source = ts.tx_audio_source;
    const uint8_t iq_freq_mode = ts.iq_freq_mode;
    const uint8_t  dsp_active = ts.dsp_active;

    static ulong        i, beep_idx = 0;

    float               post_agc_gain_scaling;

    if (tx_audio_source == TX_AUDIO_DIGIQ)
    {

        for(i = 0; i < blockSize; i++)
        {
            //
            // 16 bit format - convert to float and increment
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            if (i%USBD_AUDIO_IN_OUT_DIV == modulus)
            {
                audio_in_put_buffer(src[i].l);
                audio_in_put_buffer(src[i].r);
            }
        }
    }

#ifdef USE_FREEDV
    if (ts.dvmode == true)
    {
        audio_dv_rx_processor(src,dst,blockSize);
    }
    else
#endif
    {

        audio_rx_noise_blanker(src, blockSize);     // do noise blanker function
        // ------------------------
        // Split stereo channels
        for(i = 0; i < blockSize; i++)
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
                audio_snap_carrier(); // tunes the mcHF to the largest signal in the filterpassband
            }
        }
#endif
        AudioDriver_SpectrumNoZoomProcessSamples(blockSize);


        // Apply I/Q amplitude correction
        arm_scale_f32(adb.i_buffer, (float32_t)ts.rx_adj_gain_var_i, adb.i_buffer, blockSize);
        arm_scale_f32(adb.q_buffer, (float32_t)ts.rx_adj_gain_var_q, adb.q_buffer, blockSize);


        // Apply I/Q phase correction
        AudioDriver_IQPhaseAdjust(dmod_mode, ts.txrx_mode,blockSize);

        if(iq_freq_mode)            // is receive frequency conversion to be done?
        {
            if(iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ)           // Yes - "RX LO LOW" mode
            {
                audio_rx_freq_conv(blockSize, 1);
            }
            else                                // it is in "RX LO LOW" mode
            {
                audio_rx_freq_conv(blockSize, 0);
            }
        }

        AudioDriver_SpectrumZoomProcessSamples(blockSize);

        // ------------------------
        // IQ SSB processing - Do 0-90 degree Phase-added Hilbert Transform
        // *** *EXCEPT* in AM mode
        //    In AM, the FIR below does ONLY low-pass filtering appropriate for the filter bandwidth selected when in AM mode, in
        //    which case there is ***NO*** audio phase shift applied to the I/Q channels.
        //
        //
        arm_fir_f32(&FIR_I,adb.i_buffer, adb.i_buffer,blockSize);   // shift 0 degree FIR+LPF
        arm_fir_f32(&FIR_Q,adb.q_buffer, adb.q_buffer,blockSize);   // shift -90 degrees FIR+LPF

        //  Demodulation, optimized using fast ARM math functions as much as possible
        switch(dmod_mode)
        {
        case DEMOD_LSB:
            arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // difference of I and Q - LSB
            break;
        case DEMOD_CW:
            if(!ts.cw_lsb)  // is this USB RX mode?  (LSB of mode byte was zero)
            {
                arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // sum of I and Q - USB
            }
            else    // No, it is LSB RX mode
            {
                arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // difference of I and Q - LSB
            }
            break;
        case DEMOD_AM:
            if (ts.AM_experiment)
            {
                audio_demod_am_exp(blockSize);
            }
            else
            {
                audio_demod_am(blockSize);
            }
            break;
        case DEMOD_SAM:
            arm_sub_f32(adb.i_buffer, adb.q_buffer, adb.f_buffer, blockSize);   // difference of I and Q - LSB
            arm_add_f32(adb.i_buffer, adb.q_buffer, adb.e_buffer, blockSize);   // sum of I and Q - USB
            arm_add_f32(adb.e_buffer, adb.f_buffer, adb.a_buffer, blockSize);   // sum of LSB & USB = DSB
            break;
        case DEMOD_FM:
            audio_demod_fm(blockSize);
            break;
        case DEMOD_USB:
        case DEMOD_DIGI:
        default:
            arm_add_f32(adb.i_buffer, adb.q_buffer, adb.a_buffer, blockSize);   // sum of I and Q - USB
            break;
        }

        if(dmod_mode != DEMOD_FM)       // are we NOT in FM mode?  If we are not, do decimation, filtering, DSP notch/noise reduction, etc.
        {
            // Do decimation down to lower rate to reduce processor load
            if (DECIMATE_RX.numTaps > 0)
            {
                arm_fir_decimate_f32(&DECIMATE_RX, adb.a_buffer, adb.a_buffer, blockSize);      // LPF built into decimation (Yes, you can decimate-in-place!)
            }

            if((!ads.af_disabled) && (dsp_active & DSP_NOTCH_ENABLE) && (dmod_mode != DEMOD_CW) && (!ts.dsp_inhibit))       // No notch in CW
            {
                audio_lms_notch_filter(blockSizeDecim);     // Do notch filter
            }

            //
            // DSP noise reduction using LMS (Least Mean Squared) algorithm
            // This is the pre-filter/AGC instance
            //
            if((dsp_active & DSP_NR_ENABLE) && (!(dsp_active & DSP_NR_POSTAGC_ENABLE)) && (!ads.af_disabled) && (!ts.dsp_inhibit))      // Do this if enabled and "Pre-AGC" DSP NR enabled
            {
                audio_lms_noise_reduction(blockSizeDecim);
            }

            // Apply audio  bandpass filter
            if ((!ads.af_disabled)  && (IIR_PreFilter.numStages > 0))   // yes, we want an audio IIR filter
            {
                arm_iir_lattice_f32(&IIR_PreFilter, adb.a_buffer, adb.a_buffer, blockSizeDecim);
            }

            // now process the samples and perform the receiver AGC function
            audio_rx_agc_processor(blockSizeDecim);

            // DSP noise reduction using LMS (Least Mean Squared) algorithm
            // This is the post-filter, post-AGC instance
            //
            if((dsp_active & DSP_NR_ENABLE) && (dsp_active & DSP_NR_POSTAGC_ENABLE) && (!ads.af_disabled) && (!ts.dsp_inhibit))     // Do DSP NR if enabled and if post-DSP NR enabled
            {
                audio_lms_noise_reduction(blockSizeDecim);
            }
            //
            // Calculate scaling based on decimation rate since this affects the audio gain
            if ((FilterPathInfo[ts.filter_path].sample_rate_dec) == RX_DECIMATION_RATE_12KHZ)
            {
                post_agc_gain_scaling = POST_AGC_GAIN_SCALING_DECIMATE_4;
            }
            else
            {
                post_agc_gain_scaling = POST_AGC_GAIN_SCALING_DECIMATE_2;
            }
            //
            // Scale audio to according to AGC setting, demodulation mode and required fixed levels and scaling
            //
            if(dmod_mode == DEMOD_AM)
            {
                arm_scale_f32(adb.a_buffer,(float32_t)(ads.post_agc_gain * post_agc_gain_scaling * (AM_SCALING * AM_AUDIO_SCALING)), adb.a_buffer, blockSizeDecim); // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
            }
            else        // Not AM
            {
                arm_scale_f32(adb.a_buffer,(float32_t)(ads.post_agc_gain * post_agc_gain_scaling), adb.a_buffer, blockSizeDecim);   // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
            }

            // this is the biquad filter, a notch, peak, and lowshelf filter
            if (!ads.af_disabled)
            {
                arm_biquad_cascade_df1_f32 (&IIR_biquad_1, adb.a_buffer,adb.a_buffer, blockSizeDecim);
            }
            // resample back to original sample rate while doing low-pass filtering to minimize audible aliasing effects
            //
            if (INTERPOLATE_RX.phaseLength > 0)
            {
                arm_fir_interpolate_f32(&INTERPOLATE_RX, adb.a_buffer,(float32_t *) adb.b_buffer, blockSizeDecim);
            }
            // additional antialias filter for specific bandwidths
            // IIR ARMA-type lattice filter
            if ((!ads.af_disabled) && (IIR_AntiAlias.numStages > 0))   // yes, we want an interpolation IIR filter
            {
                arm_iir_lattice_f32(&IIR_AntiAlias, adb.b_buffer, adb.b_buffer, blockSize);
            }

        } // end NOT in FM mode
        else            // it is FM - we don't do any decimation, interpolation, filtering or any other processing - just rescale audio amplitude
        {
            if(ts.flags2 & FLAGS2_FM_MODE_DEVIATION_5KHZ)       // is this 5 kHz FM mode?  If so, scale down (reduce) audio to normalize
            {
                arm_scale_f32(adb.a_buffer,(float32_t)FM_RX_SCALING_5K, adb.b_buffer, blockSizeDecim);  // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
            }
            else        // it is 2.5 kHz FM mode:  Scale audio level accordingly
            {
                arm_scale_f32(adb.a_buffer,(float32_t)FM_RX_SCALING_2K5, adb.b_buffer, blockSizeDecim); // apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
            }
        }

        // this is the biquad filter, a highshelf filter
        if (!ads.af_disabled)
        {
            arm_biquad_cascade_df1_f32 (&IIR_biquad_2, adb.b_buffer,adb.b_buffer, blockSize);
        }
    }

    if((ads.af_disabled) || (ts.rx_muting) || ((dmod_mode == DEMOD_FM) && ads.fm_squelched))
        // fill audio buffers with zeroes if we are to mute the receiver completely while still processing data OR it is in FM and squelched
        // or when filters are switched
    {
        arm_fill_f32(0, adb.a_buffer, blockSize);
        arm_fill_f32(0, adb.b_buffer, blockSize);
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
        //
        dst[i].l = adb.b_buffer[i];        // Speaker channel (variable level)
        dst[i].r = adb.a_buffer[i];        // LINE OUT (constant level)

        // Unless this is DIGITAL I/Q Mode, we sent processed audio
        if (tx_audio_source != TX_AUDIO_DIGIQ)
        {
            if (i%USBD_AUDIO_IN_OUT_DIV == modulus)
            {
                float32_t val = adb.a_buffer[i] * ts.rx_gain[RX_AUDIO_DIG].value/31.0;
                audio_in_put_buffer(val);
                if (USBD_AUDIO_IN_CHANNELS == 2)
                {
                    audio_in_put_buffer(val);
                }
            }
        }
    }
    // calculate the first index we read so that we are not loosing
    // values.
    // For 1 and 2,4 we do not need to shift modulus
    // since (SIZE/2) % USBD_AUDIO_IN_OUT_DIV == 0
    // if someone needs lower rates, just add formula or values
    // but this would bring us down to less than 12khz bitrate
    if (USBD_AUDIO_IN_OUT_DIV == 3)
    {
        modulus++;
        modulus%=USBD_AUDIO_IN_OUT_DIV;
    }
}


//
//*----------------------------------------------------------------------------
//* Function Name       : audio_tx_compressor (look-ahead type) by KA7OEI
//* Object              :
//* Object              : speech compressor/processor for TX audio
//* Input Parameters    : size of buffer to processes, gain scaling factor
//* Input Parameters    : also processes/compresses audio in "adb.i_buffer" and "adb.q_buffer" - it looks only at data in "i" buffer
//* Output Parameters   : data via "adb.i_buffer" and "adb.q_buffer"
//* Functions called    : none
//*----------------------------------------------------------------------------
static void audio_tx_compressor(int16_t blockSize, float gain_scaling)
{

    ulong i;
    static ulong		alc_delay_inbuf = 0, alc_delay_outbuf = 0;
    static float		alc_calc, alc_var;

    if(!ts.tune)        // do post-filter gain calculations if we are NOT in TUNE mode
    {
        // perform post-filter gain operation
        // this is part of the compression
        //
        float32_t gain_calc = (float)ts.alc_tx_postfilt_gain_var;       // get post-filter gain setting
        gain_calc /= 2;                                 // halve it
        gain_calc += 0.5;                               // offset it so that 2 = unity

        arm_scale_f32(adb.i_buffer, (float32_t)gain_calc, adb.i_buffer, blockSize);      // use optimized function to apply scaling to I/Q buffers
        if (ts.dmod_mode != DEMOD_FM) {
            arm_scale_f32(adb.q_buffer, (float32_t)gain_calc, adb.q_buffer, blockSize);
        }
    }

    // ------------------------
    // Do ALC processing on audio buffer - look-ahead type by KA7OEI
    for(i = 0; i < blockSize; i++)
    {
        if(!ts.tune)	 	// if NOT in TUNE mode, do ALC processing
        {
            // perform ALC on post-filtered audio (You will notice the striking similarity to the AGC code!)
            //
            alc_calc = fabs(adb.i_buffer[i] * ads.alc_val);	// calculate current level by scaling it with ALC value (both channels will be the same amplitude-wise)
            if(alc_calc < ALC_KNEE)	 	// is audio below ALC "knee" value?
            {
                alc_var = ALC_KNEE - alc_calc;	// calculate difference between ALC value and "knee" value
                alc_var /= ALC_KNEE;			// calculate ratio of difference between knee value and this value
                ads.alc_val += ads.alc_val * ads.alc_decay * alc_var;	// (ALC DECAY) Yes - Increase gain slowly
            }
            else
            {
                alc_var = alc_calc - ALC_KNEE;			// calculate difference between ALC value and "knee" value
                alc_var /= ALC_KNEE;			// calculate ratio of difference between knee value and this value
                ads.alc_val -= ads.alc_val * ALC_ATTACK * alc_var;	// Fast attack to increase gain
                if(ads.alc_val <= ALC_VAL_MIN)	// Prevent zero or "negative" gain values
                {
                    ads.alc_val = ALC_VAL_MIN;
                }
            }
            if(ads.alc_val >= ALC_VAL_MAX)	// limit to fixed values within the code
            {
                ads.alc_val = ALC_VAL_MAX;
            }
        }
        else	 	// are we in TUNE mode?
        {
            ads.alc_val = ALC_VAL_MAX;		// yes, disable ALC and set to MAXIMUM ALC gain (e.g. unity - no gain reduction)
        }
        adb.agc_valbuf[i] = (ads.alc_val * gain_scaling);	// store in "running" ALC history buffer for later application to audio data
    }
    //
    // Delay the post-ALC audio slightly so that the ALC's "attack" will very slightly lead the audio being acted upon by the ALC.
    // This eliminates a "click" that can occur when a very strong signal appears due to the ALC lag.  The delay is adjusted based on
    // decimation rate so that it is constant for all settings.
    //
    arm_copy_f32(adb.a_buffer, (float32_t *)&agc_delay[alc_delay_inbuf], blockSize);	// put new data into the delay buffer
    arm_copy_f32((float32_t *)&agc_delay[alc_delay_outbuf], adb.a_buffer, blockSize);	// take old data out of the delay buffer
    //
    // Update the in/out pointers to the ALC delay buffer
    //
    alc_delay_inbuf += blockSize;
    alc_delay_outbuf = alc_delay_inbuf + blockSize;
    alc_delay_inbuf %= ALC_DELAY_BUFSIZE;
    alc_delay_outbuf %= ALC_DELAY_BUFSIZE;
    //
    arm_mult_f32(adb.i_buffer, adb.agc_valbuf, adb.i_buffer, blockSize);		// Apply ALC gain corrections to both TX audio channels
    arm_mult_f32(adb.q_buffer, adb.agc_valbuf, adb.q_buffer, blockSize);
}

// Equalize based on band and simultaneously apply I/Q gain AND phase adjustments
void audio_tx_final_iq_processing(float scaling, bool swap, AudioSample_t* const dst, const uint16_t blockSize)
{
    int16_t i;

    // ------------------------
    // Output I and Q as stereo data
    if(swap == false)	 			// if is it "RX LO LOW" mode, save I/Q data without swapping, putting it in "upper" sideband (above the LO)
    {
        // this is the IQ gain / amplitude adjustment
        arm_scale_f32(adb.i_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_i * scaling), adb.i_buffer, blockSize);
        arm_scale_f32(adb.q_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_q * scaling), adb.q_buffer, blockSize);
        // this is the IQ phase adjustment
        AudioDriver_IQPhaseAdjust(ts.dmod_mode,ts.txrx_mode,blockSize);
         for(i = 0; i < blockSize; i++)
        {
            // Prepare data for DAC
            dst[i].l = adb.i_buffer[i];	// save left channel
            dst[i].r = adb.q_buffer[i];	// save right channel
        }
    }
    else	 	// it is "RX LO HIGH" - swap I/Q data while saving, putting it in the "lower" sideband (below the LO)
    {
        // this is the IQ gain / amplitude adjustment
        arm_scale_f32(adb.i_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_q * scaling), adb.i_buffer, blockSize);
        arm_scale_f32(adb.q_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_i * scaling), adb.q_buffer, blockSize);
        // this is the IQ phase adjustment
        AudioDriver_IQPhaseAdjust(ts.dmod_mode,ts.txrx_mode,blockSize);

        for(i = 0; i < blockSize; i++)
        {
            // Prepare data for DAC
            dst[i].l = adb.q_buffer[i];	// save left channel
            dst[i].r = adb.i_buffer[i];	// save right channel
        }
    }
}

/**
 * @brief Delay function, basically implementing a filter which provides nothing but a delay of (numTaps -1) / 2;
 * @param[in]  S          points to an instance of the floating-point FIR structure.
 * @param[in]  pSrc       points to the block of input data.
 * @param[out] pDst       points to the block of output data.
 * @param[in]  blockSize  number of samples to process.
 */
void AudioDriver_delay_f32(
    const arm_fir_instance_f32 * S,
    float32_t * pSrc,
    float32_t * pDst,
    uint32_t blockSize)
{
    uint32_t delay = (S->numTaps-1)/2;

    if (blockSize > delay) {
        arm_copy_f32(S->pState,pDst,delay); // fill the first delay bytes from buffer content
        arm_copy_f32(pSrc,&pDst[delay],blockSize-delay); // fill the block with the remaining first bytes from new data
        arm_copy_f32(&pSrc[blockSize-delay],S->pState,delay); // and put the rest in the delay buffer;
    } else {
       arm_copy_f32(S->pState,pDst,blockSize); // take oldest part of delay and put in buffer
       arm_copy_f32(&S->pState[blockSize],S->pState,delay); // move remaining data to front of buffer;
       arm_copy_f32(pSrc,&S->pState[delay],blockSize); // append new data to buffer
    }
}

float32_t AudioDriver_absmax(float32_t* buffer, int size) {
    float32_t min, max;
    uint32_t            pindex;

    arm_max_f32(buffer, size, &max, &pindex);      // find absolute value of audio in buffer after gain applied
    arm_min_f32(buffer, size, &min, &pindex);

    return -min>max?-min:max;
}

static void AudioDriver_tx_fill_audio_buffer(AudioSample_t * const src, int16_t blockSize)
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
            gain_calc = ts.tx_gain[TX_AUDIO_DIG];     // We are in MIC In mode:  Calculate Microphone gain
            gain_calc /= 16;              // rescale microphone gain to a reasonable range
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
void AudioDriver_tx_am_sideband_processor(float32_t* I_buffer, float32_t* Q_buffer,  const int16_t blockSize) {

    // generate AM carrier by applying a "DC bias" to the audio
    //
    arm_offset_f32(I_buffer, AM_CARRIER_LEVEL, adb.i_buffer, blockSize);
    arm_offset_f32(Q_buffer, (-1 * AM_CARRIER_LEVEL), adb.q_buffer, blockSize);
    //
    // check and apply correct translate mode
    //
    audio_rx_freq_conv(blockSize, (ts.iq_freq_mode == FREQ_IQ_CONV_M6KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ));
}

static inline void AudioDriver_tx_filter_audio(bool do_bandpass, bool do_bass_treble, float32_t* inBlock, float32_t* outBlock, const uint16_t blockSize)
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

static void audio_tx_fm_processor(AudioSample_t * const src, AudioSample_t * const dst, uint16_t blockSize)
{
    static float32_t    hpf_prev_a, hpf_prev_b;
    float32_t           a, b;

    static uint32_t fm_mod_idx = 0, fm_mod_accum = 0, fm_tone_idx = 0, fm_tone_accum = 0, fm_tone_burst_idx = 0, fm_tone_burst_accum = 0;

    float32_t fm_mod_mult;
    // Fill I and Q buffers with left channel(same as right)
    //
    if(ts.flags2 & FLAGS2_FM_MODE_DEVIATION_5KHZ)   // are we in 5 kHz modulation mode?
    {
        fm_mod_mult = 2;    // yes - multiply all modulation factors by 2
    }
    else
    {
        fm_mod_mult = 1;    // not in 5 kHz mode - used default (2.5 kHz) modulation factors
    }

    AudioDriver_tx_fill_audio_buffer(src,blockSize);

    AudioDriver_tx_filter_audio(true,ts.tx_audio_source != TX_AUDIO_DIG,adb.a_buffer,adb.i_buffer, blockSize);

    audio_tx_compressor(blockSize, FM_ALC_GAIN_CORRECTION);  // Do the TX ALC and speech compression/processing
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
    for(int i = 0; i < blockSize; i++)
    {
        // Calculate next sample
        fm_mod_accum += (ulong)(FM_FREQ_MOD_WORD + (adb.a_buffer[i] * FM_MOD_SCALING * fm_mod_mult));   // change frequency using scaled audio
        fm_mod_accum &= 0xffff;             // limit to 64k range
        fm_mod_idx    = fm_mod_accum >> FM_MOD_DDS_ACC_SHIFT;
        fm_mod_idx &= (DDS_TBL_SIZE - 1);       // limit lookup to range of sine table
        adb.i_buffer[i] = (float32_t)(DDS_TABLE[fm_mod_idx]);               // Load I value
        fm_mod_idx += (DDS_TBL_SIZE/4); // do 90 degree shift by indexing 1/4 into sine table
        fm_mod_idx &= (DDS_TBL_SIZE - 1);       // limit lookup to range of sine table
        adb.q_buffer[i] = (float32_t)(DDS_TABLE[fm_mod_idx]);   // Load Q value
    }

    bool swap = (ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ || ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ);

    audio_tx_final_iq_processing(FM_MOD_AMPLITUDE_SCALING, swap, dst, blockSize);
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
static void audio_dv_tx_processor (AudioSample_t * const src, AudioSample_t * const dst, int16_t blockSize)
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

        AudioDriver_tx_fill_audio_buffer(src,blockSize);

        // *****************************   DV Modulator goes here - ads.a_buffer must be at 8 ksps

        // Freedv Test DL2FW

        // we have to add a decimation filter here BEFORE we decimate
        // for decimation-by-6 the stopband frequency is 48/6*2 = 4kHz
        // but our audio is at most 3kHz wide, so we should use 3k or 2k9


        // this is the correct DECIMATION FILTER (before the downsampling takes place):
        // use it ALWAYS, also with TUNE tone!!!
        AudioDriver_tx_filter_audio(true,false, adb.a_buffer,adb.a_buffer, blockSize);


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

            audio_rx_freq_conv(blockSize, swap);
        }
#endif

        // apply I/Q amplitude & phase adjustments
        audio_tx_final_iq_processing(20.0*SSB_GAIN_COMP, ts.dmod_mode != DEMOD_LSB, dst, blockSize);
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
static void audio_tx_processor(AudioSample_t * const src, AudioSample_t * const dst, uint16_t blockSize)
{
    // we copy volatile variables which are used multiple times to local consts to let the compiler do its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t tx_audio_source = ts.tx_audio_source;
    const uint8_t tx_audio_sink   =  (ts.debug_tx_audio == true)?TX_AUDIO_DIGIQ:TX_AUDIO_LINEIN_L;
    // we use TX_AUDIO_LINEIN_L as placeholder for no tx audio output on USB

    const uint8_t tune = ts.tune;
    const uint8_t iq_freq_mode = ts.iq_freq_mode;


    bool signal_active = false; // unless this is set to true, zero output will be generated

    // If source is digital usb in, pull from USB buffer, discard line or mic audio and
    // let the normal processing happen
    if (tx_audio_source == TX_AUDIO_DIG || tx_audio_source == TX_AUDIO_DIGIQ)
    {
        // FIXME: change type of audio_out_fill_tx_buffer to use audio sample struct
        audio_out_fill_tx_buffer((int16_t*)src,2*blockSize);
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

        audio_tx_final_iq_processing(1.0, false, dst, blockSize);
        signal_active = true;
    }
    else if (ts.dvmode) {
#ifdef USE_FREEDV
        audio_dv_tx_processor(src,dst,blockSize);
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
            signal_active = cw_gen_process(adb.i_buffer, adb.q_buffer,blockSize) != 0;
        }

        if (signal_active)
        {
            // apply I/Q amplitude & phase adjustments
            // Wouldn't it be necessary to include IF conversion here? DD4WH June 16th, 2016
            // Answer: NO, in CW that is done be changing the Si570 frequency during TX/RX switching . . .
            audio_tx_final_iq_processing(1.0, ts.cw_lsb == 0, dst, blockSize);
        }
    }
    // SSB processor
    else if((dmod_mode == DEMOD_LSB) || (dmod_mode == DEMOD_USB))
    {
        if (ads.tx_filter_adjusting == false)
        {
            AudioDriver_tx_fill_audio_buffer(src,blockSize);


            if (!tune)
            {
                AudioDriver_tx_filter_audio(true,tx_audio_source != TX_AUDIO_DIG, adb.a_buffer,adb.a_buffer, blockSize);
            }

            //
            // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
            // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
            // + 0 deg to I data
            arm_fir_f32(&FIR_I_TX,adb.a_buffer,adb.i_buffer,blockSize);
            // - 90 deg to Q data
            arm_fir_f32(&FIR_Q_TX,adb.a_buffer,adb.q_buffer, blockSize);

            audio_tx_compressor(blockSize, SSB_ALC_GAIN_CORRECTION);	// Do the TX ALC and speech compression/processing

            if(iq_freq_mode)
            {
                // is transmit frequency conversion to be done?
                // USB && (-6kHz || -12kHz) --> false, else true
                // LSB && (+6kHz || +12kHz) --> false, else true
                bool swap = dmod_mode == DEMOD_LSB && (iq_freq_mode == FREQ_IQ_CONV_M6KHZ || iq_freq_mode == FREQ_IQ_CONV_M12KHZ);
                swap = swap || ((dmod_mode == DEMOD_USB) && (iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ));

                audio_rx_freq_conv(blockSize, swap);
            }

            // apply I/Q amplitude & phase adjustments
            audio_tx_final_iq_processing(SSB_GAIN_COMP, dmod_mode == DEMOD_LSB, dst, blockSize);
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
            AudioDriver_tx_fill_audio_buffer(src,blockSize);
            //
            // Apply the TX equalization filtering:  This "flattens" the audio
            // prior to being applied to the Hilbert transformer as well as added low-pass filtering.
            // It does this by applying a "peak" to the bottom end to compensate for the roll-off caused by the Hilbert
            // and then a gradual roll-off toward the high end.  The net result is a very flat (to better than 1dB) response
            // over the 275-2500 Hz range.
            //

            AudioDriver_tx_filter_audio((ts.flags1 & FLAGS1_AM_TX_FILTER_DISABLE) == false,tx_audio_source != TX_AUDIO_DIG, adb.a_buffer,adb.a_buffer , blockSize);
            //
            // This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
            // to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
            // Apply transformation AND audio filtering to buffer data
            //
            // + 0 deg to I data
            // AudioDriver_delay_f32((arm_fir_instance_f32 *)&FIR_I_TX,(float32_t *)(adb.a_buffer),(float32_t *)(adb.i_buffer),blockSize);
            arm_fir_f32(&FIR_I_TX,adb.a_buffer,adb.i_buffer,blockSize);
            // - 90 deg to Q data
            arm_fir_f32(&FIR_Q_TX,adb.a_buffer,adb.q_buffer, blockSize);

            audio_tx_compressor(blockSize, AM_ALC_GAIN_CORRECTION);	// Do the TX ALC and speech compression/processing
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
            AudioDriver_tx_am_sideband_processor(adb.i_buffer,adb.q_buffer,blockSize);

            arm_copy_f32(adb.i_buffer, adb.i2_buffer, blockSize);
            arm_copy_f32(adb.q_buffer, adb.q2_buffer, blockSize);
            // i2/q2 now contain the LSB AM signal

            // now generate USB AM sideband signal
            AudioDriver_tx_am_sideband_processor(adb.b_buffer,adb.a_buffer,blockSize);

            arm_add_f32(adb.i2_buffer,adb.q_buffer,adb.i_buffer,blockSize);
            arm_add_f32(adb.q2_buffer,adb.i_buffer,adb.q_buffer,blockSize);

            audio_tx_final_iq_processing(AM_GAIN_COMP, false, dst, blockSize);
            signal_active = true;
        }
    }
    else if(dmod_mode == DEMOD_FM)	 	//	Is it in FM mode
    {
        //  *AND* is frequency translation active (No FM possible unless in frequency translate mode!)
        if (iq_freq_mode)
        {
            // FM handler  [KA7OEI October, 2015]
            audio_tx_fm_processor(src,dst,blockSize);
            signal_active = true;
        }
    }

    if (signal_active == false)
    {
        memset(dst,0,blockSize*sizeof(*dst));
        // Pause or inactivity
    }
    if (tx_audio_sink == TX_AUDIO_DIGIQ)
    {

        for(int i = 0; i < blockSize; i++)
        {
            //
            // 16 bit format - convert to float and increment
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            audio_in_put_buffer(dst[i].r);
            audio_in_put_buffer(dst[i].l);
        }
    }
    else if (tx_audio_sink == TX_AUDIO_DIG)
    {
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
void I2S_RX_CallBack(int32_t *src, int32_t *dst, int16_t size, uint16_t ht)
#else
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t size, uint16_t ht)
#endif
{
    static bool to_rx = 0;	// used as a flag to clear the RX buffer
    static bool to_tx = 0;	// used as a flag to clear the TX buffer
    static ulong tcount = 0;


    if(ts.show_tp_coordinates)
    {
  	  mchf_board_green_led(1);
    }
    if((ts.txrx_mode == TRX_MODE_RX))
    {
        if((to_rx) || (ts.buffer_clear))	 	// the first time back to RX, clear the buffers to reduce the "crash"
        {
            to_rx = 0;							// caused by the content of the buffers from TX - used on return from SSB TX
            arm_fill_q15(0, dst, size);
            arm_fill_q15(0, src, size);
            audio_driver_ClearAGCDelayBuffer();
        }

        audio_rx_processor((AudioSample_t*) src, (AudioSample_t*)dst,size/2);

        to_tx = 1;		// Set flag to indicate that we WERE receiving when we go back to transmit mode
    }
    else  			// Transmit mode
    {
        if((to_tx) || (ts.tx_audio_muting_flag))	 	// the first time back to RX, or TX audio muting timer still active - clear the buffers to reduce the "crash"
        {
            to_tx = 0;							// caused by the content of the buffers from TX - used on return from SSB TX
            arm_fill_q15(0, dst, size);
            arm_fill_q15(0, src, size);
            audio_driver_ClearAGCDelayBuffer();
        }
        else
        {
            audio_tx_processor((AudioSample_t*) src, (AudioSample_t*)dst,size/2);
        }
        to_rx = 1;		// Set flag to indicate that we WERE transmitting when we eventually go back to receive mode
    }
    //
    if(ts.unmute_delay_count)		// this updates at 375 Hz - used to time TX->RX delay
        ts.unmute_delay_count--;
    //
    ks.debounce_time++;				// keyboard debounce timer
    if(ks.debounce_time > DEBOUNCE_TIME_MAX)
        ks.debounce_time = DEBOUNCE_TIME_MAX;
    //
    // Perform LCD backlight PWM brightness function
    //
    UiLcdHy28_BacklightDimHandler();
    //
    //
    tcount+=CLOCKS_PER_DMA_CYCLE;		// add the number of clock cycles that would have passed between DMA cycles
    if(tcount > CLOCKS_PER_CENTISECOND)	 	// has enough clock cycles for 0.01 second passed?
    {
        tcount -= CLOCKS_PER_CENTISECOND;	// yes - subtract that many clock cycles
        ts.sysclock++;	// this clock updates at PRECISELY 100 Hz over the long term
        //
        // Has the timing for the keyboard beep expired?
        //
        if(ts.sysclock > ts.beep_timing)
        {
            ts.beep_active = 0;				// yes, turn the tone off
            ts.beep_timing = 0;
        }
    }
    //
    ts.thread_timer = 0;	// used to trigger the UI Driver thread
    //
    if(ts.spectrum_scope_scheduler)		// update thread timer if non-zero
    {
        ts.spectrum_scope_scheduler--;
    }

    if(ts.show_tp_coordinates)
    {
  	  mchf_board_green_led(0);
    }
}
