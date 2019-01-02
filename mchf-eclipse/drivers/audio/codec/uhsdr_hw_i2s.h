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

#ifndef __MCHF_HW_I2S_H
#define __MCHF_HW_I2S_H

#define IQ_SAMPLES_PER_BLOCK 32
#define AUDIO_SAMPLES_PER_BLOCK 32

/*
 * BUFF_LEN is derived from 2* 32 LR Samples per Audio-Interrupt (== 64 * int16_t )
 * since we get half of the buffer in each DMA Interrupt for processing
 */
// #define BUFF_LEN (4*IQ_SAMPLES_PER_BLOCK)

void UhsdrHwI2s_Codec_StartDMA();
void UhsdrHwI2s_Codec_StopDMA();

void UhsdrHwI2s_Codec_ClearTxDmaBuffer();

#endif

