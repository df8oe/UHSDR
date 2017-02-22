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
// Fpass 8500Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_8k5_numStages 8

const arm_iir_lattice_instance_f32 IIR_8k5_LPF =
{
    .numStages = IIR_8k5_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.147016458647620,
        -0.0800576834677209,
        0.607415025263421,
        0.311944568388449,
        0.961718465037911,
        0.531590985304236,
        0.997775367200182,
        0.597158407524272
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.116227477513591,
        0.426616080915722,
        0.501928042829669,
        0.0426267579687529,
        -0.221292633625496,
        -0.0171136752421530,
        0.0374419493102454,
        0.00134067435969376,
        -0.00112236216067886
    }
};


