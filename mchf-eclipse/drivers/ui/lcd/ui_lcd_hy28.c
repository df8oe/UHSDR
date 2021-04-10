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
 **  Licence:      GNU GPLv3                                                        **
 ************************************************************************************/

// Common
#include <stdio.h>
#include <stdlib.h>
#include "uhsdr_board.h"
#include "uhsdr_board_config.h"
#include "ui_lcd_hy28.h"

#include "ui_lcd_ra8875.h"
#include "ui_lcd_draw.h"

#define hspiDisplay hspi2
#define SPI_DISPLAY SPI2

#ifndef STM32H7
  // FIXME: H7 Port, re-enable DMA once SPI display is working
  #define USE_SPI_DMA
#endif

#if defined(STM32F7) || defined(STM32H7)
    #define USE_SPI_HAL
#endif


#include "spi.h"

#ifdef USE_DISPLAY_PAR

    #ifdef UI_BRD_OVI40
        #include "fmc.h"
        #define MEM_Init() MX_FMC_Init()
    #else
        #include "fsmc.h"
        #define MEM_Init() MX_FSMC_Init()
    #endif


    #define LCD_REG      (*((volatile unsigned short *) 0x60000000))
    #if defined(UI_BRD_MCHF)
    #define LCD_RAM      (*((volatile unsigned short *) 0x60020000))
    #elif defined(UI_BRD_OVI40)
    #define LCD_RAM      (*((volatile unsigned short *) 0x60004000))
    #endif
#endif



#ifdef TimeDebug
#define MARKER LCD_D0
#define MARKER_PIO LCD_D0_PIO

#define Marker_ON  MARKER_PIO->BSRR=MARKER;
#define Marker_OFF MARKER_PIO->BSRR=MARKER<<16;
#endif

#define SPI_START   (0x70)              /* Start byte for SPI transfer        */
#define SPI_RD      (0x01)              /* WR bit 1 within start              */
#define SPI_WR      (0x00)              /* WR bit 0 within start              */
#define SPI_DATA    (0x02)              /* RS bit 1 within start byte         */
#define SPI_INDEX   (0x00)              /* RS bit 0 within start byte         */

#define SPI_TIMEOUT 100

mchf_display_t mchf_display;

extern const uhsdr_display_info_t display_infos[];

const uhsdr_display_info_t* UiLcd_DisplayInfoGet(mchf_display_types_t display_type)
{
    const uhsdr_display_info_t* retval = NULL;
    for (int i=DISPLAY_NUM-1; i && retval == NULL; i--)
    {
        if (display_type == display_infos[i].display_type)
        {
            retval = &display_infos[i];
        }
    }
    return retval;
}

#ifdef USE_GFX_ILI932x
static const RegisterValue_t ili9320[] =
{
        { 0xE5, 0x8000},   // Set the internal vcore voltage
        { REGVAL_DELAY,  0x0001},    // Start internal OSC.

        // Direction related
        { 0x01, 0x0100},    // set SS and SM bit

        { 0x02, 0x0700},    // set 1 line inversionc
        { 0x03, 0x1038},    // set GRAM write direction and BGR=1 and ORG = 1.
        { 0x04, 0x0000},    // Resize register
        { 0x08, 0x0202},    // set the back porch and front porch
        { 0x09, 0x0000},    // set non-display area refresh cycle ISC[3:0]
        { 0x0A, 0x0000},    // FMARK function
        { 0x0C, 0x0000},    // RGB interface setting
        { 0x0D, 0x0000},    // Frame marker Position
        { 0x0F, 0x0000},    // RGB interface polarity

        // Power On sequence
        { 0x10, 0x0000},    // SAP, BT[3:0], AP, DSTB, SLP, STB
        { 0x11, 0x0000},    // DC1[2:0], DC0[2:0], VC[2:0]
        { 0x12, 0x0000},    // VREG1OUT voltage
        { 0x13, 0x0000},    // VDV[4:0] for VCOM amplitude
        { REGVAL_DELAY, 300},            // Dis-charge capacitor power voltage (300ms)
        { 0x10, 0x17B0},    // SAP, BT[3:0], AP, DSTB, SLP, STB
        { 0x11, 0x0137},    // DC1[2:0], DC0[2:0], VC[2:0]
        { REGVAL_DELAY, 100},             // Delay 100 ms
        { 0x12, 0x0139},    // VREG1OUT voltage
        { REGVAL_DELAY, 100},             // Delay 100 ms
        { 0x13, 0x1d00},    // VDV[4:0] for VCOM amplitude
        { 0x29, 0x0013},    // VCM[4:0] for VCOMH
        { REGVAL_DELAY, 100},             // Delay 100 ms
        { 0x20, 0x0000},    // GRAM horizontal Address
        { 0x21, 0x0000},    // GRAM Vertical Address

        // Adjust the Gamma Curve
        { 0x30, 0x0007},
        { 0x31, 0x0007},
        { 0x32, 0x0007},
        { 0x35, 0x0007},
        { 0x36, 0x0007},
        { 0x37, 0x0700},
        { 0x38, 0x0700},
        { 0x39, 0x0700},
        { 0x3C, 0x0700},
        { 0x3D, 0x1F00},

        // Set GRAM area
        { 0x50, 0x0000},    // Horizontal GRAM Start Address
        { 0x51, 0x00EF},    // Horizontal GRAM End Address
        { 0x52, 0x0000},    // Vertical GRAM Start Address
        { 0x53, 0x013F},    // Vertical GRAM End Address

        // Direction related
        { 0x60, 0xA700},    // Gate Scan Line
        { 0x61, 0x0001},    // NDL,VLE, REV

        { 0x6A, 0x0000},    // set scrolling line

        // Partial Display Control
        { 0x80, 0x0000},
        { 0x81, 0x0000},
        { 0x82, 0x0000},
        { 0x83, 0x0000},
        { 0x84, 0x0000},
        { 0x85, 0x0000},

        // Panel Control
        { 0x90, 0x0010},
        { 0x92, 0x0000},
        { 0x93, 0x0003},
        { 0x95, 0x0110},
        { 0x97, 0x0000},
        { 0x98, 0x0000},

        // Set GRAM write direction
        { 0x03, 0x1038},

        // 262K color and display ON
        { 0x07, 0x0173},

        // delay 50 ms
        { REGVAL_DELAY, 50},

};

static const RegisterValueSetInfo_t ili9320_regs =
{
    ili9320, sizeof(ili9320)/sizeof(RegisterValue_t)
};

static const RegisterValue_t spdfd5408b[] =
{

    { 0x01, 0x0000},   // (SS bit 8) - 0x0100 will flip 180 degree
    { 0x02, 0x0700},   // LCD Driving Waveform Contral
    { 0x03, 0x1038},   // Entry Mode (AM bit 3)

    { 0x04, 0x0000},   // Scaling Control register
    { 0x08, 0x0207},   // Display Control 2
    { 0x09, 0x0000},   // Display Control 3
    { 0x0A, 0x0000},   // Frame Cycle Control
    { 0x0C, 0x0000},   // External Display Interface Control 1
    { 0x0D, 0x0000},    // Frame Maker Position
    { 0x0F, 0x0000},   // External Display Interface Control 2
    { REGVAL_DELAY, 50},
    { 0x07, 0x0101},   // Display Control
    { REGVAL_DELAY, 50},
    { 0x10, 0x16B0},    // Power Control 1
    { 0x11, 0x0001},    // Power Control 2
    { 0x17, 0x0001},    // Power Control 3
    { 0x12, 0x0138},    // Power Control 4
    { 0x13, 0x0800},    // Power Control 5
    { 0x29, 0x0009},   // NVM read data 2
    { 0x2a, 0x0009},   // NVM read data 3
    { 0xa4, 0x0000},
    { 0x50, 0x0000},
    { 0x51, 0x00EF},
    { 0x52, 0x0000},
    { 0x53, 0x013F},

    { 0x60, 0x2700},   // Driver Output Control (GS bit 15)

    { 0x61, 0x0003},   // Driver Output Control
    { 0x6A, 0x0000},   // Vertical Scroll Control

    { 0x80, 0x0000},   // Display Position ?C Partial Display 1
    { 0x81, 0x0000},   // RAM Address Start ?C Partial Display 1
    { 0x82, 0x0000},   // RAM address End - Partial Display 1
    { 0x83, 0x0000},   // Display Position ?C Partial Display 2
    { 0x84, 0x0000},   // RAM Address Start ?C Partial Display 2
    { 0x85, 0x0000},   // RAM address End ?C Partail Display2
    { 0x90, 0x0013},   // Frame Cycle Control
    { 0x92, 0x0000},    // Panel Interface Control 2
    { 0x93, 0x0003},   // Panel Interface control 3
    { 0x95, 0x0110},   // Frame Cycle Control
    { 0x07, 0x0173},
};


static const RegisterValueSetInfo_t spdfd5408b_regs =
{
    spdfd5408b, sizeof(spdfd5408b)/sizeof(RegisterValue_t)
};
#ifdef USE_GFX_SSD1289
static const RegisterValue_t ssd1289[] =
{
        {0x00,0x0001},
        {0x03,0xA8A4},
        {0x0C,0x0000},
        {0x0D,0x080C},
        {0x0E,0x2B00},
        {0x1E,0x00B7},
        {0x01,0x2B3F},
        {0x02,0x0600},
        {0x10,0x0000},
        {0x11,0x6070},
        {0x05,0x0000},
        {0x06,0x0000},
        {0x16,0xEF1C},
        {0x17,0x0003},
        {0x07,0x0233},
        {0x0B,0x0000},
        {0x0F,0x0000},
        {0x41,0x0000},
        {0x42,0x0000},
        {0x48,0x0000},
        {0x49,0x013F},
        {0x4A,0x0000},
        {0x4B,0x0000},
        {0x44,0xEF00},
        {0x45,0x0000},
        {0x46,0x013F},
        {0x30,0x0707},
        {0x31,0x0204},
        {0x32,0x0204},
        {0x33,0x0502},
        {0x34,0x0507},
        {0x35,0x0204},
        {0x36,0x0204},
        {0x37,0x0502},
        {0x3A,0x0302},
        {0x3B,0x0302},
        {0x23,0x0000},
        {0x24,0x0000},
        {0x25,0x8000},
        {0x4f,0x0000},
        {0x4e,0x0000},
};

static const RegisterValueSetInfo_t ssd1289_regs =
{
    ssd1289, sizeof(ssd1289)/sizeof(RegisterValue_t)
};
#endif


static const RegisterValue_t ili932x[] =
{
    // NPI: { 0xE5, 0x78F0},   // set SRAM internal timing I guess this is the relevant line for getting LCDs to work which are "out-of-specs"...
    { 0x01, 0x0000},    // set SS and SM bit
    { 0x02, 0x0700},    // set 1 line inversion
    { 0x03, 0x1038},    // set GRAM write direction and BGR=1 and ORG = 1
    { 0x04, 0x0000},    // resize register
    { 0x08, 0x0207},    // set the back porch and front porch
    { 0x09, 0x0000},    // set non-display area refresh cycle
    { 0x0a, 0x0000},    // FMARK function
    { 0x0c, 0x0001},    // RGB interface setting
    // NPI: { 0x0c, 0x0000},    // RGB interface setting
    { 0x0d, 0x0000},    // frame marker position
    { 0x0f, 0x0000},    // RGB interface polarity

    // Power On sequence
    { 0x10, 0x0000},    // SAP, BT[3:0], AP, DSTB, SLP, STB
    { 0x11, 0x0007},    // DC1[2:0], DC0[2:0], VC[2:0]
    { 0x12, 0x0000},    // VREG1OUT voltage
    { 0x13, 0x0000},    // VDV[4:0] for VCOM amplitude
    // NPI: { 0x0c, 0x0001},    // RGB interface setting
    { REGVAL_DELAY, 200},         // delay 200 ms
    { 0x10, 0x1590},    // SAP, BT[3:0], AP, DSTB, SLP, STB
    // NPI: { 0x10, 0x1090}, // SAP, BT[3:0], AP, DSTB, SLP, STB
    { 0x11, 0x0227},    // set DC1[2:0], DC0[2:0], VC[2:0]
    { REGVAL_DELAY, 50},              // delay 50 ms
    { 0x12, 0x009c},    // internal reference voltage init
    // NPI: { 0x12, 0x001F},
    { REGVAL_DELAY, 50},              // delay 50 ms
    { 0x13, 0x1900},    // set VDV[4:0] for VCOM amplitude
    // NPI: { 0x13, 0x1500},
    { 0x29, 0x0023},    // VCM[5:0] for VCOMH
    // NPI: { 0x29, 0x0027},    // VCM[5:0] for VCOMH
    { 0x2b, 0x000d},    // set frame rate: changed from 0e to 0d on 03/28/2016
    { REGVAL_DELAY, 50},              // delay 50 ms
    { 0x20, 0x0000},    // GRAM horizontal address
    { 0x21, 0x0000},    // GRAM vertical address

//        /* NPI:
     // ----------- Adjust the Gamma Curve ----------
    { 0x30, 0x0000},
    { 0x31, 0x0707},
    { 0x32, 0x0307},
    { 0x35, 0x0200},
    { 0x36, 0x0008},
    { 0x37, 0x0004},
    { 0x38, 0x0000},
    { 0x39, 0x0707},
    { 0x3C, 0x0002},
    { 0x3D, 0x1D04},
//        */

    { 0x50, 0x0000},    // horizontal GRAM start address
    { 0x51, 0x00ef},    // horizontal GRAM end address
    { 0x52, 0x0000},    // vertical GRAM start address
    { 0x53, 0x013f},    // vertical GRAM end address
    { 0x60, 0xa700},    // gate scan line
    { 0x61, 0x0001},    // NDL, VLE, REV
    { 0x6a, 0x0000},    // set scrolling line
    // partial display control
    { 0x80, 0x0000},
    { 0x81, 0x0000},
    { 0x82, 0x0000},
    { 0x83, 0x0000},
    { 0x84, 0x0000},
    { 0x85, 0x0000},
    // panel control
    { 0x90, 0x0010},
    { 0x92, 0x0000},
    // NPI: { 0x92, 0x0600},
    // activate display using 262k colours
    { 0x07, 0x0133},
};

static const RegisterValueSetInfo_t ili932x_regs =
{
    ili932x, sizeof(ili932x)/sizeof(RegisterValue_t)
};


#endif
 


#ifdef USE_GFX_ILI9486
static const RegisterValue_t ili9486[] =
{
		{ 0xB0,0},

		{ 0x11,0},
		{ REGVAL_DELAY, 250},

		{ 0x3A, 0x55},      // COLMOD_PIXEL_FORMAT_SET 16 bit pixel

		{ 0xC0, 0x0f},
		{ REGVAL_DATA, 0x0f},

		{ 0xC1, 0x42},
		{ 0xC2, 0x22},

		{ 0xC5, 0x01},
		{ REGVAL_DATA, 0x29}, //4D
		{ REGVAL_DATA, 0x80},

		{ 0xB6, 0x00},
		{ REGVAL_DATA, 0x02}, //42
		{ REGVAL_DATA, 0x3b},

		{ 0xB1, 0xB0},
		{ REGVAL_DATA, 0x11},

		{ 0xB4, 0x02},

		{ 0xE0, 0x0f},
		{ REGVAL_DATA, 0x1F},
		{ REGVAL_DATA, 0x1C},
		{ REGVAL_DATA, 0x0C},
		{ REGVAL_DATA, 0x0F},
		{ REGVAL_DATA, 0x08},
		{ REGVAL_DATA, 0x48},
		{ REGVAL_DATA, 0x98},
		{ REGVAL_DATA, 0x37},
		{ REGVAL_DATA, 0x0a},
		{ REGVAL_DATA, 0x13},
		{ REGVAL_DATA, 0x04},
		{ REGVAL_DATA, 0x11},
		{ REGVAL_DATA, 0x0d},
		{ REGVAL_DATA, 0x00},

		{ 0xE1, 0x0f},
		{ REGVAL_DATA, 0x32},
		{ REGVAL_DATA, 0x2e},
		{ REGVAL_DATA, 0x0b},
		{ REGVAL_DATA, 0x0d},
		{ REGVAL_DATA, 0x05},
		{ REGVAL_DATA, 0x47},
		{ REGVAL_DATA, 0x75},
		{ REGVAL_DATA, 0x37},
		{ REGVAL_DATA, 0x06},
		{ REGVAL_DATA, 0x10},
		{ REGVAL_DATA, 0x03},
		{ REGVAL_DATA, 0x24},
		{ REGVAL_DATA, 0x20},
		{ REGVAL_DATA, 0x00},

		{ 0xE2, 0x0f},
		{ REGVAL_DATA, 0x32},
		{ REGVAL_DATA, 0x2e},
		{ REGVAL_DATA, 0x0b},
		{ REGVAL_DATA, 0x0d},
		{ REGVAL_DATA, 0x05},
		{ REGVAL_DATA, 0x47},
		{ REGVAL_DATA, 0x75},
		{ REGVAL_DATA, 0x37},
		{ REGVAL_DATA, 0x06},
		{ REGVAL_DATA, 0x10},
		{ REGVAL_DATA, 0x03},
		{ REGVAL_DATA, 0x24},
		{ REGVAL_DATA, 0x20},		{ REGVAL_DATA, 0x00},

		{ 0x13, 0x00},    //normal display mode ON
		{ 0x20, 0x00},    //display inversion off

		{ 0x36, 0x028},

		{ 0x11, 0x00},
		{ REGVAL_DELAY, 250},

		{ 0x29, 0x00},
		{ REGVAL_DELAY, 250},
};

static const RegisterValueSetInfo_t ili9486_regs =
{
    ili9486, sizeof(ili9486)/sizeof(RegisterValue_t)
};

#endif

static void UiLcd_BacklightInit()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // Set as output
    GPIO_InitStructure.Pin = LCD_BACKLIGHT;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_BACKLIGHT_PIO, &GPIO_InitStructure);

    // Backlight off
    GPIO_ResetBits(LCD_BACKLIGHT_PIO, LCD_BACKLIGHT);
}

void UiLcd_BacklightEnable(bool on)
{
    if (on)
    {
        GPIO_SetBits(LCD_BACKLIGHT_PIO, LCD_BACKLIGHT);
    }
    else
    {
        GPIO_ResetBits(LCD_BACKLIGHT_PIO, LCD_BACKLIGHT);
    }
}

/*
 * This handler creates a software pwm for the LCD backlight. It needs to be called
 * very regular to work properly. Right now it is activated from the audio interrupt
 * at a rate of 1.5khz The rate itself is not too critical,
 * just needs to be high and very regular.
 */

#define LCD_DIMMING_PWM_COUNTS 16

void UiLcd_BacklightDimHandler()
{
    static uchar lcd_dim = 0;
    static const uint16_t dimming_pattern_map[1 + LCD_DIMMING_LEVEL_MAX - LCD_DIMMING_LEVEL_MIN] =
    {
            0xffff, // 16/16
            0x3f3f, // 12/16
            0x0f0f, // 8/16
            0x0303, // 4/16
            0x0101, // 2/16
            0x0001, // 1/1
    };
    // most of the patterns generate a 1500/8 =  187.5 Hz noise, lowest 1500/16 = 93.75 Hz.

    static uint16_t dim_pattern = 0xffff; // gives us the maximum brightness

    if(!ts.lcd_blanking_flag)       // is LCD *NOT* blanked?
    {
        if (lcd_dim == 0 )
        {
            dim_pattern = dimming_pattern_map[ts.lcd_backlight_brightness - LCD_DIMMING_LEVEL_MIN];
        }

        // UiLcdHy28_BacklightEnable(lcd_dim >= dimming_map[ts.lcd_backlight_brightness - LCD_DIMMING_LEVEL_MIN]);   // LCD backlight off or on
        UiLcd_BacklightEnable((dim_pattern & 0x001) == 1);   // LCD backlight off or on

        dim_pattern >>=1;
        lcd_dim++;
        lcd_dim %= LCD_DIMMING_PWM_COUNTS;   // limit brightness PWM count to 0-3
    }
    else if(!ts.menu_mode)
    { // LCD is to be blanked - if NOT in menu mode
        UiLcd_BacklightEnable(false);
    }
}


#ifdef USE_DISPLAY_PAR
static void UiLcd_ParallelInit()
{
    MEM_Init();
}

static void UiLcd_ParallelDeInit()
{
    HAL_SRAM_DeInit(&hsram1);

}
#endif

void UiLcd_GpioInit(mchf_display_types_t display_type)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    // Common misc pins settings
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Pull = GPIO_NOPULL;


    if (mchf_display.lcd_cs_pio != NULL)
    {
        // Configure GPIO PIN for Chip select
        GPIO_InitStructure.Pin = mchf_display.lcd_cs;
        HAL_GPIO_Init(mchf_display.lcd_cs_pio, &GPIO_InitStructure);

        // Deselect : Chip Select high
        GPIO_SetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
    }
    // Configure GPIO PIN for Reset
    GPIO_InitStructure.Pin = LCD_RESET;
    HAL_GPIO_Init(LCD_RESET_PIO, &GPIO_InitStructure);

    // TODO: Function Gets Display Type (!) not controller as parameter
#ifdef USE_GFX_ILI9486
    if (display_type == DISPLAY_RPI_SPI)
    {

        // Configure GPIO PIN for RS
        GPIO_InitStructure.Pin = LCD_RS;
        HAL_GPIO_Init(LCD_RS_PIO, &GPIO_InitStructure);
    }
#endif

#ifdef TimeDebug
    //Configure GPIO pin for routine time optimization (for scope probe)
    GPIO_InitStructure.Pin = MARKER;
    HAL_GPIO_Init(MARKER_PIO, &GPIO_InitStructure);
#endif

}




static void UiLcd_Reset()
{
    // Reset
    GPIO_SetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(1);

    GPIO_ResetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(1);

    GPIO_SetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(300);
}

#ifdef STM32F4
    #define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_4)
    #define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_2)
#endif

#ifdef STM32F7
    #define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_8)
    #define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_4)
#endif

#ifdef STM32H7
    #define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_8)
    #define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_4)
#endif

inline bool UiLcd_LcdSpiUsed()
{
    bool retval = false;
#ifdef USE_DISPLAY_SPI
    retval = mchf_display.use_spi;
#endif
    return retval;
}

void UiLcd_SpiSetPrescaler(uint32_t baudrate_prescaler)
{
    /*---------------------------- SPIx CR1 Configuration ------------------------*/
    // the baud rate register differs across the different processors
#if defined(STM32H7)
    #define SPI_BR_REG CFG1
#elif defined(STM32F4) || defined(STM32F7)
    #define SPI_BR_REG CR1
#endif

    /* Get the SPIx SPI_BR_REG value */
    uint32_t tmpreg = SPI_DISPLAY->SPI_BR_REG;
    tmpreg &= ~SPI_BAUDRATEPRESCALER_256;
    tmpreg |= baudrate_prescaler;

    /* Write to SPIx CR1 */
    SPI_DISPLAY->SPI_BR_REG = tmpreg;
}



// static SPI_HandleTypeDef SPI_Handle;
void UiLcd_SpiInit(bool hispeed, mchf_display_types_t display_type)
{
    mchf_display.lcd_spi_prescaler = hispeed?SPI_PRESCALE_LCD_HIGH:SPI_PRESCALE_LCD_DEFAULT;
    hspiDisplay.Init.BaudRatePrescaler = mchf_display.lcd_spi_prescaler;

#ifdef USE_GFX_ILI9486
    if (display_type == DISPLAY_RPI_SPI)
    {
        hspiDisplay.Init.CLKPolarity = SPI_POLARITY_LOW;
        hspiDisplay.Init.CLKPhase = SPI_PHASE_1EDGE;
        hspiDisplay.Init.NSS = SPI_NSS_SOFT;

        if (HAL_SPI_Init(&hspiDisplay) != HAL_OK)
        {
            Error_Handler();
        }

    }
#endif

    // Enable the SPI periph
    // the main init is already done earlier, we need this if we want to use our own code to access SPI
    __HAL_SPI_ENABLE(&hspiDisplay);
}


static inline void UiLcd_SpiDmaStop()
{
    while (DMA1_Stream4->CR & DMA_SxCR_EN) { asm(""); }
}



void UiLcd_SpiDmaStart(uint8_t* buffer, uint32_t size)
{

    // do busy waiting here. This is just for testing if everything goes according to plan
    // if this works okay, we can let SPI DMA running while doing something else
    // and just check before next transfer if DMA is being done.
    // and finally we can move that into an interrupt, of course.
    if (size > 0)  {
        UiLcd_SpiDmaStop();
        HAL_SPI_Transmit_DMA(&hspiDisplay,buffer,size);
    }
}

void UiLcd_SpiDeInit()
{

    // __HAL_SPI_DISABLE(&hspiDisplay);
    // HAL_SPI_DeInit(&hspiDisplay);

    GPIO_InitTypeDef GPIO_InitStructure;

    // Set as inputs
    GPIO_InitStructure.Mode		= GPIO_MODE_INPUT;
    GPIO_InitStructure.Speed	= GPIO_SPEED_FREQ_LOW;
    GPIO_InitStructure.Pull		= GPIO_NOPULL;

    if (mchf_display.lcd_cs_pio != NULL)
    {
        // Deconfigure GPIO PIN for Chip select
        GPIO_InitStructure.Pin		= mchf_display.lcd_cs;
        HAL_GPIO_Init(mchf_display.lcd_cs_pio, &GPIO_InitStructure);
    }
}

static inline void UiLcd_SpiLcdCsDisable()
{
    GPIO_SetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}

void UiLcd_SpiLcdCsEnable()
{
    GPIO_ResetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}

void UiLcd_SpiSendByte(uint8_t byte)
{

#ifdef USE_SPI_HAL
    uint8_t dummy;
    HAL_SPI_TransmitReceive(&hspi2, &byte, &dummy,1,SPI_TIMEOUT);
#else
    while ((SPI_DISPLAY->SR & (SPI_FLAG_TXE)) == (uint16_t)RESET) {}
    SPI_DISPLAY->DR = byte;
    while ((SPI_DISPLAY->SR & (SPI_FLAG_RXNE)) == (uint16_t)RESET) {}
    byte = SPI_DISPLAY->DR;
#endif
}
/*
static inline void UiLcdHy28_SpiSendByteFast(uint8_t byte)
{

    while ((SPI_DISPLAY->SR & (SPI_FLAG_TXE)) == (uint16_t)RESET) {}
    SPI_DISPLAY->DR = byte;
    while ((SPI_DISPLAY->SR & (SPI_FLAG_RXNE)) == (uint16_t)RESET) {}
    byte = SPI_DISPLAY->DR;
}*/


void UiLcd_SpiFinishTransfer()
{
#ifdef STM32H7
    #ifndef USE_SPI_HAL
    static volatile uint8_t spi_dr_dummy; // used to make sure that DR is being read
    // we cannot use this with HAL, the "normal" HAL Transmit does check the flags AND resets them (ARGH)
    while (__HAL_SPI_GET_FLAG(&hspiDisplay, SPI_SR_EOT) == 0 || __HAL_SPI_GET_FLAG(&hspiDisplay, SPI_SR_EOT) == 0 ) { asm("nop"); }
    while (__HAL_SPI_GET_FLAG(&hspiDisplay, SPI_FLAG_RXWNE) != 0 || __HAL_SPI_GET_FLAG(&hspiDisplay, SPI_SR_RXPLVL) != 0 )
    {
        spi_dr_dummy = SPI_DISPLAY->RXDR;
    }
    #endif
#else
    static volatile uint8_t spi_dr_dummy; // used to make sure that DR is being read
    while ((SPI_DISPLAY->SR & (SPI_FLAG_TXE)) == (uint16_t)RESET) {}
    while (SPI_DISPLAY->SR & SPI_FLAG_BSY) {}
    if (SPI_DISPLAY->SR & SPI_FLAG_RXNE)
    {
        spi_dr_dummy = SPI_DISPLAY->DR;
    }
#endif
}

void UiLcd_LcdSpiFinishTransfer()
{
    UiLcd_SpiFinishTransfer();
    GPIO_SetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}

uint8_t UiLcd_SpiReadByte()
{
    uint8_t dummy = 0;
    uint8_t retval = 0;

    /* Send a Transmit a dummy byte and Receive Byte through the SPI peripheral */
    HAL_SPI_TransmitReceive(&hspi2, &dummy,&retval,1,SPI_TIMEOUT);

    return retval;
}

uint8_t UiLcd_SpiReadByteFast()
{
    uint8_t retval = 0;

#ifdef USE_SPI_HAL
    uint8_t dummy = 0;
    HAL_SPI_TransmitReceive(&hspi2, &dummy, &retval,1,SPI_TIMEOUT);
#else

    /* Send a Transmit a dummy byte and Receive Byte through the SPI peripheral */
    while ((SPI_DISPLAY->SR & (SPI_FLAG_TXE)) == (uint16_t)RESET) {}
    SPI_DISPLAY->DR = 0;
    while ((SPI_DISPLAY->SR & (SPI_FLAG_RXNE)) == (uint16_t)RESET) {}
    retval = SPI_DISPLAY->DR;
#endif
    return retval;
}

uint8_t UiLcd_LcdSpiWrite8Read8(uint8_t data)
{
    uint8_t retval;

    UiLcd_SpiLcdCsEnable();
    UiLcd_SpiSendByte(data);
    retval = UiLcd_SpiReadByte();
    UiLcd_LcdSpiFinishTransfer();

    return retval;
}
void UiLcd_LcdSpiWrite16(uint16_t data)
{
    UiLcd_SpiLcdCsEnable();
    UiLcd_SpiSendByte(data >> 8);
    UiLcd_SpiSendByte(data & 0xff);
    UiLcd_LcdSpiFinishTransfer();
}
// TODO: Function Per Controller Group
void UiLcd_WriteIndexSpi(unsigned char index)
{
    UiLcd_SpiLcdCsEnable();

    mchf_display.WriteIndexSpi_Prepare();
    UiLcd_SpiSendByte(index);

    UiLcd_LcdSpiFinishTransfer();
}


// TODO: Function Per Controller Group
static inline void UiLcd_WriteDataSpiStart()
{
    UiLcd_SpiLcdCsEnable();

    mchf_display.WriteDataSpiStart_Prepare();
}

void UiLcd_WriteDataSpi( unsigned short data)
{
    UiLcd_WriteDataSpiStart();
    UiLcd_SpiSendByte((data >>   8));                    /* Write D8..D15                */
    UiLcd_SpiSendByte((data & 0xFF));                    /* Write D0..D7   */
    UiLcd_LcdSpiFinishTransfer();
}

static inline void UiLcd_WriteDataOnly( unsigned short data)
{
	if(UiLcd_LcdSpiUsed())
    {
        UiLcd_SpiSendByte((data >>   8));      /* Write D8..D15                */
        UiLcd_SpiSendByte((data & 0xFF));      /* Write D0..D7                 */
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        LCD_RAM = data;
#endif
    }
}

static inline void UiLcd_WriteData( unsigned short data)
{
	if(UiLcd_LcdSpiUsed())
    {
        UiLcd_WriteDataSpiStart();
        UiLcd_SpiSendByte((data >>   8));                    /* Write D8..D15                */
        UiLcd_SpiSendByte((data & 0xFF));                    /* Write D0..D7   */
        UiLcd_LcdSpiFinishTransfer();
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        LCD_RAM = data;
#endif
    }
}

unsigned short UiLcd_LcdReadDataSpi()
{
    unsigned short value = 0;
    uchar y,z;

    UiLcd_SpiLcdCsEnable();

    UiLcd_SpiSendByte(SPI_START | SPI_RD | SPI_DATA);    /* Read: RS = 1, RW = 1         */

    UiLcd_SpiReadByte();                                /* Dummy read 1                 */

    y   = UiLcd_SpiReadByte();                      /* Read D8..D15                 */
    value = y;
    value <<= 8;
    z = UiLcd_SpiReadByte();                      /* Read D0..D7                  */

    value  |= z;

    UiLcd_LcdSpiFinishTransfer();

    return value;
}


/**
 * @brief writes a controller register in its native width 16 bit or 8bit
 * width is controller dependent (RA8875 uses 8bit, all other 16bit)
 */
void UiLcd_WriteReg(unsigned short LCD_Reg, unsigned short LCD_RegValue)
{
	mchf_display.WriteReg(LCD_Reg,LCD_RegValue);
}

void UiLcd_WriteRAM_Prepare_Index(uint16_t wr_prep_reg)
{
    if(UiLcd_LcdSpiUsed())
    {
        UiLcd_WriteIndexSpi(wr_prep_reg);
        UiLcd_WriteDataSpiStart();
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        LCD_REG = wr_prep_reg;
#endif
    }
}

static void UiLcd_BulkWrite(uint16_t* pixel, uint32_t len)
{

// if we are not using SPI DMA, we send the data as it comes
// if we are using SPI DMA, we do this only if we are NOT using SPI
#ifdef USE_SPI_DMA
    if(UiLcd_LcdSpiUsed() == false)
#endif
    {
        for (uint32_t i = len; i; i--)
        {
            UiLcd_WriteDataOnly(*(pixel++));
        }
    }
#ifdef USE_SPI_DMA
    else
    {
        for (uint32_t i = 0; i < len; i++)
        {
            pixel[i] = __REV16(pixel[i]); // reverse byte order;
        }
        UiLcd_SpiDmaStart((uint8_t*)pixel,len*2);
    }
#endif

}

void UiLcd_BulkPixel_FinishWaitWrite()
{
    if(UiLcd_LcdSpiUsed())         // SPI enabled?
    {
#ifdef USE_SPI_DMA
        UiLcd_SpiDmaStop();
#endif
        UiLcd_LcdSpiFinishTransfer();
    }
}

static void UiLcd_OpenBulkWrite(ushort x, ushort width, ushort y, ushort height)
{
    UiLcd_BulkPixel_FinishWaitWrite();
    mchf_display.SetActiveWindow(x, x + width - 1, y, y + height - 1);
    mchf_display.SetCursorA(x, y);
    mchf_display.WriteRAM_Prepare();
}

static void UiLcd_CloseBulkWrite()
{
#ifdef USE_GFX_RA8875
	if(mchf_display.DeviceCode==0x8875)
	{
	    UiLcd_BulkPixel_FinishWaitWrite();
		uint16_t MAX_X=mchf_display.MAX_X; uint16_t MAX_Y=mchf_display.MAX_Y;
		mchf_display.SetActiveWindow(0, MAX_X - 1, 0, MAX_Y - 1);
		UiLcd_WriteReg(0x40, 0);
	}
#endif
}

#define PIXELBUFFERSIZE 512
#define PIXELBUFFERCOUNT 2

static __UHSDR_DMAMEM uint16_t   pixelbuffer[PIXELBUFFERCOUNT][PIXELBUFFERSIZE];
static uint16_t pixelcount = 0;
static uint16_t pixelbufidx = 0;

void UiLcd_BulkPixel_BufferInit()
{
    pixelbufidx= (pixelbufidx+1)%PIXELBUFFERCOUNT;
    pixelcount = 0;
}


void UiLcd_BulkPixel_BufferFlush()
{
    UiLcd_BulkWrite(pixelbuffer[pixelbufidx],pixelcount);
    UiLcd_BulkPixel_BufferInit();
}

void UiLcd_BulkPixel_Put(uint16_t pixel)
{
    pixelbuffer[pixelbufidx][pixelcount++] = pixel;
    if (pixelcount == PIXELBUFFERSIZE)
    {
        UiLcd_BulkPixel_BufferFlush();
    }
}

// TODO: Not most efficient way, we could use remaining buffer size to judge
// if it will fit without flush and fill accordingly.

void UiLcd_BulkPixel_PutBuffer(uint16_t* pixel_buffer, uint32_t len)
{
    // We bypass the buffering if in parallel mode
    // since as for now, it will not benefit from it.
    // this can be changed if someone write DMA code for the parallel
    // interface (memory to memory DMA)
    if(UiLcd_LcdSpiUsed())         // SPI enabled?
    {
        for (uint32_t idx = 0; idx < len; idx++)
        {
            UiLcd_BulkPixel_Put(pixel_buffer[idx]);
        }
    }
    else
    {
        UiLcd_BulkWrite(pixel_buffer, len);
    }
}

void UiLcd_BulkPixel_OpenWrite(ushort x, ushort width, ushort y, ushort height)
{
    UiLcd_OpenBulkWrite(x, width,y,height);
    UiLcd_BulkPixel_BufferInit();
}

inline void UiLcd_BulkPixel_CloseWrite()
{
    UiLcd_BulkPixel_BufferFlush();
    UiLcd_CloseBulkWrite();
}

static void UiLcd_BulkWriteColor(uint16_t Color, uint32_t len)
{

#ifdef USE_SPI_DMA
    if(UiLcd_LcdSpiUsed())
    {
        int idx;

        UiLcd_BulkPixel_BufferInit();

        for (idx = 0; idx < len; idx++)
        {
            UiLcd_BulkPixel_Put(Color);
        }
        UiLcd_BulkPixel_BufferFlush();
    }
    else
#endif
    {
        for (uint32_t i = len; i; i--)
        {
            UiLcd_WriteDataOnly(Color);
        }
    }
}

// The _Generic functions work on the bulk pixel interface and are basically pixel
// by pixel operations, should work on any controller. Some controllers may have
// accelerated operations which should be used instead.

static void UiLcdHy28_DrawColorPoint_Generic(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
    uint16_t MAX_X=mchf_display.MAX_X; uint16_t MAX_Y=mchf_display.MAX_Y;
    if( Xpos < MAX_X && Ypos < MAX_Y )
    {
        UiLcd_OpenBulkWrite(Xpos,1,Ypos,1);
        UiLcd_WriteDataOnly(point);
        UiLcd_CloseBulkWrite();
    }
}


static void UiLcdHy28_DrawFullRect_Generic(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color)
{
    UiLcd_OpenBulkWrite(Xpos, Width, Ypos, Height);
    UiLcd_BulkWriteColor(color,(uint32_t)Height * (uint32_t)Width);
    UiLcd_CloseBulkWrite();
}




static void UiLcdHy28_DrawStraightLine_Generic(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color)
{
    if(Direction == LCD_DIR_VERTICAL)
    {
        UiLcdHy28_DrawFullRect_Generic(x,y,Length,1,color);
    }
    else
    {
        UiLcdHy28_DrawFullRect_Generic(x,y,1,Length,color);
    }
}

/*********************************************************************
 *
 * Controller Specific Functions Go Here (
 * These functions are used via mchf_display.func(...)
 * Each controller gets one single section here, guarded with USE_GFX_...
 *
 * *******************************************************************/
static void UiLcdHy28_WriteReg_ILI_SPI(unsigned short LCD_Reg, unsigned short LCD_RegValue)
{
#ifdef USE_DISPLAY_SPI
    UiLcd_WriteIndexSpi(LCD_Reg);
    UiLcd_WriteDataSpi(LCD_RegValue);
#endif
}

static void UiLcdHy28_WriteReg_ILI_PAR(unsigned short LCD_Reg, unsigned short LCD_RegValue)
{
#ifdef USE_DISPLAY_PAR
    LCD_REG = LCD_Reg;
    LCD_RAM = LCD_RegValue;
#endif
}

static unsigned short UiLcdHy28_ReadRegILI(uint16_t LCD_Reg)
{
    uint16_t retval;
    if(UiLcd_LcdSpiUsed())
    {
        // Write 16-bit Index (then Read Reg)
        UiLcd_WriteIndexSpi(LCD_Reg);
        // Read 16-bit Reg
        retval = UiLcd_LcdReadDataSpi();
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        // Write 16-bit Index (then Read Reg)
        LCD_REG = LCD_Reg;
        // Read 16-bit Reg
        retval = LCD_RAM;
#else
        retval = 0;
#endif
    }
    return retval;
}

#ifdef USE_GFX_ILI9486

static uint16_t UiLcdHy28_ReadDisplayId_ILI9486()
{
    uint16_t retval = 0x9486;

#ifdef USE_DISPLAY_PAR
    // we can't read the id from SPI if it is the dumb RPi SPI
    if (mchf_display.use_spi == false)
    {
        retval = UiLcdHy28_ReadRegILI(0xd3);
        retval = LCD_RAM;    //first dummy read
        retval = (LCD_RAM&0xff)<<8;
        retval |=LCD_RAM&0xff;
    }
#endif
    switch (retval)
    {
    case 0x9486:
    case 0x9488: // ILI9486 - Parallel & Serial interface
        mchf_display.reg_info  = &ili9486_regs;
        break;
    default:
        retval = 0;
    }
    return retval;
}

static inline void UiLcdHy28_WriteDataSpiStart_Prepare_ILI9486()
{
    GPIO_SetBits(LCD_RS_PIO, LCD_RS);
}
static void UiLcdHy28_WriteIndexSpi_Prepare_ILI9486()
{
    GPIO_ResetBits(LCD_RS_PIO, LCD_RS);
    UiLcd_SpiSendByte(0);
}

static void UiLcdHy28_SetCursorA_ILI9486( unsigned short Xpos, unsigned short Ypos )
{
}

static void UiLcdHy28_WriteRAM_Prepare_ILI9486()
{
    UiLcd_WriteRAM_Prepare_Index(0x2c);
}

static void UiLcdHy28_SetActiveWindow_ILI9486(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom)
{
    UiLcd_WriteReg(0x2a,XLeft>>8);
    UiLcd_WriteData(XLeft&0xff);
    UiLcd_WriteData((XRight)>>8);
    UiLcd_WriteData((XRight)&0xff);

    UiLcd_WriteReg(0x2b,YTop>>8);
    UiLcd_WriteData(YTop&0xff);
    UiLcd_WriteData((YBottom)>>8);
    UiLcd_WriteData((YBottom)&0xff);
}
#endif

#ifdef USE_GFX_ILI932x


static uint16_t UiLcdHy28_ReadDisplayId_ILI932x()
{
    uint16_t retval = UiLcdHy28_ReadRegILI(0x00);
    switch (retval)
    {
    case 0x9320: // HY28A - SPI interface only (ILI9320 controller)
        mchf_display.reg_info = &ili9320_regs;
        break;
    case 0x5408: // HY28A - Parallel interface only (SPFD5408B controller)
        mchf_display.reg_info = &spdfd5408b_regs;
        break;
    case 0x9325:
    case 0x9328: // HY28B - Parallel & Serial interface - latest model (ILI9325 & ILI9328 controller)
        mchf_display.reg_info = &ili932x_regs;
        break;
    default:
        retval = 0;
    }
    return retval;
}

static inline void UiLcdHy28_WriteDataSpiStart_Prepare_ILI932x()
{
    UiLcd_SpiSendByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
}

static void UiLcdHy28_WriteIndexSpi_Prepare_ILI932x()
{
    UiLcd_SpiSendByte(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0       */
    UiLcd_SpiSendByte(0);
}

static void UiLcdHy28_SetCursorA_ILI932x( unsigned short Xpos, unsigned short Ypos )
{
    UiLcd_WriteReg(0x20, Ypos );
    UiLcd_WriteReg(0x21, Xpos );
}

static void UiLcdHy28_WriteRAM_Prepare_ILI932x()
{
    UiLcd_WriteRAM_Prepare_Index(0x22);
}

static void UiLcdHy28_SetActiveWindow_ILI932x(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom)
{
    UiLcd_WriteReg(0x52, XLeft);    // Horizontal GRAM Start Address
    UiLcd_WriteReg(0x53, XRight);    // Horizontal GRAM End Address  -1

    UiLcd_WriteReg(0x50, YTop);    // Vertical GRAM Start Address
    UiLcd_WriteReg(0x51, YBottom);    // Vertical GRAM End Address    -1
}
#endif

#ifdef USE_GFX_SSD1289

static uint16_t UiLcdHy28_ReadDisplayId_SSD1289()
{
    uint16_t retval = UiLcdHy28_ReadRegILI(0x00);
    switch (retval)
    {
    case 0x8989: // HY28A - SPI interface only (ILI9320 controller)
        mchf_display.reg_info = &ssd1289_regs;
        break;
    default:
        retval = 0;
    }
    return retval;
}

static void UiLcdHy28_SetActiveWindow_SSD1289(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom)
{
     UiLcd_WriteReg(0x44, XRight << 8 | XLeft);    // Horizontal GRAM Start Address
     UiLcd_WriteReg(0x45, YTop);    // Horizontal GRAM End Address  -1

     UiLcd_WriteReg(0x45, YTop);    // Vertical GRAM Start Address
     UiLcd_WriteReg(0x46, YBottom);    // Vertical GRAM End Address    -1
}
static void UiLcdHy28_SetCursorA_SSD1289( unsigned short Xpos, unsigned short Ypos )
{
    UiLcd_WriteReg(0x4e, Ypos );
    UiLcd_WriteReg(0x4f, Xpos );
}
#endif


// ATTENTION: THIS LIST NEEDS TO HAVE A SPECIFIC ORDER:
// FIRST ALL DETECTABLE DISPLAYS THEN AT MOST ONE SINGLE UNDETECTABLE DISPLAY
//
// IF A DISPLAY DOES NOT HAVE A REAL DETECTION ROUTINE
// ALL DISPLAYS BEHIND THIS ONE IN THIS LIST WILL NEVER BE TESTED!
//
// Please note that the CubeMX generated code for FSMC/FMC init
// no multiple on/off cycles permits. Small change in fmc.c/fsmc.c fixes that
// so if during a new STM controller port the second or third enabled parallel display
// in the list below is not working, check if the code in these files
// and see if Initialized and DeInitialized variables are BOTH (!) set accordingly.
// When one is set, the other has to be cleared.
//

const uhsdr_display_info_t display_infos[] = {
        {
                DISPLAY_NONE,  "No Display", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0
        },
#ifdef USE_DISPLAY_PAR
#ifdef USE_GFX_ILI9486
        {
                DISPLAY_ILI9486_PARALLEL, "ILI9486 Para.",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_ILI9486,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_ILI9486,
                .SetCursorA = UiLcdHy28_SetCursorA_ILI9486,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI9486,
				.WriteReg = UiLcdHy28_WriteReg_ILI_PAR,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_Generic,
				.DrawFullRect = UiLcdHy28_DrawFullRect_Generic,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_Generic,
        },
#endif
#ifdef USE_GFX_ILI932x
        {
                DISPLAY_HY28B_PARALLEL, "HY28A/B Para.",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_ILI932x,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_ILI932x,
                .SetCursorA = UiLcdHy28_SetCursorA_ILI932x,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI932x,
				.WriteReg = UiLcdHy28_WriteReg_ILI_PAR,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_Generic,
				.DrawFullRect = UiLcdHy28_DrawFullRect_Generic,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_Generic,
        },
#ifdef USE_GFX_SSD1289
        {
                DISPLAY_HY32D_PARALLEL_SSD1289, "HY32D Para. SSD1289",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_SSD1289,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_SSD1289,
                .SetCursorA = UiLcdHy28_SetCursorA_SSD1289,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI932x,
				.WriteReg = UiLcdHy28_WriteReg_ILI_PAR,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_Generic,
				.DrawFullRect = UiLcdHy28_DrawFullRect_Generic,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_Generic,
        },
#endif
#endif
#ifdef USE_GFX_RA8875
        {
                DISPLAY_RA8875_PARALLEL, "RA8875 Para.",
				.ReadDisplayId = UiLcdRa8875_ReadDisplayId,
                .SetActiveWindow = UiLcdRa8875_SetActiveWindow,
                .SetCursorA = UiLcdRa8875_SetCursorA,
                .WriteRAM_Prepare = UiLcdRa8875_WriteRAM_Prepare,
				.WriteReg = UiLcdRa8875_WriteReg,
				.ReadReg = UiLcdRa8875_ReadReg,
				.DrawStraightLine = UiLcdRa8875_DrawStraightLine,
				.DrawFullRect = UiLcdRa8875_DrawFullRect,
				.DrawColorPoint = UiLcdRa8875_DrawColorPoint,

        },
#endif
#endif

#if  defined(USE_DISPLAY_SPI)
// we support HY28A SPI only on the UI_BRD_MCHF
#if defined(USE_GFX_ILI932x) && defined(UI_BRD_MCHF)
        {
                DISPLAY_HY28A_SPI, "HY28A SPI",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_ILI932x,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_ILI932x,
                .SetCursorA = UiLcdHy28_SetCursorA_ILI932x,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI932x,
                .WriteDataSpiStart_Prepare = UiLcdHy28_WriteDataSpiStart_Prepare_ILI932x,
                .WriteIndexSpi_Prepare = UiLcdHy28_WriteIndexSpi_Prepare_ILI932x,
				.WriteReg = UiLcdHy28_WriteReg_ILI_SPI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_Generic,
				.DrawFullRect = UiLcdHy28_DrawFullRect_Generic,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_Generic,
                LCD_D11_PIO, LCD_D11,
                .is_spi = true,
				.spi_speed=false
        },
#endif
#if defined(USE_GFX_ILI932x)
        {
                DISPLAY_HY28B_SPI, "HY28B SPI",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_ILI932x,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_ILI932x,
                .SetCursorA = UiLcdHy28_SetCursorA_ILI932x,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI932x,
                .WriteDataSpiStart_Prepare = UiLcdHy28_WriteDataSpiStart_Prepare_ILI932x,
                .WriteIndexSpi_Prepare = UiLcdHy28_WriteIndexSpi_Prepare_ILI932x,
                .spi_cs_port = LCD_CSA_PIO,
                .spi_cs_pin = LCD_CSA,
				.WriteReg = UiLcdHy28_WriteReg_ILI_SPI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_Generic,
				.DrawFullRect = UiLcdHy28_DrawFullRect_Generic,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_Generic,
                .is_spi = true,
				.spi_speed=false
        },
#endif
#ifdef USE_GFX_RA8875
        {
                DISPLAY_RA8875_SPI, "RA8875 SPI",
                .ReadDisplayId = UiLcdRa8875_ReadDisplayId,
                .SetActiveWindow = UiLcdRa8875_SetActiveWindow,
                .SetCursorA = UiLcdRa8875_SetCursorA,
                .WriteRAM_Prepare = UiLcdRa8875_WriteRAM_Prepare,
                .WriteDataSpiStart_Prepare = UiLcdRa8875_WriteDataSpiStart_Prepare,
                .WriteIndexSpi_Prepare = UiLcdRa8875_WriteIndexSpi_Prepare,
  				.WriteReg = UiLcdRa8875_WriteReg,
				.ReadReg = UiLcdRa8875_ReadReg,
				.DrawStraightLine = UiLcdRa8875_DrawStraightLine,
				.DrawFullRect = UiLcdRa8875_DrawFullRect,
				.DrawColorPoint = UiLcdRa8875_DrawColorPoint,
                .spi_cs_port = LCD_CSA_PIO,
                .spi_cs_pin = LCD_CSA,
                .is_spi = true,
                .spi_speed = true
        },
#endif
#if defined(USE_GFX_ILI9486)
        {       DISPLAY_RPI_SPI, "RPi 3.5 SPI",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_ILI9486,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_ILI9486,
                .SetCursorA = UiLcdHy28_SetCursorA_ILI9486,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI9486,
                .WriteDataSpiStart_Prepare = UiLcdHy28_WriteDataSpiStart_Prepare_ILI9486,
                .WriteIndexSpi_Prepare = UiLcdHy28_WriteIndexSpi_Prepare_ILI9486,
				.WriteReg = UiLcdHy28_WriteReg_ILI_SPI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_Generic,
				.DrawFullRect = UiLcdHy28_DrawFullRect_Generic,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_Generic,
                .spi_cs_port = LCD_CSA_PIO,
                .spi_cs_pin = LCD_CSA,
                .is_spi = true,
				.spi_speed=true
        },
        // RPI_SPI NEEDS TO BE LAST IN LIST!!!
#endif
#endif

        {
                DISPLAY_NUM, "Unknown", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0
        }
 };

/* Controller Specific Functions End *********************************/

static void UiLcd_SendRegisters(const RegisterValueSetInfo_t* reg_info)
{
    for (uint16_t idx = 0; idx < reg_info->size; idx++)
    {
        switch(reg_info->addr[idx].reg)
        {
        case REGVAL_DELAY:
            HAL_Delay(reg_info->addr[idx].val);
            break;
        case REGVAL_DATA:
            UiLcd_WriteData(reg_info->addr[idx].val);
            break;
        default:
            // TODO: Decide how we handle 16 vs. 8 bit writes here
            // I would propose either per register setting or per register set setting
            UiLcd_WriteReg(reg_info->addr[idx].reg, reg_info->addr[idx].val);
        }
    }
}


static uint16_t UiLcd_DetectController(const uhsdr_display_info_t* disp_info_ptr)
{

    uint16_t retval = 0;

    if (disp_info_ptr != NULL)
    {
        mchf_display.use_spi = disp_info_ptr->is_spi;
        mchf_display.lcd_cs = disp_info_ptr->spi_cs_pin;
        mchf_display.lcd_cs_pio = disp_info_ptr->spi_cs_port;
        mchf_display.SetActiveWindow = disp_info_ptr->SetActiveWindow;
        mchf_display.SetCursorA = disp_info_ptr->SetCursorA;
        mchf_display.WriteRAM_Prepare = disp_info_ptr->WriteRAM_Prepare;
        mchf_display.WriteDataSpiStart_Prepare = disp_info_ptr->WriteDataSpiStart_Prepare;
        mchf_display.WriteIndexSpi_Prepare = disp_info_ptr->WriteIndexSpi_Prepare;
        mchf_display.WriteReg = disp_info_ptr->WriteReg;
        mchf_display.ReadReg = disp_info_ptr->ReadReg;
        mchf_display.DrawStraightLine = disp_info_ptr->DrawStraightLine;
        mchf_display.DrawFullRect = disp_info_ptr->DrawFullRect;
        mchf_display.DrawColorPoint = disp_info_ptr->DrawColorPoint;
        mchf_display.reg_info = NULL;
        UiLcd_GpioInit(disp_info_ptr->display_type);

        if (mchf_display.use_spi == true)
        {
#ifdef USE_DISPLAY_SPI
            UiLcd_SpiInit(disp_info_ptr->spi_speed, disp_info_ptr->display_type);
#endif
        }
        else
        {
#ifdef USE_DISPLAY_PAR
            UiLcd_ParallelInit();
#endif
        }

        UiLcd_Reset();

        // if we have an identifier function, call it
        // WITHOUT function the display will never be used!
        if (disp_info_ptr->ReadDisplayId)
        {
            retval = disp_info_ptr->ReadDisplayId();
        }

        // if the identification set a register data set for initialization,
        // we send it to the display controller
        if (mchf_display.reg_info != NULL)
        {
            UiLcd_SendRegisters(mchf_display.reg_info);
        }

        // okay, this display was not detected,
        // cleanup data structures and prepare
        // for next try
        if (retval == 0)
        {
            mchf_display.SetActiveWindow = NULL;
            mchf_display.SetCursorA = NULL;
            mchf_display.WriteRAM_Prepare = NULL;
            mchf_display.WriteDataSpiStart_Prepare = NULL;
            mchf_display.WriteIndexSpi_Prepare = NULL;
            mchf_display.lcd_cs = 0;
            mchf_display.lcd_cs_pio = NULL;

            if (mchf_display.use_spi == true)
            {
                #ifdef USE_DISPLAY_SPI
                UiLcd_SpiDeInit();
                #endif
            }
            else
            {
                #ifdef USE_DISPLAY_PAR
                UiLcd_ParallelDeInit();
                #endif
            }

        }
    }


	return retval;
}

/*
 * brief Identifies and initializes to HY28x display family
 *
 * @param devicecode_ptr pointer to a variable to store the device code of the controller in
 * @returns 0 if no display detected, DISPLAY_HY28x_xxx otherwise, see header
 */

uint8_t UiLcd_Init()
{
    uint8_t retval = DISPLAY_NONE;
    mchf_display.DeviceCode = 0x0000;
    UiLcd_BacklightInit();

    for (uint16_t disp_idx = 1; retval == DISPLAY_NONE && display_infos[disp_idx].display_type != DISPLAY_NUM; disp_idx++)
    {
        mchf_display.DeviceCode = UiLcd_DetectController(&display_infos[disp_idx]);

        if(mchf_display.DeviceCode != 0x0000)
        {
            retval = display_infos[disp_idx].display_type;
        }
    }

    mchf_display.display_type = retval;

#ifndef BOOTLOADER_BUILD
    switch(mchf_display.DeviceCode)
    {
    case 0x8875:
    	ts.Layout=&LcdLayouts[LcdLayout_800x480];
    	disp_resolution=RESOLUTION_800_480;
    	break;
    case 0x9486:
    case 0x9488:
    	ts.Layout=&LcdLayouts[LcdLayout_480x320];
    	disp_resolution=RESOLUTION_480_320;
    	break;
    default:
    	ts.Layout=&LcdLayouts[LcdLayout_320x240];
    	disp_resolution=RESOLUTION_320_240;
    	break;
    }
    mchf_display.MAX_X=ts.Layout->Size.x;
    mchf_display.MAX_Y=ts.Layout->Size.y;
#else
    switch(mchf_display.DeviceCode)
    {
    case 0x8875:
    	mchf_display.MAX_X=800;
    	mchf_display.MAX_Y=480;
    	break;
    case 0x9486:
    case 0x9488:
    	mchf_display.MAX_X=480;
    	mchf_display.MAX_Y=320;
    	break;
    default:
    	mchf_display.MAX_X=320;
    	mchf_display.MAX_Y=240;
    	break;
    }
#endif

    return retval;
}
