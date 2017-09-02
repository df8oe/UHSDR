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

#define SI5351_I2C_WRITE 192						// I2C address for writing
#define SI5351_I2C_READ  193						// I2C address for reading



// Set up PLL with mult, num and denom
// mult is 15...90
// num is 0...1048575
// denom is 0...1048575

void setupPLL(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom)
{
	uint32_t P1;									// PLL config register P1
	uint32_t P2;									// PLL config register P2
	uint32_t P3;									// PLL config register P3

	P1 = (uint32_t)(128 * ((float)num / (float)denom));
	P1 = (uint32_t)(128 * (uint32_t)(mult) + P1 - 512);
	P2 = (uint32_t)(128 * ((float)num / (float)denom));
	P2 = (uint32_t)(128 * num - denom * P2);
	P3 = denom;

	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, pll+0, (P3 & 0x0000FF00) >> 8);
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, pll+1, (P3 & 0x000000FF));
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, pll+2, (P1 & 0x00030000) >> 16);
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, pll+3, (P1 & 0x0000FF00) >> 8);
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, pll+4, (P1 & 0x000000FF));
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, pll+5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, pll+6, (P2 & 0x0000FF00) >> 8);
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, pll+7, (P2 & 0x000000FF));
}


// Set up MultiSynth with integer divider and R divider

void setupMultisynth(uint8_t synth, uint32_t divider, uint8_t rDiv)
{
	uint32_t P1;									// Synth config register P1
	uint32_t P2;									// Synth config register P2
	uint32_t P3;									// Synth config register P3

	P1 = 128 * divider - 512;
	P2 = 0;											// P2 = 0, P3 = 1 forces an integer value for the divider
	P3 = 1;

	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, synth+0, (P3 & 0x0000FF00) >> 8);
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, synth+1, (P3 & 0x000000FF));
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, synth+2, ((P1 & 0x00030000) >> 16) | rDiv);
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, synth+3, (P1 & 0x0000FF00) >> 8);
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, synth+4, (P1 & 0x000000FF));
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, synth+5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, synth+6, (P2 & 0x0000FF00) >> 8);
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, synth+7, (P2 & 0x000000FF));
}


// Switches off Si5351a output
// Example: si5351aOutputOff(SI5351_CLK0_CONTROL);
// will switch off output CLK0

void si5351aOutputOff(uint8_t clk)
{
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, clk, SI5351_OUTPUT_OFF);
}


// Set CLK0 output ON and to the specified frequency
// Frequency is in the range 1MHz to 150MHz
// Example: si5351aSetFrequency(10000000);
// will set output CLK0 to 10MHz
//
// This example sets up PLL A
// and MultiSynth 0
// and produces the output on CLK0

void si5351aSetFrequency(uint32_t frequency)
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
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, SI5351_PLL_RESET, SI5351_PLLA_RESET);
													// Finally switch on the CLK0 output (0x4F)
													// and set the MultiSynth0 input to be PLL A
	mchf_hw_i2c1_WriteRegister(SI5351_I2C_WRITE, SI5351_CLK0_CONTROL, SI5351_OUTPUT_ON | SI5351_CLK_SRC_PLL_A);
}
