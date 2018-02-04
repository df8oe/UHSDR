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

#define PSK_OFFSET 500
#define PSK_SNAP_RANGE 100 // defines the range within which the SNAP algorithm in spectrum.c searches for the PSK carrier
#define PSK_SAMPLE_RATE 12000 // TODO This should come from elsewhere, to be fixed
#define PSK_BND_FLT_LEN 5
#define PSK_BUF_LEN (PSK_SAMPLE_RATE / PSK_OFFSET)
#define PSK_SHIFT_DIFF (1.0 * PSK_OFFSET / PSK_SAMPLE_RATE)
#define PSK_SHIFT_90 (PSK_BUF_LEN / 4)
#define SAMPLE_MAX 32766
#define PSK_COS_DDS_LEN 64
#define PSK_MAX_SYMBOL_BUF 384 // Maximum for 12000 sample rate and 31.25; for higher sample rate should value be increased

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
    float32_t* bpf_b;
    float32_t* bpf_a;
    uint16_t rate;
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

	float32_t rx_phase;
	float32_t rx_samples_in[PSK_BND_FLT_LEN];
	float32_t rx_samples[PSK_BND_FLT_LEN];
	int16_t rx_bnd_idx;
	float32_t rx_vco[PSK_BUF_LEN];
	float32_t rx_sin_prod[PSK_BUF_LEN];
	float32_t rx_cos_prod[PSK_BUF_LEN];
	float32_t rx_scmix[PSK_BUF_LEN];
	int32_t rx_idx;
	int8_t rx_last_bit;
	float32_t rx_err;
	float32_t rx_last_symbol;
	int16_t rx_symbol_len;
	int16_t rx_symbol_idx;
	// float32_t rx_symbol_buf[PSK_MAX_SYMBOL_BUF];
	uint32_t rx_word;

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
