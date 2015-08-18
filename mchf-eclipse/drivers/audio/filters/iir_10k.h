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

#ifndef __IIR_10K
#define __IIR_10K
//
// Filter designed 20140915 by C. Turner, KA7OEI using MatLAB fdatools
//
// NOTE:
//	- IIR structure is Lattice Autoregressive Moving-Average (ARMA)
//	- ARM FIR/IIR algorithms require time reverse-order coefficients!!!
//
//	8th order Elliptic lowpass filter
//	
//	
//	-6dB point:  10.0 kHz
//	-20dB point:  10.18 kHz
//	-40dB point:  10.46 kHz
//	-60dB point:  >10.88 kHz
//
//#define NCoef 8
const uint16_t IIR_10k_numStages = 8;
const float IIR_10k_pkCoeffs[] =
{
		0.3059837075323,
		-0.5896555175446,
		0.7331597393194,
		-0.713542623059,
		0.8999944361723,
		-0.4831480007607,
		0.9938574536212,
		-0.2965660617857
};

const float IIR_10k_pvCoeffs[] =
{
		0.01294033742346,
		0.06059508198778,
		0.1442543837861,
		0.184661010229,
		0.1196473741182,
		0.01735304263064,
		-0.03003092329645,
		-0.003155003081181,
		0.0003259315989964
};

#endif
