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

#ifndef __IIR_4K2
#define __IIR_4K2
//
// alternative filter designed with MATLAB fdatools by DD4WH 2016-02-15
// 12k sampling rate, Lattice ARMA structure
// 10th order IIR Elliptic lowpass LPF
// Fpass 4200Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_4k2_numStages 10

const float IIR_4k2_LPF_pkCoeffs[] =
{
	0.146928213236347,
	-0.112037098760571,
	0.573308628506104,
	0.237996369839101,
	0.949903541125154,
	0.484079856668923,
	0.995620381648115,
	0.556090559967388,
	0.999703984324404,
	0.582944939913576
};

const float IIR_4k2_LPF_pvCoeffs[] =
{
		0.101763006075955,
		0.390529350317333,
		0.507373813109782,
		0.101105933174559,
		-0.219699891693221,
		-0.0341597848533599,
		0.0407495774164026,
		0.00298273384268377,
		-0.00135023483300145,
		-4.39812314239063e-05,
		-4.24250219673017e-06
};


#endif
