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

// filter designed with MATLAB fdatools by DD4WH 2016-02-03
// initial idea for these filters: Clint KA7OEI
// 12k sampling rate, Lattice ARMA structure
// 10th order IIR Elliptic lowpass
// Fpass 4800Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB

#define IIR_4k8_numStages 10
const arm_iir_lattice_instance_f32 IIR_4k8_LPF =
{
    .numStages = IIR_4k8_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.153116856877746,
        0.180701877352361,
        0.748242393147457,
        0.672766998956057,
        0.976991110647331,
        0.763856545049567,
        0.997879618771176,
        0.793537908808440,
        0.999852510953044,
        0.806593649345542
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.213846255224293,
        0.591813811815732,
        0.344831798189898,
        -0.157607874284224,
        -0.101315055088023,
        0.0140678532972274,
        0.0109533232815195,
        -0.000216944372851913,
        -0.000345510532658189,
        -3.13949510166722e-07,
        2.38332090327087e-06
    }
};

