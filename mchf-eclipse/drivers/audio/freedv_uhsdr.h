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


#define FDV_BUFFER_SIZE     320
#define FDV_RX_AUDIO_SIZE_MAX     360

// this is kind of variable unfortunately, see freedv_api.h/.c for FREEDV1600 it is 360
#define FDV_BUFFER_AUDIO_NUM   3
#define FDV_BUFFER_IQ_NUM  3 // 3*320*8 = 7680

#define NR_BUFFER_NUM  4
#define NR_BUFFER_SIZE     256 // 4*256*8 -> 8192

typedef struct {
    int16_t samples[FDV_BUFFER_SIZE]; // this is kind of variable unfortunately, see freedv_api.h/.c for FREEDV1600 it is 360
}  FDV_Audio_Buffer;

typedef struct {
   COMP samples[FDV_BUFFER_SIZE];
}  FDV_IQ_Buffer;

typedef struct {
   COMP samples[NR_BUFFER_SIZE];
}  NR_Buffer;

typedef union
{
    FDV_IQ_Buffer fdv_iq_buff[FDV_BUFFER_IQ_NUM];
    NR_Buffer nr_audio_buff[NR_BUFFER_NUM];
} MultiModeBuffer_t;


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


extern MultiModeBuffer_t mmb;

// we allow for one more pointer to a buffer as we have buffers
// why? because our implementation will only fill up the fifo only to N-1 elements
#define FDV_BUFFER_IQ_FIFO_SIZE (FDV_BUFFER_IQ_NUM+1)
#define NR_BUFFER_FIFO_SIZE (NR_BUFFER_NUM+1)


extern FDV_Audio_Buffer fdv_audio_buff[FDV_BUFFER_AUDIO_NUM];


// we allow for one more pointer to a buffer as we have buffers
// why? because our implementation will only fill up the fifo only to N-1 elements
#endif
#endif
