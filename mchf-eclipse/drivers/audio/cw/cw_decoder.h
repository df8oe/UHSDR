/*
 * cw_decoder.h
 *
 *  Created on: 07.09.2017
 *      Author: danilo
 */

#ifndef AUDIO_CW_CW_DECODER_H_
#define AUDIO_CW_CW_DECODER_H_

void CwDecode_RxProcessor(float32_t * const src, int16_t blockSize);
void CwDecode_FilterInit();

#endif /* AUDIO_CW_CW_DECODER_H_ */
