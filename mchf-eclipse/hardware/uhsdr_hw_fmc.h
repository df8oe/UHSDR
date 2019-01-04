/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/*************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Description:    FMC boundary. Provides wrappers for HW depended functions.     **
 **  Last Modified:  rv9yw - m.chichikalov@outlook.com                              **
 **                  2018-12-20 : Added this file and initial implementation.       **
 **                                                                                 **
 **   static UHSDR_INTERFACE_t uhsdr_lcd; // statically allocated mem for struct    **
 **                                                                                 **
 **   UHSDR_FMC_GetInstance ( &uhsdr_lcd, __UHSDR_FMC_DISPLAY__ );                  **
 **   uhsdr_lcd.IO->Init( &uhsdr_lcd );                                             **
 **   UHSDR_FMC_SetAddress( volatile ushort* Address );                             **
 **                                                                                 **
 **   after this is done any functions could be called like                         **
 **                                                                                 **
 **   uhsdr_lcd.IO->Send_Polling( &uhsdr_lcd, ... );                                **
 **   uhsdr_lcd.IO->Receive_Polling( &uhsdr_lcd, ... );                             **
 **   uhsdr_lcd.IO->Send_DMA( &uhsdr_lcd, ... );                                    **
 **                                                                                 **
 **   it can simply be switched to use another interface                            **
 **   UHSDR_SPI_GetInstance ( &uhsdr_lcd, __UHSDR_SPI_DISPLAY__ );                  **
 **                                                                                 **
 **                                                                                 **
 **  License:         GNU GPLv3                                                     **
 ************************************************************************************/

#ifndef __MCHF_HW_FMC_H
#define __MCHF_HW_FMC_H

#include "uhsdr_types.h"
#include "uhsdr_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    #if defined ( STM32F4 ) || defined ( STM32F7 ) || defined ( STM32H7 )
        __UHSDR_FMC_DISPLAY__ = 0,
    #endif
        NUMBER_OF_USED_FMC, // should be the last in enum.
} fmc_e;

/*
 * TODO
 */
void UHSDR_FMC_GetInstance ( UHSDR_INTERFACE_t* uhsdr_fmc, fmc_e fmc_name );

/*
 * TODO
 */
void UHSDR_FMC_SetAddress( volatile uint16_t* Address );

/*
 * TODO
 */
void UHSDR_FMC_SetTimeout ( UHSDR_INTERFACE_t* uhsdr_spi, uint32_t time_out );


#ifdef __cplusplus
}
#endif

#endif // __MCHF_HW_FMC_H
