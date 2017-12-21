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

#ifndef __UI_LCD_HY28_FONTS_H
#define __UI_LCD_HY28_FONTS_H

//typedef const uint16_t fontPixel_t;

typedef struct tFont
{
    //fontPixel_t *table;
	const void *table;
    uint8_t  Width;
    uint8_t  Height;
    uint16_t maxCode;
    uint8_t BitCount;
    uint8_t firstCode;
    const uint16_t *offsetTable;
    const uint8_t *heightTable;
    const uint8_t *widthTable;
    //const uint8_t *DataTable;
} sFONT;


/*
#ifdef USE_Aliased_FONT
typedef struct Font_{
	const unsigned short *FontHeader;
	const unsigned char *SizeX;
	const unsigned char *SizeY;
	const unsigned short *Offset;
	const unsigned char *Data;
}sFont;

#define FontHeader_SizeX 0
#define FontHeader_SizeY 1
#define FontHeader_FirstFont 2
#define FontHeader_FontCount 3
#define FontHeader_ColorDepth 4
#define FontHeader_MaxFontDataSize 5
#endif*/

#endif //__UI_LCD_HY28_FONTS_H
