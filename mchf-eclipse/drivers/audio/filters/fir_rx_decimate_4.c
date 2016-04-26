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
**  License:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

#include "filters.h"

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

const arm_fir_decimate_instance_f32 FirRxDecimate =
{
    .numTaps = 4,
    .pCoeffs = (float*) (const float[])
    {
        -0.3105711170561,
        -0.2100034101786,
        -0.2099936469089,
        -0.3105557871709
    }
};
