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

#include "uhsdr_hw_i2c.h"
#include "ui_si5351a.h"

#ifdef USE_OSC_SI5351A
// Will be removed and made a dynamic config element
#define TEST_QUADRATURE

// reference oscillator xtal frequency
#define SI5351_XTAL_FREQ		27000000			// Crystal frequency
#define SI5351_MIN_PLL			405000000			// Min PLL frequency
#define SI5351_MAX_PLL		    900000000			// Max PLL frequency

// Register definitions
#define SI5351_CLK0_CONTROL		16
#define SI5351_CLK1_CONTROL		17
#define SI5351_CLK2_CONTROL		18
#define SI5351_SYNTH_PLL_A		26
#define SI5351_SYNTH_PLL_B		34
#define SI5351_SYNTH_MS_0		42
#define SI5351_SYNTH_MS_1		50
#define SI5351_SYNTH_MS_2		58

#define SI5351_CLK0_PHASE_OFFSET 165
#define SI5351_CLK1_PHASE_OFFSET 166
#define SI5351_CLK2_PHASE_OFFSET 167
#define SI5351_PLL_RESET		177

// R-division ratio definitions
#define SI5351_R_DIV_1			0b00000000
#define SI5351_R_DIV_2			0b00010000
#define SI5351_R_DIV_4			0b00100000
#define SI5351_R_DIV_8			0b00110000
#define SI5351_R_DIV_16			0b01000000
#define SI5351_R_DIV_32			0b01010000
#define SI5351_R_DIV_64			0b01100000
#define SI5351_R_DIV_128		0b01110000

#define SI5351_DIV_BY_4			0b00001100

#define SI5351_CLK_SRC_PLL_A	0b00000000
#define SI5351_CLK_SRC_PLL_B	0b00100000

// Commands
#define SI5351_OUTPUT_OFF		0x80
#define SI5351_OUTPUT_ON		0x4F			// see AN619 for details: this sets many options!
#define SI5351_PLLA_RESET		0x20
#define SI5351_PLLB_RESET		0x80


#define SI5351_I2C_WRITE 192						// I2C address for writing
#define SI5351_I2C_READ  193						// I2C address for reading


#define MAX_UINT20 1048575


static uint16_t Si5351a_WriteRegister(uint8_t reg, uint8_t val)
{
	return mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, reg, val);
}

static uint16_t Si5351a_WriteRegisters(uint8_t reg, uint8_t* val_p, uint32_t size)
{
	return 	MCHF_I2C_WriteBlock(SI5351A_I2C,SI5351_I2C_WRITE, reg, 1, val_p, size);
}


typedef struct
{
	uint32_t frequency;
	uint32_t pll_num;
	uint32_t pll_denom;
	uint32_t multisynth_divider;
	uint32_t pll_mult;
	bool phasedOutput;
} Si5351a_Config_t;

typedef struct
{
	bool is_present;
	Si5351a_Config_t current;
	Si5351a_Config_t next;
} Si5351a_State_t;

Si5351a_State_t si5351a_state;


// Set up PLL with mult, num and denom
// mult is 15...90
// num is 0...1048575
// denom is 0...1048575
static bool Si5351a_SetupPLL(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom)
{

	uint32_t P1;									// PLL config register P1
	uint32_t P2;									// PLL config register P2
	uint32_t P3;									// PLL config register P3


	uint32_t fract = 128.0 * ((float32_t)num / (float32_t)denom);

	P1 = 128 * (uint32_t)(mult) + fract - 512;
	P2 = 128 * num - denom * fract;
	P3 = denom;

	uint8_t pll_data[8] =
	{
			(P3 & 0x0000FF00) >> 8,
			P3 & 0x000000FF,
			(P1 & 0x00030000) >> 16,
			(P1 & 0x0000FF00) >> 8,
			P1 & 0x000000FF,
			((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16),
			(P2 & 0x0000FF00) >> 8,
			P2 & 0x000000FF
	};

	return Si5351a_WriteRegisters(pll, pll_data, 8) == HAL_OK;
}


// Set up MultiSynth with integer divider and R multisynth_divider
static bool Si5351a_SetupMultisynthInteger(uint8_t synth, uint32_t divider, uint8_t rDiv)
{
	uint32_t P1;									// Synth config register P1
	uint32_t P2;									// Synth config register P2
	uint32_t P3;									// Synth config register P3

	P1 = 128 * divider - 512;
	P2 = 0;											// P2 = 0, P3 = 1 forces an integer value for the multisynth_divider
	P3 = 1;

	uint8_t synth_data[8] =
	{
			(P3 & 0x0000FF00) >> 8,
			(P3 & 0x000000FF),
			((P1 & 0x00030000) >> 16) | rDiv | (divider == 4?SI5351_DIV_BY_4:0),
			(P1 & 0x0000FF00) >> 8,
			(P1 & 0x000000FF),
			((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16),
			(P2 & 0x0000FF00) >> 8,
			(P2 & 0x000000FF)
	};

	return Si5351a_WriteRegisters(synth, synth_data, 8) == HAL_OK;
}

static bool Si5351a_ValidateConfig(Si5351a_Config_t* config)
{
	bool retval =  (config->multisynth_divider == 4 || config->multisynth_divider == 6 || (config->multisynth_divider >= 8 && config->multisynth_divider <= 900 ));
	// TODO: Check rDiv to be power of 2 and in range 1..128

	if (retval == true)
	{
		retval = ((config->pll_mult >= 15 && config->pll_mult <= 90) && (config->pll_num <= MAX_UINT20) && (config->pll_denom <= MAX_UINT20));
	}

	return retval;
}

static bool Si5351a_CalculateConfigForDivider(Si5351a_Config_t* new_config, uint32_t divider)
{

	const uint32_t xtalFreq = SI5351_XTAL_FREQ;
	uint32_t pllFreq = divider * new_config->frequency;		// Calculate the pllFrequency: the multisynth_divider * desired output frequency


	uint32_t l = pllFreq % xtalFreq;							// It has three parts:
	float32_t f = l;											// mult is an integer that must be in the range 15...90
	f *= MAX_UINT20;								// num and denom are the fractional parts, the numerator and denominator
	f /= xtalFreq;									// each is 20 bits (range 0...1048575)

	new_config->pll_mult = pllFreq / xtalFreq;			// Determine the multiplier to get to the required pllFrequency
	new_config->pll_num = f;							// the actual multiplier is  pll_mult + pll_num / denom
	new_config->pll_denom = MAX_UINT20;					// For simplicity we set the denominator to the maximum 1048575
	new_config->multisynth_divider = divider;
	return Si5351a_ValidateConfig(new_config);
}

static bool Si5351a_CalculateConfig(uint32_t frequency, Si5351a_Config_t* new_config, Si5351a_Config_t* cur_config)
{
	bool retval = false;
	new_config->frequency = frequency;

	uint32_t divider_max = SI5351_MAX_PLL / frequency;			// Calculate the division ratio

	if ((new_config->phasedOutput == true || divider_max < 8) && divider_max % 2)
	{
		divider_max--;						// Ensure an even integer division ratio
	}

	uint32_t divider_min = SI5351_MIN_PLL / frequency;			// Calculate the division ratio
	if (SI5351_MIN_PLL % frequency)
	{
		divider_min++;
	}

	if ( (new_config->phasedOutput == true || divider_min < 8) && divider_min % 2)
	{
		divider_min++;						// Ensure an even integer division ratio
	}


	// In quadrature mode we cannot have dividers higher than 126
	const uint32_t divider_limit = new_config->phasedOutput?126:900;

	// reuse current divider if possible, theory is that we can get away without PLLRESET in this case
	// TODO: Validate theory and validate need for skipping PLLRESET
	if (cur_config->phasedOutput == new_config->phasedOutput && cur_config->multisynth_divider >= divider_min && cur_config->multisynth_divider <= divider_max)
	{
		retval = Si5351a_CalculateConfigForDivider(new_config, cur_config->multisynth_divider);
	}

	if ( retval == false && divider_max <= divider_limit)
	{
			retval = Si5351a_CalculateConfigForDivider(new_config, divider_max);
	}

	if (retval == false && divider_min >= 4)
	{
			retval = Si5351a_CalculateConfigForDivider(new_config, divider_min);
	}

	if (retval == false)
	{
		new_config->multisynth_divider = 0;
	}

	return retval;
}

static bool Si5351a_ApplyConfig(Si5351a_Config_t* config)
{

	// Set up PLL A with the calculated multiplication ratio
	bool result = Si5351a_SetupPLL(SI5351_SYNTH_PLL_A, config->pll_mult, config->pll_num, config->pll_denom);

	// Set up MultiSynth divider 0, with the calculated divider.
	// The final R division stage can divide by a power of two, from 1...128
	// represented by constants SI5351_R_DIV1 to SI5351_R_DIV128 (see si5351a.h header file)
	// If you want to output frequencies below 1MHz, you have to use the
	// final R division stage
	if (result == true)
	{
		result = Si5351a_SetupMultisynthInteger(SI5351_SYNTH_MS_0, config->multisynth_divider, SI5351_R_DIV_1);
		result |= Si5351a_SetupMultisynthInteger(SI5351_SYNTH_MS_1, config->multisynth_divider, SI5351_R_DIV_1);
	}

	// Reset the PLL. This causes a glitch in the output. For small changes to
	// the parameters, you don't need to reset the PLL, and there is no glitch
	// Finally switch on the CLK0 output (0x4F)
	// and set the MultiSynth0 input to be PLL A
	if (result == true)
	{
		Si5351a_WriteRegister(SI5351_CLK0_PHASE_OFFSET, 0);
		Si5351a_WriteRegister(SI5351_CLK1_PHASE_OFFSET, config->phasedOutput?config->multisynth_divider:0);

		Si5351a_WriteRegister( SI5351_CLK0_CONTROL, SI5351_OUTPUT_ON);
		Si5351a_WriteRegister( SI5351_CLK1_CONTROL, config->phasedOutput?SI5351_OUTPUT_ON:SI5351_OUTPUT_OFF);
		Si5351a_WriteRegister( SI5351_PLL_RESET, SI5351_PLLA_RESET);
	}

	return result;
}


bool Si5351a_IsPresent()
{
	return si5351a_state.is_present;
}
static void Si5351a_SetPPM(float32_t ppm)
{

}

static Oscillator_ResultCodes_t Si5351a_PrepareNextFrequency(uint32_t freq, int temp_factor)
{
#ifdef TEST_QUADRATURE
	si5351a_state.next.phasedOutput = freq > 3214286*TUNE_MULT;
	if (si5351a_state.next.phasedOutput == true)
	{
		freq /= TUNE_MULT;
	}
#endif
	return Si5351a_CalculateConfig(freq, &si5351a_state.next, &si5351a_state.current) == true?OSC_OK:OSC_TUNE_IMPOSSIBLE;
}

static Oscillator_ResultCodes_t Si5351a_ChangeToNextFrequency()
{
	Oscillator_ResultCodes_t retval = OSC_COMM_ERROR;
	if (Si5351a_ApplyConfig(&si5351a_state.next) == true)
	{
		memcpy(&si5351a_state.current, &si5351a_state.next, sizeof(si5351a_state.next));
		retval = OSC_OK;
	}
	return retval;
}

static bool              Si5351a_IsNextStepLarge()
{
	return false;
}

const OscillatorInterface_t osc_si5351a =
{
		.init = Si5351a_Init,
		.isPresent = Si5351a_IsPresent,
		.setPPM = Si5351a_SetPPM,
		.prepareNextFrequency = Si5351a_PrepareNextFrequency,
		.changeToNextFrequency = Si5351a_ChangeToNextFrequency,
		.isNextStepLarge = Si5351a_IsNextStepLarge
};

void Si5351a_Init()
{
	si5351a_state.current.frequency = 0;
	si5351a_state.next.frequency = 0;
	si5351a_state.current.multisynth_divider = 0;
	si5351a_state.next.multisynth_divider = 0;

	// TODO: We should move this to our I2C abstraction as general function
	si5351a_state.is_present = MCHF_I2C_DeviceReady(SI5351A_I2C,SI5351_I2C_WRITE) == HAL_OK;

	osc = Si5351a_IsPresent()?&osc_si5351a:NULL;
}
#endif
