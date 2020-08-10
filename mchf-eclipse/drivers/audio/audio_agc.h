/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  Description:   Code for automatic gain control based on the WDSP Lib          **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_AGC_H
#define __AUDIO_AGC_H

#include "uhsdr_board_config.h"
#include "uhsdr_types.h"

// these are the control parameter
// for external runtime configuration
// of the AGC
typedef struct
{
    uint8_t mode;
    uint8_t slope;
    uint8_t hang_enable;
    int     thresh;
    int     hang_thresh;
    int hang_time;
    uint8_t action;
    uint8_t switch_mode;
    uint8_t hang_action;
    int tau_decay[6];
    int tau_hang_decay;
} agc_wdsp_params_t;


extern agc_wdsp_params_t agc_wdsp_conf;

void AudioAgc_RunAgcWdsp(int16_t blockSize, float32_t (*agcbuffer)[AUDIO_BLOCK_SIZE], const bool use_stereo );
void AudioAgc_SetupAgcWdsp(float32_t sample_rate, bool remove_dc);
void AudioAgc_AgcWdsp_Init(void);


#endif
