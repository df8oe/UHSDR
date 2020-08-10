/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               UHSDR FIRMWARE                                    **
**                                                                                 **
**---------------------------------------------------------------------------------**
**  Licence:		GNU GPLv3, see LICENSE.md                                                      **
************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PSK_H
#define __PSK_H

#include "uhsdr_types.h"

// externally defined for UiSpectrum markers
#define PSK_OFFSET 500
#define PSK_SNAP_RANGE 100 // defines the range within which the SNAP algorithm in spectrum.c searches for the PSK carrier


typedef enum {
    PSK_SPEED_31,
    PSK_SPEED_63,
	PSK_SPEED_125,
    PSK_SPEED_NUM
} psk_speed_t;

typedef struct
{
    psk_speed_t id;
    float32_t value;
    const float32_t* bpf_b;
    const float32_t* bpf_a;
    uint16_t rate;
    char* label;
} psk_speed_item_t;


extern const psk_speed_item_t psk_speeds[PSK_SPEED_NUM];

typedef struct
{
    psk_speed_t speed_idx;
}  psk_ctrl_t;

extern psk_ctrl_t psk_ctrl_config;

typedef enum {
    PSK_MOD_OFF,
    PSK_MOD_PREAMBLE,
    PSK_MOD_ACTIVE,
    PSK_MOD_POSTAMBLE,
    PSK_MOD_INACTIVE
} psk_modulator_t;

psk_modulator_t Psk_Modulator_GetState(void);
psk_modulator_t Psk_Modulator_SetState(psk_modulator_t newState);


void Psk_Modem_Init(uint32_t output_sample_rate);
void Psk_Modulator_PrepareTx(void);
void Psk_Demodulator_ProcessSample(float32_t sample);
int16_t Psk_Modulator_GenSample(void);

#endif
