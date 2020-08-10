/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __TX_PROCESSOR_H
#define __TX_PROCESSOR_H

#include "uhsdr_board_config.h"
#include "uhsdr_types.h"
#include "audio_driver.h" // for types

void TxProcessor_Init(void);
void TxProcessor_Set(uint8_t dmod_mode);
void TxProcessor_PrepareRun(void);
void TxProcessor_Run(AudioSample_t * const srcCodec, IqSample_t * const dst, AudioSample_t * const audioDst, uint16_t blockSize, bool external_mute);
#endif
