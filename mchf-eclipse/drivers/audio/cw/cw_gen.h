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

void    CwGen_Init();

void    CwGen_PrepareTx();
void    CwGen_SetSpeed();

bool    CwGen_Process( float32_t *i_buffer, float32_t *q_buffer, uint32_t size );

// As showed tests calling them (CwGen_DahIRQ & CwGen_DitIRQ) is matter only for Ultimatic mode.
// After change code to work in this mode w/o them, these extra dependencies with low-level layers could be removed.
void    CwGen_DahIRQ();
void    CwGen_DitIRQ();

uint8_t CwGen_CharacterIdFunc( uint32_t );

#endif
