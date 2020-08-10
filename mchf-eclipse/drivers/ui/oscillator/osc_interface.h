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

#ifndef __OSC_INTERFACE
#define __OSC_INTERFACE

#include "uhsdr_types.h"

typedef enum
{
    OSC_OK = 0, // tuning ok
    OSC_TUNE_LIMITED, // tuning to freq close to desired freq, still ok
    OSC_TUNE_IMPOSSIBLE, // did not tune, tune freq unknown
    OSC_COMM_ERROR, // could not talk to Si570, tune freq unknown
    OSC_ERROR_VERIFY, // register do not match, tune freq unknown
    OSC_LARGE_STEP, // did not tune, just checking

} Oscillator_ResultCodes_t;

typedef enum
{
    OSC_UNKNOWN,
    OSC_DUMMY,
    OSC_SI570,
    OSC_SI5351A,
} Oscillator_Type_t;

typedef struct
{
	// startup handling
	void  (*init)(void);

	// presence information
	bool  (*isPresent)(void);
	// startup/runtime reconfiguration
	void  (*setPPM)(float32_t ppm);

	// normal operations interface
	// freq is given in Hz, is QSD mixing frequency,
	// internally multiplied by 4 for Johnson Counter clock counters if needed by circuit
	Oscillator_ResultCodes_t (*prepareNextFrequency)(ulong freq, int temp_factor);
	Oscillator_ResultCodes_t (*changeToNextFrequency)(void);
	bool 			  (*isNextStepLarge)(void);
	bool              (*readyForIrqCall)(void);
	uint32_t    (*getMinFrequency)(void);
	uint32_t    (*getMaxFrequency)(void);
    const char*  name;
    const Oscillator_Type_t  type;

} OscillatorInterface_t;

extern const OscillatorInterface_t *osc;

void Osc_Init(void);
#endif
