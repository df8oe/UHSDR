/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
#ifndef __UI_SPECTRUM_H
#define __UI_SPECTRUM_H
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
**  Licence:      GNU GPLv3                                                      **
************************************************************************************/
#include "uhsdr_board.h"
#include "uhsdr_types.h"
#include "audio_driver.h"
#include "arm_const_structs.h"

void UiSpectrum_Init();
void UiSpectrum_Clear();
void UiSpectrum_Redraw();
void UiSpectrum_WaterfallClearData();
void UiSpectrum_DisplayFilterBW();


// Settings for dB/division for spectrum display
enum
{
    DB_DIV_UNUSED = 0,
    DB_DIV_5,
    DB_DIV_7,
    DB_DIV_10,
    DB_DIV_15,
    DB_DIV_20,
    S_1_DIV,
    S_2_DIV,
    S_3_DIV,
    SCOPE_SCALE_NUM
};

#define DB_DIV_ADJUST_MIN                   DB_DIV_5
#define DB_DIV_ADJUST_MAX                   S_3_DIV
#define DB_DIV_ADJUST_DEFAULT               DB_DIV_10


// Enumeration to select which waterfall palette to use
enum
{
    WFALL_GRAY = 0,
    WFALL_HOT_COLD,
    WFALL_RAINBOW,
    WFALL_BLUE,
    WFALL_GRAY_INVERSE,
    WFALL_COLOR_NUM
};
//
#define WATERFALL_COLOR_MIN                 0
#define WATERFALL_COLOR_MAX                 (WFALL_COLOR_NUM-1)
#define WATERFALL_COLOR_DEFAULT             WFALL_GRAY
//
#define WATERFALL_STEP_SIZE_MIN             1
#define WATERFALL_STEP_SIZE_MAX             5
#define WATERFALL_STEP_SIZE_DEFAULT         2
//
#define WATERFALL_OFFSET_MIN                60
#define WATERFALL_OFFSET_MAX                140
#define WATERFALL_OFFSET_DEFAULT            100
//
#define WATERFALL_CONTRAST_MIN              10
#define WATERFALL_CONTRAST_MAX              225
#define WATERFALL_CONTRAST_DEFAULT          120
//
#define WATERFALL_SPEED_MIN                 1
#define WATERFALL_SPEED_MAX                 30
#define WATERFALL_SPEED_DEFAULT 10
//
#define WATERFALL_NOSIG_ADJUST_MIN          10
#define WATERFALL_NOSIG_ADJUST_MAX          30
#define WATERFALL_NOSIG_ADJUST_DEFAULT      20

//
#define WATERFALL_SPEED_WARN        5
#define WATERFALL_SPEED_WARN1       9
//
// Constants for waterfall size settings
//
enum
{
    SPECTRUM_NORMAL=0,
    SPECTRUM_BIG
};

enum
{
    FFT_WINDOW_RECTANGULAR=0,
    FFT_WINDOW_COSINE,
    FFT_WINDOW_BARTLETT,
    FFT_WINDOW_WELCH,
    FFT_WINDOW_HANN,
    FFT_WINDOW_HAMMING,
    FFT_WINDOW_BLACKMAN,
    FFT_WINDOW_NUTTALL,
    FFT_WINDOW_MAX
};

#define FFT_WINDOW_DEFAULT                  FFT_WINDOW_BLACKMAN

#define SPECTRUM_SIZE_DEFAULT               SPECTRUM_NORMAL


// Dependent on FFT samples,but should be less than control width!
#define SPECTRUM_WIDTH          256

// Spectrum scope operational constants

#define SPECTRUM_SCOPE_SPEED_MIN			1	// minimum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_MAX			25	// maximum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_DEFAULT		5
//
#define SPECTRUM_FILTER_MIN			1	// minimum filter setting
#define	SPECTRUM_FILTER_MAX			20	// maximum filter setting
#define SPECTRUM_FILTER_DEFAULT		4	// default filter setting
//
#define	SPECTRUM_SCOPE_AGC_MIN				1	// minimum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_MAX				50	// maximum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_DEFAULT			25	// default spectrum scope AGC rate setting
//
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MIN		10
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MAX		30
#define	SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT		20


#define	SPECTRUM_AGC_SCALING				25	// scaling factor by which the on-screen Spec. AGC Adj. is divided for adjustment.

#define	SCOPE_PREAMP_GAIN					1000//200	// amount of "amplification" in front of the FFT used for the Spectrum Scope and Waterfall used to overcome mathematical "noise floor"

#define INIT_SPEC_AGC_LEVEL					-80	// Initial offset for AGC level for spectrum/waterfall display

#define	NUMBER_WATERFALL_COLOURS			64		// number of colors in the waterfall table

// ----------------------------------------------------------
// Spectrum draw params
//
// Spectrum display
#define POS_SPECTRUM_IND_X					60
#define POS_SPECTRUM_IND_Y					150
#define POS_SPECTRUM_IND_H					80
#define POS_SPECTRUM_IND_W                  258

#define	POS_SPECTRUM_FREQ_BAR_Y				64	// reducing value moves upwards
#define	POS_SPECTRUM_FILTER_WIDTH_BAR_Y		61
#define COL_SPECTRUM_GRAD					0x40

#define     SPECTRUM_SCOPE_GRID_HORIZ 16
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

#define WATERFALL_MAX_SIZE (SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL)

// Dark grey colour used for spectrum scope grid
#define Grid                RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD)      // COL_SPECTRUM_GRAD = 0x40

#define SPECTRUM_MAX_MARKER 3
// Spectrum display
typedef struct SpectrumDisplay
{
    // Samples buffer
    float32_t   FFT_Samples[FFT_IQ_BUFF_LEN];
    float32_t   FFT_MagData[SPEC_BUFF_LEN];
    float32_t   FFT_AVGData[SPEC_BUFF_LEN];     // IIR low-pass filtered FFT buffer data

    // scope pixel data
    uint16_t    Old_PosData[SPECTRUM_WIDTH];

    // Current data ptr
    ulong   samp_ptr;

    // Addresses of vertical grid lines on x axis
    ushort  vert_grid_id[7];

    // Addresses of horizontal grid lines on x axis
    ushort  horz_grid_id[5];
    uint8_t upper_horiz_gridline;  // how many grid lines to use

    // State machine current state
    uchar   state;

    // Init done flag
    uchar   enabled;

    // Variables used in spectrum display AGC
    uint8_t   magnify;          // 2^magnify == zoom factor, max is 5

    float   display_offset;     // "vertical" offset for spectral scope, gain adjust for waterfall
    float   agc_rate;           // this holds AGC rate for the Spectrum Display
    float   db_scale;           // scaling factor for dB/division

    ushort  wfall_line_update;  // used to set the number of lines per update on the waterfall
    float   wfall_contrast;     // used to adjust the contrast of the waterfall display

    uint16_t waterfall_colours[NUMBER_WATERFALL_COLOURS+1];  // palette of colors for waterfall data

    uint8_t  waterfall[WATERFALL_MAX_SIZE][SPECTRUM_WIDTH];    // circular buffer used for storing waterfall data - remember to increase this if the waterfall is made larger!

    uint16_t wfall_line;        // pointer to current line of waterfall data
    uint16_t wfall_size;        // vertical size of the waterfall
    uint16_t wfall_ystart;

    uint16_t scope_size;
    uint16_t scope_ystart;

    float32_t pixel_per_hz;        // how many Hertz is one pixel in the spectrum
    float32_t rx_carrier_pos;      // where is the current receiving frequency carrier (in pixels)
    float32_t marker_offset[SPECTRUM_MAX_MARKER];   // how is the current transmitting frequency carrier offset from rx carrier (in pixels)

    float32_t marker_pos[SPECTRUM_MAX_MARKER];      // where is the current transmitting frequency carrier (in pixels)
    uint16_t  marker_line_pos_prev[SPECTRUM_MAX_MARKER]; // previous x-axis location of carrier line in screen coordinates, 0xffff indicates off screen
    uint16_t  marker_num; // how many marker lines we have right now
    uint16_t  marker_num_prev; // how many marker lines we had last time

    uint32_t   scope_centre_grid_colour_active;    // active colour of the spectrum scope center grid line
    uint32_t   scope_grid_colour_active;   // active color of spectrum scope grid;

} SpectrumDisplay;

// Spectrum display
extern SpectrumDisplay      sd;

#endif
