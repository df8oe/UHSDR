/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/
#ifndef __UHSDR_CANARY_H
#define __UHSDR_CANARY_H

#include "uhsdr_types.h"

void     Canary_Create ( void );
bool     Canary_IsIntact ( void );
uint8_t* Canary_GetAddr ( void );

#endif // __UHSDR_CANARY_H
