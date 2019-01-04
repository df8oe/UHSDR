/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/*************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Description:    FMC boundary. Provides wrappers for HW depended functions.     **
 **  Last Modified:  rv9yw - m.chichikalov@outlook.com                              **
 **                  2018-12-21 : Added this file and initial implementation.       **
 **                                                                                 **
 **  License:         GNU GPLv3                                                     **
 ************************************************************************************/

//TODO review dependencies !!!
#include "uhsdr_board.h"
#include "uhsdr_mcu.h"

#include <assert.h>
#include "uhsdr_hw_fmc.h"
#include "locks.h"

#ifdef UI_BRD_OVI40
	#include "fmc.h"
#else
	#include "fsmc.h"
#endif

#define DEFAULT_FMC_TIMEOUT 100

static volatile ushort* __address;

// There is only one FMC in use, so implemented in more simple way than SPI.
static HW_INTERFACE_t available_fmc[ ] = {
    #if defined ( STM32F4 )
         { (void*)&hdma_mem_to_mem, false, DEFAULT_FMC_TIMEOUT, NULL, MX_FSMC_Init, MX_FSMC_DeInit },
    #elif defined ( STM32F7 ) || defined ( STM32H7 )
         // FIXME correct code for F7 and H7
        #error "Not added mem-to-mem for F7 and H7."
         { (void*)&hdma_mem_to_mem, false, DEFAULT_FMC_TIMEOUT, NULL, MX_FSMC_Init(), NULL },
    #else
        #error "For now! UHSDR supports only stm32 MCU."
    #endif

    #if defined ( SOME_OTHERS_MCU )
    #endif
};

int32_t UHSDR_FMC_RequestTransaction ( UHSDR_INTERFACE_t* uhsdr_fmc, uint32_t time_out ) // take SPI semaphore
{
    // We do not save uhsdr_fmc as a parent as we have only one fmc interface
    return sem_take ( &uhsdr_fmc->hw->lock, time_out );
}

void UHSDR_FMC_CloseTransaction ( UHSDR_INTERFACE_t* uhsdr_fmc ) // give back SPI semaphore
{
    sem_give ( &uhsdr_fmc->hw->lock );
}

//call back on DMA done for memory-to-memory
static void MEM_TO_MEM_TxCpltCallback ( DMA_HandleTypeDef *_hdma )
{
    //Using fixed pointer to dma handler as we have only one fmc interface.
    if ( _hdma == &hdma_mem_to_mem )
    {
        __DMB ( );
        // give back FMC semaphore
        UHSDR_FMC_CloseTransaction ( available_fmc[ __UHSDR_FMC_DISPLAY__ ].parent );
    }
}

void UHSDR_FMC_Send_Polling ( UHSDR_INTERFACE_t* uhsdr_fmc, uint8_t* source, uint16_t size,
        IO_bitwidth_e width )
{
    (void) (uhsdr_fmc);
    assert( source );
    assert( size );

    if ( width == DATA_16_BIT )
    {
        ushort* __source = (uint16_t*)source;
        while ( size-- )
        {
            *__address = *__source++;
        }
    }
    else
    {
        while ( size-- )
        {
            *__address = *source++;
        }
    }

    __DMB ( );
}

//	TODO
void UHSDR_FMC_Receive_Polling ( UHSDR_INTERFACE_t* uhsdr_fmc, uint8_t* buffer, uint16_t size,
        IO_bitwidth_e width )
{
    assert( buffer );
    assert( size );
    assert( false );
}

// FIXME remove later
void UHSDR_FMC_SendReceive_Polling ( UHSDR_INTERFACE_t* uhsdr_fmc, uint8_t* tx_buffer,
        uint8_t* rx_buffer, uint16_t size, IO_bitwidth_e width )
{
    assert( rx_buffer );
    assert( tx_buffer );
    assert( size );
    assert( false ); // FAILED
}

void UHSDR_FMC_Send_DMA ( UHSDR_INTERFACE_t* uhsdr_fmc, uint8_t* source, uint16_t size,
        IO_bitwidth_e width )
{
    assert( source );
    assert( size );

    HAL_DMA_Start_IT ( &hdma_mem_to_mem, (uint32_t)source, (uint32_t)__address, size );
}

void UHSDR_FMC_Receive_DMA ( UHSDR_INTERFACE_t* uhsdr_fmc, uint8_t* source, uint16_t size,
        IO_bitwidth_e width )
{
    (void) (uhsdr_fmc);
    assert( source );
    assert( size );
    assert( false ); // FAILED
    // FIXME not implemented
}

static int32_t UHSDR_FMC_Init ( UHSDR_INTERFACE_t* uhsdr_fmc )
{
    uhsdr_fmc->hw->parent = uhsdr_fmc; // save as parent to use from irq callback

    uhsdr_fmc->hw->Init ( );
    HAL_DMA_RegisterCallback ( &hdma_mem_to_mem, HAL_DMA_XFER_CPLT_CB_ID,
            MEM_TO_MEM_TxCpltCallback );
    return 0;
}

static int32_t UHSDR_FMC_DeInit ( UHSDR_INTERFACE_t* uhsdr_fmc )
{
    uhsdr_fmc->hw->DeInit ( );
    return 0;
}

const IO_t FMC_IO = {
    .Init               = UHSDR_FMC_Init,
    .DeInit             = UHSDR_FMC_DeInit,

    .RequestTransaction = UHSDR_FMC_RequestTransaction,
    .CloseTransaction   = UHSDR_FMC_CloseTransaction,

    .Send_DMA           = UHSDR_FMC_Send_DMA,
    .Receive_DMA        = UHSDR_FMC_Receive_DMA,

    .Send_Polling       = UHSDR_FMC_Send_Polling,
    .Receive_Polling    = UHSDR_FMC_Receive_Polling,
};

inline void UHSDR_FMC_SetAddress ( volatile uint16_t* address )
{
    assert( address);

    __address = address;
}

inline void UHSDR_FMC_SetTimeout ( UHSDR_INTERFACE_t* uhsdr_fmc, uint32_t time_out )
{
    assert( uhsdr_fmc );
    assert( uhsdr_fmc->hw );

    uhsdr_fmc->hw->time_out = time_out;
}

void UHSDR_FMC_GetInstance ( UHSDR_INTERFACE_t* uhsdr_fmc, fmc_e fmc_name )
{
    uhsdr_fmc->IO = &FMC_IO;
    uhsdr_fmc->hw = &available_fmc[ fmc_name ];
}
