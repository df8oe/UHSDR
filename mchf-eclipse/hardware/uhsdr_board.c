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

#include "uhsdr_board.h"
#include "ui_configuration.h"
#include "ui_lcd_hy28.h"
#include <stdio.h>

#include "uhsdr_hw_i2c.h"
#include "uhsdr_rtc.h"

#include "ui_driver.h"

#include "ui_rotary.h"

#include "codec.h"

#include "soft_tcxo.h"
//
// Eeprom items
#include "uhsdr_flash.h"
#include "adc.h"
#include "dac.h"

#include "uhsdr_keypad.h"
#include "osc_si5351a.h"

// Transceiver state public structure
__IO __MCHF_SPECIALMEM TransceiverState ts;


static void Board_Led_Init(void)
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

#if 0
// DO NOT ENABLE UNLESS ALL TOUCHSCREEN SETUP CODE IS DISABLED
// TOUCHSCREEN AND USART SHARE PA9 Pin
static void mchf_board_debug_init(void)
{
#ifdef DEBUG_BUILD
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

}
#endif


static void Board_TxRxCntrPin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;


    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;

    // RX/TX control pin init
    GPIO_InitStructure.Pin = TXRX_CNTR;
    HAL_GPIO_Init(TXRX_CNTR_PIO, &GPIO_InitStructure);
}

static void Board_Dac_Init(void)
{

#ifdef UI_BRD_OVI40
    HAL_DAC_Start(&hdac,DAC_CHANNEL_1);
    HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_8B_R,0);
    // AUDIO PA volume zero
#endif
    HAL_DAC_Start(&hdac,DAC_CHANNEL_2);
    HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_8B_R,0);

}


static void Board_Adc_Init(void)
{
    // ADC init for Input Voltage readings
    HAL_ADC_Start(&hadc1);
    // ADC init for antenna forward power readings
    HAL_ADC_Start(&hadc2);
    // ADC init for antenna return power readings
    HAL_ADC_Start(&hadc3);
}


static void Board_PowerDown_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStructure.Pin = POWER_DOWN;
    HAL_GPIO_Init(POWER_DOWN_PIO, &GPIO_InitStructure);

    // Set initial state - low to enable main regulator
    GPIO_ResetBits(POWER_DOWN_PIO,POWER_DOWN);
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
static void Board_BandCntr_Init(void)
{
#ifdef UI_BRD_MCHF
    // FIXME: USE HAL Init here as well, this handles also the multiple Ports case
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode 	= GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull 	= GPIO_NOPULL;
    GPIO_InitStructure.Speed 	= GPIO_SPEED_LOW;

    GPIO_InitStructure.Pin = BAND0|BAND1|BAND2;
    HAL_GPIO_Init(BAND0_PIO, &GPIO_InitStructure);
#endif
    // Set initial state - low (20m band)
    GPIO_ResetBits(BAND0_PIO,BAND0);
    GPIO_ResetBits(BAND1_PIO,BAND1);

    // Pulse the latch relays line, active low, so set high to disable
    GPIO_SetBits(BAND2_PIO,BAND2);
}

static void Board_Touchscreen_Init()
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

/**
 * Get us to a state where display and touch (and some other stuff) works, we have an idea about the
 * rf hardware connected to us and then let the application do their thing
 */
void Board_InitMinimal()
{
    // Enable clock on all ports
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();

    // LED init
    Board_Led_Init();
    Board_RedLed(LED_STATE_ON);

    // Power up hardware
    Board_PowerDown_Init();

  // FROM HERE
    // Filter control lines
    Board_BandCntr_Init();

    // Touchscreen SPI Control Signals Init
    // TODO: Move to CubeMX Config
    Board_Touchscreen_Init();

    // Initialize LO
    Osc_Init();

    // TO HERE: Code be moved to init_full() if we figure out what causes the white screen @MiniTRX SPI

    // TODO: It seems that some SPI display need some time to get started...
    // LCD Init
    UiLcdHy28_Init();

    // we determine and set the correct RF board here
    ts.rf_board = Si5351a_IsPresent()?FOUND_RF_BOARD_OVI40:FOUND_RF_BOARD_MCHF;

}

/*
 * This initializes non-essential hardware for later use by the application
 */
void Board_InitFull()
{

#ifdef UI_BRD_MCHF
    // on a STM32F4 MCHF UI we can have the internal RTC only if there is an SPI display.
    if (ts.display->use_spi == true)
#endif
    {
        ts.rtc_present = Rtc_isEnabled();
    }
#ifdef UI_BRD_MCHF
    // we need to find out which keyboard layout before we init the GPIOs to use it.
    // at this point we have to have called the display init and the rtc init
    // in order to know which one to use.
    if (ts.rtc_present)
    {
        Keypad_SetLayoutRTC_MCHF();
    }
#endif

    // Init keypad hw based on button map bm
    Keypad_KeypadInit();

    // Encoders init
    UiRotaryFreqEncoderInit();
    UiRotaryEncoderOneInit();
    UiRotaryEncoderTwoInit();
    UiRotaryEncoderThreeInit();

    // Init DACs
    Board_Dac_Init();

    // Enable all ADCs
    Board_Adc_Init();
}

/*
 * @brief  handle final power-off and delay
 */
void Board_HandlePowerDown() {
    static ulong    powerdown_delay = 0;

    if(ts.powering_down)        // are we powering down?
    {
        powerdown_delay++;      // yes - do the powerdown delay
        if(powerdown_delay > POWERDOWN_DELAY_COUNT)     // is it time to power down
        {
            Board_Powerdown();
            // never reached
        }
    }
}
/*
 * @brief kills power hold immediately and waits for user to release power button
 * @returns never returns
 */
void Board_Powerdown()
{
    // we set this to input and add a pullup config
    // this seems to be more reliably handling power down
    // on F7 by rising the voltage to high enough levels.
    // simply setting the OUTPUT to high did not do the trick here
    // worked on F4, though.

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStructure.Pin = POWER_DOWN;
    HAL_GPIO_Init(POWER_DOWN_PIO, &GPIO_InitStructure);

    // disable all interrupts
    __disable_irq();

    // disable systick irq
    HAL_SuspendTick();

    // set clocks to default state
    HAL_RCC_DeInit();

    // clear Interrupt Enable Register & Interrupt Pending Register
    const size_t icer_count = sizeof(NVIC->ICER)/sizeof(NVIC->ICER[0]);

    for (size_t i = 0; i <icer_count; i++)
    {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }

    Board_GreenLed(LED_STATE_OFF);
    UiLcdHy28_BacklightEnable(false);

    for(;;) { asm("nop"); }
    // there is no coming back from here...
}

/**
 * This is called once AFTER configuration data has been loaded from persistent storage
 * (i.e. EEPROM or FLASH)
 */
void Board_PostInit(void)
{
    // Set system tick interrupt
    // Currently used for UI driver processing only
    ///mchf_board_set_system_tick_value();

    // PTT control
    Board_TxRxCntrPin_Init();

    if (ts.rtc_present)
    {
        Rtc_SetPpm(ts.rtc_calib);
    }
}

void Board_Reboot()
{
    ///Si570_ResetConfiguration();       // restore SI570 to factory default
    *(__IO uint32_t*)(SRAM2_BASE) = 0x55;
    __DSB();
#if defined(STM32F7) || defined(STM32H7)
    SCB_CleanDCache();
#endif
    NVIC_SystemReset();         // restart mcHF
}

// #pragma GCC optimize("O0")

static volatile bool busfault_detected;

#define TEST_ADDR_192 (0x20000000 + 0x0001FFFC)
#define TEST_ADDR_256 (0x20000000 + 0x0002FFFC)
#define TEST_ADDR_512 (0x20000000 + 0x0005FFFC)

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

uint32_t Board_RamSizeGet() {
    uint32_t retval = 0;
    // we enable the bus fault
    // we now get bus faults if we access not  available  memory
    // instead of hard faults
    // this will run our very special bus fault handler in case no memory
    // is at the defined location
#if defined(STM32F4) || defined(STM32F7)
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
    if (is_ram_at((volatile uint32_t*)TEST_ADDR_512)){
        retval=512;
    } else if (is_ram_at((volatile uint32_t*)TEST_ADDR_256)){
        retval=256;
    } else if (is_ram_at((volatile uint32_t*)TEST_ADDR_192)){
        retval=192;
    }
    // now we disable it
    // we'll get hard faults as usual if we access wrong addresses
    SCB->SHCSR &= ~SCB_SHCSR_BUSFAULTENA_Msk;
#elif defined(STM32H7)
    //  TODO make it detect the really available RAM
    retval = 512;
#else
    #warning Unkown processor, cannot determine ramsize
    retval = 0;
#endif
    return retval;
}


/**
 * Determines the available RAM. Only supports 192 and 256 STM32F4 models
 * Approach works but makes some assumptions. Do not change if you don't know
 * what you are doing!
 * USE WITH CARE!
 */
void Board_RamSizeDetection() {
    // we enable the bus fault
    // we now get bus faults if we access not  available  memory
    // instead of hard faults
    // this will run our very special bus fault handler in case no memory
    // is at the defined location
    ts.ramsize = Board_RamSizeGet();
}



static void Board_BandFilterPulseRelays()
{
    // FIXME: Replace non_os_delay with HAL_Delay
    GPIO_ResetBits(BAND2_PIO, BAND2);
    // TODO: Check if we can go down to 10ms as per datasheet
    // HAL_Delay(20);
    non_os_delay();
    GPIO_SetBits(BAND2_PIO, BAND2);
}

/**
 * @brief switches one of the four LPF&BPF groups into the RX/TX signal path
 * @param group 0: 80m, 1: 40m, 2: 20m , 3:10m
 */
void Board_SelectLpfBpf(uint8_t group)
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
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        Board_BandFilterPulseRelays();

        // External group -Set(High/High)
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        Board_BandFilterPulseRelays();

        // BPF
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        break;
    }

    case 1:
    {
        // Internal group - Set(High/Low)
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        Board_BandFilterPulseRelays();

        // External group - Reset(Low/High)
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        Board_BandFilterPulseRelays();

        // BPF
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        break;
    }

    case 2:
    {
        // Internal group - Reset(Low/Low)
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        Board_BandFilterPulseRelays();

        // External group - Reset(Low/High)
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        Board_BandFilterPulseRelays();

        // BPF
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        break;
    }

    case 3:
    {
        // Internal group - Reset(Low/Low)
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        Board_BandFilterPulseRelays();

        // External group - Set(High/High)
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        Board_BandFilterPulseRelays();

        // BPF
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        break;
    }

    default:
        break;
    }

}

const char* Board_BootloaderVersion()
{
    const char* outs = "Unknown BL";

    // We search for string "Version: " in bootloader memory
    // this assume the bootloader starting at 0x8000000 and being followed by the virtual eeprom
    // which starts at EEPROM_START_ADDRESS
    for(uint8_t* begin = (uint8_t*)0x8000000; begin < (uint8_t*)EEPROM_START_ADDRESS-8; begin++)
    {
        if (memcmp("Version: ",begin,9) == 0)
        {
            outs = (const char*)&begin[9];
            break;
        }
        else
        {
            if (memcmp("M0NKA 2",begin,7) == 0)
            {
                if (begin[11] == 0xb5)
                {
                    outs = "M0NKA 0.0.0.9";
                    break;
                }
                else if (begin[11] == 0xd1)
                {
                    outs = "M0NKA 0.0.0.14";
                    break;
                }
            }
        }
    }
    return outs;
}

/**
 * @brief set PA bias at the LM2931CDG (U18) using DAC Channel 2
 */
void Board_SetPaBiasValue(uint16_t bias)
{
    // Set DAC Channel 1 DHR12L register
    // DAC_SetChannel2Data(DAC_Align_8b_R,bias);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_8B_R, bias);

}

void Board_GreenLed(ledstate_t state)
{
    switch(state)
    {
    case LED_STATE_ON:
        GPIO_SetBits(GREEN_LED_PIO, GREEN_LED);
        break;
    case LED_STATE_OFF:
        GPIO_ResetBits(GREEN_LED_PIO, GREEN_LED);
        break;
    default:
        GPIO_ToggleBits(GREEN_LED_PIO, GREEN_LED);
        break;
    }
}

void Board_RedLed(ledstate_t state)
{
    switch(state)
    {
    case LED_STATE_ON:
        GPIO_SetBits(RED_LED_PIO, RED_LED);
        break;
    case LED_STATE_OFF:
        GPIO_ResetBits(RED_LED_PIO, RED_LED);
        break;
    default:
        GPIO_ToggleBits(RED_LED_PIO, RED_LED);
        break;
    }
}

#ifdef UI_BRD_OVI40
void Board_BlueLed(ledstate_t state)
{
    switch(state)
    {
    case LED_STATE_ON:
        GPIO_SetBits(BLUE_LED_PIO, BLUE_LED);
        break;
    case LED_STATE_OFF:
        GPIO_ResetBits(BLUE_LED_PIO, BLUE_LED);
        break;
    default:
        GPIO_ToggleBits(BLUE_LED_PIO, BLUE_LED);
        break;
    }
}
#endif
/**
 * @brief sets the hw ptt line and by this switches the mcHF board signal path between rx and tx configuration
 * @param tx_enable true == TX Paths, false == RX Paths
 */
void Board_EnableTXSignalPath(bool tx_enable)
{
    // to make switching as noiseless as possible, make sure the codec lineout is muted/produces zero output before switching
    if (tx_enable)
    {
        GPIO_SetBits(TXRX_CNTR_PIO,TXRX_CNTR);     // TX on and switch CODEC audio paths
        // Antenna Direction Output
        // BPF Direction Output (U1,U2)
        // PTT Optocoupler LED On (ACC Port) (U6)
        // QSD Mixer Output Disable (U15)
        // QSE Mixer Output Enable (U17)
        // Codec LineIn comes from mcHF LineIn Socket (U3)
        // Codec LineOut connected to QSE mixer (IQ Out) (U3a)
    }
    else
    {
        GPIO_ResetBits(TXRX_CNTR_PIO,TXRX_CNTR); // TX off
        // Antenna Direction Input
        // BPF Direction Input (U1,U2)
        // PTT Optocoupler LED Off (ACC Port) (U6)
        // QSD Mixer Output Enable (U15)
        // QSE Mixer Output Disable (U17)
        // Codec LineIn comes from RF Board QSD mixer (IQ In) (U3)
        // Codec LineOut disconnected from QSE mixer  (IQ Out) (U3a)
    }
}

/**
 * Is the hardware contact named DAH pressed
 */
bool Board_PttDahLinePressed() {
    return  !HAL_GPIO_ReadPin(PADDLE_DAH_PIO,PADDLE_DAH);
}

/**
 * Is the hardware contact named DIT pressed
 */
bool Board_DitLinePressed() {
    return  !HAL_GPIO_ReadPin(PADDLE_DIT_PIO,PADDLE_DIT);
}
