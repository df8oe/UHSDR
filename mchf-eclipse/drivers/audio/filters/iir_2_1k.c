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
// Fpass 2100Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
// bandpass filter 150 - 2100Hz
//
//

#define IIR_2k1_numStages 10

const arm_iir_lattice_instance_f32 IIR_2k1_LPF =
{
    .numStages = IIR_2k1_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.367665055229345,
        -0.688450049074435,
        0.805164789160803,
        -0.811607062793749,
        0.902007053621482,
        -0.678829166852664,
        0.988754839338072,
        -0.511291308813728,
        0.999489812346376,
        -0.461573314781018
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.00731063001354271,
        0.0314121915186572,
        0.0739715509453205,
        0.0977625054614904,
        0.0764634076081996,
        0.0273892594750243,
        -0.00364458069285592,
        -0.00143162744212593,
        -0.00150600852899662,
        -3.00785390039450e-05,
        1.90133500787143e-05
    }
};

const arm_iir_lattice_instance_f32 IIR_2k1_BPF =
{
    .numStages = IIR_2k1_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.388108085643651,
        -0.684535285919172,
        0.795328554327865,
        -0.851745126106827,
        0.786930711099640,
        -0.782402423654034,
        0.981748853637700,
        -0.994476024273642,
        0.999660553986239,
        -0.996813012539810
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.0113057560156174,
        -0.0493258305692081,
        -0.101429131418656,
        -0.0939936931949645,
        -0.00933213227739606,
        0.0417064397734366,
        0.0245790863191006,
        0.00146913585763049,
        -0.000159569288036600,
        -4.03203149703518e-06,
        1.41762873655249e-07
    }
};

