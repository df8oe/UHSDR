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

#ifndef __IQ_TX_FILTER_H
#define __IQ_TX_FILTER_H

#include "filters.h"

#define IQ_TX_BLOCK_SIZE		32
#define IQ_TX_NUM_TAPS			89
#define IQ_TX_NUM_TAPS_WIDE		201
#define IQ_TX_NUM_TAPS_MAX       201


extern const IQ_FilterDescriptor iq_tx_narrow;
extern const IQ_FilterDescriptor iq_tx_wide;

#endif
