/*
 * ui_vkeybrd.c
 *
 *  Created on: 31.03.2018
 *      Author: Slawomir Balon SP9BSL
 */

#include "uhsdr_board.h"
#include "ui_spectrum.h"
#include "ui_driver.h"


#define Col_BtnForeCol RGB(0xA0,0xA0,0xA0)
#define Col_BtnForePressed RGB(0x80,0x80,0x80)
#define Col_BtnLightLeftTop RGB(0xE0,0xE0,0xE0)
#define Col_BtnLightRightBot RGB(0x60,0x60,0x60)
#define Col_BtnDisabled RGB(0x80,0x80,0x80)


static void UiVk_DrawButton(uint16_t Xpos, uint16_t Ypos, bool Warning, uint8_t Vbtn_State, const char* txt, uint16_t text_color_NotPressed, uint16_t text_color_Pressed)
{
	//UiLcdHy28_DrawEmptyRect(Xpos,Ypos,ts.Layout->VbtnHeight,ts.Layout->VbtnWidth,White);
	uint16_t col_LeftUP, col_RightBot, col_Bcgr, col_Text;

	switch(Vbtn_State)
	{
	case Vbtn_State_Disabled:
		col_LeftUP=Col_BtnDisabled;
		col_RightBot=Col_BtnDisabled;
		col_Bcgr=Black;
		col_Text=Col_BtnDisabled;
		break;
	case Vbtn_State_Pressed:
		col_LeftUP=Col_BtnLightRightBot;
		col_RightBot=Col_BtnLightLeftTop;
		col_Bcgr=Col_BtnForePressed;
		col_Text=text_color_Pressed;
		break;
	case Vbtn_State_Normal:
		col_LeftUP=Col_BtnLightLeftTop;
		col_RightBot=Col_BtnLightRightBot;
		col_Bcgr=Col_BtnForeCol;
		col_Text=text_color_NotPressed;
		break;
	}

	UiLcdHy28_DrawFullRect(Xpos+1,Ypos+1,ts.Layout->VbtnHeight-2,ts.Layout->VbtnWidth-2,col_Bcgr);

	UiLcdHy28_DrawStraightLine(Xpos,Ypos,ts.Layout->VbtnHeight,LCD_DIR_VERTICAL,col_LeftUP);
	UiLcdHy28_DrawStraightLine(Xpos,Ypos,ts.Layout->VbtnWidth,LCD_DIR_HORIZONTAL,col_LeftUP);
	UiLcdHy28_DrawStraightLine(Xpos+ts.Layout->VbtnWidth-1,Ypos,ts.Layout->VbtnHeight-1,LCD_DIR_VERTICAL,col_RightBot);
	UiLcdHy28_DrawStraightLine(Xpos+1,Ypos+ts.Layout->VbtnHeight-1,ts.Layout->VbtnWidth-1,LCD_DIR_HORIZONTAL,col_RightBot);

	UiLcdHy28_PrintTextCentered(Xpos+2,Ypos+8,ts.Layout->VbtnWidth-4,txt,col_Text,col_Bcgr,0);
}

static void UiVk_GetKeybArea(UiArea_t* KeybArea)
{
	KeybArea->w=ts.VirtualKeyPad->Columns*(ts.Layout->VbtnWidth)+
			(ts.VirtualKeyPad->Columns-1)*ts.Layout->VbtnSpacing;
	KeybArea->h=ts.VirtualKeyPad->Rows*(ts.Layout->VbtnHeight)+
			(ts.VirtualKeyPad->Rows-1)*ts.Layout->VbtnSpacing;

	KeybArea->x=sd.Slayout->full.x+sd.Slayout->full.w/2-KeybArea->w/2;
	KeybArea->y=sd.Slayout->full.y+sd.Slayout->full.h/2-KeybArea->h/2;
}

static void UiVk_GetButtonRgn(uint8_t Key, UiArea_t *bp)
{
	int row, col;
	row=Key/ts.VirtualKeyPad->Columns;
	col=Key-row*ts.VirtualKeyPad->Columns;
	UiArea_t KeybArea;
	UiVk_GetKeybArea(&KeybArea);

	bp->x=KeybArea.x+col*(ts.Layout->VbtnWidth+ts.Layout->VbtnSpacing);
	bp->y=KeybArea.y+row*(ts.Layout->VbtnHeight+ts.Layout->VbtnSpacing);
	bp->w=ts.Layout->VbtnWidth;
	bp->h=ts.Layout->VbtnHeight;
}

static void UiVk_DrawVKeypad()
{
	const VKeypad* VKpad=ts.VirtualKeyPad;
	int row, col, keycnt=0;
	UiArea_t b_area;
	for(row=0;row<VKpad->Rows;row++)
	{
		for(col=0;col<VKpad->Columns;col++)
		{
			if(keycnt==VKpad->NumberOfKeys)
			{
				return;
			}
			UiVk_GetButtonRgn(keycnt,&b_area);

			UiVk_DrawButton(b_area.x,b_area.y,
					VKpad->Keys[keycnt].KeyWarning,VKpad->VKeyStateCallBack(keycnt),VKpad->Keys[keycnt].KeyText,VKpad->Keys[keycnt].TextColor,VKpad->Keys[keycnt].PressedTextColor);

			keycnt++;
		}
	}
}


bool UiVk_Process_VirtualKeypad(bool is_long_press)
{
	bool TouchProcessed=UiDriver_CheckTouchRegion(&sd.Slayout->full);	//if the area of virtual keys pressed, mark as processed for other actions

	UiArea_t bp;
	for(int i=0;i<ts.VirtualKeyPad->NumberOfKeys;i++)
	{
		UiVk_GetButtonRgn(i,&bp);
		if(UiDriver_CheckTouchRegion(&bp))
		{
			ts.VirtualKeyPad->Keys[i].ShortFnc(i);
		}
	}

	return TouchProcessed;
}


static void UiVk_DrawBackGround()
{
	UiArea_t Ka;//drawing the background
	UiVk_GetKeybArea(&Ka);

	Ka.x-=2;
	Ka.y-=2;
	Ka.w+=4;
	Ka.h+=4;
	uint16_t col_LeftUP=Col_BtnLightLeftTop;
	uint16_t col_RightBot=Col_BtnLightRightBot;
	uint16_t col_Bcgr=Col_BtnForeCol;

	UiLcdHy28_DrawFullRect(Ka.x+1,Ka.y+1,Ka.h-2,Ka.w-2,col_Bcgr);

	UiLcdHy28_DrawStraightLine(Ka.x,Ka.y,Ka.h,LCD_DIR_VERTICAL,col_LeftUP);
	UiLcdHy28_DrawStraightLine(Ka.x+1,Ka.y,Ka.w-1,LCD_DIR_HORIZONTAL,col_LeftUP);
	UiLcdHy28_DrawStraightLine(Ka.x+Ka.w,Ka.y+1,Ka.h-1,LCD_DIR_VERTICAL,col_RightBot);
	UiLcdHy28_DrawStraightLine(Ka.x+1,Ka.y+Ka.h,Ka.w-1,LCD_DIR_HORIZONTAL,col_RightBot);

}

//End of main body of virtual keyboard
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//The real definitions of virtual keyboard starts here

static void UiVk_DSPVKeyCallBackShort(uint8_t KeyNum)
{
	switch(KeyNum)
	{
	case 0:
		ts.dsp_mode=DSP_SWITCH_OFF;
		break;
	case 1:
		ts.dsp_mode=DSP_SWITCH_NR;
		break;
	case 2:
		ts.dsp_mode=DSP_SWITCH_NOTCH;
		break;
	case 3:
		ts.dsp_mode=DSP_SWITCH_NR_AND_NOTCH;
		break;
	case 4:
		ts.dsp_mode=DSP_SWITCH_NOTCH_MANUAL;
		break;
	case 5:
		ts.dsp_mode=DSP_SWITCH_PEAK_FILTER;
		break;
	}
	UiDriver_UpdateDSPmode();
}

static void UiVk_DSPVKeyCallBackLong(uint8_t KeyNum)
{

}

static uint8_t UiVk_DSPVKeyInitTypeDraw(uint8_t Keynum)
{
	uint8_t Keystate=Vbtn_State_Normal;
	uint32_t dsp_functions_active =UiDriver_GetActiveDSPFunctions();

	switch(Keynum)
	{
	case 0:
		if(dsp_functions_active==0)
		{
			Keystate=Vbtn_State_Pressed;
		}
		break;
	case 1:
		if(dsp_functions_active==DSP_NR_ENABLE)
		{
			Keystate=Vbtn_State_Pressed;
		}
		break;
	case 2:
		if(dsp_functions_active==DSP_NOTCH_ENABLE)
		{
			Keystate=Vbtn_State_Pressed;
		}
		break;
	case 3:
		if(dsp_functions_active==(DSP_NOTCH_ENABLE|DSP_NR_ENABLE))
		{
			Keystate=Vbtn_State_Pressed;
		}
		break;
	case 4:
		if(dsp_functions_active==DSP_MNOTCH_ENABLE)
		{
			Keystate=Vbtn_State_Pressed;
		}
		break;
	case 5:
		if(dsp_functions_active==DSP_MPEAK_ENABLE)
		{
			Keystate=Vbtn_State_Pressed;
		}
		break;
	}
	return Keystate;
}
#define col_Keys_DSP_pr RGB(0x20,0xff,0x20)		//text color when pressed
#define col_Keys_DSP_npr RGB(0,0x20,0x20)		//text color when in normal state

const VKey Keys_DSP[]={
		{.ShortFnc=UiVk_DSPVKeyCallBackShort, .LongFnc=UiVk_DSPVKeyCallBackLong,.KeyText="DSP\nOFF", .TextColor=RGB(0,0x20,0x20), .PressedTextColor=RGB(0,0xff,0xff)},
		{.ShortFnc=UiVk_DSPVKeyCallBackShort, .LongFnc=UiVk_DSPVKeyCallBackLong,.KeyText="NR", .TextColor=col_Keys_DSP_npr, .PressedTextColor=col_Keys_DSP_pr},
		{.ShortFnc=UiVk_DSPVKeyCallBackShort, .LongFnc=UiVk_DSPVKeyCallBackLong,.KeyText="A\nNOTCH", .TextColor=col_Keys_DSP_npr, .PressedTextColor=col_Keys_DSP_pr},
		{.ShortFnc=UiVk_DSPVKeyCallBackShort, .LongFnc=UiVk_DSPVKeyCallBackLong,.KeyText="NR\n+NOTCH", .TextColor=col_Keys_DSP_npr, .PressedTextColor=col_Keys_DSP_pr},
		{.ShortFnc=UiVk_DSPVKeyCallBackShort, .LongFnc=UiVk_DSPVKeyCallBackLong,.KeyText="M\nNOTCH", .TextColor=col_Keys_DSP_npr, .PressedTextColor=col_Keys_DSP_pr},
		{.ShortFnc=UiVk_DSPVKeyCallBackShort, .LongFnc=UiVk_DSPVKeyCallBackLong,.KeyText="PEAK", .TextColor=col_Keys_DSP_npr, .PressedTextColor=col_Keys_DSP_pr}
};

const VKeypad Keypad_DSP={
	.NumberOfKeys=6,
	.Rows=2,
	.Columns=3,
	.Keys=Keys_DSP,
	.VKeyGroupMode=Vkey_Group_OneAllowed,
	.VKeyStateCallBack=UiVk_DSPVKeyInitTypeDraw
};

uint32_t prev_dsp_functions_active;	//used for virtual DSP keys redraw detection

void UiVk_RedrawDSPVirtualKeys()
{
	if(ts.VirtualKeysShown_flag)
	{
		uint32_t dsp_functions_active =UiDriver_GetActiveDSPFunctions();
		if(prev_dsp_functions_active!=dsp_functions_active)
		{
			prev_dsp_functions_active=dsp_functions_active;
			UiVk_DrawVKeypad();
		}
	}
}

void UiVk_DSPVirtualKeys()
{
	if(ts.VirtualKeysShown_flag)
	{
		ts.VirtualKeysShown_flag=false;
		UiSpectrum_Init();
	}
	else
	{
		ts.VirtualKeyPad=&Keypad_DSP;
		prev_dsp_functions_active=-1;
		ts.VirtualKeysShown_flag=true;
		UiSpectrum_Clear();
		UiVk_DrawBackGround();
		UiVk_RedrawDSPVirtualKeys();
	}
}



