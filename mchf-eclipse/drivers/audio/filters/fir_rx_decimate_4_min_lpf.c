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

#include "filters.h"

/**************************************************************

KA7OEI, November, 2014

IMPORTANT NOTE:

This is a MINIMAL FIR Lowpass filter that is applied to the RX input
decimation.  It is NOT designed to provide any significant filtering!

The majority anti-aliasing for this decimation operation is provided ONLY by the
Phase-Added Hilbert Transformer that precedes this!

This is an absolutely minimized operation to reduce processor overhead!

3rd order, Least Pth-Norm, 48 ksps, Fpass=10000, Fstop=15000, Wpass=10, Wstop=1

Response:

DC-10330 Hz:  >=-0.5dB
-1dB @ 10900 Hz Hz
-3dB @ 13089 Hz
-6dB @ 14900 Hz
-10dB @ 16700 Hz
Notch @ 19590 Hz with -20dB points at 18510 and 20880 Hz.
Min. -13.8dB @ 23.6 kHz

***************************************************************/

const arm_fir_decimate_instance_f32 FirRxDecimateMinLPF = {
  .numTaps = 4,

  .pCoeffs = (float*) (const float[]) 
{
		-0.1848172721704,
		0.135814345129,
		0.5586516940301,
		0.4419794765896
}
};

