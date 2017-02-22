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

// alternative filter designed with MATLAB fdatools by DD4WH 2016-02-02
// 12k sampling rate, Lattice ARMA structure
// 10th order IIR Elliptic lowpass
// Fpass 4600Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB

#define IIR_4k6_numStages 10
const arm_iir_lattice_instance_f32 IIR_4k6_LPF =
{
    .numStages = IIR_4k6_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.143211048758245,
        0.0636595308157890,
        0.678384747001818,
        0.544299319747716,
        0.969133023383267,
        0.681014534387261,
        0.997187827029787,
        0.722612358980166,
        0.999805647018336,
        0.739954454035932

    },

    .pvCoeffs = (float*) (const float[])
    {
        0.167057909773121,
        0.527901313830262,
        0.437747419718857,
        -0.0980305392148662,
        -0.167408704264879,
        0.00853396918515870,
        0.0231579610435375,
        0.000144746230869686,
        -0.000845091312013047,
        -6.36386754826468e-06,
        5.95501329503367e-06
    }
};


