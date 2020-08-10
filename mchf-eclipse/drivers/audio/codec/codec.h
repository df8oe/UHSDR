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

#ifndef __CODEC_H
#define __CODEC_H

typedef enum
{
    WORD_SIZE_16 =	0,
    WORD_SIZE_24 =  2,
    WORD_SIZE_32 =  3,
} CodecSampleWidth_t;

#include "ui_configuration.h"
#ifdef UI_BRD_MCHF
#define CODEC_SPEAKER_MAX_VOLUME    16
#else
#define CODEC_SPEAKER_MAX_VOLUME AUDIO_GAIN_MAX
#endif

// --------------------------------------------------

/**
 * @brief setups up the codec according to tx/rx mode and selected sources
 * @param txrx_mode the mode for which it should be configured
 *
 */
void     Codec_SwitchTxRxMode(uint8_t txrx_mode);
void     Codec_MCUInterfaceInit(uint32_t AudioFreq);
void 	 Codec_VolumeLineOut(uint8_t txrx_mode);
void     Codec_VolumeSpkr(uint8_t vol);
void 	 Codec_LineInGainAdj(uint8_t gain);
void     Codec_IQInGainAdj(uint8_t gain);
void 	 Codec_MuteDAC(bool state);

uint32_t Codec_Reset(uint32_t AudioFreq);
void     Codec_RestartI2S(void);
void     Codec_TxSidetoneSetgain(uint8_t mode);
void     Codec_SwitchMicTxRxMode(uint8_t mode);
void     Codec_PrepareTx(uint8_t current_txrx_mode);

bool     Codec_ReadyForIrqCall(void);
#endif
