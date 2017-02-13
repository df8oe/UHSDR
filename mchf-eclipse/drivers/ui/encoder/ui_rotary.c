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

// Common
#include "mchf_board.h"
#include "ui_driver.h"
#include "stdlib.h"
#include "ui_rotary.h"
#include "tim.h"
// ------------------------------------------------
// Encoder 1-4 Array
__IO EncoderSelection		encSel[4];

typedef struct
{
 GPIO_TypeDef* pio;
 uint16_t pin;
 uint8_t af;
} pin_t;


typedef struct  {
    TIM_TypeDef* tim;
    TIM_HandleTypeDef * htim;
    pin_t pin[2];

} encoder_io_t;

const encoder_io_t encoderIO[4] =
{
        {
                .tim = TIM3,
                .htim = &htim3,
                .pin =
                {
                    { ENC_ONE_CH1_PIO, ENC_ONE_CH1, GPIO_AF2_TIM3 } ,
                    { ENC_ONE_CH2_PIO, ENC_ONE_CH2, GPIO_AF2_TIM3 }
                }
        },
        {
                .tim = TIM4,
                .htim = &htim4,
                .pin =
                {
                    { ENC_TWO_CH1_PIO, ENC_TWO_CH1, GPIO_AF2_TIM4 } ,
                    { ENC_TWO_CH2_PIO, ENC_TWO_CH2, GPIO_AF2_TIM4 }
                }
        },
        {
                .tim = TIM5,
                .htim = &htim5,
                .pin =
                {
                    { ENC_THREE_CH1_PIO, ENC_THREE_CH1, GPIO_AF2_TIM5 } ,
                    { ENC_THREE_CH2_PIO, ENC_THREE_CH2, GPIO_AF2_TIM5 }
                }
        },
        {
                .tim = TIM8,
                .htim = &htim8,
                .pin =
                {
                    { FREQ_ENC_CH1_PIO, FREQ_ENC_CH1, GPIO_AF3_TIM8 } ,
                    { FREQ_ENC_CH2_PIO, FREQ_ENC_CH2, GPIO_AF3_TIM8 }
                }
        }
};

void UiRotaryEncoderInit(uint8_t id)
{
    const encoder_io_t* encIO = &encoderIO[id];
    // Init encoder one
    encSel[id].value_old      = 0;
    encSel[id].value_new          = ENCODER_RANGE;
    encSel[id].de_detent          = 0;
    encSel[id].tim                = encIO->tim;
    HAL_TIM_Encoder_Start(encIO->htim, TIM_CHANNEL_1 | TIM_CHANNEL_2);

#if 0
    GPIO_InitTypeDef            GPIO_InitStructure;

    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;

    for (uint32_t pinIdx = 0; pinIdx < 2; pinIdx++)
    {
        GPIO_InitStructure.Alternate = encIO->pin[pinIdx].af;
        GPIO_InitStructure.Pin = encIO->pin[pinIdx].pin;
        GPIO_Init(encIO->pin[pinIdx].pio, &GPIO_InitStructure);
    }


    TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = (ENCODER_RANGE/ENCODER_LOG_D) + (ENCODER_FLICKR_BAND*2); // range + 3 + 3
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_EncoderInterfaceConfig(encIO->tim,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);

    TIM_TimeBaseInit(encIO->tim, &TIM_TimeBaseStructure);
    TIM_Cmd(encIO->tim, ENABLE);
#endif
}


//*----------------------------------------------------------------------------
//* Function Name       : UiRotaryFreqEncoderInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiRotaryFreqEncoderInit(void)
{
    ///RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    UiRotaryEncoderInit(ENCFREQ);
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
    ///RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    UiRotaryEncoderInit(ENC1);
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
    ///RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    UiRotaryEncoderInit(ENC2);
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
    ///RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    UiRotaryEncoderInit(ENC3);
}


int UiDriverEncoderRead(const uint32_t encId)
{
    bool no_change = false;
    int pot_diff = 0;

    if (encId < ENC_MAX)
    {
        encSel[encId].value_new = encSel[encId].tim->CNT;
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
