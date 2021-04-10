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

#define DUCDDC_MIN_FREQ			10000L			// Min frequency
#define DUCDDC_MAX_FREQ		    4200000000L	    // Max frequency

#define DUCDDC_I2C_WRITE 0xD2
#define DUCDDC_I2C SI570_I2C

typedef struct
{
	uint32_t rx_frequency;
	uint32_t tx_frequency;
	uint8_t sr;
    uint8_t txp;
} __attribute__ ((packed)) DucDdc_Df8oe_Config_t;

typedef struct
{
    float64_t corr_mult;
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


typedef struct
{
    uint16_t format;
    uint16_t len;
    uint16_t hwtype;
    uint16_t hwver;
    uint8_t name[8];
    uint8_t unused[16];
} __attribute__ ((packed)) DucDdc_Df8oe_Rom_t;

static DucDdc_Df8oe_Rom_t osc_rom;
uint16_t hal_res;

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

    ducddc_state.corr_mult = (float64_t)1.0 + ((float64_t)ppm / 1000000.0);
}

static bool DucDdc_Df8oe_CalculateConfig(uint32_t frequency, DucDdc_Df8oe_Config_t* new_config)
{
    bool retval = false;
    if (frequency<=DucDdc_Df8oe_getMaxFrequency() && frequency>= DucDdc_Df8oe_getMinFrequency())
    {
        // calculated frequency adjusted by ppm multiplier
        uint32_t corr_freq = (float64_t)frequency * ducddc_state.corr_mult;

        new_config->rx_frequency = new_config->tx_frequency = __bswap32(corr_freq);
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
    ducddc_state.corr_mult = 1.0;
	ducddc_state.current.rx_frequency = 0;
	ducddc_state.current.tx_frequency = 0;
	ducddc_state.current.txp = 0xff;
	ducddc_state.current.sr = IQ_SAMPLE_RATE == 192000? 2 : (IQ_SAMPLE_RATE ==  96000 ? 1 : 0);

	ducddc_state.next.rx_frequency = 0;
	ducddc_state.next.tx_frequency = 0;
	ducddc_state.next.txp = 0xff;
	ducddc_state.next.sr = IQ_SAMPLE_RATE == 192000? 2 : (IQ_SAMPLE_RATE ==  96000 ? 1 : 0);


	if (UhsdrHw_I2C_DeviceReady(DUCDDC_I2C, DUCDDC_I2C_WRITE) == HAL_OK)
	{
	    ducddc_state.is_present = true;
	    uint16_t hal_res = HAL_I2C_Master_Receive(DUCDDC_I2C, DUCDDC_I2C_WRITE, (uint8_t*)&osc_rom, sizeof(osc_rom), 100);

	    // older gateware does not implement I2C read access
	    if (hal_res != HAL_OK)
	    {
	        osc_rom.hwtype = 0xff; // indicates unidentifiable board, defaults to DDCDUC
	        memcpy(osc_rom.name, "UNKNOWN",sizeof(osc_rom.name));
	    }
	}


	return DucDdc_Df8oe_IsPresent();
}

Oscillator_Type_t DucDdc_Df80e_Type(void)
{
    return osc_rom.hwtype != 0x00? OSC_DUCDDC_DF8OE: OSC_DUCDDC_DDC2MODULE;
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
        .type = DucDdc_Df80e_Type,
        .getMinFrequency = DucDdc_Df8oe_getMinFrequency,
        .getMaxFrequency = DucDdc_Df8oe_getMaxFrequency,
};


bool DucDdc_Df8oe_TxCWOnOff(bool on)
{
    ducddc_state.next.sr = (ducddc_state.next.sr & 0x7f) | (on?0x80:0x00);
    return DucDdc_Df8oe_ChangeToNextFrequency() == OSC_OK;
}

bool DucDdc_Df8oe_EnableTx(void)
{
    // ducddc_state.next.txp = 0xff;
    // return DucDdc_Df8oe_ChangeToNextFrequency() == OSC_OK;
    return true;
}
bool DucDdc_Df8oe_EnableRx(void)
{
    return true;
    // ducddc_state.next.txp = 0xff;
    // return DucDdc_Df8oe_ChangeToNextFrequency() == OSC_OK;
}

bool DucDdc_Df8oe_PrepareTx(void)
{
    return true;
}
bool DucDdc_Df8oe_PrepareRx(void)
{
    return true;
}

bool DucDdc_Df8oe_Config(void)
{
    return true;
}

#endif
