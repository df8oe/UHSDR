/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/
#include <assert.h>
#include "uhsdr_board_config.h"
#include "ui_driver.h" /**> for UiDriver_TextMsgPutChar() */
#include "uhsdr_digi_buffer.h"
#include "uhsdr_ring_buffer.h"

/**
 * If this module is using with UHSDR
 * change this defines in config files.
 *
 * Values above is default ones
 * if others are not defined in config files.
 */
#if !defined(DIGIMODES_TX_BUFFER_SIZE)
/** @todo-> Add into config files. */
    #define DIGIMODES_TX_BUFFER_SIZE 128
#endif

/** @todo -  */
typedef enum
{
    TXT_MSG_NONE = 0,
    TXT_MSG_UPDATE_SCREEN = 1,
    TXT_MSG_CLEAN_SCREEN  = 2,
}txt_msg_actions_requested_t;

// keeps prev consumer to restore it by calling DigiModes_Restore_BufferConsumer();
static __IO uint32_t active_consumer = 0;
static __IO uint32_t prev_consumer = CW;

static uint8_t buf[ DIGIMODES_TX_BUFFER_SIZE ];
static uhsdr_ring_buffer_t digi_buffer;

inline void DigiModes_DigiBufferInit()
{
    uhsdr_ring_buffer_init( &digi_buffer, buf, DIGIMODES_TX_BUFFER_SIZE );
}

void DigiModes_Restore_BufferConsumer()
{
    active_consumer = prev_consumer;
}

/**
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

bool DigiModes_TxBufferRemove( uint8_t* c_ptr, digi_buff_consumer_t consumer )
{
    assert( c_ptr );
    assert((consumer & RTTY) || (consumer & BPSK) || (consumer & CW) || (consumer & UI));

    bool retval = false;

    if ( consumer == active_consumer && uhsdr_ring_buffer_get( &digi_buffer, c_ptr ))
    {
        /**
         * Push the removed char into UI buffer to display it on the screen.
         * Characters for UI and CW prints when they add into digi_buffer.
         */
        if ( active_consumer == RTTY || active_consumer == BPSK )
        {
            UiDriver_TextMsgPutChar( *c_ptr );
        }
        retval = true;
    }
    return retval;
}

/** no room left in the buffer returns 0 */
int32_t DigiModes_TxBufferPutChar( uint8_t c, digi_buff_consumer_t source )
{
    assert((source & CW) || (source & KeyBoard) || (source & UI));

    int32_t ret = 0;
    /**
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
        if ( uhsdr_ring_buffer_put( &digi_buffer, c ) && active_consumer == UI)
        {
            UiDriver_TextMsgPutChar( c );
            ret ++;
        }
    }
    return ret;
}

/** @todo-> Reimplement to have ability to push n-byte into buffer.  */
void DigiModes_TxBufferPutSign( const char* s, digi_buff_consumer_t source )
{
    assert( s );
    assert((source & CW) || (source & KeyBoard) || (source & UI));

    DigiModes_TxBufferPutChar( '<', source );
    DigiModes_TxBufferPutChar( s[0], source );
    DigiModes_TxBufferPutChar( s[1], source );
    DigiModes_TxBufferPutChar( '>', source );
}

void DigiModes_TxBufferReset()
{
    uhsdr_ring_buffer_flush( &digi_buffer );
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
