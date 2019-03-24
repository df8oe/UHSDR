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
    freq64_shifted <<= SOFTDDS_ACC_SHIFT;
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
    for(int i = 0; i < size; i++)
    {
        uint32_t k[2];
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

/**
 * Overlays an audio stream with a beep signal
 * @param dds The previously initialized dds configuration
 * @param buffer audio buffer of blockSize (mono/single channel) samples
 * @param blockSize
 * @param scaling scale the resulting sine wave with this factor
 */

void softdds_addSingleTone(soft_dds_t* dds_ptr, float32_t* buffer, const size_t blockSize, float32_t scaling)
{
    for(int i=0; i < blockSize; i++)                            // transfer to DMA buffer and do conversion to INT
    {
        buffer[i] += (float32_t)softdds_nextSample(dds_ptr) * scaling; // load indexed sine wave value, adding it to audio, scaling the amplitude and putting it on "b" - speaker (ONLY)
    }
}

/**
 * Overlays an audio stream with a beep signal
 * @param dds The previously initialized dds configuration
 * @param buffer1 audio buffer of blockSize (mono/single channel) samples
 * @param buffer2 audio buffer of blockSize (mono/single channel) samples
 * @param blockSize
 * @param scaling scale the resulting sine wave with this factor
 */

void softdds_addSingleToneToTwobuffers(soft_dds_t* dds_ptr, float32_t* buffer1, float32_t* buffer2, const size_t blockSize, float32_t scaling)
{
    float32_t Tone;
    for(int i=0; i < blockSize; i++)                            // transfer to DMA buffer and do conversion to INT
    {
        Tone=(float32_t) softdds_nextSample(dds_ptr) * scaling; // load indexed sine wave value, adding it to audio, scaling the amplitude and putting it on "b" - speaker (ONLY)
        buffer1[i] += Tone;
        buffer2[i] += Tone;
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

