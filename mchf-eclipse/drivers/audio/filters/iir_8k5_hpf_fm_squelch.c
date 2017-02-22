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
// Filter designed 20151022 by C. Turner, KA7OEI using MatLAB fdatools
//
// NOTE:
//	- IIR structure is Lattice Autoregressive Moving-Average (ARMA)
//	- ARM FIR/IIR algorithms require time reverse-order coefficients!!!
//
//	6th order Elliptic highpass filter
//
//
//	-6dB point:  8.4 kHz
//	-20dB point:  8.0 kHz
//	-40dB point:  7.4 kHz
//	-60dB point:  <7.0 kHz
//
//#define NCoef 8
#define IIR_8k5_numStages  6
const arm_iir_lattice_instance_f32 IIR_8k5_hpf =
{
    .numStages = IIR_8k5_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.3267431914313,
        0.2827426748037,
        0.6792677220421,
        -0.08412750477612,
        0.9796815399487,
        -0.3939883111528
    },

    .pvCoeffs = (float*) (const float[])
    {
        0.07735072858385,
        -0.306940854114,
        0.4145882522999,
        -0.09894434429254,
        -0.1687647954502,
        0.01867899629887,
        0.02161688514499
    }
};

