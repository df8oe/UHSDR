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
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

// Credits - SDR cube!!!

// Common
#include "mchf_board.h"
#include "dds_table.h"
#include "softdds.h"


// Soft DDS public structure
typedef struct SoftDds
{
    // DDS accumulator
    ulong   acc;

    // DDS step - not working if part of the structure
    ulong   step;

} SoftDds;




// Software DDS public
__IO 	SoftDds	softdds;

// Two Tone Dds
__IO    SoftDds dbldds[2];

/**
 * Initialize softdds for given frequency and sample rate
 */
void softdds_setfreq(float freq,ulong samp_rate,uchar smooth)
{
    float f = samp_rate;

    // Reset accumulator, if need smooth tone
    // transition, do not reset it (e.g. wspr)
    if(!smooth)
    {
        softdds.acc = 0;
    }
    // Calculate new step
    f   	 /= 65536.0;
    softdds.step   = (ulong)(freq / f);
}

void softdds_setfreq_dbl(float freq[2],ulong samp_rate,uchar smooth)
{
    float f = (float)(samp_rate);

    // Reset accumulator, if need smooth tone
    // transition, do not reset it (e.g. wspr)
    if(!smooth)
    {
        dbldds[0].acc = 0;
        dbldds[1].acc = 0;
    }
    // Calculate new step
    f        /= 65536.0;
    dbldds[0].step   = (ulong)(freq[0] / f);
    dbldds[1].step   = (ulong)(freq[1] / f);
}


//*----------------------------------------------------------------------------
//* Function Name       : softdds_runf
//* Object              : use two float buffer
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void softdds_runf(float *i_buff,float *q_buff,ushort size)
{
    ulong 	i,k;

    if (dbldds[1].step>0.0)
    {
        softdds_runf_dbl(i_buff,q_buff,size);
    }
    else
    {
        for(i = 0; i < size; i++)
        {
            // Calculate next sample
            softdds.acc += softdds.step;
            k    = softdds.acc >> DDS_ACC_SHIFT;

            // Fix ptr overload
            k &= (DDS_TBL_SIZE - 1);

            // Load I value
            *i_buff = DDS_TABLE[k];

            // 90 degrees shift
            k += (DDS_TBL_SIZE/4);

            // Fix ptr overload
            k &= (DDS_TBL_SIZE - 1);

            // Load Q value
            *q_buff = DDS_TABLE[k];

            // Next ptr
            i_buff++;
            q_buff++;
        }
    }
}

void softdds_runf_dbl(float *i_buff,float *q_buff,ushort size)
{
    ulong   i,k[2];

    for(i = 0; i < size; i++)
    {
        // Calculate next sample
        dbldds[0].acc += dbldds[0].step;
        k[0]    = dbldds[0].acc >> DDS_ACC_SHIFT;
        dbldds[1].acc += dbldds[1].step;
        k[1]    = dbldds[1].acc >> DDS_ACC_SHIFT;

        // Fix ptr overload
        k[0] &= (DDS_TBL_SIZE - 1);
        k[1] &= (DDS_TBL_SIZE - 1);

        // Load I value
        *i_buff = ((int32_t)DDS_TABLE[k[0]] + (int32_t)DDS_TABLE[k[1]])/2;

        // 90 degrees shift
        k[0] += (DDS_TBL_SIZE/4);
        k[1] += (DDS_TBL_SIZE/4);

        // Fix ptr overload
        k[0] &= (DDS_TBL_SIZE - 1);
        k[1] &= (DDS_TBL_SIZE - 1);

        // Load Q value
        *q_buff = ((int32_t)DDS_TABLE[k[0]] + (int32_t)DDS_TABLE[k[1]])/2;

        // Next ptr
        i_buff++;
        q_buff++;
    }
}
