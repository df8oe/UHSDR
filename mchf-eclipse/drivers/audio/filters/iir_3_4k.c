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
// LPF:
// 10th order IIR Elliptic lowpass
// Fpass 3400Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
// BPF:
// bandpass filter 150 - 3400Hz
//
//

#define IIR_3k4_numStages 10

const arm_iir_lattice_instance_f32 IIR_3k4_LPF =
{
    .numStages = IIR_3k4_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.199549437319537,
        -0.351164425170794,
        0.565426959446253,
        -0.347212945196544,
        0.903076937389513,
        0.00604837383458759,
        0.992097069597849,
        0.154906080441608,
        0.999516379489912,
        0.200159122143531
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0375360428960823,
        0.175514272047973,
        0.358379113282395,
        0.327274521409256,
        0.0418055459145037,
        -0.0747547603431524,
        -0.0419525359249555,
        0.00238252032207142,
        0.00672097481119971,
        8.86692474596773e-05,
        -0.000108177709927680
    }
};

const arm_iir_lattice_instance_f32 IIR_3k4_BPF =
{
    .numStages = IIR_3k4_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.204719622771957,
        -0.386730932575186,
        0.387584200592573,
        -0.700090533739370,
        0.652085184091423,
        -0.105219888998415,
        0.929614073821885,
        -0.994586210742946,
        0.999632199294300,
        -0.996867967817777
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.0587697225218063,
        -0.231747513031753,
        -0.311574476801376,
        -0.0140958795068678,
        0.276927602806913,
        0.0895259663386021,
        -0.0561390299585095,
        0.0254613257946724,
        -0.00162532862462556,
        -4.40492441641516e-05,
        1.51963288604756e-06
    }
};

