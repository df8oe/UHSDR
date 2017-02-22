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
// 10th order IIR Elliptic lowpass LPF
// Fpass 5000Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_5k_numStages 10

const arm_iir_lattice_instance_f32 IIR_5k_LPF =
{
    .numStages = IIR_5k_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.304008884053488,
        -0.586479754574523,
        0.728707358729566,
        -0.717591127840479,
        0.886460005029009,
        -0.509239560057348,
        0.988906686305070,
        -0.320749263130520,
        0.999445512653576,
        -0.267299520678722
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0121126909328103,
        0.0563705089486983,
        0.135077706371490,
        0.177369645199308,
        0.122940273900304,
        0.0249807561603719,
        -0.0272356075029574,
        -0.00442102257133439,
        -0.000487963028261214,
        9.54073216847401e-06,
        8.24834253008722e-05
    }
};


