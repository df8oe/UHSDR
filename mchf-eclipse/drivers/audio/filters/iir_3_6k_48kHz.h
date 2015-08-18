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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __IIR_3_6K
#define __IIR_3_6K
//
// Filter designed 20140915 by C. Turner, KA7OEI using MatLAB fdatools
//
// NOTE:
//	- IIR structure is Lattice Autoregressive Moving-Average (ARMA)
//	- ARM FIR/IIR algorithms require time reverse-order coefficients!!!
//

/*
//	10th order Elliptic bandpass filter
//	Fstop:  50, 5000 Hz
//	Fpass:  300, 3400 Hz
//	-6dB points:  291, 3498 Hz
//	-20dB points:  263, 3832 Hz
//	-40dB points:  222, 4497 Hz
//	-60dB points:  200, 5000 Hz
//
#define NCoef 10
const uint16_t IIR_3k6_numStages = NCoef;
const float IIR_3k6_pkCoeffs[] =
{
		0.7450636297885,
		-0.9494035955484,
		0.9709077139629,
		-0.972713445656,
		0.9666356805139,
		-0.9707677036845,
		0.9969006326429,
		-0.9984507515357,
		0.9999303511537,
		-0.9991610892686
};

const float IIR_3k6_pvCoeffs[] =
{
		-0.001646350869852,
		-0.004459836819556,
		-0.005012222021959,
		-0.004280093932952,
		-0.001224542341149,
		-0.0004057413291244,
		0.0001409734559939,
		8.709876701388e-06,
		-1.350424766525e-07,
		-2.629789578809e-09,
		2.261536444226e-11
};

*/
//	7th order Elliptic lowpass filter
//	Fstop:  4200 Hz
//	Fpass:  3525 Hz
//	-6dB point:  3600 Hz
//	-20dB point:  3765 Hz
//	-40dB point:  4028 Hz
//	-60dB point:  >5000 Hz
//
//#define NCoef 7
const uint16_t IIR_3k6_numStages = 7;
const float IIR_3k6_pkCoeffs[] =
{
		-0.6257054348396,
		0.9298698682423,
		-0.9633384902852,
		0.9705466139129,
		-0.9625262208223,
		0.9911084087461,
		-0.9156733831273
};

const float IIR_3k6_pvCoeffs[] =
{
		0.001626551680336,
		0.004351644196217,
		0.005556768570411,
		0.005390996745481,
		0.002445551537458,
		0.001191410667958,
		0.0001474370051449,
		2.67609891319e-05
};

#endif
