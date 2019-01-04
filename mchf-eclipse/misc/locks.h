/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/*************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Description:    Simple boolean locks for further adopting RTOS                 **
 **  Last Modified:  m-chichikalov (rv9yw) Implemented LOCKs as a step toward RTOS. **
 **  License:         GNU GPLv3                                                     **
 ************************************************************************************/

#ifndef __LOCKS_H
#define __LOCKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uhsdr_types.h"

// FIXME dedicate file (header) with defined UHSDR specific return statuses.
#define UHSDR_ERR_NONE 0

void sem_give( volatile bool* sem );

int32_t sem_take( volatile bool* sem, uint32_t timeout );

#ifdef __cplusplus
}
#endif
#endif // __LOCKS_H
