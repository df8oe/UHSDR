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
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include "uhsdr_math.h"

/**
 * Fast algorithm for log10
 *
 * This is a fast approximation to log2()
 * Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
 * log10f is exactly log2(x)/log2(10.0f)
 * Math_log10f_fast(x) =(log2f_approx(x)*0.3010299956639812f)
 *
 * @param X number want log10 for
 * @return log10(x)
 */
float32_t Math_log10f_fast(float32_t X) {
    float Y, F;
    int E;
    F = frexpf(fabsf(X), &E);
    Y = 1.23149591368684f;
    Y *= F;
    Y += -4.11852516267426f;
    Y *= F;
    Y += 6.02197014179219f;
    Y *= F;
    Y += -3.13396450166353f;
    Y += E;
    return(Y * 0.3010299956639812f);
}

/**
 * Find the absolute (i.e. ignoring the sign) maximum value
 * @param float32_t buffer to be searched
 * @param size how many elements
 * @return the actual maxium value of the size elements in buffer
 */
float32_t Math_absmax(float32_t* buffer, int size)
{
    float32_t min, max;
    uint32_t  pindex;

    arm_max_f32(buffer, size, &max, &pindex);      // find absolute value of audio in buffer after gain applied
    arm_min_f32(buffer, size, &min, &pindex);

    return -min>max?-min:max;
}

/**
 * get the sign of a float number
 * @param x the number to test
 * @return -1 if below zero, 0 if zero, 1 if above zero
 */
float32_t Math_sign_new (float32_t x) {
    return (x < 0) ? -1.0 : ( (x > 0) ? 1.0 : 0.0);
}
