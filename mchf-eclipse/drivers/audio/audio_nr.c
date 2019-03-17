/************************************************************************************
 **                                                                                 **
 **                               UHSDR Firmware                                    **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Description:                                                                   **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/

#include "uhsdr_board_config.h"
#include "audio_nr.h"
#include "arm_const_structs.h"
#include "profiling.h"

//#define debug_alternate_NR

#ifdef USE_ALTERNATE_NR

static void alt_noise_blanking();
static void spectral_noise_reduction_3();
static void AudioNr_RunNoiseReduction(float32_t* inputsamples, float32_t* outputsamples );

typedef struct NoiseReduction // declaration
{
    float32_t                   last_iFFT_result [NR_FFT_L_2 / 2];
    float32_t                   last_sample_buffer_L [NR_FFT_L_2 / 2];
    float32_t                   FFT_buffer[NR_FFT_L_2 * 2];
    float32_t                   Nest[NR_FFT_L_2 / 2][2]; // noise estimates for the current and the last FFT frame
    float32_t                   vk; // saved 0.24kbytes
    float32_t                   SNR_prio[NR_FFT_L_2 / 2];
    float32_t                   SNR_post[NR_FFT_L_2 / 2];
    float32_t                   SNR_post_pos; // saved 0.24kbytes
    float32_t                   Hk_old[NR_FFT_L_2 / 2];
//  float32_t                   VAD;
//  float32_t                   VAD_Esch; // holds the VAD sum for the Esh & Vary 2009 type of VAD
//  float32_t                   notch1_f;
//  bool                        notch1_enable;
//  float32_t                   notch2_f;
//  bool                        notch2_enable;
    //ulong                     long_tone_counter;
    uint16_t current_buffer_idx;
    bool was_here;

} NoiseReduction;


NoiseReduction __MCHF_SPECIALMEM 	NR; // definition
NoiseReduction2 NR2; // definition

__IO int32_t NR_in_head = 0;
__IO int32_t NR_in_tail = 0;
__IO int32_t NR_out_head = 0;
__IO int32_t NR_out_tail = 0;

NR_Buffer* NR_in_buffers[NR_BUFFER_FIFO_SIZE];
NR_Buffer* NR_out_buffers[NR_BUFFER_FIFO_SIZE];

/*const float32_t SQRT_van_hann[128]= {0.000000000, 0.024734427, 0.04945372, 0.074142753, 0.098786418, 0.123369638, 0.14787737, 0.172294617,
	      0.196606441, 0.220797963, 0.244854382, 0.268760979, 0.292503125, 0.316066292, 0.339436063, 0.362598137,
	      0.385538344, 0.408242645, 0.430697148, 0.452888114, 0.474801964, 0.49642529, 0.51774486, 0.53874763,
	      0.559420747, 0.579751564, 0.599727639, 0.619336749, 0.638566896, 0.657406313, 0.675843473, 0.693867094,
	      0.711466148, 0.728629866, 0.745347746, 0.761609559, 0.777405353, 0.792725465, 0.807560519, 0.821901439,
	      0.835739449, 0.849066083, 0.861873185, 0.874152919, 0.885897772, 0.897100557, 0.907754419, 0.91785284,
	      0.927389639, 0.936358983, 0.944755382, 0.952573698, 0.959809149, 0.966457306, 0.972514103, 0.977975832,
	      0.982839151, 0.987101086, 0.990759028, 0.993810738, 0.996254351, 0.99808837, 0.999311673, 0.999923511,
	      0.999923511, 0.999311673, 0.99808837, 0.996254351, 0.993810738, 0.990759028, 0.987101086, 0.982839151,
	      0.977975832, 0.972514103, 0.966457306, 0.959809149, 0.952573698, 0.944755382, 0.936358983, 0.927389639,
	      0.91785284, 0.907754419, 0.897100557, 0.885897772, 0.874152919, 0.861873185, 0.849066083, 0.835739449,
	      0.821901439, 0.807560519, 0.792725465, 0.777405353, 0.761609559, 0.745347746, 0.728629866, 0.711466148,
	      0.693867094, 0.675843473, 0.657406313, 0.638566896, 0.619336749, 0.599727639, 0.579751564, 0.559420747,
	      0.53874763, 0.51774486, 0.49642529, 0.474801964, 0.452888114, 0.430697148, 0.408242645, 0.385538344,
	      0.362598137, 0.339436063, 0.316066292, 0.292503125, 0.268760979, 0.244854382, 0.220797963, 0.196606441,
	      0.172294617, 0.14787737, 0.123369638, 0.098786418, 0.074142753, 0.04945372, 0.024734427, 0.00000000};
*/
const float32_t SQRT_von_Hann_256[256] = {0,0.01231966,0.024637449,0.036951499,0.049259941,0.061560906,0.073852527,0.086132939,0.098400278,0.110652682,0.122888291,0.135105247,0.147301698,0.159475791,0.171625679,0.183749518,0.195845467,0.207911691,0.219946358,0.231947641,0.24391372,0.255842778,0.267733003,0.279582593,0.291389747,0.303152674,0.314869589,0.326538713,0.338158275,0.349726511,0.361241666,0.372701992,0.384105749,0.395451207,0.406736643,0.417960345,0.429120609,0.440215741,0.451244057,0.462203884,0.473093557,0.483911424,0.494655843,0.505325184,0.515917826,0.526432163,0.536866598,0.547219547,0.557489439,0.567674716,0.577773831,0.587785252,0.597707459,0.607538946,0.617278221,0.626923806,0.636474236,0.645928062,0.65528385,0.664540179,0.673695644,0.682748855,0.691698439,0.700543038,0.709281308,0.717911923,0.726433574,0.734844967,0.743144825,0.75133189,0.759404917,0.767362681,0.775203976,0.78292761,0.790532412,0.798017227,0.805380919,0.812622371,0.819740483,0.826734175,0.833602385,0.840344072,0.846958211,0.853443799,0.859799851,0.866025404,0.872119511,0.878081248,0.88390971,0.889604013,0.895163291,0.900586702,0.905873422,0.911022649,0.916033601,0.920905518,0.92563766,0.930229309,0.934679767,0.938988361,0.943154434,0.947177357,0.951056516,0.954791325,0.958381215,0.961825643,0.965124085,0.968276041,0.971281032,0.974138602,0.976848318,0.979409768,0.981822563,0.984086337,0.986200747,0.988165472,0.989980213,0.991644696,0.993158666,0.994521895,0.995734176,0.996795325,0.99770518,0.998463604,0.999070481,0.99952572,0.99982925,0.999981027,0.999981027,0.99982925,0.99952572,0.999070481,0.998463604,0.99770518,0.996795325,0.995734176,0.994521895,0.993158666,0.991644696,0.989980213,0.988165472,0.986200747,0.984086337,0.981822563,0.979409768,0.976848318,0.974138602,0.971281032,0.968276041,0.965124085,0.961825643,0.958381215,0.954791325,0.951056516,0.947177357,0.943154434,0.938988361,0.934679767,0.930229309,0.92563766,0.920905518,0.916033601,0.911022649,0.905873422,0.900586702,0.895163291,0.889604013,0.88390971,0.878081248,0.872119511,0.866025404,0.859799851,0.853443799,0.846958211,0.840344072,0.833602385,0.826734175,0.819740483,0.812622371,0.805380919,0.798017227,0.790532412,0.78292761,0.775203976,0.767362681,0.759404917,0.75133189,0.743144825,0.734844967,0.726433574,0.717911923,0.709281308,0.700543038,0.691698439,0.682748855,0.673695644,0.664540179,0.65528385,0.645928062,0.636474236,0.626923806,0.617278221,0.607538946,0.597707459,0.587785252,0.577773831,0.567674716,0.557489439,0.547219547,0.536866598,0.526432163,0.515917826,0.505325184,0.494655843,0.483911424,0.473093557,0.462203884,0.451244057,0.440215741,0.429120609,0.417960345,0.406736643,0.395451207,0.384105749,0.372701992,0.361241666,0.349726511,0.338158275,0.326538713,0.314869589,0.303152674,0.291389747,0.279582593,0.267733003,0.255842778,0.24391372,0.231947641,0.219946358,0.207911691,0.195845467,0.183749518,0.171625679,0.159475791,0.147301698,0.135105247,0.122888291,0.110652682,0.098400278,0.086132939,0.073852527,0.061560906,0.049259941,0.036951499,0.024637449,0.01231966,0};

void NR_Init()
{
    nr_params.alpha = 0.94; // spectral noise reduction
    nr_params.beta = 0.96;
    nr_params.enable = false;
    nr_params.NR_FFT_L = 256;
    nr_params.NR_FFT_LOOP_NO = 1;

    nr_params.first_time = 1;
    nr_params.NR_decimation_enable = true;
    nr_params.fft_256_enable = true;
    ts.special_functions_enabled = 0;
    NR2.width = 4;
    NR2.power_threshold = 0.40;
    NR2.asnr = 30;

    // TODO: Decide to through out these unused parameters
    //  nr_params.gain_smooth_enable = false;
    //  nr_params.gain_smooth_alpha = 0.25;
    //  nr_params.long_tone_enable = false;
    //  nr_params.long_tone_alpha = 0.999;
    //  nr_params.long_tone_thresh = 10000;
    //  nr_params.long_tone_reset = true;
    //  nr_params.vad_thresh = 4.0;
    //  nr_params.vad_delay = 7;
}

#if 0
// biquad IIR filter with a maximum of four notch filters
// pre-filled with passthrough coefficients
static arm_biquad_casd_df1_inst_f32 NR_notch_biquad =
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
};

static const float32_t biquad_passthrough[] = { 1, 0, 0, 0, 0 };

static float32_t NR_notch_coeffs[5];


void AudioNR_SetBiquadCoeffs(float32_t* coeffsTo,const float32_t* coeffsFrom, float scaling)
{
    coeffsTo[0] = coeffsFrom[0]/scaling;
    coeffsTo[1] = coeffsFrom[1]/scaling;
    coeffsTo[2] = coeffsFrom[2]/scaling;
    coeffsTo[3] = coeffsFrom[3]/scaling;
    coeffsTo[4] = coeffsFrom[4]/scaling;
}

void AudioNr_CalculateAutoNotch(float32_t coeffs[6], uint8_t notch1_bin, bool notch1_active, float32_t BW, float32_t FS)
{
    // formula taken from:
	// DSP Audio-EQ-cookbook for generating the coeffs of the filters on the fly
    // www.musicdsp.org/files/Audio-EQ-Cookbook.txt  [by Robert Bristow-Johnson]
	float32_t bin_BW = FS / nr_params.NR_FFT_L;
	float32_t f0 = ((float32_t)notch1_bin + 0.5) * bin_BW; // where its happening, man ;-) centre f of the bin
    float32_t Q = 10; // larger Q gives narrower notch
    float32_t w0 = 6.28318530717958 * f0 / FS;
    float32_t alpha = sinf(w0) / (2 * Q);

//    float32_t alpha = sinf(w0)*sinh( 0.3465735902799726 * BW * w0/sinf(w0) );

    coeffs[0] = 1;
    coeffs[1] = - 2 * cosf(w0);
    coeffs[2] = 1;
    coeffs[3] = 1 + alpha;
    coeffs[4] = 2 * cosf(w0); // already negated!
    coeffs[5] = alpha - 1; // already negated!
}

void AudioNr_ActivateAutoNotch(uint8_t notch1_bin, bool notch1_active)
{
	if(notch1_active)
	{	// set coeffs to new notch frequency
		AudioNr_CalculateAutoNotch(NR_notch_coeffs, notch1_bin, notch1_active, 100, 12000);
		AudioNR_SetBiquadCoeffs(&NR_notch_biquad.pCoeffs[0], NR_notch_coeffs, NR_notch_coeffs[3]); // first biquad
	}
	else

	{	// set coeffs to passthrough = NO notch
		AudioNR_SetBiquadCoeffs(&NR_notch_biquad.pCoeffs[0], biquad_passthrough, 1.0); // first biquad --> passthrough coeffs
	}
	// second biquad
//	AudioNR_SetBiquadCoeffs(&NR_notch_biquad.pCoeffs[5], NR_notch_coeffs, NR_notch_coeffs[0],1.0); // second biquad
}
#endif

int NR_in_buffer_peek(NR_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_in_head != NR_in_tail)
    {
        NR_Buffer* c = NR_in_buffers[NR_in_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int NR_in_buffer_remove(NR_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_in_head != NR_in_tail)
    {
        NR_Buffer* c = NR_in_buffers[NR_in_tail];
        NR_in_tail = (NR_in_tail + 1) % NR_BUFFER_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int NR_in_buffer_add(NR_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (NR_in_head + 1) % NR_BUFFER_FIFO_SIZE;

    if (next_head != NR_in_tail)
    {
        /* there is room */
        NR_in_buffers[NR_in_head] = c;
        NR_in_head = next_head;
        ret ++;
    }
    return ret;
}

void NR_in_buffer_reset()
{
    NR_in_tail = NR_in_head;
}

int8_t NR_in_has_data()
{
    int32_t len = NR_in_head - NR_in_tail;
    return len < 0?len+NR_BUFFER_FIFO_SIZE:len;
}

int32_t NR_in_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return NR_BUFFER_FIFO_SIZE - 1 - NR_in_has_data();
}


//*********Out Buffer handling

int NR_out_buffer_peek(NR_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_out_head != NR_out_tail)
    {
        NR_Buffer* c = NR_out_buffers[NR_out_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int NR_out_buffer_remove(NR_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_out_head != NR_out_tail)
    {
        NR_Buffer* c = NR_out_buffers[NR_out_tail];
        NR_out_tail = (NR_out_tail + 1) % NR_BUFFER_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int NR_out_buffer_add(NR_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (NR_out_head + 1) % NR_BUFFER_FIFO_SIZE;

    if (next_head != NR_out_tail)
    {
        /* there is room */
        NR_out_buffers[NR_out_head] = c;
        NR_out_head = next_head;
        ret ++;
    }
    return ret;
}

void NR_out_buffer_reset()
{
    NR_out_tail = NR_out_head;
}

int8_t NR_out_has_data()
{
    int32_t len = NR_out_head - NR_out_tail;
    return len < 0?len+NR_BUFFER_FIFO_SIZE:len;
}

int32_t NR_out_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return NR_BUFFER_FIFO_SIZE - 1 - NR_out_has_data();
}


void AudioNr_Prepare()
{
    NR.current_buffer_idx = 0;
    NR.was_here = false;
    memset(&mmb.nr_audio_buff[0].samples[0], 0, NR_BUFFER_NUM * sizeof(mmb.nr_audio_buff[0]));
}

/**
 * External "interface" of the noise reduction, handles the buffer communication and runs the actual processing algorithm
 * Runs for a significant time (couple of milliseconds on a STM32F4) so must be run in an interruptible context.
 * All "communication" with the outside world is through parameter and the input and output buffers.
 */
void AudioNr_HandleNoiseReduction()
{

    if (NR.was_here == false)
    {
        NR.was_here = true;
        NR.current_buffer_idx = 0;
        NR_in_buffer_reset();
        NR_out_buffer_reset();
    }

    if ( NR_in_has_data() && NR_out_has_room())
    {   // audio data is ready to be processed

        NR.current_buffer_idx %= NR_BUFFER_NUM;

        NR_Buffer* input_buf = NULL;
        NR_in_buffer_remove(&input_buf); //&input_buffer points to the current valid audio data

        // inside here do all the necessary noise reduction stuff!!!!!
        // here are the current input samples:  input_buf->samples
        // NR_output samples have to be placed here: fdv_iq_buff[NR.current_buffer_idx].samples
        // but starting at an offset of NR_FFT_SIZE as we are using the same buffer for in and out
        // here is the only place where we are referring to fdv_iq... as this is the name of the used freedv buffer

        //profileTimedEventStart(ProfileTP8);

        AudioNr_RunNoiseReduction(&input_buf->samples[0].real,&mmb.nr_audio_buff[NR.current_buffer_idx].samples[NR_FFT_SIZE].real);

        //profileTimedEventStop(ProfileTP8);

        NR_out_buffer_add(&mmb.nr_audio_buff[NR.current_buffer_idx]);
        NR.current_buffer_idx++;

    }

}

/**
 * This is an internal function doing the actual noise reduction
 * @param inputsamples buffer with input samples
 * @param outputsamples buffer with processed output samples
 */
static void AudioNr_RunNoiseReduction(float32_t* inputsamples, float32_t* outputsamples )
{

    float32_t* Energy=0;

    if(is_dsp_nb_active())
    {
        alt_noise_blanking(inputsamples,NR_FFT_SIZE,Energy);
    }

    //    if((ts.dsp_active & DSP_NR_ENABLE) || (ts.dsp_active & DSP_NOTCH_ENABLE))
    if(is_dsp_nr())
    {
		profileTimedEventStart(ProfileTP8);

		/*	// spectral_noise_reduction_2(inputsamples);
		if (nr_params.mode == 0)
		  spectral_noise_reduction(inputsamples);
		else
		  if (nr_params.mode == 1)
			spectral_noise_reduction_2(inputsamples);
		  else
			  */

		spectral_noise_reduction_3(inputsamples);

		profileTimedEventStop(ProfileTP8);
    }

    for (int k=0; k < NR_FFT_SIZE;  k++)
    {
        outputsamples[k] = inputsamples[k];
    }

}

// debugging switches
//#define OLD_LONG_TONE_DETECTION
//#define NR_NOTCHTEST
//#define NR_FFT_256 // if you change this, also changnr_params.NR_FFT_LFFT_L in audio_nr.h




// window switches
// choose exactly ONE window
//#define NR_WINDOW_HANN
//#define NR_WINDOW_SIN2
//#define NR_WINDOW_SIN4
//#define NR_WINDOW_HANN_CONST


#if 0
void spectral_noise_reduction (float* in_buffer)
{

//  arm_rfft_fast_instance_f32 fftInstance;


////////////////////////////////////////////////////////////////////////////////////////
#ifdef NR_NOTCHTEST
	// Result of the test: I cannot find a combination of
	// bin gains, where the distortion is away
	// the brizzling sound is always there
	// more pronounced, when gain combination along the bins is steeper


	// bin width is 12000ksps / 128 bins = 93.75Hz
	  // bin_c = 11 is a notch centred at 10 * 93.75Hz + 93.75Hz / 2 = 984.38 Hz
#ifdef NR_FFT_256
	const uint8_t bin_c = 22; // centre_bin to be notched
#else
	const uint8_t bin_c = 11; // centre_bin to be notched
#endif
	  const uint8_t bin_p1 = bin_c + 1;
	  const uint8_t bin_p2 = bin_c + 2;
	  const uint8_t bin_p3 = bin_c + 3;
	  const uint8_t bin_p4 = bin_c + 4;
	  const uint8_t bin_m1 = bin_c - 1;
	  const uint8_t bin_m2 = bin_c - 2;
	  const uint8_t bin_m3 = bin_c - 3;
	  const uint8_t bin_m4 = bin_c - 4;
	  const float32_t bin1_att = 0.0; // -24dB
	  const float32_t bin2_att = 0.15; // -12dB
	  const float32_t bin3_att = 0.3; // -6.0dB
	  const float32_t bin4_att = 0.55; // -2.5dB
	  const float32_t bin5_att = 0.7; // -2.5dB

//	  const float32_t bin1_att = 0.05; // -26dB
//	  const float32_t bin2_att = 0.15; // -16.5dB
//	  const float32_t bin3_att = 0.45; // -10.5dB
	  // linear gains mean logarithmic bin powers
	  // the best
//	  const float32_t bin1_att = 0.001; // -24dB
//	  const float32_t bin2_att = 0.25; // -12dB
//	  const float32_t bin3_att = 0.5; // -6.0dB
//	  const float32_t bin4_att = 0.75; // -2.5dB
	  // one of the best combinations so far
//	  const float32_t bin1_att = 0.01; // -40dB
//	  const float32_t bin2_att = 0.25; // dB
//	  const float32_t bin3_att = 0.5; // dB
//	  const float32_t bin4_att = 0.75; // dB
	  // GOOD!
//	  const float32_t bin1_att = 0.1; // dB
//	  const float32_t bin2_att = 0.2; // dB
//	  const float32_t bin3_att = 0.3; // dB
//	  const float32_t bin4_att = 0.4; // dB
#endif
////////////////////////////////////////////////////////////////////////////////////////

// Frank DD4WH & Michael DL2FW, November 2017
// NOISE REDUCTION BASED ON SPECTRAL SUBTRACTION
// following Romanin et al. 2009 on the basis of Ephraim & Malah 1984 and Hu et al. 2001
// detailed technical description of the implemented algorithm
// can be found in our WIKI
// https://github.com/df8oe/UHSDR/wiki/Noise-reduction
//
// half-overlapping input buffers (= overlap 50%)
// Hann window on 128 samples
// FFT128 - inverse FFT128
// overlap-add

/*	  const float32_t v_hann_window[128]= {0 , 0.0006117919, 0.0024456704, 0.0054971478, 0.0097587564 ,0.0152200676, 0.0218677165, 0.0296854352, 0.0386540925,
	  0.0487517405, 0.0599536686, 0.0722324638 ,0.0855580779, 0.0998979008, 0.1152168405, 0.1314774092, 0.1486398144,
	  0.1666620568 ,0.1855000331, 0.2051076434, 0.2254369048, 0.2464380681, 0.2680597399, 0.2902490084, 0.3129515727,
	  0.3361118759, 0.3596732407, 0.3835780087, 0.4077676808, 0.4321830608, 0.4567644003, 0.4814515445, 0.5061840798,
	  0.5309014817, 0.5555432625, 0.5800491196, 0.6043590832, 0.6284136625, 0.6521539921, 0.6755219754, 0.698460427,
	  0.7209132127, 0.7428253867, 0.7641433263, 0.7848148629, 0.8047894099, 0.8240180861, 0.8424538357, 0.8600515434,
	  0.8767681446, 0.8925627311, 0.9073966507, 0.9212336025, 0.9340397251, 0.9457836798, 0.956436727, 0.9659727972,
	  0.9743685538, 0.981603451, 0.987659784, 0.9925227317, 0.9961803937, 0.9986238192, 0.9998470286, 0.9998470286,
	  0.9986238192, 0.9961803937, 0.9925227317, 0.987659784, 0.981603451, 0.9743685538, 0.9659727972, 0.956436727,
	  0.9457836798, 0.9340397251, 0.9212336025, 0.9073966507, 0.8925627311, 0.8767681446, 0.8600515434, 0.8424538357,
	  0.8240180861, 0.8047894099, 0.7848148629, 0.7641433263, 0.7428253867, 0.7209132127, 0.698460427, 0.6755219754,
	  0.6521539921, 0.6284136625, 0.6043590832, 0.5800491196, 0.5555432625, 0.5309014817, 0.5061840798, 0.4814515445,
	  0.4567644003, 0.4321830608, 0.4077676808, 0.3835780087, 0.3596732407, 0.3361118759, 0.3129515727, 0.2902490084,
	  0.2680597399, 0.2464380681, 0.2254369048, 0.2051076434, 0.1855000331, 0.1666620568, 0.1486398144, 0.1314774092,
	  0.1152168405, 0.0998979008, 0.0855580779, 0.0722324638, 0.0599536686, 0.0487517405, 0.0386540925, 0.0296854352,
	  0.0218677165, 0.0152200676, 0.0097587564, 0.0054971478, 0.0024456704, 0.0006117919, 0};
*/
		float32_t NR_sample_rate = 12000.0;
		if(nr_params.NR_decimation_enable)
		{
			NR_sample_rate = 6000.0;
		}
		uint8_t zero_cross_count=0;
bool  VAD_ZCR, VAD_EN, VAD_E_Z;
float32_t VAD_E,VAD_energy_ratio;
static uint8_t NR_init_counter = 0;
uint8_t VAD_low=0;
uint8_t VAD_high=63;
float32_t NR_temp_sum = 0.0;
float32_t width = FilterInfo[ts.filters_p->id].width;
float32_t offset = ts.filters_p->offset;
float32_t lf_freq = (offset - width/2) / (NR_sample_rate / nr_params.NR_FFT_L); // bin BW is 93.75Hz [12000Hz / 128 bins]
float32_t uf_freq = (offset + width/2) / (NR_sample_rate / nr_params.NR_FFT_L);

    if(nr_params.first_time == 1)
    { // TODO: properly initialize all the variables
        for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
        {
          if (nr_params.mode==0)
           {
            NR.last_sample_buffer_L[bindx] = 20.0;
            NR.Hk_old[bindx] = 0.1; // old gain or xu in development mode
            NR.Nest[bindx][0] = 50000.0;
            NR.Nest[bindx][1] = 40000.0;
            NR2.X[bindx][1] = 15000.0;
            NR.SNR_post[bindx] = 2.0;
            NR.SNR_prio[bindx] = 1.0;

           }
          else  //development mode
            {
              NR.last_sample_buffer_L[bindx] = 0.0;
              NR2.Hk[bindx] = 1.0;
			//xu[bindx] = 1.0;  //has to be replaced by other variable
              NR2.Hk_old[bindx] = 1.0; // old gain or xu in development mode
	      NR.Nest[bindx][0] = 0.0;
	      NR.Nest[bindx][1] = 1.0;
	      //NR2.X[bindx][1] = 0.0;
	      //NR.SNR_post[bindx] = 1.0;
	      //NR.SNR_prio[bindx] = 1.0;

            }
        }
	nr_params.first_time = 2; // we need to do some more a bit later down
    }


    if(nr_params.long_tone_reset)
    {
        for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
        {
        	NR2.long_tone_gain[bindx] = 1.0;
        	NR2.long_tone_counter[bindx] = 10;
        }
    	nr_params.long_tone_reset = false;
    	NR2.notch_change = false;
    	NR2.notch1_bin = 10; // frequency bin where notch filter 1 has to work
    	NR2.max_bin = 10; // holds the bin number of the strongest persistent tone during tone detection
    	NR2.long_tone_max = 100.0; // power value of the strongest persistent tone, used for max search
    	NR2.notch1_active = false; // is notch1 active?
    	NR2.notch2_active = false; // is notch21 active?
    	NR2.notch3_active = false; // is notch3 active?
    	NR2.notch4_active = false; // is notch4 active?
    }

    for(int k = 0; k < nr_params.NR_FFT_LOOP_NO; k++)
    {
    // NR_FFT_buffer is 256 floats big
    // interleaved r, i, r, i . . .
    // fill first half of FFT_buffer with last events audio samples
          for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
          {
            NR.FFT_buffer[i * 2] = NR.last_sample_buffer_L[i]; // real
            NR.FFT_buffer[i * 2 + 1] = 0.0; // imaginary
          }
    // copy recent samples to last_sample_buffer for next time!
          for(int i = 0; i < nr_params.NR_FFT_L  / 2; i++)
          {
             NR.last_sample_buffer_L [i] = in_buffer[i + k * (nr_params.NR_FFT_L / 2)];
          }
    // now fill recent audio samples into second half of FFT_buffer
          for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
          {
              NR.FFT_buffer[nr_params.NR_FFT_L + i * 2] = in_buffer[i+ k * (nr_params.NR_FFT_L / 2)]; // real
              NR.FFT_buffer[nr_params.NR_FFT_L + i * 2 + 1] = 0.0;
          }
    /////////////////////////////////7
    // WINDOWING
                if(nr_params.fft_256_enable)
                {
                    arm_cfft_f32(&arm_cfft_sR_f32_len256, NR.FFT_buffer, 0, 1);
              	  for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
                    {
              	  	  NR.FFT_buffer[idx * 2] *= SQRT_von_Hann_256[idx];
                    }
                }
                else
                {
                    arm_cfft_f32(&arm_cfft_sR_f32_len128, NR.FFT_buffer, 0, 1);
                    for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
                    {
                  	  NR.FFT_buffer[idx * 2] *= SQRT_van_hann[idx];
                    }
                }

    #ifdef NR_WINDOW_HANN
    // perform windowing on 128 real samples in the NR_FFT_buffer
          for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
          {     // Hann window
             float32_t temp_sample = 0.5 * (float32_t)(1.0 - (cosf(PI* 2.0 * (float32_t)idx / (float32_t)((nr_params.NR_FFT_L) - 1))));
             NR.FFT_buffer[idx * 2] *= temp_sample;
          }
    #endif
	#ifdef NR_WINDOW_SIN2
// perform windowing on 128 real samples in the NR_FFT_buffer
      for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
      {
          // SIN^2 window
                    float32_t SINF = (sinf(PI * (float32_t)idx / (float32_t)(nr_params.NR_FFT_L - 1)));
                    SINF = (SINF * SINF);
                    NR.FFT_buffer[idx * 2] *= SINF;
      }
	#endif
#ifdef NR_WINDOW_SIN4
// perform windowing on 128 real samples in the NR_FFT_buffer
  for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
  {
      // SIN^2 window
                float32_t SINF = (sinf(PI * (float32_t)idx / (float32_t)(nr_params.NR_FFT_L - 1)));
                SINF *= SINF;
                SINF *= SINF;
                NR.FFT_buffer[idx * 2] *= SINF;
  }
#endif

 #ifndef NR_NOTCHTEST
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
                        // this is magnitude for the current frame
                  if (nr_params.mode==0)
                    NR2.X[bindx][0] = sqrtf(NR.FFT_buffer[bindx * 2] * NR.FFT_buffer[bindx * 2] + NR.FFT_buffer[bindx * 2 + 1] * NR.FFT_buffer[bindx * 2 + 1]);
                  else   //here we need only the squared magnitude
                    NR2.X[bindx][0] = (NR.FFT_buffer[bindx * 2] * NR.FFT_buffer[bindx * 2] + NR.FFT_buffer[bindx * 2 + 1] * NR.FFT_buffer[bindx * 2 + 1]);
                    }

   if(nr_params.first_time == 2)
      { // TODO: properly initialize all the variables
		if (nr_params.mode==1)
		{
		 for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                  {
                	  NR.Nest[bindx][0] = NR.Nest[bindx][0] + 0.05* NR2.X[bindx][0];// we do it 20 times to average over 20 frames for app. 100ms only on NR_on/bandswitch/modeswitch,...
                  }
		 NR_init_counter++;
		 if (NR_init_counter > 19)//average over 20 frames for app. 100ms
		     {
			  NR_init_counter = 0;
			  nr_params.first_time = 3;  // now we did all the necessary initialization to actually start the noise reduction
		     }
		}
		else nr_params.first_time = 3;

      }
   if (nr_params.first_time == 3)
     {

              if((ts.dsp_active & DSP_NOTCH_ENABLE))
              {
// detection of long tones
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
            	  	  	NR2.long_tone[bindx][0] = (nr_params.long_tone_alpha) * NR2.long_tone[bindx][1] + (1.0 - nr_params.long_tone_alpha) * NR2.X[bindx][0]; //
            	  	  	NR2.long_tone[bindx][1] = NR2.long_tone[bindx][0];
                    }
              }

      if (nr_params.mode == 1) // in development mode we start here with the calculation of the SNRs
	{
	        // 3    calculate gamma (SNRpost)
	  	//      and also xi (SNRprio)
	  	  for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
	             {
	               NR.SNR_post[bindx] = fmax(fmin(NR2.X[bindx][0] / NR.Nest[bindx][0],1000.0),0.001); // limited to +30 /-30 dB, might be still too much of reduction, let's try it?

	               NR.SNR_prio[bindx] = fmax(nr_params.alpha * NR.Hk_old[bindx] + (1.0 - nr_params.alpha)*fmax(NR.SNR_post[bindx]-1.0,0.0),0.0);
	             }

	}


              // 2b.) voice activity detector
              // we restrict the calculation of the VAD to the bins in the filter bandwidth
              //    float32_t NR_temp_sum = 0.0;
              //    float32_t width = FilterInfo[ts.filters_p->id].width;
              //    float32_t offset = ts.filters_p->offset;

                  if (offset == 0)
                  {
                      offset = width/2;
                  }

                 // float32_t lf_freq = (offset - width/2) / (12000 / NR_FFT_L); // bin BW is 93.75Hz [12000Hz / 128 bins]
                 // float32_t uf_freq = (offset + width/2) / (12000 / NR_FFT_L);
                  VAD_low = (int)lf_freq;
                  VAD_high = (int)uf_freq;
                  if(VAD_low == VAD_high)
                  {
                	  VAD_high++;
                  }
                  if(VAD_low < 1)
                  {
                	  VAD_low = 1;
                  }
                  else
                	  if(VAD_low > nr_params.NR_FFT_L / 2 - 2)
                	  {
                		  VAD_low = nr_params.NR_FFT_L / 2 - 2;
                	  }
                  if(VAD_high < 1)
                  {
                	  VAD_high = 1;
                  }
                  else
                	  if(VAD_high > nr_params.NR_FFT_L / 2)
                	  {
                		  VAD_high = nr_params.NR_FFT_L / 2;
                	  }

// ******* alternative VAD trials

if (nr_params.mode == 0) //mode "0" is the older version
  {

	      zero_cross_count=0;
	      for (int i = 1; i < nr_params.NR_FFT_L/2; i++)
	      {
			  if ((in_buffer[i + k * (nr_params.NR_FFT_L / 2)] * in_buffer[ i-1 + k * (nr_params.NR_FFT_L / 2)]) < 0)
				{
				  zero_cross_count++;  //if product of current sample with last sample is negative, we had a zero crossing
				}
	      }	     // # of zero crossing tend to be higher at only noise containing frames

	      if (zero_cross_count > 10)
	      {
			  VAD_ZCR = false;
			//Board_RedLed(LED_STATE_OFF);
	      }
	      else
	      {
	    	  VAD_ZCR = true;
	    	  //Board_RedLed(LED_STATE_ON);
	      }

	      VAD_E = 0.0;

	      for(int bindx = VAD_low; bindx < VAD_high; bindx++)
		  {
	    	  if (nr_params.mode == 0) //if mode=1, we need to take the squareroot to have the same performance - will later e fixed
		   VAD_E = VAD_E + NR2.X[bindx][0];
	    	  else
	    	   VAD_E = VAD_E + sqrtf(NR2.X[bindx][0]);
		  }

	      VAD_energy_ratio = VAD_E / (VAD_high-VAD_low);

	      if (VAD_energy_ratio > (350 * nr_params.vad_thresh)) //nr_params.vad_thresh is per default 1000/1000!!!
    	      {
	      		  VAD_EN=true;
	      		//Board_RedLed(LED_STATE_OFF);

	      }
	      else
	      {
			  VAD_EN=false;
			  //Board_RedLed(LED_STATE_ON);
	      }

	      if (VAD_EN && VAD_ZCR)
	      {
			  VAD_E_Z=true;
			 //Board_RedLed(LED_STATE_ON);

	      }
	      else
	      {
			  VAD_E_Z=false;
			  //Board_RedLed(LED_STATE_OFF);
	      }


// ******* end of alternative VAD trials
	float32_t NR_VAD_temp;

                  for(int bindx = VAD_low; bindx < VAD_high; bindx++) // try 128:
                  {
                      float32_t D_squared = NR.Nest[bindx][0] * NR.Nest[bindx][0];
//                      NR_temp_sum += (NR_X[bindx][0]/ (D_squared) ) - logf((NR_X[bindx][0] / (D_squared) )) - 1.0; // unpredictable behaviour
//                      NR_temp_sum += (NR_X[bindx][0] * NR_X[bindx][0]/ (D_squared) ) - logf((NR_X[bindx][0] * NR_X[bindx][0] / (D_squared) )) - 1.0; //nice behaviour
                      NR_temp_sum += (NR2.X[bindx][0] * NR2.X[bindx][0]/ (D_squared) ); // try without log
                  }
                  NR.VAD = NR_temp_sum / (VAD_high - VAD_low);
                  //float32_t NR_VAD_temp;
                  if(NR2.VAD_type == 0)
                	  {
                	  	  NR_VAD_temp = NR.VAD;
                	  }
                  else
                  if(NR2.VAD_type == 1)
                  {
                	  NR_VAD_temp = 10.0 * NR.VAD_Esch;
                  }
                  else
                  if(NR2.VAD_type == 2)

                  {
                    if (VAD_E_Z == true)
                    {
                    	NR_VAD_temp = 1000000;// just very high, because we already have a boolean decision
                    }
                    else
                    {
                    	NR_VAD_temp = 0;
                    }
                  }


         if((NR_VAD_temp < nr_params.vad_thresh) || nr_params.first_time == 2)
                      { // VAD has detected NOISE
							  // noise estimation with exponential averager
							 NR2.VAD_duration=0;
							 NR2.VAD_crash_detector = 0;

						 if (NR2.VAD_delay == 0) //update noise level after Speech is really over
						   {
							 for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
									{   // exponential averager for current noise estimate
										if (nr_params.mode == 0)
										  {
										   NR.Nest[bindx][0] = (1.0 - nr_params.beta) * NR2.X[bindx][0] + nr_params.beta * NR.Nest[bindx][1]; //
										   NR.Nest[bindx][1] = NR.Nest[bindx][0];
										  }
										else
										  {
										    NR.Nest[bindx][0] = NR.Nest[bindx][0] * nr_params.beta + (1.0 - nr_params.beta) * NR2.X[bindx][0]; //already squared!
										  }
									}

							 Board_RedLed(LED_STATE_OFF);
						   }
						 else // we wait a little until the last vowel has vanished
						   {

							 if (NR2.VAD_delay > 0) NR2.VAD_delay--;

						   }
                      }
                      else // VAD has detected speech
                	  {
                    	    NR2.VAD_crash_detector++;
                    		NR2.VAD_duration++;
                    		if (NR2.VAD_duration > 1) //a vowel should be longer than app. 20ms
                    		  {
                    		     NR2.VAD_delay = nr_params.vad_delay; // we wait 1 times app.  10ms before we start updating the noisefloor
                    		     Board_RedLed(LED_STATE_ON);
                    		  }
                	  }
                      // this helps to get the noise estimate out of a deep depression
                      // sometimes the VAD is not possible to become lower than VAD_thresh, because Nest is too small
                      // this helps avoiding this
                      if(NR2.VAD_crash_detector > 100)
                      {
							 for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
									{   // increase noise estimate
										  NR.Nest[bindx][0] = NR2.X[bindx][0] * 1.2; //
										  NR.Nest[bindx][1] = NR.Nest[bindx][0];
									}
                     }


        // 3    calculate SNRpost (n, bin[i]) = (X(n, bin[i])^2 / Nest(n, bin[i])^2) - 1 (eq. 13 of Schmitt et al. 2002)
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
                        // (Yk)^2 / Dk (eq 11, Romanin et al. 2009)
                        if(NR.Nest[bindx][0] != 0.0)
                        {   // do we have to square the noise estimate NR_Nest[bindx] or not? Schmitt says yes, Romanin says no . . .
                        	//                           NR_SNR_post[bindx] = NR_X[bindx][0] / (NR_Nest[bindx][0] * NR_Nest[bindx][0]); // audio crushed
                           NR.SNR_post[bindx] = NR2.X[bindx][0] / (NR.Nest[bindx][0]); //
                        }
                        // "half-wave rectification" of NR_SR_post_pos --> always >= 0
                        if(NR.SNR_post[bindx] >= 0.0)
                        {
                            NR.SNR_post_pos = NR.SNR_post[bindx];
                        }
                        else
                        {
                            NR.SNR_post_pos = 0.0;
                        }
        // 3    calculate SNRprio (n, bin[i]) = (1 - alpha) * Q(SNRpost(n, bin[i]) + alpha * (Hk(n - 1, bin[i]) * X(n - 1, bin[i])^2 / Nest(n, bin[i])^2 (eq. 14 of Schmitt et al. 2002, eq. 13 of Romanin et al. 2009) [Q[x] = x if x>=0, else Q[x] = 0]
        // again: do we have to square the noise estimate NR_M[bindx] or not? Schmitt says yes, Romanin says no . . .
                        if(NR.Nest[bindx][0] != 0.0)
                        {
                            NR.SNR_prio[bindx] = (1.0 - nr_params.alpha) * NR.SNR_post_pos +
//                                                 nr_params.alpha * ((NR_Hk_old[bindx] * NR_Hk_old[bindx] * NR_X[bindx][1]) / (NR_Nest[bindx][0])); // no NR effect
//                                    nr_params.alpha * ((NR_Hk_old[bindx] * NR_Hk_old[bindx] * NR_X[bindx][1] * NR_X[bindx][1]) / (NR_Nest[bindx][0])); // no NR effect
//                                    nr_params.alpha * ((NR_Hk_old[bindx] * NR_Hk_old[bindx] * NR_X[bindx][1] * NR_X[bindx][1]) / (NR_Nest[bindx][0] * NR_Nest[bindx][0])); // very strong, but strange effect
                                                 nr_params.alpha * ((NR.Hk_old[bindx] * NR.Hk_old[bindx] * NR2.X[bindx][1]) / (NR.Nest[bindx][0])); // working
                        }
        // 4    calculate vk = SNRprio(n, bin[i]) / (SNRprio(n, bin[i]) + 1) * SNRpost(n, bin[i]) (eq. 12 of Schmitt et al. 2002, eq. 9 of Romanin et al. 2009)
                        NR.vk =  NR.SNR_post[bindx] * NR.SNR_prio[bindx] / (1.0 + NR.SNR_prio[bindx]);
                       // calculate Hk
        // 5    finally calculate the weighting function for each bin: Hk(n, bin[i]) = 1 / SNRpost(n, [i]) * sqrtf(0.7212 * vk + vk * vk) (eq. 26 of Romanin et al. 2009)
                        if(NR.vk > 0.0 && NR.SNR_post[bindx] != 0.0) // prevent sqrtf of negatives
                        {
                            NR2.Hk[bindx] = 1.0 / NR.SNR_post[bindx] * sqrtf(0.7212 * NR.vk + NR.vk * NR.vk);
                        }
                        else
                        {
                            NR2.Hk[bindx] = 1.0;
                        }
                        if(!(ts.dsp_active & DSP_NR_ENABLE)) // if NR is not enabled (but notch is enabled !)
                        {
                        	NR2.Hk[bindx] = 1.0;
                        }
                        NR.Hk_old[bindx] = NR2.Hk[bindx];
                        NR2.X[bindx][1] = NR2.X[bindx][0];
                    }
  }

else  //new mode under development,  following the same equations, sligthly different implemented
  {

	// ***** Starting with the voice activity detection **************
	float32_t frame_speech=0.0;
  	  for(int bindx = VAD_low; bindx < VAD_high; bindx++) // maybe we should limit this to the signal containing bins (filtering!!)
  	     {
  	        frame_speech = frame_speech + (NR.SNR_post[bindx] * NR.SNR_prio[bindx]/(1.0 + NR.SNR_prio[bindx]))
												- logf(1.0 + NR.SNR_prio[bindx]);  // looks like we had a similar one  before, Frank?? ?

  	     }

  	  if (frame_speech < (0.15 * (VAD_high - VAD_low + 1.0)))// then we have noise!!!!

  	    {
  	    Board_RedLed(LED_STATE_OFF); //Noise!
  	    for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)  //update the noise estimate
  	      	     {
  	      	         NR.Nest[bindx][0] = nr_params.beta * NR.Nest[bindx][0] + (1.0 - nr_params.beta) * NR2.X[bindx][0];
  	      	     }
  	    }
  	  else
  	    {
  	    Board_RedLed(LED_STATE_ON);//Speech!
  	    }


  	  // 4    calculate v = SNRprio(n, bin[i]) / (SNRprio(n, bin[i]) + 1) * SNRpost(n, bin[i]) (eq. 12 of Schmitt et al. 2002, eq. 9 of Romanin et al. 2009)
     //		   and calculate the HK's

  	for(int bindx = VAD_low; bindx < VAD_high; bindx++)// maybe we should limit this to the signal containing bins (filtering!!)
  	   {
  	      float32_t v = NR.SNR_prio[bindx] * NR.SNR_post[bindx] / (1.0 + NR.SNR_prio[bindx]);

  	      NR2.Hk[bindx] = 1.0 / NR.SNR_post[bindx] * sqrtf((0.7212 * v + v * v));

  	      NR.Hk_old[bindx] = NR.SNR_post[bindx] * NR2.Hk[bindx] * NR2.Hk[bindx]; //


	      if(!(ts.dsp_active & DSP_NR_ENABLE)) // if NR is not enabled (but notch is enabled !)
	      {
		      NR2.Hk[bindx] = 1.0;
	      }

  	   }
  }



#ifdef OLD_LONG_TONE_DETECTION
              if((ts.dsp_active & DSP_NOTCH_ENABLE))
              {
// long tone attenuation = automatic notch filter
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
            	  	  	  if(NR2.long_tone[bindx][0] > (float32_t)nr_params.long_tone_thresh)
            	  	  	  {
            	  	  			  NR2.long_tone_gain[bindx] = NR2.long_tone_gain[bindx] * 0.99;

            	  	  			  if(bindx != 0)
            	  	  			  {
            	  	  				  NR2.long_tone_gain[bindx - 1] = NR2.long_tone_gain[bindx - 1] * 0.9995;
                	  	  			  if(NR2.long_tone_gain[bindx - 1] < 0.2)
                	  	  			  {
                	  	  				NR2.long_tone_gain[bindx - 1] = 0.2;
                	  	  			  }
            	  	  			  }
            	  	  			  else
            	  	  			  if(bindx != (nr_params.NR_FFT_L / 2 - 1))
            	  	  			  {
            	  	  				  NR2.long_tone_gain[bindx + 1] = NR2.long_tone_gain[bindx + 1] * 0.9995;
                	  	  			  if(NR2.long_tone_gain[bindx + 1] < 0.2)
                	  	  			  {
                	  	  				NR2.long_tone_gain[bindx + 1] = 0.2;
                	  	  			  }
            	  	  			  }
            	  	  			  if(NR2.long_tone_gain[bindx] < 0.05)
            	  	  			  {
            	  	  				NR2.long_tone_gain[bindx] = 0.05;
            	  	  			  }
            	  	  	  }
            	  	  	  else
            	  	  	  {
            	  	  		  NR2.long_tone_gain[bindx] *= 1.01;
        	  	  			  if(bindx != 0)
        	  	  			  {
        	  	  				  NR2.long_tone_gain[bindx - 1] = NR2.long_tone_gain[bindx - 1] * 1.0005;
            	  	  			  if(NR2.long_tone_gain[bindx - 1] > 1.0)
            	  	  			  {
            	  	  				NR2.long_tone_gain[bindx - 1] = 1.0;
            	  	  			  }
        	  	  			  }
        	  	  			  else
        	  	  			  if(bindx != (nr_params.NR_FFT_L / 2 - 1))
        	  	  			  {
        	  	  				  NR2.long_tone_gain[bindx + 1] = NR2.long_tone_gain[bindx + 1] * 1.0005;
            	  	  			  if(NR2.long_tone_gain[bindx + 1] > 1.0)
            	  	  			  {
            	  	  				NR2.long_tone_gain[bindx + 1] = 1.0;
            	  	  			  }
        	  	  			  }
        	  	  			  if(NR2.long_tone_gain[bindx] > 1.0)
        	  	  			  {
        	  	  				NR2.long_tone_gain[bindx] = 1.0;
        	  	  			  }
            	  	  	  }
                    }
              }
#else
              if((ts.dsp_active & DSP_NOTCH_ENABLE))
              {
            	  NR2.notch_change = false;
// long tone attenuation = automatic notch filter
// first version, only one notch implemented - finds the largest persisting signal and notches it with an IIR
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
            	  	  	  // if the (strongly time smoothed) signal in a bin exceeds the threshold,
            	  	  	  // increase the counter for that bin
						  if(NR2.long_tone[bindx][0] > (float32_t)nr_params.long_tone_thresh)
						  {
							  NR2.long_tone_counter[bindx]++;
						  }
						  // if it does not exceed the threshold, decrement its counter
						  else
						  {
							  NR2.long_tone_counter[bindx]--;
						  }
						  // care for low counter values
						  if(NR2.long_tone_counter[bindx] < 1)
						  {
							  NR2.long_tone_counter[bindx] = 0;
						  }
						  // care for high counter values
						  else if (NR2.long_tone_counter[bindx] > 200)
						  {
							  NR2.long_tone_counter[bindx] = 200;
						  }
                    }

				  // before we look for new notches, we have to care for existing notches
				  if(NR2.notch1_active == true)
				  {		// Is notch1 till notchworthy ?
					  if(NR2.long_tone_counter[NR2.notch1_bin] > 100)
					  {

					  }
					  else
					  {
						  // if a notch is no longer notchworthy, switch it off
						  // and do not switch it on again for at least one second --> set counter to 0
						  NR2.long_tone_counter[NR2.notch1_bin] = 0;
						  NR2.notch1_active = false;
						  NR2.notch_change = true;
					  }
				  }

				  // set max long tone to zero for maximum search
				  NR2.long_tone_max = 0.0;
				  NR2.max_bin = -99; // -99 is the indication for reset value

	              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                  {
						  // look for new notches
						  // if a tone persists strong for at least one second = 100 frames
						  // its a notchworthy tone
						  if(NR2.long_tone_counter[bindx] > 100)
						  {
							  if(NR2.long_tone[bindx][0] > NR2.long_tone_max)
								  // find out, if this is the loudest long tone
							  {
								  NR2.long_tone_max = NR2.long_tone[bindx][0];
								  NR2.max_bin = bindx;
							  }
						  }
                    }

	              if(NR2.max_bin != -99)
	              { // yes, we found a notchworthy bin
					  NR2.notch1_active = true;
					  NR2.long_tone_counter[NR2.max_bin] = 200; // hysteresis ! This notch will stay at least one second
					  // was this bin already notched last round?
					  if(NR2.notch1_bin != NR2.max_bin)
					  {
						  NR2.notch1_bin = NR2.max_bin;
						  NR2.notch_change = true; // indicate a change
					  }
					  else
					  {

					  }
	              }
              // this activates (and deactivates) the autonotch(es)
				  if(NR2.notch_change)
				  {
					  AudioNr_ActivateAutoNotch(NR2.notch1_bin, NR2.notch1_active);
				  }
              } // END NOTCH_ENABLE
#endif


              if(nr_params.gain_smooth_enable)
              {
// we hear considerable distortion in the end result
// this can be healed significantly by frequency smoothing the gain values
// this is a trial for smoothing among the gain values

//remark: if we smooth the gains and really modify the HK's like here, the noise reduction algorithm might directly "fight" against this!
// might be better to keep the HK's internaly and smooth a copy of the gains which are than working on the signal.

				  for(int bindx = 1; bindx < (nr_params.NR_FFT_L / 2) - 1; bindx++)
				  {
					  NR2.Hk[bindx] = nr_params.gain_smooth_alpha * NR2.Hk[bindx - 1] + (1.0 - 2.0 * nr_params.gain_smooth_alpha) * NR2.Hk[bindx] + nr_params.gain_smooth_alpha * NR2.Hk[bindx + 1];

				  }
				  NR2.Hk[0] = (1.0 - nr_params.gain_smooth_alpha) * NR2.Hk[0] + nr_params.gain_smooth_alpha * NR2.Hk[1];
				  NR2.Hk[(nr_params.NR_FFT_L / 2) - 1] = (1.0 - nr_params.gain_smooth_alpha) * NR2.Hk[(nr_params.NR_FFT_L / 2) - 1] + nr_params.gain_smooth_alpha * NR2.Hk[(nr_params.NR_FFT_L / 2) - 2];
              }

	}	//end of "if nr_params.first_time == 3"



#if 1
        // FINAL SPECTRAL WEIGHTING: Multiply current FFT results with NR_FFT_buffer for 64 bins with the 64 bin-specific gain factors
              // only do this for the bins inside the filter passband
              // if you do this for all the bins, you will get distorted audio: plopping !
              //              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++) // plopping !!!!
                for(int bindx = VAD_low; bindx < VAD_high; bindx++) // no plopping
              {
                  NR.FFT_buffer[bindx * 2] = NR.FFT_buffer [bindx * 2] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // real part
                  NR.FFT_buffer[bindx * 2 + 1] = NR.FFT_buffer [bindx * 2 + 1] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // imag part
                  NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] = NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // real part conjugate symmetric
                  NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] = NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // imag part conjugate symmetric
              }

#endif
                // this is another VAD
                // following Esh & Vary 2009
                // https://pdfs.semanticscholar.org/8fdb/fef3bac889a4f7f5ac382377fa7e1a2b8a7f.pdf

                if(NR2.VAD_type == 1)
                {
					NR_temp_sum = 0;
					float32_t NR_temp_sum2 = 0;
					for(int bindx = VAD_low; bindx < VAD_high; bindx++)
					{
						NR_temp_sum += NR2.Hk[bindx] * NR2.X[bindx][0];
						NR_temp_sum2 += NR2.X[bindx][0];
					}
					NR.VAD_Esch = NR_temp_sum / NR_temp_sum2;
                }


#endif
                /*****************************************************************
         * NOISE REDUCTION CODE ENDS HERE
         *****************************************************************/
// very interesting!
// if I leave the FFT_buffer as is and just multiply the 19 bins below with 0.1, the audio
// is distorted a little bit !
// To me, this is an indicator of a problem with windowing . . .
// OR: smooth the bin gains by frequency, because the problem could be that one bin has gain 1.0 and
// the adjacent bin has gain 0.1 --> a 20dB difference!

#if 0
  for(int bindx = 1; bindx < 20; bindx++)
  // bins 2 to 29 attenuated
  // set real values to 0.1 of their original value
  {
      NR.FFT_buffer[bindx * 2] *= 0.1;
//      NR_FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
      NR.FFT_buffer[bindx * 2 + 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
//      NR_FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
  }
#endif

#ifdef NR_NOTCHTEST  // this is a test of a smoother notch filter
	  // centre bin to be notched
  	  NR.FFT_buffer[				bin_c * 2] 		*= bin1_att; // real
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_c * 2 - 2]	*= bin1_att; // imaginary
      NR.FFT_buffer[				bin_c * 2 + 1] 	*= bin1_att; // real conjugate symmetric
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_c * 2 - 1] 	*= bin1_att; // imaginary conjugate symmetric
      // centre_bin + 1 to be notched
  	  NR.FFT_buffer[				bin_p1 * 2] 	*= bin2_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p1 * 2 - 2]	*= bin2_att;
      NR.FFT_buffer[				bin_p1 * 2 + 1] *= bin2_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p1 * 2 - 1] *= bin2_att;
      // centre_bin - 1 to be notched
  	  NR.FFT_buffer[				bin_m1 * 2] 	*= bin2_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m1 * 2 - 2]	*= bin2_att;
      NR.FFT_buffer[				bin_m1 * 2 + 1] *= bin2_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m1 * 2 - 1] *= bin2_att;
      // centre_bin + 2 to be notched
  	  NR.FFT_buffer[				bin_p2 * 2] 	*= bin3_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p2 * 2 - 2]	*= bin3_att;
      NR.FFT_buffer[				bin_p2 * 2 + 1] *= bin3_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p2 * 2 - 1] *= bin3_att;
      // centre_bin - 2 to be notched
  	  NR.FFT_buffer[				bin_m2 * 2] 	*= bin3_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m2 * 2 - 2]	*= bin3_att;
      NR.FFT_buffer[				bin_m2 * 2 + 1] *= bin3_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m2 * 2 - 1] *= bin3_att;
      // centre_bin + 3 to be notched
  	  NR.FFT_buffer[				bin_p3 * 2] 	*= bin4_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p3 * 2 - 2]	*= bin4_att;
      NR.FFT_buffer[				bin_p3 * 2 + 1] *= bin4_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p3 * 2 - 1] *= bin4_att;
      // centre_bin - 3 to be notched
  	  NR.FFT_buffer[				bin_m3 * 2] 	*= bin4_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m3 * 2 - 2]	*= bin4_att;
      NR.FFT_buffer[				bin_m3 * 2 + 1] *= bin4_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m3 * 2 - 1] *= bin4_att;
      // centre_bin + 4 to be notched
  	  NR.FFT_buffer[				bin_p4 * 2] 	*= bin5_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p4 * 2 - 2]	*= bin5_att;
      NR.FFT_buffer[				bin_p4 * 2 + 1] *= bin5_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p4 * 2 - 1] *= bin5_att;
      // centre_bin - 4 to be notched
  	  NR.FFT_buffer[				bin_m4 * 2] 	*= bin5_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m4 * 2 - 2]	*= bin5_att;
      NR.FFT_buffer[				bin_m4 * 2 + 1] *= bin5_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m4 * 2 - 1] *= bin5_att;
#endif

      // Window on exit!
            if(nr_params.fft_256_enable)
            {
                arm_cfft_f32(&arm_cfft_sR_f32_len256, NR.FFT_buffer, 1, 1);
          	  for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
                {
          	  	  NR.FFT_buffer[idx * 2] *= SQRT_von_Hann_256[idx];
                }
            }
            else
            {
                arm_cfft_f32(&arm_cfft_sR_f32_len128, NR.FFT_buffer, 1, 1);
                for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
                {
              	  NR.FFT_buffer[idx * 2] *= SQRT_van_hann[idx];
                }
            }

    // do the overlap & add
          for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
          { // take real part of first half of current iFFT result and add to 2nd half of last iFFT_result
        	  //              NR_output_audio_buffer[i + k * (nr_params.NR_FFT_L / 2)] = NR_FFT_buffer[i * 2] + NR_last_iFFT_result[i];
        	  in_buffer[i + k * (nr_params.NR_FFT_L / 2)] = NR.FFT_buffer[i * 2] + NR.last_iFFT_result[i];
// FIXME: take out scaling !
//        	  in_buffer[i + k * (nr_params.NR_FFT_L / 2)] *= 0.3;
          }
          for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
          {
              NR.last_iFFT_result[i] = NR.FFT_buffer[nr_params.NR_FFT_L + i * 2];
          }
       // end of "for" loop which repeats the FFT_iFFT_chain two times !!!
    }

    // IIR biquad notch filter with four independent notches
    // arm_biquad_cascade_df1_f32 (&NR_notch_biquad, in_buffer, in_buffer, nr_params.NR_FFT_L);
}





void spectral_noise_reduction_2 (float* in_buffer)
{

//  arm_rfft_fast_instance_f32 fftInstance;


////////////////////////////////////////////////////////////////////////////////////////

// Frank DD4WH & Michael DL2FW, November 2017
// NOISE REDUCTION BASED ON SPECTRAL SUBTRACTION
// following Romanin et al. 2009 on the basis of Ephraim & Malah 1984 and Hu et al. 2001
// detailed technical description of the implemented algorithm
// can be found in our WIKI
// https://github.com/df8oe/UHSDR/wiki/Noise-reduction
//
// half-overlapping input buffers (= overlap 50%)
// Hann window on 128 samples
// FFT128 - inverse FFT128
// overlap-add

	float32_t NR_sample_rate = 12000.0;
	if(nr_params.NR_decimation_enable)
	{
		NR_sample_rate = 6000.0;
	}

static uint8_t NR_init_counter = 0;
uint8_t VAD_low=0;
uint8_t VAD_high=63;
//float32_t NR_temp_sum = 0.0;
float32_t width = FilterInfo[ts.filters_p->id].width;
float32_t offset = ts.filters_p->offset;
float32_t lf_freq = (offset - width/2) / (NR_sample_rate / nr_params.NR_FFT_L); // bin BW is 93.75Hz [12000Hz / 128 bins]
float32_t uf_freq = (offset + width/2) / (NR_sample_rate / nr_params.NR_FFT_L);

    if(nr_params.first_time == 1)
    { // TODO: properly initialize all the variables
        for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
        {
              NR.last_sample_buffer_L[bindx] = 0.0;
              NR2.Hk[bindx] = 1.0;
			//xu[bindx] = 1.0;  //has to be replaced by other variable
              NR.Hk_old[bindx] = 1.0; // old gain or xu in development mode
	      NR.Nest[bindx][0] = 0.0;
	      NR.Nest[bindx][1] = 1.0;
        }
	nr_params.first_time = 2; // we need to do some more a bit later down
    }

    if(nr_params.long_tone_reset)
    {
        for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
        {
        	NR2.long_tone_gain[bindx] = 1.0;
        	NR2.long_tone_counter[bindx] = 10;
        }
    	nr_params.long_tone_reset = false;
    	NR2.notch_change = false;
    	NR2.notch1_bin = 10; // frequency bin where notch filter 1 has to work
    	NR2.max_bin = 10; // holds the bin number of the strongest persistent tone during tone detection
    	NR2.long_tone_max = 100.0; // power value of the strongest persistent tone, used for max search
    	NR2.notch1_active = false; // is notch1 active?
    	NR2.notch2_active = false; // is notch21 active?
    	NR2.notch3_active = false; // is notch3 active?
    	NR2.notch4_active = false; // is notch4 active?
    }

    for(int k = 0; k < nr_params.NR_FFT_LOOP_NO; k++)
    {
    // NR_FFT_buffer is 256 floats big
    // interleaved r, i, r, i . . .
    // fill first half of FFT_buffer with last events audio samples
          for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
          {
            NR.FFT_buffer[i * 2] = NR.last_sample_buffer_L[i]; // real
            NR.FFT_buffer[i * 2 + 1] = 0.0; // imaginary
          }
    // copy recent samples to last_sample_buffer for next time!
          for(int i = 0; i < nr_params.NR_FFT_L  / 2; i++)
          {
             NR.last_sample_buffer_L [i] = in_buffer[i + k * (nr_params.NR_FFT_L / 2)];
          }
    // now fill recent audio samples into second half of FFT_buffer
          for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
          {
              NR.FFT_buffer[nr_params.NR_FFT_L + i * 2] = in_buffer[i+ k * (nr_params.NR_FFT_L / 2)]; // real
              NR.FFT_buffer[nr_params.NR_FFT_L + i * 2 + 1] = 0.0;
          }



                if(nr_params.fft_256_enable)
                {
                    arm_cfft_f32(&arm_cfft_sR_f32_len256, NR.FFT_buffer, 0, 1);
              	  for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
                    {
              	  	  NR.FFT_buffer[idx * 2] *= SQRT_von_Hann_256[idx];
                    }
                }
                else
                {
                    arm_cfft_f32(&arm_cfft_sR_f32_len128, NR.FFT_buffer, 0, 1);
                    for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
                    {
                  	  NR.FFT_buffer[idx * 2] *= SQRT_van_hann[idx];
                    }
                }

#ifndef NR_NOTCHTEST
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
                        // this is magnitude for the current frame
                  if (nr_params.mode==0)
                    NR2.X[bindx][0] = sqrtf(NR.FFT_buffer[bindx * 2] * NR.FFT_buffer[bindx * 2] + NR.FFT_buffer[bindx * 2 + 1] * NR.FFT_buffer[bindx * 2 + 1]);
                  else   //here we need only the squared magnitude
                    NR2.X[bindx][0] = (NR.FFT_buffer[bindx * 2] * NR.FFT_buffer[bindx * 2] + NR.FFT_buffer[bindx * 2 + 1] * NR.FFT_buffer[bindx * 2 + 1]);
                    }

   if(nr_params.first_time == 2)
      { // TODO: properly initialize all the variables
		if (nr_params.mode==1)
		{
		 for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                  {
                	  NR.Nest[bindx][0] = NR.Nest[bindx][0] + 0.05* NR2.X[bindx][0];// we do it 20 times to average over 20 frames for app. 100ms only on NR_on/bandswitch/modeswitch,...
                  }
		 NR_init_counter++;
		 if (NR_init_counter > 19)//average over 20 frames for app. 100ms
		     {
			  NR_init_counter = 0;
			  nr_params.first_time = 3;  // now we did all the necessary initialization to actually start the noise reduction
		     }
		}
		else nr_params.first_time = 3;

      }
   if (nr_params.first_time == 3)
     {

              if((ts.dsp_active & DSP_NOTCH_ENABLE))
              {
        	    // detection of long tones
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
            	  	 NR2.long_tone[bindx][0] = (nr_params.long_tone_alpha) * NR2.long_tone[bindx][1] + (1.0 - nr_params.long_tone_alpha) * NR2.X[bindx][0]; //
            	  	 NR2.long_tone[bindx][1] = NR2.long_tone[bindx][0];
                    }
              }

	  	  for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)// 1. Step of NR - calculate the SNR's
	             {
	               NR.SNR_post[bindx] = fmax(fmin(NR2.X[bindx][0] / NR.Nest[bindx][0],1000.0),0.03); // limited to +30 /-15 dB, might be still too much of reduction, let's try it?

	               NR.SNR_prio[bindx] = fmax(nr_params.alpha * NR.Hk_old[bindx] + (1.0 - nr_params.alpha)*fmax(NR.SNR_post[bindx]-1.0,0.0),0.0);
	             }


                  if (offset == 0)
                  {
                      offset = width/2;
                  }

                  VAD_low = (int)lf_freq;
                  VAD_high = (int)uf_freq;
                  if(VAD_low == VAD_high)
                  {
                	  VAD_high++;
                  }
                  if(VAD_low < 1)
                  {
                	  VAD_low = 1;
                  }
                  else
                	  if(VAD_low > nr_params.NR_FFT_L / 2 - 2)
                	  {
                		  VAD_low = nr_params.NR_FFT_L / 2 - 2;
                	  }
                  if(VAD_high < 1)
                  {
                	  VAD_high = 1;
                  }
                  else
                	  if(VAD_high > nr_params.NR_FFT_L / 2)
                	  {
                		  VAD_high = nr_params.NR_FFT_L / 2;
                	  }

// ******* alternative VAD trials

  //new mode under development,  following the same equations, sligthly different implemented


	// ***** Starting with the voice activity detection **************
	float32_t frame_speech=0.0;
  	  for(int bindx = VAD_low; bindx < VAD_high; bindx++) // maybe we should limit this to the signal containing bins (filtering!!)
  	     {
  	        frame_speech = frame_speech + (NR.SNR_post[bindx] * NR.SNR_prio[bindx]/(1.0 + NR.SNR_prio[bindx]))
												- logf(1.0 + NR.SNR_prio[bindx]);  // looks like we had a similar one  before, Frank?? ?

  	     }

  	  if (frame_speech < (0.15 * (VAD_high - VAD_low + 1.0)))// then we have noise!!!!

  	    {
  	    NR2.VAD_duration = 0;
  	    if (NR2.VAD_delay > 0)
  	      {
  		NR2.VAD_delay--;
  	      }
  	    else
  	      {
  		Board_RedLed(LED_STATE_OFF); //Noise!
  		for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)  //update the noise estimate in dpi
  	      	     {
  	      	         NR.Nest[bindx][0] = NR.Nest[bindx][0] * nr_params.beta + (1.0 - nr_params.beta) * NR2.X[bindx][0];
  	      	     }
  	      }
  	    }
  	  else
  	    {
  	      NR2.VAD_duration++;
  	      if (NR2.VAD_duration > 1)
  		{
  		 NR2.VAD_delay=nr_params.vad_delay;


  		  Board_RedLed(LED_STATE_ON);//Speech!
  		}
  	    }

  	  // 4    calculate v = SNRprio(n, bin[i]) / (SNRprio(n, bin[i]) + 1) * SNRpost(n, bin[i]) (eq. 12 of Schmitt et al. 2002, eq. 9 of Romanin et al. 2009)
     //		   and calculate the HK's

  	for(int bindx = VAD_low; bindx < VAD_high; bindx++)// maybe we should limit this to the signal containing bins (filtering!!)
  	   {
  	      float32_t v = NR.SNR_prio[bindx] * NR.SNR_post[bindx] / (1.0 + NR.SNR_prio[bindx]);

  	      NR2.Hk[bindx] = 1.0 / NR.SNR_post[bindx] * sqrtf((0.7212 * v + v * v));

  	      NR.Hk_old[bindx] = NR.SNR_post[bindx] * NR2.Hk[bindx] * NR2.Hk[bindx]; //


	      if(!(ts.dsp_active & DSP_NR_ENABLE)) // if NR is not enabled (but notch is enabled !)
	      {
		      NR2.Hk[bindx] = 1.0;
	      }

  	   }




#ifdef OLD_LONG_TONE_DETECTION
              if((ts.dsp_active & DSP_NOTCH_ENABLE))
              {
// long tone attenuation = automatic notch filter
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
            	  	  	  if(NR2.long_tone[bindx][0] > (float32_t)nr_params.long_tone_thresh)
            	  	  	  {
            	  	  			  NR2.long_tone_gain[bindx] = NR2.long_tone_gain[bindx] * 0.99;

            	  	  			  if(bindx != 0)
            	  	  			  {
            	  	  				  NR2.long_tone_gain[bindx - 1] = NR2.long_tone_gain[bindx - 1] * 0.9995;
                	  	  			  if(NR2.long_tone_gain[bindx - 1] < 0.2)
                	  	  			  {
                	  	  				NR2.long_tone_gain[bindx - 1] = 0.2;
                	  	  			  }
            	  	  			  }
            	  	  			  else
            	  	  			  if(bindx != (nr_params.NR_FFT_L / 2 - 1))
            	  	  			  {
            	  	  				  NR2.long_tone_gain[bindx + 1] = NR2.long_tone_gain[bindx + 1] * 0.9995;
                	  	  			  if(NR2.long_tone_gain[bindx + 1] < 0.2)
                	  	  			  {
                	  	  				NR2.long_tone_gain[bindx + 1] = 0.2;
                	  	  			  }
            	  	  			  }
            	  	  			  if(NR2.long_tone_gain[bindx] < 0.05)
            	  	  			  {
            	  	  				NR2.long_tone_gain[bindx] = 0.05;
            	  	  			  }
            	  	  	  }
            	  	  	  else
            	  	  	  {
            	  	  		  NR2.long_tone_gain[bindx] *= 1.01;
        	  	  			  if(bindx != 0)
        	  	  			  {
        	  	  				  NR2.long_tone_gain[bindx - 1] = NR2.long_tone_gain[bindx - 1] * 1.0005;
            	  	  			  if(NR2.long_tone_gain[bindx - 1] > 1.0)
            	  	  			  {
            	  	  				NR2.long_tone_gain[bindx - 1] = 1.0;
            	  	  			  }
        	  	  			  }
        	  	  			  else
        	  	  			  if(bindx != (nr_params.NR_FFT_L / 2 - 1))
        	  	  			  {
        	  	  				  NR2.long_tone_gain[bindx + 1] = NR2.long_tone_gain[bindx + 1] * 1.0005;
            	  	  			  if(NR2.long_tone_gain[bindx + 1] > 1.0)
            	  	  			  {
            	  	  				NR2.long_tone_gain[bindx + 1] = 1.0;
            	  	  			  }
        	  	  			  }
        	  	  			  if(NR2.long_tone_gain[bindx] > 1.0)
        	  	  			  {
        	  	  				NR2.long_tone_gain[bindx] = 1.0;
        	  	  			  }
            	  	  	  }
                    }
              }
#else
              if((ts.dsp_active & DSP_NOTCH_ENABLE))
              {
            	  NR2.notch_change = false;
// long tone attenuation = automatic notch filter
// first version, only one notch implemented - finds the largest persisting signal and notches it with an IIR
              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                    {
            	  	  	  // if the (strongly time smoothed) signal in a bin exceeds the threshold,
            	  	  	  // increase the counter for that bin
						  if(NR2.long_tone[bindx][0] > (float32_t)nr_params.long_tone_thresh)
						  {
							  NR2.long_tone_counter[bindx]++;
						  }
						  // if it does not exceed the threshold, decrement its counter
						  else
						  {
							  NR2.long_tone_counter[bindx]--;
						  }
						  // care for low counter values
						  if(NR2.long_tone_counter[bindx] < 1)
						  {
							  NR2.long_tone_counter[bindx] = 0;
						  }
						  // care for high counter values
						  else if (NR2.long_tone_counter[bindx] > 200)
						  {
							  NR2.long_tone_counter[bindx] = 200;
						  }
                    }

				  // before we look for new notches, we have to care for existing notches
				  if(NR2.notch1_active == true)
				  {		// Is notch1 till notchworthy ?
					  if(NR2.long_tone_counter[NR2.notch1_bin] > 100)
					  {

					  }
					  else
					  {
						  // if a notch is no longer notchworthy, switch it off
						  // and do not switch it on again for at least one second --> set counter to 0
						  NR2.long_tone_counter[NR2.notch1_bin] = 0;
						  NR2.notch1_active = false;
						  NR2.notch_change = true;
					  }
				  }

				  // set max long tone to zero for maximum search
				  NR2.long_tone_max = 0.0;
				  NR2.max_bin = -99; // -99 is the indication for reset value

	              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
                  {
						  // look for new notches
						  // if a tone persists strong for at least one second = 100 frames
						  // its a notchworthy tone
						  if(NR2.long_tone_counter[bindx] > 100)
						  {
							  if(NR2.long_tone[bindx][0] > NR2.long_tone_max)
								  // find out, if this is the loudest long tone
							  {
								  NR2.long_tone_max = NR2.long_tone[bindx][0];
								  NR2.max_bin = bindx;
							  }
						  }
                    }

	              if(NR2.max_bin != -99)
	              { // yes, we found a notchworthy bin
					  NR2.notch1_active = true;
					  NR2.long_tone_counter[NR2.max_bin] = 200; // hysteresis ! This notch will stay at least one second
					  // was this bin already notched last round?
					  if(NR2.notch1_bin != NR2.max_bin)
					  {
						  NR2.notch1_bin = NR2.max_bin;
						  NR2.notch_change = true; // indicate a change
					  }
					  else
					  {

					  }
	              }
              // this activates (and deactivates) the autonotch(es)
				  if(NR2.notch_change)
				  {
					  AudioNr_ActivateAutoNotch(NR2.notch1_bin, NR2.notch1_active);
				  }
              } // END NOTCH_ENABLE
#endif


              if(nr_params.gain_smooth_enable)
              {
// we hear considerable distortion in the end result
// this can be healed significantly by frequency smoothing the gain values
// this is a trial for smoothing among the gain values

//remark: if we smooth the gains and really modify the HK's like here, the noise reduction algorithm might directly "fight" against this!
// might be better to keep the HK's internaly and smooth a copy of the gains which are than working on the signal.

				  for(int bindx = 1; bindx < (nr_params.NR_FFT_L / 2) - 1; bindx++)
				  {
					  NR2.Hk[bindx] = nr_params.gain_smooth_alpha * NR2.Hk[bindx - 1] + (1.0 - 2.0 * nr_params.gain_smooth_alpha) * NR2.Hk[bindx] + nr_params.gain_smooth_alpha * NR2.Hk[bindx + 1];

				  }
				  NR2.Hk[0] = (1.0 - nr_params.gain_smooth_alpha) * NR2.Hk[0] + nr_params.gain_smooth_alpha * NR2.Hk[1];
				  NR2.Hk[(nr_params.NR_FFT_L / 2) - 1] = (1.0 - nr_params.gain_smooth_alpha) * NR2.Hk[(nr_params.NR_FFT_L / 2) - 1] + nr_params.gain_smooth_alpha * NR2.Hk[(nr_params.NR_FFT_L / 2) - 2];
              }

	}	//end of "if nr_params.first_time == 3"



        // FINAL SPECTRAL WEIGHTING: Multiply current FFT results with NR_FFT_buffer for 64 bins with the 64 bin-specific gain factors
              // only do this for the bins inside the filter passband
              // if you do this for all the bins, you will get distorted audio: plopping !
              //              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++) // plopping !!!!
                for(int bindx = VAD_low; bindx < VAD_high; bindx++) // no plopping
              {
                  NR.FFT_buffer[bindx * 2] = NR.FFT_buffer [bindx * 2] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // real part
                  NR.FFT_buffer[bindx * 2 + 1] = NR.FFT_buffer [bindx * 2 + 1] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // imag part
                  NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] = NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // real part conjugate symmetric
                  NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] = NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // imag part conjugate symmetric
              }



#endif
                /*****************************************************************
         * NOISE REDUCTION CODE ENDS HERE
         *****************************************************************/
// very interesting!
// if I leave the FFT_buffer as is and just multiply the 19 bins below with 0.1, the audio
// is distorted a little bit !
// To me, this is an indicator of a problem with windowing . . .
// OR: smooth the bin gains by frequency, because the problem could be that one bin has gain 1.0 and
// the adjacent bin has gain 0.1 --> a 20dB difference!

#if 0
  for(int bindx = 1; bindx < 20; bindx++)
  // bins 2 to 29 attenuated
  // set real values to 0.1 of their original value
  {
      NR.FFT_buffer[bindx * 2] *= 0.1;
//      NR_FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
      NR.FFT_buffer[bindx * 2 + 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
//      NR_FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] *= 0.1; //NR_iFFT_buffer[idx] * 0.1;
  }
#endif

#ifdef NR_NOTCHTEST  // this is a test of a smoother notch filter
	  // centre bin to be notched
  	  NR.FFT_buffer[				bin_c * 2] 		*= bin1_att; // real
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_c * 2 - 2]	*= bin1_att; // imaginary
      NR.FFT_buffer[				bin_c * 2 + 1] 	*= bin1_att; // real conjugate symmetric
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_c * 2 - 1] 	*= bin1_att; // imaginary conjugate symmetric
      // centre_bin + 1 to be notched
  	  NR.FFT_buffer[				bin_p1 * 2] 	*= bin2_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p1 * 2 - 2]	*= bin2_att;
      NR.FFT_buffer[				bin_p1 * 2 + 1] *= bin2_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p1 * 2 - 1] *= bin2_att;
      // centre_bin - 1 to be notched
  	  NR.FFT_buffer[				bin_m1 * 2] 	*= bin2_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m1 * 2 - 2]	*= bin2_att;
      NR.FFT_buffer[				bin_m1 * 2 + 1] *= bin2_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m1 * 2 - 1] *= bin2_att;
      // centre_bin + 2 to be notched
  	  NR.FFT_buffer[				bin_p2 * 2] 	*= bin3_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p2 * 2 - 2]	*= bin3_att;
      NR.FFT_buffer[				bin_p2 * 2 + 1] *= bin3_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p2 * 2 - 1] *= bin3_att;
      // centre_bin - 2 to be notched
  	  NR.FFT_buffer[				bin_m2 * 2] 	*= bin3_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m2 * 2 - 2]	*= bin3_att;
      NR.FFT_buffer[				bin_m2 * 2 + 1] *= bin3_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m2 * 2 - 1] *= bin3_att;
      // centre_bin + 3 to be notched
  	  NR.FFT_buffer[				bin_p3 * 2] 	*= bin4_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p3 * 2 - 2]	*= bin4_att;
      NR.FFT_buffer[				bin_p3 * 2 + 1] *= bin4_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p3 * 2 - 1] *= bin4_att;
      // centre_bin - 3 to be notched
  	  NR.FFT_buffer[				bin_m3 * 2] 	*= bin4_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m3 * 2 - 2]	*= bin4_att;
      NR.FFT_buffer[				bin_m3 * 2 + 1] *= bin4_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m3 * 2 - 1] *= bin4_att;
      // centre_bin + 4 to be notched
  	  NR.FFT_buffer[				bin_p4 * 2] 	*= bin5_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p4 * 2 - 2]	*= bin5_att;
      NR.FFT_buffer[				bin_p4 * 2 + 1] *= bin5_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_p4 * 2 - 1] *= bin5_att;
      // centre_bin - 4 to be notched
  	  NR.FFT_buffer[				bin_m4 * 2] 	*= bin5_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m4 * 2 - 2]	*= bin5_att;
      NR.FFT_buffer[				bin_m4 * 2 + 1] *= bin5_att;
      NR.FFT_buffer[nr_params.NR_FFT_L * 2 - 	bin_m4 * 2 - 1] *= bin5_att;
#endif


      // Window on exit!
            if(nr_params.fft_256_enable)
            {
                arm_cfft_f32(&arm_cfft_sR_f32_len256, NR.FFT_buffer, 1, 1);
          	  for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
                {
          	  	  NR.FFT_buffer[idx * 2] *= SQRT_von_Hann_256[idx];
                }
            }
            else
            {
                arm_cfft_f32(&arm_cfft_sR_f32_len128, NR.FFT_buffer, 1, 1);
                for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
                {
              	  NR.FFT_buffer[idx * 2] *= SQRT_van_hann[idx];
                }
            }

    // do the overlap & add
          for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
          { // take real part of first half of current iFFT result and add to 2nd half of last iFFT_result
        	  //              NR_output_audio_buffer[i + k * (nr_params.NR_FFT_L / 2)] = NR_FFT_buffer[i * 2] + NR_last_iFFT_result[i];
        	  in_buffer[i + k * (nr_params.NR_FFT_L / 2)] = NR.FFT_buffer[i * 2] + NR.last_iFFT_result[i];
// FIXME: take out scaling !
//        	  in_buffer[i + k * (nr_params.NR_FFT_L / 2)] *= 0.3;
          }
          for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
          {
              NR.last_iFFT_result[i] = NR.FFT_buffer[nr_params.NR_FFT_L + i * 2];
          }
       // end of "for" loop which repeats the FFT_iFFT_chain two times !!!
    }

    // IIR biquad notch filter with four independent notches
   // arm_biquad_cascade_df1_f32 (&NR_notch_biquad, in_buffer, in_buffer, nr_params.NR_FFT_L);
}
#endif

void spectral_noise_reduction_3 (float* in_buffer)
{
    ////////////////////////////////////////////////////////////////////////////////////////

    // Frank DD4WH & Michael DL2FW, November 2017
    // NOISE REDUCTION BASED ON SPECTRAL SUBTRACTION
    // following Romanin et al. 2009 on the basis of Ephraim & Malah 1984 and Hu et al. 2001
    // detailed technical description of the implemented algorithm
    // can be found in our WIKI
    // https://github.com/df8oe/UHSDR/wiki/Noise-reduction
    //
    // half-overlapping input buffers (= overlap 50%)
    // Hann window on 128 or 256 samples
    // FFT128 - inverse FFT128 or FFT256 / iFFT256
    // overlap-add

    const float32_t width = FilterInfo[ts.filters_p->id].width;
    const float32_t offset = ts.filters_p->offset;

    float32_t NR_sample_rate = nr_params.NR_decimation_active? 6000.0: 12000.0;

    static uint8_t NR_init_counter = 0;
    uint8_t VAD_low=0;
    uint8_t VAD_high=63;

    float32_t lf_freq = (offset - width/2) / (NR_sample_rate / nr_params.NR_FFT_L); // bin BW is 93.75Hz [12000Hz / 128 bins]
    float32_t uf_freq = (offset + width/2) / (NR_sample_rate / nr_params.NR_FFT_L);

    //const float32_t tinc = 0.00533333; // frame time 5.3333ms
    //const float32_t tax=0.071;	// noise output smoothing time constant - absolut value in seconds
    //const float32_t tap=0.152;	// speech prob smoothing time constant  - absolut value in seconds
    const float32_t psthr=0.99;	// threshold for smoothed speech probability [0.99]
    const float32_t pnsaf=0.01;	// noise probability safety value [0.01]
    //const float32_t asnr=15; 	// active SNR in dB
    const float32_t psini=0.5;	// initial speech probability [0.5]
    const float32_t pspri=0.5;	// prior speech probability [0.5]

    // for 12ksps and FFT128
    //NR2.ax = 0.9276; 		//expf(-tinc / tax);
    //NR2.ap = 0.9655; 		//expf(-tinc / tap);

    // for 6ksps and FFT256
    NR2.ax = 0.7405;
    NR2.ap = 0.8691;
    //NR2.xih1 = 31.62; 		//powf(10, (float32_t)NR2.asnr / 10.0);
    NR2.xih1 = pow10f((float32_t)NR2.asnr / 10.0);
    NR2.xih1r = 1.0 / (1.0 + NR2.xih1) - 1.0;
    NR2.pfac= (1.0 / pspri - 1.0) * (1.0 + NR2.xih1);
    NR2.snr_prio_min = 0.001; 			//powf(10, - (float32_t)NR2.snr_prio_min_int / 10.0);  //range should be down to -30dB min
    NR2.power_threshold = (float32_t)(NR2.power_threshold_int)/100.0;
    static float32_t pslp[NR_FFT_SIZE];
    static float32_t xt[NR_FFT_SIZE];
    float32_t xtr;
    float32_t ph1y[NR_FFT_SIZE];

    //ax and ap adjustment according to FFT-size and decimation
    // for 12KS and 128'er FFT we are doing well with the above values
    // This table shows the tinc values in ms for the calculation of the coefficients ax and ap
    //
    // 	   FFT Size:  |	128		256
    //    		______|________________________
    // Samplerate:  12KS  | 5.333		10.666
    //		      |
    //		 6KS  |10.666		21.333


    /*if (nr_params.fft_256_enable && nr_params.NR_decimation_enable)
  {
    NR2.ax = 0.7405;
    NR2.ap = 0.8691;
  }
else
  if (nr_params.fft_256_enable || nr_params.NR_decimation_enable)
    {
      NR2.ax = 0.8605;
      NR2.ap = 0.9322;
    } */


    if(nr_params.first_time == 1)
    { // TODO: properly initialize all the variables

        for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
        {
            NR.last_sample_buffer_L[bindx] = 0.0;
            NR2.Hk[bindx] = 1.0;
            //xu[bindx] = 1.0;  //has to be replaced by other variable
            NR.Hk_old[bindx] = 1.0; // old gain or xu in development mode
            NR.Nest[bindx][0] = 0.0;
            NR.Nest[bindx][1] = 1.0;
            pslp[bindx] = 0.5;
            //              NR2.long_tone_gain[bindx] = 1.0;
        }
        nr_params.first_time = 2; // we need to do some more a bit later down
    }

    for(int k = 0; k < nr_params.NR_FFT_LOOP_NO; k++)
    {
        // NR_FFT_buffer is 256 floats big
        // interleaved r, i, r, i . . .
        // fill first half of FFT_buffer with last events audio samples
        for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
        {
            NR.FFT_buffer[i * 2] = NR.last_sample_buffer_L[i]; // real
            NR.FFT_buffer[i * 2 + 1] = 0.0; // imaginary
        }
        // copy recent samples to last_sample_buffer for next time!
        for(int i = 0; i < nr_params.NR_FFT_L  / 2; i++)
        {
            NR.last_sample_buffer_L [i] = in_buffer[i + k * (nr_params.NR_FFT_L / 2)];
        }
        // now fill recent audio samples into second half of FFT_buffer
        for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
        {
            NR.FFT_buffer[nr_params.NR_FFT_L + i * 2] = in_buffer[i+ k * (nr_params.NR_FFT_L / 2)]; // real
            NR.FFT_buffer[nr_params.NR_FFT_L + i * 2 + 1] = 0.0;
        }
        /////////////////////////////////7
        // WINDOWING

        //          if(nr_params.fft_256_enable)
        {
            for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
            {
                NR.FFT_buffer[idx * 2] *= SQRT_von_Hann_256[idx];
            }
            arm_cfft_f32(&arm_cfft_sR_f32_len256, NR.FFT_buffer, 0, 1);
        }
        /*else
          {
              for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
              {
            	  NR.FFT_buffer[idx * 2] *= SQRT_van_hann[idx];
              }
              arm_cfft_f32(&arm_cfft_sR_f32_len128, NR.FFT_buffer, 0, 1);
          }*/

        // NR_FFT
        // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]



        for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
        {
            //here we need squared magnitude
            NR2.X[bindx][0] = (NR.FFT_buffer[bindx * 2] * NR.FFT_buffer[bindx * 2] + NR.FFT_buffer[bindx * 2 + 1] * NR.FFT_buffer[bindx * 2 + 1]);
        }

        if(nr_params.first_time == 2)
        {
            for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)
            {
                NR.Nest[bindx][0] = NR.Nest[bindx][0] + 0.05* NR2.X[bindx][0];// we do it 20 times to average over 20 frames for app. 100ms only on NR_on/bandswitch/modeswitch,...
                xt[bindx] = psini * NR.Nest[bindx][0];
            }
            NR_init_counter++;
            if (NR_init_counter > 19)//average over 20 frames for app. 100ms
            {
                NR_init_counter = 0;
                nr_params.first_time = 3;  // now we did all the necessary initialization to actually start the noise reduction
            }
        }
        if (nr_params.first_time == 3)
        {

            //new noise estimate MMSE based!!!

            for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)// 1. Step of NR - calculate the SNR's
            {
                ph1y[bindx] = 1.0 / (1.0 + NR2.pfac * expf(NR2.xih1r * NR2.X[bindx][0]/xt[bindx]));
                pslp[bindx] = NR2.ap * pslp[bindx] + (1.0 - NR2.ap) * ph1y[bindx];
                //ph1y[bindx] = fmin(ph1y[bindx], 1.0 - pnsaf * (pslp[bindx] > psthr)); //?????

                if (pslp[bindx] > psthr)
                {
                    ph1y[bindx] = 1.0 - pnsaf;
                }
                else
                {
                    ph1y[bindx] = fmin(ph1y[bindx] , 1.0);
                }
                xtr = (1.0 - ph1y[bindx]) * NR2.X[bindx][0] + ph1y[bindx] * xt[bindx];
                xt[bindx] = NR2.ax * xt[bindx] + (1.0 - NR2.ax) * xtr;
            }


            for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++)// 1. Step of NR - calculate the SNR's
            {
                NR.SNR_post[bindx] = fmax(fmin(NR2.X[bindx][0] / xt[bindx],1000.0), NR2.snr_prio_min); // limited to +30 /-15 dB, might be still too much of reduction, let's try it?

                NR.SNR_prio[bindx] = fmax(nr_params.alpha * NR.Hk_old[bindx] + (1.0 - nr_params.alpha) * fmax(NR.SNR_post[bindx] - 1.0, 0.0), 0.0);
            }

            VAD_low = (int)lf_freq;

            VAD_high = (int)uf_freq;

            if(VAD_low == VAD_high)
            {
                VAD_high++;
            }
            if(VAD_low < 1)
            {
                VAD_low = 1;
            }
            else
                if(VAD_low > nr_params.NR_FFT_L / 2 - 2)
                {
                    VAD_low = nr_params.NR_FFT_L / 2 - 2;
                }
            if(VAD_high < 1)
            {
                VAD_high = 1;
            }
            else
                if(VAD_high > nr_params.NR_FFT_L / 2)
                {
                    VAD_high = nr_params.NR_FFT_L / 2;
                }


            // 4    calculate v = SNRprio(n, bin[i]) / (SNRprio(n, bin[i]) + 1) * SNRpost(n, bin[i]) (eq. 12 of Schmitt et al. 2002, eq. 9 of Romanin et al. 2009)
            //		   and calculate the HK's

            for(int bindx = VAD_low; bindx < VAD_high; bindx++)// maybe we should limit this to the signal containing bins (filtering!!)
            {
                float32_t v = NR.SNR_prio[bindx] * NR.SNR_post[bindx] / (1.0 + NR.SNR_prio[bindx]);

                NR2.Hk[bindx] = fmax(1.0 / NR.SNR_post[bindx] * sqrtf((0.7212 * v + v * v)),0.001); //limit HK's to 0.001'

                NR.Hk_old[bindx] = NR.SNR_post[bindx] * NR2.Hk[bindx] * NR2.Hk[bindx]; //


                /*if(!(ts.dsp_active & DSP_NR_ENABLE)) // if NR is not enabled (but notch is enabled !)
			  {
				  NR2.Hk[bindx] = 1.0;
			  } */
            }
            // musical noise "artefact" reduction by dynamic averaging - depending on SNR ratio
            NR2.pre_power = 0.0;
            NR2.post_power = 0.0;
            for(int bindx = VAD_low; bindx < VAD_high; bindx++)
            {
                NR2.pre_power += NR2.X[bindx][0];
                NR2.post_power += NR2.Hk[bindx] * NR2.Hk[bindx]  * NR2.X[bindx][0];
            }

            NR2.power_ratio = NR2.post_power / NR2.pre_power;
            if (NR2.power_ratio > NR2.power_threshold)
            {
                NR2.power_ratio = 1.0;
                NR2.NN = 1;
            }
            else
            {
                NR2.NN = 1 + 2 * (int)(0.5 + NR2.width * (1.0 - NR2.power_ratio / NR2.power_threshold));
            }

            for(int bindx = VAD_low + NR2.NN/2; bindx < VAD_high - NR2.NN/2; bindx++)
            {
                NR.Nest[bindx][0] = 0.0;
                for(int m = bindx - NR2.NN/2; m <= bindx + NR2.NN/2;m++)
                {
                    NR.Nest[bindx][0] += NR2.Hk[m];
                }
                NR.Nest[bindx][0] /= (float32_t)NR2.NN;
            }

            // and now the edges - only going NN steps forward and taking the average
            // lower edge
            for(int bindx = VAD_low; bindx < VAD_low + NR2.NN/2; bindx++)
            {
                NR.Nest[bindx][0] = 0.0;
                for(int m = bindx; m < (bindx + NR2.NN);m++)
                {
                    NR.Nest[bindx][0] += NR2.Hk[m];
                }
                NR.Nest[bindx][0] /= (float32_t)NR2.NN;
            }

            // upper edge - only going NN steps backward and taking the average
            for(int bindx = VAD_high - NR2.NN; bindx < VAD_high; bindx++)
            {
                NR.Nest[bindx][0] = 0.0;
                for(int m = bindx; m > (bindx - NR2.NN); m--)
                {
                    NR.Nest[bindx][0] += NR2.Hk[m];
                }
                NR.Nest[bindx][0] /= (float32_t)NR2.NN;
            }

            // end of edge treatment

            for(int bindx = VAD_low + NR2.NN/2; bindx < VAD_high - NR2.NN/2; bindx++)
            {
                NR2.Hk[bindx] = NR.Nest[bindx][0];
            }
            // end of musical noise reduction
        }	//end of "if nr_params.first_time == 3"


        // FINAL SPECTRAL WEIGHTING: Multiply current FFT results with NR_FFT_buffer for 64 bins with the 64 bin-specific gain factors
        // only do this for the bins inside the filter passband
        // if you do this for all the bins, you will get distorted audio: plopping !
        //              for(int bindx = 0; bindx < nr_params.NR_FFT_L / 2; bindx++) // plopping !!!!
        for(int bindx = VAD_low; bindx < VAD_high; bindx++) // no plopping
        {
            NR.FFT_buffer[bindx * 2] = 						NR.FFT_buffer [bindx * 2] * NR2.Hk[bindx]; // real part
            NR.FFT_buffer[bindx * 2 + 1] = 					NR.FFT_buffer [bindx * 2 + 1] * NR2.Hk[bindx]; // imag part
            NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] = 	NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] * NR2.Hk[bindx]; // real part conjugate symmetric
            NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] = 	NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] * NR2.Hk[bindx]; // imag part conjugate symmetric
            //                  NR.FFT_buffer[bindx * 2] = NR.FFT_buffer [bindx * 2] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // real part
            //                  NR.FFT_buffer[bindx * 2 + 1] = NR.FFT_buffer [bindx * 2 + 1] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // imag part
            //                  NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] = NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 2] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // real part conjugate symmetric
            //                  NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] = NR.FFT_buffer[nr_params.NR_FFT_L * 2 - bindx * 2 - 1] * NR2.Hk[bindx] * NR2.long_tone_gain[bindx]; // imag part conjugate symmetric
        }

        /*****************************************************************
         * NOISE REDUCTION CODE ENDS HERE
         *****************************************************************/
        // NR_iFFT
        // & Window on exit!
        //      if(nr_params.fft_256_enable)
        {
            arm_cfft_f32(&arm_cfft_sR_f32_len256, NR.FFT_buffer, 1, 1);
            for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
            {
                NR.FFT_buffer[idx * 2] *= SQRT_von_Hann_256[idx];
            }
        }
        /*else
      {
          arm_cfft_f32(&arm_cfft_sR_f32_len128, NR.FFT_buffer, 1, 1);
          for (int idx = 0; idx < nr_params.NR_FFT_L; idx++)
          {
        	  NR.FFT_buffer[idx * 2] *= SQRT_van_hann[idx];
          }
      }*/

        // do the overlap & add
        for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
        { // take real part of first half of current iFFT result and add to 2nd half of last iFFT_result
            //              NR_output_audio_buffer[i + k * (nr_params.NR_FFT_L / 2)] = NR_FFT_buffer[i * 2] + NR_last_iFFT_result[i];
            in_buffer[i + k * (nr_params.NR_FFT_L / 2)] = NR.FFT_buffer[i * 2] + NR.last_iFFT_result[i];
        }
        for(int i = 0; i < nr_params.NR_FFT_L / 2; i++)
        {
            NR.last_iFFT_result[i] = NR.FFT_buffer[nr_params.NR_FFT_L + i * 2];
        }
        // end of "for" loop which repeats the FFT_iFFT_chain two times !!!
    }

    // IIR biquad notch filter with four independent notches
    //  arm_biquad_cascade_df1_f32 (&NR_notch_biquad, in_buffer, in_buffer, nr_params.NR_FFT_L);
}

//alt noise blanking is trying to localize some impulse noise within the samples and after that
//trying to replace corrupted samples by linear predicted samples.
//therefore, first we calculate the lpc coefficients which represent the actual status of the
//speech or sound generating "instrument" (in case of speech this is an estimation of the current
//filter-function of the voice generating tract behind our lips :-) )
//after finding this function we inverse filter the actual samples by this function
//so we are eliminating the speech, but not the noise. Then we do a matched filtering an thereby detecting impulses
//After that we threshold the remaining samples by some
//level and so detecting impulse noise's positions within the current frame - if one (or more) impulses are there.
//finally some area around the impulse position will be replaced by predicted samples from both sides (forward and
//backward prediction)
//hopefully we have enough processor power left....

void alt_noise_blanking(float* insamp,int Nsam, int order, float* E )  //Nsam = 128
{

#ifdef debug_alternate_NR

const float32_t NR_test_samp[128] = { 853.472351,629.066223,864.270813,1012.3078,738.378113,
        446.268219,635.763123,1062.71118,955.245667,22.6679211,-1130.45386,-1737.12817,
        -1728.79114,-1594.82227,-1545.75671,-1208.91003,-252.898315,993.880493,1820.26538,
        1915.65186,1597.90259,1248.58838,809.456909,28.6509247,-961.62677,-1604.66443,-1499.18225,
        -824.882935,-85.1342163,432.899261,782.52063,1029.38452,1040.57166,692.128662,138.820541,
        -286.785767,-420.356415,-384.165161,-348.958527,-308.304718,-171.111633,4.52698851,  //last value:4.52698851,
        -5.53196001,-368.999939,-1031.19165,-1766.01074,-2290.01587,-2293.98853,-1514.0238,
        23.0157223,1797.16394,3018.3894,3231.77148,2702.38745,2085.92676,1685.99255,1145.43176,
        -31.9259377,-1722.42847,-3112.2937,-3453.61426,-2790.31763,-1812.12769,-1028.70874,
        -1812 ,897.985779,2375.50903,3409.33472,3332.44238,2293.16602,1067.26196,183.806381,
        -548.479553,-1549.47034,-2692.18213,-3288.44702,-2873.70239,-1761.34033,-636.71936,
        250.664383,1198.7804,2336.43726,3121.80615,2848.64355,1556.67969,110.084801,-724.328186,
        -1013.82141,-1265.38879,-1506.06091,-1177.04529,-35.6577721,1209.823,1520.28088,679.406555,
        -514.541626,-1245.55945,-1508.29407,-1707.93408,-1736.12427,-965.137085,752.618347,2518.7168,
        3185.57031,2563.83838,1472.3927,613.243835,-172.269989,-1311.97058,-2534.06421,-2982.73169,
        -2282.05859,-1025.64673,12.714426,809.696228,1828.12854,2977.01709,3388.77612,2460.82178,
        751.800781,-567.183105,-1026.46143,-1190.80762,-1635.05701,-2060.84619,-1785.74683,-841.740173,
        -62.468441

};



const float32_t NR_test_sinus_samp[128] = {
        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302
};


#endif

#define boundary_blank 14 // for first trials very large!!!!
#define impulse_length 7 // has to be odd!!!! 7 / 3 should be enough
#define PL             3 // has to be (impulse_length-1)/2 !!!!
#define order         10 // lpc's order
    arm_fir_instance_f32 LPC;
    float32_t lpcs[order+1]; // we reserve one more than "order" because of a leading "1"
    float32_t reverse_lpcs[order+1]; //this takes the reversed order lpc coefficients
    float32_t firStateF32[NR_FFT_SIZE + order];
    float32_t tempsamp[NR_FFT_SIZE];
    float32_t sigma2; //taking the variance of the inpo
    float32_t lpc_power;
    float32_t impulse_threshold;
    int impulse_positions[5];  //we allow a maximum of 5 impulses per frame
    int search_pos=0;
    int impulse_count=0;
    //static float32_t last_frame_end[order+PL]; //this takes the last samples from the previous frame to do the prediction within the boundaries
#ifdef debug_alternate_NR
    static int frame_count=0;  //only used for the distortion insertion - can alter be deleted
    int dist_level=0;//only used for the distortion insertion - can alter be deleted
    int nr_setting = 0;
#endif

    float32_t R[11];  // takes the autocorrelation results
    float32_t k,alfa;

    float32_t any[order+1];  //some internal buffers for the levinson durben algorithm

    float32_t Rfw[impulse_length+order]; // takes the forward predicted audio restauration
    float32_t Rbw[impulse_length+order]; // takes the backward predicted audio restauration
    float32_t Wfw[impulse_length],Wbw[impulse_length]; // taking linear windows for the combination of fwd and bwd

    float32_t s;

    static float32_t working_buffer[NR_FFT_SIZE + 2 * order + 2 * PL]; //we need 128 + 26 floats to work on -
								      //necessary to watch for impulses as close to the frame boundaries as possible

#ifdef debug_alternate_NR  // generate test frames to test the noise blanker function
    // using the NR-setting (0..55) to select the test frame
    // 00 = noise blanker active on orig. audio; threshold factor=3
    // 01 = frame of vocal "a" undistorted
    // 02 .. 05 = frame of vocal "a" with different impulse distortion levels
    // 06 .. 09 = frame of vocal "a" with different impulse distortion levels
    //            noise blanker operating!!
    //************
    // 01..09 are now using the original received audio and applying a rythmic "click" distortion
    // 06..09 is detecting and removing the click by restoring the predicted audio!!!
    //************
    // 5 / 9 is the biggest "click" and it is slightly noticeable in the restored audio (9)
    // 10 = noise blanker active on orig. audio threshold factor=3
    // 11  = sinusoidal signal undistorted
    // 12 ..15 = sinusoidal signal with different impulse distortion levels
    // 16 ..19 = sinusoidal signal with different impulse distortion levels
    //            noise blanker operating!!
    // 20 ..50   noise blanker active on orig. audio; threshold factor varying between 3 and 0.26

    nr_setting = (int)ts.dsp_nr_strength;
    //*********************************from here just debug impulse / signal generation
    if ((nr_setting > 0) && (nr_setting < 10)) // we use the vocal "a" frame
    {
        //for (int i=0; i<128;i++)          // not using vocal "a" but the original signal
        //    insamp[i]=NR_test_samp[i];

        if ((frame_count > 19) && (nr_setting > 1))    // insert a distorting pulse
        {
            dist_level=nr_setting;
            if (dist_level > 5) dist_level=dist_level-4; // distortion level is 1...5
            insamp[4]=insamp[4] + dist_level*3000; // overlaying a short  distortion pulse +/-
            insamp[5]=insamp[5] - dist_level*1000;
        }

    }

    if ((nr_setting > 10) && (nr_setting < 20)) // we use the sinus frame
    {
        for (int i=0; i<128;i++)
            insamp[i]=NR_test_sinus_samp[i];

        if ((frame_count > 19) && (nr_setting > 11))    // insert a distorting pulse
        {
            dist_level=nr_setting-10;
            if (dist_level > 5) dist_level=dist_level-4;
            insamp[24]=insamp[24] + dist_level*1000; // overlaying a short  distortion pulse +/-
            insamp[25]=insamp[25] + dist_level*500;
            insamp[26]=insamp[26] - dist_level*200; // overlaying a short  distortion pulse +/-
            insamp[27]=insamp[27] - dist_level*100;


        }



    }


    frame_count++;
    if (frame_count > 20) frame_count=0;

#endif

    //*****************************end of debug impulse generation

    memcpy(&working_buffer[2*PL + 2*order],insamp,NR_FFT_SIZE * sizeof(float32_t));// copy incomming samples to the end of our working bufer


    //  start of test timing zone

    for (int i=0; i<impulse_length; i++)  // generating 2 Windows for the combination of the 2 predictors
    {                                     // will be a constant window later!
        Wbw[i]=1.0*i/(impulse_length-1);
        Wfw[impulse_length-i-1]=Wbw[i];
    }

    // calculate the autocorrelation of insamp (moving by max. of #order# samples)
    for(int i=0; i < (order+1); i++)
    {
    //    arm_dot_prod_f32(&insamp[0],&insamp[i],Nsam-i,&R[i]); // R is carrying the crosscorrelations
	arm_dot_prod_f32(&working_buffer[order+PL+0],&working_buffer[order+PL+i],Nsam-i,&R[i]); // R is carrying the crosscorrelations
    }
    // end of autocorrelation



    //alternative levinson durben algorithm to calculate the lpc coefficients from the crosscorrelation

    R[0] = R[0] * (1.0 + 1.0e-9);

    lpcs[0] = 1;   //set lpc 0 to 1

    for (int i=1; i < order+1; i++)
        lpcs[i]=0;                      // fill rest of array with zeros - could be done by memfill

    alfa = R[0];

    for (int m = 1; m <= order; m++)
    {
        s = 0.0;
        for (int u = 1; u < m; u++)
            s = s + lpcs[u] * R[m-u];

        k = -(R[m] + s) / alfa;

        for (int v = 1;v < m; v++)
            any[v] = lpcs[v] + k * lpcs[m-v];

        for (int w = 1; w < m; w++)
            lpcs[w] = any[w];

        lpcs[m] = k;
        alfa = alfa * (1 - k * k);
    }

    // end of levinson durben algorithm

    for (int o = 0; o < order+1; o++ )             //store the reverse order coefficients separately
        reverse_lpcs[order-o]=lpcs[o];        // for the matched impulse filter

    arm_fir_init_f32(&LPC,order+1,&reverse_lpcs[0],&firStateF32[0],NR_FFT_SIZE);                                         // we are using the same function as used in freedv

    //arm_fir_f32(&LPC,insamp,tempsamp,Nsam); //do the inverse filtering to eliminate voice and enhance the impulses
    arm_fir_f32(&LPC,&working_buffer[order+PL],tempsamp,Nsam); //do the inverse filtering to eliminate voice and enhance the impulses

    arm_fir_init_f32(&LPC,order+1,&lpcs[0],&firStateF32[0],NR_FFT_SIZE);                                         // we are using the same function as used in freedv

    arm_fir_f32(&LPC,tempsamp,tempsamp,Nsam); // do a matched filtering to detect an impulse in our now voiceless signal


    arm_var_f32(tempsamp,NR_FFT_SIZE,&sigma2); //calculate sigma2 of the original signal ? or tempsignal

    arm_power_f32(lpcs,order,&lpc_power);  // calculate the sum of the squares (the "power") of the lpc's

    //    impulse_threshold = (float32_t)ts.nb_setting * 0.5 * sqrtf(sigma2 * lpc_power);  //set a detection level (3 is not really a final setting)
    impulse_threshold = (float32_t)(16 - ts.dsp.nb_setting) * 0.5 * sqrtf(sigma2 * lpc_power);  //set a detection level (3 is not really a final setting)

    //if ((nr_setting > 20) && (nr_setting <51))
    //    impulse_threshold = impulse_threshold / (0.9 + (nr_setting-20.0)/10);  //scaling the threshold by 1 ... 0.26

    search_pos = order+PL;  // lower boundary problem has been solved! - so here we start from 1 or 0?
    //search_pos = 1;
    impulse_count=0;

    do {        //going through the filtered samples to find an impulse larger than the threshold

        if ((tempsamp[search_pos] > impulse_threshold)||(tempsamp[search_pos] < (-impulse_threshold)))
        {
            impulse_positions[impulse_count]=search_pos - order;  // save the impulse positions and correct it by the filter delay
            impulse_count++;
            search_pos+=PL;   //  set search_pos a bit away, cause we are already repairing this area later
            //  and the next impulse should not be that close
        }

        search_pos++;

    //} while ((search_pos < NR_FFT_SIZE-boundary_blank) && (impulse_count < 5));// avoid upper boundary
    } while ((search_pos < NR_FFT_SIZE) && (impulse_count < 5));


    // from here: reconstruction of the impulse-distorted audio part:

    // first we form the forward and backward prediction transfer functions from the lpcs
    // that is easy, as they are just the negated coefficients  without the leading "1"
    // we can do this in place of the lpcs, as they are not used here anymore and being recalculated in the next frame!

    arm_negate_f32(&lpcs[1],&lpcs[1],order);
    arm_negate_f32(&reverse_lpcs[0],&reverse_lpcs[0],order);


    for (int j=0; j<impulse_count; j++)
    {
        for (int k = 0; k<order; k++)   // we have to copy some samples from the original signal as
        {                           // basis for the reconstructions - could be done by memcopy

            //if ((impulse_positions[j]-PL-order+k) < 0)// this solves the prediction problem at the left boundary
            //{
            //   Rfw[k]=last_frame_end[impulse_positions[j]+k];//take the sample from the last frame
            //}
            //else
            //{
                //Rfw[k]=insamp[impulse_positions[j]-PL-order+k];//take the sample from this frame as we are away from the boundary
                Rfw[k]=working_buffer[impulse_positions[j]+k];//take the sample from this frame as we are away from the boundary
                //}

            //Rbw[impulse_length+k]=insamp[impulse_positions[j]+PL+k+1];
            Rbw[impulse_length+k]=working_buffer[order+PL+impulse_positions[j]+PL+k+1];



        }     //bis hier alles ok

        for (int i = 0; i < impulse_length; i++) //now we calculate the forward and backward predictions
        {
            arm_dot_prod_f32(&reverse_lpcs[0],&Rfw[i],order,&Rfw[i+order]);
            arm_dot_prod_f32(&lpcs[1],&Rbw[impulse_length-i],order,&Rbw[impulse_length-i-1]);

        }

        arm_mult_f32(&Wfw[0],&Rfw[order],&Rfw[order],impulse_length); // do the windowing, or better: weighing
        arm_mult_f32(&Wbw[0],&Rbw[0],&Rbw[0],impulse_length);



#ifdef debug_alternate_NR
        // in debug mode do the restoration only in some cases
        if (((ts.dsp_nr_strength > 0) && (ts.dsp_nr_strength < 6))||((ts.dsp_nr_strength > 10) && (ts.dsp_nr_strength < 16)))
        {
            // just let the distortion pass at setting 1...5 and 11...15
            //    arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);
        }
        else
        {
            //finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
            //arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);
            arm_add_f32(&Rfw[order],&Rbw[0],&working_buffer[order+PL+impulse_positions[j]-PL],impulse_length);
        }
#else
        //finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
        //arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);
        arm_add_f32(&Rfw[order],&Rbw[0],&working_buffer[order+impulse_positions[j]],impulse_length);

#endif
    }

    //for (int p=0; p<(order+PL); p++)
    //{
    //    last_frame_end[p]=insamp[NR_FFT_SIZE-1-order-PL+p];// store 13 samples from the current frame to use at the next frame
    //}
    //end of test timing zone
memcpy(insamp,&working_buffer[order+PL],NR_FFT_SIZE * sizeof(float32_t));// copy the samples of the current frame back to the insamp-buffer for output
memcpy(working_buffer,&working_buffer[NR_FFT_SIZE],(2*order + 2*PL) * sizeof(float32_t)); // copy
}



#endif
