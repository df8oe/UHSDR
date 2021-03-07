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
#include "ui_lcd_draw.h"

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


void UiLcdDraw_ColorPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	mchf_display.DrawColorPoint(Xpos,Ypos,point);
}


void UiLcdDraw_FullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color)
{
	mchf_display.DrawFullRect(Xpos,Ypos,Height,Width,color);
}

static void UiLcdHy28_DrawStraightLineWidth(ushort x, ushort y, ushort Length, uint16_t Width, uchar Direction,ushort color)
{
    if(Direction == LCD_DIR_VERTICAL)
    {
        UiLcdDraw_FullRect(x,y,Length,Width,color);
    }
    else
    {
        UiLcdDraw_FullRect(x,y,Width,Length,color);
    }
}

void UiLcdDraw_StraightLine(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color)
{
	mchf_display.DrawStraightLine(x,y,Length,Direction,color);
}

void UiLcdDraw_StraightLineDouble(ushort x, ushort y, ushort Length, uchar Direction,ushort color)
{
    UiLcdHy28_DrawStraightLineWidth(x, y, Length, 2, Direction, color);
}

void UiLcdDraw_StraightLineTriple(ushort x, ushort y, ushort Length, uchar Direction,ushort color)
{
    UiLcdHy28_DrawStraightLineWidth(x, y, Length, 3, Direction, color);
}


void UiLcdDraw_HorizLineWithGrad(ushort x, ushort y, ushort Length,ushort gradient_start)
{
    uint32_t i = 0,j = 0;
    ushort     k = gradient_start;


    UiLcd_BulkPixel_OpenWrite(x,Length,y,1);
    for(i = 0; i < Length; i++)
    {
        UiLcd_BulkPixel_Put(RGB(k,k,k));
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
    UiLcd_BulkPixel_CloseWrite();
}

void UiLcdDraw_EmptyRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color)
{
    UiLcdDraw_StraightLine(Xpos, (Ypos),          Width,        LCD_DIR_HORIZONTAL,color);
    UiLcdDraw_StraightLine(Xpos, Ypos,            Height,       LCD_DIR_VERTICAL,color);
    UiLcdDraw_StraightLine((Xpos + Width), Ypos,  (Height + 1), LCD_DIR_VERTICAL,color);
    UiLcdDraw_StraightLine(Xpos, (Ypos + Height), Width,        LCD_DIR_HORIZONTAL,color);
}

void UiLcdDraw_BottomButton(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color)
{
    UiLcdDraw_StraightLine(Xpos, (Ypos),        Width, LCD_DIR_HORIZONTAL,color);
    UiLcdDraw_StraightLine(Xpos, Ypos,          Height,LCD_DIR_VERTICAL,  color);
    UiLcdDraw_StraightLine((Xpos + Width), Ypos,Height,LCD_DIR_VERTICAL,  color);
}




#ifdef USE_8bit_FONT
static void UiLcdHy28_DrawChar_8bit(ushort x, ushort y, char symb,ushort Color, ushort bkColor,const sFONT *cf)
{

    const uint16_t charIdx = (symb >= 0x20 && symb < cf->maxCode)? cf->offsetTable[symb - cf->firstCode] : 0xFFFF;

    const uint8_t Font_H = cf->Height;
    symbolData_t* sym_ptr = charIdx == 0xFFFF? NULL:((symbolData_t*)cf->table)+charIdx;

    const uint8_t Font_W = sym_ptr == NULL? cf->Width:sym_ptr->width;

    const uint16_t charSpacing = cf->Spacing;

    UiLcd_BulkPixel_OpenWrite(x, Font_W+charSpacing, y, Font_H);

    if(sym_ptr == NULL) // NON EXISTING SYMBOL
    {
        for(int cntrY=0;cntrY < Font_H; cntrY++)
        {
            for(int cntrX=0; cntrX < Font_W; cntrX++)
            {
                UiLcd_BulkPixel_Put(Color);
            }
            for(int cntrX=0; cntrX < charSpacing; cntrX++)
            {
                UiLcd_BulkPixel_Put(bkColor);
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
                UiLcd_BulkPixel_Put(pixel);
            }

            // add spacing behind the character data
            for(int n=Font_W; n < Font_W + charSpacing ; n++)
            {
                UiLcd_BulkPixel_Put(bkColor);
            }
        }
    }

    UiLcd_BulkPixel_CloseWrite();
}
#endif

static void UiLcdHy28_DrawChar_1bit(ushort x, ushort y, char symb,ushort Color, ushort bkColor,const sFONT *cf)
{
    uint8_t   *ch = (uint8_t *)cf->table;

    // we get the address of the begin of the character table
    // we support one or two byte long character definitions
    // anything wider than 8 pixels uses two bytes
    ch+=(symb - 32) * cf->Height* ((cf->Width>8) ? 2 : 1 );

    UiLcd_BulkPixel_OpenWrite(x,cf->Width,y,cf->Height);

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
            UiLcd_BulkPixel_Put((line_data & mask) != 0 ? Color : bkColor);
            // we shift the mask in the for loop to the right one by one
        }
    }

    UiLcd_BulkPixel_CloseWrite();
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

static const sFONT   *UiLcdHy28_Font(uint8_t font)
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
uint16_t UiLcdDraw_PrintText(uint16_t XposStart, uint16_t YposStart, const char *str,const uint32_t clr_fg, const uint32_t clr_bg,uchar font)
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


uint16_t UiLcdDraw_TextHeight(uint8_t font)
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
uint16_t UiLcdDraw_TextWidth(const char *str_start, uchar font)
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
uint16_t UiLcdDraw_PrintTextRight(uint16_t XposStart, uint16_t YposStart, const char *str,const uint32_t clr_fg, const uint32_t clr_bg,uint8_t font)
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
    const uint16_t bbH = UiLcdDraw_TextHeight(font);
    const uint16_t txtW = UiLcdHy28_TextWidthLen(str, len, font);
    const uint16_t bbOffset = txtW>bbW?0:((bbW - txtW)+1)/2;

    // we draw the part of the box not used by text.
    if (bbOffset)
    {
        UiLcdDraw_FullRect(XposStart,YposStart,bbH,bbOffset,clr_bg);
    }

    UiLcdHy28_PrintTextLen((XposStart + bbOffset),YposStart,str, len, clr_fg,clr_bg,font);

    // if the text is smaller than the box, we need to draw the end part of the
    // box
    if (txtW<bbW)
    {
        UiLcdDraw_FullRect(XposStart+txtW+bbOffset,YposStart,bbH,bbW-(bbOffset+txtW),clr_bg);
    }
}

/*
 * Print text centered inside the bounding box. Using '\n' to print multline text
 */
uint16_t UiLcdDraw_PrintTextCentered(const uint16_t XposStart,const uint16_t YposStart,const uint16_t bbW,const char* str,uint32_t clr_fg,uint32_t clr_bg,uint8_t font)
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

void UiLcdDraw_LcdClear(uint16_t Color)
{
    uint32_t MAX_X=mchf_display.MAX_X; uint32_t MAX_Y=mchf_display.MAX_Y;
    mchf_display.DrawFullRect(0,0,MAX_Y, MAX_X, Color);
}

