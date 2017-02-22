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
// Fpass 1600Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
// bandpass filter 150 - 1600Hz
//
//

#define IIR_1k6_numStages 10
const arm_iir_lattice_instance_f32 IIR_1k6_LPF =
{
    .numStages = IIR_1k6_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.466488867417511,
        -0.806195990652699,
        0.886218044099720,
        -0.895050799512214,
        0.932962782640420,
        -0.830295375528087,
        0.990476931527691,
        -0.713094807516228,
        0.999609649167846,
        -0.674710639139313
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.00389683132359987,
        0.0138226995337397,
        0.0295183362623048,
        0.0347590317564048,
        0.0274564574556244,
        0.0115668027741923,
        0.00266968736496351,
        0.000176086120205525,
        -0.000357042283087743,
        -5.34430276839977e-06,
        -3.16592076707485e-06
    }
};

const arm_iir_lattice_instance_f32 IIR_1k6_BPF =
{
    .numStages = IIR_1k6_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.494195797915416,
        -0.803665234575735,
        0.885970320933212,
        -0.904522861950696,
        0.882497256186818,
        -0.879441652192076,
        0.985240463020393,
        -0.994220286123894,
        0.999679143098346,
        -0.996775926313658
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.00542804202987006,
        -0.0211221071946913,
        -0.0411248591148432,
        -0.0404574358824852,
        -0.0126311821394728,
        0.00603670441721448,
        0.00803948053906083,
        0.000449048992769978,
        -4.33255842577709e-05,
        -1.00733352962831e-06,
        3.58686948389447e-08
    }
};

