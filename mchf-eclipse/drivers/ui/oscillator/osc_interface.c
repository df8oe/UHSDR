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
#include "osc_ducddc_df8oe.h"
#include "osc_SParkle.h"
#include "soft_tcxo.h"
// -------------------------------------------------------------------------------------
// Local Oscillator
// ------------------
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

static uint32_t OscDummy_getMinFrequency(void)
{
    return 1; // 1 Hz
}

static uint32_t OscDummy_getMaxFrequency(void)
{
    return UINT32_MAX-1;
}



static bool OscDummy_Init(void)
{
	return true;
}

static const OscillatorInterface_t osc_dummy;

static const OscillatorInterface_t* oscillators[] =
{
#ifdef USE_OSC_DUCDDC
        &osc_ducddc,
#endif
#ifdef USE_OSC_SI5351A
        &osc_si5351a,
#endif
#ifdef USE_OSC_SI570
        &osc_si570,
#endif
#ifdef USE_OSC_SParkle
        &osc_SParkle_DDC,
#endif
        &osc_dummy
};

void Osc_Init()
{
    for (int osc_idx = 0; osc_idx < sizeof(oscillators)/sizeof(OscillatorInterface_t*); osc_idx++)
    {
        if (oscillators[osc_idx]->init())
        {
            osc = oscillators[osc_idx];
            break;
        }
    }

	SoftTcxo_Init();
}

static Oscillator_Type_t OscDummy_Type(void)
{
    return OSC_DUMMY;
}

static const OscillatorInterface_t osc_dummy =
{
        .init = OscDummy_Init,
        .isPresent = OscDummy_IsPresent,
        .setPPM = OscDummy_SetPPM,
        .prepareNextFrequency = OscDummy_PrepareNextFrequency,
        .changeToNextFrequency = OscDummy_ChangeToNextFrequency,
        .isNextStepLarge = OscDummy_IsNextStepLarge,
        .readyForIrqCall = OscDummy_ReadyForIrqCall,
        .name = "Dummy",
        .type = OscDummy_Type,
        .getMinFrequency = OscDummy_getMinFrequency,
        .getMaxFrequency = OscDummy_getMaxFrequency,
};
