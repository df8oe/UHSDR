/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __UI_ROTARY_H
#define __UI_ROTARY_H

// Some encoders have detend and generate x
// ammount of pulses per click
#define USE_DETENTED_ENCODERS
#define USE_DETENTED_VALUE	4

// Protective band on top and bottom scale
// dependent on encoder quality and debounce
// capacitors
#define ENCODER_FLICKR_BAND	4

// Maximum pot value
#define FREQ_ENCODER_RANGE	0xFFF

// Divider to create non linearity
#define FREQ_ENCODER_LOG_D	1

// The UI checking function will skip
// reading too often based on this value
#define FREQ_UPDATE_SKIP	100

// Frequency public structure
typedef struct DialFrequency
{
	// pot values
	//
	ulong	value_old;			// previous value
	ulong	value_new;			// most current value

	// SI570 actual frequency
	ulong	tune_old;			// previous value
	ulong	tune_new;			// most current value

	// Current tuning step
	ulong	tuning_step;		// selected step by user
	ulong	selected_idx;		// id of step
//	ulong	last_tune_step;		// last tunning step used during dial rotation
	ulong	step_new;			// Eth driver req step

	ulong	update_skip;

	// Shift used on TX
	//int		tx_shift;

	// TCXO routine factor/flag
	int		temp_factor;
	uchar	temp_enabled;

	uchar	de_detent;			// sw de-detent flag

	// Virtual segments
	uchar	dial_100_mhz;
	uchar	dial_010_mhz;
	uchar	dial_001_mhz;
	uchar	dial_100_khz;
	uchar	dial_010_khz;
	uchar	dial_001_khz;
	uchar	dial_100_hz;
	uchar	dial_010_hz;
	uchar	dial_001_hz;

	// Second display
	uchar	sdial_100_mhz;
	uchar	sdial_010_mhz;
	uchar	sdial_001_mhz;
	uchar	sdial_100_khz;
	uchar	sdial_010_khz;
	uchar	sdial_001_khz;
	uchar	sdial_100_hz;
	uchar	sdial_010_hz;
	uchar	sdial_001_hz;
				
} DialFrequency;

// --------------------------------
// Maximum pot value
#define ENCODER_ONE_RANGE	0xFFF

// Divider to create non linearity
#define ENCODER_ONE_LOG_D	1

// Audio Gain public structure
typedef struct EncoderOneSelection
{
	// pot values
	//
	ulong	value_old;			// previous value
	ulong	value_new;			// most current value
	uchar	de_detent;			// sw de-detent flag

} EncoderOneSelection;

// --------------------------------
// Maximum pot value
#define ENCODER_TWO_RANGE	0xFFF

// Divider to create non linearity
#define ENCODER_TWO_LOG_D	1

// Audio Gain public structure
typedef struct EncoderTwoSelection
{
	// pot values
	//
	ulong	value_old;			// previous value
	ulong	value_new;			// most current value
	uchar	de_detent;			// sw de-detent flag

} EncoderTwoSelection;

// --------------------------------
// Maximum pot value
#define ENCODER_THR_RANGE	0xFFF

// Divider to create non linearity
#define ENCODER_THR_LOG_D	1

// FIR selector public structure
typedef struct EncoderThreeSelection
{
	// pot values
	//
	ulong	value_old;			// previous value
	ulong	value_new;			// most current value
	uchar	de_detent;			// sw de-detent flag

} EncoderThreeSelection;

void UiRotaryFreqEncoderInit(void);
void UiRotaryEncoderOneInit(void);
void UiRotaryEncoderTwoInit(void);
void UiRotaryEncoderThreeInit(void);

#endif
