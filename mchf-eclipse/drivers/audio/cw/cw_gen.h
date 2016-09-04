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
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

#include "math.h"
#include "arm_math.h"

#ifndef __CW_GEN_H
#define __CW_GEN_H


// Exports
void 	cw_gen_init();
void    cw_set_speed();

ulong	cw_gen_process(float32_t *i_buffer,float32_t *q_buffer,ulong size);

void	w_test_first_paddle();
void 	cw_gen_dah_IRQ();
void 	cw_gen_dit_IRQ();

#endif
