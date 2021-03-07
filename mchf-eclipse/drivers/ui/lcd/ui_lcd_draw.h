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

#ifndef __UI_LCD_DRAW_H
#define __UI_LCD_DRAW_H

#include "uhsdr_types.h"



#define RGB(red,green,blue)(uint16_t)(((red>>3)<<11)|((green>>2)<<5)|(blue>> 3))

// Colors definitions, go to http://www.color-hex.com/
// choose a new one and declare here
//
#define White          		0xFFFF
#define Black          		0x0000
#define Grey           		RGB(0xb8,0xbc,0xa8)	//=0xBDF5
#define Blue           		RGB(0x50,0x50,0xFF)	// Brighter Blue	//Original - 0x001F =#0000FF
#define Blue2          		0x051F				// =#00A0FF
#define Red            		RGB(0xFF,0x38,0x38)	// Brighter (easier to see) Red	//Original 0xF800 =#FF0000
#define	Red2				RGB(0xFF,0x80,0x80)	// Even "brighter" red (almost pink)
#define Red3				RGB(0xFF,0xC0,0xC0) // A sort of pink-ish, pale red
#define Magenta        		RGB(0xFF,0x30,0xFF)	// Brighter Magenta	//Original 0xF81F =#FF00FF
#define Green          		0x07E0				// =#00FF00
#define Cyan           		0x7FFF
#define Yellow         		RGB(0xFF,0xFF,0x20)	// "Yellower" and brighter	//Original - 0xFFE0 =#FFFF00

#define Orange				RGB(0xFF,0xA8,0x20)	//"Orange-er" and brighter	//Original - RGB(0xF6,0xA0,0x1A)
#define Cream				RGB(0xED,0xE7,0xD7)

#define Grey1				RGB(0x80,0x80,0x80)
#define Grey2				RGB(0xC0,0xC0,0xC0)
#define Grey3				RGB(0xA6,0xA8,0xAD)
#define Grey4				RGB(0x40,0x40,0x40)
#define Grey5               RGB(0x60,0x60,0x60)
#define	Grey6				RGB(0x78,0x78,0x78)

#define	RX_Grey				RGB(0xb8,0xdb,0xa8)	// slightly green grey
#define TX_Grey				RGB(0xe8,0xad,0xa0)	// slightly red(ish) grey (more magenta, actually...)

#define SMALL_FONT_WIDTH            8
#define LARGE_FONT_WIDTH            16


#define LCD_DIR_HORIZONTAL	0x0000
#define LCD_DIR_VERTICAL	0x0001

#define GRADIENT_STEP			8

void 	UiLcdDraw_LcdClear(uint16_t Color);

uint16_t UiLcdDraw_PrintText(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor, uint8_t font);
uint16_t UiLcdDraw_PrintTextRight(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor, uint8_t font);
uint16_t UiLcdDraw_PrintTextCentered(const uint16_t bbX,const uint16_t bbY,const uint16_t bbW,const char* txt,uint32_t clr_fg,uint32_t clr_bg,uint8_t font);

uint16_t UiLcdDraw_TextWidth(const char *str, uint8_t font);
uint16_t UiLcdDraw_TextHeight(uint8_t font);

void 	UiLcdDraw_StraightLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction,uint16_t color);
void    UiLcdDraw_StraightLineDouble(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction,uint16_t color);
void    UiLcdDraw_StraightLineTriple(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction,uint16_t color);
void 	UiLcdDraw_HorizLineWithGrad(uint16_t Xpos, uint16_t Ypos, uint16_t Length,uint16_t gradient_start);

void 	UiLcdDraw_EmptyRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width, uint16_t color);
void 	UiLcdDraw_FullRect (uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width, uint16_t color);

void 	UiLcdDraw_ColorPoint(uint16_t x, uint16_t y, uint16_t color);


void    UiLcdDraw_BottomButton(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width,uint16_t color);
#endif
