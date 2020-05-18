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
#include <assert.h>
#include "ui_driver.h" // for pushing into UI buffer
#include "uhsdr_digi_buffer.h"

#define DIGIMODES_TX_BUFFER_SIZE 128

static __IO uint8_t digimodes_tx_buffer[DIGIMODES_TX_BUFFER_SIZE];
static __IO int32_t digimodes_tx_buffer_head = 0;
static __IO int32_t digimodes_tx_tail = 0;
static __IO uint32_t active_consumer = 0;

// keeps prev consumer to restore it by calling DigiModes_Restore_BufferConsumer();
static __IO uint32_t prev_consumer = CW;

void DigiModes_Restore_BufferConsumer()
{
    active_consumer = prev_consumer;
}

/*
 * The UI consumer has higher priority to protect it by changing modes.
 * UI should release buffer by calling DigiModes_Restore_BufferConsumer()
 */
digi_buff_consumer_t DigiModes_Set_BufferConsumer( digi_buff_consumer_t consumer )
{
    assert((consumer & RTTY) || (consumer & BPSK) || (consumer & CW) || (consumer & UI));
    if ( active_consumer == UI ) // <- UI has higher priority
    {
        prev_consumer = consumer; // in case we change mode
        return prev_consumer;
    }
    prev_consumer = active_consumer;
    active_consumer = consumer;
    return prev_consumer;
}

/**
 * Amount of available characters in tx transmit buffer
 * @return number of available characters in buffer or 0 if empty
 */
uint8_t DigiModes_TxBufferHasData()
{
    int32_t len = digimodes_tx_buffer_head - digimodes_tx_tail;
    return len < 0 ? ( len + DIGIMODES_TX_BUFFER_SIZE ) : len;
}

/**
 * Amount of available characters in tx transmit buffer for a specific customer (RTTY, UI, etc.)
 * @param consumer who wants to read (must match active consumer)
 * @return number of available characters in buffer or 0 if empty or not for active consumer
 */
uint8_t DigiModes_TxBufferHasDataFor(digi_buff_consumer_t consumer)
{
    assert((consumer & RTTY) || (consumer & BPSK) || (consumer & CW) || (consumer & UI));

    return (consumer == active_consumer) ? DigiModes_TxBufferHasData() : 0;
}


bool DigiModes_TxBufferRemove( uint8_t* c_ptr, digi_buff_consumer_t consumer )
{
    assert( c_ptr );
    assert((consumer & RTTY) || (consumer & BPSK) || (consumer & CW) || (consumer & UI));

    bool retval = false;

    if ( consumer == active_consumer && digimodes_tx_buffer_head != digimodes_tx_tail)
    {
        *c_ptr = digimodes_tx_buffer[digimodes_tx_tail];
        digimodes_tx_tail = (digimodes_tx_tail + 1) % DIGIMODES_TX_BUFFER_SIZE;

        // push removed char to UI buffer to display it on the screen.
        // characters for UI and CW prints when they add to digi_buffer
        if ( active_consumer == RTTY || active_consumer == BPSK )
        {
            UiDriver_TextMsgPutChar( *c_ptr );
        }
        retval = true;
    }
    return retval;
}

/* no room left in the buffer returns 0 */
int32_t DigiModes_TxBufferPutChar( uint8_t c, digi_buff_consumer_t source )
{
    assert((source & CW) || (source & KeyBoard) || (source & UI));

    int32_t ret = 0;
    /*
     * In case when source and consumer are equal we just
     * print on the screen without putting into digi_buffer
     * to prevent from infinite loop
     * ( CW working as a source and receiver at the same time )
     */
    if ( active_consumer == source )
    {
        UiDriver_TextMsgPutChar( c );
    }
    else
    {
        int32_t next_head = (digimodes_tx_buffer_head + 1) % DIGIMODES_TX_BUFFER_SIZE;
        if (next_head != digimodes_tx_tail)
        {
            /* there is room */
            digimodes_tx_buffer[digimodes_tx_buffer_head] = c;
            digimodes_tx_buffer_head = next_head;
            /*
             * If the consumer is UI we print on the screen
             * right the way on the inputing text into buffer
             */
            if ( active_consumer == UI )
            {
                UiDriver_TextMsgPutChar( c );
            }
            ret ++;
        }
    }
    return ret;
}

void DigiModes_TxBufferPutSign( const char* s, digi_buff_consumer_t source )
{
    assert( s );
    assert((source & CW) || (source & KeyBoard));

    DigiModes_TxBufferPutChar( '<', source );
    DigiModes_TxBufferPutChar( s[0], source );
    DigiModes_TxBufferPutChar( s[1], source );
    DigiModes_TxBufferPutChar( '>', source );
}

void DigiModes_TxBufferReset()
{
    digimodes_tx_tail = digimodes_tx_buffer_head;
}

#if defined(_UNIT_TEST_)
uint32_t DigiModes_TxBufferGetCurrentConsumer( void )
{
    return active_consumer;
}
uint32_t DigiModes_TxBufferGetPrevConsumer( void )
{
    return prev_consumer;
}
#endif
