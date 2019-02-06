/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **  Licence:     GNU GPLv3                                                        **
 ************************************************************************************/

/**
 * @file uhsdr_ring_buffer.c
 * @brief Generalized ring buffer.
 */

#include <assert.h>
#include "uhsdr_types.h"
#include "uhsdr_ring_buffer.h"


void uhsdr_ring_buffer_init( uhsdr_ring_buffer_t* const r_buf, void *buf, uint32_t size )
{
    assert( r_buf && buf && size );

    r_buf->buf         = (uint8_t *)buf;
    r_buf->size        = size;

    r_buf->head.index = r_buf->head.shadow_index = 0;
    r_buf->tail.index = r_buf->tail.shadow_index = 0;
}

uint32_t uhsdr_ring_buffer_get( uhsdr_ring_buffer_t* const r_buf, uint8_t* data )
{
    assert( r_buf && data );

    uint32_t retval = 0;

    uint32_t tail_pair;
    uint32_t new_tail;
    uhsdr_ring_buffer_ixds_pair_t* tail_pair_ptr = (uhsdr_ring_buffer_ixds_pair_t*) &tail_pair;
    do
    {
        tail_pair = __LDREXW(( volatile uint32_t* )&r_buf->tail );

        /**
         * if HEAD's index not equal HEAD's shadow_index, that means some thread obtained
         * index for putting entry into buffer, but not done yet.
         */
        if( r_buf->head.index != r_buf->head.shadow_index )
        {
            __CLREX();
            continue;
        }

        if ( r_buf->head.index != tail_pair_ptr->index )
        {
            *data = r_buf->buf[ tail_pair_ptr->index ];
            new_tail = ( tail_pair_ptr->index + 1 ) %  r_buf->size;
            retval = 1;
        }
        else
        {
            /** there is no room, write back old values */
            new_tail = (uint32_t)tail_pair;
        }
    } while( __STREXW( new_tail, ( volatile uint32_t* )&r_buf->tail));
    __DMB();

    return retval;
}

uint32_t uhsdr_ring_buffer_put( uhsdr_ring_buffer_t* const r_buf, uint8_t data )
{
    assert( r_buf );

    uint32_t retval = 0;

    uint32_t head_pair;
    uint32_t next_head;
    uhsdr_ring_buffer_ixds_pair_t* head_pair_ptr = (uhsdr_ring_buffer_ixds_pair_t*) &head_pair;
    do
    {
        head_pair = __LDREXW(( volatile uint32_t* )&r_buf->head );
        next_head = ( head_pair_ptr->index + 1 ) % r_buf->size;
        if ( next_head != r_buf->tail.index )
        {
            /** There is room */
            retval = 1;
        }
        else
        {
            /** There is no room, write back old values */
            next_head = (uint32_t)head_pair;
        }
    } while( __STREXW( next_head, ( volatile uint32_t* )&r_buf->head ));

    /**
     * Update entry in buffer only if head' index was mutually exclusively obtained
     * and there is space for one more entry.
     *
     * The concurrent thread ( which trying to get at this time)
     * will wait until index and shadow index in pair would be the same.
     */
    if  ( retval != 0 )
    {
        r_buf->buf[ head_pair_ptr->index ] = data;
        __atomic_load( &r_buf->head.index, &r_buf->head.shadow_index, __ATOMIC_SEQ_CST );
    }
    __DMB();

    return retval;
}

void uhsdr_ring_buffer_flush( uhsdr_ring_buffer_t* const r_buf )
{
    assert( r_buf );

    __atomic_load( &r_buf->head, &r_buf->tail, __ATOMIC_SEQ_CST );
}

/*** end of file ***/
