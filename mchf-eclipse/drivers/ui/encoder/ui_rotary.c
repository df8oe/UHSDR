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

// Common
#include "mchf_board.h"
#include "stdlib.h"
#include "ui_rotary.h"

// ------------------------------------------------
// Encoder 1-4 Array
__IO EncoderSelection		encSel[4];

//*----------------------------------------------------------------------------
//* Function Name       : UiRotaryFreqEncoderInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiRotaryFreqEncoderInit(void)
{
    // Init encoder one
    encSel[ENCFREQ].value_old 		= 0;
    encSel[ENCFREQ].value_new			= ENCODER_RANGE;
    encSel[ENCFREQ].de_detent			= 0;
    encSel[ENCFREQ].tim 				= TIM8;

    TIM_TimeBaseInitTypeDef 	TIM_TimeBaseStructure;
    GPIO_InitTypeDef 			GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = FREQ_ENC_CH1;
    GPIO_Init(FREQ_ENC_CH1_PIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = FREQ_ENC_CH2;
    GPIO_Init(FREQ_ENC_CH2_PIO, &GPIO_InitStructure);

    GPIO_PinAFConfig(FREQ_ENC_CH1_PIO, FREQ_ENC_CH1_SOURCE, GPIO_AF_TIM8);
    GPIO_PinAFConfig(FREQ_ENC_CH2_PIO, FREQ_ENC_CH2_SOURCE, GPIO_AF_TIM8);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = (ENCODER_RANGE/ENCODER_LOG_D) + (ENCODER_FLICKR_BAND*2);	// range + 3 + 3
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_EncoderInterfaceConfig(TIM8,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);

    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM8, ENABLE);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiRotaryEncoderOneInit
//* Object              : audio gain
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiRotaryEncoderOneInit(void)
{
    // Init encoder one
    encSel[ENC1].value_old 		= 0;
    encSel[ENC1].value_new			= ENCODER_RANGE;
    encSel[ENC1].de_detent			= 0;
    encSel[ENC1].tim 				= TIM3;


    TIM_TimeBaseInitTypeDef 	TIM_TimeBaseStructure;
    GPIO_InitTypeDef 			GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = ENC_ONE_CH1;
    GPIO_Init(ENC_ONE_CH1_PIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ENC_ONE_CH2;
    GPIO_Init(ENC_ONE_CH2_PIO, &GPIO_InitStructure);

    GPIO_PinAFConfig(ENC_ONE_CH1_PIO, ENC_ONE_CH1_SOURCE, GPIO_AF_TIM3);
    GPIO_PinAFConfig(ENC_ONE_CH2_PIO, ENC_ONE_CH2_SOURCE, GPIO_AF_TIM3);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = (ENCODER_RANGE/ENCODER_LOG_D) + (ENCODER_FLICKR_BAND*2);	// range + 3 + 3
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_EncoderInterfaceConfig(TIM3,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM3, ENABLE);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiRotaryEncoderTwoInit
//* Object              : rfgain
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiRotaryEncoderTwoInit(void)
{
    // Init encoder two
    encSel[ENC2].value_old 		= 0;
    encSel[ENC2].value_new			= ENCODER_RANGE;
    encSel[ENC2].de_detent			= 0;
    encSel[ENC2].tim 				= TIM4;


    TIM_TimeBaseInitTypeDef 	TIM_TimeBaseStructure;
    GPIO_InitTypeDef 			GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = ENC_TWO_CH1;
    GPIO_Init(ENC_TWO_CH1_PIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ENC_TWO_CH2;
    GPIO_Init(ENC_TWO_CH2_PIO, &GPIO_InitStructure);

    GPIO_PinAFConfig(ENC_TWO_CH1_PIO, ENC_TWO_CH1_SOURCE, GPIO_AF_TIM4);
    GPIO_PinAFConfig(ENC_TWO_CH2_PIO, ENC_TWO_CH2_SOURCE, GPIO_AF_TIM4);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = (ENCODER_RANGE/ENCODER_LOG_D) + (ENCODER_FLICKR_BAND*2);	// range + 3 + 3
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_EncoderInterfaceConfig(TIM4,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM4, ENABLE);


}

//*----------------------------------------------------------------------------
//* Function Name       : UiRotaryEncoderThreeInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiRotaryEncoderThreeInit(void)
{
    // Init encoder three
    encSel[ENC3].value_old 		= 0;
    encSel[ENC3].value_new			= ENCODER_RANGE;
    encSel[ENC3].de_detent			= 0;
    encSel[ENC3].tim 				= TIM5;

    TIM_TimeBaseInitTypeDef 	TIM_TimeBaseStructure;
    GPIO_InitTypeDef 			GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = ENC_THREE_CH1;
    GPIO_Init(ENC_THREE_CH1_PIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ENC_THREE_CH2;
    GPIO_Init(ENC_THREE_CH2_PIO, &GPIO_InitStructure);

    GPIO_PinAFConfig(ENC_THREE_CH1_PIO, ENC_THREE_CH1_SOURCE, GPIO_AF_TIM5);
    GPIO_PinAFConfig(ENC_THREE_CH2_PIO, ENC_THREE_CH2_SOURCE, GPIO_AF_TIM5);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = (ENCODER_RANGE/ENCODER_LOG_D) + (ENCODER_FLICKR_BAND*2);	// range + 3 + 3
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_EncoderInterfaceConfig(TIM5,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);

    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM5, ENABLE);
}


int UiDriverEncoderRead(const uint32_t encId)
{
    bool no_change = false;
    int pot_diff = 0;

    if (encId < ENC_MAX)
    {
        encSel[encId].value_new = TIM_GetCounter(encSel[encId].tim);
        // Ignore lower value flickr
        if (encSel[encId].value_new < ENCODER_FLICKR_BAND)
        {
            no_change = true;
        }
        else if (encSel[encId].value_new >
                 (ENCODER_RANGE / ENCODER_LOG_D) + ENCODER_FLICKR_BAND)
        {
            no_change = true;
        }
        else if (encSel[encId].value_old == encSel[encId].value_new)
        {
            no_change = true;
        }

        // SW de-detent routine

        if (no_change == false)
        {
            encSel[encId].de_detent+=abs(encSel[encId].value_new - encSel[encId].value_old);// corrected detent behaviour
            // double counts are now processed - not count lost!
            if (encSel[encId].de_detent < USE_DETENTED_VALUE)
            {
                encSel[encId].value_old = encSel[encId].value_new;  // update and skip
                no_change = true;
            }
            else
            {
                encSel[encId].de_detent = 0;
            }
        }
        // Encoder value to difference
        if (no_change == false)
        {
            pot_diff = encSel[encId].value_new - encSel[encId].value_old;
            encSel[encId].value_old = encSel[encId].value_new;
        }
    }
    return pot_diff;
}
