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
#include "mchf_board_config.h"

#ifdef STM32F4
    #define USE_DISPLAY_SPI
#endif
#ifdef STM32F7
    #define USE_SPI_HAL
#endif

#define USE_DISPLAY_PAR
#define USE_SPI_DMA

// #define HY28BHISPEED true // does not work with touchscreen and HY28A and some HY28B



#include "spi.h"

#ifdef USE_DISPLAY_PAR

    #ifdef STM32F7
        #include "fmc.h"
        #define MEM_Init() MX_FMC_Init()
    #else
        #include "fsmc.h"
        #define MEM_Init() MX_FSMC_Init()
    #endif

    #define LCD_REG      (*((volatile unsigned short *) 0x60000000))

    #if defined(STM32F4)
    #define LCD_RAM      (*((volatile unsigned short *) 0x60020000))
    #elif defined(STM32F7)
    #define LCD_RAM      (*((volatile unsigned short *) 0x60004000))
    #endif
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ui_lcd_hy28_fonts.h"
#include "ui_lcd_hy28.h"

mchf_display_t mchf_display;


// ----------------------------------------------------------
// Dual purpose pins (parallel + serial)
#define LCD_CS                  LCD_CSA
#define LCD_CS_SOURCE           LCD_CSA_SOURCE
#define LCD_CS_PIO              LCD_CSA_PIO




// Saved fonts
extern sFONT GL_Font8x8;
extern sFONT GL_Font8x12;
extern sFONT GL_Font8x12_bold;
extern sFONT GL_Font8x12_bold_short;
extern sFONT GL_Font12x12;
extern sFONT GL_Font16x24;


static sFONT *fontList[] =
{
#ifndef BOOTLOADER_BUILD
        &GL_Font8x12_bold,
        &GL_Font16x24,
        &GL_Font12x12,
        &GL_Font8x12,
        &GL_Font8x8,
#else
        &GL_Font8x12_bold_short,
#endif
};

// we can do this here since fontList is an array variable not just a pointer!
static const uint8_t fontCount = sizeof(fontList)/sizeof(fontList[0]);


typedef struct  {
    uint16_t reg;
    uint16_t val;
} RegisterValue_t;

static const RegisterValue_t ili9320[] =
{
        {0xE5,0x8000},   // Set the internal vcore voltage
        {0x00,  0x0001},    // Start internal OSC.

        // Direction related
        {0x01,  0x0100},    // set SS and SM bit

        {0x02,  0x0700},    // set 1 line inversionc
        {0x03,  0x1038},    // set GRAM write direction and BGR=1 and ORG = 1.
        {0x04,  0x0000},    // Resize register
        {0x08,  0x0202},    // set the back porch and front porch
        {0x09,  0x0000},    // set non-display area refresh cycle ISC[3:0]
        {0x0A, 0x0000},    // FMARK function
        {0x0C, 0x0000},    // RGB interface setting
        {0x0D, 0x0000},    // Frame marker Position
        {0x0F, 0x0000},    // RGB interface polarity

        // Power On sequence
        {0x10, 0x0000},    // SAP, BT[3:0], AP, DSTB, SLP, STB
        {0x11, 0x0000},    // DC1[2:0], DC0[2:0], VC[2:0]
        {0x12, 0x0000},    // VREG1OUT voltage
        {0x13, 0x0000},    // VDV[4:0] for VCOM amplitude
        {0x00, 300},            // Dis-charge capacitor power voltage (300ms)
        {0x10, 0x17B0},    // SAP, BT[3:0], AP, DSTB, SLP, STB
        {0x11, 0x0137},    // DC1[2:0], DC0[2:0], VC[2:0]
        {0x00, 100},             // Delay 100 ms
        {0x12, 0x0139},    // VREG1OUT voltage
        {0x00, 100},             // Delay 100 ms
        {0x13, 0x1d00},    // VDV[4:0] for VCOM amplitude
        {0x29, 0x0013},    // VCM[4:0] for VCOMH
        {0x00, 100},             // Delay 100 ms
        {0x20, 0x0000},    // GRAM horizontal Address
        {0x21, 0x0000},    // GRAM Vertical Address

        // Adjust the Gamma Curve
        {0x30, 0x0007},
        {0x31, 0x0007},
        {0x32, 0x0007},
        {0x35, 0x0007},
        {0x36, 0x0007},
        {0x37, 0x0700},
        {0x38, 0x0700},
        {0x39, 0x0700},
        {0x3C, 0x0700},
        {0x3D, 0x1F00},

        // Set GRAM area
        {0x50, 0x0000},    // Horizontal GRAM Start Address
        {0x51, 0x00EF},    // Horizontal GRAM End Address
        {0x52, 0x0000},    // Vertical GRAM Start Address
        {0x53, 0x013F},    // Vertical GRAM End Address

        // Direction related
        {0x60,  0xA700},    // Gate Scan Line
        {0x61,  0x0001},    // NDL,VLE, REV

        {0x6A, 0x0000},    // set scrolling line

        // Partial Display Control
        {0x80, 0x0000},
        {0x81, 0x0000},
        {0x82, 0x0000},
        {0x83, 0x0000},
        {0x84, 0x0000},
        {0x85, 0x0000},

        // Panel Control
        {0x90, 0x0010},
        {0x92, 0x0000},
        {0x93, 0x0003},
        {0x95, 0x0110},
        {0x97, 0x0000},
        {0x98, 0x0000},

        // Set GRAM write direction
        {0x03, 0x1038},

        // 262K color and display ON
        {0x07, 0x0173},

        // delay 50 ms
        {0x00, 50},

};

static const RegisterValue_t spdfd5408b[] =
{

    {0x01,0x0000},   // (SS bit 8) - 0x0100 will flip 180 degree
    {0x02,0x0700},   // LCD Driving Waveform Contral
    {0x03,0x1038},   // Entry Mode (AM bit 3)

    {0x04,0x0000},   // Scaling Control register
    {0x08,0x0207},   // Display Control 2
    {0x09,0x0000},   // Display Control 3
    {0x0A,0x0000},   // Frame Cycle Control
    {0x0C,0x0000},   // External Display Interface Control 1
    {0x0D,0x0000},    // Frame Maker Position
    {0x0F,0x0000},   // External Display Interface Control 2
    {0x00, 50},
    {0x07,0x0101},   // Display Control
    {0x00, 50},
    {0x10,0x16B0},    // Power Control 1
    {0x11,0x0001},    // Power Control 2
    {0x17,0x0001},    // Power Control 3
    {0x12,0x0138},    // Power Control 4
    {0x13,0x0800},    // Power Control 5
    {0x29,0x0009},   // NVM read data 2
    {0x2a,0x0009},   // NVM read data 3
    {0xa4,0x0000},
    {0x50,0x0000},
    {0x51,0x00EF},
    {0x52,0x0000},
    {0x53,0x013F},

    {0x60,0x2700},   // Driver Output Control (GS bit 15)

    {0x61,0x0003},   // Driver Output Control
    {0x6A,0x0000},   // Vertical Scroll Control

    {0x80,0x0000},   // Display Position ?C Partial Display 1
    {0x81,0x0000},   // RAM Address Start ?C Partial Display 1
    {0x82,0x0000},   // RAM address End - Partial Display 1
    {0x83,0x0000},   // Display Position ?C Partial Display 2
    {0x84,0x0000},   // RAM Address Start ?C Partial Display 2
    {0x85,0x0000},   // RAM address End ?C Partail Display2
    {0x90,0x0013},   // Frame Cycle Control
    {0x92,0x0000},    // Panel Interface Control 2
    {0x93,0x0003},   // Panel Interface control 3
    {0x95,0x0110},   // Frame Cycle Control
    {0x07,0x0173},
};


static const RegisterValue_t ili932x[] =
{
    // NPI: {0xE5, 0x78F0},   // set SRAM internal timing I guess this is the relevant line for getting LCDs to work which are "out-of-specs"...
    {0x01,0x0000},    // set SS and SM bit
    {0x02,0x0700},    // set 1 line inversion
    {0x03,0x1038},    // set GRAM write direction and BGR=1 and ORG = 1
    {0x04,0x0000},    // resize register
    {0x08,0x0207},    // set the back porch and front porch
    {0x09,0x0000},    // set non-display area refresh cycle
    {0x0a,0x0000},    // FMARK function
    {0x0c,0x0001},    // RGB interface setting
    // NPI: {0x0c,0x0000},    // RGB interface setting
    {0x0d,0x0000},    // frame marker position
    {0x0f,0x0000},    // RGB interface polarity

    // Power On sequence
    {0x10,0x0000},    // SAP, BT[3:0], AP, DSTB, SLP, STB
    {0x11,0x0007},    // DC1[2:0], DC0[2:0], VC[2:0]
    {0x12,0x0000},    // VREG1OUT voltage
    {0x13,0x0000},    // VDV[4:0] for VCOM amplitude
    // NPI: {0x0c,0x0001},    // RGB interface setting
    {0x00, 200},         // delay 200 ms
    {0x10,0x1590},    // SAP, BT[3:0], AP, DSTB, SLP, STB
    // NPI: {0x10, 0x1090}, // SAP, BT[3:0], AP, DSTB, SLP, STB
    {0x11,0x0227},    // set DC1[2:0], DC0[2:0], VC[2:0]
    {0x00, 50},              // delay 50 ms
    {0x12,0x009c},    // internal reference voltage init
    // NPI: {0x12, 0x001F},
    {0x00, 50},              // delay 50 ms
    {0x13,0x1900},    // set VDV[4:0] for VCOM amplitude
    // NPI: {0x13, 0x1500},
    {0x29,0x0023},    // VCM[5:0] for VCOMH
    // NPI: {0x29,0x0027},    // VCM[5:0] for VCOMH
    {0x2b,0x000d},    // set frame rate: changed from 0e to 0d on 03/28/2016
    {0x00, 50},              // delay 50 ms
    {0x20,0x0000},    // GRAM horizontal address
    {0x21,0x0000},    // GRAM vertical address

//        /* NPI:
     // ----------- Adjust the Gamma Curve ----------
    {0x30, 0x0000},
    {0x31, 0x0707},
    {0x32, 0x0307},
    {0x35, 0x0200},
    {0x36, 0x0008},
    {0x37, 0x0004},
    {0x38, 0x0000},
    {0x39, 0x0707},
    {0x3C, 0x0002},
    {0x3D, 0x1D04},
//        */

    {0x50,0x0000},    // horizontal GRAM start address
    {0x51,0x00ef},    // horizontal GRAM end address
    {0x52,0x0000},    // vertical GRAM start address
    {0x53,0x013f},    // vertical GRAM end address
    {0x60,0xa700},    // gate scan line
    {0x61,0x0001},    // NDL, VLE, REV
    {0x6a,0x0000},    // set scrolling line
    // partial display control
    {0x80,0x0000},
    {0x81,0x0000},
    {0x82,0x0000},
    {0x83,0x0000},
    {0x84,0x0000},
    {0x85,0x0000},
    // panel control
    {0x90,0x0010},
    {0x92,0x0000},
    // NPI: {0x92, 0x0600},
    // activate display using 262k colours
    {0x07,0x0133},
};






const uint8_t touchscreentable [] = {0x07,0x09,
        0x0c,0x0d,0x0e,0x0f,0x12,0x13,0x14,0x15,0x16,0x18,
        0x1c,0x1d,0x1e,0x1f,0x22,0x23,0x24,0x25,0x26,0x27,
        0x2c,0x2d,0x2e,0x30,0x32,0x34,0x35,0x36,0x3a,0x3c,
        0x40,0x42,0x44,0x45,0x46,0x47,0x4c,0x4d,0x4e,0x52,
        0x54,0x55,0x56,0x5c,0x5d,0x60,0x62,0x64,0x65,0x66,
        0x67,0x6c,0x6d,0x6e,0x74,0x75,0x76,0x77,0x7c,0x7d
};

typedef struct
{
    uint16_t x;
    uint16_t width;
    uint16_t y;
    uint16_t height;
} lcd_bulk_transfer_header_t;

static void UiLcdHy28_BulkWriteColor(uint16_t Color, uint32_t len);


static inline bool UiLcdHy28_SpiDisplayUsed()
{
    bool retval = false;
#ifdef USE_DISPLAY_SPI
    retval = mchf_display.use_spi;
#endif
    return retval;
}


void UiLcdHy28_BacklightInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // Set as output
    GPIO_InitStructure.Pin = LCD_BACKLIGHT;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
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


static uint16_t lcd_spi_prescaler;

// static SPI_HandleTypeDef SPI_Handle;

void UiLcdHy28_SpiInit(bool hispeed)
{

    lcd_spi_prescaler = hispeed?SPI_BAUDRATEPRESCALER_2:SPI_BAUDRATEPRESCALER_4;

    GPIO_InitTypeDef GPIO_InitStructure;


    // Enable the SPI periph
    // the main init is already done earlier, we need this if we want to use our own code to access SPI
    __HAL_SPI_ENABLE(&hspi2);

#if 0

    // Common SPI settings
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;

    // SPI SCK pin configuration
    GPIO_InitStructure.Pin = LCD_SCK;
    HAL_GPIO_Init(LCD_SCK_PIO, &GPIO_InitStructure);

    // SPI  MOSI pins configuration
    GPIO_InitStructure.Pin =  LCD_MOSI;
    HAL_GPIO_Init(LCD_MOSI_PIO, &GPIO_InitStructure);

    // SPI  MISO pins configuration
    GPIO_InitStructure.Pin =  LCD_MISO;
    HAL_GPIO_Init(LCD_MISO_PIO, &GPIO_InitStructure);


    // SPI configuration
    SPI_Handle.Init.Direction		= SPI_DIRECTION_2LINES;
    SPI_Handle.Init.DataSize		= SPI_DATASIZE_8BIT;
    SPI_Handle.Init.CLKPolarity	= SPI_POLARITY_HIGH;
    SPI_Handle.Init.CLKPhase		= SPI_PHASE_2EDGE;
    SPI_Handle.Init.NSS			= SPI_NSS_SOFT;
    SPI_Handle.Init.BaudRatePrescaler	= lcd_spi_prescaler;   // max speed presc_8 with 50Mhz GPIO, max 4 with 100 Mhz
    SPI_Handle.Init.FirstBit		= SPI_FIRSTBIT_MSB;
    SPI_Handle.Init.Mode			= SPI_MODE_MASTER;
    SPI_Handle.Instance            = SPI2;
    HAL_SPI_DeInit(&SPI_Handle);
    HAL_SPI_Init(&SPI_Handle);
#endif
    // Common misc pins settings
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Pull = GPIO_NOPULL;

    // Configure GPIO PIN for Chip select
    GPIO_InitStructure.Pin = mchf_display.lcd_cs;
    HAL_GPIO_Init(mchf_display.lcd_cs_pio, &GPIO_InitStructure);

    // Configure GPIO PIN for Reset
    GPIO_InitStructure.Pin = LCD_RESET;
    HAL_GPIO_Init(LCD_RESET_PIO, &GPIO_InitStructure);

    // Deselect : Chip Select high
    GPIO_SetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}

DMA_HandleTypeDef DMA_Handle;


inline void UiLcdHy28_SpiDmaStop()
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
        HAL_SPI_Transmit_DMA(&hspi2,buffer,size);
    }
}

void UiLcdHy28_SpiDeInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    __HAL_SPI_DISABLE(&hspi2);
    HAL_SPI_DeInit(&hspi2);

    // Set as inputs
    GPIO_InitStructure.Mode		= GPIO_MODE_INPUT;
    GPIO_InitStructure.Speed	= GPIO_SPEED_LOW;
    GPIO_InitStructure.Pull		= GPIO_NOPULL;

#if 0
    // SPI SCK pin configuration
    GPIO_InitStructure.Pin		= LCD_SCK;
    HAL_GPIO_Init(LCD_SCK_PIO, &GPIO_InitStructure);

    // SPI  MOSI pins configuration
    GPIO_InitStructure.Pin		=  LCD_MOSI;
    HAL_GPIO_Init(LCD_MOSI_PIO, &GPIO_InitStructure);

    // SPI  MISO pins configuration
    GPIO_InitStructure.Pin		=  LCD_MISO;
    HAL_GPIO_Init(LCD_MISO_PIO, &GPIO_InitStructure);
#endif
    // Configure GPIO PIN for Chip select
    GPIO_InitStructure.Pin		= mchf_display.lcd_cs;
    HAL_GPIO_Init(mchf_display.lcd_cs_pio, &GPIO_InitStructure);
}

inline void UiLcdHy28_SpiLcdCsDisable() {
    GPIO_SetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}
inline void UiLcdHy28_SpiLcdCsEnable() {
    GPIO_ResetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}

void UiLcdHy28_ParallelInit()
{
}


void UiLcdHy28_Reset()
{
    // Reset
    GPIO_SetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(1);

    GPIO_ResetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(1);

    GPIO_SetBits(LCD_RESET_PIO, LCD_RESET);
    HAL_Delay(300);
}


void UiLcdHy28_FSMCConfig(void)
{
    MEM_Init();
}

#if 0
static inline void UiLcdHy28_SpiSendByteOld(uint8_t byte)
{
    // TODO: Find out why not working with HAL as expected
    // maybe we need only Transmit, don't know. Test with TP since this was
    // not working (detection failed)
    while (__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_TXE)  == RESET) {}
    // uint8_t dummy;
    // HAL_SPI_TransmitReceive(&hspi2, &byte, &dummy,1,0);
    HAL_SPI_Transmit(&hspi2, &byte,1,0);
}
#endif

static inline void UiLcdHy28_SpiSendByte(uint8_t byte)
{

#ifdef USE_SPI_HAL
    uint8_t dummy;
    HAL_SPI_TransmitReceive(&hspi2, &byte, &dummy,1,10);
#else
    while ((SPI2->SR & (SPI_FLAG_TXE)) == (uint16_t)RESET) {}
    SPI2->DR = byte;
    while ((SPI2->SR & (SPI_FLAG_RXNE)) == (uint16_t)RESET) {}
    byte = SPI2->DR;
#endif
}

uint8_t spi_dr_dummy; // used to make sure that DR is being read
static inline void UiLcdHy28_SpiFinishTransfer()
{
    while ((SPI2->SR & (SPI_FLAG_TXE)) == (uint16_t)RESET) {}
    while (SPI2->SR & SPI_FLAG_BSY) {}
    if (SPI2->SR & SPI_FLAG_RXNE) {
        spi_dr_dummy = SPI2->DR;
    }
}

static void UiLcdHy28_LcdSpiFinishTransfer()
{
    UiLcdHy28_SpiFinishTransfer();
    GPIO_SetBits(mchf_display.lcd_cs_pio, mchf_display.lcd_cs);
}

uint8_t UiLcdHy28_SpiReadByte(void)
{
    uint8_t dummy = 0;
    uint8_t retval = 0;

    /* Send a Transmit a dummy byte and Receive Byte through the SPI peripheral */
    HAL_SPI_TransmitReceive(&hspi2, &dummy,&retval,1,10);

    return retval;
}

uint8_t UiLcdHy28_SpiReadByteFast(void)
{
    uint8_t retval = 0;

#ifdef USE_SPI_HAL
    uint8_t dummy = 0;
    HAL_SPI_TransmitReceive(&hspi2, &dummy, &retval,1,10);
#else

    /* Send a Transmit a dummy byte and Receive Byte through the SPI peripheral */
    while ((SPI2->SR & (SPI_FLAG_TXE)) == (uint16_t)RESET) {}
    SPI2->DR = 0;
    while ((SPI2->SR & (SPI_FLAG_RXNE)) == (uint16_t)RESET) {}
    retval = SPI2->DR;
#endif
    return retval;
}

void UiLcdHy28_WriteIndexSpi(unsigned char index)
{
    UiLcdHy28_SpiLcdCsEnable();

    UiLcdHy28_SpiSendByte(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0       */
    UiLcdHy28_SpiSendByte(0);
    UiLcdHy28_SpiSendByte(index);

    UiLcdHy28_LcdSpiFinishTransfer();
}

void UiLcdHy28_WriteDataSpi( unsigned short data)
{
    UiLcdHy28_SpiLcdCsEnable();

    UiLcdHy28_SpiSendByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
    UiLcdHy28_SpiSendByte((data >>   8));                    /* Write D8..D15                */
    UiLcdHy28_SpiSendByte((data & 0xFF));                    /* Write D0..D7                 */

    UiLcdHy28_LcdSpiFinishTransfer();
}

static inline void UiLcdHy28_WriteDataSpiStart()
{
    UiLcdHy28_SpiLcdCsEnable();
    UiLcdHy28_SpiSendByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
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
        LCD_RAM = data;
        __DMB();

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

void UiLcdHy28_WriteReg(unsigned short LCD_Reg, unsigned short LCD_RegValue)
{
    if(UiLcdHy28_SpiDisplayUsed())
    {
        UiLcdHy28_WriteIndexSpi(LCD_Reg);
        UiLcdHy28_WriteDataSpi(LCD_RegValue);
    }
    else
    {
        LCD_REG = LCD_Reg;
        __DMB();
        LCD_RAM = LCD_RegValue;
        __DMB();
    }
}

unsigned short UiLcdHy28_ReadReg( unsigned short LCD_Reg)
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

        // Write 16-bit Index (then Read Reg)
        LCD_REG = LCD_Reg;
        // Read 16-bit Reg
        __DMB();
        retval = LCD_RAM;
    }
    return retval;
}

static void UiLcdHy28_SetCursorA( unsigned short Xpos, unsigned short Ypos )
{
    UiLcdHy28_WriteReg(0x20, Ypos );
    UiLcdHy28_WriteReg(0x21, Xpos );
}

static void UiLcdHy28_WriteRAM_Prepare()
{
    if(UiLcdHy28_SpiDisplayUsed())
    {
        UiLcdHy28_WriteIndexSpi(0x0022);
        UiLcdHy28_WriteDataSpiStart();
    }
    else
    {
        LCD_REG = 0x22;
        __DMB();
    }
}

static void UiLcdHy28_BulkWrite(uint16_t* pixel, uint32_t len)
{

    if(UiLcdHy28_SpiDisplayUsed() == false)
    {
        uint32_t i = len;
        for (; i; i--)
        {
            UiLcdHy28_WriteDataOnly(*(pixel++));
        }
    }
    else
    {
#ifdef USE_SPI_DMA
        uint32_t i;
        for (i = 0; i < len; i++)
        {
            pixel[i] = __REV16(pixel[i]); // reverse byte order;
        }
        UiLcdHy28_SpiDmaStart((uint8_t*)pixel,len*2);
#else
        uint32_t i = len;
        for (; i; i--)
        {
            UiLcdHy28_WriteDataOnly(*(pixel++));
        }
#endif
    }


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

    UiLcdHy28_SetCursorA(x, y); // WE DON'T NEED THIS SINCE WE ALWAYS WRITE IN THE WINDOW ADDRESS due to Reg03:ORG = 1: set starting position of character

    // UiLcdHy28_WriteReg(0x03,  0x1038);    // WE DON'T NEED THIS SINCE THIS IS DONE ALREADY IN INIT: set GRAM write direction and BGR=1 and turn on cursor auto-increment
    UiLcdHy28_WriteReg(0x50, y);    // Vertical GRAM Start Address
    UiLcdHy28_WriteReg(0x51, y + height -1);    // Vertical GRAM End Address    -1
    UiLcdHy28_WriteReg(0x52, x);    // Horizontal GRAM Start Address
    UiLcdHy28_WriteReg(0x53, x + width -1);    // Horizontal GRAM End Address  -1

    UiLcdHy28_WriteRAM_Prepare();                   // prepare for bulk-write to character window


}

static void UiLcdHy28_CloseBulkWrite(void)
{
}




#define PIXELBUFFERSIZE 512
#define PIXELBUFFERCOUNT 2
static uint16_t   pixelbuffer[PIXELBUFFERCOUNT][PIXELBUFFERSIZE];
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
void UiLcdHy28_DrawColorPoint( unsigned short Xpos, unsigned short Ypos, unsigned short point)
{

    if( Xpos < MAX_X && Ypos < MAX_Y )
    {
        UiLcdHy28_OpenBulkWrite(Xpos,Xpos,Ypos,Ypos);
        UiLcdHy28_WriteDataOnly(point);
        UiLcdHy28_CloseBulkWrite();
    }
}
#endif

void UiLcdHy28_DrawFullRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width ,ushort color)
{
    UiLcdHy28_OpenBulkWrite(Xpos, Width, Ypos, Height);
    UiLcdHy28_BulkWriteColor(color,(uint32_t)Height * (uint32_t)Width);
    UiLcdHy28_CloseBulkWrite();
}


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


void UiLcdHy28_DrawStraightLine(ushort x, ushort y, ushort Length, uchar Direction,ushort color)
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
        for (; i; i--)
        {
            UiLcdHy28_WriteDataOnly(Color);
        }
    }
}




void UiLcdHy28_DrawChar(ushort x, ushort y, char symb,ushort Color, ushort bkColor,const sFONT *cf)
{
    ulong       i,j;
    ushort      a,b;
    const uchar      fw = cf->Width;
    const short   *ch = (const short *)(&cf->table[(symb - 32) * cf->Height]);

    UiLcdHy28_OpenBulkWrite(x,cf->Width,y,cf->Height);
    UiLcdHy28_BulkPixel_BufferInit();
    for(i = 0; i < cf->Height; i++)
    {
        // Draw line
        for(j = 0; j < cf->Width; j++)
        {
            a = ((ch[i] & ((0x80 << ((fw / 12 ) * 8)) >> j)));
            b = ((ch[i] &  (0x01 << j)));
            //
            UiLcdHy28_BulkPixel_Put(((!a && (fw <= 12)) || (!b && (fw > 12)))?bkColor:Color);
        }
    }
    UiLcdHy28_BulkPixel_BufferFlush();
    // flush all not yet  transferred pixel to display.
    UiLcdHy28_CloseBulkWrite();
}

const sFONT   *UiLcdHy28_Font(uint8_t font)
{
    // if we have an illegal font number, we return the first font
    return fontList[font < fontCount ? font : 0];
}

void UiLcdHy28_PrintText(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor,uchar font)
{
    uint8_t    TempChar;
    const sFONT   *cf = UiLcdHy28_Font(font);

    if (str != NULL)
    {
        while ( *str != 0 )
        {
            TempChar = *str++;

            UiLcdHy28_DrawChar(Xpos, Ypos, TempChar,Color,bkColor,cf);

            if(Xpos < (MAX_X - cf->Width))
            {
                Xpos += cf->Width;

                // Mod the 8x8 font - the shift is too big
                // because of the letters
                if(font == 4)
                {
                 //   if(*str > 0x39)
                        Xpos -= 1;
                   // else
                   //     Xpos -= 2;
                }
            }
            else if (Ypos < (MAX_Y - cf->Height))
            {
                Xpos  = 0;
                Ypos += cf->Height;
            }
            else
            {
                Xpos = 0;
                Ypos = 0;
            }
        }
    }
}


uint16_t UiLcdHy28_TextHeight(uint8_t font)
{

    const sFONT   *cf = UiLcdHy28_Font(font);
    return cf->Height;
}


uint16_t UiLcdHy28_TextWidth(const char *str, uchar font)
{

    uint16_t Xpos = 0;

    const sFONT   *cf = UiLcdHy28_Font(font);
    if (str != NULL)
    {
        while ( *str != 0 )
        {
            Xpos+=cf->Width;
            if(font == 4)
            {
                if(*str > 0x39)
                    Xpos -= 1;
                else
                    Xpos -= 2;
            }
            str++;
        }
    }
    return Xpos;
}

void UiLcdHy28_PrintTextRight(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor,uint8_t font)
{

    uint16_t Xwidth = UiLcdHy28_TextWidth(str, font);
    if (Xpos < Xwidth )
    {
        Xpos = 0; // TODO: Overflow is not handled too well, just start at beginning of line and draw over the end.
    }
    else
    {
        Xpos -= Xwidth;
    }
    UiLcdHy28_PrintText(Xpos, Ypos, str, Color, bkColor, font);
}

void UiLcdHy28_PrintTextCentered(const uint16_t bbX,const uint16_t bbY,const uint16_t bbW,const char* txt,uint32_t clr_fg,uint32_t clr_bg,uint8_t font)
{
    const uint16_t bbH = UiLcdHy28_TextHeight(font);
    const uint16_t txtW = UiLcdHy28_TextWidth(txt,font);
    const uint16_t bbOffset = txtW>bbW?0:((bbW - txtW)+1)/2;

    // we draw the part of  the box not used by text.
    UiLcdHy28_DrawFullRect(bbX,bbY,bbH,bbOffset,clr_bg);
    UiLcdHy28_PrintText((bbX + bbOffset),bbY,txt,clr_fg,clr_bg,font);

    // if the text is smaller than the box, we need to draw the end part of the
    // box
    if (txtW<bbW)
    {
        UiLcdHy28_DrawFullRect(bbX+txtW+bbOffset,bbY,bbH,bbW-(bbOffset+txtW),clr_bg);
    }
}

void UiLcdHy28_SendRegisters(const RegisterValue_t* regvals, uint16_t count)
{
    for (uint16_t idx = 0; idx < count; idx++)
    {
        if (regvals[idx].reg == 0)
        {
            HAL_Delay(regvals[idx].val);
        }
        else
        {
            UiLcdHy28_WriteReg(regvals[idx].reg, regvals[idx].val);
        }
    }
}

uint16_t UiLcdHy28_InitA(uint32_t display_type)
{

    switch(display_type)
    {

    case DISPLAY_HY28A_SPI:
        mchf_display.use_spi = true;
        // this is were HY28A have the CS line
        mchf_display.lcd_cs = LCD_D11;
        mchf_display.lcd_cs_pio = LCD_D11_PIO;

        UiLcdHy28_SpiInit(false);
        // HY28A works only with less then 32 Mhz, so we do low speed
        break;
    case DISPLAY_HY28B_SPI:
        mchf_display.use_spi = true;
        // this is were HY28A have the CS line
        mchf_display.lcd_cs = LCD_CS;
        mchf_display.lcd_cs_pio = LCD_CS_PIO;

        UiLcdHy28_SpiInit(HY28BHISPEED);
        // HY28B works sometimes faster
        break;
    case DISPLAY_HY28B_PARALLEL:
        // Select parallel
        mchf_display.use_spi = false;

        // set CS for HY28B pinout
        mchf_display.lcd_cs = LCD_CS;
        mchf_display.lcd_cs_pio = LCD_CS_PIO;

        // Parallel init
        UiLcdHy28_ParallelInit();
        UiLcdHy28_FSMCConfig();
        break;
    }

    UiLcdHy28_Reset();



    uint16_t retval = UiLcdHy28_ReadReg(0x00);

    if(retval == 0x9320)
    {
        // HY28A - SPI interface only (ILI9320 controller)
        UiLcdHy28_SendRegisters(ili9320, sizeof(ili9320)/sizeof(RegisterValue_t));
    }
    else if(retval == 0x5408)
    {
        // HY28A - Parallel interface only (SPFD5408B controller)
        UiLcdHy28_SendRegisters(spdfd5408b, sizeof(spdfd5408b)/sizeof(RegisterValue_t));
    }
    else if((retval == 0x9325) || (retval == 0x9328))
    {
        // HY28B - Parallel & Serial interface - latest model (ILI9325 & ILI9328 controller)
        UiLcdHy28_SendRegisters(ili932x, sizeof(ili932x)/sizeof(RegisterValue_t));
	}
    else
    {
        retval = 0x0000;
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
    // Backlight
    UiLcdHy28_BacklightInit();



    #ifdef USE_DISPLAY_SPI

    mchf_display.DeviceCode = UiLcdHy28_InitA(DISPLAY_HY28A_SPI);

    if(mchf_display.DeviceCode != 0x0000)
    {
        retval = DISPLAY_HY28A_SPI;
    }

    if (retval == DISPLAY_NONE)
    {
        mchf_display.DeviceCode = UiLcdHy28_InitA(DISPLAY_HY28B_SPI);
        if(mchf_display.DeviceCode != 0x0000)
        {
            retval = DISPLAY_HY28B_SPI;
        }
    }

    #endif

    if (retval == DISPLAY_NONE)
    {
        mchf_display.DeviceCode = UiLcdHy28_InitA(DISPLAY_HY28B_PARALLEL);
        if(mchf_display.DeviceCode != 0x0000)
        {
            retval = DISPLAY_HY28B_PARALLEL;
        }
    }

    mchf_display.display_type = retval;
    return retval;
}


/*
 * @brief Called to run the touch detection state machine, results are stored in ts structure
 */
static inline void UiLcdHy28_SetSpiPrescaler(const uint16_t baudrate_prescaler)
{
    /*---------------------------- SPIx CR1 Configuration ------------------------*/
    /* Get the SPIx CR1 value */
    uint16_t tmpreg = SPI2->CR1;
    tmpreg &= ~(uint16_t)((uint32_t) (SPI_BAUDRATEPRESCALER_256));
    tmpreg |= (uint16_t)((uint32_t) (baudrate_prescaler));
    /* Write to SPIx CR1 */
    SPI2->CR1 = tmpreg;
}

mchf_touchscreen_t mchf_touchscreen;

void UiLcdHy28_TouchscreenDetectPress()
{
    if (mchf_touchscreen.present)
    {
        if(!HAL_GPIO_ReadPin(TP_IRQ_PIO,TP_IRQ) && mchf_touchscreen.state != TP_DATASETS_PROCESSED)    // fetch touchscreen data if not already processed
            UiLcdHy28_TouchscreenReadCoordinates();

        if(HAL_GPIO_ReadPin(TP_IRQ_PIO,TP_IRQ) && mchf_touchscreen.state == TP_DATASETS_PROCESSED)     // clear statemachine when data is processed
        {
            mchf_touchscreen.state = 0;
            mchf_touchscreen.x = mchf_touchscreen.y = 0xff;
        }
    }
}
/*
 * @brief tells you that touchscreen coordinates are ready for processing and marks them as processed
 * @returns true if coordinates for processing are available and have been marked as processed, false otherwise
 */
bool UiLcdHy28_TouchscreenHasProcessableCoordinates() {
    bool retval = false;
    UiLcdHy28_TouchscreenReadCoordinates();
    if(mchf_touchscreen.state > TP_DATASETS_WAIT && mchf_touchscreen.state != TP_DATASETS_PROCESSED)
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
        UiLcdHy28_SetSpiPrescaler(SPI_BAUDRATEPRESCALER_4);
    }

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

    HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)xpt2046_command, xpt_response,XPT2046_COMMAND_LEN,10);

    UiLcdHy28_TouchscreenFinishSpiTransfer();

    *x_p = (xpt_response[5] << 8 | xpt_response[6]) >> 3;
    *y_p = (xpt_response[3] << 8 | xpt_response[4]) >> 3;

}



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

            uint8_t x,y;

            uint8_t xraw = mchf_touchscreen.xraw >> 5;
            uint8_t yraw = mchf_touchscreen.yraw >> 5;

            if(mchf_touchscreen.reversed == false)
            {
                uint8_t i;
                for(i=0; touchscreentable[i] < xraw && i < 60; i++);
                x = 60-i;
            }
            else
            {					// correction of unlinearity because of mirrored x
                uint8_t k = 0;

                uint8_t i;
                for(i=60; touchscreentable[i] > xraw && i > 0; i--);

                x = i--;


                if(x == 57 || (x < 7 && x > 1))	k=2;
                if(x == 56 || (x == 8 || x == 7))	k=3;
                if((x < 56 && x > 50) || (x < 16 && x > 8))	k=5;;
                if(x == 50 || (x == 17 || x == 16) || (x == 47 || x == 46))	k=6;
                if(x == 45)	k=7;
                if((x == 49 || x == 48) || x == 44 || (x < 34 && x > 30) || (x < 21 && x > 17))	k=8;
                if((x < 44 && x > 33) || (x < 27 && x > 20))	k=9;
                if(x < 31 && x > 26)	k=10;
                x = x - k;
            }

            uint8_t i;
            for(i=0; touchscreentable[i] < yraw && i < 60; i++);
            y = i--;

            if(x == mchf_touchscreen.x && y == mchf_touchscreen.y)		// got identical data
            {
                mchf_touchscreen.state++;						// touch data valid
            }
            else
            {
                // set new data
                mchf_touchscreen.x = x;
                mchf_touchscreen.y = y;
                mchf_touchscreen.state = TP_DATASETS_WAIT;		// restart machine
            }
        }
        else
        {
            mchf_touchscreen.state = TP_DATASETS_WAIT;			// do next first data read
        }
    }
}

bool UiLcdHy28_TouchscreenPresenceDetection(void)
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

void UiLcdHy28_TouchscreenInit(bool is_reversed)
{
    mchf_touchscreen.xraw = 0;
    mchf_touchscreen.yraw = 0;
    mchf_touchscreen.x = 0xFF;                        // invalid position
    mchf_touchscreen.y = 0xFF;                        // invalid position
    mchf_touchscreen.reversed = is_reversed;
    mchf_touchscreen.present = UiLcdHy28_TouchscreenPresenceDetection();
}

