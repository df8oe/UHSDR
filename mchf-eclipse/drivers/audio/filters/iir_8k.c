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
// Fpass 8000Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_8k_numStages 8

const arm_iir_lattice_instance_f32 IIR_8k_LPF =
{
    .numStages = IIR_8k_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.156730412841056,
        -0.170860629143791,
        0.567767507488873,
        0.113051837961350,
        0.950299005264617,
        0.398693812045436,
        0.997198510683089,
        0.485296093326901
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.0857789183340628,
        0.345903140429045,
        0.500262683274993,
        0.165780339808995,
        -0.198921078104878,
        -0.0482542902035035,
        0.0362835359961724,
        0.00297622750353344,
        -0.000691737718980207
    }
};


