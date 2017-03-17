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
**  Licence:        GNU GPLv3                                                      **
************************************************************************************/
#include <iq_tx_filter.h>
#include "mchf_board.h"
#include "audio_filter.h"
#include "audio_driver.h"
#include "filters.h"

#include "arm_math.h"
#include "math.h"
#include "audio_driver.h"


// SSB Hilbert TX Filter
#include "iq_tx_filter.h"
//
static __IO    FilterCoeffs   __attribute__ ((section (".ccm")))     fc;

enum
{
    FILTER_MASK_CW = 1 << FILTER_MODE_CW,
    FILTER_MASK_SSB = 1 << FILTER_MODE_SSB,
    FILTER_MASK_AM = 1 << FILTER_MODE_AM,
    FILTER_MASK_FM = 1 << FILTER_MODE_FM,
    FILTER_MASK_SAM = 1 << FILTER_MODE_SAM,
};

#define FILTER_MASK_ALL (FILTER_MASK_CW|FILTER_MASK_SSB|FILTER_MASK_AM|FILTER_MASK_FM)
#define FILTER_MASK_NOFM (FILTER_MASK_CW|FILTER_MASK_SSB|FILTER_MASK_AM)
#define FILTER_MASK_SSBAM (FILTER_MASK_SSB|FILTER_MASK_AM)
#define FILTER_MASK_SSBAMCW (FILTER_MASK_SSB|FILTER_MASK_AM|FILTER_MASK_CW)
#define FILTER_MASK_SSBSAM (FILTER_MASK_SSB|FILTER_MASK_AM|FILTER_MASK_SAM)
#define FILTER_MASK_SSBSAMCW (FILTER_MASK_SSB|FILTER_MASK_AM|FILTER_MASK_SAM|FILTER_MASK_CW)
#define FILTER_MASK_AMSAM (FILTER_MASK_AM|FILTER_MASK_SAM)
#define FILTER_MASK_SSBCW (FILTER_MASK_SSB|FILTER_MASK_CW)
#define FILTER_MASK_AMFM (FILTER_MASK_AM|FILTER_MASK_FM)
#define FILTER_MASK_NONE (0)
#define FILTER_MASK_SSBAMFM (FILTER_MASK_SSB|FILTER_MASK_AM|FILTER_MASK_FM)

const FilterDescriptor FilterInfo[AUDIO_FILTER_NUM] =
{
    // 	id ,	name	 ,  width
    {  AUDIO_300HZ,  "300Hz",   300},
    {  AUDIO_500HZ,  "500Hz",   500},
    {  AUDIO_1P4KHZ, "1.4k",  1400},
    {  AUDIO_1P6KHZ, "1.6k",  1600},
    {  AUDIO_1P8KHZ, "1.8k",  1800},
    {  AUDIO_2P1KHZ, "2.1k",  2100},
    {  AUDIO_2P3KHZ, "2.3k",  2300},
    {  AUDIO_2P5KHZ, "2.5k",  2500},
    {  AUDIO_2P7KHZ, "2.7k",  2700},
    {  AUDIO_2P9KHZ, "2.9k",  2900},
    {  AUDIO_3P2KHZ, "3.2k",  3200},
    {  AUDIO_3P4KHZ, "3.4k",  3400},
    {  AUDIO_3P6KHZ, "3.6k",  3600},
    {  AUDIO_3P8KHZ, "3.8k",  3800},
    {  AUDIO_4P0KHZ, "4.0k",  4000},
    {  AUDIO_4P2KHZ, "4.2k",  4200},
    {  AUDIO_4P4KHZ, "4.4k",  4400},
    {  AUDIO_4P6KHZ, "4.6k",  4600},
    {  AUDIO_4P8KHZ, "4.8k",  4800},
    {  AUDIO_5P0KHZ, "5.0k",  5000},
    {  AUDIO_5P5KHZ, "5.5k",  5500},
    {  AUDIO_6P0KHZ, "6.0k",  6000},
    {  AUDIO_6P5KHZ, "6.5k",  6500},
    {  AUDIO_7P0KHZ, "7.0k",  7000},
    {  AUDIO_7P5KHZ, "7.5k",  7500},
    {  AUDIO_8P0KHZ, "8.0k",  8000},
    {  AUDIO_8P5KHZ, "8.5k",  8500},
    {  AUDIO_9P0KHZ, "9.0k",  9000},
    {  AUDIO_9P5KHZ, "9.5k",  9500},
    {  AUDIO_10P0KHZ,"10.0k", 10000}
};

uint16_t filterpath_mode_map[FILTER_MODE_MAX];

/*################################################################
 * FILTER PLAYGROUND for the brave mcHF owner
 *
 * FilterPathInfo
 *
 * put together your custom filters here
 * only add NEW filters at the END of the list in FilterPathInfo!!! (otherwise your filter settings will get mashed up completely in EEPROM)
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

/*
id --> ID for bandwidth

mode name --> for display

filter_select_ID

FIR coeff_I_numTaps (= FIR coeff_Q_numTaps)
FIR coeff_I_coeffs: points to the array of FIR filter coeffs used in the I path
FIR coeff_Q_coeffs: points to the array of FIR filter coeffs used in the Q path

&decimation filter instance
[dec- filter_numTaps: points to the array of FIR coeffs used in the decimation fliter
[dec- filter_coeffs

sample rate: gives the sample rate used after decimation (12ksps, 24ksps . . .)

&IIR_Pre_Filter instance
[IIR audio coeff_numStages: points to the array of IIR coeff used for the audio IIR filter
[IIR audio coeff_pk
[IIR audio coeff_pv

&FIR_interpolation filter instance
[int. filter coeffs_numTaps
[int. filter coeffs: points to the array of IIR coeffs for the antialias interpolation filter used by the ARM interpolation routine

&IIR interpolation filter instance
[IIR_antialias_numStages
[IIR_antialias_coeff_pk
[IIR_antialias_coeff_pv: points to the array of IIR coeffs for the antialias IIR filter (works at 48ksps)

centre frequency of the filterpath in Hz (for the graphical display of the bandwidth under spectrum display)

*/

const FilterPathDescriptor FilterPathInfo[AUDIO_FILTER_PATH_NUM] =
    //
{
// ID, mode name (for display), filter_select_ID, FIR_numTaps, FIR_I_coeff_file, FIR_Q_coeff_file, &decimation filter,
//		sample_rate_dec, &IIR_PreFilter,
//		&FIR_interpolaton filter, &IIR_interpolation filter, centre frequency of the filterpath (for graphical bandwidth display)
//
    // SPECIAL AUDIO_OFF Entry
    {
        AUDIO_OFF, "", FILTER_MASK_NONE, 0, 0, NULL, NULL, NULL,
        0, NULL,
        NULL, NULL
    },

//###################################################################################################################################
// FM filters
//	very special case, FM demodulation mainly in separate void, filterpath not defined in FilterPathInfo
//###################################################################################################################################
// 1
    {
        AUDIO_3P6KHZ, "FM", FILTER_MASK_FM, 1, I_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, NULL,
        RX_DECIMATION_RATE_48KHZ, NULL,
        NULL, NULL
    },

    {
        AUDIO_5P0KHZ, "FM", FILTER_MASK_FM, 1, I_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, NULL,
        RX_DECIMATION_RATE_48KHZ, NULL,
        NULL, NULL
    },

    {
//        AUDIO_6P0KHZ, "FM", FILTER_MASK_FM, 1, I_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, NULL,
	    AUDIO_6P0KHZ, "FM", FILTER_MASK_FM, 1, I_NUM_TAPS, iq_rx_am_6k_coeffs, iq_rx_am_6k_coeffs, NULL,
	    RX_DECIMATION_RATE_48KHZ, NULL,
        NULL, NULL
    },

//###################################################################################################################################
// CW & SSB filters:
//###################################################################################################################################

    // 10 filters � 300Hz
// 4
    {
        AUDIO_300HZ, "500Hz", FILTER_MASK_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_500,
        &FirRxInterpolate, NULL, 500
    },

    {
        AUDIO_300HZ, "550Hz", FILTER_MASK_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_550,
        &FirRxInterpolate, NULL, 550
    },

    {
        AUDIO_300HZ, "600Hz", FILTER_MASK_SSBCW, 3, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_600,
        &FirRxInterpolate, NULL, 600
    },

    {
        AUDIO_300HZ, "650Hz", FILTER_MASK_SSBCW, 4, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_650,
        &FirRxInterpolate, NULL, 650
    },

    {
        AUDIO_300HZ, "700Hz", FILTER_MASK_SSBCW, 5, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_700,
        &FirRxInterpolate, NULL, 700
    },

    {
        AUDIO_300HZ, "750Hz", FILTER_MASK_SSBCW, 6, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_750,
        &FirRxInterpolate, NULL, 750
    },
//10
    {
        AUDIO_300HZ, "800Hz", FILTER_MASK_SSBCW, 7, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_800,
        &FirRxInterpolate, NULL, 800
    },

    {
        AUDIO_300HZ, "850Hz", FILTER_MASK_SSBCW, 8, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_850,
        &FirRxInterpolate, NULL, 850
    },

    {
        AUDIO_300HZ, "900Hz", FILTER_MASK_SSBCW, 9, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_900,
        &FirRxInterpolate, NULL, 900
    },

    {
        AUDIO_300HZ, "950Hz", FILTER_MASK_SSBCW, 10, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_300hz_950,
        &FirRxInterpolate, NULL, 950
    },

    // 5 filters � 500Hz
    {
        AUDIO_500HZ, "550Hz", FILTER_MASK_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_500hz_550,
        &FirRxInterpolate, NULL, 550
    },
//15
    {
        AUDIO_500HZ, "650Hz", FILTER_MASK_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_500hz_650,
        &FirRxInterpolate, NULL, 650
    },

    {
        AUDIO_500HZ, "750Hz", FILTER_MASK_SSBCW, 3, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_500hz_750,
        &FirRxInterpolate, NULL, 750
    },

    {
        AUDIO_500HZ, "850Hz", FILTER_MASK_SSBCW, 4, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_500hz_850,
        &FirRxInterpolate, NULL, 850
    },

    {
        AUDIO_500HZ, "950Hz", FILTER_MASK_SSBCW, 5, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_500hz_950,
        &FirRxInterpolate, NULL, 950
    },
// 19
    {
        AUDIO_1P4KHZ, "LPF", FILTER_MASK_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k4_LPF,
        &FirRxInterpolate, NULL
    },
//20
    {
        AUDIO_1P4KHZ, "BPF", FILTER_MASK_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k4_BPF,
        &FirRxInterpolate, NULL, 775
    },

    {
        AUDIO_1P6KHZ, "LPF", FILTER_MASK_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k6_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_1P6KHZ, "BPF", FILTER_MASK_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k6_BPF,
        &FirRxInterpolate, NULL, 875
    },

    {
        AUDIO_1P8KHZ, "1.1k", FILTER_MASK_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k8_1k125,
        &FirRxInterpolate, NULL, 1125
    },

    {
        AUDIO_1P8KHZ, "1.3k", FILTER_MASK_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k8_1k275,
        &FirRxInterpolate, NULL, 1275
    },
//25
    {
        AUDIO_1P8KHZ, "1.4k", FILTER_MASK_SSBCW, 3, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k8_1k425,
        &FirRxInterpolate, NULL, 1425
    },

    {
        AUDIO_1P8KHZ, "1.6k", FILTER_MASK_SSBCW, 4, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k8_1k575,
        &FirRxInterpolate, NULL, 1575
    },

    {
        AUDIO_1P8KHZ, "1.7k", FILTER_MASK_SSBCW, 5, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k8_1k725,
        &FirRxInterpolate, NULL, 1725
    },

    {
        AUDIO_1P8KHZ, "LPF", FILTER_MASK_SSBCW, 6, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k8_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P1KHZ, "LPF", FILTER_MASK_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k1_LPF,
        &FirRxInterpolate, NULL
    },
//30
    {
        AUDIO_2P1KHZ, "BPF", FILTER_MASK_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k1_BPF,
        &FirRxInterpolate, NULL, 1125
    },

    {
        AUDIO_2P3KHZ, "1.3k", FILTER_MASK_SSBCW, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k3_1k275,
        &FirRxInterpolate, NULL, 1275
    },

    {
        AUDIO_2P3KHZ, "1.4k", FILTER_MASK_SSBCW, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k3_1k412,
        &FirRxInterpolate, NULL, 1412
    },

    {
        AUDIO_2P3KHZ, "1.6k", FILTER_MASK_SSBCW, 3, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k3_1k562,
        &FirRxInterpolate, NULL, 1562
    },

    {
        AUDIO_2P3KHZ, "1.7k", FILTER_MASK_SSBCW, 4, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k3_1k712,
        &FirRxInterpolate, NULL, 1712
    },
//35
    {
        AUDIO_2P3KHZ, "LPF", FILTER_MASK_SSBCW, 5, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k3_LPF,
        &FirRxInterpolate, NULL
    },

//###################################################################################################################################
// SSB only filters:
//###################################################################################################################################

    {
        AUDIO_2P5KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k5_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P5KHZ, "BPF", FILTER_MASK_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k5_BPF,
        &FirRxInterpolate, NULL, 1325
    },

    {
        AUDIO_2P7KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k7_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P7KHZ, "BPF", FILTER_MASK_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k7_BPF,
        &FirRxInterpolate, NULL, 1425
    },
//40
    {
        AUDIO_2P9KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k9_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P9KHZ, "BPF", FILTER_MASK_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k9_BPF,
        &FirRxInterpolate, NULL, 1525
    },

    {
        AUDIO_3P2KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k2_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_3P2KHZ, "BPF", FILTER_MASK_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k2_BPF,
        &FirRxInterpolate, NULL, 1675
    },

    // in filters from 3k4 on, the FIR interpolate is 4 taps and an additional IIR interpolation filter
//44	// is switched in to accurately prevent alias frequencies
    {
        AUDIO_3P4KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k4_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },
//45
    {
        AUDIO_3P4KHZ, "BPF", FILTER_MASK_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k4_BPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k, 1775
    },

    {
        AUDIO_3P6KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k6_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_3P6KHZ, "BPF", FILTER_MASK_SSB, 2, I_NUM_TAPS, i_rx_3k6_coeffs, q_rx_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k6_BPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k, 1875
    },

    {
        AUDIO_3P8KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_4k5_coeffs, q_rx_4k5_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k8_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_3P8KHZ, "BPF", FILTER_MASK_SSB, 2, I_NUM_TAPS, i_rx_4k5_coeffs, q_rx_4k5_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k8_BPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k, 1975
    },
//50
    {
        AUDIO_4P0KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_4k5_coeffs, q_rx_4k5_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P2KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_4k5_coeffs, q_rx_4k5_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k2_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P4KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_4k5_coeffs, q_rx_4k5_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k4_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P6KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_4k5_coeffs, q_rx_4k5_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k6_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P8KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_4k5_coeffs, q_rx_4k5_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k8_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

//55		// new decimation rate, new decimation filter, new interpolation filter, no IIR Prefilter, no IIR interpolation filter
    {
        AUDIO_5P0KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_5P5KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_5k_coeffs, q_rx_5k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_6P0KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_6k_coeffs, q_rx_6k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_6P5KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_6k_coeffs, q_rx_6k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_7P0KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_6k_coeffs, q_rx_6k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate10KHZ, NULL
    },
//60
    {
        AUDIO_7P5KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_7k5_coeffs, q_rx_7k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate10KHZ, NULL
    },
    // additional IIR interpolation filter
    {
        AUDIO_8P0KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_8k
    },

    {
        AUDIO_8P5KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_8k5
    },

    {
        AUDIO_9P0KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_9k
    },

    {
        AUDIO_9P5KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_9k5
    },

    {
        AUDIO_10P0KHZ, "LPF", FILTER_MASK_SSB, 1, I_NUM_TAPS, i_rx_10k_coeffs, q_rx_10k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_10k
    },

    //###################################################################################################################################
    // These are the "new" AM/SAM filters, January 2017
    // designed for an IIR lowpass stopband frequency that is exactly the same as the filter bandwidth
    // In sideband-selected SAM, there is no FIR filter, so the IIR has to do all the work
    // Let�s try them
    //###################################################################################################################################

    {
        AUDIO_1P4KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k4_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_1P6KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k6_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_1P8KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_1k8_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P1KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k1_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P3KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k3_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P5KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k5_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P7KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k7_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P9KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k9_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_3P2KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k2_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_3P4KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k4_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_3P6KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k6_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_3P8KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k8_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P0KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P2KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k2_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P4KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k4_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P6KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k6_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_4P8KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k8_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_5P0KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_5k_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_6P0KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_6k_coeffs, iq_rx_am_6k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_6k_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_7P5KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_7k5_coeffs, iq_rx_am_7k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_7k5_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_10P0KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_10k_coeffs, iq_rx_am_10k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_10k_LPF,
        &FirRxInterpolate_4_10k, &IIR_aa_10k
    },


/*
//###################################################################################################################################
// AM filters: designed for an IIR lowpass stopband frequency that is approx. 1.8 times higher than the FIR bandwidth
//  --> an AM 2.5kHz filter has 2.5kHz of audio when centre tuned, but can have up to 1.8 * 2.5kHz = 4.6kHz of bandwidth (IIR filter!)
//      when tuned away from the carrier, this has been called "sideband-selected AM demodulation" . . . wow . . .
//###################################################################################################################################

     // in AM, we ALWAYS use the lowpass filter version of the IIR audio PreFilter, regardless of the selected filter_select_ID.
    // this is because we assume AM mode to be used to demodulate DSB signals, so BPF (sideband suppression) is not necessary

    {
        AUDIO_1P4KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k3_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_1P6KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_2k9_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_1P8KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k2_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P1KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k6_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_2P3KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k2_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_2P5KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k6_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_2P7KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k8_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_2P9KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_5k5_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_3P2KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_6k_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_3P4KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_6k5_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_3P6KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_7k_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_3P8KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_7k_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_4P0KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_7k5_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_4P2KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, &IIR_8k_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    // from 4.4kHz on, the AM filter has no more IIR PreFilter (at 24ksps sample rate), BUT we add IIR filtering after interpolation (at 48 ksps)!
//80
    {
        AUDIO_4P4KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_8k
    },

    {
        AUDIO_4P6KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_8k5
    },

    {
        AUDIO_4P8KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_9k
    },

    {
        AUDIO_5P0KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_9k5
    },

    // from 6kHz on, we have no PreFilter, an IIR interpolation filter of 10k and only change the FIR filters bandwidths
    // remember that the AM 5k filter is capable of up to 10kHz bandwidth, if you offtune the AM carrier
    // . . . same for 6k = 12kHz bw, 7k5 = 15kHz bw, 10kHz = max of 20kHz bandwidth

    {
        AUDIO_6P0KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_6k_coeffs, iq_rx_am_6k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_10k
    },

    {
        AUDIO_7P5KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_7k5_coeffs, iq_rx_am_7k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_10k
    },

    {
        AUDIO_10P0KHZ, "AM/SAM", FILTER_MASK_AMSAM, 1, Q_NUM_TAPS, iq_rx_am_10k_coeffs, iq_rx_am_10k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_10k
    },


*/




//###################################################################################################################################
// SAM filters:
// they now use the same filter paths as the AM filters
// so these definitions are obsolete from 2017 on
//###################################################################################################################################
/*

    {
        AUDIO_1P8KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_2k3_coeffs, iq_rx_am_2k3_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_3k2_LPF,
        &FirRxInterpolate, NULL
    },

    {
        AUDIO_2P3KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_12KHZ, &IIR_4k2_LPF,
        &FirRxInterpolate_4_5k, &IIR_aa_5k
    },

    {
        AUDIO_2P9KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_24KHZ, &IIR_5k5_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_3P4KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_3k6_coeffs, iq_rx_am_3k6_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_24KHZ, &IIR_6k5_LPF,
        &FirRxInterpolate10KHZ, NULL
    },
    // old remark, must be analysed again
    // measurements with Spectrum Lab have shown that there was considerable, but only barely
	// audible alias noise in the SAM 4k2 and 4k8 filters (25-30dB below signal, you could hear it clearly when treble gain was 20dB)
	// now I have implemented the IIR_aa_5k antialiasing filter in SAM 4k2 and 4k8 filters and all the aliases
	// are down by at least 60dB below signal level
    {
        AUDIO_4P2KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_4k5_coeffs, iq_rx_am_4k5_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_24KHZ, &IIR_8k_LPF,
        &FirRxInterpolate10KHZ, NULL
    },

    {
        AUDIO_4P8KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_5k_coeffs, iq_rx_am_5k_coeffs, &FirRxDecimate,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_9k
    },

    {
        AUDIO_6P0KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_6k_coeffs, iq_rx_am_6k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_10k
    },

    {
        AUDIO_7P5KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_7k5_coeffs, iq_rx_am_7k5_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_10k
    },

    {
        AUDIO_10P0KHZ, "SAM", FILTER_MASK_SAM, 1, I_NUM_TAPS, iq_rx_am_10k_coeffs, iq_rx_am_10k_coeffs, &FirRxDecimateMinLPF,
        RX_DECIMATION_RATE_24KHZ, NULL,
        &FirRxInterpolate_4_10k, &IIR_aa_10k
    }
*/
}; // end FilterPath

/*
 * @brief Converts demodulation mode (DEMOD_CW ... ) to FILTER_MODE_CW ...
 * @param demodulation mode (DEMOD_CW ... DEMOD_SAM)
 * @returns filter mode for given demodulation (FILTER_MODE_CW / _SSB, ... )
 */
uint16_t AudioFilter_GetFilterModeFromDemodMode(const uint8_t dmod_mode)
{
    uint16_t filter_mode;
    switch(dmod_mode)
    {
    case DEMOD_AM:
        filter_mode = FILTER_MODE_AM;
        break;
    case DEMOD_FM:
        filter_mode = FILTER_MODE_FM;
        break;
    case DEMOD_CW:
        filter_mode = FILTER_MODE_CW;
        break;
    case DEMOD_SAM:
        //        filter_mode = FILTER_MODE_SAM;
        // from 2017 on, the new SAM mode uses the AM filters!
        filter_mode = FILTER_MODE_AM;
        break;
    // case DEMOD_LSB:
    // case DEMOD_USB:
    // case DEMOD_DIGI:
    default:
        filter_mode = FILTER_MODE_SSB;
    }
    return filter_mode;
}

static bool AudioFilter_IsInPathMemory(uint16_t filter_mode, uint16_t filter_path)
{
    uint16_t idx;
    bool retval = false;
    for (idx = 1; idx < FILTER_PATH_MEM_MAX; idx++)
    {
        if (ts.filter_path_mem[filter_mode][idx] == filter_path)
        {
            retval = true;
            break;
        }
        // leave loop if match found
    }
    return retval;
}

bool AudioFilter_IsApplicableFilterPath(const uint16_t query, const uint16_t filter_mode, const uint8_t filter_path)
{
    bool retval = false;
    uint16_t filter_mode_mask = 1<<filter_mode;

    // these rules handle special cases
    // if((FilterPathInfo[idx].id == AUDIO_1P8KHZ) && ((ts.filter_cw_wide_disable) && (current_mode == DEMOD_CW))) { idx = AUDIO_300HZ; break; }
    // in this case, next applicable mode is 300 Hz, so selected and leave loop
    if ((FilterPathInfo[filter_path].mode & filter_mode_mask) != 0)
    {
        if ((query & PATH_USE_RULES) != 0)
        {
            // we have to check if this mode is IN the list of modes always offered in this mode, regardless of enablement
            /*if ((FilterInfo[FilterPathInfo[filter_path].id].always_on_modes & filter_mode_mask) != 0) {
              // okay, applicable, leave loop
              retval = true;
            } else*/ if (AudioFilter_IsInPathMemory(filter_mode,filter_path))
            {
                retval = true;
//      } else if (!ts.filter_select[FilterPathInfo[filter_path].id]) {
//        retval = false;
//      } else if((FilterPathInfo[filter_path].id == AUDIO_300HZ || FilterPathInfo[filter_path].id == AUDIO_500HZ) && ((ts.filter_ssb_narrow_disable) && (filter_mode_mask != FILTER_MASK_CW))) {
//        // jump over 300 Hz / 500 Hz if ssb_narrow_disable and voice mode
//        retval = false;
            }
            else     // no match in rules, not applicable
            {
                retval = false;
            }
        }
        else
        {
            retval = true;
        }

    }
    return retval;
}

/*
 * @brief Find Next Applicable Filter Path based on the information in the filter data structure
 *
 * Takes into account the current mode (SSB, CW, ..., some special rules, etc,). It will wrap around and always return a
 * valid filter id. In case not applicable filter was found, it returns the currently selected filter_id
 *
 * @param query specifies which selection approach is used: ALL_APPLICABLE_PATHS, NEXT_BANDWIDTH, SAME_BANDWIDTH
 * @param current_mode -> all values allowed for FILTER_MODE_xxx
 * @param current_path -> a valid filter path id, which is used as starting point
 *        use ts.filter_path unless in special cases (e.g. use for filter selection menus)
 * @returns next applicable filter id
 */


uint8_t AudioFilter_NextApplicableFilterPath(const uint16_t query, const uint16_t filter_mode, const uint8_t current_path)
{

    uint8_t last_bandwidth_id = FilterPathInfo[current_path].id;
    int idx = 0;

    if ((query & PATH_LAST_USED_IN_MODE) != 0 &&  ts.filter_path_mem[filter_mode][0] != 0)
    {
        idx = ts.filter_path_mem[filter_mode][0];
    }
    else
    {
        // we run through all audio filters, starting with the next following, making sure to wrap around
        // we leave this loop once we found a filter that is applicable using "break"
        // or skip to next filter to check using "continue"
        for (idx = current_path+((query&PATH_DOWN)?-1:1); idx != current_path;
                idx+=(query&PATH_DOWN)?-1:1)
        {
            idx %= AUDIO_FILTER_PATH_NUM;
            if (idx<0)
            {
                idx+=AUDIO_FILTER_PATH_NUM;
            }

            // skip over all filters of current bandwidth
            if (((query & PATH_NEXT_BANDWIDTH) != 0) && (last_bandwidth_id == FilterPathInfo[idx].id))
            {
                continue;
            }
            // skip over all filters of different bandwidth
            if (((query & PATH_SAME_BANDWITH) != 0) && (last_bandwidth_id != FilterPathInfo[idx].id))
            {
                continue;
            }

            if (AudioFilter_IsApplicableFilterPath(query, filter_mode, idx))
            {
                break;
            }
        }
    }
    if ((query&PATH_DONT_STORE) == 0)
    {
        ts.filter_path_mem[filter_mode][0] = idx;
    }
    return  idx;
}



//
// RX Hilbert transform (90 degree) FIR filter state tables and instances
arm_fir_instance_f32    FIR_I;
arm_fir_instance_f32    FIR_Q;

static float32_t    __attribute__ ((section (".ccm")))    FirState_I[FIR_RXAUDIO_BLOCK_SIZE+Q_NUM_TAPS];
static float32_t    __attribute__ ((section (".ccm")))    FirState_Q[FIR_RXAUDIO_BLOCK_SIZE+Q_NUM_TAPS];

//
// TX Hilbert transform (90 degree) FIR filter state tables and instances
//
// FIXME: I think this is to short should calculated from max blocksize
// (BUFLEN / 4 -> Each Interrupt == BUFLEN /2, each filter gets half of the samples, i.e. one audio channel) + max numTaps
static float   __attribute__ ((section (".ccm")))         FirState_I_TX[IQ_TX_NUM_TAPS_MAX+IQ_BUFSZ];
static float   __attribute__ ((section (".ccm")))         FirState_Q_TX[IQ_TX_NUM_TAPS_MAX+IQ_BUFSZ];

arm_fir_instance_f32    FIR_I_TX;
arm_fir_instance_f32    FIR_Q_TX;


static float   FirState_I_FREEDV[60+IQ_BUFSZ];
static float   FirState_Q_FREEDV[60+IQ_BUFSZ];
arm_fir_instance_f32    FIR_I_FREEDV;
arm_fir_instance_f32    FIR_Q_FREEDV;

// Audio RX - Decimator (numTaps+blockSize-1)
arm_fir_decimate_instance_f32   DECIMATE_SAM_I;
static float32_t           __attribute__ ((section (".ccm"))) decimSAMIState[I_BLOCK_SIZE + I_NUM_TAPS];

// Audio RX - Decimator
arm_fir_decimate_instance_f32   DECIMATE_SAM_Q;
static float32_t           __attribute__ ((section (".ccm"))) decimSAMQState[Q_BLOCK_SIZE + Q_NUM_TAPS];



/*
 * @brief Initialize RX Hilbert filters
 */
void 	AudioFilter_InitRxHilbertFIR(uint8_t dmod_mode)
{
    ulong i;

    // always make a fresh copy of the original Q and I coefficients
    // NOTE:  We are assuming that the I and Q filters are of the same length!

    // new filter_path method
    // take all info from FilterPathInfo
    // phase adjustment is now done in audio_driver.c audio_rx_processor
    //
    fc.rx_q_num_taps = FilterPathInfo[ts.filter_path].FIR_numTaps;
    fc.rx_i_num_taps = FilterPathInfo[ts.filter_path].FIR_numTaps;
    //
    fc.rx_q_block_size = Q_BLOCK_SIZE;
    fc.rx_i_block_size = I_BLOCK_SIZE;
    // in FilterPathInfo, we have stored the coefficients already, so no if . . . necessary
    // also applicable for FM case !
    for(i = 0; i < fc.rx_q_num_taps; i++)   // fc.rx_q_num_taps is ALWAYS == fc.rx_i_num_taps
    {
        fc.rx_filt_i[i] = FilterPathInfo[ts.filter_path].FIR_I_coeff_file[i];
        fc.rx_filt_q[i] = FilterPathInfo[ts.filter_path].FIR_Q_coeff_file[i];
    }

    // Initialization of the FIR/Hilbert filters
    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I,fc.rx_i_num_taps,(float32_t *)&fc.rx_filt_i[0], &FirState_I[0],fc.rx_i_block_size); // load "I" with "I" coefficients
    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_Q,fc.rx_q_num_taps,(float32_t *)&fc.rx_filt_q[0], &FirState_Q[0],fc.rx_q_block_size);     // load "Q" with "Q" coefficients
    //
    // Set up RX SAM decimation/filter
    if (dmod_mode == DEMOD_SAM || dmod_mode == DEMOD_AM)
    {
        if (FilterPathInfo[ts.filter_path].FIR_numTaps != 0)
        {
            DECIMATE_SAM_I.numTaps = FilterPathInfo[ts.filter_path].FIR_numTaps;      // Number of taps in FIR filter
            DECIMATE_SAM_Q.numTaps = FilterPathInfo[ts.filter_path].FIR_numTaps;      // Number of taps in FIR filter
            DECIMATE_SAM_I.pCoeffs = (float32_t *)&fc.rx_filt_i[0]; //FilterPathInfo[ts.filter_path].FIR_I_coeff_file;       // Filter coefficients
            DECIMATE_SAM_Q.pCoeffs = (float32_t *)&fc.rx_filt_q[0]; //FilterPathInfo[ts.filter_path].FIR_Q_coeff_file;       // Filter coefficients
        }
        else
        {
            DECIMATE_SAM_I.numTaps = 0;
            DECIMATE_SAM_Q.numTaps = 0;
            DECIMATE_SAM_I.pCoeffs = NULL;
            DECIMATE_SAM_Q.pCoeffs = NULL;
        }
        DECIMATE_SAM_I.M = ads.decimation_rate;
        DECIMATE_SAM_Q.M = ads.decimation_rate;
        DECIMATE_SAM_I.pState = decimSAMIState;            // Filter state variables
        DECIMATE_SAM_Q.pState = decimSAMQState;
        arm_fill_f32(0.0,decimSAMIState, I_BLOCK_SIZE + I_NUM_TAPS);
        arm_fill_f32(0.0,decimSAMQState, Q_BLOCK_SIZE + Q_NUM_TAPS);
    }

}


/*
 * @brief Initialize TX Hilbert filters
 */
void AudioFilter_InitTxHilbertFIR(void)
{

    ulong i;
    //
    ads.tx_filter_adjusting = 1;        // disable TX I/Q filter during adjustment
    //

    // always make a fresh copy of the original Q and I coefficients
    // NOTE:  We are assuming that the I and Q filters are of the same length!
    //
    // phase adjustment is now done in audio_driver.c audio_tx_processor

    IQ_FilterDescriptor iq_tx_filter =
            (ts.tx_filter == TX_FILTER_BASS ||
            		//             ts.tx_filter == TX_FILTER_WIDE_TREBLE) ? iq_tx_wide:iq_tx_narrow;
            		// FIXME: dirty trial: always use the "wide" 201 tap Hilbert filter
             ts.tx_filter == TX_FILTER_SOPRANO) ? iq_tx_wide:iq_tx_wide;


    fc.tx_q_num_taps = iq_tx_filter.num_taps;
    fc.tx_i_num_taps = iq_tx_filter.num_taps;

    fc.tx_q_block_size = IQ_TX_BLOCK_SIZE;
    fc.tx_i_block_size = IQ_TX_BLOCK_SIZE;

    for(i = 0; i < iq_tx_filter.num_taps; i++)
    {
        fc.tx_filt_q[i] = iq_tx_filter.q[i];
        fc.tx_filt_i[i] = iq_tx_filter.i[i];
    }

    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I_TX,fc.tx_i_num_taps,(float32_t *)&fc.tx_filt_i[0], &FirState_I_TX[0],fc.tx_i_block_size);
    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_Q_TX,fc.tx_q_num_taps,(float32_t *)&fc.tx_filt_q[0], &FirState_Q_TX[0],fc.tx_q_block_size);

    fc.freedv_i_num_taps = FirFreeDVInterpolate.numTaps;
    fc.freedv_q_num_taps = FirFreeDVInterpolate.numTaps;
    fc.freedv_i_block_size = IQ_TX_BLOCK_SIZE;
    fc.freedv_q_block_size = IQ_TX_BLOCK_SIZE;

    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I_FREEDV,fc.freedv_i_num_taps,FirFreeDVInterpolate.pCoeffs, &FirState_I_FREEDV[0],fc.freedv_i_block_size);
    arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_Q_FREEDV,fc.freedv_q_num_taps,FirFreeDVInterpolate.pCoeffs, &FirState_Q_FREEDV[0],fc.freedv_q_block_size);


    ads.tx_filter_adjusting = 0;        // re enable TX I/Q filter now that we are done
}

void AudioFilter_GetNamesOfFilterPath(uint16_t filter_path,const char** filter_names)
{

    const FilterPathDescriptor *path = &FilterPathInfo[filter_path];
    const FilterDescriptor    *filter = &FilterInfo[path->id];

    filter_names[0] = filter->name;
    filter_names[1] = path->name;
}

void AudioFilter_SetDefaultMemories()
{
    // filter selection for FM is hardcoded
    ts.filter_path_mem[FILTER_MODE_FM][1] = 1;
    ts.filter_path_mem[FILTER_MODE_FM][2] = 2;
    ts.filter_path_mem[FILTER_MODE_FM][3] = 3;
}





