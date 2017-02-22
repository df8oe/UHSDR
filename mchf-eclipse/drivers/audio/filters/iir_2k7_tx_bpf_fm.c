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
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#include "filters.h"
//
// Filter designed 20151024 by C. Turner, KA7OEI using MatLAB fdatools
//
// This filter is designed for FM voice transmission, removing low-frequency audio
//
// NOTE:
//	- IIR structure is Lattice Autoregressive Moving-Average (ARMA)
//	- ARM FIR/IIR algorithms require time reverse-order coefficients!!!
//
//	10th order Elliptic bandpass filter
//	Fpass:  225, 2700 Hz
//
//  Amplitude responses - referenced to attenuation at top
//  Filter has 0 to -2 dB ripple, below referenced to "0" dB point:
//
//	-6dB points:  216, 2786 Hz
//	-10dB points:  210, 2865 Hz
//	-20dB points:  193, 3216 Hz
//	-30dB points:  172, 3478 Hz
//  -50dB points:  136, 4436 Hz
//
//

#define IIR_TX_2k7_FM_numStages 10
const arm_iir_lattice_instance_f32 IIR_TX_2k7_FM =
{
    .numStages = IIR_TX_2k7_FM_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.795992813347,
        -0.9682173169759,
        0.9817510589125,
        -0.9814547662913,
        0.9796499689423,
        -0.9856718628494,
        0.9982624972485,
        -0.9990433054892,
        0.9999481889194,
        -0.9995181953945
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.000247932512139,
        -0.000812969420294,
        -0.001204931403655,
        -0.001166250382325,
        -0.0005214249315797,
        -0.0001861038674673,
        2.552768869046e-05,
        1.619179380559e-06,
        -1.484109718886e-08,
        -3.31354298817e-10,
        1.62095434731e-12
    }
};

