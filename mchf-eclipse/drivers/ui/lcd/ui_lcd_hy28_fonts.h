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

typedef const uint16_t fontPixel_t;

typedef struct tFont
{
    fontPixel_t *table;
    uint8_t  Width;
    uint8_t  Height;
    uint16_t maxCode;
} sFONT;


#endif
