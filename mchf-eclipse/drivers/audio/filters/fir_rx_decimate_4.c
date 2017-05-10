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
**  License:		GNU GPLv3                                                      **
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

// this is the decimation filter used for decimating the I and the Q path
// when in CW/SSB mode and filter BW <= 3k6
// 48ksps, 43 taps
// Fc=3.6kHz 0.1500
// Kaiser Beta 2.20, Raised Cosine 0.750

const arm_fir_decimate_instance_f32 FirRxDecimate_sideband_supp =
{
    .numTaps = 43,
    .pCoeffs = (float*) (const float[])
    {
        -596.2286796599541960E-6,-927.3655504344595780E-6,-940.6330523057968090E-6,-298.4871913266846950E-6, 0.001186311749146407, 0.003340717388857422, 0.005503534826465819, 0.006586352026602872, 0.005378691189561175, 0.001056745384724361,-0.006253091035763602,-0.015078013363837874,-0.022588392594615103,-0.025094197603292432,-0.018999101368626720,-0.001976514919694041, 0.025998282705991884, 0.062138450304126999, 0.101110564031515582, 0.136092352983227610, 0.160375863194488072, 0.169067382812500000, 0.160375863194488072, 0.136092352983227610, 0.101110564031515582, 0.062138450304126999, 0.025998282705991884,-0.001976514919694041,-0.018999101368626720,-0.025094197603292432,-0.022588392594615103,-0.015078013363837874,-0.006253091035763602, 0.001056745384724361, 0.005378691189561175, 0.006586352026602872, 0.005503534826465819, 0.003340717388857422, 0.001186311749146407,-298.4871913266846950E-6,-940.6330523057968090E-6,-927.3655504344595780E-6,-596.2286796599541960E-6
    }
};


const arm_fir_decimate_instance_f32 FirZoomFFTDecimate[] =
{
        {
                .numTaps = 4,
                .pCoeffs = (float*) (const float[])
                {
                    -0.3105711170561,
                    -0.2100034101786,
                    -0.2099936469089,
                    -0.3105557871709
                }
        },
        // 48ksps, 12kHz lowpass
        {
                .numTaps = 4,
                .pCoeffs = (float*) (const float[])
                {
                    475.1179397144384210E-6,
                    0.503905202786044337,
                    0.503905202786044337,
                    475.1179397144384210E-6
                }
        },
        // 48ksps, 6kHz lowpass
        {
                .numTaps = 4,
                .pCoeffs = (float*) (const float[])
                {
                    0.198273254218889416,
                    0.298085149879260325,
                    0.298085149879260325,
                    0.198273254218889416
                }
        },
        // 48ksps, 3kHz lowpass
        {
                .numTaps = 4,
                .pCoeffs = (float*) (const float[])
                {
                    0.199820836596682871,
                    0.272777397353925699,
                    0.272777397353925699,
                    0.199820836596682871
                }
        },
        // 48ksps, 1.5kHz lowpass
        {
                .numTaps = 4,
                .pCoeffs = (float*) (const float[])
                {
                    0.199820836596682871,
                    0.272777397353925699,
                    0.272777397353925699,
                    0.199820836596682871
                }
        },
        // 48ksps, 750Hz lowpass
        {
                .numTaps = 4,
                .pCoeffs = (float*) (const float[])
                {
                    0.199820836596682871,
                    0.272777397353925699,
                    0.272777397353925699,
                    0.199820836596682871
                }
        }
};



