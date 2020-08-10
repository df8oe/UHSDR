/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  File name:		ui_lcd_layouts.h                                               **
 **  Description:   Layout definitions header file                                 **
 **  Licence:		GNU GPLv3                                                      **
 **  Author: 		Slawomir Balon/SP9BSL                                          **
 ************************************************************************************/

#ifndef UI_LCD_LAYOUTS_H_
#define UI_LCD_LAYOUTS_H_


#define COL_SPECTRUM_GRAD					0x40
#define Grid                RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD)      // COL_SPECTRUM_GRAD = 0x40
#define WATERFALL_HEIGHT 70
#define SPECTRUM_MAX_MARKER 3
#define SPECTRUM_SCOPE_GRID_VERT_COUNT  8
#define SPECTRUM_SCOPE_GRID_HORIZ 16


//#define LEFTBOX_WIDTH 58 // used for the lower left side controls
//#define LEFTBOX_ROW_H  (28)
//#define LEFTBOX_ROW_2ND_OFF  (13)

#define ENC_COL_W (37)
#define ENC_ROW_H (30)
#define ENC_ROW_2ND_OFF (14)
#define Xspacing 5
#define ui_txt_msg_buffer_size 51		//this defines the buffer size for text line in maximum possible configuration.
										//Please adjust it to maximum value+1 of ts.Layout->TextMsg_buffer_max
typedef enum
{
    RESOLUTION_320_240,
    RESOLUTION_480_320,
	RESOLUTION_800_480
} disp_resolution_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} UiArea_t;

typedef struct
{
	UiArea_t region;
	void (*function_short_press)(void);
	void (*function_long_press)(void);
} touchaction_descr_t;

typedef struct
{
	const touchaction_descr_t* actions;
	int32_t size;
} touchaction_list_descr_t;

typedef struct
{
    uint16_t x;
    uint16_t y;
} UiCoord_t ;

typedef struct {
	UiCoord_t Size;					// Lcd dimension

	UiCoord_t StartUpScreen_START;

	UiArea_t SpectrumWindow;		// Definition of spectrum window parameters

	UiCoord_t TUNE_FREQ;			// Frequency display control

	uint16_t TUNE_SPLIT_FREQ_X; 	//Shift with a small split to the right to close the frequency digits
	uint16_t TUNE_SPLIT_MARKER_X;
	uint16_t TUNE_SPLIT_FREQ_Y_TX;

	UiCoord_t TUNE_SFREQ;			// Second frequency display control

	UiCoord_t BAND_MODE;			// Band selection control
	UiArea_t BAND_MODE_MASK;

	UiArea_t DEMOD_MODE_MASK;		// Demodulator mode control

	UiArea_t AGC_MASK;				//AGC display mask

	UiArea_t TUNE_STEP; 			// Tunning step control

	UiArea_t BOTTOM_BAR;			// Bottom bar

	UiCoord_t ENCODER_IND;			// Encoder controls indicator
	uint8_t ENCODER_MODE;			//horizontal/vertical draw order

	UiArea_t LEFTBOXES_IND;			// Lower DSP box
	uint16_t LEFTBOXES_ROW_2ND_OFF;
	uint8_t LEFTBOXES_MODE;

	UiArea_t PW_IND;				// Power level

	UiArea_t DIGMODE;				// Digimode item
	UiArea_t SM_IND;				// S meter position
	UiCoord_t PWR_IND;				// Supply Voltage indicator
	UiCoord_t TEMP_IND;				// Temperature Indicator
	UiCoord_t RTC_IND;				// RTC

	UiCoord_t CW_DECODER_WPM;

	UiCoord_t TextMsgLine;			// coordinates for text line (CW decoder or freedv output)
	uint16_t TextMsg_buffer_max;	// Text message buffer size
	uint8_t  TextMsg_font;

	UiCoord_t SNAP_CARRIER;		// central position of variable freq marker

	UiCoord_t PWR_NUM_IND;	// Location of numerical FWD/REV power indicator

	UiCoord_t FREEDV_SNR;	//freeDV coordinates for status display
	UiCoord_t FREEDV_BER;
	uint16_t FREEDV_FONT;

	UiCoord_t DisplayDbm;

	UiCoord_t MEMORYLABEL;

	uint16_t LOADANDDEBUG_Y;
	uint16_t DEBUG_X;
	uint16_t LOAD_X;
	uint16_t SpectrumWindowPadding;

	uint16_t MENUSIZE;				// number of menu items per page/screen

	UiCoord_t MENU_IND;       	// X position of description of menu item being changed
	uint16_t MENU_CHANGE_X;     // Position of variable being changed
	uint16_t MENU_CURSOR_X;     // Position of cursor used to indicate selected item
	uint8_t MENU_TEXT_SIZE_MAX;	// One line maximum length

	const touchaction_list_descr_t* touchaction_list;
} LcdLayout;

enum MODE_{
	MODE_VERTICAL=0,
	MODE_HORIZONTAL
};

enum LcdLayout_{
	LcdLayout_320x240=0,
	LcdLayout_480x320,
	LcdLayout_800x480,
	LcdLayoutsCount		//this is last position enumerated used for layout array definition. Insert new layout name before this one
};

extern const LcdLayout LcdLayouts[LcdLayoutsCount];
extern disp_resolution_t disp_resolution;

#endif /* UI_LCD_LAYOUTS_H_ */
