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

// Common
#include "mchf_board.h"

#include "ui_rotary.h"

extern 		DialFrequency 		df;
//extern		AudioGain			ag;

//*----------------------------------------------------------------------------
//* Function Name       : UiRotaryFreqEncoderInit
//* Object              : 
//* Input Parameters    : 
//* Output Parameters   : 
//* Functions called    : 
//*----------------------------------------------------------------------------
void UiRotaryFreqEncoderInit(void)
{
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

	TIM_TimeBaseStructure.TIM_Period = (FREQ_ENCODER_RANGE/FREQ_ENCODER_LOG_D) + (ENCODER_FLICKR_BAND*2);	// range + 3 + 3
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

	TIM_TimeBaseStructure.TIM_Period = (ENCODER_ONE_RANGE/ENCODER_ONE_LOG_D) + (ENCODER_FLICKR_BAND*2);	// range + 3 + 3
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

	TIM_TimeBaseStructure.TIM_Period = (ENCODER_TWO_RANGE/ENCODER_TWO_LOG_D) + (ENCODER_FLICKR_BAND*2);	// range + 3 + 3
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

	TIM_TimeBaseStructure.TIM_Period = (ENCODER_THR_RANGE/ENCODER_THR_LOG_D) + (ENCODER_FLICKR_BAND*2);	// range + 3 + 3
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_EncoderInterfaceConfig(TIM5,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);

	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM5, ENABLE);
}


