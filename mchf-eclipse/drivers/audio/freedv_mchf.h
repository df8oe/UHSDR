#ifndef __freedv_mchf__
#define __freedv_mchf__

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
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/
#include "mchf_board.h"

#ifdef DEBUG_FREEDV
    #define  FREEDV_TEST_BUFFER_FRAME_COUNT 50
    #define  FREEDV_TEST_BUFFER_FRAME_SIZE 320
    extern const  COMP test_buffer[FREEDV_TEST_BUFFER_FRAME_SIZE*FREEDV_TEST_BUFFER_FRAME_COUNT];
#endif

#define freedv_rx_buffer_max 45

#define FREEDV_TX_MESSAGE	" CQ CQ CQ mcHF SDR with integrated FreeDV codec calling!"
#define FREEDV_TX_DF8OE_MESSAGE	" DF8OE JO42jr using mcHF SDR with integrated FreeDV codec"

void FreeDV_mcHF_HandleFreeDV();
void FreeDV_mcHF_init();

void fdv_print_txt_msg();
void fdv_print_ber();
void fdv_clear_display();
void fdv_print_SNR();

#endif
