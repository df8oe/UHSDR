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

#ifndef __UI_LCD_RA8875_H
#define __UI_LCD_RA8875_H

#include "uhsdr_types.h"

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

void UiLcdRa8875_setScrollWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB);
void UiLcdRa8875_setScrollMode(uint8_t mode);
void UiLcdRa8875_scroll(int16_t x,int16_t y);
void UiLcdRa8875_MoveAreaDown(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h, uint16_t down);
#endif
#endif
