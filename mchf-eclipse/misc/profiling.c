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
 **  Licence:       CC BY-NC-SA 3.0                                                **
 ************************************************************************************/

// Common
#include "mchf_board.h"
#include "profiling.h"

/*
 * In order to read the counters here, you'll need to connect
 * using a real-time debugger, pause execution and read values.
 * Not a big deal with ST-Link and Eclipse or gdb.
 */
EventProfile_t eventProfile;
void dummy() {
    eventProfile.event[0].duration;
    eventProfile.event[0].count;

    eventProfile.event[2].duration;
    eventProfile.event[2].count;
    eventProfile.event[3].duration;
    eventProfile.event[3].count;
    eventProfile.event[4].duration;
    eventProfile.event[4].count;
    eventProfile.event[7].duration;
    eventProfile.event[7].count;

    eventProfile.event[8].duration;
    eventProfile.event[8].count;


}

