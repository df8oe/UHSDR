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

// Logical Button definitions
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
    BUTTON_L1_PRESSED,		// and again one more function button
#endif
    //      init code relies on this
    BUTTON_NUM, // How many buttons we have defined
    BUTTON_NOP // used for buttons with no function
};

void        Keypad_KeypadInit(void);

#ifdef UI_BRD_MCHF
// this function invoked only on the MCHF with RTC as they have different buttons layout
void        Keypad_SetLayoutRTC_MCHF(void);
#endif

bool        Keypad_IsButtonPressed( uint32_t button_num );
bool        Keypad_IsAnyButtonPressed(void);
bool        Keypad_IsKeyPressed( uint32_t key_num );
bool        Keypad_IsAnyKeyPressed(void);

void        Keypad_Scan(void);

uint32_t    Keypad_KeyStates(void);
uint32_t    Keypad_ButtonStates(void);
const char* Keypad_GetLabelOfButton( uint32_t id_button );

#endif
