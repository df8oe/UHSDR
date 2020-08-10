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

#include "uhsdr_types.h"
#include "uhsdr_board.h"
#include "uhsdr_board_config.h"
#include "comp.h"

#if defined(USE_FREEDV_700D)
    #define FDV_MAX_IQ_FRAME_LEN_MS 160
    #define FREEDV_MODE_UHSDR FREEDV_MODE_700D
#elif defined(USE_FREEDV_1600)
    #define FDV_MAX_IQ_FRAME_LEN_MS 40
    #define FREEDV_MODE_UHSDR FREEDV_MODE_1600
#endif
// one need to set this to the highest frame length to be intended to use
// FreeDV 700D requires 1280 samples for the 160ms frame, the audio size may be larger depending
// on the algorithm
// TODO: Explicitly list the variables in FreeDV to look at.


#include "rb.h"

typedef struct
{
    int16_t real;
    int16_t imag;
} COMP_int16_t;

RingBuffer_Declare(fdv_iq_rb, COMP_int16_t)
RingBuffer_Declare(fdv_audio_rb, int16_t)
RingBuffer_Declare(fdv_demod_rb, int16_t)

#define FDV_BUFFER_SIZE     (FDV_MAX_IQ_FRAME_LEN_MS*8)  // (160ms*8samples per ms)

#define NR_BUFFER_NUM  4
#define NR_BUFFER_SIZE     256 // 4*256*8 -> 8192

typedef struct {
   COMP samples[NR_BUFFER_SIZE];
}  NR_Buffer;

typedef union
{
    fdv_demod_rb_item_t fdv_demod_buff[(FDV_BUFFER_SIZE * 3) + IQ_BLOCK_SIZE];
    fdv_iq_rb_item_t fdv_iq_buff[(FDV_BUFFER_SIZE * 2) + IQ_BLOCK_SIZE];
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

typedef struct {
    char* name;
    char* label;
    uint8_t freedv_id;
} freedv_mode_desc_t;

extern    freedv_mode_desc_t freedv_modes[];
extern    const uint8_t freedv_modes_num;



typedef struct {
#define FDV_SQUELCH_OFF   0
#define FDV_SQUELCH_OFFSET (-100)
#define FDV_SQUELCH_MIN   (-99-FDV_SQUELCH_OFFSET) // equals -99 SNR
#define FDV_SQUELCH_MAX   (99-FDV_SQUELCH_OFFSET)  // equals 99 SNR
#define FDV_SQUELCH_DEFAULT   (-2-FDV_SQUELCH_OFFSET)  // equals -2 SNR

    uint8_t squelch_snr_thresh;
    uint8_t mode;
    bool    mute_if_squelched;
} freedv_conf_t;

extern freedv_conf_t freedv_conf;


void FreeDv_HandleFreeDv(void);
void FreeDV_Init(void);
int32_t FreeDV_SetMode(uint8_t fdv_mode, int32_t firstTime);
void FreeDV_Test(void);

int32_t FreeDV_Iq_Get_FrameLen(void);
int32_t FreeDV_Audio_Get_FrameLen(void);


void FreeDv_DisplayClear(void);
void FreeDv_DisplayPrepare(void);
void FreeDv_DisplayUpdate(void);

int32_t FreeDV_Get_Squelch_SNR(freedv_conf_t* freedv_conf_p);
void FreeDV_Set_Squelch_SNR(freedv_conf_t* freedv_conf_p, int8_t squelch_snr);
int FreeDV_Is_Squelch_Enable(freedv_conf_t* freedv_conf_p);
void FreeDV_Squelch_Update(freedv_conf_t* freedv_conf_p);


#endif
#if defined(USE_FREEDV) || defined(USE_ALTERNATE_NR)


extern MultiModeBuffer_t mmb;

// we allow for one more pointer to a buffer as we have buffers
// why? because our implementation will only fill up the fifo only to N-1 elements
#define FDV_BUFFER_IQ_FIFO_SIZE (FDV_BUFFER_IQ_NUM+1)

#define NR_BUFFER_FIFO_SIZE (NR_BUFFER_NUM+1)


#endif
#endif
