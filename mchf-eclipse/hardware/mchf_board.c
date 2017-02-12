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

#include "mchf_board.h"
#include "ui_configuration.h"
#include "ui_lcd_hy28.h"
#include <stdio.h>

#include "mchf_hw_i2c.h"
#include "mchf_rtc.h"

#include "ui_driver.h"

#include "ui_rotary.h"

#include "codec.h"

#include "ui_si570.h"
#include "soft_tcxo.h"
//
// Eeprom items
#include "eeprom.h"

// Transceiver state public structure
__IO __attribute__ ((section (".ccm"))) TransceiverState ts;


static void mchf_board_led_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStructure.Pin = GREEN_LED;
    HAL_GPIO_Init(GREEN_LED_PIO, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = RED_LED;
    HAL_GPIO_Init(RED_LED_PIO, &GPIO_InitStructure);
}

// DO NOT ENABLE UNLESS ALL TOUCHSCREEN SETUP CODE IS DISABLED
// TOUCHSCREEN AND USART SHARE PA9 Pin
static void mchf_board_debug_init(void)
{
#ifdef DEBUG_BUILD

#if 0
    // disabled the USART since it is used by the touch screen code
    // as well which renders it unusable
    #error "Debug Build No Longer Supported, needs alternative way of communication"
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

#endif
}

// -------------------------------------------------------
// Constant declaration of the buttons map across ports
// - update if moving buttons around !!!
const ButtonMap bm_sets[2][18] =
{
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
        },
        // alternative mapping for RTC Modification
        {
                {BUTTON_M2_PIO,     BUTTON_M2},     // 0
                {BUTTON_G2_PIO,     BUTTON_G2},     // 1
                {BUTTON_G3_PIO,     BUTTON_G3},     // 2
                {BUTTON_BNDM_PIO,   BUTTON_BNDM},   // 3
                {BUTTON_G4_PIO,     BUTTON_G4},     // 4
                {BUTTON_M3_PIO,     BUTTON_M3},     // 5
                {BUTTON_STEPM_PIO,  BUTTON_STEPM},  // 6
                {BUTTON_STEPP_PIO,  BUTTON_STEPP},  // 7
                {BUTTON_M1_PIO_RTC,     BUTTON_M1_RTC},     // 8
                {BUTTON_F3_PIO_RTC,     BUTTON_F3_RTC},     // 9
                {BUTTON_F1_PIO,     BUTTON_F1},     // 10
                {BUTTON_F2_PIO,     BUTTON_F2},     // 11
                {BUTTON_F4_PIO,     BUTTON_F4},     // 12
                {BUTTON_BNDP_PIO,   BUTTON_BNDP},   // 13
                {BUTTON_F5_PIO,     BUTTON_F5},     // 14
                {BUTTON_G1_PIO,     BUTTON_G1},     // 15
                {BUTTON_PWR_PIO, BUTTON_PWR},                // 16 Power Button
                {TP_IRQ_PIO,TP_IRQ}                 // 17 TP "Button"
        }
};

// the inital button map is the default one
const ButtonMap* bm = &bm_sets[0][0];

static void mchf_board_keypad_init(const ButtonMap* bm)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    ulong i;

    // Common init
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Pull = GPIO_PULLUP;

    // Init all from public struct declaration (ui driver)
    // we init all but the last button which is the TP virtual button
    // this needs to be done by the TP code
    // FIXME: Decide if TP pin can be setup here as well.
    for(i = 0; i < (BUTTON_NUM -1); i++)
    {
        GPIO_InitStructure.Pin = bm[i].button;
        HAL_GPIO_Init(bm[i].port, &GPIO_InitStructure);
    }
}

static void mchf_board_ptt_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;


    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;

    // RX/TX control pin init
    GPIO_InitStructure.Pin = PTT_CNTR;
    HAL_GPIO_Init(PTT_CNTR_PIO, &GPIO_InitStructure);
}

static void mchf_board_keyer_irq_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Configure PADDLE_DASH pin as input
    GPIO_InitStructure.Mode  = GPIO_MODE_IT_FALLING;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;

    GPIO_InitStructure.Pin   = PADDLE_DAH;
    HAL_GPIO_Init(PADDLE_DAH_PIO, &GPIO_InitStructure);


    GPIO_InitStructure.Pin   = PADDLE_DIT;
    HAL_GPIO_Init(PADDLE_DIT_PIO, &GPIO_InitStructure);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

static void mchf_board_power_button_irq_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ///EXTI_InitTypeDef EXTI_InitStructure;
    ///NVIC_InitTypeDef NVIC_InitStructure;

    // Enable the BUTTON Clock
    ///RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // Configure pin as input
    GPIO_InitStructure.Pin   = BUTTON_PWR;
    GPIO_InitStructure.Mode  = GPIO_MODE_IT_FALLING;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(BUTTON_PWR_PIO, &GPIO_InitStructure);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    /*
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
    */
}

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

#if 0
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

    GPIO_StructInit(&GPIO_InitStructure);


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

    GPIO_StructInit(&GPIO_InitStructure);


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

    GPIO_StructInit(&GPIO_InitStructure);


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

    GPIO_StructInit(&GPIO_InitStructure);


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
#endif

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

    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;

    GPIO_InitStructure.Pin = POWER_DOWN;
    HAL_GPIO_Init(POWER_DOWN_PIO, &GPIO_InitStructure);

    // Set initial state - low to enable main regulator
    POWER_DOWN_PIO->BSRR = POWER_DOWN  << 16U;
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

    GPIO_InitStructure.Mode 	= GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull 	= GPIO_NOPULL;
    GPIO_InitStructure.Speed 	= GPIO_SPEED_LOW;

    GPIO_InitStructure.Pin = BAND0|BAND1|BAND2;
    HAL_GPIO_Init(BAND0_PIO, &GPIO_InitStructure);

    // Set initial state - low (20m band)
    BAND0_PIO->BSRR = BAND0 << 16U;
    BAND1_PIO->BSRR = BAND1 << 16U;

    // Pulse the latch relays line, active low, so set high to disable
    BAND2_PIO->BSRR = BAND2;
}

static void mchf_board_touchscreen_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode 	= GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull 	= GPIO_PULLUP;
    GPIO_InitStructure.Speed 	= GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStructure.Pin = TP_IRQ;
    HAL_GPIO_Init(TP_IRQ_PIO, &GPIO_InitStructure);

    GPIO_InitStructure.Mode 	= GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin = TP_CS;
    HAL_GPIO_Init(TP_CS_PIO, &GPIO_InitStructure);

    GPIO_SetBits(TP_CS_PIO, TP_CS);
}
#if 0
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

#endif

void mchf_board_init(void)
{
    // Enable clock on all ports
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();

    // LED init
    mchf_board_led_init();
    MchfBoard_RedLed(LED_STATE_ON);

    // Power up hardware
    mchf_board_power_down_init();

    // Filter control lines
    mchf_board_band_cntr_init();

    // Debugging on
    mchf_board_debug_init();


    // Touchscreen Init
    mchf_board_touchscreen_init();

    // I2C init
    mchf_hw_i2c1_init();

    // Get startup frequency of Si570, by DF8OE, 201506
    Si570_Init();

    SoftTcxo_Init();

    // Codec control interface
    mchf_hw_i2c2_init();

    // LCD Init
    ts.display_type = UiLcdHy28_Init();
    // we could now implement some error strategy if no display is present
    // i.e. 0 is returned


    ts.rtc_present = MchfRtc_enabled();

    // we need to find out which keyboard layout before we init the GPIOs to use it.
    // at this point we have to have called the display init and the rtc init
    // in order to know which one to use.
    // parallel display never has a STM32 based rtc, so we do not need to check for RTC
    if ((ts.display_type == DISPLAY_HY28A_SPI || ts.display_type == DISPLAY_HY28B_SPI) && ts.rtc_present)
    {
        bm = &bm_sets[1][0];
    }

    // Init keypad hw based on button map bm
    mchf_board_keypad_init(bm);

#if 0
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
#endif

}

/*
 * @brief  handle final power-off and delay
 */
void MchfBoard_HandlePowerDown() {
    static ulong    powerdown_delay = 0;

    if(ts.powering_down)        // are we powering down?
    {
        powerdown_delay++;      // yes - do the powerdown delay
        if(powerdown_delay > POWERDOWN_DELAY_COUNT)     // is it time to power down
        {
            mchf_powerdown();
            // never reached
        }
    }
}
/*
 * @brief kills power hold immediately and waits for user to release power button
 * @returns never returns
 */
void mchf_powerdown()
{
    POWER_DOWN_PIO->BSRR = POWER_DOWN;
    for(;;) {}
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
    ///mchf_board_set_system_tick_value();

    // Init power button IRQ
    mchf_board_power_button_irq_init();

    // PTT control
    mchf_board_ptt_init();

    // Init keyer interface
    mchf_board_keyer_irq_init();

    if (ts.rtc_present)
    {
        MchfRtc_SetPpm(ts.rtc_calib);
    }
}
void mchf_reboot()
{
    ///Si570_ResetConfiguration();       // restore SI570 to factory default
    *(__IO uint32_t*)(SRAM2_BASE) = 0x55;
    NVIC_SystemReset();         // restart mcHF
}

// #pragma GCC optimize("O0")

static volatile bool busfault_detected;

#define TEST_ADDR_192 (0x20000000 + 0x0001FFFC)
#define TEST_ADDR_256 (0x20000000 + 0x0002FFFC)

// function below mostly based on http://stackoverflow.com/questions/23411824/determining-arm-cortex-m3-ram-size-at-run-time

__attribute__ ((naked)) void BusFault_Handler(void) {
  /* NAKED function so we can be sure that SP is correct when we
   * run our asm code below */

  // DO NOT clear the busfault active flag - it causes a hard fault!

  /* Instead, we must increase the value of the PC, so that when we
   * return, we don't return to the same instruction.
   *
   * Registers are stacked as follows: r0,r1,r2,r3,r12,lr,pc,xPSR
   * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337e/Babedgea.html
   *
   * So we want PC - the 6th down * 4 bytes = 24
   *
   * Then we add 2 - which IS DANGEROUS because we're assuming that the op
   * is 2 bytes, but it COULD be 4.
   */
  asm("mov r3, %0\n mov r2,#1\n str r2,[r3,#0]\n" : : "l" (&busfault_detected) );
  // WE LEAVE 1 in busfault_detected -> if we have a busfault there is no memory here.


  __asm__(
      "ldr r0, [sp, #24]\n"  // load the PC
      "add r0, #2\n"         // increase by 2 - dangerous, see above
      "str r0, [sp, #24]\n"  // save the PC back
      "bx lr\n"              // Return (function is naked so we must do this explicitly)
  );
}


/*
 * Tests if there is ram at the specified location
 * Use with care and with 4byte aligned addresses.
 * IT NEEDS A MATCHING BUSFAULT HANDLER!!!!
 */

__attribute__ ((noinline)) bool is_ram_at(volatile uint32_t* where) {
    bool retval;
    // we rely on the BusFault_Handler setting r4 to 0 (aka false) if a busfault occurs.
    // this is truly bad code as it can be broken easily. The function cannot be optimize
    // this this breaks the approach.

    uint32_t oldval;
    busfault_detected = false;
    oldval = *where;

    if (*where == oldval+1) {
        *where = oldval;
    }
    retval = busfault_detected == false;
    busfault_detected = false;
    return retval;
}

unsigned int mchf_board_get_ramsize() {
    uint32_t retval = 0;
    // we enable the bus fault
    // we now get bus faults if we access not  available  memory
    // instead of hard faults
    // this will run our very special bus fault handler in case no memory
    // is at the defined location
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
    if (is_ram_at((volatile uint32_t*)TEST_ADDR_256)){
        retval=256;
    } else if (is_ram_at((volatile uint32_t*)TEST_ADDR_192)){
        retval=192;
    }
    // now we disable it
    // we'll get hard faults as usual if we access wrong addresses
    SCB->SHCSR &= ~SCB_SHCSR_BUSFAULTENA_Msk;

    return retval;
}


/**
 * Determines the available RAM. Only supports 192 and 256 STM32F4 models
 * Approach works but makes some assumptions. Do not change if you don't know
 * what you are doing!
 * USE WITH CARE!
 */
void mchf_board_detect_ramsize() {
    // we enable the bus fault
    // we now get bus faults if we access not  available  memory
    // instead of hard faults
    // this will run our very special bus fault handler in case no memory
    // is at the defined location
    ts.ramsize = mchf_board_get_ramsize();
}



static void MchfBoard_BandFilterPulseRelays()
{
    BAND2_PIO->BSRR = BAND2 << 16U;
    non_os_delay();
    BAND2_PIO->BSRR = BAND2;
}

/**
 * @brief switches one of the four LPF&BPF groups into the RX/TX signal path
 * @param group 0: 80m, 1: 40m, 2: 20m , 3:10m
 */
void MchfBoard_SelectLpfBpf(uint8_t group)
{
    // -------------------------------------------
    //   BAND       BAND0       BAND1       BAND2
    //
    //   80m        1           1           x
    //   40m        1           0           x
    //   20/30m     0           0           x
    //   15-10m     0           1           x
    //
    // ---------------------------------------------
    // Set LPFs:
    // Set relays in groups, internal first, then external group
    // state change via two pulses on BAND2 line, then idle
    //
    // then
    //
    // Set BPFs
    // Constant line states for the BPF filter,
    // always last - after LPF change
    switch(group)
    {
    case 0:
    {
        // Internal group - Set(High/Low)
        BAND0_PIO->BSRR = BAND0;
        BAND1_PIO->BSRR = BAND1 << 16U;

        MchfBoard_BandFilterPulseRelays();

        // External group -Set(High/High)
        BAND0_PIO->BSRR = BAND0;
        BAND1_PIO->BSRR = BAND1;

        MchfBoard_BandFilterPulseRelays();

        // BPF
        BAND0_PIO->BSRR = BAND0;
        BAND1_PIO->BSRR = BAND1;

        break;
    }

    case 1:
    {
        // Internal group - Set(High/Low)
        BAND0_PIO->BSRR = BAND0;
        BAND1_PIO->BSRR = BAND1 << 16U;

        MchfBoard_BandFilterPulseRelays();

        // External group - Reset(Low/High)
        BAND0_PIO->BSRR = BAND0 << 16U;
        BAND1_PIO->BSRR = BAND1;

        MchfBoard_BandFilterPulseRelays();

        // BPF
        BAND0_PIO->BSRR = BAND0;
        BAND1_PIO->BSRR = BAND1 << 16U;

        break;
    }

    case 2:
    {
        // Internal group - Reset(Low/Low)
        BAND0_PIO->BSRR = BAND0 << 16U;
        BAND1_PIO->BSRR = BAND1 << 16U;

        MchfBoard_BandFilterPulseRelays();

        // External group - Reset(Low/High)
        BAND0_PIO->BSRR = BAND0 << 16U;
        BAND1_PIO->BSRR = BAND1;

        MchfBoard_BandFilterPulseRelays();

        // BPF
        BAND0_PIO->BSRR = BAND0 << 16U;
        BAND1_PIO->BSRR = BAND1 << 16U;

        break;
    }

    case 3:
    {
        // Internal group - Reset(Low/Low)
        BAND0_PIO->BSRR = BAND0 << 16U;
        BAND1_PIO->BSRR = BAND1 << 16U;

        MchfBoard_BandFilterPulseRelays();

        // External group - Set(High/High)
        BAND0_PIO->BSRR = BAND0;
        BAND1_PIO->BSRR = BAND1;

        MchfBoard_BandFilterPulseRelays();

        // BPF
        BAND0_PIO->BSRR = BAND0 << 16U;
        BAND1_PIO->BSRR = BAND1;

        break;
    }

    default:
        break;
    }

}
