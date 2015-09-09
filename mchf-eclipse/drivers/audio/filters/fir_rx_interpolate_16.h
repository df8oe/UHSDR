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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __FIR_RX_INTERPOLATE_16_H
#define __FIR_RX_INTERPOLATE_16_H

/**************************************************************

KA7OEI, November, 2014

IMPORTANT NOTE:

This low-pass filter is designed to remove aliasing artifacts from the interpolated output.

Response (15th order FIR, Least Pth-norm:  Fpass = 4000 Hz, Fstop = 8600 Hz Fsamp = 12000 Hz):

	-6dB:  5075 Hz
	-10dB: 5841 Hz
	-20dB: 7034 Hz
	-40dB: 8270 Hz
	-50dB:  >8550 Hz

Because the Nyquist Frequency is 6 kHz with the 12 kHz input sample rate, with filtered
audio limited to <3 kHz by the normal SSB audio filters, there will be NO aliased
content <9 kHz so the response of this low-pass filter may be relaxed to provide
a higher degree of filtering (e.g. >50dB) >8550 Hz.

In the case of the 3.6 kHz filter, the aliasing extends down no farther than 8200 Hz where the
attenuation is still at least 40dB where this, plus the effects of psychoacoustic masking
should make these artifacts inaudible.

***************************************************************/

#define	RX_INTERPOLATE_NUM_TAPS	16

const float FirRxInterpolate[] =
{
		 0.0064096284297950057,
		 0.017769515865099398,
		 0.032257990537557804,
		 0.040774242726362854,
		 0.031402652968355385,
		-0.0044545611189248744,
		-0.065457689067971975,
		-0.13802671944933273,
		-0.20064935251937976,
		-0.23328570256042488,
		-0.22670802442881149,
		-0.18633542013951426,
		-0.1286861967088595,
		-0.072726968058963559,
		-0.031602889405696735,
		-0.009023372039440496
};

#endif
