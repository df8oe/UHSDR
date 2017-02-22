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
#ifndef __CW_GEN_H
#define __CW_GEN_H

#include "arm_math.h"

// Exports
void 	CwGen_Init();
void    CwGen_PrepareTx();
void    CwGen_SetSpeed();

bool	CwGen_Process(float32_t *i_buffer,float32_t *q_buffer,ulong size);

void 	CwGen_DahIRQ();
void 	CwGen_DitIRQ();

#endif
