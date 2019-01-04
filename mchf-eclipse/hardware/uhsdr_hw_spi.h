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

#ifndef __MCHF_HW_SPI_H
#define __MCHF_HW_SPI_H

#include "uhsdr_types.h"
#include "uhsdr_structs.h"

// FIXME temporary workaround to avoid including uhsdr_mcu.h and all HAL_STM32 with it
// TODO needs to be moved to config file, after it will have been refactoring
#if defined( CORTEX_M7 )
    #ifdef STM32H743xx
        #define STM32H7
    #else
        #define STM32F7
    #endif
#elif defined( CORTEX_M4 )
    #define STM32F4
#else
    #error "CORTEX_M7 or CORTEX_M4 should be defined."
#endif

// SPI_BAUDRATEs redefined to NOT expose the STM32HAL library to every one who includes this...
// TODO -> try to find more elegant way.
#if !defined ( SPI_BAUDRATEPRESCALER_2 ) && ( defined ( STM32F4 ) || defined ( STM32F7 ))
    #define SPI_BAUDRATEPRESCALER_2         ((uint32_t)0x00000000U)
    #define SPI_BAUDRATEPRESCALER_4         ((uint32_t)0x00000008U)
    #define SPI_BAUDRATEPRESCALER_8         ((uint32_t)0x00000010U)
    #define SPI_BAUDRATEPRESCALER_16        ((uint32_t)0x00000018U)
    #define SPI_BAUDRATEPRESCALER_32        ((uint32_t)0x00000020U)
    #define SPI_BAUDRATEPRESCALER_64        ((uint32_t)0x00000028U)
    #define SPI_BAUDRATEPRESCALER_128       ((uint32_t)0x00000030U)
    #define SPI_BAUDRATEPRESCALER_256       ((uint32_t)0x00000038U)
#elif !defined ( SPI_BAUDRATEPRESCALER_2 ) && defined ( STM32H7 )
    #define SPI_BAUDRATEPRESCALER_2         (0x00000000U)
    #define SPI_BAUDRATEPRESCALER_4         (0x10000000U)
    #define SPI_BAUDRATEPRESCALER_8         (0x20000000U)
    #define SPI_BAUDRATEPRESCALER_16        (0x30000000U)
    #define SPI_BAUDRATEPRESCALER_32        (0x40000000U)
    #define SPI_BAUDRATEPRESCALER_64        (0x50000000U)
    #define SPI_BAUDRATEPRESCALER_128       (0x60000000U)
    #define SPI_BAUDRATEPRESCALER_256       (0x70000000U)
#endif

typedef enum
{
#if defined ( STM32F4 )
    SPI_clock_VeryFast = SPI_BAUDRATEPRESCALER_2,
    SPI_clock_Medium   = SPI_BAUDRATEPRESCALER_4,
    SPI_clock_VerySlow = SPI_BAUDRATEPRESCALER_64,
#elif defined ( STM32F7 )
    SPI_clock_VeryFast = SPI_BAUDRATEPRESCALER_4,
    SPI_clock_Medium   = SPI_BAUDRATEPRESCALER_8,
    SPI_clock_VerySlow = SPI_BAUDRATEPRESCALER_128,
#elif defined ( STM32H7 )
    SPI_clock_VeryFast = SPI_BAUDRATEPRESCALER_4,
    SPI_clock_Medium   = SPI_BAUDRATEPRESCALER_8,
    SPI_clock_VerySlow = SPI_BAUDRATEPRESCALER_32,
#endif
} spi_speed_clock_e;

// TODO -> try to find more elegant way.
#if !defined ( SPI_DATASIZE_8BIT ) && defined ( STM32F4 )
    #define SPI_DATASIZE_8BIT               ((uint32_t)0x00000000U)
    #define SPI_DATASIZE_16BIT              ((uint32_t)(0x1U << ( 11U )))
#elif !defined ( SPI_BAUDRATEPRESCALER_2 ) && defined ( STM32F7 )
    #define SPI_DATASIZE_8BIT               ((uint32_t)0x00000700U)
    #define SPI_DATASIZE_16BIT              ((uint32_t)0x00000F00U)
#elif !defined ( SPI_BAUDRATEPRESCALER_2 ) && defined ( STM32H7 )
    #define SPI_DATASIZE_8BIT               ((uint32_t)(0x00000007U))
    #define SPI_DATASIZE_16BIT              ((uint32_t)(0x0000000FU))
    #define SPI_DATASIZE_32BIT              ((uint32_t)(0x0000001FU))
#endif

#if !defined ( DMA_MINC_ENABLE ) && ( defined(STM32F4) || defined(STM32F7) ||defined(STM32H7))
    #define DMA_MINC_ENABLE         ((uint32_t)(0x1U << 10 ))  /*!< Memory increment mode enable  */
    #define DMA_MINC_DISABLE        ((uint32_t)0x00000000U)     /*!< Memory increment mode disable */
#endif

/*
 * \brief Described the SPI dma mode
 */
typedef enum
{
#if defined ( STM32F4 )
    SPI_8_BIT   = SPI_DATASIZE_8BIT,
    SPI_16_BIT  = SPI_DATASIZE_16BIT,
    SPI_DATASIZE_mask = SPI_DATASIZE_16BIT,
#endif
#if defined ( STM32F7 )
    SPI_8_BIT   = SPI_DATASIZE_8BIT,
    SPI_16_BIT  = SPI_DATASIZE_16BIT,
    SPI_DATASIZE_mask = SPI_DATASIZE_16BIT,
#endif
#if defined ( STM32H7 )
    SPI_8_BIT   = SPI_DATASIZE_8BIT,
    SPI_16_BIT  = SPI_DATASIZE_16BIT,
    SPI_32_BIT  = SPI_DATASIZE_32BIT,
    SPI_DATASIZE_mask = SPI_DATASIZE_32BIT,
#endif
} SPI_bitwidth_e;

/*
 * \brief Described the SPI dma mode
 */
typedef enum
{
#if defined ( STM32F4 ) || defined ( STM32F7 ) || defined ( STM32H7 )
    SPI_DMA_NORMAL_MODE    = DMA_MINC_ENABLE,
    SPI_DMA_CIRCULAR_MODE  = DMA_MINC_DISABLE,
    SPI_DMA_mask           = DMA_MINC_ENABLE,
#endif
} SPI_DMA_mode_e;

typedef enum
{
#if defined ( STM32F4 ) || defined ( STM32F7 ) || defined ( STM32H7 )
    __UHSDR_SPI2_DISPLAY__ = 0,
#endif
#if defined ( STM32F7 ) || defined ( STM32H7 )
    __UHSDR_SPI3__,
    __UHSDR_SPI6__,
#endif

    NUMBER_OF_USED_SPI, // should be the last in enum.
} spi_e;


#ifdef __cplusplus
extern "C"
{
#endif

/*
 * TODO
 */
void UHSDR_SPI_GetInstance ( UHSDR_INTERFACE_t* uhsdr_spi, spi_e spi_name );

/*
 * TODO
 */
void UHSDR_SPI_SetTimeout ( UHSDR_INTERFACE_t* uhsdr_spi, uint32_t time_out );

/*
 * TODO
 */
void UHSDR_SPI_SetChipSelectPin ( UHSDR_INTERFACE_t* uhsdr_spi, dio_t* cs );

/*
 * Changing SPI clock speed.
 * @return previous value.
 */
spi_speed_clock_e UHSDR_SPI_ChangeClockSpeed ( UHSDR_INTERFACE_t* uhsdr_spi, spi_speed_clock_e speed );
#ifdef __cplusplus
}
#endif

#endif // __MCHF_HW_SPI_H
