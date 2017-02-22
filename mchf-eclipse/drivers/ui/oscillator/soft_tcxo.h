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

//
// SI570 frequency shift table
//       0-100 degrees C range
//       shots from 20m band (56Mhz)
//
// This frequency compensation table referenced to 14.000 MHz by KA7OEI, 10/14 using
// a GPS reference in a temperature-controlled environment, extrapolated below 17C and above
// 70C.
//
// Temperature normalized to 43C, a temperature achieved very soon after power-up
// with both the Si570 and the temperature sensor thermally bonded.
//
// Note:  The exact frequency/temperature dependencies will likely vary from unit to unit
// for each Si570, but the values below appear to approximately follow typical AT-cut
// temperature-frequency curves.
//
#include "mchf_types.h"

// LO temperature compensation
typedef struct LoTcxo
{
    // Current compensation value
    // loaded to LO
    int32_t comp;
    int32_t temp;

    bool    sensor_present;
} LoTcxo;


// ------------------------------------------------
// LO Tcxo
extern LoTcxo lo;

bool SoftTcxo_HandleLoTemperatureDrift();
void SoftTcxo_Init();
