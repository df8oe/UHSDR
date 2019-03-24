/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               UHSDR FIRMWARE                                    **
**                                                                                 **
**---------------------------------------------------------------------------------**
**  Licence:		GNU GPLv3, see LICENSE.md                                                      **
************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FREQ_SHIFT_H
#define __FREQ_SHIF_H

#include "uhsdr_types.h"

typedef enum
{
    FREQ_SHIFT_UP = 0,
    FREQ_SHIFT_DOWN = 1,
} freq_shift_dir_t;

void FreqShift(float32_t* i_buffer, float32_t* q_buffer, size_t blockSize, int32_t shift);

#endif
