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
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

#ifndef __IIR_5K5
#define __IIR_5K5
//
// alternative filter designed with MATLAB fdatools by DD4WH 2016-02-18
// 24k sampling rate, Lattice ARMA structure
// 10th order IIR Elliptic lowpass LPF
// Fpass 5500Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_5k5_numStages 10

const float IIR_5k5_LPF_pkCoeffs[] =
{
		0.270056112170917,
		-0.521414453389724,
		0.678616912945954,
		-0.641289912968769,
		0.883688002282309,
		-0.381347496766270,
		0.989526871938782,
		-0.192450450475999,
		0.999443061271067,
		-0.139179149092653
};

const float IIR_5k5_LPF_pvCoeffs[] =
{
		0.0165986011732628,
		0.0791435170266696,
		0.186630364068749,
		0.233915282634195,
		0.138804057676891,
		0.00684091782624835,
		-0.0470727830131718,
		-0.00518447562091082,
		0.00219719710580432,
		8.29160622089417e-05,
		8.64931555180915e-05
};


#endif
