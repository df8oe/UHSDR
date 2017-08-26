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

// Credits - SDR cube!!!

// Common
#include "uhsdr_board.h"
#include "dds_table.h"
#include "softdds.h"




// Two Tone Dds
soft_dds_t dbldds[2];

uint32_t softdds_stepForSampleRate(float32_t freq, uint32_t samp_rate)
{
    uint64_t freq64_shifted = freq * DDS_TBL_SIZE;
    freq64_shifted <<= DDS_ACC_SHIFT;
    uint64_t step = freq64_shifted / samp_rate;
    return step;
}

/**
 * Initialize softdds for given frequency and sample rate
 */
void softdds_setFreqDDS(soft_dds_t* softdds_p, float32_t freq, uint32_t samp_rate,uint8_t smooth)
{
    // Reset accumulator, if need smooth tone
    // transition, do not reset it (e.g. wspr)
    if(smooth == false)
    {
        softdds_p->acc = 0;
    }
    // Calculate new step
    softdds_p->step = softdds_stepForSampleRate(freq,samp_rate);
}


/**
 * Initialize softdds for given frequency and sample rate
 */

void softdds_configRunIQ(float freq[2],uint32_t samp_rate,uint8_t smooth)
{
    softdds_setFreqDDS(&dbldds[0],freq[0],samp_rate,smooth);
    softdds_setFreqDDS(&dbldds[1],freq[1],samp_rate,smooth);
  }


/*
 * Get the index which represents a -90 degree shift compared to
 * k, i.e. get  k = sin(a) => cos(a)
 */
static inline uint32_t softdds_phase_shift90(uint32_t k)
{
    // make sure that
    // index wraps around properly
    return (k+(3*DDS_TBL_SIZE/4))%DDS_TBL_SIZE;
}


void softdds_genIQSingleTone(soft_dds_t* dds, float32_t *i_buff,float32_t *q_buff,uint16_t size)
{
	for(uint16_t i = 0; i < size; i++)
	{
		// Calculate next sample
		uint32_t k    = softdds_nextSampleIndex(dds);

		// Load I value (sin)
		*i_buff = DDS_TABLE[k];

		// -90 degrees shift (cos)
		// Load Q value
		*q_buff = DDS_TABLE[softdds_phase_shift90(k)];

		// Next ptr
		i_buff++;
		q_buff++;
	}
}

/*
 * Generates the addition  of two sinus frequencies as IQ data stream
 * min/max value is +/-2^15-1
 * Frequencies need to be configured using softdds_setfreq_dbl
 */
void softdds_genIQTwoTone(soft_dds_t* ddsA, soft_dds_t* ddsB, float *i_buff,float *q_buff,ushort size)
{
    ulong   i,k[2];

    for(i = 0; i < size; i++)
    {
        // Calculate next sample
        k[0]    = softdds_nextSampleIndex(ddsA);
        k[1]    = softdds_nextSampleIndex(ddsB);

        // Load I value 0.5*(sin(a)+sin(b))
        *i_buff = ((int32_t)DDS_TABLE[k[0]] + (int32_t)DDS_TABLE[k[1]])/2;

        *q_buff = ((int32_t)DDS_TABLE[softdds_phase_shift90(k[0])] + (int32_t)DDS_TABLE[softdds_phase_shift90(k[1])])/2;

        // Next ptr
        i_buff++;
        q_buff++;
    }
}

/*
 * Generates the sinus frequencies as IQ data stream
 * min/max value is +/-2^15-1
 * Frequency needs to be configured using softdds_setfreq
 */
void softdds_runIQ(float32_t *i_buff,float32_t *q_buff,uint16_t size)
{
    if (dbldds[1].step>0.0)
    {
        softdds_genIQTwoTone(&dbldds[0], &dbldds[1], i_buff, q_buff,size);
    }
    else
    {
    	softdds_genIQSingleTone(&dbldds[0],i_buff,q_buff,size);
    }
}

