/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                               UHSDR FIRMWARE                                    **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Licence:        GNU GPLv3, see LICENSE.md                                                      **
 ************************************************************************************/

// Common
#include <assert.h>
#include "uhsdr_board.h"
#include "profiling.h"

#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "audio_driver.h"
#include "audio_management.h"
#include "ui_configuration.h"
#include "ui_driver.h"
#include "rtty.h"
#include "radio_management.h"


// bits 0-4 -> baudot, bit 5 1 == LETTER, 0 == NUMBER/FIGURE
const uint8_t Ascii2Baudot[128] =
{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0b001011, //	BEL	N
		0,
		0,
		0b000010, //	\n	NL
		0,
		0,
		0b001000, //	\r	NL
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0b100100, // 	 	N
		0, //	!
		0, //	"
		0, //	#
		0,	// $
		0, //	%
		0, //	&
		0b000101, //	'	N
		0b001111, //	(	N
		0b010010, //	)	N
		0, //	*
		0b010001, //	+	N
		0b001100, //	,	N
		0b000011, //	-	N
		0b011100, //	.	N
		0b011101, //	/	N
		0b010110, //	0	N
		0b010111, //	1	N
		0b010011, //	2	N
		0b000001, //	3	N
		0b001010, //	4	N
		0b010000, //	5	N
		0b010101, //	6	N
		0b000111, //	7	N
		0b000110, //	8	N
		0b011000, //	9	N
		0b001110, //	:	N
		0, //	;
		0, //	<
		0b011110, //	=
		0, //	>
		0b011001, //	?	N
		0, //	@
		0b100011, //	A	L
		0b111001, //	B	L
		0b101110, //	C	L
		0b101001, //	D	L
		0b100001, //	E	L
		0b101101, //	F	L
		0b111010, //	G	L
		0b110100, //	H	L
		0b100110, //	I	L
		0b101011, //	J	L
		0b101111, //	K	L
		0b110010, //	L	L
		0b111100, //	M	L
		0b101100, //	N	L
		0b111000, //	O	L
		0b110110, //	P	L
		0b110111, //	Q	L
		0b101010, //	R	L
		0b100101, //	S	L
		0b110000, //	T	L
		0b100111, //	U	L
		0b111110, //	V	L
		0b110011, //	W	L
		0b111101, //	X	L
		0b110101, //	Y	L
		0b110001, //	Z	L
		0,
		0,
		0,
		0,
		0,
		0,
		0b100011, //	A	L
		0b111001, //	B	L
		0b101110, //	C	L
		0b101001, //	D	L
		0b100001, //	E	L
		0b101101, //	F	L
		0b111010, //	G	L
		0b110100, //	H	L
		0b100110, //	I	L
		0b101011, //	J	L
		0b101111, //	K	L
		0b110010, //	L	L
		0b111100, //	M	L
		0b101100, //	N	L
		0b111000, //	O	L
		0b110110, //	P	L
		0b110111, //	Q	L
		0b101010, //	R	L
		0b100101, //	S	L
		0b110000, //	T	L
		0b100111, //	U	L
		0b111110, //	V	L
		0b110011, //	W	L
		0b111101, //	X	L
		0b110101, //	Y	L
		0b110001, //	Z	L
		0,
		0,
		0,
		0,
		0,
};

#define RTTY_SYMBOL_CODE (0b011011)
#define RTTY_LETTER_CODE (0b111111)

// RTTY Experiment based on code from the DSP Tutorial at http://dp.nonoo.hu/projects/ham-dsp-tutorial/18-rtty-decoder-using-iir-filters/
// Used with permission from Norbert Varga, HA2NON under GPLv3 license

/*
 * Experimental Code
 */
#ifdef USE_RTTY_PROCESSOR

const rtty_speed_item_t rtty_speeds[RTTY_SPEED_NUM] =
{
		{ .id =RTTY_SPEED_45, .value = 45.45, .label = "45" },
		{ .id =RTTY_SPEED_50, .value = 50, .label = "50"  },
};

const rtty_shift_item_t rtty_shifts[RTTY_SHIFT_NUM] =
{
		{ RTTY_SHIFT_170, 170, "170" },
		{ RTTY_SHIFT_450, 450, "450" },
};

rtty_ctrl_t rtty_ctrl_config =
{
		.shift_idx = RTTY_SHIFT_170,
		.speed_idx = RTTY_SPEED_45,
		.stopbits_idx = RTTY_STOP_1_5
};



typedef struct
{
	float32_t gain;
	float32_t coeffs[4];
	uint16_t freq; // center freq
} rtty_bpf_config_t;

typedef struct
{
	float32_t gain;
	float32_t coeffs[2];
} rtty_lpf_config_t;

typedef struct
{
	float32_t xv[5];
	float32_t yv[5];
} rtty_bpf_data_t;

typedef struct
{
	float32_t xv[3];
	float32_t yv[3];
} rtty_lpf_data_t;



static float32_t RttyDecoder_bandPassFreq(float32_t sampleIn, const rtty_bpf_config_t* coeffs, rtty_bpf_data_t* data) {
	data->xv[0] = data->xv[1]; data->xv[1] = data->xv[2]; data->xv[2] = data->xv[3]; data->xv[3] = data->xv[4];
	data->xv[4] = sampleIn / coeffs->gain; // gain at centre
	data->yv[0] = data->yv[1]; data->yv[1] = data->yv[2]; data->yv[2] = data->yv[3]; data->yv[3] = data->yv[4];
	data->yv[4] = (data->xv[0] + data->xv[4]) - 2 * data->xv[2]
															 + (coeffs->coeffs[0] * data->yv[0]) + (coeffs->coeffs[1] * data->yv[1])
															 + (coeffs->coeffs[2] * data->yv[2]) + (coeffs->coeffs[3] * data->yv[3]);
	return data->yv[4];
}

static float32_t RttyDecoder_lowPass(float32_t sampleIn, const rtty_lpf_config_t* coeffs, rtty_lpf_data_t* data) {
	data->xv[0] = data->xv[1]; data->xv[1] = data->xv[2];
	data->xv[2] = sampleIn / coeffs->gain; // gain at DC
	data->yv[0] = data->yv[1]; data->yv[1] = data->yv[2];
	data->yv[2] = (data->xv[0] + data->xv[2]) + 2 * data->xv[1]
															 + (coeffs->coeffs[0] * data->yv[0]) + (coeffs->coeffs[1] * data->yv[1]);
	return data->yv[2];
}

typedef enum {
	RTTY_RUN_STATE_WAIT_START = 0,
	RTTY_RUN_STATE_BIT,
} rtty_run_state_t;


typedef enum {
	RTTY_MODE_LETTERS = 0,
	RTTY_MODE_SYMBOLS
} rtty_charSetMode_t;




typedef struct {
	rtty_bpf_data_t bpfSpaceData;
	rtty_bpf_data_t bpfMarkData;
	rtty_lpf_data_t lpfData;
	rtty_bpf_config_t *bpfSpaceConfig;
	rtty_bpf_config_t *bpfMarkConfig;
	rtty_lpf_config_t *lpfConfig;

	uint16_t oneBitSampleCount;
	int32_t DPLLOldVal;
	int32_t DPLLBitPhase;

	uint8_t byteResult;
	uint16_t byteResultp;

	rtty_charSetMode_t charSetMode;

	rtty_run_state_t state;

	soft_dds_t tx_dds[2];

	const rtty_mode_config_t* config_p;

} rtty_decoder_data_t;

rtty_decoder_data_t rttyDecoderData;




#if 0 // 48Khz filters, not needed
// this is for 48ksps sample rate
// for filter designing, see http://www-users.cs.york.ac.uk/~fisher/mkfilter/
// order 2 Butterworth, freqs: 865-965 Hz
rtty_bpf_config_t rtty_bp_48khz_915 =
{
		.gain = 2.356080041e+04,  // gain at centre
		.coeffs = {-0.9816582826, 3.9166274264, -5.8882201843, 3.9530488323 },
		.freq = 915
};

// order 2 Butterworth, freqs: 1035-1135 Hz
rtty_bpf_config_t rtty_bp_48khz_1085 =
{
		.gain = 2.356080365e+04,
		.coeffs = {-0.9816582826, 3.9051693660, -5.8653953990, 3.9414842213 },
		.freq = 1085
};
#endif

// order 2 Butterworth, freq: 50 Hz
rtty_lpf_config_t rtty_lp_48khz_50 =
{
		.gain = 9.381008646e+04,
		.coeffs = {-0.9907866988, 1.9907440595 }
};

// this is for 12ksps sample rate
// for filter designing, see http://www-users.cs.york.ac.uk/~fisher/mkfilter/
// order 2 Butterworth, freqs: 865-965 Hz, centre: 915 Hz
static rtty_bpf_config_t rtty_bp_12khz_915 =
{
		.gain = 1.513364755e+03,
		.coeffs = { -0.9286270861, 3.3584472566, -4.9635817596, 3.4851652468 },
		.freq = 915
};

// order 2 Butterworth, freqs: 1315-1415 Hz, centre 1365Hz
static rtty_bpf_config_t rtty_bp_12khz_1365 =
{
		.gain = 1.513365019e+03,
		.coeffs = { -0.9286270861, 2.8583904591, -4.1263569881, 2.9662407442 },
		.freq = 1365
};
// order 2 Butterworth, freqs: 1035-1135 Hz, centre: 1085Hz
static rtty_bpf_config_t rtty_bp_12khz_1085 =
{
		.gain = 1.513364927e+03,
		.coeffs = { -0.9286270861, 3.1900687350, -4.6666321298, 3.3104336142 },
		.freq = 1085
};

static rtty_lpf_config_t rtty_lp_12khz_50 =
{
		.gain = 5.944465310e+03,
		.coeffs = { -0.9636529842, 1.9629800894 }
};

static rtty_mode_config_t  rtty_mode_current_config;


void RttyDecoder_Init()
{

	// TODO: pass config as parameter and make it changeable via menu
	rtty_mode_current_config.samplerate = 12000;
	rtty_mode_current_config.shift = rtty_shifts[rtty_ctrl_config.shift_idx].value;
	rtty_mode_current_config.speed = rtty_speeds[rtty_ctrl_config.speed_idx].value;
	rtty_mode_current_config.stopbits = rtty_ctrl_config.stopbits_idx;

	rttyDecoderData.config_p = &rtty_mode_current_config;

	// common config to all supported modes
	rttyDecoderData.oneBitSampleCount = (uint16_t)roundf(rttyDecoderData.config_p->samplerate/rttyDecoderData.config_p->speed);
	rttyDecoderData.charSetMode = RTTY_MODE_LETTERS;
	rttyDecoderData.state = RTTY_RUN_STATE_WAIT_START;

	rttyDecoderData.bpfMarkConfig = &rtty_bp_12khz_915; // this is mark, or '1'
	rttyDecoderData.lpfConfig = &rtty_lp_12khz_50;

	// now we handled the specifics
	switch (rttyDecoderData.config_p->shift)
	{
	case 450:
		rttyDecoderData.bpfSpaceConfig = &rtty_bp_12khz_1365; // this is space or '0'
		break;
	case 170:
	default:
		// all unsupported shifts are mapped to 170
		rttyDecoderData.bpfSpaceConfig = &rtty_bp_12khz_1085; // this is space or '0'
	}

	// configure DDS for transmission
	softdds_setFreqDDS(&rttyDecoderData.tx_dds[0], rttyDecoderData.bpfSpaceConfig->freq, ts.samp_rate, 0);
	softdds_setFreqDDS(&rttyDecoderData.tx_dds[1], rttyDecoderData.bpfMarkConfig->freq, ts.samp_rate, 0);

}

static float32_t decayavg(float32_t average, float32_t input, int weight)
{ // adapted from https://github.com/ukhas/dl-fldigi/blob/master/src/include/misc.h
	float32_t retval;
	if (weight <= 1)
	{
		retval = input;
	}
	else
	{
		retval = ( ( input - average ) / (float32_t)weight ) + average ;
	}
	return retval;
}


// this function returns the bit value of the current sample
static int RttyDecoder_demodulator(float32_t sample)
{

	float32_t space_mag = RttyDecoder_bandPassFreq(sample, rttyDecoderData.bpfSpaceConfig, &rttyDecoderData.bpfSpaceData);
	float32_t mark_mag = RttyDecoder_bandPassFreq(sample, rttyDecoderData.bpfMarkConfig, &rttyDecoderData.bpfMarkData);

	float32_t v1 = 0.0;
	// calculating the RMS of the two lines (squaring them)
	space_mag *= space_mag;
	mark_mag *= mark_mag;

    if(ts.rtty_atc_enable)
	{   // RTTY decoding with ATC = automatic threshold correction
		// FIXME: space & mark seem to be swapped in the following code
		// dirty fix
		float32_t helper = space_mag;
		space_mag = mark_mag;
		mark_mag = helper;
		static float32_t mark_env = 0.0;
		static float32_t space_env = 0.0;
		static float32_t mark_noise = 0.0;
		static float32_t space_noise = 0.0;
		// experiment to implement an ATC (Automatic threshold correction), DD4WH, 2017_08_24
		// everything taken from FlDigi, licensed by GNU GPLv2 or later
		// https://github.com/ukhas/dl-fldigi/blob/master/src/cw_rtty/rtty.cxx
		// calculate envelope of the mark and space signals
		// uses fast attack and slow decay
		mark_env = decayavg (mark_env, mark_mag,
				(mark_mag > mark_env) ? rttyDecoderData.oneBitSampleCount / 4 : rttyDecoderData.oneBitSampleCount * 16);
		space_env = decayavg (space_env, space_mag,
				(space_mag > space_env) ? rttyDecoderData.oneBitSampleCount / 4 : rttyDecoderData.oneBitSampleCount * 16);
		// calculate the noise on the mark and space signals
		mark_noise = decayavg (mark_noise, mark_mag,
				(mark_mag < mark_noise) ? rttyDecoderData.oneBitSampleCount / 4 : rttyDecoderData.oneBitSampleCount * 48);
		space_noise = decayavg (space_noise, space_mag,
				(space_mag < space_noise) ? rttyDecoderData.oneBitSampleCount / 4 : rttyDecoderData.oneBitSampleCount * 48);
		// the noise floor is the lower signal of space and mark noise
		float32_t noise_floor = (space_noise < mark_noise) ? space_noise : mark_noise;

		// Linear ATC, section 3 of www.w7ay.net/site/Technical/ATC
		//		v1 = space_mag - mark_mag - 0.5 * (space_env - mark_env);

		// Compensating for the noise floor by using clipping
		float32_t mclipped = 0.0, sclipped = 0.0;
		mclipped = mark_mag > mark_env ? mark_env : mark_mag;
		sclipped = space_mag > space_env ? space_env : space_mag;
		if (mclipped < noise_floor)
		{
			mclipped = noise_floor;
		}
		if (sclipped < noise_floor)
		{
			sclipped = noise_floor;
		}

		// we could add options for mark-only or space-only decoding
		// however, the current implementation with ATC already works quite well with mark-only/space-only
		/*					switch (progdefaults.rtty_cwi) {
						case 1 : // mark only decode
							space_env = sclipped = noise_floor;
							break;
						case 2: // space only decode
							mark_env = mclipped = noise_floor;
						default : ;
		}
		 */

		// Optimal ATC (Section 6 of of www.w7ay.net/site/Technical/ATC)
		v1  = (mclipped - noise_floor) * (mark_env - noise_floor) -
				(sclipped - noise_floor) * (space_env - noise_floor) -
				0.25 *  ((mark_env - noise_floor) * (mark_env - noise_floor) -
						(space_env - noise_floor) * (space_env - noise_floor));

		v1 = RttyDecoder_lowPass(v1, rttyDecoderData.lpfConfig, &rttyDecoderData.lpfData);

		// MchfBoard_GreenLed((line0 > 0)? LED_STATE_OFF:LED_STATE_ON);
	}
	else
	{   // RTTY without ATC, which works very well too!
		// inverting line 1
		mark_mag *= -1;

		// summing the two lines
		v1 = mark_mag + space_mag;

		// lowpass filtering the summed line
		v1 = RttyDecoder_lowPass(v1, rttyDecoderData.lpfConfig, &rttyDecoderData.lpfData);
		// MchfBoard_GreenLed((line0 > 0)? LED_STATE_OFF:LED_STATE_ON);
	}

	return (v1 > 0)?0:1;
}

// this function returns true once at the half of a bit with the bit's value
static bool RttyDecoder_getBitDPLL(float32_t sample, bool* val_p) {
	static bool phaseChanged = false;
	bool retval = false;


	if (rttyDecoderData.DPLLBitPhase < rttyDecoderData.oneBitSampleCount)
	{
		*val_p = RttyDecoder_demodulator(sample);

		if (!phaseChanged && *val_p != rttyDecoderData.DPLLOldVal) {
			if (rttyDecoderData.DPLLBitPhase < rttyDecoderData.oneBitSampleCount/2)
			{
				// rttyDecoderData.DPLLBitPhase += rttyDecoderData.oneBitSampleCount/8; // early
				rttyDecoderData.DPLLBitPhase += rttyDecoderData.oneBitSampleCount/32; // early
			}
			else
			{
				//rttyDecoderData.DPLLBitPhase -= rttyDecoderData.oneBitSampleCount/8; // late
				rttyDecoderData.DPLLBitPhase -= rttyDecoderData.oneBitSampleCount/32; // late
			}
			phaseChanged = true;
		}
		rttyDecoderData.DPLLOldVal = *val_p;
		rttyDecoderData.DPLLBitPhase++;
	}

	if (rttyDecoderData.DPLLBitPhase >= rttyDecoderData.oneBitSampleCount)
	{
		rttyDecoderData.DPLLBitPhase -= rttyDecoderData.oneBitSampleCount;
		retval = true;
	}

	return retval;
}

// this function returns only true when the start bit is successfully received
static bool RttyDecoder_waitForStartBit(float32_t sample) {
	bool retval = false;
	int bitResult;
	static int16_t wait_for_start_state = 0;
	static int16_t wait_for_half = 0;

	bitResult = RttyDecoder_demodulator(sample);
	switch (wait_for_start_state)
	{
	case 0:
		// waiting for a falling edge
		if (bitResult != 0)
		{
			wait_for_start_state++;
		}
		break;
	case 1:
		if (bitResult != 1)
		{
			wait_for_start_state++;
		}
		break;
	case 2:
		wait_for_half = rttyDecoderData.oneBitSampleCount/2;
		wait_for_start_state ++;
		/* no break */
	case 3:
		wait_for_half--;
		if (wait_for_half == 0)
		{
			retval = (bitResult == 0);
			wait_for_start_state = 0;
		}
		break;
	}
	return retval;
}



static const char RTTYLetters[] = "<E\nA SIU\nDRJNFCKTZLWHYPQOBG^MXV^";
static const char RTTYSymbols[] = "<3\n- ,87\n$4#,.:(5+)2.60197.^./=^";


void RttyDecoder_ProcessSample(float32_t sample)
{

	switch(rttyDecoderData.state)
	{
	case RTTY_RUN_STATE_WAIT_START: // not synchronized, need to wait for start bit
		if (RttyDecoder_waitForStartBit(sample))
		{
			rttyDecoderData.state = RTTY_RUN_STATE_BIT;
			rttyDecoderData.byteResultp = 1;
			rttyDecoderData.byteResult = 0;
		}
		break;
	case RTTY_RUN_STATE_BIT:
		// reading 7 more bits
		if (rttyDecoderData.byteResultp < 8)
		{
			bool bitResult;
			if (RttyDecoder_getBitDPLL(sample, &bitResult))
			{
				switch (rttyDecoderData.byteResultp)
				{
				case 6: // stop bit 1

				case 7: // stop bit 2
				if (bitResult == false)
				{
					// not in sync
					rttyDecoderData.state = RTTY_RUN_STATE_WAIT_START;
				}
				if (rttyDecoderData.config_p->stopbits != RTTY_STOP_2 && rttyDecoderData.byteResultp == 6)
				{
					// we pretend to be at the 7th bit after receiving the first stop bit if we have less than 2 stop bits
					// this omits check for 1.5 bit condition but we should be more or less safe here, may cause
					// a little more unaligned receive but without that shortcut we simply cannot receive these configurations
					// so it is worth it
					rttyDecoderData.byteResultp = 7;
				}

				break;
				default:
					// System.out.print(bitResult);
					rttyDecoderData.byteResult |= (bitResult?1:0) << (rttyDecoderData.byteResultp-1);
				}
				rttyDecoderData.byteResultp++;
			}
		}
		if (rttyDecoderData.byteResultp == 8 && rttyDecoderData.state == RTTY_RUN_STATE_BIT)
		{
			char charResult;

			switch (rttyDecoderData.byteResult) {
			case RTTY_LETTER_CODE:
				rttyDecoderData.charSetMode = RTTY_MODE_LETTERS;
				// System.out.println(" ^L^");
				break;
			case RTTY_SYMBOL_CODE:
				rttyDecoderData.charSetMode = RTTY_MODE_SYMBOLS;
				// System.out.println(" ^F^");
				break;
			default:
				switch (rttyDecoderData.charSetMode)
				{
				case RTTY_MODE_LETTERS:
					charResult = RTTYLetters[rttyDecoderData.byteResult];
					break;
				case RTTY_MODE_SYMBOLS:
					charResult = RTTYSymbols[rttyDecoderData.byteResult];
					break;
				}
				UiDriver_TextMsgPutChar(charResult);
			}
			rttyDecoderData.state = RTTY_RUN_STATE_WAIT_START;
		}
	}
}

typedef enum
{
	MSK_IDLE = 0,
	MSK_WAIT_FOR_NEG,
	MSK_WAIT_FOR_PLUS,
	MSK_WAIT_FOR_PLUSGRAD, // wait for the values growing towards max
	MSK_WAIT_FOR_MAX, // wait for the value after max, needs to be done one the growing part of the curve
} msk_state_t;

typedef struct
{
	uint8_t char_bit_idx;
	uint16_t char_bits;
	uint32_t char_bit_samples;
	uint16_t last_bit;
	int16_t last_value;
	uint8_t current_bit;
	msk_state_t msk_state;
	rtty_charSetMode_t char_mode;

} rtty_tx_encoder_state_t;

rtty_tx_encoder_state_t  rtty_tx =
{
		.last_bit = 1,
		.last_value = INT16_MIN,
		.char_mode = RTTY_MODE_LETTERS,

};

#define DIGIMODES_TX_BUFFER_SIZE  128

static __IO uint8_t digimodes_tx_buffer[DIGIMODES_TX_BUFFER_SIZE];
static __IO int32_t digimodes_tx_buffer_head = 0;
static __IO int32_t digimodes_tx_tail = 0;

uint8_t DigiModes_TxBufferHasData()
{
    int32_t len = digimodes_tx_buffer_head - digimodes_tx_tail;
    return len < 0?len+DIGIMODES_TX_BUFFER_SIZE:len;
}

int DigiModes_TxBufferRemove(uint8_t* c_ptr)
{
	int ret = 0;

    if (digimodes_tx_buffer_head != digimodes_tx_tail)
    {
        int c = digimodes_tx_buffer[digimodes_tx_tail];
        digimodes_tx_tail = (digimodes_tx_tail + 1) % DIGIMODES_TX_BUFFER_SIZE;
        *c_ptr = (uint8_t)c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int DigiModes_TxBufferPutChar(uint8_t c)
{
	int ret = 0;
    int32_t next_head = (digimodes_tx_buffer_head + 1) % DIGIMODES_TX_BUFFER_SIZE;

    if (next_head != digimodes_tx_tail)
    {
        /* there is room */
        digimodes_tx_buffer[digimodes_tx_buffer_head] = c;
        digimodes_tx_buffer_head = next_head;
        ret ++;
    }
    return ret;
}

void DigiModes_TxBufferReset()
{
    digimodes_tx_tail = digimodes_tx_buffer_head;
}


#define USE_RTTY_MSK
#define RTTY_CODE_MODE 0b100000

static void Rtty_BaudotAdd(uint8_t bits)
{
	uint8_t bitCount = rttyDecoderData.config_p->stopbits == RTTY_STOP_1?7:8;

	bits <<= 1; // add stop bit

	if (bitCount == 7)
	{
		bits |= 0b01000000;
	}
	else
	{
		// for 1.5 and 2 we use 2 bits for now
		bits |= 0b11000000;
	}

	rtty_tx.char_bits |= bits << rtty_tx.char_bit_idx;
	// we add more bits towards the MSB if we already have bits

	rtty_tx.char_bit_idx += bitCount;
	// now remember how many bits we added
}

void Rtty_Modulator_Code2Bits(uint8_t baudot_info)
{
	rtty_tx.char_bits = 0;
	rtty_tx.char_bit_idx = 0;

	if (baudot_info & RTTY_CODE_MODE)
	{
		if(rtty_tx.char_mode != RTTY_MODE_LETTERS)
		{
			rtty_tx.char_mode = RTTY_MODE_LETTERS;
			Rtty_BaudotAdd(RTTY_LETTER_CODE);
		}
	}
	else
	{
		if(rtty_tx.char_mode != RTTY_MODE_SYMBOLS)
		{
			rtty_tx.char_mode = RTTY_MODE_SYMBOLS;
			Rtty_BaudotAdd(RTTY_SYMBOL_CODE);
		}
	}
	Rtty_BaudotAdd(baudot_info & ~RTTY_CODE_MODE);
}

// MUST BE CALLED BEFORE WE START RTTY TX, i.e. in RX Mode !!!
void Rtty_Modulator_StartTX()
{
	rtty_tx.last_bit = 1;
	rtty_tx.last_value = INT16_MIN;
	rtty_tx.char_mode = RTTY_MODE_LETTERS;
	Rtty_Modulator_Code2Bits(RTTY_LETTER_CODE);
}

const char rtty_test_string[] = " --- UHSDR FIRMWARE RTTY TX TEST DE DB4PLE --- ";

int16_t Rtty_Modulator_GenSample()
{
	if (rtty_tx.char_bit_samples == 0)
	{
		rtty_tx.char_bit_samples = rttyDecoderData.oneBitSampleCount * 4;

		rtty_tx.char_bits >>= 1;
		if (rtty_tx.char_bit_idx == 0)
		{
			// load the character and add the stop bits;

			bool bitsFilled = false;
			while (DigiModes_TxBufferHasData() && bitsFilled == false)
			{
				uint8_t current_ascii;
				DigiModes_TxBufferRemove(&current_ascii);
				uint8_t current_baudot = Ascii2Baudot[current_ascii & 0x7f];
				if (current_baudot > 0)
				{ // we have valid baudot code
					Rtty_Modulator_Code2Bits(current_baudot);
					bitsFilled = true;
				}
			}

			if (bitsFilled == false)
			{
				// IDLE
				Rtty_Modulator_Code2Bits(RTTY_LETTER_CODE);
#if 0
				for (uint8_t idx = 0; idx < sizeof(rtty_test_string); idx++)
				{
					DigiModes_TxBufferPutChar(rtty_test_string[idx]);
				}
#endif
			}
		}
		rtty_tx.char_bit_idx--;

		rtty_tx.current_bit = rtty_tx.char_bits&1;
#ifdef USE_RTTY_MSK
		if (rtty_tx.last_bit != rtty_tx.current_bit)
		{
			if (rtty_tx.last_bit == 1)
			{
				rtty_tx.msk_state = MSK_WAIT_FOR_NEG;
				// WAIT_FOR_NEG
				// WAIT_FOR_ZERO or plus
				rtty_tx.current_bit = rtty_tx.last_bit;
			}
			else
			{
				rtty_tx.msk_state = MSK_WAIT_FOR_PLUSGRAD;
				rtty_tx.current_bit = rtty_tx.last_bit;
				rtty_tx.last_value = INT16_MIN;
				// WAIT_FOR_MAX
			}
		}
#else
		rtty_tx.msk_state = MSK_IDLE;
#endif
	}

	rtty_tx.char_bit_samples--;

	int16_t current_value = softdds_nextSample(&rttyDecoderData.tx_dds[rtty_tx.current_bit]);

	switch(rtty_tx.msk_state)
	{
	case MSK_WAIT_FOR_NEG:
		if (current_value < 0)
		{
			rtty_tx.msk_state = MSK_WAIT_FOR_PLUS;
		}
		break;
	case MSK_WAIT_FOR_PLUS:
		if (current_value >= 0)
		{
			rtty_tx.msk_state = MSK_IDLE;
			rtty_tx.current_bit = rtty_tx.char_bits&1;
			rttyDecoderData.tx_dds[rtty_tx.current_bit].acc = rttyDecoderData.tx_dds[rtty_tx.last_bit].acc;
			rtty_tx.last_bit = rtty_tx.current_bit;
			current_value = softdds_nextSample(&rttyDecoderData.tx_dds[rtty_tx.current_bit]);
		}
		break;
	case MSK_WAIT_FOR_PLUSGRAD:
		if (current_value > rtty_tx.last_value)
		{
			rtty_tx.msk_state = MSK_WAIT_FOR_MAX;
		}
		break;
	case MSK_WAIT_FOR_MAX:
		if (current_value >= rtty_tx.last_value)
		{
			rtty_tx.last_value = current_value;
		}
		else if (current_value < rtty_tx.last_value && rtty_tx.last_value > 0)
		{
			rtty_tx.msk_state = MSK_IDLE;
			rtty_tx.current_bit = rtty_tx.char_bits&1;
			rttyDecoderData.tx_dds[rtty_tx.current_bit].acc = rttyDecoderData.tx_dds[rtty_tx.last_bit].acc;
			rtty_tx.last_bit = rtty_tx.current_bit;
			current_value = softdds_nextSample(&rttyDecoderData.tx_dds[rtty_tx.current_bit]);
		}
		break;
	default:
		break;
	}

	rtty_tx.last_value = current_value;

	return current_value;
}

#endif
