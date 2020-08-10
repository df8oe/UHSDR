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

#ifndef __UI_LCD_HY28_H
#define __UI_LCD_HY28_H

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

#define TOUCHSCREEN_NO_MIRROR_NOFLIP	0
#define TOUCHSCREEN_X_MIRROR_NOFLIP		1
#define TOUCHSCREEN_Y_MIRROR_NOFLIP		2
#define TOUCHSCREEN_XY_MIRROR_NOFLIP	3
#define TOUCHSCREEN_NO_MIRROR_FLIPXY	4
#define TOUCHSCREEN_X_MIRROR_FLIPXY		5
#define TOUCHSCREEN_Y_MIRROR_FLIPXY		6
#define TOUCHSCREEN_XY_MIRROR_FLIPXY	7


// ----------------------------------------------------------

typedef enum
{
    DISPLAY_NONE = 0,
    DISPLAY_HY28A_SPI,
    DISPLAY_HY28B_SPI,
    DISPLAY_HY28B_PARALLEL,
    DISPLAY_RA8875_SPI,
    DISPLAY_RA8875_PARALLEL,
    DISPLAY_ILI9486_PARALLEL,
    DISPLAY_RPI_SPI,
    DISPLAY_HY32D_PARALLEL_SSD1289,

	// keep this always at the end of the enum
	DISPLAY_NUM
} mchf_display_types_t;

typedef struct  {
    uint16_t reg;
    uint16_t val;
} RegisterValue_t;

typedef struct  RegisterValueSetInfo_s {
    const RegisterValue_t* addr;
    size_t size;
} RegisterValueSetInfo_t;

#define REGVAL_DATA (0xffff) // we indicate that the value is to be written using WriteData instead of WriteReg
#define REGVAL_DELAY (0x0000) // we indicate that the value is to be used as delay in ms instead of WriteReg

typedef struct
{
    mchf_display_types_t display_type;
    const char* name;
    uint16_t (*ReadDisplayId)(void);
    void (*SetActiveWindow) (uint16_t XLeft, uint16_t XRight, uint16_t YTop, uint16_t YBottom);
    void (*SetCursorA)( unsigned short Xpos, unsigned short Ypos );
    void (*WriteRAM_Prepare) (void);
    void (*WriteDataSpiStart_Prepare)(void);
    void (*WriteIndexSpi_Prepare)(void);
    void (*WriteReg)(unsigned short LCD_Reg, unsigned short LCD_RegValue);
    uint16_t (*ReadReg)( uint16_t LCD_Reg);
    void (*DrawStraightLine)(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color);
    void (*DrawFullRect)(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color);
    void (*DrawColorPoint)(uint16_t Xpos, uint16_t Ypos, uint16_t point);
    GPIO_TypeDef* spi_cs_port;
    uint16_t      spi_cs_pin;
    uint16_t      is_spi:1;
    uint16_t      spi_speed:1;
} uhsdr_display_info_t;


typedef struct
{
    uint8_t display_type;           // existence/identification of display type
    uint16_t DeviceCode;      		// LCD ident code
    bool use_spi;
    int16_t lcd_cs;
    uint16_t MAX_X;
    uint16_t MAX_Y;
    GPIO_TypeDef* lcd_cs_pio;
    const RegisterValueSetInfo_t* reg_info;
    void (*SetActiveWindow) (uint16_t XLeft, uint16_t XRight, uint16_t YTop, uint16_t YBottom);
    void (*SetCursorA)( unsigned short Xpos, unsigned short Ypos );
    void (*WriteRAM_Prepare) (void);
    void (*WriteDataSpiStart_Prepare)(void);
    void (*WriteIndexSpi_Prepare)(void);
    void (*WriteReg)(unsigned short LCD_Reg, unsigned short LCD_RegValue);
    uint16_t (*ReadReg)( uint16_t LCD_Reg);
    void (*DrawStraightLine)(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color);
    void (*DrawFullRect)(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color);
    void (*DrawColorPoint)(uint16_t Xpos, uint16_t Ypos, uint16_t point);
} mchf_display_t;

extern mchf_display_t mchf_display;

const uhsdr_display_info_t* UiLcdHy28_DisplayInfoGet(mchf_display_types_t display_type);

void 	UiLcdHy28_LcdClear(ushort Color);

uint16_t UiLcdHy28_PrintText(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor, uchar font);
uint16_t UiLcdHy28_PrintTextRight(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor, uchar font);
uint16_t UiLcdHy28_PrintTextCentered(const uint16_t bbX,const uint16_t bbY,const uint16_t bbW,const char* txt,uint32_t clr_fg,uint32_t clr_bg,uint8_t font);

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

void    UiLcdHy28_BulkPixel_OpenWrite(ushort x, ushort width, ushort y, ushort height);
void    UiLcdHy28_BulkPixel_CloseWrite(void);
void 	UiLcdHy28_BulkPixel_Put(uint16_t pixel);
void    UiLcdHy28_BulkPixel_PutBuffer(uint16_t* pixel_buffer, uint32_t len);
void    UiLcdHy28_BulkPixel_BufferFlush(void);

uint8_t 	UiLcdHy28_Init(void);

void    UiLcdHy28_BacklightEnable(bool on);

typedef struct
{
    uint8_t state;

    uint16_t xraw;
    uint16_t yraw;
    int16_t xraw_m1;
    int16_t xraw_m2;
    int16_t yraw_m1;
    int16_t yraw_m2;
    int16_t focus_xprev;
    int16_t focus_yprev;
    //int32_t xraw_avgBuff;
    //int32_t yraw_avgBuff;
    int16_t hr_x;
    int16_t hr_y;
    uint16_t xraw_prev;
    uint16_t yraw_prev;
    int32_t cal[6];
    bool present;

} mchf_touchscreen_t;


#define TP_DATASETS_VALID		0x04   // number of sets that must be identical for marked as VALID
#define TP_DATASETS_WAIT		0x01   // first dataset received
#define TP_DATASETS_PROCESSED	0xff
#define TP_DATASETS_NONE		0x00

extern mchf_touchscreen_t mchf_touchscreen;

void    UiLcdHy28_TouchscreenDetectPress(void);
void 	UiLcdHy28_TouchscreenReadCoordinates(void);
bool    UiLcdHy28_TouchscreenHasProcessableCoordinates(void);
void    UiLcdHy28_TouchscreenInit(uint8_t mirror);


/*
// FIXME: THIS MUST BE HANDLED DIFFERENTLY, IT DOES NOT ALWAYS SEES
// THE CONFIGURATION FROM uhsdr_board.h but due to dependency issues, we cannot include
// uhsdr_board.h at the top
#ifdef USE_DISP_480_320
    #define MAX_X  480
    #define MAX_Y  320
#elif defined(USE_DISP_800_480)
    #define MAX_X  800
    #define MAX_Y  480
#else
    #ifdef Simulate320_240_on_480_320
        #define MAX_X  480
        #define MAX_Y  320
    #else
        #define MAX_X  320
        #define MAX_Y  240
    #endif
#endif
*/
#endif
