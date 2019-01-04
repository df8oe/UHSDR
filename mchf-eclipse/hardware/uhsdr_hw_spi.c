/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/*************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Description:    SPI boundary. Provides wrappers for HW depended functions.     **
 **  Last Modified:  rv9yw - m.chichikalov@outlook.com                              **
 **                  2018-12-20 : Added this file and initial implementation.       **
 **                                                                                 **
 **  License:        GNU GPLv3                                                      **
 ************************************************************************************/
#include <assert.h>
#include "locks.h"

//TODO review dependencies !!!
#include "uhsdr_board.h"
#include "uhsdr_mcu.h"
#include "spi.h"

#include "uhsdr_hw_spi.h"
//#include "unity.h"

#define DEFAULT_SPI_TIMEOUT ( 200U ) // TODO review when RTOS added

#if defined ( STM32F7 ) || defined ( STM32H7 ) || defined ( STM32F4 )
    #define SPI_HW_CAST_TYPE (SPI_HandleTypeDef*)
#elif defined ( SOME_OTHER_ARCH_CPU )
    #error "For now! UHSDR supports only stm32 MCU."
#endif

// all spi should be at the same order as spi_e ENUM.
static HW_INTERFACE_t available_spi[ ] = {
    #if defined ( STM32F4 ) || defined ( STM32F7 ) || defined ( STM32H7 )
         { (void*)&hspi2, false, DEFAULT_SPI_TIMEOUT, NULL, MX_SPI2_Init, MX_SPI2_DeInit },
    #endif
    #if defined ( STM32F7 ) || defined ( STM32H7 )
        { (void*)&hspi3, false, DEFAULT_SPI_TIMEOUT, NULL, MX_SPI3_Init, MX_SPI3_DeInit },
        { (void*)&hspi6, false, DEFAULT_SPI_TIMEOUT, NULL, MX_SPI6_Init, MX_SPI6_DeInit },
    #endif
};

static int32_t UHSDR_SPI_Init ( UHSDR_INTERFACE_t* uhsdr_spi )
{
    assert( uhsdr_spi );

    // STM32_HAL links dma into SPI handler in spi.c
    // can be accessed by ( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->hdmatx
    uhsdr_spi->hw->Init ( );
    return 0; // FIXME should we return status?
}

static int32_t UHSDR_SPI_DeInit ( UHSDR_INTERFACE_t* uhsdr_spi )
{
    assert( uhsdr_spi );

    uhsdr_spi->hw->DeInit ( );
    return 0; // FIXME should we return status?
}

static int32_t UHSDR_SPI_RequestTransaction ( UHSDR_INTERFACE_t* uhsdr_spi, uint32_t time_out )
{
    assert( uhsdr_spi );
    assert( uhsdr_spi->extra_parameter.spi.cs.GPIO_port );

    int retval = sem_take ( &uhsdr_spi->hw->lock, time_out );

    // FIXME now the dummy semaphore implementation returns only ERR_NONE
    if ( retval == UHSDR_ERR_NONE )
    {
        uhsdr_spi->hw->parent = uhsdr_spi; // save as a parent to restore in irq handler

        // FIXME should get rid of dependency of GPIO_TypeDef from here.
        GPIO_ResetBits(( GPIO_TypeDef* ) uhsdr_spi->extra_parameter.spi.cs.GPIO_port,
                uhsdr_spi->extra_parameter.spi.cs.GPIO_pin );
    }
    return retval;
}

static void UHSDR_SPI_CloseTransaction ( UHSDR_INTERFACE_t* uhsdr_spi ) // give back SPI semaphore
{
    assert( uhsdr_spi );
    assert( uhsdr_spi->extra_parameter.spi.cs.GPIO_port );

    GPIO_SetBits( (GPIO_TypeDef* )uhsdr_spi->extra_parameter.spi.cs.GPIO_port,
            uhsdr_spi->extra_parameter.spi.cs.GPIO_pin );
    sem_give ( &uhsdr_spi->hw->lock );
}

void HAL_SPI_TxCpltCallback ( SPI_HandleTypeDef *hspi )
{
    uint32_t idx = NUMBER_OF_USED_SPI;
    do
    {
        if ( hspi == available_spi[ idx ].handler )
        {
            UHSDR_SPI_CloseTransaction ( available_spi[ idx ].parent ); // give back SPI semaphore
        }
    } while ( --idx );
}

static inline void UHSDR_SPI_SetBitWidth( UHSDR_INTERFACE_t* uhsdr_spi, uint32_t width )
{
    assert( uhsdr_spi );
    assert( width != DATA_32_BIT); // supported only by H7

    uint32_t bit_width;
    bit_width = ( width & ( ~SPI_DMA_mask )); // clear value from DMA settings
    ( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Init.DataSize = bit_width;

    #if defined ( STM32F4 )
        MODIFY_REG(( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Instance->CR1, SPI_CR1_DFF, bit_width );
    #elif defined ( STM32F7 )
        MODIFY_REG(( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Instance->CR2, SPI_CR2_DS, bit_width );
        #warning "Not tested on F7."
    #elif defined ( STM32H7 )
        MODIFY_REG(( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Instance->CFG1, SPI_CFG1_DSIZE, bit_width );
        #warning "Not tested on H7."
    #endif
}

static inline void UHSDR_SPI_SetDMA_Settings( UHSDR_INTERFACE_t* uhsdr_spi, uint32_t settings )
{
    assert( uhsdr_spi );

    #if defined ( STM32F4 ) || defined ( STM32F7 ) || defined ( STM32H7 )
    uint32_t mask = DMA_SxCR_PSIZE | DMA_SxCR_MSIZE | DMA_SxCR_MINC;
    uint32_t set = DMA_MDATAALIGN_BYTE | DMA_PDATAALIGN_BYTE | DMA_MINC_DISABLE;

    if (( settings & ( ~SPI_DMA_mask )) == SPI_16_BIT )
    {
        set |= DMA_MDATAALIGN_HALFWORD | DMA_PDATAALIGN_HALFWORD;
    }

    if (( settings & ( ~SPI_DATASIZE_mask )) == SPI_DMA_NORMAL_MODE )
    {
        set |= DMA_MINC_ENABLE;
    }

    MODIFY_REG(( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->hdmatx->Instance->CR, mask, set );
    #endif
}

static void UHSDR_SPI_Send_Polling ( UHSDR_INTERFACE_t* uhsdr_spi, uint8_t* source, uint16_t size,
        uint32_t width )
{
    assert( source );
    assert( size );

    UHSDR_SPI_SetBitWidth( uhsdr_spi, width );
    HAL_SPI_Transmit ( SPI_HW_CAST_TYPE( uhsdr_spi->hw->handler ), source, size, uhsdr_spi->hw->time_out );
}

static void UHSDR_SPI_Receive_Polling ( UHSDR_INTERFACE_t* uhsdr_spi, uint8_t* buffer, uint16_t size,
        uint32_t width )
{
    assert( buffer );
    assert( size );

//	FIXME handle different size
    HAL_SPI_Receive ( SPI_HW_CAST_TYPE( uhsdr_spi->hw->handler ), buffer, size, uhsdr_spi->hw->time_out );
}

static void UHSDR_SPI_SendReceive_Polling ( UHSDR_INTERFACE_t* uhsdr_spi, uint8_t* tx_buffer,
        uint8_t* rx_buffer, uint16_t size, uint32_t width )
{
    assert( rx_buffer );
    assert( tx_buffer );
    assert( size );

//	FIXME handle different size
    HAL_SPI_TransmitReceive ( SPI_HW_CAST_TYPE (uhsdr_spi->hw->handler), tx_buffer, rx_buffer,
            size, uhsdr_spi->hw->time_out );
}

// FIXME DMA works only with 16 bit data now.
// 'settings' could be as OR of next parameters
// (  SPI_8_BIT | SPI_DMA_CIRCULAR_MODE )
// ( SPI_16_BIT | SPI_DMA_NORMAL_MODE )
static void UHSDR_SPI_Send_DMA ( UHSDR_INTERFACE_t* uhsdr_spi, uint8_t* source, uint16_t size,
        uint32_t settings )
{
    assert( source );
    assert( size );

#if defined ( STM32H7 )
    assert(( width & ( ~SPI_DMA_mask )) != SPI_32_BIT );
#endif

    UHSDR_SPI_SetBitWidth( uhsdr_spi, settings );
    UHSDR_SPI_SetDMA_Settings( uhsdr_spi, settings );

    HAL_SPI_Transmit_DMA ( SPI_HW_CAST_TYPE (uhsdr_spi->hw->handler), source, size );
}

// We are not using HDMA to RX over SPI. Maybe will implement later.
static void UHSDR_SPI_Receive_DMA ( UHSDR_INTERFACE_t* uhsdr_spi, uint8_t* source, uint16_t size,
        uint32_t width )
{
    (void) (uhsdr_spi);
    (void) (source);
    (void) (size);
    (void) (width);
    assert( false ); // Failed if called
}

static const IO_t SPI_IO = {
    .Init               = UHSDR_SPI_Init,
    .DeInit             = UHSDR_SPI_DeInit,
    .RequestTransaction = UHSDR_SPI_RequestTransaction,
    .CloseTransaction   = UHSDR_SPI_CloseTransaction,
    .Send_DMA           = UHSDR_SPI_Send_DMA,
    .Send_Polling       = UHSDR_SPI_Send_Polling,
    .Receive_Polling    = UHSDR_SPI_Receive_Polling,
    .Receive_DMA        = UHSDR_SPI_Receive_DMA,
    // FIXME remove this one after replace call of this to regular send and receive in
    // touch driver.
    .SendReceive_Polling= UHSDR_SPI_SendReceive_Polling,
};

void UHSDR_SPI_GetInstance ( UHSDR_INTERFACE_t* uhsdr_spi, spi_e spi_name )
{
    assert( uhsdr_spi );

    uhsdr_spi->IO = &SPI_IO;
    uhsdr_spi->hw = &available_spi[ spi_name ];
}

spi_speed_clock_e UHSDR_SPI_ChangeClockSpeed ( UHSDR_INTERFACE_t* uhsdr_spi, spi_speed_clock_e speed )
{
    assert( uhsdr_spi );

    spi_speed_clock_e retval;
    #if defined ( STM32F4 ) || defined ( STM32F7 ) || defined ( STM32H7 )
        retval = ( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Init.BaudRatePrescaler;
        ( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Init.BaudRatePrescaler = speed;

        // FIXME not checked that SPI is disabled and value of register can be adjusted.
        // However, RequestTransaction() before calling this one protects from changing
        // at not proper time.
        #if defined ( STM32F4 )
            MODIFY_REG(( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Instance->CR1,
                    SPI_CR1_BR, speed );
        #elif defined ( STM32F7 )
            #warning "Changing SPI clock is not tested on F7."
            MODIFY_REG(( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Instance->CR1,
                    SPI_CR1_BR, speed );
        #elif defined ( STM32H7 )
            #warning "Changing SPI clock is not tested on H7."
            MODIFY_REG(( SPI_HW_CAST_TYPE uhsdr_spi->hw->handler)->Instance->CFG1,
                    ( 0x7U << ( 28U )), speed );
        #endif

    #else // defined ( STM32F4 ) || defined ( STM32F7 ) || defined ( STM32H7 )
        #error "For now! UHSDR supports only stm32 MCU."
    #endif // defined ( STM32F4 ) || defined ( STM32F7 ) || defined ( STM32H7 )
    return retval; // Return previous value
}

inline void UHSDR_SPI_SetTimeout ( UHSDR_INTERFACE_t* uhsdr_spi, uint32_t time_out )
{
    assert( uhsdr_spi );
    assert( uhsdr_spi->hw );
    uhsdr_spi->hw->time_out = time_out;
}

inline void UHSDR_SPI_SetChipSelectPin ( UHSDR_INTERFACE_t* uhsdr_spi, dio_t* cs )
{
    assert( uhsdr_spi );
    uhsdr_spi->extra_parameter.spi.cs = *cs;
}
