/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
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
**  Licence:		CC BY-NC-SA 3.0                                                **
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

// Frequency public structure
typedef struct DialFrequency
{
    // pot values
    //
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
    bool    temp_factor_changed;
    uchar	temp_enabled;

    // Virtual segments
    uint8_t	dial_digits[9];
    // Second display
    uint8_t	sdial_digits[9];

} DialFrequency;

// --------------------------------
// Maximum pot value
#define ENCODER_RANGE	0xFFF

// Divider to create non linearity
#define ENCODER_LOG_D	1

// Audio Gain public structure
typedef struct EncoderSelection
{
    // pot values
    //
    ulong	value_old;			// previous value
    ulong	value_new;			// most current value
    uchar	de_detent;			// sw de-detent flag
    TIM_TypeDef* tim;

} EncoderSelection;


void UiRotaryFreqEncoderInit(void);

void UiRotaryEncoderOneInit(void);
void UiRotaryEncoderTwoInit(void);
void UiRotaryEncoderThreeInit(void);
enum EncoderId
{
    ENC1 = 0,
    ENC2,
    ENC3,
    ENCFREQ,
    ENC_MAX // this needs to be the last entry
};

int UiDriverEncoderRead(const uint32_t encId);

// ------------------------------------------------
// Frequency public
extern __IO DialFrequency 				df;


#endif
