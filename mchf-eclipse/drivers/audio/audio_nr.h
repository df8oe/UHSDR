/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __AUDIO_NR_H
#define __AUDIO_NR_H

#include "uhsdr_board.h"

#ifdef USE_ALTERNATE_NR
#include "freedv_uhsdr.h"

void alternateNR_handle();

void do_alternate_NR();
void alt_noise_blanking();
void spectral_noise_reduction();


int NR_in_buffer_peek(NR_Buffer** c_ptr);
int NR_in_buffer_remove(NR_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int NR_in_buffer_add(NR_Buffer* c);
void NR_in_buffer_reset();
int8_t NR_in_has_data();
int32_t NR_in_has_room();


int NR_out_buffer_peek(NR_Buffer** c_ptr);
int NR_out_buffer_remove(NR_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int NR_out_buffer_add(NR_Buffer* c);
void NR_out_buffer_reset();
int8_t NR_out_has_data();
int32_t NR_out_has_room();
#endif

#endif
