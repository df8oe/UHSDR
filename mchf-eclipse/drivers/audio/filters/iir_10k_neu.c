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
// Fpass 10000Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_10k_numStages 8

const arm_iir_lattice_instance_f32 IIR_10k_LPF =
{
    .numStages = IIR_10k_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.184297745178662,
        0.346275356901559,
        0.835935186979480,
        0.790382467146251,
        0.986915159126712,
        0.842177143803695,
        0.999207130498260,
        0.862058584401922
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.285756161122327,
        0.647395708199686,
        0.191311565499666,
        -0.164203854783080,
        -0.0319701390832776,
        0.00976034243199386,
        0.00225291674041372,
        -0.000117333004814757,
        -4.35613343882091e-05
    }
};


