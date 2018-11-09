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
**  Licence:        GNU GPLv3                                                      **
************************************************************************************/

#ifndef DRIVERS_AUDIO_AUDIO_MANAGEMENT_H_
#define DRIVERS_AUDIO_AUDIO_MANAGEMENT_H_

#include "uhsdr_types.h"

void    AudioManagement_CalcIqPhaseGainAdjust(float32_t freq);

void    AudioManagement_CalcTxCompLevel();
void    AudioManagement_CalcNB_AGC();
void    AudioManagement_CalcAGCVals();
void    AudioManagement_CalcRFGain();
void    AudioManagement_CalcALCDecay();
void    AudioManagement_CalcAGCDecay();
void    AudioManagement_SetSidetoneForDemodMode(uint8_t dmod_mode, bool tune_mode);

void    AudioManagement_LoadToneBurstMode();
void    AudioManagement_CalcSubaudibleGenFreq();        // load/set current FM subaudible tone settings for generation
void    AudioManagement_CalcSubaudibleDetFreq();

void    AudioManagement_KeyBeep();
void    AudioManagement_KeyBeepPrepare();
void    AudioManagement_KeyBeepGenerate(float32_t* buffer, const size_t blockSize);

#define IQ_BALANCE_OFF  -128 // Minimum setting for IQ gain balance

#define MIN_IQ_GAIN_BALANCE  IQ_BALANCE_OFF // Minimum setting for IQ gain balance
#define MAX_IQ_GAIN_BALANCE  127  // Maximum setting for IQ gain balance

#define MIN_IQ_PHASE_BALANCE IQ_BALANCE_OFF // Minimum setting for IQ phase balance
#define MAX_IQ_PHASE_BALANCE 127  // Maximum setting for IQ phase balance



#endif /* DRIVERS_AUDIO_AUDIO_MANAGEMENT_H_ */
