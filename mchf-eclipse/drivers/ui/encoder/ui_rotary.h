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
**  Licence:		GNU GPLv3                                                      **
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

#endif
