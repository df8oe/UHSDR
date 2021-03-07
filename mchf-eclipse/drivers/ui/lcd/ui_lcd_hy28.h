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

const uhsdr_display_info_t* UiLcd_DisplayInfoGet(mchf_display_types_t display_type);

// interface for mass pixel transfers into rectangular areas
void    UiLcd_BulkPixel_OpenWrite(uint16_t x, uint16_t width, uint16_t y, uint16_t height);
void    UiLcd_BulkPixel_CloseWrite(void);
void 	UiLcd_BulkPixel_Put(uint16_t pixel);
void    UiLcd_BulkPixel_PutBuffer(uint16_t* pixel_buffer, uint32_t len);
void    UiLcd_BulkPixel_FinishWaitWrite(void);


uint8_t UiLcd_Init(void);

void    UiLcd_BacklightDimHandler(void);
void    UiLcd_BacklightEnable(bool on);

// =================== LOW LEVEL DRIVER INTERFACE FUNCTIONS =====================
// do not call these from the ui layer

void    UiLcd_WriteRAM_Prepare_Index(uint16_t);

// low level SPI functions
void    UiLcd_SpiSendByte(uint8_t);
//uint8_t UiLcd_SpiReadByte(void);
void    UiLcd_SpiFinishTransfer(void);
void    UiLcd_SpiSetPrescaler(uint32_t baudrate_prescaler);

// lcd spi interface specific functions
bool    UiLcd_LcdSpiUsed(void);
void    UiLcd_LcdSpiWrite16(uint16_t data);
uint8_t UiLcd_LcdSpiWrite8Read8(uint8_t data);



#endif
