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
// Fpass 9500Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_9k5_numStages  8

const arm_iir_lattice_instance_f32 IIR_9k5_LPF =
{
    .numStages = IIR_9k5_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.154589161975431,
        0.171214182840522,
        0.752540038804833,
        0.661446067312243,
        0.979967038791770,
        0.755243080342941,
        0.998791846173898,
        0.787213445621968
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.212160913842594,
        0.589885689317715,
        0.348337037956110,
        -0.155662625526521,
        -0.103203755044854,
        0.0134205375549463,
        0.0105947295297542,
        -0.000168431246632794,
        -0.000257624900844355
    }
};


