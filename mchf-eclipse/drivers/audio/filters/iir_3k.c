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
// Filter designed 20141202 by C. Turner, KA7OEI using MatLAB fdatools
//
// NOTE:
//	- IIR structure is Lattice Autoregressive Moving-Average (ARMA)
//	- ARM FIR/IIR algorithms require time reverse-order coefficients!!!
//
// *** 12 KSPS Sample Rate!
//
//
//	10th order Elliptic bandpass filter
//	Fpass:  150, 2700 Hz
//	-6dB points:  138, 3606 Hz
//	-20dB points:  120, 3853 Hz
//	-40dB points:  98, 4191 Hz
//	-60dB points:  <85, >4385 Hz
//

#define IIR_3k_numStages 10
const arm_iir_lattice_instance_f32 IIR_3k =
{
    .numStages = IIR_3k_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.00289573666141589550,
        0.02895736661415895100,
        0.13030814976371530000,
        0.34748839936990744000,
        0.60810469889733798000,
        0.72972563867680562000,
        0.60810469889733798000,
        0.34748839936990744000,
        0.13030814976371530000,
        0.02895736661415895100,
        0.00289573666141589550
    },

    .pvCoeffs = (float*) (const float[])
    {
        1.00000000000000000000,
        0.00000000000000008232,
        1.34038326769903440000,
        -0.00000000000000008232,
        0.54535390952381724000,
        0.00000000000000009261,
        0.07704116610119554600,
        -0.00000000000000000322,
        0.00316548154834338670,
        -0.00000000000000000004,
        0.00001677879772667541
    }
};

