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

#endif
