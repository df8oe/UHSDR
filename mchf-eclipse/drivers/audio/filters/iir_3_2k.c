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
// Fpass 3200Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
// bandpass filter 150 - 3200Hz
//
//

#define IIR_3k2_numStages 10

const arm_iir_lattice_instance_f32 IIR_3k2_LPF =
{
    .numStages = IIR_3k2_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.218694139928302,
        -0.403838779807303,
        0.594634035120532,
        -0.455295219667013,
        0.893598014796430,
        -0.118422108607239,
        0.991224652677821,
        0.0476094068099549,
        0.999483170165258,
        0.0963103336836062
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0292173064645854,
        0.139171967941396,
        0.302089953179196,
        0.315642874138601,
        0.0977262133806149,
        -0.0516806998414988,
        -0.0577240236966878,
        -0.000850928349123370,
        0.00696050225811562,
        0.000139795355880837,
        -5.20544243489754e-05
    }
};

const arm_iir_lattice_instance_f32 IIR_3k2_BPF =
{
    .numStages = IIR_3k2_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.226940893639584,
        -0.423427683999446,
        0.469848234909221,
        -0.730589423377186,
        0.634876055736042,
        -0.254866265688223,
        0.950443823908253,
        -0.994637626804822,
        0.999635535930601,
        -0.996861601733971
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.0465676087537323,
        -0.192308251979962,
        -0.291767361307907,
        -0.0757211101641522,
        0.218582885651641,
        0.124575788231386,
        -0.0337072066012442,
        0.0164385733761534,
        -0.00119735821375683,
        -3.26543663353371e-05,
        1.12878126917737e-06
    }
};

