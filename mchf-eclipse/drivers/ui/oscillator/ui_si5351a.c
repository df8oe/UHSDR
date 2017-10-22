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

// reference oscillator xtal frequency
#define SI5351_XTAL_FREQ		27000000			// Crystal frequency
#define SI5351_MIN_PLL			600000000			// Min PLL frequency
#define SI5351_MAX_PLL			900000000			// Max PLL frequency

// Register definitions
#define SI5351_CLK0_CONTROL		16
#define SI5351_CLK1_CONTROL		17
#define SI5351_CLK2_CONTROL		18
#define SI5351_SYNTH_PLL_A		26
#define SI5351_SYNTH_PLL_B		34
#define SI5351_SYNTH_MS_0		42
#define SI5351_SYNTH_MS_1		50
#define SI5351_SYNTH_MS_2		58
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

#define SI5351_CLK_SRC_PLL_A	0b00000000
#define SI5351_CLK_SRC_PLL_B	0b00100000

// Commands
#define SI5351_OUTPUT_OFF		0x80
#define SI5351_OUTPUT_ON		0x4F			// see AN619 for details: this sets many options!
#define SI5351_PLLA_RESET		0x20
#define SI5351_PLLB_RESET		0x80


#define SI5351_I2C_WRITE 192						// I2C address for writing
#define SI5351_I2C_READ  193						// I2C address for reading




static uint16_t Si5351a_WriteRegister(uint8_t reg, uint8_t val)
{
	return mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, reg, val);
}

static uint16_t Si5351a_WriteRegisters(uint8_t reg, uint8_t* val_p, uint32_t size)
{
	return 	MCHF_I2C_WriteBlock(SI5351A_I2C,SI5351_I2C_WRITE, reg, 1, val_p, size);
}

// The following 4 functions have been derived from
//
// Author: Hans Summers, 2015
// Website: http://www.hanssummers.com
//
// A very very simple Si5351a demonstration
// using the Si5351a module kit http://www.hanssummers.com/synth
// Please also refer to SiLabs AN619 which describes all the registers to use

// Set up PLL with mult, num and denom
// mult is 15...90
// num is 0...1048575
// denom is 0...1048575
static void setupPLL(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom)
{
	uint32_t P1;									// PLL config register P1
	uint32_t P2;									// PLL config register P2
	uint32_t P3;									// PLL config register P3

	P1 = (uint32_t)(128 * ((float)num / (float)denom));
	P1 = (uint32_t)(128 * (uint32_t)(mult) + P1 - 512);
	P2 = (uint32_t)(128 * ((float)num / (float)denom));
	P2 = (uint32_t)(128 * num - denom * P2);
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

	Si5351a_WriteRegisters(pll, pll_data, 8);
}


// Set up MultiSynth with integer divider and R divider

static void setupMultisynth(uint8_t synth, uint32_t divider, uint8_t rDiv)
{
	uint32_t P1;									// Synth config register P1
	uint32_t P2;									// Synth config register P2
	uint32_t P3;									// Synth config register P3

	P1 = 128 * divider - 512;
	P2 = 0;											// P2 = 0, P3 = 1 forces an integer value for the divider
	P3 = 1;

	uint8_t synth_data[8] =
	{
			(P3 & 0x0000FF00) >> 8,
			(P3 & 0x000000FF),
			((P1 & 0x00030000) >> 16) | rDiv,
			(P1 & 0x0000FF00) >> 8,
			(P1 & 0x000000FF),
			((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16),
			(P2 & 0x0000FF00) >> 8,
			(P2 & 0x000000FF)
	};

	Si5351a_WriteRegisters(synth, synth_data, 8);

}


#if 0
// Switches off Si5351a output
// Example: si5351aOutputOff(SI5351_CLK0_CONTROL);
// will switch off output CLK0

static void si5351aOutputOff(uint8_t clk)
{
	Si5351a_WriteRegister( clk, SI5351_OUTPUT_OFF);
}
#endif

// Set CLK0 output ON and to the specified frequency
// Frequency is in the range 1MHz to 150MHz
// Example: si5351aSetFrequency(10000000);
// will set output CLK0 to 10MHz
//
// This example sets up PLL A
// and MultiSynth 0
// and produces the output on CLK0
static void si5351aSetFrequency(uint32_t frequency)
{
	uint32_t pllFreq;
	uint32_t xtalFreq = SI5351_XTAL_FREQ;
	uint32_t l;
	float f;
	uint8_t mult;
	uint32_t num;
	uint32_t denom;
	uint32_t divider;

	divider = SI5351_MAX_PLL / frequency;			// Calculate the division ratio
	if (divider % 2) divider--;						// Ensure an even integer division ratio

	pllFreq = divider * frequency;					// Calculate the pllFrequency: the divider * desired output frequency

	mult = pllFreq / xtalFreq;						// Determine the multiplier to get to the required pllFrequency
	l = pllFreq % xtalFreq;							// It has three parts:
	f = l;											// mult is an integer that must be in the range 15...90
	f *= 1048575;									// num and denom are the fractional parts, the numerator and denominator
	f /= xtalFreq;									// each is 20 bits (range 0...1048575)
	num = f;										// the actual multiplier is  mult + num / denom
	denom = 1048575;								// For simplicity we set the denominator to the maximum 1048575

	// Set up PLL A with the calculated multiplication ratio
	setupPLL(SI5351_SYNTH_PLL_A, mult, num, denom);
	// Set up MultiSynth divider 0, with the calculated divider.
	// The final R division stage can divide by a power of two, from 1...128
	// reprented by constants SI5351_R_DIV1 to SI5351_R_DIV128 (see si5351a.h header file)
	// If you want to output frequencies below 1MHz, you have to use the
	// final R division stage
	setupMultisynth(SI5351_SYNTH_MS_0, divider, SI5351_R_DIV_1);
	// Reset the PLL. This causes a glitch in the output. For small changes to
	// the parameters, you don't need to reset the PLL, and there is no glitch
	Si5351a_WriteRegister( SI5351_PLL_RESET, SI5351_PLLA_RESET);
	// Finally switch on the CLK0 output (0x4F)
	// and set the MultiSynth0 input to be PLL A
	Si5351a_WriteRegister( SI5351_CLK0_CONTROL, SI5351_OUTPUT_ON | SI5351_CLK_SRC_PLL_A);
}
// Reused Code End

typedef struct
{
	bool is_present;
	uint32_t frequency;
} Si5351a_State_t;

static Si5351a_State_t si5351a_state;


bool Si5351a_IsPresent()
{
	return si5351a_state.is_present;
}
static void Si5351a_SetPPM(float32_t ppm)
{

}

static Oscillator_ResultCodes_t Si5351a_PrepareNextFrequency(uint32_t freq, int temp_factor)
{
	si5351a_state.frequency = freq;
	return OSC_OK;
}
static Oscillator_ResultCodes_t Si5351a_ChangeToNextFrequency()
{
	si5351aSetFrequency(si5351a_state.frequency);
	return OSC_OK;
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
	si5351a_state.is_present = true;
	// TODO: We should move this to our I2C abstraction as general function
	si5351a_state.is_present = MCHF_I2C_DeviceReady(SI5351A_I2C,SI5351_I2C_WRITE) == HAL_OK;

	osc = Si5351a_IsPresent()?&osc_si5351a:NULL;
}
