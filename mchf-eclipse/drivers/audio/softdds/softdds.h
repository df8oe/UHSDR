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

// we use a 32 bit accumulator
#define SOFTDDS_ACC_SHIFT       (32-DDS_TBL_BITS)

// Soft DDS public structure
typedef struct
{
	// DDS accumulator
	uint32_t   acc;

	// DDS step - not working if part of the structure
	uint32_t   step;
} soft_dds_t;


/**
 * Execute a single step in the sinus generation
 */
inline uint32_t softdds_nextSampleIndex(soft_dds_t* dds)
{
    uint32_t retval = (dds->acc >> SOFTDDS_ACC_SHIFT)%DDS_TBL_SIZE;

	dds->acc += dds->step;

	// now scale down precision and  make sure that
	// index wraps around properly
	return retval;
}

/*
 * Get the index which represents a -90 degree shift compared to
 * k, i.e. get  k = sin(a) => cos(a)
 */
inline uint32_t softdds_phase_shift90(uint32_t k)
{
    // make sure that
    // index wraps around properly
    return (k+(3*DDS_TBL_SIZE/4))%DDS_TBL_SIZE;
}


/**
 * Execute a single step in the sinus generation and return actual sample value
 */
inline int16_t softdds_nextSample(soft_dds_t* dds)
{
	return DDS_TABLE[softdds_nextSampleIndex(dds)];
}


void softdds_setFreqDDS(soft_dds_t* dds, float32_t freq, uint32_t samp_rate, uint8_t smooth);
void softdds_genIQSingleTone(soft_dds_t* dds, float32_t *i_buff,float32_t *q_buff,uint16_t size);
void softdds_genIQTwoTone(soft_dds_t* ddsA, soft_dds_t* ddsB, float *i_buff,float *q_buff,ushort size);

void softdds_addSingleTone(soft_dds_t* dds_ptr, float32_t* buffer, const size_t blockSize, float32_t scaling);
void softdds_addSingleToneToTwobuffers(soft_dds_t* dds_ptr, float32_t* buffer1, float32_t* buffer2, const size_t blockSize, float32_t scaling);

void softdds_runIQ(float32_t *i_buff, float32_t *q_buff, uint16_t size);
void softdds_configRunIQ(float32_t freq[2],uint32_t samp_rate,uint8_t smooth);


#endif

