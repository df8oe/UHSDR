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

#undef PI
#undef HALF_PI
#undef TWO_PI
#undef TPI
#define PI 3.1415926535897932384626433832795f
#define HALF_PI 1.5707963267948966192313216916398f
#define TWO_PI 6.283185307179586476925286766559f
#define TPI           TWO_PI
#define PIH           HALF_PI
#define FOURPI        (2.0f * TPI)
#define SIXPI         (3.0f * TPI)

#endif // __UHSDR_MATH_H
