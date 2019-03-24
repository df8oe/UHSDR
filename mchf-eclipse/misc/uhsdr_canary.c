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
#include "uhsdr_canary.h"

static const uint8_t canary_word[] = { 'D', 'O', ' ', 'G', 'N', 'U', ' ', 'G', 'P', 'L', 'v', '3' };
uint8_t* canary_word_ptr;

/**
 * this has to be called after all dynamic memory allocation has happened
 */
void Canary_Create( void )
{
    canary_word_ptr = (uint8_t*)malloc( sizeof( canary_word ));
    assert( canary_word_ptr );
    memcpy ( canary_word_ptr, canary_word, sizeof(canary_word) );
}

/**
 * Check if the canary was not destroyed (i.e. no additional allocation has happened and/or some code went wild.
 *
 * this has to be called after all dynamic memory allocation has happened AND Canary_Create has been called
 *
 * @return true if the previously written canary is still found in its location
 */
bool Canary_IsIntact( void )
{
    return memcmp ( canary_word_ptr, canary_word, sizeof(canary_word) ) == 0;
}

uint8_t* Canary_GetAddr( void )
{
    return canary_word_ptr;
}
