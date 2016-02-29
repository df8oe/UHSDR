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

#ifndef __IIR_HPF_15K
#define __IIR_HPF_15K
//
// Filter designed 20151022 by C. Turner, KA7OEI using MatLAB fdatools
//
// This high pass filter is for the FM demodulator to extract high-frequency "triangle noise" energy
// for the squelch algorithm.
//
// NOTE:
//	- IIR structure is Lattice Autoregressive Moving-Average (ARMA)
//	- ARM FIR/IIR algorithms require time reverse-order coefficients!!!
//
//	6th order Elliptic highpass filter
//	
//	
//	-6dB point:  14.9 kHz
//	-20dB point:  14.5 kHz
//	-40dB point:  13.8 kHz
//	-60dB point:  13.3 kHz
//
/*
const uint16_t IIR_15k_hpf_numStages = 6;
const float IIR_15k_hpf_pkCoeffs[] =
{
		0.513580159973,
		0.7246612586998,
		0.7246612586998,
		0.8107542886737,
		0.7534361820128,
		0.9529352399472,
		0.4826168836816
};

const float IIR_15k_hpf_pvCoeffs[] =
{
		0.01068626058243,
		-0.04830879422708,
		0.1047720411744,
		-0.1152362519753,
		0.06014978280645,
		-0.003570328867624,
		-0.01351144592324
};
*/
// Sloped version - experimental, not used
//	-3dB:  20.0 kHz
//	-6dB:  19.0 kHz
//	-20dB: 15.6 kHz
//  -40dB: 13.9 kHz
//	-58dB: <13.2 kHz

const uint16_t IIR_15k_hpf_numStages = 6;
const float IIR_15k_hpf_pkCoeffs[] =
{
		0.08584448365242,
		0.298455698329,
		0.6057782760185,
		0.7115173863477,
		0.7698117813539,
		0.7879540958152
};

const float IIR_15k_hpf_pvCoeffs[] =
{
		0.00561889294652,
		-0.02629790457241,
		0.0684050306275,
		-0.1143219836396,
		0.1169486269215,
		-0.07287150832503,
		0.02534743920497
};



#endif
