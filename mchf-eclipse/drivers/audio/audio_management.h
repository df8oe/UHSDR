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

void    AudioManagement_CalcTxCompLevel(void);
void    AudioManagement_CalcNB_AGC(void);
void    AudioManagement_CalcAGCVals(void);
void    AudioManagement_CalcRFGain(void);
void    AudioManagement_CalcALCDecay(void);
void    AudioManagement_CalcAGCDecay(void);
void    AudioManagement_SetSidetoneForDemodMode(uint8_t dmod_mode, bool tune_mode);

void    AudioManagement_LoadToneBurstMode(void);
void    AudioManagement_CalcSubaudibleGenFreq(float32_t freq);        // load/set current FM subaudible tone settings for generation
void    AudioManagement_CalcSubaudibleDetFreq(float32_t freq);

void    AudioManagement_KeyBeep(void);
void    AudioManagement_KeyBeepPrepare(void);

#define IQ_BALANCE_OFF  -128 // Minimum setting for IQ balance
#define IQ_BALANCE_MAX  127 // Maximum setting for IQ balance

#define MIN_IQ_GAIN_BALANCE  IQ_BALANCE_OFF // Minimum setting for IQ gain balance
#define MAX_IQ_GAIN_BALANCE  IQ_BALANCE_MAX  // Maximum setting for IQ gain balance

#define MIN_IQ_PHASE_BALANCE IQ_BALANCE_OFF // Minimum setting for IQ phase balance
#define MAX_IQ_PHASE_BALANCE IQ_BALANCE_MAX  // Maximum setting for IQ phase balance

typedef struct
{
    int32_t phase;
    int32_t gain;
} iq_adjust_balance_t;

typedef enum
{
    IQ_RX_GAIN,
    IQ_RX_PHASE,
    IQ_TX_TRANS_ON_GAIN,
    IQ_TX_TRANS_OFF_GAIN,
    IQ_TX_TRANS_ON_PHASE,
    IQ_TX_TRANS_OFF_PHASE,

} iq_adjust_params_t;



typedef struct {
   iq_adjust_balance_t rx;
   iq_adjust_balance_t tx[2];
} iq_adjust_vals_t;

typedef struct
{
    int32_t freq;
    iq_adjust_vals_t adj;
}  freq_adjust_point_t;


// this data structure(s) must have at least 2 elements (including the stop element)
// corresponding frequencies are stored in const array iq_adjust_freq
typedef enum {
    IQ_80M = 0,
    IQ_20M,
    IQ_15M,
    IQ_10M,
    IQ_10M_UP,
    IQ_FREQ_NUM
} iq_freq_enum_t;

// the order here must match the order in the structure below!

extern freq_adjust_point_t iq_adjust[];


#endif /* DRIVERS_AUDIO_AUDIO_MANAGEMENT_H_ */
