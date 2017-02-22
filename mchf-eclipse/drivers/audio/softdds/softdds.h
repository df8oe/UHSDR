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

#ifndef __SOFTDDS_H
#define __SOFTDDS_H

#include "dds_table.h"

uint32_t softdds_stepForSampleRate(float freq,ulong samp_rate);


// Soft DDS public structure
typedef struct
{
    // DDS accumulator
    ulong   acc;

    // DDS step - not working if part of the structure
    ulong   step;

} SoftDds;



/**
 * Execute a single step in the sinus generation
 */
inline uint32_t softdds_step(__IO SoftDds* dds)
{
    dds->acc += dds->step;

    // now scale down precision and  make sure that
    // index wraps around properly
    return (dds->acc >> DDS_ACC_SHIFT)%DDS_TBL_SIZE;
}


void 	softdds_setfreq(__IO SoftDds* dds, float freq, ulong samp_rate, uchar smooth);

void 	softdds_runf(float32_t *i_buff, float32_t *q_buff, ushort size);


void    softdds_setfreq_dbl(float freq[2],ulong samp_rate,uchar smooth);
void    softdds_runf_dbl(float32_t *i_buff, float32_t *q_buff, ushort size);
#endif

