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

#ifndef __HW_SI5351A
#define __HW_SI5351A

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

void si5351aOutputOff(uint8_t clk);
void si5351aSetFrequency(uint32_t frequency);

#endif
