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


typedef struct
{
    const char* creator;
    const char* license;
    const char* name;
} hardware_ident_t;

#if 0
typedef enum
{
  POWERSCALING_INTERNAL, // UHSDR signal processing handles power control by scaling the output
  POWERSCALING_EXTERNAL, // UHSDR signal processing outputs with constant scaling, external system handles output power control
} rf_power_scaling_t;
#endif

typedef struct {
    power_level_t id;
    int32_t   mW;
} power_level_desc_t;

typedef struct {
    const power_level_desc_t* levels;
    const uint32_t count;
    //const rf_power_scaling_t power_scaling_mode;
    const float power_factor; // POWERSCALING_INTERNAL -> not used, POWERSCALING_EXTERNAL -> used in signal processing instead of calculated power factor
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
    bool    (*ChangeFrequency)(uint32_t frequency);
    void (*CodecRestart)(void);
    bool    (*InitBoard)(void);
    bool    (*ConfigBoard)(void);
    bool    (*SetPABias)(uint32_t bias); // a bias of 0 must disable transmission without disabling the PA signal path
    bool    (*SetPowerFactor)(float pf);
    const hardware_ident_t* description;
    bool iq_balance_required; // tells us if the rfboard requires iq balance adjustments
    // if set, external power factor control is assumed, pa_info->power_factor is used in signal procssing
    // and this function is called with the calculated power factor;
} HardwareRFBoard;

extern HardwareRFBoard RFboard;


bool RFBoard_Init_Board(void);
bool RFBoard_Config_Board(void);

#endif /* DRIVERS_RFBOARD_INTERFACE_H_ */
