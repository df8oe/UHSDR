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
#include "uhsdr_board.h"

#include <math.h>
#include <osc_ducddc_df8oe.h>

#include "uhsdr_hw_i2c.h"

#ifdef USE_OSC_DUCDDC

#define DUCDDC_MIN_FREQ			100000L			// Min frequency
#define DUCDDC_MAX_FREQ		    4200000000L	    // Max frequency

#define DUCDDC_I2C_WRITE 0xD2
#define DUCDDC_I2C SI570_I2C

typedef struct
{
	uint32_t rx_frequency;
	uint32_t tx_frequency;
	uint8_t txp;
	uint8_t sr;
} __attribute__ ((packed)) DucDdc_Df8oe_Config_t;

typedef struct
{
	bool is_present;
	DucDdc_Df8oe_Config_t current;
	DucDdc_Df8oe_Config_t next;
} DucDdc_Df8oe_State_t;

DucDdc_Df8oe_State_t ducddc_state;


static uint32_t DucDdc_Df8oe_getMinFrequency()
{
    return DUCDDC_MIN_FREQ;
}

static uint32_t DucDdc_Df8oe_getMaxFrequency()
{
    return DUCDDC_MAX_FREQ;
}

static bool DucDdc_Df8oe_ApplyConfig(DucDdc_Df8oe_Config_t* config)
{
    // write 10 bytes to I2C
    return HAL_I2C_Master_Transmit(DUCDDC_I2C, DUCDDC_I2C_WRITE, (uint8_t*)config, sizeof(*config), 100) == HAL_OK;
}


bool DucDdc_Df8oe_IsPresent()
{
	return ducddc_state.is_present;
}
static void DucDdc_Df8oe_SetPPM(float32_t ppm)
{
	// ducddc_state.xtal_freq = ((float64_t)DUCDDC_XTAL_FREQ) + (((float64_t)DUCDDC_XTAL_FREQ)*(float64_t)ppm/1000000.0);
}

static bool DucDdc_Df8oe_CalculateConfig(uint32_t frequency, DucDdc_Df8oe_Config_t* new_config)
{
    bool retval = false;
    if (frequency<=DucDdc_Df8oe_getMaxFrequency() && frequency>= DucDdc_Df8oe_getMinFrequency())
    {
        new_config->rx_frequency = new_config->tx_frequency = __bswap32(frequency);
        // we need to store in big endian (i.e. the most significant byte first
        // we compile with little endian
        retval = true;
    }
    return retval;
}
static Oscillator_ResultCodes_t DucDdc_Df8oe_PrepareNextFrequency(uint32_t freq, int temp_factor)
{
    UNUSED(temp_factor);
	return DucDdc_Df8oe_CalculateConfig(freq, &ducddc_state.next) == true?OSC_OK:OSC_TUNE_IMPOSSIBLE;
}

static Oscillator_ResultCodes_t DucDdc_Df8oe_ChangeToNextFrequency()
{
	Oscillator_ResultCodes_t retval = OSC_COMM_ERROR;
	if (DucDdc_Df8oe_ApplyConfig(&ducddc_state.next) == true)
	{
		memcpy(&ducddc_state.current, &ducddc_state.next, sizeof(ducddc_state.next));
		retval = OSC_OK;
	}
	return retval;
}

static bool              DucDdc_Df8oe_IsNextStepLarge()
{
	return false;
}

/**
 * @brief Checks if all oscillator resources are available for switching frequency
 * It basically checks if the I2C is currently in use
 * This function must be called before changing the oscillator in interrupts
 * otherwise deadlocks may happen
 * @return true if it is safe to call oscillator functions in an interrupt
 */
bool DucDdc_Df8oe_ReadyForIrqCall()
{
    return (DUCDDC_I2C->Lock == HAL_UNLOCKED);
}

static bool DucDdc_Df8oe_Init()
{
	ducddc_state.current.rx_frequency = 0;
	ducddc_state.current.tx_frequency = 0;
	ducddc_state.current.txp = 0;
	ducddc_state.current.sr = 0;

	ducddc_state.next.rx_frequency = 0;
	ducddc_state.next.tx_frequency = 0;
    ducddc_state.next.txp = 0;
    ducddc_state.next.sr = 0;


	ducddc_state.is_present = UhsdrHw_I2C_DeviceReady(DUCDDC_I2C,DUCDDC_I2C_WRITE) == HAL_OK;

	return DucDdc_Df8oe_IsPresent();
}

const OscillatorInterface_t osc_ducddc =
{
        .init = DucDdc_Df8oe_Init,
        .isPresent = DucDdc_Df8oe_IsPresent,
        .setPPM = DucDdc_Df8oe_SetPPM,
        .prepareNextFrequency = DucDdc_Df8oe_PrepareNextFrequency,
        .changeToNextFrequency = DucDdc_Df8oe_ChangeToNextFrequency,
        .isNextStepLarge = DucDdc_Df8oe_IsNextStepLarge,
        .readyForIrqCall = DucDdc_Df8oe_ReadyForIrqCall,
        .name = "DUCDDC_DF8OE",
        .type = OSC_DUCDDC_DF8OE,
        .getMinFrequency = DucDdc_Df8oe_getMinFrequency,
        .getMaxFrequency = DucDdc_Df8oe_getMaxFrequency,
};

bool DucDdc_Df8oe_EnableTx(void)
{
    ducddc_state.next.txp |= 0xD0;
    return DucDdc_Df8oe_ChangeToNextFrequency() == OSC_OK;
}
bool DucDdc_Df8oe_EnableRx(void)
{
    ducddc_state.next.txp &= ~0xD0;
    return DucDdc_Df8oe_ChangeToNextFrequency() == OSC_OK;
}

bool DucDdc_Df8oe_PrepareTx(void)
{
    return true;
}
bool DucDdc_Df8oe_PrepareRx(void)
{
    return true;
}


#endif
