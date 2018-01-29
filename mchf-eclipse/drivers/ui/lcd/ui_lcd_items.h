/*
 * ui_lcd_items.h
 *
 *  Created on: 11.10.2017
 *      Author: Slawomir Balon SP9BSL
 */

#ifndef UI_LCD_UI_LCD_ITEMS_H_
#define UI_LCD_UI_LCD_ITEMS_H_


#define COL_SPECTRUM_GRAD					0x40
#define Grid                RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD)      // COL_SPECTRUM_GRAD = 0x40
#define WATERFALL_HEIGHT 70
#define SPECTRUM_MAX_MARKER 3
#define SPECTRUM_SCOPE_GRID_VERT_COUNT  8
#define SPECTRUM_SCOPE_GRID_HORIZ 16


#define LEFTBOX_WIDTH 58 // used for the lower left side controls
#define LEFTBOX_ROW_H  (28)
#define LEFTBOX_ROW_2ND_OFF  (13)

#define ENC_COL_W (37)
#define ENC_ROW_H (30)
#define ENC_ROW_2ND_OFF (14)
#define Xspacing 5
#define ui_txt_msg_buffer_size 51		//this defines the buffer size for text line in maximum possible configuration.
										//Please adjust it to maximum value+1 of ts.Layout->TextMsg_buffer_max
typedef enum
{
    RESOLUTION_320_240,
    RESOLUTION_480_320
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
	void (*function_short_press)();
	void (*function_long_press)();
} touchaction_descr_t;

typedef struct
{
	const touchaction_descr_t* actions;
	int32_t size;
} touchaction_list_descr_t;

typedef struct {
	uint16_t SizeX;					// Lcd dimension
	uint16_t SizeY;					// Lcd dimension

	uint16_t StartUpScreen_START_X;
	uint16_t StartUpScreen_START_Y;

	UiArea_t SpectrumWindow;		// Definition of spectrum window parameters

	uint16_t TUNE_FREQ_X;			// Frequency display control
	uint16_t TUNE_FREQ_Y;			// Frequency display control

	uint16_t TUNE_SPLIT_FREQ_X; 	//Shift with a small split to the right to close the frequency digits
	uint16_t TUNE_SPLIT_MARKER_X;
	uint16_t TUNE_SPLIT_FREQ_Y_TX;

	uint16_t TUNE_SFREQ_X;			// Second frequency display control
	uint16_t TUNE_SFREQ_Y;

	uint16_t BAND_MODE_X;			// Band selection control
	uint16_t BAND_MODE_Y;
	uint16_t BAND_MODE_MASK_X;
	uint16_t BAND_MODE_MASK_Y;
	uint16_t BAND_MODE_MASK_H;
	uint16_t BAND_MODE_MASK_W;

	uint16_t DEMOD_MODE_X;          // Demodulator mode control
	uint16_t DEMOD_MODE_Y;
	uint16_t DEMOD_MODE_MASK_X;
	uint16_t DEMOD_MODE_MASK_Y;
	uint16_t DEMOD_MODE_MASK_H;
	uint16_t DEMOD_MODE_MASK_W;

	UiArea_t TUNE_STEP; 			// Tunning step control

	UiArea_t BOTTOM_BAR;			// Bottom bar

	uint16_t ENCODER_IND_X;			// Encoder controls indicator
	uint16_t ENCODER_IND_Y;
	uint8_t ENCODER_MODE;			//horizontal/vertical draw order

	uint16_t LEFTBOXES_IND_X;		// Lower DSP box
	uint16_t LEFTBOXES_IND_Y;
	uint16_t LEFTBOXES_WIDTH;		// used for the lower left side controls
	uint16_t LEFTBOXES_ROW_H;
	uint16_t LEFTBOXES_ROW_2ND_OFF;
	uint8_t LEFTBOXES_MODE;

	uint16_t PW_IND_X;				// Power level
	uint16_t PW_IND_Y;

	UiArea_t DIGMODE;				// Digimode item
	UiArea_t SM_IND;				// S meter position
	UiArea_t PWR_IND;				// Supply Voltage indicator
	UiArea_t TEMP_IND;				// Temperature Indicator
	UiArea_t RTC_IND;				// RTC

	uint16_t CW_DECODER_WPM_X;
	uint16_t CW_DECODER_WPM_Y;

	uint16_t TextMsgLine_X;			// coordinates for text line (CW decoder or freedv output)
	uint16_t TextMsgLine_Y;
	uint16_t TextMsg_buffer_max;	// Text message buffer size
	uint8_t  TextMsg_font;

	uint16_t SNAP_CARRIER_X;		// central position of variable freq marker
	uint16_t SNAP_CARRIER_Y;

	uint16_t PWR_NUM_IND_X;	// Location of numerical FWD/REV power indicator
	uint16_t PWR_NUM_IND_Y;


	uint16_t FREEDV_SNR_X;	//freeDV coordinates for status display
	uint16_t FREEDV_BER_X;
	uint16_t FREEDV_SNR_Y;
	uint16_t FREEDV_BER_Y;
	uint16_t FREEDV_FONT;

	uint16_t DisplayDbm_X;
	uint16_t DisplayDbm_Y;

	uint16_t MEMORYLABEL_X;
	uint16_t MEMORYLABEL_Y;

	uint16_t LOADANDDEBUG_Y;
	uint16_t DEBUG_X;
	uint16_t LOAD_X;
	uint16_t SpectrumWindowPadding;

	uint16_t MENUSIZE;				// number of menu items per page/screen

	uint16_t MENU_IND_X;       	// X position of description of menu item being changed
	uint16_t MENU_IND_Y;        // Y position of first (top) item being changed
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
	LcdLayoutsCount		//this is last position enumerated used for layout array definition. Insert new layout name before this one
};

extern const LcdLayout LcdLayouts[LcdLayoutsCount];
extern disp_resolution_t disp_resolution;




//------------ LEFT ONLY FOR REFERENCE -----------------
//new hVGA lcd definitions
//#ifdef USE_DISP_480_320
// ----------------------------------------------------------
//Splash Screen location
//#define StartUpScreen_START_X 80
//#define StartUpScreen_START_Y 60
/*
// Spectrum draw params
//
// Dependent on FFT samples,but should be less than control width!
#define SPECTRUM_WIDTH          480

// Spectrum display
#define POS_SPECTRUM_IND_X					0
#define POS_SPECTRUM_IND_Y					130
#define POS_SPECTRUM_IND_H					80
#define POS_SPECTRUM_IND_W                  480

#define	POS_SPECTRUM_FREQ_BAR_Y				0	// reducing value moves upwards
#define	POS_SPECTRUM_FREQ_BAR_H				16	//height of freq text bar
#define	POS_SPECTRUM_FILTER_WIDTH_BAR_Y		61

//#define POS_SPECTRUM_INFO_BAR_H             16  //height of description bar (in normal size mode)

#define     SPECTRUM_SCOPE_GRID_HORIZ 16

#define     SPECTRUM_SCOPE_GRID_VERT  (SPECTRUM_WIDTH/SPECTRUM_SCOPE_GRID_VERT_COUNT)


#define SPECTRUM_SCOPE_TOP_LIMIT            5   // Top limit of spectrum scope magnitude

#define	WFALL_MEDIUM_ADDITIONAL	12					// additional vertical height in pixels of "medium" waterfall

// WARNING:  Because the waterfall uses a "block write" which is, in effect, a "blind" writing of data to the LCD, the size of the graphic
// block *MUST* exactly match the number of pixels within that block.
//
// Furthermore, the "SPECTRUM WIDTH" must match exactly with graphical width of the "X" size of each line to be written or skewing will result!
//
#define SPECTRUM_START_X		POS_SPECTRUM_IND_X
// Shift of whole spectrum in vertical direction
#define SPECTRUM_START_Y		(POS_SPECTRUM_IND_Y - 10)
// Spectrum height is bit lower that the whole control
#define SPECTRUM_HEIGHT			(POS_SPECTRUM_IND_H - 10)



// How much larger than the NORMAL spectrum display should the BIG Spectrum display be?
#define SPEC_LIGHT_MORE_POINTS 15


#define     POS_SPECTRUM_GRID_VERT_START (POS_SPECTRUM_IND_X-1)
#define     POS_SPECTRUM_GRID_HORIZ_START (POS_SPECTRUM_IND_Y + 11 + 34 + POS_SPECTRUM_FREQ_BAR_H)

//location of BW filter indicator
#define POS_FILTER_BW_Y (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y+POS_SPECTRUM_FREQ_BAR_H)

#define WATERFALL_MAX_SIZE (SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL)

	#define WATERFALL_START_Y (POS_SPECTRUM_IND_Y+POS_SPECTRUM_IND_H)//240



// Dark grey colour used for spectrum scope grid

*/


// POSITIONS START
/*
// Frequency display control
#define POS_TUNE_FREQ_X             		280
#define POS_TUNE_FREQ_Y             		36


#define POS_TUNE_SPLIT_FREQ_X           	POS_TUNE_FREQ_X+80//Shift with a small split to the right to close the frequency digits.
#define POS_TUNE_SPLIT_MARKER_X         	POS_TUNE_FREQ_X+40
#define POS_TUNE_SPLIT_FREQ_Y_TX        	POS_TUNE_FREQ_Y+12

// Second frequency display control
#define POS_TUNE_SFREQ_X            		(POS_TUNE_FREQ_X + 120)
#define POS_TUNE_SFREQ_Y            		(POS_TUNE_FREQ_Y - 20)


// Band selection control
#define POS_BAND_MODE_X             		(POS_TUNE_FREQ_X + 160)
#define POS_BAND_MODE_Y             		(POS_TUNE_FREQ_Y + 7)
#define POS_BAND_MODE_MASK_X            	(POS_BAND_MODE_X - 1)
#define POS_BAND_MODE_MASK_Y            	(POS_BAND_MODE_Y - 1)
#define POS_BAND_MODE_MASK_H            	13
#define POS_BAND_MODE_MASK_W            	33

// Demodulator mode control
#define POS_DEMOD_MODE_X            		(POS_TUNE_FREQ_X + 1)
#define POS_DEMOD_MODE_Y            		(POS_TUNE_FREQ_Y - 20)
#define POS_DEMOD_MODE_MASK_X           	(POS_DEMOD_MODE_X - 1)
#define POS_DEMOD_MODE_MASK_Y           	(POS_DEMOD_MODE_Y - 1)
#define POS_DEMOD_MODE_MASK_H           	13
#define POS_DEMOD_MODE_MASK_W           	41

// Tunning step control
#define POS_TUNE_STEP_X             		(POS_TUNE_FREQ_X + 45)
#define POS_TUNE_STEP_Y             		(POS_TUNE_FREQ_Y - 21)
#define POS_TUNE_STEP_MASK_H            	15
#define POS_TUNE_STEP_MASK_W            	(SMALL_FONT_WIDTH*7)
*/

//>>>>>>

//#define POS_RADIO_MODE_X            		4
//#define POS_RADIO_MODE_Y            		5
/*
// Bottom bar
#ifdef UI_BRD_OVI40
	#define POS_BOTTOM_BAR_X            		0
#else
	#define POS_BOTTOM_BAR_X            		32
#endif
#define POS_BOTTOM_BAR_Y            		308
#define POS_BOTTOM_BAR_BUTTON_W         	74
#define POS_BOTTOM_BAR_BUTTON_H         	16

// Virtual Button 1
#define POS_BOTTOM_BAR_F1_X         		(POS_BOTTOM_BAR_X + 2)
#define POS_BOTTOM_BAR_F1_Y         		POS_BOTTOM_BAR_Y
*/
// --------------------------------------------------
// Encoder controls indicator
/*#define POS_ENCODER_IND_X                	128
#define POS_ENCODER_IND_Y                	64*/


// --------------------------------------------------
// Standalone controls
//
// DSP mode
// Lower DSP box
/*
#define POS_LEFTBOXES_IND_X              	0
#define POS_LEFTBOXES_IND_Y              	64
*/


// Power level
/*
#define POS_PW_IND_X                		POS_DEMOD_MODE_X -1
#define POS_PW_IND_Y                		POS_DEMOD_MODE_Y - 16
*/

/*
#define POS_DIGMODE_IND_X              		215
#define POS_DIGMODE_IND_Y              		0
#define POS_DIGMODE_IND_H					58
*/
// S meter position
/*
#define POS_SM_IND_X                		0
#define POS_SM_IND_Y                		0
#define SM_IND_W (200)
#define SM_IND_H (72 - BTM_MINUS)
*/
// Supply Voltage indicator
//#define POS_PWRN_IND_X              		0
/*
#define POS_PWRN_IND_Y              		193

#define POS_PWR_IND_X               		420
#define POS_PWR_IND_Y               		307
*/

// Temperature Indicator
/*
#define POS_TEMP_IND_X              		370
#define POS_TEMP_IND_Y              		64
*/
// RTC
/*
#define POS_RTC_X							415
#define POS_RTC_Y							80
*/
/*
#define POS_LOADANDDEBUG_Y					96
#define POS_DEBUG_X							0
#define POS_LOAD_X							280
*/
//
// Location of numerical FWD/REV power indicator
//
/*
#define	POS_PWR_NUM_IND_X					320
#define	POS_PWR_NUM_IND_Y					96
*/

/*
#define POS_CW_DECODER_WPM_X 	420 //205
#define POS_CW_DECODER_WPM_Y 	290 //38 // 116 --> above DSP box //79 --> this collides with the RTC!

#define CW_SNAP_CARRIER_X	242 //232 // central position of variable freq marker
#define CW_SNAP_CARRIER_Y	29  //96 //52 // position of variable freq marker
*/
//coordinates for text line (CW decoder or freedv output)
/*
#define POS_TextMsgLine_X 						0   //205
#define POS_TextMsgLine_Y 						290 //28
#define ui_txt_msg_buffer_max 50 // no more space on single screen line
#define ui_txt_msg_font 0
*/

//freeDV coordinates for status display
/*
#define POS_FREEDV_SNR_X 280
#define POS_FREEDV_BER_X 380
#define POS_FREEDV_SNR_Y 28
#define POS_FREEDV_BER_Y 28
#define FREEDV_UI_FONT 4
*/
// POSITIONS END
/*
#else
// ----------------------------------------------------------
//Splash Screen location
#define StartUpScreen_START_X 0
#define StartUpScreen_START_Y 10

// Spectrum draw params
//
// Dependent on FFT samples,but should be less than control width!
#define SPECTRUM_WIDTH          256
// Spectrum display
#define POS_SPECTRUM_IND_X					60
#define POS_SPECTRUM_IND_Y					150
#define POS_SPECTRUM_IND_H					80
#define POS_SPECTRUM_IND_W                  258

#define	POS_SPECTRUM_FREQ_BAR_Y				64	// reducing value moves upwards
#define POS_SPECTRUM_FREQ_BAR_H             16  //height of freq text bar
#define	POS_SPECTRUM_FILTER_WIDTH_BAR_Y		61
#define COL_SPECTRUM_GRAD					0x40

#define     SPECTRUM_SCOPE_GRID_HORIZ 16
#define     SPECTRUM_SCOPE_GRID_VERT_COUNT  8
#define     SPECTRUM_SCOPE_GRID_VERT  32


#define SPECTRUM_SCOPE_TOP_LIMIT            5   // Top limit of spectrum scope magnitude

#define	WFALL_MEDIUM_ADDITIONAL	12					// additional vertical height in pixels of "medium" waterfall

// WARNING:  Because the waterfall uses a "block write" which is, in effect, a "blind" writing of data to the LCD, the size of the graphic
// block *MUST* exactly match the number of pixels within that block.
//
// Furthermore, the "SPECTRUM WIDTH" must match exactly with graphical width of the "X" size of each line to be written or skewing will result!
//
#define SPECTRUM_START_X		POS_SPECTRUM_IND_X
// Shift of whole spectrum in vertical direction
#define SPECTRUM_START_Y		(POS_SPECTRUM_IND_Y - 10)
// Spectrum height is bit lower that the whole control
#define SPECTRUM_HEIGHT			(POS_SPECTRUM_IND_H - 10)


// How much larger than the NORMAL spectrum display should the BIG Spectrum display be?
#define SPEC_LIGHT_MORE_POINTS 15


#define     POS_SPECTRUM_GRID_VERT_START (POS_SPECTRUM_IND_X-1)
#define     POS_SPECTRUM_GRID_HORIZ_START (POS_SPECTRUM_IND_Y + 11 + 32)

//location of BW filter indicator
#define POS_FILTER_BW_Y (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y)

#define WATERFALL_MAX_SIZE (SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL)

// Dark grey colour used for spectrum scope grid
#define Grid                RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD)      // COL_SPECTRUM_GRAD = 0x40

#define SPECTRUM_MAX_MARKER 3



// POSITIONS START

// Frequency display control
#define POS_TUNE_FREQ_X             		116
#define POS_TUNE_FREQ_Y             		100

#define POS_TUNE_SPLIT_FREQ_X           	POS_TUNE_FREQ_X+80//Shift with a small split to the right to close the frequency digits.
#define POS_TUNE_SPLIT_MARKER_X         	POS_TUNE_FREQ_X+40
#define POS_TUNE_SPLIT_FREQ_Y_TX        	POS_TUNE_FREQ_Y+12

// Second frequency display control
#define POS_TUNE_SFREQ_X            		(POS_TUNE_FREQ_X + 120)
#define POS_TUNE_SFREQ_Y            		(POS_TUNE_FREQ_Y - 20)

// Band selection control
#define POS_BAND_MODE_X             		(POS_TUNE_FREQ_X + 160)
#define POS_BAND_MODE_Y             		(POS_TUNE_FREQ_Y + 7)
#define POS_BAND_MODE_MASK_X            	(POS_BAND_MODE_X - 1)
#define POS_BAND_MODE_MASK_Y            	(POS_BAND_MODE_Y - 1)
#define POS_BAND_MODE_MASK_H            	13
#define POS_BAND_MODE_MASK_W            	33

// Demodulator mode control
#define POS_DEMOD_MODE_X            		(POS_TUNE_FREQ_X + 1)
#define POS_DEMOD_MODE_Y            		(POS_TUNE_FREQ_Y - 20)
#define POS_DEMOD_MODE_MASK_X           	(POS_DEMOD_MODE_X - 1)
#define POS_DEMOD_MODE_MASK_Y           	(POS_DEMOD_MODE_Y - 1)
#define POS_DEMOD_MODE_MASK_H           	13
#define POS_DEMOD_MODE_MASK_W           	41

// Tunning step control
#define POS_TUNE_STEP_X             		(POS_TUNE_FREQ_X + 45)
#define POS_TUNE_STEP_Y             		(POS_TUNE_FREQ_Y - 21)
#define POS_TUNE_STEP_MASK_H            	15
#define POS_TUNE_STEP_MASK_W            	(SMALL_FONT_WIDTH*7)

//#define POS_RADIO_MODE_X            		4
//#define POS_RADIO_MODE_Y            		5

// Bottom bar
#define POS_BOTTOM_BAR_X            		0
#define POS_BOTTOM_BAR_Y            		228
#define POS_BOTTOM_BAR_BUTTON_W         	62
#define POS_BOTTOM_BAR_BUTTON_H         	16

// Virtual Button 1
#define POS_BOTTOM_BAR_F1_X         		(POS_BOTTOM_BAR_X + 2)
#define POS_BOTTOM_BAR_F1_Y         		POS_BOTTOM_BAR_Y

// --------------------------------------------------
// Encoder controls indicator
#define POS_ENCODER_IND_X                	0
#define POS_ENCODER_IND_Y                	16


// --------------------------------------------------
// Standalone controls
//
// DSP mode
// Lower DSP box
#define POS_LEFTBOXES_IND_X              	0
#define POS_LEFTBOXES_IND_Y              	130
#define LEFTBOX_WIDTH 58 // used for the lower left side controls
#define LEFTBOX_ROW_H  (28)
#define LEFTBOX_ROW_2ND_OFF  (13)

// Power level
#define POS_PW_IND_X                		POS_DEMOD_MODE_X -1
#define POS_PW_IND_Y                		POS_DEMOD_MODE_Y - 16

#define POS_DIGMODE_IND_X              		0
#define POS_DIGMODE_IND_Y              		(191)
#define POS_DIGMODE_IND_H					58

// S meter position
#define POS_SM_IND_X                		116
#define POS_SM_IND_Y                		0
#define SM_IND_W (200)
#define SM_IND_H (72 - BTM_MINUS)

// Supply Voltage indicator
//#define POS_PWRN_IND_X              		0
#define POS_PWRN_IND_Y              		193

#define POS_PWR_IND_X               		4
#define POS_PWR_IND_Y               		(POS_PWRN_IND_Y + 15)


// Temperature Indicator
#define POS_TEMP_IND_X              		0
#define POS_TEMP_IND_Y              		0

// RTC
#define POS_RTC_X							0
#define POS_RTC_Y							79

#define POS_LOADANDDEBUG_Y					95
#define POS_DEBUG_X							0
#define POS_LOAD_X							280

//
// Location of numerical FWD/REV power indicator
//
#define	POS_PWR_NUM_IND_X					1
#define	POS_PWR_NUM_IND_Y					80

#define POS_CW_DECODER_WPM_X 	0
#define POS_CW_DECODER_WPM_Y 	108 // 116 --> above DSP box //79 --> this collides with the RTC!

#define CW_SNAP_CARRIER_X	27 // central position of variable freq marker
#define CW_SNAP_CARRIER_Y	122 // position of variable freq marker

//coordinates for text line (CW decoder or freedv output)
#define POS_TextMsgLine_X 						5
#define POS_TextMsgLine_Y 						92
#define ui_txt_msg_buffer_max 44 // no more space on single screen line
#define ui_txt_msg_font 4
//remove it!!

	#define WATERFALL_START_Y 240
	#define WATERFALL_HEIGHT 60

//freeDV coordinates for status display
#define POS_FREEDV_SNR_X 5
#define POS_FREEDV_BER_X 5
#define POS_FREEDV_SNR_Y 116
#define POS_FREEDV_BER_Y 104
#define FREEDV_UI_FONT 4
// POSITIONS END
#endif

//common items for all lcds

#define POS_DisplayDbm_X					(POS_TUNE_FREQ_X+45)
#define POS_DisplayDbm_Y					(POS_TUNE_FREQ_Y-36)

#define POS_MEMORYLABEL_X (POS_TUNE_FREQ_X+45+(SMALL_FONT_WIDTH * 11)+4)
#define POS_MEMORYLABEL_Y (POS_TUNE_FREQ_Y-36)
*/
#endif /* UI_LCD_UI_LCD_ITEMS_H_ */
