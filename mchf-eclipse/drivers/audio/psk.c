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
#include <math.h>
#include <stdlib.h>
#include "softdds.h"
#include "uhsdr_digi_buffer.h"
#include "ui_driver.h" // only necessary because of UiDriver_TextMsgPutChar
#include "radio_management.h" // only necessary because of RadioManagement_Request_TxOff


// RX,TX common constants
#define PSK_SAMPLE_RATE 12000 // TODO This should come from elsewhere, to be fixed

// RX constants
#define PSK_BND_FLT_LEN 5
#define PSK_BUF_LEN (PSK_SAMPLE_RATE / PSK_OFFSET)
// this must be an integer result without remainder and must be a multiple of 4

#define PSK_SHIFT_DIFF (1.0 * PSK_OFFSET / PSK_SAMPLE_RATE) // phase change between two samples of PSK_OFFSET Hz

// TX constants
#define SAMPLE_MAX 32766 // max amplitude of generated samples


typedef struct
{
    uint16_t rate;

    uint16_t tx_idx;
    uint8_t tx_char;
    uint16_t tx_bits;
    int16_t tx_wave_sign_next;
    int16_t tx_wave_sign_current;
    uint16_t tx_bit_phase;
    uint32_t tx_bit_len;
    int16_t tx_zeros;
    int16_t tx_ones;
    bool tx_win;

    float32_t rx_phase;
    float32_t rx_samples_in[PSK_BND_FLT_LEN];
    float32_t rx_samples[PSK_BND_FLT_LEN];
    int16_t rx_bnd_idx;
    float32_t rx_cos_prod[PSK_BUF_LEN];
    float32_t rx_sin_prod[PSK_BUF_LEN];
    float32_t rx_scmix[PSK_BUF_LEN];
    int32_t rx_idx;
    int8_t rx_last_bit;
    float32_t rx_err; // could be of interest for tuning
    float32_t rx_last_symbol;
    int16_t rx_symbol_len; // how many buffers fit into one bit
    int16_t rx_symbol_idx;
    // float32_t rx_symbol_buf[PSK_MAX_SYMBOL_BUF];
    uint32_t rx_word;
    psk_modulator_t tx_mod_state;
    float32_t rx_sum_sin;
    float32_t rx_sum_cos;
} PskState_Internal_t;


// table courtesy of fldigi pskvaricode.cxx
static const uint16_t psk_varicode[] = {
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


static const float32_t PskBndPassB_31[] = {
		6.6165543533213894e-05,
		0.0,
		-0.00013233108706642779,
		0.0,
		6.6165543533213894e-05
};

static const float32_t PskBndPassA_31[] = {
		1.0,
		-3.8414813063247664,
		5.6662277107033248,
		-3.7972899991488904,
		0.9771256616899302
};

static const float32_t PskBndPassB_63[] = {
		0.0002616526950658905,
		0.0,
		-0.000523305390131781,
		0.0,
		0.0002616526950658905
};

static const float32_t PskBndPassA_63[] = {
		1.0,
		-3.8195192250239174,
		5.6013869366249818,
		-3.7321386869273105,
		0.95477455992103932
};

static const float32_t PskBndPassB_125[] = {
		0.0010232176384709002,
		0.0,
		-0.0020464352769418003,
		0.0,
		0.0010232176384709002
};

static const float32_t PskBndPassA_125[] = {
		1.0,
		-3.7763786572915334,
		5.4745855184361272,
		-3.6055008493327723,
		0.91159449659996006
};

static soft_dds_t psk_dds;
static soft_dds_t psk_bit_dds;
static soft_dds_t psk_rx_dds;


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

static float32_t Psk_IirNext(const float32_t bpf_b[], const float32_t bpf_a[], float32_t x[], float32_t y[], int idx, int taps)
{
	float32_t resp = 0;
	int iidx; 

	for (int i = 0; i < taps; i++)
	{
		iidx = (idx - i + taps) % taps;
		resp += bpf_b[i] * x[iidx];
		if (i>0)
		{
			resp -= bpf_a[i] * y[iidx];
		}
	}
	return resp / bpf_a[0];
}




const psk_speed_item_t psk_speeds[PSK_SPEED_NUM] =
{
		{ .id =PSK_SPEED_31, .value = 31.25,  .bpf_b = PskBndPassB_31, .bpf_a = PskBndPassA_31, .rate = 384, .label = " 31" },
		{ .id =PSK_SPEED_63, .value = 62.5,   .bpf_b = PskBndPassB_63, .bpf_a = PskBndPassA_63, .rate = 192, .label = " 63"  },
		{ .id =PSK_SPEED_125, .value = 125.0, .bpf_b = PskBndPassB_125, .bpf_a = PskBndPassA_125, .rate = 96, .label = "125" }
};

psk_ctrl_t psk_ctrl_config =
{
		.speed_idx = PSK_SPEED_31
};

PskState_Internal_t  psk_state;

static void Bpsk_ResetWin() {
    // little trick, we just reset the acc
    // which brings us back to the first sample
	psk_bit_dds.acc = 0;
}

void Psk_Modulator_PrepareTx()
{
	Psk_Modulator_SetState(PSK_MOD_PREAMBLE);
}

void Bpsk_Demodulator_Init()
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
		psk_state.rx_cos_prod[i] = 0;
		psk_state.rx_sin_prod[i] = 0;
		psk_state.rx_scmix[i] = 0;
	}

	psk_state.rx_idx = 0;
	psk_state.rx_last_bit = 0;
	psk_state.rx_last_symbol = 0;
	psk_state.rx_symbol_len = psk_state.rate / PSK_BUF_LEN;
	psk_state.rx_symbol_idx = 0;
	
	// for (int i = 0; i < psk_state.rx_symbol_len; i ++)
	// {
	// 	psk_state.rx_symbol_buf[i];
	// }

	psk_state.rx_word = 0;
}


void Psk_Modem_Init(uint32_t output_sample_rate)
{

	psk_state.tx_idx = 0;

	softdds_setFreqDDS(&psk_dds,    PSK_OFFSET, output_sample_rate, true);
	softdds_setFreqDDS(&psk_rx_dds, PSK_OFFSET, PSK_SAMPLE_RATE,    true);
    // we use a sine wave with a frequency of half of the bit rate
    // as envelope generator
    softdds_setFreqDDS(&psk_bit_dds, (float32_t)psk_speeds[psk_ctrl_config.speed_idx].value / 2.0, output_sample_rate, false);

	psk_state.tx_bit_len = lround(output_sample_rate / psk_speeds[psk_ctrl_config.speed_idx].value * 2); // 480000 / 31.25 * 2 = 3072
	psk_state.rate = PSK_SAMPLE_RATE / psk_speeds[psk_ctrl_config.speed_idx].value;

	Bpsk_Demodulator_Init();
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

/**
 *
 * Basically samples the phase every bit length based on a sample which is one full sine wave
 * of the carrier frequency. So it is not using multiple sampling points or anything.
 * We throw away a lot of phase differences symbols
 * using more of these should make the code more robust, shouldn't it?
 *
 * @param symbol_out averaged phase of the samples.
 */

static void BpskDecoder_NextSymbol(float32_t symbol_out)
{
    int8_t bit;

    static float32_t symbol_store = 0;

    // if (psk_state.rx_symbol_len - psk_state.rx_symbol_idx < 6)
    {
        symbol_store = symbol_out;
    }

    psk_state.rx_symbol_idx += 1;
    if (psk_state.rx_symbol_idx >= psk_state.rx_symbol_len)
    {
        psk_state.rx_symbol_idx = 0;
        // TODO here should come additional part to check if timing of sampling should be moved
        if (psk_state.rx_last_symbol * symbol_store < 0)
        {
            bit = 0;
        }
        else
        {
            bit = 1;
        }

        psk_state.rx_last_symbol = symbol_store;
        symbol_store = 0;

        // have we found 2 consecutive 0 bits? And previously at least one received bit == 1?
        // indicates an end of character
        if (psk_state.rx_last_bit == 0 && bit == 0 && psk_state.rx_word != 0)
        {
            // we lookup up the bits received (minus the last zero, which we shift out to the right)
            // and put it into the buffer
            UiDriver_TextMsgPutChar(Bpsk_DecodeVaricode(psk_state.rx_word >> 1));

            // clean out the stored bit pattern
            psk_state.rx_word = 0;
        }
        else
        {
            psk_state.rx_word = (psk_state.rx_word << 1) | bit;
        }

        psk_state.rx_last_bit = bit;

    }
}

static float32_t BpskDecoder_Bandpass(float32_t sample)
{
    // save original sample in bpf's input buffer
    psk_state.rx_samples_in[psk_state.rx_bnd_idx] = sample;

    // IIR bandpass for signal frequency range, implemented in a ring buffer
    float32_t retval = Psk_IirNext(psk_speeds[psk_ctrl_config.speed_idx].bpf_b, psk_speeds[psk_ctrl_config.speed_idx].bpf_a, psk_state.rx_samples_in,
        psk_state.rx_samples, psk_state.rx_bnd_idx, PSK_BND_FLT_LEN);

    // save filtered sample in state buffer, used in next run;
    psk_state.rx_samples[psk_state.rx_bnd_idx] = retval;

    // increment our ring buffer index
    psk_state.rx_bnd_idx++;
    psk_state.rx_bnd_idx %= PSK_BND_FLT_LEN;

    return retval;
}

/**
 * Process an audio sample and decode signal.
 *
 * @param audio sample at PSK_SAMPLE_RATE
 */
void Psk_Demodulator_ProcessSample(float32_t sample)
{
    float32_t fsample = BpskDecoder_Bandpass(sample);

	// VCO generates a sine/cosine wave "carrier" with PSK_OFFSET Hz as frequency
    float32_t vco_sin, vco_cos;
    // we have to use a different DDS as for TX (different sample rates), IQ gives us sin and cos waves of PSK_OFFSET Hz
    softdds_genIQSingleTone(&psk_rx_dds, &vco_sin , &vco_cos, 1);

	// we now multiple these carriers with our signal
	// this allows us to compare phase differences
    float32_t sin_mix = vco_sin * fsample;
    float32_t cos_mix = vco_cos * fsample;

    // update sums by differences between old and new value in ringbuffer
    psk_state.rx_sum_sin += sin_mix - psk_state.rx_sin_prod[psk_state.rx_idx];
    psk_state.rx_sum_cos += cos_mix - psk_state.rx_cos_prod[psk_state.rx_idx];

    // store new value
    psk_state.rx_sin_prod[psk_state.rx_idx] = sin_mix;
    psk_state.rx_cos_prod[psk_state.rx_idx] = cos_mix;

	// we now calculate an average
	float32_t symbol_out = psk_state.rx_sum_sin / PSK_BUF_LEN;
	float32_t cos_out    = psk_state.rx_sum_cos / PSK_BUF_LEN;

    // GUESS: now try to estimate the frequency error of our VCO vs. the incoming signal
	float32_t rx_scmix = symbol_out * cos_out;
	psk_state.rx_err += rx_scmix - psk_state.rx_scmix[psk_state.rx_idx];
	psk_state.rx_scmix[psk_state.rx_idx] = rx_scmix;


	float32_t smax = 0;

	for (int i = 0; i < PSK_BUF_LEN; i++)
	{
		if (fabsf(psk_state.rx_cos_prod[i]) > smax)
		{
			smax = fabsf(psk_state.rx_cos_prod[i]);
		}
		if (fabsf(psk_state.rx_sin_prod[i]) > smax)
		{
			smax = fabsf(psk_state.rx_sin_prod[i]);
		}
	}

	// calculate the final correction value from rx_err
	// avoid division by zero if smax is 0
	float32_t rx_err_corr = psk_state.rx_err/ (PSK_BUF_LEN * ((smax != 0) ? (smax * smax * 4.0) : 1.0));

	// if the error is too large, we limit it to +/- 0.1
	if(fabsf(rx_err_corr) > 0.1)
	{
		rx_err_corr = (rx_err_corr > 0) ? 0.1 : -0.1;
	}
	rx_err_corr = 0;

	// now advance our phase counter with our error correction
    psk_state.rx_phase += PSK_SHIFT_DIFF + rx_err_corr * PSK_SHIFT_DIFF;


	// we just passed one "full" offset frequency wave length?
	// now see if we have enough symbols to decide if it is a 1 or 0
    // the symbol is basically an averaged phase difference over the last PSK_BUF_LEN
    // samples.
	if (psk_state.rx_phase > 1)
	{
		psk_state.rx_phase -= 1;
		BpskDecoder_NextSymbol(symbol_out);
	}

	if (psk_state.rx_phase < 0)
	{
		psk_state.rx_phase += 1;
	}

    // we prepare us for the next sample in our ring buffer
    psk_state.rx_idx = (psk_state.rx_idx + 1) % PSK_BUF_LEN;
}

static bool bit_start(uint16_t tx_bit_phase)
{
    return tx_bit_phase == psk_state.tx_bit_len / 4;
}

static bool bit_middle(uint16_t tx_bit_phase)
{
    return tx_bit_phase == 0;
}

/**
 * Generates a BPSK signal. Uses an oscillator for generating continous base signal of a
 * given frequency (defined in PSK_OFFSET). This signal is phase controlled by a variable
 * psk_state.tx_wave_sign_current. A second frequency generator provides an envelope shape based on a cosine
 * of half of the bpsk rate. For 0 bits to transmit the symbol is using the enveloped signal and shifts
 * in the middle.
 *
 *
 * @return
 */
int16_t Psk_Modulator_GenSample()
{
    // tx_bit_len / 4 -> start of a bit
    // tx_bit_len / 0 -> middle of a bit
    // tx_bit_len / 2 -> end of a bit

    int32_t retval = 0; // by default we produce silence

    // check if the modulator is supposed to be active...
    if (Psk_Modulator_GetState() != PSK_MOD_OFF)
    {
        // try to find out what to transmit next
        if (bit_start(psk_state.tx_bit_phase))
        {
            // check if we still have bits to transmit
            if (psk_state.tx_bits == 0)
            {
                // no, all bits have been transmitted
                if (psk_state.tx_zeros < 2 || (Psk_Modulator_GetState() == PSK_MOD_PREAMBLE))
                {
                    // send spacing zeros before anything else happens
                    // normal characters don't have 2 zeros following each other
                    psk_state.tx_zeros++;

                    // are we sending a preamble and have transmitted enough zeroes?
                    // we do this for roughly a second, i.e. we simply use the rate as "timer"
                    if ((Psk_Modulator_GetState() == PSK_MOD_PREAMBLE) && psk_state.tx_zeros >= psk_speeds[psk_ctrl_config.speed_idx].value)
                    {
                        Psk_Modulator_SetState(PSK_MOD_ACTIVE);
                    }
                }
                else if (DigiModes_TxBufferHasData(BPSK))
                {
                    if (DigiModes_TxBufferRemove( &psk_state.tx_char, BPSK ))
                    {
                        Psk_Modulator_SetState(PSK_MOD_ACTIVE);
                        if (psk_state.tx_char == 0x04) // EOT, stop tranmission
                        {
                            // we send from buffer, and nothing more is in the buffer
                            // request sending the trailing sequence
                            Psk_Modulator_SetState(PSK_MOD_POSTAMBLE);
                        }
                        else
                        {
                            // if all zeros have been sent, look for new
                            // input from input buffer
                            psk_state.tx_bits = Bpsk_FindCharReversed(psk_state.tx_char);
                            // reset counter for spacing zeros
                            psk_state.tx_zeros = 0;
                            // reset counter for trailing postamble (which conclude a transmission)
                            psk_state.tx_ones = 0;
                        }
                    }
                }

                if (Psk_Modulator_GetState() == PSK_MOD_POSTAMBLE)
                {
                    // this is for generating  trailing postamble if the
                    // input comes from a buffer or if we are asked to
                    // switch off,
                    // we do this for roughly a second, i.e. we simply use the rate as "timer"
                    if (psk_state.tx_ones < psk_speeds[psk_ctrl_config.speed_idx].value)
                    {
                        psk_state.tx_ones+=16;
                        psk_state.tx_bits = 0xffff; // we add 16 bits of postamble
                        // so we may send a few more postamble than request, but who cares...
                    }
                    else
                    {
                        Psk_Modulator_SetState(PSK_MOD_INACTIVE);
                    }
                }
            }

            // we test the current bit. If it is a zero, and we have no more postamble to transmit
            // we alternate the phase of our signal phase (180 degree shift)
            if ((psk_state.tx_bits & 0x1) == 0 && psk_state.tx_ones == 0)
            {
                psk_state.tx_wave_sign_next *= -1;
            }

            // if it is a phase shift, which equals a zero to transmit or we transmit our last bit
            if (psk_state.tx_wave_sign_next != psk_state.tx_wave_sign_current || Psk_Modulator_GetState() == PSK_MOD_INACTIVE)
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
            if (Psk_Modulator_GetState() == PSK_MOD_INACTIVE)
            {
                // now turn us off, we're done.
                Psk_Modulator_SetState(PSK_MOD_OFF);
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


        retval = (coeff * psk_state.tx_wave_sign_current * softdds_nextSample(&psk_dds)) / SAMPLE_MAX;
    }

    return retval;
}

/**
 * Returns the operational state of the PSK modulator
 * @return current state
 */
psk_modulator_t Psk_Modulator_GetState()
{
    return psk_state.tx_mod_state;
}

/**
 * Change the state of the psk modulator to a new operational state
 * If necessary, checks if state can be changed and executes code
 * for the state transistion such as resetting variables to a known etc.
 *
 * @param newState
 * @return the previous state
 */
psk_modulator_t Psk_Modulator_SetState(psk_modulator_t newState)
{
    psk_modulator_t retval = psk_state.tx_mod_state;

    switch(newState)
    {
    case PSK_MOD_PREAMBLE:
        psk_state.tx_ones = 0;
        psk_state.tx_win = true;
        psk_state.tx_char = '\0';
        psk_state.tx_bits = 0;
        psk_state.tx_wave_sign_next = 1;
        psk_state.tx_wave_sign_current = 1;
        psk_state.tx_bit_phase = 0;
        psk_state.tx_zeros = 0;
        psk_state.tx_mod_state = newState;
        break;
    case PSK_MOD_OFF:
        RadioManagement_Request_TxOff();
        psk_state.tx_mod_state = newState;
        break;
    default:
        psk_state.tx_mod_state = newState;
        break;
    }

    return retval;
}
