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

#ifndef __UI_LCD_HY28_H
#define __UI_LCD_HY28_H

#include "arm_math.h"
#include "math.h"
#include "ui_driver.h"
#include "ui_spectrum.h"
#include "mchf_board.h"

#define MAX_X  320
#define MAX_Y  240


#ifndef HY28BHISPEED
  #define HY28BHISPEED false
#endif

#define SPI_START   (0x70)              /* Start byte for SPI transfer        */
#define SPI_RD      (0x01)              /* WR bit 1 within start              */
#define SPI_WR      (0x00)              /* WR bit 0 within start              */
#define SPI_DATA    (0x02)              /* RS bit 1 within start byte         */
#define SPI_INDEX   (0x00)              /* RS bit 0 within start byte         */

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
#define	Grey6				RGB(0x78,0x78,0x78)
//
#define	RX_Grey				RGB(0xb8,0xdb,0xa8)	// slightly green grey
#define TX_Grey				RGB(0xe8,0xad,0xa0)	// slightly red(ish) grey (more magenta, actually...)


#define LCD_DIR_HORIZONTAL	0x0000
#define LCD_DIR_VERTICAL	0x0001

#define GRADIENT_STEP			8


// ----------------------------------------------------------

#define LCD_REG      (*((volatile unsigned short *) 0x60000000))
#define LCD_RAM      (*((volatile unsigned short *) 0x60020000))

// ----------------------------------------------------------
// Dual purpose pins (parallel + serial)
#define LCD_CS 					LCD_CSA
#define LCD_CS_SOURCE			LCD_CSA_SOURCE
#define LCD_CS_PIO	         	LCD_CSA_PIO

// ----------------------------------------------------------
void 	UiLcdHy28_LcdClear(ushort Color);

void UiLcdHy28_PrintText(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor, uchar font);
void UiLcdHy28_PrintTextRight(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor, uchar font);
void UiLcdHy28_PrintTextCentered(const uint16_t bbX,const uint16_t bbY,const uint16_t bbW,const char* txt,uint32_t clr_fg,uint32_t clr_bg,uint8_t font);

uint16_t UiLcdHy28_TextWidth(const char *str, uchar font);
uint16_t UiLcdHy28_TextHeight(uint8_t font);

void 	UiLcdHy28_DrawStraightLine(ushort Xpos, ushort Ypos, ushort Length, uchar Direction,ushort color);
void    UiLcdHy28_DrawStraightLineDouble(ushort Xpos, ushort Ypos, ushort Length, uchar Direction,ushort color);
void    UiLcdHy28_DrawStraightLineTriple(ushort Xpos, ushort Ypos, ushort Length, uchar Direction,ushort color);
void 	UiLcdHy28_DrawHorizLineWithGrad(ushort Xpos, ushort Ypos, ushort Length,ushort gradient_start);

void 	UiLcdHy28_DrawEmptyRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width, ushort color);
void 	UiLcdHy28_DrawBottomButton(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color);
void 	UiLcdHy28_DrawFullRect (ushort Xpos, ushort Ypos, ushort Height, ushort Width, ushort color);

void 	UiLcdHy28_DrawColorPoint(ushort x, ushort y, ushort color);

void 	UiLcdHy28_OpenBulkWrite(ushort x, ushort width, ushort y, ushort height);
void 	UiLcdHy28_BulkPixel_Put(uint16_t pixel);
void 	UiLcdHy28_BulkWrite(uint16_t* pixels, uint32_t len);
void 	UiLcdHy28_BulkWriteColor(uint16_t color, uint32_t len);
void 	UiLcdHy28_CloseBulkWrite(void);

uint8_t 	UiLcdHy28_Init(void);

void 	UiLcdHy28_GetTouchscreenCoordinates(bool);	// 1 == corrected data, 0 == raw data
void	UiLcdHy28_ReadTcData(void);

#endif
