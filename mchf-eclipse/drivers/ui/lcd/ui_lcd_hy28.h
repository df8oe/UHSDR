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
#include "uhsdr_board_config.h"

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


typedef struct mchf_display_t
{
    uint8_t display_type;           // existence/identification of display type
    uint16_t DeviceCode;      		// LCD ident code
    bool use_spi;
    uint32_t lcd_spi_prescaler;
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

void    UiLcdHy28_BulkPixel_OpenWrite(ushort x, ushort width, ushort y, ushort height);
void    UiLcdHy28_BulkPixel_CloseWrite(void);
void 	UiLcdHy28_BulkPixel_Put(uint16_t pixel);
void    UiLcdHy28_BulkPixel_PutBuffer(uint16_t* pixel_buffer, uint32_t len);

uint8_t 	UiLcdHy28_Init(void);

void    UiLcdHy28_BacklightEnable(bool on);

typedef struct mchf_touchscreen_t
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

#ifdef USE_GFX_RA8875
extern const RegisterValueSetInfo_t ra8875_regs;

uint16_t UiLcdRa8875_ReadDisplayId(void);
void UiLcdRa8875_SetActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
        uint16_t YBottom);
void UiLcdRa8875_WriteRAM_Prepare(void);
void UiLcdRa8875_WriteIndexSpi_Prepare(void);
void UiLcdRa8875_WriteDataSpiStart_Prepare(void);
void UiLcdRa8875_SetCursorA( unsigned short Xpos, unsigned short Ypos );
void UiLcdRa8875_DrawColorPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point);
void UiLcdRa8875_DrawStraightLine(uint16_t x, uint16_t y, uint16_t Length, uint8_t Direction,uint16_t color);
void UiLcdRa8875_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width ,uint16_t color);
void UiLcdRa8875_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t UiLcdRa8875_ReadReg(uint16_t LCD_Reg);
#endif


void UiLcdHy28_SpiSendByte(uint8_t);
uint8_t UiLcdHy28_SpiReadByte(void);
void UiLcdHy28_WriteRAM_Prepare_Index(uint16_t);

void UiLcdHy28_LcdSpiWrite16(uint16_t data);
uint8_t UiLcdHy28_LcdSpiWrite8Read8(uint8_t data);


#endif
