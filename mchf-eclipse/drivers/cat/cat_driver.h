/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#ifndef __CAT_DRIVER_H
#define __CAT_DRIVER_H

#include "mchf_types.h"

typedef enum
{
    CAT_DISCONNECTED = 0,
    CAT_CONNECTED
} CatInterfaceState;

typedef enum
{
    UNKNOWN = 0,
    FT817 = 1
} CatInterfaceProtocol;


// Exports
void CatDriver_InitInterface(void);
void CatDriver_StopInterface(void);

CatInterfaceState CatDriver_GetInterfaceState();

int CatDriver_InterfaceBufferAddData(uint8_t c);

void CatDriver_HandleProtocol();

bool CatDriver_CloneOutStart();
bool CatDriver_CloneInStart();

bool CatDriver_CWKeyPressed();
bool CatDriver_CatPttActive();

#endif
