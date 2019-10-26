/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  Description:   Please provide one                                             **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UHSDR_BOARD_CONFIG_H
#define __UHSDR_BOARD_CONFIG_H

#if defined(UI_BRD_MCHF) && defined(UI_BRD_OVI40)
    #error Only one ui board can be selected: UI_BRD_MCHF, UI_BRD_OVI40
#elif defined(UI_BRD_OVI40)
    #include "UHSDR_UI_ovi40_config.h"
#elif defined(UI_BRD_MCHF)
    #include "UHSDR_UI_mchf_config.h"
#else
    #error One ui board has to be selected: UI_BRD_MCHF, UI_BRD_OVI40
#endif

#if !defined(BOOTLOADER_BUILD)
// The rf boards we want to support, but the bootloader should compile for all if possible.
// so we don't tell the bootloader which one we have
    #if !defined(RF_BRD_OVI40) && !defined(RF_BRD_MCHF) && !defined(RF_BRD_LAPWING)
        #error At least one rf board must be selected: RF_BRD_MCHF, RF_BRD_OVI40, RF_BRD_LAPWING
    #else
        #if defined(RF_BRD_OVI40)
            #include "UHSDR_RF_ovi40_config.h"
        #endif
        #if defined(RF_BRD_MCHF)
            #include "UHSDR_RF_mchf_config.h"
        #endif
    #endif
#endif

#include "uhsdr_types.h"

//************DEFAULT_VALUES_IF_NOT_SET_IN_BOARD_CONFIG_FILES**********//

#if !defined(TRX_NAME)
    #define TRX_NAME "mcHF QRP"
#endif //TRX_NAME

#if !defined(TRX_ID)
    #define TRX_ID "mchf"
#endif // TRX_ID

#if !defined(TRX_HW_LIC)
    #define TRX_HW_LIC "CC BY-NC-SA 3.0"
    #define TRX_HW_CREATOR "K. Atanassov, M0NKA, www.m0nka.co.uk"
#endif // TRX_HW_LIC

#if !defined(__MCHF_SPECIALMEM)
    #define __MCHF_SPECIALMEM
#endif

#if !defined(__UHSDR_DMAMEM)
    #define __UHSDR_DMAMEM
#endif

#if !defined(DEVICE_STRING)
    #define DEVICE_STRING TRX_NAME " Transceiver"
#endif

/***
 * Please document all switches/parameters with what they are supposed to do and what values they can have.
 * Please use proper naming:
 * For capabilities of the software which can be enabled and disabled
 * use USE_<CAPABILITY/FEATURENAME>
 *
 * These should be defined using #define USE_CAPABILITY
 * or left undefined if not enabled so that these can be checked using #ifdef
 *
 * For related parameters DON'T USE USE_...
 *
 * In an ideal world please use PAR_<CAPABILITY/FEATURE>_<PARAMETERNAME> (we haven't done that yet)
 * Please don't define constant or local parameters here, only those a user (!) is supposed to change as part of
 * configuring a specific build variant.
 *
 */

// Fast convolution filtering
// experimental at the moment DD4WH, 2018_08_18
//#define USE_CONVOLUTION

// old LMS noise reduction
// will probably never used any more
//#define OBSOLETE_NR

// this switches on the autonotch filter based on LMS algorithm
// leave this switched on, until we have a new autonotch filter approach
#define USE_LMS_AUTONOTCH

// save processor time for the STM32F4
// changes lowpass decimation filters to 89 taps instead of 199 taps
// because they run at 48ksps, this is a considerable decrease in processing power
#ifdef STM32F4
//#define USE_SMALL_HILBERT_DECIMATION_FILTERS
#endif

// save processor time for the STM32F4
// changes lowpass decimation filters to 89 taps instead of 199 taps
// because they run at 48ksps, this is a considerable decrease in processing power
// this is ONLY relevant for STM32F4 which has SPI !
// in those machines when enabling NR and other features, ui slows down . . .
// thus we should enable the small decimation filter in those machines

/**
 * This parameter disables certain features / capabilites in order to achieve a minimum build size for
 * the 192k ram / 512k flash STM32F4 machines. Unless you have such a machine, leave this disabled.
 */
// #define IS_SMALL_BUILD

#if !defined(IS_SMALL_BUILD)
    #define USE_8bit_FONT
    #define USE_PREDEFINED_WINDOW_DATA

    // OPTION
    // with IS_SMALL_BUILD we are not automatically including USE_FREEDV as it uses lot of memory
    // both RAM and flash
    #define USE_FREEDV
#endif // IS_SMALL_BUILD

// some special switches
//#define   DEBUG_BUILD
//#define   DEBUG_FREEDV

// if enabled the alternate (read new and better) noise reduction is active
// this is the standard NR now (Febr 2018)
#define USE_ALTERNATE_NR

// you may optionally define the list of supported GFX drivers externally
// if you just define EXTERNAL_USE_GFX_CONFIG and no USE_GFX_...
// you will get a headless system using a dummy display driver
#ifndef EXTERNAL_USE_GFX_CONFIG
    #define USE_GFX_ILI932x
    #define USE_GFX_ILI9486
    // SSD1289 support is not yet working, also requires USE_GFX_ILI932x to be enabled for now.
    // #define USE_GFX_SSD1289
    #define USE_DISP_480_320
    #if defined(STM32F7) || defined(STM32H7)
      #define USE_GFX_RA8875
    #endif
#endif

#define USE_FFT_1024

// OPTION
#define USE_RTTY_PROCESSOR

// OPTION
#define USE_USBHOST
#ifdef USE_USBHOST
    // define additional USBHOST related "switches" only here!
    // #define USE_USBDRIVE
    #define USE_USBKEYBOARD
#endif


// use the STM32 internal RTC with an external quartz and
// M1 and F3 connected to PD14 and PD15 (D0 and D1 of LCD) instead of PC14 and PC15 (to which the 32768 Hz quartz has to be connected)
#define USE_RTC_LSE

// multiple oscillators may be enabled, but only the first detected oscillator is issued
// i.e. there is currently only support for a single oscillator in a TRX.
// Support for LO based on SI570
#define USE_OSC_SI570
// Support for LO based on SI5351
#define USE_OSC_SI5351A

// OPTION TO USE FLASH BASED CONFIGURATION STORAGE
#define USE_CONFIGSTORAGE_FLASH

// Option: If defined, high priority tasks are executed in the context of an PendSV interrupt
// which gives finishing these tasks a priority over "normal", less real-time critical longer running user control tasks
// such as display redraw.
// In general this should be defined but in case of issues one may want to execute High Prio tasks not concurrently
// to normal tasks, comment this in this case and see if the issue goes away. But this may cause other problems
// of course.
#define USE_PENDSV_FOR_HIGHPRIO_TASKS

// OPTION: Enable handling of TX/RX switching in an interrupt. Provides very low latency switching
// EXPERIMENTAL !!!
#define USE_HIGH_PRIO_PTT

// OPTION: IQ signal path now use 24bit samples from/to the codecs instead of the default 16bit. Slightly increases RAM usage (+0.5 - 1k).
// will finally work both on single and dual codec configurations.
#define USE_32_IQ_BITS
#define USE_32_AUDIO_BITS

// OPTION: Instead of band names for memories and enabling only band memories which are supported
// by the current RF board, use all available memories
// this changes the displayed memory name to Mem<Num> instead of Bnd<Wavelength>
#ifdef RF_BRD_LAPWING
    #define USE_MEMORY_MODE
    #define DEFAULT_MEMORY_FREQ (1240000000)
#endif

// for now: These are fixed.
#define IQ_SAMPLE_RATE (48000)
#define AUDIO_SAMPLE_RATE (48000)

// a lot of code pieces assume that this frequency
// is 1500 Hz, so don't change
#define IQ_INTERRUPT_FREQ (1500)

// we process one dma block of samples at once
// block sizes should be a power of two
// a lot of code process information in these blocks
#define IQ_BLOCK_SIZE (IQ_SAMPLE_RATE/IQ_INTERRUPT_FREQ)
#define AUDIO_BLOCK_SIZE (AUDIO_SAMPLE_RATE/IQ_INTERRUPT_FREQ)

// use for clocking based on DMA IRQ
#define SAMPLES_PER_DMA_CYCLE   (IQ_BLOCK_SIZE)
#define SAMPLES_PER_CENTISECOND (IQ_SAMPLE_RATE/100)


#ifdef STM32F4
    #define USE_SIMPLE_FREEDV_FILTERS
    #define USE_FREEDV_1600
#else
    #define USE_FREEDV_700D
#endif


#if (IQ_SAMPLE_RATE) != 48000
    #error Only 48k sample frequency supported (yet).
#endif
#if (IQ_BLOCK_SIZE * 1500) != IQ_SAMPLE_RATE
    #error Audio Interrupt Frequency must be 1500.
#endif
#if (IQ_SAMPLE_RATE/IQ_BLOCK_SIZE) != (AUDIO_SAMPLE_RATE/AUDIO_BLOCK_SIZE)
    #error IQ Interrupt frequency must be idential to Audio Interrupt Frequency
#endif


//******************************CONFIGURATION_LOGIC_CHECKS************************************//

#if !defined(USE_OSC_SI570) && !defined(USE_OSC_SI5351A)
    #error At least one of supported oscillators should be enabled.
#endif

#if !defined(USE_PENDSV_FOR_HIGHPRIO_TASKS) && defined(USE_HIGH_PRIO_PTT)
#error USE_HIGH_PRIO_PTT requires USE_PENDSV_FOR_HIGHPRIO_TASKS
#endif

#if defined(USE_32_IQ_BITS) && CODEC_NUM == 1
    #define USE_32_AUDIO_BITS
#endif

#if !defined(USE_GFX_ILI932x) && !defined(USE_GFX_ILI9486)
#warning Both USE_GFX_ILI932x and USE_GFX_ILI9486 are disabled, no display driver will be available!
#endif

#if defined(UI_BRD_MCHF) && defined(USE_TWO_CHANNEL_AUDIO)
#error UI_BRD_MCHF does not permit USE_TWO_CHANNEL_AUDIO
#endif

#if CODEC_NUM == 1 && (defined(USE_32_IQ_BITS) &&  !defined(USE_32_AUDIO_BITS)) || (!defined(USE_32_IQ_BITS) &&  defined(USE_32_AUDIO_BITS))
#error With only one codec bit width of iq and audio must match, either define both USE_32_IQ_BITS and USE_32_AUDIO_BITS or none
#endif

#endif
