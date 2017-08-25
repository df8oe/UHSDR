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
	dds->acc += dds->step;

	// now scale down precision and  make sure that
	// index wraps around properly
	return (dds->acc >> DDS_ACC_SHIFT)%DDS_TBL_SIZE;
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


void softdds_runIQ(float32_t *i_buff, float32_t *q_buff, uint16_t size);
void softdds_configRunIQ(float32_t freq[2],uint32_t samp_rate,uint8_t smooth);


#endif

