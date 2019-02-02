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
#ifndef __UHSDR_MATH_H
#define __UHSDR_MATH_H

#include "uhsdr_types.h"
float32_t Math_log10f_fast(float32_t X);
float32_t Math_absmax(float32_t* buffer, int size);
float32_t Math_sign_new (float32_t x);

#endif // __UHSDR_MATH_H
