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
// 10th order IIR Elliptic lowpass LPF
// Fpass 3800Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
// bandpass filter BPF 150 - 3800Hz
//
//

#define IIR_3k8_numStages 10

const arm_iir_lattice_instance_f32 IIR_3k8_LPF =
{
    .numStages = IIR_3k8_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.167955275345335,
        -0.241109484666904,
        0.538161432993282,
        -0.0791711604545083,
        0.926560053717762,
        0.254092584645358,
        0.993888674387578,
        0.363673903035194,
        0.999602803572954,
        0.400290937647385
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0618636668587285,
        0.269123223807235,
        0.460519308028180,
        0.270623345085776,
        -0.114078077039268,
        -0.0809257862368131,
        0.0122835687057461,
        0.00564462194559656,
        0.00205421413782014,
        -3.85276331777984e-05,
        -8.31130731588567e-05
    }
};

const arm_iir_lattice_instance_f32 IIR_3k8_BPF =
{
    .numStages = IIR_3k8_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.163149581231639,
        -0.334764418550643,
        0.204881267601001,
        -0.602380325621413,
        0.738422338450555,
        0.180850983309683,
        0.821590504163200,
        -0.994197577474067,
        0.999626101957683,
        -0.996879424958163
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.0919069404581591,
        -0.319576688929381,
        -0.305305278407872,
        0.164897561830671,
        0.348959586027291,
        -0.0157756061640711,
        -0.0440343211466503,
        0.0543351559597219,
        -0.00290737404995828,
        -7.64322618936331e-05,
        2.62841509104939e-06
    }
};

