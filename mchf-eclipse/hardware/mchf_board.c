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

#include "mchf_board.h"
#include "ui_configuration.h"
#include "ui_lcd_hy28.h"
#include <stdio.h>

#include "mchf_hw_i2c.h"
#include "mchf_hw_i2c2.h"

#include "ui_rotary.h"
#include "ui_lcd_hy28.h"
//
#include "ui_driver.h"
//
#include "codec.h"
//
#include "ui_si570.h"
//
// Eeprom items
#include "eeprom.h"
extern uint16_t VirtAddVarTab[NB_OF_VAR];

// Transceiver state public structure
extern __IO TransceiverState ts;

//
__IO	FilterCoeffs		fc;

// ------------------------------------------------
// Frequency public
__IO DialFrequency 				df;

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_led_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void mchf_board_led_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = GREEN_LED;
	GPIO_Init(GREEN_LED_PIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = RED_LED;
	GPIO_Init(RED_LED_PIO, &GPIO_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_debug_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void mchf_board_debug_init(void)
{
#ifdef DEBUG_BUILD

	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	USART_InitStructure.USART_BaudRate = 921600;//230400;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;

	// Enable UART clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// Connect PXx to USARTx_Tx
	GPIO_PinAFConfig(DEBUG_PRINT_PIO, DEBUG_PRINT_SOURCE, GPIO_AF_USART1);

	// Configure USART Tx as alternate function
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;

	GPIO_InitStructure.GPIO_Pin 	= DEBUG_PRINT;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_PRINT_PIO, &GPIO_InitStructure);

	// USART configuration
	USART_Init(USART1, &USART_InitStructure);

	// Enable USART
	USART_Cmd(USART1, ENABLE);

	// Wait tx ready
	while (USART_GetFlagStatus(DEBUG_COM, USART_FLAG_TC) == RESET);

#endif
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_keypad_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
// -------------------------------------------------------
// Constant declaration of the buttons map across ports
// - update if moving buttons around !!!
const ButtonMap bm[18] =
{
        {BUTTON_M2_PIO,     BUTTON_M2},     // 0
        {BUTTON_G2_PIO,     BUTTON_G2},     // 1
        {BUTTON_G3_PIO,     BUTTON_G3},     // 2
        {BUTTON_BNDM_PIO,   BUTTON_BNDM},   // 3
        {BUTTON_G4_PIO,     BUTTON_G4},     // 4
        {BUTTON_M3_PIO,     BUTTON_M3},     // 5
        {BUTTON_STEPM_PIO,  BUTTON_STEPM},  // 6
        {BUTTON_STEPP_PIO,  BUTTON_STEPP},  // 7
        {BUTTON_M1_PIO,     BUTTON_M1},     // 8
        {BUTTON_F3_PIO,     BUTTON_F3},     // 9
        {BUTTON_F1_PIO,     BUTTON_F1},     // 10
        {BUTTON_F2_PIO,     BUTTON_F2},     // 11
        {BUTTON_F4_PIO,     BUTTON_F4},     // 12
        {BUTTON_BNDP_PIO,   BUTTON_BNDP},   // 13
        {BUTTON_F5_PIO,     BUTTON_F5},     // 14
        {BUTTON_G1_PIO,     BUTTON_G1},     // 15
        {BUTTON_PWR_PIO, BUTTON_PWR},                // 16 Power Button
        {TP_IRQ_PIO,TP_IRQ}                 // 17 TP "Button"
};

static void mchf_board_keypad_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ulong i;

	// Common init
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	// Init all from public struct declaration (ui driver)
	// we init all but the last button which is the TP virtual button
	// this needs to be done by the TP code
	// FIXME: Decide if TP pin can be setup here as well.
	for(i = 0; i < (BUTTON_NUM -1); i++)
	{
		GPIO_InitStructure.GPIO_Pin = bm[i].button;
		GPIO_Init(bm[i].port, &GPIO_InitStructure);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_ptt_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void mchf_board_ptt_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

	// RX/TX control pin init
	GPIO_InitStructure.GPIO_Pin = PTT_CNTR;
	GPIO_Init(PTT_CNTR_PIO, &GPIO_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_keyer_irq_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void mchf_board_keyer_irq_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable the BUTTON Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// Configure PADDLE_DASH pin as input
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin   = PADDLE_DAH;
	GPIO_Init(PADDLE_DAH_PIO, &GPIO_InitStructure);

	// Connect Button EXTI Line to PADDLE_DASH GPIO Pin
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource0);

	// Configure PADDLE_DASH EXTI line
	EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set PADDLE_DASH EXTI Interrupt to the lowest priority
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Configure PADDLE_DOT pin as input
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin   = PADDLE_DIT;
	GPIO_Init(PADDLE_DIT_PIO, &GPIO_InitStructure);

    // Connect Button EXTI Line to PADDLE_DOT GPIO Pin
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource1);

    // Configure PADDLE_DOT EXTI line
	EXTI_InitStructure.EXTI_Line    = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

    // Enable and set PADDLE_DOT EXTI Interrupt to the lowest priority
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_power_button_irq_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void mchf_board_power_button_irq_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable the BUTTON Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// Configure pin as input
	GPIO_InitStructure.GPIO_Pin   = BUTTON_PWR;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(BUTTON_PWR_PIO, &GPIO_InitStructure);

	// Connect Button EXTI Line to GPIO Pin
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource13);

	// Configure PADDLE_DASH EXTI line
	EXTI_InitStructure.EXTI_Line    = EXTI_Line13;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set PADDLE_DASH EXTI Interrupt to the lowest priority
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_dac0_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
#if 0
// this function is commented out because it is static (i.e. local only) and not used
// just remove #if 0 if function needs to be used. Reason is to include only used code
// if possible

static void mchf_board_dac0_init(void)
{
	 GPIO_InitTypeDef GPIO_InitStructure;
	 DAC_InitTypeDef  DAC_InitStructure;

	 // DAC Periph clock enable
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	 // DAC channel 1 (DAC_OUT1 = PA.4)
	 GPIO_InitStructure.GPIO_Pin  = DAC0;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	 GPIO_Init(DAC0_PIO, &GPIO_InitStructure);

	 // DAC channel1 Configuration
	 DAC_InitStructure.DAC_Trigger 						= DAC_Trigger_None;
	 DAC_InitStructure.DAC_WaveGeneration 				= DAC_WaveGeneration_None;
	 DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits7_0;//DAC_TriangleAmplitude_4095;
	 DAC_InitStructure.DAC_OutputBuffer 				= DAC_OutputBuffer_Enable;
	 DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	 // Enable DAC Channel1
	 DAC_Cmd(DAC_Channel_1, ENABLE);

	 // Set DAC Channel1 DHR12L register - JFET attenuator off (0V)
	 DAC_SetChannel1Data(DAC_Align_8b_R, 0x00);
}
#endif

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_dac1_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void mchf_board_dac1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;

	 // DAC Periph clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	 // DAC channel 1 (DAC_OUT2 = PA.5)
	GPIO_InitStructure.GPIO_Pin  = DAC1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DAC0_PIO, &GPIO_InitStructure);

	 // DAC channel1 Configuration
	DAC_InitStructure.DAC_Trigger 						= DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration 				= DAC_WaveGeneration_None;
	 DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits7_0;//DAC_TriangleAmplitude_4095;
	 DAC_InitStructure.DAC_OutputBuffer 				= DAC_OutputBuffer_Enable;
	 DAC_Init(DAC_Channel_2, &DAC_InitStructure);

	 // Enable DAC Channel2
	 DAC_Cmd(DAC_Channel_2, ENABLE);

	 // Set DAC Channel2 DHR12L register - PA Bias (3.80 V)
	 DAC_SetChannel2Data(DAC_Align_8b_R, 220);
	}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_adc1_init
//* Object              : ADC1 used for power supply measurements
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
	static void mchf_board_adc1_init(void)
	{
		ADC_InitTypeDef 		ADC_InitStructure;
		ADC_CommonInitTypeDef 	ADC_CommonInitStructure;
		GPIO_InitTypeDef 		GPIO_InitStructure;

	// Enable ADC3 clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	// Configure ADC Channel 6 as analog input
		GPIO_InitStructure.GPIO_Pin 					= ADC1_PWR;
		GPIO_InitStructure.GPIO_Mode 					= GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_PuPd 					= GPIO_PuPd_NOPULL ;
		GPIO_Init(ADC1_PWR_PIO, &GPIO_InitStructure);

	// Common Init
		ADC_CommonInitStructure.ADC_Mode 				= ADC_Mode_Independent;
		ADC_CommonInitStructure.ADC_Prescaler 			= ADC_Prescaler_Div8;
		ADC_CommonInitStructure.ADC_DMAAccessMode 		= ADC_DMAAccessMode_Disabled;
		ADC_CommonInitStructure.ADC_TwoSamplingDelay	= ADC_TwoSamplingDelay_5Cycles;
		ADC_CommonInit(&ADC_CommonInitStructure);

	// Configuration
		ADC_StructInit(&ADC_InitStructure);
		ADC_InitStructure.ADC_Resolution 				= ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode 				= DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode 		= ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge 		= ADC_ExternalTrigConvEdge_None;
		ADC_InitStructure.ADC_DataAlign 				= ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion 			= 1;
		ADC_Init(ADC1,&ADC_InitStructure);

	// Regular Channel Config
		ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_3Cycles);

	// Enable
		ADC_Cmd(ADC1, ENABLE);

	// ADC2 regular Software Start Conv
		ADC_SoftwareStartConv(ADC1);
	}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_adc2_init
//* Object              : ADC2 used for forward antenna power
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
	static void mchf_board_adc2_init(void)
	{
		ADC_InitTypeDef 		ADC_InitStructure;
		ADC_CommonInitTypeDef 	ADC_CommonInitStructure;
		GPIO_InitTypeDef 		GPIO_InitStructure;

	// Enable ADC3 clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

	// Configure ADC Channel 6 as analog input
		GPIO_InitStructure.GPIO_Pin 					= ADC2_RET;
		GPIO_InitStructure.GPIO_Mode 					= GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_PuPd 					= GPIO_PuPd_NOPULL ;
		GPIO_Init(ADC2_RET_PIO, &GPIO_InitStructure);

	// Common Init
		ADC_CommonInitStructure.ADC_Mode 				= ADC_Mode_Independent;
		ADC_CommonInitStructure.ADC_Prescaler 			= ADC_Prescaler_Div8;
		ADC_CommonInitStructure.ADC_DMAAccessMode 		= ADC_DMAAccessMode_Disabled;
		ADC_CommonInitStructure.ADC_TwoSamplingDelay	= ADC_TwoSamplingDelay_5Cycles;
		ADC_CommonInit(&ADC_CommonInitStructure);

	// Configuration
		ADC_StructInit(&ADC_InitStructure);
		ADC_InitStructure.ADC_Resolution 				= ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode 				= DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode 		= ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge 		= ADC_ExternalTrigConvEdge_None;
		ADC_InitStructure.ADC_DataAlign 				= ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion 			= 1;
		ADC_Init(ADC2,&ADC_InitStructure);

	// Regular Channel Config
		ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 1, ADC_SampleTime_3Cycles);

	// Enable
		ADC_Cmd(ADC2, ENABLE);

	// ADC2 regular Software Start Conv
		ADC_SoftwareStartConv(ADC2);
	}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_adc3_init
//* Object              : ADC3 used for return antenna power
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
	static void mchf_board_adc3_init(void)
	{
		ADC_InitTypeDef 		ADC_InitStructure;
		ADC_CommonInitTypeDef 	ADC_CommonInitStructure;
		GPIO_InitTypeDef 		GPIO_InitStructure;

	// Enable ADC3 clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

	// Configure ADC Channel 6 as analog input
		GPIO_InitStructure.GPIO_Pin 					= ADC3_FWD;
		GPIO_InitStructure.GPIO_Mode 					= GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_PuPd 					= GPIO_PuPd_NOPULL ;
		GPIO_Init(ADC3_FWD_PIO, &GPIO_InitStructure);

	// Common Init
		ADC_CommonInitStructure.ADC_Mode 				= ADC_Mode_Independent;
		ADC_CommonInitStructure.ADC_Prescaler 			= ADC_Prescaler_Div8;
		ADC_CommonInitStructure.ADC_DMAAccessMode 		= ADC_DMAAccessMode_Disabled;
		ADC_CommonInitStructure.ADC_TwoSamplingDelay	= ADC_TwoSamplingDelay_5Cycles;
		ADC_CommonInit(&ADC_CommonInitStructure);

	// Configuration
		ADC_StructInit(&ADC_InitStructure);
		ADC_InitStructure.ADC_Resolution 				= ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode 				= DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode 		= ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge 		= ADC_ExternalTrigConvEdge_None;
		ADC_InitStructure.ADC_DataAlign 				= ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion 			= 1;
		ADC_Init(ADC3,&ADC_InitStructure);

	// Regular Channel Config
		ADC_RegularChannelConfig(ADC3, ADC_Channel_2, 1, ADC_SampleTime_3Cycles);

	// Enable
		ADC_Cmd(ADC3, ENABLE);

	// ADC3 regular Software Start Conv
		ADC_SoftwareStartConv(ADC3);
	}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_power_down_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void mchf_board_power_down_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = POWER_DOWN;
	GPIO_Init(POWER_DOWN_PIO, &GPIO_InitStructure);

	// Set initial state - low to enable main regulator
	POWER_DOWN_PIO->BSRRH = POWER_DOWN;
}

// Band control GPIOs setup
//
// -------------------------------------------
// 	 BAND		BAND0		BAND1		BAND2
//
//	 80m		1			1			x
//	 40m		1			0			x
//	 20/30m		0			0			x
//	 15-10m		0			1			x
//
// -------------------------------------------
//
static void mchf_board_band_cntr_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = BAND0|BAND1|BAND2;
	GPIO_Init(BAND0_PIO, &GPIO_InitStructure);

	// Set initial state - low (20m band)
	BAND0_PIO->BSRRH = BAND0;
	BAND1_PIO->BSRRH = BAND1;

	// Pulse the latch relays line, active low, so set high to disable
	BAND2_PIO->BSRRL = BAND2;
}

static void mchf_board_touchscreen_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = TP_IRQ;
	GPIO_Init(TP_IRQ_PIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;

	GPIO_InitStructure.GPIO_Pin = TP_CS;
	GPIO_Init(TP_CS_PIO, &GPIO_InitStructure);

	GPIO_SetBits(TP_CS_PIO,TP_CS);
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_watchdog_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//static void mchf_board_watchdog_init(void)
//{
	// Enable WWDG clock
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

	// WWDG clock counter = (PCLK1 (42MHz)/4096)/8 = 1281 Hz (~780 us)
//	WWDG_SetPrescaler(WWDG_Prescaler_8);

	// Set Window value to 80; WWDG counter should be refreshed only when the counter
	//    is below 80 (and greater than 64) otherwise a reset will be generated
//	WWDG_SetWindowValue(WD_REFRESH_WINDOW);

	// Enable WWDG and set counter value to 127, WWDG timeout = ~780 us * 64 = 49.92 ms
	// In this case the refresh window is: ~780 * (127-80) = 36.6ms < refresh window < ~780 * 64 = 49.9ms
	// -- so wd reset is every 40 mS --
	// --WWDG_Enable(WD_REFRESH_COUNTER);
//}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_set_system_tick_value
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void mchf_board_set_system_tick_value(void)
{
	RCC_ClocksTypeDef 	RCC_Clocks;
//	NVIC_InitTypeDef 	NVIC_InitStructure;

// Configure Systick clock source as HCLK
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

// SystTick configuration
	RCC_GetClocksFreq(&RCC_Clocks);

// Need 1mS tick for responcive UI
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

//	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_green_led
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_board_green_led(int state)
{
	switch(state)
	{
		case 1:
			GREEN_LED_PIO->BSRRL = GREEN_LED;
			break;
		case 0:
			GREEN_LED_PIO->BSRRH = GREEN_LED;
			break;
		default:
			GREEN_LED_PIO->ODR ^= GREEN_LED;
			break;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_power_off
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_board_power_off(void)
{
	ulong i;
	char	tx[40];
	// Power off all - high to disable main regulator

	UiSpectrumClearDisplay();	// clear display under spectrum scope

	Codec_Mute(1);	// mute audio when powering down

	sprintf(tx,"                           ");
	UiLcdHy28_PrintText(80,148,tx,Black,Black,0);

	sprintf(tx,"       Powering off...     ");
	UiLcdHy28_PrintText(80,156,tx,Blue2,Black,0);

	sprintf(tx,"                           ");
	UiLcdHy28_PrintText(80,168,tx,Blue2,Black,0);

	if(ts.ser_eeprom_in_use == 0xff)
	{
		sprintf(tx,"Saving settings to virt. EEPROM");
		UiLcdHy28_PrintText(60,176,tx,Blue,Black,0);
	}
	if(ts.ser_eeprom_in_use == 0x0)
	{
		sprintf(tx,"Saving settings to serial EEPROM");
		UiLcdHy28_PrintText(60,176,tx,Blue,Black,0);
	}
	if(ts.ser_eeprom_in_use == 0x20)
	{
		sprintf(tx," ...without saving settings...  ");
		UiLcdHy28_PrintText(60,176,tx,Blue,Black,0);
		for(i = 0; i < 20; i++)
			non_os_delay();
	}

	if(ts.ser_eeprom_in_use == 0xff)
	{
		sprintf(tx,"            2              ");
		UiLcdHy28_PrintText(80,188,tx,Blue,Black,0);

		sprintf(tx,"                           ");
		UiLcdHy28_PrintText(80,200,tx,Black,Black,0);


		// Delay before killing power to allow EEPROM write to finish
		for(i = 0; i < 10; i++)
			non_os_delay();

		sprintf(tx,"            1              ");
		UiLcdHy28_PrintText(80,188,tx,Blue,Black,0);

		for(i = 0; i < 10; i++)
			non_os_delay();
		
		sprintf(tx,"            0              ");
		UiLcdHy28_PrintText(80,188,tx,Blue,Black,0);

		for(i = 0; i < 10; i++)
			non_os_delay();
	}
	ts.powering_down = 1;	// indicate that we should be powering down

	if(ts.ser_eeprom_in_use != 0x20)
	    UiConfiguration_SaveEepromValues();		// save EEPROM values again - to make sure...

	//
	// Actual power-down moved to "UiDriverHandlePowerSupply()" with part of delay
	// so that EEPROM write could complete without non_os_delay
	// using the constant "POWERDOWN_DELAY_COUNT" as the last "second" of the delay
	//
	// POWER_DOWN_PIO->BSRRL = POWER_DOWN;
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_board_init(void)
{
  	// Enable clock on all ports
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	// Power up hardware
	mchf_board_power_down_init();

	// Filter control lines
	mchf_board_band_cntr_init();

	// Debugging on
	mchf_board_debug_init();

	// LED init
	mchf_board_led_init();

	// Init keypad hw
	mchf_board_keypad_init();

	// Touchscreen Init
	mchf_board_touchscreen_init();

	// I2C init
	mchf_hw_i2c_init();

	// Get startup frequency of Si570, by DF8OE, 201506
	ui_si570_calculate_startup_frequency();


	// Codec control interface
	mchf_hw_i2c2_init();

	// LCD Init
	ts.display_type = UiLcdHy28_Init();
	// we could now implement some error strategy if no display is present
	// i.e. 0 is returned

	// Encoders init
	UiRotaryFreqEncoderInit();
	UiRotaryEncoderOneInit();
	UiRotaryEncoderTwoInit();
	UiRotaryEncoderThreeInit();

	// Init DACs
//	mchf_board_dac0_init();		// disabled because pin is now TP_IRQ
	mchf_board_dac1_init();

	// Enable all ADCs
	mchf_board_adc1_init();
	mchf_board_adc2_init();
	mchf_board_adc3_init();

	// Init watchdog - not working
	//mchf_board_watchdog_init();
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_board_post_init
//* Object              : Extra init, which requires full boot up first
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_board_post_init(void)
{
	// Set system tick interrupt
	// Currently used for UI driver processing only
	mchf_board_set_system_tick_value();

	// Init power button IRQ
	mchf_board_power_button_irq_init();

	// PTT control
	mchf_board_ptt_init();

	// Init keyer interface
	mchf_board_keyer_irq_init();
}


//
// Interface for all EEPROM (ser/virt) functions and our code
//
// if ser_eeprom_in_use == 0 write/read to serial EEPROM,
// if its 0xAA use data in buffer
// otherwise use virtual EEPROM
uint16_t Read_EEPROM(uint16_t addr, uint16_t *value)
{
	if(ts.ser_eeprom_in_use == 0)
		return Read_SerEEPROM(addr, value);
	if(ts.ser_eeprom_in_use == 0xFF || ts.ser_eeprom_in_use == 0x10)
		return(EE_ReadVariable(VirtAddVarTab[addr], value));
	if(ts.ser_eeprom_in_use == 0xAA)
	{
		uint8_t lowbyte;
		uint8_t highbyte;
		uint16_t data;

		highbyte = ts.eeprombuf[addr*2];
		lowbyte = ts.eeprombuf[addr*2+1];
		data = lowbyte + (highbyte<<8);
		*value = data;
	}
	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : Write_EEPROM
//* Object              :
//* Object              :
//* Input Parameters    : addr to write to, 16 bit value as data
//* Output Parameters   : returns FLASH_COMPLETE if OK, otherwise various error codes.
//*                       FLASH_ERROR_OPERATION is also returned if eeprom_in_use contains bogus values.
//* Functions called    :
//*----------------------------------------------------------------------------
uint16_t Write_EEPROM(uint16_t addr, uint16_t value)
{
	FLASH_Status status = FLASH_ERROR_OPERATION;
	if(ts.ser_eeprom_in_use == 0)
	{
		Write_SerEEPROM(addr, value);
		status = FLASH_COMPLETE;
	}
	else if(ts.ser_eeprom_in_use == 0xFF || ts.ser_eeprom_in_use == 0x10)
	{
		status = (EE_WriteVariable(VirtAddVarTab[addr], value));
	}
	else if(ts.ser_eeprom_in_use == 0xAA)
	{
		uint8_t lowbyte;
		uint8_t highbyte;

		lowbyte = (uint8_t)((0x00FF)&value);
		value = value>>8;
		highbyte = (uint8_t)((0x00FF)&value);
		ts.eeprombuf[addr*2] = highbyte;
		ts.eeprombuf[addr*2+1] = lowbyte;
		status = FLASH_COMPLETE;
	}
	return status;
}

//
// Interface for serial EEPROM functions
//
uint16_t Read_SerEEPROM(uint16_t addr, uint16_t *value)		// reference to serial EEPROM read function
{
	uint16_t data;

	data = (uint16_t)(Read_24Cxx(addr*2, ts.ser_eeprom_type)<<8);
	data = data + (uint8_t)(Read_24Cxx(addr*2+1, ts.ser_eeprom_type));
	*value = data;

	return 0;
}

uint16_t Write_SerEEPROM(uint16_t addr, uint16_t value)		// reference to serial EEPROM write function, writing unsigned 16 bit
{
	uint8_t lowbyte, highbyte;

	lowbyte = (uint8_t)(value&(0x00FF));
	highbyte = (uint8_t)((value&(0xFF00))>>8);

	Write_24Cxx(addr*2, highbyte, ts.ser_eeprom_type);
	Write_24Cxx(addr*2+1, lowbyte, ts.ser_eeprom_type);

	return 0;
}

// copy data from virtual to serial EEPROM
void copy_virt2ser(void)
{
	bool seq = true;

	uint16_t data;
//uint8_t *p = malloc(MAX_VAR_ADDR*2+2);

	static uint8_t p[MAX_VAR_ADDR*2+2];
// length of array is 383*2 + 2 = 768
// to allow for the 2 eeprom signature bytes
// stored at index 0/1


	uint16_t i;
// copy virtual EEPROM to RAM, this reads out 383 values and stores them in  2 bytes
	for(i=1; i <= MAX_VAR_ADDR; i++)
	{
		EE_ReadVariable(VirtAddVarTab[i], &data);
		p[i*2+1] = (uint8_t)((0x00FF)&data);
		data = data>>8;
		p[i*2] = (uint8_t)((0x00FF)&data);
	}
	p[0] = Read_24Cxx(0,16);
	p[1] = Read_24Cxx(1,16);
// write RAM to serial EEPROM
	if(seq == false)
	{
		for(i=0; i <= MAX_VAR_ADDR*2;i++)
		{
		// this will write  out 768 bytes (2 signature  and 383*2 data)
			Write_24Cxx(i, p[i], ts.ser_eeprom_type);
		}
	}
	else
	{
	// this will write  out 768 bytes (2 signature  and 383*2 data)
		Write_24Cxxseq(0, p, MAX_VAR_ADDR*2, ts.ser_eeprom_type);
		Write_24Cxx(0x180, p[0x180], ts.ser_eeprom_type);
	}
	ts.ser_eeprom_in_use = 0;		// serial EEPROM in use now
}

// copy data from serial to virtual EEPROM
void copy_ser2virt(void)
{
	uint16_t count;
	uint16_t data;

	for(count=1; count <= MAX_VAR_ADDR; count++)
	{
		Read_SerEEPROM(count, &data);
		EE_WriteVariable(VirtAddVarTab[count], data);
	}
}

// verify data serial / virtual EEPROM
void verify_servirt(void)
{
	uint16_t count;
	uint16_t data1, data2;

	for(count=1; count <= MAX_VAR_ADDR; count++)
	{
		Read_SerEEPROM(count, &data1);
		EE_ReadVariable(VirtAddVarTab[count], &data2);
		if(data1 != data2)
			ts.ser_eeprom_in_use = 0x05;	// mark data copy as faulty
	}
}

void mchf_reboot() {
    ui_si570_get_configuration();       // restore SI570 to factory default
    *(__IO uint32_t*)(SRAM2_BASE) = 0x55;
    NVIC_SystemReset();         // restart mcHF
}
