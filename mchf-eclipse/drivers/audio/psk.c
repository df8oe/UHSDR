/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                               UHSDR FIRMWARE                                    **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Licence:        GNU GPLv3, see LICENSE.md                                                      **
 ************************************************************************************/

// Common
#include "psk.h"
#include "ui_driver.h"
#include "rtty.h"
#include <stdlib.h>

uint16_t psk_varicode[] = {
		0b1         , // SPACE
		0b111111111 , // !
		0b101011111 , // "
		0b111110101 , // #
		0b111011011 , // $
		0b1011010101, // %
		0b1010111011, // &
		0b101111111 , // '
		0b11111011  , // (
		0b11110111  , // )
		0b101101111 , // *
		0b111011111 , // +
		0b1110101   , // ,
		0b110101    , // -
		0b1010111   , // .
		0b110101111 , // /
		0b10110111  , // 0
		0b10111101  , // 1
		0b11101101  , // 2
		0b11111111  , // 3
		0b101110111 , // 4
		0b101011011 , // 5
		0b101101011 , // 6
		0b110101101 , // 7
		0b110101011 , // 8
		0b110110111 , // 9
		0b11110101  , // :
		0b110111101 , // ;
		0b111101101 , // <
		0b1010101   , // =
		0b111010111 , // >
		0b1010101111, // ?
		0b1010111101, // @
		0b1111101   , // A
		0b11101011  , // B
		0b10101101  , // C
		0b10110101  , // D
		0b1110111   , // E
		0b11011011  , // F
		0b11111101  , // G
		0b101010101 , // H
		0b1111111   , // I
		0b111111101 , // J
		0b101111101 , // K
		0b11010111  , // L
		0b10111011  , // M
		0b11011101  , // N
		0b10101011  , // O
		0b11010101  , // P
		0b111011101 , // Q
		0b10101111  , // R
		0b1101111   , // S
		0b1101101   , // T
		0b101010111 , // U
		0b110110101 , // V
		0b101011101 , // W
		0b101110101 , // X
		0b101111011 , // Y
		0b1010101101, // Z
		0b111110111 , // [
		0b111101111 , // backslash
		0b111111011 , // ]
		0b1010111111, // ^
		0b101101101 , // _
		0b1011011111, // `
		0b1011      , // a
		0b1011111   , // b
		0b101111    , // c
		0b101101    , // d
		0b11        , // e
		0b111101    , // f
		0b1011011   , // g
		0b101011    , // h
		0b1101      , // i
		0b111101011 , // j
		0b10111111  , // k
		0b11011     , // l
		0b111011    , // m
		0b1111      , // n
		0b111       , // o
		0b111111    , // p
		0b110111111 , // q
		0b10101     , // r
		0b10111     , // s
		0b101       , // t
		0b110111    , // u
		0b1111011   , // v
		0b1101011   , // w
		0b11011111  , // x
		0b1011101   , // y
		0b111010101 , // z
		0b1010110111, // {
		0b110111011 , // |
		0b1010110101, // }
		0b1011010111 // ~
};

#define PSK_BND_FLT_LEN 5

float32_t PskBndPassB[] = {
		0.00068698,
		0,
		-0.00137396,
		0,
		0.00068698
};

float32_t PskBndPassA[] = {
		1,
		-3.40096044,
		4.81713149,
		-3.27487565,
		0.92725284
};

float32_t bnd_ibuffer[PSK_BND_FLT_LEN];
float32_t bnd_obuffer[PSK_BND_FLT_LEN];

soft_dds_t psk_dds;
soft_dds_t psk_bit_dds;

void PskBufAdd(int len, float32_t buf[], float32_t v)
{
	for (int i = len - 1; i > 0; i--)
	{
		buf[i] = buf[i-1];
	}
	buf[0] = v;
}


#define PSK_VARICODE_NUM (sizeof(psk_varicode)/sizeof(*psk_varicode))


const psk_speed_item_t psk_speeds[PSK_SPEED_NUM] =
{
		{ .id =PSK_SPEED_31, .value = 31.25, .zeros = 50, .label = " 31" },
		{ .id =PSK_SPEED_63, .value = 62.5, .zeros = 100, .label = " 63"  },
		{ .id =PSK_SPEED_125, .value = 125.0, .zeros = 200, .label = "125" }
};

psk_ctrl_t psk_ctrl_config =
{
		.speed_idx = PSK_SPEED_31
};

PskState  psk_state;

void Bpsk_ResetWin() {
	softdds_setFreqDDS(&psk_bit_dds, psk_speeds[psk_ctrl_config.speed_idx].value / 2, ts.samp_rate, true);
}

void Bpsk_ModulatorInit(void)
{
	psk_state.tx_ones = 0;
	psk_state.tx_win = true;
	psk_state.tx_char = '\0';
	psk_state.tx_bits = 0;
	psk_state.tx_wave_sign = 1;
	psk_state.tx_wave_prev = 1;
	psk_state.tx_bit_phase = 0;
	psk_state.tx_ending = false;
	Bpsk_ResetWin();
}


void PskDecoder_Init(void)
{

	switch(psk_ctrl_config.speed_idx)
	{
	case PSK_SPEED_31:
		psk_state.rate = 384;
		break;
	case PSK_SPEED_63:
		psk_state.rate = 96;
		break;
	case PSK_SPEED_125:
		psk_state.rate = 48;
		break;
	default:
	{
		psk_state.rate = 384;
	}
	}

	psk_state.tx_idx = 0;

	softdds_setFreqDDS(&psk_dds, PSK_OFFSET, ts.samp_rate, true);
	psk_state.tx_bit_len = lround(ts.samp_rate / psk_speeds[psk_ctrl_config.speed_idx].value * 2);
	psk_state.tx_zeros = - psk_speeds[psk_ctrl_config.speed_idx].zeros;
	Bpsk_ModulatorInit();
}


char Bpsk_DecodeVaricode(uint16_t code)
{
	char result = '*';
	for (int i = 0; i<PSK_VARICODE_NUM; i++) {
		if (psk_varicode[i] == code)
		{
			result = ' ' + i;
			break;
		}
	}
	return result;
}

uint16_t Bpsk_FindCharReversed(uint8_t c)
{
	uint16_t code, retval;

	if (c < ' ' || c > '~')
	{
		return 0;
	}

	code = psk_varicode[c - ' '];
	retval = 0;

	while(code > 0)
	{
		retval *= 2;
		retval += code % 2;
		code /= 2;
	}
	return retval;
}

void BpskDecoder_ProcessSample(float32_t sample)
{
}


int16_t Psk_Modulator_GenSample()
{
	int16_t retval;
	int32_t coeff = SAMPLE_MAX;

	if (psk_state.tx_bit_phase == psk_state.tx_bit_len / 4)
	{
		if (psk_state.tx_bits == 0)
		{
			if (psk_state.tx_zeros < 2)
			{
				psk_state.tx_zeros++;
			}
			else if (DigiModes_TxBufferHasData())
			{
				DigiModes_TxBufferRemove(&psk_state.tx_char);
				psk_state.tx_bits = Bpsk_FindCharReversed(psk_state.tx_char);
				psk_state.tx_zeros = 0;
				psk_state.tx_ones = 0;
			}
			else if (ts.buffered_tx)
			{
				if (psk_state.tx_ones < psk_speeds[psk_ctrl_config.speed_idx].zeros)
				{
					psk_state.tx_ones++;
				}
				else
				{
					psk_state.tx_ending = true;
				}
			}
		}

		if (psk_state.tx_bits % 2 == 0 && psk_state.tx_ones == 0)
		{
			psk_state.tx_wave_sign *= -1;
		}

		if (psk_state.tx_wave_sign != psk_state.tx_wave_prev || psk_state.tx_ending)
		{
			psk_state.tx_win = true;
		}
		else
		{
			psk_state.tx_win = false;
		}

		psk_state.tx_bits /= 2;
	}

	if (psk_state.tx_bit_phase == 0)
	{
		psk_state.tx_wave_prev = psk_state.tx_wave_sign;
		Bpsk_ResetWin();
		if (psk_state.tx_ending)
		{
			ts.tx_stop_req = true;
		}
	}

	if (psk_state.tx_win)
	{
		coeff = abs(softdds_nextSample(&psk_bit_dds));
	}
	psk_state.tx_bit_phase = (psk_state.tx_bit_phase + 1) % (psk_state.tx_bit_len / 2);
	retval =  coeff * psk_state.tx_wave_prev * softdds_nextSample(&psk_dds) / SAMPLE_MAX;
	return retval;

}
