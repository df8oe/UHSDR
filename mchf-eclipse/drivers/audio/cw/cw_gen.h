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
#define __UI_GEN_H

// Break timeout on straight key
#define CW_BREAK			800

// States
#define	CW_IDLE				0
#define	CW_DIT_CHECK		1
#define	CW_DAH_CHECK		3
#define	CW_KEY_DOWN			4
#define	CW_KEY_UP			5
#define	CW_PAUSE			6
#define	CW_WAIT				7

#define CW_DIT_L      		0x01
#define CW_DAH_L      		0x02
#define CW_DIT_PROC   		0x04

#define CW_IAMBIC_A    		0x00
#define CW_IAMBIC_B    		0x10

#define CW_SMOOTH_LEN		16
//
//
typedef struct PaddleState
{
    // State machine and port states
    ulong 	port_state;
    ulong	cw_state;

    // Smallest element duration
    ulong   dit_time;

    // Timers
    ulong   key_timer;
    ulong 	break_timer;

    // Key clicks smoothing table current ptr
    ulong	sm_tbl_ptr;

} PaddleState;

// Exports
void 	cw_gen_init();
void    cw_set_speed();

ulong	cw_gen_process(float32_t *i_buffer,float32_t *q_buffer,ulong size);
ulong 	cw_gen_process_strk(float32_t *i_buffer,float32_t *q_buffer,ulong size);
ulong 	cw_gen_process_iamb(float32_t *i_buffer,float32_t *q_buffer,ulong size);

void 	cw_gen_dah_IRQ();
void 	cw_gen_dit_IRQ();

#endif
