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
#include "uhsdr_digi_buffer.h"

// table courtesy of fldigi pskvaricode.cxx
static uint16_t psk_varicode[] = {
    0b1010101011,       /*   0 - <NUL>  */
    0b1011011011,       /*   1 - <SOH>  */
    0b1011101101,       /*   2 - <STX>  */
    0b1101110111,       /*   3 - <ETX>  */
    0b1011101011,       /*   4 - <EOT>  */
    0b1101011111,       /*   5 - <ENQ>  */
    0b1011101111,       /*   6 - <ACK>  */
    0b1011111101,       /*   7 - <BEL>  */
    0b1011111111,       /*   8 - <BS>   */
    0b11101111,         /*   9 - <TAB>  */
    0b11101,            /*  10 - <LF>   */
    0b1101101111,       /*  11 - <VT>   */
    0b1011011101,       /*  12 - <FF>   */
    0b11111,            /*  13 - <CR>   */
    0b1101110101,       /*  14 - <SO>   */
    0b1110101011,       /*  15 - <SI>   */
    0b1011110111,       /*  16 - <DLE>  */
    0b1011110101,       /*  17 - <DC1>  */
    0b1110101101,       /*  18 - <DC2>  */
    0b1110101111,       /*  19 - <DC3>  */
    0b1101011011,       /*  20 - <DC4>  */
    0b1101101011,       /*  21 - <NAK>  */
    0b1101101101,       /*  22 - <SYN>  */
    0b1101010111,       /*  23 - <ETB>  */
    0b1101111011,       /*  24 - <CAN>  */
    0b1101111101,       /*  25 - <EM>   */
    0b1110110111,       /*  26 - <SUB>  */
    0b1101010101,       /*  27 - <ESC>  */
    0b1101011101,       /*  28 - <FS>   */
    0b1110111011,       /*  29 - <GS>   */
    0b1011111011,       /*  30 - <RS>   */
    0b1101111111,       /*  31 - <US>   */
    0b1,                /*  32 - <SPC>  */
    0b111111111,        /*  33 - !  */
    0b101011111,        /*  34 - '"'    */
    0b111110101,        /*  35 - #  */
    0b111011011,        /*  36 - $  */
    0b1011010101,       /*  37 - %  */
    0b1010111011,       /*  38 - &  */
    0b101111111,        /*  39 - '  */
    0b11111011,         /*  40 - (  */
    0b11110111,         /*  41 - )  */
    0b101101111,        /*  42 - *  */
    0b111011111,        /*  43 - +  */
    0b1110101,          /*  44 - ,  */
    0b110101,           /*  45 - -  */
    0b1010111,          /*  46 - .  */
    0b110101111,        /*  47 - /  */
    0b10110111,         /*  48 - 0  */
    0b10111101,         /*  49 - 1  */
    0b11101101,         /*  50 - 2  */
    0b11111111,         /*  51 - 3  */
    0b101110111,        /*  52 - 4  */
    0b101011011,        /*  53 - 5  */
    0b101101011,        /*  54 - 6  */
    0b110101101,        /*  55 - 7  */
    0b110101011,        /*  56 - 8  */
    0b110110111,        /*  57 - 9  */
    0b11110101,         /*  58 - :  */
    0b110111101,        /*  59 - ;  */
    0b111101101,        /*  60 - <  */
    0b1010101,          /*  61 - =  */
    0b111010111,        /*  62 - >  */
    0b1010101111,       /*  63 - ?  */
    0b1010111101,       /*  64 - @  */
    0b1111101,          /*  65 - A  */
    0b11101011,         /*  66 - B  */
    0b10101101,         /*  67 - C  */
    0b10110101,         /*  68 - D  */
    0b1110111,          /*  69 - E  */
    0b11011011,         /*  70 - F  */
    0b11111101,         /*  71 - G  */
    0b101010101,        /*  72 - H  */
    0b1111111,          /*  73 - I  */
    0b111111101,        /*  74 - J  */
    0b101111101,        /*  75 - K  */
    0b11010111,         /*  76 - L  */
    0b10111011,         /*  77 - M  */
    0b11011101,         /*  78 - N  */
    0b10101011,         /*  79 - O  */
    0b11010101,         /*  80 - P  */
    0b111011101,        /*  81 - Q  */
    0b10101111,         /*  82 - R  */
    0b1101111,          /*  83 - S  */
    0b1101101,          /*  84 - T  */
    0b101010111,        /*  85 - U  */
    0b110110101,        /*  86 - V  */
    0b101011101,        /*  87 - W  */
    0b101110101,        /*  88 - X  */
    0b101111011,        /*  89 - Y  */
    0b1010101101,       /*  90 - Z  */
    0b111110111,        /*  91 - [  */
    0b111101111,        /*  92 - \  */
    0b111111011,        /*  93 - ]  */
    0b1010111111,       /*  94 - ^  */
    0b101101101,        /*  95 - _  */
    0b1011011111,       /*  96 - `  */
    0b1011,             /*  97 - a  */
    0b1011111,          /*  98 - b  */
    0b101111,           /*  99 - c  */
    0b101101,           /* 100 - d  */
    0b11,               /* 101 - e  */
    0b111101,           /* 102 - f  */
    0b1011011,          /* 103 - g  */
    0b101011,           /* 104 - h  */
    0b1101,             /* 105 - i  */
    0b111101011,        /* 106 - j  */
    0b10111111,         /* 107 - k  */
    0b11011,            /* 108 - l  */
    0b111011,           /* 109 - m  */
    0b1111,             /* 110 - n  */
    0b111,              /* 111 - o  */
    0b111111,           /* 112 - p  */
    0b110111111,        /* 113 - q  */
    0b10101,            /* 114 - r  */
    0b10111,            /* 115 - s  */
    0b101,              /* 116 - t  */
    0b110111,           /* 117 - u  */
    0b1111011,          /* 118 - v  */
    0b1101011,          /* 119 - w  */
    0b11011111,         /* 120 - x  */
    0b1011101,          /* 121 - y  */
    0b111010101,        /* 122 - z  */
    0b1010110111,       /* 123 - {  */
    0b110111011,        /* 124 - |  */
    0b1010110101,       /* 125 - }  */
    0b1011010111,       /* 126 - ~  */
    0b1110110101,       /* 127 - <DEL>  */
    0b1110111101,       /* 128 -    */
    0b1110111111,       /* 129 -    */
    0b1111010101,       /* 130 -    */
    0b1111010111,       /* 131 -    */
    0b1111011011,       /* 132 -    */
    0b1111011101,       /* 133 -    */
    0b1111011111,       /* 134 -    */
    0b1111101011,       /* 135 -    */
    0b1111101101,       /* 136 -    */
    0b1111101111,       /* 137 -    */
    0b1111110101,       /* 138 -    */
    0b1111110111,       /* 139 -    */
    0b1111111011,       /* 140 -    */
    0b1111111101,       /* 141 -    */
    0b1111111111,       /* 142 -    */
    0b10101010101,      /* 143 -    */
    0b10101010111,      /* 144 -    */
    0b10101011011,      /* 145 -    */
    0b10101011101,      /* 146 -    */
    0b10101011111,      /* 147 -    */
    0b10101101011,      /* 148 -    */
    0b10101101101,      /* 149 -    */
    0b10101101111,      /* 150 -    */
    0b10101110101,      /* 151 -    */
    0b10101110111,      /* 152 -    */
    0b10101111011,      /* 153 -    */
    0b10101111101,      /* 154 -    */
    0b10101111111,      /* 155 -    */
    0b10110101011,      /* 156 -    */
    0b10110101101,      /* 157 -    */
    0b10110101111,      /* 158 -    */
    0b10110110101,      /* 159 -    */
    0b10110110111,      /* 160 -    */
    0b10110111011,      /* 161 -    */
    0b10110111101,      /* 162 -    */
    0b10110111111,      /* 163 -    */
    0b10111010101,      /* 164 -    */
    0b10111010111,      /* 165 -    */
    0b10111011011,      /* 166 -    */
    0b10111011101,      /* 167 -    */
    0b10111011111,      /* 168 -    */
    0b10111101011,      /* 169 -    */
    0b10111101101,      /* 170 -    */
    0b10111101111,      /* 171 -    */
    0b10111110101,      /* 172 -    */
    0b10111110111,      /* 173 -    */
    0b10111111011,      /* 174 -    */
    0b10111111101,      /* 175 -    */
    0b10111111111,      /* 176 -    */
    0b11010101011,      /* 177 -    */
    0b11010101101,      /* 178 -    */
    0b11010101111,      /* 179 -    */
    0b11010110101,      /* 180 -    */
    0b11010110111,      /* 181 -    */
    0b11010111011,      /* 182 -    */
    0b11010111101,      /* 183 -    */
    0b11010111111,      /* 184 -    */
    0b11011010101,      /* 185 -    */
    0b11011010111,      /* 186 -    */
    0b11011011011,      /* 187 -    */
    0b11011011101,      /* 188 -    */
    0b11011011111,      /* 189 -    */
    0b11011101011,      /* 190 -    */
    0b11011101101,      /* 191 -    */
    0b11011101111,      /* 192 -    */
    0b11011110101,      /* 193 -    */
    0b11011110111,      /* 194 -    */
    0b11011111011,      /* 195 -    */
    0b11011111101,      /* 196 -    */
    0b11011111111,      /* 197 -    */
    0b11101010101,      /* 198 -    */
    0b11101010111,      /* 199 -    */
    0b11101011011,      /* 200 -    */
    0b11101011101,      /* 201 -    */
    0b11101011111,      /* 202 -    */
    0b11101101011,      /* 203 -    */
    0b11101101101,      /* 204 -    */
    0b11101101111,      /* 205 -    */
    0b11101110101,      /* 206 -    */
    0b11101110111,      /* 207 -    */
    0b11101111011,      /* 208 -    */
    0b11101111101,      /* 209 -    */
    0b11101111111,      /* 210 -    */
    0b11110101011,      /* 211 -    */
    0b11110101101,      /* 212 -    */
    0b11110101111,      /* 213 -    */
    0b11110110101,      /* 214 -    */
    0b11110110111,      /* 215 -    */
    0b11110111011,      /* 216 -    */
    0b11110111101,      /* 217 -    */
    0b11110111111,      /* 218 -    */
    0b11111010101,      /* 219 -    */
    0b11111010111,      /* 220 -    */
    0b11111011011,      /* 221 -    */
    0b11111011101,      /* 222 -    */
    0b11111011111,      /* 223 -    */
    0b11111101011,      /* 224 -    */
    0b11111101101,      /* 225 -    */
    0b11111101111,      /* 226 -    */
    0b11111110101,      /* 227 -    */
    0b11111110111,      /* 228 -    */
    0b11111111011,      /* 229 -    */
    0b11111111101,      /* 230 -    */
    0b11111111111,      /* 231 -    */
    0b101010101011,     /* 232 -    */
    0b101010101101,     /* 233 -    */
    0b101010101111,     /* 234 -    */
    0b101010110101,     /* 235 -    */
    0b101010110111,     /* 236 -    */
    0b101010111011,     /* 237 -    */
    0b101010111101,     /* 238 -    */
    0b101010111111,     /* 239 -    */
    0b101011010101,     /* 240 -    */
    0b101011010111,     /* 241 -    */
    0b101011011011,     /* 242 -    */
    0b101011011101,     /* 243 -    */
    0b101011011111,     /* 244 -    */
    0b101011101011,     /* 245 -    */
    0b101011101101,     /* 246 -    */
    0b101011101111,     /* 247 -    */
    0b101011110101,     /* 248 -    */
    0b101011110111,     /* 249 -    */
    0b101011111011,     /* 250 -    */
    0b101011111101,     /* 251 -    */
    0b101011111111,     /* 252 -    */
    0b101101010101,     /* 253 -    */
    0b101101010111,     /* 254 -    */
    0b101101011011,     /* 255 -    */
};

#define PSK_VARICODE_NUM (sizeof(psk_varicode)/sizeof(*psk_varicode))


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

#if 0
static void PskBufAdd(int len, float32_t buf[], float32_t v)
{
	for (int i = len - 1; i > 0; i--)
	{
		buf[i] = buf[i-1];
	}
	buf[0] = v;
}
#endif

static float32_t Psk_IirNext(float32_t b[], float32_t a[], float32_t x[], float32_t y[], int idx, int taps)
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

static void Bpsk_ResetWin() {
    // little trick, we just reset the acc
    // which brings us back to the first sample
	psk_bit_dds.acc = 0;
}

void Bpsk_ModulatorInit(void)
{
	psk_state.tx_ones = 0;
	psk_state.tx_win = true;
	psk_state.tx_char = '\0';
	psk_state.tx_bits = 0;
	psk_state.tx_wave_sign_next = 1;
	psk_state.tx_wave_sign_current = 1;
	psk_state.tx_bit_phase = 0;
	psk_state.tx_ending = false;

	// we use a sine wave with a frequency of half of the bit rate
	// as envelope generator
    softdds_setFreqDDS(&psk_bit_dds, (float32_t)psk_speeds[psk_ctrl_config.speed_idx].value / 2.0, ts.samp_rate, false);
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
	psk_state.tx_bit_len = lround(ts.samp_rate / psk_speeds[psk_ctrl_config.speed_idx].value * 2); // 480000 / 31.25 * 2 = 3072
	psk_state.tx_zeros = - psk_speeds[psk_ctrl_config.speed_idx].zeros; // 50
	psk_state.rate = psk_speeds[psk_ctrl_config.speed_idx].rate; // 384
	Bpsk_ModulatorInit();
	Bpsk_DemodulatorInit();
}


static char Bpsk_DecodeVaricode(uint16_t code)
{
	char result = '*';
	for (int i = 0; i<PSK_VARICODE_NUM; i++) {
		if (psk_varicode[i] == code)
		{
			result = i;
			break;
		}
	}
	return result;
}

static uint16_t Bpsk_FindCharReversed(uint8_t c)
{
    uint16_t retval = 0;

    uint16_t code = psk_varicode[c];

    // bit reverse the code bit pattern, we need MSB of code to be LSB for shifting
    while(code > 0)
    {
        retval |= code & 0x1; // mask and transfer LSB
        retval <<= 1; // left shift
        code >>= 1; // right shift, next bit gets active
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

static bool bit_start(uint16_t tx_bit_phase)
{
    return tx_bit_phase == psk_state.tx_bit_len / 4;
}

static bool bit_middle(uint16_t tx_bit_phase)
{
    return tx_bit_phase == 0;
}

int16_t Psk_Modulator_GenSample()
{
    // tx_bit_len / 4 -> start of a bit
	// tx_bit_len / 0 -> middle of a bit
	// tx_bit_len / 2 -> end of a bit

    // try to find out what to transmit next
	if (bit_start(psk_state.tx_bit_phase))
	{
	    // check if we still have bits to transmit
		if (psk_state.tx_bits == 0)
		{
		    // no, all bits have been transmitted
			if (psk_state.tx_zeros < 2)
			{
			    // send trailing zeros (or are they leading?)
			    // normal characters don't have 2 zeros following each other
				psk_state.tx_zeros++;
			}
			else if ( DigiModes_TxBufferRemove( &psk_state.tx_char, BPSK ))
			{
			    // if all zeros have been sent, look for new
			    // input from input buffer
				psk_state.tx_bits = Bpsk_FindCharReversed( psk_state.tx_char );
				// reset counter for trailing/leading zeros
				psk_state.tx_zeros = 0;
				// reset counter for trailing zeros
				psk_state.tx_ones = 0;
			}
			else if (ts.buffered_tx)
			{
			    // this is for generating  trailing ones if the
			    // input comes from a buffer
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

		// we test the current bit. If it is a zero, and we have no more ones to transmit
		// we alternate the phase of our signal phase (180 degree shift)
		if ((psk_state.tx_bits & 0x1) == 0 && psk_state.tx_ones == 0)
		{
			psk_state.tx_wave_sign_next *= -1;
		}

		// if it is a phase shift, which equals a zero to transmit or we transmit our last bit
		if (psk_state.tx_wave_sign_next != psk_state.tx_wave_sign_current || psk_state.tx_ending)
		{
		    // we have to shape the signal
			psk_state.tx_win = true;
		}
		else
		{
		    // it is a one and not the end, so we simply keep the full swing,
		    // i.e. a constant amplitude signal
			psk_state.tx_win = false;
		}

		psk_state.tx_bits >>= 1; // remove "used" bit
	}

	//  here we are in the middle of bit
	//  we move the next sign in, since it may indicate a phase shift
	// in this case we are transmitting a zero
	if (bit_middle(psk_state.tx_bit_phase))
	{

		psk_state.tx_wave_sign_current = psk_state.tx_wave_sign_next;


		// if we are in the middle of a bit AND it is a zero bit
		// we have to start our envelope from null
		if (psk_state.tx_win)
		{
		    Bpsk_ResetWin(); // we start the envelope from 0 to max
		}
		if (psk_state.tx_ending)
		{
		    RadioManagement_Request_TxOff();
		}
	}

	// if we are shaping the signal envelope
	// we use the "slow" to generate our shape.
	// we use abs so that we are getting only the gain
	// not the phase from here
	// otherwise we use the SAMPLE_MAX as coeff for constant amplitude
    int32_t coeff = psk_state.tx_win ? abs(softdds_nextSample(&psk_bit_dds)) : SAMPLE_MAX;

	// the bit length counter is incremented after each sample
	// and wraps around after one  bit length
	psk_state.tx_bit_phase = (psk_state.tx_bit_phase + 1) % (psk_state.tx_bit_len / 2); // % 1576 == 1 bit length


	int32_t retval = (coeff * psk_state.tx_wave_sign_current * softdds_nextSample(&psk_dds)) / SAMPLE_MAX;

	return retval;
}
