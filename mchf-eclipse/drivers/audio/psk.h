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
#include "softdds.h"

#define PSK_OFFSET 1000
#define SAMPLE_MAX 32766

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
    uint16_t zeros;
    char* label;
} psk_speed_item_t;

typedef struct
{
	uint16_t rate;

	uint16_t tx_idx;
	uint8_t tx_char;
	uint16_t tx_bits;
	int16_t tx_wave_sign;
	int16_t tx_wave_prev;
	uint16_t tx_bit_phase;
	uint32_t tx_bit_len;
	int16_t tx_zeros;
	int16_t tx_ones;
	bool tx_ending;
	bool tx_win;
} PskState;

extern const psk_speed_item_t psk_speeds[PSK_SPEED_NUM];
extern PskState psk_state;

typedef struct
{
    psk_speed_t speed_idx;
}  psk_ctrl_t;

extern psk_ctrl_t psk_ctrl_config;

void PskDecoder_Init(void);
void Bpsk_ModulatorInit(void);
void BpskDecoder_ProcessSample(float32_t sample);
int16_t Psk_Modulator_GenSample();

#endif
