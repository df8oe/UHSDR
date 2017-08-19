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

    const rtty_mode_config_t* config_p;

} rtty_decoder_data_t;

static rtty_decoder_data_t rttyDecoderData;




#if 0 // 48Khz filters, not needed
// this is for 48ksps sample rate
// for filter designing, see http://www-users.cs.york.ac.uk/~fisher/mkfilter/
// order 2 Butterworth, freqs: 865-965 Hz
rtty_bpf_config_t rtty_bp_48khz_915 =
{
    .gain = 2.356080041e+04,  // gain at centre
    .coeffs = {-0.9816582826, 3.9166274264, -5.8882201843, 3.9530488323 }
};

// order 2 Butterworth, freqs: 1035-1135 Hz
rtty_bpf_config_t rtty_bp_48khz_1085 =
{
.gain = 2.356080365e+04,
.coeffs = {-0.9816582826, 3.9051693660, -5.8653953990, 3.9414842213 }
};

// order 2 Butterworth, freq: 50 Hz
rtty_lpf_config_t rtty_lp_48khz_50 =
{
    .gain = 9.381008646e+04,
    .coeffs = {-0.9907866988, 1.9907440595 }
};
#endif

// this is for 12ksps sample rate
// for filter designing, see http://www-users.cs.york.ac.uk/~fisher/mkfilter/
// order 2 Butterworth, freqs: 865-965 Hz, centre: 915 Hz
static rtty_bpf_config_t rtty_bp_12khz_915 =
{
        .gain = 1.513364755e+03,
        .coeffs = { -0.9286270861, 3.3584472566, -4.9635817596, 3.4851652468 }
};

// order 2 Butterworth, freqs: 1315-1415 Hz, centre 1365Hz
static rtty_bpf_config_t rtty_bp_12khz_1365 =
{
        .gain = 1.513365019e+03,
        .coeffs = { -0.9286270861, 2.8583904591, -4.1263569881, 2.9662407442 }
};
// order 2 Butterworth, freqs: 1035-1135 Hz, centre: 1085Hz
static rtty_bpf_config_t rtty_bp_12khz_1085 =
{
        .gain = 1.513364927e+03,
        .coeffs = { -0.9286270861, 3.1900687350, -4.6666321298, 3.3104336142 }
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

#if 0
    switch(ts.enable_rtty_decode)
    {
                    case 1:
                        rttyDecoderData.config_p = &ham170;
                    break;
                    case 2:
                        rttyDecoderData.config_p = &dwd450;
                    break;
                    default:
                        rttyDecoderData.config_p = &ham170;
                    break;
    }
#endif

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
}

// this function returns the bit value of the current sample
static int RttyDecoder_demodulator(float32_t sample) {
    float32_t line0 = RttyDecoder_bandPassFreq(sample, rttyDecoderData.bpfSpaceConfig, &rttyDecoderData.bpfSpaceData);
    float32_t line1 = RttyDecoder_bandPassFreq(sample, rttyDecoderData.bpfMarkConfig, &rttyDecoderData.bpfMarkData);
    // calculating the RMS of the two lines (squaring them)
    line0 *= line0;
    line1 *= line1;

    // inverting line 1
    line1 *= -1;

    // summing the two lines
    line0 += line1;

    // lowpass filtering the summed line
    line0 = RttyDecoder_lowPass(line0, rttyDecoderData.lpfConfig, &rttyDecoderData.lpfData);

    // MchfBoard_GreenLed((line0 > 0)? LED_STATE_OFF:LED_STATE_ON);
    return (line0 > 0)?0:1;
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


void RttyDecoder_ProcessSample(float32_t sample) {

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
            case 31:
                rttyDecoderData.charSetMode = RTTY_MODE_LETTERS;
                // System.out.println(" ^L^");
                break;
            case 27:
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
#endif
