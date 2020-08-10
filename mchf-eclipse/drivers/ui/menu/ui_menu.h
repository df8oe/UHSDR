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

#ifndef __UI_MENU_H
#define __UI_MENU_H

#include "uhsdr_board.h"
//
// Exports
//
void UiMenu_MapColors(uint32_t color ,char* options,volatile uint32_t* clr_ptr);


// The supported mode values
typedef enum
{
    MENU_RENDER_ONLY = 0, // no value change
    MENU_PROCESS_VALUE_CHANGE,  // user value change permitted
    MENU_PROCESS_VALUE_SETDEFAULT, // set value to default, no user change
} MenuProcessingMode_t;

void UiMenu_RenderMenu(MenuProcessingMode_t mode);

void UiMenu_RenderChangeItemValue(int16_t pot_diff);
void UiMenu_RenderChangeItem(int16_t pot_diff);
void UiMenu_RenderLastScreen(void);
void UiMenu_RenderFirstScreen(void);
bool UiMenu_RenderNextScreen(void); // returns true if screen was changed, i.e. not last screen
bool UiMenu_RenderPrevScreen(void); // returns true if screen was changed, i.e. not first screen


enum MENU_INFO_ITEM
{
    INFO_EEPROM,
    INFO_DISPLAY,
    INFO_DISPLAY_CTRL,
#ifdef USE_OSC_SI570
    INFO_SI570,
#endif
	INFO_OSC_NAME,
    INFO_TP,
    INFO_RFBOARD,
    INFO_CPU,
    INFO_FLASH,
    INFO_RAM,
    INFO_FW_VERSION,
    INFO_BL_VERSION,
    INFO_BUILD,
    INFO_RTC,
    INFO_VBAT,
    INFO_LICENCE,
    INFO_HWLICENCE,
    INFO_CODEC,
    INFO_CODEC_TWINPEAKS,
};

const char* UiMenu_GetSystemInfo(uint32_t* m_clr_ptr, int info_item);
#define MAX_MENUSIZE 14		//memory allocation of displayed menuitems at once, must be higher or equal to highest ts.Layout->MENUSIZE
/*
#ifdef USE_DISP_480_320
//
#define	MENUSIZE							14		// number of menu items per page/screen

//
// Starting position of configuration menu
//
#define POS_MENU_IND_X                      80      // X position of description of menu item being changed
#define POS_MENU_IND_Y                      110     // Y position of first (top) item being changed
#define POS_MENU_CHANGE_X                   280     // Position of variable being changed
#define POS_MENU_CURSOR_X                   360     // Position of cursor used to indicate selected item

#else
//
#define	MENUSIZE							6		// number of menu items per page/screen

//
// Starting position of configuration menu
//
#define POS_MENU_IND_X                      60      // X position of description of menu item being changed
#define POS_MENU_IND_Y                      128     // Y position of first (top) item being changed
#define POS_MENU_CHANGE_X                   244     // Position of variable being changed
#define POS_MENU_CURSOR_X                   311     // Position of cursor used to indicate selected item
#endif
*/

#endif //__UI_MENU_H
