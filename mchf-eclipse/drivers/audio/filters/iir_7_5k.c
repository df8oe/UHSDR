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
// alternative filter designed with MATLAB fdatools by DD4WH 2016-02-18
// 24k sampling rate, Lattice ARMA structure
// 8th order IIR Elliptic lowpass LPF
// Fpass 7500Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_7k5_numStages 8

const arm_iir_lattice_instance_f32 IIR_7k5_LPF =
{
    .numStages = IIR_7k5_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.171676312284558,
        -0.248915944482148,
        0.554808670292421,
        -0.0804708303821426,
        0.937673773552786,
        0.255249586500978,
        0.996592749183544,
        0.364810653255913
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0631810509418012,
        0.273620653809899,
        0.462876563831051,
        0.260420336947869,
        -0.123177317150119,
        -0.0730805396001466,
        0.0163120102308616,
        0.00402985085145935,
        0.00101856811686258
    }
};


