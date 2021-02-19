/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
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
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#ifndef __MCHF_TYPES_H
#define __MCHF_TYPES_H

#include <stdint.h>
#include "arm_math.h"
// defines float32_t

#ifndef uchar
typedef	unsigned char	uchar;
#endif

#ifndef ushort
typedef	unsigned short	ushort;
#endif

#ifndef uint
typedef	unsigned int	uint;
#endif

#ifndef ulong
typedef	unsigned long	ulong;
#endif

#ifndef bool
typedef	int				bool;
#endif

#ifndef true
#define true			1
#endif

#ifndef false
#define false			0
#endif

typedef struct {
    float32_t i;
    float32_t q;
} iq_float_t;

typedef enum {
    IQ_TRANS_OFF = 0,
    IQ_TRANS_ON,
    IQ_TRANS_NUM
} iq_trans_idx_t;

// PA power level setting enumeration
// this order MUST match the order of entries in power_levels in radio_management.c !
typedef enum
{
    PA_LEVEL_FULL = 0,
    PA_LEVEL_HIGH,
    PA_LEVEL_MEDIUM,
    PA_LEVEL_LOW,
    PA_LEVEL_MINIMAL,
    PA_LEVEL_TUNE_KEEP_CURRENT
} power_level_t;

#endif
