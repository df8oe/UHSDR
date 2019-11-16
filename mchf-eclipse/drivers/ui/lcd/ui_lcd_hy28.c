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

#include "ui_lcd_hy28_fonts.h"
#include "ui_lcd_hy28.h"

#define hspiDisplay hspi2
#define SPI_DISPLAY SPI2

#ifndef STM32H7
  // FIXME: H7 Port, re-enable DMA once SPI display is working
  #define USE_SPI_DMA
#endif

#if defined(STM32F7) || defined(STM32H7)
    #define USE_SPI_HAL
#endif

#define USE_SPI_DISPLAY
#define USE_DISPLAY_PAR


#if !defined(USE_DISPLAY_PAR) && !defined(USE_SPI_DISPLAY)
#warning Both USE_DISPLAY_PAR and USE_SPI_DISPLAY are disabled, no display driver will be available!
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


   	#define LCD_REG_RA8875      (*((volatile unsigned short *) 0x60004000))
    #define LCD_RAM_RA8875      (*((volatile unsigned short *) 0x60000000))

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

const uhsdr_display_info_t* UiLcdHy28_DisplayInfoGet(mchf_display_types_t display_type)
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

#ifndef BOOTLOADER_BUILD

// Saved fonts
extern sFONT GL_Font8x8;
extern sFONT GL_Font8x12;
extern sFONT GL_Font8x12_bold;
extern sFONT GL_Font12x12;
extern sFONT GL_Font16x24;
#ifdef USE_8bit_FONT
extern sFONT GL_Font16x24_8b_Square;
#endif

static sFONT *fontList[] =
{
        &GL_Font8x12_bold,
        &GL_Font16x24,
        &GL_Font12x12,
        &GL_Font8x12,
        &GL_Font8x8,
#ifdef USE_8bit_FONT
		&GL_Font16x24_8b_Square,
#endif
};

#else
extern sFONT GL_Font8x12_bold_short;

static sFONT *fontList[] =
{
        &GL_Font8x12_bold_short,
};
#endif

// we can do this here since fontList is an array variable not just a pointer!
static const uint8_t fontCount = sizeof(fontList)/sizeof(fontList[0]);


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
 
#ifdef USE_GFX_RA8875
static const RegisterValue_t ra8875[] =
{
        { 0x01, 0x01}, // Software reset the LCD
	    { REGVAL_DELAY, 100},              // delay 100 ms
        { 0x01, 0x00},
	    { REGVAL_DELAY, 100},              // delay 100 ms
        { 0x88, 0x0a},
	    { REGVAL_DELAY, 100},              // delay 100 ms
        { 0x89, 0x02},
	    { REGVAL_DELAY, 100},              // delay 100 ms
        { 0x10, 0x0F},   // 65K 16 bit 8080 mpu interface
        { 0x04, 0x80},   // 00b: PCLK period = System Clock period.
	    { REGVAL_DELAY, 100},              // delay 100 ms
        //Horizontal set
        { 0x14, 0x63}, //Horizontal display width(pixels) = (HDWR + 1)*8
        { 0x15, 0x00}, //Horizontal Non-Display Period Fine Tuning(HNDFT) [3:0]
        { 0x16, 0x03}, //Horizontal Non-Display Period (pixels) = (HNDR + 1)*8
        { 0x17, 0x03}, //HSYNC Start Position(PCLK) = (HSTR + 1)*8
        { 0x18, 0x0B}, //HSYNC Width [4:0]   HSYNC Pulse width(PCLK) = (HPWR + 1)*8
        //Vertical set
        { 0x19, 0xdf}, //Vertical pixels = VDHR + 1
        { 0x1a, 0x01}, //Vertical pixels = VDHR + 1
        { 0x1b, 0x20}, //Vertical Non-Display area = (VNDR + 1)
        { 0x1c, 0x00}, //Vertical Non-Display area = (VNDR + 1)
        { 0x1d, 0x16}, //VSYNC Start Position(PCLK) = (VSTR + 1)
        { 0x1e, 0x00}, //VSYNC Start Position(PCLK) = (VSTR + 1)
        { 0x1f, 0x01}, //VSYNC Pulse Width(PCLK) = (VPWR + 1)
        // setting active window 0,799,0,479
        { 0x30, 0x00},
        { 0x31, 0x00},
        { 0x34, 0x1f},
        { 0x35, 0x03},
        { 0x32, 0x00},
        { 0x33, 0x00},
        { 0x36, 0xDF},
        { 0x37, 0x01},
        { 0x8a, 0x80},
        { 0x8a, 0x81}, //open PWM
        { 0x8b, 0x1f}, //Brightness parameter 0xff-0x00
        { 0x01, 0x80}, //display on

        //UiLcdHy28_WriteReg(LCD_DPCR,0b00001111}, // rotacion 180º

        { 0x60, 0x00}, /* ra8875_red */
        { 0x61, 0x00}, /* ra8875_green */
        { 0x62, 0x00}, /* ra8875_blue */

        { 0x8E, 0x80},
};
 
static const RegisterValueSetInfo_t ra8875_regs =
{
    ra8875, sizeof(ra8875)/sizeof(RegisterValue_t)
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



typedef struct
{
    uint16_t x;
    uint16_t width;
    uint16_t y;
    uint16_t height;
} lcd_bulk_transfer_header_t;

#ifdef  USE_GFX_RA8875
void UiLcdHy28_RA8875_WaitReady();
void UiLcdRA8875_SetForegroundColor(uint16_t Color);
void UiLcdRa8875_WriteReg_8bit(uint16_t LCD_Reg, uint8_t LCD_RegValue);
void UiLcdRa8875_WriteReg_16bit(uint16_t LCD_Reg, uint16_t LCD_RegValue);
#endif

static void UiLcdHy28_BulkWriteColor(uint16_t Color, uint32_t len);

static inline bool UiLcdHy28_SpiDisplayUsed()
{
    bool retval = false;
#ifdef USE_SPI_DISPLAY
    retval = mchf_display.use_spi;
#endif
    return retval;
}


void UiLcdHy28_BacklightInit()
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

void UiLcdHy28_BacklightEnable(bool on)
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

#ifdef STM32F4
    #define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_4)
    #define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_2)
	#define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_64)
#endif

#ifdef STM32F7
    #define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_8)
    #define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_4)
	#define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_128)
#endif

#ifdef STM32H7
    #define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_8)
    #define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_4)
    #define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_32)
    // 16 may be a little bit high for some displays but works with the 480x320 display
#endif

static uint32_t lcd_spi_prescaler = SPI_PRESCALE_LCD_DEFAULT;

// static SPI_HandleTypeDef SPI_Handle;

void UiLcdHy28_SpiInit(bool hispeed, mchf_display_types_t display_type)
{
    lcd_spi_prescaler = hispeed?SPI_PRESCALE_LCD_HIGH:SPI_PRESCALE_LCD_DEFAULT;

#ifdef USE_GFX_ILI9486
    if (display_type == DISPLAY_RPI_SPI)
    {
        hspiDisplay.Init.CLKPolarity = SPI_POLARITY_LOW;
        hspiDisplay.Init.CLKPhase = SPI_PHASE_1EDGE;
        hspiDisplay.Init.NSS = SPI_NSS_SOFT;

        hspiDisplay.Init.BaudRatePrescaler = lcd_spi_prescaler;

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

void UiLcdHy28_GpioInit(mchf_display_types_t display_type)
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


DMA_HandleTypeDef DMA_Handle;


static inline void UiLcdHy28_SpiDmaStop()
{
    while (DMA1_Stream4->CR & DMA_SxCR_EN) { asm(""); }
}



void UiLcdHy28_SpiDmaStart(uint8_t* buffer, uint32_t size)
{

    // do busy waiting here. This is just for testing if everything goes according to plan
    // if this works okay, we can let SPI DMA running while doing something else
    // and just check before next transfer if DMA is being done.
    // and finally we can move that into an interrupt, of course.
    if (size > 0)  {
        UiLcdHy28_SpiDmaStop();
        HAL_SPI_Transmit_DMA(&hspiDisplay,buffer,size);
    }
}

void UiLcdHy28_SpiDeInit()
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

inline void UiLcdHy28_SpiLcdCsDisable()
{
    GPIO_SetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}
static inline void UiLcdHy28_SpiLcdCsEnable()
{
    GPIO_ResetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}

#ifdef USE_DISPLAY_PAR
static void UiLcdHy28_ParallelInit()
{
    MEM_Init();
}

static void UiLcdHy28_ParallelDeInit()
{
    HAL_SRAM_DeInit(&hsram1);

}
#endif

static void UiLcdHy28_Reset()
{
    // Reset
    GPIO_SetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(1);

    GPIO_ResetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(1);

    GPIO_SetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(300);
}

static inline void UiLcdHy28_SpiSendByte(uint8_t byte)
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


uint8_t spi_dr_dummy; // used to make sure that DR is being read
static inline void UiLcdHy28_SpiFinishTransfer()
{
#ifdef STM32H7
    #ifndef USE_SPI_HAL
    // we cannot use this with HAL, the "normal" HAL Transmit does check the flags AND resets them (ARGH)
    while (__HAL_SPI_GET_FLAG(&hspiDisplay, SPI_SR_EOT) == 0 || __HAL_SPI_GET_FLAG(&hspiDisplay, SPI_SR_EOT) == 0 ) { asm("nop"); }
    while (__HAL_SPI_GET_FLAG(&hspiDisplay, SPI_FLAG_RXWNE) != 0 || __HAL_SPI_GET_FLAG(&hspiDisplay, SPI_SR_RXPLVL) != 0 )
    {
        spi_dr_dummy = SPI_DISPLAY->RXDR;
    }
    #endif
#else
    while ((SPI_DISPLAY->SR & (SPI_FLAG_TXE)) == (uint16_t)RESET) {}
    while (SPI_DISPLAY->SR & SPI_FLAG_BSY) {}
    if (SPI_DISPLAY->SR & SPI_FLAG_RXNE)
    {
        spi_dr_dummy = SPI_DISPLAY->DR;
    }
#endif
}

static void UiLcdHy28_LcdSpiFinishTransfer()
{
    UiLcdHy28_SpiFinishTransfer();
    GPIO_SetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}

uint8_t UiLcdHy28_SpiReadByte()
{
    uint8_t dummy = 0;
    uint8_t retval = 0;

    /* Send a Transmit a dummy byte and Receive Byte through the SPI peripheral */
    HAL_SPI_TransmitReceive(&hspi2, &dummy,&retval,1,SPI_TIMEOUT);

    return retval;
}

uint8_t UiLcdHy28_SpiReadByteFast()
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


// TODO: Function Per Controller Group
void UiLcdHy28_WriteIndexSpi(unsigned char index)
{
    UiLcdHy28_SpiLcdCsEnable();

    mchf_display.WriteIndexSpi_Prepare();

    UiLcdHy28_SpiSendByte(0);
    UiLcdHy28_SpiSendByte(index);

    UiLcdHy28_LcdSpiFinishTransfer();
}


// TODO: Function Per Controller Group
static inline void UiLcdHy28_WriteDataSpiStart()
{
    UiLcdHy28_SpiLcdCsEnable();

    mchf_display.WriteDataSpiStart_Prepare();
}

void UiLcdHy28_WriteDataSpi( unsigned short data)
{
    UiLcdHy28_WriteDataSpiStart();
    UiLcdHy28_SpiSendByte((data >>   8));                    /* Write D8..D15                */
    UiLcdHy28_SpiSendByte((data & 0xFF));                    /* Write D0..D7   */
    UiLcdHy28_LcdSpiFinishTransfer();
}

static inline void UiLcdHy28_WriteDataOnly( unsigned short data)
{
	if(UiLcdHy28_SpiDisplayUsed())
    {
        UiLcdHy28_SpiSendByte((data >>   8));      /* Write D8..D15                */
        UiLcdHy28_SpiSendByte((data & 0xFF));      /* Write D0..D7                 */
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        LCD_RAM = data;
        __DMB();
#endif
    }
}
#ifdef USE_GFX_RA8875
static inline void UiLcdHy28_WriteDataOnlyRA8875( unsigned short data)
{
	LCD_RAM_RA8875 = data;
	__DMB();
}
#endif

static inline void UiLcdHy28_WriteData( unsigned short data)
{
	if(mchf_display.DeviceCode==0x8875)
	{
        LCD_RAM_RA8875 = data;
        __DMB();
	}
	else if(UiLcdHy28_SpiDisplayUsed())
    {
        UiLcdHy28_WriteDataSpiStart();
        UiLcdHy28_SpiSendByte((data >>   8));                    /* Write D8..D15                */
        UiLcdHy28_SpiSendByte((data & 0xFF));                    /* Write D0..D7   */
        UiLcdHy28_LcdSpiFinishTransfer();
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        LCD_RAM = data;
        __DMB();
#endif
    }
}

unsigned short UiLcdHy28_LcdReadDataSpi()
{
    unsigned short value = 0;
    uchar y,z;

    UiLcdHy28_SpiLcdCsEnable();

    UiLcdHy28_SpiSendByte(SPI_START | SPI_RD | SPI_DATA);    /* Read: RS = 1, RW = 1         */

    UiLcdHy28_SpiReadByte();                                /* Dummy read 1                 */

    y   = UiLcdHy28_SpiReadByte();                      /* Read D8..D15                 */
    value = y;
    value <<= 8;
    z = UiLcdHy28_SpiReadByte();                      /* Read D0..D7                  */

    value  |= z;

    UiLcdHy28_LcdSpiFinishTransfer();

    return value;
}


/**
 * @brief writes a controller register in its native width 16 bit or 8bit
 * width is controller dependent (RA8875 uses 8bit, all other 16bit)
 */
void UiLcdHy28_WriteReg(unsigned short LCD_Reg, unsigned short LCD_RegValue)
{
	mchf_display.WriteReg(LCD_Reg,LCD_RegValue);
}

void UiLcdHy28_WriteReg_ILI(unsigned short LCD_Reg, unsigned short LCD_RegValue)
{

    if(UiLcdHy28_SpiDisplayUsed())
    {
        UiLcdHy28_WriteIndexSpi(LCD_Reg);
        UiLcdHy28_WriteDataSpi(LCD_RegValue);
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        LCD_REG = LCD_Reg;
        __DMB();
        LCD_RAM = LCD_RegValue;
        __DMB();
#endif
    }
}

uint16_t UiLcdHy28_ReadReg(uint16_t LCD_Reg)
{
	return mchf_display.ReadReg(LCD_Reg);
}
#ifdef USE_GFX_RA8875

void UiLcdHy28_WriteRegRA8875(unsigned short LCD_Reg, unsigned short LCD_RegValue)
{
	UiLcdHy28_RA8875_WaitReady();
	LCD_REG_RA8875 = LCD_Reg;
	__DMB();
	LCD_RAM_RA8875 = LCD_RegValue;
	__DMB();
}

uint16_t UiLcdHy28_ReadRegRA8875(uint16_t LCD_Reg)
{
	uint16_t retval;
    // Write 16-bit Index (then Read Reg)
	LCD_REG_RA8875 = LCD_Reg;
    // Read 16-bit Reg
    __DMB();
    retval = LCD_RAM_RA8875;

    return retval;
}
#endif
unsigned short UiLcdHy28_ReadRegILI(uint16_t LCD_Reg)
{
    uint16_t retval;
    if(UiLcdHy28_SpiDisplayUsed())
    {
        // Write 16-bit Index (then Read Reg)
        UiLcdHy28_WriteIndexSpi(LCD_Reg);
        // Read 16-bit Reg
        retval = UiLcdHy28_LcdReadDataSpi();
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        // Write 16-bit Index (then Read Reg)
        LCD_REG = LCD_Reg;
        // Read 16-bit Reg
        __DMB();
        retval = LCD_RAM;
#else
        retval = 0;
#endif
    }
    return retval;
}

static void UiLcdHy28_SetCursorA( unsigned short Xpos, unsigned short Ypos )
{
    mchf_display.SetCursorA(Xpos, Ypos);
}

static void UiLcdHy28_WriteRAM_Prepare_Index(uint16_t wr_prep_reg)
{
	if(mchf_display.DeviceCode==0x8875)
	{
        LCD_REG_RA8875 = wr_prep_reg;
        __DMB();
	}
	else if(UiLcdHy28_SpiDisplayUsed())
    {
        UiLcdHy28_WriteIndexSpi(wr_prep_reg);
        UiLcdHy28_WriteDataSpiStart();
    }
    else
    {
#ifdef USE_DISPLAY_PAR
        LCD_REG = wr_prep_reg;
        __DMB();
#endif
    }
}

static void UiLcdHy28_WriteRAM_Prepare()
{

    mchf_display.WriteRAM_Prepare();
}

static void UiLcdHy28_SetActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom)
{
    mchf_display.SetActiveWindow(XLeft, XRight, YTop, YBottom);
}

static void UiLcdHy28_BulkWrite(uint16_t* pixel, uint32_t len)
{

// if we are not using SPI DMA, we send the data as it comes
// if we are using SPI DMA, we do this only if we are NOT using SPI
#ifdef USE_SPI_DMA
    if(UiLcdHy28_SpiDisplayUsed() == false)
#endif
    {
#ifdef USE_GFX_RA8875
    	if(mchf_display.DeviceCode==0x8875)
    	{
    		for (uint32_t i = len; i; i--)
    		{
    			UiLcdHy28_WriteDataOnlyRA8875(*(pixel++));
    		}
    	}
    	else
#endif
    	{
    		for (uint32_t i = len; i; i--)
    		{
    			UiLcdHy28_WriteDataOnly(*(pixel++));
    		}
    	}
    }
#ifdef USE_SPI_DMA
    else
    {
        for (uint32_t i = 0; i < len; i++)
        {
            pixel[i] = __REV16(pixel[i]); // reverse byte order;
        }
        UiLcdHy28_SpiDmaStart((uint8_t*)pixel,len*2);
    }
#endif

}

static void UiLcdHy28_FinishWaitBulkWrite()
{
    if(UiLcdHy28_SpiDisplayUsed())         // SPI enabled?
    {
#ifdef USE_SPI_DMA
        UiLcdHy28_SpiDmaStop();
#endif
        UiLcdHy28_LcdSpiFinishTransfer();
    }
}

static void UiLcdHy28_OpenBulkWrite(ushort x, ushort width, ushort y, ushort height)
{
    UiLcdHy28_FinishWaitBulkWrite();
    UiLcdHy28_SetActiveWindow(x, x + width - 1, y, y + height - 1);
    UiLcdHy28_SetCursorA(x, y);
    UiLcdHy28_WriteRAM_Prepare();
}

static void UiLcdHy28_CloseBulkWrite()
{
#ifdef USE_GFX_RA8875
	if(mchf_display.DeviceCode==0x8875)
	{
		uint16_t MAX_X=mchf_display.MAX_X; uint16_t MAX_Y=mchf_display.MAX_Y;
		UiLcdHy28_SetActiveWindow(0, MAX_X - 1, 0, MAX_Y - 1);
		UiLcdHy28_WriteReg(0x40, 0);
	}
#endif
}

#define PIXELBUFFERSIZE 512
#define PIXELBUFFERCOUNT 2

static __UHSDR_DMAMEM uint16_t   pixelbuffer[PIXELBUFFERCOUNT][PIXELBUFFERSIZE];
static uint16_t pixelcount = 0;
static uint16_t pixelbufidx = 0;

static inline void UiLcdHy28_BulkPixel_BufferInit()
{
    pixelbufidx= (pixelbufidx+1)%PIXELBUFFERCOUNT;
    pixelcount = 0;
}


inline void UiLcdHy28_BulkPixel_BufferFlush()
{
    UiLcdHy28_BulkWrite(pixelbuffer[pixelbufidx],pixelcount);
    UiLcdHy28_BulkPixel_BufferInit();
}

inline void UiLcdHy28_BulkPixel_Put(uint16_t pixel)
{
    pixelbuffer[pixelbufidx][pixelcount++] = pixel;
    if (pixelcount == PIXELBUFFERSIZE)
    {
        UiLcdHy28_BulkPixel_BufferFlush();
    }
}

// TODO: Not most efficient way, we could use remaining buffer size to judge
// if it will fit without flush and fill accordingly.

inline void UiLcdHy28_BulkPixel_PutBuffer(uint16_t* pixel_buffer, uint32_t len)
{
    // We bypass the buffering if in parallel mode
    // since as for now, it will not benefit from it.
    // this can be changed if someone write DMA code for the parallel
    // interface (memory to memory DMA)
    if(UiLcdHy28_SpiDisplayUsed())         // SPI enabled?
    {
        for (uint32_t idx = 0; idx < len; idx++)
        {
            UiLcdHy28_BulkPixel_Put(pixel_buffer[idx]);
        }
    }
    else
    {
        UiLcdHy28_BulkWrite(pixel_buffer, len);
    }
}

inline void UiLcdHy28_BulkPixel_OpenWrite(ushort x, ushort width, ushort y, ushort height)
{
    UiLcdHy28_OpenBulkWrite(x, width,y,height);
    UiLcdHy28_BulkPixel_BufferInit();
}

inline void UiLcdHy28_BulkPixel_CloseWrite()
{
    UiLcdHy28_BulkPixel_BufferFlush();
    UiLcdHy28_CloseBulkWrite();
}


void UiLcdHy28_LcdClear(ushort Color)
{
	uint32_t MAX_X=mchf_display.MAX_X; uint32_t MAX_Y=mchf_display.MAX_Y;
    UiLcdHy28_OpenBulkWrite(0,MAX_X,0,MAX_Y);
#ifdef USE_SPI_DMA
    if(UiLcdHy28_SpiDisplayUsed())
    {
        int idx;

        UiLcdHy28_BulkPixel_BufferInit();

        for (idx = 0; idx < MAX_X * MAX_Y; idx++)
        {
            UiLcdHy28_BulkPixel_Put(Color);
        }
        UiLcdHy28_BulkPixel_BufferFlush();
    }
    else
#endif
    {
        UiLcdHy28_BulkWriteColor(Color,MAX_X * MAX_Y);
    }
    UiLcdHy28_CloseBulkWrite();
}

#if 1

void UiLcdHy28_DrawColorPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	mchf_display.DrawColorPoint(Xpos,Ypos,point);
}

void UiLcdHy28_DrawColorPoint_ILI(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	uint16_t MAX_X=mchf_display.MAX_X; uint16_t MAX_Y=mchf_display.MAX_Y;
    if( Xpos < MAX_X && Ypos < MAX_Y )
    {
        UiLcdHy28_OpenBulkWrite(Xpos,1,Ypos,1);
        UiLcdHy28_WriteDataOnly(point);
        UiLcdHy28_CloseBulkWrite();
    }
}
#endif


void UiLcdHy28_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color)
{
	mchf_display.DrawFullRect(Xpos,Ypos,Height,Width,color);
}

void UiLcdHy28_DrawFullRect_ILI(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color)
{
    UiLcdHy28_OpenBulkWrite(Xpos, Width, Ypos, Height);
    UiLcdHy28_BulkWriteColor(color,(uint32_t)Height * (uint32_t)Width);
    UiLcdHy28_CloseBulkWrite();
}


#ifdef  USE_GFX_RA8875
void UiLcdHy28_DrawFullRect_RA8875(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color)
{
    UiLcdRA8875_SetForegroundColor(color);
    UiLcdRa8875_WriteReg_16bit(0x91, Xpos);				//Horizontal start
    UiLcdRa8875_WriteReg_16bit(0x95, Xpos + Width-1);	//Horizontal end
    UiLcdRa8875_WriteReg_16bit(0x93, Ypos);				//Vertical start
    UiLcdRa8875_WriteReg_16bit(0x97, Ypos + Height-1);	//Vertical end
    UiLcdHy28_WriteRegRA8875(0x90, 0xB0);				// Fill rectangle
}

void UiLcdHy28_DrawColorPoint_RA8875(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	uint16_t MAX_X=mchf_display.MAX_X; uint16_t MAX_Y=mchf_display.MAX_Y;
    if( Xpos < MAX_X && Ypos < MAX_Y )
    {
        UiLcdHy28_SetCursorA(Xpos, Ypos);
        UiLcdHy28_WriteRegRA8875(0x02, point);
    }
}

void UiLcdHy28_RA8875_WaitReady()
{
    uint16_t temp;
    do {
        temp = LCD_REG_RA8875;
    } while ((temp & 0x80) == 0x80);
}

void UiLcdRa8875_WriteReg_8bit(uint16_t LCD_Reg, uint8_t LCD_RegValue)
{
	UiLcdHy28_WriteRegRA8875(LCD_Reg, LCD_RegValue);
}
void UiLcdRa8875_WriteReg_16bit(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
	UiLcdHy28_WriteRegRA8875(LCD_Reg,LCD_RegValue & 0xff);
	UiLcdHy28_WriteRegRA8875(LCD_Reg+1,(LCD_RegValue >> 8) & 0xff);
}


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+                               SCROLL STUFF                                             +
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/**************************************************************************/
/*!
        Sets the scroll mode. This is controlled by bits 6 and 7 of
        REG[52h] Layer Transparency Register0 (LTPR0)
        Author: The Experimentalist
*/
/**************************************************************************/
void UiLcdRA8875_setScrollMode(uint8_t mode)
{

#define RA8875_LTPR0                  0x52//Layer Transparency Register 0

    uint8_t temp = UiLcdHy28_ReadReg(RA8875_LTPR0);
    temp &= 0x3F;            // Clear bits 6 and 7 to zero
    switch(mode){           // bit 7,6 of LTPR0
        case 0:  // 00b : Layer 1/2 scroll simultaneously.
            // Do nothing
        break;
        case 1:        // 01b : Only Layer 1 scroll.
            temp |= 0x40;
        break;
        case 2:        // 10b : Only Layer 2 scroll.
            temp |= 0x80;
        break;
        case 3:          // 11b: Buffer scroll (using Layer 2 as scroll buffer)
            temp |= 0xC0;
        break;
        default:
            return;             //do nothing
    }
    LCD_RAM=temp;
    __DMB();

}

/**************************************************************************/
/*!
        Define a window for perform scroll
        Parameters:
        XL: x window start left
        XR: x window end right
        YT: y window start top
        YB: y window end bottom
*/
/**************************************************************************/
void UiLcdRA8875_setScrollWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB)
{

#define RA8875_HSSW0                  0x38//Horizontal Start Point 0 of Scroll Window
//#define RA8875_HSSW1                0x39//Horizontal Start Point 1 of Scroll Window
#define RA8875_VSSW0                  0x3A//Vertical     Start Point 0 of Scroll Window
//#define RA8875_VSSW1                0x3B//Vertical     Start Point 1 of Scroll Window
#define RA8875_HESW0                  0x3C//Horizontal End   Point 0 of Scroll Window
//#define RA8875_HESW1                0x3D//Horizontal End   Point 1 of Scroll Window
#define RA8875_VESW0                  0x3E//Vertical     End   Point 0 of Scroll Window
//#define RA8875_VESW1                0x3F//Vertical     End   Point 1 of Scroll Window


    UiLcdRa8875_WriteReg_16bit(RA8875_HSSW0,XL);
    UiLcdRa8875_WriteReg_16bit(RA8875_HESW0,XR);
    UiLcdRa8875_WriteReg_16bit(RA8875_VSSW0,YT);
    UiLcdRa8875_WriteReg_16bit(RA8875_VESW0,YB);
}

/**************************************************************************/
/*!
        Perform the scroll
*/
/**************************************************************************/
void UiLcdRA8875_scroll(int16_t x,int16_t y)
{

#define RA8875_HOFS0                  0x24//Horizontal Scroll Offset Register 0
#define RA8875_HOFS1                  0x25//Horizontal Scroll Offset Register 1
#define RA8875_VOFS0                  0x26//Vertical Scroll Offset Register 0
#define RA8875_VOFS1                  0x27//Vertical Scroll Offset Register 1

    UiLcdRa8875_WriteReg_16bit(RA8875_HOFS0,x);
    UiLcdRa8875_WriteReg_16bit(RA8875_VOFS0,y);

}

void UiLcdHy28_DrawStraightLine_RA8875(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color)
{
    /* Drawing Control Registers */
    #define LCD_DCR     (0x90)      /* Draw Line/Circle/Square Control Register */
    #define LCD_DLHSR0  (0x91)      /* Draw Line/Square Horizontal Start Address Register0 */
    #define LCD_DLHSR1  (0x92)      /* Draw Line/Square Horizontal Start Address Register1 */
    #define LCD_DLVSR0  (0x93)      /* Draw Line/Square Vertical Start Address Register0 */
    #define LCD_DLVSR1  (0x94)      /* Draw Line/Square Vertical Start Address Register1 */
    #define LCD_DLHER0  (0x95)      /* Draw Line/Square Horizontal End Address Register0 */
    #define LCD_DLHER1  (0x96)      /* Draw Line/Square Horizontal End Address Register1 */
    #define LCD_DLVER0  (0x97)      /* Draw Line/Square Vertical End Address Register0 */
    #define LCD_DLVER1  (0x98)      /* Draw Line/Square Vertical End Address Register1 */

	if(Length>0)
	{
		UiLcdRA8875_SetForegroundColor(color);

		uint16_t x_end, y_end;

		if (Direction == LCD_DIR_VERTICAL)
		{
			x_end = x;
			y_end = y + Length-1;
		}
		else
		{
			x_end = x + Length-1;
			y_end = y;
		}

		if(x_end==x && y_end==y)
			UiLcdHy28_DrawColorPoint_RA8875(x,y,color);
		else
		{
			/* Horizontal + vertical start */
			UiLcdRa8875_WriteReg_16bit(LCD_DLHSR0, x);
			UiLcdRa8875_WriteReg_16bit(LCD_DLVSR0, y);
			UiLcdRa8875_WriteReg_16bit(LCD_DLHER0, x_end);
			UiLcdRa8875_WriteReg_16bit(LCD_DLVER0, y_end);

			UiLcdHy28_WriteRegRA8875(LCD_DCR, 0x80);
		}
	}
}

#endif


void UiLcdHy28_DrawStraightLineWidth(ushort x, ushort y, ushort Length, uint16_t Width, uchar Direction,ushort color)
{
    if(Direction == LCD_DIR_VERTICAL)
    {
        UiLcdHy28_DrawFullRect(x,y,Length,Width,color);
    }
    else
    {
        UiLcdHy28_DrawFullRect(x,y,Width,Length,color);
    }
}
void UiLcdHy28_DrawStraightLine(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color)
{
	mchf_display.DrawStraightLine(x,y,Length,Direction,color);
}
void UiLcdHy28_DrawStraightLine_ILI(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color)
{
	UiLcdHy28_DrawStraightLineWidth(x, y, Length, 1, Direction, color);
}


void UiLcdHy28_DrawStraightLineDouble(ushort x, ushort y, ushort Length, uchar Direction,ushort color)
{
    UiLcdHy28_DrawStraightLineWidth(x, y, Length, 2, Direction, color);
}

void UiLcdHy28_DrawStraightLineTriple(ushort x, ushort y, ushort Length, uchar Direction,ushort color)
{
    UiLcdHy28_DrawStraightLineWidth(x, y, Length, 3, Direction, color);
}


void UiLcdHy28_DrawHorizLineWithGrad(ushort x, ushort y, ushort Length,ushort gradient_start)
{
    uint32_t i = 0,j = 0;
    ushort     k = gradient_start;

    UiLcdHy28_OpenBulkWrite(x,Length,y,1);
    UiLcdHy28_BulkPixel_BufferInit();
    for(i = 0; i < Length; i++)
    {
        UiLcdHy28_BulkPixel_Put(RGB(k,k,k));
        j++;
        if(j == GRADIENT_STEP)
        {
            if(i < (Length/2))
                k += (GRADIENT_STEP/2);
            else
                k -= (GRADIENT_STEP/2);

            j = 0;
        }
    }
    UiLcdHy28_BulkPixel_BufferFlush();
    UiLcdHy28_CloseBulkWrite();
}

void UiLcdHy28_DrawEmptyRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color)
{
    UiLcdHy28_DrawStraightLine(Xpos, (Ypos),          Width,        LCD_DIR_HORIZONTAL,color);
    UiLcdHy28_DrawStraightLine(Xpos, Ypos,            Height,       LCD_DIR_VERTICAL,color);
    UiLcdHy28_DrawStraightLine((Xpos + Width), Ypos,  (Height + 1), LCD_DIR_VERTICAL,color);
    UiLcdHy28_DrawStraightLine(Xpos, (Ypos + Height), Width,        LCD_DIR_HORIZONTAL,color);
}

void UiLcdHy28_DrawBottomButton(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color)
{
    UiLcdHy28_DrawStraightLine(Xpos, (Ypos),        Width, LCD_DIR_HORIZONTAL,color);
    UiLcdHy28_DrawStraightLine(Xpos, Ypos,          Height,LCD_DIR_VERTICAL,  color);
    UiLcdHy28_DrawStraightLine((Xpos + Width), Ypos,Height,LCD_DIR_VERTICAL,  color);
}


static void UiLcdHy28_BulkWriteColor(uint16_t Color, uint32_t len)
{

#ifdef USE_SPI_DMA
    if(UiLcdHy28_SpiDisplayUsed())
    {
        int idx;

        UiLcdHy28_BulkPixel_BufferInit();

        for (idx = 0; idx < len; idx++)
        {
            UiLcdHy28_BulkPixel_Put(Color);
        }
        UiLcdHy28_BulkPixel_BufferFlush();
    }
    else
#endif
    {
    	uint32_t i = len;
#ifdef USE_GFX_RA8875
    	if(mchf_display.DeviceCode==0x8875)
    	{
    		for (uint32_t i = len; i; i--)
    		{
    			UiLcdHy28_WriteDataOnlyRA8875(Color);
    		}
    	}
    	else
#endif
    	{
    		for (; i; i--)
    		{
    			UiLcdHy28_WriteDataOnly(Color);
    		}
    	}
    }
}


#ifdef USE_8bit_FONT
static void UiLcdHy28_DrawChar_8bit(ushort x, ushort y, char symb,ushort Color, ushort bkColor,const sFONT *cf)
{

    const uint16_t charIdx = (symb >= 0x20 && symb < cf->maxCode)? cf->offsetTable[symb - cf->firstCode] : 0xFFFF;

    const uint8_t Font_H = cf->Height;
    symbolData_t* sym_ptr = charIdx == 0xFFFF? NULL:((symbolData_t*)cf->table)+charIdx;

    const uint8_t Font_W = sym_ptr == NULL? cf->Width:sym_ptr->width;

    const uint16_t charSpacing = cf->Spacing;

    UiLcdHy28_BulkPixel_OpenWrite(x, Font_W+charSpacing, y, Font_H);

    if(sym_ptr == NULL) // NON EXISTING SYMBOL
    {
        for(int cntrY=0;cntrY < Font_H; cntrY++)
        {
            for(int cntrX=0; cntrX < Font_W; cntrX++)
            {
                UiLcdHy28_BulkPixel_Put(Color);
            }
            for(int cntrX=0; cntrX < charSpacing; cntrX++)
            {
                UiLcdHy28_BulkPixel_Put(bkColor);
            }
        }
    }
    else
    {
        //gray shaded font type
        const int32_t ColBG_R=(bkColor>>11)&0x1f;
        const int32_t ColBG_G=(bkColor>>5)&0x3f;
        const int32_t ColBG_B=bkColor&0x1f;

        const int32_t ColFG_R=((Color>>11)&0x1f) - ColBG_R; //decomposition of 16 bit color data into channels
        const int32_t ColFG_G=((Color>>5)&0x3f)  - ColBG_G;
        const int32_t ColFG_B=(Color&0x1f) - ColBG_B;

        uint8_t *FontData=(uint8_t*)sym_ptr->data;

        for(uint8_t cntrY=0;cntrY<Font_H;cntrY++)
        {
            for(uint8_t cntrX=0;cntrX<Font_W;cntrX++)
            {
                uint32_t pixel;
                uint8_t FontD;

                if(cntrY<Font_H)
                {
                    FontD=*FontData++;      //get one point from bitmap
                }
                else
                {
                    FontD=0;
                }

                if(FontD==0)
                {
                    pixel=bkColor;
                }
                else
                {
                    //shading the foreground colour
                    int32_t ColFG_Ro=(ColFG_R*FontD)>>8;
                    int32_t ColFG_Go=(ColFG_G*FontD)>>8;
                    int32_t ColFG_Bo=(ColFG_B*FontD)>>8;
                    ColFG_Ro+=ColBG_R;
                    ColFG_Go+=ColBG_G;
                    ColFG_Bo+=ColBG_B;

                    pixel=(ColFG_Ro<<11)|(ColFG_Go<<5)|ColFG_Bo;    //assembly of destination colour
                }
                UiLcdHy28_BulkPixel_Put(pixel);
            }

            // add spacing behind the character data
            for(int n=Font_W; n < Font_W + charSpacing ; n++)
            {
                UiLcdHy28_BulkPixel_Put(bkColor);
            }
        }
    }

    UiLcdHy28_BulkPixel_BufferFlush();
    // flush all not yet  transferred pixel to display.
    UiLcdHy28_CloseBulkWrite();

}
#endif

static void UiLcdHy28_DrawChar_1bit(ushort x, ushort y, char symb,ushort Color, ushort bkColor,const sFONT *cf)
{
    uint8_t   *ch = (uint8_t *)cf->table;

    // we get the address of the begin of the character table
    // we support one or two byte long character definitions
    // anything wider than 8 pixels uses two bytes
    ch+=(symb - 32) * cf->Height* ((cf->Width>8) ? 2 : 1 );

    UiLcdHy28_OpenBulkWrite(x,cf->Width,y,cf->Height);
    UiLcdHy28_BulkPixel_BufferInit();

    // we now get the pixel information line by line
    for(uint32_t i = 0; i < cf->Height; i++)
    {
        uint32_t line_data; // stores pixel data for a character line, left most pixel is MSB

        // we read the current pixel line data (1 or 2 bytes)
        if(cf->Width>8)
        {
            if (cf->Width <= 12)
            {
                // small fonts <= 12 pixel width have left most pixel as MSB
                // we have to reverse that
                line_data  = ch[i*2+1]<<24;
                line_data |= ch[i*2] << 16;
            }
            else
            {
                uint32_t interim;
                interim  = ch[i*2+1]<<8;
                interim |= ch[i*2];

                line_data = __RBIT(interim); // rbit reverses a 32bit value bitwise
            }
        }
        else
        {
            // small fonts have left most pixel as MSB
            // we have to reverse that
            line_data = ch[i] << 24; // rbit reverses a 32bit value bitwise
        }

        // now go through the data pixel by pixel
        // and find out if it is background or foreground
        // then place pixel color in buffer
        uint32_t mask = 0x80000000U; // left most pixel aka MSB 32 bit mask

        for(uint32_t j = 0; j < cf->Width; mask>>=1, j++)
        {
            UiLcdHy28_BulkPixel_Put((line_data & mask) != 0 ? Color : bkColor);
            // we shift the mask in the for loop to the right one by one
        }
    }
    UiLcdHy28_BulkPixel_BufferFlush();
    // flush all not yet  transferred pixel to display.
    UiLcdHy28_CloseBulkWrite();
}

void UiLcdHy28_DrawChar(ushort x, ushort y, char symb,ushort Color, ushort bkColor,const sFONT *cf)
{
#ifdef USE_8bit_FONT
	switch(cf->BitCount)
	{
	case 1:		//1 bit font (basic type)
#endif
	    UiLcdHy28_DrawChar_1bit(x, y, symb, Color, bkColor, cf);
#ifdef USE_8bit_FONT
		break;
	case 8:	//8 bit grayscaled font
        UiLcdHy28_DrawChar_8bit(x, y, symb, Color, bkColor, cf);
	    break;
	}
#endif

}

const sFONT   *UiLcdHy28_Font(uint8_t font)
{
    // if we have an illegal font number, we return the first font
    return fontList[font < fontCount ? font : 0];
}

static void UiLcdHy28_PrintTextLen(uint16_t XposStart, uint16_t YposStart, const char *str, const uint16_t len, const uint32_t clr_fg, const uint32_t clr_bg,uchar font)
{
	uint32_t MAX_X=mchf_display.MAX_X; uint32_t MAX_Y=mchf_display.MAX_Y;
    const sFONT   *cf = UiLcdHy28_Font(font);
    int8_t Xshift =  cf->Width - ((cf->Width == 8 && cf->Height == 8)?1:0);
    // Mod the 8x8 font - the shift is too big

    uint16_t XposCurrent = XposStart;
    uint16_t YposCurrent = YposStart;

    if (str != NULL)
    {
        for (uint16_t idx = 0; idx < len; idx++)
        {
            uint8_t TempChar = *str++;

            UiLcdHy28_DrawChar(XposCurrent, YposCurrent, TempChar,clr_fg,clr_bg,cf);

            if(XposCurrent < (MAX_X - Xshift))
            {
                XposCurrent += Xshift;
            }
            else if (YposCurrent < (MAX_Y - cf->Height))
            {
                XposCurrent  = XposStart;
                YposCurrent += cf->Height;
            }
            else
            {
                XposCurrent = XposStart;
                YposCurrent = XposStart;
            }
        }
    }
}


/**
 * @returns pointer to next end of line or next end of string character
 */
static const char * UiLcdHy28_StringGetLine(const char* str)
{

    const char* retval;

    for (retval = str; *retval != '\0' && *retval != '\n'; retval++ );
    return retval;
}
/**
 * @brief Print multi-line text. New lines start right at XposStart
 * @returns next unused Y line (i.e. the Y coordinate just below the last printed text line).
 */
uint16_t UiLcdHy28_PrintText(uint16_t XposStart, uint16_t YposStart, const char *str,const uint32_t clr_fg, const uint32_t clr_bg,uchar font)
{
    const sFONT   *cf = UiLcdHy28_Font(font);
    int8_t Yshift =  cf->Height;

    uint16_t YposCurrent = YposStart;

    if (str != NULL)
    {
        const char* str_start = str;

        for (const char* str_end = UiLcdHy28_StringGetLine(str_start); str_start != str_end; str_end = UiLcdHy28_StringGetLine(str_start))
        {
            UiLcdHy28_PrintTextLen(XposStart, YposCurrent, str_start, str_end - str_start, clr_fg, clr_bg, font);
            YposCurrent += Yshift;
            if (*str_end == '\n')
            {
                // next character after line break
                str_start = str_end + 1;
            }
            else
            {
                // last character in string
                //str_start = str_end;
            	break;					//this line was added to prevent a kind of race condition causing random newline print of characters (for example in CW decoder). It needs testing.
            							//Feb 2018 SP9BSL
            }
        }
    }
    return YposCurrent;
}


uint16_t UiLcdHy28_TextHeight(uint8_t font)
{

    const sFONT   *cf = UiLcdHy28_Font(font);
    return cf->Height;
}

#if 0
/**
 * @returns pixel width of a given char (only used pixel!)
 */
uint16_t UiLcdHy28_CharWidth(const char c, uint8_t font)
{


    const sFONT   *cf = UiLcdHy28_Font(font);
    uint16_t retval;

#ifdef USE_8bit_FONT
    switch(cf->BitCount)
    {
    case 1:     //1 bit font (basic type)
#endif
        retval = UiLcdHy28_CharWidth_1bit(c, cf);
#ifdef USE_8bit_FONT
        break;
    case 8: //8 bit grayscaled font
        retval = UiLcdHy28_CharWidth_8bit(c, cf);
        break;
    }
#endif


    return retval;
}
#endif

/**
 * @returns pixelwidth of a text of given length
 */
static uint16_t UiLcdHy28_TextWidthLen(const char *str_start, uint16_t len, uint8_t font)
{

    const sFONT   *cf = UiLcdHy28_Font(font);

    int8_t char_width =  (cf->Width + cf->Spacing) - ((cf->Width == 8 && cf->Height == 8)?1:0);

    return (str_start != NULL) ? (len * char_width)  : 0;
}


/**
 * @returns pixelwidth of a text of given length or 0 for NULLPTR
 */
uint16_t UiLcdHy28_TextWidth(const char *str_start, uchar font)
{
    return (str_start != NULL) ?
            UiLcdHy28_TextWidthLen(str_start,strlen(str_start), font)
            :
            0;
}

static void UiLcdHy28_PrintTextRightLen(uint16_t Xpos, uint16_t Ypos, const char *str, uint16_t len, const uint32_t clr_fg, const uint32_t clr_bg,uint8_t font)
{

    uint16_t Xwidth = UiLcdHy28_TextWidthLen(str, len, font);
    if (Xpos < Xwidth )
    {
        Xpos = 0; // TODO: Overflow is not handled too well, just start at beginning of line and draw over the end.
    }
    else
    {
        Xpos -= Xwidth;
    }
    UiLcdHy28_PrintTextLen(Xpos, Ypos, str, len, clr_fg, clr_bg, font);
}

/**
 * @brief Print multi-line text right aligned. New lines start right at XposStart
 * @returns next unused Y line (i.e. the Y coordinate just below the last printed text line).
 */
uint16_t UiLcdHy28_PrintTextRight(uint16_t XposStart, uint16_t YposStart, const char *str,const uint32_t clr_fg, const uint32_t clr_bg,uint8_t font)
{
    // this code is a full clone of the PrintText function, with exception of the function call to PrintTextRightLen
    const sFONT   *cf = UiLcdHy28_Font(font);
    int8_t Yshift =  cf->Height;

    uint16_t YposCurrent = YposStart;

    if (str != NULL)
    {
        const char* str_start = str;

        for (const char* str_end = UiLcdHy28_StringGetLine(str_start); str_start != str_end; str_end = UiLcdHy28_StringGetLine(str_start))
        {
            UiLcdHy28_PrintTextRightLen(XposStart, YposCurrent, str_start, str_end - str_start, clr_fg, clr_bg, font);
            YposCurrent += Yshift;
            if (*str_end == '\n')
            {
                // next character after line break
                str_start = str_end + 1;
            }
            else
            {
                // last character in string
                str_start = str_end;
            }
        }
    }
    return YposCurrent;
}

static void UiLcdHy28_PrintTextCenteredLen(const uint16_t XposStart,const uint16_t YposStart,const uint16_t bbW,const char* str, uint16_t len ,uint32_t clr_fg,uint32_t clr_bg,uint8_t font)
{
    const uint16_t bbH = UiLcdHy28_TextHeight(font);
    const uint16_t txtW = UiLcdHy28_TextWidthLen(str, len, font);
    const uint16_t bbOffset = txtW>bbW?0:((bbW - txtW)+1)/2;

    // we draw the part of the box not used by text.
    if (bbOffset)
    {
        UiLcdHy28_DrawFullRect(XposStart,YposStart,bbH,bbOffset,clr_bg);
    }

    UiLcdHy28_PrintTextLen((XposStart + bbOffset),YposStart,str, len, clr_fg,clr_bg,font);

    // if the text is smaller than the box, we need to draw the end part of the
    // box
    if (txtW<bbW)
    {
        UiLcdHy28_DrawFullRect(XposStart+txtW+bbOffset,YposStart,bbH,bbW-(bbOffset+txtW),clr_bg);
    }
}

/*
 * Print text centered inside the bounding box. Using '\n' to print multline text
 */
uint16_t UiLcdHy28_PrintTextCentered(const uint16_t XposStart,const uint16_t YposStart,const uint16_t bbW,const char* str,uint32_t clr_fg,uint32_t clr_bg,uint8_t font)
{
    // this code is a full clone of the PrintText function, with exception of the function call to PrintTextCenteredLen
    const sFONT   *cf = UiLcdHy28_Font(font);
    int8_t Yshift =  cf->Height;

    uint16_t YposCurrent = YposStart;

    if (str != NULL)
    {
        const char* str_start = str;

        for (const char* str_end = UiLcdHy28_StringGetLine(str_start); str_start != str_end; str_end = UiLcdHy28_StringGetLine(str_start))
        {
            UiLcdHy28_PrintTextCenteredLen(XposStart, YposCurrent, bbW, str_start, str_end - str_start, clr_fg, clr_bg, font);
            YposCurrent += Yshift;
            if (*str_end == '\n')
            {
                // next character after line break
                str_start = str_end + 1;
            }
            else
            {
                // last character in string
                str_start = str_end;
            }
        }
    }
    return YposCurrent;
}



/*********************************************************************
 *
 * Controller Specific Functions Go Here (
 * These functions are used via mchf_display.func(...)
 * Each controller gets one single section here, guarded with USE_GFX_...
 *
 * *******************************************************************/

#ifdef USE_GFX_RA8875
static void UiLcdHy28_SetCursorA_RA8875( unsigned short Xpos, unsigned short Ypos )
{
    UiLcdRa8875_WriteReg_16bit(0x46, Xpos);
    UiLcdRa8875_WriteReg_16bit(0x48, Ypos);
}

static void UiLcdHy28_WriteRAM_Prepare_RA8875()
{
    UiLcdHy28_WriteRAM_Prepare_Index(0x02);
}

void UiLcdRA8875_SetForegroundColor(uint16_t Color)
{
    UiLcdRa8875_WriteReg_8bit(0x63, (uint16_t) (Color >> 11)); /* ra8875_red */
    UiLcdRa8875_WriteReg_8bit(0x64, (uint16_t) (Color >> 5)); /* ra8875_green */
    UiLcdRa8875_WriteReg_8bit(0x65, (uint16_t) (Color)); /* ra8875_blue */
}

static void UiLcdHy28_SetActiveWindow_RA8875(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom)
{
    /* setting active window X */
    UiLcdRa8875_WriteReg_16bit(0x30, XLeft);
    UiLcdRa8875_WriteReg_16bit(0x34, XRight);

    /* setting active window Y */
    UiLcdRa8875_WriteReg_16bit(0x32, YTop);
    UiLcdRa8875_WriteReg_16bit(0x36, YBottom);
}

static uint16_t UiLcdHy28_ReadDisplayId_RA8875()
{
	uint16_t retval =0;
	uint16_t reg_63_val=UiLcdHy28_ReadReg(0x63);
	uint16_t reg_64_val=UiLcdHy28_ReadReg(0x64);
	uint16_t reg_65_val=UiLcdHy28_ReadReg(0x65);
	uint16_t reg_88_val=UiLcdHy28_ReadReg(0x88);
	uint16_t reg_89_val=UiLcdHy28_ReadReg(0x89);

	if((reg_63_val==0x1f)&&
			(reg_64_val==0x3f)&&
			(reg_65_val==0x1f)&&
			(reg_88_val==0x07)&&
			(reg_89_val==0x03))
	{
		retval=0x8875;
		mchf_display.reg_info  = &ra8875_regs;
	}

	return retval;
}
#endif

#ifdef USE_GFX_ILI9486

static uint16_t UiLcdHy28_ReadDisplayId_ILI9486()
{
    uint16_t retval = 0x9486;

#ifdef USE_DISPLAY_PAR
    // we can't read the id from SPI if it is the dumb RPi SPI
    if (mchf_display.use_spi == false)
    {
        retval = UiLcdHy28_ReadReg(0xd3);
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
void UiLcdHy28_WriteIndexSpi_Prepare_ILI9486()
{
    GPIO_ResetBits(LCD_RS_PIO, LCD_RS);
}

static void UiLcdHy28_SetCursorA_ILI9486( unsigned short Xpos, unsigned short Ypos )
{
}

static void UiLcdHy28_WriteRAM_Prepare_ILI9486()
{
    UiLcdHy28_WriteRAM_Prepare_Index(0x2c);
}

static void UiLcdHy28_SetActiveWindow_ILI9486(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom)
{
    UiLcdHy28_WriteReg(0x2a,XLeft>>8);
    UiLcdHy28_WriteData(XLeft&0xff);
    UiLcdHy28_WriteData((XRight)>>8);
    UiLcdHy28_WriteData((XRight)&0xff);

    UiLcdHy28_WriteReg(0x2b,YTop>>8);
    UiLcdHy28_WriteData(YTop&0xff);
    UiLcdHy28_WriteData((YBottom)>>8);
    UiLcdHy28_WriteData((YBottom)&0xff);
}
#endif

#ifdef USE_GFX_ILI932x


static uint16_t UiLcdHy28_ReadDisplayId_ILI932x()
{
    uint16_t retval = UiLcdHy28_ReadReg(0x00);
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
    UiLcdHy28_SpiSendByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
}

void UiLcdHy28_WriteIndexSpi_Prepare_ILI932x()
{
    UiLcdHy28_SpiSendByte(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0       */
}

static void UiLcdHy28_SetCursorA_ILI932x( unsigned short Xpos, unsigned short Ypos )
{
    UiLcdHy28_WriteReg(0x20, Ypos );
    UiLcdHy28_WriteReg(0x21, Xpos );
}

static void UiLcdHy28_WriteRAM_Prepare_ILI932x()
{
    UiLcdHy28_WriteRAM_Prepare_Index(0x22);
}

static void UiLcdHy28_SetActiveWindow_ILI932x(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom)
{
    UiLcdHy28_WriteReg(0x52, XLeft);    // Horizontal GRAM Start Address
    UiLcdHy28_WriteReg(0x53, XRight);    // Horizontal GRAM End Address  -1

    UiLcdHy28_WriteReg(0x50, YTop);    // Vertical GRAM Start Address
    UiLcdHy28_WriteReg(0x51, YBottom);    // Vertical GRAM End Address    -1
}
#endif

static void UiLcdHy28_SendRegisters(const RegisterValueSetInfo_t* reg_info)
{
    for (uint16_t idx = 0; idx < reg_info->size; idx++)
    {
        switch(reg_info->addr[idx].reg)
        {
        case REGVAL_DELAY:
            HAL_Delay(reg_info->addr[idx].val);
            break;
        case REGVAL_DATA:
            UiLcdHy28_WriteData(reg_info->addr[idx].val);
            break;
        default:
            // TODO: Decide how we handle 16 vs. 8 bit writes here
            // I would propose either per register setting or per register set setting
            UiLcdHy28_WriteReg(reg_info->addr[idx].reg, reg_info->addr[idx].val);
        }
    }
}


#ifdef USE_GFX_SSD1289

static uint16_t UiLcdHy28_ReadDisplayId_SSD1289()
{
    uint16_t retval = UiLcdHy28_ReadReg(0x00);
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
     UiLcdHy28_WriteReg(0x44, XRight << 8 | XLeft);    // Horizontal GRAM Start Address
     UiLcdHy28_WriteReg(0x45, YTop);    // Horizontal GRAM End Address  -1

     UiLcdHy28_WriteReg(0x45, YTop);    // Vertical GRAM Start Address
     UiLcdHy28_WriteReg(0x46, YBottom);    // Vertical GRAM End Address    -1
}
static void UiLcdHy28_SetCursorA_SSD1289( unsigned short Xpos, unsigned short Ypos )
{
    UiLcdHy28_WriteReg(0x4e, Ypos );
    UiLcdHy28_WriteReg(0x4f, Xpos );
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
				.WriteReg = UiLcdHy28_WriteReg_ILI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_ILI,
				.DrawFullRect = UiLcdHy28_DrawFullRect_ILI,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_ILI,
        },
#endif
#ifdef USE_GFX_ILI932x
        {
                DISPLAY_HY28B_PARALLEL, "HY28A/B Para.",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_ILI932x,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_ILI932x,
                .SetCursorA = UiLcdHy28_SetCursorA_ILI932x,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI932x,
				.WriteReg = UiLcdHy28_WriteReg_ILI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_ILI,
				.DrawFullRect = UiLcdHy28_DrawFullRect_ILI,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_ILI,
        },
#ifdef USE_GFX_SSD1289
        {
                DISPLAY_HY32D_PARALLEL_SSD1289, "HY32D Para. SSD1289",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_SSD1289,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_SSD1289,
                .SetCursorA = UiLcdHy28_SetCursorA_SSD1289,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI932x,
				.WriteReg = UiLcdHy28_WriteReg_ILI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_ILI,
				.DrawFullRect = UiLcdHy28_DrawFullRect_ILI,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_ILI,
        },
#endif
#endif
#ifdef USE_GFX_RA8875
        {
                DISPLAY_RA8875_PARALLEL, "RA8875 Para.",
				.ReadDisplayId = UiLcdHy28_ReadDisplayId_RA8875,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_RA8875,
                .SetCursorA = UiLcdHy28_SetCursorA_RA8875,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_RA8875,
				.WriteReg = UiLcdHy28_WriteRegRA8875,
				.ReadReg = UiLcdHy28_ReadRegRA8875,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_RA8875,
				.DrawFullRect = UiLcdHy28_DrawFullRect_RA8875,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_RA8875,

        },
#endif
#endif

#if  defined(USE_SPI_DISPLAY)
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
				.WriteReg = UiLcdHy28_WriteReg_ILI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_ILI,
				.DrawFullRect = UiLcdHy28_DrawFullRect_ILI,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_ILI,
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
				.WriteReg = UiLcdHy28_WriteReg_ILI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_ILI,
				.DrawFullRect = UiLcdHy28_DrawFullRect_ILI,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_ILI,
                .is_spi = true,
				.spi_speed=false
        },
#endif
#ifdef USE_GFX_RA8875
		//currently RA8875 parallel interface supported only
/*        {
                DISPLAY_RA8875_SPI, "RA8875 SPI",
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_RA8875,
                .SetCursorA = UiLcdHy28_SetCursorA_RA8875,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_RA8875,
                .WriteDataSpiStart_Prepare = UiLcdHy28_WriteDataSpiStart_Prepare_RA8875,
                .WriteIndexSpi_Prepare = UiLcdHy28_WriteIndexSpi_Prepare_RA8875,
  				.WriteReg = UiLcdHy28_WriteRegRA8875,
				.ReadReg = UiLcdHy28_ReadRegRA8875,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_RA8875,
				.DrawFullRect = UiLcdHy28_DrawFullRect_RA8875,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_RA8875,
                .spi_cs_port = LCD_CSA_PIO,
                .spi_cs_pin = LCD_CSA,
                true, false },*/
#endif
#if defined(USE_GFX_ILI9486)
        {       DISPLAY_RPI_SPI, "RPi 3.5 SPI",
                .ReadDisplayId = UiLcdHy28_ReadDisplayId_ILI9486,
                .SetActiveWindow = UiLcdHy28_SetActiveWindow_ILI9486,
                .SetCursorA = UiLcdHy28_SetCursorA_ILI9486,
                .WriteRAM_Prepare = UiLcdHy28_WriteRAM_Prepare_ILI9486,
                .WriteDataSpiStart_Prepare = UiLcdHy28_WriteDataSpiStart_Prepare_ILI9486,
                .WriteIndexSpi_Prepare = UiLcdHy28_WriteIndexSpi_Prepare_ILI9486,
				.WriteReg = UiLcdHy28_WriteReg_ILI,
				.ReadReg = UiLcdHy28_ReadRegILI,
				.DrawStraightLine = UiLcdHy28_DrawStraightLine_ILI,
				.DrawFullRect = UiLcdHy28_DrawFullRect_ILI,
				.DrawColorPoint = UiLcdHy28_DrawColorPoint_ILI,
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


static uint16_t UiLcdHy28_DetectController(const uhsdr_display_info_t* disp_info_ptr)
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
        UiLcdHy28_GpioInit(disp_info_ptr->display_type);

        if (mchf_display.use_spi == true)
        {
#ifdef USE_SPI_DISPLAY
            UiLcdHy28_SpiInit(disp_info_ptr->spi_speed, disp_info_ptr->display_type);
#endif
        }
        else
        {
#ifdef USE_DISPLAY_PAR
            UiLcdHy28_ParallelInit();
#endif
        }

        UiLcdHy28_Reset();

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
            UiLcdHy28_SendRegisters(mchf_display.reg_info);
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
                #ifdef USE_SPI_DISPLAY
                UiLcdHy28_SpiDeInit();
                #endif
            }
            else
            {
                #ifdef USE_DISPLAY_PAR
                UiLcdHy28_ParallelDeInit();
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

uint8_t UiLcdHy28_Init()
{
    uint8_t retval = DISPLAY_NONE;
    mchf_display.DeviceCode = 0x0000;
    UiLcdHy28_BacklightInit();

    for (uint16_t disp_idx = 1; retval == DISPLAY_NONE && display_infos[disp_idx].display_type != DISPLAY_NUM; disp_idx++)
    {
        mchf_display.DeviceCode = UiLcdHy28_DetectController(&display_infos[disp_idx]);

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



static inline void UiLcdHy28_SetSpiPrescaler(uint32_t baudrate_prescaler)
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

mchf_touchscreen_t mchf_touchscreen;

/*
 * @brief Called to run the touch detection state machine, results are stored in ts structure
 */
void UiLcdHy28_TouchscreenDetectPress()
{
    if (mchf_touchscreen.present)
    {
        if(!HAL_GPIO_ReadPin(TP_IRQ_PIO,TP_IRQ) && mchf_touchscreen.state != TP_DATASETS_PROCESSED)    // fetch touchscreen data if not already processed
            UiLcdHy28_TouchscreenReadCoordinates();

        if(HAL_GPIO_ReadPin(TP_IRQ_PIO,TP_IRQ) && mchf_touchscreen.state == TP_DATASETS_PROCESSED)     // clear statemachine when data is processed
        {
            mchf_touchscreen.state = TP_DATASETS_NONE;

            mchf_touchscreen.hr_x = mchf_touchscreen.hr_y = 0x7fff;
        }
    }
}
/*
 * @brief tells you that touchscreen coordinates are ready for processing and marks them as processed
 * @returns true if coordinates for processing are available and have been marked as processed, false otherwise
 */
bool UiLcdHy28_TouchscreenHasProcessableCoordinates()
{
	bool retval = false;
    UiLcdHy28_TouchscreenReadCoordinates();

    if(mchf_touchscreen.state >= TP_DATASETS_VALID && mchf_touchscreen.state != TP_DATASETS_PROCESSED)
    //if(mchf_touchscreen.state >= TP_DATASETS_WAIT && mchf_touchscreen.state != TP_DATASETS_PROCESSED)
    {
        mchf_touchscreen.state = TP_DATASETS_NONE;     // tp data processed
         retval = true;
    }
    return retval;
}


static inline void UiLcdHy28_TouchscreenStartSpiTransfer()
{
    // we only have to care about other transfers if the SPI is
    // use by the display as well
    if (UiLcdHy28_SpiDisplayUsed())
    {
        UiLcdHy28_FinishWaitBulkWrite();
    }
    UiLcdHy28_SetSpiPrescaler(SPI_PRESCALE_TS_DEFAULT);
    GPIO_ResetBits(TP_CS_PIO, TP_CS);
}

static inline void UiLcdHy28_TouchscreenFinishSpiTransfer()
{
    UiLcdHy28_SpiFinishTransfer();
    GPIO_SetBits(TP_CS_PIO, TP_CS);
    // we only have to care about other transfers if the SPI is
    // use by the display as well
    if (UiLcdHy28_SpiDisplayUsed())
    {
        UiLcdHy28_SetSpiPrescaler(lcd_spi_prescaler);
    }
}


/*
 * @brief Extracts touchscreen touch coordinates, counts how often same position is being read consecutively
 * @param do_translate false -> raw coordinates, true -> mapped coordinates according to calibration data
 */
#define XPT2046_PD_FULL 0x00
#define XPT2046_PD_REF  0x01
#define XPT2046_PD_ADC  0x02
#define XPT2046_PD_NONE 0x03

#define XPT2046_MODE_12BIT 0x00
#define XPT2046_MODE_8BIT  0x08

#define XPT2046_CH_DFR_Y    0x50
#define XPT2046_CH_DFR_X    0x10
#define XPT2046_CONV_START  0x80

#define  XPT2046_COMMAND_LEN 7

static void UiLcdHy28_TouchscreenReadData(uint16_t* x_p,uint16_t* y_p)
{


    static const uint8_t xpt2046_command[XPT2046_COMMAND_LEN] =
    {
            XPT2046_CONV_START|XPT2046_CH_DFR_X|XPT2046_MODE_12BIT|XPT2046_PD_REF,
            0,  XPT2046_CONV_START|XPT2046_CH_DFR_Y|XPT2046_MODE_12BIT|XPT2046_PD_REF,  // the measurement for first command is here, we discard this
            0,  XPT2046_CONV_START|XPT2046_CH_DFR_X|XPT2046_MODE_12BIT|XPT2046_PD_FULL, // y measurement from previous command, next command turns off power
            0, 0                                                                        // x measurement from previous command
    };

    uint8_t xpt_response[XPT2046_COMMAND_LEN];

    UiLcdHy28_TouchscreenStartSpiTransfer();

    HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)xpt2046_command, xpt_response,XPT2046_COMMAND_LEN,SPI_TIMEOUT);

    UiLcdHy28_TouchscreenFinishSpiTransfer();

    *x_p = (xpt_response[5] << 8 | xpt_response[6]) >> 3;
    *y_p = (xpt_response[3] << 8 | xpt_response[4]) >> 3;

}
#define HIRES_TOUCH_MaxDelta 2
#define HIRES_TOUCH_MaxFocus 4

void UiLcdHy28_TouchscreenReadCoordinates()
{

    /*
    statemachine stati:
    TP_DATASETS_NONE = no touchscreen action detected
    TP_DATASETS_WAIT 1 = first touchscreen press
    >1 = x times valid data available
    TP_DATASETS_PROCESSED 0xff = data was already processed by calling function
     */


	if(mchf_touchscreen.state < TP_DATASETS_VALID)	// no valid data ready or data ready to process
	{
		if(mchf_touchscreen.state > TP_DATASETS_NONE && mchf_touchscreen.state < TP_DATASETS_VALID)	// first pass finished, get data
		{

			UiLcdHy28_TouchscreenReadData(&mchf_touchscreen.xraw,&mchf_touchscreen.yraw);

			//delta/focus algorithm for filtering the noise from touch panel data
			//based on LM8300/LM8500 datasheet

			//first calculating the delta algorithm

			int16_t TS_dx,TS_dy, TS_predicted_x, TS_predicted_y, NewDeltaX, NewDeltaY;
			TS_dx=mchf_touchscreen.xraw_m1-mchf_touchscreen.xraw_m2;
			TS_dy=mchf_touchscreen.yraw_m1-mchf_touchscreen.yraw_m2;
			TS_predicted_x=mchf_touchscreen.yraw_m1+TS_dx;
			TS_predicted_y=mchf_touchscreen.yraw_m1+TS_dy;

			NewDeltaX=TS_predicted_x-mchf_touchscreen.xraw;
			NewDeltaY=TS_predicted_y-mchf_touchscreen.yraw;

			if(NewDeltaX<0)
				NewDeltaX=-NewDeltaX;

			if(NewDeltaY<0)
				NewDeltaX=-NewDeltaY;

			if((NewDeltaX<=HIRES_TOUCH_MaxDelta) && (NewDeltaY<=HIRES_TOUCH_MaxDelta))
			{

				//ok, the delta algorithm filtered out spikes and the bigger noise
				//now we perform focus algorithm

				NewDeltaX=mchf_touchscreen.focus_xprev-mchf_touchscreen.xraw;
				NewDeltaY=mchf_touchscreen.focus_yprev-mchf_touchscreen.yraw;

				if(NewDeltaX<0)
					NewDeltaX=-NewDeltaX;

				if(NewDeltaY<0)
					NewDeltaX=-NewDeltaY;

				if((NewDeltaX<=HIRES_TOUCH_MaxFocus) && (NewDeltaY<=HIRES_TOUCH_MaxFocus))
				{
					mchf_touchscreen.xraw=mchf_touchscreen.focus_xprev;
					mchf_touchscreen.yraw=mchf_touchscreen.focus_yprev;
				}
				else
				{
					mchf_touchscreen.focus_xprev=mchf_touchscreen.xraw;
					mchf_touchscreen.focus_yprev=mchf_touchscreen.yraw;
				}


				mchf_touchscreen.state=TP_DATASETS_VALID;

				int32_t x,y;

				x=mchf_touchscreen.xraw;
				y=mchf_touchscreen.yraw;

				int32_t xn,yn;
				//transforming the coordinates by calibration coefficients calculated in touchscreen calibration
				//see the UiDriver_TouchscreenCalibration
				//xn=Ax+By+C
				//yn=Dx+Ey+F
				//all coefficients are in format 16.16
				xn=mchf_touchscreen.cal[0]*x+mchf_touchscreen.cal[1]*y+mchf_touchscreen.cal[2];
				yn=mchf_touchscreen.cal[3]*x+mchf_touchscreen.cal[4]*y+mchf_touchscreen.cal[5];

				xn>>=16;
				yn>>=16;

				mchf_touchscreen.hr_x=(int16_t)xn;
				mchf_touchscreen.hr_y=(int16_t)yn;


			}
			else
			{
				mchf_touchscreen.xraw_m2=mchf_touchscreen.xraw_m1;
				mchf_touchscreen.yraw_m2=mchf_touchscreen.yraw_m1;
				mchf_touchscreen.xraw_m1=mchf_touchscreen.xraw;
				mchf_touchscreen.yraw_m1=mchf_touchscreen.yraw;
				mchf_touchscreen.state = TP_DATASETS_WAIT;
			}
        }
        else
        {
        	mchf_touchscreen.state = TP_DATASETS_WAIT;		// restart machine
        }
    }

}

bool UiLcdHy28_TouchscreenPresenceDetection()
{
    bool retval = false;
    uint16_t x = 0xffff, y = 0xffff;

    UiLcdHy28_TouchscreenReadData(&x,&y);
    UiLcdHy28_TouchscreenReadData(&x,&y);

    mchf_touchscreen.state = TP_DATASETS_PROCESSED;

    if(x != 0xffff && y != 0xffff && x != 0 && y != 0)
    {// touchscreen data valid?
        retval = true;                      // yes - touchscreen present!
    }
    return retval;
}

void UiLcdHy28_TouchscreenInit(uint8_t mirror)
{
    mchf_touchscreen.xraw = 0;
    mchf_touchscreen.yraw = 0;

    mchf_touchscreen.hr_x = 0x7FFF;                        // invalid position
    mchf_touchscreen.hr_y = 0x7FFF;                        // invalid position
    mchf_touchscreen.present = UiLcdHy28_TouchscreenPresenceDetection();
}
