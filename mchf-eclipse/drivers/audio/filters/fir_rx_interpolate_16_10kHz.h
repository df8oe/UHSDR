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

#ifndef __FIR_RX_INTERPOLATE_16_10KHZ_H
#define __FIR_RX_INTERPOLATE_16_10KHZ_H

/**************************************************************

KA7OEI, November, 2014

IMPORTANT NOTE:

This low-pass filter is designed to remove aliasing artifacts from the interpolated output.

Response (15th order FIR, Least Pth-norm:  Fpass = 10000 Hz, Fstop = 1200 Hz Fsamp = 24000 Hz):

	Ripple:  +/- 0.5dB
	-6dB:  10640 Hz
	-10dB: 10873 Hz
	-20dB: 11240 Hz
	-30dB: 11449 Hz
	Notch at 11608 Hz
	-34dB peak > 12000 (Ripple response at this level)

This low-pass filter is a special case for the 10 kHz passband.  Due to the limited order, the ultimate response at Nyquist is
only on the order of -34dB, but this is suitable to avoid audible aliasing artifacts because of phsycoacoustic masking effects.

***************************************************************/

#define	RX_INTERPOLATE_10KHZ_NUM_TAPS	16

const float FirRxInterpolate10KHZ[] =
{
		0.05811583205356,
		-0.0367094411696,
		0.01940709235172,
		-0.02834514773976,
		-0.01544119658805,
		0.00554906655524,
		-0.08598762701749,
		0.04147536848705,
		-0.165163226107,
		0.01287481670083,
		-0.1999399439611,
		-0.1925399994825,
		-0.1329419101037,
		-0.7311312926885,
		0.05542105288489,
		0.4430274622578
};

#endif
