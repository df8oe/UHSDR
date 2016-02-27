#ifndef __UI_H
#define __UI_H


/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#include "mchf_board.h"

typedef struct BandInfo {
		uint8_t default_pf;
		uint8_t selector_5W;
		uint8_t selector_FULL;
		uint32_t tune;
		uint32_t size;
		const char* name;
} BandInfo;

extern BandInfo bandInfo[MAX_BAND_NUM];

#endif

