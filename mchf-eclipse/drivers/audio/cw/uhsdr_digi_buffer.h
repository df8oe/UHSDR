/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  Description:   Please provide one                                             **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UHSDR_DIGI_BUFFER_H
#define __UHSDR_DIGI_BUFFER_H

#include "uhsdr_types.h"

typedef enum
{
    RTTY       = 1,
    BPSK       = 2,
    CW         = 4,
    UI         = 8,
    KeyBoard   = 16, // <- keyboard should be used as a source only
} digi_buff_consumer_t;

#ifdef __cplusplus
extern "C" {
#endif

uint8_t DigiModes_TxBufferHasData();
uint8_t DigiModes_TxBufferHasDataFor(digi_buff_consumer_t consumer);

/*
 * The Digi buffer has multi consumers, some of them trying to get an entry
 * from the buffer at the same time.
 * to organize it, the allowed consumer for buffer should be set before using it.
 *
 * @return - previous consumer
 */
digi_buff_consumer_t DigiModes_Set_BufferConsumer( digi_buff_consumer_t consumer );
void                 DigiModes_Restore_BufferConsumer();

/*
 * @return - true if element was removed from buffer and available in c_ptr
 *           false if there is no elements in buffer or this consumer not
 *           allowed to remove elements from buffer.
 */
bool     DigiModes_TxBufferRemove( uint8_t* c_ptr, digi_buff_consumer_t consumer );

int32_t  DigiModes_TxBufferPutChar( uint8_t c, digi_buff_consumer_t source );
void     DigiModes_TxBufferPutSign( const char* s, digi_buff_consumer_t source );

void     DigiModes_TxBufferReset( );

#if defined(_UNIT_TEST_)
    uint32_t DigiModes_TxBufferGetCurrentConsumer( void );
    uint32_t DigiModes_TxBufferGetPrevConsumer( void );
#endif

#ifdef __cplusplus
}
#endif
#endif // __UHSDR_DIGI_BUFFER_H
