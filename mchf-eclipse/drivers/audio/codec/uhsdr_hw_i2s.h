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

#define BUFF_LEN 128

typedef struct
{
    int16_t out[BUFF_LEN];
    int16_t in[BUFF_LEN];
} dma_audio_buffer_t;


#if defined(UI_BRD_MCHF)
#define DMA_AUDIO_NUM 1
#elif defined(UI_BRD_OVI40)
#define DMA_AUDIO_NUM 2
#endif

extern __IO dma_audio_buffer_t audio_buf[DMA_AUDIO_NUM];


void MchfHw_Codec_StartDMA();
void MchfHw_Codec_StopDMA();

#endif

