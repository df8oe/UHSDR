/*
 * cw_decoder.h
 *
 *  Created on: 07.09.2017
 *      Author: danilo
 */

#ifndef AUDIO_CW_CW_DECODER_H_
#define AUDIO_CW_CW_DECODER_H_

#define CW_DECODER_BLOCKSIZE_MIN		8
#define CW_DECODER_BLOCKSIZE_MAX		128
#define CW_DECODER_BLOCKSIZE_DEFAULT	88

#define CW_DECODER_THRESH_MIN			1000
#define CW_DECODER_THRESH_MAX			50000
#define CW_DECODER_THRESH_DEFAULT		32000

typedef struct
{
	float32_t sampling_freq;
	float32_t target_freq;
//	float32_t speed;
	uint8_t speed;
//	uint8_t average;
	uint32_t thresh;
	uint8_t blocksize;

//	uint8_t AGC_enable;
	uint8_t noisecancel_enable;
	uint8_t spikecancel;
#define CW_SPIKECANCEL_MODE_OFF 0
#define CW_SPIKECANCEL_MODE_SPIKE 1
#define CW_SPIKECANCEL_MODE_SHORT 2
	bool use_3_goertzels;
	bool snap_enable;
    bool show_CW_LED; // menu choice whether the user wants the CW LED indicator to be working or not
} cw_config_t;

extern cw_config_t cw_decoder_config;


void CwDecode_RxProcessor(float32_t * const src, int16_t blockSize);
void CwDecode_Filter_Set(void);
//void CW_Decoder_WPM_display_erase(void);
void CwDecoder_WpmDisplayUpdate(bool force_update);
void CwDecoder_WpmDisplayClearOrPrepare(bool prepare);

#endif /* AUDIO_CW_CW_DECODER_H_ */
