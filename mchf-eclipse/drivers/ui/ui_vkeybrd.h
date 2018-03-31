/*
 * ui_vkeybrd.h
 *
 *  Created on: 31.03.2018
 *      Author: Slawomir Balon SP9BSL
 */

#ifndef UI_UI_VKEYBRD_H_
#define UI_UI_VKEYBRD_H_


typedef uint8_t (*VKeyStateFunc)(uint8_t KeyNum);
typedef void (*TouchFunc)(uint8_t KeyNum);

typedef struct
{
	char KeyText[16];
	uint8_t SizeX;	//multiply of normal key size, 0=normal size defined by layout VbtnHeight and VbtnWidth
	uint8_t SizeY;
	uint8_t KeyWarning;	//1= this key will be marked with warning
	uint16_t TextColor;	//Color Of Key Text when key is not pressed
	uint16_t PressedTextColor;	//Color Of Key Text when key is pressed
	TouchFunc ShortFnc;		//called function for short press
	TouchFunc LongFnc;		//called function for short press
} VKey;


typedef struct
{
	uint8_t NumberOfKeys;
	uint8_t Rows;
	uint8_t Columns;
	const VKey* Keys;
	uint8_t VKeyGroupMode;	//type of key press: see Vkey_Group_
	VKeyStateFunc VKeyStateCallBack;
} VKeypad;



#define Vbtn_State_Normal 0
#define Vbtn_State_Selected 1
#define Vbtn_State_Disabled 2
#define Vbtn_State_Pressed 8

enum Vkey_Group_ {Vkey_Group_OneAllowed, Vkey_Group_MultipleAllowed};


void UiVk_RedrawDSPVirtualKeys();
bool UiVk_Process_VirtualKeypad(bool LongPress);

void UiVk_DSPVirtualKeys();
#endif /* UI_UI_VKEYBRD_H_ */
