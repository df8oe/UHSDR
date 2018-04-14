/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/
#ifndef __UHSDR_KEYPAD_H
#define __UHSDR_KEYPAD_H
#include "uhsdr_board.h"

// Key map structure
// represents a physical key which can be pressed (via GPIO)
typedef struct
{
    GPIO_TypeDef    *keyPort;
    uint16_t        keyPin;
    uint16_t         button_id;
    const char*     label;

} Keypad_KeyPhys_t;

// represents a logical button
typedef struct
{
    uint16_t        button_id;
    const char*     label;
} UhsdrButtonLogical_t;

typedef struct
{
    const Keypad_KeyPhys_t* map;
    uint32_t num;
} UhsdrHwKey_t;

// Logical Button definitions
//
enum
{
    BUTTON_M2_PRESSED = 0,  // 0
    BUTTON_G3_PRESSED,  	// 1
    BUTTON_G2_PRESSED,  	// 2
    BUTTON_BNDM_PRESSED,    // 3
    BUTTON_G4_PRESSED,  	// 4
    BUTTON_M3_PRESSED,  	// 5
    BUTTON_STEPM_PRESSED,   // 6
    BUTTON_STEPP_PRESSED,   // 7
    BUTTON_M1_PRESSED,  	// 8
    BUTTON_F3_PRESSED,  	// 9 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F1_PRESSED,  	// 10 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F2_PRESSED,  	// 11 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_F4_PRESSED,  	// 12 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_BNDP_PRESSED,    // 13
    BUTTON_F5_PRESSED,  	// 14 - Press and release handled in UiDriverProcessFunctionKeyClick()
    BUTTON_G1_PRESSED,  	// 15
    BUTTON_PWR_PRESSED,   // 16 - Used for press and release
    TOUCHSCREEN_ACTIVE, 	// 17 - Touchscreen touched, needs to last entry before BUTTON_NUM,
#ifdef UI_BRD_OVI40
    BUTTON_F6_PRESSED,      // we have one more function button
    BUTTON_E1_PRESSED,      // encoder 1 click
    BUTTON_E2_PRESSED,      // encoder 1 click
    BUTTON_E3_PRESSED,      // encoder 1 click
    BUTTON_E4_PRESSED,      // encoder 1 click
#endif
    //      init code relies on this
    BUTTON_NUM, // How many buttons we have defined
    BUTTON_NOP // used for buttons with no function
};

extern UhsdrHwKey_t  hwKeys; // these buttons represent the gpio to logical button id mapping
extern const UhsdrButtonLogical_t  buttons[]; // this array gives us the names of the available logical buttons

const Keypad_KeyPhys_t* bm_set_normal;
#ifdef UI_BRD_MCHF
const Keypad_KeyPhys_t* bm_set_rtc;
#endif

bool Keypad_IsButtonPressed(uint32_t button_num);
bool Keypad_IsAnyButtonPressed();
bool Keypad_IsKeyPressed(uint32_t key_num);
bool Keypad_IsAnyKeyPressed();
uint32_t Keypad_KeyStates();
uint32_t Keypad_ButtonStates();
void Keypad_Scan();

void Keypad_KeypadInit(UhsdrHwKey_t* keyMap);

#endif
