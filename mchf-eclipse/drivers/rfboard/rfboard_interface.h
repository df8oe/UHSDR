/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Description:  High Level Interface for various RFBoards                        **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/
/*
 * radio_management.h
 *
 *  Created on: 23.12.2016
 *      Author: danilo
 */

#ifndef DRIVERS_RFBOARD_INTERFACE_H_
#define DRIVERS_RFBOARD_INTERFACE_H_

#include "uhsdr_types.h"
#include "uhsdr_board.h"



typedef struct {
    power_level_t id;
    int32_t   mW;
} power_level_desc_t;

typedef struct {
    const power_level_desc_t* levels;
    const uint32_t count;
} pa_power_levels_info_t;

typedef struct
{
    char* name;
    float32_t  reference_power;
    uint32_t  max_freq;
    uint32_t  min_freq;
    int32_t max_am_power;
    int32_t max_power; // power level upper limit, used for display
} pa_info_t;

//definition of specific RF hardware features
typedef int8_t (*hRFb_RXATT)(void);

typedef struct {
    hRFb_RXATT AMP_ATT_prev;
    hRFb_RXATT AMP_ATT_next;
    hRFb_RXATT AMP_ATT_getCurrent;

    const pa_power_levels_info_t* power_levelsInfo;
    const pa_info_t* pa_info;
    bool    (*EnableTx)(void);
    bool    (*EnableRx)(void);
    bool    (*PrepareTx)(void);
    bool    (*PrepareRx)(void);
    void (*CodecRestart)(void);
} HardwareRFBoard;

extern HardwareRFBoard RFboard;


void RFBoard_Init_Board(void);

#endif /* DRIVERS_RFBOARD_INTERFACE_H_ */
