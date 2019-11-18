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

typedef struct
{
    const uint8_t width;
    const void* data;
} symbolData_t;

typedef struct
{
	const void *table;
    uint8_t  Width;
    uint8_t  Height;
    uint16_t maxCode;
    uint8_t BitCount;
    uint8_t firstCode;
    uint8_t  Spacing;
#ifdef USE_8bit_FONT
    const uint16_t *offsetTable;
    const uint8_t *heightTable;
    const uint8_t *widthTable;
#endif
} sFONT;


#endif //__UI_LCD_HY28_FONTS_H
