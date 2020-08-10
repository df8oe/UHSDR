/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/
#ifndef __UHSDR_HMC1023_H
#define __UHSDR_HMC1023_H
#include "uhsdr_board.h"

#define HMC1023_OK (HAL_OK)
#define HMC1023_ERROR (0xffffffff)

typedef struct
{
    bool present;
#ifdef USE_HMC1023
    uint32_t reg1;
    uint32_t reg2;
    uint32_t reg3;

    // we use these to track the configration settings
    uint8_t coarse;
    uint8_t fine;
    uint8_t opamp;
    uint8_t drvr;

    bool gain;
    bool bypass;
#endif
} HMC1023_t;

extern HMC1023_t hmc1023;

#ifdef USE_HMC1023

void hmc1023_init(void);
void hmc1023_use_spi_settings(bool on);
void hmc1023_set_coarse(uint8_t coarse);
void hmc1023_set_fine(uint8_t fine);
void hmc1023_set_gain(bool on);
void hmc1023_set_bypass(bool on);
void hmc1023_set_bias_opamp(uint8_t value);
void hmc1023_set_bias_drvr(uint8_t value);

#endif

#endif
