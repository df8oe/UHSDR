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
#include "radio_management.h"
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


float32_t PskBndPassB_31[] = {
		6.6165543533213894e-05,
		0.0,
		-0.00013233108706642779,
		0.0,
		6.6165543533213894e-05
};

float32_t PskBndPassA_31[] = {
		1.0,
		-3.8414813063247664,
		5.6662277107033248,
		-3.7972899991488904,
		0.9771256616899302
};

float32_t PskBndPassB_63[] = {
		0.0002616526950658905,
		0.0,
		-0.000523305390131781,
		0.0,
		0.0002616526950658905
};

float32_t PskBndPassA_63[] = {
		1.0,
		-3.8195192250239174,
		5.6013869366249818,
		-3.7321386869273105,
		0.95477455992103932
};

float32_t PskBndPassB_125[] = {
		0.0010232176384709002,
		0.0,
		-0.0020464352769418003,
		0.0,
		0.0010232176384709002
};

float32_t PskBndPassA_125[] = {
		1.0,
		-3.7763786572915334,
		5.4745855184361272,
		-3.6055008493327723,
		0.91159449659996006
};

float32_t PskCosDDS[] = { 6.123233995736766e-17, -0.09801714032956066, -0.1950903220161282,
		-0.2902846772544622, -0.3826834323650897, -0.4713967368259977, -0.555570233019602,
		-0.6343932841636453, -0.7071067811865475, -0.773010453362737, -0.8314696123025453,
		-0.8819212643483549, -0.9238795325112867, -0.9569403357322088, -0.9807852804032304,
		-0.9951847266721968, -1.0, -0.9951847266721969, -0.9807852804032304, -0.9569403357322089,
		-0.9238795325112867, -0.881921264348355, -0.8314696123025455, -0.7730104533627371,
		-0.7071067811865477, -0.6343932841636453, -0.5555702330196022, -0.47139673682599786,
		-0.38268343236509034, -0.2902846772544624, -0.19509032201612866, -0.09801714032956045,
		-1.8369701987210297e-16, 0.09801714032956009, 0.19509032201612828, 0.2902846772544621,
		0.38268343236509, 0.4713967368259976, 0.5555702330196018, 0.6343932841636448, 0.7071067811865475,
		0.7730104533627367, 0.8314696123025452, 0.8819212643483549, 0.9238795325112865, 0.9569403357322088,
		0.9807852804032303, 0.9951847266721969, 1.0, 0.9951847266721969, 0.9807852804032304, 0.9569403357322089,
		0.9238795325112867, 0.881921264348355, 0.8314696123025456, 0.7730104533627369, 0.7071067811865477,
		0.634393284163646, 0.5555702330196023, 0.47139673682599803, 0.3826834323650905, 0.29028467725446255,
		0.19509032201612878, 0.09801714032956058 }; // TODO to replace with soft_dds

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

float32_t Psk_IirNext(float32_t b[], float32_t a[], float32_t x[], float32_t y[], int idx, int taps)
{
	float32_t resp = 0;
	int iidx; 

	for (int i = 0; i < taps; i++)
	{
		iidx = (idx - i + taps) % taps;
		resp += b[i] * x[iidx];
		if (i>0)
		{
			resp -= a[i] * y[iidx];
		}
	}
	return resp / a[0];
}


#define PSK_VARICODE_NUM (sizeof(psk_varicode)/sizeof(*psk_varicode))


const psk_speed_item_t psk_speeds[PSK_SPEED_NUM] =
{
		{ .id =PSK_SPEED_31, .value = 31.25, .zeros = 50, .bpf_b = PskBndPassB_31, .bpf_a = PskBndPassA_31, .rate = 384, .label = " 31" },
		{ .id =PSK_SPEED_63, .value = 62.5, .zeros = 100, .bpf_b = PskBndPassB_63, .bpf_a = PskBndPassA_63, .rate = 192, .label = " 63"  },
		{ .id =PSK_SPEED_125, .value = 125.0, .zeros = 200, .bpf_b = PskBndPassB_125, .bpf_a = PskBndPassA_125, .rate = 96, .label = "125" }
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

void Bpsk_DemodulatorInit(void)
{
	psk_state.rx_phase = 0;
	psk_state.rx_bnd_idx = 0;
	
	for (int i = 0; i < PSK_BND_FLT_LEN; i++)
	{
		psk_state.rx_samples_in[i] = 0;
		psk_state.rx_samples[i] = 0;
	}
	
	for (int i = 0; i < PSK_BUF_LEN; i++)
	{
		psk_state.rx_vco[i] = 0;
		psk_state.rx_sin_prod[i] = 0;
		psk_state.rx_cos_prod[i] = 0;
		psk_state.rx_scmix[i] = 0;
	}

	psk_state.rx_idx = 0;
	psk_state.rx_last_bit = 0;
	psk_state.rx_err = 0;
	psk_state.rx_last_symbol = 0;
	psk_state.rx_symbol_len = psk_state.rate / PSK_BUF_LEN;
	psk_state.rx_symbol_idx = 0;
	
	// for (int i = 0; i < psk_state.rx_symbol_len; i ++)
	// {
	// 	psk_state.rx_symbol_buf[i];
	// }

	psk_state.rx_word = 0;
	UiDriver_TextMsgPutChar('>');
}


void PskDecoder_Init(void)
{

	psk_state.tx_idx = 0;

	softdds_setFreqDDS(&psk_dds, PSK_OFFSET, ts.samp_rate, true);
	psk_state.tx_bit_len = lround(ts.samp_rate / psk_speeds[psk_ctrl_config.speed_idx].value * 2);
	psk_state.tx_zeros = - psk_speeds[psk_ctrl_config.speed_idx].zeros;
	psk_state.rate = psk_speeds[psk_ctrl_config.speed_idx].rate;
	Bpsk_ModulatorInit();
	Bpsk_DemodulatorInit();
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
	float32_t fsample, sum_sin = 0, sum_cos = 0, symbol_out, smax = 0;
	int8_t bit;

	psk_state.rx_samples_in[psk_state.rx_bnd_idx] = sample;
	fsample = Psk_IirNext(psk_speeds[psk_ctrl_config.speed_idx].bpf_b, psk_speeds[psk_ctrl_config.speed_idx].bpf_a, psk_state.rx_samples_in,
		psk_state.rx_samples, psk_state.rx_bnd_idx, PSK_BND_FLT_LEN);
	psk_state.rx_samples[psk_state.rx_bnd_idx] = fsample;
	psk_state.rx_bnd_idx++;
	psk_state.rx_bnd_idx %= PSK_BND_FLT_LEN;

	psk_state.rx_vco[psk_state.rx_idx] = PskCosDDS[(int)(psk_state.rx_phase * PSK_COS_DDS_LEN)]; // TODO This shall be replaced with soft_dds
	psk_state.rx_sin_prod[psk_state.rx_idx] = psk_state.rx_vco[(psk_state.rx_idx - PSK_SHIFT_90 + PSK_BUF_LEN) % PSK_BUF_LEN] * fsample;
	psk_state.rx_cos_prod[psk_state.rx_idx] = psk_state.rx_vco[psk_state.rx_idx] * fsample;

	for (int i = 0; i < PSK_BUF_LEN; i++)
	{
		sum_sin += psk_state.rx_sin_prod[i]; // Could be optimized keeping the sum in memory
		sum_cos += psk_state.rx_cos_prod[i]; //
	}

	symbol_out = sum_sin / PSK_BUF_LEN;
	psk_state.rx_scmix[psk_state.rx_idx] = symbol_out * sum_cos / PSK_BUF_LEN;

	psk_state.rx_err = 0;
	smax = 0;
	for (int i = 0; i < PSK_BUF_LEN; i++)
	{
		psk_state.rx_err += psk_state.rx_scmix[i]; // Could be optimized keeping the sum in memory
		if (fabsf(psk_state.rx_sin_prod[i]) > smax)
		{
			smax = fabsf(psk_state.rx_sin_prod[i]);
		}
		if (fabsf(psk_state.rx_cos_prod[i]) > smax)
		{
			smax = fabsf(psk_state.rx_cos_prod[i]);
		}
	}

	if (smax != 0)
	{
		psk_state.rx_err /= PSK_BUF_LEN * smax * 4;
	}
	else
	{
		psk_state.rx_err /= PSK_BUF_LEN;
	}

	if(fabsf(psk_state.rx_err) > 0.1)
	{
		psk_state.rx_err = 0.1 * ((psk_state.rx_err > 0) ? 1 : -1 );
	}

	psk_state.rx_idx += 1;
	psk_state.rx_idx %= PSK_BUF_LEN;

	psk_state.rx_phase += PSK_SHIFT_DIFF + psk_state.rx_err * PSK_SHIFT_DIFF;
	
	if (psk_state.rx_phase > 1)
	{
		psk_state.rx_phase -= 1;
		psk_state.rx_symbol_idx += 1;
		if (psk_state.rx_symbol_idx >= psk_state.rx_symbol_len)
		{
			psk_state.rx_symbol_idx = 0;
			// TODO here should come additional part to check if timing of sampling should be moved
			if (psk_state.rx_last_symbol * symbol_out < 0)
			{
				bit = 0;
			} 
			else
			{
				bit = 1;
			}

			psk_state.rx_last_symbol = symbol_out;

			if (psk_state.rx_last_bit == 0 && bit == 0 && psk_state.rx_word != 0)
			{
				UiDriver_TextMsgPutChar(Bpsk_DecodeVaricode(psk_state.rx_word / 2));
				psk_state.rx_word = 0;
			}
			else
			{
				psk_state.rx_word = 2 * psk_state.rx_word + bit;
			}

			psk_state.rx_last_bit = bit;

		}
	}

	if (psk_state.rx_phase < 0)
	{
		psk_state.rx_phase += 1;
	}

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
		    RadioManagement_Request_TxOff();
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
