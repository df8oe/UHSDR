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
#include <ui_lcd_hy28.h>
#include "uhsdr_board.h"
#include "uhsdr_board_config.h"

#include "ui_lcd_draw.h"

#ifdef USE_GFX_RA8875


#define RA8875_MRWC                   0x02//memory read write control
#define RA8875_HOFS0                  0x24//Horizontal Scroll Offset Register 0
#define RA8875_HOFS1                  0x25//Horizontal Scroll Offset Register 1
#define RA8875_VOFS0                  0x26//Vertical Scroll Offset Register 0
#define RA8875_VOFS1                  0x27//Vertical Scroll Offset Register 1

#define RA8875_HSAW0                  0x30//Horizontal Start Point 0 of Active Window
#define RA8875_HSAW1                  0x31//Horizontal Start Point 1 of Active Window
#define RA8875_VSAW0                  0x32//Vertical Start Point 0 of Active Window
#define RA8875_VSAW1                  0x33//Vertical Start Point 1 of Active Window

#define RA8875_HSSW0                  0x38//Horizontal Start Point 0 of Scroll Window
#define RA8875_HSSW1                  0x39//Horizontal Start Point 1 of Scroll Window
#define RA8875_VSSW0                  0x3A//Vertical     Start Point 0 of Scroll Window
#define RA8875_VSSW1                  0x3B//Vertical     Start Point 1 of Scroll Window
#define RA8875_HESW0                  0x3C//Horizontal End   Point 0 of Scroll Window
#define RA8875_HESW1                  0x3D//Horizontal End   Point 1 of Scroll Window
#define RA8875_VESW0                  0x3E//Vertical     End   Point 0 of Scroll Window
#define RA8875_VESW1                  0x3F//Vertical     End   Point 1 of Scroll Window


#define RA8875_CURH0                  0x46//horizontal cursor x pos 0
#define RA8875_CURH1                  0x46//horizontal cursor x pos 1
#define RA8875_CURV0                  0x47//vertical cursor y pos 0
#define RA8875_CURV1                  0x48//vertical cursor y pos 1

#define RA8875_BECR0                  0x50// bte control register
#define RA8875_BECR1                  0x51// bte control register
#define RA8875_LTPR0                  0x52//Layer Transparency Register 0

#define RA8875_HSBE0                  0x54// bte horizontal source point 0
#define RA8875_VSBE0                  0x56// bte vertical source point 0
#define RA8875_HDBE0                  0x58// bte horizontal destination point 0
#define RA8875_VDBE0                  0x5A// bte vertical destination point 0
#define RA8875_BERW0                  0x5C// bte width 0
#define RA8875_BERH0                  0x5E// bte height 0


    /* Drawing Control Registers */
#define RA8875_DCR     (0x90)      /* Draw Line/Circle/Square Control Register */
#define RA8875_DLHSR0  (0x91)      /* Draw Line/Square Horizontal Start Address Register0 */
#define RA8875_DLHSR1  (0x92)      /* Draw Line/Square Horizontal Start Address Register1 */
#define RA8875_DLVSR0  (0x93)      /* Draw Line/Square Vertical Start Address Register0 */
#define RA8875_DLVSR1  (0x94)      /* Draw Line/Square Vertical Start Address Register1 */
#define RA8875_DLHER0  (0x95)      /* Draw Line/Square Horizontal End Address Register0 */
#define RA8875_DLHER1  (0x96)      /* Draw Line/Square Horizontal End Address Register1 */
#define RA8875_DLVER0  (0x97)      /* Draw Line/Square Vertical End Address Register0 */
#define RA8875_DLVER1  (0x98)      /* Draw Line/Square Vertical End Address Register1 */

// SPI PREFIX BYTE
#define RA8875_CMDWRITE  0x80
#define RA8875_DATAWRITE 0x00
#define RA8875_CMDREAD   0xC0
#define RA8875_DATAREAD  0x40

static const RegisterValue_t ra8875[] =
{
        { 0x01, 0x01}, // Software reset the LCD
	    { REGVAL_DELAY, 100},              // delay 100 ms
        { 0x01, 0x00},
	    { REGVAL_DELAY, 100},              // delay 100 ms
        { 0x88, 0x0c},
	    { REGVAL_DELAY, 100},              // delay 100 ms
        { 0x89, 0x01},
	    { REGVAL_DELAY, 100},              // delay 100 ms
        { 0x10, 0x0C},   // 65K 16 bit 8080 mpu interface
        { 0x04, 0x81},   // 00b: PCLK period = System Clock period.
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
        { 0x63, 0x00}, /* ra8875_red */
        { 0x64, 0x00}, /* ra8875_green */
        { 0x65, 0x00}, /* ra8875_blue */

        { 0x8E, 0x80},
};
 
const RegisterValueSetInfo_t ra8875_regs =
{
    ra8875, sizeof(ra8875)/sizeof(RegisterValue_t)
};

void UiLcdRa8875_WriteDataSpiStart_Prepare()
{
    UiLcd_SpiSendByte(RA8875_DATAWRITE);
}
void UiLcdRa8875_WriteIndexSpi_Prepare()
{
    UiLcd_SpiSendByte(RA8875_CMDWRITE);
}

static void UiLcdRa8875_WaitReady(uint8_t pattern)
{
    if(UiLcd_LcdSpiUsed())
    {
        while( ( UiLcd_LcdSpiWrite8Read8(RA8875_CMDREAD) & pattern) == pattern);
    }
    else
    {
        while ((LCD_REG & pattern) == pattern);
    }
}


uint8_t ra8875_reg_cache[256];


void UiLcdRa8875_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
    UiLcdRa8875_WaitReady(0x80);
    if(UiLcd_LcdSpiUsed())
    {
        ra8875_reg_cache[LCD_Reg & 0xff] = LCD_RegValue;

        // write command
        UiLcd_LcdSpiWrite16((RA8875_CMDWRITE << 8)| LCD_Reg );
        // write data
        UiLcd_LcdSpiWrite16((RA8875_DATAWRITE << 8)| LCD_RegValue );

    }
    else
    {
        LCD_REG = LCD_Reg;
        LCD_RAM = LCD_RegValue;
    }
}

static void UiLcdRa8875_WriteReg_8bit(uint16_t LCD_Reg, uint8_t LCD_RegValue)
{
    UiLcdRa8875_WriteReg(LCD_Reg, LCD_RegValue);
}

#if 0 // currently unsused
static void UiLcdRa8875_WriteReg_16bit(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{

    UiLcdRa8875_WriteReg(LCD_Reg,LCD_RegValue & 0xff);
    UiLcdRa8875_WriteReg(LCD_Reg+1,(LCD_RegValue >> 8) & 0xff);
}
#endif

static void UiLcdRa8875_WriteReg_16bit_Seq(uint16_t LCD_Reg, uint16_t size, uint16_t* LCD_RegValues)
{
    UiLcdRa8875_WaitReady(0x80);
    if(UiLcd_LcdSpiUsed())
    {
        for (uint16_t idx = 0; idx < size; idx++)
        {
            // write command
            UiLcd_LcdSpiWrite16((RA8875_CMDWRITE << 8)| (LCD_Reg+2*idx) );
            // write data
            UiLcd_LcdSpiWrite16((RA8875_DATAWRITE << 8)| (LCD_RegValues[idx] & 0xff) );
            // write command
            UiLcd_LcdSpiWrite16((RA8875_CMDWRITE << 8)| (LCD_Reg+2*idx+1) );
            // write data
            UiLcd_LcdSpiWrite16((RA8875_DATAWRITE << 8)| (LCD_RegValues[idx] >> 8) );

        }
    }
    else
    {
        for (uint16_t idx = 0; idx < size; idx++)
        {

            LCD_REG = LCD_Reg + 2*idx;
            LCD_RAM = LCD_RegValues[idx] & 0xff;

            LCD_REG = LCD_Reg + 2*idx + 1;
            LCD_RAM = LCD_RegValues[idx] >> 8;
       }
    }
}

uint16_t UiLcdRa8875_ReadReg(uint16_t LCD_Reg)
{
	uint16_t retval;
    // Write 16-bit Index (then Read Reg)
    if(UiLcd_LcdSpiUsed())
    {
        UiLcd_LcdSpiWrite16((RA8875_CMDWRITE << 8)| LCD_Reg );
        retval = UiLcd_LcdSpiWrite8Read8( RA8875_DATAREAD );    }
    else
    {
        LCD_REG = LCD_Reg;
        // Read 16-bit Reg
        retval = LCD_RAM;
    }

    return retval;
}



void UiLcdRa8875_WriteRAM_Prepare()
{
    UiLcd_WriteRAM_Prepare_Index(RA8875_MRWC);
}


uint16_t UiLcdRa8875_ReadDisplayId(void)
{
	uint16_t retval =0;
	uint16_t reg_63_val=UiLcdRa8875_ReadReg(0x63);
	uint16_t reg_64_val=UiLcdRa8875_ReadReg(0x64);
	uint16_t reg_65_val=UiLcdRa8875_ReadReg(0x65);
	uint16_t reg_88_val=UiLcdRa8875_ReadReg(0x88);
	uint16_t reg_89_val=UiLcdRa8875_ReadReg(0x89);

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

// Driver Graphic Operations
static void UiLcdRa8875_SetForegroundColor(uint16_t Color)
{
    static uint16_t last_color;

    if (Color != last_color)
    {
        last_color = Color;

        UiLcdRa8875_WriteReg_8bit(0x63, (uint16_t) (Color >> 11)); /* ra8875_red */
        UiLcdRa8875_WriteReg_8bit(0x64, (uint16_t) (Color >> 5)); /* ra8875_green */
        UiLcdRa8875_WriteReg_8bit(0x65, (uint16_t) (Color)); /* ra8875_blue */
    }
}
static void UiLcdRa8875_DrawOp(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1 ,uint16_t color, uint8_t op)
{
    UiLcdRa8875_SetForegroundColor(color);

    uint16_t vals[] = { X0, Y0, X1, Y1 };
    UiLcdRa8875_WriteReg_16bit_Seq( RA8875_DLHSR0, 4, vals);

    UiLcdRa8875_WriteReg(RA8875_DCR, op);               // Fill rectangle
}



void UiLcdRa8875_SetActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom)
{
    uint16_t vals[] = { XLeft, YTop, XRight, YBottom};
    UiLcdRa8875_WriteReg_16bit_Seq( RA8875_HSAW0, 4, vals);
}

void UiLcdRa8875_SetCursorA( unsigned short Xpos, unsigned short Ypos )
{
    uint16_t vals[] = { Xpos, Ypos};
    UiLcdRa8875_WriteReg_16bit_Seq( RA8875_CURH0, 2, vals);
}

void UiLcdRa8875_DrawColorPoint(uint16_t Xpos,uint16_t Ypos,uint16_t color)
{
    uint16_t MAX_X=mchf_display.MAX_X; uint16_t MAX_Y=mchf_display.MAX_Y;
    if( Xpos < MAX_X && Ypos < MAX_Y )
    {
        UiLcdRa8875_SetCursorA(Xpos, Ypos);
        UiLcdRa8875_WriteReg(RA8875_MRWC, color);
    }
}

void UiLcdRa8875_DrawStraightLine(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color)
{
    if(Length == 1)
    {
        UiLcdRa8875_DrawColorPoint(x,y,color);
    }
    else if(Length>1)
    {
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

        // use draw line op of controller
        UiLcdRa8875_DrawOp(x, y, x_end, y_end, color, 0x80);
    }
}

void UiLcdRa8875_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color)
{
    // draw filled rectangle
    UiLcdRa8875_DrawOp( Xpos, Ypos, Xpos + Width-1, Ypos + Height-1, color, 0xB0);
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
void UiLcdRa8875_setScrollMode(uint8_t mode)
{
    uint8_t temp = UiLcdRa8875_ReadReg(RA8875_LTPR0);
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

    UiLcdRa8875_WriteReg(RA8875_LTPR0, temp);

}

/**
 * Define a window for perform scroll
 *
 * @param XL x window start left
 * @param XR x window end right
 * @param YT y window start top
 * @param YB y window end bottom
 */
void UiLcdRa8875_setScrollWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB)
{
    uint16_t vals[] = { XL, YT, XR, YB };
    UiLcdRa8875_WriteReg_16bit_Seq( RA8875_HSSW0, 4, vals);
}

/**
 * set scroll viewpoint (does not move the data!)
 * @param x shift original by x pixels left
 * @param y shift original by y pixel up
 */
void UiLcdRa8875_scroll(int16_t x,int16_t y)
{
    uint16_t vals[] = { x, y};
    UiLcdRa8875_WriteReg_16bit_Seq( RA8875_HOFS0, 2, vals);
}

void UiLcdRa8875_MoveAreaDown(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h, uint16_t down)
{
    uint16_t vals[] = { xs+w-1, ys+h-1, xs+w-1, ys + h - 1 + down, w, h };
    UiLcdRa8875_WriteReg_16bit_Seq( RA8875_HSBE0, 6, vals);

    // dest = source, reverse (backwards fill),
    UiLcdRa8875_WriteReg(RA8875_BECR1, 0xc0 | 0x03);

    UiLcdRa8875_WriteReg(RA8875_BECR0, 0x80);
    UiLcdRa8875_WaitReady(0x40);

    // while (UiLcdRa8875_ReadReg(RA8875_BECR0) & 0x01);

}
#endif
