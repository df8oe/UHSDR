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
// alternative filter designed with MATLAB fdatools by DD4WH 2016-02-15
// 12k sampling rate, Lattice ARMA structure
// 10th order IIR Elliptic lowpass
// Fpass 1400Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
// bandpass filter 150 - 1400Hz
//
//

#define IIR_1k4_numStages 10
const arm_iir_lattice_instance_f32 IIR_1k4_LPF =
{
    .numStages = IIR_1k4_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.513121512033492,
        -0.848156725130716,
        0.912905355716368,
        -0.920457559093897,
        0.946299472905260,
        -0.874357776863127,
        0.991745064593748,
        -0.780050411952442,
        0.999672192852107,
        -0.747759230954469
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.00303978190645145,
        0.00957370964334359,
        0.0192021255112258,
        0.0205518463597444,
        0.0157679250083878,
        0.00641323479251947,
        0.00194320223728067,
        0.000177526968367495,
        -0.000114188594530516,
        -1.00881138288648e-06,
        -1.61208577327586e-06
    }
};

const arm_iir_lattice_instance_f32 IIR_1k4_BPF =
{
    .numStages = IIR_1k4_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.544472709123517,
        -0.846448469159389,
        0.914323381297358,
        -0.924099832753511,
        0.913728327779171,
        -0.906301509630351,
        0.986341377918403,
        -0.994060117939703,
        0.999689022223705,
        -0.996755741677070
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.00397159947826007,
        -0.0142756966456071,
        -0.0258501789268387,
        -0.0249409146677880,
        -0.00842585665202606,
        0.00150853661679518,
        0.00402678083172615,
        0.000247302313425797,
        -2.24256007699125e-05,
        -5.01381740295598e-07,
        1.79705759370541e-08
    }
};
// 12k sampling rate, Lattice ARMA structure
// 10th order IIR Elliptic lowpass
// Fpass 1100Hz - 2500Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
// bandpass filter 1100 - 2500Hz
//
//
const arm_iir_lattice_instance_f32 IIR_1k4_SSTV =
{
    .numStages = IIR_1k4_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.506305489868821,
        -0.530549016052929,
        0.852752516028665,
        -0.605615780849346,
        0.908798485793881,
        -0.609090038979443,
        0.898589845278408,
        -0.527324792497554,
        0.972399687849265,
        -0.806526221782603
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.00502583555949696,
        -0.0125105344886717,
        -0.00418751674186635,
        0.0213049813421346,
        0.0261437873033528,
        -0.00847633761697014,
        -0.0252946013159023,
        -0.00396151687990829,
        0.0111353202658218,
        0.00168003453028945,
        -0.000348271075749648
    }
};


