/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  File name:		ui_vkeybrd.h                                                   **
 **  Description:   Virtual keyboard definitions header file                       **
 **  Licence:		GNU GPLv3                                                      **
 **  Author: 		Slawomir Balon/SP9BSL                                          **
 ************************************************************************************/

#ifndef UI_UI_VKEYBRD_H_
#define UI_UI_VKEYBRD_H_


typedef uint8_t (*VKeyStateFunc)(uint8_t KeyNum, uint32_t param);
typedef void (*TouchFunc)(uint8_t KeyNum, uint32_t param);
typedef void (*VKeyRedrawFunc)(void);

typedef struct
{
	char KeyText[16];
	uint8_t SizeX;	//multiply of normal key size, 0=normal size defined by VKeypad's KeyWidth
	uint8_t SizeY;	//multiply of normal key size, 0=normal size defined by VKeypad's KeyHeight
	VKeyStateFunc KeyWarning;	//if function is present, this key will be marked with warning if result of function is 1
	uint16_t TextColor;	//Color Of Key Text when key is not pressed
	uint16_t PressedTextColor;	//Color Of Key Text when key is pressed
	TouchFunc ShortFnc;		//called function for short press
	uint32_t ShortPar;		//short press function calling parameter
	TouchFunc LongFnc;		//called function for short press
	uint32_t LongPar;		//long press function calling parameter
	uint8_t KeyFont;		//font number
} VKey;


typedef struct
{
	uint8_t NumberOfKeys;	//Number of keys in keyborad (see UiVk_MaxKeyCount and increase if more wanted)
	uint8_t Rows;			//Numer of rows
	uint8_t Columns;		//Numer of columns
	uint16_t KeyWidth;		//Width of single key
	uint16_t KeyHeight;		//Height of single key
	uint8_t KeySpacing;		//gap between keys
	uint8_t Backgr_Wnlarge;	//background of keyboard width enlarge
	uint8_t Backgr_Hnlarge;	//background of keyboard width enlarge
	const VKey* Keys;		//pointer to keyboard data
	uint8_t VKeyGroupMode;	//type of key press: see Vkey_Group_
	VKeyStateFunc VKeyStateCallBack;	//pointer to callback function which controls the displayed state of the button
	uint16_t YtopMargin;	//size of additional top margin between top edge and first key
	VKeyRedrawFunc RedrawFunct;	//pointer to callback function for redraw
} VKeypad;

#define UiVk_MaxKeyCount 24

#define Vbtn_State_Normal 0
#define Vbtn_State_Selected 1
#define Vbtn_State_Disabled 2
#define Vbtn_State_Pressed 8

enum Vkey_Group_ {Vkey_Group_OneAllowed=0, Vkey_Group_MultipleAllowed};


//void UiVk_RedrawDSPVirtualKeys(void);
bool UiVk_Process_VirtualKeypad(bool LongPress);
void UiVk_Redraw(void);

void UiVk_DSPVirtualKeys(void);

//void UiVk_RedrawBndSelVirtualKeys(void);
void UiVk_BndSelVirtualKeys(void);

//void UiVk_RedrawBndFreqSetVirtualKeys(void);
void UiVk_BndFreqSetVirtualKeys(void);

//void UiVk_RedrawModSelVirtualKeys(void);
void UiVk_ModSelVirtualKeys(void);
#endif /* UI_UI_VKEYBRD_H_ */
