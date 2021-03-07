/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                                        UHSDR                                    **
**               a powerful firmware for STM32 based SDR transceivers              **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#ifndef __HW_OSC_DUCDDC_DF8OE
#define __HW_OSC_DUCDDC_DF8OE
#include "osc_interface.h"

extern const OscillatorInterface_t osc_ducddc;

bool DucDdc_Df8oe_EnableRx(void);
bool DucDdc_Df8oe_EnableTx(void);
bool DucDdc_Df8oe_PrepareTx(void);
bool DucDdc_Df8oe_PrepareRx(void);
bool DucDdc_Df8oe_TxCWOnOff(bool on);
bool DucDdc_Df8oe_Config(void);

#endif
