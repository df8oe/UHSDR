/*
 * cw_decoder.h
 *
 *  Created on: 07.09.2017
 *      Author: danilo
 */

#ifndef AUDIO_CW_CW_DECODER_H_
#define AUDIO_CW_CW_DECODER_H_

typedef struct
{
	float32_t sampling_freq;
	float32_t target_freq;
	float32_t speed;
} cw_config_t;

extern cw_config_t cw_decoder_config;


void CwDecode_RxProcessor(float32_t * const src, int16_t blockSize);
void CwDecode_FilterInit();

#endif /* AUDIO_CW_CW_DECODER_H_ */
