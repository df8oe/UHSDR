/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description: Simple Timing Profiler                                                                   **
**  Last Modified:                                                                 **
**  Licence:        CC BY-NC-SA 3.0                                                **
************************************************************************************/

#ifndef __PROFILING_H
#define __PROFILING_H

typedef enum {
    EnterAudioInterrupt = 0,
    EnterLO,
    EnterPTT,
    EnterDriverThread,
    EnterSMeter,
    EnterVoltage,
    EnterFreeDVEncode,
    FreeDVTXUnderrun,
    EventProfileMax
} ProfiledEventNames;

typedef struct {
    uint32_t count[EventProfileMax];
} EventProfile_t;

extern EventProfile_t eventProfile;

#define PROFILE_EVENTS

inline void profileEvent(ProfiledEventNames pe) {
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax && pe >= 0) {
        eventProfile.count[pe]++;
    }
#endif
}

#endif
