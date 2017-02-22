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
// Fpass 6000Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_6k_numStages 10

const arm_iir_lattice_instance_f32 IIR_6k_LPF =
{
    .numStages = IIR_6k_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.240061145139098,
        -0.456135423117119,
        0.629931199814858,
        -0.546874535504622,
        0.886954490804698,
        -0.239344357593924,
        0.990406694917175,
        -0.0598647009288412,
        0.999458839296527,
        -0.00852401189342987
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0227312628074594,
        0.109078946204801,
        0.247843364281694,
        0.285784016518540,
        0.129833944671845,
        -0.0238187084272364,
        -0.0598880195707474,
        -0.00364575842735154,
        0.00541066911948504,
        0.000142100013861043,
        2.39142895447318e-05
    }
};


