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

#ifndef __UI_LCD_TOUCHSCREEN_H
#define __UI_LCD_TOUCHSCREEN_H

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

#endif
