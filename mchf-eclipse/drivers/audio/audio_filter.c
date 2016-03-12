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
**  Licence:        CC BY-NC-SA 3.0                                                **
************************************************************************************/
#include "mchf_board.h"
#include "audio_filter.h"

#include "arm_math.h"
#include "math.h"
#include "audio_driver.h"

// SSB Hilbert filters
#include "filters/q_rx_filter_10kHz.h"
#include "filters/i_rx_filter_10kHz.h"
//
#include "filters/q_rx_filter_7kHz5.h"
#include "filters/i_rx_filter_7kHz5.h"
//
#include "filters/q_rx_filter_6kHz.h"
#include "filters/i_rx_filter_6kHz.h"
//
#include "filters/q_rx_filter_5kHz.h"
#include "filters/i_rx_filter_5kHz.h"
//
#include "filters/q_rx_filter_3k6.h"
#include "filters/i_rx_filter_3k6.h"

// SSB Hilbert TX Filter
#include "filters/q_tx_filter.h"
#include "filters/i_tx_filter.h"

// AM RX Hilbert Filters
#include "filters/iq_rx_filter_am_10kHz.h"
#include "filters/iq_rx_filter_am_7kHz5.h"
#include "filters/iq_rx_filter_am_6kHz.h"
#include "filters/iq_rx_filter_am_5kHz.h"
#include "filters/iq_rx_filter_am_3k6.h"
#include "filters/iq_rx_filter_am_2k3.h"

//
// IIR lattice ARMA filters with time-reversed elements
//
#include "filters/iir_300hz.h"
#include "filters/iir_500hz.h"
#include "filters/iir_1_4k.h"
#include "filters/iir_1_6k.h"
#include "filters/iir_1_8k.h"
#include "filters/iir_2_1k.h"
#include "filters/iir_2_3k.h"
#include "filters/iir_2_5k.h"
#include "filters/iir_2_7k.h"
#include "filters/iir_2_9k.h"
#include "filters/iir_3_2k.h"
#include "filters/iir_3_4k.h"
#include "filters/iir_3_6k.h"
#include "filters/iir_3_8k.h"
#include "filters/iir_4k.h"
#include "filters/iir_4_2k.h"
#include "filters/iir_4_4k.h"
#include "filters/iir_4_6k.h"
#include "filters/iir_4_8k.h"
#include "filters/iir_5k.h"
//#include "filters/iir_5_5k.h"
#include "filters/iir_6k.h"
//#include "filters/iir_6_5k.h"
//#include "filters/iir_7k.h"
#include "filters/iir_7_5k.h"
#include "filters/iir_8k.h"
#include "filters/iir_8_5k.h"
#include "filters/iir_9k.h"
#include "filters/iir_9_5k.h"
#include "filters/iir_10k.h"

// single file with all of the different interpolation IIR filters
#include "filters/iir_antialias.h"

#include "filters/iir_15k_hpf_fm_squelch.h"

#include "filters/iir_2k7_tx_bpf.h"
#include "filters/iir_2k7_tx_bpf_fm.h"

// FIR filters for decimation and interpolation
#include "filters/fir_rx_decimate_4.h"	// with low-pass filtering
#include "filters/fir_rx_decimate_4_min_lpf.h"	// This has minimized LPF for the 10 kHz filter mode
#include "filters/fir_rx_interpolate_16.h"	// filter for interpolate-by-16 operation
#include "filters/fir_rx_interpolate_16_10kHz.h"	// This has relaxed LPF for the 10 kHz filter mode


/*
typedef struct FilterDescriptor_s {
  uint8_t id;
  const char*  name;
  uint8_t configs_num;
  uint8_t config_default;
  const char** config_labels;
} FilterDescriptor;
*/
static const FilterConfig filter_stdLabelsLpfBpf[] =
{
    {"OFF", 0},
    {"LPF", 0 },
    {"BPF",0}
};

static const FilterConfig filter_stdLabelsOnOff[] =
{
    { "OFF", 0},
    { " ON",0}
};


static const FilterConfig filter_list_300Hz[] =      {
    {"  OFF", 0 },
    {"500Hz", 500 },
    {"550Hz", 550 },
    {"600Hz", 600 },
    {"650Hz", 650 },
    {"700Hz",700},
    {"750Hz",750},
    {"800Hz",800},
    {"850Hz",850},
    {"900Hz",900},
    {"950Hz",950}
} ;

static const FilterConfig filter_list_500Hz[] =      {
    {"  OFF", 0},
    {"550Hz", 550},
    {"650Hz", 650},
    {"750Hz", 750},
    {"850Hz",850},
    {"950Hz",950},
} ;

static const FilterConfig filter_list_1P4KHz[] =
{
    {" OFF", 0},
    {" LPF", 700 },
    {" BPF", 775},
    {"SSTV", 1800}
};

static const FilterConfig filter_list_1P8KHz[] =      {
    {"   OFF",0},
    {"1125Hz",1125},
    {"1275Hz",1275},
    {"1427Hz",1427},
    {"1575Hz",1575},
    {"1725Hz",1725},
    {"   LPF",900},
} ;

static const FilterConfig filter_list_2P3KHz[] =      {
    {"   OFF",0},
    {"1262Hz",1262},
    {"1412Hz",1412},
    {"1562Hz",1562},
    {"1712Hz",1712},
    {"   LPF",1150},
} ;

#define FILTER_ALL (FILTER_CW|FILTER_SSB|FILTER_AM|FILTER_FM)
#define FILTER_NOFM (FILTER_CW|FILTER_SSB|FILTER_AM)
#define FILTER_SSBAM (FILTER_SSB|FILTER_AM)
#define FILTER_SSBCW (FILTER_SSB|FILTER_CW)
#define FILTER_NONE (0)
#define FILTER_SSBAMFM (FILTER_SSB|FILTER_AM|FILTER_FM)

FilterDescriptor FilterInfo[AUDIO_FILTER_NUM] =
{ // 	id ,	name	 ,  width, allowed_modes, always_on_modes, configs_num, config_default, config[label, offset]
    {  AUDIO_300HZ,  " 300Hz ",   300, FILTER_NOFM,    FILTER_CW,  11, 6, filter_list_300Hz},
    {  AUDIO_500HZ,  " 500Hz ",   500, FILTER_NOFM,    FILTER_CW,   6, 3, filter_list_500Hz},
    {  AUDIO_1P4KHZ, "  1.4k ",  1400, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_list_1P4KHz},
    {  AUDIO_1P6KHZ, "  1.6k ",  1600, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf},
    {  AUDIO_1P8KHZ, "  1.8k ",  1800, FILTER_SSBAM,   FILTER_SSB,  7, 6, filter_list_1P8KHz},
    {  AUDIO_2P1KHZ, "  2.1k ",  2100, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P3KHZ, "  2.3k ",  2300, FILTER_SSBAM,   FILTER_SSB,  6, 2, filter_list_2P3KHz },
    {  AUDIO_2P5KHZ, "  2.5k ",  2500, FILTER_SSBAMFM, FILTER_FM,   3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P7KHZ, "  2.7k ",  2700, FILTER_NOFM,    FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P9KHZ, "  2.9k ",  2900, FILTER_SSBAM,   FILTER_AM,   3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P2KHZ, "  3.2k ",  3200, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P4KHZ, "  3.4k ",  3400, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P6KHZ, "  3.6k ",  3600, FILTER_SSBAMFM, FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P8KHZ, "  3.8k ",  3800, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P0KHZ, "  4.0k ",  4000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P2KHZ, "  4.2k ",  4200, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P4KHZ, "  4.4k ",  4400, FILTER_NOFM,    FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P6KHZ, "  4.6k ",  4600, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P8KHZ, "  4.8k ",  4800, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_5P0KHZ, "  5.0k ",  5000, FILTER_SSBAMFM, FILTER_FM,   2, 1, filter_stdLabelsOnOff },
//    {  AUDIO_5P5KHZ, "  5.5k ",  5500, FILTER_SSB,	   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_6P0KHZ, "  6.0k ",  6000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
//    {  AUDIO_6P5KHZ, "  6.5k ",  6500, FILTER_SSB, 	   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
//    {  AUDIO_7P0KHZ, "  7.0k ",  7000, FILTER_SSB,     FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_7P5KHZ, "  7.5k ",  7500, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_8P0KHZ, "  8.0k ",  8000, FILTER_SSB,     FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_8P5KHZ, "  8.5k ",  8500, FILTER_SSB,     FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_9P0KHZ, "  9.0k ",  9000, FILTER_SSB,     FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_9P5KHZ, "  9.5k ",  9500, FILTER_SSB,     FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_10P0KHZ," 10.0k ", 10000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff }
};

/*// filter_characteristic: bandpass (with different centre freqs), lowpass etc.

FIR coeff_I_numTaps (= FIR coeff_Q_numTaps)
FIR coeff_I_coeffs: points to the array of FIR filter coeffs used in the I path
FIR coeff_Q_coeffs: points to the array of FIR filter coeffs used in the Q path

dec- filter_numTaps: points to the array of FIR coeffs used in the decimation fliter
dec- filter_coeffs

sample rate: gives the sample rate used after decimation (12ksps, 24ksps . . .)

IIR yes/no: bool: is an audio IIR filter used after decimation?

IIR audio coeff_numStages: points to the array of IIR coeff used for the audio IIR filter
IIR audio coeff_pk
IIR audio coeff_pv

int. filter coeffs_numTaps
int. filter coeffs: points to the array of IIR coeffs for the antialias interpolation filter used by the ARM interpolation routine

IIR_antialias_yes/no: bool - is an additional IIR filter after the interpolation used?

IIR_antialias_numStages
IIR_antialias_coeff_pk
IIR_antialias_coeff_pv: points to the array of IIR coeffs for the antialias IIR filter (works at 48ksps)
*/

/*################################################################
 * FILTER PLAYGROUND for the brave mcHF owner
 *
 * FilterPathInfo
 *
 * put together your custom filters here
 * All components of the whole filterpath are defined in this file
 * But: it could be that you have to change "FilterInfo" too
 * Have fun and learn about DSP ;-) but do not forget:
 * finally, the filter has to please your own ears
 * DD4WH
 *
 * Most of the combinations below had been previously defined and put
 * together in an old version of the firmware by Clint KA7OEI,
 * so the good sound of the mcHF is his merit !
 * ###############################################################
 */

FilterPathDescriptor FilterPathInfo[80] = // how to automatically determine this figure? --> also change in audio_filter.h !!!
									// sum(
{
// ID, mode, filter_select_ID, FIR_numTaps, FIR_I_coeff_file, FIR_Q_coeff_file, FIR_dec_numTaps, FIR_dec_coeff_file,
//		sample_rate_dec,bool IIR_PreFilter_yes_no, IIR_PreFilter_numTaps, IIR_PreFilter_pk_file,
//		IIR_PreFilter_pv_file, FIR_int_numTaps, FIR_int_coeff_file, bool IIR_int_yes_no,
//		IIR_int_numTaps, IIR_int_pk_file, IIR_int_pv_file

//###################################################################################################################################
// CW & SSB filters:
//###################################################################################################################################

		// 10 filters á 300Hz

	{	AUDIO_300HZ, FILTER_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_500_pkCoeffs, IIR_300hz_500_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_550_pkCoeffs, IIR_300hz_550_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 3, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_600_pkCoeffs, IIR_300hz_600_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 4, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_650_pkCoeffs, IIR_300hz_650_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 5, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_700_pkCoeffs, IIR_300hz_700_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 6, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_750_pkCoeffs, IIR_300hz_750_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 7, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_800_pkCoeffs, IIR_300hz_800_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 8, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_850_pkCoeffs, IIR_300hz_850_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 9, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_900_pkCoeffs, IIR_300hz_900_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_300HZ, FILTER_SSBCW, 10, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_300hz_numStages, IIR_300hz_950_pkCoeffs, IIR_300hz_950_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

		// 5 filters á 500Hz
	{	AUDIO_500HZ, FILTER_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_500hz_numStages, IIR_500hz_550_pkCoeffs, IIR_500hz_550_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_500HZ, FILTER_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_500hz_numStages, IIR_500hz_650_pkCoeffs, IIR_500hz_650_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_500HZ, FILTER_SSBCW, 3, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_500hz_numStages, IIR_500hz_750_pkCoeffs, IIR_500hz_750_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_500HZ, FILTER_SSBCW, 4, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_500hz_numStages, IIR_500hz_850_pkCoeffs, IIR_500hz_850_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_500HZ, FILTER_SSBCW, 5, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_500hz_numStages, IIR_500hz_950_pkCoeffs, IIR_500hz_950_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

//###################################################################################################################################
// SSB only filters:
//###################################################################################################################################

	{	AUDIO_1P4KHZ, FILTER_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k4_numStages, IIR_1k4_LPF_pkCoeffs, IIR_1k4_LPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P4KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k4_numStages, IIR_1k4_BPF_pkCoeffs, IIR_1k4_BPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P6KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k6_numStages, IIR_1k6_LPF_pkCoeffs, IIR_1k6_LPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P6KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k6_numStages, IIR_1k6_BPF_pkCoeffs, IIR_1k6_BPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P8KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k8_numStages, IIR_1k8_1k125_pkCoeffs, IIR_1k8_1k125_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P8KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k8_numStages, IIR_1k8_1k275_pkCoeffs, IIR_1k8_1k275_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P8KHZ, FILTER_SSB, 3, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k8_numStages, IIR_1k8_1k425_pkCoeffs, IIR_1k8_1k425_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P8KHZ, FILTER_SSB, 4, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k8_numStages, IIR_1k8_1k575_pkCoeffs, IIR_1k8_1k575_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P8KHZ, FILTER_SSB, 5, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k8_numStages, IIR_1k8_1k725_pkCoeffs, IIR_1k8_1k725_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P8KHZ, FILTER_SSB, 6, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_1k8_numStages, IIR_1k8_LPF_pkCoeffs, IIR_1k8_LPF_pkCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P1KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k1_numStages, IIR_2k1_LPF_pkCoeffs, IIR_2k1_LPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P1KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k1_numStages, IIR_2k1_BPF_pkCoeffs, IIR_2k1_BPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P3KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k3_numStages, IIR_2k3_1k275_pkCoeffs, IIR_2k3_1k275_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P3KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k3_numStages, IIR_2k3_1k412_pkCoeffs, IIR_2k3_1k412_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P3KHZ, FILTER_SSB, 3, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k3_numStages, IIR_2k3_1k562_pkCoeffs, IIR_2k3_1k562_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P3KHZ, FILTER_SSB, 4, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k3_numStages, IIR_2k3_1k712_pkCoeffs, IIR_2k3_1k712_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P3KHZ, FILTER_SSB, 5, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k3_numStages, IIR_2k3_LPF_pkCoeffs, IIR_2k3_LPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},
//33
	{	AUDIO_2P5KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k5_numStages, IIR_2k5_LPF_pkCoeffs, IIR_2k5_LPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P5KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k5_numStages, IIR_2k5_BPF_pkCoeffs, IIR_2k5_BPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P7KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k7_numStages, IIR_2k7_LPF_pkCoeffs, IIR_2k7_LPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P7KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k7_numStages, IIR_2k7_BPF_pkCoeffs, IIR_2k7_BPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P9KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k9_numStages, IIR_2k9_LPF_pkCoeffs, IIR_2k9_LPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P9KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k9_numStages, IIR_2k9_BPF_pkCoeffs, IIR_2k9_BPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_3P2KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k2_numStages, IIR_3k2_LPF_pkCoeffs, IIR_3k2_LPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_3P2KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k2_numStages, IIR_3k2_BPF_pkCoeffs, IIR_3k2_BPF_pvCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

		// in filters from 3k4 on, the FIR interpolate is 4 taps and an additional IIR interpolation filter
		// is switched in to accurately prevent alias frequencies
	{	AUDIO_3P4KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k4_numStages, IIR_3k4_LPF_pkCoeffs, IIR_3k4_LPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_3P4KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k4_numStages, IIR_3k4_BPF_pkCoeffs, IIR_3k4_BPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_3P6KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k6_numStages, IIR_3k6_LPF_pkCoeffs, IIR_3k6_LPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_3P6KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k6_numStages, IIR_3k6_BPF_pkCoeffs, IIR_3k6_BPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_3P8KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k8_numStages, IIR_3k8_LPF_pkCoeffs, IIR_3k8_LPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_3P8KHZ, FILTER_SSB, 2, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k8_numStages, IIR_3k8_BPF_pkCoeffs, IIR_3k8_BPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_4P0KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_4k_numStages, IIR_4k_LPF_pkCoeffs, IIR_4k_LPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_4P2KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_4k2_numStages, IIR_4k2_LPF_pkCoeffs, IIR_4k2_LPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_4P4KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_4k4_numStages, IIR_4k4_LPF_pkCoeffs, IIR_4k4_LPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},
//50
	{	AUDIO_4P6KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_4k6_numStages, IIR_4k6_LPF_pkCoeffs, IIR_4k6_LPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

	{	AUDIO_4P8KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_4k8_numStages, IIR_4k8_LPF_pkCoeffs, IIR_4k8_LPF_pvCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages ,IIR_aa_5k_pkCoeffs , IIR_aa_5k_pvCoeffs},

		// new decimation rate, new decimation filter, new interpolation filter, no IIR Prefilter, no IIR interpolation filter
	{	AUDIO_5P0KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0 ,0 , 0},

	{	AUDIO_6P0KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_6k_coeffs, q_rx_6k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0 ,0 , 0},

	{	AUDIO_7P5KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_7k5_coeffs, q_rx_7k5_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0 ,0 , 0},
			// additional IIR interpolation filter
	{	AUDIO_8P0KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_8k_numStages ,IIR_aa_8k_pkCoeffs , IIR_aa_8k_pvCoeffs},

	{	AUDIO_8P5KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_8k5_numStages ,IIR_aa_8k5_pkCoeffs , IIR_aa_8k5_pvCoeffs},

	{	AUDIO_9P0KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_9k_numStages ,IIR_aa_9k_pkCoeffs , IIR_aa_9k_pvCoeffs},

	{	AUDIO_9P5KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_9k5_numStages ,IIR_aa_9k5_pkCoeffs , IIR_aa_9k5_pvCoeffs},

	{	AUDIO_10P0KHZ, FILTER_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_10k_numStages ,IIR_aa_10k_pkCoeffs , IIR_aa_10k_pvCoeffs},

//###################################################################################################################################
// AM filters: designed for an IIR lowpass stopband frequency that is approx. 1.8 times higher than the FIR bandwidth
//	--> an AM 2.5kHz filter has 2.5kHz of audio when centre tuned, but can have up to 1.8 * 2.5kHz = 4.6kHz of bandwidth (IIR filter!)
//		when tuned away from the carrier, this has been called "sideband-selected AM demodulation" . . . wow . . .
//###################################################################################################################################

		// in AM, we ALWAYS use the LPF IIR audio PreFilter, regardless of the selected filter_select_ID.
		// this is because we assume AM mode to be used to demodulate DSB signals, so BPF (sideband suppression) is not necessary

	{	AUDIO_1P4KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k3_numStages, IIR_2k3_LPF_pkCoeffs, IIR_2k3_LPF_pkCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P6KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_2k9_numStages, IIR_2k9_LPF_pkCoeffs, IIR_2k9_LPF_pkCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_1P8KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k2_numStages, IIR_3k2_LPF_pkCoeffs, IIR_3k2_LPF_pkCoeffs,
		RX_INTERPOLATE_NUM_TAPS, FirRxInterpolate, 0, 0 ,0 , 0},

	{	AUDIO_2P1KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_3k6_numStages, IIR_3k6_LPF_pkCoeffs, IIR_3k6_LPF_pkCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages, IIR_aa_5k_pkCoeffs, IIR_aa_5k_pvCoeffs},

	{	AUDIO_2P3KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_4k2_numStages, IIR_4k2_LPF_pkCoeffs, IIR_4k2_LPF_pkCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages, IIR_aa_5k_pkCoeffs, IIR_aa_5k_pvCoeffs},

	{	AUDIO_2P5KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_4k6_numStages, IIR_4k6_LPF_pkCoeffs, IIR_4k6_LPF_pkCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages, IIR_aa_5k_pkCoeffs, IIR_aa_5k_pvCoeffs},

	{	AUDIO_2P7KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, RX_DECIMATE_NUM_TAPS, FirRxDecimate,
		RX_DECIMATION_RATE_12KHZ, 1, IIR_4k8_numStages, IIR_4k8_LPF_pkCoeffs, IIR_4k8_LPF_pkCoeffs,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_5k, 1, IIR_aa_5k_numStages, IIR_aa_5k_pkCoeffs, IIR_aa_5k_pvCoeffs},

	{	AUDIO_2P9KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 1, IIR_5k5_numStages, IIR_5k5_LPF_pkCoeffs, IIR_5k5_LPF_pkCoeffs,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0, 0, 0},

	{	AUDIO_3P2KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 1, IIR_6k_numStages, IIR_6k_LPF_pkCoeffs, IIR_6k_LPF_pkCoeffs,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0, 0, 0},

	{	AUDIO_3P4KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 1, IIR_6k5_numStages, IIR_6k5_LPF_pkCoeffs, IIR_6k5_LPF_pkCoeffs,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0, 0, 0},

	{	AUDIO_3P6KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 1, IIR_7k_numStages, IIR_7k_LPF_pkCoeffs, IIR_7k_LPF_pkCoeffs,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0, 0, 0},

	{	AUDIO_3P8KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 1, IIR_7k_numStages, IIR_7k_LPF_pkCoeffs, IIR_7k_LPF_pkCoeffs,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0, 0, 0},

	{	AUDIO_4P0KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 1, IIR_7k5_numStages, IIR_7k5_LPF_pkCoeffs, IIR_7k5_LPF_pkCoeffs,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0, 0, 0},

	{	AUDIO_4P2KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 1, IIR_8k_numStages, IIR_8k_LPF_pkCoeffs, IIR_8k_LPF_pkCoeffs,
		RX_INTERPOLATE_10KHZ_NUM_TAPS, FirRxInterpolate10KHZ, 0, 0, 0, 0},

		// from 4.4kHz on, the AM filter has no more IIR PreFilter (at 24ksps sample rate), BUT we add IIR filtering after interpolation (at 48 ksps)!

	{	AUDIO_4P4KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_8k_numStages, IIR_aa_8k_pvCoeffs, IIR_aa_8k_pkCoeffs},

	{	AUDIO_4P6KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_8k5_numStages, IIR_aa_8k5_pvCoeffs, IIR_aa_8k5_pkCoeffs},

	{	AUDIO_4P8KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_9k_numStages, IIR_aa_9k_pvCoeffs, IIR_aa_9k_pkCoeffs},

	{	AUDIO_5P0KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_9k5_numStages, IIR_aa_9k5_pvCoeffs, IIR_aa_9k5_pkCoeffs},

		// from 6kHz on, we have no PreFilter, an IIR interpolation filter of 10k and only change the FIR filters bandwidths
		// remember that the AM 5k filter is capable of up to 10kHz bandwidth, if you offtune the AM carrier
		// . . . same for 6k = 12kHz bw, 7k5 = 15kHz bw, 10kHz = max of 20kHz bandwidth

	{	AUDIO_6P0KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_6k_coeffs, iq_rx_am_6k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_10k_numStages, IIR_aa_10k_pvCoeffs, IIR_aa_10k_pkCoeffs},

	{	AUDIO_7P5KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_7k5_coeffs, iq_rx_am_7k5_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_10k_numStages, IIR_aa_10k_pvCoeffs, IIR_aa_10k_pkCoeffs},

	{	AUDIO_10P0KHZ, FILTER_AM, 1, Q_NUM_TAPS, iq_rx_am_10k_coeffs, iq_rx_am_10k_coeffs, RX_DECIMATE_MIN_LPF_NUM_TAPS, FirRxDecimateMinLPF,
		RX_DECIMATION_RATE_24KHZ, 0, 0, 0, 0,
		RX_INTERPOLATE_4_NUM_TAPS, FirRxInterpolate_4_10k, 1, IIR_aa_10k_numStages, IIR_aa_10k_pvCoeffs, IIR_aa_10k_pkCoeffs},
	}; // end FilterPath


/*
 * @brief Find Next Applicable Filter based on the information in the filter data structure
 *
 * Takes into account the current mode (SSB, CW, ..., some special rules, etc,). It will wrap around and always return a
 * valid filter id. In case not applicable filter was found, it returns the currently selected filter_id
 *
 * @returns next applicable filter id
 */
uint8_t AudioFilter_NextApplicableFilter()
{

  bool  voice_mode;
  uint8_t retval = ts.filter_id;
  // by default we do not change the filter selection

  uint16_t myMode;
  switch(ts.dmod_mode) {
  case DEMOD_AM:
    myMode = FILTER_AM;
    break;
  case DEMOD_FM:
    myMode = FILTER_FM;
    break;
  case DEMOD_CW:
    myMode = FILTER_CW;
    break;
  // case DEMOD_LSB:
  // case DEMOD_USB:
  // case DEMOD_DIGI:
  default:
    myMode = FILTER_SSB;
  }

  if(ts.dmod_mode != DEMOD_FM) {        // bail out if FM as filters are selected in configuration menu
    int idx;

    //
    // Scan through filters to determine if the selected filter is disabled - and skip if it is.
    // NOTE:  The 2.3 kHz filter CANNOT be disabled
    //
    // This also handles filters that are disabled according to mode (e.g. CW filters in SSB mode, SSB filters in CW mode)
    //

    if((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_AM))    // check to see if we are set to a "voice" mode
    {
      voice_mode = 1;
    } else {                    // not in voice mode
      voice_mode = 0;
    }

    // we run through all audio filters, starting with the next following, making sure to wrap around
    // we leave this loop once we found a filter that is applicable using "break"
    // or skip to next filter to check using "continue"
    for (idx = (ts.filter_id+1)%AUDIO_FILTER_NUM; idx != ts.filter_id; idx = (idx+1)%AUDIO_FILTER_NUM)
    {

      // these rules handle special cases
      if((idx == AUDIO_1P8KHZ) && ((ts.filter_cw_wide_disable) && (ts.dmod_mode == DEMOD_CW))) { idx = AUDIO_300HZ; break; }
      // in this case, next applicable mode is 300 Hz, so selected and leave loop

      // we have to check if this mode is IN the list of modes always offered in this mode, regardless of enablement
      if ((FilterInfo[idx].always_on_modes & myMode) != 0) {
              // okay, applicable, leave loop
              break;
      }

      // now the rules for excluding a mode follow, in this case we call continue to go to next filter

      // not enable; next please
      if (!ts.filter_select[idx]) { continue; }

      if((idx == AUDIO_300HZ || idx == AUDIO_500HZ) && ((ts.filter_ssb_narrow_disable) && (voice_mode))) { continue; }
      // jump over 300 Hz / 500 Hz if ssb_narrow_disable and voice mode

      // last we have to check if this mode is NOT in the list of allowed modes for the filter
      if ((FilterInfo[idx].allowed_modes & myMode) == 0) {
        // okay, not applicable, next please
        continue;
      }

      // if we have arrived here, all is good, we can  use the index, so lets bail out here
      break;
    }
    retval = idx;
  }
  return retval;
}



//
// RX Hilbert transform (90 degree) FIR filter state tables and instances
//
static float32_t        FirState_I[128];
extern __IO arm_fir_instance_f32    FIR_I;
//
static float32_t        FirState_Q[128];
extern __IO arm_fir_instance_f32    FIR_Q;

//
// TX Hilbert transform (90 degree) FIR filter state tables and instances
//
static float            FirState_I_TX[128];
extern __IO arm_fir_instance_f32    FIR_I_TX;

static float            FirState_Q_TX[128];
extern __IO arm_fir_instance_f32    FIR_Q_TX;
//

/*
 * @brief Calculate RX FFT coeffients based on adjustment settings
 */
void AudioFilter_CalcRxPhaseAdj(void)
{
    float f_coeff, f_offset, var_norm, var_inv;
    ulong i;
    int phase;
    //
    // always make a fresh copy of the original Q and I coefficients
    // NOTE:  We are assuming that the I and Q filters are of the same length!
    //
    fc.rx_q_num_taps = Q_NUM_TAPS;
    fc.rx_i_num_taps = I_NUM_TAPS;
    //
    fc.rx_q_block_size = Q_BLOCK_SIZE;
    fc.rx_i_block_size = I_BLOCK_SIZE;
    //
    if(ts.dmod_mode == DEMOD_AM)    {       // AM - load low-pass, non Hilbert filters (e.g. no I/Q phase shift
        // FIR 2k3 up to 2k3 filter
        // FIR 3k6 up to 3k6 filter
        // FIR 5k up to 5k filter
        // FIR 7k5 up to 7k5 filter
        // FIR 10k up to 10k filter
        for(i = 0; i < Q_NUM_TAPS; i++) {
            if (ts.filter_id <= AUDIO_2P3KHZ){
                fc.rx_filt_q[i] = iq_rx_am_2k3_coeffs[i];
                fc.rx_filt_i[i] = iq_rx_am_2k3_coeffs[i];
            } else
                if (ts.filter_id <= AUDIO_3P6KHZ){
                    fc.rx_filt_q[i] = iq_rx_am_3k6_coeffs[i];
                    fc.rx_filt_i[i] = iq_rx_am_3k6_coeffs[i];
                } else
                    if (ts.filter_id <= AUDIO_5P0KHZ){
                        fc.rx_filt_q[i] = iq_rx_am_5k_coeffs[i];
                        fc.rx_filt_i[i] = iq_rx_am_5k_coeffs[i];
                    } else
                        if (ts.filter_id <= AUDIO_6P0KHZ){
                            fc.rx_filt_q[i] = iq_rx_am_6k_coeffs[i];
                            fc.rx_filt_i[i] = iq_rx_am_6k_coeffs[i];
                        } else
                            if (ts.filter_id <= AUDIO_7P5KHZ){
                                fc.rx_filt_q[i] = iq_rx_am_7k5_coeffs[i];
                                fc.rx_filt_i[i] = iq_rx_am_7k5_coeffs[i];
                            } else {
                                    fc.rx_filt_q[i] = iq_rx_am_10k_coeffs[i];
                                    fc.rx_filt_i[i] = iq_rx_am_10k_coeffs[i];
                            }
        } // end for
    }
    else if(ts.dmod_mode == DEMOD_FM)   {       // FM - load low-pass, non Hilbert filters (e.g. no I/Q phase shift
        for(i = 0; i < Q_NUM_TAPS; i++) {
            switch(ts.fm_rx_bandwidth)  {
                case FM_RX_BANDWIDTH_10K:
                    fc.rx_filt_q[i] = iq_rx_am_5k_coeffs[i];    // load 5 kHz FIR (2 x 5kHz = 10 kHz)
                    fc.rx_filt_i[i] = iq_rx_am_5k_coeffs[i];
                    break;
                case FM_RX_BANDWIDTH_12K:
                    fc.rx_filt_q[i] = iq_rx_am_6k_coeffs[i];    // load 6 kHz FIR (2 x 6 kHz = 12 kHz)
                    fc.rx_filt_i[i] = iq_rx_am_6k_coeffs[i];
                    break;
//              case FM_RX_BANDWIDTH_15K:
//                  fc.rx_filt_q[i] = iq_rx_am_7k5_coeffs[i];   // load 7.5kHz FIR (2 x 7.5 kHz = 15 kHz)
//                  fc.rx_filt_i[i] = iq_rx_am_7k5_coeffs[i];
//                  break;
                case FM_RX_BANDWIDTH_7K2:
                default:
                    for(i = 0; i < Q_NUM_TAPS; i++) {
                        fc.rx_filt_q[i] = iq_rx_am_3k6_coeffs[i];   // load 3.6 kHz FIR (2 x 3.6 kHz = 7.2 kHz)
                        fc.rx_filt_i[i] = iq_rx_am_3k6_coeffs[i];
                    }
                    break;
            }
        }
    }
    else    {       // SSB, Not AM or FM - load Hilbert transformation filters
        // fill Hilbert coeffs into fc.rx_filt
        for(i = 0; i < Q_NUM_TAPS; i++) {
            if (ts.filter_id <= AUDIO_3P6KHZ){
                fc.rx_filt_q[i] = q_rx_3k6_coeffs[i];
                fc.rx_filt_i[i] = i_rx_3k6_coeffs[i];
            } else
                    if (ts.filter_id <= AUDIO_5P0KHZ){
                        fc.rx_filt_q[i] = q_rx_5k_coeffs[i];
                        fc.rx_filt_i[i] = i_rx_5k_coeffs[i];
                    } else
                        if (ts.filter_id <= AUDIO_6P0KHZ){
                            fc.rx_filt_q[i] = q_rx_6k_coeffs[i];
                            fc.rx_filt_i[i] = i_rx_6k_coeffs[i];
                        } else
                            if (ts.filter_id <= AUDIO_7P5KHZ){
                                fc.rx_filt_q[i] = q_rx_7k5_coeffs[i];
                                fc.rx_filt_i[i] = i_rx_7k5_coeffs[i];
                            } else {
                                fc.rx_filt_q[i] = q_rx_10k_coeffs[i];
                                fc.rx_filt_i[i] = i_rx_10k_coeffs[i];
                            }
        } // end for
    }   // end else = SSB

        //
        if(ts.dmod_mode == DEMOD_LSB)   // get phase setting appropriate to mode
            phase = ts.rx_iq_lsb_phase_balance;     // yes, get current gain adjustment setting for LSB
        else
            phase = ts.rx_iq_usb_phase_balance;     // yes, get current gain adjustment setting for USB and other mdoes
        //
        if(phase != 0)  {   // is phase adjustment non-zero?
            var_norm = (float)phase;
            var_norm = fabs(var_norm);      // get absolute value of this gain adjustment
            var_inv = 32 - var_norm;        // calculate "inverse" of number of steps
            var_norm /= 32;     // fractionalize by the number of steps
            var_inv /= 32;                      // fractionalize this one, too
            if(phase < 0)   {   // was the phase adjustment negative?

                if(ts.filter_id <=AUDIO_3P6KHZ){
                    for(i = 0; i < Q_NUM_TAPS; i++) {
                        f_coeff = var_inv * q_rx_3k6_coeffs[i]; // get fraction of 90 degree setting
                        f_offset = var_norm * q_rx_3k6_coeffs_minus[i]; // get fraction of 89.5 degree setting
                        fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                    }
                } else
                    if(ts.filter_id <=AUDIO_5P0KHZ){
                        for(i = 0; i < Q_NUM_TAPS; i++) {
                            f_coeff = var_inv * q_rx_5k_coeffs[i];  // get fraction of 90 degree setting
                            f_offset = var_norm * q_rx_5k_coeffs_minus[i];  // get fraction of 89.5 degree setting
                            fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                        }
                    } else
                        if(ts.filter_id <=AUDIO_6P0KHZ){
                            for(i = 0; i < Q_NUM_TAPS; i++) {
                                f_coeff = var_inv * q_rx_6k_coeffs[i];  // get fraction of 90 degree setting
                                f_offset = var_norm * q_rx_6k_coeffs_minus[i];  // get fraction of 89.5 degree setting
                                fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                            }
                        }   else
                                if(ts.filter_id <=AUDIO_7P5KHZ){
                                    for(i = 0; i < Q_NUM_TAPS; i++) {
                                        f_coeff = var_inv * q_rx_7k5_coeffs[i]; // get fraction of 90 degree setting
                                        f_offset = var_norm * q_rx_7k5_coeffs_minus[i]; // get fraction of 89.5 degree setting
                                        fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                                    }
                                }   else {
                                        for(i = 0; i < Q_NUM_TAPS; i++) {
                                            f_coeff = var_inv * q_rx_10k_coeffs[i]; // get fraction of 90 degree setting
                                            f_offset = var_norm * q_rx_10k_coeffs_minus[i]; // get fraction of 89.5 degree setting
                                            fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                                        }
                                    }
            } // end phase adjustment negative
            else    {                           // adjustment was positive
                if(ts.filter_id <=AUDIO_3P6KHZ){
                    for(i = 0; i < Q_NUM_TAPS; i++) {
                        f_coeff = var_inv * q_rx_3k6_coeffs[i]; // get fraction of 90 degree setting
                        f_offset = var_norm * q_rx_3k6_coeffs_plus[i];  // get fraction of 90.5 degree setting
                        fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                    }
                } else
                    if(ts.filter_id <=AUDIO_5P0KHZ){
                        for(i = 0; i < Q_NUM_TAPS; i++) {
                            f_coeff = var_inv * q_rx_5k_coeffs[i];  // get fraction of 90 degree setting
                            f_offset = var_norm * q_rx_5k_coeffs_plus[i];   // get fraction of 90.5 degree setting
                            fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                        }
                    } else
                        if(ts.filter_id <=AUDIO_6P0KHZ){
                            for(i = 0; i < Q_NUM_TAPS; i++) {
                                f_coeff = var_inv * q_rx_6k_coeffs[i];  // get fraction of 90 degree setting
                                f_offset = var_norm * q_rx_6k_coeffs_plus[i];   // get fraction of 90.5 degree setting
                                fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                            }
                        }   else
                                if(ts.filter_id <=AUDIO_7P5KHZ){
                                    for(i = 0; i < Q_NUM_TAPS; i++) {
                                        f_coeff = var_inv * q_rx_7k5_coeffs[i]; // get fraction of 90 degree setting
                                        f_offset = var_norm * q_rx_7k5_coeffs_plus[i];  // get fraction of 90.5 degree setting
                                        fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                                    }
                                }   else {
                                        for(i = 0; i < Q_NUM_TAPS; i++) {
                                            f_coeff = var_inv * q_rx_10k_coeffs[i]; // get fraction of 90 degree setting
                                            f_offset = var_norm * q_rx_10k_coeffs_plus[i];  // get fraction of 90.5 degree setting
                                            fc.rx_filt_q[i] = f_coeff + f_offset;   // synthesize new coefficient
                                        }
                                    }
            } // end phase adjustment positive
        } // end if phase adjustment non-zero

    //
    // In AM mode we do NOT do 90 degree phase shift, so we do FIR low-pass instead of Hilbert, setting "I" channel the same as "Q"
    if(ts.dmod_mode == DEMOD_AM)        // use "Q" filter settings in AM mode for "I" channel
        arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I,fc.rx_q_num_taps,(float32_t *)&fc.rx_filt_i[0], &FirState_I[0],fc.rx_q_block_size); // load "I" with "Q" coefficients
    else                                // not in AM mode, but SSB or FM - use normal settings where I and Q are 90 degrees apart
        arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I,fc.rx_i_num_taps,(float32_t *)&fc.rx_filt_i[0], &FirState_I[0],fc.rx_i_block_size); // load "I" with "I" coefficients
    //
    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_Q,fc.rx_q_num_taps,(float32_t *)&fc.rx_filt_q[0], &FirState_Q[0],fc.rx_q_block_size);     // load "Q" with "Q" coefficients
    //
}


/*
 * @brief Calculate TX FFT coeffients based on adjustment settings
 */
void AudioFilter_CalcTxPhaseAdj(void)
{
    float f_coeff, f_offset, var_norm, var_inv;
    ulong i;
    int phase;
    //
    ads.tx_filter_adjusting = 1;        // disable TX I/Q filter during adjustment
    //

    // always make a fresh copy of the original Q and I coefficients
    // NOTE:  We are assuming that the I and Q filters are of the same length!
    //
    fc.tx_q_num_taps = Q_TX_NUM_TAPS;
    fc.tx_i_num_taps = I_TX_NUM_TAPS;
    //
    fc.tx_q_block_size = Q_TX_BLOCK_SIZE;
    fc.tx_i_block_size = I_TX_BLOCK_SIZE;
    //
    for(i = 0; i < Q_TX_NUM_TAPS; i++)  {
        fc.tx_filt_q[i] = q_tx_coeffs[i];
        fc.tx_filt_i[i] = i_tx_coeffs[i];
    }
    //
    if(ts.dmod_mode == DEMOD_LSB)
        phase = ts.tx_iq_lsb_phase_balance;     // yes, get current gain adjustment setting for LSB
    else
        phase = ts.tx_iq_usb_phase_balance;     // yes, get current gain adjustment setting
    //
    if(phase != 0)  {   // is phase adjustment non-zero?
        var_norm = (float)phase;        // yes, get current gain adjustment setting
        var_norm = fabs(var_norm);      // get absolute value of this gain adjustment
        var_inv = 32 - var_norm;        // calculate "inverse" of number of steps
        var_norm /= 32;     // fractionalize by the number of steps
        var_inv /= 32;                      // fractionalize this one, too
        if(phase < 0)   {   // was the phase adjustment negative?
            for(i = 0; i < Q_TX_NUM_TAPS; i++)  {
                f_coeff = var_inv * q_tx_coeffs[i]; // get fraction of 90 degree setting
                f_offset = var_norm * q_tx_coeffs_minus[i];
                fc.tx_filt_q[i] = f_coeff + f_offset;
            }
        }
        else    {                           // adjustment was positive
            for(i = 0; i < Q_TX_NUM_TAPS; i++)  {
                f_coeff = var_inv * q_tx_coeffs[i]; // get fraction of 90 degree setting
                f_offset = var_norm * q_tx_coeffs_plus[i];
                fc.tx_filt_q[i] = f_coeff + f_offset;
            }
        }
    }
    //
    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I_TX,fc.tx_i_num_taps,(float32_t *)&fc.tx_filt_i[0], &FirState_I_TX[0],fc.tx_i_block_size);
    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_Q_TX,fc.tx_q_num_taps,(float32_t *)&fc.tx_filt_q[0], &FirState_Q_TX[0],fc.tx_q_block_size);

    ads.tx_filter_adjusting = 0;        // re enable TX I/Q filter now that we are done
}



