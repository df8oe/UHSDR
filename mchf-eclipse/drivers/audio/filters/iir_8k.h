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

#ifndef __IIR_8K
#define __IIR_8K
//
// alternative filter designed with MATLAB fdatools by DD4WH 2016-02-18
// 24k sampling rate, Lattice ARMA structure
// 8th order IIR Elliptic lowpass LPF
// Fpass 8000Hz
// Astop 60dB
// coefficients in reverse order than that spit out by MATLAB
//
//

#define IIR_8k_numStages 8

const float IIR_8k_LPF_pkCoeffs[] =
{
		0.156730412841056,
		-0.170860629143791,
		0.567767507488873,
		0.113051837961350,
		0.950299005264617,
		0.398693812045436,
		0.997198510683089,
		0.485296093326901
};

const float IIR_8k_LPF_pvCoeffs[] =
{
		0.0857789183340628,
		0.345903140429045,
		0.500262683274993,
		0.165780339808995,
		-0.198921078104878,
		-0.0482542902035035,
		0.0362835359961724,
		0.00297622750353344,
		-0.000691737718980207
};


#endif
