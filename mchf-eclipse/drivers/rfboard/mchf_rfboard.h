/*
 * mchf_rfboard.h
 *
 *  Created on: 13 Feb 2021
 *      Author: danilo
 */

#ifndef RFBOARD_MCHF_RFBOARD_H_
#define RFBOARD_MCHF_RFBOARD_H_

void MchfRfBoard_BandCntr_Init(void);
void MchfRfBoard_SelectLpfBpf(uint8_t group);

bool Mchf_PrepareTx(void);
bool Mchf_PrepareRx(void);
bool Mchf_EnableRx(void);
bool Mchf_EnableTx(void);
bool Mchf_RfBoard_Init(void);
bool Mchf_RfBoard_SetPaBiasValue(uint32_t bias);

#endif /* RFBOARD_MCHF_RFBOARD_H_ */
