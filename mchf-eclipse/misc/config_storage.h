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
**  Licence:        GNU GPLv3                                                      **
************************************************************************************/
#ifndef __CONFIG_STORAGE_H
#define __CONFIG_STORAGE_H

#include "uhsdr_board.h"


void ConfigStorage_Init();

uint16_t ConfigStorage_ReadVariable(uint16_t addr, uint16_t *value);
uint16_t ConfigStorage_WriteVariable(uint16_t addr, uint16_t value);

void ConfigStorage_CopyFlash2Serial(void);
void ConfigStorage_CopySerial2Flash(void);

void ConfigStorage_CopySerial2RAMCache();
uint16_t ConfigStorage_CopyRAMCache2Serial();

uint16_t ConfigStorage_CopyArray2Serial(uint32_t Addr, const uint8_t *buffer, uint16_t length);
void ConfigStorage_CopySerial2Array(uint32_t Addr, uint8_t *buffer, uint16_t length);

#endif
