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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __UI_LCD_HY28_H
#define __UI_LCD_HY28_H

#include "arm_math.h"
#include "math.h"
#include "ui_driver.h"

#define MAX_X  320
#define MAX_Y  320

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

// Dark grey colour used for spectrum scope grid
#define Grid				RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD)		// COL_SPECTRUM_GRAD = 0x40

#define LCD_DIR_HORIZONTAL	0x0000
#define LCD_DIR_VERTICAL	0x0001

#define GRADIENT_STEP			8


// ----------------------------------------------------------
// Spectrum draw params
//
// WARNING:  Because the waterfall uses a "block write" which is, in effect, a "blind" writing of data to the LCD, the size of the graphic
// block *MUST* exactly match the number of pixels within that block.
//
// Furthermore, the "SPECTRUM WIDTH" must match exactly with graphical width of the "X" size of each line to be written or skewing will result!
//
#define SPECTRUM_START_X		POS_SPECTRUM_IND_X
//
// Shift of whole spectrum in vertical direction
#define SPECTRUM_START_Y		(POS_SPECTRUM_IND_Y - 10)
//
// Spectrum hight is bit lower that the whole control
#define SPECTRUM_HEIGHT			(POS_SPECTRUM_IND_H - 10)
//
// Dependent on FFT samples,but should be less than control width!
#define SPECTRUM_WIDTH			256
// ----------------------------------------------------------

#define LCD_REG      (*((volatile unsigned short *) 0x60000000))
#define LCD_RAM      (*((volatile unsigned short *) 0x60020000))

// ----------------------------------------------------------
// Dual purpose pins (parallel + serial)
#define LCD_D11 				LCD_CS
#define LCD_D11_SOURCE			LCD_CS_SOURCE
#define LCD_D11_PIO         	LCD_CS_PIO

// ----------------------------------------------------------

void 	UiLcdHy28_LcdClear(ushort Color);
void 	UiLcdHy28_PrintText(ushort Xpos, ushort Ypos, char *str,ushort Color, ushort bkColor, uchar font);

void 	UiLcdHy28_DrawStraightLine(ushort Xpos, ushort Ypos, ushort Length, uchar Direction,ushort color);
void 	UiLcdHy28_DrawHorizLineWithGrad(ushort Xpos, ushort Ypos, ushort Length,ushort gradient_start);

void 	UiLcdHy28_DrawEmptyRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width, ushort color);
void 	UiLcdHy28_DrawBottomButton(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color);
void 	UiLcdHy28_DrawFullRect (ushort Xpos, ushort Ypos, ushort Height, ushort Width, ushort color);
void 	UiLcdHy28_DrawColorPoint(ushort x, ushort y, ushort color);
void 	UiLcdHy28_OpenBulkWrite(ushort x, ushort width, ushort y, ushort height);
void 	UiLcdHy28_SendByteSpi(uint8_t byte);
void 	UiLcdHy28_WriteDataOnly( unsigned short data);

void 	UiLcdHy28_BulkWrite(ushort Color);
void 	UiLcdHy28_CloseBulkWrite(void);

void 	UiLcdHy28_DrawSpectrum(q15_t *fft,ushort color,ushort shift);
void 	UiLcdHy28_DrawSpectrum_Interleaved(q15_t *fft_old, q15_t *fft_new, ushort color_old, ushort color_new,ushort shift);

void 	UiLcdHy28_Test(void);
uchar 	UiLcdHy28_Init(void);

void 	UiLcdHy28_ShowStartUpScreen(ulong hold_time);

#endif
