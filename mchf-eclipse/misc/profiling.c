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
 **  Licence:       GNU GPLv3                                                      **
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

#if 0
// the code below is only used to ease profiling with eclipse
// you just need hover over a variable to get the value
void dummy() {

    eventProfile.event[0].duration;
    eventProfile.event[0].count;
    eventProfile.event[1].duration;
    eventProfile.event[1].count;
    eventProfile.event[2].duration;
    eventProfile.event[2].count;
    eventProfile.event[3].duration;
    eventProfile.event[3].count;
    eventProfile.event[4].duration;
    eventProfile.event[4].count;
    eventProfile.event[5].duration;
    eventProfile.event[5].count;
    eventProfile.event[6].duration;
    eventProfile.event[6].count;
    eventProfile.event[7].duration;
    eventProfile.event[7].count;

    eventProfile.event[8].duration;
    eventProfile.event[8].count;


}
#endif


void profileEventsTracePrint()
{
#ifdef XPROFILE_EVENTS

            for (int i = 0;i < 10;i++)
            {
                ProfilingTimedEvent* ev_ptr = profileTimedEventGet(i);
                if (ev_ptr->count != 0)
                {
                    trace_printf("%d: %d uS per run\n",i, (ev_ptr->duration/(ev_ptr->count*168)));
                }
            }
#endif
}
