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

// Common
#include "osc_interface.h"
#include "uhsdr_board.h"
#include "osc_si570.h"
#include "osc_si5351a.h"
#include "soft_tcxo.h"
// -------------------------------------------------------------------------------------
// Local Oscillator
// ------------------
static void OscDummy_Init();

const OscillatorInterface_t* osc;

static bool OscDummy_IsPresent()
{
	return false;
}
static void OscDummy_SetPPM(float32_t ppm)
{

}

static Oscillator_ResultCodes_t OscDummy_PrepareNextFrequency(ulong freq, int temp_factor)
{
	return OSC_OK;
}
static Oscillator_ResultCodes_t OscDummy_ChangeToNextFrequency()
{
	return OSC_OK;
}
static bool              OscDummy_IsNextStepLarge()
{
	return false;
}
/**
 * @brief Checks if all oscillator resources are available for switching frequency
 * This function must be called before changing the oscillator in interrupts
 * otherwise deadlocks may happen
 * @return true if it is safe to call oscillator functions in an interrupt
 */
static bool              OscDummy_ReadyForIrqCall()
{
    return true;
}

static uint32_t OscDummy_getMinFrequency()
{
    return 1; // 1 Hz
}

static uint32_t OscDummy_getMaxFrequency()
{
    return UINT32_MAX-1;
}


const OscillatorInterface_t osc_dummy =
{
		.init = OscDummy_Init,
		.isPresent = OscDummy_IsPresent,
		.setPPM = OscDummy_SetPPM,
		.prepareNextFrequency = OscDummy_PrepareNextFrequency,
		.changeToNextFrequency = OscDummy_ChangeToNextFrequency,
		.isNextStepLarge = OscDummy_IsNextStepLarge,
		.readyForIrqCall = OscDummy_ReadyForIrqCall,
        .name = "Dummy",
        .type = OSC_DUMMY,
        .getMinFrequency = OscDummy_getMinFrequency,
		.getMaxFrequency = OscDummy_getMaxFrequency,
};

static void OscDummy_Init()
{
	osc = &osc_dummy;
}

void Osc_Init()
{
#ifdef USE_OSC_SI5351A
	if (osc == NULL)
	{
		Si5351a_Init();
	}
#endif
#ifdef USE_OSC_SI570
	if (osc == NULL)
	{
		Si570_Init();
	}
#endif
	if (osc == NULL)
	{
		OscDummy_Init();
	}

	SoftTcxo_Init();
}
