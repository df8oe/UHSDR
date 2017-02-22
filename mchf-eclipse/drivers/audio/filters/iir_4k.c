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
// Fpass 4000Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB

#define IIR_4k_numStages 10
const arm_iir_lattice_instance_f32 IIR_4k_LPF =
{
    .numStages = IIR_4k_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.155876536292444,
        -0.180325472152569,
        0.546715175591137,
        0.0762684731194015,
        0.938604917720414,
        0.372421515579738,
        0.994769037313590,
        0.462548368276089,
        0.999652456699243,
        0.494335639815055
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0793628035363237,
        0.326663427969449,
        0.494575679160066,
        0.197555516079231,
        -0.182792933325612,
        -0.0614448809184776,
        0.0331486325344859,
        0.00478157010758418,
        -0.000230235339343743,
        -5.64895267110011e-05,
        -3.66641450827987e-05
    }
};

