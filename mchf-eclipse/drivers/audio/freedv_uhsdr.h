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
#include "uhsdr_board.h"

#ifdef USE_FREEDV
#ifdef DEBUG_FREEDV
    #define  FREEDV_TEST_BUFFER_FRAME_COUNT 50
    #define  FREEDV_TEST_BUFFER_FRAME_SIZE 320
    extern const  COMP test_buffer[FREEDV_TEST_BUFFER_FRAME_SIZE*FREEDV_TEST_BUFFER_FRAME_COUNT];
#endif

#define FREEDV_TX_MESSAGE	" CQ CQ CQ UHSDR " TRX_NAME " SDR with integrated FreeDV codec calling!"
#define FREEDV_TX_DF8OE_MESSAGE	" DF8OE JO42jr using UHSDR " TRX_NAME " SDR with integrated FreeDV codec"

void FreeDv_HandleFreeDv();
void FreeDV_mcHF_init();

void FreeDv_DisplayClear();
void FreeDv_DisplayPrepare();
void FreeDv_DisplayUpdate();

int fdv_iq_buffer_peek(FDV_IQ_Buffer** c_ptr);
int fdv_iq_buffer_remove(FDV_IQ_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int fdv_iq_buffer_add(FDV_IQ_Buffer* c);
void fdv_iq_buffer_reset();
int32_t fdv_iq_has_data();
int32_t fdv_iq_has_room();


int fdv_audio_buffer_peek(FDV_Audio_Buffer** c_ptr);
int fdv_audio_buffer_remove(FDV_Audio_Buffer** c_ptr);
/* no room left in the buffer returns 0 */
int fdv_audio_buffer_add(FDV_Audio_Buffer* c);
void fdv_audio_buffer_reset();
int32_t fdv_audio_has_data();
int32_t fdv_audio_has_room();

#endif
#if defined(USE_FREEDV) || defined(USE_ALTERNATE_NR)

#define FDV_BUFFER_IQ_FIFO_SIZE (FDV_BUFFER_IQ_NUM+1)
extern FDV_IQ_Buffer __MCHF_SPECIALMEM fdv_iq_buff[FDV_BUFFER_IQ_NUM];

// we allow for one more pointer to a buffer as we have buffers
// why? because our implementation will only fill up the fifo only to N-1 elements
#endif
#ifdef USE_ALTERNATE_NR

void alternateNR_handle();

void do_alternate_NR();
void alt_noise_blanking();
void spectral_noise_reduction();

#endif
#endif
