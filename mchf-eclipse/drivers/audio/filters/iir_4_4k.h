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

#ifndef __IIR_4_4K
#define __IIR_4_4K

// filter designed with MATLAB fdatools by DD4WH 2016-02-03
// initial idea for these filters: Clint KA7OEI
// 12k sampling rate, Lattice ARMA structure
// 10th order IIR Elliptic lowpass
// Fpass 4400Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB

#define IIR_4k4_numStages 10
const float IIR_4k4_LPF_pkCoeffs[] =
{
		0.142124344207148,
		-0.0322948634379783,
		0.618111817382185,
		0.397046008479189,
		0.960113543408077,
		0.587394367585518,
		0.996430642116453,
		0.643137617322656,
		0.999755612135519,
		0.665125140142304
};

const float IIR_4k4_LPF_pvCoeffs[] =
{
		0.130420860087981,
		0.458803381555481,
		0.490791158211600,
		-0.00438502468346513,
		-0.213305874865595,
		-0.00833743485729244,
		0.0354832807597507,
		0.00122731012273469,
		-0.00136446459308992,
		-2.19632806766068e-05,
		7.11876663672673e-06
};

#endif
