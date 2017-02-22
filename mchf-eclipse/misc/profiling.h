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
**  Licence:        GNU GPLv3                                                      **
************************************************************************************/

#ifndef __PROFILING_H
#define __PROFILING_H

typedef enum {
    ProfileAudioInterrupt = 0,
    ProfileTP1,
    ProfileTP2,
    ProfileTP3,
    ProfileTP4,
    ProfileTP5,
    ProfileTP6,
    ProfileTP7,
    ProfileTP8,
    ProfileTP9,
    ProfileFreeDV,
    FreeDVTXUnderrun,
    EventProfileMax
} ProfiledEventNames;

typedef struct {
    uint32_t count;
    uint32_t start;
    uint32_t stop;
    uint64_t duration; // to get average divide duration by count
} ProfilingTimedEvent;

typedef struct {
    ProfilingTimedEvent event[EventProfileMax];
} EventProfile_t;

extern EventProfile_t eventProfile;

#define PROFILE_EVENTS

inline void profileEvent(const ProfiledEventNames pe) {
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax && pe >= 0) {
        eventProfile.event[pe].count++;
    }
#endif
}


/***
 * How to use:
 * First start the cycle timer with profileTimeEventInit()
 * This resets the timer and makes it run.
 *
 * At any time call profileTimedEventStart(EventName)
 * to have the start being recorded
 * at the end of the event call profileTimedEventStop(EventName)
 * Duration of a single event should not be longer then 2^32 / clock frequency
 * i.e. ~25s @168 Mhz for a single event
 * total duration recorded should not be more than 2^64 cycles
 * which is quite a lot, i.e. there is no limit on that for the
 * average person in this universe
 *
 * Due to the approach multiple events can be recorded correctly but
 * outer events include the overhead of the calculation of the duration
 * only the innermost events are more or less accurate unless you time the profile functions and
 * remove the overhead later.
 */

void profileEventsTracePrint();


inline void profileTimedEventInit();
inline void profileTimedEventStart(const ProfiledEventNames pe);
inline void profileTimedEventStop(const ProfiledEventNames pe);
inline void profileTimedEventReset(const ProfiledEventNames pe);
inline  ProfilingTimedEvent* profileTimedEventGet(const ProfiledEventNames pe);


// INLINE IMPLEMENTATIONS

#define DWT_CYCCNT    ((volatile uint32_t *)0xE0001004)
#define DWT_CONTROL   ((volatile uint32_t *)0xE0001000)
#define SCB_DEMCR     ((volatile uint32_t *)0xE000EDFC)

inline void profileCycleCount_reset(){
    *SCB_DEMCR   |= 0x01000000;
    *DWT_CYCCNT  = 0; // reset the counter
    *DWT_CONTROL = 0;
}

inline void profileCycleCount_start()
{
    *DWT_CONTROL = *DWT_CONTROL | 1;
}

inline void profileCycleCount_stop()
{
    *DWT_CONTROL = *DWT_CONTROL  & ~1; //
}

inline uint32_t profileCycleCount_get()
{
    return *DWT_CYCCNT;
}

inline void profileTimedEventInit()
{
    profileCycleCount_reset();
    profileCycleCount_start();
}

inline void profileTimedEventStart(const ProfiledEventNames pe)
{
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax && pe >= 0) {
        eventProfile.event[pe].start = profileCycleCount_get();
    }
#endif

}
inline void profileTimedEventStop(const ProfiledEventNames pe)
{

#ifdef PROFILE_EVENTS
    uint32_t stop = profileCycleCount_get();
    if (pe<EventProfileMax && pe >= 0) {
        eventProfile.event[pe].stop = stop;
        eventProfile.event[pe].count++;
        eventProfile.event[pe].duration += (eventProfile.event[pe].stop-eventProfile.event[pe].start);
    }
#endif

}
inline void profileTimedEventReset(const ProfiledEventNames pe)
{
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax && pe >= 0) {
        eventProfile.event[pe].start = 0;
        eventProfile.event[pe].stop = 0;
        eventProfile.event[pe].count = 0;
        eventProfile.event[pe].duration = 0;
    }
#endif
}

inline  ProfilingTimedEvent* profileTimedEventGet(const ProfiledEventNames pe)
{
    ProfilingTimedEvent* pe_ptr = NULL;
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax && pe >= 0) {
        pe_ptr = &eventProfile.event[pe];
    }
#endif
    return pe_ptr;
}



#endif
