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

const uint16_t IIR_3k6_numStages = 10;
const float IIR_3k6_pkCoeffs[] =
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
};

const float IIR_3k6_pvCoeffs[] =
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
};

#endif
