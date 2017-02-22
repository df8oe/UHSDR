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
// Fpass 3600Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB

#define IIR_3k6_numStages 10
const arm_iir_lattice_instance_f32 IIR_3k6_LPF =
{
    .numStages = IIR_3k6_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.182604754397413,
        -0.297364372108114,
        0.545463598315966,
        -0.221554941171137,
        0.914413017285099,
        0.131148813295864,
        0.992992542452280,
        0.260698968055509,
        0.999556840879677,
        0.301864824451559

    },

    .pvCoeffs = (float*) (const float[])
    {

        0.0481999578478482,
        0.218712180455229,
        0.412921909514823,
        0.313810309656850,
        -0.0329738792744796,
        -0.0858389869457352,
        -0.0158927513034342,
        0.00483419702063959,
        0.00477236245823653,
        1.66508193628068e-05,
        -0.000116754074820341
    }
};



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
//	Fpass:  150, 3465 Hz
//	-6dB points:  138, 3606 Hz
//	-20dB points:  120, 3853 Hz
//	-40dB points:  98, 4191 Hz
//	-60dB points:  <85, >4385 Hz
//

const arm_iir_lattice_instance_f32 IIR_3k6_BPF =
{
    .numStages = IIR_3k6_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.1309746791958,
        -0.3193816788598,
        0.2931126334419,
        -0.6722769249819,
        0.6365603289927,
        -0.03819195677896,
        0.9228154008907,
        -0.9953113565282,
        0.9995716337798,
        -0.9970944160657
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.07111514509409,
        -0.2707254278064,
        -0.3391742081774,
        0.0230826255569,
        0.3263161755735,
        0.07832215297698,
        -0.06606133406622,
        0.03330939215314,
        -0.001770560523546,
        -5.405998866079e-05,
        1.724784082002e-06
    }
};

