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
// Fpass 7000Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_7k_numStages 8

const arm_iir_lattice_instance_f32 IIR_7k_LPF =
{
    .numStages = IIR_7k_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.190713577571592,
        -0.319871726375826,
        0.565395988442476,
        -0.255306414282902,
        0.924727833564385,
        0.104786836584826,
        0.995972585176399,
        0.237878968193324
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0464388990834632,
        0.211741818487612,
        0.403878817111474,
        0.311449064277157,
        -0.0264024657273338,
        -0.0789871827367324,
        -0.0150701534983805,
        0.00343825204070403,
        0.00335955618048627
    }
};


