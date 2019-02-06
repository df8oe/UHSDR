/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **  Licence:     GNU GPLv3                                                        **
 ************************************************************************************/

/**
 * @file uhsdr_ring_buffer.h
 * @author m.chichikalov@outlook.com (rv9yw)
 * @brief Generalized ring buffer.
 *
 * Lock-free implementation.
 * @notice Re-entrant and multi-threaded only when there are not more than two producers.
 *
 * @code C
 * static uint8_t buf[ DIGIMODES_TX_BUFFER_SIZE ];
 * static uhsdr_ring_buffer_t digi_buffer;
 *
 * void DigiModes_DigiBufferInit()
 * {
 *     uhsdr_ring_buffer_init( &digi_buffer, buf, DIGIMODES_TX_BUFFER_SIZE );
 * }
 * @endcode
 */

#ifndef UHSDR_RING_BUFFER_H_
#define UHSDR_RING_BUFFER_H_


typedef struct __attribute__ ((packed)) uhsdr_ring_buffer_ixds_pair
{
    uint16_t index;
    uint16_t shadow_index;
} uhsdr_ring_buffer_ixds_pair_t;

/**
 * \brief Ring buffer handler type
 */
typedef struct uhsdr_ring_buffer {
    uint8_t *buf;                       /**< buffer base address */
    uint32_t size;                      /**< buffer size */
    uhsdr_ring_buffer_ixds_pair_t tail; /**< tail pair of index */
    uhsdr_ring_buffer_ixds_pair_t head; /**< tail pair of index */
} uhsdr_ring_buffer_t;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initiate ring buffer handler.
 *
 * @example
 *
 * @param r_buf[in] Pointer to the buffer handler.
 * @param buf[in]
 * @param size[in] size of allocated data (number of bytes in statically defined array).
 */
void uhsdr_ring_buffer_init( uhsdr_ring_buffer_t* const r_buf, void *buf, uint32_t size );

/**
 * @brief Pull one byte from ring buffer.
 *
 * @param r_buf[in] Pointer at the buffer handler.
 * @param data[out] One byte space to store into.
 * @return return 1 if byte was taken successfully, otherwise 0.
 */
uint32_t uhsdr_ring_buffer_get( uhsdr_ring_buffer_t* const r_buf, uint8_t* data );

/**
 * @brief Put one entry into the ring buffer.
 *
 * @param r_buf[in] Pointer at the buffer handler.
 * @param data[in] byte of data which should be pushed into buffer.
 * @return return 1 if byte was pushed successfully, otherwise 0.
 */
uint32_t uhsdr_ring_buffer_put( uhsdr_ring_buffer_t* const r_buf, uint8_t data );

/**
 * @brief Flush(reset) ring buffer.
 *
 * @param r_buf Pointer to the buffer handler.
 */
void uhsdr_ring_buffer_flush( uhsdr_ring_buffer_t* const r_buf );


#ifdef __cplusplus
}
#endif

#endif /* UHSDR_RING_BUFFER_H_ */

/*** end of file ***/
