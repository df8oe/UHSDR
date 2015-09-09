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
**  License:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __FIR_RX_DECIMATE_4_H
#define __FIR_RX_DECIMATE_4_H

/**************************************************************

KA7OEI, November, 2014

IMPORTANT NOTE:

This is a MINIMAL FIR Lowpass filter that is applied to the RX input
decimation.  It is NOT designed to provide any significant filtering!

The majority of anti-aliasing for this decimation operation is provided BY the
Phase-Added Hilbert Transformer that PRECEDES this!

This is an absolutely minimized operation to reduce processor overhead!

Response:

DC-2370 Hz:  >=-0.25dB
-1dB @ 3492 Hz
-3dB @ 5320 Hz
-6dB @ 6958 Hz
-10dB @ 8396 Hz
Notch @ 10767 Hz with -20dB points at 10000 and 11598 Hz.
Min. -7.6dB @ 16.4 kHz
-20dB > 22.9 kHz



***************************************************************/

#define	RX_DECIMATE_NUM_TAPS	4

const float FirRxDecimate[] =
{
		-0.3105711170561,
		-0.2100034101786,
		-0.2099936469089,
		-0.3105557871709
};

#endif
