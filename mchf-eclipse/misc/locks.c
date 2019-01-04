/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/*************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Description:    Simple boolean locks for further adopting RTOS                 **
 **  Last Modified:  m-chichikalov (rv9yw) Implemented LOCKs as a step toward RTOS. **
 **  License:         GNU GPLv3                                                     **
 ************************************************************************************/

#include "locks.h"
#include <assert.h>
/* Semaphore is a bool type, should be initiated as volatile */

/**
 * Semaphore give
 */
void sem_give( volatile bool* sem )
{
    assert( sem );

	*sem = false;
//	xSemaphoreGiveFromISR( *sem, pdFALSE );
}

/**
 * Semaphore take, may suspend the caller thread
 */
int32_t sem_take( volatile bool* sem, uint32_t timeout )
{
    assert( sem );
    assert( timeout );

	(void)( timeout );
	// As long as we do not have any concurrency,
	// there is no way that semaphore is already taken by someone.
	// The below implementation for DMA or interrupt driven Functions,
	// they should give back semaphore after it's done.
	if ( !*sem )
	{
		*sem = true;
		return UHSDR_ERR_NONE;
	}
	while ( *sem )
	{
		//FIXME maybe add timeout for more robust code?
		asm("nop");
	}
	*sem = true; // take semaphore after it's released...
	return UHSDR_ERR_NONE;
//	return xSemaphoreTake( *sem, timeout ) ? UHSDR_ERR_NONE : UHSDR_ERR_TIMEOUT;
}
