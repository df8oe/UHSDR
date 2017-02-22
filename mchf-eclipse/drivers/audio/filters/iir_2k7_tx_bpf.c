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
// Filter designed 20140927 by C. Turner, KA7OEI using MatLAB fdatools
//
// This filter has relaxed band-stop properties and is designed for SSB audio TX filtering.
//
// NOTE:
//	- IIR structure is Lattice Autoregressive Moving-Average (ARMA)
//	- ARM FIR/IIR algorithms require time reverse-order coefficients!!!
//
//	10th order Elliptic bandpass filter
//	Fstop:  200, 3250 Hz
//	Fpass:  300, 2700 Hz
//
//  Amplitude responses - referenced to attenuation at 1.5 kHz:
//	-6dB points:  257, 2830 Hz
//	-10dB points:  254, 2885 Hz
//	-20dB points:  246, 2988 Hz
//	-30dB points:  <225, >3060 Hz
//
//  Low-end pre-emphasis added to offset effects of Hilbert transformer:
//	>=8.0dB:   @ 280 Hz
//	>=7.0dB:  <298 Hz
//	>=6.0dB:  <355 Hz
//	>=5.0dB:  <421 Hz
//  >=4.0dB:  <521 Hz
//	>=3.0dB:  <670 Hz
//  >=2.0dB:  <870 Hz
//	>=1.0dB:  <1130 Hz
//	>=0.5dB:  <1340 Hz
//
// The above counteracts the rolloff to produce an overall flat response to approx. 1dB between 275 Hz and 2500 Hz.
//
// Pole/Zero Info:
//	1: P0.98293, 0.36082; Z1, 0.40965
//	2: P0.99849, 0.;035664; Z1, 0.030945
//	3: P0.91702, 0.30422;  Z1, 0.53916
//	4: P0.98884, 0.038766;  Z1, 0.023267
//	5: P0.93128, 0.009238;  Z (none)
//
//
#define IIR_TX_numStages 10
#define IIR_TX_2k7_numStages 10

const arm_iir_lattice_instance_f32 IIR_TX_2k7 =
{
    .numStages = IIR_TX_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
        0.6869176968793,
        -0.9512054401742,
        0.9748233658729,
        -0.9886769453802,
        0.9699856293614,
        -0.976965264625,
        0.9989051081887,
        -0.9994482291916,
        0.9999677314304,
        -0.9993834480784
    },

    .pvCoeffs = (float*) (const float[])
    {
        -0.01873226419377,
        -0.0351077314033,
        -0.01181875664072,
        -0.0100003360675,
        -0.0005123880132686,
        -0.0002827366407254,
        6.374826243116e-05,
        3.655128410718e-06,
        -1.949876071716e-08,
        -5.55178847339e-10,
        3.472732518217e-12
    }
};


// IIR filter
// lattice ARMA coefficients designed with MATLAb fdatools
// bandpass 50 - 2750Hz
#define IIR_TX_WIDE_BASS_numStages 10
const arm_iir_lattice_instance_f32 IIR_TX_WIDE_BASS =
{
    .numStages = IIR_TX_WIDE_BASS_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
    	0.71896929903805900000,
		-0.95942907850625200000,
		0.97792311678973200000,
		-0.98088701203507700000,
		0.97297595879898900000,
		-0.99620090645682700000,
		0.99990501182910600000,
		-0.99996667856688500000,
		0.99998653853233900000,
		-0.99998543366088200000

    },

    .pvCoeffs = (float*) (const float[])
    {

    	-0.00032448877846164900,
		-0.00117728102231410000,
		-0.00209099021086666000,
		-0.00223337754782698000,
		-0.00122619886193943000,
		-0.00046485618062794300,
		0.00001256926158546300,
		0.00000019091112216183,
		-0.00000000006469610177,
		-0.00000000000289012694,
		1.24E-15

    }
};

//
// 150 - 2850Hz bandpass
//
#define IIR_TX_WIDE_TREBLE_numStages 10
const arm_iir_lattice_instance_f32 IIR_TX_WIDE_TREBLE =
{
    .numStages = IIR_TX_WIDE_TREBLE_numStages,
    .pkCoeffs  = (float*) (const float[])
    {
    	0.71896929903807000000,
		-0.95701929433651100000,
		0.97688821391048500000,
		-0.97914462752169300000,
		0.97381210430161500000,
		-0.98932784460632400000,
		0.99918319918236100000,
		-0.99964609685921500000,
		0.99997374020338600000,
		-0.99980049691966100000


    },

    .pvCoeffs = (float*) (const float[])
    {

    	-0.00032448877846164000,
		-0.00117432406256116000,
		-0.00207458605133000000,
		-0.00219730703764023000,
		-0.00117629105002028000,
		-0.00042269171278772100,
		0.00003803127953775490,
		0.00000159703882026720,
		-0.00000000535014800171,
		-0.00000000013312786485,
		2.67655E-13

    }
};

//
// 250 - 2950Hz bandpass
//
#define IIR_TX_SOPRANO_numStages 10
const arm_iir_lattice_instance_f32 IIR_TX_SOPRANO =
{
    .numStages = IIR_TX_SOPRANO_numStages,
    .pkCoeffs  = (float*) (const float[])
    {

    	0.71987365385562700,
		-0.95478395730601100,
		0.97597041246871300,
		-0.97877331988142200,
		0.97311471309818900,
		-0.98013955262452800,
		0.99791087029646100,
		-0.99903556044544100,
		0.99994405075167400,
		-0.99944176350280300

    },

    .pvCoeffs = (float*) (const float[])
    {

    	-0.00146489605640520,
		-0.00378491009751258,
		-0.00400350501493743,
		-0.00341201203499246,
		-0.00096809938766577,
		-0.00035554457716075,
		0.00007141442667891,
		0.00000426101940838,
		-0.00000003996117780,
		-0.00000000087267111,
		0.00000000000495626

    }
};



/*

// NOTE:
//	- IIR structure is Lattice Autoregressive Moving-Average (ARMA)
//	- ARM FIR/IIR algorithms require time reverse-order coefficients!!!
//
//	10th order Elliptic bandpass filter
//	Fstop:  200, 3250 Hz
//	Fpass:  300, 2575 Hz
//	-6dB points:  285, 2698 Hz
//	-10dB points:  281, 2745 Hz
//	-20dB points:  270, 2845 Hz
//	-30dB points:  <265, >2915 Hz
//
#define IIR_TX_2k7_numStages 10
const arm_iir_lattice_instance_f32 IIR_TX_2k7 = {
  .numStages = IIR_TX_numStages,
  .pkCoeffs  = (float*) (const float[])
{
		0.6630364568624,
		-0.9520893446293,
		0.9760332145655,
		-0.9865766256209,
		0.9831633680402,
		-0.9684840684235,
		0.9968012184072,
		-0.9991378973359,
		0.9999607252996,
		-0.9992727516361
},

  .pvCoeffs = (float*) (const float[])
{
		-0.01765590852027,
		-0.03197343698045,
		-0.009018268963126,
		-0.007790758301124,
		-0.0001813788700895,
		-0.0001640724508686,
		6.107606732009e-05,
		4.622811990096e-06,
		-3.806757115088e-08,
		-5.946272640944e-10,
		4.410607296057e-12
}
};
*/

